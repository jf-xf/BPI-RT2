/*
 * Copyright (C) 2018 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*/

#ifndef __RTK_FC_HELPER_H__
#define __RTK_FC_HELPER_H__

#include <net/netfilter/nf_conntrack_helper.h>
#include "../../../../net/bridge/br_private.h"
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
#include "re8686_nic.h"
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
#include "ca_ni.h"
#include "ca_ext.h"
#endif

#if defined(CONFIG_RTL9607C_SERIES) && defined(CONFIG_RTK_PTOOL)
#include <linux/ptool.h>
#endif

#include <rtk_fc_helper_wlan.h>
#include <rt_wlan_ext.h>
#include <rt_ext_mapper.h>
#include <rtk_fc_helper_multicast.h>
#include <rtk_fc_mgrTRx.h>


#if defined(CONFIG_USB_NET_DRIVERS)
#define CONFIG_RTL_FC_USB_INTF
#define CONFIG_RTL_FC_USB_AUTO_INTFNAME
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

#ifndef FAILED
#define FAILED -1
#endif

//#if defined(CONFIG_FC_QTNA_WIFI_AX)
#define RTK_FC_QTNA_WIFI_DEV_NAME "host0"
//#endif

#define RTK_FC_IP_NO_FRAG				(0x0)
#define RTK_FC_IP_FRAG					(0x1)
#define RTK_FC_IP_FIRST_FRAG				(0x2)
#define RTK_FC_IP_LAST_FRAG				(0x4)
#define RTK_FC_IPFRAG_MAX_HASH_SIZE			(32)
#define RTK_FC_IPFRAG_MAX_HASH_ENTRY_SIZE		(128)
#define RTK_FC_IPFRAG_CACHE_TIMEOUT_MSEC		(5000)			// 5s


#if IS_BUILTIN(CONFIG_RTK_L34_FC_KERNEL_MODULE)
#define __IRAM_FC_SHORTCUT		__attribute__ ((section(".iram-fc-shortcut")))
#define __IRAM_FC_NICTRX 			__attribute__ ((__section__(".iram-fwd")))
#define __SRAM_FC_DATA				__attribute__ ((__section__(".sram-fc")))
#else
#define __IRAM_FC_SHORTCUT
#define __IRAM_FC_NICTRX
#define __SRAM_FC_DATA
#endif

#define RTK_FC_MGR_RET_CODE_BASIS 0x10000

typedef struct rtk_fc_timer_list_s
{
	struct timer_list timer;
	unsigned long data;
}rtk_fc_timer_list_t;

typedef enum rtk_fc_nic_rx_e{
	RTK_FC_NIC_RX_STOP = RE8670_RX_STOP,							// free skb by NIC and stop processing this packet.
	RTK_FC_NIC_RX_CONTINUE =RE8670_RX_CONTINUE,					// continue next rx func.
	RTK_FC_NIC_RX_STOP_SKBNOFREERE = RE8670_RX_STOP_SKBNOFREE,	// free skb already, NIC stop processing this packet.
	RTK_FC_NIC_RX_RX_END = RE8670_RX_END,
	RTK_FC_NIC_RX_CON_NO_ETHTYPE_TRANS,							// continue next rx func without skb eth_type trans
	RTK_FC_NIC_RX_NETIFRX_BY_FC,										// wifi pkt
	RTK_FC_NIC_WIFI_IGR_LEARNING,
	RTK_FC_NIC_RX_IPSEC_RELEARN,
}rtk_fc_nic_rx_t;

typedef enum rtk_fc_helper_flow_direction_e{
	RTK_FC_HELPER_FLOW_DIRECTION_UPSTREAM = 0,
	RTK_FC_HELPER_FLOW_DIRECTION_DOWNSTREAM	= 1,
}rtk_fc_helper_flow_direction_t;

typedef enum rtk_fc_netdev_priv_flag_type_e{
	RTK_FC_NETDEV_PRIV_FLAG_TYPE_DOMAIN_WAN,
	RTK_FC_NETDEV_PRIV_FLAG_TYPE_DOMAIN_WLAN,
	RTK_FC_NETDEV_PRIV_FLAG_TYPE_DOMAIN_ELAN,
	RTK_FC_NETDEV_PRIV_FLAG_TYPE_VSMUX,
	RTK_FC_NETDEV_PRIV_FLAG_TYPE_RSMUX,
	RTK_FC_NETDEV_PRIV_FLAG_TYPE_OSMUX,
	RTK_FC_NETDEV_PRIV_FLAG_TYPE_EBRIDGE,
	RTK_FC_NETDEV_PRIV_FLAG_TYPE_BRIDGE_PORT,
	RTK_FC_NETDEV_PRIV_FLAG_TYPE_LIVE_ADDR_CHANGE,
}rtk_fc_netdev_priv_flag_type_t;

typedef enum rtk_fc_mgr_ret_e                               //To fit in FC core ret value.
{
	RTK_FC_MGR_RET_OK=0,
		
	RTK_FC_MGR_RET_SHORTCUT_DPI_TRAP = (RTK_FC_MGR_RET_CODE_BASIS << 3),                     // 0x80000
	RTK_FC_MGR_RET_SHORTCUT_DPI_CONTINUE,
	RTK_FC_MGR_RET_SHORTCUT_DPI_DROP,	
	RTK_FC_MGR_RET_SHORTCUT_DPI_END,
	
}rtk_fc_mgr_ret_t;

typedef enum rtk_fc_netdevice_getData_type_e
{
	RTK_FC_NETDEV_GET_MTU 			= 0,
	RTK_FC_NETDEV_GET_TYPE   		= 1,
	RTK_FC_NETDEV_GET_PRI_FLAGS     = 2,
	RTK_FC_NETDEV_GET_FLAGS   		= 3,
	RTK_FC_NETDEV_GET_DATA_MAX

}rtk_fc_netdevice_getData_type_t;

typedef enum rtk_fc_skb_param_e
{
	RTK_FC_SKB_PARAM_PKTTYPE = 0,
	RTK_FC_SKB_PARAM_LEN,

}rtk_fc_skb_param_t;

typedef enum rtk_fc_func_lock_type_e
{
	RTK_FC_FUNC_LOCK_FLOW,			//3 [CAUTION] flow lock is based on Hash index. 07C: 0~32767; 77x:  0~65535
	RTK_FC_FUNC_LOCK_SHAPER_MIB,
#if defined(CONFIG_FC_RTL8277C_SERIES)
	RTK_FC_FUNC_LOCK_FLOW_OVERFLOW,	// for all overflow flow
#endif
	RTK_FC_FUNC_LOCK_DYNAMIC_PREHASH,
	RTK_FC_FUNC_LOCK_IPSEC,
	RTK_FC_FUNC_LOCK_ACK_DELAY,
}rtk_fc_func_lock_type_t;

typedef struct rtk_fc_tableStreamId_s
{
	uint8 valid;
	uint8 deepq;
	uint8 ldpid;	// tcontid
	uint8 cos;
	uint16 gemid;	// flowid
}rtk_fc_tableStreamId_t;

typedef struct rtk_fc_coreDBInfo_streamId_s
{
	int index;
	rtk_fc_tableStreamId_t entry;
}rtk_fc_coreDBInfo_streamId_t;

typedef struct rtk_fc_coreDB_info_s
{
	union{
		rtk_fc_coreDBInfo_streamId_t data_sid;					//RT_FLOW_OPS_DELETE_FLOW
		uint8 data_tx_ctrl;							//tx_with_hdr_debug
	};
}rtk_fc_coreDB_info_t;

typedef enum rtk_fc_getFcDB_ops_e
{
	RTK_FC_COREDB_OPS_STREAMID = 0, 				// Get PON Stream id info
	RTK_FC_COREDB_OPS_TX_WO_HDR, 					// Get 8277C Tx Debug (tx_with_hdr_debug) status
	RTK_FC_COREDB_OPS_END,
} rtk_fc_coreDB_ops_t;

typedef enum rtk_fc_external_porttype_e
{
	RTK_FC_EXTERNAL_PORT_DISABLE = 0,
	RTK_FC_EXTERNAL_PORT_SWITCH, 				// Get PON Stream id info
	RTK_FC_EXTERNAL_PORT_VIRTUALPORT, 					// Get 8277C Tx Debug (tx_with_hdr_debug) status
	RTK_FC_EXTERNAL_PORT_END,
} rtk_fc_external_porttype_t;

typedef enum rtk_fc_helper_api_e
{
	FC_HELPER_TYPE_CONVERTER,
	FC_HELPER_TYPE_PS,
	FC_HELPER_TYPE_MGR,
	FC_HELPER_TYPE_MULTI_WAN,
	FC_HELPER_TYPE_VLAN,
	FC_HELPER_TYPE_WLAN,
	FC_HELPER_TYPE_RT_API,
	FC_HELPER_TYPE_EXT_PHY_QOS,
	FC_HELPER_TYPE_ALL_MAC_PORTMASK,
#if defined(CONFIG_FC_RTL8198F_SERIES)
	FC_HELPER_TYPE_83XX_QOS,
	FC_HELPER_TYPE_8367R_RELAY_MC,
#endif
	FC_HELPER_TYPE_MC,
#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_RTL8198XB_SERIES)
	FC_HELPER_TYPE_EXT_FLOW_MIB,
#endif
	FC_HELPER_TYPE_IPFRAG,
#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_FC_RTL8198F_SERIES) || defined(CONFIG_RTL8198X_SERIES)
	FC_HELPER_TYPE_TCP,
#endif
	FC_HELPER_TYPE_END,
}rtk_fc_helper_api_t;


typedef enum rtk_fc_helper_neigh_key_type_e
{
	FC_HELPER_KEY_PRIMARY_KEY,
	FC_HELPER_KEY_HA,
}rtk_fc_helper_neigh_key_type_t;

#define RTK_FC_HELPER_RET_CODE_BASIS 		0x10000
typedef enum rtk_fc_helper_ret_e
{
	RTK_FC_HELPER_RET_OK=0,
	RTK_FC_HELPER_RET_OK_NEW,

	/* error */
	RTK_FC_HELPER_RET_ERR 					= (RTK_FC_HELPER_RET_CODE_BASIS << 0), 		//0x10000
	RTK_FC_HELPER_RET_ERR_NULL_POINTER,
	RTK_FC_HELPER_RET_ERR_INVALID_PARAM,
	RTK_FC_HELPER_RET_ERR_TCP_WINDOW_CHECK_FAIL,
	RTK_FC_HELPER_RET_ERR_TBL_FULL,
}rtk_fc_helper_ret_t;

#if defined(CONFIG_RTK_SOC_RTL8198D)
typedef enum rtk_fc_counter_update_op_e
{
	RTK_FC_COUNTER_UPDATE_INCREASE = 1,
	RTK_FC_COUNTER_UPDATE_DECREASE,
	RTK_FC_COUNTER_UPDATE_UNKNOWN,
}rtk_fc_counter_update_op_t;

typedef enum rtk_fc_lan_client_type_e {
	RTK_EXT_FLOW_MIB_LAN_ETH,
	RTK_EXT_FLOW_MIB_LAN_WLAN_5G,
	RTK_EXT_FLOW_MIB_LAN_WLAN_2G,
}rtk_fc_lan_client_type_t;

typedef enum rtk_fc_counter_update_type_e
{
	RTK_FC_COUNTER_UPDATE_HW = 0,
	RTK_FC_COUNTER_UPDATE_SW,
	RTK_FC_COUNTER_UPDATE_KERNEL,
}rtk_fc_counter_update_type_t;

typedef struct rtk_fc_ext_flow_mib_param_s{
	uint32 						packet_count;
	uint32						byte_count;
	bool						is_uplink;
	rtk_fc_counter_update_op_t 	update_op;
	rtk_fc_counter_update_type_t update_type;
}rtk_fc_ext_flow_mib_param_t;

typedef struct rtk_fc_ext_flow_mib_host_mapping_s {
	uint32 		ip_addr;
	uint8		mac_addr[ETH_ALEN];
	uint8 		is_wlan_sta;
	uint8		dev_flag;
	char		dev_name[IFNAMSIZ];
} rtk_fc_ext_flow_mib_host_mapping_t;

typedef struct rtk_fc_ext_flow_mib_net_mapping_s {
	uint32 		netmask;
} rtk_fc_ext_flow_mib_net_mapping_t;

typedef struct rtk_fc_ext_flow_mib_counter_s {
	uint32 	in_packet_cnt;
	uint32 	in_byte_cnt;
	uint32 	out_packet_cnt;
	uint32 	out_byte_cnt;
} rtk_fc_ext_flow_mib_counter_t;

typedef enum rtk_ext_flow_mib_wlan_e {
	RTK_EXT_FLOW_MIB_WLAN_5G,
	RTK_EXT_FLOW_MIB_WLAN_2G,
	RTK_EXT_FLOW_MIB_WLAN_MAX
}rtk_ext_flow_mib_wlan_t;

#if defined(CONFIG_BAND_2G_ON_WLAN0) || defined(CONFIG_2G_ON_WLAN0)
#define RTK_FC_WLAN_NAME_5G				"wlan1"
#define RTK_FC_WLAN_NAME_2G				"wlan0"
#else
#define RTK_FC_WLAN_NAME_5G				"wlan0"
#define RTK_FC_WLAN_NAME_2G				"wlan1"
#endif

typedef enum rtk_ext_flow_mib_mode_e {
	RTK_EXT_FLOW_MIB_IPV4_BASED = 0U,
	RTK_EXT_FLOW_MIB_MAC_BASED,
	RTK_EXT_FLOW_MIB_MODE_MAX,
} rtk_ext_flow_mib_mode_t;
#endif

#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_RTL8198XB_SERIES)
typedef struct rtk_fc_ext_flow_mib_entry_s {
#if defined(CONFIG_RTK_SOC_RTL8198D)
	rtk_fc_ext_flow_mib_host_mapping_t 	host;
	rtk_fc_ext_flow_mib_net_mapping_t	net;
	rtk_fc_ext_flow_mib_counter_t 		counter;
	rtk_fc_ext_flow_mib_counter_t 		sw_counter;
	rtk_fc_ext_flow_mib_counter_t 		kernel_counter;
#else
	uint8 valid;
	uint8 mac_addr[ETH_ALEN];
#endif
} rtk_fc_ext_flow_mib_entry_t;

