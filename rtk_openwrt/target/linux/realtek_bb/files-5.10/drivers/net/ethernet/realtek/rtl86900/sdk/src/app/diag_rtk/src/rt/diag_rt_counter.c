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
 * Purpose : Definition those Counter command and APIs in the SDK diagnostic shell.
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
#include <rtk/rt/rt_stat.h>

/*
 * rt_mib init
 */
cparser_result_t
cparser_cmd_rt_mib_init(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rt_stat_init(), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_mib_init */

/*
 * rt_mib reset counter port ( <PORT_LIST:ports> | all )
 */
cparser_result_t
cparser_cmd_rt_mib_reset_counter_port_ports_all(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rt_port_t port;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(rt_stat_port_reset(port), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_mib_reset_counter_port_ports_all */

int32
dump_all_counter(rt_port_t port,rt_enable_t nonZero)
{
    int32 ret = RT_ERR_FAILED;
    rt_stat_port_cntr_t portCntrs;
    rt_stat_port_type_t type;
    uint64 cntr;

    if(nonZero == DISABLED)
    {
        for (type = 0; type < RT_MIB_PORT_CNTR_END; type++)
        {
            cntr = 0;
            if ((ret = rt_stat_port_get(port,type,&cntr)) != RT_ERR_OK)
            {
                continue;
            }
            
            diag_util_printf("%-35s: ",diagStr_mibName[type]);
            diag_util_printf("%25llu\n", cntr);

        }
    }
    else
    {
        for (type = 0; type < RT_MIB_PORT_CNTR_END; type++)
        {
            cntr = 0;
            if ((ret = rt_stat_port_get(port,type,&cntr)) != RT_ERR_OK)
            {
                continue;
            }
            if(cntr == 0)
                continue;

            diag_util_printf("%-35s: ",diagStr_mibName[type]);
            diag_util_printf("%25llu\n", cntr);
        }
    }
    return RT_ERR_OK;
}

/*
 * rt_mib dump counter port ( <PORT_LIST:ports> | all ) { nonZero } */
cparser_result_t
cparser_cmd_rt_mib_dump_counter_port_ports_all_nonZero(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rt_port_t port;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        diag_util_mprintf("Port: %d\n", port);
        if(5 == TOKEN_NUM())
        {
            dump_all_counter(port,DISABLED);
        }
        else
        {
            dump_all_counter(port,ENABLED);
        }
        diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_mib_dump_counter_port_ports_all_nonzero */
