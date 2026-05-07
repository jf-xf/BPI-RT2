#include <linux/skbuff.h>
#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_FC_RTL8198F_SERIES) || defined(CONFIG_RTL8198X_SERIES)
#include "rtk_fc_helper.h"
#include <net/netfilter/nf_conntrack.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
#include <net/netfilter/nf_conntrack_l3proto.h>
#endif
#include <net/netfilter/nf_conntrack_l4proto.h>
#include "rtk_fc_assistant.h"
#include <uapi/linux/if_pppox.h>
#endif


#if defined(CONFIG_RTK_L34_G3_PLATFORM)
#include "ca_ni.h"
#endif

#if defined(CONFIG_RTK_VLAN_PASSTHROUGH_SUPPORT)
#include <soc/cortina/rtk_vlan_passthrough.h>
#endif

#if defined(CONFIG_RTK_LINUX_VLAN_SUPPORT)
#include "../../rtk_linux_vlan.h"
#endif

#if defined(CONFIG_RTL_83XX_QOS_SUPPORT)
static unsigned short flow_direction_downstream = 1;		// refer to rtk_fc_struct.h
extern unsigned int g83xxQosEnabled;
#include <soc/cortina/rtk_vlan_common.h>
#endif

#include "rtk_fc_mgr.h"
#include <rtk/qos.h>
#if defined(CONFIG_RTK_SOC_RTL8198D)
#include <rtk_fc_api.h>
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAIL
#define FAIL -1
#endif


#if defined(CONFIG_RTL_8226_SUPPORT)
#include <linux/if.h>
#include <linux/netdevice.h>

extern char rtk_8226_lan_dev_name[IFNAMSIZ];
#endif
#include <br_private.h>
#include <rtk_fc_helper_wlan.h>

#ifdef CONFIG_RTK_FC_TCP_SPI_SUPPORT
extern unsigned int rtk_fc_get_conntrack_index(const struct tcphdr *tcph);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
extern bool rtk_tcp_in_window(const struct nf_conn *ct,
			  struct ip_ct_tcp *state,
			  enum ip_conntrack_dir dir,
			  unsigned int index,
			  const struct sk_buff *skb,
			  unsigned int dataoff,
			  const struct tcphdr *tcph);
#else
extern bool rtk_tcp_in_window(const struct nf_conn *ct,
			  struct ip_ct_tcp *state,
			  enum ip_conntrack_dir dir,
			  unsigned int index,
			  const struct sk_buff *skb,
			  unsigned int dataoff,
			  const struct tcphdr *tcph,
			  u_int8_t pf);
extern struct nf_conntrack_l4proto* rtk_fc_get_nf_ct_protos(unsigned int pf, unsigned int l4_proto);
extern struct nf_conntrack_l3proto* rtk_fc_nf_ct_l3proto_find(u_int16_t l3proto);
#endif
#endif

#if defined(CONFIG_RTK_SOC_RTL8198D)
#if defined(CONFIG_WFO_OFFLOAD_2G)
#define RTK_WFO_IFACE_NUM	(6)
#define RTK_WFO_STA_NUM		(64)

const char *rtk_fc_wfo_wlanmap[RTK_WFO_IFACE_NUM] = {"wlan1", "wlan1-vap0", "wlan1-vap1", "wlan1-vap2", "wlan1-vap3", "wlan1-vxd"}; 
#endif

const char *str_flow_mib_mode[RTK_EXT_FLOW_MIB_MODE_MAX] = {"ipv4", "mac"};

#define NON_ZERO_MAC(mac)		(mac[0] || mac[1] || mac[2] || mac[3] || mac[4] || mac[5])
#define ZERO_MAC(mac)			(!NON_ZERO_MAC(mac))


#define RTK_FC_EXT_FLOW_MIB_DEBUG(string) \
do { \
	if (unlikely(fc_mgr_db.extFlowMibControl.debug)) \
		fc_mgr_db.extFlowMibControl.string++; \
} while (0)

#define RTK_FC_EXT_FLOW_MIB_WFO_DEBUG(string) \
do { \
	if (unlikely(fc_mgr_db.extFlowMibControl.debug_wfo)) \
		fc_mgr_db.extFlowMibControl.string++; \
} while (0)

struct wfo_sta_st_s
{
	u64 rx_pkts;
	u64 rx_bytes;
	u64 tx_pkts;
	u64 tx_bytes;

	u8 mac_addr[ETH_ALEN];
}  __attribute__((packed));

void (*rtk_fc_wfo_deliver_sta_status)(struct wfo_sta_st_s *sta_st, u32 sta_st_sz);
EXPORT_SYMBOL(rtk_fc_wfo_deliver_sta_status);
#endif

extern int rtk_get_op_mode(void);

int rtk_get_83xx_qos_status(void)
{
#if defined(CONFIG_RTL_83XX_QOS_SUPPORT)
	if(g83xxQosEnabled)
		return 1;
#endif
		return 0;
}

void rtk_83xx_qos_set_vlan_tag(unsigned char *egrCVlanTagif, unsigned short *egrCVID, unsigned short direction, struct sk_buff *skb)
{
#if defined(CONFIG_RTL_83XX_QOS_SUPPORT)
	// insert vlan tag for passing system priority from 98F to 83XX by 802.1p
	if(egrCVlanTagif && egrCVID && skb){
	   if(g83xxQosEnabled && ((*egrCVlanTagif) == FALSE))
	   {	
		#if defined(CONFIG_RTL_8226_SUPPORT)
			/* 	v600 lan1(bit0) is 8226 phy, not external switch port, v500 all lan ports are switch ports. 
				only insert vlan tag to 83XX switch. */
			if(!rtk_8226_lan_dev_name[0] || (rtk_8226_lan_dev_name[0] && strncmp(rtk_8226_lan_dev_name, skb->dev->name, IFNAMSIZ))){
				if(direction==flow_direction_downstream){
					*egrCVlanTagif = TRUE;
					*egrCVID = RTL_LANVLANID;
				}
			}
	   	#else
	    	*egrCVlanTagif = TRUE;
			*egrCVID = ((direction==flow_direction_downstream) ? RTL_LANVLANID : RTL_WANVLANID);
		#endif
	   }
	}
#endif
}

int rtk_pass_cos_to_83xx_qos(unsigned int cos, struct sk_buff **skb, int portMask, int wanPortMask, int direction)
{
	int ret = SUCCESS;
#if defined(CONFIG_RTL_83XX_QOS_SUPPORT)
	if(skb && (*skb) && g83xxQosEnabled){
		if(!(((*skb)->data)[0] & 0x1)){
		#if defined(CONFIG_RTL_8226_SUPPORT)
			if((direction == flow_direction_downstream) && 
			(!rtk_8226_lan_dev_name[0] || (rtk_8226_lan_dev_name[0] && strncmp(rtk_8226_lan_dev_name, (*skb)->dev->name, IFNAMSIZ))))
		#endif
			{
				if(*((unsigned short *)((*skb)->data+ETH_ALEN*2)) != __constant_htons(ETH_P_8021Q)){
					unsigned short vid = (portMask & wanPortMask) ? RTL_WANVLANID : RTL_LANVLANID;
	 				ret = rtk_insert_vlan_tag_and_priority(vid, cos, skb);
					if(ret == FAIL){
						return FAIL; 
					}
				}
				else{
					ret = rtk_mod_vlan_tag_priority(cos, skb);
					if(ret == FAIL){
						return FAIL;
					}
				}
			}
	 	}
	}
#endif
   	return ret;
}

int rtk_get_vid_by_dev(unsigned short *vid, struct net_device *dev)
{
	unsigned short vlanid = 0;
	if (!vid || !dev)
		return FAIL;

	if (is_vlan_dev(dev))
		vlanid	= vlan_dev_vlan_id(dev);
#if defined(CONFIG_FC_RTL8198F_SERIES)&&defined(CONFIG_RTL_MULTI_ETH_WAN)	
	if (is_smux_vlan_dev(dev))
		vlanid = smux_vlan_dev_vlan_id(dev);	
#endif	
	
	*vid = vlanid;
	//printk("%s %d netifTable->vlanId=%u *vid=%u dev->name=%s\n", __func__, __LINE__, vlanid, *vid, dev->name);

	return SUCCESS;
}

int rtk_fc_decide_rx_device_by_spa(unsigned int phy_src_port, struct net_device **dev)
{
	*dev = nic_decide_rx_device_by_spa(phy_src_port);
	return SUCCESS;
}

#if defined(CONFIG_RTK_SOC_RTL8198D)
int rtk_get_bind_portmask(int vid,unsigned long long *bind_port,int (*ndevice2portmask)(struct net_device*, unsigned int *,rtk_fc_wlan_devmask_t *))
#else
int rtk_get_bind_portmask(int vid,unsigned int *bind_port,int (*ndevice2portmask)(struct net_device*, unsigned int *,rtk_fc_wlan_devmask_t *))
#endif
{
#if (defined(CONFIG_FC_RTL8198F_SERIES)||defined(CONFIG_RTK_SOC_RTL8198D))&&defined(CONFIG_RTL_MULTI_ETH_WAN)&&(!defined(CONFIG_RTL_SMUX_DEV))
	int ret = FAIL,idx = 0;
	unsigned int sdev_member = 0;
	unsigned int tmp_portmask = 0;
	rtk_fc_wlan_devmask_t tmp_wlanportmask = 0;
	char* lan_dev_name = NULL;
	struct net_device *lan_dev;
	*bind_port = 0;

	ret = smux_get_sdev_member(vid,&sdev_member);
	if(ret < 0)
		return FAIL;

	//Bridge mode
	if(rtk_get_op_mode()== 1)
	{
#if defined(CONFIG_RTK_SOC_RTL8198D)
		*bind_port = 0xffffffffffffffff;
#else
		*bind_port = 0xffffffff;
#endif
		return SUCCESS;
	}

	for (idx = 0; idx < 8*sizeof(unsigned int); idx++){
		if ((1<<idx) & sdev_member){
			lan_dev_name = rtl_get_dev_name_by_bind_port_mask((1<<idx));

			if(!lan_dev_name)
				continue;

			lan_dev =  __dev_get_by_name(&init_net,lan_dev_name);

			if(!lan_dev)
				continue;
			
			if(ndevice2portmask)
				ndevice2portmask(lan_dev,&tmp_portmask,(rtk_fc_wlan_devmask_t *)&tmp_wlanportmask);

			*bind_port |= tmp_portmask;
			*bind_port |= tmp_wlanportmask;
		}
	}
#else	
#if defined(CONFIG_RTK_SOC_RTL8198D)
	*bind_port = 0xffffffffffffffff;
#else
	*bind_port = 0xffffffff;
#endif
#endif
	return SUCCESS;
}


int rtk_vlan_passthrough_tx_handle(struct sk_buff **pskb)
{
#if defined(CONFIG_RTK_VLAN_PASSTHROUGH_SUPPORT)	

	return rtk_vlan_passthrough_tx(pskb);

#else

	return SUCCESS;

#endif
	
}

int rtk_set_ext_phy_qos_portprimap(void)
{
	int ret = SUCCESS;
#if defined(CONFIG_RTK_SOC_RTL8198D)
#if defined(CONFIG_EXTERNAL_PHY_POLLING) || defined(CONFIG_RTL_8221B_SUPPORT)
	if((ret = rtk_qos_portPriMap_set(RTK_FC_MAC_PORT_iNIC, 3)) != SUCCESS){
		printk("[%s:%d]init port %d interPri and Queue Mapping failed!!!\n", __FUNCTION__, __LINE__, RTK_FC_MAC_PORT_iNIC);
	}
#endif
#endif
	return ret;
}

unsigned int rtk_init_all_mac_portmask_without_cpu(void)
{
	unsigned int portmask = 0;

#if defined(CONFIG_FC_RTL9607C_RTL9603CVD_HYBRID)
	if(fc_mgr_db.chipId == RTL9607C_CHIP_ID)
		portmask = 0x3F;
	else
		portmask = 0x1F;
	
#elif defined(CONFIG_FC_RTL9607C_SERIES)
	portmask = ((1<<RTK_FC_MAC_PORT0)|(1<<RTK_FC_MAC_PORT1)|(1<<RTK_FC_MAC_PORT2)|(1<<RTK_FC_MAC_PORT3)|(1<<RTK_FC_MAC_PORT4)|(1<<RTK_FC_MAC_PORT_PON));

#elif defined(CONFIG_FC_RTL9603CVD_SERIES)
	portmask = ((1<<RTK_FC_MAC_PORT0)|(1<<RTK_FC_MAC_PORT1)|(1<<RTK_FC_MAC_PORT2)|(1<<RTK_FC_MAC_PORT3)|(1<<RTK_FC_MAC_PORT_PON));

#elif defined(CONFIG_FC_RTL8198F_SERIES)
	#if defined(CONFIG_RTL_8367S_SUPPORT) && defined(CONFIG_RTL_8226_SUPPORT)
	portmask = ((1<<RTK_FC_MAC_PORT0)|(1<<RTK_FC_MAC_PORT1)|(1<<RTK_FC_MAC_PORT2)|(1<<RTK_FC_MAC_PORT3)|(1<<RTK_FC_MAC_PORT4)|(1<<RTK_FC_MAC_PORT_PON));
	#else
	portmask = ((1<<RTK_FC_MAC_PORT0)|(1<<RTK_FC_MAC_PORT1)|(1<<RTK_FC_MAC_PORT2)|(1<<RTK_FC_MAC_PORT3)|(1<<RTK_FC_MAC_PORT_PON));
	#endif
	
#elif defined(CONFIG_FC_CAG3_SERIES)
	if(rtk_fc_mgr_check_config_setting(FC_MGR_GET_CONFIG_LOOPBACK_MODE))
		portmask = ((1<<RTK_FC_MAC_PORT0)|(1<<RTK_FC_MAC_PORT1)|(1<<RTK_FC_MAC_PORT2)|(1<<RTK_FC_MAC_PORT3)|(1<<RTK_FC_MAC_PORT6)|(1<<RTK_FC_MAC_PORT_PON));
	else
		portmask = ((1<<RTK_FC_MAC_PORT0)|(1<<RTK_FC_MAC_PORT1)|(1<<RTK_FC_MAC_PORT2)|(1<<RTK_FC_MAC_PORT3)|(1<<RTK_FC_MAC_PORT_PON));
	
#elif defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	portmask = ((1<<RTK_FC_MAC_PORT0)|(1<<RTK_FC_MAC_PORT1)|(1<<RTK_FC_MAC_PORT2)|(1<<RTK_FC_MAC_PORT3)|(1<<RTK_FC_MAC_PORT_PON));
	
	{
		/* find activated ports according to scfg */
		int stat, portidx;
		
		for(portidx=RTK_FC_MAC_PORT0; portidx<=RTK_FC_MAC_PORT_PON; portidx++) {
			aal_port_physical_port_enable_get(0, portidx, &stat);
			if(stat) portmask |= (1<<portidx);
		}
	}
#endif

#if defined(CONFIG_RTK_L34_G3_PLATFORM)
#if defined(CONFIG_LAN_SDS_FEATURE)
	portmask |= (1<<RTK_FC_MAC_PORT4);
#endif
#endif

	return portmask;
}

int rtk_fc_mgr_system_post_init(void)
{

	return SUCCESS;
}

int rtk_fc_get_ipfrag_max_hash_size(void)
{
	return RTK_FC_IPFRAG_MAX_HASH_SIZE;
}

int rtk_fc_get_ipfrag_max_hash_entry_size(void)
{
	return RTK_FC_IPFRAG_MAX_HASH_ENTRY_SIZE;
}

int rtk_fc_get_ipfrag_cache_timeout_msec(void)
{
	return RTK_FC_IPFRAG_CACHE_TIMEOUT_MSEC;
}

