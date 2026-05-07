/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*/

#ifndef __RTK_FC_INTERNAL__
#define __RTK_FC_INTERNAL__

#include <linux/if_bridge.h>
#include <br_private.h>

#if defined(CONFIG_RTK_L34_XPON_PLATFORM) || defined(CONFIG_RTK_L34_G3_PLATFORM)
#include <rtk/init.h>
#include <rtk/svlan.h>
#include <rtk/sec.h>
#include <rtk/stat.h>
#include <rtk/ponmac.h>
#include <rtk/l2.h>
#include <rtk/svlan.h>
#include <rtk/trap.h>
#include <rtk/rate.h>
#include <rtk/acl.h>
#ifndef rt_enable_t
#define rt_enable_t rtk_enable_t
typedef struct  rt_mac_s
{
    uint8 octet[ETHER_ADDR_LEN];
} rt_mac_t;
#endif

#endif

#include "rtk_fc_struct.h"
#include "rtk_fc_define.h"
#include "rtk_fc_error.h"
#include "rtk_fc_external.h"


#if defined(CONFIG_RTK_L34_G3_PLATFORM)
#if defined(CONFIG_FC_CA8277AB_SERIES)
#include "rtk_rg_g3_internal.h"
#endif
/*** RTK-SDK ***/
#include <dal/luna_g3/dal_luna_g3.h>
/*** CA-G3 ***/
#include <classifier.h>
#include <l2.h>
#include <route.h>
#include <flow.h>
#include <nat.h>
#include <aal_fdb.h>
#include <rate.h>
#include <aal_l3_cam.h>
#include <aal_l3_pe.h>
#endif

#if defined(CONFIG_RTK_FC_DRV_EVENT_RECORD_VERSION) && (CONFIG_RTK_FC_DRV_EVENT_RECORD_VERSION == 2)
#include <linux/exception_log.h>
#endif

extern void memDump (void *start, u32 size, char * strHeader);

int has_expired(const struct net_bridge *br, const struct net_bridge_fdb_entry *fdb);

int assert_eq(int func_return,int expect_return,const char* func,int line);
void assert_ok_sub_func(int assert_ret, char *assert_str,const char *func,int line);
void assert_sub_func(int assert_ret, char *assert_str,const char *func,int line);


#ifndef ASSERT_EQ
#define ASSERT_EQ(value1, value2) \
do { \
	int func_return=(value1); \
	if(assert_eq((func_return), (value2), __FUNCTION__,__LINE__)!=0) \
		return func_return; \
}while(0)
#endif

// continue without return
#ifndef ASSERT_EQ_CONT
#define ASSERT_EQ_CONT(value1, value2) \
do { \
	if(assert_eq((value1), (value2), __FUNCTION__,__LINE__)){} \
}while(0)
#endif

#ifndef ASSERT_EQ_WITH_RET
#define ASSERT_EQ_WITH_RET(value1, value2, retVal) \
do { \
        int func_return=(value1); \
        if(assert_eq((func_return), (value2), __FUNCTION__,__LINE__)!=0) \
	{ \
		retVal = -1; \
		break; \
	} \
	retVal = 0; \
}while(0)
#endif

#ifndef assert_ok
#define assert_ok(x) do { assert_ok_sub_func(x,#x,__FUNCTION__,__LINE__); } while(0)
#endif

#ifndef assert
#define assert(x) do { assert_sub_func(x,#x,__FUNCTION__,__LINE__); } while(0)
#endif

#define RTK_FC_NETIF_MIB_ALL_CPUS_SUM(netifIdx, PARAM) \
do { \
} while(0)

/*get spinlock at first bucket per lock group*/
static inline void rtk_fc_flow_batchscan_group_lock(uint32 flowBucketIdx)
{	
	if((flowBucketIdx<<fc_db.flowHashWayShift) % fc_db.flowLockGroupSize == 0)
		RTK_FC_HELPER_MGR_FUNC_SPIN_LOCK(RTK_FC_FUNC_LOCK_FLOW, (flowBucketIdx << fc_db.flowHashWayShift));
}

/*release spinlock at last bucket per lock group*/
static inline void rtk_fc_flow_batchscan_group_unlock(uint32 flowBucketIdx)
{		
	if(((flowBucketIdx+1)<<fc_db.flowHashWayShift) % fc_db.flowLockGroupSize == 0)
		RTK_FC_HELPER_MGR_FUNC_SPIN_UNLOCK(RTK_FC_FUNC_LOCK_FLOW, (flowBucketIdx << fc_db.flowHashWayShift));
}


// Packet Header
static inline void _rtk_fc_pktHdr_init(rtk_fc_pktHdr_t *pPktHdr, struct rt_skbuff *rtskb)
{
	//bzero(pPktHdr, offsetof(rtk_fc_pktHdr_t, RESET_BOUNDARY));
	//memset(&pPktHdr->RESET_BOUNDARY, -1, sizeof(rtk_fc_pktHdr_t)-offsetof(rtk_fc_pktHdr_t, RESET_BOUNDARY));
	pPktHdr->flowHashIdx = SIGNED_INVALID;
	pPktHdr->igrWlanDevIdx = RTK_FC_WLANX_NOT_FOUND;
	pPktHdr->egrWlanDevIdx = RTK_FC_WLANX_NOT_FOUND;

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	pPktHdr->pkt_fmr_idx = SIGNED_INVALID;
#endif
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)
	pPktHdr->ipsec_hook_table_index = SIGNED_INVALID;
#endif
}

static inline struct net_device* rtk_fc_getBridgedDevbyNetDev(const struct net_device* dev)
{

	struct net_bridge_port *p=NULL;
	struct net_device* br_dev = NULL;

	RTK_FC_HELPER_RCU_READ_LOCK();

	// Check if the device is still in bridge mode
	if(RTK_FC_HOOK_PS_DEV_NETIF_RUNNING((struct net_device*)dev)){
		if(!RTK_FC_HOOK_PS_DEV_IS_PRIV_FLAGS_SET((struct net_device*)dev, RTK_FC_NETDEV_PRIV_FLAG_TYPE_BRIDGE_PORT) )
		{
			RTK_FC_HELPER_RCU_READ_UNLOCK();
			return NULL;
		}

	}else{
		RTK_FC_HELPER_RCU_READ_UNLOCK();
		return NULL;

	}
	
	
	RTK_FC_HELPER_RCU_GET_BRIDGE_PORT_BY_DEV(dev, &p);
	

	if(!p){
		RTK_FC_HELPER_RCU_READ_UNLOCK();
		return NULL;
	}
	
	RTK_FC_HELPER_GET_BRIDGE_DEV_BY_BRIDGE_PORT(p, &br_dev);
	RTK_FC_HELPER_RCU_READ_UNLOCK();

	return br_dev;
}

