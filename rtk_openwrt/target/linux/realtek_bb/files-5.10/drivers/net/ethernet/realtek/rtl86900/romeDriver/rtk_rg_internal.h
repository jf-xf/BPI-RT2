#ifndef RTK_RG_INTERNAL_H
#define RTK_RG_INTERNAL_H

#include <rtk_rg_define.h>
#include <rtk_rg_struct.h>
#include <rtk_rg_debug.h>

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES)
#include <rtk_rg_apolloPro_asicDriver.h>
#include <rtk_rg_apolloPro_internal.h>
//#define CONFIG_RTL9607C_TEST_CHIP
#elif defined(CONFIG_RG_RTL9603CVD_SERIES)
#include <rtk_rg_rtl9603cvd_asicDriver.h>
#include <rtk_rg_apolloPro_internal.h>
#endif

#if defined(CONFIG_RG_RTL9600_SERIES) && defined(CONFIG_GPON_FEATURE)
#include <module/gpon_pppoe_adv/gpon_pppoe_adv.h>
#endif

#ifdef __KERNEL__
//for get link-down indicator register
#include <rtk/intr.h>
#include <module/intr_bcaster/intr_bcaster.h>
#endif


#if defined(CONFIG_RTK_RG_DRV_EVENT_RECORD_VERSION) && (CONFIG_RTK_RG_DRV_EVENT_RECORD_VERSION == 2)
#include <linux/exception_log.h>
#endif

#if 0 //ysleu: I think it's not good to mix model & rome driver codes toghter
#include <hsModel.h>
#endif

// OS-dependent defination
#ifdef CONFIG_APOLLO_MODEL
#define rtlglue_printf printf
#ifndef bzero
#define bzero(p,s) memset(p,0,s)
#endif
#define rg_lock(x) do{}while(0)
#define rg_unlock(y) do{}while(0)
#else
#ifdef __KERNEL__
#undef rtlglue_printf
#define rtlglue_printf _rtk_rg_rtlglue_printf
#define rtlglue_print_hex_dump(arg...) print_hex_dump(KERN_EMERG, ##arg)
#ifndef bzero
#define bzero(p,s) memset(p,0,s)
#endif
#define rg_lock(x) do{}while(0)
#define rg_unlock(y) do{}while(0)
#else
#ifndef u8
#define u8 uint8
#endif
#ifndef u16
#define u16 uint16
#endif
#ifndef u32
#define u32 uint32
#endif
#ifndef u64
#define u64 uint64
#endif
#endif
#endif



#ifdef CONFIG_APOLLOPRO_FPGA
#include <rtl_glue.h>
extern rtk_fb_debug_level_t debug_level;
#endif

#ifdef CONFIG_RTL8686NIC
#include <linux/interrupt.h>

#if !defined(CONFIG_RG_G3_SERIES)
extern struct net_device* nicRootDev;
extern struct re_private *nicRootDevCp;
#endif
extern int drv_nic_register_rxhook(int portmask,int priority,p2rfunc_t rx);
extern int drv_nic_unregister_rxhook(int portmask,int priority,p2rfunc_t rx);
//extern int re8670_start_xmit (struct sk_buff *skb, struct net_device *dev);
extern struct net_device *eth_net_dev;
#endif

//Extern external functions
extern void *rtk_rg_malloc(int NBYTES);
extern void rtk_rg_free(void *APTR);

void memDump (void *start, unsigned int size, char * strHeader);
ssize_t rtk_rg_seq_read(struct file *file, char __user *buf, size_t size, loff_t *ppos);
int rtk_rg_single_open(struct file *file, int (*show)(struct seq_file *, void *),void *data);

int NULL_proc_get(struct seq_file *s, void *v);
int NULL_proc_single_open(struct inode *inode, struct file *file);



//Extern RG Global Variables
extern rtk_rg_dynamic_sram_data_t rg_db_dynamic_sram;
extern rtk_rg_globalDatabase_t rg_db;
extern rtk_rg_globalDatabase_cache_t rg_db_cache;
extern rtk_rg_globalKernel_t rg_kernel;
extern struct platform pf;
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
extern rtk_rg_fbDatabase_t rgpro_db;
#endif

extern rtk_rg_intfInfo_t RG_GLB_INTF_INFO[8];

extern const char* rg_http_request_cmd[];

#ifndef DISABLE
#define DISABLE 0
#endif

#ifndef ENABLE
#define ENABLE 1
#endif

#define UNKNOW_INTF 0
#define WAN_INTF 1
#define LAN_INTF 2

//NAPT/NAPT-R Traffic
#define REG_L4_TRF0	0x00800300
#define REG_L4_TRF1	0x00800400
#define REG_NAT_CTRL	0x00800010
#define REG_V6_BD_CTRL	0x00800028
#define REG_BD_CFG		0x0080002C
#define CLR_L4_TRF_DONE	0x0
#define CLR_L4_TRF0	0x1
#define CLR_L4_TRF1	0x2
#define CLR_L4_TRF_BOTH	0x3
#define FIELD_L4_TRF_CLR_OFFSET 19
#define FIELD_L4_TRF_CHG_OFFSET 21
#define FIELD_L4_TRF_SEL_OFFSET 22

//ARP Traffic
#define REG_ARP_TRF0	0x00800500
#define REG_ARP_TRF1	0x00800600
#if defined(CONFIG_RG_RTL9602C_SERIES)
#define REG_ARP_TRF_BASE 0x00800050
#endif
#define CLR_ARP_TRF_DONE	0x0
#define CLR_ARP_TRF0	0x1
#define CLR_ARP_TRF1	0x2
#define CLR_ARP_TRF_BOTH	0x3
#define FIELD_ARP_TRF_CLR_OFFSET 15
#define FIELD_ARP_TRF_CHG_OFFSET 17
#define FIELD_ARP_TRF_SEL_OFFSET 18

//NEIGHBOR Traffic
#define REG_NEIGHBOR_TRF	0x00800018

//PPPoE Traffic
#define REG_PPPOE_TRF 0x00800014

void RGAPI_BEFORE(void);
void RGAPI_AFTER(void);


int assert_eq(int func_return,int expect_return,const char* func,int line);

#ifndef ASSERT_EQ
#define ASSERT_EQ(value1, value2) \
do { \
	int func_return=(value1); \
	if(assert_eq((func_return), (value2), __FUNCTION__,__LINE__)!=0) \
		return func_return; \
}while(0)
#endif

#ifdef CONFIG_LUNA_WDT_KTHREAD
extern void luna_watchdog_kick(void);
#endif
void _rtk_rg_dnf_game_mechanism_check(uint32 naptOutIdx, uint32 intfIdx);

//RTK API sync with rg_db MACRO
int32 RTK_L2_ADDR_ADD(rtk_l2_ucastAddr_t *pL2Addr);
int32 RTK_L2_ADDR_DEL(rtk_l2_ucastAddr_t *pL2Addr);
int32 RTK_L34_NETIFTABLE_SET(uint32 idx, rtk_l34_netif_entry_t *entry);
int32 RTK_L34_ROUTINGTABLE_SET(uint32 idx, rtk_l34_routing_entry_t *entry);
int32 RTK_L34_EXTINTIPTABLE_SET(uint32 idx, rtk_l34_ext_intip_entry_t *entry);
int32 RTK_L34_NEXTHOPTABLE_SET(uint32 idx, rtk_l34_nexthop_entry_t *entry);
int32 RTK_L34_PPPOETABLE_SET(uint32 idx, rtk_l34_pppoe_entry_t *entry);
int32 RTK_L34_ARPTABLE_SET(uint32 idx, rtk_l34_arp_entry_t *entry);
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
int32 RTK_L34_NAPTINBOUNDTABLE_SET(int8 forced,uint32 idx, rtk_rg_naptInbound_entry_t *entry);
int32 RTK_L34_NAPTOUTBOUNDTABLE_SET(int8 forced,uint32 idx, rtk_rg_naptOutbound_entry_t *entry);
#else
int32 RTK_L34_NAPTINBOUNDTABLE_SET(int8 forced,uint32 idx, rtk_l34_naptInbound_entry_t *entry);
int32 RTK_L34_NAPTOUTBOUNDTABLE_SET(int8 forced,uint32 idx, rtk_l34_naptOutbound_entry_t *entry);
#endif
int32 RTK_L34_WANTYPETABLE_SET(uint32 idx,rtk_wanType_entry_t * entry);
int32 RTK_VLAN_CREATE(rtk_vlan_t vid);
int32 RTK_VLAN_PORT_SET(rtk_vlan_t vid,rtk_portmask_t * pMember_portmask,rtk_portmask_t * pUntag_portmask);
int32 RTK_VLAN_EXTPORT_SET(rtk_vlan_t vid,rtk_portmask_t * pExt_portmask);
int32 RTK_VLAN_FID_SET(rtk_vlan_t vid,rtk_fid_t fid);
int32 RTK_VLAN_FIDMODE_SET(rtk_vlan_t vid,rtk_fidMode_t mode);
int32 RTK_VLAN_PRIORITY_SET(rtk_vlan_t vid,rtk_pri_t priority);
int32 RTK_VLAN_PRIORITYENABLE_SET(rtk_vlan_t vid,rtk_enable_t enable);
int32 RTK_VLAN_DESTROY(rtk_vlan_t vid);
int32 RTK_VLAN_PORTPVID_SET(rtk_port_t port,uint32 pvid);
int32 RTK_VLAN_EXTPORTPVID_SET(uint32 extPort,uint32 pvid);
int32 RTK_L34_BINDINGTABLE_SET(uint32 idx,rtk_binding_entry_t * bindEntry);
int32 RTK_L34_IPV6ROUTINGTABLE_SET(uint32 idx,rtk_ipv6Routing_entry_t * ipv6RoutEntry);
int32 RTK_L34_IPV6NEIGHBORTABLE_SET(uint32 idx,rtk_ipv6Neighbor_entry_t * ipv6NeighborEntry);


int32 RTK_VLAN_PORTPROTOVLAN_SET(rtk_port_t port,uint32 protoGroupIdx,rtk_vlan_protoVlanCfg_t * pVlanCfg);
void _rtk_rg_cleanPortAndProtocolSettings(rtk_port_t port);
int _rtk_rg_broadcastWithDscpRemarkMask_get(unsigned int bcMask,unsigned int *bcWithoutDscpRemarMask,unsigned int *bcWithDscpRemarByInternalpriMask,unsigned int *bcWithDscpRemarByDscpkMask);
int _rtk_rg_broadcastWithDot1pRemarkMask_get(unsigned int bcMask,unsigned int *bcWithDot1pRemarkMask,unsigned int *bcWithoutDot1pRemarkMask);
int32 _rtk_rg_sendBroadcastToWan(rtk_rg_pktHdr_t *pPktHdr, struct sk_buff *skb, int wanIdx, unsigned int dpMask);
int _rtk_rg_BroadcastPacketToLanWithEgressACLModification(int direct, int naptIdx, rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb,int l3Modify,int l4Modify, unsigned int bcTxMask, unsigned int allDestPortMask,  rtk_rg_port_idx_t egressPort);
int rtk_rg_debug_level_show(struct seq_file *s, void *v);
int rtk_rg_debug_level_change(struct file *file, const char *buffer, unsigned long count, void *data);
int rtk_rg_traceFilterShow(struct seq_file *s, void *v);
int rtk_rg_traceFilterChange(struct file *file, const char *buffer, unsigned long count, void *data);
#ifdef __KERNEL__
#ifdef CONFIG_RG_CALLBACK
int rtk_rg_callback_show(struct seq_file *s, void *v);
#endif
#endif
rtk_rg_successFailReturn_t _rtk_rg_internalPrioritySelect(rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb);
int rtk_rg_qosInternalAndRemark_show(struct seq_file *s, void *v);
rtk_rg_successFailReturn_t _rtk_rg_skipARPLearningOrNot(int l3Idx, ipaddr_t sip, int srcPortIdx, uint8 *smac);
int _rtk_rg_softwareArpTableDel(rtk_rg_arp_linkList_t *pDelArpEntry);
int _rtk_rg_freeRecentlyNotUsedArpList(void);
int _rtk_rg_softwareArpTableAdd(unsigned short routingIdx, ipaddr_t ipv4Addr, int l2Idx, int staticEntry, int *swArpIdx);
//int32 _rtk_rg_bindingRuleCheck(rtk_rg_pktHdr_t *pPktHdr, int *wanGroupIdx);
rtk_rg_successFailReturn_t _rtk_rg_createGatewayMacEntry(uint8 *gatewayMac, int vlanID, uint32 untagSet, int intfIdx, uint8 isLanIntf);
int32 _rtk_rg_updateWANPortBasedVID(rtk_rg_port_idx_t wan_port);
void _rtk_rg_refreshPPPoEPassThroughLanOrWanPortMask(void);
int _rtk_rg_globalVariableReset(void);
void rtk_rg_fwdEngineHouseKeepingTimerFunc(unsigned long task_priv);
void _rtk_rg_set_initState(rtk_rg_initState_t newState);
int _rtk_rg_updatePortBasedVIDByLanOrder(rtk_portmask_t mac_pmask, rtk_portmask_t etp_pmask);


int _rtk_rg_egressPacketSend_for_gponDsBcFilterAndRemarking(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr, int toMasterWifi);

#if 0
int32 _rtk_rg_cf_reserved_pon_intfSSIDRemap_add(int intfIdx,int ssid, int vid);
int32 _rtk_rg_cf_reserved_pon_intfSSIDRemap_del(int intfIdx);
#endif
unsigned int _rtk_rg_hashFun_xPear(uint8 len, uint8 ipProto, uint32 srcAddr, uint32 destAddr, uint32 identification);

