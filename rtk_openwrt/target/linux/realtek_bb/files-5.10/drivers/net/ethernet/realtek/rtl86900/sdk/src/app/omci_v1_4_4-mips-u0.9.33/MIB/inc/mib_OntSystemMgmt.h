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
 * Purpose : Definition of ME attribute: ont system mgmt (240)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: ont system mgmt (240)
 */

#ifndef __MIB_ONT_SYSTEM_MGMT_TABLE_H__
#define __MIB_ONT_SYSTEM_MGMT_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Table EthUni attribute index */
#define MIB_TABLE_ONT_SYSTEM_MGMT_ATTR_NUM (7)
#define MIB_TABLE_ONT_SYSTEM_MGMT_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_ONT_SYSTEM_MGMT_DISOPTICALTRANSCEIVER_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_ONT_SYSTEM_MGMT_POLLINGINTERVAL_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_ONT_SYSTEM_MGMT_POLLINGCNTROGUEONT_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_ONT_SYSTEM_MGMT_ENABLEALARM_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_ONT_SYSTEM_MGMT_ALARMINGCOUNT_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_ONT_SYSTEM_MGMT_WAITINGTIMEFORNORESPONSE_INDEX ((MIB_ATTR_INDEX)7)

/* Table ont system mgmt attribute len, only string attrubutes have length definition */

typedef enum {
    ONT_SYSTEM_MGMT_ALARM_DISABLED,
    ONT_SYSTEM_MGMT_ALARM_ENABLED
} ont_system_mgmt_attr_enable_alarm_t;

// Table Ont system mgmt entry stucture
typedef struct {
    UINT16 EntityID; // index 1
    UINT16 DisableOpticalTransceiver;
    UINT16 PollingInterval;
    UINT8  PollingCntRogueONT;
    UINT8  EnableAlarm;
    UINT8  AlarmingCount;
    UINT16 WaitingTimeForNoResponse;
} __attribute__((packed)) MIB_TABLE_ONTSYSTEMMGMT_T;


#ifdef __cplusplus
}
#endif

#endif