static inline int rtk_fc_reverse_flow_path5_match(rtk_fc_flow_direction_t direction, int search_index, rtk_fc_tableFlow_t *pFlowTable)
{
	int find = 0;
	if(direction==RTK_FC_FLOW_DIRECTION_UPSTREAM) {
		if( fc_db.flowTbl[search_index].pFlowEntry->path5.in_src_ipv4_addr == pFlowTable->pFlowEntry->path5.in_dst_ipv4_addr &&
			fc_db.flowTbl[search_index].pFlowEntry->path5.in_intf_idx == pFlowTable->pFlowEntry->path5.out_intf_idx &&
			fc_db.flowTbl[search_index].pFlowEntry->path5.in_l4_src_port == pFlowTable->pFlowEntry->path5.in_l4_dst_port &&
	   		fc_db.flowTbl[search_index].pFlowEntry->path5.in_l4_dst_port == pFlowTable->pFlowEntry->path5.out_l4_port
	    ){
	    	find = 1;
			
		}
		
	} else if (direction==RTK_FC_FLOW_DIRECTION_DOWNSTREAM) {
		if( fc_db.flowTbl[search_index].pFlowEntry->path5.in_l4_dst_port == pFlowTable->pFlowEntry->path5.in_l4_src_port	&&
	   		fc_db.flowTbl[search_index].pFlowEntry->path5.out_l4_port == pFlowTable->pFlowEntry->path5.out_l4_port 	&&
	    	fc_db.flowTbl[search_index].pFlowEntry->path5.in_src_ipv4_addr == pFlowTable->pFlowEntry->path5.out_dst_ipv4_addr&&
	    	fc_db.flowTbl[search_index].pFlowEntry->path5.in_dst_ipv4_addr == pFlowTable->pFlowEntry->path5.in_src_ipv4_addr 
	    	
	    ){
	    	find = 1;
		}
	}
	return find;
}


static inline int rtk_fc_reverse_flow_path3_match(int search_index, rtk_fc_tableFlow_t *pFlowTable)
{
	int find = 0;
	
	if( fc_db.flowTbl[search_index].pFlowEntry->path5.in_src_ipv4_addr == pFlowTable->pFlowEntry->path3.in_dst_ipv4_addr &&
		fc_db.flowTbl[search_index].pFlowEntry->path5.in_dst_ipv4_addr == pFlowTable->pFlowEntry->path3.in_src_ipv4_addr &&
		fc_db.flowTbl[search_index].pFlowEntry->path5.in_l4_src_port == pFlowTable->pFlowEntry->path3.in_l4_dst_port &&
   		fc_db.flowTbl[search_index].pFlowEntry->path5.in_l4_dst_port == pFlowTable->pFlowEntry->path3.in_l4_src_port
    )
    {
    	find = 1;
	}
	
	return find;
}

static inline unsigned int _rtk_fc_quickhash_mac_fid(unsigned char *mac, unsigned int fid)
{
	uint32 lutHashIdx = ((mac[4])<<8 | (mac[5])) & (RTK_FC_LUT_BUCKET_SIZE-1);
	return lutHashIdx;
}

static inline uint8 FLOW_INFO_SET(rtk_fc_tableFlow_t *pFlowTbl, rt_flow_extra_info_t flowInfo, bool set)
{
	pFlowTbl->flow_extra_info &= ~flowInfo;
	if(set) pFlowTbl->flow_extra_info |= flowInfo;
	return 0;
}

static inline uint8 FLOW_INFO_IS_SET(rtk_fc_tableFlow_t *pFlowTbl, rt_flow_extra_info_t flowInfo)
{
	return (pFlowTbl->flow_extra_info & flowInfo) ? TRUE : FALSE;
}

static inline void FLOW_SHORTCUT_HIT(rtk_fc_pktHdr_t *pPktHdr)
{
	pPktHdr->isHitShortcut = TRUE;
	
}

u16 inet_chksum(u8 *dataptr, u16 len);
u16 inet_chksum_pseudo(u8 *tcphdr, u16 tcplen, u32 srcip, u32 destip,  u8 proto);
u16 inet_chksum_pseudoV6(u8 *tcphdr, u16 tcplen, u8 *srcip, u8 *destip,  u8 proto);
void rtk_fc_set_bit(unsigned int nr, void *addr);
void rtk_fc_clear_bit(unsigned int nr, void *addr);
int rtk_fc_test_bit(unsigned int nr, void *addr);

int rtk_fc_user_pipe_cmd(const char *comment, ...);

#if defined(CONFIG_RTK_L34_XPON_PLATFORM) 
void rtk_fc_dynamicPrehashPtn_timerFunc(unsigned long task_priv);
#endif
void rtk_fc_rtnetlink_timerFunc(unsigned long task_priv);
void rtk_fc_rtnetlink_sync_lut(void);
void rtk_fc_rtnetlink_register_notifier(void);
void rtk_fc_rtnetlink_unregister_notifier(void);

void _rtk_fc_drv_event_record_timeoutCheck(void);
void rtk_fc_drv_event_record(rtk_fc_drv_event_record_id_t eventIdx);

rtk_fc_wlan_devidx_t rtk_fc_wlan_port_to_devid_find(rtk_fc_mac_port_idx_t macPort, rtk_fc_wlan_ext_info_t wlanExtInfo,  
													struct rt_skbuff *rtskb, struct sk_buff *skb, bool saLookup, bool lockReq);
rtk_fc_wlan_devidx_t rtk_fc_wlan_rtmbssid_to_devidx(rt_wlan_index_t wlanIdx, rt_wlan_mbssid_index_t mbssidIdx);
int rtk_fc_wlan_rtmbssidMask_to_devMask(rt_wlan_mbssid_mask_t *rtWlanMbssidMsk,rtk_fc_wlan_devmask_t *wlanDevIdMask);
int rtk_fc_wlan_devMask_to_rtmbssidMask(rtk_fc_wlan_devmask_t *wlanDevIdMask,rt_wlan_mbssid_mask_t *rtWlanMbssidMsk);
rtk_fc_ret_t rtk_fc_wan_portmask_set(rtk_fc_port_mask_t wanpmsk);

