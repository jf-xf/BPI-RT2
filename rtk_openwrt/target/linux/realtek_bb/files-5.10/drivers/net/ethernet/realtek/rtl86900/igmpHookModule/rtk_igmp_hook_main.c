#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#include <net/ip6_route.h>
//#include <net/dst.h>
//#include <net/ipv6.h>
#include <net/ip.h>

#include <rtk_igmp_hook.h>
#include <rtk_igmp_debug.h>
#include <rtk_igmp_tinyPS_mcfwd.h>
#include <rtk_igmp_proc.h>
#include <rtk_igmp_snooping.h>
#include <rtk_igmp_hook_module_api.h>


rtk_igmp_Sysdb_t  igmpSysdb;


void *rtk_igmp_malloc(int NBYTES)
{
	if(NBYTES==0) return NULL;
	return (void *)kmalloc(NBYTES,GFP_ATOMIC);
}
void rtk_igmp_free(void *APTR)
{
	kfree(APTR);
}


int rtk_igmp_packetParser( struct sk_buff *skb ,rtk_igmp_pktHdr_t *pPktHdr,const struct net_device *SrcDev,const struct net_device *DstDev)
{
	int nexthdr = 0;
	int offset=0;
	u8 *pData =NULL;
	//uint8 isRouting = FALSE;


	memset(pPktHdr,0,sizeof(rtk_igmp_pktHdr_t));
	pPktHdr->skb = skb;
	if(SrcDev)
	{
		pPktHdr->ethHdr = (struct ethhdr *)skb_mac_header(skb);
		pData = skb_mac_header(skb);
	}
	else
	{
		//routing packet difficult to find ethhdr
		pPktHdr->ethHdr=NULL;
		offset=-2;
		pData = skb_network_header(skb);
		IGMP_PARSER("pData = %p  *(pData+offset)=%x",pData,*(pData));
		if( ((*(pData))>>4)==0x4)
		{
			goto V4HDR;
		}
		else if( ((*(pData))>>4)==0x6)			
		{
			goto V6HDR;
		}
		else
		{
			IGMP_PARSER("parser Error none-v4v6 packet");
			return FAILED;
		}

	}
	
	offset+=(ETH_HLEN-2);

	//Cvlan +Svlan
	while(1)
	{
		if(((*(u16*)(pData+offset)) == htons(ETH_P_8021AD)) || ((*(u16*)(pData+offset)) == htons(ETH_P_8021Q)))
		{
			offset+=VLAN_HLEN;
		}
		else
			break;
	}

	//PPPoE
	if(((*(u16*)(pData+offset)) == htons(ETH_P_PPP_SES)) || ((*(u16*)(pData+offset)) == htons(ETH_P_PPP_DISC)))
	{
		offset += PPPOE_SES_HLEN;
	}

	if(((*(u16*)(pData+offset)) == htons(ETH_P_IP)) || ((*(u16*)(pData+offset)) == htons(0x0021)) || (pData[offset] == 0x21))
	{
		if(pData[offset]==0x21)offset-=1;
V4HDR:
		pPktHdr->iph = (struct iphdr *)((unsigned char *)(pData)+offset+2);
		offset += ((pPktHdr->iph->ihl<<2)+2);
		pPktHdr->l4proto= pPktHdr->iph->protocol;
	}
	else if((*(u16*)(pData+offset)) == htons(ETH_P_IPV6) ||((*(u16*)(pData+offset)) == htons(0x0057)) || (pData[offset] == 0x57))
	{
		if(pData[offset]==0x57)offset-=1;
V6HDR:		
		pPktHdr->ip6h = (struct ipv6hdr *)((unsigned char *)(pData)+offset+2);
		offset +=(sizeof(struct ipv6hdr)+2);
		nexthdr =pPktHdr->ip6h->nexthdr;

MORE_IPV6_EXT:
		//IGMP_PARSER("nexthdr=%d offset=%d",nexthdr,offset);
		switch(nexthdr)
		{
			case (IPPROTO_IPIP):
			{
				//2[dslite]
				//if(isRouting){
					offset -= 2; //minus 2 for IP header is is start from ((unsigned char *)(pSkb->data)+off+2)
					goto V4HDR;
				//}
				break;
			}
			case (IPPROTO_HOPOPTS):
			{
				//TRACE("Parser Hop-by-Hop");
				nexthdr = *((unsigned char *)(pData)+offset);
				offset += ((*((unsigned char *)(pData)+offset+1))+1)<<3 ; //( (nexthdr.len *8) +8 )
				goto MORE_IPV6_EXT;
			}
			case (IPPROTO_ROUTING):
			{
				//TRACE("Parser IPPROTO_ROUTING");
				nexthdr = *((unsigned char *)(pData)+offset);
				offset += ((*((unsigned char *)(pData)+offset+1))+1)<<3 ; //( (nexthdr.len *8) +8 )
				goto MORE_IPV6_EXT;
			}
			case (IPPROTO_FRAGMENT):
			{
				//TRACE("Parser Fragment pkt");
				nexthdr = *((unsigned char *)(pData)+offset);
				offset+=8;
				pPktHdr->isFragmentPkt=1;
				goto MORE_IPV6_EXT;
			}
			case (IPPROTO_ICMPV6):
			{
				break;
			}
			case (IPPROTO_NONE):
			case (IPPROTO_DSTOPTS):
			case (IPPROTO_MH):
				break;

			default:
				break;
		}

		if(pPktHdr->iph)
			pPktHdr->l4proto= pPktHdr->iph->protocol;
		else
			pPktHdr->l4proto = nexthdr;

	}

	if(pPktHdr->iph==NULL && pPktHdr->ip6h==NULL)
	{
		IGMP_PARSER("parser Error none-v4v6 packet");
		return FAILED;
	}

	if(pPktHdr->iph)
	{
		//#define IP_MF 	0x2000		/* Flag: "More Fragments"	*/
		//#define IP_OFFSET 0x1FFF		/* "Fragment Offset" part	*/
		uint16 fragOff = ntohs(pPktHdr->iph->frag_off)& 0x1FFF ;
		uint16 moreFrag = (ntohs(pPktHdr->iph->frag_off)&0x2000)?TRUE:FALSE  ; // MFbits
		if(fragOff || moreFrag)
			pPktHdr->isFragmentPkt=1;
	}
	
	if(!pPktHdr->isFragmentPkt)
	{
		switch(pPktHdr->l4proto)
		{
			case (IPPROTO_IGMP):
				pPktHdr->igmph= (struct igmphdr *)((unsigned char *)(pData)+offset);
				break;
			case (IPPROTO_ICMPV6):
				pPktHdr->icmp6h = (struct icmp6hdr *)((unsigned char *)(pData)+offset);
				break;
			case (IPPROTO_UDP):
				pPktHdr->udph = (struct udphdr *)((unsigned char *)(pData)+offset);
				break;
			default:
				IGMP_PARSER("L4Proto %d",pPktHdr->l4proto);
				break;
		}
	}

	pPktHdr->devIfindex = skb->dev->ifindex;

	if(pPktHdr->iph && ((ntohl(pPktHdr->iph->daddr) & 0xf0000000)== 0xe0000000) )
		pPktHdr->isDipMc=1;
	else if(pPktHdr->ip6h&& (((pPktHdr->ip6h->daddr.s6_addr[0])& 0xff)== 0xff) )
		pPktHdr->isDipMc=1;


#if defined(CONFIG_RTL_NEW_FLOW_BASE_HWNAT_DRIVER) || defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
	// ref to rtk_fc_enum_pktHdrTagif_e
	#define SVLAN_TAGIF (1<<0)
	#define CVLAN_TAGIF (1<<1)
	// In fact , I don't want to binding with fc driver  hmm....
	if(skb->fcIngressData.ingressTagif &CVLAN_TAGIF)
	{
		pPktHdr->ingressCtagif=1;
		pPktHdr->ingressCvid =skb->fcIngressData.srcCVlanId;
		IGMP_PARSER("VLAN BY FC  CVID=%d",pPktHdr->ingressCvid);		
	}
	if(skb->fcIngressData.ingressTagif &SVLAN_TAGIF)
	{
		pPktHdr->ingressStagif=1;
		pPktHdr->ingressSvid =skb->fcIngressData.srcSVlanId;
		IGMP_PARSER("SVLAN BY FC  SVID=%d",pPktHdr->ingressSvid);		
	}	
	memcpy(pPktHdr->smac,skb->fcIngressData.sa,sizeof(pPktHdr->smac));
	pPktHdr->isCopyToCpu = skb->fcIngressData.isHwCopy2CPU;
#endif

#if defined(CONFIG_RTL8367_SUPPORT) && defined(CONFIG_RTL9607C_SERIES) && defined(CONFIG_RTK_SKB_MARK2)

	if(igmpSysdb.skbMark2_ExtPortIDLen)
	{
		pPktHdr->DevExtPortEn = (uint32)(skb->mark2>>igmpSysdb.skbMark2_ExtPortEnStartbit)&((1LL<<1)-1);
		pPktHdr->DevExtPortId = (uint32)(skb->mark2>>igmpSysdb.skbMark2_ExtPortIDStartbit)&((1LL<<igmpSysdb.skbMark2_ExtPortIDLen)-1);
	}
	IGMP_PARSER("SKB_MARK2 %llx  shift %llx %lld DevExtPortEn:%d",skb->mark2,skb->mark2>>igmpSysdb.skbMark2_ExtPortEnStartbit,((1LL<<1)-1),pPktHdr->DevExtPortEn);
#endif

#if 0//defined(CONFIG_RTL_SMUX_DEV)
	if(skb->original_vlan_count>=1)
	{
		pPktHdr->ingressCtagif=1;
		pPktHdr->ingressCvid = skb->original_vlan_header[skb->original_vlan_count-1];
		IGMP_PARSER("VLAN BY SMUX original_vlan_count=%d CVID=%d",skb->original_vlan_count,pPktHdr->ingressCvid);
	}
#endif

	IGMP_PARSER("eth=%p ingressCtagif:%d[vid:%d] ingressStagif:%d[svid:%d] ip6h=%p iph=%p icmp6h=%p igmph=%p isDipMc=%d l4proto=%d devIfindex=%d isCopyToCpu=%d",
		pPktHdr->ethHdr,pPktHdr->ingressCtagif,pPktHdr->ingressCvid,pPktHdr->ingressStagif,pPktHdr->ingressSvid,pPktHdr->ip6h,pPktHdr->iph,pPktHdr->icmp6h,pPktHdr->igmph,pPktHdr->isDipMc,pPktHdr->l4proto,pPktHdr->devIfindex,pPktHdr->isCopyToCpu);

	return 0;
}

