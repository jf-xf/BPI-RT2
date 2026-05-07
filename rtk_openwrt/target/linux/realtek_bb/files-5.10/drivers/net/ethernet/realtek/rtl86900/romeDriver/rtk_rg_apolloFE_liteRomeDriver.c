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

#include <rtk_rg_apolloFE_liteRomeDriver.h>

struct platform pf=
{
	.rtk_rg_api_module_init= rtk_rg_apollo_api_module_init,
	.rtk_rg_driverVersion_get= rtk_rg_apollo_driverVersion_get,
	.rtk_rg_initParam_get	=rtk_rg_apollo_initParam_get,
	.rtk_rg_initParam_set	=rtk_rg_apollo_initParam_set,
	.rtk_rg_lanInterface_add	=rtk_rg_apollo_lanInterface_add,
//5
	.rtk_rg_wanInterface_add	=rtk_rg_apollo_wanInterface_add,
	.rtk_rg_staticInfo_set	=rtk_rg_apollo_staticInfo_set,
	.rtk_rg_dhcpRequest_set	=rtk_rg_apollo_dhcpRequest_set,
	.rtk_rg_dhcpClientInfo_set	=rtk_rg_apollo_dhcpClientInfo_set,
	.rtk_rg_pppoeClientInfoBeforeDial_set	=rtk_rg_apollo_pppoeClientInfoBeforeDial_set,
//10
	.rtk_rg_pppoeClientInfoAfterDial_set	=rtk_rg_apollo_pppoeClientInfoAfterDial_set,
	.rtk_rg_interface_del	=rtk_rg_apollo_interface_del,
	.rtk_rg_intfInfo_find	=rtk_rg_apollo_intfInfo_find,
	.rtk_rg_cvlan_add	=rtk_rg_apollo_cvlan_add,
	.rtk_rg_cvlan_del	=rtk_rg_apollo_cvlan_del,
//15
	.rtk_rg_cvlan_get=rtk_rg_apollo_cvlan_get,
	.rtk_rg_vlanBinding_add	=rtk_rg_apollo_vlanBinding_add,
	.rtk_rg_vlanBinding_del	=rtk_rg_apollo_vlanBinding_del,
	.rtk_rg_vlanBinding_find	=rtk_rg_apollo_vlanBinding_find,
	.rtk_rg_algServerInLanAppsIpAddr_add	=rtk_rg_apollo_algServerInLanAppsIpAddr_add,
//20
	.rtk_rg_algServerInLanAppsIpAddr_del	=rtk_rg_apollo_algServerInLanAppsIpAddr_del,
	.rtk_rg_algApps_set	=rtk_rg_apollo_algApps_set,
	.rtk_rg_algApps_get	=rtk_rg_apollo_algApps_get,
	.rtk_rg_dmzHost_set	=rtk_rg_apollo_dmzHost_set,
	.rtk_rg_dmzHost_get	=rtk_rg_apollo_dmzHost_get,
//25
	.rtk_rg_virtualServer_add	=rtk_rg_apollo_virtualServer_add,
	.rtk_rg_virtualServer_del	=rtk_rg_apollo_virtualServer_del,
	.rtk_rg_virtualServer_find	=rtk_rg_apollo_virtualServer_find,
	.rtk_rg_aclFilterAndQos_add	=rtk_rg_apollo_aclFilterAndQos_add,
	.rtk_rg_aclFilterAndQos_del	=rtk_rg_apollo_aclFilterAndQos_del,
//30
	.rtk_rg_aclFilterAndQos_find	=rtk_rg_apollo_aclFilterAndQos_find,
	.rtk_rg_macFilter_add	=rtk_rg_apollo_macFilter_add,
	.rtk_rg_macFilter_del	=rtk_rg_apollo_macFilter_del,
	.rtk_rg_macFilter_find	=rtk_rg_apollo_macFilter_find,
	.rtk_rg_mac_filter_whitelist_add =rtk_rg_apollo_macFilter_whitelist_add,
//35
	.rtk_rg_mac_filter_whitelist_del =rtk_rg_apollo_macFilter_whitelist_del,
	.rtk_rg_urlFilterString_add	=rtk_rg_apollo_urlFilterString_add,
	.rtk_rg_urlFilterString_del	=rtk_rg_apollo_urlFilterString_del,
	.rtk_rg_urlFilterString_find	=rtk_rg_apollo_urlFilterString_find,
	.rtk_rg_upnpConnection_add	=rtk_rg_apollo_upnpConnection_add,
//40
	.rtk_rg_upnpConnection_del	=rtk_rg_apollo_upnpConnection_del,
	.rtk_rg_upnpConnection_find	=rtk_rg_apollo_upnpConnection_find,
	.rtk_rg_naptConnection_add	=rtk_rg_apollo_naptConnection_add,
	.rtk_rg_naptConnection_del	=rtk_rg_apollo_naptConnection_del,
	.rtk_rg_naptConnection_find	=rtk_rg_apollo_naptConnection_find,
//45
	.rtk_rg_multicastFlow_add	=rtk_rg_apollo_multicastFlow_add,
	.rtk_rg_multicastFlow_del	=rtk_rg_apollo_multicastFlow_del,
	/* martin zhu add */
	.rtk_rg_l2MultiCastFlow_add =rtk_rg_apollo_l2MultiCastFlow_add,
	.rtk_rg_multicastFlow_find	=rtk_rg_apollo_multicastFlow_find,
	.rtk_rg_macEntry_add	=rtk_rg_apolloFE_macEntry_add,
//50
	.rtk_rg_macEntry_del	=rtk_rg_apollo_macEntry_del,
	.rtk_rg_macEntry_find	=rtk_rg_apollo_macEntry_find,
	.rtk_rg_arpEntry_add	=rtk_rg_apollo_arpEntry_add,
	.rtk_rg_arpEntry_del	=rtk_rg_apollo_arpEntry_del,
	.rtk_rg_arpEntry_find	=rtk_rg_apollo_arpEntry_find,
//55
	.rtk_rg_neighborEntry_add	=rtk_rg_apollo_neighborEntry_add,
	.rtk_rg_neighborEntry_del	=rtk_rg_apollo_neighborEntry_del,
	.rtk_rg_neighborEntry_find	=rtk_rg_apollo_neighborEntry_find,
	.rtk_rg_accessWanLimit_set	=rtk_rg_apollo_accessWanLimit_set,
	.rtk_rg_accessWanLimit_get	=rtk_rg_apollo_accessWanLimit_get,
//60
	.rtk_rg_accessWanLimitCategory_set	=rtk_rg_apollo_accessWanLimitCategory_set,
	.rtk_rg_accessWanLimitCategory_get	=rtk_rg_apollo_accessWanLimitCategory_get,
	.rtk_rg_softwareSourceAddrLearningLimit_set	=rtk_rg_apollo_softwareSourceAddrLearningLimit_set,
	.rtk_rg_softwareSourceAddrLearningLimit_get	=rtk_rg_apollo_softwareSourceAddrLearningLimit_get,
	.rtk_rg_dosPortMaskEnable_set	=rtk_rg_apollo_dosPortMaskEnable_set,
//65
	.rtk_rg_dosPortMaskEnable_get	=rtk_rg_apollo_dosPortMaskEnable_get,
	.rtk_rg_dosType_set	=rtk_rg_apollo_dosType_set,
	.rtk_rg_dosType_get	=rtk_rg_apollo_dosType_get,
	.rtk_rg_dosFloodType_set	=rtk_rg_apollo_dosFloodType_set,
	.rtk_rg_dosFloodType_get	=rtk_rg_apollo_dosFloodType_get,
//70
	.rtk_rg_portMirror_set	=rtk_rg_apollo_portMirror_set,
	.rtk_rg_portMirror_get	=rtk_rg_apollo_portMirror_get,
	.rtk_rg_portMirror_clear	=rtk_rg_apollo_portMirror_clear,
	.rtk_rg_portEgrBandwidthCtrlRate_set	=rtk_rg_apollo_portEgrBandwidthCtrlRate_set,
	.rtk_rg_portIgrBandwidthCtrlRate_set	=rtk_rg_apollo_portIgrBandwidthCtrlRate_set,
//75
	.rtk_rg_portEgrBandwidthCtrlRate_get	=rtk_rg_apollo_portEgrBandwidthCtrlRate_get,
	.rtk_rg_portIgrBandwidthCtrlRate_get	=rtk_rg_apollo_portIgrBandwidthCtrlRate_get,
	.rtk_rg_phyPortForceAbility_set	=rtk_rg_apollo_phyPortForceAbility_set,
	.rtk_rg_phyPortForceAbility_get	=rtk_rg_apollo_phyPortForceAbility_get,
	.rtk_rg_cpuPortForceTrafficCtrl_set	=rtk_rg_apollo_cpuPortForceTrafficCtrl_set,
//80
	.rtk_rg_cpuPortForceTrafficCtrl_get	=rtk_rg_apollo_cpuPortForceTrafficCtrl_get,
	.rtk_rg_portMibInfo_get	=rtk_rg_apollo_portMibInfo_get,
	.rtk_rg_portMibInfo_clear	=rtk_rg_apollo_portMibInfo_clear,
	.rtk_rg_stormControl_add	=rtk_rg_apollo_stormControl_add,
	.rtk_rg_stormControl_del	=rtk_rg_apollo_stormControl_del,
//85
	.rtk_rg_stormControl_find	=rtk_rg_apollo_stormControl_find,
	.rtk_rg_shareMeter_set	=rtk_rg_apollo_shareMeter_set,
	.rtk_rg_shareMeter_get	=rtk_rg_apollo_shareMeter_get,
	.rtk_rg_shareMeterMode_set =rtk_rg_apollo_shareMeterMode_set,
	.rtk_rg_shareMeterMode_get =rtk_rg_apollo_shareMeterMode_get,
//90
	.rtk_rg_qosStrictPriorityOrWeightFairQueue_set	=rtk_rg_apollo_qosStrictPriorityOrWeightFairQueue_set,
	.rtk_rg_qosStrictPriorityOrWeightFairQueue_get	=rtk_rg_apollo_qosStrictPriorityOrWeightFairQueue_get,
	.rtk_rg_qosInternalPriMapToQueueId_set	=rtk_rg_apollo_qosInternalPriMapToQueueId_set,
	.rtk_rg_qosInternalPriMapToQueueId_get	=rtk_rg_apollo_qosInternalPriMapToQueueId_get,
	.rtk_rg_qosInternalPriDecisionByWeight_set	=rtk_rg_apollo_qosInternalPriDecisionByWeight_set,
//95
	.rtk_rg_qosInternalPriDecisionByWeight_get	=rtk_rg_apollo_qosInternalPriDecisionByWeight_get,
	.rtk_rg_qosDscpRemapToInternalPri_set	=rtk_rg_apollo_qosDscpRemapToInternalPri_set,
	.rtk_rg_qosDscpRemapToInternalPri_get	=rtk_rg_apollo_qosDscpRemapToInternalPri_get,
	.rtk_rg_qosPortBasedPriority_set	=rtk_rg_apollo_qosPortBasedPriority_set,
	.rtk_rg_qosPortBasedPriority_get	=rtk_rg_apollo_qosPortBasedPriority_get,
//100
	.rtk_rg_qosDot1pPriRemapToInternalPri_set	=rtk_rg_apollo_qosDot1pPriRemapToInternalPri_set,
	.rtk_rg_qosDot1pPriRemapToInternalPri_get	=rtk_rg_apollo_qosDot1pPriRemapToInternalPri_get,
	.rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_set	=rtk_rg_apollo_qosDscpRemarkEgressPortEnableAndSrcSelect_set,
	.rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_get	=rtk_rg_apollo_qosDscpRemarkEgressPortEnableAndSrcSelect_get,
	.rtk_rg_qosDscpRemarkByInternalPri_set	=rtk_rg_apollo_qosDscpRemarkByInternalPri_set,
//105
	.rtk_rg_qosDscpRemarkByInternalPri_get	=rtk_rg_apollo_qosDscpRemarkByInternalPri_get,
	.rtk_rg_qosDscpRemarkByDscp_set	=rtk_rg_apollo_qosDscpRemarkByDscp_set,
	.rtk_rg_qosDscpRemarkByDscp_get	=rtk_rg_apollo_qosDscpRemarkByDscp_get,
	.rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_set	=rtk_rg_apollo_qosDot1pPriRemarkByInternalPriEgressPortEnable_set,
	.rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_get	=rtk_rg_apollo_qosDot1pPriRemarkByInternalPriEgressPortEnable_get,
//110
	.rtk_rg_qosDot1pPriRemarkByInternalPri_set	=rtk_rg_apollo_qosDot1pPriRemarkByInternalPri_set,
	.rtk_rg_qosDot1pPriRemarkByInternalPri_get	=rtk_rg_apollo_qosDot1pPriRemarkByInternalPri_get,
	.rtk_rg_portBasedCVlanId_set	=rtk_rg_apollo_portBasedCVlanId_set,
	.rtk_rg_portBasedCVlanId_get	=rtk_rg_apollo_portBasedCVlanId_get,
	.rtk_rg_portStatus_get	=rtk_rg_apollo_portStatus_get,
//115
#ifdef CONFIG_RG_NAPT_PORT_COLLISION_PREVENTION
	.rtk_rg_naptExtPortGet	=rtk_rg_apollo_naptExtPortGet,
	.rtk_rg_naptExtPortFree	=rtk_rg_apollo_naptExtPortFree,
#endif
	.rtk_rg_classifyEntry_add	=rtk_rg_apollo_classifyEntry_add,
	.rtk_rg_classifyEntry_find	=rtk_rg_apollo_classifyEntry_find,
	.rtk_rg_classifyEntry_del	=rtk_rg_apollo_classifyEntry_del,
//120
	.rtk_rg_svlanTpid_get= rtk_rg_apollo_svlanTpid_get,
	.rtk_rg_svlanTpid_set= rtk_rg_apollo_svlanTpid_set,
	.rtk_rg_svlanServicePort_set=rtk_rg_apollo_svlanServicePort_set,
	.rtk_rg_svlanServicePort_get=rtk_rg_apollo_svlanServicePort_get,
	.rtk_rg_pppoeInterfaceIdleTime_get=rtk_rg_apollo_pppoeInterfaceIdleTime_get,
//125
	.rtk_rg_gatewayServicePortRegister_add=rtk_rg_apollo_gatewayServicePortRegister_add,
	.rtk_rg_gatewayServicePortRegister_del=rtk_rg_apollo_gatewayServicePortRegister_del,
	.rtk_rg_gatewayServicePortRegister_find=rtk_rg_apollo_gatewayServicePortRegister_find,
	.rtk_rg_wlanDevBasedCVlanId_set=rtk_rg_apollo_wlanDevBasedCVlanId_set,
	.rtk_rg_wlanDevBasedCVlanId_get=rtk_rg_apollo_wlanDevBasedCVlanId_get,
//130
	.rtk_rg_wlanSoftwareSourceAddrLearningLimit_set=rtk_rg_apollo_wlanSoftwareSourceAddrLearningLimit_set,
	.rtk_rg_wlanSoftwareSourceAddrLearningLimit_get=rtk_rg_apollo_wlanSoftwareSourceAddrLearningLimit_get,
	.rtk_rg_naptFilterAndQos_add=rtk_rg_apollo_naptFilterAndQos_add,
	.rtk_rg_naptFilterAndQos_del=rtk_rg_apollo_naptFilterAndQos_del,
	.rtk_rg_naptFilterAndQos_find=rtk_rg_apollo_naptFilterAndQos_find,
//135
	.rtk_rg_pptpClientInfoBeforeDial_set=rtk_rg_apollo_pptpClientInfoBeforeDial_set,
	.rtk_rg_pptpClientInfoAfterDial_set=rtk_rg_apollo_pptpClientInfoAfterDial_set,
	.rtk_rg_l2tpClientInfoBeforeDial_set=rtk_rg_apollo_l2tpClientInfoBeforeDial_set,
	.rtk_rg_l2tpClientInfoAfterDial_set=rtk_rg_apollo_l2tpClientInfoAfterDial_set,
	.rtk_rg_stpBlockingPortmask_set=rtk_rg_apollo_stpBlockingPortmask_set,
//140
	.rtk_rg_stpBlockingPortmask_get=rtk_rg_apollo_stpBlockingPortmask_get,
	.rtk_rg_portIsolation_set=rtk_rg_apollo_portIsolation_set,
	.rtk_rg_portIsolation_get=rtk_rg_apollo_portIsolation_get,
	.rtk_rg_dsliteInfo_set=rtk_rg_apollo_dsliteInfo_set,
	.rtk_rg_pppoeDsliteInfoBeforeDial_set=rtk_rg_apollo_pppoeDsliteInfoBeforeDial_set,
//145
	.rtk_rg_pppoeDsliteInfoAfterDial_set=rtk_rg_apollo_pppoeDsliteInfoAfterDial_set,
	.rtk_rg_gponDsBcFilterAndRemarking_add=rtk_rg_apollo_gponDsBcFilterAndRemarking_add,
	.rtk_rg_gponDsBcFilterAndRemarking_del=rtk_rg_apollo_gponDsBcFilterAndRemarking_del,
	.rtk_rg_gponDsBcFilterAndRemarking_find=rtk_rg_apollo_gponDsBcFilterAndRemarking_find,
	.rtk_rg_gponDsBcFilterAndRemarking_del_all=rtk_rg_apollo_gponDsBcFilterAndRemarking_del_all,
//150
	.rtk_rg_gponDsBcFilterAndRemarking_Enable=rtk_rg_apollo_gponDsBcFilterAndRemarking_Enable,
	.rtk_rg_interfaceMibCounter_del=rtk_rg_apolloFE_interfaceMibCounter_del,
	.rtk_rg_interfaceMibCounter_get=rtk_rg_apolloFE_interfaceMibCounter_get,
	.rtk_rg_redirectHttpAll_set=rtk_rg_apollo_redirectHttpAll_set,
	.rtk_rg_redirectHttpAll_get=rtk_rg_apollo_redirectHttpAll_get,
//155
	.rtk_rg_redirectHttpURL_add=rtk_rg_apollo_redirectHttpURL_add,
	.rtk_rg_redirectHttpURL_del=rtk_rg_apollo_redirectHttpURL_del,
	.rtk_rg_redirectHttpWhiteList_add=rtk_rg_apollo_redirectHttpWhiteList_add,
	.rtk_rg_redirectHttpWhiteList_del=rtk_rg_apollo_redirectHttpWhiteList_del,
	.rtk_rg_redirectHttpRsp_set=rtk_rg_apollo_redirectHttpRsp_set,
//160
	.rtk_rg_redirectHttpRsp_get=rtk_rg_apollo_redirectHttpRsp_get,
	.rtk_rg_svlanTpid2_get= rtk_rg_apolloFE_svlanTpid2_get,
	.rtk_rg_svlanTpid2_set= rtk_rg_apolloFE_svlanTpid2_set,
	.rtk_rg_svlanTpid2_enable_get=rtk_rg_apolloFE_svlanTpid2_enable_get,
	.rtk_rg_svlanTpid2_enable_set=rtk_rg_apolloFE_svlanTpid2_enable_set,
//165
	.rtk_rg_hostPoliceControl_set=rtk_rg_apolloFE_hostPoliceControl_set,
	.rtk_rg_hostPoliceControl_get=rtk_rg_apolloFE_hostPoliceControl_get,
	.rtk_rg_hostPoliceLogging_get=rtk_rg_apolloFE_hostPoliceLogging_get,
	.rtk_rg_hostPoliceLogging_del=rtk_rg_apolloFE_hostPoliceLogging_del,
	.rtk_rg_redirectHttpCount_set=rtk_rg_apollo_redirectHttpCount_set,
//170
	.rtk_rg_redirectHttpCount_get=rtk_rg_apollo_redirectHttpCount_get,
	.rtk_rg_staticRoute_add=rtk_rg_apollo_staticRoute_add,
	.rtk_rg_staticRoute_del=rtk_rg_apollo_staticRoute_del,
	.rtk_rg_staticRoute_find=rtk_rg_apollo_staticRoute_find,
	.rtk_rg_aclLogCounterControl_get=rtk_rg_apollo_aclLogCounterControl_get,
//175
	.rtk_rg_aclLogCounterControl_set=rtk_rg_apollo_aclLogCounterControl_set,
	.rtk_rg_aclLogCounter_get=rtk_rg_apollo_aclLogCounter_get,
	.rtk_rg_aclLogCounter_reset=rtk_rg_apollo_aclLogCounter_reset,
	.rtk_rg_groupMacLimit_get=rtk_rg_apollo_groupMacLimit_get,
	.rtk_rg_groupMacLimit_set=rtk_rg_apollo_groupMacLimit_set,
//180
	.rtk_rg_igmpMldSnoopingControl_set=rtk_rg_apollo_igmpMldSnoopingControl_set,
	.rtk_rg_igmpMldSnoopingControl_get=rtk_rg_apollo_igmpMldSnoopingControl_get,
	.rtk_rg_flowMibCounter_get=NULL,
	.rtk_rg_flowMibCounter_reset=NULL,
	.rtk_rg_softwareIdleTime_set=rtk_rg_apollo_softwareIdleTime_set,
//185
	.rtk_rg_softwareIdleTime_get=rtk_rg_apollo_softwareIdleTime_get,
	.rtk_rg_funcbasedMeter_set=NULL,
	.rtk_rg_funcbasedMeter_get=NULL,
	.rtk_rg_flowHiPriEntry_add=NULL,
	.rtk_rg_flowHiPriEntry_del=NULL,
//190
	.rtk_rg_igmpMldSnoopingPortControl_add=rtk_rg_apollo_igmpMldSnoopingPortControl_add,
	.rtk_rg_igmpMldSnoopingPortControl_del=rtk_rg_apollo_igmpMldSnoopingPortControl_del,
	.rtk_rg_igmpMldSnoopingPortControl_find=rtk_rg_apollo_igmpMldSnoopingPortControl_find,
	.rtk_rg_callback_function_ptr_get=rtk_rg_apollo_callback_function_ptr_get,
	.rtk_rg_vlanGroupMacLimit_add=rtk_rg_apollo_vlanGroupMacLimit_add,
//195
	.rtk_rg_vlanGroupMacLimit_set=rtk_rg_apollo_vlanGroupMacLimit_set,
	.rtk_rg_vlanGroupMacLimit_del=rtk_rg_apollo_vlanGroupMacLimit_del,
	.rtk_rg_vlanGroupMacLimit_get=rtk_rg_apollo_vlanGroupMacLimit_get,
	.rtk_rg_vlanGroupMacLimit_find=rtk_rg_apollo_vlanGroupMacLimit_find,
	.rtk_rg_dosFloodThresholdUnit_set=rtk_rg_apollo_dosFloodThresholdUnit_set,
//200
	.rtk_rg_dosFloodThresholdUnit_get=rtk_rg_apollo_dosFloodThresholdUnit_get,
	.rtk_rg_accessDot1xControl_set=rtk_rg_apollo_accessDot1xControl_set,
	.rtk_rg_accessDot1xFilter_set=rtk_rg_apollo_accessDot1xFilter_set,
	.rtk_rg_urlflowPri_add=NULL,
	.rtk_rg_urlflowPri_del=NULL,		
//205
	.rtk_rg_portTrigger_add=rtk_rg_apollo_portTrigger_add,
	.rtk_rg_portTrigger_del=rtk_rg_apollo_portTrigger_del,
	.rtk_rg_vxlanClientInfo_set=rtk_rg_apollo_vxlanClientInfo_set,
	.rtk_rg_ipset_add=rtk_rg_apollo_ipset_add,
	.rtk_rg_ipset_del=rtk_rg_apollo_ipset_del,
//210
	.rtk_rg_ipsets_group_set=rtk_rg_apollo_ipsets_group_set,
	.rtk_rg_ipsets_set_add=rtk_rg_apollo_ipsets_set_add,
	.rtk_rg_ipsets_set_del=rtk_rg_apollo_ipsets_set_del,
	.rtk_rg_ipsets_rule_add=rtk_rg_apollo_ipsets_rule_add,
	.rtk_rg_ipsets_rule_del=rtk_rg_apollo_ipsets_rule_del,
//215
};




