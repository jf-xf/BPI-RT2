#ifndef __RTK_FC_MGRTRX__
#define __RTK_FC_MGRTRX__


#include <rtk_fc_helper.h>
#include <rtk_fc_nic_ext.h>
#include <rtk_fc_assistant.h>

#if defined(CONFIG_RTK_L34_G3_PLATFORM)
#include <ca_ext.h>
#include <aal_hash.h>
#endif

#ifdef CONFIG_RTL_ETH_RECYCLED_SKB
#include "../../nicPool/re_recycleskb.h"
#endif

#if defined(CONFIG_RTK_FC_SW_FLOW_ENTRY_BITS)
#define RTK_FC_MAX_SHORTCUT_FLOW_SIZE 	(1<<CONFIG_RTK_FC_SW_FLOW_ENTRY_BITS)
#else
#define RTK_FC_MAX_SHORTCUT_FLOW_SIZE	32768	//maximum pure sw flow count
#endif

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)

#if defined(CONFIG_FC_FLOWBASED_DYN_MAX_32K)
#define RTK_FC_TABLESIZE_HW_FLOW		32768
#elif defined(CONFIG_FC_FLOWBASED_DYN_MAX_16K)
#define RTK_FC_TABLESIZE_HW_FLOW		16384
#elif defined(CONFIG_FC_FLOWBASED_DYN_MAX_8K)
#define RTK_FC_TABLESIZE_HW_FLOW		8192
#elif defined(CONFIG_FC_FLOWBASED_DYN_MAX_4K)
#if defined(CONFIG_FC_RTL9603CVD_SERIES)
#define RTK_FC_TABLESIZE_HW_FLOW		(1024+8)
#else
#define RTK_FC_TABLESIZE_HW_FLOW		(4096+64)
#endif
#endif

#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
#if defined(CONFIG_FC_RTL8277C_SERIES)
// number of overflow flow count in the system (0~64)
#define RTK_FC_TABLESIZE_OVERFLOW_FLOW	0
// number of mainHash flow count in the system
#define RTK_FC_TABLESIZE_HASH_FLOW		65536
// total HW flow count in the system
#define RTK_FC_TABLESIZE_HW_FLOW		(RTK_FC_TABLESIZE_HASH_FLOW+RTK_FC_TABLESIZE_OVERFLOW_FLOW)
// maximum HW suppoeted mainHash flow count
#define RTK_FC_TABLESIZE_HASH_FLOW_MAX	HASH_HASH_TBL_ENTRY_MAX
#elif defined(CONFIG_FC_RTL9607F_SERIES)
// 9607F not support overflow hash
#define RTK_FC_TABLESIZE_OVERFLOW_FLOW	0
// number of mainHash flow count in the system
#define RTK_FC_TABLESIZE_HASH_FLOW		65536
// total HW flow count in the system
#define RTK_FC_TABLESIZE_HW_FLOW		(RTK_FC_TABLESIZE_HASH_FLOW)
// maximum HW suppoeted mainHash flow count
#define RTK_FC_TABLESIZE_HASH_FLOW_MAX	HASH_HASH_TBL_ENTRY_MAX
#else
#define RTK_FC_TABLESIZE_HW_FLOW		(HASH_HASH_TBL_ENTRY_MAX+HASH_OVERFLOW_TBL_ENTRY_MAX)
#endif
#endif

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)

#define RXINFO_DST_PORTMASK_CPU_BIT		(0x1)
#define RXINFO_DST_PORTMASK_IDX_MODE	RXINFO_DST_PORTMASK_CPU_BIT

/* remove unnecessary fields to reduce memory size*/
typedef struct fc_rx_info_s{
	union{
		struct{
			u32 cputag:1;//31
			u32 ptp_in_cpu_tag_exist:1;//30
			u32 svlan_tag_exist:1;//29
			u32 reason:8;//21~28
			u32 gmac:4;//17~20
			u32 ctagva:1;//16
			u32 cvlan_tag:16;//0~15
		}bit;
		u32 dw;//double word
	}opts2;
	union{
		struct{
			u32 internal_priority:3;//29~31
			u32 pon_sid_or_extspa:7;//22~28 or 26~28
			u32 l3routing:1;//21
			u32 origformat:1;//20
			u32 src_port_num:4;//16~19
			u32 fbi:1;//15
			u32 fb_hash_or_dst_portmsk:15;
		}bit;
		struct{
			u32 internal_priority:3;//29~31
			u32 extspa:3;//26~28
			u32 rsvd_1:4;//22~25
			u32 l3routing:1;//21
			u32 origformat:1;//20
			u32 src_port_num:4;//16~19
			u32 fbi:1;//15
			u32 fb_hash_or_dst_portmsk:15;//0~14 or 0~6
		}bit1;
		u32 dw;//double word
	}opts3;
}fc_rx_info_t;
#if defined(CONFIG_FC_RTL9607C_SERIES)