unsigned int _rtk_fc_hash_mac_fid(unsigned char *mac, unsigned int fid);

void _rtk_fc_learning_preparation(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr);

void _rtk_fc_lutEntry_pool_init(void);
rtk_fc_lut_entry_t *_rtk_fc_lutEntry_getFromPool(void);
void _rtk_fc_lutEntry_returnToPool(rtk_fc_lut_entry_t *pLutEntry);
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
int _rtk_fc_hw_lut_idx_get(uint8 *mac, uint32 fid, uint32 *hwL2Idx);
#endif

int RTK_FC_LUT_ADD(rtk_fc_lut_entry_t *pLutEntry, int update);
int RTK_FC_LUT_DEL(rtk_fc_lut_entry_t *pLutEntry, uint8 skipFlowFlush);

// SA Learning & timeout check
int32 _rtk_fc_lut_nonStaticEntry_flush(void);
int32 _rtk_fc_lut_wlanDevidx_del(rtk_fc_wlan_devidx_t wlan_dev_idx);
int32 _rtk_fc_lut_linkdown_del(uint32 portIdx);
int32 _rtk_fc_lut_staticEntry_del(uint32 lutIdx, uint8 isStaticWifi);
bool _rtk_fc_lookup_fdb_force(unsigned char *mac, uint16 vid, rtk_fc_fdb_entry_t *fc_fdb);
bool _rtk_fc_lookup_fdb(unsigned char *mac, uint16 vid, rtk_fc_fdb_entry_t *fc_fdb);
bool _rtk_fc_lookup_fdb_by_pvid(unsigned char *mac, int vid, rtk_fc_fdb_entry_t *fc_fdb);
bool rtk_fc_get_bridgePort_pvid_by_netDev(const struct net_device* dev,  unsigned char *mac, uint16 *pvid);
struct net_device* rtk_fc_getDev_by_fcDevIdx(int ifidx);
int rtk_fc_psIfidxStackKey_to_dev(uint32 stackKey,struct net_device **devArray);
int rtk_fc_psIfidxStackKey_to_fcDevIdx(uint32 stackKey,int32 *fcDevIdx_array);


bool _rtk_fc_lookup_v4neighbor(unsigned char *mac);
bool _rtk_fc_lookup_v6neighbor(unsigned char *mac);
void _rtk_fc_enumerate_v4neighbor(void (*cb)(struct neighbour *, void *), void *cookie);
void _rtk_fc_enumerate_v6neighbor(void (*cb)(struct neighbour *, void *), void *cookie);
int _rtk_fc_compare_neighbor_ha_nud(struct neighbour *neigh, void *cookie, unsigned char state);
int _rtk_fc_compare_neighbor_primary_key_nud(struct neighbour *neigh, void *cookie, unsigned char state);
void _rtk_fc_update_neighbor_nud(struct neighbour * neigh, unsigned char state, unsigned int flag);
rtk_fc_ret_t _rtk_fc_lut_find_fid(uint8 *mac, uint8 fid, int16 *lutIdx);
rtk_fc_ret_t _rtk_fc_lut_find(uint8 *mac, int16 *lutIdx);
rtk_fc_ret_t _rtk_fc_lut_portmoving_check(int32 lutIdx, rtk_fc_pmap_t curPortMap, rtk_fc_wlan_devidx_t wlanDevIdx, uint16 cvid);
int32 _rtk_fc_lut_LRU(uint32 lutHashIdx);
uint8 _rtk_fc_lut_learning_check(rtk_fc_pktHdr_t *pPktHdr);
rtk_fc_ret_t _rtk_fc_lut_learning(uint8 *srcMac, rtk_fc_pmap_t igrPort, uint16 vid, rtk_fc_wlan_devidx_t wlan_dev_idx,
												uint8 verify_fdb, uint8 isStatic,int16* macL2Idx, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_lut_learning_state_t learning_state);

int32 rtk_fc_abstrSwFlowSetTemplateType(uint32 typeMsk,uint8 priIdx);
int32 rtk_fc_abstrSwFlowDelTemplateType(uint8 priIdx);


void _rtk_fc_lut_timeoutCheck(void);
uint8 _rtk_fc_check_neighbor_mac(struct neighbour *neigh, unsigned char *mac);

rtk_fc_ret_t _rtk_fc_lut_learning_processing(struct rt_skbuff *rtskb, int egressPortMask, rtk_fc_pktHdr_t *pPktHdr);

rtk_fc_ret_t rtk_fc_macAddr_portGroup_lrnCnt_inc(rtk_fc_lut_entry_t *pLutEntry);
rtk_fc_ret_t rtk_fc_macAddr_portGroup_lrnCnt_dec(rtk_fc_lut_entry_t *pLutEntry);
rtk_fc_ret_t rtk_fc_macAddr_portGroup_lrnCnt_check(int spa);
rtk_fc_ret_t rtk_fc_macAddr_portGroup_lrnCnt_config(rtk_fc_port_mask_t portmask);

rtk_fc_ret_t _rtk_fc_pktHdr_inspect(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, int egressPortMask, fc_rx_info_t *pRxInfo);
void _rtk_fc_trace_log_processing(struct rt_skbuff *rtskb, fc_rx_info_t *pRxInfo, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_trafilter_stage_t isIgr);

int _rtk_fc_pktHdr_dump(rtk_fc_pktHdr_t *pPktHdr);
uint16 _rtk_fc_L3ChecksumDiff_update(uint16 ori_checksum, uint32 ori_sip, uint32 ori_dip, uint16 ori_tos ,
														uint32 new_sip, uint32 new_dip, uint16 new_tos, uint32 ttldiff);
uint16 _rtk_fc_L4ChecksumDiff_update(uint16 ori_checksum, uint32 ori_sip, uint32 ori_dip, uint16 ori_sport, uint16 ori_dport,
														uint32 new_sip, uint32 new_dip, uint16 new_sport, uint16 new_dport);
uint16 _rtk_fc_icmpChecksumDiff_update(uint16 ori_checksum, uint32 ori_id, uint32 new_id);

uint16 _rtk_fc_ipv6_L4ChecksumDiff_update(uint16 *new_ip, uint16 *old_ip, uint16 checksum);

int _rtk_fc_pktHdr_v4L4Checksum_update(rtk_fc_pktHdr_t *pPktHdr);
int _rtk_fc_pktHdr_v6L4Checksum_update(rtk_fc_pktHdr_t *pPktHdr);