rtk_rg_err_code_t rtk_rg_apolloFE_interfaceMibCounter_del(int intf_idx)
{
	//reset software and hardward counter
	if(intf_idx<0 || intf_idx>=MAX_NETIF_SW_TABLE_SIZE)
        RETURN_ERR(RT_ERR_RG_INVALID_PARAM);

#if defined(CONFIG_RG_RTL9602C_SERIES) //patch for mismatching mib ipv6 netif problem
	if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
	{
		if(rg_db.systemGlobal.interfaceInfo[intf_idx].storedInfo.is_wan==1
			&& rg_db.systemGlobal.interfaceInfo[intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type!=RTK_RG_BRIDGE)
		{
			if(rg_db.systemGlobal.interfaceInfo[intf_idx].p_wanStaticInfo->ip_version==IPVER_V4V6
				&& rg_db.systemGlobal.interfaceInfo[intf_idx].p_wanStaticInfo->napt_enable==1
				&& intf_idx>=(MAX_NETIF_HW_TABLE_SIZE/2) )
			{
				rtlglue_printf("Can not delete mib counter of ipv6 wan netif[%d].\n", intf_idx);
				RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
			}
		}
	}
#endif

	if(intf_idx>=MAX_NETIF_HW_TABLE_SIZE)
	{
		bzero(&rg_db.netif[intf_idx].netifMib,sizeof(rtk_rg_netifMib_entry_t));
		return (RT_ERR_RG_OK);
	}
	else
	{
		if(rtk_l34_mib_reset(intf_idx)==RT_ERR_OK){
			bzero(&rg_db.netif[intf_idx].netifMib,sizeof(rtk_rg_netifMib_entry_t));

#if defined(CONFIG_RG_RTL9602C_SERIES) //patch for mismatching mib ipv6 netif problem
		if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
		{
			if(rg_db.systemGlobal.interfaceInfo[intf_idx].storedInfo.is_wan==1
				&& rg_db.systemGlobal.interfaceInfo[intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type!=RTK_RG_BRIDGE)
			{
				if(rg_db.systemGlobal.interfaceInfo[intf_idx].p_wanStaticInfo->ip_version==IPVER_V4V6
					&& rg_db.systemGlobal.interfaceInfo[intf_idx].p_wanStaticInfo->napt_enable==1)
				{
					int v6IntfIdx = intf_idx+(MAX_NETIF_HW_TABLE_SIZE/2);

					if(rtk_l34_mib_reset(v6IntfIdx)!=RT_ERR_OK) RETURN_ERR(RT_ERR_RG_FAILED);
					bzero(&rg_db.netif[v6IntfIdx].netifMib,sizeof(rtk_rg_netifMib_entry_t));
				}
			}
		}
#endif

			return (RT_ERR_RG_OK);
		}
	}
	RETURN_ERR(RT_ERR_RG_FAILED);
}

rtk_rg_err_code_t rtk_rg_apolloFE_interfaceMibCounter_get(rtk_rg_netifMib_entry_t *pNetifMib)
{
	rtk_l34_mib_t l34Cnt;
	uint32 netifIdx;

	//get hardware counter and plus software counter if has.
	if(pNetifMib==NULL)RETURN_ERR(RT_ERR_RG_NULL_POINTER);
	if(pNetifMib->netifIdx>=MAX_NETIF_SW_TABLE_SIZE)RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);

	netifIdx = pNetifMib->netifIdx;
	memset(pNetifMib, 0, sizeof(rtk_rg_netifMib_entry_t));
	pNetifMib->netifIdx = netifIdx;
	if(netifIdx>=MAX_NETIF_HW_TABLE_SIZE)
	{
		pNetifMib->in_intf_uc_byte_cnt=rg_db.netif[netifIdx].netifMib.in_intf_uc_byte_cnt;
		pNetifMib->in_intf_uc_packet_cnt=rg_db.netif[netifIdx].netifMib.in_intf_uc_packet_cnt;
		pNetifMib->out_intf_uc_byte_cnt=rg_db.netif[netifIdx].netifMib.out_intf_uc_byte_cnt;
		pNetifMib->out_intf_uc_packet_cnt=rg_db.netif[netifIdx].netifMib.out_intf_uc_packet_cnt;
		return (RT_ERR_RG_OK);
	}
	else
	{
		l34Cnt.ifIndex=netifIdx;
		if(rtk_l34_mib_get(&l34Cnt)==RT_ERR_OK){
			pNetifMib->in_intf_uc_byte_cnt=l34Cnt.ifInOctets+rg_db.netif[netifIdx].netifMib.in_intf_uc_byte_cnt;
			pNetifMib->in_intf_uc_packet_cnt=l34Cnt.ifInUcstPkts+rg_db.netif[netifIdx].netifMib.in_intf_uc_packet_cnt;
			pNetifMib->out_intf_uc_byte_cnt=l34Cnt.ifOutOctets+rg_db.netif[netifIdx].netifMib.out_intf_uc_byte_cnt;
			pNetifMib->out_intf_uc_packet_cnt=l34Cnt.ifOutUcstPkts+rg_db.netif[netifIdx].netifMib.out_intf_uc_packet_cnt;

#if defined(CONFIG_RG_RTL9602C_SERIES) //patch for mismatching mib ipv6 netif problem
		if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
		{
			if(rg_db.systemGlobal.interfaceInfo[netifIdx].storedInfo.is_wan==1
				&& rg_db.systemGlobal.interfaceInfo[netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type!=RTK_RG_BRIDGE)
			{
				if(rg_db.systemGlobal.interfaceInfo[netifIdx].p_wanStaticInfo->ip_version==IPVER_V4V6
					&& rg_db.systemGlobal.interfaceInfo[netifIdx].p_wanStaticInfo->napt_enable==1
					&& netifIdx<(MAX_NETIF_HW_TABLE_SIZE/2) )
				{
					l34Cnt.ifIndex=netifIdx+(MAX_NETIF_HW_TABLE_SIZE/2);
					if(rtk_l34_mib_get(&l34Cnt)!=RT_ERR_OK) RETURN_ERR(RT_ERR_RG_FAILED);
					pNetifMib->in_intf_uc_byte_cnt += l34Cnt.ifInOctets;
					pNetifMib->in_intf_uc_packet_cnt += l34Cnt.ifInUcstPkts;
					pNetifMib->out_intf_uc_byte_cnt += l34Cnt.ifOutOctets;
					pNetifMib->out_intf_uc_packet_cnt += l34Cnt.ifOutUcstPkts;
				}
			}
		}
#endif

			return (RT_ERR_RG_OK);
		}
	}
	RETURN_ERR(RT_ERR_RG_FAILED);
}


rtk_rg_err_code_t rtk_rg_apolloFE_svlanTpid2_enable_set(rtk_rg_enable_t enable)
{
	if(enable<0 || enable>=RTK_RG_ENABLE_END)
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	assert_ok(RTK_SVLAN_TPIDENABLE_SET(1, enable));
	return (RT_ERR_RG_OK);
}
rtk_rg_err_code_t rtk_rg_apolloFE_svlanTpid2_enable_get(rtk_rg_enable_t *pEnable)
{
	if(pEnable==NULL)
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	assert_ok(rtk_svlan_tpidEnable_get(1, (rtk_enable_t *)pEnable));
	if(*pEnable!=rg_db.systemGlobal.tpid2_en)WARNING("Svlan TPID2_enable is different between software and hardware. Please check if RG APIs is mixed with RTK APIs!");
	return (RT_ERR_RG_OK);
}
rtk_rg_err_code_t rtk_rg_apolloFE_svlanTpid2_set(uint32 svlan_tag_id)
{
	if(svlan_tag_id>0xffff)
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	assert_ok(RTK_SVLAN_TPIDENTRY_SET(1, svlan_tag_id));
	return (RT_ERR_RG_OK);
}
rtk_rg_err_code_t rtk_rg_apolloFE_svlanTpid2_get(uint32 *pSvlanTagId)
{
	if(pSvlanTagId==NULL)
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	assert_ok(rtk_svlan_tpidEntry_get(1, pSvlanTagId));
	if(*pSvlanTagId!=rg_db.systemGlobal.tpid2)WARNING("Svlan TPID2 is different between software and hardware. Please check if RG APIs is mixed with RTK APIs!");
	return (RT_ERR_RG_OK);
}

rtk_rg_err_code_t rtk_rg_apolloFE_hostPoliceControl_set(rtk_rg_hostPoliceControl_t *pHostPoliceControl, int host_idx)
{
	int lut_idx=-1;
	rtk_rg_macEntry_t macEntry;
	rtk_rg_hostPoliceLinkList_t *pHostEntry,*pNextEntry;

	if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);

	//Check parameter
	if(pHostPoliceControl==NULL)RETURN_ERR(RT_ERR_RG_NULL_POINTER);
	if(host_idx<0 || host_idx>=HOST_POLICING_TABLE_SIZE)RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);

	ASSERT_EQ(rtk_rate_hostMacAddr_set(host_idx,&pHostPoliceControl->macAddr),RT_ERR_OK);
	ASSERT_EQ(rtk_rate_hostIgrBwCtrlState_set(host_idx,pHostPoliceControl->ingressLimitCtrl),RT_ERR_OK);
	ASSERT_EQ(rtk_rate_hostEgrBwCtrlState_set(host_idx,pHostPoliceControl->egressLimitCtrl),RT_ERR_OK);
	ASSERT_EQ(rtk_stat_hostState_set(host_idx,pHostPoliceControl->mibCountCtrl),RT_ERR_OK);
	ASSERT_EQ(rtk_rate_hostBwCtrlMeterIdx_set(host_idx,pHostPoliceControl->limitMeterIdx),RT_ERR_OK);	//meaningful only when igr or egr ctrl enabled


	memcpy(&rg_db.hostPoliceList[host_idx].info,pHostPoliceControl,sizeof(rtk_rg_hostPoliceControl_t));
	//Check if we are in count list
	if(!list_empty(&rg_db.hostPoliceCountListHead)){
		list_for_each_entry_safe(pHostEntry,pNextEntry,&rg_db.hostPoliceCountListHead,host_list){
			if(pHostEntry->idx==host_idx){
				if(!pHostPoliceControl->mibCountCtrl || memcmp(pHostPoliceControl->macAddr.octet,pHostEntry->info.macAddr.octet,ETHER_ADDR_LEN)){
					list_del_init(&pHostEntry->host_list);
					if(pHostEntry->lut_idx_learned){
						rg_db.lut[pHostEntry->lut_idx].host_idx_valid=0;
						pHostEntry->lut_idx_learned=0;
					}
					break;
				}else	//mibCountCtrl is enable and already in count_list, just return.
					return (RT_ERR_RG_OK);
			}
		}
	}
	if(pHostPoliceControl->mibCountCtrl){
		//add to count list
		memcpy(macEntry.mac.octet,pHostPoliceControl->macAddr.octet,ETHER_ADDR_LEN);
		if(rtk_rg_apollo_macEntry_find(&macEntry,&lut_idx)==RT_ERR_RG_OK){
			rg_db.hostPoliceList[host_idx].lut_idx=lut_idx;
			rg_db.hostPoliceList[host_idx].lut_idx_learned=1;
			rg_db.lut[rg_db.hostPoliceList[host_idx].lut_idx].host_idx=host_idx;
			rg_db.lut[rg_db.hostPoliceList[host_idx].lut_idx].host_idx_valid=1;
		}
		list_add_tail(&rg_db.hostPoliceList[host_idx].host_list,&rg_db.hostPoliceCountListHead);
	}

	return (RT_ERR_RG_OK);
}

