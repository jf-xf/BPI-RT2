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

#include <rtk_rg_apolloPro_liteRomeDriver.h>
#include <rtk_rg_apolloPro_asicDriver.h>
#include <hal/chipdef/rtl9607c/rtk_rtl9607c_reg_struct.h>
#include <rtk_rg_apolloPro_internal.h>
#if defined(CONFIG_RG_G3_SERIES)
#include <rtk_rg_g3_internal.h>
#endif
#include <rtk_rg_igmpsnooping.h>

#include <rtk_rg_acl.h>
#include <rtk_rg_cpuReason.h>


#ifndef CONFIG_RT_EDP_API
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
	.rtk_rg_multicastFlow_add	=rtk_rg_apolloPro_multicastFlow_add,
	.rtk_rg_multicastFlow_del	=rtk_rg_apolloPro_multicastFlow_del,
	/* martin zhu add */
	.rtk_rg_l2MultiCastFlow_add =rtk_rg_apolloPro_l2MultiCastFlow_add,
	.rtk_rg_multicastFlow_find	=rtk_rg_apolloPro_multicastFlow_find,
	.rtk_rg_macEntry_add	=rtk_rg_apolloPro_macEntry_add,
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
	.rtk_rg_portBasedCVlanId_get	=rtk_rg_apolloPro_portBasedCVlanId_get,
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
	.rtk_rg_interfaceMibCounter_del=rtk_rg_apolloPro_interfaceMibCounter_del,
	.rtk_rg_interfaceMibCounter_get=rtk_rg_apolloPro_interfaceMibCounter_get,
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
	.rtk_rg_svlanTpid2_get= rtk_rg_apolloPro_svlanTpid2_get,
	.rtk_rg_svlanTpid2_set= rtk_rg_apolloPro_svlanTpid2_set,
	.rtk_rg_svlanTpid2_enable_get=rtk_rg_apolloPro_svlanTpid2_enable_get,
	.rtk_rg_svlanTpid2_enable_set=rtk_rg_apolloPro_svlanTpid2_enable_set,
//165
	.rtk_rg_hostPoliceControl_set=rtk_rg_apolloPro_hostPoliceControl_set,
	.rtk_rg_hostPoliceControl_get=rtk_rg_apolloPro_hostPoliceControl_get,
	.rtk_rg_hostPoliceLogging_get=rtk_rg_apolloPro_hostPoliceLogging_get,
	.rtk_rg_hostPoliceLogging_del=rtk_rg_apolloPro_hostPoliceLogging_del,
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
	.rtk_rg_flowMibCounter_get=rtk_rg_apolloPro_flowMibCounter_get,
	.rtk_rg_flowMibCounter_reset=rtk_rg_apolloPro_flowMibCounter_reset,
	.rtk_rg_softwareIdleTime_set=rtk_rg_apollo_softwareIdleTime_set,
//185
	.rtk_rg_softwareIdleTime_get=rtk_rg_apollo_softwareIdleTime_get,
	.rtk_rg_funcbasedMeter_set=rtk_rg_apolloPro_funcbasedMeter_set,
	.rtk_rg_funcbasedMeter_get=rtk_rg_apolloPro_funcbasedMeter_get,
	.rtk_rg_flowHiPriEntry_add=rtk_rg_apolloPro_HiPriEntry_add,
	.rtk_rg_flowHiPriEntry_del=rtk_rg_apolloPro_HiPriEntry_del,
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
	.rtk_rg_urlflowPri_add=rtk_rg_apolloPro_UrlPri_add,
	.rtk_rg_urlflowPri_del=rtk_rg_apolloPro_UrlPri_del,	
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
#endif

#define EXTRATAG_BUFFER_OFFSET 64	// offset per each content extra tag action list





rtk_rg_err_code_t rtk_rg_apolloPro_interfaceMibCounter_del(int intf_idx)
{
	int32 ret = RT_ERR_RG_OK;
#if defined(CONFIG_RG_G3_SERIES)
	rtk_rg_mac_port_idx_t portIdx;
	int genIntf_idx;
#endif

	//reset software and hardward counter
	if(intf_idx<0 || intf_idx>=MAX_NETIF_SW_TABLE_SIZE)
		RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);

	if(intf_idx>=MAX_NETIF_HW_TABLE_SIZE)
		ret=RT_ERR_RG_OK;
	else
	{
#if !defined(CONFIG_RG_G3_SERIES)
		ret=rtk_rg_asic_netifMib_reset(intf_idx);
#else //defined(CONFIG_RG_G3_SERIES)
		if(rg_db.systemGlobal.interfaceInfo[intf_idx].storedInfo.is_wan)
		{
			//WAN
			portIdx = rg_db.systemGlobal.interfaceInfo[intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_port_idx;
			if((genIntf_idx = _rtk_rg_g3_generic_intf_index_get(FALSE, rg_db.systemGlobal.interfaceInfo[intf_idx].lanWan_groupIdx_for_genericIntf, portIdx, FAIL)) == FAIL)
				RETURN_ERR(RT_ERR_RG_FAILED);
			ret = ca_l3_intf_stats_clear(G3_DEF_DEVID, genIntf_idx);
		}
		else
		{
			//LAN
			for(portIdx = 0 ; portIdx <= RTK_RG_MAC_PORT_PON ; portIdx++)
			{
				if(RG_INVALID_MAC_PORT(portIdx)) continue;

				if((rg_db.systemGlobal.interfaceInfo[intf_idx].storedInfo.lan_intf.port_mask.portmask & (0x1 << portIdx)) == 0x0) continue;

				if((genIntf_idx = _rtk_rg_g3_generic_intf_index_get(TRUE, rg_db.systemGlobal.interfaceInfo[intf_idx].lanWan_groupIdx_for_genericIntf, portIdx, FAIL)) == FAIL) continue;

				ret = ca_l3_intf_stats_clear(G3_DEF_DEVID, genIntf_idx);
			}
			if(rg_db.systemGlobal.interfaceInfo[intf_idx].storedInfo.lan_intf.port_mask.portmask & (0x1<<RTK_RG_PORT_WLAN_OF_MASTER_CPU))
			{
				for(genIntf_idx=GENERIC_INTERFACE_INDEX_OF_WLAN0_ROOT; genIntf_idx<(MAX_GENERIC_INTERFACE_PER_LAN + NETIF_START_IDX); genIntf_idx++)
				{
/*					
					switch(genIntf_idx)
					{
						case GENERIC_INTERFACE_INDEX_OF_WLAN0_ROOT:
							portIdx = RTK_RG_MAC_PORT_CPU_WLAN0_ROOT;
							break;
						case GENERIC_INTERFACE_INDEX_OF_WLAN0_VAP0:
							portIdx = RTK_RG_MAC_PORT_CPU_WLAN0_VAP0;
							break;
						case GENERIC_INTERFACE_INDEX_OF_WLAN0_VAP1:
							portIdx = RTK_RG_MAC_PORT_CPU_WLAN0_VAP1;
							break;
						case GENERIC_INTERFACE_INDEX_OF_WLAN0_VAP2:
							portIdx = RTK_RG_MAC_PORT_CPU_WLAN0_VAP2;
							break;
						case GENERIC_INTERFACE_INDEX_OF_WLAN0_VAP3:
							portIdx = RTK_RG_MAC_PORT_CPU_WLAN0_VAP3;
							break;
						default:
							portIdx = RTK_RG_MAC_PORT_CPU_WLAN1_AND_OTHERS;
							break;
					}
*/					
					genIntf_idx += (rg_db.systemGlobal.interfaceInfo[intf_idx].lanWan_groupIdx_for_genericIntf * MAX_GENERIC_INTERFACE_PER_LAN);
					ret = ca_l3_intf_stats_clear(G3_DEF_DEVID, genIntf_idx);
				}
			}
		}


#endif //!defined(CONFIG_RG_G3_SERIES)
	}

	if(ret==RT_ERR_RG_OK)
	{
		bzero(&rg_db.netif[intf_idx].netifMib, sizeof(rtk_rg_netifMib_entry_t));
		return (RT_ERR_RG_OK);
	}
	else
	{
		RETURN_ERR(RT_ERR_RG_FAILED);
	}
}

rtk_rg_err_code_t rtk_rg_apolloPro_interfaceMibCounter_get(rtk_rg_netifMib_entry_t *pNetifMib)
{
	uint32 netifIdx;
#if !defined(CONFIG_RG_G3_SERIES)
	rtk_rg_asic_netifMib_entry_t asicNetifMib;
#else //defined(CONFIG_RG_G3_SERIES)
	struct ca_int_l3_intf_s intf;
	ca_l3_intf_stats_t intfMib;
	int genIntf_idx;
	rtk_rg_mac_port_idx_t portIdx;
#endif

	//get hardware counter and plus software counter if has.
	if(pNetifMib==NULL)RETURN_ERR(RT_ERR_RG_NULL_POINTER);
	if(pNetifMib->netifIdx>=MAX_NETIF_SW_TABLE_SIZE)RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);

	netifIdx = pNetifMib->netifIdx;
	memset(pNetifMib, 0, sizeof(rtk_rg_netifMib_entry_t));
	pNetifMib->netifIdx = netifIdx;
	if(!rg_db.systemGlobal.interfaceInfo[netifIdx].valid)
		RETURN_ERR(RT_ERR_RG_OK);// non-valid interface, do nothing!

	if(netifIdx>=MAX_NETIF_HW_TABLE_SIZE)
	{
		pNetifMib->in_intf_uc_packet_cnt=rg_db.netif[netifIdx].netifMib.in_intf_uc_packet_cnt;
		pNetifMib->in_intf_uc_byte_cnt=rg_db.netif[netifIdx].netifMib.in_intf_uc_byte_cnt;
		pNetifMib->in_intf_mc_packet_cnt=rg_db.netif[netifIdx].netifMib.in_intf_mc_packet_cnt;
		pNetifMib->in_intf_mc_byte_cnt=rg_db.netif[netifIdx].netifMib.in_intf_mc_byte_cnt;
		pNetifMib->in_intf_bc_packet_cnt=rg_db.netif[netifIdx].netifMib.in_intf_bc_packet_cnt;
		pNetifMib->in_intf_bc_byte_cnt=rg_db.netif[netifIdx].netifMib.in_intf_bc_byte_cnt;

		pNetifMib->out_intf_uc_packet_cnt=rg_db.netif[netifIdx].netifMib.out_intf_uc_packet_cnt;
		pNetifMib->out_intf_uc_byte_cnt=rg_db.netif[netifIdx].netifMib.out_intf_uc_byte_cnt;
		pNetifMib->out_intf_mc_packet_cnt=rg_db.netif[netifIdx].netifMib.out_intf_mc_packet_cnt;
		pNetifMib->out_intf_mc_byte_cnt=rg_db.netif[netifIdx].netifMib.out_intf_mc_byte_cnt;
		pNetifMib->out_intf_bc_packet_cnt=rg_db.netif[netifIdx].netifMib.out_intf_bc_packet_cnt;
		pNetifMib->out_intf_bc_byte_cnt=rg_db.netif[netifIdx].netifMib.out_intf_bc_byte_cnt;
		return (RT_ERR_RG_OK);
	}
	else
	{
#if !defined(CONFIG_RG_G3_SERIES)
		if(rtk_rg_asic_netifMib_get(netifIdx, &asicNetifMib)==RT_ERR_RG_OK)
		{
			pNetifMib->in_intf_uc_packet_cnt=asicNetifMib.in_intf_uc_packet_cnt+rg_db.netif[netifIdx].netifMib.in_intf_uc_packet_cnt;
			pNetifMib->in_intf_uc_byte_cnt=asicNetifMib.in_intf_uc_byte_cnt+rg_db.netif[netifIdx].netifMib.in_intf_uc_byte_cnt;
			pNetifMib->in_intf_mc_packet_cnt=asicNetifMib.in_intf_mc_packet_cnt+rg_db.netif[netifIdx].netifMib.in_intf_mc_packet_cnt;
			pNetifMib->in_intf_mc_byte_cnt=asicNetifMib.in_intf_mc_byte_cnt+rg_db.netif[netifIdx].netifMib.in_intf_mc_byte_cnt;
			pNetifMib->in_intf_bc_packet_cnt=asicNetifMib.in_intf_bc_packet_cnt+rg_db.netif[netifIdx].netifMib.in_intf_bc_packet_cnt;
			pNetifMib->in_intf_bc_byte_cnt=asicNetifMib.in_intf_bc_byte_cnt+rg_db.netif[netifIdx].netifMib.in_intf_bc_byte_cnt;

			pNetifMib->out_intf_uc_packet_cnt=asicNetifMib.out_intf_uc_packet_cnt+rg_db.netif[netifIdx].netifMib.out_intf_uc_packet_cnt;
			pNetifMib->out_intf_uc_byte_cnt=asicNetifMib.out_intf_uc_byte_cnt+rg_db.netif[netifIdx].netifMib.out_intf_uc_byte_cnt;
			pNetifMib->out_intf_mc_packet_cnt=asicNetifMib.out_intf_mc_packet_cnt+rg_db.netif[netifIdx].netifMib.out_intf_mc_packet_cnt;
			pNetifMib->out_intf_mc_byte_cnt=asicNetifMib.out_intf_mc_byte_cnt+rg_db.netif[netifIdx].netifMib.out_intf_mc_byte_cnt;
			pNetifMib->out_intf_bc_packet_cnt=asicNetifMib.out_intf_bc_packet_cnt+rg_db.netif[netifIdx].netifMib.out_intf_bc_packet_cnt;
			pNetifMib->out_intf_bc_byte_cnt=asicNetifMib.out_intf_bc_byte_cnt+rg_db.netif[netifIdx].netifMib.out_intf_bc_byte_cnt;
			return (RT_ERR_RG_OK);
		}
#else //defined(CONFIG_RG_G3_SERIES)
		pNetifMib->in_intf_uc_packet_cnt=rg_db.netif[netifIdx].netifMib.in_intf_uc_packet_cnt;
		pNetifMib->in_intf_uc_byte_cnt=rg_db.netif[netifIdx].netifMib.in_intf_uc_byte_cnt;
		pNetifMib->in_intf_mc_packet_cnt=rg_db.netif[netifIdx].netifMib.in_intf_mc_packet_cnt;
		pNetifMib->in_intf_mc_byte_cnt=rg_db.netif[netifIdx].netifMib.in_intf_mc_byte_cnt;
		pNetifMib->in_intf_bc_packet_cnt=rg_db.netif[netifIdx].netifMib.in_intf_bc_packet_cnt;
		pNetifMib->in_intf_bc_byte_cnt=rg_db.netif[netifIdx].netifMib.in_intf_bc_byte_cnt;

		pNetifMib->out_intf_uc_packet_cnt=rg_db.netif[netifIdx].netifMib.out_intf_uc_packet_cnt;
		pNetifMib->out_intf_uc_byte_cnt=rg_db.netif[netifIdx].netifMib.out_intf_uc_byte_cnt;
		pNetifMib->out_intf_mc_packet_cnt=rg_db.netif[netifIdx].netifMib.out_intf_mc_packet_cnt;
		pNetifMib->out_intf_mc_byte_cnt=rg_db.netif[netifIdx].netifMib.out_intf_mc_byte_cnt;
		pNetifMib->out_intf_bc_packet_cnt=rg_db.netif[netifIdx].netifMib.out_intf_bc_packet_cnt;
		pNetifMib->out_intf_bc_byte_cnt=rg_db.netif[netifIdx].netifMib.out_intf_bc_byte_cnt;
		if(rg_db.systemGlobal.interfaceInfo[netifIdx].storedInfo.is_wan)
		{
			//WAN
			portIdx = rg_db.systemGlobal.interfaceInfo[netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_port_idx;
			if((genIntf_idx = _rtk_rg_g3_generic_intf_index_get(FALSE, rg_db.systemGlobal.interfaceInfo[netifIdx].lanWan_groupIdx_for_genericIntf, portIdx, FAIL)) == FAIL)
				RETURN_ERR(RT_ERR_RG_FAILED);
			if(ca_generic_intf_get(G3_DEF_DEVID, genIntf_idx, &intf) == CA_E_OK)
			{
				memset(&intfMib, 0, sizeof(ca_l3_intf_stats_t));
				ca_l3_intf_stats_get(G3_DEF_DEVID, genIntf_idx, FALSE, &intfMib);
				pNetifMib->in_intf_uc_packet_cnt += intfMib.rx_pkts;
				pNetifMib->in_intf_uc_byte_cnt += intfMib.rx_bytes + (4 * intfMib.rx_pkts)/*CRC*/;
				pNetifMib->out_intf_uc_packet_cnt += intfMib.tx_pkts;
				pNetifMib->out_intf_uc_byte_cnt += intfMib.tx_bytes + (4 * intfMib.tx_pkts)/*CRC*/;
			}
			return (RT_ERR_RG_OK);
		}
		else
		{
			//LAN: add its all generic interface HW counter
			for(portIdx = 0 ; portIdx <= RTK_RG_MAC_PORT_PON ; portIdx++)
			{
				if(RG_INVALID_MAC_PORT(portIdx)) continue;

				if((rg_db.systemGlobal.interfaceInfo[netifIdx].storedInfo.lan_intf.port_mask.portmask & (0x1 << portIdx)) == 0x0) continue;

				if((genIntf_idx = _rtk_rg_g3_generic_intf_index_get(TRUE, rg_db.systemGlobal.interfaceInfo[netifIdx].lanWan_groupIdx_for_genericIntf, portIdx, FAIL)) == FAIL) continue;

				if(ca_generic_intf_get(G3_DEF_DEVID, genIntf_idx, &intf) == CA_E_OK)
				{
					memset(&intfMib, 0, sizeof(ca_l3_intf_stats_t));
					ca_l3_intf_stats_get(G3_DEF_DEVID, genIntf_idx, FALSE, &intfMib);
					pNetifMib->in_intf_uc_packet_cnt += intfMib.rx_pkts;
					pNetifMib->in_intf_uc_byte_cnt += intfMib.rx_bytes + (4 * intfMib.rx_pkts)/*CRC*/;
					pNetifMib->out_intf_uc_packet_cnt += intfMib.tx_pkts;
					pNetifMib->out_intf_uc_byte_cnt += intfMib.tx_bytes + (4 * intfMib.tx_pkts);
				}
			}
			if(rg_db.systemGlobal.interfaceInfo[netifIdx].storedInfo.lan_intf.port_mask.portmask & (0x1<<RTK_RG_PORT_WLAN_OF_MASTER_CPU))
			{
				for(genIntf_idx=GENERIC_INTERFACE_INDEX_OF_WLAN0_ROOT; genIntf_idx<(MAX_GENERIC_INTERFACE_PER_LAN + NETIF_START_IDX); genIntf_idx++)
				{
/*					
					switch(genIntf_idx)
					{
						case GENERIC_INTERFACE_INDEX_OF_WLAN0_ROOT:
							portIdx = RTK_RG_MAC_PORT_CPU_WLAN0_ROOT;
							break;
						case GENERIC_INTERFACE_INDEX_OF_WLAN0_VAP0:
							portIdx = RTK_RG_MAC_PORT_CPU_WLAN0_VAP0;
							break;
						case GENERIC_INTERFACE_INDEX_OF_WLAN0_VAP1:
							portIdx = RTK_RG_MAC_PORT_CPU_WLAN0_VAP1;
							break;
						case GENERIC_INTERFACE_INDEX_OF_WLAN0_VAP2:
							portIdx = RTK_RG_MAC_PORT_CPU_WLAN0_VAP2;
							break;
						case GENERIC_INTERFACE_INDEX_OF_WLAN0_VAP3:
							portIdx = RTK_RG_MAC_PORT_CPU_WLAN0_VAP3;
							break;
						default:
							portIdx = RTK_RG_MAC_PORT_CPU_WLAN1_AND_OTHERS;
							break;
					}
*/					
					genIntf_idx += (rg_db.systemGlobal.interfaceInfo[netifIdx].lanWan_groupIdx_for_genericIntf * MAX_GENERIC_INTERFACE_PER_LAN);
					if(ca_generic_intf_get(G3_DEF_DEVID, genIntf_idx, &intf) == CA_E_OK)
					{
						memset(&intfMib, 0, sizeof(ca_l3_intf_stats_t));
						ca_l3_intf_stats_get(G3_DEF_DEVID, genIntf_idx, FALSE, &intfMib);
						pNetifMib->in_intf_uc_packet_cnt += intfMib.rx_pkts;
						pNetifMib->in_intf_uc_byte_cnt += intfMib.rx_bytes + (4 * intfMib.rx_pkts)/*CRC*/;
						pNetifMib->out_intf_uc_packet_cnt += intfMib.tx_pkts;
						pNetifMib->out_intf_uc_byte_cnt += intfMib.tx_bytes + (4 * intfMib.tx_pkts)/*CRC*/;
					}
				}
			}
			return (RT_ERR_RG_OK);
		}
#endif
	}
	RETURN_ERR(RT_ERR_RG_FAILED);
}


rtk_rg_err_code_t rtk_rg_apolloPro_svlanTpid2_enable_set(rtk_rg_enable_t enable)
{
	if(enable<0 || enable>=RTK_RG_ENABLE_END)
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	assert_ok(RTK_SVLAN_TPIDENABLE_SET(1, enable));
	return (RT_ERR_RG_OK);
}
rtk_rg_err_code_t rtk_rg_apolloPro_svlanTpid2_enable_get(rtk_rg_enable_t *pEnable)
{
	if(pEnable==NULL)
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	assert_ok(rtk_svlan_tpidEnable_get(1, (rtk_enable_t *)pEnable));
	if(*pEnable!=rg_db.systemGlobal.tpid2_en)WARNING("Svlan TPID2_enable is different between software and hardware. Please check if RG APIs is mixed with RTK APIs!");
	return (RT_ERR_RG_OK);
}
rtk_rg_err_code_t rtk_rg_apolloPro_svlanTpid2_set(uint32 svlan_tag_id)
{
	if(svlan_tag_id>0xffff)
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	assert_ok(RTK_SVLAN_TPIDENTRY_SET(1, svlan_tag_id));
	return (RT_ERR_RG_OK);
}
rtk_rg_err_code_t rtk_rg_apolloPro_svlanTpid2_get(uint32 *pSvlanTagId)
{
	if(pSvlanTagId==NULL)
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	assert_ok(rtk_svlan_tpidEntry_get(1, pSvlanTagId));
	if(*pSvlanTagId!=rg_db.systemGlobal.tpid2)WARNING("Svlan TPID2 is different between software and hardware. Please check if RG APIs is mixed with RTK APIs!");
	return (RT_ERR_RG_OK);
}

rtk_rg_err_code_t rtk_rg_apolloPro_hostPoliceControl_set(rtk_rg_hostPoliceControl_t *pHostPoliceControl, int host_idx)
{
	int lut_idx=-1, i, j;
	rtk_rg_macEntry_t macEntry;
	rtk_rg_hostPoliceLinkList_t *pHostEntry,*pNextEntry;
	rtk_mac_t zeroMac;
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	int meterIdx = FAIL;
#elif defined(CONFIG_RG_G3_SERIES)
	rtk_rg_g3_mac_hostPolicing_info_t oriMac_hp_info, newMac_hp_info;
	unsigned char oriMac_ifUpdatePolIdx = DISABLE, newMac_ifUpdatePolIdx = DISABLE; // the flags used to identify if the flow policer index is needed update or not
	rtk_mac_t oriMac, newMac;
	rtk_rg_hostPoliceLogging_t hpMibEntry;
#endif
	rtk_rg_trapSpecificLengthAck_t trapAck;
	uint8 trapSpecificLengthAck_reservedAclAdd = FALSE;

	//Check parameter
	if(pHostPoliceControl==NULL)RETURN_ERR(RT_ERR_RG_NULL_POINTER);
	if(host_idx<0 || host_idx>=HOST_POLICING_TABLE_SIZE)RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	/*decide meter index according to meter mode and add HW host policing entry*/

	if(rg_db.systemGlobal.funbasedMeter_mode == RTK_RG_METERMODE_HW_INDEX)
	{
		//pHostPoliceControl->limitMeterIdx is HW meter index here, use it directly
		meterIdx = pHostPoliceControl->limitMeterIdx;
	}
	else if(rg_db.systemGlobal.funbasedMeter_mode == RTK_RG_METERMODE_SW_INDEX)
	{
		//pHostPoliceControl->limitMeterIdx is SW meter(funcbased Meter) index here, use it to find its mapping HW index
		if(rg_db.systemGlobal.funbasedMeter[RTK_RG_METER_HOSTPOL][pHostPoliceControl->limitMeterIdx].state == RTK_RG_DISABLED)
		{
			//funcbased Meter is disabled, not set to HW
			meterIdx = FAIL;
		}
		else
			meterIdx = rg_db.systemGlobal.funbasedMeter[RTK_RG_METER_HOSTPOL][pHostPoliceControl->limitMeterIdx].hwIdx;
	}

	ASSERT_EQ(rtk_rate_hostMacAddr_set(host_idx,&pHostPoliceControl->macAddr),RT_ERR_OK);
	ASSERT_EQ(rtk_stat_hostState_set(host_idx,pHostPoliceControl->mibCountCtrl),RT_ERR_OK);

	if(rg_db.systemGlobal.funbasedMeter_mode == RTK_RG_METERMODE_HW_INDEX || ((rg_db.systemGlobal.funbasedMeter_mode == RTK_RG_METERMODE_SW_INDEX) && (meterIdx != FAIL)))
	{
		//At RTK_RG_METERMODE_NOT_INIT mode, meter index cannot be decided, thus skip add to HW
		ASSERT_EQ(rtk_rate_hostIgrBwCtrlState_set(host_idx,pHostPoliceControl->ingressLimitCtrl),RT_ERR_OK);
		ASSERT_EQ(rtk_rate_hostEgrBwCtrlState_set(host_idx,pHostPoliceControl->egressLimitCtrl),RT_ERR_OK);
		ASSERT_EQ(rtk_rate_hostBwCtrlMeterIdx_set(host_idx,meterIdx),RT_ERR_OK);	//meaningful only when igr or egr ctrl enabled
	}
#endif
	memset(&zeroMac, 0, sizeof(rtk_mac_t));
	// recover host_dmac_rateLimit of lut
	if(memcmp(rg_db.hostPoliceList[host_idx].info.macAddr.octet, zeroMac.octet, ETHER_ADDR_LEN))
	{
		memcpy(macEntry.mac.octet, rg_db.hostPoliceList[host_idx].info.macAddr.octet, ETHER_ADDR_LEN);
		lut_idx=-1;
		if(rtk_rg_apollo_macEntry_find(&macEntry, &lut_idx)==RT_ERR_RG_OK)
		{
			DEBUG("Disable host_dmac_rateLimit of lut[%d] mac=%pM", lut_idx, macEntry.mac.octet);
			rg_db.lut[lut_idx].host_dmac_rateLimit = 0;
		}
#if defined(CONFIG_RG_G3_SERIES)
		if(memcmp(pHostPoliceControl->macAddr.octet, rg_db.hostPoliceList[host_idx].info.macAddr.octet, ETHER_ADDR_LEN))
		{	//oringinal MAC != new MAC
			oriMac_ifUpdatePolIdx = ENABLE;
			memcpy(oriMac.octet, rg_db.hostPoliceList[host_idx].info.macAddr.octet, ETHER_ADDR_LEN);
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
			ASSERT_EQ(_rtk_rg_g3_l2cls_for_hostPoliceControl_del(&oriMac), RT_ERR_OK);//delete l2 CLS rule for host policing of original MAC address.
#endif
		}
#endif
	}

	memcpy(&rg_db.hostPoliceList[host_idx].info,pHostPoliceControl,sizeof(rtk_rg_hostPoliceControl_t));

#if defined(CONFIG_RG_G3_SERIES)

	if(oriMac_ifUpdatePolIdx)
	{
		_rtk_rg_g3_get_mac_hostPolicing_info(&oriMac, &oriMac_hp_info); // get ori mac host policing info (and add L2 CLSs (if need))
		DEBUG("hostPoliceList[%d] original MAC address is %02x:%02x:%02x:%02x:%02x:%02x, need to update its flow policer indexes for logging",
			host_idx, oriMac.octet[0], oriMac.octet[1], oriMac.octet[2], oriMac.octet[3], oriMac.octet[4], oriMac.octet[5]);
	}

	if(memcmp(rg_db.hostPoliceList[host_idx].info.macAddr.octet, zeroMac.octet, ETHER_ADDR_LEN))
	{
		newMac_ifUpdatePolIdx = ENABLE;
		memcpy(newMac.octet, rg_db.hostPoliceList[host_idx].info.macAddr.octet, ETHER_ADDR_LEN);
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
		ASSERT_EQ(_rtk_rg_g3_l2cls_for_hostPoliceControl_del(&newMac), RT_ERR_OK);//delete l2 CLS rule for host policing of new MAC address.
#endif
		_rtk_rg_g3_get_mac_hostPolicing_info(&newMac, &newMac_hp_info); // get new mac host policing info (and add L2 CLSs (if need))
		DEBUG("hostPoliceList[%d] new MAC address is %02x:%02x:%02x:%02x:%02x:%02x, need to update its flow policer indexes for logging",
			host_idx, newMac.octet[0], newMac.octet[1], newMac.octet[2], newMac.octet[3], newMac.octet[4], newMac.octet[5]);
	}
#endif

	for(i=0; i<HOST_POLICING_TABLE_SIZE; i++)
	{
		if(!memcmp(rg_db.hostPoliceList[i].info.macAddr.octet, zeroMac.octet, ETHER_ADDR_LEN))
			continue;
		if(rg_db.hostPoliceList[i].info.egressLimitCtrl==ENABLED)
		{
			memcpy(macEntry.mac.octet, rg_db.hostPoliceList[i].info.macAddr.octet, ETHER_ADDR_LEN);
			lut_idx=-1;
			if(rtk_rg_apollo_macEntry_find(&macEntry, &lut_idx)==RT_ERR_RG_OK)
			{
				DEBUG("Enable host_dmac_rateLimit of lut[%d] mac=%pM", lut_idx, macEntry.mac.octet);
				rg_db.lut[lut_idx].host_dmac_rateLimit = 1;
			}
		}
#if defined(CONFIG_RG_G3_SERIES)
		rtk_rg_apolloPro_hostPoliceLogging_get(&hpMibEntry, i); //store current hw counter to sw counter before changing policer Idx

		//update flow policer indexs for oriMac logging
		if(oriMac_ifUpdatePolIdx && (!memcmp(rg_db.hostPoliceList[i].info.macAddr.octet, oriMac.octet, ETHER_ADDR_LEN)))
		{
			rg_db.hostPoliceList[i].loggingRx_policerIdx = FAIL;
			rg_db.hostPoliceList[i].loggingTx_policerIdx = FAIL;
			if(oriMac_hp_info.logging_hostPolEntryIdx == i)
			{
				if(oriMac_hp_info.ingressRateLimit_en)
					rg_db.hostPoliceList[i].loggingRx_policerIdx = oriMac_hp_info.ingressRateLimit_mtrIdx + G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_RX;
				else
					rg_db.hostPoliceList[i].loggingRx_policerIdx = oriMac_hp_info.logging_hostPolEntryIdx + G3_FLOW_POLICER_IDXSHIFT_HPLOGRX;
				if(oriMac_hp_info.egressRateLimit_en)
					rg_db.hostPoliceList[i].loggingTx_policerIdx = oriMac_hp_info.egressRateLimit_mtrIdx + G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_TX;
				else
					rg_db.hostPoliceList[i].loggingTx_policerIdx = oriMac_hp_info.logging_hostPolEntryIdx + G3_FLOW_POLICER_IDXSHIFT_HPLOGTX;

				DEBUG("Update hostPoliceList[%d] (MAC: %02x:%02x:%02x:%02x:%02x:%02x) loggingRx_policerIdx = %d, loggingTx_policerIdx = %d",
					i, oriMac.octet[0], oriMac.octet[1], oriMac.octet[2], oriMac.octet[3], oriMac.octet[4], oriMac.octet[5],
					rg_db.hostPoliceList[i].loggingRx_policerIdx, rg_db.hostPoliceList[i].loggingTx_policerIdx);
			}
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
			//update L2 CLSs indexs
			rg_db.hostPoliceList[i].l2ClsIndex.idx_ingress = oriMac_hp_info.l2ClsIndex.idx_ingress;
			rg_db.hostPoliceList[i].l2ClsIndex.idx_egress = oriMac_hp_info.l2ClsIndex.idx_egress;
#endif
		}
		//update flow policer indexs for newMac logging
		if(newMac_ifUpdatePolIdx && (!memcmp(rg_db.hostPoliceList[i].info.macAddr.octet, newMac.octet, ETHER_ADDR_LEN)))
		{
			rg_db.hostPoliceList[i].loggingRx_policerIdx = FAIL;
			rg_db.hostPoliceList[i].loggingTx_policerIdx = FAIL;
			if(newMac_hp_info.logging_hostPolEntryIdx == i)
			{
				if(newMac_hp_info.ingressRateLimit_en)
					rg_db.hostPoliceList[i].loggingRx_policerIdx = newMac_hp_info.ingressRateLimit_mtrIdx + G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_RX;
				else
					rg_db.hostPoliceList[i].loggingRx_policerIdx = newMac_hp_info.logging_hostPolEntryIdx + G3_FLOW_POLICER_IDXSHIFT_HPLOGRX;
				if(newMac_hp_info.egressRateLimit_en)
					rg_db.hostPoliceList[i].loggingTx_policerIdx = newMac_hp_info.egressRateLimit_mtrIdx + G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_TX;
				else
					rg_db.hostPoliceList[i].loggingTx_policerIdx = newMac_hp_info.logging_hostPolEntryIdx + G3_FLOW_POLICER_IDXSHIFT_HPLOGTX;

				DEBUG("Update hostPoliceList[%d] (MAC: %02x:%02x:%02x:%02x:%02x:%02x) loggingRx_policerIdx = %d, loggingTx_policerIdx = %d",
					i, newMac.octet[0], newMac.octet[1], newMac.octet[2], newMac.octet[3], newMac.octet[4], newMac.octet[5],
					rg_db.hostPoliceList[i].loggingRx_policerIdx, rg_db.hostPoliceList[i].loggingRx_policerIdx);
			}
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
			//update L2 CLSs indexs
			rg_db.hostPoliceList[i].l2ClsIndex.idx_ingress = newMac_hp_info.l2ClsIndex.idx_ingress;
			rg_db.hostPoliceList[i].l2ClsIndex.idx_egress = newMac_hp_info.l2ClsIndex.idx_egress;
#endif
		}
#endif
	}

#if defined(CONFIG_RG_G3_SERIES)
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
	/*use L2 CLSs to achieve host policing, no need to fulsh flow.*/
#else
	/*G3 use flow to achieve host policing, thus flush flow after host policing setting changed*/
	if(oriMac_ifUpdatePolIdx)
	{
		memcpy(macEntry.mac.octet, oriMac.octet, ETHER_ADDR_LEN);
		if(rtk_rg_apollo_macEntry_find(&macEntry, &lut_idx)==RT_ERR_RG_OK)
		{
			DEBUG("Clear flow by hostPolControl[%d] original MAC %02x:%02x:%02x:%02x:%02x:%02x", host_idx, oriMac.octet[0], oriMac.octet[1], oriMac.octet[2], oriMac.octet[3], oriMac.octet[4], oriMac.octet[5]);
			assert_ok(_rtk_rg_flow_del_by_l2Idx(lut_idx));
		}
	}

	if(newMac_ifUpdatePolIdx)
	{
		memcpy(macEntry.mac.octet, newMac.octet, ETHER_ADDR_LEN);
		if(rtk_rg_apollo_macEntry_find(&macEntry, &lut_idx)==RT_ERR_RG_OK)
		{
			DEBUG("Clear flow by hostPolControl[%d] new MAC %02x:%02x:%02x:%02x:%02x:%02x", host_idx, newMac.octet[0], newMac.octet[1], newMac.octet[2], newMac.octet[3], newMac.octet[4], newMac.octet[5]);
			assert_ok(_rtk_rg_flow_del_by_l2Idx(lut_idx));
		}
	}
#endif
#endif

	// check
	memset(&trapAck, 0, sizeof(rtk_rg_trapSpecificLengthAck_t));
	trapSpecificLengthAck_reservedAclAdd = FALSE;
	for(i=0; i<HOST_POLICING_TABLE_SIZE; i++)
	{
		if(!memcmp(rg_db.hostPoliceList[i].info.macAddr.octet, zeroMac.octet, ETHER_ADDR_LEN))
			continue;
		for(j=i; j<HOST_POLICING_TABLE_SIZE; j++)
		{
			if(!memcmp(rg_db.hostPoliceList[j].info.macAddr.octet, zeroMac.octet, ETHER_ADDR_LEN))
				continue;
			if(((rg_db.hostPoliceList[i].info.ingressLimitCtrl==ENABLED && rg_db.hostPoliceList[j].info.egressLimitCtrl==ENABLED) || (rg_db.hostPoliceList[i].info.egressLimitCtrl==ENABLED && rg_db.hostPoliceList[j].info.ingressLimitCtrl==ENABLED))
				&& !memcmp(rg_db.hostPoliceList[i].info.macAddr.octet, rg_db.hostPoliceList[j].info.macAddr.octet, ETHER_ADDR_LEN))
			{
				trapSpecificLengthAck_reservedAclAdd = TRUE;
				break;
			}
		}
		if(trapSpecificLengthAck_reservedAclAdd) break;
	}
	if(rg_db.systemGlobal.trapSpecificLengthAck.portMask)
		_rtk_rg_trap_specific_length_ack_reservedAcl_set(rg_db.systemGlobal.trapSpecificLengthAck, trapSpecificLengthAck_reservedAclAdd);
	else
		_rtk_rg_trap_specific_length_ack_reservedAcl_set(trapAck, trapSpecificLengthAck_reservedAclAdd);

	//Check if need to be added into count list
	if(!list_empty(&rg_db.hostPoliceCountListHead))
	{
		list_for_each_entry_safe(pHostEntry,pNextEntry,&rg_db.hostPoliceCountListHead,host_list)
		{
			if(pHostEntry->idx==host_idx)
			{
				//the entry is in the count list, delete it
				list_del_init(&pHostEntry->host_list);
				if(pHostEntry->lut_idx_learned)
				{
					rg_db.lut[pHostEntry->lut_idx].host_idx_valid=0;
					pHostEntry->lut_idx_learned=0;
				}
			}
			else
			{
				if((pHostEntry->idx < host_idx) && (!memcmp(pHostPoliceControl->macAddr.octet,pHostEntry->info.macAddr.octet,ETHER_ADDR_LEN)) && (pHostEntry->info.mibCountCtrl))
					return (RT_ERR_RG_OK); //this mac is in the count list already, and its entry index is less than host_idx=>just return.
				if((pHostEntry->idx > host_idx) && (!memcmp(pHostPoliceControl->macAddr.octet,pHostEntry->info.macAddr.octet,ETHER_ADDR_LEN)) && (pHostEntry->info.mibCountCtrl))
				{
					//this mac is in the count list already, but its entry index is large than host_idx=> delete it
					list_del_init(&pHostEntry->host_list);
					if(pHostEntry->lut_idx_learned){
						rg_db.lut[pHostEntry->lut_idx].host_idx_valid=0;
						pHostEntry->lut_idx_learned=0;
					}
				}
			}
		}
	}
	if(pHostPoliceControl->mibCountCtrl){
		//add to count list
		memcpy(macEntry.mac.octet,pHostPoliceControl->macAddr.octet,ETHER_ADDR_LEN);
		lut_idx=-1;
		if(rtk_rg_apollo_macEntry_find(&macEntry,&lut_idx)==RT_ERR_RG_OK){
			rg_db.hostPoliceList[host_idx].lut_idx=lut_idx;
			rg_db.hostPoliceList[host_idx].lut_idx_learned=1;
			rg_db.lut[rg_db.hostPoliceList[host_idx].lut_idx].host_idx=host_idx;
			rg_db.lut[rg_db.hostPoliceList[host_idx].lut_idx].host_idx_valid=1;
		}
		//Delete from head list
		list_del_init(&rg_db.hostPoliceList[host_idx].host_list);
		list_add_tail(&rg_db.hostPoliceList[host_idx].host_list,&rg_db.hostPoliceCountListHead);
	}

	return (RT_ERR_RG_OK);
}

