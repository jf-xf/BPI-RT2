#ifndef __RTK_RG_APOLLOPRO_LITEROMEDRIVER_H__
#define __RTK_RG_APOLLOPRO_LITEROMEDRIVER_H__

#include <rtk_rg_liteRomeDriver.h>
#include <rtk_rg_fwdEngine.h>
#include <rtk_rg_apollo_liteRomeDriver.h>
#include <rtk_rg_mappingAPI.h>


rtk_rg_err_code_t rtk_rg_apolloPro_interfaceMibCounter_del(int intf_idx);
rtk_rg_err_code_t rtk_rg_apolloPro_interfaceMibCounter_get(rtk_rg_netifMib_entry_t *pNetifMib);
rtk_rg_err_code_t rtk_rg_apolloPro_hostPoliceControl_set(rtk_rg_hostPoliceControl_t *pHostPoliceControl, int host_idx);
rtk_rg_err_code_t rtk_rg_apolloPro_hostPoliceControl_get(rtk_rg_hostPoliceControl_t *pHostPoliceControl, int host_idx);
rtk_rg_err_code_t rtk_rg_apolloPro_hostPoliceLogging_get(rtk_rg_hostPoliceLogging_t *pHostMibCnt, int host_idx);
rtk_rg_err_code_t rtk_rg_apolloPro_hostPoliceLogging_del(int host_idx);
rtk_rg_err_code_t rtk_rg_apolloPro_macEntry_add(rtk_rg_macEntry_t *macEntry, int *entry_idx);
rtk_rg_err_code_t rtk_rg_apolloPro_flowMibCounter_get(int index, rtk_rg_table_flowmib_t *pCounter);
rtk_rg_err_code_t rtk_rg_apolloPro_flowMibCounter_reset(int index);

rtk_rg_err_code_t rtk_rg_apolloPro_svlanTpid2_enable_set(rtk_rg_enable_t enable);
rtk_rg_err_code_t rtk_rg_apolloPro_svlanTpid2_enable_get(rtk_rg_enable_t *pEnable);
rtk_rg_err_code_t rtk_rg_apolloPro_svlanTpid2_set(uint32 svlan_tag_id);
rtk_rg_err_code_t rtk_rg_apolloPro_svlanTpid2_get(uint32 *pSvlanTagId);


rtk_rg_err_code_t rtk_rg_apolloPro_multicastFlow_find(rtk_rg_multicastFlow_t *mcFlow, int *valid_idx);
rtk_rg_err_code_t rtk_rg_apolloPro_multicastFlow_del(int flow_idx);
rtk_rg_err_code_t rtk_rg_apolloPro_multicastFlow_add(rtk_rg_multicastFlow_t *mcFlow, int *flow_idx);
rtk_rg_err_code_t rtk_rg_apolloPro_l2MultiCastFlow_add(rtk_rg_l2MulticastFlow_t *l2McFlow,int *flow_idx);

rtk_rg_err_code_t rtk_rg_apolloPro_funcbasedMeter_set(rtk_rg_funcbasedMeterConf_t meterConf);
rtk_rg_err_code_t rtk_rg_apolloPro_funcbasedMeter_get(rtk_rg_funcbasedMeterConf_t *meterConf);
rtk_rg_err_code_t rtk_rg_apolloPro_HiPriEntry_add(rtk_rg_table_highPriPatten_t hiPriEntry,int *entry_idx);
rtk_rg_err_code_t rtk_rg_apolloPro_HiPriEntry_del(int entry_idx);
rtk_rg_err_code_t rtk_rg_apolloPro_UrlPri_add(rtk_rg_urlHighPri_t* urlPriEt,int *entry_idx);
rtk_rg_err_code_t rtk_rg_apolloPro_UrlPri_del(int entry_idx);



//internal function prototype

int32 _rtk_rg_decideEgressMcastNetIfIdx(uint32 egressPmsk);
int32 _rtk_rg_decideIngressMcastNetIfIdx(uint32 spa);
rtk_rg_err_code_t _rtk_rg_multicastRecordFlowData(rtk_rg_pktHdr_t *pPktHdr,uint32 dpMask,uint32 allMask);
rtk_rg_err_code_t _modifyPacketByMulticastDecision(struct sk_buff *skb,rtk_rg_pktHdr_t *pPktHdr,uint32 txPmsk);
rtk_rg_err_code_t _flowFillDsliteMulticastToPath6(rtk_rg_pktHdr_t * pPktHdr);
rtk_rg_err_code_t rtl_rg_mcDataReFlushAndAdd(uint32* GroupIp);


