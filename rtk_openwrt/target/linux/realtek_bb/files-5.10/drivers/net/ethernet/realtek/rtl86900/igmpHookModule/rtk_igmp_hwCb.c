#if defined(CONFIG_RTL8367_SUPPORT) && defined(CONFIG_RTL9607C_SERIES) && defined(CONFIG_RTK_SKB_MARK2)
#include <rtl8367/rtk_types.h>
#include <rtl8367/l2.h>
#endif
#include <rtk_igmp_snooping.h>
#include <rt_igmp_ext.h>
#include <rtk_igmp_debug.h>
#include <rtk_igmp_hook.h>

#include <rtk_fc_helper_wlan.h>
#include <rtk_fc_helper.h>
#include <linux/workqueue.h>


#define UNCTAG -1
#define IF_SOURCECARE 1
#define IF_VLANCARE 1
#define IGMP_MAX_WQ_SIZE 512 
#define REALTEK_MAGIC_NUMBER 0xB7


int fc_igmp_mdb_callback_search(unsigned char isIpv6,unsigned int *groupip,unsigned int *srcip,short igrCvlan,short igrSvlan,rtk_fc_table_mcConfigTbl_t *mcCfg)
{
	struct rtk_igmp_multicastDataInfo multicastDataInfo;
	rtk_igmp_userResvGroup_t *grp2cpu;
	int i,j;
	rtk_igmp_flowCbEvt_t *pFlowCb=NULL;

	i=0;j=0;
	memset(&multicastDataInfo,0,sizeof(multicastDataInfo));

	if(isIpv6)
	{
		multicastDataInfo.ipVersion =IP_VERSION6;
		memcpy(&multicastDataInfo.groupAddr[0],groupip,sizeof(multicastDataInfo.groupAddr));
		memcpy(&multicastDataInfo.sourceIp[0],srcip,sizeof(multicastDataInfo.sourceIp));
	}
	else
	{
		multicastDataInfo.ipVersion =IP_VERSION4;
		multicastDataInfo.groupAddr[0] = *groupip;
		multicastDataInfo.sourceIp[0] = *srcip;
	}
	
	if(igrCvlan==FAIL)
	{
		multicastDataInfo.vlanTagif=0;
	}
	else
	{
		multicastDataInfo.vlanTagif=1;
		multicastDataInfo.vlanId = igrCvlan;
	}
	
	if(igrSvlan==FAIL)
	{
		multicastDataInfo.svlanTagif=0;
	}
	else
	{
		multicastDataInfo.svlanTagif=1;
		multicastDataInfo.svlanId = igrSvlan;
	}
	
	multicastDataInfo.l4proto=FAIL;
	multicastDataInfo.dport=FAIL;
	multicastDataInfo.sport=FAIL;


	igmp_spin_lock_bh(igmpSysdb.lock_igmp);
	
	pFlowCb=&igmpSysdb.flowCb;
	memset(pFlowCb,0,sizeof(*pFlowCb));
	rtk_igmp_flowCbAppendOtherBrToCb(FAIL,&multicastDataInfo,pFlowCb);
	
	grp2cpu =_rtk_igmp_checkGroupToPsTbl(multicastDataInfo.ipVersion,multicastDataInfo.groupAddr);

	//fill mcCfg
	memset(mcCfg,0,sizeof(*mcCfg));
	mcCfg->careIngressCvid = IF_VLANCARE;
	mcCfg->careIngressSvid = IF_VLANCARE;
	mcCfg->careSource = IF_SOURCECARE;
	mcCfg->isIpv6 = isIpv6;
	mcCfg->confIgrCvid = igrCvlan;
	mcCfg->confIgrSvid = igrSvlan;
	memcpy(mcCfg->confGroup,multicastDataInfo.groupAddr,sizeof(mcCfg->confGroup));
	memcpy(mcCfg->confSource,multicastDataInfo.sourceIp,sizeof(mcCfg->confSource));
	if(grp2cpu && grp2cpu->patten.isCopy2cpu)
		mcCfg->copy2cpu = 1;

	for(i=0;i<IGMP_MAX_DEV_NUM;i++)
	{
		if(pFlowCb->egressDevInfo[i].valid)
		{
			struct net_device *dev;
			rtk_fc_realdev_t rdev;
			rtk_fc_wlan_devidx_t wlanDevIdx;

			if(mcCfg->cnt_egrInfo>=RT_MCCFG_MAX_NUM)
				{IGMP_WARNING("fwdCnt out-of-range Error");return FAIL;}
				
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE) && defined(IGMP_SUPPORT_MCTOUC)
			if(pFlowCb->egressDevInfo[i].subDmac[0].valid)
			{
				for(j=0;j<IGMP_MAX_EXTDEV_MAC_NUM;j++)
				{
					if(pFlowCb->egressDevInfo[i].subDmac[j].valid)
					{
						if(igmpSysdb.hwCbMode==RTK_IGMP_CONFIG_BY_IFIDX)
							mcCfg->egrInfo[mcCfg->cnt_egrInfo].ifidx = pFlowCb->egressDevInfo[i].devIfIdx;
						else
							mcCfg->egrInfo[mcCfg->cnt_egrInfo].ifidx=FAIL;
								
						dev = rtk_igmp_devIfidx_to_dev(pFlowCb->egressDevInfo[i].devIfIdx);
						
						if(rtk_fc_dev_get_realdev_phyport(dev,&rdev)!=FAIL)
						{
							mcCfg->egrInfo[mcCfg->cnt_egrInfo].isWlan=0;
							mcCfg->egrInfo[mcCfg->cnt_egrInfo].portId = rdev.physicalPid;
						}
						else if(rtk_fc_dev2wlanDevIdx(dev, &wlanDevIdx)!=FAIL)
						{
							mcCfg->egrInfo[mcCfg->cnt_egrInfo].isWlan =1;
							mcCfg->egrInfo[mcCfg->cnt_egrInfo].portId =wlanDevIdx;
						}
						else 
						{
							IGMP_DATA("Can not find port by ifidx");
						}
						mcCfg->egrInfo[mcCfg->cnt_egrInfo].dmacTrans=1;
						memcpy(mcCfg->egrInfo[mcCfg->cnt_egrInfo].dmac,pFlowCb->egressDevInfo[i].subDmac[j].dmac,6);

						mcCfg->cnt_egrInfo++;

					}
					else
						break;

				}

			}
			else
#endif				
			{
				if(igmpSysdb.hwCbMode==RTK_IGMP_CONFIG_BY_IFIDX)
					mcCfg->egrInfo[mcCfg->cnt_egrInfo].ifidx = pFlowCb->egressDevInfo[i].devIfIdx;
				else
					mcCfg->egrInfo[mcCfg->cnt_egrInfo].ifidx=FAIL;
						
				dev = rtk_igmp_devIfidx_to_dev(pFlowCb->egressDevInfo[i].devIfIdx);

				if(rtk_fc_dev_get_realdev_phyport(dev,&rdev)!=FAIL)
				{
					mcCfg->egrInfo[mcCfg->cnt_egrInfo].isWlan=0;
					mcCfg->egrInfo[mcCfg->cnt_egrInfo].portId = rdev.physicalPid;
				}
				else if(rtk_fc_dev2wlanDevIdx(dev, &wlanDevIdx)!=FAIL)
				{
					mcCfg->egrInfo[mcCfg->cnt_egrInfo].isWlan =1;
					mcCfg->egrInfo[mcCfg->cnt_egrInfo].portId =wlanDevIdx;
				}
				else 
				{
					IGMP_DATA("Can not find port by ifidx");
				}
				mcCfg->cnt_egrInfo++;

			}
		}
		else
			break;
	}
	igmp_spin_unlock_bh(igmpSysdb.lock_igmp);


	IGMP_DATA("cnt_egrInfo=%d",mcCfg->cnt_egrInfo);
	for(i=0;i<mcCfg->cnt_egrInfo;i++)
	{
		IGMP_DATA("EgrPort:ID(%d) IFIDX(%d) %s",mcCfg->egrInfo[i].portId,mcCfg->egrInfo[i].ifidx,mcCfg->egrInfo[i].isWlan?"WLAN":"PHY");
		if(mcCfg->egrInfo[i].dmacTrans)
			IGMP_DATA("DmacTrans : %pM",mcCfg->egrInfo[i].dmac);
	}

	return SUCCESS;
}


