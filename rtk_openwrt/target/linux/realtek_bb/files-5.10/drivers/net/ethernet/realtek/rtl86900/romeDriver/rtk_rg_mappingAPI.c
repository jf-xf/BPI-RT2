#include <common/error.h>
#include <rtk/init.h>
#include <rtk/l34_bind_config.h>
#include <rtk/svlan.h>
#include <dal/apollomp/raw/apollomp_raw_hwmisc.h>
#include <rtk/sec.h>
#include <rtk/stat.h>
#include <rtk/ponmac.h>
#include <rtk/l2.h>
#include <rtk/svlan.h>
#include <ioal/mem32.h>
#include <rtk_rg_internal.h>
#include <rtk_rg_callback.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 30)
#else
#include <linux/config.h>
#endif

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
#include <rtk_rg_apolloPro_internal.h>
#include <rtk_rg_apolloPro_asicDriver.h>
#if defined(CONFIG_RG_G3_SERIES)
#include <rtk_rg_g3_internal.h>
#include <flow.h>
#endif
#endif



/* for set age of lut entry to 1 */
#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support lut traffic bit
int32 _rtk_rg_l2_trafficBit_reset(rtk_l2_ucastAddr_t *pL2Addr)
{
	pL2Addr->age = 1;
	return rtk_l2_addr_add(pL2Addr);
}
#endif


#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
rtk_rg_err_code_t RTK_RG_ASIC_GLOBALSTATE_GET(rtk_rg_asic_globalStateType_t stateType, rtk_enable_t *pState)
{
	int ret=RT_ERR_RG_OK;
	rtk_enable_t state;

#if defined(CONFIG_RG_G3_SERIES)
	state = rg_db.systemGlobal.fbGlobalState[stateType];
	ret=RT_ERR_RG_OK;
#else
	ret=rtk_rg_asic_globalState_get(stateType, &state);
#endif
	if(ret==RT_ERR_RG_OK)
	{
		*pState = state;
	}
	return ret;
}

rtk_rg_err_code_t RTK_RG_ASIC_GLOBALSTATE_SET(rtk_rg_asic_globalStateType_t stateType, rtk_enable_t state)
{
	int ret=RT_ERR_RG_OK;

#if defined(CONFIG_RG_G3_SERIES)
	ret=RT_ERR_RG_OK;
#else
	ret=rtk_rg_asic_globalState_set(stateType, state);
#endif
	if(ret==RT_ERR_RG_OK)
	{
		rg_db.systemGlobal.fbGlobalState[stateType]=state;
	}
	return ret;
}

rtk_rg_err_code_t RTK_RG_ASIC_PREHASHPTN_SET(rtk_rg_asic_preHashPtn_t ptnType, uint32 code)
{
	int ret=RT_ERR_RG_OK;

#if defined(CONFIG_RG_G3_SERIES)
	ret=RT_ERR_RG_OK;
#else
	ret=rtk_rg_asic_preHashPtn_set(ptnType, code);
#endif
	if(ret==RT_ERR_RG_OK)
	{
		rg_db.systemGlobal.preHashPtn[ptnType]=code;
	}
	return ret;
}

#if defined(CONFIG_RG_G3_SERIES)


static rtk_rg_err_code_t _rtk_rg_set_ca_l2_mcast_group_members(ca_l2_mcast_group_members_t *pl2mcGMember,int *changeMemCnt,rtk_rg_mcEngine_action_t *action,uint32 portIdx)
{

	if(portIdx <RTK_RG_MAC_PORT_CPU_WLAN1_AND_OTHERS)
	{
		pl2mcGMember->member[*changeMemCnt].member_port = portIdx;
		pl2mcGMember->member_count++;
		pl2mcGMember->member[*changeMemCnt].action_mask.vlan_action = TRUE;
		pl2mcGMember->member[*changeMemCnt].vid = action->cvlanID;
		pl2mcGMember->member[*changeMemCnt].vpri = action->cPri;
		pl2mcGMember->member[*changeMemCnt].cos = action->userPri;
		pl2mcGMember->member[*changeMemCnt].vlan_action = action->vlanAction;
		pl2mcGMember->member[*changeMemCnt].action_mask.mac_sa_trans = action->smacTrans;
		(*changeMemCnt)++;
	}
	else if (portIdx ==RTK_RG_MAC_PORT_CPU_WLAN1_AND_OTHERS)
	{

		int i=0;
		for( i =(RTK_RG_WIFI_FLOWID_CPU_RSVD+1) ;i< RTK_RG_WIFI_FLOWID_MAX ; i++)
		{
			pl2mcGMember->member[*changeMemCnt].member_port = RTK_RG_MAC_PORT_CPU_WLAN1_AND_OTHERS;
			pl2mcGMember->member[*changeMemCnt].wifi_flowid = i;
			pl2mcGMember->member_count++;
			pl2mcGMember->member[*changeMemCnt].action_mask.vlan_action = TRUE;
			pl2mcGMember->member[*changeMemCnt].vid = action->cvlanID;
			pl2mcGMember->member[*changeMemCnt].vpri = action->cPri;
			pl2mcGMember->member[*changeMemCnt].cos = action->userPri;
			pl2mcGMember->member[*changeMemCnt].vlan_action = action->vlanAction;
			pl2mcGMember->member[*changeMemCnt].action_mask.mac_sa_trans = action->smacTrans;
			(*changeMemCnt)++;
			TABLE("add RTK_RG_MAC_PORT_CPU_WLAN1_AND_OTHERS wifi_flowid=%d",i);
		}

	}


	return RT_ERR_RG_OK;
}

rtk_rg_err_code_t _RTK_RG_ASIC_G3_MC_FLOW_SET(rtk_rg_table_flowEntry_t *pFlowPathEntry, rtk_rg_flow_extraInfo_t *pFlowExtraInfo)
{

	ca_l2_mcast_group_members_t *pl2mcGMember=&rg_db.systemGlobal.l2mcGMember;
	ca_l2_mcast_group_members_t *pl2mcGMember_del=&rg_db.systemGlobal.l2mcGMember_2;
	rtk_rg_mcEngine_info_t 	*pMcEngineInfo;
	uint32 sourceAddress[4]={0,0,0,0};
	uint32 groupAddress [4]={0,0,0,0};
	int changeMemCnt=0;
	int changeMemCnt_del=0;
	uint64 reAddMsk=0;
	uint64 _flowPmsk=0;//include extpmsk
	int i=0;
	rtk_rg_mcEngine_action_t flowAction;

	_flowPmsk = (pFlowPathEntry->path3.out_portmask &((1 << RTK_RG_EXT_BASED_PORT)-1)) | (rg_db.extPortTbl[pFlowPathEntry->path3.out_ext_portmask_idx].extPortEnt.extpmask << RTK_RG_MAC_PORT_CPU_WLAN0_ROOT);
	bzero(pl2mcGMember, sizeof(ca_l2_mcast_group_members_t));
	bzero(pl2mcGMember_del, sizeof(ca_l2_mcast_group_members_t));
	bzero(&flowAction,sizeof(flowAction));
	flowAction.cvlanID = pFlowPathEntry->path3.out_cvlan_id;
	flowAction.cPri =pFlowPathEntry->path3.out_cpri;
	flowAction.smacTrans =pFlowPathEntry->path3.out_smac_trans;
	flowAction.userPri =pFlowPathEntry->path3.out_user_pri_act ? pFlowPathEntry->path3.out_user_priority : 0;
	flowAction.vlanActionMsk =TRUE;
	if(pFlowPathEntry->path3.out_cvid_format_act)
	{
		flowAction.vlanAction = pFlowPathEntry->path3.in_ctagif ? CA_L2_VLAN_TAG_ACTION_SWAP : CA_L2_VLAN_TAG_ACTION_PUSH;
	}
	else
	{
		flowAction.vlanAction = pFlowPathEntry->path3.in_ctagif ? CA_L2_VLAN_TAG_ACTION_POP : CA_L2_VLAN_TAG_ACTION_NOP;
	}


	if(pFlowPathEntry->path3.in_ipv4_or_ipv6)
	{
		memcpy(groupAddress, pFlowExtraInfo->v6Dip.ipv6_addr,sizeof(groupAddress) );
		memcpy(sourceAddress,pFlowExtraInfo->v6Sip.ipv6_addr,sizeof(sourceAddress));
	}
	else
	{
		sourceAddress[0]=pFlowPathEntry->path3.in_src_ipv4_addr;
		groupAddress[0]= pFlowPathEntry->path3.in_dst_ipv4_addr;
	}

	pMcEngineInfo = _rtk_rg_g3McEngineInfoFind((uint32*)groupAddress,(uint32*)sourceAddress,pFlowPathEntry->path3.in_ipv4_or_ipv6);
	if(pMcEngineInfo == NULL)
		return RT_ERR_RG_ENTRY_NOT_EXIST;

	if(pMcEngineInfo->mcHwRefCount==1)
	{
		ca_l3_mcast_entry_t l3mcEnt;
		ca_l3_mcast_group_members_t *pl3mcGMember=&rg_db.systemGlobal.l3mcGMember;

		int32 ret=0;
		bzero(&l3mcEnt, sizeof(l3mcEnt));
		bzero(pl3mcGMember, sizeof(ca_l3_mcast_group_members_t));

		if(pFlowPathEntry->path3.in_ipv4_or_ipv6==0)
		{
			l3mcEnt.group_ip_addr.afi = CA_IPV4;
			l3mcEnt.group_ip_addr.ip_addr.ipv4_addr = pFlowPathEntry->path3.in_dst_ipv4_addr;
			l3mcEnt.group_ip_addr.addr_len = 32;

		}
		else
		{
			l3mcEnt.group_ip_addr.afi = CA_IPV6;
			l3mcEnt.group_ip_addr.ip_addr.ipv6_addr[0] = ntohl( *(((uint32*)(&pFlowExtraInfo->v6Dip.ipv6_addr[0]))) );
			l3mcEnt.group_ip_addr.ip_addr.ipv6_addr[1] = ntohl( *(((uint32*)(&pFlowExtraInfo->v6Dip.ipv6_addr[4]))) );
			l3mcEnt.group_ip_addr.ip_addr.ipv6_addr[2] = ntohl( *(((uint32*)(&pFlowExtraInfo->v6Dip.ipv6_addr[8]))) );
			l3mcEnt.group_ip_addr.ip_addr.ipv6_addr[3] = ntohl( *(((uint32*)(&pFlowExtraInfo->v6Dip.ipv6_addr[12]))) );
			l3mcEnt.group_ip_addr.addr_len = 128;

		}

		ASSERT_EQ(ret = ca_l3_mcast_group_add(G3_DEF_DEVID, &l3mcEnt), CA_E_OK);
		TRACE("Add L3 mcast group idx %d, ret = %d", l3mcEnt.mcg_id, ret);

		pl3mcGMember->mcg_id = l3mcEnt.mcg_id;
		pl3mcGMember->member_count = 1;
		pl3mcGMember->member[0].member_intf = _rtk_rg_g3_generic_intf_index_get(TRUE,rg_db.systemGlobal.interfaceInfo[0].lanWan_groupIdx_for_genericIntf,0,FAIL);
		ASSERT_EQ(ret = ca_l3_mcast_member_add(G3_DEF_DEVID, pl3mcGMember), CA_E_OK);
		TRACE("Add intf %d to L3 mcast group %d, ret = %d", pl3mcGMember->member[0].member_intf, pl3mcGMember->mcg_id, ret);
		if(pl3mcGMember->mcg_id!=pMcEngineInfo->l3mcgid)
			WARNING("pl3mcGMember->mcg_id!=pMcEngineInfo->l3mcgid  Fixme");

	}
	else
	{
		TRACE("Ingore add L3 mcast mcHwRefCount=%d",pMcEngineInfo->mcHwRefCount);
	}


	/*change L2 MC members*/
	/*add new members*/
	changeMemCnt = 0;
	changeMemCnt_del=0;
	pl2mcGMember->mcg_id = pMcEngineInfo->l2mcgid;
	pl2mcGMember_del->mcg_id = pMcEngineInfo->l2mcgid;

	TABLE("p3Pmsk=%x p4Pmsk=%x out_portmask=%x in_multiple_act=%d",pMcEngineInfo->p3Pmsk,pMcEngineInfo->p4Pmsk,_flowPmsk,pFlowPathEntry->path3.in_multiple_act);
	if(pFlowPathEntry->path3.in_multiple_act==0)
		reAddMsk = (_flowPmsk & pMcEngineInfo->p4Pmsk) ;//re-add to path3
	else
		reAddMsk = (_flowPmsk & pMcEngineInfo->p3Pmsk) ;//re-add to path4


	for(i = 0; i<RTK_RG_PORT_MAX; i++)
	{
		if(RG_INVALID_PORT(i)) continue;

		if((((pMcEngineInfo->p3Pmsk& (1<<i))) && (_flowPmsk & (1<<i)) && pFlowPathEntry->path3.in_multiple_act==0) )
		{
			//add at same port check Action
			if(memcmp(&flowAction,&pMcEngineInfo->p3Action,sizeof(flowAction))==0)
			{
				continue;
			}
			else
			{
				_rtk_rg_set_ca_l2_mcast_group_members(pl2mcGMember_del,&changeMemCnt_del,&pMcEngineInfo->p3Action,i);
				IGMP("Path3 Port=%d Action change delete ",i);
				pMcEngineInfo->p3Pmsk&= ~(1<<i);
			}

		}
		if((((pMcEngineInfo->p4Pmsk& (1<<i))) && (_flowPmsk & (1<<i)) && pFlowPathEntry->path3.in_multiple_act==1) )
		{
			//add at same port check Action
			if(memcmp(&flowAction,&pMcEngineInfo->p4Action,sizeof(flowAction)==0))
			{
				continue;
			}
			else
			{
				_rtk_rg_set_ca_l2_mcast_group_members(pl2mcGMember_del,&changeMemCnt_del,&pMcEngineInfo->p4Action,i);
				IGMP("Path4 Port=%d Action change delete ",i);
				pMcEngineInfo->p4Pmsk&= ~(1<<i);
			}

		}


		if(((!(pMcEngineInfo->p3Pmsk& (1<<i))) && (_flowPmsk & (1<<i)) && pFlowPathEntry->path3.in_multiple_act==0) ||
		   ((!(pMcEngineInfo->p4Pmsk& (1<<i))) && (_flowPmsk & (1<<i)) && pFlowPathEntry->path3.in_multiple_act==1)   )
		{
			if(reAddMsk & (1<<i))
			{

				if(pFlowPathEntry->path3.in_multiple_act==0)
				{
					if(memcmp(&flowAction,&pMcEngineInfo->p4Action,sizeof(flowAction)==0))
					{
						continue; //ingore re-add to hw
					}
					else
					{
						_rtk_rg_set_ca_l2_mcast_group_members(pl2mcGMember_del,&changeMemCnt_del,&pMcEngineInfo->p4Action,i);
						IGMP("Find Port=%d re-add",i);
						pMcEngineInfo->p4Pmsk&= ~(1<<i);
					}

				}
				else
				{
					if(memcmp(&flowAction,&pMcEngineInfo->p3Action,sizeof(flowAction)==0))
					{
						continue; //ingore re-add to hw
					}
					else
					{
						_rtk_rg_set_ca_l2_mcast_group_members(pl2mcGMember_del,&changeMemCnt_del,&pMcEngineInfo->p3Action,i);
						IGMP("Find Port=%d re-add",i);
						pMcEngineInfo->p3Pmsk&= ~(1<<i);
					}
				}
			}
			TABLE("add port=%d",i);
			_rtk_rg_set_ca_l2_mcast_group_members(pl2mcGMember,&changeMemCnt,&flowAction,i);
		}
	}

	if(changeMemCnt_del)
	{
		TRACE("reAdd[0x%x] or actionChange Del %d member port(s) to MC L2 group %d (MC L3 group %d)",reAddMsk,changeMemCnt_del, pMcEngineInfo->l2mcgid, pMcEngineInfo->l3mcgid);
		ASSERT_EQ(ca_l2_mcast_member_delete(G3_DEF_DEVID, pl2mcGMember_del), CA_E_OK);
	}
	if(changeMemCnt)
	{
		TRACE("Add %d member port(s) to MC L2 group %d (MC L3 group %d)", changeMemCnt, pMcEngineInfo->l2mcgid, pMcEngineInfo->l3mcgid);
		ASSERT_EQ(ca_l2_mcast_member_add(G3_DEF_DEVID, pl2mcGMember), CA_E_OK);
	}
	 /*remove leave members*/
	 bzero(pl2mcGMember, sizeof(ca_l2_mcast_group_members_t));
	 bzero(pl2mcGMember_del, sizeof(ca_l2_mcast_group_members_t));
	 changeMemCnt = 0;
	 changeMemCnt_del=0;
	 pl2mcGMember->mcg_id = pMcEngineInfo->l2mcgid;
	 pl2mcGMember_del->mcg_id = pMcEngineInfo->l2mcgid;

	 for(i = 0; i<RTK_RG_PORT_MAX; i++)
	 {
	 	if(RG_INVALID_PORT(i)) continue;

		if(((pMcEngineInfo->p3Pmsk& (1<<i)) && (!(_flowPmsk & (1<<i))) && (pFlowPathEntry->path3.in_multiple_act==0) ) ||
		   ((pMcEngineInfo->p4Pmsk& (1<<i)) && (!(_flowPmsk & (1<<i))) && (pFlowPathEntry->path3.in_multiple_act==1) ) )
		{
			if(pMcEngineInfo->p3Pmsk& (1<<i))
			{
				_rtk_rg_set_ca_l2_mcast_group_members(pl2mcGMember_del,&changeMemCnt_del,&pMcEngineInfo->p3Action,i);
				IGMP("Del Port[%d] old by P3Action",i);
			}
			else if(pMcEngineInfo->p4Pmsk& (1<<i))
			{
				_rtk_rg_set_ca_l2_mcast_group_members(pl2mcGMember_del,&changeMemCnt_del,&pMcEngineInfo->p4Action,i);
				IGMP("Del Port[%d] old by P4Action",i);
			}
		}
	}

	if(changeMemCnt_del)
	{
		ASSERT_EQ(ca_l2_mcast_member_delete(G3_DEF_DEVID, pl2mcGMember_del), CA_E_OK);
		TRACE("Remove %d member port(s) to MC L2 group %d (MC L3 group %d)", changeMemCnt_del, pMcEngineInfo->l2mcgid, pMcEngineInfo->l3mcgid);
	}

	/*save current HW MC member port mask*/
	if(pFlowPathEntry->path3.in_multiple_act ==0)
	{
		memcpy(&pMcEngineInfo->p3Action,&flowAction,sizeof(pMcEngineInfo->p3Action));
		pMcEngineInfo->p3Pmsk= _flowPmsk ;
	}
	else
	{
		memcpy(&pMcEngineInfo->p4Action,&flowAction,sizeof(pMcEngineInfo->p4Action));
		pMcEngineInfo->p4Pmsk= _flowPmsk ;
	}

	return RT_ERR_RG_OK;

}


