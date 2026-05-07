/*
 * Copyright (C) 2017 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*/

#include "rtk_fc_struct.h"


//====================================================================================================================================//
// DUAL HDR CALL BACK FUNCTION

//register in fc_db.dualCallBackFunc[].dual_hash_patten_fill
void rtk_fc_dualHash_ip4Inip6(rtk_fc_dualHashKey_t* patten,rtk_fc_pktHdr_t *pPktHdr,rtk_fc_flow_dual_direction_t dual_direct);
void rtk_fc_dualHash_PPTP(rtk_fc_dualHashKey_t* pattern,rtk_fc_pktHdr_t *pPktHdr,rtk_fc_flow_dual_direction_t dual_direct);
void rtk_fc_dualHash_L2TP(rtk_fc_dualHashKey_t* pattern, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_flow_dual_direction_t dual_direct);
void rtk_fc_dualHash_IPSEC(rtk_fc_dualHashKey_t* patten,rtk_fc_pktHdr_t *pPktHdr,rtk_fc_flow_dual_direction_t dual_direct);
void rtk_fc_dualHash_GRE_ETH_BR(rtk_fc_dualHashKey_t* patten,rtk_fc_pktHdr_t *pPktHdr,rtk_fc_flow_dual_direction_t dual_direct);
void rtk_fc_dualHash_VXLAN(rtk_fc_dualHashKey_t* patten,rtk_fc_pktHdr_t *pPktHdr,rtk_fc_flow_dual_direction_t dual_direct);
void rtk_fc_dualHash_6RD(rtk_fc_dualHashKey_t* patten,rtk_fc_pktHdr_t *pPktHdr,rtk_fc_flow_dual_direction_t dual_direct);
void rtk_fc_dualHash_MAPT(rtk_fc_dualHashKey_t* patten,rtk_fc_pktHdr_t *pPktHdr,rtk_fc_flow_dual_direction_t dual_direct);
void rtk_fc_dualHash_XLAT(rtk_fc_dualHashKey_t* patten,rtk_fc_pktHdr_t *pPktHdr,rtk_fc_flow_dual_direction_t dual_direct);

//register in fc_db.dualCallBackFunc[].dual_netifInfo_collect
int rtk_fc_netif_dual_set_4in6(uint32 swNetifIdx,rtk_fc_flow_dual_direction_t dual_direct, struct sk_buff *skb, rtk_fc_pktHdr_t *pPktHdr);
int rtk_fc_netif_dual_set_PPTP(uint32 swNetifIdx,rtk_fc_flow_dual_direction_t dual_direc, struct sk_buff *skb, rtk_fc_pktHdr_t *pPktHdr);
int rtk_fc_netif_dual_set_L2TP(uint32 swNetifIdx, rtk_fc_flow_dual_direction_t dual_direc, struct sk_buff *skb, rtk_fc_pktHdr_t *pPktHdr);
int rtk_fc_netif_dual_set_GRE_ETH_BR(uint32 swNetifIdx,rtk_fc_flow_dual_direction_t dual_direc, struct sk_buff *skb, rtk_fc_pktHdr_t *pPktHdr);
int rtk_fc_netif_dual_set_VXLAN(uint32 swNetifIdx,rtk_fc_flow_dual_direction_t dual_direc, struct sk_buff *skb, rtk_fc_pktHdr_t *pPktHdr);
int rtk_fc_netif_dual_set_6RD(uint32 swNetifIdx,rtk_fc_flow_dual_direction_t dual_direc, struct sk_buff *skb, rtk_fc_pktHdr_t *pPktHdr);
int rtk_fc_netif_dual_set_MAPT(uint32 swNetifIdx,rtk_fc_flow_dual_direction_t dual_direc, struct sk_buff *skb, rtk_fc_pktHdr_t *pPktHdr);
int rtk_fc_netif_dual_set_XLAT(uint32 swNetifIdx,rtk_fc_flow_dual_direction_t dual_direc, struct sk_buff *skb, rtk_fc_pktHdr_t *pPktHdr);

