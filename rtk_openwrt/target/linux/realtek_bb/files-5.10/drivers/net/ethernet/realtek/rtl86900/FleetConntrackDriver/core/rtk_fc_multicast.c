		/*
 * Copyright (C) 2017 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*/


#include "rtk_fc_acl.h"
#include "rtk_fc_internal.h"
#include "rtk_fc_callback.h"
#include "rtk_fc_debug.h"
#include "rtk_fc_multicast.h"
#include "rtk_fc_mappingAPI.h"
#include "rtk_fc_external.h"

#if defined(CONFIG_RTK_L34_G3_PLATFORM)
#include "aal_l3_specpkt.h"
#include "aal_common.h"
#endif


#include <rtk/acl.h>
#include <rtk/l2.h>

#include <net/mld.h>
#include <uapi/linux/igmp.h>
#include <linux/if_bridge.h>
#include <br_private.h>


#define TOOL

#if  defined(CONFIG_RTK_L34_G3_PLATFORM)
int32 rtk_fc_l3fe_specialPktToCPU(aal_l3_specpkt_type_t l3_pkt_type,int spid)
{
    aal_l3_specpkt_ctrl_t       l3_ctrl;
    aal_l3_specpkt_ctrl_mask_t  l3_ctrl_mask;
    aal_l3fe_lpb_tbl_cfg_t      lpb_tbl_cfg;
    ca_uint8_t     lpb_profile = 0;
    L3FE_STG0_CTRL_t stg0_ctrl;
    l3_stg0_ldpid_map_t stg0_ldpid_map;
	int ret;
	int forward_to_cpu=true;

	/* get lpb_idx_mode for correct source of lpb_profile */

	/* get lpb_idx_mode for correct source of lpb_profile */
	if ((ret = aal_l3fe_stg0_ctrl_get(G3_DEF_DEVID, &stg0_ctrl)) != AAL_E_OK) {
		printk("%s: Fail to get STG0 ctrl. (ret=%d)\n", __func__, ret);
		return ret;
	}


	if (stg0_ctrl.bf.lpb_idx_mode == 0)
	{
		/* get lpb_profile from L3FE_STG0_LDPID_MAP */
		if ((ret = aal_l3_stg0_ldpid_map_get(G3_DEF_DEVID, &stg0_ldpid_map)) != CA_E_OK) {
			printk("%s: Fail to get STG0 LDPID map. (ret=%d)\n", __func__, ret);
			return ret;
		}
		if (spid == stg0_ldpid_map.rule_0)
			lpb_profile = 0;
		else if (spid == stg0_ldpid_map.rule_1)
			lpb_profile = 1;
		else if (spid == stg0_ldpid_map.rule_2)
			lpb_profile = 2;
		else
			lpb_profile = 3;

	}
	else 
	{
		/* get lpb_profile from L3FE_STG0_LPB_IDX_TBL */
		ret = aal_l3fe_lpb_idx_tbl_get(G3_DEF_DEVID, spid, &lpb_profile);
		if (CA_E_OK != ret) {
			printk("Invoke aal_l3fe_lpb_idx_tbl_get return %#x.\n", ret);
			return ret;
		}
	}

	l3_ctrl_mask.s.ldpid   = 1;
	//l3_ctrl_mask.s.cos	   = 0;
	//l3_ctrl_mask.s.pol_en_sel = 0;
	//l3_ctrl_mask.s.pol_id  = 0;
	l3_ctrl_mask.s.spcl_en = 1;

	memset(&l3_ctrl, 0, sizeof(aal_l3_specpkt_ctrl_t));
	l3_ctrl.ldpid = RTK_FC_MAC_PORT_CPU;


    if (forward_to_cpu)
        l3_ctrl.spcl_en = 1;

    if (AAL_L3_SPECPKT_TYPE_L2_PTP == l3_pkt_type || AAL_L3_SPECPKT_TYPE_L4_PTP == l3_pkt_type)
	{
        l3_ctrl_mask.s.keep_ts = 1;
        l3_ctrl.keep_ts = 1;
    }

    memset(&lpb_tbl_cfg, 0, sizeof(lpb_tbl_cfg));
    ret = aal_l3_stg0_lpb_tbl_get(G3_DEF_DEVID, lpb_profile, &lpb_tbl_cfg);
    if (CA_E_OK != ret) 
	{
        printk("Invoke aal_l3_stg0_lpb_tbl_get return %#x.\n", ret);
        return ret;
    }
    ret = aal_l3_specpkt_ctrl_set(G3_DEF_DEVID, lpb_tbl_cfg.spcl_pkt_sel,
        l3_pkt_type, l3_ctrl_mask, &l3_ctrl);


	return SUCCESS;

}

#endif

int _rtk_fc_igmp_mld_module_init(void)
{
	/* Mc flow pool*/
	rtk_fc_igmp_mcExtraFlowIdxTbl_flashInit();
	INIT_LIST_HEAD(&fc_db.listHead_mcCfgTbl);
	INIT_LIST_HEAD(&fc_db.listHead_mcCfgTbGrplDft);

	fc_db.igmpDummyPktDetectorTimer.igmpTimer = RTK_FC_HELPER_MGR_TIMER_LIST_KMALLOC();
	fc_db.igmpDummyPktDetectorTimer.nextTimerPeriod = RTK_FC_IGMP_DUMYPKT_TICK_MSECONDMS;
	fc_db.igmpKernelSyncTimerEvent.igmpTimer = RTK_FC_HELPER_MGR_TIMER_LIST_KMALLOC();
	fc_db.igmpKernelSyncTimerEvent.nextTimerPeriod = RTK_FC_IGMP_KERNEL_SHORT_TICK_MSECONDMS;
	fc_db.igmpKernelSyncTimerPeriod.igmpTimer = RTK_FC_HELPER_MGR_TIMER_LIST_KMALLOC();
	fc_db.igmpKernelSyncTimerPeriod.nextTimerPeriod = RTK_FC_IGMP_KERNEL_LONG_TICK_MSECONDMS;

	/*HW behavior*/
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	ASSERT_EQ(rtk_l2_ipmcMode_set(LOOKUP_ON_MAC_AND_VID_FID), SUCCESS);
	ASSERT_EQ(rtk_l2_ipmcVlanMode_set(LOOKUP_VLAN_BY_IVL_SVL), SUCCESS);

	ASSERT_EQ(rtk_l2_lookupMissAction_set(DLF_TYPE_ICMP6MC, ACTION_TRAP2CPU),RT_ERR_RG_OK);
	ASSERT_EQ(rtk_l2_lookupMissAction_set(DLF_TYPE_DHCP6MC, ACTION_TRAP2CPU),RT_ERR_RG_OK);
	/* 
	   disable reserved multicast to cpu 
		ipv6 control by acl !ffxe
		ipv4 control by igmpSnooping(lut) or upper layer acl rule
	*/
	//ASSERT_EQ(rtk_l2_lookupMissAction_set(DLF_TYPE_RSVIPMC, ACTION_TRAP2CPU),RT_ERR_RG_OK);
	//ASSERT_EQ(rtk_l2_lookupMissAction_set(DLF_TYPE_RSVIP6MC, ACTION_TRAP2CPU),RT_ERR_RG_OK);
	//ASSERT_EQ(rtk_l2_ip6mcReservedAddrEnable_set(LUT_IP6MCADDR_RSVDOC_FF01,1),RT_ERR_RG_OK);
	//ASSERT_EQ(rtk_l2_ip6mcReservedAddrEnable_set(LUT_IP6MCADDR_RSVDOC_FF02,1),RT_ERR_RG_OK);
	//ASSERT_EQ(rtk_l2_ip6mcReservedAddrEnable_set(LUT_IP6MCADDR_RSVDOC_FF05,1),RT_ERR_RG_OK);
	//ASSERT_EQ(rtk_l2_ip6mcReservedAddrEnable_set(LUT_IP6MCADDR_RSVDOC_FF0E,1),RT_ERR_RG_OK);
	ASSERT_EQ(rtk_l2_ip6mcReservedAddrEnable_set(LUT_IP6MCADDR_SOILCITED_NOTE,1),RT_ERR_RG_OK);

	assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP, NULL));

	rtk_l2_lookupMissAction_set(DLF_TYPE_IPMC, ACTION_DROP);
	rtk_l2_lookupMissAction_set(DLF_TYPE_IP6MC,ACTION_DROP);
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)


#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	rtk_rg_flow_default_action_update(RTK_ASIC_FLOW_PROFILE_FLOW_MC,RTK_ASIC_FLOW_DEFACT_TYPE_DROP);
	{
		// for wlan to wlan  ( wifiRx -> nicTx -> hwlookup -> nicRx -> wifiFFTx) packet will drop by hwlookup portISO(dpid not in member)
		// l2 drop reason dpid_not_in_port_mem_drp_17
		ca_uint64_t pmskAll=0xffffffffffffffff;
		int8 cpuPort;
		for(cpuPort=RTK_FC_MAC_PORT_CPU ;cpuPort<=RTK_FC_MAC_PORT_WLAN_CPU5;cpuPort++)
			aal_port_mmshp_check_set(G3_DEF_DEVID,cpuPort,pmskAll);
	}
#else
	aal_default_action_set(G3_DEF_DEVID, RG_CA_FLOW_MC, FALSE);
#endif

	assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP, NULL));
	assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_MULTICAST_RIP_TRAP, NULL));
	//DHCPv6, ICMPv6, IPv6 NDP, IPv6 MLD will trap by reserve acl	
#endif

#if defined(CONFIG_FC_RTL8198F_SERIES)
	RTK_FC_HELPER_8367R_REALY_IGMP_PROTOCOL_INIT();
#endif 


	return SUCCESS;

}

/*
	This function is not under lock.
	if get mc_config 
		return userconf 
	else
		return NULL;
*/
rtk_fc_table_mcConfigTbl_t* rtk_fc_egress_mcConfigGetByPkt(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr,rtk_fc_table_mcConfigTbl_t* userconf)
{
	rtk_fc_table_mcConfigTbl_t *p_userconf=NULL;
	rtk_fc_table_mcConfigTbl_t *p_fcdb_userconf=NULL;
	int i;
	char _isIpv6=0;
	uint32 groupIp[4]={0};
	uint32 sourceIp[4]={0};
	short igrCVlan,igrSVlan;
	rtk_fc_table_mcGroupTbl_t* pGroupTbl=NULL;

	if((RTSKB_FCIGRDATA(rtskb)->ingressTagif & CVLAN_TAGIF))
		igrCVlan=RTSKB_FCIGRDATA(rtskb)->srcCVlanId;
	else	
		igrCVlan=FAIL;

	if((RTSKB_FCIGRDATA(rtskb)->ingressTagif & SVLAN_TAGIF))
		igrSVlan=RTSKB_FCIGRDATA(rtskb)->srcSVlanId;
	else	
		igrSVlan=FAIL;


	if(pPktHdr->ip6h)
	{
		_isIpv6=1;
		memcpy(groupIp,pPktHdr->ip6h->daddr.s6_addr,sizeof(groupIp));
		memcpy(sourceIp,pPktHdr->ip6h->saddr.s6_addr,sizeof(sourceIp));
	}
	else if (pPktHdr->iph)
	{
		_isIpv6=0;
		groupIp[0]=ntohl(pPktHdr->iph->daddr);
		sourceIp[0]=ntohl(pPktHdr->iph->saddr);
	}


	if(RTK_FC_HELPER_IGMP_IS_MDB_REG())
	{
		if(RTK_FC_HELPER_IGMP_MDB_SEARCH(_isIpv6,groupIp,sourceIp,igrCVlan,igrSVlan,userconf)==SUCCESS)
			p_userconf = userconf;
		if(p_userconf) {
			for(i=0;i<p_userconf->cnt_egrInfo;i++)
			{
				if(p_userconf->egrInfo[i].dmacTrans)
					IGMP("getOutSideMdb IFIDX:%d egrssPort:%d dmacTransTo %pM",p_userconf->egrInfo[i].ifidx,p_userconf->egrInfo[i].portId,p_userconf->egrInfo[i].dmac);
				else
					IGMP("getOutSideMdb IFIDX:%d egrssPort:%d",p_userconf->egrInfo[i].ifidx,p_userconf->egrInfo[i].portId);
			}
		}
		pGroupTbl = rtk_fc_mc_findKnownGroupEntry(_isIpv6,groupIp);
		if(pGroupTbl==NULL)
		{
			IGMP("Group not ready do not add hardware");
			return NULL;
		}
	}
	else
	{

		//protect for mcConfig linkList
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH_IRQ_PROTECT();
		
		//check user mode config port
		p_fcdb_userconf=rtk_fc_mc_searchMcConfigByPatten(_isIpv6,groupIp,sourceIp,igrCVlan,igrSVlan);
		if(p_fcdb_userconf==NULL )
		{
			p_fcdb_userconf = rtk_fc_mc_searchDftMcConfigByPktPatten(_isIpv6,groupIp,sourceIp,igrCVlan,igrSVlan);
		}

		if(p_fcdb_userconf)
		{
			p_userconf = userconf;
			memcpy(p_userconf,p_fcdb_userconf,sizeof(*p_userconf));
		}
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH_IRQ_PROTECT();
		
	}

	return p_userconf;

}


int rtk_fc_egress_mcCheckPortInConfig(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr,rtk_fc_table_mcConfigTbl_t *p_userconf)
{
	int isWlan=0,ldpid=FAIL;
	int i;
	struct net_device *p_PScacheDev[DEV_STACK_MAX]={NULL,NULL,NULL,NULL};
	int filterStackingCnt=0;

	if(p_userconf==NULL)
	{
		IGMP("can't find McConfig do not add hardware");
		return FALSE;
	}

	pPktHdr->mcIsGrpDftFwd = p_userconf->dftGrpFwd;
	pPktHdr->abstrFlowCopy2cpu=p_userconf->copy2cpu;

	if(pPktHdr->egrWlanDevIdx < RTK_FC_WLANX_END_INTF) 
	{
		isWlan = 1;
		ldpid = pPktHdr->egrWlanDevIdx;
	}
	else 
	{
		ldpid = pPktHdr->egressPort.macPortIdx;
	}

	for(i=0 ; i<DEV_STACK_MAX ;i++)
	{
		p_PScacheDev[i] = rtk_fc_getDev_by_fcDevIdx(pPktHdr->logicalOutDevIfidx[i]);

		if(p_PScacheDev[i])
			filterStackingCnt++;
	}

	if(filterStackingCnt)
		IGMP("pPktHdr->egrWlanDevIdx=%d isWlan=%d ldpid=%d logicalOutDevIfidx=%d",pPktHdr->egrWlanDevIdx,isWlan,ldpid,pPktHdr->logicalOutDevIfidx[filterStackingCnt-1]);
	else
		IGMP("pPktHdr->logicalOutDevIfidx NULL");

	for(i=0 ; i<p_userconf->cnt_egrInfo ; i++ )
	{
		if(p_userconf->egrInfo[i].ifidx!=FAIL && filterStackingCnt)
		{
			if((p_userconf->egrInfo[i].ifidx == p_PScacheDev[filterStackingCnt-1]->ifindex))
			{
				IGMP("hit ifidx:%d",p_PScacheDev[filterStackingCnt-1]->ifindex);
				pPktHdr->mcPortInConfig=TRUE;
				return TRUE;
			}
		}
		else
		{
			if(p_userconf->egrInfo[i].isWlan == isWlan && p_userconf->egrInfo[i].portId==ldpid)
			{
				IGMP("check mc config hit isWlan:%d ldpid:%d",isWlan,ldpid);
				pPktHdr->mcPortInConfig=TRUE;
				return TRUE;
			}
		}
	}
	IGMP("can't Match McConfig do not add hardware egress %s Port=%d",isWlan?"WLAN":"PHY",ldpid);
	pPktHdr->mcPortInConfig=FALSE;
	return FALSE;

}


uint32 _rtk_fc_mcflowHash(uint32 groupip_LSB_32)
{
	return (groupip_LSB_32 & ((1<<RTK_FC_TABLESIZE_MCFLOW_HASH_SHIFTBITS)-1));
}


int rtk_fc_igmp_mcExtraFlowIdxTbl_flashInit(void)
{
	int i;

	for(i=0;i<RTK_FC_TABLESIZE_MCFLOW_HASH ;i++)
	{
		INIT_LIST_HEAD(&fc_db.listHead_mcExtraFlowIdxHashTbl[i]);
	}
	INIT_LIST_HEAD(&fc_db.listFree_mcExtraFlowIdxHashTbl);

	for(i=0 ; i<RTK_FC_TABLESIZE_MCFLOW;i++ )
	{
		memset(&fc_db.mcExtraFlowIdxTbl[i],0,sizeof(rtk_fc_mcExtraSwFlowIdx_entry_t));
		//fc_db.mcExtraFlowIdxTbl[i].entryIdx = i;
		INIT_LIST_HEAD(&fc_db.mcExtraFlowIdxTbl[i].entry);
		list_add_tail(&fc_db.mcExtraFlowIdxTbl[i].entry, &fc_db.listFree_mcExtraFlowIdxHashTbl);		
	}

	return (RT_ERR_RG_OK);
}


rtk_fc_mcExtraSwFlowIdx_entry_t * rtk_fc_igmp_mcExtraFlowIdxTbl_add(rtk_fc_ingress_data_t *pFcIngressData,rtk_fc_pktHdr_t *pPktHdr,struct rt_skbuff *rtskb,int32 swFlowIdx,uint8 isGrpDftFwd)
{
	rtk_fc_mcExtraSwFlowIdx_entry_t *mcSwFlowId_entry=NULL;
	rtk_fc_mcExtraSwFlowIdx_entry_t *mcSwFlowId_entry_tmp=NULL;
	uint32 mcflowHash=0;
	int32 mcflowIdxDuplicate=0;
	//uint32 tmp_entryIdx=0;
	mcflowHash =_rtk_fc_mcflowHash(pFcIngressData->dstIp);

	list_for_each_entry(mcSwFlowId_entry,&fc_db.listHead_mcExtraFlowIdxHashTbl[mcflowHash],entry)
	{
		if(mcSwFlowId_entry->swFlowIdx==swFlowIdx)
		{
			mcflowIdxDuplicate=1;
			break;
		}
	}

	if(mcflowIdxDuplicate)
	{
		IGMP("mcflowIdx Duplicate at mcflowHash=%d swFlowIdx=%d update mcflowEntry",mcflowHash,swFlowIdx);
	}
	else
	{
		if(list_empty(&fc_db.listFree_mcExtraFlowIdxHashTbl))
		{
			uint32 maxIdleSecs=0;
			uint32 maxIdle_mcFlowTbl_idx=0;	
			bool find = FALSE;
			uint32 i;
			//do lru

			for (i =0 ; i< RTK_FC_TABLESIZE_MCFLOW_HASH; i++)
			{
				list_for_each_entry(mcSwFlowId_entry,&fc_db.listHead_mcExtraFlowIdxHashTbl[i],entry)
				{
					if(fc_db.flowTbl[mcSwFlowId_entry->swFlowIdx].idleSecs > maxIdleSecs)
					{
						maxIdleSecs=fc_db.flowTbl[mcSwFlowId_entry->swFlowIdx].idleSecs;
						maxIdle_mcFlowTbl_idx=mcSwFlowId_entry->swFlowIdx;
						find = TRUE;
					}
				}
			}

			if(find && (maxIdle_mcFlowTbl_idx<RTK_FC_TABLESIZE_MCFLOW))
			{
				//delete a maxIdleSecs Entry
				IGMP("LRU delete a maxIdleSecs Entry");
				rtk_fc_igmp_mcExtraFlowIdxTbl_del(maxIdle_mcFlowTbl_idx);
			}	
		}

		if(list_empty(&fc_db.listFree_mcExtraFlowIdxHashTbl))
		{
			WARNING("all free IGMP listFree_mcExtraFlowIdxHashTbl list are allocated and no idle entry can be select...");
			return (NULL);
		}

		list_for_each_entry_safe(mcSwFlowId_entry,mcSwFlowId_entry_tmp,&fc_db.listFree_mcExtraFlowIdxHashTbl,entry)		//just return the first entry right behind of head
		{
			list_del_init(&mcSwFlowId_entry->entry);
			break;
		}
		//IGMP("New a mcflow mcSwFlowId_entry[%d] ",mcSwFlowId_entry->entryIdx);

		/*we should init every field in mcSwFlowId_entry*/
		mcSwFlowId_entry->valid=1;
		mcSwFlowId_entry->dummySyncTimes=0;
		mcSwFlowId_entry->isGrpDftEntry = isGrpDftFwd;
		if(pPktHdr->ip6h)
		{
			mcSwFlowId_entry->isIpv6=1;
			memcpy(mcSwFlowId_entry->multicastAddress,pPktHdr->ip6h->daddr.in6_u.u6_addr32,sizeof(mcSwFlowId_entry->multicastAddress));
			memcpy(mcSwFlowId_entry->sourceAddress,pPktHdr->ip6h->saddr.in6_u.u6_addr32,sizeof(mcSwFlowId_entry->sourceAddress));
		}
		else
		{
			mcSwFlowId_entry->isIpv6=0;
			bzero(mcSwFlowId_entry->multicastAddress,sizeof(mcSwFlowId_entry->multicastAddress));
			bzero(mcSwFlowId_entry->sourceAddress,sizeof(mcSwFlowId_entry->sourceAddress));
			mcSwFlowId_entry->multicastAddress[0] =pFcIngressData->dstIp;
			mcSwFlowId_entry->sourceAddress[0] =pFcIngressData->srcIp;
		}
		if(pFcIngressData->ingressTagif & CVLAN_TAGIF)
			mcSwFlowId_entry->ingressCvid=pFcIngressData->srcCVlanId;
		else
			mcSwFlowId_entry->ingressCvid=FAIL;
		if(pFcIngressData->ingressTagif & SVLAN_TAGIF)
			mcSwFlowId_entry->ingressSvid=pFcIngressData->srcSVlanId;
		else
			mcSwFlowId_entry->ingressSvid=FAIL;

		rtk_fc_flow_skbDataReasm(pFcIngressData,pPktHdr,mcSwFlowId_entry->mcDataBuf.dataBuf.data);
		mcSwFlowId_entry->mcDataBuf.skb_mark = RTSKB_MARK(rtskb);
		mcSwFlowId_entry->mcDataBuf.skb_mark2 = RTSKB_MARK2(rtskb);
		memcpy(&mcSwFlowId_entry->mcDataBuf.fcIngressData,pFcIngressData,sizeof(rtk_fc_ingress_data_t));
		mcSwFlowId_entry->mcDataBuf.fcIngressData.ct = NULL;
		mcSwFlowId_entry->mcDataBuf.fcIngressData.mcDummyPkt=1;
		{
			int i;
			for(i=0;i<DEV_STACK_MAX;i++)
			{
				mcSwFlowId_entry->mcDataBuf.fcIngressData.logicalDevIfidx[i] = DEVIFIDX_INVALID_MIN;
			}
		}
		mcSwFlowId_entry->swFlowIdx =swFlowIdx;

		list_add(&mcSwFlowId_entry->entry, &fc_db.listHead_mcExtraFlowIdxHashTbl[mcflowHash]);

	}

	return mcSwFlowId_entry;

}


