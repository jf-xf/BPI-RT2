/*
 * Copyright (C) 2016 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*/

#ifndef __RTK_FC_DEFINE__
#define __RTK_FC_DEFINE__

#include <linux/version.h>

#include "rtk_fc_port.h"

#if defined(CONFIG_RTK_L34_G3_PLATFORM)
#include <aal_hash.h>
#include <aal_l3_tm.h>
#include <aal_l3_cam.h>
#include <aal_l3qm.h>
#endif

#if defined(CONFIG_RTK_PTOOL_CPU_PERF)
#include <linux/ptool.h>
#endif

#define __IRAM_FWDENG
#define __IRAM_FWDENG_L2
#define __IRAM_FWDENG_L34
#define __IRAM_FWDENG_SLOWPATH
#define __SRAM_FWDENG_SLOWPATH
#define __SRAM_FWDENG_DATA
#define __SRAM_FWDENG_PREDATA

#define FC_UNCACHE_DW(x)  (*((volatile u32*)(((u32)x)|0xa0000000)))
#define FC_UNCACHE_ADDR(x)  (((u32)x)|0xa0000000)

#define ___ALIGN___


#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)

#define RTK_FC_HW_MAX_JOBS  256

#define RTK_FC_IPSEC_IPI_DISPATCH_CPU 		 (3)


#define RTK_FC_IPSEC_AES_MIN_KEY_SIZE	16
#define RTK_FC_IPSEC_AES_MAX_KEY_SIZE	32
#define RTK_FC_IPSEC_AES_KEYSIZE_128		16
#define RTK_FC_IPSEC_AES_KEYSIZE_192		24
#define RTK_FC_IPSEC_AES_KEYSIZE_256		32
#define RTK_FC_IPSEC_AES_BLOCK_SIZE		16
#define RTK_FC_IPSEC_AES_MAX_KEYLENGTH	(15 * 16)
#define RTK_FC_IPSEC_AES_MAX_KEYLENGTH_U32	(AES_MAX_KEYLENGTH / sizeof(u32))

/********* Protect Register Offsets **********/
#define RTK_FC_CRYPTO_REG_PROTECT             0x0000L

#define RTK_FC_CRYPTO_PROTECT_ENABLE          0x10


/********* Crypto Register Offsets **********/
#define RTK_FC_CRYPTO_REG_SDSR                 0x0000L                                                 /*Source Descriptor Status Register*/
#define RTK_FC_CRYPTO_REG_SDPTR                0x000CL                                                 /*Source Descriptor Pointer Register*/
#define RTK_FC_CRYPTO_REG_SDFWR                0x0004L                                                 /*Source Descriptor First Word Register*/
#define RTK_FC_CRYPTO_REG_SDSWR                0x0008L                                                 /*Source Descriptor Second Word Register*/

#define RTK_FC_CRYPTO_REG_DD_OFFSET            0x1000L
#define RTK_FC_CRYPTO_REG_DDSR                 (RTK_FC_CRYPTO_REG_SDSR+RTK_FC_CRYPTO_REG_DD_OFFSET)          /*Destination Descriptor Status Register*/
#define RTK_FC_CRYPTO_REG_DDPTR                0x101CL         										  /*Destination Descriptor Pointer Register*/
#define RTK_FC_CRYPTO_REG_DDFWR                (RTK_FC_CRYPTO_REG_SDFWR+RTK_FC_CRYPTO_REG_DD_OFFSET)         /*Destination Descriptor First Word Register*/
#define RTK_FC_CRYPTO_REG_DDSWR                (RTK_FC_CRYPTO_REG_SDSWR+RTK_FC_CRYPTO_REG_DD_OFFSET)         /*Destination Descriptor Second Word Register*/

#define RTK_FC_CRYPTO_REG_IPSCSR_RSTEACONFISR  0x0010L                                                 /*Reset Engine And Configure Interrupt Service Routine Register*/
#define RTK_FC_CRYPTO_REG_IPSCSR_INTM          0x0014L                                                 /*Interrupt Mask Register*/
#define RTK_FC_CRYPTO_REG_IPSCSR_DBG           0x0018L                                                 /*Debug Register*/
#define RTK_FC_CRYPTO_REG_IPSCSR_ERR_INT                  0x001CL                                                 /*Error Status Register*/
#define RTK_FC_CRYPTO_REG_IPSCSR_SWAPABURSTR   0x0030L                                                 /*Swap And Burst Register*/

#define RTK_FC_CRYPTO_REG_REVISION_ID          0x0888L                                                 /*IPsec revision ID Register*/

#define RTK_FC_CRYPTO_REG_SKDR_OFFSET		  (0x20L)
#define RTK_FC_CRYPTO_REG_SK0DR                0x0500L                                                 /*Secure Key Storage*/
#define RTK_FC_CRYPTO_REG_SKDR(x)			  (RTK_FC_CRYPTO_REG_SK0DR + x*RTK_FC_CRYPTO_REG_SKDR_OFFSET)

#define RTK_FC_CRYPTO_REG_SKP_OFFSET			  (0x100L)
#define RTK_FC_CRYPTO_REG_SKP0				  0x1100L												   /*Secure KeyPAD Storage*/
#define RTK_FC_CRYPTO_REG_SKP(x)				  (RTK_FC_CRYPTO_REG_SKP0 + x*RTK_FC_CRYPTO_REG_SKP_OFFSET)

#define RTK_FC_CRYPTO_REG_KEYPAD_LOCK_OFFSET  (0x4L)
#define RTK_FC_CRYPTO_REG_KEYPAD0_LOCK		  0x10C0L												   /*Secure KeyPAD Lock & LEN*/
#define RTK_FC_CRYPTO_REG_KEYPAD_LOCK(x)		  (RTK_FC_CRYPTO_REG_KEYPAD0_LOCK + x*RTK_FC_CRYPTO_REG_KEYPAD_LOCK_OFFSET)

#define RTK_FC_CRYPTO_REG_SENLR 				0x0024L

#define RTK_FC_BIT_MASK_ENABLE(x) (1UL << x)

/* RTK_CRYPTO_REG_IPSCSR_RSTEACONFISR Bit Masks */
#define BIT_RTK_FC_IPSCSR_RSTEACONFISR_SOFT_RST       0
#define BIT_RTK_FC_IPSCSR_RSTEACONFISR_DMA_BUSY        3
#define BIT_RTK_FC_IPSCSR_RSTEACONFISR_CMD_OK         4
#define BIT_RTK_FC_IPSCSR_RSTEACONFISR_INTR_MODE       7
#define BIT_RTK_FC_IPSCSR_RSTEACONFISR_IPSEC_RST      31

/* RTK_CRYPTO_REG_IPSCSR_SWAPABURSTR Bit Masks */
#define BIT_RTK_FC_IPSCSR_SWAPABURSTR_KEY_IV_SWAP                     1
#define BIT_RTK_FC_IPSCSR_SWAPABURSTR_KEY_PAD_SWAP                    2
#define BIT_RTK_FC_IPSCSR_SWAPABURSTR_HASH_INITIAL_VALUE_SWAP         3
#define BIT_RTK_FC_IPSCSR_SWAPABURSTR_DATA_IN_LITTLE_ENDIAN           4
#define BIT_RTK_FC_IPSCSR_SWAPABURSTR_TX_BYTE_SWAP                    8
#define BIT_RTK_FC_IPSCSR_SWAPABURSTR_DATA_OUT_LITTLE_ENDIAN          9
#define BIT_RTK_FC_IPSCSR_SWAPABURSTR_MAC_IN_LITTLE_ENDIAN            10
#define BIT_RTK_FC_IPSCSR_SWAPABURSTR_RX_WD_SWAP                      11
#define BIT_RTK_FC_IPSCSR_SWAPABURSTR_TX_WD_SWAP                      12

/* RTK_CRYPTO_REG_IPSCSR_DBG Bit Masks */
#define BIT_RTK_FC_IPSCSR_DBG_ARBITER_MODE       16
#define BIT_RTK_FC_IPSCSR_DBG_CLK_EN        24


/* Authentication algorithms from pfkeyv2.h */
#define RTK_FC_AALG_NONE			0
#define RTK_FC_AALG_MD5HMAC		2
#define RTK_FC_AALG_SHA1HMAC		3
#define RTK_FC_AALG_SHA2_256HMAC	5
#define RTK_FC_AALG_SHA2_384HMAC	6
#define RTK_FC_AALG_SHA2_512HMAC	7
#define RTK_FC_AALG_RIPEMD160HMAC	8
#define RTK_FC_AALG_AES_XCBC_MAC	9
#define RTK_FC_AALG_NULL		251	/* kame */
#define RTK_FC_AALG_MAX			251

/* Encryption algorithms from pfkeyv2.h*/
#define RTK_FC_EALG_NONE			0
#define RTK_FC_EALG_DESCBC		2
#define RTK_FC_EALG_3DESCBC		3
#define RTK_FC_EALG_CASTCBC		6
#define RTK_FC_EALG_BLOWFISHCBC		7
#define RTK_FC_EALG_NULL			11
#define RTK_FC_EALG_AESCBC		12
#define RTK_FC_EALG_AESCTR		13
#define RTK_FC_EALG_AES_CCM_ICV8	14
#define RTK_FC_EALG_AES_CCM_ICV12	15
#define RTK_FC_EALG_AES_CCM_ICV16	16
#define RTK_FC_EALG_AES_GCM_ICV8	18
#define RTK_FC_EALG_AES_GCM_ICV12	19
#define RTK_FC_EALG_AES_GCM_ICV16	20
#define RTK_FC_EALG_CAMELLIACBC		22
#define RTK_FC_EALG_NULL_AES_GMAC	23
#define RTK_FC_EALG_MAX                   253 /* last EALG */

//#define RTK_FC_IPSEC_DIR_ENCRYT     1 
//#define RTK_FC_IPSEC_DIR_DECRYT     0          

//#define RTK_FC_IPSEC_SWID_BASE      32


#define RTK_FC_IPSEC_TABLE_SIZE_BIT 5
#define RTK_FC_SPEC_XFRM_INFO_MAX_NUM (12)
#define RTK_FC_SPEC_HSAH_PADDING_MAX_NUM 400

typedef enum rtk_fc_ipsec_direction_e
{
	RTK_FC_IPSEC_DIR_DECRYT = 0,
	RTK_FC_IPSEC_DIR_ENCRYT = 1
} rtk_fc_ipsec_direction_t;
#endif


