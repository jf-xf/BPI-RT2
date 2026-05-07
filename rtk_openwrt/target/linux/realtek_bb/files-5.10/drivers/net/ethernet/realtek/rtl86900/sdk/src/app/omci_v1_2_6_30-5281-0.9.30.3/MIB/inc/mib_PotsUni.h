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
 * Purpose : Definition of ME handler: PPTP POTS UNI (53)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME handler: PPTP POTS UNI (53)
 */


#ifndef __MIB_POTSUNI_TABLE_H__
#define __MIB_POTSUNI_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Table PotsUni attribute for STRING type define each entry length */

/* Table PotsUni attribute index */
#define MIB_TABLE_POTSUNI_ATTR_NUM (13)
#define MIB_TABLE_POTSUNI_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_POTSUNI_ADMINSTATE_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_POTSUNI_IWTPPTR_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_POTSUNI_ARC_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_POTSUNI_ARCINTERVAL_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_POTSUNI_IMPEDANCE_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_POTSUNI_TXPATH_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_POTSUNI_RXGAIN_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_POTSUNI_TXGAIN_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_POTSUNI_OPSTATE_INDEX ((MIB_ATTR_INDEX)10)
#define MIB_TABLE_POTSUNI_HOOKSTATE_INDEX ((MIB_ATTR_INDEX)11)
#define MIB_TABLE_POTSUNI_POTSHOLDOVERTIME_INDEX ((MIB_ATTR_INDEX)12)
#define MIB_TABLE_POTSUNI_NOMINALFEEDVOLTAGE_INDEX ((MIB_ATTR_INDEX)13)

typedef enum {
    POTS_UNI_TEST_TYPE_ALL_MLT_TEST             = 0,
    POTS_UNI_TEST_TYPE_HAZARDOUS_POTENTIAL_TEST = 1,
    POTS_UNI_TEST_TYPE_FOREIGN_EMF_TEST         = 2,
    POTS_UNI_TEST_TYPE_RESISTIVE_FAULTS_TEST    = 3,
    POTS_UNI_TEST_TYPE_RECEIVER_OFF_HOOOK_TEST  = 4,
    POTS_UNI_TEST_TYPE_RINGER_TEST              = 5,
    POTS_UNI_TEST_TYPE_NT1_DC_SIGNATURE_TEST    = 6,
    POTS_UNI_TEST_TYPE_SELF_TEST                = 7
} pots_uni_test_type_t;

typedef enum {
    POTS_UNI_TEST_RESULT_3BITS_FLAGS_TEST_NOT_RUN       = 0,
    POTS_UNI_TEST_RESULT_3BITS_FLAGS_FAIL_NOT_REPORTED  = 2,
    POTS_UNI_TEST_RESULT_3BITS_FLAGS_FAIL_REPORTED      = 3,
    POTS_UNI_TEST_RESULT_3BITS_FLAGS_PASS_NOT_REPORTED  = 6,
    POTS_UNI_TEST_RESULT_3BITS_FLAGS_PASS_REPORTED      = 7
} pots_uni_test_result_3bits_flags_t;

typedef enum {
    POTS_UNI_TEST_MODE_NORMAL = 0,
    POTS_UNI_TEST_MODE_FORCED = 1
} pots_uni_test_mode_t;

typedef enum {
    POTS_UNI_TEST_RESULT_FAILED             = 0,
    POTS_UNI_TEST_RESULT_PASSED             = 1,
    POTS_UNI_TEST_RESULT_NOT_COMPLETED      = 2
} pots_uni_test_result_t;

typedef enum {
    POTS_IMPEDANCE_600OHMS							= 0,
    POTS_IMPEDANCE_900OHMS							= 1,
    POTS_IMPEDANCE_C1_150NF_R1_750OHM_R2_270OHM		= 2,
    POTS_IMPEDANCE_C1_115NF_R1_820OHM_R2_220OHM		= 3,
    POTS_IMPEDANCE_C1_230NF_R1_1050OHM_R2_320OHM	= 4
} pots_uni_attr_impedance_t;

typedef enum {
    POTS_HOOK_STATE_ON_HOOK		= 0,
    POTS_HOOK_STATE_OFF_HOOK	= 1
} pots_uni_attr_hook_state_t;

/* Table PotsUni attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
	UINT8    AdminState;
	UINT16   IWTPPtr;
	UINT8    ARC;
	UINT8    ARCInterval;
	UINT8    Impedance;
	UINT8    TxPath;
	UINT8    RxGain;
	UINT8    TxGain;
	UINT8    OpState;
	UINT8    HookState;
	UINT16   POTSHoldoverTime;
	UINT8    NominalFeedVoltage;
} __attribute__((aligned)) MIB_TABLE_POTSUNI_T;

#endif /* __MIB_POTSUNI_TABLE_H__ */