typedef struct rtk_fc_rxdesc_s{

	union{
		struct{
			uint32 own:1;//31
			uint32 eor:1;//30
			uint32 fs:1;//29
			uint32 ls:1;//28
			uint32 crcerr:1;//27
			uint32 ipv4csf:1;//26
			uint32 l4csf:1;//25
			uint32 rcdf:1;//24
			uint32 ipfrag:1;//23
			uint32 pppoetag:1;//22
			uint32 rwt:1;//21
			uint32 rsvd0:2;//19~20
			uint32 rsvd1:5;//14~18
#ifdef CONFIG_RG_JUMBO_FRAME
			uint32 data_length:14;//0~13
#else
			uint32 rsvd2:2;//12~13
			uint32 data_length:12;//0~11
#endif
		}bit;
		uint32 dw;//double word
	}opts1;
	uint32 addr;
	union{
		struct{
			uint32 cputag:1;//31
			uint32 ptp_in_cpu_tag_exist:1;//30
			uint32 svlan_tag_exist:1;//29
			uint32 reason:8;//21~28
			uint32 rsvd_1:4;//17~20
			uint32 ctagva:1;//16
			uint32 cvlan_tag:16;//0~15
		}bit;
		uint32 dw;//double word
	}opts2;
	union{
		struct{
			uint32 internal_priority:3;//29~31
			uint32 pon_sid_or_extspa:7;//22~28 or 26~28
			uint32 l3routing:1;//21
			uint32 origformat:1;//20
			uint32 src_port_num:4;//16~19
			uint32 fbi:1;//15
			uint32 fb_hash_or_dst_portmsk:15;//0~14 or 0~6
		}bit;
		struct{
			uint32 internal_priority:3;//29~31
			uint32 extspa:3;//26~28
			uint32 rsvd_1:4;//22~26
			uint32 l3routing:1;//21
			uint32 origformat:1;//20
			uint32 src_port_num:4;//16~19
			uint32 fbi:1;//15
			uint32 fb_hash_or_dst_portmsk:15;//0~14 or 0~6
		}bit1;
		uint32 dw;//double word
	}opts3;

	#define rx_fs						opts1.bit.fs
	#define rx_ls						opts1.bit.ls
	#define rx_netIfIdx					opts1.bit.rsvd1
	#define rx_data_length				opts1.bit.data_length
	#define rx_gmac						opts1.bit.rsvd0

	#define rx_cputag					opts2.bit.cputag
	#define rx_reason					opts2.bit.reason
	#define rx_igrLocation				opts2.bit.rsvd_1
	#define rx_ctagva					opts2.bit.ctagva
	#define rx_cvlan_tag				opts2.bit.cvlan_tag

	#define rx_internal_priority		opts3.bit.internal_priority
	#define rx_l3routing				opts3.bit.l3routing
	#define rx_origformat				opts3.bit.origformat
	#define rx_src_port_num				opts3.bit.src_port_num
	#define rx_fbi						opts3.bit.fbi
	#define rx_fb_hash_or_dst_portmsk	opts3.bit.fb_hash_or_dst_portmsk
	
	#define rx_pon_stream_id			opts3.bit.pon_sid_or_extspa
	#define rx_extspa					opts3.bit1.extspa
	#define rx_fb_hash					rx_fb_hash_or_dst_portmsk
	#define rx_dst_port_mask			rx_fb_hash_or_dst_portmsk
//----------------------------------------------------------------	

}rtk_fc_rxdesc_t;
	
#endif

typedef struct tx_info fc_tx_info_t;

#define FC_NICRX_PRI RE8686_RXPRI_RG
#define FC_NICRX_PRI_HIGHEST 10
#define FC_NICRX_PRI_NPTV6 9
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)

typedef struct fc_rx_info_s{
	union{
		struct{
			u32 cputag:1;//31
			u32 ptp_in_cpu_tag_exist:1;//30
			u32 svlan_tag_exist:1;//29
			u32 reason:8;//21~28
			u32 gmac:5;//16~20			// fc private param
			u32 ref_idx:16;//0~15			// e.g. acl hit index
		}bit;
		u32 dw;//double word
	}opts2;
	union{
		struct{
			u32 internal_priority:3;//29~31
			u32 pon_sid_or_extspa:7;//22~28 or 26~28
			u32 l3routing:1;//21
			u32 origformat:1;//20
			u32 src_port_num:5;			// 4->5 for G3
			u32 fbi:1;
			u32 fb_hash_or_dst_portmsk:14;
		}bit;
		struct{
			u32 internal_priority:3;//29~31
			u32 extspa:7;				// 3->7 for G3
			u32 l3routing:1;
			u32 origformat:1;
			u32 src_port_num:5;			// 4->5 for G3
			u32 fbi:1;
			u32 fb_hash_or_dst_portmsk:14;
		}bit1;
		u32 dw;//double word
	}opts3;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	union{
		struct{
			u32 crc32;					// hash CRC32
		}bit;
		u32 dw;//double word
	}opts4;
#endif
	union{
		struct{
			u32 isWifiFF:1;
			u32 rsvd_0:31;
		}bit;
#if defined(CONFIG_FC_RTL8277C_SERIES)
		struct{
			u32 isWifiFF:2;
			u32 hash_crc_need_recal_hw_rsn:1;
			u32 hash_crc_need_recal_acl_trp:1;	// SW work around for acl trap
			u32 hash_crc_by_dual_inner:1;		// HW CRC in mdata is calculated by inner header
			u32 rsvd_0:11;
			u32 crc16:16;//0~15 		// hash CRC16
		}bit1;
#elif defined(CONFIG_FC_RTL9607F_SERIES)
		struct{
			u32 isWifiFF:2;
			u32 hash_crc_need_recal_hw_rsn:1;
			u32 hash_crc_by_dual_inner:1;		// HW CRC in mdata is calculated by inner header
			u32 rsvd_0:12;
			u32 crc16:16;//0~15 		// hash CRC16
		}bit1;
#endif
		u32 dw;//double word
	}opts5;
}fc_rx_info_t;


