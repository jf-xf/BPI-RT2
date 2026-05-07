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



#if !defined(__CA_TYPES_H__)	//ca_types.h
#define CA_UINT8_INVALID 0xFF
#define CA_UINT16_INVALID 0xFFFF
#define CA_UINT32_INVALID 0xFFFFFFFF
#endif

rtk_fc_ret_t _rtk_fc_igmp_mld_ingressLan_learning(rtk_fc_pktHdr_t *pPktHdr,rtk_fc_pmap_t  igrPortMap, rtk_fc_wlan_devidx_t igrWlanDevIdx);
rtk_fc_ret_t _rtk_fc_igmp_lutMcAdd(uint8 isIpv6 ,uint32 groupip_LSB_32,uint8 isStatic,int16 *mcL2Idx);
rtk_fc_ret_t _rtk_fc_cpuSpa_macAdd(uint8 *mac,uint8 isStatic,int16 *mcL2Idx);
int rtk_fc_egress_mcCheckPortInConfig(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr,rtk_fc_table_mcConfigTbl_t *p_userconf);

int _rtk_fc_igmp_mld_module_init(void);
int rtk_fc_igmp_mld_dirver_show(struct seq_file *s, void *v);
int rtk_fc_igmp_show_mcExtFlowIdxTbl(struct seq_file *s, void *v);
int _rtk_fc_groupLSB32_to_mac(unsigned char* mac,uint32 isIpv6,uint32 groupip_LSB_32);
int rtk_fc_igmp_mcExtraFlowIdxTbl_free(rtk_fc_mcExtraSwFlowIdx_entry_t* mcSwFlowId_entry);
int rtk_fc_igmp_mcExtraFlowIdxTbl_flashInit(void);
int rtk_fc_igmp_mcExtraFlowIdxTbl_del(int32 swFlowIdx);
rtk_fc_mcExtraSwFlowIdx_entry_t* rtk_fc_igmp_mcExtraFlowIdxTbl_find(int32 swFlowIdx);
rtk_fc_mcExtraSwFlowIdx_entry_t* rtk_fc_igmp_mcExtraFlowIdxTbl_add(rtk_fc_ingress_data_t *pFcIngressData,rtk_fc_pktHdr_t *pPktHdr,struct rt_skbuff *rtskb,int32 swFlowIdx,uint8 isGrpDftFwd);
int32 _rtk_fc_igmp_mc_get_devConfig(rt_igmp_group_devPort_cfg_t *mcConfig);
int32 _rtk_fc_igmp_mc_set_dev_to_dirver(rt_igmp_group_devPort_cfg_t *mcConfig);
int32 _rtk_fc_igmp_mc_set_to_dirver(rt_igmp_multicast_group_cfg_t *mcConfig);
int32 rtk_fc_igmp_sendAllMcDummyPktDetector(unsigned long task_priv);
void rtk_fc_igmp_setupmcDummyPktDetectorTimer(void);
void rtk_fc_igmp_setupKernelSyncTimerEvent(void);
void rtk_fc_igmp_setupKernelSyncTimerPeriod(void);
int rtk_fc_mc2ucCopyCnt_get(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr,int *needSendCnt,rtk_fc_table_mcConfigTbl_t *p_userconf);
int rtk_fc_mc2ucPktModify(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr,int sendCntNum,rtk_fc_table_mcConfigTbl_t *p_userconf);
rtk_fc_table_mcConfigTbl_t* rtk_fc_egress_mcConfigGetByPkt(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr,rtk_fc_table_mcConfigTbl_t* userconf);
int32 rtk_fc_multicast_flowFlush_strategy  (rtk_fc_multicast_flow_flush_t strategy  ,rtk_fc_multicast_flush_patten *patten);


//unknown multicast hw setting
int rtk_fc_igmp_unknownMulticast_action_set(rt_igmp_unknownMc_cfg_t mcUnknCfg);

//known group table access
rtk_fc_table_mcGroupTbl_t* rtk_fc_mc_addKnownGroupEntry(uint32 is_ipv6,uint32 *gip);
rtk_fc_table_mcGroupTbl_t* rtk_fc_mc_findKnownGroupEntry(uint32 is_ipv6,uint32 *gip);
int32 rtk_fc_mc_delKnownGroupEntry(rtk_fc_table_mcGroupTbl_t* mcGroupEt);
int32 _rtk_fc_mc_groupToHwAsKnown(uint32 is_ipv6,uint32 *gip,int32 *hwIdx,int32 *hwIdx2);

//config table access for user mode
rtk_fc_table_mcConfigTbl_t* rtk_fc_mc_mallocMcConfig (uint32 is_ipv6,uint32 *gip,uint8 careSource,uint32 *sip,uint8 careCvid,int16 ingressCvid,uint8 careSvid,int16 ingressSvid);
rtk_fc_table_mcConfigTbl_t* rtk_fc_mc_findMcConfig(uint32 is_ipv6,uint32 *gip,uint8 careSource,uint32 *sip,uint8 careCvid,int16 ingressCvid,uint8 careSvid,int16 ingressSvid);
int rtk_fc_mc_delMcConfig(rtk_fc_table_mcConfigTbl_t* delconf);
rtk_fc_table_mcConfigTbl_t* rtk_fc_mc_searchMcConfigByPatten(uint8 isIpv6 ,uint32 *groupIp ,uint32 *sourceIp,int16 igrCvlan,int16 igrSvlan);
rtk_fc_table_mcConfigTbl_t* rtk_fc_mc_searchDftMcConfigByPktPatten(uint8 isIpv6,uint32 *groupIp,uint32 *sourceIp,int16 igrCvlan,int16 igrSvlan);

