#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/if_vlan.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <uapi/linux/netfilter_bridge.h>
#include <uapi/linux/netfilter_arp.h>
#include <linux/netfilter_bridge.h>
#include <rt_edp_struct.h>
#include <rt_edp_debug.h>

rt_edp_globalDatabase_t	rt_edp_db;
rt_edp_globalKernel_t rt_edp_kernel;

void _rt_edp_displayInfo(struct sk_buff *skb,const struct net_device *in, const struct net_device *out, int trace_type)
{
	struct ethhdr *eth=NULL;
	struct iphdr *ip=NULL;
	struct ipv6hdr *ipv6h = NULL;
	struct tcphdr *tcp=NULL;
	struct udphdr *udp=NULL;
	struct vlan_dev_priv *vlan=vlan_dev_priv(skb->dev);

	if(vlan)
	{
		TRACE_X(trace_type,"VLAN1 PROTO=%x VID=%d\n",vlan->vlan_proto,vlan->vlan_id);
	}



	eth=eth_hdr(skb);
	if(skb->protocol==ETH_P_IP)
	{
		ip=ip_hdr(skb);
		if(ip->protocol==IPPROTO_TCP) tcp=(struct tcphdr *)((u32)ip+(ip->ihl<<2));
		if(ip->protocol==IPPROTO_UDP) udp=(struct udphdr *)((u32)ip+(ip->ihl<<2));
	}
	if(skb->protocol==ETH_P_IPV6)
	{
		ipv6h = ipv6_hdr(skb);
		if(ipv6h->nexthdr==IPPROTO_TCP) tcp=(struct tcphdr *)((u32)ipv6h+sizeof(struct ipv6hdr));
		if(ipv6h->nexthdr==IPPROTO_UDP) udp=(struct udphdr *)((u32)ipv6h+sizeof(struct ipv6hdr));
	}

	TRACE_X(trace_type,"FWD_IN[%x]: DA=%02x:%02x:%02x:%02x:%02x:%02x SA=%02x:%02x:%02x:%02x:%02x:%02x ETH=%04x len=%d inIntf=%s (EXT)SPA=%d\n        EXTMASK=0x%x%s Reason=%d IntPri=%d stagIf=%d(VID=%d, PRI=%d, DEI=%d, TPID=0x%x) ctagIf=%d(VID=%d, PRI=%d, CFI=%d)",
		(unsigned int)skb&0xffff,
		eth->h_dest[0],eth->h_dest[1],eth->h_dest[2],eth->h_dest[3],eth->h_dest[4],eth->h_dest[5],
		eth->h_source[0],eth->h_source[1],eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5],
		skb->protocol,
		skb->len,
		skb->dev->name,
		/*pPktHdr->ingressPort*/0,
		/*pPktHdr->pRxDesc->rx_dst_port_mask*/0,
		/*(pPktHdr->pRxDesc->rx_dst_port_mask==0x20 && pPktHdr->pRxDesc->rx_igrLocation==0x2)?"(IGMP/MLD) ":((pPktHdr->pRxDesc->rx_dst_port_mask==0x20 && pPktHdr->pRxDesc->rx_igrLocation==0x1)?"(ARP/ND) ":((pPktHdr->pRxDesc->rx_dst_port_mask==0x20 && pPktHdr->pRxDesc->rx_igrLocation==0x0)?"(fromPS) ":((pPktHdr->pRxDesc->rx_dst_port_mask==0x10)?"(fromEXT1) ":((pPktHdr->pRxDesc->rx_dst_port_mask==0x8)?"(fromEXT0) ":((pPktHdr->pRxDesc->rx_dst_port_mask==0x4)?"(toEXT1) ":((pPktHdr->pRxDesc->rx_dst_port_mask==0x2)?"(toEXT0) ":"(toCPU) "))))))*/"",
		/*pPktHdr->pRxDesc->rx_reason*/0,
		/*pPktHdr->pRxDesc->rx_internal_priority*/0,
		/*(pPktHdr->tagif&SVLAN_TAGIF)>0*/0,
		/*(pPktHdr->tagif&SVLAN_TAGIF)>0?pPktHdr->stagVid:0*/0,
		/*(pPktHdr->tagif&SVLAN_TAGIF)>0?pPktHdr->stagPri:0*/0,
		/*(pPktHdr->tagif&SVLAN_TAGIF)>0?pPktHdr->stagDei:0*/0,
		/*(pPktHdr->tagif&SVLAN_TAGIF)>0?pPktHdr->stagTpid:0*/0,
		(skb->vlan_tci & VLAN_TAG_PRESENT)?1:0,
		skb->vlan_tci & VLAN_VID_MASK,
		(skb->vlan_tci & VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT,
		(skb->vlan_tci & VLAN_CFI_MASK)?1:0);

	if(ip)
	{
		TRACE_X(trace_type,"Ingress Src=%d.%d.%d.%d(%d) Dst=%d.%d.%d.%d(%d) %s L4_PROTO=0x%02x %s%s%s%s%s%s"
			,(ntohl(ip->saddr)>>24)&0xff,(ntohl(ip->saddr)>>16)&0xff,(ntohl(ip->saddr)>>8)&0xff,(ntohl(ip->saddr))&0xff,(tcp)?ntohs(tcp->source):((udp)?ntohs(udp->source):0)
			,(ntohl(ip->daddr)>>24)&0xff,(ntohl(ip->daddr)>>16)&0xff,(ntohl(ip->daddr)>>8)&0xff,(ntohl(ip->daddr))&0xff,(tcp)?ntohs(tcp->dest):((udp)?ntohs(udp->dest):0)
			,(ip->protocol==IPPROTO_TCP)?"TCP":((ip->protocol==IPPROTO_UDP)?"UDP":((ip->protocol==IPPROTO_ICMP)?"ICMP":((ip->protocol==IPPROTO_IGMP)?"IGMP":"OTHER")))
			,ip->protocol
			,(tcp)?"Flags:":""
			,(tcp&&tcp->syn)?"SYN ":""
			,(tcp&&tcp->ack)?"ACK ":""
			,(tcp&&tcp->psh)?"PSH ":""
			,(tcp&&tcp->fin)?"FIN ":""
			,(tcp&&tcp->rst)?"RST":""
			);
	}
	if(ipv6h)
	{
		TRACE_X(trace_type, "Ingress Src=%pI6(%d) Dst=%pI6(%d) %s L4_PROTO=0x%02x TC=%d(0x%01X) FLOWLEBEL=%d(0x%x)"
			,&ipv6h->saddr,(tcp)?ntohs(tcp->source):((udp)?ntohs(udp->source):0)
			,&ipv6h->daddr,(tcp)?ntohs(tcp->dest):((udp)?ntohs(udp->dest):0)
			,(ipv6h->nexthdr==IPPROTO_TCP)?"TCP":((ipv6h->nexthdr==IPPROTO_UDP)?"UDP":((ipv6h->nexthdr==0x3a)?"ICMPv6":((ipv6h->nexthdr==IPPROTO_IGMP)?"IGMP":"OTHER")))
			,ipv6h->nexthdr
			,((ipv6h->priority << 4) | ((ipv6h->flow_lbl[0] & 0xF0) >> 4)),((ipv6h->priority << 4) | ((ipv6h->flow_lbl[0] & 0xF0) >> 4))
			,(((ipv6h->flow_lbl[0] & 0x0F) << 16 ) | (ipv6h->flow_lbl[1] << 8) | ipv6h->flow_lbl[2]),(((ipv6h->flow_lbl[0] & 0x0F) << 16 ) | (ipv6h->flow_lbl[1] << 8) | ipv6h->flow_lbl[2])
		);
	}


/*
#ifdef CONFIG_APOLLO_MODEL
#else

#if defined(CONFIG_RG_DEBUG)
	if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_TRACE_DUMP)
	{
		int trace=1;
		if(rg_kernel.filter_level& RTK_RG_DEBUG_LEVEL_TRACE_DUMP)
		{
			trace = rg_kernel.tracefilterShow;
			//trace=_rtk_rg_trace_filter_compare(skb,pPktHdr);
		}

		if(trace==1)
		{
			dump_packet(skb->data,skb->len,"trace_dump");
		}
	}
#endif
#endif
*/


}