#if defined(CONFIG_RTL8367_SUPPORT) && defined(CONFIG_RTL9607C_SERIES) && defined(CONFIG_RTK_SKB_MARK2)
int rtk_igmp_groupLSB32_to_mac(unsigned char* mac,uint32 isIpv6,uint32 groupip_LSB_32)
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

#define LUTFID5_8367 5 


int32 rtk_igmp_switch8367_extPortHwControl(rtk_igmp_flowCbEvt_t *flowCb)
{
	int i,j;
	int ret=SUCCESS;
	rtksw_l2_mcastAddr_t mcastAddr = {0};
	mcastAddr.fid=LUTFID5_8367;

	for(i=0 ; i<IGMP_MAX_DEV_NUM ; i++)
	{
		if(flowCb->egressDevInfo[i].valid)
		{
			for(j=0 ; j<IGMP_MAX_EXTDEV_PORT_NUM ; j++ )
			{
				if(flowCb->egressDevInfo[i].subPorts[j].valid)
				{
					mcastAddr.portmask.bits[0] |= (1<<flowCb->egressDevInfo[i].subPorts[j].devExtraPortId);
				}
				else
					break;
			}
		}
		else
			break;
	}

	if(flowCb->isIPv6)
		rtk_igmp_groupLSB32_to_mac(mcastAddr.mac.octet,1,ntohl(flowCb->group[3]));
	else
		rtk_igmp_groupLSB32_to_mac(mcastAddr.mac.octet,0,flowCb->group[0]);

	if(mcastAddr.portmask.bits[0])
	{
		ret=rtksw_l2_mcastAddr_add(&mcastAddr);
		if(ret != SUCCESS)
		{
			IGMP_WARNING("rtksw_l2_mcastAddr_add FAIL, ret=%d.\n", ret);
		}
		IGMP_HWCB("8367Ctrl ADD MAC:%pM pmsk=%x fid=%d",mcastAddr.mac.octet,mcastAddr.portmask.bits[0] ,mcastAddr.fid);
	}
	else
	{
		ret=rtksw_l2_mcastAddr_del(&mcastAddr);
		IGMP_HWCB("8367Ctrl DEL MAC:%pM pmsk=%x fid=%d",mcastAddr.mac.octet,mcastAddr.portmask.bits[0] ,mcastAddr.fid);
	}

	return SUCCESS;
}

#endif

int32 _rtk_igmp_hwRegInit(void)
{	
	if(igmpSysdb.regMdbCb)
		rtk_fc_igmp_mdb_callback_register(fc_igmp_mdb_callback_search);
	else
		rtk_fc_igmp_mdb_callback_unregister();
	return rt_igmp_multicastMode_set(RT_IGMP_MULTICAST_SYNC_AUTO);
}
int32 _rtk_igmp_hwUnRegClear(void)
{
	//always unregister callback
	rtk_fc_igmp_mdb_callback_unregister();
	return 0;
}

int32 _rtk_igmp_unknownIpMc_Action(rtk_igmp_unknownMcCbEvt_t *unknownCtrl)
{
	int ret=0;
	rt_igmp_unknownMc_cfg_t mcUnknCfg;

	memset(&mcUnknCfg,0,sizeof(mcUnknCfg));

	mcUnknCfg.isipv6 = unknownCtrl->isIpv6;

	mcUnknCfg.trapPkt = (unknownCtrl->unknownAct==UNKNOWN_MC_DROP)?0:1;

	ret=rt_igmp_unknownMcAction_set(mcUnknCfg);

	if(ret)
		IGMP_HWCB("ret=%d error",ret);

	return SUCCESS;
}

