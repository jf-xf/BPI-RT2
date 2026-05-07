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

#ifndef __RTK_FC_EXTERNAL__
#define __RTK_FC_EXTERNAL__

/************************************************************
*	HOOK API: RTK_FC_HOOK_[TYPE]_[FUNC]					*
************************************************************/

static inline void RTK_FC_HOOK_CONVERTER_SKB(struct sk_buff *skb, struct rt_skbuff *rtskb)
{
	//rtk_fc_converter_skb;
	fc_db.hook_func.converter->skb(skb, rtskb);
	return;
}

static inline void RTK_FC_HOOK_CONVERTER_CT(struct nf_conn *ct, struct rt_nfconn *rtct)
{
	//rtk_fc_converter_ct
	fc_db.hook_func.converter->ct(ct, rtct);
	return;
}

static inline void RTK_FC_HOOK_PS_CT_GET(struct sk_buff *skb, struct nf_conn **ct)
{

	fc_db.hook_func.ps->ct_get(skb, ct);

	return;
}

static inline int RTK_FC_HOOK_PS_CT_IS_DYING(struct nf_conn *ct)
{
	return fc_db.hook_func.ps->ct_is_dying(ct);
}

static inline unsigned long RTK_FC_HOOK_PS_CT_STATUS_GET(struct nf_conn *ct)
{
	return fc_db.hook_func.ps->ct_status_get(ct);
}

static inline void RTK_FC_HOOK_PS_CT_EXPIRETIME_GET(struct nf_conn *ct, unsigned long *time)
{
	fc_db.hook_func.ps->ct_expiretime_get(ct, time);

	return;
}

static inline void RTK_FC_HOOK_PS_CT_PROTONUM_GET(struct nf_conn *ct, unsigned char *protonum)
{
	fc_db.hook_func.ps->ct_protonum_get(ct, protonum);

	return;
}

static inline void RTK_FC_HOOK_PS_CT_SESSION_ALIVE_CHECK(struct nf_conn *ct, bool *alive)
{
	if(fc_db.hook_func.ps->ct_session_alive_check(ct, alive) != SUCCESS){
		*alive = FALSE;
	}

	return;
}

