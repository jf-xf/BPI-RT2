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
*/

#ifndef __RTK_RG_FWDENGINE_H__
#define __RTK_RG_FWDENGINE_H__

#ifdef __KERNEL__
#include <linux/skbuff.h>
#else
#define VLAN_HLEN       4
#define VLAN_ETH_ALEN	6


struct sk_buff {
	struct sk_buff		*next;
	struct sk_buff		*prev;
	unsigned int		len;
	unsigned char		*data;
	unsigned short		vlan_tci;

};



#endif

#include <rtk_rg_liteRomeDriver.h>
#include <rtk_rg_internal.h>
//#include <rtk_rg_acl.h>
extern struct net_device* decideRxDevice(struct re_private *cp, struct rx_info *pRxInfo);


//#ifdef CONFIG_RTL8686NIC
//extern int rtk_rg_fwdEngine_xmit (struct sk_buff *skb, void *void_ptx, void *void_ptxMask);
//#endif
//extern int _rtk_rg_portBindingLookup(int srcPort, int srcExtPort, int vid, rtk_rg_sipDipClassification_t sipDipClass, rtk_rg_pktHdr_t *pPktHdr);

extern int _rtk_rg_l3lookup(ipaddr_t ip);
extern int _rtk_rg_v6L3lookup(unsigned char *ipv6);



//typedef int (*p_nicTxCallBack)(void*, rtk_rg_txdesc_t*, int);
//typedef unsigned char (*p_nicRxCallBack)(void*, rtk_rg_rxdesc_t*);

extern void rg_proc_init(void);
extern int _rtk_rg_aclDecisionClear(rtk_rg_pktHdr_t *pPktHdr);


#ifdef __KERNEL__
rtk_rg_fwdEngineReturn_t rtk_rg_fwdEngineInput(struct sk_buff *net_buf, rtk_rg_rxdesc_t *pRxDesc);
int rtk_rg_fwdEngine_xmit(struct sk_buff *skb, struct net_device *dev);
int fwdEngine_rx_skb(struct sk_buff *skb, rtk_rg_rxdesc_t *pRxDesc);
int fwdEngine_rx_skb_from_NIC(struct re_private *cp, struct sk_buff *skb,struct rx_info *pRxInfo);
#if defined(CONFIG_RG_FLOW_ENHANCED_WIFI_MODE)
int _rtk_rg_cpu_trap_hash(uint16 sport, uint16 dport);
#endif
int fwdEngine_wifi_rx(struct sk_buff *skb);

#if defined(CONFIG_RG_G3_SERIES)
int fwdEngine_rx_skb_from_g3_NIC(struct napi_struct *napi,struct net_device *dev, struct sk_buff *skb, nic_hook_private_t *nh_priv);
#endif


#ifdef CONFIG_SMP
#if 1
int rtk_rg_fwdEngine_xmit_bh(struct sk_buff *skb, struct net_device *dev);
int _rtk_rg_smp_distributed_NIC_tx(struct sk_buff * skb, struct tx_info * ptxInfo, int ring_num);
int _rtk_rg_smp_send_with_txInfo(struct sk_buff *skb, struct tx_info* ptxInfo, int ring_num);
void _rtk_rg_smp_wifi_11ac_hard_start_xmit(struct sk_buff *skb, struct net_device *dev);
void _rtk_rg_smp_wifi_11n_hard_start_xmit(struct sk_buff *skb, struct net_device *dev);

void _rtk_rg_trigger_rx_tasklet(void *data);
void _rtk_rg_trigger_rx_hi_tasklet(void *data);
void _rtk_rg_trigger_gmac0_tx_tasklet(void *data);
void _rtk_rg_trigger_gmac1_tx_tasklet(void *data);
#if defined(CONFIG_GMAC2_USABLE)
void _rtk_rg_trigger_gmac2_tx_tasklet(void *data);
#endif
void _rtk_rg_trigger_wlan0_tx_tasklet(void *data);
void _rtk_rg_trigger_wlan1_tx_tasklet(void *data);

void _rtk_rg_rx_queue_func(unsigned long data);
void _rtk_rg_rx_hi_queue_func(unsigned long data);
void _rtk_rg_gmac0_tx_func(unsigned long data);
void _rtk_rg_gmac1_tx_func(unsigned long data);
#if defined(CONFIG_GMAC2_USABLE)
void _rtk_rg_gmac2_tx_func(unsigned long data);
#endif
void _rtk_rg_wlan0_tx_func(unsigned long data);
void _rtk_rg_wlan1_tx_func(unsigned long data);
//workqueue
#ifdef RG_BY_TASKLET
void _rtk_rg_inbound_queue_func(struct rg_private *rg_data);
#else
void _rtk_rg_inbound_queue_func(struct work_struct *rg_wq);
#endif