#define RTK_FC_PATH5_DS_FLOWLIST_BUCKET_MAX_NUM (256)

/* common */
#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAIL
#define FAIL -1
#endif

#ifndef RTK_SKB_COPIED
#define RTK_SKB_COPIED 1
#endif

#if !defined(CONFIG_RTK_L34_G3_PLATFORM)
#ifndef DISABLE
#define DISABLE 0
#endif

#ifndef ENABLE
#define ENABLE 1
#endif
#endif

#ifndef FAILED
#define FAILED -1
#endif

#ifndef SIGNED_INVALID
#define SIGNED_INVALID -1
#endif

#ifndef IS_CLASSD_ADDR
#define IS_CLASSD_ADDR(ipv4addr)				((((uint32)(ipv4addr)) & 0xf0000000) == 0xe0000000)
#endif


#if defined(CONFIG_FC_CAG3_SERIES) || defined(CONFIG_FC_RTL8198F_SERIES)
#define CONFIG_FC_G3_G3LITE_SERIES 1
#endif

#if defined(CONFIG_FC_CAG3_SERIES) || defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8198F_SERIES)
#define CONFIG_FC_CA8277AB_SERIES 1
#endif

#define LAN_VID 1777U
#define WAN_VID 1778U
#define NETIF_HWTOSW(hwIdx) fc_db.netifHwTblVaild[hwIdx].swNetifIdx


#define HWNAT_MODE_TEST_MODE 100
#if defined(CONFIG_RTK_SOC_RTL8198D)
#define RTK_FC_RTNL_JOB_ARRAY_SIZE 16
#else
#define RTK_FC_RTNL_JOB_ARRAY_SIZE 8
#endif

#define RTK_FC_NATLOOPBACK_BUCKET_BITS		4
#define RTK_FC_NATLOOPBACK_WAY_BITS			2
#define RTK_FC_TABLESIZE_NATLOOPBACK_BUCKET	(1<<RTK_FC_NATLOOPBACK_BUCKET_BITS)
#define RTK_FC_TABLESIZE_NATLOOPBACK_WAY	(1<<RTK_FC_NATLOOPBACK_WAY_BITS)

#define RTK_FC_TABLESIZE_LUT_IVL_BITS			(2)
#define RTK_FC_TABLESIZE_LUT_IVL				(1<<RTK_FC_TABLESIZE_LUT_IVL_BITS)

// software meter
#define RTK_FC_TABLESIZE_SW_SHAREMTR 32

#define RTK_FC_TABLESIZE_SW_SHAPING	(RTK_FC_TABLESIZE_FBMTR+RTK_FC_TABLESIZE_SW_SHAREMTR)
#define RTK_FC_MAX_SHAPING_QUEUE_SIZE 	128		//bucket size for busting
#define RTK_FC_SW_SHAPING_CTRL_IDX_OFFSET_FLOWMETER 0
#define RTK_FC_SW_SHAPING_CTRL_IDX_OFFSET_SWMETER RTK_FC_TABLESIZE_FBMTR

#define RTK_FC_TABLESIZE_MCFLOW_HASH_SHIFTBITS	5
#define RTK_FC_TABLESIZE_MCFLOW_HASH	(1<<RTK_FC_TABLESIZE_MCFLOW_HASH_SHIFTBITS)
#define RTK_FC_TABLESIZE_MCFLOW			512
#define RTK_FC_TABLESIZE_MC_GROUP_DFTFWD	(RTK_FC_TABLESIZE_MCFLOW/4)


#if defined(CONFIG_RTK_L34_XPON_PLATFORM)

#define HW_BC_IDX 4094
#define CHIP_ID_9607C (ASIC_CHIP_ID==RTL9607C_CHIP_ID)

// Table Size
#define RTK_FC_TABLESIZE_LUT				(LUTTABLE_SRAM_SIZE+LUTTABLE_BCAM_SIZE)
#define RTK_FC_TABLESIZE_INTF				(FLOWBASED_TABLESIZE_INTF)		// hw size
#define RTK_FC_TABLESIZE_INTF_SW_BITS 	(CONFIG_RTK_FC_SW_INTF_ENTRY_BITS)
#define RTK_FC_TABLESIZE_INTF_SW			(1<<RTK_FC_TABLESIZE_INTF_SW_BITS)	//(RTK_FC_TABLESIZE_INTF<<1)	// double size for sw usage
#define RTK_FC_DEV_GW_MAC_TBL				(1<<NETIF_DEV_IDX_SHIFT)
#define RTK_FC_TABLESIZE_INDMAC				FLOWBASED_TABLESIZE_MACIND
#define RTK_FC_TABLESIZE_ETHERTYPE			FLOWBASED_TABLESIZE_ETHERTYPE
#define RTK_FC_TABLESIZE_EXTPORT			FLOWBASED_TABLESIZE_EXTPORT
#define RTK_FC_TABLESIZE_EXTMBR				FLOWBASED_TABLESIZE_EXTMBR
#define RTK_FC_TABLESIZE_FLOWSRAM			FLOWBASED_TABLESIZE_FLOWSRAM
#define RTK_FC_TABLESIZE_FLOWTCAM			FLOWBASED_TABLESIZE_FLOWTCAM
#define RTK_FC_TABLESIZE_FLOWTAG 			FLOWBASED_TABLESIZE_FLOWTAG
#define RTK_FC_TABLESIZE_CAMTAG 			FLOWBASED_TABLESIZE_CAMTAG
#define RTK_FC_TABLESIZE_EXTRATAG_LISTMIN 	FLOWBASED_EXTRATAG_LISTMIN
#define RTK_FC_TABLESIZE_EXTRATAG_LISTMAX 	FLOWBASED_EXTRATAG_LISTMAX
#define RTK_FC_TABLESIZE_EXTRATAG			(FLOWBASED_EXTRATAG_LISTMAX+1)		// 8
#define RTK_FC_TABLESIZE_EXTRATAG_ACTIONS 	FLOWBASED_EXTRATAG_ACTIONS
#define RTK_FC_DUAL_CONTENT_BUF_ENT_OFFSET	64		/*content buffer size per entry*/
#define RTK_FC_DUAL_CONTENT_BUF_SIZE		(1<<ASIC_DUAL_CONTENT_BUFFER_SIZE_BITS)		/*512*/
#define RTK_FC_EXTRATAG_IDX_TO_SRC_ADDR_OFFSET(hwidx)	(hwidx - RTK_FC_TABLESIZE_EXTRATAG_LISTMIN)
#define RTK_FC_TABLESIZE_SHAREMTR			48
#define RTK_FC_TABLESIZE_FBMTR				FLOWBASED_TABLESIZE_SHAREMTR //flow-based meter

#define RTK_FC_DEFAULT_SHORTCUT_FLOW_SIZE	RTK_FC_MAX_SHORTCUT_FLOW_SIZE	//pure sw flow count


#define EXTPTBL_RSVED_ENTRY				1	// reserve entry[0] to indicate none extension port setting

#if defined(CONFIG_FC_RTL9603CVD_SERIES)
#define RTK_FTCHECK_GRANULARITY			256		//buckets
#else
#define RTK_FTCHECK_GRANULARITY			1024	//buckets
#endif

#define MAX_FLOW_LUT_IDX_WIDTH			(11+1) //+1 for bcam
#define MAX_FLOW_IDLESEC_WIDTH			12

#define RTK_FC_NETIF_START_IDX 0

#define RTK_FC_FLOWENT_ALIGNBUF		1023

#define METER_RATE_MIN 0
#ifndef RT_RATE_HW_IDX_L3_METER_RATE_MAX
#define METER_RATE_MAX 4194296
#else
#define METER_RATE_MAX RT_RATE_HW_IDX_L3_METER_RATE_MAX // 4194296
#endif
#ifndef RT_RATE_HW_IDX_L3_METER_BURST_MAX
#define METER_BURST_MAX 0xFFFF
#else
#define METER_BURST_MAX RT_RATE_HW_IDX_L3_METER_BURST_MAX
#endif



#elif defined(CONFIG_RTK_L34_G3_PLATFORM)