#if defined(CONFIG_RTK_SOC_RTL8198D)
int rtk_linux_vlan_rx_handle(struct sk_buff **pskb, unsigned int portmask)
{
#if defined(CONFIG_RTK_LINUX_VLAN_SUPPORT) 
	return rtk_linux_vlan_rx(pskb, portmask);
#else
	return SUCCESS;
#endif
}

int rtk_linux_vlan_tx_handle(struct sk_buff **pskb, unsigned int portmask)
{
#if defined(CONFIG_RTK_LINUX_VLAN_SUPPORT) 
	return rtk_linux_vlan_tx(pskb, portmask);
#else
	return SUCCESS;
#endif
}

int rtk_fc_get_local_ip_addr_by_skb(struct sk_buff *skb, bool is_uplink)
{
	uint32 off = (ETH_HLEN - 2);

	if (unlikely(!skb || !skb->data))
		return FAIL;

	if ((*(u16*)(skb->data+off)) == htons(ETH_P_8021Q)) 
		off += VLAN_HLEN;

	if ((*(u16*)(skb->data+off)) == htons(ETH_P_IP)) {
		struct iphdr *iph = (struct iphdr *)((unsigned char *)skb->data+off+2);
		return (is_uplink ? ntohl(iph->saddr) : ntohl(iph->daddr));
	}

	return FAIL;
}

int rtk_fc_get_local_ip_addr_by_skb_data(unsigned char *data, bool is_uplink)
{
	uint32 off = (ETH_HLEN - 2);

	if (unlikely(!data))
		return FAIL;

	if ((*(u16*)(data+off)) == htons(ETH_P_8021Q)) 
		off += VLAN_HLEN;

	if ((*(u16*)(data+off)) == htons(ETH_P_IP)) {
		struct iphdr *iph = (struct iphdr *)((unsigned char *)data+off+2);
		return (is_uplink ? ntohl(iph->saddr) : ntohl(iph->daddr));
	}

	return FAIL;
}

void rtk_fc_cleanup_ext_flow_mib(void)
{
	rtk_fc_ext_flow_mib_ctrl_t *pctrl = &fc_mgr_db.extFlowMibControl;
	rtk_fc_ext_flow_mib_entry_t *extFlowMibTbl = fc_mgr_db.extFlowMibTbl;
	int i;
	rt_flow_ops_data_t flow_ops_data;

#if defined(CONFIG_WFO_OFFLOAD_2G)
	// stop traffic count wfo workqueue
	if (pctrl->wfo_wq_running)
		rtk_fc_wfo_wq_stop();
#endif

	rtk_fc_mgr_traff_cnt_spin_lock_bh();

	// clear host and counter
	memset(extFlowMibTbl, 0, (sizeof(rtk_fc_ext_flow_mib_entry_t) * RT_STAT_EXT_FLOWMIB_TABLE_SIZE));

	// clear debug counter
	memset(&pctrl->__clean_area_start, 0, offsetof(rtk_fc_ext_flow_mib_ctrl_t, __clean_area_end) -
											offsetof(rtk_fc_ext_flow_mib_ctrl_t, __clean_area_start));

	for (i = 0 ; i < RT_STAT_FLOWMIB_TABLE_SIZE; i++) {
		rt_stat_flowMib_reset(i);
	}

	rt_flow_operation_add(RT_FLOW_OPS_FLUSH_FLOWTABLE, &flow_ops_data);

	rtk_fc_mgr_traff_cnt_spin_unlock_bh();

	return;
}

uint8 rtk_fc_get_ext_flow_mib_control(void)
{
	return fc_mgr_db.extFlowMibControl.enable;
}

uint8 rtk_fc_set_ext_flow_mib_control(uint8 value)
{
	if (value != fc_mgr_db.extFlowMibControl.enable) {
		fc_mgr_db.extFlowMibControl.enable = value;
		rtk_fc_cleanup_ext_flow_mib();
	}

	return SUCCESS;
}

uint8 rtk_fc_get_ext_flow_mib_control_mode(void)
{
	return fc_mgr_db.extFlowMibControl.mode;
}

uint8 rtk_fc_set_ext_flow_mib_control_mode(uint8 mode)
{
	if (mode != fc_mgr_db.extFlowMibControl.mode) {
		fc_mgr_db.extFlowMibControl.mode = mode;
		rtk_fc_cleanup_ext_flow_mib();
	}

	return SUCCESS;
}

/*
	0: debug_off
	1: debug_on
	2: debug_wfo_off
	3: debug_wfo_on
	4: test wfo callback func
	[20:23]: wq bind cpu
	[100:10000]: wq sleep msec
*/
uint8 rtk_fc_set_ext_flow_mib_debug(uint32 value)
{
	rtk_fc_ext_flow_mib_ctrl_t *pctrl = &fc_mgr_db.extFlowMibControl;

	if (value < 2) {
		pctrl->debug = value;

		pctrl->wlan_to_nic_dec = 0;
		pctrl->wlan_to_nic_inc = 0;
		pctrl->wlan_to_nic_amsdu_inc = 0;
		pctrl->nic_to_wlan_inc = 0;
		pctrl->nic_to_wlan_agg_inc = 0;
	}
#if defined(CONFIG_WFO_OFFLOAD_2G)
	else if (value < 4) {
		pctrl->debug_wfo = (value == 2 ? 0 : 1);

		pctrl->wfo_wq_start_ok 		= 0;
		pctrl->wfo_wq_start_err 	= 0;
		pctrl->wfo_wq_end_ok 		= 0;
		pctrl->wfo_wq_end_err		= 0;
		pctrl->wfo_wq_sched 		= 0;
		pctrl->wfo_func_null 		= 0;
		pctrl->try_update 			= 0;
		pctrl->kzalloc_fail 		= 0;
		pctrl->wfo_sta_dev_null 	= 0;
		pctrl->get_wfo_dev_fail 	= 0;
		pctrl->update_by_wfo 		= 0;
		pctrl->update_miss 			= 0;
	}
	else if (value < 5) {
		if (rtk_fc_wfo_deliver_sta_status) {
			int sta_size = sizeof(struct wfo_sta_st_s) * RTK_WFO_IFACE_NUM * RTK_WFO_STA_NUM;
			struct wfo_sta_st_s *psta_st = kzalloc(sta_size, GFP_KERNEL);

			if (psta_st) {
				int m, n, offset;
				struct wfo_sta_st_s *entry;

				rtk_fc_wfo_deliver_sta_status(psta_st, sta_size);

				printk("psta_st=%p, entry_size=%d\n", psta_st, sizeof(struct wfo_sta_st_s));

				for (m = 0; m < RTK_WFO_IFACE_NUM; m++) {
					for (n = 0; n < RTK_WFO_STA_NUM; n++) {
						offset = (m * RTK_WFO_STA_NUM) + n;
						entry = psta_st + offset;

						if (entry && NON_ZERO_MAC(entry->mac_addr)) {
							printk(" - %p  [%d][%d] mac: %pM		rx: %llu %llu	tx: %llu %llu \n", 
								entry, m, n, entry->mac_addr, entry->rx_bytes, entry->rx_pkts, entry->tx_bytes, entry->tx_pkts);
						}
					}
				}

				kfree(psta_st);
			}
			else {
				printk(" - rtk_fc_wfo_deliver_sta_status kzalloc fail!\n");
			}
		}
		else {
			printk(" - rtk_fc_wfo_deliver_sta_status NULL!\n");
		}
	}
	else if (value > 19 && value < 24) {
		pctrl->wfo_wq_bind_cpu = (value - 20);
	}
	else if (value > 99 && value < 10001) {
		pctrl->wfo_wq_sleep_msec = value;
	}
#endif

	return SUCCESS;
}

// for wlan STA only
int rtk_fc_add_ext_flow_mib_count_by_lan_ip(rtk_fc_ext_flow_mib_host_mapping_t *pHost, rtk_fc_ext_flow_mib_net_mapping_t *pNet)
{
	int i, start, end, ret = RTK_FC_HELPER_RET_ERR;
	char is_wlan_5g;
	rtk_fc_ext_flow_mib_ctrl_t *pctrl = &fc_mgr_db.extFlowMibControl;
	rtk_fc_ext_flow_mib_entry_t *extFlowMibTbl = fc_mgr_db.extFlowMibTbl;
#if defined(CONFIG_WFO_OFFLOAD_2G)
	int wq_start = 0;
#endif

	if (fc_mgr_db.extFlowMibControl.mode != RTK_EXT_FLOW_MIB_IPV4_BASED)
		return RTK_FC_HELPER_RET_ERR;

	if (unlikely(!pHost || !pNet || pHost->ip_addr == 0))
		return RTK_FC_HELPER_RET_ERR_NULL_POINTER;

	if (!pHost->is_wlan_sta)
		return ret;

	rtk_fc_mgr_traff_cnt_spin_lock_bh();

	if ((rtk_fc_get_ext_flow_mib_idx_by_lan_ip(pHost)) != FAIL) {
		printk("[exist]\n");
		ret = RTK_FC_HELPER_RET_OK;
		goto out;
	}

	is_wlan_5g = (pHost->is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_WLAN_5G ? 1 : 0);

	if (is_wlan_5g) {
		start 	= pctrl->wlan_5g_hw_start;
		end 	= pctrl->wlan_5g_hw_end;
	}
	else {
		start 	= pctrl->wlan_2g_hw_start;
		end 	= pctrl->wlan_2g_hw_end;
	}

	for (i = start; i < end; i++) {
		if (extFlowMibTbl[i].host.ip_addr == 0) {
			memcpy(&extFlowMibTbl[i].host, pHost, sizeof(rtk_fc_ext_flow_mib_host_mapping_t));
			memcpy(&extFlowMibTbl[i].net, pNet, sizeof(rtk_fc_ext_flow_mib_net_mapping_t));
			ret = RTK_FC_HELPER_RET_OK;
			break;
		}
	}

	if (i == end) {		// hw entry full
		start 	= pctrl->wlan_sw_start;
		end 	= pctrl->wlan_sw_end;

		for (i = start; i < end; i++) {
			if (extFlowMibTbl[i].host.ip_addr == 0) {
				memcpy(&extFlowMibTbl[i].host, pHost, sizeof(rtk_fc_ext_flow_mib_host_mapping_t));
				memcpy(&extFlowMibTbl[i].net, pNet, sizeof(rtk_fc_ext_flow_mib_net_mapping_t));

				if (is_wlan_5g) {
					pctrl->wlan_5g_sw_count_entry++;
				}
				else {
					pctrl->wlan_2g_sw_count_entry++;
				#if defined(CONFIG_WFO_OFFLOAD_2G)
					pctrl->need_update_dev = 1;		// ipv4 based: update dev and mac
					wq_start = (pctrl->wlan_2g_sw_count_entry == 1 ? 1 : 0);
				#endif
				}

				ret = RTK_FC_HELPER_RET_OK;
				break;
			}
		}
	}

out:
	rtk_fc_mgr_traff_cnt_spin_unlock_bh();

#if defined(CONFIG_WFO_OFFLOAD_2G)
	if (wq_start) {
		rtk_fc_wfo_wq_start();
	}
#endif

	return ret;
}

// for eth device only
int rtk_fc_add_eth_ext_flow_mib_count_by_lan_ip(rtk_fc_ext_flow_mib_host_mapping_t *pHost, rtk_fc_ext_flow_mib_net_mapping_t *pNet)
{
	int i, ret = RTK_FC_HELPER_RET_ERR;

	if (fc_mgr_db.extFlowMibControl.mode != RTK_EXT_FLOW_MIB_IPV4_BASED)
		return RTK_FC_HELPER_RET_ERR;

	if (unlikely(!pHost || !pNet || pHost->ip_addr == 0))
		return RTK_FC_HELPER_RET_ERR_NULL_POINTER;

	// for eth device only
	if (pHost->is_wlan_sta)
		return RTK_FC_HELPER_RET_ERR_INVALID_PARAM;

	rtk_fc_mgr_traff_cnt_spin_lock_bh();

	if ((rtk_fc_get_ext_flow_mib_idx_by_lan_ip(pHost)) != FAIL) {
		printk("[exist]\n");
		ret = RTK_FC_HELPER_RET_OK;
		goto out;
	}

	for (i = 0; i < RT_STAT_EXT_FLOWMIB_ETH_ENTRY_SIZE; i++) {
		if (fc_mgr_db.extFlowMibTbl[i].host.ip_addr == 0) {
			memcpy(&fc_mgr_db.extFlowMibTbl[i].host, pHost, sizeof(rtk_fc_ext_flow_mib_host_mapping_t));
			memcpy(&fc_mgr_db.extFlowMibTbl[i].net, pNet, sizeof(rtk_fc_ext_flow_mib_net_mapping_t));
			ret = RTK_FC_HELPER_RET_OK;
			break;
		}
	}

	if (i == RT_STAT_EXT_FLOWMIB_ETH_ENTRY_SIZE)
		ret = RTK_FC_HELPER_RET_ERR_TBL_FULL;
out:
	rtk_fc_mgr_traff_cnt_spin_unlock_bh();
	return ret;
}

// for both wlan and eth device
int rtk_fc_add_ext_flow_mib_count_by_lan_mac(unsigned char *mac_addr, unsigned char is_wlan_sta)
{
	uint8 i, start, end;
	int ret;
	rtk_fc_ext_flow_mib_ctrl_t *pctrl = &fc_mgr_db.extFlowMibControl;
	rtk_fc_ext_flow_mib_entry_t *extFlowMibTbl = fc_mgr_db.extFlowMibTbl;
#if defined(CONFIG_WFO_OFFLOAD_2G)
	int wq_start = 0;
#endif

	if (fc_mgr_db.extFlowMibControl.mode != RTK_EXT_FLOW_MIB_MAC_BASED)
		return RTK_FC_HELPER_RET_ERR;

	if (unlikely(!mac_addr)) 
		return RTK_FC_HELPER_RET_ERR_NULL_POINTER;

	rtk_fc_mgr_traff_cnt_spin_lock_bh();

	if (rtk_fc_get_ext_flow_mib_idx_by_lan_mac(mac_addr, is_wlan_sta) != FAIL) {
		printk("[exist]\n");
		ret = RTK_FC_HELPER_RET_OK;
		goto out;
	}

	if (is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_ETH) {
		start = pctrl->eth_hw_start;
		end = pctrl->eth_hw_end;
	}
	else if (is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_WLAN_5G) {
		start 	= pctrl->wlan_5g_hw_start;
		end 	= pctrl->wlan_5g_hw_end;
	}
	else {
		start 	= pctrl->wlan_2g_hw_start;
		end 	= pctrl->wlan_2g_hw_end;
	}

	for (i = start; i < end; i++) {
		if (ZERO_MAC(fc_mgr_db.extFlowMibTbl[i].host.mac_addr)) {
			memcpy(extFlowMibTbl[i].host.mac_addr, mac_addr, ETH_ALEN);
			extFlowMibTbl[i].host.is_wlan_sta = is_wlan_sta;
			ret = RTK_FC_HELPER_RET_OK;
			goto out;
		}
	}

	if (is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_ETH && i == RT_STAT_EXT_FLOWMIB_ETH_ENTRY_SIZE) {
		ret = RTK_FC_HELPER_RET_ERR_TBL_FULL;
		goto out;
	}

	start 	= pctrl->wlan_sw_start;
	end 	= pctrl->wlan_sw_end;

	for (i = start; i < end; i++) {
		if (ZERO_MAC(fc_mgr_db.extFlowMibTbl[i].host.mac_addr)) {
			memcpy(extFlowMibTbl[i].host.mac_addr, mac_addr, ETH_ALEN);
			extFlowMibTbl[i].host.is_wlan_sta = is_wlan_sta;

			if (is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_WLAN_5G) {
				pctrl->wlan_5g_sw_count_entry++;
			}
			else {
				pctrl->wlan_2g_sw_count_entry++;
			#if defined(CONFIG_WFO_OFFLOAD_2G)
				pctrl->need_update_dev = 1;		// mac based: update dev
				wq_start = (pctrl->wlan_2g_sw_count_entry == 1 ? 1 : 0);
			#endif
			}

			ret = RTK_FC_HELPER_RET_OK;
			goto out;
		}
	}

	ret = RTK_FC_HELPER_RET_ERR_TBL_FULL;

out:
	rtk_fc_mgr_traff_cnt_spin_unlock_bh();

#if defined(CONFIG_WFO_OFFLOAD_2G)
	if (wq_start) {
		rtk_fc_wfo_wq_start();
	}
#endif

	return ret;
}