//int rtk_rg_fwdEngine_xmit_bh(struct sk_buff *skb, struct net_device *dev);
int _rtk_rg_smp_distributedWQ_NIC_tx(struct sk_buff *skb, struct tx_info* ptxInfo, int ring_num);

#ifdef OUTBOUND_BY_TASKLET
void _rtk_rg_gmac9_outbound_queue_func(struct rg_gmac9_outbound_queue_private  *rg_nic_data);
void _rtk_rg_gmac10_outbound_queue_func(struct rg_gmac10_outbound_queue_private  *rg_nic_data);

#else
void _rtk_rg_gmac9_outbound_queue_func(struct work_struct *rg_gmac9_wq);
void _rtk_rg_gmac10_outbound_queue_func(struct work_struct *rg_gmac10_wq);
void _rtk_rg_wifi_11ac_outbound_queue_func(struct work_struct *rg_wifi_11ac_wq);
void _rtk_rg_wifi_11n_outbound_queue_func(struct work_struct *rg_wifi_11n_wq);
#endif


int _rtk_rg_smp_distributed_NIC_tx(struct sk_buff * skb, struct tx_info * ptxInfo, int ring_num);
int _rtk_rg_smp_send_with_txInfo(struct sk_buff *skb, struct tx_info* ptxInfo, int ring_num);
void _rtk_rg_smp_wifi_11ac_hard_start_xmit(struct sk_buff *skb, struct net_device *dev);
void _rtk_rg_smp_wifi_11n_hard_start_xmit(struct sk_buff *skb, struct net_device *dev);

#endif
#endif
#endif

#ifdef TIMER_AGG
void _rtk_rg_systemTimerFunc(unsigned long task_priv);
#endif

void _rtk_rg_fwdEngine_captureRxPkts(struct sk_buff* skb);
void _rtk_rg_fwdEngine_captureToPsPkts(struct sk_buff* skb);


int fwdEngine_pkt_tx_testing(void);
#if defined(CONFIG_RG_IPSET_VERSION) && (CONFIG_RG_IPSET_VERSION==1)
uint32 _rtk_rg_ipsetHashIndex(rtk_rg_ipset_hash_key_t hash_key, uint32 value, uint8 *pValue);
#endif
#if defined(CONFIG_RG_IPSET_VERSION) && (CONFIG_RG_IPSET_VERSION==2)
uint32 _rtk_rg_ipsetsHashIndex(rtk_rg_ipsets_set_cfg_t *setCfg, rtk_rg_ipsets_info_t *ipsetsRule);
#endif


/* Function Name:
 *      rtk_rg_tx_setting
 * Description:
 *      Read tx information of netowrk buffer and set to tx descriptor data structure.
 * Input:
 * Output:
 *      buf - [in]<tab>Netowrk buffer.
 *		pTxInfo - [in]<tab>Tx descriptor data structure.<nl>[out]<tab>None.
 * Return:
 *      RT_ERR_RG_OK
 * Note:
 *      buf->l3cs - set to pTxInfo->opts1.bit.cputag_ipcs.<nl>
 *      buf->l4cs - set to pTxInfo->opts1.bit.cputag_l4cs.<nl>
 *      buf->txmsk - set to pTxInfo->opts3.bit.tx_portmask.<nl>
 *      buf->cpu_prisel - set to pTxInfo->opts2.bit.aspri.<nl>
 *      bufvcpu_priority - set to pTxInfo->opts2.bit.cputag_pri.<nl>
 *      buf->keep - set to pTxInfo->opts1.bit.keep.<nl>
 *      skb->l34keep - set to pTxInfo->opts3.bit.L34_keep.<nl>
 *      buf->extspa - set to pTxInfo->opts3.bit.extspa.<nl>
 *      buf->pppoeAct - set to pTxInfo->opts2.bit.tx_pppoe_action.<nl>
 *      buf->pppoeIdx - set to pTxInfo->opts2.bit.tx_pppoe_idx.<nl>
 *      buf->l2br - set to pTxInfo->opts1.bit.blu.<nl>
 */
void _rtk_rg_fwdEngineTxDescSetting(void *pTxInfo,void *ptx,void *ptxMask);

rtk_rg_successFailReturn_t rtk_rg_rome_driver_init(void);


#endif
