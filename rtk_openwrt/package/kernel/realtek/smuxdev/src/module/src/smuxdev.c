
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
 *
 */

#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/if.h>
#include <linux/etherdevice.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <linux/if_ether.h>
#include <linux/if_pppox.h>
#include <linux/inet.h>
#include <net/sock.h>
#include "rtk_smux.h"
#include "smuxdev.h"
#include <linux/if_vlan.h>
#include <linux/proc_fs.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/ppp_defs.h>
#include <net/ip.h>
#include <net/ipv6.h>
#include <linux/version.h>
#ifdef CONFIG_COMMON_RT_API
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
#include "rt_type.h"
#include "rt_acl_ext.h"
#include "rt_stat.h"
#include "rt_stat_ext.h"
#include "rtk/l2.h"
#include "rt_l2.h"
#include "common/error.h"
#endif
#endif
#ifndef DEFAULT_FID
#define DEFAULT_FID 0
#endif
#if defined(CONFIG_RTL8686NIC) && defined(CONFIG_RTL9607C_SERIES)
#include "re8686_rtl9607c.h"
#endif
#include <common/error.h>
#define SMUX_TEST 0
#if SMUX_TEST
#define TEST_MAJIC 0xbeef9607
struct test_cb {
	uint32_t key;
	void *ptr;
};
static void* build_ioctl_test(struct sk_buff *skb, struct smux_ioctl_test_s *test);
#endif

#if 0 //defined(CONFIG_LUNA)
extern void (*p_rtk_ppp_stat_set)(struct net_device *dev, struct rtnl_link_stats64 *stats64);
#endif

#ifdef CONFIG_COMMON_RT_API
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
static int smux_set_hw_vlan_filter(int method, struct smux_hw_vlan_filter_para_t *hw_vlan_filter_p);
static void smux_add_hw_vlan_filter_by_rules(struct smux_realdev_cb_t  *realdev_cb, int depth, struct smux_rule_t *rule);
static void smux_del_hw_vlan_filter_by_rules(struct smux_realdev_cb_t  *realdev_cb, int depth, struct smux_rule_t *rule);
static void smux_disable_hw_vlan_filter_by_if_delete(struct smux_realdev_cb_t  *realdev_cb);
extern int rtk_dev_update_stats(struct net_device *dev, struct rtnl_link_stats64 *stats);
#endif
#endif
extern void rtk_ppp_stat_set(struct net_device *dev, struct rtnl_link_stats64 *stats64);

#define MODULE_NAME "smuxdev"
#define MIN(a,b) ((a>b)?b:a)
static DEFINE_SPINLOCK(smux_lock);
static const struct net_device_ops smuxdev_netdev_ops;
static unsigned long smux_debug=0;

typedef struct smux_configure_t smux_config;
smux_config smux_configure;
const struct smux_debug_entry debug_table[];
struct smux_pkt_info_t;

#define SMUX_DBG(debug_index, info, fmt, ...)\
do {\
	if(smux_debug&debug_table[debug_index].debug_level && \
		(debug_table[debug_index].force_print || \
		(!smux_configure.trace_filter || (!info || ((struct smux_pkt_info_t*)info)->filter_pkt))) \
	){\
		printk("\033[1;40;34m[SMUX][%s]:" fmt "\033[0m\n", debug_table[debug_index].debug_level_str, ##__VA_ARGS__);\
	}\
} while(0);

#define SMUX_PKT_DUMP_SKB(debug_index, info, skb, len)\
do {\
	if(smux_debug&debug_table[debug_index].debug_level && \
		(debug_table[debug_index].force_print || \
		(!smux_configure.trace_filter || (!info || ((struct smux_pkt_info_t*)info)->filter_pkt))) \
	){\
		printk("[SMUX][%s]: SKB(%p), DEV(%s)\n", debug_table[debug_index].debug_level_str, skb, ((skb->dev)?skb->dev->name:"NULL"));\
		print_hex_dump(KERN_WARNING, "", DUMP_PREFIX_ADDRESS, 16, 1, (char *)skb_mac_header(skb), len, false);\
	}\
} while(0);

#define SMUX_PKT_DUMP_DATA(debug_index, info, data, len)\
do {\
	if(smux_debug&debug_table[debug_index].debug_level && \
		(debug_table[debug_index].force_print || \
		(!smux_configure.trace_filter || (!info || ((struct smux_pkt_info_t*)info)->filter_pkt))) \
	){\
		printk("[SMUX][%s]:\n", debug_table[debug_index].debug_level_str);\
		print_hex_dump(KERN_WARNING, "", DUMP_PREFIX_ADDRESS, 16, 1, (char *)data, len, false);\
	}\
} while(0);

#define SMUX_MAX_DSCP_NUM	64

#define BITWISE_NUM 8

static inline u32 smux_priv_flags_get(const struct net_device *dev, u32 flags_mask) {
	u32 ret = 0;

	if (dev)
		#ifdef RTK_NETDEV_PRIV_FLAGS
			#define PRIV_RSMUX       RTK_IFF_RSMUX
			#define PRIV_OSMUX       RTK_IFF_OSMUX
			#define PRIV_VSMUX       RTK_IFF_VSMUX
			#define PRIV_DOMAIN_ELAN RTK_IFF_DOMAIN_ELAN
			#define PRIV_DOMAIN_WAN  RTK_IFF_DOMAIN_WAN
			#define PRIV_DOMAIN_WLAN RTK_IFF_DOMAIN_WLAN
		ret = rtk_netdev_get_flags(dev) & flags_mask;
		#else
			#define PRIV_RSMUX       IFF_RSMUX
			#define PRIV_OSMUX       IFF_OSMUX
			#define PRIV_VSMUX       IFF_VSMUX
			#define PRIV_DOMAIN_ELAN IFF_DOMAIN_ELAN
			#define PRIV_DOMAIN_WAN  IFF_DOMAIN_WAN
			#define PRIV_DOMAIN_WLAN IFF_DOMAIN_WLAN
		ret = dev->priv_flags & flags_mask;
		#endif
	return ret;
}

static inline void smux_priv_flags_mask_set(struct net_device *dev, u32 flags_mask) {
	if (dev)
	{
		#ifdef RTK_NETDEV_PRIV_FLAGS
		rtk_netdev_set_flags(dev, rtk_netdev_get_flags(dev) | flags_mask);
		#else
		dev->priv_flags |= flags_mask;
		#endif
	}
}

static inline void smux_priv_flags_mask_clear(struct net_device *dev, u32 flags_mask) {
	if (dev)
	{
		#ifdef RTK_NETDEV_PRIV_FLAGS
		rtk_netdev_set_flags(dev, rtk_netdev_get_flags(dev) & ~flags_mask);
		#else
		dev->priv_flags &= ~flags_mask;
		#endif
	}
}

struct smux_dscp_mapping_entry_t dscp_mapping_table_default[SMUX_MAX_DSCP_NUM] = {
        [46] = {1, "EF", 0xb8, {0x8100,0xa000}},
        [0 ] = {1, "Best Effort", 0x0, {0x8100,0x0000}},
        [10] = {1, "AF11", 0x28, {0x8100,0x2000}},
        [12] = {1, "AF12", 0x30, {0x8100,0x2000}},
        [14] = {1, "AF13", 0x38, {0x8100,0x2000}},
        [18] = {1, "AF21", 0x48, {0x8100,0xa000}},
        [20] = {1, "AF22", 0x50, {0x8100,0xa000}},
        [22] = {1, "AF23", 0x58, {0x8100,0xa000}},
        [26] = {1, "AF31", 0x68, {0x8100,0x6000}},
        [28] = {1, "AF32", 0x70, {0x8100,0x6000}},
        [30] = {1, "AF33", 0x78, {0x8100,0x6000}},
        [34] = {1, "AF41", 0x88, {0x8100,0x8000}},
        [36] = {1, "AF42", 0x90, {0x8100,0x8000}},
        [38] = {1, "AF43", 0x98, {0x8100,0x8000}},
        [8 ] = {1, "CS1", 0x20, {0x8100,0x2000}},
        [16] = {1, "CS2", 0x40, {0x8100,0xa000}},
        [24] = {1, "CS3", 0x60, {0x8100,0x6000}},
        [32] = {1, "CS4", 0x80, {0x8100,0x8000}},
        [40] = {1, "CS5", 0xa0, {0x8100,0xa000}},
        [48] = {1, "CS6", 0xc0, {0x8100,0xc000}},
        [56] = {1, "CS7", 0xe0, {0x8100,0xe000}}
};

const struct smux_debug_entry debug_table[SMUX_DEBUG_INDEX_MAX] = {
    [SMUX_DEBUG_INDEX_RX]={SMUX_DEBUG_LEVEL_RX, "RX", 0},
	[SMUX_DEBUG_INDEX_RX_PACKET]={SMUX_DEBUG_LEVEL_RX_PACKET, "RXPKT", 0},
    [SMUX_DEBUG_INDEX_TX]={SMUX_DEBUG_LEVEL_TX, "TX", 0},
    [SMUX_DEBUG_INDEX_TX_PACKET]={SMUX_DEBUG_LEVEL_TX_PACKET, "TXPKT", 0},
    [SMUX_DEBUG_INDEX_RXMC]={SMUX_DEBUG_LEVEL_RXMC, "RXMC", 0},
    [SMUX_DEBUG_INDEX_RXMC_PACKET]={SMUX_DEBUG_LEVEL_RXMC_PACKET, "RXMCPKT", 0},
    [SMUX_DEBUG_INDEX_IOCTRL]={SMUX_DEBUG_LEVEL_IOCTRL, "IOCTRL", 1},
    [SMUX_DEBUG_INDEX_MISC]={SMUX_DEBUG_LEVEL_MISC, "MISC", 1},
    [SMUX_DEBUG_INDEX_MULTICAST]={SMUX_DEBUG_LEVEL_MULTICAST, "MULTICAST", 0},
	[SMUX_DEBUG_INDEX_ERROR]={SMUX_DEBUG_LEVEL_ERROR, "ERROR", 1},
};

const char *dir_str[] = {
	[SMUX_DIR_RX] = "RX",
	[SMUX_DIR_TX] = "TX",
	[SMUX_DIR_RXMC] = "RXMC",
	[SMUX_DIR_MAX] = NULL,
};

const char *target_str[] = {
	[SMUX_TARGET_CONTINUE] = "CONTINUE",
	[SMUX_TARGET_ACCEPT] = "ACCEPT",
	[SMUX_TARGET_DROP] = "DROP",
	[SMUX_TARGET_END] = NULL,
};

const char *action_str[] = {
    [SMUX_ACT_SET_ETHTYPE]="set-ethertype",
    [SMUX_ACT_POP_TAG]="pop-tag",
    [SMUX_ACT_PUSH_TAG]="push-tag",
	[SMUX_ACT_DUP_FORWARD]="duplicate-forward",
    [SMUX_ACT_SET_RXIF]="set-rxif",
    [SMUX_ACT_SET_TPID]="set-tpid",
    [SMUX_ACT_SET_VID]="set-vid",
    [SMUX_ACT_SET_PRIORITY]="set-priority",
    [SMUX_ACT_SET_CFI]="set-cfi",
    [SMUX_ACT_SET_SKB_MARK]="set-skb-mark",
#ifdef CONFIG_RTK_SKB_MARK2
    [SMUX_ACT_SET_SKB_MARK2]="set-skb-mark2",
#endif
    [SMUX_ACT_SET_DSCP]="set-dscp",
    [SMUX_ACT_COPY_8021Q]="copy-8021q",
    [SMUX_ACT_COPY_TPID]="copy-tpid",
    [SMUX_ACT_COPY_VID]="copy-vid",
    [SMUX_ACT_COPY_PRIORITY]="copy-priority",
    [SMUX_ACT_COPY_CFI]="copy-cfi",
    [SMUX_ACT_MAP_DSCP_TO_PRIORITY]="map-dscp-to-pbit",
    [SMUX_ACT_SET_TARGET]="target",
    [SMUX_ACT_REVERT_VLAN]="revert-vlan",
};

const char *filter_str[] = {
    [SMUX_FLT_ETHTYPE]="filter-ethertype",
	[SMUX_FLT_SIP]="filter-sip",
	[SMUX_FLT_DIP]="filter-dip",
	[SMUX_FLT_SIP_RANGE]="filter-sip-range",
	[SMUX_FLT_DIP_RANGE]="filter-dip-range",
	[SMUX_FLT_IPL4PROTO]="filter-ipl4proto",
	//[SMUX_FLT_IPDSCP]="SMUX_FLT_IPDSCP",
	[SMUX_FLT_VTAGS]="filter-vtags",
	[SMUX_FLT_ORG_VTAGS]="filter-org-vtags",
	[SMUX_FLT_RXIF_NAME]="filter-rxif",
	[SMUX_FLT_TXIF_NAME]="filter-txif",
	[SMUX_FLT_SKB_MARK]="filter-skb-mark",
#ifdef CONFIG_RTK_SKB_MARK2
	[SMUX_FLT_SKB_MARK2]="filter-skb-mark2",
#endif
	[SMUX_FLT_UNI_MAC]="filter-unicast-mac",
	[SMUX_FLT_DMAC] = "filter-dmac",
	[SMUX_FLT_SMAC] = "filter-smac",
	[SMUX_FLT_MULTICAST]="filter-multicast",
	[SMUX_FLT_DSCP_RANGE]="filter-dscp-range",
	/*for N:1 vlan translate, downstream eggress need to set corressponding vlan by mac for PPTP. */
	[SMUX_FLT_UNI_DMAC_OUTERMOST_VLAN] = "filter-uni-dmac-outermost-vlan",
};

const char *cmode_str[] = {
	[SMUX_CARRIER_AUTO] = "Auto",
	[SMUX_CARRIER_MANUAL] = "Manual",
	[SMUX_CARRIER_NONE] = "None"
};

struct smux_pkt_info_t {
	struct sk_buff *skb;
	struct smux_vlan_tag_t *vtags;
	struct smux_vlan_tag_t *org_vtags;
	struct iphdr *iphdr; 
	struct ipv6hdr *ipv6hdr; 
	struct net_device *rxdev;
	int org_vtag_count;
	u32 debug_level_idx;
	u32 is_multicast;
	u32 target_jumped;
	u32 target;
	u16 ethertype;
	u8 vtag_count;	
	enum smux_direction_e dir;
	u8 filter_pkt;
#ifdef CONFIG_COMMON_RT_API
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
	struct smux_vlan_tag_t vtags_inquire_by_lut; //only support cvid now
#endif
#endif
};

typedef int smux_filter_func_t(struct smux_rule_t *,struct smux_pkt_info_t *);
typedef struct sk_buff *smux_action_func_t(struct smux_rule_action_t *, struct smux_pkt_info_t *, struct smux_table_t *);
typedef int smux_dump_filter_func_t(struct smux_rule_filter_t *);
typedef int smux_dump_action_func_t(struct smux_rule_action_t *);

static struct smux_realdev_cb_t *smux_realdev_cb_get(struct net_device *dev);
/* data path */
//static DEFINE_PER_CPU(struct smux_pkt_info_t, pkt_info);
static int smux_build_cb(struct sk_buff *skb, struct smux_realdev_cb_t *dev_cb, struct smux_pkt_info_t *info);
static int _process_packet(enum smux_direction_e dir, struct sk_buff *skb, struct smux_realdev_cb_t *dev_cb, struct smux_pkt_info_t *info);
static int smux_rule_del(const char *realname, enum smux_direction_e dir, int depth, int method, u32 *id, void *value);
static int mapping_dscp_to_priority(struct smux_rule_action_t *act, struct smux_pkt_info_t *info, int idx);
static int smux_dev_change_mtu(struct net_device *dev, int new_mtu)
{
	/* TODO: gotta make sure the underlying layer can handle it,
	 * maybe an IFF_VLAN_CAPABLE flag for devices?
	 */
	 
	SMUX_DBG(SMUX_DEBUG_INDEX_MISC, NULL, "%s", __func__); 
	if (smux_dev_priv(dev)->real_dev->mtu < new_mtu)
		return -ERANGE;

	dev->mtu = new_mtu;

	return 0;
}

static int smux_update_pkt_info(struct sk_buff *skb, struct smux_pkt_info_t *info);
static inline struct sk_buff *smux_unshareSKB(struct smux_pkt_info_t *info, struct sk_buff *skb)
{
    if(skb_cloned(skb))
    {
		struct sk_buff *nskb = skb_copy(skb, GFP_ATOMIC);

		/* Free our shared copy */
		if (likely(nskb))
			consume_skb(skb);
		else
			kfree_skb(skb);
		
		SMUX_DBG(info->debug_level_idx, info, "[%s] new SKB %p, old SKB %p\n",__func__, nskb, skb);	
		
		skb = nskb;
		
		smux_update_pkt_info(skb, info);
    }

    return skb;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0)
static inline bool ether_addr_equal_masked(const u8 *addr1, const u8 *addr2,
					   const u8 *mask)
{
	int i;

	for (i = 0; i < ETH_ALEN; i++) {
		if ((addr1[i] ^ addr2[i]) & mask[i])
			return false;
	}

	return true;
}
#endif

#if 0 
static int smux_dev_hard_header(struct sk_buff *skb, struct net_device *dev,
				unsigned short type,
				const void *daddr, const void *saddr,
				unsigned int len)
{
	struct smux_dev_priv *priv = smux_dev_priv(dev);

	int rc;
	SMUX_DBG("%s", __func__); 
	
	SMUX_DBG("skb:%p,%p,%p,%p,len=%d len=%d type=%x", skb->head,skb->data,skb->tail,skb->end,skb->len,len,type); 
	if (daddr)
		print_hex_dump(KERN_WARNING, "daddr", DUMP_PREFIX_ADDRESS, 16, 1, daddr, 6, false);
	if (saddr)
		print_hex_dump(KERN_WARNING, "saddr", DUMP_PREFIX_ADDRESS, 16, 1, saddr, 6, false);
	print_hex_dump(KERN_WARNING, "", DUMP_PREFIX_ADDRESS, 16, 1, skb->data, 64, false);

	/* Before delegating work to the lower layer, enter our MAC-address */
	if (saddr == NULL)
		saddr = dev->dev_addr;

	/* Now make the underlying real hard header */
	dev = priv->real_dev;
	rc = dev_hard_header(skb, dev, type, daddr, saddr, len /*+ vhdrlen*/);
	//if (rc > 0)
	//	rc += vhdrlen;
	return rc;
}


static int smux_dev_rebuild_header(struct sk_buff *skb)
{
	SMUX_DBG("%s", __func__);
	
	return 0;
}

static const struct header_ops smux_dev_header_ops = {
	.create	 = smux_dev_hard_header,
	.rebuild = smux_dev_rebuild_header,
	.parse	 = eth_header_parse,
};
#endif 

static int smux_dev_init(struct net_device *dev)
{
	struct net_device *real_dev = smux_dev_priv(dev)->real_dev;

	SMUX_DBG(SMUX_DEBUG_INDEX_MISC, NULL, "%s", __func__);
	
	netif_carrier_off(dev);

	/* IFF_BROADCAST|IFF_MULTICAST; ??? */
	dev->flags  = real_dev->flags & ~(IFF_UP | IFF_PROMISC | IFF_ALLMULTI |
					  IFF_MASTER | IFF_SLAVE);
	//dev->iflink = real_dev->ifindex;
	dev->state  = (real_dev->state & ((1<<__LINK_STATE_NOCARRIER) |
					  (1<<__LINK_STATE_DORMANT))) |
		      (1<<__LINK_STATE_PRESENT);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 5, 0)
	dev->hw_features = NETIF_F_ALL_CSUM | NETIF_F_SG |
			   NETIF_F_FRAGLIST | NETIF_F_ALL_TSO |
			   NETIF_F_HIGHDMA | NETIF_F_SCTP_CSUM |
			   NETIF_F_ALL_FCOE;
#else
	dev->hw_features = NETIF_F_CSUM_MASK | NETIF_F_SG |
			   NETIF_F_FRAGLIST | NETIF_F_ALL_TSO |
			   NETIF_F_HIGHDMA | NETIF_F_SCTP_CRC |
			   NETIF_F_ALL_FCOE;
#endif

	dev->features |= real_dev->vlan_features | NETIF_F_LLTX;
	dev->gso_max_size = real_dev->gso_max_size;
	if (dev->features & NETIF_F_VLAN_FEATURES)
		netdev_warn(real_dev, "VLAN features are set incorrectly.  Q-in-Q configurations may not work correctly.\n");
	dev->vlan_features |= real_dev->vlan_features;

	/* ipv6 shared card related stuff */
	dev->dev_id = real_dev->dev_id;

	if (is_zero_ether_addr(dev->dev_addr))
		eth_hw_addr_inherit(dev, real_dev);
	if (is_zero_ether_addr(dev->broadcast))
		memcpy(dev->broadcast, real_dev->broadcast, dev->addr_len);

	dev->needed_headroom = real_dev->needed_headroom;
	//if (vlan_hw_offload_capable(real_dev->features,
	//			    vlan_dev_priv(dev)->vlan_proto)) {
	//	dev->header_ops      = &vlan_passthru_header_ops;
	//	dev->hard_header_len = real_dev->hard_header_len;
	//} else {
		//dev->header_ops      = &smux_dev_header_ops;
		//dev->hard_header_len = real_dev->hard_header_len + 4;
		dev->hard_header_len = real_dev->hard_header_len;
		
	//}

	dev->netdev_ops = &smuxdev_netdev_ops;
	
	return 0;
}

static void smux_dev_uninit(struct net_device *dev)
{
	SMUX_DBG(SMUX_DEBUG_INDEX_MISC, NULL, "%s", __func__);
}

static int smux_dev_open(struct net_device *dev)
{
	SMUX_DBG(SMUX_DEBUG_INDEX_MISC, NULL, "%s", __func__);
	return 0;
}

static int smux_dev_stop(struct net_device *dev)
{
	SMUX_DBG(SMUX_DEBUG_INDEX_MISC, NULL, "%s", __func__);
	memset(&(smux_dev_priv(dev)->net_stats), 0 ,sizeof(smux_dev_priv(dev)->net_stats));
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
#ifdef CONFIG_COMMON_RT_API
	{
		rt_stat_netif_mib_t netifMibCnt = {0};
		if(rt_stat_netifMib_get(dev->name, &netifMibCnt) == RT_ERR_OK)
		{
			rt_stat_netifMib_reset(dev->name);
		}
	}
#endif
#endif
	return 0;
}

static struct sk_buff * smux_retore_skb_vlan(struct sk_buff *skb, struct smux_pkt_info_t *info)
{
	int headerLen, v_idx;

	SMUX_DBG(info->debug_level_idx, info, "%s", __func__);
	if(skb->reserv_vlan_count > 0 && skb->reserv_vlan_count <= SMUX_MAX_TAGS)
	{
		if(info->vtag_count < skb->reserv_vlan_count)
		{
			headerLen = (skb->reserv_vlan_count - info->vtag_count) * VLAN_HLEN;
			SMUX_DBG(info->debug_level_idx, info, "restore VLAN count %d", skb->reserv_vlan_count);
			if(skb_cow_head(skb, headerLen) < 0){
				SMUX_DBG(info->debug_level_idx, info, "%s Error memory for resize %d", __func__, headerLen);
				return skb;
			}
			skb_push(skb, headerLen);
			memmove(skb->data, skb->data + headerLen, ETH_ALEN*2);
			skb->mac_header -= headerLen;
			//skb->network_header -= headerLen;
			info->vtags = (struct smux_vlan_tag_t *)(skb_mac_header(skb) + ETH_ALEN*2);
			info->vtag_count = skb->reserv_vlan_count;
#if 0
			if(info->iphdr != NULL){
				//info->iphdr = (struct iphdr *)(((char*)info->iphdr) + headerLen);
				SMUX_DBG(info->debug_level_idx, info, "%s  iphdr", __func__);
				SMUX_PKT_DUMP_DATA(info->debug_level_idx+1, info, info->iphdr, 20);
			}
			if(info->ipv6hdr != NULL){
				//info->ipv6hdr = (struct ipv6hdr *)(((char*)info->ipv6hdr) + headerLen);
				SMUX_DBG(info->debug_level_idx, info, "%s  ipv6hdr", __func__);
				SMUX_PKT_DUMP_DATA(info->debug_level_idx+1, info, info->ipv6hdr, 40);
			}
#endif
			v_idx = 0;
		}
		else {
			headerLen = skb->reserv_vlan_count * VLAN_HLEN;
			v_idx = info->vtag_count - skb->reserv_vlan_count;
		}
		
		memcpy(info->vtags+v_idx, 
			skb->reserv_vlan_header, 
			headerLen);

		skb->reserv_vlan_count = 0;
		skb->protocol = info->vtags[0].vlan_proto;
		info->skb = skb;
		SMUX_PKT_DUMP_SKB(info->debug_level_idx+1, info, skb, 64);
	}
	return skb;
}

static struct sk_buff * smux_save_skb_vlan(struct sk_buff *skb, struct smux_pkt_info_t *info)
{
	int headerLen;
	
	SMUX_DBG(info->debug_level_idx, info, "%s", __func__);
	if(info->vtag_count > 0)
	{
		SMUX_PKT_DUMP_SKB(info->debug_level_idx+1, info, skb, 64);
		
		info->skb = smux_unshareSKB(info, skb);
		skb = info->skb;
		
		if(skb->dev->priv_flags & IFF_802_1Q_VLAN){
			headerLen = VLAN_HLEN;
			SMUX_DBG(info->debug_level_idx, info, "save VLAN count %d", 1);
			
			info->vtag_count -= 1;
			if(info->vtag_count <= 0){
				info->vtag_count = 0;
				info->vtags = NULL;
				skb->protocol = htons(info->ethertype);
			}else{
				info->vtags += headerLen;
				skb->protocol = info->vtags[0].vlan_proto;
			}
		}
		else {
			skb->reserv_vlan_count = info->vtag_count;
			SMUX_DBG(info->debug_level_idx, info, "save VLAN count %d", skb->reserv_vlan_count);
			headerLen = skb->reserv_vlan_count * VLAN_HLEN;
			memcpy(skb->reserv_vlan_header,
				info->vtags,
				headerLen);
				
			skb->protocol = htons(info->ethertype);
			info->vtag_count = 0;
			info->vtags = NULL;
		}
		skb_pull_rcsum(skb, headerLen);
		memmove(skb->data - ETH_HLEN,
			skb->data - ETH_HLEN - headerLen,
			2 * ETH_ALEN);
		skb->mac_header += headerLen;
		//skb->network_header += headerLen;
#if 0
		if(info->iphdr != NULL){
			SMUX_DBG(info->debug_level_idx, info, "%s  iphdr", __func__);
			SMUX_PKT_DUMP_DATA(info->debug_level_idx+1, info, info->iphdr, 20);
		}
		if(info->ipv6hdr != NULL){
			SMUX_DBG(info->debug_level_idx, info, "%s  ipv6hdr", __func__);
			SMUX_PKT_DUMP_DATA(info->debug_level_idx+1, info, info->ipv6hdr, 40);
		} 
#endif
		SMUX_PKT_DUMP_SKB(info->debug_level_idx+1, info, skb, 64);
	}
	return skb;
}

static netdev_tx_t smux_dev_hard_start_xmit(struct sk_buff *skb, struct net_device *dev) 
{	
    struct smux_realdev_cb_t *realdev_cb, *realdev_cb1;
    struct smux_pkt_info_t *info = NULL, pkt_info;
    struct smux_dev_priv  *priv = smux_dev_priv(dev);
	struct net_device *skb_dev = skb->dev;
    struct net_device *real_dev  = priv->real_dev;

	if (real_dev==NULL || NULL==(realdev_cb = smux_realdev_cb_get(real_dev)))
	{
		SMUX_DBG(SMUX_DEBUG_INDEX_ERROR, NULL, "[SMUX][TX] Cannot found realdev for vif(%s), SKB (%p)\n", ((real_dev)?real_dev->name:""), skb);
		goto drop;
	}
	
	info = &pkt_info;
	memset(info, 0, sizeof(struct smux_pkt_info_t));
	info->target_jumped = 0;
	info->target = SMUX_TARGET_CONTINUE;
	info->skb = skb;
	info->debug_level_idx = SMUX_DEBUG_INDEX_TX;
	smux_build_cb(skb, realdev_cb, info);
	
	SMUX_DBG(SMUX_DEBUG_INDEX_TX, info, "==============[Tx packet boundary start]==============");
	SMUX_DBG(SMUX_DEBUG_INDEX_TX, info, "skb:%p,%p,%p,%p,len=%d,skb_dev=%s,priv_flags=0x%x", skb->head,skb->data,skb->tail,skb->end,skb->len,skb_dev->name,real_dev->priv_flags); 
	SMUX_PKT_DUMP_SKB(SMUX_DEBUG_INDEX_TX_PACKET, info, skb, 64);	
	
	if(skb->reserv_vlan_count > 0 && 
		skb->reserv_vlan_count <= SMUX_MAX_TAGS)
	{
		skb = smux_retore_skb_vlan(skb, info);
		info->skb = skb;
	}

	//first filter
	if ((smux_priv_flags_get(skb_dev, PRIV_RSMUX|PRIV_VSMUX)==(PRIV_RSMUX|PRIV_VSMUX))
		&& (realdev_cb1 = priv->real_dev_priv))
	{
		_process_packet(SMUX_DIR_TX, skb, realdev_cb1, info);
		skb = info->skb;
		SMUX_PKT_DUMP_SKB(SMUX_DEBUG_INDEX_TX_PACKET, info, skb, 64);
		//if no any rule to change TARGET, use the default policy rule of the chain 
		if(info->target_jumped == 0) 
			info->target = realdev_cb->default_policy[SMUX_DIR_TX];
		
		if(unlikely(info->target == SMUX_TARGET_DROP))
		{
			SMUX_DBG(SMUX_DEBUG_INDEX_TX, info, "jump to target DROP !");
			goto drop;
		}
		
		info->target_jumped = 0;
		info->target = SMUX_TARGET_CONTINUE;
		smux_build_cb(skb, realdev_cb, info);
	}
	
	_process_packet(SMUX_DIR_TX, skb, realdev_cb, info);
	skb = info->skb;
	//if no any rule to change TARGET, use the default policy rule of the chain 
	if(info->target_jumped == 0) 
		info->target = realdev_cb->default_policy[SMUX_DIR_TX];
	
	skb->dev = real_dev;
	SMUX_DBG(SMUX_DEBUG_INDEX_TX, info, "%s(%d) real_dev is %s",__func__,__LINE__,skb->dev->name);
	SMUX_PKT_DUMP_SKB(SMUX_DEBUG_INDEX_TX_PACKET, info, skb, 64);
	
	if(unlikely(info->target == SMUX_TARGET_DROP))
	{
		SMUX_DBG(SMUX_DEBUG_INDEX_TX, info, "jump to target DROP !");
		goto drop;
	}
	else
	{
		if(likely(info->target == SMUX_TARGET_CONTINUE))
		{
			SMUX_DBG(SMUX_DEBUG_INDEX_TX, info, "jump to target CONTINUE !");
		}
		else if(likely(info->target == SMUX_TARGET_ACCEPT))
		{
			SMUX_DBG(SMUX_DEBUG_INDEX_TX, info, "jump to target ACCEPT !");
		}
		
		smux_dev_priv(dev)->net_stats.tx_packets++; 
		smux_dev_priv(dev)->net_stats.tx_bytes += skb->len;
		switch (skb->pkt_type) 
		{
			case PACKET_BROADCAST:
				break;

			case PACKET_MULTICAST:
				smux_dev_priv(dev)->net_stats.multicast++;
				break;
		}
		
#if SMUX_TEST
		do {
			struct test_cb *tcb;
			tcb = (struct test_cb *)skb->cb;
			if (tcb->key == TEST_MAJIC)
				build_ioctl_test(skb, tcb->ptr);
			else
				dev_queue_xmit(skb); 
		} while (0);
#else
		dev_queue_xmit(skb); 
#endif 
	}
	
	SMUX_DBG(SMUX_DEBUG_INDEX_TX, info, "==============[Tx packet boundary end]==============");
	return NETDEV_TX_OK;
	
drop:
#if SMUX_TEST
	do {
		struct test_cb *tcb;
		tcb = (struct test_cb *)skb->cb;
		if (tcb->key == TEST_MAJIC)
			;
		else
			dev_kfree_skb(skb); 
	} while (0);
#else
	dev_kfree_skb(skb);
#endif
	smux_dev_priv(dev)->net_stats.tx_dropped++; 
	
	return NETDEV_TX_OK;
}

#if 0
static int smux_dev_set_mac_address(struct net_device *dev, void *p) {	
	struct sockaddr *addr = (struct sockaddr *)(p);
	int i, flgs;

	SMUX_DBG(SMUX_DEBUG_INDEX_MISC, NULL, "%s", __func__);
	if (netif_running(dev))
		return -EBUSY;

	memcpy(dev->dev_addr, addr->sa_data, ETH_ALEN);
	
	memset(dev->broadcast, 0xff, ETH_ALEN);
	
	return 0;
}
#endif

static void smux_dev_set_rx_mode(struct net_device *vlan_dev) {
	SMUX_DBG(SMUX_DEBUG_INDEX_MISC, NULL, "%s", __func__);	
}

static int smux_dev_set_features(struct net_device *dev, netdev_features_t features) {
	SMUX_DBG(SMUX_DEBUG_INDEX_MISC, NULL, "%s", __func__);
	return 0;
}

#if 0
static int smux_dev_change_rx_flags(const struct net_device *dev, u32 flags, u32 mask) {
	SMUX_DBG(SMUX_DEBUG_INDEX_MISC, NULL, "%s", __func__);
	return 0;
}
#endif

static int smux_dev_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd) {
	struct net_device *real_dev  = smux_dev_priv(dev)->real_dev;
	const struct net_device_ops *ops = real_dev->netdev_ops;
	int err = 0;
	
	SMUX_DBG(SMUX_DEBUG_INDEX_MISC, NULL, "%s", __func__);
	
	if (ops->ndo_do_ioctl) {
		if (netif_device_present(real_dev))
			err = ops->ndo_do_ioctl(real_dev, ifr, cmd);
		else
			err = -ENODEV;
	}
	return err;
}