rtk_rg_err_code_t rtk_rg_apolloPro_hostPoliceControl_get(rtk_rg_hostPoliceControl_t *pHostPoliceControl, int host_idx)
{
	//Check parameter
	if(pHostPoliceControl==NULL)RETURN_ERR(RT_ERR_RG_NULL_POINTER);
	if(host_idx<0 || host_idx>=HOST_POLICING_TABLE_SIZE)RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);

	memcpy(pHostPoliceControl,&rg_db.hostPoliceList[host_idx].info,sizeof(rtk_rg_hostPoliceControl_t));

	return (RT_ERR_RG_OK);
}

rtk_rg_err_code_t rtk_rg_apolloPro_hostPoliceLogging_get(rtk_rg_hostPoliceLogging_t *pHostMibCnt, int host_idx)
{
	//Check parameter
	if(pHostMibCnt==NULL)RETURN_ERR(RT_ERR_RG_NULL_POINTER);
	if(host_idx<0 || host_idx>=HOST_POLICING_TABLE_SIZE)RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	ASSERT_EQ(rtk_stat_hostCnt_get(host_idx,STAT_HOST_RX_OCTETS,&pHostMibCnt->rx_count),RT_ERR_OK);
	ASSERT_EQ(rtk_stat_hostCnt_get(host_idx,STAT_HOST_TX_OCTETS,&pHostMibCnt->tx_count),RT_ERR_OK);
#elif defined(CONFIG_RG_G3_SERIES)
	{
		memset(pHostMibCnt, 0, sizeof(rtk_rg_hostPoliceLogging_t));
		if((rg_db.hostPoliceList[host_idx].info.mibCountCtrl) && (rg_db.hostPoliceList[host_idx].loggingRx_policerIdx != FAIL || rg_db.hostPoliceList[host_idx].loggingTx_policerIdx != FAIL))
		{
			if(rg_db.hostPoliceList[host_idx].loggingRx_policerIdx == FAIL)
				WARNING("hostPoliceList[host_idx]: loggingRx_policerIdx is FAIL and loggingTx_policerIdx is NOT FAIL, please check");
			else if(rg_db.hostPoliceList[host_idx].loggingTx_policerIdx == FAIL)
				WARNING("hostPoliceList[host_idx]: loggingRx_policerIdx is NOT FAIL and loggingTx_policerIdx is FAIL, please check");
			else
			{
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
				/*use L2 CLS to acheive host policing*/
				aal_l2_te_pm_policer_t l2_pm_data;
				ASSERT_EQ(aal_l2_te_pm_policer_flow_get(G3_DEF_DEVID, rg_db.hostPoliceList[host_idx].loggingRx_policerIdx, &l2_pm_data), CA_E_OK);
   				rg_db.hostPoliceList[host_idx].count.rx_count += (l2_pm_data.green_bytes + l2_pm_data.yellow_bytes) - (4 * (l2_pm_data.green_pkt+ l2_pm_data.yellow_pkt)/*fake vlan*/); // read clear, store counter to SW

				ASSERT_EQ(aal_l2_te_pm_policer_flow_get(G3_DEF_DEVID, rg_db.hostPoliceList[host_idx].loggingTx_policerIdx, &l2_pm_data), CA_E_OK);
   				rg_db.hostPoliceList[host_idx].count.tx_count += (l2_pm_data.green_bytes + l2_pm_data.yellow_bytes); // read clear, store counter to SW
#else
				/*use L3 hash to acheive host policing*/
				aal_l3_te_pm_policer_t l3_pm_data;
				ASSERT_EQ(aal_l3_te_pm_policer_flow_get(G3_DEF_DEVID, rg_db.hostPoliceList[host_idx].loggingRx_policerIdx, &l3_pm_data), CA_E_OK);
   				rg_db.hostPoliceList[host_idx].count.rx_count += (l3_pm_data.green_bytes + l3_pm_data.yellow_bytes); // read clear, store counter to SW

				ASSERT_EQ(aal_l3_te_pm_policer_flow_get(G3_DEF_DEVID, rg_db.hostPoliceList[host_idx].loggingTx_policerIdx, &l3_pm_data), CA_E_OK);
   				rg_db.hostPoliceList[host_idx].count.tx_count += (l3_pm_data.green_bytes + l3_pm_data.yellow_bytes); // read clear, store counter to SW
#endif
			}
		}
	}

#endif
	pHostMibCnt->rx_count+=rg_db.hostPoliceList[host_idx].count.rx_count;
	pHostMibCnt->tx_count+=rg_db.hostPoliceList[host_idx].count.tx_count;

	return (RT_ERR_RG_OK);
}

rtk_rg_err_code_t rtk_rg_apolloPro_hostPoliceLogging_del(int host_idx)
{
	//Check parameter
	if(host_idx<0 || host_idx>=HOST_POLICING_TABLE_SIZE)RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	ASSERT_EQ(rtk_stat_hostCnt_reset(host_idx),RT_ERR_OK);
#elif defined(CONFIG_RG_G3_SERIES)
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
	{	//clear hw counter
		aal_l2_te_pm_policer_t l2_pm_data;
		if(rg_db.hostPoliceList[host_idx].loggingRx_policerIdx != FAIL)
			ASSERT_EQ(aal_l2_te_pm_policer_flow_get(G3_DEF_DEVID, rg_db.hostPoliceList[host_idx].loggingRx_policerIdx, &l2_pm_data), CA_E_OK);//read clear
		if(rg_db.hostPoliceList[host_idx].loggingTx_policerIdx != FAIL)
			ASSERT_EQ(aal_l2_te_pm_policer_flow_get(G3_DEF_DEVID, rg_db.hostPoliceList[host_idx].loggingTx_policerIdx, &l2_pm_data), CA_E_OK);//read clear
	}
#else
	{	//clear hw counter
		aal_l3_te_pm_policer_t l3_pm_data;
		if(rg_db.hostPoliceList[host_idx].loggingRx_policerIdx != FAIL)
			ASSERT_EQ(aal_l3_te_pm_policer_flow_get(G3_DEF_DEVID, rg_db.hostPoliceList[host_idx].loggingRx_policerIdx, &l3_pm_data), CA_E_OK);//read clear
		if(rg_db.hostPoliceList[host_idx].loggingTx_policerIdx != FAIL)
			ASSERT_EQ(aal_l3_te_pm_policer_flow_get(G3_DEF_DEVID, rg_db.hostPoliceList[host_idx].loggingTx_policerIdx, &l3_pm_data), CA_E_OK);//read clear

	}
#endif
#endif
	bzero(&rg_db.hostPoliceList[host_idx].count,sizeof(rtk_rg_hostPoliceLogging_t));

	return (RT_ERR_RG_OK);
}

rtk_rg_err_code_t rtk_rg_apolloPro_macEntry_add(rtk_rg_macEntry_t *macEntry, int *entry_idx)
{
	int ret, i;
	rtk_mac_t zeroMac;

	ret=rtk_rg_apollo_macEntry_add(macEntry,entry_idx);

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

		memset(&zeroMac, 0, sizeof(rtk_mac_t));
		for(i=0; i<HOST_POLICING_TABLE_SIZE; i++)
		{
			if(!memcmp(rg_db.hostPoliceList[i].info.macAddr.octet, zeroMac.octet, ETHER_ADDR_LEN))
				continue;
			if(rg_db.hostPoliceList[i].info.egressLimitCtrl==ENABLED
				&& !memcmp(macEntry->mac.octet, rg_db.hostPoliceList[i].info.macAddr.octet, ETHER_ADDR_LEN))
			{
				rg_db.lut[*entry_idx].host_dmac_rateLimit = 1;
				break;
			}
		}
	}

	return ret;
}

rtk_rg_err_code_t rtk_rg_apolloPro_flowMibCounter_get(int index, rtk_rg_table_flowmib_t *pCounter)
{

	if(pCounter==NULL)RETURN_ERR(RT_ERR_RG_NULL_POINTER);
	if(((index >= 0) && (index < MAX_FLOWMIB_TABLE_SIZE)) || ((index >= PURE_SW_FLOWMIB_IDX_OFFSET) && (index < PURE_SW_FLOWMIB_IDX_OFFSET+PURE_SW_FLOWMIB_TABLE_SIZE)))
		ASSERT_EQ(RTK_RG_ASIC_FLOWMIB_GET(index, pCounter), RT_ERR_RG_OK); //hw mib OR pure SW mib

	else
		RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);

	return (RT_ERR_RG_OK);
}

rtk_rg_err_code_t rtk_rg_apolloPro_flowMibCounter_reset(int index)
{
	if(((index >= 0) && (index < MAX_FLOWMIB_TABLE_SIZE)) || ((index >= PURE_SW_FLOWMIB_IDX_OFFSET) && (index < PURE_SW_FLOWMIB_IDX_OFFSET+PURE_SW_FLOWMIB_TABLE_SIZE)))
		ASSERT_EQ(RTK_RG_ASIC_FLOWMIB_RESET(index), RT_ERR_RG_OK);
	else
		RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);

	return (RT_ERR_RG_OK);
}


//#####################################internal function declaration###########

int _rtk_rg_hardwareArpTableLookUp(unsigned short routingIdx, ipaddr_t ipAddr, rtk_rg_arp_linkList_t **phardwareArpEntry, int resetIdleTime)
{

	return (RT_ERR_RG_OK);
}

int _rtk_rg_hardwareArpTableAdd(unsigned short routingIdx, ipaddr_t ipv4Addr, int l2Idx, int staticEntry, uint16 *hwArpIdx)
{

	return (RT_ERR_RG_OK);
}

int _rtk_rg_hardwareArpTableDel(rtk_rg_arp_linkList_t *pDelArpEntry)
{

	return (RT_ERR_RG_OK);
}

int32 _rtk_rg_deleteHwARP(int intfIdx, int routingIdx)
{

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
				(memcmp(&(rg_db.lut[search_index].rtk_lut.entry.ipmcEntry.dip6.ipv6_addr[0]),&(gipv6->ipv6_addr[0]),sizeof(rtk_ipv6_addr_t))==0) )
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

#if 0 // for compiler
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
#endif

//mib counter and cf decision
void _rtk_rg_IntfIdxDecisionForCF(rtk_rg_pktHdr_t *pPktHdr)
{
	// FIXME
}

rtk_rg_entryGetReturn_t _rtk_rg_extPMaskEntry_find(uint32 extPMask)
{
	uint8 idx = 0, freeIdx = 0, hit = FALSE;

	if(extPMask == 0x0)
	{
		hit = TRUE;
		idx = 0;
	}
	else
	{
		// reserve entry[0] for no-extension port used, so search the same one or empty one from entry[1].
		for(idx = EXTPTBL_RSVED_ENTRY ; idx < MAX_EXTPORT_TABLE_SIZE ; idx++)
		{
			// find the first empty entry
			if((rg_db.extPortTbl[idx].extPortRefCount==0) && (freeIdx==0))
			{
				freeIdx = idx;
			}
			// find the same setting
			if(rg_db.extPortTbl[idx].extPortEnt.extpmask == extPMask)
			{
				hit = TRUE;
				break;
			}
		}

		if(idx == MAX_EXTPORT_TABLE_SIZE && (hit==FALSE) && (freeIdx==0))
			return RG_RET_ENTRY_NOT_GET; //RT_ERR_RG_ENTRY_FULL

		// if hit, use the found index(idx). if not, use the empty entry(freeIdx).
		if(!hit)
			idx = freeIdx;
	}

	//DEBUG("find extension port mask entry[%d], freeidx = %d", idx, freeIdx);

	return idx;
}

rtk_rg_entryGetReturn_t _rtk_rg_extPMaskEntryIdx_get(uint32 extPMask)
{
	int32 hitIdx = RG_RET_ENTRY_NOT_GET, idx;

	if(extPMask == 0x0)
	{
		hitIdx = 0;
	}
	else
	{
		// reserve entry[0] for no-extension port used, so search the same one or empty one from entry[1].
		for(idx = EXTPTBL_RSVED_ENTRY ; idx < MAX_EXTPORT_TABLE_SIZE ; idx++)
		{
			// find the same setting
			if(rg_db.extPortTbl[idx].extPortEnt.extpmask == extPMask)
			{
				hitIdx = idx;
				break;
			}
		}
	}

	return hitIdx;
}

rtk_rg_entryGetReturn_t _rtk_rg_indirectMacEntry_find(int l2Idx)
{
	uint8 hit = FALSE;
	uint16 idx = 0, freeIdx = 0;

	for(idx = 0 ; idx < MAX_INDMAC_TABLE_SIZE ; idx++)
	{
		// find the first empty entry
		if((rg_db.indMacTbl[idx].indMacRefCount == 0) && (freeIdx==0))
		{
			freeIdx = idx;
		}
		// find the same setting
		if(rg_db.indMacTbl[idx].indMacEnt.l2_idx == l2Idx)
		{
			hit = TRUE;
			break;
		}
	}

	if(idx == MAX_INDMAC_TABLE_SIZE && (hit==FALSE) && (freeIdx==0))
		return RG_RET_ENTRY_NOT_GET; //RT_ERR_RG_ENTRY_FULL

	// if hit, use the found index(idx). if not, use the empty entry(freeIdx).
	if(!hit)
		idx = freeIdx;

	return idx;
}

rtk_rg_err_code_t _rtk_rg_netifAllowIgrPMask_set(int netifIdx, uint32 igrPMask, uint32 igrExtPMask)
{
	rtk_rg_err_code_t ret = RT_ERR_RG_OK;
#if !defined(CONFIG_RG_G3_SERIES)
	// configure pppoe action & sessionid to netif table
	rtk_rg_asic_netif_entry_t netifEnt;
	bzero(&netifEnt, sizeof(netifEnt));
	ret = rtk_rg_asic_netifTable_get(netifIdx, &netifEnt);

	if(!netifEnt.valid) return RT_ERR_RG_ENTRY_NOT_EXIST;

	netifEnt.allow_ingress_portmask.bits[0] = igrPMask;
	netifEnt.allow_ingress_ext_portmask.bits[0] = igrExtPMask;

	ret = rtk_rg_asic_netifTable_add(netifIdx, &netifEnt);

	TABLE("[netif] assign netif[%d] allowed igr pmask: 0x%x, extpmak: 0x%x", netifIdx, igrPMask, igrExtPMask);
#endif //!defined(CONFIG_RG_G3_SERIES)
	return ret;
}

rtk_rg_err_code_t _rtk_rg_netifPPPoESession_set(int netifIdx, rtk_rg_asic_netifpppoeAct_t action, uint32 sessionId)
{
	rtk_rg_err_code_t ret = RT_ERR_RG_OK;
#if !defined(CONFIG_RG_G3_SERIES)
	// configure pppoe action & sessionid to netif table
	rtk_rg_asic_netif_entry_t netifEnt;
	bzero(&netifEnt, sizeof(netifEnt));
	ret = rtk_rg_asic_netifTable_get(netifIdx, &netifEnt);

	if(!netifEnt.valid) return RT_ERR_RG_ENTRY_NOT_EXIST;

	netifEnt.out_pppoe_act = action;

	if(action == FB_NETIFPPPOE_ACT_ADD || action == FB_NETIFPPPOE_ACT_MODIFY)
		netifEnt.out_pppoe_sid = sessionId;
	else
		netifEnt.out_pppoe_sid = 0;

	ret = rtk_rg_asic_netifTable_add(netifIdx, &netifEnt);
#endif //!defined(CONFIG_RG_G3_SERIES)
	return ret;

}

rtk_rg_err_code_t _rtk_rg_l34WanAccessLimit_tableRst(void)
{
	// reset allow list
#if !defined(CONFIG_RG_G3_SERIES)
	rtk_rg_asic_table_reset(FB_RST_WAL_TYPE);
#endif
	bzero(&rg_db.wanAccessLimit[0], sizeof(rg_db.wanAccessLimit));

	return RT_ERR_RG_OK;
}

rtk_rg_err_code_t _rtk_rg_l34WanAccessLimit_delL2Idx(uint32 l2Idx)
{
	int32 i = 0;

	// if wan access limit feature is disabled, there is no necessary to search entry.
	if(rg_db.systemGlobal.activeLimitFunction == RG_ACCESSWAN_TYPE_UNLIMIT)
		return RT_ERR_RG_OK;

	// search allowed list
	for(i = 0; i < MAX_WANACCESSLIMIT_TABLE_SIZE; i++)
	{
		if(rg_db.wanAccessLimit[i].wanAllowEnt.valid && (rg_db.wanAccessLimit[i].wanAllowEnt.sa_idx==l2Idx))
		{
			RTK_RG_ASIC_WANACCESSLIMITTABLE_DEL(i);
			break;
		}
	}

 	return RT_ERR_RG_OK;
}

/*
 * Return:
 *      RG_RET_SUCCESS	- hold permission to access l34 wan or still have free entry to apply.
 *      RG_RET_FAIL             	- without permission and permitted list is full.
*/
rtk_rg_successFailReturn_t _rtk_rg_l34WanAccessLimit_permissionCheck(rtk_rg_pktHdr_t *pPktHdr, int32 *availableIdx)
{
	rtk_rg_port_idx_t spa = pPktHdr->ingressPort;
	int32 i = 0, firstInvalidEntIdx = -1;
	int32 category = 0;

	// no matter what AccessWanType is, check if sa belong to wan access limit allowed list
	// if not, check if there is enough limit count to apply

	// search allowed list
	for(i = 0; i < MAX_WANACCESSLIMIT_TABLE_SIZE; i++)
	{
		if(rg_db.wanAccessLimit[i].wanAllowEnt.valid)
		{
			if(rg_db.wanAccessLimit[i].wanAllowEnt.sa_idx==pPktHdr->smacL2Idx){
				DEBUG("Wan Access Limit SA l2Idx[%d] hit entry[%d]", pPktHdr->smacL2Idx, i);
				return (RG_RET_SUCCESS);
			}
		}
		else if(firstInvalidEntIdx == -1)
			firstInvalidEntIdx = i;
	}

	// check if over limitation
	if(rg_db.systemGlobal.accessWanLimitPortBased[spa]>=0 &&
		rg_db.systemGlobal.accessWanLimitPortBased[spa]<=atomic_read(&rg_db.systemGlobal.accessWanLimitPortBasedCount[spa]))
	{
		MACLN("Port Limit Action: DROP..");
		return (RG_RET_FAIL);
	}
	if(rg_db.systemGlobal.activeLimitFunction==RG_ACCESSWAN_TYPE_PORTMASK)
	{
		if(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask&(0x1<<(spa)) &&
			rg_db.systemGlobal.accessWanLimitPortMask<=atomic_read(&rg_db.systemGlobal.accessWanLimitPortMaskCount))
		{
			MACLN("Portmask Limit Action: DROP..");
			return (RG_RET_FAIL);
		}
	}else if(rg_db.systemGlobal.activeLimitFunction==RG_ACCESSWAN_TYPE_CATEGORY)
	{
		category = rg_db.lut[pPktHdr->smacL2Idx].category;

		if(rg_db.systemGlobal.accessWanLimitCategory[category]>=0 &&
			rg_db.systemGlobal.accessWanLimitCategory[category]<=atomic_read(&rg_db.systemGlobal.accessWanLimitCategoryCount[category]))
		{
			MACLN("Category Limit Action: DROP..");
			return RG_RET_FAIL;
		}
	}

	// SA didn't over limitation yet, prepare to add smacL2Idx to wanAccessLimit list
	*availableIdx = firstInvalidEntIdx;

	return (RG_RET_SUCCESS);
}