// Table Size
#define RTK_FC_TABLESIZE_LUT			(LUTTABLE_SRAM_SIZE+LUTTABLE_BCAM_SIZE)
#define RTK_FC_TABLESIZE_INTF			(FLOWBASED_TABLESIZE_INTF)	// hw size, in order to compatible with flow in/out intf definition
#define RTK_FC_TABLESIZE_INTF_SW_BITS (CONFIG_RTK_FC_SW_INTF_ENTRY_BITS)
#define RTK_FC_TABLESIZE_INTF_SW		(1<<RTK_FC_TABLESIZE_INTF_SW_BITS)		// sw size, same as G3 hw entries,
#define RTK_FC_DEV_GW_MAC_TBL			(1<<NETIF_DEV_IDX_SHIFT)
#define RTK_FC_TABLESIZE_INTF_MC_ACL	(RTK_FC_TABLESIZE_INTF)		// hw acl size, table AAL_TABLE_L3_CAM_MAC_DA 7 entry + 16 hw acl entry
#define RTK_FC_TABLESIZE_INTF_ACC_ACL	(RTK_FC_TABLESIZE_INTF)		// hw acl size, table AAL_TABLE_L3_CAM_MAC_DA 7 entry + 16 hw acl entry
#define RTK_FC_TABLESIZE_INDMAC		256
#define RTK_FC_TABLESIZE_ETHERTYPE	((L3_CAM_ETHERTYPE_TBL_ENTRY_MAX>15) ? 15 : L3_CAM_ETHERTYPE_TBL_ENTRY_MAX)
#define RTK_FC_TABLESIZE_EXTPORT		32
#define RTK_FC_TABLESIZE_EXTMBR		0
#define RTK_FC_TABLESIZE_FLOWSRAM		0
#define RTK_FC_TABLESIZE_FLOWTCAM		0
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
#define RTK_FC_TABLESIZE_EXTRATAG_LISTMIN 		0
#define RTK_FC_TABLESIZE_EXTRATAG_LISTMAX 		15
#define RTK_FC_TABLESIZE_EXTRATAG				16
#define RTK_FC_TABLESIZE_EXTRATAG_ACTIONS 		8
#define RTK_FC_DUAL_CONTENT_BUF_ENT_OFFSET			64		/*content buffer size per entry*/
#define RTK_FC_DUAL_CONTENT_BUF_SIZE			(1<<ASIC_DUAL_CONTENT_BUFFER_SIZE_BITS)		/*1024*/
#else
#define RTK_FC_TABLESIZE_EXTRATAG_LISTMIN 		1
#define RTK_FC_TABLESIZE_EXTRATAG_LISTMAX 		7
#define RTK_FC_TABLESIZE_EXTRATAG				8
#define RTK_FC_TABLESIZE_EXTRATAG_ACTIONS 		8
#define RTK_FC_DUAL_CONTENT_BUF_ENT_OFFSET			64		/*content buffer size per entry*/
#define RTK_FC_DUAL_CONTENT_BUF_SIZE			(1<<ASIC_DUAL_CONTENT_BUFFER_SIZE_BITS)		/*512*/
#endif
#define RTK_FC_EXTRATAG_IDX_TO_SRC_ADDR_OFFSET(hwidx)	(hwidx - RTK_FC_TABLESIZE_EXTRATAG_LISTMIN)
#define RTK_FC_TABLESIZE_SHAREMTR		32
#define RTK_FC_TABLESIZE_FBMTR		32 //flow-based meter
#if defined(CONFIG_FC_CA8277B_SERIES)
#define RTK_FC_DMAAFTACTION_SIZE_BITS	4
#define RTK_FC_TABLESIZE_DMAAFTACTION	(1<<RTK_FC_DMAAFTACTION_SIZE_BITS) //16
#define RTK_FC_TABLESIZE_POLREMAP		8		//ACL remap pol to pol2
#define RTK_FC_TABLESIZE_STREAMID		128
#endif
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
#define RTK_FC_DMAAFTACTION_RSV_SIZE_BITS	4
#define RTK_FC_TABLESIZE_DMAAFTACTION_RSV	(1<<RTK_FC_DMAAFTACTION_RSV_SIZE_BITS) //16
#define RTK_FC_DMAAFTACTION_SIZE_BITS	6
#define RTK_FC_TABLESIZE_DMAAFTACTION	(1<<RTK_FC_DMAAFTACTION_SIZE_BITS) //64
#if defined(CONFIG_FC_RTL9607F_SERIES)
#define RTK_FC_DMAAFTMAP_SIZE_BITS	6
#define RTK_FC_TABLESIZE_DMAAFTMAP_RSV	2		//reserve by rtk_ni_init_tx_dma_lso
#define RTK_FC_TABLESIZE_DMAAFTMAP		(1<<RTK_FC_DMAAFTMAP_SIZE_BITS) //64
#endif
#define RTK_FC_TABLESIZE_POLREMAP		32		//ACL remap pol to pol3
#define RTK_FC_TABLESIZE_STREAMID		256
#endif

#if defined(CONFIG_FC_RTL8198F_SERIES)
#define RTK_FC_DEFAULT_SHORTCUT_FLOW_SIZE	8192	//pure sw flow count
#else
#define RTK_FC_DEFAULT_SHORTCUT_FLOW_SIZE	RTK_FC_MAX_SHORTCUT_FLOW_SIZE	//pure sw flow count
#endif


#define EXTPTBL_RSVED_ENTRY				1	// reserve entry[0] to indicate none extension port setting

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
#define RTK_FTCHECK_GRANULARITY			256		//buckets										// def 8K*8way, 32 timer ticks per flow_sync_period
#define RTK_MHASH_TRF_CHECK_GRANULARITY	(RTK_FTCHECK_GRANULARITY<< fc_db.flowHashWayShift)	// how many flow entries we can at one tick? 256<<3 = 2K
#else
#define RTK_FTCHECK_GRANULARITY			2048	//buckets										// def 32K*2way, 16 timer ticks per flow_sync_period
#define RTK_MHASH_TRF_CHECK_GRANULARITY	(RTK_FTCHECK_GRANULARITY << fc_db.flowHashWayShift) // check 2xRTK_FTCHECK_GRANULARITY mainHash traffic at a time
#endif

#define MAX_FLOW_LUT_IDX_WIDTH			(12+1) // 4k+32
#define MAX_FLOW_IDLESEC_WIDTH			12

#define RTK_FC_NETIF_START_IDX 1
#if defined(CONFIG_FC_CA8277B_SERIES)
// To support interface MC count: MC use netif counter 32~63
#define RTK_FC_MC_HW_NETIF_IDXSHIFT 32
#elif defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
#define RTK_FC_DUAL_NETIF_START_IDX 16
#define RTK_FC_DUAL_CONTROL_IDX(hwNetifIdx) (hwNetifIdx - RTK_FC_DUAL_NETIF_START_IDX)
#define RTK_FC_DUAL_CONTENT_BUFFER_SIZE 64
#define RTK_FC_L3FE_CLOCK_MS 333000
#if !defined(CONFIG_FC_RTL9607F_SERIES)
//TODO: 9607F flow cache mib
#define RTK_FC_FLOW_CACHE_MIB_SIZE_BITS	(11+1) //MAX 2K + 1 bit for invalid
#endif
#define RTK_FC_NIC_TX_WO_HDR(rtskb) ((RTSKB_LEN(rtskb) < (0x1<<11)) && !fc_db.controlFuc.tx_with_hdr_debug)
#endif

#define RTK_FC_FLOWENT_ALIGNBUF		0
#define G3_DEF_DEVID						0
#define G3_FLOW_AGE_STATE_IDLE			HASH_AGING_1		// 1
#define G3_FLOW_AGE_STATE_ALIVE			HASH_AGING_6		// 6
#define G3_FLOWIDX_INVALID					0xFFFFFFFF
#define RTK_FC_UINT32_INVALID 0xFFFFFFFF

#define METER_RATE_MIN 0
#ifndef RT_RATE_HW_IDX_L3_METER_RATE_MAX
#define METER_RATE_MAX 32767999
#else
#define METER_RATE_MAX RT_RATE_HW_IDX_L3_METER_RATE_MAX //Note. If rate >= 32767999, disable l3 policer and rate will be 32767999
#endif
#ifndef RT_RATE_HW_IDX_L3_METER_BURST_MAX
#define METER_BURST_MAX 0XFFF
#else
#define METER_BURST_MAX RT_RATE_HW_IDX_L3_METER_BURST_MAX
#endif


#if defined(CONFIG_FC_CAG3_SERIES)
/*
***valid when CONFIG_COMMON_RT_API disabled*** (CONFIG_COMMON_RT_API enabled: RT meter decide the hw index)===
FC: control L3 policer only
===L3 policers for rate limiting  
Policer[320:351]: host policing (rate limit)
Policer[352:383]: ACL rate limiting
Policer[384:415]: flow meter
===L3 policers for counting===
Policer[416:447]: flow mib
Policer[448:479]: host policing rx (pure logging)
Policer[480:511]: host policing tx (pure logging)
*/
#define G3_FLOW_POLICER_FLOWMTR_SIZE		RTK_FC_TABLESIZE_FBMTR	//the number of flow policers used for flow meter
#define G3_FLOW_POLICER_HOSTPOLICING_SIZE	RTK_FC_TABLESIZE_FBMTR	//the number of flow policers used for host policing
#define G3_FLOW_POLICER_ACL_SIZE			RTK_FC_TABLESIZE_FBMTR  //the number of flow policers used for acl rate limit
#define G3_FLOW_POLICER_FLOWMIB_SIZE		RTK_FC_TABLESIZE_FBMTR	//the number of flow policers used for flow mib
#define G3_FLOW_POLICER_HOSTLOGGINGRX_SIZE	RTK_FC_TABLESIZE_FBMTR	//the number of flow policers used for host policing rx (pure logging)
#define G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE	RTK_FC_TABLESIZE_FBMTR	//the number of flow policers used for host policing tx (pure logging)
#define G3_FLOW_POLICER_MC_NETIF_RXMIB_SIZE	0						//the number of flow policers used for MC netif mib
#define G3_FLOW_POLICER_HASH_DFT_DROP_SIZE	0						//the number of flow policers used for HASH default action drop
#define G3_FLOW_POLICER_HASH_DFT_MISS_SIZE	0						//the number of flow policers used for HASH default action miss (if flow miss meter enabled.)

#elif defined(CONFIG_FC_CA8277B_SERIES)
/*
FC: control L3 policer_[1-3] only
===L3 policer_1===
PON gemid
===L3 policer_2===
Policer[0:7]: ACL rate limiting
Policer[32:63]: flow meter + flow mib (flow_meter_mib_conf_dependence always 1)
===L3 policer_3===
host logging only (not support HW host rate limiting)
*/
#define G3_FLOW_POLICER_FLOWMTR_SIZE		0						//flow meter use policer 2, not use policer1
#define G3_FLOW_POLICER_HOSTPOLICING_SIZE	0						//not support HW host rate limiting
#define G3_FLOW_POLICER_ACL_SIZE			0  						//ACL rate limiting use policer 2, not policer1
#define G3_FLOW_POLICER_FLOWMIB_SIZE		0						//flow mib use policer 3, not use policer1
#define G3_FLOW_POLICER_HOSTLOGGINGRX_SIZE	0						//host logging use policer 3, not use policer1
#define G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE	0						//host logging use policer 3, not use policer1

#define G3_L3_FLOW_POLICER2_SIZE				(CA_AAL_L3_MAX_FLOW2_ID + 1)
#define G3_L3_FLOW_POLICER3_SIZE				(CA_AAL_L3_MAX_FLOW3_ID + 1)
#define G3_FLOW_POLICER_MC_NETIF_RXMIB_SIZE	0						//the number of flow policers used for MC netif mib
#define G3_FLOW_POLICER_HASH_DFT_DROP_SIZE	0						//the number of flow policers used for HASH default action drop
#define G3_FLOW_POLICER_HASH_DFT_MISS_SIZE	0						//the number of flow policers used for HASH default action miss (if flow miss meter enabled.)

#elif defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
/*
FC: control L3 policer_[1-3] only
===L3 policer_1===
host host rate limiting + logging
MC netif mib
===L3 policer_2===
Policer[0:31]: flow meter + flow mib (if flow_meter_mib_conf_dependence is enabled)
Policer[32:63]: acl mib
===L3 policer_3===
Policer[0:31]: ACL rate limiting
Policer[32:63]: flow mib (if flow_meter_mib_conf_dependence is disabled)
*/
#define G3_FLOW_POLICER_FLOWMTR_SIZE		0								//flow meter use policer 2, not use policer1
#define G3_FLOW_POLICER_HOSTPOLICING_SIZE	RT_RATE_HOSTPOLICING_TABLE_SIZE	//the number of flow policers used for host policing
#define G3_FLOW_POLICER_ACL_SIZE			0  								//ACL rate limiting use policer 3, not policer1
#define G3_FLOW_POLICER_FLOWMIB_SIZE		0  								//flo mib use policer 3, not policer1
#define G3_FLOW_POLICER_HOSTLOGGINGRX_SIZE	RT_RATE_HOSTPOLICING_TABLE_SIZE	//the number of flow policers used for host policing rx (pure logging)
#define G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE	RT_RATE_HOSTPOLICING_TABLE_SIZE	//the number of flow policers used for host policing tx (pure logging)