static int smux_dev_neigh_setup(struct net_device *dev, struct neigh_parms *pa) {
	
	struct net_device *real_dev  = smux_dev_priv(dev)->real_dev;
	const struct net_device_ops *ops = real_dev->netdev_ops;
	int err = 0;
	
	SMUX_DBG(SMUX_DEBUG_INDEX_MISC, NULL, "%s", __func__);

	if (netif_device_present(real_dev) && ops->ndo_neigh_setup) {
		SMUX_DBG(SMUX_DEBUG_INDEX_MISC, NULL, "%s(%d):fn=%p", __func__,__LINE__,ops->ndo_neigh_setup);
		err = ops->ndo_neigh_setup(real_dev, pa);
	}

	return err;	
}

static void smux_dev_get_stats64(struct net_device *dev, struct rtnl_link_stats64 *stats64) {
	SMUX_DBG(SMUX_DEBUG_INDEX_MISC, NULL, "%s", __func__);

	if (unlikely(stats64 == NULL))
		return;

	stats64->rx_packets 	= smux_dev_priv(dev)->net_stats.rx_packets;
	stats64->rx_bytes   	= smux_dev_priv(dev)->net_stats.rx_bytes;
	stats64->rx_dropped 	= smux_dev_priv(dev)->net_stats.rx_dropped;
	stats64->tx_packets 	= smux_dev_priv(dev)->net_stats.tx_packets;
	stats64->tx_bytes   	= smux_dev_priv(dev)->net_stats.tx_bytes;
	stats64->tx_dropped  	= smux_dev_priv(dev)->net_stats.tx_dropped;
	stats64->multicast  	= smux_dev_priv(dev)->net_stats.multicast;

#if defined(CONFIG_COMMON_RT_API) && defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
	rtk_dev_update_stats(dev, stats64);
#endif
}

static const struct net_device_ops smuxdev_netdev_ops = {
	.ndo_change_mtu		= smux_dev_change_mtu,
	.ndo_init		= smux_dev_init,
	.ndo_uninit		= smux_dev_uninit,
	.ndo_open		= smux_dev_open,
	.ndo_stop		= smux_dev_stop,
	.ndo_start_xmit =  smux_dev_hard_start_xmit,
	.ndo_validate_addr	= eth_validate_addr,
	//.ndo_set_mac_address	= smux_dev_set_mac_address,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_set_rx_mode	= smux_dev_set_rx_mode,
	.ndo_set_features	= smux_dev_set_features,
	//.ndo_change_rx_flags	= smux_dev_change_rx_flags,
	.ndo_do_ioctl		= smux_dev_ioctl,
	.ndo_neigh_setup	= smux_dev_neigh_setup,
	.ndo_get_stats64	= smux_dev_get_stats64,
};

static void smux_dev_free(struct net_device *dev)
{
	//struct vlan_dev_priv *vlan = vlan_dev_priv(dev);
	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s", __func__);
	//free_percpu(vlan->vlan_pcpu_stats);
	//vlan->vlan_pcpu_stats = NULL;
	free_netdev(dev);
}

static void smux_ethtool_get_drvinfo(struct net_device *dev,
				     struct ethtool_drvinfo *info)
{
	SMUX_DBG(SMUX_DEBUG_INDEX_MISC, NULL, "%s", __func__);
	
	strlcpy(info->driver, "RTK SMUX Gen2", sizeof(info->driver));
	strlcpy(info->version, "beta", sizeof(info->version));
	strlcpy(info->fw_version, "N/A", sizeof(info->fw_version));
}
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 6, 0)
static int smux_ethtool_get_settings(struct net_device *dev,
				     struct ethtool_cmd *cmd)
{
	//const struct vlan_dev_priv *vlan = vlan_dev_priv(dev);
	//const struct net_device *real_dev = smux_dev_priv(dev)->real_dev;
	SMUX_DBG(SMUX_DEBUG_INDEX_MISC, NULL, "%s", __func__);
	return __ethtool_get_settings(smux_dev_priv(dev)->real_dev, cmd);
}
#else
static int smux_ethtool_get_link_ksettings(struct net_device *dev,
				     struct ethtool_link_ksettings *link_ksettings)
{
	//const struct vlan_dev_priv *vlan = vlan_dev_priv(dev);
	//const struct net_device *real_dev = smux_dev_priv(dev)->real_dev;
	SMUX_DBG(SMUX_DEBUG_INDEX_MISC, NULL, "%s", __func__);
	return  __ethtool_get_link_ksettings(smux_dev_priv(dev)->real_dev, link_ksettings);
}
#endif

static const struct ethtool_ops smuxdev_ethtool_ops = {
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 6, 0)
	.get_settings	        = smux_ethtool_get_settings,
#else
	.get_link_ksettings = smux_ethtool_get_link_ksettings,
#endif
	.get_drvinfo	        = smux_ethtool_get_drvinfo,
	.get_link		= ethtool_op_get_link,

};

static void smux_dev_setup(struct net_device *dev) {
	
	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s", __func__);
	
	ether_setup(dev);

	//dev->priv_flags		|= IFF_RTK_SMUX_VIRT;
	smux_priv_flags_mask_set(dev,PRIV_VSMUX);
	dev->priv_flags		&= ~IFF_TX_SKB_SHARING;
	netif_keep_dst(dev);
	dev->tx_queue_len	= 0;

	dev->netdev_ops	     = &smuxdev_netdev_ops;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 11, 9)
	dev->destructor		= smux_dev_free;
#else
	dev->priv_destructor = smux_dev_free;
#endif
	dev->ethtool_ops     = &smuxdev_ethtool_ops;

	memset(dev->broadcast, 0, ETH_ALEN);
}


static int __register_smux_dev(struct net_device *dev)
{	
	struct net_device *real_dev = smux_dev_priv(dev)->real_dev;
	int err;
	
	err = register_netdevice(dev);
	if (err < 0)
		goto out;
	
	#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0)
	err = netdev_upper_dev_link(real_dev, dev);
	#else
	err = netdev_upper_dev_link(real_dev, dev, NULL);
	#endif
	
	if (err)
		goto out_unregister_netdev;
	
	/* Account for reference in struct vlan_dev_priv */
	dev_hold(real_dev);
	

	netif_stacked_transfer_operstate(real_dev, dev);
	linkwatch_fire_event(dev); /* _MUST_ call rfc2863_policy() */
	return 0;
	
out_unregister_netdev:
	unregister_netdevice(dev);
out:
	return err;
}

static void __unregister_smux_dev(struct net_device *dev) {
	//struct net_device *dev;
	struct net_device *real_dev = smux_dev_priv(dev)->real_dev;
	
	SMUX_DBG(SMUX_DEBUG_INDEX_MISC, NULL, "%s", __func__);
	
	ASSERT_RTNL();
		
	netdev_upper_dev_unlink(real_dev, dev);
	
	unregister_netdevice(dev);
	
	dev_put(real_dev);	
}

static LIST_HEAD(real_dev_list);

static struct smux_realdev_cb_t *smux_realdev_cb_get(struct net_device *dev)  {
	struct smux_realdev_cb_t  *realdev_cb, *ret = NULL;
	rcu_read_lock();
	list_for_each_entry_rcu(realdev_cb, &real_dev_list, list_real) {
		if (realdev_cb->realdev == dev) {
			ret = realdev_cb;
			break;
		}
	}
	rcu_read_unlock();
	return ret;
}

static struct smux_realdev_cb_t *smux_realdev_cb_get_by_ifname(const char *ifname)  {
	struct smux_realdev_cb_t  *realdev_cb, *ret = NULL;
	
	if(ifname == NULL || *ifname == '\0')
		return NULL;
	rcu_read_lock();
	list_for_each_entry_rcu(realdev_cb, &real_dev_list, list_real) {
		if (!strcmp(realdev_cb->realdev->name, ifname)) {
			ret = realdev_cb;
			break;
		}
	}
	rcu_read_unlock();
	return ret;
}

static struct net_device *get_virt_dev(struct smux_realdev_cb_t *realdev_cb, const char *virt_name) {
	struct smux_dev_priv *priv;
	struct smux_realdev_cb_t  *realdev_cb_priv;
	struct net_device *dev = NULL;
	
	if(realdev_cb != NULL)
	{
		rcu_read_lock();
		list_for_each_entry_rcu(priv, &realdev_cb->list_virt, list_virt) {
			if (!strcmp(virt_name, priv->virt_dev->name)) {
				dev = priv->virt_dev;
				break;
			}			
		}
		rcu_read_unlock();
	}
	if(dev == NULL)
	{
		rcu_read_lock();
		list_for_each_entry_rcu(realdev_cb_priv, &real_dev_list, list_real) {
			if (!strcmp(realdev_cb_priv->realdev->name, virt_name)) {
				dev = realdev_cb_priv->realdev;
			}
			else
			{
				list_for_each_entry_rcu(priv, &realdev_cb_priv->list_virt, list_virt) {
					if (!strcmp(virt_name, priv->virt_dev->name)) {
						dev = priv->virt_dev;
						break;
					}			
				}
			}
			if(dev != NULL) break;
		}
		rcu_read_unlock();
		
		if(dev == NULL)
		{
			dev = __dev_get_by_name(&init_net, virt_name);
		}
	}
	return dev;
}


//static void smux_init_table(struct smux_table_t *tbl) {
//	
//	INIT_LIST_HEAD(&tbl->rule_head);
//	
//	tbl->default_tpi = htons(0x8100);
//	tbl->default_vlan_tci = htons(0x0001); /* pri=0, cfi=0, vid = 1 */
//}

static void smux_init_realdev_cb(struct smux_realdev_cb_t *cb) {
	int dir, idx;
	struct smux_table_t *tbl;
	
	INIT_LIST_HEAD(&cb->list_real);
	INIT_LIST_HEAD(&cb->list_virt);
	
	cb->tpids[0] = htons(0x8100);
	cb->tpids[1] = htons(0x9100);
	cb->tpids[2] = htons(0x88a8);
	
	for (dir=0; dir < SMUX_DIR_MAX; dir++) {
		for (idx=0; idx <= SMUX_MAX_TAGS; idx++) {
			tbl = &cb->tables[dir][idx];
			
			INIT_LIST_HEAD(&tbl->rule_head);
	
			tbl->default_tpid = htons(0x8100);
			tbl->default_vlan_tci = htons(0x0001); /* pri=0, cfi=0, vid = 1 */
		}
	}
}

static int smux_interface_delete(struct net_device *dev);

static void smux_realdev_cb_delete(struct smux_realdev_cb_t  *realdev_cb) {	
	struct smux_dev_priv *priv, *tmp;
	int dir, depth;

	for(dir=SMUX_DIR_RX;dir<SMUX_DIR_MAX;dir++)
		for(depth=0;depth<=SMUX_MAX_TAGS;depth++)
			smux_rule_del(realdev_cb->realdev->name, dir, depth, SMUX_RULE_METHOD_DELETE_ALL, NULL, NULL);

	list_for_each_entry_safe(priv, tmp, &realdev_cb->list_virt, list_virt) {
		//rtnl_lock();		
		smux_interface_delete(priv->virt_dev); // remove every virtual interface 			
		//rtnl_unlock();
	}
	rcu_read_lock();
	list_del_rcu(&realdev_cb->list_real);
	rcu_read_unlock();
	
	synchronize_rcu();
	kfree(realdev_cb);	
}

static int smux_interface_delete(struct net_device *dev)
{
	int dir, depth;
	struct smux_dev_priv  *priv = smux_dev_priv(dev);
	struct smux_realdev_cb_t *realdev_cb;
	
	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s %s", __func__, dev->name);

	if((!smux_priv_flags_get(dev,PRIV_VSMUX)) || priv == NULL)
	{
		return -ENODEV;
	}

	if(smux_priv_flags_get(dev,PRIV_RSMUX))
	{
		realdev_cb = priv->real_dev_priv;
		if(realdev_cb)
		{		
			/*if(!list_empty(&realdev_cb->list_virt))
				return -EBUSY;*/			
			smux_realdev_cb_delete(realdev_cb);
			priv->real_dev_priv = NULL;
			smux_priv_flags_mask_clear(dev,PRIV_RSMUX);
		}
	}
	
	for(dir=SMUX_DIR_RX;dir<SMUX_DIR_MAX;dir++)
		for(depth=0;depth<=SMUX_MAX_TAGS;depth++)
			smux_rule_del(priv->real_dev->name, dir, depth, SMUX_RULE_METHOD_DELETE_IFNAME, NULL, dev->name);
		
	list_del_rcu(&priv->list_virt);
#if 0	
	realdev_cb = smux_realdev_cb_get(priv->real_dev);
	if(realdev_cb && list_empty(&realdev_cb->list_virt))
	{
#ifdef CONFIG_COMMON_RT_API
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
		smux_disable_hw_vlan_filter_by_if_delete(realdev_cb);
#endif
#endif
		smux_realdev_cb_delete(realdev_cb);
		smux_priv_flags_mask_clear(priv->real_dev,PRIV_RSMUX);
	}
#endif
	__unregister_smux_dev(dev);
	
	return 0;
}

static int smux_interface_parameter_set(struct net_device *real_dev, struct smux_interface_s *psmuxif)
{
	struct smux_realdev_cb_t  *realdev_cb = NULL;
	struct smux_dev_priv *priv = NULL;
	int i;

	if(smux_priv_flags_get(real_dev, PRIV_RSMUX))
	realdev_cb = smux_realdev_cb_get(real_dev);
	if (smux_priv_flags_get(real_dev,PRIV_VSMUX))
		priv = smux_dev_priv(real_dev);
	
	if(psmuxif->dev_flags & SMUXCTL_IF_FLAGS_RSMUX)
	{
		SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s Append IFF_RSMUX ..", __func__); 
		if (NULL == realdev_cb) { /* create a new one */
			realdev_cb = kzalloc(sizeof(*realdev_cb), GFP_KERNEL);
			if (!realdev_cb) 
				return -1;
			smux_init_realdev_cb(realdev_cb);
			realdev_cb->realdev = real_dev;			
			smux_priv_flags_mask_set(real_dev,PRIV_RSMUX);
			list_add_rcu(&realdev_cb->list_real, &real_dev_list);
			if(priv) priv->real_dev_priv = realdev_cb;
		}
	}

	if(realdev_cb)
	{
		for(i=SMUX_DIR_RX;i<SMUX_DIR_MAX;i++)
		{
			if(psmuxif->policy[i] < SMUX_TARGET_END)
			{
				realdev_cb->default_policy[i] = psmuxif->policy[i];
			}
		}
		
		if((psmuxif->rx_multi >= SMUX_RX_MULTI_NORMAL && 
			psmuxif->rx_multi < SMUX_RX_MULTI_NONE))
		{
			realdev_cb->rx_multi_mode = psmuxif->rx_multi;
		}
	}
	
	if(priv)
	{
		if((psmuxif->carrier >= SMUX_CARRIER_AUTO && 
			psmuxif->carrier < SMUX_CARRIER_NONE))
		{
			priv->carrier_mode = psmuxif->carrier;
		}
	}
	
	return 0;
}

static int smux_interface_create(struct net_device *real_dev, const char *name, struct smux_interface_s *psmuxif)
{
	struct net_device *new_dev;
	struct smux_dev_priv *smux;
	struct net *net = dev_net(real_dev);
	struct smux_realdev_cb_t  *realdev_cb;
	int err, i;
	
	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s", __func__);
	
	new_dev = alloc_netdev(sizeof(struct smux_dev_priv), name,
			       NET_NAME_UNKNOWN, smux_dev_setup);

	if (new_dev == NULL){
	    SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s Create new_dev Fail!!", __func__);    
		return -ENOBUFS;
	}	

	dev_net_set(new_dev, net);
	/* need 4 bytes for extra VLAN header info,
	 * hope the underlying device can handle it.
	 */
	new_dev->mtu = real_dev->mtu;
	new_dev->priv_flags |= (real_dev->priv_flags & IFF_UNICAST_FLT);

	smux = smux_dev_priv(new_dev);
	smux->real_dev = real_dev;
	smux->virt_dev = new_dev;
	INIT_LIST_HEAD(&smux->list_virt);

	new_dev->features |= (real_dev->features & (NETIF_F_HW_CSUM | NETIF_F_GRO | NETIF_F_SG | NETIF_F_GSO | NETIF_F_TSO | NETIF_F_TSO6));
	if(new_dev->features & NETIF_F_GSO)
	{
		netif_set_gso_max_size(new_dev, real_dev->gso_max_size);
	}
	new_dev->hw_features |=  real_dev->hw_features;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,10,0)
	new_dev->min_mtu = real_dev->min_mtu;
	new_dev->max_mtu = real_dev->max_mtu;
#endif

	//new_dev->rtnl_link_ops = &vlan_link_ops;
	err = __register_smux_dev(new_dev);
	if (err < 0){
	    SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s __register_smux_dev return error!!", __func__);
		goto out_free_newdev;
	}	
	
	if((psmuxif->carrier >= SMUX_CARRIER_AUTO && 
		psmuxif->carrier < SMUX_CARRIER_NONE))
	{
		smux->carrier_mode = psmuxif->carrier;
	}
	else smux->carrier_mode = SMUX_CARRIER_AUTO;
	
	realdev_cb = smux_realdev_cb_get(real_dev);
	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s(%d): %p ",__func__,__LINE__,realdev_cb);
	if (NULL == realdev_cb) { /* create a new one */
		realdev_cb = kzalloc(sizeof(*realdev_cb), GFP_KERNEL);
		if (!realdev_cb) 
			goto out_free_smuxdev;
		
		smux_init_realdev_cb(realdev_cb);
		
		//printk("%s(%d): %p %p/%p %p\n",__func__,__LINE__,real_dev_list.next,real_dev_list.prev, realdev_cb->list_real.next, realdev_cb->list_real.prev);
		realdev_cb->realdev = real_dev;
		smux_priv_flags_mask_set(real_dev,PRIV_RSMUX);
		list_add_rcu(&realdev_cb->list_real, &real_dev_list);
		if(smux_priv_flags_get(real_dev,PRIV_VSMUX))
			smux_dev_priv(real_dev)->real_dev_priv = realdev_cb;
		
#ifdef CONFIG_COMMON_RT_API	
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
		for(i=0 ; i<SMUX_HW_VLAN_FILTER_DEFAULT_NUM ; i++)
			realdev_cb->hw_vlan_filter.default_acl_index[i] = -1;
		for(i=0 ; i<SMUX_HW_VLAN_FILTER_NUM ; i++)
		{
			realdev_cb->hw_vlan_filter.entry[i].valid = 0;
			realdev_cb->hw_vlan_filter.entry[i].acl_index = -1;
			realdev_cb->hw_vlan_filter.entry[i].ref_count = 0;
		}
#if defined(CONFIG_RTL8686NIC) && defined(CONFIG_RTL9607C_SERIES)
		realdev_cb->hw_vlan_filter.port_mask = (((struct re_dev_private*)netdev_priv(realdev_cb->realdev))->txPortMask);
#endif
#endif
#endif
		//printk("%s(%d): %p %p\n",__func__,__LINE__,real_dev_list.next,real_dev_list.prev);
	}
	list_add_rcu(&smux->list_virt, &realdev_cb->list_virt);

	if(psmuxif->dev_flags & SMUXCTL_IF_FLAGS_RSMUX){
		SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s Append IFF_RSMUX ..", __func__); 
		realdev_cb = smux_realdev_cb_get(new_dev);
		if (NULL == realdev_cb) { /* create a new one */
			realdev_cb = kzalloc(sizeof(*realdev_cb), GFP_KERNEL);
			if (!realdev_cb) 
				goto out_free_smuxdev;

			smux_init_realdev_cb(realdev_cb);
			realdev_cb->realdev = new_dev;
			smux_priv_flags_mask_set(new_dev,PRIV_RSMUX);
			list_add_rcu(&realdev_cb->list_real, &real_dev_list);
			if(smux_priv_flags_get(new_dev,PRIV_VSMUX))
				smux_dev_priv(new_dev)->real_dev_priv = realdev_cb;
			
			for(i=SMUX_DIR_RX;i<SMUX_DIR_MAX;i++)
			{
				if(psmuxif->policy[i] < SMUX_TARGET_END)
				{
					realdev_cb->default_policy[i] = psmuxif->policy[i];
				}
				else 
				{
					realdev_cb->default_policy[i] = SMUX_TARGET_CONTINUE;
				}		
			}
		}
	}

	#ifdef RTK_NETDEV_PRIV_FLAGS
	if (rtk_netdev_get_flags(real_dev)&SMUXDEV_INHERIT_PRIV_FLAG) {
		u32 tmp = rtk_netdev_get_flags(real_dev)&SMUXDEV_INHERIT_PRIV_FLAG;
		rtk_netdev_set_flags(new_dev, rtk_netdev_get_flags(new_dev)|tmp);
	}
	#else
	if(real_dev->priv_flags & SMUXDEV_INHERIT_PRIV_FLAG)
	{
		new_dev->priv_flags |= (real_dev->priv_flags & SMUXDEV_INHERIT_PRIV_FLAG);
	}
	#endif
	
	if(netif_carrier_ok(new_dev) && smux->carrier_mode != SMUX_CARRIER_AUTO)
	{
		netif_carrier_off(new_dev);
	}

	return 0;

out_free_smuxdev:
	__unregister_smux_dev(new_dev);
out_free_newdev:
	free_netdev(new_dev);
	return err;
}

static struct smux_ruleset_t *ruleset_find_by_id(struct list_head *head, u32 id) {
	struct smux_ruleset_t *s, *ret = NULL;
	
	rcu_read_lock();
	list_for_each_entry_rcu(s, head, list) {
		if (s->id == id) {
			ret = s;
			break;
		}
	}
	rcu_read_unlock();
	return ret;	
}

/* what we do here is modify data field for data path processing: ie. do htons here so we don't have to do it in datapath */
static void prepare_ruleset(struct smux_ruleset_t *ruleset) {
	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s", __func__);
};

#ifndef list_for_each_entry_rcu_reverse
#define list_for_each_entry_rcu_reverse(pos, head, member) \
	for (pos = list_entry_rcu((head)->prev, typeof(*pos), member); \
		&pos->member != (head); \
		pos = list_entry_rcu(pos->member.prev, typeof(*pos), member))
#endif

static int smux_rule_add(const char *realname, enum smux_direction_e dir, int depth, struct smux_rule_t *rule, int method, u32 *id) {
	struct smux_realdev_cb_t *realdev_cb;
	struct smux_ruleset_t *ruleset, *target, *s;
	struct smux_table_t *tbl;
	struct list_head *head_list;
	int ret = 0, add_success = 0;
	
	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "KRuleAdd: m=%d,id=%d %s/%d/%d  nr=%d", method, *id, realname, dir, depth,rule->action_count);
	
	if ((dir >= SMUX_DIR_MAX) || (depth > SMUX_MAX_TAGS))
		return -EINVAL;
	
	realdev_cb = smux_realdev_cb_get_by_ifname(realname);
	if (NULL==realdev_cb)
		return -ENODEV;
	
	ruleset = kzalloc(sizeof(*ruleset), GFP_KERNEL);
	if (NULL==ruleset) {		
		ret = -ENOMEM;
		goto out;
	}
	
	tbl = &realdev_cb->tables[dir][depth];
	
	memcpy(&ruleset->rule, rule, sizeof(ruleset->rule));
	INIT_LIST_HEAD(&ruleset->list);
	ruleset->id = tbl->curr_id;
	prepare_ruleset(ruleset);

	if (strlen(ruleset->rule.rxif_name)) {
		ruleset->rx_dev = get_virt_dev(realdev_cb, ruleset->rule.rxif_name);
		if (ruleset->rx_dev == NULL) {
			SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "virtual device not exist (%s)", ruleset->rule.rxif_name);
			ret = -EINVAL;
			goto err_free;
		}
	}

	spin_lock(&smux_lock);	
	
	if(list_empty(&tbl->rule_head))
	{
		list_add_rcu(&ruleset->list, &tbl->rule_head);
	}
	else
	{
		if(method == SMUX_RULE_METHOD_INSERT_BEFORE || method == SMUX_RULE_METHOD_INSERT_AFTER)
		{
			target = ruleset_find_by_id(&tbl->rule_head, *id);
			if(target == NULL) head_list = &tbl->rule_head;
			else head_list = &target->list;
		}
		else head_list = &tbl->rule_head;
		
		if(method == SMUX_RULE_METHOD_INSERT || method == SMUX_RULE_METHOD_INSERT_BEFORE)
		{
			list_for_each_entry_rcu(s, head_list, list) 
			{
				if(s->rule.priority <= ruleset->rule.priority)
				{
					list_add_tail_rcu(&ruleset->list, &s->list);
					add_success = 1;
					break;
				}
			}
			if(!add_success) list_add_tail_rcu(&ruleset->list, head_list);
		}
		else
		{
			list_for_each_entry_rcu_reverse(s, head_list, list) 
			{
				if(s->rule.priority >= ruleset->rule.priority)
				{
					list_add_rcu(&ruleset->list, &s->list);
					add_success = 1;
					break;
				}
			}
			if(!add_success) list_add_rcu(&ruleset->list, head_list);
		}
	}
#if 0
	switch (method) 
	{
		case SMUX_RULE_METHOD_APPEND:
		default:
			list_add_tail_rcu(&ruleset->list, &tbl->rule_head);		
			SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s(%d): add id %d",__func__,__LINE__,ruleset->id);
			break;
		case SMUX_RULE_METHOD_INSERT:
			list_add_rcu(&ruleset->list, &tbl->rule_head);		
			SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s(%d): add id %d",__func__,__LINE__,ruleset->id);
			break;	
		case SMUX_RULE_METHOD_INSERT_BEFORE:
		case SMUX_RULE_METHOD_INSERT_AFTER:
			target = ruleset_find_by_id(&tbl->rule_head, *id);
			if (target) {
				if (method==SMUX_RULE_METHOD_INSERT_BEFORE)
					list_add_tail_rcu(&ruleset->list, &target->list);
				else 
					list_add_rcu(&ruleset->list, &target->list);
			} else {
				ret = -EINVAL;
				spin_unlock(&smux_lock);
				SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s(%d): fail to insert before/after id %d",__func__,__LINE__,*id);
				goto err_free;
			}
			break;
	}
#endif
	
#ifdef CONFIG_COMMON_RT_API
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
	if(dir != SMUX_DIR_TX)
	{
		smux_add_hw_vlan_filter_by_rules(realdev_cb, depth, rule);
	}
#endif
#endif

	spin_unlock(&smux_lock);
	
	*id = ruleset->id;
	tbl->curr_id++;
	tbl->rule_count++;
	if(dir==SMUX_DIR_RXMC)
	{
		realdev_cb->multicast.rx_multicast_rule_count++;		
		SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s(%d): rx_multicast_rule_count=%d",__func__,__LINE__, realdev_cb->multicast.rx_multicast_rule_count);
		if(!realdev_cb->multicast.rx_multicast_rule_exist)
			realdev_cb->multicast.rx_multicast_rule_exist = 1;
	}
			
	return 0;
err_free:
	kfree(ruleset);
out:
	return ret;
}

int smux_rule_del(const char *realname, enum smux_direction_e dir, int depth, int method, u32 *id, void *arg) {
	struct smux_realdev_cb_t *realdev_cb;
	struct smux_ruleset_t *target;
	struct smux_table_t *tbl;
	int ret = 0;
	
	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "KRuleDel: m=%d %s/%d  nr=%d", method, realname, dir, depth);
	
	if ((dir >= SMUX_DIR_MAX) || (depth > SMUX_MAX_TAGS))
		return -EINVAL;
	
	realdev_cb = smux_realdev_cb_get_by_ifname(realname);
	if (NULL==realdev_cb)
		return -ENODEV;
	
	tbl = &realdev_cb->tables[dir][depth];
	
	spin_lock(&smux_lock);	
	
	switch (method) {
		case SMUX_RULE_METHOD_DELETE_ALL:
		{
			rcu_read_lock();
			list_for_each_entry_rcu(target, &tbl->rule_head, list) {
#ifdef CONFIG_COMMON_RT_API
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
				smux_del_hw_vlan_filter_by_rules(realdev_cb, depth, &(target->rule));
#endif
#endif
				kfree_rcu(target, rcu);
			}    
			INIT_LIST_HEAD(&tbl->rule_head);
			tbl->curr_id = 0;
			tbl->rule_count = 0;
			rcu_read_unlock();
			SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s(%d): Remove all rule",__func__,__LINE__);
			break;	
		}
		case SMUX_RULE_METHOD_DELETE:
		{
			target = ruleset_find_by_id(&tbl->rule_head, *id);
			if (target) {
#ifdef CONFIG_COMMON_RT_API
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
				smux_del_hw_vlan_filter_by_rules(realdev_cb, depth, &(target->rule));
#endif
#endif
				list_del_rcu(&target->list);
				kfree_rcu(target, rcu);
				//tbl->curr_id--; 
				if(dir==SMUX_DIR_RXMC)
					realdev_cb->multicast.rx_multicast_rule_count--;
				tbl->rule_count--;
	#if 0		     
				rcu_read_lock();
				list_for_each_entry_rcu(target, &tbl->rule_head, list) {	
					if(target->id > *id){
						target->id--;    
					}    
				}    
				rcu_read_unlock();
	#endif	        
			} else {
				ret = -EINVAL;
				spin_unlock(&smux_lock);
				SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s(%d): fail to remove rule id %d",__func__,__LINE__,*id);
				goto out;
			}
			break;
		}
		case SMUX_RULE_METHOD_DELETE_ALIAS:
		{
			char *alias = (char*) arg;
			int full_math = 1, match_len, ret;
			if(alias)
			{
				if(alias[strlen(alias)-1] == '+')
				{
					full_math = 0;
					match_len = strlen(alias)-1;
				}
				rcu_read_lock();
				list_for_each_entry_rcu(target, &tbl->rule_head, list) {
					ret = (full_math) ? strcmp(target->rule.alias_name, alias) : strncmp(target->rule.alias_name, alias, match_len);
					if(ret == 0)
					{
#ifdef CONFIG_COMMON_RT_API
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
						smux_del_hw_vlan_filter_by_rules(realdev_cb, depth, &(target->rule));
#endif
#endif
						list_del_rcu(&target->list);
						if(dir==SMUX_DIR_RXMC)
							realdev_cb->multicast.rx_multicast_rule_count--;
						tbl->rule_count--;
						SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s(%d): Remove rule id %d\n",__func__,__LINE__, target->id);
						kfree_rcu(target, rcu);
						if(full_math) break;
					}
				}
				rcu_read_unlock();
			}
			break;
		}
		case SMUX_RULE_METHOD_DELETE_IFNAME:
		{
			char *ifname = (char*) arg;
			if(ifname)
			{
				rcu_read_lock();
				list_for_each_entry_rcu(target, &tbl->rule_head, list) {
					if((target->rx_dev && !strcmp(target->rx_dev->name, ifname)) || 
						((target->rule.filters.en_filter & (1<<SMUX_FLT_TXIF_NAME)) && !strcmp(target->rule.filters.txif_name, ifname)) ||
						((target->rule.filters.en_filter & (1<<SMUX_FLT_RXIF_NAME)) && !strcmp(target->rule.filters.rxif_name, ifname))
						)
					{
#ifdef CONFIG_COMMON_RT_API
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
						smux_del_hw_vlan_filter_by_rules(realdev_cb, depth, &(target->rule));
#endif
#endif
						list_del_rcu(&target->list);
						if(dir==SMUX_DIR_RXMC)
							realdev_cb->multicast.rx_multicast_rule_count--;
						tbl->rule_count--;
						SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s(%d): Remove rule id %d",__func__,__LINE__, target->id);
						kfree_rcu(target,rcu);
					}
				}    
				rcu_read_unlock();
				
			}
			break;
		}
	}

	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s(%d): rx_multicast_rule_count=%d",__func__,__LINE__, realdev_cb->multicast.rx_multicast_rule_count);
	if(dir==SMUX_DIR_RXMC && !realdev_cb->multicast.rx_multicast_rule_count && realdev_cb->multicast.rx_multicast_rule_exist)
		realdev_cb->multicast.rx_multicast_rule_exist = 0;
	
	spin_unlock(&smux_lock);

	
	return 0;

out:
	return ret;
}

static inline const char *filter_idx2str(int filter){
    int idx;
    for(idx=0; idx<SMUX_FLT_MAX; idx++) {
        if (filter & (1<<idx))
            break;
    }
	if(idx < SMUX_FLT_MAX)
		return filter_str[idx];
	else
		return "none";
}

int dump_filter_sip(struct smux_rule_filter_t *f)
{
	printk("%*s%s %pI4\n", 20, "", filter_str[SMUX_FLT_SIP], &f->sip);
	return 0;
}

int dump_filter_dip(struct smux_rule_filter_t *f)
{
	printk("%*s%s %pI4\n", 20, "", filter_str[SMUX_FLT_DIP], &f->dip);
	return 0;
}

int dump_filter_sip_range(struct smux_rule_filter_t *f)
{
	printk("%*s%s %pI4~%pI4\n", 20, "", filter_str[SMUX_FLT_SIP_RANGE], &f->sip_start, &f->sip_end);
	return 0;
}

int dump_filter_dip_range(struct smux_rule_filter_t *f)
{
	printk("%*s%s %pI4~%pI4\n", 20, "", filter_str[SMUX_FLT_DIP_RANGE], &f->dip_start, &f->dip_end);
	return 0;
}

int dump_filter_ipl4proto(struct smux_rule_filter_t *f)
{
	printk("%*s%s 0x%02x\n", 20, "", filter_str[SMUX_FLT_IPL4PROTO], f->ipl4proto);
	return 0;
}

int dump_filter_multicast(struct smux_rule_filter_t *f)
{
	printk("%*s%s\n", 20, "", filter_str[SMUX_FLT_MULTICAST]);
	return 0;
}

int dump_filter_dscp_range(struct smux_rule_filter_t *f)
{
	int i,j;
	printk("%*s%s ", 20, "", filter_str[SMUX_FLT_DSCP_RANGE]);
	for(i=0 ; i<DSCP_LEN ; i++)
	{
		for(j=0 ; j<BITWISE_NUM ; j++)
		{
			if((f->ipdscp[i]&(1<<j)) != 0)
				printk("%d,", i*BITWISE_NUM+j);
		}
	}
	printk("\n");
	return 0;
}

