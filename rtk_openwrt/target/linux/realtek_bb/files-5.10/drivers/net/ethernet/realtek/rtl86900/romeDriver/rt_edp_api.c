#include <linux/kmod.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/if_ether.h>
#include <rt_edp_debug.h>
#include <rt_edp_internal.h>
#include <rt_edp_struct.h>

// rtk include
#include <rtk/mirror.h>

#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <linux/netfilter/nf_conntrack_ftp.h>
#include <linux/netfilter/nf_conntrack_sip.h>

typedef enum rt_edp_alg_type_idx_e
{
	RT_EDP_ALG_TYPE_SIP_UDP_IDX,
	RT_EDP_ALG_TYPE_SIP_TCP_IDX,
	RT_EDP_ALG_TYPE_FTP_TCP_IDX,
	RT_EDP_ALG_TYPE_MAX_IDX
}rt_edp_alg_type_idx_t;

static struct nf_conntrack_helper *rt_edp_helper[RT_EDP_ALG_TYPE_MAX_IDX];

int rt_edp_api_module_init(void)
{
	memset(&rt_edp_db,0,sizeof(rt_edp_globalDatabase_t));

	return 0;
}

rtk_rg_err_code_t rt_edp_driverVersion_get(rtk_rg_VersionString_t *version_string)
{
	return 0;
}

rtk_rg_err_code_t rt_edp_initParam_get(rtk_rg_initParams_t *init_param)
{
	return 0;
}


rtk_rg_err_code_t rt_edp_initParam_set(rtk_rg_initParams_t *init_param)
{
	int i, wanIntfMark;
	struct nf_conntrack_helper *helper = NULL;
	unsigned char cmd_buf[16];

	//Clean all rg_db variables!!
	memset(&rt_edp_db, 0, sizeof(rt_edp_globalDatabase_t));

	//init dev port mapping
	for(i = 0; i < RT_EDP_MAC_PORT_MAX; i++) {
		rt_edp_db.dev_port_map[i].port_idx = i;
		memset(cmd_buf, 0, sizeof(cmd_buf));
		if(i == RT_EDP_MAC_PORT_PON)
			snprintf(cmd_buf, sizeof(cmd_buf), "nas0");
		else if(i < RT_EDP_MAC_PORT_PON)
			snprintf(cmd_buf, sizeof(cmd_buf), "eth0.%d", 2+i);
		else
			snprintf(cmd_buf, sizeof(cmd_buf), "eth0");
		memcpy(rt_edp_db.dev_port_map[i].dev_name,cmd_buf,sizeof(rt_edp_db.dev_port_map[i].dev_name));
	}

	//down all actived netdev
	_rt_edp_pipe_cmd("	ethdev=$(ifconfig | grep Ethernet | awk '{ print $1 }')\nfor dev in ${ethdev}\n	do\n	  ifconfig $dev down\n	done\n");


	//remove all netdev from br0
	_rt_edp_pipe_cmd("	ethdev=$(ifconfig -a | grep Ethernet | awk '{ print $1 }')\nfor dev in ${ethdev}\n	do\n	  brctl delif br0 $dev\n	done\n");


	//remove all brX netdev
	_rt_edp_pipe_cmd("	ethdev=$(ifconfig -a | grep Ethernet |  awk '{ print $1 }' | grep br)\nfor dev in ${ethdev}\n	do\n	  brctl delbr $dev\n	done\n");


	//reset iptables/ip6tables/ebtables
	for(i = 0; i <= 2; i++) {
		memset(cmd_buf, 0, sizeof(cmd_buf));
		if(i == 0) {	//iptables
			snprintf(cmd_buf, sizeof(cmd_buf), "%s", EDP_IPTABLES_UTILITY);
		}else if(i == 1) {	//ip6tables
			snprintf(cmd_buf, sizeof(cmd_buf), "%s", EDP_IP6TABLES_UTILITY);
		}else {	//ebtables
			snprintf(cmd_buf, sizeof(cmd_buf), "%s", EDP_EBTABLES_UTILITY);
		}

		//filter table
		_rt_edp_pipe_cmd("%s -F", cmd_buf);
		_rt_edp_pipe_cmd("%s -X", cmd_buf);
		_rt_edp_pipe_cmd("%s -P INPUT ACCEPT", cmd_buf);
		_rt_edp_pipe_cmd("%s -P OUTPUT ACCEPT", cmd_buf);
		_rt_edp_pipe_cmd("%s -P FORWARD ACCEPT", cmd_buf);

		//nat table
		_rt_edp_pipe_cmd("%s -t nat -F", cmd_buf);
		_rt_edp_pipe_cmd("%s -t nat -X", cmd_buf);
		_rt_edp_pipe_cmd("%s -t nat -P PREROUTING ACCEPT", cmd_buf);
		if(i <= 1) _rt_edp_pipe_cmd("%s -t nat -P INPUT ACCEPT", cmd_buf);
		_rt_edp_pipe_cmd("%s -t nat -P OUTPUT ACCEPT", cmd_buf);
		_rt_edp_pipe_cmd("%s -t nat -P POSTROUTING ACCEPT", cmd_buf);

		if(i <= 1) {
			//mangle table
			_rt_edp_pipe_cmd("%s -t mangle -F", cmd_buf);
			_rt_edp_pipe_cmd("%s -t mangle -X", cmd_buf);
			_rt_edp_pipe_cmd("%s -t mangle -P PREROUTING ACCEPT", cmd_buf);
			_rt_edp_pipe_cmd("%s -t mangle -P INPUT ACCEPT", cmd_buf);
			_rt_edp_pipe_cmd("%s -t mangle -P FORWARD ACCEPT", cmd_buf);
			_rt_edp_pipe_cmd("%s -t mangle -P OUTPUT ACCEPT", cmd_buf);
			_rt_edp_pipe_cmd("%s -t mangle -P POSTROUTING ACCEPT", cmd_buf);
		}else if(i == 2) {
			//broute table
			_rt_edp_pipe_cmd("%s -t broute -F", cmd_buf);
			_rt_edp_pipe_cmd("%s -t broute -X", cmd_buf);
			_rt_edp_pipe_cmd("%s -t broute -P BROUTING ACCEPT", cmd_buf);
		}
	}

	/* ++++++ Reference: _rtl9607c_fc_lut_init() Start ++++++ */
	_rt_edp_pipe_cmd("diag l2 set lookup-miss port all broadcast action follow-fb");

	// set lookup miss only  to CPU
	_rt_edp_pipe_cmd("diag l2 set lookup-miss unicast flood-ports 9");
	_rt_edp_pipe_cmd("diag l2 set lookup-miss broadcast flood-ports 9");
	_rt_edp_pipe_cmd("diag l2 set lookup-miss multicast flood-ports 9");
	_rt_edp_pipe_cmd("diag l2 set lookup-miss port all unicast action trap-to-cpu");

	//Turn off Lut auto-learning, set unknown SA and port-move trap
	for(i=0;i<RT_EDP_MAC_PORT_MAX;i++)		//CPU port will use auto-learning, and do not turn on DMAC2CViD function
	{
		if(RT_EDP_ALL_MAC_CPU_PORTMASK & (0x1<<i)) {
			//Turn off CPU port LUT auto-learning, and set Action to Forward
#if !defined(CONFIG_RTK_L34_G3_PLATFORM)
			_rt_edp_pipe_cmd("diag l2 set limit-learning port %d count 0", i);
			_rt_edp_pipe_cmd("diag l2 set limit-learning port %d action forward", i);
#endif
			_rt_edp_pipe_cmd("diag l2 set port-move port %d action trap-to-cpu", i);
		}else {
			_rt_edp_pipe_cmd("diag l2 set limit-learning port %d count 0", i);
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
			// G3 use L3FE CLS as ACL rule, so we expect no L2 trap happens before ACL.
			// in such design, adding hw l2 entry is no necessary. that means no unknown sa, no unmatch vlan, no port moving.
			_rt_edp_pipe_cmd("diag l2 set limit-learning port %d action forward", i);
			_rt_edp_pipe_cmd("diag l2 set port-move port %d action forward", i);
#else
			_rt_edp_pipe_cmd("diag l2 set limit-learning port %d action trap-to-cpu", i);
			_rt_edp_pipe_cmd("diag l2 set port-move port %d action trap-to-cpu", i);
#endif
		}

		// disable lut aging
		_rt_edp_pipe_cmd("diag l2 set aging-out port %d state disable", i);

		_rt_edp_pipe_cmd("diag l2 set lookup-miss port %d broadcast action trap-to-cpu", i);
		_rt_edp_pipe_cmd("diag l2 set lookup-miss port %d unicast action trap-to-cpu", i);
	}
	/* ++++++ Reference: _rtl9607c_fc_lut_init() End ++++++ */

	// disable hw vlan filter
	_rt_edp_pipe_cmd("diag vlan set ingress-filter port all state disable");
	_rt_edp_pipe_cmd("diag vlan set state disable");

	//Init ACL
	ASSERT_EQ(_rt_edp_acl_linux_init(),RT_EDP_ERR_OK);

	//Init ALG helper
	helper = nf_conntrack_helper_try_module_get("sip",AF_INET,IPPROTO_TCP);
	if(helper)
	{
		rt_edp_db.algFunctionMask |= RTK_RG_ALG_SIP_TCP_BIT;
		if(rt_edp_helper[RT_EDP_ALG_TYPE_SIP_TCP_IDX] == NULL)
		{
			rt_edp_helper[RT_EDP_ALG_TYPE_SIP_TCP_IDX] = kmalloc(sizeof(struct nf_conntrack_helper),GFP_KERNEL);
			memcpy(rt_edp_helper[RT_EDP_ALG_TYPE_SIP_TCP_IDX],helper,sizeof(struct nf_conntrack_helper));
		}
	}
	helper = nf_conntrack_helper_try_module_get("sip",AF_INET,IPPROTO_UDP);
	if(helper)
	{
		rt_edp_db.algFunctionMask |= RTK_RG_ALG_SIP_UDP_BIT;
		if(rt_edp_helper[RT_EDP_ALG_TYPE_SIP_UDP_IDX] == NULL)
		{
			rt_edp_helper[RT_EDP_ALG_TYPE_SIP_UDP_IDX] = kmalloc(sizeof(struct nf_conntrack_helper),GFP_KERNEL);
			memcpy(rt_edp_helper[RT_EDP_ALG_TYPE_SIP_UDP_IDX],helper,sizeof(struct nf_conntrack_helper));
		}
	}
	helper = nf_conntrack_helper_try_module_get("ftp",PF_INET,IPPROTO_TCP);
	if(helper)
	{
		rt_edp_db.algFunctionMask |= RTK_RG_ALG_FTP_TCP_BIT;
		if(rt_edp_helper[RT_EDP_ALG_TYPE_FTP_TCP_IDX] == NULL)
		{
			rt_edp_helper[RT_EDP_ALG_TYPE_FTP_TCP_IDX] = kmalloc(sizeof(struct nf_conntrack_helper),GFP_KERNEL);
			memcpy(rt_edp_helper[RT_EDP_ALG_TYPE_FTP_TCP_IDX],helper,sizeof(struct nf_conntrack_helper));
		}
	}

	// init wan interface mark value
	for(i=0, wanIntfMark=0x4000; i<RT_EDP_WAN_INTF_MARK_SIZE; i++, wanIntfMark+=0x4000)
	{
		sprintf(rt_edp_db.wan_intf_mark[i], "0x%x/0x7C000", wanIntfMark);
	}
	// For policy route/binding
	for(i=0; i<EDP_MAX_NETIF_SW_TABLE_SIZE; i++)
	{
		_rt_edp_pipe_cmd("%s rule add fwmark %s table %d", EDP_IP_UTILITY, rt_edp_db.wan_intf_mark[i], RT_EDP_POLICY_ROUTE_TABLE_START+i);
		_rt_edp_pipe_cmd("%s -6 rule add fwmark %s table %d", EDP_IP_UTILITY, rt_edp_db.wan_intf_mark[i], RT_EDP_POLICY_ROUTE_TABLE_START+i);
	}

	return 0;
}

rtk_rg_err_code_t rt_edp_lanInterface_add(rtk_rg_lanIntfConf_t *lan_info,int *intf_idx)
{
	int i, taggingCPU;
	int intfIdx=FAIL, ipv4Enable=0, ipv6Enable=0;
	unsigned char ip[16];
	unsigned char mask[16];

	//==========================Check input parameters==========================
	if(lan_info == NULL || intf_idx == NULL)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_NULL_POINTER);
	if(rt_edp_db.systemGlobal.initParam.macBasedTagDecision && lan_info->isIVL)		//IVL can not be set when DMAC2CVID is trun on
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);
	if(lan_info->intf_vlan_id<0 || lan_info->intf_vlan_id>=EDP_MAX_VLAN_SW_TABLE_SIZE)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);
	taggingCPU=(lan_info->untag_mask.portmask&RT_EDP_ALL_MAC_MASTER_CPU_PORTMASK)?0:1;
	if(taggingCPU && lan_info->intf_vlan_id>=EDP_MAX_VLAN_HW_TABLE_SIZE)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);
	if(lan_info->ip_version<0 || lan_info->ip_version>=(rtk_rg_ip_version_t)RT_EDP_IPVER_END)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);
	if((lan_info->ip_version==(rtk_rg_ip_version_t)RT_EDP_IPVER_V4ONLY || lan_info->ip_version==(rtk_rg_ip_version_t)RT_EDP_IPVER_V4V6) && lan_info->ip_network_mask>0)
		ipv4Enable=1;
	if((lan_info->ip_version==(rtk_rg_ip_version_t)RT_EDP_IPVER_V6ONLY || lan_info->ip_version==(rtk_rg_ip_version_t)RT_EDP_IPVER_V4V6) && lan_info->ipv6_network_mask_length>0)
		ipv6Enable=1;
	if(ipv4Enable==1 && (lan_info->ip_addr == 0 || lan_info->ip_network_mask == 0))
		ipv4Enable=0;
	if(ipv6Enable==1)
	{
		if(lan_info->ipv6_network_mask_length==0)
			ipv6Enable=0;
		else if(lan_info->ipv6_network_mask_length>128)	//interface route should not bigger than 128bit
			RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);
		else
		{
			//Check for valid IPv6 address
			if(*(unsigned int *)lan_info->ipv6_addr.ipv6_addr == 0 &&
				*(unsigned int *)(lan_info->ipv6_addr.ipv6_addr+4) == 0 &&
				*(unsigned int *)(lan_info->ipv6_addr.ipv6_addr+8) == 0 &&
				*(unsigned int *)(lan_info->ipv6_addr.ipv6_addr+12) == 0)
					ipv6Enable=0;
		}
	}
	if((ipv6Enable || ipv4Enable) && lan_info->gmac.octet[0] == 0 && lan_info->gmac.octet[1] == 0 && lan_info->gmac.octet[2] == 0 &&
		lan_info->gmac.octet[3] == 0 && lan_info->gmac.octet[4] == 0 && lan_info->gmac.octet[5] == 0)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);
	if(lan_info->gmac.octet[0]&1)	//interface MAC can not use multicast address
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);
	if(lan_info->mtu==0)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);
	if(lan_info->port_mask.portmask == 0 ||	RT_EDP_INVALID_MAC_PORTMASK(lan_info->untag_mask.portmask)) //untag set didn't contain extension port
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);
	if(RT_EDP_INVALID_PORTMASK(lan_info->port_mask.portmask))
		WARNING("LAN port_mask(%x) is not valid");
	if((rt_edp_db.vlan[rt_edp_db.systemGlobal.initParam.fwdVLAN_CPU].valid && lan_info->intf_vlan_id == rt_edp_db.systemGlobal.initParam.fwdVLAN_CPU) ||
		(rt_edp_db.vlan[rt_edp_db.systemGlobal.initParam.fwdVLAN_CPU_SVLAN].valid && lan_info->intf_vlan_id == rt_edp_db.systemGlobal.initParam.fwdVLAN_CPU_SVLAN) ||
		(rt_edp_db.vlan[rt_edp_db.systemGlobal.initParam.fwdVLAN_Proto_Block].valid && lan_info->intf_vlan_id == rt_edp_db.systemGlobal.initParam.fwdVLAN_Proto_Block) ||
		(rt_edp_db.systemGlobal.initParam.macBasedTagDecision==1 && (lan_info->intf_vlan_id == rt_edp_db.systemGlobal.initParam.fwdVLAN_BIND_INTERNET ||
		(lan_info->intf_vlan_id >= rt_edp_db.systemGlobal.initParam.fwdVLAN_BIND_OTHER &&
		lan_info->intf_vlan_id <= rt_edp_db.systemGlobal.initParam.fwdVLAN_BIND_OTHER+EDP_DEFAULT_BIND_LAN_OFFSET))))
		RT_EDP_RETURN_ERR(RT_EDP_ERR_VLAN_USED_BY_SYSTEM);

	if(lan_info->vlan_based_pri_enable==(rtk_rg_enable_t)RT_EDP_DISABLED)
		lan_info->vlan_based_pri=-1;
	else if(lan_info->vlan_based_pri<0 || lan_info->vlan_based_pri>7)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);

	//If user didn't add CPU port, we set it to untagged
	if((lan_info->port_mask.portmask&RT_EDP_ALL_MASTER_CPU_PORTMASK) != RT_EDP_ALL_MASTER_CPU_PORTMASK)
	{
		lan_info->port_mask.portmask |= RT_EDP_ALL_MASTER_CPU_PORTMASK;
		lan_info->untag_mask.portmask |= RT_EDP_ALL_MAC_MASTER_CPU_PORTMASK;
		WARNING("Forcibly enable lan cpu port and set it to untagged, port_mask=0x%x, untag_mask=0x%x", lan_info->port_mask.portmask, lan_info->untag_mask.portmask);
	}
	//Check VLAN-binding use this VLAN or not
	for(i=0;i<MAX_BIND_SW_TABLE_SIZE;i++)
		if(rt_edp_db.bind[i].valid && rt_edp_db.bind[i].rtk_bind.vidLan==lan_info->intf_vlan_id)
			RT_EDP_RETURN_ERR(RT_EDP_ERR_VLAN_USED_BY_VLANBINDING);
	//Check Customer VLAN use this VLAN or not
	if(rt_edp_db.vlan[lan_info->intf_vlan_id].valid && rt_edp_db.vlan[lan_info->intf_vlan_id].addedAsCustomerVLAN)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_VLAN_USED_BY_CVLAN);

	for(i=0; i<rt_edp_db.systemGlobal.lanIntfTotalNum; i++)
	{
		//Check if there is a LAN interface has the same information but IP setting with the adding intf
		if((rtk_rg_ip_version_t)rt_edp_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->ip_version==lan_info->ip_version &&
			!memcmp(rt_edp_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet,lan_info->gmac.octet,ETHER_ADDR_LEN) &&
			rt_edp_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->port_mask.portmask==lan_info->port_mask.portmask &&
			rt_edp_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->untag_mask.portmask==lan_info->untag_mask.portmask &&
			rt_edp_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->intf_vlan_id==lan_info->intf_vlan_id &&
			(rtk_rg_enable_t)rt_edp_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->vlan_based_pri_enable==lan_info->vlan_based_pri_enable &&
			rt_edp_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->vlan_based_pri==lan_info->vlan_based_pri &&
			rt_edp_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->isIVL==lan_info->isIVL)
		{
			intfIdx=rt_edp_db.systemGlobal.lanIntfGroup[i].index;
			break;
		}
	}
	//Check interface table available or not
	if(intfIdx==FAIL)
	{
		for(i=0; i<MAX_NETIF_SW_TABLE_SIZE; i++)
		{
			if(rt_edp_db.systemGlobal.interfaceInfo[i].valid == RT_EDP_IF_INVALID_ENTRY)
				break;
		}
		if(i==MAX_NETIF_SW_TABLE_SIZE)
			RT_EDP_RETURN_ERR(RT_EDP_ERR_ENTRY_FULL);

		intfIdx=i;		// keep
	}

	//==========================Setup linux commands==========================
	//add lan interface
	memcpy(ip, (void*)_rt_edp_inet_ntoa(lan_info->ip_addr),16);
	memcpy(mask, (void*)_rt_edp_inet_ntoa(lan_info->ip_network_mask),16);

	_rt_edp_pipe_cmd("brctl addbr %s", EDP_BR_DEVICE_NAME);
	_rt_edp_pipe_cmd("%s link set %s down", EDP_IP_UTILITY, EDP_BR_DEVICE_NAME);
	_rt_edp_pipe_cmd("%s link set %s down", EDP_IP_UTILITY, EDP_LAN_DEVICE_NAME);

