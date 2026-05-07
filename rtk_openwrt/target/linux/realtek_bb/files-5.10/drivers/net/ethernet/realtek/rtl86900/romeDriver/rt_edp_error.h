#ifndef __RT_EDP_ERROR_H__
#define __RT_EDP_ERROR_H__

/*
 * Include Files
 */
#include <common/error.h>

/*
 * Data Type Declaration
 */

/* ERROR NO =============================================================== */
typedef enum rt_edp_err_code_e
{
	RT_EDP_ERR_OK=0,

	RT_EDP_ERR_FAILED = (RT_ERR_COMMON_END+2), //0x10001
	RT_EDP_ERR_NOT_SUPPORT_TESTCHIP,		//the API enable some feature not support by test chip			//unused
	RT_EDP_ERR_BUF_OVERFLOW,
	RT_EDP_ERR_NULL_POINTER,				//input pointer is null
	RT_EDP_ERR_INITPM_UNINIT,			//initparm's pointers is uninit					//0x10005

	RT_EDP_ERR_PON_INVALID,				//PON port can not be set as WAN port in RLE0371
	RT_EDP_ERR_INVALID_PARAM,			//parameters is invalid
	RT_EDP_ERR_ENTRY_FULL,				//the entry table is full
	RT_EDP_ERR_NOT_INIT,					//the module is not init
	RT_EDP_ERR_VLAN_BASED_OVERLAP_SUBNET,//diff VLAN-based interfaces can not have overlap subnet
	RT_EDP_ERR_DEF_ROUTE_EXIST,			//the internet connection is exist				//0x1000b

	RT_EDP_ERR_STATIC_NOT_FOUND,			//DHCP server index did not find
	RT_EDP_ERR_ARP_NOT_FOUND,			//ARP did not find
	RT_EDP_ERR_ARP_FULL,					//ARP table overflow
	RT_EDP_ERR_VLAN_BIND_UNINIT,			//vlan-binding is uninit
	RT_EDP_ERR_PPPOE_UNINIT,				//pppoe_before is not called before				//0x10010

	RT_EDP_ERR_CHIP_NOT_SUPPORT,			//the function is not supported in this chip version
	RT_EDP_ERR_ENTRY_NOT_EXIST,
	RT_EDP_ERR_INDEX_OUT_OF_RANGE,
	RT_EDP_ERR_NO_MORE_ENTRY_FOUND,
	RT_EDP_ERR_MODIFY_LAN_AT_WAN_EXIST,	//deprecated, LAN intf can not add or delete when WAN exist		//0x10015

	RT_EDP_ERR_LAN_NOT_EXIST,			//LAN intf did not created before binding
	RT_EDP_ERR_GW_MAC_NOT_SET,			//for lite romeDriver, set WAN with default gw must has MAC address input
	RT_EDP_ERR_VLAN_SET_FAIL,			//vlan set failed
	RT_EDP_ERR_VLAN_GET_FAIL,			//vlan get failed
	RT_EDP_ERR_INTF_SET_FAIL,			//interface set failed
	RT_EDP_ERR_INTF_GET_FAIL,			//interface get failed							//0x1001b

	RT_EDP_ERR_ROUTE_SET_FAIL,				//routing table set failed
	RT_EDP_ERR_ROUTE_GET_FAIL,				//routing table get failed
	RT_EDP_ERR_EXTIP_SET_FAIL,			//internal external IP table set failed
	RT_EDP_ERR_EXTIP_GET_FAIL,			//internal external IP table get failed
	RT_EDP_ERR_NXP_SET_FAIL,				//nexthop table set failed						//0x10020

	RT_EDP_ERR_NXP_GET_FAIL,				//nexthop table get failed
	RT_EDP_ERR_PPPOE_SET_FAIL,			//pppoe table set failed
	RT_EDP_ERR_PPPOE_GET_FAIL,			//pppoe table get failed
	RT_EDP_ERR_PORT_BIND_SET_FAIL,		//Port-binding set failed
	RT_EDP_ERR_PORT_BIND_GET_FAIL,		//Port-binding get failed						//0x10025

	RT_EDP_ERR_EXTPORT_BIND_SET_FAIL,	//extension Port-binding set failed
	RT_EDP_ERR_EXTPORT_BIND_GET_FAIL,	//extension Port-binding get failed
	RT_EDP_ERR_WANTYPE_SET_FAIL,			//WAN type table set failed
	RT_EDP_ERR_WANTYPE_GET_FAIL,			//WAN type table get failed
	RT_EDP_ERR_VLAN_BIND_SET_FAIL,		//VLAN-Port binding set failed					//0x1002a

	RT_EDP_ERR_VLAN_BIND_GET_FAIL,		//VLAN-Port binding get failed
	RT_EDP_ERR_ACL_CF_FIELD_CONFLICT,	//aclFilter assigned some conflict field(ex:ipv4 ipv6 at the same time, DMAC != ingress_INTF.gmac, CVID!= ingress_INTF.vid, can be reference to _rt_edp_conflictField_and_flowDirection_check())
	RT_EDP_ERR_ACL_CF_FLOW_DIRECTION_ERROR, //return this error while in lan_to_lan /wan_to_wan and assignde CF pattern (CF ASIC just support in upstream/dowmstream)
	RT_EDP_ERR_ACL_ENTRY_FULL,			//ACL ASIC entry is full
	RT_EDP_ERR_ACL_ENTRY_ACCESS_FAILED,	//set/get ACL ASIC entry failed					//0x1002f

	RT_EDP_ERR_ACL_IPTABLE_FULL,			//ACL IP range table ASIC full
	RT_EDP_ERR_ACL_IPTABLE_ACCESS_FAILED,//set/get ACL IP range table ASIC failed
	RT_EDP_ERR_ACL_PORTTABLE_FULL,		//ACL port range table ASIC full
	RT_EDP_ERR_ACL_PORTTABLE_ACCESS_FAILED,//set/get ACL port range table ASIC failed
	RT_EDP_ERR_CF_ENTRY_FULL,			//CF ASIC entry is full						//0x10034

	RT_EDP_ERR_CF_ENTRY_ACCESS_FAILED,	//set/get CF ASIC entry failed
	RT_EDP_ERR_CF_IPTABLE_FULL,			//CF IP range table ASIC full
	RT_EDP_ERR_CF_IPTABLE_ACCESS_FAILED,	//set/get CF IP range table ASIC failed
	RT_EDP_ERR_CF_PORTTABLE_FULL,		//CF port range table ASIC full
	RT_EDP_ERR_CF_PORTTABLE_ACCESS_FAILED,//set/get ACL port range table ASIC failed			//0x10039

	RT_EDP_ERR_CF_DSCPTABLE_FULL,		//CF dscp remarking table ASIC full
	RT_EDP_ERR_CF_DSCPTABLE_ACCESS_FAILED,//set/get ACL dscp remarking table ASIC failed
	RT_EDP_ERR_ACL_SW_ENTRY_FULL,		//software aclFilterEntry full
	RT_EDP_ERR_ACL_SW_ENTRY_ACCESS_FAILED,//set/get ACL aclFilterEntry failed
	RT_EDP_ERR_ACL_SW_ENTRY_NOT_FOUND,	//not found valid aclFilterEntry					//0x1003e

	RT_EDP_ERR_ACL_SW_ENTRY_USED,		//set an aclFilterEntry which is in used(not used aclFilterEntry should clean to zero)
	RT_EDP_ERR_L2_ENTRY_ACCESS_FAILED,		//set/get L2 ASIC entry failed
	RT_EDP_ERR_L2_MACFILTER_ENTRY_ACCESS_FAILED,//access software macFulterEntry failed
	RT_EDP_ERR_L2_MACFILTER_ENTRY_FULL,		//software macFulterEntry is full (size defined by MAX_MAC_FILTER_ENTRY_SIZE)
	RT_EDP_ERR_L2_MACFILTER_ENTRY_NOT_FOUND,	//not found valid macFilterEntry				//0x10043

	RT_EDP_ERR_URLFILTER_ENTRY_ACCESS_FAILED, //access software urlFilterEntry failed
	RT_EDP_ERR_URLFILTER_ENTRY_FULL,		//software urlFulterEntry is full (size defined by MAX_URL_FILTER_ENTRY_SIZE)
	RT_EDP_ERR_URLFILTER_ENTRY_NOT_FOUND,//not found valid software urlFilterEntry
	RT_EDP_ERR_SVRPORT_SW_ENTRY_NOT_FOUND,
	RT_EDP_ERR_UPNP_SW_ENTRY_NOT_FOUND,											//0x10048

	RT_EDP_ERR_NAPT_SW_ENTRY_NOT_FOUND,	//0x10049
	RT_EDP_ERR_EXTIP_TYPE_MISMATCH,
	RT_EDP_ERR_NAPT_SET_FAIL,
	RT_EDP_ERR_NAPT_GET_FAIL,
	RT_EDP_ERR_NAPT_OVERFLOW,

	RT_EDP_ERR_NAPTR_SET_FAIL,		//0x1004e
	RT_EDP_ERR_NAPTR_GET_FAIL,
	RT_EDP_ERR_NAPTR_OVERFLOW,
	RT_EDP_ERR_NAPT_FLOW_DUPLICATE,
	RT_EDP_ERR_NAPT_FLOW_OVERFLOW,

	RT_EDP_ERR_ARP_MAPPING_OVERFLOW,	//0x10053
	RT_EDP_ERR_ARP_ENTRY_STATIC,			//add ARP failed because ARP entry is exist and STATIC
	RT_EDP_ERR_ARP_OVERFLOW,
	RT_EDP_ERR_L2_ENTRY_NOT_FOUND,
	RT_EDP_ERR_PPB_SET_FAILED,			//set port-and-protocol based vlan failed

	RT_EDP_ERR_UNTAG_BRIDGEWAN_TWICE,	//set untag bridge WAN at same port twice
	RT_EDP_ERR_NEIGHBOR_NOT_FOUND,				//Neighbor did not find
	RT_EDP_ERR_NEIGHBOR_FULL,					//Neighbor table overflow				//0x1005a
	RT_EDP_ERR_SUBNET_INTERFACE_ASYMMETRIC,		//new routing entry has same IP-range with other interface, but has different VLANID or MAC address or type
	RT_EDP_ERR_VLAN_USED_BY_INTERFACE,			//the vlan entered is overlaped with interface setting

	RT_EDP_ERR_VLAN_USED_BY_VLANBINDING,			//the vlan entered is overlaped with vlanBinding setting
	RT_EDP_ERR_VLAN_USED_BY_CVLAN,				//the vlan entered is overlaped with customer vlan setting
	RT_EDP_ERR_VLAN_NOT_CREATED_BY_CVLAN,				//the vlan entered is not created as customer vlan setting
	RT_EDP_ERR_UNBIND_BDWAN_SHOULD_EQUAL_LAN_VLAN,		//un-bind bridge wan should equals to LAN's VLAN ID		//0x10060
	RT_EDP_ERR_BIND_WITH_UNBIND_WAN,				//binding can not set to un-bind WAN

	RT_EDP_ERR_CVLAN_CREATED,					//the customer vlan id had been created before
	RT_EDP_ERR_CVLAN_RESERVED,					//the customer vlan id had been reserved by system
	RT_EDP_ERR_ALG_SRV_IN_LAN_EXIST,				//the ALG Service In Lan service had been assigned
	RT_EDP_ERR_ALG_SRV_IN_LAN_NO_IP,				//the ALG Service In Lan service did not assign server ip				//0x10065
	RT_EDP_ERR_CREATE_GATEWAY_LUT_FAIL,			//get error when add gateway Mac entry in Interface_add

	RT_EDP_ERR_DELETE_GATEWAY_LUT_FAIL,			//get error when del gateway Mac entry in Interface_del
	RT_EDP_ERR_STORMCONTROL_TYPE_FULL,			//at most support 4 types
	RT_EDP_ERR_STORMCONTROL_ENTRY_FULL,			//at most 16 entries(limit by sharemeter)
	RT_EDP_ERR_STORMCONTROL_ENTRY_HAS_BEEN_SET,  //0x1006a
	RT_EDP_ERR_STORMCONTROL_ENTRY_NOT_FOUND,

	RT_EDP_ERR_SHAREMETER_SET_FAILED,
	RT_EDP_ERR_SHAREMETER_GET_FAILED,
	RT_EDP_ERR_SHAREMETER_INVALID_METER_INDEX,
	RT_EDP_ERR_SHAREMETER_INVALID_RATE,
	RT_EDP_ERR_SHAREMETER_INVALID_INPUT,			//0x10070

	RT_EDP_ERR_ADD_ARP_TO_SW_TABLE,				//the routing will direct go to fwdEngine for sw arp table
	RT_EDP_ERR_ADD_ARP_MAC_FAILED,				//return FAIL when call _rt_edp_arpAndMacEntryAdd
	RT_EDP_ERR_PPPOEPASSTHROUGHT_NOT_SUPPORTED,
	RT_EDP_ERR_IPMC_IP_LIST_OUT_OF_RANGE,		//IPMC include or exclude IP list out of range.
	RT_EDP_ERR_IPMC_EXCLUDE_MODE_NOT_SUPPORT_EXTPORT,

	RT_EDP_ERR_ALG_SRV_IN_LAN_ENABLED,				//the ALG Service In Lan service had been enabled, turn off it before deleting or changing IP
	RT_EDP_ERR_INTF_OVERLAP_AND_SAME_SUBNET,		//two interfaces can not have the same ip subnet at all
	RT_EDP_ERR_GPON_TWO_UNTAG_BRIDGE_WAN,		//PON port can not support two untag bridge WAN at same time
	RT_EDP_ERR_GPON_MODULE_NOT_INIT,
	RT_EDP_ERR_GPON_SET_INFO_FAILED,				//0x1007a

	RT_EDP_ERR_GPON_GET_INFO_FAILED,
	RT_EDP_ERR_GPON_ALREADY_ACTIVATED,
	RT_EDP_ERR_GPON_ACTIVATED_FAILED,
	RT_EDP_ERR_GPON_DEACTIVATED_FAILED,
	RT_EDP_ERR_GPON_GET_STATUS_FAILED,

	RT_EDP_ERR_EPON_MODULE_NOT_INIT,				//0x10080
	RT_EDP_ERR_EPON_SET_INFO_FAILED,
	RT_EDP_ERR_EPON_GET_INFO_FAILED,
	RT_EDP_ERR_PON_GET_INFO_FAILED,
	RT_EDP_ERR_ACL_NOT_SUPPORT,

	RT_EDP_ERR_CF_NOT_SUPPORT,					//0x10085
	RT_EDP_ERR_VLAN_TWO_UNTAG_CPU_LAN,		//since vconfig need tag to separate each interface, we can only have one untag LAN at same time
	RT_EDP_ERR_VLAN_PRI_CONFLICT_WIFI,		//when set up dualband wifi, the VLAN priority can't be set as CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI
	RT_EDP_ERR_VLAN_USED_BY_SYSTEM,
	RT_EDP_ERR_ACCESSWAN_DISABLE,			//if the activeFunction is not category, set_mac_act should return this value

	RT_EDP_ERR_ACCESSWAN_NOT_LAN,			//0x1008a		//if the MAC address is not LAN host, we can't change it's category
	RT_EDP_ERR_VLAN_OVER_RANGE,				//the VLAN is beyond the resonable range
	RT_EDP_ERR_CPU_TAG_DIFF_BRIDGE_WAN,		//when add bridge WAN, VLAN's CPU port should follow bridge WAN's tag or untag
	RT_EDP_ERR_ENTRY_EXIST,					//for any table if the adding entry has existed in tables
	RT_EDP_ERR_ENTRY_NOT_FOUND,				//for any table if the search/delete entry can not found in tables

	RT_EDP_ERR_IPV6_LAN_MORE_SUBNET_FAIL,	//IPv6 can't be added as more subnet at same interface.
	RT_EDP_ERR_WLAN_BINDING_CONFLICT,		//0x10090	//the WLAN device binding is enabled by wlan_dev_binding_mask, port_binding_mask should not enable ext0 port
	RT_EDP_ERR_WLAN_BINDING_OVERLAP,			//two WAN are settings the same WLAN device
	RT_EDP_ERR_WLAN_BINDING_ABSENCE,			//the WLAN device non-existence
	RT_EDP_ERR_ACCESSWAN_WLAN_CONFLICT,		//if ext0 had been set limit, wlan0_dev_mask should not be set

	RT_EDP_ERR_ACCESSWAN_WLAN_NOT_ZERO,		//if wan access type is category, wlan0_dev_mask should keep zero
	RT_EDP_ERR_NAPTFILTERANDQOS_SW_ENTRY_NOT_FOUND,//0x10095
	RT_EDP_ERR_NAPTFILTERANDQOS_SW_ENTRY_FULL,
	RT_EDP_ERR_DHCP_LEASED_INVALID_IP,		//when set DHCP info with status leased, IP address can't be zero
	RT_EDP_ERR_STP_BLOCKING_ENABLED,			//when STP blocking mode is enabled, port-isolation should not be used

	RT_EDP_ERR_B4_IP_NOT_SET,				//set DSLITE WAN must has B4 address input
	RT_EDP_ERR_AFTR_IP_NOT_SET,				//set DSLITE WAN must has AFTR address input		//0x1009a
	RT_EDP_ERR_AFTR_MAC_NOT_SET,				//set DSLITE WAN without autolearn must has MAC address input
	RT_EDP_ERR_PPTP_MAC_NOT_SET,				//set PPTP WAN without autolearn must has MAC address input
	RT_EDP_ERR_L2TP_MAC_NOT_SET,				//set L2TP WAN without autolearn must has MAC address input

	RT_EDP_ERR_POLICYROUTE_NO_NH,			//set policy router should set nexthop or no nexthop idx could be approached
	RT_EDP_ERR_WWAN_SAME_VXD,				//for one vxd can only setup one wwan
	RT_EDP_ERR_WWAN_NOT_EXIST,				//WWAN device is not exist or initialized		//0x100a0
	RT_EDP_ERR_GPON_NOT_SUPPORT,				//returned by APIs that should't be called under GPON mode
	RT_EDP_ERR_ACL_EGRESS_WAN_MIX,			//mixed egress WAN pattern rules with non-egress-WAN pattern rules at same weight

	RT_EDP_ERR_DSLITE_UNMATCH,				//intfIdx is not match the adding WAN
	RT_EDP_ERR_DSLITE_UNINIT,				//the dslite table is not init before set WAN
	RT_EDP_ERR_DRIVER_NOT_SUPPORT,			//RG API didn't support for specific chip model	//0x100a5
	RT_EDP_ERR_CVLAN_INVALID,				//CVLAN_get try to get VLAN invalid
	RT_EDP_ERR_SR_TO_CPU,					//Static Route to nexthop IP which leads to CPU

	RT_EDP_ERR_SR_TO_DROP,					//Static Route to nexthop IP which leads to DROP	//0x100a8
	RT_EDP_ERR_SR_MAC_FAILED,				//Static Route create MAC entry failed
	RT_EDP_ERR_SR_ROUTE_NOT_FOUND,			//Static Route to nexthop IP which can't find routing entry
	RT_EDP_ERR_SR_REDUNDANT,					//try to create redundant static route with old one
	RT_EDP_ERR_SR_EXIST,						//try to delete wan interface with static route on it

	RT_EDP_ERR_DUMMY_DMAC_IDX,				//dmacL2Idx is dummy index(dmac=00:00:00:00:00:00)	//0x100ad
	RT_EDP_ERR_BINDING_DMAC2CVID_CONFLICT,	//Simultaneously enable binding and dmac2cvid in 9602C (apollo fe)
	RT_EDP_ERR_FLOW_NOT_FOUND,				//FLOW entry is not found
	RT_EDP_ERR_FLOW_FULL,					//FLOW table is overflow
	RT_EDP_ERR_ACL_DROP,						//hit ACL_RG drop rule
	RT_EDP_ERR_ACL_TRAP_TO_PS,				//hit ACL_RG Trap_to_PS rule

	RT_EDP_ERR_TRAP_TO_PS,					//trap tp PS
	RT_EDP_ERR_DROP,							//Drop
}rt_edp_err_code_t;
/* End of ERROR NO ======================================================== */

#endif