static inline void RTK_FC_HOOK_PS_CT_HELPER_EXIST_CHECK(struct nf_conn *ct, struct nf_conntrack_helper **helper)
{
	*helper = NULL;

	fc_db.hook_func.ps->ct_helper_exist_check(ct, helper);

	return;
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_CT_HELPER_IS_CONENAT_CHECK( struct nf_conntrack_helper *helper)
{

	return fc_db.hook_func.ps->ct_helper_is_conenat_check(helper);

}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_CT_REFRESH(struct nf_conn *ct)
{	
	return fc_db.hook_func.ps->ct_timer_refresh(ct);
}

static inline rtk_fc_ret_t RTK_FC_HELPER_PS_CT_TCP_LIBERAL_SET(struct nf_conn *ct)
{	
	return fc_db.hook_func.ps->ct_tcp_liberal_set(ct);
}

static inline char RTK_FC_HELPER_PS_BR_MULTICAST_DISABLED_GET(struct net_bridge* br)
{
	return fc_db.hook_func.ps->br_multicast_disabled_get(br);
}

static inline char RTK_FC_HELPER_PS_BR_MULTICAST_QUERIER_GET(struct net_bridge* br)
{
	return fc_db.hook_func.ps->br_multicast_querier_get(br);
}

static inline rtk_fc_ret_t RTK_FC_HELPER_PS_CT_RT_5TUPLE_INFO_GET(struct nf_conn *ct, rt_nf_5tuple_info_t *nf_tuple_info)
{
	return fc_db.hook_func.ps->ct_rt_5tuple_info_get(ct, nf_tuple_info);
}

static inline rtk_fc_ret_t RTK_FC_HELPER_PS_CT_RT_TCP_STATE_GET(struct nf_conn *ct, rt_tcp_conntrack_state_t *rt_tcp_state)
{
	return fc_db.hook_func.ps->ct_rt_tcp_state_get(ct, rt_tcp_state);
}

static inline rtk_fc_ret_t RTK_FC_HELPER_PS_CT_RT_TCP_STATE_SET(struct nf_conn *ct, rt_tcp_conntrack_state_t rt_tcp_state)
{
	return fc_db.hook_func.ps->ct_rt_tcp_state_set(ct, rt_tcp_state);
}

static inline rtk_fc_ret_t RTK_FC_HELPER_PS_CT_NF_CT_PUT(struct nf_conntrack *nfct)
{
	return fc_db.hook_func.ps->nf_ct_put(nfct);
}

// NOTICE: DO NOT hold spinlock since func call might sleep.
static inline void RTK_FC_HOOK_PS_CT_FLUSH(void)
{
	fc_db.hook_func.ps->ct_flush();

	return;
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_FDB_REFRESH(struct net_device *br_netdev, unsigned char *mac, unsigned short vid)
{
	return fc_db.hook_func.ps->fdb_time_refresh(br_netdev, mac, vid);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_FDB_TIME_GET(struct net_device *br_netdev, unsigned char *mac, unsigned short vid, rtk_fc_fdb_entry_t *fc_fdb)
{
	return fc_db.hook_func.ps->fdb_time_get(br_netdev, mac, vid, fc_fdb);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_NEIGHV4_GET(unsigned char *mac)
{
	return fc_db.hook_func.ps->neighv4_exist_check(mac);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_NEIGHV6_GET(unsigned char *mac)
{
	return fc_db.hook_func.ps->neighv6_exist_check(mac);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_NEIGHV4_ENUMERATE(void (*cb)(struct neighbour *, void *), void *cookie)
{
	return fc_db.hook_func.ps->neighv4_enumerate(cb, cookie);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_NEIGHV6_ENUMERATE(void (*cb)(struct neighbour *, void *), void *cookie)
{
	return fc_db.hook_func.ps->neighv6_enumerate(cb, cookie);
}


static inline char *RTK_FC_HOOK_PS_NEIGH_KEY_GET(struct neighbour *neigh)
{
	return fc_db.hook_func.ps->neigh_key_get(neigh);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_NEIGH_KEY_NUD_COMPARE(struct neighbour *neigh, rtk_fc_helper_neigh_key_type_t type, void *cookie, unsigned char state)
{
	return fc_db.hook_func.ps->neigh_key_nud_compare(neigh, type, cookie, state);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_NEIGH_NUD_UPDATE(struct neighbour *neigh, unsigned char state, unsigned int flag)
{
	return fc_db.hook_func.ps->neigh_nud_state_update(neigh, state, flag);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_DEV_ALLOC_SKB(unsigned int length, struct sk_buff **skb)
{
	return fc_db.hook_func.ps->skb_dev_alloc_skb(length, skb);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_DEV_ALLOC_CRITICAL_SKB(unsigned int length, struct sk_buff **skb)
{
	return fc_db.hook_func.ps->skb_dev_allocCritical_skb(length, skb);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_SWAP_RECYCLE_SKB(struct sk_buff *skb, struct sk_buff **newskb)
{
	return fc_db.hook_func.ps->skb_swap_recycle_skb(skb, newskb);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_DEV_KFREE_SKB_ANY(struct sk_buff *skb)
{
	return fc_db.hook_func.ps->skb_dev_kfree_skb_any(skb);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_ETH_TYPE_TRANS(struct sk_buff *skb, struct net_device *dev)
{
	return fc_db.hook_func.ps->skb_eth_type_trans(skb, dev);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_NETIF_RX(struct sk_buff *skb)
{
	return fc_db.hook_func.ps->skb_netif_rx(skb);
}
static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_NETIF_RX_WITH_HOOK(struct sk_buff *skb)
{
	//rtk_fc_skb_netif_rx_with_hook
	return fc_db.hook_func.ps->skb_netif_rx_with_hook(skb);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_NETIF_RECEIVE_SKB(struct sk_buff *skb)
{
	return fc_db.hook_func.ps->skb_netif_receive_skb(skb);
}


static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_SKB_PUSH(struct sk_buff *skb, unsigned int len, unsigned char **pData)
{
	//rtk_fc_skb_skb_push
	fc_db.hook_func.ps->skb_skb_push(skb, len, pData);
	return RTK_FC_RET_OK;
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_SKB_PULL(struct sk_buff *skb, unsigned int len, unsigned char **pData)
{
	//rtk_fc_skb_skb_pull
	fc_db.hook_func.ps->skb_skb_pull(skb, len, pData);
	return RTK_FC_RET_OK;
}


static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_SKB_PUT(struct sk_buff *skb, unsigned int len, unsigned char **pData)
{
	fc_db.hook_func.ps->skb_skb_put(skb, len, pData);
	
	return RTK_FC_RET_OK;
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_SKB_CLONED(struct sk_buff *skb)
{
	return fc_db.hook_func.ps->skb_skb_cloned(skb);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_SKB_CLONE(struct sk_buff *skb, gfp_t gfp_mask, struct sk_buff **newskb)
{
	*newskb = NULL;

	fc_db.hook_func.ps->skb_skb_clone(skb, gfp_mask, newskb);

	return RTK_FC_RET_OK;
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_SKB_COPY(struct sk_buff *skb, gfp_t gfp_mask, struct sk_buff **newskb)
{
	*newskb = NULL;

	fc_db.hook_func.ps->skb_skb_copy(skb, gfp_mask, newskb);

	return RTK_FC_RET_OK;
}


static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_SKB_COW_HEAD(struct sk_buff *skb, unsigned int headroom)
{
	//rtk_fc_skb_skb_cow_head
	return fc_db.hook_func.ps->skb_skb_cow_head(skb, headroom);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_SKB_TRIM(struct sk_buff *skb, unsigned int len)
{
	//rtk_fc_skb_skb_trim
	return fc_db.hook_func.ps->skb_skb_trim(skb, len);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_SKB_RESET_MAC_HEADER(struct sk_buff *skb)
{
	//rtk_fc_skb_skb_reset_mac_header
	return fc_db.hook_func.ps->skb_skb_reset_mac_header(skb);
}
static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_SKB_SET_NETWORK_HEADER(struct sk_buff *skb, const int offset)
{
	//rtk_fc_skb_skb_set_network_header
	return fc_db.hook_func.ps->skb_skb_set_network_header(skb, offset);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_SKB_SET_TRANSPORT_HEADER(struct sk_buff *skb, const int offset)
{
	//rtk_fc_skb_skb_set_transport_header
	return fc_db.hook_func.ps->skb_skb_set_transport_header(skb, offset);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_IP_SUMMED_SET(struct sk_buff *skb, uint8 value)
{
	return fc_db.hook_func.ps->skb_ip_summed_set(skb, value);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_IP_SUMMED_GET(struct sk_buff *skb, uint8 *value)
{
	return fc_db.hook_func.ps->skb_ip_summed_get(skb, value);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_PARAM_SET(struct sk_buff *skb, rtk_fc_skb_param_t param ,uint32 value)
{
	return fc_db.hook_func.ps->skb_param_set(skb, param, value);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_SKB_PARAM_GET(struct sk_buff *skb, rtk_fc_skb_param_t param ,uint32 *value)
{
	return fc_db.hook_func.ps->skb_param_get(skb, param, value);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_DEV_GET_BY_INDEX(struct net *net, int ifindex, struct net_device **dev)
{
	// rtk_fc_dev_get_by_index
	return fc_db.hook_func.ps->dev_get_by_index(net, ifindex, dev);
}

static inline void *RTK_FC_HOOK_PS_DEV_GET_PRIV(struct net_device *dev)
{
	return fc_db.hook_func.ps->dev_get_priv(dev);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_DEV_GET_PRIV_FLAGS(struct net_device *dev, unsigned int *privflags)
{
	// rtk_fc_dev_get_priv_flags
	return fc_db.hook_func.ps->dev_get_priv_flags(dev, privflags);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_DEV_IS_PRIV_FLAGS_SET(struct net_device *dev, rtk_fc_netdev_priv_flag_type_t flag)
{
	// rtk_fc_dev_is_priv_flags_set
	return fc_db.hook_func.ps->dev_is_priv_flags_set(dev, flag);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_DEV_IS_WLAN_DEV(struct net_device *dev, bool *iswlandev)
{
	// rtk_fc_dev_is_wlan_dev
	return fc_db.hook_func.wlan->wlan_dev_is_wifidev(dev, iswlandev);
}

// consider wlan dev!reference rdev->rdev only
static inline rtk_fc_ret_t RTK_FC_HOOK_PS_DEV_GET_REALDEV(struct net_device *dev, rtk_fc_realdev_t *rdev)
{
	bool isWlanDev = FALSE;

	RTK_FC_HOOK_PS_DEV_IS_WLAN_DEV(dev,&isWlanDev);
	if(isWlanDev) {
		rdev->rdev = dev;
		rdev->physicalPid = RTK_FC_MAC_PORT_MAX;		// meaningless

		return RTK_FC_RET_OK;
	}
	// rtk_fc_dev_get_realdev_phyport
	return fc_db.hook_func.ps->dev_get_realdev_phyport(dev, rdev);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_DEV_GET_REALDEV_PHYPORT(struct net_device *dev, rtk_fc_realdev_t *rdev)
{
	// rtk_fc_dev_get_realdev_phyport
	return fc_db.hook_func.ps->dev_get_realdev_phyport(dev, rdev);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_DEV_GET_DEV_TYPE(struct net_device *dev, unsigned int *devtype)
{
	// rtk_fc_dev_get_dev_type
	return fc_db.hook_func.ps->dev_get_dev_type(dev, devtype);
	
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_DEV_NETIF_RUNNING(struct net_device *dev)
{
	// rtk_fc_dev_netif_running
	return fc_db.hook_func.ps->dev_netif_running(dev);
}

static inline void RTK_FC_HOOK_PS_DEV_DEV_HOLD(struct net_device *dev)
{
	//rtk_fc_dev_dev_hold
	fc_db.hook_func.ps->dev_dev_hold(dev);
	return ;
}

static inline void RTK_FC_HOOK_PS_DEV_DEV_PUT(struct net_device *dev)
{
	//rtk_fc_dev_dev_put
	fc_db.hook_func.ps->dev_dev_put(dev);
	return ;
}

static inline struct net_device*  RTK_FC_HOOK_PS_VLAN_DEV_GET_REAL_REV(struct net_device *dev)
{
	return fc_db.hook_func.ps->vlan_dev_get_real_dev(dev);
}
static inline rtk_fc_ret_t RTK_FC_HOOK_PS_COPY_FROM_USER(char *procBuffer, const char __user * userbuf, size_t count)
{
	// rtk_fc_dev_get_dev_type
	return fc_db.hook_func.ps->fc_copy_from_user(procBuffer, userbuf, count);
	
}

static inline void * RTK_FC_HELPER_FC_KMALLOC(size_t size, gfp_t gfp_flag)
{
	void *fc_malloc=NULL;

	fc_db.memUsage += size;
       
      fc_malloc = fc_db.hook_func.ps->fc_helper_fc_kmalloc(size, gfp_flag);
	if(fc_db.controlFuc.mem_leak_debug && fc_malloc)
	{
		rtk_fc_mem_alloc_debug *fc_maclloc_debug = (rtk_fc_mem_alloc_debug *)fc_db.hook_func.ps->fc_helper_fc_kmalloc(sizeof(rtk_fc_mem_alloc_debug), gfp_flag);
		fc_db.hook_func.ps->fc_helper_spin_lock_bh_irq_protect(&(fc_db.lock_memDebug));

		list_add(&(fc_maclloc_debug->mem_alloc_list), &(fc_db.memAlloc));

		fc_db.hook_func.ps->fc_helper_spin_unlock_bh_irq_protect(&(fc_db.lock_memDebug));
		
		fc_maclloc_debug->alloc_func_addr = (void*)_THIS_IP_;
		fc_maclloc_debug->alloc_ret_addr = (void*)_RET_IP_;
		fc_maclloc_debug->alloc_size = size;
		fc_maclloc_debug->malloc_addr = fc_malloc;
	}
	  return fc_malloc;
}

static inline int RTK_FC_HELPER_FC_KFREE(const void *ptr,size_t size)
{
       fc_db.memUsage -= size;
	   if(fc_db.controlFuc.mem_leak_debug && ptr)
	  {
	 	 rtk_fc_mem_alloc_debug *memalloc, *tmpmemalloc;
		 unsigned char deleted=0;

			fc_db.hook_func.ps->fc_helper_spin_lock_bh_irq_protect(&(fc_db.lock_memDebug));
		
			list_for_each_entry_safe(memalloc,tmpmemalloc, &(fc_db.memAlloc), mem_alloc_list) {
				if(memalloc->malloc_addr == ptr)
				{
					list_del(&(memalloc->mem_alloc_list));
					fc_db.hook_func.ps->fc_helper_fc_kfree(memalloc, sizeof(rtk_fc_mem_alloc_debug));
					deleted=1;
					break;
				}
			}
			
			fc_db.hook_func.ps->fc_helper_spin_unlock_bh_irq_protect(&(fc_db.lock_memDebug));

			if(deleted == 0)
			{
				printk("[%s %d] no fc mem debug entry found when mem free, maybe double free!!\n",__func__,__LINE__);
				dump_stack();
			}
	  }
       return fc_db.hook_func.ps->fc_helper_fc_kfree(ptr, size);
}



static inline int RTK_FC_HELPER_TIMER_PENDING(rtk_fc_timer_list_t *pTimer)
{
	//rtk_fc_timer_timer_pending
	return fc_db.hook_func.ps->fc_helper_timer_timer_pending(pTimer);
}
static inline int RTK_FC_HELPER_DEL_TIMER(rtk_fc_timer_list_t *pTimer)
{
	//rtk_fc_timer_del_timer
	return fc_db.hook_func.ps->fc_helper_timer_del_timer(pTimer);
}
static inline int RTK_FC_HELPER_INIT_TIMER(rtk_fc_timer_list_t *pTimer, void * function)
{
	//timer_init_timer
	return fc_db.hook_func.ps->fc_helper_timer_init_timer(pTimer, function);
}
static inline int RTK_FC_HELPER_MOD_TIMER(rtk_fc_timer_list_t *pTimer,unsigned long	expires)
{
	//rtk_fc_timer_mod_timer
	return fc_db.hook_func.ps->fc_helper_timer_mod_timer(pTimer,expires);
}
static inline int RTK_FC_HELPER_SETUP_TIMER(rtk_fc_timer_list_t *pTimer,void * function,unsigned long	data)
{
	//rtk_fc_timer_setting
	return fc_db.hook_func.ps->fc_helper_timer_setup_timer(pTimer,function,data);
}

static inline unsigned long RTK_FC_HELPER_TIMER_DATA_GET(unsigned long callbackFunc_data)
{
	return fc_db.hook_func.ps->fc_helper_timer_data_get(callbackFunc_data);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PID_TASK(struct task_struct **pTask, struct pid *pid, enum pid_type type)
{
	return fc_db.hook_func.ps->pid_task(pTask, pid, type);
}

static inline char* RTK_FC_HOOK_TASK_COMM_GET(struct task_struct *pTask)
{
	return fc_db.hook_func.ps->task_comm_get(pTask);
}

static inline struct signal_struct *RTK_FC_HOOK_TASK_SIGNAL_GET(struct task_struct *pTask)
{
	return fc_db.hook_func.ps->task_signal_get(pTask);
}

static inline struct tty_struct *RTK_FC_HOOK_TASK_SIGNAL_TTY_GET(struct task_struct *pTask)
{
	return fc_db.hook_func.ps->task_signal_tty_get(pTask);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_FIND_VPID(struct pid **pPid, int nr)
{
	return fc_db.hook_func.ps->find_vpid(pPid, nr);
}

static inline rtk_fc_ret_t RTK_FC_HOOK_GET_VID_BY_DEV(unsigned short *vid, struct net_device *dev)
{
	return fc_db.hook_func.rtk_multi_wan->rtk_get_vid_by_dev(vid, dev);
}

static inline rtk_fc_ret_t RTK_FC_HELPER_DECIDE_RX_DEV_BY_SPA(unsigned int phy_src_port, struct net_device **dev)
{
	//rtk_fc_decide_rx_device_by_spa
	return fc_db.hook_func.rtk_multi_wan->rtk_fc_decide_rx_device_by_spa(phy_src_port, dev);
}

#if defined(CONFIG_RTK_SOC_RTL8198D)
static inline rtk_fc_ret_t RTK_FC_HOOK_GET_BIND_PORTMASK(int vid,unsigned long long *bind_port,int (*ndevice2portmask)(struct net_device*, uint32 *,rtk_fc_wlan_devmask_t *))
#else
static inline rtk_fc_ret_t RTK_FC_HOOK_GET_BIND_PORTMASK(int vid,uint32 *bind_port,int (*ndevice2portmask)(struct net_device*, uint32 *,rtk_fc_wlan_devmask_t *))
#endif
{
	return fc_db.hook_func.rtk_multi_wan->rtk_get_bind_portmask(vid,bind_port,ndevice2portmask);
}


static inline rtk_fc_ret_t RTK_FC_HOOK_VLAN_PASSTHROUGH_TX_HANDLE(struct sk_buff **pskb, struct rt_skbuff *rtskb)
{
	int ret = RTK_FC_RET_OK;
	struct sk_buff *skb = RTSKB_SKB(rtskb);
	ret = fc_db.hook_func.rtk_vlan->rtk_vlan_passthrough_tx_handle(&skb);
	if(ret == RTK_SKB_COPIED){
		*pskb = skb;
		RTK_FC_HOOK_CONVERTER_SKB(skb, rtskb);
		ret = RTK_FC_RET_OK;
	}
	return ret;
}

static inline void RTK_FC_HELPER_WLAN_SET_USBNAME(struct net_device *dev)
{
	if(fc_db.hook_func.wlan && fc_db.hook_func.wlan->wlan_set_usbname){
		fc_db.hook_func.wlan->wlan_set_usbname(dev);
	}
}

static inline void RTK_FC_HELPER_SET_WFO_PORTID(struct net_device *dev)
{
	if(fc_db.hook_func.wlan && fc_db.hook_func.wlan->set_wfo_portid){
		fc_db.hook_func.wlan->set_wfo_portid(dev);
	}
}

static inline rtk_fc_ret_t RTK_FC_HELPER_WLAN_RTMBSSID_TO_DEVIDX(rt_wlan_index_t wlanIdx, rt_wlan_mbssid_index_t mbssidIdx, rtk_fc_wlan_devidx_t *wlanDevIdx)
{
	return fc_db.hook_func.wlan->wlan_rtmbssid2devidx(wlanIdx, mbssidIdx, wlanDevIdx);
}

static inline rtk_fc_ret_t RTK_FC_HELPER_WLAN_RTMBSSIDMSK_TO_DEVMSK(rt_wlan_mbssid_mask_t *rtWlanMbssidMsk,rtk_fc_wlan_devmask_t *wlanDevIdMask)
{
	return fc_db.hook_func.wlan->wlan_rtmbssidMask2devMask(rtWlanMbssidMsk, wlanDevIdMask);
}

static inline rtk_fc_ret_t RTK_FC_HELPER_WLAN_DEVMSK_TO_RTMBSSIDMSK(rtk_fc_wlan_devmask_t *wlanDevIdMask,rt_wlan_mbssid_mask_t *rtWlanMbssidMsk)
{
	return fc_db.hook_func.wlan->wlan_devmask_to_rtmbssidMask(wlanDevIdMask, rtWlanMbssidMsk);
}

static inline rtk_fc_ret_t RTK_FC_HELPER_WLAN_REGISTER(rtk_fc_wlan_devidx_t wlanDevIdx, struct net_device *dev)
{
	return fc_db.hook_func.wlan->wlan_dev_register(wlanDevIdx, dev);
}

static inline rtk_fc_ret_t RTK_FC_HELPER_WLAN_UNREGISTER(struct net_device *dev)
{
	return fc_db.hook_func.wlan->wlan_dev_unregister(dev);
}

static inline char *RTK_FC_HELPER_WLAN_DEVMAP_DEVNAME_GET(rtk_fc_wlan_devmap_t *devmap)
{
	return fc_db.hook_func.wlan->wlan_devmap_devname_get(devmap);
}

static inline int RTK_FC_HELPER_WLAN_DEVMAP_MACEXTPORTIDX_GET(rtk_fc_wlan_devmap_t *devmap)
{
	return fc_db.hook_func.wlan->wlan_devmap_macextportidx_get(devmap);
}

static inline int RTK_FC_HELPER_WLAN_DEVMAP_MACPORTIDX_GET(rtk_fc_wlan_devmap_t *devmap)
{
	return fc_db.hook_func.wlan->wlan_devmap_macportidx_get(devmap);
}

static inline int RTK_FC_HELPER_WLAN_DEVMAP_SHAREEXTPORT_GET(rtk_fc_wlan_devmap_t *devmap)
{
	return fc_db.hook_func.wlan->wlan_devmap_shareextport_get(devmap);
}

static inline int RTK_FC_HELPER_WLAN_DEVMAP_BAND_GET(rtk_fc_wlan_devmap_t *devmap)
{
	return fc_db.hook_func.wlan->wlan_devmap_band_get(devmap);
}

static inline rtk_fc_ret_t RTK_FC_HELPER_WLAN_DEVMAP_GET(rtk_fc_wlan_devidx_t wlanDevIdx, rtk_fc_wlan_devmap_t **devmap)
{
	return fc_db.hook_func.wlan->wlan_devmap_get(wlanDevIdx, devmap);
}

static inline rtk_fc_ret_t RTK_FC_HELPER_WLAN_DEVMAP_INITMAP_DUMP(void)
{
	return fc_db.hook_func.wlan->wlan_devmap_initmap_dump();
}

static inline rtk_fc_ret_t RTK_FC_HELPER_WLAN_DEVMAP_INITMAP_SET(rtk_fc_wlan_initmap_t wlanConfig)
{
	return fc_db.hook_func.wlan->wlan_devmap_initmap_set(wlanConfig);
}

static inline int RTK_FC_HELPER_WLAN_DEVID_TO_BANDIDX(rtk_fc_wlan_devidx_t wlanDevIdx)
{
	return fc_db.hook_func.wlan->wlan_devidx_to_bandidx(wlanDevIdx);
}

static inline rtk_fc_ret_t RTK_FC_HELPER_WLAN_DEVID_TO_DEV(rtk_fc_wlan_devidx_t wlan_dev_idx, struct net_device **pWifiDev)
{
	return fc_db.hook_func.wlan->wlan_devidx_to_dev(wlan_dev_idx, pWifiDev);
}

static inline rtk_fc_ret_t RTK_FC_HELPER_WLAN_CHECK_DEV_IS_RUNNING(rtk_fc_wlan_devidx_t wlan_dev_idx)
{
	return fc_db.hook_func.wlan->wlan_device_is_running(wlan_dev_idx);
}


static inline rtk_fc_ret_t RTK_FC_HELPER_WLAN_DEV_TO_DEVID(struct net_device *dev, rtk_fc_wlan_devidx_t *wlan_dev_idx)
{
	// return FAIL if not found
	return fc_db.hook_func.wlan->wlan_dev_to_devIdx(dev, wlan_dev_idx);
}

static inline rtk_fc_ret_t RTK_FC_HELPER_WLAN_DEVNAME_TO_DEVID(char *devName, rtk_fc_wlan_devidx_t *wlanDevIdx, rtk_fc_wlan_devmask_t *wlanDevIdMask)
{
	// return FAIL if not found
	return fc_db.hook_func.wlan->wlan_devName_to_devIdx(devName, wlanDevIdx, wlanDevIdMask);
}

static inline rtk_fc_ret_t RTK_FC_HELPER_WLAN_DEVID_TO_PORTID(rtk_fc_wlan_devidx_t wlanDevIdx, rtk_fc_mac_port_idx_t *macPort, rtk_fc_mac_ext_port_idx_t *macExtPort)
{
	//rtk_fc_wlanDevIdx2port
	return fc_db.hook_func.wlan->wlan_devidx_to_portidx(wlanDevIdx, macPort, macExtPort);
}

static inline bool RTK_FC_HELPER_WLAN_CLIENT_MODE_CB_IS_REGISTERED(void)
{
	return fc_db.hook_func.wlan->wlan_client_mode_cb_is_registered();
}

static inline bool RTK_FC_HELPER_WLAN_IS_CLIENT_MODE(rtk_fc_wlan_devidx_t wlanDevIdx)
{
	rtk_fc_wlan_devmap_t *devmap = NULL;
	RTK_FC_HELPER_WLAN_DEVMAP_GET(wlanDevIdx, &devmap);
	if(devmap) {
		if(devmap->attr.client_mode)
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}

static inline bool RTK_FC_HELPER_WLAN_IS_BLOCK_RELAY(rtk_fc_wlan_devidx_t wlanDevIdx)
{
	rtk_fc_wlan_devmap_t *devmap = NULL;
	RTK_FC_HELPER_WLAN_DEVMAP_GET(wlanDevIdx, &devmap);
	if(devmap) {
		if(devmap->attr.block_relay)
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}

#if defined(CONFIG_RTK_SOC_RTL8198D)
static inline int RTK_FC_HELPER_IS_WLAN_5G(char *devname)
{
	return fc_db.hook_func.wlan->is_wlan_5g(devname);
}
#endif

static inline rtk_fc_ret_t RTK_FC_HELPER_WLAN_DEVMASK_TO_EXTPORMASK(rtk_fc_wlan_devmask_t wlanDevIdMask, rtk_fc_ext_port_mask_t *extPortMask)
{
	return fc_db.hook_func.wlan->wlan_devmask_to_extportmask(wlanDevIdMask, extPortMask);
}

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
static inline rtk_fc_ret_t RTK_FC_HELPER_WLAN_PORT_TO_DEVID(rtk_fc_mac_port_idx_t macPort, rtk_fc_mac_ext_port_idx_t macExtPort, rtk_fc_wlan_devidx_t *wlanDevIdx, rtk_fc_wlan_devmask_t *wlanDevIdMask)
{
	return fc_db.hook_func.wlan->wlan_port_to_devIdx(macPort, macExtPort, wlanDevIdx, wlanDevIdMask);
}
#endif

static inline rtk_fc_ret_t RTK_FC_HELPER_WLAN_CPUPORT_TO_DEVID(rtk_fc_mac_port_idx_t macPort, rtk_fc_wlan_devidx_t *wlanDevIdx, rtk_fc_wlan_devmask_t *wlanDevIdMask)
{
	return fc_db.hook_func.wlan->wlan_cpuport_to_devIdx(macPort, wlanDevIdx, wlanDevIdMask);
}

static inline rtk_fc_ret_t RTK_FC_HELPER_WLAN_TX_QOS_MAPPING_SET(uint8 enabled, uint8 mappingTbl[8])
{
	return fc_db.hook_func.wlan->wlan_tx_qos_mapping_set(enabled, mappingTbl);
}

static inline rtk_fc_ret_t RTK_FC_HELPER_WLAN_RX_LOOKUP_GMAC_DECISION(uint32 spa, uint32 *gmacid)
{
	return fc_db.hook_func.wlan->wlan_rx_lookup_gmac_decision(spa, gmacid);
}

#if defined(CONFIG_FC_RTL9607C_SERIES) && defined(CONFIG_RTK_SOC_RTL8198D) && !defined(CONFIG_FC_WIFI_RX_NONE)
static inline void RTK_FC_HELPER_WLAN_RX_GMAC_RESET(void)
{
	fc_db.hook_func.wlan->wlan_rx_gmac_reset();
}

static inline void RTK_FC_HELPER_WLAN_RX_GMAC_SET(int port, int mode)
{
	fc_db.hook_func.wlan->wlan_rx_gmac_set(port, mode);
}
#endif
// decide [dirtx] and [smpid] by manager module
static inline rtk_fc_devtx_t RTK_FC_HELPER_WLAN_HARD_START_XMIT(rtk_fc_wlan_devidx_t wlan_dev_idx, struct sk_buff *skb, fc_rx_info_t *pRxInfo)
{
	return fc_db.hook_func.wlan->wlan_dev_hard_start_xmit(wlan_dev_idx, skb, pRxInfo);
}

static inline rtk_fc_ret_t RTK_FC_HELPER_WLAN_SYSTEM_POST_INIT(void)
{
	return fc_db.hook_func.wlan->wlan_system_post_init();
}

static inline int RTK_FC_HELPER_RT_EXT_MAPPER_REGISTER(rt_ext_mapper_t* pMapper)
{
	return fc_db.hook_func.rt_help_api->fc_helper_rt_ext_mapper_register(pMapper);

}

static inline int RTK_FC_HELPER_RT_RATE_SHAREMETER_MAPPING_HW_GET(uint32 v_index,            rt_rate_meter_mapping_t* pMeterMap)
{
	return fc_db.hook_func.rt_help_api->fc_helper_rt_rate_shareMeter_mapping_hw_get(v_index, pMeterMap);
}

static inline int RTK_FC_HELPER_RT_CLS_FWDPORT_SET(rt_port_t port, rt_port_t fwdPort)
{
	return fc_db.hook_func.rt_help_api->fc_helper_rt_cls_fwdPort_set(port, fwdPort);
}

static inline int RTK_FC_HELPER_RT_SWITCH_MODE_GET(rt_switch_mode_t *mode)
{
	return fc_db.hook_func.rt_help_api->fc_helper_rt_switch_mode_get(mode);
}

static inline int RTK_FC_HELPER_SPIN_LOCK_INIT(spinlock_t *lock)
{
	return fc_db.hook_func.ps->fc_helper_spin_lock_init(lock);
}

static inline int RTK_FC_HELPER_SPIN_LOCK(spinlock_t *lock)
{
	return fc_db.hook_func.ps->fc_helper_spin_lock(lock);
}

static inline int RTK_FC_HELPER_SPIN_UNLOCK(spinlock_t *lock)
{
	return fc_db.hook_func.ps->fc_helper_spin_unlock(lock);
}

static inline int RTK_FC_HELPER_SPIN_LOCK_BH(spinlock_t *lock)
{
	return fc_db.hook_func.ps->fc_helper_spin_lock_bh(lock);
}

static inline int RTK_FC_HELPER_SPIN_UNLOCK_BH(spinlock_t *lock)
{
	return fc_db.hook_func.ps->fc_helper_spin_unlock_bh(lock);
}

static inline int RTK_FC_HELPER_SPIN_LOCK_BH_IRQ_PROTECT(spinlock_t *lock)
{
	return fc_db.hook_func.ps->fc_helper_spin_lock_bh_irq_protect(lock);
}

static inline int RTK_FC_HELPER_SPIN_UNLOCK_BH_IRQ_PROTECT(spinlock_t *lock)
{
	return fc_db.hook_func.ps->fc_helper_spin_unlock_bh_irq_protect(lock);
}

static inline int RTK_FC_HELPER_RCU_READ_LOCK(void)
{
	return  fc_db.hook_func.ps->fc_rcu_read_lock();
}

static inline int RTK_FC_HELPER_RCU_READ_UNLOCK(void)
{
	return  fc_db.hook_func.ps->fc_rcu_read_unlock();
}

static inline int RTK_FC_HELPER_SYNCHRONIZE_RCU(void)
{
	return fc_db.hook_func.ps->fc_synchronize_rcu();
}

static inline int RTK_FC_HELPER_CALL_RCU(struct rcu_head *head, rcu_callback_t func)
{
	return fc_db.hook_func.ps->fc_call_rcu(head, func);
}

static inline int RTK_FC_HELPER_RCU_GET_BRIDGE_PORT_BY_DEV(const struct net_device* dev, struct net_bridge_port **p)
{
	return  fc_db.hook_func.ps->fc_rcu_get_bridgePort_by_dev(dev, p);
}
static inline int RTK_FC_HELPER_GET_BRIDGE_DEV_BY_BRIDGE_PORT( struct net_bridge_port *p, struct net_device** dev)
{
	return  fc_db.hook_func.ps->fc_helper_get_bridgeDev_by_bridgePort(p, dev);
}
static inline int RTK_FC_HELPER_GET_BRIDGE_BY_BRIDGE_PORT( struct net_bridge_port *p, struct net_bridge** br)
{
	return	fc_db.hook_func.ps->fc_helper_get_bridge_by_bridgePort(p, br);
}
static inline int RTK_FC_HELPER_GET_BRIDGE_PORT_PVID( struct net_bridge_port *p, int *pvid)
{
	return  fc_db.hook_func.ps->fc_helper_get_bridgePort_pvid(p, pvid);
}
static inline int RTK_FC_HELPER_GET_FDB_BY_PVID( struct net_device *br_netdev, unsigned char *mac, int pvid, rtk_fc_fdb_entry_t *fc_fdb)
{
	return  fc_db.hook_func.ps->fc_helper_get_fdb_by_pvid(br_netdev,mac, pvid, fc_fdb);
}


static inline unsigned long RTK_FC_HELPER_GET_BRIDGE_PORT_FLAGS(struct net_bridge_port *p)
{
	return	fc_db.hook_func.ps->fc_helper_bridge_port_flags_get(p);
}

static inline bool RTK_FC_HELPER_GET_BR_PORT_EXIST(struct net_device *dev)
{
	return	fc_db.hook_func.ps->bridge_port_exists(dev);
}

static inline int RTK_FC_HELPER_MC_KERNEL_SNOOPING_DISABLE_GET(struct net_bridge* br)
{
	return	fc_db.hook_func.ps->fc_helper_mc_kernel_snooping_disable_get(br);
}

static inline int RTK_FC_HELPER_RCU_IN6_DEV_GET(struct net_device* dev, struct inet6_dev **in6_dev)
{
	return  fc_db.hook_func.ps->fc_rcu_in6_dev_get(dev, in6_dev);
}

static inline int RTK_FC_HELPER_RCU_INDEV_GET_IPADDR(struct net_device* dev, int *ipAddr)
{
	return fc_db.hook_func.ps->fc_rcu_indev_get_addr(dev, ipAddr);
}

static inline int RTK_FC_HELPER_NETDEV_GET_IF_INFO(struct net_device* dev, struct in_ifaddr **if_info)
{
	return fc_db.hook_func.ps->fc_helper_netdev_get_if_info(dev, if_info);

}

static inline int RTK_FC_HELPER_NETDEV_IFA_GLOBAL_ADDR6_SET(struct inet6_dev *in6_dev, uint32 *global_ipv6_addr_set)
{
	return fc_db.hook_func.ps->fc_helper_netdev_ifa_global_addr6_set(in6_dev, global_ipv6_addr_set);
}

static inline int RTK_FC_HELPER_NETDEV_SET_NETFILTER_CHECKSUM(struct net_device **dev)
{
	return fc_db.hook_func.ps->fc_helper_netdev_set_nf_cs_checking(dev);
}

static inline int RTK_FC_HELPER_NETDEV_NOTIFIERINFO_TO_DEV(void *ptr, struct net_device** dev)
{
	return fc_db.hook_func.ps->fc_helper_netdev_notifierInfo_to_dev(ptr, dev);
}


static inline int RTK_FC_HELPER_NETDEV_IFAV6_TO_DEV(void *ptr, struct net_device **dev)
{
	return fc_db.hook_func.ps->fc_helper_netdev_ifav6_to_dev(ptr, dev);
}

static inline int RTK_FC_HELPER_NETDEV_IFA_TO_DEV(void *ptr, struct net_device **dev)
{
	return fc_db.hook_func.ps->fc_helper_netdev_ifa_to_dev(ptr, dev);
}

static inline int RTK_FC_HELPER_NETDEV_SET_NFTABLE_MACADDR(struct net_device* dev, uint8 *octet)
{
	return fc_db.hook_func.ps->fc_helper_netdev_set_nfTbl_macAddr(dev, octet);
}

static inline int RTK_FC_HELPER_NETDEV_GET_DEV_DATA(struct net_device *dev, unsigned int *data, rtk_fc_netdevice_getData_type_t data_type)
{
	return fc_db.hook_func.ps->fc_helper_netdev_get_dev_data(dev, data, data_type);
}

static inline int RTK_FC_HELPER_NETDEV_CHECK_VXLAN_DEV(struct net_device *dev)
{
	return fc_db.hook_func.ps->fc_helper_netdev_check_vxlan_dev(dev);
}

static inline int RTK_FC_HELPER_NETDEV_GET_VXLAN_DEV_VNI(struct net_device *dev, int *vni)
{
	return fc_db.hook_func.ps->fc_helper_netdev_get_vxlan_dev_vni(dev, vni);
}

static inline struct net_device* RTK_FC_HELPER_FIRST_NET_DEVICE_GET(struct net *net)
{
	return fc_db.hook_func.ps->fc_helper_first_net_device_get(net);
}

static inline struct net_device* RTK_FC_HELPER_NEXT_NET_DEVICE_GET(struct net_device *dev)
{
	return fc_db.hook_func.ps->fc_helper_next_net_device_get(dev);
}

static inline int RTK_FC_HELPER_NETDEV_COMPARE_V6ADDR_BY_V6ADDRHASH(struct net_device *dev, uint32 v6AddrHash, uint8 direct)
{
	return fc_db.hook_func.ps->fc_helper_netdev_compare_v6addr_by_v6AddrHash(dev, v6AddrHash, direct);
}

static inline int RTK_FC_HELPER_NETDEV_COMPARE_V6ADDR_BY_V6ADDR(struct net_device *dev, struct	in6_addr	*saddr)
{
	return fc_db.hook_func.ps->fc_helper_netdev_compare_v6addr_by_v6Addr(dev, saddr);
}

static inline int RTK_FC_HELPER_NETDEV_IGNORE_LOOKUP(struct net_device *dev, unsigned long event)
{
	return fc_db.hook_func.ps->fc_helper_netdev_ignore_lookup(dev, event);
}

static inline int RTK_FC_HELPER_MGR_DEBUG_CONFIG_SET(int value)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_debug_config_set(value);
}

static inline int RTK_FC_HELPER_MGR_DEBUG_CONFIG_GET(int *value)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_debug_config_get(value);
}

static inline int RTK_FC_HELPER_MGR_MIRROR_CONFIG_SET(bool if_enable, uint8 mirrored_port)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_mirror_config_set(if_enable, mirrored_port);
}

static inline int RTK_FC_HELPER_MGR_MIRROR_CONFIG_GET(bool *if_enable, uint8 *mirrored_port)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_mirror_config_get(if_enable, mirrored_port);
}

#if defined(CONFIG_SMP)
static inline int RTK_FC_HELPER_MGR_DISPATCH_MODE_GET(rtk_fc_dispatch_mode_t *dispatch_mode, rtk_fc_dispatch_point_t dispatch_point)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_dispatchMode_get(dispatch_mode, dispatch_point);
}
static inline int RTK_FC_HELPER_MGR_DISPATCH_MODE_SET(rtk_fc_dispatch_mode_t dispatch_mode, uint32 mask, rtk_fc_dispatch_point_t dispatch_point, int *cpu_port_mapping_array)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_dispatchMode_set(dispatch_mode, mask, dispatch_point, cpu_port_mapping_array);
}
static inline int RTK_FC_MGR_DISPATCH_SMP_MODE_WIFI_TX_CPUID_GET(uint8 *cpuId, uint16 *wifi_tx_dispatch_mode, uint32 hashIdx, rtk_fc_dispatch_point_t dispatch_point)
{
	return fc_db.hook_func.mgr->fc_mgr_dispatch_smp_mode_wifi_tx_cpuId_get(cpuId, wifi_tx_dispatch_mode, hashIdx, dispatch_point);
}
static inline int RTK_FC_HELPER_MGR_RING_BUFFER_STATE_GET(rtk_fc_mgr_dispatch_point_t func, rtk_fc_mgr_ring_buf_state_t *state)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_ring_buffer_state_get(func, state);
}

static inline rtk_fc_ret_t RTK_FC_HELPER_MGR_INGRESS_RPS_GET(uint16 *fc_rps_bit_mask, uint16 *mode, int *cpu_port_mapping_array)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_ingress_rps_get(fc_rps_bit_mask, mode, cpu_port_mapping_array);
}
static inline rtk_fc_ret_t RTK_FC_HELPER_MGR_INGRESS_RPS_SET(uint16 mode, uint16 cpu_bit_mask, int *cpu_port_mapping_array)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_ingress_rps_set(mode,cpu_bit_mask, cpu_port_mapping_array);
}

#endif

static inline int RTK_FC_HELPER_SMP_MGRTRX_STATIC_GET(rtk_fc_mgr_smp_static_t *smp_statics, rtk_fc_mgr_smp_static_type_t type)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_smp_statistic_get(smp_statics, type);
}
static inline int RTK_FC_HELPER_SMP_MGRTRX_STATIC_SET(bool flag)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_smp_statistic_set(flag);
}

static inline int RTK_FC_HELPER_SMP_MGR_MEMINFO_DUMP(void)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_meminfo_dump();
}

__IRAM_FC_SHORTCUT
static inline int RTK_FC_HELPER_MGR_NIC_TX(void *pNicTx_privateInfo_data)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_nic_tx(pNicTx_privateInfo_data);
}

static inline int RTK_FC_HELPER_MGR_NIC_TX_WITH_PON_SID(struct sk_buff *skb, uint32 ponsid)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_nic_tx_with_pon_sid(skb, ponsid);
}

static inline int RTK_FC_HELPER_MGR_WIFI_RX(struct sk_buff *skb)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_wifi_rx(skb);
}

static inline int RTK_FC_HELPER_PROC_SEQFILE_SETTING(struct file *file, struct seq_file *p, const struct seq_operations *op)
{
	return fc_db.hook_func.ps->fc_helper_proc_file_seq_file_setting(file, p, op);
}

static inline struct proc_dir_entry* RTK_FC_HELPER_PS_PROC_MKDIR(const char *name,             struct proc_dir_entry *parent)
{
       return fc_db.hook_func.ps->fc_helper_proc_mkdir(name, parent);
}

static inline int RTK_FC_HELPER_PS_PROC_SINGLE_OPEN(struct file *file, int (*show)(struct seq_file *, void *),void *data)
{
       return fc_db.hook_func.ps->fc_helper_proc_single_open(file, show, data);
}


static inline int RTK_FC_HELPER_PS_PROC_SINGLE_OPEN_SIZE(struct file *file, int (*show)(struct seq_file *, void *),		void *data, size_t size)
{
       return fc_db.hook_func.ps->fc_helper_proc_single_open_size(file, show, data, size);
}

static inline int RTK_FC_HELPER_PROC_FILE_SET_PRIVATE_DATA(struct file *file, void *data)
{
	return fc_db.hook_func.ps->fc_helper_proc_file_set_private_data(file, data);
}

static inline int RTK_FC_HELPER_PROC_FILE_SET_PRIVATE_DATA_BUF(struct file *file, char *buf, size_t size)
{
	return fc_db.hook_func.ps->fc_helper_proc_file_set_private_data_buf(file, buf, size);
}

static inline unsigned int RTK_FC_HELPER_PROC_FILE_GET_INODE_ID(struct seq_file *s)
{
	return fc_db.hook_func.ps->fc_helper_proc_file_get_inode_id(s);
}

static inline int RTK_FC_HELPER_NETLINK_GET_NLHDR_FROM_SKBDATA(struct sk_buff *skb, struct nlmsghdr **nlh)
{
	return fc_db.hook_func.ps->fc_helper_netlink_get_nlhdr_fromSKBData(skb, nlh);
}

static inline int RTK_FC_HELPER_NETLINK_GET_NET_FROM_SOCK(struct sock *sk, struct net **net)
{
	return fc_db.hook_func.ps->fc_helper_netlink_get_net_from_sock(sk, net);
}

static inline int RTK_FC_HELPER_NETLINK_SET_SOCK_DATA(struct sock *sk, void	(*sk_data_ready)(struct sock *sk), gfp_t sk_allocation)
{
	return fc_db.hook_func.ps->fc_helper_netlink_set_sock_data(sk, sk_data_ready, sk_allocation);
}

static inline int RTK_FC_HELPER_SET_EXT_PHY_QOS_PORTPRIMAP(void)
{
	return fc_db.hook_func.rtk_ext_phy_qos->rtk_set_ext_phy_qos_portprimap();
}

static inline unsigned int RTK_FC_HELPER_INIT_ALL_MAC_PORTMASK_WITHOUT_CPU(void)
{
	return fc_db.hook_func.rtk_all_mac_portmask->rtk_init_all_mac_portmask_without_cpu();	
}

static inline rtk_fc_ret_t RTK_FC_HELPER_MSECS_TO_JIFFIES(int msecs, unsigned long int *jiffies)
{
	return fc_db.hook_func.ps->fc_helper_msecs_to_jiffies(msecs, jiffies);
}

static inline rtk_fc_ret_t RTK_FC_HELPER_JIFFIES_TO_SECS(unsigned long int jiffies, int *secs)
{
	return fc_db.hook_func.ps->fc_helper_jiffies_to_secs(jiffies, secs);
}

static inline int RTK_FC_HELPER_LOCAL_BH_DISABLE(void)
{
	return fc_db.hook_func.ps->fc_helper_local_bh_disable();
}
static inline int RTK_FC_HELPER_LOCAL_BH_ENABLE(void)
{
	return fc_db.hook_func.ps->fc_helper_local_bh_enable();
}

static inline int RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(unsigned int irq, const struct cpumask *m)
{
	return fc_db.hook_func.ps->fc_helper_irq_set_affinity_hint(irq, m);
}

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
static inline int RTK_FC_HELPER_MGR_WIFI_FLOW_CRTL_INFO_GET(rtk_fc_wifi_flow_crtl_info_t *wifi_flow_crtl_info)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_wifi_flow_crtl_info_get(wifi_flow_crtl_info);
}
static inline int RTK_FC_HELPER_MGR_WIFI_FLOW_CRTL_INFO_SET(bool if_en)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_wifi_flow_crtl_info_set(if_en);
}

static inline int RTK_FC_HELPER_MGR_WIFI_FLOW_CRTL_INFO_CLEAR(void)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_wifi_flow_crtl_info_clear();
}
#endif

static inline int RTK_FC_HELPER_MGR_EPON_LLID_FORMAT_SET(int formatConfig)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_epon_llid_format_set(formatConfig);
}

static inline int RTK_FC_HELPER_MGR_HWNAT_MODE_SET(rt_flow_hwnat_mode_t hwnat_mode)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_hwnat_mode_set(hwnat_mode);
}

static inline int RTK_FC_HELPER_MGR_WAN_PORT_MASK_SET(rtk_fc_port_mask_t portmask)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_wan_port_mask_set(portmask);
}

static inline int RTK_FC_HELPER_MGR_BR0_IP_MAC_SET(uint32 ip_addr, uint8 *mac_addr, uint8 flag)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_br0_ip_mac_set(ip_addr, mac_addr, flag);
}

static inline int RTK_FC_HELPER_MGR_SKBMARK_CONF_SET(rtk_fc_mgr_skbmarkTarget_t target, rtk_fc_skbmark_t skbmark_conf)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_skbmark_conf_set(target, skbmark_conf);
}

static inline int RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK_IDX_GET(rtk_fc_func_lock_type_t type, uint32 idx)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_func_spin_lock_idx_get(type, idx);
}

static inline int RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(rtk_fc_func_lock_type_t type, uint32 idx)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_func_spin_lock(type, idx);
}

static inline int RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(rtk_fc_func_lock_type_t type, uint32 idx)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_func_spin_unlock(type, idx);
}

static inline int RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK(void)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_global_spin_lock();
}

static inline int RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK(void)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_global_spin_unlock();
}

static inline int RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH(void)
{	
	//rtk_fc_mgr_global_spin_lock_bh
	return fc_db.hook_func.mgr->fc_helper_mgr_global_spin_lock_bh();
}

static inline int RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH(void)
{
	//rtk_fc_mgr_global_spin_unlock_bh
	return fc_db.hook_func.mgr->fc_helper_mgr_global_spin_unlock_bh();
}

static inline int RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT(void)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_global_spin_lock_bh_irq_protect();
}

static inline int RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT(void)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_global_spin_unlock_bh_irq_protect();
}

static inline int RTK_FC_HELPER_MGR_RTNETLINK_TIMER_SPIN_LOCK_BH(void)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_rtnetlink_timer_spin_lock_bh();
}

static inline int RTK_FC_HELPER_MGR_RTNETLINK_TIMER_SPIN_UNLOCK_BH(void)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_rtnetlink_timer_spin_unlock_bh();
}

static inline int RTK_FC_HELPER_MGR_TRACEFILTER_SPIN_LOCK_BH(void)
{
	//rtk_fc_mgr_tracefilter_spin_lock_bh
	fc_db.hook_func.mgr->fc_helper_mgr_tracefilter_spin_lock_bh();
	fc_db.tracefilterCpuOwner[smp_processor_id()] = TRUE;
	return 0;
}

static inline int RTK_FC_HELPER_MGR_TRACEFILTER_SPIN_UNLOCK_BH(void)
{
	//rtk_fc_mgr_tracefilter_spin_unlock_bh
	fc_db.tracefilterCpuOwner[smp_processor_id()] = FALSE;
	fc_db.hook_func.mgr->fc_helper_mgr_tracefilter_spin_unlock_bh();
	return 0;
}


static inline int RTK_FC_HELPER_MGR_SW_FLOW_MIB_ADD(int flowTblIdx, rt_flow_op_sw_flow_mib_info_t counts)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_sw_flow_mib_add(flowTblIdx, counts);
}

static inline int RTK_FC_HELPER_MGR_SW_FLOW_MIB_GET(int flowTblIdx, rt_flow_op_sw_flow_mib_info_t *counts)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_sw_flow_mib_get(flowTblIdx, counts);
}

static inline int RTK_FC_HELPER_MGR_SW_FLOW_MIB_CLEAR(int flowTblIdx)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_sw_flow_mib_clear(flowTblIdx);
}

static inline int RTK_FC_HELPER_MGR_SW_FLOW_MIB_CLEARALL(void)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_sw_flow_mib_clearAll();
}

static inline void *RTK_FC_HELPER_MGR_PROC_FOPS_KMALLOC(void)
{
	fc_db.memUsage += fc_db.hook_func.mgr->fc_mgr_proc_fops_size();
	return fc_db.hook_func.mgr->fc_mgr_proc_fops_kmalloc();
}

static inline void RTK_FC_HELPER_MGR_PROC_FOPS_KFREE(void *pProc_fops)
{
	fc_db.memUsage -= fc_db.hook_func.mgr->fc_mgr_proc_fops_size();
	return fc_db.hook_func.mgr->fc_mgr_proc_fops_kfree(pProc_fops);
}

static inline void RTK_FC_HELPER_MGR_PROC_FOPS_OPEN_SET(void *pProc_fops, void *pFunc)
{
	return fc_db.hook_func.mgr->fc_mgr_proc_fops_open_set(pProc_fops, pFunc);
}

static inline void RTK_FC_HELPER_MGR_PROC_FOPS_WRITE_SET(void *pProc_fops, void *pFunc)
{
	return fc_db.hook_func.mgr->fc_mgr_proc_fops_write_set(pProc_fops, pFunc);
}

static inline void RTK_FC_HELPER_MGR_PROC_FOPS_READ_SET(void *pProc_fops, void *pFunc)
{
	return fc_db.hook_func.mgr->fc_mgr_proc_fops_read_set(pProc_fops, pFunc);
}

static inline void RTK_FC_HELPER_MGR_PROC_FOPS_LSEEK_SET(void *pProc_fops, void *pFunc)
{
	return fc_db.hook_func.mgr->fc_mgr_proc_fops_lseek_set(pProc_fops, pFunc);
}

static inline void RTK_FC_HELPER_MGR_PROC_FOPS_RELEASE_SET(void *pProc_fops, void *pFunc)
{
	return fc_db.hook_func.mgr->fc_mgr_proc_fops_release_set(pProc_fops, pFunc);
}

static inline struct proc_dir_entry *RTK_FC_HELPER_MGR_PROC_CREATE_DATA(const char *name, umode_t mode,
																					struct proc_dir_entry *parent,
																					void *pProc_fops, void *data)
{
	return fc_db.hook_func.mgr->fc_mgr_proc_create_data(name, mode, parent, pProc_fops, data);
}

static inline unsigned int RTK_FC_HELPER_MGR_PROC_INODE_ID_GET(struct proc_dir_entry *pProc_dir)
{
	return fc_db.hook_func.mgr->fc_mgr_proc_inode_id_get(pProc_dir);
}

static inline void *RTK_FC_HELPER_MGR_TIMER_LIST_KMALLOC(void)
{
	fc_db.memUsage += sizeof(rtk_fc_timer_list_t);
	return fc_db.hook_func.mgr->fc_helper_mgr_timer_list_kmalloc();
}

static inline void RTK_FC_HELPER_MGR_TIMER_LIST_KFREE(rtk_fc_timer_list_t *timerList)
{
	fc_db.memUsage -= sizeof(rtk_fc_timer_list_t);
	return fc_db.hook_func.mgr->fc_helper_mgr_timer_list_kfree(timerList);
}

static inline int RTK_FC_HELPER_MGR_SKIPHWLOOKUP_STAT_SET(rtk_fc_skipHwlookUp_stat_t skipHwlookUp_stat)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_skipHwlookUp_stat_set(skipHwlookUp_stat);
}

static inline int RTK_FC_HELPER_MGR_SKIPHWLOOKUP_STAT_GET(rtk_fc_skipHwlookUp_stat_t *skipHwlookUp_stat)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_skipHwlookUp_stat_get(skipHwlookUp_stat);
}

