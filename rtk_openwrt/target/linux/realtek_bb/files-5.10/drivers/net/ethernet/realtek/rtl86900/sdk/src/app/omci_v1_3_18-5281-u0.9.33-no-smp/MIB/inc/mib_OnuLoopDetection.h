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
 * Purpose : Definition of ME attribute: CTC ONU Loop Detection (65528)
 *
 * Feature : The file includes the following modules and sub-modules
 */

#ifndef __MIB_ONU_LOOP_DETECTION_TABLE_H__
#define __MIB_ONU_LOOP_DETECTION_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define MIB_TBL_ONULOOPDETN_ATTR_OPERATORID_LEN (4)
#define MIB_TBL_ONULOOPDETN_ATTR_PORTANDVLANTABLE_LEN (7)

/* Table EthUni attribute index */
#define MIB_TABLE_ONU_LOOP_DETECTIONION_ATTR_NUM (7)
#define MIB_TABLE_ONU_LOOP_DETECTION_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_ONU_LOOP_DETECTION_OPERATORID_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_ONU_LOOP_DETECTION_LOOPDETECTIONMNG_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_ONU_LOOP_DETECTION_LOOPEDPORTDOWN_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_ONU_LOOP_DETECTION_LOOPDETECTIONMSGFREQ_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_ONU_LOOP_DETECTION_LOOPRECOVERYINTVL_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_ONU_LOOP_DETECTION_PORTANDVLANTABLE_INDEX ((MIB_ATTR_INDEX)7)

enum
{
   ONU_LOOP_DETECTIONMNG_INACTIVATED = 0,
   ONU_LOOP_DETECTIONMNG_ACTIVATED,
   END_ONU_LOOP_DETECTIONMNG
};

enum 
{
   LOOPEDPORT_DOWN_NONAUTO,
   LOOPEDPORT_DOWN_AUTO,
   END_LOOPEDPORT_DOWN,
};


typedef struct
omci_portAndVlan_row_entry_s 
{
   UINT8  TblIdx;
   UINT16 MeID;
   UINT16 SVLAN;
   UINT16 CVLAN;
}  __attribute__((aligned(8))) omci_portAndVlan_row_entry_t; 
//
// Table ONU Loop Detection entry stucture
//
typedef struct {
    UINT16 EntityID; // index 1
    UINT8  OperatorID[MIB_TBL_ONULOOPDETN_ATTR_OPERATORID_LEN];
	UINT16 LpDetnMng;
	UINT16 LpPortDown;
	UINT16 LpDetnMsgFreq;
	UINT16 LpRecovIntvl;
	UINT8  PortAndVlanTable[MIB_TBL_ONULOOPDETN_ATTR_PORTANDVLANTABLE_LEN];
	UINT16 curPortAndVlanEntryCnt;
	LIST_HEAD(PortAndVlanHead, PortAndVlanTableEntry_s) head;
} __attribute__((aligned)) MIB_TABLE_ONU_LOOP_DETECTION_T;


#ifdef __cplusplus
}
#endif

#endif