int32 _rtk_igmp_flowDelCbEvt(rtk_igmp_flowCbEvt_t *flowCb)
{
	int ret=0;
	rt_igmp_group_devPort_cfg_t mcDevConfig;

#if defined(CONFIG_RTL8367_SUPPORT) && defined(CONFIG_RTL9607C_SERIES) && defined(CONFIG_RTK_SKB_MARK2)
	rtk_igmp_switch8367_extPortHwControl(flowCb);
#endif
	memset(&mcDevConfig,0,sizeof(mcDevConfig));
	mcDevConfig.groupBehavior=RT_MC_BEHAVIOR_FLOW_FORWAED;
	mcDevConfig.is_ipv6=flowCb->isIPv6;

	mcDevConfig.careIngressCvid=IF_VLANCARE;
	mcDevConfig.careSourceAddress=IF_SOURCECARE;

	if(flowCb->ingressCvlan==FAIL)
		mcDevConfig.ingress_ctagif=0;
	else
	{
		mcDevConfig.ingress_ctagif=1;
		mcDevConfig.ingress_cvid = flowCb->ingressCvlan;
	}
	
	if(flowCb->ingressSvlan==FAIL)
		mcDevConfig.ingress_stagif=0;
	else
	{
		mcDevConfig.ingress_stagif=1;
		mcDevConfig.ingress_svid = flowCb->ingressSvlan;
	}


	if(flowCb->isIPv6)
		memcpy(&mcDevConfig.source_addr.ipv6[0],&flowCb->sourceAddr[0],sizeof(mcDevConfig.source_addr));
	else
		mcDevConfig.source_addr.ipv4=flowCb->sourceAddr[0];


	if(flowCb->isIPv6)
		memcpy(&mcDevConfig.group_addr.ipv6[0],&flowCb->group[0],sizeof(mcDevConfig.group_addr));
	else
		mcDevConfig.group_addr.ipv4=flowCb->group[0];

	ret = rt_igmp_multicastGroupDev_set (mcDevConfig);

//may delete same sip/dip but  dport/sport different not a error
//	if(ret)
//		IGMP_WARNING("ret=%d Error",ret);

	return ret;

}