static inline int RTK_FC_HELPER_MGR_SKIPHWLOOKUP_STAT_CLEAR(void)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_skipHwlookUp_stat_clear();
}

static inline int RTK_FC_HELPER_MGR_GLB_CONFIG_SET(rtk_fc_mgr_config_type_t glbConfig, void *config)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_glb_config_set(glbConfig, config);
}

static inline int RTK_FC_HELPER_MGR_CHECK_CONFIG_SETTING(rtk_fc_mgr_config_type_t get_config_type)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_check_config_setting(get_config_type);
}

static inline int RTK_FC_HELPER_MGR_SYSTEM_POST_INIT(void)
{
	return fc_db.hook_func.mgr->fc_helper_mgr_system_post_init();
}

#if 0// mc kernel mode is not supported.
static inline rtk_fc_ret_t RTK_FC_HOOK_PS_MC_MDB_SCAN(struct net_device *br_netdev,
				int (*nbrport2pmsk)(struct net_bridge_port *, uint32 *, rtk_fc_wlan_devmask_t *),
				int (*exec)(uint32 ,uint32 *,uint32, rtk_fc_wlan_devmask_t, int32 , uint8 , uint32, rtk_fc_wlan_devmask_t, int32, uint8,uint32,int32,uint32,uint32 *))
{
	return fc_db.hook_func.ps->mc_mdb_scan(br_netdev, nbrport2pmsk, exec);
}