int dump_filter_ethtype(struct smux_rule_filter_t *f)
{
	printk("%*s%s 0x%04x\n", 20, "", filter_str[SMUX_FLT_ETHTYPE], f->ethtype);
	return 0;
}

int dump_filter_vtags(struct smux_rule_filter_t *f)
{
	int i;
	char buf[256], *pbuf;
	
	printk("%*s%s\n", 20, "", filter_str[SMUX_FLT_VTAGS]);
	for(i=0; i<=SMUX_MAX_TAGS; i++)
	{
		if(f->vtags[i].vlan_tci_mask | f->vtags[i].vlan_proto_mask){
			pbuf = buf;
			pbuf += sprintf(pbuf, "%*s tag:%d  ", 25, "", i);
			if(f->vtags[i].vlan_proto_mask)	{
				pbuf += sprintf(pbuf, "tpid:0x%04x  ", ntohs(f->vtags[i].vlan_proto));
			}
			if((f->vtags[i].vlan_tci_mask & PRIO_MASK) == PRIO_MASK) {
				pbuf += sprintf(pbuf, "pri:%u  ",  ntohs((f->vtags[i].vlan_tci&PRIO_MASK)) >> PRIO_SHIFT);
			}
			if((f->vtags[i].vlan_tci_mask & CFI_MASK) == CFI_MASK) {
				pbuf += sprintf(pbuf, "cfi:%u  ",  ntohs((f->vtags[i].vlan_tci&CFI_MASK)) >> CFI_SHIFT);
			}
			if((f->vtags[i].vlan_tci_mask & VID_MASK) == VID_MASK) {
				pbuf += sprintf(pbuf, "vid:%u  ",  ntohs((f->vtags[i].vlan_tci&VID_MASK)));
			}
			printk("%s\n", buf);
		}
	}
	return 0;
}

int dump_filter_uni_dmac_outermost_vlan(struct smux_rule_filter_t *f)
{
	printk("%*s%s %d\n", 20, "", filter_str[SMUX_FLT_UNI_DMAC_OUTERMOST_VLAN], f->check_outermost_vlan_map_dmac_by_lut);
	return 0;
}

int dump_filter_org_vtags(struct smux_rule_filter_t *f)
{
	int i;
	char buf[256], *pbuf;
	
	printk("%*s%s\n", 20, "", filter_str[SMUX_FLT_ORG_VTAGS]);
	for(i=0; i<=SMUX_MAX_TAGS; i++)
	{
		if(f->org_vtags[i].vlan_tci_mask | f->org_vtags[i].vlan_proto_mask){
			pbuf = buf;
			pbuf += sprintf(pbuf, "%*s tag:%d  ", 25, "", i);
			if(f->org_vtags[i].vlan_proto_mask) {
				pbuf += sprintf(pbuf, "tpid:0x%04x  ", ntohs(f->org_vtags[i].vlan_proto));
			}
			if((f->org_vtags[i].vlan_tci_mask & PRIO_MASK) == PRIO_MASK) {
				pbuf += sprintf(pbuf, "pri:%u  ",  ntohs((f->org_vtags[i].vlan_tci&PRIO_MASK)) >> PRIO_SHIFT);
			}
			if((f->org_vtags[i].vlan_tci_mask & CFI_MASK) == CFI_MASK) {
				pbuf += sprintf(pbuf, "cfi:%u  ",  ntohs((f->org_vtags[i].vlan_tci&CFI_MASK)) >> CFI_SHIFT);
			}
			if((f->org_vtags[i].vlan_tci_mask & VID_MASK) == VID_MASK) {
				pbuf += sprintf(pbuf, "vid:%u  ",  ntohs((f->org_vtags[i].vlan_tci&VID_MASK)));
			}
			printk("%s\n", buf);
		}
	}
	return 0;
}

int dump_filter_txif(struct smux_rule_filter_t *f)
{
	printk("%*s%s %s%s\n", 20, "", filter_str[SMUX_FLT_TXIF_NAME], f->txif_name, ((f->txif_match>=IFNAMSIZ)?"":"+"));
	return 0;
}

int dump_filter_skb_mark(struct smux_rule_filter_t *f)
{
	printk("%*s%s 0x%x/0x%x\n", 20, "", filter_str[SMUX_FLT_SKB_MARK], f->mark.value, f->mark.mask);
	return 0;
}

#ifdef CONFIG_RTK_SKB_MARK2
int dump_filter_skb_mark2(struct smux_rule_filter_t *f)
{
        printk("%*s%s 0x%llx/0x%llx\n", 20, "", filter_str[SMUX_FLT_SKB_MARK2], f->mark2.value, f->mark2.mask);
        return 0;
}
#endif

int dump_filter_uni_mac(struct smux_rule_filter_t *f)
{
	printk("%*s%s %d\n", 20, "", filter_str[SMUX_FLT_UNI_MAC], f->check_mac);
	return 0;
}

int dump_filter_dmac(struct smux_rule_filter_t *f)
{
	unsigned char *m = f->dmac;
	unsigned char *k = f->dmac_mask;
	printk("%*s%s %02X%02X%02X%02X%02X%02X  %02X%02X%02X%02X%02X%02X\n", 20, "", filter_str[SMUX_FLT_DMAC],
		m[0], m[1], m[2], m[3], m[4], m[5],
		k[0], k[1], k[2], k[3], k[4], k[5]);
	return 0;
}

int dump_filter_smac(struct smux_rule_filter_t *f)
{
	unsigned char *m = f->smac;
	unsigned char *k = f->smac_mask;
	printk("%*s%s %02X%02X%02X%02X%02X%02X  %02X%02X%02X%02X%02X%02X\n", 20, "", filter_str[SMUX_FLT_SMAC],
		m[0], m[1], m[2], m[3], m[4], m[5],
		k[0], k[1], k[2], k[3], k[4], k[5]);
	return 0;
}

static smux_dump_filter_func_t *dump_filter_funcs[] = {
	[SMUX_FLT_ETHTYPE] = dump_filter_ethtype,
	[SMUX_FLT_VTAGS] = dump_filter_vtags,
	[SMUX_FLT_ORG_VTAGS] = dump_filter_org_vtags,
	[SMUX_FLT_TXIF_NAME] = dump_filter_txif,
	[SMUX_FLT_SKB_MARK] = dump_filter_skb_mark,
#ifdef CONFIG_RTK_SKB_MARK2
	[SMUX_FLT_SKB_MARK2] = dump_filter_skb_mark2,
#endif
	[SMUX_FLT_UNI_MAC] = dump_filter_uni_mac,
	[SMUX_FLT_DMAC] = dump_filter_dmac,
	[SMUX_FLT_SMAC] = dump_filter_smac,
	[SMUX_FLT_IPL4PROTO] = dump_filter_ipl4proto,
	[SMUX_FLT_SIP] = dump_filter_sip,
	[SMUX_FLT_DIP] = dump_filter_dip,
	[SMUX_FLT_SIP_RANGE] = dump_filter_sip_range,
	[SMUX_FLT_DIP_RANGE] = dump_filter_dip_range,
	[SMUX_FLT_MULTICAST] = dump_filter_multicast,
	[SMUX_FLT_DSCP_RANGE] = dump_filter_dscp_range,
	/*for N:1 vlan translate, downstream eggress need to set corressponding vlan by mac for PPTP. */
	[SMUX_FLT_UNI_DMAC_OUTERMOST_VLAN] = dump_filter_uni_dmac_outermost_vlan,
};

static void dump_filters(struct smux_rule_filter_t *pfilter)
{
	int idx;
	unsigned int flags = pfilter->en_filter;
	smux_dump_filter_func_t *fn;

	for(idx=0; idx<SMUX_FLT_MAX; idx++) 
	{
		if (!flags) return;
		
		if (flags & (1<<idx))
		{
			flags &= ~(1<<idx);
			fn = dump_filter_funcs[idx];
			fn(pfilter);	
	    }
	}
}

int dump_action_set_rxif(struct smux_rule_action_t *act)
{
	printk("%*s%s %s\n", 20, "", action_str[SMUX_ACT_SET_RXIF], act->rxif_name);
	return 0;
}

int dump_action_set_tpid(struct smux_rule_action_t *act)
{
	int idx = act->args[1];
	u16 tpid = ntohs(act->args[0]);
	printk("%*s%s tag:%d  tpid:0x%04x \n", 20, "", action_str[SMUX_ACT_SET_TPID], idx, tpid);
	return 0;
}

int dump_action_set_vid(struct smux_rule_action_t *act)
{
	int idx = act->args[1];
	u16 vlan_tci = ntohs(act->args[0]);
	printk("%*s%s tag:%d  vlan:%hu \n", 20, "", action_str[SMUX_ACT_SET_VID], idx, vlan_tci);
	return 0;
}

int dump_action_set_priority(struct smux_rule_action_t *act)
{
	int idx = act->args[1];
	u16 priority = ntohs(act->args[0]) >> PRIO_SHIFT;
	printk("%*s%s tag:%d  priority:%hu \n", 20, "", action_str[SMUX_ACT_SET_PRIORITY], idx, priority);
	return 0;
}

int dump_action_set_cfi(struct smux_rule_action_t *act)
{
	int idx = act->args[1];
	u16 cfi = ntohs(act->args[0]) >> CFI_SHIFT;
	printk("%*s%s tag:%d  cfi:%hu \n", 20, "", action_str[SMUX_ACT_SET_CFI], idx, cfi);
	return 0;
}

int dump_action_set_ethtype(struct smux_rule_action_t *act)
{
	unsigned short eth = (unsigned short)act->args[0];
	printk("%*s%s 0x%04x\n", 20, "", action_str[SMUX_ACT_SET_ETHTYPE], eth);
	return 0;
}

int dump_action_pop_tag(struct smux_rule_action_t *act)
{
	printk("%*s%s\n", 20, "", action_str[SMUX_ACT_POP_TAG]);
	return 0;
}

int dump_action_push_tag(struct smux_rule_action_t *act)
{
	printk("%*s%s\n", 20, "", action_str[SMUX_ACT_PUSH_TAG]);
	return 0;
}

int dump_action_dup_forward(struct smux_rule_action_t *act)
{
	printk("%*s%s\n", 20, "", action_str[SMUX_ACT_DUP_FORWARD]);
	return 0;
}

int dump_action_set_skb_mark(struct smux_rule_action_t *act)
{
	printk("%*s%s 0x%x/0x%x\n", 20, "", action_str[SMUX_ACT_SET_SKB_MARK], act->args[1], act->args[0]);
	return 0;
}

#ifdef CONFIG_RTK_SKB_MARK2
int dump_action_set_skb_mark2(struct smux_rule_action_t *act)
{
        printk("%*s%s 0x%llx/0x%llx\n", 20, "", action_str[SMUX_ACT_SET_SKB_MARK2], act->args_64[1], act->args_64[0]);
        return 0;
}
#endif

int dump_action_set_dscp(struct smux_rule_action_t *act)
{
	uint8_t dscp = act->args[0];
	uint32_t dscpv6 = act->args[1];
	printk("%*s%s dscp:%hhu (v6:%u)\n", 20, "", action_str[SMUX_ACT_SET_DSCP], IP_TOS_GET_DSCP(((struct smux_ip_tos_t*)&dscp)), IP6_TOS_GET_DSCP(((struct smux_ipv6_tos_t*)&dscpv6)));
	return 0;
}

int dump_action_copy_8021q(struct smux_rule_action_t *act)
{
	int idx_src = act->args[0];
	int idx_dest = act->args[1];
	
	printk("%*s%s from:%d  to:%d \n", 20, "", action_str[SMUX_ACT_COPY_8021Q], idx_src, idx_dest);
	return 0;
}

int dump_action_copy_tpid(struct smux_rule_action_t *act)
{
	int idx_src = act->args[0];
	int idx_dest = act->args[1];
	
	printk("%*s%s from:%d  to:%d \n", 20, "", action_str[SMUX_ACT_COPY_TPID], idx_src, idx_dest);
	return 0;
}

int dump_action_copy_vid(struct smux_rule_action_t *act)
{
	int idx_src = act->args[0];
	int idx_dest = act->args[1];
	
	printk("%*s%s from:%d  to:%d \n", 20, "", action_str[SMUX_ACT_COPY_VID], idx_src, idx_dest);
	return 0;
}

int dump_action_copy_priority(struct smux_rule_action_t *act)
{
	int idx_src = act->args[0];
	int idx_dest = act->args[1];
	
	printk("%*s%s from:%d  to:%d \n", 20, "", action_str[SMUX_ACT_COPY_PRIORITY], idx_src, idx_dest);
	return 0;
}

int dump_action_copy_cfi(struct smux_rule_action_t *act)
{
	int idx_src = act->args[0];
	int idx_dest = act->args[1];
	
	printk("%*s%s from:%d  to:%d \n", 20, "", action_str[SMUX_ACT_COPY_CFI], idx_src, idx_dest);
	return 0;
}

int dump_action_map_dscp_to_priority(struct smux_rule_action_t *act)
{
	int idx = act->args[0];
	
	printk("%*s%s tag:%d \n", 20, "", action_str[SMUX_ACT_MAP_DSCP_TO_PRIORITY], idx);
	return 0;
}

int dump_action_target(struct smux_rule_action_t *act)
{
	char target[64] = {0};
	
	if(act->args[0] == SMUX_TARGET_ACCEPT)
		sprintf(target, "ACCEPT");
	else if(act->args[0] == SMUX_TARGET_DROP)
		sprintf(target, "DROP");
	else if(act->args[0] == SMUX_TARGET_CONTINUE)
		sprintf(target, "CONTINUE");
	
	printk("%*s%s %s \n", 20, "", action_str[SMUX_ACT_SET_TARGET], target);
	return 0;
}

int dump_action_revert_vlan(struct smux_rule_action_t *act)
{
	printk("%*s%s\n", 20, "", action_str[SMUX_ACT_REVERT_VLAN]);
	return 0;
}

static smux_dump_action_func_t *dump_action_funcs[] = {
	[SMUX_ACT_SET_RXIF] = dump_action_set_rxif,
	[SMUX_ACT_SET_TPID] = dump_action_set_tpid,
	[SMUX_ACT_SET_VID] = dump_action_set_vid,
	[SMUX_ACT_SET_PRIORITY] = dump_action_set_priority,
	[SMUX_ACT_SET_CFI] = dump_action_set_cfi,
	[SMUX_ACT_SET_ETHTYPE] = dump_action_set_ethtype,
	[SMUX_ACT_POP_TAG] = dump_action_pop_tag,
	[SMUX_ACT_PUSH_TAG] = dump_action_push_tag,
	[SMUX_ACT_DUP_FORWARD] = dump_action_dup_forward,
	[SMUX_ACT_SET_SKB_MARK] = dump_action_set_skb_mark,
#ifdef CONFIG_RTK_SKB_MARK2
	[SMUX_ACT_SET_SKB_MARK2] = dump_action_set_skb_mark2,
#endif
	[SMUX_ACT_SET_DSCP] = dump_action_set_dscp,
	[SMUX_ACT_COPY_8021Q] = dump_action_copy_8021q,
	[SMUX_ACT_COPY_TPID] = dump_action_copy_tpid,
	[SMUX_ACT_COPY_VID] = dump_action_copy_vid,
	[SMUX_ACT_COPY_PRIORITY] = dump_action_copy_priority,
	[SMUX_ACT_COPY_CFI] = dump_action_copy_cfi,
	[SMUX_ACT_MAP_DSCP_TO_PRIORITY] = dump_action_map_dscp_to_priority,
	[SMUX_ACT_SET_TARGET] = dump_action_target,
	[SMUX_ACT_REVERT_VLAN] = dump_action_revert_vlan,
};

static void dump_actions(struct smux_rule_action_t *pactions, int count)
{
	int i;
	smux_dump_action_func_t *fn;
	struct smux_rule_action_t *act;
	
	for (i=0; i<count; i++) 
	{
		act = pactions+i;
		fn = dump_action_funcs[act->action];
		fn(act);
	}
}

static int smux_show_table(const char *realname, enum smux_direction_e dir, int depth) {
	struct smux_realdev_cb_t *realdev_cb;
	struct smux_ruleset_t *target;
	struct smux_table_t *tbl;
	struct smux_rule_t *rule;
	
	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "KShowTable: %s/%d  nr=%d", realname, dir, depth);
	
	if ((dir >= SMUX_DIR_MAX) || (depth > SMUX_MAX_TAGS))
		return -EINVAL;

	realdev_cb = smux_realdev_cb_get_by_ifname(realname);
	if (NULL==realdev_cb)
		return -ENODEV;
	
	tbl = &realdev_cb->tables[dir][depth];
	if(smux_configure.dump_all==0 && tbl->rule_count <= 0) return 0;
	printk("*******************************************************\n");
	printk("IF: %s   DIR: %s (policy %s)   TAGS: %d   rules: %d\n",realname,(dir == SMUX_DIR_TX)?"TX":((dir == SMUX_DIR_RX)?"RX":"RXMC")
		, (realdev_cb->default_policy[dir]==SMUX_TARGET_CONTINUE) ? "CONTINUE" : ((realdev_cb->default_policy[dir]==SMUX_TARGET_ACCEPT) ? "ACCEPT" : "DROP"),depth
		, tbl->rule_count);
	printk("*******************************************************\n");
	rcu_read_lock();
	list_for_each_entry_rcu(target, &tbl->rule_head, list) {
	    rule = &target->rule;	
	    printk("\n[Rule]: %d   alias:%s   priority:%u   hit:%d\n",target->id, rule->alias_name, rule->priority, rule->hit_count);
	    printk("-----------------------------------------------------------\n");
	    printk("%*s[filter]: \n", 10, ""); dump_filters(&rule->filters);
	    printk("%*s[action]: \n", 10, ""); dump_actions(rule->actions, rule->action_count);   
	}    
	rcu_read_unlock();
	
	return 0;
}

static int smux_reset_table_count(const char *realname, enum smux_direction_e dir, int depth) {
	struct smux_realdev_cb_t *realdev_cb;
	struct smux_ruleset_t *target;
	struct smux_table_t *tbl;
	struct smux_rule_t *rule;
	
	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "KShowTable: %s/%d  nr=%d", realname, dir, depth);
	
	if ((dir >= SMUX_DIR_MAX) || (depth > SMUX_MAX_TAGS))
		return -EINVAL;

	realdev_cb = smux_realdev_cb_get_by_ifname(realname);
	if (NULL==realdev_cb)
		return -ENODEV;
	
	tbl = &realdev_cb->tables[dir][depth];
	rcu_read_lock();
	list_for_each_entry_rcu(target, &tbl->rule_head, list) {
	    rule = &target->rule;
		rule->hit_count = 0;  
	}    
	rcu_read_unlock();
	
	return 0;
}

static int smux_set_default_value(const char *realname, enum smux_direction_e dir, int depth, int method, __be16 tpid, __be16 vid) {
	struct smux_realdev_cb_t *realdev_cb;
	struct smux_table_t *tbl;
	
	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "KSetDefaultValue: %s/%d  nr=%d", realname, dir, depth);
	
	if ((dir >= SMUX_DIR_MAX) || (depth > SMUX_MAX_TAGS))
		return -EINVAL;
	
	realdev_cb = smux_realdev_cb_get_by_ifname(realname);
	if (NULL==realdev_cb)
		return -ENODEV;
	
	tbl = &realdev_cb->tables[dir][depth];
	
	switch (method) {
		case SMUX_DEFAULT_METHOD_TPID:
		    tbl->default_tpid = htons(tpid);
		    SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s default_tpid=%x",__func__,tpid);
			break;
		case SMUX_DEFAULT_METHOD_VID:
		    tbl->default_vlan_tci = htons(vid);
		    SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s default_vlan_tci=%x",__func__,vid);
			break;
	}
	return 0;
}

static int smux_set_qos(const char *realname, int method, struct smux_qos_rule_t *qos) {
	struct smux_vlan_tag_t *vlan_tag;
	struct smux_ip_tos_t *ip_tos;		
	struct smux_realdev_cb_t *realdev_cb;
	struct smux_dscp_mapping_entry_t *dscp_entry;
	
	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "KSetQoS: method=%d, ifname=%s", method, (realname)?realname:"");

	if(method == SMUX_QOS_METHOD_DSCP_MAPPING_CHANGE_DEFAULT)
		goto skip_dev_check;

	realdev_cb = smux_realdev_cb_get_by_ifname(realname);
	if (NULL==realdev_cb)
		return -ENODEV;

skip_dev_check:
	
	switch (method) {
		case SMUX_QOS_METHOD_DSCP_MAPPING_ENABLE:
			realdev_cb->qos.vlan_idx = qos->vlan_idx;
			realdev_cb->qos.dscp_mapping_enable = qos->dscp_mapping_enable;
			if(realdev_cb->qos.dscp_mapping_table == NULL) 
				realdev_cb->qos.dscp_mapping_table = kzalloc(sizeof(dscp_mapping_table_default), GFP_KERNEL);
			memcpy(realdev_cb->qos.dscp_mapping_table, &dscp_mapping_table_default[0], sizeof(dscp_mapping_table_default));
		    SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s DSCP for vlan_idx=%d enable=%d",__func__,realdev_cb->qos.vlan_idx,realdev_cb->qos.dscp_mapping_enable);
			break;
		case SMUX_QOS_METHOD_DSCP_MAPPING_SET:		
			if(qos->dscp < SMUX_MAX_DSCP_NUM)
			{
				if(realdev_cb->qos.dscp_mapping_table == NULL) {
					realdev_cb->qos.dscp_mapping_table = kzalloc(sizeof(dscp_mapping_table_default), GFP_KERNEL);
					memcpy(realdev_cb->qos.dscp_mapping_table, &dscp_mapping_table_default[0], sizeof(dscp_mapping_table_default));
				}
				
				dscp_entry = &realdev_cb->qos.dscp_mapping_table[qos->dscp];
				dscp_entry->enable = 1;
				ip_tos = (struct smux_ip_tos_t*)&(dscp_entry->ip_tos);
				IP_TOS_SET_DSCP(ip_tos, ((qos->dscp << IP_DSCP_SHIFT)&IP_DSCP_MASK));
				vlan_tag = &(dscp_entry->vlan_tag);
				vlan_tag->vlan_tci = (vlan_tag->vlan_tci & ~PRIO_MASK) | htons(qos->priority << PRIO_SHIFT);
				
				SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s mapping DSCP=0x%02x(%d) to VLAN priority %d",__func__,qos->dscp,qos->dscp,qos->priority);
			}
			else
			{
				SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s not found DSCP=0x%02x(%d) in table",__func__,qos->dscp,qos->dscp);
			}
			break;
		case SMUX_QOS_METHOD_DSCP_MAPPING_DEFAULT:
			if(realdev_cb->qos.dscp_mapping_table != NULL) {
				memcpy(realdev_cb->qos.dscp_mapping_table, &dscp_mapping_table_default[0], sizeof(dscp_mapping_table_default));
			}
			SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s %s set DSCP mapping to default",__func__,realname);
			break;
		case SMUX_QOS_METHOD_DSCP_MAPPING_CHANGE_DEFAULT:
			if(qos->dscp < SMUX_MAX_DSCP_NUM)
			{
				dscp_entry = &dscp_mapping_table_default[qos->dscp];
				dscp_entry->enable = 1;
				ip_tos = (struct smux_ip_tos_t*)&(dscp_entry->ip_tos);
				IP_TOS_SET_DSCP(ip_tos, ((qos->dscp << IP_DSCP_SHIFT)&IP_DSCP_MASK));
				vlan_tag = &(dscp_entry->vlan_tag);
				vlan_tag->vlan_tci = (vlan_tag->vlan_tci & ~PRIO_MASK) | htons(qos->priority << PRIO_SHIFT);
				
				SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s mapping DSCP=0x%02x(%d) to VLAN priority %d",__func__,qos->dscp,qos->dscp,qos->priority);
			}
			else
			{
				SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s not found DSCP=0x%02x(%d) in table",__func__,qos->dscp,qos->dscp);
			}
			break;
	}
	return 0;
}

static int check_rxmc_rule_mark_exit(struct smux_mc_gem_port_skbmark_t *mc_gem_port_skbmark_p)
{
	int ret = 0, depth;
	struct smux_realdev_cb_t  *realdev_cb, *tmp;
	struct smux_table_t *tbl;
	struct smux_ruleset_t *target;
	struct smux_rule_t *rule;
	enum smux_direction_e dir = SMUX_DIR_RXMC;
	uint32_t mark_chk1, mark_chk2;
#ifdef CONFIG_RTK_SKB_MARK2
	uint64_t mark_64b_chk1, mark_64b_chk2;
#endif

	rtnl_lock();
	list_for_each_entry_safe(realdev_cb, tmp, &real_dev_list, list_real) 
	{
		for(depth=0;depth<=SMUX_MAX_TAGS;depth++)
		{
			tbl = &realdev_cb->tables[dir][depth];
			rcu_read_lock();
			list_for_each_entry_rcu(target, &tbl->rule_head, list) {
				rule = &target->rule;
				if(mc_gem_port_skbmark_p->mark_select == 1 && 
					(rule->filters.en_filter & 1<<SMUX_FLT_SKB_MARK))
				{
					mark_chk1 = rule->filters.mark.mask & rule->filters.mark.value;
					mark_chk2 = mc_gem_port_skbmark_p->mark1 & mc_gem_port_skbmark_p->mark_mask1;
					if(mark_chk1 == mark_chk2)
						ret = 1;
				}
#ifdef CONFIG_RTK_SKB_MARK2
				if(mc_gem_port_skbmark_p->mark_select == 2 && 
					(rule->filters.en_filter & 1<<SMUX_FLT_SKB_MARK2))
				{
					mark_64b_chk1 = rule->filters.mark2.mask & rule->filters.mark2.value;
					mark_64b_chk2 = mc_gem_port_skbmark_p->mark2 & mc_gem_port_skbmark_p->mark_mask2;
					if(mark_64b_chk1 == mark_64b_chk2)
						ret = 1;	
				}
#endif
				if(ret) break;
			}    
			rcu_read_unlock();
			if(ret) break;
		}
		if(ret) break;
	}
	rtnl_unlock();
	
	return ret;
}

static int smux_set_mc_gem_port_skbmark(int method, struct smux_mc_gem_port_skbmark_t *mc_gem_port_skbmark_p) {
	int i, j;
	
	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "KSetMCGemPortSkbmark:");
	
	switch (method) {
		case SMUX_MC_GEM_PORT_SKBMARK_METHOD_ADD:
			for(i=0 ; i<SMUX_MC_GEM_PORT_SKBMARK_NUM ; i++)
			{
				if(!memcmp(&smux_configure.mc_gem_port_skbmark_tbl[i], mc_gem_port_skbmark_p, sizeof(struct smux_mc_gem_port_skbmark_t)))
				{
					SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "entry exist !");
					return 0;
				}
			}
			for(i=0 ; i<SMUX_MC_GEM_PORT_SKBMARK_NUM ; i++)
			{
				if(smux_configure.mc_gem_port_skbmark_tbl[i].mark_select == 0)
					break;
			}
			if(i<SMUX_MC_GEM_PORT_SKBMARK_NUM)
			{
				memcpy(&smux_configure.mc_gem_port_skbmark_tbl[i], mc_gem_port_skbmark_p, sizeof(struct smux_mc_gem_port_skbmark_t));
			}
			else
			{
				SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "table full !");
				return -1;
			}
			break;
		case SMUX_MC_GEM_PORT_SKBMARK_METHOD_DEL:
			if(check_rxmc_rule_mark_exit(mc_gem_port_skbmark_p))
			{
				SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "RXMC rule exists!  Cannot delete gem port filter.");
				return 0;
			}
			for(i=0 ; i<SMUX_MC_GEM_PORT_SKBMARK_NUM ; i++)
			{
				if(!memcmp(&smux_configure.mc_gem_port_skbmark_tbl[i], mc_gem_port_skbmark_p, sizeof(struct smux_mc_gem_port_skbmark_t)))
				{
					memset(&smux_configure.mc_gem_port_skbmark_tbl[i], 0, sizeof(struct smux_mc_gem_port_skbmark_t));
					break;
				}
			}
			if(i==SMUX_MC_GEM_PORT_SKBMARK_NUM)
			{
				SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "not found !");
				return -1;
			}
			else
			{
				for(j=(i+1) ; j<SMUX_MC_GEM_PORT_SKBMARK_NUM ; j++)
				{
					memcpy(&smux_configure.mc_gem_port_skbmark_tbl[j-1], &smux_configure.mc_gem_port_skbmark_tbl[j], sizeof(struct smux_mc_gem_port_skbmark_t));
				}
				memset(&smux_configure.mc_gem_port_skbmark_tbl[SMUX_MC_GEM_PORT_SKBMARK_NUM-1], 0, sizeof(struct smux_mc_gem_port_skbmark_t));
			}
			break;
		default:
			return -1;
	}
	return 0;
}

#ifdef CONFIG_COMMON_RT_API
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
static int smux_set_hw_vlan_filter_acl_add(struct smux_realdev_cb_t  *realdev_cb, unsigned int vid, int isDefault)
{
	rt_acl_filterAndQos_t acl_filter = {0};
	unsigned int acl_idx;

	if(isDefault)
		acl_filter.acl_weight = SMUX_HW_VLAN_FILTER_ACL_WEIGHT_LOW;
	else
		acl_filter.acl_weight = SMUX_HW_VLAN_FILTER_ACL_WEIGHT_MED;

	acl_filter.filter_fields |= RT_ACL_INGRESS_CTAGIF_BIT; 
	acl_filter.ingress_ctagif = 1;	

	if(!isDefault)
	{
		acl_filter.filter_fields |= RT_ACL_INGRESS_CTAG_VID_BIT; 
		acl_filter.ingress_ctag_vid = vid;
	}

	acl_filter.filter_fields |= RT_ACL_INGRESS_PORT_MASK_BIT; 
	acl_filter.ingress_port_mask = realdev_cb->hw_vlan_filter.port_mask;
	
	if(realdev_cb->hw_vlan_filter.mode == 0x1)
	{
		acl_filter.filter_fields |= RT_ACL_INGRESS_DMAC_BIT; 
		acl_filter.ingress_dmac[0] = 0x0;
		acl_filter.ingress_dmac_mask[0] = 0x1;
	}
	else if(realdev_cb->hw_vlan_filter.mode == 0x2)
	{
		acl_filter.filter_fields |= RT_ACL_INGRESS_DMAC_BIT; 
		acl_filter.ingress_dmac[0] = acl_filter.ingress_dmac_mask[0] = 0xff;
		acl_filter.ingress_dmac[1] = acl_filter.ingress_dmac_mask[1] = 0xff;
		acl_filter.ingress_dmac[2] = acl_filter.ingress_dmac_mask[2] = 0xff;
		acl_filter.ingress_dmac[3] = acl_filter.ingress_dmac_mask[3] = 0xff;
		acl_filter.ingress_dmac[4] = acl_filter.ingress_dmac_mask[4] = 0xff;
		acl_filter.ingress_dmac[5] = acl_filter.ingress_dmac_mask[5] = 0xff;
	}
	else if(realdev_cb->hw_vlan_filter.mode == 0x4)
	{
		acl_filter.filter_fields |= RT_ACL_INGRESS_DMAC_BIT; 
		acl_filter.ingress_dmac[0] = 0x1;
		acl_filter.ingress_dmac_mask[0] = 0x1;
	}

	if(isDefault)
		acl_filter.action_fields = RT_ACL_ACTION_FORWARD_GROUP_DROP_BIT;
	else
		acl_filter.action_fields = RT_ACL_ACTION_FORWARD_GROUP_PERMIT_BIT;
	
	if(rt_acl_filterAndQos_add(acl_filter, &acl_idx))
	{
		SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s %d: add ACL FAIL !", __func__, __LINE__);
		return -1;
	}

	if(isDefault)
	{
		realdev_cb->hw_vlan_filter.default_acl_index[0] = acl_idx;
		memset(&acl_filter, 0, sizeof(rt_acl_filterAndQos_t));
		acl_filter.acl_weight = SMUX_HW_VLAN_FILTER_ACL_WEIGHT_HIGH;

		acl_filter.filter_fields |= RT_ACL_INGRESS_CTAGIF_BIT; 
		acl_filter.ingress_ctagif = 1;	

		acl_filter.filter_fields |= RT_ACL_INGRESS_PORT_MASK_BIT; 
		acl_filter.ingress_port_mask = realdev_cb->hw_vlan_filter.port_mask;

		if(realdev_cb->hw_vlan_filter.mode == 0x4 || realdev_cb->hw_vlan_filter.mode == 0x5)
		{
			memset(&acl_filter.ingress_dmac[0], 0, sizeof(acl_filter.ingress_dmac));
			acl_filter.filter_fields |= RT_ACL_INGRESS_DMAC_BIT; 
			acl_filter.ingress_dmac[0] = acl_filter.ingress_dmac_mask[0] = 0xff;
			acl_filter.ingress_dmac[1] = acl_filter.ingress_dmac_mask[1] = 0xff;
			acl_filter.ingress_dmac[2] = acl_filter.ingress_dmac_mask[2] = 0xff;
			acl_filter.ingress_dmac[3] = acl_filter.ingress_dmac_mask[3] = 0xff;
			acl_filter.ingress_dmac[4] = acl_filter.ingress_dmac_mask[4] = 0xff;
			acl_filter.ingress_dmac[5] = acl_filter.ingress_dmac_mask[5] = 0xff;

			acl_filter.action_fields = RT_ACL_ACTION_FORWARD_GROUP_PERMIT_BIT;

			if(rt_acl_filterAndQos_add(acl_filter, &acl_idx))
			{
				SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s %d: add ACL FAIL !", __func__, __LINE__);
				return -1;
			}

			realdev_cb->hw_vlan_filter.default_acl_index[1] = acl_idx;
		}
		else if(realdev_cb->hw_vlan_filter.mode == 0x3)
		{
			memset(&acl_filter.ingress_dmac[0], 0, sizeof(acl_filter.ingress_dmac));
			acl_filter.filter_fields |= RT_ACL_INGRESS_DMAC_BIT; 
			acl_filter.ingress_dmac[0] = 0x1;
			acl_filter.ingress_dmac_mask[0] = 0xff;

			acl_filter.action_fields = RT_ACL_ACTION_FORWARD_GROUP_PERMIT_BIT;

			if(rt_acl_filterAndQos_add(acl_filter, &acl_idx))
			{
				SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s %d: add ACL FAIL !", __func__, __LINE__);
				return -1;
			}

			realdev_cb->hw_vlan_filter.default_acl_index[1] = acl_idx;

			memset(&acl_filter.ingress_dmac[0], 0, sizeof(acl_filter.ingress_dmac));
			acl_filter.filter_fields |= RT_ACL_INGRESS_DMAC_BIT; 
			acl_filter.ingress_dmac[0] = 0x33;
			acl_filter.ingress_dmac_mask[0] = 0xff;
			acl_filter.ingress_dmac[1] = 0x33;
			acl_filter.ingress_dmac_mask[1] = 0xff;

			acl_filter.action_fields = RT_ACL_ACTION_FORWARD_GROUP_PERMIT_BIT;

			if(rt_acl_filterAndQos_add(acl_filter, &acl_idx))
			{
				SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s %d: add ACL FAIL !", __func__, __LINE__);
				return -1;
			}

			realdev_cb->hw_vlan_filter.default_acl_index[2] = acl_idx;
		}
		else if(realdev_cb->hw_vlan_filter.mode == 0x6)
		{
			memset(&acl_filter.ingress_dmac[0], 0, sizeof(acl_filter.ingress_dmac));
			acl_filter.filter_fields |= RT_ACL_INGRESS_DMAC_BIT; 
			acl_filter.ingress_dmac[0] = 0x0;
			acl_filter.ingress_dmac_mask[0] = 0x1;

			acl_filter.action_fields = RT_ACL_ACTION_FORWARD_GROUP_PERMIT_BIT;

			if(rt_acl_filterAndQos_add(acl_filter, &acl_idx))
			{
				SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s %d: add ACL FAIL !", __func__, __LINE__);
				return -1;
			}

			realdev_cb->hw_vlan_filter.default_acl_index[1] = acl_idx;
		}

		return 0;
	}
	else
		return acl_idx;
}