int32 _rtk_igmp_groupFlowPrepare_devPort_cfg(rtk_igmp_flowCbEvt_t *flowCb,rt_igmp_group_devPort_cfg_t *mcConfig,int32 groupBehavior)
{
	if(igmpSysdb.hwCbMode==RTK_IGMP_CONFIG_BY_IFIDX || igmpSysdb.hwCbMode==RTK_IGMP_CONFIG_BY_PORT)
	{
		int i,j;
		int fwdCnt=0;
		rtk_fc_wlan_devidx_t wlanDevIdx;
		rt_wlan_mbssid_mask_t wlanMbssidMask[RT_WLAN_DEVICE_MAX];
		rtk_fc_wlan_devmask_t wlanDevIdMask;
		rt_igmp_group_devPort_cfg_t *mcDevConfig=mcConfig;

		i=0;j=0;
		memset(mcDevConfig,0,sizeof(rt_igmp_group_devPort_cfg_t));
		mcDevConfig->groupBehavior=groupBehavior;
		if(igmpSysdb.hwCbMode==RTK_IGMP_CONFIG_BY_IFIDX)
			mcDevConfig->egrInfoType = EGRINFO_DEVIFIDX;
		else
			mcDevConfig->egrInfoType = EGRINFO_PID;

		mcDevConfig->is_ipv6=flowCb->isIPv6;

		mcDevConfig->careIngressCvid=IF_VLANCARE;
		mcDevConfig->careIngressSvid=IF_VLANCARE;
		mcDevConfig->careSourceAddress=IF_SOURCECARE;

		if(flowCb->ingressCvlan==FAIL)
			mcDevConfig->ingress_ctagif=0;
		else
		{
			mcDevConfig->ingress_ctagif=1;
			mcDevConfig->ingress_cvid = flowCb->ingressCvlan;
		}

		if(flowCb->ingressSvlan==FAIL)
			mcDevConfig->ingress_stagif=0;
		else
		{
			mcDevConfig->ingress_stagif=1;
			mcDevConfig->ingress_svid = flowCb->ingressSvlan;
		}


		if(flowCb->isIPv6)
			memcpy(&mcDevConfig->source_addr.ipv6[0],&flowCb->sourceAddr[0],sizeof(mcDevConfig->source_addr));
		else
			mcDevConfig->source_addr.ipv4=flowCb->sourceAddr[0];


		if(flowCb->isIPv6)
			memcpy(&mcDevConfig->group_addr.ipv6[0],&flowCb->group[0],sizeof(mcDevConfig->group_addr));
		else
			mcDevConfig->group_addr.ipv4=flowCb->group[0];

		mcDevConfig->copy2cpu = flowCb->copy2cpu;

		if(igmpSysdb.hwCbMode==RTK_IGMP_CONFIG_BY_IFIDX)
		{
			for(i=0;i<IGMP_MAX_DEV_NUM;i++)
			{
				if(flowCb->egressDevInfo[i].valid)
				{
					if(fwdCnt>=RT_MCCFG_MAX_NUM)
					{
						IGMP_WARNING("fwdCnt out-of-range Error");
						return FAIL;
					}
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE) && defined(IGMP_SUPPORT_MCTOUC)					
					if(flowCb->egressDevInfo[i].subDmac[0].valid)
					{
						for(j=0;j<IGMP_MAX_EXTDEV_MAC_NUM;j++)
						{
							if(flowCb->egressDevInfo[i].subDmac[j].valid)
							{
								mcDevConfig->egrInfo.devIfidx[fwdCnt].dmacTrans=1;
								memcpy(mcDevConfig->egrInfo.devIfidx[fwdCnt].dmac,flowCb->egressDevInfo[i].subDmac[j].dmac,6);
								mcDevConfig->egrInfo.devIfidx[fwdCnt].ifindex = flowCb->egressDevInfo[i].devIfIdx;
								mcDevConfig->egrInfo.devIfidx[fwdCnt].resv0 = REALTEK_MAGIC_NUMBER; /*WARNING This is Realtek IGMP Module only internal Magic number ,Other Vendor DO NOT COPY THIS */
								fwdCnt++;
							}
							else
								break;

						}
					}
					else
#endif						
					{
						mcDevConfig->egrInfo.devIfidx[fwdCnt].ifindex = flowCb->egressDevInfo[i].devIfIdx;
						mcDevConfig->egrInfo.devIfidx[fwdCnt].resv0 = REALTEK_MAGIC_NUMBER; /*WARNING This is Realtek IGMP Module only internal Magic number ,Other Vendor DO NOT COPY THIS */
						fwdCnt++;
					}
					
				}
				else
					break;
			}

		}
		else
		{
			//Port Mode
			for(i=0;i<IGMP_MAX_DEV_NUM;i++)
			{
				if(flowCb->egressDevInfo[i].valid)
				{
					struct net_device *dev;
					rtk_fc_realdev_t rdev;
					
					igmp_spin_lock_bh(igmpSysdb.lock_igmp); 		
					dev = rtk_igmp_devIfidx_to_dev(flowCb->egressDevInfo[i].devIfIdx);
					igmp_spin_unlock_bh(igmpSysdb.lock_igmp);

					if(dev==NULL)
						continue;

					if(rtk_fc_dev_get_realdev_phyport(dev,&rdev)!=FAIL)
					{
						if(fwdCnt>=RT_MCCFG_MAX_NUM)
						{
							IGMP_WARNING("fwdCnt out-of-range Error");
							return FAIL;
						}
						
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE) && defined(IGMP_SUPPORT_MCTOUC)						
						if(flowCb->egressDevInfo[i].subDmac[0].valid)
						{
							for(j=0;j<IGMP_MAX_EXTDEV_MAC_NUM;j++)
							{
								if(flowCb->egressDevInfo[i].subDmac[j].valid)
								{
									mcDevConfig->egrInfo.portId[fwdCnt].isWlan=0;
									mcDevConfig->egrInfo.portId[fwdCnt].phyPortIdx=rdev.physicalPid;
									mcDevConfig->egrInfo.portId[fwdCnt].resv0 = REALTEK_MAGIC_NUMBER; /*WARNING This is Realtek IGMP Module only internal Magic number ,Other Vendor DO NOT COPY THIS */									
									memcpy(mcDevConfig->egrInfo.portId[fwdCnt].dmac,flowCb->egressDevInfo[i].subDmac[j].dmac,6);
									mcDevConfig->egrInfo.devIfidx[fwdCnt].dmacTrans=1;
									fwdCnt++;
								}
								else
									break;

							}
						}
						else
#endif							
						{
							mcDevConfig->egrInfo.portId[fwdCnt].isWlan=0;
							mcDevConfig->egrInfo.portId[fwdCnt].phyPortIdx=rdev.physicalPid;
							mcDevConfig->egrInfo.portId[fwdCnt].resv0 = REALTEK_MAGIC_NUMBER; /*WARNING This is Realtek IGMP Module only internal Magic number ,Other Vendor DO NOT COPY THIS */
							fwdCnt++;
						}
					}
					else if(rtk_fc_dev2wlanDevIdx(dev, &wlanDevIdx)!=FAIL)
					{
						rt_wlan_mbssid_index_t mbssidIdx;
						rt_wlan_index_t wlanIdx;
						wlanDevIdMask = ((rtk_fc_wlan_devmask_t)1LL) << wlanDevIdx;
						rtk_fc_wlan_devMask2RtmbssidMask(&wlanDevIdMask,wlanMbssidMask);
						for(wlanIdx=0 ; wlanIdx<RT_WLAN_DEVICE_MAX ; wlanIdx++)
						{
							for(mbssidIdx=0;mbssidIdx<RT_WLAN_MBSSID_MAX;mbssidIdx++)
							{
								if(wlanMbssidMask[wlanIdx] & (1<<mbssidIdx))
									goto FOUND_WLAN;
							}

						}
						if(0)
						{
FOUND_WLAN:
							if(fwdCnt>=RT_MCCFG_MAX_NUM)
							{
								IGMP_WARNING("fwdCnt out-of-range Error");
								return FAIL;
							}

#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE) && defined(IGMP_SUPPORT_MCTOUC)							
							if(flowCb->egressDevInfo[i].subDmac[0].valid)
							{
								for(j=0;j<IGMP_MAX_EXTDEV_MAC_NUM;j++)
								{
									if(flowCb->egressDevInfo[i].subDmac[j].valid)
									{
										mcDevConfig->egrInfo.portId[fwdCnt].isWlan=1;
										mcDevConfig->egrInfo.portId[fwdCnt].wlan.wlanDevIdx = wlanIdx;
										mcDevConfig->egrInfo.portId[fwdCnt].wlan.wlanMbssid = mbssidIdx;
										mcDevConfig->egrInfo.portId[fwdCnt].resv0 = REALTEK_MAGIC_NUMBER; /*WARNING This is Realtek IGMP Module only internal Magic number ,Other Vendor DO NOT COPY THIS */							
										memcpy(mcDevConfig->egrInfo.portId[fwdCnt].dmac,flowCb->egressDevInfo[i].subDmac[j].dmac,6);
										mcDevConfig->egrInfo.devIfidx[fwdCnt].dmacTrans=1;										
										fwdCnt++;
									}
									else
										break;

								}
							}
							else
#endif								
							{
								mcDevConfig->egrInfo.portId[fwdCnt].isWlan=1;
								mcDevConfig->egrInfo.portId[fwdCnt].wlan.wlanDevIdx = wlanIdx;
								mcDevConfig->egrInfo.portId[fwdCnt].wlan.wlanMbssid = mbssidIdx;
								mcDevConfig->egrInfo.portId[fwdCnt].resv0 = REALTEK_MAGIC_NUMBER; /*WARNING This is Realtek IGMP Module only internal Magic number ,Other Vendor DO NOT COPY THIS */							
								fwdCnt++;
							}
						}
					}
					
				}
				else
					break;
			}
		}
		mcDevConfig->cntEgrInfo = fwdCnt;

		return SUCCESS;
	}

	return SUCCESS;
}



