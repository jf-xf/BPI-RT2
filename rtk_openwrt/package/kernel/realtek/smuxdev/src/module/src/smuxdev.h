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

#ifndef _SMUXDEV_H_
#define _SMUXDEV_H_

#ifdef RTK_NETDEV_PRIV_FLAGS
#define SMUXDEV_INHERIT_PRIV_FLAG	(RTK_IFF_DOMAIN_ELAN|RTK_IFF_DOMAIN_WLAN|RTK_IFF_DOMAIN_WAN)
#else
#define SMUXDEV_INHERIT_PRIV_FLAG	(IFF_DOMAIN_ELAN|IFF_DOMAIN_WLAN|IFF_DOMAIN_WAN)
#endif

enum SMUX_DEBUG_INDEX {
	SMUX_DEBUG_INDEX_RX = 0,
	SMUX_DEBUG_INDEX_RX_PACKET,
	SMUX_DEBUG_INDEX_TX,
	SMUX_DEBUG_INDEX_TX_PACKET,
	SMUX_DEBUG_INDEX_RXMC,
	SMUX_DEBUG_INDEX_RXMC_PACKET,
	SMUX_DEBUG_INDEX_IOCTRL,
	SMUX_DEBUG_INDEX_MISC,
	SMUX_DEBUG_INDEX_MULTICAST,
	SMUX_DEBUG_INDEX_ERROR,
	SMUX_DEBUG_INDEX_MAX
};

enum SMUX_DEBUG_LEVEL{
	SMUX_DEBUG_LEVEL_PRINT_NOTHING = 0,
	SMUX_DEBUG_LEVEL_RX = (1<<SMUX_DEBUG_INDEX_RX),
	SMUX_DEBUG_LEVEL_RX_PACKET = (1<<SMUX_DEBUG_INDEX_RX_PACKET),
	SMUX_DEBUG_LEVEL_TX = (1<<SMUX_DEBUG_INDEX_TX),
	SMUX_DEBUG_LEVEL_TX_PACKET = (1<<SMUX_DEBUG_INDEX_TX_PACKET),
	SMUX_DEBUG_LEVEL_RXMC = (1<<SMUX_DEBUG_INDEX_RXMC),
	SMUX_DEBUG_LEVEL_RXMC_PACKET = (1<<SMUX_DEBUG_INDEX_RXMC_PACKET),
	SMUX_DEBUG_LEVEL_IOCTRL = (1<<SMUX_DEBUG_INDEX_IOCTRL),
	SMUX_DEBUG_LEVEL_MISC = (1<<SMUX_DEBUG_INDEX_MISC),
	SMUX_DEBUG_LEVEL_MULTICAST = (1<<SMUX_DEBUG_INDEX_MULTICAST),
	SMUX_DEBUG_LEVEL_ERROR = (1<<SMUX_DEBUG_INDEX_ERROR)
};

struct smux_debug_entry {
	int debug_level;
	char *debug_level_str;
	int force_print;
};

//#define IFF_RTK_SMUX_VIRT  (1<<31) /* MUST redefined to value not conflict with LINUX  */

//#define SMUX_DBG(fmt, ...)  printk("DBG:" fmt, ##__VA_ARGS__)
struct smux_realdev_cb_t;
#define SMUX_ASSERT(expr) if (unlikely((expr))) { printk("%s(%d): assertion failed\n",__func__,__LINE__); BUG(); }
struct smux_dev_priv {
	struct net_device *real_dev;
	struct net_device *virt_dev;
	struct smux_realdev_cb_t *real_dev_priv;
	struct list_head list_virt; 
	int carrier_mode;
	struct rtnl_link_stats64 net_stats;
};

struct smux_ruleset_t {
	u32    id;
	struct smux_rule_t rule;	
	struct net_device *rx_dev;
	//struct net_device *txif;
	struct list_head list;
	struct rcu_head rcu;
};

struct smux_table_t {
	struct list_head rule_head;
	u32		rule_count;
	u32     curr_id; /* auto inc */
	__be16  default_tpid;
	__be16  default_vlan_tci;
};

struct smux_qos_t {
	struct smux_dscp_mapping_entry_t *dscp_mapping_table;
	u8 dscp_mapping_enable;
	u8 vlan_idx;
};

struct smux_multicast_t {
	u32 rx_multicast_rule_count;
	u8 rx_multicast_rule_exist;
};

#ifdef CONFIG_COMMON_RT_API
struct smux_hw_vlan_filter_entry_t {
	uint32_t valid;
	int acl_index;
	uint32_t vlan_id;
	uint32_t ref_count;
};