int rtk_fc_delete_ext_flow_mib_count_by_lan_ip(rtk_fc_ext_flow_mib_host_mapping_t *pHost)
{
	int i, ret = RTK_FC_HELPER_RET_ERR;
	rtk_fc_ext_flow_mib_ctrl_t *pctrl = &fc_mgr_db.extFlowMibControl;
#if defined(CONFIG_WFO_OFFLOAD_2G)
	int wq_stop = 0;
#endif

	if (pctrl->mode != RTK_EXT_FLOW_MIB_IPV4_BASED)
		return RTK_FC_HELPER_RET_ERR;

	if (unlikely(!pHost || pHost->ip_addr == 0))
		return FAIL;

	rtk_fc_mgr_traff_cnt_spin_lock_bh();

	if ((i = rtk_fc_get_ext_flow_mib_idx_by_lan_ip(pHost)) != FAIL) {
		memset(&fc_mgr_db.extFlowMibTbl[i], 0, sizeof(rtk_fc_ext_flow_mib_entry_t));
		ret = RTK_FC_HELPER_RET_OK;
	}

	if (i >= 0 && i < RT_STAT_EXT_HW_FLOWMIB_ENTRY_SIZE) {
		rt_stat_flowMib_reset(RT_STAT_UP_HW_FLOW_MIB_MAPPED_FROM_EXT_FLOW_MIB(i));
		rt_stat_flowMib_reset(RT_STAT_DOWN_HW_FLOW_MIB_MAPPED_FROM_EXT_FLOW_MIB(i));
	}

	if (i >= pctrl->wlan_sw_start && i < pctrl->wlan_sw_end) {
		if (pHost->is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_WLAN_5G) {
			pctrl->wlan_5g_sw_count_entry--;
		}
		else {
			pctrl->wlan_2g_sw_count_entry--;
		#if defined(CONFIG_WFO_OFFLOAD_2G)
			wq_stop = (pctrl->wlan_2g_sw_count_entry == 0 ? 1 : 0);
		#endif
		}
	}

	rtk_fc_mgr_traff_cnt_spin_unlock_bh();

#if defined(CONFIG_WFO_OFFLOAD_2G)
	if (wq_stop) {
		rtk_fc_wfo_wq_stop();
	}
#endif

	return ret;
}

int rtk_fc_delete_ext_flow_mib_count_by_lan_mac(unsigned char *mac_addr, unsigned char is_wlan_sta)
{
	int i, ret = RTK_FC_HELPER_RET_ERR;
	rtk_fc_ext_flow_mib_ctrl_t *pctrl = &fc_mgr_db.extFlowMibControl;
#if defined(CONFIG_WFO_OFFLOAD_2G)
	int wq_stop = 0;
#endif

	if (pctrl->mode != RTK_EXT_FLOW_MIB_MAC_BASED)
		return RTK_FC_HELPER_RET_ERR;

	if (unlikely(!mac_addr))
		return FAIL;

	rtk_fc_mgr_traff_cnt_spin_lock_bh();

	if ((i = rtk_fc_get_ext_flow_mib_idx_by_lan_mac(mac_addr, is_wlan_sta)) != FAIL) {
		memset(&fc_mgr_db.extFlowMibTbl[i], 0, sizeof(rtk_fc_ext_flow_mib_entry_t));
		ret = RTK_FC_HELPER_RET_OK;
	}

	if (i >= 0 && i < RT_STAT_EXT_HW_FLOWMIB_ENTRY_SIZE) {
		rt_stat_flowMib_reset(RT_STAT_UP_HW_FLOW_MIB_MAPPED_FROM_EXT_FLOW_MIB(i));
		rt_stat_flowMib_reset(RT_STAT_DOWN_HW_FLOW_MIB_MAPPED_FROM_EXT_FLOW_MIB(i));

	}

	if (i >= pctrl->wlan_sw_start && i < pctrl->wlan_sw_end) {
		if (is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_WLAN_5G) {
			pctrl->wlan_5g_sw_count_entry--;
		}
		else {
			pctrl->wlan_2g_sw_count_entry--;
		#if defined(CONFIG_WFO_OFFLOAD_2G)
			wq_stop = (pctrl->wlan_2g_sw_count_entry == 0 ? 1 : 0);
		#endif
		}
	}

	rtk_fc_mgr_traff_cnt_spin_unlock_bh();

#if defined(CONFIG_WFO_OFFLOAD_2G)
	if (wq_stop) {
		rtk_fc_wfo_wq_stop();
	}
#endif

	return ret;
}

int rtk_fc_flush_ext_flow_mib_count_by_lan_ip(rtk_fc_ext_flow_mib_host_mapping_t *pHost)
{
	int i, ret = RTK_FC_HELPER_RET_ERR;

	if (fc_mgr_db.extFlowMibControl.mode != RTK_EXT_FLOW_MIB_IPV4_BASED)
		return RTK_FC_HELPER_RET_ERR;

	if (unlikely(!pHost || pHost->ip_addr == 0))
		return RTK_FC_HELPER_RET_ERR_NULL_POINTER;

	rtk_fc_mgr_traff_cnt_spin_lock_bh();

	if ((i = rtk_fc_get_ext_flow_mib_idx_by_lan_ip(pHost)) != FAIL) {
		memset(&fc_mgr_db.extFlowMibTbl[i].counter, 0, sizeof(rtk_fc_ext_flow_mib_counter_t));
		memset(&fc_mgr_db.extFlowMibTbl[i].sw_counter, 0, sizeof(rtk_fc_ext_flow_mib_counter_t));
		memset(&fc_mgr_db.extFlowMibTbl[i].kernel_counter, 0, sizeof(rtk_fc_ext_flow_mib_counter_t));
		ret = RTK_FC_HELPER_RET_OK;
	}


	if (i >= 0 && i < RT_STAT_EXT_HW_FLOWMIB_ENTRY_SIZE) {
		rt_stat_flowMib_reset(RT_STAT_UP_HW_FLOW_MIB_MAPPED_FROM_EXT_FLOW_MIB(i));
		rt_stat_flowMib_reset(RT_STAT_DOWN_HW_FLOW_MIB_MAPPED_FROM_EXT_FLOW_MIB(i));
	}

	rtk_fc_mgr_traff_cnt_spin_unlock_bh();

	return ret;
}

int rtk_fc_flush_ext_flow_mib_count_by_lan_mac(unsigned char *mac_addr, unsigned char is_wlan_sta)
{
	int i, ret = RTK_FC_HELPER_RET_ERR;

	if (fc_mgr_db.extFlowMibControl.mode != RTK_EXT_FLOW_MIB_MAC_BASED)
		return RTK_FC_HELPER_RET_ERR;

	if (unlikely(!mac_addr))
		return RTK_FC_HELPER_RET_ERR_NULL_POINTER;

	rtk_fc_mgr_traff_cnt_spin_lock_bh();

	if ((i = rtk_fc_get_ext_flow_mib_idx_by_lan_mac(mac_addr, is_wlan_sta)) != FAIL) {
		memset(&fc_mgr_db.extFlowMibTbl[i].counter, 0, sizeof(rtk_fc_ext_flow_mib_counter_t));
		memset(&fc_mgr_db.extFlowMibTbl[i].sw_counter, 0, sizeof(rtk_fc_ext_flow_mib_counter_t));
		memset(&fc_mgr_db.extFlowMibTbl[i].kernel_counter, 0, sizeof(rtk_fc_ext_flow_mib_counter_t));
		ret = RTK_FC_HELPER_RET_OK;
	}


	if (i >= 0 && i < RT_STAT_EXT_HW_FLOWMIB_ENTRY_SIZE) {
		rt_stat_flowMib_reset(RT_STAT_UP_HW_FLOW_MIB_MAPPED_FROM_EXT_FLOW_MIB(i));
		rt_stat_flowMib_reset(RT_STAT_DOWN_HW_FLOW_MIB_MAPPED_FROM_EXT_FLOW_MIB(i));
	}

	rtk_fc_mgr_traff_cnt_spin_unlock_bh();

	return ret;
}

int rtk_fc_get_ext_flow_mib_idx_by_lan_ip(rtk_fc_ext_flow_mib_host_mapping_t *pHost)
{
	int i;
	rtk_fc_ext_flow_mib_host_mapping_t *host;
	rtk_fc_ext_flow_mib_entry_t *extFlowMibTbl = fc_mgr_db.extFlowMibTbl;
	rtk_fc_ext_flow_mib_ctrl_t ctrl = fc_mgr_db.extFlowMibControl;
	int start = ctrl.wlan_start, end = ctrl.wlan_end;

	if (ctrl.mode != RTK_EXT_FLOW_MIB_IPV4_BASED)
		return FAIL;

	if (unlikely(!pHost || pHost->ip_addr == 0))
		return FAIL;

	if (pHost->is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_ETH) {
		start	= ctrl.eth_hw_start;
		end 	= ctrl.eth_hw_end;
	}

	for (i = start; i < end; i++) {
		host = &extFlowMibTbl[i].host;
		if (host->ip_addr && (host->ip_addr == pHost->ip_addr) && (host->is_wlan_sta == pHost->is_wlan_sta))
			return i;
	}

	return FAIL;
}

int rtk_fc_get_ext_flow_mib_idx_by_lan_mac(unsigned char *mac_addr, unsigned char is_wlan_sta)
{
	int i;
	rtk_fc_ext_flow_mib_host_mapping_t *host;
	rtk_fc_ext_flow_mib_entry_t *extFlowMibTbl = fc_mgr_db.extFlowMibTbl;
	rtk_fc_ext_flow_mib_ctrl_t ctrl = fc_mgr_db.extFlowMibControl;
	int start = ctrl.wlan_start, end = ctrl.wlan_end;

	if (ctrl.mode != RTK_EXT_FLOW_MIB_MAC_BASED)
		return FAIL;

	if (unlikely(!mac_addr))
		return FAIL;

	if (is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_ETH) {
		start	= ctrl.eth_hw_start;
		end 	= ctrl.eth_hw_end;
	}

	for (i = start; i < end; i++) {
		host = &extFlowMibTbl[i].host;
		if (NON_ZERO_MAC(host->mac_addr) && (host->is_wlan_sta == is_wlan_sta) && ether_addr_equal(host->mac_addr, mac_addr))
			return i;
	}

	return FAIL;
}

int rtk_fc_print_ext_flow_mib_count_by_mib_idx(uint32 idx)
{
	if (unlikely(idx >= RT_STAT_EXT_FLOWMIB_TABLE_SIZE))
		return RTK_FC_HELPER_RET_ERR_INVALID_PARAM;

	if (fc_mgr_db.extFlowMibControl.mode == RTK_EXT_FLOW_MIB_IPV4_BASED) {
		printk("[%d] ip_addr: %pI4, is_wlan_sta: %d, netmask: %pI4\n", idx, 
			&fc_mgr_db.extFlowMibTbl[idx].host.ip_addr, fc_mgr_db.extFlowMibTbl[idx].host.is_wlan_sta, 
			&fc_mgr_db.extFlowMibTbl[idx].net.netmask);

		printk("     mac_addr: %pM, dev_flag: %d, dev : %s\n", 
			&fc_mgr_db.extFlowMibTbl[idx].host.mac_addr, fc_mgr_db.extFlowMibTbl[idx].host.dev_flag, fc_mgr_db.extFlowMibTbl[idx].host.dev_name);
	}
	else {	// mac-based
		printk("[%d] mac_addr: %pM, dev_flag: %d, dev : %s\n", idx, &fc_mgr_db.extFlowMibTbl[idx].host.mac_addr, fc_mgr_db.extFlowMibTbl[idx].host.dev_flag, fc_mgr_db.extFlowMibTbl[idx].host.dev_name);
	}

	printk("\t   HW:\n");
	printk("\t   rx_packet: %u, rx_byte: %u\n", 
		fc_mgr_db.extFlowMibTbl[idx].counter.in_packet_cnt, fc_mgr_db.extFlowMibTbl[idx].counter.in_byte_cnt);
	printk("\t   tx_packet: %u, tx_byte: %u\n", 
		fc_mgr_db.extFlowMibTbl[idx].counter.out_packet_cnt, fc_mgr_db.extFlowMibTbl[idx].counter.out_byte_cnt);
	printk("\t   SW:\n");
	printk("\t   rx_packet: %u, rx_byte: %u\n", 
		fc_mgr_db.extFlowMibTbl[idx].sw_counter.in_packet_cnt, fc_mgr_db.extFlowMibTbl[idx].sw_counter.in_byte_cnt);
	printk("\t   tx_packet: %u, tx_byte: %u\n", 
		fc_mgr_db.extFlowMibTbl[idx].sw_counter.out_packet_cnt, fc_mgr_db.extFlowMibTbl[idx].sw_counter.out_byte_cnt);
	printk("\t   Kernel:\n");
	printk("\t   rx_packet: %u, rx_byte: %u\n", 
		fc_mgr_db.extFlowMibTbl[idx].kernel_counter.in_packet_cnt, fc_mgr_db.extFlowMibTbl[idx].kernel_counter.in_byte_cnt);
	printk("\t   tx_packet: %u, tx_byte: %u\n", 
		fc_mgr_db.extFlowMibTbl[idx].kernel_counter.out_packet_cnt, fc_mgr_db.extFlowMibTbl[idx].kernel_counter.out_byte_cnt);
	return RTK_FC_HELPER_RET_OK;
}