#define G3_L3_FLOW_POLICER2_SIZE			(CA_AAL_L3_MAX_FLOW2_ID + 1)
#define G3_L3_FLOW_POLICER3_SIZE			(CA_AAL_L3_MAX_FLOW3_ID + 1)
#define G3_FLOW_POLICER_MC_NETIF_RXMIB_SIZE	32								//the number of flow policers used for MC netif mib
#define G3_FLOW_POLICER_HASH_DFT_DROP_SIZE	1								//the number of flow policers used for HASH default action drop
#define G3_FLOW_POLICER_HASH_DFT_MISS_SIZE	1								//the number of flow policers used for HASH default action miss (if flow miss meter enabled.)

#elif defined(CONFIG_FC_RTL8198F_SERIES)
/*
FC: control L3 policer only: G3 lite only has 128 L3 policers (256 L2 policers)
===L3 policers for rate limiting ***valid when CONFIG_COMMON_RT_API disabled*** (CONFIG_COMMON_RT_API enabled: RT meter decide the hw index)===
Policer[0:31]: flow meter
Policer[32:47]: host policing (rate limit)
Policer[48:63]: ACL rate limiting
===L3 policers for counting===
Policer[64:95]: flow mib
Policer[96:111]: host policing rx (pure logging)
Policer[112:127]: host policing tx (pure logging)
 */
#define G3_FLOW_POLICER_FLOWMTR_SIZE		RTK_FC_TABLESIZE_FBMTR			//the number of flow policers used for flow meter
#define G3_FLOW_POLICER_HOSTPOLICING_SIZE	RT_RATE_HOSTPOLICING_TABLE_SIZE	//the number of flow policers used for host policing
#define G3_FLOW_POLICER_ACL_SIZE			16								//the number of flow policers used for acl rate limit
#define G3_FLOW_POLICER_FLOWMIB_SIZE		RTK_FC_TABLESIZE_FBMTR			//the number of flow policers used for flow mib
#define G3_FLOW_POLICER_HOSTLOGGINGRX_SIZE	RT_RATE_HOSTPOLICING_TABLE_SIZE	//the number of flow policers used for host policing rx (pure logging)
#define G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE	RT_RATE_HOSTPOLICING_TABLE_SIZE	//the number of flow policers used for host policing tx (pure logging)
#define G3_FLOW_POLICER_MC_NETIF_RXMIB_SIZE	0								//the number of flow policers used for MC netif mib
#define G3_FLOW_POLICER_HASH_DFT_DROP_SIZE	0								//the number of flow policers used for HASH default action drop
#define G3_FLOW_POLICER_HASH_DFT_MISS_SIZE	0								//the number of flow policers used for HASH default action miss (if flow miss meter enabled.)

#endif
#define G3_L2_FLOW_POLICER_SIZE				(CA_AAL_MAX_FLOW_ID + 1)
#define G3_L3_FLOW_POLICER_SIZE				(CA_AAL_L3_MAX_FLOW_ID + 1)
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
#define G3_L3_FLOW2_POLICER_SIZE			(CA_AAL_L3_MAX_FLOW2_ID + 1)
#define G3_L3_FLOW3_POLICER_SIZE			(CA_AAL_L3_MAX_FLOW3_ID + 1)
#endif


#if defined(CONFIG_FC_CAG3_SERIES)
#define G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR	(G3_L3_FLOW_POLICER_SIZE - G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE - G3_FLOW_POLICER_HOSTLOGGINGRX_SIZE - G3_FLOW_POLICER_FLOWMIB_SIZE - G3_FLOW_POLICER_FLOWMTR_SIZE - G3_FLOW_POLICER_ACL_SIZE - G3_FLOW_POLICER_HOSTPOLICING_SIZE)
//#define G3_FLOW_POLICER_IDXSHIFT_ACL		(G3_L3_FLOW_POLICER_SIZE - G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE - G3_FLOW_POLICER_HOSTLOGGINGRX_SIZE - G3_FLOW_POLICER_FLOWMIB_SIZE - G3_FLOW_POLICER_FLOWMTR_SIZE - G3_FLOW_POLICER_ACL_SIZE)
#define G3_FLOW_POLICER_IDXSHIFT_FLOWMTR	(G3_L3_FLOW_POLICER_SIZE - G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE - G3_FLOW_POLICER_HOSTLOGGINGRX_SIZE - G3_FLOW_POLICER_FLOWMIB_SIZE - G3_FLOW_POLICER_FLOWMTR_SIZE)
#elif defined(CONFIG_FC_CA8277B_SERIES)
#define G3_FLOW_POLICER_IDXSHIFT_FLOWMTR	32 //use policer2 32~63 (first 8 entries may be used by CLS grp_policer action)
#define G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR	(G3_L3_FLOW_POLICER_SIZE - G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE - G3_FLOW_POLICER_HOSTLOGGINGRX_SIZE - G3_FLOW_POLICER_FLOWMIB_SIZE - G3_FLOW_POLICER_ACL_SIZE - G3_FLOW_POLICER_HOSTPOLICING_SIZE)
//#define G3_FLOW_POLICER_IDXSHIFT_ACL		(G3_L3_FLOW_POLICER_SIZE - G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE - G3_FLOW_POLICER_HOSTLOGGINGRX_SIZE - G3_FLOW_POLICER_FLOWMIB_SIZE - G3_FLOW_POLICER_ACL_SIZE)
#else
#define G3_FLOW_POLICER_IDXSHIFT_FLOWMTR	0 // should not change (because flow meter is indicated by skb->mark)
#define G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR	(G3_L3_FLOW_POLICER_SIZE - G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE - G3_FLOW_POLICER_HOSTLOGGINGRX_SIZE - G3_FLOW_POLICER_FLOWMIB_SIZE - G3_FLOW_POLICER_ACL_SIZE - G3_FLOW_POLICER_HOSTPOLICING_SIZE)
//#define G3_FLOW_POLICER_IDXSHIFT_ACL		(G3_L3_FLOW_POLICER_SIZE - G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE - G3_FLOW_POLICER_HOSTLOGGINGRX_SIZE - G3_FLOW_POLICER_FLOWMIB_SIZE - G3_FLOW_POLICER_ACL_SIZE)
#endif

#if defined(CONFIG_FC_CA8277B_SERIES)
#if 0 // 8277B support support flow_meter_mib_conf_dependence is 1 only
#define G3_FLOW_POLICER_IDXSHIFT_FLOWMIB	32	// 8277B: flow mib use policer2, use last 32 entries. (first 8 entries may be used by CLS grp_policer action)
#endif
#define G3_FLOW_POLICER_IDXSHIFT_HPLOGRX	0																		// use policer3 entry 0~31
#define G3_FLOW_POLICER_IDXSHIFT_HPLOGTX	(G3_FLOW_POLICER_IDXSHIFT_HPLOGRX + RT_RATE_HOSTPOLICING_TABLE_SIZE)	// use policer3 entry 32~63
#elif defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
// 8277C Policer3[32:63]: flow mib (if flow_meter_mib_conf_dependence is disabled)
#define G3_FLOW_POLICER_IDXSHIFT_FLOWMIB	32
#define G3_FLOW_POLICER_IDXSHIFT_ACLMIB	32
#define G3_FLOW_POLICER_IDXSHIFT_HASH_DFT_MISS (G3_FLOW_POLICER_IDXSHIFT_FLOWMTR)
#define G3_FLOW_POLICER_IDXSHIFT_HASH_DFT_DROP (G3_L3_FLOW_POLICER_SIZE - G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE - G3_FLOW_POLICER_HOSTLOGGINGRX_SIZE - G3_FLOW_POLICER_MC_NETIF_RXMIB_SIZE - G3_FLOW_POLICER_HASH_DFT_DROP_SIZE)
#define G3_FLOW_POLICER_IDXSHIFT_MC_NETIF_RXMIB	(G3_L3_FLOW_POLICER_SIZE - G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE - G3_FLOW_POLICER_HOSTLOGGINGRX_SIZE - G3_FLOW_POLICER_MC_NETIF_RXMIB_SIZE)
#define G3_FLOW_POLICER_IDXSHIFT_HPLOGRX		(G3_L3_FLOW_POLICER_SIZE - G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE - G3_FLOW_POLICER_HOSTLOGGINGRX_SIZE)
#define G3_FLOW_POLICER_IDXSHIFT_HPLOGTX		(G3_L3_FLOW_POLICER_SIZE - G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE)
#else
#define G3_FLOW_POLICER_IDXSHIFT_FLOWMIB	(G3_L3_FLOW_POLICER_SIZE - G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE - G3_FLOW_POLICER_HOSTLOGGINGRX_SIZE - G3_FLOW_POLICER_FLOWMIB_SIZE)
#define G3_FLOW_POLICER_IDXSHIFT_HPLOGRX	(G3_L3_FLOW_POLICER_SIZE - G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE - G3_FLOW_POLICER_HOSTLOGGINGRX_SIZE)
#define G3_FLOW_POLICER_IDXSHIFT_HPLOGTX	(G3_L3_FLOW_POLICER_SIZE - G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE)
#endif
/*
	loopback mode: use	L2 policer
	[RT meter enable]
	0~158: RT meter
	128~159: host policing
	160~223: host TRX logging
	224~: other (dos)
	[RT meter disable]
	128~159: host rate limit
	160~223: host TRX logging
	223~: other (dos)
*/
#define G3_LOOPBACKMODE_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR	128
#define G3_LOOPBACKMODE_FLOW_POLICER_IDXSHIFT_HPLOGRX	160											//index 159~190
#define G3_LOOPBACKMODE_FLOW_POLICER_IDXSHIFT_HPLOGTX	(160 + G3_FLOW_POLICER_HOSTLOGGINGRX_SIZE)	//infdex 191~222