rtk_rg_err_code_t rtk_rg_apolloFE_hostPoliceControl_get(rtk_rg_hostPoliceControl_t *pHostPoliceControl, int host_idx)
{
	//Check parameter
	if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	if(pHostPoliceControl==NULL)RETURN_ERR(RT_ERR_RG_NULL_POINTER);
	if(host_idx<0 || host_idx>=HOST_POLICING_TABLE_SIZE)RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);

	memcpy(pHostPoliceControl,&rg_db.hostPoliceList[host_idx].info,sizeof(rtk_rg_hostPoliceControl_t));

	return (RT_ERR_RG_OK);
}

rtk_rg_err_code_t rtk_rg_apolloFE_hostPoliceLogging_get(rtk_rg_hostPoliceLogging_t *pHostMibCnt, int host_idx)
{
	//Check parameter
	if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	if(pHostMibCnt==NULL)RETURN_ERR(RT_ERR_RG_NULL_POINTER);
	if(host_idx<0 || host_idx>=HOST_POLICING_TABLE_SIZE)RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);

	ASSERT_EQ(rtk_stat_hostCnt_get(host_idx,STAT_HOST_RX_OCTETS,&pHostMibCnt->rx_count),RT_ERR_OK);
	ASSERT_EQ(rtk_stat_hostCnt_get(host_idx,STAT_HOST_TX_OCTETS,&pHostMibCnt->tx_count),RT_ERR_OK);
	pHostMibCnt->rx_count+=rg_db.hostPoliceList[host_idx].count.rx_count;
	pHostMibCnt->tx_count+=rg_db.hostPoliceList[host_idx].count.tx_count;

	return (RT_ERR_RG_OK);
}