unsigned int prerouting_before_mangle(const struct nf_hook_ops *ops,
			       struct sk_buff *skb,
			       const struct net_device *in,
			       const struct net_device *out,
			       int (*okfn)(struct sk_buff *))
{

	TRACE_PREROUTING("[IPT] before mangle skb=%x",(u32)skb);
	return NF_ACCEPT;
}

unsigned int prerouting_after_mangle(const struct nf_hook_ops *ops,
			       struct sk_buff *skb,
			       const struct net_device *in,
			       const struct net_device *out,
			       int (*okfn)(struct sk_buff *))
{
	TRACE_PREROUTING("[IPT] after mangle skb=%x",(u32)skb);
	return NF_ACCEPT;
}
unsigned int prerouting_before_nat(const struct nf_hook_ops *ops,
			       struct sk_buff *skb,
			       const struct net_device *in,
			       const struct net_device *out,
			       int (*okfn)(struct sk_buff *))
{
	TRACE_PREROUTING("[IPT] before nat skb=%x",(u32)skb);
	return NF_ACCEPT;
}
unsigned int prerouting_after_nat(const struct nf_hook_ops *ops,
			       struct sk_buff *skb,
			       const struct net_device *in,
			       const struct net_device *out,
			       int (*okfn)(struct sk_buff *))
{
	TRACE_PREROUTING("[IPT] after nat skb=%x",(u32)skb);
	return NF_ACCEPT;
}


