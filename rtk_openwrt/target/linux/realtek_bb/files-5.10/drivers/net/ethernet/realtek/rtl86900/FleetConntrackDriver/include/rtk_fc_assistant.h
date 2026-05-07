#ifndef __RTK_FC_ASSISTANT__
#define __RTK_FC_ASSISTANT__

/*
 * Copyright (C) 2017 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*/



#include <linux/module.h>
#include <../net/bridge/br_private.h>
#include <net/neighbour.h>

#include <rtk_fc_helper.h>
#include <rtk_fc_helper_wlan.h>
#include <rtk_fc_wfo.h>
#include <rt_wlan_ext.h>

#if defined(CONFIG_RTK_FC_CRYPTO_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
typedef struct rtk_fc_crypto_desc
{
    unsigned int first_dw;
    unsigned int second_dw;
} rtk_fc_crypto_desc;

typedef void (*rtk_fc_crypto_hw_crypto_ready_cnt_callback)(unsigned int ready_cnt);
#endif

typedef struct rtk_fc_realdev_s
{
	struct net_device *rdev;
	__u32 physicalPid;
}rtk_fc_realdev_t;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
typedef  struct __call_single_data __call_single_data_t;
#else
typedef  struct call_single_data __call_single_data_t;
#endif

typedef int (*ptrFuncWifiRx_t)(struct sk_buff *skb);

typedef struct rtk_fc_export_symbol_s
{
	int (*_rtk_fc_wlan_register)(rtk_fc_wlan_devidx_t wlanDevIdx, struct net_device *dev);
	int (*_rtk_fc_wlan_devMask2RtmbssidMask)(rtk_fc_wlan_devmask_t *wlanDevIdMask, rt_wlan_mbssid_mask_t *rtWlanMbssidMsk);
	int (*_rtk_fc_dev2wlanDevIdx)(struct net_device *dev, rtk_fc_wlan_devidx_t *wlan_dev_idx);
	int (*_rtk_fc_dev_get_realdev_phyport)(struct net_device *dev, rtk_fc_realdev_t *rdev);
	int (*_rtk_fc_wifi_amsdu_pe_offload_mac_id_set)(unsigned int mac_id, rtk_fc_wifi_amsdu_pe_offload_sta_conf_sel_t sta_conf_sel, rtk_fc_wifi_amsdu_pe_offload_sta_info_t sta_conf, unsigned char* mac_addr);
	int (*_rtk_fc_wifi_amsdu_pe_offload_mac_id_del)(unsigned int mac_id);
	int (*_rtk_fc_wifi_amsdu_pe_offload_mac_id_flush)(void);
	int (*_rtk_fc_wifi_amsdu_pe_offload_mode_set)(rtk_fc_wifi_amsdu_pe_offload_mode_t mode);
	int (*_rtk_fc_wifi_amsdu_pe_offload_mode_get)(rtk_fc_wifi_amsdu_pe_offload_mode_t *mode);
	int (*_rtk_fc_wifi_dev_attr_set)(struct net_device *dev, rtk_fc_wifi_dev_attr_t attr);
	int (*_rtk_fc_wifi_dev_attr_get)(struct net_device *dev, rtk_fc_wifi_dev_attr_t *attr);
	int (*_rtk_fc_wifi_dev_to_devidx_get)(struct net_device *dev, unsigned int *wlan_dev_idx);
	int (*_rtk_fc_wifi_client_mode_cb_register)(rtk_fc_wifi_callback pfunc);

	int (*_rtk_fc_igmp_mdb_callback_register)(fc_igmp_mdb_callback searchMdbFunc);
	int (*_rtk_fc_igmp_mdb_callback_unregister)(void);
	
	int (*_rtk_fc_skb_wifi_rx)(struct sk_buff *skb);
	int (*_rtk_fc_fastfwd_netif_rx)(struct sk_buff *skb);
	gro_result_t (*_rtk_fc_fastfwd_napi_gro_receive)(struct napi_struct *napi, struct sk_buff *skb);
#if defined(CONFIG_RTK_FC_CRYPTO_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
	int (*_rtk_fc_crypto_set_src_desc)(unsigned int first_dw, unsigned int second_dw);
	int (*_rtk_fc_crypto_set_dst_desc)(unsigned int first_dw, unsigned int second_dw, unsigned char start, unsigned char end);
	int (*_rtk_fc_crypto_ps_pop_done)(void);
	int (*_rtk_fc_crypto_send_desc)(void);
	int (*_rtk_fc_crypto_wait_put_desc_done)(void);
	int (*_rtk_fc_crypto_register_hw_crypto_ready_cnt_callback)(rtk_fc_crypto_hw_crypto_ready_cnt_callback cb_func);
#endif
}rtk_fc_export_symbol_t;