int rtk_fc_igmp_mcExtraFlowIdxTbl_free(rtk_fc_mcExtraSwFlowIdx_entry_t* mcSwFlowId_entry)
{
	list_del_init(&mcSwFlowId_entry->entry);
	mcSwFlowId_entry->valid=0;
	list_add(&mcSwFlowId_entry->entry, &fc_db.listFree_mcExtraFlowIdxHashTbl);
	//IGMP("Del mcExtraFlowIdxTbl[%d] ref to swFlowIdx[%d]",mcSwFlowId_entry->entryIdx,mcSwFlowId_entry->swFlowIdx);
	IGMP("Del mcExtraFlowIdxTbl ref to swFlowIdx[%d]",mcSwFlowId_entry->swFlowIdx);	
	return (RT_ERR_RG_OK);
}

int rtk_fc_igmp_mcExtraFlowIdxTbl_del(int32 swFlowIdx)
{
	int i;
	rtk_fc_mcExtraSwFlowIdx_entry_t *mcSwFlowId_entry=NULL;
	rtk_fc_mcExtraSwFlowIdx_entry_t *mcSwFlowId_entry_tmp=NULL;
	
	for (i =0 ; i< RTK_FC_TABLESIZE_MCFLOW_HASH; i++)
	{
		list_for_each_entry_safe(mcSwFlowId_entry,mcSwFlowId_entry_tmp,&fc_db.listHead_mcExtraFlowIdxHashTbl[i],entry)
		{
			if(mcSwFlowId_entry->swFlowIdx==swFlowIdx)
			{
				rtk_fc_igmp_mcExtraFlowIdxTbl_free(mcSwFlowId_entry);
				return SUCCESS;
			}
		}
	}
	IGMP("Can't find swFlowIdx ");
	return FAIL;
}

rtk_fc_mcExtraSwFlowIdx_entry_t* rtk_fc_igmp_mcExtraFlowIdxTbl_find(int32 swFlowIdx)
{
	int i;
	rtk_fc_mcExtraSwFlowIdx_entry_t *mcSwFlowId_entry=NULL;
	rtk_fc_mcExtraSwFlowIdx_entry_t *mcSwFlowId_entry_find=NULL;
	
	for (i =0 ; i< RTK_FC_TABLESIZE_MCFLOW_HASH; i++)
	{
		list_for_each_entry(mcSwFlowId_entry,&fc_db.listHead_mcExtraFlowIdxHashTbl[i],entry)
		{
			if(mcSwFlowId_entry->swFlowIdx==swFlowIdx)
			{
				mcSwFlowId_entry_find=mcSwFlowId_entry;
				break;
			}
		}
	}

	if(mcSwFlowId_entry_find==NULL)
	{
		IGMP("Can't find swFlowIdx ");
	}

	return mcSwFlowId_entry_find;

}



int _rtk_fc_groupLSB32_to_mac(unsigned char* mac,uint32 isIpv6,uint32 groupip_LSB_32)
{
	if(isIpv6)
	{
		mac[0]=0x33;
		mac[1]=0x33;
		mac[2]=(groupip_LSB_32>>24)&0xff;
		mac[3]=(groupip_LSB_32>>16)&0xff;
		mac[4]=(groupip_LSB_32>>8 )&0xff;
		mac[5]=(groupip_LSB_32>>0 )&0xff;
	}
	else
	{
		mac[0]=0x01;
		mac[1]=0x00;
		mac[2]=0x5e;
		mac[3]=(groupip_LSB_32>>16)&0x7f;
		mac[4]=(groupip_LSB_32>>8 )&0xff;
		mac[5]=(groupip_LSB_32>>0 )&0xff;
	}

	return SUCCESS;
}



rtk_fc_ret_t _rtk_fc_cpuSpa_macAdd(uint8 *mac,uint8 isStatic,int16 *mcL2Idx)
{
#if defined(CONFIG_RTK_L34_XPON_PLATFORM) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	int32 ret, i;
	rtk_fc_lut_entry_t *pLutEntry, *pNewLutEntry;
	int32 lutHashIdx=0, lutIdx=0, first_invalid=FAIL;
	rtk_fc_lutCam_linkList_t *pLutCamEntry, *pNextLutCamEntry;

	lutHashIdx = _rtk_fc_hash_mac_fid(mac, DEFAULT_FID);
	//Duplicate check
	list_for_each_entry_rcu(pLutEntry, &fc_db.lut_hash_list_head[lutHashIdx], lutList)
	{
		if(pLutEntry && !memcmp(pLutEntry->l2Addr, mac, ETH_ALEN) )
		{
			if(isStatic)
			{
				if(pLutEntry->isStatic==0)
					pLutEntry->isStatic=1;
				pLutEntry->staticRefCount++;
			}
			pLutEntry->last_jiffies=jiffies;
			*mcL2Idx = pLutEntry->lutIdx;
			return RTK_FC_RET_OK;
		}
	}

	lutIdx = lutHashIdx<<RTK_FC_LUT_HASH_WAY_SHIFT;
	for(i=lutIdx; i<(lutIdx+RTK_FC_LUT_HASH_WAY_SIZE); i++)
	{
		if(fc_db.lutTbl[i]==NULL)
		{
			first_invalid = i;
			TRACE("Found invalid lut[%d]", first_invalid);
			break;
		}
	}
	if(first_invalid==FAIL)
	{
		if(!list_empty(&fc_db.lutCam_freeListHead))
		{
			list_for_each_entry_safe(pLutCamEntry, pNextLutCamEntry, &fc_db.lutCam_freeListHead, lut_list)	//just return the first entry right behind of head
			{
				first_invalid = pLutCamEntry->idx;
				TRACE("Found invalid lut[%d] in cam", first_invalid);
				break;
			}
		}
	}
	if(first_invalid==FAIL)
	{
		if((first_invalid = _rtk_fc_lut_LRU(lutHashIdx)) != FAIL)
		{
			TRACE("Replace least recently used lut[%d]", first_invalid);
		}
		else
		{
			WARNING("Lut table is full, lutHashIdx=%d", lutHashIdx);
			return RTK_FC_RET_LRN_FAIL;
		}
	}

	//clear hw entry,if hybrid mode hardware autolearning may cause hw/sw not sync
	{
		rtk_l2_addr_table_t l2table;
		uint32 getidx=first_invalid;
		if(RT_ERR_OK ==	(rtk_l2_nextValidEntry_get(&getidx, &l2table)))
		{
			if(first_invalid == getidx && l2table.entryType==RTK_LUT_L2UC)
			{
				TABLE("clear hw lut for sw learning Idx:%d",first_invalid);
				rtk_l2_addr_del(&l2table.entry.l2UcEntry);
			}
		}
	}

	pNewLutEntry = _rtk_fc_lutEntry_getFromPool();
	if(pNewLutEntry==NULL)
	{
		WARNING("[Fail to add lut] Lut table pool is full.");
		return RTK_FC_RET_LRN_FAIL;
	}
#if defined(CONFIG_RTK_L34_XPON_PLATFORM) 
	pNewLutEntry->spa = RTK_FC_MAC_PORT_CPU;
#else
	pNewLutEntry->spa = RTK_FC_MAC_PORT_L3_WAN;
#endif

	pNewLutEntry->extspa = RTK_FC_MAC_EXT_PORT_MAX;
	pNewLutEntry->wlan_dev_idx=RTK_FC_WLANX_NOT_FOUND;
	memcpy(pNewLutEntry->l2Addr, mac, ETH_ALEN);
	pNewLutEntry->isStatic = isStatic;
	pNewLutEntry->last_jiffies = jiffies;
	if(isStatic)
		pNewLutEntry->staticRefCount=1;
	pNewLutEntry->isSync2PsEntry = FALSE;
	pNewLutEntry->hostPolIdx = FAIL;
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	pNewLutEntry->wifiMacId = 0;
#endif

	ret = RTK_FC_LUT_ADD(pNewLutEntry, FALSE);
	if(ret)
	{
		TABLE("MC MAC ADD FAILED ! ret=%d",ret);
		_rtk_fc_lutEntry_returnToPool(pNewLutEntry);
		return RTK_FC_RET_LRN_FAIL;
	}
	//if(pNewLutEntry->lutIdx!=first_invalid)
		//WARNING("New lut's index(%d) is not as expected index(%d)", pNewLutEntry->lutIdx, first_invalid);
	*mcL2Idx = pNewLutEntry->lutIdx;

#endif

	return RTK_FC_RET_OK;
}


/*
* groupip_LSB_32: host byte order
*/
rtk_fc_ret_t _rtk_fc_igmp_lutMcAdd(uint8 isIpv6 ,uint32 groupip_LSB_32,uint8 isStatic,int16 *mcL2Idx)
{
#if defined(CONFIG_RTK_L34_XPON_PLATFORM) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	uint8 mcMac[ETH_ALEN]={0};

	_rtk_fc_groupLSB32_to_mac(mcMac, isIpv6, groupip_LSB_32);

	return _rtk_fc_cpuSpa_macAdd(mcMac,isStatic,mcL2Idx);

#else
	return RTK_FC_RET_OK;
#endif
}



#define MC_DUMMY_SKB_PRI 7

#if 0 // not support
typedef struct rtk_fc_skb_netif_receiceWK_s
{
	struct work_struct work;
	struct sk_buff *skb;
}rtk_fc_skb_netif_receice_wq_t;

void rtk_fc_netif_reveive_skb_wq(struct work_struct *p_work)
{
	rtk_fc_skb_netif_receice_wq_t *p_callback_work = container_of(p_work, rtk_fc_skb_netif_receice_wq_t, work);
	RTK_FC_HOOK_PS_SKB_NETIF_RECEIVE_SKB(p_callback_work->skb);
	RTK_FC_HELPER_FC_KFREE(p_callback_work,sizeof(*p_callback_work));
}

int32 rtk_fc_skb_netif_receive_skb_wq_schedule(struct sk_buff *skb)
{
	int32 ret;
	rtk_fc_skb_netif_receice_wq_t *work = RTK_FC_HELPER_FC_KMALLOC(sizeof(typeof(*work)),GFP_ATOMIC);
	INIT_WORK(&work->work, rtk_fc_netif_reveive_skb_wq);
	work->skb = skb;
	ret=schedule_work(&work->work);
	if(ret==false)
		WARNING("Wq Error\n");	
	return SUCCESS;
}
#endif

//int32	hwIdx;					//9607C Series is lut idx / G3 Series is hash Idx
//int32	hwIdx2; 				//9607C Series is FAIL / G3 Series 8277C is lutIdx
int32 _rtk_fc_mc_groupToHwAsUnknown(int32 hwIdx,int32 hwIdx2)
{

	int32 ret=FAIL;
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	int32 deleteIdx=hwIdx;
	ca_ip_address_t groupAddr;				//non-used parameter
	bzero(&groupAddr,sizeof(groupAddr));
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
{
	rtk_fc_tableFlow_t *pFlowTable;

	ret=rtk_rg_flow_mcast_group_asunknown(groupAddr,&deleteIdx);
	if(ret!=SUCCESS )
	{
		WARNING("rtk_rg_flow_mcast_group_asunknown Error ret=%d",ret);
		return ret;
	}

	//invalid hwFlow_validBitsArray
	if(deleteIdx < fc_db.flowHwTableSize)
	{
		fc_db.g3_mHashTbl_validBitsArray[(deleteIdx >> 5)] &= ~(0x1 << (deleteIdx&0x1f));
	}
	
	pFlowTable = &fc_db.flowTbl[deleteIdx];

	pFlowTable->pFlowEntry->path1.valid = 0;

	// initialize sw field of hwflow
	pFlowTable->cachedCt = NULL;
	//--------------- RESET: init value which is 0	   -------------------------------------------------------------------------------//
	memset((uint8 *)pFlowTable + offsetof(rtk_fc_tableFlow_t, cachedCt) , 0 , (sizeof(rtk_fc_tableFlow_t)- offsetof(rtk_fc_tableFlow_t, cachedCt) ) );
	//------------------------------------------------------------------------------------------------------------------------------//
	
	//----------------RESET: init value which is not 0 -------------------------------------------------------------------------------//
	pFlowTable->loopbackRevFlowIdx = SIGNED_INVALID;		// shared info
	pFlowTable->lutIgrSaIdx = SIGNED_INVALID;
	pFlowTable->lutEgrDaIdx = SIGNED_INVALID;
	pFlowTable->pAbstrSwFlowEt=NULL;
	//------------------------------------------------------------------------------------------------------------------------------//


	if(hwIdx2 != FAIL)
	{
		if(fc_db.lutTbl[hwIdx2])
		{
			ASSERT_EQ(_rtk_fc_lut_staticEntry_del(hwIdx2, FALSE), SUCCESS);
		}
	}


}
#else
	ret=rtl_l2_mcast_group_asunknown(G3_DEF_DEVID,groupAddr,&deleteIdx);
#endif
	IGMP("set rtl_l2_mcast_group_asunknown ");
	if(ret!=CA_E_OK )
	{
		WARNING("rtl_l2_mcast_group_asunknown Error ret=%d",ret);
	}			
#else

	ret=_rtk_fc_lut_staticEntry_del((uint32)hwIdx, FALSE);
	if(ret!=SUCCESS )
	{
		WARNING("_rtk_fc_lut_staticEntry_del Error ret=%d",ret);
	}

#endif

	return ret;

}

//int32	hwIdx;					//9607C Series is lut idx / G3 Series is hash Idx
//int32	hwIdx2; 				//9607C Series is FAIL / G3 Series 8277C is lutIdx
int32 _rtk_fc_mc_groupToHwAsKnown(uint32 is_ipv6,uint32 *gip,int32 *hwIdx,int32 *hwIdx2)
{

	int32 ret=RTK_FC_RET_LRN_FAIL;
	*hwIdx=FAIL;
	*hwIdx2=FAIL;
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
{
	ca_ip_address_t groupAddr;
	bzero(&groupAddr,sizeof(groupAddr));
	
	if(is_ipv6)
	{
		groupAddr.afi =CA_IPV6;
		groupAddr.addr_len=128;
		groupAddr.ip_addr.ipv6_addr[0] = ntohl(gip[0]);
		groupAddr.ip_addr.ipv6_addr[1] = ntohl(gip[1]);
		groupAddr.ip_addr.ipv6_addr[2] = ntohl(gip[2]);
		groupAddr.ip_addr.ipv6_addr[3] = ntohl(gip[3]);
	}
	else
	{
		groupAddr.afi =CA_IPV4;
		groupAddr.addr_len=32;
		groupAddr.ip_addr.ipv4_addr =gip[0];
	}
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
{
	int16 macIdx=FAIL;
	ret=rtk_rg_flow_mcast_group_asknown(groupAddr,hwIdx);
	if(ret != RTK_FC_RET_OK)
	{
		IGMP("add known group fail %d",ret);
		return ret;
	}
	else
		IGMP("group Idx=%d",*hwIdx);

	//set to path6 is a static entry
	fc_db.flowTbl[*hwIdx].pFlowEntry->path6.valid=TRUE;
	fc_db.flowTbl[*hwIdx].pFlowEntry->path6.in_path= FB_PATH_6;
	fc_db.flowTbl[*hwIdx].candidateState=CANDIDATE_STATE_NEW;
	fc_db.flowTbl[*hwIdx].lutIgrSaIdx = FAIL;
	fc_db.flowTbl[*hwIdx].lutEgrDaIdx = FAIL;
	fc_db.flowTbl[*hwIdx].flow_extra_info = 0;
	fc_db.g3_mHashTbl_validBitsArray[((*hwIdx) >> 5)] |= (0x1 << ((*hwIdx)&0x1f));
	FLOW_INFO_SET(&fc_db.flowTbl[*hwIdx], FLOW_INFO_STATIC_ENTRY, 1);


	if(is_ipv6)
	{
		ret=_rtk_fc_igmp_lutMcAdd(is_ipv6,ntohl(gip[3]),1,&macIdx);
	}
	else
	{
		ret=_rtk_fc_igmp_lutMcAdd(is_ipv6,gip[0],1,&macIdx);
	}
	if(ret==RTK_FC_RET_OK)
		*hwIdx2=macIdx;	

}
#else	
	ret=rtl_l2_mcast_group_asknown(G3_DEF_DEVID,groupAddr,hwIdx);
#endif
}
#else
{
	int16 macIdx;
	if(is_ipv6)
	{
		ret=_rtk_fc_igmp_lutMcAdd(is_ipv6,ntohl(gip[3]),1,&macIdx);
	}
	else
	{
		ret=_rtk_fc_igmp_lutMcAdd(is_ipv6,gip[0],1,&macIdx);
	}
	*hwIdx=macIdx;

}
#endif
	if(ret != RTK_FC_RET_OK)
		IGMP("add known group fail %x",ret);
	else
		IGMP("group hwIdx=%d hwIdx2=%d",*hwIdx,*hwIdx2);

	return ret;
}