#if defined(CONFIG_RTK_L34_XPON_PLATFORM) || defined(CONFIG_RTK_L34_G3_PLATFORM)
void _rtk_fc_lut_timeout_byCheckAging(void);
void _rtk_fc_lut_timeout_byCheckTrfBits(void);
rtk_fc_ret_t _rtk_fc_lutCamListAdd(uint32 lutHashIdx, uint32 addLutCamIdx);
rtk_fc_ret_t _rtk_fc_lutCamListDel(uint32 delLutCamIdx);
rtk_fc_ret_t _rtk_fc_lutHostPolicingIdxUpdate(uint8 *mac);
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
rtk_fc_ret_t _rtk_fc_wifi_amsdu_pe_offload_init(bool if_amsdu_pe_mode_change);
rtk_fc_ret_t _rtk_fc_wifi_amsdu_pe_offload_voq_info_get(unsigned int mac_id, uint8 *ldpid, uint8 *cos);
rtk_fc_ret_t _rtk_fc_wifi_amsdu_pe_offload_wlanDevIdx_get(unsigned int mac_id, rtk_fc_wlan_devidx_t *wlanDevIdx);
rtk_fc_ret_t _rtk_fc_lutMacIdIdxUpdate(uint8 *mac);
rtk_fc_ret_t _rtk_fc_lutStaInfoUpdate(uint8 *mac, unsigned int mac_id, rtk_fc_wlan_devidx_t wlan_dev_idx, rtk_fc_wifi_amsdu_pe_offload_sta_info_update_mask_t update_mask);
#endif

struct net_device* rtk_fc_getIngressDev(rtk_fc_pktHdr_t *pPktHdr, struct rt_skbuff *rtskb, rtk_fc_forwardingType_t fwdType, rtk_fc_flow_direction_t dir);
struct net_device* rtk_fc_getEgressDev(rtk_fc_pktHdr_t *pPktHdr, struct rt_skbuff *rtskb, rtk_fc_forwardingType_t fwdType, rtk_fc_flow_direction_t dir);
int16 rtk_fc_getIngressSwNetifIdx(rtk_fc_pktHdr_t *pPktHdr, struct sk_buff *skb, rtk_fc_forwardingType_t fwdType, rtk_fc_flow_direction_t dir);
int16 rtk_fc_getEgressSwNetifIdx(rtk_fc_pktHdr_t *pPktHdr, struct sk_buff *skb, rtk_fc_forwardingType_t fwdType, rtk_fc_flow_direction_t dir);
void rtk_fc_internal_vid_fdb_find(rtk_fc_pktHdr_t *pPktHdr, struct sk_buff *skb);



rtk_fc_ret_t rtk_fc_flow_ct_decision(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, struct rt_nfconn **rtct);
rtk_fc_ret_t rtk_fc_flow_ct_detach(rtk_fc_tableFlow_t *pFlowTbl);
rtk_fc_ret_t rtk_fc_flow_flush_forHashChange(void);
rtk_fc_ret_t rtk_fc_flow_flush(void);
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
rtk_fc_ret_t rtk_fc_ct_info_flush(void);
#endif
rtk_fc_ret_t rtk_fc_reset_v6NatTable(void);
rtk_fc_ret_t rtk_fc_reset_v6MaptTable(void);
rtk_fc_ret_t rtk_fc_flow_flush_all(void);
rtk_fc_ret_t rtk_fc_flow_delete_by_pattern(rt_flow_op_flow_pattern_t flowPattern);
rtk_fc_ret_t rtk_fc_flow_delete_by_intfIdx(int intfIdx);
rtk_fc_ret_t rtk_fc_flow_delete_by_l2Idx(int l2Idx);
rtk_fc_ret_t rtk_fc_flow_delete_by_wlanIdx(rtk_fc_wlan_devidx_t wlanIdx);
rtk_fc_ret_t rtk_fc_flow_delete_by_spa(int spa, int extspa);


rtk_fc_ret_t rtk_fc_flow_delete_by_entryIdx(uint32 entIdx);
rtk_fc_ret_t rtk_fc_abstrSwFlow_BC_UUC_UMC_floodingDomain_delete(void);
rtk_fc_ret_t rtk_fc_flow_delete(int flowIdx);
rtk_fc_ret_t rtk_fc_flow_invalid_hw(int flowIdx);
rtk_fc_ret_t rtk_fc_flow_add(uint32 *pFlowIdx, uint32 flowHashIdx, void *pPathData, struct rt_nfconn *rtct,
											rtk_fc_igrExtraInfo_t *pExtraInfo, uint8 isMulticast, rtk_fc_pmap_t ingressPort, uint8 addSwOnly, uint8 staticEntry,
											rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo,  rtk_fc_pktHdr_t *pPktHdr);
rtk_fc_ret_t rtk_fc_flow_reverse_add(uint32 *pFlowIdx, uint32 flowHashIdx, void *pPathData, struct rt_nfconn *rtct,
									rtk_fc_igrExtraInfo_t *pExtraInfo, uint8 isMulticast, rtk_fc_pmap_t ingressPort, uint8 addSwOnly, uint8 staticEntry,
									rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo);
rtk_fc_flowEntSearchReturn_t rtk_fc_flow_searchByIngressPktHdr(uint32 flowHashIdx, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_tableFlow_t *pFlowTable,rtk_fc_tableFlow_t *pFlowTable_mutiAction,uint32* flowTblIdx,uint32* flowTblMutiActIdx,rtk_fc_tableFlow_t **pFlowTablePointer);

rtk_fc_ret_t rtk_fc_flow_skbDataReasm(rtk_fc_ingress_data_t *pFcIngressData , rtk_fc_pktHdr_t *pPktHdr, uint8 *pData);
#if defined(CONFIG_FC_FLOW_AUTO_EXTEND)
rtk_fc_ret_t rtk_fc_flow_dummyPktBuffer_alloc(void);
rtk_fc_ret_t rtk_fc_flow_dummyPktBuffer_free(void);
rtk_fc_ret_t rtk_fc_flow_autoExtendByDummyPkt(rtk_fc_tableFlow_t *pFlowTable);
#endif
rtk_fc_ret_t rtk_fc_flow_updateTimer_and_checkTimeout(int idx, unsigned char trfBit);