unsigned int localin_before_filter(const struct nf_hook_ops *ops,
			       struct sk_buff *skb,
			       const struct net_device *in,
			       const struct net_device *out,
			       int (*okfn)(struct sk_buff *))
{

	TRACE_LOCALIN("[IPT] before filter skb=%x",(u32)skb);
	TRACE_LOCALIN("[ToPS]");
	return NF_ACCEPT;
}


unsigned int postrouting_before_mangle(const struct nf_hook_ops *ops,
			       struct sk_buff *skb,
			       const struct net_device *in,
			       const struct net_device *out,
			       int (*okfn)(struct sk_buff *))
{
	TRACE_POSTROUTING("[IPT] before mangle skb=%x",(u32)skb);
	return NF_ACCEPT;
}

unsigned int postrouting_after_mangle(const struct nf_hook_ops *ops,
			       struct sk_buff *skb,
			       const struct net_device *in,
			       const struct net_device *out,
			       int (*okfn)(struct sk_buff *))
{
	TRACE_POSTROUTING("[IPT] after mangle skb=%x",(u32)skb);
	return NF_ACCEPT;
}
unsigned int postrouting_before_nat(const struct nf_hook_ops *ops,
			       struct sk_buff *skb,
			       const struct net_device *in,
			       const struct net_device *out,
			       int (*okfn)(struct sk_buff *))
{
	//_rt_edp_displayInfo(skb,in,out);
	TRACE_POSTROUTING("[IPT] before nat skb=%x",(u32)skb);
	//rt_edp_memDump(skb->data-14,64,"beforepost");
	return NF_ACCEPT;
}
unsigned int postrouting_after_nat(const struct nf_hook_ops *ops,
			       struct sk_buff *skb,
			       const struct net_device *in,
			       const struct net_device *out,
			       int (*okfn)(struct sk_buff *))
{


	//_rt_edp_displayInfo(skb,in,out);
	TRACE_POSTROUTING("[IPT] after nat skb=%x",(u32)skb);
	//rt_edp_memDump(skb->data-14,64,"afterpost");

#if defined(EDP_ACL_SUPPORT_HOOK_CHECK) && (EDP_ACL_SUPPORT_HOOK_CHECK==1)
	assert_ok(_rt_edp_egressACLPatternCheckAndAction(skb));
	assert_ok(_rt_edp_modifyPacketByACLAction(skb));
#endif	//EDP_ACL_SUPPORT_HOOK_CHECK

	return NF_ACCEPT;
}


unsigned int localout_after_filter(const struct nf_hook_ops *ops,
			       struct sk_buff *skb,
			       const struct net_device *in,
			       const struct net_device *out,
			       int (*okfn)(struct sk_buff *))
{
	//_rt_edp_displayInfo(skb,in,out);
	TRACE_LOCALOUT("==================== PACKET BOUNDARY (IPT LocalOut)====================");
	//rt_edp_memDump(skb->data-14,64,"localout");

#if defined(EDP_ACL_SUPPORT_HOOK_CHECK) && (EDP_ACL_SUPPORT_HOOK_CHECK==1)
	assert_ok(_rt_edp_ingressACLPatternCheckAndAction(skb));
#endif	//EDP_ACL_SUPPORT_HOOK_CHECK

	return NF_ACCEPT;
}