static int smux_set_hw_vlan_filter_acl_del(int acl_index)
{
	if(acl_index == -1)
		return 0;
	
	if(!rt_acl_filterAndQos_del(acl_index))
	{
		return 0;
	}
	else
	{
		SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s %d: del ACL FAIL !\n", __func__, __LINE__);
		return -1;
	}
}

static int smux_set_hw_vlan_filter(int method, struct smux_hw_vlan_filter_para_t *hw_vlan_filter_p) {
	struct smux_realdev_cb_t  *realdev_cb, *tmp;
	int i;
	
	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "KSetHWVlanFilter: m=%d", method);

	switch (method) {		
		case SMUX_MC_HW_VLAN_FILTER_METHOD_SWITCH_ON:
			smux_configure.hw_vlan_filter = 1;
			break;
		case SMUX_MC_HW_VLAN_FILTER_METHOD_SWITCH_OFF:
			smux_configure.hw_vlan_filter = 0;
			list_for_each_entry_safe(realdev_cb, tmp, &real_dev_list, list_real) {
				if(!smux_priv_flags_get(realdev_cb->realdev, PRIV_VSMUX))
				{
					if(realdev_cb->hw_vlan_filter.mode)
					{
						for(i=0 ; i<SMUX_HW_VLAN_FILTER_DEFAULT_NUM ; i++)
						{
							smux_set_hw_vlan_filter_acl_del(realdev_cb->hw_vlan_filter.default_acl_index[i]);
							realdev_cb->hw_vlan_filter.default_acl_index[i] = -1;
						}

						for(i=0 ; i<SMUX_HW_VLAN_FILTER_NUM ; i++)
						{
							if(realdev_cb->hw_vlan_filter.entry[i].valid == 1)
							{
								smux_set_hw_vlan_filter_acl_del(realdev_cb->hw_vlan_filter.entry[i].acl_index);
								memset(&realdev_cb->hw_vlan_filter.entry[i], 0, sizeof(struct smux_hw_vlan_filter_entry_t));
								realdev_cb->hw_vlan_filter.entry[i].acl_index = -1;
							}
						}
					}
					realdev_cb->hw_vlan_filter.mode = 0;
				}
			}
			break;
		case SMUX_MC_HW_VLAN_FILTER_METHOD_ENABLE:	
			
			if(!smux_configure.hw_vlan_filter)
			{
				SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s %d: global not enabled !\n", __func__, __LINE__);
				return -1;
			}
			
			list_for_each_entry_safe(realdev_cb, tmp, &real_dev_list, list_real) {
				if(!(smux_priv_flags_get(realdev_cb->realdev,PRIV_VSMUX)) && realdev_cb->realdev == hw_vlan_filter_p->dev)
				{
					if(realdev_cb->hw_vlan_filter.mode == 0)
					{
						realdev_cb->hw_vlan_filter.mode = hw_vlan_filter_p->mode;
						if(smux_set_hw_vlan_filter_acl_add(realdev_cb, 0, 1))
						{
							return -1;
						}						
					}
					else
					{
						for(i=0 ; i<SMUX_HW_VLAN_FILTER_DEFAULT_NUM ; i++)
						{
							smux_set_hw_vlan_filter_acl_del(realdev_cb->hw_vlan_filter.default_acl_index[i]);
							realdev_cb->hw_vlan_filter.default_acl_index[i] = -1;
						}						
						for(i=0 ; i<SMUX_HW_VLAN_FILTER_NUM ; i++)
						{
							if(realdev_cb->hw_vlan_filter.entry[i].valid == 1)
							{
								smux_set_hw_vlan_filter_acl_del(realdev_cb->hw_vlan_filter.entry[i].acl_index);
								realdev_cb->hw_vlan_filter.entry[i].acl_index = -1;
							}
						}
						realdev_cb->hw_vlan_filter.mode |= hw_vlan_filter_p->mode;
						if(smux_set_hw_vlan_filter_acl_add(realdev_cb, 0, 1))
						{
							return -1;
						}
						for(i=0 ; i<SMUX_HW_VLAN_FILTER_NUM ; i++)
						{
							if(realdev_cb->hw_vlan_filter.entry[i].valid == 1)
							{
								realdev_cb->hw_vlan_filter.entry[i].acl_index = smux_set_hw_vlan_filter_acl_add(realdev_cb, realdev_cb->hw_vlan_filter.entry[i].vlan_id, 0);
							}
						}
					}
				}
			}
			break;
		case SMUX_MC_HW_VLAN_FILTER_METHOD_DISABLE:

			if(!smux_configure.hw_vlan_filter)
			{
				SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s %d: global not enabled !\n", __func__, __LINE__);
				return -1;
			}
			
			list_for_each_entry_safe(realdev_cb, tmp, &real_dev_list, list_real) {
				if(!smux_priv_flags_get(realdev_cb->realdev,PRIV_VSMUX) && realdev_cb->realdev == hw_vlan_filter_p->dev)
				{
					if(realdev_cb->hw_vlan_filter.mode)
					{
						for(i=0 ; i<SMUX_HW_VLAN_FILTER_DEFAULT_NUM ; i++)
						{
							smux_set_hw_vlan_filter_acl_del(realdev_cb->hw_vlan_filter.default_acl_index[i]);
							realdev_cb->hw_vlan_filter.default_acl_index[i] = -1;
						}	
						for(i=0 ; i<SMUX_HW_VLAN_FILTER_NUM ; i++)
						{
							if(realdev_cb->hw_vlan_filter.entry[i].valid == 1)
							{
								smux_set_hw_vlan_filter_acl_del(realdev_cb->hw_vlan_filter.entry[i].acl_index);
								realdev_cb->hw_vlan_filter.entry[i].acl_index = -1;
								if(!(realdev_cb->hw_vlan_filter.mode & ~(hw_vlan_filter_p->mode)))
								{
									realdev_cb->hw_vlan_filter.entry[i].valid = 0;
									realdev_cb->hw_vlan_filter.entry[i].vlan_id = 0;
									realdev_cb->hw_vlan_filter.entry[i].ref_count = 0;
								}
							}
						}

						realdev_cb->hw_vlan_filter.mode &= ~(hw_vlan_filter_p->mode);
						if(realdev_cb->hw_vlan_filter.mode)
						{
							if(smux_set_hw_vlan_filter_acl_add(realdev_cb, 0, 1))
							{
								return -1;
							}
							for(i=0 ; i<SMUX_HW_VLAN_FILTER_NUM ; i++)
							{
								if(realdev_cb->hw_vlan_filter.entry[i].valid == 1)
								{
									realdev_cb->hw_vlan_filter.entry[i].acl_index = smux_set_hw_vlan_filter_acl_add(realdev_cb, realdev_cb->hw_vlan_filter.entry[i].vlan_id, 0);
								}
							}
						}
					}
				}
			}
			break;
		case SMUX_MC_HW_VLAN_FILTER_METHOD_ADD_BY_RULE:
		case SMUX_MC_HW_VLAN_FILTER_METHOD_ADD:
			
			if(!smux_configure.hw_vlan_filter)
			{
				SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s %d: global not enabled !\n", __func__, __LINE__);
				return -1;
			}
			
			list_for_each_entry_safe(realdev_cb, tmp, &real_dev_list, list_real) {
				if((!smux_priv_flags_get(realdev_cb->realdev,PRIV_VSMUX)) && realdev_cb->realdev == hw_vlan_filter_p->dev)
				{
					if(!realdev_cb->hw_vlan_filter.mode)
					{
						SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s %d: interface=%s not enabled !\n", __func__, __LINE__, hw_vlan_filter_p->dev->name);
						return -1;
					}
					
					for(i=0 ; i<SMUX_HW_VLAN_FILTER_NUM ; i++)
					{
						if(realdev_cb->hw_vlan_filter.entry[i].valid 
							&& realdev_cb->hw_vlan_filter.entry[i].vlan_id == hw_vlan_filter_p->vlan_id
							&& realdev_cb->realdev == hw_vlan_filter_p->dev)
						{
							SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s %d: rule exist !\n", __func__, __LINE__);
							if(method == SMUX_MC_HW_VLAN_FILTER_METHOD_ADD_BY_RULE)
							{
								realdev_cb->hw_vlan_filter.entry[i].ref_count++;
							}
							return 0;
						}
					}
					for(i=0 ; i<SMUX_HW_VLAN_FILTER_NUM ; i++)
					{
						if(!realdev_cb->hw_vlan_filter.entry[i].valid)
						{
							realdev_cb->hw_vlan_filter.entry[i].vlan_id = hw_vlan_filter_p->vlan_id;
							realdev_cb->hw_vlan_filter.entry[i].acl_index = smux_set_hw_vlan_filter_acl_add(realdev_cb, hw_vlan_filter_p->vlan_id, 0);
							if(realdev_cb->hw_vlan_filter.entry[i].acl_index == -1)
							{								
								realdev_cb->hw_vlan_filter.entry[i].valid = 0;
								return -1;
							}
							else
							{
								realdev_cb->hw_vlan_filter.entry[i].valid = 1;
								if(method == SMUX_MC_HW_VLAN_FILTER_METHOD_ADD_BY_RULE)
								{
									realdev_cb->hw_vlan_filter.entry[i].ref_count++;
								}
							}
							break;
						}
					}
				}
			}
			break;
		case SMUX_MC_HW_VLAN_FILTER_METHOD_DEL_BY_RULE:
		case SMUX_MC_HW_VLAN_FILTER_METHOD_DEL:
			
			if(!smux_configure.hw_vlan_filter)
			{
				SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s %d: global not enabled !\n", __func__, __LINE__);
				return -1;
			}
			
			list_for_each_entry_safe(realdev_cb, tmp, &real_dev_list, list_real) {
				if((!smux_priv_flags_get(realdev_cb->realdev,PRIV_VSMUX)) && realdev_cb->realdev == hw_vlan_filter_p->dev)
				{
					if(!realdev_cb->hw_vlan_filter.mode)
					{
						SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s %d: interface=%s not enabled !\n", __func__, __LINE__, hw_vlan_filter_p->dev->name);
						return -1;
					}
					
					for(i=0 ; i<SMUX_HW_VLAN_FILTER_NUM ; i++)
					{
						if(realdev_cb->hw_vlan_filter.entry[i].valid 
							&& realdev_cb->hw_vlan_filter.entry[i].vlan_id == hw_vlan_filter_p->vlan_id
							&& realdev_cb->realdev == hw_vlan_filter_p->dev)
						{
							if(method == SMUX_MC_HW_VLAN_FILTER_METHOD_DEL_BY_RULE)
							{
								realdev_cb->hw_vlan_filter.entry[i].ref_count--;
								if(!realdev_cb->hw_vlan_filter.entry[i].ref_count)
								{
									smux_set_hw_vlan_filter_acl_del(realdev_cb->hw_vlan_filter.entry[i].acl_index);
									realdev_cb->hw_vlan_filter.entry[i].valid = 0;
								}
							}
							else
							{
								smux_set_hw_vlan_filter_acl_del(realdev_cb->hw_vlan_filter.entry[i].acl_index);
								realdev_cb->hw_vlan_filter.entry[i].valid = 0;
							}
						}
					}
				}
			}
			break;
		case SMUX_MC_HW_VLAN_FILTER_METHOD_REFRESH:
			
			if(!smux_configure.hw_vlan_filter)
			{
				SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s %d: global not enabled !", __func__, __LINE__);
				return -1;
			}
			
			list_for_each_entry_safe(realdev_cb, tmp, &real_dev_list, list_real) {
				if(!smux_priv_flags_get(realdev_cb->realdev,PRIV_VSMUX))
				{
					if(hw_vlan_filter_p->dev && realdev_cb->realdev != hw_vlan_filter_p->dev)
					{
						continue;
					}

#if defined(CONFIG_RTL8686NIC) && defined(CONFIG_RTL9607C_SERIES)
					realdev_cb->hw_vlan_filter.port_mask = (((struct re_dev_private*)netdev_priv(realdev_cb->realdev))->txPortMask);
#endif	
					if(realdev_cb->hw_vlan_filter.mode)
					{
						for(i=0 ; i<SMUX_HW_VLAN_FILTER_DEFAULT_NUM ; i++)
						{
							smux_set_hw_vlan_filter_acl_del(realdev_cb->hw_vlan_filter.default_acl_index[i]);
							realdev_cb->hw_vlan_filter.default_acl_index[i] = -1;
						}	
						for(i=0 ; i<SMUX_HW_VLAN_FILTER_NUM ; i++)
						{
							if(realdev_cb->hw_vlan_filter.entry[i].valid == 1)
							{
								smux_set_hw_vlan_filter_acl_del(realdev_cb->hw_vlan_filter.entry[i].acl_index);
								realdev_cb->hw_vlan_filter.entry[i].acl_index = -1;
							}
						}
						if(smux_set_hw_vlan_filter_acl_add(realdev_cb, 0, 1))
						{
							return -1;
						}
						for(i=0 ; i<SMUX_HW_VLAN_FILTER_NUM ; i++)
						{
							if(realdev_cb->hw_vlan_filter.entry[i].valid == 1)
							{
								realdev_cb->hw_vlan_filter.entry[i].acl_index = smux_set_hw_vlan_filter_acl_add(realdev_cb, realdev_cb->hw_vlan_filter.entry[i].vlan_id, 0);
							}
						}
					}
				}
			}
			break;
		default:
			return -1;
	}
	
	return 0;
}

static void smux_add_hw_vlan_filter_by_rules(struct smux_realdev_cb_t  *realdev_cb, int depth, struct smux_rule_t *rule)
{
	struct smux_realdev_cb_t *realdev_cb_tmp, *tmp;
	struct smux_dev_priv *priv;
	int isRealDevFound = 0;
	
	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s enter dev=%s depth=%d enf=%d fvid=%d", __func__, realdev_cb->realdev->name, depth, rule->filters.en_filter&(1<<SMUX_FLT_VTAGS), rule->filters.vtags[depth].vlan_tci_mask&VID_MASK);
	if(rule->filters.en_filter&(1<<SMUX_FLT_VTAGS) 
		&& rule->filters.vtags[depth].vlan_tci_mask&VID_MASK)
	{
		struct smux_hw_vlan_filter_para_t mc_hw_vlan_filter_t = {0};
		if(smux_priv_flags_get(realdev_cb->realdev,PRIV_VSMUX))
		{
			list_for_each_entry_safe(realdev_cb_tmp, tmp, &real_dev_list, list_real) {
				list_for_each_entry_rcu(priv, &realdev_cb_tmp->list_virt, list_virt) {
					if(!strcmp(priv->virt_dev->name, realdev_cb->realdev->name))
					{
						mc_hw_vlan_filter_t.dev = realdev_cb_tmp->realdev;						
						SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "isRealDevFound=1 dev=%s", realdev_cb_tmp->realdev->name);
						isRealDevFound = 1;
					}
				}
			}
		}
		else
		{
			mc_hw_vlan_filter_t.dev = realdev_cb->realdev;			
			SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "isRealDevFound=1 dev=%s", realdev_cb->realdev->name);
			isRealDevFound = 1;
		}

		if(isRealDevFound)
		{
			mc_hw_vlan_filter_t.vlan_id = ntohs(rule->filters.vtags[depth].vlan_tci&VID_MASK);
			smux_set_hw_vlan_filter(SMUX_MC_HW_VLAN_FILTER_METHOD_ADD_BY_RULE, &mc_hw_vlan_filter_t);
		}
	}
}

static void smux_del_hw_vlan_filter_by_rules(struct smux_realdev_cb_t  *realdev_cb, int depth, struct smux_rule_t *rule)
{
	struct smux_realdev_cb_t *realdev_cb_tmp, *tmp;
	struct smux_dev_priv *priv;
	int isRealDevFound = 0;

	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s enter dev=%s depth=%d enf=%d fvid=%d", __func__, realdev_cb->realdev->name, depth, rule->filters.en_filter&(1<<SMUX_FLT_VTAGS), rule->filters.vtags[depth].vlan_tci_mask&VID_MASK);
	if(rule->filters.en_filter&(1<<SMUX_FLT_VTAGS) 
		&& rule->filters.vtags[depth].vlan_tci_mask&VID_MASK)
	{
		struct smux_hw_vlan_filter_para_t mc_hw_vlan_filter_t = {0};
		if(smux_priv_flags_get(realdev_cb->realdev,PRIV_VSMUX))
		{
			list_for_each_entry_safe(realdev_cb_tmp, tmp, &real_dev_list, list_real) {
				list_for_each_entry_rcu(priv, &realdev_cb_tmp->list_virt, list_virt) {
					if(!strcmp(priv->virt_dev->name, realdev_cb->realdev->name))
					{
						mc_hw_vlan_filter_t.dev = realdev_cb_tmp->realdev;
						SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "isRealDevFound=1 dev=%s", realdev_cb_tmp->realdev->name);
						isRealDevFound = 1;
					}
				}
			}
		}
		else
		{
			mc_hw_vlan_filter_t.dev = realdev_cb->realdev;
			SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "isRealDevFound=1 dev=%s", realdev_cb->realdev->name);
			isRealDevFound = 1;
		}

		if(isRealDevFound)
		{
			mc_hw_vlan_filter_t.vlan_id = ntohs(rule->filters.vtags[depth].vlan_tci&VID_MASK);
			SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "vlan_id=%d", mc_hw_vlan_filter_t.vlan_id);
			smux_set_hw_vlan_filter(SMUX_MC_HW_VLAN_FILTER_METHOD_DEL_BY_RULE, &mc_hw_vlan_filter_t);
		}
	}
}

static void smux_disable_hw_vlan_filter_by_if_delete(struct smux_realdev_cb_t  *realdev_cb)
{
	struct smux_realdev_cb_t *realdev_cb_tmp, *tmp;
	struct smux_dev_priv *priv;
	int isRealDevFound = 0;	
	struct smux_hw_vlan_filter_para_t mc_hw_vlan_filter_t = {0};

	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s enter dev=%s", __func__, realdev_cb->realdev->name);
	if(smux_priv_flags_get(realdev_cb->realdev,PRIV_VSMUX))
	{
		list_for_each_entry_safe(realdev_cb_tmp, tmp, &real_dev_list, list_real) {
			list_for_each_entry_rcu(priv, &realdev_cb_tmp->list_virt, list_virt) {
				if(!strcmp(priv->virt_dev->name, realdev_cb->realdev->name))
				{
					mc_hw_vlan_filter_t.dev = realdev_cb_tmp->realdev;
					SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "isRealDevFound=1 dev=%s", realdev_cb_tmp->realdev->name);
					isRealDevFound = 1;
				}
			}
		}
	}
	else
	{
		mc_hw_vlan_filter_t.dev = realdev_cb->realdev;
		SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "isRealDevFound=1 dev=%s", realdev_cb->realdev->name);
		isRealDevFound = 1;
	}

	if(isRealDevFound)
	{
		mc_hw_vlan_filter_t.mode = 0x7;
		smux_set_hw_vlan_filter(SMUX_MC_HW_VLAN_FILTER_METHOD_DISABLE, &mc_hw_vlan_filter_t);
	}
}
#endif
#endif

#if 0 
static int smuxdev_open(struct inode *inode, struct file *file)
{
    pr_info("%s:\n", __func__);
    return 0;
}

static int smuxdev_close(struct inode *inodep, struct file *filp)
{
    pr_info("%s:\n", __func__);
    return 0;
}

static ssize_t smuxdev_write(struct file *file, const char __user *buf,
		       size_t len, loff_t *ppos)
{
    pr_info("%s: consumed %d bytes\n", __func__,len);
    return len; /* But we don't actually do anything with the data */
}
#endif 

static rx_handler_result_t smux_receive_skb(struct sk_buff **pskb);

#if SMUX_TEST

static void* build_ioctl_test(struct sk_buff *skb, struct smux_ioctl_test_s *test) {
	//struct smux_ioctl_test_s *test;
	struct smux_ioctl_test_skbinfo_s *si = &test->skbinfo;
	
	//if (skb_headroom(skb)<offsetof(struct smux_ioctl_test_s, payload)) {
	//	printk("%s(%d): failed to build test_s, %d needed %d available\n",__func__,__LINE__,
	//		offsetof(struct smux_ioctl_test_s, payload),skb_headroom(skb));
	//	return NULL;
	//}
	//test = (struct smux_ioctl_test_s *)(skb->data - offsetof(struct smux_ioctl_test_s, payload));
	si = &test->skbinfo;
	si->out_skb = skb;
	si->mark = skb->mark;
	si->priority = skb->priority;
	strncpy(si->devname, skb->dev->name, IFNAMSIZ);
	test->payload_len = skb->len;
	memcpy(test->payload, skb->data, test->payload_len);
	return (void *)test;
}


static long ioctl_handle_test(struct file *file, unsigned int cmd, unsigned long arg) {
	struct smux_ioctl_test_s *test;
	struct sk_buff *skb;
	struct net_device *dev;
	struct smux_ioctl_test_skbinfo_s skbinfo;	
	long ret;
	rx_handler_result_t result;
	
	skb = dev_alloc_skb(sizeof(*test));
	if (!skb)
		return -ENOMEM;
	
	test = skb->data;
	
	copy_from_user(test, (struct smux_ioctl_test_s *)arg, sizeof(*test));
	memcpy(&skbinfo, &test->skbinfo, sizeof(skbinfo));
	
	/* prepare skb */
	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s: name=%s, dir=%d len=%d flag=%x", __func__, test->skbinfo.devname, test->dir, test->payload_len, test->flags);
	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "000skb:%p,%p,%p,%p,len=%d", skb->head,skb->data,skb->tail,skb->end,skb->len); 	
	dev = dev_get_by_name(&init_net, skbinfo.devname);
	if (!dev) {
		ret = -ENODEV;
		goto out_free_skb;
	}
	
	skbinfo.in_skb = skb;
	skb->mark = skbinfo.mark;
	skb->priority = skbinfo.priority;
	skb->len = test->payload_len;
	skb->data = test->payload;
	skb->tail = skb->data + skb->len;
	skb->dev = dev;
	skb_reset_mac_header(skb);
	
	if (test->dir == SMUX_DIR_RX) {	
		result = smux_receive_skb(&skb);	
		
		/* result */
		test = (struct smux_ioctl_test_s *)(skb->data - offsetof(struct smux_ioctl_test_s, payload));
		memcpy(&test->skbinfo, &skbinfo, sizeof(skbinfo));
		test->skbinfo.out_skb = skb;
		test->skbinfo.mark = skb->mark;
		test->skbinfo.priority = skb->priority;
		strncpy(test->skbinfo.devname, skb->dev->name, IFNAMSIZ);
		test->result = result;
		test->payload_len = skb->len;
		copy_to_user((struct smux_ioctl_test_s *)arg, test, sizeof(*test));
	} else {		
		struct test_cb *tcb = skb->cb;
		test = kmalloc(sizeof(struct smux_ioctl_test_s),__GFP_ATOMIC);
		if (test) {
			tcb->key = TEST_MAJIC;
			tcb->ptr = test;
			result = smux_dev_hard_start_xmit(skb, dev);
			test->result = result;
			copy_to_user((struct smux_ioctl_test_s *)arg, test, sizeof(*test));
			kfree(test);
		} else {
			ret = -ENOMEM;
		}		
	}
	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "001skb:%p,%p,%p,%p,len=%d", skb->head,skb->data,skb->tail,skb->end,skb->len);	
	dev_put(dev);
	kfree_skb(skb);
	return 0;
	
out_free_skb:	
	kfree_skb(skb);
	
	return ret;
}
#endif

static long smuxdev_ioctl (struct file *file, unsigned int cmd, unsigned long arg) 
{
	//struct socket *sock;
	//struct sock *sk;
	struct smux_interface_s smuxif;
	struct smux_ioctl_rule_t iocrule;
	struct net_device *dev;	
	long ret = 0;
	int dir,dir_max,tags,tags_max;
	
	//printk("%s(%d):%p\n",__func__,__LINE__,file->private_data);
	//sock = file->private_data;
	//printk("%s(%d):%p %p\n",__func__,__LINE__,sock,sock->sk);
	//sk = sock->sk;
	//net = sock_net(sk);
	
	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s(%d): cmd=%u",__func__,__LINE__ ,cmd);

	switch (cmd) {
	case SMUXCTL_CMD_IF_CREATE:
		rtnl_lock();
		copy_from_user(&smuxif, (struct smux_interface_s *)arg, sizeof(smuxif));
		dev = get_virt_dev(NULL, smuxif.real_name);
		SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s(%d):%s, %p",__func__,__LINE__,smuxif.real_name,dev);
		if (!dev) {			
			ret = -ENODEV;
			rtnl_unlock();
			goto out;
		}
		ret = smux_interface_create(dev, smuxif.virt_name, &smuxif);
		rtnl_unlock();
		break;
	case SMUXCTL_CMD_IF_DELETE:
		rtnl_lock();
		copy_from_user(&smuxif, (struct smux_interface_s *)arg, sizeof(smuxif));
		dev = get_virt_dev(NULL, smuxif.virt_name);
		if (!dev || !(smux_priv_flags_get(dev,PRIV_VSMUX))) {
			ret = -ENODEV;
			rtnl_unlock();
			goto out;
		}
		ret = smux_interface_delete(dev);		
		rtnl_unlock();
		break;
	case SMUXCTL_CMD_SET_IF_PARM:
		rtnl_lock();
		copy_from_user(&smuxif, (struct smux_interface_s *)arg, sizeof(smuxif));
		dev = get_virt_dev(NULL, smuxif.real_name);
		if (!dev) {
			ret = -ENODEV;
			rtnl_unlock();
			goto out;
		}
		ret = smux_interface_parameter_set(dev, &smuxif);		
		rtnl_unlock();
		break;
	case SMUXCTL_CMD_RULE_ADD:	
		copy_from_user(&iocrule, (struct smux_ioctl_rule_t *)arg, sizeof(iocrule));
		
		ret = smux_rule_add(iocrule.table.ifname, iocrule.table.dir, iocrule.table.nr_tag,
					&iocrule.rule, iocrule.method, &iocrule.id);
		if(ret == 0) {
			SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "K:id=%d",iocrule.id);
			copy_to_user((struct smux_ioctl_rule_t *)arg, &iocrule, offsetof(struct smux_ioctl_rule_t, id) + sizeof(iocrule.id));
		}
		
		break;
	case SMUXCTL_CMD_RULE_DELETE:
		copy_from_user(&iocrule, (struct smux_ioctl_rule_t *)arg, sizeof(iocrule));
		
		ret = smux_rule_del(iocrule.table.ifname, iocrule.table.dir, iocrule.table.nr_tag,
				 iocrule.method, &iocrule.id, iocrule.arg);
		//if(ret == 0) {
		//	printk("K:id=%d\n",iocrule.id);
		//	copy_to_user((struct smux_ioctl_rule_t *)arg, &iocrule, offsetof(struct smux_ioctl_rule_t, id) + sizeof(iocrule.id));
		//}
		
		break;	
	case SMUXCTL_CMD_RULE_RESET:
		SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "K: in SMUXCTL_CMD_RULE_RESET");
		copy_from_user(&iocrule, (struct smux_ioctl_rule_t *)arg, sizeof(iocrule));
		
		if(iocrule.table.ifname[0] == '\0'){
			ret = -ENODEV;
			goto out;
		}
		
		if(iocrule.table.dir >= SMUX_DIR_MAX){
			dir = 0;
			dir_max = SMUX_DIR_MAX-1;
		}
		else{
			dir_max = dir = iocrule.table.dir;
		}
		for(;dir <= dir_max; dir++){
			if(iocrule.table.nr_tag < 0){
				tags = 0;
				tags_max = SMUX_MAX_TAGS;
			}
			else{
				tags_max = tags = iocrule.table.nr_tag;
			}
			for(;tags <= tags_max; tags++){
				ret = smux_reset_table_count(iocrule.table.ifname, dir, tags);
				if(ret != 0) goto out;
			}
		}
		break;
		
	case SMUXCTL_CMD_SHOW_TABLE:
		SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "K: in SMUXCTL_CMD_SHOW_TABLE");
		copy_from_user(&iocrule, (struct smux_ioctl_rule_t *)arg, sizeof(iocrule));

		if(iocrule.table.ifname[0] == '\0'){
			ret = -ENODEV;
			goto out;
		}
		
		if(iocrule.table.dir >= SMUX_DIR_MAX){
			dir = 0;
			dir_max = SMUX_DIR_MAX-1;
		}
		else{
			dir_max = dir = iocrule.table.dir;
		}
		for(;dir <= dir_max; dir++){
			if(iocrule.table.nr_tag < 0){
				tags = 0;
				tags_max = SMUX_MAX_TAGS;
			}
			else{
				tags_max = tags = iocrule.table.nr_tag;
			}
			for(;tags <= tags_max; tags++){
				ret = smux_show_table(iocrule.table.ifname, dir, tags);
				if(ret != 0) goto out;
			}
		}
		break;	
		
	case SMUXCTL_CMD_SET_DEFAULT:
		SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "K: in SMUXCTL_CMD_SET_DEFAULT");
		copy_from_user(&iocrule, (struct smux_ioctl_rule_t *)arg, sizeof(iocrule));
		
		ret = smux_set_default_value(iocrule.table.ifname, iocrule.table.dir, iocrule.table.nr_tag, iocrule.method, iocrule.table.default_tpid, iocrule.table.default_vlan_tci);
		
		break;	

	case SMUXCTL_CMD_SET_QOS:
		SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "K: in SMUXCTL_CMD_SET_QOS");
		copy_from_user(&iocrule, (struct smux_ioctl_rule_t *)arg, sizeof(iocrule));
		
		ret = smux_set_qos(iocrule.table.ifname, iocrule.method, &iocrule.qos);
		
		break;	
	#if SMUX_TEST	
	case SMUXCTL_CMD_TEST:
		ret = ioctl_handle_test(file, cmd, arg);
		break;
	#endif	
	}
out:
	
	return ret;
}

static inline int is_valid_tpid(__be16 tpid, struct smux_realdev_cb_t *dev_cb) {
	int i;
	if(dev_cb)
	{
		for (i=0;i<SMUX_MAX_TAGS;i++) {
			if (!dev_cb->tpids[i])
				break;
			if (dev_cb->tpids[i]==tpid)
				return 1;
		}
	}
	else
	{
		if(tpid == ntohs(0x8100) || tpid == ntohs(0x88a8) || tpid == ntohs(0x9100))
			return 1;
	}
	return 0;
}

static int smux_adjust_skb(struct sk_buff *skb, struct smux_pkt_info_t *info)
{
	__be16 proto;
	struct ethhdr *eth;
	struct smux_vlan_tag_t *tag;

	eth = (struct ethhdr *)skb_mac_header(skb);
	tag = (struct smux_vlan_tag_t *)((char *)eth + (ETH_ALEN*2));
	
	if (likely(eth_proto_is_802_3(tag->vlan_proto)))
	{
		proto = tag->vlan_proto;
	}
	else
	{
		unsigned short _service_access_point;
		const unsigned short *sap;
		sap = skb_header_pointer(skb, 0, sizeof(*sap), &_service_access_point);
		if (sap && *sap == 0xFFFF)
			proto = htons(ETH_P_802_3);
		else
			proto = htons(ETH_P_802_2);
	}	
	if(skb->protocol != proto) {
		SMUX_DBG(info->debug_level_idx, info, "[%s] change SKB(%p) protocol form 0x%04x to 0x%04x  !!", __func__, skb, ntohs(skb->protocol), ntohs(proto));
		skb->protocol = proto;
	}
	return 0;
}