typedef struct fc_tx_info_s{
	union{
		struct{
			u32 own:1;//31
			u32 eor:1;//30
			u32 fs:1;//29
			u32 ls:1;//28
			u32 ipcs:1;//27
			u32 l4cs:1;//26
			u32 tpid_sel:1;//25
			u32 stag_aware:1;//24		
			u32 crc:1;//23
#if defined(CONFIG_FC_RTL9607F_SERIES)
			u32 dma_aft_map_idx:6;//17~22
#else
			u32 rsvd:6;//17~22
#endif
			u32 data_length:17;//0~16
		}bit;
		u32 dw;//double word
	}opts1;
	u32 addr;
	union{
		struct{
			u32 cputag:1;//31
			u32 tx_svlan_action:2;//29~30
			u32 tx_cvlan_action:2;//27~28
			u32 tx_portmask:11;//16~26
			u32 cvlan_vidl:8;//8~15
			u32 cvlan_prio:3;//5~7
			u32 cvlan_cfi:1;// 4
			u32 cvlan_vidh:4;//0~3
		}bit;
		u32 dw;//double word
	}opts2;
	union{
		struct{
			u32 extspa:4;//13~15	=> 28~31 for G3
			u32 aspri:1;//27
			u32 cputag_pri:3;//24~26
			u32 keep:1;//23
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
			u32 tx_with_hdr:1;//22
#else
			u32 rsvd2:1;//22
#endif
			u32 dislrn:1;//21
			u32 cputag_psel:1;//20
			u32 rsvd1:2;//18~19	
			u32 l34_keep:1;//17			
			u32 gmac_id:4;//13~16		 //software used for gmac_tx_idx(0:gmac9, 1:gmac10, 2:gmac7)
			u32 tx_pppoe_action:2;//11~12
			u32 tx_pppoe_idx:4;//7~10
			u32 tx_dst_stream_id:7;//0~6
		}bit;
		u32 dw;//double word
	}opts3;
	union{
		struct{
			u32 lgsen:1;//31
			u32 lgmtu:11;//20~30
			u32 rsvd:4;//16~19
			u32 svlan_vidl:8;//8~15
			u32 svlan_prio:3;//5~7
			u32 svlan_cfi:1;// 4
			u32 svlan_vidh:4;//0~3
		}bit;
		u32 dw;//double word
	}opts4;
#if defined(CONFIG_FC_RTL8198F_SERIES)
	union{
		struct{
			u32 pe_portmask:23; //9~31
			u32 reserved:9; //0~8
		}bit;
		u32 dw;//double word
	}opts5;
#endif
}fc_tx_info_t;




#define FC_NICRX_PRI RENIC_RXPRI_RG
#define FC_NICRX_PRI_HIGHEST RENIC_RXPRI_WIFI
#define FC_NICRX_PRI_NPTV6 RENIC_RXPRI_PATCH

#endif

#define RTK_FC_MGR_IPSEC_TABLE_SIZE_BIT 5
#define RTK_FC_IPSEC_SWID_BASE      32

// rx descriptor macro
#define RXINFO_ORIG_FORMAT(pRxInfo)					(pRxInfo->opts3.bit.origformat)
#define RXINFO_SPA(pRxInfo)							(pRxInfo->opts3.bit.src_port_num)
#define RXINFO_REASON(pRxInfo)			            (pRxInfo->opts2.bit.reason)		
#define RXINFO_INT_PRI(pRxInfo)			            (pRxInfo->opts3.bit.internal_priority)	
#define RXINFO_FBI(pRxInfo)							(pRxInfo->opts3.bit.fbi)	
#define RXINFO_FB_HASH(pRxInfo)					    (pRxInfo->opts3.bit.fb_hash_or_dst_portmsk)
#define RXINFO_DST_EXTPMASK(pRxInfo)	            (pRxInfo->opts3.bit.fb_hash_or_dst_portmsk)			// for 9607c
#define RXINFO_DST_EXTPIDX(pRxInfo)	                (pRxInfo->opts3.bit.fb_hash_or_dst_portmsk)		// for 8277
#define RXINFO_CPU_EXTSPA(pRxInfo)					(pRxInfo->opts3.bit1.extspa)
#define RXINFO_PON_SID(pRxInfo)           			(pRxInfo->opts3.bit.pon_sid_or_extspa)
#define RXINFO_L3ROUTING(pRxInfo)           		(pRxInfo->opts3.bit.l3routing)
#define RXINFO_GMAC(pRxInfo)			            (pRxInfo->opts2.bit.gmac)

