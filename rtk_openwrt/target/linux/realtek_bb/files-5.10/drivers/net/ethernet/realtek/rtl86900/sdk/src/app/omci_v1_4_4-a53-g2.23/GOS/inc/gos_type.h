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
 * Purpose : Definition of OMCI generic OS type define
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI generic OS type define
 */

#ifndef __GOSTYPE_H__
#define __GOSTYPE_H__

#ifdef  __cplusplus
extern "C" {
#endif


#define _BIG_ENDIAN     1234

#define _BYTE_ORDER     _BIG_ENDIAN


typedef char  CHAR;
typedef char  INT8;
typedef short INT16;
typedef int   INT32;

typedef unsigned char  UINT8;
typedef unsigned short UINT16;
typedef unsigned int   UINT32;
typedef unsigned long long UINT64;

#ifndef BOOL
typedef UINT32         BOOL;
#endif
#ifndef STATUS
typedef INT32          STATUS;
#endif
typedef unsigned long  ulong_t;

typedef void *  (*FUNCPTR)(void *);
typedef void    (*VOIDFUNCPTR)(void *);
typedef UINT32  (*RW_FUNC)(UINT32, UINT32, UINT32, UINT32);

#if !defined(WAIT_FOREVER)
#define WAIT_FOREVER  (0xffffffff)
#endif

#define STD_IN     0
#define STD_OUT    1
#define STD_ERR    2


#if !defined(FALSE) || (FALSE!=0)
#define FALSE       ((BOOL)0)
#endif

#if !defined(TRUE) || (TRUE!=1)
#define TRUE        ((BOOL)1)
#endif

#define ERROR -1
#define OK    0

extern int gWordBase;

#define CASTING(x) ((gWordBase) == 8 ? (UINT64)x : (UINT32)x)


typedef enum
{
    GOS_OK = 0,             // OK
    GOS_FAIL,               // Common failure
    GOS_ERR_PARAM,          // Parameter illegal, general error
    GOS_ERR_NOTSUPPORT,     // Not Supported, it should supported in future
    GOS_ERR_DISABLE,        // Object is disabled
    GOS_ERR_STATE,          // unacceptable state
    GOS_ERR_EVENT,          // unacceptable event
    GOS_ERR_OVERFLOW,       // Requests are more than the max capacity
    GOS_ERR_HARDWARE,       // hardware handle failed
    GOS_ERR_FPGA_DOWNLOAD,  // FPGA download failed
    GOS_ERR_UNREACHABLE,    // Unreachable case, if so, there are some sequence error or design fail
    GOS_ERR_SOCKFAIL,       // Failed to read/write on the socket
    GOS_ERR_INVALID_CHAR,   // Receive a invalid char
    GOS_ERR_USER_CANCEL,    // User canceled
    GOS_ERR_INVALID_INPUT,  // Invalid input from user
    GOS_ERR_NOT_FOUND,      // Object not found
    GOS_ERR_SYS_CALL_FAIL,  // System call returned failure,
    GOS_ERR_USER_EXIT,      // User exit the program
    GOS_ERR_DUPLICATED,     // Object or operation duplicated
    GOS_ERR_CRC,            // CRC check failed
    GOS_ERR_NOT_INIT,       // Not initialized
    GOS_ERR_UNKNOWN = 255
}GOS_ERROR_CODE;

#define GOS_ASSERT(cond) if (!(cond)) \
                      {\
                        printf("\r\nAssertion Failed:"#cond", file %s, line %d, pid %d\r\n", __FILE__, __LINE__, getpid()); \
                        abort();\
                      }\
                      else {}

#define GOS_NELEM(array)   (sizeof(array)/sizeof(array[0]))

#ifdef OMCI_X86
typedef enum rt_port_type_e
{
    RT_PORT_NONE = 0,
    RT_FE_PORT,
    RT_GE_PORT,
    RT_GE_COMBO_PORT,
    RT_GE_SERDES_PORT,
    RT_CPU_PORT,
    RT_INT_FE_PORT,
    RT_SWPBO_PORT,
    RT_PORT_TYPE_END
} rt_port_type_t;

#define RTK_GPON_PLOAM_MSG_LEN           10

typedef struct rtk_gpon_ploam_e{
    UINT8 onuid;
    UINT8 type;
    UINT8 msg[RTK_GPON_PLOAM_MSG_LEN];
}rtk_gpon_ploam_t;

typedef struct rtk_gpon_alarm_s{
    UINT8 msg[2];//first byte is type, snd byte is state
}rtk_gpon_alarm_t;

#define RTK_GPON_OMCI_MSG_LEN 1500

typedef struct rtk_gpon_omci_msg_s{
    UINT8 msg[RTK_GPON_OMCI_MSG_LEN];
    UINT32 len;
}rtk_gpon_omci_msg_t;

typedef enum bcast_msg_type_e
{
    // interrupt
    MSG_TYPE_LINK_CHANGE = 0,
    MSG_TYPE_METER_EXCEED,
    MSG_TYPE_LEARN_OVER,
    MSG_TYPE_SPEED_CHANGE,
    MSG_TYPE_SPECIAL_CONGEST,
    MSG_TYPE_LOOP_DETECTION,
    MSG_TYPE_CABLE_DIAG_FIN,
    MSG_TYPE_ACL_ACTION,
    MSG_TYPE_GPHY,
    MSG_TYPE_SERDES,
    MSG_TYPE_GPON,
    MSG_TYPE_EPON,
    MSG_TYPE_PTP,
    MSG_TYPE_DYING_GASP,
    MSG_TYPE_THERMAL,
    MSG_TYPE_ADC,
    MSG_TYPE_EEPROM_UPDATE_110OR118,
    MSG_TYPE_EEPROM_UPDATE_128TO247,
    MSG_TYPE_PKTBUFFER_ERROR,
    // gpon event
    MSG_TYPE_ONU_STATE = 20,
    MSG_TYPE_RLDP_LOOP_STATE_CHNG,
    MSG_TYPE_OMCI_EVENT,
    // epon event
    MSG_TYPE_EPON_INTR_LOS =30,
    MSG_TYPE_TOD_UPDATE_ENABLE,
    MSG_TYPE_END
} bcast_msg_type_t;

typedef enum rtk_enable_e
{
    DISABLED = 0,
    ENABLED,
    RTK_ENABLE_END
} rtk_enable_t;

typedef struct {
    bcast_msg_type_t    intrType;
    UINT32              intrSubType;
    UINT32              intrBitMask;
    rtk_enable_t        intrStatus;
} intrBcasterMsg_t;

#define MAX_BYTE_PER_INTR_BCASTER_MSG   (sizeof(intrBcasterMsg_t))

typedef enum rtk_intr_status_e
{
    INTR_STATUS_SPEED_CHANGE = 0,
    INTR_STATUS_LINKUP,
    INTR_STATUS_LINKDOWN,
    INTR_STATUS_GPHY,

    INTR_STATUS_END
} rtk_intr_status_t;

#define INTR_BCASTER_NETLINK_TYPE   NETLINK_UNUSED

#define RTK_GPON_EXTMESG_MEX_LEN 256

typedef struct rtk_gpon_extMsg_s{
    int optId;
    int len;
    char extValue[RTK_GPON_EXTMESG_MEX_LEN];
}rtk_gpon_extMsg_t;

typedef enum rtk_gpon_onuState_e{
    GPON_STATE_UNKNOWN=0,
    GPON_STATE_O1,
    GPON_STATE_O2,
    GPON_STATE_O3,
    GPON_STATE_O4,
    GPON_STATE_O5,
    GPON_STATE_O6,
    GPON_STATE_O7,
    GPON_STATE_END
}rtk_gpon_onuState_t;


#define RTK_MAX_NUM_OF_PORTS 32
#define ETHER_ADDR_LEN 6

#endif

#ifdef  __cplusplus
}
#endif

#endif