#if defined(CONFIG_RTL_MULTI_LAN_DEV)
	for(i=0;i<RT_EDP_PORT_PON;i++)
	{
		if(RT_EDP_INVALID_PORT(i)) continue;
		if(lan_info->port_mask.portmask&(0x1<<i))
		{
			_rt_edp_pipe_cmd("%s link set %s.%d down", EDP_IP_UTILITY, EDP_LAN_DEVICE_NAME, i+EDP_LAN_DEVICE_START_INDEX);
			_rt_edp_pipe_cmd("brctl addif %s %s.%d", EDP_BR_DEVICE_NAME, EDP_LAN_DEVICE_NAME, i+EDP_LAN_DEVICE_START_INDEX);
		}
	}
#endif

	_rt_edp_pipe_cmd("brctl addif %s wlan0", EDP_BR_DEVICE_NAME);
#ifdef CONFIG_DUALBAND_CONCURRENT
	_rt_edp_pipe_cmd("brctl addif %s wlan1", EDP_BR_DEVICE_NAME);
	_rt_edp_pipe_cmd("echo 3 > /proc/vwlan");	//echo 3 means down
#elif defined(CONFIG_RG_WLAN_HWNAT_ACCELERATION)
	_rt_edp_pipe_cmd("brctl addif %s wlan1", EDP_BR_DEVICE_NAME);
#endif

	//1 FIXME:br0.vid should match br0's ether address, but it's not definitely required
	if(!taggingCPU)
	{
		_rt_edp_pipe_cmd("%s link set %s address %02x:%02x:%02x:%02x:%02x:%02x", EDP_IP_UTILITY, EDP_BR_DEVICE_NAME,
				lan_info->gmac.octet[0], lan_info->gmac.octet[1],
				lan_info->gmac.octet[2], lan_info->gmac.octet[3],
				lan_info->gmac.octet[4], lan_info->gmac.octet[5]);
	}

#if defined(CONFIG_RTL_MULTI_LAN_DEV)
	if(lan_info->port_mask.portmask&RT_EDP_ALL_MASTER_EXT_PORTMASK
#ifdef CONFIG_DUALBAND_CONCURRENT
		|| lan_info->port_mask.portmask&RT_EDP_ALL_SLAVE_EXT_PORTMASK
#endif
		)	//sync eth0 mac with wlan mac when multiple LAN is on.
#endif
	{
		_rt_edp_pipe_cmd("%s link set %s address %02x:%02x:%02x:%02x:%02x:%02x",
					EDP_IP_UTILITY, EDP_LAN_DEVICE_NAME,
				   	lan_info->gmac.octet[0],
				   	lan_info->gmac.octet[1],
				   	lan_info->gmac.octet[2],
				  	lan_info->gmac.octet[3],
				   	lan_info->gmac.octet[4],
				   	lan_info->gmac.octet[5]);
	}
	_rt_edp_pipe_cmd("%s link set %s up", EDP_IP_UTILITY, EDP_LAN_DEVICE_NAME);


	for(i=0;i<RT_EDP_PORT_PON;i++)
	{
		if(RT_EDP_INVALID_PORT(i)) continue;
		if(lan_info->port_mask.portmask&(0x1<<i))
		{
			_rt_edp_pipe_cmd("%s link set %s.%d address %02x:%02x:%02x:%02x:%02x:%02x",
					EDP_IP_UTILITY, EDP_LAN_DEVICE_NAME, i+EDP_LAN_DEVICE_START_INDEX,
				   	lan_info->gmac.octet[0],
				   	lan_info->gmac.octet[1],
				   	lan_info->gmac.octet[2],
				   	lan_info->gmac.octet[3],
				   	lan_info->gmac.octet[4],
				   	lan_info->gmac.octet[5]);
			_rt_edp_pipe_cmd("%s link set %s.%d up", EDP_IP_UTILITY, EDP_LAN_DEVICE_NAME, i+EDP_LAN_DEVICE_START_INDEX);
		}
	}

	// to remove the bridge data path to unknown port. (tx devportmapping eth0->0x0)
	_rt_edp_pipe_cmd("brctl delif %s %s", EDP_BR_DEVICE_NAME, EDP_LAN_DEVICE_NAME);

	_rt_edp_pipe_cmd("%s link set %s up", EDP_IP_UTILITY, EDP_BR_DEVICE_NAME);


	//=================================Add virtual ether device for VLAN==============================================
	if(taggingCPU)
		_rt_edp_pipe_cmd("vconfig add %s %d", EDP_BR_DEVICE_NAME, lan_info->intf_vlan_id);

	/***IPv4 Setting***/
	if(lan_info->ip_version==(rtk_rg_ip_version_t)RT_EDP_IPVER_V4ONLY || lan_info->ip_version==(rtk_rg_ip_version_t)RT_EDP_IPVER_V4V6)
	{
		if(taggingCPU)
		{
			//for change IP at same interface
			if(lan_info->replace_subnet)
			{
				//when replace, the "more subnet" added before will be flushed also
				_rt_edp_pipe_cmd("%s -4 addr flush dev %s.%d", EDP_IP_UTILITY, EDP_BR_DEVICE_NAME, lan_info->intf_vlan_id);
				if(lan_info->ip_version==(rtk_rg_ip_version_t)RT_EDP_IPVER_V4ONLY)
				{
					_rt_edp_pipe_cmd("%s -6 addr flush dev %s.%d", EDP_IP_UTILITY, EDP_BR_DEVICE_NAME, lan_info->intf_vlan_id);
					_rt_edp_pipe_cmd("%s -6 route flush dev %s.%d", EDP_IP_UTILITY, EDP_BR_DEVICE_NAME, lan_info->intf_vlan_id);
				}
			}

			//CPU tagged, set br0.vid
			//remove br0's ipv4 address, may be set by boa or other app.
			_rt_edp_pipe_cmd("%s link set %s.%d mtu %d", EDP_IP_UTILITY, EDP_BR_DEVICE_NAME, lan_info->intf_vlan_id, lan_info->mtu);
			if(lan_info->ip_network_mask != 0)
			{
				_rt_edp_pipe_cmd("%s addr add %s/%s broadcast + dev %s.%d", EDP_IP_UTILITY, ip, mask, EDP_BR_DEVICE_NAME, lan_info->intf_vlan_id);
			}
			//Setup dhcpd for the adding interface
			//_rg_setup_dhcpd(lan_info->ip_addr, lan_info->ip_network_mask, lan_info->intf_vlan_id);
		}
		else
		{
			//Before add address, forced delete once!(for change IP at same interface)
			if(lan_info->replace_subnet)
			{
				//when replace, the "more subnet" added before will be flushed also
				_rt_edp_pipe_cmd("%s -4 addr flush dev %s", EDP_IP_UTILITY, EDP_BR_DEVICE_NAME);
				if(lan_info->ip_version==(rtk_rg_ip_version_t)RT_EDP_IPVER_V4ONLY)
				{
					_rt_edp_pipe_cmd("%s -6 addr flush dev %s", EDP_IP_UTILITY, EDP_BR_DEVICE_NAME);
					_rt_edp_pipe_cmd("%s -6 route flush dev %s", EDP_IP_UTILITY, EDP_BR_DEVICE_NAME);
				}
			}

			//CPU untag, set br0 directly
			_rt_edp_pipe_cmd("%s link set %s mtu %d", EDP_IP_UTILITY, EDP_BR_DEVICE_NAME, lan_info->mtu);
			if(lan_info->ip_network_mask != 0)
			{
				_rt_edp_pipe_cmd("%s addr add %s/%s broadcast + dev %s", EDP_IP_UTILITY, ip, mask, EDP_BR_DEVICE_NAME);
			}

			//Setup dhcpd for the adding interface
			//_rg_setup_dhcpd(lan_info->ip_addr, lan_info->ip_network_mask, -1);
		}
	}

	/***IPv6 Setting***/
	if(lan_info->ip_version==(rtk_rg_ip_version_t)RT_EDP_IPVER_V6ONLY || lan_info->ip_version==(rtk_rg_ip_version_t)RT_EDP_IPVER_V4V6)
	{
		if(lan_info->ipv6_network_mask_length != 0 &&
			!(*(unsigned int *)lan_info->ipv6_addr.ipv6_addr == 0 &&
			*(unsigned int *)(lan_info->ipv6_addr.ipv6_addr+4) == 0 &&
			*(unsigned int *)(lan_info->ipv6_addr.ipv6_addr+8) == 0 &&
			*(unsigned int *)(lan_info->ipv6_addr.ipv6_addr+12) == 0))
		{
			if(taggingCPU)
			{
				//Turn on IPv6 at /proc/sys/net
				_rt_edp_pipe_cmd("echo 0 > /proc/sys/net/ipv6/conf/%s.%d/disable_ipv6", EDP_BR_DEVICE_NAME, lan_info->intf_vlan_id);

				//Before add address, forced delete once!(for change IP at same interface)
				if(lan_info->replace_subnet)
				{
					//FIXME:since we don't support IPv6 more subnet right now, here we just flush all(one) IPv6 address.
					_rt_edp_pipe_cmd("%s -6 addr flush dev %s.%d", EDP_IP_UTILITY, EDP_BR_DEVICE_NAME, lan_info->intf_vlan_id);
					_rt_edp_pipe_cmd("%s -6 route flush dev %s.%d", EDP_IP_UTILITY, EDP_BR_DEVICE_NAME, lan_info->intf_vlan_id);
					if(lan_info->ip_version==(rtk_rg_ip_version_t)RT_EDP_IPVER_V6ONLY)
					{
						_rt_edp_pipe_cmd("%s -4 addr flush dev %s.%d", EDP_IP_UTILITY, EDP_BR_DEVICE_NAME, lan_info->intf_vlan_id);
						_rt_edp_pipe_cmd("%s -4 route flush dev %s.%d", EDP_IP_UTILITY, EDP_BR_DEVICE_NAME, lan_info->intf_vlan_id);
					}
				}

				//CPU tagged, set br0.vid
				_rt_edp_pipe_cmd("%s link set %s.%d mtu %d", EDP_IP_UTILITY, EDP_BR_DEVICE_NAME, lan_info->intf_vlan_id, lan_info->mtu);
				//remove br0's ipv6 address, may be set by boa or other app.
				_rt_edp_pipe_cmd("%s addr add %s/%d dev %s.%d", EDP_IP_UTILITY, _rt_edp_inet_n6toa(lan_info->ipv6_addr.ipv6_addr), lan_info->ipv6_network_mask_length, EDP_BR_DEVICE_NAME, lan_info->intf_vlan_id);

				//Setup radvd for the adding interface
				//_rg_setup_radvd(lan_info->ipv6_addr.ipv6_addr, lan_info->ipv6_network_mask_length, lan_info->intf_vlan_id);
			}
			else
			{
				//Turn on IPv6 at /proc/sys/net
				_rt_edp_pipe_cmd("echo 0 >/proc/sys/net/ipv6/conf/%s/disable_ipv6", EDP_BR_DEVICE_NAME);

				//Before add address, forced delete once!(for change IP at same interface)
				if(lan_info->replace_subnet)
				{
					//FIXME:since we don't support IPv6 more subnet right now, here we just flush all(one) IPv6 address.
					_rt_edp_pipe_cmd("%s -6 addr flush dev %s", EDP_IP_UTILITY, EDP_BR_DEVICE_NAME);
					_rt_edp_pipe_cmd("%s -6 route flush dev %s", EDP_IP_UTILITY, EDP_BR_DEVICE_NAME);
					if(lan_info->ip_version==(rtk_rg_ip_version_t)RT_EDP_IPVER_V6ONLY)
					{
						_rt_edp_pipe_cmd("%s -4 addr flush dev %s", EDP_IP_UTILITY, EDP_BR_DEVICE_NAME);
						_rt_edp_pipe_cmd("%s -4 route flush dev %s", EDP_IP_UTILITY, EDP_BR_DEVICE_NAME);
					}
				}

				//CPU untag, set br0 directly
				_rt_edp_pipe_cmd("%s link set %s mtu %d", EDP_IP_UTILITY, EDP_BR_DEVICE_NAME, lan_info->mtu);
				_rt_edp_pipe_cmd("%s addr add %s/%d dev %s", EDP_IP_UTILITY, _rt_edp_inet_n6toa(lan_info->ipv6_addr.ipv6_addr), lan_info->ipv6_network_mask_length, EDP_BR_DEVICE_NAME);

				//Setup radvd for the adding interface
				//_rg_setup_radvd(lan_info->ipv6_addr.ipv6_addr, lan_info->ipv6_network_mask_length, -1);
			}
		}
	}

	if(taggingCPU)
	{
		_rt_edp_pipe_cmd("%s link set %s.%d address %02x:%02x:%02x:%02x:%02x:%02x",
				EDP_IP_UTILITY, EDP_BR_DEVICE_NAME, lan_info->intf_vlan_id,
			   	lan_info->gmac.octet[0], lan_info->gmac.octet[1],
			   	lan_info->gmac.octet[2], lan_info->gmac.octet[3],
			   	lan_info->gmac.octet[4], lan_info->gmac.octet[5]);
		_rt_edp_pipe_cmd("%s link set %s.%d up", EDP_IP_UTILITY, EDP_BR_DEVICE_NAME, lan_info->intf_vlan_id);
	}

	//When change LAN's ip setting, the vlan-binding should be remove and readd for new IP address!!
	//Re-check all Vlan-binding rule for re-create vconfig
	for(i=0; i<EDP_MAX_BIND_SW_TABLE_SIZE; i++)
	{
		rt_edp_table_bind_t localBind;
		uint32 bindingPortIdx;

		//RGDB_LOCK
		memcpy(&localBind, &rt_edp_db.bind[i], sizeof(rt_edp_table_bind_t));
		//RGDB_UNLOCK
		if(localBind.valid && localBind.rtk_bind.vidLan!=0)
		{
			if(localBind.rtk_bind.portMask.bits[0]>0)
			{
				if(localBind.rtk_bind.portMask.bits[0] & lan_info->port_mask.portmask)
				{
					bindingPortIdx = __ffs(localBind.rtk_bind.portMask.bits[0]);
					if(((0x1<<bindingPortIdx) & RT_EDP_ALL_MAC_PORTMASK_WITHOUT_CPU)==0x0)
					{
						WARNING("vlan-binding[%d] portMask wrong...%x", i, localBind.rtk_bind.portMask.bits[0]);
						continue;
					}
					//delete it first, readd later
#if defined(CONFIG_RTL_MULTI_LAN_DEV)
					_rt_edp_pipe_cmd("vconfig rem %s.%d.%d", EDP_LAN_DEVICE_NAME, EDP_LAN_DEVICE_START_INDEX+bindingPortIdx, localBind.rtk_bind.vidLan);
					_rt_edp_pipe_cmd("ebtables -t broute -D BROUTING -i %s.%d -p 802_1Q -j DROP", EDP_LAN_DEVICE_NAME, EDP_LAN_DEVICE_START_INDEX+bindingPortIdx);
#else
					_rt_edp_pipe_cmd("vconfig rem %s.%d", EDP_LAN_DEVICE_NAME, localBind.rtk_bind.vidLan);
					_rt_edp_pipe_cmd("ebtables -t broute -D BROUTING -i %s -p 802_1Q -j DROP", EDP_LAN_DEVICE_NAME);
#endif
					_rt_edp_checkVlanBindingAndAddVconfig(bindingPortIdx, localBind.rtk_bind.vidLan);
				}
			}
			else
			{
				if(localBind.rtk_bind.extPortMask.bits[0] & (lan_info->port_mask.portmask>>RT_EDP_EXT_PORT0))
				{
					bindingPortIdx = __ffs(localBind.rtk_bind.extPortMask.bits[0]);
					if(((0x1<<bindingPortIdx) & ((0x1<<(RT_EDP_EXT_PORT0-RT_EDP_EXT_PORT0))|(0x1<<(RT_EDP_EXT_PORT1-RT_EDP_EXT_PORT0))))==0x0)
					{
						WARNING("vlan-binding[%d] ext-portMask wrong...%x",i,localBind.rtk_bind.extPortMask.bits[0]);
						continue;
					}
					//delete it first, readd later
					_rt_edp_pipe_cmd("vconfig rem %s.%d", EDP_LAN_DEVICE_NAME, localBind.rtk_bind.vidLan);
					_rt_edp_pipe_cmd("ebtables -t broute -D BROUTING -i %s -p 802_1Q -j DROP", EDP_LAN_DEVICE_NAME);

					_rt_edp_checkVlanBindingAndAddVconfig(RT_EDP_EXT_PORT0+bindingPortIdx, localBind.rtk_bind.vidLan);
				}
			}
		}
	}