int rtk_fc_igmp_unknownMulticast_action_set(rt_igmp_unknownMc_cfg_t mcUnknCfg)
{


#if defined(CONFIG_RTK_L34_XPON_PLATFORM)

	if(mcUnknCfg.isipv6)
		fc_db.controlFuc.unknowv6McTrap = mcUnknCfg.trapPkt;
	else
		fc_db.controlFuc.unknowv4McTrap = mcUnknCfg.trapPkt;

	if(mcUnknCfg.trapPkt==0)
	{
		if(mcUnknCfg.isipv6)
			rtk_l2_lookupMissAction_set(DLF_TYPE_IP6MC,ACTION_DROP);
		else
			rtk_l2_lookupMissAction_set(DLF_TYPE_IPMC, ACTION_DROP);
	}
	else if(mcUnknCfg.trapPkt==1)
	{
		if(mcUnknCfg.isipv6)
			rtk_l2_lookupMissAction_set(DLF_TYPE_IP6MC,ACTION_TRAP2CPU);
		else
			rtk_l2_lookupMissAction_set(DLF_TYPE_IPMC, ACTION_TRAP2CPU);
	}
	else
	{
		if(mcUnknCfg.isipv6)
			rtk_l2_lookupMissAction_set(DLF_TYPE_IP6MC,ACTION_FORWARD);
		else
			rtk_l2_lookupMissAction_set(DLF_TYPE_IPMC, ACTION_FORWARD);

	}
#else

	if(mcUnknCfg.trapPkt>=2)
		WARNING(" Not Support forward action ");

	if(mcUnknCfg.trapPkt)
	{
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		rtk_rg_flow_default_action_update(RTK_ASIC_FLOW_PROFILE_FLOW_MC,RTK_ASIC_FLOW_DEFACT_TYPE_TRAP);
#else
		aal_default_action_set(G3_DEF_DEVID, RG_CA_FLOW_MC, TRUE);
#endif
		fc_db.controlFuc.unknowv6McTrap=1;
		fc_db.controlFuc.unknowv4McTrap=1;

	}
	else
	{
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		rtk_rg_flow_default_action_update(RTK_ASIC_FLOW_PROFILE_FLOW_MC,RTK_ASIC_FLOW_DEFACT_TYPE_DROP);
#else
		aal_default_action_set(G3_DEF_DEVID, RG_CA_FLOW_MC, FALSE);
#endif
		fc_db.controlFuc.unknowv6McTrap=0;
		fc_db.controlFuc.unknowv4McTrap=0;

	}

#endif

	API("set multicast isip6=%d trap=%d ",mcUnknCfg.isipv6,mcUnknCfg.trapPkt);

	
	return SUCCESS;

}


rtk_fc_table_mcGroupTbl_t* rtk_fc_mc_findKnownGroupEntry(uint32 is_ipv6,uint32 *gip)
{
	int i;

	for(i=0;i<RTK_FC_TABLESIZE_MCFLOW;i++)
	{
		if(fc_db.mcGroupTbl[i].valid)
		{
			if(is_ipv6 != fc_db.mcGroupTbl[i].isIpv6)
				continue;
			
			if(is_ipv6)
			{
				if(memcmp(fc_db.mcGroupTbl[i].mcGroup,gip,sizeof(fc_db.mcGroupTbl[i].mcGroup))==0)
				{
					return &fc_db.mcGroupTbl[i];
				}
				else
				{
					//IGMP("GroupTbl[%d] %pI6 != %pI6",i,fc_db.mcGroupTbl[i].mcGroup,gip);
				}
			}
			else
			{
				if(fc_db.mcGroupTbl[i].mcGroup[0]==gip[0])
				{
					return &fc_db.mcGroupTbl[i];
				}
				else
				{
					//IGMP("GroupTbl[%d] %pI4h != %pI4h",i,&fc_db.mcGroupTbl[i].mcGroup[0],&gip[0]);
				}
			}
				
		}
	}

	return NULL;


}


int32 rtk_fc_mc_delKnownGroupEntry(rtk_fc_table_mcGroupTbl_t* mcGroupEt)
{
	if(mcGroupEt->valid && (mcGroupEt->groupRefCnt>0))
	{
		IGMP("delKnownGroupEntry  groupRefCnt:%d->%d",mcGroupEt->groupRefCnt,mcGroupEt->groupRefCnt-1);
		mcGroupEt->groupRefCnt--;
		if(mcGroupEt->groupRefCnt==0)
		{
			_rtk_fc_mc_groupToHwAsUnknown(mcGroupEt->hwIdx,mcGroupEt->hwIdx2);
			mcGroupEt->valid=0;
			return SUCCESS;
		}

	}
	return SUCCESS;
}

rtk_fc_table_mcGroupTbl_t* rtk_fc_mc_addKnownGroupEntry(uint32 is_ipv6,uint32 *gip)
{
	int i;
	int32 firtInvalidIdx=FAIL;
	int32 dupEtIdx=FAIL;

	for(i=0;i<RTK_FC_TABLESIZE_MCFLOW;i++)
	{
		if(fc_db.mcGroupTbl[i].valid)
		{
			if(is_ipv6 != fc_db.mcGroupTbl[i].isIpv6)
				continue;
			
			if(is_ipv6)
			{

				if(memcmp(fc_db.mcGroupTbl[i].mcGroup,gip,sizeof(fc_db.mcGroupTbl[i].mcGroup))==0)
					dupEtIdx=i;
			}
			else
			{
				if(fc_db.mcGroupTbl[i].mcGroup[0]==gip[0])
					dupEtIdx=i;
			}
				
		}
		else
		{
			if(firtInvalidIdx==FAIL)
				firtInvalidIdx=i;
		}	
	}

	if(firtInvalidIdx==FAIL && dupEtIdx==FAIL)
	{
		IGMP("mcGroupTbl Full ");
		rtk_fc_drv_event_record(RTK_FC_DRV_EVENT_REC_MC_TBL_FULL);
		return NULL;
	}

	if(dupEtIdx==FAIL)
	{
		//newEt
		bzero(&fc_db.mcGroupTbl[firtInvalidIdx],sizeof(rtk_fc_table_mcGroupTbl_t));
		if(_rtk_fc_mc_groupToHwAsKnown(is_ipv6,gip,&fc_db.mcGroupTbl[firtInvalidIdx].hwIdx,&fc_db.mcGroupTbl[firtInvalidIdx].hwIdx2)!=RTK_FC_RET_OK)
			return NULL;		
		fc_db.mcGroupTbl[firtInvalidIdx].valid=1;
		fc_db.mcGroupTbl[firtInvalidIdx].groupRefCnt=1;
		fc_db.mcGroupTbl[firtInvalidIdx].isIpv6=is_ipv6;
		if(is_ipv6)
		{
			memcpy(fc_db.mcGroupTbl[firtInvalidIdx].mcGroup,gip,sizeof(fc_db.mcGroupTbl[firtInvalidIdx].mcGroup));
		}
		else
		{
			fc_db.mcGroupTbl[firtInvalidIdx].mcGroup[0]=gip[0];			
		}

		return &fc_db.mcGroupTbl[firtInvalidIdx];
	}
	else
	{
		IGMP("Using a old groupEt add ref to %d->%d",fc_db.mcGroupTbl[dupEtIdx].groupRefCnt,fc_db.mcGroupTbl[dupEtIdx].groupRefCnt+1);
		//dupEt
		fc_db.mcGroupTbl[dupEtIdx].groupRefCnt++;
		return &fc_db.mcGroupTbl[dupEtIdx];
	}


}

rtk_fc_table_mcConfigTbl_t* rtk_fc_mc_mallocMcGrpDefaultConfig (uint32 is_ipv6,uint32 *gip,uint8 careCvid,int16 ingressCvid,uint8 careSvid,int16 ingressSvid)
{
	rtk_fc_table_mcConfigTbl_t *pMcConfig;

	if(gip==NULL )
		return NULL;

	pMcConfig = RTK_FC_HELPER_FC_KMALLOC(sizeof(rtk_fc_table_mcConfigTbl_t), GFP_ATOMIC);

	if(pMcConfig==NULL)
	{
		IGMP("mcGroupTbl no memory ");
		return NULL;
	}


	//new entry
	if(rtk_fc_mc_addKnownGroupEntry(is_ipv6,gip)==NULL)
	{
		if(pMcConfig) RTK_FC_HELPER_FC_KFREE(pMcConfig, sizeof(rtk_fc_table_mcConfigTbl_t));
		return NULL;
	}

	bzero(pMcConfig,sizeof(rtk_fc_table_mcConfigTbl_t));
	pMcConfig->dftGrpFwd=1;
	
	pMcConfig->careIngressCvid=careCvid;
	if(careCvid)
		pMcConfig->confIgrCvid=ingressCvid;
	
	pMcConfig->careIngressSvid=careSvid;
	if(careSvid)
		pMcConfig->confIgrSvid=ingressSvid;		
	
	pMcConfig->isIpv6=is_ipv6;
	if(is_ipv6)
	{
		memcpy(pMcConfig->confGroup,gip,sizeof(pMcConfig->confGroup));

	}
	else
	{
		pMcConfig->confGroup[0]=gip[0];

	}

	//add to list head
	INIT_LIST_HEAD(&pMcConfig->lsMcConfigEntry);
	list_add_tail(&pMcConfig->lsMcConfigEntry,&fc_db.listHead_mcCfgTbGrplDft);

	
	return pMcConfig;


}


rtk_fc_table_mcConfigTbl_t* rtk_fc_mc_mallocMcConfig (uint32 is_ipv6,uint32 *gip,uint8 careSource,uint32 *sip,uint8 careCvid,int16 ingressCvid,uint8 careSvid,int16 ingressSvid)
{
	rtk_fc_table_mcConfigTbl_t *pMcConfig;

	if(gip==NULL || (careSource && sip==NULL))
		return NULL;


	pMcConfig = RTK_FC_HELPER_FC_KMALLOC(sizeof(rtk_fc_table_mcConfigTbl_t), GFP_ATOMIC);

	if(pMcConfig==NULL)
	{
		IGMP("mcGroupTbl no memory ");
		rtk_fc_drv_event_record(RTK_FC_DRV_EVENT_REC_MC_TBL_FULL);
		return NULL;
	}

	//new entry
	if(rtk_fc_mc_addKnownGroupEntry(is_ipv6,gip)==NULL)
	{
		RTK_FC_HELPER_FC_KFREE(pMcConfig,sizeof(rtk_fc_table_mcConfigTbl_t));
		return NULL;
	}
	bzero(pMcConfig,sizeof(rtk_fc_table_mcConfigTbl_t));
	
	pMcConfig->careIngressCvid=careCvid;
	if(careCvid)
		pMcConfig->confIgrCvid=ingressCvid;
	
	pMcConfig->careIngressSvid=careSvid;
	if(careSvid)
		pMcConfig->confIgrSvid=ingressSvid;		
	
	pMcConfig->careSource=careSource;
	pMcConfig->isIpv6=is_ipv6;
	if(is_ipv6)
	{
		memcpy(pMcConfig->confGroup,gip,sizeof(pMcConfig->confGroup));
		if(careSource && sip)
			memcpy(pMcConfig->confSource,sip,sizeof(pMcConfig->confSource));
	}
	else
	{
		pMcConfig->confGroup[0]=gip[0];
		if(careSource && sip)
			pMcConfig->confSource[0]=sip[0];
	}

	//add to list head
	INIT_LIST_HEAD(&pMcConfig->lsMcConfigEntry);
	list_add_tail(&pMcConfig->lsMcConfigEntry,&fc_db.listHead_mcCfgTbl);
	
	return pMcConfig;


}

rtk_fc_table_mcConfigTbl_t* rtk_fc_mc_getMcConfig(uint32 is_ipv6,uint32 *gip,uint8 careSource,uint32 *sip,uint8 careCvid,int16 ingressCvid,uint8 careSvid,int16 ingressSvid)
{
	int32 gipHit=0,sipHit=0,vlanHit=0,vlanSHit=0;
	rtk_fc_table_mcConfigTbl_t *pMcConfig;
	rtk_fc_table_mcConfigTbl_t *pMcConfigHit=NULL;

	if(gip==NULL || (careSource && sip==NULL))
		return NULL;


	list_for_each_entry(pMcConfig,&fc_db.listHead_mcCfgTbl,lsMcConfigEntry)
	{
		sipHit=0;vlanHit=0;vlanSHit=0;gipHit=0;
		if(is_ipv6 != pMcConfig->isIpv6)
			continue;
		if(careSource != pMcConfig->careSource)
			continue;
		if(careCvid != pMcConfig->careIngressCvid)
			continue;
		if(careSvid != pMcConfig->careIngressSvid)
			continue;		
		
		if(is_ipv6)
		{
			if(memcmp(pMcConfig->confGroup,gip,sizeof(pMcConfig->confGroup))==0)
				gipHit=1;
			if(careSource && sip )
			{
				if(memcmp(pMcConfig->confSource,sip,sizeof(pMcConfig->confSource))==0)
					sipHit=1;
			}
			else
				sipHit=1;
		}
		else
		{
			if(pMcConfig->confGroup[0]==gip[0])
				gipHit=1;
			if(careSource && sip)
			{
				if(pMcConfig->confSource[0]==sip[0])
					sipHit=1;
			}
			else
				sipHit=1;
		}
		
		if(careCvid)
		{
			if( pMcConfig->confIgrCvid==ingressCvid)
				vlanHit=1;
		}
		else
			vlanHit=1;

		if(careSvid)
		{
			if( pMcConfig->confIgrSvid==ingressSvid)
				vlanSHit=1;
		}
		else
			vlanSHit=1;
		
		IGMP("gipHit=%d sipHit=%d vlanHit=%d vlanSHit:%d",gipHit,sipHit,vlanHit,vlanSHit);
		if(gipHit && sipHit && vlanHit && vlanSHit)
		{
			pMcConfigHit=pMcConfig;
			break;
		}

	}
	
	return pMcConfigHit;
	

}


rtk_fc_table_mcConfigTbl_t* rtk_fc_mc_getDftMcConfig(uint32 is_ipv6,uint32 *gip,uint8 careCvid,int16 ingressCvid,uint8 careSvid,int16 ingressSvid)
{
	int32 gipHit=0,vlanHit=0,vlanSHit=0;
	rtk_fc_table_mcConfigTbl_t *pMcConfig;
	rtk_fc_table_mcConfigTbl_t *pMcConfigHit=NULL;

	if(gip==NULL )
		return NULL;


	list_for_each_entry(pMcConfig,&fc_db.listHead_mcCfgTbGrplDft,lsMcConfigEntry)
	{
		vlanHit=0;vlanSHit=0;gipHit=0;
		if(is_ipv6 != pMcConfig->isIpv6)
			continue;
		if(careCvid != pMcConfig->careIngressCvid)
			continue;
		if(careSvid != pMcConfig->careIngressSvid)
			continue;

		
		if(is_ipv6)
		{
			if(memcmp(pMcConfig->confGroup,gip,sizeof(pMcConfig->confGroup))==0)
				gipHit=1;
		}
		else
		{
			if(pMcConfig->confGroup[0]==gip[0])
				gipHit=1;
		}
		
		if(careCvid)
		{
			if( pMcConfig->confIgrCvid==ingressCvid)
				vlanHit=1;
		}
		else
			vlanHit=1;

		if(careSvid)
		{
			if( pMcConfig->confIgrSvid==ingressSvid)
				vlanSHit=1;
		}
		else
			vlanSHit=1;		
		
		IGMP("gipHit=%d vlanHit=%d vlanSHit:%d",gipHit,vlanHit,vlanSHit);
		if(gipHit && vlanHit && vlanSHit)
		{
			pMcConfigHit = pMcConfig;
			break;
		}

	}

	return pMcConfigHit;
	
}


//first search mcCfgTbl
//second search mcCfgTbGrplDft
//if all not hit return NULL
rtk_fc_table_mcConfigTbl_t* rtk_fc_mc_searchAllMcConfig(uint32 is_ipv6,uint32 *gip,uint8 careSource,uint32 *sip,uint8 careCvid,int16 ingressCvid,uint8 careSvid,int16 ingressSvid)
{
	rtk_fc_table_mcConfigTbl_t* mcCfg=NULL;
	mcCfg = rtk_fc_mc_getMcConfig(is_ipv6,gip,careSource,sip,careCvid,ingressCvid,careSvid,ingressSvid);
	if(mcCfg==NULL)
		mcCfg = rtk_fc_mc_getDftMcConfig(is_ipv6,gip,careCvid,ingressCvid,careSvid,ingressSvid);
	return mcCfg;
}


int rtk_fc_mc_delMcConfig(rtk_fc_table_mcConfigTbl_t* delconf)
{
	rtk_fc_table_mcGroupTbl_t* knownGroup= rtk_fc_mc_findKnownGroupEntry(delconf->isIpv6,delconf->confGroup);
	if(knownGroup)
		rtk_fc_mc_delKnownGroupEntry(knownGroup);
	list_del(&delconf->lsMcConfigEntry);
	RTK_FC_HELPER_FC_KFREE(delconf,sizeof(*delconf));
	return SUCCESS;
}

//cvlan -1 is untag
rtk_fc_table_mcConfigTbl_t* rtk_fc_mc_searchMcConfigByPatten(uint8 isIpv6 ,uint32 *groupIp ,uint32 *sourceIp,int16 igrCvlan,int16 igrSvlan)
{

	int32 gipHit=0,sipHit=0,vlanHit=0,vlanSHit=0;
	rtk_fc_table_mcConfigTbl_t *pMcConfig;
	rtk_fc_table_mcConfigTbl_t *pMcConfigHit=NULL;

	list_for_each_entry(pMcConfig,&fc_db.listHead_mcCfgTbl,lsMcConfigEntry)
	{
		sipHit=0;vlanHit=0;vlanSHit=0;gipHit=0;

		if(isIpv6==1 && pMcConfig->isIpv6==1)
		{
			if(memcmp(pMcConfig->confGroup,groupIp,sizeof(pMcConfig->confGroup))==0)
				gipHit=1;
			if(pMcConfig->careSource )
			{
				if(memcmp(pMcConfig->confSource,sourceIp,sizeof(pMcConfig->confSource))==0)
					sipHit=1;
			}
			else
				sipHit=1;
		}
		else if(isIpv6==0 && pMcConfig->isIpv6==0)
		{
			if(pMcConfig->confGroup[0]== (*groupIp))
				gipHit=1;
			if(pMcConfig->careSource)
			{
				if(pMcConfig->confSource[0]==(*sourceIp))
					sipHit=1;
			}
			else
				sipHit=1;
		}

		if(pMcConfig->careIngressCvid)
		{
			if( pMcConfig->confIgrCvid==igrCvlan)
				vlanHit=1;
		}
		else
			vlanHit=1;

		if(pMcConfig->careIngressSvid)
		{
			if( pMcConfig->confIgrSvid==igrSvlan)
				vlanSHit=1;
		}
		else
			vlanSHit=1;		

		IGMP("gipHit=%d sipHit=%d vlanHit=%d vlanSHit:%d",gipHit,sipHit,vlanHit,vlanSHit);
		if(gipHit && sipHit && vlanHit && vlanSHit)
		{
			pMcConfigHit=pMcConfig;
			break;
		}
	}

	return pMcConfigHit;

}


//just reutrn first hit
rtk_fc_table_mcConfigTbl_t* rtk_fc_mc_searchDftMcConfigByPktPatten(uint8 isIpv6,uint32 *groupIp,uint32 *sourceIp,int16 igrCvlan,int16 igrSvlan)
{
	int32 gipHit=0,vlanHit=0,vlanSHit=0;
	rtk_fc_table_mcConfigTbl_t *pMcConfig;
	rtk_fc_table_mcConfigTbl_t *pMcConfigHit=NULL;

	list_for_each_entry(pMcConfig,&fc_db.listHead_mcCfgTbGrplDft,lsMcConfigEntry)
	{
		vlanHit=0;vlanSHit=0;gipHit=0;

		if(isIpv6==1 && pMcConfig->isIpv6==1)
		{
			if(memcmp(pMcConfig->confGroup,groupIp,sizeof(pMcConfig->confGroup))==0)
				gipHit=1;
		}
		else if(isIpv6==0 && pMcConfig->isIpv6==0)
		{
			if(pMcConfig->confGroup[0]==ntohl(groupIp[0]))
				gipHit=1;
		}

		if(pMcConfig->careIngressCvid )
		{
			if( pMcConfig->confIgrCvid==igrCvlan)
				vlanHit=1;
		}
		else
			vlanHit=1;

		if(pMcConfig->careIngressSvid )
		{
			if( pMcConfig->confIgrSvid==igrSvlan)
				vlanSHit=1;
		}
		else
			vlanSHit=1;


		IGMP("gipHit=%d vlanHit=%d vlanSHit=%d",gipHit,vlanHit,vlanSHit);
		if(gipHit  && vlanHit && vlanSHit)
		{
			pMcConfigHit=pMcConfig;
			break;
		}

	}

	return pMcConfigHit;
}



