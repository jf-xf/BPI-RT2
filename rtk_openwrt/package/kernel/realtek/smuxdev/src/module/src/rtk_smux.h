#ifndef _SMUXCTL_H_
#define _SMUXCTL_H_
#ifndef IFNAMSIZ
#define IFNAMSIZ	16
#endif

#include <linux/ioctl.h>

#define SMUXDEV_NODE_MAGIC 0xF1
#define SMUXDEV_NODE_NAME "smuxdev"
#ifndef SMUX_MAX_TAGS
#define SMUX_MAX_TAGS 4
#endif
/* commands - up to 31 */

#define SMUXCTL_CMD_NONE 					_IO(SMUXDEV_NODE_MAGIC, 0)
#define SMUXCTL_CMD_IF_CREATE				_IOW(SMUXDEV_NODE_MAGIC, 1, struct smux_interface_s)
#define SMUXCTL_CMD_IF_DELETE				_IOW(SMUXDEV_NODE_MAGIC, 2, struct smux_interface_s)
#define SMUXCTL_CMD_RULE_ADD				_IOW(SMUXDEV_NODE_MAGIC, 3, struct smux_ioctl_rule_t)
#define SMUXCTL_CMD_RULE_DELETE			    _IOW(SMUXDEV_NODE_MAGIC, 4, struct smux_ioctl_rule_t)
#define SMUXCTL_CMD_RULE_RESET			    _IOW(SMUXDEV_NODE_MAGIC, 5, struct smux_ioctl_rule_t)
#define SMUXCTL_CMD_SHOW_TABLE			    _IOW(SMUXDEV_NODE_MAGIC, 6, struct smux_ioctl_rule_t)
#define SMUXCTL_CMD_SET_DEFAULT		        _IOW(SMUXDEV_NODE_MAGIC, 7, struct smux_ioctl_rule_t)
#define SMUXCTL_CMD_SET_QOS		        	_IOW(SMUXDEV_NODE_MAGIC, 8, struct smux_ioctl_rule_t)
#define SMUXCTL_CMD_SET_IF_PARM				_IOW(SMUXDEV_NODE_MAGIC, 9, struct smux_interface_s)
#define SMUXCTL_CMD_TEST					_IOW(SMUXDEV_NODE_MAGIC, 10, struct smux_ioctl_test_s)


#if defined(__LITTLE_ENDIAN_BITFIELD)
#define VID_MASK           0xff0f
#define PRIO_MASK          0x00e0
#define CFI_MASK           0x0010
#define IP_DSCP_MASK       0xfc
#define IPV6_DSCP_MASK     0x0000c00f
#elif defined(__BIG_ENDIAN_BITFIELD)
#define VID_MASK           0x0fff
#define PRIO_MASK          0xe000
#define CFI_MASK           0x1000
#define IP_DSCP_MASK       0xfc
#define IPV6_DSCP_MASK     0x0fc00000
#endif

#define CFI_SHIFT 12
#define PRIO_SHIFT 13
#define IP_DSCP_SHIFT 2
#define IPV6_DSCP_SHIFT 22

enum smux_direction_e {
	SMUX_DIR_RX,
	SMUX_DIR_TX,
	SMUX_DIR_RXMC,
	SMUX_DIR_MAX
};

struct smux_interface_s {
	char real_name[IFNAMSIZ];
	char virt_name[IFNAMSIZ];
	unsigned int dev_flags;
	unsigned int policy[SMUX_DIR_MAX];
	int carrier;
	int rx_multi;
};

#define SMUXCTL_IF_FLAGS_MCAST    		(0x0001U)
#define SMUXCTL_IF_FLAGS_ROUTED   		(0x0002U)
#define SMUXCTL_IF_FLAGS_MODE_ONT 		(0x0004U)
#define SMUXCTL_IF_FLAGS_MODE_RG  		(0x0008U)
#define SMUXCTL_IF_FLAGS_RSMUX	  		(0x0010U)
#define SMUXCTL_IF_FLAGS_EDIT	  		(0x8000U)

/* always store in network byte order to speed up compare */
struct smux_filter_vlan_tag_t {
	uint16_t vlan_tci;  /* pbit:3 cfi:1 vid:12 */
	uint16_t vlan_tci_mask;
	uint16_t vlan_proto;
	uint16_t vlan_proto_mask;
};

enum smux_rule_filter_e  {
	SMUX_FLT_ETHTYPE,
	SMUX_FLT_SIP,
	SMUX_FLT_DIP,
	SMUX_FLT_SIP_RANGE,
	SMUX_FLT_DIP_RANGE,
	SMUX_FLT_IPL4PROTO,
	//SMUX_FLT_IPDSCP,
	SMUX_FLT_VTAGS, /* vlan tags , include TPID, VID, CFI, PRI*/
	SMUX_FLT_ORG_VTAGS,
	SMUX_FLT_RXIF_NAME,
	SMUX_FLT_TXIF_NAME,
	SMUX_FLT_SKB_MARK,
#ifdef CONFIG_RTK_SKB_MARK2
	SMUX_FLT_SKB_MARK2,
#endif
	SMUX_FLT_UNI_MAC,
	SMUX_FLT_DMAC,
	SMUX_FLT_SMAC,
	/*for N:1 vlan translate, downstream eggress need to set corressponding vlan by mac for PPTP. */
	SMUX_FLT_UNI_DMAC_OUTERMOST_VLAN,
	SMUX_FLT_MULTICAST,
	SMUX_FLT_DSCP_RANGE,
	SMUX_FLT_MAX,
};