typedef struct rtk_fc_ext_flow_mib_ctrl_s {
	uint8 	enable;
#if defined(CONFIG_RTK_SOC_RTL8198D)
	uint8	mode;
	uint8	debug;
	uint8	debug_wfo;

	uint8	eth_hw_start;
	uint8	eth_hw_end;					//	[start, end)

	uint8	wlan_start;
	uint8	wlan_5g_hw_start;
	uint8	wlan_5g_hw_end;				//	[start, end)
	uint8	wlan_2g_hw_start;
	uint8	wlan_2g_hw_end;				//	[start, end)
	uint8	wlan_sw_start;
	uint8	wlan_sw_end;
	uint8	wlan_end;

	struct delayed_work wfo_w;
	uint16	wfo_wq_sleep_msec;
	uint8	wfo_wq_bind_cpu;

	uint32	__clean_area_start[0];

	uint8	wlan_5g_sw_count_entry;
	uint8	wlan_2g_sw_count_entry;
	uint32	wlan_to_nic_inc;
	uint32	wlan_to_nic_amsdu_inc;
	uint32	wlan_to_nic_dec;
	uint32	nic_to_wlan_inc;
	uint32	nic_to_wlan_agg_inc;

	uint8	need_update_dev;
	uint8	wfo_wq_running;
	uint32	wfo_wq_start_ok;
	uint32	wfo_wq_start_err;
	uint32	wfo_wq_end_ok;
	uint32	wfo_wq_end_err;
	uint32	wfo_wq_sched;
	uint32	wfo_func_null;
	uint32	try_update;
	uint32	kzalloc_fail;
	uint32  wfo_sta_dev_null;
	uint32  get_wfo_dev_fail;
	uint32	update_by_wfo;
	uint32	update_miss;

	uint32	__clean_area_end[0];
#endif
} rtk_fc_ext_flow_mib_ctrl_t;
#endif

#if defined(CONFIG_DEBUG_SPINLOCK)
#define ASSERT_NO_HARDIRQ()	\
	do{ \
		if(in_irq()){													\
		printk("call spin_lock when in_irq() is TRUE");	\
		dump_stack();}												\
	}while(0)

#define ASSERT_IN_INTERRUPT() \
	do{ \
		if(!in_interrupt()){	\
		printk("expect hardirq or softirq here, preempt_count = %d", preempt_count());	\
		dump_stack();} \
	}while(0)

#define ASSERT_PREEMPT_DISABLE() \
	do{ \
		if(preempt_count()<=0) {	\
		printk("expect preempt should be disabled");	\
		dump_stack();} \
	}while(0)
#else
#define ASSERT_NO_HARDIRQ()
#define ASSERT_IN_INTERRUPT()
#define ASSERT_PREEMPT_DISABLE()
#endif

#define fc_spin_lock(lock)			\
	do {						\
		ASSERT_NO_HARDIRQ();\
		ASSERT_IN_INTERRUPT();\
		spin_lock(lock); \
	} while(0)

#define fc_spin_unlock(lock)			\
	do {						\
		spin_unlock(lock); \
	} while(0)


#define fc_spin_lock_bh(lock)	\
	do {					\
		ASSERT_NO_HARDIRQ();\
		spin_lock_bh(lock);\
	} while(0)

#define fc_spin_unlock_bh(lock)	\
	do {					\
		spin_unlock_bh(lock);	\
	} while(0)


#define fc_spin_lock_bh_irq_protect(lock)			\
	do {		\
		if(irqs_disabled() && preempt_count()>0){	\
			fc_spin_lock(lock);}			\
		else{							\
			fc_spin_lock_bh(lock);}		\
	} while(0)
#define fc_spin_unlock_bh_irq_protect(lock)			\
	do {						\
		if(irqs_disabled()){			\
			fc_spin_unlock(lock);}		\
		else{							\
			fc_spin_unlock_bh(lock);}	\
	} while(0)


#if defined(CONFIG_DEBUG_SPINLOCK)
#define MGR_ASSERT_NO_HARDIRQ()	\
	do{ \
		if(in_irq()){													\
		printk("call spin_lock when in_irq() is TRUE\n");	\
		dump_stack();}												\
	}while(0)

#define MGR_ASSERT_IN_INTERRUPT() \
	do{ \
		if(!in_interrupt()){	\
		printk("expect hardirq or softirq here, preempt_count = %d\n", preempt_count());	\
		dump_stack();} \
	}while(0)

#define MGR_ASSERT_PREEMPT_DISABLE() \
	do{ \
		if(preempt_count()<=0) {	\
		printk("expect preempt should be disabled\n");	\
		dump_stack();} \
	}while(0)
#else
#define MGR_ASSERT_NO_HARDIRQ()
#define MGR_ASSERT_IN_INTERRUPT()
#define MGR_ASSERT_PREEMPT_DISABLE()
#endif


#define RTSKB_SKB(rtskb) (rtskb->skb)
#define RTSKB_DATA(rtskb) (*rtskb->data)
#define RTSKB_DEV(rtskb) (*rtskb->dev)
#define RTSKB_FROMDEV(rtskb) (*rtskb->from_dev)
#define RTSKB_HASH(rtskb) (*rtskb->hash)
#define RTSKB_FCIGRDATA(rtskb) (rtskb->fcIngressData)
#define RTSKB_LEN(rtskb) (*rtskb->len)
#define RTSKB_DATA_LEN(rtskb) (*rtskb->data_len)
#define RTSKB_MAC_HEADER(rtskb) (*rtskb->mac_header)
#define RTSKB_TRANSPORT_HEADER(rtskb) (*rtskb->transport_header)
#define RTSKB_NETWORK_HEADER(rtskb) (*rtskb->network_header)
#define RTSKB_MARK(rtskb) (*rtskb->mark)
#define RTSKB_MARK2(rtskb)((rtskb->mark2!=NULL) ? *rtskb->mark2 : 0)
#define RTSKB_MARK2_FORCE(rtskb) (*rtskb->mark2)
#define RTSKB_MARK2_EXIST(rtskb)((rtskb->mark2!=NULL) ? TRUE : FALSE)
#define RTSKB_RECYCLABLE(rtskb) (rtskb->recyclable)

#if defined(CONFIG_LUNA_G3_SERIES)
#if !defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
#define G3_LOOPBACK_DOWNSTREAM_VID_P0			0x10
#define	G3_LOOPBACK_DOWNSTREAM_VID_P1			0x20
#define G3_LOOPBACK_DOWNSTREAM_VID_P2			0x30
#define G3_LOOPBACK_DOWNSTREAM_VID_P3			0x40
#define G3_LOOPBACK_DOWNSTREAM_VID_BASE			G3_LOOPBACK_DOWNSTREAM_VID_P0
#define G3_LOOPBACK_DOWNSTREAM_VID(port, cos)	(G3_LOOPBACK_DOWNSTREAM_VID_BASE * (1+port) + cos)
#define G3_LOOPBACK_DOWNSTREAM_VID_MIN			G3_LOOPBACK_DOWNSTREAM_VID(AAL_LPORT_ETH_NI0,0)
#define G3_LOOPBACK_DOWNSTREAM_VID_MAX			G3_LOOPBACK_DOWNSTREAM_VID(AAL_LPORT_ETH_NI3,7)
#define G3_LOOPBACK_UPSTREAM_VID_P0				4000
#define G3_LOOPBACK_UPSTREAM_VID_P1				4001
#define G3_LOOPBACK_UPSTREAM_VID_P2				4002
#define G3_LOOPBACK_UPSTREAM_VID_P3				4003
#define G3_LOOPBACK_UPSTREAM_VID_BASE			G3_LOOPBACK_UPSTREAM_VID_P0
#define G3_LOOPBACK_UPSTREAM_VID(port)			(G3_LOOPBACK_UPSTREAM_VID_BASE + port)
#define G3_LOOPBACK_UPSTREAM_LSPID(VID)			(VID - G3_LOOPBACK_UPSTREAM_VID_BASE)
#define G3_LOOPBACK_UPSTREAM_VID_MIN			G3_LOOPBACK_UPSTREAM_VID(AAL_LPORT_ETH_NI0)
#define G3_LOOPBACK_UPSTREAM_VID_MAX			G3_LOOPBACK_UPSTREAM_VID(AAL_LPORT_ETH_NI3)
#define G3_LOOPBACK_P_START						AAL_LPORT_ETH_NI0
#define G3_LOOPBACK_P_END						AAL_LPORT_ETH_NI3
#define G3_LOOPBACK_P_MASK						((1 << AAL_LPORT_ETH_NI0) | (1 << AAL_LPORT_ETH_NI1) | (1 << AAL_LPORT_ETH_NI2) | (1 << AAL_LPORT_ETH_NI3))
#define G3_LOOPBACK_P_RSVSPA					AAL_LPORT_ETH_NI5		// DO NOT configure cls rule, merge p5/p6 entries for host policing
#define G3_LOOPBACK_P_NEWSPA					AAL_LPORT_ETH_NI6		// loopback from p6, igrcls(p6) for dmac host pol, egrcls(p25) for smac host pol.
//uint32 rtk_rg_g3_l2fe_hostpolicing_init(void);

#endif
#endif

#ifdef CONFIG_IPV6_MAPE_PSID_KERNEL_HOOK
extern __u32 ftp_alg_get_new_ct_timeout(void *nf_ct, __u8 state, __u32 ct_timeout);
#endif


struct rt_skbuff {
	struct sk_buff *skb;		// skb pointer ONLY used in doing dev tx!
	unsigned char **data;		// refer actual data buffer here.
	struct net_device	**dev;
	struct net_device	**from_dev;

	rtk_fc_ingress_data_t *fcIngressData;
	unsigned int *len;
	unsigned int *data_len;
	__u16 *mac_header;
	__u16 *transport_header;
	__u16 *network_header;	
	__u32 *mark;
	__u64 *mark2;
	__u32 *hash;
	__u16 recyclable;		//bit-field cannot take address of it
#if defined(CONFIG_FC_RTL9607C_SERIES)
	uint8 nptv6_setMeterFlag;
	uint8 vxlan_extra_setNic_flag;
	uint8 vxlan_setNic_flag;
	uint8 vxlan_down_setNic_flag;
	uint8 vxlan_extra_down_setNic_flag;
#endif
};


struct rt_nfconn {
	struct nf_conn *ct;
	union nf_conntrack_proto *proto;
	unsigned long status;
	u_int32_t mark;
	struct nf_conntrack *ct_general;
	spinlock_t *lock;
};

typedef enum rt_ip_conntrack_dir_e {
	RT_IP_CT_DIR_ORIGINAL = 0,
	RT_IP_CT_DIR_REPLY,
	RT_IP_CT_DIR_MAX,
}rt_ip_conntrack_dir_t;

typedef struct rt_nf_5tuple_s{
	union{
		uint32			v4_addr;	/* IPv4 address */
		struct in6_addr	v6_addr;	/* IPv6 address */
	}src_ip;
	union{
		uint32			v4_addr;	/* IPv4 address */
		struct in6_addr	v6_addr;	/* IPv6 address */
	}dest_ip;
	uint16 src_port;
	uint16 dest_port;
}rt_nf_5tuple_t;

typedef enum rt_nf_5tuple_l3Type_e{
	RT_NF_5TUPLE_l3TYPE_IPV4 = 0,
	RT_NF_5TUPLE_l3TYPE_IPV6,
	RT_NF_5TUPLE_l3TYPE_MAX,
}rt_nf_5tuple_l3Type_t;

typedef enum rt_nf_5tuple_l4Type_e{
	RT_NF_5TUPLE_l4TYPE_UDP = 0,
	RT_NF_5TUPLE_l4TYPE_TCP,
	RT_NF_5TUPLE_l4TYPE_MAX,
}rt_nf_5tuple_l4Type_t;

typedef struct rt_nf_5tuple_info_s{
	rt_nf_5tuple_l3Type_t l3_type;
	rt_nf_5tuple_l4Type_t l4_type;
	rt_nf_5tuple_t tuple[RT_IP_CT_DIR_MAX];
}rt_nf_5tuple_info_t;

typedef enum rtk_fc_enum_pktHdrTagif_e
{
	SVLAN_TAGIF=(1<<0),
	CVLAN_TAGIF=(1<<1),
	PPPOE_TAGIF=(1<<2),		// include PPPOE_DISCOVERY_TAGIF & PPPOE_SESSION_TAGIF
	IPV4_TAGIF=(1<<3),
	IPV6_TAGIF=(1<<4),
	TCP_TAGIF=(1<<5),
	UDP_TAGIF=(1<<6),
	IGMP_TAGIF=(1<<7),
	PPTP_TAGIF=(1<<8),
	L2TP_TAGIF=(1<<9),
	IP4_IN_IP6=(1<<10),			//ingress is 4in6
	GRESEQ_TAGIF=(1<<11),
	GREACK_TAGIF=(1<<12),
	IPSEC_ESP_TAGIF=(1<<13), 		// L3 + ESP: no udp encapsulation
	GRE_ETH_BR_TAGIF=(1<<14),
	VXLAN_TAGIF=(1<<15),
	OUTER_SVLAN_TAGIF=(1<<16),
	OUTER_CVLAN_TAGIF=(1<<17),
	OUTER_PPPOE_TAGIF=(1<<18),
	ICMP_TAGIF=(1<<19),
	V6RD_TAGIF=(1<<20),
} rtk_fc_enum_pktHdrTagif_t;

// tcp conntrack for tracking and sync state back to linux
typedef enum rt_tcp_conntrack_state_e {
	RT_TCP_CONNTRACK_NONE,
	RT_TCP_CONNTRACK_SYN_SENT,
	RT_TCP_CONNTRACK_SYN_RECV,
	RT_TCP_CONNTRACK_ESTABLISHED,
	RT_TCP_CONNTRACK_FIN_WAIT,
	RT_TCP_CONNTRACK_CLOSE_WAIT,
	RT_TCP_CONNTRACK_LAST_ACK,
	RT_TCP_CONNTRACK_TIME_WAIT,
	RT_TCP_CONNTRACK_CLOSE,
	RT_TCP_CONNTRACK_LISTEN,
	RT_TCP_CONNTRACK_MAX,
	RT_TCP_CONNTRACK_IGNORE,
}rt_tcp_conntrack_state_t;

static const char *const rt_tcp_conntrack_names[RT_TCP_CONNTRACK_IGNORE+1] = {
	"NONE",
	"SYN_SENT",
	"SYN_RECV",
	"ESTABLISHED",
	"FIN_WAIT",
	"CLOSE_WAIT",
	"LAST_ACK",
	"TIME_WAIT",
	"CLOSE",
	"SYN_SENT2",
	"INVALID",
	"IGNORE"
};

#define sNO RT_TCP_CONNTRACK_NONE
#define sSS RT_TCP_CONNTRACK_SYN_SENT
#define sSR RT_TCP_CONNTRACK_SYN_RECV
#define sES RT_TCP_CONNTRACK_ESTABLISHED
#define sFW RT_TCP_CONNTRACK_FIN_WAIT
#define sCW RT_TCP_CONNTRACK_CLOSE_WAIT
#define sLA RT_TCP_CONNTRACK_LAST_ACK
#define sTW RT_TCP_CONNTRACK_TIME_WAIT
#define sCL RT_TCP_CONNTRACK_CLOSE
#define sS2 RT_TCP_CONNTRACK_LISTEN
#define sIV RT_TCP_CONNTRACK_MAX
#define sIG RT_TCP_CONNTRACK_IGNORE