#if defined(CONFIG_RG_G3_SERIES)
#if defined(CONFIG_RTL_MULTI_LAN_DEV)
for(i=0;i<RT_EDP_MAC_PORT_PON;i++)
{
	if(RT_EDP_INVALID_MAC_PORT(i)) continue;
	//reset all port-mapping to multi-LAN dev
	_rt_edp_pipe_cmd("echo %d %s.%d > /proc/driver/cortina/ni/dev_port_mapping", i, EDP_LAN_DEVICE_NAME, i+EDP_LAN_DEVICE_START_INDEX);
}
#endif

#else
	//do cmd
	for(i=0;i<RT_EDP_MAC_PORT_MAX;i++){
		if(RT_EDP_INVALID_MAC_PORT(i)) continue;
		if(lan_info->port_mask.portmask & (1<<i)){
			_rt_edp_pipe_cmd("echo %d %s > /proc/rtl8686gmac/dev_port_mapping", i, EDP_LAN_DEVICE_NAME);
		}
	}
#if defined(CONFIG_RTL_MULTI_LAN_DEV)
	for(i=0;i<RT_EDP_MAC_PORT_PON;i++)
	{
		if(RT_EDP_INVALID_MAC_PORT(i)) continue;
		if(lan_info->port_mask.portmask & (1<<i))
			_rt_edp_pipe_cmd("echo %d %s.%d > /proc/rtl8686gmac/dev_port_mapping", i, EDP_LAN_DEVICE_NAME, i+EDP_LAN_DEVICE_START_INDEX);
	}
#endif
#endif	// end defined(CONFIG_RG_G3_SERIES)


#ifdef CONFIG_DUALBAND_CONCURRENT
	_rt_edp_pipe_cmd("echo 2 > /proc/vwlan");	//echo 2 means up
#endif
	//patch for iptables
	_rt_edp_pipe_cmd("iptables -A INPUT -s %s -d %s -j ACCEPT", ip, ip);


	//==========================Store software data==========================
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].valid = RT_EDP_IF_VALID_ENTRY;
	memset(rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.intf_name, 0, 32);
	sprintf(rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.intf_name, "LAN%d", intfIdx);
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.is_wan = 0;

	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.lan_intf.ip_version = lan_info->ip_version;
	memcpy(rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.lan_intf.gmac.octet, lan_info->gmac.octet, ETHER_ADDR_LEN);
	if(ipv4Enable==1)
	{
		rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.lan_intf.ip_addr = lan_info->ip_addr;
		rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.lan_intf.ip_network_mask = lan_info->ip_network_mask;
	}
	if(ipv6Enable==1)
	{
		memcpy(&rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.lan_intf.ipv6_addr, &lan_info->ipv6_addr, sizeof(rtk_ipv6_addr_t));
		rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.lan_intf.ipv6_network_mask_length = lan_info->ipv6_network_mask_length;
	}
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.lan_intf.port_mask.portmask = lan_info->port_mask.portmask;
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.lan_intf.untag_mask.portmask = lan_info->untag_mask.portmask;
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.lan_intf.intf_vlan_id = lan_info->intf_vlan_id;
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.lan_intf.vlan_based_pri_enable = lan_info->vlan_based_pri_enable;
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.lan_intf.vlan_based_pri = lan_info->vlan_based_pri;
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.lan_intf.mtu = lan_info->mtu;
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.lan_intf.isIVL = lan_info->isIVL;
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.lan_intf.replace_subnet = lan_info->replace_subnet;

	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].p_lanIntfConf = &rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.lan_intf;

	//store information in Global variable
	rt_edp_db.systemGlobal.lanIntfGroup[rt_edp_db.systemGlobal.lanIntfTotalNum].index = intfIdx;
	rt_edp_db.systemGlobal.lanIntfGroup[rt_edp_db.systemGlobal.lanIntfTotalNum].p_intfInfo = &rt_edp_db.systemGlobal.interfaceInfo[intfIdx];
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].lan_or_wan_index = rt_edp_db.systemGlobal.lanIntfTotalNum;
	rt_edp_db.systemGlobal.lanIntfTotalNum++;

	//update LAN port mask
	rt_edp_db.systemGlobal.lanPortMask.portmask |= lan_info->port_mask.portmask;

	*intf_idx = intfIdx;
	return 0;
}

rtk_rg_err_code_t rt_edp_wanInterface_add(rtk_rg_wanIntfConf_t *wanintf, int *wan_intf_idx)
{
	rt_edp_err_code_t errorno=RT_EDP_ERR_OK;
	int i, intfIdx=FAIL, devIdx=FAIL, tunnelDevIdx=FAIL;
	char devName[IFNAMSIZ]={0}, tunnelDevName[IFNAMSIZ]={0};
	rt_edp_wirelessWan_t wirelessWan=RT_EDP_WWAN_WIRED;

	//==========================Check input parameters==========================

	//Check if we are re-add same WAN interface
	if(!wanintf->forcedAddNewIntf)
	{
		for(i=0; i<rt_edp_db.systemGlobal.wanIntfTotalNum; i++)
		{
			if(wanintf->wan_type==(rtk_rg_wan_type_t)rt_edp_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->wan_type &&
				!memcmp(wanintf->gmac.octet, rt_edp_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->gmac.octet, ETHER_ADDR_LEN) &&
				wanintf->wan_port_idx==(rtk_rg_port_idx_t)rt_edp_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->wan_port_idx &&
				wanintf->egress_vlan_tag_on==rt_edp_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->egress_vlan_tag_on &&
				wanintf->egress_vlan_id==rt_edp_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->egress_vlan_id &&
				wanintf->vlan_based_pri_enable==(rtk_rg_enable_t)rt_edp_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->vlan_based_pri_enable &&
				wanintf->vlan_based_pri==rt_edp_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->vlan_based_pri &&
				wanintf->isIVL==rt_edp_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->isIVL &&
				wanintf->none_internet==rt_edp_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->none_internet)
				{
					intfIdx=rt_edp_db.systemGlobal.wanIntfGroup[i].index;	//keep
					break;
				}
		}
	}
	//Check interface table available or not
	if(intfIdx==FAIL)
	{
		for(i=0; i<MAX_NETIF_SW_TABLE_SIZE; i++)
		{
			if(rt_edp_db.systemGlobal.interfaceInfo[i].valid == RT_EDP_IF_INVALID_ENTRY)
				break;
		}
		if(i==MAX_NETIF_SW_TABLE_SIZE)
			RT_EDP_RETURN_ERR(RT_EDP_ERR_ENTRY_FULL);

		intfIdx=i;		// keep
	}

	if(!strlen(rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.devName))
	{
		if(wirelessWan==RT_EDP_WWAN_WLAN0_VXD)
			sprintf(devName, "wlan0-vxd");
		else if(wirelessWan==RT_EDP_WWAN_WLAN1_VXD)
			sprintf(devName, "wlan1-vxd");
		else
		{
			errorno = _rt_edp_freeWanDevIdx_get(RT_EDP_WAN_DEV_TYPE_NAS, &devIdx);
			if(errorno!=RT_EDP_ERR_OK)
				RT_EDP_RETURN_ERR(errorno);
			sprintf(devName, "%s_%d", EDP_WAN_DEVICE_NAME, devIdx);

			switch(wanintf->wan_type)
			{
				case RT_EDP_PPPoE:
				case RT_EDP_PPPoE_DSLITE:
				case RT_EDP_PPTP:
				case RT_EDP_L2TP:
					errorno = _rt_edp_freeWanDevIdx_get(RT_EDP_WAN_DEV_TYPE_PPP, &tunnelDevIdx);
					if(errorno!=RT_EDP_ERR_OK)
					{
						_rt_edp_freeWanDevIdx_put(RT_EDP_WAN_DEV_TYPE_NAS, devIdx);
						RT_EDP_RETURN_ERR(errorno);
					}
					sprintf(tunnelDevName, "%s%d", EDP_WAN_TUNNEL_DEVICE_NAME, tunnelDevIdx);
					break;
				default:
					break;
			}
		}
	}

	//==========================Setup linux commands==========================

#if defined(CONFIG_RG_G3_SERIES)
	_rt_edp_pipe_cmd("echo %d %s > /proc/driver/cortina/ni/dev_port_mapping", wanintf->wan_port_idx, EDP_WAN_DEVICE_NAME);
#else
	_rt_edp_pipe_cmd("echo %d %s > /proc/rtl8686gmac/dev_port_mapping", wanintf->wan_port_idx, EDP_WAN_DEVICE_NAME);
#endif

	_rt_edp_pipe_cmd("brctl delif %s %s", EDP_BR_DEVICE_NAME, devName);

	if(wirelessWan==RT_EDP_WWAN_WIRED)
	{
		_rt_edp_pipe_cmd("%s link set %s up", EDP_IP_UTILITY, EDP_WAN_DEVICE_NAME);

		_rt_edp_pipe_cmd("ethctl remsmux bridge %s %s", EDP_WAN_DEVICE_NAME, devName);

		if(wanintf->wan_type==RTK_RG_BRIDGE)
		{
			if(wanintf->egress_vlan_tag_on)
				_rt_edp_pipe_cmd("ethctl addsmux bridge %s %s napt brpppoe vlan %d", EDP_WAN_DEVICE_NAME, devName, wanintf->egress_vlan_id);
			else
				_rt_edp_pipe_cmd("ethctl addsmux bridge %s %s napt brpppoe", EDP_WAN_DEVICE_NAME, devName);
		}
		else	// not bridge wan
		{
			if(wanintf->egress_vlan_tag_on)
				_rt_edp_pipe_cmd("ethctl addsmux ipoe %s %s nonapt vlan %d", EDP_WAN_DEVICE_NAME, devName, wanintf->egress_vlan_id);
			else
				_rt_edp_pipe_cmd("ethctl addsmux ipoe %s %s", EDP_WAN_DEVICE_NAME, devName);
		}
	}
	_rt_edp_pipe_cmd("%s link set %s address %02x:%02x:%02x:%02x:%02x:%02x", EDP_IP_UTILITY, devName,
			   wanintf->gmac.octet[0],
			   wanintf->gmac.octet[1],
			   wanintf->gmac.octet[2],
			   wanintf->gmac.octet[3],
			   wanintf->gmac.octet[4],
			   wanintf->gmac.octet[5]);

	_rt_edp_pipe_cmd("%s link set %s txqueuelen 10", EDP_IP_UTILITY, devName);
	_rt_edp_pipe_cmd("%s link set %s up", EDP_IP_UTILITY, devName);

	//create nas0_X
	if(wanintf->wan_type==RTK_RG_BRIDGE)
	{
		_rt_edp_pipe_cmd("brctl addif %s %s", EDP_BR_DEVICE_NAME, devName);

		//If add bridge tagged WAN, we should add virtual ether device for it's VLAN
		if(wanintf->egress_vlan_tag_on)
		{
			_rt_edp_pipe_cmd("vconfig add %s %d", devName, wanintf->egress_vlan_id);
			_rt_edp_pipe_cmd("%s link set %s.%d up", EDP_IP_UTILITY, devName, wanintf->egress_vlan_id);
			_rt_edp_pipe_cmd("brctl addif %s %s.%d", EDP_BR_DEVICE_NAME, devName, wanintf->egress_vlan_id);
		}
	}
#ifdef CONFIG_GPON_FEATURE
	//Trigger OMCI to create related CF rules, parameters: netIfIdx, vid, pri, type, service, isBinding, bAdd
	if(rt_edp_db.systemGlobal.initParam.wanPortGponMode)
	{
		if(wirelessWan==RT_EDP_WWAN_WIRED)
		{
			if(wanintf->wan_type==RTK_RG_PPPoE)
			{
				if(wanintf->egress_vlan_tag_on)
					_rt_edp_pipe_cmd("echo %d %d %d 0 %d %d 1 > /proc/omci/wanInfo", *wan_intf_idx, wanintf->egress_vlan_id,
						wanintf->vlan_based_pri_enable?wanintf->vlan_based_pri:-1,
						wanintf->none_internet?0:1,
						(wanintf->vlan_binding_mask.portmask|wanintf->port_binding_mask.portmask)>0?1:0);
				else
					_rt_edp_pipe_cmd("echo %d -1 -1 0 %d %d 1 > /proc/omci/wanInfo", *wan_intf_idx,
						wanintf->none_internet?0:1,
						(wanintf->vlan_binding_mask.portmask|wanintf->port_binding_mask.portmask)>0?1:0);
			}else if(wanintf->wan_type==RTK_RG_BRIDGE)
			{
				if(wanintf->egress_vlan_tag_on)
					_rt_edp_pipe_cmd("echo %d %d %d 2 %d %d 1 > /proc/omci/wanInfo", *wan_intf_idx, wanintf->egress_vlan_id,
						wanintf->vlan_based_pri_enable?wanintf->vlan_based_pri:-1,
						wanintf->none_internet?0:1,
						(wanintf->vlan_binding_mask.portmask|wanintf->port_binding_mask.portmask)>0?1:0);
				else
					_rt_edp_pipe_cmd("echo %d -1 -1 2 %d %d 1 > /proc/omci/wanInfo",*wan_intf_idx,
						wanintf->none_internet?0:1,
						(wanintf->vlan_binding_mask.portmask|wanintf->port_binding_mask.portmask)>0?1:0);
			}else if(wanintf->egress_vlan_tag_on)
				_rt_edp_pipe_cmd("echo %d %d %d 1 %d %d  1 > /proc/omci/wanInfo", *wan_intf_idx, wanintf->egress_vlan_id,
					wanintf->vlan_based_pri_enable?wanintf->vlan_based_pri:-1,
					wanintf->none_internet?0:1,
					(wanintf->vlan_binding_mask.portmask|wanintf->port_binding_mask.portmask)>0?1:0);
			else
				_rt_edp_pipe_cmd("echo %d -1 -1 1 %d %d 1 > /proc/omci/wanInfo", *wan_intf_idx,
					wanintf->none_internet?0:1,
					(wanintf->vlan_binding_mask.portmask|wanintf->port_binding_mask.portmask)>0?1:0);
		}
	}
#endif

	//==========================Store software data==========================
	//Set Global variables
	rt_edp_db.systemGlobal.wanIntfGroup[rt_edp_db.systemGlobal.wanIntfTotalNum].index=intfIdx;
	//rt_edp_db.systemGlobal.wanIntfGroup[rt_edp_db.systemGlobal.wanIntfTotalNum].disableBroadcast=0;	//FIXME
	rt_edp_db.systemGlobal.wanIntfGroup[rt_edp_db.systemGlobal.wanIntfTotalNum].p_intfInfo=&rt_edp_db.systemGlobal.interfaceInfo[intfIdx];
	rt_edp_db.systemGlobal.wanIntfGroup[rt_edp_db.systemGlobal.wanIntfTotalNum].p_wanIntfConf=&rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.wan_intf_conf;
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].lan_or_wan_index=rt_edp_db.systemGlobal.wanIntfTotalNum;
	rt_edp_db.systemGlobal.wanIntfTotalNum++;		//add WAN interface number


	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].valid = RT_EDP_IF_VALID_ENTRY;
	memset(rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.intf_name, 0, 32);
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.is_wan = 1;
	//rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.bind_wan_type_ipv4=wantypeIdx;
	//rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.bind_wan_type_ipv6=v6wantypeIdx;
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.wirelessWan = wirelessWan;
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.devIdx = devIdx;
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.tunnelDevIdx = tunnelDevIdx;
	memcpy(rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.devName, devName, IFNAMSIZ);
	memcpy(rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.tunnelDevName, tunnelDevName, IFNAMSIZ);
	//rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.nexthop_ipv4=nxpIdx;
	//rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.nexthop_ipv6=v6nxpIdx;
	//rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.pppoe_idx=pppoeIdx;
	//rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.extip_idx=extipIdx;
	//rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.baseIntf_idx=baseIntfIdx;
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.flowEntryIdx = FAIL;
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.extraTagActionListIdx = 0;
#endif
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.wan_intf_conf.wan_type = wanintf->wan_type;
	memcpy(rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.wan_intf_conf.gmac.octet, wanintf->gmac.octet, ETHER_ADDR_LEN);
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.wan_intf_conf.wan_port_idx = wanintf->wan_port_idx;
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.wan_intf_conf.port_binding_mask.portmask = wanintf->port_binding_mask.portmask;
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.wan_intf_conf.egress_vlan_tag_on = wanintf->egress_vlan_tag_on;
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.wan_intf_conf.egress_vlan_id = wanintf->egress_vlan_id;
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.wan_intf_conf.vlan_based_pri_enable = wanintf->vlan_based_pri_enable;
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.wan_intf_conf.vlan_based_pri = wanintf->vlan_based_pri;
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.wan_intf_conf.isIVL = wanintf->isIVL;
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.wan_intf_conf.none_internet = wanintf->none_internet;
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.wan_intf_conf.wlan0_dev_binding_mask = wanintf->wlan0_dev_binding_mask;
#endif
	//This variable decides BC or MC L2 packets which hit binding to routing WAN should be forwarded or not.
	rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.wan_intf_conf.bridgeToBindingWanByProtocol = wanintf->bridgeToBindingWanByProtocol;

	if(wanintf->wan_type == RTK_RG_STATIC)
		rt_edp_db.systemGlobal.interfaceInfo[intfIdx].p_wanStaticInfo = &rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.static_info;		//short-cut of wan static info structure
	else if(wanintf->wan_type == RTK_RG_DHCP)
		rt_edp_db.systemGlobal.interfaceInfo[intfIdx].p_wanStaticInfo = &rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.dhcp_client_info.hw_info;		//short-cut of wan static info structure
	else if(wanintf->wan_type == RTK_RG_PPPoE)
		rt_edp_db.systemGlobal.interfaceInfo[intfIdx].p_wanStaticInfo = &rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.pppoe_info.after_dial.hw_info; 	//short-cut of wan static info structure
	else if(wanintf->wan_type == RTK_RG_PPTP)
		rt_edp_db.systemGlobal.interfaceInfo[intfIdx].p_wanStaticInfo = &rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.pptp_info.after_dial.hw_info;		//short-cut of wan static info structure
	else if(wanintf->wan_type == RTK_RG_L2TP)
		rt_edp_db.systemGlobal.interfaceInfo[intfIdx].p_wanStaticInfo = &rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.l2tp_info.after_dial.hw_info;		//short-cut of wan static info structure
	else if(wanintf->wan_type == RTK_RG_DSLITE)
		rt_edp_db.systemGlobal.interfaceInfo[intfIdx].p_wanStaticInfo = &rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.dslite_info.static_info;		//short-cut of wan static info structure
	else if(wanintf->wan_type == RTK_RG_PPPoE_DSLITE)
		rt_edp_db.systemGlobal.interfaceInfo[intfIdx].p_wanStaticInfo = &rt_edp_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.static_info;		//short-cut of wan static info structure
	else	//bridge WAN
		rt_edp_db.systemGlobal.interfaceInfo[intfIdx].p_wanStaticInfo = NULL;
	//Forcibly IP version to IPVER_V4V6 only for newborn interface.
	if(rt_edp_db.systemGlobal.interfaceInfo[intfIdx].p_wanStaticInfo && wanintf->forcedAddNewIntf)
		rt_edp_db.systemGlobal.interfaceInfo[intfIdx].p_wanStaticInfo->ip_version = RT_EDP_IPVER_V4V6;

	//FIXME: WAN port is not PON or fiber, WAN_MODE should be configured as HWNAT mode.
	for(i=0; i<EDP_MAX_NETIF_SW_TABLE_SIZE; i++)
	{
		if(rt_edp_db.systemGlobal.interfaceInfo[i].valid==RT_EDP_IF_INVALID_ENTRY)
			continue;
		if(rt_edp_db.systemGlobal.interfaceInfo[i].storedInfo.is_wan)
		{
			rt_edp_db.systemGlobal.wanPortMask.portmask |= (0x1<<rt_edp_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_port_idx);
			if(rt_edp_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_port_idx != RT_EDP_PORT_PON)
			{
				_rt_edp_pipe_cmd("echo 0 > /proc/rtk_smux/wan_mode");
			}
		}
	}

	*wan_intf_idx = intfIdx;
	return 0;
}

