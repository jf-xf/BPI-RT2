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


#ifndef __MIB_VOIPVOICECTP_TABLE_H__
#define __MIB_VOIPVOICECTP_TABLE_H__

/* Table VoIPVoiceCTP attribute for STRING type define each entry length */

/* Table VoIPVoiceCTP attribute index */
#define MIB_TABLE_VOIPVOICECTP_ATTR_NUM (5)
#define MIB_TABLE_VOIPVOICECTP_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_VOIPVOICECTP_USERPROTOCOLPOINTER_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_VOIPVOICECTP_PPTPPOINTER_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_VOIPVOICECTP_VOIPMEDIAPROFILEPOINTER_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_VOIPVOICECTP_SIGNALLINGCODE_INDEX ((MIB_ATTR_INDEX)5)

/* Table VoIPVoiceCTP attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
	UINT16   UserProtocolPointer;
	UINT16   PPTPPointer;
	UINT16   VOIPMediaProfilePointer;
	UINT8    SignallingCode;
} __attribute__((packed)) MIB_TABLE_VOIPVOICECTP_T;

#endif /* __MIB_VOIPVOICECTP_TABLE_H__ */