rtk_rg_err_code_t RTK_RG_ASIC_G3_FLOW_SET(uint32 idx, rtk_rg_table_flowEntry_t *pFlowPathEntry, rtk_rg_flow_extraInfo_t flowExtraInfo)
{
	int32 mainHash_hwFlowIdx = FAIL;
	ca_status_t ca_ret = CA_E_OK;
	ca_flow_t flow_config;
	int genIntf_idx=FAIL;
	bool outerSVlanEn = FALSE;
	uint16 outerSVlanId = 0, outerSVlanPri = 0, ounterTPID = rg_db.systemGlobal.tpid;
#if !defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
	rtk_rg_g3_mac_hostPolicing_info_t sa_hostPolicing_info;
	rtk_rg_g3_mac_hostPolicing_info_t da_hostPolicing_info;
#endif
	int isFlowIdByHp = 0; //flag used for identify if the flow id is decided by host policing
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
	bool fromLanPort = ((G3_LOOPBACK_P_START <= rg_db.lut[flowExtraInfo.lutSaIdx].rtk_lut.entry.l2UcEntry.port) && (rg_db.lut[flowExtraInfo.lutSaIdx].rtk_lut.entry.l2UcEntry.port <= G3_LOOPBACK_P_END))?TRUE:FALSE;
#endif

#if !defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
	//check host policing info by mac
	_rtk_rg_g3_get_mac_hostPolicing_info(&(rg_db.lut[flowExtraInfo.lutSaIdx].rtk_lut.entry.l2UcEntry.mac), &sa_hostPolicing_info);
	_rtk_rg_g3_get_mac_hostPolicing_info(&(rg_db.lut[flowExtraInfo.lutDaIdx].rtk_lut.entry.l2UcEntry.mac), &da_hostPolicing_info);
#endif

	if(flowExtraInfo.addSwOnly)
	{
		TRACE("Skip add G3 hw main hash flow...add sw only");
		mainHash_hwFlowIdx = FAIL;
	}
	else if(_rtk_rg_flow_isMulticast(*pFlowPathEntry, flowExtraInfo.path34_isGmac))
	{
		if(pFlowPathEntry->path3.in_pppoeif && flowExtraInfo.path34_isGmac==0)
			goto ADD_HW_FLOW;
		TRACE("Translate Multicast flow to G3 hw format");
		mainHash_hwFlowIdx = FAIL;
		_RTK_RG_ASIC_G3_MC_FLOW_SET(pFlowPathEntry,&flowExtraInfo);
	}
	else	// add hw flow
	{
ADD_HW_FLOW:
		memset(&flow_config, 0, sizeof(flow_config));
		// common setting
		flow_config.aging_time = 30; // don't care
		flow_config.actions.forward = CA_CLASSIFIER_FORWARD_INTERFACE;

		switch(pFlowPathEntry->path1.in_path)
		{
			case FB_PATH_12:
			{
				rtk_rg_asic_path1_entry_t *pFlowPath1 = (rtk_rg_asic_path1_entry_t *)&pFlowPathEntry->path1;

				TRACE("Translate flow path1/2 to G3 hw flow format");
				flow_config.key_type = RG_CA_FLOW_UC2TUPLE_BRIDGE;

				// Key: L2
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
				if(fromLanPort)
				{
					flow_config.key.l2_key.outer_vlan.vid = rg_db.lut[flowExtraInfo.lutSaIdx].rtk_lut.entry.l2UcEntry.port + G3_LOOPBACK_UPSTREAM_VID_BASE;
					flow_config.key.l2_key.outer_vlan.pri = 0;
					if((pFlowPath1->in_stagif + pFlowPath1->in_ctagif) == 2){		// double tag
						flow_config.key.l2_key.inner_vlan.vid = flowExtraInfo.igrSVID;
						flow_config.key.l2_key.inner_vlan.pri = flowExtraInfo.igrSPRI;
					}else{													// single tag or untag
						if(pFlowPath1->in_stagif){
							//flow_config.key.l2_key.outer_vlan.tpid = 0x8100;
							flow_config.key.l2_key.inner_vlan.vid = flowExtraInfo.igrSVID;
							flow_config.key.l2_key.inner_vlan.pri = flowExtraInfo.igrSPRI;
						}
						if(pFlowPath1->in_ctagif){
							//flow_config.key.l2_key.outer_vlan.tpid = 0x8100;
							flow_config.key.l2_key.inner_vlan.vid = flowExtraInfo.igrCVID;
							flow_config.key.l2_key.inner_vlan.pri = pFlowPath1->in_cvlan_pri;
						}
					}
					flow_config.key.l2_key.o_lspid = G3_LOOPBACK_P_NEWSPA;
				}
				else
#endif

			{
				if((pFlowPath1->in_stagif + pFlowPath1->in_ctagif) == 2){		// double tag
					flow_config.key.l2_key.outer_vlan.vid = flowExtraInfo.igrSVID;
					flow_config.key.l2_key.outer_vlan.pri = flowExtraInfo.igrSPRI;
					flow_config.key.l2_key.inner_vlan.vid = flowExtraInfo.igrCVID;
					flow_config.key.l2_key.inner_vlan.pri = pFlowPath1->in_cvlan_pri;
				}else{													// single tag or untag
					if(pFlowPath1->in_stagif){
						//flow_config.key.l2_key.outer_vlan.tpid = 0x8100;
						flow_config.key.l2_key.outer_vlan.vid = flowExtraInfo.igrSVID;
						flow_config.key.l2_key.outer_vlan.pri = flowExtraInfo.igrSPRI;
					}
					if(pFlowPath1->in_ctagif){
						//flow_config.key.l2_key.outer_vlan.tpid = 0x8100;
						flow_config.key.l2_key.outer_vlan.vid = flowExtraInfo.igrCVID;
						flow_config.key.l2_key.outer_vlan.pri = pFlowPath1->in_cvlan_pri;
					}
				}
				//if(pFlowPath1->in_spa_check)		//force enabled to support unknown sa trap by main hash
					flow_config.key.l2_key.o_lspid = rg_db.lut[flowExtraInfo.lutSaIdx].rtk_lut.entry.l2UcEntry.port;
			}
				memcpy(&flow_config.key.l2_key.mac_sa, rg_db.lut[pFlowPath1->in_smac_lut_idx].rtk_lut.entry.l2UcEntry.mac.octet, ETHER_ADDR_LEN);
				memcpy(&flow_config.key.l2_key.mac_da, rg_db.lut[pFlowPath1->in_dmac_lut_idx].rtk_lut.entry.l2UcEntry.mac.octet, ETHER_ADDR_LEN);

				// Key: L3
				if(pFlowPath1->in_tos_check)
				{
					flow_config.key.l3_key.dscp = (pFlowPath1->in_tos >>2)&0x3f;
					flow_config.key.l3_key.ecn= pFlowPath1->in_tos&0x3;
				}

				// Action: L2
				if(!rg_kernel.autoTestMode){
					// normal mode
					if((pFlowPath1->in_spa!=RTK_RG_MAC_PORT_PON) && pFlowPath1->in_out_stream_idx_check_act){
						outerSVlanEn = TRUE;
						outerSVlanId = pFlowPath1->in_out_stream_idx;
						outerSVlanPri = pFlowPath1->out_cpri;
					}
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
					else if(rg_db.lut[flowExtraInfo.lutDaIdx].rtk_lut.entry.l2UcEntry.port <= G3_LOOPBACK_P_END) {
						outerSVlanEn = TRUE;
						outerSVlanId = G3_LOOPBACK_DOWNSTREAM_VID(rg_db.lut[flowExtraInfo.lutDaIdx].rtk_lut.entry.l2UcEntry.port, pFlowPath1->out_user_priority);
						outerSVlanPri = 0;
						ounterTPID = 0x8100;
					}
#endif
				}else{
					// hwnat auto test mode
					if(pFlowPath1->out_svid_format_act){
						outerSVlanEn = TRUE;
						outerSVlanId = pFlowPath1->out_svlan_id;
						outerSVlanPri = pFlowPath1->out_spri;
					}
					if(pFlowPath1->in_out_stream_idx_check_act){
						DEBUG("SID action is not supported in auto test env. bypass it");
					}
				}
				if( (outerSVlanEn + pFlowPath1->out_cvid_format_act) == 2){		// double tag
					flow_config.actions.options.masks.outer_vlan_act = TRUE;
					flow_config.actions.options.masks.outer_dot1p = TRUE;
					flow_config.actions.options.masks.outer_dei = TRUE;
					flow_config.actions.options.masks.outer_tpid = TRUE;
					flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_SET;
					flow_config.actions.options.outer_vid = outerSVlanId;
					flow_config.actions.options.outer_dot1p = outerSVlanPri;
					flow_config.actions.options.outer_dei = 0;
					flow_config.actions.options.outer_tpid = ounterTPID;

					flow_config.actions.options.masks.inner_vlan_act = TRUE;
					flow_config.actions.options.masks.inner_dot1p = TRUE;
					flow_config.actions.options.masks.inner_dei = TRUE;
					flow_config.actions.options.inner_vlan_act = CA_FLOW_VLAN_ACTION_SET;
					flow_config.actions.options.inner_vid = pFlowPath1->out_cvlan_id;
					flow_config.actions.options.inner_dot1p = pFlowPath1->out_cpri;
					flow_config.actions.options.inner_dei = 0;
				}else{
					if(outerSVlanEn){
						flow_config.actions.options.masks.outer_vlan_act = TRUE;
						flow_config.actions.options.masks.outer_dot1p = TRUE;
						flow_config.actions.options.masks.outer_tpid = TRUE;
						flow_config.actions.options.masks.outer_dei = TRUE;
						flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_SET;
						flow_config.actions.options.outer_vid = outerSVlanId;
						flow_config.actions.options.outer_dot1p = outerSVlanPri;
						flow_config.actions.options.outer_tpid = ounterTPID;
						flow_config.actions.options.outer_dei = 0;
					}else if(pFlowPath1->out_cvid_format_act){
						flow_config.actions.options.masks.outer_vlan_act = TRUE;
						flow_config.actions.options.masks.outer_dot1p = TRUE;
						flow_config.actions.options.masks.outer_dei = TRUE;
						flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_SET;
						flow_config.actions.options.outer_vid = pFlowPath1->out_cvlan_id;
						flow_config.actions.options.outer_dot1p = pFlowPath1->out_cpri;
						flow_config.actions.options.outer_dei = 0;
					}else{
						flow_config.actions.options.masks.outer_vlan_act = TRUE;
						flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_POP;
					}
				}

				if(pFlowPath1->out_smac_trans){
					flow_config.actions.options.masks.decrement_ttl = rg_db.systemGlobal.fbGlobalState[FB_GLOBAL_TTL_1];
					flow_config.actions.options.masks.mac_sa = TRUE;
					memcpy(&flow_config.actions.options.mac_sa, rg_db.netif[pFlowPath1->out_intf_idx].rtk_netif.gateway_mac.octet, ETHER_ADDR_LEN);
				}else{
					flow_config.actions.options.masks.decrement_ttl = FALSE;
					flow_config.actions.options.masks.mac_sa = TRUE;
					memcpy(&flow_config.actions.options.mac_sa, rg_db.lut[flowExtraInfo.lutSaIdx].rtk_lut.entry.l2UcEntry.mac.octet, ETHER_ADDR_LEN);
				}
				if(pFlowPath1->out_dmac_trans){
					flow_config.actions.options.masks.mac_da = TRUE;
					memcpy(&flow_config.actions.options.mac_da, rg_db.lut[flowExtraInfo.lutDaIdx].rtk_lut.entry.l2UcEntry.mac.octet, ETHER_ADDR_LEN);
				}

				// Action: L3
				flow_config.actions.options.masks.dscp = pFlowPath1->out_dscp_act;
				flow_config.actions.options.dscp = pFlowPath1->out_dscp;

				// Action: Others
				flow_config.actions.options.masks.cos = pFlowPath1->out_user_pri_act;
				flow_config.actions.options.cos = pFlowPath1->out_user_priority;

				flow_config.actions.dest.port = rg_db.lut[flowExtraInfo.lutDaIdx].rtk_lut.entry.l2UcEntry.port;
				// Decide generic interface
				genIntf_idx = _rtk_rg_g3_generic_intf_index_get((rg_db.systemGlobal.interfaceInfo[pFlowPath1->out_intf_idx].storedInfo.is_wan)?FALSE:TRUE,
																rg_db.systemGlobal.interfaceInfo[pFlowPath1->out_intf_idx].lanWan_groupIdx_for_genericIntf,
																rg_db.lut[flowExtraInfo.lutDaIdx].rtk_lut.entry.l2UcEntry.port,
																rg_db.lut[flowExtraInfo.lutDaIdx].wlan_device_idx);
				if(genIntf_idx==FAIL) return RT_ERR_RG_FAILED;
				flow_config.actions.dest.intf = genIntf_idx;

				flow_config.actions.options.masks.flow_id= pFlowPath1->out_share_meter_act;
				flow_config.actions.options.flow_id = pFlowPath1->out_share_meter_idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMTR;
				if(!pFlowPath1->out_share_meter_act)
				{
					flow_config.actions.options.masks.flow_id = pFlowPath1->out_flow_counter_act;
					flow_config.actions.options.flow_id = pFlowPath1->out_flow_counter_idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMIB;
				}
			}
				break;
			case FB_PATH_34:
			{
				rtk_rg_asic_path3_entry_t *pFlowPath3 = (rtk_rg_asic_path3_entry_t *)&pFlowPathEntry->path3;

				TRACE("Translate flow path3/4 to G3 hw flow format");
				if( (1<<rg_db.lut[flowExtraInfo.lutSaIdx].rtk_lut.entry.l2UcEntry.port) & rg_db.systemGlobal.wanPortMask.portmask)
					flow_config.key_type = RG_CA_FLOW_UC5TUPLE_DS;
				else
					flow_config.key_type = RG_CA_FLOW_UC5TUPLE_US;

				// Key: L2
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
				if(fromLanPort)
				{
					flow_config.key.l2_key.o_lspid = G3_LOOPBACK_P_NEWSPA;

					flow_config.key.l2_key.outer_vlan.vid = rg_db.lut[flowExtraInfo.lutSaIdx].rtk_lut.entry.l2UcEntry.port + G3_LOOPBACK_UPSTREAM_VID_BASE;
					flow_config.key.l2_key.outer_vlan.pri = 0;
					if((pFlowPath3->in_stagif + pFlowPath3->in_ctagif) == 2){		// double tag
						flow_config.key.l2_key.inner_vlan.vid = flowExtraInfo.igrSVID;
						flow_config.key.l2_key.inner_vlan.pri = flowExtraInfo.igrSPRI;
					}else{													// single tag or untag
						if(pFlowPath3->in_stagif){
							//flow_config.key.l2_key.outer_vlan.tpid = 0x8100;
							flow_config.key.l2_key.inner_vlan.vid = flowExtraInfo.igrSVID;
							flow_config.key.l2_key.inner_vlan.pri = flowExtraInfo.igrSPRI;
						}
						if(pFlowPath3->in_ctagif){
							//flow_config.key.l2_key.outer_vlan.tpid = 0x8100;
							flow_config.key.l2_key.inner_vlan.vid = flowExtraInfo.igrCVID;
							flow_config.key.l2_key.inner_vlan.pri = pFlowPath3->in_cvlan_pri;
						}
					}
				}
				else
#endif

			{
				flow_config.key.l2_key.o_lspid = rg_db.lut[flowExtraInfo.lutSaIdx].rtk_lut.entry.l2UcEntry.port;		//force enabled to support unknown sa trap by main hash

				if((pFlowPath3->in_stagif + pFlowPath3->in_ctagif) == 2){		// double tag
					flow_config.key.l2_key.outer_vlan.vid = flowExtraInfo.igrSVID;
					flow_config.key.l2_key.outer_vlan.pri = flowExtraInfo.igrSPRI;
					flow_config.key.l2_key.inner_vlan.vid = flowExtraInfo.igrCVID;
					flow_config.key.l2_key.inner_vlan.pri = pFlowPath3->in_cvlan_pri;
				}else{													// single tag or untag
					if(pFlowPath3->in_stagif){
						//flow_config.key.l2_key.outer_vlan.tpid = 0x8100;
						flow_config.key.l2_key.outer_vlan.vid = flowExtraInfo.igrSVID;
						flow_config.key.l2_key.outer_vlan.pri = flowExtraInfo.igrSPRI;
					}
					if(pFlowPath3->in_ctagif){
						//flow_config.key.l2_key.outer_vlan.tpid = 0x8100;
						flow_config.key.l2_key.outer_vlan.vid = flowExtraInfo.igrCVID;
						flow_config.key.l2_key.outer_vlan.pri = pFlowPath3->in_cvlan_pri;
					}
				}
			}

				// Key: L3
				flow_config.key.l3_key.ip_valid = TRUE;
				flow_config.key.l3_key.ip_version = pFlowPath3->in_ipv4_or_ipv6 ? 6 : 4;
				flow_config.key.l3_key.ip_protocol = pFlowPath3->in_l4proto ? RG_IP_PROTO_TCP : RG_IP_PROTO_UDP;
				if(pFlowPath3->in_tos_check)
				{
					flow_config.key.l3_key.dscp = (pFlowPath3->in_tos >>2)&0x3f;
					flow_config.key.l3_key.ecn	= pFlowPath3->in_tos&0x3;
				}

				if(pFlowPath3->in_ipv4_or_ipv6){
					flow_config.key.l3_key.ip_sa.afi = CA_IPV6;
					flow_config.key.l3_key.ip_da.afi = CA_IPV6;
					flow_config.key.l3_key.ip_sa.addr_len = 128;
					flow_config.key.l3_key.ip_da.addr_len = 128;
					flow_config.key.l3_key.ip_sa.ip_addr.ipv6_addr[0] =  ntohl(*(uint32*)&flowExtraInfo.v6Sip.ipv6_addr[0]);
					flow_config.key.l3_key.ip_sa.ip_addr.ipv6_addr[1] =  ntohl(*(uint32*)&flowExtraInfo.v6Sip.ipv6_addr[4]);
					flow_config.key.l3_key.ip_sa.ip_addr.ipv6_addr[2] =  ntohl(*(uint32*)&flowExtraInfo.v6Sip.ipv6_addr[8]);
					flow_config.key.l3_key.ip_sa.ip_addr.ipv6_addr[3] =  ntohl(*(uint32*)&flowExtraInfo.v6Sip.ipv6_addr[12]);
					flow_config.key.l3_key.ip_da.ip_addr.ipv6_addr[0] =  ntohl(*(uint32*)&flowExtraInfo.v6Dip.ipv6_addr[0]);
					flow_config.key.l3_key.ip_da.ip_addr.ipv6_addr[1] =  ntohl(*(uint32*)&flowExtraInfo.v6Dip.ipv6_addr[4]);
					flow_config.key.l3_key.ip_da.ip_addr.ipv6_addr[2] =  ntohl(*(uint32*)&flowExtraInfo.v6Dip.ipv6_addr[8]);
					flow_config.key.l3_key.ip_da.ip_addr.ipv6_addr[3] =  ntohl(*(uint32*)&flowExtraInfo.v6Dip.ipv6_addr[12]);

				}else{
					flow_config.key.l3_key.ip_sa.afi = CA_IPV4;
					flow_config.key.l3_key.ip_da.afi = CA_IPV4;
					flow_config.key.l3_key.ip_sa.ip_addr.ipv4_addr = pFlowPath3->in_src_ipv4_addr;
					flow_config.key.l3_key.ip_da.ip_addr.ipv4_addr = pFlowPath3->in_dst_ipv4_addr;
					flow_config.key.l3_key.ip_sa.addr_len = 32;
					flow_config.key.l3_key.ip_da.addr_len = 32;
				}

				// Key: L4
				flow_config.key.l4_key.src_l4_port = pFlowPath3->in_l4_src_port;
				flow_config.key.l4_key.dst_l4_port = pFlowPath3->in_l4_dst_port;


				// Action: L2
				if(!rg_kernel.autoTestMode){
					// normal mode
					if(pFlowPath3->out_stream_idx_act){
						outerSVlanEn = TRUE;
						outerSVlanId = pFlowPath3->out_stream_idx;
						outerSVlanPri = pFlowPath3->out_cpri;
					}
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
					else if(rg_db.lut[flowExtraInfo.lutDaIdx].rtk_lut.entry.l2UcEntry.port <= G3_LOOPBACK_P_END) {
						outerSVlanEn = TRUE;
						outerSVlanId = G3_LOOPBACK_DOWNSTREAM_VID(rg_db.lut[flowExtraInfo.lutDaIdx].rtk_lut.entry.l2UcEntry.port, pFlowPath3->out_user_priority);
						outerSVlanPri = 0;
						ounterTPID = 0x8100;
					}
#endif
				}else{
					// hwnat auto test mode
					if(pFlowPath3->out_svid_format_act){
						outerSVlanEn = TRUE;
						outerSVlanId = pFlowPath3->out_svlan_id;
						outerSVlanPri = pFlowPath3->out_spri;
					}
					if(pFlowPath3->out_stream_idx_act){
						DEBUG("SID action is not supported in auto test env. bypass it");
					}
				}

				if((outerSVlanEn + pFlowPath3->out_cvid_format_act) == 2){		// double tag
					flow_config.actions.options.masks.outer_vlan_act = TRUE;
					flow_config.actions.options.masks.outer_dot1p = TRUE;
					flow_config.actions.options.masks.outer_dei = TRUE;
					flow_config.actions.options.masks.outer_tpid = TRUE;
					flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_SET;
					flow_config.actions.options.outer_vid = outerSVlanId;
					flow_config.actions.options.outer_dot1p = outerSVlanPri;
					flow_config.actions.options.outer_dei = 0;
					flow_config.actions.options.outer_tpid = ounterTPID;

					flow_config.actions.options.masks.inner_vlan_act = TRUE;
					flow_config.actions.options.masks.inner_dot1p = TRUE;
					flow_config.actions.options.masks.inner_dei = TRUE;
					flow_config.actions.options.inner_vlan_act = CA_FLOW_VLAN_ACTION_SET;
					flow_config.actions.options.inner_vid = pFlowPath3->out_cvlan_id;
					flow_config.actions.options.inner_dot1p = pFlowPath3->out_cpri;
					flow_config.actions.options.inner_dei = 0;
				}else{
					if(outerSVlanEn){
						flow_config.actions.options.masks.outer_vlan_act = TRUE;
						flow_config.actions.options.masks.outer_dot1p = TRUE;
						flow_config.actions.options.masks.outer_tpid = TRUE;
						flow_config.actions.options.masks.outer_dei = TRUE;
						flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_SET;
						flow_config.actions.options.outer_vid = outerSVlanId;
						flow_config.actions.options.outer_dot1p = outerSVlanPri;
						flow_config.actions.options.outer_tpid = ounterTPID;
						flow_config.actions.options.outer_dei = 0;
					}else if(pFlowPath3->out_cvid_format_act){
						flow_config.actions.options.masks.outer_vlan_act = TRUE;
						flow_config.actions.options.masks.outer_dot1p = TRUE;
						flow_config.actions.options.masks.outer_dei = TRUE;
						flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_SET;
						flow_config.actions.options.outer_vid = pFlowPath3->out_cvlan_id;
						flow_config.actions.options.outer_dot1p = pFlowPath3->out_cpri;
						flow_config.actions.options.outer_dei = 0;
					}else{
						flow_config.actions.options.masks.outer_vlan_act = TRUE;
						flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_POP;
					}
				}

				if(pFlowPath3->out_smac_trans){
					flow_config.actions.options.masks.decrement_ttl = rg_db.systemGlobal.fbGlobalState[FB_GLOBAL_TTL_1];
					flow_config.actions.options.masks.mac_sa = TRUE;
					memcpy(&flow_config.actions.options.mac_sa, rg_db.netif[pFlowPath3->out_intf_idx].rtk_netif.gateway_mac.octet, ETHER_ADDR_LEN);
				}else{
					flow_config.actions.options.masks.decrement_ttl = FALSE;
					flow_config.actions.options.masks.mac_sa = TRUE;
					memcpy(&flow_config.actions.options.mac_sa, rg_db.lut[flowExtraInfo.lutSaIdx].rtk_lut.entry.l2UcEntry.mac.octet, ETHER_ADDR_LEN);
				}

				if(pFlowPath3->out_dmac_trans){

					flow_config.actions.options.masks.mac_da = TRUE;
					memcpy(&flow_config.actions.options.mac_da, rg_db.lut[flowExtraInfo.lutDaIdx].rtk_lut.entry.l2UcEntry.mac.octet, ETHER_ADDR_LEN);
				}

				// Action: L3
				flow_config.actions.options.masks.dscp = pFlowPath3->out_dscp_act;
				flow_config.actions.options.dscp = pFlowPath3->out_dscp;


				// Action: Others
				flow_config.actions.options.masks.cos = pFlowPath3->out_user_pri_act;
				flow_config.actions.options.cos = pFlowPath3->out_user_priority;

				flow_config.actions.dest.port = rg_db.lut[flowExtraInfo.lutDaIdx].rtk_lut.entry.l2UcEntry.port;
				// Decide generic interface
				genIntf_idx = _rtk_rg_g3_generic_intf_index_get((rg_db.systemGlobal.interfaceInfo[pFlowPath3->out_intf_idx].storedInfo.is_wan)?FALSE:TRUE,
																rg_db.systemGlobal.interfaceInfo[pFlowPath3->out_intf_idx].lanWan_groupIdx_for_genericIntf,
																rg_db.lut[flowExtraInfo.lutDaIdx].rtk_lut.entry.l2UcEntry.port,
																rg_db.lut[flowExtraInfo.lutDaIdx].wlan_device_idx);
				if(genIntf_idx==FAIL) return RT_ERR_RG_FAILED;
				flow_config.actions.dest.intf = genIntf_idx;

				flow_config.actions.options.masks.flow_id= pFlowPath3->out_share_meter_act;
				flow_config.actions.options.flow_id = pFlowPath3->out_share_meter_idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMTR;
				if(!pFlowPath3->out_share_meter_act)
				{
					flow_config.actions.options.masks.flow_id = pFlowPath3->out_flow_counter_act;
					flow_config.actions.options.flow_id = pFlowPath3->out_flow_counter_idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMIB;
				}

			}
				break;
			case FB_PATH_5:
			{
				rtk_rg_asic_path5_entry_t *pFlowPath5 = (rtk_rg_asic_path5_entry_t *)&pFlowPathEntry->path5;

				TRACE("Translate flow path5 to G3 hw flow format");
				if( (1<<rg_db.lut[flowExtraInfo.lutSaIdx].rtk_lut.entry.l2UcEntry.port) & rg_db.systemGlobal.wanPortMask.portmask)
					flow_config.key_type = RG_CA_FLOW_UC5TUPLE_DS;
				else
					flow_config.key_type = RG_CA_FLOW_UC5TUPLE_US;

				// Key: L2
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
				if(fromLanPort)
				{
					flow_config.key.l2_key.o_lspid = G3_LOOPBACK_P_NEWSPA;
					flow_config.key.l2_key.outer_vlan.vid = rg_db.lut[flowExtraInfo.lutSaIdx].rtk_lut.entry.l2UcEntry.port + G3_LOOPBACK_UPSTREAM_VID_BASE;
					flow_config.key.l2_key.outer_vlan.pri = 0;
					if((pFlowPath5->in_stagif + pFlowPath5->in_ctagif) == 2){		// double tag
						flow_config.key.l2_key.inner_vlan.vid = flowExtraInfo.igrSVID;
						flow_config.key.l2_key.inner_vlan.pri = flowExtraInfo.igrSPRI;
					}else{													// single tag or untag
						if(pFlowPath5->in_stagif){
							//flow_config.key.l2_key.outer_vlan.tpid = 0x8100;
							flow_config.key.l2_key.inner_vlan.vid = flowExtraInfo.igrSVID;
							flow_config.key.l2_key.inner_vlan.pri = flowExtraInfo.igrSPRI;
						}
						if(pFlowPath5->in_ctagif){
							//flow_config.key.l2_key.outer_vlan.tpid = 0x8100;
							flow_config.key.l2_key.inner_vlan.vid = flowExtraInfo.igrCVID;
							flow_config.key.l2_key.inner_vlan.pri = pFlowPath5->in_cvlan_pri;
						}
					}
				}
				else
#endif
			{
				flow_config.key.l2_key.o_lspid = rg_db.lut[flowExtraInfo.lutSaIdx].rtk_lut.entry.l2UcEntry.port;		//force enabled to support unknown sa trap by main hash

				if((pFlowPath5->in_stagif + pFlowPath5->in_ctagif) == 2){		// double tag
					flow_config.key.l2_key.outer_vlan.vid = flowExtraInfo.igrSVID;
					flow_config.key.l2_key.outer_vlan.pri = flowExtraInfo.igrSPRI;
					flow_config.key.l2_key.inner_vlan.vid = flowExtraInfo.igrCVID;
					flow_config.key.l2_key.inner_vlan.pri = pFlowPath5->in_cvlan_pri;
				}else{													// single tag or untag
					if(pFlowPath5->in_stagif){
						//flow_config.key.l2_key.outer_vlan.tpid = 0x8100;
						flow_config.key.l2_key.outer_vlan.vid = flowExtraInfo.igrSVID;
						flow_config.key.l2_key.outer_vlan.pri = flowExtraInfo.igrSPRI;
					}
					if(pFlowPath5->in_ctagif){
						//flow_config.key.l2_key.outer_vlan.tpid = 0x8100;
						flow_config.key.l2_key.outer_vlan.vid = flowExtraInfo.igrCVID;
						flow_config.key.l2_key.outer_vlan.pri = pFlowPath5->in_cvlan_pri;
					}
				}
			}


				// Key: L3
				flow_config.key.l3_key.ip_valid = TRUE;
				flow_config.key.l3_key.ip_version = pFlowPath5->in_ipv4_or_ipv6 ? 6 : 4;
				flow_config.key.l3_key.ip_protocol = pFlowPath5->in_l4proto ? RG_IP_PROTO_TCP : RG_IP_PROTO_UDP;
				if(pFlowPath5->in_tos_check)
				{
					flow_config.key.l3_key.dscp = (pFlowPath5->in_tos >>2)&0x3f;
					flow_config.key.l3_key.ecn	= pFlowPath5->in_tos&0x3;
				}

				if(pFlowPath5->in_ipv4_or_ipv6){
					flow_config.key.l3_key.ip_sa.afi = CA_IPV6;
					flow_config.key.l3_key.ip_da.afi = CA_IPV6;
					flow_config.key.l3_key.ip_sa.addr_len = 128;
					flow_config.key.l3_key.ip_da.addr_len = 128;
					flow_config.key.l3_key.ip_sa.ip_addr.ipv6_addr[0] =  ntohl(*(uint32*)&flowExtraInfo.v6Sip.ipv6_addr[0]);
					flow_config.key.l3_key.ip_sa.ip_addr.ipv6_addr[1] =  ntohl(*(uint32*)&flowExtraInfo.v6Sip.ipv6_addr[4]);
					flow_config.key.l3_key.ip_sa.ip_addr.ipv6_addr[2] =  ntohl(*(uint32*)&flowExtraInfo.v6Sip.ipv6_addr[8]);
					flow_config.key.l3_key.ip_sa.ip_addr.ipv6_addr[3] =  ntohl(*(uint32*)&flowExtraInfo.v6Sip.ipv6_addr[12]);
					flow_config.key.l3_key.ip_da.ip_addr.ipv6_addr[0] =  ntohl(*(uint32*)&flowExtraInfo.v6Dip.ipv6_addr[0]);
					flow_config.key.l3_key.ip_da.ip_addr.ipv6_addr[1] =  ntohl(*(uint32*)&flowExtraInfo.v6Dip.ipv6_addr[4]);
					flow_config.key.l3_key.ip_da.ip_addr.ipv6_addr[2] =  ntohl(*(uint32*)&flowExtraInfo.v6Dip.ipv6_addr[8]);
					flow_config.key.l3_key.ip_da.ip_addr.ipv6_addr[3] =  ntohl(*(uint32*)&flowExtraInfo.v6Dip.ipv6_addr[12]);

				}else{
					flow_config.key.l3_key.ip_sa.afi = CA_IPV4;
					flow_config.key.l3_key.ip_da.afi = CA_IPV4;
					flow_config.key.l3_key.ip_sa.ip_addr.ipv4_addr = pFlowPath5->in_src_ipv4_addr;
					flow_config.key.l3_key.ip_sa.addr_len = 32;

					if(pFlowPath5->out_l4_act==1 && pFlowPath5->out_l4_direction==0)	//naptr
					{
						flow_config.key.l3_key.ip_da.ip_addr.ipv4_addr = rg_db.netif[pFlowPath5->in_intf_idx].rtk_netif.ipAddr;
						flow_config.key.l3_key.ip_da.addr_len = 32;
					}
					else	// napt, routing
					{
						flow_config.key.l3_key.ip_da.ip_addr.ipv4_addr = pFlowPath5->in_dst_ipv4_addr;
						flow_config.key.l3_key.ip_da.addr_len = 32;
					}
				}

				// Key: L4
				flow_config.key.l4_key.src_l4_port = pFlowPath5->in_l4_src_port;
				flow_config.key.l4_key.dst_l4_port = pFlowPath5->in_l4_dst_port;

				// Action: L2
				{
					// routing
					flow_config.actions.options.masks.decrement_ttl = rg_db.systemGlobal.fbGlobalState[FB_GLOBAL_TTL_1];

					flow_config.actions.options.masks.mac_sa = TRUE;
					memcpy(&flow_config.actions.options.mac_sa, rg_db.netif[pFlowPath5->out_intf_idx].rtk_netif.gateway_mac.octet, ETHER_ADDR_LEN);

					flow_config.actions.options.masks.mac_da = TRUE;
					memcpy(&flow_config.actions.options.mac_da, rg_db.lut[rg_db.indMacTbl[pFlowPath5->out_dmac_idx].indMacEnt.l2_idx].rtk_lut.entry.l2UcEntry.mac.octet, ETHER_ADDR_LEN);
				}
				if(!rg_kernel.autoTestMode){
					// normal mode
					if(pFlowPath5->out_stream_idx_act){
						outerSVlanEn = TRUE;
						outerSVlanId = pFlowPath5->out_stream_idx;
						outerSVlanPri = pFlowPath5->out_cpri;
					}
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
					else if(rg_db.lut[flowExtraInfo.lutDaIdx].rtk_lut.entry.l2UcEntry.port <= G3_LOOPBACK_P_END) {
						outerSVlanEn = TRUE;
						outerSVlanId = G3_LOOPBACK_DOWNSTREAM_VID(rg_db.lut[flowExtraInfo.lutDaIdx].rtk_lut.entry.l2UcEntry.port, pFlowPath5->out_user_priority);
						outerSVlanPri = 0;
						ounterTPID = 0x8100;
					}
#endif
				}else{
					// hwnat auto test mode
					if(pFlowPath5->out_svid_format_act){
						outerSVlanEn = TRUE;
						outerSVlanId = pFlowPath5->out_svlan_id;
						outerSVlanPri = pFlowPath5->out_spri;
					}
					if(pFlowPath5->out_stream_idx_act){
						DEBUG("SID action is not supported in auto test env. bypass it");
					}
				}
				if((outerSVlanEn + pFlowPath5->out_cvid_format_act) == 2){		// double tag = streamid + cvaln
					flow_config.actions.options.masks.outer_vlan_act = TRUE;
					flow_config.actions.options.masks.outer_dot1p = TRUE;
					flow_config.actions.options.masks.outer_dei = TRUE;
					flow_config.actions.options.masks.outer_tpid = TRUE;
					flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_SET;
					flow_config.actions.options.outer_vid = outerSVlanId;
					flow_config.actions.options.outer_dot1p = outerSVlanPri;
					flow_config.actions.options.outer_dei = 0;
					flow_config.actions.options.outer_tpid = ounterTPID;

					flow_config.actions.options.masks.inner_vlan_act = TRUE;
					flow_config.actions.options.masks.inner_dot1p = TRUE;
					flow_config.actions.options.masks.inner_dei = TRUE;
					flow_config.actions.options.inner_vlan_act = CA_FLOW_VLAN_ACTION_SET;
					flow_config.actions.options.inner_vid = pFlowPath5->out_cvlan_id;
					flow_config.actions.options.inner_dot1p = pFlowPath5->out_cpri;
					flow_config.actions.options.inner_dei = 0;
				}else{											// single tag or untag
					if(outerSVlanEn){
						flow_config.actions.options.masks.outer_vlan_act = TRUE;
						flow_config.actions.options.masks.outer_dot1p = TRUE;
						flow_config.actions.options.masks.outer_tpid = TRUE;
						flow_config.actions.options.masks.outer_dei = TRUE;
						flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_SET;
						flow_config.actions.options.outer_vid = outerSVlanId;
						flow_config.actions.options.outer_dot1p = outerSVlanPri;
						flow_config.actions.options.outer_tpid = ounterTPID;
						flow_config.actions.options.outer_dei = 0;
					}else if(pFlowPath5->out_cvid_format_act){
						flow_config.actions.options.masks.outer_vlan_act = TRUE;
						flow_config.actions.options.masks.outer_dot1p = TRUE;
						flow_config.actions.options.masks.outer_dei = TRUE;
						flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_SET;
						flow_config.actions.options.outer_vid = pFlowPath5->out_cvlan_id;
						flow_config.actions.options.outer_dot1p = pFlowPath5->out_cpri;
						flow_config.actions.options.outer_dei = 0;
					}else{
						flow_config.actions.options.masks.outer_vlan_act = TRUE;
						flow_config.actions.options.outer_vlan_act = CA_FLOW_VLAN_ACTION_POP;
					}
				}

				if(rg_db.systemGlobal.interfaceInfo[pFlowPath5->out_intf_idx].storedInfo.is_wan==1)
				{
					rtk_rg_wan_type_t wan_type = rg_db.systemGlobal.interfaceInfo[pFlowPath5->out_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type;
					if(wan_type==RTK_RG_PPPoE || wan_type==RTK_RG_PPPoE_DSLITE)
					{
						flow_config.actions.options.masks.egress_pppoe_action = TRUE;
						flow_config.actions.options.pppoe_session_id =	rg_db.pppoe[rg_db.systemGlobal.interfaceInfo[pFlowPath5->out_intf_idx].storedInfo.wan_intf.pppoe_idx].rtk_pppoe.sessionID;
					}
				}
				else	// to lan
				{
					flow_config.actions.options.masks.egress_pppoe_action = TRUE;
					flow_config.actions.options.pppoe_session_id =	0;
				}

				// Action: L3 L4
				flow_config.actions.options.masks.dscp = pFlowPath5->out_dscp_act;
				flow_config.actions.options.dscp = pFlowPath5->out_dscp;

				if(pFlowPath5->out_l4_act){
					// NAPT
					if(pFlowPath5->out_l4_direction){
						// outbound
						flow_config.actions.options.masks.ip_sa = TRUE;
						flow_config.actions.options.masks.src_l4_port = TRUE;
						flow_config.actions.options.ip_sa.afi = CA_IPV4;
						flow_config.actions.options.ip_sa.ip_addr.ipv4_addr = rg_db.netif[pFlowPath5->out_intf_idx].rtk_netif.ipAddr;
						flow_config.actions.options.ip_sa.addr_len = 32;
						flow_config.actions.options.src_l4_port = pFlowPath5->out_l4_port;
					}else{
						// inbound
						flow_config.actions.options.masks.ip_da = TRUE;
						flow_config.actions.options.masks.dst_l4_port = TRUE;
						flow_config.actions.options.ip_da.afi = CA_IPV4;
						flow_config.actions.options.ip_da.ip_addr.ipv4_addr = pFlowPath5->out_dst_ipv4_addr;
						flow_config.actions.options.ip_da.addr_len = 32;
						flow_config.actions.options.dst_l4_port = pFlowPath5->out_l4_port;
					}
				}

				// Action: Others
				flow_config.actions.options.masks.cos = pFlowPath5->out_user_pri_act;
				flow_config.actions.options.cos = pFlowPath5->out_user_priority;

				flow_config.actions.dest.port = rg_db.lut[flowExtraInfo.lutDaIdx].rtk_lut.entry.l2UcEntry.port;
				// Decide generic interface
				genIntf_idx = _rtk_rg_g3_generic_intf_index_get((rg_db.systemGlobal.interfaceInfo[pFlowPath5->out_intf_idx].storedInfo.is_wan)?FALSE:TRUE,
																rg_db.systemGlobal.interfaceInfo[pFlowPath5->out_intf_idx].lanWan_groupIdx_for_genericIntf,
																rg_db.lut[flowExtraInfo.lutDaIdx].rtk_lut.entry.l2UcEntry.port,
																rg_db.lut[flowExtraInfo.lutDaIdx].wlan_device_idx);
				if(genIntf_idx==FAIL) return RT_ERR_RG_FAILED;
				flow_config.actions.dest.intf = genIntf_idx;

				flow_config.actions.options.masks.flow_id = pFlowPath5->out_share_meter_act;
				flow_config.actions.options.flow_id = pFlowPath5->out_share_meter_idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMTR;
				if(!pFlowPath5->out_share_meter_act)
				{
					flow_config.actions.options.masks.flow_id = pFlowPath5->out_flow_counter_act;
					flow_config.actions.options.flow_id = pFlowPath5->out_flow_counter_idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMIB;
				}

			}
				break;
			case FB_PATH_6:
				WARNING("[G3] No need to add Path6 to main hash flow\n");
				return RT_ERR_RG_CHIP_NOT_SUPPORT;
			default:
				return RT_ERR_RG_CHIP_NOT_SUPPORT;
		}

		if(rg_db.lut[flowExtraInfo.lutDaIdx].wlan_device_idx>=0)
		{
			rtk_rg_mbssidDev_t wlanDevIdx = rg_db.lut[flowExtraInfo.lutDaIdx].wlan_device_idx;
#if defined(CONFIG_GMAC1_USABLE) && (defined(CONFIG_WLAN0_2G_WLAN1_5G) || defined(CONFIG_BAND_2G_ON_WLAN0))
			//swap
			if(wlanDevIdx>=RG_RET_MBSSID_SLAVE_ROOT_INTF)
				wlanDevIdx -= RG_RET_MBSSID_SLAVE_ROOT_INTF;
			else
				wlanDevIdx += RG_RET_MBSSID_SLAVE_ROOT_INTF;
#endif
			switch(wlanDevIdx)
			{
				case RG_RET_MBSSID_MASTER_ROOT_INTF:
				case RG_RET_MBSSID_MASTER_VAP0_INTF:
				case RG_RET_MBSSID_MASTER_VAP1_INTF:
				case RG_RET_MBSSID_MASTER_VAP2_INTF:
				case RG_RET_MBSSID_MASTER_VAP3_INTF:
					break;
#if defined(CONFIG_WLAN_MBSSID_NUM) && (CONFIG_WLAN_MBSSID_NUM==7)
				case RG_RET_MBSSID_MASTER_VAP4_INTF:
					flow_config.actions.options.masks.sw_id = TRUE;
					flow_config.actions.options.sw_id[0] = RTK_RG_WIFI0_FLOWID_VAP4;
					break;
				case RG_RET_MBSSID_MASTER_VAP5_INTF:
					flow_config.actions.options.masks.sw_id = TRUE;
					flow_config.actions.options.sw_id[0] = RTK_RG_WIFI0_FLOWID_VAP5;
					break;
				case RG_RET_MBSSID_MASTER_VAP6_INTF:
					flow_config.actions.options.masks.sw_id = TRUE;
					flow_config.actions.options.sw_id[0] = RTK_RG_WIFI0_FLOWID_VAP6;
					break;
#endif
#if defined(CONFIG_RTL_WDS_SUPPORT)
				case RG_RET_MBSSID_MASTER_WDS0_INTF:
				case RG_RET_MBSSID_MASTER_WDS1_INTF:
				case RG_RET_MBSSID_MASTER_WDS2_INTF:
				case RG_RET_MBSSID_MASTER_WDS3_INTF:
				case RG_RET_MBSSID_MASTER_WDS4_INTF:
				case RG_RET_MBSSID_MASTER_WDS5_INTF:
				case RG_RET_MBSSID_MASTER_WDS6_INTF:
#ifdef CONFIG_RTL_MESH_SUPPORT
				case RG_RET_MBSSID_MASTER_MESH_INTF:
#else	// not CONFIG_RTL_MESH_SUPPORT
				case RG_RET_MBSSID_MASTER_WDS7_INTF:
#endif	// end CONFIG_RTL_MESH_SUPPORT
#else	// not CONFIG_RTL_WDS_SUPPORT
#ifdef CONFIG_RTL_MESH_SUPPORT
				case RG_RET_MBSSID_MASTER_MESH_INTF:
#endif	// end CONFIG_RTL_MESH_SUPPORT
#endif	// end CONFIG_RTL_WDS_SUPPORT
#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
				case RG_RET_MBSSID_MASTER_CLIENT_INTF:
#endif
					flow_config.actions.options.masks.sw_id = TRUE;
					flow_config.actions.options.sw_id[0] = RTK_RG_WIFI0_FLOWID_OTHER;
					break;
				case RG_RET_MBSSID_SLAVE_ROOT_INTF:
					flow_config.actions.options.masks.sw_id = TRUE;
					flow_config.actions.options.sw_id[0] = RTK_RG_WIFI1_FLOWID_ROOT;
					break;
				case RG_RET_MBSSID_SLAVE_VAP0_INTF:
					flow_config.actions.options.masks.sw_id = TRUE;
					flow_config.actions.options.sw_id[0] = RTK_RG_WIFI1_FLOWID_VAP0;
					break;
				case RG_RET_MBSSID_SLAVE_VAP1_INTF:
					flow_config.actions.options.masks.sw_id = TRUE;
					flow_config.actions.options.sw_id[0] = RTK_RG_WIFI1_FLOWID_VAP1;
					break;
				case RG_RET_MBSSID_SLAVE_VAP2_INTF:
					flow_config.actions.options.masks.sw_id = TRUE;
					flow_config.actions.options.sw_id[0] = RTK_RG_WIFI1_FLOWID_VAP2;
					break;
				case RG_RET_MBSSID_SLAVE_VAP3_INTF:
					flow_config.actions.options.masks.sw_id = TRUE;
					flow_config.actions.options.sw_id[0] = RTK_RG_WIFI1_FLOWID_VAP3;
					break;
#if defined(CONFIG_WLAN_MBSSID_NUM) && (CONFIG_WLAN_MBSSID_NUM==7)
				case RG_RET_MBSSID_SLAVE_VAP4_INTF:
					flow_config.actions.options.masks.sw_id = TRUE;
					flow_config.actions.options.sw_id[0] = RTK_RG_WIFI1_FLOWID_VAP4;
					break;
				case RG_RET_MBSSID_SLAVE_VAP5_INTF:
					flow_config.actions.options.masks.sw_id = TRUE;
					flow_config.actions.options.sw_id[0] = RTK_RG_WIFI1_FLOWID_VAP5;
					break;
				case RG_RET_MBSSID_SLAVE_VAP6_INTF:
					flow_config.actions.options.masks.sw_id = TRUE;
					flow_config.actions.options.sw_id[0] = RTK_RG_WIFI1_FLOWID_VAP6;
					break;
#endif
#if defined(CONFIG_RTL_WDS_SUPPORT)
				case RG_RET_MBSSID_SLAVE_WDS0_INTF:
				case RG_RET_MBSSID_SLAVE_WDS1_INTF:
				case RG_RET_MBSSID_SLAVE_WDS2_INTF:
				case RG_RET_MBSSID_SLAVE_WDS3_INTF:
				case RG_RET_MBSSID_SLAVE_WDS4_INTF:
				case RG_RET_MBSSID_SLAVE_WDS5_INTF:
				case RG_RET_MBSSID_SLAVE_WDS6_INTF:
				case RG_RET_MBSSID_SLAVE_WDS7_INTF:
#endif
#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
				case RG_RET_MBSSID_SLAVE_CLIENT_INTF:
#endif
					flow_config.actions.options.masks.sw_id = TRUE;
					flow_config.actions.options.sw_id[0] = RTK_RG_WIFI1_FLOWID_OTHER;
					break;
				default:
					break;
			}
		}
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
				/* use L2 CLS to acheive host policing */
#else

		/*
			update flow_id by host policing info
			if the flow hit multiple host policing action, flow_id decision rule: SMAC rate limit > DMAC rate limit > SMAC logging(RX) > DMAC logging(TX)
			Note. on some condition, the couter can not covered the packets belongs to this flow. (e.g. SMAC rate limit and DMAC logging at the same time)
		*/

		if(sa_hostPolicing_info.ingressRateLimit_en)
		{
			if(sa_hostPolicing_info.ingressRateLimit_mtrIdx == FAIL)
				WARNING("Ingress host policing is enabled, but no meter index!");

			if(!isFlowIdByHp)
			{
				flow_config.actions.options.masks.flow_id = sa_hostPolicing_info.ingressRateLimit_en;
				flow_config.actions.options.flow_id = sa_hostPolicing_info.ingressRateLimit_mtrIdx + G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_RX;
				isFlowIdByHp = 1;
				rg_db.flow[idx].hitHostPolingState = RTK_RG_G3_FLOW_HOSTPOLICING_HIT_SA;
			}
		}

		if(da_hostPolicing_info.egressRateLimit_en)
		{
			if(da_hostPolicing_info.egressRateLimit_mtrIdx == FAIL)
				WARNING("egress host policing is enabled, but no meter index!");

			if(!isFlowIdByHp)
			{
				flow_config.actions.options.masks.flow_id = da_hostPolicing_info.egressRateLimit_en;
				flow_config.actions.options.flow_id = da_hostPolicing_info.egressRateLimit_mtrIdx+ G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_TX;
				isFlowIdByHp = 1;
				rg_db.flow[idx].hitHostPolingState = RTK_RG_G3_FLOW_HOSTPOLICING_HIT_DA;
			}
		}


		if(sa_hostPolicing_info.logging_en)
		{
			if(sa_hostPolicing_info.logging_hostPolEntryIdx == FAIL)
				WARNING("SMAC host policing mib is enabled, but no host policing entry index!");
			else
			{
				/*Checking logging index stored in host policing entry if consistent or not (just for dedugging)*/
				if(sa_hostPolicing_info.ingressRateLimit_en)
				{
					//SMAC hits host policing entry for ingress rate limit, use policer id of rate limiting to get rx count
					if(rg_db.hostPoliceList[sa_hostPolicing_info.logging_hostPolEntryIdx].loggingRx_policerIdx != (sa_hostPolicing_info.ingressRateLimit_mtrIdx + G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_RX))
						WARNING("Rx logging index of hostPoliceList[%d] is %d, but it should be %d", sa_hostPolicing_info.logging_hostPolEntryIdx, rg_db.hostPoliceList[sa_hostPolicing_info.logging_hostPolEntryIdx].loggingRx_policerIdx, (sa_hostPolicing_info.ingressRateLimit_mtrIdx + G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_RX));
				}
				else
				{
					//SMAC does not hit host policing entry for ingress rate limit, use policer id of pure rx counter to get rx count
					if(rg_db.hostPoliceList[sa_hostPolicing_info.logging_hostPolEntryIdx].loggingRx_policerIdx != (sa_hostPolicing_info.logging_hostPolEntryIdx + G3_FLOW_POLICER_IDXSHIFT_HPLOGRX))
						WARNING("Rx logging index of hostPoliceList[%d] is %d, but it should be %d", sa_hostPolicing_info.logging_hostPolEntryIdx, rg_db.hostPoliceList[sa_hostPolicing_info.logging_hostPolEntryIdx].loggingRx_policerIdx, sa_hostPolicing_info.logging_hostPolEntryIdx + G3_FLOW_POLICER_IDXSHIFT_HPLOGRX);
				}

				if(sa_hostPolicing_info.egressRateLimit_en)
				{
					//SMAC hits host policing entry for egress rate limit, use policer id of rate limiting to get tx count
					if(rg_db.hostPoliceList[sa_hostPolicing_info.logging_hostPolEntryIdx].loggingTx_policerIdx != (sa_hostPolicing_info.egressRateLimit_mtrIdx + G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_TX))
						WARNING("Tx logging index of hostPoliceList[%d] is %d, but it should be %d", sa_hostPolicing_info.logging_hostPolEntryIdx, rg_db.hostPoliceList[sa_hostPolicing_info.logging_hostPolEntryIdx].loggingTx_policerIdx, (sa_hostPolicing_info.egressRateLimit_mtrIdx + G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_TX));
				}
				else
				{
					//SMAC does not hit host policing entry for egress rate limit, use policer id of pure tx counter to get tx count
					if(rg_db.hostPoliceList[sa_hostPolicing_info.logging_hostPolEntryIdx].loggingTx_policerIdx != (sa_hostPolicing_info.logging_hostPolEntryIdx + G3_FLOW_POLICER_IDXSHIFT_HPLOGTX))
						WARNING("Tx logging index of hostPoliceList[%d] is %d, but it should be %d", sa_hostPolicing_info.logging_hostPolEntryIdx, rg_db.hostPoliceList[sa_hostPolicing_info.logging_hostPolEntryIdx].loggingTx_policerIdx, (sa_hostPolicing_info.logging_hostPolEntryIdx + G3_FLOW_POLICER_IDXSHIFT_HPLOGTX));
				}
				/*Checking logging index stored in host policing entry if consistent or not END*/
			}

			if(!isFlowIdByHp)
			{
				flow_config.actions.options.masks.flow_id = sa_hostPolicing_info.logging_en;
				flow_config.actions.options.flow_id = sa_hostPolicing_info.logging_hostPolEntryIdx + G3_FLOW_POLICER_IDXSHIFT_HPLOGRX;
				isFlowIdByHp = 1;
				rg_db.flow[idx].hitHostPolingState = RTK_RG_G3_FLOW_HOSTPOLICING_HIT_SA;
			}
		}

		if(da_hostPolicing_info.logging_en)
		{
			if(da_hostPolicing_info.logging_hostPolEntryIdx== FAIL)
				WARNING("DMAC host policing mib is enabled, but no host policing entry index!");
			else
			{
				/*Checking logging index stored in host policing entry if consistent or not (just for dedugging)*/
				if(da_hostPolicing_info.ingressRateLimit_en)
				{
					//DMAC hits host policing entry for ingress rate limit, use policer id of rate limiting to get rx count
					if(rg_db.hostPoliceList[da_hostPolicing_info.logging_hostPolEntryIdx].loggingRx_policerIdx != (da_hostPolicing_info.ingressRateLimit_mtrIdx + G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_RX))
						WARNING("Rx logging index of hostPoliceList[%d] is %d, but it should be %d", da_hostPolicing_info.logging_hostPolEntryIdx, rg_db.hostPoliceList[da_hostPolicing_info.logging_hostPolEntryIdx].loggingRx_policerIdx, (da_hostPolicing_info.ingressRateLimit_mtrIdx + G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_RX));
				}
				else
				{
					//DMAC does not hit host policing entry for ingress rate limit, use policer id of pure rx counter to get rx count
					if(rg_db.hostPoliceList[da_hostPolicing_info.logging_hostPolEntryIdx].loggingRx_policerIdx != (da_hostPolicing_info.logging_hostPolEntryIdx + G3_FLOW_POLICER_IDXSHIFT_HPLOGRX))
						WARNING("Rx logging index of hostPoliceList[%d] is %d, but it should be %d", da_hostPolicing_info.logging_hostPolEntryIdx, rg_db.hostPoliceList[da_hostPolicing_info.logging_hostPolEntryIdx].loggingRx_policerIdx, da_hostPolicing_info.logging_hostPolEntryIdx + G3_FLOW_POLICER_IDXSHIFT_HPLOGRX);
				}

				if(da_hostPolicing_info.egressRateLimit_en)
				{
					//DMAC hits host policing entry for egress rate limit, use policer id of rate limiting to get tx count
					if(rg_db.hostPoliceList[da_hostPolicing_info.logging_hostPolEntryIdx].loggingTx_policerIdx != (da_hostPolicing_info.egressRateLimit_mtrIdx + G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_TX))
						WARNING("Tx logging index of hostPoliceList[%d] is %d, but it should be %d", da_hostPolicing_info.logging_hostPolEntryIdx, rg_db.hostPoliceList[da_hostPolicing_info.logging_hostPolEntryIdx].loggingTx_policerIdx, (da_hostPolicing_info.egressRateLimit_mtrIdx + G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_TX));
				}
				else
				{
					//DMAC does not hit host policing entry for egress rate limit, use policer id of pure tx counter to get tx count
					if(rg_db.hostPoliceList[da_hostPolicing_info.logging_hostPolEntryIdx].loggingTx_policerIdx != (da_hostPolicing_info.logging_hostPolEntryIdx + G3_FLOW_POLICER_IDXSHIFT_HPLOGTX))
						WARNING("Tx logging index of hostPoliceList[%d] is %d, but it should be %d", da_hostPolicing_info.logging_hostPolEntryIdx, rg_db.hostPoliceList[da_hostPolicing_info.logging_hostPolEntryIdx].loggingTx_policerIdx, (da_hostPolicing_info.logging_hostPolEntryIdx + G3_FLOW_POLICER_IDXSHIFT_HPLOGTX));
				}
				/*Checking logging index stored in host policing entry if consistent or not END*/
			}

			if(!isFlowIdByHp)
			{
				flow_config.actions.options.masks.flow_id = da_hostPolicing_info.logging_en;
				flow_config.actions.options.flow_id = da_hostPolicing_info.logging_hostPolEntryIdx + G3_FLOW_POLICER_IDXSHIFT_HPLOGTX;
				isFlowIdByHp = 1;
				rg_db.flow[idx].hitHostPolingState = RTK_RG_G3_FLOW_HOSTPOLICING_HIT_DA;
			}
		}
#endif

		// add ca hw flow
		ca_ret = ca_flow_add(G3_DEF_DEVID, &flow_config);

		if(ca_ret==CA_E_OK)
		{
			mainHash_hwFlowIdx = flow_config.index;
			TABLE(">>>>> Add Hw Main Hash flow[%d] of flow[%d]", flow_config.index, idx);
			ca_flow_age_set(G3_DEF_DEVID, flow_config.index, G3_FLOW_AGE_STATE_IDLE);
		}
		else if(ca_ret==CA_E_FULL)
		{
			mainHash_hwFlowIdx = FAIL;
			TABLE(">>>>> Hw Main Hash table is full");
		}
		else if(ca_ret==CA_E_EXISTS)
		{
			if((rg_db.mainHashValidSet[flow_config.index>>5] & (0x1<<(flow_config.index&0x1f)))
				&& (rg_db.swFlowIdx_mainHashMapping[flow_config.index].swFlowIdx < MAX_FLOW_SW_TABLE_SIZE))
			{
				if(rg_db.swFlowIdx_mainHashMapping[flow_config.index].swFlowIdx==idx)
				{
					mainHash_hwFlowIdx = flow_config.index;
					ca_flow_age_set(G3_DEF_DEVID, flow_config.index, G3_FLOW_AGE_STATE_IDLE);
				}
				else
				{
					mainHash_hwFlowIdx = FAIL;
					WARNING(">>>>> Hw Main Hash table is full and it exists a same crc32 hash entry");		
				}
				WARNING(">>>>> Hw Main Hash flow[%d] of flow[%d] exists !!!", flow_config.index, idx);
			}
			else
			{
				WARNING(">>>>> Hw Main Hash flow[%d] exists, but sw flow[%d] has no record !!!", flow_config.index, idx);
				return RT_ERR_RG_FAILED;
			}
			
		}
		else
		{
			mainHash_hwFlowIdx = FAIL;
			WARNING(">>>>> Add Hw Main Hash	flow type[%d] of flow[%d] fail, ca_ret = 0x%x", flow_config.key_type, idx, ca_ret);
		}
	}

	memcpy(&RG_PFLOW(idx)->path1, &pFlowPathEntry->path1, sizeof(rtk_rg_asic_path1_entry_t));
	rg_db.flow[idx].mainHash_hwFlowIdx = mainHash_hwFlowIdx;
	if(rg_db.flow[idx].mainHash_hwFlowIdx>=0)
	{
		rg_db.mainHashValidSet[rg_db.flow[idx].mainHash_hwFlowIdx>>5] |= (0x1<<(rg_db.flow[idx].mainHash_hwFlowIdx&0x1f));
		rg_db.swFlowIdx_mainHashMapping[rg_db.flow[idx].mainHash_hwFlowIdx].swFlowIdx = idx;
	}
	if(!isFlowIdByHp)
		rg_db.flow[idx].hitHostPolingState = RTK_RG_G3_FLOW_HOSTPOLICING_HIT_NONE; //no hit host policing entry
	return RT_ERR_RG_OK;
}

#else	//1 not CONFIG_RG_G3_SERIES
#if 0
rtk_rg_err_code_t RTK_RG_ASIC_FLOWPATH1_ADD(uint32 *idx, rtk_rg_asic_path1_entry_t *pP1Data, uint16 igrSVID, uint16 igrCVID)
{
	rtk_rg_err_code_t ret;

	ret = rtk_rg_asic_flowPath1_add(idx, pP1Data, igrSVID, igrCVID);
#if defined(CONFIG_RG_FLOW_4K_MODE)
	if(ret==RT_ERR_RG_OK)
	{
		memcpy(&RG_PFLOW(*idx)->path1, pP1Data, sizeof(rtk_rg_asic_path1_entry_t));
	}
#endif
	return ret;
}
#endif
rtk_rg_err_code_t RTK_RG_ASIC_FLOWPATH1_SET(uint32 idx, rtk_rg_asic_path1_entry_t *pP1Data)
{
	rtk_rg_err_code_t ret;

	if(idx<MAX_FLOW_HW_TABLE_SIZE)
	{
		ret = rtk_rg_asic_flowPath1_set(idx, pP1Data);
#if defined(CONFIG_RG_FLOW_4K_MODE)
		if(ret==RT_ERR_RG_OK)
		{
			memcpy(&RG_PFLOW(idx)->path1, pP1Data, sizeof(rtk_rg_asic_path1_entry_t));
		}
#endif
	}
	else
	{
		ret=RT_ERR_RG_OK;
		memcpy(&RG_PFLOW(idx)->path1, pP1Data, sizeof(rtk_rg_asic_path1_entry_t));
	}

	return ret;
}

#if 0
rtk_rg_err_code_t RTK_RG_ASIC_FLOWPATH2_ADD(uint32 *idx, rtk_rg_asic_path2_entry_t *pP2Data, uint16 igrSVID, uint16 igrCVID)
{
	rtk_rg_err_code_t ret;

	ret = rtk_rg_asic_flowPath2_add(idx, pP2Data, igrSVID, igrCVID);
#if defined(CONFIG_RG_FLOW_4K_MODE)
	if(ret==RT_ERR_RG_OK)
	{
		memcpy(&RG_PFLOW(*idx)->path2, pP2Data, sizeof(rtk_rg_asic_path2_entry_t));
	}
#endif
	return ret;
}
#endif

rtk_rg_err_code_t RTK_RG_ASIC_FLOWPATH2_SET(uint32 idx, rtk_rg_asic_path2_entry_t *pP2Data)
{
	rtk_rg_err_code_t ret;

	if(idx<MAX_FLOW_HW_TABLE_SIZE)
	{
		ret = rtk_rg_asic_flowPath2_set(idx, pP2Data);
#if defined(CONFIG_RG_FLOW_4K_MODE)
		if(ret==RT_ERR_RG_OK)
		{
			memcpy(&RG_PFLOW(idx)->path2, pP2Data, sizeof(rtk_rg_asic_path2_entry_t));
		}
#endif
	}
	else
	{
		ret=RT_ERR_RG_OK;
		memcpy(&RG_PFLOW(idx)->path2, pP2Data, sizeof(rtk_rg_asic_path2_entry_t));
	}

	return ret;
}

#if 0
rtk_rg_err_code_t RTK_RG_ASIC_FLOWPATH3DAHASH_ADD(uint32 *idx, rtk_rg_asic_path3_entry_t * pP3Data, uint16 igrSVID, uint16 igrCVID, uint16 lutDaIdx)
{
	rtk_rg_err_code_t ret;

	ret = rtk_rg_asic_flowPath3DAHash_add(idx, pP3Data, igrSVID, igrCVID, lutDaIdx);
#if defined(CONFIG_RG_FLOW_4K_MODE)
	if(ret==RT_ERR_RG_OK)
	{
		memcpy(&RG_PFLOW(*idx)->path3, pP3Data, sizeof(rtk_rg_asic_path3_entry_t));
	}
#endif
	return ret;
}

rtk_rg_err_code_t RTK_RG_ASIC_FLOWPATH3_ADD(uint32 *idx, rtk_rg_asic_path3_entry_t *pP3Data, uint16 igrSVID, uint16 igrCVID)
{
	rtk_rg_err_code_t ret;

	ret = rtk_rg_asic_flowPath3_add(idx, pP3Data, igrSVID, igrCVID);
#if defined(CONFIG_RG_FLOW_4K_MODE)
	if(ret==RT_ERR_RG_OK)
	{
		memcpy(&RG_PFLOW(*idx)->path3, pP3Data, sizeof(rtk_rg_asic_path3_entry_t));
	}
#endif
	return ret;
}
#endif


rtk_rg_err_code_t RTK_RG_ASIC_FLOWPATH3_SET(uint32 idx, rtk_rg_asic_path3_entry_t *pP3Data)
{
	rtk_rg_err_code_t ret;

	if(idx<MAX_FLOW_HW_TABLE_SIZE)
	{
		ret = rtk_rg_asic_flowPath3_set(idx, pP3Data);
#if defined(CONFIG_RG_FLOW_4K_MODE)
		if(ret==RT_ERR_RG_OK)
		{
			memcpy(&RG_PFLOW(idx)->path3, pP3Data, sizeof(rtk_rg_asic_path3_entry_t));
		}
#endif
	}
	else
	{
		ret=RT_ERR_RG_OK;
		memcpy(&RG_PFLOW(idx)->path3, pP3Data, sizeof(rtk_rg_asic_path3_entry_t));
	}

	return ret;
}

#if 0
rtk_rg_err_code_t RTK_RG_ASIC_FLOWPATH4DAHASH_ADD(uint32 *idx, rtk_rg_asic_path4_entry_t *pP4Data, uint16 igrSVID, uint16 igrCVID, uint16 lutDaIdx)
{
	rtk_rg_err_code_t ret;

	ret = rtk_rg_asic_flowPath4DAHash_add(idx, pP4Data, igrSVID, igrCVID, lutDaIdx);
#if defined(CONFIG_RG_FLOW_4K_MODE)
	if(ret==RT_ERR_RG_OK)
	{
		memcpy(&RG_PFLOW(*idx)->path4, pP4Data, sizeof(rtk_rg_asic_path4_entry_t));
	}
#endif
	return ret;
}

rtk_rg_err_code_t RTK_RG_ASIC_FLOWPATH4_ADD(uint32 *idx, rtk_rg_asic_path4_entry_t *pP4Data, uint16 igrSVID, uint16 igrCVID)
{
	rtk_rg_err_code_t ret;

	ret = rtk_rg_asic_flowPath4_add(idx, pP4Data, igrSVID, igrCVID);
#if defined(CONFIG_RG_FLOW_4K_MODE)
	if(ret==RT_ERR_RG_OK)
	{
		memcpy(&RG_PFLOW(*idx)->path4, pP4Data, sizeof(rtk_rg_asic_path4_entry_t));
	}
#endif
	return ret;
}
#endif

rtk_rg_err_code_t RTK_RG_ASIC_FLOWPATH4_SET(uint32 idx, rtk_rg_asic_path4_entry_t *pP4Data)
{
	rtk_rg_err_code_t ret;

	if(idx<MAX_FLOW_HW_TABLE_SIZE)
	{
		ret = rtk_rg_asic_flowPath4_set(idx, pP4Data);
#if defined(CONFIG_RG_FLOW_4K_MODE)
		if(ret==RT_ERR_RG_OK)
		{
			memcpy(&RG_PFLOW(idx)->path4, pP4Data, sizeof(rtk_rg_asic_path4_entry_t));
		}
#endif
	}
	else
	{
		ret=RT_ERR_RG_OK;
		memcpy(&RG_PFLOW(idx)->path4, pP4Data, sizeof(rtk_rg_asic_path4_entry_t));
	}

	return ret;
}

#if 0
rtk_rg_err_code_t RTK_RG_ASIC_FLOWPATH5_ADD(uint32 *idx, rtk_rg_asic_path5_entry_t *pP5Data, uint16 igrSVID, uint16 igrCVID)
{
	rtk_rg_err_code_t ret;

	ret = rtk_rg_asic_flowPath5_add(idx, pP5Data, igrSVID, igrCVID);
#if defined(CONFIG_RG_FLOW_4K_MODE)
	if(ret==RT_ERR_RG_OK)
	{
		memcpy(&RG_PFLOW(*idx)->path5, pP5Data, sizeof(rtk_rg_asic_path5_entry_t));
	}
#endif
	return ret;
}
#endif

rtk_rg_err_code_t RTK_RG_ASIC_FLOWPATH5_SET(uint32 idx, rtk_rg_asic_path5_entry_t *pP5Data)
{
	rtk_rg_err_code_t ret;

	if(idx<MAX_FLOW_HW_TABLE_SIZE)
	{
		ret = rtk_rg_asic_flowPath5_set(idx, pP5Data);
#if defined(CONFIG_RG_FLOW_4K_MODE)
		if(ret==RT_ERR_RG_OK)
		{
			memcpy(&RG_PFLOW(idx)->path5, pP5Data, sizeof(rtk_rg_asic_path5_entry_t));
		}
#endif
	}
	else
	{
		ret=RT_ERR_RG_OK;
		memcpy(&RG_PFLOW(idx)->path5, pP5Data, sizeof(rtk_rg_asic_path5_entry_t));
	}

	return ret;
}

#if 0
rtk_rg_err_code_t RTK_RG_ASIC_FLOWPATH6_ADD(uint32 *idx, rtk_rg_asic_path6_entry_t *pP6Data, uint16 igrSVID, uint16 igrCVID)
{
	rtk_rg_err_code_t ret;

	ret = rtk_rg_asic_flowPath6_add(idx, pP6Data, igrSVID, igrCVID);
#if defined(CONFIG_RG_FLOW_4K_MODE)
	if(ret==RT_ERR_RG_OK)
	{
		memcpy(&RG_PFLOW(*idx)->path6, pP6Data, sizeof(rtk_rg_asic_path6_entry_t));
	}
#endif
	return ret;
}
#endif

rtk_rg_err_code_t RTK_RG_ASIC_FLOWPATH6_SET(uint32 idx, rtk_rg_asic_path6_entry_t *pP6Data)
{
	rtk_rg_err_code_t ret;

	if(idx<MAX_FLOW_HW_TABLE_SIZE)
	{
		ret = rtk_rg_asic_flowPath6_set(idx, pP6Data);
#if defined(CONFIG_RG_FLOW_4K_MODE)
		if(ret==RT_ERR_RG_OK)
		{
			memcpy(&RG_PFLOW(idx)->path6, pP6Data, sizeof(rtk_rg_asic_path6_entry_t));
		}
#endif
	}
	else
	{
		ret=RT_ERR_RG_OK;
		memcpy(&RG_PFLOW(idx)->path6, pP6Data, sizeof(rtk_rg_asic_path6_entry_t));
	}

	return ret;
}
#endif


rtk_rg_err_code_t _rtk_rg_flowTracer(uint32 idx)
{
	if((rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_TRACE_DUMP) && (rg_kernel.filter_level&RTK_RG_DEBUG_LEVEL_TRACE_DUMP))
	{
		int i=0,hit=0;
		int oriShow=rg_kernel.tracefilterShow;
		for(i=0 ; i< TRACFILTER_MAX ;i++)
		{
			if(rg_kernel.trace_filter[i].flowIdxCh== idx)
				hit=1;
		}
		if(hit)
		{
			rg_kernel.tracefilterShow=1;//forceshow
			WARNING("flow change Idx=%d",idx);
			WARNING("===============================================================");
			_rtk_rg_dump_stack();
			WARNING("===============================================================");
			rg_kernel.tracefilterShow=oriShow;
		}
	}
	return RT_ERR_RG_OK;

}


rtk_rg_err_code_t RTK_RG_ASIC_FLOWPATH_DEL(uint32 idx)
{
	rtk_rg_err_code_t ret=RT_ERR_RG_OK;

	if(idx>=MAX_FLOW_SW_TABLE_SIZE)
		RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);

#if defined(CONFIG_RG_G3_SERIES)
{

	if(rg_db.flow[idx].isMulticast)
	{
		int i,swFlowIdx=FAIL;
		rtk_rg_flow_extraInfo_t flowExtraInfo;
		bzero(&flowExtraInfo,sizeof(flowExtraInfo));
		for(i=0 ; i < DEFAULT_MAX_FLOW_COUNT; i++)
		{
			if(rg_db.mcflowIdxtbl[i].vaild ==0)
				continue;
			if(rg_db.mcflowIdxtbl[i].path3Idx ==idx)
			{
				swFlowIdx=i;
				break;
			}
			if(rg_db.mcflowIdxtbl[i].path4Idx ==idx)
			{
				swFlowIdx=i;
				break;
			}
		}
		if(swFlowIdx!=FAIL)
		{
			memcpy(flowExtraInfo.v6Dip.ipv6_addr,rg_db.mcflowIdxtbl[swFlowIdx].multicastAddress,sizeof(flowExtraInfo.v6Dip.ipv6_addr));
		}
		IGMP("MC remove flow %d  pmsk=%x ",idx,RG_PFLOW(idx)->path3.out_portmask);
		RG_PFLOW(idx)->path3.out_portmask=0;
		_RTK_RG_ASIC_G3_MC_FLOW_SET(RG_PFLOW(idx),&flowExtraInfo);

	}


	if(rg_db.flow[idx].mainHash_hwFlowIdx>=0)
	{
		ca_status_t ret;
		ret = ca_flow_delete(G3_DEF_DEVID, rg_db.flow[idx].mainHash_hwFlowIdx);
		if(ret == CA_E_OK)
		{
			TABLE(">>>>> Delete Hw Main Hash flow[%d] of flow[%d]", rg_db.flow[idx].mainHash_hwFlowIdx, idx);
			ret=RT_ERR_RG_OK;
			memset(&RG_PFLOW(idx)->path1, 0, sizeof(rtk_rg_asic_path1_entry_t));
			_rtk_rg_init_mainHash_swData_by_swFlowIdx(idx);

		}
		else
		{
			WARNING(">>>>> Delete Hw Main Hash flow[%d] of flow[%d] fail, ca_ret = 0x%x", rg_db.flow[idx].mainHash_hwFlowIdx, idx, ret);
			ret=RT_ERR_RG_FAILED;
		}
	}
	else
	{
		ret=RT_ERR_RG_OK;
		memset(&RG_PFLOW(idx)->path1, 0, sizeof(rtk_rg_asic_path1_entry_t));
		rg_db.flow[idx].mainHash_hwFlowIdx = FAIL;
	}
}
#else	// not CONFIG_RG_G3_SERIES
{
	if(idx<MAX_FLOW_HW_TABLE_SIZE)
	{
#if defined(CONFIG_RG_FLOW_4K_MODE)
		ret=rtk_rg_asic_flowPath_del(idx);
		// delete SW entry
		if(ret==RT_ERR_RG_OK)
		{
			memset(&RG_PFLOW(idx)->path1, 0, sizeof(rtk_rg_asic_path1_entry_t));
		}
#else	// not CONFIG_RG_FLOW_4K_MODE
		// delete DRAM entry - the entry was pointed by RG_PFLOW(idx)
		ret=rtk_rg_asic_flowPath_del(idx);
#endif
	}
	else
	{
		ret=RT_ERR_RG_OK;
		RG_PFLOW(idx)->path1.valid = 0;
	}
}
#endif

	_rtk_rg_flowTracer(idx);

	return ret;
}