typedef enum rtk_fc_tcpPktType_s
{
	RT_TCP_PKT_TYPE_SYN,
	RT_TCP_PKT_TYPE_SYNACK,
	RT_TCP_PKT_TYPE_FIN,
	RT_TCP_PKT_TYPE_ACK,
	RT_TCP_PKT_TYPE_RST,
	RT_TCP_PKT_TYPE_NONE,
	RT_TCP_PKT_TYPE_END,
}rtk_fc_tcpPktType_t;

static const u8 rt_tcp_conntracks[RT_IP_CT_DIR_MAX][RT_TCP_PKT_TYPE_END][RT_TCP_CONNTRACK_MAX] = {
	{
/* ORIGINAL */
/*		 sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*syn*/    { sSS, sSS, sIG, sIG, sIG, sIG, sIG, sSS, sSS, sS2 },
/*
 *	sNO -> sSS	Initialize a new connection
 *	sSS -> sSS	Retransmitted SYN
 *	sS2 -> sS2	Late retransmitted SYN
 *	sSR -> sIG
 *	sES -> sIG	Error: SYNs in window outside the SYN_SENT state
 *			are errors. Receiver will reply with RST
 *			and close the connection.
 *			Or we are not in sync and hold a dead connection.
 *	sFW -> sIG
 *	sCW -> sIG
 *	sLA -> sIG
 *	sTW -> sSS	Reopened connection (RFC 1122).
 *	sCL -> sSS
 */
/*		 sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*synack*/ { sIV, sIV, sSR, sIV, sIV, sIV, sIV, sIV, sIV, sSR },
/*
 *	sNO -> sIV	Too late and no reason to do anything
 *	sSS -> sIV	Client can't send SYN and then SYN/ACK
 *	sS2 -> sSR	SYN/ACK sent to SYN2 in simultaneous open
 *	sSR -> sSR	Late retransmitted SYN/ACK in simultaneous open
 *	sES -> sIV	Invalid SYN/ACK packets sent by the client
 *	sFW -> sIV
 *	sCW -> sIV
 *	sLA -> sIV
 *	sTW -> sIV
 *	sCL -> sIV
 */
/*		 sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*fin*/    { sIV, sIV, sFW, sFW, sLA, sLA, sLA, sTW, sCL, sIV },
/*
 *	sNO -> sIV	Too late and no reason to do anything...
 *	sSS -> sIV	Client migth not send FIN in this state:
 *			we enforce waiting for a SYN/ACK reply first.
 *	sS2 -> sIV
 *	sSR -> sFW	Close started.
 *	sES -> sFW
 *	sFW -> sLA	FIN seen in both directions, waiting for
 *			the last ACK.
 *			Migth be a retransmitted FIN as well...
 *	sCW -> sLA
 *	sLA -> sLA	Retransmitted FIN. Remain in the same state.
 *	sTW -> sTW
 *	sCL -> sCL
 */
/*		 sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*ack*/    { sES, sIV, sES, sES, sCW, sCW, sTW, sTW, sCL, sIV },
/*
 *	sNO -> sES	Assumed.
 *	sSS -> sIV	ACK is invalid: we haven't seen a SYN/ACK yet.
 *	sS2 -> sIV
 *	sSR -> sES	Established state is reached.
 *	sES -> sES	:-)
 *	sFW -> sCW	Normal close request answered by ACK.
 *	sCW -> sCW
 *	sLA -> sTW	Last ACK detected (RFC5961 challenged)
 *	sTW -> sTW	Retransmitted last ACK. Remain in the same state.
 *	sCL -> sCL
 */
/*		 sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*rst*/    { sIV, sCL, sCL, sCL, sCL, sCL, sCL, sCL, sCL, sCL },
/*none*/   { sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV }
	}
	,
	{
/* REPLY */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*syn*/	   { sIV, sS2, sIV, sIV, sIV, sIV, sIV, sSS, sIV, sS2 },
/*
 *	sNO -> sIV	Never reached.
 *	sSS -> sS2	Simultaneous open
 *	sS2 -> sS2	Retransmitted simultaneous SYN
 *	sSR -> sIV	Invalid SYN packets sent by the server
 *	sES -> sIV
 *	sFW -> sIV
 *	sCW -> sIV
 *	sLA -> sIV
 *	sTW -> sSS	Reopened connection, but server may have switched role
 *	sCL -> sIV
 */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*synack*/ { sIV, sSR, sIG, sIG, sIG, sIG, sIG, sIG, sIG, sSR },
/*
 *	sSS -> sSR	Standard open.
 *	sS2 -> sSR	Simultaneous open
 *	sSR -> sIG	Retransmitted SYN/ACK, ignore it.
 *	sES -> sIG	Late retransmitted SYN/ACK?
 *	sFW -> sIG	Might be SYN/ACK answering ignored SYN
 *	sCW -> sIG
 *	sLA -> sIG
 *	sTW -> sIG
 *	sCL -> sIG
 */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*fin*/    { sIV, sIV, sFW, sFW, sLA, sLA, sLA, sTW, sCL, sIV },
/*
 *	sSS -> sIV	Server might not send FIN in this state.
 *	sS2 -> sIV
 *	sSR -> sFW	Close started.
 *	sES -> sFW
 *	sFW -> sLA	FIN seen in both directions.
 *	sCW -> sLA
 *	sLA -> sLA	Retransmitted FIN.
 *	sTW -> sTW
 *	sCL -> sCL
 */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*ack*/	   { sIV, sIG, sSR, sES, sCW, sCW, sTW, sTW, sCL, sIG },
/*
 *	sSS -> sIG	Might be a half-open connection.
 *	sS2 -> sIG
 *	sSR -> sSR	Might answer late resent SYN.
 *	sES -> sES	:-)
 *	sFW -> sCW	Normal close request answered by ACK.
 *	sCW -> sCW
 *	sLA -> sTW	Last ACK detected (RFC5961 challenged)
 *	sTW -> sTW	Retransmitted last ACK.
 *	sCL -> sCL
 */
/* 	     sNO, sSS, sSR, sES, sFW, sCW, sLA, sTW, sCL, sS2	*/
/*rst*/    { sIV, sCL, sCL, sCL, sCL, sCL, sCL, sCL, sCL, sCL },
/*none*/   { sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV, sIV }
	}
};


typedef enum rtk_fc_devtx_e {
	RTK_FC_DEVTX_OK = NETDEV_TX_OK,	 		// = 0x00,	/* driver took care of packet */
	RTK_FC_DEVTX_BUSY =NETDEV_TX_BUSY,		// = 0x10,	/* driver tx path was busy*/
	/*RTK_FC_DEVTX_LOCKED = NETDEV_TX_LOCKED, 	// = 0x20,*/	/* driver tx lock was already taken */
	RTK_FC_DEVTX_ERROR,
}rtk_fc_devtx_t;

typedef enum rtk_fc_mgr_smp_static_type_e
{
	FC_MGR_SMP_STATIC_GMAC_RX = 0,
	FC_MGR_SMP_STATIC_GMAC_RX_SKIP_FC, //trap by ACL, skip FC, forward to PS directly
	FC_MGR_SMP_STATIC_WIFI_RX,
#if defined(CONFIG_RTK_NIC_HWLOOKUP_DEAMSDU_OFFLOAD)
	FC_MGR_SMP_STATIC_WIFI_AMSDU_RX,
#endif
	FC_MGR_SMP_STATIC_IPI_TO_FC_RX,	// count for IPI de-quqeq
	FC_MGR_SMP_STATIC_FROM_PS_RX_SKIP_FC, //skb_mark.fwd_byPS || ((skb_shinfo(skb)->nr_frags != 0) && (skb->fcIngressData.doLearning == FALSE))
	FC_MGR_SMP_STATIC_IPI_TO_FC_RX_DROP,	// count for packet drop due to full of IPI queue 
	FC_MGR_SMP_STATIC_IPI_BAND0_HW_LOOKUP_DROP,	// count for packet drop due to full of IPI queue
	FC_MGR_SMP_STATIC_IPI_BAND1_HW_LOOKUP_DROP,	// count for packet drop due to full of IPI queue
	FC_MGR_SMP_STATIC_IPI_BAND2_HW_LOOKUP_DROP,	// count for packet drop due to full of IPI queue

	FC_MGR_SMP_STATIC_GMAC0_TX,	//9607C: GMAC9 ;	G3: 0x10
	FC_MGR_SMP_STATIC_GMAC1_TX,	//9607C: GMAC10 ;	G3: 0x11
	FC_MGR_SMP_STATIC_GMAC2_TX,	//9607C: GMAC10 ;	G3: 0x12
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	FC_MGR_SMP_STATIC_GMAC3_TX,	//					G3: 0x13
	FC_MGR_SMP_STATIC_GMAC4_TX,	//					G3: 0x14
	FC_MGR_SMP_STATIC_GMAC5_TX,	//					G3: 0x15
	FC_MGR_SMP_STATIC_GMAC6_TX,	//					G3: 0x16
	FC_MGR_SMP_STATIC_GMAC7_TX,	//					G3: 0x17
#endif
	FC_MGR_SMP_FC_IPI_TO_NIC_TX_DROP,

	FC_MGR_SMP_PS_NETIF_RX,
	FC_MGR_SMP_PS_NETIF_RX_DROP,

	FC_MGR_SMP_STATIC_IPI_TO_WIFI_BAND0_TX_DROP,	// count for packet drop due to full of IPI queue
	FC_MGR_SMP_STATIC_IPI_TO_WIFI_BAND1_TX_DROP,	// count for packet drop due to full of IPI queue
	FC_MGR_SMP_STATIC_IPI_TO_WIFI_BAND2_TX_DROP,	// count for packet drop due to full of IPI queue
	FC_MGR_SMP_WIFI_BAND0_TX,
	FC_MGR_SMP_WIFI_BAND1_TX,
	FC_MGR_SMP_WIFI_BAND2_TX,
	FC_MGR_SMP_WIFI_BANDx_TX_FF,
	FC_MGR_SMP_WIFI_BANDx_TX_FF_AGG,
	FC_MGR_SMP_WIFI_BAND0_TX_DROP,
	FC_MGR_SMP_WIFI_BAND1_TX_DROP,
	FC_MGR_SMP_WIFI_BAND2_TX_DROP,
	FC_MGR_SMP_WIFI_BAND0_TXQ_STOPPED_TIMES,
	FC_MGR_SMP_WIFI_BAND1_TXQ_STOPPED_TIMES,
	FC_MGR_SMP_WIFI_BAND2_TXQ_STOPPED_TIMES,
	FC_MGR_SMP_RPS_CPU_DISTRIBUTE,
	FC_MGR_SMP_STA_OFFLINE_AMSDU_DROP,

	FC_MGR_SMP_STATIC_TYPE_MAX,
}rtk_fc_mgr_smp_static_type_t;

typedef struct rtk_fc_mgr_smp_static_s
{
	atomic_t smp_static[NR_CPUS];
}rtk_fc_mgr_smp_static_t;

typedef enum rtk_fc_mgr_skbmarkTarget_e
{
       FC_MGR_SKBMARK_QID = 0,
       FC_MGR_SKBMARK_STREAMID,
       FC_MGR_SKBMARK_STREAMID_EN,
       FC_MGR_SKBMARK_SKIPFCFUNC,

       FC_MGR_SKBMARK_END
}rtk_fc_mgr_skbmarkTarget_t;

typedef enum rtk_fc_mgr_config_type_e
{
	// compile flag
	FC_MGR_GET_CONFIG_RT_API = 0,
	FC_MGR_GET_CONFIG_FLOW_LOCK_GROUP_SIZE,
	FC_MGR_GET_CONFIG_LOOPBACK_MODE,
	FC_MGR_GET_CONFIG_PROBE_LOG,
	FC_MGR_GET_CONFIG_PORT4_HSGMII_EN,
	FC_MGR_GET_CONFIG_SPECIAL_FAST_FORWARD,
	FC_MGR_GET_CONFIG_HWNAT_CUSTOMIZE,
	FC_MGR_GET_CONFIG_HWNAT_CUSTOMIZE_NPTV6_SRAM_ACC_V2,
	FC_MGR_GET_CONFIG_PER_SW_FLOW_MIB,
	FC_MGR_GET_CONFIG_GMAC2_USABLE,
	FC_MGR_GET_CONFIG_MEMLEAK_DEBUG,
	FC_MGR_GET_CONFIG_EXTERNAL_SWITCH_ENABLE,
	FC_MGR_GET_CONFIG_EXTERNAL_SWITCH_PORT_OFFSET,
	FC_MGR_GET_CONFIG_VLAN_FILTERING,
	FC_MGR_GET_CONFIG_IPSEC_FASTFWD,

	// global setting: support set
	FC_MGR_GLB_CONFIG_WIFI_RX_HASH,
	FC_MGR_GLB_CONFIG_WIFI_RX_GMAC_AUTO_SEL,
	FC_MGR_GLB_CONFIG_WIFI_TX_TRAP_HASH,
	FC_MGR_GLB_CONFIG_WIFI_TX_GMAC_AUTO_SEL,
	FC_MGR_GLB_CONFIG_WIFI_GMAC_TRUNKING_NUM,
	FC_MGR_GLB_CONFIG_PON_DS_P7_LOOPBACK,
	FC_MGR_GET_CONFIG_MAX
}rtk_fc_mgr_config_type_t;

#if defined(CONFIG_FC_WIFI_GMAC_TRUNKING_NUM_3)
#define GMAC_TRUNKING_NUM        3
#else
#define GMAC_TRUNKING_NUM        2
#endif

typedef struct rtk_fc_fdb_entry_s
{
	unsigned long	updated;
	struct net_device *br_port_netdev;
}rtk_fc_fdb_entry_t;

typedef struct rtk_fc_skbmark_s
{
	int8 startBit;	// -1 indicate function was disabled
	int8 mark1or2;  // 0: mark 1; 1: mark 2;
	uint8 len;
}rtk_fc_skbmark_t;

#define RTK_FC_MGR_RMK_UNDEF 		-1

typedef struct rtk_fc_mgr_ring_buf_state_s
{
	uint32 free_idx;
	uint32 sched_idx;
}rtk_fc_mgr_ring_buf_state_t;
typedef struct rtk_fc_mgr_rps_map_s {
	int len;
	union{
		int cpus[16];
		int port_to_cpu[RTK_FC_MAC_PORT_MAX];
	};
	uint8 mode;
}rtk_fc_mgr_rps_map_t;


#if 0 // reduce dispatch struct size
typedef enum rtk_fc_dispathmode_e
{
	RTK_FC_DISPATCH_MODE_NONE,		// original_mdoe
	RTK_FC_DISPATCH_MODE_IPI,    		// ipi dispatch mode
	RTK_FC_DISPATCH_MODE_END,
}rtk_fc_dispatchmode_t;
#define rtk_fc_dispatchcpu_t uint32