struct smux_hw_vlan_filter_t {
	u8 mode; //bit0: UC, bit1: BC, bit2: MC, bit0~2: ALL
	u32 port_mask;
	int default_acl_index[SMUX_HW_VLAN_FILTER_DEFAULT_NUM];
	struct smux_hw_vlan_filter_entry_t entry[SMUX_HW_VLAN_FILTER_NUM];
};
#endif

struct smux_realdev_cb_t  {
	struct list_head list_real;
	struct list_head list_virt;
	struct net_device *realdev;
	struct smux_table_t tables[SMUX_DIR_MAX][SMUX_MAX_TAGS+1];
	__be16 tpids[SMUX_MAX_TAGS];
	struct smux_qos_t qos;
	struct smux_multicast_t multicast;
#ifdef CONFIG_COMMON_RT_API
	struct smux_hw_vlan_filter_t hw_vlan_filter;
#endif
	u32 default_policy[SMUX_DIR_MAX];
	int rx_multi_mode;
};

struct smux_vlan_tag_t {
	u16 vlan_proto;
	u16 vlan_tci;
};

struct smux_ip_tos_t {
        union {
                u8 bit1;
                u8 bit2;
                u8 tos;
        } ip_tos;
};
#define IP_TOS_GET_DSCP(t) ((t->ip_tos.tos & IP_DSCP_MASK) >> IP_DSCP_SHIFT)
#define IP_TOS_SET_DSCP(t, dscp) (t->ip_tos.tos=((t->ip_tos.tos & ~IP_DSCP_MASK) | dscp))

struct smux_ipv6_tos_t {
	u32 tos;
};
#define IP6_TOS_GET_DSCP(t) (ntohl(t->tos & IPV6_DSCP_MASK) >> IPV6_DSCP_SHIFT)
#define IP6_TOS_SET_DSCP(t, dscp) (t->tos=((t->tos & ~IPV6_DSCP_MASK) | dscp))

struct smux_ppp_header_t {
	__be16 ppp_proto;
	char payload[0];
};

struct smux_dscp_mapping_entry_t  {
	u8 enable;
	char *meaning;
	u8 ip_tos;
	struct smux_vlan_tag_t vlan_tag;
};

//IF,DA,SA,ETH,TPID,VID,SIP,DIP,L4PROTO,SPORT,DPORT,SIP6,DIP6
enum SMUX_TRACE_FILETR_INDEX{
	SMUX_TRACE_FILETR_IF = 0,
	SMUX_TRACE_FILETR_SA,
	SMUX_TRACE_FILETR_DA,
	SMUX_TRACE_FILETR_ETH,
	SMUX_TRACE_FILETR_TPID1,
	SMUX_TRACE_FILETR_VID1,
	SMUX_TRACE_FILETR_TPID2,
	SMUX_TRACE_FILETR_VID2,
	SMUX_TRACE_FILETR_TPID3,
	SMUX_TRACE_FILETR_VID3,
	SMUX_TRACE_FILETR_TPID4,
	SMUX_TRACE_FILETR_VID4,
	SMUX_TRACE_FILETR_SIP,
	SMUX_TRACE_FILETR_DIP,
	SMUX_TRACE_FILETR_SIP6,
	SMUX_TRACE_FILETR_DIP6,
	SMUX_TRACE_FILETR_L4PROTO,
	SMUX_TRACE_FILETR_SPORT,
	SMUX_TRACE_FILETR_DPORT,
	SMUX_TRACE_FILETR_TIMES,
	SMUX_TRACE_FILETR_MAX
};

#define SMUX_TRACE_FILETR_RULES_MAX 4
#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif
typedef struct {
	u32 bits;
	char ifname[32];
	u8 sa[ETH_ALEN];
	u8 sa_mask[ETH_ALEN];
	u8 da[ETH_ALEN];
	u8 da_mask[ETH_ALEN];
	struct smux_vlan_tag_t vlan[4];
	u16 eth_proto;
	u32 sip;
	u32 dip;
	u8 proto;
	u16 sport;
	u16 dport;
	int times;
	struct in6_addr sip6;
	struct in6_addr dip6;
}traceFilterRule;

struct smux_configure_t {
	u32  multicast_mode;
	u32  dump_all;
	struct smux_mc_gem_port_skbmark_t  mc_gem_port_skbmark_tbl[SMUX_MC_GEM_PORT_SKBMARK_NUM];
	u32  trace_filter;
	u32  hw_vlan_filter;
	u32  mapping_outermost_vlan_by_mac_from_lut;
	u32  auto_reserve_vlan;
	traceFilterRule trace_filter_rules[SMUX_TRACE_FILETR_RULES_MAX];
};

static inline struct smux_dev_priv *smux_dev_priv(const struct net_device *dev)
{
	return netdev_priv(dev);
}

#endif /* _SMUXDEV_H_ */