struct smux_skb_mark_t {
    uint32_t    mask;
    uint32_t    value;
};

#ifdef CONFIG_RTK_SKB_MARK2
struct smux_skb_mark2_t {
    uint64_t    mask;
    uint64_t    value;
};
#endif

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif
#ifndef DSCP_LEN
#define DSCP_LEN 8
#endif
struct smux_rule_filter_t {
	uint32_t en_filter;
	
	/*for N:1 vlan translate, downstream eggress need to set corressponding vlan by dmac for PPTP. */
	uint8_t check_outermost_vlan_map_dmac_by_lut;
	struct smux_filter_vlan_tag_t vtags[SMUX_MAX_TAGS+1];
	struct smux_filter_vlan_tag_t org_vtags[SMUX_MAX_TAGS+1];
	
	char rxif_name[IFNAMSIZ+1];
	char txif_name[IFNAMSIZ+1];
	char txif_match;
	
	uint16_t ethtype;
	uint8_t  ipdscp[DSCP_LEN];
	uint8_t  check_mac;
	uint8_t  dmac[ETH_ALEN];
	uint8_t  dmac_mask[ETH_ALEN];
	uint8_t  smac[ETH_ALEN];
	uint8_t  smac_mask[ETH_ALEN];
	struct smux_skb_mark_t mark;
#ifdef CONFIG_RTK_SKB_MARK2
	struct smux_skb_mark2_t mark2;
#endif

	uint32_t sip;
	uint32_t dip;
	uint32_t sip_start;
	uint32_t sip_end;
	uint32_t dip_start;
	uint32_t dip_end;
	uint8_t  ipl4proto;	
};

int smux_rule_filter_set_ethtype(struct smux_rule_filter_t *f, uint16_t ethtype);

struct smux_rule_action_t {
	uint8_t		action;
	uint32_t 	args[2];
#ifdef CONFIG_RTK_SKB_MARK2
	uint64_t	args_64[2];
#endif
	char rxif_name[IFNAMSIZ];
};

#define SMUX_ACTION_MAX 16
#define SMUX_RULE_ALIAS_NAME 64
struct smux_rule_t {
	struct smux_rule_filter_t filters;
	struct smux_rule_action_t actions[SMUX_ACTION_MAX];
	char rxif_name[IFNAMSIZ];
	uint8_t action_count;
	uint32_t hit_count;
	char alias_name[SMUX_RULE_ALIAS_NAME+1];
	uint32_t priority;
	uint8_t dup_forward;
};

enum smux_rule_action_e {
	SMUX_ACT_NONE, /* no acton? */
	SMUX_ACT_SET_ETHTYPE,
	//SMUX_ACT_SET_PBITS,
	SMUX_ACT_POP_TAG,
	SMUX_ACT_PUSH_TAG,
	SMUX_ACT_DUP_FORWARD,
	SMUX_ACT_SET_RXIF,
	SMUX_ACT_SET_TPID,
	SMUX_ACT_SET_VID,
	SMUX_ACT_SET_PRIORITY,
	SMUX_ACT_SET_CFI,
	SMUX_ACT_SET_SKB_MARK,
#ifdef CONFIG_RTK_SKB_MARK2
	SMUX_ACT_SET_SKB_MARK2,
#endif
	SMUX_ACT_SET_DSCP,
	SMUX_ACT_COPY_8021Q,
	SMUX_ACT_COPY_TPID,
	SMUX_ACT_COPY_VID,
	SMUX_ACT_COPY_PRIORITY,
	SMUX_ACT_COPY_CFI,
	SMUX_ACT_MAP_DSCP_TO_PRIORITY,
	SMUX_ACT_SET_TARGET,
	SMUX_ACT_REVERT_VLAN,
	SMUX_ACT_MAX
};

enum SMUX_CARRIER{
	SMUX_CARRIER_AUTO = 0,
	SMUX_CARRIER_MANUAL,
	SMUX_CARRIER_NONE
};

enum SMUX_RX_MULTI{
	SMUX_RX_MULTI_NORMAL = 0,
	SMUX_RX_MULTI_MATCH,
	SMUX_RX_MULTI_NONE
};

enum SMUX_TARGET {
	SMUX_TARGET_CONTINUE = 0,
	SMUX_TARGET_ACCEPT,
	SMUX_TARGET_DROP,
	SMUX_TARGET_END
};