/*
 * Output:
 *      pmsk
*/

static inline rtk_fc_ret_t RTK_FC_HOOK_PS_MC_MDB_LOOKUP(struct net_device *br_netdev, bool isIPv6, uint32 *mcGIP, uint32 vid,
				int (*nbrport2pmsk)(struct net_bridge_port *, uint32 *, rtk_fc_wlan_devmask_t *),
				uint32 *pmsk, rtk_fc_wlan_devmask_t *p3_wlanMsk)
{
	return fc_db.hook_func.ps->mc_mdb_lookup(br_netdev, isIPv6, mcGIP, vid, nbrport2pmsk, pmsk, p3_wlanMsk);
}
#endif


#if defined(CONFIG_FC_RTL8198F_SERIES)
static inline rtk_fc_ret_t RTK_FC_HOOK_GET_83XX_QOS_STATUS(void)
{
	return fc_db.hook_func.rtk83xxqos->rtk_get_83xx_qos_status();
}

static inline rtk_fc_ret_t RTK_FC_HOOK_83XX_QOS_SET_VLAN_TAG(uint8 *egrCVlanTagif,uint16 *egrCVID, uint16 direction, struct sk_buff *skb)
{
	fc_db.hook_func.rtk83xxqos->rtk_83xx_qos_set_vlan_tag(egrCVlanTagif, egrCVID, direction, skb);
	return RTK_FC_RET_OK;
}