rtk_rg_err_code_t rtk_rg_apolloFE_hostPoliceLogging_del(int host_idx)
{
	//Check parameter
	if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	if(host_idx<0 || host_idx>=HOST_POLICING_TABLE_SIZE)RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);

	ASSERT_EQ(rtk_stat_hostCnt_reset(host_idx),RT_ERR_OK);
	bzero(&rg_db.hostPoliceList[host_idx].count,sizeof(rtk_rg_hostPoliceLogging_t));

	return (RT_ERR_RG_OK);
}

rtk_rg_err_code_t rtk_rg_apolloFE_macEntry_add(rtk_rg_macEntry_t *macEntry, int *entry_idx)
{
	int ret;

	ret=rtk_rg_apollo_macEntry_add(macEntry,entry_idx);

	if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT1))
	{
		if(ret==RT_ERR_RG_OK){
			rtk_rg_hostPoliceLinkList_t *pHostEntry;
			//20160202LUKE: check host policing link list
			if(!list_empty(&rg_db.hostPoliceCountListHead)){
				list_for_each_entry(pHostEntry,&rg_db.hostPoliceCountListHead,host_list){
					if(!memcmp(macEntry->mac.octet,pHostEntry->info.macAddr.octet,ETHER_ADDR_LEN)){
						pHostEntry->lut_idx=*entry_idx;
						pHostEntry->lut_idx_learned=1;
						rg_db.lut[*entry_idx].host_idx=pHostEntry->idx;
						rg_db.lut[*entry_idx].host_idx_valid=1;
						break;
					}
				}
			}
		}
	}

	return ret;
}