rtk_rg_err_code_t rt_edp_staticInfo_set(int wan_intf_idx, rtk_rg_ipStaticInfo_t *static_info)
{
	rt_edp_err_code_t errorno;
	rt_edp_ipStaticInfo_t ipStaticInfo;

	//1 To prevent the data structure of parameter of RG API is changed, use orginal RG data structure temporarily.
	ipStaticInfo.ip_version = static_info->ip_version;
	ipStaticInfo.napt_enable = static_info->napt_enable;
	ipStaticInfo.ipv6_napt_enable = static_info->ipv6_napt_enable;
	ipStaticInfo.ip_addr = static_info->ip_addr;
	ipStaticInfo.remote_host_ip_addr = static_info->remote_host_ip_addr;
	ipStaticInfo.ip_network_mask = static_info->ip_network_mask;
	ipStaticInfo.ipv4_default_gateway_on = static_info->ipv4_default_gateway_on;
	ipStaticInfo.gateway_ipv4_addr = static_info->gateway_ipv4_addr;
	memcpy(&ipStaticInfo.ipv6_addr, &static_info->ipv6_addr, sizeof(rtk_ipv6_addr_t));
	ipStaticInfo.ipv6_mask_length = static_info->ipv6_mask_length;
	ipStaticInfo.ipv6_default_gateway_on = static_info->ipv6_default_gateway_on;
	memcpy(&ipStaticInfo.gateway_ipv6_addr, &static_info->gateway_ipv6_addr, sizeof(rtk_ipv6_addr_t));
	ipStaticInfo.mtu = static_info->mtu;
	ipStaticInfo.gw_mac_auto_learn_for_ipv4 = static_info->gw_mac_auto_learn_for_ipv4;
	ipStaticInfo.gw_mac_auto_learn_for_ipv6 = static_info->gw_mac_auto_learn_for_ipv6;
	memcpy(&ipStaticInfo.gateway_mac_addr_for_ipv4, &static_info->gateway_mac_addr_for_ipv4, sizeof(rtk_mac_t));
	memcpy(&ipStaticInfo.gateway_mac_addr_for_ipv6, &static_info->gateway_mac_addr_for_ipv6, sizeof(rtk_mac_t));
	ipStaticInfo.static_route_with_arp = static_info->static_route_with_arp;

	errorno = _rt_edp_internal_wanSet(wan_intf_idx, &ipStaticInfo);

	RT_EDP_RETURN_ERR(errorno);
}

rtk_rg_err_code_t rt_edp_dsliteInfo_set(int wan_intf_idx, rtk_rg_ipDslitStaticInfo_t *dslite_info)
{
	return 0;
}

rtk_rg_err_code_t rt_edp_dhcpRequest_set(int wan_intf_idx)
{
	return 0;
}

rtk_rg_err_code_t rt_edp_dhcpClientInfo_set(int wan_intf_idx, rtk_rg_ipDhcpClientInfo_t *dhcpClient_info)
{
	rt_edp_err_code_t errorno=RT_EDP_ERR_OK;
	rt_edp_ipStaticInfo_t ipStaticInfo;

	//1 To prevent the data structure of parameter of RG API is changed, use orginal RG data structure temporarily.
	ipStaticInfo.ip_version = dhcpClient_info->hw_info.ip_version;
	ipStaticInfo.napt_enable = dhcpClient_info->hw_info.napt_enable;
	ipStaticInfo.ipv6_napt_enable = dhcpClient_info->hw_info.ipv6_napt_enable;
	ipStaticInfo.ip_addr = dhcpClient_info->hw_info.ip_addr;
	ipStaticInfo.remote_host_ip_addr = dhcpClient_info->hw_info.remote_host_ip_addr;
	ipStaticInfo.ip_network_mask = dhcpClient_info->hw_info.ip_network_mask;
	ipStaticInfo.ipv4_default_gateway_on = dhcpClient_info->hw_info.ipv4_default_gateway_on;
	ipStaticInfo.gateway_ipv4_addr = dhcpClient_info->hw_info.gateway_ipv4_addr;
	memcpy(&ipStaticInfo.ipv6_addr, &dhcpClient_info->hw_info.ipv6_addr, sizeof(rtk_ipv6_addr_t));
	ipStaticInfo.ipv6_mask_length = dhcpClient_info->hw_info.ipv6_mask_length;
	ipStaticInfo.ipv6_default_gateway_on = dhcpClient_info->hw_info.ipv6_default_gateway_on;
	memcpy(&ipStaticInfo.gateway_ipv6_addr, &dhcpClient_info->hw_info.gateway_ipv6_addr, sizeof(rtk_ipv6_addr_t));
	ipStaticInfo.mtu = dhcpClient_info->hw_info.mtu;
	ipStaticInfo.gw_mac_auto_learn_for_ipv4 = dhcpClient_info->hw_info.gw_mac_auto_learn_for_ipv4;
	ipStaticInfo.gw_mac_auto_learn_for_ipv6 = dhcpClient_info->hw_info.gw_mac_auto_learn_for_ipv6;
	memcpy(&ipStaticInfo.gateway_mac_addr_for_ipv4, &dhcpClient_info->hw_info.gateway_mac_addr_for_ipv4, sizeof(rtk_mac_t));
	memcpy(&ipStaticInfo.gateway_mac_addr_for_ipv6, &dhcpClient_info->hw_info.gateway_mac_addr_for_ipv6, sizeof(rtk_mac_t));
	ipStaticInfo.static_route_with_arp = dhcpClient_info->hw_info.static_route_with_arp;

	if(dhcpClient_info->stauts==DHCP_STATUS_LEASED)
	{
		errorno = _rt_edp_internal_wanSet(wan_intf_idx, &ipStaticInfo);
	}
	else	// dhcp release
	{
		errorno = _rt_edp_dhcp_release(wan_intf_idx, &ipStaticInfo);
	}

	if(errorno==RT_EDP_ERR_OK)
		rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf.dhcp_client_info.stauts = (rt_edp_dhcp_status_t)dhcpClient_info->stauts;

	RT_EDP_RETURN_ERR(errorno);
}

rtk_rg_err_code_t rt_edp_pppoeClientInfoBeforeDial_set(int wan_intf_idx, rtk_rg_pppoeClientInfoBeforeDial_t *app_info)
{
	return 0;
}

rtk_rg_err_code_t rt_edp_pppoeClientInfoAfterDial_set(int wan_intf_idx, rtk_rg_pppoeClientInfoAfterDial_t *clientPppoe_info)
{
	rt_edp_err_code_t errorno=RT_EDP_ERR_OK;
	rt_edp_ipStaticInfo_t ipStaticInfo;

	//1 To prevent the data structure of parameter of RG API is changed, use orginal RG data structure temporarily.
	ipStaticInfo.ip_version = clientPppoe_info->hw_info.ip_version;
	ipStaticInfo.napt_enable = clientPppoe_info->hw_info.napt_enable;
	ipStaticInfo.ipv6_napt_enable = clientPppoe_info->hw_info.ipv6_napt_enable;
	ipStaticInfo.ip_addr = clientPppoe_info->hw_info.ip_addr;
	ipStaticInfo.remote_host_ip_addr = clientPppoe_info->hw_info.remote_host_ip_addr;
	ipStaticInfo.ip_network_mask = clientPppoe_info->hw_info.ip_network_mask;
	ipStaticInfo.ipv4_default_gateway_on = clientPppoe_info->hw_info.ipv4_default_gateway_on;
	ipStaticInfo.gateway_ipv4_addr = clientPppoe_info->hw_info.gateway_ipv4_addr;
	memcpy(&ipStaticInfo.ipv6_addr, &clientPppoe_info->hw_info.ipv6_addr, sizeof(rtk_ipv6_addr_t));
	ipStaticInfo.ipv6_mask_length = clientPppoe_info->hw_info.ipv6_mask_length;
	ipStaticInfo.ipv6_default_gateway_on = clientPppoe_info->hw_info.ipv6_default_gateway_on;
	memcpy(&ipStaticInfo.gateway_ipv6_addr, &clientPppoe_info->hw_info.gateway_ipv6_addr, sizeof(rtk_ipv6_addr_t));
	ipStaticInfo.mtu = clientPppoe_info->hw_info.mtu;
	ipStaticInfo.gw_mac_auto_learn_for_ipv4 = clientPppoe_info->hw_info.gw_mac_auto_learn_for_ipv4;
	ipStaticInfo.gw_mac_auto_learn_for_ipv6 = clientPppoe_info->hw_info.gw_mac_auto_learn_for_ipv6;
	memcpy(&ipStaticInfo.gateway_mac_addr_for_ipv4, &clientPppoe_info->hw_info.gateway_mac_addr_for_ipv4, sizeof(rtk_mac_t));
	memcpy(&ipStaticInfo.gateway_mac_addr_for_ipv6, &clientPppoe_info->hw_info.gateway_mac_addr_for_ipv6, sizeof(rtk_mac_t));
	ipStaticInfo.static_route_with_arp = clientPppoe_info->hw_info.static_route_with_arp;

	errorno = _rt_edp_internal_wanSet(wan_intf_idx, &ipStaticInfo);
	if(errorno==RT_EDP_ERR_OK)
	{
		rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf.pppoe_info.after_dial.sessionId = clientPppoe_info->sessionId;
	}

	RT_EDP_RETURN_ERR(errorno);
}


rtk_rg_err_code_t rt_edp_pptpClientInfoBeforeDial_set(int wan_intf_idx, rtk_rg_pptpClientInfoBeforeDial_t *app_info)
{
	return 0;
}

rtk_rg_err_code_t rt_edp_pptpClientInfoAfterDial_set(int wan_intf_idx, rtk_rg_pptpClientInfoAfterDial_t *clientPptp_info)
{
	return 0;
}

rtk_rg_err_code_t rt_edp_l2tpClientInfoBeforeDial_set(int wan_intf_idx, rtk_rg_l2tpClientInfoBeforeDial_t *app_info)
{
	return 0;
}

rtk_rg_err_code_t rt_edp_l2tpClientInfoAfterDial_set(int wan_intf_idx, rtk_rg_l2tpClientInfoAfterDial_t *clientL2tp_info)
{
	return 0;
}

rtk_rg_err_code_t rt_edp_pppoeDsliteInfoBeforeDial_set(int wan_intf_idx, rtk_rg_pppoeClientInfoBeforeDial_t *app_info)
{
	return 0;
}

rtk_rg_err_code_t rt_edp_pppoeDsliteInfoAfterDial_set(int wan_intf_idx, rtk_rg_pppoeDsliteInfoAfterDial_t *pppoeDslite_info)
{
	return 0;
}

rtk_rg_err_code_t rt_edp_interface_del(int lan_or_wan_intf_idx)
{
	return 0;
}

rtk_rg_err_code_t rt_edp_intfInfo_find(rtk_rg_intfInfo_t *intf_info, int *valid_lan_or_wan_intf_idx)
{
	return 0;
}

rtk_rg_err_code_t rt_edp_cvlan_add(rtk_rg_cvlan_info_t *cvlan_info)
{

	unsigned int i,j;
	_rt_edp_pipe_cmd("brctl addbr br%d",cvlan_info->vlanId);
	_rt_edp_pipe_cmd("ifconfig br%d up",cvlan_info->vlanId);

	for(i=1,j=0;i<=1<<RTK_RG_PORT_MAX;i<<=1,j++)
	{
		if(cvlan_info->memberPortMask.portmask&i)
		{
			if(j<=4)
			{
				_rt_edp_pipe_cmd("ifconfig eth0.%d up",j+2);
				_rt_edp_pipe_cmd("vconfig add eth0.%d %d",j+2,cvlan_info->vlanId);
				_rt_edp_pipe_cmd("ifconfig eth0.%d.%d up",j+2,cvlan_info->vlanId);

				if((cvlan_info->untagPortMask.portmask&i))
				{
					if(rt_edp_db.systemGlobal.portBasedVID[j]==cvlan_info->vlanId)
					{
						_rt_edp_pipe_cmd("brctl delif br%d eth0.%d.%d",cvlan_info->vlanId,j+2,cvlan_info->vlanId);
						_rt_edp_pipe_cmd("brctl addif br%d eth0.%d",cvlan_info->vlanId,j+2);
					}
					else
					{
						WARNING("if br%d want to add eth0.%d, the pvid of eth0.%d must equal %d.",cvlan_info->vlanId,j+2,j+2,cvlan_info->vlanId);
						_rt_edp_pipe_cmd("brctl delif br%d eth0.%d",cvlan_info->vlanId,j+2);
						_rt_edp_pipe_cmd("brctl addif br%d eth0.%d.%d",cvlan_info->vlanId,j+2,cvlan_info->vlanId);
					}
				}
				else
				{
					_rt_edp_pipe_cmd("brctl delif br%d eth0.%d",cvlan_info->vlanId,j+2);
					_rt_edp_pipe_cmd("brctl addif br%d eth0.%d.%d",cvlan_info->vlanId,j+2,cvlan_info->vlanId);
				}
			}
		}
		else
		{
			if(j<=4)
			{
				_rt_edp_pipe_cmd("ifconfig eth0.%d.%d down",j+2,cvlan_info->vlanId);
				_rt_edp_pipe_cmd("brctl delif br%d eth0.%d.%d",cvlan_info->vlanId,j+2,cvlan_info->vlanId);
				if(rt_edp_db.systemGlobal.portBasedVID[j]==cvlan_info->vlanId)
				{
					_rt_edp_pipe_cmd("ifconfig eth0.%d down",j+2);
				}
				_rt_edp_pipe_cmd("brctl delif br%d eth0.%d",cvlan_info->vlanId,j+2);
			}
		}
	}


	/*
	wifi
	cvlan_info->wlan0DevMask
	cvlan_info->wlan0UntagMask
	*/



	return 0;
}

rtk_rg_err_code_t rt_edp_cvlan_del(int cvlan_id)
{
	return 0;
}

rtk_rg_err_code_t rt_edp_cvlan_get(rtk_rg_cvlan_info_t *cvlan_info)
{
	return 0;
}