rtk_rg_err_code_t RTK_RG_ASIC_FLOWPATH_SET(uint32 idx, rtk_rg_table_flowEntry_t *pFlowPathEntry, rtk_rg_flow_extraInfo_t flowExtraInfo /*only for G3 platform*/)
{
	rtk_rg_err_code_t ret=RT_ERR_RG_OK;
#if defined(CONFIG_RG_G3_SERIES)
	ret = RTK_RG_ASIC_G3_FLOW_SET(idx, pFlowPathEntry, flowExtraInfo);
#else	// not CONFIG_RG_G3_SERIES
	if(pFlowPathEntry->path1.in_path==FB_PATH_12)
	{
		if(pFlowPathEntry->path1.in_multiple_act==0)		//path 1
		{
			ret = RTK_RG_ASIC_FLOWPATH1_SET(idx, &pFlowPathEntry->path1);
		}
		else												//path 2
		{
			ret = RTK_RG_ASIC_FLOWPATH2_SET(idx, &pFlowPathEntry->path2);
		}
	}
	else if(pFlowPathEntry->path1.in_path==FB_PATH_34)
	{
		if(pFlowPathEntry->path1.in_multiple_act==0)		//path 3
		{
			ret = RTK_RG_ASIC_FLOWPATH3_SET(idx, &pFlowPathEntry->path3);
		}
		else												//path 4
		{
			ret = RTK_RG_ASIC_FLOWPATH4_SET(idx, &pFlowPathEntry->path4);
		}
	}
	else if(pFlowPathEntry->path1.in_path==FB_PATH_5)		//path 5
	{
		ret = RTK_RG_ASIC_FLOWPATH5_SET(idx, &pFlowPathEntry->path5);
	}
	else if(pFlowPathEntry->path1.in_path==FB_PATH_6)		//path 6
	{
		ret = RTK_RG_ASIC_FLOWPATH6_SET(idx, &pFlowPathEntry->path6);
	}
#endif
	_rtk_rg_flowTracer(idx);

	return ret;
}


