/*
 * Copyright (C) 2019 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*/
#define COMPILE_RTK_L34_FC_MGR_MODULE 1

#include <dal/dal_mapper.h>
#include <dal/rtl9607c/dal_rtl9607c.h>
#include <linux/netdevice.h>

#include <rtk_fc_helper_wlan.h>
#include <rtk_fc_helper.h>
#include <rtk_fc_mgr.h>
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE) || defined(CONFIG_RTK_SOC_RTL8198D)
#include <rtk_fc_api.h> //for rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_set() and rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_del()
#endif
#if defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT) || defined(CONFIG_FC_WIFI_TX_GMAC_AUTO_SEL_SUPPORT)
#include <rtk/trap.h>
#include <rtk/l2.h>
#endif
#if defined(CONFIG_RTK_SOC_RTL8198D)
#include <rtk/rt/rt_l2.h>
#endif

#if defined(CONFIG_GMAC2_USABLE)
#include <rtk/cpu.h>
#endif

extern rtk_fc_wlan_initmap_t wlanInitMap[];
extern size_t wlanInitMap_size;

#if defined(CONFIG_RTL_FC_USB_INTF)
#include "rtk_fc_usb.c"
#endif

static int rtk_fc_wlanDevidx2bandidx_by_idGroup(rtk_fc_wlan_devidx_t wlanDevIdx)
{
	// software band index. 
	rtk_fc_wlan_id_t band = 0;

	if(wlanDevIdx >= RTK_FC_WLANX_END_INTF){
		FCMGR_ERR("wlanDevIdx %d is not support, please have a check\n", wlanDevIdx);
		return 0;
	}

	if(wlanDevIdx < RTK_FC_WLAN1_ROOT_INTF)
		band = RTK_FC_WLAN_ID_0;
	else if (wlanDevIdx < RTK_FC_WLAN2_ROOT_INTF)
		band = RTK_FC_WLAN_ID_1;
#if defined(CONFIG_GMAC2_USABLE)	
	else 
		band = RTK_FC_WLAN_ID_2;
#endif

	return band;
}

int rtk_fc_wlanDevidx2bandidx(rtk_fc_wlan_devidx_t wlanDevIdx)
{
	if(wlanDevIdx >= RTK_FC_WLANX_END_INTF){
		FCMGR_ERR("wlanDevIdx %d is not support, please have a check\n", wlanDevIdx);
		return 0;
	}
	
	return fc_mgr_db.wlanDevMap[wlanDevIdx].band;
}

int rtk_fc_wlanDevIdx2port(rtk_fc_wlan_devidx_t wlanDevIdx, rtk_fc_mac_port_idx_t *macPort, rtk_fc_mac_ext_port_idx_t *macExtPort)
{
	if(wlanDevIdx<RTK_FC_WLANX_END_INTF)
	{
		if(fc_mgr_db.wlanDevMap[wlanDevIdx].valid) {

			*macPort = fc_mgr_db.wlanDevMap[wlanDevIdx].portmap.macPortIdx;
			*macExtPort = fc_mgr_db.wlanDevMap[wlanDevIdx].portmap.macExtPortIdx;
			
		}else
			return FAIL;
	
	}else
		return FAIL;
		
	return SUCCESS;
}

#if defined(CONFIG_RTK_SOC_RTL8198D)
int rtk_fc_is_wlan_5g(char *devname)
{
	if (unlikely(!devname))
		return FAIL;

	if (!strncmp(devname, RTK_FC_WLAN_NAME_5G, strlen(RTK_FC_WLAN_NAME_5G)))
		return RTK_EXT_FLOW_MIB_LAN_WLAN_5G;

	if (!strncmp(devname, RTK_FC_WLAN_NAME_2G, strlen(RTK_FC_WLAN_NAME_2G)))
		return RTK_EXT_FLOW_MIB_LAN_WLAN_2G;

	return FAIL;
}

int rtk_fc_wlanDevIdx2devName(rtk_fc_wlan_devidx_t wlan_dev_idx, unsigned char *name)
{
	rtk_fc_wlan_devmap_t *pwlanDevMap;

	if (unlikely(!name))
		return FAIL;

	if (wlan_dev_idx < RTK_FC_WLANX_END_INTF) {
		pwlanDevMap = &fc_mgr_db.wlanDevMap[wlan_dev_idx];

		if(pwlanDevMap->valid && pwlanDevMap->wlanDev) {
			memcpy(name, pwlanDevMap->wlanDev->name, IFNAMSIZ);
			return SUCCESS;
		}
	}

	return FAIL;
}
#endif

__IRAM_FC_NICTRX
int rtk_fc_wlanDevIdx2dev(rtk_fc_wlan_devidx_t wlan_dev_idx, struct net_device **pWifiDev)
{		
	*pWifiDev =NULL;
	
	if(wlan_dev_idx<RTK_FC_WLANX_END_INTF)
	{
		if(fc_mgr_db.wlanDevMap[wlan_dev_idx].valid && 
			fc_mgr_db.wlanDevMap[wlan_dev_idx].wlanDev && 
			netif_running(fc_mgr_db.wlanDevMap[wlan_dev_idx].wlanDev)) {

			*pWifiDev = fc_mgr_db.wlanDevMap[wlan_dev_idx].wlanDev;
		}
	
	}

	if(wlan_dev_idx < RTK_FC_WLANX_END_INTF &&  *pWifiDev==NULL){
		FCMGR_ERR("wlan dev id %d is not ready", wlan_dev_idx);
		return FAIL;
	}else
		return SUCCESS;
}

int rtk_fc_check_wlan_device_is_running(rtk_fc_wlan_devidx_t wlan_dev_idx)
{		
	
	if(wlan_dev_idx<RTK_FC_WLANX_END_INTF)
	{
		if(fc_mgr_db.wlanDevMap[wlan_dev_idx].valid && 
			fc_mgr_db.wlanDevMap[wlan_dev_idx].wlanDev ) {

			if(netif_running(fc_mgr_db.wlanDevMap[wlan_dev_idx].wlanDev))
				return SUCCESS;
			else
				return FAIL;
		}
	
	}

	return FAIL;
}	


int _rtk_fc_dev2wlanDevIdx(struct net_device *dev, rtk_fc_wlan_devidx_t *wlan_dev_idx)
{
	rtk_fc_wlan_devmap_t *pDevMap=NULL, *pNextDevMap=NULL;
	uint32_t hashidx = rtk_fc_wlan_ifidx_devlist_hash(dev);
	
	*wlan_dev_idx = RTK_FC_WLANX_NOT_FOUND;

	list_for_each_entry_safe(pDevMap, pNextDevMap, &fc_mgr_db.wlanIfidxDevHead[hashidx], ifidxDevList)
	{
		if(pDevMap && (pDevMap->wlanDev!=NULL) && (pDevMap->wlanDev == dev))
		{
			*wlan_dev_idx = pDevMap->wlandevidx;
		}
	}

	if(*wlan_dev_idx!=RTK_FC_WLANX_NOT_FOUND)
		return SUCCESS;
	else{
		return FAIL;
	}
}

int rtk_fc_devName2wlanDevIdx(char *devName, rtk_fc_wlan_devidx_t *wlanDevIdx, rtk_fc_wlan_devmask_t *wlanDevIdMask)
{
	rtk_fc_wlan_devidx_t i;

	*wlanDevIdx= RTK_FC_WLANX_NOT_FOUND;
	*wlanDevIdMask = 0LL;

	// control path, low performance search is fine.
	for(i = 0; i < RTK_FC_WLANX_END_INTF; i++)
	{
		if(!fc_mgr_db.wlanDevMap[i].valid)
			continue;

		if(!strncmp(fc_mgr_db.wlanDevMap[i].wlanDev->name, devName, IFNAMSIZ))
		{
			*wlanDevIdx = i;
			*wlanDevIdMask |= (1LL<<i);
		}
	}

	if(*wlanDevIdx!=RTK_FC_WLANX_NOT_FOUND)
		return SUCCESS;
	else
		return FAIL;
}

__IRAM_FC_NICTRX
int rtk_fc_port2wlanDevidx(rtk_fc_mac_port_idx_t macPort, rtk_fc_mac_ext_port_idx_t macExtPort, rtk_fc_wlan_devidx_t *wlanDevIdx, rtk_fc_wlan_devmask_t *wlanDevIdMask)
{
#if defined (CONFIG_RTK_L34_XPON_PLATFORM)
	rtk_fc_wlan_devmap_t *pDevMap=NULL, *pNextDevMap=NULL;
	uint32_t hashidx = rtk_fc_wlan_port_devlist_hash(macPort, macExtPort);

	*wlanDevIdx = RTK_FC_WLANX_NOT_FOUND;
	*wlanDevIdMask = 0LL;
	
	list_for_each_entry_safe(pDevMap, pNextDevMap, &fc_mgr_db.wlanPortDevHead[hashidx], portDevList)
	{
		if(pDevMap 
			&& (pDevMap->portmap.macPortIdx == macPort) 
			&& (pDevMap->portmap.macExtPortIdx == macExtPort)
			&& (pDevMap->wlandevidx < RTK_FC_WLANX_END_INTF))
		{

			*wlanDevIdMask |= (1LL<<(pDevMap->wlandevidx%63));
			
			if(!pDevMap->shareExtPort) {
				*wlanDevIdx = pDevMap->wlandevidx;
				break;
				
			}else {
				// N wlan to 1 port: could not decide correct wlandevid
				*wlanDevIdx = RTK_FC_WLANX_MULTI_INTF;
			}

		}
	}
	
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)	
	// not support. no necessary.