rtk_fc_ret_t rtk_fc_dualHdrInfo_set(rtk_rg_asic_dualHdrInfo_t target, uint8 intfIdx, uint32 value);
rtk_fc_ret_t rtk_fc_dualHdrInfo_get(rtk_rg_asic_dualHdrInfo_t target, uint8 intfIdx, uint32 *value);
void rtk_fc_update_pppoeHwNetif_info(int i, int sid);

int _rtk_fc_flowPath_get(rtk_fc_tableFlowEntry_t *pFlow);
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
int _rtk_fc_mainHashFlowTrf_update(void);
void _rtk_fc_lut_timeout_byCheckKernel(void);
int _rtk_fc_lut_updateTimer(int l2Idx);
#endif
int _rtk_fc_flow_timeoutCheck(void);
void _rtk_fc_pppoeConnectionAutoExtend_task(unsigned long task_priv);
int _rtk_fc_intfMIB_timeoutCheck(void);
int _rtk_fc_sw_perFlowCounting(int flowTblIdx, int pktLenIgr, uint16 payloadLen);
uint64 _rtk_fc_sw_hostMib_all_cpu_get(uint32 hostCtrlIdx, rtk_fc_hostMibType_t type);
uint64 _rtk_fc_sw_flowMib_all_cpu_get(uint32 flowmibIdx, rtk_fc_flowMibType_t type);
uint64 _rtk_fc_sw_netifMib_all_cpu_get(uint32 netifIdx, rtk_fc_netifMibType_t type);
int _rtk_fc_sw_hostPolicingMibCounting(int16 smacHostPolIdx, int16 dmacHostPolIdx, int pktLenIgr, int pktLenEgr, uint16 payloadLen);
rtk_fc_ret_t _rtk_fc_flowEgressTx(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, int portMask, rtk_fc_wlan_devidx_t egrWlanDevIdx);
int _rtk_fc_flowModifyAndDirTx(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_tableFlow_t *pFlowTable, uint32 flow_Idx, bool ifSkipNicL2Offload);
bool _rtk_fc_flow_isTheSamePatternExist_get(int32 checkFlowIdx, uint32 flowHashIdx , uint8 isMulticast, uint32 *theSamePtnFlowIdx);
rtk_fc_ret_t _rtk_fc_flow_wan_downstream_loopback_flow_prepare
	(void *flowEntryData, void *revFlowEntryData, int *revHashIdx, rtk_fc_igrExtraInfo_t *extraInfo, rtk_fc_igrExtraInfo_t *revExtraInfo, 
	rtk_fc_g3IgrExtraInfo_t *g3IgrExtraInfo, rtk_fc_g3IgrExtraInfo_t *revG3IgrExtraInfo, rtk_fc_pktHdr_t *pPktHdr);
int _rtk_fc_flow_swField_set(uint32 flowIdx, uint32 flowHashIdx, struct nf_conn *ct, rtk_fc_igrExtraInfo_t *pExtraInfo, int indMacIdx, uint8 swOnly, uint8 staticEntry,uint8 isMulticast, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, rtk_fc_pktHdr_t *pPktHdr);
int _rtk_fc_flowTable_init(void);
int _rtk_fc_flowDataBase_init(void);
int _rtk_fc_flowTcamListAdd(uint32 flowHashIdx, uint32 addFlowIdx);
int _rtk_fc_flowTcamListDel(uint32 delFlowIdx);
rtk_fc_ret_t _rtk_fc_extPMaskEntry_find(uint32 extPMask, uint32 *entryIdx);
rtk_fc_ret_t _rtk_fc_ethTypeEntry_acl_ref(uint16 ethtype, uint32 *hwIdx);
rtk_fc_ret_t _rtk_fc_ethTypeEntry_acl_deref(uint32 hwIdx);
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
uint32 _rtk_fc_flow_hashIndex(rtk_fc_tableFlowEntry_t flowPathEntry, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, rtk_rg_asic_flow_hash_crc_t *flow_hash_crc);
int32 _rtk_fc_flow_hashIndex_byDualPktOuterHdr(rtk_fc_pktHdr_t *pPktHdr, rtk_rg_asic_flow_hash_crc_t *flow_hash_crc);
int32 _rtk_fc_flow_hashIndex_byPktHdr(rtk_fc_pktHdr_t *pPktHdr, rtk_rg_asic_flow_hash_crc_t *flow_hash_crc);
#else
uint32 _rtk_fc_flow_hashIndex(rtk_fc_tableFlowEntry_t flowPathEntry, uint16 igrSVID, uint16 igrCVID, uint16 lutDaIdx_gmacChk /*used by path3 & path4*/);
int32 _rtk_fc_flow_hashIndex_byDualPktOuterHdr(rtk_fc_pktHdr_t *pPktHdr);
int32 _rtk_fc_flow_hashIndex_byPktHdr(rtk_fc_pktHdr_t *pPktHdr);
#endif


void rtk_fc_reg_io_write32 (void *addr, unsigned long val);
unsigned long rtk_fc_reg_io_read32 (void *addr);
void rtk_fc_hexdump(unsigned char *buf, unsigned int len);
void rtk_fc_reg_to_dev32_s(void *addr, const unsigned char *src, unsigned long nword);
rtk_fc_ret_t _rtk_fc_flow_hashIndex_get(rtk_fc_pktHdr_t *pPktHdr, fc_rx_info_t *pRxInfo,struct sk_buff *skb);
#if !(defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES))
uint32 _rtk_fc_sw_flowHashIndexStep1_get_with_prehashPattern(uint16 param1, uint16 param2, uint32 param3, uint32 param4, uint32 extraItem, uint32 sip_ptn, uint32 dip_ptn, uint32 sport_ptn, uint32 dport_ptn);
uint32 _rtk_fc_sw_flowHashIndexStep1_get(uint16 param1, uint16 param2, uint32 param3, uint32 param4, uint32 extraItem);
uint32 _rtk_fc_sw_flowHashIndexStep2_get(uint32 step1Idx);
#endif

uint32 _rtk_fc_flow_matchOrNot_tblEntry(rtk_fc_tableFlow_t *pFlowCmp, rtk_fc_tableFlow_t *pFlowCur);
uint32 _rtk_fc_flow_matchOrNot(rtk_fc_tableFlow_t *pFlowEntryCmp, rtk_fc_tableFlowEntry_t *pFlowEntry, rtk_fc_g3IgrExtraInfo_t *pIgrExtraInfo, uint8 isMulticast);