#if defined(CONFIG_FC_CAG3_SERIES)
/*
reserved 32 policer for flow meter
RT meter(virtual index)--->		RT meter(hw index)--->	real index (be shifted by FC)
0								0							384
1								1							385
...
31								31							415
This solution is for flow_meter_mib_conf_dependence is enableing.
policer counter may be impact by L2 policer index (or defaut value 0)
*/
#define G3_L3_FLOW_POLICER_SIZE_MIB			(G3_FLOW_POLICER_FLOWMIB_SIZE + G3_FLOW_POLICER_HOSTLOGGINGRX_SIZE + G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE + G3_FLOW_POLICER_FLOWMTR_SIZE)	//# L3 policer for counting
#else
#define G3_L3_FLOW_POLICER_SIZE_MIB			(G3_FLOW_POLICER_FLOWMIB_SIZE + G3_FLOW_POLICER_HOSTLOGGINGRX_SIZE + G3_FLOW_POLICER_HOSTLOGGINGTX_SIZE + G3_FLOW_POLICER_MC_NETIF_RXMIB_SIZE)	//# L3 policer for counting
#endif
#define G3_L3_FLOW_POLICER_SIZE_METER		(G3_L3_FLOW_POLICER_SIZE - G3_L3_FLOW_POLICER_SIZE_MIB - G3_FLOW_POLICER_HASH_DFT_DROP_SIZE - G3_FLOW_POLICER_HASH_DFT_MISS_SIZE)	//# L3 policer for metering
#endif


#define RG_INVALID_PORT(port) (((1ULL<<port) & RTK_FC_ALL_MAC_PORTMASK)==0x0)
#define RG_INVALID_MAC_PORT(port) (((1ULL<<port) & RTK_FC_ALL_MAC_PORTMASK)==0x0)
#define RG_INVALID_PORTMASK(portMsk) RG_INVALID_MAC_PORTMASK(portMsk)
#define RG_INVALID_MAC_PORTMASK(portMsk) ((portMsk & RTK_FC_ALL_MAC_PORTMASK)!=portMsk)

#define MAX_AUTOEXT_PKT_SIZE		100
#define MAX_SW_FRAGIPID_SIZE		8

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
#define VLAN_CFI_SHIFT		12
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
#else
#define VLAN_PRIO_MASK          0xe000 /* Priority Code Point */
#define VLAN_PRIO_SHIFT         13
#define VLAN_CFI_MASK           0x1000 /* Canonical Format Indicator */
#define VLAN_TAG_PRESENT        VLAN_CFI_MASK
//#define VLAN_VID_MASK           0x0fff /* VLAN Identifier */
#define VLAN_N_VID              4096
#endif

#if !defined(VLAN_CFI_SHIFT)
#define VLAN_CFI_SHIFT 12
#endif

#ifndef __IRAM_HWNAT
#define __IRAM_HWNAT			__attribute__ ((section(".iram")))
#endif

#define RTK_FC_API_MODULE_NAME	"RTK_FC_API_MODULE"
#define	RTK_FC_TIMER_TICK_SECOND	2
#define	RTK_FC_RTNL_EVENT_TICK_MSECONDMS		5000	//ms
#define	RTK_FC_DYNAMIC_PREHASHPTN_EVENT_TICK_MSECONDMS		2000	//ms
#define	RTK_FC_IGMP_DUMYPKT_TICK_MSECONDMS		2000	//ms
#define	RTK_FC_IGMP_KERNEL_SHORT_TICK_MSECONDMS		200	//ms
#define	RTK_FC_IGMP_KERNEL_LONG_TICK_MSECONDMS		10000	//ms
#define	RTK_FC_SW_RATE_LIMIT_JIFFIES			(CONFIG_HZ/2)
#define	RTK_FC_SW_SHAPING_JIFFIES				(CONFIG_HZ/5)	// 0.2 second
#define	RTK_FC_SW_SHAPING_TIMER_TICK			(CONFIG_HZ/100)	// 0.01 second
#define	RTK_FC_SW_SHAPING_BURST_BYTES			10240	// acceptable packet bytes more than expect bytes per burst
#define DEFAULT_MCGROUP_TRAP_TIMEOUT 450
#define RTK_FC_DYNAMIC_PREHASHPTN_JIFFIES (CONFIG_HZ*60) // 60 second
#define RTK_FC_RESERVED_DMAC_SIZE	8

#define ETH_P_8021AD	0x88A8		/* 802.1ad Service VLAN		*/

#define IPV_NONE	0x0
#define IPV_IPV4	0x1
#define IPV_IPV6	0x2

#define ICMPV6_NS	135
#define ICMPV6_NA	136

//vlan group mac limit
#define MAX_VLAN_GROUP_MAC_LIMIT_NUMBER 32


#if defined(CONFIG_RTK_L34_XPON_PLATFORM) || defined(CONFIG_RTK_L34_G3_PLATFORM)
/*8198f only has 128 L3 policers, thus reduce its host policing table size to 16*/
#if defined(CONFIG_FC_RTL8198F_SERIES)
#define HOST_POLICING_METER_SIZE	16	//only used when RT API disabled
#else
#define HOST_POLICING_METER_SIZE	RTK_FC_TABLESIZE_SHAREMTR //only used when RT API disabled
#endif
#endif

#define RTK_FC_DYNAMIC_PREHASHPTH_KEEP_FLOW_LEARNING 0
#define RTK_FC_DYNAMIC_PREHASHPTH_SKIP_FLOW_LEARNING 1
#define RTK_FC_DYNAMIC_PREHASHPTH_FAILED 2

#define RTK_PER_HASH_BUCKET_SWFLOW_LIST_COUNT									(1 << fc_db.swflowlistBucketExtendShift)
#define RTK_SW_FLOW_LIST_HEAD_COUNT												(fc_db.flowHashBuckets << fc_db.swflowlistBucketExtendShift)
#define RTK_FOR_EACH_SW_FLOW_LIST_IN_BUCKET(sw_flow_list_idx, hash_bucket_idx)	for(sw_flow_list_idx = (hash_bucket_idx << fc_db.swflowlistBucketExtendShift); sw_flow_list_idx < ((hash_bucket_idx << fc_db.swflowlistBucketExtendShift) + RTK_PER_HASH_BUCKET_SWFLOW_LIST_COUNT); sw_flow_list_idx++)
#define RTK_HASH_VAL_TO_SW_FLOW_LIST_IDX(flow_hash)								(flow_hash >> (fc_db.flowHashWayShift-fc_db.swflowlistBucketExtendShift))
#if defined(CONFIG_RTK_L34_XPON_PLATFORM) || defined(CONFIG_FC_CA8277AB_SERIES)
#define RTK_FLOW_IDX_TO_FLOW_HASH_IDX(flow_ent_idx)								(0)	// not support
#else
#define RTK_FLOW_IDX_TO_FLOW_HASH_IDX(flow_ent_idx)								(fc_db.flowTbl[flow_ent_idx].crc16)
#endif

#define FLOW_MULTICAST_RESV1(addr)	((((uint32)(addr)) & 0xFFFFFF00) == 0xe0000000)	// 224.0.0.x
#define RESERVE_MULTICAST_SSDP 	0xEFFFFFFA	////239.255.255.250
#define FLOW_MULTICAST_SSDP(addr)	((((uint32)(addr)) & 0xFFFFFFFF) == RESERVE_MULTICAST_SSDP)	//239.255.255.250

/*
refer to rfc4291
Reserved Multicast Addresses:   FF0X:0:0:0:0:0:0:0
The above multicast addresses are reserved and shall never be assigned to any multicast group.
*/
#define FLOW_MULTICAST6_RESV1_FFOX(addr)   	 ((((uint32)(addr)) & 0xFFF00000) == 0xFF000000)
#define FLOW_MULTICAST6_DATA_NOT_FFXE(addr)  (((((uint32)(addr)) & 0xFF000000) == 0xFF000000)&&((((uint32)(addr)) & 0xFF0F0000) != 0xFF0E0000))
#define FLOW_MULTICAST_RESV_IPV4_ALL(addr)	(FLOW_MULTICAST_RESV1(addr) || FLOW_MULTICAST_SSDP(addr))
#define FLOW_MULTICAST_RESV_IPV6_ALL(addr)	(FLOW_MULTICAST6_RESV1_FFOX(addr))
#define FLOW_IPV6_MULTICAST_ADDRESS(addr)  	 ((((uint32)(addr)) & 0xFF000000) == 0xFF000000)
#define FLOW_IPV4_MULTICAST_ADDRESS(addr)	 (IS_CLASSD_ADDR(addr))

#if defined(CONFIG_RTK_FC_SW_ACK_DELAY_WIDTH) && (CONFIG_RTK_FC_SW_ACK_DELAY_WIDTH > 5)
#define RTK_FC_ACKDELAY_ENTRY_WIDTH 		(CONFIG_RTK_FC_SW_ACK_DELAY_WIDTH-1)	//default is 15, 32K
#define RTK_FC_ACKDELAY_ENTRY_SIZE 			(1<<RTK_FC_ACKDELAY_ENTRY_WIDTH)
#define RTK_FC_ACKDELAY_HASH_WIDTH			(CONFIG_RTK_FC_SW_ACK_DELAY_WIDTH-5)	//default is 11, 2K
#define RTK_FC_ACKDELAY_HASH_SIZE			(1<<RTK_FC_ACKDELAY_HASH_WIDTH)
#define RTK_FC_ACKDELAY_HASH_IDX(crc16)			(crc16 >> (16-RTK_FC_ACKDELAY_HASH_WIDTH))
#define RTK_FC_ACKDELAY_HWLKP_LSPID			RTK_FC_MAC_PORT_CPU1
#endif

#define L2HOUSE_KEEP_NUM 256

#define RTK_FC_CT_HASH_INFO_BUCKET_SIZE (1<<8)


//Lut
#define RTK_FC_LUTENTRY_POOL_EXTRA_ENTRY	8		// for LRU
#define RTK_FC_LUTENTRY_TIMEOUT_SECONDS		4096	// refer to omci spec, age time is 1..4095
#define RTK_FC_LUTENTRY_POOL_SIZE 		(RTK_FC_TABLESIZE_LUT+RTK_FC_LUTENTRY_POOL_EXTRA_ENTRY)
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
#define RTK_FC_LUT_HASH_WAY_SHIFT		2
#define RTK_FC_LUT_BUCKET_SIZE_SHIFT		9
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
#define RTK_FC_LUT_HASH_WAY_SHIFT		3
#define RTK_FC_LUT_BUCKET_SIZE_SHIFT		9
#else
#error
#endif