rtk_rg_successFailReturn_t _rtk_rg_l34WanAccessLimit_permissionApply(rtk_rg_pktHdr_t *pPktHdr, int32 availableIdx)
{
	rtk_rg_err_code_t ret = RT_ERR_RG_OK;
	rtk_rg_asic_wanAccessLimit_entry_t walEnt;

	TRACE("Wan Access Limit apply l2Idx[%d] to allowed list[%d]", pPktHdr->smacL2Idx, availableIdx);

	walEnt.valid = TRUE;
	walEnt.sa_idx = pPktHdr->smacL2Idx;
	if((ret = RTK_RG_ASIC_WANACCESSLIMITTABLE_ADD(availableIdx, &walEnt))!= RT_ERR_RG_OK)
		return ret;
	atomic_inc(&rg_db.systemGlobal.accessWanLimitPortBasedCount[pPktHdr->ingressPort]);
	if(rg_db.systemGlobal.activeLimitFunction==RG_ACCESSWAN_TYPE_PORTMASK)
	{
		atomic_inc(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
	}else if(rg_db.systemGlobal.activeLimitFunction==RG_ACCESSWAN_TYPE_CATEGORY)
	{
		atomic_inc(&rg_db.systemGlobal.accessWanLimitCategoryCount[(unsigned int)rg_db.lut[pPktHdr->smacL2Idx].category]);
	}

	return (ret);
}


/*
 * Return:
 *      RT_ERR_RG_OK                 - keep forwarding
 *      RT_ERR_FAILED             	- drop
*/
rtk_rg_err_code_t _rtk_rg_l34WanAccessLimit(rtk_rg_pktHdr_t *pPktHdr)
{
	#define NONEEDEDTOADD -1
	rtk_rg_successFailReturn_t ret = RG_RET_SUCCESS;
	int32 availableIdx = NONEEDEDTOADD;

	if(rg_db.systemGlobal.activeLimitFunction==RG_ACCESSWAN_TYPE_UNLIMIT)
	{
		DEBUG("Wan Access Limit is turn off.");
		return (RT_ERR_RG_OK);
	}
	else if(rg_db.systemGlobal.activeLimitField==RG_ACCESSWAN_LIMIT_BY_SIP)
	{
		//20170301LUKE: check mac limit only when field is source mac.
		DEBUG("Wan Access Limit is By SIP.");
		return (RT_ERR_RG_OK);
	}
	else if((rg_db.systemGlobal.wanAccessLimit_pmask.portmask & (1<<pPktHdr->ingressPort))==0x0)
	{
		DEBUG("Wan Access Limit pmask 0x%x didn't include spa %d, skip.", rg_db.systemGlobal.wanAccessLimit_pmask.portmask, pPktHdr->ingressPort);
		return (RT_ERR_RG_OK);
	}
	else
	{
		if((ret = _rtk_rg_l34WanAccessLimit_permissionCheck(pPktHdr, &availableIdx)) != RG_RET_SUCCESS)
		{
			// No Permission && Action is Drop	: drop and do nothing
			// No Permission && Action is PermitL2: do arp request to check hosts are still alive.
			switch(rg_db.systemGlobal.activeLimitFunction)
			{
				case RG_ACCESSWAN_TYPE_PORTMASK:
					if(rg_db.systemGlobal.accessWanLimitPortMaskAction == SA_LEARN_EXCEED_ACTION_PERMIT_L2)
					{
						//send by all ARP and ND of all port in port mask
					#ifdef CONFIG_MASTER_WLAN0_ENABLE
						_rtk_rg_lutReachLimit_init(RG_ACCESSWAN_TYPE_PORTMASK, _rtk_rg_lutReachLimit_portmask, (unsigned long)(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask|(rg_db.systemGlobal.accessWanLimitPortMask_wlan0member<<16)));
					#else
						_rtk_rg_lutReachLimit_init(RG_ACCESSWAN_TYPE_PORTMASK, _rtk_rg_lutReachLimit_portmask, (unsigned long)rg_db.systemGlobal.accessWanLimitPortMask_member.portmask);
					#endif
					}
					break;
				case RG_ACCESSWAN_TYPE_CATEGORY:
					if(rg_db.systemGlobal.accessWanLimitCategoryAction[(unsigned long)rg_db.lut[pPktHdr->smacL2Idx].category] == SA_LEARN_EXCEED_ACTION_PERMIT_L2)
					{
						//send by all ARP and ND of same category
						_rtk_rg_lutReachLimit_init(RG_ACCESSWAN_TYPE_CATEGORY, _rtk_rg_lutReachLimit_category, (unsigned long)rg_db.lut[pPktHdr->smacL2Idx].category);
					}
					break;
				default:
					break;
			}

			return (RT_ERR_RG_FAILED);
		}

		if(availableIdx != NONEEDEDTOADD)
			ret = _rtk_rg_l34WanAccessLimit_permissionApply(pPktHdr, availableIdx);
	}


	return (RT_ERR_RG_OK);
}




//support WAN to LAN only,egress intf only check Lan intf
int32 _rtk_rg_decideEgressMcastNetIfIdx(uint32 egressPmsk)
{
	int32 ifidx=FAILED;
	int tmpNetifId;
	uint32 egressPortmask;

#if defined(CONFIG_MASTER_WLAN0_ENABLE) && defined(CONFIG_RG_FLOW_NEW_WIFI_MODE)
	if(egressPmsk & (RTK_RG_ALL_REAL_MASTER_EXT_PORTMASK | RTK_RG_ALL_REAL_SLAVE_EXT_PORTMASK))
	{
		egressPortmask=(1<<RTK_RG_EXT_PORT0);
	}
	else
#endif
	{
		egressPortmask=egressPmsk;
	}

	//search Lan first
	for(tmpNetifId=0;tmpNetifId<MAX_NETIF_SW_TABLE_SIZE;tmpNetifId++)
	{
		if(!(rg_db.systemGlobal.interfaceInfo[tmpNetifId].valid)) continue;
		if(rg_db.systemGlobal.interfaceInfo[tmpNetifId].storedInfo.is_wan) continue;

		//egressPmsk all to same Lan intf
		if((egressPortmask & rg_db.systemGlobal.interfaceInfo[tmpNetifId].p_lanIntfConf->port_mask.portmask)==egressPortmask)
		{
			IGMP("Hit egress Lan idx=%d",tmpNetifId);
			ifidx=tmpNetifId;
			break;
		}
	}


#if !defined(CONFIG_RG_G3_SERIES)
	//if non-hit any lan ,search wan
	if(ifidx==FAILED )
	{
		for(tmpNetifId=0;tmpNetifId<MAX_NETIF_SW_TABLE_SIZE;tmpNetifId++)
		{
			if(!(rg_db.systemGlobal.interfaceInfo[tmpNetifId].valid)) continue;
			if(!rg_db.systemGlobal.interfaceInfo[tmpNetifId].storedInfo.is_wan) continue;

			if(egressPortmask & (1 << rg_db.systemGlobal.interfaceInfo[tmpNetifId].storedInfo.wan_intf.wan_intf_conf.wan_port_idx))
			{
				IGMP("Hit egress Wan idx=%d",tmpNetifId);
				ifidx=tmpNetifId;
				break;
			}
		}
	}
#endif

	if(ifidx==FAILED)
	{
		ifidx=DEFAULT_L2_WANIF_IDX;
		IGMP("Warning Can't decide egress interface We used DEFAULT_L2_WANIF_IDX=%d",ifidx);
	}


	return ifidx;

}


int32 _rtk_rg_decideIngressMcastNetIfIdx(uint32 spa)
{
	int32 ifidx=FAILED;
	uint16 netIf_cvid;
	int tmpNetifId;
	int hitInterface=0;


	//search PPPoE wan and bridge wan first
	for(tmpNetifId=0;tmpNetifId<MAX_NETIF_SW_TABLE_SIZE;tmpNetifId++)
	{
		if(!(rg_db.systemGlobal.interfaceInfo[tmpNetifId].valid)) continue;
		if(!rg_db.systemGlobal.interfaceInfo[tmpNetifId].storedInfo.is_wan) continue;
		if(rg_db.systemGlobal.interfaceInfo[tmpNetifId].storedInfo.wan_intf.wan_intf_conf.wan_port_idx != spa) continue;

		//from  PPPoE Wan Using Uc_Interface
		if(rg_db.pktHdr->tagif & PPPOE_TAGIF )
		{
			//hit pppoe interface
			if((rg_db.pktHdr->isGatewayPacket) && rg_db.pktHdr->sessionId==rg_db.pppoe[rg_db.systemGlobal.interfaceInfo[tmpNetifId].storedInfo.wan_intf.pppoe_idx].rtk_pppoe.sessionID)
			{
				hitInterface=1;
				ifidx=tmpNetifId;
				IGMP("hit ingress pppoe wan ifidx=%d",ifidx);
				break;
			}
		}
		else if( !(rg_db.pktHdr->tagif & CVLAN_TAGIF) && (rg_db.systemGlobal.interfaceInfo[tmpNetifId].storedInfo.wan_intf.wan_intf_conf.wan_type ==RTK_RG_BRIDGE) &&  (rg_db.systemGlobal.interfaceInfo[tmpNetifId].storedInfo.wan_intf.wan_intf_conf.egress_vlan_tag_on==0) )
		{
			//1 FIXME:if unctag hit any acl to change vlan , ingress interface may not for BRIDGE_WAN
			//untag and bridge wan
			hitInterface=1;
			ifidx = tmpNetifId;
			IGMP("Hit ingress BRIDGE_WAN ifidx=%d",ifidx);
			break;
		}

	}

	//if unmatch using vlan to find ingress wan interface
	if(hitInterface ==0)
	{
		for(tmpNetifId=0;tmpNetifId<MAX_NETIF_SW_TABLE_SIZE;tmpNetifId++)
		{
			if(!(rg_db.systemGlobal.interfaceInfo[tmpNetifId].valid)) continue;
			if(!rg_db.systemGlobal.interfaceInfo[tmpNetifId].storedInfo.is_wan) continue;
			if(rg_db.systemGlobal.interfaceInfo[tmpNetifId].storedInfo.wan_intf.wan_intf_conf.wan_port_idx != spa) continue;
			//find a vlan match interface
			netIf_cvid = rg_db.systemGlobal.interfaceInfo[tmpNetifId].storedInfo.wan_intf.wan_intf_conf.egress_vlan_id ;
			if(rg_db.pktHdr->ingressDecideVlanID==netIf_cvid)
			{
				hitInterface=1;
				ifidx = tmpNetifId;
				IGMP("Hit ingress WAN ifidx=%d",ifidx);
				break;
			}

		}
	}


	if(hitInterface ==0 )
	{
		int vlanMatch=FAILED;
		for(tmpNetifId=0;tmpNetifId<MAX_NETIF_SW_TABLE_SIZE;tmpNetifId++)
		{
			if(!(rg_db.systemGlobal.interfaceInfo[tmpNetifId].valid)) continue;
			if(rg_db.systemGlobal.interfaceInfo[tmpNetifId].storedInfo.is_wan) continue;
			if(!(rg_db.systemGlobal.interfaceInfo[tmpNetifId].storedInfo.lan_intf.port_mask.portmask & (1<<spa))) continue;

			netIf_cvid = rg_db.systemGlobal.interfaceInfo[tmpNetifId].storedInfo.lan_intf.intf_vlan_id;
			if( (rg_db.pktHdr->tagif & CVLAN_TAGIF)==0  &&  (rg_db.systemGlobal.interfaceInfo[tmpNetifId].storedInfo.lan_intf.untag_mask.portmask& (1<<spa)))
			{
				hitInterface=1;
				ifidx = tmpNetifId;

			}
			if(rg_db.pktHdr->ingressDecideVlanID==netIf_cvid)
			{
				hitInterface=1;
				vlanMatch = tmpNetifId;
				//IGMP("Hit ingress LAN ifidx=%d",ifidx);
			}
		}
		if(vlanMatch!=FAILED)
			ifidx=vlanMatch;
		IGMP("Hit ingress LAN ifidx=%d",ifidx);

	}

	if(ifidx==FAILED)
	{
		ifidx=DEFAULT_L2_WANIF_IDX;
		IGMP("Warning Can't decide ingress interface We used DEFAULT_L2_WANIF_IDX=%d",ifidx);
	}
	return ifidx;
}


rtk_rg_lookupIdxReturn_t _rtk_rg_hwMacLookup(u8 *pMac, int vlanId)
{
	int l2Idx,search_index;
	int count=0;

	if(rg_db.vlan[vlanId].fidMode==VLAN_FID_IVL)
	{
		l2Idx=_rtk_rg_hash_mac_vid_efid(pMac,vlanId,0);		//FIXME;current efid is always 0
	}
	else
	{
		l2Idx=_rtk_rg_hash_mac_fid_efid(pMac,rg_db.vlan[vlanId].fid,0);		//FIXME;current efid is always 0
	}

	do
	{
		search_index=(l2Idx<<MAX_LUT_HASH_WAY_SHIFT)+count;

		DEBUG("search_index =%d Dmac=%02x%02x:%02x%02x:%02x%02x vid=%d",search_index,pMac[0],pMac[1],pMac[2],pMac[3],pMac[4],pMac[5],vlanId);

		if(rg_db.lut[search_index].valid &&
			(!memcmp(rg_db.lut[search_index].rtk_lut.entry.l2McEntry.mac.octet,pMac,ETHER_ADDR_LEN)))
		{
			DEBUG("vaild!!  fidmode=%d lutvid=%d lutfid=%d" ,rg_db.vlan[vlanId].fidMode);
			if((rg_db.vlan[vlanId].fidMode==VLAN_FID_IVL && rg_db.lut[search_index].rtk_lut.entry.l2McEntry.vid==vlanId) ||
				(rg_db.vlan[vlanId].fidMode==VLAN_FID_SVL && rg_db.lut[search_index].rtk_lut.entry.l2McEntry.fid==rg_db.vlan[vlanId].fid))
			{
				return search_index;
			}
		}

		count++; //search from next entry
	}
	while(count < MAX_LUT_HASH_WAY_SIZE);

	//Check bCAM LUT, if match, just return
	for(search_index=MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE;search_index<MAX_LUT_HW_TABLE_SIZE;search_index++)
	{
		DEBUG("search_index =%d",search_index);
		if(rg_db.lut[search_index].valid )
		{
			if(memcmp(rg_db.lut[search_index].rtk_lut.entry.l2McEntry.mac.octet,pMac,ETHER_ADDR_LEN)==0)
			{
				if((rg_db.vlan[vlanId].fidMode==VLAN_FID_IVL && rg_db.lut[search_index].rtk_lut.entry.l2McEntry.vid==vlanId) ||
				(rg_db.vlan[vlanId].fidMode==VLAN_FID_SVL && rg_db.lut[search_index].rtk_lut.entry.l2McEntry.fid==rg_db.vlan[vlanId].fid))
				{
					//HIT!
					return search_index;
				}
			}
		}
	}


	return RG_RET_LOOKUPIDX_NOT_FOUND;

}


/*
multicast_v4				IPV4_TAGIF
multicast_v6				IPV6_TAGIF
pppoe_multicast_v4			IPV4_TAGIF/PPPOE_TAGIF
pppoe_multicast_v6			IPV6_TAGIF/PPPOE_TAGIF
dslite_multicast			IPV4_TAGIF/DSLITEMC_INNER_TAGIF
pppoe_dslite_multicast		IPV4_TAGIF/PPPOE_TAGIF/DSLITEMC_INNER_TAGIF
*/
rtk_rg_err_code_t _modifyPacketByMulticastDecision(struct sk_buff *skb,rtk_rg_pktHdr_t *pPktHdr,uint32 txPmsk)
{

	uint8 *pTTL_hotlimit;
	uint32 txPmskNotInVlanMbr=0;
	uint32 ifEgressVlanTagif,ifEgressVlanID;
	uint8  isBridgeWanToLan=0;

	if(pPktHdr->pRxDesc->rx_reason==RG_CPU_REASON_NORMAL_FWD ){TRACE("forward to cpu pakcet skip modify packet");return RT_ERR_RG_OK;}
	if(!FWD_DECISION_IS_MC(pPktHdr->fwdDecision)){TRACE("only modify multicast data packet (RG_FWD_DECISION_FLOW_MC)");return RT_ERR_RG_OK;	}
	if(pPktHdr->fwdDecision==RG_FWD_DECISION_FLOW_RESERVED_MC){TRACE("only modify multicast data packet (RG_FWD_DECISION_FLOW_MC)");return RT_ERR_RG_OK;	}

	if(pPktHdr->srcNetifIdx == FAILED)
		pPktHdr->srcNetifIdx=_rtk_rg_decideIngressMcastNetIfIdx(pPktHdr->ingressPort);

	pPktHdr->netifIdx =_rtk_rg_decideEgressMcastNetIfIdx(txPmsk);
	if(pPktHdr->netifIdx == FAILED || pPktHdr->srcNetifIdx==FAILED)
	{
		TRACE("[Drop]Egress interface Can't find");
		return RT_ERR_RG_FAILED;
	}

	if( (rg_db.systemGlobal.interfaceInfo[pPktHdr->srcNetifIdx].storedInfo.is_wan) &&
		(rg_db.systemGlobal.interfaceInfo[pPktHdr->srcNetifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type ==RTK_RG_BRIDGE) &&
		(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.is_wan == 0 ) )
	{
		isBridgeWanToLan=1;
	}

	if(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.is_wan)
	{
		//if non_other_aclcf action we use wan vlan to send packet
		ifEgressVlanTagif = rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.egress_vlan_tag_on;
		ifEgressVlanID =rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.egress_vlan_id;
	}
	else
	{
		if(txPmsk&RTK_RG_ALL_REAL_MASTER_EXT_PORTMASK)
		{
			//do nothing default untag
			//decide by _rtk_rg_checkWlanUntagMask
			ifEgressVlanTagif=0;
		}
		else if(txPmsk&RTK_RG_ALL_REAL_SLAVE_EXT_PORTMASK)
		{
#ifdef CONFIG_DUALBAND_CONCURRENT
			ifEgressVlanTagif = (rg_db.vlan[rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.lan_intf.intf_vlan_id].UntagPortmask.bits[0] & (txPmsk&RTK_RG_ALL_MAC_SLAVE_CPU_PORTMASK))?0:1;
#else
			ifEgressVlanTagif=0;
#endif
		}
		else
		{
			ifEgressVlanTagif = (rg_db.vlan[rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.lan_intf.intf_vlan_id].UntagPortmask.bits[0] & txPmsk)?0:1;
		}
		ifEgressVlanID =rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.lan_intf.intf_vlan_id;
	}

	/*
		how to decide out_smac_trans?
		InterfaceVlan.mbr & dpMask ==0  (9600Serise and 9602C hardware decision)
		pppoe multicast routing (dmac==gmac)
	*/
	//change sa and ttl-1
	if(txPmsk & (RTK_RG_ALL_REAL_MASTER_EXT_PORTMASK | RTK_RG_ALL_REAL_SLAVE_EXT_PORTMASK))
	{
		rtk_rg_port_idx_t portIdx;
		rtk_rg_mac_port_idx_t out_mac_port;
		rtk_rg_mac_ext_port_idx_t out_mac_extPort;
#if defined(CONFIG_MASTER_WLAN0_ENABLE) && defined(CONFIG_RG_FLOW_NEW_WIFI_MODE)
		if(pPktHdr->egressWlanDevIdx>=0)
			_rtk_rg_wlanDevToPort_translator(pPktHdr->egressWlanDevIdx,&portIdx);
		else
		{
			TRACE("[Drop] pPktHdr->egressWlanDevIdx=%d error",pPktHdr->egressWlanDevIdx);
			return RT_ERR_RG_FAILED;
		}
#if defined(CONFIG_RG_G3_SERIES)
		_rtk_rg_lutExtport_translator(&portIdx);
#endif
#else	// not CONFIG_RG_FLOW_NEW_WIFI_MODE
		portIdx=RTK_RG_EXT_PORT0;
#endif
		_rtk_rg_portToMacPort_translator(portIdx,&out_mac_port,&out_mac_extPort);
		txPmskNotInVlanMbr=_rtk_rg_isVlanMember(pPktHdr->ingressDecideVlanID,out_mac_port,out_mac_extPort)?0:1;
	}
	else
	{
		txPmskNotInVlanMbr= (rg_db.vlan[pPktHdr->ingressDecideVlanID].MemberPortmask.bits[0] & txPmsk) ? 0:1;
		TRACE("ingressDecideVlanID=%d  txPmskNotInVlanMbr=%d  vlanMbr=%x txPmsk=%x",pPktHdr->ingressDecideVlanID,txPmskNotInVlanMbr,(rg_db.vlan[pPktHdr->ingressDecideVlanID].MemberPortmask.bits[0] & txPmsk),txPmsk);

	}



	if( (txPmskNotInVlanMbr && !isBridgeWanToLan) || pPktHdr->isGatewayPacket/* pppoe multicast routing*/)
	{

		//check egress interface MTU
		//FIXME: check pPktHdr->netifIdx
		if(pPktHdr->netifIdx!=DEFAULT_MC_WANIF_IDX && rg_db.netif[pPktHdr->netifIdx].rtk_netif.mtu < pPktHdr->l3Len)
		{
			pPktHdr->overMTU = 1;
			//1 FIXME: Over MTU should Trap to PS
			TRACE("[Drop] multicast egress interface[%d] over MTU=%d ",pPktHdr->netifIdx,rg_db.netif[pPktHdr->netifIdx].rtk_netif.mtu);
			return RT_ERR_RG_FAILED;
		}


		// gponDsBCModuleEnable decision > acl decision >  interface decision
		// only hit cf rule and to wifi will using interface decision (fixme:remove condition )
		if(((pPktHdr->aclDecision.ds_action_field & CF_DS_ACTION_CTAG_BIT)&& ((txPmsk&RTK_RG_ALL_REAL_MASTER_EXT_PORTMASK) || (txPmsk&RTK_RG_ALL_REAL_SLAVE_EXT_PORTMASK)))||
			(pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_CVLAN_ACT_DONE_BIT)==0x0)
		{
			TRACE("Use non-aclaction interface vlan decide egressVlanID=%d and egressVlanTagif=%d",ifEgressVlanID,ifEgressVlanTagif);
			pPktHdr->egressVlanTagif = ifEgressVlanTagif;
			pPktHdr->egressVlanID = ifEgressVlanID ;
		}


		if(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.is_wan)
			memcpy(skb->data + 6 ,rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.gmac.octet,ETHER_ADDR_LEN);
		else
			memcpy(skb->data + 6 ,rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.lan_intf.gmac.octet,ETHER_ADDR_LEN);


		if(pPktHdr->tagif & IPV4_TAGIF)
			pTTL_hotlimit=skb->data+ (pPktHdr->l3Offset + 8 );
		else
			pTTL_hotlimit=skb->data+ (pPktHdr->l3Offset + 7 );


		if( (*pTTL_hotlimit) <=1)
		{
			TRACE("Drop by TTL check");
			return RT_ERR_RG_FAILED;
		}
		else
		{
			(*pTTL_hotlimit)--;

			if(pPktHdr->tagif & IPV4_TAGIF)
			{
				uint16 *l3chksum = (uint16 *)(skb->data+ (pPktHdr->l3Offset + 10 /* chksum*/));
				*l3chksum=htons(_rtk_rg_fwdengine_L3checksumUpdate(ntohs(*pPktHdr->pIpv4Checksum),pPktHdr->ipv4Dip,pPktHdr->ipv4TTL,pPktHdr->ipProtocol,ntohl(*pPktHdr->pIpv4Dip),*pTTL_hotlimit));
				TRACE("update l3 checksum by multicast routing");
			}

		}
		pPktHdr->l3Modify =1;
		TRACE("change SA and ttl/hotlimit");

	}
	else
	{

		pPktHdr->l3Modify =0;
	}



	// high priority igmp_auto_learn_ctagif
	if(rg_db.systemGlobal.igmp_auto_learn_ctagif)
	{
		int i=0;
		for (i=0 ; i< RTK_RG_PORT_MAX ;i++)
		{
			if((1<<i) & txPmsk)
				break;
		}
		if(i==RTK_RG_PORT_MAX)
			return RT_ERR_RG_FAILED;
		
		if(txPmsk & pPktHdr->McFlowEgressCtagifMsk)
		{
			pPktHdr->egressVlanTagif = 1;
			pPktHdr->egressVlanID = pPktHdr->McFlowVlanTag[i];
			TRACE("IGMP Learning Tag CVLAN=%d  to Port:%d",pPktHdr->egressVlanID,i);
		}
		else
		{
			pPktHdr->egressVlanTagif = 0;
			TRACE("IGMP Learning UnTag to Port:%d",i);
		}

	}


	return RT_ERR_RG_OK;
}



rtk_rg_err_code_t _flowFillDsliteMulticastToPath6(rtk_rg_pktHdr_t * pPktHdr)
{
	rtk_rg_table_flowEntry_t path6Entry;
	rtk_rg_flow_extraInfo_t flowExtraInfo;
	uint32 p6flow_idx;
	int32 igrSVID,igrCVID;
	igrSVID = (pPktHdr->tagif&SVLAN_TAGIF)?pPktHdr->stagVid:0;
	igrCVID = (pPktHdr->tagif&CVLAN_TAGIF)?pPktHdr->ctagVid:0;


	bzero(&path6Entry,sizeof(path6Entry));


	path6Entry.path6.in_dsliteif = 1;
	path6Entry.path6.valid = 1 ;
	path6Entry.path6.in_path = FB_PATH_6;
	path6Entry.path6.in_protocol =FB_INPROTOCOL_ALL_ACCEPT ;

	/*  we fill this field at _rtk_rg_flow_entryCheckAndInit */
	// path6Entry.path6.in_dst_ip_check = 0;
	// path6Entry.path6.in_src_ip_check = 0;
	// path6Entry.path6.in_src_mac_check = 0;
	// path6Entry.path6.in_dst_mac_check = 0;


	/* not support now*/
	// path6Entry.path6.in_tos_check
	// path6Entry.path6.in_l4_dst_port
	// path6Entry.path6.in_l4_dst_port_check
	// path6Entry.path6.in_l4_src_port
	// path6Entry.path6.in_l4_src_port_check


	path6Entry.path6.in_tos = pPktHdr->tos;

	if(pPktHdr->tagif &  CVLAN_TAGIF)
		path6Entry.path6.in_ctagif = 1;

	if(pPktHdr->tagif &  SVLAN_TAGIF)
		path6Entry.path6.in_stagif = 1;

	if(pPktHdr->tagif &  PPPOE_TAGIF)
	{
		path6Entry.path6.in_pppoeif = 1;
		//path6Entry.path6.in_pppoe_sid_check = 1; //we fill this field at _rtk_rg_flow_entryCheckAndInit
		path6Entry.path6.in_pppoe_sid = pPktHdr->sessionId;
	}

	path6Entry.path6.in_intf_idx =pPktHdr->srcNetifIdx;

	path6Entry.path6.in_dst_ipv6_addr_hash = pPktHdr->ipv6Dip_hash;
	path6Entry.path6.in_src_ipv6_addr_hash = pPktHdr->ipv6Sip_hash;

	/*
	TABLE("v6Dip %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
		pPktHdr->pIpv6Dip[0],pPktHdr->pIpv6Dip[1],pPktHdr->pIpv6Dip[2],pPktHdr->pIpv6Dip[3],
		pPktHdr->pIpv6Dip[4],pPktHdr->pIpv6Dip[5],pPktHdr->pIpv6Dip[6],pPktHdr->pIpv6Dip[7],
		pPktHdr->pIpv6Dip[8],pPktHdr->pIpv6Dip[9],pPktHdr->pIpv6Dip[10],pPktHdr->pIpv6Dip[11],
		pPktHdr->pIpv6Dip[12],pPktHdr->pIpv6Dip[13],pPktHdr->pIpv6Dip[14],pPktHdr->pIpv6Dip[15]);
	TABLE("v6Sip %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
		pPktHdr->pIpv6Sip[0],pPktHdr->pIpv6Sip[1],pPktHdr->pIpv6Sip[2],pPktHdr->pIpv6Sip[3],
		pPktHdr->pIpv6Sip[4],pPktHdr->pIpv6Sip[5],pPktHdr->pIpv6Sip[6],pPktHdr->pIpv6Sip[7],
		pPktHdr->pIpv6Sip[8],pPktHdr->pIpv6Sip[9],pPktHdr->pIpv6Sip[10],pPktHdr->pIpv6Sip[11],
		pPktHdr->pIpv6Sip[12],pPktHdr->pIpv6Sip[13],pPktHdr->pIpv6Sip[14],pPktHdr->pIpv6Sip[15]);
	*/

	//Dmac = multicastDIP mapping Mac
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	{
		uint32 ipmcl2idx;
		rtl_igmp_lookupGroupLutIdx(pPktHdr->pIpv4Dip,0,&ipmcl2idx);
		path6Entry.path6.in_dmac_lut_idx =  ipmcl2idx;
	}
#endif
	path6Entry.path6.in_smac_lut_idx = pPktHdr->smacL2Idx;

	memset(&flowExtraInfo, 0, sizeof(rtk_rg_flow_extraInfo_t));
	if(pPktHdr->tagif&IPV6_TAGIF)
	{
		memcpy(flowExtraInfo.v6Sip.ipv6_addr, pPktHdr->pIpv6Sip, IPV6_ADDR_LEN);
		memcpy(flowExtraInfo.v6Dip.ipv6_addr, pPktHdr->pIpv6Dip, IPV6_ADDR_LEN);
	}
	flowExtraInfo.igrSVID = igrSVID;
	flowExtraInfo.igrSPRI = (pPktHdr->tagif&SVLAN_TAGIF) ? pPktHdr->stagPri : 0;
	flowExtraInfo.igrCVID = igrCVID;
	flowExtraInfo.lutSaIdx = pPktHdr->smacL2Idx;
	flowExtraInfo.lutDaIdx = path6Entry.path6.in_dmac_lut_idx;
	flowExtraInfo.path34_isGmac = 0;
	flowExtraInfo.naptOrTcpUdpGroupIdx = FAIL;
	flowExtraInfo.arpOrNeighborIdx_src = FAIL;
	flowExtraInfo.arpOrNeighborIdx_dst = FAIL;
	flowExtraInfo.staticEntry = 1;
	flowExtraInfo.addSwOnly = 0;
	flowExtraInfo.byDmac2cvid = 0;
	flowExtraInfo.pNaptFilterInfo = NULL;
	flowExtraInfo.pIgrACLFilterInfo = NULL;
	flowExtraInfo.pEgrACLFilterInfo = NULL;
	flowExtraInfo.tinyAckStreamId = -1;
	if(_rtk_rg_flow_add(&p6flow_idx, FAIL, &path6Entry, flowExtraInfo)!=RT_ERR_RG_OK)
	{
		IGMP("ADD hw Path6 Error");
		return RT_ERR_RG_FAILED;
	}
	pPktHdr->path6Idx = p6flow_idx;

	return RT_ERR_RG_OK;

}


rtk_rg_err_code_t _rtk_rg_reserved_MAC_multicastRecordFlowData(rtk_rg_pktHdr_t *pPktHdr,uint32 dpMask,uint32 allMask)
{


	rtk_rg_table_flowEntry_t portAction;
	rtk_rg_flow_extraInfo_t flowExtraInfo;
	uint32 i,msbPmsk=0;
	int32 ret = RT_ERR_RG_OK;
	int32 path1idx=-1;
	rtk_rg_table_flowEntry_t *pPath1action = &pPktHdr->McFlowPath3action;
	uint32* path3PortMsk = &pPktHdr->McFlowPath3PortMsk;
	uint32* path4PortMsk = &pPktHdr->McFlowPath4PortMsk;
	uint32* path3ExtPortMsk = &pPktHdr->McFlowPath3ExtPortMsk;
	uint32* path4ExtPortMsk = &pPktHdr->McFlowPath4ExtPortMsk;
	uint8 forceAdd=0;
	rtk_rg_mac_port_idx_t spaIdx;
	rtk_rg_mac_ext_port_idx_t extSpaIdx;
	int32 shareMeterIdx = FAIL;
	rtk_l2_mcastAddr_t mcastAddr;
	int32 mcL2Idx=FAIL;

	memset(&mcastAddr,0,sizeof(mcastAddr));
	mcastAddr.portmask.bits[0]=0;
	mcastAddr.ext_portmask_idx=31;
	mcastAddr.vid=0;
	mcastAddr.fid=LAN_FID;
	memcpy(mcastAddr.mac.octet,pPktHdr->pDmac,6);

	mcL2Idx=_rtk_rg_findAndReclamL2mcEntry(&mcastAddr.mac, mcastAddr.fid);


	if(dpMask == (1<<RTK_RG_PORT_MAX) && pPktHdr->McFlowAddTohw==0 && (*path3PortMsk !=0 || *path4PortMsk!=0 || *path3ExtPortMsk!=0 || *path4ExtPortMsk!=0))
	{
		TRACE("force Add to hw");
		forceAdd=1;
		goto ADD_DIRECT;
	}
	bzero(&portAction,sizeof(portAction));
	portAction.path1.valid=1;
	portAction.path1.in_path = FB_PATH_12;

	/* Ingress patten check */
	//portAction.path3.in_l4proto =0; //0:UDP 1:TCP but multicast always UDP
	//portAction.path3.in_tos_check =0; //not support now

	_rtk_rg_portToMacPort_translator(pPktHdr->ingressPort, &spaIdx, &extSpaIdx);

	portAction.path1.in_smac_lut_idx = pPktHdr->smacL2Idx;
	portAction.path1.in_dmac_lut_idx = mcL2Idx;
	//spa
	//flowPathEntry.path1.in_spa_check	will be set automatically.
	portAction.path1.in_spa = spaIdx;
	portAction.path1.in_ext_spa = extSpaIdx;
	portAction.path1.in_protocol = FB_INPROTOCOL_ALL_ACCEPT; //all

	portAction.path1.in_tos = pPktHdr->tos;
	portAction.path1.in_pppoeif = (pPktHdr->tagif&PPPOE_TAGIF)?TRUE:FALSE;
	portAction.path1.in_pppoe_sid = (pPktHdr->tagif&PPPOE_TAGIF)?pPktHdr->sessionId:0;

	//svlan
	portAction.path1.in_stagif = (pPktHdr->tagif&SVLAN_TAGIF)?TRUE:FALSE;
	portAction.path1.in_svlan_id = pPktHdr->stagVid;

	//cvlan
	portAction.path1.in_ctagif = (pPktHdr->tagif&CVLAN_TAGIF)?TRUE:FALSE;
	portAction.path1.in_cvlan_id = pPktHdr->ctagVid;



#if defined(CONFIG_GPON_FEATURE)
	if(portAction.path1.in_spa==RTK_RG_MAC_PORT_PON) //from pon port
	{
		//flowPathEntry.path1.in_out_stream_idx_check_act  will be set automatically.
		portAction.path1.in_out_stream_idx = pPktHdr->pRxDesc->rx_pon_stream_id;
	}
	if(dpMask== (1<<RTK_RG_MAC_PORT_PON)) //to pon port
	{
		portAction.path1.in_out_stream_idx_check_act = TRUE;
		if(rg_db.systemGlobal.initParam.wanPortGponMode)
		{
			portAction.path1.in_out_stream_idx = pPktHdr->streamID;
		}
		else
		{
			//20180212: Stream ID of EPON: MSB 3-bits(LLID)+LSB 4-bits(QID)
			portAction.path1.in_out_stream_idx = ((pPktHdr->streamID&0x7) << 4) | (pPktHdr->internalPriority&0x7);
		}
	}
#endif


	/** setting in/out interface **/
	portAction.path1.in_intf_idx = pPktHdr->srcNetifIdx;
	portAction.path1.out_intf_idx= pPktHdr->netifIdx;

	//ingore egress vlan filter
	portAction.path1.out_egress_cvid_act = 1;
	portAction.path1.out_cvlan_id = 1;


	//egress cvlan decision
	portAction.path1.out_ctag_format_act = 1;
	if(pPktHdr->egressVlanTagif)
	{
		portAction.path1.out_cpri_format_act = 1;
		portAction.path1.out_cvid_format_act = 1;
		portAction.path1.out_cvlan_id = pPktHdr->egressVlanID;
		portAction.path1.out_cpri	  = pPktHdr->egressPriority;
	}

	//egress svlan decision
	portAction.path1.out_stag_format_act = 1;
	if(pPktHdr->egressServiceVlanTagif)
	{
		portAction.path1.out_spri_format_act = 1;
		portAction.path1.out_svid_format_act = 1;
		portAction.path1.out_svlan_id = pPktHdr->egressServiceVlanID;
		portAction.path1.out_spri	  = pPktHdr->egressServicePriority ;
	}

	//egress DSCP decision
	if(pPktHdr->egressDSCPRemarking)
	{
		portAction.path1.out_dscp_act=1;
		portAction.path1.out_dscp = pPktHdr->egressDSCP ;
	}

	//if(pPktHdr->flow_out_user_pri_act)
	{
		portAction.path1.out_user_pri_act = TRUE;
		portAction.path1.out_user_priority = pPktHdr->internalPriority;
	}

	shareMeterIdx = (pPktHdr->aclDecision.qos_actions&ACL_ACTION_SHARE_METER_BIT)?pPktHdr->aclDecision.action_share_meter:FAIL;
	if(shareMeterIdx>=0)
	{
		portAction.path1.out_share_meter_act = TRUE;
		portAction.path1.out_share_meter_idx = shareMeterIdx;
	}



	// check with path1entry and path4 entry
	if(pPath1action->path1.valid)
	{
		//compare with path1entry
		if(memcmp(pPath1action,&portAction,sizeof(portAction)) == 0)
		{
			//equal update path1 out_portmask
			if(dpMask < (1<<RTK_RG_PORT_MAINCPU))
			{
				//physical port
				*path3PortMsk |= dpMask;
				TRACE("equal update path1 out_portmask =%x ",*path3PortMsk);
			}
			else
			{
				//extensionPort
#if defined(CONFIG_RG_G3_SERIES)
			// one-to-one mapping ports
			*path3ExtPortMsk|= ((dpMask&((1<<RTK_RG_PORT_CPU_WLAN1_AND_OTHERS)-1)) >> (RTK_RG_PORT_CPU_WLAN0_ROOT));
			// shared port starts from 0x18
			*path3ExtPortMsk|= ((dpMask>>(RTK_RG_PORT_CPU_WLAN1_AND_OTHERS+1))<<(RTK_RG_PORT_CPU_WLAN1_AND_OTHERS-RTK_RG_PORT_CPU_WLAN0_ROOT));
#else
				*path3ExtPortMsk|= (dpMask >> RTK_RG_EXT_BASED_PORT);
#endif
				TRACE("equal update path1 path1ExtPortMsk=%x",*path3ExtPortMsk);
			}
		}
		else
		{
			TRACE("we only support one action for reserved multicast hardware forward");
			ret = RT_ERR_RG_CHIP_NOT_SUPPORT;
			goto ERROR_OUT;

		}
	}
	else
	{
		//set new path3 entry
		if(dpMask < (1<<RTK_RG_PORT_MAINCPU))
		{
			*path3PortMsk = dpMask;
			memcpy(pPath1action,&portAction,sizeof(portAction));
			TRACE("set new path1entry out_portmask=%x",*path3PortMsk);
		}
		else
		{
			//extensionPort
#if defined(CONFIG_RG_G3_SERIES)
			// one-to-one mapping ports
			*path3ExtPortMsk|= ((dpMask&((1<<RTK_RG_PORT_CPU_WLAN1_AND_OTHERS)-1)) >> (RTK_RG_PORT_CPU_WLAN0_ROOT));
			// shared port starts from 0x18
			*path3ExtPortMsk|= ((dpMask>>(RTK_RG_PORT_CPU_WLAN1_AND_OTHERS+1))<<(RTK_RG_PORT_CPU_WLAN1_AND_OTHERS-RTK_RG_PORT_CPU_WLAN0_ROOT));
#else
			*path3ExtPortMsk|= (dpMask >> RTK_RG_EXT_BASED_PORT);
#endif
			memcpy(pPath1action,&portAction,sizeof(portAction));
			TRACE("set new path1entry path3ExtPortMsk=%x",*path3ExtPortMsk);

		}
	}



	//find MSB bit in allMask
	for(i=0 ; i<RTK_RG_PORT_MAX ;i++ )
	{
		if(RG_INVALID_PORT(i)) continue;
		if(allMask & (1<<i))
			msbPmsk=(1<<i);
	}

ADD_DIRECT:

	//the last packet send add flow to hw
	if((msbPmsk & dpMask)||forceAdd)
	{

		int32 path3ExtPmskIdx=0;
		int32 igrSVID,igrCVID;
		uint32 lutDaIdx=-1;
		uint32 sourceAddr[4];
		TRACE("the last packet send add flow to hw ");

		bzero(sourceAddr,sizeof(sourceAddr));
		//check extportMsk value
		if(((*path3ExtPortMsk &((1<<RTK_RG_MAC_EXT_PORT_MAX)-1))&&(*path4ExtPortMsk&((1<<RTK_RG_MAC_EXT_PORT_MAX)-1))) || ((*path3ExtPortMsk &(((1<<RTK_RG_MAC_EXT_PORT_MAX)-1)<<6))&&(*path4ExtPortMsk&(((1<<RTK_RG_MAC_EXT_PORT_MAX)-1)<<6))) || ((*path3ExtPortMsk &(((1<<RTK_RG_MAC_EXT_PORT_MAX)-1)<<12))&&(*path4ExtPortMsk&(((1<<RTK_RG_MAC_EXT_PORT_MAX)-1)<<12))))
		{
			TRACE("Extension Port muti-action Do not add Hw");
			ret = RT_ERR_RG_CHIP_NOT_SUPPORT;
			goto ERROR_OUT;
		}


		igrSVID = (pPktHdr->tagif&SVLAN_TAGIF)?pPktHdr->stagVid:0;
		igrCVID = (pPktHdr->tagif&CVLAN_TAGIF)?pPktHdr->ctagVid:0;


		//add lut da to lut table
		ASSERT_EQ(RTK_L2_MCASTADDR_ADD(&mcastAddr),RT_ERR_OK);
		ASSERT_EQ(mcL2Idx,mcastAddr.index);
		lutDaIdx = mcL2Idx;
		TABLE("Add lut [%d] success",mcL2Idx);


		if(pPath1action->path1.valid  && (*path3PortMsk || *path3ExtPortMsk))
		{
			path3ExtPmskIdx=0;
			if(RTK_RG_ASIC_EXTPORTMASKTABLE_ADD(*path3ExtPortMsk,&path3ExtPmskIdx)!=RT_ERR_RG_OK)
				goto ERROR_OUT;
			pPath1action->path1.out_portmask=*path3PortMsk;
			pPath1action->path1.out_ext_portmask_idx =path3ExtPmskIdx;



			pPath1action->path1.lock=0;

			memset(&flowExtraInfo, 0, sizeof(rtk_rg_flow_extraInfo_t));
			if(pPktHdr->tagif&IPV6_TAGIF)
			{
				memcpy(flowExtraInfo.v6Sip.ipv6_addr, pPktHdr->pIpv6Sip, IPV6_ADDR_LEN);
				memcpy(flowExtraInfo.v6Dip.ipv6_addr, pPktHdr->pIpv6Dip, IPV6_ADDR_LEN);
			}
			flowExtraInfo.igrSVID = igrSVID;
			flowExtraInfo.igrSPRI = (pPktHdr->tagif&SVLAN_TAGIF) ? pPktHdr->stagPri : 0;
			flowExtraInfo.igrCVID = igrCVID;
			flowExtraInfo.lutSaIdx = pPktHdr->smacL2Idx;
			flowExtraInfo.lutDaIdx = lutDaIdx;
			flowExtraInfo.path34_isGmac = pPktHdr->isGatewayPacket;
			flowExtraInfo.naptOrTcpUdpGroupIdx = FAIL;
			flowExtraInfo.arpOrNeighborIdx_src = FAIL;
			flowExtraInfo.arpOrNeighborIdx_dst = FAIL;
			flowExtraInfo.staticEntry = 0;
			flowExtraInfo.addSwOnly = 0;
			flowExtraInfo.byDmac2cvid = 0;
			flowExtraInfo.pNaptFilterInfo = NULL;
			flowExtraInfo.pIgrACLFilterInfo = NULL;
			flowExtraInfo.pEgrACLFilterInfo = NULL;
			flowExtraInfo.tinyAckStreamId = -1;
			ret = _rtk_rg_flow_add(&path1idx, FAIL, pPath1action, flowExtraInfo);

			if(ret !=RT_ERR_RG_OK)
			{
				IGMP("ADD hw Path1 Error");
				ASSERT_EQ(RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(path3ExtPmskIdx),RT_ERR_RG_OK);
				goto ERROR_OUT;
			}

		}


	}

	return RT_ERR_RG_OK;

ERROR_OUT:


	return ret;

}


rtk_rg_err_code_t _rtk_rg_reserved_UDP_multicastRecordFlowData(rtk_rg_pktHdr_t *pPktHdr,uint32 dpMask,uint32 allMask)
{


	rtk_rg_table_flowEntry_t portAction;
	rtk_rg_flow_extraInfo_t flowExtraInfo;
	uint32 i,msbPmsk=0;
	int32 ret = RT_ERR_RG_OK;
	int32 path3idx=-1;
	rtk_rg_table_flowEntry_t *pPath3action = &pPktHdr->McFlowPath3action;
	uint32* path3PortMsk = &pPktHdr->McFlowPath3PortMsk;
	uint32* path4PortMsk = &pPktHdr->McFlowPath4PortMsk;
	uint32* path3ExtPortMsk = &pPktHdr->McFlowPath3ExtPortMsk;
	uint32* path4ExtPortMsk = &pPktHdr->McFlowPath4ExtPortMsk;
	uint8 forceAdd=0;
	int32 shareMeterIdx=FAIL;


	if(dpMask == (1<<RTK_RG_PORT_MAX) && pPktHdr->McFlowAddTohw==0 && (*path3PortMsk !=0 || *path4PortMsk!=0 || *path3ExtPortMsk!=0 || *path4ExtPortMsk!=0))
	{
		TRACE("force Add to hw");
		forceAdd=1;
		goto ADD_DIRECT;
	}
	bzero(&portAction,sizeof(portAction));
	portAction.path3.valid=1;
	portAction.path3.in_path = FB_PATH_34;

	/* Ingress patten check */
	//portAction.path3.in_l4proto =0; //0:UDP 1:TCP but multicast always UDP
	//portAction.path3.in_tos_check =0; //not support now
	portAction.path3.in_tos = pPktHdr->tos;
	if(pPktHdr->tagif & PPPOE_TAGIF)
	{
		portAction.path3.in_pppoeif=1;
	}

	if(pPktHdr->tagif & SVLAN_TAGIF)
		portAction.path3.in_stagif = 1;

	if(pPktHdr->tagif & CVLAN_TAGIF)
	{
		portAction.path3.in_ctagif = 1;
#if defined(CONFIG_RG_RTL9607C_SERIES)
		if(!(rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
#endif
			portAction.path3.in_cvlan_pri = pPktHdr->ctagPri;
	}

	portAction.path3.in_l4_dst_port = pPktHdr->dport ;
	portAction.path3.in_l4_src_port = pPktHdr->sport ;

	if(pPktHdr->tagif & IPV4_TAGIF)
	{
		portAction.path3.in_dst_ipv4_addr= ntohl(*(pPktHdr->pIpv4Dip));
		portAction.path3.in_src_ipv4_addr= ntohl(*(pPktHdr->pIpv4Sip));
	}
	else
	{
		portAction.path3.in_ipv4_or_ipv6=1;
		portAction.path3.in_dst_ipv6_addr_hash =  pPktHdr->ipv6Dip_hash;
		portAction.path3.in_src_ipv6_addr_hash =  pPktHdr->ipv6Sip_hash;
	}


	/** setting in/out interface **/
	portAction.path3.in_intf_idx = pPktHdr->srcNetifIdx;
	portAction.path3.out_intf_idx= pPktHdr->netifIdx;


	//ingore egress vlan filter
	portAction.path3.out_egress_cvid_act = 1;
	portAction.path3.out_cvlan_id = 1;
	if(pPktHdr->l3Modify)
	{
		TRACE(" out_smac_trans=1 dpMask=%x interVid=%d",dpMask,rg_db.pktHdr->internalVlanID);
		portAction.path3.out_smac_trans=1;
	}


	//egress cvlan decision
	portAction.path3.out_ctag_format_act = 1;
	if(pPktHdr->egressVlanTagif)
	{
		portAction.path3.out_cpri_format_act = 1;
		portAction.path3.out_cvid_format_act = 1;
		portAction.path3.out_cvlan_id = pPktHdr->egressVlanID;
		portAction.path3.out_cpri	  = pPktHdr->egressPriority;
	}

	//egress svlan decision
	portAction.path3.out_stag_format_act = 1;
	if(pPktHdr->egressServiceVlanTagif)
	{
		portAction.path3.out_spri_format_act = 1;
		portAction.path3.out_svid_format_act = 1;
		portAction.path3.out_svlan_id = pPktHdr->egressServiceVlanID;
		portAction.path3.out_spri	  = pPktHdr->egressServicePriority ;
	}

	//egress DSCP decision
	if(pPktHdr->egressDSCPRemarking)
	{
		portAction.path3.out_dscp_act=1;
		portAction.path3.out_dscp = pPktHdr->egressDSCP ;
	}

	//if(pPktHdr->flow_out_user_pri_act)
	{
		portAction.path3.out_user_pri_act = TRUE;
		portAction.path3.out_user_priority = pPktHdr->internalPriority;
	}


	shareMeterIdx = (pPktHdr->aclDecision.qos_actions&ACL_ACTION_SHARE_METER_BIT)?pPktHdr->aclDecision.action_share_meter:FAIL;
	if(shareMeterIdx>=0)
	{
		portAction.path3.out_share_meter_act = TRUE;
		portAction.path3.out_share_meter_idx = shareMeterIdx;
	}

#if 1//defined(CONFIG_GPON_FEATURE)
	if(pPktHdr->ingressMacPort==RTK_RG_MAC_PORT_PON) //from pon port
	{
		portAction.path3.out_stream_idx = pPktHdr->pRxDesc->rx_pon_stream_id;
	}
	if(dpMask== (1<<RTK_RG_MAC_PORT_PON)) //to pon port
	{
		portAction.path3.out_stream_idx_act = TRUE;
		if(rg_db.systemGlobal.initParam.wanPortGponMode)
		{
			portAction.path3.out_stream_idx = pPktHdr->streamID;
		}
		else
		{
			//20180212: Stream ID of EPON: MSB 3-bits(LLID)+LSB 4-bits(QID)
			portAction.path3.out_stream_idx = ((pPktHdr->streamID&0x7) << 4) | (pPktHdr->internalPriority&0x7);
		}
	}
#endif


	// check with path3entry and path4 entry
	if(pPath3action->path3.valid)
	{
		//compare with path3entry
		if(memcmp(pPath3action,&portAction,sizeof(portAction)) == 0)
		{
			//equal update path3 out_portmask
			if(dpMask < (1<<RTK_RG_PORT_MAINCPU))
			{
				//physical port
				*path3PortMsk |= dpMask;
				TRACE("equal update path3 out_portmask =%x ",*path3PortMsk);
			}
			else
			{
				//extensionPort
#if defined(CONFIG_RG_G3_SERIES)
			// one-to-one mapping ports
			*path3ExtPortMsk|= ((dpMask&((1<<RTK_RG_PORT_CPU_WLAN1_AND_OTHERS)-1)) >> (RTK_RG_PORT_CPU_WLAN0_ROOT));
			// shared port starts from 0x18
			*path3ExtPortMsk|= ((dpMask>>(RTK_RG_PORT_CPU_WLAN1_AND_OTHERS+1))<<(RTK_RG_PORT_CPU_WLAN1_AND_OTHERS-RTK_RG_PORT_CPU_WLAN0_ROOT));
#else
				*path3ExtPortMsk|= (dpMask >> RTK_RG_EXT_BASED_PORT);
#endif
				TRACE("equal update path3 path3ExtPortMsk=%x",*path3ExtPortMsk);
			}
		}
		else
		{
			TRACE("we only support one action for reserved multicast hardware forward");
			ret = RT_ERR_RG_CHIP_NOT_SUPPORT;
			goto ERROR_OUT;

		}
	}
	else
	{
		//set new path3 entry
		if(dpMask < (1<<RTK_RG_PORT_MAINCPU))
		{
			*path3PortMsk = dpMask;
			memcpy(pPath3action,&portAction,sizeof(portAction));
			TRACE("set new path3entry out_portmask=%x",*path3PortMsk);
		}
		else
		{
			//extensionPort
#if defined(CONFIG_RG_G3_SERIES)
			// one-to-one mapping ports
			*path3ExtPortMsk|= ((dpMask&((1<<RTK_RG_PORT_CPU_WLAN1_AND_OTHERS)-1)) >> (RTK_RG_PORT_CPU_WLAN0_ROOT));
			// shared port starts from 0x18
			*path3ExtPortMsk|= ((dpMask>>(RTK_RG_PORT_CPU_WLAN1_AND_OTHERS+1))<<(RTK_RG_PORT_CPU_WLAN1_AND_OTHERS-RTK_RG_PORT_CPU_WLAN0_ROOT));
#else
			*path3ExtPortMsk|= (dpMask >> RTK_RG_EXT_BASED_PORT);
#endif
			memcpy(pPath3action,&portAction,sizeof(portAction));
			TRACE("set new path3entry path3ExtPortMsk=%x",*path3ExtPortMsk);

		}
	}



	//find MSB bit in allMask
	for(i=0 ; i<RTK_RG_PORT_MAX ;i++ )
	{
		if(RG_INVALID_PORT(i)) continue;
		if(allMask & (1<<i))
			msbPmsk=(1<<i);
	}

ADD_DIRECT:

	//the last packet send add flow to hw
	if((msbPmsk & dpMask)||forceAdd)
	{

		int32 path1ExtPmskIdx=0;
		int32 igrSVID,igrCVID;
		uint32 lutDaIdx=-1;
		uint32 sourceAddr[4];
		TRACE("the last packet send add flow to hw ");

		bzero(sourceAddr,sizeof(sourceAddr));
		//check extportMsk value
		if(((*path3ExtPortMsk &((1<<RTK_RG_MAC_EXT_PORT_MAX)-1))&&(*path4ExtPortMsk&((1<<RTK_RG_MAC_EXT_PORT_MAX)-1))) || ((*path3ExtPortMsk &(((1<<RTK_RG_MAC_EXT_PORT_MAX)-1)<<6))&&(*path4ExtPortMsk&(((1<<RTK_RG_MAC_EXT_PORT_MAX)-1)<<6))) || ((*path3ExtPortMsk &(((1<<RTK_RG_MAC_EXT_PORT_MAX)-1)<<12))&&(*path4ExtPortMsk&(((1<<RTK_RG_MAC_EXT_PORT_MAX)-1)<<12))))
		{
			TRACE("Extension Port muti-action Do not add Hw");
			ret = RT_ERR_RG_CHIP_NOT_SUPPORT;
			goto ERROR_OUT;
		}


		igrSVID = (pPktHdr->tagif&SVLAN_TAGIF)?pPktHdr->stagVid:0;
		igrCVID = (pPktHdr->tagif&CVLAN_TAGIF)?pPktHdr->ctagVid:0;


		//add lut da to lut table
{
		rtk_l2_mcastAddr_t mcastAddr;
		int32 mcL2Idx=FAIL;
		memset(&mcastAddr,0,sizeof(mcastAddr));
		mcastAddr.portmask.bits[0]=0;
		mcastAddr.ext_portmask_idx=31;

		if((pPktHdr->tagif & IPV4_TAGIF))
		{
			mcastAddr.mac.octet[0]=0x01;
			mcastAddr.mac.octet[1]=0x00;
			mcastAddr.mac.octet[2]=0x5e;
			mcastAddr.mac.octet[3]=(pPktHdr->ipv4Dip>>16)&0x7f;
			mcastAddr.mac.octet[4]=(pPktHdr->ipv4Dip>>8 )&0xff;
			mcastAddr.mac.octet[5]=(pPktHdr->ipv4Dip>>0 )&0xff;
		}
		else
		{
			mcastAddr.mac.octet[0]=0x33;
			mcastAddr.mac.octet[1]=0x33;
			mcastAddr.mac.octet[2]=(pPktHdr->pIpv6Dip[12])&0xff;
			mcastAddr.mac.octet[3]=(pPktHdr->pIpv6Dip[13])&0xff;
			mcastAddr.mac.octet[4]=(pPktHdr->pIpv6Dip[14])&0xff;
			mcastAddr.mac.octet[5]=(pPktHdr->pIpv6Dip[15])&0xff;

		}

		if(mcastAddr.flags&RTK_L2_MCAST_FLAG_IVL)
			mcL2Idx=_rtk_rg_findAndReclamL2mcEntryIVL(&mcastAddr.mac, mcastAddr.vid);
		else
			mcL2Idx=_rtk_rg_findAndReclamL2mcEntry(&mcastAddr.mac, mcastAddr.fid);

		ASSERT_EQ(RTK_L2_MCASTADDR_ADD(&mcastAddr),RT_ERR_OK);
		ASSERT_EQ(mcL2Idx,mcastAddr.index);
		lutDaIdx = mcL2Idx;
		TABLE("Add lut [%d] success",mcL2Idx);
}

		if(pPath3action->path3.valid  && (*path3PortMsk || *path3ExtPortMsk))
		{
			path1ExtPmskIdx=0;
			if(RTK_RG_ASIC_EXTPORTMASKTABLE_ADD(*path3ExtPortMsk,&path1ExtPmskIdx)!=RT_ERR_RG_OK)
				goto ERROR_OUT;
			pPath3action->path3.out_portmask=*path3PortMsk;
			pPath3action->path3.out_ext_portmask_idx =path1ExtPmskIdx;

			pPath3action->path3.lock=0;

			memset(&flowExtraInfo, 0, sizeof(rtk_rg_flow_extraInfo_t));
			if(pPktHdr->tagif&IPV6_TAGIF)
			{
				memcpy(flowExtraInfo.v6Sip.ipv6_addr, pPktHdr->pIpv6Sip, IPV6_ADDR_LEN);
				memcpy(flowExtraInfo.v6Dip.ipv6_addr, pPktHdr->pIpv6Dip, IPV6_ADDR_LEN);
			}
			flowExtraInfo.igrSVID = igrSVID;
			flowExtraInfo.igrSPRI = (pPktHdr->tagif&SVLAN_TAGIF) ? pPktHdr->stagPri : 0;
			flowExtraInfo.igrCVID = igrCVID;
			flowExtraInfo.lutSaIdx = pPktHdr->smacL2Idx;
			flowExtraInfo.lutDaIdx = lutDaIdx;
			flowExtraInfo.path34_isGmac = pPktHdr->isGatewayPacket;
			flowExtraInfo.naptOrTcpUdpGroupIdx = FAIL;
			flowExtraInfo.arpOrNeighborIdx_src = FAIL;
			flowExtraInfo.arpOrNeighborIdx_dst = FAIL;
			flowExtraInfo.staticEntry = 0;
			flowExtraInfo.addSwOnly = 0;
			flowExtraInfo.byDmac2cvid = 0;
			flowExtraInfo.pNaptFilterInfo = NULL;
			flowExtraInfo.pIgrACLFilterInfo = NULL;
			flowExtraInfo.pEgrACLFilterInfo = NULL;
			flowExtraInfo.tinyAckStreamId = -1;
			ret = _rtk_rg_flow_add(&path3idx, FAIL, pPath3action, flowExtraInfo);

			if(ret !=RT_ERR_RG_OK)
			{
				IGMP("ADD hw Path3 Error");
				ASSERT_EQ(RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(path1ExtPmskIdx),RT_ERR_RG_OK);
				goto ERROR_OUT;
			}

		}


	}

	return RT_ERR_RG_OK;

ERROR_OUT:


	return ret;

}



rtk_rg_err_code_t _rtk_rg_reserved_multicastRecordFlowData(rtk_rg_pktHdr_t *pPktHdr,uint32 dpMask,uint32 allMask)
{
	if(pPktHdr->tagif & UDP_TAGIF)
	{
		return _rtk_rg_reserved_UDP_multicastRecordFlowData(pPktHdr,dpMask,allMask);
	}
	else
	{
		return _rtk_rg_reserved_MAC_multicastRecordFlowData(pPktHdr,dpMask,allMask);
	}
}


#if defined(CONFIG_RG_RTL9607C_SERIES)
#define IS_WLAN_SHARE_PORT(dpMask)   ((dpMask== (1<<RTK_RG_EXT_PORT5)) || (dpMask== (1<<RTK_RG_MAC10_EXT_PORT5)) || (dpMask== (1<<RTK_RG_MAC7_EXT_PORT5)))
#elif defined(CONFIG_RG_G3_SERIES)
#define IS_WLAN_SHARE_PORT(dpMask)   ((dpMask== (1<<RTK_RG_PORT_CPU_WLAN1_AND_OTHERS)))
#elif defined(CONFIG_RG_RTL9603CVD_SERIES)
#define IS_WLAN_SHARE_PORT(dpMask)   ((dpMask== (1<<RTK_RG_EXT_PORT5)))
#else
#error
#endif

/* dpMask = (1 << rtk_rg_port_idx_t) */
rtk_rg_err_code_t _rtk_rg_multicastRecordFlowData(rtk_rg_pktHdr_t *pPktHdr,uint32 dpMask,uint32 allMask)
{
	rtk_rg_table_flowEntry_t portAction;
	rtk_rg_flow_extraInfo_t flowExtraInfo;
	uint32 i,msbPmsk=0;
	int32 ret = RT_ERR_RG_OK;
	int32 path3idx=-1,path4idx=-1;
	rtk_rg_table_flowEntry_t *pPath3action = &pPktHdr->McFlowPath3action;
	rtk_rg_table_flowEntry_t*pPath4action = &pPktHdr->McFlowPath4action;
	uint32* path3PortMsk = &pPktHdr->McFlowPath3PortMsk;
	uint32* path4PortMsk = &pPktHdr->McFlowPath4PortMsk;
	uint32* path3ExtPortMsk = &pPktHdr->McFlowPath3ExtPortMsk;
	uint32* path4ExtPortMsk = &pPktHdr->McFlowPath4ExtPortMsk;
	uint8 forceAdd=0;

	//reserved mc hw accelerater
	if(pPktHdr->fwdDecision==RG_FWD_DECISION_FLOW_RESERVED_MC)
		return _rtk_rg_reserved_multicastRecordFlowData(pPktHdr,dpMask,allMask);

	if(!(pPktHdr->tagif & UDP_TAGIF))
	{
		TRACE("Chip not support non-Udp flow Packet add to hw ");
		return RT_ERR_RG_CHIP_NOT_SUPPORT;
	}

	if(pPktHdr->McFlowAddTohw){TRACE("flow Was added Ingore to add again");return RT_ERR_RG_OK;}

	if(dpMask == (1<<RTK_RG_PORT_MAX) && pPktHdr->McFlowAddTohw==0 && (*path3PortMsk !=0 || *path4PortMsk!=0 || *path3ExtPortMsk!=0 || *path4ExtPortMsk!=0))
	{
		TRACE("force Add to hw");
		forceAdd=1;
		goto ADD_DIRECT;
	}
	bzero(&portAction,sizeof(portAction));
	portAction.path3.valid=1;
	portAction.path3.in_path = FB_PATH_34;

	/* Ingress patten check */
	//portAction.path3.in_l4proto =0; //0:UDP 1:TCP but multicast always UDP
	//portAction.path3.in_tos_check =0;	//not support now
	portAction.path3.in_tos = pPktHdr->tos;
	if(pPktHdr->tagif & PPPOE_TAGIF)
	{
		portAction.path3.in_pppoeif=1;
		//portAction.path3.in_pppoe_sid_check=1;  //we fill this field at _rtk_rg_flow_entryCheckAndInit
	}

	if(pPktHdr->tagif & SVLAN_TAGIF)
		portAction.path3.in_stagif = 1;

	if(pPktHdr->tagif & CVLAN_TAGIF)
	{
		portAction.path3.in_ctagif = 1;
#if defined(CONFIG_RG_RTL9607C_SERIES)
		if(!(rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
#endif
			portAction.path3.in_cvlan_pri = pPktHdr->ctagPri;
	}

	portAction.path3.in_l4_dst_port = pPktHdr->dport ;
	portAction.path3.in_l4_src_port = pPktHdr->sport ;

	if(pPktHdr->tagif & IPV4_TAGIF)
	{
		portAction.path3.in_dst_ipv4_addr= ntohl(*(pPktHdr->pIpv4Dip));
		portAction.path3.in_src_ipv4_addr= ntohl(*(pPktHdr->pIpv4Sip));
	}
	else
	{
		portAction.path3.in_ipv4_or_ipv6=1;
		portAction.path3.in_dst_ipv6_addr_hash =  pPktHdr->ipv6Dip_hash;
		portAction.path3.in_src_ipv6_addr_hash =  pPktHdr->ipv6Sip_hash;
	}


	/** setting in/out interface **/
	portAction.path3.in_intf_idx = pPktHdr->srcNetifIdx;
	portAction.path3.out_intf_idx= pPktHdr->netifIdx;


	//ingore egress vlan filter
	portAction.path3.out_egress_cvid_act = 1;
	portAction.path3.out_cvlan_id = 1;
	if(pPktHdr->l3Modify)
	{
		TRACE(" out_smac_trans=1 dpMask=%x interVid=%d",dpMask,rg_db.pktHdr->internalVlanID);
		portAction.path3.out_smac_trans=1;
	}


	//egress cvlan decision
	portAction.path3.out_ctag_format_act = 1;
	if(pPktHdr->egressVlanTagif)
	{
		portAction.path3.out_cpri_format_act = 1;
		portAction.path3.out_cvid_format_act = 1;
		portAction.path3.out_cvlan_id = pPktHdr->egressVlanID;
		portAction.path3.out_cpri 	  = pPktHdr->egressPriority;
	}

	//egress svlan decision
	portAction.path3.out_stag_format_act = 1;
	if(pPktHdr->egressServiceVlanTagif)
	{
		portAction.path3.out_spri_format_act = 1;
		portAction.path3.out_svid_format_act = 1;
		portAction.path3.out_svlan_id = pPktHdr->egressServiceVlanID;
		portAction.path3.out_spri 	  = pPktHdr->egressServicePriority ;
	}

	//egress DSCP decision
	if(pPktHdr->egressDSCPRemarking)
	{
		portAction.path3.out_dscp_act=1;
		portAction.path3.out_dscp = pPktHdr->egressDSCP ;
	}

	//if(pPktHdr->flow_out_user_pri_act)
	{
		portAction.path3.out_user_pri_act = TRUE;
		portAction.path3.out_user_priority = pPktHdr->internalPriority;
	}

	// check with path3entry and path4 entry
	if(pPath3action->path3.valid)
	{
		//compare with path3entry
		if(memcmp(pPath3action,&portAction,sizeof(portAction)) == 0)
		{
			//equal update path3 out_portmask
			if(dpMask < (1<<RTK_RG_PORT_MAINCPU))
			{
				//physical port
				*path3PortMsk |= dpMask;
				TRACE("equal update path3 out_portmask =%x ",*path3PortMsk);
			}
			else
			{
				//extensionPort
#if defined(CONFIG_RG_G3_SERIES)
				*path3ExtPortMsk|= (dpMask >> RTK_RG_PORT_CPU_WLAN0_ROOT);
#else
				*path3ExtPortMsk|= (dpMask >> RTK_RG_EXT_BASED_PORT);
#endif
				TRACE("equal update path3 path3ExtPortMsk=%x",*path3ExtPortMsk);
			}
		}
		else
		{
			// non-equal compare with path4entry
			if(pPath4action->path4.valid)
			{
				if(memcmp(pPath4action,&portAction,sizeof(portAction)) == 0)
				{
					//equal update path4 out_portmask
					if(dpMask < (1<<RTK_RG_PORT_MAINCPU))
					{
						//physical port
						*path4PortMsk |= dpMask;
						TRACE("equal update path4 out_portmask=%x",*path4PortMsk);
					}
					else
					{
						//extensionPort
#if defined(CONFIG_RG_G3_SERIES)
						*path4ExtPortMsk|= (dpMask >> RTK_RG_PORT_CPU_WLAN0_ROOT);
#else
						*path4ExtPortMsk|= (dpMask >> RTK_RG_EXT_BASED_PORT);
#endif
						TRACE("equal update path4 path4ExtPortMsk=%x",*path4ExtPortMsk);
					}
				}
				else
				{
					//non-equal compare with path3entry and path4entry
					//set in_path to FB_PATH_6 for memcmp , portAction never same as  portActionpPath3action/pPath4action
					pPath3action->path3.in_path = FB_PATH_6;
					pPath4action->path4.in_path = FB_PATH_6;
					*path3PortMsk=0; *path3ExtPortMsk=0; *path4PortMsk=0; *path4ExtPortMsk=0;
					WARNING(" chip not support multi-action >2 for multicast");
					ret = RT_ERR_RG_CHIP_NOT_SUPPORT;
					goto ERROR_OUT;
				}
			}
			else
			{
				//set new path4entry
				if(dpMask < (1<<RTK_RG_PORT_MAINCPU))
				{
					*path4PortMsk = dpMask;
					memcpy(pPath4action,&portAction,sizeof(portAction));
					TRACE("set new path4entry out_portmask=%x",*path4PortMsk);
				}
				else
				{
					//extensionPort
#if defined(CONFIG_RG_G3_SERIES)
					*path4ExtPortMsk|= (dpMask >> RTK_RG_PORT_CPU_WLAN0_ROOT);
#else
					*path4ExtPortMsk|= (dpMask >> RTK_RG_EXT_BASED_PORT);
#endif
					memcpy(pPath4action,&portAction,sizeof(portAction));
					TRACE("set new path4entry path4ExtPortMsk=%x",*path4ExtPortMsk);
				}
			}

		}
	}
	else
	{
		//set new path3 entry
		if(dpMask < (1<<RTK_RG_PORT_MAINCPU))
		{
			*path3PortMsk = dpMask;
			memcpy(pPath3action,&portAction,sizeof(portAction));
			TRACE("set new path3entry out_portmask=%x",*path3PortMsk);
		}
		else
		{
			//extensionPort
#if defined(CONFIG_RG_G3_SERIES)
			*path3ExtPortMsk|= (dpMask >> RTK_RG_PORT_CPU_WLAN0_ROOT);
#else
			*path3ExtPortMsk|= (dpMask >> RTK_RG_EXT_BASED_PORT);
#endif
			memcpy(pPath3action,&portAction,sizeof(portAction));
			TRACE("set new path3entry path3ExtPortMsk=%x",*path3ExtPortMsk);

		}
	}

	if(IS_WLAN_SHARE_PORT(dpMask)){TRACE("ingore add wlan share port");return RT_ERR_RG_OK;	}
	if(dpMask >= (1<<RTK_RG_PORT_MAINCPU))
	{
		TRACE("Ingore Wlan add to hw (we will add by force add)");
		return RT_ERR_RG_OK;
	}



	//find MSB bit in allMask
	for(i=0 ; i<RTK_RG_PORT_MAX ;i++ )
	{
		if(RG_INVALID_PORT(i)) continue;
		if(allMask & (1<<i))
			msbPmsk=(1<<i);
	}

ADD_DIRECT:

	//the last packet send add flow to hw
	if((msbPmsk & dpMask)||forceAdd)
	{

		int32 updateFlow=-1,firstInvalid=-1,mcFlowidx,path3ExtPmskIdx=0,path4ExtPmskIdx=0;
		int32 igrSVID,igrCVID;
		uint32 lutDaIdx=-1;
		uint32 sourceAddr[4];
		TRACE("the last packet send add flow to hw ");

		bzero(sourceAddr,sizeof(sourceAddr));
		//check extportMsk value
		if(((*path3ExtPortMsk &((1<<RTK_RG_MAC_EXT_PORT_MAX)-1))&&(*path4ExtPortMsk&((1<<RTK_RG_MAC_EXT_PORT_MAX)-1))) ||
			((*path3ExtPortMsk &(((1<<RTK_RG_MAC_EXT_PORT_MAX)-1)<<6))&&(*path4ExtPortMsk&(((1<<RTK_RG_MAC_EXT_PORT_MAX)-1)<<6))) ||
			((*path3ExtPortMsk &(((1<<RTK_RG_MAC_EXT_PORT_MAX)-1)<<12))&&(*path4ExtPortMsk&(((1<<RTK_RG_MAC_EXT_PORT_MAX)-1)<<12))))
		{
			TRACE("Extension Port muti-action Do not add Hw");
			ret = RT_ERR_RG_CHIP_NOT_SUPPORT;
			goto ERROR_OUT;
		}


		igrSVID = (pPktHdr->tagif&SVLAN_TAGIF)?pPktHdr->stagVid:0;
		igrCVID = (pPktHdr->tagif&CVLAN_TAGIF)?pPktHdr->ctagVid:0;


#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
		if(pPktHdr->tagif & IPV4_TAGIF)
			ASSERT_EQ(rtl_igmp_lookupGroupLutIdx(pPktHdr->pIpv4Dip,0,&lutDaIdx),SUCCESS);
		else
			ASSERT_EQ(rtl_igmp_lookupGroupLutIdx((uint32*)pPktHdr->pIpv6Dip,1,&lutDaIdx),SUCCESS);
#elif defined(CONFIG_RG_G3_SERIES)

#endif


		if(pPath3action->path3.valid  && (*path3PortMsk || *path3ExtPortMsk))
		{
			path3ExtPmskIdx=0;
			if(RTK_RG_ASIC_EXTPORTMASKTABLE_ADD(*path3ExtPortMsk,&path3ExtPmskIdx)!=RT_ERR_RG_OK)
				goto ERROR_OUT;
			pPath3action->path3.out_portmask=*path3PortMsk;
			pPath3action->path3.out_ext_portmask_idx =path3ExtPmskIdx;

			if(pPath4action->path4.valid && (*path4PortMsk || *path4ExtPortMsk))
				pPath3action->path3.out_multiple_act = 1;

			pPath3action->path3.lock=1;

			memset(&flowExtraInfo, 0, sizeof(rtk_rg_flow_extraInfo_t));
			if(pPktHdr->tagif&IPV6_TAGIF)
			{
				memcpy(flowExtraInfo.v6Sip.ipv6_addr, pPktHdr->pIpv6Sip, IPV6_ADDR_LEN);
				memcpy(flowExtraInfo.v6Dip.ipv6_addr, pPktHdr->pIpv6Dip, IPV6_ADDR_LEN);
			}
			flowExtraInfo.igrSVID = igrSVID;
			flowExtraInfo.igrSPRI = (pPktHdr->tagif&SVLAN_TAGIF) ? pPktHdr->stagPri : 0;
			flowExtraInfo.igrCVID = igrCVID;
			flowExtraInfo.lutSaIdx = pPktHdr->smacL2Idx;
			flowExtraInfo.lutDaIdx = lutDaIdx;
			flowExtraInfo.path34_isGmac = pPktHdr->isGatewayPacket;
			flowExtraInfo.naptOrTcpUdpGroupIdx = FAIL;
			flowExtraInfo.arpOrNeighborIdx_src = FAIL;
			flowExtraInfo.arpOrNeighborIdx_dst = FAIL;
			flowExtraInfo.staticEntry = 1;
			flowExtraInfo.addSwOnly = 0;
			flowExtraInfo.byDmac2cvid = 0;
			flowExtraInfo.pNaptFilterInfo = NULL;
			flowExtraInfo.pIgrACLFilterInfo = NULL;
			flowExtraInfo.pEgrACLFilterInfo = NULL;
			flowExtraInfo.isMulticast=1;
			flowExtraInfo.tinyAckStreamId = -1;
			ret = _rtk_rg_flow_add(&path3idx, FAIL, pPath3action, flowExtraInfo);

			if(ret !=RT_ERR_RG_OK)
			{
				IGMP("ADD hw Path3 Error");
				ASSERT_EQ(RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(path3ExtPmskIdx),RT_ERR_RG_OK);
				goto ERROR_OUT;
			}
			if(pPath3action->path3.out_multiple_act)
			{
				path4ExtPmskIdx=0;
				if(RTK_RG_ASIC_EXTPORTMASKTABLE_ADD(*path4ExtPortMsk,&path4ExtPmskIdx)!=RT_ERR_RG_OK)
					goto ERROR_OUT;
				pPath4action->path4.out_portmask = *path4PortMsk;
				pPath4action->path4.out_ext_portmask_idx =path4ExtPmskIdx;

				pPath4action->path4.in_multiple_act =  1;
				pPath4action->path4.lock=1;

				memset(&flowExtraInfo, 0, sizeof(rtk_rg_flow_extraInfo_t));
				if(pPktHdr->tagif&IPV6_TAGIF)
				{
					memcpy(flowExtraInfo.v6Sip.ipv6_addr, pPktHdr->pIpv6Sip, IPV6_ADDR_LEN);
					memcpy(flowExtraInfo.v6Dip.ipv6_addr, pPktHdr->pIpv6Dip, IPV6_ADDR_LEN);
				}
				flowExtraInfo.igrSVID = igrSVID;
				flowExtraInfo.igrSPRI = (pPktHdr->tagif&SVLAN_TAGIF) ? pPktHdr->stagPri : 0;
				flowExtraInfo.igrCVID = igrCVID;
				flowExtraInfo.lutSaIdx = pPktHdr->smacL2Idx;
				flowExtraInfo.lutDaIdx = lutDaIdx;
				flowExtraInfo.path34_isGmac = pPktHdr->isGatewayPacket;
				flowExtraInfo.naptOrTcpUdpGroupIdx = FAIL;
				flowExtraInfo.arpOrNeighborIdx_src = FAIL;
				flowExtraInfo.arpOrNeighborIdx_dst = FAIL;
				flowExtraInfo.staticEntry = 1;
				flowExtraInfo.addSwOnly = 0;
				flowExtraInfo.byDmac2cvid = 0;
				flowExtraInfo.pNaptFilterInfo = NULL;
				flowExtraInfo.pIgrACLFilterInfo = NULL;
				flowExtraInfo.pEgrACLFilterInfo = NULL;
				flowExtraInfo.isMulticast=1;
				flowExtraInfo.tinyAckStreamId = -1;
				ret = _rtk_rg_flow_add(&path4idx, FAIL, pPath4action, flowExtraInfo);
				if(ret !=RT_ERR_RG_OK)
				{
					ASSERT_EQ(RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(path4ExtPmskIdx),RT_ERR_RG_OK);

					IGMP("ADD hw Path4 Error");
					goto ERROR_OUT;
				}
			}
		}


		//get mcflowIdx
		for(i=0 ; i< DEFAULT_MAX_FLOW_COUNT ;i++)
		{
			if(rg_db.mcflowIdxtbl[i].vaild)
			{
				if(rg_db.mcflowIdxtbl[i].path3Idx == path3idx) //find same entry
				{
					updateFlow=i;
					break;
				}
			}
			else
			{
				if(firstInvalid==-1)
					firstInvalid=i;
			}
		}


		if(firstInvalid==-1 && updateFlow==-1)
		{
			ret = RT_ERR_RG_ENTRY_FULL;
			/* delete by  path3ExtPmskIdx/path4ExtPmskIdx ERROR_OUT _rtk_rg_flow_del(path3idx/path4idx)*/
			//ASSERT_EQ(RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(path3ExtPmskIdx),RT_ERR_RG_OK);
			//ASSERT_EQ(RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(path4ExtPmskIdx),RT_ERR_RG_OK);

			goto ERROR_OUT;
		}

		if(updateFlow!=-1)
		{
			mcFlowidx=updateFlow;

			//decrease old extMskIdx reference count
			if(rg_db.mcflowIdxtbl[mcFlowidx].path3ExtMskIdx>0)
				ASSERT_EQ(RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(rg_db.mcflowIdxtbl[mcFlowidx].path3ExtMskIdx),RT_ERR_RG_OK);


			//sa change delete old smacRefCnt
			if(rg_db.mcflowIdxtbl[mcFlowidx].lutSaIdx!=pPktHdr->smacL2Idx)
			{
				if(rg_db.mcflowIdxtbl[mcFlowidx].lutSaIdx!=FAIL)
					rg_db.lut[rg_db.mcflowIdxtbl[mcFlowidx].lutSaIdx].mcStaticRefCnt--;

				if(pPktHdr->smacL2Idx!=FAIL)
				{
					rg_db.lut[pPktHdr->smacL2Idx].mcStaticRefCnt++;
				}

			}
			if(rg_db.mcflowIdxtbl[mcFlowidx].path4Idx !=FAIL && path4idx==FAIL)
			{
				//delete old path4 flow
				IGMP("delete old path4[%d] flow ",rg_db.mcflowIdxtbl[mcFlowidx].path4Idx);
				ASSERT_EQ(_rtk_rg_flow_del(rg_db.mcflowIdxtbl[mcFlowidx].path4Idx,1),RT_ERR_RG_OK);
				//note:delete flow also auto clean extpmsk table
			}
			else
			{
				if(rg_db.mcflowIdxtbl[mcFlowidx].path4ExtMskIdx>0)
					ASSERT_EQ(RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(rg_db.mcflowIdxtbl[mcFlowidx].path4ExtMskIdx),RT_ERR_RG_OK);
			}

			DEBUG("UPDATE mcflowIdxtbl[%d]",mcFlowidx);
		}
		else
		{
			mcFlowidx=firstInvalid;
			bzero(&rg_db.mcflowIdxtbl[mcFlowidx],sizeof(rtk_rg_table_mcFlowIdxMapping_t));
			rg_db.mcflowIdxtbl[mcFlowidx].lutIdx=lutDaIdx;

			memcpy(rg_db.mcflowIdxtbl[mcFlowidx].flowDataBuf,pPktHdr->flowDataBuf,sizeof(rg_db.mcflowIdxtbl[mcFlowidx].flowDataBuf));
			rg_db.mcflowIdxtbl[mcFlowidx].skbLen = pPktHdr->flowDataSkbLen ;

			if(pPktHdr->tagif & IPV4_TAGIF)
			{
				memcpy(&rg_db.mcflowIdxtbl[mcFlowidx].multicastAddress[0],&pPktHdr->ipv4Dip,sizeof(uint32));
			}
			else
			{
				rg_db.mcflowIdxtbl[mcFlowidx].isIpv6=1;
				memcpy(&rg_db.mcflowIdxtbl[mcFlowidx].multicastAddress[0],(uint32*)pPktHdr->pIpv6Dip,sizeof(uint32)*4);
			}
#if defined(CONFIG_RG_G3_SERIES)
			rg_db.mcflowIdxtbl[mcFlowidx].pMcEngineInfo = _rtk_rg_g3McEngineInfoFind(rg_db.mcflowIdxtbl[mcFlowidx].multicastAddress,NULL,rg_db.mcflowIdxtbl[mcFlowidx].isIpv6);

			if(rg_db.mcflowIdxtbl[mcFlowidx].pMcEngineInfo)
			{
				rg_db.mcflowIdxtbl[mcFlowidx].pMcEngineInfo->mcHwRefCount++;
				IGMP(" %p rg_db.mcflowIdxtbl[mcFlowidx].pMcEngineInfo->mcHwRefCount =%d",rg_db.mcflowIdxtbl[mcFlowidx].pMcEngineInfo,rg_db.mcflowIdxtbl[mcFlowidx].pMcEngineInfo->mcHwRefCount);
			}
			else
				WARNING("where  rg_db.mcflowIdxtbl[mcFlowidx].pMcEngineInfo %p" ,rg_db.mcflowIdxtbl[mcFlowidx].pMcEngineInfo);
#endif
			rg_db.mcflowIdxtbl[mcFlowidx].vaild=1;

			if(pPktHdr->smacL2Idx!=FAIL)
			{
				rg_db.lut[pPktHdr->smacL2Idx].mcStaticRefCnt++;
			}


			DEBUG("ADD NEW mcflowIdxtbl[%d]",mcFlowidx);
		}

		// update path3Idx/path4Idx for  old/new entry
		rg_db.mcflowIdxtbl[mcFlowidx].path3Idx=path3idx;
		rg_db.mcflowIdxtbl[mcFlowidx].path4Idx=path4idx;
		rg_db.mcflowIdxtbl[mcFlowidx].path6Idx=pPktHdr->path6Idx;
		rg_db.mcflowIdxtbl[mcFlowidx].path3ExtMskIdx = path3ExtPmskIdx;
		rg_db.mcflowIdxtbl[mcFlowidx].path4ExtMskIdx = path4ExtPmskIdx;
		rg_db.mcflowIdxtbl[mcFlowidx].flowDataSpa = pPktHdr->ingressPort;
		rg_db.mcflowIdxtbl[mcFlowidx].streamId_llid= pPktHdr->pRxDesc->rx_pon_stream_id;
		//store smacL2Idx and set to static entry and increase static refcnt
		rg_db.mcflowIdxtbl[mcFlowidx].lutSaIdx = pPktHdr->smacL2Idx;

		if(pPktHdr->tagif & IPV4_TAGIF)
			memcpy(&sourceAddr[0],&pPktHdr->ipv4Sip,sizeof(uint32));
		else
			memcpy(&sourceAddr[0],pPktHdr->pIpv6Sip,sizeof(uint32)*4);

		//update mcflowIdxtbl idx to igmpSnnooping module for delete entry
		if(pPktHdr->tagif & IPV4_TAGIF)
			rtl_igmp_updateMCastFlowIdx(rg_db.mcflowIdxtbl[mcFlowidx].multicastAddress,sourceAddr,0,mcFlowidx,pPktHdr->sport,pPktHdr->dport);
		else
			rtl_igmp_updateMCastFlowIdx(rg_db.mcflowIdxtbl[mcFlowidx].multicastAddress,sourceAddr,1,mcFlowidx,pPktHdr->sport,pPktHdr->dport);

		pPktHdr->McFlowAddTohw=1;
		TABLE("%s mcflowIdxtbl[%d] SUCCESS address %x %x %x %x  path3Idx=%d path4Idx=%d path6Idx=%d lutIdx=%d ",
			(updateFlow!=-1)?"UPDATE":"ADD",mcFlowidx,rg_db.mcflowIdxtbl[mcFlowidx].multicastAddress[0],rg_db.mcflowIdxtbl[mcFlowidx].multicastAddress[1],rg_db.mcflowIdxtbl[mcFlowidx].multicastAddress[2],rg_db.mcflowIdxtbl[mcFlowidx].multicastAddress[3],path3idx,path4idx,pPktHdr->path6Idx,rg_db.mcflowIdxtbl[mcFlowidx].lutIdx);

	}

	return RT_ERR_RG_OK;

ERROR_OUT:
	if(ret == RT_ERR_RG_CHIP_NOT_SUPPORT)
	{
		//Delete all dip match flow entry
		for(i=0 ; i< DEFAULT_MAX_FLOW_COUNT ;i++)
		{
			if(rg_db.mcflowIdxtbl[i].vaild)
			{
				if((rg_db.mcflowIdxtbl[i].isIpv6==1 && memcmp(rg_db.mcflowIdxtbl[i].multicastAddress,pPktHdr->pIpv6Dip,sizeof(rg_db.mcflowIdxtbl[i].multicastAddress))) ||
				   (rg_db.mcflowIdxtbl[i].isIpv6==0 && rg_db.mcflowIdxtbl[i].multicastAddress[0]== pPktHdr->ipv4Dip))
				{

					if(rg_db.mcflowIdxtbl[i].path6Idx != -1)
					{
						ASSERT_EQ(_rtk_rg_flow_del(rg_db.mcflowIdxtbl[i].path6Idx,1),RT_ERR_RG_OK);
						rg_db.mcflowIdxtbl[i].path6Idx=-1;
					}

					if(rg_db.mcflowIdxtbl[i].path4Idx != -1)
					{
						ASSERT_EQ(_rtk_rg_flow_del(rg_db.mcflowIdxtbl[i].path4Idx,1),RT_ERR_RG_OK);
						rg_db.mcflowIdxtbl[i].path4Idx=-1;

					}

					if(rg_db.mcflowIdxtbl[i].path3Idx != -1)
					{
						ASSERT_EQ(_rtk_rg_flow_del(rg_db.mcflowIdxtbl[i].path3Idx,1),RT_ERR_RG_OK);
						rg_db.mcflowIdxtbl[i].path3Idx=-1;

					}

				}
			}
		}

	}

	if(pPktHdr->path6Idx != -1)
	{
		ASSERT_EQ(_rtk_rg_flow_del(pPktHdr->path6Idx,1),RT_ERR_RG_OK);
		pPktHdr->path6Idx=-1;
	}
	if(path4idx != -1)
	{
		ASSERT_EQ(_rtk_rg_flow_del(path4idx,1),RT_ERR_RG_OK);
	}
	if(path3idx != -1)
	{
		ASSERT_EQ(_rtk_rg_flow_del(path3idx,1),RT_ERR_RG_OK);
	}

return ret;

}



rtk_rg_err_code_t rtk_rg_apolloPro_multicastFlow_add(rtk_rg_multicastFlow_t *mcFlow, int *flow_idx)
{

	uint32 updateFlow=FAIL,firstInvalid=FAIL,mcFlowidx;
	uint32 lutDaIdx=FAIL;
	int32 i;
	uint32 macPmsk=0;
	uint8 dipHit=0;
	uint8 sipHit=0;

	if(mcFlow==NULL || flow_idx==NULL) RETURN_ERR(RT_ERR_RG_NULL_POINTER);
	if(mcFlow->isIVL && (mcFlow->vlanID>=MAX_VLAN_SW_TABLE_SIZE)) RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	if(mcFlow->port_mask.portmask>(1<<(RTK_RG_EXT_PORT1+1)))WARNING("MC port_mask(%x) shoud not bigger than %x",mcFlow->port_mask.portmask,(0x1<<(RTK_RG_EXT_PORT1+1))-1);

	if(rg_db.systemGlobal.initParam.ivlMulticastSupport != mcFlow->isIVL )
	{
		WARNING("RT_ERR_RG_INVALID_PARAM rg_init please check multicast ivl support ivlMulticastSupport=%d != mcFlow->isIVL=%d",
			rg_db.systemGlobal.initParam.ivlMulticastSupport,mcFlow->isIVL);
		return RT_ERR_RG_INVALID_PARAM;
	}
	if(mcFlow->srcFilterMode == RTK_RG_IPV4MC_EXCLUDE)
	{
		WARNING("RT_ERR_RG_INVALID_PARAM");
		return RT_ERR_RG_INVALID_PARAM;
	}
	else if(mcFlow->srcFilterMode == RTK_RG_IPV4MC_INCLUDE)
	{
		if(rg_db.systemGlobal.initParam.igmpSnoopingEnable!=2)
		{
			WARNING("auto change igmpSnoopingEnable to mode 2 care multicast sip");
			rg_db.systemGlobal.initParam.igmpSnoopingEnable=0;
			_rtk_rg_igmpSnoopingOnOff(0,0,rg_db.systemGlobal.initParam.ivlMulticastSupport);
			rg_db.systemGlobal.initParam.igmpSnoopingEnable=2;
			_rtk_rg_igmpSnoopingOnOff(rg_db.systemGlobal.initParam.igmpSnoopingEnable,0,rg_db.systemGlobal.initParam.ivlMulticastSupport);
		}
	}
	else if(mcFlow->srcFilterMode == RTK_RG_IPV4MC_DONT_CARE_SRC)
	{
		if(rg_db.systemGlobal.initParam.igmpSnoopingEnable!=1)
		{
			WARNING("auto change igmpSnoopingEnable to mode 1 ");
			rg_db.systemGlobal.initParam.igmpSnoopingEnable=0;
			_rtk_rg_igmpSnoopingOnOff(0,0,rg_db.systemGlobal.initParam.ivlMulticastSupport);
			rg_db.systemGlobal.initParam.igmpSnoopingEnable=1;
			_rtk_rg_igmpSnoopingOnOff(rg_db.systemGlobal.initParam.igmpSnoopingEnable,0,rg_db.systemGlobal.initParam.ivlMulticastSupport);
		}
	}


	//get mcflowIdx
	for(i=0 ; i< DEFAULT_MAX_FLOW_COUNT ;i++)
	{
		if(rg_db.mcflowIdxtbl[i].vaild)
		{
			dipHit=0;
			sipHit=0;

			//find dummy entry
			if(rg_db.mcflowIdxtbl[i].path3Idx== FAIL && rg_db.mcflowIdxtbl[i].path4Idx== FAIL && mcFlow->isIPv6==rg_db.mcflowIdxtbl[i].isIpv6
				 &&  (((mcFlow->isIPv6==1) && (memcmp(rg_db.mcflowIdxtbl[i].multicastAddress,mcFlow->multicast_ipv6_addr,sizeof(rg_db.mcflowIdxtbl[i].multicastAddress))==0))  ||
				 	  ((mcFlow->isIPv6==0) && (rg_db.mcflowIdxtbl[i].multicastAddress[0] ==mcFlow->multicast_ipv4_addr))))
			{
				dipHit=1;
			}

			if(rg_db.mcflowIdxtbl[i].careSip && mcFlow->srcFilterMode == RTK_RG_IPV4MC_INCLUDE)
			{
				if(   ((mcFlow->isIPv6==1) && (memcmp(rg_db.mcflowIdxtbl[i].multicastSrcAddr,mcFlow->includeOrExclude_v6Ip,sizeof(rg_db.mcflowIdxtbl[i].multicastAddress))==0))  ||
				 	  ((mcFlow->isIPv6==0) && (rg_db.mcflowIdxtbl[i].multicastSrcAddr[0]==mcFlow->includeOrExcludeIp)))
				{
					sipHit=1;
				}
			}
			else
				sipHit=1;


			if(dipHit && sipHit)
			{
				updateFlow=i;
				break;
			}

		}
		else
		{
			if(firstInvalid==-1)
				firstInvalid=i;
		}
	}

	if(firstInvalid == FAIL && updateFlow==FAIL){
		rtk_rg_drv_event_record(RTK_RG_DRV_EVENT_REC_MC_TBL_FULL);
		return RT_ERR_RG_ENTRY_FULL;
	}
	if(updateFlow!= FAIL )
	{
		//fined same entry
		mcFlowidx=updateFlow;
		IGMP(" Multicast Update at flowIdx[%d]",mcFlowidx);
	}
	else
	{
		mcFlowidx=firstInvalid;
		bzero(&rg_db.mcflowIdxtbl[mcFlowidx],sizeof(rtk_rg_table_mcFlowIdxMapping_t));
	}

	macPmsk= mcFlow->port_mask.portmask & ((1<<RTK_RG_EXT_PORT0)-1);
	if(mcFlow->port_mask.portmask & (1<<RTK_RG_EXT_PORT0))
	{
#if defined(CONFIG_MASTER_WLAN0_ENABLE) && defined(CONFIG_RG_FLOW_NEW_WIFI_MODE)
		macPmsk |= (RTK_RG_ALL_REAL_MASTER_EXT_PORTMASK | RTK_RG_ALL_REAL_SLAVE_EXT_PORTMASK);
#else
		macPmsk |= RTK_RG_ALL_REAL_MASTER_EXT_PORTMASK;
#endif
	}
#ifdef CONFIG_DUALBAND_CONCURRENT
	if(mcFlow->port_mask.portmask & (1<<RTK_RG_EXT_PORT1))
	{
		macPmsk |= RTK_RG_ALL_REAL_SLAVE_EXT_PORTMASK;
	}
#endif

	//add new entry


	if(mcFlow->isIPv6)
	{
		memcpy(&(rg_db.mcflowIdxtbl[mcFlowidx].multicastAddress[0]),&(mcFlow->multicast_ipv6_addr[0]),sizeof(uint32)*4);
		if(mcFlow->srcFilterMode == RTK_RG_IPV4MC_INCLUDE)
		{
			rg_db.mcflowIdxtbl[mcFlowidx].careSip =1 ;
			memcpy(&(rg_db.mcflowIdxtbl[mcFlowidx].multicastSrcAddr[0]),&(mcFlow->includeOrExclude_v6Ip[0]),sizeof(uint32)*4);
		}

		ASSERT_EQ(rtl_igmp_addVirtualGroup(rg_db.mcflowIdxtbl[mcFlowidx].multicastAddress,mcFlow->isIPv6,macPmsk,mcFlow->isIVL,mcFlow->vlanID,rg_db.mcflowIdxtbl[mcFlowidx].careSip,rg_db.mcflowIdxtbl[mcFlowidx].multicastSrcAddr),RT_ERR_RG_OK);

	}
	else
	{
		rg_db.mcflowIdxtbl[mcFlowidx].multicastAddress[0]=mcFlow->multicast_ipv4_addr;
		if(mcFlow->srcFilterMode == RTK_RG_IPV4MC_INCLUDE)
		{
			rg_db.mcflowIdxtbl[mcFlowidx].careSip =1 ;
			rg_db.mcflowIdxtbl[mcFlowidx].multicastSrcAddr[0]=mcFlow->includeOrExcludeIp;
		}
		ASSERT_EQ(rtl_igmp_addVirtualGroup(rg_db.mcflowIdxtbl[mcFlowidx].multicastAddress,mcFlow->isIPv6,macPmsk,mcFlow->isIVL,mcFlow->vlanID,rg_db.mcflowIdxtbl[mcFlowidx].careSip,rg_db.mcflowIdxtbl[mcFlowidx].multicastSrcAddr),RT_ERR_RG_OK);

	}

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	if(mcFlow->isIPv6)
	{
		rtl_igmp_lookupGroupLutIdx(&(mcFlow->multicast_ipv6_addr[0]),mcFlow->isIPv6,&lutDaIdx);
	}
	else
	{
		rtl_igmp_lookupGroupLutIdx(&(mcFlow->multicast_ipv4_addr),mcFlow->isIPv6,&lutDaIdx);
	}
#elif defined(CONFIG_RG_G3_SERIES)
	if(mcFlow->isIPv6)
	{
		rg_db.mcflowIdxtbl[mcFlowidx].pMcEngineInfo=_rtk_rg_g3McEngineInfoFind(rg_db.mcflowIdxtbl[mcFlowidx].multicastAddress,NULL,mcFlow->isIPv6);
	}
	else
	{
		rg_db.mcflowIdxtbl[mcFlowidx].pMcEngineInfo=_rtk_rg_g3McEngineInfoFind(rg_db.mcflowIdxtbl[mcFlowidx].multicastAddress,NULL,mcFlow->isIPv6);
	}

#endif

	rg_db.mcflowIdxtbl[mcFlowidx].vaild = 1;
	rg_db.mcflowIdxtbl[mcFlowidx].staticEnty = 1;
	rg_db.mcflowIdxtbl[mcFlowidx].path3Idx = FAIL;
	rg_db.mcflowIdxtbl[mcFlowidx].path4Idx = FAIL;
	rg_db.mcflowIdxtbl[mcFlowidx].path6Idx = FAIL;
	rg_db.mcflowIdxtbl[mcFlowidx].lutIdx = lutDaIdx;
	rg_db.mcflowIdxtbl[mcFlowidx].isIpv6 = mcFlow->isIPv6;
	rg_db.mcflowIdxtbl[mcFlowidx].skbLen = FAIL;
	rg_db.mcflowIdxtbl[mcFlowidx].macPmsk = macPmsk;
	rg_db.mcflowIdxtbl[mcFlowidx].isIVL= mcFlow->isIVL;
	rg_db.mcflowIdxtbl[mcFlowidx].fid_vlan=mcFlow->vlanID;

	IGMP(" ADD Multicast at flowIdx[%d]  path3Idx=%d path4Idx=%d lutIdx=%d macPmsk=%x",
		mcFlowidx,rg_db.mcflowIdxtbl[mcFlowidx].path3Idx,rg_db.mcflowIdxtbl[mcFlowidx].path4Idx,rg_db.mcflowIdxtbl[mcFlowidx].lutIdx,rg_db.mcflowIdxtbl[mcFlowidx].macPmsk);

	*flow_idx = mcFlowidx;

	return RT_ERR_RG_OK;

}


rtk_rg_err_code_t rtk_rg_apolloPro_multicastFlow_del(int flow_idx)
{
	if(flow_idx<0 || DEFAULT_MAX_FLOW_COUNT <= flow_idx)
		return RT_ERR_RG_INVALID_PARAM;
	if(rg_db.mcflowIdxtbl[flow_idx].vaild==0)
		return RT_ERR_RG_ENTRY_NOT_EXIST;

	if(rg_db.mcflowIdxtbl[flow_idx].path3Idx==-1 && rg_db.mcflowIdxtbl[flow_idx].path4Idx==-1 && rg_db.mcflowIdxtbl[flow_idx].staticEnty)
	{
		//Delete dummy igmp Entry
		rtl_igmp_multicastGroupDel(&(rg_db.mcflowIdxtbl[flow_idx].multicastAddress[0]),rg_db.mcflowIdxtbl[flow_idx].isIpv6,rg_db.mcflowIdxtbl[flow_idx].careSip,rg_db.mcflowIdxtbl[flow_idx].multicastSrcAddr,1);
		rg_db.mcflowIdxtbl[flow_idx].vaild=0;
	}
	else
	{
		//Delet this McFlow
		rtl_igmp_multicastFlow_del(flow_idx);
		//rg_db.mcflowIdxtbl[flow_idx].vaild=0; //invalid by rtk_igmp_multicastFlow_del
	}

	return RT_ERR_RG_OK;

}


rtk_rg_err_code_t rtl_rg_mcDataReFlushAndAdd(uint32* GroupIp)
{
	struct sk_buff *skb;
	int i,rg_fwdengine_ret_code=0;
	uint32 zeroGroup[4]={0};
	uint8 donCareGroup=0;
	uint8 tmpTracefilterShow= rg_kernel.tracefilterShow;
	rg_kernel.tracefilterShow =0; //disable tracefilter show

	if (_rtk_rg_get_initState()!=RTK_RG_INIT_FINISHED)
		return RT_ERR_RG_OK;


	if(GroupIp==NULL)
	{
		WARNING("GroupIp NULL");
		return RT_ERR_RG_NULL_POINTER;
	}

	if(memcmp(zeroGroup,GroupIp,sizeof(zeroGroup))==0)
		donCareGroup=1;
	//IGMP("ReFlashAndAdd Group %pI6",GroupIp);

	for(i=0 ; i< DEFAULT_MAX_FLOW_COUNT; i++)
	{
		if(!rg_db.mcflowIdxtbl[i].vaild  || rg_db.mcflowIdxtbl[i].skbLen<=0 || rg_db.mcflowIdxtbl[i].staticEnty)
			continue;

		if( !donCareGroup && memcmp(GroupIp,rg_db.mcflowIdxtbl[i].multicastAddress,sizeof(rg_db.mcflowIdxtbl[i].multicastAddress))!=0)
			continue;

		rg_db.pktHdr=&rg_db.systemGlobal.pktHeader_2;

		rg_db.pktHdr->ingressPort = rg_db.mcflowIdxtbl[i].flowDataSpa;
#ifdef CONFIG_GPON_FEATURE
		rg_db.pktHdr->streamID =  rg_db.mcflowIdxtbl[i].streamId_llid ;
#endif

		skb=(struct sk_buff *)_rtk_rg_getAlloc(RG_FWDENGINE_PKT_LEN);

		//call fwdEngineInput, the alloc counter will be added. so don't need to add again
		if ((skb)&&(rg_db.systemGlobal.fwdStatistic))
		{
			#if RTK_RG_SKB_PREALLOCATE
			rg_db.systemGlobal.statistic.perPortCnt_skb_pre_alloc_for_uc[rg_db.pktHdr->ingressPort]--;
			#else
			rg_db.systemGlobal.statistic.perPortCnt_skb_alloc[rg_db.pktHdr->ingressPort]--;
			#endif
		}
		if (skb==NULL)
		{
			WARNING("Error: skb null");
			return RT_ERR_RG_FAILED;
		}

		bzero((void*)skb->data, DEFAULT_MAX_DATA_PATTEN);
		skb_reserve(skb, RX_OFFSET);

		memcpy((void*)skb->data,rg_db.mcflowIdxtbl[i].flowDataBuf,rg_db.mcflowIdxtbl[i].skbLen);
		skb_put(skb, rg_db.mcflowIdxtbl[i].skbLen);

		//we need spa to decide ingress interface


		rg_kernel.rxInfoFromMcDataBuf.rx_src_port_num=RTK_RG_MAC_PORT_MAINCPU;
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
		rg_kernel.rxInfoFromMcDataBuf.rx_extspa=RTK_RG_MAC_EXT_PORT_MAX;
#else
		rg_kernel.rxInfoFromMcDataBuf.rx_dst_port_mask=0x20;
#endif

		rg_fwdengine_ret_code = rtk_rg_fwdEngineInput(skb, &rg_kernel.rxInfoFromMcDataBuf);
		//Processing packets

		//unexpect Warning
		if(rg_db.mcflowIdxtbl[i].pppoePassthroughEntry )
		{
			if(rg_fwdengine_ret_code!=RG_FWDENGINE_RET_DIRECT_TX)
			{
				//pppoe passthrough is unicast ,packet free by _rtk_rg_egressPacketSend pPktHdr->ingressLocation == RG_IGR_MC_DATA_BUF
				//do not free this packet again
				WARNING("unexpect ret vaule on pppoePassthroughEntry ret=%d",rg_fwdengine_ret_code);
				rg_db.mcflowIdxtbl[i].vaild=0;
			}
		}
		else
		{
			//normal multicast entry
			if(rg_fwdengine_ret_code != RG_FWDENGINE_RET_DROP)
			{
				WARNING("unexpect ret vaule on multicast mcflow ret=%d",rg_fwdengine_ret_code);
				rg_db.mcflowIdxtbl[i].vaild=0;
			}
		}

		if(rg_fwdengine_ret_code == RG_FWDENGINE_RET_TO_PS)
		{
			//FIXME:iPhone 5 change wireless connection from master to slave will send strange unicast ARP request for LAN gateway IP, and forwarded by protocol stack
			TRACE("RG_IGR_MC_DATA_BUF[%x]: To Protocol-Stack...FREE SKB!!",(POINTER_CAST)skb&0xffff);
			//dump_packet(skb->data,skb->len,"dump_back_to_PS");
			_rtk_rg_dev_kfree_skb_any(skb);
		}
		else if (rg_fwdengine_ret_code == RG_FWDENGINE_RET_DROP)
		{
			TRACE("ARP_RG_IGR_MC_DATA_BUFGEN[%x]: Drop...FREE SKB!!",(POINTER_CAST)skb&0xffff);
			_rtk_rg_dev_kfree_skb_any(skb);
		}
		else
		{

			TRACE("RG_IGR_MC_DATA_BUF[%x]: Forward",(POINTER_CAST)skb&0xffff);
		}




		rg_db.pktHdr=&rg_db_cache.pktHeader_1;

	}

	rg_kernel.tracefilterShow = tmpTracefilterShow;


	return RT_ERR_RG_OK;

}



rtk_rg_err_code_t rtk_rg_apolloPro_multicastFlow_find(rtk_rg_multicastFlow_t *mcFlow, int *valid_idx)
{
	WARNING("rtk_rg_apolloPro_multicastFlow_find RT_ERR_RG_CHIP_NOT_SUPPORT");
	return RT_ERR_RG_CHIP_NOT_SUPPORT;
}

rtk_rg_err_code_t rtk_rg_apolloPro_l2MultiCastFlow_add(rtk_rg_l2MulticastFlow_t *l2McFlow,int *flow_idx)
{
	WARNING("rtk_rg_apolloPro_l2MultiCastFlow_add RT_ERR_RG_CHIP_NOT_SUPPORT");
	return RT_ERR_RG_CHIP_NOT_SUPPORT;
}


rtk_rg_err_code_t rtk_rg_apolloPro_UrlPri_add(rtk_rg_urlHighPri_t* urlPriEt,int *entry_idx)
{
	int i,first_invalid=-1;

	if(entry_idx==NULL)
		return RT_ERR_RG_NULL_POINTER;
	if(urlPriEt->urlpri < 0 || urlPriEt->urlpri >7)
		return RT_ERR_RG_INVALID_PARAM;

	for(i=0 ; i< MAX_URL_HiPri_ENTRY_SIZE ;i++ )
	{
		if(!rg_db.systemGlobal.urlHiPri_table_entry[i].valid)
		{
			if(first_invalid==-1)
				first_invalid=i;
			break;
		}
	}

	if(first_invalid==-1)
		return RT_ERR_RG_ENTRY_FULL;

	memcpy(&rg_db.systemGlobal.urlHiPri_table_entry[first_invalid].urlHighPriEt,urlPriEt,sizeof(rtk_rg_urlHighPri_t));
	rg_db.systemGlobal.urlHiPri_table_entry[first_invalid].valid=1;
	INIT_LIST_HEAD(&rg_db.systemGlobal.urlHiPri_table_entry[first_invalid].hiPriEntryIdxListHdr);

	TABLE("ADD URL-PRI TABLE[%d]",first_invalid);
	*entry_idx = first_invalid;
	if(rg_db.systemGlobal.urlHiPri_En==0)
	{
		rg_db.systemGlobal.urlHiPri_En=1;
#if !defined(CONFIG_RG_G3_SERIES)		
		ASSERT_EQ(rtk_rg_asic_unmatchedCpuPriority_set(RTK_RG_UNMATCH_FLOW_TRAP_PRI_HTTP),RT_ERR_RG_OK);
#endif
	}
	_rtk_rg_flow_clear();

	return RT_ERR_RG_OK;

}

rtk_rg_err_code_t rtk_rg_apolloPro_UrlPri_del(int entry_idx)
{
	rtk_rg_hiPriEntryIdx_t* p_hiPriEtIdx;
	rtk_rg_hiPriEntryIdx_t* p_hiPriEtIdx_tmp;
	int i;
	int first_valid=-1;
	if(entry_idx<0 || entry_idx>MAX_URL_HiPri_ENTRY_SIZE)		
		return RT_ERR_RG_INVALID_PARAM;
	if(rg_db.systemGlobal.urlHiPri_table_entry[entry_idx].valid==0)
		return RT_ERR_RG_ENTRY_NOT_EXIST;

	//delete idx list
	list_for_each_entry_safe(p_hiPriEtIdx,p_hiPriEtIdx_tmp,&rg_db.systemGlobal.urlHiPri_table_entry[entry_idx].hiPriEntryIdxListHdr,hiPriEntryIdx_list)	
	{
		rtk_rg_apolloPro_HiPriEntry_del(p_hiPriEtIdx->hiPriEntryIdx);
		rtk_rg_apolloPro_HiPriEntry_del(p_hiPriEtIdx->hiPriEntryIdx2);
		list_move(&p_hiPriEtIdx->hiPriEntryIdx_list,&rg_db.systemGlobal.urlHiPriIdxfreeEntry); 
	}

	rg_db.systemGlobal.urlHiPri_table_entry[entry_idx].valid=0;
	TABLE("DEL URL-PRI TABLE[%d]",entry_idx);

	for(i=0 ; i< MAX_URL_HiPri_ENTRY_SIZE ;i++ )
	{
		if(rg_db.systemGlobal.urlHiPri_table_entry[i].valid)
		{
			if(first_valid==-1)
				first_valid=i;
			break;
		}
	}
	if(first_valid==-1)
	{
		rg_db.systemGlobal.urlHiPri_En=0;
#if !defined(CONFIG_RG_G3_SERIES)		
		if(rg_db.systemGlobal.prevent_control_packet_drop == ENABLE)
			ASSERT_EQ(rtk_rg_asic_unmatchedCpuPriority_set(RTK_RG_UNMATCH_FLOW_TRAP_PRI),RT_ERR_RG_OK);
		else
			ASSERT_EQ(rtk_rg_asic_unmatchedCpuPriority_set(RTK_RG_UNMATCH_FLOW_TRAP_PRI_DEFAULT),RT_ERR_RG_OK);		
#endif		
		TABLE("disable urlHiPri function ");
	}
	_rtk_rg_flow_clear();


	return RT_ERR_RG_OK;

}



rtk_rg_err_code_t rtk_rg_apolloPro_HiPriEntry_add(rtk_rg_table_highPriPatten_t hiPriEntry,int *entry_idx)
{
	int i,first_invalid=-1;

	if(entry_idx==NULL)
		return RT_ERR_RG_NULL_POINTER;
	if(hiPriEntry.careSipEn==0 && hiPriEntry.careDipEn==0 && hiPriEntry.careSportEn==0 && hiPriEntry.careDportEn==0 && hiPriEntry.careL4ProtoEn==0)
		return RT_ERR_RG_INVALID_PARAM;
	if(hiPriEntry.hp_tcpudp!=0 && hiPriEntry.hp_tcpudp!=1)
		return RT_ERR_RG_INVALID_PARAM;


	for(i=0 ; i< MAX_FLOW_HIGHPRI_TABLE_SIZE ;i++)
	{
		if(rg_db.flowHighPriTbl[i].valid==0)
		{
			if(first_invalid==-1)
				first_invalid=i;
		}
		else if(memcmp(&rg_db.flowHighPriTbl[i].highPriPatten,&hiPriEntry,sizeof(hiPriEntry))==0)
		{
			TABLE("ADD SAME PATTEN [%d] Add RefCount[%d->%d]",i,rg_db.flowHighPriTbl[i].hiPriRefCnt,rg_db.flowHighPriTbl[i].hiPriRefCnt+1);		
			rg_db.flowHighPriTbl[i].hiPriRefCnt++;
			*entry_idx = i;
			return RT_ERR_RG_OK;
		}
	}

	if(first_invalid==-1)
		return RT_ERR_RG_ENTRY_FULL;

	memcpy(&rg_db.flowHighPriTbl[first_invalid].highPriPatten,&hiPriEntry,sizeof(hiPriEntry));
	rg_db.flowHighPriTbl[first_invalid].valid=1;
	rg_db.flowHighPriTbl[first_invalid].hiPriRefCnt=1;
	*entry_idx = first_invalid;
	TABLE("ADD flowHighPriTbl[%d]",*entry_idx);
	//_rtk_rg_flow_clear();

	return RT_ERR_RG_OK;
}

rtk_rg_err_code_t rtk_rg_apolloPro_HiPriEntry_del(int entry_idx)
{
	uint32 i, v6SipHash=0, v6DipHash=0;

	if( entry_idx < 0	|| MAX_FLOW_HIGHPRI_TABLE_SIZE <= entry_idx)
		return RT_ERR_RG_INVALID_PARAM;

	if(rg_db.flowHighPriTbl[entry_idx].valid==0)
		return RT_ERR_RG_ENTRY_NOT_EXIST;

	if(rg_db.flowHighPriTbl[entry_idx].hiPriRefCnt>1)
	{
		TABLE("flowHighPriTbl[%d] refcoutn decrease[%d->%d]",entry_idx,rg_db.flowHighPriTbl[entry_idx].hiPriRefCnt,rg_db.flowHighPriTbl[entry_idx].hiPriRefCnt-1);
		rg_db.flowHighPriTbl[entry_idx].hiPriRefCnt--;
	 	return RT_ERR_RG_OK;
	}

	if(rg_db.flowHighPriTbl[entry_idx].highPriPatten.isIpv6)
	{
		v6SipHash = _rtk_rg_sw_flowHashIPv6SrcAddr_get((uint8*)&rg_db.flowHighPriTbl[entry_idx].highPriPatten.hp_sip[0]);
		v6DipHash = _rtk_rg_sw_flowHashIPv6DstAddr_get((uint8*)&rg_db.flowHighPriTbl[entry_idx].highPriPatten.hp_dip[0]);
	}
	for(i=0; i<MAX_FLOW_SW_TABLE_SIZE; i++)
	{
		if(rg_db.flow[i].sw_valid==0)
			continue;
		if(RG_PFLOW(i)->path1.in_path!=FB_PATH_5)
			continue;
		if(RG_PFLOW(i)->path5.in_ipv4_or_ipv6!=rg_db.flowHighPriTbl[entry_idx].highPriPatten.isIpv6)
			continue;
		if(rg_db.flowHighPriTbl[entry_idx].highPriPatten.careSipEn)
		{
			if(RG_PFLOW(i)->path5.in_ipv4_or_ipv6==0)
			{
				if(RG_PFLOW(i)->path5.in_src_ipv4_addr!=rg_db.flowHighPriTbl[entry_idx].highPriPatten.hp_sip[0])
					continue;
			}
			else
			{
				if(RG_PFLOW(i)->path5.in_src_ipv6_addr_hash!=v6SipHash)
					continue;
			}
		}
		if(rg_db.flowHighPriTbl[entry_idx].highPriPatten.careDipEn)
		{
			if(RG_PFLOW(i)->path5.in_ipv4_or_ipv6==0)
			{
				if(RG_PFLOW(i)->path5.in_dst_ipv4_addr!=rg_db.flowHighPriTbl[entry_idx].highPriPatten.hp_dip[0])
					continue;
			}
			else
			{
				if(RG_PFLOW(i)->path5.in_dst_ipv6_addr_hash!=v6DipHash)
					continue;
			}
		}
		if(rg_db.flowHighPriTbl[entry_idx].highPriPatten.careSportEn && RG_PFLOW(i)->path5.in_l4_src_port!=rg_db.flowHighPriTbl[entry_idx].highPriPatten.hp_sport)
			continue;
		if(rg_db.flowHighPriTbl[entry_idx].highPriPatten.careDportEn && RG_PFLOW(i)->path5.in_l4_dst_port!=rg_db.flowHighPriTbl[entry_idx].highPriPatten.hp_dport)
			continue;
		if(rg_db.flowHighPriTbl[entry_idx].highPriPatten.careL4ProtoEn && RG_PFLOW(i)->path5.in_l4proto!=rg_db.flowHighPriTbl[entry_idx].highPriPatten.hp_tcpudp)
			continue;
		// clear highPri2HwEntry bit
		rg_db.flow[i].highPri2HwEntry = 0;
	}

	rg_db.flowHighPriTbl[entry_idx].valid=0;
	rg_db.flowHighPriTbl[entry_idx].hiPriRefCnt=0;
	//_rtk_rg_flow_clear();
	TABLE("DEL flowHighPriTbl[%d]",entry_idx);

	return RT_ERR_RG_OK;
}



rtk_rg_err_code_t rtk_rg_apolloPro_funcbasedMeter_set(rtk_rg_funcbasedMeterConf_t meterConf)
{
	int32 hwIdx = FAILED;
	int ret;
	uint32 rate;
	rtk_rg_enable_t ifgInclude;
	char typeName[25];
#if defined(CONFIG_RG_RTL9607C_SERIES)	 || defined(CONFIG_RG_RTL9603CVD_SERIES)
	rtk_rg_enable_t ifReaddfuncEntry = RTK_RG_DISABLED;
#endif

	if(rg_db.systemGlobal.funbasedMeter_mode == RTK_RG_METERMODE_NOT_INIT)
	{
		//The first time to set meter, funbasedMeter enabled.
		rg_db.systemGlobal.funbasedMeter_mode = RTK_RG_METERMODE_SW_INDEX;
	}
	else if(rg_db.systemGlobal.funbasedMeter_mode == RTK_RG_METERMODE_HW_INDEX)
	{
		WARNING("Confilct meter mode. Please use the same API to set/get meter for consistency.");
		RETURN_ERR(RT_ERR_RG_FAILED);
	}

	if(meterConf.idx >= MAX_FUNCBASEDMETER_SIZE)
	{
		WARNING("Invalid funbasedMeter index! The max funcbasedMeter size is %d", MAX_FUNCBASEDMETER_SIZE);
		RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);
	}

	switch(meterConf.type)
	{
		case RTK_RG_METER_ACL:
			strcpy(typeName, "RTK_RG_METER_ACL");
			break;
		case RTK_RG_METER_HOSTPOL:
			strcpy(typeName, "RTK_RG_METER_HOSTPOL");
			break;
		case RTK_RG_METER_STORMCTL:
			strcpy(typeName, "RTK_RG_METER_STORMCTL");
			break;
		case RTK_RG_METER_PROC:
			strcpy(typeName, "RTK_RG_METER_PROC");
			break;
		default:
			WARNING("Invalid funbasedMeter type!!");
			RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	}


	if(meterConf.state == RTK_RG_DISABLED)
	{
		hwIdx = rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].hwIdx;

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
		//Reset its HW meter to default value
		if((meterConf.type == RTK_RG_METER_HOSTPOL) || (meterConf.type == RTK_RG_METER_STORMCTL) || (meterConf.type == RTK_RG_METER_PROC))
		{
			if(hwIdx != FAILED)
			{
				rg_db.systemGlobal.hw_shareMeterUsedState[hwIdx].used = FALSE;
				//rg_db.hw_shareMeterUsedState[hwIdx].usedFuncbasedMeterType = meterConf.type;
				rtk_rate_shareMeter_set(hwIdx, MAX_SWITCH_METER_RATE, ENABLE);
			}
		}
		else if(meterConf.type == RTK_RG_METER_ACL)
		{
			if(hwIdx != FAILED)
				rtk_rg_asic_shareMeter_set(hwIdx, MAX_SWITCH_METER_RATE, ENABLE);
		}
#elif defined(CONFIG_RG_G3_SERIES)
		if(hwIdx != FAILED)
		{
			//Reset its HW meter to default value. (G3 ifgInclude is a golbal setting, thus keep its status)
			if((ret = rtk_rate_shareMeter_get(hwIdx, &rate, (void*)&ifgInclude)) != RT_ERR_OK)
				WARNING("Get funcBased meter[%s][%d] failed, ret = %d", typeName, meterConf.idx, ret);
			rtk_rate_shareMeter_set(hwIdx, MAX_SWITCH_METER_RATE, ifgInclude); /*Set to l2 policer*/
			_rtk_rg_fc_g3L3Policer_set(hwIdx, MAX_SWITCH_METER_RATE, ifgInclude);
		}
#endif
		hwIdx = FAILED;
	}
	else
	{	//meterConf.state == RTK_RG_ENABLED
		if(rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].state)
			hwIdx = rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].hwIdx; //current funcbased Meter has been already set to hardware

#if defined(CONFIG_RG_RTL9607C_SERIES)	|| defined(CONFIG_RG_RTL9603CVD_SERIES)
		if(meterConf.type == RTK_RG_METER_ACL)
		{
			//ACL: use L34 FB meter (total 32 entry, 1-to-1 mapping)
			if(hwIdx == FAILED)
				hwIdx = meterConf.idx;

			//set to FB meter
			ret = rtk_rg_asic_shareMeter_set(hwIdx, meterConf.rate, meterConf.ifgInclude);
			if(ret != RT_ERR_OK)
			{
				rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].state = RTK_RG_DISABLED;
				rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].hwIdx = FAIL;
				WARNING("Set funbasedMeter to HW failed!");
				if(ret == RT_ERR_FAILED){ RETURN_ERR(RT_ERR_RG_SHAREMETER_SET_FAILED);}
				else if(ret == RT_ERR_RG_INDEX_OUT_OF_RANGE){ WARNING("hw_Index = %d", hwIdx); RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);}
				else if(ret == RT_ERR_RG_INVALID_PARAM){ RETURN_ERR(RT_ERR_RG_INVALID_PARAM);}
				else if(ret == RT_ERR_RG_NULL_POINTER){ RETURN_ERR(RT_ERR_RG_NULL_POINTER);}
				else{ RETURN_ERR(RT_ERR_RG_SHAREMETER_SET_FAILED);}
			}
			{
				ret = rtk_rg_asic_shareMeter_get(hwIdx, &rate , (void*)&ifgInclude);
				if(ret != RT_ERR_OK)
				{
					rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].state = RTK_RG_DISABLED;
					rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].hwIdx = FAIL;
					WARNING("Get funbasedMeter from HW failed!");
					if(ret == RT_ERR_RG_INDEX_OUT_OF_RANGE){ WARNING("hw_Index = %d", hwIdx); RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);}
					if(ret == RT_ERR_RG_NULL_POINTER){ RETURN_ERR(RT_ERR_RG_NULL_POINTER);}
					else{RETURN_ERR(RT_ERR_RG_SHAREMETER_GET_FAILED);}
				}
				rtlglue_printf("funcbasedMeter[%d](type=%s, rate=%d, ifgInclude=%d) !\n", meterConf.idx, typeName, rate, ifgInclude);
			}
			TRACE("funcbased[%s][idx].idx = %d ","RTK_RG_METER_ACL", meterConf.idx, hwIdx);
		}
		else if((meterConf.type == RTK_RG_METER_HOSTPOL) || (meterConf.type == RTK_RG_METER_STORMCTL) || (meterConf.type == RTK_RG_METER_PROC))
		{
			//L2 share meter are shared with host policing and storm control (total 48 entry, not 1-to-1 mapping.)
			if(hwIdx == FAILED)
			{
				hwIdx = _rtk_rg_hw_funcbasedMeter_hwIdx_get();

				if(hwIdx == FAILED)
				{	// all switch meter are used
					rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].state = RTK_RG_DISABLED;
					rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].hwIdx = FAIL;
					WARNING("No free switch sharemeter!");
					return(RT_ERR_RG_FAILED);
				}
			}

			//set to L2 share meter
			ret = rtk_rate_shareMeter_set(hwIdx, meterConf.rate, meterConf.ifgInclude);

			if(ret == RT_ERR_OK)
			{
				rg_db.systemGlobal.hw_shareMeterUsedState[hwIdx].used = TRUE;
			}
			else
			{
				rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].state = RTK_RG_DISABLED;
				rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].hwIdx = FAIL;
				WARNING("Set funbasedMeter to HW failed!");
				if(ret == RT_ERR_FAILED){ RETURN_ERR(RT_ERR_RG_SHAREMETER_SET_FAILED);}
				else if(ret == RT_ERR_FILTER_METER_ID){ WARNING("hw_Index = %d", hwIdx); RETURN_ERR(RT_ERR_RG_SHAREMETER_INVALID_METER_INDEX);}
				else if(ret == RT_ERR_RATE){ RETURN_ERR(RT_ERR_RG_SHAREMETER_INVALID_RATE);}
				else if(ret == RT_ERR_INPUT){ RETURN_ERR(RT_ERR_RG_SHAREMETER_INVALID_INPUT);}
				else{  RETURN_ERR(RT_ERR_RG_SHAREMETER_SET_FAILED);}
			}
			{
				ret = rtk_rate_shareMeter_get(hwIdx, &rate , (void*)&ifgInclude);

				if(ret != RT_ERR_OK)
				{
					rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].state = RTK_RG_DISABLED;
					rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].hwIdx = FAIL;
					rg_db.systemGlobal.hw_shareMeterUsedState[hwIdx].used = FALSE;
					WARNING("Get funbasedMeter from HW failed!");

					if(ret == RT_ERR_FILTER_METER_ID){ WARNING("hw_Index = %d\n", hwIdx); RETURN_ERR(RT_ERR_RG_SHAREMETER_INVALID_METER_INDEX);}
					else {RETURN_ERR(RT_ERR_RG_SHAREMETER_GET_FAILED);}
				}
				rtlglue_printf("funcbasedMeter[%d](type=%s, rate=%d, ifgInclude=%d) !\n", meterConf.idx, typeName, rate, ifgInclude);
				TRACE("funcbased[%s][idx].idx = %d ", typeName, meterConf.idx, hwIdx);
			}

			//the funcbased meter was truned to enable from disable, to preventing the inconsistency of hwIdx, readd the host policing/storm control index
			if(rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].state == RTK_RG_DISABLED)
				ifReaddfuncEntry = RTK_RG_ENABLED;

		}
