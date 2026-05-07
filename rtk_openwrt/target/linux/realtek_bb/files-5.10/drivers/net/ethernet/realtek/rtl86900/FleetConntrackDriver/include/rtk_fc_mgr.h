#ifndef __RTK_FC_MGRINIT__
#define __RTK_FC_MGRINIT__

#include <rtk_fc_helper.h>
#include <rtk_fc_mgrTRx.h>
#include <rtk_fc_helper_wlan.h>
#include <rtk_fc_assistant.h>
#if defined(CONFIG_RTL8686NIC)
#include <re8686_rtl9607c.h>
#endif

#if defined(CONFIG_FC_RTL9607C_RTL9603CVD_HYBRID)
#include <rtk/switch.h>
#include <dal/rtl9607c/dal_rtl9607c_switch.h>
#endif


/*
				 15         k+m-1        m       0
				 +-----------+-----------+-------+
   Ports in 	 |	   A	 |	  PSID	 |	 j	 |
the CE port set  |	  > 0	 |			 |		 |
				 +-----------+-----------+-------+
				 |	a bits	 |	k bits	 |m bits |
　　　　　　　　　　　　　　↑　　　　　↑
					psidEndOffset    psidStartOffset
				 
*/
typedef struct rtk_fc_mapet_hwfmrInfo_s
{
	char devName[32];
	unsigned char fmrv6Prefix[8];
	unsigned char fmrPrefixLen;			/* End-user IPv6 prefix (Rule IPv6 prefix + EA bits)*/
	unsigned char draftVersion;
	unsigned char psidStartOffset;		//psidEndOffset - psidStartOffset >=0
	unsigned char psidEndOffset;
	int hwFmrIdx;				//output
}rtk_fc_mapet_hwfmrInfo_t;

typedef struct rtk_fc_xlat464_Info_s
{
	unsigned char clat_prefixLen;  //local
	unsigned char plat_prefixLen;  //remote
}rtk_fc_xlat464_Info_t;


typedef struct rtk_fc_dual_devInfoConfig
{
   union
   {
	   rtk_fc_mapet_hwfmrInfo_t mape_mapt_hwInfo; //RTK_FC_DUALTYPE_MAPT | RTK_FC_DUALTYPE_MAPE
	   rtk_fc_xlat464_Info_t xlat464_info;
   };
}rtk_fc_dual_devInfoConfig_t;


typedef enum rtk_fc_api_dualHdrtype_e
{
	RTK_FC_API_DUALTYPE_MAPE =1,		//MAPE
	RTK_FC_API_DUALTYPE_MAPT,
	RTK_FC_API_DUALTYPE_XLAT464,
	RTK_FC_API_DUALTYPE_END,
}rtk_fc_api_dualHdrtype_t;


/****************************************/
/* 			User Configuration				*/
/****************************************/

/*
 * IPI global queue size
 */
#define FC_IPSEC_QUEUE_SIZE_OFFSET		8	// 10: 1024
#define FC_NIC_RX_QUEUE_SIZE_OFFSET		CONFIG_RTK_FC_IPI_NIC_RX_RING_BITS		// default 10: 1024
#define FC_NIC_TX_QUEUE_SIZE_OFFSET		CONFIG_RTK_FC_IPI_NIC_TX_RING_BITS		// default 10: 1024
#define FC_WLAN_RX_QUEUE_SIZE_OFFSET		CONFIG_RTK_FC_IPI_WIFI_RX_RING_BITS		// default 12: 4096
#define FC_WLAN_TX_QUEUE_SIZE_OFFSET		CONFIG_RTK_FC_IPI_WIFI_TX_RING_BITS		// default 12: 4096
#define FC_PS_NETIF_RX_QUEUE_SIZE_OFFSET	9	// 9: 512

/*****************************************/

/*
 * NIC Rx: dispatch priority packet to high priority buffer
 */
