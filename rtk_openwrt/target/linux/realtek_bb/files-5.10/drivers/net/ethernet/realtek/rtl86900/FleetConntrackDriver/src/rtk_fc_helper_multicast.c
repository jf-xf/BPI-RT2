/*
 * Copyright (C) 2018 Realtek Semiconductor Corp.
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
/*
#include <linux/netfilter.h>
#include <linux/netdevice.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#include <net/netfilter/nf_conntrack.h>
*/
#include <rtk_fc_helper.h>
#include <rtk_fc_mgr.h>
#include <rtk_fc_api.h>
#include <rtk_fc_helper_multicast.h>


//#define CONFIG_RTK_TRAP_USER_GROUP_PS

int _rtk_fc_igmp_mdb_callback_register(fc_igmp_mdb_callback searchMdbFunc)
{
	fc_mgr_db._rtk_fc_igmp_mdb_search = searchMdbFunc;
	return SUCCESS;
}
int _rtk_fc_igmp_mdb_callback_unregister(void)
{
	fc_mgr_db._rtk_fc_igmp_mdb_search = NULL;
	return SUCCESS;
}
int rtk_fc_igmp_mdb_callback_handler(unsigned char isIpv6,unsigned int *groupip,unsigned int *srcip,short igrCvlan,short igrSvlan,rtk_fc_table_mcConfigTbl_t *mcCfg)
{
	if(fc_mgr_db._rtk_fc_igmp_mdb_search)
		return fc_mgr_db._rtk_fc_igmp_mdb_search(isIpv6,groupip,srcip,igrCvlan,igrSvlan,mcCfg);
	else
		return FAILED;
}
int rtk_fc_igmp_mdb_isRegister(void)
{
	if(fc_mgr_db._rtk_fc_igmp_mdb_search)
		return TRUE;
	else
		return FALSE;
}

EXPORT_SYMBOL(_rtk_fc_igmp_mdb_callback_register);
EXPORT_SYMBOL(_rtk_fc_igmp_mdb_callback_unregister);
EXPORT_SYMBOL(rtk_fc_igmp_mdb_callback_handler);
EXPORT_SYMBOL(rtk_fc_igmp_mdb_isRegister);


#if defined(CONFIG_RTK_SOC_RTL8198D)


rtk_fc_userGroup_t userGroupList[MAX_USER_GROUP_NUMBER];


int rtk_fc_set_user_group(unsigned int* groupAddr, unsigned short action,unsigned short ipVersion )
{
	int i ;
	int ret = FAIL;

	
	if(NULL == groupAddr)
	{	
		return ret;	
	}
	//printk(" --%s:%d ,groupAddr =0x%x \n",__FUNCTION__,__LINE__,groupAddr[0]);
	
	for(i = 0; i < MAX_USER_GROUP_NUMBER; i++)
	{
		if((userGroupList[i].valid==0)
		&&(action==ADD_USER_GROUP_FLAG))
		{
			userGroupList[i].valid =1;
			userGroupList[i].ipVersion=ipVersion;

			if(ipVersion == IP6_USER_GROUP_FLAG)
				memcpy(userGroupList[i].groupAddr,groupAddr,sizeof(userGroupList[i].groupAddr));

			if(ipVersion == IP4_USER_GROUP_FLAG)
				userGroupList[i].groupAddr[0] = groupAddr[0] ;
				//memcpy(reservedMCastRecord[i].groupAddr,groupAddr,sizeof(reservedMCastRecord[i].groupAddr[0]));

				//if(ipVersion == IP6_RESERVED_MULTICAST_FLAG)
		    	//printk("[%s]ADD:ip%d reservedMCastRecord[%d].groupAddr =0x%x%x%x%x\n",__FUNCTION__,ipVersion,i,reservedMCastRecord[i].groupAddr[0],reservedMCastRecord[i].groupAddr[0],reservedMCastRecord[i].groupAddr[0],reservedMCastRecord[i].groupAddr[0]);

				if(ipVersion == IP4_USER_GROUP_FLAG)
				//printk("[%s] i =%d add user.groupAddr =0x%x,userGroupList[i].ipVersion =%d\n",__FUNCTION__,i,userGroupList[i].groupAddr[0],userGroupList[i].ipVersion);

			ret =SUCCESS;
			break;
		}
		else if((userGroupList[i].valid==1)
			&&(action==DEL_USER_GROUP_FLAG))
		{		
			if((ipVersion == IP6_USER_GROUP_FLAG)
			&&(0 == memcmp(userGroupList[i].groupAddr,groupAddr,sizeof(userGroupList[i].groupAddr))))
			{
				userGroupList[i].valid =0;
				memset(userGroupList[i].groupAddr, 0 , sizeof(userGroupList[i].groupAddr));
				ret =SUCCESS;
				// printk("[%s]DEL v6:ip%d reservedMCastRecord[%d].groupAddr =0x%x%x%x%x\n",__FUNCTION__,ipVersion,i,reservedMCastRecord[i].groupAddr[0],reservedMCastRecord[i].groupAddr[1],reservedMCastRecord[i].groupAddr[2],reservedMCastRecord[i].groupAddr[3]);
				break;
			}

			if((ipVersion == IP4_USER_GROUP_FLAG)
				&& (userGroupList[i].groupAddr[0] == groupAddr[0]))
			{
				userGroupList[i].valid =0;
				memset(userGroupList[i].groupAddr, 0 , sizeof(userGroupList[i].groupAddr));
				ret =SUCCESS;
				//printk("[%s]DEL v4:ip%d reservedMCastRecord[%d].groupAddr =0x%x\n",__FUNCTION__,ipVersion,i,userGroupList[i].groupAddr[0]);
				break;
				
			}			
		}

	}

 	if((i >= MAX_USER_GROUP_NUMBER)&&(action==ADD_USER_GROUP_FLAG))
 	{
 		printk("User group entry is full\n");
		ret = FAIL;
 	}
	if((i >= MAX_USER_GROUP_NUMBER)&&(action==DEL_USER_GROUP_FLAG))
	{
		printk("GIP ip not in user group\n");
		ret = FAIL;
	}
	return ret;
	
}