int32 rtk_fc_igmp_checkFlow(rtk_fc_table_mcConfigTbl_t* mcConfig)
{

	int i;
	int32 gorupIpHit=0;
	int32 careSourceAndHit=0;
	int32 careCvidAndHit=0;
	int32 careSvidAndHit=0;

	for(i=0 ; i<RTK_FC_TABLESIZE_MCFLOW ;i++)
	{

		gorupIpHit=0;
		careSourceAndHit=0;
		careCvidAndHit=0;
		careSvidAndHit=0;

		if(!fc_db.mcExtraFlowIdxTbl[i].valid)
			continue;

		if(mcConfig->dftGrpFwd != fc_db.mcExtraFlowIdxTbl[i].isGrpDftEntry)
			continue;

		if(mcConfig->isIpv6==fc_db.mcExtraFlowIdxTbl[i].isIpv6 && memcmp(fc_db.mcExtraFlowIdxTbl[i].multicastAddress,mcConfig->confGroup,sizeof(fc_db.mcExtraFlowIdxTbl[i].multicastAddress))==0)
			gorupIpHit=1;
		
		if(mcConfig->careSource)
		{
			if(mcConfig->isIpv6==fc_db.mcExtraFlowIdxTbl[i].isIpv6 && (memcmp(fc_db.mcExtraFlowIdxTbl[i].sourceAddress,mcConfig->confSource,sizeof(fc_db.mcExtraFlowIdxTbl[i].sourceAddress))==0))
				careSourceAndHit=1;
		}
		else
			careSourceAndHit=1; // treat as hit
		
		if(mcConfig->careIngressCvid)
		{
			if(( mcConfig->confIgrCvid == fc_db.mcExtraFlowIdxTbl[i].ingressCvid) )
				careCvidAndHit=1;
		}
		else
			careCvidAndHit=1;  // treat as hit

		if(mcConfig->careIngressSvid)
		{
			if(( mcConfig->confIgrSvid == fc_db.mcExtraFlowIdxTbl[i].ingressSvid) )
				careSvidAndHit=1;
		}
		else
			careSvidAndHit=1;  // treat as hit


		//all hit delete relative table
		IGMP("mcflowIdx[%d] gorupIpHit:%d careSourceAndHit:%d careCvidAndHit:%d careSvidAndHit:%d",i,gorupIpHit,careSourceAndHit,careCvidAndHit,careSvidAndHit);
		if(gorupIpHit && careSourceAndHit && careCvidAndHit && careSvidAndHit)
			return FALSE;
	}
	
	return TRUE;
} 



struct sk_buff* rtk_fc_abstrSwflow_dummyPkt_genskb(rtk_fc_mcExtraSwFlowIdx_entry_t *mcFlowIdxEt)
{
	struct sk_buff *skb = NULL;
	struct rt_skbuff *rtskb, rtSKB;
	struct net_device *SrcDev=NULL;

	if(mcFlowIdxEt==NULL)
		return NULL;
	if(mcFlowIdxEt->valid==0)
		return NULL;
	RTK_FC_HOOK_PS_SKB_DEV_ALLOC_SKB(SKB_BUF_SIZE, &skb);
	if(skb==NULL)
		return NULL;
	if(DEVIFIDX_VALID_MIN <= mcFlowIdxEt->mcDataBuf.fcIngressData.SrcDevDevGwMacIdx)
		SrcDev = fc_db.devGwMacTbl[mcFlowIdxEt->mcDataBuf.fcIngressData.SrcDevDevGwMacIdx].dev;
	if(SrcDev==NULL)
		return NULL;

	// set l4_hash=1 sw_hash=1; and hash =0 to avoid enqueue to backlog again
	if(fc_db.hook_func.ps->fc_skb_set_sw_hash)
		fc_db.hook_func.ps->fc_skb_set_sw_hash(skb);
	if(fc_db.hook_func.ps->fc_skb_set_l4_hash)
		fc_db.hook_func.ps->fc_skb_set_l4_hash(skb);
	if(fc_db.hook_func.ps->fc_skb_set_priority)
		fc_db.hook_func.ps->fc_skb_set_priority(skb,MC_DUMMY_SKB_PRI);

	rtskb = &rtSKB;
	RTK_FC_HOOK_CONVERTER_SKB(skb, rtskb);

	RTSKB_DEV(rtskb) = SrcDev;

	RTSKB_MARK(rtskb) = mcFlowIdxEt->mcDataBuf.skb_mark;	
	if(RTSKB_MARK2_EXIST(rtskb))
		RTSKB_MARK2_FORCE(rtskb) = mcFlowIdxEt->mcDataBuf.skb_mark2;

	RTSKB_HASH(rtskb)=0;

	/* *pData = skb_put(skb, len); */
	RTK_FC_HOOK_PS_SKB_SKB_PUT(RTSKB_SKB(rtskb),MAX_AUTOEXT_PKT_SIZE,&RTSKB_DATA(rtskb));

	memcpy(RTSKB_DATA(rtskb), mcFlowIdxEt->mcDataBuf.dataBuf.data, RTSKB_LEN(rtskb));
	memcpy(RTSKB_FCIGRDATA(rtskb),&mcFlowIdxEt->mcDataBuf.fcIngressData,sizeof(rtk_fc_ingress_data_t));
	atomic_inc(&fc_db.statistic.perPortCnt_dummpPktAlloc[RTSKB_FCIGRDATA(rtskb)->ingressPort]);

	//netif_rx receive RTSKB_DATA(rtskb) = skb->header+ ETH_HLEN(14) ,we should set RTSKB_PROTOCOL(rtskb) correct
	IGMP("[McData] send dummy pkt via netdev %s len=%d", RTSKB_DEV(rtskb)->name,RTSKB_LEN(rtskb));
	if(fc_db.debugLevel & FC_DEBUG_LEVEL_TRACE_DUMP )
		dump_packet( mcFlowIdxEt->mcDataBuf.dataBuf.data, MAX_AUTOEXT_PKT_SIZE, "[McData]");


	return RTSKB_SKB(rtskb);
}


rtk_fc_ret_t rtk_fc_abstrSwflow_sendMcDummyPkt(rtk_fc_mcExtraSwFlowIdx_entry_t *mcFlowIdxEt)
{
	struct sk_buff *skb = NULL;
	skb = rtk_fc_abstrSwflow_dummyPkt_genskb(mcFlowIdxEt);
	if(NULL==skb)
		return RTK_FC_RET_ERR;

/*
//	way1.using workqueue:
//		-get a warning message "NOHZ:local_softirq_peding 08"
//		-get a GPL problem when lock debug open

	rtk_fc_skb_netif_receive_skb_wq_schedule(RTSKB_SKB(rtskb));
*/

/*
//	way2.using netif_receive_skb
//		-get a tx deadlock problem
		
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();
	if(RTK_FC_HOOK_PS_SKB_NETIF_RECEIVE_SKB(RTSKB_SKB(rtskb))== NET_RX_DROP)
	{
		WARNING("[McData] send dummy pkt fail, dev = %s", RTSKB_DEV(rtskb)->name);
	}
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();
*/


//	way3. using netif_rx
//		-binding with linux backlog hacking (we hope do not binding with hacking)
//		-should set priority
	RTK_FC_HOOK_PS_SKB_NETIF_RX_WITH_HOOK(skb);

	return (RTK_FC_RET_OK);
}



int32 rtk_fc_igmp_mcConfig_abstrSwFlow_sendDummy(rtk_fc_table_mcConfigTbl_t* mcConfig)
{
	int32 gorupIpHit=0;
	int32 careSourceAndHit=0;
	int32 careCvidAndHit=0;
	int32 careSvidAndHit=0;
	uint32 groupHash=0,mcExtFlowHash=0;
	rtk_fc_mcExtraSwFlowIdx_entry_t *mcSwFlowId_entry=NULL;
	rtk_fc_mcExtraSwFlowIdx_entry_t *mcTmpSwFlowId_entry=NULL;

	if(mcConfig->isIpv6)
		groupHash=_rtk_rg_flowHashIPv6DstAddr_get((uint8*)&mcConfig->confGroup[0]);
	else
		groupHash=mcConfig->confGroup[0];

	mcExtFlowHash=_rtk_fc_mcflowHash(groupHash);


	list_for_each_entry_safe(mcSwFlowId_entry,mcTmpSwFlowId_entry,&fc_db.listHead_mcExtraFlowIdxHashTbl[mcExtFlowHash],entry)
	{
		gorupIpHit=0;careSourceAndHit=0;careCvidAndHit=0;careSvidAndHit=0;

		if(mcConfig->dftGrpFwd != mcSwFlowId_entry->isGrpDftEntry)
			continue;

		if(mcConfig->isIpv6==mcSwFlowId_entry->isIpv6 && memcmp(mcSwFlowId_entry->multicastAddress,mcConfig->confGroup,sizeof(mcSwFlowId_entry->multicastAddress))==0)
			gorupIpHit=1;

		if(mcConfig->careSource)
		{
			if(mcConfig->isIpv6==mcSwFlowId_entry->isIpv6 && (memcmp(mcSwFlowId_entry->sourceAddress,mcConfig->confSource,sizeof(mcSwFlowId_entry->sourceAddress))==0))
				careSourceAndHit=1;
		}
		else
			careSourceAndHit=1; // treat as hit

		if(mcConfig->careIngressCvid)
		{
			if(( mcConfig->confIgrCvid == mcSwFlowId_entry->ingressCvid) )
				careCvidAndHit=1;
		}
		else
			careCvidAndHit=1;  // treat as hit

		if(mcConfig->careIngressSvid)
		{
			if(( mcConfig->confIgrSvid == mcSwFlowId_entry->ingressSvid) )
				careSvidAndHit=1;
		}
		else
			careSvidAndHit=1;  // treat as hit



		//IGMP("[%d]swFlowIdx[%d] gorupIpHit:%d careSourceAndHit:%d careCvidAndHit:%d",mcSwFlowId_entry->entryIdx,mcSwFlowId_entry->swFlowIdx,gorupIpHit,careSourceAndHit,careCvidAndHit);
		IGMP("swFlowIdx[%d] gorupIpHit:%d careSourceAndHit:%d careCvidAndHit:%d careSvidAndHit:%d",mcSwFlowId_entry->swFlowIdx,gorupIpHit,careSourceAndHit,careCvidAndHit,careSvidAndHit);			
		if(gorupIpHit && careSourceAndHit && careCvidAndHit && careSvidAndHit)
		{
			//send dummy packet by flow for add port purpose
			IGMP("send dummy packet");
			rtk_fc_abstrSwflow_sendMcDummyPkt(mcSwFlowId_entry);
		}
		
	}


	return SUCCESS;

}


bool rtk_fc_igmp_ckeckDmacTransIsSame(rtk_fc_mcPort_t* pConfigMcPort,rtk_fc_abstrSwFlowLdpid_entry_t *pLdpid)
{
	
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)	
{
	int16 lutIdx;
	IGMP("pConfigMcPort->dmacTrans:%d g3PortPriv.swMacIdEn:%d",pConfigMcPort->dmacTrans,pLdpid->g3PortPriv.swMacIdEn);
	if(pConfigMcPort->dmacTrans==1    && pLdpid->g3PortPriv.swMacIdEn==1 )
	{
		if(_rtk_fc_lut_find(pConfigMcPort->dmac,&lutIdx)==RTK_FC_RET_OK)
		{
			if(fc_db.lutTbl[lutIdx]->wifiMacId ==pLdpid->g3PortPriv.swMacId)
				return TRUE;
		}
	}
	else if(pConfigMcPort->dmacTrans==0    && pLdpid->g3PortPriv.swMacIdEn==0 )
		return TRUE;

	//default return FALSE
	return FALSE;
}
#endif
#endif
	return TRUE;
}


int32 rtk_fc_multicast_flowFlush_strategy  (rtk_fc_multicast_flow_flush_t strategy  ,rtk_fc_multicast_flush_patten *pPatten)
{
	int i;
	rtk_fc_abstrSwFlowList_entry_t *pAbstrSwFlowEt=NULL;
	rtk_fc_mcExtraSwFlowIdx_entry_t *mcSwFlowId_entry=NULL;
	rtk_fc_mcExtraSwFlowIdx_entry_t *mcTmpSwFlowId_entry=NULL;
	rtk_fc_abstrSwFlowActionList_entry_t *pSwFlowAction=NULL;
	rtk_fc_abstrSwFlowActionList_entry_t *pTmpSwFlowAction=NULL;


	if(strategy==MC_FLWO_FLUSH_ALL)
	{
		IGMP("Flush ALL MC FLOW");
		for (i =0 ; i< RTK_FC_TABLESIZE_MCFLOW_HASH; i++)
		{
			list_for_each_entry_safe(mcSwFlowId_entry,mcTmpSwFlowId_entry,&fc_db.listHead_mcExtraFlowIdxHashTbl[i],entry)
			{
					rtk_fc_flow_delete(mcSwFlowId_entry->swFlowIdx);
			}
		}	

	}
	else if(strategy==MC_FLWO_FLUSH_BY_GROUP)
	{
		if(pPatten->isIpv6)
			IGMP("Flush MC By Group %pI6",pPatten->gip);
		else
			IGMP("Flush MC By Group %pI4h",pPatten->gip);
		for (i =0 ; i< RTK_FC_TABLESIZE_MCFLOW_HASH; i++)
		{				
			list_for_each_entry_safe(mcSwFlowId_entry,mcTmpSwFlowId_entry,&fc_db.listHead_mcExtraFlowIdxHashTbl[i],entry)
			{
				if(memcmp(mcSwFlowId_entry->multicastAddress,pPatten->gip,sizeof(mcSwFlowId_entry->multicastAddress))==0 && mcSwFlowId_entry->isIpv6==pPatten->isIpv6)
				{
					rtk_fc_flow_delete(mcSwFlowId_entry->swFlowIdx);
				}
			}
		}

	}
	else if(strategy==MC_FLWO_FLUSH_BY_DMAC_TRANS_ACTION)
	{
		int flowDoDamcTrans=0;
		IGMP("Flush MC By DmacTrans action");
		for (i =0 ; i< RTK_FC_TABLESIZE_MCFLOW_HASH; i++)
		{
			flowDoDamcTrans=0;
			list_for_each_entry_safe(mcSwFlowId_entry,mcTmpSwFlowId_entry,&fc_db.listHead_mcExtraFlowIdxHashTbl[i],entry)
			{
				pAbstrSwFlowEt=fc_db.flowTbl[mcSwFlowId_entry->swFlowIdx].pAbstrSwFlowEt;
				list_for_each_entry_safe(pSwFlowAction,pTmpSwFlowAction, &pAbstrSwFlowEt->swFlowActionHdr, swFlowActionList)
				{
					if(pSwFlowAction->swFlowAction.bits.dmacTrans)
					{
						flowDoDamcTrans=1;
						break;
					}
				}
				if(flowDoDamcTrans)
					rtk_fc_flow_delete(mcSwFlowId_entry->swFlowIdx);
			}
		}
	}
	return SUCCESS;

}

//just remove leave port in swflow and hw, if join we need send dummy packet to add swflow action
int32 rtk_fc_igmp_applyMcConfig_abstrSwFlow(rtk_fc_table_mcConfigTbl_t* mcConfig)
{
	int j;
	int32 gorupIpHit=0;
	int32 careSourceAndHit=0;
	int32 careCvidAndHit=0;
	int32 careSvidAndHit=0;
	rtk_fc_abstrSwFlowList_entry_t *pAbstrSwFlowEt=NULL;
	rtk_fc_mcExtraSwFlowIdx_entry_t *mcSwFlowId_entry=NULL;
	rtk_fc_mcExtraSwFlowIdx_entry_t *mcTmpSwFlowId_entry=NULL;
	rtk_fc_abstrSwFlowActionList_entry_t *pSwFlowAction=NULL;
	rtk_fc_abstrSwFlowActionList_entry_t *pTmpSwFlowAction=NULL;
	rtk_fc_abstrSwFlowLdpid_entry_t *pLdpid=NULL;
	rtk_fc_abstrSwFlowLdpid_entry_t *pTmpLdpid=NULL;
	uint32 needAddDev=0;
	uint32 needDelDev=0;
	uint32 copy2cpuChange=0;
	uint32 groupHash=0,mcExtFlowHash=0;
	uint8 hitSame=0;
	uint8 abstrFlowExtistPortCnt=0;

	if(mcConfig->isIpv6)
		groupHash=_rtk_rg_flowHashIPv6DstAddr_get((uint8*)&mcConfig->confGroup[0]);
	else
		groupHash=mcConfig->confGroup[0];

	mcExtFlowHash=_rtk_fc_mcflowHash(groupHash);	


	list_for_each_entry_safe(mcSwFlowId_entry,mcTmpSwFlowId_entry,&fc_db.listHead_mcExtraFlowIdxHashTbl[mcExtFlowHash],entry)
	{
		needAddDev=0;needDelDev=0;copy2cpuChange=0;abstrFlowExtistPortCnt=0;
		gorupIpHit=0;careSourceAndHit=0;careCvidAndHit=0;careSvidAndHit=0;
		pAbstrSwFlowEt=fc_db.flowTbl[mcSwFlowId_entry->swFlowIdx].pAbstrSwFlowEt;

		if(mcConfig->dftGrpFwd != mcSwFlowId_entry->isGrpDftEntry)
			continue;

		/* may we */
		if(mcConfig->isIpv6==mcSwFlowId_entry->isIpv6 && memcmp(mcSwFlowId_entry->multicastAddress,mcConfig->confGroup,sizeof(mcSwFlowId_entry->multicastAddress))==0)
			gorupIpHit=1;

		if(mcConfig->careSource)
		{
			if(mcConfig->isIpv6==mcSwFlowId_entry->isIpv6 && (memcmp(mcSwFlowId_entry->sourceAddress,mcConfig->confSource,sizeof(mcSwFlowId_entry->sourceAddress))==0))
				careSourceAndHit=1;
		}
		else
			careSourceAndHit=1; // treat as hit

		if(mcConfig->careIngressCvid)
		{
			if(( mcConfig->confIgrCvid == mcSwFlowId_entry->ingressCvid) )
				careCvidAndHit=1;
		}
		else
			careCvidAndHit=1;  // treat as hit

		if(mcConfig->careIngressSvid)
		{
			if(( mcConfig->confIgrSvid == mcSwFlowId_entry->ingressSvid) )
				careSvidAndHit=1;
		}
		else
			careSvidAndHit=1;  // treat as hit

		//IGMP("[%d]swFlowIdx[%d] gorupIpHit:%d careSourceAndHit:%d careCvidAndHit:%d",mcSwFlowId_entry->entryIdx,mcSwFlowId_entry->swFlowIdx,gorupIpHit,careSourceAndHit,careCvidAndHit);
		IGMP("swFlowIdx[%d] gorupIpHit:%d careSourceAndHit:%d careCvidAndHit:%d careSvidAndHit:%d",mcSwFlowId_entry->swFlowIdx,gorupIpHit,careSourceAndHit,careCvidAndHit,careSvidAndHit);

		if(gorupIpHit && careSourceAndHit && careCvidAndHit && careSvidAndHit)
		{
			list_for_each_entry_safe(pSwFlowAction,pTmpSwFlowAction, &pAbstrSwFlowEt->swFlowActionHdr, swFlowActionList)
			{
				list_for_each_entry_safe(pLdpid,pTmpLdpid, &pSwFlowAction->ldpidListHdr, ldpidList)
				{
					hitSame=0;
					for(j=0;j< mcConfig->cnt_egrInfo;j++)
					{

						if(mcConfig->egrInfo[j].ifidx==FAIL)
						{
							if(pLdpid->isWlan==mcConfig->egrInfo[j].isWlan &&
							   pLdpid->flowLdpid == mcConfig->egrInfo[j].portId &&
							   rtk_fc_igmp_ckeckDmacTransIsSame(&mcConfig->egrInfo[j],pLdpid))
							{
								IGMP("IGMP exist ignore ifidx:%d port(%d) ",pLdpid->devIfIdx,pLdpid->flowLdpid);
								hitSame=1;
								break;
							}
						}
						else
						{
							if(pLdpid->devIfIdx == mcConfig->egrInfo[j].ifidx &&
								rtk_fc_igmp_ckeckDmacTransIsSame(&mcConfig->egrInfo[j],pLdpid))
							{
								IGMP("IGMP exist ignore ifidx:%d port(%d) ",pLdpid->devIfIdx,pLdpid->flowLdpid);
								hitSame=1;
								break;
							}
						}
					}
					if(!hitSame)
					{
						//entry not found in new config , we delete this port
						IGMP("IGMP remove ifidx:%d port(%d) ",pLdpid->devIfIdx,pLdpid->flowLdpid);
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)		
						if(pLdpid->g3PortPriv.hwFibHashAcc)
						{
							IGMP("delete hwFibHashIdx flow :%d",pLdpid->g3PortPriv.hwFibHashIdx);
							rtk_fc_flow_delete(pLdpid->g3PortPriv.hwFibHashIdx);
							pLdpid->g3PortPriv.hwFibHashAcc=0;
						}
						if(pLdpid->g3PortPriv.swMacIdEn)
						{
							rtk_fc_delete_mc_macId(pLdpid->g3PortPriv.swMacId);
							pLdpid->g3PortPriv.swMacIdEn=0;
						}						
#endif								
						rtk_fc_abstrSwFlowLdpidFree(pLdpid);
						needDelDev=1;
					}
					else
						abstrFlowExtistPortCnt++;
				}

				if(list_empty(&pSwFlowAction->ldpidListHdr))
				{
					//this ation no-more ports delete action
					IGMP("delete pSwFlowAction");
					rtk_fc_abstrSwFlowActionFree(pSwFlowAction);
				}

			}

			//after delete if counter not match mean needAddDev
			if(abstrFlowExtistPortCnt!=mcConfig->cnt_egrInfo)
				needAddDev=1;
				
			if(mcConfig->copy2cpu!=pAbstrSwFlowEt->copy2cpu)
			{	
				pAbstrSwFlowEt->copy2cpu = mcConfig->copy2cpu;
				copy2cpuChange=1;
			}

			if(needDelDev || copy2cpuChange)
			{
				//if Action list empty delete flow 
				if(list_empty(&pAbstrSwFlowEt->swFlowActionHdr))
				{
					IGMP("delete swFlowIdx=%d",mcSwFlowId_entry->swFlowIdx);
					rtk_fc_flow_delete(mcSwFlowId_entry->swFlowIdx);
				}
				else 
				{
					// any action here update to hardware
					if(!FLOW_INFO_IS_SET(&fc_db.flowTbl[mcSwFlowId_entry->swFlowIdx], FLOW_INFO_SOFTWARE_ONLY))
						rtk_fc_abstrSwSyncToHw_update(mcSwFlowId_entry->swFlowIdx);
				}
				
			}


			if(needAddDev)
			{
				//send dummy packet by flow for add port purpose
				IGMP("send dummy packet");
				rtk_fc_abstrSwflow_sendMcDummyPkt(mcSwFlowId_entry);
				mcSwFlowId_entry->dummySyncTimes=0;
				rtk_fc_igmp_setupmcDummyPktDetectorTimer();
			}
		
		}
		
	}


	return SUCCESS;

}