int rtk_fc_ext_symbol_register(rtk_fc_export_symbol_t *ext_symbol);

void rtk_fc_ext_dma_cache_wback_inv(unsigned long start, unsigned long sz);
bool rtk_fc_ext_br_allowed_ingress(struct net_bridge *br, struct net_bridge_port *p, struct sk_buff *skb, u16 *vid);
struct net_bridge_fdb_entry *rtk_fc_ext_br_fdb_get(struct net_bridge *br, const unsigned char *addr, __u16 vid);
struct net_bridge_fdb_entry *rtk_fc_ext_br_fdb_get_by_pvid(struct net_bridge *br, const unsigned char *addr, __u16 vid);

int fwdEngine_wifi_rx(struct sk_buff *skb);


int rtk_fc_g_smp_call_function_single_async(int cpu, __call_single_data_t *csd);
struct pid *rtk_fc_g_find_vpid(int nr);
struct inet6_dev *rtk_fc_g_in6_dev_get(const struct net_device *dev);
struct in_device *rtk_fc_g_in_dev_get_rcu(const struct net_device *dev);
struct net_bridge_port *rtk_fc_g_br_port_get_rcu(const struct net_device *dev);
int rtk_fc_g_skb_skb_cow_data(struct sk_buff *skb, int len, struct sk_buff **trailer);
void rtk_fc_g_rcu_read_lock(void);
void rtk_fc_g_rcu_read_unlock(void);
void rtk_fc_g_rcu_read_lock_bh(void);
void rtk_fc_g_rcu_read_unlock_bh(void);
void rtk_fc_g_call_rcu(struct rcu_head *head, rcu_callback_t func);
void rtk_fc_g_synchronize_rcu(void);
int  rtk_fc_g_neigh_for_each_read_v4(int (*cb)(struct neighbour *, unsigned char *), unsigned char *mac);
int  rtk_fc_g_neigh_for_each_read_v6(int (*cb)(struct neighbour *, unsigned char *), unsigned char *mac);
int rtk_fc_g_neighv4_enumerate(void (*cb)(struct neighbour *, void *), void *cookie);
int rtk_fc_g_neighv6_enumerate(void (*cb)(struct neighbour *, void *), void *cookie);
int rtk_fc_g_irq_set_affinity_hint(unsigned int irq, const struct cpumask *m);
void rtk_fc_g_nf_ct_iterate_cleanup(struct net *net, int (*iter)(struct nf_conn *i, void *data), void *data, u32 portid, int report);
#ifdef CONFIG_RTK_FC_TCP_SPI_SUPPORT
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
bool rtk_fc_g_nf_ct_get_tuple(const struct sk_buff *skb,
		unsigned int nhoff,
		unsigned int dataoff,
		u_int16_t l3num,
		u_int8_t protonum,
		struct net *net,
		struct nf_conntrack_tuple *tuple);
#else
bool rtk_fc_g_nf_ct_get_tuple(const struct sk_buff *skb,
	   unsigned int nhoff,
	   unsigned int dataoff,
	   u_int16_t l3num,
	   u_int8_t protonum,
	   struct net *net,
	   struct nf_conntrack_tuple *tuple,
	   const struct nf_conntrack_l3proto *l3proto,
	   const struct nf_conntrack_l4proto *l4proto);
#endif
#endif
int rtk_fc_g_ct_helper_exist_check(struct nf_conn *ct, struct nf_conntrack_helper **helper);

#endif
