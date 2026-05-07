
#include <rtk_rg_struct.h>

#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <rt_edp_struct.h>
#include <rt_edp_debug.h>

int _rt_edp_pasring_proc_string_to_integer(const char *buff,unsigned long len)
{
	int ret;
	unsigned char tmpBuf[16] = {0};
	int count = (len > 15) ? 15 : len;
	if(len > sizeof(tmpBuf))
		WARNING("FIXME... (string len %lu > buffer size %d).", __FUNCTION__, __LINE__, len, sizeof(tmpBuf));

	if (buff && !copy_from_user(tmpBuf, buff, count))
	{
		tmpBuf[count] = '\0';
	}
	ret=simple_strtol(tmpBuf, NULL, 0);

	return ret;
}

int rt_edp_debug_level_show(struct seq_file *s, void *v)
{
	int len=0;
	char *onString="\033[1;33mon\033[0m";

	PROC_PRINTF("RomeDriver Debug level=0x%x [0x1:WARN=%s][0x2:IGMP=%s][0x4:DEBUG=%s][0x8:CMD=%s][0x10:ACL=%s][0x40:ACL_CTRL=%s][0x100:PREROUTING=%s][0x200:FORWARD=%s][0x400:POSTROUTING=%s][0x800:LOCAL_IN=%s][0x1000:LOCAL_OUT=%s][0x10000:PREROUTING_DUMP=%s][0x20000:FORWARD_DUMP=%s][0x40000:POSTROUTING_DUMP=%s][0x80000:LOCAL_IN_DUMP=%s][0x100000:LOCAL_OUT_DUMP=%s]\n"
		,rt_edp_kernel.debug_level,
		(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_WARN)?onString:"off",
		(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_IGMP)?onString:"off",
		(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_DEBUG)?onString:"off",
		(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_CMD)?onString:"off",
		(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_ACL)?onString:"off",
		(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_ACL_CONTROL_PATH)?onString:"off",
		(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_TRACE_PREROUTING)?onString:"off",
		(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_TRACE_FORWARD)?onString:"off",
		(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_TRACE_POSTROUTING)?onString:"off",
		(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_TRACE_LOCAL_IN)?onString:"off",
		(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_TRACE_LOCAL_OUT)?onString:"off",
		(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_TRACE_PREROUTING_DUMP)?onString:"off",
		(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_TRACE_FORWARD_DUMP)?onString:"off",
		(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_TRACE_POSTROUTING_DUMP)?onString:"off",
		(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_TRACE_LOCAL_IN_DUMP)?onString:"off",
		(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_TRACE_LOCAL_OUT_DUMP)?onString:"off"
		);
	return len;
}

int rt_edp_debug_level_change(struct file *file, const char *buffer, unsigned long count, void *data)
{
	unsigned char tmpBuf[16] = {0};
	int len = (count > 15) ? 15 : count;

	if (buffer && !copy_from_user(tmpBuf, buffer, len))
	{
		rt_edp_kernel.debug_level=simple_strtoul(tmpBuf, NULL, 16);
		rt_edp_debug_level_show(NULL,NULL);

		return count;
	}
	return -EFAULT;
}

int32 _rt_edp_aclDumpParameterEnable_get(struct seq_file *s, void *v)
{
	int len=0;
	PROC_PRINTF("acl_rg_add_parameter_dump: %s\n",rt_edp_db.systemGlobal.acl_rg_add_parameter_dump?"Enabled":"Disabled");
	PROC_PRINTF("============== rg_acl debug ============== \n");
	_dump_rt_edp_acl(s);
	PROC_PRINTF("============== rg_acl diag shell dump debug ============== \n");
	_dump_rt_edp_acl_and_cf_diagshell(s);
	return len;
}

int _rt_edp_aclDumpParameterEnable_set(struct file *filp, const char *buff,unsigned long count, void *data)
{
	rt_edp_db.systemGlobal.acl_rg_add_parameter_dump=_rt_edp_pasring_proc_string_to_integer(buff,count);
	_rt_edp_aclDumpParameterEnable_get(NULL,NULL);
	return count;
}



int _rt_edp_get_mcast_query_sec(struct seq_file *s, void *v)
{
	int len=0;
	if (rt_edp_db.systemGlobal.mcast_query_sec==0)
	{
		PROC_PRINTF("disable (0:disable; default: %d secs)\n",RTK_RG_DEFAULT_MCAST_QUERY_INTERVAL);
	}else{
		PROC_PRINTF("mcast_query_sec%d (0:disable;)\n",rt_edp_db.systemGlobal.mcast_query_sec,RTK_RG_DEFAULT_MCAST_QUERY_INTERVAL);
	}
	return len;	
}

int _rt_edp_set_mcast_query_sec( struct file *filp, const char *buff,unsigned long len, void *data )
{
	rt_edp_db.systemGlobal.mcast_query_sec =_rt_edp_pasring_proc_string_to_integer(buff,len);
	_rt_edp_get_mcast_query_sec(NULL,NULL);
	return len;	
}