void _rtk_rg_trap_specific_length_ack_reservedAcl_set(rtk_rg_trapSpecificLengthAck_t trapAck, uint8 addAcl);

//setup aclSWEntry
int _rtk_rg_acl_asic_init(void);
int _rtk_rg_classify_asic_init(void);
int _rtk_rg_portmask_translator(rtk_rg_portmask_t in_pmask, rtk_portmask_t* out_mac_pmask, rtk_portmask_t* out_ext_pmask);
rtk_rg_err_code_t _rtk_rg_portToMacPort_translator(rtk_rg_port_idx_t in_Port, rtk_rg_mac_port_idx_t* out_mac_port, rtk_rg_mac_ext_port_idx_t* out_mac_extPort);
rtk_rg_err_code_t _rtk_rg_macPortToPort_translator(rtk_rg_port_idx_t* out_Port, rtk_rg_mac_port_idx_t in_mac_port, rtk_rg_mac_ext_port_idx_t in_mac_extPort);
uint32 _rtk_rg_isVlanMember(uint32 vlanId, rtk_rg_mac_port_idx_t mac_port, rtk_rg_mac_ext_port_idx_t mac_extPort);
rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngineVLANFiltering(uint32 vlanId, rtk_rg_mac_port_idx_t mac_port, rtk_rg_mac_ext_port_idx_t mac_extPort, rtk_rg_mbssidDev_t wlan_dev);
rtk_rg_err_code_t _rtk_rg_delShortcutAndHwFlow(rtk_rg_flowFilterInfo_t delFlowInfo);
rtk_rg_err_code_t _rtk_rg_delShortcutAndHwFlow_byNaptOutIdx(uint32 naptOutIdx);
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 30)
void _rtk_rg_NPTv6_outbound_check(rtk_rg_pktHdr_t *pPktHdr);
void _rtk_rg_NPTv6_inbound_check(rtk_rg_pktHdr_t *pPktHdr);
#endif
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
rtk_rg_entryGetReturn_t _rtk_rg_extPMaskEntry_find(uint32 extPMask);
rtk_rg_entryGetReturn_t _rtk_rg_extPMaskEntryIdx_get(uint32 extPMask);
rtk_rg_entryGetReturn_t _rtk_rg_indirectMacEntry_find(int l2Idx);
uint32 _rtk_rg_flow_matchOrNot_byPktHdr(uint32 flow_idx, rtk_rg_pktHdr_t *pPktHdr, int smacL2Idx, int dmacL2Idx);
uint32 _rtk_rg_decideNetIfIdx(rtk_rg_port_idx_t checkPort, rtk_rg_mac_port_idx_t checkMacPort, uint8 checkCtagIf, uint16 checkCvid);
uint8 _rtk_rg_flow_isMulticast(rtk_rg_table_flowEntry_t flowPathEntry, uint16 path34_isGmac);
uint32 _rtk_rg_sw_flowHashIPv6DstAddr_get(uint8 ipDes[16]);
uint32 _rtk_rg_sw_flowHashIPv6SrcAddr_get(uint8 ipSrc[16]);
uint32 _rtk_rg_flow_hashIndex(rtk_rg_table_flowEntry_t flowPathEntry, uint16 igrSVID, uint16 igrCVID, uint16 lutDaIdx_gmacChk /*used by path3 & path4*/);
int32 _rtk_rg_flow_add_hw_index_check(rtk_rg_table_flowEntry_t flowPathEntry, uint16 igrSVID, uint16 igrCVID, uint16 lutDaIdx, uint16 path34_isGmac);
rtk_rg_err_code_t _rtk_rg_flow_add(uint32 *pFlow_idx, int32 forced_flowHashIdx, rtk_rg_table_flowEntry_t *pFlowPathEntry, rtk_rg_flow_extraInfo_t flowExtraInfo);
rtk_rg_err_code_t _rtk_rg_flow_del(uint32 flow_idx, uint8 replaceHwFlow);
rtk_rg_err_code_t _rtk_rg_flow_clear(void);
rtk_rg_err_code_t _rtk_rg_flow_del_by_tcpUdpTrackingGroup(uint32 delGroupIdx, uint8 replaceHwFlow);
rtk_rg_err_code_t _rtk_rg_flow_del_by_naptOutIdx(uint32 naptOutIdx, uint8 replaceHwFlow);
rtk_rg_err_code_t _rtk_rg_flow_del_by_arpIdx(uint32 arpIdx);
rtk_rg_err_code_t _rtk_rg_flow_del_by_neighborIdx(uint32 neighborIdx);
rtk_rg_err_code_t _rtk_rg_flow_del_by_l2Idx(uint32 l2Idx);
rtk_rg_err_code_t _rtk_rg_flow_del_by_dmacL2Idx(uint32 dmacL2Idx);
rtk_rg_err_code_t _rtk_rg_hwFlow_del_by_dmacL2Idx(uint32 dmacL2Idx);
rtk_rg_err_code_t _rtk_rg_flow_del_by_dmacL2Idx_and_dmac2cvid(uint32 dmacL2Idx);
rtk_rg_err_code_t _rtk_rg_flow_del_by_spa(uint32 spa);
rtk_rg_err_code_t _rtk_rg_flow_del_by_intf(uint32 netifIdx);
void _rtk_rg_flowUpdateIdleTime(uint32 flowIdx);
void _rtk_rg_flowUpdate(rtk_rg_pktHdr_t *pPktHdr);
rtk_rg_err_code_t _rtk_rg_moveFlowFromHwToSw(uint32 swFlowIdx, uint32 hwFlowIdx, uint32 hashIdx);
rtk_rg_err_code_t _rtk_rg_swapHwFlowOfNaptToSw(uint32 naptOutIdx);
#if !defined(CONFIG_RG_G3_SERIES)
rtk_rg_err_code_t _rtk_rg_moveFlowFromSwToHw(uint32 hwFlowIdx, uint32 swFlowIdx, uint32 hashIdx);
#endif	// end !defined(CONFIG_RG_G3_SERIES)
rtk_rg_err_code_t _rtk_rg_moveMostRecentlyUsedFlowFromSwToHw(uint32 hwFlowIdx, uint32 hashIdx);
uint32 _rtk_rg_flow_canAddNaptInboundInHw(uint32 sip, uint32 dip, uint16 sport, uint16 dport, uint32 isTcp);
rtk_rg_err_code_t _rtk_rg_l34WanAccessLimit_delL2Idx(uint32 l2Idx);
int _rtk_rg_naptRecorded_flowListAdd(uint32 naptOutIdx, uint32 addFlowIdx);
int _rtk_rg_naptRecorded_flowListDel(uint32 delFlowIdx);

uint32 _rtk_rg_fwdEngine_isL2OrL3(rtk_rg_pktHdr_t *pPktHdr);
rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_L2L3_TcpUdpConnectionTracking(rtk_rg_pktHdr_t *pPktHdr);
uint32 _rtk_rg_L2L3_flowTcpUdpTrackingHashIndex(uint32 srcIP, uint32 destIP, uint16 srcPort, uint16 destPort);
int _rtk_rg_L2L3_flowTcpUdpTrackingListAdd(uint32 groupIdx, uint32 addFlowIdx);
int _rtk_rg_L2L3_flowTcpUdpTrackingListDel(uint32 delFlowIdx);
rtk_rg_err_code_t _rtk_rg_L2L3_flowTcpUdpTrackingGroupLookUp(rtk_rg_pktHdr_t *pPktHdr, int32 *pGroupIdx);
rtk_rg_err_code_t _rtk_rg_L2L3_flowTcpUdpTrackingGroupAdd(rtk_rg_pktHdr_t *pPktHdr, int32 *pGroupIdx);
rtk_rg_err_code_t _rtk_rg_L2L3_flowTcpUdpTrackingGroupDel(uint32 delGroupIdx);
rtk_rg_err_code_t _rtk_rg_L2L3_flowTcpUdpTrackingGroupLRU(uint32 flowTcpUdpTrackingHashIdx);

uint32 _rtk_rg_L2L3_fragmentHashIndex(uint8 ipProto, uint32 srcIP, uint32 destIP, uint32 identification);
void _rtk_rg_L2L3_fragmentListFlush(void);
rtk_rg_err_code_t _rtk_rg_L2L3_fragmentListLookUp(rtk_rg_pktHdr_t *pPktHdr, int32 *pFragIdx);
rtk_rg_err_code_t _rtk_rg_L2L3_fragmentListAdd(rtk_rg_pktHdr_t *pPktHdr, rtk_rg_fwdEngineReturn_t fragAction, int32 *pFragIdx);
rtk_rg_err_code_t _rtk_rg_L2L3_fragmentListDel(uint32 delFragIdx);
rtk_rg_err_code_t _rtk_rg_L2L3_fragmentPacketQueuing(rtk_rg_pktHdr_t *pPktHdr, struct sk_buff *skb);
rtk_rg_err_code_t _rtk_rg_L2L3_fragmentQueueProcessing(rtk_rg_pktHdr_t *pPktHdr, rtk_rg_fwdEngineReturn_t fragAction);
rtk_rg_fwdEngineReturn_t _rtk_rg_L2L3_fragmentHandler(rtk_rg_pktHdr_t *pPktHdr, struct sk_buff *skb, rtk_rg_fwdEngineReturn_t fragAction);
void _rtk_rg_L2L3_fragmentTimeoutCheck(void);
rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_L2L3_fragmentsShortcut(rtk_rg_pktHdr_t *pPktHdr, int32 *pFlowHitIdx);
rtk_rg_err_code_t _rtk_rg_flowIndex_of_fragmentList_change(int32 newFlowIdx, int32 oldFlowIdx);
void _rtk_rg_flow_fragmentUsedBit_reset_by_flowIndex(uint32 flowIdx);
void _rtk_rg_flow_fragmentUsedBit_clear(void);
void _rtk_rg_L2L3_flow_fragmentUsedBit_reset(void);
void _rtk_rg_L4_flow_fragmentOutUsedBit_reset(void);
void _rtk_rg_L4_flow_fragmentInUsedBit_reset(void);
void _rtk_rg_log_flow_ops(uint32 isTCP, uint32 sip, uint32 dip, uint16 sport, uint16 dport, int establish_connection);
#endif	//CONFIG_RG_FLOW_BASED_PLATFORM

void _rtk_rg_sw_txPPPoEUpdate(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr);
void _rtk_rg_sw_txChecksumUpdate(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr);
void _rtk_rg_sw_txPadding(struct sk_buff *skb, struct sk_buff **new_skb, rtk_rg_pktHdr_t *pPktHdr);
#if defined(CONFIG_APOLLO_FPGA_PHY_TEST)
void _rtk_rg_fpgaTest_removeCpuTag(struct sk_buff *skb, rtk_rg_rxdesc_t *pRxDesc);
void _rtk_rg_fpgaTest_addCpuTag(struct sk_buff *skb, rtk_rg_txdesc_t *pTxDesc);
void _rtk_rg_fpgaTest_txProcess(struct sk_buff *skb, struct sk_buff **new_skb, void *pTxDescPtr, rtk_rg_pktHdr_t *pPktHdr);
#endif
int _rtk_rg_is_aclSWEntry_init(void);
int _rtk_rg_aclSWEntry_init(void);
int _rtk_rg_aclSWEntry_get(int index, rtk_rg_aclFilterEntry_t* aclSWEntry);
int _rtk_rg_aclSWEntry_set(int index, rtk_rg_aclFilterEntry_t aclSWEntry);
int _rtk_rg_aclSWEntry_empty_find(int* index);
int _rtk_rg_aclSWEntry_dump(void);
int _rtk_rg_classifySWEntry_init(void);
int _rtk_rg_classifySWEntry_get(int index, rtk_rg_classifyEntry_t* cfSWEntry);
int _rtk_rg_classifySWEntry_set(int index, rtk_rg_classifyEntry_t cfSWEntry);
int _rtk_rg_is_stormControlEntry_init(void);
int _rtk_rg_stormControlEntry_init(void);

int32 _rtk_rg_acl_reserved_stag_ingressCVidFromPVID(uint32 in_pvid, uint32 in_port);
#if 1
int32 _rtk_rg_acl_reserved_pppoePassthrough_IntfisPppoewan_add(int intf_idx, rtk_mac_t gmac);
int32 _rtk_rg_acl_reserved_pppoePassthrough_IntfisPppoewan_del(int intf_idx);
void _rtk_rg_acl_reserved_pppoePassthroughDefaultRule_add(unsigned int lan_pmask, unsigned int wan_pmsk, int remark_vid);
void _rtk_rg_acl_reserved_pppoePassthroughDefaultRule_del(void);
#endif

