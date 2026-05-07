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
 *
 * $Revision: 93636 $
 * $Date: 2018-11-16 12:52:37 +0800 (Fri, 16 Nov 2018) $
 *
 * Purpose : Realtek Switch SDK Rtdrv Netfilter Extension Module in the SDK.
 *
 * Feature : Realtek Switch SDK Rtdrv Netfilter Extension Module
 *
 */


/*
 * Include Files
 */
//#include <asm/uaccess.h>
#include <net/sock.h>
#include <common/rt_error.h>

#include <rtdrv_ext_netfilter.h>


/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */


/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */


/* Function Name:
 *      do_rtdrv_ext_set_ctl
 * Description:
 *      This function is called whenever a process tries to do setsockopt
 * Input:
 *      *sk   - network layer representation of sockets
 *      cmd   - ioctl commands
 *      *user - data buffer handled between user and kernel space
 *      len   - data length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
 #if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
int32 do_rtdrv_ext_set_ctl(struct sock *sk, int cmd, sockptr_t arg, unsigned int len)
#else
int32 do_rtdrv_ext_set_ctl(struct sock *sk, int cmd, void *user, unsigned int len)
#endif
{
	int32                           ret = RT_ERR_FAILED;
	rtdrv_ext_union_t                buf;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
	void *user = arg.user;
#endif

	switch(cmd)
	{

        /* Rate */
        case RTDRV_RT_RATE_HOSTPOLICERCONTROL_SET:
            copy_from_user(&buf.rt_rate_cfg, user, sizeof(rtdrv_rt_rateCfg_ext_t));
            ret = rt_rate_hostPolicerControl_set(buf.rt_rate_cfg.index, buf.rt_rate_cfg.policerControl);
            break;

        case RTDRV_RT_RATE_HOSTPOLICERMIB_CLEAR:
            copy_from_user(&buf.rt_rate_cfg, user, sizeof(rtdrv_rt_rateCfg_ext_t));
            ret = rt_rate_hostPolicerMib_clear(buf.rt_rate_cfg.index);
            break;

		case RTDRV_RT_RATE_SW_RATE_LIMIT_SET:
            copy_from_user(&buf.rt_rate_cfg, user, sizeof(rtdrv_rt_rateCfg_ext_t));
            ret = rt_rate_sw_rate_limit_set(buf.rt_rate_cfg.type, buf.rt_rate_cfg.swRateLimit_conf);
            break;	

		case RTDRV_RT_IGMP_MULTICASTMODE_SET:
			copy_from_user(&buf.rt_igmp_cfg, user, sizeof(rtdrv_rt_igmpCfg_ext_t));
			ret = rt_igmp_multicastMode_set(buf.rt_igmp_cfg.mcMode);
			break;

		case RTDRV_RT_IGMP_UNKNOWN_MC_SET:
			copy_from_user(&buf.rt_igmp_cfg, user, sizeof(rtdrv_rt_igmpCfg_ext_t));
			ret = rt_igmp_unknownMcAction_set(buf.rt_igmp_cfg.mcUnknCfg);
			break;

			
		case RTDRV_RT_IGMP_MULTICASTGROUP_SET:
			copy_from_user(&buf.rt_igmp_cfg, user, sizeof(rtdrv_rt_igmpCfg_ext_t));
			ret = rt_igmp_multicastGroup_set(buf.rt_igmp_cfg.mcConfig);
			break;
			
		case RTDRV_RT_IGMP_MULTICASTGROUP_DEV_SET:
			copy_from_user(&buf.rt_igmp_cfg, user, sizeof(rtdrv_rt_igmpCfg_ext_t));
			ret = rt_igmp_multicastGroupDev_set(buf.rt_igmp_cfg.mcDevConfig);
			break;

		case RTDRV_RT_STAT_FLOW_CACHE_MIB_RESET:
			copy_from_user(&buf.rt_stat_cfg, user, sizeof(rtdrv_rt_statCfg_ext_t));
			ret = rt_stat_flowCache_mib_reset(buf.rt_stat_cfg.index);
			break;
			
		case RTDRV_RT_STAT_FLOWMIB_RESET:
			copy_from_user(&buf.rt_stat_cfg, user, sizeof(rtdrv_rt_statCfg_ext_t));
			ret = rt_stat_flowMib_reset(buf.rt_stat_cfg.index);
			break;

		case RTDRV_RT_STAT_NETIFMIB_RESET:
			copy_from_user(&buf.rt_stat_cfg, user, sizeof(rtdrv_rt_statCfg_ext_t));
			ret = rt_stat_netifMib_reset(&buf.rt_stat_cfg.dev_name[0]);
			break;
	
		case RTDRV_RT_STAT_GEMFILTER_SET:
			copy_from_user(&buf.rt_stat_cfg, user, sizeof(rtdrv_rt_statCfg_ext_t));
			ret = rt_stat_gemFilter_set(buf.rt_stat_cfg.index, buf.rt_stat_cfg.conf);
			break;

		case RTDRV_RT_STAT_GEMFILTERMIB_RESET:
			copy_from_user(&buf.rt_stat_cfg, user, sizeof(rtdrv_rt_statCfg_ext_t));
			ret = rt_stat_gemFilterMib_reset(buf.rt_stat_cfg.index);
			break;

		case RTDRV_RT_STAT_ACLMIB_SET:
			copy_from_user(&buf.rt_stat_cfg, user, sizeof(rtdrv_rt_statCfg_ext_t));
			ret = rt_stat_aclMib_set(buf.rt_stat_cfg.index, buf.rt_stat_cfg.aclMib);
			break;

		case RTDRV_RT_STAT_ACLMIB_RESET:
			copy_from_user(&buf.rt_stat_cfg, user, sizeof(rtdrv_rt_statCfg_ext_t));
			ret = rt_stat_aclMib_reset(buf.rt_stat_cfg.index);
			break;

		case RTDRV_RT_ACL_FILTERANDQOS_DEL:
			copy_from_user(&buf.rt_acl_cfg, user, sizeof(rtdrv_rt_aclCfg_ext_t));
			ret = rt_acl_filterAndQos_del(buf.rt_acl_cfg.acl_filter_idx);
			break; 

		case RTDRV_RT_ACL_RESERVED_STATUS_SET:
			copy_from_user(&buf.rt_acl_cfg, user, sizeof(rtdrv_rt_aclCfg_ext_t));
			ret = rt_acl_reserved_status_set(buf.rt_acl_cfg.reserved_type, buf.rt_acl_cfg.reserved_status);
			break; 

		case RTDRV_RT_MISC_WLAN_MAC_LEARNING_LIMIT_SET:
			copy_from_user(&buf.rt_misc_cfg, user, sizeof(rtdrv_rt_miscCfg_ext_t));
			ret = rt_misc_wlan_macAddr_learning_limit_set(buf.rt_misc_cfg.wlan_idx, buf.rt_misc_cfg.device_idx, buf.rt_misc_cfg.macAddr_learning_limit);
			break;	

		case RTDRV_RT_MISC_WAN_ACCESS_LIMIT_SET:
			copy_from_user(&buf.rt_misc_cfg, user, sizeof(rtdrv_rt_miscCfg_ext_t));
			ret = rt_misc_wan_access_limit_set(buf.rt_misc_cfg.wan_access_limit);
			break;

		case RTDRV_RT_MISC_BURST_PACKET_SET:
			copy_from_user(&buf.rt_misc_cfg, user, sizeof(rtdrv_rt_miscCfg_ext_t));
			ret = rt_misc_burst_packet_set(buf.rt_misc_cfg.burst_packet);
			break;

		case RTDRV_RT_MISC_WANPHYPORTMASK_SET:
			copy_from_user(&buf.rt_misc_cfg, user, sizeof(rtdrv_rt_miscCfg_ext_t));
			ret = rt_misc_wanPhyPortMask_set(buf.rt_misc_cfg.wan_portMask);
			break;

		case RTDRV_RT_MISC_WANFASTFWDDEV_ADD:
			copy_from_user(&buf.rt_misc_cfg, user, sizeof(rtdrv_rt_miscCfg_ext_t));
			ret = rt_misc_wanFastFwdDev_add(&buf.rt_misc_cfg.dev_name[0]);
			break;

		case RTDRV_RT_MISC_WANFASTFWDDEV_DEL:
			copy_from_user(&buf.rt_misc_cfg, user, sizeof(rtdrv_rt_miscCfg_ext_t));
			ret = rt_misc_wanFastFwdDev_del(&buf.rt_misc_cfg.dev_name[0]);
			break;

		case RTDRV_RT_MISC_VLANGROUPMACLIMIT_SET:
			copy_from_user(&buf.rt_vlanGroupMacLimit_set_cfg, user, sizeof(rtdrv_rt_vlanGroupMacLimit_set_t));
			ret = rt_misc_vlanGroupMacLimit_set(buf.rt_vlanGroupMacLimit_set_cfg.groupIdx,buf.rt_vlanGroupMacLimit_set_cfg.vlanId,buf.rt_vlanGroupMacLimit_set_cfg.groupLimit);
			break;

		case RTDRV_RT_MISC_VLANGROUPMACLIMIT_DEL:
			copy_from_user(&buf.rt_vlanGroupMacLimit_del_cfg, user, sizeof(rtdrv_rt_vlanGroupMacLimit_del_t));
			ret = rt_misc_vlanGroupMacLimit_del(buf.rt_vlanGroupMacLimit_del_cfg.groupIdx, buf.rt_vlanGroupMacLimit_del_cfg.vlanId);
			break;
			
		case RTDRV_RT_FLOW_HWNATMODE_SET:
			copy_from_user(&buf.rt_flow_cfg, user, sizeof(rtdrv_rt_flowCfg_ext_t));
			ret = rt_flow_hwnatMode_set(buf.rt_flow_cfg.hwnatMode);
			break;

		case RTDRV_RT_QOS_AQM_SET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_set(buf.rt_qos_cfg.aqm_cfg);
            break;
			
		case RTDRV_RT_QOS_AQM_COS_SET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_cos_set(buf.rt_qos_cfg.aqm_cos_cfg);
            break;

        case RTDRV_RT_QOS_AQM_THRESHOLD_SET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_threshold_set(buf.rt_qos_cfg.aqm_threshold_cfg);
            break;

        case RTDRV_RT_QOS_AQM_TIMER_SET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_timer_set(buf.rt_qos_cfg.aqm_timer_cfg);
            break;

		case RTDRV_RT_QOS_AQM_PIE_ENABLE_SET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_pie_enable_set(buf.rt_qos_cfg.aqmPie_enable);
            break;

        case RTDRV_RT_QOS_AQM_PIE_MATCH_SET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_pie_match_set(buf.rt_qos_cfg.aqmPie_match);
            break;

		case RTDRV_RT_QOS_AQM_PIE_MAPPING_SET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_pie_mapping_set(buf.rt_qos_cfg.aqmPie_mapping);
            break;

        case RTDRV_RT_QOS_AQM_PIE_THRESHOLD_SET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_pie_threshold_set(buf.rt_qos_cfg.aqmPie_threshold);
            break;

        case RTDRV_RT_QOS_AQM_PIE_DPSCALE_SET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_pie_dpScale_set(buf.rt_qos_cfg.aqmPie_dpScale);
            break;

        case RTDRV_RT_QOS_AQM_PIE_DPTHRESHOLD_SET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_pie_dpThreshold_set(buf.rt_qos_cfg.aqmPie_dpThreshold);
            break;

        case RTDRV_RT_QOS_AQM_PIE_DPBURST_SET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_pie_dpBurst_set(buf.rt_qos_cfg.aqmPie_dpBurst);
            break;

        case RTDRV_RT_QOS_AQM_PIE_QDELAY_SET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_pie_qdelay_set(buf.rt_qos_cfg.aqmPie_qdelay);
            break;

        case RTDRV_RT_QOS_AQM_PIE_RATE_SET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_pie_rate_set(buf.rt_qos_cfg.aqmPie_rate);
            break;

        case RTDRV_RT_QOS_AQM_PIE_CFG_SET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_pie_cfg_set(buf.rt_qos_cfg.aqmPie_cfg);
            break;
			
		default:
			printk("do_rtdrv_ext_set_ctl unknown cmd (%d)\n", cmd);
		
	}
	return ret;
}