rtk_rg_proc_t rtEdpProc[]=
{

//===========proc for test start===========



#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES)
	{
		.name="test_flow" ,
		//.get=NULL,
		//.set=_rt_edp_test_flow_set,
		.group_bitmask = RTK_RG_PROC_GROUP_TEST_BITMASK,
	},
#endif


	{
		.name="auto_test_result_check" ,
		//.get=_rt_edp_auto_test_result_check_get,
		//.set=_rt_edp_auto_test_result_check_set,
		.group_bitmask = RTK_RG_PROC_GROUP_TEST_BITMASK,
	},
	{
		.name="auto_test_fail_arp_interval_sec" ,
		//.get=_rt_edp_auto_test_fail_arp_interval_sec_get,
		//.set=_rt_edp_auto_test_fail_arp_interval_sec_set,
		.group_bitmask = RTK_RG_PROC_GROUP_TEST_BITMASK,
	},
	{
		.name="auto_test_checked_qid" ,
		//.get=_rt_edp_auto_test_checkintPri_get,
		//.set=_rt_edp_auto_test_checkintPri_set,
		.group_bitmask = RTK_RG_PROC_GROUP_TEST_BITMASK,
	},
	{
		.name="auto_test_checked_egrPort" ,
		//.get=_rt_edp_auto_test_checkEgrPort_get,
		//.set=_rt_edp_auto_test_checkEgrPort_set,
		.group_bitmask = RTK_RG_PROC_GROUP_TEST_BITMASK,
	},

	{
		.name="auto_test_checked_fwdRsn" ,
		//.get=_rt_edp_auto_test_checkFwdRsn_get,
		//.set=_rt_edp_auto_test_checkFwdRsn_set,
		.group_bitmask = RTK_RG_PROC_GROUP_TEST_BITMASK,
	},

//===========proc for test end===========


#if defined(CONFIG_RG_G3_SERIES)
	{
		.name="rg_mirror_ingress_pkt_to_port" ,
		//.get= rt_edp_mirror_ingress_pkt_to_port_get,
		//.set= rt_edp_mirror_ingress_pkt_to_port_set,
		.group_bitmask = RTK_RG_PROC_GROUP_TEST_BITMASK ,
	},
#endif


#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9602C_SERIES) || defined(CONFIG_RG_G3_SERIES)
	{
		.name="fiber_mode_qos_enabled" ,
		//.get=  _rt_edp_fiber_mode_qos_enabled_get,
		//.set=  _rt_edp_fiber_mode_qos_enabled_set,
		.group_bitmask = RTK_RG_PROC_GROUP_QOS_BITMASK | RTK_RG_PROC_GROUP_PON_BITMASK,
	},
#endif
#if defined(CONFIG_RG_RTL9607C_SERIES)
	{
		.name="downstream_rate_limit_by_shaper" ,
		//.get=  _rt_edp_downstream_rate_limit_by_shaper_get,
		//.set=  _rt_edp_downstream_rate_limit_by_shaper_set,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK | RTK_RG_PROC_GROUP_PON_BITMASK,
	},
#endif
	{
		.name="arp_request_interval_sec" ,
		//.get= _rt_edp_arp_request_interval_sec_get ,
		//.set= _rt_edp_arp_request_interval_sec_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_ARP_BITMASK | RTK_RG_PROC_GROUP_AGING_BITMASK,
	},
	{
		.name="icmp_timeout" ,
		//.get= _rt_edp_icmp_tracking_timeout_sec_get ,
		//.set= _rt_edp_icmp_tracking_timeout_sec_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_AGING_BITMASK,
	},		
	{
		.name="wan_access_limit_request_interval" ,
		//.get= _rt_edp_get_wanAccessLimit_interval ,
		//.set= _rt_edp_set_wanAccessLimit_interval ,
		.group_bitmask = RTK_RG_PROC_GROUP_CTC_BITMASK,		
	},	
	{
		.name="napt_access_limit_number" ,
		//.get= _rt_edp_get_naptAccessLimit_number ,
		//.set= _rt_edp_set_naptAccessLimit_number ,
		.group_bitmask = RTK_RG_PROC_GROUP_CTC_BITMASK | RTK_RG_PROC_GROUP_TCPUDP_BITMASK,		
	},
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
					//apolloPro not support this proc
#else
	{
		.name="ipv6MC_tranlate_ingressVid" ,
		//.get= _rt_edp_ipv6MC_tranlate_ingressVid_state ,
		//.set= _rt_edp_ipv6MC_tranlate_ingressVid_control ,
		.group_bitmask = RTK_RG_PROC_GROUP_MULTICAST_BITMASK|RTK_RG_PROC_GROUP_VLAN_BITMASK,	
	},
#endif
	{
		.name="unknownDA_trap_to_PS" ,
		//.get= _rt_edp_unknowDA_trap_to_PS_state ,
		//.set= _rt_edp_unknowDA_trap_to_PS_change ,
		.group_bitmask = RTK_RG_PROC_GROUP_TRAP_BITMASK|RTK_RG_PROC_GROUP_PS_BITMASK,
	},

	{
		.name="hwnat" ,
		//.get= rt_edp_hwnat_is_enabled ,
		//.set= rt_edp_hwnat_enable ,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK | RTK_RG_PROC_GROUP_TRAP_BITMASK ,
	},
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	{
		.name="hwnat_change_without_flush_flow" ,
		//.get= rt_edp_hwnat_change_without_flush_get ,
		//.set= rt_edp_hwnat_change_without_flush_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TEST_BITMASK | RTK_RG_PROC_GROUP_TRAP_BITMASK ,
	},
#endif
#if !defined(CONFIG_RG_G3_SERIES)	// CONFIG_RG_G3_SERIES_DEVELOPMENT
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	{
		.name="fragment_ipv4_per_l34_intf_trap" ,
		//.get= rt_edp_fragment_ipv4_trap_get,
		//.set= rt_edp_fragment_ipv4_trap_set,
		.group_bitmask = RTK_RG_PROC_GROUP_ACL_BITMASK | RTK_RG_PROC_GROUP_TRAP_BITMASK ,
	},
#endif
#endif
	{
		.name="debug_level" ,
		.get= rt_edp_debug_level_show ,
		.set= rt_edp_debug_level_change ,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
	{
		.name="filter_level" ,
		//.get= rt_edp_filter_level_show ,
		//.set= rt_edp_filter_level_change ,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,		
	},
	{
		.name="trace_filter" ,
		//.get= rt_edp_traceFilterShow ,
		//.set= rt_edp_traceFilterChange ,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,		
	},
#if defined(CONFIG_RG_RTL9600_SERIES)
	{
		.name="http_trap_bridg_only" ,
		//.get=  _acl_trap_bridge_upstream_http_get,
		//.set=  _acl_trap_bridge_upstream_http_set,
		.group_bitmask = RTK_RG_PROC_GROUP_TRAP_BITMASK,		
	},
#endif	
#if defined(rt_edp_INGRESS_QOS_TEST_PATCH) && defined(CONFIG_RG_RTL9600_SERIES)
	{
		.name="qos_type" ,
		//.get= NULL ,
		//.set= rt_edp_qos_type_sel ,
		.group_bitmask = RTK_RG_PROC_GROUP_QOS_BITMASK,	
	},
#endif

#ifdef CONFIG_RG_CALLBACK
	{
		.name="callback" ,
		//.get= rt_edp_callback_show ,
		//.set= NULL ,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
#endif


	{
		.name="acl_drop_ip_range_rule_handle_by_sw" ,
		//.get= acl_drop_ip_range_by_sw_get ,
		//.set= acl_drop_ip_range_by_sw_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_ACL_BITMASK | RTK_RG_PROC_GROUP_DROP_BITMASK,
	},
	{
		.name="acl_permit_ip_range_rule_handle_by_sw" ,
		//.get= acl_permit_ip_range_by_sw_get ,
		//.set= acl_permit_ip_range_by_sw_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_ACL_BITMASK,
	},
	{
		.name="qosInternalAndRemark" ,
		//.get= rt_edp_qosInternalAndRemark_show ,
		//.set= NULL ,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
	{
		.name="acl_reserved_arrange" ,
		//.get= rt_edp_reserved_acl_arrange_show ,
		//.set= NULL ,
		.group_bitmask = RTK_RG_PROC_GROUP_ACL_BITMASK | RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
				//apolloPro not support this proc
#else
	{
		.name="acl_skip_hw_rearrange" ,
		//.get= _rt_edp_acl_skip_hw_rearrange_get,
		//.set= _rt_edp_acl_skip_hw_rearrange_set,
		.group_bitmask = RTK_RG_PROC_GROUP_ACL_BITMASK,
	},
#endif
	{
		.name="arp_timeout" ,
		//.get= _rt_edp_get_arp_timeout ,
		//.set= _rt_edp_set_arp_timeout ,
		.group_bitmask = RTK_RG_PROC_GROUP_ARP_BITMASK | RTK_RG_PROC_GROUP_AGING_BITMASK,
	},
	{
		.name="neighbor_timeout" ,
		//.get= _rt_edp_get_neighbor_timeout ,
		//.set= _rt_edp_set_neighbor_timeout ,
		.group_bitmask = RTK_RG_PROC_GROUP_ARP_BITMASK | RTK_RG_PROC_GROUP_AGING_BITMASK,
	},
	{
		.name="tcp_long_timeout" ,
		//.get= _rt_edp_get_tcp_long_timeout ,
		//.set= _rt_edp_set_tcp_long_timeout ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK | RTK_RG_PROC_GROUP_AGING_BITMASK,
	},
	{
		.name="tcp_short_timeout" ,
		//.get= _rt_edp_get_tcp_short_timeout ,
		//.set= _rt_edp_set_tcp_short_timeout ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK | RTK_RG_PROC_GROUP_AGING_BITMASK,		
	},
	{
		.name="udp_long_timeout" ,
		//.get= _rt_edp_get_udp_long_timeout ,
		//.set= _rt_edp_set_udp_long_timeout ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK | RTK_RG_PROC_GROUP_AGING_BITMASK,		
	},
	{
		.name="udp_short_timeout" ,
		//.get= _rt_edp_get_udp_short_timeout ,
		//.set= _rt_edp_set_udp_short_timeout ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK | RTK_RG_PROC_GROUP_AGING_BITMASK,		
	},
#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support lut traffic bit
	{
		.name="l2_timeout" ,
		//.get= _rt_edp_get_l2_timeout ,
		//.set= _rt_edp_set_l2_timeout ,
		.group_bitmask = RTK_RG_PROC_GROUP_L2_BITMASK | RTK_RG_PROC_GROUP_AGING_BITMASK,	
	},
#endif
	{
		.name="fragment_timeout" ,
		//.get= _rt_edp_get_fragment_timeout ,
		//.set= _rt_edp_set_fragment_timeout ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK | RTK_RG_PROC_GROUP_AGING_BITMASK,	
	},
	{
		.name="algSipPreserve_timeout" ,
		//.get= _rt_edp_get_alg_sip_preserve_timeout ,
		//.set= _rt_edp_set_alg_sip_preserve_timeout ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK | RTK_RG_PROC_GROUP_AGING_BITMASK,	
	},
	{
		.name="house_keep_sec" ,
		//.get= _rt_edp_get_house_keep_sec ,
		//.set= _rt_edp_set_house_keep_sec ,
		.group_bitmask = RTK_RG_PROC_GROUP_AGING_BITMASK,
	},
	{
		.name="igmp_sys_timer_sec" ,
		//.get= _rt_edp_get_igmp_sys_timer_sec ,
		//.set= _rt_edp_set_igmp_sys_timer_sec ,
		.group_bitmask = RTK_RG_PROC_GROUP_AGING_BITMASK | RTK_RG_PROC_GROUP_MULTICAST_BITMASK,
	},
	{
		.name="igmp_max_simultaneous_group_size" ,
		//.get= _rt_edp_get_igmp_max_simultaneous_group_size ,
		//.set= _rt_edp_set_igmp_max_simultaneous_group_size ,
		.group_bitmask = RTK_RG_PROC_GROUP_MULTICAST_BITMASK,
	},
	{
		.name="igmp_max_system_client_size" ,
		//.get= _rt_edp_get_igmp_max_system_client_size ,
		//.set= _rt_edp_set_igmp_max_system_client_size ,
		.group_bitmask = RTK_RG_PROC_GROUP_MULTICAST_BITMASK,
	},
	{
		.name="mld_max_system_client_size" ,
		//.get= _rt_edp_get_mld_max_system_client_size ,
		//.set= _rt_edp_set_mld_max_system_client_size ,
		.group_bitmask = RTK_RG_PROC_GROUP_MULTICAST_BITMASK,
	},
	{
		.name="igmp_max_specific_client_join_group_size" ,
		//.get= _rt_edp_get_igmp_max_specific_client_join_group_size ,
		//.set= _rt_edp_set_igmp_max_specific_client_join_group_size ,
		.group_bitmask = RTK_RG_PROC_GROUP_MULTICAST_BITMASK,
	},
	{
		.name="mld_max_specific_client_join_group_size" ,
		//.get= _rt_edp_get_mld_max_specific_client_join_group_size ,
		//.set= _rt_edp_set_mld_max_specific_client_join_group_size ,
		.group_bitmask = RTK_RG_PROC_GROUP_MULTICAST_BITMASK,
	},	
#if defined(CONFIG_RG_RTL9600_SERIES)
	{
		.name="igmp_pppoe_passthrough_learning" ,
		//.get= _rt_edp_get_igmp_pppoe_passthrough_learning ,
		//.set= _rt_edp_set_igmp_pppoe_passthrough_learning ,
	},			
#endif
	{
		.name="igmp_whitelist" ,
		//.get= rt_edp_igmp_whitelistShow,
		//.set= rt_edp_igmp_whitelistSet ,
		.group_bitmask = RTK_RG_PROC_GROUP_MULTICAST_BITMASK,
	},
	{
		.name="igmp_blacklist" ,
		//.get= rt_edp_igmp_blacklistShow,
		//.set= rt_edp_igmp_blacklistSet ,
		.group_bitmask = RTK_RG_PROC_GROUP_MULTICAST_BITMASK,
	},	
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)	
	{
		.name="igmp_auto_learn_ctagif" ,
		//.get= _rt_edp_get_igmp_auto_learn_ctagif,
		//.set= _rt_edp_set_igmp_auto_learn_ctagif ,
		.group_bitmask = RTK_RG_PROC_GROUP_MULTICAST_BITMASK ,
	},		
#endif	
	{
		.name="mcast_query_sec" ,
		.get= _rt_edp_get_mcast_query_sec ,
		.set= _rt_edp_set_mcast_query_sec ,
		.group_bitmask = RTK_RG_PROC_GROUP_MULTICAST_BITMASK | RTK_RG_PROC_GROUP_AGING_BITMASK,
	},
	{
		.name="mcast_protocol" ,
		//.get= _rt_edp_get_mcast_protocol ,
		//.set= _rt_edp_set_mcast_protocol ,
		.group_bitmask = RTK_RG_PROC_GROUP_MULTICAST_BITMASK,
	},
	{
		.name="mcast_force_report_sec" ,
		//.get= _rt_edp_get_mcast_force_report_sec ,
		//.set= _rt_edp_set_mcast_force_report_sec ,
		.group_bitmask = RTK_RG_PROC_GROUP_MULTICAST_BITMASK | RTK_RG_PROC_GROUP_AGING_BITMASK,
	},
	{
		.name="unlearnedSA_rate_limit" ,
		//.get= rt_edp_unlearnedSARateLimit_get ,
		//.set= rt_edp_unlearnedSARateLimit_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK,
	},
	{
		.name="overMTU_rate_limit" ,
		//.get= rt_edp_overMTURateLimit_get ,
		//.set= rt_edp_overMTURateLimit_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK,
	},
	{
		.name="overMTU_rate_limit_portMask" ,
		//.get= rt_edp_overMTURateLimitPortMask_get ,
		//.set= rt_edp_overMTURateLimitPortMask_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK,		
	},
	{
		.name="ArpReq_rate_limit" ,
		//.get= rt_edp_ArpReqRateLimit_get ,
		//.set= rt_edp_ArpReqRateLimit_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK,
	},
	{
		.name="ArpReq_rate_limit_portMask" ,
		//.get= rt_edp_ArpReqRateLimitPortMask_get ,
		//.set= rt_edp_ArpReqRateLimitPortMask_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK,		
	},
	{
		.name="dos_rate_limit" ,
		//.get= rt_edp_dosRateLimit_get ,
		//.set= rt_edp_dosRateLimit_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK,
	},
	{
		.name="dos_rate_limit_pattern" ,
		//.get= rt_edp_dosRateLimitPattern_get ,
		//.set= rt_edp_dosRateLimitPattern_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK,		
	},
	{
		.name="igmp_rate_limit" ,
		//.get= rt_edp_igmpRateLimit_get ,
		//.set= rt_edp_igmpRateLimit_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK,
	},
	{
		.name="igmp_rate_limit_portMask" ,
		//.get= rt_edp_igmpRateLimitPortMask_get ,
		//.set= rt_edp_igmpRateLimitPortMask_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK,		
	},
	{
		.name="dhcp_rate_limit" ,
		//.get= rt_edp_dhcpRateLimit_get ,
		//.set= rt_edp_dhcpRateLimit_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK,
	},
	{
		.name="dhcp_rate_limit_portMask" ,
		//.get= rt_edp_dhcpRateLimitPortMask_get ,
		//.set= rt_edp_dhcpRateLimitPortMask_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK,		
	},
	{
		.name="unknownDA_rate_limit" ,
		//.get= rt_edp_unknownDARateLimit_get ,
		//.set= rt_edp_unknownDARateLimit_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK,	
	},
	{
		.name="unknownDA_rate_limit_portMask" ,
		//.get= rt_edp_unknownDARateLimitPortMask_get ,
		//.set= rt_edp_unknownDARateLimitPortMask_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK,	
	},
	{
		.name="BC_rate_limit" ,
		//.get= rt_edp_BCRateLimit_get ,
		//.set= rt_edp_BCRateLimit_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK,	
	},
	{
		.name="BC_rate_limit_portMask" ,
		//.get= rt_edp_BCRateLimitPortMask_get ,
		//.set= rt_edp_BCRateLimitPortMask_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK,	
	},
	{
		.name="IPv6_MC_rate_limit" ,
		//.get= rt_edp_IPv6MCRateLimit_get ,
		//.set= rt_edp_IPv6MCRateLimit_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK | RTK_RG_PROC_GROUP_MULTICAST_BITMASK,
	},
	{
		.name="IPv6_MC_rate_limit_portMask" ,
		//.get= rt_edp_IPv6MCRateLimitPortMask_get ,
		//.set= rt_edp_IPv6MCRateLimitPortMask_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK | RTK_RG_PROC_GROUP_MULTICAST_BITMASK,		
	},
	{
		.name="IPv4_MC_rate_limit" ,
		//.get= rt_edp_IPv4MCRateLimit_get ,
		//.set= rt_edp_IPv4MCRateLimit_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK | RTK_RG_PROC_GROUP_MULTICAST_BITMASK,		
	},
	{
		.name="IPv4_MC_rate_limit_portMask" ,
		//.get= rt_edp_IPv4MCRateLimitPortMask_get ,
		//.set= rt_edp_IPv4MCRateLimitPortMask_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK | RTK_RG_PROC_GROUP_MULTICAST_BITMASK,		
	},
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	{
		.name="wifi_ingress_rate_limit" ,
		//.get= rt_edp_WifiIngressRateLimit_get ,
		//.set= rt_edp_WifiIngressRateLimit_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK | RTK_RG_PROC_GROUP_WIFI_BITMASK,		
	},
	{
		.name="wifi_egress_rate_limit" ,
		//.get= rt_edp_WifiEgressRateLimit_get ,
		//.set= rt_edp_WifiEgressRateLimit_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK | RTK_RG_PROC_GROUP_WIFI_BITMASK,	
	},
	{
		.name="wifi_device_enumerate" ,
		//.get= NULL ,
		//.set= rt_edp_wifiDeviceEnumerate ,
		.group_bitmask = RTK_RG_PROC_GROUP_WIFI_BITMASK,	
	},
	{
		.name="wifi_check_device" ,
		//.get= _rt_edp_checkWifiDev_get ,
		//.set= _rt_edp_checkWifiDev_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_WIFI_BITMASK,
	},
#endif
	{
		.name="alg_user_defined_port_num" ,
		//.get= rt_edp_algUserDefinedPortNum_get ,
		//.set= rt_edp_algUserDefinedPortNum_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_ALG_BITMASK,
	},
	{
		.name="alg_user_defined_time_out" ,
		//.get= rt_edp_algUserDefinedTimeOut_get ,
		//.set= rt_edp_algUserDefinedTimeOut_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_ALG_BITMASK | RTK_RG_PROC_GROUP_AGING_BITMASK,
	},
	{
		.name="tcp_hw_learning_at_syn" ,
		//.get= rt_edp_tcp_hw_learning_at_syn_get ,
		//.set= rt_edp_tcp_hw_learning_at_syn_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK,
	},
	{
		.name="tcp_in_shortcut_learning_at_syn" ,
		//.get= rt_edp_tcp_in_shortcut_learning_at_syn_get ,
		//.set= rt_edp_tcp_in_shortcut_learning_at_syn_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK,		
	},
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM) && !defined(CONFIG_RG_G3_SERIES)
	{
		.name="tcp_disable_syn_ack_trap" ,
		//.get= rt_edp_tcp_disable_syn_ack_trap_get ,
		//.set= rt_edp_tcp_disable_syn_ack_trap_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK,		
	},
#endif
#ifdef CONFIG_ROME_NAPT_SHORTCUT
	{
		.name="turn_off_ipv4_shortcut" ,
		//.get= rt_edp_ipv4_shortcutOff_function_get ,
		//.set= rt_edp_ipv4_shortcutOff_function_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK | RTK_RG_PROC_GROUP_DEBUG_BITMASK,			
	},
	{
		.name="ipv4_shortcut_timeout" ,
		//.get= _rt_edp_ipv4_shortcut_timeout_get ,
		//.set= _rt_edp_ipv4_shortcut_timeout_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK | RTK_RG_PROC_GROUP_AGING_BITMASK,			
	},
#endif
	{
		.name="turn_off_fragShortcut" ,
		//.get= rt_edp_fragShortcutOff_function_get ,
		//.set= rt_edp_fragShortcutOff_function_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK | RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
	{
		.name="turn_off_ipv6_shortcut" ,
		//.get= rt_edp_ipv6_shortcutOff_function_get ,
		//.set= rt_edp_ipv6_shortcutOff_function_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK | RTK_RG_PROC_GROUP_DEBUG_BITMASK,	
	},
	{
		.name="ipv6_shortcut_timeout" ,
		//.get= _rt_edp_ipv6_shortcut_timeout_get ,
		//.set= _rt_edp_ipv6_shortcut_timeout_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK | RTK_RG_PROC_GROUP_AGING_BITMASK,
	},
#endif
#if defined(CONFIG_RG_FLOW_AUTO_AGEOUT)
	{
		.name="flow_timeout" ,
		//.get= _rt_edp_flow_timeout_get ,
		//.set= _rt_edp_flow_timeout_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK | RTK_RG_PROC_GROUP_AGING_BITMASK,
	},
#endif
	{
		.name="strange_packet_drop" ,
		//.get= _rt_edp_strangePacketDrop_state ,
		//.set= _rt_edp_strangePacketDrop_change ,
		.group_bitmask = RTK_RG_PROC_GROUP_DROP_BITMASK | RTK_RG_PROC_GROUP_L2_BITMASK,
	},
	{
		.name="portBindingByProtocal" ,
		//.get= _rt_edp_portBindingByProtocal_state ,
		//.set= _rt_edp_portBindingByProtocal_change ,
		.group_bitmask = RTK_RG_PROC_GROUP_CTC_BITMASK,
	},
	{
		.name="portBindingByProtocal_filter_downstream_vid" ,
		//.get= _rt_edp_portBindingByProtocal_filter_vid_for_downstream_get ,
		//.set= _rt_edp_portBindingByProtocal_filter_vid_for_downstream_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_CTC_BITMASK | RTK_RG_PROC_GROUP_VLAN_BITMASK,		
	},
	{
		.name="l4ways" ,
		//.get= _rt_edp_proc_l4ways_dump ,
		//.set= _rt_edp_proc_l4ways_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
	{
		.name="l4ways_list" ,
		//.get= _rt_edp_proc_l4waysList_get ,
		//.set= _rt_edp_proc_l4waysList_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
	{
		.name="l4_choice_hw_in" ,
		//.get= _rt_edp_proc_l4ChoiceHwIn_get ,
		//.set= _rt_edp_proc_l4ChoiceHwIn_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
	{
		.name="tcp_do_not_del_when_rst_fin" ,
		//.get= _rt_edp_proc_tcpDoNotDelWhenRstFin_get ,
		//.set= _rt_edp_proc_tcpDoNotDelWhenRstFin_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK,
	},
	{
		.name="tcp_swap_fin_del_rst" ,
		//.get= _rt_edp_proc_tcpSwapFinDelRst_get ,
		//.set= _rt_edp_proc_tcpSwapFinDelRst_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK,
	},
	{
		.name="tcp_disable_stateful_tracking" ,
		//.get= _rt_edp_proc_tcpDisableStatefulTracking_get ,
		//.set= _rt_edp_proc_tcpDisableStatefulTracking_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK,
	},
	{
		.name="tcp_disable_simultaneous_SYN" ,
		//.get= _rt_edp_proc_tcpDisableSimultaneousSYN_get ,
		//.set= _rt_edp_proc_tcpDisableSimultaneousSYN_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK | RTK_RG_PROC_GROUP_DROP_BITMASK,
	},
	{
		.name="fwd_statistic" ,
		//.get= _rt_edp_proc_fwdStatistic_get ,
		//.set= _rt_edp_proc_fwdStatistic_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
	{
		.name="tcp_short_timeout_housekeep_jiffies" ,
		//.get= _rt_edp_proc_tcpShortTimeoutHouseKeep_get ,
		//.set= _rt_edp_proc_tcpShortTimeoutHouseKeep_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK | RTK_RG_PROC_GROUP_AGING_BITMASK,
	},
	{
		.name="wanIntf_disable_ipv6_linkLocal_rsvACL" ,
		//.get= _wanIntf_disable_add_ipv6_linkLocal_state_get ,
		//.set= _wanIntf_disable_add_ipv6_linkLocal_state_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_CTC_BITMASK,
		
	},
	{
		.name="bridgeWan_drop_by_protocal" ,
		//.get= _bridgeWan_drop_by_protocal_get ,
		//.set= _bridgeWan_drop_by_protocal_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_CTC_BITMASK | RTK_RG_PROC_GROUP_DROP_BITMASK,
	},
	{
		.name="turn_off_arp_hw_traffic_info" ,
		//.get= _rt_edp_proc_turnOffARPTrafficInfo_get ,
		//.set= _rt_edp_proc_turnOffARPTrafficInfo_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_ARP_BITMASK,
	},
	{
		.name="arp_max_request_count" ,
		//.get= _rt_edp_proc_ARPMaxRequestCount_get ,
		//.set= _rt_edp_proc_ARPMaxRequestCount_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_ARP_BITMASK,
	},
	{
		.name="mac_anti_spoofing_enable",
		//.get=_rt_edp_proc_MacAntiSpoof_get,
		//.set=_rt_edp_proc_MacAntiSpoof_set,
		.group_bitmask = RTK_RG_PROC_GROUP_ARP_BITMASK | RTK_RG_PROC_GROUP_L2_BITMASK,
	},
	{
		.name="ip_anti_spoofing_enable",
		//.get=_rt_edp_proc_IpAntiSpoof_get,
		//.set=_rt_edp_proc_IpAntiSpoof_set,
		.group_bitmask = RTK_RG_PROC_GROUP_ARP_BITMASK | RTK_RG_PROC_GROUP_L2_BITMASK,
	},
	{
		.name="remove_l34_tag_for_same_mac" ,
		//.get= _rt_edp_proc_RemoveL34TagOption_get ,
		//.set= _rt_edp_proc_RemoveL34TagOption_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_CTC_BITMASK | RTK_RG_PROC_GROUP_VLAN_BITMASK,
	},
	{
		.name="urlFilter_mode" ,
		//.get= _rt_edp_proc_UrlFilterMode_get ,
		//.set= _rt_edp_proc_UrlFilterMode_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_DROP_BITMASK,
	},

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
		//apolloPro no need this proc to support avalanche
#else		
	{
		.name="trap_syn_and_disable_svlan" ,
		//.get= _rt_edp_proc_trapSynState_get ,
		//.set= _rt_edp_proc_trapSynState_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TRAP_BITMASK | RTK_RG_PROC_GROUP_VLAN_BITMASK,
	},
#endif
	{
		.name="assign_ack_priority_and_disable_svlan" , //for apollo 
		//.get= _rt_edp_proc_assignAckPriority_get_and_disabled_svlan ,
		//.set= _rt_edp_proc_assignAckPriority_set_and_disabled_svlan ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK | RTK_RG_PROC_GROUP_VLAN_BITMASK | RTK_RG_PROC_GROUP_QOS_BITMASK,
	},	
	{
		.name="assign_ack_priority" ,
		//.get= _rt_edp_proc_assignAckPriority_get ,
		//.set= _rt_edp_proc_assignAckPriority_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK | RTK_RG_PROC_GROUP_QOS_BITMASK,
	},
	{
		.name="trap_specific_length_ack" ,
		//.get= _rt_edp_proc_trap_specific_length_ack_get ,
		//.set= _rt_edp_proc_trap_specific_length_ack_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK | RTK_RG_PROC_GROUP_QOS_BITMASK | RTK_RG_PROC_GROUP_TRAP_BITMASK,
	},
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	{
		.name="assign_syn_share_meter" ,
		//.get= _rt_edp_proc_assignSynShareMeter_get ,
		//.set= _rt_edp_proc_assignSynShareMeter_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_RATELIMIT_BITMASK,
	},
#if defined(CONFIG_RG_RTL9607C_SERIES)	//backward compatible for trap with priority related feature on 9607C
	{
		.name="assign_arp_priority" ,
		//.get= _rt_edp_proc_assignArpPriority_get ,
		//.set= _rt_edp_proc_assignArpPriority_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_ARP_BITMASK | RTK_RG_PROC_GROUP_QOS_BITMASK,
	},
	
	{
		.name="assign_loop_detect_priority" ,
		//.get= _rt_edp_proc_assignLoopDetectPriority_get ,
		//.set= _rt_edp_proc_assignLoopDetectPriority_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_QOS_BITMASK,
	},
	
	{
		.name="assign_snmp_priority" ,
		//.get= _rt_edp_proc_assignSnmpPriority_get ,
		//.set= _rt_edp_proc_assignSnmpPriority_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_QOS_BITMASK,
	},
	
	{
		.name="assign_igmp_priority" ,
		//.get= _rt_edp_proc_assignIgmpPriority_get ,
		//.set= _rt_edp_proc_assignIgmpPriority_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_QOS_BITMASK | RTK_RG_PROC_GROUP_MULTICAST_BITMASK,
	},
	{
		.name="assign_dhcp_priority" ,
		//.get= _rt_edp_proc_assignDhcpPriority_get ,
		//.set= _rt_edp_proc_assignDhcpPriority_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_QOS_BITMASK,
	},
	
	{
		.name="assign_dhcpv6_priority" ,
		//.get= _rt_edp_proc_assignDhcpv6Priority_get ,
		//.set= _rt_edp_proc_assignDhcpv6Priority_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_QOS_BITMASK,
	},
	
	{
		.name="assign_access_ip_priority" ,
		//.get= _rt_edp_proc_assignAccessIpPriority_get ,
		//.set= _rt_edp_proc_assignAccessIpPriority_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_QOS_BITMASK,
	},
	{
		.name="assign_access_ip" ,
		//.get= _rt_edp_proc_assignAccessIp_get ,
		//.set= _rt_edp_proc_assignAccessIp_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_QOS_BITMASK,
	},
#endif
#endif	// end CONFIG_RG_FLOW_BASED_PLATFORM
	{
		.name="redirect_first_http_req_by_mac" ,
		//.get= _rt_edp_proc_redirectFirstHttpMAC_show ,
		//.set= _rt_edp_proc_redirectFirstHttpMAC_modify ,
		.group_bitmask = RTK_RG_PROC_GROUP_CTC_BITMASK,
	},
	{
		.name="redirect_first_http_req_set_url" ,
		//.get= _rt_edp_proc_redirectFirstHttpURL_show ,
		//.set= _rt_edp_proc_redirectFirstHttpURL_modify ,
		.group_bitmask = RTK_RG_PROC_GROUP_CTC_BITMASK,
	},
	{
		.name="avoid_force_portal_set_url" ,
		//.get= _rt_edp_proc_avoidPortalURL_show ,
		//.set= _rt_edp_proc_avoidPortalURL_modify ,	
		.group_bitmask = RTK_RG_PROC_GROUP_CTC_BITMASK,
	},
	{
		.name="trap_lan_enable" ,
		//.get= _rt_edp_proc_trapLan_get ,
		//.set= _rt_edp_proc_trapLan_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TRAP_BITMASK,
	},
	{
		.name="trap_lan_add_host" ,
		//.get= _rt_edp_proc_trapLan_show ,
		//.set= _rt_edp_proc_trapLanAdd_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TRAP_BITMASK,
	},
	{
		.name="trap_lan_del_host" ,
		//.get= _rt_edp_proc_trapLan_show ,
		//.set= _rt_edp_proc_trapLanDel_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TRAP_BITMASK,
	},
	{
		.name="stag_enable" ,
		//.get= rt_edp_stag_is_enabled ,
		//.set= rt_edp_stag_enable ,
		.group_bitmask = RTK_RG_PROC_GROUP_VLAN_BITMASK,
	},
	{
		.name="send_from_cpu" ,
		//.get= NULL ,
		//.set= _rt_edp_proc_sendFromCpu ,
		.unlockBefortWrite=1,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
	{
		.name="wifi_tx_redirect_to_port0" ,
		//.get= _rt_edp_proc_wifiTxRedirect_get ,
		//.set= _rt_edp_proc_wifiTxRedirect_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TEST_BITMASK,
	},

	{
		.name="ps_rx_mirror_to_port0" ,
		//.get= _rt_edp_proc_psRxMirrorToPort0_get ,
		//.set= _rt_edp_proc_psRxMirrorToPort0_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TEST_BITMASK,
	},
	{
		.name="proc_to_pipe" ,
		//.get= _rt_edp_proc_procToPipe_get ,
		//.set= _rt_edp_proc_procToPipe_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
#if !defined(CONFIG_OPENWRT_RG)
	{
		.name="dump_ps_rx_pkt" ,
		//.get= _rt_edp_proc_dumpPSRxPkt_get ,
		//.set= _rt_edp_proc_dumpPSRxPkt_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
	{
		.name="dump_ps_tx_pkt" ,
		//.get= _rt_edp_proc_dumpPSTxPkt_get ,
		//.set= _rt_edp_proc_dumpPSTxPkt_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
#endif
	{
		.name="igmpProxyOnly2Wifi" ,
		//.get= _rt_edp_igmp_igmpProxyOnly2Wifi_get ,
		//.set= _rt_edp_igmp_igmpProxyOnly2Wifi_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_MULTICAST_BITMASK|RTK_RG_PROC_GROUP_WIFI_BITMASK,
	},

	{
		.name="igmp_report_ingress_filter_portmask" ,
		//.get= _rt_edp_igmp_report_ingress_filter_portmask_get ,
		//.set= _rt_edp_igmp_report_ingress_filter_portmask_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_MULTICAST_BITMASK,
	},
	{
		.name="igmp_report_filter_portmask" ,
		//.get= _rt_edp_igmp_report_filter_portmask_get ,
		//.set= _rt_edp_igmp_report_filter_portmask_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_MULTICAST_BITMASK,
	},
	{
		.name="igmp_leave_filter_portmask" ,
		//.get= _rt_edp_igmp_leave_filter_portmask_get ,
		//.set= _rt_edp_igmp_leave_filter_portmask_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_MULTICAST_BITMASK,
	},	
	{
		.name="igmp_mld_query_filter_portmask" ,
		//.get= _rt_edp_igmp_mld_query_filter_portmask_get ,
		//.set= _rt_edp_igmp_mld_query_filter_portmask_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_MULTICAST_BITMASK,
	},

	{
		.name="gponDsBCModuleEnable" ,
		//.get= _rt_edp_gponDsBCModuleEnable_get ,
		//.set= _rt_edp_gponDsBCModuleEnable_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_MULTICAST_BITMASK | RTK_RG_PROC_GROUP_PON_BITMASK | RTK_RG_PROC_GROUP_VLAN_BITMASK,
	},
	{
		.name="acl_rg_add_parameter_dump" ,
		.get= _rt_edp_aclDumpParameterEnable_get ,
		.set= _rt_edp_aclDumpParameterEnable_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_ACL_BITMASK | RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},	
	{
		.name="pppoe_bc_passthrought_to_bindingWan" ,
		//.get= _rt_edp_pppoe_bc_passthrought_to_bindingWan_get ,
		//.set= _rt_edp_pppoe_bc_passthrought_to_bindingWan_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_CTC_BITMASK,
	},
	{
		.name="pppoe_mc_routing_trap" ,
		//.get= _rt_edp_pppoe_mc_routing_trap_state_get ,
		//.set= _rt_edp_pppoe_mc_routing_trap_state_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_MULTICAST_BITMASK | RTK_RG_PROC_GROUP_TRAP_BITMASK,
	},
	{
		.name="keep_protocol_stack_packets_orig_cvlan" ,
		//.get= _rt_edp_procotolStackKeepCVLANOrig_get ,
		//.set= _rt_edp_procotolStackKeepCVLANOrig_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_PS_BITMASK | RTK_RG_PROC_GROUP_VLAN_BITMASK,
	},
	{
		.name="check_http_mechanism_before_ps" ,
		//.get= _rt_edp_checkHttpMechanismBeforeProcotolStack_get ,
		//.set= _rt_edp_checkHttpMechanismBeforeProcotolStack_set ,
	},
	{
		.name="pppoe_proxy_only_for_binding_packet" ,
		//.get= _rt_edp_pppoeProxyAllowBindingOnly_get ,
		//.set= _rt_edp_pppoeProxyAllowBindingOnly_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_CTC_BITMASK,
	},
	
#if defined(CONFIG_APOLLO_GPON_FPGATEST)
	{
		.name="virtualMAC_with_PON" ,
		//.get= _rt_edp_virtualMAC_with_PON_display ,
		//.set= _rt_edp_virtualMAC_with_PON_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_PON_BITMASK | RTK_RG_PROC_GROUP_TEST_BITMASK,
	},
#endif
	{
		.name="control_hw_TTL_minus" ,
		//.get= _rt_edp_control_hw_ttl_minus_open ,
		//.set= _rt_edp_control_hw_ttl_minus_write ,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,		
	},

	{
		.name="dpi_callback_preroute" ,
		//.get= _rt_edp_proc_PreRouteCallback_get ,
		//.set= _rt_edp_proc_PreRouteCallback_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_CTC_BITMASK,
	},
	{
		.name="dpi_callback_forward" ,
		//.get= _rt_edp_proc_FwdCallback_get ,
		//.set= _rt_edp_proc_FwdCallback_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_CTC_BITMASK,
	},
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	{
        .name="flow_flush" ,
        //.get= _rt_edp_flushFlow_get,
        //.set= _rt_edp_flushFlow_set,
        .group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
    },
#else //not CONFIG_RG_FLOW_BASED_PLATFORM
#if defined(CONFIG_ROME_NAPT_SHORTCUT) || defined(CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT)
    {
        .name="flush_shortcut" ,
        //.get= _rt_edp_flushShortcut_get,
        //.set= _rt_edp_flushShortcut_set,
        .group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
    },
#endif
#endif
	{
        .name="flush_l4" ,
        //.get= _rt_edp_flushL4_get,
        //.set= _rt_edp_flushL4_set,
        .group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
    },
	{
		.name="gather_lanNetInfo" ,
		//.get= _rt_edp_proc_gatherLanNetInfo_get,
		//.set= _rt_edp_proc_gatherLanNetInfo_set,
		.group_bitmask = RTK_RG_PROC_GROUP_CTC_BITMASK,
	},

#ifdef CONFIG_APOLLOPRO_FPGA
	{
		.name="fpga_testing" ,
		//.get= NULL,
		//.set= single_test,
		.group_bitmask = RTK_RG_PROC_GROUP_TEST_BITMASK,
	},
#endif
#ifdef CONFIG_RG_BRIDGE_PPP_STATUS
	{
		.name="brg_pppstate" ,
		//.get= rt_edp_proc_brg_pppstate_get,
		//.set= NULL,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
#endif
	{
		.name="wan_dmac2cvid_force_disabled" ,
		//.get= _rt_edp_proc_disbaled_pon_dmac2cvid_get,
		//.set= _rt_edp_proc_disbaled_pon_dmac2cvid_set,
		.group_bitmask = RTK_RG_PROC_GROUP_L2_BITMASK | RTK_RG_PROC_GROUP_VLAN_BITMASK,
	},
	{
		.name="lan_dmac2cvid_force_disabled" ,
		//.get= _rt_edp_proc_disbaled_lan_dmac2cvid_get,
		//.set= _rt_edp_proc_disbaled_lan_dmac2cvid_set,
		.group_bitmask = RTK_RG_PROC_GROUP_L2_BITMASK | RTK_RG_PROC_GROUP_VLAN_BITMASK,
	},
	{
		.name="pppoe_gpon_small_bandwidth_control" ,
		//.get= rt_edp_proc_pppoe_gpon_small_bandwidth_control_get,
		//.set= rt_edp_proc_pppoe_gpon_small_bandwidth_control_set,
		.group_bitmask = RTK_RG_PROC_GROUP_PON_BITMASK,
	},
	{
		.name="gpon_pppoe_status" ,
		//.get= rt_edp_proc_gpon_pppoe_status_get,
		//.set= rt_edp_proc_gpon_pppoe_status_set,
	},
	{
		.name="pppoe_gpon_small_bandwidth_control_bridge_dmac_as_pppoe" ,
		//.get= rt_edp_proc_pppoe_gpon_small_bandwidth_control_support_bridgeWan_dmac_as_pppoe_get,
		//.set= rt_edp_proc_pppoe_gpon_small_bandwidth_control_support_bridgeWan_dmac_as_pppoe_set,
		.group_bitmask = RTK_RG_PROC_GROUP_PON_BITMASK,
	},
	
#ifdef CONFIG_DUALBAND_CONCURRENT
	{
		.name="slaveWifiBind" ,
		//.get= _rt_edp_enableSlaveWifiBind_get ,
		//.set= _rt_edp_enableSlaveWifiBind_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_CTC_BITMASK | RTK_RG_PROC_GROUP_WIFI_BITMASK,
	},
#endif
	{
		.name="port_range_used_by_ps" ,
		//.get= _rt_edp_proc_port_range_used_by_protocolStack_get ,
		//.set= _rt_edp_proc_port_range_used_by_protocolStack_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_PS_BITMASK,
	},
	{
		.name="log_rx_pcap" ,
		//.get= _rt_edp_proc_log_rx_pcap_get ,
		//.set= _rt_edp_proc_log_rx_pcap_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
	{
		.name="log_to_PS_pcap" ,
		//.get= _rt_edp_proc_log_to_PS_pcap_get ,
		//.set= _rt_edp_proc_log_to_PS_pcap_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK | RTK_RG_PROC_GROUP_PS_BITMASK,
	},
	{
		.name="layer2LookupMissFlood2CPU" ,
		//.get= _rt_edp_proc_layer2LookupMissFlood2CPU_get ,
		//.set= _rt_edp_proc_layer2LookupMissFlood2CPU_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_L2_BITMASK,
	},
	{
		.name="wan_remoteGWMac_update" ,
		//.get= _rt_edp_remoteGatewayMacUpdate_get ,
		//.set= _rt_edp_remoteGatewayMacUpdate_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_ARP_BITMASK,
	},
	{
		.name="local_in_napt_add_to_shortcut" ,
		//.get= _rt_edp_localInNaptAddToShortcut_get ,
		//.set= _rt_edp_localInNaptAddToShortcut_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_PS_BITMASK,
	},
	{
		.name="whiteListState" ,
		//.get= _rt_edp_whiteListState_get ,
		//.set= _rt_edp_whiteListState_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
    {
		.name="L2TcpUdpStatefulTracking" ,
		//.get= _rt_edp_L2TcpUdpStatefulTracking_get ,
		//.set= _rt_edp_L2TcpUdpStatefulTracking_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK | RTK_RG_PROC_GROUP_CTC_BITMASK,
	},
	{
		.name="L3TcpUdpStatefulTracking" ,
		//.get= _rt_edp_L3TcpUdpStatefulTracking_get ,
		//.set= _rt_edp_L3TcpUdpStatefulTracking_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK | RTK_RG_PROC_GROUP_CTC_BITMASK,
	},
	{
		.name="replaceHwFlow" ,
		//.get= _rt_edp_replaceHwFlow_get ,
		//.set= _rt_edp_replaceHwFlow_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
	{
        .name="flow_delete" ,
        //.get= NULL,
        //.set= _rt_edp_deleteFlow_set,
        .group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
    },
    {
        .name="flow_statistic" ,
        //.get= _rt_edp_sw_flowStatistic_get,
        //.set= _rt_edp_sw_flowStatistic_set,
        .group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
    },
    {
        .name="dip_list_on_the_intf" ,
        //.get= _rt_edp_dip_list_on_the_intf_get,
        //.set= _rt_edp_dip_list_on_the_intf_set,
        .group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
    },
    {
		.name="flow_not_update_in_real_time" ,
		//.get= _rt_edp_flow_not_update_in_real_time_get ,
		//.set= _rt_edp_flow_not_update_in_real_time_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK | RTK_RG_PROC_GROUP_CTC_BITMASK,
	},
#endif
	{
		.name="napt_flow_check_dpi_again" ,
		//.get= NULL ,
		//.set= _rt_edp_naptFlowCheckDpiAgain_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_CTC_BITMASK,
	},

#ifdef CONFIG_SMP
	{
		.name="smp_statistic" ,
		//.get= _rt_edp_proc_smpStatistic_get,
		//.set= _rt_edp_proc_smpStatistic_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},

	{
		.name="smp_nic_tx_cpu_from_cpu0" ,
		//.get= _rt_edp_proc_smpMapping_get,
		//.set= _rt_edp_proc_smpNicTxCpuFromCpu0_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},

	{
		.name="smp_nic_tx_cpu_from_cpu1" ,
		//.get= _rt_edp_proc_smpMapping_get,
		//.set= _rt_edp_proc_smpNicTxCpuFromCpu1_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},		
#if CONFIG_NR_CPUS == 4
	{
		.name="smp_nic_tx_cpu_from_cpu2" ,
		//.get= _rt_edp_proc_smpMapping_get,
		//.set= _rt_edp_proc_smpNicTxCpuFromCpu2_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},		

	{
		.name="smp_nic_tx_cpu_from_cpu3" ,
		//.get= _rt_edp_proc_smpMapping_get,
		//.set= _rt_edp_proc_smpNicTxCpuFromCpu3_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},
#endif
	{
		.name="smp_wifi_11ac_tx_cpu_from_cpu0" ,
		//.get= _rt_edp_proc_smpMapping_get,
		//.set= _rt_edp_proc_smpWifi11acTxCpuFromCpu0_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},

	{
		.name="smp_wifi_11ac_tx_cpu_from_cpu1" ,
		//.get= _rt_edp_proc_smpMapping_get,
		//.set= _rt_edp_proc_smpWifi11acTxCpuFromCpu1_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},
#if CONFIG_NR_CPUS == 4
	{
		.name="smp_wifi_11ac_tx_cpu_from_cpu2" ,
		//.get= _rt_edp_proc_smpMapping_get,
		//.set= _rt_edp_proc_smpWifi11acTxCpuFromCpu2_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},		

	{
		.name="smp_wifi_11ac_tx_cpu_from_cpu3" ,
		//.get= _rt_edp_proc_smpMapping_get,
		//.set= _rt_edp_proc_smpWifi11acTxCpuFromCpu3_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},			
#endif

	{
		.name="smp_wifi_11n_tx_cpu_from_cpu0" ,
		//.get= _rt_edp_proc_smpMapping_get,
		//.set= _rt_edp_proc_smpWifi11nTxCpuFromCpu0_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},
	
	{
		.name="smp_wifi_11n_tx_cpu_from_cpu1" ,
		//.get= _rt_edp_proc_smpMapping_get,
		//.set= _rt_edp_proc_smpWifi11nTxCpuFromCpu1_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},
#if CONFIG_NR_CPUS == 4
	{
		.name="smp_wifi_11n_tx_cpu_from_cpu2" ,
		//.get= _rt_edp_proc_smpMapping_get,
		//.set= _rt_edp_proc_smpWifi11nTxCpuFromCpu2_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},		
	
	{
		.name="smp_wifi_11n_tx_cpu_from_cpu3" ,
		//.get= _rt_edp_proc_smpMapping_get,
		//.set= _rt_edp_proc_smpWifi11nTxCpuFromCpu3_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},			
#endif

	{
		.name="smp_select_tx_gmac_from_cpu0" ,
		//.get= _rt_edp_proc_smpMapping_get,
		//.set= _rt_edp_proc_smpSelectTxGmacFromCpu0_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},

	{
		.name="smp_select_tx_gmac_from_cpu1" ,
		//.get= _rt_edp_proc_smpMapping_get,
		//.set= _rt_edp_proc_smpSelectTxGmacFromCpu1_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},
#if CONFIG_NR_CPUS == 4
	{
		.name="smp_select_tx_gmac_from_cpu2" ,
		//.get= _rt_edp_proc_smpMapping_get,
		//.set= _rt_edp_proc_smpSelectTxGmacFromCpu2_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},

	{
		.name="smp_select_tx_gmac_from_cpu3" ,
		//.get= _rt_edp_proc_smpMapping_get,
		//.set= _rt_edp_proc_smpSelectTxGmacFromCpu3_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},
#endif
	{
		.name="smp_gmac9_rx_by_which_cpu" ,
		//.get= _rt_edp_proc_smpGmac9RxByWhichCpu_get,
		//.set= _rt_edp_proc_smpGmac9RxByWhichCpu_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},
	{
		.name="smp_gmac10_rx_by_which_cpu" ,
		//.get= _rt_edp_proc_smpGmac10RxByWhichCpu_get,
		//.set= _rt_edp_proc_smpGmac10RxByWhichCpu_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},

#ifdef RG_BY_TASKLET
#else
	{
		.name="smp_rg_rx_cpu_from_cpu0" ,
		//.get= _rt_edp_proc_smpMapping_get,
		//.set= _rt_edp_proc_smpRgRxCpuFromCpu0_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},
	
	{
		.name="smp_rg_rx_cpu_from_cpu1" ,
		//.get= _rt_edp_proc_smpMapping_get,
		//.set= _rt_edp_proc_smpRgRxCpuFromCpu1_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},		
#if CONFIG_NR_CPUS == 4
	{
		.name="smp_rg_rx_cpu_from_cpu2" ,
		//.get= _rt_edp_proc_smpMapping_get,
		//.set= _rt_edp_proc_smpRgRxCpuFromCpu2_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},		
	
	{
		.name="smp_rg_rx_cpu_from_cpu3" ,
		//.get= _rt_edp_proc_smpMapping_get,
		//.set= _rt_edp_proc_smpRgRxCpuFromCpu3_set,
		.group_bitmask = RTK_RG_PROC_GROUP_SMP_BITMASK,
	},
#endif
#endif	

		
#endif
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	{
		.name="prehashptn_sport" ,
		//.get= _rt_edp_proc_prehashptn_sport_get,
		//.set= _rt_edp_proc_prehashptn_sport_set,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
	{
		.name="prehashptn_dport" ,
		//.get= _rt_edp_proc_prehashptn_dport_get,
		//.set= _rt_edp_proc_prehashptn_dport_set,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
	{
		.name="prehashptn_sip" ,
		//.get= _rt_edp_proc_prehashptn_sip_get,
		//.set= _rt_edp_proc_prehashptn_sip_set,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
	{
		.name="prehashptn_dip" ,
		//.get= _rt_edp_proc_prehashptn_dip_get,
		//.set= _rt_edp_proc_prehashptn_dip_set,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
	{
		.name="trap_hash_result_to_cpu_port" ,
		//.get= _rt_edp_proc_trap_hash_result_to_cpu_port_get,
		//.set= _rt_edp_proc_trap_hash_result_to_cpu_port_set,
		.group_bitmask = RTK_RG_PROC_GROUP_TRAP_BITMASK | RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
	{
		.name="trap_hash_select_state" ,
		//.get= _rt_edp_proc_trap_hash_select_state_get,
		//.set= _rt_edp_proc_trap_hash_select_state_set,
		.group_bitmask = RTK_RG_PROC_GROUP_TRAP_BITMASK | RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},
#endif
	{
		.name="autoTestMode" ,
		//.get= _rt_edp_autoTestMode_get ,
		//.set= _rt_edp_autoTestMode_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TEST_BITMASK,
	},
	{
		.name="srcAddrBindingEnable" ,
		//.get= _rt_edp_srcAddrBindingEnable_get ,
		//.set= _rt_edp_srcAddrBindingEnable_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TCPUDP_BITMASK,
	},
	{
		.name="keep_ps_eth" ,
		//.get= rt_edp_proc_protocolStackKeep_get ,
		//.set= rt_edp_proc_protocolStackKeep_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_PS_BITMASK,
	},
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_G3_SERIES)	
	{
		.name="prevent_control_packet_drop" ,
		//.get= rt_edp_proc_prevent_control_packet_drop_get ,
		//.set= rt_edp_proc_prevent_control_packet_drop_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_QOS_BITMASK,
	},
	{
		.name="tunnelHwAcceleration_enable" ,
		//.get= rt_edp_proc_tunnelHwAcceleration_enable_get ,
		//.set= rt_edp_proc_tunnelHwAcceleration_enable_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_TRAP_BITMASK | RTK_RG_PROC_GROUP_DEBUG_BITMASK,		
	},
#endif	
	{
		.name="debug_message_display_to_current_tty" ,
		//.get= rt_edp_proc_debug_message_display_to_tty_enable_get ,
		//.set= rt_edp_proc_debug_message_display_to_tty_enable_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_DEBUG_BITMASK,		
	},
	{
		.name="drop_superfluous_packet" ,
		//.get= _rt_edp_superfluousPacketDrop_get ,
		//.set= _rt_edp_superfluousPacketDrop_set ,
		.group_bitmask =  RTK_RG_PROC_GROUP_ARP_BITMASK | RTK_RG_PROC_GROUP_DROP_BITMASK,
	},
	{
		.name="hybrid_pptp_portmask" ,
		//.get= _rt_edp_hybrid_pptp_portmask_get ,
		//.set= _rt_edp_hybrid_pptp_portmask_set ,
		.group_bitmask =  RTK_RG_PROC_GROUP_PS_BITMASK | RTK_RG_PROC_GROUP_PON_BITMASK,
	},
#if defined(CONFIG_RG_RTL9600_SERIES)
	{
		.name="reset_sw_recovery_hwnat" ,
		//.get= _rt_edp_swResetAndRecoveryHWNAT_get ,
		//.set= _rt_edp_swResetAndRecoveryHWNAT_set ,
		.group_bitmask =  RTK_RG_PROC_GROUP_DEBUG_BITMASK,
	},	
#endif
#if defined(CONFIG_RG_RTL9607C_SERIES)	
	{
		.name="wlan0_flow_ctrl_on_threshold_mbps" ,
		//.get= rt_edp_proc_wifi_flow_ctrl_threshold_get ,
		//.set= rt_edp_proc_wlan0_flow_ctrl_on_threshold_mbps_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_QOS_BITMASK,
	},
	{
		.name="wlan0_flow_ctrl_off_threshold_mbps" ,
		//.get= rt_edp_proc_wifi_flow_ctrl_threshold_get ,
		//.set= rt_edp_proc_wlan0_flow_ctrl_off_threshold_mbps_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_QOS_BITMASK,	
	},
	{
		.name="wlan1_flow_ctrl_on_threshold_mbps" ,
		//.get= rt_edp_proc_wifi_flow_ctrl_threshold_get ,
		//.set= rt_edp_proc_wlan1_flow_ctrl_on_threshold_mbps_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_QOS_BITMASK,	
	},
	{
		.name="wlan1_flow_ctrl_off_threshold_mbps" ,
		//.get= rt_edp_proc_wifi_flow_ctrl_threshold_get ,
		//.set= rt_edp_proc_wlan1_flow_ctrl_off_threshold_mbps_set ,
		.group_bitmask = RTK_RG_PROC_GROUP_QOS_BITMASK,	
	},
#endif	



};






static void *rt_edp_single_start(struct seq_file *p, loff_t *pos)
{
	return NULL + (*pos == 0);
}

static void *rt_edp_single_next(struct seq_file *p, void *v, loff_t *pos)
{
	++*pos;
	return NULL;
}

static void rt_edp_single_stop(struct seq_file *p, void *v)
{
}

int rt_edp_seq_open(struct file *file, const struct seq_operations *op)
{
	struct seq_file *p = file->private_data;

	if (!p) {
			p = kmalloc(sizeof(*p), GFP_ATOMIC);
			if (!p){
				return -ENOMEM;
			}
			file->private_data = p;
	}
	memset(p, 0, sizeof(*p));
	mutex_init(&p->lock);
	p->op = op;
#ifdef CONFIG_USER_NS
	p->user_ns = file->f_cred->user_ns;
#endif

	file->f_version = 0;

	file->f_mode &= ~FMODE_PWRITE;
	return 0;
}


int rt_edp_single_open(struct file *file, int (*show)(struct seq_file *, void *),void *data)
{
	struct seq_operations *op = kmalloc(sizeof(*op), GFP_ATOMIC);
	int res = -ENOMEM;

	if (op) {
		op->start = rt_edp_single_start;
		op->next = rt_edp_single_next;
		op->stop = rt_edp_single_stop;
		op->show = show;
		res = rt_edp_seq_open(file, op);
		if (!res)
			((struct seq_file *)file->private_data)->private = data;
		else
			kfree(op);
	}
	return res;
}


static int common_rt_edp_single_open(struct inode *inode, struct file *file)
{
    int i,r=-1,j,match_idx=-1;
	
#if 0
#ifdef CONFIG_SMP
	int smp_id=smp_processor_id();
#ifdef RG_BY_TASKLET
#else
	smp_id=rg_db.systemGlobal.smpRgCpuFromCpu[smp_id];
#endif
	rg_inbound_queue_lock(smp_id,&rg_kernel.rg_inbound_queue_lock);
#endif
#endif

    for( i=0; i< (sizeof(rtEdpProc)/sizeof(rtk_rg_proc_t)) ;i++)
    {
		//printk("common_single_open inode_id=%u i_ino=%u\n",rtEdpProc[i].inode_id,(unsigned int)inode->i_ino);

		for(j=0;j<RTK_RG_MAX_PROC_GROUP;j++)
		{
	        if(rtEdpProc[i].inode_id[j]==(unsigned int)inode->i_ino)
	        {
				r = (rt_edp_single_open(file, rtEdpProc[i].get, NULL));
				match_idx=i;
				break;
	        }
		}
		if(match_idx!=-1) break;
    }

#if 0	
#ifdef CONFIG_SMP
	rg_inbound_queue_unlock(&rg_kernel.rg_inbound_queue_lock);
#endif	
#endif

    return r;       
}


static ssize_t common_rt_edp_single_write(struct file * file, const char __user * userbuf,
				size_t count, loff_t * off)
{
    int i,r=-1,j,match_idx=-1;

#if 0	
#ifdef CONFIG_SMP
	int smp_id=smp_processor_id();
#ifdef RG_BY_TASKLET
#else
	smp_id=rg_db.systemGlobal.smpRgCpuFromCpu[smp_id];
#endif
	rg_inbound_queue_lock(smp_id,&rg_kernel.rg_inbound_queue_lock);
#endif
#endif


    for( i=0; i< (sizeof(rtEdpProc)/sizeof(rtk_rg_proc_t)) ;i++)
    {
		//printk("common_single_write inode_id=%u i_ino=%u\n",rtEdpProc[i].inode_id,(unsigned int)file->f_dentry->d_inode->i_ino);
		for(j=0;j<RTK_RG_MAX_PROC_GROUP;j++)
		{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 3)
			if(rtEdpProc[i].inode_id[j]==(unsigned int)file->f_inode->i_ino)
#else
	        if(rtEdpProc[i].inode_id[j]==(unsigned int)file->f_dentry->d_inode->i_ino)
#endif
	        {


#if 0
#ifdef CONFIG_SMP
				if(rtEdpProc[i].unlockBefortWrite==1)				
					rg_inbound_queue_unlock(&rg_kernel.rg_inbound_queue_lock);
				
#endif
#endif

				r=rtEdpProc[i].set(file,userbuf,count,off);
				match_idx=i;

#if 0
#ifdef CONFIG_SMP
				if(rtEdpProc[i].unlockBefortWrite!=1)
					rg_inbound_queue_unlock(&rg_kernel.rg_inbound_queue_lock);
#endif
#endif

				break;
	        }
		}
		if(match_idx!=-1) break;
    }

    return r;      
}


static int NULL_rt_edp_get(struct seq_file *s, void *v){ return 0;}

static int NULL_rt_edp_single_open(struct inode *inode, struct file *file)
{
	int r=-1;
#if 0
#ifdef CONFIG_SMP
	int smp_id=smp_processor_id();
#ifdef RG_BY_TASKLET
#else
	smp_id=rg_db.systemGlobal.smpRgCpuFromCpu[smp_id];
#endif
	rg_inbound_queue_lock(smp_id,&rg_kernel.rg_inbound_queue_lock);
#endif
#endif

	r = rt_edp_single_open(file, NULL_rt_edp_get, NULL);
#if 0
#ifdef CONFIG_SMP
	rg_inbound_queue_unlock(&rg_kernel.rg_inbound_queue_lock);
#endif
#endif

	return r;
}



struct proc_dir_entry *proc_rt_edp_group_list[RTK_RG_PROC_GROUP_MAX];
struct proc_dir_entry *proc_rt_edp_group;
struct proc_dir_entry *proc_rt_edp;



void rt_edp_proc_init(void){

	struct proc_dir_entry *p;//,*start;
	int32 i,j;

	char proc_group[RTK_RG_PROC_GROUP_MAX][16];


#ifdef CONFIG_SMP
	printk("\nrg_system_proc_init CPUID: %d\n",smp_processor_id());
#endif	

	proc_rt_edp=proc_mkdir("rg", NULL); 
	proc_rt_edp_group=proc_mkdir("rg2", NULL); 

	memset(proc_group,0,sizeof(proc_group));
	strcpy(&proc_group[RTK_RG_PROC_GROUP_ACL][0],"acl");
	strcpy(&proc_group[RTK_RG_PROC_GROUP_RATELIMIT][0],"ratelimit");
	strcpy(&proc_group[RTK_RG_PROC_GROUP_MULTICAST][0],"multicast");
	strcpy(&proc_group[RTK_RG_PROC_GROUP_DROP][0],"drop");
	strcpy(&proc_group[RTK_RG_PROC_GROUP_TRAP][0],"trap");
	strcpy(&proc_group[RTK_RG_PROC_GROUP_DEBUG][0],"debug");
	strcpy(&proc_group[RTK_RG_PROC_GROUP_VLAN][0],"vlan");
	strcpy(&proc_group[RTK_RG_PROC_GROUP_AGING][0],"aging");
	strcpy(&proc_group[RTK_RG_PROC_GROUP_QOS][0],"qos");
	strcpy(&proc_group[RTK_RG_PROC_GROUP_TCPUDP][0],"tcpudp");
	strcpy(&proc_group[RTK_RG_PROC_GROUP_CTC][0],"ctc");
	strcpy(&proc_group[RTK_RG_PROC_GROUP_WIFI][0],"wifi");
	strcpy(&proc_group[RTK_RG_PROC_GROUP_ARP][0],"arp");
	strcpy(&proc_group[RTK_RG_PROC_GROUP_ALG][0],"alg");
	strcpy(&proc_group[RTK_RG_PROC_GROUP_PS][0],"ps");
	strcpy(&proc_group[RTK_RG_PROC_GROUP_PON][0],"pon");
	strcpy(&proc_group[RTK_RG_PROC_GROUP_TEST][0],"test");
	strcpy(&proc_group[RTK_RG_PROC_GROUP_L2][0],"l2");
	strcpy(&proc_group[RTK_RG_PROC_GROUP_SMP][0],"smp");


	for(i=0;i<RTK_RG_PROC_GROUP_MAX;i++)
	{
		if(proc_group[i][0]!=0)
		{			
			if(proc_rt_edp_group_list[i]==NULL)
				proc_rt_edp_group_list[i] = proc_mkdir(proc_group[i], proc_rt_edp_group); 			
		}
	}


	for( i=0; i< (sizeof(rtEdpProc)/sizeof(rtk_rg_proc_t)) ;i++)
	{
		if(rtEdpProc[i].get==NULL)
			rtEdpProc[i].proc_fops.open=NULL_rt_edp_single_open;
		else
			rtEdpProc[i].proc_fops.open=common_rt_edp_single_open;
		
		if(rtEdpProc[i].set==NULL)
			rtEdpProc[i].proc_fops.write=NULL;
		else
			rtEdpProc[i].proc_fops.write=common_rt_edp_single_write;
		
		rtEdpProc[i].proc_fops.read=seq_read;
		rtEdpProc[i].proc_fops.llseek=seq_lseek;
		rtEdpProc[i].proc_fops.release=single_release;

		
		p = proc_create_data(rtEdpProc[i].name, 0644, proc_rt_edp , &(rtEdpProc[i].proc_fops),NULL);
		if(!p){
			printk("create proc rg/%s failed!\n",rtEdpProc[i].name);
		}
		
		rtEdpProc[i].inode_id[0] = p->low_ino;


		for(j=0;j<RTK_RG_PROC_GROUP_MAX;j++)
		{
			if((1<<j)&rtEdpProc[i].group_bitmask)
			{
				p = proc_create_data(rtEdpProc[i].name, 0644, proc_rt_edp_group_list[j] , &(rtEdpProc[i].proc_fops),NULL);
				if(!p){
					printk("create proc rg/%s failed!\n",rtEdpProc[i].name);
				}
				if(rtEdpProc[i].inode_id[1]==0) rtEdpProc[i].inode_id[1]=p->low_ino;
				else if(rtEdpProc[i].inode_id[2]==0) rtEdpProc[i].inode_id[2]=p->low_ino;
				else if(rtEdpProc[i].inode_id[3]==0) rtEdpProc[i].inode_id[3]=p->low_ino;
			}			
		}


		
		
	}

	
}



