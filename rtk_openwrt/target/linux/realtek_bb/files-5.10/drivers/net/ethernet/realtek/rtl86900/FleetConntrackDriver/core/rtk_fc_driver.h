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

#ifndef __RTK_FC_DRIVER__
#define __RTK_FC_DRIVER__

#include "rtk_fc_struct.h"


#define RTK_FC_TRACE_FILTER_UNLOCK_CHECK()\
do {\
	if (fc_db.tracefilterCpuOwner[smp_processor_id()]){ \
		fc_db.tracefilterShow = 0U;\
		if(fc_db.debugLevel_tmp){\
			fc_db.debugLevel = fc_db.debugLevel_tmp;\
			fc_db.debugLevel_tmp = 0;\
		}\
        	RTK_FC_HELPER_MGR_TRACEFILTER_SPIN_UNLOCK_BH();\
	}\
} while (0)

int rtk_fc_netif_inetaddr_event(struct notifier_block *this, unsigned long event, void *ptr);
int rtk_fc_netif_inet6addr_event(struct notifier_block *this, unsigned long event, void *ptr);
int rtk_fc_netif_netdev_event(struct notifier_block *this, unsigned long event, void *ptr);

void rtk_fc_switchLinkChangeHandler(intrBcasterMsg_t *pMsgData);
rtk_rg_err_code_t rtk_fc_portStatus_get(rtk_fc_mac_port_idx_t port, rtk_fc_portStatusInfo_t *portInfo);

int rtk_fc_egress_skbMarkSetting(struct rt_skbuff *rtskb, rtk_fc_skbmarkTarget_t target);
int rtk_fc_get_ipv6_nat_indirectMap_ref(rtk_fc_pktHdr_t *pPktHdr, uint32 *i6NatIndirectIndex);
int rtk_fc_get_ipv6_mapt_indirectMap_ref(rtk_fc_pktHdr_t *pPktHdr, rtk_fc_maptIpv6Info_t *maptInfo);
int rtk_fc_get_ipv6Hash_mapt_indirectMap_ref(rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_maptIpv6Info_t *maptInfo);

#if defined(CONFIG_FC_RTL9607C_SERIES)

int _rtk_fc_NPTv6FastForward(char *skbData, rtk_fc_rxdesc_t *pRxDesc, struct sk_buff *skb);
int _rtk_fc_nptv6_ipoe_customized_TxHook(struct re_private *cp, struct tx_info *pTxInfo, int len);
int _rtk_fc_nptv6_pppoe_customized_TxHook(struct re_private *cp, struct tx_info *pTxInfo, int len);
int nic_rx_skb_NPTv6FastForward(struct re_private *cp, struct rx_info *pRxInfo, int len);
int _rtk_fc_nptv6_customized_txPrepare(struct re_private *cp, struct tx_info *pTxInfo);

void _rtk_fc_nptv6_acceleration_mechanism_set(int value);
int rtk_fc_nptv6_fastFwd_outerFlow_prepare(rtk_rg_asic_path3_entry_t *pFlowPath3, int outer_flow_hash_index, rtk_fc_pktHdr_t *pPktHdr,struct rt_skbuff *rtskb, int srcNetifIdx, int egrNetifIdx,rtk_fc_igrExtraInfo_t *extraInfo, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo,struct rt_nfconn *rtct);
int _rtk_fc_get_nptv6_flowInfo_idx(int *idx);
#endif

int rtk_fc_param_init(void);

#if defined(CONFIG_FC_RTL8198F_SERIES)
unsigned int ca_ni_get_wan_port_id(void);
unsigned char ca_ni_get_pe_fc_enable(void);
unsigned char ca_ni_get_pe_port(void);
#endif
#endif // __RTK_FC_DRIVER__