// Table List Bucket Size
#define RTK_FLOWBASE_BUCKETSIZE_INDMAC		32
#define RTK_FC_LUT_HASH_WAY_SIZE			(1<<RTK_FC_LUT_HASH_WAY_SHIFT)
#define RTK_FC_LUT_BUCKET_SIZE			(1<<RTK_FC_LUT_BUCKET_SIZE_SHIFT)

#define RTK_FC_WAN_ACCESS_IP_BUCKET_SIZE_SHIFT		8
#define RTK_FC_WAN_ACCESS_IP_BUCKET_SIZE	(1<<RTK_FC_WAN_ACCESS_IP_BUCKET_SIZE_SHIFT)

#define DEFAULT_FID 0U

#define CMD_BUFF_PROC_LEN 64
#define CMD_BUFF_SIZE 256
#define CMD_BUFF_SIZE_MAX 512
#define CMD_PARAM_COUNT 16

#define FORCE_STATIC	2U

#define RTK_FC_MAX_LUT_HW_HASH_HEAD	(RTK_FC_TABLESIZE_LUT>>RTK_FC_LUT_HASH_WAY_SHIFT)

typedef enum rtk_fc_flow_deley_mode_e
{
	RTK_FC_SHORTCUT_DELEY_MODE = 0,		// deley to hw flow, but add shortcut flow
	RTK_FC_KERNEL_DELEY_MODE,			// deley to both sw and hw flow
}rtk_fc_flow_deley_mode_t;

#define RTK_FC_HIT_NUMBER_TO_DELAY_MAX ((1<<RTK_FC_HIT_NUMBER_TO_DELAY_BITS) - 1)
#define RTK_FC_HIT_NUMBER_TO_DELAY_BITS 6

#define RTK_FC_DUAL_PASSTHROUGH_FLOWMAPPING_SIZE_BITS	8
#define RTK_FC_DUAL_PASSTHROUGH_FLOWMAPPING_SIZE		((1<<RTK_FC_DUAL_PASSTHROUGH_FLOWMAPPING_SIZE_BITS))	//index 0: reserve one for non passthrough flow
#define RTK_FC_DUAL_PT_TBL_INVALID_IDX 0
#define RTK_FC_DUAL_PT_TBL_START_IDX (RTK_FC_DUAL_PT_TBL_INVALID_IDX+1)

#define RTK_FC_TABLESIZE_I6NAT_MAPPING_TABLE_SHIFT_BITS 6
#define RTK_FC_TABLESIZE_I6NAT_MAPPING_TABLE (1<<RTK_FC_TABLESIZE_I6NAT_MAPPING_TABLE_SHIFT_BITS)
#define RTK_FC_TABLESIZE_L2DUAL_TABLE_SHIFT_BITS 6
#define RTK_FC_TABLESIZE_L2DUAL_TABLE (1<<RTK_FC_TABLESIZE_L2DUAL_TABLE_SHIFT_BITS)
#define RTK_FC_TABLESIZE_I6MAPT_MAPPING_TABLE_SHIFT_BITS 6
#define RTK_FC_TABLESIZE_I6MAPT_MAPPING_TABLE (1<<RTK_FC_TABLESIZE_I6MAPT_MAPPING_TABLE_SHIFT_BITS)
#define RTK_FC_L2DUAL_CONTENTBUFFER_SIZE 86 // Max: outer-eth, outer-svlan, outer-cvlan, outer-ppph, outer-iph, outer-udph, outer-vxlan
#define RTK_FC_I6NAT_TABLE_START_INDEX 1
#define RTK_FC_L2DUAL_TABLE_START_INDEX 1
#define RTK_FC_L2DUAL_TABLE_INVALID_IDX 0
#define RTK_FC_I6MAPT_TABLE_START_INDEX 1

#define RTK_FC_VXLAN_EXTRA_MAX_NUM 8

#define RTK_FC_FASTFORWARD_FAKE_VLAN_TYPE_NONE 0
#define RTK_FC_FASTFORWARD_FAKE_VLAN_TYPE_V6NAT 1
#define RTK_FC_FASTFORWARD_FAKE_VLAN_TYPE_VXLAN_UPSTREAM 2
#define RTK_FC_FASTFORWARD_FAKE_VLAN_TYPE_VXLAN_DOWNSTREAM 3

//vxlan acceleration mechanism
#define RTK_FC_VXLAN_ACC_CPU_PORTMASK RTK_FC_ALL_MAC_CPU_PORTMASK
#define RTK_FC_VXLAN_ACC_UPSTREAM_EXTPMAK (0x1<<RTK_FC_MAC_EXT_PORT3)
#define RTK_FC_VXLAN_ACC_DOWNSTREAM_EXTPMAK (0x1<<RTK_FC_MAC_EXT_PORT4)
#define RTK_FC_VXLAN_ACC_CPU_EXT_PORTMASK (RTK_FC_VXLAN_ACC_UPSTREAM_EXTPMAK|RTK_FC_VXLAN_ACC_DOWNSTREAM_EXTPMAK)
#define RTK_FC_VXLAN_ACC_EPHEMERAL_SPORT	49152
#define RTK_FC_VXLAN_ACC_EXTRAPMSK			0xe
#define RTK_FC_VXLAN_ACC_EXTRAPMSK_RATE		217784
#define RTK_FC_VXLAN_ACC_EXTRAPMSK_RATE_FOR_VA	195000
#define RTK_FC_VXLAN_ACC_EXTRAPMSK_METER	24

#define RX_OFFSET 2
#define RTK_FC_SPECIAL_FASTFWD_MODE_VXLAN 0
#define RTK_FC_SPECIAL_FASTFWD_MODE_NPTV6 1


//nptv6 acceleration mechanism
#if 1// defined(CONFIG_CMCC) //CMCC
#define MAX_NPTV6_ACC_UPSTREAM_BIT_RATE 4194296
#define MAX_NPTV6_ACC_UPSTREAM_LAN0_BIT_RATE 465000 // original 468753
#define MAX_NPTV6_ACC_UPSTREAM_LAN0_BIT_RATE_FOR_RL6831 468753 // original 468753
#define MAX_NPTV6_ACC_UPSTREAM_LAN1_BIT_RATE (253600*(102+20)*8/1000) //247513
#define MAX_NPTV6_ACC_UPSTREAM_LAN2_BIT_RATE MAX_NPTV6_ACC_UPSTREAM_LAN0_BIT_RATE
#define MAX_NPTV6_ACC_UPSTREAM_LAN2_BIT_RATE_FOR_RL6831 MAX_NPTV6_ACC_UPSTREAM_LAN0_BIT_RATE_FOR_RL6831
#define MAX_NPTV6_ACC_UPSTREAM_LAN3_BIT_RATE (107000*(102+20)*8/1000) //104432
#define MAX_NPTV6_ACC_UPSTREAM_LAN3_BIT_RATE_FOR_RL6831 130000 //104432
#define MAX_NPTV6_ACC_DOWNSTREAM_BIT_RATE (670100*(102+20)*8/1000) // 654017

#elif defined(CONFIG_YUEME)

#define MAX_NPTV6_ACC_UPSTREAM_BIT_RATE 4194296
#define MAX_NPTV6_ACC_UPSTREAM_LAN0_BIT_RATE ((330000)*(106+20)*8/1000)//(310000*(102+20)*8/1000) // 468753
#define MAX_NPTV6_ACC_UPSTREAM_LAN1_BIT_RATE ((276537)*(106+20)*8/1000)//MAX_NPTV6_ACC_UPSTREAM_LAN0_BIT_RATE//(253600*(102+20)*8/1000) //247513
#define MAX_NPTV6_ACC_UPSTREAM_LAN2_BIT_RATE ((330000)*(106+20)*8/1000)//MAX_NPTV6_ACC_UPSTREAM_LAN0_BIT_RATE
#define MAX_NPTV6_ACC_UPSTREAM_LAN3_BIT_RATE ((330000)*(106+20)*8/1000) //104432
#define MAX_NPTV6_ACC_DOWNSTREAM_BIT_RATE (670100*(106+20)*8/1000) // 654017

#else //For making compiler happy
#define MAX_NPTV6_ACC_UPSTREAM_BIT_RATE 4194296
#define MAX_NPTV6_ACC_UPSTREAM_LAN0_BIT_RATE (480280*(102+20)*8/1000) // 468753
#define MAX_NPTV6_ACC_UPSTREAM_LAN1_BIT_RATE (253600*(102+20)*8/1000) //247513
#define MAX_NPTV6_ACC_UPSTREAM_LAN2_BIT_RATE MAX_NPTV6_ACC_UPSTREAM_LAN0_BIT_RATE
#define MAX_NPTV6_ACC_UPSTREAM_LAN3_BIT_RATE (107000*(102+20)*8/1000) //104432
#define MAX_NPTV6_ACC_DOWNSTREAM_BIT_RATE (670100*(102+20)*8/1000) // 654017



#endif
#define MAX_NPTV6_ACC_2LAN_PON_WAN_UPSTREAM_LAN0_BIT_RATE 937506 //960554 * (102+20)*8/1000
#define MAX_NPTV6_ACC_2LAN_PON_WAN_UPSTREAM_LAN1_BIT_RATE 351945 //358607 * (102+20)*8/1000
#define MAX_NPTV6_ACC_2LAN_PON_WAN_UPSTREAM_LAN2_BIT_RATE MAX_NPTV6_ACC_2LAN_PON_WAN_UPSTREAM_LAN0_BIT_RATE
#define MAX_NPTV6_ACC_2LAN_PON_WAN_UPSTREAM_LAN3_BIT_RATE MAX_NPTV6_ACC_2LAN_PON_WAN_UPSTREAM_LAN1_BIT_RATE
#define MAX_NPTV6_ACC_2LAN_PON_WAN_DOWNSTREAM_BIT_RATE 654017 //(670100*(102+20)*8/1000)

#define MAX_NPTV6_ACC_2LAN_ETHERNET_WAN_UPSTREAM_LAN0_BIT_RATE 4194296
#define MAX_NPTV6_ACC_2LAN_ETHERNET_WAN_UPSTREAM_LAN1_BIT_RATE 30000
#define MAX_NPTV6_ACC_2LAN_ETHERNET_WAN_UPSTREAM_LAN2_BIT_RATE MAX_NPTV6_ACC_2LAN_ETHERNET_WAN_UPSTREAM_LAN0_BIT_RATE
#define MAX_NPTV6_ACC_2LAN_ETHERNET_WAN_UPSTREAM_LAN3_BIT_RATE MAX_NPTV6_ACC_2LAN_ETHERNET_WAN_UPSTREAM_LAN1_BIT_RATE
#define MAX_NPTV6_ACC_2LAN_ETHERNET_WAN_DOWNSTREAM_BIT_RATE 900000