int32 _rtk_igmp_groupFlowPrepare_pmskgroup_cfg(rtk_igmp_flowCbEvt_t *flowCb,rt_igmp_multicast_group_cfg_t *mcConfig,int32 groupBehavior)
{
	if(igmpSysdb.hwCbMode==RTK_IGMP_CONFIG_BY_PORTMASK)
	{
		int i,j;
		rtk_fc_wlan_devidx_t wlanDevIdx;
		rtk_fc_wlan_devmask_t wlanDevIdMask;
		rt_wlan_mbssid_mask_t wlanMbssidMask[RT_WLAN_DEVICE_MAX];
		rt_igmp_multicast_group_cfg_t *mcPmskConfig=mcConfig;

		//if(flowCb->l4proto != IPPROTO_UDP)
			//return FAIL;

		memset(mcPmskConfig,0,sizeof(rt_igmp_multicast_group_cfg_t));
		mcPmskConfig->groupBehavior=groupBehavior;
		mcPmskConfig->is_ipv6=flowCb->isIPv6;

		mcPmskConfig->careIngressCvid=IF_VLANCARE;
		mcPmskConfig->careIngressSvid=IF_VLANCARE;
		mcPmskConfig->careSourceAddress=IF_SOURCECARE;

		if(flowCb->ingressCvlan==FAIL)
			mcPmskConfig->ingress_ctagif=0;
		else
		{
			mcPmskConfig->ingress_ctagif=1;
			mcPmskConfig->ingress_cvid = flowCb->ingressCvlan;
		}
		
		if(flowCb->ingressSvlan==FAIL)
			mcPmskConfig->ingress_stagif=0;
		else
		{
			mcPmskConfig->ingress_stagif=1;
			mcPmskConfig->ingress_svid = flowCb->ingressSvlan;
		}


		if(flowCb->isIPv6)
			memcpy(&mcPmskConfig->source_addr.ipv6[0],&flowCb->sourceAddr[0],sizeof(mcPmskConfig->source_addr));
		else
			mcPmskConfig->source_addr.ipv4=flowCb->sourceAddr[0];


		if(flowCb->isIPv6)
			memcpy(&mcPmskConfig->group_addr.ipv6[0],&flowCb->group[0],sizeof(mcPmskConfig->group_addr));
		else
			mcPmskConfig->group_addr.ipv4=flowCb->group[0];


		//we only used first_act_portmask only ( first_act_portmask == (first_act_portmask|second_act_portmask)
		for(i=0;i<IGMP_MAX_DEV_NUM;i++)
		{
			if(flowCb->egressDevInfo[i].valid)
			{
				struct net_device *dev;
				rtk_fc_realdev_t rdev;
				
				igmp_spin_lock_bh(igmpSysdb.lock_igmp); 		
				dev = rtk_igmp_devIfidx_to_dev(flowCb->egressDevInfo[i].devIfIdx);
				igmp_spin_unlock_bh(igmpSysdb.lock_igmp);

				if(dev==NULL)
					continue;

				if(rtk_fc_dev_get_realdev_phyport(dev,&rdev)!=FAIL)
				{
					if(mcPmskConfig->first_act_portmask & (1 << rdev.physicalPid) )
					{
						//multi-packet send to same physical port not support for now
						IGMP_DATA("multi-packet send to same physical port not support for now");
						return FAIL;
					}
					else
					{
						mcPmskConfig->first_act_portmask |= (1 << rdev.physicalPid);
					}
				}
				else if(rtk_fc_dev2wlanDevIdx(dev, &wlanDevIdx)!=FAIL)
				{
					wlanDevIdMask = ((rtk_fc_wlan_devmask_t)1) << wlanDevIdx;
					rtk_fc_wlan_devMask2RtmbssidMask(&wlanDevIdMask,wlanMbssidMask);
					for(j=0 ; j<RT_WLAN_DEVICE_MAX ; j++)
						mcPmskConfig->first_act_wlanMbssidMask[j] |= wlanMbssidMask[j];
				}
				
			}
			else
				break;
		}
		IGMP_HWCB("set first_action_portmask to %x first_act_wlanMbssidMask[ %x %x %x ]",mcPmskConfig->first_act_portmask,mcPmskConfig->first_act_wlanMbssidMask[0],mcPmskConfig->first_act_wlanMbssidMask[1],mcPmskConfig->first_act_wlanMbssidMask[2]);

		return SUCCESS;
	}
	return SUCCESS;
}

/*
l4proto/dport/sport  will miss here
*/
int32 _rtk_igmp_flowSetCbEvt(rtk_igmp_flowCbEvt_t *flowCb)
{
#if defined(CONFIG_RTL8367_SUPPORT) && defined(CONFIG_RTL9607C_SERIES) && defined(CONFIG_RTK_SKB_MARK2)
		rtk_igmp_switch8367_extPortHwControl(flowCb);
#endif

	if(igmpSysdb.hwCbMode==RTK_IGMP_CONFIG_BY_IFIDX || igmpSysdb.hwCbMode==RTK_IGMP_CONFIG_BY_PORT)
	{
		int ret=0;
		rt_igmp_group_devPort_cfg_t mcDevConfig;

		if(_rtk_igmp_groupFlowPrepare_devPort_cfg(flowCb,&mcDevConfig,RT_MC_BEHAVIOR_FLOW_FORWAED)==SUCCESS)
		{
			ret = rt_igmp_multicastGroupDev_set (mcDevConfig);
			if(ret)
				IGMP_WARNING("ret=%d Error",ret);
		}
		return ret;
	}
	else if(igmpSysdb.hwCbMode==RTK_IGMP_CONFIG_BY_PORTMASK)
	{
		int ret=0;
		rt_igmp_multicast_group_cfg_t mcConfig;

		if(_rtk_igmp_groupFlowPrepare_pmskgroup_cfg(flowCb,&mcConfig,RT_MC_BEHAVIOR_FLOW_FORWAED)==SUCCESS)
		{
			ret = rt_igmp_multicastGroup_set (mcConfig);
			if(ret)
				IGMP_WARNING("ret=%d Error",ret);
		}

		return ret;
	}
	return SUCCESS;
}