int rtk_fc_print_ext_flow_mib_count_all(void)
{
	uint32 i;
	rtk_fc_ext_flow_mib_ctrl_t ctrl = fc_mgr_db.extFlowMibControl;

	printk("Extend flow mib enable: %d\n", ctrl.enable);
	printk(" - mode: %s\n", str_flow_mib_mode[ctrl.mode]);

	printk(" - wan port mask\t\t0x%llx\n", fc_mgr_db.wanPortMask.portmask);
	printk(" - br0_ip\t\t%pI4\n", &fc_mgr_db.br0_ip);
	printk(" - br0_mac\t\t%pM\n", fc_mgr_db.br0_mac);

	printk(" - eth\t\t\t[%d,	%d)\n", ctrl.eth_hw_start, ctrl.eth_hw_end);
	printk(" - wlan\t\t\t[%d,	%d)\n", ctrl.wlan_start, ctrl.wlan_end);
	printk(" -	|hw 5g\t\t[%d,	%d)\n", ctrl.wlan_5g_hw_start, ctrl.wlan_5g_hw_end);
	printk(" -	|hw 2g\t\t[%d,	%d)\n", ctrl.wlan_2g_hw_start, ctrl.wlan_2g_hw_end);
	printk(" -	|sw\t\t[%d,	%d)\n\n",ctrl.wlan_sw_start, ctrl.wlan_sw_end);

	printk(" - wlan sw 5g\t\t[%s: %d]\n", RTK_FC_WLAN_NAME_5G, ctrl.wlan_5g_sw_count_entry);
	printk(" - wlan sw 2g\t\t[%s: %d]\n", RTK_FC_WLAN_NAME_2G, ctrl.wlan_2g_sw_count_entry);
	printk(" - need_update_dev\t\t%d\n\n", ctrl.need_update_dev);

	if (ctrl.debug) {
		printk("\ndebug: %d\n", ctrl.debug);
		printk(" - wlan_to_nic_inc: \t\t%u\n",  ctrl.wlan_to_nic_inc);
		printk(" - wlan_to_nic_amsdu_inc: \t%u\n",  ctrl.wlan_to_nic_amsdu_inc);
		printk(" - wlan_to_nic_dec: \t\t%u\n",  ctrl.wlan_to_nic_dec);
		printk(" - nic_to_wlan_inc: \t\t%u\n",  ctrl.nic_to_wlan_inc);
		printk(" - nic_to_wlan_agg_inc: \t\t%u\n\n",  ctrl.nic_to_wlan_agg_inc);
	}

	if (ctrl.debug_wfo) {
		printk("\ndebug_wfo: %d\n", ctrl.debug_wfo);
		printk(" - wfo_wq_running: \t\t%u\n", ctrl.wfo_wq_running);
		printk(" - wq_bind_cpu: \t\t\t%u\n", ctrl.wfo_wq_bind_cpu);
		printk(" - wq_sleep_msec: \t\t%u\n", ctrl.wfo_wq_sleep_msec);
		printk(" - wq_start_ok: \t\t\t%u\n", ctrl.wfo_wq_start_ok);
		printk(" - wq_end_ok: \t\t\t%u\n", ctrl.wfo_wq_end_ok);
		printk(" - wq_sched: \t\t\t%u\n", ctrl.wfo_wq_sched);
		printk(" - try_update: \t\t\t%u\n", ctrl.try_update);
		printk(" - update_by_wfo: \t\t%u\n\n", ctrl.update_by_wfo);

		printk(" - wq_start_err: \t\t%u\n", ctrl.wfo_wq_start_err);
		printk(" - wq_end_err: \t\t\t%u\n", ctrl.wfo_wq_end_err);
		printk(" - wfo_func_null: \t\t%u\n", ctrl.wfo_func_null);
		printk(" - kzalloc_fail: \t\t%u\n", ctrl.kzalloc_fail);
		printk(" - wfo_sta_dev_null: \t\t%u\n", ctrl.wfo_sta_dev_null);
		printk(" - get_wfo_dev_fail: \t\t%u\n", ctrl.get_wfo_dev_fail);
		printk(" - update_miss: \t\t\t%u\n", ctrl.update_miss);
	}

	printk("===================================================\n");

	if (ctrl.enable) {
		rtk_fc_mgr_traff_cnt_spin_lock_bh();

		for (i = 0U; i < RT_STAT_EXT_HW_FLOWMIB_ENTRY_SIZE; i++) {
			if (rtk_fc_update_ext_flow_mib_by_hw_flow_mib(i) != 0)
				printk("rtk_fc_update_ext_flow_mib_by_hw_flow_mib [%d] fail!\n", i);
		}

		for (i = 0U; i < RT_STAT_EXT_FLOWMIB_TABLE_SIZE; i++) {
			if (ctrl.mode == RTK_EXT_FLOW_MIB_IPV4_BASED) {
				if (!fc_mgr_db.extFlowMibTbl[i].host.ip_addr)
					continue;
			}
			else {	// mac based
				if (ZERO_MAC(fc_mgr_db.extFlowMibTbl[i].host.mac_addr))
					continue;
			}

			rtk_fc_print_ext_flow_mib_count_by_mib_idx(i);
		}

		rtk_fc_mgr_traff_cnt_spin_unlock_bh();
	}

	return RTK_FC_HELPER_RET_OK;
}

int rtk_fc_update_ext_flow_mib_by_hw_flow_mib(uint32 mib_idx)
{
	rt_stat_flow_mib_t hwFlowMib1, hwFlowMib2;
	uint32 hw_mib_idx;

	if (mib_idx >= RT_STAT_EXT_HW_FLOWMIB_ENTRY_SIZE)
		return RTK_FC_HELPER_RET_ERR;

	hw_mib_idx = RT_STAT_UP_HW_FLOW_MIB_MAPPED_FROM_EXT_FLOW_MIB(mib_idx);
	hwFlowMib1.ingress_packet_count	= 0;
	hwFlowMib1.ingress_byte_count 	= 0;
	hwFlowMib1.egress_packet_count	= 0;
	hwFlowMib1.egress_byte_count 	= 0;

	if (rt_stat_hwFlowMib_get(hw_mib_idx, &hwFlowMib1) != 0)
		return RTK_FC_HELPER_RET_ERR;

	hw_mib_idx = RT_STAT_DOWN_HW_FLOW_MIB_MAPPED_FROM_EXT_FLOW_MIB(mib_idx);
	hwFlowMib2.ingress_packet_count	= 0;
	hwFlowMib2.ingress_byte_count 	= 0;
	hwFlowMib2.egress_packet_count	= 0;
	hwFlowMib2.egress_byte_count 	= 0;

	if (rt_stat_hwFlowMib_get(hw_mib_idx, &hwFlowMib2) != 0)
		return RTK_FC_HELPER_RET_ERR;

	fc_mgr_db.extFlowMibTbl[mib_idx].counter.in_packet_cnt 	= hwFlowMib1.egress_packet_count;		// flow rx
	fc_mgr_db.extFlowMibTbl[mib_idx].counter.in_byte_cnt 	= (uint32)hwFlowMib1.egress_byte_count;

	fc_mgr_db.extFlowMibTbl[mib_idx].counter.out_packet_cnt 	= hwFlowMib2.egress_packet_count;	// flow tx
	fc_mgr_db.extFlowMibTbl[mib_idx].counter.out_byte_cnt 	= (uint32)hwFlowMib2.egress_byte_count;

	return RTK_FC_HELPER_RET_OK;
}

int rtk_fc_update_ext_flow_mib_by_idx(uint32 mib_idx, rtk_fc_ext_flow_mib_param_t *pParam)
{
	rtk_fc_ext_flow_mib_entry_t *extFlowMibTbl = fc_mgr_db.extFlowMibTbl;
	
	if (unlikely(!pParam))
		return RTK_FC_HELPER_RET_ERR_NULL_POINTER;

	if (mib_idx >= RT_STAT_EXT_FLOWMIB_TABLE_SIZE)
		return RTK_FC_HELPER_RET_ERR;

	if(pParam->update_type == RTK_FC_COUNTER_UPDATE_HW){//count hw
		if (pParam->is_uplink) {
			if (pParam->update_op == RTK_FC_COUNTER_UPDATE_INCREASE) {
				extFlowMibTbl[mib_idx].counter.in_packet_cnt 	+= pParam->packet_count;	// flow rx
				extFlowMibTbl[mib_idx].counter.in_byte_cnt 	+= pParam->byte_count;
			}
			else {
				extFlowMibTbl[mib_idx].counter.in_packet_cnt 	-= pParam->packet_count;	// flow rx
				extFlowMibTbl[mib_idx].counter.in_byte_cnt 	-= pParam->byte_count;
			}
		}
		else {
			if (pParam->update_op == RTK_FC_COUNTER_UPDATE_INCREASE) {
				extFlowMibTbl[mib_idx].counter.out_packet_cnt += pParam->packet_count;	// flow tx
				extFlowMibTbl[mib_idx].counter.out_byte_cnt 	+= pParam->byte_count;
			}
			else {
				extFlowMibTbl[mib_idx].counter.out_packet_cnt -= pParam->packet_count;	// flow tx
				extFlowMibTbl[mib_idx].counter.out_byte_cnt 	-= pParam->byte_count;
			}
		}
	}
	else if(pParam->update_type == RTK_FC_COUNTER_UPDATE_SW){//count sw
		if (pParam->is_uplink) {
			extFlowMibTbl[mib_idx].sw_counter.in_packet_cnt 	+= pParam->packet_count;	// flow rx
			extFlowMibTbl[mib_idx].sw_counter.in_byte_cnt 	+= pParam->byte_count;
		}
		else{
			extFlowMibTbl[mib_idx].sw_counter.out_packet_cnt += pParam->packet_count;	// flow tx
			extFlowMibTbl[mib_idx].sw_counter.out_byte_cnt 	+= pParam->byte_count;
		}
	}
	else if(pParam->update_type == RTK_FC_COUNTER_UPDATE_KERNEL){//count kernel
		if (pParam->is_uplink) {
			extFlowMibTbl[mib_idx].kernel_counter.in_packet_cnt 	+= pParam->packet_count;	// flow rx
			extFlowMibTbl[mib_idx].kernel_counter.in_byte_cnt 	+= pParam->byte_count;
		}
		else{
			extFlowMibTbl[mib_idx].kernel_counter.out_packet_cnt += pParam->packet_count;	// flow tx
			extFlowMibTbl[mib_idx].kernel_counter.out_byte_cnt 	+= pParam->byte_count;
		}
	}

	return RTK_FC_HELPER_RET_OK;
}

int rtk_fc_update_ext_flow_mib_by_lan_ip(rtk_fc_ext_flow_mib_host_mapping_t *pHost, rtk_fc_ext_flow_mib_param_t *pParam)
{
	int idx;
	rtk_fc_ext_flow_mib_ctrl_t ctrl = fc_mgr_db.extFlowMibControl;

	if (ctrl.mode != RTK_EXT_FLOW_MIB_IPV4_BASED)
		return RTK_FC_HELPER_RET_ERR;

	if (unlikely(!pHost || !pParam || pHost->ip_addr == 0))
		return RTK_FC_HELPER_RET_ERR_NULL_POINTER;

	if ((idx = rtk_fc_get_ext_flow_mib_idx_by_lan_ip(pHost)) != FAIL) {
		// hw flowmib count
		if (pParam->update_type == RTK_FC_COUNTER_UPDATE_HW && idx < RT_STAT_EXT_HW_FLOWMIB_ENTRY_SIZE) 
			return RTK_FC_HELPER_RET_OK;

	#if defined(CONFIG_WFO_OFFLOAD_2G)
		// wfo workqueue counting
		if (ctrl.wlan_2g_sw_count_entry && (idx >= ctrl.wlan_sw_start) && (pHost->is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_WLAN_2G))
			return RTK_FC_HELPER_RET_OK;
	#endif

		return rtk_fc_update_ext_flow_mib_by_idx(idx, pParam);
	}

	return RTK_FC_HELPER_RET_ERR;
}

int rtk_fc_update_ext_flow_mib_by_lan_mac(unsigned char *mac_addr, unsigned char is_wlan_sta, rtk_fc_ext_flow_mib_param_t *pParam)
{
	int idx;
	rtk_fc_ext_flow_mib_ctrl_t ctrl = fc_mgr_db.extFlowMibControl;

	if (ctrl.mode != RTK_EXT_FLOW_MIB_MAC_BASED)
		return RTK_FC_HELPER_RET_ERR;

	if (unlikely(!mac_addr || !pParam))
		return RTK_FC_HELPER_RET_ERR_NULL_POINTER;

	if ((idx = rtk_fc_get_ext_flow_mib_idx_by_lan_mac(mac_addr, is_wlan_sta)) != FAIL) {
		// hw flowmib count
		if (pParam->update_type == RTK_FC_COUNTER_UPDATE_HW && idx < RT_STAT_EXT_HW_FLOWMIB_ENTRY_SIZE) 
			return RTK_FC_HELPER_RET_OK;

	#if defined(CONFIG_WFO_OFFLOAD_2G)
		// wfo workqueue counting
		if (ctrl.wlan_2g_sw_count_entry && (idx >= ctrl.wlan_sw_start) && (is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_WLAN_2G))
			return RTK_FC_HELPER_RET_OK;
	#endif
		return rtk_fc_update_ext_flow_mib_by_idx(idx, pParam);
	}

	return RTK_FC_HELPER_RET_ERR;
}

int rtk_fc_update_dev_to_ext_flow_mib(struct sk_buff *skb, bool is_uplink, char *devname)
{
#if defined(CONFIG_WFO_OFFLOAD_2G)
	rtk_fc_ext_flow_mib_ctrl_t *pctrl = &fc_mgr_db.extFlowMibControl;
	uint8 i, start = pctrl->wlan_sw_start, end = pctrl->wlan_sw_end;
	rtk_fc_ext_flow_mib_entry_t *pextFlowMibTbl = fc_mgr_db.extFlowMibTbl;
	rtk_fc_ext_flow_mib_host_mapping_t *phost;
	uint32 off = (ETH_HLEN - 2), ip_addr;
	uint8 *pmac = NULL;
	int ret = FAIL;

	if (unlikely(!skb || !skb->data))
		return FAIL;

	if (pctrl->need_update_dev) {
		rtk_fc_mgr_traff_cnt_spin_lock_bh();

		for (i = start; i < end; i++) {
			phost = &pextFlowMibTbl[i].host;

			if (phost->is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_WLAN_2G && !phost->dev_flag) {
				if (pctrl->mode == RTK_EXT_FLOW_MIB_IPV4_BASED && phost->ip_addr)
					break;
				else if (pctrl->mode == RTK_EXT_FLOW_MIB_MAC_BASED && NON_ZERO_MAC(phost->mac_addr))
					break;
			}
		}

		rtk_fc_mgr_traff_cnt_spin_unlock_bh();

		if (i == end) {		// all sw 2g have updated dev
			pctrl->need_update_dev = 0;
			return SUCCESS;
		}

		// parse skb
		pmac = is_uplink ? (skb->data + ETH_ALEN) : skb->data;

		if (pctrl->mode == RTK_EXT_FLOW_MIB_IPV4_BASED) {
			if ((*(u16*)(skb->data+off)) == htons(ETH_P_8021Q)) 
				off += VLAN_HLEN;

			if ((*(u16*)(skb->data+off)) == htons(ETH_P_IP)) {
				struct iphdr *iph = (struct iphdr *)((unsigned char *)skb->data+off+2);
				ip_addr = is_uplink ? ntohl(iph->saddr) : ntohl(iph->daddr);
				ret = SUCCESS;
			}
		}
		else {	// mac based
			ret = SUCCESS;
		}

		if (ret != SUCCESS)
			return FAIL;

		rtk_fc_mgr_traff_cnt_spin_lock_bh();
		for (i = start; i < end; i++) {
			phost = &pextFlowMibTbl[i].host;

			if (phost->is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_WLAN_2G && !phost->dev_flag) {
				if (pctrl->mode == RTK_EXT_FLOW_MIB_IPV4_BASED) {
					if (phost->ip_addr && phost->ip_addr == ip_addr) {
						memcpy(phost->mac_addr, pmac, ETH_ALEN);
						phost->dev_flag = 1;
						memcpy(phost->dev_name, devname, IFNAMSIZ);
					}
				}
				else {	// mac based
					if (NON_ZERO_MAC(phost->mac_addr) && ether_addr_equal(phost->mac_addr, pmac)) {
						phost->dev_flag = 1;
						memcpy(phost->dev_name, devname, IFNAMSIZ);
					}
				}
			}
		}
		rtk_fc_mgr_traff_cnt_spin_unlock_bh();
	}
#endif

	return SUCCESS;
}