/* Function Name:
 *      do_rtdrv_ext_get_ctl
 * Description:
 *      This function is called whenever a process tries to do getsockopt
 * Input:
 *      *sk   - network layer representation of sockets
 *      cmd   - ioctl commands
 * Output:
 *      *user - data buffer handled between user and kernel space
 *      len   - data length
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32 do_rtdrv_ext_get_ctl(struct sock *sk, int cmd, void *user, int *len)
{
	int32               ret = RT_ERR_FAILED;
	rtdrv_ext_union_t    buf;
	switch(cmd)
	{

        /* Rate */
        case RTDRV_RT_RATE_HOSTPOLICERCONTROL_GET:
            copy_from_user(&buf.rt_rate_cfg, user, sizeof(rtdrv_rt_rateCfg_ext_t));
            ret = rt_rate_hostPolicerControl_get(buf.rt_rate_cfg.index, &buf.rt_rate_cfg.policerControl);
            copy_to_user(user, &buf.rt_rate_cfg, sizeof(rtdrv_rt_rateCfg_ext_t));
            break;

        case RTDRV_RT_RATE_HOSTPOLICERMIB_GET:
            copy_from_user(&buf.rt_rate_cfg, user, sizeof(rtdrv_rt_rateCfg_ext_t));
            ret = rt_rate_hostPolicerMib_get(buf.rt_rate_cfg.index, &buf.rt_rate_cfg.policerMib);
            copy_to_user(user, &buf.rt_rate_cfg, sizeof(rtdrv_rt_rateCfg_ext_t));
            break;

		case RTDRV_RT_RATE_SW_RATE_LIMIT_GET:
            copy_from_user(&buf.rt_rate_cfg, user, sizeof(rtdrv_rt_rateCfg_ext_t));
            ret = rt_rate_sw_rate_limit_get(buf.rt_rate_cfg.type, &buf.rt_rate_cfg.swRateLimit_conf);
			copy_to_user(user, &buf.rt_rate_cfg, sizeof(rtdrv_rt_rateCfg_ext_t));
            break;	

		case RTDRV_RT_STAT_FLOW_CACHE_MIB_GET:
			copy_from_user(&buf.rt_stat_cfg, user, sizeof(rtdrv_rt_statCfg_ext_t));
			ret = rt_stat_flowCache_mib_get(buf.rt_stat_cfg.index, &buf.rt_stat_cfg.flowCacheMib);
			copy_to_user(user, &buf.rt_stat_cfg, sizeof(rtdrv_rt_statCfg_ext_t));
			break;
			
		case RTDRV_RT_STAT_FLOWMIB_GET:
			copy_from_user(&buf.rt_stat_cfg, user, sizeof(rtdrv_rt_statCfg_ext_t));
			ret = rt_stat_flowMib_get(buf.rt_stat_cfg.index, &buf.rt_stat_cfg.flowMib);
			copy_to_user(user, &buf.rt_stat_cfg, sizeof(rtdrv_rt_statCfg_ext_t));
			break;
		
		case RTDRV_RT_STAT_NETIFMIB_GET:
			copy_from_user(&buf.rt_stat_cfg, user, sizeof(rtdrv_rt_statCfg_ext_t));
			ret = rt_stat_netifMib_get(&buf.rt_stat_cfg.dev_name[0], &buf.rt_stat_cfg.netifMib);
			copy_to_user(user, &buf.rt_stat_cfg, sizeof(rtdrv_rt_statCfg_ext_t));
			break;
			
		case RTDRV_RT_STAT_GEMFILTER_GET:
            		copy_from_user(&buf.rt_stat_cfg, user, sizeof(rtdrv_rt_statCfg_ext_t));
            		ret = rt_stat_gemFilter_get(buf.rt_stat_cfg.index, &buf.rt_stat_cfg.conf);
	         	copy_to_user(user, &buf.rt_stat_cfg, sizeof(rtdrv_rt_statCfg_ext_t));
	            	break;

		case RTDRV_RT_STAT_GEMFILTERMIB_GET:
			copy_from_user(&buf.rt_stat_cfg, user, sizeof(rtdrv_rt_statCfg_ext_t));
			ret = rt_stat_gemFilterMib_get(buf.rt_stat_cfg.index, &buf.rt_stat_cfg.cntr);
			copy_to_user(user, &buf.rt_stat_cfg, sizeof(rtdrv_rt_statCfg_ext_t));
			break;
				
		case RTDRV_RT_STAT_ACLMIB_GET:
			copy_from_user(&buf.rt_stat_cfg, user, sizeof(rtdrv_rt_statCfg_ext_t));
			ret = rt_stat_aclMib_get(buf.rt_stat_cfg.index, &buf.rt_stat_cfg.aclMib);
			copy_to_user(user, &buf.rt_stat_cfg, sizeof(rtdrv_rt_statCfg_ext_t));
			break;

		case RTDRV_RT_ACL_FILTERANDQOS_ADD:
			copy_from_user(&buf.rt_acl_cfg, user, sizeof(rtdrv_rt_aclCfg_ext_t));
			ret = rt_acl_filterAndQos_add(buf.rt_acl_cfg.acl_filter, &buf.rt_acl_cfg.acl_filter_idx);
			copy_to_user(user, &buf.rt_acl_cfg, sizeof(rtdrv_rt_aclCfg_ext_t));
			break;

		case RTDRV_RT_ACL_FILTERANDQOS_GET:
			copy_from_user(&buf.rt_acl_cfg, user, sizeof(rtdrv_rt_aclCfg_ext_t));
			ret = rt_acl_filterAndQos_get(buf.rt_acl_cfg.acl_filter_idx, &buf.rt_acl_cfg.acl_filter);
			copy_to_user(user, &buf.rt_acl_cfg, sizeof(rtdrv_rt_aclCfg_ext_t));
			break;

		case RTDRV_RT_ACL_RESERVED_STATUS_GET:
			copy_from_user(&buf.rt_acl_cfg, user, sizeof(rtdrv_rt_aclCfg_ext_t));
			ret = rt_acl_reserved_status_get(buf.rt_acl_cfg.reserved_type, &buf.rt_acl_cfg.reserved_status);
			copy_to_user(user, &buf.rt_acl_cfg, sizeof(rtdrv_rt_aclCfg_ext_t));
			break;

		case RTDRV_RT_MISC_WLAN_MAC_LEARNING_LIMIT_GET:
			copy_from_user(&buf.rt_misc_cfg, user, sizeof(rtdrv_rt_miscCfg_ext_t));
			ret = rt_misc_wlan_macAddr_learning_limit_get(buf.rt_misc_cfg.wlan_idx, buf.rt_misc_cfg.device_idx, &buf.rt_misc_cfg.macAddr_learning_limit);
			copy_to_user(user, &buf.rt_misc_cfg, sizeof(rtdrv_rt_miscCfg_ext_t));
			break;	

		case RTDRV_RT_MISC_WAN_ACCESS_LIMIT_GET:
			copy_from_user(&buf.rt_misc_cfg, user, sizeof(rtdrv_rt_miscCfg_ext_t));
			ret = rt_misc_wan_access_limit_get(&buf.rt_misc_cfg.wan_access_limit);
			copy_to_user(user, &buf.rt_misc_cfg, sizeof(rtdrv_rt_miscCfg_ext_t));
			break;

		case RTDRV_RT_MISC_WANPHYPORTMASK_GET:
			copy_from_user(&buf.rt_misc_cfg, user, sizeof(rtdrv_rt_miscCfg_ext_t));
			ret = rt_misc_wanPhyPortMask_get(&buf.rt_misc_cfg.wan_portMask);
			copy_to_user(user, &buf.rt_misc_cfg, sizeof(rtdrv_rt_miscCfg_ext_t));
			break;

		case RTDRV_RT_MISC_VLANGROUPMACLIMIT_ADD:
			copy_from_user(&buf.rt_vlanGroupMacLimit_add_cfg, user, sizeof(rtdrv_rt_vlanGroupMacLimit_add_t));
			ret = rt_misc_vlanGroupMacLimit_add(buf.rt_vlanGroupMacLimit_add_cfg.port, buf.rt_vlanGroupMacLimit_add_cfg.groupLimit, &buf.rt_vlanGroupMacLimit_add_cfg.pGroupIdx);
			copy_to_user(user, &buf.rt_vlanGroupMacLimit_add_cfg, sizeof(rtdrv_rt_vlanGroupMacLimit_add_t));
			break;

		case RTDRV_RT_MISC_VLANGROUPMACLIMIT_GET:
			copy_from_user(&buf.rt_vlanGroupMacLimit_get_cfg, user, sizeof(rtdrv_rt_vlanGroupMacLimit_get_t));
			ret = rt_misc_vlanGroupMacLimit_get(buf.rt_vlanGroupMacLimit_get_cfg.groupIdx, &buf.rt_vlanGroupMacLimit_get_cfg.pGroupInfo);
            		copy_to_user(user, &buf.rt_vlanGroupMacLimit_get_cfg, sizeof(rtdrv_rt_vlanGroupMacLimit_get_t));
			break;

		case RTDRV_RT_MISC_VLANGROUPMACLIMIT_FIND:
			copy_from_user(&buf.rt_vlanGroupMacLimit_find_cfg, user, sizeof(rtdrv_rt_vlanGroupMacLimit_find_t));
			ret = rt_misc_vlanGroupMacLimit_find(buf.rt_vlanGroupMacLimit_find_cfg.port, buf.rt_vlanGroupMacLimit_find_cfg.vlanId, &buf.rt_vlanGroupMacLimit_find_cfg.pGroupIdx);
			copy_to_user(user, &buf.rt_vlanGroupMacLimit_find_cfg, sizeof(rtdrv_rt_vlanGroupMacLimit_find_t));
			break;

		case RTDRV_RT_FLOW_OPERATION_ADD:
			copy_from_user(&buf.rt_flow_cfg, user, sizeof(rtdrv_rt_flowCfg_ext_t));
			ret = rt_flow_operation_add(buf.rt_flow_cfg.flowOp, &buf.rt_flow_cfg.opData);
			copy_to_user(user, &buf.rt_flow_cfg, sizeof(rtdrv_rt_flowCfg_ext_t)); // rt_flow_operation_set include get operation (e.g. mib get)
			break;

		case RTDRV_RT_FLOW_HWNATMODE_GET:
			copy_from_user(&buf.rt_flow_cfg, user, sizeof(rtdrv_rt_flowCfg_ext_t));
			ret = rt_flow_hwnatMode_get(&buf.rt_flow_cfg.hwnatMode);
			copy_to_user(user, &buf.rt_flow_cfg, sizeof(rtdrv_rt_flowCfg_ext_t));
			break;
			
		case RTDRV_RT_IGMP_MULTICASTGROUP_DEV_GET:
			copy_from_user(&buf.rt_igmp_cfg, user, sizeof(rtdrv_rt_igmpCfg_ext_t));
			ret = rt_igmp_multicastGroupDev_get(&buf.rt_igmp_cfg.mcDevConfig);
			copy_to_user(user, &buf.rt_igmp_cfg, sizeof(rtdrv_rt_igmpCfg_ext_t));
			break;
			
		case RTDRV_RT_QOS_AQM_GET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_get(&buf.rt_qos_cfg.aqm_cfg);
            copy_to_user(user, &buf.rt_qos_cfg, sizeof(rtdrv_rt_qosCfg_ext_t));
            break;
			
		case RTDRV_RT_QOS_AQM_COS_GET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_cos_get(&buf.rt_qos_cfg.aqm_cos_cfg);
            copy_to_user(user, &buf.rt_qos_cfg, sizeof(rtdrv_rt_qosCfg_ext_t));
            break;

        case RTDRV_RT_QOS_AQM_THRESHOLD_GET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_threshold_get(&buf.rt_qos_cfg.aqm_threshold_cfg);
            copy_to_user(user, &buf.rt_qos_cfg, sizeof(rtdrv_rt_qosCfg_ext_t));
            break;

        case RTDRV_RT_QOS_AQM_TIMER_GET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_timer_get(&buf.rt_qos_cfg.aqm_timer_cfg);
            copy_to_user(user, &buf.rt_qos_cfg, sizeof(rtdrv_rt_qosCfg_ext_t));
            break;

		case RTDRV_RT_QOS_AQM_FLOW_STATISTIC_GET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_flow_statistic_get(&buf.rt_qos_cfg.aqm_flow_statistic);
            copy_to_user(user, &buf.rt_qos_cfg, sizeof(rtdrv_rt_qosCfg_ext_t));
            break;	

		case RTDRV_RT_QOS_AQM_PIE_ENABLE_GET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_pie_enable_get(&buf.rt_qos_cfg.aqmPie_enable);
            copy_to_user(user, &buf.rt_qos_cfg, sizeof(rtdrv_rt_qosCfg_ext_t));
            break;

        case RTDRV_RT_QOS_AQM_PIE_MATCH_GET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_pie_match_get(&buf.rt_qos_cfg.aqmPie_match);
            copy_to_user(user, &buf.rt_qos_cfg, sizeof(rtdrv_rt_qosCfg_ext_t));
            break;

		case RTDRV_RT_QOS_AQM_PIE_MAPPING_GET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_pie_mapping_get(&buf.rt_qos_cfg.aqmPie_mapping);
            copy_to_user(user, &buf.rt_qos_cfg, sizeof(rtdrv_rt_qosCfg_ext_t));
            break;

        case RTDRV_RT_QOS_AQM_PIE_THRESHOLD_GET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_pie_threshold_get(&buf.rt_qos_cfg.aqmPie_threshold);
            copy_to_user(user, &buf.rt_qos_cfg, sizeof(rtdrv_rt_qosCfg_ext_t));
            break;

        case RTDRV_RT_QOS_AQM_PIE_DPSCALE_GET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_pie_dpScale_get(&buf.rt_qos_cfg.aqmPie_dpScale);
            copy_to_user(user, &buf.rt_qos_cfg, sizeof(rtdrv_rt_qosCfg_ext_t));
            break;

        case RTDRV_RT_QOS_AQM_PIE_DPTHRESHOLD_GET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_pie_dpThreshold_get(&buf.rt_qos_cfg.aqmPie_dpThreshold);
            copy_to_user(user, &buf.rt_qos_cfg, sizeof(rtdrv_rt_qosCfg_ext_t));
            break;

        case RTDRV_RT_QOS_AQM_PIE_DPBURST_GET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_pie_dpBurst_get(&buf.rt_qos_cfg.aqmPie_dpBurst);
            copy_to_user(user, &buf.rt_qos_cfg, sizeof(rtdrv_rt_qosCfg_ext_t));
            break;

        case RTDRV_RT_QOS_AQM_PIE_QDELAY_GET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_pie_qdelay_get(&buf.rt_qos_cfg.aqmPie_qdelay);
            copy_to_user(user, &buf.rt_qos_cfg, sizeof(rtdrv_rt_qosCfg_ext_t));
            break;

        case RTDRV_RT_QOS_AQM_PIE_RATE_GET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_pie_rate_get(&buf.rt_qos_cfg.aqmPie_rate);
            copy_to_user(user, &buf.rt_qos_cfg, sizeof(rtdrv_rt_qosCfg_ext_t));
            break;

        case RTDRV_RT_QOS_AQM_PIE_CFG_GET:
            copy_from_user(&buf.rt_qos_cfg, user, sizeof(rtdrv_rt_qosCfg_ext_t));
            ret = rt_qos_aqm_pie_cfg_get(&buf.rt_qos_cfg.aqmPie_cfg);
            copy_to_user(user, &buf.rt_qos_cfg, sizeof(rtdrv_rt_qosCfg_ext_t));
            break;
			
		case RTDRV_RT_PE_HTTP_TEST_GET:
            copy_from_user(&buf.rt_pe_cfg, user, sizeof(rtdrv_rt_peCfg_ext_t));
            ret = rt_pe_http_test(buf.rt_pe_cfg.http_test_request, &buf.rt_pe_cfg.http_test_result);
            copy_to_user(user, &buf.rt_pe_cfg, sizeof(rtdrv_rt_peCfg_ext_t));
            break;	
