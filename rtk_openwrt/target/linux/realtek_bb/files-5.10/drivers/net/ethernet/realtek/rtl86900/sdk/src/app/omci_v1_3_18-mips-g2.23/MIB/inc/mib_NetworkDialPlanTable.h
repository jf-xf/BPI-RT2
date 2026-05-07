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
 */


#ifndef __MIB_NETWORKDIALPLANTABLE_TABLE_H__
#define __MIB_NETWORKDIALPLANTABLE_TABLE_H__

/* Table NetworkDialPlanTable attribute for STRING type define each entry length */
#define MIB_TABLE_DIALPLANTABLE_LEN         (30)
#define MIB_TABLE_DIALPLANTABLE_TOKEN_LEN   (28)
/* Table NetworkDialPlanTable attribute index */
#define MIB_TABLE_NETWORKDIALPLANTABLE_ATTR_NUM (7)
#define MIB_TABLE_NETWORKDIALPLANTABLE_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_NETWORKDIALPLANTABLE_DIALPLANNUMBER_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_NETWORKDIALPLANTABLE_DIALPLANTABLEMAXSIZE_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_NETWORKDIALPLANTABLE_CRITICALDIALTIMEOUT_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_NETWORKDIALPLANTABLE_PARTIALDIALTIMEOUT_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_NETWORKDIALPLANTABLE_DIALPLANFORMAT_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_NETWORKDIALPLANTABLE_DIALPLANTABLE_INDEX ((MIB_ATTR_INDEX)7)


typedef enum {
    NETWORK_DIAL_PLAN_TBL_DIAL_PLAN_FMT_NOT_DEFINED     = 0,
    NETWORK_DIAL_PLAN_TBL_DIAL_PLAN_FMT_H248            = 1,
    NETWORK_DIAL_PLAN_TBL_DIAL_PLAN_FMT_NCS             = 2,
    NETWORK_DIAL_PLAN_TBL_DIAL_PLAN_FMT_VENDOR_SPECIFIC = 3,
} network_dial_plan_tbl_dial_plan_fmt_t;

typedef enum {
    NETWORK_DIAL_PLAN_TBL_DIAL_PLAN_ENTRY_ACT_DEL       = 0,
    NETWORK_DIAL_PLAN_TBL_DIAL_PLAN_ENTRY_ACT_ADD       = 1,
} network_dial_plan_tbl_dial_plan_entry_act_t;

typedef struct omci_dial_plan_row_entry_s
{
    UINT8   dpId;
    UINT8   act;
    UINT8   dpToken[MIB_TABLE_DIALPLANTABLE_TOKEN_LEN];
} __attribute__((packed)) omci_dial_plan_row_entry_t;

typedef struct dpTableEntry_s{

	omci_dial_plan_row_entry_t tableEntry;
	LIST_ENTRY(dpTableEntry_s) entries;

}dpTableEntry_t;

/* Table NetworkDialPlanTable attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
	UINT16   DialPlanNumber;
	UINT16   DialPlanTableMaxSize;
	UINT16   CriticalDialTimeout;
	UINT16   PartialDialTimeout;
	UINT8    DialPlanFormat;
	UINT8    DialPlanTable[MIB_TABLE_DIALPLANTABLE_LEN];
	LIST_HEAD(dpHead, dpTableEntry_s) head;
} __attribute__((aligned)) MIB_TABLE_NETWORKDIALPLANTABLE_T;

#endif /* __MIB_NETWORKDIALPLANTABLE_TABLE_H__ */