int rtk_fc_skb_ingress_count_in_ext_flow_mib(struct sk_buff *skb, uint8 is_wlan_sta, bool is_uplink, uint32 *pIdx, struct net_device *wlan_dev)
{
	int ret = FAIL;
	uint8 mode = fc_mgr_db.extFlowMibControl.mode;

	// eth skb is checked in egress
	if (is_wlan_sta) {
		uint32 off = (ETH_HLEN - 2), gw_ip_addr = 0;
		struct net_bridge_port *p;
		rtk_mac_t gw_mac;

		if (unlikely(!skb || !skb->data || !pIdx || !wlan_dev))
			return FAIL;

		if ((p = br_port_get_rcu(wlan_dev)) == NULL)
			goto out;

		if (unlikely(!p->br || !p->br->dev))
			goto out;

#ifdef CONFIG_OPENWRT_SDK
		if (likely(!strncmp(p->br->dev->name, "br-lan", IFNAMSIZ)))
#else
		if (likely(!strncmp(p->br->dev->name, "br0", IFNAMSIZ)))
#endif
		{
			gw_ip_addr = fc_mgr_db.br0_ip;

			gw_mac.octet[0] = fc_mgr_db.br0_mac[0];
			gw_mac.octet[1] = fc_mgr_db.br0_mac[1];
			gw_mac.octet[2] = fc_mgr_db.br0_mac[2];
			gw_mac.octet[3] = fc_mgr_db.br0_mac[3];
			gw_mac.octet[4] = fc_mgr_db.br0_mac[4];
			gw_mac.octet[5] = fc_mgr_db.br0_mac[5];
		}
		else {
			if ((ret = rt_stat_brNetifInfo_get(p->br->dev->name, &gw_mac, &gw_ip_addr)) != SUCCESS)
				goto out;
		}

		ret = FAIL;

		// bridge traffic,  dmac != gw mac
		if (!ether_addr_equal(gw_mac.octet, skb->data))
			goto out;

		if ((*(u16*)(skb->data+off)) == htons(ETH_P_8021Q))
			off += VLAN_HLEN;

		if ((*(u16*)(skb->data+off)) != htons(ETH_P_IP) && (*(u16*)(skb->data+off)) != htons(ETH_P_IPV6))
			goto out;

		if ((*(u16*)(skb->data+off)) == htons(ETH_P_IPV6)) {
			struct ipv6hdr *ip6h = (struct ipv6hdr *)((unsigned char *)(skb->data) + off + 2);

			if (ip6h->nexthdr != NEXTHDR_TCP && ip6h->nexthdr != NEXTHDR_UDP)
				goto out;
		}

		if (mode == RTK_EXT_FLOW_MIB_MAC_BASED) {
			uint8 *mac_addr = is_uplink ? (skb->data + ETH_ALEN) : skb->data;

			rtk_fc_mgr_traff_cnt_spin_lock_bh();

			if ((*pIdx = rtk_fc_get_ext_flow_mib_idx_by_lan_mac(mac_addr, is_wlan_sta)) == FAIL) {
				rtk_fc_mgr_traff_cnt_spin_unlock_bh();
				goto out;
			}

			if (*pIdx < RT_STAT_EXT_HW_FLOWMIB_ENTRY_SIZE) {
				rtk_fc_mgr_traff_cnt_spin_unlock_bh();
				goto out;
			}

			rtk_fc_mgr_traff_cnt_spin_unlock_bh();
			ret = SUCCESS;
		}

		if ((*(u16*)(skb->data+off)) == htons(ETH_P_IP)) {
			struct iphdr *iph = (struct iphdr *)((unsigned char *)(skb->data) + off + 2);
			rtk_fc_ext_flow_mib_host_mapping_t entry;
			uint32 netmask;
			int l3Len, l3Offset, packet_len;

			if (iph->protocol != IPPROTO_TCP && iph->protocol != IPPROTO_UDP) {
				ret = FAIL;
				goto out;
			}

			if (mode == RTK_EXT_FLOW_MIB_IPV4_BASED) {
				entry.is_wlan_sta = is_wlan_sta;
				entry.ip_addr = (is_uplink ? ntohl(iph->saddr) : ntohl(iph->daddr));

				rtk_fc_mgr_traff_cnt_spin_lock_bh();
				if ((*pIdx = rtk_fc_get_ext_flow_mib_idx_by_lan_ip(&entry)) == FAIL) {
					rtk_fc_mgr_traff_cnt_spin_unlock_bh();
					goto out;
				}

				//printk("[%s:%d] ext flowmib index = %d\n", __FUNCTION__, __LINE__, *pIdx);
				if (*pIdx < RT_STAT_EXT_HW_FLOWMIB_ENTRY_SIZE) {
					rtk_fc_mgr_traff_cnt_spin_unlock_bh();
					goto out;
				}

				netmask = fc_mgr_db.extFlowMibTbl[*pIdx].net.netmask;
				rtk_fc_mgr_traff_cnt_spin_unlock_bh();

				if ((ntohl(iph->daddr) & netmask) != (gw_ip_addr & netmask)) {
					ret = SUCCESS;
				}

				if (ret == SUCCESS) {
					// for wifi rx small packets (< 64B), there may be padding in cpu port tx
					l3Len = ntohs(iph->tot_len);
					l3Offset = (uint8 *)(iph) - skb->data;
					packet_len = l3Offset + l3Len;

					// printk("[%s:%d] trim skb len from %d to %d\n", __FUNCTION__, __LINE__, skb->len, packet_len);
					rtk_fc_skb_skb_trim(skb, packet_len);
				}
			}
		}
	}

out:
	return ret;
}

int rtk_fc_skb_data_ingress_count_in_ext_flow_mib(unsigned char *data, uint8 is_wlan_sta, bool is_uplink, uint32 *pIdx, struct net_device *wlan_dev)
{
	int ret = FAIL;
	uint8 mode = fc_mgr_db.extFlowMibControl.mode;

	// eth skb is checked in egress
	if (is_wlan_sta) {
		uint32 off = (ETH_HLEN - 2), gw_ip_addr = 0;
		struct net_bridge_port *p;
		rtk_mac_t gw_mac;

		if (unlikely(!data || !pIdx || !wlan_dev))
			return FAIL;

		if ((p = br_port_get_rcu(wlan_dev)) == NULL)
			goto out;

		if (unlikely(!p->br || !p->br->dev))
			goto out;

#ifdef CONFIG_OPENWRT_SDK
		if (likely(!strncmp(p->br->dev->name, "br-lan", IFNAMSIZ))) 
#else
		if (likely(!strncmp(p->br->dev->name, "br0", IFNAMSIZ))) 
#endif
		{
			gw_ip_addr = fc_mgr_db.br0_ip;

			gw_mac.octet[0] = fc_mgr_db.br0_mac[0];
			gw_mac.octet[1] = fc_mgr_db.br0_mac[1];
			gw_mac.octet[2] = fc_mgr_db.br0_mac[2];
			gw_mac.octet[3] = fc_mgr_db.br0_mac[3];
			gw_mac.octet[4] = fc_mgr_db.br0_mac[4];
			gw_mac.octet[5] = fc_mgr_db.br0_mac[5];
		}
		else {
			if ((ret = rt_stat_brNetifInfo_get(p->br->dev->name, &gw_mac, &gw_ip_addr)) != SUCCESS)
				goto out;
		}

		ret = FAIL;

		// bridge traffic,  dmac != gw mac
		if (!ether_addr_equal(gw_mac.octet, data))
			goto out;

		if ((*(u16*)(data+off)) == htons(ETH_P_8021Q))
			off += VLAN_HLEN;

		if ((*(u16*)(data+off)) != htons(ETH_P_IP) && (*(u16*)(data+off)) != htons(ETH_P_IPV6))
			goto out;

		if ((*(u16*)(data+off)) == htons(ETH_P_IPV6)) {
			struct ipv6hdr *ip6h = (struct ipv6hdr *)((unsigned char *)(data + off + 2));

			if (ip6h->nexthdr != NEXTHDR_TCP && ip6h->nexthdr != NEXTHDR_UDP)
				goto out;
		}

		if (mode == RTK_EXT_FLOW_MIB_MAC_BASED) {
			uint8 *mac_addr = is_uplink ? (data + ETH_ALEN) : data;

			rtk_fc_mgr_traff_cnt_spin_lock_bh();

			if ((*pIdx = rtk_fc_get_ext_flow_mib_idx_by_lan_mac(mac_addr, is_wlan_sta)) == FAIL) {
				rtk_fc_mgr_traff_cnt_spin_unlock_bh();
				goto out;
			}

			if (*pIdx < RT_STAT_EXT_HW_FLOWMIB_ENTRY_SIZE) {
				rtk_fc_mgr_traff_cnt_spin_unlock_bh();
				goto out;
			}

			rtk_fc_mgr_traff_cnt_spin_unlock_bh();
			ret = SUCCESS;
		}

		if ((*(u16*)(data+off)) == htons(ETH_P_IP)) {
			struct iphdr *iph = (struct iphdr *)(data + off + 2);
			rtk_fc_ext_flow_mib_host_mapping_t entry;
			uint32 netmask;

			if (iph->protocol != IPPROTO_TCP && iph->protocol != IPPROTO_UDP) {
				ret = FAIL;
				goto out;
			}

			if (mode == RTK_EXT_FLOW_MIB_IPV4_BASED) {
				entry.is_wlan_sta = is_wlan_sta;
				entry.ip_addr = (is_uplink ? ntohl(iph->saddr) : ntohl(iph->daddr));

				rtk_fc_mgr_traff_cnt_spin_lock_bh();
				if ((*pIdx = rtk_fc_get_ext_flow_mib_idx_by_lan_ip(&entry)) == FAIL) {
					rtk_fc_mgr_traff_cnt_spin_unlock_bh();
					goto out;
				}

				//printk("[%s:%d] ext flowmib index = %d\n", __FUNCTION__, __LINE__, *pIdx);
				if (*pIdx < RT_STAT_EXT_HW_FLOWMIB_ENTRY_SIZE) {
					rtk_fc_mgr_traff_cnt_spin_unlock_bh();
					goto out;
				}

				netmask = fc_mgr_db.extFlowMibTbl[*pIdx].net.netmask;
				rtk_fc_mgr_traff_cnt_spin_unlock_bh();

				if ((ntohl(iph->daddr) & netmask) != (gw_ip_addr & netmask)) {
					ret = SUCCESS;
				}
			}
		}
	}

out:
	return ret;
}

int rtk_fc_skb_egress_count_in_ext_flow_mib(struct sk_buff *skb, uint8 is_wlan_sta, bool is_uplink, uint32 *pIdx)
{
	struct iphdr *iph = NULL;
	struct ipv6hdr *ip6h = NULL;
	uint32 netmask;
	rtk_fc_ext_flow_mib_host_mapping_t entry;
	unsigned char *data;
	uint32 off = ETH_HLEN - 2;
	uint32 ppp_off = PPPOE_SES_HLEN - 2;
	uint8 mode = fc_mgr_db.extFlowMibControl.mode;
	
	if (unlikely(!skb || !skb->data || !pIdx))
		return FAIL;

	data = skb_mac_header(skb);

	if (skb->protocol == htons(ETH_P_PPP_SES)) { //ppp
		if ((*(u16*)(data+off)) == htons(ETH_P_8021Q)) {
			off += VLAN_HLEN;
		}

		off += 2; // mac+vlan header is completed here

		if (((*(u16*)(data+off+ppp_off)) == htons(0x0021))) { //pppoe data (IP protocol)
			iph = (struct iphdr *)(data + off + PPPOE_SES_HLEN);
		}
		else if (((*(u16*)(data+off+ppp_off)) == htons(0x0057))) { //pppoe data (IPv6 protocol)
			ip6h = (struct ipv6hdr *)(data + off + PPPOE_SES_HLEN);
		}
		else
			return FAIL;
	}
	else if (skb->protocol == htons(ETH_P_IP)) { //ipv4
		if ((iph = ip_hdr(skb)) == NULL)
			return FAIL;

		if (iph->protocol != IPPROTO_TCP && iph->protocol != IPPROTO_UDP)
			return FAIL;
	}
	else if (skb->protocol == htons(ETH_P_IPV6)) { //ipv6
		if ((ip6h = ipv6_hdr(skb)) == NULL)
			return FAIL;

		if (ip6h->nexthdr != NEXTHDR_TCP && ip6h->nexthdr != NEXTHDR_UDP)
			return FAIL;
	}
	else 
		return FAIL;

	if (iph && mode == RTK_EXT_FLOW_MIB_IPV4_BASED) {
		entry.is_wlan_sta = is_wlan_sta;
		entry.ip_addr = (is_uplink ? skb->stat_ip : ntohl(iph->daddr));

		rtk_fc_mgr_traff_cnt_spin_lock_bh();
		if ((*pIdx = rtk_fc_get_ext_flow_mib_idx_by_lan_ip(&entry)) == FAIL) {
			rtk_fc_mgr_traff_cnt_spin_unlock_bh();
			return FAIL;
		}

		netmask = fc_mgr_db.extFlowMibTbl[*pIdx].net.netmask;
		rtk_fc_mgr_traff_cnt_spin_unlock_bh();

		if (is_uplink) {
			// sip and dip are in the different subnet
			if ((skb->stat_ip & netmask) != (ntohl(iph->daddr) & netmask))
				return SUCCESS;
		}
		else {
			if ((ntohl(iph->saddr) & netmask) != (ntohl(iph->daddr) & netmask))
				return SUCCESS;
		}
	}
	else {
		uint8 *mac_addr = is_uplink ? skb->fcIngressData.ingress_sa : data;

		if (ether_addr_equal(data, skb->fcIngressData.ingress_da) && ether_addr_equal(data+ETH_ALEN, skb->fcIngressData.ingress_sa))
			return FAIL;

		rtk_fc_mgr_traff_cnt_spin_lock_bh();
		if ((*pIdx = rtk_fc_get_ext_flow_mib_idx_by_lan_mac(mac_addr, is_wlan_sta)) == FAIL) {
			rtk_fc_mgr_traff_cnt_spin_unlock_bh();
			return FAIL;
		}
		rtk_fc_mgr_traff_cnt_spin_unlock_bh();

		return SUCCESS;
	}

	return FAIL;
}

int rtk_fc_get_ext_flow_mib_count_by_lan_ip(uint32 ip_addr, uint8 is_wlan_sta, uint32 *rx_bytes, uint32 *tx_bytes, uint32 *rx_pkts, uint32 *tx_pkts, uint8 flag)
{
	rtk_fc_ext_flow_mib_host_mapping_t host;
	rtk_fc_ext_flow_mib_counter_t *pcounter;
	rtk_fc_ext_flow_mib_ctrl_t ctrl = fc_mgr_db.extFlowMibControl;
	int idx, ret = FAIL;

	if (!ctrl.enable || ctrl.mode != RTK_EXT_FLOW_MIB_IPV4_BASED)
		return FAIL;

	if (unlikely(ip_addr == 0 || !rx_bytes || !tx_bytes || !rx_pkts || !tx_pkts))
		return FAIL;

	host.ip_addr = ip_addr;
	host.is_wlan_sta = is_wlan_sta;

	rtk_fc_mgr_traff_cnt_spin_lock();
	if ((idx = rtk_fc_get_ext_flow_mib_idx_by_lan_ip(&host)) == FAIL) {
		goto out;
	}

	if(flag == RTK_FC_COUNTER_UPDATE_HW) {
		if (idx >= 0 && idx < RT_STAT_EXT_HW_FLOWMIB_ENTRY_SIZE) {
			if ((ret = rtk_fc_update_ext_flow_mib_by_hw_flow_mib(idx)) != 0)
				goto out;
		}

		pcounter = &fc_mgr_db.extFlowMibTbl[idx].counter;

		*rx_bytes	= pcounter->in_byte_cnt;
		*rx_pkts	= pcounter->in_packet_cnt;
		*tx_bytes	= pcounter->out_byte_cnt;
		*tx_pkts	= pcounter->out_packet_cnt;

		ret = RTK_FC_HELPER_RET_OK;
	}
	else if(flag == RTK_FC_COUNTER_UPDATE_SW) {
		pcounter = &fc_mgr_db.extFlowMibTbl[idx].sw_counter;

		*rx_bytes	= pcounter->in_byte_cnt;
		*rx_pkts	= pcounter->in_packet_cnt;
		*tx_bytes	= pcounter->out_byte_cnt;
		*tx_pkts	= pcounter->out_packet_cnt;

		ret = RTK_FC_HELPER_RET_OK;
	}
	else if(flag == RTK_FC_COUNTER_UPDATE_KERNEL) {
		pcounter = &fc_mgr_db.extFlowMibTbl[idx].kernel_counter;

		*rx_bytes 	= pcounter->in_byte_cnt;
		*rx_pkts 	= pcounter->in_packet_cnt;
		*tx_bytes 	= pcounter->out_byte_cnt;
		*tx_pkts 	= pcounter->out_packet_cnt;

		ret = RTK_FC_HELPER_RET_OK;
	}
out:
	rtk_fc_mgr_traff_cnt_spin_unlock();
	return ret;
}