rtk_rg_err_code_t rt_edp_vlanBinding_add(rtk_rg_vlanBinding_t *vlan_binding_info, int *vlan_binding_idx)
{
	return 0;
}

rtk_rg_err_code_t rt_edp_vlanBinding_del(int vlan_binding_idx)
{
	return 0;
}

rtk_rg_err_code_t rt_edp_vlanBinding_find(rtk_rg_vlanBinding_t *vlan_binding_info, int *valid_idx)
{
	return 0;
}

rtk_rg_err_code_t rt_edp_algServerInLanAppsIpAddr_add(rtk_rg_alg_serverIpMapping_t *srvIpMapping)
{
	int idx;

	if(srvIpMapping->algType<RTK_RG_ALG_SIP_TCP_SRV_IN_LAN_BIT || srvIpMapping->algType>RTK_RG_ALG_FTP_UDP_SRV_IN_LAN_BIT)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);

	for(idx = 0; idx < EDP_MAX_ALG_SERV_IN_LAN_NUM; idx++)
	{
		if(rt_edp_db.algServInLanIpMapping[idx].algType == 0)
			break;
		else if(rt_edp_db.algServInLanIpMapping[idx].serverAddress == srvIpMapping->serverAddress && rt_edp_db.algServInLanIpMapping[idx].algType == srvIpMapping->algType)
			RT_EDP_RETURN_ERR(RT_EDP_ERR_ENTRY_EXIST);
 	}
	if(idx == EDP_MAX_ALG_SERV_IN_LAN_NUM)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_ENTRY_FULL);

	rt_edp_db.algServInLanIpMapping[idx].serverAddress = srvIpMapping->serverAddress;
	rt_edp_db.algServInLanIpMapping[idx].algType = srvIpMapping->algType;

	if(srvIpMapping->algType == RTK_RG_ALG_FTP_TCP_SRV_IN_LAN_BIT)
		_rt_edp_pipe_cmd("iptables -t nat -A PREROUTING -p tcp --dport 21 -j DNAT --to-destination %pI4",&srvIpMapping->serverAddress);

	rt_edp_db.algServInLanIpMask |= srvIpMapping->algType;

	return 0;
}

rtk_rg_err_code_t rt_edp_algServerInLanAppsIpAddr_del(rtk_rg_alg_type_t delServerMapping)
{
	int i;
	//Check Param
	if(delServerMapping<RTK_RG_ALG_SIP_TCP_SRV_IN_LAN_BIT || delServerMapping>RTK_RG_ALG_FTP_UDP_SRV_IN_LAN_BIT)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);
	if((rt_edp_db.algFunctionMask&delServerMapping)>0)	//you can not delete serverIP when the ALG function is enabled...
		RT_EDP_RETURN_ERR(RT_EDP_ERR_ALG_SRV_IN_LAN_ENABLED);

	//Delete service/IP mapping in rg_db
	for(i=0;i<MAX_ALG_SERV_IN_LAN_NUM;i++)
	{
		if((delServerMapping&rt_edp_db.algServInLanIpMapping[i].algType)>0)
		{
			DEBUG("delete server address %pI4 from algType 0x%x",rt_edp_db.algServInLanIpMapping[i].serverAddress,rt_edp_db.algServInLanIpMapping[i].algType);

			_rt_edp_pipe_cmd("iptables -t nat -D PREROUTING -p tcp --dport 21 -j DNAT --to-destination %pI4",&rt_edp_db.algServInLanIpMapping[i].serverAddress);

			rt_edp_db.algServInLanIpMapping[i].serverAddress=0;

			rt_edp_db.algServInLanIpMask &= (~rt_edp_db.algServInLanIpMapping[i].algType);
		}
	}

	return 0;
}


static int kill_ct_by_port(struct nf_conn *i, void *data)
{
	int *port = (int *)data;
	if(i->tuplehash[IP_CT_DIR_ORIGINAL].tuple.src.u.all == *port || i->tuplehash[IP_CT_DIR_ORIGINAL].tuple.dst.u.all == *port ||
		i->tuplehash[IP_CT_DIR_REPLY].tuple.src.u.all == *port || i->tuplehash[IP_CT_DIR_REPLY].tuple.dst.u.all == *port)
	{
		DEBUG("kill ct by port:%d\n",*port);
		return 1;
	}
	else
		return 0;
}

rtk_rg_err_code_t rt_edp_algApps_set(rtk_rg_alg_type_t alg_app)
{
	int i;
	rtk_rg_alg_type_t algStateChange,algType;
	struct nf_conntrack_helper *helper=NULL;
	int moduleId,moduleNum,kill_port;
	char moduleName[NF_CT_HELPER_NAME_LEN];

	algStateChange=alg_app^rt_edp_db.algFunctionMask;

	DEBUG("ALG changed: 0x%x\n",algStateChange);
	for(i = 0; (0x1<<i) < RTK_RG_ALG_TYPE_END; i++)
	{
		int algProto = 0;
		if((algStateChange & (0x1<<i)) == 0)
			continue;

		if((0x1<<i) & RTK_RG_ALG_SIP_TCP_BIT)
		{
			algType = RTK_RG_ALG_SIP_TCP_BIT;
			moduleId = RT_EDP_ALG_TYPE_SIP_TCP_IDX;
			moduleNum = AF_INET;
			algProto = IPPROTO_TCP;
			kill_port = SIP_PORT;
			snprintf(moduleName,NF_CT_HELPER_NAME_LEN,"sip");
		}
		else if((0x1<<i) & RTK_RG_ALG_SIP_UDP_BIT)
		{
			algType = RTK_RG_ALG_SIP_UDP_BIT;
			moduleId = RT_EDP_ALG_TYPE_SIP_UDP_IDX;
			moduleNum = AF_INET;
			algProto = IPPROTO_UDP;
			kill_port = SIP_PORT;
			snprintf(moduleName,NF_CT_HELPER_NAME_LEN,"sip");
		}
		else if((0x1<<i) & RTK_RG_ALG_FTP_TCP_BIT)
		{
			algType = RTK_RG_ALG_FTP_TCP_BIT;
			moduleNum = PF_INET;
			moduleId = RT_EDP_ALG_TYPE_FTP_TCP_IDX;
			algProto = IPPROTO_TCP;
			kill_port = FTP_PORT;
			snprintf(moduleName,NF_CT_HELPER_NAME_LEN,"ftp");
		}
		else
			continue;

		if((0x1<<i) & algType)
		{
			if(alg_app & algType)
			{
				DEBUG("Enable %s ALG!\n",moduleName);
				helper = nf_conntrack_helper_try_module_get(moduleName,moduleNum,algProto);
				if(helper==NULL && rt_edp_helper[moduleId])
				{
					int ret;
					nf_ct_iterate_cleanup(&init_net, kill_ct_by_port, &kill_port, 0, 0);
					ret = nf_conntrack_helper_register(rt_edp_helper[moduleId]);
				}
				else
					WARNING("Register ftp alg failed!");
			}
			else
			{
				DEBUG("Disable %s ALG!\n",moduleName);
				nf_ct_iterate_cleanup(&init_net, kill_ct_by_port, &kill_port, 0, 0);
				helper = nf_conntrack_helper_try_module_get(moduleName,moduleNum,algProto);
				if(helper)
				{
					DEBUG("Unregister helper %s proto:%d @ %s %d\n",helper->name,algProto,__func__,__LINE__);
					nf_conntrack_helper_unregister(helper);
				}
			}
		}
	}
	rt_edp_db.algFunctionMask = alg_app;

	return 0;
}

rtk_rg_err_code_t rt_edp_algApps_get(rtk_rg_alg_type_t *alg_app)
{
	*alg_app = rt_edp_db.algFunctionMask;
	return 0;
}

rtk_rg_err_code_t rt_edp_dmzHost_set(int wan_intf_idx, rtk_rg_dmzInfo_t *dmz_info)
{
	if(wan_intf_idx > EDP_MAX_DMZ_TABLE_SIZE || wan_intf_idx < 0)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);

	rt_edp_db.dmzInfo[wan_intf_idx].ipversion = dmz_info->ipversion;

	if(dmz_info->ipversion == IPVER_V4ONLY || dmz_info->ipversion == IPVER_V4V6)
	{
		if(dmz_info->enabled)
		{
			rt_edp_db.dmzInfo[wan_intf_idx].enabled = 1;
			rt_edp_db.dmzInfo[wan_intf_idx].private_ip = dmz_info->private_ip;
			_rt_edp_pipe_cmd("iptables -t nat -N dmz");
			_rt_edp_pipe_cmd("iptables -t nat -A PREROUTING -j dmz");
			_rt_edp_pipe_cmd("iptables -t nat -A dmz -i nas0_%d -j DNAT --to-destination %pI4",wan_intf_idx,&dmz_info->private_ip);
		}
		else
		{
			rt_edp_db.dmzInfo[wan_intf_idx].enabled = 0;
			_rt_edp_pipe_cmd("iptables -t nat -D dmz -i nas0_%d -j DNAT --to-destination %pI4",wan_intf_idx,&rt_edp_db.dmzInfo[wan_intf_idx].private_ip);
		}
	}

	return 0;
}

rtk_rg_err_code_t rt_edp_dmzHost_get(int wan_intf_idx, rtk_rg_dmzInfo_t *dmz_info)
{
	if(dmz_info==NULL)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);

	dmz_info->enabled = rt_edp_db.dmzInfo[wan_intf_idx].enabled;
	dmz_info->mac_mapping_enabled = rt_edp_db.dmzInfo[wan_intf_idx].mac_mapping_enabled;
	dmz_info->ipversion = rt_edp_db.dmzInfo[wan_intf_idx].ipversion;
	dmz_info->private_ip = rt_edp_db.dmzInfo[wan_intf_idx].private_ip;
	memcpy(&dmz_info->private_ipv6.ipv6_addr[0],&rt_edp_db.dmzInfo[wan_intf_idx].private_ipv6.ipv6_addr[0],IPV6_ADDR_LEN);

	return 0;
}

rtk_rg_err_code_t rt_edp_virtualServer_add(rtk_rg_virtualServer_t *virtual_server, int *virtual_server_idx)
{
	int idx;
	char toPort[12];
	char act[4];
	int moduleId,moduleNum,moduleProto;
	char moduleName[NF_CT_HELPER_NAME_LEN];
	int kill_port;

	if(virtual_server->mappingPortRangeCnt <= 0)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);

	for(idx = 0; idx < EDP_MAX_VIRTUAL_SERVER_SW_TABLE_SIZE; idx++)
	{
		if(rt_edp_db.virtualServer[idx].valid == 0)
			break;
		else if(rt_edp_db.virtualServer[idx].ipversion == IPVER_V4ONLY || rt_edp_db.virtualServer[idx].ipversion == IPVER_V4V6)
		{
			//Duplicate check
			if(virtual_server->valid == 1
				&& (virtual_server->ipversion == IPVER_V4ONLY || virtual_server->ipversion == IPVER_V4ONLY)
				&& rt_edp_db.virtualServer[idx].is_tcp == virtual_server->is_tcp
				&& rt_edp_db.virtualServer[idx].local_ip == virtual_server->local_ip
				&& rt_edp_db.virtualServer[idx].local_port_start == virtual_server->local_port_start
				&& rt_edp_db.virtualServer[idx].mappingPortRangeCnt == virtual_server->mappingPortRangeCnt)
			{
					RT_EDP_RETURN_ERR(RT_EDP_ERR_ENTRY_EXIST);
			}

		}
	}
	if(idx == EDP_MAX_VIRTUAL_SERVER_SW_TABLE_SIZE)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_ENTRY_FULL);

	rt_edp_db.virtualServer[idx].ipversion = virtual_server->ipversion;
	rt_edp_db.virtualServer[idx].wan_intf_idx = virtual_server->wan_intf_idx;
	rt_edp_db.virtualServer[idx].is_tcp = virtual_server->is_tcp;
	rt_edp_db.virtualServer[idx].local_ip = virtual_server->local_ip;
	rt_edp_db.virtualServer[idx].local_port_start = virtual_server->local_port_start;
	rt_edp_db.virtualServer[idx].gateway_port_start = virtual_server->gateway_port_start;
	rt_edp_db.virtualServer[idx].mappingPortRangeCnt = virtual_server->mappingPortRangeCnt;
	rt_edp_db.virtualServer[idx].mappingType = virtual_server->mappingType;

	if(virtual_server->mappingType == VS_MAPPING_N_TO_N)
		snprintf(toPort, 12, "%d-%d",virtual_server->local_port_start, virtual_server->local_port_start+virtual_server->mappingPortRangeCnt-1);
	else if(virtual_server->mappingType == VS_MAPPING_N_TO_1)
		snprintf(toPort, 12, "%d",virtual_server->local_port_start);
	else
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);

	if(virtual_server->valid)
	{
		_rt_edp_pipe_cmd("iptables -t nat -N vtsvr");
		_rt_edp_pipe_cmd("iptables -t nat -A PREROUTING -j vtsvr");
		rt_edp_db.virtualServer[idx].valid = 1;
		snprintf(act, 4, "%s","-A");
	}
	else
	{
		rt_edp_db.virtualServer[idx].valid = 0;
		snprintf(act, 4, "%s","-D");
	}

	if(virtual_server->ipversion == IPVER_V4ONLY || virtual_server->ipversion == IPVER_V4V6)
	{
		_rt_edp_pipe_cmd("iptables -t nat %s vtsvr -i nas0_%d -p %s --dport %d:%d -j DNAT --to-destination %pI4:%s"
			,act
			,virtual_server->wan_intf_idx,virtual_server->is_tcp?"TCP":"UDP"
			,virtual_server->gateway_port_start
			,virtual_server->gateway_port_start+virtual_server->mappingPortRangeCnt-1
			,&virtual_server->local_ip
			,toPort);
	}

	*virtual_server_idx = idx;

	/* ALG */
	if(virtual_server->hookAlgType == RTK_RG_ALG_SIP_TCP_SRV_IN_LAN_BIT)
	{
		moduleId = RT_EDP_ALG_TYPE_SIP_TCP_IDX;
		moduleNum = AF_INET;
		moduleProto = IPPROTO_TCP;
		snprintf(moduleName,NF_CT_HELPER_NAME_LEN,"sip");
	}
	else if(virtual_server->hookAlgType == RTK_RG_ALG_SIP_UDP_SRV_IN_LAN_BIT)
	{
		moduleId = RT_EDP_ALG_TYPE_SIP_UDP_IDX;
		moduleNum = AF_INET;
		moduleProto = IPPROTO_UDP;
		snprintf(moduleName,NF_CT_HELPER_NAME_LEN,"sip");
	}
	else if(virtual_server->hookAlgType == RTK_RG_ALG_FTP_TCP_SRV_IN_LAN_BIT)
	{
		moduleId = RT_EDP_ALG_TYPE_FTP_TCP_IDX;
		moduleNum = PF_INET;
		moduleProto = IPPROTO_TCP;
		snprintf(moduleName,NF_CT_HELPER_NAME_LEN,"ftp");
	}
	else
		return 0;

	rt_edp_db.virtualServer[idx].hookAlgType = virtual_server->hookAlgType;
	if(rt_edp_helper[moduleId])
	{
		int i;

		for(i = 0; i < virtual_server->mappingPortRangeCnt; i++)
		{
			struct nf_conntrack_helper *helper = NULL;
			char helper_name[NF_CT_HELPER_NAME_LEN];

			if((virtual_server->local_port_start+i) == FTP_PORT)
				snprintf(helper_name,NF_CT_HELPER_NAME_LEN,moduleName);
			else
				snprintf(helper_name,NF_CT_HELPER_NAME_LEN,"%s-%d",moduleName,virtual_server->local_port_start+i);

			//Flush associated connections
			kill_port = virtual_server->local_port_start+i;
			nf_ct_iterate_cleanup(&init_net, kill_ct_by_port, &kill_port, 0, 0);

			helper = nf_conntrack_helper_try_module_get(helper_name,moduleNum,moduleProto);
			if(helper==NULL)
			{
				helper = kmalloc(sizeof(struct nf_conntrack_helper),GFP_KERNEL);
				memset(helper,0,sizeof(struct nf_conntrack_helper));
				memcpy(helper,rt_edp_helper[moduleId],sizeof(struct nf_conntrack_helper));
				snprintf(helper->name,NF_CT_HELPER_NAME_LEN,helper_name);
				helper->tuple.src.u.tcp.port = virtual_server->local_port_start+i;
				DEBUG("Register module:%s @ %s %d\n",helper->name,__func__,__LINE__);
				if(virtual_server->valid)
					nf_conntrack_helper_register(helper);
				else
					nf_conntrack_helper_unregister(helper);
			}
		}
	}

	return 0;
}

