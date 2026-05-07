#ifndef __RTK_RG_G3_INTERNAL_H__
#define __RTK_RG_G3_INTERNAL_H__


#if defined(CONFIG_CA8279_SERIES) || defined(CONFIG_ARCH_RTL8198F)
#ifndef CONFIG_CA_G3_G3LITE_SERIES
#define CONFIG_CA_G3_G3LITE_SERIES 1
#endif
#endif


#if defined(CONFIG_HWNAT_RG)||defined(CONFIG_HWNAT_RG_MODULE)	//built-in or module
#include <rtk_rg_internal.h>
#include <rtk_rg_ca8277_asicDriver.h>	//8277B for RG is still under development
#elif defined(CONFIG_HWNAT_FLEETCONNTRACK)
#include <rtk_fc_internal.h>
#include <rtk_rg_apolloPro_asicDriver.h>
#endif

#include <aal_l3_stg0.h>
#include <aal_l3_te.h>
#include <aal_l2_te.h>
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
#include <aal_l2_te.h>
#include <port.h>			//for loopback definition
#endif
#include <aal_l2_tm.h>		//for CA_AAL_MAX_FLOW_ID
#include <aal_l3_tm.h> 		//for CA_AAL_L3_MAX_FLOW_ID
#include <classifier.h>		//for ca_classifier_rule_add
#include <route.h>		//for ca_generic_intf_add
#include <aal_hash.h>
#include <flow.h>
#include <rate.h>
#include <aal_mcast.h>
#include <ca_ext.h>


typedef enum{
	RG_CA_FLOW_UC5TUPLE_DS		= 0, 	//HASH_PROFILE_0, 	// WAN (wan to lan)
	RG_CA_FLOW_UC5TUPLE_US		= 1, 	//HASH_PROFILE_1, 	// LAN (lan to wan or lan to lan)
	RG_CA_FLOW_MC 				= 2, 	//HASH_PROFILE_2, 	// init in CA aal_hash_init()
	RG_CA_FLOW_UC2TUPLE_BRIDGE	= 3, 	//HASH_PROFILE_3, 	// without L4 bridge only
	RG_CA_FLOW_MC_FORWARD 		= 4,	//HASH_PROFILE_2, 
	RG_CA_FLOW_CLS_DOS_POL		= 5,	//HASH_PROFILE_4,	//[8277A] need Hash to check DOS Attack Prevent and do policing if necessary: synfin deny, syn flood deny, fin flood deny, icmp flood deny
	RG_CA_FLOW_CLS_TRAP_GRPPOL	= 5,	//HASH_PROFILE_4,	//[8277B] for packet trap with group policer by L3 CLS, need Hash fib to enable pol2_en
	RG_CA_FLOW_MAX = CA_FLOW_TYPE_MAX,
}rg_ca_flow_profile_keytype_t;

typedef enum{
	RG_CA_FLOW_TUPPLE_PRI_0,		// DEFAULT, the lowest priority
	RG_CA_FLOW_TUPPLE_PRI_1,
	RG_CA_FLOW_TUPPLE_PRI_2,
	RG_CA_FLOW_TUPPLE_PRI_3,
	RG_CA_FLOW_TUPPLE_PRI_4,
	RG_CA_FLOW_TUPPLE_PRI_5,
	RG_CA_FLOW_TUPPLE_PRI_6,
	RG_CA_FLOW_TUPPLE_PRI_MAX,
}rg_ca_flow_tuple_priority_t;


typedef struct rtk_rg_flow_key_mask_e{
	uint8 P12_spa;
	uint8 P12_vlanId;
	uint8 P12_vlanPri;
	uint8 P345_vlanId;
	uint8 P345_vlanPri;
	uint8 P345_dscp;
	uint8 P345_ecn;
	uint8 pall_vlan_deicfi;
	uint8 p12_stream_id;
	uint8 p345_stream_id;
} rtk_rg_flow_key_mask_t;