rtk_rg_err_code_t RTK_RG_ASIC_EXTPORTMASKTABLE_ADD(uint32 extPMask, int32 *extPMaskIdx)
{
	rtk_rg_err_code_t ret = RT_ERR_RG_OK;
	int32 idx;

	// find an available entry in indirect access table
	if((idx = _rtk_rg_extPMaskEntry_find(extPMask)) == RG_RET_ENTRY_NOT_GET)
	{
		WARNING("FAIL to add indirect mac table because of entry full");
		return RT_ERR_RG_ENTRY_FULL;
	}

	if(rg_db.extPortTbl[idx].extPortEnt.extpmask == extPMask)
	{
		// increase referenced entry
		rg_db.extPortTbl[idx].extPortRefCount++;
		TABLE("Add extPMask entry[%d]=0x%x extPortRefCount:%d", idx, extPMask,rg_db.extPortTbl[idx].extPortRefCount);
	}else
	{
		TABLE("Add extPMask entry[%d]=0x%x", idx, extPMask);
#if defined(CONFIG_RG_G3_SERIES)
		ret = RT_ERR_RG_OK;
#else
		{
			// configure a new one
			rtk_rg_asic_extPortMask_entry_t extPMaskEnt;
			extPMaskEnt.extpmask = extPMask;
			ret=rtk_rg_asic_extPortMaskTable_add(idx, &extPMaskEnt);
		}
#endif
		if(ret==RT_ERR_RG_OK)
		{
			//Copy to software table
			rg_db.extPortTbl[idx].extPortEnt.extpmask = extPMask;
			rg_db.extPortTbl[idx].extPortRefCount = 1;
		}
	}

	*extPMaskIdx = idx;

	//DEBUG("target extpmaskidx = %d, refcount = %d, mask = 0x%x", idx, rg_db.extPortTbl[idx].extPortRefCount, rg_db.extPortTbl[idx].extPortEnt.extpmask);

	return ret;
}

rtk_rg_err_code_t RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(int32 extPMaskIdx)
{
	rtk_rg_err_code_t ret = RT_ERR_RG_OK;

	if(extPMaskIdx >= EXTPTBL_RSVED_ENTRY )
	{
		if(rg_db.extPortTbl[extPMaskIdx].extPortRefCount > 0)
		{
			rg_db.extPortTbl[extPMaskIdx].extPortRefCount -= 1;
			TABLE("Del(keep by refCount) extPMask entry[%d]=0x%x ref=%d", extPMaskIdx, rg_db.extPortTbl[extPMaskIdx].extPortEnt.extpmask,rg_db.extPortTbl[extPMaskIdx].extPortRefCount);

		}
		if(rg_db.extPortTbl[extPMaskIdx].extPortRefCount == 0)
		{
			TABLE("Del extPMask entry[%d]=0x%x", extPMaskIdx, rg_db.extPortTbl[extPMaskIdx].extPortEnt.extpmask);
#if defined(CONFIG_RG_G3_SERIES)
			ret = RT_ERR_RG_OK;
#else
			ret = rtk_rg_asic_extPortMaskTable_del(extPMaskIdx);
#endif
			if(ret==RT_ERR_RG_OK)
			{
				rg_db.extPortTbl[extPMaskIdx].extPortEnt.extpmask = 0;
			}
		}

		//DEBUG("target extpmaskidx = %d, refcount = %d, mask = 0x%x", extPMaskIdx, rg_db.extPortTbl[extPMaskIdx].extPortRefCount, rg_db.extPortTbl[extPMaskIdx].extPortEnt.extpmask);
	}else
	{
		// entry[0] was reserved, do nothing
	}

	return ret;

}

rtk_rg_err_code_t RTK_RG_ASIC_INDIRECTMACTABLE_ADD(int32 l2Idx, int32 *indMacIdx)
{
	rtk_rg_err_code_t ret = RT_ERR_RG_OK;
	int32 idx;

	if(l2Idx<0)
	{
		WARNING("FAIL to add indirect mac table because of l2Idx invalid");
		return RT_ERR_RG_INVALID_PARAM;
	}

	// find an available entry in indirect access table
	if((idx = _rtk_rg_indirectMacEntry_find(l2Idx)) == RG_RET_ENTRY_NOT_GET)
	{
		WARNING("FAIL to add indirect mac table because of entry full");
		return RT_ERR_RG_ENTRY_FULL;
	}

	if(rg_db.indMacTbl[idx].indMacEnt.l2_idx == l2Idx)
	{
		// increase referenced entry
		rg_db.indMacTbl[idx].indMacRefCount++;
	}else
	{
		TABLE("Add indirect MAC entry[%d]=%d", idx, l2Idx);
#if defined(CONFIG_RG_G3_SERIES)
		ret = RT_ERR_RG_OK;
#else
		{
			// configure a new one
			rtk_rg_asic_indirectMac_entry_t indMacEnt;
			indMacEnt.l2_idx = l2Idx;
			ret = rtk_rg_asic_indirectMacTable_add(idx, &indMacEnt);
		}
#endif
		if(ret==RT_ERR_RG_OK)
		{
			rg_db.indMacTbl[idx].indMacEnt.l2_idx = l2Idx;
			rg_db.indMacTbl[idx].indMacRefCount = 1;
		}
	}

	*indMacIdx = idx;

	return ret;
}

rtk_rg_err_code_t RTK_RG_ASIC_INDIRECTMACTABLE_DEL(int32 indMacIdx)
{
	rtk_rg_err_code_t ret = RT_ERR_RG_OK;

	rg_db.indMacTbl[indMacIdx].indMacRefCount -= 1;

	if(rg_db.indMacTbl[indMacIdx].indMacRefCount == 0)
	{
		TABLE("Del indirect MAC entry[%d]=%d", indMacIdx, rg_db.indMacTbl[indMacIdx].indMacEnt.l2_idx);
#if defined(CONFIG_RG_G3_SERIES)
		ret = RT_ERR_RG_OK;
#else
		ret = rtk_rg_asic_indirectMacTable_del(indMacIdx);
#endif
		if(ret==RT_ERR_RG_OK)
		{
			rg_db.indMacTbl[indMacIdx].indMacEnt.l2_idx = 0;
		}
	}

	return ret;
}

rtk_rg_err_code_t RTK_RG_ASIC_WANACCESSLIMITTABLE_ADD(int32 idx, rtk_rg_asic_wanAccessLimit_entry_t *pWALimitEntry)
{
	rtk_rg_err_code_t ret = RT_ERR_RG_OK;

	TABLE("Add wan access limit entry[%d]=%d", idx, pWALimitEntry->sa_idx);
#if defined(CONFIG_RG_RTL9607C_SERIES)
	ret = rtk_rg_asic_wanAccessLimitTable_add(idx, pWALimitEntry);
#else
	ret = RT_ERR_RG_OK;
#endif
	if(ret==RT_ERR_RG_OK)
	{
		memcpy(&rg_db.wanAccessLimit[idx].wanAllowEnt, pWALimitEntry, sizeof(rtk_rg_asic_wanAccessLimit_entry_t));
	}
	return ret;
}

rtk_rg_err_code_t RTK_RG_ASIC_WANACCESSLIMITTABLE_DEL(int32 idx)
{
	rtk_rg_err_code_t ret = RT_ERR_RG_OK;

	TABLE("Del wan access limit entry[%d]=%d", idx, rg_db.wanAccessLimit[idx].wanAllowEnt.sa_idx);
#if defined(CONFIG_RG_RTL9607C_SERIES)
	ret = rtk_rg_asic_wanAccessLimitTable_del(idx);
#else
	ret = RT_ERR_RG_OK;
#endif
	if(ret==RT_ERR_RG_OK)
	{
		rg_db.wanAccessLimit[idx].wanAllowEnt.valid = FALSE;
		rg_db.wanAccessLimit[idx].wanAllowEnt.sa_idx = 0;
	}
	return ret;
}

rtk_rg_err_code_t RTK_RG_ASIC_WANACCESSLIMITPORTMASK_SET(uint32 portMask)
{
	rtk_rg_err_code_t ret = RT_ERR_RG_OK;

#if defined(CONFIG_RG_RTL9607C_SERIES)
	ret = rtk_rg_asic_wanAccessLimitPortMask_set(portMask);
#else
	ret = RT_ERR_RG_OK;
#endif
	if(ret==RT_ERR_RG_OK)
	{
		rg_db.systemGlobal.wanAccessLimit_pmask.portmask = portMask;
	}
	return ret;
}

rtk_rg_err_code_t RTK_RG_ASIC_FLOWMIB_GET(uint32 idx, rtk_rg_table_flowmib_t *pFlowMib)
{
	// return flow mib counters recorded by sorfware and hardware
	rtk_rg_err_code_t ret = RT_ERR_RG_OK;
	rtk_rg_asic_flowMib_entry_t hwFlowMib;

	bzero(&hwFlowMib, sizeof(hwFlowMib));
	if(idx < MAX_FLOWMIB_TABLE_SIZE)
	{//hw mib
#if defined(CONFIG_RG_G3_SERIES)
		{
			aal_l3_te_pm_policer_t l3_pm_data;
			if(rg_db.systemGlobal.flow_meter_mib_conf_dependence)
				ASSERT_EQ(aal_l3_te_pm_policer_flow_get(G3_DEF_DEVID, (idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMTR), &l3_pm_data), CA_E_OK);
			else
				ASSERT_EQ(aal_l3_te_pm_policer_flow_get(G3_DEF_DEVID, (idx + G3_FLOW_POLICER_IDXSHIFT_FLOWMIB), &l3_pm_data), CA_E_OK);

			rg_db.flowMIBTbl[idx].in_packet_cnt += l3_pm_data.total_pkt; //read clear, store into SW
			rg_db.flowMIBTbl[idx].out_packet_cnt += l3_pm_data.total_pkt - l3_pm_data.red_pkt; //read clear, store into SW
			rg_db.flowMIBTbl[idx].in_byte_cnt += l3_pm_data.total_bytes;
			rg_db.flowMIBTbl[idx].out_byte_cnt += l3_pm_data.total_bytes - l3_pm_data.red_bytes;
		}
#else
		ret = rtk_rg_asic_flowMib_get(idx, &hwFlowMib);
#endif
	}
	if(ret==RT_ERR_RG_OK)
	{
		pFlowMib->in_byte_cnt = rg_db.flowMIBTbl[idx].in_byte_cnt + hwFlowMib.in_byte_cnt;
		pFlowMib->in_packet_cnt = rg_db.flowMIBTbl[idx].in_packet_cnt + hwFlowMib.in_packet_cnt;
		pFlowMib->out_byte_cnt = rg_db.flowMIBTbl[idx].out_byte_cnt + hwFlowMib.out_byte_cnt;
		pFlowMib->out_packet_cnt = rg_db.flowMIBTbl[idx].out_packet_cnt + hwFlowMib.out_packet_cnt;
	}
	return ret;
}

rtk_rg_err_code_t RTK_RG_ASIC_FLOWMIB_RESET(uint32 idx)
{
	// return flow mib counters recorded by sorfware and hardware
	rtk_rg_err_code_t ret = RT_ERR_RG_OK;
	if(idx < MAX_FLOWMIB_TABLE_SIZE)
	{	//hw mib
#if defined(CONFIG_RG_G3_SERIES)
		{
			rtk_rg_table_flowmib_t pFlowMib;
			ret = RTK_RG_ASIC_FLOWMIB_GET(idx, &pFlowMib);//read clear
		}
#else
		ret = rtk_rg_asic_flowMib_reset(idx);
#endif
	}
	if(ret==RT_ERR_RG_OK)
	{
		rg_db.flowMIBTbl[idx].in_byte_cnt = 0;
		rg_db.flowMIBTbl[idx].in_packet_cnt = 0;
		rg_db.flowMIBTbl[idx].out_byte_cnt = 0;
		rg_db.flowMIBTbl[idx].out_packet_cnt = 0;
	}
	return ret;
}

#endif

int32 RTK_INIT_WITHOUT_PON(void)
{
	int ret=RT_ERR_OK;
	ret = rtk_init_without_pon();
	return ret;
}

int32 RTK_L2_INIT(void)
{
	int ret=RT_ERR_OK;
	ret = rtk_l2_init();
	return ret;
}

/* don't care collision ,just add and update entry*/
int32 RTK_L2_ADDR_ADD(rtk_l2_ucastAddr_t *pL2Addr)
{
	int ret, l2Idx, svlL2Idx=FAIL;

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	if(pL2Addr->vid >= MAX_VLAN_HW_TABLE_SIZE)
	{
		if(pL2Addr->flags&RTK_L2_UCAST_FLAG_IVL)	//IVL
		{
			WARNING("[Fail to add] IVL lut entry can not use vid[%d] that its value >= %d", pL2Addr->vid, MAX_VLAN_HW_TABLE_SIZE);
			return RT_ERR_FAILED;
		}
		else	//SVL
		{
			TRACE("Change internal-used vid[%d] to vid[0]", pL2Addr->vid);
			pL2Addr->vid = 0;
		}
	}
#endif

#if defined(CONFIG_RG_RTL9600_SERIES)
	//20130801: for avoiding L2 age out issue. (pure L2 entries are always enabling ARP_USED.)
	pL2Addr->flags|=RTK_L2_UCAST_FLAG_ARP_USED;
#endif
	//set lut traffic bit to 1(age=7), so idle time will not be add in first round.
	pL2Addr->age=7;

	//find corresponding SVL lut entry
	if(pL2Addr->flags&RTK_L2_UCAST_FLAG_IVL)	//IVL
	{
		svlL2Idx = _rtk_rg_svlMacLookup(pL2Addr->mac);
		if(svlL2Idx==FAIL)
		{
			WARNING("[Fail to add] there is no corresponding SVL lut entry.");
			return RT_ERR_FAILED;
		}
	}

#if defined(CONFIG_RG_G3_SERIES)
	if(rg_db.systemGlobal.doNotAddHwLut==0)
	{
		ret=rtk_l2_addr_add(pL2Addr);
	}
	else
	{
		if(_rtk_rg_lutIdx_find(pL2Addr->mac.octet, ((pL2Addr->flags&RTK_L2_UCAST_FLAG_IVL) ? pL2Addr->vid : rg_db.systemGlobal.initParam.fwdVLAN_CPU), &pL2Addr->index) != RG_RET_LUTIDX_NOFREE)
			ret=RT_ERR_OK;
		else
			ret=RT_ERR_ENTRY_FULL;
	}
	// G3 platform: -2 stands for lan interface, -3 stands for wan interface
	// Recover port translation for sw data path
	if((pL2Addr->port==AAL_LPORT_L3_LAN || pL2Addr->port==AAL_LPORT_L3_WAN) && pL2Addr->ext_port==0)
	{
		pL2Addr->port = RTK_RG_MAC_PORT_MAINCPU;
		pL2Addr->ext_port = 0;
	}
#else
{
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	uint32 ori_port=pL2Addr->port, ori_extPort=pL2Addr->ext_port;
	if(rg_db.systemGlobal.phy2ExtPort_mappingMsk & (0x1<<ori_port))
	{
		TABLE("Match phy2ExtPort_mappingMsk, change portIdx from phyPort %d to extPort %d", ori_port, ori_port);
		pL2Addr->port = RTK_RG_MAC_PORT_MAINCPU;
		pL2Addr->ext_port = (ori_port-RTK_RG_MAC_PORT0) + 1;
	}
#endif
	ret=rtk_l2_addr_add(pL2Addr);
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
	pL2Addr->port = ori_port;
	pL2Addr->ext_port = ori_extPort;
#endif
}
#endif

	if(ret==RT_ERR_OK)
	{
#if defined(CONFIG_RG_G3_SERIES)
		if(rg_db.systemGlobal.doNotAddHwLut==0)
		{
			if(_rtk_rg_hw_lut_idx_get(pL2Addr->mac.octet, (pL2Addr->flags&RTK_L2_UCAST_FLAG_IVL)?TRUE:FALSE, (pL2Addr->flags&RTK_L2_UCAST_FLAG_IVL)?pL2Addr->vid:pL2Addr->fid, &pL2Addr->index)!=RT_ERR_OK)
				return RT_ERR_ENTRY_NOTFOUND;
		}
#endif
		TABLE("add l2 %d", pL2Addr->index);

		rg_db.lut[pL2Addr->index].rtk_lut.entryType=RTK_LUT_L2UC;
		memcpy(&rg_db.lut[pL2Addr->index].rtk_lut.entry.l2UcEntry,pL2Addr,sizeof(rtk_l2_ucastAddr_t));
		rg_db.lut[pL2Addr->index].valid=1;

		//SVL: add to lut group of itself,  IVL: add to lut group of corresponding SVL group
		if(pL2Addr->flags&RTK_L2_UCAST_FLAG_IVL)	//IVL
		{
			_rtk_rg_lutGroupListAdd(svlL2Idx, pL2Addr->index);
		}
		else	//SVL
		{
			_rtk_rg_lutGroupListAdd(pL2Addr->index, pL2Addr->index);
		}

		if(pL2Addr->index<MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE)		//4-way
		{
			//Keep the next of newest
			l2Idx=(pL2Addr->index>>MAX_LUT_HASH_WAY_SHIFT)<<MAX_LUT_HASH_WAY_SHIFT;
			rg_db.layer2NextOfNewestCountIdx[l2Idx>>MAX_LUT_HASH_WAY_SHIFT]=(pL2Addr->index-l2Idx+1)%MAX_LUT_HASH_WAY_SIZE;
		}
#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support lut traffic bit
		if((pL2Addr->index>=MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE) && (pL2Addr->index<MAX_LUT_HW_TABLE_SIZE))
		{
			if(pL2Addr->flags&RTK_L2_UCAST_FLAG_IVL)
				_rtk_rg_lutCamListAdd(_rtk_rg_hash_mac_vid_efid(pL2Addr->mac.octet,pL2Addr->vid,0), pL2Addr->index);
			else
				_rtk_rg_lutCamListAdd(_rtk_rg_hash_mac_fid_efid(pL2Addr->mac.octet,pL2Addr->fid,0), pL2Addr->index);
		}
#endif
	}

	return ret;

}

