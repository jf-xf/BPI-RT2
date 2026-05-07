#include <rtk_rg_wlan_internal.h>

#if defined(CONFIG_RG_WLAN_HWNAT_ACCELERATION) && !defined(CONFIG_ARCH_LUNA_SLAVE)
// wlan0
struct net_device *wlan_root_netdev=NULL;
struct net_device *wlan_vap_netdev[RG_WLAN_VAP_NUM]={0};
#if defined(CONFIG_RTL_WDS_SUPPORT)
struct net_device *wlan_wds_netdev[8]={0};
#endif
#ifdef CONFIG_RTL_MESH_SUPPORT
struct net_device *wlan_mesh_netdev=NULL;
#endif
#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
struct net_device *wlan_vxd_netdev=NULL;
#endif
// wlan1
struct net_device *wlan1_root_netdev=NULL;
struct net_device *wlan1_vap_netdev[RG_WLAN_VAP_NUM]={0};
#if defined(CONFIG_RTL_WDS_SUPPORT)
struct net_device *wlan1_wds_netdev[8]={0};
#endif
#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
struct net_device *wlan1_vxd_netdev=NULL;
#endif

/* Function Name:
 *      rtk_rg_wlan_netDevice_set
 * Description:
 *      Set address of net device of wlan
 * Input:
 *      wlanDevIdx	- wlan device index(refer to rtk_rg_mbssidDev_t)
 *      pDev		- address of net device of wlan
 * Output:
 *      None
 * Return:
 *      RT_ERR_RG_OK
 *      RT_ERR_RG_INDEX_OUT_OF_RANGE
 *      RT_ERR_RG_NULL_POINTER
 */
rtk_rg_err_code_t rtk_rg_wlan_netDevice_set(rtk_rg_mbssidDev_t wlanDevIdx, struct net_device *pDev)
{	
	if(!(RG_RET_MBSSID_MASTER_ROOT_INTF<=wlanDevIdx && wlanDevIdx<=RG_RET_MBSSID_SLAVE_LAST_INTF))
		return(RT_ERR_RG_INDEX_OUT_OF_RANGE);
	if(pDev==NULL)
		return(RT_ERR_RG_NULL_POINTER);

	switch(wlanDevIdx)
	{
		// wlan0
		case RG_RET_MBSSID_MASTER_ROOT_INTF:
			wlan_root_netdev = pDev;
			printk("Set net_device of wlan0 root to %p", wlan_root_netdev);
			break;
		case RG_RET_MBSSID_MASTER_VAP0_INTF:
		case RG_RET_MBSSID_MASTER_VAP1_INTF:
		case RG_RET_MBSSID_MASTER_VAP2_INTF:
		case RG_RET_MBSSID_MASTER_VAP3_INTF:
#if defined(CONFIG_WLAN_MBSSID_NUM) && (CONFIG_WLAN_MBSSID_NUM==7)
		case RG_RET_MBSSID_MASTER_VAP4_INTF:
		case RG_RET_MBSSID_MASTER_VAP5_INTF:
		case RG_RET_MBSSID_MASTER_VAP6_INTF:
#endif
			wlan_vap_netdev[wlanDevIdx-RG_RET_MBSSID_MASTER_VAP0_INTF] = pDev;
			printk("Set net_device of wlan0 vap%d to %p", wlanDevIdx-RG_RET_MBSSID_MASTER_VAP0_INTF, wlan_vap_netdev[wlanDevIdx-RG_RET_MBSSID_MASTER_VAP0_INTF]);
			break;
#if defined(CONFIG_RTL_WDS_SUPPORT)
		case RG_RET_MBSSID_MASTER_WDS0_INTF:
		case RG_RET_MBSSID_MASTER_WDS1_INTF:
		case RG_RET_MBSSID_MASTER_WDS2_INTF:
		case RG_RET_MBSSID_MASTER_WDS3_INTF:
		case RG_RET_MBSSID_MASTER_WDS4_INTF:
		case RG_RET_MBSSID_MASTER_WDS5_INTF:
		case RG_RET_MBSSID_MASTER_WDS6_INTF:
#ifdef CONFIG_RTL_MESH_SUPPORT
			wlan_wds_netdev[wlanDevIdx-RG_RET_MBSSID_MASTER_WDS0_INTF] = pDev;
			printk("Set net_device of wlan0 wds%d to %p", wlanDevIdx-RG_RET_MBSSID_MASTER_WDS0_INTF, wlan_wds_netdev[wlanDevIdx-RG_RET_MBSSID_MASTER_WDS0_INTF]);
			break;
		case RG_RET_MBSSID_MASTER_MESH_INTF:
			wlan_mesh_netdev = pDev;
			printk("Set net_device of wlan0 mesh to %p", wlan_mesh_netdev);
			break;
#else	// not CONFIG_RTL_MESH_SUPPORT
		case RG_RET_MBSSID_MASTER_WDS7_INTF:
			wlan_wds_netdev[wlanDevIdx-RG_RET_MBSSID_MASTER_WDS0_INTF] = pDev;
			printk("Set net_device of wlan0 wds%d to %p", wlanDevIdx-RG_RET_MBSSID_MASTER_WDS0_INTF, wlan_wds_netdev[wlanDevIdx-RG_RET_MBSSID_MASTER_WDS0_INTF]);
			break;
#endif	// end CONFIG_RTL_MESH_SUPPORT
#else	// not CONFIG_RTL_WDS_SUPPORT
#ifdef CONFIG_RTL_MESH_SUPPORT
		case RG_RET_MBSSID_MASTER_MESH_INTF:
			wlan_mesh_netdev = pDev;
			printk("Set net_device of wlan0 mesh to %p", wlan_mesh_netdev);
			break;
#endif	// end CONFIG_RTL_MESH_SUPPORT
#endif	// end CONFIG_RTL_WDS_SUPPORT
#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
		case RG_RET_MBSSID_MASTER_CLIENT_INTF:
			wlan_vxd_netdev = pDev;
			printk("Set net_device of wlan0 vxd to %p", wlan_vxd_netdev);
			break;
#endif
		// wlan1
		case RG_RET_MBSSID_SLAVE_ROOT_INTF:
			wlan1_root_netdev = pDev;
			printk("Set net_device of wlan1 root to %p", wlan1_root_netdev);
			break;
		case RG_RET_MBSSID_SLAVE_VAP0_INTF:
		case RG_RET_MBSSID_SLAVE_VAP1_INTF:
		case RG_RET_MBSSID_SLAVE_VAP2_INTF:
		case RG_RET_MBSSID_SLAVE_VAP3_INTF:
#if defined(CONFIG_WLAN_MBSSID_NUM) && (CONFIG_WLAN_MBSSID_NUM==7)
		case RG_RET_MBSSID_SLAVE_VAP4_INTF:
		case RG_RET_MBSSID_SLAVE_VAP5_INTF:
		case RG_RET_MBSSID_SLAVE_VAP6_INTF:
#endif
			wlan1_vap_netdev[wlanDevIdx-RG_RET_MBSSID_SLAVE_VAP0_INTF] = pDev;
			printk("Set net_device of wlan1 vap%d to %p", wlanDevIdx-RG_RET_MBSSID_SLAVE_VAP0_INTF, wlan1_vap_netdev[wlanDevIdx-RG_RET_MBSSID_SLAVE_VAP0_INTF]);
			break;
#if defined(CONFIG_RTL_WDS_SUPPORT)
		case RG_RET_MBSSID_SLAVE_WDS0_INTF:
		case RG_RET_MBSSID_SLAVE_WDS1_INTF:
		case RG_RET_MBSSID_SLAVE_WDS2_INTF:
		case RG_RET_MBSSID_SLAVE_WDS3_INTF:
		case RG_RET_MBSSID_SLAVE_WDS4_INTF:
		case RG_RET_MBSSID_SLAVE_WDS5_INTF:
		case RG_RET_MBSSID_SLAVE_WDS6_INTF:
		case RG_RET_MBSSID_SLAVE_WDS7_INTF:
			wlan1_wds_netdev[wlanDevIdx-RG_RET_MBSSID_SLAVE_WDS0_INTF] = pDev;
			printk("Set net_device of wlan1 wds%d to %p", wlanDevIdx-RG_RET_MBSSID_SLAVE_WDS0_INTF, wlan1_wds_netdev[wlanDevIdx-RG_RET_MBSSID_SLAVE_WDS0_INTF]);
			break;
#endif
#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
		case RG_RET_MBSSID_SLAVE_CLIENT_INTF:
			wlan1_vxd_netdev = pDev;
			printk("Set net_device of wlan1 vxd to %p", wlan1_vxd_netdev);
			break;
#endif
		default:
			return(RT_ERR_RG_INDEX_OUT_OF_RANGE);
	}

	return RT_ERR_RG_OK;
}