static inline rtk_fc_ret_t RTK_FC_HOOK_PASS_COS_TO_83XX_QOS(uint32 cos, struct rt_skbuff *rtskb, uint32 portMask, uint32 wanPortMask, int direction)
{
	int ret = RTK_FC_RET_OK;
	struct sk_buff *skb = RTSKB_SKB(rtskb);
	ret = fc_db.hook_func.rtk83xxqos->rtk_pass_cos_to_83xx_qos(cos, &skb, portMask, wanPortMask, direction);
	if(ret == RTK_SKB_COPIED){
		RTK_FC_HOOK_CONVERTER_SKB(skb, rtskb);
		ret = RTK_FC_RET_OK;
	}
	return ret;
}

static inline rtk_fc_ret_t RTK_FC_HOOK_IP4InIP6_IPV6_FGRAMENT(struct sk_buff *skb, struct net_device *dev,
	ca_ni_tx_config_t *tx_config,
	netdev_tx_t (*xmit)(struct sk_buff *, struct net_device *, ca_ni_tx_config_t *))
{
	int ret = RTK_FC_RET_ERR;

	if(fc_db.hook_func.ps && fc_db.hook_func.ps->dslite_ipv6_fragment){
		if (!skb || !dev ||  !tx_config || !xmit)
			ret = RTK_FC_RET_OK;
		else
			return fc_db.hook_func.ps->dslite_ipv6_fragment(skb, dev, tx_config, xmit);
	}

	return ret;
}