static int smux_update_pkt_info(struct sk_buff *skb, struct smux_pkt_info_t *info)
{
	__be16 proto; 
	struct ethhdr *eth = (struct ethhdr *)skb_mac_header(skb);
	struct smux_vlan_tag_t *tag;

	tag = (struct smux_vlan_tag_t *)((char *)eth + (ETH_ALEN*2));
	info->vtags = tag;
	
	if(info->vtag_count > 0)
		tag = tag + info->vtag_count;
	
	if (likely(eth_proto_is_802_3(tag->vlan_proto)))
	{
		proto = ntohs(tag->vlan_proto);
	}
	else
	{
		unsigned short _service_access_point;
		const unsigned short *sap;
		  /*
		  *   This is a magic hack to spot IPX packets. Older Novell breaks
		  *   the protocol design and runs IPX over 802.3 without an 802.2 LLC
		  *   layer. We look for FFFF which isn't a used 802.2 SSAP/DSAP. This
		  *   won't work for fault tolerant netware but does for the rest.
		  */
		sap = skb_header_pointer(skb, 0, sizeof(*sap), &_service_access_point);
		if (sap && *sap == 0xFFFF)
			proto = ETH_P_802_3;
		else
			proto = ETH_P_802_2;
	}
	
	info->ethertype = proto;
	info->iphdr = NULL;
	info->ipv6hdr = NULL;
	
	if (proto==ETH_P_IP) {
		//info->ip_offset = (u16) ((unsigned char *)&tag[1] - skb->head);
		info->iphdr = (struct iphdr *)(((char*)tag)+2);
	} else if (proto==ETH_P_IPV6) {
		info->ipv6hdr = (struct ipv6hdr *)(((char*)tag)+2);
	} else if (proto==ETH_P_PPP_SES) {
		struct pppoe_hdr *pppoe;
		pppoe = (struct pppoe_hdr *)(((char*)tag)+2);
		if (pppoe->code == 0) { // Session data
			struct smux_ppp_header_t *ppp = (struct smux_ppp_header_t *)pppoe->tag;
			if(ppp->ppp_proto == htons(PPP_IP)) {
				info->iphdr = ((struct iphdr *)(&ppp->payload[0]));
			} else if(ppp->ppp_proto == htons(PPP_IPV6)) {
				SMUX_DBG(info->debug_level_idx, info, "%s(%d) PPP_IPV6 hit !! ", __func__,__LINE__);
				info->ipv6hdr = ((struct ipv6hdr *)(&ppp->payload[0]));
			}
		}		
	}
	
	info->org_vtag_count = skb->original_vlan_count;
	info->org_vtags = ((struct smux_vlan_tag_t *)(&skb->original_vlan_header[0]));
	
	return 0;
}

static int smux_build_cb(struct sk_buff *skb, struct smux_realdev_cb_t *dev_cb, struct smux_pkt_info_t *info) {
	int i, j, k, match;	
	struct ethhdr *eth = (struct ethhdr *)skb_mac_header(skb);
	struct smux_vlan_tag_t *tag;
	traceFilterRule *f = NULL;
	
	info->vtag_count = 0;
	tag = (struct smux_vlan_tag_t *)((char *)eth + (ETH_ALEN*2));
	info->filter_pkt = skb->smux_filter_pkt;

	/*SMUX_DBG(info->debug_level_idx, info, "%s(%d) tag=%p ", __func__,__LINE__,tag);
	SMUX_DBG(info->debug_level_idx, info, "%s(%d) tag->vlan_proto=0x%x ", __func__,__LINE__,tag->vlan_proto);
	SMUX_DBG(info->debug_level_idx, info, "%s(%d) tag->vlan_tci=0x%x ", __func__,__LINE__,tag->vlan_tci);*/
	while (is_valid_tpid(tag->vlan_proto, dev_cb)) {
		tag++;
		/*SMUX_DBG(info->debug_level_idx, info, "%s(%d) tag->vlan_proto=0x%x ", __func__,__LINE__,tag->vlan_proto);
		SMUX_DBG(info->debug_level_idx, info, "%s(%d) tag->vlan_tci=0x%x ", __func__,__LINE__,tag->vlan_tci);*/
		info->vtag_count++;
	}
	
	if(skb->original_vlan_count == 0) {
		if(info->vtag_count > 0) {
			skb->original_vlan_count = info->vtag_count;
			tag = (struct smux_vlan_tag_t *)((char *)eth + (ETH_ALEN*2));
			memcpy(skb->original_vlan_header, tag, (skb->original_vlan_count * VLAN_HLEN));
		}
		else {
			skb->original_vlan_count = -1;
		}
	}

	smux_update_pkt_info(skb, info);
	
	if(smux_configure.trace_filter && smux_debug && info->filter_pkt == 0)
	{
		for(i=0;i<SMUX_TRACE_FILETR_RULES_MAX;i++)
		{
			f = &(smux_configure.trace_filter_rules[i]);
			match = 0;
			if(f->bits == 0) 
				match = 0;
			else if( !(f->bits & 1<<SMUX_TRACE_FILETR_TIMES) || f->times > 0 ){
				match = 1;
				for(j=0;j<SMUX_TRACE_FILETR_MAX;j++)
				{
					if(!(f->bits & (1<<j))) continue;
					if(j == SMUX_TRACE_FILETR_IF){
						if(strcmp(skb->dev->name, f->ifname)){
							match = 0;
						}
					}
					else if(j == SMUX_TRACE_FILETR_SA){
						eth = (struct ethhdr *)skb_mac_header(skb);
						for(k=0;k<ETH_ALEN;k++){
							if((eth->h_source[k] & f->sa_mask[k]) != (f->sa[k] & f->sa_mask[k])){
								match = 0;
							}
						}
					}
					else if(j == SMUX_TRACE_FILETR_DA){
						eth = (struct ethhdr *)skb_mac_header(skb);
						for(k=0;k<ETH_ALEN;k++){
							if((eth->h_dest[k] & f->da_mask[k]) != (f->da[k] & f->da_mask[k])){
								match = 0;
							}
						}
					}
					else if(j == SMUX_TRACE_FILETR_ETH){
						if(info->ethertype != f->eth_proto){
							match = 0;
							break;
						}
					}
					else if(j == SMUX_TRACE_FILETR_TPID1 || j == SMUX_TRACE_FILETR_VID1){
						if(info->vtag_count >= 1){
							struct smux_vlan_tag_t *v = &(info->vtags[info->vtag_count-1]);
							struct smux_vlan_tag_t *fv = &(f->vlan[0]);
							if((j == SMUX_TRACE_FILETR_TPID1 && fv->vlan_proto != v->vlan_proto)
								|| (j == SMUX_TRACE_FILETR_VID1 && (fv->vlan_tci&VID_MASK) != (v->vlan_tci&VID_MASK)))
							{
								match = 0;
							}
						}
						else match = 0;
					}
					else if(j == SMUX_TRACE_FILETR_TPID2 || j == SMUX_TRACE_FILETR_VID2){
						if(info->vtag_count >= 2){
							struct smux_vlan_tag_t *v = &(info->vtags[info->vtag_count-2]);
							struct smux_vlan_tag_t *fv = &(f->vlan[1]);
							if((j == SMUX_TRACE_FILETR_TPID2 && fv->vlan_proto != v->vlan_proto)
								|| (j == SMUX_TRACE_FILETR_VID2 && (fv->vlan_tci&VID_MASK) != (v->vlan_tci&VID_MASK)))
							{
								match = 0;
							}
						}
						else match = 0;
					}
					else if(j == SMUX_TRACE_FILETR_TPID3 || j == SMUX_TRACE_FILETR_VID3){
						if(info->vtag_count >= 3){
							struct smux_vlan_tag_t *v = &(info->vtags[info->vtag_count-3]);
							struct smux_vlan_tag_t *fv = &(f->vlan[2]);
							if((j == SMUX_TRACE_FILETR_TPID3 && fv->vlan_proto != v->vlan_proto)
								|| (j == SMUX_TRACE_FILETR_VID3 && (fv->vlan_tci&VID_MASK) != (v->vlan_tci&VID_MASK)))
							{
								match = 0;
							}
						}
						else match = 0;
					}
					else if(j == SMUX_TRACE_FILETR_TPID4 || j == SMUX_TRACE_FILETR_VID4){
						if(info->vtag_count >= 4){
							struct smux_vlan_tag_t *v = &(info->vtags[info->vtag_count-4]);
							struct smux_vlan_tag_t *fv = &(f->vlan[3]);
							if((j == SMUX_TRACE_FILETR_TPID4 && fv->vlan_proto != v->vlan_proto)
								|| (j == SMUX_TRACE_FILETR_VID4 && (fv->vlan_tci&VID_MASK) != (v->vlan_tci&VID_MASK)))
							{
								match = 0;
							}
						}
						else match = 0;
					}
					else if(j == SMUX_TRACE_FILETR_SIP){
						if(info->iphdr == NULL || (f->sip != info->iphdr->saddr)){
							match = 0;
						}
					}
					else if(j == SMUX_TRACE_FILETR_DIP){
						if(info->iphdr == NULL || (f->dip != info->iphdr->daddr)){
							match = 0;
						}
					}
					else if(j == SMUX_TRACE_FILETR_SIP6){
						if(info->ipv6hdr == NULL || memcmp(&info->ipv6hdr->saddr, &f->sip6, sizeof(struct in6_addr))){
							match = 0;
						}
					}
					else if(j == SMUX_TRACE_FILETR_DIP6){
						if(info->ipv6hdr == NULL || memcmp(&info->ipv6hdr->daddr, &f->dip6, sizeof(struct in6_addr))){
							match = 0;
						}
					}
					else if(j == SMUX_TRACE_FILETR_L4PROTO){
						if((info->iphdr && f->proto != info->iphdr->protocol)
							|| (info->ipv6hdr && f->proto != info->ipv6hdr->nexthdr)
							|| (!info->iphdr && !info->ipv6hdr))
						{
							match = 0;
						}
					}
					else if(j == SMUX_TRACE_FILETR_SPORT || j == SMUX_TRACE_FILETR_DPORT){
						if(info->iphdr){
							if(info->iphdr->protocol == IPPROTO_TCP){
								struct tcphdr *tcph = NULL;
								tcph = (struct tcphdr *)(((char*)(info->iphdr))+20);
								if((j == SMUX_TRACE_FILETR_SPORT && f->sport != tcph->source)
									|| (j == SMUX_TRACE_FILETR_DPORT && f->dport != tcph->dest))
									match = 0;
							}
							else if(info->iphdr->protocol == IPPROTO_UDP){
								struct udphdr *udph = NULL;
								udph = (struct udphdr *)(((char*)(info->iphdr))+20);
								if((j == SMUX_TRACE_FILETR_SPORT && f->sport != udph->source)
									|| (j == SMUX_TRACE_FILETR_DPORT && f->dport != udph->dest))
									match = 0;
							}
							else{
								match = 0;
							}
						}
						else if(info->ipv6hdr){
							if(info->ipv6hdr->nexthdr == NEXTHDR_TCP){
								struct tcphdr *tcph = NULL;
								tcph = (struct tcphdr *)(((char*)(info->ipv6hdr))+sizeof(struct ipv6hdr));
								if((j == SMUX_TRACE_FILETR_SPORT && f->sport != tcph->source)
									|| (j == SMUX_TRACE_FILETR_DPORT && f->dport != tcph->dest))
									match = 0;
							}
							else if(info->ipv6hdr->nexthdr == NEXTHDR_UDP){
								struct udphdr *udph = NULL;
								udph = (struct udphdr *)(((char*)(info->ipv6hdr))+sizeof(struct ipv6hdr));
								if((j == SMUX_TRACE_FILETR_SPORT && f->sport != udph->source)
									|| (j == SMUX_TRACE_FILETR_DPORT && f->dport != udph->dest))
									match = 0;
							}
							else{
								match = 0;
							}
						}
						else{
							match = 0;
						}
					}
					
					if(match == 0) break;
				}
			}
			if(match){
				info->filter_pkt = 1;
				skb->smux_filter_pkt = 1;
				if(f->times > 0) f->times--;
			}
		}
	}

	SMUX_DBG(info->debug_level_idx, info, "skb=%p, dev=%s, mark=0x%x"
#ifdef CONFIG_RTK_SKB_MARK2
				", mark2=0x%llx"
#endif
				, skb, skb->dev->name, skb->mark
#ifdef CONFIG_RTK_SKB_MARK2
				, skb->mark2
#endif
				);
	
	SMUX_DBG(info->debug_level_idx, info, "org_vtag_count=%d, vtag_count=%d, proto=0x%x, iphdr=%p, ipv6hdr=%p", info->org_vtag_count, info->vtag_count, info->ethertype, info->iphdr, info->ipv6hdr);
	
#ifdef CONFIG_COMMON_RT_API
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
	if (smux_configure.mapping_outermost_vlan_by_mac_from_lut)
	{
		int ret = RT_ERR_OK;
		rt_l2_ucastAddr_t l2UcAddr;
		
		SMUX_DBG(info->debug_level_idx, info, "%s(%d) skb->dmac = %pM, skb->smac = %pM !!", __func__,__LINE__, eth->h_dest, eth->h_source);

		if (eth->h_dest[0] & 0x1) {
			;//MCBC don't need to get l2UcAddr
		}
		else {
			memset(&l2UcAddr, 0, sizeof(l2UcAddr));
			/* l2UcAddr.flags = 0x0 <-- memset do this already */
			memcpy(&l2UcAddr.mac.octet, eth->h_dest, ETH_ALEN);
			ret = rt_l2_addr_get(&l2UcAddr);

			if (ret == RT_ERR_OK) {
				SMUX_DBG(info->debug_level_idx, info, "%s(%d) l2UcAddr.mac = %pM, l2UcAddr vid = %u !!", __func__,__LINE__, l2UcAddr.mac.octet, l2UcAddr.vid);
				//only support vid for outermost tag now
				info->vtags_inquire_by_lut.vlan_proto = GET_VID_BY_LUT;
				info->vtags_inquire_by_lut.vlan_tci = htons(l2UcAddr.vid);
			}
			/* If it can't get l2UcAddr info, default is UNKOWN_VID_BY_LUT. */
		}
	}
#endif
#endif

	return 0;
}

static int filter_sip(struct smux_rule_t *rule, struct smux_pkt_info_t *info) {
	int ret;
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);

	if(info->iphdr == NULL)
	{
		SMUX_DBG(info->debug_level_idx, info, "%s(%d): without IPv4 header !",__func__,__LINE__);
		return 0;
	}

	if(info->iphdr->saddr == rule->filters.sip)
		ret = 1;
	else
		ret = 0;
	
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): ret=%d",__func__,__LINE__,ret);
	return ret;
}

static int filter_dip(struct smux_rule_t *rule, struct smux_pkt_info_t *info) {
	int ret;
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);

	if(info->iphdr == NULL)
	{
		SMUX_DBG(info->debug_level_idx, info, "%s(%d): without IPv4 header !",__func__,__LINE__);
		return 0;
	}

	if(info->iphdr->daddr == rule->filters.dip)
		ret = 1;
	else
		ret = 0;
	
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): ret=%d",__func__,__LINE__,ret);
	return ret;
}

static int filter_sip_range(struct smux_rule_t *rule, struct smux_pkt_info_t *info) {
	int ret;
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);

	if(info->iphdr == NULL)
	{
		SMUX_DBG(info->debug_level_idx, info, "%s(%d): without IPv4 header !",__func__,__LINE__);
		return 0;
	}

	if(info->iphdr->saddr >= rule->filters.sip_start && info->iphdr->saddr <= rule->filters.sip_end)
		ret = 1;
	else
		ret = 0;
	
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): ret=%d",__func__,__LINE__,ret);
	return ret;
}

static int filter_dip_range(struct smux_rule_t *rule, struct smux_pkt_info_t *info) {
	int ret;
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);

	if(info->iphdr == NULL)
	{
		SMUX_DBG(info->debug_level_idx, info, "%s(%d): without IPv4 header !",__func__,__LINE__);
		return 0;
	}

	if(info->iphdr->daddr >= rule->filters.dip_start && info->iphdr->daddr <= rule->filters.dip_end)
		ret = 1;
	else
		ret = 0;
	
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): ret=%d",__func__,__LINE__,ret);
	return ret;
}

static int filter_ipl4proto(struct smux_rule_t *rule, struct smux_pkt_info_t *info) {
	int ret;
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);

	if(info->iphdr == NULL && info->ipv6hdr == NULL)
	{
		SMUX_DBG(info->debug_level_idx, info, "%s(%d): without IPv4/IPv6 header !",__func__,__LINE__);
		return 0;
	}

	if(info->iphdr)
	{
		if(info->iphdr->protocol == rule->filters.ipl4proto)
			ret = 1;
		else
			ret = 0;
	}
	else
	{
		if(info->ipv6hdr->nexthdr == rule->filters.ipl4proto)
			ret = 1;
		else
			ret = 0;
	}
	
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): ret=%d",__func__,__LINE__,ret);
	return ret;
}

static int filter_multicast(struct smux_rule_t *rule, struct smux_pkt_info_t *info) {
	int ret;
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);

	if(info->skb->pkt_type == PACKET_MULTICAST)
		ret = 1;
	else
		ret = 0;
	
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): ret=%d",__func__,__LINE__,ret);
	return ret;
}

static int filter_dscp_range(struct smux_rule_t *rule, struct smux_pkt_info_t *info) {
	int i, j;
	struct smux_ip_tos_t *ip_tos_p;
	uint8_t dscp;

	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);

	if(info->iphdr == NULL)
	{
		SMUX_DBG(info->debug_level_idx, info, "%s(%d): without IPv4 header !",__func__,__LINE__);
		return 0;
	}
	ip_tos_p = (struct smux_ip_tos_t *) &(info->iphdr->tos);
	dscp = IP_TOS_GET_DSCP(ip_tos_p);

	for(i=0 ; i<DSCP_LEN ; i++)
	{
		for(j=0 ; j<BITWISE_NUM ;j++)
		{
			if((rule->filters.ipdscp[i]&(1<<j)) && dscp == i*BITWISE_NUM+j)
			{
				SMUX_DBG(info->debug_level_idx, info, "%s(%d): match dscp=%u",__func__,__LINE__,dscp);
				return 1;
			}
		}
	}

	return 0;
}

static int filter_ethtype(struct smux_rule_t *rule, struct smux_pkt_info_t *info) {
	int ret;
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);
	if (info->ethertype == rule->filters.ethtype)
		ret = 1;
	else
		ret = 0;
	
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): ret=%d",__func__,__LINE__,ret);
	return ret;
}

static int filter_vtags(struct smux_rule_t *rule, struct smux_pkt_info_t *info) {
	int i, bound;
	int ret = 1;
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);
	
	if(info->vtag_count)
	{
		bound = MIN(info->vtag_count,SMUX_MAX_TAGS);
		for (i=0; i<bound && ret; i++) {
			ret = ret && 
				((rule->filters.vtags[i+1].vlan_proto_mask & info->vtags[bound-i-1].vlan_proto) == rule->filters.vtags[i+1].vlan_proto) &&
				((rule->filters.vtags[i+1].vlan_tci_mask & info->vtags[bound-i-1].vlan_tci) == rule->filters.vtags[i+1].vlan_tci);
		}

		if(ret && (rule->filters.vtags[0].vlan_proto_mask || rule->filters.vtags[0].vlan_tci_mask))
		{
			ret = ret && 
				((rule->filters.vtags[0].vlan_proto_mask & info->vtags[0].vlan_proto) == rule->filters.vtags[0].vlan_proto) &&
				((rule->filters.vtags[0].vlan_tci_mask & info->vtags[0].vlan_tci) == rule->filters.vtags[0].vlan_tci);
		}
	}
	else
		ret = 0;
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): ret=%d",__func__,__LINE__,ret);
	return ret;
}

static int filter_uni_dmac_outermost_vlan(struct smux_rule_t *rule, struct smux_pkt_info_t *info) {
	int ret = 0;
#if defined(CONFIG_COMMON_RT_API) && defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
	int i = 0, bound = 0;
	struct smux_rule_action_t *act = NULL;
	struct smux_vlan_tag_t *v = NULL;
	uint16_t act_vid = 0, vtag_count = 0;

	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);

	if (smux_configure.mapping_outermost_vlan_by_mac_from_lut &&
		info->vtags_inquire_by_lut.vlan_proto == GET_VID_BY_LUT) 
	{	
		//compare with inner VID with LUT VID
		vtag_count = info->vtag_count;
		if(vtag_count > 0)
		{
			bound = MIN(vtag_count, SMUX_MAX_TAGS);
			v = &(info->vtags[bound - 1]); //get inner VID
			act_vid = (v->vlan_tci & VID_MASK);
		}
		
		for (i = 0; i < rule->action_count; i++) 
		{
			act = &rule->actions[i];
			if (act->action == SMUX_ACT_PUSH_TAG) 
			{
				vtag_count++;
			}
			else if (act->action == SMUX_ACT_POP_TAG) 
			{
				vtag_count--;
			}
			else if (act->action == SMUX_ACT_SET_VID && vtag_count > 0) 
			{
				if(act->args[1] == 1)
					act_vid = act->args[0];
			}
		}
		
		if(vtag_count <= 0) 
			act_vid = 0;
		
		SMUX_DBG(info->debug_level_idx, info, "%s(%d): LUT CVID=%u, action CVID=%u",__func__,__LINE__, ntohs(info->vtags_inquire_by_lut.vlan_tci), ntohs(act_vid));
		
		if (info->vtags_inquire_by_lut.vlan_tci == act_vid) {
			ret = 1;
		}
	}
	else {
		SMUX_DBG(info->debug_level_idx, info, "%s(%d): Ignore match VID of the LUT!", __func__, __LINE__);
		ret = 1;
	}
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): ret=%d", __func__, __LINE__, ret);
#else
	ret = 1;
#endif
	return ret;
}

static int filter_org_vtags(struct smux_rule_t *rule, struct smux_pkt_info_t *info) {
	int i, bound;
	int ret = 1;
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);
	if(info->org_vtag_count > 0)
	{
		bound = MIN(info->org_vtag_count,SMUX_MAX_TAGS);
		for (i=0; i<bound && ret; i++) {
			ret = ret && 
				((rule->filters.org_vtags[i+1].vlan_proto_mask & info->org_vtags[bound-i-1].vlan_proto) == rule->filters.org_vtags[i+1].vlan_proto) &&
				((rule->filters.org_vtags[i+1].vlan_tci_mask & info->org_vtags[bound-i-1].vlan_tci) == rule->filters.org_vtags[i+1].vlan_tci);
		}
		if(ret && (rule->filters.org_vtags[0].vlan_proto_mask || rule->filters.org_vtags[0].vlan_tci_mask))
		{
			ret = ret && 
				((rule->filters.org_vtags[0].vlan_proto_mask & info->org_vtags[0].vlan_proto) == rule->filters.org_vtags[0].vlan_proto) &&
				((rule->filters.org_vtags[0].vlan_tci_mask & info->org_vtags[0].vlan_tci) == rule->filters.org_vtags[0].vlan_tci);
		}
	}
	else
		ret = 0;
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): ret=%d",__func__,__LINE__,ret);
	return ret;
}

static int filter_txif(struct smux_rule_t *rule, struct smux_pkt_info_t *info) {
	int ret;
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);
	if (!strncmp(rule->filters.txif_name, info->skb->dev->name, rule->filters.txif_match))
	    ret = 1;
	else
	    ret = 0;    
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): ret=%d",__func__,__LINE__,ret);
	return ret;
}

static int filter_skb_mark(struct smux_rule_t *rule, struct smux_pkt_info_t *info) {
	int ret;
	
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): rule->filters.mark.mask=0x%x, rule->filters.mark.value=0x%x, skb->mark=0x%x",__func__,__LINE__,rule->filters.mark.mask,rule->filters.mark.value,info->skb->mark);
	
	if (((info->skb->mark) & (rule->filters.mark.mask)) == rule->filters.mark.value)
	    ret = 1;
	else
	    ret = 0;    
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): ret=%d",__func__,__LINE__,ret);
	return ret;
}

#ifdef CONFIG_RTK_SKB_MARK2
static int filter_skb_mark2(struct smux_rule_t *rule, struct smux_pkt_info_t *info) {
	int ret;
	
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): rule->filters.mark2.mask=0x%llx, rule->filters.mark2.value=0x%llx, skb->mark2=0x%llx",__func__,__LINE__,rule->filters.mark2.mask,rule->filters.mark2.value,info->skb->mark2);

	if (((info->skb->mark2) & (rule->filters.mark2.mask)) == rule->filters.mark2.value)
		ret = 1;
	else
		ret = 0;
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): ret=%d",__func__,__LINE__,ret);
	return ret;
}
#endif

static int filter_uni_mac(struct smux_rule_t *rule, struct smux_pkt_info_t *info) {
	int ret = 0;
	struct smux_realdev_cb_t  *realdev_cb;
	unsigned char *mac,*mac2;
	struct sk_buff *skb = info->skb;
	struct ethhdr *eth;
	struct smux_dev_priv *priv;
	
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): rule->filters.check_uni=%d, rule->filters.rx_if=%s",__func__,__LINE__,rule->filters.check_mac,(info->rxdev)?info->rxdev->name:"NULL");
	
	if(rule->filters.check_mac && info->rxdev)
	{
		eth = (struct ethhdr *)skb_mac_header(skb);
		mac = eth->h_dest;
		mac2 = info->rxdev->dev_addr;
		if(eth->h_dest[0]&0x1)
			ret = 1;
		else if(ether_addr_equal(mac, mac2))
			ret = 1;
		else //check other virtual if mac for rxdev
		{
			ret = 0;
			if(smux_priv_flags_get(info->rxdev,PRIV_RSMUX) && (realdev_cb = smux_realdev_cb_get(info->rxdev)))
			{
				list_for_each_entry_rcu(priv, &realdev_cb->list_virt, list_virt) 
				{
					mac2 = priv->virt_dev->dev_addr;
					if(ether_addr_equal(mac, mac2))
					{
						ret = 1; 
						break;
					}
				}
			}
		}
	}
	else ret = 1;
	
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): ret=%d",__func__,__LINE__,ret);
	return ret;
}

static int filter_dmac(struct smux_rule_t *rule, struct smux_pkt_info_t *info) {
	int ret = 0;
	struct ethhdr *eth;
	unsigned char *mac,*mac2,*mask;
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);

	eth = (struct ethhdr *)skb_mac_header(info->skb);
	mac = eth->h_dest;
	mac2 = rule->filters.dmac;
	mask = rule->filters.dmac_mask;

	if(ether_addr_equal_masked(mac, mac2, mask)) ret = 1;
	
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): ret=%d",__func__,__LINE__,ret);
	return ret;
}

static int filter_smac(struct smux_rule_t *rule, struct smux_pkt_info_t *info) {
	int ret = 0;
	struct ethhdr *eth;
	unsigned char *mac,*mac2,*mask;
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);

	eth = (struct ethhdr *)skb_mac_header(info->skb);
	mac = eth->h_source;
	mac2 = rule->filters.smac;
	mask = rule->filters.smac_mask;

	if(ether_addr_equal_masked(mac, mac2, mask)) ret = 1;
	
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): ret=%d",__func__,__LINE__,ret);
	return ret;
}

static smux_filter_func_t *filter_funcs[] = {
	[SMUX_FLT_ETHTYPE] = filter_ethtype,
	[SMUX_FLT_VTAGS] = filter_vtags,
	[SMUX_FLT_ORG_VTAGS] = filter_org_vtags,
	[SMUX_FLT_TXIF_NAME] = filter_txif,
	[SMUX_FLT_SKB_MARK] = filter_skb_mark,
#ifdef CONFIG_RTK_SKB_MARK2
	[SMUX_FLT_SKB_MARK2] = filter_skb_mark2,
#endif
	[SMUX_FLT_UNI_MAC] = filter_uni_mac,
	[SMUX_FLT_DMAC] = filter_dmac,
	[SMUX_FLT_SMAC] = filter_smac,
	[SMUX_FLT_IPL4PROTO] = filter_ipl4proto,
	[SMUX_FLT_SIP] = filter_sip,
	[SMUX_FLT_DIP] = filter_dip,
	[SMUX_FLT_SIP_RANGE] = filter_sip_range,
	[SMUX_FLT_DIP_RANGE] = filter_dip_range,
	[SMUX_FLT_MULTICAST] = filter_multicast,
	[SMUX_FLT_DSCP_RANGE] = filter_dscp_range,
	/*for N:1 vlan translate, downstream eggress need to set corressponding vlan by mac for PPTP. */
	[SMUX_FLT_UNI_DMAC_OUTERMOST_VLAN] = filter_uni_dmac_outermost_vlan,
};

static struct sk_buff *action_set_ethtype(struct smux_rule_action_t *act, struct smux_pkt_info_t *info, struct smux_table_t *tbl) {
	struct ethhdr *eth;

	if(info->ethertype == act->args[0]) {
		return info->skb;
	}
	
	info->skb = smux_unshareSKB(info, info->skb);
	info->ethertype = act->args[0];
	if((eth = (struct ethhdr *)skb_mac_header(info->skb))) {
		eth->h_proto = htons(info->ethertype);
	}
	SMUX_DBG(info->debug_level_idx, info, "SetEthType: to %x", act->args[0]);
	return info->skb;
}

static struct sk_buff *smuxdev_vlan_push(int dir, struct sk_buff *skb, int push_count, __be16 default_tpid, __be16  default_vlan_tci)
{
	struct sk_buff *skb_tmp;
	struct ethhdr *eth;
	struct smux_vlan_tag_t *tag;
	int i;
	
	//SMUX_DBG(info->debug_level_idx, info, "%s",__func__);	
	if(push_count <= 0) return skb;
	
	skb_tmp = skb;
#if 1	
	if(skb_cow_head(skb_tmp, VLAN_HLEN*push_count) < 0){
		//SMUX_DBG(info->debug_level_idx, info, "%s Error memory for resize %d", __func__, VLAN_HLEN*push_count);
		return skb;
	}

	skb_push(skb_tmp, VLAN_HLEN*push_count);
	if(dir == SMUX_DIR_RX || dir == SMUX_DIR_RXMC)
	{
		memmove(skb_tmp->data - ETH_HLEN, 
				skb_tmp->data - ETH_HLEN + (VLAN_HLEN*push_count), 
				2 * ETH_ALEN);
	}
	else
	{
		memmove(skb_tmp->data, 
				skb_tmp->data + (VLAN_HLEN*push_count), 
				2 * ETH_ALEN);
				
	}
	skb_tmp->mac_header -= (VLAN_HLEN*push_count);
#else
	if(vlan_insert_tag_set_proto(skb_tmp, default_tpid, default_vlan_tci) == NULL)
	{
		//SMUX_DBG(info->debug_level_idx, info, "[%s]error when add cvlan tag",__FUNCTION__);
		return skb;
	}
#endif

	eth = (struct ethhdr *)skb_mac_header(skb_tmp);
	tag = (struct smux_vlan_tag_t *)((char *)eth + (ETH_ALEN*2));
	for(i=0;i<push_count;i++)
	{
		tag[i].vlan_proto = default_tpid;
		tag[i].vlan_tci = htons(default_vlan_tci);
	}

	return skb_tmp;
}

static struct sk_buff *action_push_tag(struct smux_rule_action_t *act, struct smux_pkt_info_t *info, struct smux_table_t *tbl) {
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);		
	SMUX_DBG(info->debug_level_idx, info, "push-tag with default_tpid=%x, default_vlan_tci=0x%04x",ntohs(tbl->default_tpid),ntohs(tbl->default_vlan_tci));

	info->skb = smux_unshareSKB(info, info->skb);
	
	info->skb = smuxdev_vlan_push(info->dir, info->skb, 1, tbl->default_tpid, tbl->default_vlan_tci);
	info->vtag_count++;
	smux_update_pkt_info(info->skb, info);

	SMUX_DBG(info->debug_level_idx, info, "[%s] vtags=%p vtag_count=%d ",__func__,info->vtags,info->vtag_count);
	
	return info->skb;
}

static struct sk_buff *smuxdev_vlan_pop(int dir, struct sk_buff *skb, int pop_count)
{
	struct ethhdr *eth;
	struct smux_vlan_tag_t *tag;
	
	//SMUX_DBG(info->debug_level_idx, info, "%s",__func__);	
	if(pop_count <= 0) return skb;
	
	//SMUX_DBG(info->debug_level_idx, info, "%s(%d): skb=%p", __func__,__LINE__,skb_tmp);
	skb_pull_rcsum(skb, VLAN_HLEN*pop_count);
	if(dir == SMUX_DIR_RX || dir == SMUX_DIR_RXMC)
	{
		memmove(skb->data - ETH_HLEN,
				skb->data - ETH_HLEN - (VLAN_HLEN*pop_count),
				2 * ETH_ALEN);
	}
	else
	{
		memmove(skb->data,
				skb->data - (VLAN_HLEN*pop_count),
				2 * ETH_ALEN);
	}
	skb->mac_header += (VLAN_HLEN*pop_count);

	return skb;
}

static struct sk_buff *action_pop_tag(struct smux_rule_action_t *act, struct smux_pkt_info_t *info, struct smux_table_t *tbl) {	
	struct ethhdr *eth;
	struct smux_vlan_tag_t *tag;
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);
	if (info->vtag_count <= 0) {
		SMUX_DBG(info->debug_level_idx, info, "ERR: pop-tag with vtag count=%d",info->vtag_count);		
		return info->skb;
	}
	
	info->skb = smux_unshareSKB(info, info->skb);

	info->skb = smuxdev_vlan_pop(info->dir, info->skb, 1);
	info->vtag_count--;
	smux_update_pkt_info(info->skb, info);

	SMUX_DBG(info->debug_level_idx, info, "[%s] vtags=%p vtag_count=%d ",__func__,info->vtags,info->vtag_count);

	return info->skb;
}

static struct sk_buff *action_dup_forward(struct smux_rule_action_t *act, struct smux_pkt_info_t *info, struct smux_table_t *tbl) {
	//nothing to do here, action at RX chian 
	return info->skb;
}

static struct sk_buff *action_set_tpid(struct smux_rule_action_t *act, struct smux_pkt_info_t *info, struct smux_table_t *tbl) {
    int idx=act->args[1];
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);
	
	if(idx<1 || idx>SMUX_MAX_TAGS){ //allow tag 1 ~ SMUX_MAX_TAGS
	    SMUX_DBG(info->debug_level_idx, info, "%s(%d): invalid tag %d", __func__,__LINE__,act->args[1]);
	    return info->skb;
	}
	idx = info->vtag_count-idx;
	if(idx>=0 && idx<info->vtag_count) {
		info->skb = smux_unshareSKB(info, info->skb);
		info->vtags[idx].vlan_proto = act->args[0]&0xffff;		
		SMUX_DBG(info->debug_level_idx, info, "[%s] vtags[%d] %p ", __func__,idx,&info->vtags[idx]);
	}
	SMUX_DBG(info->debug_level_idx, info, "%s(%d):After vtags[%d].vlan_proto 0x%04x", __func__,__LINE__,idx,info->vtags[idx].vlan_proto);
	return info->skb;
}