int32 RTK_L2_ADDR_DEL(rtk_l2_ucastAddr_t *pL2Addr)
{
	rtk_rg_port_idx_t portIdx;
	int ret;

#if defined(CONFIG_RG_G3_SERIES)
	if(rg_db.systemGlobal.doNotAddHwLut==0)
	{
		ret=rtk_l2_addr_del(pL2Addr);
	}
	else
	{
		if((pL2Addr->index = _rtk_rg_macLookup(pL2Addr->mac.octet, ((pL2Addr->flags&RTK_L2_UCAST_FLAG_IVL) ? pL2Addr->vid : rg_db.systemGlobal.initParam.fwdVLAN_CPU), FALSE)) != RG_RET_LOOKUPIDX_NOT_FOUND)
			ret=RT_ERR_OK;
		else
			ret=RT_ERR_ENTRY_NOTFOUND;
	}
#else
	ret=rtk_l2_addr_del(pL2Addr);
#endif
	if(ret==RT_ERR_OK)
	{
		TABLE("del l2 %d", pL2Addr->index);

		//init lut group index, it must be called before using _rtk_rg_layer2CleanL34ReferenceTable().
		_rtk_rg_lutGroupListDelAndInit(pL2Addr->index);
		
		//delete L34 related entries which refer to this l2 index
		_rtk_rg_layer2CleanL34ReferenceTable(pL2Addr->index);
		
		//2 20140402LUKE:static MAC should not add to learning count
		if((pL2Addr->flags&RTK_L2_UCAST_FLAG_STATIC)==0)
		{
			//------------------ Critical Section start -----------------------//
			//rg_lock(&rg_kernel.saLearningLimitLock);
			_rtk_rg_macPortToPort_translator(&portIdx, rg_db.lut[pL2Addr->index].rtk_lut.entry.l2UcEntry.port, rg_db.lut[pL2Addr->index].rtk_lut.entry.l2UcEntry.ext_port);
#if defined(CONFIG_MASTER_WLAN0_ENABLE) && defined(CONFIG_RG_FLOW_NEW_WIFI_MODE)
			_rtk_rg_lutExtport_translator(&portIdx);
#endif
			if(rg_db.lut[pL2Addr->index].countingInLearningLimit)
				assert_ok(_rtk_rg_mac_learning_limit_count_dec(portIdx, rg_db.lut[pL2Addr->index].wlan_device_idx));

			if(!list_empty(&rg_db.systemGlobal.vlanGroupMACLimit_macHead[pL2Addr->index>>MAX_LUT_HASH_WAY_SHIFT]))
			{
				rtk_rg_vlanGroupMacLimit_mac_t *pMac,*pNextMac;
				list_for_each_entry_safe(pMac, pNextMac, &rg_db.systemGlobal.vlanGroupMACLimit_macHead[pL2Addr->index>>MAX_LUT_HASH_WAY_SHIFT], mac_list)
				{
					if(!memcmp(pMac->mac.octet,pL2Addr->mac.octet,ETHER_ADDR_LEN))
					{
						if(pMac->pGroup)
						{
							atomic_dec(&pMac->pGroup->group_info.mac_count);
							list_del(&pMac->group_list);
						}
						list_del(&pMac->mac_list);
						rtk_rg_free(pMac);
					}
				}
			}
			
			//20170222: wan access limit only counts SVL lut entry
			if((pL2Addr->flags&RTK_L2_UCAST_FLAG_IVL)==0)
			{
				//20170301LUKE: modify count when limit field is source mac.
				if(rg_db.systemGlobal.activeLimitField==RG_ACCESSWAN_LIMIT_BY_SMAC)
				{
					if(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask&(0x1<<portIdx) && rg_db.lut[pL2Addr->index].permit_for_l34_forward)
						atomic_dec(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
					if(_rtK_rg_checkCategoryPortmask_spa(portIdx)==SUCCESS)
						atomic_dec(&rg_db.systemGlobal.accessWanLimitCategoryCount[(unsigned int)rg_db.lut[pL2Addr->index].category]);
#ifdef CONFIG_MASTER_WLAN0_ENABLE
					if(portIdx > RTK_RG_PORT_LASTCPU)
					{
						//decrease wlan's device count
						if(((RTK_RG_ALL_MASTER_EXT_PORTMASK&(0x1<<portIdx))
#if defined(CONFIG_DUALBAND_CONCURRENT) || defined(CONFIG_RG_FLOW_ENHANCED_WIFI_MODE)
							|| (RTK_RG_ALL_SLAVE_EXT_PORTMASK&(0x1<<portIdx))
#endif
							) && rg_db.lut[pL2Addr->index].wlan_device_idx>=0)
						{
							//20170301LUKE: modify count when limit field is source mac.
								if(rg_db.systemGlobal.accessWanLimitPortMask_wlan0member&(0x1<<(rg_db.lut[pL2Addr->index].wlan_device_idx)))
									atomic_dec(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
						}
					}
#endif	
				}
			}
			//------------------ Critical Section End -----------------------//
			//rg_unlock(&rg_kernel.saLearningLimitLock);
		}

#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support lut traffic bit
		if((pL2Addr->index>=MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE) && (pL2Addr->index<MAX_LUT_HW_TABLE_SIZE))
			_rtk_rg_lutCamListDel(pL2Addr->index);
		else
#endif
			memset(&rg_db.lut[pL2Addr->index],0,sizeof(rtk_rg_table_lut_t));

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	{
		// sync to del the lut entry in wan access limit
		_rtk_rg_l34WanAccessLimit_delL2Idx(pL2Addr->index);
	}
#endif

		
	}
	return ret;
}

int32 RTK_L2_ADDR_DELALL(uint32 includeStatic)
{
	int ret=RT_ERR_OK;
	ret = rtk_l2_addr_delAll(includeStatic);
	return ret;
}

int32 RTK_L2_IPMCASTADDR_ADD(rtk_l2_ipMcastAddr_t *pIpmcastAddr)
{
#if 1
	int ret;
	ret=rtk_l2_ipMcastAddr_add(pIpmcastAddr);
#else
	int ret=RT_ERR_OK;
	pIpmcastAddr->index =_rtk_rg_findAndReclamIpmcEntry(pIpmcastAddr->sip,pIpmcastAddr->dip,0,LAN_FID,0,0);
#endif
	if(ret==RT_ERR_OK)
	{
#if defined(CONFIG_RG_RTL9602C_SERIES)
		if(pIpmcastAddr->flags & RTK_L2_IPMCAST_FLAG_IPV6)
			rg_db.lut[pIpmcastAddr->index].rtk_lut.entryType=RTK_LUT_L3V6MC;
		else
#endif
		{
			rg_db.lut[pIpmcastAddr->index].rtk_lut.entryType=RTK_LUT_L3MC;
		}

		memcpy(&rg_db.lut[pIpmcastAddr->index].rtk_lut.entry.ipmcEntry,pIpmcastAddr,sizeof(rtk_l2_ipMcastAddr_t));
		rg_db.lut[pIpmcastAddr->index].valid=1;


#if defined(CONFIG_RG_RTL9602C_SERIES)
//support lut traffic bit
		if((pIpmcastAddr->index>=MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE) && (pIpmcastAddr->index<MAX_LUT_HW_TABLE_SIZE))
		{
			if(pIpmcastAddr->flags & RTK_L2_IPMCAST_FLAG_IPV6)
			{
				_rtk_rg_lutCamListAdd(_hash_ipm_dipv6(&(pIpmcastAddr->dip6.ipv6_addr[0])), pIpmcastAddr->index);
			}
			else
			{
				if(pIpmcastAddr->flags &RTK_L2_IPMCAST_FLAG_IVL)
					_rtk_rg_lutCamListAdd(_hash_dip_vidfid_sipidx_sipfilter(1,pIpmcastAddr->dip,pIpmcastAddr->vid,pIpmcastAddr->sip_index,(pIpmcastAddr->flags&RTK_L2_IPMCAST_FLAG_SIP_FILTER)?1:0), pIpmcastAddr->index);
				else
					_rtk_rg_lutCamListAdd(_hash_dip_vidfid_sipidx_sipfilter(0,pIpmcastAddr->dip,pIpmcastAddr->fid,pIpmcastAddr->sip_index,(pIpmcastAddr->flags&RTK_L2_IPMCAST_FLAG_SIP_FILTER)?1:0), pIpmcastAddr->index);
			}
		}
#endif
	}
	return ret;
}

int32 RTK_L2_IPMCASTADDR_DEL(rtk_l2_ipMcastAddr_t *pIpmcastAddr)
{
	int ret;
	ret=rtk_l2_ipMcastAddr_del(pIpmcastAddr);
	if(ret==RT_ERR_OK)
	{
#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support lut traffic bit
		if((pIpmcastAddr->index>=MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE) && (pIpmcastAddr->index<MAX_LUT_HW_TABLE_SIZE))
			_rtk_rg_lutCamListDel(pIpmcastAddr->index);
#endif
		memset(&rg_db.lut[pIpmcastAddr->index],0,sizeof(rtk_rg_table_lut_t));
	}
	return ret;
}

int32 RTK_L2_MCASTADDR_ADD(rtk_l2_mcastAddr_t *pMcastAddr)
{
	int ret;

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	if(pMcastAddr->vid >= MAX_VLAN_HW_TABLE_SIZE)
	{
		TRACE("Change internal-used vid[%d] to vid[0]", pMcastAddr->vid);
		pMcastAddr->vid = 0;
	}
#endif
	ret=rtk_l2_mcastAddr_add(pMcastAddr);
	if(ret==RT_ERR_OK)
	{
		if(!rg_db.lut[pMcastAddr->index].valid)
		{
			rg_db.lut[pMcastAddr->index].valid=1;
			rg_db.lut[pMcastAddr->index].mcStaticRefCnt=0;
		}
		rg_db.lut[pMcastAddr->index].rtk_lut.entryType=RTK_LUT_L2MC;
		memcpy(&rg_db.lut[pMcastAddr->index].rtk_lut.entry.l2McEntry,pMcastAddr,sizeof(rtk_l2_mcastAddr_t));

#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support lut traffic bit
		if((pMcastAddr->index>=MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE) && (pMcastAddr->index<MAX_LUT_HW_TABLE_SIZE))
		{
			if(pMcastAddr->flags&RTK_L2_MCAST_FLAG_IVL)
				_rtk_rg_lutCamListAdd(_rtk_rg_hash_mac_vid_efid(pMcastAddr->mac.octet,pMcastAddr->vid,0), pMcastAddr->index);
			else
				_rtk_rg_lutCamListAdd(_rtk_rg_hash_mac_fid_efid(pMcastAddr->mac.octet,pMcastAddr->fid,0), pMcastAddr->index);
		}
#endif
	}
	return ret;

}

int32 RTK_L2_MCASTADDR_DEL(rtk_l2_mcastAddr_t *pMcastAddr)
{
	int ret;

	ret=rtk_l2_mcastAddr_del(pMcastAddr);
	if(ret==RT_ERR_OK)
	{
#if defined(CONFIG_RG_RTL9600_SERIES)
#else	//support lut traffic bit
		if((pMcastAddr->index>=MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE) && (pMcastAddr->index<MAX_LUT_HW_TABLE_SIZE))
			_rtk_rg_lutCamListDel(pMcastAddr->index);
#endif
		memset(&rg_db.lut[pMcastAddr->index],0,sizeof(rtk_rg_table_lut_t));
	}
	return ret;
}

#if defined(CONFIG_RG_RTL9602C_SERIES)
int32 RTK_L2_IPMCSIPFILTER_ADD(ipaddr_t filterIp,int32 *idx)
{
	int i;
	int ret;
	int index=-1;

	//Is entry exist?
	for(i=0;i<MAX_IPMCFILTER_HW_TABLE_SIZE;i++)
	{
		//find first invaild entry
		if(rg_db.ipmcfilter[i].valid==FALSE && index==-1)
			index=i;
		if((rg_db.ipmcfilter[i].filterIp==filterIp)&&(rg_db.ipmcfilter[i].valid==TRUE))
		{
			rg_db.ipmcfilter[i].ipFilterRefCount++;
			*idx=i;
			return (RT_ERR_OK);
		}
	}

	//check we have vaild inedx
	if(index==-1){ return (RT_ERR_FAILED);}

	ret=rtk_l2_ipmcSipFilter_set(index,filterIp);
	TABLE("ADD FilterTable[%d] Sip=%d.%d.%d.%d  " ,index,(filterIp>>24)&0xff,(filterIp>>16)&0xff,(filterIp>>8)&0xff,(filterIp>>0)&0xff);

	if(ret == RT_ERR_OK)
	{
		rg_db.ipmcfilter[index].valid=1;
		rg_db.ipmcfilter[index].filterIp=filterIp;
		rg_db.ipmcfilter[index].ipFilterRefCount=1;
		for(i =0 ; i< MAX_IPMCGRP_SW_TABLE_SIZE; i++)
		{
			if(rg_db.ipmcgrp[i].valid)
			{
				int32 ruleIdx;
				int32 default_flowIdx;
				rtk_l2_ipMcastAddr_t ipmLut;
				memset(&ipmLut,0,sizeof(rtk_l2_ipMcastAddr_t));
				//get default ipm rule
				default_flowIdx = _rtk_rg_findAndReclamIpmcEntry(0,rg_db.ipmcgrp[i].groupIp,0,LAN_FID,0,0);
				if(default_flowIdx==FAIL || (!(rg_db.lut[default_flowIdx].valid))) RETURN_ERR(RT_ERR_RG_ENTRY_NOT_EXIST);

				//add new rule for this sip
				ruleIdx=_rtk_rg_findAndReclamIpmcEntry(filterIp,rg_db.ipmcgrp[i].groupIp,0,LAN_FID,index,1);
				if(ruleIdx==FAIL) RETURN_ERR(RT_ERR_RG_ENTRY_FULL);
				ipmLut.dip=rg_db.ipmcgrp[i].groupIp;
				ipmLut.portmask.bits[0]= rg_db.lut[default_flowIdx].rtk_lut.entry.ipmcEntry.portmask.bits[0];
				ipmLut.ext_portmask.bits[0]=rg_db.lut[default_flowIdx].rtk_lut.entry.ipmcEntry.ext_portmask.bits[0];
				ipmLut.sip_index = index ;
				ipmLut.flags = rg_db.lut[default_flowIdx].rtk_lut.entry.ipmcEntry.flags | (RTK_L2_IPMCAST_FLAG_SIP_FILTER); //use (default rule)|filter mode
				ipmLut.fid = LAN_FID;
				ipmLut.l3_trans_index = rg_db.lut[default_flowIdx].rtk_lut.entry.ipmcEntry.l3_trans_index;
				ASSERT_EQ(RTK_L2_IPMCASTADDR_ADD(&ipmLut),RT_ERR_OK);
				ASSERT_EQ(ruleIdx,ipmLut.index);
				TABLE("[PATCH] ADD multicast LUT[%d] Sip:%d:%d:%d:%d Gip:%d.%d.%d.%d from Group Table  portMask=%x",ipmLut.index,
					(rg_db.ipmcfilter[index].filterIp>>24)&0xff,(rg_db.ipmcfilter[index].filterIp>>16)&0xff,(rg_db.ipmcfilter[index].filterIp>>8)&0xff,(rg_db.ipmcfilter[index].filterIp)&0xff,
					(ipmLut.dip>>24)&0xff,(ipmLut.dip>>16)&0xff,(ipmLut.dip>>8)&0xff,(ipmLut.dip)&0xff,rg_db.ipmcgrp[i].portMsk);


			}
		}
		*idx=index;
		return RT_ERR_OK;
	}
	return (RT_ERR_FAILED);

}

/* output to delIdx */
int32 RTK_L2_IPMCSIPFILTER_DEL(ipaddr_t filterIp,int32 *delIdx)
{
	int i;
	int ret;

	//find entry
	for(i=0;i<MAX_IPMCFILTER_HW_TABLE_SIZE;i++)
	{
		if((rg_db.ipmcfilter[i].filterIp==filterIp)&&(rg_db.ipmcfilter[i].valid==TRUE))
		{
			rg_db.ipmcfilter[i].ipFilterRefCount--;
			*delIdx=i;
			if(rg_db.ipmcfilter[i].ipFilterRefCount == 0)
			{
				ret=rtk_l2_ipmcSipFilter_set(i,0);
				if(ret == RT_ERR_OK)
				{
					int32 j,idx;
					for(j=0;j<MAX_IPMCGRP_SW_TABLE_SIZE;j++)
					{
						if(rg_db.ipmcgrp[j].valid==TRUE)
						{
							idx = _rtk_rg_findAndReclamIpmcEntry(rg_db.ipmcfilter[i].filterIp,rg_db.ipmcgrp[j].groupIp,0,LAN_FID,i,1);
							if(idx==FAIL || (!rg_db.lut[idx].valid)) RETURN_ERR(RT_ERR_RG_ENTRY_NOT_EXIST);
							TABLE("[PATCH] Del multicast LUT[%d] Sip:%d:%d:%d:%d Gip:%d.%d.%d.%d from Group Table  portMask=%x",(rg_db.lut[idx].rtk_lut.entry.ipmcEntry.index),
								(filterIp>>24)&0xff,(filterIp>>16)&0xff,(filterIp>>8)&0xff,(filterIp)&0xff,
								(rg_db.ipmcgrp[j].groupIp>>24)&0xff,(rg_db.ipmcgrp[j].groupIp>>16)&0xff,(rg_db.ipmcgrp[j].groupIp>>8)&0xff,(rg_db.ipmcgrp[j].groupIp)&0xff,rg_db.ipmcgrp[j].portMsk);
							ASSERT_EQ(RTK_L2_IPMCASTADDR_DEL(&(rg_db.lut[idx].rtk_lut.entry.ipmcEntry)),RT_ERR_OK);
						}
					}
					rg_db.ipmcfilter[i].valid=0;
					return RT_ERR_OK;
				}
			}
			else
			{
				return RT_ERR_OK;
			}
		}
	}

	return (RT_ERR_FAILED);
}
#endif


int32 RTK_L2_IPMCGROUP_ADD(ipaddr_t gip, rtk_portmask_t *pPortmask,int32 *idx)
{
	int i,first_invaild=-1,ret=RT_ERR_OK;

	//Is entry exist?
	for(i=0;i<MAX_IPMCGRP_SW_TABLE_SIZE;i++)
	{
		if(first_invaild==-1 && rg_db.ipmcgrp[i].valid==FALSE)
			first_invaild=i;

		if((rg_db.ipmcgrp[i].groupIp==gip)&&(rg_db.ipmcgrp[i].valid==TRUE))
		{
			if(rg_db.ipmcgrp[i].portMsk.bits[0] != pPortmask->bits[0])
			{
				WARNING("Error portmask %x != %x",rg_db.ipmcgrp[i].portMsk.bits[0],pPortmask->bits[0]);
				return (RT_ERR_FAILED);
			}
			rg_db.ipmcgrp[i].ipmRefCount++;
			*idx= i;
			TABLE(" multicast Gip:%d.%d.%d.%d to Group Table %s portMask=%x refCount=%d ",
				(gip>>24)&0xff,(gip>>16)&0xff,(gip>>8)&0xff,(gip)&0xff,pPortmask->bits[0]?"excludeMode":"includeMode",pPortmask->bits[0],rg_db.ipmcgrp[i].ipmRefCount);
			return (RT_ERR_OK);
		}
	}

	if(first_invaild==-1)
	{
		WARNING("Group Table FULL FAILED ");
		return (RT_ERR_FAILED);
	}

#if defined(CONFIG_RG_RTL9600_SERIES)
	ret=rtk_l2_ipmcGroup_add(gip,pPortmask);
#elif defined(CONFIG_RG_RTL9602C_SERIES)
{
	//Sure default rule added
	int32 default_flowIdx;
	default_flowIdx = _rtk_rg_findAndReclamIpmcEntry(0,gip,0,LAN_FID,0,0);
	if(default_flowIdx==FAIL || (!(rg_db.lut[default_flowIdx].valid))) RETURN_ERR(RT_ERR_RG_ENTRY_NOT_EXIST);


	for(i=0;i<MAX_IPMCFILTER_HW_TABLE_SIZE;i++)
	{
		if(rg_db.ipmcfilter[i].valid )
		{
			int32 AddIdx;
			rtk_l2_ipMcastAddr_t lut;
			memset(&lut,0,sizeof(rtk_l2_ipMcastAddr_t));
			AddIdx=_rtk_rg_findAndReclamIpmcEntry(rg_db.ipmcfilter[i].filterIp,gip,0,LAN_FID,i,1);
			if(AddIdx==FAIL) RETURN_ERR(RT_ERR_RG_ENTRY_FULL);
			lut.dip=gip;
			lut.portmask.bits[0]=rg_db.lut[default_flowIdx].rtk_lut.entry.ipmcEntry.portmask.bits[0];
			lut.ext_portmask.bits[0] =rg_db.lut[default_flowIdx].rtk_lut.entry.ipmcEntry.ext_portmask.bits[0];
			lut.flags=rg_db.lut[default_flowIdx].rtk_lut.entry.ipmcEntry.flags  | (RTK_L2_IPMCAST_FLAG_SIP_FILTER); //use (default rule)|filter mode
			lut.sip_index =i;
			lut.fid = LAN_FID;
			lut.l3_trans_index = rg_db.lut[default_flowIdx].rtk_lut.entry.ipmcEntry.l3_trans_index ;
			ASSERT_EQ(RTK_L2_IPMCASTADDR_ADD(&lut),RT_ERR_OK);
			ASSERT_EQ(AddIdx,lut.index);
			TABLE("[PATHC] ADD multicast LUT[%d] Sip:%d:%d:%d:%d Gip:%d.%d.%d.%d from Group Table portMask=%x",lut.index,
				(rg_db.ipmcfilter[i].filterIp>>24)&0xff,(rg_db.ipmcfilter[i].filterIp>>16)&0xff,(rg_db.ipmcfilter[i].filterIp>>8)&0xff,(rg_db.ipmcfilter[i].filterIp)&0xff,
				(gip>>24)&0xff,(gip>>16)&0xff,(gip>>8)&0xff,(gip)&0xff,rg_db.ipmcgrp[first_invaild].portMsk);

		}
	}
}
#endif
	if(ret==RT_ERR_OK)
	{

		TABLE("ADD multicast Group Table[%d]  Gip:%d.%d.%d.%d   %s portMask=%x",first_invaild,
			(gip>>24)&0xff,(gip>>16)&0xff,(gip>>8)&0xff,(gip)&0xff,pPortmask->bits[0]?"excludeMode":"includeMode",pPortmask->bits[0]);
		rg_db.ipmcgrp[first_invaild].groupIp=gip;
		rg_db.ipmcgrp[first_invaild].portMsk.bits[0]=pPortmask->bits[0];
		rg_db.ipmcgrp[first_invaild].valid=TRUE;
		rg_db.ipmcgrp[first_invaild].ipmRefCount=1;
		*idx=first_invaild;
	}
	return ret;


}

int32 RTK_L2_IPMCGROUP_DEL(ipaddr_t gip,int32 *delIdx)
{
	int i;
	for(i=0;i<MAX_IPMCGRP_SW_TABLE_SIZE;i++)
	{
		if((rg_db.ipmcgrp[i].groupIp==gip)&&(rg_db.ipmcgrp[i].valid==TRUE))
		{
			int r=RT_ERR_OK;
			rg_db.ipmcgrp[i].ipmRefCount--;
			TABLE("Group Table Gip:%d.%d.%d.%d Refconunt=%d ",(gip>>24)&0xff,(gip>>16)&0xff,(gip>>8)&0xff,(gip)&0xff,rg_db.ipmcgrp[i].ipmRefCount);
			if(	rg_db.ipmcgrp[i].ipmRefCount == 0)
			{
#if defined(CONFIG_RG_RTL9600_SERIES)
				r=rtk_l2_ipmcGroup_del(gip);
#elif defined(CONFIG_RG_RTL9602C_SERIES)
				//group table invaild,delete default rule
				int32 default_flowIdx,sipFilter_idx,j;
				default_flowIdx = _rtk_rg_findAndReclamIpmcEntry(0,gip,0,LAN_FID,0,0);
				if(default_flowIdx==FAIL) RETURN_ERR(RT_ERR_RG_ENTRY_NOT_EXIST);
				ASSERT_EQ(RTK_L2_IPMCASTADDR_DEL(&(rg_db.lut[default_flowIdx].rtk_lut.entry.ipmcEntry)),RT_ERR_OK);
				//delete for all sip filter rule
				for(j=0 ; j<MAX_IPMCFILTER_HW_TABLE_SIZE ; j++)
				{
					if(rg_db.ipmcfilter[j].valid)
					{
						sipFilter_idx= _rtk_rg_findAndReclamIpmcEntry(rg_db.ipmcfilter[j].filterIp,gip,0,LAN_FID,j,1);
						if(sipFilter_idx==FAIL) RETURN_ERR(RT_ERR_RG_ENTRY_NOT_EXIST);
						TABLE("[PATCH] Del multicast LUT[%d] Sip:%d:%d:%d:%d Gip:%d.%d.%d.%d from Group Table %s portMask=%x",rg_db.lut[sipFilter_idx].rtk_lut.entry.ipmcEntry.index,
							(rg_db.ipmcfilter[j].filterIp>>24)&0xff,(rg_db.ipmcfilter[j].filterIp>>16)&0xff,(rg_db.ipmcfilter[j].filterIp>>8)&0xff,(rg_db.ipmcfilter[j].filterIp)&0xff,
							(gip>>24)&0xff,(gip>>16)&0xff,(gip>>8)&0xff,(gip)&0xff,rg_db.ipmcgrp[i].portMsk.bits[0]?"excludeMode":"includeMode",rg_db.ipmcgrp[i].portMsk);
						ASSERT_EQ(RTK_L2_IPMCASTADDR_DEL(&(rg_db.lut[sipFilter_idx].rtk_lut.entry.ipmcEntry)),RT_ERR_OK);
					}
				}
#endif
				if(r==RT_ERR_OK) rg_db.ipmcgrp[i].valid=FALSE;
				TABLE("Del multicast GroupTable[%d] Gip:%d.%d.%d.%d from Group Table %s portMask=%x",i,
					(gip>>24)&0xff,(gip>>16)&0xff,(gip>>8)&0xff,(gip)&0xff,rg_db.ipmcgrp[i].portMsk.bits[0]?"excludeMode":"includeMode",rg_db.ipmcgrp[i].portMsk);
			}
			*delIdx=i;
			return r;
		}
	}
	return (RT_ERR_FAILED);
}

int32 RTK_L2_PORTLIMITLEARNINGCNT_SET(rtk_port_t port, uint32 macCnt)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_l2_portLimitLearningCnt_set(port, macCnt);
}

int32 RTK_L2_PORTLIMITLEARNINGCNTACTION_SET(rtk_port_t port, rtk_l2_limitLearnCntAction_t learningAction)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_l2_portLimitLearningCntAction_set(port, learningAction);
}

int32 RTK_L2_PORTLOOKUPMISSACTION_SET(rtk_port_t port, rtk_l2_lookupMissType_t type, rtk_action_t action)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))
	{
		//just return without changing anything
	}
	else
		ret=rtk_l2_portLookupMissAction_set(port, type, action);
	return ret;
}

int32 RTK_L2_NEWMACOP_SET(rtk_port_t port,rtk_l2_newMacLrnMode_t lrnMode,rtk_action_t fwdAction)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_l2_newMacOp_set(port, lrnMode, fwdAction);
}

int32 RTK_L2_ILLEGALPORTMOVEACTION_SET(rtk_port_t port,rtk_action_t fwdAction)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_l2_illegalPortMoveAction_set(port, fwdAction);
}

int32 RTK_L2_SRCPORTEGRFILTERMASK_SET(rtk_portmask_t * pFilter_portmask)
{
	int ret=RT_ERR_OK;
	rtk_portmask_t portmask;
	portmask.bits[0]=(pFilter_portmask->bits[0])&rg_db.systemGlobal.phyPortStatus;
#if !defined(CONFIG_RG_G3_SERIES)
	ret = rtk_l2_srcPortEgrFilterMask_set(&portmask);
#endif
	return ret;
}
int32 RTK_L2_PORTAGINGENABLE_SET(rtk_port_t port, rtk_enable_t enable)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))
	{
		//just return without changing anything
	}
	else
		ret=rtk_l2_portAgingEnable_set(port,enable);
	return ret;
}

int32 RTK_L2_VIDUNMATCHACTION_SET(rtk_port_t port, rtk_action_t fwdAction)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
#if !defined(CONFIG_RG_G3_SERIES)
	ret=rtk_l2_vidUnmatchAction_set(port,fwdAction);
#endif
	return ret;

}