//#####################################internal function declaration###########

int _rtk_rg_hardwareArpTableLookUp(unsigned short routingIdx, ipaddr_t ipAddr, rtk_rg_arp_linkList_t **phardwareArpEntry, int resetIdleTime)
{
	rtk_rg_arp_linkList_t *pArpEntry;

	if(list_empty(&rg_db.hardwareArpTableHead[ipAddr&0xff]))
		goto NOT_FOUND;

	list_for_each_entry(pArpEntry,&rg_db.hardwareArpTableHead[ipAddr&0xff],arp_list)
	{
		if(rg_db.arp[pArpEntry->idx].routingIdx==routingIdx && rg_db.arp[pArpEntry->idx].ipv4Addr==ipAddr)
		{
			TRACE("Found! HW ARP[%d] is match with %x",pArpEntry->idx,ipAddr);
			//Reset idle time
			if(resetIdleTime)
			{
				rg_db.arp[pArpEntry->idx].idleSecs=0;
				rg_db.arp[pArpEntry->idx].sendReqCount=0;
			}
			*phardwareArpEntry=pArpEntry;
			return (RT_ERR_RG_OK);
		}
	}

NOT_FOUND:
	//not found
	*phardwareArpEntry=NULL;
	return (RT_ERR_RG_OK);
}

int _rtk_rg_hardwareArpTableAdd(unsigned short routingIdx, ipaddr_t ipv4Addr, int l2Idx, int staticEntry, uint16 *hwArpIdx)
{
	rtk_rg_arp_linkList_t *pNewArpEntry;
	rtk_l34_arp_entry_t asic_arp_entry;

	//Check if we have not-used free arp list
	if(list_empty(&rg_db.hardwareArpFreeListHead))
	{
		DEBUG("all free HW ARP list are allocated...");
		return (RT_ERR_RG_ARP_FULL);
	}

	asic_arp_entry.valid=1;
	asic_arp_entry.ipAddr=ipv4Addr;
	asic_arp_entry.nhIdx=l2Idx;

	//Get one from free list
	pNewArpEntry=list_first_entry(&rg_db.hardwareArpFreeListHead, rtk_rg_arp_linkList_t, arp_list);
	asic_arp_entry.index=pNewArpEntry->idx;
	if(RTK_L34_ARPTABLE_SET(pNewArpEntry->idx,&asic_arp_entry) != RT_ERR_OK)
		RETURN_ERR(RT_ERR_RG_FAILED);
	list_move_tail(&pNewArpEntry->arp_list, &rg_db.hardwareArpTableHead[ipv4Addr&0xff]);
	//DEBUG("the free HW ARP %p idx is %d, routing=%d",pNewArpEntry,pNewArpEntry->idx,pNewArpEntry->routingIdx);

	//Setup ARP information
	//rg_db.arp[pNewArpEntry->idx].rtk_arp.nhIdx=l2Idx;
	//rg_db.arp[pNewArpEntry->idx].rtk_arp.valid=1;
	rg_db.arp[pNewArpEntry->idx].ipv4Addr=ipv4Addr;
	rg_db.arp[pNewArpEntry->idx].staticEntry=staticEntry;
	rg_db.arp[pNewArpEntry->idx].idleSecs=0;
	rg_db.arp[pNewArpEntry->idx].sendReqCount=0;
	rg_db.arp[pNewArpEntry->idx].routingIdx=routingIdx;
#if !defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	rg_db.arpValidSet[pNewArpEntry->idx>>5] |= (0x1<<(pNewArpEntry->idx&31));
#endif

	DEBUG("the HW arp[%d] has ip=%x, static=%d, nhIdx=%d",pNewArpEntry->idx,rg_db.arp[pNewArpEntry->idx].ipv4Addr,rg_db.arp[pNewArpEntry->idx].staticEntry,rg_db.arp[pNewArpEntry->idx].rtk_arp.nhIdx);

	//Add to hash head list
	//list_add(&pNewArpEntry->arp_list,&rg_db.hardwareArpTableHead[ipv4Addr&0xff]);

#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support lut traffic bit
	if(rg_db.lut[l2Idx].valid) rg_db.lut[l2Idx].arp_refCount++;
#endif

	*hwArpIdx = pNewArpEntry->idx;

	return (RT_ERR_RG_OK);
}