#define FC_NIC_RX_PRI_TO_HI_QUEUE			6	

#if defined(FC_NIC_RX_PRI_TO_HI_QUEUE) && (FC_NIC_RX_PRI_TO_HI_QUEUE>7)
#ERROR_CONFIG
#endif

/*
 * NIC Tx: dispatch priority packet to corresponding ring buffer
 *   bit[3:0] for internal priority 0, 
 *   bit[7:4] for internal priority 1, 
 *   ...
 *   bit[31:28] for internal priority 7
 */
#define FC_NIC_TX_PRI_TO_RING				0x22110000

/*****************************************/


// IPI queue size: must be power of 2
#define MAX_FC_NIC_RX_HIQUEUE_SIZE			(1<<5)

#define MAX_FC_NIC_RX_QUEUE_SIZE 				(1<<FC_NIC_RX_QUEUE_SIZE_OFFSET)
#define MAX_FC_NIC_TX_QUEUE_SIZE 				(1<<FC_NIC_TX_QUEUE_SIZE_OFFSET) 
#define MAX_FC_WLAN_RX_QUEUE_SIZE 			(1<<FC_WLAN_RX_QUEUE_SIZE_OFFSET) 
#define MAX_FC_WLAN_TX_QUEUE_SIZE 			(1<<FC_WLAN_TX_QUEUE_SIZE_OFFSET)
#define MAX_FC_PS_NETIF_RX_QUEUE_SIZE 		(1<<FC_PS_NETIF_RX_QUEUE_SIZE_OFFSET)

#define FC_IPI_QUEUE_RSVED_CNT				(4)

#define MAX_FC_IPSEC_QUEUE_SIZE 				(1<<FC_IPSEC_QUEUE_SIZE_OFFSET)

/*****************************************/

#define RTK_FC_FLOW_LOCK_GROUP_OFFSET	(6)
#define RTK_FC_FLOW_LOCK_GROUP_SIZE		(1<<RTK_FC_FLOW_LOCK_GROUP_OFFSET)
#define RTK_FC_FLOW_LOCK_CNT				((RTK_FC_TABLESIZE_HW_FLOW/RTK_FC_FLOW_LOCK_GROUP_SIZE)+1)

/*****************************************/
#if CONFIG_NR_CPUS==2
#define RTK_FC_DEFAULT_DISPATCH_CPU 1
#else
#define RTK_FC_DEFAULT_DISPATCH_CPU 2
#endif
/*****************************************/
/*
 *Per cpu ring buffer
 */
typedef struct rtk_fc_nicrx_ring_ctrl_s
{
	rtk_fc_smp_nicRx_work_info_t priv_work[MAX_FC_NIC_RX_QUEUE_SIZE];
}rtk_fc_nicrx_ring_ctrl_t;

typedef struct rtk_fc_nicrx_hiring_ctrl_s
{
	rtk_fc_smp_nicRx_work_info_t priv_work[MAX_FC_NIC_RX_HIQUEUE_SIZE];
}rtk_fc_nicrx_hiring_ctrl_t;

typedef struct rtk_fc_nicrx_ipi_ctrl_s
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
	struct __call_single_data csd ____cacheline_aligned_in_smp;
#else
	struct call_single_data csd ____cacheline_aligned_in_smp;
#endif
	atomic_t csd_available;

	struct tasklet_struct tasklet;
	
	atomic_t priv_free_idx; 
	atomic_t priv_sched_idx; 

	struct tasklet_struct tasklet_empty_check;
	uint8 empty_check;
	
	uint32 priv_work_cnt;
	rtk_fc_smp_nicRx_work_info_t *priv_work;

}rtk_fc_nicrx_ipi_ctrl_t;

typedef struct rtk_fc_mgr_portmask_s
{
	rtk_fc_port_mask_t portmask;
}rtk_fc_mgr_portmask_t;