/*
l4proto/dport/sport  will miss here
*/
int32 _rtk_igmp_flowUpdateCbEvt(rtk_igmp_flowCbEvt_t *flowCb)
{
#if defined(CONFIG_RTL8367_SUPPORT) && defined(CONFIG_RTL9607C_SERIES) && defined(CONFIG_RTK_SKB_MARK2)
	rtk_igmp_switch8367_extPortHwControl(flowCb);
#endif
	if(igmpSysdb.hwCbMode==RTK_IGMP_CONFIG_BY_IFIDX || igmpSysdb.hwCbMode==RTK_IGMP_CONFIG_BY_PORT)
	{
		int ret=0;
		rt_igmp_group_devPort_cfg_t mcDevConfig;

		if(_rtk_igmp_groupFlowPrepare_devPort_cfg(flowCb,&mcDevConfig,RT_MC_BEHAVIOR_FLOW_UPDATE)==SUCCESS)
		{
			ret = rt_igmp_multicastGroupDev_set (mcDevConfig);
			if(ret)
				IGMP_WARNING("ret=%d Error",ret);
		}
		return ret;
	}
	else if(igmpSysdb.hwCbMode==RTK_IGMP_CONFIG_BY_PORTMASK)
	{
		int ret=0;
		rt_igmp_multicast_group_cfg_t mcConfig;

		if(_rtk_igmp_groupFlowPrepare_pmskgroup_cfg(flowCb,&mcConfig,RT_MC_BEHAVIOR_FLOW_UPDATE)==SUCCESS)
		{
			ret = rt_igmp_multicastGroup_set (mcConfig);
			if(ret)
				IGMP_WARNING("ret=%d Error",ret);
		}

		return ret;
	}
	return SUCCESS;
}





int32 _rtk_igmp_groupAddCbEvt(rtk_igmp_groupCbEvt_t *groupCb)
{
	if(igmpSysdb.hwCbMode==RTK_IGMP_CONFIG_BY_IFIDX || igmpSysdb.hwCbMode==RTK_IGMP_CONFIG_BY_PORT)
	{
		int ret=0;
		rt_igmp_group_devPort_cfg_t mcDevConfig;
		memset(&mcDevConfig,0,sizeof(mcDevConfig));

		mcDevConfig.careIngressCvid=IF_VLANCARE;
		mcDevConfig.careIngressSvid=IF_VLANCARE;
		mcDevConfig.careSourceAddress=IF_SOURCECARE;

		mcDevConfig.groupBehavior=RT_MC_BEHAVIOR_GROUP_AS_KNOW;
		mcDevConfig.is_ipv6=groupCb->isIPv6;

		if(groupCb->isIPv6)
			memcpy(&mcDevConfig.group_addr.ipv6[0],&groupCb->group[0],sizeof(mcDevConfig.group_addr));
		else
			mcDevConfig.group_addr.ipv4=groupCb->group[0];

		ret = rt_igmp_multicastGroupDev_set(mcDevConfig);

		if(ret)
			IGMP_WARNING("ret=%d Error",ret);

		return SUCCESS;
	}
	else if(igmpSysdb.hwCbMode==RTK_IGMP_CONFIG_BY_PORTMASK)
	{
		int ret=0;
		rt_igmp_multicast_group_cfg_t mcConfig;
		memset(&mcConfig,0,sizeof(mcConfig));

		mcConfig.careIngressCvid=IF_VLANCARE;
		mcConfig.careIngressSvid=IF_VLANCARE;
		mcConfig.careSourceAddress=IF_SOURCECARE;

		mcConfig.groupBehavior=RT_MC_BEHAVIOR_GROUP_AS_KNOW;
		mcConfig.is_ipv6=groupCb->isIPv6;

		if(groupCb->isIPv6)
			memcpy(&mcConfig.group_addr.ipv6[0],&groupCb->group[0],sizeof(mcConfig.group_addr));
		else
			mcConfig.group_addr.ipv4=groupCb->group[0];

		ret = rt_igmp_multicastGroup_set (mcConfig);

		if(ret)
			IGMP_WARNING("ret=%d Error",ret);

		return SUCCESS;
	}
	return SUCCESS;
}

int32 _rtk_igmp_groupDelCbEvt(rtk_igmp_groupCbEvt_t *groupCb)
{
	if(igmpSysdb.hwCbMode==RTK_IGMP_CONFIG_BY_IFIDX || igmpSysdb.hwCbMode==RTK_IGMP_CONFIG_BY_PORT)
	{
		int ret=0;
		rt_igmp_group_devPort_cfg_t mcDevConfig;
		memset(&mcDevConfig,0,sizeof(mcDevConfig));

		mcDevConfig.careIngressCvid=IF_VLANCARE;
		mcDevConfig.careIngressSvid=IF_VLANCARE;
		mcDevConfig.careSourceAddress=IF_SOURCECARE;;

		mcDevConfig.groupBehavior=RT_MC_BEHAVIOR_GROUP_AS_UNKNOW;
		mcDevConfig.is_ipv6=groupCb->isIPv6;
		mcDevConfig.copy2cpu = groupCb->asunknownNotFlush;

		if(groupCb->isIPv6)
			memcpy(&mcDevConfig.group_addr.ipv6[0],&groupCb->group[0],sizeof(mcDevConfig.group_addr));
		else
			mcDevConfig.group_addr.ipv4=groupCb->group[0];

		ret = rt_igmp_multicastGroupDev_set (mcDevConfig);

		if(ret)
			IGMP_WARNING("ret=%d Error",ret);

		return SUCCESS;
	}
	else if(igmpSysdb.hwCbMode==RTK_IGMP_CONFIG_BY_PORTMASK)
	{
		int ret=0;
		rt_igmp_multicast_group_cfg_t mcConfig;
		memset(&mcConfig,0,sizeof(mcConfig));

		mcConfig.careIngressCvid=IF_VLANCARE;
		mcConfig.careIngressSvid=IF_VLANCARE;
		mcConfig.careSourceAddress=IF_SOURCECARE;;

		mcConfig.groupBehavior=RT_MC_BEHAVIOR_GROUP_AS_UNKNOW;
		mcConfig.is_ipv6=groupCb->isIPv6;

		if(groupCb->isIPv6)
			memcpy(&mcConfig.group_addr.ipv6[0],&groupCb->group[0],sizeof(mcConfig.group_addr));
		else
			mcConfig.group_addr.ipv4=groupCb->group[0];

		ret = rt_igmp_multicastGroup_set (mcConfig);

		if(ret)
			IGMP_WARNING("ret=%d Error",ret);

		return SUCCESS;
	}
	return SUCCESS;
}





#define IGMP_MAX_WQ_SIZE 512 

typedef struct rtk_igmp_CbWorkq_s
{
	struct work_struct work;
	union
	{
		rtk_igmp_groupCbEvt_t groupCb;
		rtk_igmp_flowCbEvt_t flowCb;
		rtk_igmp_unknownMcCbEvt_t unknownCtrl;
	}data;
	struct list_head work_list;
}rtk_igmp_CbWorkq_t;

