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
 * Purpose : Definition of ME attribute: ONT self loop detect (244)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: ont self loop detect (244)
 */

#ifndef __MIB_ONTSELFLOOPDETECT_TABLE_H__
#define __MIB_ONTSELFLOOPDETECT_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table ont self loop detect attribute index */
#define MIB_TABLE_ONTSELFLOOPDETECT_ATTR_NUM (9)
#define MIB_TABLE_ONTSELFLOOPDETECT_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_ONTSELFLOOPDETECT_ENABLELOOPDETECT_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_ONTSELFLOOPDETECT_DETECTACTION_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_ONTSELFLOOPDETECT_SENDINGPERIOD_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_ONTSELFLOOPDETECT_BLOCKTIMER_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_ONTSELFLOOPDETECT_ONUUNIDETECTSTATE_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_ONTSELFLOOPDETECT_ONUUNIBLOCKSTATE_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_ONTSELFLOOPDETECT_UNBLOCKREASON_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_ONTSELFLOOPDETECT_UNBLOCKONUUNI_INDEX ((MIB_ATTR_INDEX)9)


/* Table ont self loop detect attribute len, only string attrubutes have length definition */
typedef enum {
    ONT_SELF_LOOP_DETECT_DISABLED,
    ONT_SELF_LOOP_DETECT_ENABLED
} ont_self_loop_detect_attr_enable_loop_detect_t;

typedef enum {
    ONT_SELF_LOOP_DETECT_ONLY_DETECT_ACTION,
    ONT_SELF_LOOP_DETECT_BLOCK_UNIPORT_ACTION
} ont_self_loop_detect_attr_detect_action_t;

typedef enum {
    ONT_SELF_LOOP_DETECT_ONU_UNI_UNDETECTED_STATE,
    ONT_SELF_LOOP_DETECT_ONU_UNI_DETECTED_STATE
} ont_self_loop_detect_attr_onu_uni_detect_state_t;

typedef enum {
    ONT_SELF_LOOP_DETECT_ONU_UNI_UNBLOCKED_STATE,
    ONT_SELF_LOOP_DETECT_ONU_UNI_BLOCKED_STATE
} ont_self_loop_detect_attr_onu_uni_block_state_t;

typedef enum {
    ONT_SELF_LOOP_DETECT_UNBLOCK_BY_INIT_REASON,
    ONT_SELF_LOOP_DETECT_UNBLOCK_BY_CMD_REASON,
    ONT_SELF_LOOP_DETECT_UNBLOCK_BY_TIMER_EXPIRE_REASON,
    ONT_SELF_LOOP_DETECT_UNBLOCK_BY_DISABLE_REASON
} ont_self_loop_detect_attr_unblock_reason_t;

// Table ont self loop detect entry stucture
typedef struct {
    UINT16 EntityID; // index 1
    UINT8  EnableLoopDetect;
    UINT8  DetectAction;
    UINT16 SendingPeriod;
    UINT16 BlockTimer;
    UINT8  OnuUniDetectState;
    UINT8  OnuUniBlockState;
    UINT8  UnBlockReason;
    UINT8  UnBlockOnuUni;
} __attribute__((packed)) MIB_TABLE_ONTSELFLOOPDETECT_T;


#ifdef __cplusplus
}
#endif

#endif