#if 0			
		case RTDRV_RT_CRYPTO_KEY_SET:
			copy_from_user(&buf.rt_crypto_cfg, user, sizeof(rtdrv_rt_cryptoCfg_ext_t));
			ret = rt_crypto_key_set(&buf.rt_crypto_cfg.rt_crypto_key_set_cfg, &buf.rt_crypto_cfg.rt_crypto_key_set_cfg.key_index);
			copy_to_user(user, &buf.rt_crypto_cfg, sizeof(rtdrv_rt_cryptoCfg_ext_t));
        	break;	
			
		case RTDRV_RT_CRYPTO_KEY_GET:
			copy_from_user(&buf.rt_crypto_cfg, user, sizeof(rtdrv_rt_cryptoCfg_ext_t));
			ret = rt_crypto_key_get_index(&buf.rt_crypto_cfg.rt_crypto_key_set_cfg.key_index);
			copy_to_user(user, &buf.rt_crypto_cfg, sizeof(rtdrv_rt_cryptoCfg_ext_t));
        	break;	
			
		case RTDRV_RT_CRYPTO_KEY_SET_BY_INDEX:
			copy_from_user(&buf.rt_crypto_cfg, user, sizeof(rtdrv_rt_cryptoCfg_ext_t));
			ret = rt_crypto_key_add_by_index(&buf.rt_crypto_cfg.rt_crypto_key_set_cfg);
			copy_to_user(user, &buf.rt_crypto_cfg, sizeof(rtdrv_rt_cryptoCfg_ext_t));
        	break;	
			
		case RTDRV_RT_CRYPTO_KEY_DEL_BY_INDEX:
			copy_from_user(&buf.rt_crypto_cfg, user, sizeof(rtdrv_rt_cryptoCfg_ext_t));
			ret = rt_crypto_key_del_by_index(&buf.rt_crypto_cfg.rt_crypto_key_set_cfg.key_index);
			copy_to_user(user, &buf.rt_crypto_cfg, sizeof(rtdrv_rt_cryptoCfg_ext_t));
        	break;	
			
		case RTDRV_RT_CRYPTO_KEYHASH_GET:
			copy_from_user(&buf.rt_crypto_cfg, user, sizeof(rtdrv_rt_cryptoCfg_ext_t));
			ret = rt_crypto_keyhash_get_index(&buf.rt_crypto_cfg.rt_crypto_key_set_cfg.key_index);
			copy_to_user(user, &buf.rt_crypto_cfg, sizeof(rtdrv_rt_cryptoCfg_ext_t));
        	break;	
			
		case RTDRV_RT_CRYPTO_KEYHASH_SET_BY_INDEX:
			copy_from_user(&buf.rt_crypto_cfg, user, sizeof(rtdrv_rt_cryptoCfg_ext_t));
			ret = rt_crypto_keyhash_add_by_index(&buf.rt_crypto_cfg.rt_crypto_key_set_cfg);
			copy_to_user(user, &buf.rt_crypto_cfg, sizeof(rtdrv_rt_cryptoCfg_ext_t));
        	break;	
			
		case RTDRV_RT_CRYPTO_KEYHASH_DEL_BY_INDEX:
			copy_from_user(&buf.rt_crypto_cfg, user, sizeof(rtdrv_rt_cryptoCfg_ext_t));
			ret = rt_crypto_keyhash_del_by_index(&buf.rt_crypto_cfg.rt_crypto_key_set_cfg.key_index);
			copy_to_user(user, &buf.rt_crypto_cfg, sizeof(rtdrv_rt_cryptoCfg_ext_t));
        	break;
#endif			
		default:
			printk("do_rtdrv_ext_get_ctl unknown cmd (%d)\n", cmd);
	}
	return ret;
}