int32 _rtk_fc_igmp_mc_get_devConfig(rt_igmp_group_devPort_cfg_t *mcConfig)
{
	uint8 isipv6 = mcConfig->is_ipv6;
	uint32 *gip = NULL;
	uint32 *sip = NULL;
	int32 ingress_cvid = FAIL;
	int32 ingress_svid = FAIL;
	int i;

	gip = (uint32*)&(mcConfig->group_addr.ipv6[0]);
	if(gip[0]==0 && gip[1]==0 && gip[2]==0 && gip[3]==0)
		gip=NULL;
	if(mcConfig->careSourceAddress)
		sip = (uint32*)&(mcConfig->source_addr.ipv6[0]);

	if(mcConfig->ingress_ctagif)
		ingress_cvid=mcConfig->ingress_cvid;
	if(mcConfig->ingress_stagif)
		ingress_svid=mcConfig->ingress_svid;


	if(mcConfig->is_ipv6)
	{
		IGMP("get groupBehavior:%d is_ipv6=%d cntEgrInfo=%d careSip/careCvid/careSvid=[%d/%d/%d] group_addr=%pI6 source_addr=%pI6 tagif[%d]ingressCVid=%d tagif[%d]ingressSVid=%d"
			,mcConfig->groupBehavior,mcConfig->is_ipv6,mcConfig->cntEgrInfo,mcConfig->careSourceAddress,mcConfig->careIngressCvid,mcConfig->careIngressSvid,&mcConfig->group_addr.ipv6[0],&mcConfig->source_addr.ipv6[0],mcConfig->ingress_ctagif,mcConfig->ingress_cvid,mcConfig->ingress_stagif,mcConfig->ingress_svid);		
	}
	else
	{
		IGMP("get groupBehavior:%d is_ipv6=%d cntEgrInfo=%d careSip/careVid/careSvid=[%d/%d/%d] group_addr=%pI4h source_addr=%pI4h tagif[%d]ingressCVid=%d tagif[%d]ingressSVid=%d"
			,mcConfig->groupBehavior,mcConfig->is_ipv6,mcConfig->cntEgrInfo,mcConfig->careSourceAddress,mcConfig->careIngressCvid,mcConfig->careIngressSvid,&mcConfig->group_addr.ipv4,&mcConfig->source_addr.ipv4,mcConfig->ingress_ctagif,mcConfig->ingress_cvid,mcConfig->ingress_stagif,mcConfig->ingress_svid);
	}

	if(gip && ((mcConfig->groupBehavior==RT_MC_BEHAVIOR_FLOW_FORWAED) || (mcConfig->groupBehavior==RT_MC_BEHAVIOR_GROUP_DEFAULT_FWD)))
	{
		rtk_fc_table_mcConfigTbl_t *mcSetEntry;

		if(mcConfig->groupBehavior==RT_MC_BEHAVIOR_FLOW_FORWAED)
			mcSetEntry = rtk_fc_mc_getMcConfig(isipv6,gip,mcConfig->careSourceAddress,sip,mcConfig->careIngressCvid,ingress_cvid,mcConfig->careIngressSvid,ingress_svid);
		else if(mcConfig->groupBehavior==RT_MC_BEHAVIOR_GROUP_DEFAULT_FWD)
			mcSetEntry = rtk_fc_mc_getDftMcConfig(isipv6,gip,mcConfig->careIngressCvid,ingress_cvid,mcConfig->careIngressSvid,ingress_svid);
			
		if(mcSetEntry == NULL)
			return FAIL;

		for(i=0;i<mcSetEntry->cnt_egrInfo;i++)
		{
			if(mcConfig->egrInfoType==EGRINFO_PID)
			{
				mcConfig->egrInfo.portId[i].isWlan=mcSetEntry->egrInfo[i].isWlan;
				if(mcConfig->egrInfo.portId[i].isWlan)
				{

					rt_wlan_mbssid_index_t mbssidIdx;
					rt_wlan_index_t wlanIdx;
					rtk_fc_wlan_devmask_t wlanDevIdMask;
					rt_wlan_mbssid_mask_t wlanMbssidMask[RT_WLAN_DEVICE_MAX];
					wlanDevIdMask = ((rtk_fc_wlan_devmask_t)1) << mcSetEntry->egrInfo[i].portId;
					rtk_fc_wlan_devMask_to_rtmbssidMask(&wlanDevIdMask,wlanMbssidMask);
					for(wlanIdx=0 ; wlanIdx<RT_WLAN_DEVICE_MAX ; wlanIdx++)
					{
						for(mbssidIdx=0;mbssidIdx<RT_WLAN_MBSSID_MAX;mbssidIdx++)
						{
							if(wlanMbssidMask[wlanIdx] & (1<<mbssidIdx))
							{
								mcConfig->egrInfo.portId[i].wlan.wlanDevIdx = wlanIdx;
								mcConfig->egrInfo.portId[i].wlan.wlanMbssid = mbssidIdx;
							}
						}
					}
				}
				else
				{
					mcConfig->egrInfo.portId[i].phyPortIdx=mcSetEntry->egrInfo[i].portId;
				}
			}
			else
			{
				mcConfig->egrInfo.devIfidx[i].ifindex=mcSetEntry->egrInfo[i].ifidx;
			}

		}
		mcConfig->cntEgrInfo = mcSetEntry->cnt_egrInfo;
	}

	return FAILED;
}

#define REALTEK_MAGIC_NUMBER 0xB7
rtk_fc_table_mcConfigTbl_t * _rtk_fc_igmp_user2InternalConfigTrasnfer(rtk_fc_table_mcConfigTbl_t *mcConfigInternal,rt_igmp_group_devPort_cfg_t *mcConfigUser)
{
	int32 ingress_cvid = FAIL;
	int32 ingress_svid = FAIL;
	rtk_fc_table_mcConfigTbl_t *p_mcSetInternal;
	uint32 *gip = NULL;
	uint32 *sip = NULL;

	if(mcConfigUser->ingress_ctagif)
		ingress_cvid=mcConfigUser->ingress_cvid;
	
	if(mcConfigUser->ingress_stagif)
		ingress_svid=mcConfigUser->ingress_svid;
	
	gip = (uint32*)&(mcConfigUser->group_addr.ipv6[0]);
	if(mcConfigUser->careSourceAddress)
		sip = (uint32*)&(mcConfigUser->source_addr.ipv6[0]);
	
	if(RTK_FC_HELPER_IGMP_IS_MDB_REG())
	{
		p_mcSetInternal = mcConfigInternal;
		memset(p_mcSetInternal,0,sizeof(*p_mcSetInternal));

		p_mcSetInternal->isIpv6 = mcConfigUser->is_ipv6;
		memcpy(p_mcSetInternal->confGroup,gip,sizeof(p_mcSetInternal->confGroup));

		p_mcSetInternal->careIngressCvid = mcConfigUser->careIngressCvid;
		p_mcSetInternal->confIgrCvid = ingress_cvid;
		
		p_mcSetInternal->careIngressSvid = mcConfigUser->careIngressSvid;
		p_mcSetInternal->confIgrSvid = ingress_svid;

		p_mcSetInternal->careSource = mcConfigUser->careSourceAddress;
		if(mcConfigUser->careSourceAddress)
			memcpy(p_mcSetInternal->confSource,sip,sizeof(p_mcSetInternal->confSource));
	
		//assign by outside
		p_mcSetInternal->cnt_egrInfo = mcConfigUser->cntEgrInfo;
		p_mcSetInternal->copy2cpu = mcConfigUser->copy2cpu;
		//p_mcSetInternal->egrInfo

		//non-use just init
		p_mcSetInternal->do_delete = 0;
		p_mcSetInternal->dftGrpFwd = 0;
		INIT_LIST_HEAD(&p_mcSetInternal->lsMcConfigEntry);
	}
	else
	{
		if((mcConfigUser->groupBehavior==RT_MC_BEHAVIOR_FLOW_FORWAED))
		{
			p_mcSetInternal = rtk_fc_mc_getMcConfig(mcConfigUser->is_ipv6,gip,mcConfigUser->careSourceAddress,sip,mcConfigUser->careIngressCvid,ingress_cvid,mcConfigUser->careIngressSvid,ingress_svid);
			if(p_mcSetInternal == NULL)
			{
				if(mcConfigUser->cntEgrInfo!=0)
				{
					//new a entry
					p_mcSetInternal = rtk_fc_mc_mallocMcConfig(mcConfigUser->is_ipv6,gip,mcConfigUser->careSourceAddress,sip,mcConfigUser->careIngressCvid,ingress_cvid,mcConfigUser->careIngressSvid,ingress_svid);				
				}
				else
				{
					IGMP("delete a non-exist entry do nothing and return");
					return NULL;
				}
			}
		}
		else if(mcConfigUser->groupBehavior==RT_MC_BEHAVIOR_GROUP_DEFAULT_FWD)
		{
			p_mcSetInternal = rtk_fc_mc_getDftMcConfig(mcConfigUser->is_ipv6,gip,mcConfigUser->careIngressCvid,ingress_cvid,mcConfigUser->careIngressSvid,ingress_svid);
			if(p_mcSetInternal == NULL)
			{
				if(mcConfigUser->cntEgrInfo!=0)
				{
					//new a entry
					p_mcSetInternal = rtk_fc_mc_mallocMcGrpDefaultConfig(mcConfigUser->is_ipv6,gip,mcConfigUser->careIngressCvid,ingress_cvid,mcConfigUser->careIngressSvid,ingress_svid);				
				}
				else
				{
					IGMP("delete a non-exist Dft entry do nothing and return");
					return NULL;
				}
			}
		}
		else
		{
			p_mcSetInternal = rtk_fc_mc_searchAllMcConfig(mcConfigUser->is_ipv6,gip,mcConfigUser->careSourceAddress,sip,mcConfigUser->careIngressCvid,ingress_cvid,mcConfigUser->careIngressSvid,ingress_svid);

		}
	}

	return p_mcSetInternal;

}


int32 _rtk_fc_igmp_mc_set_dev_to_dirver(rt_igmp_group_devPort_cfg_t *mcConfigUser)
{
	uint8 isipv6 = mcConfigUser->is_ipv6;
	uint32 *gip = NULL;
	uint32 *sip = NULL;
	int32 ingress_cvid = FAIL;
	int32 ingress_svid = FAIL;
	int j;
	struct net_device *dev=NULL;
	rtk_fc_realdev_t rdev;
	rtk_fc_wlan_devidx_t wlanIdx=RTK_FC_WLANX_NOT_FOUND;	
	rtk_fc_table_mcConfigTbl_t *p_mcSetInternal;
	rtk_fc_table_mcConfigTbl_t mcSetInternal;
	rtk_fc_multicast_flush_patten patten;


	if(mcConfigUser->cntEgrInfo >= RT_MCCFG_MAX_NUM){WARNING("Error Parameter cntEgrInfo(%d)>=%d",mcConfigUser->cntEgrInfo,RT_MCCFG_MAX_NUM);}

	gip = (uint32*)&(mcConfigUser->group_addr.ipv6[0]);
	if(gip[0]==0 && gip[1]==0 && gip[2]==0 && gip[3]==0)
		gip=NULL;
	if(mcConfigUser->careSourceAddress)
		sip = (uint32*)&(mcConfigUser->source_addr.ipv6[0]);

	if(mcConfigUser->ingress_ctagif)
		ingress_cvid=mcConfigUser->ingress_cvid;
	if(mcConfigUser->ingress_stagif)
		ingress_svid=mcConfigUser->ingress_svid;


	if(mcConfigUser->is_ipv6)
	{
		IGMP("set groupBehavior:%d is_ipv6=%d cntEgrInfo=%d careSip/careCVid/careSVid=[%d/%d/%d] group_addr=%pI6 source_addr=%pI6 tagif[%d]ingressVid=%d tagif[%d]ingressVid=%d copy2cou=%d"
			,mcConfigUser->groupBehavior,mcConfigUser->is_ipv6,mcConfigUser->cntEgrInfo,mcConfigUser->careSourceAddress,mcConfigUser->careIngressCvid,mcConfigUser->careIngressSvid,&mcConfigUser->group_addr.ipv6[0],&mcConfigUser->source_addr.ipv6[0],mcConfigUser->ingress_ctagif,mcConfigUser->ingress_cvid,mcConfigUser->ingress_stagif,mcConfigUser->ingress_svid,mcConfigUser->copy2cpu);		
	}
	else
	{
		IGMP("set groupBehavior:%d is_ipv6=%d cntEgrInfo=%d careSip/careCVid/careSVid=[%d/%d/%d] group_addr=%pI4h source_addr=%pI4h tagif[%d]ingressVid=%d tagif[%d]ingressVid=%d copy2cpu=%d"
			,mcConfigUser->groupBehavior,mcConfigUser->is_ipv6,mcConfigUser->cntEgrInfo,mcConfigUser->careSourceAddress,mcConfigUser->careIngressCvid,mcConfigUser->careIngressSvid,&mcConfigUser->group_addr.ipv4,&mcConfigUser->source_addr.ipv4,mcConfigUser->ingress_ctagif,mcConfigUser->ingress_cvid,mcConfigUser->ingress_stagif,mcConfigUser->ingress_svid,mcConfigUser->copy2cpu);
	}

	//just printk debug info
	for(j=0;j<mcConfigUser->cntEgrInfo;j++)
	{
		if(mcConfigUser->egrInfoType==EGRINFO_DEVIFIDX)
		{
			if( (fc_db.mcSetMode==RT_IGMP_MULTICAST_SYNC_AUTO && mcConfigUser->egrInfo.devIfidx[j].resv0 != REALTEK_MAGIC_NUMBER) ||
				(fc_db.mcSetMode==RT_IGMP_MULTICAST_SYNC_MODE_USER && mcConfigUser->egrInfo.devIfidx[j].resv0 == REALTEK_MAGIC_NUMBER)) 
			{ WARNING(" mcSetMode=%d should only use one igmp module in system",fc_db.mcSetMode);}
			RTK_FC_HOOK_PS_DEV_GET_BY_INDEX(&init_net,mcConfigUser->egrInfo.devIfidx[j].ifindex,&dev);
			if(dev)
			{
				if(RTK_FC_HOOK_PS_DEV_GET_REALDEV_PHYPORT(dev,&rdev)!=FAIL)
					IGMP("\t %s[Ifidx:%d]=>[Port:%d] DmacTrans:%d dmac:%pM",dev->name,mcConfigUser->egrInfo.devIfidx[j].ifindex,rdev.physicalPid,mcConfigUser->egrInfo.devIfidx[j].dmacTrans,mcConfigUser->egrInfo.devIfidx[j].dmac);
				else if(RTK_FC_HELPER_WLAN_DEV_TO_DEVID(dev,&wlanIdx)!=FAIL)
					IGMP("\t %s[Ifidx:%d]=>[WlanIdx:%d] DmacTrans:%d dmac:%pM",dev->name,mcConfigUser->egrInfo.devIfidx[j].ifindex,wlanIdx,mcConfigUser->egrInfo.devIfidx[j].dmacTrans,mcConfigUser->egrInfo.devIfidx[j].dmac);
			}
		}
		else
		{
			if( (fc_db.mcSetMode==RT_IGMP_MULTICAST_SYNC_AUTO && mcConfigUser->egrInfo.portId[j].resv0 != REALTEK_MAGIC_NUMBER) ||
				(fc_db.mcSetMode==RT_IGMP_MULTICAST_SYNC_MODE_USER && mcConfigUser->egrInfo.portId[j].resv0 == REALTEK_MAGIC_NUMBER)) 
			{ WARNING(" mcSetMode=%d should only use one igmp module in system",fc_db.mcSetMode);}			
			if(mcConfigUser->egrInfo.portId[j].isWlan)
				IGMP("\t [WLANPort][DEVIDX=%d][MBSSID=%d]",mcConfigUser->egrInfo.portId[j].wlan.wlanDevIdx,mcConfigUser->egrInfo.portId[j].wlan.wlanMbssid);
			else
				IGMP("\t [PHYPort:%d]",mcConfigUser->egrInfo.portId[j].phyPortIdx);
		}
	}		


	if(gip)
	{
		if((mcConfigUser->groupBehavior==RT_MC_BEHAVIOR_FLOW_FORWAED) || (mcConfigUser->groupBehavior==RT_MC_BEHAVIOR_GROUP_DEFAULT_FWD))
		{
			rtk_fc_table_mcConfigTbl_t *p_mcSetInternal;
			rtk_fc_table_mcConfigTbl_t mcSetInternal;

			p_mcSetInternal = _rtk_fc_igmp_user2InternalConfigTrasnfer(&mcSetInternal,mcConfigUser);

			if(p_mcSetInternal == NULL)
				return FAILED;

			//if kernel mode do not delete this entry
			p_mcSetInternal->do_delete=0;
			
			//assign Ports info from Userconfig to Internal config
			p_mcSetInternal->cnt_egrInfo = mcConfigUser->cntEgrInfo;
			
			for(j=0;j<mcConfigUser->cntEgrInfo;j++)
			{

				if(mcConfigUser->egrInfoType==EGRINFO_PID)
				{
					if(!mcConfigUser->egrInfo.portId[j].isWlan)
					{
						//phyPort
						p_mcSetInternal->egrInfo[j].portId = mcConfigUser->egrInfo.portId[j].phyPortIdx;
						p_mcSetInternal->egrInfo[j].isWlan=0;
					}
					else
					{
						//wlan
						p_mcSetInternal->egrInfo[j].portId = rtk_fc_wlan_rtmbssid_to_devidx(mcConfigUser->egrInfo.portId[j].wlan.wlanDevIdx,mcConfigUser->egrInfo.portId[j].wlan.wlanMbssid);
						p_mcSetInternal->egrInfo[j].isWlan=1;

					}
					p_mcSetInternal->egrInfo[j].ifidx=FAIL;
					p_mcSetInternal->egrInfo[j].dmacTrans = mcConfigUser->egrInfo.portId[j].dmacTrans;
					if(p_mcSetInternal->egrInfo[j].dmacTrans)
						memcpy(p_mcSetInternal->egrInfo[j].dmac,mcConfigUser->egrInfo.portId[j].dmac,6);
				}
				else
				{
					p_mcSetInternal->egrInfo[j].ifidx=mcConfigUser->egrInfo.devIfidx[j].ifindex;
					p_mcSetInternal->egrInfo[j].dmacTrans = mcConfigUser->egrInfo.devIfidx[j].dmacTrans;
					if(p_mcSetInternal->egrInfo[j].dmacTrans)
						memcpy(p_mcSetInternal->egrInfo[j].dmac,mcConfigUser->egrInfo.devIfidx[j].dmac,6);					
					RTK_FC_HOOK_PS_DEV_GET_BY_INDEX(&init_net,mcConfigUser->egrInfo.devIfidx[j].ifindex,&dev);
					if(dev)
					{
						if(RTK_FC_HOOK_PS_DEV_GET_REALDEV_PHYPORT(dev,&rdev)!=FAIL)
						{
							//phyPort
							p_mcSetInternal->egrInfo[j].portId = rdev.physicalPid;
						}
						else if(RTK_FC_HELPER_WLAN_DEV_TO_DEVID(dev,&wlanIdx)!=FAIL)
						{
							//wlan
							p_mcSetInternal->egrInfo[j].portId = wlanIdx;
						}
					}
				}
			}
			
			p_mcSetInternal->copy2cpu = mcConfigUser->copy2cpu;


			rtk_fc_igmp_applyMcConfig_abstrSwFlow(p_mcSetInternal);

			if(!RTK_FC_HELPER_IGMP_IS_MDB_REG() && mcConfigUser->cntEgrInfo==0)
				rtk_fc_mc_delMcConfig(p_mcSetInternal);


		}
		else if(mcConfigUser->groupBehavior==RT_MC_BEHAVIOR_GROUP_AS_KNOW)
		{
			if(rtk_fc_mc_addKnownGroupEntry(isipv6,gip)==NULL)
			{
				WARNING("IGMP Table FULL");
				return FAILED;
			}			
		}
		else if(mcConfigUser->groupBehavior==RT_MC_BEHAVIOR_GROUP_AS_UNKNOW)
		{
			//delete  knowGroup
			rtk_fc_table_mcGroupTbl_t *groupTbl=rtk_fc_mc_findKnownGroupEntry(isipv6,gip);
			if(groupTbl)
			{
				//we need flush flow table for this group (snooping disable and igmpporxy enable  )
				if(mcConfigUser->copy2cpu==0)
				{
					bzero(&patten,sizeof(patten));
					patten.isIpv6 = isipv6;
					if(isipv6)
						memcpy(patten.gip,gip,sizeof(patten.gip));
					else
						patten.gip[0]=*gip;
					rtk_fc_multicast_flowFlush_strategy(MC_FLWO_FLUSH_BY_GROUP,&patten);
				}
				rtk_fc_mc_delKnownGroupEntry(groupTbl);
			}
		
		}
		else if(mcConfigUser->groupBehavior==RT_MC_CHECK_UNKNOWN_FLOOD_FLOW)
		{
			p_mcSetInternal = _rtk_fc_igmp_user2InternalConfigTrasnfer(&mcSetInternal,mcConfigUser);
			if(p_mcSetInternal == NULL)
				return FAIL;
				
			mcConfigUser->floodFlowDelete = rtk_fc_igmp_checkFlow(p_mcSetInternal);
			return SUCCESS; 
		}
		else if(mcConfigUser->groupBehavior==RT_MC_BEHAVIOR_FLOW_UPDATE)
		{
			p_mcSetInternal = _rtk_fc_igmp_user2InternalConfigTrasnfer(&mcSetInternal,mcConfigUser);
			if(p_mcSetInternal == NULL)
				return FAIL;

			rtk_fc_igmp_mcConfig_abstrSwFlow_sendDummy(p_mcSetInternal);
		}
	}
	else
	{

		rtk_fc_table_mcConfigTbl_t *pMcConfig;
		rtk_fc_table_mcConfigTbl_t *pMcConfig_tmp;
		bzero(&patten,sizeof(patten));
		rtk_fc_multicast_flowFlush_strategy(MC_FLWO_FLUSH_ALL,&patten);
		
		list_for_each_entry_safe(pMcConfig,pMcConfig_tmp,&fc_db.listHead_mcCfgTbl,lsMcConfigEntry)
		{
			rtk_fc_mc_delMcConfig(pMcConfig);
		}
		list_for_each_entry_safe(pMcConfig,pMcConfig_tmp,&fc_db.listHead_mcCfgTbGrplDft,lsMcConfigEntry)
		{
			rtk_fc_mc_delMcConfig(pMcConfig);
		}
		
		IGMP("Group NULL Flush Mc abstrsw mc flow Table");
	}
	return SUCCESS;
}