#if defined(CONFIG_RTK_L34_G3_PLATFORM)
#define RXINFO_DST_EXTPORT_IDX(pRxInfo)				(pRxInfo->opts3.bit.fb_hash_or_dst_portmsk)		
#define RXINFO_REFIDX(pRxInfo)			            (pRxInfo->opts2.bit.ref_idx)
#define RXINFO_IS_WIFI_FF(pRxInfo)			        (pRxInfo->opts5.bit.isWifiFF)

#define RXINFO_REF_ACL_RSN_BIT						0xFFF
#define RXINFO_REF_ACL_RSN_CTRL_TRAP				0x800
#define RXINFO_REF_ACL_RSN_CTRL_RSV					0x400
#define RXINFO_REF_ACL_RSN_CTRL_USER 				0x200
#define RXINFO_REF_ACL_RSN_VAL						0x1FF
#define RXINFO_REF_TRAP_RSN_SHIFT_BIT               12

#define RXINFO_REF_HW_RSN_SHIFT_BIT               	8

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
#define MDATAL_SWID(mdata_l)						((mdata_l)&0xFF)
#define MDATAL_REASON(mdata_l)						((mdata_l>>8)&0xF)
#define MDATAL_REASON_SW(mdata_l)					((mdata_l>>12)&0xF)
#define MDATAL_CLS_RSN(mdata_l)						((mdata_l>>16)&0xFFF)
#define MDATAL_CRC16(mdata_l)						((mdata_l>>16)&0xFFFF)
#define MDATAH_CRC32(mdata_h)						(mdata_h&0xFFFFFFFF)

#define RXINFO_HASH_CRC16(pRxInfo)					(pRxInfo->opts5.bit1.crc16)
#define RXINFO_HASH_CRC32(pRxInfo)					(pRxInfo->opts4.bit.crc32)
#define RXINFO_HASH_CRC_NEED_RECAL_HW_RSN(pRxInfo)	(pRxInfo->opts5.bit1.hash_crc_need_recal_hw_rsn)
#if defined(CONFIG_FC_RTL8277C_SERIES)
#define RXINFO_HASH_CRC_NEED_RECAL_ACL_TRP(pRxInfo)	(pRxInfo->opts5.bit1.hash_crc_need_recal_acl_trp)
#else
#define RXINFO_HASH_CRC_NEED_RECAL_ACL_TRP(pRxInfo)	(0)
#endif
#define RXINFO_HASH_CRC_BY_DUAL_INNER(pRxInfo)		(pRxInfo->opts5.bit1.hash_crc_by_dual_inner)
#else
#define RXINFO_REF_VALID_BIT                                            0xFFFF
#define RXINFO_REF_TRAP_RSN_BIT                                         0xF000          //check rxinfo_ref_trap_rsn
#endif
#endif	//CONFIG_RTK_L34_G3_PLATFORM

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
#define RXINFO_REF_ACL_RSN_BIT						0xFFF
#define RXINFO_REF_ACL_RSN_CTRL_TRAP				0x800
#define RXINFO_REF_ACL_RSN_CTRL_RSV					0x400
#define RXINFO_REF_ACL_RSN_CTRL_USER 				0x200
#define RXINFO_REF_ACL_RSN_VAL						0x1FF
#endif