int _rtk_rg_hardwareArpTableDel(rtk_rg_arp_linkList_t *pDelArpEntry)
{
	rtk_l34_arp_entry_t asic_arp_entry;
#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support lut traffic bit
	int retval=0, i, l2Idx, nxthopRefFlag;
	rtk_l2_addr_table_t asic_l2_entry;
	uint32 arpL2Idx = rg_db.arp[pDelArpEntry->idx].rtk_arp.nhIdx;
#endif

	memset(&asic_arp_entry, 0, sizeof(rtk_l34_arp_entry_t));
	if(RTK_L34_ARPTABLE_SET(pDelArpEntry->idx, &asic_arp_entry) != RT_ERR_OK)
		RETURN_ERR(RT_ERR_RG_FAILED);

	//Delete from head list
	list_del_init(&pDelArpEntry->arp_list);

	//Clear data
	memset(&rg_db.arp[pDelArpEntry->idx],0,sizeof(rtk_rg_table_arp_t));
#if !defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	rg_db.arpValidSet[pDelArpEntry->idx>>5] &= ~(0x1<<(pDelArpEntry->idx&31));
#endif

	//Add back to free list
	list_add(&pDelArpEntry->arp_list,&rg_db.hardwareArpFreeListHead);

#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support lut traffic bit
	if(rg_db.lut[arpL2Idx].valid)
	{
		if(rg_db.lut[arpL2Idx].arp_refCount>0)
			rg_db.lut[arpL2Idx].arp_refCount--;
		if(rg_db.lut[arpL2Idx].arp_refCount==0)
		{
			nxthopRefFlag = 0;
			for(i=0; i<MAX_NEXTHOP_SW_TABLE_SIZE; i++)
			{
				if(arpL2Idx==rg_db.nexthop[i].rtk_nexthop.nhIdx)
				{
					nxthopRefFlag = 1;
					break;
				}
			}

			if(nxthopRefFlag==0)
			{
				rtk_rg_lut_linkList_t *pLutList, *pNextLutList;
				uint32 lutGroupIdx = rg_db.lut[arpL2Idx].lutGroupIdx;

				if(lutGroupIdx<MAX_LUT_HW_TABLE_SIZE){
					list_for_each_entry_safe(pLutList, pNextLutList, &rg_db.lutGroupTableHead[lutGroupIdx], lut_list)	//just return the first entry right behind of head
					{
						//Sync to LUT
						l2Idx = pLutList->idx;
						if(rg_db.lut[l2Idx].valid && rg_db.lut[l2Idx].rtk_lut.entryType==RTK_LUT_L2UC)
						{
							memcpy(&asic_l2_entry, &rg_db.lut[l2Idx].rtk_lut, sizeof(rtk_l2_addr_table_t));
							if((asic_l2_entry.entry.l2UcEntry.flags & RTK_L2_UCAST_FLAG_ARP_USED)!=0
								&& (asic_l2_entry.entry.l2UcEntry.flags & RTK_L2_UCAST_FLAG_STATIC)==0)
							{
								asic_l2_entry.entry.l2UcEntry.flags &= (~RTK_L2_UCAST_FLAG_ARP_USED);
								// [Call RTK_L2_ADDR_ADD directly] disable arp used
								retval = RTK_L2_ADDR_ADD(&asic_l2_entry.entry.l2UcEntry);
								ASSERT_EQ(retval,RT_ERR_OK);
							}
						}
					}
				}else
					DEBUG("lutGroupIdx is %d!",lutGroupIdx);
			}
		}
	}
#endif

	return (RT_ERR_RG_OK);
}

int32 _rtk_rg_deleteHwARP(int intfIdx, int routingIdx)
{
	int i;
	rtk_rg_arp_linkList_t *pHwArpList,*pNextHwArpList;

	for(i=0;i<MAX_ARP_HW_TABLE_HEAD;i++)
	{
		if(!list_empty(&rg_db.hardwareArpTableHead[i]))
		{
			list_for_each_entry_safe(pHwArpList,pNextHwArpList,&rg_db.hardwareArpTableHead[i],arp_list)
			{
				if(rg_db.arp[pHwArpList->idx].routingIdx==routingIdx)
				{
					//Free hardware ARP list
					ASSERT_EQ(_rtk_rg_hardwareArpTableDel(pHwArpList), RT_ERR_RG_OK);
				}
			}
		}
	}

	return (RT_ERR_RG_OK);
}

//support lut traffic bit
int _rtk_rg_lutCamListAdd(int l2Idx, uint32 addLutCamIdx)
{
	rtk_rg_lut_linkList_t *pLutCamEntry;

	pLutCamEntry = &rg_db.lutBCAMLinkList[addLutCamIdx-(MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE)];

	//Delete from head list
	list_del_init(&pLutCamEntry->lut_list);

	//Add to hash head list
	list_add_tail(&pLutCamEntry->lut_list,&rg_db.lutBCAMTableHead[l2Idx]);

	DEBUG("add lutCam[%d] to lutCamTable[%d]\n", addLutCamIdx, l2Idx);

	return (RT_ERR_RG_OK);
}

int _rtk_rg_lutCamListDel(uint32 delLutCamIdx)
{
	rtk_rg_lut_linkList_t *pDelLutCamEntry;
	rtk_rg_lut_linkList_t *pLutCamEntry,*pNextLutCamEntry;

	pDelLutCamEntry = &rg_db.lutBCAMLinkList[delLutCamIdx-(MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE)];

	//Delete from head list
	list_del_init(&pDelLutCamEntry->lut_list);

	if(list_empty(&rg_db.lutBCAMFreeListHead))
	{
		list_add(&pDelLutCamEntry->lut_list, &rg_db.lutBCAMFreeListHead);
	}
	else
	{
		list_for_each_entry_safe(pLutCamEntry,pNextLutCamEntry,&rg_db.lutBCAMFreeListHead,lut_list)
		{
			if(pDelLutCamEntry->idx < pLutCamEntry->idx)
			{
				list_add_tail(&pDelLutCamEntry->lut_list, &pLutCamEntry->lut_list);
				break;
			}

			if(&pNextLutCamEntry->lut_list == &rg_db.lutBCAMFreeListHead)
			{
				list_add(&pDelLutCamEntry->lut_list, &pLutCamEntry->lut_list);
				break;
			}
		}
	}

	//Clear data
	memset(&rg_db.lut[pDelLutCamEntry->idx],0,sizeof(rtk_rg_table_lut_t));

	return (RT_ERR_RG_OK);
}


rtk_rg_entryGetReturn_t _rtk_rg_findAndReclamIPv6mcEntry( rtk_ipv6_addr_t *gipv6,int32 ivlsvl)
{
	int32 l2Idx,search_index,count=0,first_invalid=-1;
	rtk_rg_lut_linkList_t *pLutBCAMList;

	if(ivlsvl==1)
	{
		WARNING("IVL NOT SUPPORT");
		return RT_ERR_FAILED;
	}

	l2Idx = _hash_ipm_dipv6(&(gipv6->ipv6_addr[0]));
	l2Idx<<=MAX_LUT_HASH_WAY_SHIFT;
	do
	{
		search_index = l2Idx+count;
		//DEBUG("search_idx is %d\n",search_index);
		if(rg_db.lut[search_index].valid==0)
		{
			if(first_invalid==-1)
				first_invalid=search_index;
			//break;	//empty, just add
			count++; //search from next entry
			continue;
		}

		if( (rg_db.lut[search_index].rtk_lut.entryType==RTK_LUT_L3V6MC) &&
			(memcmp(&(rg_db.lut[search_index].rtk_lut.entry.ipmcEntry.dip6.ipv6_addr[0]),&(gipv6->ipv6_addr[0]),sizeof(rtk_ipv6_addr_t))==0) )
		{
			DEBUG("v6Mc match!! froced replace LUT entry[%d] for v6MC entry!!",search_index);
			return search_index;
		}


		count++; //search from next entry
	}while(count < MAX_LUT_HASH_WAY_SIZE);


	if(count==MAX_LUT_HASH_WAY_SIZE)
	{

		//lookup BCAM first!
		list_for_each_entry(pLutBCAMList,&rg_db.lutBCAMTableHead[l2Idx>>MAX_LUT_HASH_WAY_SHIFT],lut_list)
		{
			if(rg_db.lut[pLutBCAMList->idx].valid &&
				(rg_db.lut[pLutBCAMList->idx].rtk_lut.entryType==RTK_LUT_L3V6MC) &&
				(memcmp(&(rg_db.lut[pLutBCAMList->idx].rtk_lut.entry.ipmcEntry.dip6.ipv6_addr[0]),&(gipv6->ipv6_addr[0]),sizeof(rtk_ipv6_addr_t))==0) )
			{
				DEBUG("v6Mc match!! froced replace LUT entry[%d] for v6MC entry!!",pLutBCAMList->idx);
				return pLutBCAMList->idx;
			}
		}

		if(first_invalid>=0)
			return first_invalid;	//not in 4-way and bCAM, use the first invalid to add one!
		else{
			count=_rtk_rg_layer2GarbageCollection(l2Idx);		//check if there is asynchronus between software and hardware table
			if(count==MAX_LUT_HASH_WAY_SIZE){

				search_index=_rtk_rg_layer2LeastRecentlyUsedReplace(l2Idx, FAIL); //replace the least recently used entry for new entry

				if(search_index==RG_RET_ENTRY_NOT_GET) {
					FIXME("must add software LUT entry for LUT entry full.");
					return RG_RET_ENTRY_NOT_GET;
				}
			}else
				search_index=l2Idx+count;
		}

	}
	return search_index;
}