struct smux_table_id_t {
	char ifname[IFNAMSIZ];
	enum smux_direction_e dir;
	int nr_tag;
	uint16_t default_tpid;
	uint16_t default_vlan_tci;
};

struct smux_qos_rule_t {
	uint32_t dscp_mapping_enable;
	uint8_t vlan_idx;
	uint8_t dscp;
	uint8_t priority;
};

#define SMUX_MC_GEM_PORT_SKBMARK_NUM	10
struct smux_mc_gem_port_skbmark_t {
	uint32_t mark1, mark_mask1;
#ifdef CONFIG_RTK_SKB_MARK2
	uint64_t mark2, mark_mask2;
#endif
	uint8_t mark_select; //1: mark1, 2:mark2
};

#ifdef CONFIG_COMMON_RT_API
#define SMUX_HW_VLAN_FILTER_ACL_WEIGHT_HIGH	500
#define SMUX_HW_VLAN_FILTER_ACL_WEIGHT_MED	400
#define SMUX_HW_VLAN_FILTER_ACL_WEIGHT_LOW	300
#define SMUX_HW_VLAN_FILTER_DEFAULT_NUM	3
#define SMUX_HW_VLAN_FILTER_NUM	30
struct smux_hw_vlan_filter_para_t {
	struct net_device *dev;
	uint32_t vlan_id;
	uint32_t mode; //bit0: UC, bit1: BC, bit2: MC, bit0~2: ALL
};

#define UNKOWN_VID_BY_LUT 0x0
#define GET_VID_BY_LUT 0x01

#endif

enum smux_rule_method_e {
	SMUX_RULE_METHOD_APPEND	        = 1,
	SMUX_RULE_METHOD_INSERT	        ,
	SMUX_RULE_METHOD_INSERT_BEFORE	,
	SMUX_RULE_METHOD_INSERT_AFTER	,
	SMUX_RULE_METHOD_DELETE			,
	SMUX_RULE_METHOD_DELETE_ALL		,
	SMUX_RULE_METHOD_DELETE_IFNAME	,
	SMUX_RULE_METHOD_DELETE_ALIAS	,
	SMUX_RULE_METHOD_RESET_COUNT	,
};

#define SMUX_DEFAULT_METHOD_TPID		6
#define SMUX_DEFAULT_METHOD_VID		    7

enum smux_qos_method_e {
	SMUX_QOS_METHOD_DSCP_MAPPING_ENABLE	= 1,
	SMUX_QOS_METHOD_DSCP_MAPPING_SET	,
	SMUX_QOS_METHOD_DSCP_MAPPING_DEFAULT,
	SMUX_QOS_METHOD_DSCP_MAPPING_CHANGE_DEFAULT	,
};

enum smux_mc_gem_port_skbmark_method_e {
	SMUX_MC_GEM_PORT_SKBMARK_METHOD_ADD	= 1,
	SMUX_MC_GEM_PORT_SKBMARK_METHOD_DEL	,
};

#ifdef CONFIG_COMMON_RT_API
enum smux_hw_vlan_filter_method_e {
	SMUX_MC_HW_VLAN_FILTER_METHOD_SWITCH_ON = 1,
	SMUX_MC_HW_VLAN_FILTER_METHOD_SWITCH_OFF	,
	SMUX_MC_HW_VLAN_FILTER_METHOD_ENABLE	,
	SMUX_MC_HW_VLAN_FILTER_METHOD_DISABLE	,
	SMUX_MC_HW_VLAN_FILTER_METHOD_ADD	,
	SMUX_MC_HW_VLAN_FILTER_METHOD_DEL	,
	SMUX_MC_HW_VLAN_FILTER_METHOD_ADD_BY_RULE	,
	SMUX_MC_HW_VLAN_FILTER_METHOD_DEL_BY_RULE	,
	SMUX_MC_HW_VLAN_FILTER_METHOD_REFRESH	,
};
#endif
/*  */

struct smux_ioctl_rule_t {
	uint8_t  method; /* append/insert-before/insert-after/remove/ */
	uint32_t id; /* in/out: on success, return new id  */
	char arg[SMUX_RULE_ALIAS_NAME+1];
	struct smux_table_id_t  table;
	struct smux_rule_t   rule;
	struct smux_qos_rule_t	qos;
};

/* testing interface:
  create an skb with dev <= ifname, len=payload_len to the smuxdev interface
  on success, content is updated according to filter and actions executed.
*/
struct smux_ioctl_test_skbinfo_s {
	void *in_skb;
	void *out_skb;
	uint32_t mark;
	uint32_t priority;
	char devname[IFNAMSIZ];
};

struct smux_ioctl_test_s {
	int result;
	struct smux_ioctl_test_skbinfo_s skbinfo; /* in/out */
	uint16_t flags; /* 0x01 => use cloned skb, */
	enum smux_direction_e dir;
	uint16_t payload_len;
	uint8_t payload[1600];	
};


#endif /* _SMUXCTL_H_ */