// tx descriptor macro
#define TXINFO_FS(ptxInfo)			(ptxInfo->opts1.bit.fs)
#define TXINFO_LS(ptxInfo)			(ptxInfo->opts1.bit.ls)
#define TXINFO_IPCS(ptxInfo)		(ptxInfo->opts1.bit.ipcs)
#define TXINFO_L4CS(ptxInfo)		(ptxInfo->opts1.bit.l4cs)
#define TXINFO_TPID_SEL(ptxInfo)	(ptxInfo->opts1.bit.tpid_sel)
#define TXINFO_STAG_AWARE(ptxInfo)	(ptxInfo->opts1.bit.stag_aware)
#define TXINFO_DATA_LEN(ptxInfo)	(ptxInfo->opts1.bit.data_length)
#define TXINFO_CPUTAG(ptxInfo)		(ptxInfo->opts2.bit.cputag)
#define TXINFO_SVLAN_ACT(ptxInfo)	(ptxInfo->opts2.bit.tx_svlan_action)
#define TXINFO_CVLAN_ACT(ptxInfo)	(ptxInfo->opts2.bit.tx_cvlan_action)
#define TXINFO_TX_PORTMSK(ptxInfo)	(ptxInfo->opts2.bit.tx_portmask)
#define TXINFO_CVLAN_VIDL(ptxInfo)	(ptxInfo->opts2.bit.cvlan_vidl)
#define TXINFO_CVLAN_PRIO(ptxInfo)	(ptxInfo->opts2.bit.cvlan_prio)
#define TXINFO_CVLAN_CFI(ptxInfo)	(ptxInfo->opts2.bit.cvlan_cfi)
#define TXINFO_CVLAN_VIDH(ptxInfo)	(ptxInfo->opts2.bit.cvlan_vidh)
#define TXINFO_ASPRI(ptxInfo)		(ptxInfo->opts3.bit.aspri)
#define TXINFO_CPUTAG_PRI(ptxInfo)	(ptxInfo->opts3.bit.cputag_pri)
#define TXINFO_KEEP(ptxInfo)		(ptxInfo->opts3.bit.keep)
#define TXINFO_DISLRN(ptxInfo)		(ptxInfo->opts3.bit.dislrn)
#define TXINFO_CPUTAG_PSEL(ptxInfo)	(ptxInfo->opts3.bit.cputag_psel)
#define TXINFO_L34_KEEP(ptxInfo)	(ptxInfo->opts3.bit.l34_keep)				//direct tx
#define TXINFO_GMAC_ID(ptxInfo)		(ptxInfo->opts3.bit.gmac_id)				//software used for gmac_tx_idx(0:gmac9, 1:gmac10)
#define TXINFO_EXTSPA(ptxInfo)		(ptxInfo->opts3.bit.extspa)
#define TXINFO_TX_PPPOE_ACT(ptxInfo) 		(ptxInfo->opts3.bit.tx_pppoe_action)
#define TXINFO_TX_PPPOE_IDX(ptxInfo)		(ptxInfo->opts3.bit.tx_pppoe_idx)
#define TXINFO_TX_DST_STREAM_ID(ptxInfo)	(ptxInfo->opts3.bit.tx_dst_stream_id)
#define TXINFO_SVLAN_VIDL(ptxInfo)  (ptxInfo->opts4.bit.svlan_vidl)
#define TXINFO_SVLAN_PRIO(ptxInfo)	(ptxInfo->opts4.bit.svlan_prio)
#define TXINFO_SVLAN_CFI(ptxInfo)	(ptxInfo->opts4.bit.svlan_cfi)
#define TXINFO_SVLAN_VIDH(ptxInfo)	(ptxInfo->opts4.bit.svlan_vidh)
#define TXINFO_EOR(ptxInfo)         (ptxInfo->opts1.bit.eor)
#define TXINFO_CRC(ptxInfo)         (ptxInfo->opts1.bit.crc)
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
#if defined(CONFIG_FC_RTL9607F_SERIES)
#define TXINFO_DMAAFT_MAP_IDX(ptxInfo)		(ptxInfo->opts1.bit.dma_aft_map_idx)
#define TXINFO_DMAAFT_MAP_IDX_DIR_TX(lspid)		((lspid == RTK_FC_MAC_PORT_CPU1)?NITX_LSPID_MAP_DIRTX_WAN:NITX_LSPID_MAP_DIRTX_LAN)
#endif
#define TXINFO_TX_WITH_HDR(ptxInfo)			(ptxInfo->opts3.bit.tx_with_hdr)
#endif

#if defined(CONFIG_FC_RTL8198F_SERIES)
#define TXINFO_PE_PORTMSK(ptxInfo)	 (ptxInfo->opts5.bit.pe_portmask)
#endif


#if defined(CONFIG_RTK_L34_G3_PLATFORM)
#define RTK_FC_GMAC_TO_MACPORT(gmac)	(gmac + RTK_FC_MAC_PORT_WLAN_CPU0)
#define RTK_FC_MACPORT_TO_GMAC(macport) (macport-RTK_FC_MAC_PORT_WLAN_CPU0)
#endif

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
#define RTK_FC_IS_WIFI_HWLOOKUP_MISS(pRxInfo)	(((1<<(RXINFO_SPA(pRxInfo))) & RTK_FC_ALL_MAC_WLANCPU_PORTMASK) && ((RXINFO_CPU_EXTSPA(pRxInfo)) != RTK_FC_MAC_EXT_CPU))
#elif defined(CONFIG_RTK_L34_G3_PLATFORM) && (defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES))
#define RTK_FC_IS_WIFI_HWLOOKUP_MISS(pRxInfo) (((RXINFO_SPA(pRxInfo)) !=  RTK_FC_MAC_PORT_PON) && (RXINFO_CPU_EXTSPA(pRxInfo)))
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
#define RTK_FC_IS_WIFI_HWLOOKUP_MISS(pRxInfo)	((1<<(RXINFO_SPA(pRxInfo))) & RTK_FC_ALL_MAC_WLANCPU_PORTMASK)
#else
#define RTK_FC_IS_WIFI_HWLOOKUP_MISS(pRxInfo)	(FALSE)
#endif

//Only For 32bit parameter x
#define RTK_FC_ONE_COUNT(x)\
do {\
	x = (x & 0x55555555) + ((x & 0xaaaaaaaa) >> 1);\
	x = (x & 0x33333333) + ((x & 0xcccccccc) >> 2);\
	x = (x & 0x0f0f0f0f) + ((x & 0xf0f0f0f0) >> 4);\
    x = (x & 0x00ff00ff) + ((x & 0xff00ff00) >> 8);\
    x = (x & 0x0000ffff) + ((x & 0xffff0000) >> 16);\
} while (0)


// hardware lookup
typedef struct rtk_fc_wlan_info_s{
	struct sk_buff *skb;
	unsigned int wlandevidx;
	uint8 outputQid	:3;
}rtk_fc_wlan_info_t;
typedef rtk_fc_wlan_info_t rtk_fc_wlanrx_info_t;
typedef rtk_fc_wlan_info_t rtk_fc_wlantx_info_t;
typedef rtk_fc_wlan_info_t rtk_fc_smp_wifiTx_private_t;

typedef struct rtk_fc_ps_netif_rx_info_s{
	struct sk_buff *skb;
}rtk_fc_ps_netif_rx_info_t;