void rtk_rg_wlan_netDevice_show(void)
{
	int i;

	printk("wlan_root_netdev\t\t= %p \n", wlan_root_netdev);
	for(i=0; i<RG_WLAN_VAP_NUM; i++)
		printk("wlan_vap_netdev[%d]\t= %p \n", i, wlan_vap_netdev[i]);
#if defined(CONFIG_RTL_WDS_SUPPORT)
#ifdef CONFIG_RTL_MESH_SUPPORT
	for(i=0; i<7; i++)
		printk("wlan_wds_netdev[%d]\t= %p \n", i, wlan_wds_netdev[i]);
	printk("wlan_mesh_netdev\t\t= %p \n", wlan_mesh_netdev);
#else	// not CONFIG_RTL_MESH_SUPPORT
	for(i=0; i<8; i++)
		printk("wlan_wds_netdev[%d]\t= %p \n", i, wlan_wds_netdev[i]);
#endif	// end CONFIG_RTL_MESH_SUPPORT
#else	// not CONFIG_RTL_WDS_SUPPORT
#ifdef CONFIG_RTL_MESH_SUPPORT
	printk("wlan_mesh_netdev\t\t= %p \n", wlan_mesh_netdev);
#endif	// end CONFIG_RTL_MESH_SUPPORT
#endif	// end CONFIG_RTL_WDS_SUPPORT
#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
	printk("wlan_vxd_netdev\t\t= %p \n", wlan_vxd_netdev);
#endif

	printk("wlan1_root_netdev\t= %p \n", wlan1_root_netdev);
	for(i=0; i<RG_WLAN_VAP_NUM; i++)
		printk("wlan1_vap_netdev[%d]\t= %p \n", i, wlan1_vap_netdev[i]);
#if defined(CONFIG_RTL_WDS_SUPPORT)
	for(i=0; i<8; i++)
		printk("wlan1_wds_netdev[%d]\t= %p \n", i, wlan1_wds_netdev[i]);
#endif
#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
	printk("wlan1_vxd_netdev\t\t= %p \n", wlan1_vxd_netdev);
#endif

	return;
}

EXPORT_SYMBOL(rtk_rg_wlan_netDevice_set);
EXPORT_SYMBOL(rtk_rg_wlan_netDevice_show);
#endif	// end defined(CONFIG_RG_WLAN_HWNAT_ACCELERATION) && !defined(CONFIG_ARCH_LUNA_SLAVE)