int rtk_fc_abstrSwSyncToHw_update(int32 swFlowIdx);
int rtk_fc_abstrSwSyncToHw_del(int32 swFlowIdx);
int rtk_fc_abstrSwSyncToHw_add(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr,int32 swFlowIdx);
int rtk_fc_parseAbstrSwPattenField(uint8 isIpv6,rtk_fc_abstrSwFlowPattern_entry_t *swFlowKey,rtk_fc_abstrSwFlowPattenField_entry_t *pField);
int rtk_fc_parseAbstrSwActionField(  uint8 isIpv6,rtk_fc_abstrSwFlowAction_entry_t *pSwFlowAction,rtk_fc_abstrSwFlowActionField_entry_t *pField);
rtk_fc_ret_t rtk_fc_abstrSwflow_freeEntry_get(int32 *pFlowIdx, uint32 flowHashIdx, uint8 forceGet,rtk_fc_abstrSwFlowPattern_entry_t *swFlowPattern);
rtk_fc_ret_t rtk_fc_abstrHwflow_freeEntry_get(uint32 *pFlowIdx, uint32 flowHashIdx, rtk_fc_tableFlowEntry_t flowPathEntry, uint8 isMulticast, uint8 notTcam, uint8 forceGet);

rtk_fc_ret_t _rtk_fc_flow_freeEntry_get(uint32 *pFlowIdx, uint32 flowHashIdx, rtk_fc_tableFlowEntry_t flowPathEntry, uint8 isMulticast, uint8 notTcam, uint8 addHwOnly, uint8 addSwOnly, uint8 forceGet, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, uint8 isHitSwFwdedAclTrapPri);
rtk_fc_ret_t _rtk_fc_flow_chenkingTime_set(uint32 flowSyncPeriod);
#endif

// flow learning
void _rtk_fc_tuple_set(struct nf_conntrack_tuple *pTuple, rtk_fc_pktHdr_t *pPktHdr);
rtk_fc_ret_t _rtk_fc_flow_direction(rtk_fc_pktHdr_t *pPktHdr, rtk_fc_ingress_data_t *ingress_data);
rtk_fc_ret_t _rtk_fc_flow_fwdType(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_ingress_data_t *ingress_data, uint8 isRouting);


//tracefilter
void _rtk_fc_trace_filter_flowTracker(uint32 flowidx);
void _rtk_fc_trace_filter_compare(struct rt_skbuff *rtskb,rtk_fc_pktHdr_t *pPktHdr, rtk_fc_trafilter_stage_t isIgr);


//string parsing function
void _rtk_fc_str2mac(unsigned char *mac_string,rtk_mac_t *pMacEntry);
int32 _rtk_fc_strToMac(uint8 *pMac, int8 *pStr);


void _rtk_fc_displayInfo(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, fc_rx_info_t *pRxInfo);
void _rtk_fc_displayInfo_toPs(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, fc_rx_info_t *pRxInfo, rtk_fc_toPsReason_t toPsReason);
void _rtk_fc_record_ingress_statistic(rtk_fc_pktHdr_t *pPktHdr);
void _rtk_fc_record_egress_statistic(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr);
void _rtk_fc_record_shortcut_statistic(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_tableFlow_t *pFlow);
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
uint32 _rtk_fc_flow_intf_mapping_idx_get(uint32 hwNetifIdx);
#if RTK_FC_TABLESIZE_OVERFLOW_FLOW
int _rtk_fc_overFlowHashListAdd(uint32 addFlowIdx);
int _rtk_fc_overFlowHashListDel(uint32 delFlowIdx);
#endif
#endif
int _rtk_fc_swFlowListAdd(uint32 flowHashListIdx, uint32 addFlowIdx);
int _rtk_fc_swFlowListDel(uint32 flowHashListIdx, uint32 delFlowIdx);

rtk_fc_ret_t _rtk_fc_abstrSwShortCutCheck(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr);
rtk_fc_ret_t _rtk_fc_shortCut_earlyCheck(struct rt_skbuff *rtskb, fc_rx_info_t *pRxInfo, rtk_fc_pktHdr_t *pPktHdr);
rtk_fc_ret_t _rtk_fc_shortCutCheck(struct rt_skbuff *rtskb, fc_rx_info_t *pRxInfo, rtk_fc_pktHdr_t *pPktHdr);

rtk_fc_ret_t _rtk_fc_ackDelay_check(struct rt_skbuff *rtskb, fc_rx_info_t *pRxInfo, rtk_fc_pktHdr_t *pPktHdr);
void rtk_fc_sw_ackDelay_kicktx_timerfunc(unsigned long task_priv);

void rtk_fc_sw_shaper_kicktx_timerfunc(unsigned long meter_Idx);

#if defined(CONFIG_SMP)
int rtk_fc_nic_tx_smp_dispatch(rtk_fc_smp_nicTx_private_t *nicTx_info);
void rtk_fc_nic_tx_smp_tasklet(void *info);
void rtk_fc_nic_tx_smp_exec(unsigned long data);
#endif

rtk_fc_ret_t rtk_fc_flow_info_natloopback_alloc(rtk_fc_flow_info_natloopback_t *natloopbackEnt);
rtk_fc_ret_t rtk_fc_flow_info_natloopback_get(uint32 flowEntIdx, rtk_fc_flow_info_natloopback_t **natloopbackEnt);
rtk_fc_ret_t rtk_fc_flow_info_natloopback_free(uint32 flowEntIdx);
rtk_fc_ret_t rtk_fc_flow_info_lutivl_ref(uint16 pvid, uint16 sa_idx, uint32 *lutivl_sel_idx);
rtk_fc_ret_t rtk_fc_flow_info_lutivl_deref(uint16 sa_idx, uint32 lutivl_sel_idx);
rtk_fc_ret_t rtk_fc_flow_info_lutivl_trf_set(uint16 sa_idx, uint32 lutivl_sel_idx);
rtk_fc_ret_t rtk_fc_sw_gemmib_filtering_ingress(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr);
rtk_fc_ret_t rtk_fc_sw_gemmib_filtering_egress(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_shaping_destType_t txDest, void *pTxPriv);
int rtk_fc_sw_shaper_skip_specific_tcp(rtk_fc_pktHdr_t *pPktHdr);
//rtk_fc_ret_t rtk_fc_sw_shaper(rtk_fc_shaping_destType_t txDest, void *pTxPriv, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, bool isIngress, bool isEgress);
uint32 rtk_fc_nic_send_with_txInfo(struct rt_skbuff *rtskb, fc_tx_info_t* ptxInfo, int ring_num, rtk_fc_pktHdr_t* pPktHdr);
rtk_fc_ret_t rtk_fc_wifi_send_with_wlandevidx(rtk_fc_wlan_devidx_t wlan_dev_idx, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t* pPktHdr, uint8 outputQid);
int rtk_fc_gmac_svlan_tpid_enable(uint16 svlan_tpid, int vlan_register);
int rtk_fc_skbVlanTag_update(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint16 vlanID, uint8 isStag);
int rtk_fc_inner_skbVlanTag_insert(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint16 vlanTCI, uint8 isStag, uint16 stpid);
int rtk_fc_skbVlanTag_insert(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint16 vlanTCI, uint8 isStag, uint16 stpid);
int rtk_fc_skbVlanTag_remove(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint8 isStag);
int rtk_fc_skbVlanTag_fast_insert(struct rt_skbuff *rtskb, uint16 insertOffset,uint16 tpid ,uint16 vlanTCI);
int rtk_fc_skbVlanTag_fast_remove(struct rt_skbuff *rtskb, uint16 removeOffset);
int _rtk_fc_netif_dummy_packet_set(uint32 hwNetifIdx);
int rtk_fc_netif_dummy_packet_clear(uint32 hwNetifIdx);

