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


#ifndef __MIB_ENHANCEDSECCTL_TABLE_H__
#define __MIB_ENHANCEDSECCTL_TABLE_H__

/* Table EnhancedSecCtl attribute for STRING type define each entry length */
#define MIB_TABLE_OLTCRYPTOCAP_LEN (16)
#define MIB_TABLE_OLTRAMCHALLTABLE_LEN (17)
#define MIB_TABLE_ONURAMCHALLTABLE_LEN (16)
#define MIB_TABLE_ONUAUTHRSLTABLE_LEN (16)
#define MIB_TABLE_OLTAUTHRSLTABLE_LEN (17)
#define MIB_TABLE_MASTERSESSKEYNAME_LEN (16)
#define MIB_TABLE_BCASTKEY_LEN (18)

#define MIB_TABLE_OLTRAMCHALLTABLE_CONTENT_LEN (16)
#define MIB_TABLE_ONURAMCHALLTABLE_CONTENT_LEN (16)
#define MIB_TABLE_ONUAUTHRSLTABLE_CONTENT_LEN (16)
#define MIB_TABLE_OLTAUTHRSLTABLE_CONTENT_LEN (16)
#define MIB_TABLE_BCASTKEY_CONTENT_LEN (16)

/* Table EnhancedSecCtl attribute index */
#define MIB_TABLE_ENHANCEDSECCTL_ATTR_NUM (13)
#define MIB_TABLE_ENHANCEDSECCTL_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_ENHANCEDSECCTL_OLTCRYPTOCAP_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_ENHANCEDSECCTL_OLTRAMCHALLTABLE_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_ENHANCEDSECCTL_OLTCHALLST_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_ENHANCEDSECCTL_ONUSELCRYPTOCAP_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_ENHANCEDSECCTL_ONURAMCHALLTABLE_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_ENHANCEDSECCTL_ONUAUTHRSLTABLE_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_ENHANCEDSECCTL_OLTAUTHRSLTABLE_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_ENHANCEDSECCTL_OLTRSLST_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_ENHANCEDSECCTL_ONUAUTHST_INDEX ((MIB_ATTR_INDEX)10)
#define MIB_TABLE_ENHANCEDSECCTL_MASTERSESSKEYNAME_INDEX ((MIB_ATTR_INDEX)11)
#define MIB_TABLE_ENHANCEDSECCTL_BCASTKEY_INDEX ((MIB_ATTR_INDEX)12)
#define MIB_TABLE_ENHANCEDSECCTL_EFFECKETLEN_INDEX ((MIB_ATTR_INDEX)13)

/* Misc */
#define  ENABLE_SSL
#define PEER_1_IDENTITY_LEN (8)

typedef enum {
	ENHANCEDSECCTL_AVC_STATE_AUTH_RESULT	 =  0x01,
	ENHANCEDSECCTL_AVC_STATE_AUTH_STATUS	 =  0x02,
}escAvcState_t;

typedef struct omci_olt_ram_chall_entry_s
{
	UINT8 rowNum; 
	UINT8 content[MIB_TABLE_OLTRAMCHALLTABLE_CONTENT_LEN];
} __attribute__((packed)) omci_olt_ram_chall_entry_t;

typedef struct omci_onu_ram_chall_entry_s
{
	UINT8 content[MIB_TABLE_ONURAMCHALLTABLE_CONTENT_LEN];
} __attribute__((packed)) omci_onu_ram_chall_entry_t;

typedef struct omci_onu_auth_rsl_entry_s
{
	UINT8 content[MIB_TABLE_ONUAUTHRSLTABLE_CONTENT_LEN];
} __attribute__((packed)) omci_onu_auth_rsl_entry_t;

typedef struct omci_olt_auth_rsl_entry_s
{
	UINT8 rowNum; 
	UINT8 content[MIB_TABLE_OLTAUTHRSLTABLE_CONTENT_LEN];
} __attribute__((packed)) omci_olt_auth_rsl_entry_t;

typedef struct omci_bcast_key_entry_s
{
	UINT8 rowCtl; 
	UINT8 rowId; 
	UINT8 content[MIB_TABLE_BCASTKEY_CONTENT_LEN];
} __attribute__((packed)) omci_bcast_key_entry_t;

typedef struct oltRamChallTblEntry_s
{
	omci_olt_ram_chall_entry_t tableEntry;
	LIST_ENTRY(oltRamChallTblEntry_s) entries;
} __attribute__((packed)) oltRamChallTblEntry_t;

typedef struct onuRamChallTblEntry_s
{
	omci_onu_ram_chall_entry_t tableEntry;
	LIST_ENTRY(onuRamChallTblEntry_s) entries;
} __attribute__((packed)) onuRamChallTblEntry_t;

typedef struct onuAuthRslTblEntry_s
{
	omci_onu_auth_rsl_entry_t tableEntry;
	LIST_ENTRY(onuAuthRslTblEntry_s) entries;
} __attribute__((packed)) onuAuthRslTblEntry_t;

typedef struct oltAuthRslTblEntry_s
{
	omci_olt_auth_rsl_entry_t tableEntry;
	LIST_ENTRY(oltAuthRslTblEntry_s) entries;
} __attribute__((packed)) oltAuthRslTblEntry_t;

typedef struct bcastKeyTblEntry_s
{
	omci_bcast_key_entry_t tableEntry;
	LIST_ENTRY(bcastKeyTblEntry_s) entries;
} __attribute__((packed)) bcastKeyTblEntry_t;

/* Table EnhancedSecCtl attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
	UINT8    OltCryptoCap[MIB_TABLE_OLTCRYPTOCAP_LEN];
	UINT8    OltRamChallTbl[MIB_TABLE_OLTRAMCHALLTABLE_LEN];
	UINT8    OltChallSt;
	UINT8    OnuSelCryptoCap;
	UINT8    OnuRamChallTbl[MIB_TABLE_ONURAMCHALLTABLE_LEN];
	UINT8    OnuAuthRslTbl[MIB_TABLE_ONUAUTHRSLTABLE_LEN];
	UINT8    OltAuthRslTbl[MIB_TABLE_OLTAUTHRSLTABLE_LEN];
	UINT8    OltRslSt;
	UINT8    OnuAuthSt;
	UINT8    MasterSessKeyName[MIB_TABLE_MASTERSESSKEYNAME_LEN + 1];
	UINT8    BcastKeyTbl[MIB_TABLE_BCASTKEY_LEN];
	UINT16   EffecKetLen;

	UINT32   curOltRamChallCnt;
	UINT32	 curOnuRamChallCnt;
	UINT32   curOnuAuthRslCnt;
	UINT32	 curOltAuthRslCnt;
	UINT32   curBcastKeyCnt;
	LIST_HEAD(oltRamChallTblEntryHead_t, oltRamChallTblEntry_s) oltRamChallHead;
	LIST_HEAD(onuRamChallTblEntryHead_t, onuRamChallTblEntry_s) onuRamChallHead;
	LIST_HEAD(onuAuthRslTblEntryHead_t, onuAuthRslTblEntry_s) onuAuthRslHead;
	LIST_HEAD(oltAuthRslTblEntryHead_t, oltAuthRslTblEntry_s) oltAuthRslHead;
	LIST_HEAD(bcastKeyTblEntryHead_t, bcastKeyTblEntry_s) bcastKeyHead;
} __attribute__((packed)) MIB_TABLE_ENHANCEDSECCTL_T;

#endif /* __MIB_ENHANCEDSECCTL_TABLE_H__ */
