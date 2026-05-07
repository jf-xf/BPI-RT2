/*
 * Copyright (C) 2019 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*/

#ifndef __RTK_RG_CA8277_ASICDRIVER__
#define __RTK_RG_CA8277_ASICDRIVER__


#if defined(CONFIG_CA8279_SERIES) || defined(CONFIG_ARCH_RTL8198F)
#ifndef CONFIG_CA_G3_G3LITE_SERIES
#define CONFIG_CA_G3_G3LITE_SERIES 1
#endif
#endif


/*
 * Include Files
 */
 /* Linux */
#include <linux/types.h>

/* RTK */
#include "common/type.h"
#include "rtk_rg_apolloPro_asicDriver.h"

/* CA */
#include <flow.h>
#include <mcast.h>
#include <vlan.h>
#include <l2.h>
#include <nat.h>


/*
 * Symbol Definition
 */
 
#define CA_DEF_DEVID		0


typedef enum
{
	// reserved error code for ca return value
	/*
	CA_E_ERROR          = -1,
	CA_E_OK             = 0x0,
	CA_E_RESOURCE       = 0x1,
	CA_E_PARAM          = 0x2,
	CA_E_NOT_FOUND      = 0x3,
	CA_E_CONFLICT       = 0x4,
	CA_E_TIMEOUT        = 0x5,
	CA_E_INTERNAL       = 0x6,
	CA_E_NOT_SUPPORT    = 0x7,
	CA_E_CONFIG         = 0x8,
	CA_E_UNAVAIL        = 0x9,
	CA_E_MEMORY         = 0xa,
	CA_E_BUSY           = 0xb,
	CA_E_FULL           = 0xc,
	CA_E_EMPTY          = 0xd,
	CA_E_EXISTS         = 0xe,
	CA_E_DEV            = 0xf,
	CA_E_PORT           = 0x10,
	CA_E_LLID           = 0x11,
	CA_E_VLAN           = 0x12,
	CA_E_INIT           = 0x13,
	CA_E_INTF           = 0x14,
	CA_E_NEXTHOP        = 0x15,
	CA_E_ROUTE          = 0x16,
	CA_E_DB_CHANGED     = 0x17,
	CA_E_INACTIVE       = 0x18,
	CA_E_ALREADY_SET    = 0x19,
	*/

	ASIC_RET_FAIL				=-1,
	ASIC_RET_SUCCESS			=0,
	
	ASIC_RET_NULL_POINTER		=0xE001,
	ASIC_RET_OUT_OF_RANGE	=0xE002,
	ASIC_RET_FULL				=0xE003,
	ASIC_RET_NOT_FOUND		=0xE004,
	ASIC_RET_EXIST				=0xE005,

	ASIC_RET_NOT_SUPPORT		=0xE006,
} asic_ret_t;

/**************************************************************************************************
	* Physical Port ID(PPID) definition (ref: aal_port.h) *
***************************************************************************************************/
typedef enum
{
	ASIC_PPORT_L3_WAN		= 0x0a,
} asic_pport_t;

/**************************************************************************************************
	* Logical Port ID(LPID) definition (ref: aal_port.h) *
***************************************************************************************************/
typedef enum
{
	ASIC_LPORT_ETH_NI0		= 0x00,
	ASIC_LPORT_ETH_NI1		= 0x01,
	ASIC_LPORT_ETH_NI2		= 0x02,
	ASIC_LPORT_ETH_NI3		= 0x03,
	ASIC_LPORT_ETH_NI4		= 0x04,
	ASIC_LPORT_ETH_NI5		= 0x05,
	ASIC_LPORT_ETH_NI6		= 0x06,
	ASIC_LPORT_ETH_NI7		= 0x07,
	ASIC_LPORT_CPU_0		= 0x10,
	ASIC_LPORT_CPU_2		= 0x12,
	ASIC_LPORT_CPU_7		= 0x17,
	ASIC_LPORT_L3_WAN		= 0x18,	/* LSPID of LAN_L3 and LDPID of L3_LAN in L3 Engine */
	ASIC_LPORT_L3_LAN		= 0x19,		/* LSPID of WAN_L3 and LDPID of L3_WAN in L3 Engine */
	ASIC_LPORT_MC			= 0x1b,
	ASIC_LPORT_PON_US_0		= 0x20,
	ASIC_LPORT_PON_US_4		= 0x24,
	ASIC_LPORT_PON_US_8		= 0x28,
	ASIC_LPORT_PON_US_15	= 0x2f,
	ASIC_LPORT_PON_US_16	= 0x30,
	ASIC_LPORT_PON_US_31	= 0x3f,
} asic_lport_t;