#if defined(CONFIG_RG_G3_SERIES)
rtk_rg_mcEngine_info_t *_rtk_rg_g3McEngineInfoFind(uint32 *groupAddress ,uint32 *sourceAddress,uint32 isIPv6);
rtk_rg_mcEngine_info_t *_rtk_rg_g3McEngineInfoGet(uint32 *groupAddress ,uint32 *sourceAddress,uint32 isIPv6,uint32 l2mcgid, uint32 l3mcgid);
int32 _rtk_rg_g3McEngineInfoDel(rtk_rg_mcEngine_info_t *pMcEngineInfo);
int32 _rtk_rg_fc_g3L3Policer_set(uint32 index, uint32 rate, rtk_enable_t ifgInclude);
int32 _rtk_rg_fc_g3L3Policer_get(uint32 index, uint32 *pRate , rtk_enable_t *pIfgInclude);
int32 _rtk_rg_fc_g3L3PolicerMode_set(uint32 index, rtk_rate_metet_mode_t meterMode);
int32 _rtk_rg_fc_g3L3PolicerMode_get(uint32 index, rtk_rate_metet_mode_t *pMeterMode);
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
int32 _rtk_rg_fc_g3L2Policer_forSmacHostPol_set(uint32 index, uint32 rate, rtk_enable_t ifgInclude);
#endif
#endif

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
int _rtk_rg_hw_funcbasedMeter_hwIdx_get(void);
int32 _rtk_rg_hostPolicongAndStormControlAndResAcl_readdByShareMeter(void);
int32 _rtk_rg_hostPolicongAndStormControlAndResAcl_readdByFuncbasedMeter(rtk_rg_meter_type_t type, uint32 idx);
#endif
#if defined(CONFIG_RG_G3_SERIES)
int32 _rtk_rg_g3_get_mac_hostPolicing_info(rtk_mac_t *target_mac, rtk_rg_g3_mac_hostPolicing_info_t *hp_info);
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
int32 _rtk_rg_g3_l2cls_for_hostPoliceControl_add(rtk_mac_t *target_mac, rtk_rg_g3_mac_hostPolicing_info_t *hp_info);
int32 _rtk_rg_g3_l2cls_for_hostPoliceControl_del(rtk_mac_t *target_mac);
#endif

#endif


#if 0
int32 _rtk_rg_acl_trap_broadcast_add(void);
int32 _rtk_rg_acl_trap_broadcast_del(void);
#endif

int32 _rtk_rg_acl_reserved_wifi_extPMaskTranslate_add(int patchFor,unsigned int igr_extPmsk,unsigned int egr_extPmsk);
int _rtk_rg_ingressACLPatternCheck(rtk_rg_pktHdr_t *pPktHdr, int ingressCvidRuleIdxArray[]);
rtk_rg_fwdEngineReturn_t _rtk_rg_ingressACLAction(rtk_rg_pktHdr_t *pPktHdr);
int _rtk_rg_egressACLPatternCheck(int direct, int naptIdx, rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb,int l3Modify,int l4Modify,rtk_rg_port_idx_t egressPort);
int _rtk_rg_egressACLAction(int direct, rtk_rg_pktHdr_t *pPktHdr);
rtk_rg_fwdEngineReturn_t _rtk_rg_modifyPacketByACLAction(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr,rtk_rg_port_idx_t egressPort);
int _rtk_rg_TranslateVlanSvlan2Packet(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr, uint32 dataPathToWifi);
void _rtk_rg_egressPacketSend(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr);
int _rtk_rg_dscpRemarkToSkb(rtk_rg_qosDscpRemarkSrcSelect_t dscpSrc,rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb, int dscp_off);
int _rtk_rg_qosDscpRemarking(rtk_rg_mac_port_idx_t egressPort,rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb);
void _rtk_rg_hwnatACLManipulate(rtk_enable_t natSwitch);
int _rtk_rg_getPortLinkupStatus(void);
int _rtk_rg_clearMacRelatedDataByPort(unsigned int port);
int _rtk_rg_egressPortMaskCheck(rtk_rg_pktHdr_t *pPktHdr, unsigned int bcTxMask, unsigned int allDestPortMask);
int _rtk_rg_egressPacketDoQosRemarkingDecision(rtk_rg_pktHdr_t *pPktHdr, struct sk_buff *skb, struct sk_buff *bcSkb, unsigned int dpMask, unsigned int internalVlanID);



/* MIB / Counter */
rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_flowEgressCountPredict(rtk_rg_pktHdr_t *pPktHdr, uint32 *ingressPktCnt, uint32 *egressPktCnt, uint32 *egressByteCnt);
rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_flowMIBUpdate(int idx, uint32 ingressPktCnt, uint32 egressPktCnt, uint32 egressByteCnt);


/*IGMP*/
int32 _rtk_rg_igmpReport_portmask_check_and_limit(rtk_rg_pktHdr_t *pPktHdr, uint32* egress_port_mask);
int32 _rtk_rg_igmpLeave_portmask_check_and_limit(rtk_rg_pktHdr_t *pPktHdr, uint32* egress_port_mask);
int32 _rtk_rg_igmpMldQuery_portmask_check_and_limit(rtk_rg_pktHdr_t *pPktHdr, uint32* egress_port_mask,uint32* wlan0Pmsk);



/* MAC/URL Filter */
int _rtk_rg_is_macFilter_table_init(void);
int _rtk_rg_macFilter_table_init(void);
int _rtk_rg_is_urlFilter_table_init(void);
int _rtk_rg_urlFilter_table_init(void);
int _rtk_rg_urlAssignPri_table_init(void);


/* NIC Module */
#define RG_FWDENGINE_PKT_LEN	2048
#define RX_OFFSET	2

/* LUT */
rtk_rg_err_code_t _rtk_rg_l2_nextValidEntry_get(int32 *pScanIdx, rtk_l2_addr_table_t *pL2Entry);
rtk_rg_lookupIdxReturn_t _rtk_rg_macLookup(u8 *pMac, int vlanId, uint8 checkSwlut);
rtk_rg_lutIdx_return_t _rtk_rg_lutIdx_find(u8 *pMac, int vlanId, int *lutIdx);
void _rtk_rg_layer2CleanL34ReferenceTable(int l2Idx);
int32 _rtk_rg_softwareLut_add(rtk_rg_macEntry_t *macEntry, int hashIdx, char category, char wlan_dev_index);
rtk_rg_successFailReturn_t _rtk_rg_softwareLut_addFromHw(int l2Idx, char category);
void _rtk_rg_softwareLut_allDelete(void);
void _rtk_rg_softwareLut_portDelete(rtk_rg_port_idx_t port_idx);
#ifdef CONFIG_MASTER_WLAN0_ENABLE
void _rtk_rg_softwareLut_wlan0DevDelete(int dev_idx);
#if defined(CONFIG_RG_FLOW_NEW_WIFI_MODE)
void _rtk_rg_lutExtport_translator(rtk_rg_port_idx_t *portIdx);
void _rtk_rg_wlanDevToPort_translator(rtk_rg_mbssidDev_t wlanDev, rtk_rg_port_idx_t *portIdx);
void _rtk_rg_wlanDevToExtPortmask_translator(rtk_rg_mbssidDev_t wlanDev, rtk_portmask_t *etpmsk);
void _rtk_rg_wlanDevMaskToExtPortmask_translator(uint32 wlanDevMask, rtk_portmask_t *etpmsk);
void _rtk_rg_egressExtPort_translator(rtk_rg_pktHdr_t *pPktHdr);
#endif
#endif
int _rtk_rg_wlanDeviceCount_dec(int wlan_idx, unsigned char *macAddr, int *dev_idx);
void _rtk_rg_check_wlan_device_exist_or_not(void);

void _rtk_rg_lutReachLimit_init(rtk_rg_accessWanLimitType_t type, void (*function)(unsigned long), unsigned long data);
void _rtk_rg_lutReachLimit_category(unsigned long category);
void _rtk_rg_lutReachLimit_portmask(unsigned long portmsk);
rtk_rg_err_code_t _rtK_rg_checkCategoryPortmask(rtk_l2_ucastAddr_t *lut);
int32 _rtK_rg_checkCategoryPortmask_spa(rtk_rg_port_idx_t spa);
void _rtk_rg_disablePortmaskPermitedLut(uint32 l2Idx);
int32 _rtk_rg_checkPortNotExistByPhy(rtk_rg_port_idx_t port);
rtk_rg_entryGetReturn_t _rtk_rg_findAndReclamIpmcEntry(ipaddr_t sip,ipaddr_t dip,int32 ivlsvl,int32 vid_fid,int32 ipfilterIdx,int32 ipfilterEn);


/* NAPT */
int _rtk_rg_l3lookup(ipaddr_t ip);
unsigned int _rtk_rg_NAPTRemoteHash_get( unsigned int ip, unsigned int port);
rtk_rg_extPortGetReturn_t _rtk_rg_naptExtPortGetAndUse(int force, int isTcp, uint32 internalIp, uint16 internalPort, uint16 wishPort, uint32 *pIp, int addRefCnt, int naptrLookupHit);
int _rtk_rg_naptExtPortInUsedCheck(int force, int isTcp, uint32 internalIp, uint16 internalPort, uint16 wishPort, int addRefCount, int naptrLookupHit, int skipPreDefinedPort);
int _rtk_rg_naptExtPortFree(int force, int isTcp, uint32 internalIp, uint16 internalPort, uint16 port, int fromPS);
int32 _rtk_rg_shortCut_clear(void);
#ifdef CONFIG_ROME_NAPT_SHORTCUT
rtk_rg_entryGetReturn_t _rtk_rg_shortcutARPFind(int routingIdx, ipaddr_t ipAddr);
int32 _rtk_rg_v4ShortCut_delete(int deleteIdx);
#endif
#if defined(CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT)||defined(CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT)
rtk_rg_entryGetReturn_t _rtk_rg_shortcutNEIGHBORFind(unsigned char *ipv6Addr);
#endif
#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
int32 _rtk_rg_v6ShortCut_delete(int deleteIdx);
#endif
rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_connType_lookup(rtk_rg_pktHdr_t *pPktHdr, ipaddr_t *transIP, int16 *transPort);
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
uint32 _rtk_rg_flowTcpUdpTrackingHashIndex(uint32 srcIP, uint32 destIP, uint16 srcPort, uint16 destPort);
void _rtk_rg_fwdEngine_updateHardwareFlow(rtk_rg_pktHdr_t *pPktHdr, rtk_rg_fwdEngineReturn_t ret);
#else	//not CONFIG_RG_FLOW_BASED_PLATFORM
int32 _rtk_rg_delNaptShortCutEntrybyOutboundIdx(int outIdx);
uint32 _rtk_rg_shortcutHashIndex(uint32 srcIP, uint32 destIP, uint16 srcPort, uint16 destPort);
uint32 _rtk_rg_ipv6ShortcutHashIndex(uint32 srcIP, uint32 destIP, uint16 srcPort, uint16 destPort);
#endif
void _rtk_rg_naptFilterCopyToPS(rtk_rg_pktHdr_t *pPktHdr);
rtk_rg_fwdEngineReturn_t _rtk_rg_napt_trap_to_ps(int direct, rtk_rg_pktHdr_t *pPktHdr, int naptIdx,int l3Modify,int l4Modify);
rtk_rg_fwdEngineReturn_t _rtk_rg_naptPriority_assign(int direct, rtk_rg_pktHdr_t *pPktHdr, rtk_rg_table_naptOut_t *naptout_entry, rtk_rg_table_naptIn_t *naptin_entry, int naptIdx,int l3Modify,int l4Modify);
rtk_rg_entryGetReturn_t _rtk_rg_naptPriority_pattern_check(int rule_direction, int pkt_direction, rtk_rg_pktHdr_t *pPktHdr, rtk_rg_sw_naptFilterAndQos_t *pNaptPriorityRuleStart,int direct, int naptIdx,int l3Modify,int l4Modify);
void _rtk_rg_fwdEngine_updateFlowStatus(rtk_rg_pktHdr_t *pPktHdr);

void _rtk_rg_naptInfoCollectForCallback(int naptOutIdx, rtk_rg_naptInfo_t *naptInfo);
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM) || defined(CONFIG_RG_RTL9600_SERIES)
void _rtk_rg_aclUDPencapMirror(struct sk_buff *skb,rtk_rg_pktHdr_t *pPktHdr_orig);
#endif
int _rtk_rg_add_pppoe_lcp_reserved_acl_detect(void);
int _rtk_rg_add_l2tp_lcp_reserved_acl_detect(void);

#ifdef CONFIG_RG_RTL9602C_SERIES
void _rtk_rg_dslite_routing_reserved_acl_decision(void);
#endif


/* platform */
int32 _rtk_rg_platform_function_register_check(struct platform *pf);


/* IPv6 */
uint8 _rtk_rg_CompareIFID(uint8* dip, uint64 interfaceid);
int _rtk_rg_ipv6IFIDCompare(unsigned int prefixLen, unsigned char *ipv6Addr, unsigned char *ipv6AddrCmp);
int _rtk_rg_v6L3lookup(unsigned char *ipv6);
int _rtk_rg_ipv6Addr_cmp(uint8 *ipv6_addr0, uint8 *ipv6_addr1, uint32 prefixLen);
int _rtk_rg_ipv6Addr_cpy(uint8 *ipv6_addr0, uint8 *ipv6_addr1, uint32 prefixLen);

/* CP3 PROFILE */
int profile(const char *func);
void profile_end(int idx);

/* debug */
void _rtk_rg_psRxMirrorToPort0(struct sk_buff *skb, struct net_device *dev);



/*Other Module*/

//extern rtk_rg_arp_request_t	RG_GLB_ARP_REQUEST[MAX_NETIF_HW_TABLE_SIZE];