#if defined(CONFIG_RTK_L34_G3_PLATFORM)
int rtk_fc_abstrSwSyncToHw_g3McReFlushALL(void);

uint32 _rtk_rg_flowHashPreProcessPort(uint16 port, uint32 cf_ptn);
uint32 _rtk_rg_flowHashPreProcessIP(uint32 ip, uint32 cf_ptn);
uint32 _rtk_rg_flowHashIPv6DstAddr_get(uint8 ipDes[16]);
uint32 _rtk_rg_flowHashIPv6SrcAddr_get(uint8 ipSrc[16]);
uint32 _rtk_rg_fbMode_get(void);
int32 _rtk_fc_g3_l2cls_for_hostPoliceControl_add(int hostPolIdx);
int32 _rtk_fc_g3_l2cls_for_hostPoliceControl_del(int hostPolIdx);
int32 _rtk_fc_g3L3Policer_set(uint32 index, uint32 rate, rtk_enable_t ifgInclude);
int32 _rtk_fc_g3L3Policer_get(uint32 index, uint32 *pRate , rtk_enable_t *pIfgInclude);
void _rtk_fc_g3FlowIdDecision(rtk_rg_asic_path1_entry_t *flow, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, bool *flowIdActMask, uint16 *flowId);
int32 _rtk_fc_g3L3PolicerBurstSize_set(uint32 index, uint32 bucketSize);
int32 _rtk_fc_g3L3PolicerBurstSize_get(uint32 index, uint32* pBucketSize);
int32 _rtk_fc_g3L3PolicerRateMode_set(uint32 index, rtk_rate_metet_mode_t meterMode);
int32 _rtk_fc_g3L3PolicerRateMode_get(uint32 index, rtk_rate_metet_mode_t *meterMode);
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
int32 _rtk_fc_g3L3Policer2_set(uint32 index, uint32 rate, rtk_enable_t ifgInclude);
int32 _rtk_fc_g3L3Policer2_get(uint32 index, uint32 *pRate , rtk_enable_t *pIfgInclude);
int32 _rtk_fc_g3L3Policer2BurstSize_set(uint32 index, uint32 bucketSize);
int32 _rtk_fc_g3L3Policer2BurstSize_get(uint32 index, uint32* pBucketSize);
int32 _rtk_fc_g3L3Policer2RateMode_set(uint32 index, rtk_rate_metet_mode_t meterMode);
int32 _rtk_fc_g3L3Policer2RateMode_get(uint32 index, rtk_rate_metet_mode_t *meterMode);
int32 _rtk_fc_g3L3Policer3_set(uint32 index, uint32 rate, rtk_enable_t ifgInclude);
int32 _rtk_fc_g3L3Policer3_get(uint32 index, uint32 *pRate , rtk_enable_t *pIfgInclude);
int32 _rtk_fc_g3L3Policer3BurstSize_set(uint32 index, uint32 bucketSize);
int32 _rtk_fc_g3L3Policer3BurstSize_get(uint32 index, uint32* pBucketSize);
int32 _rtk_fc_g3L3Policer3RateMode_set(uint32 index, rtk_rate_metet_mode_t meterMode);
int32 _rtk_fc_g3L3Policer3RateMode_get(uint32 index, rtk_rate_metet_mode_t *meterMode);
int32 _rtk_fc_g3PolRemap_sync(int hw_index);
int32 _rtk_fc_dmaAftAction_add(uint32* index, rtk_rg_asic_dmaAftFib_t fib, uint8 is_rsv);
int32 _rtk_fc_dmaAftAction_del(uint32 index);
int32 _rtk_fc_flow_dmaAftAction_update(rtk_rg_asic_path3_entry_t *pFlow, uint32 flowIdx, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo);
#endif