#endif

	return SUCCESS;
}

#if defined(CONFIG_RTK_SOC_RTL8198D)
int rtk_fc_port2wlanDevName(rtk_fc_mac_port_idx_t macPort, rtk_fc_mac_ext_port_idx_t macExtPort, unsigned char *dev_name)
{
	rtk_fc_wlan_devmap_t *pDevMap=NULL, *pNextDevMap=NULL;
	uint32_t hashidx = rtk_fc_wlan_port_devlist_hash(macPort, macExtPort);

	if (unlikely(!dev_name))
		return -1;

	list_for_each_entry_safe(pDevMap, pNextDevMap, &fc_mgr_db.wlanPortDevHead[hashidx], portDevList)
	{
		if(pDevMap 
			&& (pDevMap->portmap.macPortIdx == macPort) 
			&& (pDevMap->portmap.macExtPortIdx == macExtPort)
			&& (pDevMap->wlandevidx < RTK_FC_WLANX_END_INTF))
		{
			if (!pDevMap->shareExtPort) {
				memcpy(dev_name, pDevMap->wlanDev->name, IFNAMSIZ);
				return 0;
			}
			else {
				return 1;
			}
		}
	}

	return -1;
}
#endif

int rtk_fc_cpuport2wlanDevidx(rtk_fc_mac_port_idx_t macPort, rtk_fc_wlan_devidx_t *wlanDevIdx, rtk_fc_wlan_devmask_t *wlanDevIdMask)
{
	rtk_fc_wlan_devidx_t i;
	bool found = FALSE, duplicate = FALSE;

	*wlanDevIdx = RTK_FC_WLANX_NOT_FOUND;
	*wlanDevIdMask = 0;

	// control path, low performance search is fine.
	for(i = 0; i < RTK_FC_WLANX_END_INTF; i++) 
	{
		if(!fc_mgr_db.wlanDevMap[i].valid)
			continue;
		
		if(fc_mgr_db.wlanDevMap[i].portmap.macPortIdx == macPort) {
			if(found)
				duplicate = TRUE;
			
			found = TRUE;
			*wlanDevIdx = i;
			*wlanDevIdMask |= (1LL<<i);
		}
	}

	if(duplicate)
		*wlanDevIdx = RTK_FC_WLANX_MULTI_INTF;

	return SUCCESS;
	
}



int rtk_fc_wlanDevMask2extMask(rtk_fc_wlan_devmask_t wlanDevIdMask, rtk_fc_ext_port_mask_t *extPortMask)
{
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	int i, wlanDevStart, wlanDevEnd;
	rtk_fc_ext_port_list_t extpidx = 0;

	if(wlanDevIdMask==0)
		return FAIL;
	
	wlanDevStart  = __ffs64(wlanDevIdMask);
	wlanDevEnd = (63 - __builtin_clzll(wlanDevIdMask));

	*extPortMask = FC_EXTMASK_EMPTY;
	
	for(i=wlanDevStart; i<=wlanDevEnd; i++)
	{
		if((1LL<<i) & wlanDevIdMask) {

			extpidx = (fc_mgr_db.wlanDevMap[i].portmap.macExtPortIdx - RTK_FC_MAC_EXT_PORT0);	// offset 1

			if(fc_mgr_db.wlanDevMap[i].portmap.macPortIdx == RTK_FC_MAC_PORT_MAINCPU) {
				//extpidx += FC_EXT_MAC9_PORT0;
			}
#if defined(CONFIG_FC_RTL9607C_SERIES) || defined(CONFIG_FC_RTL9607C_RTL9603CVD_HYBRID)
			else if(fc_mgr_db.wlanDevMap[i].portmap.macPortIdx == RTK_FC_MAC_PORT_MASTERCPU_CORE1) {
				extpidx += FC_EXT_MAC10_PORT0;
			}else if(fc_mgr_db.wlanDevMap[i].portmap.macPortIdx == RTK_FC_MAC_PORT_SLAVECPU) {
				extpidx += FC_EXT_MAC7_PORT0;
			}
#endif
			else {
				// ERROR
			}

			*extPortMask |= (1<<extpidx);			
		}
	}

		
#elif defined (CONFIG_RTK_L34_G3_PLATFORM)

	*extPortMask = wlanDevIdMask;

#endif

	return SUCCESS;
}

int rtk_fc_wlan_tx_qos_mapping_set(uint8 enabled, uint8 mappingTbl[8])
{
	fc_mgr_db.wifi_tx_qos_enable = enabled;
	memcpy(&fc_mgr_db.wifi_tx_qos_mapping[0], &mappingTbl[0], sizeof(fc_mgr_db.wifi_tx_qos_mapping));

	return SUCCESS;
}

int rtk_fc_wlan_rx_lookup_gmac_decision(uint32 spa, uint32 *gmacid)
{
#if defined(CONFIG_FC_RTL9607C_SERIES)	
	if(fc_mgr_db.wifi_rx_gmac_auto_sel_en)
		*gmacid = _rtk_fc_wlan_rx_lookup_gmac_decision_by_port(spa);
	else if(fc_mgr_db.wifi_rx_hash_en)
		*gmacid = _rtk_fc_wlan_rx_lookup_gmac_decision_by_hash();
	else
#endif

		*gmacid = 0;

	return SUCCESS;
}