int32 RTK_L34_NETIFTABLE_SET(uint32 idx, rtk_l34_netif_entry_t *entry)
{
	int ret=RT_ERR_OK;
#if !defined(CONFIG_RG_FLOW_BASED_PLATFORM)
{
	if(idx>=MAX_NETIF_SW_TABLE_SIZE)
		return RT_ERR_ENTRY_INDEX;
	if(idx < MAX_NETIF_HW_TABLE_SIZE)
		ret=rtk_l34_netifTable_set(idx,entry);
}

#else	// CONFIG_RG_FLOW_BASED_PLATFORM

#if defined(CONFIG_RG_G3_SERIES)
{

}
#else	// not CONFIG_RG_G3_SERIES
{
	rtk_rg_asic_netif_entry_t netifEnt;
	if(idx>=MAX_NETIF_SW_TABLE_SIZE)
		return RT_ERR_ENTRY_INDEX;
	bzero(&netifEnt, sizeof(netifEnt));
	if(entry->valid)
	{
		netifEnt.valid = entry->valid;
		netifEnt.gateway_ipv4_addr = entry->ipAddr;
		memcpy(&netifEnt.gateway_mac_addr.octet[0], &entry->gateway_mac.octet[0], ETHER_ADDR_LEN);
		netifEnt.deny_ipv4 = entry->deny_ipv4;
		netifEnt.deny_ipv6 = entry->deny_ipv6;
		// Configure default setting first
		netifEnt.intf_mtu_check = (entry->mtu==DEFAULT_BRIDGE_WAN_MTU)?FALSE:TRUE;
		netifEnt.intf_mtu = entry->mtu;
		netifEnt.out_pppoe_act = FB_NETIFPPPOE_ACT_KEEP;
		netifEnt.allow_ingress_portmask.bits[0] = RTK_RG_ALL_MAC_PORTMASK;
		netifEnt.allow_ingress_ext_portmask.bits[0] = RTK_RG_ALL_VIRUAL_PORTMASK;
	}

	if(idx < MAX_NETIF_HW_TABLE_SIZE)
		ret=rtk_rg_asic_netifTable_add(idx, &netifEnt);
}
#endif 	// defined(CONFIG_RG_G3_SERIES)
#endif 	// !defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	if(ret==RT_ERR_OK)
	{
		memcpy(&rg_db.netif[idx].rtk_netif,entry,sizeof(*entry));
		if((rg_db.netif[idx].rtk_netif.valid==0) && (0<=rg_db.netif[idx].l2_idx && rg_db.netif[idx].l2_idx<MAX_LUT_HW_TABLE_SIZE) && (rg_db.lut[rg_db.netif[idx].l2_idx].valid))
		{
			uint32 i, delL2Idx=TRUE;

			for(i=0; i<MAX_NETIF_SW_TABLE_SIZE; i++)
			{
				if(rg_db.netif[i].rtk_netif.valid && rg_db.netif[i].l2_idx==rg_db.netif[idx].l2_idx)
				{
					delL2Idx=FALSE;
					break;
				}
			}
			if(delL2Idx)
			{
				ret=(pf.rtk_rg_macEntry_del)(rg_db.netif[idx].l2_idx);
			}
		}
	}
	return ret;
}

extern int _rtk_rg_aclAndCfReservedRuleDel(rtk_rg_aclAndCf_reserved_type_t rsvType);
extern int _rtk_rg_aclAndCfReservedRuleAdd(rtk_rg_aclAndCf_reserved_type_t rsvType, void *parameter);


int32 RTK_L34_ROUTINGTABLE_SET(uint32 idx, rtk_l34_routing_entry_t *entry)
{
	int ret=RT_ERR_OK;
	if(idx >= MAX_L3_SW_TABLE_SIZE)
		return RT_ERR_ENTRY_INDEX;
	//delete  entry
	if(entry->valid==0)
	{
		if(idx < MAX_L3_HW_TABLE_SIZE)
			ret=rtk_l34_routingTable_set(idx,entry);
		if(ret==RT_ERR_OK)
			rg_db.l3[idx].valid = INVALID_ENTRY;
	}
	else //if(entry->valid==1)  //add entry
	{
		//20171106LUKE: enable rt2wan here for checking bind before goto protocol stack.
    	if(idx==V4_DEFAULT_ROUTE_IDX)entry->rt2waninf=1;
		if((idx >= MAX_L3_HW_TABLE_SIZE) || ( entry->netifIdx >=MAX_NETIF_HW_TABLE_SIZE) || (entry->nhStart >=MAX_NEXTHOP_HW_TABLE_SIZE))
		{
			rg_db.l3[idx].valid = SOFTWARE_ONLY_ENTRY;
		}
		else
		{
			ret=rtk_l34_routingTable_set(idx,entry);
			if(ret==RT_ERR_OK)
				rg_db.l3[idx].valid = SOFTWARE_HARDWARE_SYNC_ENTRY;
		}
	}

	if(ret==RT_ERR_OK)
	{
		memcpy(&rg_db.l3[idx].rtk_l3,entry,sizeof(*entry));
		if(entry->ipMask==0)
			rg_db.l3[idx].netmask=0;
		else
			rg_db.l3[idx].netmask=~((1<<(31-entry->ipMask))-1);
		//20160601LUKE: initialize gateway IP
		if(!rg_db.l3[idx].rtk_l3.valid)rg_db.l3[idx].gateway_ip=0;
	}
	return ret;
}

int32 RTK_L34_EXTINTIPTABLE_SET(uint32 idx, rtk_l34_ext_intip_entry_t *entry)
{
	int ret=RT_ERR_OK;

	if(entry->valid==0)
	{	//delete entry
		if(idx < MAX_EXTIP_HW_TABLE_SIZE)
			ret=rtk_l34_extIntIPTable_set(idx,entry);
		if(ret==RT_ERR_OK)
			rg_db.extip[idx].valid = INVALID_ENTRY;
	}
	else
	{	//add entry
		if((idx >= MAX_EXTIP_HW_TABLE_SIZE) || (entry->nhIdx >=MAX_NEXTHOP_HW_TABLE_SIZE) || rg_db.nexthop[entry->nhIdx].valid==SOFTWARE_ONLY_ENTRY)
		{
			rg_db.extip[idx].valid =SOFTWARE_ONLY_ENTRY;
		}
		else
		{
			ret=rtk_l34_extIntIPTable_set(idx,entry);
			if(ret==RT_ERR_OK)
				rg_db.extip[idx].valid =SOFTWARE_HARDWARE_SYNC_ENTRY;
		}
	}

	if(ret==RT_ERR_OK)
	{
		//20160527LUKE: keep extip index in nexthop table for link.
		if(entry->valid)
			rg_db.nexthop[entry->nhIdx].extIPMask|=0x1<<idx;
		else
			rg_db.nexthop[rg_db.extip[idx].rtk_extip.nhIdx].extIPMask&=(~(0x1<<idx));	//20170523LUKE: dismiss bitmask in nexthop when deleting extIP.
		memcpy(&rg_db.extip[idx].rtk_extip,entry,sizeof(*entry));
	}
	return ret;
}


//WARNING:caller should set rg_db.nexthop[idx].valid=(0/1) before RTK_L34_NEXTHOPTABLE_SET
int32 RTK_L34_NEXTHOPTABLE_SET(uint32 idx, rtk_l34_nexthop_entry_t *entry)
{
	int ret=RT_ERR_OK;

	if(rg_db.nexthop[idx].valid==INVALID_ENTRY)
	{	//delete entry
		if( (entry->ifIdx >= MAX_NETIF_HW_TABLE_SIZE) || (idx>= MAX_NEXTHOP_HW_TABLE_SIZE) || (entry->pppoeIdx >=MAX_PPPOE_HW_TABLE_SIZE))
			ret=RT_ERR_OK;
		else
			ret=rtk_l34_nexthopTable_set(idx,entry);
		//rg_db.nexthop[idx].valid =INVALID_ENTRY;
	}
	else
	{	//add entry
		if( (entry->ifIdx >= MAX_NETIF_HW_TABLE_SIZE) || (idx>= MAX_NEXTHOP_HW_TABLE_SIZE) || (entry->pppoeIdx >=MAX_PPPOE_HW_TABLE_SIZE))
		{
			rg_db.nexthop[idx].valid=SOFTWARE_ONLY_ENTRY;
		}
		else
		{

			ret=rtk_l34_nexthopTable_set(idx,entry);
			if(ret==RT_ERR_OK)
				rg_db.nexthop[idx].valid=SOFTWARE_HARDWARE_SYNC_ENTRY;
		}
	}

	if(ret==RT_ERR_OK)
	{
		memcpy(&rg_db.nexthop[idx].rtk_nexthop,entry,sizeof(*entry));
	}
	return ret;
}

int32 RTK_L34_PPPOETABLE_SET(uint32 idx, rtk_l34_pppoe_entry_t *entry)
{
	int ret=RT_ERR_OK;

	if(entry->sessionID==0)
	{	//delete entry
		if(idx < MAX_PPPOE_HW_TABLE_SIZE)
			ret=rtk_l34_pppoeTable_set(idx,entry);
		if(ret==RT_ERR_OK)
			rg_db.pppoe[idx].valid=INVALID_ENTRY;
	}
	else
	{	//add entry
		if(idx < MAX_PPPOE_HW_TABLE_SIZE)
		{
			ret=rtk_l34_pppoeTable_set(idx,entry);
			if(ret==RT_ERR_OK)
				rg_db.pppoe[idx].valid=SOFTWARE_HARDWARE_SYNC_ENTRY;
		}
		else
		{
			rg_db.pppoe[idx].valid=SOFTWARE_ONLY_ENTRY;
		}
	}


	if(ret==RT_ERR_OK)
	{
		memcpy(&rg_db.pppoe[idx].rtk_pppoe,entry,sizeof(*entry));
		rg_db.pppoe[idx].idleSecs=0;
	}
	return ret;
}

int32 RTK_L34_ARPTABLE_SET(uint32 idx, rtk_l34_arp_entry_t *entry)
{
	int ret;
	uint32 arpL2Idx = rg_db.arp[idx].rtk_arp.nhIdx;
	//printk("rtk_l34_arpTable_set(%d,idx=%d,ipAddr=0x%x,nhIdx=0x%x,valid=%d)\n",idx,entry->index,entry->ipAddr,entry->nhIdx,entry->valid);
	ret=rtk_l34_arpTable_set(idx,entry);
	if(ret==RT_ERR_OK)
	{
		memcpy(&rg_db.arp[idx].rtk_arp,entry,sizeof(*entry));
		//20170307LUKE: decrease count when we delete permited ARP.
		if(!entry->valid)
		{
			if(rg_db.arp[idx].permit_for_l34_forward==WanAccessLimitPermit)
			{
				if(rg_db.systemGlobal.activeLimitFunction==RG_ACCESSWAN_TYPE_PORTMASK)
				{
					atomic_dec(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
					rg_db.arp[idx].permit_for_l34_forward=WanAccessLimitForbid;
				}
			}
			else _rtk_rg_disablePortmaskPermitedLut(arpL2Idx);//20180111LUKE: remove l34permit when arp is invalided.
		}
	}
	return ret;
}
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
int32 RTK_L34_NAPTINBOUNDTABLE_SET(int8 forced,uint32 idx, rtk_rg_naptInbound_entry_t *entry)
#else
int32 RTK_L34_NAPTINBOUNDTABLE_SET(int8 forced,uint32 idx, rtk_l34_naptInbound_entry_t *entry)
#endif
{
	int ret=RT_ERR_OK;
	if((idx>=MAX_NAPT_IN_HW_TABLE_SIZE) || (entry->extIpIdx >=MAX_EXTIP_HW_TABLE_SIZE))
		ret=RT_ERR_OK;
	else
	{
#if !defined(CONFIG_RG_FLOW_BASED_PLATFORM)
		ret=rtk_l34_naptInboundTable_set(forced,idx,entry);
#endif
	}

	if(ret==RT_ERR_OK)
	{
		memcpy(&rg_db.naptIn[idx].rtk_naptIn,entry,sizeof(*entry));
	}
	return ret;
}

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
int32 RTK_L34_NAPTOUTBOUNDTABLE_SET(int8 forced,uint32 idx, rtk_rg_naptOutbound_entry_t *entry)
#else
int32 RTK_L34_NAPTOUTBOUNDTABLE_SET(int8 forced,uint32 idx, rtk_l34_naptOutbound_entry_t *entry)
#endif
{
	int ret=RT_ERR_OK;
	if(idx>=MAX_NAPT_OUT_HW_TABLE_SIZE)
		ret=RT_ERR_OK;
	else
	{
#if !defined(CONFIG_RG_FLOW_BASED_PLATFORM)
		ret=rtk_l34_naptOutboundTable_set(forced,idx,entry);
#endif
		}


	if(ret==RT_ERR_OK)
	{
		memcpy(&rg_db.naptOut[idx].rtk_naptOut,entry,sizeof(*entry));
	}
	return ret;
}

//WARNING caller should set  rg_db.wantype[idx].valid = (0/1) before RTK_L34_WANTYPETABLE_SET
int32 RTK_L34_WANTYPETABLE_SET(uint32 idx,rtk_wanType_entry_t * entry)
{
	int ret=RT_ERR_OK;

	if(idx >= MAX_WANTYPE_SW_TABLE_SIZE)
		return RT_ERR_ENTRY_INDEX;

#if defined(CONFIG_APOLLO_RLE0371)
	ret=RT_ERR_OK;
#else

	if(rg_db.wantype[idx].valid==INVALID_ENTRY)
	{
		//delete entry
		if((idx >= MAX_WANTYPE_HW_TABLE_SIZE) || (entry->nhIdx >=MAX_NEXTHOP_HW_TABLE_SIZE) || (rg_db.nexthop[entry->nhIdx].valid==SOFTWARE_ONLY_ENTRY))
			ret=RT_ERR_OK;
		else
			ret=rtk_l34_wanTypeTable_set(idx,entry);
		//rg_db.wantype[idx].valid =INVALID_ENTRY ;
	}
	else
	{
		//add entry
		if((idx >= MAX_WANTYPE_HW_TABLE_SIZE) || (entry->nhIdx >=MAX_NEXTHOP_HW_TABLE_SIZE) || (rg_db.nexthop[entry->nhIdx].valid==SOFTWARE_ONLY_ENTRY))
		{
			rg_db.wantype[idx].valid = SOFTWARE_ONLY_ENTRY;
		}
		else
		{
			ret=rtk_l34_wanTypeTable_set(idx,entry);
			if(ret==RT_ERR_OK)
				rg_db.wantype[idx].valid = SOFTWARE_HARDWARE_SYNC_ENTRY;
		}
	}

#endif
	if(ret==RT_ERR_OK)
	{
		memcpy(&rg_db.wantype[idx].rtk_wantype,entry,sizeof(rtk_wanType_entry_t));
	}
	return ret;
}

int32 RTK_L34_BINDINGACTION_SET(rtk_l34_bindType_t bindType,rtk_l34_bindAct_t bindAction)
{
	int ret;
#if defined(CONFIG_APOLLO_RLE0371) || defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	ret=RT_ERR_OK;
#else
	ret=rtk_l34_bindingAction_set(bindType,bindAction);
#endif
	if(ret==RT_ERR_OK)
	{
		rg_db.systemGlobal.l34BindAction[bindType]=bindAction;
	}
	return ret;
}

#if !defined(CONFIG_RG_FLOW_BASED_PLATFORM)
int32 RTK_L34_GLOBALSTATE_SET(rtk_l34_globalStateType_t stateType,rtk_enable_t state)
{
	int ret;

	ret=rtk_l34_globalState_set(stateType,state);
	if(ret==RT_ERR_OK)
	{
		rg_db.systemGlobal.l34GlobalState[stateType]=state;
	}
	return ret;
}
#endif

int32 RTK_L34_BINDINGTABLE_SET(uint32 idx,rtk_binding_entry_t * bindEntry)
{
	int ret=RT_ERR_OK;
	if(idx>=MAX_BIND_SW_TABLE_SIZE)
		return RT_ERR_ENTRY_INDEX;

	if(bindEntry->portMask.bits[0]==0 && bindEntry->extPortMask.bits[0]==0)
	{
		//delete entry
		TABLE("delete binding[%d]",idx);
		if( (idx>=MAX_BIND_HW_TABLE_SIZE) || (bindEntry->wanTypeIdx >=MAX_WANTYPE_HW_TABLE_SIZE) || (rg_db.wantype[bindEntry->wanTypeIdx].valid==SOFTWARE_ONLY_ENTRY))
			ret=RT_ERR_OK;
		else
			ret=rtk_l34_bindingTable_set(idx,bindEntry);
		rg_db.bind[idx].valid=INVALID_ENTRY;
	}
	else
	{
		//add entry
		TABLE("add binding[%d] bindEntry->wanTypeIdx=%d",idx,bindEntry->wanTypeIdx);
		if( (idx>=MAX_BIND_HW_TABLE_SIZE) || (bindEntry->wanTypeIdx >=MAX_WANTYPE_HW_TABLE_SIZE) || (rg_db.wantype[bindEntry->wanTypeIdx].valid==SOFTWARE_ONLY_ENTRY))
		{
			rg_db.bind[idx].valid=SOFTWARE_ONLY_ENTRY;
		}
		else
		{
			ret=rtk_l34_bindingTable_set(idx,bindEntry);
			if(ret==RT_ERR_OK)
				rg_db.bind[idx].valid=SOFTWARE_HARDWARE_SYNC_ENTRY;
		}
	}

	if(ret==RT_ERR_OK)
	{
		//Copy to software MIB
		memcpy(&rg_db.bind[idx].rtk_bind,bindEntry,sizeof(rtk_binding_entry_t));
		if(rg_db.bind[idx].valid!=INVALID_ENTRY)
			rg_db.systemGlobal.bindingUsed|=(0x1<<idx);
		else
			rg_db.systemGlobal.bindingUsed&=(~(0x1<<idx));

		//20160809LUKE: turn on this reserved ACL only when there is binding in system.
#if defined(CONFIG_RG_RTL9602C_SERIES)
		if(rg_db.systemGlobal.initParam.macBasedTagDecision){
			int i;
			uint8 haveBind=0, bindIfIdx=0;
			rtk_rg_aclAndCf_reserved_none_ip_bind_translate_vid_t none_ip_bind_translate_vid[MAX_NETIF_HW_TABLE_SIZE];
			bzero(&none_ip_bind_translate_vid, sizeof(rtk_rg_aclAndCf_reserved_none_ip_bind_translate_vid_t)*MAX_NETIF_HW_TABLE_SIZE);

			for(i=0; i<MAX_NETIF_HW_TABLE_SIZE; i++)
				_rtk_rg_aclAndCfReservedRuleDel(RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF0_NONE_IP_VLAN_TRANSLATE+i);
			for(i=0; i<MAX_BIND_SW_TABLE_SIZE; i++)
			{
				if(rg_db.bind[i].valid==INVALID_ENTRY)
					continue;

				haveBind=1;
				if(rg_db.wantype[rg_db.bind[i].rtk_bind.wanTypeIdx].rtk_wantype.wanType==L34_WAN_TYPE_L2_BRIDGE
					&& rg_db.bind[i].rtk_bind.vidLan==0)	//port-binding bridge
				{
					bindIfIdx = rg_db.nexthop[rg_db.wantype[rg_db.bind[i].rtk_bind.wanTypeIdx].rtk_wantype.nhIdx].rtk_nexthop.ifIdx;
					if(bindIfIdx<MAX_NETIF_HW_TABLE_SIZE)
					{
						none_ip_bind_translate_vid[bindIfIdx].ingress_vid=rg_db.netif[bindIfIdx].rtk_netif.vlan_id;
						none_ip_bind_translate_vid[bindIfIdx].portmask|=rg_db.bind[i].rtk_bind.portMask.bits[0];
					}
				}
			}
			for(i=0; i<MAX_NETIF_HW_TABLE_SIZE; i++)
			{
				if(none_ip_bind_translate_vid[i].portmask)
					_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF0_NONE_IP_VLAN_TRANSLATE+i, &none_ip_bind_translate_vid[i]);
			}

			if(haveBind==0)//no binding
				_rtk_rg_aclAndCfReservedRuleDel(RTK_RG_ACLANDCF_RESERVED_PORTBINDING_FAIL_TRAP);
			else//have binding
				_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_PORTBINDING_FAIL_TRAP, NULL);
		}
#endif
	}
	return ret;
}

int32 RTK_L34_IPV6ROUTINGTABLE_SET(uint32 idx,rtk_ipv6Routing_entry_t * ipv6RoutEntry)
{
	int ret=RT_ERR_OK;

	if(idx>=MAX_IPV6_ROUTING_SW_TABLE_SIZE)
		return RT_ERR_ENTRY_INDEX;

	if(ipv6RoutEntry->valid==0)
	{
		//delete v6 routing entry
		if(idx < MAX_IPV6_ROUTING_HW_TABLE_SIZE)
			ret=rtk_l34_ipv6RoutingTable_set(idx,ipv6RoutEntry);
		if(ret == RT_ERR_OK)
			rg_db.v6route[idx].valid = INVALID_ENTRY;
	}
	else
	{
		rtk_l34_ipv6RouteType_t orig_type=ipv6RoutEntry->type;
#if !defined(CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT) || !defined(CONFIG_RG_IPV6_NAPT_SUPPORT)			
		//20180518LUKE: for default route with prefix len equal to zero, we should not add it as software only entry.
		int sw_only=(ipv6RoutEntry->ipv6PrefixLen<64)&&(idx!=V6_DEFAULT_ROUTE_IDX);
#endif
		//20171106LUKE: enable rt2wan here for checking bind before goto protocol stack.
    	if(idx==V6_DEFAULT_ROUTE_IDX)ipv6RoutEntry->rt2waninf=1;
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
		//assign HW table to TRAP, SW table keep original
		ipv6RoutEntry->type=L34_IPV6_ROUTE_TYPE_TRAP;

		#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
		if(idx==V6_DEFAULT_ROUTE_IDX){
			//patch for HW: CONFIG_RG_IPV6_NAPT_SUPPORT makes software routing table entry to 8.
			ret=rtk_l34_ipv6RoutingTable_set(V6_DEFAULT_ROUTE_IDX,ipv6RoutEntry);//only force set HWNAT default route to trap, rest routing will not be set to HWNAT while IPv6 NAPT.
			if(ret == RT_ERR_OK)
				rg_db.v6route[idx].valid = SOFTWARE_ONLY_ENTRY ;
		}
		#else
		if((idx >= MAX_IPV6_ROUTING_HW_TABLE_SIZE) ||
			((ipv6RoutEntry->type==L34_IPV6_ROUTE_TYPE_LOCAL) &&(ipv6RoutEntry->nhOrIfidIdx >=MAX_NETIF_HW_TABLE_SIZE)) ||
			((ipv6RoutEntry->type==L34_IPV6_ROUTE_TYPE_GLOBAL) && (ipv6RoutEntry->nhOrIfidIdx >=MAX_NEXTHOP_HW_TABLE_SIZE)))
		{
			rg_db.v6route[idx].valid = SOFTWARE_ONLY_ENTRY ;
			ret=RT_ERR_OK;
		}
		else
		{
			if(sw_only)ipv6RoutEntry->type=L34_IPV6_ROUTE_TYPE_TRAP;
			ret=rtk_l34_ipv6RoutingTable_set(idx,ipv6RoutEntry);
			if(ret == RT_ERR_OK) //software only , stateful  v6 hardware set to trap
				rg_db.v6route[idx].valid = SOFTWARE_ONLY_ENTRY ;
		}
		#endif

		ipv6RoutEntry->type=orig_type;
#else
		//assign HW table to TRAP, SW table keep original
		if(sw_only)ipv6RoutEntry->type=L34_IPV6_ROUTE_TYPE_TRAP;

		if((idx >= MAX_IPV6_ROUTING_HW_TABLE_SIZE) ||
			((ipv6RoutEntry->type==L34_IPV6_ROUTE_TYPE_LOCAL) &&(ipv6RoutEntry->nhOrIfidIdx >=MAX_NETIF_HW_TABLE_SIZE)) ||
			((ipv6RoutEntry->type==L34_IPV6_ROUTE_TYPE_GLOBAL) && (ipv6RoutEntry->nhOrIfidIdx >=MAX_NEXTHOP_HW_TABLE_SIZE)))
		{
			rg_db.v6route[idx].valid = SOFTWARE_ONLY_ENTRY ;
			ret=RT_ERR_OK;
		}
		else
		{
			if(sw_only)ipv6RoutEntry->type=L34_IPV6_ROUTE_TYPE_TRAP;
			ret=rtk_l34_ipv6RoutingTable_set(idx,ipv6RoutEntry);
			if(ret == RT_ERR_OK)
			{
				if(sw_only)
					rg_db.v6route[idx].valid = SOFTWARE_ONLY_ENTRY ;
				else
					rg_db.v6route[idx].valid = SOFTWARE_HARDWARE_SYNC_ENTRY ;
			}
		}

		if(sw_only)ipv6RoutEntry->type=orig_type;
#endif

	}

	if(ret==RT_ERR_OK)
	{
		//Copy to software MIB
		memcpy(&rg_db.v6route[idx].rtk_v6route,ipv6RoutEntry,sizeof(rtk_ipv6Routing_entry_t));
		//20160601LUKE: initialize gateway IP
		if(!rg_db.v6route[idx].rtk_v6route.valid)bzero(&rg_db.v6route[idx].gateway_ipv6Addr,sizeof(rtk_ipv6_addr_t));
	}
	return ret;
}

int32 RTK_L34_IPV6NEIGHBORTABLE_SET(uint32 idx,rtk_ipv6Neighbor_entry_t * ipv6NeighborEntry)
{
	int ret;
	uint32 nbL2Idx = rg_db.v6neighbor[idx].neighborEntry.l2Idx;
	if(idx<0 || idx>=MAX_IPV6_NEIGHBOR_SW_TABLE_SIZE)
		return RT_ERR_ENTRY_INDEX;

	if(idx>=MAX_IPV6_NEIGHBOR_HW_TABLE_SIZE)
		ret=RT_ERR_OK;
	else
		ret=rtk_l34_ipv6NeighborTable_set(idx,ipv6NeighborEntry);
	if(ret==RT_ERR_OK)
	{
		//Copy to software MIB
		rg_db.v6neighbor[idx].neighborEntry.valid=ipv6NeighborEntry->valid;
		rg_db.v6neighbor[idx].neighborEntry.l2Idx=ipv6NeighborEntry->l2Idx;
		rg_db.v6neighbor[idx].neighborEntry.matchRouteIdx=ipv6NeighborEntry->ipv6RouteIdx;
		//20180419LUKE: we don't copy ifid here, since we store whole IPv6 address in software structure.
		//20180111LUKE: remove l34permit when nb is invalided.
		if(!ipv6NeighborEntry->valid)_rtk_rg_disablePortmaskPermitedLut(nbL2Idx);
	}
	return ret;
}


int32 RTK_CLASSIFY_CFSEL_SET(rtk_port_t port, rtk_classify_cf_sel_t cfSel)
{
	int ret=RT_ERR_OK;
#if !defined(CONFIG_RG_G3_SERIES) && !defined(CONFIG_RG_RTL9603CVD_SERIES)
	ret=rtk_classify_cfSel_set(port,cfSel);
#endif
	if(ret==RT_ERR_OK)
	{
		if(port==RTK_RG_MAC_PORT_PON)
		{
			rg_db.systemGlobal.pon_is_cfport = cfSel;
		}
#if defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RTL9601B_SERIES)
		else if(port==RTK_RG_MAC_PORT_RGMII)
		{
			rg_db.systemGlobal.rgmii_is_cfport = cfSel;
		}
#endif
	}

	return ret;
}



int32 RTK_VLAN_CREATE(rtk_vlan_t vid)
{
	int ret=RT_ERR_OK;

	//20190828LUKE: check vlan boundary at first
	if(vid>=MAX_VLAN_SW_TABLE_SIZE)
		return RT_ERR_VLAN_VID;

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
#if !defined(CONFIG_RG_G3_SERIES)
	if(vid<MAX_VLAN_HW_TABLE_SIZE)
		ret=rtk_vlan_create(vid);
	else
#endif
	{
		if(rg_db.vlan[vid].valid)
			ret = RT_ERR_VLAN_EXIST;
		else
		{
			rg_db.vlan[vid].Ext_portmask.bits[0]=0;
			ret = RT_ERR_OK;
		}
	}
#else	// not CONFIG_RG_FLOW_BASED_PLATFORM
	ret=rtk_vlan_create(vid);
#endif

	if(ret==RT_ERR_OK || ret==RT_ERR_VLAN_EXIST)
		rg_db.vlan[vid].valid = 1;		//Enable software MIB record
	return ret;
}

int32 RTK_VLAN_VLANFUNCTIONENABLE_SET(rtk_enable_t enable)
{
	int ret=RT_ERR_OK;
	ret=rtk_vlan_vlanFunctionEnable_set(enable);
	return ret;
}