typedef struct rtk_fc_wifi_tx_smp_mode_cpu_sel_s
{
	uint8 cpu_id[NR_CPUS];
	uint8 valid_cpu_count;
}rtk_fc_wifi_tx_smp_mode_cpu_sel_t;

typedef struct rtk_fc_mgr_database_s
{
	char debug_prk;
	bool smpStatistic;
	rt_flow_hwnat_mode_t hwnat_mode;

#if defined(CONFIG_SMP)
	rtk_fc_mgr_dispatch_mode_t smp_dispatch[RTK_FC_MGR_DISPATCH_ARRAY_SIZE];

#if defined(CONFIG_RTK_L34_FC_IPI_NIC_RX)
	rtk_fc_nicrx_ring_ctrl_t *nicrx_ring[NR_CPUS];
	rtk_fc_nicrx_ipi_ctrl_t *nicrx_ipi[NR_CPUS];
	
	rtk_fc_nicrx_hiring_ctrl_t *nicrx_hiring[NR_CPUS];
	rtk_fc_nicrx_ipi_ctrl_t *nicrx_hi_ipi[NR_CPUS];
#endif

#if defined(CONFIG_RTK_L34_FC_IPI_NIC_TX)
	rtk_fc_smp_ipi_ctrl_t *nictx_ipi[CONFIG_NR_CPUS];

	atomic_t nictx_free_idx[CONFIG_NR_CPUS];
	atomic_t nictx_sched_idx[CONFIG_NR_CPUS];	
	
	rtk_fc_smp_nicTx_work_t *nictx_work[CONFIG_NR_CPUS];
#endif

#if defined(CONFIG_RTK_L34_FC_IPI_WIFI_RX)
	rtk_fc_smp_ipi_ctrl_t *wlanrx_ipi[CONFIG_NR_CPUS];

	atomic_t wlanrx_free_idx[CONFIG_NR_CPUS];
	atomic_t wlanrx_sched_idx[CONFIG_NR_CPUS];
	
	rtk_fc_smp_wlanRx_work_t *wlanrx_work[CONFIG_NR_CPUS];
#endif

#if defined(CONFIG_RTK_L34_FC_IPI_WIFI_TX)
	rtk_fc_smp_ipi_ctrl_t *wlantx_ipi[CONFIG_NR_CPUS];

	atomic_t wlantx_free_idx[CONFIG_NR_CPUS];
	atomic_t wlantx_sched_idx[CONFIG_NR_CPUS];

	rtk_fc_smp_wlanTx_work_t *wlantx_work[CONFIG_NR_CPUS];
#endif

	rtk_fc_smp_ipi_ctrl_t *ps_netif_rx_ipi[CONFIG_NR_CPUS];
	atomic_t ps_netif_rx_free_idx[CONFIG_NR_CPUS];
	atomic_t ps_netif_rx_sched_idx[CONFIG_NR_CPUS];
	rtk_fc_smp_ps_netif_rx_work_t *ps_netif_rx_work[CONFIG_NR_CPUS];

	rtk_fc_mgr_rps_map_t fc_rps_maps;
	unsigned int fc_rps_cpu_bit_mask;
	rtk_fc_wifi_tx_smp_mode_cpu_sel_t wifi_tx_smp_mode_cpu_sel[RTK_FC_WLAN_BAND_NUM];	
#endif

#if defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
	int rtl_fc_skb_cb_value;
#endif
#if defined(CONFIG_RTK_L34_G3_PLATFORM) && defined(CONFIG_FC_QTNA_WIFI_AX)
	bool isWifiFF_skip_dispatch;
#endif

	rtk_fc_mgr_portmask_t wanPortMask;		// set by func: rtk_fc_wan_portmask_set
	uint32 br0_ip;
	uint8 br0_mac[ETHER_ADDR_LEN];

	spinlock_t lock_fc;
	spinlock_t lock_traff_cnt;
	spinlock_t lock_rtnetlinkTimer;
	spinlock_t lock_tracefilterShow;
	/*** HELPER API ***/
	rtk_fc_converter_api_t 	converterapi;
	rtk_fc_ps_api_t 		psapi;
	rtk_fc_mgr_api_t		mgrapi;
	rtk_fc_multi_wan_api_t 	rtk_multi_wan;
	rtk_fc_vlan_api_t 		rtk_vlan;
	rtk_fc_wlan_api_t 		wlanapi;
	rtk_fc_rt_helper_api_t  rt_helper_api;
	rtk_ext_phy_qos_t       rtk_ext_phy_qos;
	rtk_fc_init_all_mac_portmask_t rkt_all_mac_portmask;
#if defined(CONFIG_FC_RTL8198F_SERIES)
	rtk_fc_83xx_qos_api_t 	rtk83xxqos;
	rtk_fc_8367r_relay_mc_api_t rtk_8367r_relay_mc;
#endif
	rtk_fc_mcast_api_t rtk_mcast_api;
#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_RTL8198XB_SERIES)
	rtk_fc_ext_flow_mib_t rtk_ext_flow_mib;
#endif
	rtk_fc_ipfrag_helper_t 	rtk_ipfrag;
#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_FC_RTL8198F_SERIES) || defined(CONFIG_RTL8198X_SERIES)
	rtk_fc_tcp_helper_t rtk_fc_tcp_helper;
#endif
	
	/*** WLAN ACCELERATION ***/
	int wlan_first_ifidex;
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	uint8 wlan_port_to_devidx[MAX_GMAC_NUM][RTK_FC_MAC_EXT_PORT_MAX];
#endif
	rtk_fc_wlan_devmap_t wlanDevMap[RTK_FC_WLANX_END_INTF];
	struct list_head wlanIfidxDevHead[RTK_FC_WLANX_END_INTF];
	struct list_head wlanPortDevHead[RTK_FC_WLAN_PORT_BUCKET_SIZE];

	rtk_fc_mgr_smp_static_t mgr_smp_statistic[FC_MGR_SMP_STATIC_TYPE_MAX];

	rtk_fc_skipHwlookUp_stat_t skipHwlookUp_stat;	// for share ext port dev, if unknown SA packet do HW lookup directly, FC ingress will not be able to do device recovery. The packets will be drop.	
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	rtk_fc_wifi_flow_crtl_info_t wifi_flow_crtl_info;
#if defined(CONFIG_FC_RTL9607C_SERIES)
	uint32 pon_ds_p7_loopback_en:1;				// if enabled, P7/GMAC2 should not be used for by CPU receiving packet.
#endif
#endif
	uint32 wifi_rx_hash_en:1;
	uint32 wifi_rx_gmac_auto_sel_en:1;
	uint32 wifi_rx_gmac_wan_use_rr:1;
	uint32 wifi_rx_gmac_sel_current:2;				// record for RTK_FC_WIFI_RX_GMAC_TBD to decide RTK_FC_WIFI_RX_GMAC_SEL_0 or RTK_FC_WIFI_RX_GMAC_SEL_1
	uint32 wifi_tx_qos_enable:1;
#if defined(CONFIG_FC_RTL9607C_SERIES)
#if defined(CONFIG_RTK_SOC_RTL8198D)
	rtk_fc_timer_list_t *gmac_sel_timer;
#endif
	uint8 wifi_rx_gmac_sel[RTK_FC_MAC_PORT_MAX]; 	// rtk_fc_wifi_gmac_sel_t 
#endif
	uint8 wifi_tx_qos_mapping[8];

#if defined(CONFIG_RTK_FC_PER_SW_FLOW_MIB)
	rt_flow_op_sw_flow_mib_info_t sw_flow_mib[RTK_FC_TABLESIZE_HW_FLOW + RTK_FC_MAX_SHORTCUT_FLOW_SIZE]; // per flow mib
#endif
	rtk_fc_skbmark_t mgr_skbmark[FC_MGR_SKBMARK_END];
#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_RTL8198XB_SERIES)
	rtk_fc_ext_flow_mib_ctrl_t extFlowMibControl;
	rtk_fc_ext_flow_mib_entry_t extFlowMibTbl[RT_STAT_EXT_FLOWMIB_TABLE_SIZE];
#endif

#if defined(CONFIG_FC_RTL9607C_RTL9603CVD_HYBRID)
	uint32 chipId;
	uint8 macport_pon;
	uint8 macport_scpu;
	uint8 macport_mcpu_0;
	uint8 macport_mcpu_1;
	
	uint8 mac10extport_0;
	uint8 mac7extport_0;
#endif
	uint32 mirror_enable:1;
	uint32 mirrored_port:8;

	/*** Functional Lock ***/
	spinlock_t flow_lock[RTK_FC_FLOW_LOCK_CNT];
	spinlock_t fragAcc_lock;
	spinlock_t shapermib_lock;
#if defined(CONFIG_FC_RTL8277C_SERIES)
	spinlock_t flow_overflow_lock; // for all overflow flow
#endif
	spinlock_t dynamic_lock;
	spinlock_t ipsec_lock;

	/*** OTHERS ***/
	void* mgr_null_pointer;
	atomic_t epon_llid_format; //0: remarking LLID as SID;  1: remarking LLID merge qid as whole SID
	char dbgprint_buf[256];

	int (*_rtk_fc_post_nic_skb_rx)(struct sk_buff *skb, fc_rx_info_t *pRxInfo);		// register by rtk_fc_skb_nic_rxhook_register
	int (*_rtk_fc_post_wifi_skb_rx)(struct sk_buff *skb, fc_rx_info_t *pRxInfo);		// register by rtk_fc_skb_wifi_rxhook_register
	int (*_rtk_fc_wifi_client_mode_cb)(struct net_device *dev, struct sk_buff *skb, bool rx);	// register by rtk_fc_wifi_client_mode_cb_register
	int (*_rtk_fc_igmp_mdb_search)(unsigned char isIpv6,unsigned int *groupip,unsigned int *srcip,short igrCvlan,short igrSvlan,rtk_fc_table_mcConfigTbl_t *mcCfg); 		// if untag igrCvlan==-1
	
}rtk_fc_mgr_database_t;