// wifi register: tx func & port map
int _rtk_fc_wlan_register(rtk_fc_wlan_devidx_t wlanDevIdx, struct net_device *dev)
{

#if defined(CONFIG_RTK_FC_WLAN_HWNAT_ACCELERATION)

	int i = 0, targetInitTblIdx = 0;

	if(dev==NULL) return FAIL;

	FCMGR_PRK("REGISTER wlan dev %s, bind to wlandevid %d\n", dev->name, wlanDevIdx);
	
	// two method to do wlan dev register
	//	#1. call from FC core netdev event, the wlandevid is implemented by string comparison if wlanDevIdx==RTK_FC_WLANX_NOT_FOUND.
	//	#2. call form other device driver, it is used for manual registration if wlanInitMap[i].manuallyReg = 1.
	
	for(i = 0; i < wlanInitMap_size; i++) {
		if((!strncmp(wlanInitMap[i].ifname, dev->name, IFNAMSIZ))) {

			if(wlanDevIdx == RTK_FC_WLANX_NOT_FOUND) {
				//method #1: auto registration
				if(wlanInitMap[i].manuallyReg == 1)
					return FAIL;
				else 
					wlanDevIdx = wlanInitMap[i].wlanDevIdx;
			}
			else if (wlanDevIdx != RTK_FC_WLANX_NOT_FOUND) {		
				// method #2: manual registration	
				if(wlanInitMap[i].manuallyReg == 0)
					return FAIL;

			} 
			
			targetInitTblIdx = i;
			
			break;
		}
	}

	if(wlanDevIdx < RTK_FC_WLANX_END_INTF) {

		struct net_device_ops *pOps=NULL;	
		uint32_t hashidx;
#if defined(CONFIG_FC_CA8277B_SERIES) && defined(CONFIG_FC_SPECIAL_FAST_FORWARD)
		if(wlanInitMap[targetInitTblIdx].portmap.macPortIdx == RTK_FC_MAC_PORT_WLAN_CPU4 ||
			wlanInitMap[targetInitTblIdx].portmap.macPortIdx == RTK_FC_MAC_PORT_WLAN_CPU5 )
		{
			return FAIL;
		}
#endif
		if(fc_mgr_db.wlanDevMap[wlanDevIdx].valid == FALSE) {
			// start registration

			pOps = &fc_mgr_db.wlanDevMap[wlanDevIdx].wlan_fc_ops;
			
			if((fc_mgr_db.wlanDevMap[wlanDevIdx].wlan_native_devops == NULL) && (pOps->ndo_start_xmit != rtk_fc_fastfwd_dev_xmit) ){
				
				FCMGR_PRK("REGISTER wlan dev %s, bind to wlandevid %d\n", dev->name, wlanDevIdx);
				
				// save native tx function and replace to fc wlan tx for egress learning							
				fc_mgr_db.wlanDevMap[wlanDevIdx].wlan_native_devops = dev->netdev_ops;	
				memcpy(pOps,dev->netdev_ops,sizeof(struct net_device_ops));
				pOps->ndo_start_xmit = rtk_fc_fastfwd_dev_xmit;
				dev->netdev_ops = pOps;
		
				fc_mgr_db.wlanDevMap[wlanDevIdx].valid = TRUE;
				fc_mgr_db.wlanDevMap[wlanDevIdx].band = (wlanInitMap[targetInitTblIdx].initBand==RTK_FC_WLAN_INIT_ID_NONE) ?  
																	rtk_fc_wlanDevidx2bandidx_by_idGroup(wlanDevIdx) : 
																	(wlanInitMap[targetInitTblIdx].initBand-1);
				fc_mgr_db.wlanDevMap[wlanDevIdx].wlandevidx = wlanDevIdx;
				fc_mgr_db.wlanDevMap[wlanDevIdx].wlanDev = dev;

				// init wlan dev map

				// sync share port info.
				for(i = 0; i < RTK_FC_WLANX_END_INTF; i++) {
					if((fc_mgr_db.wlanDevMap[i].valid == TRUE) && 
						(fc_mgr_db.wlanDevMap[i].portmap.macPortIdx == wlanInitMap[targetInitTblIdx].portmap.macPortIdx)) {
						
						fc_mgr_db.wlanDevMap[i].shareCpuPort = TRUE;
						fc_mgr_db.wlanDevMap[wlanDevIdx].shareCpuPort = TRUE;

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
						if(fc_mgr_db.wlanDevMap[i].portmap.macExtPortIdx== wlanInitMap[targetInitTblIdx].portmap.macExtPortIdx) {
							fc_mgr_db.wlanDevMap[i].shareExtPort = TRUE;
							fc_mgr_db.wlanDevMap[wlanDevIdx].shareExtPort = TRUE;
						}
#endif
					}
				}
				
				memcpy(&fc_mgr_db.wlanDevMap[wlanDevIdx].portmap, &wlanInitMap[targetInitTblIdx].portmap, sizeof(rtk_fc_pmap_t));

				fc_mgr_db.wlanDevMap[wlanDevIdx].gmac_tx_trunking = wlanInitMap[targetInitTblIdx].gmac_tx_trunking;

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
				{
					uint8 gmacid = 0;
					uint8 extportid = 0;

					gmacid = 0;
					extportid = fc_mgr_db.wlanDevMap[wlanDevIdx].portmap.macExtPortIdx;
					
#if defined(CONFIG_FC_RTL9607C_SERIES) || defined(CONFIG_FC_RTL9607C_RTL9603CVD_HYBRID)
					if(fc_mgr_db.wlanDevMap[wlanDevIdx].portmap.macPortIdx != RTK_FC_MAC_PORT_MASTERCPU_CORE0) {
						if(fc_mgr_db.wlanDevMap[wlanDevIdx].portmap.macPortIdx == RTK_FC_MAC_PORT_MASTERCPU_CORE1) {
							gmacid = 1;
						}
						if(fc_mgr_db.wlanDevMap[wlanDevIdx].portmap.macPortIdx == RTK_FC_MAC_PORT_SLAVECPU) {
							gmacid = 2;
						}
					}
#endif

#if defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT)
					if(gmacid == 2)
					{
						if(fc_mgr_db.wlanDevMap[wlanDevIdx].shareExtPort)
							fc_mgr_db.wlan_port_to_devidx[gmacid][extportid] = RTK_FC_WLANX_MULTI_INTF;
						else
							fc_mgr_db.wlan_port_to_devidx[gmacid][extportid] = wlanDevIdx;
					}
					else
					{
						if(fc_mgr_db.wlanDevMap[wlanDevIdx].shareExtPort)
						{
							fc_mgr_db.wlan_port_to_devidx[0][extportid] = RTK_FC_WLANX_MULTI_INTF;
							fc_mgr_db.wlan_port_to_devidx[1][extportid] = RTK_FC_WLANX_MULTI_INTF;
						}
						else
						{
							fc_mgr_db.wlan_port_to_devidx[0][extportid] = wlanDevIdx;
							fc_mgr_db.wlan_port_to_devidx[1][extportid] = wlanDevIdx;
						}
					}
#elif defined(CONFIG_FC_WIFI_TX_GMAC_AUTO_SEL_SUPPORT)
					{
						if(fc_mgr_db.wlanDevMap[wlanDevIdx].shareExtPort)
						{
							fc_mgr_db.wlan_port_to_devidx[0][extportid] = RTK_FC_WLANX_MULTI_INTF;
							fc_mgr_db.wlan_port_to_devidx[1][extportid] = RTK_FC_WLANX_MULTI_INTF;
							fc_mgr_db.wlan_port_to_devidx[2][extportid] = RTK_FC_WLANX_MULTI_INTF;
						}
						else
						{
							fc_mgr_db.wlan_port_to_devidx[0][extportid] = wlanDevIdx;
							fc_mgr_db.wlan_port_to_devidx[1][extportid] = wlanDevIdx;
							fc_mgr_db.wlan_port_to_devidx[2][extportid] = wlanDevIdx;
						}
					}
					
#else
					if(fc_mgr_db.wlanDevMap[wlanDevIdx].shareExtPort)
						fc_mgr_db.wlan_port_to_devidx[gmacid][extportid] = RTK_FC_WLANX_MULTI_INTF;
					else
						fc_mgr_db.wlan_port_to_devidx[gmacid][extportid] = wlanDevIdx;
#endif
				}
#endif

				hashidx = rtk_fc_wlan_ifidx_devlist_hash(dev);
				fc_mgr_db.wlanDevMap[wlanDevIdx].devHashIdx = hashidx;
				list_add_tail(&fc_mgr_db.wlanDevMap[wlanDevIdx].ifidxDevList, &fc_mgr_db.wlanIfidxDevHead[hashidx]);

				hashidx = rtk_fc_wlan_port_devlist_hash(fc_mgr_db.wlanDevMap[wlanDevIdx].portmap.macPortIdx, fc_mgr_db.wlanDevMap[wlanDevIdx].portmap.macExtPortIdx);
				list_add_tail(&fc_mgr_db.wlanDevMap[wlanDevIdx].portDevList, &fc_mgr_db.wlanPortDevHead[hashidx]);			
			}
			
		}


#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE) && !defined(CONFIG_RTK_FC_WIFI_DRIVER_OFFLOAD_BY_PE)
		if(!strncmp(&dev->name[6], "vxd", 3)) {		// wlan0-vxd
			rtk_fc_wifi_dev_attr_t attr = {0};
			attr.client_mode = TRUE;
			rtk_fc_mgr_global_spin_unlock();
			rtk_fc_wifi_dev_attr_set(dev, attr);
			rtk_fc_mgr_global_spin_lock();
		}
#endif

	}
#else
	//Do nothing, just return success
#endif
	return SUCCESS;
}

int rtk_fc_wlan_unregister(struct net_device *dev)
{

#if defined(CONFIG_RTK_FC_WLAN_HWNAT_ACCELERATION)

	int i = 0, wlanDevIdx = -1;

	if(dev==NULL) return FAIL;
	
	FCMGR_PRK("UN-REGISTER wlan dev %s\n", dev->name);
	
	for(i = 0; i < RTK_FC_WLANX_END_INTF; i++) 
	{
		if(fc_mgr_db.wlanDevMap[i].valid == TRUE 
			&& fc_mgr_db.wlanDevMap[i].wlanDev == dev) 
		{
			wlanDevIdx = i;
			break;
		}
	}
	
	if(wlanDevIdx >= 0)
	{
		list_del(&fc_mgr_db.wlanDevMap[wlanDevIdx].ifidxDevList);
		list_del(&fc_mgr_db.wlanDevMap[wlanDevIdx].portDevList);
		
		dev->netdev_ops = fc_mgr_db.wlanDevMap[wlanDevIdx].wlan_native_devops;
		
		memset(&fc_mgr_db.wlanDevMap[wlanDevIdx], 0, sizeof(rtk_fc_wlan_devmap_t));
	}
	
#else
	//Do nothing, just return success
#endif
	return SUCCESS;
}

char *rtk_fc_wlan_devmap_devname_get(rtk_fc_wlan_devmap_t *devmap)
{
	return devmap->wlanDev->name;
}

int rtk_fc_wlan_devmap_macportidx_get(rtk_fc_wlan_devmap_t *devmap)
{
	return devmap->portmap.macPortIdx;
}

int rtk_fc_wlan_devmap_macextportidx_get(rtk_fc_wlan_devmap_t *devmap)
{
	return devmap->portmap.macExtPortIdx;
}

int rtk_fc_wlan_devmap_shareextport_get(rtk_fc_wlan_devmap_t *devmap)
{
	if(devmap->shareExtPort == TRUE) {
		return TRUE;
	} else {
		return FALSE;
	}
}

int rtk_fc_wlan_devmap_band_get(rtk_fc_wlan_devmap_t *devmap)
{
	return devmap->band;
}