/* interface table */
typedef struct rtk_rg_asic_netif_ref_s
{
	uint8_t valid;
	rtk_mac_t netif_mac_addr;
	uint32_t mtu_idx;
	uint32_t mac_idx;
#if defined(CONFIG_CA_G3_G3LITE_SERIES)
	uint32_t acl_info;
#elif defined(CONFIG_CA8277B_SERIES)
	uint32_t acl_info;
	uint32_t acl_info_flow_acc;
	uint32_t acl_info_flow_acc_wan;
	uint32_t hash_idx;
#endif
}rtk_rg_asic_netif_ref_t;

#if defined(CONFIG_CA8277B_SERIES)
#define DMA_AFT_FIB_SIZE 16
typedef struct rtk_rg_asic_dmaAftFib_s
{
	uint32_t session_id           : 16;
    uint32_t pppoe_cmd            :  2;
    uint32_t inner_1p_sel         :  2;
    uint32_t inner_802_1p         :  3;
    uint32_t inner_dei_sel        :  2;
    uint32_t inner_dei            :  1;
    uint32_t inner_vid            :  12;
    uint32_t inner_tpid_sel       :  2 ;
    uint32_t inner_tpid_enc       :  3 ;
    uint32_t top_1p_sel           :  2 ;
    uint32_t top_802_1p           :  3 ;
    uint32_t top_dei_sel          :  2 ;
    uint32_t top_dei              :  1 ;
    uint32_t top_vid              : 12 ;
    uint32_t top_tpid_sel         :  2 ;
    uint32_t top_tpid_enc         :  3 ;
    uint32_t inner_vlan_cmd       :  2 ;
    uint32_t vlan_cnt             :  2 ;
    uint32_t vlan_vld             :  1 ;
}rtk_rg_asic_dmaAftFib_t;

typedef struct rtk_rg_asic_dmaAftTpid_s
{
	uint16_t tpid_0;
	uint16_t tpid_1;
	uint16_t tpid_2;
	uint16_t tpid_3;
}rtk_rg_asic_dmaAftTpid_t;

#endif


/*
 * Function Declaration
 */
asic_ret_t rtk_rg_asic_l3qm_p4_init(void);
#if 0
asic_ret_t rtk_rg_asic_l3qm_p3EQ_used_by_p4(void);
#endif
asic_ret_t rtk_rg_asic_netifTable_add(uint32_t idx, rtk_rg_asic_netif_entry_t *pNetif);
asic_ret_t rtk_rg_asic_netifTable_del(uint32_t idx);

#if defined(CONFIG_CA8277B_SERIES)
asic_ret_t rtk_rg_asic_netifTable_get(uint32_t idx, rtk_rg_asic_netif_ref_t *pAsicNetif);
asic_ret_t rtk_rg_asic_netifTable_set(uint32_t idx, rtk_rg_asic_netif_ref_t *pAsicNetif);

asic_ret_t rtk_rg_asic_dmaAftFib_set(uint32_t idx, rtk_rg_asic_dmaAftFib_t fib);
asic_ret_t rtk_rg_asic_dmaAftFib_get(uint32_t idx, rtk_rg_asic_dmaAftFib_t *pFib);
asic_ret_t rtk_rg_asic_dmaAftTpid_set(char tpid_set_msk, rtk_rg_asic_dmaAftTpid_t aftTpid);
asic_ret_t rtk_rg_asic_dmaAftTpid_get(rtk_rg_asic_dmaAftTpid_t *pAftTpid);
#endif

asic_ret_t rtk_rg_asic_fb_init(void);
	
#endif
