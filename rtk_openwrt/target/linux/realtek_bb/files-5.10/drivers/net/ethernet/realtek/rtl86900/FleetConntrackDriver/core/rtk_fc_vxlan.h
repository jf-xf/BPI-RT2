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



#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL9607C_SERIES)

void _rtk_fc_vxlan_8277b_reservedACL_del(void);
int _rtk_fc_vxlan_special_fastFwd_upstream_setting(rtk_fc_pktHdr_t *pPktHdr, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, int useExtra);
int _rtk_fc_vxlan_special_fastFwd_downstream_setting(rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo);
int _rtk_fc_l2_vxlan_up_stream_outer_flow_prepare(rtk_rg_asic_path1_entry_t *pFlowEntry, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, int outter_out_intf_idx);

int _rtk_fc_vxlan_up_stream_outer_flow_prepare(rtk_rg_asic_path1_entry_t *pFlowEntry, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, int outter_out_intf_idx);
int _rtk_fc_vxlan_down_stream_outer_flow_prepare(rtk_rg_asic_path3_entry_t *pFlowPath3, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint32 flowIdx,int igrNetifIdx,int egrNetifIdx, int is_checking_flow_can_add);
void _rtk_fc_vxlan_8277b_reservedACL_del_by_idx(int index, int direction);
void _rtk_fc_vxlan_hwIssue_workAround_setting(void);
int _rtk_fc_vxlan_processing(rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, int egrNetifIdx,
									rtk_rg_asic_path1_entry_t *pFlowEntry, struct rt_skbuff *rtskb, int igr_netifIdx, struct rt_nfconn *rtct, rtk_fc_igrExtraInfo_t *pExtraInfo, int16 outer_daIdx,
									uint32 swOnly_flowIdx);
void _rtk_fc_dualHeader_vxlan_get_fastFwd_outer_content(rtk_fc_pktHdr_t *pPktHdr);
int _rtk_fc_l2Dual_vxlan_flow_del(rtk_fc_tableFlow_t *pFlowTable, int flowIdx);

#endif


#if defined(CONFIG_FC_RTL9607C_SERIES)
int rtk_fc_vxlan_customized_upstreamTxHook(struct re_private *cp, struct tx_info *pTxInfo, int len);
int rtk_fc_vxlan_customized_rxHook(struct re_private *cp, struct rx_info *pRxInfo, int len);
int rtk_fc_vxlan_customized_txPrepare(struct re_private *cp, struct tx_info *pTxInfo);
int rtk_fc_vxlan_customized_extraTxPrepare(struct re_private *cp, struct tx_info *pTxInfo);
int rtk_fc_vxlan_customized_rxPrepare(struct re_private *cp, struct rx_info *pRxInfo);
int _rtk_fc_vxlan_acceleration_mechanism_set(int value);
int _rtk_fc_vxlan_acceleration_extraPmsk_set(unsigned int pmsk);
int _rtk_fc_vxlan_up_stream_innerFlow_prepare(rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, rtk_fc_pktHdr_t *pPktHdr, rtk_rg_asic_path5_entry_t *pFlowPath5,struct rt_skbuff *rtskb);

#endif
#if 0 
int rtk_fc_vxlan_inner_skbVlanTag_insert(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint16 vlanTCI, uint8 isStag, uint32 l2DualTbl_index);
int rtk_fc_vxlan_inner_skbVlanTag_remove(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint8 isStag, uint32 l2DualTbl_index);
#endif
//int rtk_fc_l2Dual_table_setting(rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, rtk_fc_pktHdr_t *pPktHdr, uint32 *index);


void _rtk_fc_vxlan_ingress_interface_dataCaching(rtk_fc_pktHdr_t *pPktHdr);

rtk_fc_ret_t rtk_fc_vxlan_get_outerNetif_index(rtk_fc_pktHdr_t *pPktHdr, int direction);

rtk_fc_ret_t rtk_fc_vxlan_get_innerNetif_index(rtk_fc_pktHdr_t *pPktHdr, int direction);

int _rtk_fc_vxlan_downstream_pktHdr_recover(rtk_fc_pktHdr_t *pPktHdr, int sw_inner_inf_idx, int sw_outer_intf_idx);


#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)



int _rtk_fc_vxlan_8277C_downStream_reservedACL_add(int *result_idx, rtk_fc_aclAndCf_reserved_dual_hdr_chk_t *dual_hdr_chk);
int rtk_fc_vxlan_8277C_upstream_flow_setting(rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, int igrNetifIdx, int egrNetifIdx, rtk_fc_pktHdr_t *pPktHdr, bool swOnly);
int rtk_fc_vxlan_8277C_downstream_flow_setting(rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, int igrNetifIdx, int egrNetifIdx, rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr, bool swOnly);
int rtk_fc_vxlan_get_outer_content(int outer_header_size, rtk_fc_pktHdr_t *pPktHdr);

#endif