static struct sk_buff *action_set_vid(struct smux_rule_action_t *act, struct smux_pkt_info_t *info, struct smux_table_t *tbl) {
    int idx=act->args[1];
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);
	
	if(idx<1 || idx>SMUX_MAX_TAGS){ //allow tag 1 ~ SMUX_MAX_TAGS
	    SMUX_DBG(info->debug_level_idx, info, "%s(%d): invalid tag %d", __func__,__LINE__,act->args[1]);
	    return info->skb;
	}
	idx = info->vtag_count-idx;
	if(idx>=0 && idx<info->vtag_count) {	
		info->skb = smux_unshareSKB(info, info->skb);
		info->vtags[idx].vlan_tci = (info->vtags[idx].vlan_tci & ~VID_MASK) | act->args[0];
		SMUX_DBG(info->debug_level_idx, info, "[%s] vtags[%d] %p ", __func__,idx,&info->vtags[idx]);
	}
	SMUX_DBG(info->debug_level_idx, info, "%s(%d):After vtags[%d].vlan_tci 0x%04x", __func__,__LINE__,idx,info->vtags[idx].vlan_tci);
	return info->skb;
}

static struct sk_buff *action_set_priority(struct smux_rule_action_t *act, struct smux_pkt_info_t *info, struct smux_table_t *tbl) {
    int idx=act->args[1];
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);

	if(idx<1 || idx>SMUX_MAX_TAGS){ //allow tag 1 ~ SMUX_MAX_TAGS
	    SMUX_DBG(info->debug_level_idx, info, "%s(%d): invalid tag %d", __func__,__LINE__,act->args[1]);
	    return info->skb;
	}
	idx = info->vtag_count-idx;	
	if(idx>=0 && idx<info->vtag_count) {
		info->skb = smux_unshareSKB(info, info->skb);
		info->vtags[idx].vlan_tci = (info->vtags[idx].vlan_tci & ~PRIO_MASK) | act->args[0];
		SMUX_DBG(info->debug_level_idx, info, "[%s] vtags[%d] %p ", __func__,idx,&info->vtags[idx]);
	}
	SMUX_DBG(info->debug_level_idx, info, "%s(%d):After vtags[%d].vlan_tci 0x%04x", __func__,__LINE__,idx,info->vtags[idx].vlan_tci);
	return info->skb;
}

static struct sk_buff *action_set_cfi(struct smux_rule_action_t *act, struct smux_pkt_info_t *info, struct smux_table_t *tbl) {
    int idx=act->args[1];
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);

	if(idx<1 || idx>SMUX_MAX_TAGS){ //allow tag 1 ~ SMUX_MAX_TAGS
	    SMUX_DBG(info->debug_level_idx, info, "%s(%d): invalid tag %d", __func__,__LINE__,act->args[1]);
	    return info->skb;
	}
	idx = info->vtag_count-idx;
	if(idx>=0 && idx<info->vtag_count) {
		info->skb = smux_unshareSKB(info, info->skb);
		info->vtags[idx].vlan_tci = (info->vtags[idx].vlan_tci & ~CFI_MASK) | act->args[0];
		SMUX_DBG(info->debug_level_idx, info, "[%s] vtags[%d] %p ", __func__,idx,&info->vtags[idx]);
	}
	SMUX_DBG(info->debug_level_idx, info, "%s(%d):After vtags[%d].vlan_tci 0x%4x", __func__,__LINE__,idx,info->vtags[idx].vlan_tci);
	return info->skb;
}

static struct sk_buff *action_copy_8021q(struct smux_rule_action_t *act, struct smux_pkt_info_t *info, struct smux_table_t *tbl) {
	int idx_src=act->args[0];
    int idx_dest=act->args[1];
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);
	
	if(idx_src<1 || idx_src>SMUX_MAX_TAGS){ //allow tag 1 ~ SMUX_MAX_TAGS
	    SMUX_DBG(info->debug_level_idx, info, "%s(%d): invalid tag idx_src=%d", __func__,__LINE__,act->args[0]);
	    return info->skb;
	}

	if(idx_dest<1 || idx_dest>SMUX_MAX_TAGS){ //allow tag 1 ~ SMUX_MAX_TAGS
	    SMUX_DBG(info->debug_level_idx, info, "%s(%d): invalid tag idx_dest=%d", __func__,__LINE__,act->args[1]);
	    return info->skb;
	}
	
	idx_src = info->vtag_count-idx_src;
	idx_dest = info->vtag_count-idx_dest;
	
	if(idx_src<info->vtag_count && idx_dest<info->vtag_count) {
		info->skb = smux_unshareSKB(info, info->skb);
		info->vtags[idx_dest].vlan_proto = info->vtags[idx_src].vlan_proto;
		info->vtags[idx_dest].vlan_tci = info->vtags[idx_src].vlan_tci;
		SMUX_DBG(info->debug_level_idx, info, "[%s] copy from vtags[%d] %p to vtags[%d] %p", __func__,idx_src,&info->vtags[idx_src],idx_dest,&info->vtags[idx_dest]);
	}
	SMUX_DBG(info->debug_level_idx, info, "%s(%d):After copied vtags[%d].vlan_proto 0x%4x .vlan_tci 0x%4x", __func__,__LINE__,idx_src,info->vtags[idx_src].vlan_proto,info->vtags[idx_src].vlan_tci);
	SMUX_DBG(info->debug_level_idx, info, "%s(%d):      writed vtags[%d].vlan_proto 0x%4x .vlan_tci 0x%4x", __func__,__LINE__,idx_dest,info->vtags[idx_dest].vlan_proto,info->vtags[idx_dest].vlan_tci);
	return info->skb;
}

static struct sk_buff *action_copy_tpid(struct smux_rule_action_t *act, struct smux_pkt_info_t *info, struct smux_table_t *tbl) {
	int idx_src=act->args[0];
    int idx_dest=act->args[1];
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);
	
	if(idx_src<1 || idx_src>SMUX_MAX_TAGS){ //allow tag 1 ~ SMUX_MAX_TAGS
	    SMUX_DBG(info->debug_level_idx, info, "%s(%d): invalid tag idx_src=%d", __func__,__LINE__,act->args[0]);
	    return info->skb;
	}

	if(idx_dest<1 || idx_dest>SMUX_MAX_TAGS){ //allow tag 1 ~ SMUX_MAX_TAGS
	    SMUX_DBG(info->debug_level_idx, info, "%s(%d): invalid tag idx_dest=%d", __func__,__LINE__,act->args[1]);
	    return info->skb;
	}
	
	idx_src = info->vtag_count-idx_src;
	idx_dest = info->vtag_count-idx_dest;
	
	if(idx_src<info->vtag_count && idx_dest<info->vtag_count) {
		info->skb = smux_unshareSKB(info, info->skb);
		info->vtags[idx_dest].vlan_proto = info->vtags[idx_src].vlan_proto;
		SMUX_DBG(info->debug_level_idx, info, "[%s] copy from vtags[%d] %p to vtags[%d] %p", __func__,idx_src,&info->vtags[idx_src],idx_dest,&info->vtags[idx_dest]);
	}
	SMUX_DBG(info->debug_level_idx, info, "%s(%d):After copied vtags[%d].vlan_proto 0x%4x .vlan_tci 0x%4x", __func__,__LINE__,idx_src,info->vtags[idx_src].vlan_proto,info->vtags[idx_src].vlan_tci);
	SMUX_DBG(info->debug_level_idx, info, "%s(%d):      writed vtags[%d].vlan_proto 0x%4x .vlan_tci 0x%4x", __func__,__LINE__,idx_dest,info->vtags[idx_dest].vlan_proto,info->vtags[idx_dest].vlan_tci);
	return info->skb;
}

static struct sk_buff *action_copy_vid(struct smux_rule_action_t *act, struct smux_pkt_info_t *info, struct smux_table_t *tbl) {
	int idx_src=act->args[0];
    int idx_dest=act->args[1];
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);

	if(idx_src<1 || idx_src>SMUX_MAX_TAGS){ //allow tag 1 ~ SMUX_MAX_TAGS
	    SMUX_DBG(info->debug_level_idx, info, "%s(%d): invalid tag idx_src=%d", __func__,__LINE__,act->args[0]);
	    return info->skb;
	}

	if(idx_dest<1 || idx_dest>SMUX_MAX_TAGS){ //allow tag 1 ~ SMUX_MAX_TAGS
	    SMUX_DBG(info->debug_level_idx, info, "%s(%d): invalid tag idx_dest=%d", __func__,__LINE__,act->args[1]);
	    return info->skb;
	}
	
	idx_src = info->vtag_count-idx_src;
	idx_dest = info->vtag_count-idx_dest;
	
	if(idx_src<info->vtag_count && idx_dest<info->vtag_count) {
		info->skb = smux_unshareSKB(info, info->skb);
		info->vtags[idx_dest].vlan_tci = (info->vtags[idx_dest].vlan_tci & ~VID_MASK) | (info->vtags[idx_src].vlan_tci & VID_MASK);
		SMUX_DBG(info->debug_level_idx, info, "[%s] copy from vtags[%d] %p to vtags[%d] %p", __func__,idx_src,&info->vtags[idx_src],idx_dest,&info->vtags[idx_dest]);
	}
	SMUX_DBG(info->debug_level_idx, info, "%s(%d):After copied vtags[%d].vlan_proto 0x%4x .vlan_tci 0x%4x", __func__,__LINE__,idx_src,info->vtags[idx_src].vlan_proto,info->vtags[idx_src].vlan_tci);
	SMUX_DBG(info->debug_level_idx, info, "%s(%d):      writed vtags[%d].vlan_proto 0x%4x .vlan_tci 0x%4x", __func__,__LINE__,idx_dest,info->vtags[idx_dest].vlan_proto,info->vtags[idx_dest].vlan_tci);
	return info->skb;
}

static struct sk_buff *action_copy_priority(struct smux_rule_action_t *act, struct smux_pkt_info_t *info, struct smux_table_t *tbl) {
	int idx_src=act->args[0];
    int idx_dest=act->args[1];
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);
	
	if(idx_src<1 || idx_src>SMUX_MAX_TAGS){ //allow tag 1 ~ SMUX_MAX_TAGS
	    SMUX_DBG(info->debug_level_idx, info, "%s(%d): invalid tag idx_src=%d", __func__,__LINE__,act->args[0]);
	    return info->skb;
	}

	if(idx_dest<1 || idx_dest>SMUX_MAX_TAGS){ //allow tag 1 ~ SMUX_MAX_TAGS
	    SMUX_DBG(info->debug_level_idx, info, "%s(%d): invalid tag idx_dest=%d", __func__,__LINE__,act->args[1]);
	    return info->skb;
	}
	
	idx_src = info->vtag_count-idx_src;
	idx_dest = info->vtag_count-idx_dest;
	
	if(idx_src<info->vtag_count && idx_dest<info->vtag_count) {
		info->skb = smux_unshareSKB(info, info->skb);
		info->vtags[idx_dest].vlan_tci = (info->vtags[idx_dest].vlan_tci & ~PRIO_MASK) | (info->vtags[idx_src].vlan_tci & PRIO_MASK);
		SMUX_DBG(info->debug_level_idx, info, "[%s] copy from vtags[%d] %p to vtags[%d] %p", __func__,idx_src,&info->vtags[idx_src],idx_dest,&info->vtags[idx_dest]);
	}
	SMUX_DBG(info->debug_level_idx, info, "%s(%d):After copied vtags[%d].vlan_proto 0x%4x .vlan_tci 0x%4x", __func__,__LINE__,idx_src,info->vtags[idx_src].vlan_proto,info->vtags[idx_src].vlan_tci);
	SMUX_DBG(info->debug_level_idx, info, "%s(%d):      writed vtags[%d].vlan_proto 0x%4x .vlan_tci 0x%4x", __func__,__LINE__,idx_dest,info->vtags[idx_dest].vlan_proto,info->vtags[idx_dest].vlan_tci);
	return info->skb;
}

static struct sk_buff *action_copy_cfi(struct smux_rule_action_t *act, struct smux_pkt_info_t *info, struct smux_table_t *tbl) {
	int idx_src=act->args[0];
    int idx_dest=act->args[1];
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);

	if(idx_src<1 || idx_src>SMUX_MAX_TAGS){ //allow tag 1 ~ SMUX_MAX_TAGS
	    SMUX_DBG(info->debug_level_idx, info, "%s(%d): invalid tag idx_src=%d", __func__,__LINE__,act->args[0]);
	    return info->skb;
	}

	if(idx_dest<1 || idx_dest>SMUX_MAX_TAGS){ //allow tag 1 ~ SMUX_MAX_TAGS
	    SMUX_DBG(info->debug_level_idx, info, "%s(%d): invalid tag idx_dest=%d", __func__,__LINE__,act->args[1]);
	    return info->skb;
	}
	
	idx_src = info->vtag_count-idx_src;
	idx_dest = info->vtag_count-idx_dest;
	
	if(idx_src<info->vtag_count && idx_dest<info->vtag_count) {
		info->skb = smux_unshareSKB(info, info->skb);
		info->vtags[idx_dest].vlan_tci = (info->vtags[idx_dest].vlan_tci & ~CFI_MASK) | (info->vtags[idx_src].vlan_tci & CFI_MASK);
		SMUX_DBG(info->debug_level_idx, info, "[%s] copy from vtags[%d] %p to vtags[%d] %p", __func__,idx_src,&info->vtags[idx_src],idx_dest,&info->vtags[idx_dest]);
	}
	SMUX_DBG(info->debug_level_idx, info, "%s(%d):After copied vtags[%d].vlan_proto 0x%4x .vlan_tci 0x%4x", __func__,__LINE__,idx_src,info->vtags[idx_src].vlan_proto,info->vtags[idx_src].vlan_tci);
	SMUX_DBG(info->debug_level_idx, info, "%s(%d):      writed vtags[%d].vlan_proto 0x%4x .vlan_tci 0x%4x", __func__,__LINE__,idx_dest,info->vtags[idx_dest].vlan_proto,info->vtags[idx_dest].vlan_tci);
	return info->skb;
}

static struct sk_buff *action_map_dscp_to_priority(struct smux_rule_action_t *act, struct smux_pkt_info_t *info, struct smux_table_t *tbl) {
	int idx=act->args[0];
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);
	
	if(idx<1 || idx>SMUX_MAX_TAGS){ //allow tag 1 ~ SMUX_MAX_TAGS
	    SMUX_DBG(info->debug_level_idx, info, "%s(%d): invalid tag idx=%d", __func__,__LINE__,act->args[0]);
	    return info->skb;
	}
	
	idx = info->vtag_count-idx;
	
	if(idx>=0 && idx<info->vtag_count) {
		info->skb = smux_unshareSKB(info, info->skb);
		mapping_dscp_to_priority(act, info, idx);
	}
	return info->skb;
}

static struct sk_buff *action_target(struct smux_rule_action_t *act, struct smux_pkt_info_t *info, struct smux_table_t *tbl) {
	SMUX_DBG(info->debug_level_idx, info, "%s target_jumped=%d",__func__, info->target_jumped);
	if(!info->target_jumped || 
		(act->args[0] == SMUX_TARGET_CONTINUE || act->args[0] == SMUX_TARGET_ACCEPT))
	{
		info->target_jumped = 1;
		info->target = act->args[0];
		SMUX_DBG(info->debug_level_idx, info, "%s jump to %s(target=%d)",__func__, info->target == SMUX_TARGET_CONTINUE ? "CONTINUE" : (info->target == SMUX_TARGET_ACCEPT ? "ACCEPT" : "DROP"), info->target);
	}
	return info->skb;
}

static struct sk_buff *action_set_rxif(struct smux_rule_action_t *act, struct smux_pkt_info_t *info, struct smux_table_t *tbl) {
	struct net_device *dev;
    struct sk_buff *skb = info->skb;
    
	SMUX_DBG(info->debug_level_idx, info, "%s set skb->dev = %s",__func__, act->rxif_name);
	dev = get_virt_dev(NULL, act->rxif_name);
	if (NULL==dev){
		SMUX_DBG(SMUX_DEBUG_INDEX_ERROR, NULL, "%s(%d) Error found device(%s)",__func__, __LINE__, act->rxif_name);
		return NULL;
	}

    skb->dev=dev;

	return info->skb;
}

static struct sk_buff *action_set_skb_mark(struct smux_rule_action_t *act, struct smux_pkt_info_t *info, struct smux_table_t *tbl) {
    struct sk_buff *skb = info->skb;
    
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): Before skb->mark 0x%x", __func__,__LINE__,skb->mark);
	/* act->args[0] is mask, act->args[1] is value */
    skb->mark=(skb->mark&(~act->args[0]))|(act->args[0]&act->args[1]);
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): After skb->mark 0x%x", __func__,__LINE__,skb->mark);
	return info->skb;
}

#ifdef CONFIG_RTK_SKB_MARK2
static struct sk_buff *action_set_skb_mark2(struct smux_rule_action_t *act, struct smux_pkt_info_t *info, struct smux_table_t *tbl) {
	struct sk_buff *skb = info->skb;

	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): Before skb->mark2 0x%llx", __func__,__LINE__,skb->mark2);
	/* act->args[0] is mask, act->args[1] is value */
	skb->mark2=(skb->mark2&(~act->args_64[0]))|(act->args_64[0]&act->args_64[1]);
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): After skb->mark2 0x%llx", __func__,__LINE__,skb->mark2);
	return info->skb;
}
#endif

static struct sk_buff *action_set_dscp(struct smux_rule_action_t *act, struct smux_pkt_info_t *info, struct smux_table_t *tbl) {
	struct smux_ip_tos_t *ip_tos_p;
	struct smux_ipv6_tos_t *ipv6_tos_p;
	uint8_t dscp=act->args[0];
	uint32_t dscp_v6=act->args[1];
	
	SMUX_DBG(info->debug_level_idx, info, "%s",__func__);
	if(info->iphdr == NULL && info->ipv6hdr == NULL)
	{
		SMUX_DBG(info->debug_level_idx, info, "%s(%d): without IP header !",__func__,__LINE__);
		return info->skb;
	}
	
	info->skb = smux_unshareSKB(info, info->skb);

	if(info->iphdr)
	{
		ip_tos_p = (struct smux_ip_tos_t *) &(info->iphdr->tos);
		SMUX_DBG(info->debug_level_idx, info, "%s(%d):Before IP DSCP = %u", __func__,__LINE__, IP_TOS_GET_DSCP(ip_tos_p));
		IP_TOS_SET_DSCP(ip_tos_p, dscp);
		SMUX_DBG(info->debug_level_idx, info, "%s(%d):After IP DSCP = %u", __func__,__LINE__, IP_TOS_GET_DSCP(ip_tos_p));
	}
	else
	{
		ipv6_tos_p = (struct smux_ipv6_tos_t *) info->ipv6hdr;
		SMUX_DBG(info->debug_level_idx, info, "%s(%d):Before IPv6 DSCP = %u", __func__,__LINE__, IP6_TOS_GET_DSCP(ipv6_tos_p));
		IP6_TOS_SET_DSCP(ipv6_tos_p, dscp_v6);
		SMUX_DBG(info->debug_level_idx, info, "%s(%d):After IPv6 DSCP = %u", __func__,__LINE__, IP6_TOS_GET_DSCP(ipv6_tos_p));
	}
	
	return info->skb;
}

static struct sk_buff *action_revert_vlan(struct smux_rule_action_t *act, struct smux_pkt_info_t *info, struct smux_table_t *tbl) 
{
	SMUX_DBG(info->debug_level_idx, info, "%s revert to original vlan, org_vtag_count=%d, vtag_count=%d ",__func__, info->org_vtag_count, info->vtag_count);

	if(info->org_vtag_count > 0)
	{
		info->skb = smux_unshareSKB(info, info->skb);
		if(info->org_vtag_count > info->vtag_count)
		{
			info->skb = smuxdev_vlan_push(info->dir, info->skb, (info->org_vtag_count - info->vtag_count), htons(0x8100), 0x0001);
		}
		else if(info->org_vtag_count < info->vtag_count)
		{
			info->skb = smuxdev_vlan_pop(info->dir, info->skb, (info->vtag_count - info->org_vtag_count));
		}
		
		memcpy((char*)info->vtags, (char*)info->org_vtags, sizeof(struct smux_vlan_tag_t)*info->vtag_count);
		info->vtag_count = info->org_vtag_count;
		
		smux_update_pkt_info(info->skb, info);
	}
	else
	{
		if(info->vtag_count > 0)
		{
			info->skb = smux_unshareSKB(info, info->skb);
			info->skb = smuxdev_vlan_pop(info->dir, info->skb, info->vtag_count);
			info->vtag_count = 0;
			smux_update_pkt_info(info->skb, info);
		}
	}
	return info->skb;
}

static smux_action_func_t *action_funcs[] = {
	[SMUX_ACT_SET_RXIF] = action_set_rxif,
	[SMUX_ACT_SET_TPID] = action_set_tpid,
	[SMUX_ACT_SET_VID] = action_set_vid,
	[SMUX_ACT_SET_PRIORITY] = action_set_priority,
	[SMUX_ACT_SET_CFI] = action_set_cfi,
	[SMUX_ACT_SET_ETHTYPE] = action_set_ethtype,
	[SMUX_ACT_POP_TAG] = action_pop_tag,
	[SMUX_ACT_PUSH_TAG] = action_push_tag,
	[SMUX_ACT_DUP_FORWARD] = action_dup_forward,
	[SMUX_ACT_SET_SKB_MARK] = action_set_skb_mark,
#ifdef CONFIG_RTK_SKB_MARK2
	[SMUX_ACT_SET_SKB_MARK2] = action_set_skb_mark2,
#endif
	[SMUX_ACT_SET_DSCP] = action_set_dscp,
	[SMUX_ACT_COPY_8021Q] = action_copy_8021q,
	[SMUX_ACT_COPY_TPID] = action_copy_tpid,
	[SMUX_ACT_COPY_VID] = action_copy_vid,
	[SMUX_ACT_COPY_PRIORITY] = action_copy_priority,
	[SMUX_ACT_COPY_CFI] = action_copy_cfi,
	[SMUX_ACT_MAP_DSCP_TO_PRIORITY] = action_map_dscp_to_priority,
	[SMUX_ACT_SET_TARGET] = action_target,
	[SMUX_ACT_REVERT_VLAN] = action_revert_vlan,
};

static int is_from_multicast_gem_port(struct smux_pkt_info_t *info)
{
	int i;

	for(i=0 ; i<SMUX_MC_GEM_PORT_SKBMARK_NUM ; i++)
	{
		if(smux_configure.mc_gem_port_skbmark_tbl[i].mark_select == 1)
		{
			if((info->skb->mark&smux_configure.mc_gem_port_skbmark_tbl[i].mark_mask1) == smux_configure.mc_gem_port_skbmark_tbl[i].mark1)
				return 1;
		}
#ifdef CONFIG_RTK_SKB_MARK2
		else if(smux_configure.mc_gem_port_skbmark_tbl[i].mark_select == 2)
		{
			if((info->skb->mark2&smux_configure.mc_gem_port_skbmark_tbl[i].mark_mask2) == smux_configure.mc_gem_port_skbmark_tbl[i].mark2)
				return 1;
		}
#endif
	}
	return 0;
}

static int is_rule_matched(struct sk_buff *skb,struct smux_ruleset_t *rule, struct smux_pkt_info_t *info, int max_count) {
	int idx, matched = 1;
	unsigned int flags = rule->rule.filters.en_filter;
	smux_filter_func_t *fn;
	
	SMUX_DBG(info->debug_level_idx, info, "%s: rule[%s] flag 0x%x ",__func__,rule->rule.alias_name, flags);
	for(idx=0; idx<SMUX_FLT_MAX && matched; idx++) {
		if (!flags)
			break;
		
		if (flags & (1<<idx)){
			SMUX_DBG(info->debug_level_idx, info, "%s filter: %s",__func__, filter_idx2str((1<<idx)));
			flags &= ~(1<<idx);
			fn = filter_funcs[idx];
			matched = matched && fn(&rule->rule, info);
	    }
	}
	SMUX_DBG(info->debug_level_idx, info, "%s: rule[%s] flags=%x matched=%d",__func__, rule->rule.alias_name, flags, matched);
	return matched;
}

static int _forwardMulticastPkt(struct sk_buff *skb, struct smux_pkt_info_t *info, struct smux_ruleset_t *rule, struct smux_table_t *tbl)
{
	struct sk_buff *new_skb = NULL;
	struct smux_rule_action_t *act;
	smux_action_func_t *fn;
	struct smux_pkt_info_t new_info;
	int i;
	
	SMUX_DBG(SMUX_DEBUG_INDEX_MULTICAST, info, "%s COPY skb and send to %s",__func__, rule->rx_dev->name);
	new_skb = skb_copy(skb, GFP_ATOMIC);
	if (likely(new_skb != NULL)) 
	{
		/* apply action */
		new_info.debug_level_idx = SMUX_DEBUG_INDEX_MULTICAST;
		smux_build_cb(new_skb, NULL, &new_info);
		new_info.skb = new_skb;
		new_info.dir = info->dir;
		for (i=0; i<rule->rule.action_count; i++) {
			act = &rule->rule.actions[i];
			fn = action_funcs [act->action];
			new_skb = fn(act, &new_info, tbl);
			if (!new_skb) {
				SMUX_DBG(SMUX_DEBUG_INDEX_ERROR, NULL, "%s: Error fo rule function(%s), free skb\n", __FUNCTION__, action_str[act->action]);
				kfree_skb(new_info.skb);
				return 0;
			}
			new_info.skb = new_skb;
		}
		SMUX_DBG(SMUX_DEBUG_INDEX_MULTICAST, info, "%s(%d) skb:%p,%p,%p,%p,len=%d %d/%d/%d dev=%s", __func__,__LINE__,
			new_skb->head,new_skb->data,new_skb->tail,new_skb->end,new_skb->len,new_skb->mac_header,new_skb->network_header,new_skb->transport_header,new_skb->dev->name); 
		
		if (smux_priv_flags_get(new_skb->dev,PRIV_VSMUX)) {
			smux_dev_priv(new_skb->dev)->net_stats.rx_packets++;
			smux_dev_priv(new_skb->dev)->net_stats.rx_bytes += new_skb->len;
			if(new_skb->pkt_type == PACKET_MULTICAST)
				smux_dev_priv(new_skb->dev)->net_stats.multicast++;
		}
		else {
			new_skb->dev->stats.rx_packets++;
			new_skb->dev->stats.rx_bytes += new_skb->len;
			if(new_skb->pkt_type == PACKET_MULTICAST)
				new_skb->dev->stats.multicast++;
		}

		if(smux_configure.auto_reserve_vlan && new_info.vtag_count > 0 && smux_priv_flags_get(skb->dev,PRIV_RSMUX|PRIV_VSMUX) != PRIV_RSMUX)
		{
			new_skb = smux_save_skb_vlan(new_skb, &new_info);
		}
		netif_rx(new_skb);
	}
	return 0;
}

static int mapping_dscp_to_priority(struct smux_rule_action_t *act, struct smux_pkt_info_t *info, int idx)
{
	int i;
	struct smux_vlan_tag_t *vlan_tag;
	struct smux_ip_tos_t *ip_tos_p;
	struct smux_ipv6_tos_t *ipv6_tos_p;
	struct smux_realdev_cb_t *realdev_cb;
	struct smux_dscp_mapping_entry_t *dscp_tbl;

	realdev_cb = smux_realdev_cb_get(info->skb->dev);
	if (realdev_cb == NULL)
	{
			SMUX_DBG(info->debug_level_idx, info, "%s(%d): realdev_cb error !",__func__,__LINE__);
			return -1;
	}

	if(info->iphdr == NULL && info->ipv6hdr == NULL)
	{
		SMUX_DBG(info->debug_level_idx, info, "%s(%d): without IP header !",__func__,__LINE__);
		return -1;
	}
	
	if((realdev_cb->qos.dscp_mapping_enable && realdev_cb->qos.dscp_mapping_table != NULL))
		dscp_tbl = &realdev_cb->qos.dscp_mapping_table[0];
	else
		dscp_tbl = &dscp_mapping_table_default[0];
	
	if(info->iphdr) 
	{
		ip_tos_p = (struct smux_ip_tos_t *) &(info->iphdr->tos);
		i = IP_TOS_GET_DSCP(ip_tos_p);
		if(i < SMUX_MAX_DSCP_NUM && (dscp_tbl+i)->enable)
		{
			vlan_tag = &((dscp_tbl+i)->vlan_tag);
			info->vtags[idx].vlan_tci  = (info->vtags[idx].vlan_tci & ~PRIO_MASK) | (vlan_tag->vlan_tci & PRIO_MASK);
			SMUX_DBG(info->debug_level_idx, info, "%s(%d): match IPv4 DSCP=0x%02x(%d) mapping priority=%d to vlanidx=%d",__func__,__LINE__, IP_TOS_GET_DSCP(ip_tos_p), IP_TOS_GET_DSCP(ip_tos_p), ntohs(vlan_tag->vlan_tci) >> PRIO_SHIFT, idx);
		}
		else
		{	
			SMUX_DBG(info->debug_level_idx, info, "%s(%d): no match IPv4 DSCP=0x%02x(%d)",__func__,__LINE__, IP_TOS_GET_DSCP(ip_tos_p), IP_TOS_GET_DSCP(ip_tos_p));
		}
	}
	else 
	{
		ipv6_tos_p = (struct smux_ipv6_tos_t *) info->ipv6hdr;
		i = IP6_TOS_GET_DSCP(ipv6_tos_p);
		if(i < SMUX_MAX_DSCP_NUM && (dscp_tbl+i)->enable)
		{
			vlan_tag = &((dscp_tbl+i)->vlan_tag);
			info->vtags[idx].vlan_tci  = (info->vtags[idx].vlan_tci & ~PRIO_MASK) | (vlan_tag->vlan_tci & PRIO_MASK);
			SMUX_DBG(info->debug_level_idx, info, "%s(%d): match IPv6 DSCP=0x%02x(%d) mapping priority=%d to vlanidx=%d",__func__,__LINE__, IP6_TOS_GET_DSCP(ipv6_tos_p), IP6_TOS_GET_DSCP(ipv6_tos_p), ntohs(vlan_tag->vlan_tci) >> PRIO_SHIFT, idx);
		}
		else
		{
			SMUX_DBG(info->debug_level_idx, info, "%s(%d): no match IPv6 DSCP=0x%02x(%d)",__func__,__LINE__, IP6_TOS_GET_DSCP(ipv6_tos_p), IP6_TOS_GET_DSCP(ipv6_tos_p));
		}
	}
	
	return 0;
}

/* max level */
static int _process_packet(enum smux_direction_e dir, struct sk_buff *skb, struct smux_realdev_cb_t *dev_cb, struct smux_pkt_info_t *info) {
	struct smux_ruleset_t *rule, *first_rule = NULL;
	struct smux_table_t *tbl;
	struct smux_rule_action_t *act;
	smux_action_func_t *fn;	
	int is_matched;
	int ret = 0;
	int do_multicast = 0;	
	int i;
	
	if (info->vtag_count>SMUX_MAX_TAGS)
		return -EINVAL;
	
	rcu_read_lock();
	info->dir = dir;
	
	if(info->is_multicast && smux_configure.multicast_mode)
		do_multicast = 1;
	
	tbl = &dev_cb->tables[dir][info->vtag_count];
	SMUX_DBG(info->debug_level_idx, info, "%s(%d): tbl=%p",__func__,__LINE__,tbl);

	list_for_each_entry_rcu(rule, &tbl->rule_head, list) 
	{
		if (rule->rx_dev)
			info->rxdev = rule->rx_dev;
		is_matched = is_rule_matched(skb, rule, info, info->vtag_count);
		if (is_matched > 0) 
		{
			if(do_multicast && 
				(dir == SMUX_DIR_RXMC || 
				(dir == SMUX_DIR_RX && dev_cb->rx_multi_mode == SMUX_RX_MULTI_MATCH))
			){
				if(first_rule)
				{
					first_rule->rule.hit_count++;
					_forwardMulticastPkt(skb, info, first_rule, tbl);
					first_rule = NULL;
				}
				if(rule->rx_dev && rule->rx_dev != skb->dev)
				{
					if(first_rule == NULL){
						first_rule = rule;
					}
					continue;
				}
			}

			rule->rule.hit_count++;
			/* apply action */
			SMUX_DBG(info->debug_level_idx, info, "%s(%d): rule=%p, action_count=%d",__func__,__LINE__, rule, rule->rule.action_count);
			
			if(rule->rule.dup_forward && dir == SMUX_DIR_RX && rule->rx_dev != skb->dev)
			{
				_forwardMulticastPkt(skb, info, rule, tbl);
				continue;
			}
			else
			{
				for (i=0; i<rule->rule.action_count; i++) {
					act = &rule->rule.actions[i];
					fn = action_funcs [act->action];
					skb = fn(act, info, tbl);
					if (!skb) {
						SMUX_DBG(SMUX_DEBUG_INDEX_ERROR, NULL, "%s: Error fo rule function(%s)\n", __FUNCTION__, action_str[act->action]);
						info->target = SMUX_TARGET_DROP;
						goto out;
					}
					info->skb = skb;
				}
			}
			if(info->target_jumped && 
				//((dir == SMUX_DIR_RXMC /*|| dir == SMUX_DIR_RX*/) && do_multicast && info->target == SMUX_TARGET_DROP) ||
				(info->target!=SMUX_TARGET_CONTINUE)
			){
				SMUX_DBG(info->debug_level_idx, info, "No need process rules after here~");
				ret = 1;
				break;
			}
		} else if (is_matched < 0) {
			/* error? */
		}
	}
	
	if(first_rule && do_multicast)
	{
		rule = first_rule;
		rule->rule.hit_count++;
		/* apply action */
		SMUX_DBG(info->debug_level_idx, info, "%s(%d): first_rule=%p, action_count=%d",__func__,__LINE__, rule, rule->rule.action_count);
		
		for (i=0; i<rule->rule.action_count; i++) {
			act = &rule->rule.actions[i];
			fn = action_funcs [act->action];
			skb = fn(act, info, tbl);
			if (!skb) {
				SMUX_DBG(SMUX_DEBUG_INDEX_ERROR, NULL, "%s: Error fo rule function(%s)\n", __FUNCTION__, action_str[act->action]);
				info->target = SMUX_TARGET_DROP;
				goto out;
			}
			info->skb = skb;
		}
	}
	
	smux_adjust_skb(skb, info);

	info->skb = skb;
out:
	rcu_read_unlock();
	return ret;
};