unsigned int ebt_prerouting_first(const struct nf_hook_ops *ops,
			       struct sk_buff *skb,
			       const struct net_device *in,
			       const struct net_device *out,
			       int (*okfn)(struct sk_buff *))
{

	TRACE_PREROUTING("==================== PACKET BOUNDARY (EBT PreRouting)====================");
	_rt_edp_displayInfo(skb,in,out,RT_EDP_DEBUG_LEVEL_TRACE_PREROUTING);
	//rt_edp_memDump(skb->data-14,64,"localout");

#if defined(EDP_ACL_SUPPORT_HOOK_CHECK) && (EDP_ACL_SUPPORT_HOOK_CHECK==1)
	assert_ok(_rt_edp_ingressACLPatternCheckAndAction(skb));
#endif	//EDP_ACL_SUPPORT_HOOK_CHECK

	TRACE_PREROUTING("ebtable first\n");
	return NF_ACCEPT;
}


unsigned int ebt_forward_brnf(const struct nf_hook_ops *ops,
			       struct sk_buff *skb,
			       const struct net_device *in,
			       const struct net_device *out,
			       int (*okfn)(struct sk_buff *))
{

	TRACE_FORWARD("[EBT] forward_brnf=%x",(u32)skb);
	return NF_ACCEPT;
}


unsigned int ebt_localout_after_filter(const struct nf_hook_ops *ops,
			       struct sk_buff *skb,
			       const struct net_device *in,
			       const struct net_device *out,
			       int (*okfn)(struct sk_buff *))
{
	//_rt_edp_displayInfo(skb,in,out);
	TRACE_LOCALOUT("==================== PACKET BOUNDARY (EBT LocalOut)====================");
	//rt_edp_memDump(skb->data-14,64,"localout");
	return NF_ACCEPT;
}


unsigned int ebt_postrouting_last(const struct nf_hook_ops *ops,
			       struct sk_buff *skb,
			       const struct net_device *in,
			       const struct net_device *out,
			       int (*okfn)(struct sk_buff *))
{
	_rt_edp_displayInfo(skb,in,out,RT_EDP_DEBUG_LEVEL_TRACE_POSTROUTING);
	//rt_edp_memDump(skb->data-14,64,"localout");
	//TRACE_POSTROUTING("ebtable last\n");
	TRACE_POSTROUTING("[EBT][ToFC TX]\n");
	return NF_ACCEPT;
}


unsigned int arpt_localin_before_filter(const struct nf_hook_ops *ops,
			       struct sk_buff *skb,
			       const struct net_device *in,
			       const struct net_device *out,
			       int (*okfn)(struct sk_buff *))
{

	TRACE_LOCALIN("==================== PACKET BOUNDARY (ARPT LocalIn)====================");
	_rt_edp_displayInfo(skb,in,out,RT_EDP_DEBUG_LEVEL_TRACE_LOCAL_IN);
	TRACE_LOCALIN("[ToPS]");
	return NF_ACCEPT;
}


unsigned int arpt_localout_after_filter(const struct nf_hook_ops *ops,
			       struct sk_buff *skb,
			       const struct net_device *in,
			       const struct net_device *out,
			       int (*okfn)(struct sk_buff *))
{

	TRACE_LOCALOUT("==================== PACKET BOUNDARY (ARPT LocalOut)====================");
	_rt_edp_displayInfo(skb,in,out,RT_EDP_DEBUG_LEVEL_TRACE_LOCAL_OUT);
	return NF_ACCEPT;
}


static struct nf_hook_ops hook_ops[] __read_mostly = {