uint32 skb_get_mtu(uint32 isIpv6 , struct sk_buff * skb)
{
	uint32 mtu=0;

	if(skb_dst(skb) == NULL)
	{
		//printk("\n%s:%d skb_dst(skb) is NULL!!!\n",__FUNCTION__,__LINE__);
		return mtu;
	}
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0)
	if(!isIpv6)
	{
		mtu=ip_skb_dst_mtu(skb->sk,skb);
	}
	else
	{
		mtu=ip6_skb_dst_mtu(skb);
	}
#else
	if(!isIpv6)
	{
		mtu=ip_skb_dst_mtu(skb);
	}
	else
	{
		mtu=ip6_skb_dst_mtu(skb);
	}
#endif
	return mtu;
}


unsigned int main_hook(const struct nf_hook_ops *ops,
			       struct sk_buff *skb,
			       const struct net_device *in,
			       const struct net_device *out,
			       int (*okfn)(struct sk_buff *))
{
	rtk_igmp_nf_ret_e_t ret=RTK_IGMP_NF_ACCEPT;
	rtk_igmp_pktHdr_t pkthdr;
	uint32 mtu;


	if(igmpSysdb.disHook)
		return RTK_IGMP_NF_ACCEPT;

	//ingore mac header was not set packets
	if(!skb_mac_header_was_set(skb)){IGMP_PARSER("ingore mac header was not set packets"); return RTK_IGMP_NF_ACCEPT;}
		
	//bridge packet  ingore uc mac
	if(in &&  !(skb_mac_header(skb)[0] & 0x1)){IGMP_PARSER("bridge packet  ingore uc mac"); return RTK_IGMP_NF_ACCEPT;}

	if(rtk_igmp_packetParser(skb,&pkthdr,in,out)!=SUCCESS)
	{
		IGMP_PARSER("IGMP Parser Error");
		return RTK_IGMP_NF_ACCEPT;
	}

	//ingore unicast packet
	if(pkthdr.isDipMc == 0 ){IGMP_PARSER("ingore not multicsast packet"); return RTK_IGMP_NF_ACCEPT;}
	if(pkthdr.isCopyToCpu) {IGMP_PARSER("IGNORE AND ACCEPT HW COPY TO CPU FLOW");return RTK_IGMP_NF_ACCEPT;}


	mtu=skb_get_mtu(pkthdr.ip6h?1:0,skb);
	if(mtu > 0 && skb->len > mtu)
	{
		IGMP_PARSER("ignore large packet len=%d mtu=%d",skb->len,mtu);
		return RTK_IGMP_NF_ACCEPT;
	}

	if(igmpSysdb.igmp_debug_level&RTK_IGMP_DEBUG_LEVEL_DUMP)
	{
		igmp_dump_packet(skb_mac_header(skb),skb->len+skb->mac_len,"IGMP");
		IGMP_PARSER("protocol=%d head=%p data=%p l3=%p l3offset=%d l4=%p l4offset=%d skb->len=%d skb_mac_header=%p",skb->protocol,skb->head,skb->data,skb_network_header(skb),skb->network_header,skb_transport_header(skb),skb->transport_header,skb->len,skb_mac_header(skb));
		IGMP_PARSER("len=%d  data_len=%d mac_len=%d hdr_len=%d",skb->len,skb->data_len,skb->mac_len,skb->hdr_len);
		IGMP_PARSER("In Dev:");
		if(in)
			IGMP_PARSER("name:%s ifidx:%d",in->name,in->ifindex)	;
		IGMP_PARSER("Out Dev:");
		if(out)
			IGMP_PARSER("name:%s ifidx:%d",out->name,out->ifindex);
		IGMP_PARSER("SKB Dev:");
		if(skb->dev)
			IGMP_PARSER("name:%s ifidx:%d",skb->dev->name,skb->dev->ifindex) ;
	}

	if( pkthdr.igmph || pkthdr.icmp6h )
	{
		//IGMP/MLD Control packet
		igmp_spin_lock_bh(igmpSysdb.lock_igmp);
		if(in)
			ret = rtk_igmp_mld_process(&pkthdr,in);
		igmp_spin_unlock_bh(igmpSysdb.lock_igmp);
	}
	else if(pkthdr.iph || pkthdr.ip6h)
	{
		ret = rtk_igmp_data_process(skb,&pkthdr,in,out);
	}

	if(ret == RTK_IGMP_NF_CONTINUE)
	{
		IGMP_WARNING("ret RTK_IGMP_NF_CONTINUE to chain !?");
	}

	return ret;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 1, 0)