#define MAX_NPTV6_OUTER_LEN 64
#define MAX_NPTV6_ACC_UPSTREAM_NIC_FLOW_NUM 4
#define MAX_NPTV6_ACC_DOWNSTREAM_NIC_FLOW_NUM 4
#define MAX_NPTV6_ACC_UPSTREAM0_NIC_FLOW_NUM 2
#define MAX_NPTV6_ACC_DOWNSTREAM0_NIC_FLOW_NUM 2
#define MAX_NPTV6_ACC_UPSTREAM_SIZE 32
#define MAX_NPTV6_ACC_DOWNSTREAM_SIZE 32
#define MAX_NPTV6_ACC_UPSTREAM0_SIZE 16
#define MAX_NPTV6_ACC_DOWNSTREAM0_SIZE 16

#define FC_NPTV6_ACC_MAC_CPU_PORTMASK RTK_FC_ALL_MAC_CPU_PORTMASK
#define FC_NPTV6_ACC_UPSTREAM_MAC_CPU_EXT_PORTMASK (0x1<<RTK_FC_MAC_EXT_PORT1)
#define FC_NPTV6_ACC_DOWNSTREAM_MAC_CPU_EXT_PORTMASK (0x1<<RTK_FC_MAC_EXT_PORT2)
#define FC_NPTV6_ACC_MAC_CPU_EXT_PORTMASK (FC_NPTV6_ACC_UPSTREAM_MAC_CPU_EXT_PORTMASK | FC_NPTV6_ACC_DOWNSTREAM_MAC_CPU_EXT_PORTMASK)




#define RTK_FC_V6NAT_DIRECTION_NONE 0
#define RTK_FC_V6NAT_DIRECTION_UPSTREAM 1
#define RTK_FC_V6NAT_DIRECTION_DOWNSTREAM 2

#define RTK_FC_VXLAN_DS_FF_NUM_MAX 4
#define RTK_FC_VXLAN_US_FF_NUM_MAX 4
// default bits definition of remarking function
#define RTK_FC_RMK_UNDEF 		-1	//relate to: RTK_FC_MGR_RMK_UNDEF
#define RTK_FC_RMK_QID_BIT_DEF 0
#define RTK_FC_RMK_QID_LEN_DEF 3
#define RTK_FC_RMK_STREAMID_BIT_DEF 3
#define RTK_FC_RMK_STREAMID_LEN_DEF 7
#define RTK_FC_RMK_METERIDX_BIT_DEF 10
#define RTK_FC_RMK_METERIDX_LEN_DEF 5
#define RTK_FC_RMK_FWDBYPS_BIT_DEF 15
#define RTK_FC_RMK_FWDBYPS_LEN_DEF 1
#define RTK_FC_RMK_STREAMID_EN_BIT_DEF 16
#define RTK_FC_RMK_STREAMID_EN_LEN_DEF 1
#define RTK_FC_RMK_METERIDX_EN_BIT_DEF 17
#define RTK_FC_RMK_METERIDX_EN_LEN_DEF 1
#define RTK_FC_RMK_SWSHAPER_EN_LEN_DEF 1
#define RTK_FC_RMK_MIBIDX_BIT_DEF 18
#define RTK_FC_RMK_MIBIDX_LEN_DEF 5
#define RTK_FC_RMK_MIBIDX_EN_BIT_DEF 23
#define RTK_FC_RMK_MIBIDX_EN_LEN_DEF 1
#define RTK_FC_RMK_FLOW_CACHE_MIB_EN_BIT_DEF 0
#define RTK_FC_RMK_FLOW_CACHE_MIB_EN_LEN_DEF 1
#define RTK_FC_RMK_SKIPFCEGRESSFUNC_BIT_DEF 24
#define RTK_FC_RMK_SKIPFCEGRESSFUNC_LEN_DEF 1
#define RTK_FC_RMK_PSFLOODFWDACC_LEN_DEF 1
#define RTK_FC_RMK_MAPET_FMR_LEN_DEF 1

//---SKB mark2 vlan setting
#define RTK_FC_RMK_CVLAN_ID_BIT_DEF 2
#define RTK_FC_RMK_CVLAN_ID_LEN_DEF 12
#define RTK_FC_RMK_CVLAN_PRI_BIT_DEF 14
#define RTK_FC_RMK_CVLAN_PRI_LEN_DEF 3
#define RTK_FC_RMK_CVLAN_ACTION_BIT_DEF 17
#define RTK_FC_RMK_CVLAN_ACTION_LEN_DEF 2

#define RTK_FC_RMK_SVLAN_ID_BIT_DEF 19
#define RTK_FC_RMK_SVLAN_ID_LEN_DEF 12
#define RTK_FC_RMK_SVLAN_PRI_BIT_DEF 31
#define RTK_FC_RMK_SVLAN_PRI_LEN_DEF 3
#define RTK_FC_RMK_SVLAN_ACTION_BIT_DEF 34
#define RTK_FC_RMK_SVLAN_ACTION_LEN_DEF 2
#define RTK_FC_RMK_SVLAN_TPID_BIT_DEF 36
#define RTK_FC_RMK_SVLAN_TPID_LEN_DEF 1
#define RTK_FC_RMK_WANDSLOOPBACK_EN_BIT_DEF 0
#define RTK_FC_RMK_WANDSLOOPBACK_EN_LEN_DEF 1
#define RTK_FC_RMK_SWINTFIDX_ASSIGN_EN_BIT_DEF 0
#define RTK_FC_RMK_SWINTFIDX_ASSIGN_EN_LEN_DEF 1
#define RTK_FC_RMK_SWINTFIDX_ASSIGN_INOUT_BIT_DEF 0
#define RTK_FC_RMK_SWINTFIDX_ASSIGN_INOUT_LEN_DEF 1
#define RTK_FC_RMK_SWINTFIDX_ASSIGN_IDX_BIT_DEF 0
#define RTK_FC_RMK_SWINTFIDX_ASSIGN_IDX_LEN_DEF 5
#define RTK_FC_RMK_SKIP_FC_ALG_CHECK_BIT_DEF 0
#define RTK_FC_RMK_SKIP_FC_ALG_CHECK_LEN_DEF 1


// default setting of remarking API
#define RTK_FC_ETHERTYPE_BYPASS_NUM	4
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
#define RTK_FC_ETHERTYPE_DONT_CARE	FB_INPROTOCOL_ALL_ACCEPT
#else
#define RTK_FC_ETHERTYPE_DONT_CARE	0
#endif

#define RTK_FC_TPID_SELECTION_MAX_NUM 2
#define RTK_FC_TPID_FROM_SKBMARK_USE_STAG_0 0
#define RTK_FC_TPID_FROM_SKBMARK_USE_STAG_1 1
#define CVLAN_TPID 0x8100
//+++++++++++++++ ACL Related Define Start +++++++++++++++
//user defined ACL
#define MAX_ACL_SW_ENTRY_SIZE 128

#define RT_ACL_PATTERN_SVLAN_TPID fc_db.systemGlobal.stagTPID[0]
#define RT_ACL_PATTERN_SVLAN_TPID2 fc_db.systemGlobal.stagTPID[1]
#define RT_ACL_PATTERN_CVLAN_TPID 0x8100
#define RT_ACL_PATTERN_IS_STPID_EQ_CTPID ((RT_ACL_PATTERN_SVLAN_TPID == RT_ACL_PATTERN_CVLAN_TPID) || (fc_db.systemGlobal.ifstagTPID1_enabled && (RT_ACL_PATTERN_SVLAN_TPID2 == RT_ACL_PATTERN_CVLAN_TPID)))

#define RT_ACL_ACTION_FORWARD_GROUP_BITS		(RT_ACL_ACTION_FORWARD_GROUP_DROP_BIT|RT_ACL_ACTION_FORWARD_GROUP_TRAP_BIT|RT_ACL_ACTION_FORWARD_GROUP_TRAP_SLAVECPU_BIT|RT_ACL_ACTION_FORWARD_GROUP_TRAP_TO_PS_BIT|RT_ACL_ACTION_FORWARD_GROUP_PERMIT_BIT|RT_ACL_ACTION_FORWARD_GROUP_REDIRECT_BIT)
#define RT_ACL_ACTION_PRIORITY_GROUP_BITS		(RT_ACL_ACTION_PRIORITY_GROUP_TRAP_PRIORITY_BIT|RT_ACL_ACTION_PRIORITY_GROUP_ACL_PRIORITY_BIT)
#define RT_ACL_ACTION_METER_GROUP_BITS			(RT_ACL_ACTION_METER_GROUP_SHARE_METER_BIT)
#define RT_ACL_ACTION_LOGGING_GROUP_BITS		(RT_ACL_ACTION_LOGGING_GROUP_MIB_BIT)

#define RT_ACL_PORT_ALL_LAN_BIT		(fc_db.lanPortMask.portmask)
#define RT_ACL_PORT_ALL_WAN_BIT		(fc_db.wanPortMask.portmask)

#define ARP_TARGET_IP_BYTE_OFFSET (38)

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)

#define MAX_ACL_ENTRY_SIZE 				FLOWBASED_TABLESIZE_ACL
#define MAX_ACL_IPRANGETABLE_SIZE 		FLOWBASED_TABLESIZE_ACL_IPRANGETABLE
#define MAX_ACL_PORTRANGETABLE_SIZE 	FLOWBASED_TABLESIZE_ACL_PORTRANGETABLE
#define MAX_ACL_PACKETRANGETABLE_SIZE 	(FLOWBASED_TABLESIZE_ACL_PKTLENRANGETABLE-1)
#define RSV_ACL_PACKETRANGETABLE_IDX 	(MAX_ACL_PACKETRANGETABLE_SIZE)		//reserve latest packet length range to rsv acl(pptp gre ctrl and lcp)
#define MAX_ACL_TEMPLATE_SIZE 			FLOWBASED_TABLESIZE_ACL_TEMPLATE
#define MAX_ACL_LOG_MIB_NUM 			64	//for 64-bits byte count, index 1,3,5,...,2n+1 is invalid
#define MAX_ACL_MIB_TABLE_SIZE			(RT_STAT_ACLMIB_TABLE_SIZE-1)		//reserve latest mib to trap to slave cpu