#else
#define RTK_FC_DISPATCH_MODE_NONE 			0
#define RTK_FC_DISPATCH_MODE_IPI 			1
#define RTK_FC_DISPATCH_MODE_RPS 			2
#define RTK_FC_DISPATCH_MODE_SMP_BY_HASH 	3
#define RTK_FC_DISPATCH_MODE_SMP_BY_RR 		4
#define RTK_FC_DISPATCH_MODE_SMP_BY_ACL		5
#define RTK_FC_DISPATCH_MODE_SMP_BY_PORT	6
#define RTK_FC_DISPATCH_MODE_END 			7
#define RTK_FC_DISPATCH_MODEMSK_NIC_TX		((1 << RTK_FC_DISPATCH_MODE_NONE) | (1 << RTK_FC_DISPATCH_MODE_IPI))
#define RTK_FC_DISPATCH_MODEMSK_WIFI_RX		((1 << RTK_FC_DISPATCH_MODE_NONE) | (1 << RTK_FC_DISPATCH_MODE_IPI))
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
#define RTK_FC_DISPATCH_MODEMSK_NIC_RX		((1 << RTK_FC_DISPATCH_MODE_NONE) | (1 << RTK_FC_DISPATCH_MODE_IPI) | (1 << RTK_FC_DISPATCH_MODE_SMP_BY_ACL) | (1 << RTK_FC_DISPATCH_MODE_RPS) | (1 << RTK_FC_DISPATCH_MODE_SMP_BY_PORT))
#define RTK_FC_DISPATCH_MODEMSK_WIFI_TX		((1 << RTK_FC_DISPATCH_MODE_NONE) | (1 << RTK_FC_DISPATCH_MODE_IPI) | (1 << RTK_FC_DISPATCH_MODE_SMP_BY_HASH) | (1 << RTK_FC_DISPATCH_MODE_SMP_BY_RR))
#else
#define RTK_FC_DISPATCH_MODEMSK_NIC_RX		((1 << RTK_FC_DISPATCH_MODE_NONE) | (1 << RTK_FC_DISPATCH_MODE_IPI) | (1 << RTK_FC_DISPATCH_MODE_RPS)  | (1 << RTK_FC_DISPATCH_MODE_SMP_BY_PORT))
#define RTK_FC_DISPATCH_MODEMSK_WIFI_TX		((1 << RTK_FC_DISPATCH_MODE_NONE) | (1 << RTK_FC_DISPATCH_MODE_IPI))
#endif
#define RTK_FC_DISPATCH_NEED_SW_RING_BUF	((1 << RTK_FC_DISPATCH_MODE_IPI) | (1 << RTK_FC_DISPATCH_MODE_RPS) | (1 << RTK_FC_DISPATCH_MODE_SMP_BY_PORT))
#define rtk_fc_dispatchmode_t uint16
#define rtk_fc_dispatchcpu_t uint16
#endif

typedef struct rtk_fc_mgr_dispatch_mode_s
{
	rtk_fc_dispatchmode_t mode;
	rtk_fc_dispatchcpu_t smp_id; // cpu core ID for RTK_FC_DISPATCH_MODE_IPI, CPU mask for RTK_FC_DISPATCH_MODE_SMP_BY_HASH/RTK_FC_DISPATCH_MODE_SMP_BY_RR
}rtk_fc_mgr_dispatch_mode_t;
#define rtk_fc_dispatch_mode_t rtk_fc_mgr_dispatch_mode_t

#define RTK_FC_RPS_DISPATCH_MODE_ORIGINAL 0
#define RTK_FC_RPS_DISPATCH_MODE_PORT 1
#define RTK_FC_RPS_DISPATCH_MODE_END 2
typedef struct rtk_fc_mgr_rps_mode_s
{
	uint16 mode;
	uint16 cpu_bitMsk;
}rtk_fc_mgr_rps_mode_t;

typedef enum rtk_fc_mgr_dispatch_point_e
{
	RTK_FC_MGR_DISPATCH_START = 0,
	RTK_FC_MGR_DISPATCH_NIC_RX = RTK_FC_MGR_DISPATCH_START,
	RTK_FC_MGR_DISPATCH_HIGHPRI_NIC_RX,
	RTK_FC_MGR_DISPATCH_NIC_TX,
	RTK_FC_MGR_DISPATCH_PS_NETIF_RX,
	RTK_FC_MGR_DISPATCH_WLAN0_RX,
	RTK_FC_MGR_DISPATCH_WLAN1_RX,
	RTK_FC_MGR_DISPATCH_WLAN2_RX,
	RTK_FC_MGR_DISPATCH_WLAN0_TX,
	RTK_FC_MGR_DISPATCH_WLAN1_TX,
	RTK_FC_MGR_DISPATCH_WLAN2_TX,
	RTK_FC_MGR_DISPATCH_MAX,
}rtk_fc_mgr_dispatch_point_t;
#define rtk_fc_dispatch_point_t rtk_fc_mgr_dispatch_point_t

#define RTK_FC_MGR_DISPATCH_ARRAY_SIZE RTK_FC_MGR_DISPATCH_MAX+1

typedef enum rtk_fc_mgr_dispatch_mode_bitmask_e
{
	RTK_FC_MGR_DISPATCH_MASK_NONE 	= 0,
	RTK_FC_MGR_DISPATCH_MASK_MODE 	= (1<<0),
	RTK_FC_MGR_DISPATCH_MASK_CPU 	= (1<<1),
	RTK_FC_MGR_DISPATCH_MASK_ALL	= 0xffffffff,
}rtk_fc_mgr_dispatch_mode_mask_t;
#define rtk_fc_dispatch_mode_mask_t rtk_fc_mgr_dispatch_mode_mask_t

typedef struct rtk_fc_skipHwlookUp_stat_s
{
	char status;
	rtk_fc_pmap_t portInfo;
	unsigned long last_jiffies;
}rtk_fc_skipHwlookUp_stat_t;

typedef struct rtk_fc_converter_api_s
{
	int (*skb)(struct sk_buff *skb, struct rt_skbuff *rtskb);
	int (*ct)(struct nf_conn *ct, struct rt_nfconn *rtct);

}rtk_fc_converter_api_t;

typedef struct rtk_fc_ps_api_s
{
	int (*ct_get)(struct sk_buff *skb, struct nf_conn **ct);
	int (*ct_is_dying)(struct nf_conn *ct);
	unsigned long (*ct_status_get)(struct nf_conn *ct);
	int (*ct_expiretime_get)(struct nf_conn *ct, unsigned long *time);
	int (*ct_protonum_get)(struct nf_conn *ct, unsigned char *protonum);
	int (*ct_session_alive_check)(struct nf_conn *ct, bool *alive);
	int (*ct_helper_exist_check)(struct nf_conn *ct, struct nf_conntrack_helper **helper);
	int (*ct_helper_is_conenat_check)(struct nf_conntrack_helper *helper);
	int (*ct_timer_refresh)(struct nf_conn *ct);
	int (*ct_flush)(void);
	int (*ct_tcp_liberal_set)(struct nf_conn *ct);
	int (*ct_rt_5tuple_info_get)(struct nf_conn *ct, rt_nf_5tuple_info_t *nf_tuple_info);
	int (*ct_rt_tcp_state_get)(struct nf_conn *ct, rt_tcp_conntrack_state_t *rt_tcp_state);
	int (*ct_rt_tcp_state_set)(struct nf_conn *ct, rt_tcp_conntrack_state_t rt_tcp_state);
	int (*nf_ct_put)(struct nf_conntrack *nfct);

	int (*br_allowed_ingress)(struct net_bridge *br, struct net_bridge_port *p, struct sk_buff *skb, unsigned short *vid, bool *allow);
	char (*br_multicast_disabled_get)(struct net_bridge* br);
	char (*br_multicast_querier_get)(struct net_bridge* br);
	int (*fdb_time_refresh)(struct net_device *br_netdev, unsigned char *mac, unsigned short vid);
	int (*fdb_time_get)(struct net_device *br_netdev, unsigned char *mac, unsigned short vid, rtk_fc_fdb_entry_t *fc_fdb);

	int (*neighv4_exist_check)(unsigned char *mac);
	int (*neighv6_exist_check)(unsigned char *mac);

	int (*neighv4_enumerate)(void (*cb)(struct neighbour *, void *), void *cookie);
	int (*neighv6_enumerate)(void (*cb)(struct neighbour *, void *), void *cookie);

	char* (*neigh_key_get)(struct neighbour *neigh);
	int (*neigh_key_nud_compare)(struct neighbour *neigh, rtk_fc_helper_neigh_key_type_t type, void *cookie, unsigned char state);
	int (*neigh_nud_state_update)(struct neighbour *neigh, unsigned char state, unsigned int flag);

	int (*skb_eth_type_trans)(struct sk_buff *skb, struct net_device *dev);
	int (*skb_netif_rx)(struct sk_buff *skb);
	int (*skb_netif_rx_with_hook)(struct sk_buff *skb);	
	int (*skb_netif_receive_skb)(struct sk_buff *skb);
	int (*skb_dev_alloc_skb)(unsigned int length, struct sk_buff **skb);
	int (*skb_dev_allocCritical_skb)(unsigned int length, struct sk_buff **skb);
	int (*skb_swap_recycle_skb)(struct sk_buff *skb, struct sk_buff **newskb);
	int (*skb_dev_kfree_skb_any)(struct sk_buff *skb);
	int (*skb_skb_push)(struct sk_buff *skb, unsigned int len, unsigned char **pData);
	int (*skb_skb_pull)(struct sk_buff *skb, unsigned int len, unsigned char **pData);
	int (*skb_skb_put)(struct sk_buff *skb, unsigned int len, unsigned char **pData);
	int (*skb_skb_cloned)(struct sk_buff *skb);
	int (*skb_skb_clone)(struct sk_buff *skb, gfp_t gfp_mask, struct sk_buff **newskb);
	int (*skb_skb_copy)(struct sk_buff *skb, gfp_t gfp_mask, struct sk_buff **newskb);	
	int (*skb_skb_cow_head)(struct sk_buff *skb, unsigned int headroom);
	int (*skb_skb_trim)(struct sk_buff *skb, unsigned int len);

	int (*skb_skb_reset_mac_header)(struct sk_buff *skb);
	int (*skb_skb_set_network_header)(struct sk_buff *skb, const int offset);
	int (*skb_skb_set_transport_header)(struct sk_buff *skb, const int offset);

	int (*fc_skb_set_sw_hash)(struct sk_buff *skb);
	int (*fc_skb_set_l4_hash)(struct sk_buff *skb);
	int (*fc_skb_set_priority)(struct sk_buff *skb,uint32 priority);

	int (*skb_ip_summed_set)(struct sk_buff *skb, uint8 value);
	int (*skb_ip_summed_get)(struct sk_buff *skb, uint8* value);
	int (*skb_param_set)(struct sk_buff *skb, rtk_fc_skb_param_t param ,uint32 value);
	int (*skb_param_get)(struct sk_buff *skb, rtk_fc_skb_param_t param ,uint32 *value);

	int (*dev_get_by_index)(struct net *net, int ifindex, struct net_device **dev);
	void* (*dev_get_priv)(struct net_device *dev);

	int (*dev_get_priv_flags)(struct net_device *dev, unsigned int *privflags);
	bool (*dev_is_priv_flags_set)(struct net_device *dev, rtk_fc_netdev_priv_flag_type_t flag);
	int (*dev_get_realdev_phyport)(struct net_device *dev, rtk_fc_realdev_t *rdev);
	int (*dev_get_dev_type)(struct net_device *dev, unsigned int *devtype);
	bool (*dev_netif_running)(struct net_device *dev);
	void (*dev_dev_hold)(struct net_device *dev);
	void (*dev_dev_put)(struct net_device *dev);
	struct net_device* (*vlan_dev_get_real_dev)(struct net_device *dev);

	int (*fc_copy_from_user)(char *procBuffer, const char __user * userbuf, size_t count);
#if 0// not supprot	
	int (*mc_mdb_scan)(struct net_device *br_netdev,
		int (*nbrport2pmsk)(struct net_bridge_port *, unsigned int *, rtk_fc_wlan_devmask_t *),
		int (*exec)(unsigned int , unsigned int *, unsigned int, rtk_fc_wlan_devmask_t, int, unsigned char, unsigned int, rtk_fc_wlan_devmask_t, int , unsigned char,unsigned int,int,unsigned int,unsigned int *));
	int (*mc_mdb_lookup)(struct net_device *br_netdev, bool isIPv6, unsigned int *mcGIP, unsigned int vid,
		int (*nbrport2pmsk)(struct net_bridge_port *, unsigned int *,  rtk_fc_wlan_devmask_t *),
		unsigned int *pmsk, rtk_fc_wlan_devmask_t *);
#endif

	int (*pid_task)(struct task_struct **pTask, struct pid *pid, enum pid_type type);
	char* (*task_comm_get)(struct task_struct *pTask);
	struct signal_struct * (*task_signal_get)(struct task_struct *pTask);
	struct tty_struct * (*task_signal_tty_get)(struct task_struct *pTask);
	int (*find_vpid)(struct pid **pPid, int nr);
#if defined(CONFIG_FC_RTL8198F_SERIES)
	int (*dslite_ipv6_fragment)(struct sk_buff *skb, struct net_device *dev, ca_ni_tx_config_t *tx_config,
			netdev_tx_t (*xmit)(struct sk_buff *, struct net_device *, ca_ni_tx_config_t *));
#endif

	int (*mape_get_ipid)(struct nf_conn *ct, uint16 *ipid);

	int (*hw_csum)(char *dev_name);
	
	int (*fc_helper_spin_lock_init)(spinlock_t* lock);
	int (*fc_helper_spin_lock)(spinlock_t* lock);
	int (*fc_helper_spin_unlock)(spinlock_t* lock);
	int (*fc_helper_spin_lock_bh)(spinlock_t* lock);
	int (*fc_helper_spin_unlock_bh)(spinlock_t* lock);
	int (*fc_helper_spin_lock_bh_irq_protect)(spinlock_t* lock);
	int (*fc_helper_spin_unlock_bh_irq_protect)(spinlock_t* lock);

	int (*fc_synchronize_rcu)(void);
	int (*fc_call_rcu)(struct rcu_head *head, rcu_callback_t func);
	int (*fc_rcu_read_lock)(void);
	int (*fc_rcu_read_unlock)(void);
	int (*fc_rcu_get_bridgePort_by_dev)(const struct net_device *dev, struct net_bridge_port **p);
	int (*fc_helper_get_bridgeDev_by_bridgePort)(struct net_bridge_port *p, struct net_device** dev);
	int (*fc_helper_get_bridge_by_bridgePort)( struct net_bridge_port *p, struct net_bridge** br);
	int (*fc_helper_get_bridgePort_pvid)( struct net_bridge_port *p, int *pvid);
	int (*fc_helper_get_fdb_by_pvid)( struct net_device *br_netdev, unsigned char *mac, int pvid, rtk_fc_fdb_entry_t *fc_fdb);
	unsigned long (*fc_helper_bridge_port_flags_get)(struct net_bridge_port *p);
	bool (*bridge_port_exists)(struct net_device *br_netdev);
	int (*fc_helper_mc_kernel_snooping_disable_get)(struct net_bridge* br);
	
	int (*fc_rcu_in6_dev_get)(struct net_device* dev, struct inet6_dev **in6_dev);
	int (*fc_rcu_indev_get_addr)(struct net_device* dev, int *ipAddr);

	int (*fc_helper_netdev_get_if_info)(struct net_device* dev, struct in_ifaddr **if_info);
	int (*fc_helper_netdev_ifa_global_addr6_set)(struct inet6_dev *in6_dev, uint32 *global_ipv6_addr_set);
	int (*fc_helper_netdev_set_nf_cs_checking)(struct net_device **dev);
	int (*fc_helper_netdev_notifierInfo_to_dev)(void *ptr, struct net_device **dev);
	int (*fc_helper_netdev_ifav6_to_dev)(void *ptr, struct net_device **dev);
	int (*fc_helper_netdev_ifa_to_dev)(void *ptr, struct net_device **dev);
	int (*fc_helper_netdev_set_nfTbl_macAddr)(struct net_device* dev, uint8 *octet);
	int (*fc_helper_netdev_get_dev_data)(struct net_device *dev, unsigned int *data, rtk_fc_netdevice_getData_type_t data_type);
	int (*fc_helper_netdev_check_vxlan_dev)(struct net_device *dev);
	int (*fc_helper_netdev_get_vxlan_dev_vni)(struct net_device *dev, int *vni);
	
	struct net_device* (*fc_helper_first_net_device_get)(struct net *net);
	struct net_device* (*fc_helper_next_net_device_get)(struct net_device *dev);
	int (*fc_helper_netdev_compare_v6addr_by_v6AddrHash)(struct net_device *dev, uint32 v6AddrHash,uint8 direct);
	int (*fc_helper_netdev_compare_v6addr_by_v6Addr)(struct net_device *dev, struct	in6_addr *saddr);
	
	int (*fc_helper_netdev_ignore_lookup)(struct net_device *dev, unsigned long event);

	int (*fc_helper_proc_file_seq_file_setting)(struct file *file, struct seq_file *p, const struct seq_operations *op);
	int (*fc_helper_proc_file_set_private_data)(struct file *file, void *data);
	int (*fc_helper_proc_file_set_private_data_buf)(struct file *file, char *buf, size_t size);
	unsigned int (*fc_helper_proc_file_get_inode_id)(struct seq_file *s);
	struct proc_dir_entry* (*fc_helper_proc_mkdir)(const char *name, struct proc_dir_entry *parent);
	int (*fc_helper_proc_single_open)(struct file *file, int (*show)(struct seq_file *, void *),void *data);
	int (*fc_helper_proc_single_open_size)(struct file *file, int (*show)(struct seq_file *, void *),		void *data, size_t size);

	int (*fc_helper_netlink_get_nlhdr_fromSKBData)(struct sk_buff *skb, struct nlmsghdr **nlh);
	int (*fc_helper_netlink_get_net_from_sock)(struct sock *sk, struct net **net);
	int (*fc_helper_netlink_set_sock_data)(struct sock *sk, void	(*sk_data_ready)(struct sock *sk), gfp_t sk_allocation);
	int (*fc_helper_msecs_to_jiffies)(int msecs, unsigned long int *jiffies);
	int (*fc_helper_jiffies_to_secs)(unsigned long int jiffies, int *secs);
	int (*fc_helper_local_bh_disable)(void);
	int (*fc_helper_local_bh_enable)(void);
	void *(*fc_helper_fc_kmalloc)(size_t size, gfp_t gfp_flag);
	int (*fc_helper_fc_kfree)(const void *ptr,size_t size);
	int (*fc_helper_timer_timer_pending)(rtk_fc_timer_list_t *pTimer);
	int (*fc_helper_timer_del_timer)(rtk_fc_timer_list_t *pTimer);	
	int (*fc_helper_timer_init_timer)(rtk_fc_timer_list_t *pTimer, void * function);
	int (*fc_helper_timer_mod_timer)(rtk_fc_timer_list_t *pTimer,unsigned long expires);
	int (*fc_helper_timer_setup_timer)(rtk_fc_timer_list_t *pTimer,void * function,unsigned long data);
	int (*fc_helper_timer_data_set)(rtk_fc_timer_list_t *pTimer, unsigned long data);
	unsigned long (*fc_helper_timer_data_get)(unsigned long callbackFunc_data);
	
	int (*fc_helper_irq_set_affinity_hint)(unsigned int irq, const struct cpumask *m);
}rtk_fc_ps_api_t;


