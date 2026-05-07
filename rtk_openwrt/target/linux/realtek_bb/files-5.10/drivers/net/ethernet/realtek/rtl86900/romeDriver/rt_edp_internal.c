#include <linux/kmod.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/if_ether.h>
#include <common/rt_type.h>
#include <common/type.h>
#include <rt_edp_debug.h>
#include <rt_edp_struct.h>

static char cmd_buff[EDP_CB_CMD_BUFF_SIZE];
static char env_PATH[EDP_CB_CMD_BUFF_SIZE];
//static struct nf_hook_ops nfho;


int _rt_edp_pipe_cmd(const char *comment, ...) 
{
	char * envp[]={	//element size 3
		"HOME=/",
		env_PATH,
		NULL
	};
	char * argv[]={ //element size 4
		"/bin/bash",
		"-c",
		cmd_buff,
		NULL
	};
	int retval;
	va_list argList;
	va_start(argList, comment);
	//snprintf( env_PATH, EDP_CB_CMD_BUFF_SIZE, "PATH=%s", CONFIG_RG_CALLBACK_ENVIRONMENT_VARIABLE_PATH);
	snprintf( env_PATH, EDP_CB_CMD_BUFF_SIZE, "PATH=%s", "/sbin:/usr/sbin:/bin:/usr/bin");
	vsprintf(cmd_buff, comment, argList);
	CMD("[%s]\n",cmd_buff);
	retval=call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);	
	va_end(argList);
	if(retval != 0)
		WARNING("[CMD][%s] ret=%d", cmd_buff, retval);
	return retval;
}

void _rt_edp_checkVlanBindingAndAddVconfig(rt_edp_port_idx_t portIdx, int vlanId)
{
	int i,lanTotalNum;
	rtk_mac_t gmac;
	rt_edp_portmask_t portmask;
	
	//RGDB_LOCK
	lanTotalNum=rt_edp_db.systemGlobal.lanIntfTotalNum;
	//RGDB_UNLOCK
	for(i=0; i<lanTotalNum; i++)
	{
		//RGDB_LOCK
		portmask.portmask = rt_edp_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->port_mask.portmask;
		memcpy(&gmac, &rt_edp_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac, sizeof(rtk_mac_t));
		//RGDB_UNLOCK
		if(portmask.portmask&(0x1<<portIdx))
		{
#if defined(CONFIG_RTL_MULTI_LAN_DEV)
			if(portIdx >= RT_EDP_PORT_LASTCPU)
			{
				//add virtual ether device for VLAN
				_rt_edp_pipe_cmd("vconfig add %s %d", EDP_LAN_DEVICE_NAME, vlanId);
				_rt_edp_pipe_cmd("%s link set %s.%d address %02x:%02x:%02x:%02x:%02x:%02x", EDP_IP_UTILITY, EDP_LAN_DEVICE_NAME, vlanId,
					gmac.octet[0],
					gmac.octet[1],
					gmac.octet[2],
					gmac.octet[3],
					gmac.octet[4],
					gmac.octet[5]);
				
				_rt_edp_pipe_cmd("%s link set %s.%d up", EDP_IP_UTILITY, EDP_LAN_DEVICE_NAME, vlanId);
				_rt_edp_pipe_cmd("brctl addif br0 %s.%d", EDP_LAN_DEVICE_NAME, vlanId);
				_rt_edp_pipe_cmd("ebtables -t broute -A BROUTING -i %s -p 802_1Q -j DROP", EDP_LAN_DEVICE_NAME);
			}
			else
			{
				//add virtual ether device for VLAN
				_rt_edp_pipe_cmd("vconfig add %s.%d %d", EDP_LAN_DEVICE_START_INDEX+portIdx, EDP_LAN_DEVICE_NAME, vlanId);
				_rt_edp_pipe_cmd("%s link set %s.%d.%d address %02x:%02x:%02x:%02x:%02x:%02x", EDP_IP_UTILITY, EDP_LAN_DEVICE_NAME, 2+portIdx, vlanId,
					gmac.octet[0],
					gmac.octet[1],
					gmac.octet[2],
					gmac.octet[3],
					gmac.octet[4],
					gmac.octet[5]);
				
				_rt_edp_pipe_cmd("%s link set %s.%d.%d up", EDP_IP_UTILITY, EDP_LAN_DEVICE_NAME, EDP_LAN_DEVICE_START_INDEX+portIdx, vlanId);
				_rt_edp_pipe_cmd("brctl addif br0 %s.%d.%d", EDP_LAN_DEVICE_NAME, EDP_LAN_DEVICE_START_INDEX+portIdx, vlanId);
				_rt_edp_pipe_cmd("ebtables -t broute -A BROUTING -i eth0.%d -p 802_1Q -j DROP", EDP_LAN_DEVICE_START_INDEX+portIdx);
			}
#else
			//add virtual ether device for VLAN
			_rt_edp_pipe_cmd("vconfig add %s %d", EDP_LAN_DEVICE_NAME, vlanId);
			_rt_edp_pipe_cmd("%s link set %s.%d address %02x:%02x:%02x:%02x:%02x:%02x", EDP_IP_UTILITY, EDP_LAN_DEVICE_NAME, vlanId,
				gmac.octet[0],
				gmac.octet[1],
				gmac.octet[2],
				gmac.octet[3],
				gmac.octet[4],
				gmac.octet[5]);
			
			_rt_edp_pipe_cmd("%s link set %s.%d up", EDP_IP_UTILITY, EDP_LAN_DEVICE_NAME, vlanId);
			_rt_edp_pipe_cmd("brctl addif br0 %s.%d", EDP_LAN_DEVICE_NAME, vlanId);
			_rt_edp_pipe_cmd("ebtables -t broute -A BROUTING -i %s -p 802_1Q -j DROP", EDP_LAN_DEVICE_NAME);
#endif
			break;
		}
	}
}