static inline int RTK_FC_HELPER_8367R_REALY_SET_IPMCAST_ADDR(rtk_rg_asic_path3_entry_t *pP3, unsigned int l2mcgid,rtk_fc_g3IgrExtraInfo_t * pG3IgrExtraInfo, unsigned int ipv6)
{
	int ret = RT_ERR_OK;
	unsigned int wan_portMask = 0;
	rtk_fc_l2_mcast_members_t l2_mcast_members;
	

	if(fc_db.hook_func.rtk_8367r_relay_mc && fc_db.hook_func.rtk_8367r_relay_mc->rtk_l2_set_8367_ipMcastAddr){
		 
		l2_mcast_members.in_dst_ipv4_addr = pP3->in_dst_ipv4_addr;
		l2_mcast_members.in_src_ipv4_addr = pP3->in_src_ipv4_addr;
		l2_mcast_members.out_portmask = pP3->out_portmask;
		l2_mcast_members.out_cvlan_id = pP3->out_cvlan_id;
		l2_mcast_members.out_cpri = pP3->out_cpri;
		l2_mcast_members.out_user_pri_act = pP3->out_user_pri_act;
		l2_mcast_members.in_ctagif = pP3->in_ctagif ;
		l2_mcast_members.out_smac_trans = pP3->out_smac_trans;
		l2_mcast_members.out_user_priority = pP3->out_user_priority;
		l2_mcast_members.out_cvid_format_act = pP3->out_cvid_format_act;
		if(ipv6 == 1)
			memcpy(l2_mcast_members.addr32, pG3IgrExtraInfo->ipv6_Daddr.s6_addr32, 4);

		wan_portMask = fc_db.wanPortMask.portmask;
		
		return fc_db.hook_func.rtk_8367r_relay_mc->rtk_l2_set_8367_ipMcastAddr(&l2_mcast_members, l2mcgid,wan_portMask,ipv6);
	}

	return ret;
}