int rtk_fc_get_ext_flow_mib_count_by_lan_mac(unsigned char *mac_addr, uint8 is_wlan_sta, uint32 *rx_bytes, uint32 *tx_bytes, uint32 *rx_pkts, uint32 *tx_pkts, uint8 flag)
{
	int idx, ret = FAIL;
	rtk_fc_ext_flow_mib_counter_t *pcounter;
	rtk_fc_ext_flow_mib_ctrl_t ctrl = fc_mgr_db.extFlowMibControl;

	if (!ctrl.enable || ctrl.mode != RTK_EXT_FLOW_MIB_MAC_BASED)
		return FAIL;

	if (unlikely(!mac_addr || !rx_bytes || !tx_bytes || !rx_pkts || !tx_pkts))
		return FAIL;

	rtk_fc_mgr_traff_cnt_spin_lock();
	if ((idx = rtk_fc_get_ext_flow_mib_idx_by_lan_mac(mac_addr, is_wlan_sta)) == FAIL) {
		goto out;
	}

	if(flag == RTK_FC_COUNTER_UPDATE_HW) {
		if (idx >= 0 && idx < RT_STAT_EXT_HW_FLOWMIB_ENTRY_SIZE) {
			if ((ret = rtk_fc_update_ext_flow_mib_by_hw_flow_mib(idx)) != 0)
				goto out;
		}

		pcounter = &fc_mgr_db.extFlowMibTbl[idx].counter;

		*rx_bytes	= pcounter->in_byte_cnt;
		*rx_pkts	= pcounter->in_packet_cnt;
		*tx_bytes	= pcounter->out_byte_cnt;
		*tx_pkts	= pcounter->out_packet_cnt;

		ret = RTK_FC_HELPER_RET_OK;
	}
	else if(flag == RTK_FC_COUNTER_UPDATE_SW) {
		pcounter = &fc_mgr_db.extFlowMibTbl[idx].sw_counter;

		*rx_bytes	= pcounter->in_byte_cnt;
		*rx_pkts	= pcounter->in_packet_cnt;
		*tx_bytes	= pcounter->out_byte_cnt;
		*tx_pkts	= pcounter->out_packet_cnt;

		ret = RTK_FC_HELPER_RET_OK;
	}
	else if(flag == RTK_FC_COUNTER_UPDATE_KERNEL) {
		pcounter = &fc_mgr_db.extFlowMibTbl[idx].kernel_counter;

		*rx_bytes 	= pcounter->in_byte_cnt;
		*rx_pkts 	= pcounter->in_packet_cnt;
		*tx_bytes 	= pcounter->out_byte_cnt;
		*tx_pkts 	= pcounter->out_packet_cnt;

		ret = RTK_FC_HELPER_RET_OK;
	}
out:
	rtk_fc_mgr_traff_cnt_spin_unlock();
	return ret;
}

int rtk_fc_delete_ext_flow_mib_count_and_flow_by_lan_ip(rtk_fc_ext_flow_mib_host_mapping_t *pHost)
{
	int i, ret = RTK_FC_HELPER_RET_ERR;
	rtk_fc_ext_flow_mib_ctrl_t *pctrl = &fc_mgr_db.extFlowMibControl;
	rt_flow_ops_data_t flow_ops_data;
#if defined(CONFIG_WFO_OFFLOAD_2G)
	int wq_stop = 0;
#endif

	if (pctrl->mode != RTK_EXT_FLOW_MIB_IPV4_BASED)
		return FAIL;

	if (unlikely(!pHost || pHost->ip_addr == 0))
		return FAIL;

	rtk_fc_mgr_traff_cnt_spin_lock_bh();

	if ((i = rtk_fc_get_ext_flow_mib_idx_by_lan_ip(pHost)) != FAIL) {
		memset(&fc_mgr_db.extFlowMibTbl[i], 0, sizeof(rtk_fc_ext_flow_mib_entry_t));
		ret = RTK_FC_HELPER_RET_OK;
	}

	if (i >= 0 && i < RT_STAT_EXT_HW_FLOWMIB_ENTRY_SIZE) {
		rt_stat_flowMib_reset(RT_STAT_UP_HW_FLOW_MIB_MAPPED_FROM_EXT_FLOW_MIB(i));
		rt_stat_flowMib_reset(RT_STAT_DOWN_HW_FLOW_MIB_MAPPED_FROM_EXT_FLOW_MIB(i));
	}

	if (i >= pctrl->wlan_sw_start && i < pctrl->wlan_sw_end) {
		if (pHost->is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_WLAN_5G) {
			pctrl->wlan_5g_sw_count_entry--;
		}
		else {
			pctrl->wlan_2g_sw_count_entry--;
		#if defined(CONFIG_WFO_OFFLOAD_2G)
			wq_stop = (pctrl->wlan_2g_sw_count_entry == 0 ? 1 : 0);
		#endif
		}
	}

	rtk_fc_mgr_traff_cnt_spin_unlock_bh();

	memset(&flow_ops_data, 0, sizeof(flow_ops_data));

	flow_ops_data.data_delFlow.flowKeyType = RT_FLOW_OP_FLOW_KEY_TYPE_PATTERN;
	flow_ops_data.data_delFlow.flowKey.flowPattern.sip4_valid = 1U;
	flow_ops_data.data_delFlow.flowKey.flowPattern.sip4 = pHost->ip_addr;
	rt_flow_operation_add(RT_FLOW_OPS_DELETE_FLOW, &flow_ops_data);

	flow_ops_data.data_delFlow.flowKey.flowPattern.sip4_valid = 0U;
	flow_ops_data.data_delFlow.flowKey.flowPattern.sip4 = 0U;
	flow_ops_data.data_delFlow.flowKey.flowPattern.dip4_valid = 1U;
	flow_ops_data.data_delFlow.flowKey.flowPattern.dip4 = pHost->ip_addr;
	rt_flow_operation_add(RT_FLOW_OPS_DELETE_FLOW, &flow_ops_data);

#if defined(CONFIG_WFO_OFFLOAD_2G)
	if (wq_stop) {
		rtk_fc_wfo_wq_stop();
	}
#endif

	return ret;
}

int rtk_fc_delete_ext_flow_mib_count_and_flow_by_lan_mac(unsigned char *mac_addr, unsigned char is_wlan_sta)
{
	int i, ret = RTK_FC_HELPER_RET_ERR;
	rtk_fc_ext_flow_mib_ctrl_t *pctrl = &fc_mgr_db.extFlowMibControl;
	rt_flow_ops_data_t flow_ops_data;

#if defined(CONFIG_WFO_OFFLOAD_2G)
	int wq_stop = 0;
#endif

	if (pctrl->mode != RTK_EXT_FLOW_MIB_MAC_BASED)
		return FAIL;

	if (unlikely(!mac_addr))
		return FAIL;

	rtk_fc_mgr_traff_cnt_spin_lock_bh();

	if ((i = rtk_fc_get_ext_flow_mib_idx_by_lan_mac(mac_addr, is_wlan_sta)) != FAIL) {
		memset(&fc_mgr_db.extFlowMibTbl[i], 0, sizeof(rtk_fc_ext_flow_mib_entry_t));
		ret = RTK_FC_HELPER_RET_OK;
	}

	if (i >= 0 && i < RT_STAT_EXT_HW_FLOWMIB_ENTRY_SIZE) {
		rt_stat_flowMib_reset(RT_STAT_UP_HW_FLOW_MIB_MAPPED_FROM_EXT_FLOW_MIB(i));
		rt_stat_flowMib_reset(RT_STAT_DOWN_HW_FLOW_MIB_MAPPED_FROM_EXT_FLOW_MIB(i));
	}

	if (i >= pctrl->wlan_sw_start && i < pctrl->wlan_sw_end) {
		if (is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_WLAN_5G) {
			pctrl->wlan_5g_sw_count_entry--;
		}
		else {
			pctrl->wlan_2g_sw_count_entry--;
		#if defined(CONFIG_WFO_OFFLOAD_2G)
			wq_stop = (pctrl->wlan_2g_sw_count_entry == 0 ? 1 : 0);
		#endif
		}
	}

	rtk_fc_mgr_traff_cnt_spin_unlock_bh();

	memset(&flow_ops_data, 0, sizeof(flow_ops_data));

	flow_ops_data.data_delFlow.flowKeyType = RT_FLOW_OP_FLOW_KEY_TYPE_PATTERN;
	flow_ops_data.data_delFlow.flowKey.flowPattern.mac_valid = 1U;
	memcpy(flow_ops_data.data_delFlow.flowKey.flowPattern.mac, mac_addr, ETH_ALEN);

	rt_flow_operation_add(RT_FLOW_OPS_DELETE_FLOW, &flow_ops_data); 

#if defined(CONFIG_WFO_OFFLOAD_2G)
	if (wq_stop) {
		rtk_fc_wfo_wq_stop();
	}
#endif

	return ret;
}

void rtk_fc_knl_pkt_count(struct sk_buff *skb)
{
	bool from_wan = false, from_lan = false;
	bool to_wan = false, to_lan = false;
	bool is_uplink;
	int8 is_wlan_sta = 0;
	struct net_device *wlan_dev = NULL;
	uint32 ext_flowmib_idx;
	rtk_fc_ext_flow_mib_ctrl_t ctrl = fc_mgr_db.extFlowMibControl;
	rtk_fc_ext_flow_mib_param_t param;

	if (!skb || (skb->data[0] & 1) || !skb->from_dev || !skb->dev)
		return;

	if (ctrl.enable) {
	#ifdef RTK_NETDEV_PRIV_FLAGS
		to_wan = ((rtk_netdev_get_flags(skb->dev) & RTK_IFF_DOMAIN_WAN)
		#ifdef CONFIG_RTL_MULTI_ETH_WAN
			&& !(rtk_netdev_get_flags(skb->dev) & RTK_IFF_VSMUX)
		#endif
		);

		to_lan = (((rtk_netdev_get_flags(skb->dev) & RTK_IFF_DOMAIN_ELAN)
		#ifdef CONFIG_RTL_MULTI_ETH_WAN
			&& !(rtk_netdev_get_flags(skb->dev) & RTK_IFF_VSMUX)
		#endif
		) || (rtk_netdev_get_flags(skb->dev) & RTK_IFF_DOMAIN_WLAN));

		from_wan = (rtk_netdev_get_flags(skb->from_dev) & RTK_IFF_DOMAIN_WAN);
		from_lan = ((rtk_netdev_get_flags(skb->from_dev) & RTK_IFF_DOMAIN_ELAN) || (rtk_netdev_get_flags(skb->from_dev) & RTK_IFF_DOMAIN_WLAN));

		if ((rtk_netdev_get_flags(skb->dev) & RTK_IFF_DOMAIN_WLAN) || (rtk_netdev_get_flags(skb->from_dev) & RTK_IFF_DOMAIN_WLAN)) {
			is_wlan_sta = 1;
		}
	#else
		to_wan = ((skb->dev->priv_flags & IFF_DOMAIN_WAN)
		#ifdef CONFIG_RTL_MULTI_ETH_WAN
			&& !(skb->dev->priv_flags & IFF_VSMUX)
		#endif
		);

		to_lan = (((skb->dev->priv_flags & IFF_DOMAIN_ELAN) 
		#ifdef CONFIG_RTL_MULTI_ETH_WAN
			&& !(skb->dev->priv_flags & IFF_VSMUX)
		#endif
			) || (skb->dev->priv_flags & IFF_DOMAIN_WLAN));

		from_wan = (skb->from_dev->priv_flags & IFF_DOMAIN_WAN);
		from_lan = ((skb->from_dev->priv_flags & IFF_DOMAIN_ELAN) || (skb->from_dev->priv_flags & IFF_DOMAIN_WLAN));

		if ((skb->dev->priv_flags & IFF_DOMAIN_WLAN) || (skb->from_dev->priv_flags & IFF_DOMAIN_WLAN)) {
			is_wlan_sta = 1;
		}
	#endif

		if (from_lan && to_wan) {
			is_uplink = true;
		}
		else if (from_wan && to_lan) {
			is_uplink = false;
		}
		else {
			return;
		}

		if (is_wlan_sta) {
			wlan_dev = is_uplink ? skb->from_dev : skb->dev;
			if ((is_wlan_sta = rtk_fc_is_wlan_5g(wlan_dev->name)) == FAIL)
				return;
		}

		if (rtk_fc_skb_egress_count_in_ext_flow_mib(skb, is_wlan_sta, is_uplink, &ext_flowmib_idx) == SUCCESS) {
		#if defined(CONFIG_WFO_OFFLOAD_2G)
			// wfo workqueue counting
			if (ctrl.wlan_2g_sw_count_entry && (ext_flowmib_idx >= ctrl.wlan_sw_start) && (is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_WLAN_2G))
				return;
		#endif

			//update kernel count
			param.packet_count	= 1;
			param.byte_count	= skb->len + 4; 	// CRC
			param.is_uplink 	= is_uplink;
			param.update_type	= RTK_FC_COUNTER_UPDATE_KERNEL;

			rtk_fc_mgr_traff_cnt_spin_lock_bh();
			rtk_fc_update_ext_flow_mib_by_idx(ext_flowmib_idx,&param);
			rtk_fc_mgr_traff_cnt_spin_unlock_bh();
		}
	}

	return;
}

void rtk_fc_ext_count_wifi_rx_to_nic_tx_inc(uint8 amsdu, struct net_device	*dev, struct sk_buff *skb, unsigned char *data, uint32 len)
{
	int8 is_wlan_sta = FAIL;
	uint32 ext_flowmib_idx;
	int ret;
	rtk_fc_ext_flow_mib_ctrl_t *pctrl = &fc_mgr_db.extFlowMibControl;

	// skb may be NULL for amsdu case
	if (unlikely(!data || !dev))
		return;

	if ((data[0] & 1) || !pctrl->enable || (!pctrl->wlan_5g_sw_count_entry && !pctrl->wlan_2g_sw_count_entry))
		return;

	if ((is_wlan_sta = rtk_fc_is_wlan_5g(dev->name)) == FAIL)
		return;

#if defined(CONFIG_WFO_OFFLOAD_2G)
	if (!pctrl->wlan_5g_sw_count_entry || is_wlan_sta != RTK_EXT_FLOW_MIB_LAN_WLAN_5G)	// only 5G do sw count in MIPs
		return;
#else
	if (is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_WLAN_5G && !pctrl->wlan_5g_sw_count_entry)
		return;
	else if (is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_WLAN_2G && !pctrl->wlan_2g_sw_count_entry)
		return;
#endif

	if (amsdu) {
		ret = rtk_fc_skb_data_ingress_count_in_ext_flow_mib(data, is_wlan_sta, TRUE, &ext_flowmib_idx, dev);
	}
	else {
		ret = rtk_fc_skb_ingress_count_in_ext_flow_mib(skb, is_wlan_sta, TRUE, &ext_flowmib_idx, dev);
	}

	if (ret == SUCCESS) {
		rtk_fc_ext_flow_mib_param_t param;

		param.packet_count 	= 1;
		param.byte_count	= (amsdu ? (len + 4) : (skb->len + 4));		// CRC
		param.is_uplink 	= TRUE;
		param.update_op		= RTK_FC_COUNTER_UPDATE_INCREASE;
		param.update_type	= RTK_FC_COUNTER_UPDATE_HW;

		rtk_fc_mgr_traff_cnt_spin_lock_bh();
		rtk_fc_update_ext_flow_mib_by_idx(ext_flowmib_idx, &param);
		rtk_fc_mgr_traff_cnt_spin_unlock_bh();

		if (amsdu) {
			RTK_FC_EXT_FLOW_MIB_DEBUG(wlan_to_nic_amsdu_inc);
		}
		else {
			RTK_FC_EXT_FLOW_MIB_DEBUG(wlan_to_nic_inc);
		}
	}

	return;
}