int8 *_rt_edp_inet_mactoa_p(const uint8 *mac, int8 *p)
{
	assert(p!=NULL);

	sprintf(p, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

	return p;
}

static inline int8 *_ui8tod( uint8 n, int8 *p )
{
	if( n > 99 ) *p++ = (n/100) + '0';
	if( n >  9 ) *p++ = ((n/10)%10) + '0';
	*p++ = (n%10) + '0';
	return p;
}

int8 *_rt_edp_inet_ntoa(rtk_ip_addr_t ina)
{
	static int8 buf[4*sizeof "123"];
	int8 *p = buf;
	uint8 *ucp = (unsigned char *)&ina;
#ifdef __LITTLE_ENDIAN
	p = _ui8tod( ucp[3] & 0xFF, p);
	*p++ = '.';
	p = _ui8tod( ucp[2] & 0xFF, p);
	*p++ = '.';
	p = _ui8tod( ucp[1] & 0xFF, p);
	*p++ = '.';
	p = _ui8tod( ucp[0] & 0xFF, p);
	*p++ = '\0';
#else
	p = _ui8tod( ucp[0] & 0xFF, p);
	*p++ = '.';
	p = _ui8tod( ucp[1] & 0xFF, p);
	*p++ = '.';
	p = _ui8tod( ucp[2] & 0xFF, p);
	*p++ = '.';
	p = _ui8tod( ucp[3] & 0xFF, p);
	*p++ = '\0';
#endif
	return (buf);
}

int8 *_rt_edp_inet_ntoa_p(rtk_ip_addr_t ina, int8 *p)
{
	assert(p!=NULL);

#ifdef __LITTLE_ENDIAN
	sprintf(p, "%d.%d.%d.%d", (ina&0xff), ((ina>>8)&0xff), ((ina>>16)&0xff), ((ina>>24)&0xff));
#else
	sprintf(p, "%d.%d.%d.%d", ((ina>>24)&0xff), ((ina>>16)&0xff), ((ina>>8)&0xff), (ina&0xff));
#endif

	return (p);
}

/*IPv6 address to string*/
int8 *_rt_edp_inet_n6toa(const uint8 *ipv6)
{
#define RT_EDP_IPV6_TMP_BUFFER_LENGTH 8

	static int8 buf[8*sizeof "FFFF:"];
    uint32  i;
    uint16  ipv6_ptr[RT_EDP_IPV6_TMP_BUFFER_LENGTH] = {0};

    for (i = 0; i < RT_EDP_IPV6_TMP_BUFFER_LENGTH ;i++)
    {
        ipv6_ptr[i] = ipv6[i*2+1];
        ipv6_ptr[i] |=  ipv6[i*2] << 8;
    }

    sprintf(buf, "%x:%x:%x:%x:%x:%x:%x:%x", ipv6_ptr[0], ipv6_ptr[1], ipv6_ptr[2], ipv6_ptr[3]
    , ipv6_ptr[4], ipv6_ptr[5], ipv6_ptr[6], ipv6_ptr[7]);
	return (buf);
}

int8 *_rt_edp_inet_n6toa_p(const uint8 *ipv6, int8 *p)
{
#define RT_EDP_IPV6_TMP_BUFFER_LENGTH 8

    uint32  i;
    uint16  ipv6_ptr[RT_EDP_IPV6_TMP_BUFFER_LENGTH] = {0};
	assert(p!=NULL);

    for (i = 0; i < RT_EDP_IPV6_TMP_BUFFER_LENGTH ;i++)
    {
        ipv6_ptr[i] = ipv6[i*2+1];
        ipv6_ptr[i] |=  ipv6[i*2] << 8;
    }

    sprintf(p, "%x:%x:%x:%x:%x:%x:%x:%x", ipv6_ptr[0], ipv6_ptr[1], ipv6_ptr[2], ipv6_ptr[3]
    , ipv6_ptr[4], ipv6_ptr[5], ipv6_ptr[6], ipv6_ptr[7]);
	return (p);
}

void _rt_edp_dump_stack(void)
{
#if 0
	struct pt_regs regs;
	unsigned long sp,ra,pc;
//	prepare_frametrace(&regs);

    memset(&regs, 0, sizeof(regs));

	__asm__ __volatile__(
	".set push\n\t"
	".set noat\n\t"
	"1: la $1, 1b\n\t"
	"sw $1, %0\n\t"
	"sw $29, %1\n\t"
	"sw $31, %2\n\t"
	".set pop\n\t"
	: "=m" (regs.cp0_epc),
	"=m" (regs.regs[29]), "=m" (regs.regs[31])
	: : "memory");

	
	sp = regs.regs[29];
	ra = regs.regs[31];
	pc = regs.cp0_epc;
	
	if (!__kernel_text_address(pc)) 
	{
		return;
	}	
	
	rtlglue_printf("\033[1;33;41m");
	pc = unwind_stack(current, &sp, pc, &ra);
	while(1)
	{
		if(!pc) break;
		pc = unwind_stack(current, &sp, pc, &ra);
		if(!pc) break;
		rtlglue_printf("[%p][%pS]\n", (void *)pc, (void *)pc);
		//printk("[%p:%pS]\n", (void *) pc, (void *) pc);		
	}
	rtlglue_printf("\033[0m\n");
#endif
	return;
}

void _rt_edp_error_id_mapping(int id, char *mappingName)
{
	switch(id)
	{
		case RT_EDP_ERR_ADD_ARP_MAC_FAILED: strcpy(mappingName,"RT_EDP_ERR_ADD_ARP_MAC_FAILED"); break;
		case RT_EDP_ERR_ARP_NOT_FOUND: strcpy(mappingName,"RT_EDP_ERR_ARP_NOT_FOUND"); break;
		case RT_EDP_ERR_CHIP_NOT_SUPPORT: strcpy(mappingName,"RT_EDP_ERR_CHIP_NOT_SUPPORT"); break;
		case RT_EDP_ERR_CPU_TAG_DIFF_BRIDGE_WAN: strcpy(mappingName,"RT_EDP_ERR_CPU_TAG_DIFF_BRIDGE_WAN"); break;
		case RT_EDP_ERR_CREATE_GATEWAY_LUT_FAIL: strcpy(mappingName,"RT_EDP_ERR_CREATE_GATEWAY_LUT_FAIL"); break;
		case RT_EDP_ERR_DELETE_GATEWAY_LUT_FAIL: strcpy(mappingName,"RT_EDP_ERR_DELETE_GATEWAY_LUT_FAIL"); break;
		case RT_EDP_ERR_INTF_SET_FAIL: strcpy(mappingName,"RT_EDP_ERR_INTF_SET_FAIL"); break;
		case RT_EDP_ERR_INVALID_PARAM: strcpy(mappingName,"RT_EDP_ERR_INVALID_PARAM"); break;
		case RT_EDP_ERR_NXP_SET_FAIL: strcpy(mappingName,"RT_EDP_ERR_NXP_SET_FAIL"); break;
		case RT_EDP_ERR_PPPOE_SET_FAIL: strcpy(mappingName,"RT_EDP_ERR_PPPOE_SET_FAIL"); break;
		case RT_EDP_ERR_PORT_BIND_SET_FAIL: strcpy(mappingName,"RT_EDP_ERR_PORT_BIND_SET_FAIL"); break;
		case RT_EDP_ERR_ROUTE_SET_FAIL: strcpy(mappingName,"RT_EDP_ERR_ROUTE_SET_FAIL"); break;
		case RT_EDP_ERR_WANTYPE_SET_FAIL: strcpy(mappingName,"RT_EDP_ERR_WANTYPE_SET_FAIL"); break;
		case RT_EDP_ERR_VLAN_SET_FAIL: strcpy(mappingName,"RT_EDP_ERR_VLAN_SET_FAIL"); break;
		case RT_EDP_ERR_VLAN_PRI_CONFLICT_WIFI: strcpy(mappingName,"RT_EDP_ERR_VLAN_PRI_CONFLICT_WIFI"); break;
		default: strcpy(mappingName,"Please refer to rt_edp_error.h!"); break;
	}

	return;
}

rt_edp_err_code_t _rt_edp_return_err_mapping(int err_id, char *err_name,int line)
{
	char StringErrName[64]={0};
	
	if(err_id!=RT_EDP_ERR_OK) 
	{			
		if(memcmp(err_name, "RT_EDP_ERR_", 10)==0)
		{
			strcpy(StringErrName, err_name);
		}
		else
		{
			_rt_edp_error_id_mapping(err_id, StringErrName);
		}
		WARNING("Return Error (0x%x:%s) at line:%d", err_id, StringErrName, line);
		if(rt_edp_kernel.debug_level&RT_EDP_DEBUG_LEVEL_WARN)
		{			
			_rt_edp_dump_stack();			
		}
	}
	return err_id;
}

rt_edp_err_code_t _rt_edp_freeWanDevIdx_get(rt_edp_wan_dev_type_t wanDevType, int *wanDevIdx)
{
	int i;

	if(wanDevType<0 || wanDevType>=RT_EDP_WAN_DEV_TYPE_END)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);
	if(wanDevIdx==NULL)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_NULL_POINTER);
	
	for(i=0; i<EDP_MAX_NETIF_SW_TABLE_SIZE; i++)
	{
		if(rt_edp_db.systemGlobal.wanDevUsed[wanDevType][i]==0)
		{
			rt_edp_db.systemGlobal.wanDevUsed[wanDevType][i] = 1;
			*wanDevIdx = i;
			return RT_EDP_ERR_OK;
		}
	}

	*wanDevIdx = FAIL;
	return RT_EDP_ERR_ENTRY_FULL;
}