rtk_rg_err_code_t rt_edp_virtualServer_del(int virtual_server_idx)
{
	char toPort[20];
	int cnt;
	int moduleNum,moduleProto;
	char moduleName[NF_CT_HELPER_NAME_LEN];

	if(rt_edp_db.virtualServer[virtual_server_idx].mappingType == VS_MAPPING_N_TO_N)
		snprintf(toPort, 12, "%d-%d",rt_edp_db.virtualServer[virtual_server_idx].local_port_start, rt_edp_db.virtualServer[virtual_server_idx].local_port_start+rt_edp_db.virtualServer[virtual_server_idx].mappingPortRangeCnt-1);
	else if(rt_edp_db.virtualServer[virtual_server_idx].mappingType == VS_MAPPING_N_TO_1)
		snprintf(toPort, 12, "%d",rt_edp_db.virtualServer[virtual_server_idx].local_port_start);
	else
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);

	if(rt_edp_db.virtualServer[virtual_server_idx].mappingPortRangeCnt <= 0)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);

	if(rt_edp_db.virtualServer[virtual_server_idx].valid == 0)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);

	_rt_edp_pipe_cmd("iptables -t nat -D vtsvr -i nas0_%d -p %s --dport %d:%d -j DNAT --to-destination %pI4:%s"
	,rt_edp_db.virtualServer[virtual_server_idx].wan_intf_idx,rt_edp_db.virtualServer[virtual_server_idx].is_tcp?"TCP":"UDP"
	,rt_edp_db.virtualServer[virtual_server_idx].gateway_port_start
	,rt_edp_db.virtualServer[virtual_server_idx].gateway_port_start+rt_edp_db.virtualServer[virtual_server_idx].mappingPortRangeCnt-1
	,&rt_edp_db.virtualServer[virtual_server_idx].local_ip
	,toPort);

	// ALG
	if(rt_edp_db.virtualServer[virtual_server_idx].hookAlgType)
	{
		if(rt_edp_db.virtualServer[virtual_server_idx].hookAlgType == RTK_RG_ALG_SIP_TCP_SRV_IN_LAN_BIT)
		{
			moduleNum = AF_INET;
			moduleProto = IPPROTO_TCP;
			snprintf(moduleName,NF_CT_HELPER_NAME_LEN,"sip");
		}
		else if(rt_edp_db.virtualServer[virtual_server_idx].hookAlgType == RTK_RG_ALG_SIP_UDP_SRV_IN_LAN_BIT)
		{
			moduleNum = AF_INET;
			moduleProto = IPPROTO_UDP;
			snprintf(moduleName,NF_CT_HELPER_NAME_LEN,"sip");
		}
		else if(rt_edp_db.virtualServer[virtual_server_idx].hookAlgType == RTK_RG_ALG_FTP_TCP_SRV_IN_LAN_BIT)
		{
			moduleNum = PF_INET;
			moduleProto = IPPROTO_TCP;
			snprintf(moduleName,NF_CT_HELPER_NAME_LEN,"ftp");
		}
		else
		{
			memset(&rt_edp_db.virtualServer[virtual_server_idx],0,sizeof(rtk_rg_virtualServer_t));
			return 0;
		}

		for(cnt = 0; cnt < rt_edp_db.virtualServer[virtual_server_idx].mappingPortRangeCnt; cnt++)
		{
			int kill_port;
			struct nf_conntrack_helper *del_helper;
			char helper_name[NF_CT_HELPER_NAME_LEN];

			//Flush associated connections
			kill_port = rt_edp_db.virtualServer[virtual_server_idx].local_port_start+cnt;
			nf_ct_iterate_cleanup(&init_net, kill_ct_by_port, &kill_port, 0, 0);

			//Unregister assoicated ALG port
			if((rt_edp_db.virtualServer[virtual_server_idx].local_port_start+cnt) == FTP_PORT ||
				(rt_edp_db.virtualServer[virtual_server_idx].local_port_start+cnt) == SIP_PORT)
				snprintf(helper_name,NF_CT_HELPER_NAME_LEN,moduleName);
			else
				snprintf(helper_name,NF_CT_HELPER_NAME_LEN,"%s-%d",moduleName,rt_edp_db.virtualServer[virtual_server_idx].local_port_start+cnt);
			del_helper = NULL;
			del_helper = nf_conntrack_helper_try_module_get(helper_name,moduleNum,moduleProto);
			if(del_helper)
			{
				DEBUG("Unregister helper name:%s\n",del_helper->name);
				nf_conntrack_helper_unregister(del_helper);
				//kfree(del_helper);
			}
		}
	}

	memset(&rt_edp_db.virtualServer[virtual_server_idx],0,sizeof(rtk_rg_virtualServer_t));
	return 0;
}

rtk_rg_err_code_t rt_edp_virtualServer_find(rtk_rg_virtualServer_t *virtual_server, int *valid_idx)
{
	int idx=0;

	if(virtual_server==NULL || valid_idx==NULL)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);
	if(*valid_idx<0 || *valid_idx>=EDP_MAX_VIRTUAL_SERVER_SW_TABLE_SIZE)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);

	//Find server port mapping
	for(idx=*valid_idx;idx<EDP_MAX_VIRTUAL_SERVER_SW_TABLE_SIZE;idx++)
	{
		if(rt_edp_db.virtualServer[idx].valid)
		{
			*valid_idx=idx;
			memcpy(virtual_server,&rt_edp_db.virtualServer[idx],sizeof(rtk_rg_virtualServer_t));
			return RT_EDP_ERR_OK;
		}
	}
    return RT_EDP_ERR_SVRPORT_SW_ENTRY_NOT_FOUND;


	return 0;
}

rtk_rg_err_code_t rt_edp_aclFilterAndQos_add(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx)
{
	return _rt_edp_aclFilterAndQos_add(acl_filter,acl_filter_idx);
}

rtk_rg_err_code_t rt_edp_aclFilterAndQos_del(int acl_filter_idx)
{
	return _rt_edp_aclFilterAndQos_del(acl_filter_idx);
}

rtk_rg_err_code_t rt_edp_aclFilterAndQos_find(rtk_rg_aclFilterAndQos_t *acl_filter, int *valid_idx)
{
	return _rt_edp_aclFilterAndQos_find(acl_filter,valid_idx);
}

rtk_rg_err_code_t rt_edp_naptFilterAndQos_add(int *index,rtk_rg_naptFilterAndQos_t *napt_filter){


	return 0;
}
rtk_rg_err_code_t rt_edp_naptFilterAndQos_del(int index){



	return 0;
}
rtk_rg_err_code_t rt_edp_naptFilterAndQos_find(int *index,rtk_rg_naptFilterAndQos_t *napt_filter){



	return 0;
}

rtk_rg_err_code_t rt_edp_macFilter_add(rtk_rg_macFilterEntry_t *macFilterEntry,int *mac_filter_idx)
{



	return 0;
}

rtk_rg_err_code_t rt_edp_macFilter_del(int mac_filter_idx)
{



	return 0;
}

rtk_rg_err_code_t rt_edp_macFilter_find(rtk_rg_macFilterEntry_t *macFilterEntry, int *valid_idx)
{



	return 0;
}

rtk_rg_err_code_t rt_edp_urlFilterString_add(rtk_rg_urlFilterString_t *filter,int *url_idx)
{



	return 0;
}

rtk_rg_err_code_t rt_edp_urlFilterString_del(int url_idx)
{



	return 0;
}

rtk_rg_err_code_t rt_edp_urlFilterString_find(rtk_rg_urlFilterString_t *filter, int *valid_idx)
{



	return 0;
}



rtk_rg_err_code_t rt_edp_upnpConnection_add(rtk_rg_upnpConnection_t *upnp, int *upnp_idx)
{



	return 0;
}

rtk_rg_err_code_t rt_edp_upnpConnection_del(int upnp_idx)
{



	return 0;
}

rtk_rg_err_code_t rt_edp_upnpConnection_find(rtk_rg_upnpConnection_t *upnp, int *valid_idx)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_naptConnection_add(rtk_rg_naptEntry_t *naptFlow, int *flow_idx)
{



	return 0;
}

rtk_rg_err_code_t rt_edp_naptConnection_del(int flow_idx)
{



	return 0;
}

rtk_rg_err_code_t rt_edp_naptConnection_find(rtk_rg_naptInfo_t *naptInfo,int *valid_idx)
{



	return 0;
}


//we not support flow_idx
rtk_rg_err_code_t rt_edp_multicastFlow_add(rtk_rg_multicastFlow_t *mcFlow, int *flow_idx)
{

	int i,hitidx=FAIL,firstInvaild=FAIL,configidx;

	if(mcFlow->srcFilterMode!=RTK_RG_IPV4MC_DONT_CARE_SRC)
		return RT_ERR_RG_CHIP_NOT_SUPPORT;
	if(mcFlow->isIVL)
		return RT_ERR_RG_CHIP_NOT_SUPPORT;

	//force setting to User Mode
	rtk_fc_igmp_mcMode(RTK_FC_MC_SYNC_BY_USER);

	for(i=0 ; i< EDP_MAX_MC_API_CONFIG_NUM ;i++)
	{
		if(rt_edp_db.mcApiMapping[i].valid)
		{
			if(mcFlow->isIPv6 && rt_edp_db.mcApiMapping[i].mcConfig.is_ipv6)
			{
				if( (mcFlow->isIPv6 && memcmp(rt_edp_db.mcApiMapping[i].mcConfig.groupAddr.ipv6,mcFlow->multicast_ipv6_addr,sizeof(rt_edp_db.mcApiMapping[i].mcConfig.groupAddr.ipv6))==0) ||
					((mcFlow->isIPv6==0) && (mcFlow->multicast_ipv4_addr == rt_edp_db.mcApiMapping[i].mcConfig.groupAddr.ipv4) ))
				{
					hitidx=i;
					break;
				}
			}
		}
		else
		{
			if(firstInvaild== FAIL)
				firstInvaild=i;
		}
	}
	if(firstInvaild==FAIL && hitidx==FAIL)
		return RT_ERR_RG_ENTRY_FULL;

	if(hitidx!=FAIL)
		configidx=hitidx;
	else
		configidx=firstInvaild;

	bzero(&rt_edp_db.mcApiMapping[configidx].mcConfig,sizeof(rt_edp_db.mcApiMapping[configidx].mcConfig));
	rt_edp_db.mcApiMapping[configidx].mcConfig.act0_pmsk = mcFlow->port_mask.portmask;
	rt_edp_db.mcApiMapping[configidx].mcConfig.is_ipv6 = mcFlow->isIPv6;
	if(mcFlow->isIPv6)
		memcpy(rt_edp_db.mcApiMapping[configidx].mcConfig.groupAddr.ipv6,mcFlow->multicast_ipv6_addr,sizeof(rt_edp_db.mcApiMapping[configidx].mcConfig.groupAddr.ipv6));
	else
		rt_edp_db.mcApiMapping[configidx].mcConfig.groupAddr.ipv4 = mcFlow->multicast_ipv4_addr;

	rtk_fc_igmp_mcGroupSet(&rt_edp_db.mcApiMapping[configidx].mcConfig);

	rt_edp_db.mcApiMapping[configidx].valid = 1;

	DEBUG("add/modify mcApiMapping[%d]",configidx);
	*flow_idx =configidx;

	return RT_ERR_RG_CHIP_NOT_SUPPORT;
}

rtk_rg_err_code_t rt_edp_multicastFlow_del(int flow_idx)
{

	rt_edp_db.mcApiMapping[flow_idx].mcConfig.act0_pmsk=0;
	rtk_fc_igmp_mcGroupSet(&rt_edp_db.mcApiMapping[flow_idx].mcConfig);
	rt_edp_db.mcApiMapping[flow_idx].valid=0;
	DEBUG("del mcApiMapping[%d] entry",flow_idx);

	return RT_ERR_RG_CHIP_NOT_SUPPORT;
}


rtk_rg_err_code_t rt_edp_l2MultiCastFlow_add(rtk_rg_l2MulticastFlow_t *l2McFlow,int *flow_idx)
{
	WARNING("rt_edp_l2MultiCastFlow_add RT_ERR_RG_CHIP_NOT_SUPPORT");
	return RT_ERR_RG_CHIP_NOT_SUPPORT;
}

rtk_rg_err_code_t rt_edp_multicastFlow_find(rtk_rg_multicastFlow_t *mcFlow, int *valid_idx)
{
	WARNING("rt_edp_multicastFlow_find RT_ERR_RG_CHIP_NOT_SUPPORT");
	return RT_ERR_RG_CHIP_NOT_SUPPORT;
}

rtk_rg_err_code_t rt_edp_macEntry_add(rtk_rg_macEntry_t *macEntry, int *entry_idx)
{
	//_rtk_rg_fc_lut_learning =>RTK_RG_FC_LUT_ADD


	return 0;
}

rtk_rg_err_code_t rt_edp_macEntry_del(int entry_idx)
{
	//RTK_RG_FC_LUT_DEL

	return 0;
}

rtk_rg_err_code_t rt_edp_macEntry_find(rtk_rg_macEntry_t *macEntry,int *valid_idx)
{

	return 0;
}

rtk_rg_err_code_t rt_edp_arpEntry_add(rtk_rg_arpEntry_t *arpEntry, int *arp_entry_idx)
{
	return 0;
}

rtk_rg_err_code_t rt_edp_arpEntry_del(int arp_entry_idx)
{

	return 0;
}

rtk_rg_err_code_t rt_edp_arpEntry_find(rtk_rg_arpInfo_t *arpInfo,int *arp_valid_idx)
{

	return 0;
}

rtk_rg_err_code_t rt_edp_neighborEntry_add(rtk_rg_neighborEntry_t *neighborEntry,int *neighbor_idx)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_neighborEntry_del(int neighbor_idx)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_neighborEntry_find(rtk_rg_neighborInfo_t *neighborInfo,int *neighbor_valid_idx)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_softwareIdleTime_set(rtk_rg_idle_time_type_t idleTimeType, int idleTime)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_softwareIdleTime_get(rtk_rg_idle_time_type_t idleTimeType, int *pIdleTime)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_accessWanLimit_set(rtk_rg_accessWanLimitData_t access_wan_info)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_accessWanLimit_get(rtk_rg_accessWanLimitData_t *access_wan_info)
{

	return 0;
}

rtk_rg_err_code_t rt_edp_accessWanLimitCategory_set(rtk_rg_accessWanLimitCategory_t macCategory_info)
{

	return 0;
}

rtk_rg_err_code_t rt_edp_accessWanLimitCategory_get(rtk_rg_accessWanLimitCategory_t *macCategory_info)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_softwareSourceAddrLearningLimit_set(rtk_rg_saLearningLimitInfo_t sa_learnLimit_info, rtk_rg_port_idx_t port_idx)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_softwareSourceAddrLearningLimit_get(rtk_rg_saLearningLimitInfo_t *sa_learnLimit_info, rtk_rg_port_idx_t port_idx)
{




	return 0;
}

rtk_rg_err_code_t rt_edp_wlanSoftwareSourceAddrLearningLimit_set(rtk_rg_saLearningLimitInfo_t sa_learnLimit_info, int wlan_idx, int dev_idx)
{

	return 0;
}

rtk_rg_err_code_t rt_edp_wlanSoftwareSourceAddrLearningLimit_get(rtk_rg_saLearningLimitInfo_t *sa_learnLimit_info, int wlan_idx, int dev_idx)
{

	return 0;
}

rtk_rg_err_code_t rt_edp_dosPortMaskEnable_set(rtk_rg_mac_portmask_t dos_port_mask)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_dosPortMaskEnable_get(rtk_rg_mac_portmask_t *dos_port_mask)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_dosType_set(rtk_rg_dos_type_t dos_type,int dos_enabled,rtk_rg_dos_action_t dos_action)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_dosType_get(rtk_rg_dos_type_t dos_type,int *dos_enabled,rtk_rg_dos_action_t *dos_action)
{

	return 0;
}

rtk_rg_err_code_t rt_edp_dosFloodType_set(rtk_rg_dos_type_t dos_type,int dos_enabled,rtk_rg_dos_action_t dos_action,int dos_threshold)
{



	return 0;
}

rtk_rg_err_code_t rt_edp_dosFloodType_get(rtk_rg_dos_type_t dos_type,int *dos_enabled,rtk_rg_dos_action_t *dos_action,int *dos_threshold)
{



	return 0;
}

rtk_rg_err_code_t rt_edp_portMirror_set(rtk_rg_portMirrorInfo_t portMirrorInfo)
{
	rtk_portmask_t mirroredRxPortmask;
	rtk_portmask_t mirroredTxPortmask;
	if(portMirrorInfo.direct < RTK_RG_MIRROR_TX_RX_BOTH || portMirrorInfo.direct>=RTK_RG_MIRROR_END ) return (RT_ERR_RG_INVALID_PARAM);

	memset(&mirroredRxPortmask,0,sizeof(rtk_portmask_t));
	memset(&mirroredTxPortmask,0,sizeof(rtk_portmask_t));

	if(portMirrorInfo.direct==RTK_RG_MIRROR_RX_ONLY){
		mirroredRxPortmask.bits[0] = portMirrorInfo.enabledPortMask.portmask;
		mirroredTxPortmask.bits[0] = 0x0;
	}else if(portMirrorInfo.direct==RTK_RG_MIRROR_TX_ONLY){
		mirroredRxPortmask.bits[0] = 0x0;
		mirroredTxPortmask.bits[0] = portMirrorInfo.enabledPortMask.portmask;
	}else{//RTK_RG_MIRROR_TX_RX_BOTH
		mirroredRxPortmask.bits[0] = portMirrorInfo.enabledPortMask.portmask;
		mirroredTxPortmask.bits[0] = portMirrorInfo.enabledPortMask.portmask;
	}
	rtk_mirror_portBased_set(portMirrorInfo.monitorPort, &mirroredRxPortmask, &mirroredTxPortmask);

	return (RT_ERR_RG_OK);

}

rtk_rg_err_code_t rt_edp_portMirror_get(rtk_rg_portMirrorInfo_t *portMirrorInfo)
{

	rtk_portmask_t mirroredRxPortmask;
	rtk_portmask_t mirroredTxPortmask;
	if(portMirrorInfo==NULL) return(RT_ERR_RG_NULL_POINTER);
	memset(&mirroredRxPortmask,0,sizeof(rtk_portmask_t));
	memset(&mirroredTxPortmask,0,sizeof(rtk_portmask_t));
	rtk_mirror_portBased_get(&(portMirrorInfo->monitorPort), &mirroredRxPortmask, &mirroredTxPortmask);
	if(mirroredRxPortmask.bits[0]==0x0 && mirroredTxPortmask.bits[0]!=0x0){
		portMirrorInfo->enabledPortMask.portmask= mirroredTxPortmask.bits[0] ;
		portMirrorInfo->direct = RTK_RG_MIRROR_TX_ONLY;
	}else if(mirroredRxPortmask.bits[0]!=0x0 && mirroredTxPortmask.bits[0]==0x0){
		portMirrorInfo->enabledPortMask.portmask = mirroredRxPortmask.bits[0] ;
		portMirrorInfo->direct = RTK_RG_MIRROR_RX_ONLY;
	}else if(mirroredRxPortmask.bits[0]!=0x0 && mirroredTxPortmask.bits[0]!=0x0){
		portMirrorInfo->enabledPortMask.portmask= (mirroredRxPortmask.bits[0] | mirroredTxPortmask.bits[0]) ;
		portMirrorInfo->direct = RTK_RG_MIRROR_TX_RX_BOTH;
	}

	return (RT_ERR_RG_OK);

}