int _rtk_rg_hardwareArpTableLookUp(unsigned short routingIdx, ipaddr_t ipAddr, rtk_rg_arp_linkList_t **phardwareArpEntry, int resetIdleTime);
int _rtk_rg_hardwareArpTableAdd(unsigned short routingIdx, ipaddr_t ipv4Addr, int l2Idx, int staticEntry, uint16 *hwArpIdx);
int _rtk_rg_hardwareArpTableDel(rtk_rg_arp_linkList_t *pDelArpEntry);
int32 _rtk_rg_deleteHwARP(int intfIdx, int routingIdx);
int _rtk_rg_lutCamListAdd(int l2Idx, uint32 addLutCamIdx);
int _rtk_rg_lutCamListDel(uint32 delLutCamIdx);
rtk_rg_entryGetReturn_t _rtk_rg_findAndReclamIPv6mcEntry( rtk_ipv6_addr_t *gipv6,int32 ivlsvl);
rtk_rg_entryGetReturn_t _rtk_rg_extPMaskEntry_find(uint32 extPMask);
rtk_rg_entryGetReturn_t _rtk_rg_extPMaskEntryIdx_get(uint32 extPMask);
rtk_rg_entryGetReturn_t _rtk_rg_indirectMacEntry_find(int l2Idx);
rtk_rg_err_code_t _rtk_rg_netifAllowIgrPMask_set(int netifIdx, uint32 igrPMask, uint32 igrExtPMask);
rtk_rg_err_code_t _rtk_rg_netifPPPoESession_set(int netifIdx, rtk_rg_asic_netifpppoeAct_t action, uint32 sessionId);
rtk_rg_err_code_t _rtk_rg_l34WanAccessLimit_tableRst(void);
rtk_rg_err_code_t _rtk_rg_l34WanAccessLimit(rtk_rg_pktHdr_t *pPktHdr);

unsigned int _hash_dip_vidfid_sipidx_sipfilter(int is_ivl,unsigned int dip,unsigned int vid_fid, unsigned int sip_idx, unsigned int sip_filter_en);
unsigned int _hash_ipm_dipv6(uint8 *dip);
#if 0 // for compiler
int32 _rtk_rg_apollo_ipv6MultiCastFlow_add(rtk_rg_ipv6MulticastFlow_t *ipv6McFlow,int *flow_idx);
#endif
void _rtk_rg_IntfIdxDecisionForCF(rtk_rg_pktHdr_t *pPktHdr);

int _rtk_rg_extraTagActionList_empty_find(void);
int _rtk_rg_extraTagActionList_apply(int actionListIdx, int intfIdx);
int _rtk_rg_extraTagActionList_clear(int actionListIdx);
int _rtk_rg_extraTag_setupPPTP(int wan_intf_idx, uint8 *outerHdr, int *extraTagActListIdx);
int _rtk_rg_extraTag_setupL2TP(int wan_intf_idx, uint8 *outerHdr, int *extraTagActListIdx);
int _rtk_rg_extraTag_setupDsLite(int wan_intf_idx, uint8 *outerHdr, int *extraTagActListIdx);
rtk_rg_err_code_t _rtk_rg_flow_syncTunnelWANBasedWAN(int tunnelIntfIdx, int baseIntfIdx);
int _rtk_rg_flow_deleteTunnelConfig(int wan_intf_idx);
int _rtk_rg_flow_setupPPTP(int wan_intf_idx, int gmacL2Idx);
int _rtk_rg_flow_setupL2TP(int wan_intf_idx, int gmacL2Idx);
int _rtk_rg_flow_setupDSLite(int wan_intf_idx, int gmacL2Idx, uint8 overPPPoE);
int32 _rtk_rg_determind_fb_flow_hash_flexible_pattern(void);
uint32 _rtk_rg_fb_init(void);
rtk_rg_err_code_t rtk_rg_apolloPro_portBasedCVlanId_get(rtk_rg_port_idx_t port_idx,int *pPvid);

#endif