static inline int RTK_FC_HELPER_8367R_REALY_DEL_IP4MCAST_ADDR(u32 dip, u32 sip, u32 vid)
{
	int ret = RT_ERR_OK;

	if(fc_db.hook_func.rtk_8367r_relay_mc && fc_db.hook_func.rtk_8367r_relay_mc->rtk_l2_del_8367_ipMcastAddr){
		return fc_db.hook_func.rtk_8367r_relay_mc->rtk_l2_del_8367_ipMcastAddr( dip,sip,vid);
	}

	return ret;
}

static inline int RTK_FC_HELPER_8367R_REALY_DEL_IP6MCAST_ADDR(u32* ipv6McAddr ,u32 vid)
{
	int ret = RT_ERR_OK;

	if(fc_db.hook_func.rtk_8367r_relay_mc && fc_db.hook_func.rtk_8367r_relay_mc->rtk_l2_del_8367_ip6McastAddr){
		return fc_db.hook_func.rtk_8367r_relay_mc->rtk_l2_del_8367_ip6McastAddr(ipv6McAddr ,vid);
	}

	return ret;
}

static inline int RTK_FC_HELPER_8367R_REALY_CHECK_8367R_PORT(u32 portnum)
{
	int ret = RT_ERR_OK;
	u32 wan_portMask = 0;

	if(fc_db.hook_func.rtk_8367r_relay_mc && fc_db.hook_func.rtk_8367r_relay_mc->rtk_fc_check_8367Port){

		wan_portMask = fc_db.wanPortMask.portmask;
		return fc_db.hook_func.rtk_8367r_relay_mc->rtk_fc_check_8367Port(portnum,wan_portMask);
	}

	return ret;
}

static inline int RTK_FC_HELPER_8367R_REALY_GET_8367R_PORTID(u32 portnum)
{
	int ret = RT_ERR_OK;
	u32 wan_portMask = 0;

	if(fc_db.hook_func.rtk_8367r_relay_mc && fc_db.hook_func.rtk_8367r_relay_mc->rtk_fc_get_portNumber_in_8367){
		wan_portMask = fc_db.wanPortMask.portmask;
		return fc_db.hook_func.rtk_8367r_relay_mc->rtk_fc_get_portNumber_in_8367(portnum,wan_portMask);
	}

	return ret;
}

static inline int RTK_FC_HELPER_8367R_REALY_IGMP_PROTOCOL_INIT(void)
{
	int ret = RT_ERR_OK;

	if(fc_db.hook_func.rtk_8367r_relay_mc && fc_db.hook_func.rtk_8367r_relay_mc->rtk_l2_8367_igmp_protocol_init){
		return fc_db.hook_func.rtk_8367r_relay_mc->rtk_l2_8367_igmp_protocol_init();
	}

	return ret;
}
#endif

static inline int RTK_FC_HELPER_IGMP_MDB_SEARCH(unsigned char isIpv6,unsigned int *groupip,unsigned int *srcip,short igrCvlan,short igrSvlan,rtk_fc_table_mcConfigTbl_t *mcCfg)
{
	int ret = RT_ERR_FAILED;
	if(fc_db.hook_func.rtk_mcast_api &&fc_db.hook_func.rtk_mcast_api->rtk_fc_igmp_mdb_hdler)
	{
		//rtk_fc_igmp_mdb_callback_handler
		return fc_db.hook_func.rtk_mcast_api->rtk_fc_igmp_mdb_hdler(isIpv6,groupip,srcip,igrCvlan,igrSvlan,mcCfg);
	}
	return ret;
}

static inline int RTK_FC_HELPER_IGMP_IS_MDB_REG(void)
{
	if(fc_db.mcSetMode==RT_IGMP_MULTICAST_SYNC_AUTO && fc_db.hook_func.rtk_mcast_api &&fc_db.hook_func.rtk_mcast_api->rtk_fc_igmp_mdb_isReg)
	{
		//rtk_fc_igmp_mdb_isRegister
		return fc_db.hook_func.rtk_mcast_api->rtk_fc_igmp_mdb_isReg();
	}
	else
		return 0;
}


#if defined(CONFIG_RTK_SOC_RTL8198D)
static inline rtk_fc_ret_t RTK_FC_HOOK_LINUX_VLAN_RX_HANDLE(struct rt_skbuff *rtskb, uint32 portmask)
{
	int ret = RTK_FC_RET_OK;
	struct sk_buff *skb = RTSKB_SKB(rtskb);

	if(fc_db.hook_func.rtk_vlan && fc_db.hook_func.rtk_vlan->rtk_linux_vlan_rx_handle){
		ret = fc_db.hook_func.rtk_vlan->rtk_linux_vlan_rx_handle(&skb, portmask);
		if(ret == RTK_SKB_COPIED){
			RTK_FC_HOOK_CONVERTER_SKB(skb, rtskb);
			ret = RTK_FC_RET_OK;
		}
	}

	return ret;
}

static inline rtk_fc_ret_t RTK_FC_HOOK_LINUX_VLAN_TX_HANDLE(struct rt_skbuff *rtskb, uint32 portmask)
{
	int ret = RTK_FC_RET_OK;
	struct sk_buff *skb = RTSKB_SKB(rtskb);

	if(fc_db.hook_func.rtk_vlan && fc_db.hook_func.rtk_vlan->rtk_linux_vlan_tx_handle){
		ret = fc_db.hook_func.rtk_vlan->rtk_linux_vlan_tx_handle(&skb, portmask);
		if(ret == RTK_SKB_COPIED){
			RTK_FC_HOOK_CONVERTER_SKB(skb, rtskb);
			ret = RTK_FC_RET_OK;
		}
	}

	return ret;
}


static inline int RTK_FC_HELPER_CHECK_USER_GROUP_FLOW(unsigned int*groupip,unsigned short isIpv6,unsigned int* sourceip,unsigned int sourcePort,unsigned int destPort)
{
	int ret = RT_ERR_FAILED;

	if(fc_db.hook_func.rtk_mcast_api &&fc_db.hook_func.rtk_mcast_api->rtk_check_user_group_flow)
	{
		return fc_db.hook_func.rtk_mcast_api->rtk_check_user_group_flow(groupip,isIpv6,sourceip,sourcePort,destPort);
	}

	return ret;
}

static inline int RTK_FC_HELPER_CHECK_USER_GROUP(unsigned char * skbData)
{
	int ret = RT_ERR_FAILED;

	if(fc_db.hook_func.rtk_mcast_api &&fc_db.hook_func.rtk_mcast_api->rtk_fc_check_user_group)
	{
		return fc_db.hook_func.rtk_mcast_api->rtk_fc_check_user_group(skbData);
	}

	return ret;
}

static inline uint8 RTK_FC_HELPER_GET_EXT_FLOW_MIB_CONTROL_MODE(void)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_get_ext_flow_mib_control_mode)
		return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_get_ext_flow_mib_control_mode();

	return 0;
}

static inline uint8 RTK_FC_HELPER_SET_EXT_FLOW_MIB_CONTROL_MODE(uint8 value)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_set_ext_flow_mib_control_mode)
		return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_set_ext_flow_mib_control_mode(value);

	return 0;
}

static inline uint8 RTK_FC_HELPER_SET_EXT_FLOW_MIB_DEBUG(uint32 value)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_set_ext_flow_mib_debug)
		return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_set_ext_flow_mib_debug(value);

	return 0;
}

static inline int RTK_FC_HELPER_SKB_EGRESS_COUNT_IN_EXT_FLOW_MIB(struct sk_buff *skb, uint8 is_wlan_sta, bool is_uplink, uint32 *pIdx)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_skb_egress_count_in_ext_flow_mib)
		return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_skb_egress_count_in_ext_flow_mib(skb, is_wlan_sta, is_uplink, pIdx);

	return FAIL;
}

static inline int RTK_FC_HELPER_ADD_EXT_FLOW_MIB_COUNT_BY_LAN_IP(rtk_fc_ext_flow_mib_host_mapping_t *pHost, rtk_fc_ext_flow_mib_net_mapping_t *pNet)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_add_ext_flow_mib_count_by_lan_ip)
		return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_add_ext_flow_mib_count_by_lan_ip(pHost, pNet);

	return FAIL;
}