rtk_rg_err_code_t rt_edp_portMirror_clear(void)
{
	rtk_portmask_t mirroredRxPortmask;
	rtk_portmask_t mirroredTxPortmask;
	memset(&mirroredRxPortmask,0,sizeof(rtk_portmask_t));
	memset(&mirroredTxPortmask,0,sizeof(rtk_portmask_t));
	mirroredRxPortmask.bits[0] = 0x0;
	mirroredTxPortmask.bits[0] = 0x0;
	rtk_mirror_portBased_set(0x0, &mirroredRxPortmask, &mirroredTxPortmask);
	return (RT_ERR_RG_OK);

}

rtk_rg_err_code_t rt_edp_portEgrBandwidthCtrlRate_set(rtk_rg_mac_port_idx_t port, uint32 rate)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_portIgrBandwidthCtrlRate_set(rtk_rg_mac_port_idx_t port, uint32 rate)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_portEgrBandwidthCtrlRate_get(rtk_rg_mac_port_idx_t port, uint32 *rate)
{

	return 0;
}

rtk_rg_err_code_t rt_edp_portIgrBandwidthCtrlRate_get(rtk_rg_mac_port_idx_t port, uint32 *rate)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_phyPortForceAbility_set(rtk_rg_mac_port_idx_t port, rtk_rg_phyPortAbilityInfo_t ability)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_phyPortForceAbility_get(rtk_rg_mac_port_idx_t port, rtk_rg_phyPortAbilityInfo_t *ability)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_cpuPortForceTrafficCtrl_set(rtk_rg_enable_t tx_fc_state,	rtk_rg_enable_t rx_fc_state)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_cpuPortForceTrafficCtrl_get(rtk_rg_enable_t *pTx_fc_state,	rtk_rg_enable_t *pRx_fc_state)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_portMibInfo_get(rtk_rg_mac_port_idx_t port, rtk_rg_port_mib_info_t *mibInfo)
{



	return 0;
}

rtk_rg_err_code_t rt_edp_portMibInfo_clear(rtk_rg_mac_port_idx_t port)
{



	return 0;
}

rtk_rg_err_code_t rt_edp_portIsolation_set(rtk_rg_port_isolation_t isolationSetting)
{



	return 0;
}

rtk_rg_err_code_t rt_edp_portIsolation_get(rtk_rg_port_isolation_t *isolationSetting)
{



	return 0;
}

rtk_rg_err_code_t rt_edp_stormControl_add(rtk_rg_stormControlInfo_t *stormInfo,int *stormInfo_idx)
{



	return 0;
}

rtk_rg_err_code_t rt_edp_stormControl_del(int stormInfo_idx)
{



	return 0;
}

rtk_rg_err_code_t rt_edp_stormControl_find(rtk_rg_stormControlInfo_t *stormInfo,int *stormInfo_idx)
{

	return 0;
}

rtk_rg_err_code_t rt_edp_shareMeter_set(uint32 index, uint32 rate, rtk_rg_enable_t ifgInclude)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_shareMeter_get(uint32 index, uint32 *pRate , rtk_rg_enable_t *pIfgInclude)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_shareMeterMode_set(uint32 index, rtk_rate_metet_mode_t meterMode)
{

	return 0;
}

rtk_rg_err_code_t rt_edp_shareMeterMode_get(uint32 index, rtk_rate_metet_mode_t *pMeterMode)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_qosStrictPriorityOrWeightFairQueue_set(rtk_rg_mac_port_idx_t port_idx,rtk_rg_qos_queue_weights_t q_weight)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_qosStrictPriorityOrWeightFairQueue_get(rtk_rg_mac_port_idx_t port_idx,rtk_rg_qos_queue_weights_t *pQ_weight)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_qosInternalPriMapToQueueId_set(int int_pri, int queue_id)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_qosInternalPriMapToQueueId_get(int int_pri, int *pQueue_id)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_qosInternalPriDecisionByWeight_set(rtk_rg_qos_priSelWeight_t weightOfPriSel)
{



	return 0;
}

rtk_rg_err_code_t rt_edp_qosInternalPriDecisionByWeight_get(rtk_rg_qos_priSelWeight_t *pWeightOfPriSel)
{

	return 0;
}

rtk_rg_err_code_t rt_edp_qosDscpRemapToInternalPri_set(uint32 dscp,uint32 int_pri)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_qosDscpRemapToInternalPri_get(uint32 dscp,uint32 *pInt_pri)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_qosPortBasedPriority_set(rtk_rg_mac_port_idx_t port_idx,uint32 int_pri)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_qosPortBasedPriority_get(rtk_rg_mac_port_idx_t port_idx,uint32 *pInt_pri)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_qosDot1pPriRemapToInternalPri_set(uint32 dot1p,uint32 int_pri)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_qosDot1pPriRemapToInternalPri_get(uint32 dot1p,uint32 *pInt_pri)
{

	return 0;
}

rtk_rg_err_code_t rt_edp_qosDscpRemarkEgressPortEnableAndSrcSelect_set(rtk_rg_mac_port_idx_t rmk_port,rtk_rg_enable_t rmk_enable, rtk_rg_qos_dscpRmkSrc_t rmk_src_select)
{

	return 0;
}

rtk_rg_err_code_t rt_edp_qosDscpRemarkEgressPortEnableAndSrcSelect_get(rtk_rg_mac_port_idx_t rmk_port,rtk_rg_enable_t *pRmk_enable, rtk_rg_qos_dscpRmkSrc_t *pRmk_src_select)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_qosDscpRemarkByInternalPri_set(int int_pri,int rmk_dscp)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_qosDscpRemarkByInternalPri_get(int int_pri,int *pRmk_dscp)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_qosDscpRemarkByDscp_set(int dscp,int rmk_dscp)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_qosDscpRemarkByDscp_get(int dscp,int *pRmk_dscp)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_qosDot1pPriRemarkByInternalPriEgressPortEnable_set(rtk_rg_mac_port_idx_t rmk_port, rtk_rg_enable_t rmk_enable)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_qosDot1pPriRemarkByInternalPriEgressPortEnable_get(rtk_rg_mac_port_idx_t rmk_port, rtk_rg_enable_t *pRmk_enable)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_qosDot1pPriRemarkByInternalPri_set(int int_pri,int rmk_dot1p)
{

	return 0;
}

rtk_rg_err_code_t rt_edp_qosDot1pPriRemarkByInternalPri_get(int int_pri,int *pRmk_dot1p)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_portBasedCVlanId_set(rtk_rg_port_idx_t port_idx,int pvid)
{
	rt_edp_db.systemGlobal.portBasedVID[port_idx]=pvid;
	return 0;
}

rtk_rg_err_code_t rt_edp_portBasedCVlanId_get(rtk_rg_port_idx_t port_idx,int *pPvid)
{
	*pPvid=rt_edp_db.systemGlobal.portBasedVID[port_idx];
	return 0;
}

rtk_rg_err_code_t rt_edp_wlanDevBasedCVlanId_set(int wlan_idx,int dev_idx,int dvid)
{

	return 0;
}

rtk_rg_err_code_t rt_edp_wlanDevBasedCVlanId_get(int wlan_idx,int dev_idx,int *pDvid)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_portStatus_get(rtk_rg_mac_port_idx_t port, rtk_rg_portStatusInfo_t *portInfo)
{

	return 0;
}

#ifdef CONFIG_RG_NAPT_PORT_COLLISION_PREVENTION
rtk_rg_err_code_t rt_edp_naptExtPortGet(int isTcp,uint16 *pPort)
{

	return 0;
}

rtk_rg_err_code_t rt_edp_naptExtPortFree(int isTcp,uint16 port)
{

	return 0;
}
#endif

rtk_rg_err_code_t rt_edp_classifyEntry_add(rtk_rg_classifyEntry_t *classifyFilter)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_classifyEntry_find(int index, rtk_rg_classifyEntry_t *classifyFilter)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_classifyEntry_del(int index)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_svlanTpid_set(uint32 svlan_tag_id)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_svlanTpid_get(uint32 *pSvlanTagId)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_svlanServicePort_set(rtk_port_t port, rtk_enable_t enable)
{

	return 0;
}

rtk_rg_err_code_t rt_edp_svlanServicePort_get(rtk_port_t port, rtk_enable_t *pEnable)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_svlanTpid2_enable_set(rtk_rg_enable_t enable)
{


	return 0;
}
rtk_rg_err_code_t rt_edp_svlanTpid2_enable_get(rtk_rg_enable_t *pEnable)
{

	return 0;
}
rtk_rg_err_code_t rt_edp_svlanTpid2_set(uint32 svlan_tag_id)
{
	return 0;
}
rtk_rg_err_code_t rt_edp_svlanTpid2_get(uint32 *pSvlanTagId)
{
	return 0;
}



rtk_rg_err_code_t rt_edp_pppoeInterfaceIdleTime_get(int intfIdx,uint32 *idleSec)
{
	return 0;
}

rtk_rg_err_code_t rt_edp_gatewayServicePortRegister_add(rtk_rg_gatewayServicePortEntry_t *serviceEntry, int *index)
{
	return 0;
}

rtk_rg_err_code_t rt_edp_gatewayServicePortRegister_del(int index)
{
	return 0;
}

rtk_rg_err_code_t rt_edp_gatewayServicePortRegister_find(rtk_rg_gatewayServicePortEntry_t *serviceEntry, int *index)
{
	return 0;
}

rtk_rg_err_code_t rt_edp_stpBlockingPortmask_set(rtk_rg_portmask_t Mask){


	return 0;
}
rtk_rg_err_code_t rt_edp_stpBlockingPortmask_get(rtk_rg_portmask_t *pMask){


	return 0;
}

rtk_rg_err_code_t rt_edp_gponDsBcFilterAndRemarking_Enable(rtk_rg_enable_t enable)
{

	return 0;
}


rtk_rg_err_code_t rt_edp_gponDsBcFilterAndRemarking_add(rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t *filterRule,int *index)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_gponDsBcFilterAndRemarking_del(int index)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_gponDsBcFilterAndRemarking_del_all(void)
{

	return 0;
}

rtk_rg_err_code_t rt_edp_gponDsBcFilterAndRemarking_find(int *index,rtk_rg_gpon_ds_bc_vlanfilterAndRemarking_t *filterRule){

	return 0;
}


rtk_rg_err_code_t rt_edp_interfaceMibCounter_del(int intf_idx)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_interfaceMibCounter_get(rtk_rg_netifMib_entry_t *pNetifMib)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_redirectHttpAll_set(rtk_rg_redirectHttpAll_t *pRedirectHttpAll)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_redirectHttpAll_get(rtk_rg_redirectHttpAll_t *pRedirectHttpAll)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_redirectHttpURL_add(rtk_rg_redirectHttpURL_t *pRedirectHttpURL)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_redirectHttpURL_del(rtk_rg_redirectHttpURL_t *pRedirectHttpURL)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_redirectHttpWhiteList_add(rtk_rg_redirectHttpWhiteList_t *pRedirectHttpWhiteList)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_redirectHttpWhiteList_del(rtk_rg_redirectHttpWhiteList_t *pRedirectHttpWhiteList)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_redirectHttpRsp_set(rtk_rg_redirectHttpRsp_t *pRedirectHttpRsp)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_redirectHttpRsp_get(rtk_rg_redirectHttpRsp_t *pRedirectHttpRsp)
{



	return 0;
}

rtk_rg_err_code_t rt_edp_redirectHttpCount_set(rtk_rg_redirectHttpCount_t *pRedirectHttpCount)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_redirectHttpCount_get(rtk_rg_redirectHttpCount_t *pRedirectHttpCount)
{

	return 0;
}


rtk_rg_err_code_t rt_edp_hostPoliceControl_set(rtk_rg_hostPoliceControl_t *pHostPoliceControl, int host_idx)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_hostPoliceControl_get(rtk_rg_hostPoliceControl_t *pHostPoliceControl, int host_idx)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_hostPoliceLogging_get(rtk_rg_hostPoliceLogging_t *pHostMibCnt, int host_idx)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_hostPoliceLogging_del(int host_idx)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_staticRoute_add(rtk_rg_staticRoute_t *pStaticRoute, int *index)
{

	return 0;

}

rtk_rg_err_code_t rt_edp_staticRoute_del(int index)
{

	return 0;

}

rtk_rg_err_code_t rt_edp_staticRoute_find(rtk_rg_staticRoute_t *pStaticRoute, int *index)
{

	return 0;
}

rtk_rg_err_code_t rt_edp_aclLogCounterControl_get(int index, int *type,  int *mode)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_aclLogCounterControl_set(int index,  int type,  int mode)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_aclLogCounter_get(int index, uint64 *count)
{


	return 0;
}

rtk_rg_err_code_t rt_edp_aclLogCounter_reset(int index)
{
	return 0;

}

rtk_rg_err_code_t rt_edp_groupMacLimit_set(rtk_rg_groupMacLimit_t group_mac_info)
{
	return 0;

}

rtk_rg_err_code_t rt_edp_groupMacLimit_get(rtk_rg_groupMacLimit_t *pGroup_mac_info)
{
	return 0;

}

rtk_rg_err_code_t rt_edp_igmpMldSnoopingControl_set(rtk_rg_igmpMldSnoopingControl_t *config )
{
	return 0;

}

rtk_rg_err_code_t rt_edp_igmpMldSnoopingControl_get(rtk_rg_igmpMldSnoopingControl_t *config )
{
	return 0;

}

rtk_rg_err_code_t rt_edp_igmpMldSnoopingPortControl_add(rtk_rg_port_idx_t port_idx,rtk_rg_igmpMldSnoopingPortControl_t *config )
{
	return 0;

}
rtk_rg_err_code_t rt_edp_igmpMldSnoopingPortControl_del(rtk_rg_port_idx_t port_idx)
{
	return 0;

}
rtk_rg_err_code_t rt_edp_igmpMldSnoopingPortControl_find(rtk_rg_port_idx_t port_idx,rtk_rg_igmpMldSnoopingPortControl_t *config )
{
	return 0;

}

rtk_rg_err_code_t rt_edp_flowMibCounter_get(int index, rtk_rg_table_flowmib_t *pCounter)
{
	return 0;

}

rtk_rg_err_code_t rt_edp_flowMibCounter_reset(int index)
{
	return 0;

}

rtk_rg_err_code_t rt_edp_funcbasedMeter_set(rtk_rg_funcbasedMeterConf_t meterConf)
{
	return 0;

}

rtk_rg_err_code_t rt_edp_funcbasedMeter_get(rtk_rg_funcbasedMeterConf_t *meterConf)
{
	return 0;

}

rtk_rg_err_code_t rt_edp_flowHiPriEntry_add(rtk_rg_table_highPriPatten_t hiPriEntry,int *entry_idx)
{
	return 0;

}

rtk_rg_err_code_t rt_edp_flowHiPriEntry_del(int entry_idx)
{
	return 0;

}


rtk_rg_err_code_t rt_edp_callback_function_ptr_get(rtk_rg_callbackFunctionPtrGet_t *callback_function_ptr_get_info)
{

	return 0;
}

rtk_rg_err_code_t rt_edp_mac_filter_whitelist_add(rtk_rg_macFilterWhiteList_t *mac_filter_whitelist_info)
{

	return 0;
}

rtk_rg_err_code_t rt_edp_mac_filter_whitelist_del(rtk_rg_macFilterWhiteList_t *mac_filter_whitelist_info)
{

	return 0;
}