//called back from Layer 2, return the IPaddr and mac for gateway
//if the ARP request is not found or timeout, return L2Idx = -1
int32 _rtk_rg_internal_wanSet(int wan_intf_idx, rtk_rg_ipStaticInfo_t *hw_static_info);
rtk_rg_err_code_t _rtk_rg_apollo_wanInterface_add(rtk_rg_wanIntfConf_t *wanintf, int *wan_intf_idx);
int __rtk_rg_internal_GWMACSetup(ipaddr_t ipAddr, int l2Idx, int force);
int _rtk_rg_internal_GWMACSetup(ipaddr_t IPaddr, int L2Idx);
int _rtk_rg_internal_GWMACSetup_softUpdate(ipaddr_t ipAddr, int l2Idx);
int _rtk_rg_internal_GWMACSetup_stage2(int matchIdx, int l2Idx);
int _rtk_rg_internal_PPTPMACSetup(ipaddr_t ipAddr, int l2Idx);
void _rtk_rg_PPTPLearningTimerInitialize(int wan_intf_idx);
int _rtk_rg_internal_L2TPMACSetup(ipaddr_t ipAddr, int l2Idx);
void _rtk_rg_L2TPLearningTimerInitialize(int wan_intf_idx);
int __rtk_rg_internal_IPV6GWMACSetup(unsigned char *ipv6Addr, int L2Idx, int force);
int _rtk_rg_internal_IPV6GWMACSetup(unsigned char *ipv6Addr, int L2Idx);
int _rtk_rg_internal_IPV6GWMACSetup_softUpdate(unsigned char *ipv6Addr, int l2Idx);
int _rtk_rg_internal_IPV6GWMACSetup_stage2(int matchIdx, int l2Idx);
int _rtk_rg_internal_STATICROUTEMACSetup(ipaddr_t ipAddr, int l2Idx);
int _rtk_rg_globalVariableReset(void);
void _rtk_rg_fwdEngineGlobalVariableReset(void);
int _rtk_rg_layer2GarbageCollection(int l2Idx);
int _rtk_rg_layer2HashedReplace(int l2Idx, int lutGroupIdx /*does not LRU lut entries in this lut group*/);
rtk_rg_entryGetReturn_t _rtk_rg_layer2LeastRecentlyUsedReplace(int l2Idx, int lutGroupIdx /*does not LRU lut entries in this lut group*/);
int _rtk_rg_decreaseNexthopReference(int nexthopIdx);
int _rtk_rg_decreaseExtIPReference(int ipIdx);
rtk_rg_fwdEngineReturn_t _rtk_rg_bindingRuleCheck(rtk_rg_pktHdr_t *pPktHdr, int *wanGroupIdx);
int32 rtk_rg_ipv6_externalIp_set(int index, rtk_rg_table_v6ExtIp_t v6ExtIp_entry);
int32 _rtk_rg_ipv6_externalIp_get(int index, rtk_rg_table_v6ExtIp_t *v6ExtIp_entry);
rtk_rg_fwdEngineReturn_t _rtk_rg_checkGwIp(rtk_rg_pktHdr_t *pPktHdr);
int32 _rtk_rg_updateBindWanIntf(rtk_rg_wanIntfConf_t *wanintf);
int32 _rtk_rg_updateBindOtherWanPortBasedVID(rtk_rg_wanIntfConf_t *otherWanIntf);
int32 _rtk_rg_updateNoneBindingPortmask(uint32 wanPmsk);
rtk_rg_err_code_t _rtk_rg_dmac2cvid_and_vidUnmatch_update(void);
#if defined(CONFIG_RG_WAN_MSS_CACHE)
void _rtk_rg_mssCache_reset(int intf_idx);
void _rtk_rg_mssCache_add(int intf_idx, ipaddr_t dest, unsigned short advmss);
void _rtk_rg_mssCache_update(int intf_idx, ipaddr_t dest, unsigned short advmss);
void _rtk_rg_v6mssCache_add(int intf_idx, unsigned char *v6dest, unsigned short advmss);
void _rtk_rg_v6mssCache_update(int intf_idx, unsigned char *v6dest, unsigned short advmss);
unsigned int _rtk_rg_mssCache_search(int intf_idx, ipaddr_t dest);
unsigned int _rtk_rg_v6mssCache_search(int intf_idx, unsigned char *v6dest);
#endif
uint32 rtk_rg_apollo_urlFilterBySmacHash(uint8* urlFilterSmac);
#if defined(CONFIG_RG_8021X_MAC_TABLE_SIZE) && (CONFIG_RG_8021X_MAC_TABLE_SIZE!=0)
rtk_rg_fwdEngineReturn_t _rtk_rg_dropByAccessDot1x_Check(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr);
#endif
void _rtk_rg_v6NeighborAutoLearned(rtk_rg_pktHdr_t *pPktHdr);
#ifdef CONFIG_MASTER_WLAN0_ENABLE
rtk_rg_successFailReturn_t _rtk_rg_fwdEngine_fromWifiToProtocolStack(struct sk_buff *skb, rtk_rg_mbssidDev_t wlan_dev_idx);
rtk_rg_successFailReturn_t _rtk_rg_checkIngressWifiCopyToPS(struct sk_buff *skb, struct rx_info *pRxDesc, rtk_rg_sw_copyToPSWork_t **pWifiIngressCopyWork, rtk_rg_mbssidDev_t orig_wlan_idx);
rtk_rg_successFailReturn_t _rtk_rg_checkEgressWifiCopyToPS(struct sk_buff *skb, struct rx_info *pRxDesc, rtk_rg_sw_copyToPSWork_t **pWifiEgressCopyWork, rtk_rg_mbssidDev_t wlanDevIdx);
#endif

void _rtk_rg_drv_event_record_timeoutCheck(void);
void rtk_rg_drv_event_record(rtk_rg_drv_event_record_id_t eventIdx);

#ifdef __KERNEL__
//void _rtk_rg_switchLinkChangeHandler(void);

void _rtk_rg_switchLinkChangeHandler(intrBcasterMsg_t *pMsgData);
int _rtk_rg_broadcastForwardWithPkthdr(rtk_rg_pktHdr_t *pPktHdr, struct sk_buff *skb, unsigned int internalVlanID, unsigned int srcPort,unsigned int extSpa);
#endif

#define RG_ONE_COUNT(x)\
do {\
	x = (x & 0x55555555) + ((x & 0xaaaaaaaa) >> 1);\
	x = (x & 0x33333333) + ((x & 0xcccccccc) >> 2);\
	x = (x & 0x0f0f0f0f) + ((x & 0xf0f0f0f0) >> 4);\
    x = (x & 0x00ff00ff) + ((x & 0xff00ff00) >> 8);\
    x = (x & 0x0000ffff) + ((x & 0xffff0000) >> 16);\
} while (0)

typedef struct _rg_DHCPStaticEntry_s
{
	ipaddr_t ip;
	rtk_mac_t mac;
	int valid;
} _rg_DHCPStaticEntry_t;

unsigned int _rtk_rg_hash_mac_fid_efid(unsigned char *mac,unsigned int fid,unsigned int efid);
unsigned int _rtk_rg_hash_mac_vid_efid(unsigned char *mac,unsigned int vid,unsigned int efid);
#if defined(CONFIG_RG_RTL9602C_SERIES)
unsigned int _hash_dip_vidfid_sipidx_sipfilter(int is_ivl,unsigned int dip,unsigned int vid_fid,unsigned int sip_idx,unsigned int sip_filter_en);
unsigned int _hash_ipm_dipv6(uint8 *dip);
#elif defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
unsigned int _rtk_rg_hash_sip_gip(unsigned int sip,unsigned int gip,int isIvl,int vid_fid);
#endif

int32 _rtk_rg_svlMacLookup(rtk_mac_t mac);
#if defined(CONFIG_RG_G3_SERIES)
rtk_rg_err_code_t _rtk_rg_g3_flow_key_mask_init(void);
int32 _rtk_rg_hw_lut_idx_get(uint8 *mac, uint8 isIVL, uint32 vid_fid, uint32 *hwL2Idx);
int _rtk_rg_g3_generic_intf_index_get(uint8 isLanIntf, uint32 lanWan_groupIdx_for_genericIntf, rtk_rg_mac_port_idx_t macPort, rtk_rg_mbssidDev_t wlanDevIdx);
rtk_rg_err_code_t _rtk_rg_generic_lan_intf_add(uint32 intfIdx);
rtk_rg_err_code_t _rtk_rg_generic_lan_intf_del(uint32 intfIdx);
rtk_rg_err_code_t _rtk_rg_generic_wan_intf_add(uint32 intfIdx);
rtk_rg_err_code_t _rtk_rg_generic_wan_intf_del(uint32 intfIdx);
void _rtk_rg_reflash_lut_table(void);
void _rtk_rg_init_mainHash_swData_by_swFlowIdx(uint32 swFlowIdx);
#endif

#if !defined(__KERNEL__)
void _rtk_rg_interfaceVlanIDPriority(rtk_rg_pktHdr_t *pPktHdr,rtk_rg_intfInfo_t *pStoredInfo,struct tx_info *ptxInfo);
void _rtk_rg_interfaceVlanIDPriority_directTX(rtk_rg_pktHdr_t *pPktHdr,rtk_rg_intfInfo_t *pStoredInfo,struct tx_info *ptxInfo);
#else
void _rtk_rg_interfaceVlanIDPriority(rtk_rg_pktHdr_t *pPktHdr,rtk_rg_intfInfo_t *pStoredInfo,rtk_rg_txdesc_t *ptxInfo);
void _rtk_rg_interfaceVlanIDPriority_directTX(rtk_rg_pktHdr_t *pPktHdr,rtk_rg_intfInfo_t *pStoredInfo,rtk_rg_txdesc_t *ptxInfo);
#endif
void _rtk_rg_wanVlanTagged(rtk_rg_pktHdr_t *pPktHdr,int vlan_tag_on);
void _rtk_rg_addPPPoETag(rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb,int l3Modify,int l4Modify);
void _rtk_rg_removeTunnelTag(rtk_rg_pktHdr_t *pPktHdr);
void _rtk_rg_removePPPoETag_disableHwOffload(rtk_rg_pktHdr_t *pPktHdr, uint8 diableHwOffload);
void _rtk_rg_removePPPoETag(rtk_rg_pktHdr_t *pPktHdr);
void _rtk_rg_txPPPoEUpdate(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr);
rtk_rg_fwdEngineReturn_t _rtk_rg_lookupBasedWANForSourceIP(int *pNetIfIdx,ipaddr_t gateway_ipv4_addr);
void _rtk_rg_vlanSvlanTag2SkbBuffer(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr);
int _rtk_rg_arpGeneration(uint8 netIfIdx,ipaddr_t gwIpAddr,rtk_rg_arp_request_t *arpReq);
int _rtk_rg_NDGeneration(uint8 netIfIdx,rtk_ipv6_addr_t gwIpAddr,rtk_rg_neighbor_discovery_t *neighborDisc);
rtk_rg_successFailReturn_t _rtk_rg_arpAndMacEntryAdd(ipaddr_t sip,int sipL3Idx,uint8 *pSmac,int srcPortIdx, int srcWlanDevIdx, int *pL2Idx,int cvid,int cvidForceAdd,int arpEntryForceAdd);
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
int _rtk_rg_naptConnection_add(int naptIdx, rtk_rg_naptOutbound_entry_t *asic_napt, rtk_rg_naptInbound_entry_t *asic_naptr);
#else
int _rtk_rg_naptConnection_add(int naptIdx, rtk_l34_naptOutbound_entry_t *asic_napt, rtk_l34_naptInbound_entry_t *asic_naptr);
#endif
uint32 _rtk_rg_naptTcpUdpOutHashIndex(int8 isTcp, ipaddr_t srcAddr, uint16 srcPort, ipaddr_t destAddr, uint16 destPort);
uint32 _rtk_rg_naptTcpUdpInHashIndex(uint16 isTcp, uint32 dip, uint16 dport);
rtk_rg_lookupIdxReturn_t _rtk_rg_naptTcpUdpOutHashIndexLookup(int8 isTcp, ipaddr_t srcAddr, uint16 srcPort, ipaddr_t destAddr, uint16 destPort);
rtk_rg_lookupIdxReturn_t _rtk_rg_naptTcpUdpOutHashIndexLookupByPktHdr(int8 isTcp, rtk_rg_pktHdr_t *pPktHdr);
rtk_rg_lookupIdxReturn_t _rtk_rg_naptTcpUdpInHashIndexLookup(int8 isTcp, ipaddr_t remoteAddr, uint16 remotePort, ipaddr_t extAddr, uint16 extPort);
rtk_rg_entryGetReturn_t _rtk_rg_swNaptOutFreeEntryGet(int naptHashOutIdx);
rtk_rg_entryGetReturn_t _rtk_rg_swNaptInFreeEntryGet(int naptHashInIdx);

uint8 _rtk_rg_IPv6NeighborHash(const uint8 *ifid, uint8 rtidx);
rtk_rg_successFailReturn_t _rtk_rg_neighborAndMacEntryAdd(unsigned char *sip,int sipOrDipL3Idx,uint8 *pSmac,int srcPortIdx,int srcWlanDevIdx,int *pNeighborOrMacIdx);
rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_naptPacketModify(int direct, int naptIdx, rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb,int l3Modify,int l4Modify);
rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_ipv6PacketModify(rtk_rg_naptDirection_t direct, rtk_l34_nexthop_type_t intfType,rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb);


int32 _rtk_rg_addArpRoutingArray(rtk_rg_routing_arpInfo_t *newAddingEntry, ipaddr_t newIpAddr, int intfVlanId);
int32 _rtk_rg_delArpRoutingArray(rtk_rg_routing_arpInfo_t *deletingEntry);
int32 _rtk_rg_arpRearrange(rtk_rg_routing_arpInfo_t *newAddingEntry, ipaddr_t newIpAddr, int routingARPNum);

rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngineDirectTx(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr);
int _rtk_rg_fwdengine_handleArpMiss(rtk_rg_pktHdr_t *pPktHdr);
int _rtk_rg_fwdengine_handleArpMissInRoutingLookUp(rtk_rg_pktHdr_t *pPktHdr);
rtk_rg_successFailReturn_t _rtk_rg_fwdEngine_upnpCheck(void*,ipaddr_t*,uint16*);
rtk_rg_successFailReturn_t _rtk_rg_fwdEngine_virtualServerCheck(void*,ipaddr_t*,uint16*);
rtk_rg_successFailReturn_t _rtk_rg_fwdEngine_dmzCheck(void*,ipaddr_t*,uint16*);
rtk_rg_successFailReturn_t _rtk_rg_fwdEngine_portTriggerCheck(void*,ipaddr_t*,uint16*);
void _rtk_rg_portTriggerManipulateRelateList(int is_Add, int inIdx, int outIdx);
int _rtk_rg_redirectGeneration(rtk_rg_pktHdr_t *pPktHdr, int type_idx, char *url_string);
int _rtk_rg_eiplookup(ipaddr_t ip);
int _rtk_rg_ipv4MatchBindingDomain(ipaddr_t ip, int bindNextHopIdx);
int _rtk_rg_softwareArpTableLookUp(unsigned short routingIdx, ipaddr_t ipAddr, rtk_rg_arp_linkList_t **pSoftwareArpEntry, int resetIdleTime);
#if defined(CONFIG_RG_RTL9602C_SERIES)
int _rtk_rg_hardwareArpTableLookUp(unsigned short routingIdx, ipaddr_t ipAddr, rtk_rg_arp_linkList_t **pSoftwareArpEntry, int resetIdleTime);
int _rtk_rg_hardwareArpTableAdd(unsigned short routingIdx, ipaddr_t ipv4Addr, int l2Idx, int staticEntry, uint16 *hwArpIdx);
int _rtk_rg_hardwareArpTableDel(rtk_rg_arp_linkList_t *pDelArpEntry);
#endif
#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support lut traffic bit
int _rtk_rg_lutCamListAdd(int l2Idx, uint32 addLutCamIdx);
int _rtk_rg_lutCamListDel(uint32 delLutCamIdx);
#endif

int _rtk_rg_lutGroupListAdd(uint32 lutGroupIdx, uint32 addL2Idx);
int _rtk_rg_lutGroupListDelAndInit(uint32 delL2Idx);
void _rtk_rg_moveLutGroup(uint32 new_lutGroupIdx, uint32 old_lutGroupIdx);
#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support lut traffic bit
int _rtk_rg_update_lutIdleTime(uint32 l2Idx);
#endif
uint32 _rtk_rg_isInLutGroup(uint32 lutGroupIdx, uint32 l2Idx);

void _rtk_rg_lutToMacEntry_translator(rtk_rg_table_lut_t lutEntry, rtk_rg_macEntry_t *macEntry);
rtk_rg_err_code_t _rtk_rg_ivlMacEntry_add(uint32 svlL2Idx, uint32 ivlVlanId, rtk_rg_port_idx_t ingressPort, rtk_rg_mbssidDev_t wlan_dev_idx, uint32 *ivlL2Idx);
#if defined(CONFIG_RG_RTL9602C_SERIES)
rtk_rg_err_code_t _rtk_rg_create_ivlMacEntries_of_internalUsedVid(uint32 svlL2Idx, uint32 ingressVid, rtk_rg_port_idx_t ingressPort, rtk_rg_mbssidDev_t wlan_dev_idx);
rtk_rg_err_code_t _rtk_rg_update_lanIntf_ivlMacEntries(void);
#endif
int32 _rtk_rg_apollo_naptFilterAndQos_init(void);
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
void _rtk_rg_fwdEngine_ipv6ConnList_del(rtk_rg_ipv6_layer4_linkList_t *pConnList);
#endif

int _rtk_rg_dot1pPriRemarking2InternalPri_search(int target_rmk_dot1p, int *duplicateSize,int* intPri);
int32 _rtk_rg_cvidCpri2Sidmapping_get(uint16 cvid, uint8 cpri, uint8 *sid);

//sw rate limit
void _rtk_rg_renew_sw_rate_timer(void);
rtk_rg_fwdEngineReturn_t _rtk_rg_unlearnedSARateLimit_check(void);

rtk_rg_fwdEngineReturn_t _rtk_rg_overMTURateLimit_check(struct sk_buff *skb,rtk_rg_pktHdr_t *pPktHdr);

rtk_rg_fwdEngineReturn_t _rtk_rg_unknownDARateLimit_check(struct sk_buff *skb,rtk_rg_pktHdr_t *pPktHdr);

rtk_rg_fwdEngineReturn_t _rtk_rg_BCMCRateLimit_check(struct sk_buff *skb,rtk_rg_pktHdr_t *pPktHdr);

rtk_rg_fwdEngineReturn_t _rtk_rg_naptSwRateLimit_check(rtk_rg_pktHdr_t *pPktHdr, int limit_naptFilter_idx);
rtk_rg_fwdEngineReturn_t _rtk_rg_swDoS_check(rtk_rg_pktHdr_t *pPktHdr);
int _rtk_rg_fwdEngine_ipv6DmzCheck(void *data,rtk_ipv6_addr_t *transIP,uint16 *transPort);
int _rtk_rg_fwdEngine_ipv6VirtualServerCheck(void *data,rtk_ipv6_addr_t *transIP,uint16 *transPort);



//Multicast
//1 FIXME: this declaration should be delete when the liteRomeDriver.h is ready!!
void _rtk_rg_apollo_ipmcMultiCast_transTbl_add(int32 isIpv6,rtk_l2_ipMcastAddr_t *lut);
int32 _rtk_rg_apollo_ipv4MultiCastFlow_add(rtk_rg_ipv4MulticastFlow_t *ipv4McFlow,int *flow_idx);
int32 _rtk_rg_apollo_l2MultiCastFlow_add(rtk_rg_l2MulticastFlow_t *l2McFlow,int *flow_idx);
rtk_rg_entryGetReturn_t _rtk_rg_findAndReclamL2mcEntry(rtk_mac_t *mac,int fid);
rtk_rg_entryGetReturn_t _rtk_rg_findAndReclamL2mcEntryIVL(rtk_mac_t *mac,int vid);
#if defined(CONFIG_RG_RTL9602C_SERIES)
rtk_rg_entryGetReturn_t _rtk_rg_findAndReclamIPv6mcEntry( rtk_ipv6_addr_t *gipv6,int32 ivlsvl);
#endif
rtk_rg_entryGetReturn_t _rtk_rg_findAndReclamIpmcEntry(ipaddr_t sip,ipaddr_t dip,int32 ivlsvl,int32 vid_fid,int32 ipfilterIdx,int32 ipfilterEn);

//IGMP
int32 rtl_flushAllIgmpRecord(int forceFlush);
int _rtk_rg_igmpSnoopingOnOff(int isOn, int onlyChangeTimer, int isIVL);



//ALG
int _rtk_rg_algCheckEnable(unsigned char isTCP,unsigned short checkPort);
rtk_rg_successFailReturn_t _rtk_rg_algSrvInLanCheckEnable(unsigned char isTCP, unsigned short checkPort);
rtk_rg_successFailReturn_t _rtk_rg_algDynamicPort_set(p_algRegisterFunction registerFunction, unsigned int serverInLan, ipaddr_t intIP, unsigned short int portNum, int isTCP, int timeout);
int32 dump_isakmpDB(struct seq_file *s, void *v);
void _rtk_rg_algPreservePort_delete(uint16 portNum, int isTcp, uint32 internalIp, uint16 internalPort);
rtk_rg_successFailReturn_t _rtk_rg_algPreservePort_set(uint16 *pPortNum, int isTCP, uint32 internalIp, uint16 internalPort, int timeout);

//WIFI
void _rtk_rg_wlanMbssidLearning(u8* smac,rtk_rg_pktHdr_t *pPktHdr);
rtk_rg_lookupIdxReturn_t _rtk_rg_wlanMbssidLookup(u8 *dmac,rtk_rg_mbssidDev_t *wlan_dev_idx);
rtk_rg_mbssidDev_t _rtk_master_wlan_mbssid_tx(rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb);

//TOOL
void _rtk_rg_str2mac(unsigned char *mac_string,rtk_mac_t *pMacEntry);
unsigned short inet_chksum(unsigned char *dataptr, unsigned short len);
u16 inet_chksum_pseudo(u8 *tcphdr, u16 tcplen, u32 srcip, u32 destip,  u8 proto);
u16 inet_chksum_pseudoV6(u8 *tcphdr, u16 tcplen, u8 *srcip, u8 *destip,  u8 proto);
int _rtk_rg_pasring_proc_string_to_integer(const char *buff,unsigned long len);
void _rtk_rg_tcpShortTimeoutHouseKeep_set(uint32 jiffies_interval);
void _rtk_rg_tcpShortTimeoutHouseKeepingHandler(int outIdx);
void _rtk_rg_copyWork_handler(struct work_struct *work);


//DEBUG
int _rtk_rg_trace_filter_compare(struct sk_buff *skb,rtk_rg_pktHdr_t *pPktHdr);



//NIC
struct sk_buff *re8670_getAlloc(unsigned int size);
struct sk_buff *re8670_getBcAlloc(unsigned int size);
struct sk_buff *re8670_getMcAlloc(unsigned int size);
#if defined(CONFIG_RG_SUPPORT_JUMBO_FRAME)
struct sk_buff *re8670_getJumboAlloc(unsigned int size);
#endif

void _rtk_rg_splitJumboSendToNicWithTxInfo(rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb, struct tx_info* ptxInfo, int ring_num);
rtk_rg_fwdEngineReturn_t _rtk_rg_splitJumboSendToMasterWifi(rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb, struct net_device *dev);
void _rtk_rg_wifiTxRedirect(struct sk_buff *skb,struct net_device *netdev);

rtk_rg_err_code_t _rtk_rg_shareMeterConf_get(uint32 index, rtk_rg_meterConf_t **meterConf);
rtk_rg_fwdEngineReturn_t _rtk_rg_dropBySwRateLimit_check(uint32 meterIndex, uint32 byteCount, uint32 packetCount, rtk_rg_meter_type_t funcbasedMeterType);


void _rtk_rg_updateMibInfo(rtk_rg_pktHdr_t *pPktHdr, struct sk_buff *skb);
void _rtk_rg_updateMib(rtk_rg_mibUpdate_entry_t mibUpdateInfo);
void _rtk_rg_nic_tx(struct sk_buff *skb, struct tx_info* ptxInfo, int ring_num, rtk_rg_port_idx_t ingressPort, rtk_rg_mibUpdate_entry_t mibUpdateInfo);
void _rtk_rg_wifi_tx(struct sk_buff *skb, struct net_device *dev, rtk_rg_mibUpdate_entry_t mibUpdateInfo);
rtk_rg_fwdEngineReturn_t rtk_rg_sw_shaper_rate_couting(uint32 swMeterIdx, uint32 skbLen, uint8 reschedule);
void rtk_rg_sw_shaper_kicktx_timerfunc(unsigned long sw_meter_Idx);
rtk_rg_fwdEngineReturn_t rtk_rg_sw_shaper(bool wifiTx, void *pTxPriv, uint32 swMeterIdx);

//SKB
void _rtk_rg_dev_kfree_skb_any(struct sk_buff *skb);
//struct sk_buff *_rtk_rg_dev_alloc_skb(unsigned int length);		//deprecated, please use pre-allocated skb API "_rtk_rg_getAlloc" !!
#if defined(CONFIG_RG_SUPPORT_JUMBO_FRAME)
struct sk_buff *_rtk_rg_getJumboAlloc(unsigned int size);
#endif
struct sk_buff *_rtk_rg_getAlloc(unsigned int size);
struct sk_buff *_rtk_rg_skb_copy(const struct sk_buff *skb, unsigned gfp_mask);	//deprecated, use this carefully!!
struct sk_buff *_rtk_rg_skb_clone(struct sk_buff *skb, unsigned gfp_mask);		//deprecated, use this carefully!!
struct sk_buff *rtk_rg_skbCopyToPreAllocSkb(struct sk_buff *skb);




#ifdef TIMER_AGG
void _rtk_rg_del_timer(rtk_rg_timer_t *pRgTimer);
void _rtk_rg_init_timer(rtk_rg_timer_t *pRgTimer);
void _rtk_rg_mod_timer(rtk_rg_timer_t *pRgTimer,unsigned long next_jiffies);
int _rtk_rg_timer_pending(rtk_rg_timer_t *pRgTimer);
#else
void _rtk_rg_del_timer(struct timer_list *timer);
void _rtk_rg_init_timer(struct timer_list *timer);
void _rtk_rg_mod_timer(struct timer_list *timer, unsigned long expires);
int _rtk_rg_timer_pending(struct timer_list *timer);
#endif



//USERSPACE HELPER
/* used for command which need to pipe to file, ex: >  >>*/
int rtk_rg_callback_pipe_cmd(const char *comment,...);

uint16 _rtk_rg_fwdengine_ICMPchecksumUpdate(uint16 ori_checksum, uint32 ori_ip, uint16 ori_port, uint16 ori_ipchecksum, uint16 ori_l4checksum, uint32 new_ip, uint16 new_port, uint16 new_ipchecksum, uint16 new_l4checksum);
uint16 _rtk_rg_fwdengine_L4checksumUpdate(uint8 acktag, uint16 ori_checksum, uint32 ori_ip, uint16 ori_port, uint32 ori_seq, uint32 ori_ack, uint32 new_ip, uint16 new_port, uint32 new_seq, uint32 new_ack);
uint16 _rtk_rg_fwdengine_UDPchecksumUpdate(uint8 acktag, uint16 ori_checksum, uint32 ori_ip, uint16 ori_port, uint32 ori_seq, uint32 ori_ack, uint32 new_ip, uint16 new_port, uint32 new_seq, uint32 new_ack);
uint16 _rtk_rg_fwdengine_L4checksumUpdateForMss(uint16 ori_checksum, uint16 ori_mss, uint16 new_mss);
#define _rtk_rg_fwdengine_L4checksumUpdateForPPTPCallID _rtk_rg_fwdengine_L4checksumUpdateForMss
uint16 _rtk_rg_fwdengine_L3checksumUpdate(uint16 ori_checksum, uint32 ori_sip, uint8 ori_ttl, uint8 ori_protocol, uint32 new_sip, uint8 new_ttl);
uint16 _rtk_rg_fwdengine_L3checksumUpdateDSCP(uint16 ori_checksum, uint8 header_length, uint8 ori_DSCP, uint8 new_DSCP);
uint16 _rtk_rg_fwdengine_L3checksumUpdateTotalLen(uint16 ori_checksum, uint16 ori_len, uint16 new_len);