typedef struct rtk_fc_mgr_api_s
{
	int (*fc_helper_mgr_nic_tx)(void *pNicTx_privateInfo_data);
	int (*fc_helper_mgr_nic_tx_with_pon_sid)(struct sk_buff *skb, uint32 ponsid);
	int (*fc_helper_mgr_wifi_rx)(struct sk_buff *skb);
	int (*fc_helper_mgr_func_spin_lock_idx_get)(rtk_fc_func_lock_type_t type, uint32 idx);
	int (*fc_helper_mgr_func_spin_lock)(rtk_fc_func_lock_type_t type, uint32 idx);
	int (*fc_helper_mgr_func_spin_unlock)(rtk_fc_func_lock_type_t type, uint32 idx);
	int (*fc_helper_mgr_global_spin_lock)(void);
	int (*fc_helper_mgr_global_spin_unlock)(void);
	int (*fc_helper_mgr_global_spin_lock_bh)(void);
	int (*fc_helper_mgr_global_spin_unlock_bh)(void);
	int (*fc_helper_mgr_global_spin_lock_bh_irq_protect)(void);
	int (*fc_helper_mgr_global_spin_unlock_bh_irq_protect)(void);
	int (*fc_helper_mgr_rtnetlink_timer_spin_lock_bh)(void);
	int (*fc_helper_mgr_rtnetlink_timer_spin_unlock_bh)(void);
	int (*fc_helper_mgr_tracefilter_spin_lock_bh)(void);
	int (*fc_helper_mgr_tracefilter_spin_unlock_bh)(void);
	int (*fc_helper_mgr_debug_config_set)(int value);
	int (*fc_helper_mgr_debug_config_get)(int *value);
	int (*fc_helper_mgr_mirror_config_set)(bool if_enable, uint8 mirrored_port);
	int (*fc_helper_mgr_mirror_config_get)(bool *if_enable, uint8 *mirrored_port);
#if defined(CONFIG_SMP)
	
	int (*fc_helper_mgr_dispatchMode_set)(rtk_fc_dispatch_mode_t dispatch_mode, uint32 mask, rtk_fc_dispatch_point_t dispatch_point, int *cpu_port_mapping_array);
	int (*fc_helper_mgr_dispatchMode_get)(rtk_fc_dispatch_mode_t *dispatch_mode, rtk_fc_dispatch_point_t dispatch_point);
	int (*fc_mgr_dispatch_smp_mode_wifi_tx_cpuId_get)(uint8 *cpuId, rtk_fc_dispatchmode_t *wifi_tx_dispatch_mode, uint32 hashIdx, rtk_fc_dispatch_point_t dispatch_point);
	int (*fc_helper_mgr_ring_buffer_state_get)(rtk_fc_mgr_dispatch_point_t func, rtk_fc_mgr_ring_buf_state_t *state);
	int (*fc_helper_mgr_ingress_rps_get)(uint16 *fc_rps_bit_mask, uint16 *mode, int *cpu_port_mapping_array);
	int (*fc_helper_mgr_ingress_rps_set)(uint16 mode, uint16 cpu_bit_mask, int *cpu_port_mapping_array);
#endif

	int (*fc_helper_mgr_smp_statistic_get)(rtk_fc_mgr_smp_static_t *smp_statics, rtk_fc_mgr_smp_static_type_t type);
	int (*fc_helper_mgr_smp_statistic_set)(bool flag);
	int (*fc_helper_mgr_meminfo_dump)(void);

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	int (*fc_helper_mgr_wifi_flow_crtl_info_get)(rtk_fc_wifi_flow_crtl_info_t *wifi_flow_crtl_info);
	int (*fc_helper_mgr_wifi_flow_crtl_info_set)(bool if_en);
	int (*fc_helper_mgr_wifi_flow_crtl_info_clear)(void);
#endif

	int (*fc_helper_mgr_epon_llid_format_set)(int formatConfig);
	int (*fc_helper_mgr_hwnat_mode_set)(rt_flow_hwnat_mode_t hwnat_mode);
	int (*fc_helper_mgr_wan_port_mask_set)(rtk_fc_port_mask_t portmask);
	int (*fc_helper_mgr_br0_ip_mac_set)(uint32 ip_addr, uint8 *mac_addr, uint8 flag);

	int (*fc_helper_mgr_skbmark_conf_set)(rtk_fc_mgr_skbmarkTarget_t target, rtk_fc_skbmark_t skbmark_conf);

	int (*fc_helper_mgr_sw_flow_mib_add)(int flowTblIdx, rt_flow_op_sw_flow_mib_info_t counts);
	int (*fc_helper_mgr_sw_flow_mib_get)(int flowTblIdx, rt_flow_op_sw_flow_mib_info_t *counts);
	int (*fc_helper_mgr_sw_flow_mib_clear)(int flowTblIdx);
	int (*fc_helper_mgr_sw_flow_mib_clearAll)(void);
	int (*fc_mgr_proc_fops_size)(void);
	void *(*fc_mgr_proc_fops_kmalloc)(void);
	void (*fc_mgr_proc_fops_kfree)(void *pProc_fops);
	void (*fc_mgr_proc_fops_open_set)(void *pProc_fops, void *pFunc);
	void (*fc_mgr_proc_fops_write_set)(void *pProc_fops, void *pFunc);
	void (*fc_mgr_proc_fops_read_set)(void *pProc_fops, void *pFunc);
	void (*fc_mgr_proc_fops_lseek_set)(void *pProc_fops, void *pFunc);
	void (*fc_mgr_proc_fops_release_set)(void *pProc_fops, void *pFunc);
	struct proc_dir_entry *(*fc_mgr_proc_create_data)(const char *name, umode_t mode,
															struct proc_dir_entry *parent,
															void *pProc_fops, void *data);
	unsigned int (*fc_mgr_proc_inode_id_get)(struct proc_dir_entry *pProc_dir);
	
	void *(*fc_helper_mgr_timer_list_kmalloc)(void);
	void (*fc_helper_mgr_timer_list_kfree)(rtk_fc_timer_list_t *timer);
	int (*fc_helper_mgr_skipHwlookUp_stat_set)(rtk_fc_skipHwlookUp_stat_t skipHwlookUp_stat);
	int (*fc_helper_mgr_skipHwlookUp_stat_get)(rtk_fc_skipHwlookUp_stat_t *skipHwlookUp_stat);
	int (*fc_helper_mgr_skipHwlookUp_stat_clear)(void);
	int (*fc_helper_mgr_glb_config_set)(rtk_fc_mgr_config_type_t glbConfig, void *config);
	int (*fc_helper_mgr_check_config_setting)(rtk_fc_mgr_config_type_t type);
	int (*fc_helper_mgr_system_post_init)(void);
}rtk_fc_mgr_api_t;


typedef struct rtk_fc_rt_helper_api_s{
	/*RT API HOOK FUNCTION*/
	int (*fc_helper_rt_ext_mapper_register)(rt_ext_mapper_t* pMapper);
	int (*fc_helper_rt_rate_shareMeter_mapping_hw_get)(uint32 v_index, rt_rate_meter_mapping_t* pMeterMap);
	int (*fc_helper_rt_cls_fwdPort_set)(rt_port_t port, rt_port_t fwdPort);
	int (*fc_helper_rt_switch_mode_get)(rt_switch_mode_t *mode);

}rtk_fc_rt_helper_api_t;

typedef struct rtk_fc_multi_wan_api_s
{
	int (*rtk_get_vid_by_dev)(unsigned short *vid, struct net_device *dev);
	int (*rtk_fc_decide_rx_device_by_spa)(unsigned int phy_src_port, struct net_device **dev);
#if defined(CONFIG_RTK_SOC_RTL8198D)
	int (*rtk_get_bind_portmask)(int vid,unsigned long long *bind_port,int (*ndevice2portmask)(struct net_device*, unsigned int *,rtk_fc_wlan_devmask_t *));
#else	
	int (*rtk_get_bind_portmask)(int vid,unsigned int *bind_port,int (*ndevice2portmask)(struct net_device*, unsigned int *,rtk_fc_wlan_devmask_t *));
#endif
}rtk_fc_multi_wan_api_t;

typedef struct rtk_fc_vlan_api_s
{
	int (*rtk_vlan_passthrough_tx_handle)(struct sk_buff **pskb);
#if defined(CONFIG_RTK_SOC_RTL8198D)
	int (*rtk_linux_vlan_rx_handle)(struct sk_buff **pskb, uint32 portmask);
	int (*rtk_linux_vlan_tx_handle)(struct sk_buff **pskb, uint32 portmask);
#endif
}rtk_fc_vlan_api_t;

