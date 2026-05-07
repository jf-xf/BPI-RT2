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
 * Purpose : Definition of ME attribute: private tellion ont statistics (255)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: private tellion ont statistics (255)
 */

#ifndef __MIB_PRIVATETELLIONONTSTATISTICS_TABLE_H__
#define __MIB_PRIVATETELLIONONTSTATISTICS_TABLE_H__


/* Table private tellion ont statistics attribute index */
#define MIB_TABLE_PRIVATETELLIONONTSTATISTICS_ATTR_NUM (16)
#define MIB_TABLE_PRIVATETELLIONONTSTATISTICS_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_PRIVATETELLIONONTSTATISTICS_RESET_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_PRIVATETELLIONONTSTATISTICS_UPTOTALFRAME_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_PRIVATETELLIONONTSTATISTICS_UPPAUSEFRAME_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_PRIVATETELLIONONTSTATISTICS_UPERRORFRAME_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_PRIVATETELLIONONTSTATISTICS_UPUCFRAME_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_PRIVATETELLIONONTSTATISTICS_UPMCFRAME_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_PRIVATETELLIONONTSTATISTICS_UPBCFRAME_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_PRIVATETELLIONONTSTATISTICS_UPDROPFRAME_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_PRIVATETELLIONONTSTATISTICS_DOWNTOTALFRAME_INDEX ((MIB_ATTR_INDEX)10)
#define MIB_TABLE_PRIVATETELLIONONTSTATISTICS_DOWNPAUSEFRAME_INDEX ((MIB_ATTR_INDEX)11)
#define MIB_TABLE_PRIVATETELLIONONTSTATISTICS_DOWNERRORFRAME_INDEX ((MIB_ATTR_INDEX)12)
#define MIB_TABLE_PRIVATETELLIONONTSTATISTICS_DOWNUCFRAME_INDEX ((MIB_ATTR_INDEX)13)
#define MIB_TABLE_PRIVATETELLIONONTSTATISTICS_DOWNMCFRAME_INDEX ((MIB_ATTR_INDEX)14)
#define MIB_TABLE_PRIVATETELLIONONTSTATISTICS_DOWNBCFRAME_INDEX ((MIB_ATTR_INDEX)15)
#define MIB_TABLE_PRIVATETELLIONONTSTATISTICS_DOWNDROPFRAME_INDEX ((MIB_ATTR_INDEX)16)

/* Table private tellion ont statistics attribute len, only string attrubutes have length definition */

// Table private tellion ont statistics entry stucture
typedef struct {
	UINT16   EntityId;
	UINT8    reset;
	UINT32   total_frame_up;
	UINT32   pause_frame_up;
	UINT32   error_frame_up;
	UINT32   uc_frame_up;
	UINT32   mc_frame_up;
	UINT32   bc_frame_up;
	UINT32   dropped_frame_up;
	UINT32   total_frame_down;
	UINT32   pause_frame_down;
	UINT32   error_frame_down;
	UINT32   uc_frame_down;
	UINT32   mc_frame_down;
	UINT32   bc_frame_down;
	UINT32   dropped_frame_down;
} __attribute__((aligned)) MIB_TABLE_PRIVATETELLIONONTSTATISTICS_T;

#endif