unsigned int main_hook_br_preRtLast(void *priv,
			       struct sk_buff *skb,
			       const struct nf_hook_state *state)
{
	int32 ret=NF_ACCEPT;
	IGMP_HOOK_POINT("main_hook_br  NF_BR_PRE_ROUTING NF_BR_PRI_LAST");
	ret=main_hook(NULL,skb,state->in,state->out,NULL);
	return ret;
}

unsigned int main_hook_ip_postRtLast(void *priv,
			       struct sk_buff *skb,
			       const struct nf_hook_state *state)
{
	int32 ret=NF_ACCEPT;
	IGMP_HOOK_POINT("main_hook_ip NF_INET_POST_ROUTING NF_IP_PRI_LAST");
	//in ip post routing inDev:NULL outDev:br_x
	if(state && state->out && skb && skb->dev && state->out->ifindex == skb->dev->ifindex)
		ret = main_hook(NULL,skb,NULL,state->out,NULL);
	return ret;
}

#else
unsigned int main_hook_br_preRtLast(const struct nf_hook_ops *ops,
			       struct sk_buff *skb,
			       const struct net_device *in,
			       const struct net_device *out,
			       int (*okfn)(struct sk_buff *))
{
	int32 ret=NF_ACCEPT;
	IGMP_HOOK_POINT("main_hook_br  NF_BR_PRE_ROUTING NF_BR_PRI_LAST");
	ret = main_hook(ops,skb,in,out,okfn);
	return ret;
}