int rtk_fc_wlan_devmap_get(rtk_fc_wlan_devidx_t wlanDevIdx, rtk_fc_wlan_devmap_t **devmap)
{
	if((wlanDevIdx < RTK_FC_WLANX_END_INTF) && (fc_mgr_db.wlanDevMap[wlanDevIdx].valid)) {

		*devmap = &fc_mgr_db.wlanDevMap[wlanDevIdx];	
		return SUCCESS;
	}else{
	
		*devmap = NULL;
		return FAIL;
	}
	
}

int rtk_fc_wlan_devmap_initmap_dump(void)
{
	rtk_fc_wlan_devidx_t wlanDevIdx = 0;
	rtk_fc_wlan_devmap_t *devmap = NULL;


	printk("\nwlan devmap (runtime-config)\n");
	printk("%-8s %-13s %-8s %-8s %-8s %-8s %-12s %-12s %s %-12s\n", "DEVID", "DEV_NAME", "MAC_PORT", "EXTPORT", "PROPERTY", "BAND", "BLOCK_RELAY", "CLIENT_MODE", "TX_TRUNKING", "DEV_PTR");

	for(wlanDevIdx = 0; wlanDevIdx < RTK_FC_WLANX_END_INTF; wlanDevIdx++) {
		
		devmap = &fc_mgr_db.wlanDevMap[wlanDevIdx];

		if(devmap->valid == FALSE)
			continue;
		
		printk("%-8d %-13s %-2d(0x%02x) %-8d %-8s %-8d %-12d %-12d %-12d %px\n", 
				wlanDevIdx, devmap->wlanDev->name, devmap->portmap.macPortIdx, devmap->portmap.macPortIdx, devmap->portmap.macExtPortIdx, 
				devmap->shareExtPort ? "N-to-1" : "1-to-1", devmap->band, devmap->attr.block_relay, devmap->attr.client_mode, devmap->gmac_tx_trunking, devmap->wlanDev);
	}

	printk("\n--------------------------------------------------------------------------------------\n");
	printk("wlan initmap (pre-config)\n");
	printk("%-8s %-13s %-8s %-8s %-8s %-8s %-8s\n", "DEVID", "DEV_NAME", "MAC_PORT", "EXTPORT", "", "BAND", "");
	for(wlanDevIdx = 0; wlanDevIdx < wlanInitMap_size; wlanDevIdx++) {
		if(wlanInitMap[wlanDevIdx].wlanDevIdx == RTK_FC_WLAN_INVALID_INTF)
			continue;
		printk("%-8d %-13s %-2d(0x%02x) %-8d %-8s %-4s(%d)\n", 
				wlanInitMap[wlanDevIdx].wlanDevIdx, wlanInitMap[wlanDevIdx].ifname, 
				wlanInitMap[wlanDevIdx].portmap.macPortIdx, wlanInitMap[wlanDevIdx].portmap.macPortIdx, wlanInitMap[wlanDevIdx].portmap.macExtPortIdx,
				"", (wlanInitMap[wlanDevIdx].initBand == RTK_FC_WLAN_INIT_ID_NONE) ? "Auto" : "Fix", (wlanInitMap[wlanDevIdx].initBand == RTK_FC_WLAN_INIT_ID_NONE) ? RTK_FC_WLAN_INIT_ID_MAX : (wlanInitMap[wlanDevIdx].initBand-1));
	}

#if defined(CONFIG_FC_RTL9607C_SERIES)	
	{
		int spa = 0;
		printk("\n--------------------------------------------------------------------------------------\n");
		printk("wifi_rx_gmac_sel table:   (auto_detect_en: %d)\n", fc_mgr_db.wifi_rx_gmac_auto_sel_en);
		printk("%-8s %-13s\n", "SPA", "MODE");
		for(spa = RTK_FC_MAC_PORT0; spa < RTK_FC_MAC_PORT_MAX; spa++) {
			
			printk("%-8d %-13d\n", spa, fc_mgr_db.wifi_rx_gmac_sel[spa]);
		}
	}
#endif
	
	return SUCCESS;
}

int rtk_fc_wlan_devmap_initmap_set(rtk_fc_wlan_initmap_t wlanConfig)
{
	// wlanConfig.initBand == RTK_FC_WLAN_INIT_ID_NONE : DEL cmd
	// wlanConfig.initBand == RTK_FC_WLAN_INIT_ID_0-2 : ADD/UPDATE cmd
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	rtk_fc_wlan_devidx_t i = 0, firstFreeDevIdx = RTK_FC_WLAN_INVALID_INTF;
	rtk_fc_wlan_devmask_t	allDevMask = 0LL;
	rtk_fc_wlan_devmap_t *devmap = NULL;
	uint8 doDel = (wlanConfig.initBand == RTK_FC_WLAN_INIT_ID_NONE) ? TRUE : FALSE;
	uint8 find = FALSE;
	int32 firstFreeInitIdx = -1;

	// run-time config
	for(i = 0; i < RTK_FC_WLANX_END_INTF; i++) {
		devmap = &fc_mgr_db.wlanDevMap[i];
		if(devmap->valid == TRUE && ((!strncmp(devmap->wlanDev->name, wlanConfig.ifname, IFNAMSIZ)) || (devmap->wlandevidx == wlanConfig.wlanDevIdx))) {
			if(doDel) {
				rtk_fc_wlan_unregister(devmap->wlanDev);
			}else {
				// update! modify port & band only.
				devmap->portmap.macPortIdx = wlanConfig.portmap.macPortIdx;
				devmap->band = (wlanConfig.initBand-RTK_FC_WLAN_INIT_ID_0);
			}
			break;
		}
	}

	// initial config
	for(i = 0; i < wlanInitMap_size; i++) {
		if(firstFreeInitIdx == -1 && wlanInitMap[i].wlanDevIdx == RTK_FC_WLAN_INVALID_INTF)
			firstFreeInitIdx = i;

		allDevMask |= (1LL<<wlanInitMap[i].wlanDevIdx);
		
		if((!strncmp(wlanInitMap[i].ifname, wlanConfig.ifname, IFNAMSIZ)) || (wlanInitMap[i].wlanDevIdx == wlanConfig.wlanDevIdx)) {
			find = TRUE;
			FCMGR_PRK("update entry[%d] devIdx %d\n", i, wlanInitMap[i].wlanDevIdx);
			if(doDel)
				memset(&wlanInitMap[i], 0, sizeof(rtk_fc_wlan_initmap_t));
			else {
				// update
				wlanInitMap[i].portmap.macPortIdx = wlanConfig.portmap.macPortIdx;
				wlanInitMap[i].initBand = wlanConfig.initBand;
			}
		}
	}
	
	FCMGR_PRK("debug: first freeidx %d find %d\n", firstFreeInitIdx, find);
	if(find == FALSE  && firstFreeInitIdx>=0 && firstFreeInitIdx < RTK_FC_WLANX_END_INTF) {
		// add
		allDevMask = ~allDevMask;
		firstFreeDevIdx = __ffs64(allDevMask); 
		
		FCMGR_PRK("add entry[%d] with devIdx %d (all free mask 0x%llx)\n", firstFreeInitIdx, firstFreeDevIdx, allDevMask);
		memcpy(&wlanInitMap[firstFreeInitIdx], &wlanConfig, sizeof(rtk_fc_wlan_initmap_t));
		wlanInitMap[firstFreeInitIdx].wlanDevIdx = firstFreeDevIdx;
	}
#endif

	return SUCCESS;
}