int rtk_fc_get_user_group_by_index(unsigned int index,unsigned short *valid, unsigned int* groupAddress, unsigned short* ipVersion)
{
	int ret = FAIL;

	if(index >= MAX_USER_GROUP_NUMBER)
		return ret;
	
	if(userGroupList[index].valid == 1)
	{
		if(userGroupList[index].ipVersion == IP6_USER_GROUP_FLAG)
		{
			
			memcpy(groupAddress,userGroupList[index].groupAddr,sizeof(userGroupList[index].groupAddr));
			*valid = userGroupList[index].valid;
			*ipVersion = userGroupList[index].ipVersion ;
		}
		else
		{
			*valid = userGroupList[index].valid;
			groupAddress[0] = userGroupList[index].groupAddr[0];
			*ipVersion = userGroupList[index].ipVersion ;
		}

		ret = SUCCESS;
	}

	return ret;
}



int rtk_fc_get_user_group(unsigned int* groupAddress, unsigned short ipVersion)
{
	int i ;
	int ret = FAIL;

	if(NULL == groupAddress)
		return ret;	

	for(i=0; i<MAX_USER_GROUP_NUMBER; i++)
	{
		 if((userGroupList[i].valid == 1) && (userGroupList[i].ipVersion == ipVersion))
		{		
			if(ipVersion == IP6_USER_GROUP_FLAG)
			{
				if(0 == memcmp(groupAddress,userGroupList[i].groupAddr,sizeof(userGroupList[i].groupAddr)))
				{
					ret =SUCCESS;
					break;
				}
			}
		

			if(ipVersion == IP4_USER_GROUP_FLAG)	
			{
				if(groupAddress[0] == userGroupList[i].groupAddr[0])
				{
					ret =SUCCESS;
					break;
				}
			}
						
		}
	}

	if(i >= MAX_USER_GROUP_NUMBER)
		ret = FAIL;

	return ret;
}


int rtk_fc_add_lutMac(unsigned int*_group ,unsigned short isIpv6)
{
	int ret = FAIL;
	ret = _rtk_fc_add_lutMac(_group ,isIpv6);
	return ret;
}

int rtk_fc_check_user_group(unsigned char * skbData)
{
	int ret = FAIL;
#if defined(CONFIG_RTK_SOC_RTL8198D)
	ret = _rtk_fc_check_user_group( skbData);
#endif
	return ret;
}

#endif


#if defined(CONFIG_FC_RTL8198F_SERIES)


//check current port is 8367 port or not.
int rtk_fc_check_8367Port(u32 portnum, u32 wan_portMask)
{
	unsigned int lanPhyPort_8226;
	unsigned int lanPortNum = 0;
	
	if((portnum > RTK_FC_MAC_PORT_PON) || (((1<<portnum)) == wan_portMask))
		return CA_UINT32_INVALID;

	//if((1<<portnum) > fc_db.wanPortMask.portmask)
	if((1<<portnum) > wan_portMask)
		lanPortNum = portnum - 1;
	else
		lanPortNum  = portnum ;
	
	if(rtl_get_8226_lan_dev_portnum_hook){
		lanPhyPort_8226 = (*rtl_get_8226_lan_dev_portnum_hook)();
	}

	printk("lanPhyPort_8226 =%d ,lanPortNum =%d,portnum =%d\n",lanPhyPort_8226,lanPortNum,portnum); 
	if(CA_UINT32_INVALID == lanPhyPort_8226)
		return CA_UINT32_INVALID;
		
	if(lanPortNum != lanPhyPort_8226)
		return IS_8367_LAN_PHY_PORT;
	else
		return IS_8226_LAN_PHY_PORT;

}