//register in fc_db.dualCallBackFunc[].dual_hw_setting
int _rtk_fc_flow_setup6RD(rtk_fc_pktHdr_t *pPktHdr,struct sk_buff *skb, int wan_intf_idx);
int _rtk_fc_flow_setupGreEthBr(rtk_fc_pktHdr_t *pPktHdr,struct sk_buff *skb, int hw_intf_idx);
int _rtk_fc_flow_setupIP4InIP6(rtk_fc_pktHdr_t *pPktHdr,struct sk_buff *skb, int wan_intf_idx);
int _rtk_fc_flow_setupL2TP(rtk_fc_pktHdr_t *pPktHdr,struct sk_buff *skb, int wan_intf_idx);
int _rtk_fc_flow_setupPPTP(rtk_fc_pktHdr_t *pPktHdr,struct sk_buff *skb, int wan_intf_idx);
int _rtk_fc_flow_setupVXLAN(rtk_fc_pktHdr_t *pPktHdr,struct sk_buff *skb, int hw_intf_idx);
int _rtk_fc_flow_setupMAPT(rtk_fc_pktHdr_t *pPktHdr,struct sk_buff *skb, int hwNetif);
int _rtk_fc_flow_setupXLAT(rtk_fc_pktHdr_t *pPktHdr,struct sk_buff *skb, int hwNetif);
//====================================================================================================================================//



int _rtk_fc_netif_greEthBr_info_set(uint32 swNetifIdx, uint32 hwNetifIdx, rtk_fc_dev_type_t devType, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr);
// ingress/egress learning
int rtk_fc_ingress_outerHdrPathUpdate(rtk_fc_pktHdr_t *pPktHdr, uint32 path6Idx);
int rtk_fc_dual_hw_setting(rtk_fc_pktHdr_t *pPktHdr,struct sk_buff *skb, uint32 netifIdx, int32 *extraTagIdx);
int rtk_fc_mapt_accelerate(rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr, uint32 igrNetifIdx, uint32 egrNetifIdx);
unsigned int rtk_fc_netif_dualHashKey_config_byPktHdr(rtk_fc_dualHdrtype_t dualType, rtk_fc_pktHdr_t *pPktHdr,rtk_fc_flow_dual_direction_t dual_direct);

// shortcut operation
int rtk_fc_mapt_translate(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_tableFlow_t *pFlowTable);
int rtk_fc_outerHdr_insert(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_tableFlow_t *pFlowTblEnt, bool *ifSkipNicL2Offload);
int rtk_fc_outerHdr_remove(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_tableFlow_t *pFlowTblEnt);
#if 0
int rtk_fc_l2Dual_outerHdr_remove(struct rt_skbuff *rtskb, uint8 outer_tag_len, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_tableFlow_t *pFlowTable);
int rtk_fc_l2Dual_outerHdr_insert(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_tableFlow_t *pFlowTable);
#endif
// Others
int rtk_fc_egress_tunnelInfoSync(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr);
int rtk_fc_netif_dualConfig_del(int swIntfIdx);
int rtk_fc_dualInfoInit(void);

// mape extra header
int rtk_fc_mape_dst6Info_init(void);
int rtk_fc_is_mape_upstream_flow(uint32 flowIdx);
void rtk_fc_mape_dst6_update(rtk_fc_pktHdr_t *pPktHdr, uint32 outif_idx, uint16 fmr_idx);
int rtk_fc_mape_dst6_decision(rtk_fc_pktHdr_t *pPktHdr, uint32 flowIdx);
int rtk_fc_mape_dst6_refcnt_put(uint32 flowIdx);
void rtk_fc_netif_dualinfo_setting(rtk_fc_dev_type_t devType,rtk_fc_flow_direction_t dir ,uint32 swNetifIdx, struct sk_buff *skb, rtk_fc_pktHdr_t *pPktHdr,int devStackingCnt,struct net_device**  devArray);
void rtk_fc_dual_4in6NetifInfo_update_by_netdev_priv(uint32 swNetifIdx,struct net_device *dev);
uint32 rtk_fc_dualHashKey(rtk_fc_dualHdrtype_t dualType ,rtk_fc_dualHashKey_t *key);
int rtk_fc_netif_update_dualinfo_by_dualHashKey(rtk_fc_pktHdr_t *pPktHdr,struct sk_buff *skb);

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
rtk_fc_ret_t rtk_fc_dual_control_entry_setVXLAN(int idx, rtk_fc_pktHdr_t *pPktHdr);
rtk_fc_ret_t rtk_fc_dual_control_content_setVXLAN(uint32 dual_content_tbl_idx, u8 *outer_header_content, int outer_header_size);

#endif
