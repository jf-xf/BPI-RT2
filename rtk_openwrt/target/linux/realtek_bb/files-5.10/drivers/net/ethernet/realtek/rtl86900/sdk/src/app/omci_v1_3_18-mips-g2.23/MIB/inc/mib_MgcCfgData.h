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
 * Purpose : Definition of ME handler: MGC config data (155)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME handler: MGC config data (155)
 */


#ifndef __MIB_MGCCFGDATA_TABLE_H__
#define __MIB_MGCCFGDATA_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Table MGC config data  attribute for STRING type define each entry length */

/* Table MGC config data  attribute index */
#define MIB_TABLE_MGCCFGDATA_ATTR_NUM (12)
#define MIB_TABLE_MGCCFGDATA_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_MGCCFGDATA_PRIMARYMGC_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_MGCCFGDATA_SECONDARYMGC_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_MGCCFGDATA_TCPUDPPTR_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_MGCCFGDATA_VERSION_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_MGCCFGDATA_MAXFMT_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_MGCCFGDATA_MAXRETRYTIME_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_MGCCFGDATA_MAXRETRYNUM_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_MGCCFGDATA_SVCCHGDELAY_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_MGCCFGDATA_TERMIDBASE_INDEX ((MIB_ATTR_INDEX)10)
#define MIB_TABLE_MGCCFGDATA_SOFTSW_INDEX ((MIB_ATTR_INDEX)11)
#define MIB_TABLE_MGCCFGDATA_MSGIDPTR_INDEX ((MIB_ATTR_INDEX)12)

#define MIB_TABLE_MGCCFGDATA_TERMINATION_ID_BASE_LEN        (25)

typedef enum {
    MGC_CFG_DATA_MSG_FMT_TEXT_LONG    = 0,
    MGC_CFG_DATA_MSG_FMT_TEXT_SHORT   = 1,
    MGC_CFG_DATA_MSG_FMT_BINARY       = 2
} mgc_cfg_data_attr_msg_fmt_t;

/* Table MGC config data attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
	UINT16   PrimaryMGC;
	UINT16   SecondaryMGC;
	UINT16   TcpUdpPtr;
	UINT8    Version;
	UINT8    MsgFmt;
	UINT16   MaxRetryTime;
	UINT16   MaxRetryAttempts;
	UINT16   SvcChangeDelay;
	CHAR     TerminationIdBase[MIB_TABLE_MGCCFGDATA_TERMINATION_ID_BASE_LEN];
	UINT32   Softswitch;
	UINT16   MsgIdPtr;
} __attribute__((aligned)) MIB_TABLE_MGCCFGDATA_T;

#endif /* __MIB_MGCCFGDATA_TABLE_H__ */