//Get port number in 8367 .
int rtk_fc_get_portNumber_in_8367(unsigned int portnum,unsigned int wan_portMask)
{
	unsigned int lanPhyPort_8367 = 0;
	unsigned int lanPortNum = 0;
	
	if((portnum > RTK_FC_MAC_PORT_PON) || (((1<<portnum)) == wan_portMask))
		return CA_UINT32_INVALID;

	if((1<<portnum) > wan_portMask)
		lanPortNum = portnum - 1;
	else
		lanPortNum  = portnum ;


	if(rtl_get_portnum_in_8367_hook){
		lanPhyPort_8367 = (*rtl_get_portnum_in_8367_hook)(lanPortNum);
	}
	
	if(CA_UINT32_INVALID == lanPhyPort_8367)
		return CA_UINT32_INVALID;	

	// return 8367 port number from index 0 
	return lanPhyPort_8367;
	
}

//int rtk_l2_set_8367_ipMcastAddr(rtk_rg_asic_path3_entry_t *pP3, unsigned int l2mcgid,rtk_fc_g3IgrExtraInfo_t * pG3IgrExtraInfo, unsigned int ipv6)
int rtk_l2_set_8367_ipMcastAddr(rtk_fc_l2_mcast_members_t *pP3, unsigned int l2mcgid,unsigned int wan_portMask,unsigned int ipv6)
{
	int ret = FAIL;
	rtk_portmask_t ipMcastmask,getIpMcastmask;
	int i = 0 ;
	int port = 0;
	ca_l2_mcast_group_members_t* pl2mcGMember = NULL;
	u32 dip , sip, vid, portmask ,Mcastmask;
	char macOctet[ETHER_ADDR_LEN];
	u32 groupIp =0;

	
	dip = pP3->in_dst_ipv4_addr;
	sip = pP3->in_src_ipv4_addr;

	memset(macOctet,0 ,ETHER_ADDR_LEN);
	pl2mcGMember = kmalloc(sizeof(ca_l2_mcast_group_members_t), GFP_ATOMIC);
	if(NULL == pl2mcGMember)
		return CA_UINT32_INVALID;
	
	vid = 0;
	Mcastmask = 0;
	portmask = pP3->out_portmask;
	
	memset(&ipMcastmask, 0x0, sizeof(rtk_portmask_t));
	memset(&getIpMcastmask, 0 ,sizeof(getIpMcastmask));

	for(i = 0 ; i <= RTK_FC_MAC_PORT_PON ; i++)
	{	
		if((1<<i) == wan_portMask)
			continue;
		if(portmask & (1<<i))
		{
			if(rtk_fc_check_8367Port(i,wan_portMask) == 1)
			{
				if(CA_UINT32_INVALID == (port = rtk_fc_get_portNumber_in_8367(i,wan_portMask)))
				{
					if(NULL != pl2mcGMember)
						kfree(pl2mcGMember);
					return CA_UINT32_INVALID;
				}
				ipMcastmask.bits[0] |= (0x00000001 << port);
			}
		}
	}

	memset(pl2mcGMember, 0, sizeof(ca_l2_mcast_group_members_t));
	
	pl2mcGMember->mcg_id = l2mcgid;
	pl2mcGMember->member_count =1;
	
	//8367 lan port , send to lspid 0x6
	pl2mcGMember->member[0].member_port = 0x6;
	pl2mcGMember->member[0].action_mask.vlan_action = TRUE;
	pl2mcGMember->member[0].vid = pP3->out_cvlan_id;
	pl2mcGMember->member[0].vpri = pP3->out_cpri;
	pl2mcGMember->member[0].cos = pP3->out_user_pri_act ? pP3->out_user_priority : 0;
	//printk("--pP3->out_cvid_format_act =%d, pP3->in_ctagif =%d\n",pP3->out_cvid_format_act,pP3->in_ctagif);
	if(pP3->out_cvid_format_act)
	{
		pl2mcGMember->member[0].vlan_action = pP3->in_ctagif ? CA_L2_VLAN_TAG_ACTION_SWAP : CA_L2_VLAN_TAG_ACTION_PUSH;
	}
	else
	{
		pl2mcGMember->member[0].vlan_action = pP3->in_ctagif ? CA_L2_VLAN_TAG_ACTION_POP : CA_L2_VLAN_TAG_ACTION_NOP;
	}
	pl2mcGMember->member[0].action_mask.mac_sa_trans = pP3->out_smac_trans;

	
	if(0 == ipMcastmask.bits[0])
	{
		//ret = ca_l2_mcast_member_delete(G3_DEF_DEVID, pl2mcGMember);
		ret = ca_l2_mcast_member_delete(0, pl2mcGMember);
		//printk("---ca_l2_mcast_member_del ,ret =%d\n",ret);
		if(NULL != pl2mcGMember)
			kfree(pl2mcGMember);
		return SUCCESS;
	}
	else
	{
		
		//ret = ca_l2_mcast_member_add(G3_DEF_DEVID, pl2mcGMember);
		ret = ca_l2_mcast_member_add(0, pl2mcGMember);
		//printk("---ca_l2_mcast_member_add ,ret =%d\n",ret);
	}
	
	
	if(ipv6 != 1)
	{	
		memset(&getIpMcastmask, 0x0, sizeof(rtk_portmask_t));
		vid =0;
		
		if(SUCCESS != (ret = rtk_l2_ipMcastGroupEntry_add(dip, vid, &getIpMcastmask)))
		{
			printk("%s %d, rtk_l2_ipMcastGroupEntry_add failed. ret =%d\n",__FUNCTION__,__LINE__,ret);
			if(NULL != pl2mcGMember)
				kfree(pl2mcGMember);
			return ret;
		}

		Mcastmask =  ipMcastmask.bits[0];
		
		if(SUCCESS != (ret = rtk_l2_8367_ipMcastAddr_add(dip,sip,Mcastmask)))
		{
		 	printk("\n err: %s %d ,ret =%d, dip = 0x%x ,sip =0x%x\n",__FUNCTION__,__LINE__,ret,dip ,sip);
			if(NULL != pl2mcGMember)
				kfree(pl2mcGMember);
			return ret;
		}	
	}
	else
	{
		vid = pl2mcGMember->member[0].vid;
			
		macOctet[0] = 0x33;
		macOctet[1] = 0x33;
		//groupIp = ntohl(pG3IgrExtraInfo->ipv6_Daddr.s6_addr32[3]);
		groupIp  = ntohl(pP3->addr32[3]);
		macOctet[2] = (groupIp & 0xFF000000) >> 24;
		macOctet[3] = (groupIp & 0xFF0000) >> 16;
		macOctet[4] = (groupIp & 0xFF00) >> 8;
		macOctet[5] =  groupIp & 0xFF;

		//printk("%s:%d, daAddr =0x%x-%x-%x-%x \n",__FUNCTION__,__LINE__,pG3IgrExtraInfo->ipv6_Daddr.s6_addr32[0],pG3IgrExtraInfo->ipv6_Daddr.s6_addr32[1],pG3IgrExtraInfo->ipv6_Daddr.s6_addr32[2],pG3IgrExtraInfo->ipv6_Daddr.s6_addr32[3]);
		//printk("%s:%d,mac =0x%x-%x-%x-%x-%x-%x\n",__FUNCTION__,__LINE__,macOctet[0],macOctet[1],macOctet[2],macOctet[3],macOctet[4],macOctet[5]);
		if(SUCCESS != (ret =  rtk_l2_8367_mcastAddr_add(macOctet,ipMcastmask.bits[0],vid)))
		{
			printk("\n err:  %s %d, ret =%d\n",__FUNCTION__,__LINE__,ret);
			if(NULL != pl2mcGMember)
				kfree(pl2mcGMember);
			return ret;
		}
	}
	
if(NULL != pl2mcGMember)
		kfree(pl2mcGMember);
	
	return SUCCESS;
}


