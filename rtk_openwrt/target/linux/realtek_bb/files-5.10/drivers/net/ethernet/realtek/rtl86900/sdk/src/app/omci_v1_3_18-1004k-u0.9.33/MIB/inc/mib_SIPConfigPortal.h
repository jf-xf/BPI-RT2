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


#ifndef __MIB_SIPCONFIGPORTAL_TABLE_H__
#define __MIB_SIPCONFIGPORTAL_TABLE_H__

/* Table SIPConfigPortal attribute for STRING type define each entry length */
#define MIB_TABLE_CONFIGURATIONTEXTTABLE_LEN (24)

/* Table SIPConfigPortal attribute index */
#define MIB_TABLE_SIPCONFIGPORTAL_ATTR_NUM (2)
#define MIB_TABLE_SIPCONFIGPORTAL_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_SIPCONFIGPORTAL_CONFIGURATIONTEXTTABLE_INDEX ((MIB_ATTR_INDEX)2)

/* Table SIPConfigPortal attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
	UINT8    ConfigurationTextTable[MIB_TABLE_CONFIGURATIONTEXTTABLE_LEN];
} __attribute__((aligned)) MIB_TABLE_SIPCONFIGPORTAL_T;

#endif /* __MIB_SIPCONFIGPORTAL_TABLE_H__ */