#if defined(CONFIG_RG_NAPT_NEW_EXTPORT_MECHANISM)
int32 _rtk_rg_napt_extPort_record_hashIdx_get(uint32 internalIp, uint16 internalPort);
rtk_rg_err_code_t _rtk_rg_napt_extPort_record_find(uint8 force, uint8 isTcp, uint32 internalIp, uint16 internalPort, uint16 *pExtPort);
rtk_rg_err_code_t _rtk_rg_napt_extPort_record_add(uint8 force, uint8 isTcp, uint32 internalIp, uint16 internalPort, uint16 extPort);
rtk_rg_err_code_t _rtk_rg_napt_extPort_record_del(uint8 force, uint8 isTcp, uint32 internalIp, uint16 internalPort, uint16 extPort);
#endif
rtk_rg_successFailReturn_t _rtk_rg_mac_learning_limit_check(rtk_rg_port_idx_t portIdx, int32 wlan_device_idx);
rtk_rg_err_code_t _rtk_rg_mac_learning_limit_count_inc(rtk_rg_port_idx_t portIdx, int32 wlan_device_idx);
rtk_rg_err_code_t _rtk_rg_mac_learning_limit_count_dec(rtk_rg_port_idx_t portIdx, int32 wlan_device_idx);

int _check_urlFilter_is_enabled(rtk_rg_pktHdr_t *pPktHdr);
uint8 _rtk_rg_lut_is_sw_static(uint32 l2Idx);

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
int nic_rx_skb_NPTv6FastForward(struct re_private *cp, struct rx_info *pRxInfo, int len);
rtk_rg_fwdEngineReturn_t _rtk_rg_nptv6_acceleration_fail_to_do_hwlookup_check(rtk_rg_pktHdr_t *pPktHdr);
void _rtk_rg_nptv6_acceleration_hwlookup_check(rtk_rg_pktHdr_t *pPktHdr);
int vxlan_customized_rxPrepare(struct re_private *cp, struct rx_info *pRxInfo);
int vxlan_customized_txPrepare(struct re_private *cp, struct tx_info *pTxInfo);
int vxlan_customized_extraTxPrepare(struct re_private *cp, struct tx_info *pTxInfo);
int vxlan_customized_rxHook(struct re_private *cp, struct rx_info *pRxInfo, int len);
int vxlan_customized_upstreamTxHook(struct re_private *cp, struct tx_info *pTxInfo, int len);
void _rtk_rg_vxlanInNaptShortcutUpdate(rtk_rg_pktHdr_t *pPktHdr);
#endif

#if 1//ndef CONFIG_APOLLO_TESTING

#ifdef __KERNEL__
#include <linux/if_vlan.h>
#include <linux/skbuff.h>


//VLAN TAG
#define VLAN_HLEN 4
#define VLAN_ETH_ALEN 6
#define IPv4_HLEN 20
#define UDP_HLEN 8
#define PROTOCAL_LEN 2

#define IP6_HLEN	40

struct vlan_double_ethhdr {
	unsigned char	h_dest[ETH_ALEN];
	unsigned char	h_source[ETH_ALEN];
	__be16		h_outter_vlan_proto;
	__be16		h_outter_vlan_content;
	__be16		h_inner_vlan_proto;
	__be16		h_inner_vlan_content;
	__be16		h_vlan_encapsulated_proto;
};

struct ipv4_ethhdr {
	__be16	h_ver_ihl_tos;
	__be16	h_length;
	__be16	h_id;
	__be16	h_flags_fragoff;
	__be16	h_ttl_proto;
	__be16	h_checksum;
	__be32	h_sip;
	__be32	h_dip;
};

struct ipv6_ethhdr {
	__be16	h_ver_tc_flowlabel;
	__be16	h_flowlabel;
	__be16	h_payloadlen;
	__be16	h_nh_hp;
	__be32	h_sip[4];
	__be32	h_dip[4];
};

struct udp_ethhdr {
	__be16	h_sport;
	__be16	h_dport;
	__be16	h_length;
	__be16	h_checksum;
};



/**
 * _vlan_insert_tag - regular VLAN/SVLAN tag inserting
 * @skb: skbuff to tag
 * @outter_tagif: insert outterTag or not
 * @outter_protocal: VLAN protocal, usually 0x8100 or ox88a8
 * @outter_content: VLAN content, usually is ((vlan_pri<<13)|(vlan_tci<<12)|vlan_id)
 * @inner_tagif: insert innerTag or not (only supported while outter_tagif is valid)
 * @inner_protocal: VLAN protocal, usually 0x8100
 * @inner_content: VLAN content, usually is ((vlan_pri<<13)|(vlan_tci<<12)|vlan_id)
 *
 * Inserts the VLAN tag into @skb as part of the payload(at most add double vlan tags)
 * Returns a VLAN tagged skb. If a new skb is created, @skb is freed.
 *
 * Following the skb_unshare() example, in case of error, the calling function
 * doesn't have to worry about freeing the original skb.
 */
 static inline struct sk_buff *_vlan_insert_tag(rtk_rg_pktHdr_t *pPktHdr, struct sk_buff *skb,u16 outter_tagif,u16 outter_protocal,u16 outter_content,u16 inner_tagif,u16 inner_protocal,u16 inner_content)
{
	struct vlan_double_ethhdr *veth;
	int num_of_tag=0;


	if(outter_tagif){ //outter_tag must assign first
		num_of_tag++;

		if(inner_tagif)
			num_of_tag++;
	}

	if((skb_tail_pointer(skb) - skb->data)!=skb->len)
		WARNING("[skb error] skb->head=0x%x, skb->data=0x%x, skb->tail=0x%x, skb->end=0x%x, skb->len=%d\n",
		skb->head, skb->data, skb->tail, skb->end, skb->len);
	if (skb_cow_head(skb, num_of_tag*VLAN_HLEN) < 0) {
		WARNING("skb head room is not enough..return without insert tag");
		rtk_rg_drv_event_record(RTK_RG_DRV_EVENT_REC_VLAN_INSERT_FAIL);
		return NULL;
	}

	veth = (struct vlan_double_ethhdr *)skb_push(skb, num_of_tag*VLAN_HLEN);

	/* Move the mac addresses to the beginning of the new header. */
	memmove(skb->data, skb->data + num_of_tag*VLAN_HLEN, 2 * VLAN_ETH_ALEN);
	skb->mac_header -= num_of_tag*VLAN_HLEN;

	if(outter_tagif){
		/* first, the outter prtocal type */
		veth->h_outter_vlan_proto = htons(outter_protocal);
		/* now, the outter content */
		veth->h_outter_vlan_content = htons(outter_content);

		skb->protocol = htons(outter_protocal);

		if(inner_tagif){
			/* the inner prtocal type */
			veth->h_inner_vlan_proto = htons(inner_protocal);
			/*the inner  content*/
			veth->h_inner_vlan_content = htons(inner_content);
		}
	}

	if(pPktHdr!=NULL){
		pPktHdr->l3Offset += (num_of_tag*VLAN_HLEN);
		pPktHdr->l4Offset += (num_of_tag*VLAN_HLEN);
		pPktHdr->outer_l3Offset += (num_of_tag*VLAN_HLEN);
	}

	return skb;
}

/**
 * _vlan_remove_tag - regular VLAN/SVLAN tag remove
 * @skb: skbuff to tag
 * @protocal: target VLAN protocal, usually 0x8100 or ox88a8
 *
 * Remove the VLAN tag payload from @skb
 * Returns a tag-removed skb.
 */
static inline struct sk_buff *_vlan_remove_tag(rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb, u16 protocal)
{
	if ((skb==NULL) || (skb->data==NULL) ) {
		return NULL;
	}

	//check the protocal is the same
	if((skb->data[2*VLAN_ETH_ALEN]<<0x8 | skb->data[2*VLAN_ETH_ALEN+1] )!= protocal){
		WARNING("Assigned Tag protocal is not found, removing tag failed.");
		rtk_rg_drv_event_record(RTK_RG_DRV_EVENT_REC_VLAN_REMOVE_FAIL);
		return skb;
	}

	/* Move the mac addresses to the beginning of the new header. */
	//memmove(skb->data, skb->data - VLAN_HLEN, 2 * VLAN_ETH_ALEN);
	memmove(skb->data+VLAN_HLEN, skb->data, 2 * VLAN_ETH_ALEN);
	skb->mac_header += VLAN_HLEN;
	skb->data += VLAN_HLEN;
	skb->len  -= VLAN_HLEN;

	if(pPktHdr!=NULL){
		skb->protocol = pPktHdr->etherType;	//when we remove the vlan tag, we should recover skb->protocol from etherType
		if(pPktHdr->l3Offset!=0) //pPktHdr->l3Offset has been parsed
			pPktHdr->l3Offset -= VLAN_HLEN;
		if(pPktHdr->l4Offset!=0) //pPktHdr->l4Offset has been parsed
			pPktHdr->l4Offset -= VLAN_HLEN;
		if(pPktHdr->outer_l3Offset!=0)
			pPktHdr->outer_l3Offset -= VLAN_HLEN;
	}

	return skb;
}

/**
 * _vlan_remove_doubleTag - regular SVLAN and VLAN tag remove (can be using in fwEngine)
 * @skb: skbuff to tag (must have Stag and Ctag)
 *
 * Remove the VLAN tag payload from @skb
 * Returns a tag-removed skb.
 */
static inline struct sk_buff *_vlan_remove_doubleTag(rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb)
{
	if (skb==NULL || skb->data==NULL ) {
		return NULL;
	}

	//check the outter tag protocal
	if((skb->data[2*VLAN_ETH_ALEN]<<0x8 | skb->data[2*VLAN_ETH_ALEN+1] )!= rg_db.systemGlobal.tpid){
		WARNING("stag protocal is not found, removing tag failed.");
		rtk_rg_drv_event_record(RTK_RG_DRV_EVENT_REC_VLAN_REMOVE_FAIL);
		return skb;
	}
	//check the inner tag protocal
	if((skb->data[2*VLAN_ETH_ALEN+VLAN_HLEN]<<0x8 | skb->data[2*VLAN_ETH_ALEN+VLAN_HLEN+1] )!= 0x8100){
		WARNING("ctag protocal is not found, removing tag failed.");
		rtk_rg_drv_event_record(RTK_RG_DRV_EVENT_REC_VLAN_REMOVE_FAIL);
		return skb;
	}

	/* Move the mac addresses to the beginning of the new header. */
	memmove(skb->data+(VLAN_HLEN*2), skb->data, 2 * VLAN_ETH_ALEN);
	skb->mac_header += (VLAN_HLEN*2);
	skb->data += (VLAN_HLEN*2);
	skb->len  -= (VLAN_HLEN*2);

	if(pPktHdr!=NULL){
		skb->protocol = pPktHdr->etherType;	//when we remove the vlan tag, we should recover skb->protocol from etherType
		if(pPktHdr->l3Offset!=0) //pPktHdr->l3Offset has been parsed
			pPktHdr->l3Offset -= (VLAN_HLEN*2);
		if(pPktHdr->l4Offset!=0) //pPktHdr->l4Offset has been parsed
			pPktHdr->l4Offset -= (VLAN_HLEN*2);
		if(pPktHdr->outer_l3Offset!=0)
			pPktHdr->outer_l3Offset -= (VLAN_HLEN*2);
	}

	return skb;
}


/**
 * _vlan_modify_tag - outter modify, usually using in VLAN<=>SVLAN exchange
 * @skb: skbuff to tag
 * @ori_protocal: target VLAN protocal for checking is tag exist.
 * @mod_protocal: target VLAN protocal, usually 0x8100 or ox88a8
 * @mod_content: target VLAN content, usually is ((vlan_pri<<13)|(vlan_tci<<12)|vlan_id)
 *
 * Modify the VLAN tag payload from @skb
 * Returns a modified skb.
 */
static inline struct sk_buff *_vlan_modify_tag(rtk_rg_pktHdr_t *pPktHdr, struct sk_buff *skb, u16 ori_protocal,u16 mod_protocal,u16 mod_content)
{
	//struct vlan_ethhdr *veth;
	//u16 vlan_pattern;

	if (skb==NULL || skb->data==NULL ) {
		return NULL;
	}

	//check the protocal is the same
	if((skb->data[2*VLAN_ETH_ALEN]<<0x8 | skb->data[2*VLAN_ETH_ALEN+1] )!= ori_protocal){
		WARNING("Assigned Tag protocal is not found, modify tag failed.");
		rtk_rg_drv_event_record(RTK_RG_DRV_EVENT_REC_VLAN_MODIFY_FAIL);
		return skb;
	}


	//modified protocal
	(*(u16*)(skb->data+2*VLAN_ETH_ALEN))=htons(mod_protocal);

	//modified content
	(*(u16*)(skb->data+2*VLAN_ETH_ALEN+2))=htons(mod_content);


	return skb;
}