// flowid value should mapping to the sequence of rtk_rg_port_idx_t(extport index)
typedef enum{
	RTK_RG_WIFI_FLOWID_CPU_RSVD=0,
#if defined(CONFIG_WLAN_MBSSID_NUM) && (CONFIG_WLAN_MBSSID_NUM==7)
	RTK_RG_WIFI0_FLOWID_VAP4,
	RTK_RG_WIFI0_FLOWID_VAP5,
	RTK_RG_WIFI0_FLOWID_VAP6,
#endif	
	RTK_RG_WIFI0_FLOWID_OTHER,				// for 11ac other

	RTK_RG_WIFI1_FLOWID_ROOT,				// for 11n root
	RTK_RG_WIFI1_FLOWID_VAP0,
	RTK_RG_WIFI1_FLOWID_VAP1,
	RTK_RG_WIFI1_FLOWID_VAP2,
	RTK_RG_WIFI1_FLOWID_VAP3,
#if defined(CONFIG_WLAN_MBSSID_NUM) && (CONFIG_WLAN_MBSSID_NUM==7)
	RTK_RG_WIFI1_FLOWID_VAP4,
	RTK_RG_WIFI1_FLOWID_VAP5,
	RTK_RG_WIFI1_FLOWID_VAP6,
#endif
	RTK_RG_WIFI1_FLOWID_OTHER,				// for 11n other
	
	RTK_RG_WIFI_FLOWID_MAX,	
}rtk_rg_wifi_flowid_t;


#define rg_ca_skb_in(skb, trace) 	nic_ca_skb_in(skb, __FILE__, __LINE__, trace)
#define rg_ca_skb_out(skb) 		nic_ca_skb_out(skb, __FILE__, __LINE__)
#ifndef RT_RATE_HW_IDX_L3_METER_RATE_MAX
#define POL_RATE_MAX 32767999	// policer1 maximum rate (0xFFFF * 500 + 499)
#else
#define POL_RATE_MAX	RT_RATE_HW_IDX_L3_METER_RATE_MAX
#endif
#ifndef RT_RATE_HW_IDX_L3_METER_BURST_MAX
#define POL_BURST_MAX 0XFFF
#else
#define POL_BURST_MAX	RT_RATE_HW_IDX_L3_METER_BURST_MAX
#endif


typedef struct {
	ca_uint32_t					mac_sa_trans	:1;
	ca_uint32_t					mac_da_overwrite	:1;
	ca_uint32_t					vlan_action		:1;
} rtl_mcast_egress_action_mask_t;


typedef enum {
    RTL_L2_VLAN_TAG_ACTION_NOP  = 0,
    RTL_L2_VLAN_TAG_ACTION_PUSH = 1,
    RTL_L2_VLAN_TAG_ACTION_POP  = 2,
    RTL_L2_VLAN_TAG_ACTION_SWAP = 3,
    RTL_L2_VLAN_TAG_ACTION_POP_ALL = 4,
    RTL_L2_VLAN_TAG_ACTION_SWAP_TOP_POP_INNER =5,    
} rtl_vlan_tag_action_t;


typedef struct rtl_l2_mcast_member_s {
	rtl_mcast_egress_action_mask_t			action_mask;
	ca_mac_addr_t 					new_mac_da;
	rtl_vlan_tag_action_t				vlan_action;
	ca_uint16_t					vid;
	ca_uint16_t					vpri:3;
	ca_uint16_t					cos:3;
	ca_uint16_t 					wifi_flowid:10;
	ca_uint32_t 					reserved[CA_MC_EGRESS_ACTION_RESERVED];
	ca_port_id_t					member_port;
	ca_mac_addr_t 					new_mac_sa;
} rtl_l2_mcast_member_t;

typedef struct rtl_l2_mcast_group_members_s {
	ca_uint32_t					mcg_id;
	ca_uint32_t					member_count;
	ca_uint32_t					mcHwIdx;
	rtl_l2_mcast_member_t		member[CA_MAX_L2_MC_MEMBER];
} rtl_l2_mcast_group_members_t;



typedef struct l2_mcg_node_s {
	struct list_head	list;

	ca_uint16_t		sw_mcgid;	//software multicast group idx
	ca_uint16_t		mce_indx; 	//hardware real mcgid
	
	struct list_head	port_list;
	
	ca_uint16_t			llid;
	ca_uint16_t			vlan;
	ca_ip_address_t			grp_addr;
	ca_mac_addr_t			mac_addr;

	ca_boolean_t			is_knownMcGroup;
	ca_ip_address_t			src_addr;
	ca_uint32_t				hash_idx;
} rtl_l2_mcg_t;

typedef struct rtl_port_node_s {
	struct list_head		list;
	portForwardAction_t		key;
} rtl_port_t;