__IRAM_FC_NICTRX
rtk_fc_devtx_t rtk_fc_wlan_hard_start_xmit(rtk_fc_wlan_devidx_t wlan_dev_idx, struct sk_buff *skb, fc_rx_info_t *pRxInfo)
{
	rtk_fc_devtx_t wlanrc = RTK_FC_DEVTX_OK;
#if defined(CONFIG_SMP) && defined(CONFIG_RTK_L34_FC_IPI_WIFI_TX)
	rtk_fc_mgr_dispatch_mode_t *wlanTxDispatch;
#endif

	if(unlikely((wlan_dev_idx >= RTK_FC_WLANX_END_INTF) || (fc_mgr_db.wlanDevMap[wlan_dev_idx].valid == FALSE))) {
		FCMGR_ERR("ERROR: dev %s wlandevidx %d skb %p \n", skb->dev->name, wlan_dev_idx, skb); 
		dev_kfree_skb_any(skb);	
		return RTK_FC_DEVTX_ERROR;
	}
	
	skb->dev = fc_mgr_db.wlanDevMap[wlan_dev_idx].wlanDev;

	if(unlikely(!netif_running(skb->dev))){
		FCMGR_ERR("ERROR: dev %s wlandevidx %d skb %p dev not running\n", skb->dev->name, wlan_dev_idx, skb); 
		dev_kfree_skb_any(skb);	
		return RTK_FC_DEVTX_ERROR;
	}

	if(fc_mgr_db.wifi_tx_qos_enable && wlan_dev_idx >= RTK_FC_WLAN0_ROOT_INTF && wlan_dev_idx <= RTK_FC_WLAN2_MESH_INTF) 
	{
		if(pRxInfo!=NULL) {
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
			if(RXINFO_IS_WIFI_FF(pRxInfo) == 2)
			{
				uint8 cos = RXINFO_INT_PRI(pRxInfo)%8;
				switch(cos)
				{
					case 0:
						/* fall through */
					case 1:
						/* fall through */
					case 2:
						/* fall through */
					case 3:
						skb->cb[0] = 1 + (cos << 1);
						break;
					case 4:
						/* fall through */
					case 5:
						/* fall through */
					case 6:
						/* fall through */
					case 7:
						skb->cb[0] = 1 + ((cos - 4) << 1);
						break;
					default:
						break;
				}
				FCMGR_PRK("[TX_AMSDU] wifi WMM mapping cos=%u to skb->cb[0]=%d\n", cos, skb->cb[0]);
			}
			else
#endif
			{
				skb->cb[0] = fc_mgr_db.wifi_tx_qos_mapping[RXINFO_INT_PRI(pRxInfo)%8];
				FCMGR_PRK("wifi WMM mapping cos=%d to skb->cb[0]=%d\n", RXINFO_INT_PRI(pRxInfo), skb->cb[0]);
			}
		}
	}

#if defined(CONFIG_RTK_FC_ADSL_HWNAT_ACCELERATION)	
	if(unlikely(wlan_dev_idx >= RTK_FC_WLANx_ATM_VC0_INTF && wlan_dev_idx <= RTK_FC_WLANx_ATM_VC7_INTF)) 
	{
		if(pRxInfo!=NULL) {
			/* fast forward handling */
			if(fc_mgr_db.mgr_skbmark[FC_MGR_SKBMARK_QID].startBit != RTK_FC_MGR_RMK_UNDEF) {
				_rtk_fc_set_skbMark_vlaue(skb, FC_MGR_SKBMARK_QID, RXINFO_INT_PRI(pRxInfo));
				FCMGR_PRK("fast forward to devidx %d, skb->mark = 0x%x", wlan_dev_idx, skb->mark);
			}
		}
	}
#endif

#if defined(CONFIG_RTK_SOC_RTL8198D)
	if (rtk_fc_is_highpri_pkt(skb->data))
		goto direct_tx;
#endif

#if defined(CONFIG_SMP) && defined(CONFIG_RTK_L34_FC_IPI_WIFI_TX)
	wlanTxDispatch = &fc_mgr_db.smp_dispatch[RTK_FC_MGR_DISPATCH_WLAN0_TX + fc_mgr_db.wlanDevMap[wlan_dev_idx].band];

	if(unlikely(wlanTxDispatch->mode == RTK_FC_DISPATCH_MODE_IPI)
#if defined(CONFIG_RTK_L34_G3_PLATFORM) && defined(CONFIG_FC_QTNA_WIFI_AX)
		&& unlikely(!(pRxInfo!=NULL && RXINFO_IS_WIFI_FF(pRxInfo) && fc_mgr_db.isWifiFF_skip_dispatch))
#endif
		) {
		rtk_fc_wlantx_info_t wlanTxInfo={0};
		wlanTxInfo.skb = skb;
		wlanTxInfo.wlandevidx = wlan_dev_idx;
		
		// IPI tx enqueue
		rtk_fc_smp_wlan_tx_dispatch(&wlanTxInfo);

	}else 
#endif
	{
#if defined(CONFIG_RTK_SOC_RTL8198D)
direct_tx:
#endif

#if defined(CONFIG_RTL_ETH_RECYCLED_SKB_DEBUG)
		{
			rtl_recycle_skb_debug_t rtl_skb_debug_parameter;
			rtl_skb_debug_parameter.cb=fc_mgr_db.rtl_fc_skb_cb_value;
			rtl_skb_debug_parameter.check_skb=skb;
			rtl_skb_debug_parameter.skb_pointer_off=-1;
			rtl_skb_debug_parameter.headroom=-1;
			rtl_skb_debug_parameter.dump_stack_en=0;
			rtl_recycle_skb_cb_section_enter(&rtl_skb_debug_parameter);
			rtl_recycle_skb_cb_section_leave(&rtl_skb_debug_parameter);
		}
#endif

		// direct tx
		wlanrc = fc_mgr_db.wlanDevMap[wlan_dev_idx].wlan_native_devops->ndo_start_xmit(skb, skb->dev);
#if defined(CONFIG_FC_QTNA_WIFI_AX)
		if(unlikely(wlanrc != RTK_FC_DEVTX_OK))
			dev_kfree_skb_any(skb); //free skb here
#endif
		if(unlikely(fc_mgr_db.smpStatistic)){
		
			if(RTK_FC_DEVTX_OK == wlanrc) {
				atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_WIFI_BAND0_TX+fc_mgr_db.wlanDevMap[wlan_dev_idx].band].smp_static[smp_processor_id()]);
			}else {
				atomic_inc(&fc_mgr_db.mgr_smp_statistic[FC_MGR_SMP_WIFI_BAND0_TX_DROP+fc_mgr_db.wlanDevMap[wlan_dev_idx].band].smp_static[smp_processor_id()]);
			}
		}
		
	}

	return wlanrc;
}

