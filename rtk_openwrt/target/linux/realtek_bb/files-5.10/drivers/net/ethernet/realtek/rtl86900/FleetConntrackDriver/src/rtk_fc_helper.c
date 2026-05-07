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
#define COMPILE_RTK_L34_FC_MGR_MODULE 1

#include <linux/netfilter.h>
#include <linux/netdevice.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#include <net/netfilter/nf_conntrack.h>

#include <rtk_fc_helper.h>
#include <rtk_fc_mgr.h>
#include <rtk_fc_helper_wlan.h>
#include <rtk_fc_api.h>
#include <rtk_fc_helper_multicast.h>
//#include <rtk_fc_callback.h>



//#if defined(CONFIG_RTK_L34_NETFILTER_HOOK)

#if IS_ENABLED(CONFIG_BRIDGE)
#include <linux/netfilter_bridge.h>
#endif

int devStackto_skb_fcIngressData(struct sk_buff *skb,struct net_device *dev,int isPostRouting)
{
	int stackingStart,stackingend;
	int firstInvalid=FAIL;
	int fcDevIdx=DEVIFIDX_INVALID_MIN;
	fcDevIdx = rtk_fc_devGwMacIdx_get(dev);

#if 0
	{
		//just for debug
		struct net_device *stackDev0 = NULL;
		struct net_device *stackDev1 = NULL;
		struct net_device *stackDev2 = NULL;
		struct net_device *stackDev3 = NULL;
		rcu_read_lock();
		stackDev0 = dev_get_by_index_rcu(&init_net,skb->fcIngressData.logicalDevIfidx[0]);
		stackDev1 = dev_get_by_index_rcu(&init_net,skb->fcIngressData.logicalDevIfidx[1]);
		stackDev2 = dev_get_by_index_rcu(&init_net,skb->fcIngressData.logicalDevIfidx[2]);
		stackDev3 = dev_get_by_index_rcu(&init_net,skb->fcIngressData.logicalDevIfidx[3]);
		rcu_read_unlock();

		FCMGR_ERR("[%d][%d][%d][%d] isPostRouting:%d isDownStream:%d devname[%s(%d)]",skb->fcIngressData.logicalDevIfidx[0],skb->fcIngressData.logicalDevIfidx[1],skb->fcIngressData.logicalDevIfidx[2],skb->fcIngressData.logicalDevIfidx[3],isPostRouting,skb->fcIngressData.isDownStream,dev->name,dev->ifindex);	
		FCMGR_ERR("[%s][%s][%s][%s]",stackDev0?stackDev0->name:"NULL",stackDev1?stackDev1->name:"NULL",stackDev2?stackDev2->name:"NULL",stackDev3?stackDev3->name:"NULL");
		
	}
#endif
	if((fcDevIdx == DEVIFIDX_INVALID_MIN) )
	{
		FCMGR_PRK("inindex(%d) == DEVIFIDX_VALID_MAX(%d)",fcDevIdx,DEVIFIDX_INVALID_MIN);
		return FAILED;
	}


	if(skb->fcIngressData.isDownStream && !isPostRouting)
	{
		//downstream preRouting stacking [0][1][2]
		stackingStart=0;
		stackingend=DEV_STACK0_MAX;
	}
	else if(skb->fcIngressData.isDownStream && isPostRouting)
	{
		//downstream postRouting stacking [3]
		stackingStart=DEV_STACK0_MAX;
		stackingend=(DEV_STACK0_MAX+DEV_STACK1_MAX);		
	}
	else if(!skb->fcIngressData.isDownStream && !isPostRouting)
	{
		//upstream preRouting stacking [3]
		stackingStart=DEV_STACK0_MAX;
		stackingend=(DEV_STACK0_MAX+DEV_STACK1_MAX);				
	}
	else if(!skb->fcIngressData.isDownStream && isPostRouting)
	{
		//upstream postRouting stacking [0][1][2]
		stackingStart=0;
		stackingend=DEV_STACK0_MAX;
	}

	if(dev->priv_flags&IFF_EBRIDGE)
	{
		if(skb->fcIngressData.logicalDevIfidx[stackingStart]==DEVIFIDX_INVALID_MIN)
		{
			//if non-record any device record brx first 
			skb->fcIngressData.logicalDevIfidx[stackingStart] = fcDevIdx;
			return SUCCESS;
		}
		else
		{
			//if any device here ,we ignore all brx
			return FAILED;
		}
	}
	else
	{

		if(skb->fcIngressData.logicalDevIfidx[stackingStart]!=DEVIFIDX_INVALID_MIN )
		{
			struct net_device *firstStackDev = NULL;
			rcu_read_lock();
			firstStackDev = dev_get_by_index_rcu(&init_net,skb->fcIngressData.logicalDevIfidx[stackingStart]);
			rcu_read_unlock();
			//if first entry is brx ,replace brx to new stacking dev
			if(firstStackDev && firstStackDev->priv_flags&IFF_EBRIDGE)
			{
				skb->fcIngressData.logicalDevIfidx[stackingStart] = fcDevIdx;
				return SUCCESS;
			}
		}
	}

	//normal stacking
	if(skb->fcIngressData.isDownStream && isPostRouting)	//always update
	{
		firstInvalid=stackingStart;
	}else{
		for(; stackingStart< stackingend ;stackingStart++)
		{
			//if brx here replace it (we do not like stacking brx)
			if(skb->fcIngressData.logicalDevIfidx[stackingStart]==DEVIFIDX_INVALID_MIN )
			{
				if(firstInvalid==FAIL)
					firstInvalid=stackingStart;
			}
			else if((skb->fcIngressData.logicalDevIfidx[stackingStart]==fcDevIdx)||
				(!skb->fcIngressData.isDownStream && !isPostRouting))	//keep the oldest one
			{
				//already stacking
				return SUCCESS;
			}
		}
	}

	if(firstInvalid==FAIL)
	{
		FCMGR_ERR("stack full please check  DEV_STACK_MAX[%d] size isdownstream:%d isPostRouting:%d",DEV_STACK_MAX,skb->fcIngressData.isDownStream,isPostRouting);
		FCMGR_ERR("logicalDevIfidx[0]=%d, logicalDevIfidx[1]=%d, logicalDevIfidx[2]=%d, logicalDevIfidx[3]=%d",skb->fcIngressData.logicalDevIfidx[0],skb->fcIngressData.logicalDevIfidx[1],skb->fcIngressData.logicalDevIfidx[2],skb->fcIngressData.logicalDevIfidx[3]);
		return FAILED;
	}

	skb->fcIngressData.logicalDevIfidx[firstInvalid] = fcDevIdx;
	
	return SUCCESS;

}