rtk_igmp_CbWorkq_t callbackWorkq[IGMP_MAX_WQ_SIZE];
struct list_head listFreeHead_workq;

atomic_t wqAllocCnt=ATOMIC_INIT(0);
atomic_t wqFreeCnt=ATOMIC_INIT(0);
atomic_t wqInit=ATOMIC_INIT(0);

int32 rtk_igmp_callbackwq_flushInit(void)
{
	int i;
	igmp_spin_lock_bh(igmpSysdb.lock_callBack);	
	INIT_LIST_HEAD(&listFreeHead_workq);
	for(i=0;i<IGMP_MAX_WQ_SIZE;i++)
	{
		memset(&callbackWorkq[i],0,sizeof(rtk_igmp_CbWorkq_t));
		INIT_LIST_HEAD(&callbackWorkq[i].work_list);
		list_add_tail(&callbackWorkq[i].work_list, &listFreeHead_workq);	
	}
	igmp_spin_unlock_bh(igmpSysdb.lock_callBack);
	return SUCCESS;
}


rtk_igmp_CbWorkq_t * rtk_igmp_callbackwq_malloc(void)
{
	rtk_igmp_CbWorkq_t *callbackwq_entry;
	rtk_igmp_CbWorkq_t *callbackwq_entry_tmp;

	if(atomic_read(&wqInit)==0)
	{
		rtk_igmp_callbackwq_flushInit();
		atomic_inc(&wqInit);
	}
	igmp_spin_lock_bh(igmpSysdb.lock_callBack); 

	if(list_empty(&listFreeHead_workq))
	{
		IGMP_HWCB("all free IGMP listFreeHead_workq are allocated...");
		igmp_spin_unlock_bh(igmpSysdb.lock_callBack);
		return (NULL);
	}
	list_for_each_entry_safe(callbackwq_entry,callbackwq_entry_tmp,&listFreeHead_workq,work_list)		//just return the first entry right behind of head
	{
		list_del_init(&callbackwq_entry->work_list);
		break;
	}
	igmp_spin_unlock_bh(igmpSysdb.lock_callBack);

	memset(&callbackwq_entry->data,0,sizeof(callbackwq_entry->data));
	
	return callbackwq_entry;
}



int rtk_igmp_callbackwq_free(rtk_igmp_CbWorkq_t* workq)
{
	igmp_spin_lock_bh(igmpSysdb.lock_callBack);	
	list_del_init(&workq->work_list);
	list_add(&workq->work_list, &listFreeHead_workq);
	igmp_spin_unlock_bh(igmpSysdb.lock_callBack);
	return (SUCCESS);
}



void w_rtk_igmp_hwRegInit(struct work_struct *p_work)
{
	int ret;
	rtk_igmp_CbWorkq_t *p_callback_work = container_of(p_work, rtk_igmp_CbWorkq_t, work);
	ret = _rtk_igmp_hwRegInit();
	atomic_inc(&wqFreeCnt);
	rtk_igmp_callbackwq_free(p_callback_work);
	if(ret)
		printk("igmp callback w_rtk_igmp_hwRegInit return error %d",ret );
}
void w_rtk_igmp_hwUnRegClear(struct work_struct *p_work)
{
	int ret;
	rtk_igmp_CbWorkq_t *p_callback_work = container_of(p_work, rtk_igmp_CbWorkq_t, work);
	ret = _rtk_igmp_hwUnRegClear();
	atomic_inc(&wqFreeCnt);
	rtk_igmp_callbackwq_free(p_callback_work);
	if(ret)
		printk("igmp callback w_rtk_igmp_hwUnRegClear return error %d",ret );	
}
void w_rtk_igmp_unknownIpMc_Action(struct work_struct *p_work)
{
	int ret;
	rtk_igmp_CbWorkq_t *p_callback_work = container_of(p_work, rtk_igmp_CbWorkq_t, work);
	ret = _rtk_igmp_unknownIpMc_Action(&p_callback_work->data.unknownCtrl);
	atomic_inc(&wqFreeCnt);
	rtk_igmp_callbackwq_free(p_callback_work);
	if(ret)
		printk("igmp callback w_rtk_igmp_unknownIpMc_Action return error %d",ret );	
}
void w_rtk_igmp_flowDelCbEvt(struct work_struct *p_work)
{
	int ret;
	rtk_igmp_CbWorkq_t *p_callback_work = container_of(p_work, rtk_igmp_CbWorkq_t, work);
	ret = _rtk_igmp_flowDelCbEvt(&p_callback_work->data.flowCb);
	atomic_inc(&wqFreeCnt);
	rtk_igmp_callbackwq_free(p_callback_work);
	if(ret)
		printk("igmp callback w_rtk_igmp_flowDelCbEvt return error %d",ret );	
}
void w_rtk_igmp_flowSetCbEvt(struct work_struct *p_work)
{
	int ret;
	rtk_igmp_CbWorkq_t *p_callback_work = container_of(p_work, rtk_igmp_CbWorkq_t, work);
	ret = _rtk_igmp_flowSetCbEvt(&p_callback_work->data.flowCb);
	atomic_inc(&wqFreeCnt);
	rtk_igmp_callbackwq_free(p_callback_work);
	if(ret)
		printk("igmp callback w_rtk_igmp_flowSetCbEvt return error %d",ret );	
}
void w_rtk_igmp_flowUpdateCbEvt(struct work_struct *p_work)
{
	int ret;
	rtk_igmp_CbWorkq_t *p_callback_work = container_of(p_work, rtk_igmp_CbWorkq_t, work);
	ret = _rtk_igmp_flowUpdateCbEvt(&p_callback_work->data.flowCb);
	atomic_inc(&wqFreeCnt);
	rtk_igmp_callbackwq_free(p_callback_work);
	if(ret)
		printk("igmp callback w_rtk_igmp_flowUpdateCbEvt return error %d",ret );	
}

