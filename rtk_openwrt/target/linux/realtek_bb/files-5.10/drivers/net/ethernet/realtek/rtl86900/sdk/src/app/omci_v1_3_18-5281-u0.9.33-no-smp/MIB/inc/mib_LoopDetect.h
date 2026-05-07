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
 * Purpose : Definition of ME attribute: PPTP Ethernet UNI (11)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: PPTP Ethernet UNI (11)
 */

#ifndef __MIB_LOOP_DETECT_TABLE_H__
#define __MIB_LOOP_DETECT_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table EthUni attribute index */
#define MIB_TABLE_LOOP_DETECT_ATTR_NUM (5)
#define MIB_TABLE_LOOP_DETECT_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_LOOP_DETECT_ADMINSTATE_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_LOOP_DETECT_ARC_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_LOOP_DETECT_ARCINTERVAL_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_LOOP_DETECT_ETHLOOPDETECTCFG_INDEX ((MIB_ATTR_INDEX)5)
// Table Loop Detect entry stucture
typedef struct {
    UINT16 EntityID; // index 1
    UINT8  AdminState;
    UINT8  ARC;
    UINT8  ARCInterval;
    UINT8  EthLoopDetectCfg;
} __attribute__((aligned)) MIB_TABLE_LOOP_DETECT_T;


#ifdef __cplusplus
}
#endif

#endif