#endif // CONFIG_RTK_L34_G3_PLATFORM
void _rtk_fc_g3IgrExtraInfo_init(rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo);
int32 _rtk_fc_checkPortNotExistByPhy(rtk_fc_mac_port_idx_t port);
int32 _rtk_fc_hwFlowMib_get(uint32 flowMibIdx, rtk_fc_flowOrHostmib_counter_t *counter);
int32 _rtk_fc_hwFlowMib_clear(uint32 flowMibIdx);
int32 _rtk_fc_hwHostPolingMib_get(uint32 hostPolingIdx, uint64 *rx_count, uint64 *tx_count);
int32 _rtk_fc_hwHostPolingMib_clear(uint32 hostPolingIdx);
int _rtk_fc_l3Meter_set(rt_rate_ext_meter_type_t meterType, uint32 index, uint32 rate, rtk_enable_t ifgInclude);
int _rtk_fc_l3Meter_get(rt_rate_ext_meter_type_t meterType, uint32 index, uint32 *pRate , rtk_enable_t *pIfgInclude);
int _rtk_fc_l3MeterBucket_set(rt_rate_ext_meter_type_t meterType, uint32 index, uint32 bucketSize);
int _rtk_fc_l3MeterBucket_get(rt_rate_ext_meter_type_t meterType, uint32 index, uint32 *pBucketSize);
int _rtk_fc_l3MeterRateMode_set(rt_rate_ext_meter_type_t meterType, uint32 index, rtk_rate_metet_mode_t meterMode);
int _rtk_fc_l3MeterRateMode_get(rt_rate_ext_meter_type_t meterType, uint32 index, rtk_rate_metet_mode_t *meterMode);
int _rtk_fc_wanAccessLimit_IP_insert(uint32 sip);
int _rtk_fc_wanAccessLimit_IP_delete(uint32 sip);
int _rtk_fc_wanAccessLimit_IP_reset(void);
int _rtk_fc_wanAccessLimit_IP_lookup(uint32 sip);
void _rtk_fc_wanAccessLimit_timer_init(void);
void _rtk_fc_wanAccessLimit_byMac_function(unsigned long task_priv);
void _rtk_fc_wanAccessLimit_byMac_cb(struct neighbour *neigh, void *cookie);
void _rtk_fc_wanAccessLimit_byIP_function(unsigned long task_priv);
void _rtk_fc_wanAccessLimit_byIP_exist_cb(struct neighbour *neigh, void *cookie);
void _rtk_fc_wanAccessLimit_byIP_connected_cb(struct neighbour *neigh, void *cookie);
void _rtk_fc_wanAccessLimit_byIP_cb(struct neighbour *neigh, void *cookie);
void _rtk_fc_sharing_image_flow_structure_convert(rtk_rg_asic_path1_entry_t *pP1Data);
#if defined(CONFIG_FC_RTL9607C_RTL9603CVD_HYBRID)
void _rtk_fc_sharing_image_port_define(void);
#endif


int32 rtk_fc_abstrSwFlowModule_init(void);
int32 rtk_fc_abstrSwFlowPatternSet(void);
int32 rtk_fc_abstrSwFlowPatternGenByPktHdr(rtk_fc_pktHdr_t *pPktHdr,rtk_fc_abstrSwFlowPattern_entry_t *swFlowPattern,uint8 flowTye);
int32 rtk_fc_abstrSwFlowPatternGenBySkb(rtk_fc_pktHdr_t *pPktHdr,struct rt_skbuff *rtskb,rtk_fc_abstrSwFlowPattern_entry_t *swFlowPattern,uint8 flowTye);
rtk_fc_abstrSwFlowList_entry_t* rtk_fc_abstrSwFlowAlloc(rtk_fc_abstrSwFlowPattern_entry_t *swFlowKey);
rtk_fc_abstrSwFlowActionList_entry_t* rtk_fc_abstrSwFlowActionAlloc(rtk_fc_abstrSwFlowAction_entry_t *swFlowAct);
rtk_fc_abstrSwFlowLdpid_entry_t* rtk_fc_abstrSwFlowLdpidAlloc(void);
int rtk_fc_abstrSwFlowLdpidFree(rtk_fc_abstrSwFlowLdpid_entry_t* pLdpid);
int rtk_fc_abstrSwFlowActionFree(rtk_fc_abstrSwFlowActionList_entry_t *pSwFlowAct);
int rtk_fc_abstrSwFlowFree(rtk_fc_abstrSwFlowList_entry_t *pSwFlowList);
int32 rtk_fc_abstrSwFlowActGenBySkb(rtk_fc_pktHdr_t *pPktHdr,struct rt_skbuff *rtskb,rtk_fc_abstrSwFlowAction_entry_t *action);
bool rtk_fc_abstrSwFlowActionCompare(int isIPv6,rtk_fc_abstrSwFlowAction_entry_t *swFlowAct_1,rtk_fc_abstrSwFlowAction_entry_t *swFlowAct_2);
int rtk_fc_delete_mc_macId(int swMacid);

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)	
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
int rtk_fc_abstrSwSyncToHw_WifiMcToUc(rtk_fc_mcExtraSwFlowIdx_entry_t *p_mcSwFlowIdx, uint8 *dmac, portForwardActionVlanExtend_t *pfwdAction, uint32 *amsdu_h_idx, uint32 *swMacId, uint32 updateFlow);
#endif
#endif

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
extern int re8686_set_flow_control(unsigned int gmac, unsigned int ring, unsigned char enable);
#if defined(CONFIG_RTL8686_SWITCH)
extern void re8686_set_pauseBySw(unsigned char enable);
#endif
#endif

#if defined(CONFIG_RTK_SOC_RTL8198D)
int rtk_fc_flow_limit_set(bool swOnly, uint32 in_path, int limit);
int rtk_fc_flow_limit_check(bool *swOnly, uint32 in_path);
int rtk_fc_flow_limit_count_update(bool swOnly, uint32 in_path, bool op_add);
int rtk_fc_flow_limit_init(void);
#endif

rtk_fc_ret_t rtk_fc_ipFrag_cache_add(rtk_fc_pktHdr_t *pktHdr);
rtk_fc_ret_t rtk_fc_ipFrag_cache_del(rtk_fc_ipFrag_linkList_t *ipFragList);
rtk_fc_ret_t rtk_fc_ipFrag_cache_find(rtk_fc_pktHdr_t *pktHdr);
rtk_fc_ret_t rtk_fc_negative_ipFrag_cache_add(rtk_fc_pktHdr_t *pktHdr);
rtk_fc_ret_t rtk_fc_negative_ipFrag_cache_find(rtk_fc_pktHdr_t *pktHdr);
rtk_fc_ret_t rtk_fc_ipFrag_cache_timeout(unsigned long task_priv);
rtk_fc_ret_t rtk_fc_negtive_ipFrag_cache_timeout(unsigned long task_priv);
rtk_fc_ret_t rtk_fc_ipfrag_ingress_del_cache(rtk_fc_pktHdr_t *pktHdr);
rtk_fc_ipFrag_linkList_t *rtk_fc_ipfrag_shortcut_find_cache(rtk_fc_pktHdr_t *pktHdr);

int _rtk_fc_skb_cow_head_and_pktHdr_update(struct sk_buff *skb, unsigned int headroom, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr);
int _rtk_fc_swPktHdrModify(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_tableFlow_t *pFlowEnt);

rtk_fc_ret_t _rtk_fc_page_alloc(rtk_fc_page_t *pPageInfo);
rtk_fc_ret_t _rtk_fc_page_free(rtk_fc_page_t *pPageInfo);
#if defined(CONFIG_RTK_FC_HTTP_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
void _rtk_fc_pe_http_test_free_resource(void);
#endif

#endif //__RTK_FC_INTERNAL__

