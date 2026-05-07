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
#ifndef __MIB_EXTENDED_IP_HOST_CFG_DATA_TABLE_H__
#define __MIB_EXTENDED_IP_HOST_CFG_DATA_TABLE_H__

/* Table extended_ip_host_cfg_data attribute len, only string attrubutes have length definition */
#define MIB_TABLE_EXTENDED_IP_HOST_CFG_DATA_STRING_PART_LEN (25)

/* Table extended_ip_host_cfg_data attribute index */
#define MIB_TABLE_EXTENDED_IP_HOST_CFG_DATA_ATTR_NUM (9)
#define MIB_TABLE_EXTENDED_IP_HOST_CFG_DATA_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_EXTENDED_IP_HOST_CFG_DATA_NATSTATUS_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_EXTENDED_IP_HOST_CFG_DATA_MODE_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_EXTENDED_IP_HOST_CFG_DATA_CONNECT_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_EXTENDED_IP_HOST_CFG_DATA_RELEASETIME_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_EXTENDED_IP_HOST_CFG_DATA_USER_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_EXTENDED_IP_HOST_CFG_DATA_PASSWORD_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_EXTENDED_IP_HOST_CFG_DATA_STATE_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_EXTENDED_IP_HOST_CFG_DATA_ONLINE_DURATION_INDEX ((MIB_ATTR_INDEX)9)

typedef enum {
    EXTENDED_IP_HOST_CFG_DATA_AUTO      = 0,
    EXTENDED_IP_HOST_CFG_DATA_CHAP      = 1,
    EXTENDED_IP_HOST_CFG_DATA_PAP       = 2
} extended_ip_host_cfg_data_mode_t;

typedef enum {
    EXTENDED_IP_HOST_CFG_DATA_ALWAYS    = 0,
    EXTENDED_IP_HOST_CFG_DATA_ONDEMAND  = 1,
    EXTENDED_IP_HOST_CFG_DATA_MANUAL    = 2
} extended_ip_host_cfg_data_connect_t;

typedef enum {
    EXTENDED_IP_HOST_CFG_DATA_UNCONFIGURED  = 0,    // indicates that the PPPoE connection is not configured
    EXTENDED_IP_HOST_CFG_DATA_CONNECTING,           // indicates that the connection is in progress
    EXTENDED_IP_HOST_CFG_DATA_AUTHENTICATING,       // indicates that the authentication is in progress
    EXTENDED_IP_HOST_CFG_DATA_CONNECTED,            // indicates that the connection is established
    EXTENDED_IP_HOST_CFG_DATA_PENDING_DISCONNECT,   // indicates that the decision of disconnection is in progress
    EXTENDED_IP_HOST_CFG_DATA_DISCONNECTING,        // indicates that the disconnection is in progress
    EXTENDED_IP_HOST_CFG_DATA_DISCONNECTED,         // indicates that the connection is down
    EXTENDED_IP_HOST_CFG_DATA_DEMAND                // indicates that the connection is needed
} extended_ip_host_cfg_data_state_t;


/* Table extended_ip_host_cfg_data attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
    UINT8    NatStatus;
    UINT8    Mode;
    UINT8    Connect;
    UINT16   ReleaseTime;
	CHAR     User[MIB_TABLE_EXTENDED_IP_HOST_CFG_DATA_STRING_PART_LEN+1];
	CHAR     Password[MIB_TABLE_EXTENDED_IP_HOST_CFG_DATA_STRING_PART_LEN+1];
    UINT8    State;
    UINT32   OnlineDuration;
} __attribute__((packed)) MIB_TABLE_EXTENDED_IP_HOST_CFG_DATA_T;

#endif /* __MIB_EXTENDED_IP_HOST_CFG_DATA_TABLE_H__ */