int rtk_fc_wlan_system_post_init(void)
{
	// mamager module system init should be later than core module init
#if (defined(CONFIG_RTK_L34_XPON_PLATFORM) &&  defined(CONFIG_GMAC2_USABLE)) || defined(CONFIG_RTK_CPU_PORT_FLOW_CONTROL)
	rtk_port_macAbility_t cpuAbility;
#endif

#if (defined(CONFIG_RTK_L34_XPON_PLATFORM) &&  defined(CONFIG_GMAC2_USABLE))
	memset(&cpuAbility,0,sizeof(cpuAbility));
	cpuAbility.duplex = PORT_FULL_DUPLEX;
	cpuAbility.linkStatus = PORT_LINKUP;
	cpuAbility.speed = PORT_SPEED_1000M;
#if defined(CONFIG_RTK_CPU_PORT_FLOW_CONTROL)
	cpuAbility.txFc=ENABLED;
	cpuAbility.rxFc=ENABLED;
#endif
	if(rtk_port_macForceAbility_set(RTK_FC_MAC_PORT_SLAVECPU,cpuAbility))FCMGR_ERR("Set slave CPU port fail!!!\n");
	if(rtk_port_macForceAbilityState_set(RTK_FC_MAC_PORT_SLAVECPU,ENABLED))FCMGR_ERR("Set slave CPU port fail!!!\n");
			
	rtk_cpu_tagAwareByPort_set(RTK_FC_MAC_PORT_SLAVECPU,ENABLED);
	rtk_cpu_trapInsertTagByPort_set(RTK_FC_MAC_PORT_SLAVECPU,ENABLED);
#endif

#if defined(CONFIG_RTK_L34_XPON_PLATFORM) && defined(CONFIG_FC_RTL9607C_SERIES) && defined(CONFIG_LAN_SDS1_FEATURE)
	// 9607C only: take switch port 7 as SDS interface, enable src port block feature. 
	{
		rtk_portmask_t srcPortBlocking;
		rtk_l2_srcPortEgrFilterMask_get(&srcPortBlocking);
		srcPortBlocking.bits[0] |= RTK_FC_MAC_PORT_SLAVECPU;
		rtk_l2_srcPortEgrFilterMask_set(&srcPortBlocking);
	}
#endif

#if defined(CONFIG_RTK_CPU_PORT_FLOW_CONTROL)
	/*Enable cpu port flow control*/
	if(rtk_port_macForceAbility_get(RTK_FC_MAC_PORT_MAINCPU,&cpuAbility)) {
		FCMGR_ERR("Get main CPU port flow control failed!!!\n");
	} else {
		cpuAbility.txFc=ENABLED;
		cpuAbility.rxFc=ENABLED;
		if(rtk_port_macForceAbility_set(RTK_FC_MAC_PORT_MAINCPU,cpuAbility))FCMGR_ERR("Get main CPU port flow control failed!!!\n");
	}

	if(rtk_port_macForceAbility_get(RTK_FC_MAC_PORT_LASTCPU,&cpuAbility)) {
		FCMGR_ERR("Get last CPU port flow control failed!!!\n");
	} else {
		cpuAbility.txFc=ENABLED;
		cpuAbility.rxFc=ENABLED;
		if(rtk_port_macForceAbility_set(RTK_FC_MAC_PORT_LASTCPU,cpuAbility))FCMGR_ERR("Set last CPU port flow control failed!!!\n");
	}
#endif

#if defined(CONFIG_RTK_L34_XPON_PLATFORM) && defined(CONFIG_FC_RTL9607C_SERIES)

#if (defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT) || defined(CONFIG_FC_WIFI_RX_HASH_SUPPORT) || defined(CONFIG_FC_WIFI_RX_GMAC_AUTO_SEL_SUPPORT))
	rtk_l2_illegalPortMoveAction_set(RTK_FC_MAC_PORT_MASTERCPU_CORE1, ACTION_FORWARD); // wifi RX packets may use port 9 or port 10 (Its MAC address will be learned in port 9)
	rtk_l2_illegalPortMoveAction_set(RTK_FC_MAC_PORT_SLAVECPU, ACTION_FORWARD); // wifi RX packets may use port 9, 10, 7
#endif

	//rtk_trap_cpuTrapHashState_set(ENABLED);
	rtk_trap_cpuTrapHashMask_set(TRAP_HASH_SPA, ENABLED);
	rtk_trap_cpuTrapHashMask_set(TRAP_HASH_SMAC, ENABLED);
	rtk_trap_cpuTrapHashMask_set(TRAP_HASH_DMAC, ENABLED);
	rtk_trap_cpuTrapHashMask_set(TRAP_HASH_SIP_INNER, ENABLED);
	rtk_trap_cpuTrapHashMask_set(TRAP_HASH_DIP_INNER, ENABLED);
	rtk_trap_cpuTrapHashMask_set(TRAP_HASH_SPORT_INNER, ENABLED);
	rtk_trap_cpuTrapHashMask_set(TRAP_HASH_DPORT_INNER, ENABLED);

	rtk_trap_cpuTrapHashPort_set(0, RTK_FC_MAC_PORT_MASTERCPU_CORE0);
	rtk_trap_cpuTrapHashPort_set(1, RTK_FC_MAC_PORT_MASTERCPU_CORE1);
	rtk_trap_cpuTrapHashPort_set(2, RTK_FC_MAC_PORT_MASTERCPU_CORE0);
	rtk_trap_cpuTrapHashPort_set(3, RTK_FC_MAC_PORT_MASTERCPU_CORE1);
	rtk_trap_cpuTrapHashPort_set(4, RTK_FC_MAC_PORT_MASTERCPU_CORE0);
	rtk_trap_cpuTrapHashPort_set(5, RTK_FC_MAC_PORT_MASTERCPU_CORE1);
	rtk_trap_cpuTrapHashPort_set(6, RTK_FC_MAC_PORT_MASTERCPU_CORE0);
	rtk_trap_cpuTrapHashPort_set(7, RTK_FC_MAC_PORT_MASTERCPU_CORE1);
	rtk_trap_cpuTrapHashPort_set(8, RTK_FC_MAC_PORT_MASTERCPU_CORE0);
	rtk_trap_cpuTrapHashPort_set(9, RTK_FC_MAC_PORT_MASTERCPU_CORE1);
	rtk_trap_cpuTrapHashPort_set(10, RTK_FC_MAC_PORT_MASTERCPU_CORE0);
	rtk_trap_cpuTrapHashPort_set(11, RTK_FC_MAC_PORT_MASTERCPU_CORE1);
	rtk_trap_cpuTrapHashPort_set(12, RTK_FC_MAC_PORT_MASTERCPU_CORE0);
	rtk_trap_cpuTrapHashPort_set(13, RTK_FC_MAC_PORT_MASTERCPU_CORE1);
	rtk_trap_cpuTrapHashPort_set(14, RTK_FC_MAC_PORT_MASTERCPU_CORE0);
	rtk_trap_cpuTrapHashPort_set(15, RTK_FC_MAC_PORT_MASTERCPU_CORE1);

#if defined(CONFIG_FC_WIFI_TRAP_HASH_SUPPORT)
	// HW hash
	rtk_trap_cpuTrapHashState_set(ENABLED);
#else
	// fully decided by flow 
	rtk_trap_cpuTrapHashState_set(DISABLED);
#endif
#endif

	return SUCCESS;
}


int rtk_fc_wlan_init(void)
{
	int i = 0;

#if defined(CONFIG_FC_RTL9607C_RTL9603CVD_HYBRID)
	for(i = 0; i <wlanInitMap_size; i++) {
		
		if(fc_mgr_db.chipId == RTL9607C_CHIP_ID) {
			// 3 GMACs
			if (wlanInitMap[i].wlanDevIdx == RTK_FC_WLANx_USB_INTF)
				wlanInitMap[i].portmap.macPortIdx = 10;
			else if(wlanInitMap[i].wlanDevIdx < RTK_FC_WLAN1_ROOT_INTF)
				wlanInitMap[i].portmap.macPortIdx = 9;
			else if(wlanInitMap[i].wlanDevIdx < RTK_FC_WLAN2_ROOT_INTF)
				wlanInitMap[i].portmap.macPortIdx = 10;
			else
				wlanInitMap[i].portmap.macPortIdx = 7;
				
		}else{
			// 1 GMAC
			wlanInitMap[i].portmap.macPortIdx = 5;
		}
	}
#endif

	for(i=0;i<RTK_FC_WLAN_PORT_BUCKET_SIZE;i++)
		INIT_LIST_HEAD(&fc_mgr_db.wlanPortDevHead[i]);

	
	for(i=0;i<RTK_FC_WLANX_END_INTF;i++) {
		INIT_LIST_HEAD(&fc_mgr_db.wlanIfidxDevHead[i]);
		
		INIT_LIST_HEAD(&fc_mgr_db.wlanDevMap[i].ifidxDevList);
		INIT_LIST_HEAD(&fc_mgr_db.wlanDevMap[i].portDevList);
	}

	return SUCCESS;
}



int rtk_fc_dev_is_wlan_dev(struct net_device *dev, bool *wlandev)
{
	int i = 0;
	
	*wlandev = FALSE;
	
	if (rtk_fc_dev_is_priv_flags_set(dev, RTK_FC_NETDEV_PRIV_FLAG_TYPE_DOMAIN_WLAN))
		*wlandev = TRUE;
	else if (rtk_fc_dev_is_priv_flags_set(dev, RTK_FC_NETDEV_PRIV_FLAG_TYPE_DOMAIN_ELAN))
		*wlandev = FALSE;	
	else if (rtk_fc_dev_is_priv_flags_set(dev, RTK_FC_NETDEV_PRIV_FLAG_TYPE_DOMAIN_WAN))
		*wlandev = FALSE;
	else if(!strncmp(dev->name, "wlan", 4))
		*wlandev = TRUE;
#if defined(CONFIG_RTK_FC_ADSL_HWNAT_ACCELERATION)
	else if(!strncmp(dev->name, "vc", 2) && dev->name[3] == '\0')
		*wlandev = TRUE;
#endif
#if defined(CONFIG_RTL_FC_USB_INTF)
#if defined(CONFIG_RTL_FC_USB_AUTO_INTFNAME)
	else if(usb_idx && !strcmp(dev->name, wlanInitMap[usb_idx].ifname))
		*wlandev = TRUE;
#else
	else if(!strcmp(dev->name, "eth1"))
		*wlandev = TRUE;
#endif
#endif
#if defined(CONFIG_FC_QTNA_WIFI_AX)
	else if(!strcmp(dev->name, RTK_FC_QTNA_WIFI_DEV_NAME))
		*wlandev = TRUE;
#endif
	else {
		//match any one
		for(i = 0; i <wlanInitMap_size; i++) {
			if(!strncmp(wlanInitMap[i].ifname, dev->name, IFNAMSIZ))
				*wlandev = TRUE;
		}
	}

	return SUCCESS;	
}



