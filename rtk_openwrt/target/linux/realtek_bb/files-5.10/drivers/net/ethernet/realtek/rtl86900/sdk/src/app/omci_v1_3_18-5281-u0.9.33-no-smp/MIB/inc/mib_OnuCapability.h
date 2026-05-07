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
 */

#ifndef __MIB_ONUCAPABILITY_TABLE_H__
#define __MIB_ONUCAPABILITY_TABLE_H__

/* Table onu capability attribute len, only string attrubutes have length definition */
#define MIB_TABLE_ONUCAPABILITY_OPERATIONID_LEN (4)

/* Table onu capability attribute index */
#define MIB_TABLE_ONUCAPABILITY_ATTR_NUM (6)
#define MIB_TABLE_ONUCAPABILITY_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_ONUCAPABILITY_OPERATIONID_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_ONUCAPABILITY_CTCSPEC_VERSION_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_ONUCAPABILITY_ONU_TYPE_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_ONUCAPABILITY_TXPWR_SUPPLY_CTRL_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_ONUCAPABILITY_CUSTOMIZED_ONU_TYPE_INDEX ((MIB_ATTR_INDEX)6)

typedef enum {
    ONUCAPABILITY_CTC_SPEC_VERSION,
    ONUCAPABILITY_RSD_SPEC_VERSION,
} onucapability_attr_spec_verion_t;

typedef enum {
    ONUCAPABILITY_SFU_ONU_TYPE,
    ONUCAPABILITY_HGU_ONU_TYPE,
    ONUCAPABILITY_SBU_ONU_TYPE,
    ONUCAPABILITY_CBU_ONU_TYPE,
    ONUCAPABILITY_MDU_ONU_TYPE,
    ONUCAPABILITY_MTU_ONU_TYPE,
    ONUCAPABILITY_RSD_ONU_TYPE,
} onucapability_attr_onu_type_t;

typedef enum {
    ONUCAPABILITY_NOT_SUPPORT_PWR_CTRL,
    ONUCAPABILITY_SUPPORT_TX_PWR_CTRL,
    ONUCAPABILITY_SUPPORT_TXRX_PWR_CTRL,
    ONUCAPABILITY_RSD_PWR_CTRL,
} onucapability_attr_onu_pwr_ctrl_t;

/* Table onu capability attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
	CHAR     OperationId[MIB_TABLE_ONUCAPABILITY_OPERATIONID_LEN+1];
	UINT8    CtcSpecVer;
	UINT8    OnuType;
	UINT8    TxPwrSupplyCtrl;
    UINT8    CustomizedOnuType;
} MIB_TABLE_ONUCAPABILITY_T;

#endif /* __MIB_ONUCAPABILITY_TABLE_H__ */