typedef struct rtk_fc_wlan_api_s
{
	int (*wlan_dev_is_wifidev)(struct net_device *dev, bool *wlandev);
	int (*wlan_rtmbssid2devidx)(rt_wlan_index_t wlanIdx, rt_wlan_mbssid_index_t mbssidIdx, rtk_fc_wlan_devidx_t *wlanDevIdx);
	int (*wlan_rtmbssidMask2devMask)(rt_wlan_mbssid_mask_t *rtWlanMbssidMsk,rtk_fc_wlan_devmask_t *wlanDevIdMask);
	int (*wlan_dev_register)(rtk_fc_wlan_devidx_t wlanDevIdx, struct net_device *dev);
	int (*wlan_dev_unregister)(struct net_device *dev);
	char *(*wlan_devmap_devname_get)(rtk_fc_wlan_devmap_t *devmap);
	int (*wlan_devmap_macportidx_get)(rtk_fc_wlan_devmap_t *devmap);
	int (*wlan_devmap_macextportidx_get)(rtk_fc_wlan_devmap_t *devmap);
	int (*wlan_devmap_shareextport_get)(rtk_fc_wlan_devmap_t *devmap);
	int (*wlan_devmap_band_get)(rtk_fc_wlan_devmap_t *devmap);
	int (*wlan_devmap_get)(rtk_fc_wlan_devidx_t wlanDevIdx, rtk_fc_wlan_devmap_t **devmap);
	int (*wlan_devmap_initmap_dump)(void);
	int (*wlan_devmap_initmap_set)(rtk_fc_wlan_initmap_t wlanConfig);
	int (*wlan_devidx_to_bandidx)(rtk_fc_wlan_devidx_t wlanDevIdx);
	int (*wlan_devidx_to_dev)(rtk_fc_wlan_devidx_t wlan_dev_idx, struct net_device **pWifiDev);
	int (*wlan_device_is_running)(rtk_fc_wlan_devidx_t wlan_dev_idx);
	int (*wlan_dev_to_devIdx)(struct net_device *dev, rtk_fc_wlan_devidx_t *wlan_dev_idx);
	int (*wlan_devName_to_devIdx)(char *devName, rtk_fc_wlan_devidx_t *wlanDevIdx, rtk_fc_wlan_devmask_t *wlanDevIdMask);
	int (*wlan_port_to_devIdx)(rtk_fc_mac_port_idx_t macPort, rtk_fc_mac_ext_port_idx_t macExtPort, rtk_fc_wlan_devidx_t *wlanDevIdx, unsigned long long *wlanDevIdMask);
	int (*wlan_cpuport_to_devIdx)(rtk_fc_mac_port_idx_t macPort, rtk_fc_wlan_devidx_t *wlanDevIdx, rtk_fc_wlan_devmask_t *wlanDevIdMask);
	int (*wlan_devidx_to_portidx)(rtk_fc_wlan_devidx_t wlanDevIdx, rtk_fc_mac_port_idx_t *macPort, rtk_fc_mac_ext_port_idx_t *macExtPort);
	bool (*wlan_client_mode_cb_is_registered)(void);
	int (*is_wlan_5g)(char *devname);
	int (*wlan_devmask_to_extportmask)(rtk_fc_wlan_devmask_t wlanDevIdMask, rtk_fc_ext_port_mask_t *extPortMask);
	int (*wlan_devmask_to_rtmbssidMask)(rtk_fc_wlan_devmask_t *wlanDevIdMask, rt_wlan_mbssid_mask_t *rtWlanMbssidMsk);
	int (*wlan_tx_qos_mapping_set)(uint8 enabled, uint8 mappingTbl[8]);
	int (*wlan_rx_lookup_gmac_decision)(uint32 spa, uint32 *gmacid);
#if defined(CONFIG_FC_RTL9607C_SERIES) && defined(CONFIG_RTK_SOC_RTL8198D) && !defined(CONFIG_FC_WIFI_RX_NONE)
	void (*wlan_rx_gmac_reset)(void);
	void (*wlan_rx_gmac_set)(int port, int mode);
#endif
	rtk_fc_devtx_t (*wlan_dev_hard_start_xmit)(rtk_fc_wlan_devidx_t wlan_dev_idx, struct sk_buff *skb, fc_rx_info_t *pRxInfo);
	int (*wlan_system_post_init)(void);

	void (*wlan_set_usbname)(struct net_device *dev);
	void (*set_wfo_portid)(struct net_device *dev);
	
}rtk_fc_wlan_api_t;

typedef struct rtk_ext_phy_qos_s
{
	int (*rtk_set_ext_phy_qos_portprimap)(void);
}rtk_ext_phy_qos_t;

typedef struct rtk_fc_init_all_mac_portmask_s
{
	unsigned int (*rtk_init_all_mac_portmask_without_cpu)(void);
}rtk_fc_init_all_mac_portmask_t;

#if defined(CONFIG_FC_RTL8198F_SERIES)
typedef struct rtk_fc_83xx_qos_api_s
{
	int (*rtk_get_83xx_qos_status)(void);
	void (*rtk_83xx_qos_set_vlan_tag)(unsigned char *egrCVlanTagif, unsigned short *egrCVID, unsigned short direction, struct sk_buff *skb);
	int (*rtk_pass_cos_to_83xx_qos)(unsigned int cos, struct sk_buff **skb, int portMask, int wanPortMask, int direction);
}rtk_fc_83xx_qos_api_t;

typedef struct rtk_fc_8367r_relay_mc_api_s
{
	int (*rtk_l2_set_8367_ipMcastAddr)(rtk_fc_l2_mcast_members_t *pP3, unsigned int l2mcgid,unsigned int wan_portMask,unsigned int ipv6);
	int (*rtk_fc_get_portNumber_in_8367)(unsigned int portnum,unsigned int wan_portMask);
	int (*rtk_fc_check_8367Port)(u32 port_number,u32 wan_portMask);
	int (*rtk_l2_del_8367_ipMcastAddr)(u32 dip, u32 sip, u32 vid);
	int (*rtk_l2_del_8367_ip6McastAddr)(u32 * mcAddr,u32 vid);
	int (*rtk_l2_8367_igmp_protocol_init)(void);
}rtk_fc_8367r_relay_mc_api_t;
#endif

typedef struct rtk_fc_mcast_api_s
{
	int (*rtk_fc_igmp_mdb_hdler)(unsigned char isIpv6,unsigned int *groupip,unsigned int *srcip,short igrCvlan,short igrSvlan,rtk_fc_table_mcConfigTbl_t *mcCfg);
	int (*rtk_fc_igmp_mdb_isReg)(void);
#if defined(CONFIG_RTK_SOC_RTL8198D)	
	int (*rtk_fc_check_user_group)(unsigned char * skbData);
	int (*rtk_check_user_group_flow)(unsigned int*groupip,unsigned short isIpv6,unsigned int* sourceip,unsigned int sourcePort,unsigned int destPort);
#endif	
}rtk_fc_mcast_api_t;


#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_RTL8198XB_SERIES)

typedef struct rtk_fc_ext_flow_mib_s
{
	uint8 (*rtk_fc_get_ext_flow_mib_control)(void);
	uint8 (*rtk_fc_set_ext_flow_mib_control)(uint8 value);
	int (*rtk_fc_print_ext_flow_mib_count_all)(void);
#if defined(CONFIG_RTK_SOC_RTL8198D)
	uint8 (*rtk_fc_get_ext_flow_mib_control_mode)(void);
	uint8 (*rtk_fc_set_ext_flow_mib_control_mode)(uint8 value);
	uint8 (*rtk_fc_set_ext_flow_mib_debug)(uint32 value);
	int (*rtk_fc_skb_egress_count_in_ext_flow_mib)(struct sk_buff *skb, uint8 is_wlan_sta, bool is_uplink, uint32 *pIdx);
	int (*rtk_fc_add_ext_flow_mib_count_by_lan_ip)(rtk_fc_ext_flow_mib_host_mapping_t *pHost, rtk_fc_ext_flow_mib_net_mapping_t *pNet);
	int (*rtk_fc_add_eth_ext_flow_mib_count_by_lan_ip)(rtk_fc_ext_flow_mib_host_mapping_t *pHost, rtk_fc_ext_flow_mib_net_mapping_t *pNet);
	int (*rtk_fc_add_ext_flow_mib_count_by_lan_mac)(unsigned char *mac_addr, unsigned char is_wlan_sta);
	int (*rtk_fc_delete_ext_flow_mib_count_by_lan_ip)(rtk_fc_ext_flow_mib_host_mapping_t *pHost);
	int (*rtk_fc_delete_ext_flow_mib_count_by_lan_mac)(unsigned char *mac_addr, unsigned char is_wlan_sta);
	int (*rtk_fc_flush_ext_flow_mib_count_by_lan_ip)(rtk_fc_ext_flow_mib_host_mapping_t *pHost);
	int (*rtk_fc_flush_ext_flow_mib_count_by_lan_mac)(unsigned char *mac_addr, unsigned char is_wlan_sta);
	int (*rtk_fc_get_ext_flow_mib_count_by_lan_ip)(uint32 ip_addr, uint8 is_wlan_sta, uint32 *rx_bytes, uint32 *tx_bytes, uint32 *rx_pkts, uint32 *tx_pkts, uint8 flag);
	int (*rtk_fc_get_ext_flow_mib_count_by_lan_mac)(unsigned char *mac_addr, uint8 is_wlan_sta, uint32 *rx_bytes, uint32 *tx_bytes, uint32 *rx_pkts, uint32 *tx_pkts, uint8 flag);
	int (*rtk_fc_delete_ext_flow_mib_count_and_flow_by_lan_ip)(rtk_fc_ext_flow_mib_host_mapping_t *pHost);
	int (*rtk_fc_delete_ext_flow_mib_count_and_flow_by_lan_mac)(unsigned char *mac_addr, unsigned char is_wlan_sta);
	int (*rtk_fc_update_ext_flow_mib_by_lan_ip)(rtk_fc_ext_flow_mib_host_mapping_t *pHost, rtk_fc_ext_flow_mib_param_t *pParam);
	int (*rtk_fc_update_ext_flow_mib_by_lan_mac)(unsigned char *mac_addr, unsigned char is_wlan_sta, rtk_fc_ext_flow_mib_param_t *pParam);
#else
	int (*rtk_fc_add_ext_flow_mib_count_by_lan_mac)(unsigned char *mac_addr);
	int (*rtk_fc_delete_ext_flow_mib_count_by_lan_mac)(unsigned char *mac_addr);
	int (*rtk_fc_flush_ext_flow_mib_count_by_lan_mac)(unsigned char *mac_addr);
	int (*rtk_fc_get_ext_flow_mib_count_by_lan_mac)(unsigned char *mac_addr, uint32 *rx_bytes, uint32 *tx_bytes, uint32 *rx_pkts, uint32 *tx_pkts);
	int (*rtk_fc_delete_ext_flow_mib_count_and_flow_by_lan_mac)(unsigned char *mac_addr);
#endif
}rtk_fc_ext_flow_mib_t;
#endif

typedef struct rtk_fc_ipfrag_helper_s
{
	int (*rtk_fc_get_ipfrag_max_hash_size)(void);
	int (*rtk_fc_get_ipfrag_max_hash_entry_size)(void);
	int (*rtk_fc_get_ipfrag_cache_timeout_msec)(void);
}rtk_fc_ipfrag_helper_t;

#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_FC_RTL8198F_SERIES) || defined(CONFIG_RTL8198X_SERIES)
typedef struct rtk_fc_tcp_helper_s
{
	int (*rtk_fc_tcp_in_window)(struct sk_buff *skb, struct nf_conn *fc_cached_ct, uint32 is_ipv4, uint8 *l3h, int l3h_offset, struct tcphdr *tcph);
}rtk_fc_tcp_helper_t;
int rtk_fc_tcp_in_window(struct sk_buff *skb, struct nf_conn *fc_cached_ct, uint32 is_ipv4, uint8 *l3h, int l3h_offset, struct tcphdr *tcph);
#endif


int rtk_fc_mgr_system_post_init(void);

int _rtk_fc_set_skbMark_vlaue(struct sk_buff *skb, rtk_fc_mgr_skbmarkTarget_t target, int32 value);
int _rtk_fc_get_skbMark_vlaue(struct sk_buff *skb, rtk_fc_mgr_skbmarkTarget_t target);

int rtk_fc_neigh_for_each_read(struct neigh_table *tbl, int (*cb)(struct neighbour *, unsigned char *), unsigned char *mac);
int rtk_fc_neigh_for_each_write(struct neigh_table *tbl, int (*cb)(struct neighbour *, unsigned char *), unsigned char *mac);
int rtk_fc_ct_get(struct sk_buff *skb, struct nf_conn **ct);
int rtk_fc_ct_is_dying(struct nf_conn *ct);
unsigned long rtk_fc_ct_status_get(struct nf_conn *ct);
int rtk_fc_ct_expiretime_get(struct nf_conn *ct, unsigned long *time);
int rtk_fc_ct_protonum_get(struct nf_conn *ct, unsigned char *protonum);
int rtk_fc_ct_session_alive_check(struct nf_conn *ct, bool *alive);
int rtk_fc_ct_helper_exist_check(struct nf_conn *ct, struct nf_conntrack_helper **helper);
int rtk_fc_ct_helper_is_conenat_check(struct nf_conntrack_helper *helper);
int rtk_fc_ct_timer_refresh(struct nf_conn *ct);
int rtk_fc_ct_flush(void);
int rtk_fc_ct_tcp_liberal_set(struct nf_conn *ct);
int rtk_fc_ct_rt_5tuple_info_get(struct nf_conn *ct, rt_nf_5tuple_info_t *nf_tuple_info);
int rtk_fc_nf_ct_put(struct nf_conntrack *nfct);
int rtk_fc_ct_rt_tcp_state_get(struct nf_conn *ct, rt_tcp_conntrack_state_t *rt_tcp_state);
int rtk_fc_ct_rt_tcp_state_set(struct nf_conn *ct, rt_tcp_conntrack_state_t rt_tcp_state);

int rtk_fc_br_allowed_ingress(struct net_bridge *br, struct net_bridge_port *p, struct sk_buff *skb,	u16 *vid, bool *allow);
char rtk_fc_br_multicast_disabled_get(struct net_bridge* br);
char rtk_fc_br_multicast_querier_get(struct net_bridge* br);

int rtk_fc_fdb_time_refresh(struct net_device *br_netdev, unsigned char *mac, unsigned short vid);
int rtk_fc_fdb_time_get(struct net_device *br_netdev, unsigned char *mac, unsigned short vid, rtk_fc_fdb_entry_t *fc_fdb);
int rtk_fc_neighv4_exist_check(unsigned char *mac);
int rtk_fc_neighv6_exist_check(unsigned char *mac);
int rtk_fc_neighv4_enumerate(void (*cb)(struct neighbour *, void *), void *cookie);
int rtk_fc_neighv6_enumerate(void (*cb)(struct neighbour *, void *), void *cookie);
char *rtk_fc_neigh_key_get(struct neighbour *neigh);
int rtk_fc_neigh_key_nud_compare(struct neighbour *neigh, rtk_fc_helper_neigh_key_type_t type, void *cookie, unsigned char state);
int rtk_fc_neigh_nud_state_update(struct neighbour *neigh, unsigned char state, unsigned int flag);

int rtk_fc_skb_dev_alloc_skb(unsigned int length, struct sk_buff **skb);
int rtk_fc_skb_dev_allocCritical_skb(unsigned int length, struct sk_buff **skb);
int rtk_fc_skb_swap_recycle_skb(struct sk_buff *skb, struct sk_buff **newskb);
int rtk_fc_skb_dev_kfree_skb_any(struct sk_buff *skb);
int rtk_fc_skb_eth_type_trans(struct sk_buff *skb, struct net_device *dev);
int rtk_fc_skb_netif_rx(struct sk_buff *skb);
int rtk_fc_skb_netif_rx_with_hook(struct sk_buff *skb);
int rtk_fc_skb_netif_receive_skb(struct sk_buff *skb);
int rtk_fc_skb_skb_push(struct sk_buff *skb, unsigned int len, unsigned char **pData);
int rtk_fc_skb_skb_pull(struct sk_buff *skb, unsigned int len, unsigned char **pData);
int rtk_fc_skb_skb_put(struct sk_buff *skb, unsigned int len, unsigned char **pData);
int rtk_fc_skb_skb_cloned(struct sk_buff * skb);
int rtk_fc_skb_skb_clone(struct sk_buff *skb, gfp_t gfp_mask, struct sk_buff **newskb);
int rtk_fc_skb_skb_copy(struct sk_buff *skb, gfp_t gfp_mask, struct sk_buff **newskb);
int rtk_fc_skb_skb_cow_head(struct sk_buff *skb, unsigned int headroom);
int rtk_fc_skb_skb_trim(struct sk_buff *skb, unsigned int len);
int rtk_fc_skb_skb_reset_mac_header(struct sk_buff *skb);
int rtk_fc_skb_skb_set_network_header(struct sk_buff *skb, const int offset);
int rtk_fc_skb_skb_set_transport_header(struct sk_buff *skb, const int offset);
int rtk_fc_skb_ip_summed_set(struct sk_buff *skb, uint8 value);
int rtk_fc_skb_ip_summed_get(struct sk_buff *skb, uint8 *value);
int rtk_fc_skb_param_set(struct sk_buff *skb, rtk_fc_skb_param_t param ,uint32 value);
int rtk_fc_skb_param_get(struct sk_buff *skb, rtk_fc_skb_param_t param ,uint32 *value);
	