#elif defined(CONFIG_RG_G3_SERIES)
		if(hwIdx == FAILED)
		{
			//set hwIdx to flow policer ID (each function has 32 entries, 1-to-1 mapping)
			if(meterConf.type == RTK_RG_METER_ACL)
				hwIdx = meterConf.idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMTR;
			else if(meterConf.type == RTK_RG_METER_HOSTPOL)
				hwIdx = meterConf.idx + G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_RX;
			else if(meterConf.type == RTK_RG_METER_STORMCTL)
				hwIdx = meterConf.idx + G3_FLOW_POLICER_IDXSHIFT_STORMCTL;
			else if(meterConf.type == RTK_RG_METER_PROC)
				hwIdx = meterConf.idx + G3_FLOW_POLICER_IDXSHIFT_PROC;
		}
		//set to meter
		ret = rtk_rate_shareMeter_set(hwIdx, meterConf.rate, meterConf.ifgInclude);/*set to L2 policer*/

		if(ret != RT_ERR_OK)
		{
			rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].state = RTK_RG_DISABLED;
			rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].hwIdx = FAIL;
			WARNING("Set funbasedMeter to HW failed!");
			if(ret == RT_ERR_FAILED){ RETURN_ERR(RT_ERR_RG_SHAREMETER_SET_FAILED);}
			else if(ret == RT_ERR_FILTER_METER_ID){ WARNING("hw_Index = %d", hwIdx); RETURN_ERR(RT_ERR_RG_SHAREMETER_INVALID_METER_INDEX);}
			else if(ret == RT_ERR_RATE){ RETURN_ERR(RT_ERR_RG_SHAREMETER_INVALID_RATE);}
			else if(ret == RT_ERR_INPUT){ RETURN_ERR(RT_ERR_RG_SHAREMETER_INVALID_INPUT);}
			else{  RETURN_ERR(RT_ERR_RG_SHAREMETER_SET_FAILED);}
		}

		ret = _rtk_rg_fc_g3L3Policer_set(hwIdx, meterConf.rate, meterConf.ifgInclude);/*set to L3 policer*/

		if(ret != RT_ERR_OK)
		{
			rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].state = RTK_RG_DISABLED;
			rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].hwIdx = FAIL;
			WARNING("Set funbasedMeter to HW failed!");
			if(ret == RT_ERR_FAILED){ RETURN_ERR(RT_ERR_RG_SHAREMETER_SET_FAILED);}
			else if(ret == RT_ERR_FILTER_METER_ID){ WARNING("hw_Index = %d", hwIdx); RETURN_ERR(RT_ERR_RG_SHAREMETER_INVALID_METER_INDEX);}
			else if(ret == RT_ERR_RATE){ RETURN_ERR(RT_ERR_RG_SHAREMETER_INVALID_RATE);}
			else if(ret == RT_ERR_INPUT){ RETURN_ERR(RT_ERR_RG_SHAREMETER_INVALID_INPUT);}
			else{  RETURN_ERR(RT_ERR_RG_SHAREMETER_SET_FAILED);}
		}
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
		if(meterConf.type == RTK_RG_METER_HOSTPOL)
		{
			hwIdx = meterConf.idx + G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_TX;
		}
		//set to meter
		ret = rtk_rate_shareMeter_set(hwIdx, meterConf.rate, meterConf.ifgInclude);/*set to L2 policer*/

		if(ret != RT_ERR_OK)
		{
			rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].state = RTK_RG_DISABLED;
			rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].hwIdx = FAIL;
			WARNING("Set funbasedMeter to HW failed!");
			if(ret == RT_ERR_FAILED){ RETURN_ERR(RT_ERR_RG_SHAREMETER_SET_FAILED);}
			else if(ret == RT_ERR_FILTER_METER_ID){ WARNING("hw_Index = %d", hwIdx); RETURN_ERR(RT_ERR_RG_SHAREMETER_INVALID_METER_INDEX);}
			else if(ret == RT_ERR_RATE){ RETURN_ERR(RT_ERR_RG_SHAREMETER_INVALID_RATE);}
			else if(ret == RT_ERR_INPUT){ RETURN_ERR(RT_ERR_RG_SHAREMETER_INVALID_INPUT);}
			else{  RETURN_ERR(RT_ERR_RG_SHAREMETER_SET_FAILED);}
		}

		ret = _rtk_rg_fc_g3L3Policer_set(hwIdx, meterConf.rate, meterConf.ifgInclude);/*set to L3 policer*/

		if(ret != RT_ERR_OK)
		{
			rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].state = RTK_RG_DISABLED;
			rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].hwIdx = FAIL;
			WARNING("Set funbasedMeter to HW failed!");
			if(ret == RT_ERR_FAILED){ RETURN_ERR(RT_ERR_RG_SHAREMETER_SET_FAILED);}
			else if(ret == RT_ERR_FILTER_METER_ID){ WARNING("hw_Index = %d", hwIdx); RETURN_ERR(RT_ERR_RG_SHAREMETER_INVALID_METER_INDEX);}
			else if(ret == RT_ERR_RATE){ RETURN_ERR(RT_ERR_RG_SHAREMETER_INVALID_RATE);}
			else if(ret == RT_ERR_INPUT){ RETURN_ERR(RT_ERR_RG_SHAREMETER_INVALID_INPUT);}
			else{  RETURN_ERR(RT_ERR_RG_SHAREMETER_SET_FAILED);}
		}