int32 _rtk_fc_igmp_mc_set_to_dirver(rt_igmp_multicast_group_cfg_t *mcConfig)
{
	/*transfer rt_igmp_multicast_group_cfg_t to rt_igmp_group_devPort_cfg_t	and calling  _rtk_fc_igmp_mc_set_dev_to_dirver */
	int i,j;
	int devCnt=0;
	rtk_fc_wlan_devmask_t p3_wlanMask=0,p4_wlanMask=0;

	rt_igmp_group_devPort_cfg_t sw_mcConfig;


	//error check
	if(mcConfig->first_act_portmask & mcConfig->second_act_portmask)
		return FAIL;

	bzero(&sw_mcConfig,sizeof(sw_mcConfig));

	sw_mcConfig.is_ipv6 =  mcConfig->is_ipv6;
	sw_mcConfig.careIngressCvid = mcConfig->careIngressCvid;
	sw_mcConfig.careIngressSvid = mcConfig->careIngressSvid;	
	sw_mcConfig.careSourceAddress = mcConfig->careSourceAddress;
	sw_mcConfig.groupBehavior = mcConfig->groupBehavior;
	sw_mcConfig.ingress_ctagif = mcConfig->ingress_ctagif;
	sw_mcConfig.ingress_cvid =  mcConfig->ingress_cvid;
	sw_mcConfig.ingress_stagif = mcConfig->ingress_stagif;
	sw_mcConfig.ingress_svid =  mcConfig->ingress_svid;

	sw_mcConfig.egrInfoType = EGRINFO_PID;

	memcpy(sw_mcConfig.group_addr.ipv6,mcConfig->group_addr.ipv6,sizeof(mcConfig->group_addr.ipv6));
	memcpy(sw_mcConfig.source_addr.ipv6,mcConfig->source_addr.ipv6,sizeof(mcConfig->source_addr.ipv6));
	
	rtk_fc_wlan_rtmbssidMask_to_devMask(mcConfig->first_act_wlanMbssidMask,&p3_wlanMask);
	rtk_fc_wlan_rtmbssidMask_to_devMask(mcConfig->second_act_wlanMbssidMask,&p4_wlanMask);


	for(i=0;i<31 /*(rt_port_phy_port_mask_t)int32  max 31 bit pmsk */;i++)
	{
		if((1<<i) & mcConfig->first_act_portmask)
		{
			sw_mcConfig.egrInfo.portId[devCnt].isWlan=0;
			sw_mcConfig.egrInfo.portId[devCnt].phyPortIdx = i;
			devCnt++;
		}
		if((1<<i) & mcConfig->second_act_portmask)
		{
			sw_mcConfig.egrInfo.portId[devCnt].isWlan=0;
			sw_mcConfig.egrInfo.portId[devCnt].phyPortIdx = i;
			devCnt++;
		}
	}
	for(i=0;i<RT_WLAN_DEVICE_MAX;i++)
	{

		for(j=0;j<RT_WLAN_MBSSID_MAX;j++)
		{
			if(mcConfig->first_act_wlanMbssidMask[i] & (1<<j))
			{
				sw_mcConfig.egrInfo.portId[devCnt].isWlan=1;
				sw_mcConfig.egrInfo.portId[devCnt].wlan.wlanDevIdx=i;
				sw_mcConfig.egrInfo.portId[devCnt].wlan.wlanMbssid =j;
				devCnt++;
			}
		}
		
	}
	sw_mcConfig.cntEgrInfo = devCnt;

	_rtk_fc_igmp_mc_set_dev_to_dirver(&sw_mcConfig);

	if(sw_mcConfig.groupBehavior==RT_MC_CHECK_UNKNOWN_FLOOD_FLOW)
		mcConfig->floodFlowDelete = sw_mcConfig.floodFlowDelete;
	return SUCCESS;

}



#define UCIGMP_ADD 0
#define UCIGMP_DEL 1

rtk_fc_ret_t _rtk_fc_ucIGMPflowModify_passthroughMc(rtk_fc_pktHdr_t *pPktHdr,rtk_fc_pmap_t  igrPortMap,rtk_fc_wlan_devidx_t igrWlanDevIdx,uint32* group,uint32 addZero_delOne)
{
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)

	int i;
	uint32 groupHash=0;
	rtk_rg_asic_path3_entry_t p3Data;
	uint32 modify_phymsk= 1<< igrPortMap.macPortIdx;
	uint32 modify_extdevMask = (1LL << igrWlanDevIdx);
	rtk_fc_ext_port_mask_t modify_extPmsk=0;
	uint32 ori_extPmsk,ori_phymsk;
	uint32 final_extMask,final_phymsk;
	uint32 p3_oldExtpmskIdx=0,p3_extPmskIdx=0;
	
	//not passthrough unicast multicast
	if(pPktHdr->isMulticast!=0)
		return RTK_FC_RET_OK;

	//bypass search flow if unknow_da (avoid attack igmp packets cause cpu 100%)
	if(pPktHdr->dmacL2Idx==FAIL)
		return RTK_FC_RET_OK;


	for(i=0;i<(RTK_FC_TABLESIZE_HW_FLOW + RTK_FC_MAX_SHORTCUT_FLOW_SIZE);i++)
	{
		if(fc_db.flowTbl[i].pFlowEntry->path3.valid && fc_db.flowTbl[i].pFlowEntry->path3.in_path==FB_PATH_34 )
		{
			ori_extPmsk=0;
			ori_phymsk=0;
			p3_oldExtpmskIdx=0;
			p3_extPmskIdx=0;
			modify_extPmsk=0;

			if(pPktHdr->ip6h && fc_db.flowTbl[i].pFlowEntry->path3.in_ipv4_or_ipv6 ==1)
				groupHash = _rtk_rg_flowHashIPv6DstAddr_get((uint8 *)group);
			else if(pPktHdr->iph && fc_db.flowTbl[i].pFlowEntry->path3.in_ipv4_or_ipv6 ==0 )
				groupHash = group[0];
			else
				continue;
			
			//both v4 & v6
			if(groupHash == fc_db.flowTbl[i].pFlowEntry->path3.in_dst_ipv4_addr)
			{
				memcpy(&p3Data, &fc_db.flowTbl[i].pFlowEntry->path3, sizeof(rtk_rg_asic_path3_entry_t));
				_rtk_fc_sharing_image_flow_structure_convert((rtk_rg_asic_path1_entry_t *)&p3Data);


				if ( igrWlanDevIdx!=RTK_FC_WLANX_NOT_FOUND)
				{
					if(p3Data.out_ext_portmask_idx)
					{
						p3_oldExtpmskIdx = p3Data.out_ext_portmask_idx;
						ori_extPmsk = fc_db.extPortTbl[p3Data.out_ext_portmask_idx].extPortEnt.extpmask;
					}

					RTK_FC_HELPER_WLAN_DEVMASK_TO_EXTPORMASK(modify_extdevMask, &modify_extPmsk);

					if(addZero_delOne==UCIGMP_ADD)
						final_extMask = ori_extPmsk | modify_extPmsk;
					else
						final_extMask = ori_extPmsk & (~modify_extPmsk);

					if(ori_extPmsk != final_extMask )
					{
						IGMP("FLOW[%d] extpmsk 0x%x -> 0x%x by passthrough igmp packet",i,ori_extPmsk,final_extMask);

						if(RTK_RG_ASIC_EXTPORTMASKTABLE_ADD(final_extMask,&p3_extPmskIdx)!=RTK_FC_RET_OK)
						{
							WARNING("RTK_RG_ASIC_EXTPORTMASKTABLE_ADD ret FAILED");
							continue;
						}
						
						p3Data.out_ext_portmask_idx = p3_extPmskIdx;
						
						ASSERT_EQ(RTK_RG_ASIC_FLOWPATH_SET(&i, &p3Data, NULL, FLOW_INFO_IS_SET(&fc_db.flowTbl[i], FLOW_INFO_SOFTWARE_ONLY)), RTK_FC_RET_OK);

						if(RTK_RG_ASIC_EXTPORTMASKTABLE_DEL(p3_oldExtpmskIdx)!=RTK_FC_RET_OK)
						{
							WARNING("RTK_RG_ASIC_EXTPORTMASKTABLE_DEL ret FAILED");
							continue;
						}	
					}
				}
				else if(modify_phymsk)
				{
					ori_phymsk = p3Data.out_portmask;
					
					if(addZero_delOne==UCIGMP_ADD)
						final_phymsk =  ori_phymsk | modify_phymsk;
					else
						final_phymsk =  ori_phymsk & (~modify_phymsk);

					if(ori_phymsk != final_phymsk)
					{
						p3Data.out_portmask = final_phymsk;
						IGMP("FLOW[%d] pmsk 0x%x -> 0x%x by passthrough igmp packet",i,ori_phymsk,final_phymsk);
						ASSERT_EQ(RTK_RG_ASIC_FLOWPATH_SET(&i, &p3Data, NULL, FLOW_INFO_IS_SET(&fc_db.flowTbl[i], FLOW_INFO_SOFTWARE_ONLY)), RTK_FC_RET_OK);
					}
				}
				
			}

		}
	}
#endif

	return RTK_FC_RET_OK;
	
}


//portMask is single port
#define FC_IS_CLASSD_ADDR(ipv4addr)				((((uint32)(ipv4addr)) & 0xf0000000) == 0xe0000000)

rtk_fc_ret_t _rtk_fc_igmp_mld_ingressLan_learning(rtk_fc_pktHdr_t *pPktHdr,rtk_fc_pmap_t  igrPortMap, rtk_fc_wlan_devidx_t igrWlanDevIdx)
{
	struct igmpv3_report *ih;
	struct igmpv3_grec *grec;
	struct mld2_grec *grecmld;
	int i;
	int len=0;
	int numGrp;
	int type;
	__be32 _group[4]={0};
	int16 mcidx=-1;
	uint32 _lanPortMask =0;
	uint16 grec_nsrcs = 0;

	//skip normal packet 
	if(pPktHdr->igmph==NULL && pPktHdr->icmp6h==NULL)
	{
		//IGMP("IGMP Skip non igmph/icmp6h packet");
		return RTK_FC_RET_OK;
	}
	//skip strange packet
	if( (pPktHdr->isFragment) ||
		(pPktHdr->iph && (!FC_IS_CLASSD_ADDR(ntohl(pPktHdr->iph->daddr)))) || 
		(pPktHdr->ip6h && pPktHdr->ip6h->daddr.in6_u.u6_addr8[0]!=0xff)) 
	{
		//IGMP("IGMP strange packet or not MC packet");
		return RTK_FC_RET_OK;
	}


	_lanPortMask = fc_db.lanPortMask.portmask;

	if(pPktHdr->igmph  && ( ((1<<igrPortMap.macPortIdx) &  _lanPortMask) || (igrWlanDevIdx < RTK_FC_WLANX_END_INTF)))
	{

		if(pPktHdr->igmph->type==IGMP_HOST_MEMBERSHIP_REPORT || pPktHdr->igmph->type==IGMPV2_HOST_MEMBERSHIP_REPORT)
		{
			_group[0] = ntohl(pPktHdr->igmph->group);
			if(!FC_IS_CLASSD_ADDR(_group[0])){IGMP("IGMP Error packet");return RTK_FC_RET_LRN_SKIP_IGMP_MLD;}			
			_rtk_fc_ucIGMPflowModify_passthroughMc(pPktHdr,igrPortMap,igrWlanDevIdx,_group,UCIGMP_ADD);
			if(fc_db.mcSetMode==RT_IGMP_MULTICAST_SYNC_MODE_KERNEL)
				rtk_fc_igmp_setupKernelSyncTimerEvent();
			IGMP("receive IGMPv1/IGMPv2 Report packet at LANPort[%d]! WLANDEVID[%d]! Group=%pI4h lutidx=%d",igrPortMap.macPortIdx, igrWlanDevIdx, &_group[0],mcidx);
		}
		else if (pPktHdr->igmph->type==IGMPV3_HOST_MEMBERSHIP_REPORT)
		{
			ih =(struct igmpv3_report *)pPktHdr->igmph;
			numGrp = ntohs(ih->ngrec);
			grec = ih->grec ;
			if(numGrp>8){IGMP("skip large numberGrp search to avoid attack");return RTK_FC_RET_LRN_SKIP_IGMP_MLD;}
			for (i = 0; i < numGrp; i++)
			{
				_group[0] = ntohl(grec->grec_mca);
				type = grec->grec_type;
				grec_nsrcs = ntohs(grec->grec_nsrcs);
				len =  (8 + ntohs(grec->grec_nsrcs)*4 + ntohs(grec->grec_auxwords)*4) ;
				grec = ((void*)grec) +len;

				if(!FC_IS_CLASSD_ADDR(_group[0])){IGMP("IGMP Error packet");return RTK_FC_RET_LRN_SKIP_IGMP_MLD;}			
				if(grec_nsrcs>8){IGMP("skip large grec_nsrcs search to avoid attack");return RTK_FC_RET_LRN_SKIP_IGMP_MLD;}

				if ((type == IGMPV3_CHANGE_TO_INCLUDE ||
					type == IGMPV3_MODE_IS_INCLUDE) &&
					grec_nsrcs == 0)
				{
					_rtk_fc_ucIGMPflowModify_passthroughMc(pPktHdr,igrPortMap,igrWlanDevIdx,_group,UCIGMP_DEL);
					if(fc_db.mcSetMode==RT_IGMP_MULTICAST_SYNC_MODE_KERNEL)
						rtk_fc_igmp_setupKernelSyncTimerEvent();			
					IGMP("receive a IGMP leave packet form LANPort[%d]!! WLANDEVID[%d]!! Group=%pI4h ", igrPortMap.macPortIdx,igrWlanDevIdx,&_group[0]);
				}
				else
				{
					_rtk_fc_ucIGMPflowModify_passthroughMc(pPktHdr,igrPortMap,igrWlanDevIdx,_group,UCIGMP_ADD);
					if(fc_db.mcSetMode==RT_IGMP_MULTICAST_SYNC_MODE_KERNEL)
						rtk_fc_igmp_setupKernelSyncTimerEvent();			
					IGMP("receive IGMPv3 Report packet at LANPort[%d]!! WLANDEVID[%d]!! Group=%pI4h", igrPortMap.macPortIdx,igrWlanDevIdx,&_group[0]);
				}
			}
		}
		else if (pPktHdr->igmph->type==IGMP_HOST_LEAVE_MESSAGE)
		{
			_group[0] = ntohl(pPktHdr->igmph->group);
			if(!FC_IS_CLASSD_ADDR(_group[0])){IGMP("IGMP Error packet");return RTK_FC_RET_LRN_SKIP_IGMP_MLD;}			
			_rtk_fc_ucIGMPflowModify_passthroughMc(pPktHdr,igrPortMap,igrWlanDevIdx,_group,UCIGMP_DEL);
			if(fc_db.mcSetMode==RT_IGMP_MULTICAST_SYNC_MODE_KERNEL)
				rtk_fc_igmp_setupKernelSyncTimerEvent();
			IGMP("receive a IGMP leave packet form LANPort[%d]!! WLANDEVID[%d]!! Group=%pI4h ", igrPortMap.macPortIdx,igrWlanDevIdx,&_group[0]);
		}

	}
	else if (pPktHdr->icmp6h && (((1<<igrPortMap.macPortIdx) & _lanPortMask) || (igrWlanDevIdx < RTK_FC_WLANX_END_INTF)))
	{
		if(pPktHdr->icmp6h->icmp6_type ==ICMPV6_MGM_REPORT )
		{
			memcpy(_group, ((void*)pPktHdr->icmp6h)+8 ,sizeof(_group));
			if((ntohl(_group[0])&0xff000000) !=0xff000000){IGMP("MLD Error packet group:%pI6",_group);return RTK_FC_RET_LRN_SKIP_IGMP_MLD;}			
			_rtk_fc_ucIGMPflowModify_passthroughMc(pPktHdr,igrPortMap,igrWlanDevIdx,_group,UCIGMP_ADD);
			if(fc_db.mcSetMode==RT_IGMP_MULTICAST_SYNC_MODE_KERNEL)
				rtk_fc_igmp_setupKernelSyncTimerEvent();
			IGMP("receive MlDv1 Report packet at LANPort[%d]! WLANDEVID[%d]! Group=%pI6",igrPortMap.macPortIdx,igrWlanDevIdx,&_group[0]);
		}
		else if (pPktHdr->icmp6h->icmp6_type ==ICMPV6_MLD2_REPORT )
		{
			numGrp = ntohs(pPktHdr->icmp6h->icmp6_dataun.un_data16[1]);
			grecmld = ((void*)pPktHdr->icmp6h)+8 ;
			if(numGrp>8){IGMP("skip large numberGrp search to avoid attack");return RTK_FC_RET_LRN_SKIP_IGMP_MLD;}
			for (i = 0; i < numGrp; i++)
			{
				memcpy(_group,grecmld->grec_mca.s6_addr,sizeof(_group));
				type = grecmld->grec_type;
				grec_nsrcs = ntohs(grecmld->grec_nsrcs);
				len =  (20 + ntohs(grecmld->grec_nsrcs)*4 + ntohs(grecmld->grec_auxwords)*4) ;
				grecmld = ((void*)grecmld) +len;

				if((ntohl(_group[0])&0xff000000) !=0xff000000){IGMP("MLD Error packet group:%pI6",_group);return RTK_FC_RET_LRN_SKIP_IGMP_MLD;}
				if(grec_nsrcs>8){IGMP("skip large grec_nsrcs search to avoid attack");return RTK_FC_RET_LRN_SKIP_IGMP_MLD;}

				if ((type == MLD2_CHANGE_TO_INCLUDE ||
					type == MLD2_MODE_IS_INCLUDE) &&
					grec_nsrcs == 0)
				{
					_rtk_fc_ucIGMPflowModify_passthroughMc(pPktHdr,igrPortMap,igrWlanDevIdx,_group,UCIGMP_DEL);
					if(fc_db.mcSetMode==RT_IGMP_MULTICAST_SYNC_MODE_KERNEL)
						rtk_fc_igmp_setupKernelSyncTimerEvent();			
					IGMP("receive a MLD leave packet form LANPort[%x]! WLANDEVID[%d]! Group=%pI6 ",igrPortMap.macPortIdx,igrWlanDevIdx,&_group[0]);
				}
				else
				{
					_rtk_fc_ucIGMPflowModify_passthroughMc(pPktHdr,igrPortMap,igrWlanDevIdx,_group,UCIGMP_ADD);
					if(fc_db.mcSetMode==RT_IGMP_MULTICAST_SYNC_MODE_KERNEL)
						rtk_fc_igmp_setupKernelSyncTimerEvent();
					IGMP("receive MlDv2 Report packet at LANPort[%x]! WLANDEVID[%d]! Group=%pI6",igrPortMap.macPortIdx,igrWlanDevIdx,&_group[0]);
				}
			}
		}
		else if (pPktHdr->icmp6h->icmp6_type ==ICMPV6_MGM_REDUCTION )
		{
			memcpy(_group, ((void*)pPktHdr->icmp6h)+8 ,sizeof(_group));
			if((ntohl(_group[0])&0xff000000) !=0xff000000){IGMP("MLD Error packet group:%pI6",_group);return RTK_FC_RET_LRN_SKIP_IGMP_MLD;}
			_rtk_fc_ucIGMPflowModify_passthroughMc(pPktHdr,igrPortMap,igrWlanDevIdx,_group,UCIGMP_DEL);
			if(fc_db.mcSetMode==RT_IGMP_MULTICAST_SYNC_MODE_KERNEL)
				rtk_fc_igmp_setupKernelSyncTimerEvent();
			IGMP("receive a MLD leave packet form LANPort[%x]! WLANDEVID[%d]! Group=%pI6 ",igrPortMap.macPortIdx,igrWlanDevIdx,&_group[0]);
		}
		else
		{
			//not mld packet
			return RTK_FC_RET_LRN_SKIP_ICMPv6;
		}
	}
	else {

		return RTK_FC_RET_OK;
	}


	return RTK_FC_RET_LRN_SKIP_IGMP_MLD;


}