extern rtk_fc_mgr_database_t fc_mgr_db;

#define FCMGR_MOD_NAME	"[FCMGR]"

#define FCMGR_PRK(comment, arg...) \
do {\
	if(unlikely(fc_mgr_db.debug_prk == 1)) { \
		int mt_trace_i;\
		sprintf(fc_mgr_db.dbgprint_buf, comment,## arg);\
		for(mt_trace_i=1;mt_trace_i<256;mt_trace_i++) \
		{ \
		        if(fc_mgr_db.dbgprint_buf[mt_trace_i]==0) \
		        { \
		                if(fc_mgr_db.dbgprint_buf[mt_trace_i-1]=='\n') fc_mgr_db.dbgprint_buf[mt_trace_i-1]=' '; \
		                else break; \
		        } \
		} \
		if(printk_ratelimit()) \
			printk("\033[1;36;40m  "FCMGR_MOD_NAME" %s \033[1;30;40m@%s(%d)\033[0m\n",fc_mgr_db.dbgprint_buf,__FUNCTION__,__LINE__); \
	} \
} while(0)

#define FCMGR_ERR(comment, arg...) \
do {\
	int mt_trace_i;\
	sprintf(fc_mgr_db.dbgprint_buf, comment,## arg);\
	for(mt_trace_i=1;mt_trace_i<256;mt_trace_i++) \
	{ \
	        if(fc_mgr_db.dbgprint_buf[mt_trace_i]==0) \
	        { \
	                if(fc_mgr_db.dbgprint_buf[mt_trace_i-1]=='\n') fc_mgr_db.dbgprint_buf[mt_trace_i-1]=' '; \
	                else break; \
	        } \
	} \
	if(printk_ratelimit()) \
		printk(FCMGR_MOD_NAME" %s @%s(%d)\n",fc_mgr_db.dbgprint_buf,__FUNCTION__,__LINE__); \
} while(0)

#define FCMGR_DBG(comment, arg...) \
do {\
	if(unlikely(fc_mgr_db.debug_prk == 2)) { \
		int mt_trace_i;\
		snprintf(fc_mgr_db.dbgprint_buf, sizeof(fc_mgr_db.dbgprint_buf), comment,## arg);\
		for(mt_trace_i=1;mt_trace_i<256;mt_trace_i++) \
		{ \
				if(fc_mgr_db.dbgprint_buf[mt_trace_i]==0) \
				{ \
						if(fc_mgr_db.dbgprint_buf[mt_trace_i-1]=='\n') fc_mgr_db.dbgprint_buf[mt_trace_i-1]=' '; \
						else break; \
				} \
		} \
		if(printk_ratelimit()) \
			printk("\033[1;36;40m  "FCMGR_MOD_NAME" %s \033[1;30;40m@%s(%d)\033[0m\n",fc_mgr_db.dbgprint_buf,__FUNCTION__,__LINE__); \
	} \
} while(0)

/* =================== */
/*		EXPORT_SYMBOL		*/
/* =================== */

/* Function Name:
*	rtk_fc_skb_rx
* Description:
*	NI rx function
* Input:
*	skb		- sk buffer
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	others  	- error code
*/
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
#if defined(CONFIG_RX_WIFI_FF_AGG)
int rtk_fc_nicRx_to_wifiTx_agg(int gmac, struct sk_buff *skb, struct rx_info *pOriRxInfo);
int rtk_fc_nicRx_to_nicTx_agg(int gmac, struct sk_buff *skb, struct rx_info *pOriRxInfo);
#endif
int rtk_fc_nicRx_to_wifiTx(struct re_private *cp, struct sk_buff *skb, struct rx_info *pOriRxInfo);
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
int rtk_fc_nicRx_to_wifiTx(struct napi_struct *napi,struct net_device *dev, struct sk_buff *skb, nic_hook_private_t *nh_priv);
#endif	

/* Function Name:
*	rtk_fc_skb_rx
* Description:
*	NI rx function
* Input:
*	skb		- sk buffer
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	others  	- error code
*/
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
int rtk_fc_skb_rx(struct re_private *cp, struct sk_buff *skb, struct rx_info *pOriRxInfo);
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
int rtk_fc_skb_rx(struct napi_struct *napi,struct net_device *dev, struct sk_buff *skb, nic_hook_private_t *nh_priv);
#endif

#if defined(CONFIG_FC_RTL9607C_SERIES)
int rtk_fc_nicHook_rx_skb_NPTv6FastForward(struct re_private *cp, struct sk_buff *skb, struct rx_info *pRxInfo);
#endif

/* Function Name:
*	rtk_fc_skb_tx
* Description:
*	NI tx function
* Input:
*	skb		- sk buffer
*	dev		- net device
* Output:
*	N/A
* Return:
*	0		- SUCCESS
*	others  	- error code
*/
int rtk_fc_skb_tx(struct sk_buff *skb, struct net_device *dev);

/* Function Name:
*      rtk_fc_skb_wifi_rx
* Description:
*      wlan rx function
* Input:
*      skb             - sk buffer
* Output:
*      N/A
* Return:
*      0               - SUCCESS
*      others          - error code
*/
int rtk_fc_skb_wifi_rx(struct sk_buff *skb);


/* Function Name:
*	rtk_fc_fastfwd_netif_rx
* Description:
*	wlan rx function - recomment other dev driver call this one instead of fwdEngine_wifi_rx.
* Input:
*	skb		- sk buffer
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	others  	- error code
*/
int rtk_fc_fastfwd_netif_rx(struct sk_buff *skb);

/* Function Name:
*	rtk_fc_fastfwd_napi_gro_receive
* Description:
*	wlan rx function - recomment other dev driver call this one instead of fwdEngine_wifi_rx.
* Input:
*	napi		- napi struct
*	skb		- sk buffer
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	others  	- error code
*/
gro_result_t rtk_fc_fastfwd_napi_gro_receive(struct napi_struct *napi, struct sk_buff *skb);

#if defined(CONFIG_RTK_FC_CRYPTO_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
/* Function Name:
*	rtk_fc_crypto_set_src_desc
* Description:
*	set crypto engine source descriptor.
* Input:
*	src_desc		        - source descriptor
*	end		                - end of source descriptor
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	-1      - error code
*/
int rtk_fc_crypto_set_src_desc(unsigned int first_dw, unsigned int second_dw);
/* Function Name:
*	rtk_fc_crypto_set_dst_desc
* Description:
*	set crypto engine source descriptor.
* Input:
*	src_desc		        - source descriptor
*	end		                - end of source descriptor
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	-1      - error code
*/
int rtk_fc_crypto_set_dst_desc(unsigned int first_dw, unsigned int second_dw, unsigned char start, unsigned char end);
/* Function Name:
*	rtk_fc_crypto_ps_pop_done
* Description:
*	set wait_pop_done to 0.
* Input:
*	N/A
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	-1      - error code
*/
int rtk_fc_crypto_ps_pop_done(void);
/* Function Name:
*	rtk_fc_crypto_send_desc
* Description:
*	send src/dst desc to pe .
* Input:
*	src_desc		        - source descriptor
*	end		                - end of source descriptor
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	-1      - error code
*/
int rtk_fc_crypto_send_desc(void);
/* Function Name:
*	rtk_fc_crypto_wait_put_desc_done
* Description:
*	wait for pe putting desc to hw crypto engine successfully.
* Input:
*	N/A
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	-1      - error code
*/
int rtk_fc_crypto_wait_put_desc_done(void);
/* Function Name:
*	rtk_fc_crypto_register_hw_crypto_ready_cnt_callback
* Description:
*	register call back.
* Input:
*	cb_func		        - cb_func
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	-1      - error code
*/
int rtk_fc_crypto_register_hw_crypto_ready_cnt_callback(rtk_fc_crypto_hw_crypto_ready_cnt_callback cb_func);
#endif

/* Function Name:
*	rtk_fc_fastfwd_dev_xmit
* Description:
*	wlan tx function
* Input:
*	skb		- sk buffer
*	dev		- net device
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	others  	- error code
*/
int rtk_fc_fastfwd_dev_xmit(struct sk_buff *skb, struct net_device *dev);

/*
*	pRxInfo may be meaningless if receiving packet from wifi driver and without hardware lookup.
*/
typedef int (*fc_rx_callback)(struct sk_buff *skb, fc_rx_info_t *pRxInfo);

/* Function Name:
*	rtk_fc_skb_nic_rxhook_register
* Description:
*	register NIC rx function, callback func will be executed after FC ingress learning and before eth_type_trans
* Input:
*	rxfunc	- callback function
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	others  	- error code
*/
int rtk_fc_skb_nic_rxhook_register(fc_rx_callback rxfunc);

/* Function Name:
*	rtk_fc_skb_wifi_rxhook_register
* Description:
*	register WIFI rx function, callback func will be executed after FC ingress learning and before eth_type_trans
* Input:
*	rxfunc	- callback function
* Output:
*	N/A
* Return:
*	0	   	- SUCCESS
*	others  	- error code
*/
int rtk_fc_skb_wifi_rxhook_register(fc_rx_callback rxfunc);

#endif