	//iptables
    {
        .hook 		= prerouting_before_mangle,
        .pf 		= PF_INET,
        .hooknum 	= NF_INET_PRE_ROUTING,
        .priority 	= NF_IP_PRI_MANGLE-1,
        .owner 		= THIS_MODULE,
    }, {
        .hook 		= prerouting_after_mangle,
        .pf 		= PF_INET,
        .hooknum 	= NF_INET_PRE_ROUTING,
        .priority 	= NF_IP_PRI_MANGLE+1,
        .owner 		= THIS_MODULE,
    }, {
        .hook 		= prerouting_before_nat,
        .pf 		= PF_INET,
        .hooknum 	= NF_INET_PRE_ROUTING,
        .priority 	= NF_IP_PRI_NAT_DST-1,
        .owner 		= THIS_MODULE,
	}, {
		.hook		= localin_before_filter,
		.pf 		= PF_INET,
		.hooknum	= NF_INET_LOCAL_IN,
		.priority	= NF_IP_PRI_FILTER-1,
		.owner		= THIS_MODULE,
	}, {
        .hook 		= prerouting_after_nat,
        .pf 		= PF_INET,
        .hooknum 	= NF_INET_PRE_ROUTING,
        .priority 	= NF_IP_PRI_NAT_SRC+1,
        .owner 		= THIS_MODULE,
    }, {
        .hook 		= postrouting_before_mangle,
        .pf 		= PF_INET,
        .hooknum 	= NF_INET_POST_ROUTING,
        .priority 	= NF_IP_PRI_MANGLE-1,
        .owner 		= THIS_MODULE,
    }, {
        .hook 		= postrouting_after_mangle,
        .pf 		= PF_INET,
        .hooknum 	= NF_INET_POST_ROUTING,
        .priority 	= NF_IP_PRI_MANGLE+1,
        .owner 		= THIS_MODULE,
    }, {
        .hook 		= postrouting_before_nat,
        .pf 		= PF_INET,
        .hooknum 	= NF_INET_POST_ROUTING,
        .priority 	= NF_IP_PRI_NAT_DST-1,
        .owner 		= THIS_MODULE,
    }, {
        .hook 		= postrouting_after_nat,
        .pf 		= PF_INET,
        .hooknum 	= NF_INET_POST_ROUTING,
        .priority 	= NF_IP_PRI_NAT_SRC+1,
        .owner		= THIS_MODULE,
    },
    {
        .hook 		= localout_after_filter,
        .pf 		= PF_INET,
        .hooknum	= NF_INET_LOCAL_OUT,
        .priority 	= NF_IP_PRI_FILTER+1,
        .owner 		= THIS_MODULE,
    },

	// ip6ables


	// ebtables
	{
		.hook		= ebt_prerouting_first,
		.pf			= PF_BRIDGE,
		.hooknum	= NF_BR_PRE_ROUTING,
		.priority	= NF_BR_PRI_FIRST,
		.owner		= THIS_MODULE,
	},

	{
		.hook		= ebt_forward_brnf,
		.pf			= PF_BRIDGE,
		.hooknum	= NF_BR_FORWARD,
		.priority	= NF_BR_PRI_BRNF,
		.owner		= THIS_MODULE,
	},
	{
		.hook		= ebt_postrouting_last,
		.pf			= PF_BRIDGE,
		.hooknum	= NF_BR_POST_ROUTING,
		.priority	= NF_BR_PRI_LAST,
		.owner		= THIS_MODULE,
	},

    {
        .hook 		= ebt_localout_after_filter,
        .pf 		= PF_BRIDGE,
        .hooknum 	= NF_BR_LOCAL_OUT,
        .priority 	= NF_BR_PRI_FILTER_OTHER+1,
        .owner 		= THIS_MODULE,
    },


	//arptables
    {
        .hook 		= arpt_localin_before_filter,
        .pf 		= NFPROTO_ARP,
        .hooknum 	= NF_ARP_IN,
        .priority 	= NF_IP_PRI_FILTER-1,
        .owner 		= THIS_MODULE,
    },

	{
		.hook		= arpt_localout_after_filter,
		.pf 		= NFPROTO_ARP,
		.hooknum	= NF_ARP_OUT,
		.priority	= NF_IP_PRI_FILTER+1,
		.owner		= THIS_MODULE,
	},

};

#ifdef CONFIG_HWNAT_FLEETCONNTRACK_EDP
void rt_edp_proc_init(void);

#endif

static int __init init_modules(void)
{
#if defined(CONFIG_HWNAT_FLEETCONNTRACK_EDP)
	rt_edp_proc_init();
#endif

    if (nf_register_hooks(hook_ops, ARRAY_SIZE(hook_ops)) < 0) {
        printk("nf_register_hook failed\n");
    }


	memset(&rt_edp_db,0, sizeof(rt_edp_globalDatabase_t));
	memset(&rt_edp_kernel,0, sizeof(rt_edp_globalKernel_t));

	rt_edp_kernel.debug_level = RT_EDP_DEBUG_LEVEL_WARN;
	rt_edp_kernel.filter_level = 0;

	return 0;

}

static void __exit exit_modules(void)
{
    nf_unregister_hooks(hook_ops, ARRAY_SIZE(hook_ops));
}

module_init(init_modules);
module_exit(exit_modules);