static inline int RTK_FC_HELPER_ADD_ETH_EXT_FLOW_MIB_COUNT_BY_LAN_IP(rtk_fc_ext_flow_mib_host_mapping_t *pHost, rtk_fc_ext_flow_mib_net_mapping_t *pNet)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_add_eth_ext_flow_mib_count_by_lan_ip)
		return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_add_eth_ext_flow_mib_count_by_lan_ip(pHost, pNet);

	return FAIL;
}

static inline int RTK_FC_HELPER_ADD_EXT_FLOW_MIB_COUNT_BY_LAN_MAC(unsigned char *mac_addr, unsigned char is_wlan_sta)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_add_ext_flow_mib_count_by_lan_mac)
		return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_add_ext_flow_mib_count_by_lan_mac(mac_addr, is_wlan_sta);

	return FAIL;
}

static inline int RTK_FC_HELPER_DEL_EXT_FLOW_MIB_COUNT_BY_LAN_IP(rtk_fc_ext_flow_mib_host_mapping_t *pHost)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_delete_ext_flow_mib_count_by_lan_ip)
			return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_delete_ext_flow_mib_count_by_lan_ip(pHost);

	return FAIL;
}

static inline int RTK_FC_HELPER_DEL_EXT_FLOW_MIB_COUNT_BY_LAN_MAC(unsigned char *mac_addr, unsigned char is_wlan_sta)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_delete_ext_flow_mib_count_by_lan_mac)
			return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_delete_ext_flow_mib_count_by_lan_mac(mac_addr, is_wlan_sta);

	return FAIL;
}

static inline int RTK_FC_HELPER_FLUSH_EXT_FLOW_MIB_COUNT_BY_LAN_IP(rtk_fc_ext_flow_mib_host_mapping_t *pHost)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_flush_ext_flow_mib_count_by_lan_ip)
			return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_flush_ext_flow_mib_count_by_lan_ip(pHost);

	return FAIL;
}

static inline int RTK_FC_HELPER_FLUSH_EXT_FLOW_MIB_COUNT_BY_LAN_MAC(unsigned char *mac_addr, unsigned char is_wlan_sta)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_flush_ext_flow_mib_count_by_lan_mac)
			return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_flush_ext_flow_mib_count_by_lan_mac(mac_addr, is_wlan_sta);

	return FAIL;
}

static inline int RTK_FC_HELPER_UPDATE_EXT_FLOW_MIB_BY_LAN_IP(rtk_fc_ext_flow_mib_host_mapping_t *pHost, rtk_fc_ext_flow_mib_param_t *pParam)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_update_ext_flow_mib_by_lan_ip)
		return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_update_ext_flow_mib_by_lan_ip(pHost, pParam);

	return FAIL;
}

static inline int RTK_FC_HELPER_UPDATE_EXT_FLOW_MIB_BY_LAN_MAC(unsigned char *mac_addr, unsigned char is_wlan_sta, rtk_fc_ext_flow_mib_param_t *pParam)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_update_ext_flow_mib_by_lan_mac)
		return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_update_ext_flow_mib_by_lan_mac(mac_addr, is_wlan_sta, pParam);

	return FAIL;
}

static inline int RTK_FC_HELPER_GET_EXT_FLOW_MIB_COUNT_BY_LAN_IP(uint32 ip_addr, uint8 is_wlan_sta, uint32 *rx_bytes, uint32 *tx_bytes, uint32 *rx_pkts, uint32 *tx_pkts, uint8 flag)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_get_ext_flow_mib_count_by_lan_ip)
			return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_get_ext_flow_mib_count_by_lan_ip(ip_addr, is_wlan_sta, rx_bytes, tx_bytes, rx_pkts, tx_pkts, flag);

	return FAIL;
}

static inline int RTK_FC_HELPER_GET_EXT_FLOW_MIB_COUNT_BY_LAN_MAC(unsigned char *mac_addr, uint8 is_wlan_sta, uint32 *rx_bytes, uint32 *tx_bytes, uint32 *rx_pkts, uint32 *tx_pkts, uint8 flag)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_get_ext_flow_mib_count_by_lan_mac)
			return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_get_ext_flow_mib_count_by_lan_mac(mac_addr, is_wlan_sta, rx_bytes, tx_bytes, rx_pkts, tx_pkts, flag);

	return FAIL;
}

static inline int RTK_FC_HELPER_DEL_EXT_FLOW_MIB_COUNT_AND_FLOW_BY_LAN_IP(rtk_fc_ext_flow_mib_host_mapping_t *pHost)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_delete_ext_flow_mib_count_and_flow_by_lan_ip)
			return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_delete_ext_flow_mib_count_and_flow_by_lan_ip(pHost);

	return FAIL;
}

static inline int RTK_FC_HELPER_DEL_EXT_FLOW_MIB_COUNT_AND_FLOW_BY_LAN_MAC(unsigned char *mac_addr, unsigned char is_wlan_sta)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_delete_ext_flow_mib_count_and_flow_by_lan_mac)
			return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_delete_ext_flow_mib_count_and_flow_by_lan_mac(mac_addr, is_wlan_sta);

	return FAIL;
}
#endif

#if defined(CONFIG_RTL8198XB_SERIES)
static inline int RTK_FC_HELPER_ADD_EXT_FLOW_MIB_COUNT_BY_LAN_MAC(unsigned char *mac_addr)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_add_ext_flow_mib_count_by_lan_mac)
		return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_add_ext_flow_mib_count_by_lan_mac(mac_addr);

	return FAIL;
}

static inline int RTK_FC_HELPER_DEL_EXT_FLOW_MIB_COUNT_BY_LAN_MAC(unsigned char *mac_addr)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_delete_ext_flow_mib_count_by_lan_mac)
			return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_delete_ext_flow_mib_count_by_lan_mac(mac_addr);

	return FAIL;
}

static inline int RTK_FC_HELPER_FLUSH_EXT_FLOW_MIB_COUNT_BY_LAN_MAC(unsigned char *mac_addr)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_flush_ext_flow_mib_count_by_lan_mac)
			return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_flush_ext_flow_mib_count_by_lan_mac(mac_addr);

	return FAIL;
}

static inline int RTK_FC_HELPER_GET_EXT_FLOW_MIB_COUNT_BY_LAN_MAC(unsigned char *mac_addr, uint32 *rx_bytes, uint32 *tx_bytes, uint32 *rx_pkts, uint32 *tx_pkts)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_get_ext_flow_mib_count_by_lan_mac)
			return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_get_ext_flow_mib_count_by_lan_mac(mac_addr, rx_bytes, tx_bytes, rx_pkts, tx_pkts);

	return FAIL;
}

static inline int RTK_FC_HELPER_DEL_EXT_FLOW_MIB_COUNT_AND_FLOW_BY_LAN_MAC(unsigned char *mac_addr)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_delete_ext_flow_mib_count_and_flow_by_lan_mac)
			return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_delete_ext_flow_mib_count_and_flow_by_lan_mac(mac_addr);

	return FAIL;
}

#endif

#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_RTL8198XB_SERIES)
static inline uint8 RTK_FC_HELPER_GET_EXT_FLOW_MIB_CONTROL(void)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_get_ext_flow_mib_control)
		return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_get_ext_flow_mib_control();

	return 0;
}

static inline uint8 RTK_FC_HELPER_SET_EXT_FLOW_MIB_CONTROL(uint8 value)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_set_ext_flow_mib_control)
		return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_set_ext_flow_mib_control(value);

	return 0;
}

static inline int RTK_FC_HELPER_PRINT_EXT_FLOW_MIB_COUNT_ALL(void)
{
	if (fc_db.hook_func.rtk_ext_flow_mib && fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_print_ext_flow_mib_count_all)
			return fc_db.hook_func.rtk_ext_flow_mib->rtk_fc_print_ext_flow_mib_count_all();

	return 0;
}
#endif

static inline int RTK_FC_GET_IPFRAG_MAX_HASH_SIZE(void)
{
	if (fc_db.hook_func.rtk_ipfrag && fc_db.hook_func.rtk_ipfrag->rtk_fc_get_ipfrag_max_hash_size)
		return fc_db.hook_func.rtk_ipfrag->rtk_fc_get_ipfrag_max_hash_size();

	return 0;
}

static inline int RTK_FC_GET_IPFRAG_MAX_HASH_ENTRY_SIZE(void)
{
	if (fc_db.hook_func.rtk_ipfrag && fc_db.hook_func.rtk_ipfrag->rtk_fc_get_ipfrag_max_hash_entry_size)
		return fc_db.hook_func.rtk_ipfrag->rtk_fc_get_ipfrag_max_hash_entry_size();

	return 0;
}

static inline int RTK_FC_GET_IPFRAG_CACHE_TIMEOUT_MSEC(void)
{
	if (fc_db.hook_func.rtk_ipfrag && fc_db.hook_func.rtk_ipfrag->rtk_fc_get_ipfrag_cache_timeout_msec)
		return fc_db.hook_func.rtk_ipfrag->rtk_fc_get_ipfrag_cache_timeout_msec();

	return 0;
}

#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_FC_RTL8198F_SERIES) || defined(CONFIG_RTL8198X_SERIES)
static inline int RTK_FC_HELPER_TCP_IN_WINDOW(struct sk_buff *skb, struct nf_conn *fc_cached_ct, uint32 is_ipv4, uint8 *l3h, int l3h_offset, struct tcphdr *tcph)
{
        if(fc_db.hook_func.rtk_fc_tcp_helper && fc_db.hook_func.rtk_fc_tcp_helper->rtk_fc_tcp_in_window)
                return fc_db.hook_func.rtk_fc_tcp_helper->rtk_fc_tcp_in_window(skb, fc_cached_ct, is_ipv4, l3h, l3h_offset, tcph);

        return 0;
}
#endif


static inline rtk_fc_ret_t RTK_FC_HOOK_MAPE_IPID_GET(struct nf_conn *ct, uint16 *ipid)
{
	int ret = RTK_FC_RET_ERR;
	if (fc_db.hook_func.ps && fc_db.hook_func.ps->mape_get_ipid){
		if (!ct || !ipid)
			return ret;
		else{
			if (fc_db.hook_func.ps->mape_get_ipid(ct, ipid) == SUCCESS)
				ret = RTK_FC_RET_OK;
		}
	}
	return ret;
}


#endif

