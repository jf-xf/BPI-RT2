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

#ifndef __RTK_RG_LITEROMEDRIVER_H__
#define __RTK_RG_LITEROMEDRIVER_H__

#include <rtk_rg_define.h>
#include <rtk_rg_struct.h>
#include <rtk_rg_liteRomeDriver.h>
#include <rtk_rg_internal.h>
#include <rtdrv_rg_netfilter.h> //for diagshell

#ifdef __linux__
#include <linux/init.h>
#include <linux/slab.h> //for kmalloc/kfree
#include <linux/delay.h> //for mdelay
#include <linux/skbuff.h>
#include <linux/timer.h>
#include <linux/proc_fs.h> //for create proc
#include <linux/netfilter.h> //for sockopt
#include <linux/module.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 30)
#else
#include <linux/config.h>
#endif
#include <linux/netdevice.h>
#endif
#include <rtk_rg_callback.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 30)
spinlock_t rgApiLock;
#else
spinlock_t rgApiLock = SPIN_LOCK_UNLOCKED; //used when each RG API is called.
#endif

atomic_t rg_api_entrance = ATOMIC_INIT(0);
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
volatile int rg_api_flowClearAccelerate; 		//0:init 1:should to clear flow in RGAPI_AFTER  2:AFTER API clear flow
volatile int rg_api_aclReserveRearrangeAccelerate; 		//0:init 1:should to rearrange reserve acl in RGAPI_AFTER  2:AFTER API rearrange reserve acl

extern int _rtk_rg_aclAndCfReservedRuleAdd(rtk_rg_aclAndCf_reserved_type_t rsvType, void *parameter);
#endif


void RGAPI_BEFORE(void)
{
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	if(atomic_read(&rg_api_entrance)>=1)
	{
		rg_api_flowClearAccelerate=0;
		rg_api_aclReserveRearrangeAccelerate=0;
	}
#endif
	return;
}

void RGAPI_AFTER(void)
{
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	if(rg_api_flowClearAccelerate)
	{
		//clear shorcut
		rg_api_flowClearAccelerate=2;
#if !defined(CONFIG_RT_EDP_API)
		_rtk_rg_flow_clear();
#endif
	}
	if(rg_api_aclReserveRearrangeAccelerate)
	{
		//rearrange reserve acl
		rg_api_aclReserveRearrangeAccelerate=2;
#if !defined(CONFIG_RT_EDP_API)
		_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_TAIL_END, NULL);
#endif
	}
	rg_api_flowClearAccelerate=0;
	rg_api_aclReserveRearrangeAccelerate=0;
#endif
	return;
}


#if defined(CONFIG_RTK_PTOOL_CPU_PERF) && defined(CONFIG_RTK_PTOOL_LA_BY_GPIO)
#define RGAPI_LOCK_BEFORE  PROFILE_START;  LA_GPIO_HIGH(LA_GPIO_AUX0);
#define RGAPI_LOCK_AFTER PROFILE_END;  LA_GPIO_LOW(LA_GPIO_AUX0);
#elif defined(CONFIG_RTK_PTOOL_CPU_PERF)
#define RGAPI_LOCK_BEFORE  PROFILE_START;
#define RGAPI_LOCK_AFTER PROFILE_END;
#elif defined(CONFIG_RTK_PTOOL_LA_BY_GPIO)
#define RGAPI_LOCK_BEFORE LA_GPIO_HIGH(LA_GPIO_AUX0);
#define RGAPI_LOCK_AFTER LA_GPIO_LOW(LA_GPIO_AUX0);
#else
#define RGAPI_LOCK_BEFORE do{}while(0);
#define RGAPI_LOCK_AFTER do{}while(0);
#endif


#ifdef CONFIG_SMP
#define rg_api_lock(lock) \
	RGAPI_LOCK_BEFORE;\
	do {						\
		if(atomic_read(&rg_api_entrance)>=1) \
			RGAPI("RG API LOCK REDUPLICATE ENTRANCE!!");	\
		RGAPI("[Enter]"); \
		rg_inbound_queue_lock(smp_processor_id(),&rg_kernel.rg_inbound_queue_lock); \
		spin_lock_bh(lock);	\
		atomic_inc(&rg_api_entrance);	\
		RGAPI_BEFORE();\
	} while (0);

#define rg_api_unlock(lock) \
	do {						\
		RGAPI_AFTER();			\
		atomic_dec(&rg_api_entrance);	\
		spin_unlock_bh(lock);	\
		rg_inbound_queue_unlock(&rg_kernel.rg_inbound_queue_lock); \
		RGAPI("[Exit ret=0x%x]",r); \
		if(atomic_read(&rg_api_entrance)<0)	\
			RGAPI("RG API UNLOCK REDUPLICATE RELEASE!!");	\
	} while (0); \
	RGAPI_LOCK_AFTER;\

#else
#define rg_api_lock(lock) \
	do {						\
		if(atomic_read(&rg_api_entrance)>=1) \
			WARNING("RG API LOCK REDUPLICATE ENTRANCE!!");	\
		preempt_disable(); \
		local_bh_disable(); \
		spin_lock_bh(lock);	\
		atomic_inc(&rg_api_entrance);	\
		RGAPI("[Enter]"); \
		RGAPI_BEFORE();	  \
	} while (0)

#define rg_api_unlock(lock) \
	do {						\
		RGAPI_AFTER();			\
		RGAPI("[Exit ret=0x%x]",r); \
		atomic_dec(&rg_api_entrance);	\
		spin_unlock_bh(lock);	\
		preempt_enable(); \
		local_bh_enable(); \
		if(atomic_read(&rg_api_entrance)<0)	\
			WARNING("RG API UNLOCK REDUPLICATE RELEASE!!");	\
	} while (0)

#endif