#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
#define CPU_REASON_FB_LO_BOUND 1
#define CPU_REASON_FB_UP_BOUND 63
	/* ApolloPro Reason Table */
enum cpu_reason {
	CPU_REASON_NORMAL_FWD = 0,		//Error Reason
	CPU_REASON_FLOWMISS = 1,
	CPU_REASON_DUALFAIL,
	CPU_REASON_IP6WITHGRE,
	CPU_REASON_WOTCPUDP,
	CPU_REASON_MC_DMACTRANS = 5,
	CPU_REASON_FRAG,
	CPU_REASON_TCPFLAG,
	CPU_REASON_MULTIHIT,
	CPU_REASON_IPEXT,
	CPU_REASON_DUALHDR_NAPT = 10,
	CPU_REASON_TTL,
	CPU_REASON_MTU,
	CPU_REASON_IGSINTF,
	CPU_REASON_SPADENY,
	CPU_REASON_EGSINTF = 15,

	CPU_REASON_DROP_FLOW = 20,
	CPU_REASON_DROP_METER,
	CPU_REASON_DROP_WAL,
	CPU_REASON_DROP_CHKSUM,
	CPU_REASON_DROP_IGSINTF,
	CPU_REASON_DROP_IGSIPDENY = 25,
	CPU_REASON_DROP_EGSIPDENY,
	CPU_REASON_DROP_SPADENY,
	CPU_REASON_DROP_EGSINTF = 28,

	CPU_REASON_S2RSN = 32,
	CPU_REASON_FLOWMISS_S2 = 33,
	CPU_REASON_DUALFAIL_S2,
	CPU_REASON_IP6WITHGRE_S2 = 35,
	CPU_REASON_WOTCPUDP_S2,
	CPU_REASON_MC_DMACTRANS_S2,
	CPU_REASON_FRAG_S2,
	CPU_REASON_TCPFLAG_S2,
	CPU_REASON_MULTIHIT_S2 = 40,
	CPU_REASON_IPEXT_S2,
	CPU_REASON_DUALHDR_NAPT_S2,
	CPU_REASON_TTL_S2,
	CPU_REASON_MTU_S2,
	CPU_REASON_IGSINTF_S2 = 45,
	CPU_REASON_SPADENY_S2,
	CPU_REASON_EGSINTF_S2 = 47,

	CPU_REASON_DROP_FLOW_S2 = 52,
	CPU_REASON_DROP_METER_S2,
	CPU_REASON_DROP_WAL_S2,
	CPU_REASON_DROP_CHKSUM_S2 = 55,
	CPU_REASON_DROP_IGSINTF_S2,
	CPU_REASON_DROP_IGSIPDENY_S2,
	CPU_REASON_DROP_EGSIPDENY_S2,
	CPU_REASON_DROP_SPADENY_S2,
	CPU_REASON_DROP_EGSINTF_S2=60,

	CPU_REASON_L34_FWD=64,
	CPU_REASON_HOST_DA_POLICY	= 103,
	CPU_REASON_UNMATCH_VLAN 	= 201,
	CPU_REASON_FLOOD 				= 202,
	CPU_REASON_BCAST_TRAP 		= 203,
	CPU_REASON_LUT				= 204,
	CPU_REASON_SVLAN_IG 			= 206,
	CPU_REASON_L2_LEARN_LIMIT_PERPORT = 210,
	CPU_REASON_UNKNOWN_DA 		= 211,
	CPU_REASON_UNMATCH_SA 		= 212,
	CPU_REASON_UNKNOWN_SA 		= 213,
	CPU_REASON_RESV_TRAP			= 215,
	CPU_REASON_V6_ICMP_DHCP		= 216,
	CPU_REASON_IGMP				= 219,
	CPU_REASON_HOST_SA_POLICY	= 222,
	CPU_REASON_ACL				= 227,
	CPU_REASON_CF					= 228,
	CPU_REASON_CPUTAG_FORCE 	= 246,
};