int32 RTK_VLAN_PORT_SET(rtk_vlan_t vid,rtk_portmask_t * pMember_portmask,rtk_portmask_t * pUntag_portmask)
{
	int ret=RT_ERR_OK;
	rtk_portmask_t portmask,portmask2;

	//20190828LUKE: check vlan boundary at first
	if(vid>=MAX_VLAN_SW_TABLE_SIZE)
		return RT_ERR_VLAN_VID;

	portmask.bits[0]=(pMember_portmask->bits[0])&rg_db.systemGlobal.phyPortStatus;
	portmask2.bits[0]=(pUntag_portmask->bits[0])&rg_db.systemGlobal.phyPortStatus;

#if defined(CONFIG_RG_RTL9600_SERIES)
	if(rg_db.systemGlobal.pppoeGponSmallbandwithControl && (portmask.bits[0]& (1<<RTK_RG_PORT_PON))){
		DEBUG("Have pppoeGponSmallbandwithControl,so add RGMII to memberPortMask.");
		portmask.bits[0] |= (1<<RTK_RG_PORT_RGMII);
	}
	if(rg_db.systemGlobal.pppoeGponSmallbandwithControl && (portmask2.bits[0]& (1<<RTK_RG_PORT_PON))){
		DEBUG("Have pppoeGponSmallbandwithControl,sync RGMII untag to PON.");
		portmask2.bits[0] |= (1<<RTK_RG_PORT_RGMII);
	}
#endif
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	//no need to access rtk api
#else
	if(vid<MAX_VLAN_HW_TABLE_SIZE)
		ret=rtk_vlan_port_set(vid,&portmask,&portmask2);
	else
		ret=RT_ERR_VLAN_VID;
#endif
	if(ret==RT_ERR_OK)
	{
		//Copy to software MIB
		memcpy(&rg_db.vlan[vid].MemberPortmask,&portmask,sizeof(rtk_portmask_t));
		memcpy(&rg_db.vlan[vid].UntagPortmask,&portmask2,sizeof(rtk_portmask_t));

		TABLE("vlan[%d] MemberPortmask=0x%x UntagPortmask=0x%x", vid, rg_db.vlan[vid].MemberPortmask.bits[0], rg_db.vlan[vid].UntagPortmask.bits[0]);
	}
	return ret;
}

int32 RTK_VLAN_EXTPORT_SET(rtk_vlan_t vid,rtk_portmask_t * pExt_portmask)
{
	int ret=RT_ERR_OK;

	//20190828LUKE: check vlan boundary at first
	if(vid>=MAX_VLAN_SW_TABLE_SIZE)
		return RT_ERR_VLAN_VID;

	// if(!rg_db.vlan[vid].valid)
	// 	return RT_ERR_VLAN_ENTRY_NOT_FOUND;
#if !defined (CONFIG_RG_FLOW_BASED_PLATFORM)
	if(vid<MAX_VLAN_HW_TABLE_SIZE)
		ret=rtk_vlan_extPort_set(vid,pExt_portmask);
	else
		ret=RT_ERR_VLAN_VID;
#endif

	if(ret==RT_ERR_OK)
	{
#if defined (CONFIG_RG_FLOW_BASED_PLATFORM)
{
		int32 extPMaskIdx = 0;

		//DEBUG("VID = %d, extpmask = 0x%x", vid, pExt_portmask->bits[0]);
		//del the original setting to sync reference count
		// ADD case: when first time to config vlan table, the ext_maskidx is 0. software skip the del(decrease) action.
		// UPDATE case: the reference count of original extpmask will be decreased, then re-add the extension port info.
		//1 FIXME:we don't dereference extpmsk in Vlan anymore
#if 1
		if((extPMaskIdx = _rtk_rg_extPMaskEntryIdx_get(rg_db.vlan[vid].Ext_portmask.bits[0])) != RG_RET_ENTRY_NOT_GET)
		{
			TABLE("Vlan Del extPMaskIdx=%d",extPMaskIdx);
			ASSERT_EQ(RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(extPMaskIdx), RT_ERR_RG_OK);
		}
#endif
		//add target port mask back to table
		ASSERT_EQ(RTK_RG_ASIC_EXTPORTMASKTABLE_ADD(pExt_portmask->bits[0], &extPMaskIdx), RT_ERR_RG_OK);
		TABLE("Vlan Add extPMaskIdx=%d",extPMaskIdx);
}
#endif

		//Copy to software MIB
		memcpy(&rg_db.vlan[vid].Ext_portmask,pExt_portmask,sizeof(rtk_portmask_t));
	}
	return ret;
}

int32 RTK_VLAN_FID_SET(rtk_vlan_t vid,rtk_fid_t fid)
{
	int ret=RT_ERR_OK;

	//20190828LUKE: check vlan boundary at first
	if(vid>=MAX_VLAN_SW_TABLE_SIZE)
		return RT_ERR_VLAN_VID;

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	//no need to access rtk api
#else
	if(vid<MAX_VLAN_HW_TABLE_SIZE)
		ret=rtk_vlan_fid_set(vid,fid);
	else
		ret=RT_ERR_VLAN_VID;
#endif
	if(ret==RT_ERR_OK)
	{
		//Copy to software MIB
		memcpy(&rg_db.vlan[vid].fid,&fid,sizeof(rtk_fid_t));
	}
	return ret;
}

int32 RTK_VLAN_FIDMODE_SET(rtk_vlan_t vid,rtk_fidMode_t mode)
{

	int ret=RT_ERR_OK;

	//20190828LUKE: check vlan boundary at first
	if(vid>=MAX_VLAN_SW_TABLE_SIZE)
		return RT_ERR_VLAN_VID;

#if !defined(CONFIG_RG_G3_SERIES)
	if(vid<MAX_VLAN_HW_TABLE_SIZE)
		ret=rtk_vlan_fidMode_set(vid,mode);
#if	!defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	else
		ret=RT_ERR_VLAN_VID;
#endif
#endif
	if(ret==RT_ERR_OK)
	{
		//Copy to software MIB
		memcpy(&rg_db.vlan[vid].fidMode,&mode,sizeof(rtk_fidMode_t));
	}
	return ret;
}


int32 RTK_VLAN_PRIORITY_SET(rtk_vlan_t vid,rtk_pri_t priority)
{
	int ret=RT_ERR_OK;

	//20190828LUKE: check vlan boundary at first
	if(vid>=MAX_VLAN_SW_TABLE_SIZE)
		return RT_ERR_VLAN_VID;

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	// not support
#else
	if(vid<MAX_VLAN_HW_TABLE_SIZE)
		ret=rtk_vlan_priority_set(vid,priority);
	else
		ret=RT_ERR_VLAN_VID;
#endif
	if(ret==RT_ERR_OK)
	{
		//Copy to software MIB
		memcpy(&rg_db.vlan[vid].priority,&priority,sizeof(rtk_pri_t));
	}
	return ret;
}


int32 RTK_VLAN_PRIORITYENABLE_SET(rtk_vlan_t vid,rtk_enable_t enable)
{
	int ret=RT_ERR_OK;

	//20190828LUKE: check vlan boundary at first
	if(vid>=MAX_VLAN_SW_TABLE_SIZE)
		return RT_ERR_VLAN_VID;

#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	// not support
#else
	if(vid<MAX_VLAN_HW_TABLE_SIZE)
		ret=rtk_vlan_priorityEnable_set(vid,enable);
	else
		ret=RT_ERR_VLAN_VID;
#endif
	if(ret==RT_ERR_OK)
	{
		//Copy to software MIB
		memcpy(&rg_db.vlan[vid].priorityEn,&enable,sizeof(rtk_enable_t));
	}
	return ret;
}

int32 RTK_VLAN_DESTROY(rtk_vlan_t vid)
{
	int ret=RT_ERR_OK;

	//20190828LUKE: check vlan boundary at first
	if(vid>=MAX_VLAN_SW_TABLE_SIZE)
		return RT_ERR_VLAN_VID;

	// if(!rg_db.vlan[vid].valid)
	// 	return RT_ERR_VLAN_ENTRY_NOT_FOUND;

#if !defined(CONFIG_RG_G3_SERIES)
	if(vid<MAX_VLAN_HW_TABLE_SIZE)
		ret=rtk_vlan_destroy(vid);
#if	!defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	else
		ret=RT_ERR_VLAN_VID;
#endif
#endif
	if(ret==RT_ERR_OK)
	{
		TABLE("RTK_VLAN_DESTROY Vid:%d",vid);
		//1 FIXME:we don't dereference extpmsk in Vlan anymore
#if defined (CONFIG_RG_FLOW_BASED_PLATFORM)
		{
			int32 extPMaskIdx = 0;

			if((extPMaskIdx = _rtk_rg_extPMaskEntryIdx_get(rg_db.vlan[vid].Ext_portmask.bits[0])) != RG_RET_ENTRY_NOT_GET)
			{
				ASSERT_EQ(RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(extPMaskIdx), RT_ERR_RG_OK);
			}
		}
#endif

	}
		bzero(&rg_db.vlan[vid],sizeof(rtk_rg_table_vlan_t));		//Disable software MIB record
	return ret;
}

int32 RTK_VLAN_PORTPVID_SET(rtk_port_t port,uint32 pvid)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
#if !defined(CONFIG_RG_G3_SERIES)
	if(pvid<MAX_VLAN_HW_TABLE_SIZE)
		ret=rtk_vlan_portPvid_set(port,pvid);
#if	!defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	else
		ret=RT_ERR_VLAN_VID;
#endif
#endif
	if(ret==RT_ERR_OK)
	{
		rg_db.systemGlobal.portBasedVID[port]=pvid;
#if defined(CONFIG_RG_RTL9600_SERIES)
		if (rg_kernel.stag_enable==RTK_RG_ENABLED) {
			if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT2) && (rg_db.systemGlobal.service_pmsk.portmask&(1<<port)))
			{
				ASSERT_EQ(_rtk_rg_acl_reserved_stag_ingressCVidFromPVID(pvid, port),RT_ERR_RG_OK);
			}
		}
#endif
	}
	return ret;
}

int32 RTK_VLAN_EXTPORTPVID_SET(uint32 extPort /*Note!! For FB: bit'0 stands for ext0 not cpu port*/,uint32 pvid)
{
	int ret=RT_ERR_OK;
#if !defined(CONFIG_RG_G3_SERIES)
	if(pvid<MAX_VLAN_HW_TABLE_SIZE)
		ret=rtk_vlan_extPortPvid_set(extPort,pvid);
#if	!defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	else
		ret=RT_ERR_VLAN_VID;
#endif
#endif
	if(ret==RT_ERR_OK)
		rg_db.systemGlobal.portBasedVID[extPort+RTK_RG_EXT_BASED_PORT]=pvid;
	return ret;
}

int32 RTK_VLAN_PROTOGROUP_SET(uint32 protoGroupIdx,rtk_vlan_protoGroup_t *pProtoGroup)
{
	int ret=RT_ERR_OK;
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	//no need to access rtk api
#else
	ret=rtk_vlan_protoGroup_set(protoGroupIdx,pProtoGroup);
#endif
	if(ret==RT_ERR_OK)
	{
		//Copy to software MIB
		memcpy(&rg_db.systemGlobal.protoGroup[protoGroupIdx],pProtoGroup,sizeof(rtk_vlan_protoGroup_t));
	}
	return ret;
}

int32 RTK_VLAN_PORTPROTOVLAN_SET(rtk_port_t port,uint32 protoGroupIdx,rtk_vlan_protoVlanCfg_t * pVlanCfg)
{
	int ret=RT_ERR_OK;
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	//no need to access rtk api
#else
	if(pVlanCfg->vid<MAX_VLAN_HW_TABLE_SIZE)
		ret=rtk_vlan_portProtoVlan_set(port,protoGroupIdx,pVlanCfg);
	else
		ret=RT_ERR_VLAN_VID;
#endif
	if(ret==RT_ERR_OK)
	{
		//Copy to software MIB
		memcpy(&rg_db.systemGlobal.protoBasedVID[port].protoVLANCfg[protoGroupIdx],pVlanCfg,sizeof(rtk_vlan_protoVlanCfg_t));
	}
	return ret;
}

int32 RTK_VLAN_PORTIGRFILTERENABLE_SET(rtk_port_t port, rtk_enable_t igr_filter)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	//no need to access rtk api
	return ret;
#else
	return rtk_vlan_portIgrFilterEnable_set(port, igr_filter);
#endif
}

int32 RTK_QOS_1PPRIREMAPGROUP_SET(
    uint32      grpIdx,
    rtk_pri_t   dot1pPri,
    rtk_pri_t   intPri,
    uint32      dp)
{
	int ret=RT_ERR_OK;
#if !defined(CONFIG_RG_G3_SERIES)
	ret=rtk_qos_1pPriRemapGroup_set(grpIdx,dot1pPri,intPri,dp);
#endif
	if(ret==RT_ERR_OK)
		rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemapToInternalPriTbl[dot1pPri]=intPri;

	return ret;
}

int32 RTK_QOS_DSCPPRIREMAPDROUP_SET(
	uint32      grpIdx,
    uint32      dscp,
    rtk_pri_t   intPri,
    uint32      dp)
{
	int ret;
	ret=rtk_qos_dscpPriRemapGroup_set(grpIdx,dscp,intPri,dp);

	if(ret==RT_ERR_OK)
		rg_db.systemGlobal.qosInternalDecision.qosDscpRemapToInternalPri[dscp]=intPri;

	return ret;
}

int32 RTK_QOS_PORTPRI_SET(rtk_port_t port, rtk_pri_t intPri)
{
	int ret=RT_ERR_OK;
#if !defined(CONFIG_RG_G3_SERIES)
	ret=rtk_qos_portPri_set(port,intPri);
#endif
	if(ret==RT_ERR_OK)
		rg_db.systemGlobal.qosInternalDecision.qosPortBasedPriority[port]=intPri;

	return ret;
}

int32 RTK_QOS_PRISELGROUP_SET(uint32 grpIdx, rtk_qos_priSelWeight_t *pWeightOfPriSel)
{
	int ret=RT_ERR_OK;
#if !defined(CONFIG_RG_G3_SERIES)
	ret=rtk_qos_priSelGroup_set(grpIdx,pWeightOfPriSel);
#endif
	if(ret==RT_ERR_OK){
		rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_PORTBASED]=pWeightOfPriSel->weight_of_portBased;
		rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_DOT1Q]=pWeightOfPriSel->weight_of_dot1q;
		rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_DSCP]=pWeightOfPriSel->weight_of_dscp;
		rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_ACL]=pWeightOfPriSel->weight_of_acl;
		rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_LUTFWD]=pWeightOfPriSel->weight_of_lutFwd;
		rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_SABASED]=pWeightOfPriSel->weight_of_saBaed;
		rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_VLANBASED]=pWeightOfPriSel->weight_of_vlanBased;
		rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_SVLANBASED]=pWeightOfPriSel->weight_of_svlanBased;
		rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_L4BASED]=pWeightOfPriSel->weight_of_l4Based;
	}
	return ret;
}

int32 RTK_QOS_PRISELGROUP_GET(uint32 grpIdx, rtk_qos_priSelWeight_t *pWeightOfPriSel)
{
	int ret=RT_ERR_OK;
#if defined(CONFIG_RG_G3_SERIES)
	pWeightOfPriSel->weight_of_portBased=rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_PORTBASED];
	pWeightOfPriSel->weight_of_dot1q=rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_DOT1Q];
	pWeightOfPriSel->weight_of_dscp=rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_DSCP];
	pWeightOfPriSel->weight_of_acl=rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_ACL];
	pWeightOfPriSel->weight_of_lutFwd=rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_LUTFWD];
	pWeightOfPriSel->weight_of_saBaed=rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_SABASED];
	pWeightOfPriSel->weight_of_vlanBased=rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_VLANBASED];
	pWeightOfPriSel->weight_of_svlanBased=rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_SVLANBASED];
	pWeightOfPriSel->weight_of_l4Based=rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[WEIGHT_OF_L4BASED];
#else
	ret=rtk_qos_priSelGroup_get(grpIdx,pWeightOfPriSel);
#endif
	return ret;
}

int32 RTK_QOS_DSCPREMARKENABLE_SET(rtk_port_t port, rtk_enable_t enable)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_qos_dscpRemarkEnable_set(port, enable);
}

int32 RTK_QOS_PORTDSCPREMARKSRCSEL_SET(rtk_port_t port, rtk_qos_dscpRmkSrc_t type)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_qos_portDscpRemarkSrcSel_set(port, type);
}

int32 RTK_QOS_PORTPRIMAP_SET(rtk_port_t port, uint32 group)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
#if !defined(CONFIG_RG_G3_SERIES)
	ret = rtk_qos_portPriMap_set(port, group);
#endif
	return ret;
}

#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
int32  RTK_ACL_IGRRULEENTRY_ADD(rtk_acl_ingress_entry_t *pAclRule, int ref_info){
	int ret;
	int vid;
	//check is the vlan exist in rg_db to avoid H/W ACL auto-create vlan that cause un-sync.
	if( (pAclRule->act.enableAct[ACL_IGR_CVLAN_ACT]==1)&&
		(pAclRule->act.forwardAct.act != ACL_IGR_FORWARD_TRAP_ACT /*trap will keep original, so we can use cvid to debug uaer acl rule index with trap*/) &&
		pAclRule->act.cvlanAct.act==ACL_IGR_CVLAN_IGR_CVLAN_ACT){
		vid = pAclRule->act.cvlanAct.cvid;
		if(rg_db.vlan[vid].valid==0){
			WARNING("ACL is assigning a none-exist VLAN[%d] as ingress_cvid, add rule fail!",vid);
			return RT_ERR_RG_VLAN_NOT_CREATED_BY_CVLAN;
		}
	}
	if( (ref_info != FAIL) && (pAclRule->act.enableAct[ACL_IGR_FORWARD_ACT] == ENABLED) && 
		(pAclRule->act.forwardAct.act == ACL_IGR_FORWARD_TRAP_ACT) &&
		(pAclRule->act.enableAct[ACL_IGR_CVLAN_ACT] != ENABLED)){
	
		pAclRule->act.enableAct[ACL_IGR_CVLAN_ACT] = ENABLED;
		pAclRule->act.cvlanAct.act = ACL_IGR_CVLAN_IGR_CVLAN_ACT;
		if(ref_info >= RTK_RG_ACLANDCF_RESERVED_TAIL_END){
			pAclRule->act.cvlanAct.cvid = ref_info - RTK_RG_ACLANDCF_RESERVED_TAIL_END;
			pAclRule->act.cvlanAct.cvid |= RXINFO_REF_ACL_RSN_CTRL_USER;
		}
		else{
			pAclRule->act.cvlanAct.cvid = ref_info;
			pAclRule->act.cvlanAct.cvid |= RXINFO_REF_ACL_RSN_CTRL_RSV;
		}
		pAclRule->act.cvlanAct.cvid |= RXINFO_REF_ACL_RSN_CTRL_TRAP;
	}

	ret=rtk_acl_igrRuleEntry_add(pAclRule);
	return ret;
}
#else
int32  RTK_ACL_IGRRULEENTRY_ADD(rtk_acl_ingress_entry_t *pAclRule){
	int ret;
	int vid;
	//check is the vlan exist in rg_db to avoid H/W ACL auto-create vlan that cause un-sync.
	if( (pAclRule->act.enableAct[ACL_IGR_CVLAN_ACT]==1)&&
		(pAclRule->act.forwardAct.act != ACL_IGR_FORWARD_TRAP_ACT /*trap will keep original, so we can use cvid to debug uaer acl rule index with trap*/) &&
		pAclRule->act.cvlanAct.act==ACL_IGR_CVLAN_IGR_CVLAN_ACT){
		vid = pAclRule->act.cvlanAct.cvid;
		if(rg_db.vlan[vid].valid==0){
			WARNING("ACL is assigning a none-exist VLAN[%d] as ingress_cvid, add rule fail!",vid);
			return RT_ERR_RG_VLAN_NOT_CREATED_BY_CVLAN;
		}
	}

	ret=rtk_acl_igrRuleEntry_add(pAclRule);
	return ret;
}
#endif

/*avoid L2 CF directly call rtk classify api & using wrong index. it should call rtk_rg_classify_cfgEntry_add()*/
int32 RTK_CLASSIFY_CFGENTRY_ADD(rtk_classify_cfg_t *pClassifyCfg){
	int ret;
#if !defined(CONFIG_RG_RTL9603CVD_SERIES)
	ret=rtk_classify_cfgEntry_add(pClassifyCfg);
#else
	ret= SUCCESS;
#endif
	return ret;
}

/*avoid L2 CF directly call rtk classify api & using wrong index. it should call rtk_rg_classify_cfgEntry_add()*/
int32 RTK_SVLAN_TPIDENTRY_SET(uint32 svlan_index, uint32 svlan_tag_id)
{
	int ret;
#if defined(CONFIG_APOLLO_FPGA_PHY_TEST) || defined(CONFIG_RG_RTL9600_SERIES)
#else
	uint32 val;
#endif

#if defined(CONFIG_RG_G3_SERIES)

	ca_uint32_t outerTPID[3] = {0x8100, 0x88a8, 0x9100};
	ca_uint32_t innerTPID[3] = {0x8100, 0x88a8, 0x9100};

	// TPID[0] is reserved for ctag 0x8100
	outerTPID[svlan_index+1] = svlan_tag_id & 0xffff;
	innerTPID[svlan_index+1] = svlan_tag_id & 0xffff;

	ret = ca_l2_vlan_tpid_set(G3_DEF_DEVID, innerTPID, 3, outerTPID, 3);
#else
	ret=rtk_svlan_tpidEntry_set(svlan_index,svlan_tag_id);
#endif
	if(ret==RT_ERR_OK)
	{
		if(svlan_index==0)
		{
#if defined(CONFIG_APOLLO_FPGA_PHY_TEST) || defined(CONFIG_RG_RTL9600_SERIES)
#elif defined(CONFIG_RG_G3_SERIES)
			val = (0x80000000 | (svlan_tag_id & 0xffff));
			_rtk_rg_g3_devmem_set(DMA_SEC_DMA_GLB_DMA_LSO_VLAN_TAG_TYPE0, val);
			//rtk_ne_reg_write(val, DMA_SEC_DMA_GLB_DMA_LSO_VLAN_TAG_TYPE0);
#else
			struct tx_info txInfo={{{0}},0,{{0}},{{0}}};
			//20170505LUKE: set GMAC for tpid if we are 9602C_series, 9607C_series.
			ASSERT_EQ(re8686_get_vlan_register(&txInfo,svlan_index,&val),0);
			val = ((svlan_tag_id & 0xffff)<<16) | (val & 0xffff);
			ASSERT_EQ(re8686_set_vlan_register(&txInfo,svlan_index,val),0);
#endif
			rg_db.systemGlobal.tpid = svlan_tag_id;
		}
		else if(svlan_index==1)
		{
#if defined(CONFIG_APOLLO_FPGA_PHY_TEST) || defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9602C_SERIES)
#elif defined(CONFIG_RG_G3_SERIES)
			val = (0x80000000 | (svlan_tag_id & 0xffff));
			_rtk_rg_g3_devmem_set(DMA_SEC_DMA_GLB_DMA_LSO_VLAN_TAG_TYPE1, val);
			//rtk_ne_reg_write(val, DMA_SEC_DMA_GLB_DMA_LSO_VLAN_TAG_TYPE1);
#else // gmac support tpid2
			struct tx_info txInfo={{{0}},0,{{0}},{{0}}};
			//20170505LUKE: set GMAC for tpid2 if we are 9607C_series.
			ASSERT_EQ(re8686_get_vlan_register(&txInfo,svlan_index,&val),0);
			val = ((svlan_tag_id & 0xffff)<<16) | (val & 0xffff);
			ASSERT_EQ(re8686_set_vlan_register(&txInfo,svlan_index,val),0);
#endif
			rg_db.systemGlobal.tpid2 = svlan_tag_id;
		}
		else
		{
			ret = RT_ERR_CHIP_NOT_SUPPORTED;
		}
	}

	return ret;
}

int32 RTK_SVLAN_TPIDENABLE_SET(uint32 svlanIndex, rtk_enable_t enable)
{
	int ret;
#if defined(CONFIG_APOLLO_FPGA_PHY_TEST)  || defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9602C_SERIES) || defined(CONFIG_RG_G3_SERIES)
#else // gmac support tpid enable
	uint32 val;
#endif

	if(svlanIndex==0)
	{
		if(enable==DISABLED)	//tpid0 is always enabled in switch
			ret=RT_ERR_CHIP_NOT_SUPPORTED;
		else
		{
#if defined(CONFIG_APOLLO_FPGA_PHY_TEST) || defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9602C_SERIES) || defined(CONFIG_RG_G3_SERIES)
#else // gmac support tpid enable
			struct tx_info txInfo={{{0}},0,{{0}},{{0}}};
			ASSERT_EQ(re8686_get_vlan_register(&txInfo,svlanIndex,&val),0);
			val |= 0xc000;
			ASSERT_EQ(re8686_set_vlan_register(&txInfo,svlanIndex,val),0);
#endif
			ret=RT_ERR_OK;
		}
	}
	else if(svlanIndex==1)
	{
		ret=rtk_svlan_tpidEnable_set(svlanIndex, enable);
		if(ret==RT_ERR_OK)
		{
#if defined(CONFIG_APOLLO_FPGA_PHY_TEST)  || defined(CONFIG_RG_RTL9600_SERIES) || defined(CONFIG_RG_RTL9602C_SERIES) || defined(CONFIG_RG_G3_SERIES)
#else // gmac support tpid2 enable
			struct tx_info txInfo={{{0}},0,{{0}},{{0}}};
			ASSERT_EQ(re8686_get_vlan_register(&txInfo,svlanIndex,&val),0);
			if(enable==ENABLED)
				val |= 0xc000;
			else
				val &= ~(0xc000);
			ASSERT_EQ(re8686_set_vlan_register(&txInfo,svlanIndex,val),0);
#endif
			rg_db.systemGlobal.tpid2_en = enable;
		}
	}
	else
	{
		ret = RT_ERR_CHIP_NOT_SUPPORTED;
	}

	return ret;
}

int32 RTK_SVLAN_SERVICEPORT_SET(rtk_port_t port, rtk_enable_t enable)
{
	//DEBUG("\ndebug: RTK_SVLAN_SERVICEPORT_SET port=0x%x enable=%d\n", port, enable);
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
#if !defined(CONFIG_RG_G3_SERIES)
	ret=rtk_svlan_servicePort_set(port,enable);
#endif
	if(ret==RT_ERR_OK){
		rg_db.systemGlobal.service_pmsk.portmask &= (~(1<<port));
		if(enable==ENABLED)
		{
			rg_db.systemGlobal.service_pmsk.portmask|= (1<<port);
		}
	}
	return ret;
}

int32 RTK_SWITCH_MAXPKTLENBYPORT_SET(rtk_port_t port, int pktlen)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_switch_maxPktLenByPort_set(port, pktlen);
}

int32 RTK_SVLAN_SERVICEPORT_GET(rtk_port_t port, rtk_enable_t *pEnable)
{
	int ret=RT_ERR_OK;
	//DEBUG("\ndebug: RTK_SVLAN_SERVICEPORT_GET port=0x%x enable=%d\n", port, *pEnable);
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_svlan_servicePort_get(port, pEnable);
}

int32 RTK_SVLAN_DMACVIDSELSTATE_SET(rtk_port_t port, rtk_enable_t enable)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	ret=RT_ERR_OK;
#else
	ret=rtk_svlan_dmacVidSelState_set(port, enable);
#endif
	if(ret==RT_ERR_OK){
		rg_db.systemGlobal.svlan_EP_DMAC_CTRL_pmsk.portmask &= (~(1<<port));
		if(enable==ENABLED)
		{
			rg_db.systemGlobal.svlan_EP_DMAC_CTRL_pmsk.portmask|= (1<<port);
		}
	}
	return ret;
}

int32 RTK_SVLAN_MEMBERPORT_SET(rtk_vlan_t svid, rtk_portmask_t * pSvlanPortmask, rtk_portmask_t * pSvlanUntagPortmask)
{
	int ret=RT_ERR_OK;
	rtk_portmask_t portmask,portmask2;
	portmask.bits[0]=(pSvlanPortmask->bits[0])&rg_db.systemGlobal.phyPortStatus;
	portmask2.bits[0]=(pSvlanUntagPortmask->bits[0])&rg_db.systemGlobal.phyPortStatus;
#if !defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	ret=rtk_svlan_memberPort_set(svid, &portmask, &portmask2);
#endif
	return ret;
}

int32 RTK_SVLAN_PORTSVID_SET(rtk_port_t port, rtk_vlan_t svid)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
#if !defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	ret=rtk_svlan_portSvid_set(port,svid);
#endif
	return ret;
}

int32 RTK_SVLAN_FIDENABLE_SET(rtk_vlan_t svid, rtk_enable_t enable)
{
	return rtk_svlan_fidEnable_set(svid, enable);
}

int32 RTK_SVLAN_FID_SET(rtk_vlan_t svid, rtk_fid_t fid)
{
	return rtk_svlan_fid_set(svid, fid);
}

int32 RTK_SVLAN_UNTAGACTION_SET(rtk_svlan_action_t action, rtk_vlan_t svid)
{
#if !defined(CONFIG_RG_G3_SERIES)
	return rtk_svlan_untagAction_set(action,svid);
#else
	return RT_ERR_OK;
#endif
}

