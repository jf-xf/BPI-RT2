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
 * Purpose : Definition of ME attribute: RTP PMHD (144)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: RTP PMHD (144)
 */


#ifndef __MIB_RTPPERFORMANCEMONITORINGHISTORYDATA_TABLE_H__
#define __MIB_RTPPERFORMANCEMONITORINGHISTORYDATA_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Table RTPPerformanceMonitoringHistoryData attribute for STRING type define each entry length */

/* Table RTPPerformanceMonitoringHistoryData attribute index */
#define MIB_TABLE_RTPPERFORMANCEMONITORINGHISTORYDATA_ATTR_NUM (9)
#define MIB_TABLE_RTPPERFORMANCEMONITORINGHISTORYDATA_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_RTPPERFORMANCEMONITORINGHISTORYDATA_INTERVALENDTIME_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_RTPPERFORMANCEMONITORINGHISTORYDATA_THRESHOLDDATA12ID_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_RTPPERFORMANCEMONITORINGHISTORYDATA_RTPERRORS_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_RTPPERFORMANCEMONITORINGHISTORYDATA_PACKETLOSS_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_RTPPERFORMANCEMONITORINGHISTORYDATA_MAXIMUMJITTER_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_RTPPERFORMANCEMONITORINGHISTORYDATA_MAXIMUMTIMEBETWEENRTCPPACKETS_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_RTPPERFORMANCEMONITORINGHISTORYDATA_BUFFERUNDERFLOWS_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_RTPPERFORMANCEMONITORINGHISTORYDATA_BUFFEROVERFLOWS_INDEX ((MIB_ATTR_INDEX)9)

/* Table RTPPerformanceMonitoringHistoryData attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
	UINT8    IntervalEndTime;
	UINT16   ThresholdData12Id;
	UINT32   RTPErrors;
	UINT32   PacketLoss;
	UINT32   MaximumJitter;
	UINT32   MaximumTimeBetweenRTCPPackets;
	UINT32   BufferUnderflows;
	UINT32   BufferOverflows;
} __attribute__((aligned)) MIB_TABLE_RTPPERFORMANCEMONITORINGHISTORYDATA_T;

#ifdef __cplusplus
}
#endif

#endif /* __MIB_RTPPERFORMANCEMONITORINGHISTORYDATA_TABLE_H__ */