//For LOOKUP_ON_DIP_AND_VID_FID
unsigned int _hash_dip_vidfid_sipidx_sipfilter(int is_ivl,unsigned int dip,unsigned int vid_fid, unsigned int sip_idx, unsigned int sip_filter_en)
{

	uint8 hashidx[8]={0};

	if(is_ivl)
	{
	/*
		IPv4 L3 Hash {GIP, VID, SIP_IDX, SIP_FILTER_EN } algorithm:
		Index[07] = GIP07¡òGIP15¡òGIP23¡òVID07¡òSIP_IDX3
		Index[06] = GIP06¡òGIP14¡òGIP22¡òVID06¡òSIP_IDX2
		Index[05] = GIP05¡òGIP13¡òGIP21¡òVID05¡òSIP_IDX1
		Index[04] = GIP04¡òGIP12¡òGIP20¡òVID04¡òSIP_IDX0
		Index[03] = GIP03¡òGIP11¡òGIP19¡òGIP27¡òVID03¡òVID11
		Index[02] = GIP02¡òGIP10¡òGIP18¡òGIP26¡òVID02¡òVID10
		Index[01] = GIP01¡òGIP09¡òGIP17¡òGIP25¡òVID01¡òVID09¡òSIP_FILTER_EN
		Index[00] = GIP00¡òGIP08¡òGIP16¡òGIP24¡òVID00¡òVID08
	*/
		hashidx[7]=1&((dip>>7)^(dip>>15)^(dip>>23)^(vid_fid>>7)^(sip_idx>>3));
		hashidx[6]=1&((dip>>6)^(dip>>14)^(dip>>22)^(vid_fid>>6)^(sip_idx>>2));
		hashidx[5]=1&((dip>>5)^(dip>>13)^(dip>>21)^(vid_fid>>5)^(sip_idx>>1));
		hashidx[4]=1&((dip>>4)^(dip>>12)^(dip>>20)^(vid_fid>>4)^(sip_idx>>0));
		hashidx[3]=1&((dip>>3)^(dip>>11)^(dip>>19)^(dip>>27)^(vid_fid>>3)^(vid_fid>>11));
		hashidx[2]=1&((dip>>2)^(dip>>10)^(dip>>18)^(dip>>26)^(vid_fid>>2)^(vid_fid>>10));
		hashidx[1]=1&((dip>>1)^(dip>>9)^( dip>>17)^(dip>>25)^(vid_fid>>1)^(vid_fid>>9)^sip_filter_en);
		hashidx[0]=1&((dip>>0)^(dip>>8)^( dip>>16)^(dip>>24)^(vid_fid>>0)^(vid_fid>>8));

		return ((hashidx[7]<<7)|(hashidx[6]<<6)|(hashidx[5]<<5)|(hashidx[4]<<4)|(hashidx[3]<<3)|(hashidx[2]<<2)|(hashidx[1]<<1)|(hashidx[0]<<0));
	}
	else
	{
	/*
		IPv4 L3 Hash{ GIP, FID, SIP_IDX, SIP_FILTER_EN } algorithm:
		Index[07] = GIP07¡òGIP15¡òGIP23¡òSIP_IDX3
		Index[06] = GIP06¡òGIP14¡òGIP22¡òSIP_IDX2
		Index[05] = GIP05¡òGIP13¡òGIP21¡òSIP_IDX1
		Index[04] = GIP04¡òGIP12¡òGIP20¡òSIP_IDX0
		Index[03] = GIP03¡òGIP11¡òGIP19¡òGIP27
		Index[02] = GIP02¡òGIP10¡òGIP18¡òGIP26
		Index[01] = GIP01¡òGIP09¡òGIP17¡òGIP25¡òSIP_FILTER_EN
		Index[00] = GIP00¡òGIP08¡òGIP16¡òGIP24¡òFID

	*/
		hashidx[7]=1&((dip>>7)^(dip>>15)^(dip>>23)^(sip_idx>>3));
		hashidx[6]=1&((dip>>6)^(dip>>14)^(dip>>22)^(sip_idx>>2));
		hashidx[5]=1&((dip>>5)^(dip>>13)^(dip>>21)^(sip_idx>>1));
		hashidx[4]=1&((dip>>4)^(dip>>12)^(dip>>20)^(sip_idx>>0));
		hashidx[3]=1&((dip>>3)^(dip>>11)^(dip>>19)^(dip>>27));
		hashidx[2]=1&((dip>>2)^(dip>>10)^(dip>>18)^(dip>>26));
		hashidx[1]=1&((dip>>1)^(dip>>9)^( dip>>17)^(dip>>25)^sip_filter_en);
		hashidx[0]=1&((dip>>0)^(dip>>8)^( dip>>16)^(dip>>24)^(vid_fid>>0));

		return ((hashidx[7]<<7)|(hashidx[6]<<6)|(hashidx[5]<<5)|(hashidx[4]<<4)|(hashidx[3]<<3)|(hashidx[2]<<2)|(hashidx[1]<<1)|(hashidx[0]<<0));


	}

}


//For: 9602bvb	Ip6 LOOKUP_ON_DIP
unsigned int _hash_ipm_dipv6(uint8 *dip)
{
	u8 hashidx[9]={0};

	/*
		IPv6 L3 Hash algorithm:
		Index[07] = DIP6_07¡òDIP6_15¡òDIP6_23¡òDIP6_31¡òDIP6_39 ¡òDIP6_107¡òDIP6_115
		Index[06] = DIP6_06¡òDIP6_14¡òDIP6_22¡òDIP6_30¡òDIP6_38 ¡òDIP6_106¡òDIP6_114
		Index[05] = DIP6_05¡òDIP6_13¡òDIP6_21¡òDIP6_29¡òDIP6_37 ¡òDIP6_105¡òDIP6_113
		Index[04] = DIP6_04¡òDIP6_12¡òDIP6_20¡òDIP6_28¡òDIP6_36 ¡òDIP6_104¡òDIP6_112
		Index[03] = DIP6_03¡òDIP6_11¡òDIP6_19¡òDIP6_27¡òDIP6_35 ¡òDIP6_43¡òDIP6_111
		Index[02] = DIP6_02¡òDIP6_10¡òDIP6_18¡òDIP6_26¡òDIP6_34 ¡òDIP6_42¡òDIP6_110
		Index[01] = DIP6_01¡òDIP6_09¡òDIP6_17¡òDIP6_25¡òDIP6_33 ¡òDIP6_41¡òDIP6_109¡òDIP6_117
		Index[00] = DIP6_00¡òDIP6_08¡òDIP6_16¡òDIP6_24¡òDIP6_32 ¡òDIP6_40¡òDIP6_108¡òDIP6_116

	*/
	hashidx[7]=((dip[15]>>7)&1)^((dip[14]>>7)&1)^((dip[13]>>7)&1)^((dip[12]>>7)&1)^((dip[11]>>7)&1)/*DIP6_39*/^((dip[2]>>3)&1)/*DIP6_107*/^((dip[1]>>3)&1)/*DIP6_115*/;
	hashidx[6]=((dip[15]>>6)&1)^((dip[14]>>6)&1)^((dip[13]>>6)&1)^((dip[12]>>6)&1)^((dip[11]>>6)&1)/*DIP6_38*/^((dip[2]>>2)&1)/*DIP6_106*/^((dip[1]>>2)&1)/*DIP6_114*/;
	hashidx[5]=((dip[15]>>5)&1)^((dip[14]>>5)&1)^((dip[13]>>5)&1)^((dip[12]>>5)&1)^((dip[11]>>5)&1)/*DIP6_37*/^((dip[2]>>1)&1)/*DIP6_105*/^((dip[1]>>1)&1)/*DIP6_113*/;
	hashidx[4]=((dip[15]>>4)&1)^((dip[14]>>4)&1)^((dip[13]>>4)&1)^((dip[12]>>4)&1)^((dip[11]>>4)&1)/*DIP6_36*/^((dip[2]>>0)&1)/*DIP6_104*/^((dip[1]>>0)&1)/*DIP6_112*/;
	hashidx[3]=((dip[15]>>3)&1)^((dip[14]>>3)&1)^((dip[13]>>3)&1)^((dip[12]>>3)&1)^((dip[11]>>3)&1)/*DIP6_35*/^((dip[10]>>3)&1) /*DIP6_43*/ ^((dip[2]>>7)&1)/*DIP6_111*/;
	hashidx[2]=((dip[15]>>2)&1)^((dip[14]>>2)&1)^((dip[13]>>2)&1)^((dip[12]>>2)&1)^((dip[11]>>2)&1)/*DIP6_34*/^((dip[10]>>2)&1) /*DIP6_42*/ ^((dip[2]>>6)&1)/*DIP6_110*/;
	hashidx[1]=((dip[15]>>1)&1)^((dip[14]>>1)&1)^((dip[13]>>1)&1)^((dip[12]>>1)&1)^((dip[11]>>1)&1)/*DIP6_33*/^((dip[10]>>1)&1) /*DIP6_41*/ ^((dip[2]>>5)&1)/*DIP6_109*/^((dip[1]>>5)&1)/*DIP6_117*/;
	hashidx[0]=((dip[15]>>0)&1)^((dip[14]>>0)&1)^((dip[13]>>0)&1)^((dip[12]>>0)&1)^((dip[11]>>0)&1)/*DIP6_32*/^((dip[10]>0)&1)  /*DIP6_40*/ ^((dip[2]>>4)&1)/*DIP6_108*/^((dip[1]>>4)&1)/*DIP6_116*/;

	return ((hashidx[7]<<7)|(hashidx[6]<<6)|(hashidx[5]<<5)|(hashidx[4]<<4)|(hashidx[3]<<3)|(hashidx[2]<<2)|(hashidx[1]<<1)|(hashidx[0]<<0));

}