int rtk_fc_wlan_rtmbssid2devidx(rt_wlan_index_t wlanIdx, rt_wlan_mbssid_index_t mbssidIdx, rtk_fc_wlan_devidx_t *wlanDevIdx)
{
	*wlanDevIdx = RTK_FC_WLANX_NOT_FOUND;

#if defined(CONFIG_COMMON_RT_API) && defined(CONFIG_RTK_FC_WLAN_HWNAT_ACCELERATION)

	if(wlanIdx == RT_WLAN_DEVICE0_INDEX) {
		switch(mbssidIdx) {
			case RT_WLAN_MBSSID_ROOT_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN0_ROOT_INTF;
				break;
			case RT_WLAN_MBSSID_VAP0_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN0_VAP0_INTF;
				break;
			case RT_WLAN_MBSSID_VAP1_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN0_VAP1_INTF;
				break;
			case RT_WLAN_MBSSID_VAP2_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN0_VAP2_INTF;
				break;
			case RT_WLAN_MBSSID_VAP3_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN0_VAP3_INTF;
				break;
			case RT_WLAN_MBSSID_VAP4_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN0_VAP4_INTF;
				break;
			case RT_WLAN_MBSSID_VAP5_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN0_VAP5_INTF;
				break;
			case RT_WLAN_MBSSID_VAP6_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN0_VAP6_INTF;
				break;
			case RT_WLAN_MBSSID_VAP7_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN0_VAP7_INTF;
				break;
			case RT_WLAN_MBSSID_CLIENT_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN0_CLIENT_INTF;
				break;
			case RT_WLAN_MBSSID_MESH_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN0_MESH_INTF;
				break;
			default:
				*wlanDevIdx = RTK_FC_WLANX_NOT_FOUND;
				break; 
		}
	}else if(wlanIdx == RT_WLAN_DEVICE1_INDEX) {
		switch(mbssidIdx) {
			case RT_WLAN_MBSSID_ROOT_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN1_ROOT_INTF;
				break;
			case RT_WLAN_MBSSID_VAP0_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN1_VAP0_INTF;
				break;
			case RT_WLAN_MBSSID_VAP1_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN1_VAP1_INTF;
				break;
			case RT_WLAN_MBSSID_VAP2_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN1_VAP2_INTF;
				break;
			case RT_WLAN_MBSSID_VAP3_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN1_VAP3_INTF;
				break;
			case RT_WLAN_MBSSID_VAP4_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN1_VAP4_INTF;
				break;
			case RT_WLAN_MBSSID_VAP5_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN1_VAP5_INTF;
				break;
			case RT_WLAN_MBSSID_VAP6_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN1_VAP6_INTF;
				break;
			case RT_WLAN_MBSSID_VAP7_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN1_VAP7_INTF;
				break;
			case RT_WLAN_MBSSID_CLIENT_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN1_CLIENT_INTF;
				break;
			case RT_WLAN_MBSSID_MESH_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN1_MESH_INTF;
				break;
			default:
				*wlanDevIdx = RTK_FC_WLANX_NOT_FOUND;
				break; 
		}
	}else if(wlanIdx == RT_WLAN_DEVICE2_INDEX) {
		switch(mbssidIdx) {
			case RT_WLAN_MBSSID_ROOT_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN2_ROOT_INTF;
				break;
			case RT_WLAN_MBSSID_VAP0_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN2_VAP0_INTF;
				break;
			case RT_WLAN_MBSSID_VAP1_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN2_VAP1_INTF;
				break;
			case RT_WLAN_MBSSID_VAP2_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN2_VAP2_INTF;
				break;
			case RT_WLAN_MBSSID_VAP3_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN2_VAP3_INTF;
				break;
			case RT_WLAN_MBSSID_VAP4_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN2_VAP4_INTF;
				break;
			case RT_WLAN_MBSSID_VAP5_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN2_VAP5_INTF;
				break;
			case RT_WLAN_MBSSID_VAP6_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN2_VAP6_INTF;
				break;
			case RT_WLAN_MBSSID_VAP7_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN2_VAP7_INTF;
				break;
			case RT_WLAN_MBSSID_CLIENT_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN2_CLIENT_INTF;
				break;
			case RT_WLAN_MBSSID_MESH_INTF_INDEX:
				*wlanDevIdx = RTK_FC_WLAN2_MESH_INTF;
				break;
			default:
				*wlanDevIdx = RTK_FC_WLANX_NOT_FOUND;
				break; 
		}
	}

#endif // CONFIG_COMMON_RT_API

	return SUCCESS;
}

int rtk_fc_wlan_rtmbssidMask2devMask(rt_wlan_mbssid_mask_t *rtWlanMbssidMsk,rtk_fc_wlan_devmask_t *wlanDevIdMask)
{
#if defined(CONFIG_COMMON_RT_API) && defined(CONFIG_RTK_FC_WLAN_HWNAT_ACCELERATION)
	int i;
	rt_wlan_mbssid_index_t mbssid_index=0;
	rtk_fc_wlan_devidx_t wlanDevIdx;
	*wlanDevIdMask =0;

	for(i=0 ;i<RT_WLAN_DEVICE_MAX ;i++)
	{
		for(mbssid_index=0;mbssid_index<RT_WLAN_MBSSID_MAX;mbssid_index++)
		{
			if((1<<mbssid_index) & rtWlanMbssidMsk[i]) {
				rtk_fc_wlan_rtmbssid2devidx(i, mbssid_index, &wlanDevIdx);
				if(wlanDevIdx < RTK_FC_WLANX_END_INTF)
					*wlanDevIdMask |= (1LL<<wlanDevIdx);
				else {
					FCMGR_ERR("translate rt wlan %d ssidmsk 0x%x but ssid %d is failed to mapping to fc dev idx", i, rtWlanMbssidMsk[i], mbssid_index);
				}
			}
		}
	}
#endif // CONFIG_COMMON_RT_API

	return SUCCESS;
}

int _rtk_fc_wlan_devMask2RtmbssidMask(rtk_fc_wlan_devmask_t *wlanDevIdMask, rt_wlan_mbssid_mask_t *rtWlanMbssidMsk)
{
#if defined(CONFIG_COMMON_RT_API) && defined(CONFIG_RTK_FC_WLAN_HWNAT_ACCELERATION)
	rtWlanMbssidMsk[0]=0x0;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN0_ROOT_INTF)
		rtWlanMbssidMsk[0]|=RT_WLAN_MBSSID_ROOT_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN0_VAP0_INTF)
		rtWlanMbssidMsk[0]|=RT_WLAN_MBSSID_VAP0_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN0_VAP1_INTF)
		rtWlanMbssidMsk[0]|=RT_WLAN_MBSSID_VAP1_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN0_VAP2_INTF)
		rtWlanMbssidMsk[0]|=RT_WLAN_MBSSID_VAP2_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN0_VAP3_INTF)
		rtWlanMbssidMsk[0]|=RT_WLAN_MBSSID_VAP3_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN0_VAP4_INTF)
		rtWlanMbssidMsk[0]|=RT_WLAN_MBSSID_VAP4_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN0_VAP5_INTF)
		rtWlanMbssidMsk[0]|=RT_WLAN_MBSSID_VAP5_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN0_VAP6_INTF)
		rtWlanMbssidMsk[0]|=RT_WLAN_MBSSID_VAP6_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN0_VAP7_INTF)
		rtWlanMbssidMsk[0]|=RT_WLAN_MBSSID_VAP7_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN0_CLIENT_INTF)
		rtWlanMbssidMsk[0]|=RT_WLAN_MBSSID_CLIENT_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN0_MESH_INTF)
		rtWlanMbssidMsk[0]|=RT_WLAN_MBSSID_MESH_INTF_BIT;

	rtWlanMbssidMsk[1]=0x0;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN1_ROOT_INTF)
		rtWlanMbssidMsk[1]|=RT_WLAN_MBSSID_ROOT_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN1_VAP0_INTF)
		rtWlanMbssidMsk[1]|=RT_WLAN_MBSSID_VAP0_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN1_VAP1_INTF)
		rtWlanMbssidMsk[1]|=RT_WLAN_MBSSID_VAP1_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN1_VAP2_INTF)
		rtWlanMbssidMsk[1]|=RT_WLAN_MBSSID_VAP2_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN1_VAP3_INTF)
		rtWlanMbssidMsk[1]|=RT_WLAN_MBSSID_VAP3_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN1_VAP4_INTF)
		rtWlanMbssidMsk[1]|=RT_WLAN_MBSSID_VAP4_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN1_VAP5_INTF)
		rtWlanMbssidMsk[1]|=RT_WLAN_MBSSID_VAP5_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN1_VAP6_INTF)
		rtWlanMbssidMsk[1]|=RT_WLAN_MBSSID_VAP6_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN1_VAP7_INTF)
		rtWlanMbssidMsk[1]|=RT_WLAN_MBSSID_VAP7_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN1_CLIENT_INTF)
		rtWlanMbssidMsk[1]|=RT_WLAN_MBSSID_CLIENT_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN1_MESH_INTF)
		rtWlanMbssidMsk[1]|=RT_WLAN_MBSSID_MESH_INTF_BIT;
	rtWlanMbssidMsk[2]=0x0;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN2_ROOT_INTF)
		rtWlanMbssidMsk[2]|=RT_WLAN_MBSSID_ROOT_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN2_VAP0_INTF)
		rtWlanMbssidMsk[2]|=RT_WLAN_MBSSID_VAP0_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN2_VAP1_INTF)
		rtWlanMbssidMsk[2]|=RT_WLAN_MBSSID_VAP1_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN2_VAP2_INTF)
		rtWlanMbssidMsk[2]|=RT_WLAN_MBSSID_VAP2_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN2_VAP3_INTF)
		rtWlanMbssidMsk[2]|=RT_WLAN_MBSSID_VAP3_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN2_VAP4_INTF)
		rtWlanMbssidMsk[2]|=RT_WLAN_MBSSID_VAP4_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN2_VAP5_INTF)
		rtWlanMbssidMsk[2]|=RT_WLAN_MBSSID_VAP5_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN2_VAP6_INTF)
		rtWlanMbssidMsk[2]|=RT_WLAN_MBSSID_VAP6_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN2_VAP7_INTF)
		rtWlanMbssidMsk[2]|=RT_WLAN_MBSSID_VAP7_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN2_CLIENT_INTF)
		rtWlanMbssidMsk[2]|=RT_WLAN_MBSSID_CLIENT_INTF_BIT;
	if(*wlanDevIdMask&1LL<<RTK_FC_WLAN2_MESH_INTF)
		rtWlanMbssidMsk[2]|=RT_WLAN_MBSSID_MESH_INTF_BIT;
