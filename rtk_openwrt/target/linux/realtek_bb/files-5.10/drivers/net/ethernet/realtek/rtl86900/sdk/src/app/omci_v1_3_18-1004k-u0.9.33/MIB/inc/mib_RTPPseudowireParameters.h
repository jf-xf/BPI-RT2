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


#ifndef __MIB_RTPPSEUDOWIREPARAMETERS_TABLE_H__
#define __MIB_RTPPSEUDOWIREPARAMETERS_TABLE_H__

/* Table RTPPseudowireParameters attribute for STRING type define each entry length */

/* Table RTPPseudowireParameters attribute index */
#define MIB_TABLE_RTPPSEUDOWIREPARAMETERS_ATTR_NUM (7)
#define MIB_TABLE_RTPPSEUDOWIREPARAMETERS_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_RTPPSEUDOWIREPARAMETERS_CLOCKREFERENCE_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_RTPPSEUDOWIREPARAMETERS_RTPTIMESTAMPMODE_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_RTPPSEUDOWIREPARAMETERS_PTYPE_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_RTPPSEUDOWIREPARAMETERS_SSRC_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_RTPPSEUDOWIREPARAMETERS_EXPECTEDPTYPE_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_RTPPSEUDOWIREPARAMETERS_EXPECTEDSSRC_INDEX ((MIB_ATTR_INDEX)7)

/* Table RTPPseudowireParameters attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
	UINT16   ClockReference;
	UINT8    RTPTimeStampMode;
	UINT16   PTYPE;
	UINT32   SSRC;
	UINT16   ExpectedPTYPE;
	UINT32   ExpectedSSRC;
} __attribute__((aligned)) MIB_TABLE_RTPPSEUDOWIREPARAMETERS_T;

#endif /* __MIB_RTPPSEUDOWIREPARAMETERS_TABLE_H__ */
