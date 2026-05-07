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


#ifndef __MIB_OMCI_TABLE_H__
#define __MIB_OMCI_TABLE_H__

/* Table Omci attribute for STRING type define each entry length */

#define MIB_TABLE_METYPETBL_LEN 2
#define MIB_TABLE_MSGTYPETBL_LEN 1

/* Table Omci attribute index */
#define MIB_TABLE_OMCI_ATTR_NUM (3)
#define MIB_TABLE_OMCI_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_OMCI_METYPETBL_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_OMCI_MSGTYPETBL_INDEX ((MIB_ATTR_INDEX)3)

/* Table Omci attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16	meTypeTbl;
} __attribute__((packed)) omci_meType_tbl_t;

typedef struct omci_meType_tbl_entry_s {
    omci_meType_tbl_t					tableEntry;
    LIST_ENTRY(omci_meType_tbl_entry_s)	entries;
} __attribute__((aligned)) omci_meType_tbl_entry_t;

typedef struct {
	UINT16	msgTypeTbl;
} __attribute__((packed)) omci_msgType_tbl_t;

typedef struct omci_msgType_tbl_entry_s {
    omci_msgType_tbl_t					tableEntry;
    LIST_ENTRY(omci_msgType_tbl_entry_s)	entries;
} __attribute__((aligned)) omci_msgType_tbl_entry_t;

typedef struct {
	UINT16   EntityId;
	UINT8    MeTypeTbl[MIB_TABLE_METYPETBL_LEN];
	UINT8    MsgTypeTbl[MIB_TABLE_MSGTYPETBL_LEN];
	UINT32	 MeTypeTbl_size;
	UINT32	 MsgTypeTbl_size;
	LIST_HEAD(meTypeHead,omci_meType_tbl_entry_s) meType_head;
	LIST_HEAD(msgTypeHead,omci_msgType_tbl_entry_s) msgType_head;
} __attribute__((packed)) MIB_TABLE_OMCI_T;

#endif /* __MIB_OMCI_TABLE_H__ */