int rtk_fc_igmp_show_mcExtFlowIdxTbl(struct seq_file *s, void *v)
{
	int i;
	
	for(i=0 ; i<RTK_FC_TABLESIZE_MCFLOW ;i++)
	{
		if(!fc_db.mcExtraFlowIdxTbl[i].valid)
			continue;

		if(fc_db.mcExtraFlowIdxTbl[i].isIpv6)
		{
			PROC_PRINTF("[%d]GroupAddress:[%pI6] SIP:[%pI6] CVID[%d] SVID[%d]\n",
				i,fc_db.mcExtraFlowIdxTbl[i].multicastAddress,fc_db.mcExtraFlowIdxTbl[i].sourceAddress,fc_db.mcExtraFlowIdxTbl[i].ingressCvid,fc_db.mcExtraFlowIdxTbl[i].ingressSvid);
		}
		else
		{
			PROC_PRINTF("[%d]GroupAddress:[%pI4h] SIP:[%pI4h] CVID[%d] SVID[%d]\n",
				i,&fc_db.mcExtraFlowIdxTbl[i].multicastAddress[0],fc_db.mcExtraFlowIdxTbl[i].sourceAddress,fc_db.mcExtraFlowIdxTbl[i].ingressCvid,fc_db.mcExtraFlowIdxTbl[i].ingressSvid);
		}

		PROC_PRINTF("dummySyncTimes:%d swFlowIdx=%d\n",fc_db.mcExtraFlowIdxTbl[i].dummySyncTimes,fc_db.mcExtraFlowIdxTbl[i].swFlowIdx);

		//dump_packet(fc_db.mcExtraFlowIdxTbl[i].mcDataBuf.dataBuf.data, MAX_AUTOEXT_PKT_SIZE, "dummyPkt");


	}
	return SUCCESS;
}

int rtk_fc_igmp_mld_dirver_show(struct seq_file *s, void *v)
{
	rtk_fc_table_mcConfigTbl_t *pMcConfig;
	int i;
	char* actStrMap[]={"DROP","TRAP","FORWARD"};

	PROC_PRINTF("*HW_UNKNOWN_IPMCv4 : [%s]\n",actStrMap[fc_db.controlFuc.unknowv4McTrap]);
	PROC_PRINTF("*HW_UNKNOWN_IPMCv6 : [%s]\n",actStrMap[fc_db.controlFuc.unknowv6McTrap]);
	PROC_PRINTF("==============================\n");

	PROC_PRINTF("KNOWN GROUP LIST:\n");

	for(i=0 ; i<RTK_FC_TABLESIZE_MCFLOW ;i++)
	{
		if(!fc_db.mcGroupTbl[i].valid)
			continue;
	
		if(fc_db.mcGroupTbl[i].isIpv6)
		{
			PROC_PRINTF("[%d]GroupAddress [%pI6] refCnt=%d \n",
				i,fc_db.mcGroupTbl[i].mcGroup,fc_db.mcGroupTbl[i].groupRefCnt);
		}
		else
		{
			PROC_PRINTF("[%d]GroupAddress [%pI4h] refCnt=%d \n",
				i,&fc_db.mcGroupTbl[i].mcGroup[0],fc_db.mcGroupTbl[i].groupRefCnt);
		}
		PROC_PRINTF("\t hwIdx=%d ",fc_db.mcGroupTbl[i].hwIdx);
		PROC_PRINTF("\n");
	
	}
	PROC_PRINTF("==============================\n");
	PROC_PRINTF("CONFIG LIST:\n");
	
	list_for_each_entry(pMcConfig,&fc_db.listHead_mcCfgTbl,lsMcConfigEntry)
	{
		int j;
		struct net_device *dev=NULL;
		rtk_fc_realdev_t rdev;
		
		if(pMcConfig->isIpv6)
		{
			PROC_PRINTF("[%d]GroupAddress [%pI6]   <== (carebit:%d)SourceAddr [%pI6]  (carebit:%d)IngressCvid[%d] (carebit:%d)IngressSvid[%d] %s\n",
				i,pMcConfig->confGroup,pMcConfig->careSource, pMcConfig->confSource,pMcConfig->careIngressCvid,pMcConfig->confIgrCvid,pMcConfig->careIngressSvid,pMcConfig->confIgrSvid,pMcConfig->copy2cpu?"COPY2CPU":"");
		}
		else
		{
			PROC_PRINTF("[%d]GroupAddress [%pI4h]   <== (carebit:%d)SourceAddr [%pI4h]  (carebit:%d)IngressCvid[%d] (carebit:%d)IngressSvid[%d] %s\n",
				i,&pMcConfig->confGroup,pMcConfig->careSource,&pMcConfig->confSource,pMcConfig->careIngressCvid,pMcConfig->confIgrCvid,pMcConfig->careIngressSvid,pMcConfig->confIgrSvid,pMcConfig->copy2cpu?"COPY2CPU":"");
		}


		PROC_PRINTF("\t");
		for(j=0;j<pMcConfig->cnt_egrInfo;j++)
		{

			if(pMcConfig->egrInfo[j].ifidx!=FAIL)
			{
				RTK_FC_HOOK_PS_DEV_GET_BY_INDEX(&init_net,pMcConfig->egrInfo[j].ifidx,&dev);
				if(dev)
				{
					RTK_FC_HOOK_PS_DEV_GET_REALDEV_PHYPORT(dev,&rdev);
					if(pMcConfig->egrInfo[j].isWlan)
						PROC_PRINTF(" [WlanPort:%d(%s ifidx=%d)] ",pMcConfig->egrInfo[j].portId,dev->name,pMcConfig->egrInfo[j].ifidx);
					else
						PROC_PRINTF(" [Port:%d(%s ifidx=%d)] ",pMcConfig->egrInfo[j].portId,dev->name,pMcConfig->egrInfo[j].ifidx);

				}
			}
			else
			{
				if(pMcConfig->egrInfo[j].isWlan)
					PROC_PRINTF(" [WlanPort:%d] ",pMcConfig->egrInfo[j].portId);
				else
					PROC_PRINTF(" [Port:%d] ",pMcConfig->egrInfo[j].portId);
			}
		}
		PROC_PRINTF("\n");
	}

	PROC_PRINTF("==============================\n");
	PROC_PRINTF("FLOW LIST:\n");

	for (i =0 ; i< RTK_FC_TABLESIZE_MCFLOW_HASH; i++)
	{
		rtk_fc_mcExtraSwFlowIdx_entry_t *mcSwFlowId_entry=NULL;
		list_for_each_entry(mcSwFlowId_entry,&fc_db.listHead_mcExtraFlowIdxHashTbl[i],entry)
		{
			PROC_PRINTF("[%d]swMcFlow["COLOR_Y"%d"COLOR_NM"]:\n",i,mcSwFlowId_entry->swFlowIdx);
			dump_abstrSwFlowPatten_table(s,v,fc_db.flowTbl[mcSwFlowId_entry->swFlowIdx].pAbstrSwFlowEt);
		}
	}
	return SUCCESS;

}


int32 _rtk_fc_igmp_mld_show_group_info(uint32 isIpv6,uint32 * groupip,uint32 p3PortMask, rtk_fc_wlan_devmask_t p3WlanMask,int32 p3Cvid,uint8 p3Cpri,uint32 p4PortMask, rtk_fc_wlan_devmask_t p4WlanMask,int32 p4Cvid,uint8 p4Cpri,uint32 careIngressCvid,int32 ingress_cvid,uint32 careSip,uint32 * sip)
{

	if(isIpv6==0) {
		rtlglue_printf("Group_v4 %pI4h \n", groupip);
		rtlglue_printf("\t PortMask = %x WlanDevMask = %llx\n",p3PortMask, p3WlanMask);

	}else {
		rtlglue_printf("Group_v6 %pI6 \n", groupip);
		rtlglue_printf("\t PortMask = %x WlanDevMask = %llx\n", p3PortMask, p3WlanMask);

	}

	return SUCCESS;
}


// this function is called in RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH() locked;
// Warning should sure caller is simple
int32 rtk_fc_igmp_sendAllMcDummyPktDetector(unsigned long task_priv)
{
	int i;
	struct sk_buff *skb = NULL;
	rtk_fc_mcExtraSwFlowIdx_entry_t *mcSwFlowId_entry=NULL;

	IGMP("rtk_fc_igmp_sendAllMcDummyPktDetector");

	//unlock first
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();
		
	for (i =0 ; i< RTK_FC_TABLESIZE_MCFLOW; i++)
	{
		skb=NULL;
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();		
		if(fc_db.mcExtraFlowIdxTbl[i].valid)
		{
			mcSwFlowId_entry =    &fc_db.mcExtraFlowIdxTbl[i];		
			skb = rtk_fc_abstrSwflow_dummyPkt_genskb(mcSwFlowId_entry);
		}
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();		
		if(skb)
			RTK_FC_HOOK_PS_SKB_NETIF_RX_WITH_HOOK(skb);
	}
	
	//recover to lock
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();		


	return SUCCESS;
}


void rtk_fc_igmp_mcDummyPktDetectorConfigPort(unsigned long task_priv)
{
	int i;
	rtk_fc_mcExtraSwFlowIdx_entry_t *mcSwFlowId_entry=NULL;
	rtk_fc_abstrSwFlowActionList_entry_t *pTmpSwFlowAction;
	rtk_fc_abstrSwFlowLdpid_entry_t *pLdpid=NULL;
	uint32 portCnt=0;
	rtk_fc_table_mcConfigTbl_t *p_userconf=NULL;
	rtk_fc_table_mcConfigTbl_t userconf;

	
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();

	for(i=0;i<TRACFILTER_MAX;i++)
	{
		if(fc_db.trace_filter_bitmask[i]&FC_DEBUG_TRACE_FILTER_DUMMYMC)
		{
			fc_db.tracefilterShow=1U;
		}
	}


	for (i =0 ; i< RTK_FC_TABLESIZE_MCFLOW_HASH; i++)
	{
		list_for_each_entry(mcSwFlowId_entry,&fc_db.listHead_mcExtraFlowIdxHashTbl[i],entry)
		{
			p_userconf=NULL;

			/* sent dummy packet 3 times , period for RTK_FC_IGMP_DUMYPKT_TICK_MSECONDMS */
			if(mcSwFlowId_entry->dummySyncTimes >= 3)
				continue;
			if(fc_db.flowTbl[mcSwFlowId_entry->swFlowIdx].pAbstrSwFlowEt==NULL)
				continue;

			//check config and abstr-port not sync
			
			if(RTK_FC_HELPER_IGMP_IS_MDB_REG())
			{
				int j;
				if(RTK_FC_HELPER_IGMP_MDB_SEARCH(mcSwFlowId_entry->isIpv6,mcSwFlowId_entry->multicastAddress,mcSwFlowId_entry->sourceAddress,mcSwFlowId_entry->ingressCvid,mcSwFlowId_entry->ingressSvid,&userconf)==SUCCESS)
					p_userconf = &userconf;
				if(p_userconf) {
					if(mcSwFlowId_entry->isIpv6)
						IGMP("Group:%pI6 ingressCvid:%d ingressSvid:%d",mcSwFlowId_entry->multicastAddress,mcSwFlowId_entry->ingressCvid,mcSwFlowId_entry->ingressSvid);
					else
						IGMP("Group:%pI4h ingressCvid:%d ingressSvid:%d",&mcSwFlowId_entry->multicastAddress[0],mcSwFlowId_entry->ingressCvid,mcSwFlowId_entry->ingressSvid);
					for(j=0;j<p_userconf->cnt_egrInfo;j++)
					{
						IGMP("getOutSideMdb IFIDX:%d egrssPort:%d",p_userconf->egrInfo[j].ifidx,p_userconf->egrInfo[j].portId);
					}				
				}
			}
			else
			{
				p_userconf=rtk_fc_mc_searchMcConfigByPatten(mcSwFlowId_entry->isIpv6,mcSwFlowId_entry->multicastAddress,mcSwFlowId_entry->sourceAddress,mcSwFlowId_entry->ingressCvid,mcSwFlowId_entry->ingressSvid);
				if(p_userconf==NULL )
				{
					p_userconf = rtk_fc_mc_searchDftMcConfigByPktPatten(mcSwFlowId_entry->isIpv6,mcSwFlowId_entry->multicastAddress,mcSwFlowId_entry->sourceAddress,mcSwFlowId_entry->ingressCvid,mcSwFlowId_entry->ingressSvid);
				}
			}
			if(p_userconf==NULL)
				continue;
			
			portCnt=0;
			list_for_each_entry(pTmpSwFlowAction, &fc_db.flowTbl[mcSwFlowId_entry->swFlowIdx].pAbstrSwFlowEt->swFlowActionHdr, swFlowActionList)
			{
				list_for_each_entry(pLdpid, &pTmpSwFlowAction->ldpidListHdr, ldpidList)
				{
					portCnt++;
				}
			}

			//only care egress packet lose
			if(p_userconf->cnt_egrInfo > portCnt)
			{
				IGMP("mcConfig->cnt_egrInfo=%d and portCnt=%d not sync send dummy packet for flow[%d]",p_userconf->cnt_egrInfo,portCnt,mcSwFlowId_entry->swFlowIdx);
				rtk_fc_abstrSwflow_sendMcDummyPkt(mcSwFlowId_entry);
				mcSwFlowId_entry->dummySyncTimes++;
				rtk_fc_igmp_setupmcDummyPktDetectorTimer();
			}
			
		}
	}

	fc_db.tracefilterShow=0U;
	RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();


	return;
}


void rtk_fc_igmp_setupmcDummyPktDetectorTimer(void)
{
	unsigned long int msec2jiffies=0;
		

	if(RTK_FC_HELPER_TIMER_PENDING(fc_db.igmpDummyPktDetectorTimer.igmpTimer))
	{
		IGMP("update igmpDummyPktDetectorTimer stop time");
		RTK_FC_HELPER_MSECS_TO_JIFFIES(fc_db.igmpDummyPktDetectorTimer.nextTimerPeriod, &msec2jiffies);
		//update stop time
		//fc_db.igmpDummyPktDetectorTimer.stopjiffies = jiffies + 3*msec2jiffies ;
	}
	else
	{
		IGMP("IGMP START a DummyPktDetectorTimer Timer");
		RTK_FC_HELPER_INIT_TIMER(fc_db.igmpDummyPktDetectorTimer.igmpTimer,rtk_fc_igmp_mcDummyPktDetectorConfigPort);
		RTK_FC_HELPER_SETUP_TIMER(fc_db.igmpDummyPktDetectorTimer.igmpTimer,rtk_fc_igmp_mcDummyPktDetectorConfigPort,(unsigned long)NULL);
		RTK_FC_HELPER_MSECS_TO_JIFFIES(fc_db.igmpDummyPktDetectorTimer.nextTimerPeriod, &msec2jiffies);
		//fc_db.igmpDummyPktDetectorTimer.stopjiffies = jiffies + 3*msec2jiffies+1 ;
		RTK_FC_HELPER_MOD_TIMER(fc_db.igmpDummyPktDetectorTimer.igmpTimer, jiffies+msec2jiffies);
	}

	return ;
}

#define KERNEL_MODE