#endif
		{
			ret = rtk_rate_shareMeter_get(hwIdx, &rate , (void*)&ifgInclude);

			if(ret != RT_ERR_OK)
			{
				rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].state = RTK_RG_DISABLED;
				rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].hwIdx = FAIL;
				WARNING("Get funbasedMeter from HW failed!");

				if(ret == RT_ERR_FILTER_METER_ID){ WARNING("hw_Index = %d\n", hwIdx); RETURN_ERR(RT_ERR_RG_SHAREMETER_INVALID_METER_INDEX);}
				else {RETURN_ERR(RT_ERR_RG_SHAREMETER_GET_FAILED);}
			}
			rtlglue_printf("funcbasedMeter[%d](type=%s, rate=%d, ifgInclude=%d) !\n", meterConf.idx, typeName, rate, ifgInclude);
			TRACE("funcbased[%s][idx].idx = %d ", typeName, meterConf.idx, hwIdx);
		}
#endif

	}
	rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].state = meterConf.state;
	rg_db.systemGlobal.funbasedMeter[meterConf.type][meterConf.idx].hwIdx = hwIdx;
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	if(ifReaddfuncEntry)
		_rtk_rg_hostPolicongAndStormControlAndResAcl_readdByFuncbasedMeter(meterConf.type, meterConf.idx);