#else// !defined(CONFIG_RTK_L34_XPON_PLATFORM)
#if defined(CONFIG_FC_RTL8277C_SERIES)
enum cpu_reason {
	CPU_REASON_RESERVED			= 0,		//reserved for any other HW trap.
	CPU_REASON_STG0_TRAP		= 1,		//L3FE PP or stag0 trap. (e.g. error detected packets, DoS, SPB trap)
	CPU_REASON_ACL			= 2,		//acl trap. (cls hit && ldpid = 0x10 && keep_orig_pkt = 1)
	CPU_REASON_FLOWMISS			= 3,		//flow miss. (hash default action: may be trap or drop)
	CPU_REASON_TTL				= 4,		//hash hit but ip ttl=0.
	CPU_REASON_DMAC_LOOKUP_MISS	= 5,		//hash hit but l2(fdb) DMAC lookup miss
	CPU_REASON_MTU				= 6,		//over MTU
	CPU_REASON_L34_FWD			= 15,		//normal forward, a.k.a. flow hit.
	CPU_REASON_ACL_TRAP_PS		= 16,		//acl trap to ps, carry by RXINFO_REF_TRAP_RSN_ACL_TO_PS
	CPU_REASON_FLOW_COPY		= 17,		//flow copy to cpu
};
enum rxinfo_ref_sw_rsn {
	RXINFO_REF_TRAP_RSN_ACL = 1,			//ACL trap without bypass Hash
	RXINFO_REF_TRAP_RSN_DUAL_DS = 2, 		//Hit ACL for Dual DS pop outter but flow miss trap
	RXINFO_REF_TRAP_RSN_ACL_TO_PS = 4,		//ACL trap to PS
	RXINFO_REF_TRAP_RSN_MAX = 15,
};
#elif defined(CONFIG_FC_RTL9607F_SERIES)
enum cpu_reason {
	CPU_REASON_RESERVED			= 0,		//reserved for any other HW trap.
	CPU_REASON_STG0_TRAP		= 1,		//L3FE PP or stag0 trap. (e.g. error detected packets, DoS, SPB trap)
	CPU_REASON_ACL_TRAP_PS		= 2,		//acl trap. (update by CLS fib, reserve to acl trap to ps)
	CPU_REASON_FLOWMISS			= 3,		//flow miss. (hash default action: may be trap or drop)
	CPU_REASON_TTL				= 4,		//hash hit but ip ttl=0.
	CPU_REASON_DMAC_LOOKUP_MISS	= 5,		//hash hit but l2(fdb) DMAC lookup miss
	CPU_REASON_MTU				= 6,		//over MTU
	CPU_REASON_ACL				= 14,		//acl trap with hash crc (update by CLS fib, reserve to acl trap without bypass hash)
	CPU_REASON_L34_FWD			= 15,		//normal forward, a.k.a. flow hit.
	CPU_REASON_FLOW_COPY		= 17,		//flow copy to cpu
};
enum rxinfo_ref_sw_rsn {
	RXINFO_REF_TRAP_RSN_DUAL_DS = 2, 		//Hit ACL for Dual DS pop outter but flow miss trap
	RXINFO_REF_TRAP_RSN_MAX = 15,
};
#else
enum cpu_reason {
	CPU_REASON_FLOWMISS			= 1,
	CPU_REASON_L34_FWD			= 64,
	CPU_REASON_UNKNOWN_DA		= 211,
	CPU_REASON_ACL				= 227,
	CPU_REASON_ACL_NON_TRAP 	= 229,
	CPU_REASON_ACL_TRAP_PS	 	= 230,
	CPU_REASON_FLOW_COPY		= 231,		//flow copy to cpu
	// not support but declaration is necesary
	CPU_REASON_TTL,
	CPU_REASON_MTU,
};
enum rxinfo_ref_trap_rsn {
	RXINFO_REF_TRAP_RSN_ACL = 1,			//use in rtk_fc_acl.c
	RXINFO_REF_TRAP_RSN_FLOWMISS,			//use in aal_hash.c
	RXINFO_REF_TRAP_RSN_UNKNOWN_DA,			//use in rtk_rg_g3_internal.c
	RXINFO_REF_TRAP_RSN_ACL_TO_PS,			//use in rtk_fc_acl.c
	RXINFO_REF_RSN_MAX = 15,
};
#endif
#endif

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)	
#define IS_COPY2CPU_PKT(pRxInfo)  ((RXINFO_REASON(pRxInfo) == CPU_REASON_L34_FWD) && (RXINFO_DST_EXTPMASK(pRxInfo)==0x1	))
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
#define IS_COPY2CPU_PKT(pRxInfo)  (RXINFO_REASON(pRxInfo) == CPU_REASON_FLOW_COPY)

#if defined(CONFIG_FC_RTL9607F_SERIES)
#define HAS_ACL_RSN(reason, ref_idx)  (((reason == CPU_REASON_ACL) || (reason == CPU_REASON_ACL_TRAP_PS)) && ref_idx)
#define DMALSO_LSPID_MAP_IDX(lspid)	 (((lspid >= RTK_FC_MAC_PORT_CPU) && (lspid <= RTK_FC_MAC_PORT_WLAN_CPU5))?(lspid - RTK_FC_MAC_PORT_CPU):0)	//ARM usage is reserve by rtk_ni_init_tx_dma_lso
#else
#define HAS_ACL_RSN(reason, ref_idx)  ((reason == CPU_REASON_ACL) && ref_idx)
#endif

#endif


#define IPI_SCHED 	0
#define IPI_IDLE		1

#define IPI_EMPTY_CHK_NONE 	0
#define IPI_EMPTY_CHK_EXEC		1

typedef struct rtk_fc_smp_ipi_ctrl_s
{
	struct tasklet_struct tasklet;
	__call_single_data_t csd ____cacheline_aligned_in_smp;
	atomic_t csd_available;
	struct tasklet_struct tasklet_empty_check;
	uint8 empty_check;
}rtk_fc_smp_ipi_ctrl_t;
typedef struct rtk_fc_smp_nicRx_private_s
{
	struct sk_buff *skb;
	fc_rx_info_t rxInfo;
	void *extraInfo;	//for XPON chip skb information need cp
}__attribute__((packed)) rtk_fc_smp_nicRx_private_t;

typedef enum rtk_fc_smp_work_state_e
{
	SMP_WORK_STATE_FREE,
	SMP_WORK_STATE_SCHED,
}rtk_fc_smp_work_state_t;