static rx_handler_result_t smux_receive_skb(struct sk_buff **pskb) {
	int pkt_dump_idx = SMUX_DEBUG_INDEX_RX_PACKET, debug_idx = 0;
	enum smux_direction_e dir = SMUX_DIR_RX;
	struct smux_pkt_info_t *info, pkt_info;
	struct sk_buff *skb = *pskb;
	struct smux_realdev_cb_t *realdev_cb;
	rx_handler_result_t ret = RX_HANDLER_PASS;

	if (!skb->dev || !smux_priv_flags_get(skb->dev,PRIV_RSMUX))
	{
		ret = RX_HANDLER_PASS;
		goto done;
	}
		
	realdev_cb = smux_realdev_cb_get(skb->dev);
	if (NULL==realdev_cb)
	{
		SMUX_DBG(SMUX_DEBUG_INDEX_ERROR, NULL, "[SMUX][RX] Cannot found realdev for vif(%s), SKB (%p)\n", skb->dev->name, skb);
		ret = RX_HANDLER_PASS;
		goto done;
	}

	info = &pkt_info;
	memset(info, 0, sizeof(struct smux_pkt_info_t));
	info->skb = (*pskb);

	if(skb->pkt_type == PACKET_BROADCAST || skb->pkt_type == PACKET_MULTICAST)
		info->is_multicast = 1;
	
	//don't care packet is multicast or not if the packet which was form the broadcast/multicast GEM port
	if(unlikely(realdev_cb->multicast.rx_multicast_rule_exist /*&& info->is_multicast */ 
		&& is_from_multicast_gem_port(info)
		))
	{
		dir = SMUX_DIR_RXMC;
		debug_idx = info->debug_level_idx = SMUX_DEBUG_INDEX_RXMC;
		pkt_dump_idx = SMUX_DEBUG_INDEX_RXMC_PACKET;
	}
	else
		debug_idx = info->debug_level_idx = SMUX_DEBUG_INDEX_RX;

	info->target_jumped = 0;
	info->target = SMUX_TARGET_CONTINUE;
	smux_build_cb(skb, realdev_cb, info);
	
	SMUX_DBG(debug_idx, info, "==============[Rx packet boundary start]==============");
	
	//info = (struct smux_pkt_info_t *)(*pskb)->cb;

	SMUX_DBG(debug_idx, info, "skb:%p,%p,%p,%p,len=%d %d/%d/%d dev=%s",
		skb->head,skb->data,skb->tail,skb->end,skb->len,skb->mac_header,skb->network_header,skb->transport_header,skb->dev->name); 


	SMUX_PKT_DUMP_SKB(pkt_dump_idx, info, skb, 64);
	_process_packet(dir, skb, realdev_cb, info);
	skb = info->skb;
	SMUX_PKT_DUMP_SKB(pkt_dump_idx, info, skb, 64);
	
	//if no any rule to change TARGET, use the default policy rule of the chain 
	if(info->target_jumped == 0) 
		info->target = realdev_cb->default_policy[dir];
	
#ifdef CONFIG_RTK_APP_PORT_BINDING_SUPPORT
	if(skb->dev)
		skb->from_dev = skb->dev;   //update from_dev for igmp:from_dev_ifindex in ipv4_pktinfo_prepare() and rtk_recv_message()
#endif
	*pskb = skb;

	if(unlikely((!skb->dev || info->target == SMUX_TARGET_DROP)))
	{
		if(!skb->dev)
			SMUX_DBG(SMUX_DEBUG_INDEX_ERROR, NULL, "%s: Drop it, dev=NULL, skb:%p", __FUNCTION__, skb);
		SMUX_DBG(debug_idx, info, "jump to target DROP !");
		kfree_skb(skb);
		if (smux_priv_flags_get(realdev_cb->realdev,PRIV_VSMUX)) {
			smux_dev_priv(realdev_cb->realdev)->net_stats.rx_dropped++;
		}
		else {
			realdev_cb->realdev->stats.rx_dropped++;
		}
		SMUX_DBG(debug_idx, info, "==============[Rx packet boundary end]==============");
		ret = RX_HANDLER_CONSUMED;
		goto done;
	}
	else if(likely(info->target == SMUX_TARGET_CONTINUE))
	{
		SMUX_DBG(debug_idx, info, "jump to target CONTINUE !");
	}
	else if(likely(info->target == SMUX_TARGET_ACCEPT))
	{
		SMUX_DBG(debug_idx, info, "jump to target ACCEPT !");
	}
	
	if (smux_priv_flags_get(realdev_cb->realdev,PRIV_VSMUX)) {
		smux_dev_priv(realdev_cb->realdev)->net_stats.rx_packets++;
		smux_dev_priv(realdev_cb->realdev)->net_stats.rx_bytes += skb->len;
		if(skb->pkt_type == PACKET_MULTICAST)
			smux_dev_priv(realdev_cb->realdev)->net_stats.multicast++;
	}
	else {
		realdev_cb->realdev->stats.rx_packets++;
		realdev_cb->realdev->stats.rx_bytes += skb->len;
		if(skb->pkt_type == PACKET_MULTICAST)
			realdev_cb->realdev->stats.multicast++;
	}

	if(skb->pkt_type == PACKET_OTHERHOST)
	{
		/* Our lower layer thinks this is not local, let's make sure.
		 * This allows the VLAN to have a different MAC than the underlying
		 * device, and still route correctly.
		 */
		if (ether_addr_equal(eth_hdr(skb)->h_dest, skb->dev->dev_addr))
			skb->pkt_type = PACKET_HOST;
	}

	if(skb->dev != realdev_cb->realdev)
	{
		if(smux_priv_flags_get(skb->dev,PRIV_RSMUX) && (skb->dev->flags & IFF_UP))
		{
			SMUX_DBG(debug_idx, info, "==============[Rx packet boundary end]==============");
			SMUX_DBG(debug_idx, info, "==============[Call netif_rx]==============");
			netif_rx(skb);
			ret = RX_HANDLER_CONSUMED;	
			goto done;
		}
	}

	if(ret == RX_HANDLER_PASS)
	{
		if(smux_configure.auto_reserve_vlan && info->vtag_count > 0)
		{
			skb = smux_save_skb_vlan(skb, info);
			*pskb = skb;
		}
		
		if(skb->dev != realdev_cb->realdev)
		{
			if (smux_priv_flags_get(skb->dev,PRIV_VSMUX)) {
				smux_dev_priv(skb->dev)->net_stats.rx_packets++;
				smux_dev_priv(skb->dev)->net_stats.rx_bytes += skb->len;
				if(skb->pkt_type == PACKET_MULTICAST)
					smux_dev_priv(skb->dev)->net_stats.multicast++;
			}
			else {
				skb->dev->stats.rx_packets++;
				skb->dev->stats.rx_bytes += skb->len;
				if(skb->pkt_type == PACKET_MULTICAST)
					skb->dev->stats.multicast++;
			}
		}
	}
	
	SMUX_DBG(debug_idx, info, "==============[Rx packet boundary end]==============");

done:	
	return ret;
}

static const struct file_operations smuxdev_fops = {
    .owner			= THIS_MODULE,
    //.write			= smuxdev_write,
    //.open			= smuxdev_open,
    //.release		= smuxdev_close,	
    .llseek 		= no_llseek,
	.unlocked_ioctl = smuxdev_ioctl,
};

struct miscdevice smux_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = SMUXDEV_NODE_NAME,
    .fops = &smuxdev_fops,
};

static int smux_handle_event(struct notifier_block *unused, unsigned long event, void *ptr) {
	struct net_device *dev = netdev_notifier_info_to_dev(ptr);	
	struct net_device *virt_dev;
	struct smux_realdev_cb_t *realdev_cb;
	struct smux_dev_priv *priv;
		
	realdev_cb = smux_realdev_cb_get(dev);
	if (NULL == realdev_cb)
		return NOTIFY_DONE;
	
	priv = smux_dev_priv(dev);
	
	switch(event) {
	case NETDEV_DOWN:
		SMUX_DBG(SMUX_DEBUG_INDEX_MISC, NULL, "SMUX:%s down",dev->name);
		rcu_read_lock();		
		list_for_each_entry(priv, &realdev_cb->list_virt, list_virt) {
			virt_dev = priv->virt_dev;
			if (!(virt_dev->flags & IFF_UP))
				continue;
			rcu_read_unlock();
			#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0)
			dev_change_flags(virt_dev, virt_dev->flags & ~IFF_UP);
			#else
			dev_change_flags(virt_dev, virt_dev->flags & ~IFF_UP, NULL);
			#endif
			rcu_read_lock();
			//netif_stacked_transfer_operstate(dev, priv->virt_dev);		
		}
		rcu_read_unlock();
		
		break;
#if 0 //not change vif when rif up
	case NETDEV_UP:
		SMUX_DBG(SMUX_DEBUG_INDEX_MISC, NULL, "SMUX:%s up",dev->name);
		rcu_read_lock();		
		list_for_each_entry(priv, &realdev_cb->list_virt, list_virt) {
			virt_dev = priv->virt_dev;
			if (virt_dev->flags & IFF_UP)
				continue;
			rcu_read_unlock();
			#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0)
			dev_change_flags(virt_dev, virt_dev->flags | IFF_UP);
			#else
			dev_change_flags(virt_dev, virt_dev->flags | IFF_UP,NULL);
			#endif
			//netif_stacked_transfer_operstate(dev, priv->virt_dev);
			rcu_read_lock();
		}
		rcu_read_unlock();
		
		break;
#endif
	case NETDEV_CHANGE:
		//SMUX_DBG(SMUX_DEBUG_INDEX_MISC, NULL, "SMUX:%s down",dev->name);			
		rcu_read_lock();		
		list_for_each_entry(priv, &realdev_cb->list_virt, list_virt) {
			rcu_read_unlock();
			//netif_stacked_transfer_operstate(dev, priv->virt_dev);
			if (dev->operstate == IF_OPER_DORMANT)
				netif_dormant_on(priv->virt_dev);
			else
				netif_dormant_off(priv->virt_dev);
			
			if (netif_carrier_ok(dev)) 
			{
				//EPON and GPON mode smux carrier by OAM or OMCI
				if(priv->carrier_mode == SMUX_CARRIER_AUTO) 
				{
					if (!netif_carrier_ok(priv->virt_dev)){
						printk("[SMUX] %s carrier on !!!\n", priv->virt_dev->name);
						netif_carrier_on(priv->virt_dev);
					}
				}
			} else {
				if (netif_carrier_ok(priv->virt_dev)){
					printk("[SMUX] %s carrier off !!!\n", priv->virt_dev->name);
					netif_carrier_off(priv->virt_dev);
					memset(&(smux_dev_priv(priv->virt_dev)->net_stats), 0 ,sizeof(smux_dev_priv(priv->virt_dev)->net_stats));
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
#ifdef CONFIG_COMMON_RT_API
					{
						rt_stat_netif_mib_t netifMibCnt = {0};
						if(rt_stat_netifMib_get(priv->virt_dev->name, &netifMibCnt) == RT_ERR_OK)
						{

							rt_stat_netifMib_reset(priv->virt_dev->name);
						}
					}
#endif
#endif
				}
			}
			rcu_read_lock();			
		}
		rcu_read_unlock();				
		break;
		
	case NETDEV_UNREGISTER:
		if (NULL != realdev_cb && !smux_priv_flags_get(dev,PRIV_VSMUX))
		{
			SMUX_DBG(SMUX_DEBUG_INDEX_MISC, NULL, "SMUX:%s unregister",dev->name);
			//rcu_read_lock();
			smux_realdev_cb_delete(realdev_cb);
			priv->real_dev_priv = NULL;
			smux_priv_flags_mask_clear(dev,PRIV_RSMUX);
			//rcu_read_unlock();
		}
		break;
	}
//out:
	return NOTIFY_DONE;	
}

static int proc_smuxdev_rules_fops_read(struct seq_file *seq, void *v)
{
	struct smux_realdev_cb_t  *realdev_cb, *tmp;
	int dir, depth;

	rtnl_lock();
	list_for_each_entry_safe(realdev_cb, tmp, &real_dev_list, list_real) {
		for(dir=SMUX_DIR_RX;dir<SMUX_DIR_MAX;dir++)
			for(depth=0;depth<=SMUX_MAX_TAGS;depth++)
				smux_show_table(realdev_cb->realdev->name, dir, depth);
	}
	rtnl_unlock();
	
	return 0;
}

static int proc_smuxdev_rules_fops_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_smuxdev_rules_fops_read, inode->i_private);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops smuxdev_rules_fops = {
        .proc_open           = proc_smuxdev_rules_fops_open,
        .proc_read           = seq_read,
        .proc_write          = NULL,
        .proc_lseek         = seq_lseek,
        .proc_release        = single_release,
};
#else
static const struct file_operations smuxdev_rules_fops = {
        .owner          = THIS_MODULE,
        .open           = proc_smuxdev_rules_fops_open,
        .read           = seq_read,
        .write          = NULL,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

static int proc_smuxdev_intf_fops_read(struct seq_file *seq, void *v)
{
	struct smux_realdev_cb_t  *realdev_cb, *tmp;
	struct smux_dev_priv *priv;
	unsigned char *mac;
	int i;
	
	seq_printf(seq, "  %12s  %12s  %-12s  %10s  %10s  %7s\n", "Real-If", "Virtual-If", "MACADDR", "Flags", "Priv-Flags", "Carrier");
	seq_printf(seq, "--------------------------------------------------------------------------------------\n");
	rtnl_lock();	
	list_for_each_entry_safe(realdev_cb, tmp, &real_dev_list, list_real) {
		seq_printf(seq, "  %12s    policy: ", realdev_cb->realdev->name);
		for(i=SMUX_DIR_RX;i<SMUX_DIR_MAX;i++){
			seq_printf(seq, "%s[%s] ",dir_str[i], target_str[realdev_cb->default_policy[i]]);
		}
		if(realdev_cb->rx_multi_mode == SMUX_RX_MULTI_MATCH)
			seq_printf(seq,"  RX_MULTI");
		seq_printf(seq, "  \n");
		rcu_read_lock();
		list_for_each_entry_rcu(priv, &realdev_cb->list_virt, list_virt) {
			mac = priv->virt_dev->dev_addr;
			seq_printf(seq, "  %12s  %12s  %02X%02X%02X%02X%02X%02X  %10x  %10x  %7s\n", "",  priv->virt_dev->name, 
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], priv->virt_dev->flags, priv->virt_dev->priv_flags, cmode_str[priv->carrier_mode]);
		}
		rcu_read_unlock();
		seq_printf(seq, "--------------------------------------------------------------------------------------\n");
	}
	rtnl_unlock();
	
	return 0;
}

static int proc_smuxdev_intf_fops_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_smuxdev_intf_fops_read, inode->i_private);
}

static ssize_t proc_smuxdev_intf_fops_write(struct file *filp, const char *buf, size_t count, loff_t *offp)
{
	char tmpbuf[128] = {0}, *val, *split_str;	
	char ifname[IFNAMSIZ+1]={0}, mode[32]={0};
	int len = (count > sizeof(tmpbuf)-1) ? sizeof(tmpbuf)-1 : count;
	char	*strptr, *var, *tokptr;
	struct net_device *dev;

	if (buf && !copy_from_user(tmpbuf, buf, len))
	{
		tmpbuf[len] = '\0';
		if((val = strchr(tmpbuf, '\r'))) *val='\0';
		if((val = strchr(tmpbuf, '\n'))) *val='\0';
		strptr=tmpbuf;
		while(strptr != NULL)
		{
			split_str=strsep(&strptr," ");
			if(strcasecmp(split_str, "carrier")==0)
			{
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");				
				memset(ifname, 0, sizeof(ifname));
				strncpy(ifname, split_str, sizeof(ifname)-1);
				dev = get_virt_dev(NULL, ifname);
				if (NULL==dev)
				{
					printk("[SMUX] Cannot find dev: %s !\n", ifname);
					//SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s cannot find dev: %s !\n", __func__, ifname);
					goto errout;
				}
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				if(!strcmp("on", split_str))
				{
					if(!netif_carrier_ok(dev)){
						printk("[SMUX] %s carrier on !!!\n", dev->name);
						netif_carrier_on(dev);
					}
				}
				else if(!strcmp("off", split_str))
				{
					if(netif_carrier_ok(dev)){
						printk("[SMUX] %s carrier off !!!\n", dev->name);
						netif_carrier_off(dev);
						memset(&dev->stats, 0 ,sizeof(dev->stats));
						if (smux_priv_flags_get(dev,PRIV_VSMUX)) {
							memset(&(smux_dev_priv(dev)->net_stats), 0 ,sizeof(smux_dev_priv(dev)->net_stats));
						}
						else {
							memset(&dev->stats, 0 ,sizeof(dev->stats));
						}
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
#ifdef CONFIG_COMMON_RT_API
						{
							rt_stat_netif_mib_t netifMibCnt = {0};
							if(rt_stat_netifMib_get(dev->name, &netifMibCnt) == RT_ERR_OK)
							{
								rt_stat_netifMib_reset(dev->name);
							}
						}
#endif
#endif
					}
				}	
			}
#if 0 //defined(CONFIG_LUNA)	
			if(strcasecmp(split_str, "stats")==0)
			{
				struct rtnl_link_stats64 stats64;
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				memset(ifname, 0, sizeof(ifname));
				strncpy(ifname, split_str, sizeof(ifname)-1);
				dev = get_virt_dev(NULL, ifname);
				if (NULL==dev)
				{
					printk("[SMUX] Cannot find dev: %s !\n", ifname);
					//SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s cannot find dev: %s !\n", __func__, ifname);
					goto errout;
				}
				if(strptr==NULL) break;
				dev_get_stats(dev, &stats64);
				while(strptr != NULL)
				{
					split_str=strsep(&strptr," ");
					if(!strcmp("rx_bytes", split_str))
					{
						if(strptr==NULL) break;
						split_str = strsep(&strptr," ");
						stats64.rx_bytes = simple_strtoll(split_str, NULL, 10);
					}
					else if(!strcmp("rx_packets", split_str))
					{
						if(strptr==NULL) break;
						split_str = strsep(&strptr," ");
						stats64.rx_packets = simple_strtoll(split_str, NULL, 10);
					}
					else if(!strcmp("tx_bytes", split_str))
					{
						if(strptr==NULL) break;
						split_str = strsep(&strptr," ");
						stats64.tx_bytes = simple_strtoll(split_str, NULL, 10);
					}
					else if(!strcmp("tx_packets", split_str))
					{
						if(strptr==NULL) break;
						split_str = strsep(&strptr," ");
						stats64.tx_packets = simple_strtoll(split_str, NULL, 10);
					}
					else if(!strcmp("rx_dropped", split_str))
					{
						if(strptr==NULL) break;
						split_str = strsep(&strptr," ");
						stats64.rx_dropped = simple_strtoll(split_str, NULL, 10);
					}
					else if(!strcmp("tx_dropped", split_str))
					{
						if(strptr==NULL) break;
						split_str = strsep(&strptr," ");
						stats64.tx_dropped = simple_strtoll(split_str, NULL, 10);
					}
					else if(!strcmp("multicast", split_str))
					{
						if(strptr==NULL) break;
						split_str = strsep(&strptr," ");
						stats64.multicast = simple_strtoll(split_str, NULL, 10);
					}
					else if(!strcmp("reset", split_str))
					{
						memset(&stats64, 0, sizeof(struct rtnl_link_stats64));
						stats64.multicast = simple_strtoll(split_str, NULL, 10);
					}
					else break;
				}
				
				SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "Update IF(%s) stats \n", ifname);
				
				if (dev->type == ARPHRD_PPP)
				{
					if (p_rtk_ppp_stat_set) {
						p_rtk_ppp_stat_set(dev, &stats64);
					}
				}
				else if ((smux_priv_flags_get(dev, PRIV_VSMUX)==PRIV_VSMUX))
				{
					memcpy(&(smux_dev_priv(dev)->net_stats), &stats64, sizeof(struct rtnl_link_stats64));
				}
				
#ifdef CONFIG_COMMON_RT_API
				rt_stat_netifMib_reset(ifname);
#endif
			}
#endif
		}

		return count;
	}