//EXTERN FUNCTION
int aal_l3fe_keep_lspid_unchange_set(ca_device_id_t device_id, ca_uint8_t *config);
netdev_tx_t nic_egress_start_xmit(struct sk_buff *skb, struct net_device *dev, ca_ni_tx_config_t *tx_config);
netdev_tx_t ca_ni_start_xmit_native(struct sk_buff *skb, struct net_device *dev, ca_ni_tx_config_t *tx_config);
void nic_ca_skb_in(struct sk_buff *skb, const char *fname, int line, bool trace);
void nic_ca_skb_out(struct sk_buff *skb, const char *fname, int line);
int ca_flow_age_set(ca_device_id_t device_id, ca_uint32_t hash_idx, ca_uint32_t age);
int ca_flow_age_get(ca_device_id_t device_id, ca_uint32_t hash_idx, ca_uint32_t *age);
int ca_flow_traffic_status_get(ca_device_id_t device_id, ca_uint32_t hash_idx, ca_uint32_t *trfStatus);

int nic_rxhook_default(struct napi_struct *napi,struct net_device *dev, struct sk_buff *skb, nic_hook_private_t *nh_priv);
#ifdef CONFIG_RTK_NIC_TX_HOOK
int nic_register_txhook(p2tfunc_t tx);
int nic_txhook_init(void);
int nic_txhook_exit(void);
#endif

#ifdef CONFIG_LUNA_G3_SERIES
void flow_table_dump(void);
#endif

//Prototype
//#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
uint32_t rtk_rg_g3_l2fe_hostpolicing_init(void);
//#endif
#if defined(CONFIG_RG_G3_L3FE_MC_DEEPQ)
uint32_t rtk_rg_g3_l3fe_mcbuffering_init(void);
#endif
uint32_t rtk_rg_g3_cpuport_init(void) ;
#if defined(CONFIG_CA_G3_G3LITE_SERIES)
int32 rtk_rg_g3_l3fe_unknownDAStormCtrl(unsigned int port, bool enable, int l3fe_pol_idx);
int32 rtk_rg_g3_l3fe_unknownDA_wan_default_enable(unsigned int wan_port_idx);
int32 rtk_rg_g3_l3fe_knownDA_add(unsigned char *dmac, unsigned int *entryidx);
int32 rtk_rg_g3_l3fe_knownDA_del(unsigned int entryidx);
int32 rtk_rg_g3_l3fe_knownDA_flush(void);
#elif defined(CONFIG_CA8277B_SERIES)
int32 rtk_8277b_asic_hl_egrsmac_trans_add(unsigned char egrNetifIdx, unsigned char *egrSMAC, unsigned int *entryidx);
int32 rtk_8277b_asic_hl_egrsmac_trans_del(unsigned int entryidx);
int32 rtk_8277b_asic_ni_p7_rxsel_l3fe(unsigned char enable);
#endif
uint32_t rtk_rg_g3_flow_init(rtk_rg_flow_key_mask_t flowKeyMask);
uint32_t rtk_rg_g3_init(void);
uint32_t rtk_rg_g3_exit(void);

//ACL Prototype
int _rtk_rg_aclAndCfReservedRuleAdd(rtk_rg_aclAndCf_reserved_type_t rsvType, void *parameter);
int _rtk_rg_aclAndCfReservedRuleAddSpecial(rtk_rg_aclAndCf_reserved_type_t rsvType, void *parameter);
int _rtk_rg_aclAndCfReservedRuleDelSpecial(rtk_rg_aclAndCf_reserved_type_t rsvType, void *parameter);



ca_status_t rtl_l2_mcast_group_asknown (
	ca_device_id_t device_id,
	ca_ip_address_t groupAddr,
	ca_int32_t *hwIdx
);
ca_status_t rtl_l2_mcast_group_asunknown (
	ca_device_id_t device_id,
	ca_ip_address_t groupAddr,
	ca_int32_t *hwIdx
);
ca_status_t rtl_l2_mcast_group_add (
	ca_device_id_t device_id,
	ca_l2_mcast_entry_t *entry
);
ca_status_t rtl_l2_mcast_group_delete (
	ca_device_id_t		 device_id,
	ca_uint32_t		  mcg_id
);
ca_status_t rtl_l2_mcast_group_dump (
	ca_device_id_t		 device_id,
	ca_uint32_t		  mcg_id
);
ca_status_t rtl_l2_port_member_add(
	ca_device_id_t         device_id,
	rtl_l2_mcast_group_members_t *members
);
ca_status_t rtl_l2_port_member_delete(
	ca_device_id_t         device_id,
	rtl_l2_mcast_group_members_t *members
);


#endif //__RTK_RG_G3_INTERNAL_H__