int rtk_fc_dev_get_by_index(struct net *net, int ifindex, struct net_device **dev);

void *rtk_fc_dev_get_priv(struct net_device *dev);

int rtk_fc_dev_get_priv_flags(struct net_device *dev, unsigned int *privflags);
bool rtk_fc_dev_is_priv_flags_set(struct net_device *dev, rtk_fc_netdev_priv_flag_type_t flag);
int rtk_fc_dev_get_realdev_phyport(struct net_device *dev, rtk_fc_realdev_t *rdev);
int _rtk_fc_dev_get_realdev_phyport(struct net_device *dev, rtk_fc_realdev_t *rdev);
int rtk_fc_dev_get_dev_type(struct net_device *dev, unsigned int *devtype);
bool rtk_fc_dev_netif_running(struct net_device *dev);
void rtk_fc_dev_dev_hold(struct net_device *dev);
void rtk_fc_dev_dev_put(struct net_device *dev);
struct net_device* rtk_fc_vlan_dev_get_real_dev(struct net_device *dev);


int rtk_fc_copy_from_user(char *procBuffer, const char __user * userbuf, size_t count);

#if 0 //not support
int rtk_fc_mc_mdb_scan(struct net_device *br_netdev,
							int (*nbrport2pmsk)(struct net_bridge_port *, unsigned int *, rtk_fc_wlan_devmask_t *),
							int (*exec)(unsigned int ,unsigned int *,unsigned int, rtk_fc_wlan_devmask_t, int , unsigned char , unsigned int, rtk_fc_wlan_devmask_t, int , unsigned char,unsigned int,int,unsigned int,unsigned int *));
int rtk_fc_mc_mdb_lookup(struct net_device *br_netdev, bool isIPv6, unsigned int *mcGIP, unsigned int vid,
							int (*nbrport2pmsk)(struct net_bridge_port *, unsigned int *, rtk_fc_wlan_devmask_t *),
							unsigned int *pmsk, rtk_fc_wlan_devmask_t *wlanMsk);
#endif

int rtk_fc_pid_task(struct task_struct **pTask, struct pid *pid,enum pid_type type);
char *rtk_fc_get_task_comm(struct task_struct *pTask);
struct signal_struct *rtk_fc_get_task_signal(struct task_struct *pTask);
struct tty_struct *rtk_fc_get_task_signal_tty(struct task_struct *pTask);
int rtk_fc_find_vpid(struct pid **pPid, int nr);

int rtk_get_vid_by_dev(unsigned short *vid, struct net_device *dev);
int rtk_fc_decide_rx_device_by_spa(unsigned int phy_src_port, struct net_device **dev);

#if defined(CONFIG_RTK_SOC_RTL8198D)
int rtk_get_bind_portmask(int vid,unsigned long long *bind_port,int (*ndevice2portmask)(struct net_device*, unsigned int *,rtk_fc_wlan_devmask_t *));
#else
int rtk_get_bind_portmask(int vid,unsigned int *bind_port,int (*ndevice2portmask)(struct net_device*, unsigned int *,rtk_fc_wlan_devmask_t *));
#endif
int rtk_set_ext_phy_qos_portprimap(void);
unsigned int rtk_init_all_mac_portmask_without_cpu(void);


int rtk_vlan_passthrough_tx_handle(struct sk_buff **pskb);
int rtk_set_hw_csum(char *dev_name);

int rtk_fc_spin_lock_init(spinlock_t* lock);
int rtk_fc_spin_lock(spinlock_t* lock);
int rtk_fc_spin_unlock(spinlock_t* lock);
int rtk_fc_spin_lock_bh(spinlock_t* lock);
int rtk_fc_spin_unlock_bh(spinlock_t* lock);
int rtk_fc_spin_lock_bh_irq_protect(spinlock_t* lock);
int rtk_fc_spin_unlock_bh_irq_protect(spinlock_t* lock);

int rtk_fc_mgr_func_spin_lock_idx_get(rtk_fc_func_lock_type_t type, uint32 idx);
int rtk_fc_mgr_func_spin_lock(rtk_fc_func_lock_type_t type, uint32 idx);
int rtk_fc_mgr_func_spin_unlock(rtk_fc_func_lock_type_t type, uint32 idx);
int rtk_fc_mgr_global_spin_lock(void);
int rtk_fc_mgr_global_spin_unlock(void);
int rtk_fc_mgr_global_spin_lock_bh(void);
int rtk_fc_mgr_global_spin_unlock_bh(void);
int rtk_fc_mgr_global_spin_lock_bh_irq_protect(void);
int rtk_fc_mgr_global_spin_unlock_bh_irq_protect(void);
int rtk_fc_mgr_traff_cnt_spin_lock(void);
int rtk_fc_mgr_traff_cnt_spin_unlock(void);
int rtk_fc_mgr_traff_cnt_spin_lock_bh(void);
int rtk_fc_mgr_traff_cnt_spin_unlock_bh(void);
int rtk_fc_mgr_rtnetlink_timer_spin_lock_bh(void);
int rtk_fc_mgr_rtnetlink_timer_spin_unlock_bh(void);
int rtk_fc_mgr_tracefilter_spin_lock_bh(void);
int rtk_fc_mgr_tracefilter_spin_unlock_bh(void);

int rtk_fc_synchronize_rcu(void);
int rtk_fc_call_rcu(struct rcu_head *head, rcu_callback_t func);
int rtk_fc_rcu_read_lock(void);
int rtk_fc_rcu_read_unlock(void);
int rtk_fc_rcu_get_bridgePort_by_dev(const struct net_device *dev, struct net_bridge_port **p);

int rtk_fc_helper_get_bridgeDev_by_bridgePort(struct net_bridge_port *p, struct net_device** dev);
int rtk_fc_helper_get_bridge_by_bridgePort(struct net_bridge_port *p, struct net_bridge** br);
int rtk_fc_helper_get_fdb_by_pvid( struct net_device *br_netdev, unsigned char *mac, int pvid, rtk_fc_fdb_entry_t *fc_fdb);
int rtk_fc_helper_get_bridgePort_pvid(struct net_bridge_port *p, int *pvid);

unsigned long rtk_fc_helper_bridge_port_flags_get(struct net_bridge_port *p);
bool rtk_fc_br_port_exists(struct net_device	*dev);
int rtk_fc_helper_mc_kernel_snooping_disable_get(struct net_bridge* br);



int rtk_fc_rcu_get_in6_dev(struct net_device *dev, struct inet6_dev **in6_dev);
int rtk_fc_rcu_indev_get_addr(struct net_device* dev, int *ipAddr);

int rtk_fc_netdev_get_if_info(struct net_device* dev, struct in_ifaddr **if_info);
int rtk_fc_netdev_ifa_global_addr6_set(struct inet6_dev *in6_dev, uint32 *global_ipv6_addr_set);
int rtk_fc_netdev_set_nf_cs_checking(struct net_device **dev);
int rtk_fc_netdev_notifierInfo_to_Dev(void *ptr, struct net_device	**dev);
int rtk_fc_netdev_ifav6_to_dev(void *ptr, struct net_device	**dev);
int rtk_fc_netdev_ifa_to_dev(void *ptr, struct net_device **dev);
int rtk_fc_netdev_set_nfTbl_macAddr(struct net_device	*dev, uint8 *octet);
int rtk_fc_netdev_get_dev_data(struct net_device *dev, unsigned int *data, rtk_fc_netdevice_getData_type_t data_type);
int rtk_fc_netdev_check_vxlan_dev(struct net_device *dev);
int rtk_fc_netdev_get_vxlan_dev_vni(struct net_device *dev, int *vni);



struct net_device* rtk_fc_first_net_device_get(struct net *net);
struct net_device* rtk_fc_next_net_device_get(struct net_device *dev);

int rtk_fc_netdev_compare_v6addr_by_v6AddrHash(struct net_device *dev, uint32 v6AddrHash, uint8 direct);
int rtk_fc_netdev_compare_v6addr_by_v6Addr(struct net_device *dev, struct	in6_addr	*saddr);

int rtk_fc_netdev_ignore_lookup(struct net_device *dev, unsigned long event);


int rtk_fc_mgr_debug_config_set(int value);
int rtk_fc_mgr_debug_config_get(int *value);
int rtk_fc_mgr_mirror_config_set(bool if_enable, uint8 mirrored_port);
int rtk_fc_mgr_mirror_config_get(bool *if_enable, uint8 *mirrored_port);

#if defined(CONFIG_SMP)

int rtk_fc_mgr_dispatchMode_set(rtk_fc_dispatch_mode_t dispatch_mode, uint32 mask, rtk_fc_dispatch_point_t dispatch_point, int *cpu_port_mapping_array);
int rtk_fc_mgr_dispatchMode_get(rtk_fc_dispatch_mode_t *dispatch_mode, rtk_fc_dispatch_point_t dispatch_point);
int rtk_fc_mgr_dispatch_smp_mode_wifi_tx_cpuId_get(uint8 *cpuId, rtk_fc_dispatchmode_t *wifi_tx_dispatch_mode, uint32 hashIdx, rtk_fc_dispatch_point_t dispatch_point);
int rtk_fc_mgr_ring_buffer_state_get(rtk_fc_mgr_dispatch_point_t func, rtk_fc_mgr_ring_buf_state_t *state);
int rtk_fc_mgr_ingress_rps_set(uint16 mode, uint16 cpu_bit_maps, int *cpu_port_mapping_array);
int rtk_fc_mgr_ingress_rps_get(uint16 *fc_rps_bit_mask, uint16 *mode, int *cpu_port_mapping_array);

#endif

int rtk_fc_mgr_smp_statistic_get(rtk_fc_mgr_smp_static_t *smp_statics, rtk_fc_mgr_smp_static_type_t type);
int rtk_fc_mgr_smp_statistic_set(bool flag);
int rtk_fc_mgr_memInfo_dump(void);

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
int rtk_fc_mgr_wifi_flow_ctrl_info_get(rtk_fc_wifi_flow_crtl_info_t *wifi_flow_crtl_info);
int rtk_fc_mgr_wifi_flow_ctrl_info_set(bool if_en);
int rtk_fc_mgr_wifi_flow_ctrl_info_clear(void);
#endif
int rtk_fc_mgr_trap_to_ps_set(int trap2PsConfig);
int rtk_fc_mgr_epon_llid_format_set(int formatConf);
int rtk_fc_mgr_hwnat_mode_set(rt_flow_hwnat_mode_t hwnat_mode);
int rtk_fc_mgr_wan_port_mask_set(rtk_fc_port_mask_t portmask);
int rtk_fc_mgr_br0_ip_mac_set(uint32 ip_addr, uint8 *mac_addr, uint8 flag);
int rtk_fc_mgr_skbmark_conf_set(rtk_fc_mgr_skbmarkTarget_t target, rtk_fc_skbmark_t skbmark_conf);

int rtk_fc_mgr_sw_flow_mib_add(int flowTblIdx, rt_flow_op_sw_flow_mib_info_t counts);
int rtk_fc_mgr_sw_flow_mib_get(int flowTblIdx, rt_flow_op_sw_flow_mib_info_t *counts);
int rtk_fc_mgr_sw_flow_mib_clear(int flowTblIdx);
int rtk_fc_mgr_sw_flow_mib_clearAll(void);
int rtk_fc_mgr_proc_fops_size(void);
void *rtk_fc_mgr_proc_fops_kmalloc(void);
void rtk_fc_mgr_proc_fops_kfree(void *pProc_fops);
void rtk_fc_mgr_proc_fops_open_set(void *pProc_fops, void *pFunc);
void rtk_fc_mgr_proc_fops_write_set(void *pProc_fops, void *pFunc);
void rtk_fc_mgr_proc_fops_read_set(void *pProc_fops, void *pFunc);
void rtk_fc_mgr_proc_fops_lseek_set(void *pProc_fops, void *pFunc);
void rtk_fc_mgr_proc_fops_release_set(void *pProc_fops, void *pFunc);
struct proc_dir_entry *rtk_fc_mgr_proc_create_data(const char *name, umode_t mode,
															struct proc_dir_entry *parent,
															void *pProc_fops, void *data);
unsigned int rtk_fc_mgr_proc_inode_id_get(struct proc_dir_entry *pProc_dir);


void *rtk_fc_mgr_timer_list_kmalloc(void);
void rtk_fc_mgr_timer_list_kfree(rtk_fc_timer_list_t *timerList);
int rtk_fc_mgr_skipHwlookUp_stat_set(rtk_fc_skipHwlookUp_stat_t skipHwlookUp_stat);
int rtk_fc_mgr_skipHwlookUp_stat_get(rtk_fc_skipHwlookUp_stat_t *skipHwlookUp_stat);
int rtk_fc_mgr_skipHwlookUp_stat_clear(void);
int rtk_fc_mgr_glb_config_set(rtk_fc_mgr_config_type_t glbConfig, void *config);
int rtk_fc_mgr_check_config_setting(rtk_fc_mgr_config_type_t type);


void* rtk_fc_kmalloc(size_t size, gfp_t gfp_flag);
int rtk_fc_kfree(const void *ptr,size_t size);

int rtk_fc_timer_timer_pending(rtk_fc_timer_list_t *pTimer);	
int rtk_fc_timer_del_timer(rtk_fc_timer_list_t *pTimer);
int timer_init_timer(rtk_fc_timer_list_t *pTimer, void * function);	
int rtk_fc_timer_mod_timer(rtk_fc_timer_list_t *pTimer, unsigned long expires);
int rtk_fc_timer_setup_timer(rtk_fc_timer_list_t *pTimer, void * function, unsigned long data);
unsigned long rtk_fc_timer_data_get(unsigned long callbackFunc_data);


int rtk_fc_proc_file_seq_file_setting(struct file *file, struct seq_file *p, const struct seq_operations *op);
int rtk_fc_proc_file_set_private_data(struct file *file, void *data);