void w_rtk_igmp_groupAddCbEvt(struct work_struct *p_work)
{
	int ret;
	rtk_igmp_CbWorkq_t *p_callback_work = container_of(p_work, rtk_igmp_CbWorkq_t, work);
	ret = _rtk_igmp_groupAddCbEvt(&p_callback_work->data.groupCb);
	atomic_inc(&wqFreeCnt);
	rtk_igmp_callbackwq_free(p_callback_work);
	if(ret)
		printk("igmp callback w_rtk_igmp_groupAddCbEvt return error %d",ret );	
}
void w_rtk_igmp_groupDelCbEvt(struct work_struct *p_work)
{
	int ret;
	rtk_igmp_CbWorkq_t *p_callback_work = container_of(p_work, rtk_igmp_CbWorkq_t, work);
	ret = _rtk_igmp_groupDelCbEvt(&p_callback_work->data.groupCb);
	atomic_inc(&wqFreeCnt);
	rtk_igmp_callbackwq_free(p_callback_work);
	if(ret)
		printk("igmp callback w_rtk_igmp_groupDelCbEvt return error %d",ret );	
}


int32 rtk_igmp_hwRegInit(void)
{
	int32 ret;
	rtk_igmp_CbWorkq_t *work = rtk_igmp_callbackwq_malloc();
	if(work==NULL)
	{
		printk("igmp no Menory Error");
		return SUCCESS;
	}
	atomic_inc(&wqAllocCnt);
	INIT_WORK(&work->work, w_rtk_igmp_hwRegInit);
	ret=schedule_work(&work->work);
	if(ret==false)
		printk("Wq Error\n");	
	return SUCCESS;
}
int32 rtk_igmp_hwUnRegClear(void)
{
	int32 ret;
	rtk_igmp_CbWorkq_t *work =  rtk_igmp_callbackwq_malloc();
	if(work==NULL)
	{
		printk("igmp no Menory Error");
		return SUCCESS;
	}	
	atomic_inc(&wqAllocCnt);
	INIT_WORK(&work->work, w_rtk_igmp_unknownIpMc_Action);
	ret=schedule_work(&work->work);
	if(ret==false)
		printk("Wq Error\n");	
	return SUCCESS;
}
int32 rtk_igmp_unknownIpMc_Action(rtk_igmp_unknownMcCbEvt_t *unknownCtrl)
{
	int32 ret;
	rtk_igmp_CbWorkq_t *work = rtk_igmp_callbackwq_malloc();
	if(work==NULL)
	{
		printk("igmp no Menory Error");
		return SUCCESS;
	}	
	atomic_inc(&wqAllocCnt);
	INIT_WORK(&work->work, w_rtk_igmp_unknownIpMc_Action);
	memcpy(&work->data.unknownCtrl,unknownCtrl,sizeof(work->data.unknownCtrl));
	ret=schedule_work(&work->work);
	if(ret==false)
		printk("Wq Error\n");	
	return SUCCESS;
}
int32 rtk_igmp_flowDelCbEvt(rtk_igmp_flowCbEvt_t *flowCb)
{
	int32 ret;
	rtk_igmp_CbWorkq_t *work =  rtk_igmp_callbackwq_malloc();
	if(work==NULL)
	{
		printk("igmp no Menory Error");
		return SUCCESS;
	}	
	atomic_inc(&wqAllocCnt);
	INIT_WORK(&work->work, w_rtk_igmp_flowDelCbEvt);
	memcpy(&work->data.flowCb,flowCb,sizeof(work->data.flowCb));
	ret=schedule_work(&work->work);
	if(ret==false)
		printk("Wq Error\n");	
	return SUCCESS;
}
int32 rtk_igmp_flowSetCbEvt(rtk_igmp_flowCbEvt_t *flowCb)
{
	int32 ret;
	rtk_igmp_CbWorkq_t *work =  rtk_igmp_callbackwq_malloc();
	if(work==NULL)
	{
		printk("igmp no Menory Error");
		return SUCCESS;
	}	
	atomic_inc(&wqAllocCnt);
	INIT_WORK(&work->work, w_rtk_igmp_flowSetCbEvt);
	memcpy(&work->data.flowCb,flowCb,sizeof(work->data.flowCb));
	ret=schedule_work(&work->work);
	if(ret==false)
		printk("Wq Error\n");	
	return SUCCESS;
}
int32 rtk_igmp_flowUpdateCbEvt(rtk_igmp_flowCbEvt_t *flowCb)
{
	int32 ret;
	rtk_igmp_CbWorkq_t *work =  rtk_igmp_callbackwq_malloc();
	if(work==NULL)
	{
		printk("igmp no Menory Error");
		return SUCCESS;
	}	
	atomic_inc(&wqAllocCnt);
	INIT_WORK(&work->work, w_rtk_igmp_flowUpdateCbEvt);
	memcpy(&work->data.flowCb,flowCb,sizeof(work->data.flowCb));
	ret=schedule_work(&work->work);
	if(ret==false)
		printk("Wq Error\n");	
	return SUCCESS;
}

int32 rtk_igmp_groupAddCbEvt(rtk_igmp_groupCbEvt_t *groupCb)
{
	int32 ret;
	rtk_igmp_CbWorkq_t *work =  rtk_igmp_callbackwq_malloc();
	if(work==NULL)
	{
		printk("igmp no Menory Error");
		return SUCCESS;
	}	
	atomic_inc(&wqAllocCnt);
	INIT_WORK(&work->work, w_rtk_igmp_groupAddCbEvt);
	memcpy(&work->data.groupCb,groupCb,sizeof(work->data.groupCb));	
	ret=schedule_work(&work->work);
	if(ret==false)
		printk("Wq Error\n");	
	return SUCCESS;
}
int32 rtk_igmp_groupDelCbEvt(rtk_igmp_groupCbEvt_t *groupCb)
{
	int32 ret;
	rtk_igmp_CbWorkq_t *work =  rtk_igmp_callbackwq_malloc();
	if(work==NULL)
	{
		printk("igmp no Menory Error");
		return SUCCESS;
	}	
	atomic_inc(&wqAllocCnt);
	INIT_WORK(&work->work, w_rtk_igmp_groupDelCbEvt);
	memcpy(&work->data.groupCb,groupCb,sizeof(work->data.groupCb));	
	ret=schedule_work(&work->work);
	if(ret==false)
		printk("Wq Error\n");
	return SUCCESS;
}