#endif // CONFIG_COMMON_RT_API

	return SUCCESS;
}

bool rtk_fc_wlan_client_mode_cb_is_registered(void)
{
	return fc_mgr_db._rtk_fc_wifi_client_mode_cb ? TRUE : FALSE;
}

#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
int _rtk_fc_wifi_amsdu_pe_offload_mac_id_set(unsigned int mac_id, rtk_fc_wifi_amsdu_pe_offload_sta_conf_sel_t sta_conf_sel, rtk_fc_wifi_amsdu_pe_offload_sta_info_t sta_conf, unsigned char* mac_addr)
{
	if(!rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_set(mac_id, sta_conf_sel, sta_conf, mac_addr))
		return RTK_FC_HELPER_RET_OK;
	else
		return RTK_FC_HELPER_RET_ERR;
}

int _rtk_fc_wifi_amsdu_pe_offload_mac_id_del(unsigned int mac_id)
{
	if(!rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_del(mac_id))
		return RTK_FC_HELPER_RET_OK;
	else
		return RTK_FC_HELPER_RET_ERR;
}

int _rtk_fc_wifi_amsdu_pe_offload_mac_id_flush(void)
{
	if(!rtk_fc_internal_wifi_amsdu_pe_offload_mac_id_flush())
		return RTK_FC_HELPER_RET_OK;
	else
		return RTK_FC_HELPER_RET_ERR;
}

int _rtk_fc_wifi_amsdu_pe_offload_mode_set(rtk_fc_wifi_amsdu_pe_offload_mode_t mode)
{
	if(!rtk_fc_internal_wifi_amsdu_pe_offload_mode_set(mode))
		return RTK_FC_HELPER_RET_OK;
	else
		return RTK_FC_HELPER_RET_ERR;
}
int _rtk_fc_wifi_amsdu_pe_offload_mode_get(rtk_fc_wifi_amsdu_pe_offload_mode_t *mode)
{
	if(!rtk_fc_internal_wifi_amsdu_pe_offload_mode_get(mode))
		return RTK_FC_HELPER_RET_OK;
	else
		return RTK_FC_HELPER_RET_ERR;

}

int _rtk_fc_wifi_dev_attr_set(struct net_device *dev, rtk_fc_wifi_dev_attr_t attr)
{
	int ret = 0;
	rtk_fc_wlan_devidx_t wlanDevIdx = RTK_FC_WLANX_NOT_FOUND;

	if(dev==NULL)
		return RTK_FC_HELPER_RET_ERR;

	_rtk_fc_dev2wlanDevIdx(dev, &wlanDevIdx);
	
	if(wlanDevIdx >= RTK_FC_WLANX_END_INTF)
		return RTK_FC_HELPER_RET_ERR;

	// additional opertation for core module
	rtk_fc_internal_wifi_dev_attr_set(wlanDevIdx, fc_mgr_db.wlanDevMap[wlanDevIdx].attr, attr);

	// set new config
	rtk_fc_mgr_global_spin_lock_bh();
	memcpy(&fc_mgr_db.wlanDevMap[wlanDevIdx].attr, &attr, sizeof(rtk_fc_wifi_dev_attr_t));
	rtk_fc_mgr_global_spin_unlock_bh();

	return ret ;
}
int _rtk_fc_wifi_dev_attr_get(struct net_device *dev, rtk_fc_wifi_dev_attr_t *attr)
{
	int ret = 0;
	rtk_fc_wlan_devidx_t wlanDevIdx = RTK_FC_WLANX_NOT_FOUND;

	if(dev==NULL)
		return RTK_FC_HELPER_RET_ERR;

	rtk_fc_mgr_global_spin_lock_bh();

	if((ret = _rtk_fc_dev2wlanDevIdx(dev, &wlanDevIdx)) == SUCCESS) {
		memcpy(attr, &fc_mgr_db.wlanDevMap[wlanDevIdx].attr, sizeof(rtk_fc_wifi_dev_attr_t));
	}
	
	rtk_fc_mgr_global_spin_unlock_bh();

	return ret ;
}

int _rtk_fc_wifi_dev_to_devidx_get(struct net_device *dev, unsigned int *wlan_dev_idx)
{
	int ret = 0;
	rtk_fc_wlan_devidx_t wlanDevIdx = RTK_FC_WLANX_NOT_FOUND;
	
	rtk_fc_mgr_global_spin_lock_bh();

	if((ret = _rtk_fc_dev2wlanDevIdx(dev, &wlanDevIdx)) == SUCCESS) {
		*wlan_dev_idx = wlanDevIdx;
	}
	
	rtk_fc_mgr_global_spin_unlock_bh();
	
	return ret ;
}
int _rtk_fc_wifi_client_mode_cb_register(rtk_fc_wifi_callback pfunc)
{
	fc_mgr_db._rtk_fc_wifi_client_mode_cb = pfunc;
	printk("rtk_fc_wifi_client_mode_cb_register: cb func %px\n", pfunc);
		
	return RTK_FC_HELPER_RET_OK;
}
#endif

#if defined(CONFIG_RTK_SOC_RTL8198D)
int rtk_fc_devName2wlanDevIdxPort(char *devName, rtk_fc_wlan_devidx_t *wlanDevIdx, unsigned char *cport, unsigned char *extport)
{
	rtk_fc_wlan_devidx_t i;

	if (unlikely(!wlanDevIdx || !cport || !extport)) 
		return FAIL;

	*wlanDevIdx= RTK_FC_WLANX_NOT_FOUND;
	*cport = 0;
	*extport = 0;

	for(i = 0; i < RTK_FC_WLANX_END_INTF; i++)
	{
		if(!fc_mgr_db.wlanDevMap[i].valid)
			continue;

		if(!strncmp(fc_mgr_db.wlanDevMap[i].wlanDev->name, devName, IFNAMSIZ))
		{
			*wlanDevIdx = i;
			*cport = fc_mgr_db.wlanDevMap[i].portmap.macPortIdx;
			*extport = fc_mgr_db.wlanDevMap[i].portmap.macExtPortIdx;
			//printk("%d  %d\n", *cport, *extport);
			break;
		}
	}

	if (*wlanDevIdx != RTK_FC_WLANX_NOT_FOUND) {
		return SUCCESS;
	}
	else {
		return FAIL;
	}
}

int rtk_fc_external_lut_process(bool add, char *wlan_devname, char *sta_mac)
{
	int ret = FAIL;
#if defined(CONFIG_RTK_SOC_RTL8198D) && (defined(CONFIG_FC_WIFI_TX_GMAC_TRUNKING_SUPPORT) || defined(CONFIG_FC_WIFI_TX_GMAC_AUTO_SEL_SUPPORT))
	rtk_fc_pmap_t port;
	rtk_fc_wlan_devidx_t devidx;
	uint16 vid = 0;
	int16 macL2Idx;
	unsigned char cport, extport;
	rtk_fc_mgr_l2_info_t l2info = {0};

	if (unlikely(!wlan_devname || !sta_mac)) {
		return FAIL;
	}

	rtk_fc_mgr_global_spin_lock_bh_irq_protect();

	if (rtk_fc_devName2wlanDevIdxPort(wlan_devname, &devidx, &cport, &extport) != SUCCESS) {
		if (add)
			printk("[%s:%d] fail to get devidx by %s\n", __FUNCTION__, __LINE__, wlan_devname);
		else
			FCMGR_DBG("[del] fail to get devidx by %s\n", wlan_devname);

		rtk_fc_mgr_global_spin_unlock_bh_irq_protect();
		return FAIL;
	}

	if (add) {
		port.macPortIdx = cport;
		port.macExtPortIdx = extport;

		if (!rtk_fc_ext_lut_learning(sta_mac, port, vid, devidx, FALSE, TRUE, &macL2Idx, 0)) {
			ret = SUCCESS;
		}
	}
	else {	// del
		if (!rtk_fc_l2Info_get(sta_mac, &l2info)) {
			if (l2info.spa != cport || l2info.extspa != extport) {
				FCMGR_DBG("[del] mac %pM cport %d extport %d mismatch lut entry %d spa %d extspa %d, do nothing!", 
					sta_mac, cport, extport, l2info.lutIdx, l2info.spa, l2info.extspa);
			}
			else {
				if (!_rtk_fc_lut_staticEntry_del(l2info.lutIdx, TRUE)) {
					ret = SUCCESS;
				}
				FCMGR_DBG("[del] mac %pM cport %d extport %d with l2 index %d", sta_mac, cport, extport, l2info.lutIdx);
			}
		}
	}

	rtk_fc_mgr_global_spin_unlock_bh_irq_protect();
#endif

	return ret;
}
EXPORT_SYMBOL(rtk_fc_external_lut_process);
#endif