int32 rtk_rg_driverVersion_get(rtk_rg_VersionString_t *version_string)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_driverVersion_get(version_string);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_initParam_get(rtk_rg_initParams_t *init_param)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_initParam_get(init_param);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_initParam_set(rtk_rg_initParams_t *init_param)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_initParam_set(init_param);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_lanInterface_add(rtk_rg_lanIntfConf_t *lan_info,int *intf_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_lanInterface_add(lan_info,intf_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_wanInterface_add(rtk_rg_wanIntfConf_t *wanintf, int *wan_intf_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_wanInterface_add(wanintf,wan_intf_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_staticInfo_set(int wan_intf_idx, rtk_rg_ipStaticInfo_t *static_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_staticInfo_set(wan_intf_idx,static_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_dsliteInfo_set(int wan_intf_idx, rtk_rg_ipDslitStaticInfo_t *dslite_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_dsliteInfo_set(wan_intf_idx,dslite_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_dhcpRequest_set(int wan_intf_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_dhcpRequest_set(wan_intf_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_dhcpClientInfo_set(int wan_intf_idx, rtk_rg_ipDhcpClientInfo_t *dhcpClient_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_dhcpClientInfo_set(wan_intf_idx,dhcpClient_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_pppoeClientInfoBeforeDial_set(int wan_intf_idx, rtk_rg_pppoeClientInfoBeforeDial_t *app_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_pppoeClientInfoBeforeDial_set(wan_intf_idx,app_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_pppoeClientInfoAfterDial_set(int wan_intf_idx, rtk_rg_pppoeClientInfoAfterDial_t *clientPppoe_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_pppoeClientInfoAfterDial_set(wan_intf_idx,clientPppoe_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_pptpClientInfoBeforeDial_set(int wan_intf_idx, rtk_rg_pptpClientInfoBeforeDial_t *app_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_pptpClientInfoBeforeDial_set(wan_intf_idx,app_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_pptpClientInfoAfterDial_set(int wan_intf_idx, rtk_rg_pptpClientInfoAfterDial_t *clientPptp_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_pptpClientInfoAfterDial_set(wan_intf_idx,clientPptp_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_l2tpClientInfoBeforeDial_set(int wan_intf_idx, rtk_rg_l2tpClientInfoBeforeDial_t *app_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_l2tpClientInfoBeforeDial_set(wan_intf_idx,app_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_l2tpClientInfoAfterDial_set(int wan_intf_idx, rtk_rg_l2tpClientInfoAfterDial_t *clientL2tp_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_l2tpClientInfoAfterDial_set(wan_intf_idx,clientL2tp_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_vxlanClientInfo_set(int wan_intf_idx, rtk_rg_vxlanClientInfoAfterDial_t *vxlanClient_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_vxlanClientInfo_set(wan_intf_idx, vxlanClient_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_pppoeDsliteInfoBeforeDial_set(int wan_intf_idx, rtk_rg_pppoeClientInfoBeforeDial_t *app_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_pppoeDsliteInfoBeforeDial_set(wan_intf_idx,app_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_pppoeDsliteInfoAfterDial_set(int wan_intf_idx, rtk_rg_pppoeDsliteInfoAfterDial_t *pppoeDslite_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_pppoeDsliteInfoAfterDial_set(wan_intf_idx,pppoeDslite_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_interface_del(int lan_or_wan_intf_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_interface_del(lan_or_wan_intf_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_intfInfo_find(rtk_rg_intfInfo_t *intf_info, int *valid_lan_or_wan_intf_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_intfInfo_find(intf_info,valid_lan_or_wan_intf_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_cvlan_add(rtk_rg_cvlan_info_t *cvlan_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_cvlan_add(cvlan_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_cvlan_del(int cvlan_id)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_cvlan_del(cvlan_id);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_cvlan_get(rtk_rg_cvlan_info_t *cvlan_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_cvlan_get(cvlan_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_vlanBinding_add(rtk_rg_vlanBinding_t *vlan_binding_info, int *vlan_binding_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_vlanBinding_add(vlan_binding_info,vlan_binding_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_vlanBinding_del(int vlan_binding_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_vlanBinding_del(vlan_binding_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_vlanBinding_find(rtk_rg_vlanBinding_t *vlan_binding_info, int *valid_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_vlanBinding_find(vlan_binding_info,valid_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_algServerInLanAppsIpAddr_add(rtk_rg_alg_serverIpMapping_t *srvIpMapping)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_algServerInLanAppsIpAddr_add(srvIpMapping);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_algServerInLanAppsIpAddr_del(rtk_rg_alg_type_t delServerMapping)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_algServerInLanAppsIpAddr_del(delServerMapping);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_algApps_set(rtk_rg_alg_type_t alg_app)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_algApps_set(alg_app);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_algApps_get(rtk_rg_alg_type_t *alg_app)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_algApps_get(alg_app);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_dmzHost_set(int wan_intf_idx, rtk_rg_dmzInfo_t *dmz_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_dmzHost_set(wan_intf_idx,dmz_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_dmzHost_get(int wan_intf_idx, rtk_rg_dmzInfo_t *dmz_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_dmzHost_get(wan_intf_idx,dmz_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_virtualServer_add(rtk_rg_virtualServer_t *virtual_server, int *virtual_server_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_virtualServer_add(virtual_server,virtual_server_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_virtualServer_del(int virtual_server_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_virtualServer_del(virtual_server_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_virtualServer_find(rtk_rg_virtualServer_t *virtual_server, int *valid_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_virtualServer_find(virtual_server,valid_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_portTrigger_add(rtk_rg_portTrigger_info_t *port_trigger)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_portTrigger_add(port_trigger);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_portTrigger_del(rtk_rg_portTrigger_info_t *port_trigger)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_portTrigger_del(port_trigger);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_ipset_add(rtk_rg_ipset_rule_t *ipset_rule, int *index)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_ipset_add(ipset_rule, index);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_ipset_del(int set_idx, int index)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_ipset_del(set_idx, index);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_ipsets_group_set(rtk_rg_ipsets_group_cfg_t *ipsets_group_cfg, int index)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_ipsets_group_set(ipsets_group_cfg, index);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_ipsets_set_add(rtk_rg_ipsets_set_cfg_t *ipsets_set_cfg, int *index)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_ipsets_set_add(ipsets_set_cfg, index);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_ipsets_set_del(int index)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_ipsets_set_del(index);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_ipsets_rule_add(rtk_rg_ipsets_rule_t *ipsets_rule_cfg, int *index)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_ipsets_rule_add(ipsets_rule_cfg, index);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_ipsets_rule_del(int set_idx, int index)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_ipsets_rule_del(set_idx, index);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_aclFilterAndQos_add(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_aclFilterAndQos_add(acl_filter,acl_filter_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_aclFilterAndQos_del(int acl_filter_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_aclFilterAndQos_del(acl_filter_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_naptFilterAndQos_add(int *index,rtk_rg_naptFilterAndQos_t *napt_filter){
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_naptFilterAndQos_add(index,napt_filter);
	rg_api_unlock(&rgApiLock);
	return r;
}
int32 rtk_rg_naptFilterAndQos_del(int index){
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_naptFilterAndQos_del(index);
	rg_api_unlock(&rgApiLock);
	return r;
}
int32 rtk_rg_naptFilterAndQos_find(int *index,rtk_rg_naptFilterAndQos_t *napt_filter){
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_naptFilterAndQos_find(index,napt_filter);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_aclFilterAndQos_find(rtk_rg_aclFilterAndQos_t *acl_filter, int *valid_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_aclFilterAndQos_find(acl_filter,valid_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_macFilter_add(rtk_rg_macFilterEntry_t *macFilterEntry,int *mac_filter_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_macFilter_add(macFilterEntry,mac_filter_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_macFilter_del(int mac_filter_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_macFilter_del(mac_filter_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_macFilter_find(rtk_rg_macFilterEntry_t *macFilterEntry, int *valid_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_macFilter_find(macFilterEntry,valid_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_mac_filter_whitelist_add(rtk_rg_macFilterWhiteList_t *mac_filter_whitelist_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_mac_filter_whitelist_add(mac_filter_whitelist_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_mac_filter_whitelist_del(rtk_rg_macFilterWhiteList_t *mac_filter_whitelist_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_mac_filter_whitelist_del(mac_filter_whitelist_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_urlFilterString_add(rtk_rg_urlFilterString_t *filter,int *url_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_urlFilterString_add(filter,url_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_urlFilterString_del(int url_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_urlFilterString_del(url_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_urlFilterString_find(rtk_rg_urlFilterString_t *filter, int *valid_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_urlFilterString_find(filter,valid_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}



int32 rtk_rg_upnpConnection_add(rtk_rg_upnpConnection_t *upnp, int *upnp_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_upnpConnection_add(upnp,upnp_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_upnpConnection_del(int upnp_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_upnpConnection_del(upnp_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_upnpConnection_find(rtk_rg_upnpConnection_t *upnp, int *valid_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_upnpConnection_find(upnp,valid_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_naptConnection_add(rtk_rg_naptEntry_t *naptFlow, int *flow_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_naptConnection_add(naptFlow,flow_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_naptConnection_del(int flow_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_naptConnection_del(flow_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_naptConnection_find(rtk_rg_naptInfo_t *naptInfo,int *valid_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_naptConnection_find(naptInfo,valid_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_multicastFlow_add(rtk_rg_multicastFlow_t *mcFlow, int *flow_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_multicastFlow_add(mcFlow,flow_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_multicastFlow_del(int flow_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_multicastFlow_del(flow_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

/* martin ZHU add */
int32 rtk_rg_l2MultiCastFlow_add(rtk_rg_l2MulticastFlow_t *l2McFlow,int *flow_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_l2MultiCastFlow_add(l2McFlow, flow_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_multicastFlow_find(rtk_rg_multicastFlow_t *mcFlow, int *valid_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_multicastFlow_find(mcFlow,valid_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_macEntry_add(rtk_rg_macEntry_t *macEntry, int *entry_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_macEntry_add(macEntry,entry_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_macEntry_del(int entry_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_macEntry_del(entry_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_macEntry_find(rtk_rg_macEntry_t *macEntry,int *valid_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_macEntry_find(macEntry,valid_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_arpEntry_add(rtk_rg_arpEntry_t *arpEntry, int *arp_entry_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_arpEntry_add(arpEntry,arp_entry_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_arpEntry_del(int arp_entry_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_arpEntry_del(arp_entry_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_arpEntry_find(rtk_rg_arpInfo_t *arpInfo,int *arp_valid_idx)
{
	int r;
	//this API may call by Timer(it can't add mutex to lock.)
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_arpEntry_find(arpInfo,arp_valid_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_neighborEntry_add(rtk_rg_neighborEntry_t *neighborEntry,int *neighbor_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_neighborEntry_add(neighborEntry,neighbor_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_neighborEntry_del(int neighbor_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_neighborEntry_del(neighbor_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_neighborEntry_find(rtk_rg_neighborInfo_t *neighborInfo,int *neighbor_valid_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_neighborEntry_find(neighborInfo,neighbor_valid_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_softwareIdleTime_set(rtk_rg_idle_time_type_t idleTimeType, int idleTime)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_softwareIdleTime_set(idleTimeType,idleTime);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_softwareIdleTime_get(rtk_rg_idle_time_type_t idleTimeType, int *pIdleTime)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_softwareIdleTime_get(idleTimeType,pIdleTime);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_accessWanLimit_set(rtk_rg_accessWanLimitData_t access_wan_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_accessWanLimit_set(access_wan_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_accessWanLimit_get(rtk_rg_accessWanLimitData_t *access_wan_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_accessWanLimit_get(access_wan_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_accessWanLimitCategory_set(rtk_rg_accessWanLimitCategory_t macCategory_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=(int32)pf.rtk_rg_accessWanLimitCategory_set(macCategory_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_accessWanLimitCategory_get(rtk_rg_accessWanLimitCategory_t *macCategory_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=(int32)pf.rtk_rg_accessWanLimitCategory_get(macCategory_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_accessDot1xControl_set(rtk_rg_accessDot1xCtrl_t blocking_ctrl){
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_accessDot1xControl_set(blocking_ctrl);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_accessDot1xFilter_set(rtk_rg_accessDot1xFilter_t unblocking_filter){
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_accessDot1xFilter_set(unblocking_filter);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_softwareSourceAddrLearningLimit_set(rtk_rg_saLearningLimitInfo_t sa_learnLimit_info, rtk_rg_port_idx_t port_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_softwareSourceAddrLearningLimit_set(sa_learnLimit_info,port_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_softwareSourceAddrLearningLimit_get(rtk_rg_saLearningLimitInfo_t *sa_learnLimit_info, rtk_rg_port_idx_t port_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_softwareSourceAddrLearningLimit_get(sa_learnLimit_info,port_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_wlanSoftwareSourceAddrLearningLimit_set(rtk_rg_saLearningLimitInfo_t sa_learnLimit_info, int wlan_idx, int dev_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_wlanSoftwareSourceAddrLearningLimit_set(sa_learnLimit_info,wlan_idx,dev_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_wlanSoftwareSourceAddrLearningLimit_get(rtk_rg_saLearningLimitInfo_t *sa_learnLimit_info, int wlan_idx, int dev_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_wlanSoftwareSourceAddrLearningLimit_get(sa_learnLimit_info,wlan_idx,dev_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_dosPortMaskEnable_set(rtk_rg_mac_portmask_t dos_port_mask)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_dosPortMaskEnable_set(dos_port_mask);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_dosPortMaskEnable_get(rtk_rg_mac_portmask_t *dos_port_mask)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_dosPortMaskEnable_get(dos_port_mask);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_dosType_set(rtk_rg_dos_type_t dos_type,int dos_enabled,rtk_rg_dos_action_t dos_action)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_dosType_set(dos_type,dos_enabled,dos_action);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_dosType_get(rtk_rg_dos_type_t dos_type,int *dos_enabled,rtk_rg_dos_action_t *dos_action)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_dosType_get(dos_type,dos_enabled,dos_action);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_dosFloodType_set(rtk_rg_dos_type_t dos_type,int dos_enabled,rtk_rg_dos_action_t dos_action,int dos_threshold)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_dosFloodType_set(dos_type,dos_enabled,dos_action,dos_threshold);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_dosFloodType_get(rtk_rg_dos_type_t dos_type,int *dos_enabled,rtk_rg_dos_action_t *dos_action,int *dos_threshold)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_dosFloodType_get(dos_type,dos_enabled,dos_action,dos_threshold);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_dosFloodThresholdUnit_set(rtk_rg_dos_type_t dos_type, int dos_threshUnit)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_dosFloodThresholdUnit_set(dos_type,dos_threshUnit);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_dosFloodThresholdUnit_get(rtk_rg_dos_type_t dos_type, int *pDos_threshUnit)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_dosFloodThresholdUnit_get(dos_type,pDos_threshUnit);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_portMirror_set(rtk_rg_portMirrorInfo_t portMirrorInfo)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_portMirror_set(portMirrorInfo);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_portMirror_get(rtk_rg_portMirrorInfo_t *portMirrorInfo)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_portMirror_get(portMirrorInfo);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_portMirror_clear(void)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_portMirror_clear();
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_portEgrBandwidthCtrlRate_set(rtk_rg_mac_port_idx_t port, uint32 rate)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_portEgrBandwidthCtrlRate_set(port,rate);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_portIgrBandwidthCtrlRate_set(rtk_rg_mac_port_idx_t port, uint32 rate)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_portIgrBandwidthCtrlRate_set(port,rate);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_portEgrBandwidthCtrlRate_get(rtk_rg_mac_port_idx_t port, uint32 *rate)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_portEgrBandwidthCtrlRate_get(port,rate);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_portIgrBandwidthCtrlRate_get(rtk_rg_mac_port_idx_t port, uint32 *rate)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_portIgrBandwidthCtrlRate_get(port,rate);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_phyPortForceAbility_set(rtk_rg_mac_port_idx_t port, rtk_rg_phyPortAbilityInfo_t ability)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_phyPortForceAbility_set(port,ability);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_phyPortForceAbility_get(rtk_rg_mac_port_idx_t port, rtk_rg_phyPortAbilityInfo_t *ability)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_phyPortForceAbility_get(port,ability);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_cpuPortForceTrafficCtrl_set(rtk_rg_enable_t tx_fc_state,	rtk_rg_enable_t rx_fc_state)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_cpuPortForceTrafficCtrl_set(tx_fc_state,rx_fc_state);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_cpuPortForceTrafficCtrl_get(rtk_rg_enable_t *pTx_fc_state,	rtk_rg_enable_t *pRx_fc_state)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_cpuPortForceTrafficCtrl_get(pTx_fc_state,pRx_fc_state);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_portMibInfo_get(rtk_rg_mac_port_idx_t port, rtk_rg_port_mib_info_t *mibInfo)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_portMibInfo_get(port,mibInfo);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_portMibInfo_clear(rtk_rg_mac_port_idx_t port)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_portMibInfo_clear(port);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_portIsolation_set(rtk_rg_port_isolation_t isolationSetting)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_portIsolation_set(isolationSetting);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_portIsolation_get(rtk_rg_port_isolation_t *isolationSetting)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_portIsolation_get(isolationSetting);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_stormControl_add(rtk_rg_stormControlInfo_t *stormInfo,int *stormInfo_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_stormControl_add(stormInfo,stormInfo_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_stormControl_del(int stormInfo_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_stormControl_del(stormInfo_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_stormControl_find(rtk_rg_stormControlInfo_t *stormInfo,int *stormInfo_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_stormControl_find(stormInfo,stormInfo_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_shareMeter_set(uint32 index, uint32 rate, rtk_rg_enable_t ifgInclude)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_shareMeter_set(index,rate,ifgInclude);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_shareMeter_get(uint32 index, uint32 *pRate , rtk_rg_enable_t *pIfgInclude)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_shareMeter_get(index,pRate,pIfgInclude);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_shareMeterMode_set(uint32 index, rtk_rate_metet_mode_t meterMode)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_shareMeterMode_set(index,meterMode);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_shareMeterMode_get(uint32 index, rtk_rate_metet_mode_t *pMeterMode)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_shareMeterMode_get(index,pMeterMode);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32
rtk_rg_qosStrictPriorityOrWeightFairQueue_set(rtk_rg_mac_port_idx_t port_idx,rtk_rg_qos_queue_weights_t q_weight)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_qosStrictPriorityOrWeightFairQueue_set(port_idx,q_weight);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_qosStrictPriorityOrWeightFairQueue_get(rtk_rg_mac_port_idx_t port_idx,rtk_rg_qos_queue_weights_t *pQ_weight)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_qosStrictPriorityOrWeightFairQueue_get(port_idx,pQ_weight);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_qosInternalPriMapToQueueId_set(int int_pri, int queue_id)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_qosInternalPriMapToQueueId_set(int_pri,queue_id);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_qosInternalPriMapToQueueId_get(int int_pri, int *pQueue_id)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_qosInternalPriMapToQueueId_get(int_pri,pQueue_id);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_qosInternalPriDecisionByWeight_set(rtk_rg_qos_priSelWeight_t weightOfPriSel)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_qosInternalPriDecisionByWeight_set(weightOfPriSel);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_qosInternalPriDecisionByWeight_get(rtk_rg_qos_priSelWeight_t *pWeightOfPriSel)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_qosInternalPriDecisionByWeight_get(pWeightOfPriSel);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_qosDscpRemapToInternalPri_set(uint32 dscp,uint32 int_pri)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_qosDscpRemapToInternalPri_set(dscp,int_pri);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_qosDscpRemapToInternalPri_get(uint32 dscp,uint32 *pInt_pri)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_qosDscpRemapToInternalPri_get(dscp,pInt_pri);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_qosPortBasedPriority_set(rtk_rg_mac_port_idx_t port_idx,uint32 int_pri)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_qosPortBasedPriority_set(port_idx,int_pri);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_qosPortBasedPriority_get(rtk_rg_mac_port_idx_t port_idx,uint32 *pInt_pri)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_qosPortBasedPriority_get(port_idx,pInt_pri);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_qosDot1pPriRemapToInternalPri_set(uint32 dot1p,uint32 int_pri)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_qosDot1pPriRemapToInternalPri_set(dot1p,int_pri);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_qosDot1pPriRemapToInternalPri_get(uint32 dot1p,uint32 *pInt_pri)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_qosDot1pPriRemapToInternalPri_get(dot1p,pInt_pri);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_set(rtk_rg_mac_port_idx_t rmk_port,rtk_rg_enable_t rmk_enable, rtk_rg_qos_dscpRmkSrc_t rmk_src_select)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_set(rmk_port,rmk_enable,rmk_src_select);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_get(rtk_rg_mac_port_idx_t rmk_port,rtk_rg_enable_t *pRmk_enable, rtk_rg_qos_dscpRmkSrc_t *pRmk_src_select)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_get(rmk_port,pRmk_enable,pRmk_src_select);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_qosDscpRemarkByInternalPri_set(int int_pri,int rmk_dscp)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_qosDscpRemarkByInternalPri_set(int_pri,rmk_dscp);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_qosDscpRemarkByInternalPri_get(int int_pri,int *pRmk_dscp)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_qosDscpRemarkByInternalPri_get(int_pri,pRmk_dscp);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_qosDscpRemarkByDscp_set(int dscp,int rmk_dscp)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_qosDscpRemarkByDscp_set(dscp,rmk_dscp);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_qosDscpRemarkByDscp_get(int dscp,int *pRmk_dscp)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_qosDscpRemarkByDscp_get(dscp,pRmk_dscp);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_set(rtk_rg_mac_port_idx_t rmk_port, rtk_rg_enable_t rmk_enable)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_set(rmk_port,rmk_enable);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_get(rtk_rg_mac_port_idx_t rmk_port, rtk_rg_enable_t *pRmk_enable)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_get(rmk_port,pRmk_enable);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_qosDot1pPriRemarkByInternalPri_set(int int_pri,int rmk_dot1p)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_qosDot1pPriRemarkByInternalPri_set(int_pri,rmk_dot1p);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_qosDot1pPriRemarkByInternalPri_get(int int_pri,int *pRmk_dot1p)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_qosDot1pPriRemarkByInternalPri_get(int_pri,pRmk_dot1p);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_portBasedCVlanId_set(rtk_rg_port_idx_t port_idx,int pvid)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_portBasedCVlanId_set(port_idx,pvid);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_portBasedCVlanId_get(rtk_rg_port_idx_t port_idx,int *pPvid)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_portBasedCVlanId_get(port_idx,pPvid);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_wlanDevBasedCVlanId_set(int wlan_idx,int dev_idx,int dvid)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_wlanDevBasedCVlanId_set(wlan_idx,dev_idx,dvid);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_wlanDevBasedCVlanId_get(int wlan_idx,int dev_idx,int *pDvid)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_wlanDevBasedCVlanId_get(wlan_idx,dev_idx,pDvid);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_portStatus_get(rtk_rg_mac_port_idx_t port, rtk_rg_portStatusInfo_t *portInfo)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_portStatus_get(port,portInfo);
	rg_api_unlock(&rgApiLock);
	return r;
}

#ifdef CONFIG_RG_NAPT_PORT_COLLISION_PREVENTION
int32 rtk_rg_naptExtPortGet(int isTcp,uint16 *pPort)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_naptExtPortGet(isTcp,pPort);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_naptExtPortFree(int isTcp,uint16 port)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_naptExtPortFree(isTcp,port);
	rg_api_unlock(&rgApiLock);
	return r;
}
#endif

int32 rtk_rg_classifyEntry_add(rtk_rg_classifyEntry_t *classifyFilter)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_classifyEntry_add(classifyFilter);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_classifyEntry_find(int index, rtk_rg_classifyEntry_t *classifyFilter)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_classifyEntry_find(index,classifyFilter);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_classifyEntry_del(int index)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_classifyEntry_del(index);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_svlanTpid_set(uint32 svlan_tag_id)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_svlanTpid_set(svlan_tag_id);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_svlanTpid_get(uint32 *pSvlanTagId)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_svlanTpid_get(pSvlanTagId);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_svlanServicePort_set(rtk_port_t port, rtk_enable_t enable)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_svlanServicePort_set(port, enable);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_svlanServicePort_get(rtk_port_t port, rtk_enable_t *pEnable)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_svlanServicePort_get(port, pEnable);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_svlanTpid2_enable_set(rtk_rg_enable_t enable)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_svlanTpid2_enable_set(enable);
	rg_api_unlock(&rgApiLock);
	return r;
}
int32 rtk_rg_svlanTpid2_enable_get(rtk_rg_enable_t *pEnable)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_svlanTpid2_enable_get(pEnable);
	rg_api_unlock(&rgApiLock);
	return r;
}
int32 rtk_rg_svlanTpid2_set(uint32 svlan_tag_id)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_svlanTpid2_set(svlan_tag_id);
	rg_api_unlock(&rgApiLock);
	return r;
}
int32 rtk_rg_svlanTpid2_get(uint32 *pSvlanTagId)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_svlanTpid2_get(pSvlanTagId);
	rg_api_unlock(&rgApiLock);
	return r;
}





int32 rtk_rg_pppoeInterfaceIdleTime_get(int intfIdx,uint32 *idleSec)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_pppoeInterfaceIdleTime_get(intfIdx,idleSec);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_gatewayServicePortRegister_add(rtk_rg_gatewayServicePortEntry_t *serviceEntry, int *index){
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_gatewayServicePortRegister_add(serviceEntry,index);
	rg_api_unlock(&rgApiLock);
	return r;

}

int32 rtk_rg_gatewayServicePortRegister_del(int index){
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_gatewayServicePortRegister_del(index);
	rg_api_unlock(&rgApiLock);
	return r;

}

int32 rtk_rg_gatewayServicePortRegister_find(rtk_rg_gatewayServicePortEntry_t *serviceEntry, int *index){
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_gatewayServicePortRegister_find(serviceEntry,index);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_stpBlockingPortmask_set(rtk_rg_portmask_t Mask){
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_stpBlockingPortmask_set(Mask);
	rg_api_unlock(&rgApiLock);
	return r;
}
int32 rtk_rg_stpBlockingPortmask_get(rtk_rg_portmask_t *pMask){
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_stpBlockingPortmask_get(pMask);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_gponDsBcFilterAndRemarking_Enable(rtk_rg_enable_t enable)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_gponDsBcFilterAndRemarking_Enable(enable);
	rg_api_unlock(&rgApiLock);
	return r;
}


int32 rtk_rg_gponDsBcFilterAndRemarking_add(rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t *filterRule,int *index)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_gponDsBcFilterAndRemarking_add(filterRule,index);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_gponDsBcFilterAndRemarking_del(int index)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_gponDsBcFilterAndRemarking_del(index);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_gponDsBcFilterAndRemarking_del_all(void)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_gponDsBcFilterAndRemarking_del_all();
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_gponDsBcFilterAndRemarking_find(int *index,rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t *filterRule){
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_gponDsBcFilterAndRemarking_find(index,filterRule);
	rg_api_unlock(&rgApiLock);
	return r;
}


rtk_rg_err_code_t rtk_rg_interfaceMibCounter_del(int intf_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_interfaceMibCounter_del(intf_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

rtk_rg_err_code_t rtk_rg_interfaceMibCounter_get(rtk_rg_netifMib_entry_t *pNetifMib)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_interfaceMibCounter_get(pNetifMib);
	rg_api_unlock(&rgApiLock);
	return r;
}

rtk_rg_err_code_t rtk_rg_redirectHttpAll_set(rtk_rg_redirectHttpAll_t *pRedirectHttpAll)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_redirectHttpAll_set(pRedirectHttpAll);
	rg_api_unlock(&rgApiLock);
	return r;
}

rtk_rg_err_code_t rtk_rg_redirectHttpAll_get(rtk_rg_redirectHttpAll_t *pRedirectHttpAll)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_redirectHttpAll_get(pRedirectHttpAll);
	rg_api_unlock(&rgApiLock);
	return r;
}

rtk_rg_err_code_t rtk_rg_redirectHttpURL_add(rtk_rg_redirectHttpURL_t *pRedirectHttpURL)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_redirectHttpURL_add(pRedirectHttpURL);
	rg_api_unlock(&rgApiLock);
	return r;
}

rtk_rg_err_code_t rtk_rg_redirectHttpURL_del(rtk_rg_redirectHttpURL_t *pRedirectHttpURL)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_redirectHttpURL_del(pRedirectHttpURL);
	rg_api_unlock(&rgApiLock);
	return r;
}

rtk_rg_err_code_t rtk_rg_redirectHttpWhiteList_add(rtk_rg_redirectHttpWhiteList_t *pRedirectHttpWhiteList)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_redirectHttpWhiteList_add(pRedirectHttpWhiteList);
	rg_api_unlock(&rgApiLock);
	return r;
}

rtk_rg_err_code_t rtk_rg_redirectHttpWhiteList_del(rtk_rg_redirectHttpWhiteList_t *pRedirectHttpWhiteList)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_redirectHttpWhiteList_del(pRedirectHttpWhiteList);
	rg_api_unlock(&rgApiLock);
	return r;
}

rtk_rg_err_code_t rtk_rg_redirectHttpRsp_set(rtk_rg_redirectHttpRsp_t *pRedirectHttpRsp)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_redirectHttpRsp_set(pRedirectHttpRsp);
	rg_api_unlock(&rgApiLock);
	return r;
}

rtk_rg_err_code_t rtk_rg_redirectHttpRsp_get(rtk_rg_redirectHttpRsp_t *pRedirectHttpRsp)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_redirectHttpRsp_get(pRedirectHttpRsp);
	rg_api_unlock(&rgApiLock);
	return r;
}

rtk_rg_err_code_t rtk_rg_redirectHttpCount_set(rtk_rg_redirectHttpCount_t *pRedirectHttpCount)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_redirectHttpCount_set(pRedirectHttpCount);
	rg_api_unlock(&rgApiLock);
	return r;
}

rtk_rg_err_code_t rtk_rg_redirectHttpCount_get(rtk_rg_redirectHttpCount_t *pRedirectHttpCount)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_redirectHttpCount_get(pRedirectHttpCount);
	rg_api_unlock(&rgApiLock);
	return r;
}


rtk_rg_err_code_t rtk_rg_hostPoliceControl_set(rtk_rg_hostPoliceControl_t *pHostPoliceControl, int host_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_hostPoliceControl_set(pHostPoliceControl,host_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

rtk_rg_err_code_t rtk_rg_hostPoliceControl_get(rtk_rg_hostPoliceControl_t *pHostPoliceControl, int host_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_hostPoliceControl_get(pHostPoliceControl,host_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

rtk_rg_err_code_t rtk_rg_hostPoliceLogging_get(rtk_rg_hostPoliceLogging_t *pHostMibCnt, int host_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_hostPoliceLogging_get(pHostMibCnt,host_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

rtk_rg_err_code_t rtk_rg_hostPoliceLogging_del(int host_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_hostPoliceLogging_del(host_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_staticRoute_add(rtk_rg_staticRoute_t *pStaticRoute, int *index){
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_staticRoute_add(pStaticRoute,index);
	rg_api_unlock(&rgApiLock);
	return r;

}

int32 rtk_rg_staticRoute_del(int index){
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_staticRoute_del(index);
	rg_api_unlock(&rgApiLock);
	return r;

}

int32 rtk_rg_staticRoute_find(rtk_rg_staticRoute_t *pStaticRoute, int *index){
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_staticRoute_find(pStaticRoute,index);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_aclLogCounterControl_get(int index, int *type,  int *mode)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_aclLogCounterControl_get(index, type, mode);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_aclLogCounterControl_set(int index,  int type,  int mode)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_aclLogCounterControl_set(index, type, mode);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_aclLogCounter_get(int index, uint64 *count)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_aclLogCounter_get(index,count);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_aclLogCounter_reset(int index)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_aclLogCounter_reset(index);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_groupMacLimit_set(rtk_rg_groupMacLimit_t group_mac_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_groupMacLimit_set(group_mac_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_groupMacLimit_get(rtk_rg_groupMacLimit_t *pGroup_mac_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_groupMacLimit_get(pGroup_mac_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_vlanGroupMacLimit_add(rtk_rg_mac_port_idx_t port, int groupLimit, int *pGroupIdx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_vlanGroupMacLimit_add(port, groupLimit, pGroupIdx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_vlanGroupMacLimit_set(int groupIdx, int vlanId, int groupLimit)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_vlanGroupMacLimit_set(groupIdx, vlanId, groupLimit);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_vlanGroupMacLimit_del(int groupIdx, int vlanId)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_vlanGroupMacLimit_del(groupIdx, vlanId);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_vlanGroupMacLimit_get(int groupIdx, rtk_rg_vlanGroupMacLimit_info_t *pGroup)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_vlanGroupMacLimit_get(groupIdx, pGroup);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_vlanGroupMacLimit_find(rtk_rg_mac_port_idx_t port, int vlanId, int *pGroupIdx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_vlanGroupMacLimit_find(port, vlanId, pGroupIdx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_igmpMldSnoopingControl_set(rtk_rg_igmpMldSnoopingControl_t *config )
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_igmpMldSnoopingControl_set(config);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_igmpMldSnoopingControl_get(rtk_rg_igmpMldSnoopingControl_t *config )
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_igmpMldSnoopingControl_get(config);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_igmpMldSnoopingPortControl_add(rtk_rg_port_idx_t port_idx,rtk_rg_igmpMldSnoopingPortControl_t *config )
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_igmpMldSnoopingPortControl_add(port_idx,config);
	rg_api_unlock(&rgApiLock);
	return r;
}
int32 rtk_rg_igmpMldSnoopingPortControl_del(rtk_rg_port_idx_t port_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_igmpMldSnoopingPortControl_del(port_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}
int32 rtk_rg_igmpMldSnoopingPortControl_find(rtk_rg_port_idx_t port_idx,rtk_rg_igmpMldSnoopingPortControl_t *config )
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_igmpMldSnoopingPortControl_find(port_idx,config);
	rg_api_unlock(&rgApiLock);
	return r;
}




int32 rtk_rg_flowMibCounter_get(int index, rtk_rg_table_flowmib_t *pCounter)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_flowMibCounter_get(index, pCounter);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_flowMibCounter_reset(int index)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_flowMibCounter_reset(index);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_funcbasedMeter_set(rtk_rg_funcbasedMeterConf_t meterConf)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_funcbasedMeter_set(meterConf);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_funcbasedMeter_get(rtk_rg_funcbasedMeterConf_t *meterConf)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_funcbasedMeter_get(meterConf);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_flowHiPriEntry_add(rtk_rg_table_highPriPatten_t hiPriEntry,int *entry_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_flowHiPriEntry_add(hiPriEntry,entry_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_flowHiPriEntry_del(int entry_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_flowHiPriEntry_del(entry_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_urlflowPri_add(rtk_rg_urlHighPri_t* urlPriEt,int *entry_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_urlflowPri_add(urlPriEt,entry_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}

int32 rtk_rg_urlflowPri_del(int entry_idx)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_urlflowPri_del(entry_idx);
	rg_api_unlock(&rgApiLock);
	return r;
}


int32 rtk_rg_callback_function_ptr_get(rtk_rg_callbackFunctionPtrGet_t *callback_function_ptr_get_info)
{
	int r;
	rg_api_lock(&rgApiLock);
	r=pf.rtk_rg_callback_function_ptr_get(callback_function_ptr_get_info);
	rg_api_unlock(&rgApiLock);
	return r;
}

#ifdef __KERNEL__

/*
 * Data Declaration
 */
static struct nf_sockopt_ops rtdrv_rg_sockopts = {
    .pf = PF_INET,
    .set_optmin = RTDRV_BASE_CTL+RTDRV_RG_OFFSET,
    .set_optmax = RTDRV_BASE_CTL+RTDRV_RGEND_OFFSET+1,
    .set = do_rtdrv_rg_set_ctl,
    .get_optmin = RTDRV_BASE_CTL+RTDRV_RG_OFFSET,
    .get_optmax = RTDRV_BASE_CTL+RTDRV_RGEND_OFFSET+1,
    .get = do_rtdrv_rg_get_ctl,
};

int __init rtk_rg_api_module_init(void)
{
	int ret;
#if defined(CONFIG_APOLLO_ROMEDRIVER)
	int rtk_rg_rome_driver_module_init(void);
#endif
	//Don't lock sem_rgApiCall here! the lock has not inited!

	spin_lock_init(&rgApiLock);

	//liteRomeDriver Init
	ret=pf.rtk_rg_api_module_init();

#if defined(CONFIG_APOLLO_ROMEDRIVER)
	//fwdEngine Init
	rtk_rg_rome_driver_module_init();
#endif
	//diagShell Init
	if (nf_register_sockopt(&rtdrv_rg_sockopts))WARNING("[%s]: nf_register_sockopt failed.\n", __FUNCTION__);
	return ret;
}

void __exit rtk_rg_api_module_exit(void)
{
#if defined(CONFIG_APOLLO_ROMEDRIVER)
	void rtk_rg_rome_driver_module_exit(void);
	//fwdEngine Exit
	rtk_rg_rome_driver_module_exit();
#endif
	//diagShell Exit
	nf_unregister_sockopt(&rtdrv_rg_sockopts);
}

module_init(rtk_rg_api_module_init);
module_exit(rtk_rg_api_module_exit);
MODULE_AUTHOR("Realtek Semiconductor Corp.");
MODULE_DESCRIPTION("romeDriver");
MODULE_LICENSE("GPL");


EXPORT_SYMBOL(rtk_rg_api_module_init);
EXPORT_SYMBOL(rtk_rg_driverVersion_get);
EXPORT_SYMBOL(rtk_rg_initParam_get);
EXPORT_SYMBOL(rtk_rg_initParam_set);
EXPORT_SYMBOL(rtk_rg_lanInterface_add);
//5
EXPORT_SYMBOL(rtk_rg_wanInterface_add);
EXPORT_SYMBOL(rtk_rg_staticInfo_set);
EXPORT_SYMBOL(rtk_rg_dhcpRequest_set);
EXPORT_SYMBOL(rtk_rg_dhcpClientInfo_set);
EXPORT_SYMBOL(rtk_rg_pppoeClientInfoBeforeDial_set);
//10
EXPORT_SYMBOL(rtk_rg_pppoeClientInfoAfterDial_set);
EXPORT_SYMBOL(rtk_rg_interface_del);
EXPORT_SYMBOL(rtk_rg_intfInfo_find);
EXPORT_SYMBOL(rtk_rg_cvlan_add);
EXPORT_SYMBOL(rtk_rg_cvlan_del);
//15
EXPORT_SYMBOL(rtk_rg_cvlan_get);
EXPORT_SYMBOL(rtk_rg_vlanBinding_add);
EXPORT_SYMBOL(rtk_rg_vlanBinding_del);
EXPORT_SYMBOL(rtk_rg_vlanBinding_find);
EXPORT_SYMBOL(rtk_rg_algServerInLanAppsIpAddr_add);
//20
EXPORT_SYMBOL(rtk_rg_algServerInLanAppsIpAddr_del);
EXPORT_SYMBOL(rtk_rg_algApps_set);
EXPORT_SYMBOL(rtk_rg_algApps_get);
EXPORT_SYMBOL(rtk_rg_dmzHost_set);
EXPORT_SYMBOL(rtk_rg_dmzHost_get);
//25
EXPORT_SYMBOL(rtk_rg_virtualServer_add);
EXPORT_SYMBOL(rtk_rg_virtualServer_del);
EXPORT_SYMBOL(rtk_rg_virtualServer_find);
EXPORT_SYMBOL(rtk_rg_aclFilterAndQos_add);
EXPORT_SYMBOL(rtk_rg_aclFilterAndQos_del);
//30
EXPORT_SYMBOL(rtk_rg_aclFilterAndQos_find);
EXPORT_SYMBOL(rtk_rg_macFilter_add);
EXPORT_SYMBOL(rtk_rg_macFilter_del);
EXPORT_SYMBOL(rtk_rg_macFilter_find);
EXPORT_SYMBOL(rtk_rg_mac_filter_whitelist_add);
//35
EXPORT_SYMBOL(rtk_rg_mac_filter_whitelist_del);
EXPORT_SYMBOL(rtk_rg_urlFilterString_add);
EXPORT_SYMBOL(rtk_rg_urlFilterString_del);
EXPORT_SYMBOL(rtk_rg_urlFilterString_find);
EXPORT_SYMBOL(rtk_rg_upnpConnection_add);
//40
EXPORT_SYMBOL(rtk_rg_upnpConnection_del);
EXPORT_SYMBOL(rtk_rg_upnpConnection_find);
EXPORT_SYMBOL(rtk_rg_naptConnection_add);
EXPORT_SYMBOL(rtk_rg_naptConnection_del);
EXPORT_SYMBOL(rtk_rg_naptConnection_find);
//45
EXPORT_SYMBOL(rtk_rg_multicastFlow_add);
EXPORT_SYMBOL(rtk_rg_multicastFlow_del);
	/* martin zhu add */
EXPORT_SYMBOL(rtk_rg_l2MultiCastFlow_add);
EXPORT_SYMBOL(rtk_rg_multicastFlow_find);
EXPORT_SYMBOL(rtk_rg_macEntry_add);
//50
EXPORT_SYMBOL(rtk_rg_macEntry_del);
EXPORT_SYMBOL(rtk_rg_macEntry_find);
EXPORT_SYMBOL(rtk_rg_arpEntry_add);
EXPORT_SYMBOL(rtk_rg_arpEntry_del);
EXPORT_SYMBOL(rtk_rg_arpEntry_find);
//55
EXPORT_SYMBOL(rtk_rg_neighborEntry_add);
EXPORT_SYMBOL(rtk_rg_neighborEntry_del);
EXPORT_SYMBOL(rtk_rg_neighborEntry_find);
EXPORT_SYMBOL(rtk_rg_accessWanLimit_set);
EXPORT_SYMBOL(rtk_rg_accessWanLimit_get);
//60
EXPORT_SYMBOL(rtk_rg_accessWanLimitCategory_set);
EXPORT_SYMBOL(rtk_rg_accessWanLimitCategory_get);
EXPORT_SYMBOL(rtk_rg_softwareSourceAddrLearningLimit_set);
EXPORT_SYMBOL(rtk_rg_softwareSourceAddrLearningLimit_get);
EXPORT_SYMBOL(rtk_rg_dosPortMaskEnable_set);
//65
EXPORT_SYMBOL(rtk_rg_dosPortMaskEnable_get);
EXPORT_SYMBOL(rtk_rg_dosType_set);
EXPORT_SYMBOL(rtk_rg_dosType_get);
EXPORT_SYMBOL(rtk_rg_dosFloodType_set);
EXPORT_SYMBOL(rtk_rg_dosFloodType_get);
//70
EXPORT_SYMBOL(rtk_rg_portMirror_set);
EXPORT_SYMBOL(rtk_rg_portMirror_get);
EXPORT_SYMBOL(rtk_rg_portMirror_clear);
EXPORT_SYMBOL(rtk_rg_portEgrBandwidthCtrlRate_set);
EXPORT_SYMBOL(rtk_rg_portIgrBandwidthCtrlRate_set);
//75
EXPORT_SYMBOL(rtk_rg_portEgrBandwidthCtrlRate_get);
EXPORT_SYMBOL(rtk_rg_portIgrBandwidthCtrlRate_get);
EXPORT_SYMBOL(rtk_rg_phyPortForceAbility_set);
EXPORT_SYMBOL(rtk_rg_phyPortForceAbility_get);
EXPORT_SYMBOL(rtk_rg_cpuPortForceTrafficCtrl_set);
//80
EXPORT_SYMBOL(rtk_rg_cpuPortForceTrafficCtrl_get);
EXPORT_SYMBOL(rtk_rg_portMibInfo_get);
EXPORT_SYMBOL(rtk_rg_portMibInfo_clear);
EXPORT_SYMBOL(rtk_rg_stormControl_add);
EXPORT_SYMBOL(rtk_rg_stormControl_del);
//85
EXPORT_SYMBOL(rtk_rg_stormControl_find);
EXPORT_SYMBOL(rtk_rg_shareMeter_set);
EXPORT_SYMBOL(rtk_rg_shareMeter_get);
EXPORT_SYMBOL(rtk_rg_shareMeterMode_set);
EXPORT_SYMBOL(rtk_rg_shareMeterMode_get);
//90
EXPORT_SYMBOL(rtk_rg_qosStrictPriorityOrWeightFairQueue_set);
EXPORT_SYMBOL(rtk_rg_qosStrictPriorityOrWeightFairQueue_get);
EXPORT_SYMBOL(rtk_rg_qosInternalPriMapToQueueId_set);
EXPORT_SYMBOL(rtk_rg_qosInternalPriMapToQueueId_get);
EXPORT_SYMBOL(rtk_rg_qosInternalPriDecisionByWeight_set);
//95
EXPORT_SYMBOL(rtk_rg_qosInternalPriDecisionByWeight_get);
EXPORT_SYMBOL(rtk_rg_qosDscpRemapToInternalPri_set);
EXPORT_SYMBOL(rtk_rg_qosDscpRemapToInternalPri_get);
EXPORT_SYMBOL(rtk_rg_qosPortBasedPriority_set);
EXPORT_SYMBOL(rtk_rg_qosPortBasedPriority_get);
//100
EXPORT_SYMBOL(rtk_rg_qosDot1pPriRemapToInternalPri_set);
EXPORT_SYMBOL(rtk_rg_qosDot1pPriRemapToInternalPri_get);
EXPORT_SYMBOL(rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_set);
EXPORT_SYMBOL(rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_get);
EXPORT_SYMBOL(rtk_rg_qosDscpRemarkByInternalPri_set);
//105
EXPORT_SYMBOL(rtk_rg_qosDscpRemarkByInternalPri_get);
EXPORT_SYMBOL(rtk_rg_qosDscpRemarkByDscp_set);
EXPORT_SYMBOL(rtk_rg_qosDscpRemarkByDscp_get);
EXPORT_SYMBOL(rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_set);
EXPORT_SYMBOL(rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_get);
//110
EXPORT_SYMBOL(rtk_rg_qosDot1pPriRemarkByInternalPri_set);
EXPORT_SYMBOL(rtk_rg_qosDot1pPriRemarkByInternalPri_get);
EXPORT_SYMBOL(rtk_rg_portBasedCVlanId_set);
EXPORT_SYMBOL(rtk_rg_portBasedCVlanId_get);
EXPORT_SYMBOL(rtk_rg_portStatus_get);
//115
#ifdef CONFIG_RG_NAPT_PORT_COLLISION_PREVENTION
EXPORT_SYMBOL(rtk_rg_naptExtPortGet);
EXPORT_SYMBOL(rtk_rg_naptExtPortFree);
#endif
EXPORT_SYMBOL(rtk_rg_classifyEntry_add);
EXPORT_SYMBOL(rtk_rg_classifyEntry_find);
EXPORT_SYMBOL(rtk_rg_classifyEntry_del);
//120
EXPORT_SYMBOL(rtk_rg_svlanTpid_get);
EXPORT_SYMBOL(rtk_rg_svlanTpid_set);
EXPORT_SYMBOL(rtk_rg_svlanServicePort_set);
EXPORT_SYMBOL(rtk_rg_svlanServicePort_get);
EXPORT_SYMBOL(rtk_rg_pppoeInterfaceIdleTime_get);
//125
EXPORT_SYMBOL(rtk_rg_gatewayServicePortRegister_add);
EXPORT_SYMBOL(rtk_rg_gatewayServicePortRegister_del);
EXPORT_SYMBOL(rtk_rg_gatewayServicePortRegister_find);
EXPORT_SYMBOL(rtk_rg_wlanDevBasedCVlanId_set);
EXPORT_SYMBOL(rtk_rg_wlanDevBasedCVlanId_get);
//130
EXPORT_SYMBOL(rtk_rg_wlanSoftwareSourceAddrLearningLimit_set);
EXPORT_SYMBOL(rtk_rg_wlanSoftwareSourceAddrLearningLimit_get);
EXPORT_SYMBOL(rtk_rg_naptFilterAndQos_add);
EXPORT_SYMBOL(rtk_rg_naptFilterAndQos_del);
EXPORT_SYMBOL(rtk_rg_naptFilterAndQos_find);
//135
EXPORT_SYMBOL(rtk_rg_pptpClientInfoBeforeDial_set);
EXPORT_SYMBOL(rtk_rg_pptpClientInfoAfterDial_set);
EXPORT_SYMBOL(rtk_rg_l2tpClientInfoBeforeDial_set);
EXPORT_SYMBOL(rtk_rg_l2tpClientInfoAfterDial_set);
EXPORT_SYMBOL(rtk_rg_stpBlockingPortmask_set);
//140
EXPORT_SYMBOL(rtk_rg_stpBlockingPortmask_get);
EXPORT_SYMBOL(rtk_rg_portIsolation_set);
EXPORT_SYMBOL(rtk_rg_portIsolation_get);
EXPORT_SYMBOL(rtk_rg_dsliteInfo_set);
EXPORT_SYMBOL(rtk_rg_pppoeDsliteInfoBeforeDial_set);
//145
EXPORT_SYMBOL(rtk_rg_pppoeDsliteInfoAfterDial_set);
EXPORT_SYMBOL(rtk_rg_gponDsBcFilterAndRemarking_add);
EXPORT_SYMBOL(rtk_rg_gponDsBcFilterAndRemarking_del);
EXPORT_SYMBOL(rtk_rg_gponDsBcFilterAndRemarking_find);
EXPORT_SYMBOL(rtk_rg_gponDsBcFilterAndRemarking_del_all);
//150
EXPORT_SYMBOL(rtk_rg_gponDsBcFilterAndRemarking_Enable);
EXPORT_SYMBOL(rtk_rg_interfaceMibCounter_del);
EXPORT_SYMBOL(rtk_rg_interfaceMibCounter_get);
EXPORT_SYMBOL(rtk_rg_redirectHttpAll_set);
EXPORT_SYMBOL(rtk_rg_redirectHttpAll_get);
//155
EXPORT_SYMBOL(rtk_rg_redirectHttpURL_add);
EXPORT_SYMBOL(rtk_rg_redirectHttpURL_del);
EXPORT_SYMBOL(rtk_rg_redirectHttpWhiteList_add);
EXPORT_SYMBOL(rtk_rg_redirectHttpWhiteList_del);
EXPORT_SYMBOL(rtk_rg_redirectHttpRsp_set);
//160
EXPORT_SYMBOL(rtk_rg_redirectHttpRsp_get);
EXPORT_SYMBOL(rtk_rg_svlanTpid2_get);//supported by 9602c
EXPORT_SYMBOL(rtk_rg_svlanTpid2_set);//supported by 9602c
EXPORT_SYMBOL(rtk_rg_svlanTpid2_enable_get);//supported by 9602c
EXPORT_SYMBOL(rtk_rg_svlanTpid2_enable_set);//supported by 9602c
//165
EXPORT_SYMBOL(rtk_rg_hostPoliceControl_set);
EXPORT_SYMBOL(rtk_rg_hostPoliceControl_get);
EXPORT_SYMBOL(rtk_rg_hostPoliceLogging_get);
EXPORT_SYMBOL(rtk_rg_hostPoliceLogging_del);
EXPORT_SYMBOL(rtk_rg_redirectHttpCount_set);
//170
EXPORT_SYMBOL(rtk_rg_redirectHttpCount_get);
EXPORT_SYMBOL(rtk_rg_staticRoute_add);
EXPORT_SYMBOL(rtk_rg_staticRoute_del);
EXPORT_SYMBOL(rtk_rg_staticRoute_find);
EXPORT_SYMBOL(rtk_rg_aclLogCounterControl_get);
//175
EXPORT_SYMBOL(rtk_rg_aclLogCounterControl_set);
EXPORT_SYMBOL(rtk_rg_aclLogCounter_get);
EXPORT_SYMBOL(rtk_rg_aclLogCounter_reset);
EXPORT_SYMBOL(rtk_rg_groupMacLimit_get);
EXPORT_SYMBOL(rtk_rg_groupMacLimit_set);
//180
EXPORT_SYMBOL(rtk_rg_igmpMldSnoopingControl_set);
EXPORT_SYMBOL(rtk_rg_igmpMldSnoopingControl_get);
EXPORT_SYMBOL(rtk_rg_flowMibCounter_get);
EXPORT_SYMBOL(rtk_rg_flowMibCounter_reset);
EXPORT_SYMBOL(rtk_rg_softwareIdleTime_set);
//185
EXPORT_SYMBOL(rtk_rg_softwareIdleTime_get);
EXPORT_SYMBOL(rtk_rg_funcbasedMeter_set);
EXPORT_SYMBOL(rtk_rg_funcbasedMeter_get);
EXPORT_SYMBOL(rtk_rg_flowHiPriEntry_add);
EXPORT_SYMBOL(rtk_rg_flowHiPriEntry_del);
//190
EXPORT_SYMBOL(rtk_rg_igmpMldSnoopingPortControl_add);
EXPORT_SYMBOL(rtk_rg_igmpMldSnoopingPortControl_del);
EXPORT_SYMBOL(rtk_rg_igmpMldSnoopingPortControl_find);
EXPORT_SYMBOL(rtk_rg_callback_function_ptr_get);
EXPORT_SYMBOL(rtk_rg_vlanGroupMacLimit_add);
//195
EXPORT_SYMBOL(rtk_rg_vlanGroupMacLimit_set);
EXPORT_SYMBOL(rtk_rg_vlanGroupMacLimit_del);
EXPORT_SYMBOL(rtk_rg_vlanGroupMacLimit_get);
EXPORT_SYMBOL(rtk_rg_vlanGroupMacLimit_find);
EXPORT_SYMBOL(rtk_rg_dosFloodThresholdUnit_set);
//200
EXPORT_SYMBOL(rtk_rg_dosFloodThresholdUnit_get);
EXPORT_SYMBOL(rtk_rg_accessDot1xControl_set);
EXPORT_SYMBOL(rtk_rg_accessDot1xFilter_set);
EXPORT_SYMBOL(rtk_rg_urlflowPri_add);
EXPORT_SYMBOL(rtk_rg_urlflowPri_del);
//205
EXPORT_SYMBOL(rtk_rg_portTrigger_add);
EXPORT_SYMBOL(rtk_rg_portTrigger_del);
EXPORT_SYMBOL(rtk_rg_vxlanClientInfo_set);
EXPORT_SYMBOL(rtk_rg_ipset_add);
EXPORT_SYMBOL(rtk_rg_ipset_del);
//210
EXPORT_SYMBOL(rtk_rg_ipsets_group_set);
EXPORT_SYMBOL(rtk_rg_ipsets_set_add);
EXPORT_SYMBOL(rtk_rg_ipsets_set_del);
EXPORT_SYMBOL(rtk_rg_ipsets_rule_add);
EXPORT_SYMBOL(rtk_rg_ipsets_rule_del);
//215
#endif

#endif