int rtk_l2_del_8367_ipMcastAddr(u32 dip, u32 sip, u32 vid)
{
	int ret = FAIL;
	
	if(SUCCESS != (ret = rtk_l2_8367_ipMcastAddr_del(dip,sip)))
	{
		printk("\n %s %d ,rtk_l2_8367_ipMcastAddr_del failed! ret =%d, dip = 0x%x ,sip =0x%x\n",__FUNCTION__,__LINE__,ret,dip ,sip);
		return ret;
	}	
		
	return SUCCESS;
}


int rtk_l2_del_8367_ip6McastAddr(u32* ipv6McAddr ,u32 vid)
{
	int ret = FAIL;
	u32 groupIp = 0;

	char macOctet[ETHER_ADDR_LEN];
	
	macOctet[0] = 0x33;
	macOctet[1] = 0x33;
	groupIp = ntohl(ipv6McAddr[3]);
	macOctet[2] = (groupIp & 0xFF000000) >> 24;
	macOctet[3] = (groupIp & 0xFF0000) >> 16;
	macOctet[4] = (groupIp & 0xFF00) >> 8;
	macOctet[5] =  groupIp & 0xFF;
	
	if(SUCCESS != (ret = rtk_l2_8367_mcastAddr_del(macOctet, vid)))
	{
		printk("\n err: %s %d ,ret =%d, \n",__FUNCTION__,__LINE__,ret);
		return ret;
	}	
	
	return SUCCESS;
}

#endif