rt_edp_err_code_t _rt_edp_freeWanDevIdx_put(rt_edp_wan_dev_type_t wanDevType, int wanDevIdx)
{
	if(wanDevType<0 || wanDevType>=RT_EDP_WAN_DEV_TYPE_END)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);
	if(wanDevIdx<0 || wanDevIdx>=EDP_MAX_NETIF_SW_TABLE_SIZE)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);

	if(rt_edp_db.systemGlobal.wanDevUsed[wanDevType][wanDevIdx]==1)
		rt_edp_db.systemGlobal.wanDevUsed[wanDevType][wanDevIdx] = 0;
	else
	{
		if(wanDevType==RT_EDP_WAN_DEV_TYPE_NAS)
			WARNING("%s_%d is not used", EDP_WAN_DEVICE_NAME, wanDevIdx);
		else if(wanDevType==RT_EDP_WAN_DEV_TYPE_PPP)
			WARNING("%s%d is not used", EDP_WAN_TUNNEL_DEVICE_NAME, wanDevIdx);
		return RT_EDP_ERR_INVALID_PARAM;
	}
	
	return RT_EDP_ERR_OK;
}

rt_edp_err_code_t _rt_edp_internal_wanSet(int wan_intf_idx, rt_edp_ipStaticInfo_t *static_info)
{
	int i;
	ipaddr_t ip_domain = static_info->ip_addr&static_info->ip_network_mask;
	char devName[IFNAMSIZ]; //device name, ex:nas0_0
	char tunnelDevName[IFNAMSIZ];
	char gwDevName[IFNAMSIZ];
	rtk_mac_t zeroMAC;
	rtk_ipv6_addr_t zeroV6Addr;

	if(rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].valid == RT_EDP_IF_INVALID_ENTRY)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_ENTRY_NOT_EXIST);
	
	memset(&zeroMAC, 0, sizeof(rtk_mac_t));
	memset(&zeroV6Addr, 0, sizeof(rtk_ipv6_addr_t));

	memcpy(devName, rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf.devName, IFNAMSIZ);
	memcpy(tunnelDevName, rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf.tunnelDevName, IFNAMSIZ);

	//setup MTU
	_rt_edp_pipe_cmd("%s link set %s mtu %d", EDP_IP_UTILITY, devName, static_info->mtu);
		
	//TCP MSS clamping
	if(static_info->mtu < RT_EDP_PPPoE_MTU && rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type == RT_EDP_PPPoE)
	{
		_rt_edp_pipe_cmd("iptables -D FORWARD -p tcp -o ppp+ --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu");
		_rt_edp_pipe_cmd("iptables -D FORWARD -p tcp -i ppp+ --tcp-flags SYN,RST SYN -m tcpmss --mss %d:1536 -j TCPMSS --set-mss %d",static_info->mtu-40,static_info->mtu-40);
		_rt_edp_pipe_cmd("iptables -A FORWARD -p tcp -o ppp+ --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu");
		_rt_edp_pipe_cmd("iptables -A FORWARD -p tcp -i ppp+ --tcp-flags SYN,RST SYN -m tcpmss --mss %d:1536 -j TCPMSS --set-mss %d",static_info->mtu-40,static_info->mtu-40);
	}
	else if(static_info->mtu < RT_EDP_ETH_MTU && (rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type == RT_EDP_STATIC
					|| rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type == RT_EDP_DHCP))
	{
		_rt_edp_pipe_cmd("iptables -D FORWARD -p tcp -o %s --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu",devName);
		_rt_edp_pipe_cmd("iptables -D FORWARD -p tcp -i %s --tcp-flags SYN,RST SYN -m tcpmss --mss %d:1536 -j TCPMSS --set-mss %d",devName,static_info->mtu-40,static_info->mtu-40);
		_rt_edp_pipe_cmd("iptables -A FORWARD -p tcp -o %s --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu",devName);
		_rt_edp_pipe_cmd("iptables -A FORWARD -p tcp -i %s --tcp-flags SYN,RST SYN -m tcpmss --mss %d:1536 -j TCPMSS --set-mss %d",devName,static_info->mtu-40,static_info->mtu-40);
	}

	if(rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf.wan_intf_conf.port_binding_mask.portmask)
	{
		// Set binding
		for(i=0; i<RT_EDP_MAC_PORT_MAX; i++)
		{
			// skip non-lan port
			if(((0x1<<i) & RT_EDP_ALL_LAN_PORTMASK)==0x0
				|| ((0x1<<i) & rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf.wan_intf_conf.port_binding_mask.portmask)==0x0)
				continue;
			_rt_edp_pipe_cmd("iptables -t mangle -D PREROUTING -m physdev --physdev-in %s -j MARK --set-mark %s", rt_edp_db.dev_port_map[i].dev_name, rt_edp_db.wan_intf_mark[wan_intf_idx]);
			_rt_edp_pipe_cmd("iptables -t mangle -A PREROUTING -m physdev --physdev-in %s -j MARK --set-mark %s", rt_edp_db.dev_port_map[i].dev_name, rt_edp_db.wan_intf_mark[wan_intf_idx]);
		}
	}

	/***IPv4 Setting***/
	if(static_info->ip_version == RT_EDP_IPVER_V4ONLY || static_info->ip_version == RT_EDP_IPVER_V4V6)
	{
		//enable forwarding state
		_rt_edp_pipe_cmd("echo 1 > /proc/sys/net/ipv4/ip_forward");

		if(!strlen(tunnelDevName))
		{
			//Before add address, forced delete once!(for change IP at same interface)
			_rt_edp_pipe_cmd("%s -4 addr flush dev %s", EDP_IP_UTILITY, devName);
			_rt_edp_pipe_cmd("%s -4 route flush dev %s", EDP_IP_UTILITY, devName);
			if(static_info->ip_version==RT_EDP_IPVER_V4ONLY)
			{
				_rt_edp_pipe_cmd("%s -6 addr flush dev %s", EDP_IP_UTILITY, devName);
				_rt_edp_pipe_cmd("%s -6 route flush dev %s", EDP_IP_UTILITY, devName);
			}
		}
		switch(rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type)
		{
			case RT_EDP_STATIC:
			case RT_EDP_DHCP:
			case RT_EDP_DSLITE:
				if(static_info->ip_addr!=0)
					_rt_edp_pipe_cmd("%s addr add %pI4/%pI4 broadcast + dev %s", EDP_IP_UTILITY, &static_info->ip_addr, &static_info->ip_network_mask, devName);
				if(static_info->gateway_ipv4_addr!=0)
				{
					ipaddr_t tmp_ip_mask, longest_gw_ip_mask = ((static_info->ip_addr & static_info->ip_network_mask)==(static_info->gateway_ipv4_addr & static_info->ip_network_mask)) ? static_info->ip_network_mask : 0;

					if(longest_gw_ip_mask!=0)
						memcpy(gwDevName, devName, IFNAMSIZ);
					else
						memset(gwDevName, 0, IFNAMSIZ);
					for(i=0; i<rt_edp_db.systemGlobal.wanIntfTotalNum; i++)
					{
						if(rt_edp_db.systemGlobal.wanIntfGroup[i].p_intfInfo->p_wanStaticInfo->ip_network_mask == 0)
							continue;
						tmp_ip_mask = rt_edp_db.systemGlobal.wanIntfGroup[i].p_intfInfo->p_wanStaticInfo->ip_network_mask;
						if(((rt_edp_db.systemGlobal.wanIntfGroup[i].p_intfInfo->p_wanStaticInfo->ip_addr & tmp_ip_mask)==(static_info->gateway_ipv4_addr & tmp_ip_mask))
							&& (tmp_ip_mask>longest_gw_ip_mask))
						{
							longest_gw_ip_mask = tmp_ip_mask;
							memcpy(gwDevName, rt_edp_db.systemGlobal.wanIntfGroup[i].p_intfInfo->storedInfo.wan_intf.devName, IFNAMSIZ);	
						}
					}
					if(longest_gw_ip_mask==0)
						WARNING("Gateway %pI4 is unreachable by using route table.", static_info->gateway_ipv4_addr);
						
					if(static_info->ipv4_default_gateway_on)
					{
						_rt_edp_pipe_cmd("%s route add default via %pI4 dev %s mtu %d", EDP_IP_UTILITY, &static_info->gateway_ipv4_addr, gwDevName, static_info->mtu);
					}
					else
					{
						_rt_edp_pipe_cmd("%s route del %pI4/%pI4", EDP_IP_UTILITY, &ip_domain, &static_info->ip_network_mask);
						if(longest_gw_ip_mask==static_info->ip_network_mask)
							_rt_edp_pipe_cmd("%s route add %pI4/32 dev %s", EDP_IP_UTILITY, &static_info->gateway_ipv4_addr, devName);
						_rt_edp_pipe_cmd("%s route add %pI4/%pI4 via %pI4", EDP_IP_UTILITY, &ip_domain, &static_info->ip_network_mask, &static_info->gateway_ipv4_addr);
					}
					if(static_info->gw_mac_auto_learn_for_ipv4==0 && memcmp(&static_info->gateway_mac_addr_for_ipv4, &zeroMAC, sizeof(rtk_mac_t)))
						_rt_edp_pipe_cmd("%s neigh add %pI4 dev %s lladdr %pM", EDP_IP_UTILITY, &static_info->gateway_ipv4_addr, gwDevName, &static_info->gateway_mac_addr_for_ipv4.octet[0]);
					else
						_rt_edp_pipe_cmd("%s neigh add %pI4 dev %s lladdr %pM nud incomplete", EDP_IP_UTILITY, &static_info->gateway_ipv4_addr, gwDevName, &zeroMAC.octet[0]);
				}
				// For policy route/binding
				if(static_info->ip_addr!=0)
				{
					_rt_edp_pipe_cmd("%s route del %pI4/%pI4 table 0x%x", EDP_IP_UTILITY, &ip_domain, &static_info->ip_network_mask, RT_EDP_POLICY_ROUTE_TABLE_START+wan_intf_idx);
					_rt_edp_pipe_cmd("%s route del default table 0x%x", EDP_IP_UTILITY, RT_EDP_POLICY_ROUTE_TABLE_START+wan_intf_idx);
					_rt_edp_pipe_cmd("%s route add %pI4/%pI4 dev %s table 0x%x", EDP_IP_UTILITY, &ip_domain, &static_info->ip_network_mask, devName, RT_EDP_POLICY_ROUTE_TABLE_START+wan_intf_idx);
					_rt_edp_pipe_cmd("%s route add default dev %s table 0x%x", EDP_IP_UTILITY, devName, RT_EDP_POLICY_ROUTE_TABLE_START+wan_intf_idx);
				}
				// SNAT
				_rt_edp_pipe_cmd("iptables -t nat -D POSTROUTING -o %s -j MASQUERADE", devName);
				if(static_info->napt_enable && static_info->ip_addr!=0) //add NAPT in iptables, if routing no need this
				{
					_rt_edp_pipe_cmd("iptables -t nat -A POSTROUTING -o %s -j MASQUERADE", devName);
				}
				break;
			case RT_EDP_PPTP:
			case RT_EDP_L2TP:
				if(static_info->ipv4_default_gateway_on)
				{
					_rt_edp_pipe_cmd("%s route add default via %pI4 dev %s mtu %d", EDP_IP_UTILITY, &static_info->gateway_ipv4_addr, tunnelDevName, static_info->mtu);
				}	
				break;
			case RT_EDP_PPPoE:
			case RT_EDP_PPPoE_DSLITE:
			default:
				break;
		}	
	}

	/***IPv6 Setting***/
	if(static_info->ip_version==RT_EDP_IPVER_V6ONLY || static_info->ip_version==RT_EDP_IPVER_V4V6)
	{
		//Turn on IPv6 at /proc/sys/net
		_rt_edp_pipe_cmd("echo 0 >/proc/sys/net/ipv6/conf/%s/disable_ipv6", devName);
		
		//For setup IPv6 in protocol stack, the MTU need to be bigger than 1280!!
		if(static_info->mtu<1280)
			WARNING("For IPv6, the smallest MTU is 1280, otherwise IPv6 address and route could not be added.");

		if(!strlen(tunnelDevName))
		{
			//Before add address, forced delete once!(for change IP at same interface)
			_rt_edp_pipe_cmd("%s -6 addr flush dev %s", EDP_IP_UTILITY, devName);
			_rt_edp_pipe_cmd("%s -6 route flush dev %s", EDP_IP_UTILITY, devName);
			if(static_info->ip_version==RT_EDP_IPVER_V6ONLY)
			{
				_rt_edp_pipe_cmd("%s -4 addr flush dev %s", EDP_IP_UTILITY, devName);
				_rt_edp_pipe_cmd("%s -4 route flush dev %s", EDP_IP_UTILITY, devName);
			}
		}
		
		if(rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type==RT_EDP_PPPoE)
		{
			//NAT set by _rtk_rg_pppoeBeforeDiagByHwCallBack
		}
		else
		{
			_rt_edp_pipe_cmd("echo 1 > /proc/sys/net/ipv6/conf/all/forwarding");
			//Support ipv6_static only!!!
			if(memcmp(&static_info->ipv6_addr, &zeroV6Addr, sizeof(rtk_ipv6_addr_t)))
				_rt_edp_pipe_cmd("%s -6 addr add %s/%d dev %s", EDP_IP_UTILITY, _rt_edp_inet_n6toa(static_info->ipv6_addr.ipv6_addr), static_info->ipv6_mask_length, devName);

			if(memcmp(&static_info->gateway_ipv6_addr, &zeroV6Addr, sizeof(rtk_ipv6_addr_t)))
			{
				rtk_ipv6_addr_t tmp_ipv6_addr;
				uint32 tmp_ip_len, longest_gw_ip_len=0;
				uint16 idx, bitMask;

				for(i=-1; i<(int)rt_edp_db.systemGlobal.wanIntfTotalNum; i++)
				{
					memcpy(tmp_ipv6_addr.ipv6_addr, (i==-1) ? static_info->ipv6_addr.ipv6_addr : rt_edp_db.systemGlobal.wanIntfGroup[i].p_intfInfo->p_wanStaticInfo->ipv6_addr.ipv6_addr, IPV6_ADDR_LEN);
					tmp_ip_len = (i==-1) ? static_info->ipv6_mask_length : rt_edp_db.systemGlobal.wanIntfGroup[i].p_intfInfo->p_wanStaticInfo->ipv6_mask_length;

					if(tmp_ip_len==128)
					{
						if(!memcmp(tmp_ipv6_addr.ipv6_addr, static_info->gateway_ipv6_addr.ipv6_addr, IPV6_ADDR_LEN))
						{
							longest_gw_ip_len = tmp_ip_len;
							memcpy(gwDevName, (i==-1) ? devName : rt_edp_db.systemGlobal.wanIntfGroup[i].p_intfInfo->storedInfo.wan_intf.devName, IFNAMSIZ);
							break;
						}
					}
					else
					{
						idx = (tmp_ip_len>>3)&0xff;
						if((tmp_ip_len&0x7)==0) bitMask=0;
						else bitMask = (0xff<<(8-(tmp_ip_len&0x7)))&0xff;

						if(tmp_ip_len>longest_gw_ip_len && memcmp(tmp_ipv6_addr.ipv6_addr, static_info->gateway_ipv6_addr.ipv6_addr, idx)==0 
							&& ((tmp_ipv6_addr.ipv6_addr[idx]&bitMask)==(static_info->gateway_ipv6_addr.ipv6_addr[idx]&bitMask)))
						{
							longest_gw_ip_len = tmp_ip_len;
							memcpy(gwDevName, (i==-1) ? devName : rt_edp_db.systemGlobal.wanIntfGroup[i].p_intfInfo->storedInfo.wan_intf.devName, IFNAMSIZ);
						}
					}
				}

				if(longest_gw_ip_len==0)
					WARNING("Gateway %s is unreachable by using ipv6 route table.", _rt_edp_inet_n6toa(static_info->gateway_ipv6_addr.ipv6_addr));
				
				if(static_info->ipv6_default_gateway_on)
				{
					_rt_edp_pipe_cmd("%s -6 route add default via %s dev %s mtu %d", EDP_IP_UTILITY, _rt_edp_inet_n6toa(static_info->gateway_ipv6_addr.ipv6_addr), gwDevName, static_info->mtu);
					
				}
				else
				{
					_rt_edp_pipe_cmd("%s -6 route del %s/%d", EDP_IP_UTILITY, _rt_edp_inet_n6toa(static_info->ipv6_addr.ipv6_addr), static_info->ipv6_mask_length);
					if(longest_gw_ip_len==static_info->ipv6_mask_length)
						_rt_edp_pipe_cmd("%s -6 route add %s/128 via %s dev %s", EDP_IP_UTILITY, _rt_edp_inet_n6toa(static_info->ipv6_addr.ipv6_addr), _rt_edp_inet_n6toa(static_info->gateway_ipv6_addr.ipv6_addr), devName);
					_rt_edp_pipe_cmd("%s -6 route add %s/%d via %s dev %s", EDP_IP_UTILITY, _rt_edp_inet_n6toa(static_info->ipv6_addr.ipv6_addr), static_info->ipv6_mask_length, _rt_edp_inet_n6toa(static_info->gateway_ipv6_addr.ipv6_addr), devName);
				}
				if(static_info->gw_mac_auto_learn_for_ipv6==0 && memcmp(&static_info->gateway_mac_addr_for_ipv6, &zeroMAC, sizeof(rtk_mac_t)))
					_rt_edp_pipe_cmd("%s -6 neigh add %s dev %s lladdr %pM", EDP_IP_UTILITY, _rt_edp_inet_n6toa(static_info->gateway_ipv6_addr.ipv6_addr), gwDevName, &static_info->gateway_mac_addr_for_ipv6.octet[0]);
				else
					_rt_edp_pipe_cmd("%s -6 neigh add %s dev %s lladdr %pM nud incomplete", EDP_IP_UTILITY, _rt_edp_inet_n6toa(static_info->gateway_ipv6_addr.ipv6_addr), gwDevName, &zeroMAC.octet[0]);
			}
			// For policy route/binding
			if(memcmp(&static_info->ipv6_addr, &zeroV6Addr, sizeof(rtk_ipv6_addr_t)))
			{
				_rt_edp_pipe_cmd("%s -6 route del %s/%d table 0x%x", EDP_IP_UTILITY, _rt_edp_inet_n6toa(static_info->ipv6_addr.ipv6_addr), static_info->ipv6_mask_length, RT_EDP_POLICY_ROUTE_TABLE_START+wan_intf_idx);
				_rt_edp_pipe_cmd("%s -6 route del default table 0x%x", EDP_IP_UTILITY, RT_EDP_POLICY_ROUTE_TABLE_START+wan_intf_idx);
				_rt_edp_pipe_cmd("%s -6 route add %s/%d dev %s table 0x%x", EDP_IP_UTILITY, _rt_edp_inet_n6toa(static_info->ipv6_addr.ipv6_addr), static_info->ipv6_mask_length, devName, RT_EDP_POLICY_ROUTE_TABLE_START+wan_intf_idx);
				_rt_edp_pipe_cmd("%s -6 route add default dev %s table 0x%x", EDP_IP_UTILITY, devName, RT_EDP_POLICY_ROUTE_TABLE_START+wan_intf_idx);
			}
			//For DSLITE
			if(rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type==RT_EDP_DSLITE)
				_rt_edp_pipe_cmd("%s -6 addr add %s/%d dev %s", EDP_IP_UTILITY, _rt_edp_inet_n6toa(rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr), static_info->ipv6_mask_length, devName);
			else if(rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type==RT_EDP_PPPoE_DSLITE)
				_rt_edp_pipe_cmd("%s -6 addr add %s/%d dev %s", EDP_IP_UTILITY, _rt_edp_inet_n6toa(rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr), static_info->ipv6_mask_length, devName);
		}
	}

	//==========================Store software data==========================
	rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->ip_version = static_info->ip_version;
	rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->napt_enable = static_info->napt_enable;
	rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->ipv6_napt_enable = static_info->ipv6_napt_enable;
	rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->ip_addr = static_info->ip_addr;
	rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->remote_host_ip_addr = static_info->remote_host_ip_addr;
	rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->ip_network_mask = static_info->ip_network_mask;
	rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->ipv4_default_gateway_on = static_info->ipv4_default_gateway_on;
	rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->gateway_ipv4_addr = static_info->gateway_ipv4_addr;
	memcpy(&rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->ipv6_addr, &static_info->ipv6_addr, sizeof(rtk_ipv6_addr_t));
	rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->ipv6_mask_length = static_info->ipv6_mask_length;
	rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->ipv6_default_gateway_on = static_info->ipv6_default_gateway_on;
	memcpy(&rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->gateway_ipv6_addr, &static_info->gateway_ipv6_addr, sizeof(rtk_ipv6_addr_t));
	rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->mtu = static_info->mtu;
	rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->gw_mac_auto_learn_for_ipv4 = static_info->gw_mac_auto_learn_for_ipv4;
	rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->gw_mac_auto_learn_for_ipv6 = static_info->gw_mac_auto_learn_for_ipv6;
	memcpy(&rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->gateway_mac_addr_for_ipv4, &static_info->gateway_mac_addr_for_ipv4, sizeof(rtk_mac_t));
	memcpy(&rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->gateway_mac_addr_for_ipv6, &static_info->gateway_mac_addr_for_ipv6, sizeof(rtk_mac_t));
	rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->static_route_with_arp = static_info->static_route_with_arp;
	
	return RT_EDP_ERR_OK;
}