int rtk_fc_proc_file_set_private_data_buf(struct file *file, char *buf, size_t size);
unsigned int rtk_fc_proc_file_get_inode_id(struct seq_file *s);
struct proc_dir_entry* rtk_fc_proc_mkdir(const char *name, struct proc_dir_entry *parent);
int rtk_fc_proc_single_open(struct file *file, int (*show)(struct seq_file *, void *),void *data);
int rtk_fc_proc_single_open_size(struct file *file, int (*show)(struct seq_file *, void *),		void *data, size_t size);

int rtk_fc_netlink_get_nlhdr_fromSKBData(struct sk_buff *skb, struct nlmsghdr **nlh);
int rtk_fc_netlink_get_net_from_sock(struct sock *sk, struct net **net);
int rtk_fc_netlink_set_sock_data(struct sock *sk, void	(*sk_data_ready)(struct sock *sk), gfp_t sk_allocation);
int rtk_fc_msecs_to_jiffies(int msecs, unsigned long int *jiffies);
int rtk_fc_jiffies_to_secs(unsigned long int jiffies, int32 *secs);
int rtk_fc_local_bh_disable(void);
int rtk_fc_local_bh_enable(void);
int rtk_fc_irq_set_affinity_hint(unsigned int irq, const struct cpumask *m);

int rtk_fc_wlanDevidx2bandidx(rtk_fc_wlan_devidx_t wlanDevIdx);
int rtk_fc_wlanDevIdx2port(rtk_fc_wlan_devidx_t wlanDevIdx, rtk_fc_mac_port_idx_t *macPort, rtk_fc_mac_ext_port_idx_t *macExtPort);
#if defined(CONFIG_RTK_SOC_RTL8198D)
int rtk_fc_is_wlan_5g(char *devname);
int rtk_fc_wlanDevIdx2devName(rtk_fc_wlan_devidx_t wlan_dev_idx, unsigned char *name);
#endif
int rtk_fc_wlanDevIdx2dev(rtk_fc_wlan_devidx_t wlan_dev_idx, struct net_device **pWifiDev);
int rtk_fc_check_wlan_device_is_running(rtk_fc_wlan_devidx_t wlan_dev_idx);
int rtk_fc_dev2wlanDevIdx(struct net_device *dev, rtk_fc_wlan_devidx_t *wlan_dev_idx);
int _rtk_fc_dev2wlanDevIdx(struct net_device *dev, rtk_fc_wlan_devidx_t *wlan_dev_idx);
int rtk_fc_devName2wlanDevIdx(char *devName, rtk_fc_wlan_devidx_t *wlanDevIdx, rtk_fc_wlan_devmask_t *wlanDevIdMask);
int rtk_fc_port2wlanDevidx(rtk_fc_mac_port_idx_t macPort, rtk_fc_mac_ext_port_idx_t macExtPort, rtk_fc_wlan_devidx_t *wlanDevIdx, rtk_fc_wlan_devmask_t *wlanDevIdMask);
#if defined(CONFIG_RTK_SOC_RTL8198D)
int rtk_fc_port2wlanDevName(rtk_fc_mac_port_idx_t macPort, rtk_fc_mac_ext_port_idx_t macExtPort, unsigned char *dev_name);
#endif
int rtk_fc_cpuport2wlanDevidx(rtk_fc_mac_port_idx_t macPort, rtk_fc_wlan_devidx_t *wlanDevIdx, rtk_fc_wlan_devmask_t *wlanDevIdMask);
int rtk_fc_wlanDevMask2extMask(rtk_fc_wlan_devmask_t wlanDevIdMask, rtk_fc_ext_port_mask_t *extPortMask);
int rtk_fc_wlan_tx_qos_mapping_set(uint8 enabled, uint8 mappingTbl[8]);
int rtk_fc_wlan_rx_lookup_gmac_decision(uint32 spa, uint32 *gmacid);
#if defined(CONFIG_FC_RTL9607C_SERIES) && defined(CONFIG_RTK_SOC_RTL8198D) && !defined(CONFIG_FC_WIFI_RX_NONE)
void rtk_fc_rx_gmac_sel_reset(void);
void rtk_fc_rx_gmac_sel_set(int port, int mode);
#endif
int rtk_fc_wlan_register(rtk_fc_wlan_devidx_t wlanDevIdx, struct net_device *dev);
int _rtk_fc_wlan_register(rtk_fc_wlan_devidx_t wlanDevIdx, struct net_device *dev);
int rtk_fc_wlan_unregister(struct net_device *dev);
char *rtk_fc_wlan_devmap_devname_get(rtk_fc_wlan_devmap_t *devmap);
int rtk_fc_wlan_devmap_macportidx_get(rtk_fc_wlan_devmap_t *devmap);
int rtk_fc_wlan_devmap_macextportidx_get(rtk_fc_wlan_devmap_t *devmap);
int rtk_fc_wlan_devmap_shareextport_get(rtk_fc_wlan_devmap_t *devmap);
int rtk_fc_wlan_devmap_band_get(rtk_fc_wlan_devmap_t *devmap);
int rtk_fc_wlan_devmap_get(rtk_fc_wlan_devidx_t wlanDevIdx, rtk_fc_wlan_devmap_t **devmap);
int rtk_fc_wlan_devmap_initmap_dump(void);
int rtk_fc_wlan_devmap_initmap_set(rtk_fc_wlan_initmap_t wlanConfig);
rtk_fc_devtx_t rtk_fc_wlan_hard_start_xmit(rtk_fc_wlan_devidx_t wlan_dev_idx, struct sk_buff *skb, fc_rx_info_t *pRxInfo);
bool rtk_fc_wlan_client_mode_cb_is_registered(void);

int rtk_fc_dev_is_wlan_dev(struct net_device *dev, bool *wlandev);
#if defined(CONFIG_USB_NET_DRIVERS)
void rtk_fc_wlan_set_usbname(struct net_device *dev);
#endif

#if 1//defined(CONFIG_RTK_WFOAX)
void rtk_fc_set_wfo_portid(struct net_device *dev);
extern int wfo_enable;
#endif /* CONFIG_RTK_WFOAX */

int rtk_fc_wlan_rtmbssid2devidx(rt_wlan_index_t wlanIdx, rt_wlan_mbssid_index_t mbssidIdx, rtk_fc_wlan_devidx_t *wlanDevIdx);
int rtk_fc_wlan_rtmbssidMask2devMask(rt_wlan_mbssid_mask_t *rtWlanMbssidMsk,rtk_fc_wlan_devmask_t *wlanDevIdMask);
int rtk_fc_wlan_devMask2RtmbssidMask(rtk_fc_wlan_devmask_t *wlanDevIdMask, rt_wlan_mbssid_mask_t *rtWlanMbssidMsk);
int _rtk_fc_wlan_devMask2RtmbssidMask(rtk_fc_wlan_devmask_t *wlanDevIdMask, rt_wlan_mbssid_mask_t *rtWlanMbssidMsk);
int rtk_fc_wlan_system_post_init(void);

int rtk_fc_rt_ext_mapper_register(rt_ext_mapper_t* pMapper);
int rtk_fc_rt_rate_shareMeter_mapping_hw_get(uint32 v_index,            rt_rate_meter_mapping_t* pMeterMap);
int rtk_fc_rt_cls_fwdPort_set(rt_port_t port, rt_port_t fwdPort);
int rtk_fc_rt_switch_mode_get(rt_switch_mode_t *mode);

int rtk_fc_converter_skb(struct sk_buff *skb, struct rt_skbuff *rtskb);
int rtk_fc_converter_ct(struct nf_conn *ct, struct rt_nfconn *rtct);

int rtk_fc_skb_set_pkt_type_offset_sw_hash(struct sk_buff *skb);
int rtk_fc_skb_set_pkt_type_offset_l4_hash(struct sk_buff *skb);
int rtk_fc_skb_set_pkt_priority(struct sk_buff *skb,uint32 priority);


int rtk_fc_get_ipfrag_max_hash_size(void);
int rtk_fc_get_ipfrag_max_hash_entry_size(void);
int rtk_fc_get_ipfrag_cache_timeout_msec(void);

#if defined(CONFIG_FC_RTL8198F_SERIES)
int rtk_get_83xx_qos_status(void);
void rtk_83xx_qos_set_vlan_tag(unsigned char *egrCVlanTagif, unsigned short *egrCVID, unsigned short direction, struct sk_buff *skb);
int rtk_pass_cos_to_83xx_qos(unsigned int cos, struct sk_buff **skb, int portMask, int wanPortMask, int direction);

int rtk_fc_dslite_ipv6_fragment(struct sk_buff *skb, struct net_device *dev, ca_ni_tx_config_t *tx_config,
	netdev_tx_t (*xmit)(struct sk_buff *, struct net_device *, ca_ni_tx_config_t *));
#endif
#if defined(CONFIG_RTK_SOC_RTL8198D)
int rtk_linux_vlan_rx_handle(struct sk_buff **pskb, uint32 portmask);
int rtk_linux_vlan_tx_handle(struct sk_buff **pskb, uint32 portmask);

uint8 rtk_fc_get_ext_flow_mib_control_mode(void);
uint8 rtk_fc_set_ext_flow_mib_control_mode(uint8 value);
uint8 rtk_fc_set_ext_flow_mib_debug(uint32 value);
int rtk_fc_skb_ingress_count_in_ext_flow_mib(struct sk_buff *skb, uint8 is_wlan_sta, bool is_uplink, uint32 *pIdx, struct net_device *wlan_dev);
int rtk_fc_skb_egress_count_in_ext_flow_mib(struct sk_buff *skb, uint8 is_wlan_sta, bool is_uplink, uint32 *pIdx);
int rtk_fc_get_local_ip_addr_by_skb(struct sk_buff *skb, bool is_uplink);
int rtk_fc_update_ext_flow_mib_by_lan_ip(rtk_fc_ext_flow_mib_host_mapping_t *pHost, rtk_fc_ext_flow_mib_param_t *pParam);
int rtk_fc_update_ext_flow_mib_by_lan_mac(unsigned char * mac_addr, unsigned char is_wlan_sta, rtk_fc_ext_flow_mib_param_t * pParam);
int rtk_fc_update_ext_flow_mib_by_idx(uint32 mib_idx, rtk_fc_ext_flow_mib_param_t *pParam);
int rtk_fc_update_dev_to_ext_flow_mib(struct sk_buff *skb, bool is_uplink, char *devname);
int rtk_fc_get_ext_flow_mib_idx_by_lan_ip(rtk_fc_ext_flow_mib_host_mapping_t *pHost);
int rtk_fc_get_ext_flow_mib_idx_by_lan_mac(unsigned char *mac_addr, unsigned char is_wlan_sta);
int rtk_fc_add_ext_flow_mib_count_by_lan_ip(rtk_fc_ext_flow_mib_host_mapping_t *pHost, rtk_fc_ext_flow_mib_net_mapping_t *pNet);
int rtk_fc_add_eth_ext_flow_mib_count_by_lan_ip(rtk_fc_ext_flow_mib_host_mapping_t *pHost, rtk_fc_ext_flow_mib_net_mapping_t *pNet);
int rtk_fc_add_ext_flow_mib_count_by_lan_mac(unsigned char * mac_addr, unsigned char is_wlan_sta);
int rtk_fc_delete_ext_flow_mib_count_by_lan_ip(rtk_fc_ext_flow_mib_host_mapping_t *pHost);
int rtk_fc_delete_ext_flow_mib_count_by_lan_mac(unsigned char * mac_addr, unsigned char is_wlan_sta);
int rtk_fc_flush_ext_flow_mib_count_by_lan_ip(rtk_fc_ext_flow_mib_host_mapping_t *pHost);
int rtk_fc_flush_ext_flow_mib_count_by_lan_mac(unsigned char *mac_addr, unsigned char is_wlan_sta);
int rtk_fc_get_ext_flow_mib_count_by_lan_ip(uint32 ip_addr, uint8 is_wlan_sta, uint32 *rx_bytes, uint32 *tx_bytes, uint32 *rx_pkts, uint32 *tx_pkts, uint8 flag);
int rtk_fc_get_ext_flow_mib_count_by_lan_mac(unsigned char *mac_addr, uint8 is_wlan_sta, uint32 *rx_bytes, uint32 *tx_bytes, uint32 *rx_pkts, uint32 *tx_pkts, uint8 flag);
int rtk_fc_delete_ext_flow_mib_count_and_flow_by_lan_ip(rtk_fc_ext_flow_mib_host_mapping_t *pHost);
int rtk_fc_delete_ext_flow_mib_count_and_flow_by_lan_mac(unsigned char *mac_addr, unsigned char is_wlan_sta);
int rtk_fc_update_ext_flow_mib_by_hw_flow_mib(uint32 mib_idx);
void rtk_fc_ext_count_wifi_rx_to_nic_tx_inc(uint8 amsdu, struct net_device	*dev, struct sk_buff *skb, unsigned char *data, uint32 len);
void rtk_fc_ext_count_wifi_rx_to_nic_tx_dec(struct sk_buff *skb, rtk_fc_mac_port_idx_t spa, rtk_fc_mac_ext_port_idx_t extspa);
void rtk_fc_ext_count_nic_rx_to_wifi_tx_inc(struct sk_buff *skb, rtk_fc_mac_port_idx_t spa, uint8 l3routing, rtk_fc_wlan_devidx_t wlandevid);
#if defined(CONFIG_WFO_OFFLOAD_2G)
void rtk_fc_wfo_wq_sched(struct work_struct *work);
int rtk_fc_wfo_wq_start(void);
int rtk_fc_wfo_wq_stop(void);
#endif
#endif

#if defined(CONFIG_RTL8198XB_SERIES)
int rtk_fc_add_ext_flow_mib_count_by_lan_mac(unsigned char * mac_addr);
int rtk_fc_delete_ext_flow_mib_count_by_lan_mac(unsigned char * mac_addr);
int rtk_fc_flush_ext_flow_mib_count_by_lan_mac(unsigned char * mac_addr);
int rtk_fc_get_ext_flow_mib_count_by_lan_mac(unsigned char *mac_addr, uint32 *rx_bytes, uint32 *tx_bytes, uint32 *rx_pkts, uint32 *tx_pkts);
int rtk_fc_delete_ext_flow_mib_count_and_flow_by_lan_mac(unsigned char *mac_addr);
#endif

#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_RTL8198XB_SERIES)
uint8 rtk_fc_get_ext_flow_mib_control(void);
uint8 rtk_fc_set_ext_flow_mib_control(uint8 value);
int rtk_fc_print_ext_flow_mib_count_all(void);
#endif

inline int rtk_fc_is_highpri_pkt(u8 *data);

int rtk_fc_mape_get_ipid(struct nf_conn *ct, uint16 *ipid);

int rtk_fc_helper_init(void);
void rtk_fc_helper_exit(void);





#endif //__RTK_FC_HELPER_H__