/* only support SVL mode*/
int32 _rtk_rg_apollo_ipv6MultiCastFlow_add(rtk_rg_ipv6MulticastFlow_t *ipv6McFlow,int *flow_idx)
{
	int search_index;
	rtk_l2_ipMcastAddr_t lut;

	memset(&lut,0,sizeof(rtk_l2_ipMcastAddr_t));
	memcpy(&(lut.dip6.ipv6_addr[0]),&(ipv6McFlow->groupIp6.ipv6_addr[0]),16);
	lut.flags=RTK_L2_IPMCAST_FLAG_STATIC|RTK_L2_IPMCAST_FLAG_L3MC_ROUTE_EN|RTK_L2_IPMCAST_FLAG_IPV6;

	_rtk_rg_apollo_ipmcMultiCast_transTbl_add(1,&lut);

	_rtk_rg_portmask_translator(ipv6McFlow->portMaskIPMC6,&lut.portmask,&lut.ext_portmask);

	search_index=_rtk_rg_findAndReclamIPv6mcEntry(&(ipv6McFlow->groupIp6),0);

	ASSERT_EQ(RTK_L2_IPMCASTADDR_ADD(&lut),RT_ERR_OK);
#if 0  //for debug
	if(search_index != lut.index)
	{
		int i;
		rtk_l2_ipmcMode_t mode;
		rtk_l2_ipv6mcMode_get(&mode);
		WARNING("search_index= %d lut.index=%d rtk_l2_ipv6mcMode_get mode=%d ",search_index,lut.index,mode);
		WARNING("hash =%d ",_hash_ipm_dipv6(&lut.dip6.ipv6_addr[0] ));

		printk("ipv6addr = ");
		for(i=0;i<16;i++){
			printk("%02x",lut.dip6.ipv6_addr[i]);
			if((i%2)==1)
				printk(" ");
		}
		printk("\n");
	}
#endif
	ASSERT_EQ(search_index,lut.index);

	*flow_idx=lut.index;
	return (RT_ERR_RG_OK);
}

//mib counter and cf decision
void _rtk_rg_IntfIdxDecisionForCF(rtk_rg_pktHdr_t *pPktHdr)
{
	int i;
	uint8 *checkMac;
	int checkVlanID=0;
	int checkTagif;
	unsigned int checkIP=0;
	rtk_l34_netif_entry_t *pNetif;

	if(pPktHdr->ingressPort==RTK_RG_PORT_PON){
		//From PON port, and PON is CF-sel port after rg_init.
		pPktHdr->mibDirect=RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM;
		checkMac=pPktHdr->dmac;
		checkIP=pPktHdr->ipv4Dip;
		checkTagif=((pPktHdr->tagif&CVLAN_TAGIF)>0);
		if(checkTagif)
			checkVlanID=pPktHdr->ctagVid;
	}else{
		//TRACE("pPktHdr->egressMacPort=%d",pPktHdr->egressMacPort);
		if((pPktHdr->egressMacPort!=RTK_RG_MAC_PORT_PON) && ((pPktHdr->pDmac[0]&1) == 0) && (pPktHdr->fwdDecision != RG_FWD_DECISION_NO_PS_BC)){	// unknown DA flooding: continue to find matched interface
			pPktHdr->mibNetifIdx=rg_db.systemGlobal.mib_l2_wanif_idx;
			pPktHdr->mibDirect=RTK_RG_CLASSIFY_DIRECTION_END;	//do not update mib
			TRACE("[MiBCnt]Non-CF port...CF use default_l2_intf_idx %d",pPktHdr->mibNetifIdx);
			return;
		}
		pPktHdr->mibDirect=RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
		checkMac=pPktHdr->pSmac;
		if(pPktHdr->tagif&IPV4_TAGIF)checkIP=ntohl(*pPktHdr->pIpv4Sip);	//Binding>L34>HSB
		checkVlanID=pPktHdr->internalVlanID;
		if(pPktHdr->dmac2VlanID>=0)
			checkTagif=pPktHdr->dmac2VlanTagif;
		else
			checkTagif=((rg_db.vlan[checkVlanID].UntagPortmask.bits[0]&(0x1<<RTK_RG_PORT_PON))==0);

		//for packet upstream, we can use ppkthdr->netifidx to check if compatible,
		//if netifidx is FAIL, check each for L2 interface.
		if(pPktHdr->netifIdx>=0){
			// If netifIdx is meaningful(!=-1), we could always use it as mibNitifIdx no matter CVLAN is matched or not.
			pPktHdr->mibNetifIdx=pPktHdr->netifIdx;
			pNetif=&rg_db.netif[pPktHdr->netifIdx].rtk_netif;
			if(pNetif->ipAddr!=0 && pNetif->ipAddr!=checkIP)goto CONF_ERR;
			if(pNetif->isL34 && memcmp(checkMac,pNetif->gateway_mac.octet,ETHER_ADDR_LEN))goto CONF_ERR;
			if(pNetif->isCtagIf!=checkTagif)goto CONF_ERR;
			if(pNetif->isCtagIf && pNetif->vlan_id!=checkVlanID)goto CONF_ERR;
			TRACE("[MiBCnt]DirectTX to netifIdx[%d] as CF wan_intf_idx!!",pPktHdr->mibNetifIdx);
			return;
CONF_ERR:
			TRACE("[MiBCnt]Egress packet is incompatible with netifIdx[%d]..",pPktHdr->netifIdx);
			return;
		}
	}

	//Check for each interface entry in sw
	for(i=0;i<rg_db.systemGlobal.wanIntfTotalNum;i++){
		pNetif=&rg_db.netif[rg_db.systemGlobal.wanIntfGroup[i].index].rtk_netif;
		if(pNetif->ipAddr!=0 && pNetif->ipAddr!=checkIP)continue;
		if(pNetif->isL34 && memcmp(checkMac,pNetif->gateway_mac.octet,ETHER_ADDR_LEN))continue;
		if(pNetif->isCtagIf!=checkTagif)continue;
		if(pNetif->isCtagIf && pNetif->vlan_id!=checkVlanID)continue;
		TRACE("[MiBCnt]Hit! use netifIdx[%d] as CF wan_intf_idx!!",rg_db.systemGlobal.wanIntfGroup[i].index);
		pPktHdr->mibNetifIdx=rg_db.systemGlobal.wanIntfGroup[i].index;
		return;
	}

	#if 1
	if(pPktHdr->pDmac[0]&1){
		rtk_mac_t bcMac;
		memset(bcMac.octet, 0xff, ETHER_ADDR_LEN);
		if(!memcmp(pPktHdr->pDmac, bcMac.octet, ETHER_ADDR_LEN)){
			pPktHdr->mibNetifIdx=rg_db.systemGlobal.mib_l2_wanif_idx;
			TRACE("[MiBCnt]Broadcast...CF use default_l2_intf_idx %d",pPktHdr->mibNetifIdx);
		}
		else{
			pPktHdr->mibNetifIdx=rg_db.systemGlobal.mib_mc_wanif_idx;
			TRACE("[MiBCnt]Multicast...CF use default_mc_intf_idx %d",pPktHdr->mibNetifIdx);
		}
		pPktHdr->mibDirect=RTK_RG_CLASSIFY_DIRECTION_END;	//do not update mib
		return;
	}
	#endif

	//unmatch any WAN..
	if(pPktHdr->mibNetifIdx==FAIL){
		pPktHdr->mibNetifIdx=rg_db.systemGlobal.mib_l2_wanif_idx;
		pPktHdr->mibDirect=RTK_RG_CLASSIFY_DIRECTION_END;	//do not update mib
		TRACE("[MiBCnt]unHit...CF use default_l2_intf_idx %d",pPktHdr->mibNetifIdx);
	}
}

