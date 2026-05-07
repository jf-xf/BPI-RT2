#ifndef __RTK_FC_MAPPINGAPI__
#define __RTK_FC_MAPPINGAPI__

#if defined(CONFIG_RTK_L34_G3_PLATFORM)
rtk_fc_ret_t RTK_FC_ASIC_PONRX_TO_L3FE_SET(uint8 enable);
#endif
rtk_fc_ret_t RTK_RG_ASIC_GLOBALSTATE_SET(rtk_rg_asic_globalStateType_t stateType, rtk_enable_t state);
rtk_fc_ret_t RTK_RG_ASIC_PREHASHPTN_SET(rtk_rg_asic_preHashPtn_t ptnType, uint32 code);
rtk_fc_ret_t RTK_RG_ASIC_FLOWPATH_INVALID(uint32 idx);
rtk_fc_ret_t RTK_RG_ASIC_FLOWPATH_DEL(uint32 idx);
rtk_fc_ret_t RTK_RG_ASIC_FLOWPATH_SET(uint32 *idx, void *pData, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, uint8 swOnly);
rtk_fc_ret_t RTK_RG_ASIC_EXTPORTMASKTABLE_ADD(uint32 extPMask, int32 *extPMaskIdx);
rtk_fc_ret_t RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(int32 extPMaskIdx);
rtk_fc_ret_t RTK_RG_ASIC_EXTRATAGACTION_ADD(uint32 actionList, uint32 actionIdx, rtk_rg_asic_extraTagAction_t *pExtraTagAction);
rtk_fc_ret_t RTK_RG_ASIC_EXTRATAGACTION_DEL(uint32 actionList, uint32 actionIdx);
rtk_fc_ret_t RTK_RG_ASIC_EXTRATAGACTION_GET(uint32 actionList, uint32 actionIdx, rtk_rg_asic_extraTagAction_t *pExtraTagAction);
rtk_fc_ret_t RTK_RG_ASIC_EXTRATAGCONTENTBUFFER_SET(uint32 actionList, char *pContentBuffer);
rtk_fc_ret_t RTK_RG_ASIC_EXTRATAGCONTENTBUFFER_GET(uint32 offset, uint32 len, char *pContentBuffer);
rtk_fc_ret_t RTK_RG_ASIC_NETIFTABLE_ADD(uint32 hwIdx, rtk_rg_asic_netif_entry_t *intfEntry);
rtk_fc_ret_t RTK_RG_ASIC_NETIFTABLE_DEL(uint32 hwIdx);
rtk_fc_ret_t RTK_RG_ASIC_ETHTYPE_ADD(uint32 swIdx, uint16 ethtype);
rtk_fc_ret_t RTK_RG_ASIC_ETHTYPE_DEL(uint32 swIdx);
rtk_fc_ret_t RTK_RG_ASIC_ETHTYPE_GET_HW_IDX(uint16 ethtype, uint32 *hwIdx);

rtk_fc_ret_t RTK_L2_PORTLIMITLEARNINGCNT_SET(rtk_port_t port, uint32 macCnt);
rtk_fc_ret_t RTK_L2_PORTLIMITLEARNINGCNTACTION_SET(rtk_port_t port, rtk_l2_limitLearnCntAction_t learningAction);
rtk_fc_ret_t RTK_L2_PORTLOOKUPMISSACTION_SET(rtk_port_t port, rtk_l2_lookupMissType_t type, rtk_action_t action);
rtk_fc_ret_t RTK_L2_ILLEGALPORTMOVEACTION_SET(rtk_port_t port, rtk_action_t fwdAction);
rtk_fc_ret_t RTK_L2_PORTAGINGENABLE_SET(rtk_port_t port, rtk_enable_t enable);
rtk_fc_ret_t RTK_L2_VIDUNMATCHACTION_SET(rtk_port_t port, rtk_action_t fwdAction);
rtk_fc_ret_t RTK_VLAN_PORTPVID_SET(rtk_port_t port,uint32 pvid);
rtk_fc_ret_t RTK_QOS_PORTPRIMAP_SET(rtk_port_t port, uint32 group);
rtk_fc_ret_t RTK_QOS_PORTPRISELGROUP_SET(rtk_port_t port, uint32 priSelGrpIdx);
rtk_fc_ret_t RTK_QOS_PORTPRI_SET(rtk_port_t port, rtk_pri_t intPri);
rtk_fc_ret_t RTK_SVLAN_SERVICEPORT_SET(rtk_port_t port, rtk_enable_t enable);
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
rtk_fc_ret_t RTK_ACL_IGRRULEENTRY_ADD(rtk_acl_ingress_entry_t *pAclRule, int ref_info);
rtk_fc_ret_t RTK_ACL_IGRSTATE_SET(rtk_port_t port, rtk_enable_t state);
rtk_fc_ret_t RTK_ACL_IGRUNMATCHACTION_SET(rtk_port_t port, rtk_filter_unmatch_action_type_t action);
#endif
rtk_fc_ret_t RTK_PORT_MACFORCEABILITY_GET(rtk_port_t port, rtk_port_macAbility_t * pMacAbility);
rtk_fc_ret_t RTK_PORT_MACFORCEABILITY_SET(rtk_port_t port,rtk_port_macAbility_t macAbility);
rtk_fc_ret_t RTK_PORT_MACFORCEABILITYSTATE_SET(rtk_port_t port,rtk_enable_t state);
#endif//__RTK_FC_MAPPINGAPI__