//1 To prevent the data structure of parameter of RG API is changed, use orginal RG data structure temporarily.
struct platform pf=
{
	.rtk_rg_api_module_init=rt_edp_api_module_init,
	.rtk_rg_driverVersion_get = rt_edp_driverVersion_get,
	.rtk_rg_initParam_get	=rt_edp_initParam_get,
	.rtk_rg_initParam_set	=rt_edp_initParam_set,
	.rtk_rg_lanInterface_add	=rt_edp_lanInterface_add,
//5
	.rtk_rg_wanInterface_add	=rt_edp_wanInterface_add,
	.rtk_rg_staticInfo_set	=rt_edp_staticInfo_set,
	.rtk_rg_dhcpRequest_set	=rt_edp_dhcpRequest_set,
	.rtk_rg_dhcpClientInfo_set	=rt_edp_dhcpClientInfo_set,
	.rtk_rg_pppoeClientInfoBeforeDial_set	=rt_edp_pppoeClientInfoBeforeDial_set,
//10
	.rtk_rg_pppoeClientInfoAfterDial_set	=rt_edp_pppoeClientInfoAfterDial_set,
	.rtk_rg_interface_del	=rt_edp_interface_del,
	.rtk_rg_intfInfo_find	=rt_edp_intfInfo_find,
	.rtk_rg_cvlan_add	=rt_edp_cvlan_add,
	.rtk_rg_cvlan_del	=rt_edp_cvlan_del,
//15
	.rtk_rg_cvlan_get=rt_edp_cvlan_get,
	.rtk_rg_vlanBinding_add	=rt_edp_vlanBinding_add,
	.rtk_rg_vlanBinding_del	=rt_edp_vlanBinding_del,
	.rtk_rg_vlanBinding_find	=rt_edp_vlanBinding_find,
	.rtk_rg_algServerInLanAppsIpAddr_add	=rt_edp_algServerInLanAppsIpAddr_add,
//20
	.rtk_rg_algServerInLanAppsIpAddr_del	=rt_edp_algServerInLanAppsIpAddr_del,
	.rtk_rg_algApps_set	=rt_edp_algApps_set,
	.rtk_rg_algApps_get	=rt_edp_algApps_get,
	.rtk_rg_dmzHost_set	=rt_edp_dmzHost_set,
	.rtk_rg_dmzHost_get	=rt_edp_dmzHost_get,
//25
	.rtk_rg_virtualServer_add	=rt_edp_virtualServer_add,
	.rtk_rg_virtualServer_del	=rt_edp_virtualServer_del,
	.rtk_rg_virtualServer_find	=rt_edp_virtualServer_find,
	.rtk_rg_aclFilterAndQos_add	=rt_edp_aclFilterAndQos_add,
	.rtk_rg_aclFilterAndQos_del	=rt_edp_aclFilterAndQos_del,
//30
	.rtk_rg_aclFilterAndQos_find	=rt_edp_aclFilterAndQos_find,
	.rtk_rg_macFilter_add	=rt_edp_macFilter_add,
	.rtk_rg_macFilter_del	=rt_edp_macFilter_del,
	.rtk_rg_macFilter_find	=rt_edp_macFilter_find,
	.rtk_rg_mac_filter_whitelist_add =rt_edp_mac_filter_whitelist_add,
//35
	.rtk_rg_mac_filter_whitelist_del =rt_edp_mac_filter_whitelist_del,
	.rtk_rg_urlFilterString_add	=rt_edp_urlFilterString_add,
	.rtk_rg_urlFilterString_del	=rt_edp_urlFilterString_del,
	.rtk_rg_urlFilterString_find	=rt_edp_urlFilterString_find,
	.rtk_rg_upnpConnection_add	=rt_edp_upnpConnection_add,
//40
	.rtk_rg_upnpConnection_del	=rt_edp_upnpConnection_del,
	.rtk_rg_upnpConnection_find	=rt_edp_upnpConnection_find,
	.rtk_rg_naptConnection_add	=rt_edp_naptConnection_add,
	.rtk_rg_naptConnection_del	=rt_edp_naptConnection_del,
	.rtk_rg_naptConnection_find	=rt_edp_naptConnection_find,
//45
	.rtk_rg_multicastFlow_add	=rt_edp_multicastFlow_add,
	.rtk_rg_multicastFlow_del	=rt_edp_multicastFlow_del,
	/* martin zhu add */
	.rtk_rg_l2MultiCastFlow_add =rt_edp_l2MultiCastFlow_add,
	.rtk_rg_multicastFlow_find	=rt_edp_multicastFlow_find,
	.rtk_rg_macEntry_add	=rt_edp_macEntry_add,
//50
	.rtk_rg_macEntry_del	=rt_edp_macEntry_del,
	.rtk_rg_macEntry_find	=rt_edp_macEntry_find,
	.rtk_rg_arpEntry_add	=rt_edp_arpEntry_add,
	.rtk_rg_arpEntry_del	=rt_edp_arpEntry_del,
	.rtk_rg_arpEntry_find	=rt_edp_arpEntry_find,
//55
	.rtk_rg_neighborEntry_add	=rt_edp_neighborEntry_add,
	.rtk_rg_neighborEntry_del	=rt_edp_neighborEntry_del,
	.rtk_rg_neighborEntry_find	=rt_edp_neighborEntry_find,
	.rtk_rg_accessWanLimit_set	=rt_edp_accessWanLimit_set,
	.rtk_rg_accessWanLimit_get	=rt_edp_accessWanLimit_get,
//60
	.rtk_rg_accessWanLimitCategory_set	=rt_edp_accessWanLimitCategory_set,
	.rtk_rg_accessWanLimitCategory_get	=rt_edp_accessWanLimitCategory_get,
	.rtk_rg_softwareSourceAddrLearningLimit_set	=rt_edp_softwareSourceAddrLearningLimit_set,
	.rtk_rg_softwareSourceAddrLearningLimit_get	=rt_edp_softwareSourceAddrLearningLimit_get,
	.rtk_rg_dosPortMaskEnable_set	=rt_edp_dosPortMaskEnable_set,
//65
	.rtk_rg_dosPortMaskEnable_get	=rt_edp_dosPortMaskEnable_get,
	.rtk_rg_dosType_set	=rt_edp_dosType_set,
	.rtk_rg_dosType_get	=rt_edp_dosType_get,
	.rtk_rg_dosFloodType_set	=rt_edp_dosFloodType_set,
	.rtk_rg_dosFloodType_get	=rt_edp_dosFloodType_get,
//70
	.rtk_rg_portMirror_set	=rt_edp_portMirror_set,
	.rtk_rg_portMirror_get	=rt_edp_portMirror_get,
	.rtk_rg_portMirror_clear	=rt_edp_portMirror_clear,
	.rtk_rg_portEgrBandwidthCtrlRate_set	=rt_edp_portEgrBandwidthCtrlRate_set,
	.rtk_rg_portIgrBandwidthCtrlRate_set	=rt_edp_portIgrBandwidthCtrlRate_set,
//75
	.rtk_rg_portEgrBandwidthCtrlRate_get	=rt_edp_portEgrBandwidthCtrlRate_get,
	.rtk_rg_portIgrBandwidthCtrlRate_get	=rt_edp_portIgrBandwidthCtrlRate_get,
	.rtk_rg_phyPortForceAbility_set	=rt_edp_phyPortForceAbility_set,
	.rtk_rg_phyPortForceAbility_get	=rt_edp_phyPortForceAbility_get,
	.rtk_rg_cpuPortForceTrafficCtrl_set	=rt_edp_cpuPortForceTrafficCtrl_set,
//80
	.rtk_rg_cpuPortForceTrafficCtrl_get	=rt_edp_cpuPortForceTrafficCtrl_get,
	.rtk_rg_portMibInfo_get	=rt_edp_portMibInfo_get,
	.rtk_rg_portMibInfo_clear	=rt_edp_portMibInfo_clear,
	.rtk_rg_stormControl_add	=rt_edp_stormControl_add,
	.rtk_rg_stormControl_del	=rt_edp_stormControl_del,
//85
	.rtk_rg_stormControl_find	=rt_edp_stormControl_find,
	.rtk_rg_shareMeter_set	=rt_edp_shareMeter_set,
	.rtk_rg_shareMeter_get	=rt_edp_shareMeter_get,
	.rtk_rg_shareMeterMode_set =rt_edp_shareMeterMode_set,
	.rtk_rg_shareMeterMode_get =rt_edp_shareMeterMode_get,
//90
	.rtk_rg_qosStrictPriorityOrWeightFairQueue_set	=rt_edp_qosStrictPriorityOrWeightFairQueue_set,
	.rtk_rg_qosStrictPriorityOrWeightFairQueue_get	=rt_edp_qosStrictPriorityOrWeightFairQueue_get,
	.rtk_rg_qosInternalPriMapToQueueId_set	=rt_edp_qosInternalPriMapToQueueId_set,
	.rtk_rg_qosInternalPriMapToQueueId_get	=rt_edp_qosInternalPriMapToQueueId_get,
	.rtk_rg_qosInternalPriDecisionByWeight_set	=rt_edp_qosInternalPriDecisionByWeight_set,
//95
	.rtk_rg_qosInternalPriDecisionByWeight_get	=rt_edp_qosInternalPriDecisionByWeight_get,
	.rtk_rg_qosDscpRemapToInternalPri_set	=rt_edp_qosDscpRemapToInternalPri_set,
	.rtk_rg_qosDscpRemapToInternalPri_get	=rt_edp_qosDscpRemapToInternalPri_get,
	.rtk_rg_qosPortBasedPriority_set	=rt_edp_qosPortBasedPriority_set,
	.rtk_rg_qosPortBasedPriority_get	=rt_edp_qosPortBasedPriority_get,
//100
	.rtk_rg_qosDot1pPriRemapToInternalPri_set	=rt_edp_qosDot1pPriRemapToInternalPri_set,
	.rtk_rg_qosDot1pPriRemapToInternalPri_get	=rt_edp_qosDot1pPriRemapToInternalPri_get,
	.rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_set	=rt_edp_qosDscpRemarkEgressPortEnableAndSrcSelect_set,
	.rtk_rg_qosDscpRemarkEgressPortEnableAndSrcSelect_get	=rt_edp_qosDscpRemarkEgressPortEnableAndSrcSelect_get,
	.rtk_rg_qosDscpRemarkByInternalPri_set	=rt_edp_qosDscpRemarkByInternalPri_set,
//105
	.rtk_rg_qosDscpRemarkByInternalPri_get	=rt_edp_qosDscpRemarkByInternalPri_get,
	.rtk_rg_qosDscpRemarkByDscp_set	=rt_edp_qosDscpRemarkByDscp_set,
	.rtk_rg_qosDscpRemarkByDscp_get	=rt_edp_qosDscpRemarkByDscp_get,
	.rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_set	=rt_edp_qosDot1pPriRemarkByInternalPriEgressPortEnable_set,
	.rtk_rg_qosDot1pPriRemarkByInternalPriEgressPortEnable_get	=rt_edp_qosDot1pPriRemarkByInternalPriEgressPortEnable_get,
//110
	.rtk_rg_qosDot1pPriRemarkByInternalPri_set	=rt_edp_qosDot1pPriRemarkByInternalPri_set,
	.rtk_rg_qosDot1pPriRemarkByInternalPri_get	=rt_edp_qosDot1pPriRemarkByInternalPri_get,
	.rtk_rg_portBasedCVlanId_set	=rt_edp_portBasedCVlanId_set,
	.rtk_rg_portBasedCVlanId_get	=rt_edp_portBasedCVlanId_get,
	.rtk_rg_portStatus_get	=rt_edp_portStatus_get,
//115
#ifdef CONFIG_RG_NAPT_PORT_COLLISION_PREVENTION
	.rtk_rg_naptExtPortGet	=rt_edp_naptExtPortGet,
	.rtk_rg_naptExtPortFree	=rt_edp_naptExtPortFree,
#endif
	.rtk_rg_classifyEntry_add	=rt_edp_classifyEntry_add,
	.rtk_rg_classifyEntry_find	=rt_edp_classifyEntry_find,
	.rtk_rg_classifyEntry_del	=rt_edp_classifyEntry_del,
//120
	.rtk_rg_svlanTpid_get=rt_edp_svlanTpid_get,
	.rtk_rg_svlanTpid_set=rt_edp_svlanTpid_set,
	.rtk_rg_svlanServicePort_set=rt_edp_svlanServicePort_set,
	.rtk_rg_svlanServicePort_get=rt_edp_svlanServicePort_get,
	.rtk_rg_pppoeInterfaceIdleTime_get=rt_edp_pppoeInterfaceIdleTime_get,
//125
	.rtk_rg_gatewayServicePortRegister_add=rt_edp_gatewayServicePortRegister_add,
	.rtk_rg_gatewayServicePortRegister_del=rt_edp_gatewayServicePortRegister_del,
	.rtk_rg_gatewayServicePortRegister_find=rt_edp_gatewayServicePortRegister_find,
	.rtk_rg_wlanDevBasedCVlanId_set=rt_edp_wlanDevBasedCVlanId_set,
	.rtk_rg_wlanDevBasedCVlanId_get=rt_edp_wlanDevBasedCVlanId_get,
//130
	.rtk_rg_wlanSoftwareSourceAddrLearningLimit_set=rt_edp_wlanSoftwareSourceAddrLearningLimit_set,
	.rtk_rg_wlanSoftwareSourceAddrLearningLimit_get=rt_edp_wlanSoftwareSourceAddrLearningLimit_get,
	.rtk_rg_naptFilterAndQos_add=rt_edp_naptFilterAndQos_add,
	.rtk_rg_naptFilterAndQos_del=rt_edp_naptFilterAndQos_del,
	.rtk_rg_naptFilterAndQos_find=rt_edp_naptFilterAndQos_find,
//135
	.rtk_rg_pptpClientInfoBeforeDial_set=rt_edp_pptpClientInfoBeforeDial_set,
	.rtk_rg_pptpClientInfoAfterDial_set=rt_edp_pptpClientInfoAfterDial_set,
	.rtk_rg_l2tpClientInfoBeforeDial_set=rt_edp_l2tpClientInfoBeforeDial_set,
	.rtk_rg_l2tpClientInfoAfterDial_set=rt_edp_l2tpClientInfoAfterDial_set,
	.rtk_rg_stpBlockingPortmask_set=rt_edp_stpBlockingPortmask_set,
//140
	.rtk_rg_stpBlockingPortmask_get=rt_edp_stpBlockingPortmask_get,
	.rtk_rg_portIsolation_set=rt_edp_portIsolation_set,
	.rtk_rg_portIsolation_get=rt_edp_portIsolation_get,
	.rtk_rg_dsliteInfo_set=rt_edp_dsliteInfo_set,
	.rtk_rg_pppoeDsliteInfoBeforeDial_set=rt_edp_pppoeDsliteInfoBeforeDial_set,
//145
	.rtk_rg_pppoeDsliteInfoAfterDial_set=rt_edp_pppoeDsliteInfoAfterDial_set,
	.rtk_rg_gponDsBcFilterAndRemarking_add=rt_edp_gponDsBcFilterAndRemarking_add,
	.rtk_rg_gponDsBcFilterAndRemarking_del=rt_edp_gponDsBcFilterAndRemarking_del,
	.rtk_rg_gponDsBcFilterAndRemarking_find=rt_edp_gponDsBcFilterAndRemarking_find,
	.rtk_rg_gponDsBcFilterAndRemarking_del_all=rt_edp_gponDsBcFilterAndRemarking_del_all,
//150
	.rtk_rg_gponDsBcFilterAndRemarking_Enable=rt_edp_gponDsBcFilterAndRemarking_Enable,
	.rtk_rg_interfaceMibCounter_del=rt_edp_interfaceMibCounter_del,
	.rtk_rg_interfaceMibCounter_get=rt_edp_interfaceMibCounter_get,
	.rtk_rg_redirectHttpAll_set=rt_edp_redirectHttpAll_set,
	.rtk_rg_redirectHttpAll_get=rt_edp_redirectHttpAll_get,
//155
	.rtk_rg_redirectHttpURL_add=rt_edp_redirectHttpURL_add,
	.rtk_rg_redirectHttpURL_del=rt_edp_redirectHttpURL_del,
	.rtk_rg_redirectHttpWhiteList_add=rt_edp_redirectHttpWhiteList_add,
	.rtk_rg_redirectHttpWhiteList_del=rt_edp_redirectHttpWhiteList_del,
	.rtk_rg_redirectHttpRsp_set=rt_edp_redirectHttpRsp_set,
//160
	.rtk_rg_redirectHttpRsp_get=rt_edp_redirectHttpRsp_get,
	.rtk_rg_svlanTpid2_get= rt_edp_svlanTpid2_get,
	.rtk_rg_svlanTpid2_set= rt_edp_svlanTpid2_set,
	.rtk_rg_svlanTpid2_enable_get=rt_edp_svlanTpid2_enable_get,
	.rtk_rg_svlanTpid2_enable_set=rt_edp_svlanTpid2_enable_set,
//165
	.rtk_rg_hostPoliceControl_set=rt_edp_hostPoliceControl_set,
	.rtk_rg_hostPoliceControl_get=rt_edp_hostPoliceControl_get,
	.rtk_rg_hostPoliceLogging_get=rt_edp_hostPoliceLogging_get,
	.rtk_rg_hostPoliceLogging_del=rt_edp_hostPoliceLogging_del,
	.rtk_rg_redirectHttpCount_set=rt_edp_redirectHttpCount_set,
//170
	.rtk_rg_redirectHttpCount_get=rt_edp_redirectHttpCount_get,
	.rtk_rg_staticRoute_add=rt_edp_staticRoute_add,
	.rtk_rg_staticRoute_del=rt_edp_staticRoute_del,
	.rtk_rg_staticRoute_find=rt_edp_staticRoute_find,
	.rtk_rg_aclLogCounterControl_get=rt_edp_aclLogCounterControl_get,
//175
	.rtk_rg_aclLogCounterControl_set=rt_edp_aclLogCounterControl_set,
	.rtk_rg_aclLogCounter_get=rt_edp_aclLogCounter_get,
	.rtk_rg_aclLogCounter_reset=rt_edp_aclLogCounter_reset,
	.rtk_rg_groupMacLimit_get=rt_edp_groupMacLimit_get,
	.rtk_rg_groupMacLimit_set=rt_edp_groupMacLimit_set,
//180
	.rtk_rg_igmpMldSnoopingControl_set=rt_edp_igmpMldSnoopingControl_set,
	.rtk_rg_igmpMldSnoopingControl_get=rt_edp_igmpMldSnoopingControl_get,
	.rtk_rg_flowMibCounter_get=rt_edp_flowMibCounter_get,
	.rtk_rg_flowMibCounter_reset=rt_edp_flowMibCounter_reset,
	.rtk_rg_softwareIdleTime_set=rt_edp_softwareIdleTime_set,
//185
	.rtk_rg_softwareIdleTime_get=rt_edp_softwareIdleTime_get,
	.rtk_rg_funcbasedMeter_set=rt_edp_funcbasedMeter_set,
	.rtk_rg_funcbasedMeter_get=rt_edp_funcbasedMeter_get,
	.rtk_rg_flowHiPriEntry_add=rt_edp_flowHiPriEntry_add,
	.rtk_rg_flowHiPriEntry_del=rt_edp_flowHiPriEntry_del,
//190
	.rtk_rg_igmpMldSnoopingPortControl_add=rt_edp_igmpMldSnoopingPortControl_add,
	.rtk_rg_igmpMldSnoopingPortControl_del=rt_edp_igmpMldSnoopingPortControl_del,
	.rtk_rg_igmpMldSnoopingPortControl_find=rt_edp_igmpMldSnoopingPortControl_find,
	.rtk_rg_callback_function_ptr_get=rt_edp_callback_function_ptr_get,
//194
};