#define ACL_PER_RULE_FIELD_SIZE 8
#define GLOBAL_ACL_FIELD_SIZE RTK_ACL_TEMPLATE_END
#define GLOBAL_ACL_RULE_SIZE 8

#define MAX_ACL_TEMPLATE_MULTIPLE_HIT_SIZE 4

#define MIN_ACL_ENTRY_INDEX RESERVED_ACL_BEFORE
#define MAX_ACL_ENTRY_INDEX RESERVED_ACL_AFTER

#define RESERVED_ACL_BEFORE fc_db.aclAndCfReservedRule.aclLowerBoundary
#define RESERVED_ACL_AFTER fc_db.aclAndCfReservedRule.aclUpperBoundary

#endif
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
#define RTK_FC_RT_ACL_PORTMASK_IDX 128
#define RTK_FC_RT_ACL_PORTMASK_INVALID 0xff
#endif


//reserved ACL
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
#if defined (CONFIG_FC_RTL8277C_SERIES) || defined (CONFIG_FC_RTL9607F_SERIES)
#define CLS_T2CTRL_MAINHASH_PROFILE_WAN RTK_ASIC_FLOW_PROFILE_FLOW_5TUPLE
#define CLS_T2CTRL_MAINHASH_PROFILE_LAN RTK_ASIC_FLOW_PROFILE_FLOW_5TUPLE
#define CLS_T2CTRL_MAINHASH_PROFILE_MC RTK_ASIC_FLOW_PROFILE_FLOW_MC
#define CLS_T2CTRL_MAINHASH_PROFILE_2TUPLE_BRIDGE RTK_ASIC_FLOW_PROFILE_FLOW_2TUPLE
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
#define CLS_T2CTRL_MAINHASH_PROFILE_MC_WIFI_TX RTK_ASIC_FLOW_PROFILE_MC_WIFI_TX_AMSDU_OFLD
#else
#define CLS_T2CTRL_MAINHASH_PROFILE_MC_WIFI_TX 0xf
#endif

#undef FC_USER_ACL_CA_CLS_SUPPORT

#if defined(CONFIG_FC_RTL8277C_SERIES)
#define MAX_ACL_CA_AAL_CLS_RULE_SIZE (256*2)		//ca aal info
#else
#define MAX_ACL_CA_AAL_CLS_RULE_SIZE (128*2)		//ca aal info
#endif

#else
#define CLS_T2CTRL_MAINHASH_PROFILE_WAN 0
#define CLS_T2CTRL_MAINHASH_PROFILE_LAN 1
#define CLS_T2CTRL_MAINHASH_PROFILE_MC 2
#define CLS_T2CTRL_MAINHASH_PROFILE_2TUPLE_BRIDGE 3
#define CLS_T2CTRL_MAINHASH_PROFILE_DOS_POL 4
#define CLS_T2CTRL_MAINHASH_PROFILE_MC_WIFI_TX 0xf

#define FC_USER_ACL_CA_CLS_SUPPORT

#ifdef CONFIG_FC_RTL8198F_SERIES
#define MAX_ACL_CA_CLS_RULE_SIZE (64*2)			//ca sw index, this value is define in classifier.c: CLS_RULE_MAX=L3_CLS_KEY_TBL_ENTRY_MAX*4
#define MAX_ACL_CA_AAL_CLS_RULE_SIZE (64)		//ca aal info
#else
#define MAX_ACL_CA_CLS_RULE_SIZE (128*2)		//ca sw index, this value is define in classifier.c: CLS_RULE_MAX=L3_CLS_KEY_TBL_ENTRY_MAX*4
#define MAX_ACL_CA_AAL_CLS_RULE_SIZE (128)		//ca aal info
#endif
#endif

#define MAX_ACL_LOG_MIB_NUM 			32		//for 40-bits byte count, use pol2 32~32+32

#define MAX_ACL_SPECIAL_PACKET_RULE_SIZE CA_PKT_TYPE_MAX

#define CA_L3_CLS_PROFILE_LAN (1<<AAL_LPORT_ETH_NI0)
#define CA_L3_CLS_PROFILE_WAN (1<<AAL_LPORT_ETH_NI7)

#define CA_L3_CLS_CPU_TX_PORTMASK ((1<<0x18)|(1<<RTK_FC_MAC_PORT_CPU1)|(1<<RTK_FC_MAC_PORT_WLAN_CPU0)|(1<<RTK_FC_MAC_PORT_WLAN_CPU1))	//cpu tx VP choosen by smp (0x10 will use 0x18)

//for support ca aal api
#define CA_L3_AAL_CLS_PROFILE_WAN 0
#define CA_L3_AAL_CLS_PROFILE_LAN 1
#define CA_L3_AAL_KEY_MSK_ENABLE 0
#define CA_L3_AAL_KEY_MSK_DISABLE 1

//for support ca aal api, get the index of profile, it maybe wan, lan
#define CA_SCAN_AAL_PROFILE_MASK(profile, profilemask)	for(profile = CA_L3_AAL_CLS_PROFILE_WAN; profile <= CA_L3_AAL_CLS_PROFILE_LAN; profile++)  if(((0x1<<profile) & profilemask) != 0x0)
//get the port index for trap to cpu, it is physical port, ext port
//---------------CONFIG_FC_CA8277B_SERIES-ACL check this, for 0x30~0x3f
#define CA_SCAN_ALL_PORT(port)	for(port = RTK_FC_MAC_PORT0; port <= RTK_FC_MAC_PORT_LASTCPU; port++)  if((port != RTK_FC_MAC_PORT_CPU) && (((0x1<<port) & RTK_FC_ALL_MAC_PORTMASK) != 0x0))
//get the index from portmask, it maybe physical port
#define CA_SCAN_PORTMASK_WITHOUT_CPU(port, portmask)	for(port = RTK_FC_MAC_PORT0; port <= RTK_FC_MAC_PORT_MAINCPU; port++)  if(((0x1<<port) & portmask) != 0x0)
//get the index from portmask, it maybe physical port, cpu port, ext port
#define CA_SCAN_PORTMASK(port, portmask)	for(port = RTK_FC_MAC_PORT0; port <= RTK_FC_MAC_PORT_MAX; port++)  if(((1ULL<<port) & portmask) != 0x0)
#endif
//+++++++++++++++ ACL Related Define End +++++++++++++++


#ifdef CONFIG_RTK_L34_XPON_PLATFORM
#define RTK_FC_DEFAULT_WIFI_FLOW_CTRL_DETECT_INTERVAL 1  //unit: (1/10)sec
#endif

#define COLOR_Y "\033[1;33m"
#define COLOR_NM "\033[0m"
#define COLOR_H "\033[1;37;41m"
#define COLOR_G "\033[1;32m"

#ifdef CONFIG_RTK_L34_XPON_PLATFORM
#define RTK_FC_UNMATCH_FLOW_TRAP_PRI	3
#define RTK_FC_UNMATCH_FLOW_TRAP_PRI_DEFAULT	0

#endif

#ifndef bzero
#define bzero(p,s) memset(p,0,s)
#endif

#ifndef rtlglue_printf
#if	defined(__KERNEL__)
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
#define rtlglue_printf	printk
#else
#define rtlglue_printf	_rtk_fc_rtlglue_printf
#endif
#else
#define rtlglue_printf	printf
#endif
#endif

#define FC_PARAM_CHK(expr, errCode)\
do {\
    if ((int32)(expr)) {\
        return errCode; \
    }\
} while (0)

// In order to share an image between 9607C and 9603CVD
#ifndef PATH12_SPA
#if defined(CONFIG_RTL9607C_SERIES) && defined(CONFIG_RTL9603CVD_SERIES)
#define PATH12_SPA(in_spa) ((ASIC_CHIP_ID==RTL9603CVD_CHIP_ID)?(in_spa>>1):(in_spa))
#else
#define PATH12_SPA(in_spa) (in_spa)
#endif
#endif
// In order to share an image between 9607C and 9603CVD
#ifndef PATH1234_OUTPORTMASK
#if defined(CONFIG_RTL9607C_SERIES) && defined(CONFIG_RTL9603CVD_SERIES)
#define PATH1234_OUTPORTMASK(out_portmask) ((ASIC_CHIP_ID==RTL9603CVD_CHIP_ID)?(out_portmask>>5):(out_portmask))
#else
#define PATH1234_OUTPORTMASK(out_portmask) (out_portmask)
#endif
#endif


/**
 * ether_addr_equal - Compare two Ethernet addresses
 * @addr1: Pointer to a six-byte array containing the Ethernet address
 * @addr2: Pointer other six-byte array containing the Ethernet address
 *
 * Compare two Ethernet addresses, returns true if equal
 *
 * Please note: addr1 & addr2 must both be aligned to u16.
 */
//#define FC_MAC_EQ(addr1, addr2) ether_addr_equal(addr1, addr2)

#define WLAN_IFACE_NUM_MAX 3

#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_FC_RTL8198F_SERIES) || defined(CONFIG_RTL8198X_SERIES)
typedef enum rtk_fc_tcp_check_action_e
{
	RTK_FC_TCP_IN_WINDOW_FAIL_CONTINUE_SHORTCUT = 0,
	RTK_FC_TCP_IN_WINDOW_FAIL_FREE_SKB, 
	RTK_FC_TCP_IN_WINDOW_FAIL_TO_PS,
}rtk_fc_fc_tcp_check_action_t;

#define RTK_FC_STR_FAIL_CONTINUE_SHORTCUT			"do nothing, continue to shortcut forwarding"
#define RTK_FC_STR_FAIL_FREE_SKB					"free skb"
#define RTK_FC_STR_FAIL_TO_PS						"return to protocol stack"
#endif

#define MAX_DEFAULT_ROUTE_INFO_ENTRY_SIZE			8

#if (defined(CONFIG_RTK_L34_XPON_PLATFORM) && IS_MODULE(CONFIG_RTK_L34_FC_KERNEL_MODULE)) 
extern char _flowEntryDataPool[];
#define FLOWDATAPOOL _flowEntryDataPool
#else
#define FLOWDATAPOOL fc_db.flowEntryDataPool
#endif


/*
 *  FC features support list
 */
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
#define FC_F_SHORTCUT_EARLYCHECK	1
#define FC_F_SWFLOWLIST_IDX_MODE	1		// flow entry support CRC16 value
#endif

//#define FC_F_SHORTCUT_CALLBACK 	1		// not support

#define UPDATE_IFF_FLAG (IFF_POINTOPOINT|IFF_NOARP)		


#endif //__RTK_FC_DEFINE__
