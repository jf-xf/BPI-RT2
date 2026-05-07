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


#ifndef __MIB_RTPPROFILEDATA_TABLE_H__
#define __MIB_RTPPROFILEDATA_TABLE_H__

/* Table RTPProfileData attribute for STRING type define each entry length */

/* Table RTPProfileData attribute index */
#define MIB_TABLE_RTPPROFILEDATA_ATTR_NUM (9)
#define MIB_TABLE_RTPPROFILEDATA_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_RTPPROFILEDATA_LOCALPORTMIN_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_RTPPROFILEDATA_LOCALPORTMAX_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_RTPPROFILEDATA_DSCPMARK_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_RTPPROFILEDATA_PIGGYBACKEVENTS_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_RTPPROFILEDATA_TONEEVENTS_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_RTPPROFILEDATA_DTMFEVENTS_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_RTPPROFILEDATA_CASEVENTS_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_RTPPROFILEDATA_IPHOSTCONFIGPOINTER_INDEX ((MIB_ATTR_INDEX)9)

/* Table RTPProfileData attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
	UINT16   LocalPortMin;
	UINT16   LocalPortMax;
	UINT8    DSCPMark;
	UINT8    PiggybackEvents;
	UINT8    ToneEvents;
	UINT8    DTMFEvents;
	UINT8    CASEvents;
	UINT16   IPHostConfigPointer;
} __attribute__((aligned)) MIB_TABLE_RTPPROFILEDATA_T;

#endif /* __MIB_RTPPROFILEDATA_TABLE_H__ */