unsigned int main_hook_ip_postRtLast(const struct nf_hook_ops *ops,
			       struct sk_buff *skb,
			       const struct net_device *in,
			       const struct net_device *out,
			       int (*okfn)(struct sk_buff *))
{
	int32 ret=NF_ACCEPT;
	IGMP_HOOK_POINT("main_hook_ip NF_INET_POST_ROUTING NF_IP_PRI_LAST");
	//in ip post routing inDev:NULL outDev:br_x
	if(out && skb && skb->dev && out->ifindex == skb->dev->ifindex)
		ret = main_hook(ops,skb,in,out,okfn);

	return ret;
}
#endif

static struct nf_hook_ops hook_ops[] __read_mostly =
{
	//ip6tables
    {
        .hook 		= main_hook_ip_postRtLast,
        .pf 		= PF_INET6,
        .hooknum 	= NF_INET_POST_ROUTING,
        .priority 	= NF_IP_PRI_LAST,
    },

	//iptables
    {
        .hook 		= main_hook_ip_postRtLast,
        .pf 		= PF_INET,
        .hooknum 	= NF_INET_POST_ROUTING,
        .priority 	= NF_IP_PRI_LAST,
    },

	// ebtables
	{
		.hook		= main_hook_br_preRtLast,
		.pf			= PF_BRIDGE,
		.hooknum	= NF_BR_PRE_ROUTING,
		.priority	= NF_BR_PRI_BRNF-1,
	},



};


