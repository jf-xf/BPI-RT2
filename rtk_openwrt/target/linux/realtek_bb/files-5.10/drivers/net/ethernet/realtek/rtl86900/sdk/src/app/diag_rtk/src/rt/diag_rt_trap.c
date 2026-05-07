#include <stdio.h>
#include <string.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <diag_util.h>
#include <diag_str.h>
#include <parser/cparser_priv.h>
#include <rtk/rt/rt_trap.h>

/*
 * rt_trap init
 */
cparser_result_t
cparser_cmd_rt_trap_init(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rt_trap_init(), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_trap_init */

/*
 * rt_trap get oam state
 */
cparser_result_t 
cparser_cmd_rt_trap_get_oam_state(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rt_action_t action;
    rt_enable_t state;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rt_trap_oamPduAction_get(&action), ret);

    if(action == ACTION_TRAP2CPU)
        state = ENABLED;
    else
        state = DISABLED;

    diag_util_printf("\n OAM Trap : %s\n", diagStr_enable[state]);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_trap_get_oam_state */

/*
 * rt_trap set oam state ( disable | enable )
 */
cparser_result_t 
cparser_cmd_rt_trap_set_oam_state_disable_enable(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rt_action_t action;

    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(4, 0))
        action = ACTION_TRAP2CPU;
    else
        action = ACTION_FORWARD;

    DIAG_UTIL_ERR_CHK(rt_trap_oamPduAction_set(action), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_trap_set_oam_state_disable_enable */

/*
 * rt_trap get omci state
 */
cparser_result_t 
cparser_cmd_rt_trap_get_omci_state(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rt_action_t action;
    rt_enable_t state;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rt_trap_omciAction_get(&action), ret);

    if(action == ACTION_TRAP2CPU)
        state = ENABLED;
    else
        state = DISABLED;

    diag_util_printf("\n OMCI Trap : %s\n", diagStr_enable[state]);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_trap_get_omci_state */

/*
 * rt_trap set omci state ( disable | enable )
 */
cparser_result_t 
cparser_cmd_rt_trap_set_omci_state_disable_enable(cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rt_action_t action;

    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(4, 0))
        action = ACTION_TRAP2CPU;
    else
        action = ACTION_FORWARD;

    DIAG_UTIL_ERR_CHK(rt_trap_omciAction_set(action), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_trap_set_omci_state_disable_enable */


/*
 * rt_trap set igmp port ( <PORT_LIST:ports> | all ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_rt_trap_set_igmp_port_ports_all_state_disable_enable(
    cparser_context_t *context,
    char * *ports_ptr)
{
    rt_port_t port;
    diag_portlist_t portlist;
    rt_action_t action;
    int32 ret;
    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(6, 0))
        action = RT_ACTION_TRAP2CPU;
    else
        action = RT_ACTION_FORWARD;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rt_trap_portIgmpMldCtrlPktAction_set(port, RT_IGMPMLD_TYPE_IGMPV2, action), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_trap_set_igmp_port_ports_all_state_disable_enable */

/*
 * rt_trap get igmp port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t
cparser_cmd_rt_trap_get_igmp_port_ports_all_state(
    cparser_context_t *context,
    char * *ports_ptr)
{
    rt_port_t port;
    diag_portlist_t portlist;
    rt_action_t action;
    rt_enable_t state;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rt_trap_portIgmpMldCtrlPktAction_get(port, RT_IGMPMLD_TYPE_IGMPV2, &action), ret);
        if(action == ACTION_TRAP2CPU)
            state = ENABLED;
        else
            state = DISABLED;
        diag_util_printf("\n  Port %d IGMP/MLD trap state: %s", port, diagStr_enable[state]);
    }
    diag_util_printf("\n");
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_trap_get_igmp_port_ports_all_state */

/*
 * rt_trap set dhcp port ( <PORT_LIST:ports> | all ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_rt_trap_set_dhcp_port_ports_all_state_disable_enable(
    cparser_context_t *context,
    char * *ports_ptr)
{
    rt_port_t port;
    diag_portlist_t portlist;
    rt_action_t action;
    int32 ret;
    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(6, 0))
        action = RT_ACTION_TRAP2CPU;
    else
        action = RT_ACTION_FORWARD;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rt_trap_portDhcpPktAction_set(port, action), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_trap_set_dhcp_port_ports_all_state_disable_enable */

/*
 * rt_trap get dhcp port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t
cparser_cmd_rt_trap_get_dhcp_port_ports_all_state(
    cparser_context_t *context,
    char * *ports_ptr)
{
    rt_port_t port;
    diag_portlist_t portlist;
    rt_action_t action;
    rt_enable_t state;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rt_trap_portDhcpPktAction_get(port, &action), ret);
        if(action == ACTION_TRAP2CPU)
            state = ENABLED;
        else
            state = DISABLED;
        diag_util_printf("\n  Port %d DHCP trap state: %s", port, diagStr_enable[state]);
    }
    diag_util_printf("\n");
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_trap_get_dhcp_port_ports_all_state */

/*
 * rt_trap set pppoe port ( <PORT_LIST:ports> | all ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_rt_trap_set_pppoe_port_ports_all_state_disable_enable(
    cparser_context_t *context,
    char * *ports_ptr)
{
    rt_port_t port;
    diag_portlist_t portlist;
    rt_action_t action;
    int32 ret;
    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(6, 0))
        action = RT_ACTION_TRAP2CPU;
    else
        action = RT_ACTION_FORWARD;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rt_trap_portPppoePktAction_set(port, action), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_trap_set_pppoe_port_ports_all_state_disable_enable */

/*
 * rt_trap get pppoe port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t
cparser_cmd_rt_trap_get_pppoe_port_ports_all_state(
    cparser_context_t *context,
    char * *ports_ptr)
{
    rt_port_t port;
    diag_portlist_t portlist;
    rt_action_t action;
    rt_enable_t state;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rt_trap_portPppoePktAction_get(port, &action), ret);
        if(action == ACTION_TRAP2CPU)
            state = ENABLED;
        else
            state = DISABLED;
        diag_util_printf("\n  Port %d PPPoE trap state: %s", port, diagStr_enable[state]);
    }
    diag_util_printf("\n");
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_trap_get_pppoe_port_ports_all_state */

/*
 * rt_trap set stp port ( <PORT_LIST:ports> | all ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_rt_trap_set_stp_port_ports_all_state_disable_enable(
    cparser_context_t *context,
    char * *ports_ptr)
{
    rt_port_t port;
    diag_portlist_t portlist;
    rt_action_t action;
    int32 ret;
    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(6, 0))
        action = RT_ACTION_TRAP2CPU;
    else
        action = RT_ACTION_FORWARD;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rt_trap_portStpPktAction_set(port, action), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_trap_set_stp_port_ports_all_state_disable_enable */

/*
 * rt_trap get stp port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t
cparser_cmd_rt_trap_get_stp_port_ports_all_state(
    cparser_context_t *context,
    char * *ports_ptr)
{
    rt_port_t port;
    diag_portlist_t portlist;
    rt_action_t action;
    rt_enable_t state;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rt_trap_portStpPktAction_get(port, &action), ret);
        if(action == ACTION_TRAP2CPU)
            state = ENABLED;
        else
            state = DISABLED;
        diag_util_printf("\n  Port %d STP trap state: %s", port, diagStr_enable[state]);
    }
    diag_util_printf("\n");
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_trap_get_stp_port_ports_all_state */

/*
 * rt_trap set host port ( <PORT_LIST:ports> | all ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_rt_trap_set_host_port_ports_all_state_disable_enable(
    cparser_context_t *context,
    char * *ports_ptr)
{
    rt_port_t port;
    diag_portlist_t portlist;
    rt_action_t action;
    int32 ret;
    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(6, 0))
        action = RT_ACTION_TRAP2CPU;
    else
        action = RT_ACTION_FORWARD;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rt_trap_portHostPktAction_set(port, action), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_trap_set_host_port_ports_all_state_disable_enable */

/*
 * rt_trap get host port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t
cparser_cmd_rt_trap_get_host_port_ports_all_state(
    cparser_context_t *context,
    char * *ports_ptr)
{
    rt_port_t port;
    diag_portlist_t portlist;
    rt_action_t action;
    rt_enable_t state;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rt_trap_portHostPktAction_get(port, &action), ret);
        if(action == ACTION_TRAP2CPU)
            state = ENABLED;
        else
            state = DISABLED;
        diag_util_printf("\n  Port %d Host packet trap state: %s", port, diagStr_enable[state]);
    }
    diag_util_printf("\n");
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_trap_get_host_port_ports_all_state */

/*
 * rt_trap set ethertype port ( <PORT_LIST:ports> | all ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_rt_trap_set_ethertype_port_ports_all_state_disable_enable(
    cparser_context_t *context,
    char * *ports_ptr)
{
    rt_port_t port;
    diag_portlist_t portlist;
    rt_action_t action;
    int32 ret;
    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(6, 0))
        action = RT_ACTION_TRAP2CPU;
    else
        action = RT_ACTION_FORWARD;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rt_trap_portEthertypePktAction_set(port, action), ret);
    }
    diag_util_printf("\n");
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_trap_set_ethertype_port_ports_all_state_disable_enable */

/*
 * rt_trap get ethertype port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t
cparser_cmd_rt_trap_get_ethertype_port_ports_all_state(
    cparser_context_t *context,
    char * *ports_ptr)
{
    rt_port_t port;
    diag_portlist_t portlist;
    rt_action_t action;
    rt_enable_t state;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rt_trap_portEthertypePktAction_get(port, &action), ret);
        if(action == ACTION_TRAP2CPU)
            state = ENABLED;
        else
            state = DISABLED;
        diag_util_printf("\n  Port %d special ethertype packet trap state: %s", port, diagStr_enable[state]);
    }
    diag_util_printf("\n");
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_trap_get_ethertype_port_ports_all_state */

/*
 * rt_trap set arp port ( <PORT_LIST:ports> | all ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_rt_trap_set_arp_port_ports_all_state_disable_enable(
    cparser_context_t *context,
    char * *ports_ptr)
{
    rt_port_t port;
    diag_portlist_t portlist;
    rt_action_t action;
    int32 ret;
    DIAG_UTIL_PARAM_CHK();

    if('e' == TOKEN_CHAR(6, 0))
        action = RT_ACTION_TRAP2CPU;
    else
        action = RT_ACTION_FORWARD;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rt_trap_portArpPktAction_set(port, action), ret);
    }
    diag_util_printf("\n");
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_trap_set_arp_port_ports_all_state_disable_enable */

/*
 * rt_trap get arp port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t
cparser_cmd_rt_trap_get_arp_port_ports_all_state(
    cparser_context_t *context,
    char * *ports_ptr)
{
    rt_port_t port;
    diag_portlist_t portlist;
    rt_action_t action;
    rt_enable_t state;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rt_trap_portArpPktAction_get(port, &action), ret);
        if(action == ACTION_TRAP2CPU)
            state = ENABLED;
        else
            state = DISABLED;
        diag_util_printf("\n  Port %d ARP trap state: %s", port, diagStr_enable[state]);
    }
    diag_util_printf("\n");
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_trap_get_arp_port_ports_all_state */

/*
 * rt_trap get pattern ethertype
 */
cparser_result_t
cparser_cmd_rt_trap_get_pattern_ethertype(
    cparser_context_t *context)
{
    rt_trap_pattern_t pattern;
    int32 ret;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&pattern, 0, sizeof(rt_trap_pattern_t));
    DIAG_UTIL_ERR_CHK(rt_trap_pattern_get(RT_TRAP_PATTERN_TYPE_ETHERTYPE, &pattern), ret);
    diag_util_mprintf("\n  Trap ethertype: 0x%04x\n",pattern.ethertype);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_trap_get_pattern_ethertype */

/*
 * rt_trap get pattern host-mac
 */
cparser_result_t
cparser_cmd_rt_trap_get_pattern_host_mac(
    cparser_context_t *context)
{
    rt_trap_pattern_t pattern;
    int32 ret;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&pattern, 0, sizeof(rt_trap_pattern_t));
    DIAG_UTIL_ERR_CHK(rt_trap_pattern_get(RT_TRAP_PATTERN_TYPE_HOST_MAC, &pattern), ret);
    diag_util_mprintf("\n  Trap host MAC : %s\n",diag_util_inet_mactoa(pattern.hostMac));

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_trap_get_pattern_host_mac */

/*
 * rt_trap set pattern ethertype <UINT:ethertype>
 */
cparser_result_t
cparser_cmd_rt_trap_set_pattern_ethertype_ethertype(
    cparser_context_t *context,
    uint32_t  *ethertype_ptr)
{
    rt_trap_pattern_t pattern;
    int32 ret;
    DIAG_UTIL_PARAM_CHK();

    memset(&pattern, 0, sizeof(rt_trap_pattern_t));
    pattern.ethertype = *ethertype_ptr;
    DIAG_UTIL_ERR_CHK(rt_trap_pattern_set(RT_TRAP_PATTERN_TYPE_ETHERTYPE, pattern), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_trap_set_pattern_ethertype_ethertype */

/*
 * rt_trap set pattern host-mac <MACADDR:mac> */
cparser_result_t
cparser_cmd_rt_trap_set_pattern_host_mac_mac(
    cparser_context_t *context,
    cparser_macaddr_t  *mac_ptr)
{
    rt_trap_pattern_t pattern;
    int32 ret;
    DIAG_UTIL_PARAM_CHK();

    memset(&pattern, 0, sizeof(rt_trap_pattern_t));
    memcpy(pattern.hostMac, mac_ptr->octet, ETHER_ADDR_LEN);
    DIAG_UTIL_ERR_CHK(rt_trap_pattern_set(RT_TRAP_PATTERN_TYPE_HOST_MAC, pattern), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_rt_trap_set_pattern_host_mac_mac */