typedef enum rtk_fc_smp_buffer_state_e
{
	SMP_BUF_PRIV_AVAILABLE_PRIV_CONSUME,
	SMP_BUF_PRIV_FULL_PRIV_CONSUME,
	SMP_BUF_PRIV_AVAILABLE_GLOBAL_CONSUME,
}rtk_fc_smp_buffer_state_t;

// private ring buffer
typedef struct rtk_fc_smp_nicRx_work_info_s
{
	rtk_fc_smp_nicRx_private_t smp_nicRx_info;	
}rtk_fc_smp_nicRx_work_info_t;

typedef struct rtk_fc_smp_nicTx_private_s
{
	struct sk_buff *skb;
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	fc_tx_info_t txInfo;
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
	//struct net_device *dev;	// get from skb->dev	
	uint32 flow_id;
	ni_tx_core_config_t core_config;
	dma_lsopara_0_t lso_para0;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	uint16 stream_id;
	uint8 stream_id_en:1;
	uint8 force_def_tx_en:1;
	uint8 chk_sel_en:1;
	rtk_tx_dma_lso_checksum_selection_t chk_sel:3;
#if defined(CONFIG_FC_CA8277AB_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES)
#else // 9607F later: support lspid_map and dmaaft_map table
	uint16 hwlookup_lspid_mapIdx;
	uint8 dmaaft_map:6;
#endif
#endif
	uint8 isHwlookup:1;
	uint8 hwlookup_lspid;
#endif
}__attribute__((packed)) rtk_fc_smp_nicTx_private_t;

typedef struct rtk_fc_smp_nicTx_work_s
{
	rtk_fc_smp_nicTx_private_t smp_nicTx_info;
}rtk_fc_smp_nicTx_work_t;

typedef struct rtk_fc_smp_wlanRx_work_s
{
	rtk_fc_wlanrx_info_t smp_wlanRx_info;	
}rtk_fc_smp_wlanRx_work_t;

typedef struct rtk_fc_smp_wlanTx_work_s
{
	rtk_fc_wlantx_info_t smp_wlanTx_info;
}rtk_fc_smp_wlanTx_work_t;

typedef struct rtk_fc_smp_ps_netif_rx_work_s
{
	rtk_fc_ps_netif_rx_info_t smp_ps_netif_rx_info;
}rtk_fc_smp_ps_netif_rx_work_t;


typedef struct rtk_fc_mgr_l2_info_s
{	
	unsigned char spa;
	unsigned char extspa;
	rtk_fc_wlan_devidx_t wlan_dev_idx;			// rtk_fc_wlan_devidx_t, init as RTK_FC_WLANX_NOT_FOUND
	unsigned char isGMAC:1;
	unsigned short lutIdx;
}rtk_fc_mgr_l2_info_t;

int rtk_fc_trx_init(void);
int rtk_fc_nic_tx_with_pon_sid(struct sk_buff *skb, uint32 ponsid);
int rtk_fc_nic_tx(void *pNicTx_privateInfo_data);

int rtk_fc_smp_wlan_tx_dispatch(rtk_fc_wlantx_info_t *pWlanTxInfo);
int rtk_fc_smp_ps_netif_rx_dispatch(rtk_fc_ps_netif_rx_info_t *pNetif_rxInfo);

#if defined(CONFIG_FC_RTL9607C_SERIES)
uint32 _rtk_fc_wlan_rx_lookup_gmac_decision_by_hash(void);
uint32 _rtk_fc_wlan_rx_lookup_gmac_decision_by_port(uint32 spa);
uint32 _rtk_fc_wlan_rx_lookup_gmac_decision_by_da(unsigned char *da);
#endif

void rtk_fc_smp_nic_rx_exec(unsigned long data);
void rtk_fc_smp_nic_rx_tasklet(void *info);
void rtk_fc_smp_nic_tx_exec(unsigned long data);
void rtk_fc_smp_nic_tx_tasklet(void *info);
void rtk_fc_smp_wlan_rx_exec(unsigned long data);
void rtk_fc_smp_wlan_rx_tasklet(void *info);
void rtk_fc_smp_wlan_tx_exec(unsigned long data);
void rtk_fc_smp_wlan_tx_tasklet(void *info);
void rtk_fc_smp_ps_netif_rx_exec(unsigned long data);
void rtk_fc_smp_ps_netif_rx_tasklet(void *info);


void rtk_fc_skb_rxhook_skb_handling(struct sk_buff *skb, int ret);

//extern functions
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
netdev_tx_t nic_egress_start_xmit(struct sk_buff *skb, struct net_device *dev, ca_ni_tx_config_t *tx_config);
netdev_tx_t nic_egress_start_xmit_for_fc_dirTx(struct sk_buff *skb, struct net_device *dev, ca_ni_tx_config_t *tx_config);
netdev_tx_t nic_egress_start_xmit_for_fc_wifi_hw_lookup(struct sk_buff *skb, struct net_device *dev, ca_ni_tx_config_t *tx_config);
netdev_tx_t ca_ni_start_xmit_native(struct sk_buff *skb, struct net_device *dev, ca_ni_tx_config_t *tx_config);
#endif

#endif
