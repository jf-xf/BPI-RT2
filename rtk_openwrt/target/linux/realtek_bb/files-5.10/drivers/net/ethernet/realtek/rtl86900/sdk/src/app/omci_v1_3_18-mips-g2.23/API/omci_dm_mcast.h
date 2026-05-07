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
 * Purpose : Definition of Dual Management multicast shared define
 */

#ifndef __OMCI_DM_MCAST_H__
#define __OMCI_DM_MCAST_H__

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************/
/* Include Files                                                */
/****************************************************************/


/****************************************************************/
/* Symbol Definition                                            */
/****************************************************************/
typedef enum omci_dm_mcast_cmd_e
{
    OMCI_DM_MCAST_CMD_RESET_ALL,
    OMCI_DM_MCAST_CMD_PROFILE_PER_PORT_ADD,
    OMCI_DM_MCAST_CMD_PROFILE_PER_PORT_DEL,
    OMCI_DM_MCAST_CMD_PROFILE_ADD,
    OMCI_DM_MCAST_CMD_PROFILE_DEL,
    OMCI_DM_MCAST_CMD_SNOOP_ENABLE_SET,

    /*ME-309*/   
    OMCI_DM_MCAST_CMD_IGMP_VERSION_SET,
    OMCI_DM_MCAST_CMD_IGMP_FUNCTION_SET,    
    OMCI_DM_MCAST_CMD_FAST_LEAVE_SET,
    OMCI_DM_MCAST_CMD_US_IGMP_RATE_SET,
    OMCI_DM_MCAST_CMD_DYNAMIC_ACL_SET,
    OMCI_DM_MCAST_CMD_DYNAMIC_ACL_DEL,
    OMCI_DM_MCAST_CMD_STATIC_ACL_SET,
    OMCI_DM_MCAST_CMD_STATIC_ACL_DEL,
    OMCI_DM_MCAST_CMD_ROBUSTNESS_SET,
    OMCI_DM_MCAST_CMD_QUERIER_IPADDR_SET,
    OMCI_DM_MCAST_CMD_QUERY_INTERVAL_SET,
    OMCI_DM_MCAST_CMD_QUERY_MAX_RESPONSE_TIME_SET,
    OMCI_DM_MCAST_CMD_LAST_MBR_QUERY_INTERVAL_SET,
    OMCI_DM_MCAST_CMD_UNAUTHORIZED_JOIN_BEHAVIOR_SET,

    /*ME-310*/
    OMCI_DM_MCAST_CMD_MAX_GRP_NUM_SET,
    OMCI_DM_MCAST_CMD_MAX_BW_SET,
    OMCI_DM_MCAST_CMD_BW_ENFORCE_SET,

    /*ME-311*/
    OMCI_DM_MCAST_CMD_CURRENT_BW_GET,
    OMCI_DM_MCAST_CMD_JOIN_MSG_COUNT_GET,
    OMCI_DM_MCAST_CMD_BW_EXCEEDED_COUNT_GET,
    OMCI_DM_MCAST_CMD_IPV4_ACTIVE_GROUP_GET,
    OMCI_DM_MCAST_CMD_END
}omci_dm_mcast_cmd_t;



typedef enum omci_dm_mcast_igmpMode_e
{
	MCAST_IGMP_MODE_SNOOPING = 0,
	MCAST_IGMP_MODE_PROXY,
	MCAST_IGMP_MODE_SPR, /*snopping with proxy reporting*/
	MCAST_IGMP_MODE_END
} omci_dm_mcast_igmpMode_t;

typedef enum omci_dm_mcast_ipType_e
{
	MCAST_TYPE_IPV4,
	MCAST_TYPE_IPV6,
	MCAST_TYPE_END
} omci_dm_mcast_ipType_t;

typedef enum omci_dm_mcast_get_groupTbl_subCmd_s
{
	MCAST_GET_GROUP_TBL_CMD_START = 0,
	MCAST_GET_GROUP_TBL_CMD_GET,
	MCAST_GET_GROUP_TBL_CMD_END
} omci_dm_mcast_get_groupTbl_subCmd_t;



/****************************************************************/
/* Type Definition                                              */
/****************************************************************/
typedef struct omci_dm_mcast_acl_entry_s
{
    uint16   profileId;
    uint32   aclTableEntryId;
    uint16   gemId;
    uint16   aniVid;
    uint32   imputedGrpBw;
    uint16   previewLen;
    uint16   previewRepeatTime;
    uint16   previewRepeatCnt;
    uint16   previewResetTime;
	union
	{
		uint32	ipv4;
		uint8	ipv6[16];
	}sip;
	union
	{
		uint32	ipv4;
		uint8	ipv6[16];
	}dipStart;
	union
	{
		uint32	ipv4;
		uint8	ipv6[16];
	}dipEnd;
}omci_dm_mcast_acl_entry_t;

// for ME 309
typedef struct omci_dm_mcast_prof_s
{
    uint16   profileId;
    uint8    IGMPVersion;
    uint8    IGMPFun;
    uint8    ImmediateLeave;
    uint32   UsIGMPRate;
    uint8    robustness;
    uint32   querierIpAddr;
    uint32   queryIntval;
    uint32   queryMaxRspTime;
    uint32   lastMbrQueryIntval;
    uint8    unAuthJoinRqtBhvr;
}omci_dm_mcast_prof_t;  

// for ME 310, 311
typedef struct omci_dm_mcast_port_Info_s
{
    uint32   portId;
    uint32   SnoopingEnable;
    uint16   profileId;
    uint32   profileMaxGrpNum;        //per port per profile              
    uint32   portMaxGrpNum;           //per port
    uint32   profileMaxBw;            //per port per profile
    uint32   portMaxBw;               //per port 
    uint32   bwEnforceB;
    uint32   currBw;
    uint32   joinCount;
    uint32   bwExcdCount;
}omci_dm_mcast_port_Info_t;

typedef struct omci_dm_mcast_ipv4_active_group_entry_s
{
    uint16	vid;
    uint32	sip;
    uint32	dip;
    uint32	beActualBandwidthEst;
    uint32	clientIp;
    uint32	elapseTime;
    uint16	reserved;
} omci_dm_mcast_ipv4_active_group_entry_t;

typedef struct omci_dm_mcast_ipv4_active_group_table_s
{
    uint32	cmd;
    uint32	activeGrpCnt;
    uint32	entryCnt;
    omci_dm_mcast_ipv4_active_group_entry_t	*pEntry;
} omci_dm_mcast_ipv4_active_group_table_t;




#ifdef __cplusplus
}
#endif

#endif