errout:
	printk("echo by:\n");
	printk("	carrier [ifname] [on/off]\n");
	printk("	stats [ifname] [rx_bytes/rx_packets/tx_bytes/tx_packets/rx_dropped/tx_dropped/multicast/reset] [num]\n");
	return -EFAULT;
}  

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops smuxdev_intf_fops = {
        .proc_open           = proc_smuxdev_intf_fops_open,
        .proc_write          = proc_smuxdev_intf_fops_write,
        .proc_read           = seq_read,
        .proc_lseek         = seq_lseek,
        .proc_release        = single_release,
};
#else
static const struct file_operations smuxdev_intf_fops = {
        .owner          = THIS_MODULE,
        .open           = proc_smuxdev_intf_fops_open,
        .write          = proc_smuxdev_intf_fops_write,
        .read           = seq_read,        
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

static int proc_smuxdev_listintf_fops_read(struct seq_file *seq, void *v)
{
	struct smux_realdev_cb_t  *realdev_cb, *tmp;
	struct smux_dev_priv *priv;
	unsigned char *mac;
	
	seq_printf(seq, "=  %12s  %12s  %12s  %-12s  =\n", "IfIndex", "Real-If", "Virtual-If", "MACADDR");
	seq_printf(seq, "------------------------------------------------------------\n");
	rtnl_lock();	
	list_for_each_entry_safe(realdev_cb, tmp, &real_dev_list, list_real) {
		rcu_read_lock();
		list_for_each_entry_rcu(priv, &realdev_cb->list_virt, list_virt) {
			mac = priv->virt_dev->dev_addr;
			seq_printf(seq, "   %12d  %12s  %12s  %02X%02X%02X%02X%02X%02X \n", priv->virt_dev->ifindex, realdev_cb->realdev->name,  priv->virt_dev->name, 
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}
		rcu_read_unlock();
	}
	rtnl_unlock();
	
	return 0;
}

static int proc_smuxdev_listintf_fops_open(struct inode *inode, struct file *file)
{
	return single_open(file, proc_smuxdev_listintf_fops_read, inode->i_private);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops smuxdev_listintf_fops = {
        .proc_open           = proc_smuxdev_listintf_fops_open,
        .proc_write          = NULL,
        .proc_read           = seq_read,
        .proc_lseek         = seq_lseek,
        .proc_release        = single_release,
};
#else
static const struct file_operations smuxdev_listintf_fops = {
        .owner          = THIS_MODULE,
        .open           = proc_smuxdev_listintf_fops_open,
        .write          = NULL,
        .read           = seq_read,        
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

static struct notifier_block smux_notifier_block __read_mostly = {
	.notifier_call = smux_handle_event,
};

static ssize_t proc_smuxdev_debug_fops_write(struct file *filp, const char *buf, size_t count, loff_t *offp)
{
     char tmpbuf[16] = {0};
     int len = (count > 15) ? 15 : count;
    
	 if (buf && !copy_from_user(tmpbuf, buf, len))
     {
		smux_debug = simple_strtoul(tmpbuf, NULL, 16);
		return count;
     }

	 return -EFAULT;
}   

static int proc_smuxdev_debug_fops_read(struct seq_file *seq, void *v)
{
	int i;
	
	seq_printf(seq, "smux_debug=0x%lx\n",smux_debug);
	for(i=0 ; i<SMUX_DEBUG_INDEX_MAX ; i++)
	{
		seq_printf(seq, "[0x%02x][%s:%s]\n", debug_table[i].debug_level, debug_table[i].debug_level_str, (smux_debug&debug_table[i].debug_level)?"on":"off");
	}
    return 0;
}

static int proc_smuxdev_debug_fops_open(struct inode *inode, struct file *file)
{
        return single_open(file, proc_smuxdev_debug_fops_read, inode->i_private);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops smuxdev_debug_fops = {
        .proc_open           = proc_smuxdev_debug_fops_open,
        .proc_write          = proc_smuxdev_debug_fops_write,
        .proc_read           = seq_read,
        .proc_lseek         = seq_lseek,
        .proc_release        = single_release,
};
#else
static const struct file_operations smuxdev_debug_fops = {
        .owner          = THIS_MODULE,
        .open           = proc_smuxdev_debug_fops_open,
        .write          = proc_smuxdev_debug_fops_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

static ssize_t proc_smuxdev_filter_fops_write(struct file *filp, const char *buf, size_t count, loff_t *offp)
{
	//SPA,DA,SA,ETH,SIP,DIP,IP,L4PROTO,SPORT,DPORT,TPID,VID,SIP6,DIP6
	char tmpbuf[1024] = {0}, *strptr = NULL, *split_str = NULL, *val;
	int len = sizeof(tmpbuf)-1, i, config_first=1;
	traceFilterRule *f = NULL;
	
	memset(tmpbuf, 0, sizeof(tmpbuf));
	if (buf && !copy_from_user(tmpbuf, buf, len))
	{
		if((val = strchr(tmpbuf, '\r'))) *val='\0';
		if((val = strchr(tmpbuf, '\n'))) *val='\0';
		//memset(smux_configure.trace_filter_rules, 0, sizeof(smux_configure.trace_filter_rules));
		smux_configure.trace_filter = 0;
		f = &(smux_configure.trace_filter_rules[0]);
		strptr = tmpbuf;
		while(strptr != NULL)
		{
			split_str=strsep(&strptr," ");
next_token:
			if(strcasecmp(split_str,"RULE")==0){
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");				
				i=simple_strtol(split_str, NULL, 0);
				if(i>=SMUX_TRACE_FILETR_RULES_MAX || i<0) 
					i=0;
				f = &(smux_configure.trace_filter_rules[i]);
				memset(f, 0, sizeof(traceFilterRule));
			}
			else if(config_first){
				memset(f, 0, sizeof(traceFilterRule));
				config_first=0;
			}
			
			if(strcasecmp(split_str,"IF")==0){
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				if(split_str && *split_str != '\0'){
					strncpy(f->ifname, split_str, sizeof(f->ifname)-1);
					f->bits |= 1<<SMUX_TRACE_FILETR_IF;
				}
			}
			else if(strcasecmp(split_str,"SA")==0){
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				val = f->sa;
				if(!mac_pton(split_str, val))
					goto next_token;
				f->bits |= 1<<SMUX_TRACE_FILETR_SA;
				memset(f->sa_mask, 0xff, sizeof(f->sa_mask));
				val = f->sa_mask;
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				if(!mac_pton(split_str, val)){
					memset(f->sa_mask, 0xff, sizeof(f->sa_mask));
					goto next_token;
				}
			}
			else if(strcasecmp(split_str,"DA")==0){
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				val = f->da;
				if(!mac_pton(split_str, val))
					goto next_token;
				f->bits |= 1<<SMUX_TRACE_FILETR_DA;
				memset(f->da_mask, 0xff, sizeof(f->da_mask));
				val = f->da_mask;
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				if(!mac_pton(split_str, val)){
					memset(f->da_mask, 0xff, sizeof(f->da_mask));
					goto next_token;
				}
			}
			else if(strcasecmp(split_str,"ETH")==0){
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				f->bits |= 1<<SMUX_TRACE_FILETR_ETH;
				//don't do endian, because build_info will do
				f->eth_proto = simple_strtol(split_str, NULL, 16);
			}
			else if(strcasecmp(split_str,"SIP")==0){
				struct sockaddr_in ip;
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				f->bits |= 1<<SMUX_TRACE_FILETR_SIP;
				f->sip = in_aton(split_str);
			}
			else if(strcasecmp(split_str,"DIP")==0){
				struct sockaddr_in ip;
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				f->bits |= 1<<SMUX_TRACE_FILETR_DIP;
				f->dip = in_aton(split_str);
			}
			else if(strcasecmp(split_str,"SIP6")==0){
				struct sockaddr_in ip;
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				f->bits |= 1<<SMUX_TRACE_FILETR_SIP6;
				in6_pton(split_str,-1,((u8 *)&(f->sip6)),-1,NULL);
			}
			else if(strcasecmp(split_str,"DIP6")==0){
				struct sockaddr_in ip;
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				f->bits |= 1<<SMUX_TRACE_FILETR_DIP6;
				in6_pton(split_str,-1,((u8 *)&(f->dip6)),-1,NULL);
			}
			else if(strcasecmp(split_str,"L4PROTO")==0){
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				f->bits |= 1<<SMUX_TRACE_FILETR_L4PROTO;
				f->proto = (u8)simple_strtol(split_str, NULL, 16);
			}
			else if(strcasecmp(split_str,"SPORT")==0){
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				f->sport = (u16)htons(simple_strtol(split_str, NULL, 10));
					f->bits |= 1<<SMUX_TRACE_FILETR_SPORT;
			}
			else if(strcasecmp(split_str,"DPORT")==0){
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				f->dport = (u16)htons(simple_strtol(split_str, NULL, 10));
					f->bits |= 1<<SMUX_TRACE_FILETR_DPORT;
			} 
			else if(strcasecmp(split_str,"TPID1")==0){
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				f->vlan[0].vlan_proto = (u16)htons(simple_strtol(split_str, NULL, 16));
				f->bits |= 1<<SMUX_TRACE_FILETR_TPID1;
			}
			else if(strcasecmp(split_str,"VID1")==0){
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				f->vlan[0].vlan_tci = (u16)htons(simple_strtol(split_str, NULL, 10) & VLAN_VID_MASK);
				f->bits |= 1<<SMUX_TRACE_FILETR_VID1;
			}
			else if(strcasecmp(split_str,"TPID2")==0){
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				f->vlan[1].vlan_proto = (u16)htons(simple_strtol(split_str, NULL, 16));
				f->bits |= 1<<SMUX_TRACE_FILETR_TPID2;
			}
			else if(strcasecmp(split_str,"VID2")==0){
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				f->vlan[1].vlan_tci = (u16)htons(simple_strtol(split_str, NULL, 10) & VLAN_VID_MASK);
				f->bits |= 1<<SMUX_TRACE_FILETR_VID2;
			}
			else if(strcasecmp(split_str,"TPID3")==0){
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				f->vlan[2].vlan_proto = (u16)htons(simple_strtol(split_str, NULL, 16));
				f->bits |= 1<<SMUX_TRACE_FILETR_TPID3;
			}
			else if(strcasecmp(split_str,"VID3")==0){
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				f->vlan[2].vlan_tci = (u16)htons(simple_strtol(split_str, NULL, 10) & VLAN_VID_MASK);
				f->bits |= 1<<SMUX_TRACE_FILETR_VID3;
			}
			else if(strcasecmp(split_str,"TPID4")==0){
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				f->vlan[3].vlan_proto = (u16)htons(simple_strtol(split_str, NULL, 16));
				f->bits |= 1<<SMUX_TRACE_FILETR_TPID4;
			}
			else if(strcasecmp(split_str,"VID4")==0){
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				f->vlan[3].vlan_tci = (u16)htons(simple_strtol(split_str, NULL, 10) & VLAN_VID_MASK);
				f->bits |= 1<<SMUX_TRACE_FILETR_VID4;
			}
			else if(strcasecmp(split_str,"TIMES")==0){
				if(strptr==NULL) break;
				split_str=strsep(&strptr," ");
				f->times = (int)simple_strtol(split_str, NULL, 10);
				f->bits |= 1<<SMUX_TRACE_FILETR_TIMES;
			}
		}
		
		for(i=0;i<SMUX_TRACE_FILETR_RULES_MAX;i++)
		{
			if(smux_configure.trace_filter_rules[i].bits > 0){
				smux_configure.trace_filter = 1;
				break;
			}
		}
		return count;
	}

	return -EFAULT;
}   

static int proc_smuxdev_filter_fops_read(struct seq_file *seq, void *v)
{
	int i,j;
	traceFilterRule *f = NULL;
	seq_printf(seq,"== Filetr Items: RULE,IF,SA,DA,ETH,TPID1,VID1,TPID2,VID2,TPID3,VID3,TPID4,VID4,SIP,DIP,SIP6,DIP6,L4PROTO,SPORT,DPORT,TIMES ==\n");
	for(i=0;i<SMUX_TRACE_FILETR_RULES_MAX;i++)
	{
		f = &(smux_configure.trace_filter_rules[i]);
		seq_printf(seq,"[Rule %d]:\n", i);
		for(j=0;j<SMUX_TRACE_FILETR_MAX;j++)
		{
			if(!(f->bits & (1<<j))) continue;
			seq_printf(seq, "    ");
			switch(j)
			{
				case SMUX_TRACE_FILETR_IF:
					seq_printf(seq,"IF %s\n", f->ifname);
					break;
				case SMUX_TRACE_FILETR_SA:
					seq_printf(seq,"SA %02X:%02X:%02X:%02X:%02X:%02X  %02X:%02X:%02X:%02X:%02X:%02X\n",
						f->sa[0],f->sa[1],f->sa[2],f->sa[3],f->sa[4],f->sa[5],
						f->sa_mask[0],f->sa_mask[1],f->sa_mask[2],f->sa_mask[3],f->sa_mask[4],f->sa_mask[5]);
					break;
				case SMUX_TRACE_FILETR_DA:
					seq_printf(seq,"DA %02X:%02X:%02X:%02X:%02X:%02X  %02X:%02X:%02X:%02X:%02X:%02X\n",
						f->da[0],f->da[1],f->da[2],f->da[3],f->da[4],f->da[5],
						f->da_mask[0],f->da_mask[1],f->da_mask[2],f->da_mask[3],f->da_mask[4],f->da_mask[5]);
					break;
				case SMUX_TRACE_FILETR_ETH:
					seq_printf(seq,"ETH 0x%04X\n", f->eth_proto);
					break;
				case SMUX_TRACE_FILETR_TPID1:
					seq_printf(seq,"TPID1 0x%04X\n", ntohs(f->vlan[0].vlan_proto));
					break;
				case SMUX_TRACE_FILETR_VID1:
					seq_printf(seq,"VID1 %u\n", (ntohs(f->vlan[0].vlan_tci)&VLAN_VID_MASK));
					break;
				case SMUX_TRACE_FILETR_TPID2:
					seq_printf(seq,"TPID2 0x%04X\n", ntohs(f->vlan[1].vlan_proto));
					break;
				case SMUX_TRACE_FILETR_VID2:
					seq_printf(seq,"VID2 %u\n", (ntohs(f->vlan[1].vlan_tci)&VLAN_VID_MASK));
					break;
				case SMUX_TRACE_FILETR_TPID3:
					seq_printf(seq,"TPID3 0x%04X\n", ntohs(f->vlan[2].vlan_proto));
					break;
				case SMUX_TRACE_FILETR_VID3:
					seq_printf(seq,"VID3 %u\n", (ntohs(f->vlan[2].vlan_tci)&VLAN_VID_MASK));
					break;
				case SMUX_TRACE_FILETR_TPID4:
					seq_printf(seq,"TPID4 0x%04X\n", ntohs(f->vlan[3].vlan_proto));
					break;
				case SMUX_TRACE_FILETR_VID4:
					seq_printf(seq,"VID4 %u\n", (ntohs(f->vlan[3].vlan_tci)&VLAN_VID_MASK));
					break;
				case SMUX_TRACE_FILETR_SIP:
					seq_printf(seq,"SIP %pI4\n", &f->sip);
					break;
				case SMUX_TRACE_FILETR_DIP:
					seq_printf(seq,"DIP %pI4\n", &f->dip);
					break;
				case SMUX_TRACE_FILETR_SIP6:
					seq_printf(seq,"SIP6 %pI6\n", &f->sip6);
					break;
				case SMUX_TRACE_FILETR_DIP6:
					seq_printf(seq,"DIP6 %pI6\n", &f->dip6);
					break;
				case SMUX_TRACE_FILETR_L4PROTO:
					seq_printf(seq,"L4PROTO 0x%02X\n", f->proto);
					break;
				case SMUX_TRACE_FILETR_SPORT:
					seq_printf(seq,"SPORT %hu\n", ntohs(f->sport));
					break;
				case SMUX_TRACE_FILETR_DPORT:
					seq_printf(seq,"DPORT %hu\n", ntohs(f->dport));
					break;
				case SMUX_TRACE_FILETR_TIMES:
					seq_printf(seq,"TIMES %hu\n", f->times);
			}
		}
	}
    return 0;
}

static int proc_smuxdev_filter_fops_open(struct inode *inode, struct file *file)
{
        return single_open(file, proc_smuxdev_filter_fops_read, inode->i_private);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops smuxdev_filter_fops = {
        .proc_open           = proc_smuxdev_filter_fops_open,
        .proc_write          = proc_smuxdev_filter_fops_write,
        .proc_read           = seq_read,
        .proc_lseek         = seq_lseek,
        .proc_release        = single_release,
};
#else
static const struct file_operations smuxdev_filter_fops = {
        .owner          = THIS_MODULE,
        .open           = proc_smuxdev_filter_fops_open,
        .write          = proc_smuxdev_filter_fops_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

static ssize_t proc_smuxdev_configure_fops_write(struct file *filp, const char *buf, size_t count, loff_t *offp)
{
	char tmpbuf[128] = {0};	
	int len = (count > 127) ? 127 : count;
	char	*strptr, *var, *tokptr;

	if (buf && !copy_from_user(tmpbuf, buf, len))
	{
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		var = strsep(&strptr," ");
		if (var==NULL)
		{
			goto errout;
		}
		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		if(!strcmp("multicast_mode", var))
			smux_configure.multicast_mode = !!simple_strtoul(tokptr, NULL, 0);
		else if(!strcmp("dump_all", var))
			smux_configure.dump_all = !!simple_strtoul(tokptr, NULL, 0);
		else if(!strcmp("trace_filter", var))
			smux_configure.trace_filter = !!simple_strtoul(tokptr, NULL, 0);
		else if(!strcmp("hw_vlan_filter", var))
			smux_configure.hw_vlan_filter = !!simple_strtoul(tokptr, NULL, 0);
		else if(!strcmp("mapping_outermost_vlan_by_mac_from_lut", var))
			smux_configure.mapping_outermost_vlan_by_mac_from_lut = !!simple_strtoul(tokptr, NULL, 0);
		else if(!strcmp("auto_reserve_vlan", var))
			smux_configure.auto_reserve_vlan = !!simple_strtoul(tokptr, NULL, 0);
		else
			goto errout;

		printk("Set %s to %lu OK.\n", var, simple_strtoul(tokptr, NULL, 0));
		return count;
	}

errout:
	printk("echo by:\n");
	printk("	multicast_mode [0/1]\n");
	return -EFAULT;
}   

static int proc_smuxdev_configure_fops_read(struct seq_file *seq, void *v)
{
    seq_printf(seq, "multicast_mode: [%s]\n", smux_configure.multicast_mode ? "ON":"OFF");
	seq_printf(seq, "dump_all: [%s]\n", smux_configure.dump_all ? "ON":"OFF");
	seq_printf(seq, "trace_filter: [%s]\n", smux_configure.trace_filter ? "ON":"OFF");
	seq_printf(seq, "hw_vlan_filter: [%s]\n", smux_configure.hw_vlan_filter ? "ON":"OFF");
	seq_printf(seq, "mapping_outermost_vlan_by_mac_from_lut: [%s]\n", smux_configure.mapping_outermost_vlan_by_mac_from_lut ? "ON":"OFF");
	seq_printf(seq, "auto_reserve_vlan: [%s]\n", smux_configure.auto_reserve_vlan ? "ON":"OFF");
    return 0;
}

static int proc_smuxdev_configure_fops_open(struct inode *inode, struct file *file)
{
        return single_open(file, proc_smuxdev_configure_fops_read, inode->i_private);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops smuxdev_configure_fops = {
        .proc_open           = proc_smuxdev_configure_fops_open,
        .proc_write          = proc_smuxdev_configure_fops_write,
        .proc_read           = seq_read,
        .proc_lseek         = seq_lseek,
        .proc_release        = single_release,
};
#else
static const struct file_operations smuxdev_configure_fops = {
        .owner          = THIS_MODULE,
        .open           = proc_smuxdev_configure_fops_open,
        .write          = proc_smuxdev_configure_fops_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

static ssize_t proc_smuxdev_dscp_mapping_fops_write(struct file *filp, const char *buf, size_t count, loff_t *offp)
{
	char tmpbuf[128] = {0};	
	int len = (count > 127) ? 127 : count;
	char	*strptr, *var, *tokptr;
	struct smux_qos_rule_t	qos;
	struct net_device *dev;
	int method, ret;
	const char *ifname = NULL;

	if (buf && !copy_from_user(tmpbuf, buf, len))
	{
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		var = strsep(&strptr," \n");
		if (var==NULL)
		{
			goto errout;
		}
		
		if(!strcmp("dscp_mapping_change_default", var))
			goto skip_dev_check;

		tokptr = strsep(&strptr," \n");
		if (tokptr==NULL)
		{			
			goto errout;
		}
		
		dev = get_virt_dev(NULL, tokptr);
		if (NULL==dev)
		{
			SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s cannot find dev: %s !\n", __func__, tokptr);
			goto errout;
		}

		if (!smux_priv_flags_get(dev,PRIV_RSMUX))
		{
			SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s dev(%s) is not a real smux device !\n", __func__, tokptr);
			goto errout;
		}
		
		ifname = dev->name;
		
skip_dev_check:
		memset(&qos, 0, sizeof(qos));
		
		if(!strcmp("dscp_mapping_enable", var))
		{
			tokptr = strsep(&strptr," \n");
			if (tokptr==NULL)
			{
				goto errout;
			}
			qos.dscp_mapping_enable = !!simple_strtoul(tokptr, NULL, 0);
			qos.vlan_idx = 1;
			method = SMUX_QOS_METHOD_DSCP_MAPPING_ENABLE;
		}	
		else if(!strcmp("dscp_mapping_vlan_idx", var))
		{
			int vlan_idx;
			tokptr = strsep(&strptr," \n");
			if (tokptr==NULL)
			{
				goto errout;
			}
			vlan_idx = simple_strtoul(tokptr, NULL, 0);
			if(vlan_idx<1 || vlan_idx>SMUX_MAX_TAGS)
			{
				SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s invalid VLAN index=%d.\n", __func__, vlan_idx);
				goto errout;
			}
			qos.vlan_idx = vlan_idx;
			method = SMUX_QOS_METHOD_DSCP_MAPPING_ENABLE;
		}
		else if(!strcmp("dscp_mapping_set", var))
		{
			char dscp, priority;
			tokptr = strsep(&strptr," \n");
			if (tokptr==NULL)
			{
				goto errout;
			}
			dscp = simple_strtoul(tokptr, NULL, 0);
			tokptr = strsep(&strptr," \n");
			if (tokptr==NULL)
			{
				goto errout;
			}
			priority = simple_strtoul(tokptr, NULL, 0);
			qos.dscp = dscp;
			qos.priority = priority;
			method = SMUX_QOS_METHOD_DSCP_MAPPING_SET;
		}
		else if(!strcmp("dscp_mapping_default", var))
		{
			method = SMUX_QOS_METHOD_DSCP_MAPPING_DEFAULT;
		}
		else if(!strcmp("dscp_mapping_change_default", var))
		{
			char dscp, priority;
			tokptr = strsep(&strptr," \n");
			if (tokptr==NULL)
			{
				goto errout;
			}
			dscp = simple_strtoul(tokptr, NULL, 0);
			tokptr = strsep(&strptr," \n");
			if (tokptr==NULL)
			{
				goto errout;
			}
			priority = simple_strtoul(tokptr, NULL, 0);
			qos.dscp = dscp;
			qos.priority = priority;
			method = SMUX_QOS_METHOD_DSCP_MAPPING_CHANGE_DEFAULT;
		}
		else
			goto errout;

		ret = smux_set_qos(ifname, method, &qos);
		
		SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s Set dev %s %s %s \n", __func__, (ifname)?ifname:"", var, ((!ret)?"OK":"FAIL"));

		return count;
	}

errout:
	printk("echo by:\n");
	printk("	dscp_mapping_enable [ifname] [0/1]\n");
	printk("	dscp_mapping_vlan_idx [ifname] [1~%d]\n", SMUX_MAX_TAGS);
	printk("	dscp_mapping_set [ifname] [DSCP value] [802.1q priority]\n");
	printk("	dscp_mapping_default [ifname]\n");
	printk("	dscp_mapping_change_default [DSCP value] [802.1q priority]\n");
	return -EFAULT;
}   

static int proc_smuxdev_dscp_mapping_fops_read(struct seq_file *seq, void *v)
{
	struct smux_realdev_cb_t  *realdev_cb, *tmp;
	struct smux_vlan_tag_t *vlan_tag;
	struct smux_ip_tos_t *ip_tos;
	struct smux_dscp_mapping_entry_t *dscp_entry;
	int i;
	
	seq_printf(seq, "default table:\n");
	seq_printf(seq, "===============================================\n");
	seq_printf(seq, "  DSCP          meaning		 VLAN priority	\n");
	seq_printf(seq, "===============================================\n");
	for(i=0 ; i<SMUX_MAX_DSCP_NUM ; i++)
	{
		dscp_entry = &dscp_mapping_table_default[i];
		if(dscp_entry->enable)
		{
			ip_tos = (struct smux_ip_tos_t*)&dscp_entry->ip_tos;
			vlan_tag = &dscp_entry->vlan_tag;
			seq_printf(seq, "  0x%02x(%02d)      %-10s	 %-10d\n", IP_TOS_GET_DSCP(ip_tos), IP_TOS_GET_DSCP(ip_tos), ((dscp_entry->meaning)?dscp_entry->meaning:""), ntohs(vlan_tag->vlan_tci) >> PRIO_SHIFT);
		}
	}
	
	rtnl_lock();
	list_for_each_entry_safe(realdev_cb, tmp, &real_dev_list, list_real) {
		rcu_read_lock();
		if(realdev_cb->qos.dscp_mapping_enable && realdev_cb->qos.dscp_mapping_table != NULL)
		{
			seq_printf(seq, "table of %s (vlanidx=%d):\n", realdev_cb->realdev->name, realdev_cb->qos.vlan_idx);
			seq_printf(seq, "===============================================\n");
			seq_printf(seq, "  DSCP          meaning		 VLAN priority	\n");
			seq_printf(seq, "===============================================\n");
			for(i=0 ; i<SMUX_MAX_DSCP_NUM ; i++)
			{
				dscp_entry = &realdev_cb->qos.dscp_mapping_table[i];
				if(dscp_entry->enable)
				{
					ip_tos = (struct smux_ip_tos_t*)&dscp_entry->ip_tos;
					vlan_tag = &dscp_entry->vlan_tag;
					seq_printf(seq, "  0x%02x(%02d)      %-10s	 %-10d\n", IP_TOS_GET_DSCP(ip_tos), IP_TOS_GET_DSCP(ip_tos), ((dscp_entry->meaning)?dscp_entry->meaning:""), ntohs(vlan_tag->vlan_tci) >> PRIO_SHIFT);
				}
			}
		}
		rcu_read_unlock();
	}
	rtnl_unlock();
	
    return 0;
}

static int proc_smuxdev_dscp_mapping_fops_open(struct inode *inode, struct file *file)
{
        return single_open(file, proc_smuxdev_dscp_mapping_fops_read, inode->i_private);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops smuxdev_dscp_mapping_fops = {
        .proc_open           = proc_smuxdev_dscp_mapping_fops_open,
        .proc_write          = proc_smuxdev_dscp_mapping_fops_write,
        .proc_read           = seq_read,
        .proc_lseek         = seq_lseek,
        .proc_release        = single_release,
};
#else
static const struct file_operations smuxdev_dscp_mapping_fops = {
        .owner          = THIS_MODULE,
        .open           = proc_smuxdev_dscp_mapping_fops_open,
        .write          = proc_smuxdev_dscp_mapping_fops_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

static ssize_t proc_smuxdev_mc_gem_port_skbmark_fops_write(struct file *filp, const char *buf, size_t count, loff_t *offp)
{
	struct smux_mc_gem_port_skbmark_t mc_gem_port_skbmark_t = {0};
	char tmpbuf[512] = {0};	
	int len = (count > 511) ? 511 : count;
	char	*strptr, *var, *tokptr;
	int method, ret;

	if (buf && !copy_from_user(tmpbuf, buf, len))
	{
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		var = strsep(&strptr," ");
		if (var==NULL)
		{
			goto errout;
		}
		
		if(!strcmp("add", var))
		{
			method = SMUX_MC_GEM_PORT_SKBMARK_METHOD_ADD;
		}	
		else if(!strcmp("del", var))
		{
			method = SMUX_MC_GEM_PORT_SKBMARK_METHOD_DEL;
		}
		else
			goto errout;

		tokptr = strsep(&strptr," ");
		if (tokptr==NULL)
		{
			goto errout;
		}
		if(!strcmp("mark1", tokptr))
		{
			mc_gem_port_skbmark_t.mark_select = 1;
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			mc_gem_port_skbmark_t.mark1 = simple_strtoul(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			mc_gem_port_skbmark_t.mark_mask1 = simple_strtoul(tokptr, NULL, 0);
		}
#ifdef CONFIG_RTK_SKB_MARK2
		else if(!strcmp("mark2", tokptr))
		{
			mc_gem_port_skbmark_t.mark_select = 2;
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			mc_gem_port_skbmark_t.mark2 = simple_strtoull(tokptr, NULL, 0);
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			mc_gem_port_skbmark_t.mark_mask2 = simple_strtoull(tokptr, NULL, 0);
		}
#endif
		else
			goto errout;

		ret = smux_set_mc_gem_port_skbmark(method, &mc_gem_port_skbmark_t);
		
		SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s %s %s \n", __func__, var, ((!ret)?"OK":"FAIL"));

		return count;
	}

errout:
	printk("echo by:\n");
#ifdef CONFIG_RTK_SKB_MARK2
	printk("	add [mark1/mark2] [skbmark] [skbmarkmask]\n");
	printk("	del [mark1/mark2] [skbmark] [skbmarkmask]\n");
#else
	printk("	add [mark1] [skbmark] [skbmarkmask]\n");
	printk("	del [mark1] [skbmark] [skbmarkmask]\n");
#endif

	return -EFAULT;
}   

static int proc_smuxdev_mc_gem_port_skbmark_fops_read(struct seq_file *seq, void *v)
{
	int i;

	rcu_read_lock();	
	seq_printf(seq, "================================================================\n");
	seq_printf(seq, "                              MARK1\n");
	seq_printf(seq, "================================================================\n");
	for(i=0 ; i<SMUX_MC_GEM_PORT_SKBMARK_NUM ; i++)
	{
		if(smux_configure.mc_gem_port_skbmark_tbl[i].mark_select)
		{
			seq_printf(seq, "[%d] mark%d ", i, smux_configure.mc_gem_port_skbmark_tbl[i].mark_select);
			if(smux_configure.mc_gem_port_skbmark_tbl[i].mark_select == 1)
			{
				seq_printf(seq, "mark1=0x%08x mark_mask1=0x%08x\n", smux_configure.mc_gem_port_skbmark_tbl[i].mark1, smux_configure.mc_gem_port_skbmark_tbl[i].mark_mask1);
			}
		}
	}
#ifdef CONFIG_RTK_SKB_MARK2
	seq_printf(seq, "================================================================\n");
	seq_printf(seq, "                              MARK2\n");
	seq_printf(seq, "================================================================\n");
	for(i=0 ; i<SMUX_MC_GEM_PORT_SKBMARK_NUM ; i++)
	{
		if(smux_configure.mc_gem_port_skbmark_tbl[i].mark_select)
		{
			seq_printf(seq, "[%d] mark%d ", i, smux_configure.mc_gem_port_skbmark_tbl[i].mark_select);
			if(smux_configure.mc_gem_port_skbmark_tbl[i].mark_select == 2)
			{
				seq_printf(seq, "mark2=0x%016llx mark_mask2=0x%016llx\n", smux_configure.mc_gem_port_skbmark_tbl[i].mark2, smux_configure.mc_gem_port_skbmark_tbl[i].mark_mask2);
			}
		}
	}
#endif
	rcu_read_unlock();
	
    return 0;
}

static int proc_smuxdev_mc_gem_port_skbmark_fops_open(struct inode *inode, struct file *file)
{
        return single_open(file, proc_smuxdev_mc_gem_port_skbmark_fops_read, inode->i_private);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops smuxdev_mc_gem_port_skbmark_fops = {
        .proc_open           = proc_smuxdev_mc_gem_port_skbmark_fops_open,
        .proc_write          = proc_smuxdev_mc_gem_port_skbmark_fops_write,
        .proc_read           = seq_read,
        .proc_lseek         = seq_lseek,
        .proc_release        = single_release,
};
#else
static const struct file_operations smuxdev_mc_gem_port_skbmark_fops = {
        .owner          = THIS_MODULE,
        .open           = proc_smuxdev_mc_gem_port_skbmark_fops_open,
        .write          = proc_smuxdev_mc_gem_port_skbmark_fops_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

#ifdef CONFIG_COMMON_RT_API
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
static ssize_t proc_smuxdev_hw_vlan_filter_fops_write(struct file *filp, const char *buf, size_t count, loff_t *offp)
{
	struct smux_hw_vlan_filter_para_t mc_hw_vlan_filter_t = {0};
	char tmpbuf[512] = {0};	
	int len = (count > 511) ? 511 : count;
	char	*strptr, *var, *tokptr;
	struct net_device *dev;
	int method, ret;

	if (buf && !copy_from_user(tmpbuf, buf, len))
	{
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		var = strsep(&strptr," ");
		if (var==NULL)
		{
			goto errout;
		}

		if(strstr(var, "switch_on"))
		{
			method = SMUX_MC_HW_VLAN_FILTER_METHOD_SWITCH_ON;
			goto cmd_compelete;
		}
		else if(strstr(var, "switch_off"))
		{
			method = SMUX_MC_HW_VLAN_FILTER_METHOD_SWITCH_OFF;
			goto cmd_compelete;
		}
		else if(!strcmp("enable", var) || !strcmp("disable", var) || !strcmp("add", var) || !strcmp("del", var))
		{
			if(!strcmp("enable", var))
				method = SMUX_MC_HW_VLAN_FILTER_METHOD_ENABLE;
			else if(!strcmp("disable", var))
				method = SMUX_MC_HW_VLAN_FILTER_METHOD_DISABLE;
			else if(!strcmp("add", var))
				method = SMUX_MC_HW_VLAN_FILTER_METHOD_ADD;
			else if(!strcmp("del", var))
				method = SMUX_MC_HW_VLAN_FILTER_METHOD_DEL;

			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{			
				goto errout;
			}
			dev = __dev_get_by_name(&init_net, tokptr);
			if (NULL==dev)
			{
				tokptr[strlen(tokptr)-1] = '\0';
				dev = __dev_get_by_name(&init_net, tokptr);
				if (NULL==dev)
				{
					SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s cannot find dev: %s !\n", __func__, tokptr);
					goto errout;
				}
			}

			mc_hw_vlan_filter_t.dev = dev;

			if(!strcmp("enable", var) || !strcmp("disable", var))
			{
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
				{			
					goto errout;
				}
				tokptr[strlen(tokptr)-1] = '\0';
				
				if(!strcmp(tokptr, "UC"))
					mc_hw_vlan_filter_t.mode = 0x1;
				else if(!strcmp(tokptr, "BC"))
					mc_hw_vlan_filter_t.mode = 0x2;
				else if(!strcmp(tokptr, "MC"))
					mc_hw_vlan_filter_t.mode = 0x4;
				else if(!strcmp(tokptr, "ALL"))
					mc_hw_vlan_filter_t.mode = 0x7;
				else
					goto errout;
			}
			else
			{
				tokptr = strsep(&strptr," ");
				if (tokptr==NULL)
				{
					goto errout;
				}
				tokptr[strlen(tokptr)-1] = '\0';

				if(simple_strtoul(tokptr, NULL, 0) > 4095)
					goto errout;
				
				mc_hw_vlan_filter_t.vlan_id = simple_strtoul(tokptr, NULL, 0);
			}
		}
		else if(!strcmp("refresh", var))
		{
			method = SMUX_MC_HW_VLAN_FILTER_METHOD_REFRESH;
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}
			tokptr[strlen(tokptr)-1] = '\0';
			
			if(!strcmp(tokptr, "ALL"))
				mc_hw_vlan_filter_t.mode = 0x1;
			else
			{
				dev = __dev_get_by_name(&init_net, tokptr);
				if (NULL==dev)
				{
					tokptr[strlen(tokptr)-1] = '\0';
					dev = __dev_get_by_name(&init_net, tokptr);
					if (NULL==dev)
					{
						SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s cannot find dev: %s !\n", __func__, tokptr);
						goto errout;
					}
				}

				mc_hw_vlan_filter_t.dev = dev;
			}
		}
		else
			goto errout;
	}
	else 
		goto errout;
	
cmd_compelete:

	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s: method=%d dev=%s mode=0x%x vlan_id=%d\n", __func__, method, mc_hw_vlan_filter_t.dev->name, mc_hw_vlan_filter_t.mode, mc_hw_vlan_filter_t.vlan_id);
	ret = smux_set_hw_vlan_filter(method, &mc_hw_vlan_filter_t);
	
	SMUX_DBG(SMUX_DEBUG_INDEX_IOCTRL, NULL, "%s %s %s \n", __func__, var, ((!ret)?"OK":"FAIL"));

	return count;
errout:
	printk("echo by:\n");
	printk("	switch_on -- main switch of functon ON\n");
	printk("	switch_off -- main switch of functon OFF\n");
	printk("	enable [ifName] [UC/BC/MC/ALL] -- enable filtering mode for specific interface\n");
	printk("	disable [ifName] [UC/BC/MC/ALL] -- disable filtering mode for specific interface\n");
	printk("	add [ifName] [VID] -- add VID for specific interface into allowed list\n");
	printk("	del [ifName] [VID] -- delete VID for specific interface from allowed list\n");
	//printk("	refresh [ALL/ifName]\n");
	return -EFAULT;
}   

static int proc_smuxdev_hw_vlan_filter_fops_read(struct seq_file *seq, void *v)
{
	struct smux_realdev_cb_t  *realdev_cb, *tmp;
	int i;
	
	rcu_read_lock();
	seq_printf(seq, "global HW VLAN filter: [%s] \n", smux_configure.hw_vlan_filter ? "ON":"OFF");
	seq_printf(seq, "-------------------------------------------\n");
	list_for_each_entry_safe(realdev_cb, tmp, &real_dev_list, list_real) {
		if(!smux_priv_flags_get(realdev_cb->realdev,PRIV_VSMUX))
		{
			seq_printf(seq, "%s %s port_mask=0x%04x mode=0x%x ", realdev_cb->realdev->name, realdev_cb->hw_vlan_filter.mode?"ON":"OFF", realdev_cb->hw_vlan_filter.port_mask, realdev_cb->hw_vlan_filter.mode);
			if(realdev_cb->hw_vlan_filter.mode)
			{
				
				if(realdev_cb->hw_vlan_filter.mode == 0x7)
					seq_printf(seq, "ALL\n");
				else 
				{
					seq_printf(seq, "(");
					if(realdev_cb->hw_vlan_filter.mode&0x1)
						seq_printf(seq, "UC");
					if(realdev_cb->hw_vlan_filter.mode&0x2)
					{
						if(realdev_cb->hw_vlan_filter.mode&0x1)
							seq_printf(seq, "/BC");
						else
							seq_printf(seq, "BC");
					}
					if(realdev_cb->hw_vlan_filter.mode&0x4)
					{
						if(realdev_cb->hw_vlan_filter.mode&0x3)
							seq_printf(seq, "/MC");
						else
							seq_printf(seq, "MC");
					}
					seq_printf(seq, ")\n");
				}
				for(i=0 ; i<SMUX_HW_VLAN_FILTER_DEFAULT_NUM ; i++)
				{
					if(realdev_cb->hw_vlan_filter.default_acl_index[i] != -1)
						seq_printf(seq, "	default_acl_index[%d]=%d\n", i, realdev_cb->hw_vlan_filter.default_acl_index[i]);
				}
			}
			else
				seq_printf(seq, "none\n");

			for(i=0 ; i<SMUX_HW_VLAN_FILTER_NUM ; i++)
			{
				if(realdev_cb->hw_vlan_filter.entry[i].valid)
				{
					seq_printf(seq, "	[%d] vlan_id=%d acl_index=%d (ref_count=%d)\n", i, realdev_cb->hw_vlan_filter.entry[i].vlan_id, realdev_cb->hw_vlan_filter.entry[i].acl_index, realdev_cb->hw_vlan_filter.entry[i].ref_count);
				}
			}
			
			seq_printf(seq, "-------------------------------------------\n");
		}
	}
	rcu_read_unlock();
    return 0;
}

static int proc_smuxdev_hw_vlan_filter_fops_open(struct inode *inode, struct file *file)
{
        return single_open(file, proc_smuxdev_hw_vlan_filter_fops_read, inode->i_private);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
static const struct proc_ops smuxdev_hw_vlan_filter_fops = {
        .proc_open           = proc_smuxdev_hw_vlan_filter_fops_open,
        .proc_write          = proc_smuxdev_hw_vlan_filter_fops_write,
        .proc_read           = seq_read,
        .proc_lseek         = seq_lseek,
        .proc_release        = single_release,
};
#else
static const struct file_operations smuxdev_hw_vlan_filter_fops = {
        .owner          = THIS_MODULE,
        .open           = proc_smuxdev_hw_vlan_filter_fops_open,
        .write          = proc_smuxdev_hw_vlan_filter_fops_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#endif

#endif
#endif

static char *smux_dev_name = "rtk_smuxdev";
static struct proc_dir_entry *procfs = NULL;
static int smuxdev_init_procfs(void)
{
    struct proc_dir_entry *entry = NULL;

	/* create a directory */
	procfs = proc_mkdir(smux_dev_name, NULL);
	if(procfs == NULL)
	{
		printk("Register /proc/%s failed\n", "test");
		return -ENOMEM;
	}

	entry = proc_create("debug", 0644, procfs, &smuxdev_debug_fops);
	if (entry == NULL)
	{
		printk("Register /proc/%s/debug failed\n", smux_dev_name);
		remove_proc_entry(smux_dev_name, NULL);
		return -ENOMEM;
	}
	
	entry = proc_create("filter", 0644, procfs, &smuxdev_filter_fops);
	if (entry == NULL)
	{
		printk("Register /proc/%s/filter failed\n", smux_dev_name);
		remove_proc_entry(smux_dev_name, NULL);
		return -ENOMEM;
	}
	
	entry = proc_create("interface", 0644, procfs, &smuxdev_intf_fops);
	if (entry == NULL)
	{
		printk("Register /proc/%s/interface failed\n", smux_dev_name);
		remove_proc_entry(smux_dev_name, NULL);
		return -ENOMEM;
	}
	
	entry = proc_create("list_intf", 0644, procfs, &smuxdev_listintf_fops);
	if (entry == NULL)
	{
		printk("Register /proc/%s/list_intf failed\n", smux_dev_name);
		remove_proc_entry(smux_dev_name, NULL);
		return -ENOMEM;
	}
	
	entry = proc_create("rules", 0644, procfs, &smuxdev_rules_fops);
	if (entry == NULL)
	{
		printk("Register /proc/%s/rules failed\n", smux_dev_name);
		remove_proc_entry(smux_dev_name, NULL);
		return -ENOMEM;
	}

	entry = proc_create("configure", 0644, procfs, &smuxdev_configure_fops);
	if (entry == NULL)
	{
		printk("Register /proc/%s/configure failed\n", smux_dev_name);
		remove_proc_entry(smux_dev_name, NULL);
		return -ENOMEM;
	}

	entry = proc_create("dscp_mapping", 0644, procfs, &smuxdev_dscp_mapping_fops);
	if (entry == NULL)
	{
		printk("Register /proc/%s/dscp_mapping failed\n", smux_dev_name);
		remove_proc_entry(smux_dev_name, NULL);
		return -ENOMEM;
	}

	entry = proc_create("gem_port_skbmark", 0644, procfs, &smuxdev_mc_gem_port_skbmark_fops);
	if (entry == NULL)
	{
		printk("Register /proc/%s/gem_port_skbmark failed\n", smux_dev_name);
		remove_proc_entry(smux_dev_name, NULL);
		return -ENOMEM;
	}

#ifdef CONFIG_COMMON_RT_API
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
	entry = proc_create("hw_vlan_filter", 0644, procfs, &smuxdev_hw_vlan_filter_fops);
	if (entry == NULL)
	{
		printk("Register /proc/%s/hw_vlan_filter failed\n", smux_dev_name);
		remove_proc_entry(smux_dev_name, NULL);
		return -ENOMEM;
	}
#endif
#endif

    return 0;
} 

extern rx_handler_func_t *smux_handler;
static int __init misc_init(void)
{
    int err;	

	//SMUX_ASSERT(sizeof(struct smux_pkt_info_t) > sizeof(((struct sk_buff *)0)->cb));
	//LIST_INIT(&real_dev_list);
	
    err = misc_register(&smux_device);
    if (err) {
        pr_err("can't misc_register :(\n");	
        goto out;
    }
	
	err = register_netdevice_notifier(&smux_notifier_block);
	if (err < 0)
		goto unregister_misc;

	rcu_assign_pointer(smux_handler, smux_receive_skb);
		
	err = smuxdev_init_procfs();
	if (err < 0)
		goto unregister_misc;	

	smux_debug = SMUX_DEBUG_LEVEL_ERROR;
	smux_configure.multicast_mode = 1;
	smux_configure.dump_all = 0;
	memset(&smux_configure.mc_gem_port_skbmark_tbl[0], 0 , sizeof(smux_configure.mc_gem_port_skbmark_tbl));
	smux_configure.trace_filter = 0;
	smux_configure.hw_vlan_filter = 0;
	smux_configure.mapping_outermost_vlan_by_mac_from_lut = 0;
	smux_configure.auto_reserve_vlan = 0;

	int i;
	for(i=0;i<SMUX_MAX_DSCP_NUM;i++)
		dscp_mapping_table_default[i].vlan_tag.vlan_tci = htons(dscp_mapping_table_default[i].vlan_tag.vlan_tci);
		
    pr_info(MODULE_NAME ": loaded\n");
	
	return 0;
unregister_misc:
	misc_deregister(&smux_device);
out:
    return err;
}

static void __exit misc_exit(void)
{
	struct smux_realdev_cb_t  *realdev_cb, *tmp;
	
	unregister_netdevice_notifier(&smux_notifier_block);
	rcu_assign_pointer(smux_handler, NULL);
    misc_deregister(&smux_device);
	
	
	rtnl_lock();
	list_for_each_entry_safe(realdev_cb, tmp, &real_dev_list, list_real) {
		smux_realdev_cb_delete(realdev_cb);
	}
	rtnl_unlock();
	remove_proc_entry("debug", procfs);
	remove_proc_entry("filter", procfs);
	remove_proc_entry("interface", procfs);
	remove_proc_entry("list_intf", procfs);
	remove_proc_entry("rules", procfs);
	remove_proc_entry("configure", procfs);
	remove_proc_entry("dscp_mapping", procfs);
	remove_proc_entry("gem_port_skbmark", procfs);
#ifdef CONFIG_COMMON_RT_API
#ifdef CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE
	remove_proc_entry("hw_vlan_filter", procfs);
#endif
#endif
	remove_proc_entry(smux_dev_name, NULL);
	
    pr_info(MODULE_NAME ": unloaded\n");
}

module_init(misc_init)
module_exit(misc_exit)

MODULE_DESCRIPTION("SMUX Device");
MODULE_AUTHOR("sd5@realtk.com>");
MODULE_LICENSE("GPL");
