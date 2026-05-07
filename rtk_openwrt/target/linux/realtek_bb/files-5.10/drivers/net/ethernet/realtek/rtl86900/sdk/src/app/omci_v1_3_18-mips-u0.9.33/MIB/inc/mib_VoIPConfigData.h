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
 * Purpose : Definition of ME attribute: VoIP config data (138)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: VoIP config data (138)
 */


#ifndef __MIB_VOIPCONFIGDATA_TABLE_H__
#define __MIB_VOIPCONFIGDATA_TABLE_H__

#ifdef __cplusplus
extern "C" {
#endif


/* Table VoIPConfigData attribute for STRING type define each entry length */
#define MIB_TABLE_PROFILEVERSION_LEN (25)

/* Table VoIPConfigData attribute index */
#define MIB_TABLE_VOIPCONFIGDATA_ATTR_NUM (9)
#define MIB_TABLE_VOIPCONFIGDATA_ENTITYID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_VOIPCONFIGDATA_AVAILABLESIGNALLINGPROTOCOLS_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_VOIPCONFIGDATA_SIGNALLINGPROTOCOLUSED_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_VOIPCONFIGDATA_AVAILABLEVOIPCONFIGURATIONMETHODS_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_VOIPCONFIGDATA_VOIPCONFIGURATIONMETHODUSED_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_VOIPCONFIGDATA_VOIPCONFIGURATIONADDRESSPOINTER_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_VOIPCONFIGDATA_VOIPCONFIGURATIONSTATE_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_VOIPCONFIGDATA_RETRIEVEPROFILE_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_VOIPCONFIGDATA_PROFILEVERSION_INDEX ((MIB_ATTR_INDEX)9)


typedef enum {
    VCD_AVAILABLE_SIG_PROTOCOL_SIP		= (1 << 0),
    VCD_AVAILABLE_SIG_PROTOCOL_H248		= (1 << 1),
    VCD_AVAILABLE_SIG_PROTOCOL_MGCP		= (1 << 2)
} vcd_attr_available_sig_protocol_t;

typedef enum {
    VCD_SIG_PROTOCOL_USED_NONE		= 0,
    VCD_SIG_PROTOCOL_USED_SIP		= 1,
    VCD_SIG_PROTOCOL_USED_H248		= 2,
    VCD_SIG_PROTOCOL_USED_MGCP		= 3,
    VCD_SIG_PROTOCOL_USED_NON_OMCI	= 0xFF
} vcd_attr_sig_protocol_used_t;

typedef enum {
    VCD_AVAILABLE_VOIP_CFG_METHOD_OMCI			= (1 << 0),
    VCD_AVAILABLE_VOIP_CFG_METHOD_CFG_FILE		= (1 << 1),
    VCD_AVAILABLE_VOIP_CFG_METHOD_TR069			= (1 << 2),
    VCD_AVAILABLE_VOIP_CFG_METHOD_SIPPING		= (1 << 3)
} vcd_attr_available_voip_cfg_method_t;

typedef enum {
    VCD_CFG_METHOD_USED_NONE		= 0,
    VCD_CFG_METHOD_USED_OMCI		= 1,
    VCD_CFG_METHOD_USED_CFG_FILE	= 2,
    VCD_CFG_METHOD_USED_TR069		= 3,
    VCD_CFG_METHOD_USED_SIPPING		= 4
} vcd_attr_voip_cfg_method_used_t;

typedef enum {
    VCD_CFG_STATE_INACTIVE		= 0,
    VCD_CFG_STATE_ACTIVE		= 1,
    VCD_CFG_STATE_INITIALIZING	= 2,
    VCD_CFG_STATE_FAULT			= 3
} vcd_attr_voip_cfg_state_t;

/* Table VoIPConfigData attribute len, only string attrubutes have length definition */
typedef struct {
	UINT16   EntityId;
	UINT8    AvailableSignallingProtocols;
	UINT8    SignallingProtocolUsed;
	UINT32   AvailableVoIPConfigurationMethods;
	UINT8    VOIPConfigurationMethodUsed;
	UINT16   VOIPConfigurationAddressPointer;
	UINT8    VOIPConfigurationState;
	UINT8    RetrieveProfile;
	CHAR     ProfileVersion[MIB_TABLE_PROFILEVERSION_LEN+1];
} __attribute__((aligned)) MIB_TABLE_VOIPCONFIGDATA_T;

#ifdef __cplusplus
}
#endif

#endif /* __MIB_VOIPCONFIGDATA_TABLE_H__ */
