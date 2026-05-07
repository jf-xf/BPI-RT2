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

#ifndef __MIB_OLTLOCATIONCFGDATA_TABLE_H__
#define __MIB_OLTLOCATIONCFGDATA_TABLE_H__

/* Table OLTLocationConfigData attribute len, only string attrubutes have length definition */


/* Table OLTLocationConfigData attribute index */
#define MIB_TABLE_OLTLOCATIONCFGDATA_ATTR_NUM (9)
#define MIB_TABLE_OLTLOCATIONCFGDATA_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_OLTLOCATIONCFGDATA_LONGITUDE_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_OLTLOCATIONCFGDATA_LATITUDE_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_OLTLOCATIONCFGDATA_ELEVATION_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_OLTLOCATIONCFGDATA_HORIZONTALERROR_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_OLTLOCATIONCFGDATA_ALTITUDEERROR_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_OLTLOCATIONCFGDATA_AREACODE_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_OLTLOCATIONCFGDATA_TIMESTAMP_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_OLTLOCATIONCFGDATA_GISDIGEST_INDEX ((MIB_ATTR_INDEX)9)

/* Table OLTLocationConfigData attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
	INT32    Longitude;
	INT32    Latitude;
	INT16    Elevation;
	UINT16   Horizontalerror;
	UINT16 	 Altitudeerror;
	UINT64   Areacode;
	UINT32	 TimeStamp;
	UINT64   GISDigest;
} __attribute__((packed)) MIB_TABLE_OLTLOCATIONCFGDATA_T;

#endif /* __MIB_OLTLOCATIONCFGDATA_TABLE_H__ */