int32 RTK_ACL_IGRSTATE_SET(rtk_port_t port, rtk_enable_t state)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_acl_igrState_set(port, state);
}

int32 RTK_ACL_IGRUNMATCHACTION_SET(rtk_port_t port, rtk_filter_unmatch_action_type_t action)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_acl_igrUnmatchAction_set(port, action);
}

int32 RTK_TRAP_PORTIGMPMLDCTRLPKTACTION_SET(rtk_port_t port, rtk_trap_igmpMld_type_t igmpMldType, rtk_action_t action)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_trap_portIgmpMldCtrlPktAction_set(port, igmpMldType, action);
}

int32 RTK_PORT_ISOLATIONIPMCLEAKY_SET(rtk_port_t port, rtk_enable_t enable)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
#if !defined(CONFIG_RG_G3_SERIES)
	ret = rtk_port_isolationIpmcLeaky_set(port, enable);
#endif
	return ret;
}

int32 RTK_PORT_ISOLATIONENTRY_SET(rtk_port_isoConfig_t mode, rtk_port_t port, rtk_portmask_t * pPortmask, rtk_portmask_t * pExtPortmask)
{
	int ret=RT_ERR_OK;
	rtk_portmask_t portmask;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	portmask.bits[0]=(pPortmask->bits[0])&rg_db.systemGlobal.phyPortStatus;
	return rtk_port_isolationEntry_set(mode, port, &portmask, pExtPortmask);
}

int32 RTK_PORT_ISOLATIONENTRYEXT_SET(rtk_port_isoConfig_t mode, rtk_port_t port, rtk_portmask_t *pPortmask, rtk_portmask_t *pExtPortmask)
{
	int ret=RT_ERR_OK;
	rtk_portmask_t portmask;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	portmask.bits[0]=(pPortmask->bits[0])&rg_db.systemGlobal.phyPortStatus;
#if !defined(CONFIG_RG_G3_SERIES)
	ret=rtk_port_isolationEntryExt_set(mode, port, &portmask, pExtPortmask);
#endif
	return ret;
}

int32 RTK_PORT_MACFORCEABILITY_GET(rtk_port_t port, rtk_port_macAbility_t * pMacAbility)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_port_macForceAbility_get(port, pMacAbility);
}

int32 RTK_PORT_MACFORCEABILITY_SET(rtk_port_t port,rtk_port_macAbility_t macAbility)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_port_macForceAbility_set(port, macAbility);
}

int32 RTK_PORT_MACFORCEABILITYSTATE_SET(rtk_port_t port,rtk_enable_t state)
{
	int ret=RT_ERR_OK;
	if(_rtk_rg_checkPortNotExistByPhy(port))return ret;	//just return without changing anything
	return rtk_port_macForceAbilityState_set(port, state);
}

#if defined(CONFIG_RG_RTL9602C_SERIES)
int32 RTK_L34_DSLITEINFTABLE_SET(rtk_l34_dsliteInf_entry_t *pDsliteInfEntry)
{
	int ret;

	ret=rtk_l34_dsliteInfTable_set(pDsliteInfEntry);
	if(ret==RT_ERR_OK)
	{
		memcpy(&rg_db.dslite[pDsliteInfEntry->index].rtk_dslite,pDsliteInfEntry,sizeof(*pDsliteInfEntry));
	}
	return ret;
}

#endif

int32 RTK_L34_DSLITEMULTICAST_SET(rtk_l34_dsliteMc_entry_t *pDsliteMcEntry)
{
	int ret=RT_ERR_OK;
	if(pDsliteMcEntry->index >= MAX_DSLITEMC_SW_TABLE_SIZE)
		return RT_ERR_ENTRY_INDEX;

#if defined(CONFIG_RG_RTL9602C_SERIES)
	if(pDsliteMcEntry->index < MAX_DSLITEMC_HW_TABLE_SIZE)
		ret=rtk_l34_dsliteMcTable_set(pDsliteMcEntry);
#endif
	if(ret==RT_ERR_OK)
	{
		memcpy(&rg_db.dsliteMc[pDsliteMcEntry->index].rtk_dsliteMc,pDsliteMcEntry,sizeof(*pDsliteMcEntry));
	}
	return ret;
}

int32 RTK_SEC_PORTATTACKPREVENTSTATE_SET(rtk_port_t port, rtk_enable_t enable)
{
	int ret;
	ret=rtk_sec_portAttackPreventState_set(port, enable);
	if(ret==RT_ERR_OK)
	{
		if(enable==ENABLED)
			rg_db.systemGlobal.swDosPortMask.portmask|=(0x1<<port);
		else
			rg_db.systemGlobal.swDosPortMask.portmask&=(~(0x1<<port));
	}
	return ret;
}

int32 RTK_SEC_ATTACKPREVENT_GET(    rtk_sec_attackType_t         attackType, rtk_action_t *pAction)
{
	int ret=RT_ERR_OK;
#if defined(CONFIG_RG_G3_SERIES)
	switch(attackType)
	{
		case SYNFIN_DENY:
		case SYNFLOOD_DENY:
		case FINFLOOD_DENY:
		case ICMPFLOOD_DENY:
			ret=rtk_sec_attackPrevent_get(attackType, pAction);
			break;
		default:
			switch(rg_db.systemGlobal.swDosAction[attackType])
			{
				case RTK_RG_DOS_ACTION_DROP:
					*pAction=ACTION_DROP;
					break;
				case RTK_RG_DOS_ACTION_TRAP:
					*pAction=ACTION_TRAP2CPU;
					break;
				default:
					*pAction=ACTION_FORWARD;
					break;
			}
			break;
	}
#else
	ret=rtk_sec_attackPrevent_get(attackType, pAction);
#endif
	return ret;
}


int32 RTK_SEC_ATTACKPREVENT_SET(  rtk_sec_attackType_t attackType, rtk_action_t action)
{
	int ret=RT_ERR_OK;
#if defined(CONFIG_RG_G3_SERIES)
	switch(attackType)
	{
		case SYNFIN_DENY:
		case SYNFLOOD_DENY:
		case FINFLOOD_DENY:
		case ICMPFLOOD_DENY:
			ret=rtk_sec_attackPrevent_set(attackType, action);
			break;
		default:
			switch(action)
			{
				case ACTION_DROP:
					rg_db.systemGlobal.swDosAction[attackType]=RTK_RG_DOS_ACTION_DROP;
					break;
				case ACTION_TRAP2CPU:
					rg_db.systemGlobal.swDosAction[attackType]=RTK_RG_DOS_ACTION_TRAP;
					break;
				default:
					rg_db.systemGlobal.swDosAction[attackType]=RTK_RG_DOS_ACTION_FORWARD;
					break;
			}
			break;
	}
#else
	switch(attackType)
	{
		case SYNFLOOD_DENY:
		case FINFLOOD_DENY:
		case ICMPFLOOD_DENY:
			switch(action)
			{
				case ACTION_DROP:
					rg_db.systemGlobal.swDosAction[attackType]=RTK_RG_DOS_ACTION_DROP;
					break;
				case ACTION_TRAP2CPU:
					rg_db.systemGlobal.swDosAction[attackType]=RTK_RG_DOS_ACTION_TRAP;
					break;
				default:
					rg_db.systemGlobal.swDosAction[attackType]=RTK_RG_DOS_ACTION_FORWARD;
					break;
			}
			break;
		default:
			break;
	}
	ret=rtk_sec_attackPrevent_set(attackType, action);
#endif
	return ret;
}

int32 RTK_TRAP_CPUTRAPHASHPORT_SET(uint32 value, rtk_port_t port)
{
	int ret;
	ret=rtk_trap_cpuTrapHashPort_set(value, port);
#if defined(CONFIG_RG_FLOW_ENHANCED_WIFI_MODE)
	if(ret==RT_ERR_OK)
		rg_db.systemGlobal.cpuTrapHashPort[value]=port;
#endif
	return ret;
}

long COPY_FROM_USER(void *to, 		const void __user * from, unsigned long n)
{
	memcpy(to,from,n);
	return SUCCESS;
}
#if defined(CONFIG_RG_RTL9600_SERIES)
int RTK_RG_SOFTWARE_RESET(void)
{
	int i,j;
	uint32 lanPacketLen,wanPacketLen,regValue;
	rtk_classify_cfg_t classifyCfg_zero;

	//get switch accept packet size, set it to zero
	ASSERT_EQ(rtk_switch_maxPktLenByPort_get(RTK_RG_MAC_PORT0, &lanPacketLen),RT_ERR_OK);
	ASSERT_EQ(rtk_switch_maxPktLenByPort_get(RTK_RG_MAC_PORT_PON, &wanPacketLen),RT_ERR_OK);
	ASSERT_EQ(rtk_switch_maxPktLenByPort_set(RTK_RG_MAC_PORT0, 0),RT_ERR_OK);
	ASSERT_EQ(rtk_switch_maxPktLenByPort_set(RTK_RG_MAC_PORT_PON, 0),RT_ERR_OK);

	//keep register SWTCR0, V6_BD_CTL, BD_CFG
	ioal_mem32_read((uint32)(REG_NAT_CTRL),&rg_db.hw_REG_NAT_CTRL);
	ioal_mem32_read((uint32)(REG_V6_BD_CTRL),&rg_db.hw_REG_V6_BD_CTRL);
	ioal_mem32_read((uint32)(REG_BD_CFG),&rg_db.hw_REG_BD_CFG);

	//keep cf entry zero
	memset(&classifyCfg_zero,0,sizeof(classifyCfg_zero));
	classifyCfg_zero.index=0;
	ASSERT_EQ(rtk_classify_cfgEntry_get(&classifyCfg_zero),RT_ERR_OK);

	/*keep hwnat table
	1.	IPv6 routing table
	2.	interface table
	3.	external ip table
	4.	IPv4 routing table
	5.	pppoe table
	6.	wan type table
	7.	next hop table
	8.	binding table
	*/
	for(i=0;i<MAX_IPV6_ROUTING_HW_TABLE_SIZE;i++)
		ASSERT_EQ(rtk_l34_ipv6RoutingTable_get(i, &rg_db.hw_v6route[i]),RT_ERR_OK);
	for(i=0;i<MAX_NETIF_HW_TABLE_SIZE;i++)
		ASSERT_EQ(rtk_l34_netifTable_get(i, &rg_db.hw_netif[i]),RT_ERR_OK);
	for(i=0;i<MAX_EXTIP_HW_TABLE_SIZE;i++)
		ASSERT_EQ(rtk_l34_extIntIPTable_get(i, &rg_db.hw_extip[i]),RT_ERR_OK);
	for(i=0;i<MAX_L3_HW_TABLE_SIZE;i++)
		ASSERT_EQ(rtk_l34_routingTable_get(i, &rg_db.hw_l3[i]),RT_ERR_OK);
	for(i=0;i<MAX_PPPOE_HW_TABLE_SIZE;i++)
		ASSERT_EQ(rtk_l34_pppoeTable_get(i, &rg_db.hw_pppoe[i]),RT_ERR_OK);
	for(i=0;i<MAX_WANTYPE_HW_TABLE_SIZE;i++)
		ASSERT_EQ(rtk_l34_wanTypeTable_get(i, &rg_db.hw_wantype[i]),RT_ERR_OK);
	for(i=0;i<MAX_NEXTHOP_HW_TABLE_SIZE;i++)
		ASSERT_EQ(rtk_l34_nexthopTable_get(i, &rg_db.hw_nexthop[i]),RT_ERR_OK);
	for(i=0;i<MAX_BIND_HW_TABLE_SIZE;i++)
		ASSERT_EQ(rtk_l34_bindingTable_get(i, &rg_db.hw_bind[i]),RT_ERR_OK);

	//reset sw
	regValue = 0x1;
	ASSERT_EQ(reg_field_write(APOLLOMP_SOFTWARE_RSTr, APOLLOMP_SW_RSTf, &regValue),RT_ERR_OK);

	//recovery table
	for(i=0;i<MAX_IPV6_ROUTING_HW_TABLE_SIZE;i++)
		ASSERT_EQ(rtk_l34_ipv6RoutingTable_set(i, &rg_db.hw_v6route[i]),RT_ERR_OK);
	for(i=0;i<MAX_NETIF_HW_TABLE_SIZE;i++)
		ASSERT_EQ(rtk_l34_netifTable_set(i, &rg_db.hw_netif[i]),RT_ERR_OK);
	for(i=0;i<MAX_EXTIP_HW_TABLE_SIZE;i++)
		ASSERT_EQ(rtk_l34_extIntIPTable_set(i, &rg_db.hw_extip[i]),RT_ERR_OK);
	for(i=0;i<MAX_L3_HW_TABLE_SIZE;i++)
		ASSERT_EQ(rtk_l34_routingTable_set(i, &rg_db.hw_l3[i]),RT_ERR_OK);
	for(i=0;i<MAX_PPPOE_HW_TABLE_SIZE;i++)
		ASSERT_EQ(rtk_l34_pppoeTable_set(i, &rg_db.hw_pppoe[i]),RT_ERR_OK);
	for(i=0;i<MAX_WANTYPE_HW_TABLE_SIZE;i++)
		ASSERT_EQ(rtk_l34_wanTypeTable_set(i, &rg_db.hw_wantype[i]),RT_ERR_OK);
	for(i=0;i<MAX_NEXTHOP_HW_TABLE_SIZE;i++)
		ASSERT_EQ(rtk_l34_nexthopTable_set(i, &rg_db.hw_nexthop[i]),RT_ERR_OK);
	for(i=0;i<MAX_BIND_HW_TABLE_SIZE;i++)
		ASSERT_EQ(rtk_l34_bindingTable_set(i, &rg_db.hw_bind[i]),RT_ERR_OK);
	for(i=0;i<MAX_LUT_HW_TABLE_SIZE;i++)
	{
		if(rg_db.lut[i].valid)
		{
			switch(rg_db.lut[i].rtk_lut.entryType)
			{
				case RTK_LUT_L2UC:
					ASSERT_EQ(rtk_l2_addr_add(&rg_db.lut[i].rtk_lut.entry.l2UcEntry),RT_ERR_OK);
					break;
				case RTK_LUT_L2MC:
					ASSERT_EQ(rtk_l2_mcastAddr_add(&rg_db.lut[i].rtk_lut.entry.l2McEntry),RT_ERR_OK);
					break;
				case RTK_LUT_L3MC:
				case RTK_LUT_L3V6MC:
					ASSERT_EQ(rtk_l2_ipMcastAddr_add(&rg_db.lut[i].rtk_lut.entry.ipmcEntry),RT_ERR_OK);
					break;
				default:
					break;
			}
		}
	}

	//recovery l34 global register
	ioal_mem32_write((uint32)(REG_NAT_CTRL),rg_db.hw_REG_NAT_CTRL);
	ioal_mem32_write((uint32)(REG_V6_BD_CTRL),rg_db.hw_REG_V6_BD_CTRL);
	ioal_mem32_write((uint32)(REG_BD_CFG),rg_db.hw_REG_BD_CFG);

	//keep cf entry zero
	for(i=0;i<HAL_CLASSIFY_ENTRY_MAX();i++)
	{
		rtk_classify_cfg_t classifyCfg;
		memset(&classifyCfg,0,sizeof(rtk_classify_cfg_t));
		if(i==0)
		{
			memcpy(&classifyCfg,&classifyCfg_zero,sizeof(rtk_classify_cfg_t));
		}
		else
		{
			classifyCfg.index=i;
			ASSERT_EQ(rtk_classify_cfgEntry_get(&classifyCfg),RT_ERR_OK);
		}
		if(classifyCfg.valid)
		{
			bzero(rg_db.systemGlobal.classifyField_1, CLASSIFY_FIELD_END*sizeof(rtk_classify_field_t));

			if(i<64){//L34 patterns
				//parse PortRange
				if((classifyCfg.field.readField.dataFieldRaw[2]>>12)&0x8)
				{
					ACL("\tPORT: must hit TABLE[%d]\n",(classifyCfg.field.readField.dataFieldRaw[2]>>12)&0x7);
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_PORT_RANGE].fieldType = CLASSIFY_FIELD_PORT_RANGE;
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_PORT_RANGE].classify_pattern.fieldData.value = ((1<<3) | ((classifyCfg.field.readField.dataFieldRaw[2]>>12)&0x7)); //(1<<3) is the valid bit
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_PORT_RANGE].classify_pattern.fieldData.mask=0xf;
				}

				//parse IpRange
				if((classifyCfg.field.readField.dataFieldRaw[2]>>8)&0x8)
				{
					ACL("\tIP: must hit TABLE[%d]\n",(classifyCfg.field.readField.dataFieldRaw[2]>>8)&0x7);
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IP_RANGE].fieldType =CLASSIFY_FIELD_IP_RANGE;
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IP_RANGE].classify_pattern.fieldData.value=((1<<3) | ((classifyCfg.field.readField.dataFieldRaw[2]>>8)&0x7)); //(1<<3) is the valid bit
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IP_RANGE].classify_pattern.fieldData.mask=0xf;
				}

				//parse Acl Hit
				if((classifyCfg.field.readField.dataFieldRaw[2]>>0)&0x80)
				{
					ACL("\tACL: must hit ACL[%d]\n",(classifyCfg.field.readField.dataFieldRaw[2]>>0)&0x7f);
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_ACL_HIT].fieldType = CLASSIFY_FIELD_ACL_HIT;
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_ACL_HIT].classify_pattern.fieldData.value = ((1<<7) | ((classifyCfg.field.readField.dataFieldRaw[2]>>0)&0x7f)); //(1<<7) is the valid bit
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_ACL_HIT].classify_pattern.fieldData.mask=0xff;
				}

				//parse Wan Interface
				ACL("\tWAN_IF:%d \t(Mask:0x%x)\n",(classifyCfg.field.readField.dataFieldRaw[1]>>12)&0x7,(classifyCfg.field.readField.careFieldRaw[1]>>12)&0x7);
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_WAN_IF].fieldType =CLASSIFY_FIELD_WAN_IF;
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_WAN_IF].classify_pattern.fieldData.value=(classifyCfg.field.readField.dataFieldRaw[1]>>12)&0x7;
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_WAN_IF].classify_pattern.fieldData.mask=(classifyCfg.field.readField.careFieldRaw[1]>>12)&0x7;

				//parse IPv6_MC
				if((classifyCfg.field.readField.careFieldRaw[1]>>11)&0x1)
				{
					ACL("\tIPv6_MC:%s\n", ((classifyCfg.field.readField.dataFieldRaw[1]>>11)&0x1)?"Must":"Must Not");
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IP6_MC].fieldType =CLASSIFY_FIELD_IP6_MC;
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IP6_MC].classify_pattern.fieldData.value=((classifyCfg.field.readField.dataFieldRaw[1]>>11)&0x1);
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IP6_MC].classify_pattern.fieldData.mask=0x1;
				}
				//parse IPv4_MC
				if( (classifyCfg.field.readField.careFieldRaw[1]>>10)&0x1)
				{
					ACL("\tIPv4_MC:%s\n", ((classifyCfg.field.readField.dataFieldRaw[1]>>10)&0x1)?"Must":"Must Not");
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IP4_MC].fieldType =CLASSIFY_FIELD_IP4_MC;
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IP4_MC].classify_pattern.fieldData.value=((classifyCfg.field.readField.dataFieldRaw[1]>>10)&0x1);
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IP4_MC].classify_pattern.fieldData.mask=0x1;
				}
				//parse MLD
				if( (classifyCfg.field.readField.careFieldRaw[1]>>9)&0x1)
				{
					ACL("\tMLD:%s\n", ((classifyCfg.field.readField.dataFieldRaw[1]>>9)&0x1)?"Must":"Must Not");
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_MLD].fieldType =CLASSIFY_FIELD_MLD;
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_MLD].classify_pattern.fieldData.value=((classifyCfg.field.readField.dataFieldRaw[1]>>9)&0x1);
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_MLD].classify_pattern.fieldData.mask=0x1;
				}
				//parse IGMP
				if((classifyCfg.field.readField.careFieldRaw[1]>>8)&0x1)
				{
					ACL("\tIGMP:%s\n", ((classifyCfg.field.readField.dataFieldRaw[1]>>8)&0x1)?"Must":"Must Not");
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IGMP].fieldType =CLASSIFY_FIELD_IGMP;
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IGMP].classify_pattern.fieldData.value=((classifyCfg.field.readField.dataFieldRaw[1]>>8)&0x1);
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IGMP].classify_pattern.fieldData.mask=0x1;
				}
				//parse DEI
				if((classifyCfg.field.readField.careFieldRaw[1]>>7)&0x1)
				{
					ACL("\tDEI:%d \t(Mask:0x%x)\n",(classifyCfg.field.readField.dataFieldRaw[1]>>7)&0x1,(classifyCfg.field.readField.careFieldRaw[1]>>7)&0x1);
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IGMP].fieldType =CLASSIFY_FIELD_IGMP;
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IGMP].classify_pattern.fieldData.value=(classifyCfg.field.readField.dataFieldRaw[1]>>7)&0x1;
					rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IGMP].classify_pattern.fieldData.mask=(classifyCfg.field.readField.careFieldRaw[1]>>7)&0x1;
				}
			}else{//L2 patterns
				//parse Ethertype
				if(classifyCfg.field.readField.careFieldRaw[2]){
					ACL("\tEhtertype:0x%x \t(Mask:0x%x)\n",classifyCfg.field.readField.dataFieldRaw[2],classifyCfg.field.readField.careFieldRaw[2]);
					rg_db.systemGlobal.classifyField_1[CF_PATTERN_ETHERTYPE].fieldType=CLASSIFY_FIELD_ETHERTYPE;
					rg_db.systemGlobal.classifyField_1[CF_PATTERN_ETHERTYPE].classify_pattern.fieldData.value=classifyCfg.field.readField.dataFieldRaw[2];
					rg_db.systemGlobal.classifyField_1[CF_PATTERN_ETHERTYPE].classify_pattern.fieldData.mask=classifyCfg.field.readField.careFieldRaw[2];
				}
				//parse GemIdx or LLID
				if((classifyCfg.field.readField.careFieldRaw[1]>>7)&0xff){
					ACL("\tGemIdx/LLID/TOS:0x%x \t(Mask:0x%x)\n",(classifyCfg.field.readField.dataFieldRaw[1]>>7)&0xff,(classifyCfg.field.readField.careFieldRaw[1]>>7)&0xff);
					rg_db.systemGlobal.classifyField_1[CF_PATTERN_GEMIDX_OR_LLID].fieldType=CLASSIFY_FIELD_TOS_DSIDX;
					rg_db.systemGlobal.classifyField_1[CF_PATTERN_GEMIDX_OR_LLID].classify_pattern.fieldData.value=(classifyCfg.field.readField.dataFieldRaw[1]>>7)&0xff;
					rg_db.systemGlobal.classifyField_1[CF_PATTERN_GEMIDX_OR_LLID].classify_pattern.fieldData.mask=(classifyCfg.field.readField.careFieldRaw[1]>>7)&0xff;
				}
			}
			//parse VID
			if((((classifyCfg.field.readField.careFieldRaw[1]>>0)&0x7f)<<5) | ((classifyCfg.field.readField.careFieldRaw[0])>>11 &0x1f))
			{
				ACL("\tVID:%d \t(Mask:0x%x)\n",(((classifyCfg.field.readField.dataFieldRaw[1]>>0)&0x7f)<<5) | ((classifyCfg.field.readField.dataFieldRaw[0])>>11 &0x1f),(((classifyCfg.field.readField.careFieldRaw[1]>>0)&0x7f)<<5) | ((classifyCfg.field.readField.careFieldRaw[0])>>11 &0x1f));
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_TAG_VID].fieldType =CLASSIFY_FIELD_TAG_VID;
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_TAG_VID].classify_pattern.fieldData.value=(((classifyCfg.field.readField.dataFieldRaw[1]>>0)&0x7f)<<5) | ((classifyCfg.field.readField.dataFieldRaw[0])>>11 &0x1f);
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_TAG_VID].classify_pattern.fieldData.mask=(((classifyCfg.field.readField.careFieldRaw[1]>>0)&0x7f)<<5) | ((classifyCfg.field.readField.careFieldRaw[0])>>11 &0x1f);
			}
			//parse PRI
			if((classifyCfg.field.readField.careFieldRaw[0]>>8)&0x7)
			{
				ACL("\tPRI:%d \t\t(Mask:0x%x)\n",(classifyCfg.field.readField.dataFieldRaw[0]>>8)&0x7,(classifyCfg.field.readField.careFieldRaw[0]>>8)&0x7);
				//must be ctagged
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IS_CTAG].fieldType =CLASSIFY_FIELD_IS_CTAG;
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IS_CTAG].classify_pattern.fieldData.value=1;
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IS_CTAG].classify_pattern.fieldData.mask=0x1;

				//egress_ctag_pri will be taggged pri;
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_TAG_PRI].fieldType =CLASSIFY_FIELD_TAG_PRI;
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_TAG_PRI].classify_pattern.fieldData.value=(classifyCfg.field.readField.dataFieldRaw[0]>>8)&0x7;
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_TAG_PRI].classify_pattern.fieldData.mask=(classifyCfg.field.readField.careFieldRaw[0]>>8)&0x7;
			}
			//parse INTER_PRI
			if((classifyCfg.field.readField.careFieldRaw[0]>>5)&0x7)
			{
				ACL("\tINTER_PRI:%d \t(Mask:0x%x)\n",(classifyCfg.field.readField.dataFieldRaw[0]>>5)&0x7,(classifyCfg.field.readField.careFieldRaw[0]>>5)&0x7);
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_INTER_PRI].fieldType =CLASSIFY_FIELD_INTER_PRI;
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_INTER_PRI].classify_pattern.fieldData.value=(classifyCfg.field.readField.dataFieldRaw[0]>>5)&0x7;
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_INTER_PRI].classify_pattern.fieldData.mask=(classifyCfg.field.readField.careFieldRaw[0]>>5)&0x7;
			}
			//parse STAG, CTAG
			if((classifyCfg.field.readField.careFieldRaw[0]>>4)&0x1)
			{
				ACL("\tSTAG_IF:%s \n",((classifyCfg.field.readField.dataFieldRaw[0]>>4)&0x1)?"Must Stagged":"Must don't have Stag");
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IS_STAG].fieldType =CLASSIFY_FIELD_IS_STAG;
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IS_STAG].classify_pattern.fieldData.value=((classifyCfg.field.readField.dataFieldRaw[0]>>4)&0x1);
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IS_STAG].classify_pattern.fieldData.mask=0x1;
			}
			if((classifyCfg.field.readField.careFieldRaw[0]>>3)&0x1)
			{
				ACL("\tCTAG_IF:%s \n",((classifyCfg.field.readField.dataFieldRaw[0]>>3)&0x1)?"Must Ctagged":"Must don't have Ctag");
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IS_CTAG].fieldType =CLASSIFY_FIELD_IS_CTAG;
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IS_CTAG].classify_pattern.fieldData.value=((classifyCfg.field.readField.dataFieldRaw[0]>>3)&0x1);
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_IS_CTAG].classify_pattern.fieldData.mask=0x1;
			}
			//parse UNI
			if(classifyCfg.field.readField.careFieldRaw[0]&0x7)
			{
				ACL("\tUNI:%d \t\t(Mask:0x%x)\n",classifyCfg.field.readField.dataFieldRaw[0]&0x7,classifyCfg.field.readField.careFieldRaw[0]&0x7);
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_UNI].fieldType =CLASSIFY_FIELD_UNI;
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_UNI].classify_pattern.fieldData.value=classifyCfg.field.readField.dataFieldRaw[0]&0x7;
				rg_db.systemGlobal.classifyField_1[CLASSIFY_FIELD_UNI].classify_pattern.fieldData.mask=classifyCfg.field.readField.careFieldRaw[0]&0x7;
			}

			classifyCfg.field.pFieldHead=NULL;
			for(j=0; j<CLASSIFY_FIELD_END; j++)
			{
				if(memcmp(&rg_db.systemGlobal.classifyField_1[j],&rg_db.systemGlobal.empty_classifyField,sizeof(rtk_classify_field_t)))
				{
					//if(i == CLASSIFY_FIELD_ACL_HIT)continue;
					ACL("adding field[%d] to rg_db.systemGlobal.cfRule_1",j);
					ASSERT_EQ(rtk_classify_field_add(&classifyCfg, &rg_db.systemGlobal.classifyField_1[j]),RT_ERR_OK);
				}
			}

			ASSERT_EQ(rtk_classify_cfgEntry_add(&classifyCfg),RT_ERR_OK);
		}
	}

	//recovery switch accept packet size settings
	ASSERT_EQ(rtk_switch_maxPktLenByPort_set(RTK_RG_MAC_PORT0, lanPacketLen),RT_ERR_OK);
	ASSERT_EQ(rtk_switch_maxPktLenByPort_set(RTK_RG_MAC_PORT_PON, wanPacketLen),RT_ERR_OK);

	return 0;
}
#endif
