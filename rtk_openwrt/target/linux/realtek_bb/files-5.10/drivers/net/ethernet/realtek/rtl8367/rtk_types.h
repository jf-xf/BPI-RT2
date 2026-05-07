#ifndef _RTL8367C_TYPES_H_
#define _RTL8367C_TYPES_H_

#ifndef __KERNEL__
#include <stdio.h>
#endif

typedef unsigned long long      rtksw_uint64;
typedef long long               rtksw_int64;
typedef unsigned int            rtksw_uint32;
typedef int                     rtksw_int32;
typedef unsigned short          rtksw_uint16;
typedef short                   rtksw_int16;
typedef unsigned char           rtksw_uint8;
typedef char                    rtksw_int8;

#define CONST_T     const

#define RTKSW_TOTAL_NUM_OF_WORD_FOR_1BIT_PORT_LIST    1

#define RTKSW_MAX_NUM_OF_PORT                         8
#define RTKSW_PORT_ID_MAX                             (RTKSW_MAX_NUM_OF_PORT-1)
#define RTKSW_PHY_ID_MAX                              (RTKSW_MAX_NUM_OF_PORT-4)
#define RTKSW_MAX_PORT_MASK                           0xFF

#define RTKSW_WHOLE_SYSTEM                            0xFF

typedef struct rtksw_portmask_s
{
    rtksw_uint32  bits[RTKSW_TOTAL_NUM_OF_WORD_FOR_1BIT_PORT_LIST];
} rtksw_portmask_t;

typedef enum rtksw_enable_e
{
    RTKSW_DISABLED = 0,
    RTKSW_ENABLED,
    RTKSW_ENABLE_END
} rtksw_enable_t;

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN      6
#endif

/* ethernet address type */
typedef struct  rtksw_mac_s
{
    rtksw_uint8 octet[ETHER_ADDR_LEN];
} rtksw_mac_t;

typedef rtksw_uint32  rtksw_pri_t;      /* priority vlaue */
typedef rtksw_uint32  rtksw_qid_t;      /* queue id type */
typedef rtksw_uint32  rtksw_data_t;
typedef rtksw_uint32  rtksw_dscp_t;     /* dscp vlaue */
typedef rtksw_uint32  rtksw_fid_t;      /* filter id type */
typedef rtksw_uint32  rtksw_vlan_t;     /* vlan id type */
typedef rtksw_uint32  rtksw_mac_cnt_t;  /* MAC count type  */
typedef rtksw_uint32  rtksw_meter_id_t; /* meter id type  */
typedef rtksw_uint32  rtksw_rate_t;     /* rate type  */

typedef enum rtksw_port_e
{
    UTP_PORT0 = 0,
    UTP_PORT1,
    UTP_PORT2,
    UTP_PORT3,
    UTP_PORT4,
    UTP_PORT5,
    UTP_PORT6,
    UTP_PORT7,

    EXT_PORT0 = 16,
    EXT_PORT1,
    EXT_PORT2,

    UNDEFINE_PORT = 30,
    RTKSW_PORT_MAX = 31
} rtksw_port_t;


#ifndef _RTL_TYPES_H

#if 0
typedef unsigned long long      uint64;
typedef long long               int64;
typedef unsigned int            uint32;
typedef int                     int32;
typedef unsigned short          uint16;
typedef short                   int16;
typedef unsigned char           uint8;
typedef char                    int8;
#endif

typedef rtksw_uint32                  ipaddr_t;
typedef rtksw_uint32                  memaddr;

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN      6
#endif

typedef struct ether_addr_s {
    rtksw_uint8 octet[ETHER_ADDR_LEN];
} ether_addr_t;

#ifdef __KERNEL__
#define rtlglue_printf printk
#else
#define rtlglue_printf printf
#endif
#define PRINT           rtlglue_printf
#endif /*_RTL_TYPES_H*/

/* type abstraction */
#ifdef EMBEDDED_SUPPORT

typedef rtksw_int16                   rtksw_api_ret_t;
typedef rtksw_int16                   ret_t;
typedef rtksw_uint32                  rtksw_u_long;

#else

typedef rtksw_int32                   rtksw_api_ret_t;
typedef rtksw_int32                   ret_t;
typedef rtksw_uint64                  rtksw_u_long_t;

#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define CONST           const
#endif /* _RTL8367C_TYPES_H_ */
