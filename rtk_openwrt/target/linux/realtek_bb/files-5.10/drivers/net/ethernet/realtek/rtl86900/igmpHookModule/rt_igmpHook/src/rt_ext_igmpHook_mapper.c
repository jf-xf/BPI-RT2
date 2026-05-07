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
 * $Revision: 93636 $
 * $Date: 2018-11-16 12:52:37 +0800 (Fri, 16 Nov 2018) $
 *
 * Purpose : Definition of Rtdrv Extension Mapper
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Mapper register
 *           (2) Mapper get
 *
 */


/*
 * Include Files
 */

#include <linux/kernel.h>
#include <linux/string.h>

#include <rt_ext_igmpHook_mapper.h>


/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

static rt_ext_igmpHook_mapper_t rt_ext_igmpHook_mapper = {0};


void rt_ext_igmpHook_mapper_register(rt_ext_igmpHook_mapper_t *pMapper)
{
    memcpy(&rt_ext_igmpHook_mapper,pMapper,sizeof(rt_ext_igmpHook_mapper_t));
}

EXPORT_SYMBOL(rt_ext_igmpHook_mapper_register);


rt_ext_igmpHook_mapper_t* rt_ext_igmpHook_mapper_get(void)
{
    return &rt_ext_igmpHook_mapper;
} /* end of rt_ext_igmpHook_mapper_get */