void rtk_fc_ext_count_wifi_rx_to_nic_tx_dec(struct sk_buff *skb, rtk_fc_mac_port_idx_t spa, rtk_fc_mac_ext_port_idx_t extspa)
{
	rtk_fc_ext_flow_mib_ctrl_t *pctrl = &fc_mgr_db.extFlowMibControl;
	unsigned char dev_name[IFNAMSIZ] = {0};
	int8 is_wlan_sta;
	int ret;
	rtk_fc_mgr_l2_info_t l2info = {0};
	struct net_device* wlan_dev;
	uint32 ext_flowmib_idx;

	if (unlikely(!skb))
		return;

	if ((skb->data[0] & 1) || !pctrl->enable || (!pctrl->wlan_5g_sw_count_entry && !pctrl->wlan_2g_sw_count_entry))
		return;

	if (((1<<spa) & RTK_FC_ALL_MAC_WLANCPU_PORTMASK) && (extspa != RTK_FC_MAC_EXT_CPU)) {
		if ((ret = rtk_fc_port2wlanDevName(spa, extspa, dev_name)) < 0)
			return;

		if (ret == 1) {	// N-to-1
			if (rtk_fc_l2Info_get(skb->data + ETH_ALEN, &l2info) == SUCCESS) {		// by sa
				if (l2info.wlan_dev_idx < RTK_FC_WLANX_END_INTF) {
					rtk_fc_wlanDevIdx2devName(l2info.wlan_dev_idx, dev_name);
				}
			}
		}

		if (dev_name[0] == '\0')
			return;

		if ((is_wlan_sta = rtk_fc_is_wlan_5g(dev_name)) == FAIL)
			return;

	#if defined(CONFIG_WFO_OFFLOAD_2G)
		if (!pctrl->wlan_5g_sw_count_entry || is_wlan_sta != RTK_EXT_FLOW_MIB_LAN_WLAN_5G)	// only 5G do sw count in MIPs
			goto wfo;
	#else
		if (is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_WLAN_5G && !pctrl->wlan_5g_sw_count_entry)
			return;
		else if (is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_WLAN_2G && !pctrl->wlan_2g_sw_count_entry)
			return;
	#endif

		if ((wlan_dev =  __dev_get_by_name(&init_net, dev_name)) != NULL) {
			if (rtk_fc_skb_ingress_count_in_ext_flow_mib(skb, is_wlan_sta, TRUE, &ext_flowmib_idx, wlan_dev) == SUCCESS) {
				rtk_fc_ext_flow_mib_param_t param;

				param.packet_count 	= 1;
				param.byte_count	= skb->len + 4;		// CRC
				param.is_uplink 	= TRUE;
				param.update_op		= RTK_FC_COUNTER_UPDATE_DECREASE;
				param.update_type	= RTK_FC_COUNTER_UPDATE_HW;

				rtk_fc_mgr_traff_cnt_spin_lock_bh();
				rtk_fc_update_ext_flow_mib_by_idx(ext_flowmib_idx, &param);
				rtk_fc_mgr_traff_cnt_spin_unlock_bh();

				RTK_FC_EXT_FLOW_MIB_DEBUG(wlan_to_nic_dec);
			}
		}

#if defined(CONFIG_WFO_OFFLOAD_2G)
wfo:
		// store sta mac and wlan devname into fc_mgr_db.extFlowMibTbl
		rtk_fc_update_dev_to_ext_flow_mib(skb, TRUE, dev_name);
#endif
	}

	return;
}

void rtk_fc_ext_count_nic_rx_to_wifi_tx_inc(struct sk_buff *skb, rtk_fc_mac_port_idx_t spa, uint8 l3routing, rtk_fc_wlan_devidx_t wlandevid)
{
	rtk_fc_ext_flow_mib_ctrl_t *pctrl = &fc_mgr_db.extFlowMibControl;
	unsigned char dev_name[IFNAMSIZ] = {0};
	int8 is_wlan_sta;
	rtk_fc_ext_flow_mib_host_mapping_t host;
	rtk_fc_ext_flow_mib_param_t param;

	if (unlikely(!skb || wlandevid >= RTK_FC_WLANX_END_INTF))
		return;

	if ((skb->data[0] & 1) || !l3routing || !pctrl->enable || (!pctrl->wlan_5g_sw_count_entry && !pctrl->wlan_2g_sw_count_entry))
		return;

	if (!((1 << spa) & fc_mgr_db.wanPortMask.portmask))
		return;

	if (rtk_fc_wlanDevIdx2devName(wlandevid, dev_name) != SUCCESS) 
		return;

	if ((is_wlan_sta = rtk_fc_is_wlan_5g(dev_name)) == FAIL)
		return;

#if defined(CONFIG_WFO_OFFLOAD_2G)
	if (!pctrl->wlan_5g_sw_count_entry || is_wlan_sta != RTK_EXT_FLOW_MIB_LAN_WLAN_5G)	// only 5G do sw count in MIPs
		return;
#else
	if (is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_WLAN_5G && !pctrl->wlan_5g_sw_count_entry)
		return;
	else if (is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_WLAN_2G && !pctrl->wlan_2g_sw_count_entry)
		return;
#endif

	param.packet_count 	= 1;
	param.byte_count	= skb->len + 4;		// CRC
	param.is_uplink 	= FALSE;
	param.update_op		= RTK_FC_COUNTER_UPDATE_INCREASE;
	param.update_type	= RTK_FC_COUNTER_UPDATE_HW;

	if (pctrl->mode == RTK_EXT_FLOW_MIB_IPV4_BASED) {
		host.is_wlan_sta		= is_wlan_sta;
		if ((host.ip_addr = rtk_fc_get_local_ip_addr_by_skb(skb, FALSE)) != FAIL) {
			rtk_fc_mgr_traff_cnt_spin_lock_bh();
			if (!rtk_fc_update_ext_flow_mib_by_lan_ip(&host, &param)) {
				RTK_FC_EXT_FLOW_MIB_DEBUG(nic_to_wlan_inc);
			}
			rtk_fc_mgr_traff_cnt_spin_unlock_bh();
		}
	}
	else {	// mac based
		rtk_fc_mgr_traff_cnt_spin_lock_bh();
		if (!rtk_fc_update_ext_flow_mib_by_lan_mac(skb->data, is_wlan_sta, &param)) {
			RTK_FC_EXT_FLOW_MIB_DEBUG(nic_to_wlan_inc);
		}
		rtk_fc_mgr_traff_cnt_spin_unlock_bh();
	}

	return;
}

void rtk_fc_ext_count_nic_rx_to_wifi_tx_agg_inc(struct sk_buff *skb, uint8 l3routing, uint8 spa, uint8 gmac, uint32 dst_pmsk)
{
	rtk_fc_ext_flow_mib_ctrl_t *pctrl = &fc_mgr_db.extFlowMibControl;
	unsigned char dev_name[IFNAMSIZ] = {0};
	int8 is_wlan_sta;
	rtk_fc_ext_flow_mib_host_mapping_t host;
	rtk_fc_ext_flow_mib_param_t param;
	rtk_fc_wlan_devidx_t wlandevid;

	if (unlikely(!skb))
		return;

	if ((skb->data[0] & 1) || !l3routing || !pctrl->enable || (!pctrl->wlan_5g_sw_count_entry && !pctrl->wlan_2g_sw_count_entry))
		return;

	if (!((1 << spa) & fc_mgr_db.wanPortMask.portmask))
		return;

	wlandevid = fc_mgr_db.wlan_port_to_devidx[gmac][__ffs(dst_pmsk)];

	if (wlandevid == RTK_FC_WLANX_MULTI_INTF) {
		rtk_fc_mgr_l2_info_t l2info = {0};

		if (rtk_fc_l2Info_get(skb->data, &l2info) == SUCCESS) { 	// by da
			if (l2info.wlan_dev_idx < RTK_FC_WLANX_END_INTF)
				wlandevid = l2info.wlan_dev_idx;
		}
	}

	if (rtk_fc_wlanDevIdx2devName(wlandevid, dev_name) != SUCCESS) 
		return;

	if ((is_wlan_sta = rtk_fc_is_wlan_5g(dev_name)) == FAIL)
		return;

#if defined(CONFIG_WFO_OFFLOAD_2G)
	if (!pctrl->wlan_5g_sw_count_entry || is_wlan_sta != RTK_EXT_FLOW_MIB_LAN_WLAN_5G)	// only 5G do sw count in MIPs
		return;
#else
	if (is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_WLAN_5G && !pctrl->wlan_5g_sw_count_entry)
		return;
	else if (is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_WLAN_2G && !pctrl->wlan_2g_sw_count_entry)
		return;
#endif

	param.packet_count 	= 1;
	param.byte_count	= skb->len + 4;		// CRC
	param.is_uplink 	= FALSE;
	param.update_op		= RTK_FC_COUNTER_UPDATE_INCREASE;
	param.update_type	= RTK_FC_COUNTER_UPDATE_HW;

	if (pctrl->mode == RTK_EXT_FLOW_MIB_IPV4_BASED) {
		host.is_wlan_sta		= is_wlan_sta;
		if ((host.ip_addr = rtk_fc_get_local_ip_addr_by_skb(skb, FALSE)) != FAIL) {
			rtk_fc_mgr_traff_cnt_spin_lock_bh();
			if (!rtk_fc_update_ext_flow_mib_by_lan_ip(&host, &param)) {
				RTK_FC_EXT_FLOW_MIB_DEBUG(nic_to_wlan_agg_inc);
			}
			rtk_fc_mgr_traff_cnt_spin_unlock_bh();
		}
	}
	else {	// mac based
		rtk_fc_mgr_traff_cnt_spin_lock_bh();
		if (!rtk_fc_update_ext_flow_mib_by_lan_mac(skb->data, is_wlan_sta, &param)) {
			RTK_FC_EXT_FLOW_MIB_DEBUG(nic_to_wlan_agg_inc);
		}
		rtk_fc_mgr_traff_cnt_spin_unlock_bh();
	}

	return;
}

#if defined(CONFIG_WFO_OFFLOAD_2G)
void rtk_fc_wfo_wq_sched(struct work_struct *work)
{
	rtk_fc_ext_flow_mib_ctrl_t *pctrl = &fc_mgr_db.extFlowMibControl;
	int ret;

    RTK_FC_EXT_FLOW_MIB_WFO_DEBUG(wfo_wq_sched);

	if (!rtk_fc_wfo_deliver_sta_status) {
		RTK_FC_EXT_FLOW_MIB_WFO_DEBUG(wfo_func_null);
		goto end;
	}

	if (pctrl->enable && pctrl->wlan_2g_sw_count_entry) {
		int sta_size = sizeof(struct wfo_sta_st_s) * RTK_WFO_IFACE_NUM * RTK_WFO_STA_NUM;
		struct wfo_sta_st_s *psta_st = kzalloc(sta_size, GFP_KERNEL);

		RTK_FC_EXT_FLOW_MIB_WFO_DEBUG(try_update);

		if (psta_st) {
			int i, m, n, offset;
			int sw_count = pctrl->wlan_2g_sw_count_entry;
			struct wfo_sta_st_s *entry;
			const char **pname = rtk_fc_wfo_wlanmap;

			rtk_fc_wfo_deliver_sta_status(psta_st, sta_size);

			rtk_fc_mgr_traff_cnt_spin_lock_bh();

			for (i = pctrl->wlan_sw_start; i < pctrl->wlan_sw_end; i++) {
				rtk_fc_ext_flow_mib_host_mapping_t *phost = &fc_mgr_db.extFlowMibTbl[i].host;
				rtk_fc_ext_flow_mib_counter_t *pcounter = &fc_mgr_db.extFlowMibTbl[i].counter;

				if (phost->is_wlan_sta == RTK_EXT_FLOW_MIB_LAN_WLAN_2G) {
					if (!phost->dev_flag || phost->dev_name[0] == '\0') {
						RTK_FC_EXT_FLOW_MIB_WFO_DEBUG(wfo_sta_dev_null);
						goto break_check;
					}

					for (m = 0; m < RTK_WFO_IFACE_NUM; m++) {
						if (!strncmp(phost->dev_name, pname[m], IFNAMSIZ))
							break;
					}

					if (m == RTK_WFO_IFACE_NUM) {
						RTK_FC_EXT_FLOW_MIB_WFO_DEBUG(get_wfo_dev_fail);
						goto break_check;
					}

					for (n = 0; n < RTK_WFO_STA_NUM; n++) {
						offset = (m * RTK_WFO_STA_NUM) + n;
						entry = psta_st + offset;

						if (entry && NON_ZERO_MAC(entry->mac_addr) && ether_addr_equal(entry->mac_addr, phost->mac_addr)) {
							RTK_FC_EXT_FLOW_MIB_WFO_DEBUG(update_by_wfo);

							pcounter->in_byte_cnt		= (u32)entry->rx_bytes;
							pcounter->in_packet_cnt 	= (u32)entry->rx_pkts;
							pcounter->out_byte_cnt 		= (u32)entry->tx_bytes;
							pcounter->out_packet_cnt	= (u32)entry->tx_pkts;
							break;
						}
					}

					if (n == RTK_WFO_STA_NUM) {
						RTK_FC_EXT_FLOW_MIB_WFO_DEBUG(update_miss);
					}

break_check:
					if (--sw_count == 0)
						break;
				}
			}

			rtk_fc_mgr_traff_cnt_spin_unlock_bh();
			kfree(psta_st);
		}
		else {
			RTK_FC_EXT_FLOW_MIB_WFO_DEBUG(kzalloc_fail);
		}
	}

end:
	if (!work_pending(work)) {
		ret = schedule_delayed_work_on(pctrl->wfo_wq_bind_cpu, &pctrl->wfo_w, (pctrl->wfo_wq_sleep_msec * CONFIG_HZ / 1000));

		if (ret == false)
			RTK_FC_EXT_FLOW_MIB_WFO_DEBUG(wfo_wq_start_err);
		else
			RTK_FC_EXT_FLOW_MIB_WFO_DEBUG(wfo_wq_start_ok);
	}

	return;
}

int rtk_fc_wfo_wq_start(void)
{
	rtk_fc_ext_flow_mib_ctrl_t *pctrl = &fc_mgr_db.extFlowMibControl;
	int ret;

	if (!pctrl->wfo_wq_running) {
		INIT_DELAYED_WORK(&pctrl->wfo_w, rtk_fc_wfo_wq_sched);
		ret = schedule_delayed_work_on(pctrl->wfo_wq_bind_cpu, &pctrl->wfo_w, (pctrl->wfo_wq_sleep_msec * CONFIG_HZ / 1000));

		if (ret == false) {
			RTK_FC_EXT_FLOW_MIB_WFO_DEBUG(wfo_wq_start_err);
		}
		else {
			RTK_FC_EXT_FLOW_MIB_WFO_DEBUG(wfo_wq_start_ok);
			pctrl->wfo_wq_running = 1;
		}
	}
	else {
		RTK_FC_EXT_FLOW_MIB_WFO_DEBUG(wfo_wq_start_err);
	}

	return 0;
}

int rtk_fc_wfo_wq_stop(void)
{
	rtk_fc_ext_flow_mib_ctrl_t *pctrl = &fc_mgr_db.extFlowMibControl;

	if (pctrl->wfo_wq_running) {
		cancel_delayed_work_sync(&pctrl->wfo_w);
		pctrl->wfo_wq_running = 0;
		RTK_FC_EXT_FLOW_MIB_WFO_DEBUG(wfo_wq_end_ok);
	}
	else {
		RTK_FC_EXT_FLOW_MIB_WFO_DEBUG(wfo_wq_end_err);
	}

	return 0;
}
#endif