int rtk_fc_igmp_kernelMode_syncToDriverConfig(struct net_device *br_netdev)
{
#if defined(CONFIG_BRIDGE_IGMP_SNOOPING)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
	// there is no struct net_bridge_mdb_htable in linux 5.10
	return FAIL;
#else
	int i;

	struct net_bridge *br =NULL;
	struct net_bridge_mdb_htable *mdb=NULL;
	struct net_bridge_mdb_entry *mp=NULL;
	rt_igmp_group_devPort_cfg_t groupInfo;

	IGMP("rtk_fc_igmp_kernelMode_syncToDriverConfig ");
	
	br = netdev_priv(br_netdev);

	if (!netif_running(br->dev))
	{
		IGMP("br is not runing");
		return FAIL;
	}
	if((br->multicast_disabled==1))
	{
		IGMP("Kernel Disable Snooping multicast_disabled=%d  br->multicast_querier=%d",br->multicast_disabled,br->multicast_querier);
		return FAIL;
	}

	if(spin_is_locked(&br->multicast_lock))
	{
		IGMP("multicast_lock lock sync next time");
		return FAIL;
	}
	rtk_fc_g_rcu_read_lock();
	spin_lock_bh(&br->multicast_lock);

	mdb = mlock_dereference(br->mdb, br);

	if(mdb==NULL)
	{
		IGMP("can't get mdb, skip this run");
		goto END;
	}

	IGMP("br mdb  %p, max hash idx %d", mdb, mdb->max);

	for(i=0 ; i< mdb->max ;i++)
	{
		hlist_for_each_entry_rcu(mp, &mdb->mhash[i], hlist[mdb->ver])
		{
			struct net_bridge_port_group *p;
			struct net_bridge_port_group __rcu **pp;

			memset(&groupInfo,0,sizeof(groupInfo));
			groupInfo.groupBehavior=RT_MC_BEHAVIOR_FLOW_FORWAED;
			groupInfo.egrInfoType=EGRINFO_DEVIFIDX;
			

			IGMP("i = %d, mdb entry (mp) = %p", i, mp);
			if(mp->br!=br){
				IGMP("mhash[%d] mdb entry br invalid", i);
				break;
			}
			if(mp->ports==NULL){
				IGMP("mhash[%d] mp ports is invalid %p", i, mp->ports);
				break;
			}

			IGMP("mhash[%d] scan mp ports: %p", i, mp->ports);
			for (pp = &mp->ports; (p = rcu_dereference(*pp)) != NULL; pp = &p->next) {

				if (p!=NULL && p->port!=NULL) {
					IGMP("mhash[%d] find dev %s proto = 0x%04x", i, p->port->dev->name, ntohs(mp->addr.proto));
					groupInfo.egrInfo.devIfidx[groupInfo.cntEgrInfo].ifindex = p->port->dev->ifindex;
					groupInfo.cntEgrInfo++;
				}
			}


			if(groupInfo.cntEgrInfo)
			{

				if(mp->addr.proto == htons(ETH_P_IP))
				{
					groupInfo.is_ipv6=0;
					groupInfo.group_addr.ipv4 = ntohl(mp->addr.u.ip4);
				}
#if IS_ENABLED(CONFIG_IPV6)
				else if(mp->addr.proto == htons(ETH_P_IPV6))
				{
					groupInfo.is_ipv6=1;
					memcpy(&groupInfo.group_addr.ipv6[0],mp->addr.u.ip6.s6_addr32,sizeof(groupInfo.group_addr.ipv6));
				}
#endif
				_rtk_fc_igmp_mc_set_dev_to_dirver(&groupInfo);
			}
		

		}
	}

END:
	spin_unlock_bh(&br->multicast_lock);

	rtk_fc_g_rcu_read_unlock();



	return SUCCESS;
#endif

#else
	return FAIL;
#endif
}


int32 _rtk_fc_igmp_sync_kernel_to_dirver(unsigned long task_priv)
{
	rt_igmp_group_devPort_cfg_t groupInfo;
	int i;
	int32 ret=SUCCESS;

	if(fc_db.mcSetMode==RT_IGMP_MULTICAST_SYNC_MODE_KERNEL)
	{
		
		//init valid entry default delete
		rtk_fc_table_mcConfigTbl_t *pMcConfig;
		rtk_fc_table_mcConfigTbl_t *pMcConfig_tmp;
		list_for_each_entry(pMcConfig,&fc_db.listHead_mcCfgTbl,lsMcConfigEntry)
		{
			pMcConfig->do_delete=1;
		}
		

		for(i = DEVIFIDX_VALID_MIN ; i< RTK_FC_DEV_GW_MAC_TBL ; i++)
		{
			if( fc_db.devGwMacTbl[i].dev==NULL)
				continue;
			if(!RTK_FC_HOOK_PS_DEV_IS_PRIV_FLAGS_SET(fc_db.devGwMacTbl[i].dev, RTK_FC_NETDEV_PRIV_FLAG_TYPE_EBRIDGE)) // check bridge dev
				continue;

			ret=rtk_fc_igmp_kernelMode_syncToDriverConfig(fc_db.devGwMacTbl[i].dev);			
		}

		if(ret!=SUCCESS)
			return ret;

		//delete non-update entry
		list_for_each_entry_safe(pMcConfig,pMcConfig_tmp,&fc_db.listHead_mcCfgTbl,lsMcConfigEntry)
		{
			if(!pMcConfig->do_delete)
				continue;
			memset(&groupInfo,0,sizeof(groupInfo));
			if(pMcConfig->isIpv6)
				memcpy(&groupInfo.group_addr.ipv6[0],pMcConfig->confGroup,sizeof(groupInfo.group_addr));
			else
				groupInfo.group_addr.ipv4=pMcConfig->confGroup[0];

			_rtk_fc_igmp_mc_set_dev_to_dirver(&groupInfo);

		}		
		
	}


	return SUCCESS;
}


void rtk_fc_igmp_periodTimer(unsigned long task_priv)
{
	int32 ret;
	unsigned long int msec2jiffies=0;
		
	if(fc_db.controlFuc.hwnat_mode==RT_FLOW_HWNAT_MODE_DIS_ACC || (fc_db.controlFuc.hwnat_mode==RT_FLOW_HWNAT_MODE_TRAP_ALL)) //Trap to PS mode
	{
		return;
	}
	if(fc_db.mcSetMode==RT_IGMP_MULTICAST_SYNC_MODE_KERNEL)
	{
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();
		//========================= Critical Section Start =========================//
		TIMER("IGMP TIME periodTimer TICK");
		ret = _rtk_fc_igmp_sync_kernel_to_dirver(task_priv);
		if(ret != SUCCESS){
			RTK_FC_HELPER_MSECS_TO_JIFFIES(100, &msec2jiffies);
			RTK_FC_HELPER_MOD_TIMER(fc_db.igmpKernelSyncTimerPeriod.igmpTimer, jiffies+msec2jiffies);
		}else{
			RTK_FC_HELPER_MSECS_TO_JIFFIES(fc_db.igmpKernelSyncTimerPeriod.nextTimerPeriod, &msec2jiffies);
			RTK_FC_HELPER_MOD_TIMER(fc_db.igmpKernelSyncTimerPeriod.igmpTimer, jiffies+msec2jiffies);
		}
		//========================= Critical Section End =========================//
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();
	}

	return;
}

void rtk_fc_igmp_eventTimer(unsigned long task_priv)
{
	int32 ret;
	unsigned long int msec2jiffies=0;

	if(fc_db.controlFuc.hwnat_mode==RT_FLOW_HWNAT_MODE_DIS_ACC || (fc_db.controlFuc.hwnat_mode==RT_FLOW_HWNAT_MODE_TRAP_ALL))//Trap to PS mode
	{
		return;
	}

	if(fc_db.mcSetMode==RT_IGMP_MULTICAST_SYNC_MODE_KERNEL)
	{
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();
		//========================= Critical Section Start =========================//
		ret = _rtk_fc_igmp_sync_kernel_to_dirver(task_priv);
		TIMER("IGMP TIME eventTimer TICK %lu  %lu",jiffies,fc_db.igmpKernelSyncTimerEvent.stopjiffies);
		if((time_before(jiffies,fc_db.igmpKernelSyncTimerEvent.stopjiffies)) || ret!=SUCCESS)
		{
			RTK_FC_HELPER_MSECS_TO_JIFFIES(fc_db.igmpKernelSyncTimerEvent.nextTimerPeriod, &msec2jiffies);
			//go timer next
			RTK_FC_HELPER_MOD_TIMER(fc_db.igmpKernelSyncTimerEvent.igmpTimer, jiffies+msec2jiffies);
		}
		else
		{
			//delete timer
			if(RTK_FC_HELPER_TIMER_PENDING(fc_db.igmpKernelSyncTimerEvent.igmpTimer))
				RTK_FC_HELPER_DEL_TIMER(fc_db.igmpKernelSyncTimerEvent.igmpTimer);
		}
		//========================= Critical Section End =========================//
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();
	}

	return;
}

#define REPERT_TIMES 2
void rtk_fc_igmp_setupKernelSyncTimerEvent(void)
{
	unsigned long int msec2jiffies=0;
		
	if(fc_db.mcSetMode!=RT_IGMP_MULTICAST_SYNC_MODE_KERNEL)
		return ;

	if(RTK_FC_HELPER_TIMER_PENDING(fc_db.igmpKernelSyncTimerEvent.igmpTimer))
	{
		IGMP("update rtk_fc_igmp_setupKernelSyncTimerEvent stop time");
		RTK_FC_HELPER_MSECS_TO_JIFFIES(fc_db.igmpKernelSyncTimerEvent.nextTimerPeriod, &msec2jiffies);
		//update stop time
		fc_db.igmpKernelSyncTimerEvent.stopjiffies = jiffies + REPERT_TIMES*msec2jiffies+1 ;
	}
	else
	{
		RTK_FC_HELPER_INIT_TIMER(fc_db.igmpKernelSyncTimerEvent.igmpTimer,rtk_fc_igmp_eventTimer);
		RTK_FC_HELPER_SETUP_TIMER(fc_db.igmpKernelSyncTimerEvent.igmpTimer,rtk_fc_igmp_eventTimer,(unsigned long)NULL);
		RTK_FC_HELPER_MSECS_TO_JIFFIES(fc_db.igmpKernelSyncTimerEvent.nextTimerPeriod, &msec2jiffies);
		RTK_FC_HELPER_MOD_TIMER(fc_db.igmpKernelSyncTimerEvent.igmpTimer, jiffies+msec2jiffies);
		fc_db.igmpKernelSyncTimerEvent.stopjiffies = jiffies + REPERT_TIMES*msec2jiffies+1 ;		
		IGMP("IGMP START a rtk_fc_igmp_setupKernelSyncTimerEvent Timer");
	}

	return ;
}


void rtk_fc_igmp_setupKernelSyncTimerPeriod(void)
{
	unsigned long int msec2jiffies=0;
		
	if(fc_db.mcSetMode!=RT_IGMP_MULTICAST_SYNC_MODE_KERNEL)
		return ;

	if(RTK_FC_HELPER_TIMER_PENDING(fc_db.igmpKernelSyncTimerPeriod.igmpTimer))
	{
		//IGMP("update rtk_fc_igmp_setupKernelSyncTimerPeriod stop time");
		//RTK_FC_HELPER_MSECS_TO_JIFFIES(fc_db.igmpKernelSyncTimerPeriod.nextTimerPeriod, &msec2jiffies);
		//update stop time
		//fc_db.igmpDummyPktDetectorTimer.stopjiffies = jiffies + 3*msec2jiffies ;
	}
	else
	{
		IGMP("IGMP START a rtk_fc_igmp_setupKernelSyncTimerPeriod Timer");
		RTK_FC_HELPER_INIT_TIMER(fc_db.igmpKernelSyncTimerPeriod.igmpTimer,rtk_fc_igmp_periodTimer);
		RTK_FC_HELPER_SETUP_TIMER(fc_db.igmpKernelSyncTimerPeriod.igmpTimer,rtk_fc_igmp_periodTimer,(unsigned long)NULL);
		RTK_FC_HELPER_MSECS_TO_JIFFIES(fc_db.igmpKernelSyncTimerPeriod.nextTimerPeriod, &msec2jiffies);
		RTK_FC_HELPER_MOD_TIMER(fc_db.igmpKernelSyncTimerPeriod.igmpTimer, jiffies+msec2jiffies);
	}

	return ;
}




#if defined(CONFIG_RTK_SOC_RTL8198D)

int _rtk_fc_add_lutMac(unsigned int*_group ,unsigned short isIpv6)
{
	unsigned char mac[6]={0};
	int16 mcidx;
	int16 mcL2Idx=-1;
	
	if(isIpv6)
		_rtk_fc_groupLSB32_to_mac(mac,isIpv6,ntohl(_group[3]));
	else
		_rtk_fc_groupLSB32_to_mac(mac,isIpv6,_group[0]);


	//check group mac in LUT 
	if(_rtk_fc_lut_find((uint8 *)mac, &mcidx) != RTK_FC_RET_OK)
	{
		// add group mac to LUT
		if(isIpv6)
			ASSERT_EQ(_rtk_fc_igmp_lutMcAdd(isIpv6, ntohl(_group[3]),TRUE,&mcL2Idx),RTK_FC_RET_OK);
		else
			ASSERT_EQ(_rtk_fc_igmp_lutMcAdd(isIpv6, _group[0],TRUE,&mcL2Idx),RTK_FC_RET_OK);

		//printk("_rtk_fc_igmp_lutMcAdd ok ,gip= %x,mcidx =%x\n",_group[0],mcL2Idx);
	}
	
	return SUCCESS;
	
}


int _rtk_fc_check_user_group(unsigned char * skbData)
{
	unsigned int group[4] = {0};
	unsigned short isIpv6 = 0;
	unsigned char* pdata = skbData;

	//check DIP
	if((pdata[0]==0x01)&&(pdata[1]==0x00)&&(pdata[2]==0x5e))  
	{
		pdata = pdata + (ETH_ALEN<<1);
	   if(*((unsigned short*)pdata) == (unsigned short)htons(0x8100))
	   {
			pdata = pdata + 4;	
	   }
	   pdata = pdata + 18;	

	   group[0] = htonl(*((unsigned int*)pdata));
	   
	   isIpv6 = 0;
	}
	else if((pdata[0]==0x33)&&(pdata[1]==0x33)&&(pdata[2]==0xff))
	{

		pdata = pdata + (ETH_ALEN<<1);
	   if(*((unsigned short*)pdata) == (unsigned short)htons(0x8100))
	   {
			pdata = pdata + 4;	
	   }
	   pdata = pdata + 26;	

	   group[0] = htonl(*((unsigned int*)pdata));
	   group[1] = htonl(*(((unsigned int*)pdata)+1));
	   group[2] = htonl(*(((unsigned int*)pdata)+2));
	   group[3] = htonl(*(((unsigned int*)pdata)+3));

	   isIpv6 = 1;
	}
	else
	 return FALSE;


	
	if(rtk_fc_get_user_group(group, isIpv6) != 0)
		return FALSE;
	else
	{
		//printk("rtk_fc_check_user_group, group =%x,isIpv6 =%d\n",group[0],isIpv6);
		return TRUE;
	}
}

#endif

int rtk_fc_mc2ucCopyCnt_get(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr,int *needSendCnt,rtk_fc_table_mcConfigTbl_t *p_userconf)
{
	int isWlan=0,ldpid=FAIL;

	int i;
	struct net_device *p_PScacheDev[DEV_STACK_MAX]={NULL,NULL,NULL,NULL};
	int filterStackingCnt=0;

	*needSendCnt=1;

	if(p_userconf==NULL)
	{
		//IGMP("can't find McConfig do not add hardware");
		return FALSE;
	}

	if(pPktHdr->egrWlanDevIdx < RTK_FC_WLANX_END_INTF) 
	{
		isWlan = 1;
		ldpid = pPktHdr->egrWlanDevIdx;
	}
	else 
	{
		ldpid = pPktHdr->egressPort.macPortIdx;
	}

	for(i=0 ; i<DEV_STACK_MAX ;i++)
	{
		p_PScacheDev[i] = rtk_fc_getDev_by_fcDevIdx(pPktHdr->logicalOutDevIfidx[i]);

		if(p_PScacheDev[i])
			filterStackingCnt++;
	}

	for(i=0 ; i<p_userconf->cnt_egrInfo ; i++ )
	{
		if(p_userconf->egrInfo[i].ifidx!=FAIL && filterStackingCnt)
		{
			if((p_userconf->egrInfo[i].ifidx == p_PScacheDev[filterStackingCnt-1]->ifindex) && p_userconf->egrInfo[i].dmacTrans)
			{
				IGMP("hit ifidx:%d dmacTrans:%d",p_PScacheDev[filterStackingCnt-1]->ifindex,p_userconf->egrInfo[i].dmacTrans);
				(*needSendCnt)++;
			}
		}
		else
		{
			if((p_userconf->egrInfo[i].isWlan == isWlan && p_userconf->egrInfo[i].portId==ldpid) &&  p_userconf->egrInfo[i].dmacTrans)
			{
				IGMP("check mc config hit isWlan:%d ldpid:%d dmacTrans:%d",isWlan,ldpid,p_userconf->egrInfo[i].dmacTrans);
				(*needSendCnt)++;
			}
		}
	}

	//because needSendcCnt start form 1
	if(*needSendCnt >1)
		*needSendCnt-=1;

	IGMP("needSendCnt :%d",*needSendCnt);

	return SUCCESS;

}



int rtk_fc_mc2ucPktModify(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr,int sendCntNum,rtk_fc_table_mcConfigTbl_t *p_userconf)
{

	//modify skb->data da
	int isWlan=0,ldpid=FAIL;
	int i;
	struct net_device *p_PScacheDev[DEV_STACK_MAX]={NULL,NULL,NULL,NULL};
	int filterStackingCnt=0;
	int counter=0;
	uint8 *p_dmac=NULL;

	if(p_userconf==NULL)
	{
		//IGMP("can't find McConfig do not add hardware");
		return FALSE;
	}

	if(pPktHdr->egrWlanDevIdx < RTK_FC_WLANX_END_INTF) 
	{
		isWlan = 1;
		ldpid = pPktHdr->egrWlanDevIdx;
	}
	else 
	{
		ldpid = pPktHdr->egressPort.macPortIdx;
	}

	for(i=0 ; i<DEV_STACK_MAX ;i++)
	{
		p_PScacheDev[i] = rtk_fc_getDev_by_fcDevIdx(pPktHdr->logicalOutDevIfidx[i]);

		if(p_PScacheDev[i])
			filterStackingCnt++;
	}

	for(i=0 ; i<p_userconf->cnt_egrInfo ; i++ )
	{
		if(p_userconf->egrInfo[i].ifidx!=FAIL && filterStackingCnt)
		{
			if((p_userconf->egrInfo[i].ifidx == p_PScacheDev[filterStackingCnt-1]->ifindex) && p_userconf->egrInfo[i].dmacTrans)
			{
				IGMP("hit ifidx:%d dmacTrans:%d",p_PScacheDev[filterStackingCnt-1]->ifindex,p_userconf->egrInfo[i].dmacTrans);
				counter++;
				if(counter==sendCntNum)
				{
					p_dmac=p_userconf->egrInfo[i].dmac;
					break;
				}
			}
		}
		else
		{
			if((p_userconf->egrInfo[i].isWlan == isWlan && p_userconf->egrInfo[i].portId==ldpid) &&  p_userconf->egrInfo[i].dmacTrans)
			{
				IGMP("check mc config hit isWlan:%d ldpid:%d dmacTrans:%d",isWlan,ldpid,p_userconf->egrInfo[i].dmacTrans);
				counter++;
				if(counter==sendCntNum)
				{
					p_dmac=p_userconf->egrInfo[i].dmac;
					break;
				}

			}
		}
	}

	if(p_dmac)
	{
		//_rtk_fc_pktHdr_inspect
		int16 lutIdx;

		IGMP("trans dmac to %pM",p_dmac);
		memcpy(rtskb->skb->data,p_dmac,6);

		if(sendCntNum==1)
		{
			// ingress: in_DA
			if(_rtk_fc_lut_find(rtskb->skb->data, &lutIdx)==RTK_FC_RET_OK) {
				pPktHdr->dmacL2Idx = lutIdx;
				IGMP("update lutIdx to :%d",lutIdx);
			}else 
				pPktHdr->dmacL2Idx = SIGNED_INVALID;
		}
		else
		{
			pPktHdr->dmacL2Idx=FAILED;
		}
		
		return SUCCESS;	
	}
	else
	{
		IGMP("dmac counter not get, do not need transfer dmac to uc");
		return FAIL;
	}
	
}