/**
 * _vlan_modify_doubleTag - outter & inner tag modify
 * @skb: skbuff to tag
 * @mod_outter_protocal: target SVLAN protocal, usually 0x8100 or ox88a8
 * @mod_outter_content: target SVLAN content, usually is ((svlan_pri<<13)|(svlan_dei<<12)|svlan_id)
 * @mod_inner_protocal: target VLAN protocal, usually 0x8100
 * @mod_inner_content: target VLAN content, usually is ((vlan_pri<<13)|(vlan_tci<<12)|vlan_id)
 *
 * Modify the VLAN tag payload from @skb
 * Returns a modified skb.
 */
static inline struct sk_buff *_vlan_modify_doubleTag(rtk_rg_pktHdr_t *pPktHdr, struct sk_buff *skb, u16 mod_outter_protocal,u16 mod_outter_content,u16 mod_inner_protocal,u16 mod_inner_content)
{
	//struct vlan_ethhdr *veth;
	//u16 vlan_pattern;

	if (skb==NULL || skb->data==NULL ) {
		return NULL;
	}

	//check the outter tag protocal
#if defined(CONFIG_RG_RTL9602C_SERIES)
	if((skb->data[2*VLAN_ETH_ALEN]<<0x8 | skb->data[2*VLAN_ETH_ALEN+1] )!= pPktHdr->stagTpid){
#else //9600 series
	if((skb->data[2*VLAN_ETH_ALEN]<<0x8 | skb->data[2*VLAN_ETH_ALEN+1] )!= rg_db.systemGlobal.tpid){
#endif
		WARNING("Assigned stag protocal is not found, removing tag failed.");
		rtk_rg_drv_event_record(RTK_RG_DRV_EVENT_REC_VLAN_MODIFY_FAIL);
		return skb;
	}
	//check the inner tag protocal
	if((skb->data[2*VLAN_ETH_ALEN+VLAN_HLEN]<<0x8 | skb->data[2*VLAN_ETH_ALEN+VLAN_HLEN+1] )!= 0x8100){
		WARNING("Assigned ctag protocal is not found, removing tag failed.");
		rtk_rg_drv_event_record(RTK_RG_DRV_EVENT_REC_VLAN_MODIFY_FAIL);
		return skb;
	}



	//modified outter protocal
	(*(u16*)(skb->data+2*VLAN_ETH_ALEN))=htons(mod_outter_protocal);

	//modified outter content
	(*(u16*)(skb->data+2*VLAN_ETH_ALEN+2))=htons(mod_outter_content);

	//modified inner protocal
	(*(u16*)(skb->data+2*VLAN_ETH_ALEN+VLAN_HLEN))=htons(mod_inner_protocal);

	//modified inner content
	(*(u16*)(skb->data+2*VLAN_ETH_ALEN+VLAN_HLEN+2))=htons(mod_inner_content);


	return skb;
}



/**
 * _insert_IP_UDP_tag - specific IP/UDP tag inserting
 * @skb: skbuff to tag
 * @encap_info: encapulate structure
 *
 * Inserts the IP/UDP tag into @skb, and consider original IP/UDP as part of the payload
 * Returns a inserted IP/UDP header skb. If a new skb is created, @skb is freed.
 *
 * Following the skb_unshare() example, in case of error, the calling function
 * doesn't have to worry about freeing the original skb.
 */
static inline struct sk_buff *_insert_IP_UDP_tag(rtk_rg_pktHdr_t *pPktHdr, struct sk_buff *skb,rtk_rg_mirror_udp_encap_tag_action_t *encap_info)
{
	uint8 debug_print = FALSE;
	struct ipv4_ethhdr *ins_ip_h = NULL;
	struct ipv6_ethhdr *ins_ip6_h = NULL;
	struct udp_ethhdr *ins_udp_h = NULL;
	u8 *pData=skb->data;
	uint8 ipv6_encap = FALSE;	//insert ipv6 header + udp header
	uint8 encap_ip_len = 0;
	uint8 encap_tcp_rule_idx_len = 0;

	int da_sa_offset = 12; //DA + SA from skb->data
	int da_sa_prorotcal_offset = 14; //DA + SA + protocal  from skb->data
	int tags_before_ip_header_size;
	int pushed_room_size = 0;
	int l3_offset, l4_offset;

	ipv6_encap = encap_info->encap_ip_ipv6?TRUE:FALSE;
	if(ipv6_encap){
		encap_ip_len = IP6_HLEN;	//new_IP_header(40)
	}else{
		encap_ip_len = IPv4_HLEN;		//new_IP_header(20)
	}
	pushed_room_size = (encap_ip_len+UDP_HLEN); //insert header [new_IP_header + new_UDP_header(8)]

	if(rg_db.systemGlobal.acl_debug_udp_encap_packet_dump && ((rg_kernel.filter_level == 0x0) || (rg_kernel.tracefilterShow)))
		debug_print = TRUE;	//only dump packet when <proc enable> AND <not configure trace filter OR hit trace filter>

	if(pPktHdr==NULL){
		WARNING("pPktHdr is NULL ... return without insert tag");
		return NULL;
	}

	l3_offset=pPktHdr->l3Offset;
	l4_offset=pPktHdr->l4Offset;
	tags_before_ip_header_size = (l3_offset - da_sa_prorotcal_offset); //all tag before IP Header (ex:cvlan, pppoe tag) size, which will be encap into new IP/UDP header*/

	if((skb_tail_pointer(skb) - skb->data)!=skb->len)
		WARNING("[skb error] skb->head=0x%x, skb->data=0x%x, skb->tail=0x%x, skb->end=0x%x, skb->len=%d\n",
		skb->head, skb->data, skb->tail, skb->end, skb->len);


	if (skb_cow_head(skb, pushed_room_size) < 0) {
		WARNING("skb head room is not enough..return without insert tag");
		return NULL;
	}


	//step-prepare: original packet already send to destination, force removed pppoe/cvlan header before mirror to PS.
	if(tags_before_ip_header_size > 0)
	{
		u32 i;
		u16 total_len;
		if(debug_print)
			dump_packet(skb->data,skb->len,"mirror packet before remove Tags");


		if(pPktHdr->tagif&IPV6_TAGIF)
		{
			total_len=l3_offset+pPktHdr->l3Len;
			skb->data[l3_offset-2]=0x86;
			skb->data[l3_offset-1]=0xdd;
		}
		else
		{
			total_len=l3_offset+pPktHdr->l3Len;
			skb->data[l3_offset-2]=0x08;
			skb->data[l3_offset-1]=0x00;
		}

		//DEBUG("%s...total len is %d, tag before ip header %d",pPktHdr->tagif&IPV6_TAGIF?"IPV6":"IPv4",total_len, tags_before_ip_header_size);
		//remove pppoe/cvlan header here
		//dump_packet(skb->data,skb->len,"tags_del_before");

		//copy new DA/SA
		for(i=(l3_offset-1)-2/*keep new protocol*/;i>=tags_before_ip_header_size;i--)
			skb->data[i]=skb->data[i-tags_before_ip_header_size];
		//dump_packet(skb->data,skb->len,"tags_del_ongoing: copy new DA/SA");

		skb->data+=tags_before_ip_header_size;
		skb->len=total_len-tags_before_ip_header_size;
		skb_reset_mac_header(skb);

		//DEBUG("after remove pppoe/cvlan header, tot_len=%d l3off=%d l3len=%d l4off=%d",total_len,pPktHdr->l3Offset,pPktHdr->l3Len,pPktHdr->l4Offset);
		//dump_packet(skb->data,skb->len,"tags_del_after");

		//reset DMAC and SMAC pointer
		//pPktHdr->pDmac=&skb->data[0];
		//pPktHdr->pSmac=&skb->data[6];

		if(l4_offset>0)
			l4_offset-=tags_before_ip_header_size;
		if(l3_offset>0)
			l3_offset-=tags_before_ip_header_size;
		tags_before_ip_header_size = 0;

		//move the dmac/smac pointer.
		pData=skb->data;

		if(debug_print)
			dump_packet(skb->data,skb->len,"mirror packet after remove Tags");

	}
	
	if(pPktHdr->tagif&TCP_TAGIF){
		if(encap_info->tcp_header_only){
			//cut off tcp payload
			//DEBUG("tcp_header_only!!!! skb->len from %d to %d!!!!!!!!",skb->len,l4_offset+pPktHdr->headerLen);
			skb_trim(skb,l4_offset+pPktHdr->headerLen);
		}
		if(encap_info->tcp_rule_append && skb_tailroom(skb)>=2){
			//append rule idx in the end of skb
			//DEBUG("append rule idx!!!!");dump_packet(skb->data,skb->len,"mirror packet before append");
			*(unsigned short *)(skb_tail_pointer(skb))=htons(encap_info->tcp_rule_idx);
			skb_put(skb,2);//dump_packet(skb->data,skb->len,"mirror packet after append");
			encap_tcp_rule_idx_len=2;
		}
	}

	if(debug_print){
		dump_packet(skb->data,skb->len,"original packet");
	}

	/*step0: remarking the DA', SA' based on ACL action*/
	//remark dmac if assign value
	if((encap_info->encap_dmac.octet[0] || encap_info->encap_dmac.octet[1] || encap_info->encap_dmac.octet[2] ||
		encap_info->encap_dmac.octet[3]|| encap_info->encap_dmac.octet[4] || encap_info->encap_dmac.octet[5])!=0x0)
		memcpy(&pData[0],&encap_info->encap_dmac.octet[0],ETHER_ADDR_LEN);

	//remark smac if assign value
	if((encap_info->encap_smac.octet[0] || encap_info->encap_smac.octet[1] || encap_info->encap_smac.octet[2] ||
		encap_info->encap_smac.octet[3]|| encap_info->encap_smac.octet[4] || encap_info->encap_smac.octet[5])!=0x0)
		memcpy(&pData[6],&encap_info->encap_smac.octet[0],ETHER_ADDR_LEN);

	if(debug_print){
		dump_packet(skb->data,skb->len,"Step 0: DA' SA' remarked");
	}


	/*step1: skb_push the inser header room (additional IP(20) + UDP(8) )*/
	skb_push(skb, pushed_room_size);
	if(debug_print){
		if(ipv6_encap)
			dump_packet(skb->data,skb->len,"Step 1: skb_push for additional IP6(40) + UDP(8)");
		else
			dump_packet(skb->data,skb->len,"Step 1: skb_push for additional IP(20) + UDP(8)");
	}


	/*step2:  Move the da, sa to skb->data from original da sa base*/
	memmove(skb->data, skb->data+pushed_room_size, da_sa_offset);
	if(debug_print){
		dump_packet(skb->data,skb->len,"Step 2:move the da' sa' to new skb->data");
	}

	/*step3: assign the header protocal to 0x0800*/
	if(ipv6_encap){
		skb->data[da_sa_offset] = 0x86;
		skb->data[da_sa_offset+1] = 0xdd;
	}else{
		skb->data[da_sa_offset] = 0x08;
		skb->data[da_sa_offset+1] = 0x00;
	}
	if(debug_print){
		if(ipv6_encap)
			dump_packet(skb->data,skb->len,"step 3: assign header protocol to 0x86dd");
		else
			dump_packet(skb->data,skb->len,"step 3: assign header protocol to 0x0800");
	}


	/*step4: clear the push room for new header and content*/
	skb->mac_header -= pushed_room_size;
	memset((skb->data+ da_sa_prorotcal_offset),0,pushed_room_size);

	if(debug_print){
		dump_packet(skb->data,skb->len,"step 4: clear the push room space");
	}


	/*step5: copy original IP header to new IP header,*/
	if(((pPktHdr->tagif&IPV6_TAGIF) && ipv6_encap) || ((pPktHdr->tagif&IPV4_TAGIF) && !ipv6_encap)){
		memmove((skb->data + da_sa_prorotcal_offset), (skb->data + l3_offset + pushed_room_size), encap_ip_len);
		if(ipv6_encap)
			ins_ip6_h = (struct ipv6_ethhdr *)(skb->data+ da_sa_prorotcal_offset);
		else
			ins_ip_h = (struct ipv4_ethhdr *)(skb->data+ da_sa_prorotcal_offset);
	}else if(ipv6_encap){
		if(debug_print)
			ACL("Original packet is %s but need to encapulate as %s, only update version", (pPktHdr->tagif&IPV6_TAGIF)?"IPv6":"IPv4", ipv6_encap?"IPv6":"IPv4");
		ins_ip6_h = (struct ipv6_ethhdr *)(skb->data+ da_sa_prorotcal_offset);
		ins_ip6_h->h_ver_tc_flowlabel = htons(0x6000);
	}else{
		if(debug_print)
			ACL("Original packet is %s but need to encapulate as %s, only update version", (pPktHdr->tagif&IPV6_TAGIF)?"IPv6":"IPv4", ipv6_encap?"IPv6":"IPv4");
		ins_ip_h = (struct ipv4_ethhdr *)(skb->data+ da_sa_prorotcal_offset);
		ins_ip_h->h_ver_ihl_tos = htons(0x4500);
	}
	if(debug_print){
		dump_packet(skb->data,skb->len,"step 5: copy original IP header to new IP header space");
	}

	/*step 6: remark new IP header	sip'/dip' if assign value*/
	if(ipv6_encap){
		if(encap_info->encap_Sip_ipv6[0]|encap_info->encap_Sip_ipv6[1]|encap_info->encap_Sip_ipv6[2]|encap_info->encap_Sip_ipv6[3]|
			encap_info->encap_Sip_ipv6[4]|encap_info->encap_Sip_ipv6[5]|encap_info->encap_Sip_ipv6[6]|encap_info->encap_Sip_ipv6[7]|
			encap_info->encap_Sip_ipv6[8]|encap_info->encap_Sip_ipv6[9]|encap_info->encap_Sip_ipv6[10]|encap_info->encap_Sip_ipv6[11]|
			encap_info->encap_Sip_ipv6[12]|encap_info->encap_Sip_ipv6[13]|encap_info->encap_Sip_ipv6[14]|encap_info->encap_Sip_ipv6[15]){
			ins_ip6_h->h_sip[3] = htonl((encap_info->encap_Sip_ipv6[12]<<24)|(encap_info->encap_Sip_ipv6[13]<<16)|(encap_info->encap_Sip_ipv6[14]<<8)|(encap_info->encap_Sip_ipv6[15]));
			ins_ip6_h->h_sip[2] = htonl((encap_info->encap_Sip_ipv6[8]<<24)|(encap_info->encap_Sip_ipv6[9]<<16)|(encap_info->encap_Sip_ipv6[10]<<8)|(encap_info->encap_Sip_ipv6[11]));
			ins_ip6_h->h_sip[1] = htonl((encap_info->encap_Sip_ipv6[4]<<24)|(encap_info->encap_Sip_ipv6[5]<<16)|(encap_info->encap_Sip_ipv6[6]<<8)|(encap_info->encap_Sip_ipv6[7]));
			ins_ip6_h->h_sip[0] = htonl((encap_info->encap_Sip_ipv6[0]<<24)|(encap_info->encap_Sip_ipv6[1]<<16)|(encap_info->encap_Sip_ipv6[2]<<8)|(encap_info->encap_Sip_ipv6[3]));
		}
		if(encap_info->encap_Dip_ipv6[0]|encap_info->encap_Dip_ipv6[1]|encap_info->encap_Dip_ipv6[2]|encap_info->encap_Dip_ipv6[3]|
			encap_info->encap_Dip_ipv6[4]|encap_info->encap_Dip_ipv6[5]|encap_info->encap_Dip_ipv6[6]|encap_info->encap_Dip_ipv6[7]|
			encap_info->encap_Dip_ipv6[8]|encap_info->encap_Dip_ipv6[9]|encap_info->encap_Dip_ipv6[10]|encap_info->encap_Dip_ipv6[11]|
			encap_info->encap_Dip_ipv6[12]|encap_info->encap_Dip_ipv6[13]|encap_info->encap_Dip_ipv6[14]|encap_info->encap_Dip_ipv6[15]){
			ins_ip6_h->h_dip[3] = htonl((encap_info->encap_Dip_ipv6[12]<<24)|(encap_info->encap_Dip_ipv6[13]<<16)|(encap_info->encap_Dip_ipv6[14]<<8)|(encap_info->encap_Dip_ipv6[15]));
			ins_ip6_h->h_dip[2] = htonl((encap_info->encap_Dip_ipv6[8]<<24)|(encap_info->encap_Dip_ipv6[9]<<16)|(encap_info->encap_Dip_ipv6[10]<<8)|(encap_info->encap_Dip_ipv6[11]));
			ins_ip6_h->h_dip[1] = htonl((encap_info->encap_Dip_ipv6[4]<<24)|(encap_info->encap_Dip_ipv6[5]<<16)|(encap_info->encap_Dip_ipv6[6]<<8)|(encap_info->encap_Dip_ipv6[7]));
			ins_ip6_h->h_dip[0] = htonl((encap_info->encap_Dip_ipv6[0]<<24)|(encap_info->encap_Dip_ipv6[1]<<16)|(encap_info->encap_Dip_ipv6[2]<<8)|(encap_info->encap_Dip_ipv6[3]));
		}
	}else{
		if(encap_info->encap_Sip!=0x0)
			ins_ip_h->h_sip = htonl(encap_info->encap_Sip);
		if(encap_info->encap_Dip!=0x0)
			ins_ip_h->h_dip = htonl(encap_info->encap_Dip);
	}

	if(debug_print){
		if(ipv6_encap){
			ACL("ins_ip6_h->h_sip=%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",(ins_ip6_h->h_sip[0]>>16), (ins_ip6_h->h_sip[0]&0xffff), (ins_ip6_h->h_sip[1]>>16), (ins_ip6_h->h_sip[1]&0xffff), (ins_ip6_h->h_sip[2]>>16), (ins_ip6_h->h_sip[2]&0xffff), (ins_ip6_h->h_sip[3]>>16), (ins_ip6_h->h_sip[3]&0xffff));
			ACL("ins_ip6_h->h_dip=%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x",(ins_ip6_h->h_dip[0]>>16), (ins_ip6_h->h_dip[0]&0xffff), (ins_ip6_h->h_dip[1]>>16), (ins_ip6_h->h_dip[1]&0xffff), (ins_ip6_h->h_dip[2]>>16), (ins_ip6_h->h_dip[2]&0xffff), (ins_ip6_h->h_dip[3]>>16), (ins_ip6_h->h_dip[3]&0xffff));
			ACL("ADDR[ins_ip6_h->h_sip]=0x%p",&ins_ip6_h->h_sip);
			ACL("ADDR[ins_ip6_h->h_dip]=0x%p",&ins_ip6_h->h_dip);
		}else{
			ACL("ins_ip_h->h_sip=0x%x",ins_ip_h->h_sip);
			ACL("ins_ip_h->h_dip=0x%x",ins_ip_h->h_dip);
			ACL("ADDR[ins_ip_h->h_sip]=0x%p",&ins_ip_h->h_sip);
			ACL("ADDR[ins_ip_h->h_dip]=0x%p",&ins_ip_h->h_dip);
		}
		dump_packet(skb->data,skb->len,"step 6:new header SIP'/DIP' remarking");
	}


	/*step 7: clear new l3_chksum(ipv4), and assigned length, ttl/hop limit, ipProto/next header to new IP header*/
	if(ipv6_encap){
		ins_ip6_h->h_payloadlen= htons((pPktHdr->l3Len) + UDP_HLEN + tags_before_ip_header_size + encap_tcp_rule_idx_len);	//payload_len = ori_IP_len + new_udp + original_tags_before_ip_header(ex:cvlan.PPPoE) + encap_tcp_rule_idx_len
		ins_ip6_h->h_nh_hp = htons(RG_IP_PROTO_UDP<<8|(254));//nextHeader=UDP(17), hop limit=254
	}else{
		ins_ip_h->h_checksum = 0;
		ins_ip_h->h_length= htons((pPktHdr->l3Len) + pushed_room_size + tags_before_ip_header_size + encap_tcp_rule_idx_len);	//new_IP_len = ori_IP_len + pushed_room_size[new_IP_header(20) + new_UDP_header(8) + new_protocal(2)] + original_tags_before_ip_header(ex:cvlan.PPPoE) + encap_tcp_rule_idx_len
		ins_ip_h->h_ttl_proto = htons((254<<8)|RG_IP_PROTO_UDP);//ttl=254, ipProto=UDP(17)
	}

	if(debug_print){
		if(ipv6_encap){
			ACL("ins_ip6_h->h_payloadlen =%d",ins_ip6_h->h_payloadlen);
			ACL("ins_ip6_h->h_nh_hp =0x%x",ins_ip6_h->h_nh_hp);

			ACL("ADDR[ins_ip6_h] =0x%p",ins_ip6_h);
			ACL("ADDR[ins_ip6_h->h_payloadlen]=0x%p",&ins_ip6_h->h_payloadlen);
			ACL("ADDR[ins_ip6_h->h_nh_hp]=0x%p",&ins_ip6_h->h_nh_hp);
			dump_packet(skb->data,skb->len,"step 7: assigned payload length, next header, hop limit to new IP header");
		}else{
			ACL("ins_ip_h->h_checksum =0x%x",ins_ip_h->h_checksum);
			ACL("ins_ip_h->h_length =%d",ins_ip_h->h_length);
			ACL("ins_ip_h->h_ttl_proto =0x%x",ins_ip_h->h_ttl_proto);

			ACL("ADDR[ins_ip_h] =0x%p",ins_ip_h);
			ACL("ADDR[ins_ip_h->h_length]=0x%p",&ins_ip_h->h_length);
			ACL("ADDR[ins_ip_h->h_ttl_proto]=0x%p",&ins_ip_h->h_ttl_proto);
			ACL("ADDR[ins_ip_h->h_checksum]=0x%p",&ins_ip_h->h_checksum);
			dump_packet(skb->data,skb->len,"step 7: clear new l3_chksum, and assigned length, ttl, ipProto to new IP header");
		}
	}


	/*step8: remark new UDP header sport*/
	ins_udp_h = (struct udp_ethhdr *)(skb->data+ da_sa_prorotcal_offset +encap_ip_len);
	ins_udp_h->h_sport = htons(18888); //force assign as spec requirement (dport always =18888)

	if(debug_print){
		ACL("ins_udp_h->h_sport=%d",ins_udp_h->h_sport);
		ACL("ADDR[ins_udp_h=0x%p]",ins_udp_h);
		ACL("ADDR[ins_udp_h->h_sport=0x%p]",&ins_udp_h->h_sport);
		dump_packet(skb->data,skb->len,"step8: remark new UDP header sport");
	}


	/*step9: remark new UDP header dport*/
	if(encap_info->encap_Dport!=0)
		ins_udp_h->h_dport = htons(encap_info->encap_Dport);

	if(debug_print){
		ACL("ins_udp_h->h_dport=%d",ins_udp_h->h_dport);
		ACL("ADDR[ins_udp_h=0x%p]",ins_udp_h);
		ACL("ADDR[ins_udp_h->h_dport]=0x%p",&ins_udp_h->h_dport);
		dump_packet(skb->data,skb->len,"step9: remark new UDP header dport");
	}


	/*step10: assign new UDP header len and chksum*/
	ins_udp_h->h_length = htons((pPktHdr->l3Len)-encap_ip_len+ pushed_room_size + tags_before_ip_header_size + encap_tcp_rule_idx_len); //new_UDP_len = ori_IP_len - ori_IP_header(20 for v4/40 for v6) + pushed_room_size[new_IP_header(20 for v4/40 for v6) + new_UDP_header(8) + new_protocal(2)] + original_tags_before_ip_header(ex:cvlan.PPPoE) + encap_tcp_rule_idx_len
	ins_udp_h->h_checksum = 0;

	if(debug_print){
		ACL("pPktHdr->l3Len=%d",pPktHdr->l3Len);
		ACL("tags_before_ip_header_size=%d",tags_before_ip_header_size);
		ACL("ins_udp_h->h_length=%d",ins_udp_h->h_length);
		ACL("ins_udp_h->h_checksum=0x%x",ins_udp_h->h_checksum);
		ACL("ADDR[ins_udp_h]=0x%p",ins_udp_h);
		ACL("ADDR[ins_udp_h->h_length=0x%p]",&ins_udp_h->h_length);
		ACL("ADDR[ins_udp_h->h_checksum]=0x%p",&ins_udp_h->h_checksum);
		dump_packet(skb->data,skb->len,"step10: assign new UDP header len and chksum");
	}



	/* step11: new IP/UDP header checksum recaculate */
	if(!ipv6_encap)	//only ipv4 need to update ip checksum
		ins_ip_h->h_checksum=htons(inet_chksum((u8*)ins_ip_h,encap_ip_len));
	if(ipv6_encap)
		ins_udp_h->h_checksum =htons(inet_chksum_pseudoV6((u8*)ins_udp_h,ntohs(ins_udp_h->h_length),(u8*)ins_ip6_h->h_sip,(u8*)ins_ip6_h->h_dip,RG_IP_PROTO_UDP/*always insert UDP header*/));
	else
		ins_udp_h->h_checksum =htons(inet_chksum_pseudo((u8*)ins_udp_h,ntohs(ins_udp_h->h_length),ntohl(ins_ip_h->h_sip),ntohl(ins_ip_h->h_dip),RG_IP_PROTO_UDP/*always insert UDP header*/));

	if(debug_print){
		if(!ipv6_encap)
			ACL("encap_ip_len=%d",encap_ip_len);
		ACL("ins_udp_h->h_length=%d",ins_udp_h->h_length);
		if(!ipv6_encap)
			ACL("ADDR[ins_ip_h->h_checksum]=0x%p",&ins_ip_h->h_checksum);
		ACL("ADDR[ins_udp_h->h_checksum]=0x%p",&ins_udp_h->h_checksum);
		dump_packet(skb->data,skb->len,"step11: new IP/UDP header checksum recaculate ");
	}

	if(debug_print)
		dump_packet(skb->data,skb->len,"Final UDP encap and mirrored packet to PS");

	return skb;
}


static inline void rg_set_bit(unsigned int nr, void *addr)
{
	((unsigned int *) addr)[nr>>5] |= (0x1 << (nr&0x1f));
}

static inline void rg_clear_bit(unsigned int nr, void *addr)
{
	((unsigned int *) addr)[nr>>5] &= ~(0x1 << (nr&0x1f));
}

static inline int rg_test_bit(unsigned int nr, void *addr)
{
	return 1UL & (((unsigned int *) addr)[nr>>5] >> (nr&0x1f));
}

int8 *_inet_ntoa_r(ipaddr_t ipaddr, int8 *p);
int8 *_inet_ntoa(rtk_ip_addr_t ina);
int8 *_rtk_rg_inet_n6toa(const uint8 *ipv6);
int8 *diag_util_inet_ntoa(uint32 ina);

/*internal utils*/
#define inet_ntoa_r(x,y) _inet_ntoa_r(x,y)
#define inet_ntoa(x) _inet_ntoa(x)

rtk_rg_initState_t _rtk_rg_get_initState(void);


#endif

#endif


#endif