int rtk_fc_l2_wlandev_get(unsigned char *mac, struct net_device **wlandev)
{
	rtk_fc_mgr_l2_info_t l2info;
	int ret = FAIL;

	if (unlikely(!mac || !wlandev))
		return ret;

	if (rtk_fc_l2Info_get(mac, &l2info) == SUCCESS) {
		rtk_fc_mgr_global_spin_lock_bh();
		ret = rtk_fc_wlanDevIdx2dev(l2info.wlan_dev_idx, wlandev);
		rtk_fc_mgr_global_spin_unlock_bh();
	}

	return ret;
}
EXPORT_SYMBOL(rtk_fc_l2_wlandev_get);
#endif


#if defined(CONFIG_RTL8198XB_SERIES)
uint8 rtk_fc_get_ext_flow_mib_control(void)
{
	return fc_mgr_db.extFlowMibControl.enable;
}

uint8 rtk_fc_set_ext_flow_mib_control(uint8 value)
{
	if (value != fc_mgr_db.extFlowMibControl.enable) {
		fc_mgr_db.extFlowMibControl.enable = value;
	}

	return SUCCESS;
}

int rtk_fc_print_ext_flow_mib_count_all(void)
{
	uint32 i;
	int ret;
	rtk_fc_ext_flow_mib_ctrl_t ctrl = fc_mgr_db.extFlowMibControl;
	rt_rate_host_policer_mib_t policer_mib;

	printk("Extend flow mib enable: %d\n", ctrl.enable);

	if (ctrl.enable) {
		rtk_fc_mgr_traff_cnt_spin_lock_bh();

		for (i = 0U; i < RT_STAT_EXT_FLOWMIB_TABLE_SIZE; i++) {
			if (fc_mgr_db.extFlowMibTbl[i].valid) {
				printk("[%d] mac_addr: %pM\n", i, &fc_mgr_db.extFlowMibTbl[i].mac_addr);

				if ((ret = rt_rate_hostPolicerMib_get(i, &policer_mib)) == RT_ERR_OK) {
					printk("\t   rx_packet: %u, rx_byte: %u\n", policer_mib.rx_pkt, (uint32)policer_mib.rx_count);
					printk("\t   tx_packet: %u, tx_byte: %u\n", policer_mib.tx_pkt, (uint32)policer_mib.tx_count);
				}
			}
		}

		rtk_fc_mgr_traff_cnt_spin_unlock_bh();
	}

	return RTK_FC_HELPER_RET_OK;
}

int rtk_fc_add_ext_flow_mib_count_by_lan_mac(unsigned char *mac_addr)
{
	int i, ret;
	rtk_fc_ext_flow_mib_entry_t *pTable = fc_mgr_db.extFlowMibTbl;

	if (unlikely(!mac_addr)) {
		return RTK_FC_HELPER_RET_ERR_NULL_POINTER;
	}

	rtk_fc_mgr_traff_cnt_spin_lock_bh();

	for (i = 0; i < RT_STAT_EXT_FLOWMIB_TABLE_SIZE; i++) {
		if (pTable[i].valid && !memcmp(pTable[i].mac_addr, mac_addr, ETH_ALEN)) {
			printk("[exist]\n");
			ret = RTK_FC_HELPER_RET_OK;
			goto out;
		}
	}

	for (i = 0; i < RT_STAT_EXT_FLOWMIB_TABLE_SIZE; i++) {
		if (!pTable[i].valid) {
			pTable[i].valid = 1U;
			memcpy(pTable[i].mac_addr, mac_addr, ETH_ALEN);
			ret = RTK_FC_HELPER_RET_OK_NEW;
			goto out;
		}
	}

	ret = RTK_FC_HELPER_RET_ERR_TBL_FULL;

out:
	rtk_fc_mgr_traff_cnt_spin_unlock_bh();

	if (ret == RTK_FC_HELPER_RET_OK_NEW) {
		rt_rate_host_policer_control_t host_police_entry;

		memset(&host_police_entry, 0, sizeof(host_police_entry));

		host_police_entry.mib_count_control = ENABLED;
		memcpy(host_police_entry.mac_addr, mac_addr, ETH_ALEN);

		ret = rt_rate_hostPolicerControl_set(i, host_police_entry);
	}

	return ret;
}

int rtk_fc_delete_ext_flow_mib_count_by_lan_mac(unsigned char *mac_addr)
{
	int i, ret = RTK_FC_HELPER_RET_ERR;
	rtk_fc_ext_flow_mib_entry_t *pTable = fc_mgr_db.extFlowMibTbl;

	if (unlikely(!mac_addr)) {
		return RTK_FC_HELPER_RET_ERR_NULL_POINTER;
	}

	rtk_fc_mgr_traff_cnt_spin_lock_bh();

	for (i = 0; i < RT_STAT_EXT_FLOWMIB_TABLE_SIZE; i++) {
		if (pTable[i].valid && !memcmp(pTable[i].mac_addr, mac_addr, ETH_ALEN)) {
			memset(&pTable[i], 0, sizeof(rtk_fc_ext_flow_mib_entry_t));
			ret = RTK_FC_HELPER_RET_OK_NEW;
			break;
		}
	}

	rtk_fc_mgr_traff_cnt_spin_unlock_bh();

	if (ret == RTK_FC_HELPER_RET_OK_NEW) {
		rt_rate_host_policer_control_t host_police_entry;

		memset(&host_police_entry, 0, sizeof(host_police_entry));
		ret = rt_rate_hostPolicerControl_set(i, host_police_entry);
	}

	return ret;
}

int rtk_fc_flush_ext_flow_mib_count_by_lan_mac(unsigned char *mac_addr)
{
	int i, ret = RTK_FC_HELPER_RET_ERR;
	rtk_fc_ext_flow_mib_entry_t *pTable = fc_mgr_db.extFlowMibTbl;

	if (unlikely(!mac_addr)) {
		return RTK_FC_HELPER_RET_ERR_NULL_POINTER;
	}

	rtk_fc_mgr_traff_cnt_spin_lock_bh();

	for (i = 0; i < RT_STAT_EXT_FLOWMIB_TABLE_SIZE; i++) {
		if (pTable[i].valid && !memcmp(pTable[i].mac_addr, mac_addr, ETH_ALEN)) {
			ret = RTK_FC_HELPER_RET_OK_NEW;
			break;
		}
	}

	rtk_fc_mgr_traff_cnt_spin_unlock_bh();

	if (ret == RTK_FC_HELPER_RET_OK_NEW) {
		ret = rt_rate_hostPolicerMib_clear(i);
	}

	return ret;
}

int rtk_fc_get_ext_flow_mib_count_by_lan_mac(unsigned char *mac_addr, uint32 *rx_bytes, uint32 *tx_bytes, uint32 *rx_pkts, uint32 *tx_pkts)
{
	int i, ret = RTK_FC_HELPER_RET_ERR;
	rtk_fc_ext_flow_mib_entry_t *pTable = fc_mgr_db.extFlowMibTbl;

	if (!fc_mgr_db.extFlowMibControl.enable)
		return FAIL;

	if (unlikely(!mac_addr || !rx_bytes || !tx_bytes || !rx_pkts || !tx_pkts))
		return FAIL;

	rtk_fc_mgr_traff_cnt_spin_lock();

	for (i = 0; i < RT_STAT_EXT_FLOWMIB_TABLE_SIZE; i++) {
		if (pTable[i].valid && !memcmp(pTable[i].mac_addr, mac_addr, ETH_ALEN)) {
			ret = RTK_FC_HELPER_RET_OK_NEW;
			break;
		}
	}

	rtk_fc_mgr_traff_cnt_spin_unlock();

	if (ret == RTK_FC_HELPER_RET_OK_NEW) {
		rt_rate_host_policer_mib_t policer_mib;

		memset(&policer_mib, 0, sizeof(policer_mib));
		if ((ret = rt_rate_hostPolicerMib_get(i, &policer_mib)) == RT_ERR_OK) {
			*rx_bytes	= policer_mib.rx_count;
			*rx_pkts	= policer_mib.rx_pkt;
			*tx_bytes	= policer_mib.tx_count;
			*tx_pkts	= policer_mib.tx_pkt;
		}
	}

	return ret;
}

int rtk_fc_delete_ext_flow_mib_count_and_flow_by_lan_mac(unsigned char *mac_addr)
{
	int i, ret = RTK_FC_HELPER_RET_ERR;
	rtk_fc_ext_flow_mib_entry_t *pTable = fc_mgr_db.extFlowMibTbl;

	if (unlikely(!mac_addr)) {
		return RTK_FC_HELPER_RET_ERR_NULL_POINTER;
	}

	rtk_fc_mgr_traff_cnt_spin_lock_bh();

	for (i = 0; i < RT_STAT_EXT_FLOWMIB_TABLE_SIZE; i++) {
		if (pTable[i].valid && !memcmp(pTable[i].mac_addr, mac_addr, ETH_ALEN)) {
			memset(&pTable[i], 0, sizeof(rtk_fc_ext_flow_mib_entry_t));
			ret = RTK_FC_HELPER_RET_OK_NEW;
			break;
		}
	}

	rtk_fc_mgr_traff_cnt_spin_unlock_bh();

	if (ret == RTK_FC_HELPER_RET_OK_NEW) {
		rt_rate_host_policer_control_t host_police_entry;
		rt_flow_ops_data_t flow_ops_data;

		memset(&host_police_entry, 0, sizeof(host_police_entry));
		ret = rt_rate_hostPolicerControl_set(i, host_police_entry);

		memset(&flow_ops_data, 0, sizeof(flow_ops_data));
		flow_ops_data.data_delFlow.flowKeyType = RT_FLOW_OP_FLOW_KEY_TYPE_PATTERN;
		flow_ops_data.data_delFlow.flowKey.flowPattern.mac_valid = 1U;
		memcpy(flow_ops_data.data_delFlow.flowKey.flowPattern.mac, mac_addr, ETH_ALEN);

		rt_flow_operation_add(RT_FLOW_OPS_DELETE_FLOW, &flow_ops_data);
	}

	return ret;
}
#endif

/* 	this function should be called within conntrack lock!!! */
int rtk_fc_tcp_in_window(struct sk_buff *skb, struct nf_conn *fc_cached_ct, unsigned int is_ipv4, unsigned char *l3h, int l3h_offset, struct tcphdr *tcph)
{
#ifdef CONFIG_RTK_FC_TCP_SPI_SUPPORT
	struct net *net = &init_net;
	struct nf_conntrack_tuple *reply_tuple;
	unsigned int pf = is_ipv4 ? PF_INET : PF_INET6;
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
	struct nf_conntrack_l3proto *l3proto;
	struct nf_conntrack_l4proto *l4proto;
#endif
	unsigned int dataoff;
	enum ip_conntrack_info ctinfo;
	struct nf_conntrack_tuple tuple;
	enum ip_conntrack_dir dir;
	unsigned int index;
	int set_reply = 0;

	if(!skb || !fc_cached_ct || !l3h || !tcph) {
		return RTK_FC_HELPER_RET_ERR_NULL_POINTER;
	}

	if (IS_ERR(fc_cached_ct)) {
		printk("[%s:%d]fc_cached_ct error.\n", __FUNCTION__, __LINE__);
		return RTK_FC_HELPER_RET_ERR_INVALID_PARAM;
	}

	skb->fcIngressData.skip_nf_conntrack_in = 0;
	
	skb_set_network_header(skb, l3h_offset);
	reply_tuple = &(fc_cached_ct->tuplehash[IP_CT_DIR_REPLY].tuple);

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
	l3proto = rtk_fc_nf_ct_l3proto_find(pf);
	l4proto = rtk_fc_get_nf_ct_protos(pf, IPPROTO_TCP);
#endif

	if(is_ipv4) {
		struct iphdr *iph = (struct iphdr*)l3h;
		dataoff = skb_network_offset(skb) + (iph->ihl<<2);
	}
	else {
		unsigned int extoff = skb_network_offset(skb) + sizeof(struct ipv6hdr);
		__be16 frag_off;
		int protoff;
		u8 nexthdr;

		if (skb_copy_bits(skb, skb_network_offset(skb) + offsetof(struct ipv6hdr, nexthdr),
		&nexthdr, sizeof(nexthdr)) != 0) {
			return RTK_FC_HELPER_RET_ERR_INVALID_PARAM;
		}

		protoff = ipv6_skip_exthdr(skb, extoff, &nexthdr, &frag_off);
		if (protoff < 0 || (frag_off & htons(~0x7)) != 0) {
			return RTK_FC_HELPER_RET_ERR_INVALID_PARAM;
		}

		dataoff = protoff;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
	if (!rtk_fc_g_nf_ct_get_tuple(skb, skb_network_offset(skb),
			     dataoff, pf, IPPROTO_TCP, net, &tuple))
#else
	if (!rtk_fc_g_nf_ct_get_tuple(skb, skb_network_offset(skb),
			     dataoff, pf, IPPROTO_TCP, net, &tuple, l3proto,
			     l4proto))
#endif
	{
		printk("[%s:%d] Can't get tuple\n", __FUNCTION__, __LINE__);
		return RTK_FC_HELPER_RET_ERR_INVALID_PARAM;
	}
	tuple.dst.dir = IP_CT_DIR_REPLY;

	/* 	ctinfo (ct status):						e.g. IP_CT_NEW, IP_CT_ESTABLISHED...
		index  (tcp flags in tcp header):		e.g. TCP_SYNACK_SET, TCP_SYN_SET...
		ct->proto.tcp.state (tcp ct status):	e.g. TCP_CONNTRACK_SYN_SENT, TCP_CONNTRACK_ESTABLISHED
	*/
	if(!memcmp(&tuple, reply_tuple, sizeof(struct nf_conntrack_tuple))) {
		ctinfo = IP_CT_ESTABLISHED_REPLY;
		set_reply = 1;
	}
	else {
		if (test_bit(IPS_SEEN_REPLY_BIT, &fc_cached_ct->status)) {
			ctinfo = IP_CT_ESTABLISHED;
		} else if (test_bit(IPS_EXPECTED_BIT, &fc_cached_ct->status)) {
			ctinfo = IP_CT_RELATED;
		} else {
			ctinfo = IP_CT_NEW;
		}
	}

	dir = CTINFO2DIR(ctinfo);
	index = rtk_fc_get_conntrack_index(tcph);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
	if (!rtk_tcp_in_window(fc_cached_ct, &fc_cached_ct->proto.tcp, dir, index, skb, dataoff, tcph))
#else
	if (!rtk_tcp_in_window(fc_cached_ct, &fc_cached_ct->proto.tcp, dir, index, skb, dataoff, tcph, pf))
#endif
	{
		return RTK_FC_HELPER_RET_ERR_TCP_WINDOW_CHECK_FAIL;
	}

	if(set_reply && !test_and_set_bit(IPS_SEEN_REPLY_BIT, &fc_cached_ct->status)) {
		set_bit(IPS_SEEN_REPLY_BIT, &fc_cached_ct->status);
	}
	
	return RTK_FC_HELPER_RET_OK;
#else
	return RTK_FC_HELPER_RET_ERR_TCP_WINDOW_CHECK_FAIL;
#endif
}

EXPORT_SYMBOL(rtk_fc_tcp_in_window);

inline int rtk_fc_is_highpri_pkt(u8 *data)
{
	u8 *ptr;
	unsigned short eth_type;
	unsigned char ip_proto;

	ptr = data + 12U;
	eth_type =  *((unsigned short *)(ptr));
	if(eth_type == __constant_htons(ETH_P_8021Q))
	{
		ptr += 4U;
		eth_type = *((unsigned short *)(ptr));
	}

	if(eth_type == __constant_htons(0x893a)) {		// 1905 pkt
		return 1;
	}
	else if (eth_type == __constant_htons(ETH_P_IP)) {
		ptr += 2U;		// iph
		ptr += 9U;
		ip_proto = *ptr;

		if (ip_proto == 1U) {	// icmp
			return 1;
		}
	}

	return 0;
}

