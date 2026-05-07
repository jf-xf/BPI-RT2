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


#ifndef __MIB_MGCCONFIGPORTAL_TABLE_H__
#define __MIB_MGCCONFIGPORTAL_TABLE_H__

/* Table MGCConfigPortal attribute for STRING type define each entry length */
#define MIB_TABLE_MGCCONFIGPORTAL_CFGTBL_ENTRY_LEN (24)

/* Table MGCConfigPortal attribute index */
#define MIB_TABLE_MGCCONFIGPORTAL_ATTR_NUM (2)
#define MIB_TABLE_MGCCONFIGPORTAL_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_MGCCONFIGPORTAL_CONFIGURATIONTEXTTABLE_INDEX ((MIB_ATTR_INDEX)2)

/* Table MGCConfigPortal attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
	UINT8    ConfigurationTextTable[MIB_TABLE_MGCCONFIGPORTAL_CFGTBL_ENTRY_LEN];
} __attribute__((aligned)) MIB_TABLE_MGCCONFIGPORTAL_T;

#endif /* __MIB_MGCCONFIGPORTAL_TABLE_H__ */