#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
static unsigned int fc_nf_prerouting_cache(void *priv,struct sk_buff *skb,const struct nf_hook_state *state)
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
static unsigned int fc_nf_prerouting_cache(const struct nf_hook_ops *ops,struct sk_buff *skb,const struct net_device *in,const struct net_device *out,int (*okfn)(struct sk_buff *))
#endif
{

	enum ip_conntrack_info ctinfo;
	struct nf_conn *ct=nf_ct_get(skb, &ctinfo);

	if(skb->fcIngressData.doLearning) 
	{
		//only stack Input dev In pre-routing
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
		if(state && state->in)
			devStackto_skb_fcIngressData(skb,state->in,0);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
		if(in)
			devStackto_skb_fcIngressData(skb,in,0)
#endif
		if (skb->fcIngressData.ct == NULL)
		{
			if (ct)
				nf_conntrack_get(&(ct->ct_general));
			skb->fcIngressData.ct = ct;
		}

		FCMGR_PRK("skb[%p] cache prerouting ct[%p] to fcIngressData\n", skb, ct);
	}

#ifdef CONFIG_RTK_SOC_RTL8198D
	if (fc_mgr_db.extFlowMibControl.enable) {
		if (fc_mgr_db.extFlowMibControl.mode == RTK_EXT_FLOW_MIB_MAC_BASED) {
			memcpy(skb->fcIngressData.ingress_sa, (skb_mac_header(skb) + ETH_ALEN), ETH_ALEN);
			memcpy(skb->fcIngressData.ingress_da, skb_mac_header(skb), ETH_ALEN);

			FCMGR_PRK("skb[%p] data[%p] mac_hdr[%p] ingress_sa[%pM] ingress_da[%pM]\n", skb, skb->data, skb_mac_header(skb), skb->fcIngressData.ingress_sa, skb->fcIngressData.ingress_da);
		}
#ifdef CONFIG_OPENWRT_SDK
		else {
			struct iphdr *iph = ip_hdr(skb);
			skb->stat_ip = ntohl(iph->saddr);

			FCMGR_PRK("skb[%p] data[%p] iph_hdr[%p] stat_ip[%pI4]\n", skb, skb->data, iph, &skb->stat_ip);
		}
#endif
	}
#endif

       return NF_ACCEPT;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
static unsigned int fc_nf_postRouting_cache(void *priv,struct sk_buff *skb,const struct nf_hook_state *state)
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
static unsigned int fc_nf_postRouting_cache(const struct nf_hook_ops *ops,struct sk_buff *skb,const struct net_device *in,const struct net_device *out,int (*okfn)(struct sk_buff *))
#endif

{
	if(skb->fcIngressData.doLearning ) 
	{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
		if(state && state->out)
			devStackto_skb_fcIngressData(skb,state->out,1);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
		if(out)
			devStackto_skb_fcIngressData(skb,out,1)
#endif
	}

       return NF_ACCEPT;
}


#if IS_ENABLED(CONFIG_BRIDGE)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
static unsigned int fc_nf_br_preRouting_cache(void *priv,struct sk_buff *skb,const struct nf_hook_state *state)
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
static unsigned int fc_nf_br_preRouting_cache(const struct nf_hook_ops *ops,struct sk_buff *skb,const struct net_device *in, const struct net_device *out,int (*okfn)(struct sk_buff *))
#endif
{
	if(skb->fcIngressData.doLearning) 
	{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
		if(state && state->in)
			devStackto_skb_fcIngressData(skb,state->in,0);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
		if(in)
			devStackto_skb_fcIngressData(skb,in,0)
#endif
	}

#ifdef CONFIG_RTK_SOC_RTL8198D
	if (fc_mgr_db.extFlowMibControl.enable && fc_mgr_db.extFlowMibControl.mode == RTK_EXT_FLOW_MIB_MAC_BASED) {
		memcpy(skb->fcIngressData.ingress_sa, (skb_mac_header(skb) + ETH_ALEN), ETH_ALEN);
		memcpy(skb->fcIngressData.ingress_da, skb_mac_header(skb), ETH_ALEN);

		FCMGR_PRK("skb[%p] data[%p] mac_hdr[%p] ingress_sa[%pM] ingress_da[%pM]\n", skb, skb->data, skb_mac_header(skb), skb->fcIngressData.ingress_sa, skb->fcIngressData.ingress_da);
	}
#endif

       return NF_ACCEPT;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
static unsigned int fc_nf_br_forward_cache(void *priv,struct sk_buff *skb,const struct nf_hook_state *state)
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
static unsigned int fc_nf_br_forward_cache(const struct nf_hook_ops *ops,struct sk_buff *skb,const struct net_device *in, const struct net_device *out,int (*okfn)(struct sk_buff *))
#endif
{
	if(skb->fcIngressData.doLearning) {
		if(skb->cloned)
		{
			FCMGR_PRK("skb[%p] skb->cloned is TRUE. Bridge packet may flooding by PS\n", skb);
			skb->fcIngressData.skbCloned = 1;
		}
	}

	return NF_ACCEPT;
}

static unsigned int
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
fc_nf_br_postRouting_first_cache(void *priv,struct sk_buff *skb,const struct nf_hook_state *state)
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
fc_nf_br_postRouting_first_cache(const struct nf_hook_ops *ops,struct sk_buff *skb,const struct net_device *in,const struct net_device *out,int (*okfn)(struct sk_buff *))
#endif
{
	if(skb->fcIngressData.doLearning ) 
	{
		//only stack output dev In post-routing
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
		if(state && state->out)
			devStackto_skb_fcIngressData(skb,state->out,1);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
		if(out)
			devStackto_skb_fcIngressData(skb,out,1)
#endif

#if defined(CONFIG_RTK_SOC_RTL8198D)
		if (!skb->fcIngressData.skbCloned && skb->cloned) {
			FCMGR_PRK("skb[%p] skb->cloned is TRUE. Bridge local out packet may flooding by PS\n", skb);
			skb->fcIngressData.skbCloned = 1;
		}
#endif
	}
	

       return NF_ACCEPT;
}

static unsigned int
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
fc_nf_br_postRouting_cache(void *priv,struct sk_buff *skb,const struct nf_hook_state *state)
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
fc_nf_br_postRouting_cache(const struct nf_hook_ops *ops,struct sk_buff *skb,const struct net_device *in,const struct net_device *out,int (*okfn)(struct sk_buff *))
#endif
{
	if(skb->fcIngressData.doLearning ) 
	{
		//only stack output dev In post-routing
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
		if(state && state->out)
			devStackto_skb_fcIngressData(skb,state->out,1);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
		if(out)
			devStackto_skb_fcIngressData(skb,out,1)
#endif

	}
	

       return NF_ACCEPT;
}
#endif

static struct nf_hook_ops ct_cache_ops[]  __read_mostly =
{
	/* CT caching */
	// ipv4
	{
		.hook		= fc_nf_prerouting_cache,
		.pf			= NFPROTO_IPV4,
		.hooknum	= NF_INET_PRE_ROUTING,
		.priority	= NF_IP_PRI_CONNTRACK_CONFIRM - 1,
	},
	// ipv6
	{
		.hook		= fc_nf_prerouting_cache,
		.pf			= NFPROTO_IPV6,
		.hooknum	= NF_INET_PRE_ROUTING,
		.priority	= NF_IP_PRI_CONNTRACK_CONFIRM - 1,
	},
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
	/* DEV caching */
	{
		.hook           = fc_nf_postRouting_cache,
		.pf             = NFPROTO_IPV4,
		.hooknum        = NF_INET_POST_ROUTING,
		.priority       = NF_IP_PRI_FIRST,
	},
	{
		.hook           = fc_nf_postRouting_cache,
		.pf             = NFPROTO_IPV6,
		.hooknum        = NF_INET_POST_ROUTING,
		.priority       = NF_IP_PRI_LAST,
	},
#endif
#if IS_ENABLED(CONFIG_BRIDGE)
	// bridge
	{
		.hook 		= fc_nf_br_preRouting_cache,
		.pf 		= NFPROTO_BRIDGE,
		.hooknum 	= NF_BR_PRE_ROUTING,
		.priority 	= NF_BR_PRI_FIRST,
	},
	{
		.hook 		= fc_nf_br_postRouting_first_cache,
		.pf 		= NFPROTO_BRIDGE,
		.hooknum 	= NF_BR_POST_ROUTING,
		.priority 	= NF_BR_PRI_FIRST,
	},
	{
		.hook 		= fc_nf_br_forward_cache,
		.pf 		= NFPROTO_BRIDGE,
		.hooknum 	= NF_BR_FORWARD,
		.priority 	= NF_BR_PRI_BRNF - 2,		// higher than br_nf_forward_ip
	},
	{
		.hook 		= fc_nf_br_postRouting_cache,
		.pf 		= NFPROTO_BRIDGE,
		.hooknum 	= NF_BR_POST_ROUTING,
		.priority 	= NF_BR_PRI_LAST,
	},
#endif
};


static int  rtk_fc_ext_ct_cache_init(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
	return nf_register_net_hooks(&init_net, ct_cache_ops, ARRAY_SIZE(ct_cache_ops));
#else
       return nf_register_hooks(ct_cache_ops, ARRAY_SIZE(ct_cache_ops));
#endif
}

static void  rtk_fc_ext_ct_cache_exit(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
	nf_unregister_net_hooks(&init_net, ct_cache_ops, ARRAY_SIZE(ct_cache_ops));
#else
       nf_unregister_hooks(ct_cache_ops, ARRAY_SIZE(ct_cache_ops));
#endif
}

//#endif

__IRAM_FC_SHORTCUT
int rtk_fc_converter_skb(struct sk_buff *skb, struct rt_skbuff *rtskb)
{
	rtskb->skb 			= skb;
	rtskb->data			= &skb->data;
	rtskb->dev			= &skb->dev;
#if defined(CONFIG_RTK_APP_PORT_BINDING_SUPPORT)
	rtskb->from_dev		= &skb->from_dev;
#else
	rtskb->from_dev		= &skb->dev;
#endif	

	rtskb->fcIngressData 	= &skb->fcIngressData;
	rtskb->len			= &skb->len;
	rtskb->data_len		= &skb->data_len;// non_linear data length

	rtskb->mac_header	= &skb->mac_header;
	rtskb->transport_header = &skb->transport_header;
	rtskb->network_header = &skb->network_header;
	rtskb->mark			= &skb->mark;
	rtskb->hash			= &skb->hash;
#if defined(CONFIG_RTK_SKB_MARK2)
	rtskb->mark2		= &skb->mark2;
#else
	rtskb->mark2		= (__u64*)fc_mgr_db.mgr_null_pointer;
#endif
#if defined(CONFIG_RTL_ETH_RECYCLED_SKB)
	rtskb->recyclable	= skb->recyclable;
#else
	rtskb->recyclable	= 0;
#endif
#if defined(CONFIG_FC_RTL9607C_SERIES)
	rtskb->nptv6_setMeterFlag = 0;
	rtskb->vxlan_extra_setNic_flag = 0;
	rtskb->vxlan_setNic_flag = 0;
	rtskb->vxlan_down_setNic_flag = 0;
	rtskb->vxlan_extra_down_setNic_flag = 0;
#endif
	return SUCCESS;
}

int rtk_fc_converter_ct(struct nf_conn *ct, struct rt_nfconn *rtct)
{
	rtct->ct			= ct;
	rtct->proto		= &ct->proto;
	rtct->status		= ct->status;
#if defined(CONFIG_NF_CONNTRACK_MARK)
	rtct->mark		= ct->mark;
#else
	rtct->mark		= 0;
#endif
	rtct->ct_general = &ct->ct_general;
	rtct->lock		= &ct->lock;

	return SUCCESS;
}

int rtk_fc_helper_init(void)
{
	rtk_fc_converter_api_t *converterapi = &fc_mgr_db.converterapi;
	rtk_fc_ps_api_t *psapi = &fc_mgr_db.psapi;
	rtk_fc_mgr_api_t *mgrapi = &fc_mgr_db.mgrapi;
	rtk_fc_multi_wan_api_t *rtk_multi_wan = &fc_mgr_db.rtk_multi_wan;
	rtk_fc_vlan_api_t *rtk_vlan = &fc_mgr_db.rtk_vlan;
	rtk_fc_wlan_api_t *wlanapi = & fc_mgr_db.wlanapi;
	rtk_fc_rt_helper_api_t *rt_helper_api = &fc_mgr_db.rt_helper_api;
	rtk_ext_phy_qos_t *rtk_ext_phy_qos = &fc_mgr_db.rtk_ext_phy_qos;
	rtk_fc_init_all_mac_portmask_t *rkt_all_mac_portmask = &fc_mgr_db.rkt_all_mac_portmask;
	
#if defined(CONFIG_FC_RTL8198F_SERIES)
	rtk_fc_83xx_qos_api_t *rtk83xxqos = &fc_mgr_db.rtk83xxqos;
#if defined(CONFIG_RTK_8367_RELAY)
	rtk_fc_8367r_relay_mc_api_t *rtk_8367r_relay_mc = &fc_mgr_db.rtk_8367r_relay_mc;
#endif
#endif
	rtk_fc_mcast_api_t * rtk_mcast_api = &fc_mgr_db.rtk_mcast_api;
#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_RTL8198XB_SERIES)
	rtk_fc_ext_flow_mib_t	*rtk_ext_flow_mib = &fc_mgr_db.rtk_ext_flow_mib;
#endif
	rtk_fc_ipfrag_helper_t *rtk_ipfrag = &fc_mgr_db.rtk_ipfrag;
#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_FC_RTL8198F_SERIES) || defined(CONFIG_RTL8198X_SERIES)
	rtk_fc_tcp_helper_t *rtk_fc_tcp_helper = &fc_mgr_db.rtk_fc_tcp_helper;
#endif

	fc_mgr_db.mgr_null_pointer=NULL;


	converterapi->skb	= rtk_fc_converter_skb;
	converterapi->ct		= rtk_fc_converter_ct;

	rtk_fc_helper_register(FC_HELPER_TYPE_CONVERTER, converterapi);



	psapi->ct_get				= rtk_fc_ct_get;
	psapi->ct_is_dying			= rtk_fc_ct_is_dying;
	psapi->ct_status_get		= rtk_fc_ct_status_get;
	psapi->ct_expiretime_get		= rtk_fc_ct_expiretime_get;
	psapi->ct_protonum_get		= rtk_fc_ct_protonum_get;
	psapi->ct_session_alive_check	= rtk_fc_ct_session_alive_check;
	psapi->ct_helper_exist_check	= rtk_fc_ct_helper_exist_check;
	psapi->ct_helper_is_conenat_check = rtk_fc_ct_helper_is_conenat_check;
	psapi->ct_timer_refresh 		= rtk_fc_ct_timer_refresh;
	psapi->ct_flush				= rtk_fc_ct_flush;
	psapi->ct_tcp_liberal_set		= rtk_fc_ct_tcp_liberal_set;
	psapi->ct_rt_5tuple_info_get	= rtk_fc_ct_rt_5tuple_info_get;
	psapi->ct_rt_tcp_state_get		= rtk_fc_ct_rt_tcp_state_get;
	psapi->ct_rt_tcp_state_set		= rtk_fc_ct_rt_tcp_state_set;
	psapi->nf_ct_put				= rtk_fc_nf_ct_put;

	psapi->br_allowed_ingress		= rtk_fc_br_allowed_ingress;
	psapi->br_multicast_disabled_get	= rtk_fc_br_multicast_disabled_get;
	psapi->br_multicast_querier_get		= rtk_fc_br_multicast_querier_get;
	psapi->fdb_time_refresh 		= rtk_fc_fdb_time_refresh;
	psapi->fdb_time_get			= rtk_fc_fdb_time_get;

	psapi->neighv4_exist_check	= rtk_fc_neighv4_exist_check;
	psapi->neighv6_exist_check	= rtk_fc_neighv6_exist_check;

	psapi->neighv4_enumerate = rtk_fc_neighv4_enumerate;
	psapi->neighv6_enumerate = rtk_fc_neighv6_enumerate;

	psapi->neigh_key_get			= rtk_fc_neigh_key_get;

	psapi->neigh_key_nud_compare = rtk_fc_neigh_key_nud_compare;
	psapi->neigh_nud_state_update = rtk_fc_neigh_nud_state_update;

	psapi->skb_dev_alloc_skb		= rtk_fc_skb_dev_alloc_skb;
	psapi->skb_dev_allocCritical_skb	= rtk_fc_skb_dev_allocCritical_skb;
	psapi->skb_swap_recycle_skb		= rtk_fc_skb_swap_recycle_skb;
	psapi->skb_dev_kfree_skb_any	= rtk_fc_skb_dev_kfree_skb_any;
	psapi->skb_eth_type_trans		= rtk_fc_skb_eth_type_trans;
	psapi->skb_netif_rx			= rtk_fc_skb_netif_rx;
	psapi->skb_netif_rx_with_hook			= rtk_fc_skb_netif_rx_with_hook;
	psapi->skb_netif_receive_skb		= rtk_fc_skb_netif_receive_skb;
	psapi->skb_skb_push			= rtk_fc_skb_skb_push;
	psapi->skb_skb_pull			= rtk_fc_skb_skb_pull;
	psapi->skb_skb_put			= rtk_fc_skb_skb_put;
	psapi->skb_skb_cloned			= rtk_fc_skb_skb_cloned;
	psapi->skb_skb_clone		= rtk_fc_skb_skb_clone;
	psapi->skb_skb_copy			= rtk_fc_skb_skb_copy;
	psapi->skb_skb_cow_head		= rtk_fc_skb_skb_cow_head;
	psapi->skb_skb_trim			= rtk_fc_skb_skb_trim;

	psapi->skb_skb_reset_mac_header 		= rtk_fc_skb_skb_reset_mac_header;
	psapi->skb_skb_set_network_header		= rtk_fc_skb_skb_set_network_header;
	psapi->skb_skb_set_transport_header		= rtk_fc_skb_skb_set_transport_header;
	psapi->skb_ip_summed_set					= rtk_fc_skb_ip_summed_set;
	psapi->skb_ip_summed_get					= rtk_fc_skb_ip_summed_get;
	psapi->skb_param_set					= rtk_fc_skb_param_set;
	psapi->skb_param_get					= rtk_fc_skb_param_get;

	psapi->fc_skb_set_sw_hash				= rtk_fc_skb_set_pkt_type_offset_sw_hash;
	psapi->fc_skb_set_l4_hash				= rtk_fc_skb_set_pkt_type_offset_l4_hash;
	psapi->fc_skb_set_priority				= rtk_fc_skb_set_pkt_priority;

	psapi->dev_get_by_index			= rtk_fc_dev_get_by_index;
	psapi->dev_get_priv				= rtk_fc_dev_get_priv;
	psapi->dev_get_priv_flags			= rtk_fc_dev_get_priv_flags;
	psapi->dev_is_priv_flags_set		= rtk_fc_dev_is_priv_flags_set;
	psapi->dev_get_realdev_phyport	= _rtk_fc_dev_get_realdev_phyport;
	psapi->dev_get_dev_type			= rtk_fc_dev_get_dev_type;
	psapi->dev_netif_running		= rtk_fc_dev_netif_running;
	psapi->dev_dev_hold				= rtk_fc_dev_dev_hold;
	psapi->dev_dev_put				= rtk_fc_dev_dev_put;
	psapi->vlan_dev_get_real_dev	= rtk_fc_vlan_dev_get_real_dev;
	
	psapi->fc_copy_from_user			= rtk_fc_copy_from_user;
	
#if 0 // not support
	psapi->mc_mdb_scan			= rtk_fc_mc_mdb_scan;
	psapi->mc_mdb_lookup		= rtk_fc_mc_mdb_lookup;
#endif

	psapi->pid_task				= rtk_fc_pid_task;
	psapi->task_comm_get		= rtk_fc_get_task_comm;
	psapi->task_signal_get		= rtk_fc_get_task_signal;
	psapi->task_signal_tty_get		= rtk_fc_get_task_signal_tty;
	psapi->find_vpid			= rtk_fc_find_vpid;

#if defined(CONFIG_FC_RTL8198F_SERIES)
	psapi->dslite_ipv6_fragment = rtk_fc_dslite_ipv6_fragment;
#endif

	psapi->mape_get_ipid = rtk_fc_mape_get_ipid;

	psapi->hw_csum=rtk_set_hw_csum;

	psapi->fc_helper_spin_lock_init			   = rtk_fc_spin_lock_init;
	psapi->fc_helper_spin_lock                 = rtk_fc_spin_lock;
	psapi->fc_helper_spin_unlock               = rtk_fc_spin_unlock;
	psapi->fc_helper_spin_lock_bh              = rtk_fc_spin_lock_bh;
	psapi->fc_helper_spin_unlock_bh            = rtk_fc_spin_unlock_bh;
	psapi->fc_helper_spin_lock_bh_irq_protect  = rtk_fc_spin_lock_bh_irq_protect;
	psapi->fc_helper_spin_unlock_bh_irq_protect= rtk_fc_spin_unlock_bh_irq_protect;

	psapi->fc_synchronize_rcu		 			= rtk_fc_synchronize_rcu;
	psapi->fc_call_rcu		 					= rtk_fc_call_rcu;
	psapi->fc_rcu_read_lock						= rtk_fc_rcu_read_lock;
	psapi->fc_rcu_read_unlock					= rtk_fc_rcu_read_unlock;
	psapi->fc_rcu_get_bridgePort_by_dev			= rtk_fc_rcu_get_bridgePort_by_dev;
	psapi->fc_helper_get_bridgeDev_by_bridgePort= rtk_fc_helper_get_bridgeDev_by_bridgePort;
	psapi->fc_helper_get_bridge_by_bridgePort   = rtk_fc_helper_get_bridge_by_bridgePort;
	psapi->fc_helper_get_bridgePort_pvid   		= rtk_fc_helper_get_bridgePort_pvid;
	psapi->fc_helper_get_fdb_by_pvid   			= rtk_fc_helper_get_fdb_by_pvid;
	
	psapi->fc_helper_bridge_port_flags_get		= rtk_fc_helper_bridge_port_flags_get;
	psapi->bridge_port_exists					= rtk_fc_br_port_exists,
	psapi->fc_helper_mc_kernel_snooping_disable_get   = rtk_fc_helper_mc_kernel_snooping_disable_get;
	
	psapi->fc_rcu_in6_dev_get					= rtk_fc_rcu_get_in6_dev;
	psapi->fc_rcu_indev_get_addr				= rtk_fc_rcu_indev_get_addr;

	psapi->fc_helper_netdev_get_if_info 		= rtk_fc_netdev_get_if_info;
	psapi->fc_helper_netdev_ifa_global_addr6_set	= rtk_fc_netdev_ifa_global_addr6_set;
	psapi->fc_helper_netdev_set_nf_cs_checking		= rtk_fc_netdev_set_nf_cs_checking;
	psapi->fc_helper_netdev_notifierInfo_to_dev 	= rtk_fc_netdev_notifierInfo_to_Dev;
	psapi->fc_helper_netdev_ifav6_to_dev			= rtk_fc_netdev_ifav6_to_dev;
	psapi->fc_helper_netdev_ifa_to_dev				= rtk_fc_netdev_ifa_to_dev;
	psapi->fc_helper_netdev_set_nfTbl_macAddr		= rtk_fc_netdev_set_nfTbl_macAddr;
	psapi->fc_helper_netdev_get_dev_data			= rtk_fc_netdev_get_dev_data;
	psapi->fc_helper_netdev_check_vxlan_dev			= rtk_fc_netdev_check_vxlan_dev;
	psapi->fc_helper_netdev_get_vxlan_dev_vni		= rtk_fc_netdev_get_vxlan_dev_vni;
	
	psapi->fc_helper_first_net_device_get			= rtk_fc_first_net_device_get;
	psapi->fc_helper_next_net_device_get			= rtk_fc_next_net_device_get;
	psapi->fc_helper_netdev_compare_v6addr_by_v6AddrHash	= rtk_fc_netdev_compare_v6addr_by_v6AddrHash;
	psapi->fc_helper_netdev_compare_v6addr_by_v6Addr	= rtk_fc_netdev_compare_v6addr_by_v6Addr;
	psapi->fc_helper_netdev_ignore_lookup			= rtk_fc_netdev_ignore_lookup;

	psapi->fc_helper_proc_file_seq_file_setting 	= rtk_fc_proc_file_seq_file_setting;
	psapi->fc_helper_proc_file_set_private_data 	= rtk_fc_proc_file_set_private_data;
	psapi->fc_helper_proc_file_set_private_data_buf	= rtk_fc_proc_file_set_private_data_buf;
	psapi->fc_helper_proc_file_get_inode_id			= rtk_fc_proc_file_get_inode_id;
	psapi->fc_helper_proc_mkdir 					= rtk_fc_proc_mkdir;
	psapi->fc_helper_proc_single_open				= rtk_fc_proc_single_open;
	psapi->fc_helper_proc_single_open_size				= rtk_fc_proc_single_open_size;

	psapi->fc_helper_netlink_get_nlhdr_fromSKBData	= rtk_fc_netlink_get_nlhdr_fromSKBData;
	psapi->fc_helper_netlink_get_net_from_sock		= rtk_fc_netlink_get_net_from_sock;
	psapi->fc_helper_netlink_set_sock_data			= rtk_fc_netlink_set_sock_data;
	psapi->fc_helper_msecs_to_jiffies				= rtk_fc_msecs_to_jiffies;
	psapi->fc_helper_jiffies_to_secs				= rtk_fc_jiffies_to_secs;
	psapi->fc_helper_local_bh_disable				= rtk_fc_local_bh_disable;
	psapi->fc_helper_local_bh_enable				= rtk_fc_local_bh_enable;
	psapi->fc_helper_fc_kmalloc 					= rtk_fc_kmalloc;
	psapi->fc_helper_fc_kfree						= rtk_fc_kfree;

	psapi->fc_helper_timer_timer_pending			= rtk_fc_timer_timer_pending;
	psapi->fc_helper_timer_del_timer				= rtk_fc_timer_del_timer;
	psapi->fc_helper_timer_init_timer				= timer_init_timer;
	psapi->fc_helper_timer_mod_timer				= rtk_fc_timer_mod_timer;
	psapi->fc_helper_timer_setup_timer				= rtk_fc_timer_setup_timer;
	psapi->fc_helper_timer_data_get					= rtk_fc_timer_data_get;
	
	psapi->fc_helper_irq_set_affinity_hint			= rtk_fc_irq_set_affinity_hint;

	rtk_fc_helper_register(FC_HELPER_TYPE_PS, psapi);
	

	mgrapi->fc_helper_mgr_debug_config_set				= rtk_fc_mgr_debug_config_set;
	mgrapi->fc_helper_mgr_debug_config_get				= rtk_fc_mgr_debug_config_get;
	mgrapi->fc_helper_mgr_mirror_config_set				= rtk_fc_mgr_mirror_config_set;
	mgrapi->fc_helper_mgr_mirror_config_get				= rtk_fc_mgr_mirror_config_get;


#if defined(CONFIG_SMP)
	mgrapi->fc_helper_mgr_dispatchMode_set				= rtk_fc_mgr_dispatchMode_set;
	mgrapi->fc_helper_mgr_dispatchMode_get				= rtk_fc_mgr_dispatchMode_get;
	mgrapi->fc_mgr_dispatch_smp_mode_wifi_tx_cpuId_get	= rtk_fc_mgr_dispatch_smp_mode_wifi_tx_cpuId_get;
	mgrapi->fc_helper_mgr_ring_buffer_state_get			= rtk_fc_mgr_ring_buffer_state_get;
	mgrapi->fc_helper_mgr_ingress_rps_get				= rtk_fc_mgr_ingress_rps_get;
	mgrapi->fc_helper_mgr_ingress_rps_set				= rtk_fc_mgr_ingress_rps_set;
#endif
	mgrapi->fc_helper_mgr_smp_statistic_get				= rtk_fc_mgr_smp_statistic_get;
	mgrapi->fc_helper_mgr_smp_statistic_set			= rtk_fc_mgr_smp_statistic_set;
	mgrapi->fc_helper_mgr_meminfo_dump				= rtk_fc_mgr_memInfo_dump;
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	mgrapi->fc_helper_mgr_wifi_flow_crtl_info_get	= rtk_fc_mgr_wifi_flow_ctrl_info_get;
	mgrapi->fc_helper_mgr_wifi_flow_crtl_info_set	= rtk_fc_mgr_wifi_flow_ctrl_info_set;
	mgrapi->fc_helper_mgr_wifi_flow_crtl_info_clear	= rtk_fc_mgr_wifi_flow_ctrl_info_clear;
#endif
	mgrapi->fc_helper_mgr_epon_llid_format_set				= rtk_fc_mgr_epon_llid_format_set;
	mgrapi->fc_helper_mgr_hwnat_mode_set					= rtk_fc_mgr_hwnat_mode_set;
	mgrapi->fc_helper_mgr_wan_port_mask_set					= rtk_fc_mgr_wan_port_mask_set;
	mgrapi->fc_helper_mgr_br0_ip_mac_set					= rtk_fc_mgr_br0_ip_mac_set;
	mgrapi->fc_helper_mgr_skbmark_conf_set				= rtk_fc_mgr_skbmark_conf_set;


	mgrapi->fc_helper_mgr_nic_tx						= rtk_fc_nic_tx;
	mgrapi->fc_helper_mgr_nic_tx_with_pon_sid			= rtk_fc_nic_tx_with_pon_sid;
	mgrapi->fc_helper_mgr_wifi_rx						= rtk_fc_skb_wifi_rx;

	mgrapi->fc_helper_mgr_func_spin_lock_idx_get				= rtk_fc_mgr_func_spin_lock_idx_get;
	mgrapi->fc_helper_mgr_func_spin_lock					= rtk_fc_mgr_func_spin_lock;
	mgrapi->fc_helper_mgr_func_spin_unlock					= rtk_fc_mgr_func_spin_unlock;
	mgrapi->fc_helper_mgr_global_spin_lock					= rtk_fc_mgr_global_spin_lock;
	mgrapi->fc_helper_mgr_global_spin_unlock				= rtk_fc_mgr_global_spin_unlock;
	mgrapi->fc_helper_mgr_global_spin_lock_bh				= rtk_fc_mgr_global_spin_lock_bh;
	mgrapi->fc_helper_mgr_global_spin_unlock_bh				= rtk_fc_mgr_global_spin_unlock_bh;
	mgrapi->fc_helper_mgr_global_spin_lock_bh_irq_protect	= rtk_fc_mgr_global_spin_lock_bh_irq_protect;
	mgrapi->fc_helper_mgr_global_spin_unlock_bh_irq_protect	= rtk_fc_mgr_global_spin_unlock_bh_irq_protect;

	mgrapi->fc_helper_mgr_rtnetlink_timer_spin_lock_bh		= rtk_fc_mgr_rtnetlink_timer_spin_lock_bh;
	mgrapi->fc_helper_mgr_rtnetlink_timer_spin_unlock_bh	= rtk_fc_mgr_rtnetlink_timer_spin_unlock_bh;

	mgrapi->fc_helper_mgr_tracefilter_spin_lock_bh		= rtk_fc_mgr_tracefilter_spin_lock_bh;
	mgrapi->fc_helper_mgr_tracefilter_spin_unlock_bh	= rtk_fc_mgr_tracefilter_spin_unlock_bh;

	mgrapi->fc_helper_mgr_sw_flow_mib_add = rtk_fc_mgr_sw_flow_mib_add;
	mgrapi->fc_helper_mgr_sw_flow_mib_get =  rtk_fc_mgr_sw_flow_mib_get;
	mgrapi->fc_helper_mgr_sw_flow_mib_clear =  rtk_fc_mgr_sw_flow_mib_clear;
	mgrapi->fc_helper_mgr_sw_flow_mib_clearAll =  rtk_fc_mgr_sw_flow_mib_clearAll;
	mgrapi->fc_mgr_proc_fops_size =  rtk_fc_mgr_proc_fops_size;
	mgrapi->fc_mgr_proc_fops_kmalloc =  rtk_fc_mgr_proc_fops_kmalloc;
	mgrapi->fc_mgr_proc_fops_kfree =  rtk_fc_mgr_proc_fops_kfree;
	mgrapi->fc_mgr_proc_fops_open_set = rtk_fc_mgr_proc_fops_open_set;
	mgrapi->fc_mgr_proc_fops_write_set = rtk_fc_mgr_proc_fops_write_set;
	mgrapi->fc_mgr_proc_fops_read_set = rtk_fc_mgr_proc_fops_read_set;
	mgrapi->fc_mgr_proc_fops_lseek_set = rtk_fc_mgr_proc_fops_lseek_set;
	mgrapi->fc_mgr_proc_fops_release_set = rtk_fc_mgr_proc_fops_release_set;
	mgrapi->fc_mgr_proc_create_data = rtk_fc_mgr_proc_create_data;
	mgrapi->fc_mgr_proc_inode_id_get = rtk_fc_mgr_proc_inode_id_get;
	
	mgrapi->fc_helper_mgr_timer_list_kmalloc =  rtk_fc_mgr_timer_list_kmalloc;
	mgrapi->fc_helper_mgr_timer_list_kfree =  rtk_fc_mgr_timer_list_kfree;
	mgrapi->fc_helper_mgr_skipHwlookUp_stat_set = rtk_fc_mgr_skipHwlookUp_stat_set;
	mgrapi->fc_helper_mgr_skipHwlookUp_stat_get = rtk_fc_mgr_skipHwlookUp_stat_get;
	mgrapi->fc_helper_mgr_skipHwlookUp_stat_clear = rtk_fc_mgr_skipHwlookUp_stat_clear;
	mgrapi->fc_helper_mgr_glb_config_set =  rtk_fc_mgr_glb_config_set;
	mgrapi->fc_helper_mgr_check_config_setting =  rtk_fc_mgr_check_config_setting;
	mgrapi->fc_helper_mgr_system_post_init = rtk_fc_mgr_system_post_init;

	rtk_fc_helper_register(FC_HELPER_TYPE_MGR, mgrapi);

	/*RT API*/
	rt_helper_api->fc_helper_rt_ext_mapper_register                 = rtk_fc_rt_ext_mapper_register;
	rt_helper_api->fc_helper_rt_rate_shareMeter_mapping_hw_get      = rtk_fc_rt_rate_shareMeter_mapping_hw_get;
	rt_helper_api->fc_helper_rt_cls_fwdPort_set		       			= rtk_fc_rt_cls_fwdPort_set;
	rt_helper_api->fc_helper_rt_switch_mode_get		       			= rtk_fc_rt_switch_mode_get;

	rtk_fc_helper_register(FC_HELPER_TYPE_RT_API, rt_helper_api);
	

	rtk_multi_wan->rtk_get_vid_by_dev = rtk_get_vid_by_dev;
	rtk_multi_wan->rtk_fc_decide_rx_device_by_spa = rtk_fc_decide_rx_device_by_spa;
	rtk_multi_wan->rtk_get_bind_portmask = rtk_get_bind_portmask;

	rtk_fc_helper_register(FC_HELPER_TYPE_MULTI_WAN, rtk_multi_wan);

	rtk_vlan->rtk_vlan_passthrough_tx_handle = rtk_vlan_passthrough_tx_handle;
#if defined(CONFIG_RTK_SOC_RTL8198D)
	rtk_vlan->rtk_linux_vlan_rx_handle = rtk_linux_vlan_rx_handle;
	rtk_vlan->rtk_linux_vlan_tx_handle = rtk_linux_vlan_tx_handle;
#endif
	rtk_fc_helper_register(FC_HELPER_TYPE_VLAN, rtk_vlan);

	wlanapi->wlan_dev_is_wifidev			= rtk_fc_dev_is_wlan_dev;
	wlanapi->wlan_rtmbssid2devidx			= rtk_fc_wlan_rtmbssid2devidx;
	wlanapi->wlan_rtmbssidMask2devMask	= rtk_fc_wlan_rtmbssidMask2devMask;
	wlanapi->wlan_dev_register 			= _rtk_fc_wlan_register;
	wlanapi->wlan_dev_unregister 			= rtk_fc_wlan_unregister;
	wlanapi->wlan_devmap_get				= rtk_fc_wlan_devmap_get;
	wlanapi->wlan_devmap_devname_get		= rtk_fc_wlan_devmap_devname_get;
	wlanapi->wlan_devmap_macportidx_get		= rtk_fc_wlan_devmap_macportidx_get;
	wlanapi->wlan_devmap_macextportidx_get	= rtk_fc_wlan_devmap_macextportidx_get;
	wlanapi->wlan_devmap_shareextport_get	= rtk_fc_wlan_devmap_shareextport_get;
	wlanapi->wlan_devmap_band_get			= rtk_fc_wlan_devmap_band_get;
	wlanapi->wlan_devmap_initmap_dump		= rtk_fc_wlan_devmap_initmap_dump;
	wlanapi->wlan_devmap_initmap_set		= rtk_fc_wlan_devmap_initmap_set;
	wlanapi->wlan_devidx_to_bandidx 		= rtk_fc_wlanDevidx2bandidx;
	wlanapi->wlan_devidx_to_dev 			= rtk_fc_wlanDevIdx2dev;
	wlanapi->wlan_device_is_running 		= rtk_fc_check_wlan_device_is_running;
	wlanapi->wlan_dev_to_devIdx 			= _rtk_fc_dev2wlanDevIdx;
	wlanapi->wlan_devName_to_devIdx			= rtk_fc_devName2wlanDevIdx;
	wlanapi->wlan_port_to_devIdx			= rtk_fc_port2wlanDevidx;
	wlanapi->wlan_cpuport_to_devIdx 		= rtk_fc_cpuport2wlanDevidx;
	wlanapi->wlan_devidx_to_portidx			= rtk_fc_wlanDevIdx2port;
	wlanapi->wlan_client_mode_cb_is_registered	= rtk_fc_wlan_client_mode_cb_is_registered;
#if defined(CONFIG_RTK_SOC_RTL8198D)
	wlanapi->is_wlan_5g						= rtk_fc_is_wlan_5g;
#endif
	wlanapi->wlan_devmask_to_extportmask 	= rtk_fc_wlanDevMask2extMask;
	wlanapi->wlan_devmask_to_rtmbssidMask 	= _rtk_fc_wlan_devMask2RtmbssidMask;
	wlanapi->wlan_tx_qos_mapping_set 		= rtk_fc_wlan_tx_qos_mapping_set;
	wlanapi->wlan_rx_lookup_gmac_decision	= rtk_fc_wlan_rx_lookup_gmac_decision;
#if defined(CONFIG_FC_RTL9607C_SERIES) && defined(CONFIG_RTK_SOC_RTL8198D) && !defined(CONFIG_FC_WIFI_RX_NONE)
	wlanapi->wlan_rx_gmac_reset		= rtk_fc_rx_gmac_sel_reset;
	wlanapi->wlan_rx_gmac_set		= rtk_fc_rx_gmac_sel_set;
#endif
	wlanapi->wlan_dev_hard_start_xmit 		= rtk_fc_wlan_hard_start_xmit;
	wlanapi->wlan_system_post_init			= rtk_fc_wlan_system_post_init;
#if defined(CONFIG_RTL_FC_USB_INTF)
#if defined(CONFIG_RTL_FC_USB_AUTO_INTFNAME)
	wlanapi->wlan_set_usbname 			    = rtk_fc_wlan_set_usbname;
#endif
#endif
#if defined(CONFIG_RTK_WFO)
	wlanapi->set_wfo_portid			    = rtk_fc_set_wfo_portid;
#endif /* CONFIG_RTK_WFO */

	rtk_fc_helper_register(FC_HELPER_TYPE_WLAN, wlanapi);

	rtk_ext_phy_qos->rtk_set_ext_phy_qos_portprimap = rtk_set_ext_phy_qos_portprimap;

	rtk_fc_helper_register(FC_HELPER_TYPE_EXT_PHY_QOS, rtk_ext_phy_qos);

	rkt_all_mac_portmask->rtk_init_all_mac_portmask_without_cpu = rtk_init_all_mac_portmask_without_cpu;
	
	rtk_fc_helper_register(FC_HELPER_TYPE_ALL_MAC_PORTMASK, rkt_all_mac_portmask);
	
//#if defined(CONFIG_RTK_L34_NETFILTER_HOOK)
	rtk_fc_ext_ct_cache_init();
//#endif

#if defined(CONFIG_FC_RTL8198F_SERIES)
	rtk83xxqos->rtk_get_83xx_qos_status	= rtk_get_83xx_qos_status;
	rtk83xxqos->rtk_83xx_qos_set_vlan_tag	= rtk_83xx_qos_set_vlan_tag;
	rtk83xxqos->rtk_pass_cos_to_83xx_qos 	= rtk_pass_cos_to_83xx_qos;
	
	rtk_fc_helper_register(FC_HELPER_TYPE_83XX_QOS, rtk83xxqos);

#if defined(CONFIG_RTK_8367_RELAY)
	rtk_8367r_relay_mc->rtk_l2_set_8367_ipMcastAddr = rtk_l2_set_8367_ipMcastAddr;
	rtk_8367r_relay_mc->rtk_fc_get_portNumber_in_8367 = rtk_fc_get_portNumber_in_8367;
	rtk_8367r_relay_mc->rtk_fc_check_8367Port = rtk_fc_check_8367Port; 
	rtk_8367r_relay_mc->rtk_l2_del_8367_ipMcastAddr = rtk_l2_del_8367_ipMcastAddr;
	rtk_8367r_relay_mc->rtk_l2_del_8367_ip6McastAddr = rtk_l2_del_8367_ip6McastAddr;
	rtk_8367r_relay_mc->rtk_l2_8367_igmp_protocol_init = rtk_l2_8367_igmp_protocol_init;
	rtk_fc_helper_register(FC_HELPER_TYPE_8367R_RELAY_MC, rtk_8367r_relay_mc);
#endif
#endif

	rtk_mcast_api->rtk_fc_igmp_mdb_hdler = rtk_fc_igmp_mdb_callback_handler;
	rtk_mcast_api->rtk_fc_igmp_mdb_isReg = rtk_fc_igmp_mdb_isRegister;
#if defined(CONFIG_RTK_SOC_RTL8198D)
	rtk_mcast_api->rtk_fc_check_user_group = rtk_fc_check_user_group;
#endif

	rtk_fc_helper_register(FC_HELPER_TYPE_MC, rtk_mcast_api);

#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_RTL8198XB_SERIES) 
	rtk_ext_flow_mib->rtk_fc_get_ext_flow_mib_control = rtk_fc_get_ext_flow_mib_control;
	rtk_ext_flow_mib->rtk_fc_set_ext_flow_mib_control = rtk_fc_set_ext_flow_mib_control;
	rtk_ext_flow_mib->rtk_fc_print_ext_flow_mib_count_all = rtk_fc_print_ext_flow_mib_count_all;
	rtk_ext_flow_mib->rtk_fc_add_ext_flow_mib_count_by_lan_mac = rtk_fc_add_ext_flow_mib_count_by_lan_mac;
	rtk_ext_flow_mib->rtk_fc_delete_ext_flow_mib_count_by_lan_mac = rtk_fc_delete_ext_flow_mib_count_by_lan_mac;
	rtk_ext_flow_mib->rtk_fc_flush_ext_flow_mib_count_by_lan_mac = rtk_fc_flush_ext_flow_mib_count_by_lan_mac;
	rtk_ext_flow_mib->rtk_fc_get_ext_flow_mib_count_by_lan_mac = rtk_fc_get_ext_flow_mib_count_by_lan_mac;
	rtk_ext_flow_mib->rtk_fc_delete_ext_flow_mib_count_and_flow_by_lan_mac = rtk_fc_delete_ext_flow_mib_count_and_flow_by_lan_mac;
#if defined(CONFIG_RTK_SOC_RTL8198D)
	rtk_ext_flow_mib->rtk_fc_get_ext_flow_mib_control_mode = rtk_fc_get_ext_flow_mib_control_mode;
	rtk_ext_flow_mib->rtk_fc_set_ext_flow_mib_control_mode = rtk_fc_set_ext_flow_mib_control_mode;
	rtk_ext_flow_mib->rtk_fc_set_ext_flow_mib_debug = rtk_fc_set_ext_flow_mib_debug;
	rtk_ext_flow_mib->rtk_fc_skb_egress_count_in_ext_flow_mib = rtk_fc_skb_egress_count_in_ext_flow_mib;
	rtk_ext_flow_mib->rtk_fc_add_ext_flow_mib_count_by_lan_ip = rtk_fc_add_ext_flow_mib_count_by_lan_ip;
	rtk_ext_flow_mib->rtk_fc_add_eth_ext_flow_mib_count_by_lan_ip = rtk_fc_add_eth_ext_flow_mib_count_by_lan_ip;
	rtk_ext_flow_mib->rtk_fc_delete_ext_flow_mib_count_by_lan_ip = rtk_fc_delete_ext_flow_mib_count_by_lan_ip;
	rtk_ext_flow_mib->rtk_fc_flush_ext_flow_mib_count_by_lan_ip = rtk_fc_flush_ext_flow_mib_count_by_lan_ip;
	rtk_ext_flow_mib->rtk_fc_get_ext_flow_mib_count_by_lan_ip = rtk_fc_get_ext_flow_mib_count_by_lan_ip;
	rtk_ext_flow_mib->rtk_fc_delete_ext_flow_mib_count_and_flow_by_lan_ip = rtk_fc_delete_ext_flow_mib_count_and_flow_by_lan_ip;
	rtk_ext_flow_mib->rtk_fc_update_ext_flow_mib_by_lan_ip = rtk_fc_update_ext_flow_mib_by_lan_ip;
	rtk_ext_flow_mib->rtk_fc_update_ext_flow_mib_by_lan_mac = rtk_fc_update_ext_flow_mib_by_lan_mac;
#endif
	rtk_fc_helper_register(FC_HELPER_TYPE_EXT_FLOW_MIB, rtk_ext_flow_mib);
#endif

	rtk_ipfrag->rtk_fc_get_ipfrag_max_hash_size = rtk_fc_get_ipfrag_max_hash_size;
	rtk_ipfrag->rtk_fc_get_ipfrag_max_hash_entry_size = rtk_fc_get_ipfrag_max_hash_entry_size;
	rtk_ipfrag->rtk_fc_get_ipfrag_cache_timeout_msec = rtk_fc_get_ipfrag_cache_timeout_msec;
	rtk_fc_helper_register(FC_HELPER_TYPE_IPFRAG, rtk_ipfrag);

#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_FC_RTL8198F_SERIES) || defined(CONFIG_RTL8198X_SERIES)
	rtk_fc_tcp_helper->rtk_fc_tcp_in_window = rtk_fc_tcp_in_window;
	rtk_fc_helper_register(FC_HELPER_TYPE_TCP, rtk_fc_tcp_helper);
#endif

	rtk_fc_helper_register(FC_HELPER_TYPE_END, NULL);
	return 0;
}

void rtk_fc_helper_exit(void)
{
	FCMGR_PRK("helper func exit\n");

	rtk_fc_helper_unregister(FC_HELPER_TYPE_CONVERTER);
	rtk_fc_helper_unregister(FC_HELPER_TYPE_PS);
	rtk_fc_helper_unregister(FC_HELPER_TYPE_MULTI_WAN);
	rtk_fc_helper_unregister(FC_HELPER_TYPE_VLAN);
	rtk_fc_helper_unregister(FC_HELPER_TYPE_WLAN);
	rtk_fc_helper_unregister(FC_HELPER_TYPE_RT_API);
	rtk_fc_helper_unregister(FC_HELPER_TYPE_ALL_MAC_PORTMASK);
#if defined(CONFIG_FC_RTL8198F_SERIES)
	rtk_fc_helper_unregister(FC_HELPER_TYPE_83XX_QOS);
	rtk_fc_helper_unregister(FC_HELPER_TYPE_8367R_RELAY_MC);
#endif
	rtk_fc_helper_unregister(FC_HELPER_TYPE_MC);
#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_RTL8198XB_SERIES)
	rtk_fc_helper_unregister(FC_HELPER_TYPE_EXT_FLOW_MIB);
#endif
	rtk_fc_helper_unregister(FC_HELPER_TYPE_IPFRAG);
#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_FC_RTL8198F_SERIES) || defined(CONFIG_RTL8198X_SERIES)
	rtk_fc_helper_unregister(FC_HELPER_TYPE_TCP);
#endif
//#if defined(CONFIG_RTK_L34_NETFILTER_HOOK)
	rtk_fc_ext_ct_cache_exit();
//#endif


	return;
}