#endif
	return RT_ERR_RG_OK;

}

rtk_rg_err_code_t rtk_rg_apolloPro_funcbasedMeter_get(rtk_rg_funcbasedMeterConf_t *meterConf)
{
	int32 hwIdx;
	int ret;
	uint32 rate;
	rtk_rg_enable_t ifgInclude;

	if(rg_db.systemGlobal.funbasedMeter_mode == RTK_RG_METERMODE_HW_INDEX)
	{
		WARNING("Confilct meter mode. Please use the same API to set/get meter for consistency.");
		RETURN_ERR(RT_ERR_RG_FAILED);
	}

	if(meterConf->type >= RTK_RG_METER_MAX)
	{
		WARNING("Invalid funbasedMeter type!!");
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	}

	if(meterConf->idx >= MAX_FUNCBASEDMETER_SIZE)
	{
		WARNING("Invalid funbasedMeter index! The max funcbasedMeter size is %d", MAX_FUNCBASEDMETER_SIZE);
		RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);
	}


	meterConf->state = rg_db.systemGlobal.funbasedMeter[meterConf->type][meterConf->idx].state;

	if(meterConf->state != RTK_RG_DISABLED)
	{
		hwIdx = rg_db.systemGlobal.funbasedMeter[meterConf->type][meterConf->idx].hwIdx;

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
		if(meterConf->type == RTK_RG_METER_ACL)
		{
			ret = rtk_rg_asic_shareMeter_get(hwIdx, &rate , (void*)&ifgInclude);
			if(ret != RT_ERR_OK)
			{
				WARNING("Get funbasedMeter from HW failed!");
				if(ret == RT_ERR_RG_INDEX_OUT_OF_RANGE){ WARNING("hw_Index = %d", hwIdx); RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);}
				if(ret == RT_ERR_RG_NULL_POINTER){ RETURN_ERR(RT_ERR_RG_NULL_POINTER);}
				else{RETURN_ERR(RT_ERR_RG_SHAREMETER_GET_FAILED);}
			}
			meterConf->rate = rate;
			meterConf->ifgInclude = ifgInclude;
		}
		else if((meterConf->type == RTK_RG_METER_HOSTPOL) || (meterConf->type == RTK_RG_METER_STORMCTL) || (meterConf->type == RTK_RG_METER_PROC))
		{
			ret = rtk_rate_shareMeter_get(hwIdx, &rate , (void*)&ifgInclude);

			if(ret != RT_ERR_OK)
			{
				WARNING("Get funbasedMeter from HW failed!");

				if(ret == RT_ERR_FILTER_METER_ID){ WARNING("hw_Index = %d\n", hwIdx); RETURN_ERR(RT_ERR_RG_SHAREMETER_INVALID_METER_INDEX);}
				else {RETURN_ERR(RT_ERR_RG_SHAREMETER_GET_FAILED);}
			}
			meterConf->rate = rate;
			meterConf->ifgInclude = ifgInclude;
		}
#elif defined(CONFIG_RG_G3_SERIES)
		ret = rtk_rate_shareMeter_get(hwIdx, &rate , (void*)&ifgInclude);

		if(ret != RT_ERR_OK)
		{
			WARNING("Get funbasedMeter from HW failed!");

			if(ret == RT_ERR_FILTER_METER_ID){ WARNING("hw_Index = %d\n", hwIdx); RETURN_ERR(RT_ERR_RG_SHAREMETER_INVALID_METER_INDEX);}
			else {RETURN_ERR(RT_ERR_RG_SHAREMETER_GET_FAILED);}
		}
		meterConf->rate = rate;
		meterConf->ifgInclude = ifgInclude;
#endif
	}

	return (RT_ERR_RG_OK);

}

int _rtk_rg_extraTagActionList_empty_find(void)
{
	/* return 1~7 for action list idx */
	int i = 0;
	for(i = 0; i < MAX_EXTRATAG_TABLE_SIZE; i++)
	{
		if(rg_db.systemGlobal.extraTagActionList_netifIdx[i] == -1)
			return i+1;
	}

	WARNING("NO empty action list could be used!");
	return 0;
}

int _rtk_rg_extraTagActionList_apply(int actionListIdx, int intfIdx)
{
	if((actionListIdx<FLOWBASED_EXTRATAG_LISTMIN) || (actionListIdx>FLOWBASED_EXTRATAG_LISTMAX))
		return FAIL;
	if(rg_db.systemGlobal.extraTagActionList_netifIdx[actionListIdx-1] != -1)
		DEBUG("Try to replace action list, used by intf %d, new intf: %d",rg_db.systemGlobal.extraTagActionList_netifIdx[actionListIdx-1], intfIdx);

	rg_db.systemGlobal.extraTagActionList_netifIdx[actionListIdx-1] = intfIdx;

	return SUCCESS;
}

int _rtk_rg_extraTagActionList_clear(int actionListIdx)
{
	if((actionListIdx<FLOWBASED_EXTRATAG_LISTMIN) || (actionListIdx>FLOWBASED_EXTRATAG_LISTMAX))
		return FAIL;

	rg_db.systemGlobal.extraTagActionList_netifIdx[actionListIdx-1] = -1;

	return SUCCESS;
}