#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 12, 0)
static int __net_init igmp_nf_register(struct net *net)
{
	return nf_register_net_hooks(net, hook_ops,ARRAY_SIZE(hook_ops));
}

static void __net_exit igmp_nf_unregister(struct net *net)
{
	nf_unregister_net_hooks(net, hook_ops, ARRAY_SIZE(hook_ops));
}

static struct pernet_operations igmp_net_ops = {
	.init = igmp_nf_register,
	.exit = igmp_nf_unregister,
};

#else
#endif

static int __init rtk_igmp_moudle_init(void)
{

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 12, 0)
	register_pernet_subsys(&igmp_net_ops);
#else
    if (nf_register_hooks(hook_ops, ARRAY_SIZE(hook_ops)) < 0)
	{
        printk("nf_register_hook failed\n");
    }
#endif
	memset(&igmpSysdb,0,sizeof(igmpSysdb));
	igmpSysdb.igmp_debug_level = RTK_IGMP_DEBUG_LEVEL_WARNING ;
	igmpSysdb.regMdbCb=1;
#if 1
	igmpSysdb.hwCbMode=RTK_IGMP_CONFIG_BY_IFIDX;
#else
	igmpSysdb.hwCbMode=RTK_IGMP_CONFIG_BY_PORTMASK;
#endif

	//global parameter init
	rtk_igmp_init();

	//proc init
	rtk_igmp_proc_init();

	//rt api init
#if defined(CONFIG_COMMON_RT_API)
	rtk_rt_igmpHook_mapper_api_init();
#endif

	printk(KERN_INFO "multicast_moudle_init \n");
	return 0;
}

static void __exit rtk_igmp_module_exit(void)
{

	rtk_igmp_proc_exit();
	rtk_igmp_exit();

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 12, 0)
	unregister_pernet_subsys(&igmp_net_ops);
#else
	nf_unregister_hooks(hook_ops, ARRAY_SIZE(hook_ops));
#endif
	printk(KERN_INFO "multicast_module_exit\n");
}

//MODULE_LICENSE("Realtek 2019 IGMP Snooping");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("IGMP/MLD for layer2 Snooping Module");
MODULE_AUTHOR("Boyce <boyceli@realtek.com>");



module_init(rtk_igmp_moudle_init);
module_exit(rtk_igmp_module_exit);
