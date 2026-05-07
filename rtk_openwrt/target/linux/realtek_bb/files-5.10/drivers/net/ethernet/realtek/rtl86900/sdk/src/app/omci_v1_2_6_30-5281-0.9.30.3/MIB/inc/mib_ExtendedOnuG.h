/*
 * Copyright (C) 2014 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : Definition of ME attribute: Extended ONU-G (65408)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: Extended ONU-G (65408)
 */

#ifndef __MIB_EXTENDED_ONU_G_H__
#define __MIB_EXTENDED_ONU_G_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_EXTENDED_ONU_G_ATTR_NUM (2)
#define MIB_TABLE_EXTENDED_ONU_G_ENTITY_ID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_EXTENDED_ONU_G_RESET_DEFAULT_INDEX ((MIB_ATTR_INDEX)2)


typedef struct {
	UINT16	EntityId;
	UINT8	ResetDefault;
} __attribute__((aligned)) MIB_TABLE_EXTENDED_ONU_G_T;


#ifdef __cplusplus
}
#endif

#endif