int _rtk_rg_extraTag_setupPPTP(int wan_intf_idx, uint8 *outerHdr, int *extraTagActListIdx)
{
#if !defined(CONFIG_RG_G3_SERIES)
	rtk_rg_asic_extraTagAction_t extraTagAction;
	int extraTagIdx = 0, actSeq = 0;
	int TUNN_tag_len=40;	//IP(20)+GRE(16)+PPP(4) for PPTP

	if(*extraTagActListIdx==0)
	{
		// need to get one available ActionList
		extraTagIdx = _rtk_rg_extraTagActionList_empty_find();
		if(extraTagIdx==0) return FAIL;
	}else{
		// use the force assigned ActionList
		extraTagIdx = *extraTagActListIdx;
	}

	_rtk_rg_extraTagActionList_apply(extraTagIdx, wan_intf_idx);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type1.act_bit = FB_EXTG_ACTBIT_1;
	extraTagAction.type1.length = TUNN_tag_len;
	extraTagAction.type1.src_addr_offset = 0 + (EXTRATAG_BUFFER_OFFSET * (extraTagIdx-1));
	ASSERT_EQ(rtk_rg_asic_extraTagAction_add(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type2.act_bit = FB_EXTG_ACTBIT_2;
	extraTagAction.type2.ethertype = 0x0800;
	ASSERT_EQ(rtk_rg_asic_extraTagAction_add(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type3.act_bit = FB_EXTG_ACTBIT_3;
	extraTagAction.type3.pkt_buff_offset = 2;
	extraTagAction.type3.length = 2;
	extraTagAction.type3.value = TUNN_tag_len;
	extraTagAction.type3.operation = 0;
	ASSERT_EQ(rtk_rg_asic_extraTagAction_add(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type4.act_bit = FB_EXTG_ACTBIT_4;
	extraTagAction.type4.pkt_buff_offset = 4;
	extraTagAction.type4.data_src_type = 1;						// 1:IP ID;
	extraTagAction.type4.seq_ack_reg_idx = wan_intf_idx;
	ASSERT_EQ(rtk_rg_asic_extraTagAction_add(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type5.act_bit = FB_EXTG_ACTBIT_5;
	extraTagAction.type6.pkt_buff_offset = 10;
	ASSERT_EQ(rtk_rg_asic_extraTagAction_add(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type3.act_bit = FB_EXTG_ACTBIT_3;
	extraTagAction.type3.pkt_buff_offset = 24;
	extraTagAction.type3.length = 2;
	extraTagAction.type3.value = 4;
	extraTagAction.type3.operation = 0;
	ASSERT_EQ(rtk_rg_asic_extraTagAction_add(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type4.act_bit = FB_EXTG_ACTBIT_4;
	extraTagAction.type4.pkt_buff_offset = 28;
	extraTagAction.type4.data_src_type = 0;
	extraTagAction.type4.reduce_seq = 0;
	extraTagAction.type4.reduce_ack = 0;
	extraTagAction.type4.seq_ack_reg_idx = wan_intf_idx;
	ASSERT_EQ(rtk_rg_asic_extraTagAction_add(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	ASSERT_EQ(rtk_rg_asic_extraTagAction_add(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	//memDump(outerHdr, TUNN_tag_len, "Content Buffer - Before");
	ASSERT_EQ(rtk_rg_asic_extraTagContentBuffer_set(extraTagIdx, outerHdr), SUCCESS);
	//memDump(outerHdr, TUNN_tag_len, "Content Buffer - After");

	*extraTagActListIdx = extraTagIdx;
#endif //!defined(CONFIG_RG_G3_SERIES)
	return SUCCESS;
}

int _rtk_rg_extraTag_setupL2TP(int wan_intf_idx, uint8 *outerHdr, int *extraTagActListIdx)
{
#if !defined(CONFIG_RG_G3_SERIES)
	rtk_rg_asic_extraTagAction_t extraTagAction;
	int extraTagIdx = 0, actSeq = 0;
	int TUNN_tag_len=40;	//IP(20)+UDP(8)+L2TP(8)+PPP(4) for L2TP

	if(*extraTagActListIdx==0)
	{
		// need to get one available ActionList
		extraTagIdx = _rtk_rg_extraTagActionList_empty_find();
		if(extraTagIdx==0) return FAIL;
	}else{
		// use the force assigned ActionList
		extraTagIdx = *extraTagActListIdx;
	}

	_rtk_rg_extraTagActionList_apply(extraTagIdx, wan_intf_idx);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type1.act_bit = FB_EXTG_ACTBIT_1;
	extraTagAction.type1.length = TUNN_tag_len;
	extraTagAction.type1.src_addr_offset = 0 + (EXTRATAG_BUFFER_OFFSET * (extraTagIdx-1));
	ASSERT_EQ(rtk_rg_asic_extraTagAction_add(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type2.act_bit = FB_EXTG_ACTBIT_2;
	extraTagAction.type2.ethertype = 0x0800;
	ASSERT_EQ(rtk_rg_asic_extraTagAction_add(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type3.act_bit = FB_EXTG_ACTBIT_3;
	extraTagAction.type3.pkt_buff_offset = 2;
	extraTagAction.type3.length = 2;
	extraTagAction.type3.value = TUNN_tag_len;
	extraTagAction.type3.operation = 0;
	ASSERT_EQ(rtk_rg_asic_extraTagAction_add(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type4.act_bit = FB_EXTG_ACTBIT_4;
	extraTagAction.type4.pkt_buff_offset = 4;
	extraTagAction.type4.data_src_type = 1;						// 1:IP ID;
	extraTagAction.type4.seq_ack_reg_idx = wan_intf_idx;
	ASSERT_EQ(rtk_rg_asic_extraTagAction_add(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type5.act_bit = FB_EXTG_ACTBIT_5;
	extraTagAction.type6.pkt_buff_offset = 10;
	ASSERT_EQ(rtk_rg_asic_extraTagAction_add(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type3.act_bit = FB_EXTG_ACTBIT_3;
	extraTagAction.type3.pkt_buff_offset = 24;
	extraTagAction.type3.length = 2;
	extraTagAction.type3.value = TUNN_tag_len-20;		// 20: ipv4 header length
	extraTagAction.type3.operation = 0;
	ASSERT_EQ(rtk_rg_asic_extraTagAction_add(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type3.act_bit = FB_EXTG_ACTBIT_3;
	extraTagAction.type3.pkt_buff_offset = 30;
	extraTagAction.type3.length = 2;
	extraTagAction.type3.value = TUNN_tag_len-20-8;		//20: ipv4 header length; 8: udp header length
	extraTagAction.type3.operation = 0;
	ASSERT_EQ(rtk_rg_asic_extraTagAction_add(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type6.act_bit = FB_EXTG_ACTBIT_6;
	extraTagAction.type6.pkt_buff_offset = 26;
	ASSERT_EQ(rtk_rg_asic_extraTagAction_add(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);


	//memDump(outerHdr, TUNN_tag_len, "Content Buffer - Before");
	ASSERT_EQ(rtk_rg_asic_extraTagContentBuffer_set(extraTagIdx, outerHdr), SUCCESS);
	//memDump(outerHdr, TUNN_tag_len, "Content Buffer - After");

	*extraTagActListIdx = extraTagIdx;
#endif //!defined(CONFIG_RG_G3_SERIES)
	return SUCCESS;
}

int _rtk_rg_extraTag_setupDsLite(int wan_intf_idx, uint8 *outerHdr, int *extraTagActListIdx)
{
#if !defined(CONFIG_RG_G3_SERIES)
	rtk_rg_asic_extraTagAction_t extraTagAction;
	int extraTagIdx = 0, actSeq = 0;
	int TUNN_tag_len=40;	//IPv6(40)

	if(*extraTagActListIdx==0)
	{
		// need to get one available ActionList
		extraTagIdx = _rtk_rg_extraTagActionList_empty_find();
		if(extraTagIdx==0) return FAIL;
	}else{
		// use the force assigned ActionList
		extraTagIdx = *extraTagActListIdx;
	}

	_rtk_rg_extraTagActionList_apply(extraTagIdx, wan_intf_idx);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type1.act_bit = FB_EXTG_ACTBIT_1;
	extraTagAction.type1.length = TUNN_tag_len;
	extraTagAction.type1.src_addr_offset = 0 + (EXTRATAG_BUFFER_OFFSET * (extraTagIdx-1));
	ASSERT_EQ(rtk_rg_asic_extraTagAction_add(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type2.act_bit = FB_EXTG_ACTBIT_2;
	extraTagAction.type2.ethertype = 0x86dd;
	ASSERT_EQ(rtk_rg_asic_extraTagAction_add(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type3.act_bit = FB_EXTG_ACTBIT_3;
	extraTagAction.type3.pkt_buff_offset = 4;
	extraTagAction.type3.length = 2;
	extraTagAction.type3.value = 0;
	extraTagAction.type3.operation = 0;
	ASSERT_EQ(rtk_rg_asic_extraTagAction_add(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	ASSERT_EQ(rtk_rg_asic_extraTagAction_add(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	//memDump(outerHdr, TUNN_tag_len, "Content Buffer - Before");
	ASSERT_EQ(rtk_rg_asic_extraTagContentBuffer_set(extraTagIdx, outerHdr), SUCCESS);
	//memDump(outerHdr, TUNN_tag_len, "Content Buffer - After");

	*extraTagActListIdx = extraTagIdx;
#endif //!defined(CONFIG_RG_G3_SERIES)
	return SUCCESS;
}

rtk_rg_err_code_t _rtk_rg_flow_syncTunnelWANBasedWAN(int tunnelIntfIdx, int baseIntfIdx)
{
	rtk_rg_err_code_t ret = RT_ERR_RG_OK;

	//Check: Some configuration of Tunnel WAN must be synced with Based WAN, thus the egress format could be correct!

	// 1. for HW forwarding, tunnel WAN MAC must be configured as based WAN MAC. (because src IP and src MAC are all decided by egress intf(out_intf_idx).)
	if(memcmp(&rg_db.netif[tunnelIntfIdx].rtk_netif.gateway_mac, &rg_db.netif[baseIntfIdx].rtk_netif.gateway_mac, sizeof(rtk_mac_t)))
	{
		memcpy(&rg_db.netif[tunnelIntfIdx].rtk_netif.gateway_mac, &rg_db.netif[baseIntfIdx].rtk_netif.gateway_mac, sizeof(rtk_mac_t));
		if((ret = RTK_L34_NETIFTABLE_SET(tunnelIntfIdx, &rg_db.netif[tunnelIntfIdx].rtk_netif))!=RT_ERR_RG_OK)
		{
			WARNING("Fail to set netif[%d]", tunnelIntfIdx);
			return (RT_ERR_RG_INTF_SET_FAIL);
		}
	}
	// 2. for HW forwarding, the tunnel WAN must sync with based WAN if pppoe type.
	if(rg_db.systemGlobal.interfaceInfo[baseIntfIdx].storedInfo.wan_intf.wan_intf_conf.wan_type == RTK_RG_PPPoE)
	{
		if((ret = _rtk_rg_netifPPPoESession_set(tunnelIntfIdx, FB_NETIFPPPOE_ACT_ADD, rg_db.systemGlobal.interfaceInfo[baseIntfIdx].storedInfo.wan_intf.pppoe_info.after_dial.sessionId))!=RT_ERR_RG_OK)
		{
			WARNING("Fail to set PPPoE session of netif[%d]", tunnelIntfIdx);
			return (RT_ERR_RG_INTF_SET_FAIL);
		}
	}

	return ret;
}

/*
* To delete PPTP/L2TP/DSLite related configuration
* 1. Extra tag actions and content buffer
* 2. Path6 flow entry
*/
int _rtk_rg_flow_deleteTunnelConfig(int wan_intf_idx)
{
	uint flowIdx;
	rtk_rg_wanIntfInfo_t *pWanIntfInfo = &rg_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf;

	//Delete all tunnel flow entries
	for(flowIdx=0; flowIdx<MAX_FLOW_SW_TABLE_SIZE; flowIdx++)
	{
		if(rg_db.flow[flowIdx].sw_valid==0) 
			continue;
		if((RG_PFLOW(flowIdx)->path1.in_path==FB_PATH_5 && RG_PFLOW(flowIdx)->path5.out_extra_tag_index>0) ||
			RG_PFLOW(flowIdx)->path1.out_intf_idx == wan_intf_idx)
		{
			ASSERT_EQ(_rtk_rg_flow_del(flowIdx, FALSE), RT_ERR_RG_OK);
		}
	}
	_rtk_rg_shortCut_clear();

	// Del flow entra tag action
	if(pWanIntfInfo->extraTagActionListIdx>0)
	{
#if !defined(CONFIG_RG_G3_SERIES)
		ASSERT_EQ(rtk_rg_asic_extraTagAction_del(pWanIntfInfo->extraTagActionListIdx, 0), RT_ERR_RG_OK);
		ASSERT_EQ(_rtk_rg_extraTagActionList_clear(pWanIntfInfo->extraTagActionListIdx), RT_ERR_RG_OK);
		pWanIntfInfo->extraTagActionListIdx=0;
#endif //!defined(CONFIG_RG_G3_SERIES)
		//TODO: clear extra tag content buffer
	}

	// Del flow path6 entry
	if(pWanIntfInfo->flowEntryIdx!=-1)
	{
		_rtk_rg_flow_del(pWanIntfInfo->flowEntryIdx, 0);		// delete and ignore return fail (e.g. flow not found) because flow entry may be deleded already.
		pWanIntfInfo->flowEntryIdx=-1;
	}
	return (RT_ERR_RG_OK);
}

int _rtk_rg_flow_setupPPTP(int wan_intf_idx, int gmacL2Idx)
{
	rtk_rg_err_code_t ret;
	rtk_rg_table_flowEntry_t flowPathEntry;
	rtk_rg_flow_extraInfo_t flowExtraInfo;
	rtk_rg_asic_path6_entry_t *flowPath6 = &flowPathEntry.path6;
	rtk_rg_ipPPTPClientInfo_t *pPPTPInfo = NULL;
	rtk_rg_wanIntfInfo_t *pWanIntfInfo = NULL;
	uint32 flow_idx;
	int32 baseIntfIdx = -1, extraTagActListIdx = 0;

	if(rg_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.is_wan &&
		rg_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type == RTK_RG_PPTP)
		TRACE("prepare PPTP tag");
	else
		WARNING("prepare PPTP tag but assigned intf idx doesn't belong to PPTP");

	pWanIntfInfo=&rg_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf;
	pPPTPInfo=&pWanIntfInfo->pptp_info;

	if(_rtk_rg_lookupBasedWANForSourceIP(&baseIntfIdx, pPPTPInfo->before_dial.pptp_ipv4_addr)==RG_FWDENGINE_RET_ERROR){
		TRACE("based WAN do not exist! FAIL!");
		RETURN_ERR(RT_ERR_RG_ENTRY_NOT_EXIST);
	}
	pWanIntfInfo->baseIntf_idx = baseIntfIdx;

	if((ret = _rtk_rg_flow_syncTunnelWANBasedWAN(wan_intf_idx, baseIntfIdx))!=RT_ERR_RG_OK)
		RETURN_ERR(ret);

	//1 1: Add flow extra tag
	{
		// Generate IPv4+GRE+PPP header
		u8 contenBuffer[64]={0};
		u8 *outerHdr = &contenBuffer[0];
		TRACE("Gen outer header and add to SRAM content buffer");

		// Prepare IP(20)+GRE(16)+PPP(4) for PPTP
		// L3 header --
		{
			*((uint16 *)(outerHdr + 0)) = htons(0x4500);	// ver, IHL
			//*((uint16 *)(outerHdr + 2)) = htons(0x0);	// total length	: updated by extra tag actions
			//*((uint16 *)(outerHdr + 4)) = htons(0x0);	// ipid		: updated by extra tag actions
			//*((uint16 *)(outerHdr + 6)) = htons(0x0);	// flags, frag offset
			*((uint16 *)(outerHdr + 8)) = htons(0x402f);	// ttl, protocol
			//*((uint16 *)(outerHdr + 10)) = htons(0x0);	// hdr checksum
			*((uint32 *)(outerHdr + 12)) = htonl(rg_db.netif[baseIntfIdx].rtk_netif.ipAddr);	// src ip
			*((uint32 *)(outerHdr + 16)) = htonl(pPPTPInfo->before_dial.pptp_ipv4_addr);	// dst ip
		}

		// GRE --
		*(uint32 *)(outerHdr + 20) = htonl(0x3081880b);								//Flags, version, Protocol Type=PPP
		//*(uint16 *)(outerHdr + 24) = htons(0x0);									//Payload length
		*(uint16 *)(outerHdr + 26) = htons(pPPTPInfo->after_dial.gateway_callId);			//Peer CallID
		//*(uint32 *)(outerHdr + 28) = htonl(0x0);										//Sequence Num
		//*(uint32 *)(outerHdr + 32) = htonl(0x0);										//Acknowledgment Num

		// PPP --
		{
			*(uint32 *)(outerHdr + 36) = htonl(0xff030021);							// Address, Control, Protocol=IPv4
		}

		extraTagActListIdx = pWanIntfInfo->extraTagActionListIdx;	// use the previous used list if readd.
		if((ret = _rtk_rg_extraTag_setupPPTP(wan_intf_idx, outerHdr, &extraTagActListIdx)) == SUCCESS)
			pWanIntfInfo->extraTagActionListIdx = extraTagActListIdx;

	}

	//1 2: Add flow path6
	bzero(flowPath6, sizeof(rtk_rg_asic_path6_entry_t));
	flowPath6->in_path = FB_PATH_6;
	flowPath6->in_src_ipv4_addr = pPPTPInfo->before_dial.pptp_ipv4_addr;
	flowPath6->in_dst_ipv4_addr = rg_db.netif[baseIntfIdx].rtk_netif.ipAddr;
	flowPath6->in_intf_idx = wan_intf_idx;
	flowPath6->in_ctagif = pWanIntfInfo->wan_intf_conf.egress_vlan_tag_on;
	flowPath6->in_stagif = 0;																				// FIXME, tagif or not
	flowPath6->in_pppoeif= (rg_db.systemGlobal.interfaceInfo[baseIntfIdx].storedInfo.wan_intf.wan_intf_conf.wan_type == RTK_RG_PPPoE) ? TRUE : FALSE;
	flowPath6->in_pppoe_sid = (rg_db.systemGlobal.interfaceInfo[baseIntfIdx].storedInfo.wan_intf.wan_intf_conf.wan_type == RTK_RG_PPPoE)? rg_db.systemGlobal.interfaceInfo[baseIntfIdx].storedInfo.wan_intf.pppoe_info.after_dial.sessionId : 0;
	flowPath6->in_smac_lut_idx = gmacL2Idx;
	flowPath6->in_dmac_lut_idx = rg_db.netif[baseIntfIdx].l2_idx;
	flowPath6->in_pptpif = TRUE;
	flowPath6->in_gre_call_id_check = TRUE;
	flowPath6->in_gre_call_id = pPPTPInfo->after_dial.callId;
	flowPath6->in_protocol = FB_INPROTOCOL_ALL_ACCEPT;


	memset(&flowExtraInfo, 0, sizeof(rtk_rg_flow_extraInfo_t));
	flowExtraInfo.igrSVID = 0;
	flowExtraInfo.igrSPRI = 0;
	flowExtraInfo.igrCVID = flowPath6->in_ctagif ? pWanIntfInfo->wan_intf_conf.egress_vlan_id : 0;
	flowExtraInfo.lutSaIdx = gmacL2Idx;
	flowExtraInfo.lutDaIdx = flowPath6->in_dmac_lut_idx;
	flowExtraInfo.path34_isGmac = 0;
	flowExtraInfo.naptOrTcpUdpGroupIdx = FAIL;
	flowExtraInfo.arpOrNeighborIdx_src = FAIL;
	flowExtraInfo.arpOrNeighborIdx_dst = FAIL;
	flowExtraInfo.staticEntry = 1;
	flowExtraInfo.addSwOnly = 0;
	flowExtraInfo.byDmac2cvid = 0;
	flowExtraInfo.pNaptFilterInfo = NULL;
	flowExtraInfo.pIgrACLFilterInfo = NULL;
	flowExtraInfo.pEgrACLFilterInfo = NULL;
	flowExtraInfo.tinyAckStreamId = -1;
	ret = _rtk_rg_flow_add(&flow_idx, FAIL, &flowPathEntry, flowExtraInfo);

	pWanIntfInfo->flowEntryIdx = flow_idx;

	//1 3: GRE SEQ starts from 1
#if !defined(CONFIG_RG_G3_SERIES)
	if((ret = rtk_rg_asic_dualHdrInfo_set(FB_DUALHDR_GRESEQ, wan_intf_idx, 1)) != RT_ERR_RG_OK)
		WARNING("ASIC API to set gre sequence was fail");
#endif //!defined(CONFIG_RG_G3_SERIES)

	return ret;
}


int _rtk_rg_flow_setupL2TP(int wan_intf_idx, int gmacL2Idx)
{
	rtk_rg_err_code_t ret;
	rtk_rg_table_flowEntry_t flowPathEntry;
	rtk_rg_flow_extraInfo_t flowExtraInfo;
	rtk_rg_asic_path6_entry_t *flowPath6 = &flowPathEntry.path6;
	rtk_rg_ipL2TPClientInfo_t *pL2TPInfo = NULL;
	rtk_rg_wanIntfInfo_t *pWanIntfInfo = NULL;
	uint32 flow_idx;
	int32 baseIntfIdx = -1, extraTagActListIdx = 0;

	if(rg_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.is_wan &&
		rg_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type == RTK_RG_L2TP)
		TRACE("prepare L2TP tag");
	else
		WARNING("prepare L2TP tag but assigned intf idx doesn't belong to L2TP");

	pWanIntfInfo=&rg_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf;
	pL2TPInfo=&pWanIntfInfo->l2tp_info;

	if(_rtk_rg_lookupBasedWANForSourceIP(&baseIntfIdx, pL2TPInfo->before_dial.l2tp_ipv4_addr)==RG_FWDENGINE_RET_ERROR){
		TRACE("based WAN do not exist! FAIL!");
		return RT_ERR_RG_ENTRY_NOT_EXIST;
	}
	pWanIntfInfo->baseIntf_idx = baseIntfIdx;

	if((ret = _rtk_rg_flow_syncTunnelWANBasedWAN(wan_intf_idx, baseIntfIdx))!=RT_ERR_RG_OK)
		RETURN_ERR(ret);

	//1 1: Add flow extra tag
	{
		// Generate IPv4+UDP+L2TP header
		u8 contenBuffer[64]={0};
		u8 *outerHdr = &contenBuffer[0];
		TRACE("Gen outer header and add to SRAM content buffer");

		// Prepare IP(20)+UDP(8)+L2TP(8)+PPP(4) for L2TP
		// L3 header --
		{
			*((uint16 *)(outerHdr + 0)) = htons(0x4500);	// ver, IHL
			//*((uint16 *)(outerHdr + 2)) = htons(0x0);	// total length	: updated by extra tag actions
			//*((uint16 *)(outerHdr + 4)) = htons(0x0);	// ipid		: updated by extra tag actions
			//*((uint16 *)(outerHdr + 6)) = htons(0x0);	// flags, frag offset
			*((uint16 *)(outerHdr + 8)) = htons(0x4011);	// ttl, protocol
			//*((uint16 *)(outerHdr + 10)) = htons(0x0);	// hdr checksum
			*((uint32 *)(outerHdr + 12)) = htonl(rg_db.netif[baseIntfIdx].rtk_netif.ipAddr);	// src ip
			*((uint32 *)(outerHdr + 16)) = htonl(pL2TPInfo->before_dial.l2tp_ipv4_addr);	// dst ip
		}
		// L4 UDP --
		{
			*(uint16 *)(outerHdr + 20) = htons(pL2TPInfo->after_dial.outer_port);	// src port
			*(uint16 *)(outerHdr + 22) = htons(pL2TPInfo->after_dial.gateway_outer_port);	// dst port
			//*(uint16 *)(outerHdr + 24) = htons(0x0);	// len: updated by extra tag actions
			//*(uint16 *)(outerHdr + 26) = htons(0x0);	// checksum: updated by extra tag actions
		}
		// L2TP --
		{
			*(uint16 *)(outerHdr + 28) = htons(0x4002);	// Type(0), Length(1), Sequence(0), Offset(0), Priority(0), Version(2)
			//*(uint16 *)(outerHdr + 30) = htons(0x0);	//Total length: updated by extra tag actions
			*(uint16 *)(outerHdr + 32) = htons(pL2TPInfo->after_dial.gateway_tunnelId);	//Peer's Tunnel ID
			*(uint16 *)(outerHdr + 34) = htons(pL2TPInfo->after_dial.gateway_sessionId);	//Peer's Session ID
		}
		// PPP --
		{
			*(uint32 *)(outerHdr + 36) = htonl(0xff030021);	// Address, Control, Protocol=IPv4
		}

		extraTagActListIdx = pWanIntfInfo->extraTagActionListIdx;	// use the previous used list if readd.
		if((ret = _rtk_rg_extraTag_setupL2TP(wan_intf_idx, outerHdr, &extraTagActListIdx)) == SUCCESS)
			pWanIntfInfo->extraTagActionListIdx = extraTagActListIdx;

	}

	//1 2: Add flow path6
	bzero(flowPath6, sizeof(rtk_rg_asic_path6_entry_t));
	flowPath6->in_path = FB_PATH_6;
	flowPath6->in_src_ipv4_addr = pL2TPInfo->before_dial.l2tp_ipv4_addr;
	flowPath6->in_dst_ipv4_addr = rg_db.netif[baseIntfIdx].rtk_netif.ipAddr;
	flowPath6->in_l2tp_tunnel_id = pL2TPInfo->after_dial.tunnelId;
	flowPath6->in_l2tp_session_id = pL2TPInfo->after_dial.sessionId;
	flowPath6->in_l4_src_port = pL2TPInfo->after_dial.gateway_outer_port;
	flowPath6->in_l4_dst_port = pL2TPInfo->after_dial.outer_port;
	flowPath6->in_intf_idx = wan_intf_idx;
	flowPath6->in_ctagif = pWanIntfInfo->wan_intf_conf.egress_vlan_tag_on;
	flowPath6->in_stagif = 0;																				// FIXME, tagif or not
	flowPath6->in_pppoeif= (rg_db.systemGlobal.interfaceInfo[baseIntfIdx].storedInfo.wan_intf.wan_intf_conf.wan_type == RTK_RG_PPPoE) ? TRUE : FALSE;
	flowPath6->in_pppoe_sid = (rg_db.systemGlobal.interfaceInfo[baseIntfIdx].storedInfo.wan_intf.wan_intf_conf.wan_type == RTK_RG_PPPoE)? rg_db.systemGlobal.interfaceInfo[baseIntfIdx].storedInfo.wan_intf.pppoe_info.after_dial.sessionId : 0;
	flowPath6->in_smac_lut_idx = gmacL2Idx;
	flowPath6->in_dmac_lut_idx = rg_db.netif[baseIntfIdx].l2_idx;
	flowPath6->in_l2tpif = TRUE;
	flowPath6->in_protocol = FB_INPROTOCOL_ALL_ACCEPT;

	memset(&flowExtraInfo, 0, sizeof(rtk_rg_flow_extraInfo_t));
	flowExtraInfo.igrSVID = 0;
	flowExtraInfo.igrSPRI = 0;
	flowExtraInfo.igrCVID = flowPath6->in_ctagif ? pWanIntfInfo->wan_intf_conf.egress_vlan_id : 0;
	flowExtraInfo.lutSaIdx = gmacL2Idx;
	flowExtraInfo.lutDaIdx = flowPath6->in_dmac_lut_idx;
	flowExtraInfo.path34_isGmac = 0;
	flowExtraInfo.naptOrTcpUdpGroupIdx = FAIL;
	flowExtraInfo.arpOrNeighborIdx_src = FAIL;
	flowExtraInfo.arpOrNeighborIdx_dst = FAIL;
	flowExtraInfo.staticEntry = 1;
	flowExtraInfo.addSwOnly = 0;
	flowExtraInfo.byDmac2cvid = 0;
	flowExtraInfo.pNaptFilterInfo = NULL;
	flowExtraInfo.pIgrACLFilterInfo = NULL;
	flowExtraInfo.pEgrACLFilterInfo = NULL;
	flowExtraInfo.tinyAckStreamId = -1;
	ret = _rtk_rg_flow_add(&flow_idx, FAIL, &flowPathEntry, flowExtraInfo);

	pWanIntfInfo->flowEntryIdx = flow_idx;

	return ret;
}

int _rtk_rg_flow_setupDSLite(int wan_intf_idx, int gmacL2Idx, uint8 overPPPoE)
{
	rtk_rg_err_code_t ret;
	rtk_rg_table_flowEntry_t flowPathEntry;
	rtk_rg_flow_extraInfo_t flowExtraInfo;
	rtk_rg_asic_path6_entry_t *flowPath6 = &flowPathEntry.path6;
	rtk_rg_ipDslitStaticInfo_t *pDSLiteInfo = NULL;
	rtk_rg_ipPppoeDsliteInfo_t *pPPPoEDSLiteInfo = NULL;
	rtk_rg_wanIntfInfo_t *pWanIntfInfo = NULL;
	uint32 flow_idx;
	int32 extraTagActListIdx = 0;
	uint32 ihl_tc_flow = 0x60000000;	// version: 6

	if(rg_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.is_wan)
	{
		if((overPPPoE && rg_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type == RTK_RG_PPPoE_DSLITE)
			|| (!overPPPoE && rg_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type == RTK_RG_DSLITE)
			)
			TRACE("prepare %sDsLite tag", overPPPoE?"PPPoE ":"");
	}
	else
		WARNING("prepare DSLite tag but assigned intf idx doesn't belong to DSLite");

	pWanIntfInfo=&rg_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf;

	//clear old tunnel hw info
	ret = _rtk_rg_flow_deleteTunnelConfig(wan_intf_idx);
	if(ret!=RT_ERR_RG_OK) return ret;

	//1 1: Add flow extra tag
	{
		// Generate IPv4+UDP+L2TP header
		u8 contenBuffer[64]={0};
		u8 *outerHdr = &contenBuffer[0];
		TRACE("Gen outer header and add to SRAM content buffer");

		// Prepare IP(20)+UDP(8)+L2TP(8)+PPP(4) for L2TP
		// L3 header --
		if(!overPPPoE)
		{
			pDSLiteInfo=&pWanIntfInfo->dslite_info;

			if(pDSLiteInfo->rtk_dslite.tcOpt == RTK_L34_DSLITE_TC_OPT_ASSIGN)
				ihl_tc_flow |= (pDSLiteInfo->rtk_dslite.tc << 20);
			ihl_tc_flow |= pDSLiteInfo->rtk_dslite.flowLabel;
			*(uint32 *)(outerHdr + 0) = htonl(ihl_tc_flow);									//ver, tc, flowlabel
			//*(uint16 *)(pData + 4) = htons(0x0);										//Payload length: updated by extra tag actions
			*(uint16 *)(outerHdr + 6) = htons(pDSLiteInfo->rtk_dslite.hopLimit|0x0400);			//Next header(IPIP:4), Hop limit
			memcpy(outerHdr + 8, pDSLiteInfo->rtk_dslite.ipB4.ipv6_addr, IPV6_ADDR_LEN);	// src ip
			memcpy(outerHdr + 24, pDSLiteInfo->rtk_dslite.ipAftr.ipv6_addr, IPV6_ADDR_LEN);	// dst ip
		}
		else // overPPPoE
		{
			pPPPoEDSLiteInfo = &pWanIntfInfo->pppoe_dslite_info;

			if(pPPPoEDSLiteInfo->after_dial.dslite_hw_info.rtk_dslite.tcOpt == RTK_L34_DSLITE_TC_OPT_ASSIGN)
				ihl_tc_flow |= (pPPPoEDSLiteInfo->after_dial.dslite_hw_info.rtk_dslite.tc << 20);
			ihl_tc_flow |= pPPPoEDSLiteInfo->after_dial.dslite_hw_info.rtk_dslite.flowLabel;
			*(uint32 *)(outerHdr + 0) = htonl(ihl_tc_flow);									//ver, tc, flowlabel
			//*(uint16 *)(pData + 4) = htons(0x0);										//Payload length: updated by extra tag actions
			*(uint16 *)(outerHdr + 6) = htons(pPPPoEDSLiteInfo->after_dial.dslite_hw_info.rtk_dslite.hopLimit|0x0400);		//Next header(IPIP:4), Hop limit
			memcpy(outerHdr + 8, pPPPoEDSLiteInfo->after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr, IPV6_ADDR_LEN);	// src ip
			memcpy(outerHdr + 24, pPPPoEDSLiteInfo->after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr, IPV6_ADDR_LEN);	// dst ip
		}

		extraTagActListIdx = pWanIntfInfo->extraTagActionListIdx;	// use the previous used list if readd.
		if((ret = _rtk_rg_extraTag_setupDsLite(wan_intf_idx, outerHdr, &extraTagActListIdx)) == SUCCESS)
			pWanIntfInfo->extraTagActionListIdx = extraTagActListIdx;
	}

	//1 2: Add flow path6
	bzero(flowPath6, sizeof(rtk_rg_asic_path6_entry_t));
	flowPath6->in_path = FB_PATH_6;
	flowPath6->in_intf_idx = wan_intf_idx;
	flowPath6->in_smac_lut_idx = gmacL2Idx;
	flowPath6->in_dmac_lut_idx = rg_db.netif[wan_intf_idx].l2_idx;
	flowPath6->in_ctagif = pWanIntfInfo->wan_intf_conf.egress_vlan_tag_on;
	flowPath6->in_stagif = 0;											// FIXME, tagif or not
	flowPath6->in_pppoeif= overPPPoE;
	flowPath6->in_pppoe_sid = overPPPoE ? pPPPoEDSLiteInfo->after_dial.sessionId : 0;
	flowPath6->in_dsliteif = TRUE;
	flowPath6->in_protocol = FB_INPROTOCOL_ALL_ACCEPT;

	memset(&flowExtraInfo, 0, sizeof(rtk_rg_flow_extraInfo_t));
	if(!overPPPoE)
	{
		flowPath6->in_src_ipv6_addr_hash = _rtk_rg_sw_flowHashIPv6SrcAddr_get(pDSLiteInfo->rtk_dslite.ipAftr.ipv6_addr);
		flowPath6->in_dst_ipv6_addr_hash =  _rtk_rg_sw_flowHashIPv6DstAddr_get(pDSLiteInfo->rtk_dslite.ipB4.ipv6_addr);
		memcpy(flowExtraInfo.v6Sip.ipv6_addr, pDSLiteInfo->rtk_dslite.ipAftr.ipv6_addr, IPV6_ADDR_LEN);
		memcpy(flowExtraInfo.v6Dip.ipv6_addr, pDSLiteInfo->rtk_dslite.ipB4.ipv6_addr, IPV6_ADDR_LEN);
	}
	else
	{
		flowPath6->in_src_ipv6_addr_hash = _rtk_rg_sw_flowHashIPv6SrcAddr_get(pPPPoEDSLiteInfo->after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr);
		flowPath6->in_dst_ipv6_addr_hash=  _rtk_rg_sw_flowHashIPv6DstAddr_get(pPPPoEDSLiteInfo->after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr);
		memcpy(flowExtraInfo.v6Sip.ipv6_addr, pPPPoEDSLiteInfo->after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr, IPV6_ADDR_LEN);
		memcpy(flowExtraInfo.v6Dip.ipv6_addr, pPPPoEDSLiteInfo->after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr, IPV6_ADDR_LEN);
	}
	flowExtraInfo.igrSVID = 0;
	flowExtraInfo.igrSPRI = 0;
	flowExtraInfo.igrCVID = flowPath6->in_ctagif ? pWanIntfInfo->wan_intf_conf.egress_vlan_id : 0;
	flowExtraInfo.lutSaIdx = gmacL2Idx;
	flowExtraInfo.lutDaIdx = flowPath6->in_dmac_lut_idx;
	flowExtraInfo.path34_isGmac = 0;
	flowExtraInfo.naptOrTcpUdpGroupIdx = FAIL;
	flowExtraInfo.arpOrNeighborIdx_src = FAIL;
	flowExtraInfo.arpOrNeighborIdx_dst = FAIL;
	flowExtraInfo.staticEntry = 1;
	flowExtraInfo.addSwOnly = 0;
	flowExtraInfo.byDmac2cvid = 0;
	flowExtraInfo.pNaptFilterInfo = NULL;
	flowExtraInfo.pIgrACLFilterInfo = NULL;
	flowExtraInfo.pEgrACLFilterInfo = NULL;
	flowExtraInfo.tinyAckStreamId = -1;
	ret = _rtk_rg_flow_add(&flow_idx, FAIL, &flowPathEntry, flowExtraInfo);

	pWanIntfInfo->flowEntryIdx = flow_idx;

	return ret;
}



#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)

int32 _rtk_rg_determind_fb_flow_hash_flexible_pattern(void)
{
	//if any module need to enabled the hash flexible pattern, finally it will be enabled.
	int i;
	uint32 hash_cpri=0,hash_dscp=0;
	/*ACL determind*/
	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++)
	{
		if(rg_db.systemGlobal.acl_SW_table_entry[i].valid==RTK_RG_ENABLED)
		{

			if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_CTAG_PRI_BIT)
			{
				hash_cpri = 1;
				DEBUG("FB hash with cpri ... due to ACL has cpri pattern");
			}

			if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_DSCP_BIT ||
				rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_DSCP_BIT ||
				rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_TOS_BIT ||
				rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_TC_BIT)
			{
				hash_dscp = 1;
				DEBUG("FB hash with cpri ... due to ACL has dscp pattern");
			}

		}
	}

	/*CF determind*/
	for(i=0;i<TOTAL_CF_ENTRY_SIZE;i++)
	{
		if(rg_db.systemGlobal.classify_SW_table_entry[i].index!=FAIL){
			if(rg_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_TAGPRI_BIT){
				hash_cpri = 1;
				DEBUG("FB hash with cpri ... due to CF has cpri pattern");
				break;
			}
		}
	}


	/*QoS determind*/
	if(rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_DOT1Q] >=rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_PORTBASED])
	{
		hash_cpri = 1;
		DEBUG("FB hash with cpri ... due to QoS has cpri to internal-priroity and weight is higher than port-based");
	}

	if(rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_DSCP] >=rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_PORTBASED])
	{
		hash_dscp = 1;
		DEBUG("FB hash with cpri ... due to QoS has dscp to internal-priroity and weight is higher than port-based");
	}


#if defined(CONFIG_RG_RTL9607C_SERIES)
	if(!(rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0))
#endif
	{
		//determind cpri
		if(hash_cpri == 1){
			ASSERT_EQ(RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH12_SKIP_CPRI, DISABLED), SUCCESS);
			ASSERT_EQ(RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH34_UCBC_SKIP_CPRI, DISABLED), SUCCESS);
			ASSERT_EQ(RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH34_MC_SKIP_CPRI, DISABLED), SUCCESS);
			ASSERT_EQ(RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH5_SKIP_CPRI, DISABLED), SUCCESS);
			ASSERT_EQ(RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH6_SKIP_CPRI, DISABLED), SUCCESS);
			DEBUG("FB hash with cpri");
		}else{
			ASSERT_EQ(RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH12_SKIP_CPRI, ENABLED), SUCCESS);
			ASSERT_EQ(RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH34_UCBC_SKIP_CPRI, ENABLED), SUCCESS);
			ASSERT_EQ(RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH34_MC_SKIP_CPRI, ENABLED), SUCCESS);
			ASSERT_EQ(RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH5_SKIP_CPRI, ENABLED), SUCCESS);
			ASSERT_EQ(RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH6_SKIP_CPRI, ENABLED), SUCCESS);
			DEBUG("FB hash without cpri");
		}


		//determind dscp
		if(hash_dscp == 1){
			ASSERT_EQ(RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATHALL_SKIP_DSCP, DISABLED), SUCCESS);
			rg_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_TOS] = ENABLED;
			rg_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH34_TOS] = ENABLED;
			rg_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH5_TOS] = ENABLED;
			rg_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_TOS] = ENABLED;
			DEBUG("FB hash with dscp");
		}else{
			ASSERT_EQ(RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATHALL_SKIP_DSCP, ENABLED), SUCCESS);
			rg_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_TOS] = DISABLED;
			rg_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH34_TOS] = DISABLED;
			rg_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH5_TOS] = DISABLED;
			rg_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_TOS] = DISABLED;
			DEBUG("FB hash without dscp");
		}

#if defined(CONFIG_RG_G3_SERIES)
	ASSERT_EQ(_rtk_rg_g3_flow_key_mask_init(), RT_ERR_RG_OK);
#endif
	}


	//clear all flow to make sure it will learning right
	assert_ok(_rtk_rg_flow_clear());

	return RT_ERR_RG_OK;
}
#endif

#if defined(CONFIG_RG_G3_SERIES)
rtk_rg_mcEngine_info_t *_rtk_rg_g3McEngineInfoFind(uint32 *groupAddress ,uint32 *sourceAddress,uint32 isIPv6)
{
	int i,duplicatEntry=FAIL;
	if(groupAddress == NULL )
			return NULL;

	for(i=0 ; i< DEFAULT_MAX_FLOW_COUNT ;i++)
	{
		if(rg_db.mcHwEngineInfoTbl[i].valid)
		{
			//check hw entry duplication
			if((rg_db.mcHwEngineInfoTbl[i].isipv6 == isIPv6) && (memcmp(rg_db.mcHwEngineInfoTbl[i].groupAddress,groupAddress,sizeof(rg_db.mcHwEngineInfoTbl[i].groupAddress))==0))
			{
				duplicatEntry=i;
				break;
			}
		}
	}
	if(duplicatEntry ==FAIL)
		return NULL;

	return &rg_db.mcHwEngineInfoTbl[duplicatEntry];

}



rtk_rg_mcEngine_info_t *_rtk_rg_g3McEngineInfoGet(uint32 *groupAddress ,uint32 *sourceAddress,uint32 isIPv6,uint32 l2mcgid, uint32 l3mcgid)
{
	int i,firstInvalid=FAIL,duplicatEntry=FAIL,mcEngineInfoIdx=FAIL;
	if(groupAddress == NULL )
			return NULL;

	for(i=0 ; i< DEFAULT_MAX_FLOW_COUNT ;i++)
	{
		if(rg_db.mcHwEngineInfoTbl[i].valid)
		{
			//check hw entry duplication
			if((rg_db.mcHwEngineInfoTbl[i].isipv6==isIPv6) && (memcmp(rg_db.mcHwEngineInfoTbl[i].groupAddress,groupAddress,sizeof(rg_db.mcHwEngineInfoTbl[i].groupAddress))==0))
			{
				duplicatEntry=1;
				break;
			}
		}
		else
		{
			if(firstInvalid == FAIL)
				firstInvalid=i;
		}
	}

	if( firstInvalid == FAIL  &&  duplicatEntry ==FAIL)
		return NULL;

	if(duplicatEntry !=FAIL)
	{
		mcEngineInfoIdx = duplicatEntry;
	}
	else
	{
		mcEngineInfoIdx = firstInvalid;
		bzero(&rg_db.mcHwEngineInfoTbl[mcEngineInfoIdx],sizeof(rtk_rg_mcEngine_info_t));
		memcpy(rg_db.mcHwEngineInfoTbl[mcEngineInfoIdx].groupAddress,groupAddress,sizeof(rg_db.mcHwEngineInfoTbl[mcEngineInfoIdx].groupAddress));
		if(sourceAddress)
			memcpy(rg_db.mcHwEngineInfoTbl[mcEngineInfoIdx].sourceAddress,sourceAddress,sizeof(rg_db.mcHwEngineInfoTbl[mcEngineInfoIdx].sourceAddress));
		rg_db.mcHwEngineInfoTbl[mcEngineInfoIdx].l2mcgid=l2mcgid;
		rg_db.mcHwEngineInfoTbl[mcEngineInfoIdx].l3mcgid=l3mcgid;
		rg_db.mcHwEngineInfoTbl[mcEngineInfoIdx].isipv6 = isIPv6;
		rg_db.mcHwEngineInfoTbl[mcEngineInfoIdx].valid=1;
	}


	rg_db.mcHwEngineInfoTbl[mcEngineInfoIdx].mcHwRefCount++;
	return &rg_db.mcHwEngineInfoTbl[mcEngineInfoIdx];

}



int32 _rtk_rg_g3McEngineInfoDel(rtk_rg_mcEngine_info_t *pMcEngineInfo)
{
	if(pMcEngineInfo ==NULL ){WARNING("NULL Point");return FAIL;}
	if(pMcEngineInfo->valid==0 || pMcEngineInfo->mcHwRefCount==0)
	{
		TABLE(" pMcEngineInfo->valid ==0[%d] || pMcEngineInfo->mcHwRefCount==0[%d] Check Parameter",pMcEngineInfo->valid,pMcEngineInfo->mcHwRefCount==0);
		return FAIL;
	}

	pMcEngineInfo->mcHwRefCount--;
	if(pMcEngineInfo->mcHwRefCount==0)
	{
		//delete hw Entry
		TABLE("mcHwRefCount to zero delete etnry for hwMainhash");
		ASSERT_EQ(ca_l2_mcast_group_delete(G3_DEF_DEVID, pMcEngineInfo->l2mcgid), CA_E_OK);
		ASSERT_EQ(ca_l3_mcast_group_delete(G3_DEF_DEVID, pMcEngineInfo->l3mcgid), CA_E_OK);
		pMcEngineInfo->valid=0;
	}
	else if(pMcEngineInfo->mcHwRefCount==1)
	{
		TABLE("Reset Group to Trap");
		rtk_rg_GrpSetToTrap(pMcEngineInfo->groupAddress,pMcEngineInfo->isipv6);
	}

	TABLE("Delete L2/L3 GID SUCCESS l2mcgid=%d l3mcgid=%d mcHwRefCount=%d",pMcEngineInfo->l2mcgid,pMcEngineInfo->l3mcgid,pMcEngineInfo->mcHwRefCount);
	return SUCCESS;
}
/*====================
_rtk_rg_fc_g3L3Policer_set(): Set L3 policer.
	copy from dal_ca8279_rate_shareMeter_set().
	_rtk_rg_fc_g3L3Policer_set():		Set L3 policer
	dal_ca8279_rate_shareMeter_set():	Set L2 policer
====================*/
int32 _rtk_rg_fc_g3L3Policer_set(uint32 index, uint32 rate, rtk_enable_t ifgInclude)
{
	ca_status_t ret=CA_E_OK;
	ca_uint16_t flow_id,group_base,i;
	ca_policer_t policer;
	ca_policer_config_t config;

	/* parameter check */
	if((G3_FLOW_POLICER_IDXSHIFT_PROC + G3_FLOW_POLICER_PROC_SIZE) <= index)
		return RT_ERR_RG_INVALID_PARAM;
	if(MAX_SWITCH_METER_RATE < rate)
		return RT_ERR_RG_INVALID_PARAM;
	if(RTK_ENABLE_END < ifgInclude)
		return RT_ERR_RG_INVALID_PARAM;


	if(rate >= 32767499) //Disable policer
	{
		flow_id = index;

		if((ret = ca_l3_flow_policer_get(G3_DEF_DEVID,flow_id,&policer)) != CA_E_OK)
			return RT_ERR_RG_FAILED;

		policer.pps = 0;
		policer.cir = 32767499;
		policer.cbs = 0xfff;
		policer.pir = 32767499;
		policer.pbs = 0xfff;

		if((ret = ca_l3_flow_policer_set(G3_DEF_DEVID,flow_id,&policer)) != CA_E_OK)
			return RT_ERR_RG_FAILED;


		group_base = (index/32)*32;

		for(i=0;i<32;i++) /*Policer is 32 set of 1 group*/
		{
			flow_id = i+group_base;

			if((ret = ca_l3_flow_policer_get(G3_DEF_DEVID,flow_id,&policer)) != CA_E_OK)
				return RT_ERR_RG_FAILED;

			if(policer.pir < 32767499 || policer.cir < 32767499)
				return RT_ERR_OK; /*Because RG not retuen Error*/
		}

		flow_id = index;

		if((ret = ca_l3_flow_policer_get(G3_DEF_DEVID,flow_id,&policer)) != CA_E_OK)
			return RT_ERR_RG_FAILED;

		policer.mode = CA_POLICER_MODE_DISABLE;
		policer.pps = 0;
		policer.cir = 32767499;
		policer.cbs = 0xfff;
		policer.pir = 32767499;
		policer.pbs = 0xfff;
	}
	else
	{
		flow_id = index;

		if((ret = ca_l3_flow_policer_get(G3_DEF_DEVID,flow_id,&policer)) != CA_E_OK)
			return RT_ERR_RG_FAILED;

		policer.mode = CA_POLICER_MODE_SRTCM;
		policer.cir = rate;

		/*
		(According to CA recommendation, the value for CBS and PBS should have relation with CIR and PIR.
		CIR and PIR bigger, CBS and PBS should be bigger.)
		The following settings are provided by CA.
		If CIR = 100kbps, cbs can be programmed to 10(2560 byte).
		If CIR = 1Mbps ~10Mbps, cbs can be programmed to 100(25600 byte).
		If CIR = 10Mbps~100Mbps, CBS can be programmed to 1000(256000 byte).
		If CIR >100Mbps, CBS can be programmed to 0xfff.
		*/
		if(policer.cir == 0)
		{
			policer.cbs = 1; //set to minimum value
		}
		else if(policer.cir < 100)
		{
			policer.cbs = 10;
		}
		else if((policer.cir >= 100) && (policer.cir < 1000))
		{
			policer.cbs = 64; //set to the value that approach to RTK setting (64  * 256 = 16384 ~= 0x3fff)
		}
		else if((policer.cir >= 1000) && (policer.cir < 10000))
		{
			policer.cbs = 100;
		}
		else if((policer.cir >= 10000) && (policer.cir < 100000))
		{
			policer.cbs = 1000;
		}
		else
		{
			policer.cbs = 0xfff;
		}

		policer.pir = rate;

		if(policer.pir == 0)
		{
			policer.pbs = 1; //set to minimum value
		}
		else if(policer.pir < 100)
		{
			policer.pbs = 10;
		}
		else if((policer.pir >= 100) && (policer.pir < 1000))
		{
			policer.pbs = 64; //set to the value that approach to RTK setting (64  * 256 = 16384 ~= 0x3fff)
		}
		else if((policer.pir >= 1000) && (policer.pir < 10000))
		{
			policer.pbs = 100;
		}
		else if((policer.pir >= 10000) && (policer.pir < 100000))
		{
			policer.pbs = 1000;
		}
		else
		{
			policer.pbs = 0xfff;
		}
	}

	if((ret = ca_l3_flow_policer_set(G3_DEF_DEVID,flow_id,&policer)) != CA_E_OK)
		return RT_ERR_RG_FAILED;
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
	/*
	L3FE->L3TE: packet length does not include CRC, add 4 bytes in over head  (CRC: +4 bytes, fake vlan: -4 bytes)
	LAN-> LAN: packet ingress to L3FE with fake vlan
	LAN-> WAN: packet ingress to L3FE with fake vlan
	WAN-> LAN: packet ingress to L3FE without fake vlan (there will be some error for rate limiting)
	*/

	if(ifgInclude == ENABLED)
		config.overhead = 20;
	else
		config.overhead = 0;
#else
		/*L3FE->L3TE: packet length does not include CRC, add 4 bytes in over head*/
		if(ifgInclude == ENABLED)
			config.overhead = 24;
		else
			config.overhead = 4;
#endif

	if((ret = ca_l3_policer_config_set(G3_DEF_DEVID,&config)) != CA_E_OK)
		return RT_ERR_RG_FAILED;

	return SUCCESS;
}

/*====================
_rtk_rg_fc_g3L3Policer_get(): Get L3 policer.
	Get L3 policer: copy from dal_ca8279_rate_shareMeter_get().
	_rtk_rg_fc_g3L3Policer_set():		Get L3 policer
	dal_ca8279_rate_shareMeter_set():	Get L2 policer
====================*/
int32 _rtk_rg_fc_g3L3Policer_get(uint32 index, uint32 *pRate , rtk_enable_t *pIfgInclude)
{
	ca_status_t ret=CA_E_OK;
	ca_uint16_t flow_id;
	ca_policer_t policer;
	ca_policer_config_t config;

	/* parameter check */
	if((G3_FLOW_POLICER_IDXSHIFT_PROC + G3_FLOW_POLICER_PROC_SIZE) <= index)
		return RT_ERR_RG_INVALID_PARAM;
	if(NULL == pRate)
		return RT_ERR_RG_INVALID_PARAM;
	if(NULL == pIfgInclude)
		return RT_ERR_RG_INVALID_PARAM;

	flow_id = index;

	if((ret = ca_l3_flow_policer_get(G3_DEF_DEVID,flow_id,&policer)) != CA_E_OK)
		return RT_ERR_RG_FAILED;

	*pRate = policer.cir;

	if((ret = ca_l3_policer_config_get(G3_DEF_DEVID,&config)) != CA_E_OK)
		return RT_ERR_RG_FAILED;

	if(config.overhead == 0)
		*pIfgInclude = DISABLED;
	else
		*pIfgInclude = ENABLED;
	return SUCCESS;
}

int32 _rtk_rg_fc_g3L3PolicerMode_set(uint32 index, rtk_rate_metet_mode_t meterMode)
{
	ca_status_t ret=CA_E_OK;
	ca_uint16_t flow_id;
	ca_policer_t policer;

	/* parameter check */
	if((G3_FLOW_POLICER_IDXSHIFT_PROC + G3_FLOW_POLICER_PROC_SIZE) <= index)
		return RT_ERR_RG_INVALID_PARAM;
	if(METER_MODE_END <= meterMode)
		return RT_ERR_RG_INVALID_PARAM;

	flow_id = index;

	if((ret = ca_l3_flow_policer_get(0,flow_id,&policer)) != CA_E_OK)
		return RT_ERR_RG_FAILED;


	if(meterMode != METER_MODE_PACKET_RATE)
		policer.pps = 0; /* True - PPS mode, false - BPS mode */
	else
		policer.pps = 1; /* True - PPS mode, false - BPS mode */

	if((ret = ca_l3_flow_policer_set(0,flow_id,&policer)) != CA_E_OK)
		return RT_ERR_RG_FAILED;

	return SUCCESS;
}


int32 _rtk_rg_fc_g3L3PolicerMode_get(uint32 index, rtk_rate_metet_mode_t *pMeterMode)
{
	ca_status_t ret=CA_E_OK;
	ca_uint16_t flow_id;
	ca_policer_t policer;


	/* parameter check */
	if((G3_FLOW_POLICER_IDXSHIFT_PROC + G3_FLOW_POLICER_PROC_SIZE) <= index)
		return RT_ERR_RG_INVALID_PARAM;
	if(NULL == pMeterMode)
		return RT_ERR_RG_INVALID_PARAM;
	flow_id = index;

	if((ret = ca_l3_flow_policer_get(0,flow_id,&policer)) != CA_E_OK)
		return RT_ERR_RG_FAILED;

	if(policer.pps == 1)
		*pMeterMode = METER_MODE_PACKET_RATE; /* True - PPS mode, false - BPS mode */
	else
		*pMeterMode = METER_MODE_BIT_RATE; /* True - PPS mode, false - BPS mode */

	return RT_ERR_OK;
}
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
int32 _rtk_rg_fc_g3L2Policer_forSmacHostPol_set(uint32 index, uint32 rate, rtk_enable_t ifgInclude)
{
	ca_status_t ret=CA_E_OK;
	ca_uint16_t flow_id,group_base,i;
	ca_policer_t policer;
	ca_policer_config_t config;
	aal_l2_te_policer_cfg_t     cfg;
    aal_l2_te_policer_cfg_msk_t msk;

	/* parameter check */
	if((G3_FLOW_POLICER_IDXSHIFT_PROC + G3_FLOW_POLICER_PROC_SIZE) <= index)
		return RT_ERR_RG_INVALID_PARAM;
	if(MAX_SWITCH_METER_RATE < rate)
		return RT_ERR_RG_INVALID_PARAM;
	if(RTK_ENABLE_END < ifgInclude)
		return RT_ERR_RG_INVALID_PARAM;


	if(rate >= 32767499) //Disable policer
	{
		flow_id = index;

		if((ret = ca_l2_flow_policer_get(G3_DEF_DEVID,flow_id,&policer)) != CA_E_OK)
			return RT_ERR_RG_FAILED;

		policer.pps = 0;
		policer.cir = 32767499;
		policer.cbs = 0xfff;
		policer.pir = 32767499;
		policer.pbs = 0xfff;

		if((ret = ca_l2_flow_policer_set(G3_DEF_DEVID,flow_id,&policer)) != CA_E_OK)
			return RT_ERR_RG_FAILED;


		group_base = (index/32)*32;

		for(i=0;i<32;i++) /*Policer is 32 set of 1 group*/
		{
			flow_id = i+group_base;

			if((ret = ca_l2_flow_policer_get(G3_DEF_DEVID,flow_id,&policer)) != CA_E_OK)
				return RT_ERR_RG_FAILED;

			if(policer.pir < 32767499 || policer.cir < 32767499)
				return RT_ERR_OK; /*Because RG not retuen Error*/
		}

		flow_id = index;

		if((ret = ca_l2_flow_policer_get(G3_DEF_DEVID,flow_id,&policer)) != CA_E_OK)
			return RT_ERR_RG_FAILED;

		policer.mode = CA_POLICER_MODE_DISABLE;
		policer.pps = 0;
		policer.cir = 32767499;
		policer.cbs = 0xfff;
		policer.pir = 32767499;
		policer.pbs = 0xfff;
	}
	else
	{
		flow_id = index;

		if((ret = ca_l2_flow_policer_get(G3_DEF_DEVID,flow_id,&policer)) != CA_E_OK)
			return RT_ERR_RG_FAILED;

		policer.mode = CA_POLICER_MODE_SRTCM;
		policer.cir = rate;

		/*
		(According to CA recommendation, the value for CBS and PBS should have relation with CIR and PIR.
		CIR and PIR bigger, CBS and PBS should be bigger.)
		The following settings are provided by CA.
		If CIR = 100kbps, cbs can be programmed to 10(2560 byte).
		If CIR = 1Mbps ~10Mbps, cbs can be programmed to 100(25600 byte).
		If CIR = 10Mbps~100Mbps, CBS can be programmed to 1000(256000 byte).
		If CIR >100Mbps, CBS can be programmed to 0xfff.
		*/
		if(policer.cir == 0)
		{
			policer.cbs = 1; //set to minimum value
		}
		else if(policer.cir < 100)
		{
			policer.cbs = 10;
		}
		else if((policer.cir >= 100) && (policer.cir < 1000))
		{
			policer.cbs = 64; //set to the value that approach to RTK setting (64  * 256 = 16384 ~= 0x3fff)
		}
		else if((policer.cir >= 1000) && (policer.cir < 10000))
		{
			policer.cbs = 100;
		}
		else if((policer.cir >= 10000) && (policer.cir < 100000))
		{
			policer.cbs = 1000;
		}
		else
		{
			policer.cbs = 0xfff;
		}

		policer.pir = rate;

		if(policer.pir == 0)
		{
			policer.pbs = 1; //set to minimum value
		}
		else if(policer.pir < 100)
		{
			policer.pbs = 10;
		}
		else if((policer.pir >= 100) && (policer.pir < 1000))
		{
			policer.pbs = 64; //set to the value that approach to RTK setting (64  * 256 = 16384 ~= 0x3fff)
		}
		else if((policer.pir >= 1000) && (policer.pir < 10000))
		{
			policer.pbs = 100;
		}
		else if((policer.pir >= 10000) && (policer.pir < 100000))
		{
			policer.pbs = 1000;
		}
		else
		{
			policer.pbs = 0xfff;
		}
	}

	if((ret = ca_l2_flow_policer_set(G3_DEF_DEVID,flow_id,&policer)) != CA_E_OK)
		return RT_ERR_RG_FAILED;
	/*
	SMAC host plicing:
	with fake vlan --> L2 FE --> with fake vlan
	ifgInclude == ENABLED => -4 to remove fake vlan overhead
	ifgInclude == DISABLED => overhead can not be set to -4, fake vlan overhead will impact egress rate
	*/

	memset((void *)&cfg, 0, sizeof(cfg));
    memset((void *)&msk, 0, sizeof(msk));
	msk.s.ipg = 1;
	if(ifgInclude == ENABLED)
		cfg.ipg   = (20 - 4);
	else
		config.overhead = 0;
	if (aal_l2_te_policer_flow_cfg_set(G3_DEF_DEVID, i, msk, &cfg) != CA_E_OK)
		return RT_ERR_RG_FAILED;

	return SUCCESS;
}
#endif
#endif


#if !defined(CONFIG_RG_G3_SERIES)
int _rtk_rg_hw_funcbasedMeter_hwIdx_get(void)
{
	int i;

	for(i = 0 ; i < MAX_HW_SHAREMETER_TABLE_SIZE ; i++)
	{
		if(rg_db.systemGlobal.hw_shareMeterUsedState[i].used == FALSE)
		{
			return i;
		}
	}
	return FAILED;
}

int32 _rtk_rg_hostPolicongAndStormControlAndResAcl_readdByShareMeter(void)
{
	int i;

	if(rg_db.systemGlobal.funbasedMeter_mode != RTK_RG_METERMODE_HW_INDEX)
	{
		WARNING("Check! This function is called only when rg_db.systemGlobal.funbasedMeter_mode is RTK_RG_METERMODE_HW_INDEX");
		return RT_ERR_RG_FAILED;
	}


	for(i = 0 ; i < HOST_POLICING_TABLE_SIZE ; i++)
	{
		rtk_rg_hostPoliceControl_t HostPolicingEntry;
		memcpy(&HostPolicingEntry, &rg_db.hostPoliceList[i].info, sizeof(rtk_rg_hostPoliceControl_t));

		if(HostPolicingEntry.ingressLimitCtrl || HostPolicingEntry.egressLimitCtrl)
		{
			if(!rtk_rg_apolloPro_hostPoliceControl_set(&HostPolicingEntry, i))
			{
				TRACE("Readd host policing entry[%d] failed.", i);
			}
		}
	}
	for(i = 0 ; i < MAX_STORMCONTROL_ENTRY_SIZE ; i++)
	{
		if(rg_db.systemGlobal.stormControlInfoEntry[i].valid==RTK_RG_ENABLED)
		{
			assert_ok(rtk_rate_stormControlPortEnable_set(rg_db.systemGlobal.stormControlInfoEntry[i].port, rg_db.systemGlobal.stormControlInfoEntry[i].stormType, ENABLED));
			assert_ok(rtk_rate_stormControlMeterIdx_set(rg_db.systemGlobal.stormControlInfoEntry[i].port, rg_db.systemGlobal.stormControlInfoEntry[i].stormType, rg_db.systemGlobal.stormControlInfoEntry[i].meterIdx));

		}
	}

	if(rg_db.systemGlobal.ArpReqRateLimitShareMeterIdx != -1)
	{
		//readd reserve ACL
		rtk_rg_aclAndCf_reserved_arp_packet_assign_share_meter_t arp_packet_assign_share_meter;

		_rtk_rg_aclAndCfReservedRuleDel(RTK_RG_ACLANDCF_RESERVED_ARP_PACKET_ASSIGN_SHARE_METER);

		bzero(&arp_packet_assign_share_meter,sizeof(rtk_rg_aclAndCf_reserved_arp_packet_assign_share_meter_t));
		arp_packet_assign_share_meter.portmask = rg_db.systemGlobal.ArpReqRateLimitPortMask;
		arp_packet_assign_share_meter.share_meter = rg_db.systemGlobal.ArpReqRateLimitShareMeterIdx;

		_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_ARP_PACKET_ASSIGN_SHARE_METER,&arp_packet_assign_share_meter);
	}
	if(rg_db.systemGlobal.dosRateLimitCount)
	{
		RTK_RG_DOS_RATE_LIMIT_VALID_SCAN(i)
		{
			//update hw meter idx
			rg_db.systemGlobal.dosRateLimit[i].shareMeterIdx_hw = rg_db.systemGlobal.dosRateLimit[i].shareMeterIdx;
		}

		_rtk_rg_aclAndCfReservedRuleDel(RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER);
		_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER,NULL);
	}
	if(rg_db.systemGlobal.synRateLimitShareMeterIdx != -1)
	{
		//readd reserve ACL
		rtk_rg_aclAndCf_reserved_syn_packet_assign_share_meter_t syn_packet_assign_share_meter;

		_rtk_rg_aclAndCfReservedRuleDel(RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_ASSIGN_SHARE_METER);

		bzero(&syn_packet_assign_share_meter,sizeof(rtk_rg_aclAndCf_reserved_syn_packet_assign_share_meter_t));
		syn_packet_assign_share_meter.share_meter=rg_db.systemGlobal.synRateLimitShareMeterIdx;
		_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_ASSIGN_SHARE_METER, &syn_packet_assign_share_meter);
	}

	return RT_ERR_RG_OK;
}


int32 _rtk_rg_hostPolicongAndStormControlAndResAcl_readdByFuncbasedMeter(rtk_rg_meter_type_t type, uint32 idx)
{
	int i;

	if(rg_db.systemGlobal.funbasedMeter_mode != RTK_RG_METERMODE_SW_INDEX)
	{
		WARNING("Check! This function is called only when rg_db.systemGlobal.funbasedMeter_mode is RTK_RG_METERMODE_SW_INDEX");
		return RT_ERR_RG_FAILED;
	}

	if(type == RTK_RG_METER_HOSTPOL)
	{
		for(i = 0 ; i < HOST_POLICING_TABLE_SIZE ; i++)
		{
			rtk_rg_hostPoliceControl_t HostPolicingEntry;
			memcpy(&HostPolicingEntry, &rg_db.hostPoliceList[i].info, sizeof(rtk_rg_hostPoliceControl_t));
			if(HostPolicingEntry.limitMeterIdx == idx)
			{
				if(HostPolicingEntry.ingressLimitCtrl || HostPolicingEntry.egressLimitCtrl)
				{
					if(!rtk_rg_apolloPro_hostPoliceControl_set(&HostPolicingEntry, i))
					{
						TRACE("Readd host policing entry[%d] failed.", i);
					}
				}
			}
		}
	}
	else if(type == RTK_RG_METER_STORMCTL)
	{
		for(i = 0 ; i < MAX_STORMCONTROL_ENTRY_SIZE ; i++)
		{
			if(rg_db.systemGlobal.stormControlInfoEntry[i].valid==RTK_RG_ENABLED)
			{
				if(rg_db.systemGlobal.stormControlInfoEntry[i].meterIdx == idx)
				{
					uint32 meterIdx = rg_db.systemGlobal.funbasedMeter[type][idx].hwIdx;
					assert_ok(rtk_rate_stormControlPortEnable_set(rg_db.systemGlobal.stormControlInfoEntry[i].port, rg_db.systemGlobal.stormControlInfoEntry[i].stormType, ENABLED));
					assert_ok(rtk_rate_stormControlMeterIdx_set(rg_db.systemGlobal.stormControlInfoEntry[i].port, rg_db.systemGlobal.stormControlInfoEntry[i].stormType, meterIdx));
				}
			}
		}
	}
	else if(type == RTK_RG_METER_PROC)
	{
		if(rg_db.systemGlobal.ArpReqRateLimitShareMeterIdx == idx)
		{
			//readd reserve ACL
			rtk_rg_aclAndCf_reserved_arp_packet_assign_share_meter_t arp_packet_assign_share_meter;

			_rtk_rg_aclAndCfReservedRuleDel(RTK_RG_ACLANDCF_RESERVED_ARP_PACKET_ASSIGN_SHARE_METER);

			bzero(&arp_packet_assign_share_meter,sizeof(rtk_rg_aclAndCf_reserved_arp_packet_assign_share_meter_t));
			arp_packet_assign_share_meter.portmask = rg_db.systemGlobal.ArpReqRateLimitPortMask;
			arp_packet_assign_share_meter.share_meter = rg_db.systemGlobal.funbasedMeter[type][idx].hwIdx;

			_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_ARP_PACKET_ASSIGN_SHARE_METER,&arp_packet_assign_share_meter);
		}
		if(rg_db.systemGlobal.dosRateLimitCount)
		{
			RTK_RG_DOS_RATE_LIMIT_VALID_SCAN(i)
			{
				if(rg_db.systemGlobal.dosRateLimit[i].shareMeterIdx != idx)
					continue;
				//update hw meter idx
				rg_db.systemGlobal.dosRateLimit[i].shareMeterIdx_hw = rg_db.systemGlobal.funbasedMeter[type][idx].hwIdx;
			}

			_rtk_rg_aclAndCfReservedRuleDel(RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER);
			_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER,NULL);
		}
		if(rg_db.systemGlobal.synRateLimitShareMeterIdx == idx)
		{
			//readd reserve ACL
			rtk_rg_aclAndCf_reserved_syn_packet_assign_share_meter_t syn_packet_assign_share_meter;

			_rtk_rg_aclAndCfReservedRuleDel(RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_ASSIGN_SHARE_METER);

			bzero(&syn_packet_assign_share_meter,sizeof(rtk_rg_aclAndCf_reserved_syn_packet_assign_share_meter_t));
			syn_packet_assign_share_meter.share_meter = rg_db.systemGlobal.funbasedMeter[type][idx].hwIdx;
			_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_ASSIGN_SHARE_METER, &syn_packet_assign_share_meter);
		}
		if(rg_db.systemGlobal.dhcpRateLimitShareMeterIdx == idx)
		{
			//readd reserve ACL
			rtk_rg_aclAndCf_reserved_dhcp_packet_assign_share_meter_t dhcp_packet_assign_share_meter;

			_rtk_rg_aclAndCfReservedRuleDel(RTK_RG_ACLANDCF_RESERVED_DHCP_PACKET_ASSIGN_SHARE_METER);

			bzero(&dhcp_packet_assign_share_meter,sizeof(rtk_rg_aclAndCf_reserved_dhcp_packet_assign_share_meter_t));
			dhcp_packet_assign_share_meter.portmask = rg_db.systemGlobal.dhcpRateLimitPortMask;
			dhcp_packet_assign_share_meter.share_meter = rg_db.systemGlobal.funbasedMeter[type][idx].hwIdx;

			_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_DHCP_PACKET_ASSIGN_SHARE_METER,&dhcp_packet_assign_share_meter);
		}
	}
	else
	{
		WARNING("Invalid type: %d", type);
		return RT_ERR_RG_FAILED;
	}
	return RT_ERR_RG_OK;
}

#else //defined(CONFIG_RG_G3_SERIES)
int32 _rtk_rg_g3_get_mac_hostPolicing_info(rtk_mac_t *target_mac, rtk_rg_g3_mac_hostPolicing_info_t *hp_info)
{
	int i;
	rtk_mac_t zeroMac;
	memset(&zeroMac, 0, sizeof(rtk_mac_t));

	//init host policing info
	hp_info->ingressRateLimit_en = RTK_RG_DISABLED;
	hp_info->ingressRateLimit_mtrIdx = FAIL;
	hp_info->egressRateLimit_en = RTK_RG_DISABLED;
	hp_info->egressRateLimit_mtrIdx = FAIL;
	hp_info->logging_en= RTK_RG_DISABLED;
	hp_info->logging_hostPolEntryIdx = FAIL;

	for(i = 0 ; i < HOST_POLICING_TABLE_SIZE; i++)
	{
		if((hp_info->ingressRateLimit_en == RTK_RG_ENABLED) && (hp_info->egressRateLimit_en== RTK_RG_ENABLED) && (hp_info->logging_en== RTK_RG_ENABLED))
			break; // no need to check

		if(!memcmp(rg_db.hostPoliceList[i].info.macAddr.octet, zeroMac.octet, ETHER_ADDR_LEN))
			continue; //invalid host policing entry

		if(!memcmp(rg_db.hostPoliceList[i].info.macAddr.octet, target_mac->octet, ETHER_ADDR_LEN))
		{
			if((hp_info->ingressRateLimit_en == RTK_RG_DISABLED) && (rg_db.hostPoliceList[i].info.ingressLimitCtrl == ENABLED))
			{
				hp_info->ingressRateLimit_en = RTK_RG_ENABLED;
				hp_info->ingressRateLimit_mtrIdx= rg_db.hostPoliceList[i].info.limitMeterIdx;
			}

			if((hp_info->egressRateLimit_en == RTK_RG_DISABLED) && (rg_db.hostPoliceList[i].info.egressLimitCtrl == ENABLED))
			{
				hp_info->egressRateLimit_en = RTK_RG_ENABLED;
				hp_info->egressRateLimit_mtrIdx = rg_db.hostPoliceList[i].info.limitMeterIdx;
			}

			if((hp_info->logging_en== RTK_RG_DISABLED) && (rg_db.hostPoliceList[i].info.mibCountCtrl == ENABLED))
			{
				hp_info->logging_en= RTK_RG_ENABLED;
				hp_info->logging_hostPolEntryIdx= i;
			}
		}
	}
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
	hp_info->l2ClsIndex.idx_ingress = FAIL;
	hp_info->l2ClsIndex.idx_egress = FAIL;
	_rtk_rg_g3_l2cls_for_hostPoliceControl_add(target_mac, hp_info); //update L2 CLSs for the mac
#endif
	return RT_ERR_OK;
}
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
int32 _rtk_rg_g3_l2cls_for_hostPoliceControl_add(rtk_mac_t *target_mac, rtk_rg_g3_mac_hostPolicing_info_t *hp_info)
{
	/*Add L2 CLS for host policing*/
	rtk_rg_aclAndCf_reserved_hostPoliceSaDa_t hostPoliceSaDa_Info;
	memcpy(&(hostPoliceSaDa_Info.hostMac), target_mac, sizeof(rtk_mac_t));

	TABLE("Add L2 CLS rule for host MAC: %pM", target_mac->octet);
	//SMAC
	hostPoliceSaDa_Info.flow_id = FAIL;
	hostPoliceSaDa_Info.ca_cls_index = FAIL;
	if(hp_info->ingressRateLimit_en == RTK_RG_ENABLED) //enble ingress rate limit
		hostPoliceSaDa_Info.flow_id = hp_info->ingressRateLimit_mtrIdx + G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_RX;
	else if(hp_info->logging_hostPolEntryIdx != FAIL) //disable ingress rate limit but enable logging
		hostPoliceSaDa_Info.flow_id = hp_info->logging_hostPolEntryIdx + G3_FLOW_POLICER_IDXSHIFT_HPLOGRX;
	if(hostPoliceSaDa_Info.flow_id != FAIL) // add l2 CLS for ingress SA
	{
		if(_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_L2_INGRESS_SA_HOSTPOLICING, &hostPoliceSaDa_Info) != RT_ERR_OK)
		{
			WARNING("Add L2 CLS rule for SMAC(%pM) host policing failed.", target_mac->octet);
		}
		else
		{
			hp_info->l2ClsIndex.idx_ingress = hostPoliceSaDa_Info.ca_cls_index;
			TABLE("Add L2 CLS rule for SMAC(%pM): ca_acl[%d]", target_mac->octet, hp_info->l2ClsIndex.idx_ingress);
		}
	}

	//DMAC
	hostPoliceSaDa_Info.flow_id = FAIL;
	hostPoliceSaDa_Info.ca_cls_index = FAIL;
	if(hp_info->egressRateLimit_en == RTK_RG_ENABLED) //enble egress rate limit
		hostPoliceSaDa_Info.flow_id = hp_info->egressRateLimit_mtrIdx + G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_TX;
	else if(hp_info->logging_hostPolEntryIdx != FAIL) //disable egress rate limit but enable logging
		hostPoliceSaDa_Info.flow_id = hp_info->logging_hostPolEntryIdx + G3_FLOW_POLICER_IDXSHIFT_HPLOGTX;
	if(hostPoliceSaDa_Info.flow_id != FAIL) // add l2 CLS for egress DA
	{
		if(_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_L2_EGRESS_DA_HOSTPOLICING, &hostPoliceSaDa_Info) != RT_ERR_OK)
		{
			WARNING("Add L2 CLS rule for DMAC(%pM) host policing failed.", target_mac->octet);
		}
		else
		{
			hp_info->l2ClsIndex.idx_egress = hostPoliceSaDa_Info.ca_cls_index;
			TABLE("Add L2 CLS rule for SMAC(%pM): ca_acl[%d]", target_mac->octet, hp_info->l2ClsIndex.idx_egress);
		}
	}
	return RT_ERR_OK;
}
int32 _rtk_rg_g3_l2cls_for_hostPoliceControl_del(rtk_mac_t *target_mac)
{
	/* A MAC can be set into multiple host policing entry at the same time, thus del function should traverse the whole table */
	int i;
	bool do_deleted = TRUE;
	rtk_mac_t zeroMac;
	memset(&zeroMac, 0, sizeof(rtk_mac_t));

	for(i = 0 ; i < HOST_POLICING_TABLE_SIZE; i++)
	{
		if(!memcmp(rg_db.hostPoliceList[i].info.macAddr.octet, zeroMac.octet, ETHER_ADDR_LEN))
			continue; //invalid host policing entry

		if(!memcmp(rg_db.hostPoliceList[i].info.macAddr.octet, target_mac->octet, ETHER_ADDR_LEN))
		{
			if(do_deleted)
			{
				/*Because a mac may be set in multiple host policing entries, find the first valid CLS index to delete the corresponding CLS rules*/
				if((rg_db.hostPoliceList[i].l2ClsIndex.idx_ingress != FAIL) || (rg_db.hostPoliceList[i].l2ClsIndex.idx_ingress != FAIL))
				{
					if(rg_db.hostPoliceList[i].l2ClsIndex.idx_ingress != FAIL)
					{
						if(ca_classifier_rule_delete(G3_DEF_DEVID, rg_db.hostPoliceList[i].l2ClsIndex.idx_ingress) != CA_E_OK)
						{
							WARNING("delete ca_acl[%d] failed", rg_db.hostPoliceList[i].l2ClsIndex.idx_ingress);
							return RT_ERR_FAILED;
						}
					}
					if(rg_db.hostPoliceList[i].l2ClsIndex.idx_egress != FAIL)
					{
						if(ca_classifier_rule_delete(G3_DEF_DEVID, rg_db.hostPoliceList[i].l2ClsIndex.idx_egress) != CA_E_OK)
						{
							WARNING("delete ca_acl[%d] failed", rg_db.hostPoliceList[i].l2ClsIndex.idx_egress);
							return RT_ERR_FAILED;
						}
					}
					TABLE("delete L2 CLS rule for MAC: %pM, SA: ca_acl[%d], DA: ca_acl[%d]", target_mac->octet, rg_db.hostPoliceList[i].l2ClsIndex.idx_ingress, rg_db.hostPoliceList[i].l2ClsIndex.idx_egress);
					do_deleted = FALSE;
				}
			}
			rg_db.hostPoliceList[i].l2ClsIndex.idx_ingress = FAIL;
			rg_db.hostPoliceList[i].l2ClsIndex.idx_egress = FAIL;

		}
	}
	return RT_ERR_OK;
}
#endif

#endif

uint32 _rtk_rg_fb_init(void)
{
#if defined(CONFIG_APOLLO_ROMEDRIVER)
	int i = 0;

#if !defined(CONFIG_RG_G3_SERIES)
	ASSERT_EQ(_rtk_rg_fb_hw_init(), RT_ERR_OK);
#endif

#if defined(CONFIG_RG_G3_SERIES)
	for(i=0; i<MAX_FLOW_SW_TABLE_SIZE; i++)
		rg_db.flow[i].mainHash_hwFlowIdx = FAIL;
#endif

	//init flow sw free link list
	_rtk_rg_init_list_head(&rg_db.flowFreeListHead);

	//init flow sw head table
	for(i=0;i<(MAX_FLOW_TABLE_SIZE>>MAX_FLOW_WAYS_SHIFT);i++)
		_rtk_rg_init_list_head(&rg_db.flowListHead[i]);


	for(i=0; i<(MAX_FLOW_SW_TABLE_SIZE-MAX_FLOW_HW_TABLE_SIZE); i++)
	{
		_rtk_rg_init_list(&rg_db.flowList[i], &rg_db.flowList[0], flow_idxList);
#if 0 //we can use address-based to get index of flowList, and flowList[i] is mapping to flow[i+MAX_FLOW_HW_TABLE_SIZE]. Thus, no need to store idx here.
		rg_db.flowList[i].idx=MAX_FLOW_HW_TABLE_SIZE;
		rg_db.flowList[i].idx+=i;
#endif

		//add free list to free list head
		_rtk_rg_list_add_to_tail(&rg_db.flowList[i], &rg_db.flowList[0], &rg_db.flowFreeListHead, flow_idxList);
	}

#if defined(CONFIG_RG_FLOW_4K_MODE)
	//init flow Tcam free link list
	INIT_LIST_HEAD(&rg_db.flowTcamFreeListHead);
	//init flow Tcam head table
	for(i=0;i<(MAX_FLOW_TABLE_SIZE>>MAX_FLOW_WAYS_SHIFT);i++)
		INIT_LIST_HEAD(&rg_db.flowTcamListHead[i]);

	for(i=0;i<MAX_FLOW_TCAM_TABLE_SIZE;i++)
	{
		INIT_LIST_HEAD(&rg_db.flowTcamList[i].flowTcam_list);
		rg_db.flowTcamList[i].idx=MAX_FLOW_TABLE_SIZE;
		rg_db.flowTcamList[i].idx+=i;

		//add free list to free list head
		list_add_tail(&rg_db.flowTcamList[i].flowTcam_list,&rg_db.flowTcamFreeListHead);
	}
#endif

	//init flow head table of napt recorded
	for(i=0; i<MAX_NAPT_OUT_SW_TABLE_SIZE; i++)
		_rtk_rg_init_list_head(&rg_db.naptOut[i].flowListHead);
	//init flow list of napt recorded
	for(i=0; i<MAX_FLOW_SW_TABLE_SIZE; i++)
	{
		_rtk_rg_init_list(&rg_db.naptRecordedFlowList[i], &rg_db.naptRecordedFlowList[0], flow_idxList);
#if 0 //we can use address-based to get index of naptRecordedFlowList, and naptRecordedFlowList[i] is mapping to flow[i]. Thus, no need to store idx here.
		rg_db.naptRecordedFlowList[i].idx=i;
#endif
	}

	//init flow head table of tcp/udp tracking
	for(i=0; i<MAX_FLOW_TCP_UDP_TRACKING_TABLE_SIZE; i++)
		_rtk_rg_init_list_head(&rg_db.flowTcpUdpTrackingGroupHead[i]);
	//init free flow head of tcp/udp tracking group
	_rtk_rg_init_list_head(&rg_db.flowTcpUdpTrackingFreeGroupHead);
	for(i=0; i<MAX_FLOW_TCP_UDP_TRACKING_GROUP_SIZE; i++)
	{
		_rtk_rg_init_list(&rg_db.flowTcpUdpTrackingGroupList[i], &rg_db.flowTcpUdpTrackingGroupList[0], group_idxList);
		rg_db.flowTcpUdpTrackingGroupList[i].group_idx=i;
		//add free list to free list head
		_rtk_rg_list_add_to_tail(&rg_db.flowTcpUdpTrackingGroupList[i], &rg_db.flowTcpUdpTrackingGroupList[0], &rg_db.flowTcpUdpTrackingFreeGroupHead, group_idxList);

		_rtk_rg_init_list_head(&rg_db.flowTcpUdpTrackingGroupList[i].flowTcpUdpTrackingHead);
	}
	//init flow list of tcp/udp tracking
	for(i=0; i<MAX_FLOW_SW_TABLE_SIZE; i++)
	{
		_rtk_rg_init_list(&rg_db.flowTcpUdpTrackingList[i], &rg_db.flowTcpUdpTrackingList[0], flow_idxList);
#if 0 //we can use address-based to get index of flowTcpUdpTrackingList, and flowTcpUdpTrackingList[i] is mapping to flow[i]. Thus, no need to store idx here.
		rg_db.flowTcpUdpTrackingList[i].idx=i;
#endif
	}

	{
		//init head table of L2/L3 fragment
		for(i=0; i<MAX_L2L3_FRAGMENT_HASH_BUCKET; i++)
			INIT_LIST_HEAD(&rg_db.L2L3FragHashListHead[i]);
		//init free head of L2/L3 fragment
		INIT_LIST_HEAD(&rg_db.L2L3FragFreeListHead);
		for(i=0; i<MAX_L2L3_FRAGMENT_TABLE_SIZE; i++)
		{
			INIT_LIST_HEAD(&rg_db.L2L3FragList[i].frag_list);
			rg_db.L2L3FragList[i].frag_idx=i;
			//add free list to free list head
			list_add_tail(&rg_db.L2L3FragList[i].frag_list, &rg_db.L2L3FragFreeListHead);

			INIT_LIST_HEAD(&rg_db.L2L3FragList[i].fragQueueHead);
		}

		//init free head of L2/L3 fragment queue
		INIT_LIST_HEAD(&rg_db.L2L3FragQueueFreeListHead);
		for(i=0; i<MAX_L2L3_FRAGMENT_QUEUE_SIZE; i++)
		{
			INIT_LIST_HEAD(&rg_db.L2L3FragQueueList[i].fragQueue_list);
			//add free list to free list head
			list_add_tail(&rg_db.L2L3FragQueueList[i].fragQueue_list, &rg_db.L2L3FragQueueFreeListHead);
		}
	}
#endif //CONFIG_APOLLO_ROMEDRIVER

	return RT_ERR_OK;
}

rtk_rg_err_code_t rtk_rg_apolloPro_portBasedCVlanId_get(rtk_rg_port_idx_t port_idx,int *pPvid)
{
	if(RG_INVALID_PORT(port_idx)) RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	if(pPvid==NULL) RETURN_ERR(RT_ERR_RG_NULL_POINTER);
	if(port_idx<=RTK_RG_PORT_LASTCPU)
	{
		*pPvid=rg_db.systemGlobal.portBasedVID[port_idx];
	}
	else
	{
		*pPvid=rg_db.systemGlobal.portBasedVID[port_idx-RTK_RG_EXT_BASED_PORT];
	}
	return (RT_ERR_RG_OK);
}