rt_edp_err_code_t _rt_edp_dhcp_release(int wan_intf_idx, rt_edp_ipStaticInfo_t *static_info)
{
	char devName[IFNAMSIZ]; //device name, ex:nas0_0

	if(strlen(rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf.devName))
	{
		memcpy(devName, rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].storedInfo.wan_intf.devName, IFNAMSIZ);
		
		if(static_info->ip_version!=RT_EDP_IPVER_V6ONLY)
		{
			//Clear V4 setting!
			_rt_edp_pipe_cmd("%s -4 addr flush dev %s", EDP_IP_UTILITY, devName);
			_rt_edp_pipe_cmd("%s -4 route flush dev %s", EDP_IP_UTILITY, devName);

			if(static_info->napt_enable && static_info->ip_addr!=0)
				_rt_edp_pipe_cmd("iptables -t nat -D POSTROUTING -o %s -j MASQUERADE", devName);
			
			//==========================Store software data==========================
			rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->napt_enable = 0;
	        rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->ip_addr = 0;
	        rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->ip_network_mask = 0;
			rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->ipv4_default_gateway_on = 0;
			rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->gateway_ipv4_addr = 0;
			rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->gw_mac_auto_learn_for_ipv4 = 0;
			memset(&rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->gateway_mac_addr_for_ipv4, 0, sizeof(rtk_mac_t));
		}
		if(static_info->ip_version!=RT_EDP_IPVER_V4ONLY)
		{
			//Clear V6 setting!!
			_rt_edp_pipe_cmd("%s -6 addr flush dev %s", EDP_IP_UTILITY, devName);
			_rt_edp_pipe_cmd("%s -6 route flush dev %s", EDP_IP_UTILITY, devName);

			//==========================Store software data==========================
			memset(rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->ipv6_addr.ipv6_addr, 0, IPV6_ADDR_LEN);
			rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->ipv6_mask_length = 0;
			rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->ipv6_default_gateway_on = 0;
			memset(rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->gateway_ipv6_addr.ipv6_addr, 0, IPV6_ADDR_LEN);
			rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->gw_mac_auto_learn_for_ipv6 = 0;
			memset(&rt_edp_db.systemGlobal.interfaceInfo[wan_intf_idx].p_wanStaticInfo->gateway_mac_addr_for_ipv6, 0, sizeof(rtk_mac_t));	
		}

		_rt_edp_pipe_cmd("ethctl remsmux ipoe %s %s", EDP_WAN_DEVICE_NAME, devName);
	}
	
	return RT_EDP_ERR_OK;
}

