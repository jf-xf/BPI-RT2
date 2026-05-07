#include "rtk_fc_struct.h"
#include "rtk_fc_callback.h"
#include "rtk_fc_define.h"
#include "rtk_fc_debug.h"
#include "rtk_fc_internal.h"
#include "rtk_fc_driver.h"
#include "rtk_fc_vxlan.h"
#include "rtk_fc_mappingAPI.h"
#include "rtk_fc_acl.h"

#include "rtk_fc_dualHeader.h"

#include <uapi/linux/if_ether.h>
#include <uapi/linux/if_pppox.h>
#if defined(CONFIG_FC_CA8277B_SERIES)
#include <ca_ni_tx.h>

extern rtk_scfg_t rtkScfg;
extern int ca_ni_init_tx_dma_lso_special_fast_fwd(u8 *vxlanContentBuffer, int vxlanContentBufferSize, int inner_extra_len);
extern int ca_ni_init_set_inner_extra_len(int direction, int inner_extra_len);
extern int ca_ni_init_tx_dma_lso_special_fast_fwd_init_skb_pool(void);
extern int ca_ni_init_extra_ff_global_param_setting(void);
extern int ca_ni_init_tx_dma_lso_extra_ff_setting(u8 *vxlanContentBuffer, int vxlanContentBufferSize, int inner_extra_len, int *extraIndex);
extern int ca_ni_init_tx_dma_lso_extra_ff_del(int extraIndex);

#endif
#if defined(CONFIG_FC_RTL9607C_SERIES) && defined(CONFIG_SMP)
static cpumask_t cpumask_gmac0 = {{0}};
static cpumask_t cpumask_gmac1 = {{0}};
static cpumask_t cpumask_gmac2 = {{0}};

#endif


void rtk_fc_vxlan_hexdump(unsigned char *buf, unsigned int len)
{
	print_hex_dump(KERN_CONT, "", DUMP_PREFIX_OFFSET,
			16, 1,
			buf, len, false);
}



#if defined(CONFIG_FC_RTL9607C_SERIES)
extern uint32 dynamic_sram_desc;


int _rtk_fc_vxlan_up_stream_innerFlow_prepare(rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, rtk_fc_pktHdr_t *pPktHdr, rtk_rg_asic_path5_entry_t *pFlowPath5, struct rt_skbuff *rtskb)
{
	unsigned char extraFwd,prepared,modified;
	int ret, virtual_indMacIdx;
	struct rtl8686_hwnat_customized_entry customized_entry={0};
 
	{
		DEBUG("pPktHdr->ingressPort.macPortIdx = %d vxlan_acceleration_extraPmsk = %x",pPktHdr->ingressPort.macPortIdx, fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraPmsk);
		if((0x1<<pPktHdr->ingressPort.macPortIdx)&fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraPmsk){
			extraFwd=1;
			customized_entry.gmac=fc_db_fastFwd_data.vxlan_acc.vxlan_extraGmac; // 0
			customized_entry.rxRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamRxRingNum; // 4
			customized_entry.txRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamTxRingNum; // 3
			prepared=fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamInfo_prepared;  // 1
			modified=fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamRing_modified;  // 1
			pFlowPath5->out_user_priority = fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamPriority; // 6
			ret = rtk_fc_indMac_idx_get(fc_db.vxlan_accelerate_extra_upstreamL2Idx, &virtual_indMacIdx);
			DEBUG("for extra outbound VXLAN flow!! virtual_indMacIdx = %d", virtual_indMacIdx); 
			if(fc_db.indMacTbl[virtual_indMacIdx].valid){
				fc_db.indMacTbl[virtual_indMacIdx].indMacRefCount++;
				DEBUG("indMacIdx = %d",virtual_indMacIdx);
			}
		}else{
			extraFwd=0;
			customized_entry.gmac=fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamGmac; // 1
			customized_entry.rxRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamRxRingNum; //4
			customized_entry.txRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamTxRingNum; //3
			prepared=fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamInfo_prepared;  //1
			modified=fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamRing_modified;   //1
			pFlowPath5->out_user_priority = fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamPriority; //6
			ret = rtk_fc_indMac_idx_get(fc_db.vxlan_accelerate_upstreamL2Idx, &virtual_indMacIdx);
			DEBUG("for outbound VXLAN flow!! virtual_indMacIdx = %d",virtual_indMacIdx);	
			if(fc_db.indMacTbl[virtual_indMacIdx].valid){
				fc_db.indMacTbl[virtual_indMacIdx].indMacRefCount++;
				DEBUG("indMacIdx = %d",virtual_indMacIdx);
			}
		}
		
		
		if(ret==RTK_FC_RET_OK)
		{
			pFlowPath5->out_dmac_idx = virtual_indMacIdx;
			pFlowPath5->out_user_pri_act = TRUE;
			pFlowPath5->out_extra_tag_index = 0;
			if(pPktHdr->cvh){
				pFlowPath5->out_cvlan_id = pPktHdr->cvlanid;
				pFlowPath5->out_cpri = 0;
				pFlowPath5->out_cvid_format_act = TRUE;
				pFlowPath5->out_cpri_format_act = FALSE;
			}else{
				pFlowPath5->out_cvlan_id = 0;
				pFlowPath5->out_cpri = 0;
				pFlowPath5->out_cvid_format_act = FALSE;
				pFlowPath5->out_cpri_format_act = FALSE;
			}					
			if(extraFwd && fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraMeter>=0){
				pFlowPath5->out_share_meter_act = TRUE;
				pFlowPath5->out_share_meter_idx = fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraMeter;
			}
			TRACE("Accelerate vxlan upstream by indMacIdx[%d]", pFlowPath5->out_dmac_idx);

			if(prepared && modified==0){
				customized_entry.valid=TRUE;
				customized_entry.txPreLen=fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length+sizeof(unsigned int);	//for v2
				customized_entry.txInfo_addr_offset_v1=fc_db_fastFwd_data.vxlan_acc.vxlan_upstream_txAddrOffset;
				customized_entry.txInfo_addr_offset_v2=RX_OFFSET+4;
				customized_entry.pCustomized_txInfo=(struct tx_info *)&fc_db_fastFwd_data.vxlan_acc.vxlan_upstream_txInfo;
				customized_entry.rx_ext_pmsk=RTK_FC_VXLAN_ACC_UPSTREAM_EXTPMAK; //(0x1<<4)
				customized_entry.type=CUSTOMIZE_TYPE_VXLAN_UP; //3
				if(extraFwd)
				{
					if(re8686_customized_rx_and_tx(customized_entry, rtk_fc_vxlan_customized_rxPrepare, rtk_fc_vxlan_customized_extraTxPrepare, rtk_fc_vxlan_customized_rxHook, rtk_fc_vxlan_customized_upstreamTxHook)==0)
						fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamRing_modified=1;
			
				}else{
				
					if(re8686_customized_rx_and_tx(customized_entry, rtk_fc_vxlan_customized_rxPrepare, rtk_fc_vxlan_customized_txPrepare, rtk_fc_vxlan_customized_rxHook, rtk_fc_vxlan_customized_upstreamTxHook)==0)
						fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamRing_modified=1;
				
				}
			}
		}

	}
							
	return SUCCESS;
}

void _rtk_fc_vxlan_fastfwd_downstream_setup_fc_db_fastFwd_data(void)
{
	fc_db_fastFwd_data.vxlan_acc.vxlan_downstream_txAddrOffset=RX_OFFSET+8+8+20+14;
				
	bzero(&fc_db_fastFwd_data.vxlan_acc.vxlan_downstream_txInfo,sizeof(fc_db_fastFwd_data.vxlan_acc.vxlan_downstream_txInfo));
	fc_db_fastFwd_data.vxlan_acc.vxlan_downstream_txInfo.opts2.bit.cputag=1;
	fc_db_fastFwd_data.vxlan_acc.vxlan_downstream_txInfo.opts3.bit.dislrn=1;
	
	fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamInfo_prepared=1;
	fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamInfo_prepared=1;
}
void _rtk_fc_vxlan_fastfwd_upstream_setup_fc_db_fastFwd_data(rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo,
																 int offset_without_vlan_pppoe,
																 u8 *vxlanContentBuffer_without_vlan_pppoe,
																 int isExtra)
{	
	if(fc_db.vxlan_fastFwd_data_general_setting_isSet == 0)
	{
		fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length=offset_without_vlan_pppoe;//pPktHdr->vxlan_outerTag_off;
		fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_udp_length_offset=14+20+4;
		fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_ip_length_offset=14+2;
		fc_db_fastFwd_data.vxlan_acc.vxlan_accelerated_intf_idx=0; // FC Doesn't care

		bzero(&fc_db_fastFwd_data.vxlan_acc.vxlan_upstream_txInfo,sizeof(fc_db_fastFwd_data.vxlan_acc.vxlan_upstream_txInfo));
		fc_db_fastFwd_data.vxlan_acc.vxlan_upstream_txInfo.opts2.bit.cputag=1;
		fc_db_fastFwd_data.vxlan_acc.vxlan_upstream_txInfo.opts3.bit.dislrn=1;

		memcpy(fc_db_fastFwd_data.vxlan_acc.vxlan_upstream_dmac,&pG3IgrExtraInfo->vxlan_info.vxlan_DA[0],4); // inner DA [0]-[4]
		
		if(fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length & 0xf)
			fc_db_fastFwd_data.vxlan_acc.vxlan_upstream_txAddrOffset=16-((fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length-RX_OFFSET)-((fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length>>4)<<4));
		else
			fc_db_fastFwd_data.vxlan_acc.vxlan_upstream_txAddrOffset=0;
		
		fc_db.vxlan_fastFwd_data_general_setting_isSet = 1;
	
	}

	if(fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length > sizeof(fc_db_fastFwd_data.vxlan_acc.vxlan_extra_preallocated_outer))
		WARNING("Unexpected outer conten len %d", fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length);
	else if(isExtra)
	{
		//Extra sport always use RTK_FC_VXLAN_ACC_EPHEMERAL_SPORT
		DEBUG("Setting extra port outer content");
		//Don't have vlan!
		memcpy(fc_db_fastFwd_data.vxlan_acc.vxlan_extra_preallocated_outer,&vxlanContentBuffer_without_vlan_pppoe[0],fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length);
		rtk_fc_vxlan_hexdump(fc_db_fastFwd_data.vxlan_acc.vxlan_extra_preallocated_outer, fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length);

		*( (u16*)(&fc_db_fastFwd_data.vxlan_acc.vxlan_extra_preallocated_outer[0]+34)) = htons(RTK_FC_VXLAN_ACC_EPHEMERAL_SPORT);
		fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamInfo_prepared=1;

	}
	else
	{	
		DEBUG("Setting port 0 outer content");
		//Don't have vlan!
		memcpy(fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer,&vxlanContentBuffer_without_vlan_pppoe[0],fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length);
	
		rtk_fc_vxlan_hexdump(vxlanContentBuffer_without_vlan_pppoe, fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length);
		fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamInfo_prepared=1;

	}
}
void _rtk_fc_vxlan_fastfwd_set_fake_l2_entry(int direction, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo)
{
	rtk_fc_pktHdr_t tmp_pktHdr;
	struct ethhdr tmp_eth;
	int ret = 0;
	int16 lutIdx = 0;

	if(direction ==RTK_FC_L2DUAL_ACT_ADD)
	{
		// Should be inner DA
		tmp_pktHdr.eth = &tmp_eth;
		memcpy(&tmp_pktHdr.eth->h_dest[0],&pG3IgrExtraInfo->vxlan_info.vxlan_DA[0],6);
		
		tmp_pktHdr.eth->h_dest[0] = 0x0;
		tmp_pktHdr.eth->h_dest[1] = 0x0;
		tmp_pktHdr.eth->h_dest[2] = 0x5e;
		tmp_pktHdr.eth->h_dest[3] = 0x0;
		if(fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamGmac==1)
		{
			tmp_pktHdr.egressPort.macPortIdx = RTK_FC_MAC_PORT_MASTERCPU_CORE1;
			tmp_pktHdr.egressPort.macExtPortIdx = RTK_FC_MAC_EXT_PORT3;
		}
		else if(fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamGmac==2)
		{
			tmp_pktHdr.egressPort.macPortIdx = RTK_FC_MAC_PORT_SLAVECPU;
			tmp_pktHdr.egressPort.macExtPortIdx = RTK_FC_MAC_EXT_PORT3;
		}
		else
		{
			tmp_pktHdr.egressPort.macPortIdx = RTK_FC_MAC_PORT_MASTERCPU_CORE0;
			tmp_pktHdr.egressPort.macExtPortIdx = RTK_FC_MAC_EXT_PORT3;
		}
		
		tmp_pktHdr.cvlanid = 0; //FIXME
		tmp_pktHdr.egrWlanDevIdx = RTK_FC_WLANX_NOT_FOUND;

		if(_rtk_fc_lut_find(tmp_pktHdr.eth->h_dest, &lutIdx) == RTK_FC_RET_OK)
		{
			DEBUG("[VXLAN]Upstream fake LUT Learned before, no need to learn again.");
			ret = RTK_FC_RET_OK;
		}
		else if((ret = _rtk_fc_lut_learning(tmp_pktHdr.eth->h_dest, tmp_pktHdr.egressPort, tmp_pktHdr.cvlanid, RTK_FC_WLANX_NOT_FOUND,
									FALSE, TRUE, &lutIdx, NULL, LUT_DIR_DONT_CARE)) != RTK_FC_RET_OK) 
		{
			WARNING("LUT DA learning is fail, ret = 0x%x", ret);
			//return ret;
		}
									
		fc_db.vxlan_accelerate_upstreamL2Idx = lutIdx;


		//00:00:5e:01:ff:ff
		tmp_pktHdr.eth->h_dest[3] = 0x01;	

		if(fc_db_fastFwd_data.vxlan_acc.vxlan_extraGmac==1)
		{
			tmp_pktHdr.egressPort.macPortIdx = RTK_FC_MAC_PORT_MASTERCPU_CORE1;
			tmp_pktHdr.egressPort.macExtPortIdx = RTK_FC_MAC_EXT_PORT3;
		}
		else if(fc_db_fastFwd_data.vxlan_acc.vxlan_extraGmac==2)
		{
			tmp_pktHdr.egressPort.macPortIdx = RTK_FC_MAC_PORT_SLAVECPU;
			tmp_pktHdr.egressPort.macExtPortIdx = RTK_FC_MAC_EXT_PORT3;
		}
		else
		{
			tmp_pktHdr.egressPort.macPortIdx = RTK_FC_MAC_PORT_MASTERCPU_CORE0;
			tmp_pktHdr.egressPort.macExtPortIdx = RTK_FC_MAC_EXT_PORT3;
		}
		tmp_pktHdr.cvlanid = 0; //FIXME
		tmp_pktHdr.egrWlanDevIdx = RTK_FC_WLANX_NOT_FOUND;

		if(_rtk_fc_lut_find(tmp_pktHdr.eth->h_dest, &lutIdx) == RTK_FC_RET_OK)
		{
			DEBUG("[VXLAN]Extra upstream fake LUT Learned before, no need to learn again.");
			ret = RTK_FC_RET_OK;
		}
		else if((ret = _rtk_fc_lut_learning(tmp_pktHdr.eth->h_dest, tmp_pktHdr.egressPort, tmp_pktHdr.cvlanid, RTK_FC_WLANX_NOT_FOUND,
									FALSE, TRUE, &lutIdx, NULL, LUT_DIR_DONT_CARE)) != RTK_FC_RET_OK) 
		{
			TRACE("LUT DA learning is fail, ret = 0x%x", ret);
			//return ret;
		}

		fc_db.vxlan_accelerate_extra_upstreamL2Idx = lutIdx;

		if(fc_db.vxlan_accelerate_upstreamL2Idx!= -1 && fc_db.vxlan_accelerate_extra_upstreamL2Idx != -1 )
		{
			fc_db.vxlan_upstream_set_ok = 1;
		}
	}
	else if(direction ==RTK_FC_L2DUAL_ACT_REMOVE)
	{
		
		//Should be outer DA
		tmp_pktHdr.eth = &tmp_eth;
		memcpy(&tmp_pktHdr.eth->h_dest[0],&pG3IgrExtraInfo->vxlan_info.vxlan_DA[0],6);
		
		//00:00:5e:ff:ff:ff
		tmp_pktHdr.eth->h_dest[0] = 0x0;
		tmp_pktHdr.eth->h_dest[1] = 0x0;
		tmp_pktHdr.eth->h_dest[2] = 0x5e;
		tmp_pktHdr.eth->h_dest[3] = 0x0;
		if(fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamGmac==1)
		{
			tmp_pktHdr.egressPort.macPortIdx = RTK_FC_MAC_PORT_MASTERCPU_CORE1;
			tmp_pktHdr.egressPort.macExtPortIdx = RTK_FC_MAC_EXT_PORT4;
		}
		else if(fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamGmac==2)
		{
			tmp_pktHdr.egressPort.macPortIdx = RTK_FC_MAC_PORT_SLAVECPU;
			tmp_pktHdr.egressPort.macExtPortIdx = RTK_FC_MAC_EXT_PORT4;
		}
		else
		{
			tmp_pktHdr.egressPort.macPortIdx = RTK_FC_MAC_PORT_MASTERCPU_CORE0;
			tmp_pktHdr.egressPort.macExtPortIdx = RTK_FC_MAC_EXT_PORT4;
		}
		
		tmp_pktHdr.cvlanid = 0; //FIXME
		tmp_pktHdr.egrWlanDevIdx = RTK_FC_WLANX_NOT_FOUND;

		if(_rtk_fc_lut_find(tmp_pktHdr.eth->h_dest, &lutIdx) == RTK_FC_RET_OK)
		{
			DEBUG("[VXLAN]Downstream fake LUT Learned before, no need to learn again.");
			ret = RTK_FC_RET_OK;
		}
		else if((ret = _rtk_fc_lut_learning(tmp_pktHdr.eth->h_dest, tmp_pktHdr.egressPort, tmp_pktHdr.cvlanid, RTK_FC_WLANX_NOT_FOUND,
									FALSE, TRUE, &lutIdx,NULL, LUT_DIR_DONT_CARE)) != RTK_FC_RET_OK) {
			TRACE("LUT DA learning is fail, ret = 0x%x", ret);
			//return ret;
		}
		fc_db.vxlan_accelerate_downstreamL2Idx = lutIdx;
		
		tmp_pktHdr.eth->h_dest[3] = 0x01;

		if(fc_db_fastFwd_data.vxlan_acc.vxlan_extraGmac==1)
		{
			tmp_pktHdr.egressPort.macPortIdx = RTK_FC_MAC_PORT_MASTERCPU_CORE1;
			tmp_pktHdr.egressPort.macExtPortIdx = RTK_FC_MAC_EXT_PORT4;
		}
		else if(fc_db_fastFwd_data.vxlan_acc.vxlan_extraGmac==2)
		{
			tmp_pktHdr.egressPort.macPortIdx = RTK_FC_MAC_PORT_SLAVECPU;
			tmp_pktHdr.egressPort.macExtPortIdx = RTK_FC_MAC_EXT_PORT4;
		}
		else
		{
			tmp_pktHdr.egressPort.macPortIdx = RTK_FC_MAC_PORT_MASTERCPU_CORE0;
			tmp_pktHdr.egressPort.macExtPortIdx = RTK_FC_MAC_EXT_PORT4;
		}
		tmp_pktHdr.cvlanid = 0; //FIXME
		tmp_pktHdr.egrWlanDevIdx = RTK_FC_WLANX_NOT_FOUND;

		if(_rtk_fc_lut_find(tmp_pktHdr.eth->h_dest, &lutIdx) == RTK_FC_RET_OK)
		{
			DEBUG("[VXLAN]Extra downstream fake LUT Learned before, no need to learn again.");
			ret = RTK_FC_RET_OK;
		}
		else if((ret = _rtk_fc_lut_learning(tmp_pktHdr.eth->h_dest, tmp_pktHdr.egressPort, tmp_pktHdr.cvlanid, RTK_FC_WLANX_NOT_FOUND,
									FALSE, TRUE, &lutIdx,NULL, LUT_DIR_DONT_CARE)) != RTK_FC_RET_OK) {
			TRACE("LUT DA learning is fail, ret = 0x%x", ret);
			//return ret;
		}
		fc_db.vxlan_accelerate_extra_downstreamL2Idx = lutIdx;

		if(fc_db.vxlan_accelerate_downstreamL2Idx!= -1 && fc_db.vxlan_accelerate_extra_downstreamL2Idx != -1 )
		{
			fc_db.vxlan_downstream_set_ok = 1;
		}
	}
	
}

int _rtk_fc_vxlan_acceleration_extraPmsk_set(unsigned int pmsk)
{
	if(fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_mechanism){
		unsigned int orig_extraPmsk=fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraPmsk;
		fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraPmsk = pmsk;
		//pmsk changed, reset all
		if(orig_extraPmsk!=fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraPmsk){
			int tmp;
			struct rtl8686_hwnat_customized_entry customized_entry={0};
			//clear all flow
			tmp=fc_db.systemGlobal.flow_not_update_in_real_time;
			fc_db.systemGlobal.flow_not_update_in_real_time=0;
			rtk_fc_flow_flush();
			fc_db.systemGlobal.flow_not_update_in_real_time=tmp;

			//reset nic rx and tx ring
			fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamInfo_prepared=0;
			if(fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamRing_modified){
				customized_entry.type=CUSTOMIZE_TYPE_VXLAN_UP;
				customized_entry.gmac=fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamGmac;
				customized_entry.rxRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamRxRingNum;
				customized_entry.txRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamTxRingNum;
				re8686_customized_rx_and_tx(customized_entry, NULL, NULL, NULL, NULL);			
				fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamRing_modified=0;
			}
			fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamInfo_prepared=0;
			if(fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamRing_modified){
				customized_entry.type=CUSTOMIZE_TYPE_VXLAN_UP;
				customized_entry.gmac=fc_db_fastFwd_data.vxlan_acc.vxlan_extraGmac;
				customized_entry.rxRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamRxRingNum;
				customized_entry.txRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamTxRingNum;
				re8686_customized_rx_and_tx(customized_entry, NULL, NULL, NULL, NULL);			
				fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamRing_modified=0;
			}
			fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamInfo_prepared=0;
			if(fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamRing_modified){
				struct rtl8686_hwnat_customized_entry customized_entry={0};
				customized_entry.type=CUSTOMIZE_TYPE_VXLAN_DOWN;
				customized_entry.gmac=fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamGmac;
				customized_entry.rxRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamRxRingNum;
				customized_entry.txRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamTxRingNum;
				re8686_customized_rx_and_tx(customized_entry, NULL, NULL, NULL, NULL);
				fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamRing_modified=0;
			}
			fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamInfo_prepared=0;
			if(fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamRing_modified){
				customized_entry.type=CUSTOMIZE_TYPE_VXLAN_DOWN;
				customized_entry.gmac=fc_db_fastFwd_data.vxlan_acc.vxlan_extraGmac;
				customized_entry.rxRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamRxRingNum;
				customized_entry.txRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamTxRingNum;
				re8686_customized_rx_and_tx(customized_entry, NULL, NULL, NULL, NULL);
				fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamRing_modified=0;
			}
		}
		
	}
	return SUCCESS;
}
int _rtk_fc_vxlan_acceleration_mechanism_set(int value)
{
	rtk_qos_pri2queue_t pri2queue;
	unsigned int orig_mechanism;
	int i;

	orig_mechanism=fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_mechanism;
	fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_mechanism = (value!=0)?1:0;
	assert_ok(rtk_fc_flow_flush_all());
	
	fc_db.vxlan_ds_fastForward_num = 0;
	fc_db.vxlan_us_fastForward_num = 0;
	fc_db.vxlan_fastFwd_data_general_setting_isSet = 0;
	for(i =0; i < 4 ; i++)
	{
		fc_db.vxlan_upStream_record[i].cpuPort = 0;
		fc_db.vxlan_upStream_record[i].inner_flow_index =0;
		fc_db.vxlan_upStream_record[i].outer_flow_index =0;
		fc_db.vxlan_upStream_record[i].isSet =0;
		fc_db.vxlan_downStream_record[i].cpuPort =0;
		fc_db.vxlan_downStream_record[i].inner_flow_index =0;
		fc_db.vxlan_downStream_record[i].outer_flow_index =0;
		fc_db.vxlan_downStream_record[i].isSet = 0;
	}

#if 1
	if(fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_mechanism==1)
	{
		struct rtl8686_hwnat_customized_entry customized_entry={0};
		
		TRACE("Set vxlan_acceleration_mechanism\n");

		rtk_fc_flow_flush_all();

		//clean up rx and tx ring
		if(fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamRing_modified){
			customized_entry.type		= CUSTOMIZE_TYPE_VXLAN_UP;
			customized_entry.gmac		= 1;
			customized_entry.rxRingNum	= 4;
			customized_entry.txRingNum	= 3;
			re8686_customized_rx_and_tx(customized_entry, NULL, NULL, NULL, NULL);			
		}
		if(fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamRing_modified){
			customized_entry.type		= CUSTOMIZE_TYPE_VXLAN_UP;
			customized_entry.gmac		= 0;
			customized_entry.rxRingNum 	= 4;
			customized_entry.txRingNum	= 3;
			re8686_customized_rx_and_tx(customized_entry, NULL, NULL, NULL, NULL);			
		}
		if(fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamRing_modified){
			customized_entry.type		= CUSTOMIZE_TYPE_VXLAN_DOWN;
			customized_entry.gmac		= 2;
			customized_entry.rxRingNum	= 4;
			customized_entry.txRingNum	= 3;
			re8686_customized_rx_and_tx(customized_entry, NULL, NULL, NULL, NULL);
		}
		if(fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamRing_modified){
			customized_entry.type		= CUSTOMIZE_TYPE_VXLAN_DOWN;
			customized_entry.gmac		= 0;
			customized_entry.rxRingNum	= 3;
			customized_entry.txRingNum	= 2;
			re8686_customized_rx_and_tx(customized_entry, NULL, NULL, NULL, NULL);
		}
		//clear all
		bzero(&fc_db_fastFwd_data.vxlan_acc,sizeof(rtk_fc_vxlan_acceleration_data_t));

		
		//setup priority to queue since extra gmac will use priority to separate upstream and downstream.
		pri2queue.pri2queue[0] = 0;
		pri2queue.pri2queue[1] = 1;
		pri2queue.pri2queue[2] = 2;
		pri2queue.pri2queue[3] = 3;
		pri2queue.pri2queue[4] = 4;
		pri2queue.pri2queue[5] = 6;
		pri2queue.pri2queue[6] = 6;
		pri2queue.pri2queue[7] = 7;
		assert_ok(rtk_qos_priMap_set(1, &pri2queue));
		
		assert_ok(rtk_qos_portPriMap_set(RTK_FC_MAC_PORT_MASTERCPU_CORE0, 1));
		assert_ok(rtk_qos_portPriMap_set(RTK_FC_MAC_PORT_MASTERCPU_CORE1, 1));
		assert_ok(rtk_qos_portPriMap_set(RTK_FC_MAC_PORT_SLAVECPU, 1));
		
#ifdef CONFIG_SMP
#if CONFIG_NR_CPUS == 4
		cpumask_gmac0.bits[0]=0x1;	//CPU 0
		RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC0_IRQ, &cpumask_gmac0); //GMAC9

		cpumask_gmac1.bits[0]=0x8;	//CPU 3
		RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC1_IRQ, &cpumask_gmac1); //GMAC10
		
//#if defined(CONFIG_GMAC2_USABLE)
		if(fc_db.controlFuc.gmac2_usable==1)
		{
			cpumask_gmac2.bits[0]=0x2;	//CPU 1
			RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC2_IRQ, &cpumask_gmac2); //GMAC7
		}
//#endif
#elif CONFIG_NR_CPUS == 2
		cpumask_gmac0.bits[0]=0x1;	//CPU 0
		RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC0_IRQ, &cpumask_gmac0); //GMAC9

		cpumask_gmac1.bits[0]=0x2;	//CPU 1
		RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC1_IRQ, &cpumask_gmac1); //GMAC10
		
//#if defined(CONFIG_GMAC2_USABLE)
		if(fc_db.controlFuc.gmac2_usable==1)
		{
			cpumask_gmac2.bits[0]=0x1;	//CPU 0
			RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC2_IRQ, &cpumask_gmac2); //GMAC7
		}
//#endif
#endif
#endif
		fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_mechanism=1;

		fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamGmac=1;
		fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamRxRingNum=4;
		fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamTxRingNum=3;
		fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamPriority=fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamRxRingNum+2;

		fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamGmac=2;
		fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamRxRingNum=4;
		fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamTxRingNum=3;
		fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamPriority=fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamRxRingNum+2;

		fc_db_fastFwd_data.vxlan_acc.vxlan_extraGmac=0;
		fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamRxRingNum=4;
		fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamTxRingNum=3;
		fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamPriority=fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamRxRingNum+2;
		fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamRxRingNum=3;
		fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamTxRingNum=2;
		fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamPriority=fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamRxRingNum+2;
		
		if(dynamic_sram_desc==2 && fc_db.vxlan_rt_meter_index == -1){
			
			//fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraMeter=RTK_FC_VXLAN_ACC_EXTRAPMSK_METER;
			//assert_ok((pf.rtk_fc_shareMeter_set)(fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraMeter, RTK_FC_VXLAN_ACC_EXTRAPMSK_RATE, RTK_RG_ENABLED));		
			//assert_ok(_rtk_fc_l3Meter_set(RT_RATE_EXT_METER_TYPE_FLOW, fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraMeter, RTK_FC_VXLAN_ACC_EXTRAPMSK_RATE, RTK_FC_ENABLED));
			uint32 index = 0;
			fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraPmsk=RTK_FC_VXLAN_ACC_EXTRAPMSK;
			
			RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();
			// rt_rate add to get index
			
			rt_rate_shareMeterType_add(RT_METER_TYPE_FLOW, &index);
			//WARNING("[VXLAN]ADD RT_METER_TYPE_FLOW, index = %d",index);
			fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraMeter = index;
			fc_db.vxlan_rt_meter_index = index;
			if(ASICDRIVERVER == 0x3)
				rt_rate_shareMeterRate_set(fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraMeter, RTK_FC_VXLAN_ACC_EXTRAPMSK_RATE);
			else
				rt_rate_shareMeterRate_set(fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraMeter, RTK_FC_VXLAN_ACC_EXTRAPMSK_RATE_FOR_VA);
			rt_rate_shareMeterBucket_set(fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraMeter, 3000);
			
			RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();
		}
		else if(dynamic_sram_desc==2 && fc_db.vxlan_rt_meter_index != -1) //Use meter index which is recorded in fc_db. Don't get shareMeter index multiple times.
		{
			fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraPmsk	= RTK_FC_VXLAN_ACC_EXTRAPMSK;
			fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraMeter 	= fc_db.vxlan_rt_meter_index;

		}
		
	}
#else
	if( (orig_mechanism==0 && fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_mechanism==1) )
	{
		TRACE("Set vxlan_acceleration_mechanism\n");
		//setup priority to queue since extra gmac will use priority to separate upstream and downstream.
		pri2queue.pri2queue[0] = 0;
		pri2queue.pri2queue[1] = 1;
		pri2queue.pri2queue[2] = 2;
		pri2queue.pri2queue[3] = 3;
		pri2queue.pri2queue[4] = 4;
		pri2queue.pri2queue[5] = 6;
		pri2queue.pri2queue[6] = 6;
		pri2queue.pri2queue[7] = 7;
		assert_ok(rtk_qos_priMap_set(1, &pri2queue));
		
		assert_ok(rtk_qos_portPriMap_set(RTK_FC_MAC_PORT_MASTERCPU_CORE0, 1));
		assert_ok(rtk_qos_portPriMap_set(RTK_FC_MAC_PORT_MASTERCPU_CORE1, 1));
		assert_ok(rtk_qos_portPriMap_set(RTK_FC_MAC_PORT_SLAVECPU, 1));
		
#ifdef CONFIG_SMP
#if CONFIG_NR_CPUS == 4
			cpumask_gmac0.bits[0]=0x1;  //CPU 0
			RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC0_IRQ, &cpumask_gmac0); //GMAC9
	
			cpumask_gmac1.bits[0]=0x8;  //CPU 3
			RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC1_IRQ, &cpumask_gmac1); //GMAC10
			
#if defined(CONFIG_GMAC2_USABLE)
			cpumask_gmac2.bits[0]=0x2;  //CPU 1
			RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC2_IRQ, &cpumask_gmac2); //GMAC7
#endif
#elif CONFIG_NR_CPUS == 2
			cpumask_gmac0.bits[0]=0x1;  //CPU 0
			RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC0_IRQ, &cpumask_gmac0); //GMAC9
	
			cpumask_gmac1.bits[0]=0x2;  //CPU 1
			RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC1_IRQ, &cpumask_gmac1); //GMAC10
			
#if defined(CONFIG_GMAC2_USABLE)
			cpumask_gmac2.bits[0]=0x1;  //CPU 0
			RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC2_IRQ, &cpumask_gmac2); //GMAC7
#endif
#endif
#endif

		fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamGmac=1;
		fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamRxRingNum=4;
		fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamTxRingNum=3;
		fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamPriority=fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamRxRingNum+2;

		fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamGmac=2;
		fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamRxRingNum=4;
		fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamTxRingNum=3;
		fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamPriority=fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamRxRingNum+2;

		fc_db_fastFwd_data.vxlan_acc.vxlan_extraGmac=0;
		fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamRxRingNum=4;
		fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamTxRingNum=3;
		fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamPriority=fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamRxRingNum+2;
		fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamRxRingNum=3;
		fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamTxRingNum=2;
		fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamPriority=fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamRxRingNum+2;

		if(dynamic_sram_desc==2){
			
			//fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraMeter=RTK_FC_VXLAN_ACC_EXTRAPMSK_METER;
			//assert_ok((pf.rtk_fc_shareMeter_set)(fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraMeter, RTK_FC_VXLAN_ACC_EXTRAPMSK_RATE, RTK_RG_ENABLED));		
			//assert_ok(_rtk_fc_l3Meter_set(RT_RATE_EXT_METER_TYPE_FLOW, fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraMeter, RTK_FC_VXLAN_ACC_EXTRAPMSK_RATE, RTK_FC_ENABLED));
			uint32 index = 0;
			fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraPmsk=RTK_FC_VXLAN_ACC_EXTRAPMSK;
			
			RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();
			// rt_rate add to get index
			
			rt_rate_shareMeterType_add(RT_METER_TYPE_FLOW, &index);
			//WARNING("[VXLAN]ADD RT_METER_TYPE_FLOW, index = %d",index);
			fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraMeter = index;
			rt_rate_shareMeterRate_set(fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraMeter, RTK_FC_VXLAN_ACC_EXTRAPMSK_RATE);
			RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();
		}
		
	}
#endif
	else if(fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_mechanism==0)
	{
		int tmp;
		struct rtl8686_hwnat_customized_entry customized_entry={0};

		
		TRACE("Reset vxlan_acceleration_mechanism\n");
		//clear all flow
		tmp=fc_db.systemGlobal.flow_not_update_in_real_time;
		fc_db.systemGlobal.flow_not_update_in_real_time=0;
		rtk_fc_flow_flush();
		fc_db.systemGlobal.flow_not_update_in_real_time=tmp;
			
		//clean up rx and tx ring
		if(fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamRing_modified){
			customized_entry.type=CUSTOMIZE_TYPE_VXLAN_UP;
			customized_entry.gmac=fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamGmac;
			customized_entry.rxRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamRxRingNum;
			customized_entry.txRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamTxRingNum;
			re8686_customized_rx_and_tx(customized_entry, NULL, NULL, NULL, NULL);			
		}
		if(fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamRing_modified){
			customized_entry.type=CUSTOMIZE_TYPE_VXLAN_UP;
			customized_entry.gmac=fc_db_fastFwd_data.vxlan_acc.vxlan_extraGmac;
			customized_entry.rxRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamRxRingNum;
			customized_entry.txRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamTxRingNum;
			re8686_customized_rx_and_tx(customized_entry, NULL, NULL, NULL, NULL);			
		}
		if(fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamRing_modified){
			customized_entry.type=CUSTOMIZE_TYPE_VXLAN_DOWN;
			customized_entry.gmac=fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamGmac;
			customized_entry.rxRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamRxRingNum;
			customized_entry.txRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamTxRingNum;
			re8686_customized_rx_and_tx(customized_entry, NULL, NULL, NULL, NULL);
		}
		if(fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamRing_modified){
			customized_entry.type=CUSTOMIZE_TYPE_VXLAN_DOWN;
			customized_entry.gmac=fc_db_fastFwd_data.vxlan_acc.vxlan_extraGmac;
			customized_entry.rxRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamRxRingNum;
			customized_entry.txRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamTxRingNum;
			re8686_customized_rx_and_tx(customized_entry, NULL, NULL, NULL, NULL);
		}
		//clear all
		bzero(&fc_db_fastFwd_data.vxlan_acc,sizeof(rtk_fc_vxlan_acceleration_data_t));

		//reset priority to queue
		pri2queue.pri2queue[0] = 0;
		pri2queue.pri2queue[1] = 0;
		pri2queue.pri2queue[2] = 0;
		pri2queue.pri2queue[3] = 0;
		pri2queue.pri2queue[4] = 0;
		pri2queue.pri2queue[5] = 0;
		pri2queue.pri2queue[6] = 0;
		pri2queue.pri2queue[7] = 0;
		assert_ok(rtk_qos_priMap_set(1, &pri2queue));
		{
			assert_ok(rtk_qos_portPriMap_set(RTK_FC_MAC_PORT_MASTERCPU_CORE0, 3));
			assert_ok(rtk_qos_portPriMap_set(RTK_FC_MAC_PORT_MASTERCPU_CORE1, 3));
			assert_ok(rtk_qos_portPriMap_set(RTK_FC_MAC_PORT_SLAVECPU, 3));
		}
#ifdef CONFIG_SMP
		cpumask_gmac0.bits[0]=0x2;  //CPU 1
		RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC0_IRQ, &cpumask_gmac0); //GMAC9

		cpumask_gmac1.bits[0]=0x2;  //CPU 1
		RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC1_IRQ, &cpumask_gmac1); //GMAC10
		
//#if defined(CONFIG_GMAC2_USABLE)
		if(fc_db.controlFuc.gmac2_usable==1)
		{
			cpumask_gmac2.bits[0]=0x2;  //CPU all
			RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC2_IRQ, &cpumask_gmac2); //GMAC7
		}
//#endif
#endif


	}	

	if(dynamic_sram_desc==2 && fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraMeter == -1)
	{
		uint32 index = 0;
		fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraPmsk=RTK_FC_VXLAN_ACC_EXTRAPMSK;
		
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_UNLOCK_BH();
		// rt_rate add to get index
		
		rt_rate_shareMeterType_add(RT_METER_TYPE_FLOW, &index);
		//WARNING("[VXLAN]ADD RT_METER_TYPE_FLOW, index = %d",index);
		fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraMeter = index;
		rt_rate_shareMeterRate_set(fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraMeter, RTK_FC_VXLAN_ACC_EXTRAPMSK_RATE);
		RTK_FC_HELPER_MGR_GLOBAL_SPIN_LOCK_BH();
	}

	if(fc_db.vxlan_accelerate_upstreamL2Idx!=-1)
	{
		_rtk_fc_lut_staticEntry_del(fc_db.vxlan_accelerate_upstreamL2Idx, FALSE);
		fc_db.vxlan_accelerate_upstreamL2Idx = -1;
	}
	if(fc_db.vxlan_accelerate_extra_upstreamL2Idx!=-1)
	{
		_rtk_fc_lut_staticEntry_del(fc_db.vxlan_accelerate_extra_upstreamL2Idx, FALSE);
		fc_db.vxlan_accelerate_extra_upstreamL2Idx = -1;
	}
	
	fc_db.vxlan_upstream_set_ok = 0;
	
	if(fc_db.vxlan_accelerate_downstreamL2Idx!=-1)
	{
		_rtk_fc_lut_staticEntry_del(fc_db.vxlan_accelerate_downstreamL2Idx, FALSE);
		fc_db.vxlan_accelerate_downstreamL2Idx = -1;
	}
	if(fc_db.vxlan_accelerate_extra_downstreamL2Idx!=-1)
	{
		_rtk_fc_lut_staticEntry_del(fc_db.vxlan_accelerate_extra_downstreamL2Idx, FALSE);
		fc_db.vxlan_accelerate_extra_downstreamL2Idx = -1;
	}
	fc_db.vxlan_downstream_set_ok = 0;

	// reset info prepared
	fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamInfo_prepared = 0;
	fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamInfo_prepared = 0;
	fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamInfo_prepared = 0;
	fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamInfo_prepared = 0;


	printk("Ready to set CPU affinity\n");

	if(dynamic_sram_desc==2)
	{
#ifdef CONFIG_SMP
#if CONFIG_NR_CPUS == 4
		cpumask_gmac0.bits[0]=0x1;  //CPU 0
		RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC0_IRQ, &cpumask_gmac0); //GMAC9
	
		cpumask_gmac1.bits[0]=0x8;  //CPU 3
		RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC1_IRQ, &cpumask_gmac1); //GMAC10
		
//#if defined(CONFIG_GMAC2_USABLE)
		if(fc_db.controlFuc.gmac2_usable==1)
		{
			cpumask_gmac2.bits[0]=0x2;  //CPU 1
			RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC2_IRQ, &cpumask_gmac2); //GMAC7
		}
//#endif
#elif CONFIG_NR_CPUS == 2
		cpumask_gmac0.bits[0]=0x1;  //CPU 0
		RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC0_IRQ, &cpumask_gmac0); //GMAC9
	
		cpumask_gmac1.bits[0]=0x2;  //CPU 1
		RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC1_IRQ, &cpumask_gmac1); //GMAC10
		
//#if defined(CONFIG_GMAC2_USABLE)
		if(fc_db.controlFuc.gmac2_usable==1)
		{
			cpumask_gmac2.bits[0]=0x1;  //CPU 0
			RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC2_IRQ, &cpumask_gmac2); //GMAC7
		}
//#endif
#endif
#endif

	}
	else
	{
	
#ifdef CONFIG_SMP
		cpumask_gmac0.bits[0]=0x2;  //CPU 1
		RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC0_IRQ, &cpumask_gmac0); //GMAC9

		cpumask_gmac1.bits[0]=0x2;  //CPU 1
		RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC1_IRQ, &cpumask_gmac1); //GMAC10
		
//#if defined(CONFIG_GMAC2_USABLE)
		if(fc_db.controlFuc.gmac2_usable==1)
		{
			cpumask_gmac2.bits[0]=0x2;  //CPU all
			RTK_FC_HELPER_IRQ_SET_AFFINITY_HINT(BSP_INT0_GMAC2_IRQ, &cpumask_gmac2); //GMAC7
		}
//#endif
#endif

	}
	return 0;
}

int rtk_fc_vxlan_customized_upstreamTxHook(struct re_private *cp, struct tx_info *pTxInfo, int len)
{	
	*(unsigned short *)(pTxInfo->addr+fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_udp_length_offset)=htons(len+8+8);
	*(unsigned short *)(pTxInfo->addr+fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_ip_length_offset)=htons(len+8+8+20);

	return len-4;
}


int rtk_fc_vxlan_customized_rxHook(struct re_private *cp, struct rx_info *pRxInfo, int len)
{
	if(pRxInfo->opts3.bit.fb_hash_or_dst_portmsk&RTK_FC_VXLAN_ACC_UPSTREAM_EXTPMAK){
		//upstream
		*(unsigned short *)pRxInfo->addr=htons(*(unsigned short *)(fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer+fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length-RX_OFFSET));
		*(unsigned int *)(pRxInfo->addr+RX_OFFSET)=*(unsigned int *)(fc_db_fastFwd_data.vxlan_acc.vxlan_upstream_dmac);		//recovery dmac to vxlan remote dmac
		//rtk_fc_vxlan_hexdump(fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer, fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length);

		*(unsigned short *)(pRxInfo->addr-fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length+RX_OFFSET+fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_udp_length_offset)=htons(len+8+8);////*(unsigned short *)(skb->data+vxlan_udpHdr_len_offset)=htons(len+8);			//payload + udp hdr length
		*(unsigned short *)(pRxInfo->addr-fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length+RX_OFFSET+fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_ip_length_offset)=htons(len+8+8+20);////*(unsigned short *)(skb->data+vxlan_ipHdr_len_offset)=htons(len+8+20);			//payload + udp hdr + ip hdr length
		return len+fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length;
	}else{
		//downstream
		return len-fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length;
	}
}

int rtk_fc_vxlan_customized_txPrepare(struct re_private *cp, struct tx_info *pTxInfo)
{
	unsigned int offset = fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length;
	unsigned char *pData=(unsigned char *)pTxInfo->addr;
	memcpy(pData, fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer, offset);
	*(unsigned int *)(pData+offset)=*(unsigned int *)(fc_db_fastFwd_data.vxlan_acc.vxlan_upstream_dmac);

	return 0;
}

int rtk_fc_vxlan_customized_extraTxPrepare(struct re_private *cp, struct tx_info *pTxInfo)
{
	unsigned int offset = fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length;
	unsigned char *pData=(unsigned char *)pTxInfo->addr;
	memcpy(pData, fc_db_fastFwd_data.vxlan_acc.vxlan_extra_preallocated_outer, offset);
	*(unsigned int *)(pData+offset)=*(unsigned int *)(fc_db_fastFwd_data.vxlan_acc.vxlan_upstream_dmac);

	return 0;
}

int rtk_fc_vxlan_customized_rxPrepare(struct re_private *cp, struct rx_info *pRxInfo)
{
	unsigned int offset;
	unsigned char *pData=(unsigned char *)pRxInfo->addr;

	
	offset = fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length;
	
	if(offset & 0xf){
		int padding=16-((offset-RX_OFFSET)-((offset>>4)<<4));

		pRxInfo->addr+=padding;
		pData=(unsigned char *)pRxInfo->addr;
	}

	memcpy(pData, fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer, offset);
	
	pRxInfo->addr+=offset;
	pRxInfo->addr-=RX_OFFSET;

	return 0;
}

#endif
#if 0
__IRAM_FC_SHORTCUT
int rtk_fc_vxlan_inner_skbVlanTag_insert(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint16 vlanTCI, uint8 isStag, uint32 l2DualTbl_index)
{
	uint8 *pData = RTSKB_DATA(rtskb);
	int insertOff = 0, outer_tag_len = 0;

	FC_PARAM_CHK(pPktHdr->eth==NULL, RTK_FC_RET_ERR_INVALID_PARAM);
	
	if(fc_db.l2DualTbl[l2DualTbl_index].action == RTK_FC_L2DUAL_ACT_ADD)
		outer_tag_len = fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_tag_len;
	else
		outer_tag_len = fc_db.l2DualTbl[l2DualTbl_index].vxlan_ds.outer_tag_len;

	insertOff += outer_tag_len + (ETH_HLEN - 2);
	
	TRACE("[VXLAN][SHORTCUT]insert inner %sTag TCI=%x",isStag?"S":"C",vlanTCI);

	if(isStag){
		FC_PARAM_CHK(pPktHdr->svh!=NULL, RTK_FC_RET_ERR_INVALID_PARAM);
		// insert stag after ethhdr

	}else{
		FC_PARAM_CHK(pPktHdr->cvh!=NULL, RTK_FC_RET_ERR_INVALID_PARAM);
		// insert ctag after stag or ethhdr
		if(pPktHdr->svh) insertOff += VLAN_HLEN;
	}

	if(_rtk_fc_skb_cow_head_and_pktHdr_update(RTSKB_SKB(rtskb), VLAN_HLEN, rtskb, pPktHdr)<0)
	{
		WARNING("skb head room is not enough");
		return FAIL;
	}

	RTK_FC_HOOK_PS_SKB_SKB_PUSH(RTSKB_SKB(rtskb), VLAN_HLEN, &pData);
	memmove(pData, pData + VLAN_HLEN, insertOff);

	RTSKB_MAC_HEADER(rtskb) -= (VLAN_HLEN);
	// reset header pointer
	
	pPktHdr->outer_eth = (struct ethhdr *)pData;

	if(fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_ppph_tag_off!=0)
	{
		pPktHdr->ppph 		= (struct pppoe_hdr *)(pData + fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_ppph_tag_off);
		pPktHdr->ppph->length = htons(ntohs(pPktHdr->ppph->length) + VLAN_HLEN); 
	}
	

	
	if(fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_is_ipv6)
	{
		pPktHdr->outer_ip6h = (struct ipv6hdr *)(pData + fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_iph_tag_off);
		pPktHdr->outer_ip6h->payload_len = htons(ntohs(pPktHdr->outer_ip6h->payload_len)+4) ;
	
	}
	else
	{
		TRACE("[VXLAN][SHORTCUT]Original ip len: %d will be change to %d", ntohs(*( (u16*)(pData + fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_ip_len_off))), ntohs(*( (u16*)(pData + fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_ip_len_off)))+4);
		pPktHdr->outer_iph = (struct iphdr * )(pData + fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_iph_tag_off);
		*( (u16*)(pData + fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_ip_len_off)) = htons(ntohs(*( (u16*)(pData + fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_ip_len_off)))+VLAN_HLEN);
	}

	
	TRACE("[VXLAN][SHORTCUT]Original udp len: %d will be change to %d", ntohs(*( (u16*)(pData + fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_udp_len_off))), ntohs(*( (u16*)(pData + fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_udp_len_off)))+4);
		
	pPktHdr->outer_udph = (struct udphdr *)(pData + fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_udp_len_off);
	*( (u16*)(pData + fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_udp_len_off)) = htons(ntohs(*( (u16*)(pData + fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_udp_len_off)))+VLAN_HLEN);

	
	pPktHdr->eth = (struct ethhdr *)(pData+outer_tag_len);
	if(isStag){
		pPktHdr->eth->h_proto = htons(ETH_P_8021AD);

		pPktHdr->svh = (struct vlan_hdr *)(pData+insertOff+2);
		pPktHdr->svh->h_vlan_TCI = htons(vlanTCI);
		pPktHdr->svlanid = vlanTCI & VLAN_VID_MASK;
		pPktHdr->svlanpri = (vlanTCI & VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
	}
	else{
		if(pPktHdr->svh){
			pPktHdr->svh = (struct vlan_hdr *)((uint8 *)(pPktHdr->svh) - VLAN_HLEN);
			pPktHdr->svh->h_vlan_encapsulated_proto = htons(ETH_P_8021Q);
		}else
			pPktHdr->eth->h_proto = htons(ETH_P_8021Q);

		pPktHdr->cvh = (struct vlan_hdr *)(pData+insertOff+2);
		pPktHdr->cvh->h_vlan_TCI = htons(vlanTCI);
		pPktHdr->cvlanid = vlanTCI & VLAN_VID_MASK;
		pPktHdr->cvlanpri = (vlanTCI & VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
	}

	return (SUCCESS);
}

__IRAM_FC_SHORTCUT
int rtk_fc_vxlan_inner_skbVlanTag_remove(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint8 isStag, uint32 l2DualTbl_index)
{
	int rmOff = 0, outer_tag_len = 0;
	uint8 *pData = RTSKB_DATA(rtskb);

	FC_PARAM_CHK(pPktHdr->eth==NULL, RTK_FC_RET_ERR_INVALID_PARAM);

	TRACE("[VXLAN][ShortCut]remove %sTag ",isStag?"S":"C");
	if(fc_db.l2DualTbl[l2DualTbl_index].action == RTK_FC_L2DUAL_ACT_ADD)
		outer_tag_len = fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_tag_len;
	else
		outer_tag_len = fc_db.l2DualTbl[l2DualTbl_index].vxlan_ds.outer_tag_len;

	rmOff += outer_tag_len+(ETH_HLEN - 2);
	TRACE("[VXLAN][ShortCut]rmOff = %d",rmOff);
	if(isStag){
		FC_PARAM_CHK(pPktHdr->svh==NULL, RTK_FC_RET_ERR_INVALID_PARAM);
		// insert stag after ethhdr
	}else{
		FC_PARAM_CHK(pPktHdr->cvh==NULL, RTK_FC_RET_ERR_INVALID_PARAM);
		// insert ctag after stag or ethhdr
		if(pPktHdr->svh) rmOff += VLAN_HLEN;

	}

	memmove(pData+VLAN_HLEN, pData, rmOff);		// remove vlan header

	RTSKB_DATA(rtskb)+=VLAN_HLEN;
	RTSKB_LEN(rtskb)-=VLAN_HLEN;
	RTK_FC_HOOK_PS_SKB_SKB_RESET_MAC_HEADER(RTSKB_SKB(rtskb));

	if(fc_db.l2DualTbl[l2DualTbl_index].action == RTK_FC_L2DUAL_ACT_ADD)
	{
		
		TRACE("l2DualTbl_index = %d, outer_iph_tag_off = %d, outer_udp_len_off = %d ",l2DualTbl_index, fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_iph_tag_off, fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_udph_tag_off);
		pPktHdr->eth = (struct ethhdr *)(RTSKB_DATA(rtskb)+outer_tag_len);
		
		if(fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_ppph_tag_off!=0)
		{
			pPktHdr->ppph		= (struct pppoe_hdr *)(RTSKB_DATA(rtskb) + fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_ppph_tag_off);
			pPktHdr->ppph->length = htons(ntohs(pPktHdr->ppph->length) - VLAN_HLEN); 
		}
		
		if(!(fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_is_ipv6) && pPktHdr->outer_iph)
		{
			TRACE("[VXLAN][SHORTCUT]Original outer ip len: %d will be change to %d", ntohs(pPktHdr->outer_iph->tot_len), ntohs(pPktHdr->outer_iph->tot_len)-4);
			pPktHdr->outer_iph = (struct iphdr * )(RTSKB_DATA(rtskb)+ fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_iph_tag_off);
			pPktHdr->outer_iph->tot_len = htons(ntohs(pPktHdr->outer_iph->tot_len)-4) ;

		}
		else if(fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_is_ipv6)
		{
			pPktHdr->outer_ip6h = (struct ipv6hdr *)(RTSKB_DATA(rtskb)+ fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_iph_tag_off);
			pPktHdr->outer_ip6h->payload_len = htons(ntohs(pPktHdr->outer_ip6h->payload_len)-4) ;

		}

		TRACE("[VXLAN][SHORTCUT]Original outer udp len: %d will be change to %d", ntohs(pPktHdr->outer_udph->len), ntohs(pPktHdr->outer_udph->len)-4);
		pPktHdr->outer_udph = (struct udphdr *)(RTSKB_DATA(rtskb)+ fc_db.l2DualTbl[l2DualTbl_index].vxlan_us.outer_udph_tag_off);
		pPktHdr->outer_udph->len = htons(ntohs(pPktHdr->outer_udph->len)-4) ;
	
	}
	if(isStag){
		pPktHdr->svh = NULL;
		pPktHdr->svlanid = 0;
		pPktHdr->svlanpri = 0;
	}else{
		if(pPktHdr->svh)
			pPktHdr->svh+=VLAN_HLEN;
		pPktHdr->cvh = NULL;
		pPktHdr->cvlanid = 0;
		pPktHdr->cvlanpri = 0;
	}

	return (SUCCESS);
}
#endif
#if defined(CONFIG_FC_RTL8277C_SERIES)

#endif


#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL9607C_SERIES)
void _rtk_fc_dualHeader_vxlan_get_fastFwd_outer_content(rtk_fc_pktHdr_t *pPktHdr)
{
	int offset = 0, inner_extra_len = 0;
	struct ethhdr tmp_outer_ethhdr;
	u8 *outer_header_content = &pPktHdr->vxlan_info.outer_content[0];
	if(pPktHdr == NULL)
		return;
	


	// DMAC(6)+SMAC(6)+[SVLAN(4)+CVLAN(4)]+Ethertype(2)+[PPPoE(8)]+IP header(20/40)+UDP(8)+VxLAN(8)
	memset(outer_header_content, 0, 86);
	
	//outer l2
	
	memcpy(&tmp_outer_ethhdr, pPktHdr->outer_eth, sizeof(struct ethhdr));
	tmp_outer_ethhdr.h_proto = ntohs(0x0800);
	memcpy(outer_header_content+offset, &tmp_outer_ethhdr, sizeof(struct ethhdr));
	offset +=sizeof(struct ethhdr);
	

	//outer ip
	if(pPktHdr->outer_iph)
	{
		memcpy(outer_header_content + offset, pPktHdr->outer_iph, sizeof(struct iphdr));

		offset += sizeof(struct iphdr);
	}
	else if(pPktHdr->outer_ip6h)
	{
		memcpy(outer_header_content + offset, pPktHdr->outer_ip6h, sizeof(struct ipv6hdr));
		offset += sizeof(struct ipv6hdr);
	}
	
	//outer udp
	
	
	memcpy(outer_header_content+offset, pPktHdr->outer_udph, sizeof(struct udphdr));
	offset += sizeof(struct udphdr);

	//outer vxlan
	memcpy(outer_header_content+offset, pPktHdr->vxlan_info.vxlanHdr, sizeof(rtk_fc_vxlanhdr_t));
	offset += sizeof(rtk_fc_vxlanhdr_t);

	if(pPktHdr->cvh)
	{
		DEBUG("[VXLAN][US]ingress inner has ctag");
		inner_extra_len+=4;
	}
	if(pPktHdr->svh)
	{
		DEBUG("[VXLAN][US]ingress inner has stag");
		inner_extra_len+=4;
	}
	if(pPktHdr->ppph)
	{
		DEBUG("[VXLAN][US]ingress inner has pppoetag");
		inner_extra_len+=8;
	}
	pPktHdr->vxlan_info.inner_extra_len = inner_extra_len;

	

	return;
}

#if 0
struct net_device* rtk_fc_vxlan_getOuterHdrDestDev(rtk_fc_pktHdr_t *pPktHdr, struct rt_skbuff *rtskb, int direction)
{
	int i =0;
	struct net_device *netif_dev=NULL;
	unsigned char outer_da[6] = {0};
	rtk_fc_ingress_data_t *pFcIngressData = RTSKB_FCIGRDATA(rtskb);

	if(direction == RTK_FC_L2DUAL_ACT_ADD)
		memcpy(&outer_da[0], &pPktHdr->eth->h_dest, 6);
	else if (direction == RTK_FC_L2DUAL_ACT_REMOVE)
		memcpy(&outer_da[0],&pFcIngressData->vxlan_info.outer_da[0], 6);

	
	for(i = 0; i< RTK_FC_TABLESIZE_INTF_SW; i++)
	{
		if(fc_db.netifTbl[i].intf.valid==FALSE)  continue;

		if( !memcmp(&fc_db.netifTbl[i].intf.gateway_mac_addr.octet, &outer_da[0], ETH_ALEN) &&
			(fc_db.netifTbl[i].intf.gateway_ipv4_addr==pFcIngressData->vxlan_info.outer_dstIp))
		{
			netif_dev = fc_db.netifTbl[i].dev;
			DEBUG("[VXLAN][%s] use netif_dev : %s (%pM)",direction==(RTK_FC_L2DUAL_ACT_ADD)?"Upstream":"Downstream", netif_dev->name, &outer_da[0]);
			break;
		}
	}

	return netif_dev;
}
struct net_device* rtk_fc_vxlan_getOuterHdrSourceDev(rtk_fc_pktHdr_t *pPktHdr, struct rt_skbuff *rtskb, int direction)
{
	int i =0;
	struct net_device *netif_dev=NULL;
	unsigned char outer_sa[6];

	memcpy(&outer_sa[0], &pPktHdr->outer_eth->h_source, 6);

	
	for(i = 0; i< RTK_FC_TABLESIZE_INTF_SW; i++)
	{
		if(fc_db.netifTbl[i].intf.valid==FALSE)  continue;

		if(pPktHdr->outer_iph)
		{
			if( !memcmp(&fc_db.netifTbl[i].intf.gateway_mac_addr.octet, &outer_sa[0], ETH_ALEN) &&
				(fc_db.netifTbl[i].intf.gateway_ipv4_addr==ntohl(pPktHdr->outer_iph->saddr)) 
			)
			{
				netif_dev = fc_db.netifTbl[i].dev;
				DEBUG("[VXLAN][%s] use source netif_dev : %s (%pM)",direction==(RTK_FC_L2DUAL_ACT_ADD)?"Upstream":"Downstream", netif_dev->name, &outer_sa[0]);
				break;
			}
		}
		else
		{
			DEBUG("ipv6 vxlan sw only!");
			return NULL;
		}
	}

	return netif_dev;
}
#endif
int _rtk_fc_vxlan_special_fastFwd_downstream_setting(rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo)
{
#if defined(CONFIG_FC_CA8277B_SERIES)
			int ret;
			// For Fast Forward
			if(fc_db.controlFuc.special_fast_forward_mode==1 && fc_db.vxlan_upstream_ca_extra_set ==0)
			{
				ca_ni_init_set_inner_extra_len(0,pG3IgrExtraInfo->vxlan_info.innerExtra_len);
				fc_db.vxlan_upstream_ca_extra_set = 1;
			}
			ret =  ca_ni_init_tx_dma_lso_special_fast_fwd_init_skb_pool();
			
			if(ret != SUCCESS)
			{
				return FC_VXLAN_FLOW_UPSTREAM_PREALLOCATE_SKB_FAILED;
			}
#elif defined(CONFIG_FC_RTL9607C_SERIES)
			// For Fast Forward

			if(fc_db.controlFuc.special_fast_forward_mode==1 &&
			   fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_mechanism && 
			   fc_db.vxlan_downstream_set_ok == 0
			  )
			{
				_rtk_fc_vxlan_fastfwd_set_fake_l2_entry(RTK_FC_L2DUAL_ACT_REMOVE, pG3IgrExtraInfo);
				
			}
			if(fc_db.controlFuc.special_fast_forward_mode==1 &&
			   fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamInfo_prepared==0 && 
			   fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamInfo_prepared==0
			  )
			{
				_rtk_fc_vxlan_fastfwd_downstream_setup_fc_db_fastFwd_data();	
			}

#endif
	return FC_VXLAN_FLOW_OK;
}
int _rtk_fc_l2_vxlan_special_fastFwd_upstream_setting(rtk_fc_pktHdr_t *pPktHdr, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo)
{
	if(fc_db.controlFuc.special_fast_forward_mode==1)
	{
#if defined(CONFIG_FC_CA8277B_SERIES)
		int inner_extra_len = 0;
		int ret, extraIndex = 0;

		// For Fast Forward
		
		inner_extra_len = pG3IgrExtraInfo->vxlan_info.innerExtra_len;//fc_db.l2DualTbl[l2DualTbl_idx].vxlan_us.inner_extra_len;

		
		//pG3IgrExtraInfo->vxlan_info.l2DualTableIndex = l2DualTbl_idx;
	
		
		ca_ni_init_extra_ff_global_param_setting();
		
		ret = ca_ni_init_tx_dma_lso_extra_ff_setting(pPktHdr->vxlan_info.outer_content , 50, inner_extra_len, &extraIndex);
		if(ret!= SUCCESS)
		{
			return FC_VXLAN_FLOW_EXTRA_SETTING_FAIL;
		}
		else if(extraIndex ==0)
		{
			return FC_VXLAN_FLOW_EXTRA_SETTING_FAIL;
		}
		else
		{
			pG3IgrExtraInfo->vxlan_info.extraIndex = extraIndex;
		}
		
		
#endif
	}
	return FC_VXLAN_FLOW_OK;



}

int _rtk_fc_vxlan_special_fastFwd_upstream_setting(rtk_fc_pktHdr_t *pPktHdr, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, int useExtra)
{
	if(fc_db.controlFuc.special_fast_forward_mode==1)
	{
#if defined(CONFIG_FC_CA8277B_SERIES)
		int inner_extra_len = 0;
		// For Fast Forward
		
		inner_extra_len = pPktHdr->vxlan_info.inner_extra_len;

		if(fc_db.vxlan_downstream_ca_extra_set ==0){
			ca_ni_init_set_inner_extra_len(1,inner_extra_len);
			fc_db.vxlan_downstream_ca_extra_set =1;
		}
		
		//pG3IgrExtraInfo->vxlan_info.l2DualTableIndex = l2DualTbl_idx;
		
		if(fc_db.txBuffInitDone==0)
		{
			int ret;
			ret = ca_ni_init_tx_dma_lso_special_fast_fwd(pPktHdr->vxlan_info.outer_content, 50, inner_extra_len);
			if(ret == SUCCESS)
			{	
				fc_db.txBuffInitDone = 1;
			}
			else
				return FC_VXLAN_FLOW_UPSTREAM_PREALLOCATE_SKB_FAILED;


			ret =  ca_ni_init_tx_dma_lso_special_fast_fwd_init_skb_pool();
			
			if(ret != SUCCESS)
			{
				return FC_VXLAN_FLOW_UPSTREAM_PREALLOCATE_SKB_FAILED;
			}
		
		}

		if(useExtra==1)
		{
			int ret, extraIndex = 0;
			
			ca_ni_init_extra_ff_global_param_setting();
		
			ret = ca_ni_init_tx_dma_lso_extra_ff_setting(pPktHdr->vxlan_info.outer_content, 50, inner_extra_len, &extraIndex);
			if(ret!= SUCCESS)
			{
				return FC_VXLAN_FLOW_EXTRA_SETTING_FAIL;
			}
			else if(extraIndex ==0)
			{
				return FC_VXLAN_FLOW_EXTRA_SETTING_FAIL;
			}
			else
			{
				pG3IgrExtraInfo->vxlan_info.extraIndex = extraIndex;
			}
		}
				
#elif defined(CONFIG_FC_RTL9607C_SERIES)

		//Setting up upstream/ extra upstream inner DA fake l2 entry
		if(fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_mechanism && fc_db.vxlan_upstream_set_ok == 0){
			_rtk_fc_vxlan_fastfwd_set_fake_l2_entry(RTK_FC_L2DUAL_ACT_ADD, pG3IgrExtraInfo);
			
		}

		// Setting up upstream	info  in fc_db_fastFwd_data 
		if(fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_mechanism && 
			(fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamInfo_prepared==0||fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamInfo_prepared==0)
		  )
		{
			int isExtra = 0;
			DEBUG("[VXLAN]Try to set nic FastFwd outer content");
			if((0x1<<pPktHdr->ingressPort.macPortIdx)&fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraPmsk)
				isExtra = 1;
			
			_rtk_fc_vxlan_fastfwd_upstream_setup_fc_db_fastFwd_data(pG3IgrExtraInfo, 
																	50,
																	pPktHdr->vxlan_info.outer_content, 
																	isExtra);
		}

		
#endif
	}
	return FC_VXLAN_FLOW_OK;



}

int _rtk_fc_l2_vxlan_down_stream_outer_flow_prepare(rtk_rg_asic_path3_entry_t *pFlowPath3, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint32 flowIdx,int igr_netifIdx)
{
#if defined(CONFIG_FC_CA8277B_SERIES)
	rtk_fc_ingress_data_t *pFcIngressData = RTSKB_FCIGRDATA(rtskb);	
	int outer_igrSVlanTagif = 0, outer_igrCVlanTagif = 0, outer_igrPPPoETagif = 0;
	
	{


		if(pFcIngressData->ingressTagif & OUTER_SVLAN_TAGIF)
			outer_igrSVlanTagif = TRUE;
		if(pFcIngressData->ingressTagif & OUTER_CVLAN_TAGIF)
			outer_igrCVlanTagif = TRUE;
		if(pFcIngressData->ingressTagif & OUTER_PPPOE_TAGIF)
			outer_igrPPPoETagif = TRUE;

		TRACE("outer_igrSVlanTagif = %d outer_igrCVlanTagif = %d outer_igrPPPoETagif = %d",outer_igrSVlanTagif, outer_igrCVlanTagif,outer_igrPPPoETagif);
		bzero(pFlowPath3, sizeof(*pFlowPath3));

		
		pFlowPath3->valid = TRUE;
		pFlowPath3->in_path = FB_PATH_34;
		pFlowPath3->in_multiple_act = FALSE;
		pFlowPath3->in_ipv4_or_ipv6 = 0;
		pFlowPath3->in_src_ipv4_addr =	(pPktHdr->vxlan_info.remote_v4_ip);
		pFlowPath3->in_dst_ipv4_addr =	(pPktHdr->vxlan_info.gateway_v4_ip);
		pFlowPath3->in_l4_src_port = 	(pFcIngressData->vxlan_info.outer_srcPort);
		
		pFlowPath3->in_l4_dst_port = (4789);
		pFlowPath3->in_l4proto_num = IPPROTO_UDP;

		pFlowPath3->in_intf_idx = igr_netifIdx;
		pFlowPath3->in_stagif = outer_igrSVlanTagif;
		pFlowPath3->in_ctagif = outer_igrCVlanTagif;
		pFlowPath3->in_pppoeif = outer_igrPPPoETagif;
		pFlowPath3->in_pppoe_sid_check = FALSE; 			//2Note: for pppoe passthrough, it must turn off!!
		pFlowPath3->in_tos_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH34_TOS];
		pFlowPath3->in_tos = 0;
		pFlowPath3->in_cvlan_pri = pFcIngressData->srcCVlanPri;

		//-------------------------egress action-------------------------

		pFlowPath3->out_uc_lut_lookup = FALSE;		// tx port refer to portmask and ext_portmask_idx
		pFlowPath3->out_intf_idx = 0;
			
		pFlowPath3->out_portmask = 1 << (pPktHdr->egressPort.macPortIdx);
		pFlowPath3->out_ext_portmask_idx = 0;
		
		pFlowPath3->out_smac_trans = 0;
		pFlowPath3->out_dmac_trans = 0;
		//svlan
		pFlowPath3->out_egress_svid_act = TRUE;
		pFlowPath3->out_svlan_id		= 0;
		pFlowPath3->out_stag_format_act = TRUE;
		pFlowPath3->out_svid_format_act = FALSE;
		pFlowPath3->out_spri_format_act = FALSE;
		pFlowPath3->out_spri 			= 0;
		//cvlan
		pFlowPath3->out_egress_cvid_act = TRUE;
		pFlowPath3->out_cvlan_id 		= 0;
		pFlowPath3->out_ctag_format_act = TRUE;
		pFlowPath3->out_cvid_format_act = FALSE;
		pFlowPath3->out_cpri_format_act = FALSE;
		pFlowPath3->out_cpri 			=  0;

		//other
		pFlowPath3->out_multiple_act = FALSE;
		pFlowPath3->out_user_pri_act = FALSE;
		pFlowPath3->out_user_priority = 0;
		if(fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH34_TOS])
		{
			// remarking dscp only when dscp as flow key
			pFlowPath3->out_dscp_act = TRUE;
			pFlowPath3->out_dscp = 0;
		}

		pFlowPath3->out_stream_idx_act = 0;
		pFlowPath3->out_stream_idx = 0;

		pFlowPath3->out_share_meter_act = 0;
		pFlowPath3->out_share_meter_idx = 0;

		pFlowPath3->out_flow_counter_act = 0;
		pFlowPath3->out_flow_counter_idx = 0;

		pFlowPath3->out_extra_tag_index = 0;
		pFlowPath3->out_egress_port_to_vid_act = FALSE;
		pFlowPath3->out_drop = 0;
		pFlowPath3->lock = FALSE;

	}

#endif
	return SUCCESS;
}

void rtk_fc_l2_vxlan_check_flow_canBeAdded(uint8 *flow_can_add, int direction , void *pPathData, 
														int inner_flowHashIdx, int outer_flowHashIdx, 
														rtk_fc_g3IgrExtraInfo_t * pG3IgrExtraInfo ,
												 		rtk_fc_pktHdr_t *pPktHdr, 
												 		struct rt_skbuff *rtskb,
												 		int igr_netifIdx, int* by_pass_vxlan_add)
{
#if defined(CONFIG_FC_CA8277B_SERIES)

	int canAdd = 1;
	int flowIndex = 0, ret = 0;
	int inner_exist = 0, outer_exist = 0;
	
	if(direction ==RTK_FC_FLOW_DIRECTION_DOWNSTREAM)
	{
		
		rtk_fc_g3IgrExtraInfo_t *outer_pG3IgrExtraInfo=NULL;
		rtk_rg_asic_path1_entry_t outerFlowEntry;
		rtk_rg_asic_path3_entry_t *outer_pFlowPath3 = (rtk_rg_asic_path3_entry_t *)&outerFlowEntry;
		TRACE("[VXLAN][DS]Flow index %d check inner flow free entry",inner_flowHashIdx);
		
		ret = _rtk_fc_flow_freeEntry_get(&flowIndex, inner_flowHashIdx, *(rtk_fc_tableFlowEntry_t *)pPathData, 0, 0, 0, 0, 0, pG3IgrExtraInfo, 0);
		if(ret == RTK_FC_RET_ERR_ENTRY_EXIST)
		{
			inner_exist = 1;
			DEBUG("[VXLAN][DOWNSTREAM]inner flow exist! Do nothing");
		}
		else if(ret !=RTK_FC_RET_OK )
		{
			DEBUG("[VXLAN][DOWNSTREAM]inner flow get entry failed!! Cannot add outer flow!ret = %x",ret);
			canAdd = 0;
		}


		outer_pG3IgrExtraInfo = RTK_FC_HELPER_FC_KMALLOC(sizeof(rtk_fc_g3IgrExtraInfo_t), GFP_ATOMIC);
		_rtk_fc_g3IgrExtraInfo_init(outer_pG3IgrExtraInfo);
		//prepare info that needed for adding G3 HW entry
		outer_pG3IgrExtraInfo->pon_stream_id = pG3IgrExtraInfo->pon_stream_id;
		
		_rtk_fc_l2_vxlan_down_stream_outer_flow_prepare(outer_pFlowPath3, rtskb, pPktHdr, outer_flowHashIdx, igr_netifIdx);

		TRACE("[VXLAN][DS]Flow index %d check outer flow free entry",outer_flowHashIdx);
		ret = _rtk_fc_flow_freeEntry_get(&flowIndex, outer_flowHashIdx, *(rtk_fc_tableFlowEntry_t *)(&outerFlowEntry), 0, 0, 0, 0, 0, outer_pG3IgrExtraInfo, 0);
		if(ret == RTK_FC_RET_ERR_ENTRY_EXIST)
		{
			outer_exist = 1;
			DEBUG("[VXLAN][DOWNSTREAM]outer flow exist! Do nothing");
		}
		else if(ret !=RTK_FC_RET_OK )
		{
			DEBUG("[VXLAN][DOWNSTREAM]outer flow get entry failed!! Cannot add outer flow!ret = %x",ret);
			canAdd = 0;
		}
		else if(flowIndex >= fc_db.flowHwTableSize)
		{
			if(fc_db.flowTbl[flowIndex].mainHashIdx!=G3_FLOWIDX_INVALID)
			{
				DEBUG("[VXLAN][DOWNSTREAM]outer flow is added before, mainHash[%d]",fc_db.flowTbl[flowIndex].mainHashIdx);
				canAdd = 0;
			}
		}
		//if(outer_pG3IgrExtraInfo) 
			RTK_FC_HELPER_FC_KFREE(outer_pG3IgrExtraInfo, sizeof(rtk_fc_g3IgrExtraInfo_t));
	}
	else
	{
		rtk_fc_g3IgrExtraInfo_t *outer_pG3IgrExtraInfo;
		rtk_rg_asic_path1_entry_t outerFlowEntry;
		
		//rtk_rg_asic_path5_entry_t *outer_pFlowPath5 = (rtk_rg_asic_path5_entry_t *)&outerFlowEntry;
		TRACE("[VXLAN][US]Flow index %d check inner flow free entry",inner_flowHashIdx);
		ret = _rtk_fc_flow_freeEntry_get(&flowIndex, inner_flowHashIdx, *(rtk_fc_tableFlowEntry_t *)pPathData, 0, 0, 0, 0, 0, pG3IgrExtraInfo, 0);
		if(ret == RTK_FC_RET_ERR_ENTRY_EXIST)
		{
			inner_exist = 1;
			DEBUG("[VXLAN][UPSTREAM]inner flow exist! Do nothing");
		}
		else if(ret !=RTK_FC_RET_OK )
		{
			DEBUG("[VXLAN][UPSTREAM]inner flow get entry failed!! Cannot add outer flow! ret = %x", ret);
			canAdd = 0;
		}
		else if(flowIndex >= fc_db.flowHwTableSize)
		{
			if(fc_db.flowTbl[flowIndex].mainHashIdx!=G3_FLOWIDX_INVALID)
			{
				DEBUG("[VXLAN][UPSTREAM]inner flow is added before, mainHash[%d]",fc_db.flowTbl[flowIndex].mainHashIdx);
				canAdd = 0;
			}			
		}

		outer_pG3IgrExtraInfo = RTK_FC_HELPER_FC_KMALLOC(sizeof(rtk_fc_g3IgrExtraInfo_t), GFP_ATOMIC);
		_rtk_fc_g3IgrExtraInfo_init(outer_pG3IgrExtraInfo);
		//prepare info that needed for adding G3 HW entry
		
	
		_rtk_fc_l2_vxlan_up_stream_outer_flow_prepare(&outerFlowEntry, rtskb, pPktHdr, igr_netifIdx);

		TRACE("[VXLAN][US]Flow index %d check outer flow free entry",outer_flowHashIdx);
				
		ret = _rtk_fc_flow_freeEntry_get(&flowIndex, outer_flowHashIdx, *(rtk_fc_tableFlowEntry_t *)(&outerFlowEntry), 0, 0, 0, 0, 0, outer_pG3IgrExtraInfo, 0);
		if(ret == RTK_FC_RET_ERR_ENTRY_EXIST)
		{
			outer_exist = 1;
			DEBUG("[VXLAN][UPSTREAM]outer flow exist! Do nothing");
		}
		else if(ret !=RTK_FC_RET_OK )
		{
			DEBUG("[VXLAN][UPSTREAM]outer flow get entry failed!! Cannot add outer flow!ret = %x",ret);
			canAdd = 0;
		}
			
		//if(outer_pG3IgrExtraInfo) 
			RTK_FC_HELPER_FC_KFREE(outer_pG3IgrExtraInfo, sizeof(rtk_fc_g3IgrExtraInfo_t));
	}
	
	if(canAdd == 0)
		*flow_can_add = 0;

	if(inner_exist == 1 && outer_exist == 1)
	{
		DEBUG("[VXLAN]Outer & Inner all exist: By-pass adding!");
		*by_pass_vxlan_add = 1;
	}
	
#endif	
}


void rtk_fc_vxlan_check_flow_canBeAdded(uint8 *flow_can_add, int direction , void *pPathData, 
														int inner_flowHashIdx, int outer_flowHashIdx, 
														rtk_fc_g3IgrExtraInfo_t * pG3IgrExtraInfo ,
												 		rtk_fc_pktHdr_t *pPktHdr, 
												 		struct rt_skbuff *rtskb,
												 		int igr_netifIdx, int egrNetifIdx, int* by_pass_vxlan_add)
{
	int canAdd = 1;
	int flowIndex = 0, ret = 0;
	int inner_exist = 0, outer_exist = 0;
	int32 true_outer_flow_hash = 0;
	
	if(direction ==RTK_FC_FLOW_DIRECTION_DOWNSTREAM)
	{
		
		rtk_fc_g3IgrExtraInfo_t *outer_pG3IgrExtraInfo=NULL;
		rtk_fc_tableFlowEntry_t outerFlowTable;
		//rtk_rg_asic_path1_entry_t outerFlowEntry;
		rtk_rg_asic_path3_entry_t *outer_pFlowPath3 = (rtk_rg_asic_path3_entry_t *)(&outerFlowTable.path1);// (rtk_rg_asic_path5_entry_t *)&outerFlowEntry;
		TRACE("[VXLAN][DS]Flow index %d check inner flow free entry",inner_flowHashIdx);
		
		ret = _rtk_fc_flow_freeEntry_get(&flowIndex, inner_flowHashIdx, *(rtk_fc_tableFlowEntry_t *)pPathData, 0, 0, 0, 0, 0, pG3IgrExtraInfo, 0);
		if(ret == RTK_FC_RET_ERR_ENTRY_EXIST)
		{
			inner_exist = 1;
			DEBUG("[VXLAN][DOWNSTREAM]inner flow exist! Do nothing");
		}
		else if(ret !=RTK_FC_RET_OK )
		{
			DEBUG("[VXLAN][DOWNSTREAM]inner flow get entry failed!! Cannot add outer flow!ret = %x",ret);
			canAdd = 0;
		}
#if defined(CONFIG_FC_RTL9607C_SERIES)	
		else if(flowIndex >= fc_db.flowHwTableSize)
		{	
			DEBUG("[VXLAN][DOWNSTREAM]inner flow is sw flow!! Cannot add outer flow!");
			canAdd = 0;
		}
#endif

		outer_pG3IgrExtraInfo = RTK_FC_HELPER_FC_KMALLOC(sizeof(rtk_fc_g3IgrExtraInfo_t), GFP_ATOMIC);
		_rtk_fc_g3IgrExtraInfo_init(outer_pG3IgrExtraInfo);
		//prepare info that needed for adding G3 HW entry
		outer_pG3IgrExtraInfo->pon_stream_id = pG3IgrExtraInfo->pon_stream_id;
		
		_rtk_fc_vxlan_down_stream_outer_flow_prepare(outer_pFlowPath3, rtskb, pPktHdr, outer_flowHashIdx, igr_netifIdx, egrNetifIdx, 1);

		true_outer_flow_hash = _rtk_fc_flow_hashIndex(outerFlowTable, fc_db.netifTbl[NETIF_HWTOSW(igr_netifIdx)].dualUniInfo.vxlan_netif_info.outer_svlan_id, 
																	  fc_db.netifTbl[NETIF_HWTOSW(igr_netifIdx)].dualUniInfo.vxlan_netif_info.outer_cvlan_id, FALSE);

		TRACE("[VXLAN][DS]Flow index %d check outer flow free entry",true_outer_flow_hash);
		ret = _rtk_fc_flow_freeEntry_get(&flowIndex, true_outer_flow_hash, *(rtk_fc_tableFlowEntry_t *)(&(outerFlowTable.path1)), 0, 0, 0, 0, 0, outer_pG3IgrExtraInfo, 0);
		if(ret == RTK_FC_RET_ERR_ENTRY_EXIST)
		{
#if 1//defined(CONFIG_FC_CA8277B_SERIES)	
			outer_exist = 1;
			DEBUG("[VXLAN][DOWNSTREAM]outer flow exist! Do nothing");
#endif			
		}
		else if(ret !=RTK_FC_RET_OK )
		{
			DEBUG("[VXLAN][DOWNSTREAM]outer flow get entry failed!! Cannot add outer flow!ret = %x",ret);
			canAdd = 0;
		}
		else if(flowIndex >= fc_db.flowHwTableSize)
		{
#if defined(CONFIG_FC_RTL9607C_SERIES)		
			DEBUG("[VXLAN][DOWNSTREAM]outer flow is sw flow!! Cannot add outer flow!");
			canAdd = 0;
#elif defined(CONFIG_FC_CA8277B_SERIES)
			if(fc_db.flowTbl[flowIndex].mainHashIdx!=G3_FLOWIDX_INVALID)
			{
				DEBUG("[VXLAN][DOWNSTREAM]outer flow is added before, mainHash[%d]",fc_db.flowTbl[flowIndex].mainHashIdx);
				canAdd = 0;
			}
#endif
		}
		//if(outer_pG3IgrExtraInfo) 
			RTK_FC_HELPER_FC_KFREE(outer_pG3IgrExtraInfo, sizeof(rtk_fc_g3IgrExtraInfo_t));
	}
	else
	{
		rtk_fc_g3IgrExtraInfo_t *outer_pG3IgrExtraInfo;
		rtk_rg_asic_path1_entry_t outerFlowEntry;
		
		//rtk_rg_asic_path5_entry_t *outer_pFlowPath5 = (rtk_rg_asic_path5_entry_t *)&outerFlowEntry;
		TRACE("[VXLAN][US]Flow index %d check inner flow free entry",inner_flowHashIdx);
		ret = _rtk_fc_flow_freeEntry_get(&flowIndex, inner_flowHashIdx, *(rtk_fc_tableFlowEntry_t *)pPathData, 0, 0, 0, 0, 0, pG3IgrExtraInfo, 0);
		if(ret == RTK_FC_RET_ERR_ENTRY_EXIST)
		{
			inner_exist = 1;
			DEBUG("[VXLAN][UPSTREAM]inner flow exist! Do nothing");
		}
		else if(ret !=RTK_FC_RET_OK )
		{
			DEBUG("[VXLAN][UPSTREAM]inner flow get entry failed!! Cannot add outer flow! ret = %x", ret);
			canAdd = 0;
		}
		else if(flowIndex >= fc_db.flowHwTableSize)
		{
#if defined(CONFIG_FC_RTL9607C_SERIES)		
			DEBUG("[VXLAN][UPSTREAM]inner flow is sw flow!! Cannot add outer flow!");
			canAdd = 0;
#elif defined(CONFIG_FC_CA8277B_SERIES)
			if(fc_db.flowTbl[flowIndex].mainHashIdx!=G3_FLOWIDX_INVALID)
			{
				DEBUG("[VXLAN][UPSTREAM]inner flow is added before, mainHash[%d]",fc_db.flowTbl[flowIndex].mainHashIdx);
				canAdd = 0;
			}
#endif			
		}

		outer_pG3IgrExtraInfo = RTK_FC_HELPER_FC_KMALLOC(sizeof(rtk_fc_g3IgrExtraInfo_t), GFP_ATOMIC);
		_rtk_fc_g3IgrExtraInfo_init(outer_pG3IgrExtraInfo);
		//prepare info that needed for adding G3 HW entry
		
	
		_rtk_fc_vxlan_up_stream_outer_flow_prepare(&outerFlowEntry, rtskb, pPktHdr, igr_netifIdx);

		TRACE("[VXLAN][US]Flow index %d check outer flow free entry",outer_flowHashIdx);
				
		ret = _rtk_fc_flow_freeEntry_get(&flowIndex, outer_flowHashIdx, *(rtk_fc_tableFlowEntry_t *)(&outerFlowEntry), 0, 0, 0, 0, 0, outer_pG3IgrExtraInfo, 0);
		if(ret == RTK_FC_RET_ERR_ENTRY_EXIST)
		{
#if 1//defined(CONFIG_FC_CA8277B_SERIES)		
			outer_exist = 1;
			DEBUG("[VXLAN][UPSTREAM]outer flow exist! Do nothing");
#endif
		}
		else if(ret !=RTK_FC_RET_OK )
		{
			DEBUG("[VXLAN][UPSTREAM]outer flow get entry failed!! Cannot add outer flow!ret = %x",ret);
			canAdd = 0;
		}
#if defined(CONFIG_FC_RTL9607C_SERIES)		
		else if(flowIndex >= fc_db.flowHwTableSize)
		{

			DEBUG("[VXLAN][UPSTREAM]outer flow is sw flow!! Cannot add outer flow!");
			canAdd = 0;	
		}
#endif				
		//if(outer_pG3IgrExtraInfo) 
			RTK_FC_HELPER_FC_KFREE(outer_pG3IgrExtraInfo, sizeof(rtk_fc_g3IgrExtraInfo_t));
	}
	
	if(canAdd == 0)
		*flow_can_add = 0;

	if(inner_exist == 1 && outer_exist == 1)
	{
		DEBUG("[VXLAN]Outer & Inner all exist: By-pass adding!");
		*by_pass_vxlan_add = 1;
	}
	
	
}
int _rtk_fc_l2_vxlan_up_stream_outer_flow_prepare(rtk_rg_asic_path1_entry_t *pFlowEntry, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, int igr_netifIdx)
{
#if defined(CONFIG_FC_CA8277B_SERIES)	

	int outer_igrSVlanTagif = 0, outer_igrCVlanTagif = 0, outer_igrPPPoETagif = 0;

	if(pPktHdr->outer_svh)
		outer_igrSVlanTagif = TRUE;
	if(pPktHdr->outer_cvh)
		outer_igrCVlanTagif = TRUE;
	if(pPktHdr->outer_ppph)
		outer_igrPPPoETagif = TRUE;
	
	{
		rtk_rg_asic_path3_entry_t *pFlowPath3 = (rtk_rg_asic_path3_entry_t *)pFlowEntry;
		
		bzero(pFlowPath3, sizeof(*pFlowPath3));

		// ========================  ip header bridge flow ========================
		
		pFlowPath3->valid = TRUE;
		pFlowPath3->in_path = FB_PATH_34;
		pFlowPath3->in_multiple_act = FALSE;
		pFlowPath3->in_ipv4_or_ipv6 = 0;
		pFlowPath3->in_src_ipv4_addr =	ntohl(pPktHdr->outer_iph->saddr);
		pFlowPath3->in_dst_ipv4_addr =	ntohl(pPktHdr->outer_iph->daddr);
		pFlowPath3->in_l4_src_port = ntohs(pPktHdr->outer_udph->source);
		pFlowPath3->in_l4_dst_port = ntohs(pPktHdr->outer_udph->dest);
		pFlowPath3->in_l4proto_num = IPPROTO_UDP;

		//-------------------------egress action-------------------------

		pFlowPath3->out_uc_lut_lookup = FALSE;		// tx port refer to portmask and ext_portmask_idx
		pFlowPath3->out_intf_idx = igr_netifIdx;
		
		pFlowPath3->out_portmask = 1 << (pPktHdr->egressPort.macPortIdx);
		pFlowPath3->out_ext_portmask_idx = 0;
		
		pFlowPath3->out_smac_trans = FALSE;
		pFlowPath3->out_dmac_trans = 0;
		//svlan
		pFlowPath3->out_egress_svid_act = outer_igrSVlanTagif ? TRUE : FALSE;
		pFlowPath3->out_svlan_id = outer_igrSVlanTagif ? ntohs(pPktHdr->outer_svh->h_vlan_TCI)&VLAN_VID_MASK:0;
		pFlowPath3->out_stag_format_act = TRUE;
		pFlowPath3->out_svid_format_act = outer_igrSVlanTagif?TRUE:FALSE;
		pFlowPath3->out_spri_format_act = (outer_igrSVlanTagif)?TRUE:FALSE;
		pFlowPath3->out_spri = outer_igrSVlanTagif ? ((ntohs(pPktHdr->outer_svh->h_vlan_TCI)&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT):0;
		//cvlan
		pFlowPath3->out_egress_cvid_act = TRUE;
		pFlowPath3->out_cvlan_id = outer_igrCVlanTagif ? ntohs(pPktHdr->outer_cvh->h_vlan_TCI)&VLAN_VID_MASK:0;;
		pFlowPath3->out_ctag_format_act = TRUE;
		pFlowPath3->out_cvid_format_act = (outer_igrCVlanTagif)?TRUE:FALSE;
		pFlowPath3->out_cpri_format_act = (outer_igrCVlanTagif)?TRUE:FALSE;
	
		pFlowPath3->out_cpri = outer_igrCVlanTagif ? ((ntohs(pPktHdr->outer_cvh->h_vlan_TCI)&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT):0;
		

		//other
		pFlowPath3->out_multiple_act = FALSE;
		if(pPktHdr->remarkDec.outputQid)
			pFlowPath3->out_user_pri_act = TRUE;
		else
			pFlowPath3->out_user_pri_act = FALSE;
		pFlowPath3->out_user_priority = pPktHdr->remarkDec.outputQid;
		if(fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH34_TOS])
		{
			// remarking dscp only when dscp as flow key
			pFlowPath3->out_dscp_act = TRUE;
			pFlowPath3->out_dscp = ((pPktHdr->outer_iph->tos)>>2) & 0x3f;
		}

		pFlowPath3->out_stream_idx_act = pPktHdr->remarkDec.streamId_en;
		pFlowPath3->out_stream_idx = pPktHdr->remarkDec.streamId;

		pFlowPath3->out_share_meter_act = pPktHdr->remarkDec.meterIdx_en;
		pFlowPath3->out_share_meter_idx = pPktHdr->remarkDec.meterIdx;

		pFlowPath3->out_flow_counter_act = pPktHdr->remarkDec.mibIdx_en;
		pFlowPath3->out_flow_counter_idx = pPktHdr->remarkDec.mibIdx;

		pFlowPath3->out_extra_tag_index = 0;
		pFlowPath3->out_egress_port_to_vid_act = FALSE;
		pFlowPath3->out_drop = 0;
		pFlowPath3->lock = (pFlowPath3->out_share_meter_act) ? TRUE: FALSE;


	}
	
#endif
	return SUCCESS;
}

int _rtk_fc_vxlan_up_stream_outer_flow_prepare(rtk_rg_asic_path1_entry_t *pFlowEntry, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, int igr_netifIdx)
{

	//rtk_fc_ingress_data_t *pFcIngressData = RTSKB_FCIGRDATA(rtskb);	
	int outer_igrSVlanTagif = 0, outer_igrCVlanTagif = 0, outer_igrPPPoETagif = 0;
#if defined(CONFIG_FC_RTL9607C_SERIES)
	int out_intf_idx = igr_netifIdx;
#endif

	if(pPktHdr->outer_svh)
		outer_igrSVlanTagif = TRUE;
	if(pPktHdr->outer_cvh)
		outer_igrCVlanTagif = TRUE;
	if(pPktHdr->outer_ppph)
		outer_igrPPPoETagif = TRUE;
	//Outbound or Inbound
	TRACE("outer_igrSVlanTagif = %d outer_igrCVlanTagif = %d outer_igrPPPoETagif = %d igr_netifIdx = %d",outer_igrSVlanTagif, outer_igrCVlanTagif,outer_igrPPPoETagif, igr_netifIdx);

#if defined(CONFIG_FC_RTL9607C_SERIES)

	{
		rtk_rg_asic_path3_entry_t *pFlowPath3 = (rtk_rg_asic_path3_entry_t *)pFlowEntry;
		
		bzero(pFlowPath3, sizeof(*pFlowPath3));

		// ========================  ip header bridge flow ========================
		
		pFlowPath3->valid = TRUE;
		pFlowPath3->in_path = FB_PATH_34;
		pFlowPath3->in_multiple_act = FALSE;
		pFlowPath3->in_ipv4_or_ipv6 = 0;
		pFlowPath3->in_src_ipv4_addr =	ntohl(pPktHdr->outer_iph->saddr);
		pFlowPath3->in_dst_ipv4_addr =	ntohl(pPktHdr->outer_iph->daddr);
		if((0x1<<pPktHdr->ingressPort.macPortIdx)&fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraPmsk)
			pFlowPath3->in_l4_src_port = RTK_FC_VXLAN_ACC_EPHEMERAL_SPORT;
		else
			pFlowPath3->in_l4_src_port = ntohs(pPktHdr->outer_udph->source);
		pFlowPath3->in_l4_dst_port = ntohs(pPktHdr->outer_udph->dest);
		pFlowPath3->in_l4proto = 0; 					//tcp: 1, udp: 0

		//pFlowPath3->in_intf_idx = igrNetifIdx;
		//pFlowPath3->in_stagif = igrSVlanTagif;
		//pFlowPath3->in_ctagif = igrCVlanTagif;
		//pFlowPath3->in_pppoeif = igrPPPoETagif;
		//pFlowPath3->in_pppoe_sid_check = FALSE;				//2Note: for pppoe passthrough, it must turn off!!
		//pFlowPath3->in_tos_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH34_TOS];
		//pFlowPath3->in_tos = isIpv6 ? pFcIngressData->v6tos : pFcIngressData->v4tos;
		//pFlowPath3->in_cvlan_pri = pFcIngressData->srcCVlanPri;

		//-------------------------egress action-------------------------

		pFlowPath3->out_uc_lut_lookup = FALSE;		// tx port refer to portmask and ext_portmask_idx
		pFlowPath3->out_intf_idx = out_intf_idx;
		
		pFlowPath3->out_portmask = 1 << (pPktHdr->egressPort.macPortIdx);
		pFlowPath3->out_ext_portmask_idx = 0;
		
		pFlowPath3->out_smac_trans = FALSE;
		pFlowPath3->out_dmac_trans = 0;
		//svlan
		pFlowPath3->out_egress_svid_act = outer_igrSVlanTagif ? TRUE : FALSE;
		pFlowPath3->out_svlan_id = outer_igrSVlanTagif ? ntohs(pPktHdr->outer_svh->h_vlan_TCI)&VLAN_VID_MASK:0;
		pFlowPath3->out_stag_format_act = TRUE;
		pFlowPath3->out_svid_format_act = outer_igrSVlanTagif?TRUE:FALSE;
		pFlowPath3->out_spri_format_act = (outer_igrSVlanTagif)?TRUE:FALSE;
		pFlowPath3->out_spri = outer_igrSVlanTagif ? ((ntohs(pPktHdr->outer_svh->h_vlan_TCI)&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT):0;
		//cvlan
		pFlowPath3->out_egress_cvid_act = TRUE;
		pFlowPath3->out_cvlan_id = outer_igrCVlanTagif ? ntohs(pPktHdr->outer_cvh->h_vlan_TCI)&VLAN_VID_MASK:0;;
		pFlowPath3->out_ctag_format_act = TRUE;
		pFlowPath3->out_cvid_format_act = (outer_igrCVlanTagif)?TRUE:FALSE;
		pFlowPath3->out_cpri_format_act = (outer_igrCVlanTagif)?TRUE:FALSE;
	
		pFlowPath3->out_cpri = outer_igrCVlanTagif ? ((ntohs(pPktHdr->outer_cvh->h_vlan_TCI)&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT):0;
		

		//other
		pFlowPath3->out_multiple_act = FALSE;
		if(pPktHdr->remarkDec.outputQid)
			pFlowPath3->out_user_pri_act = TRUE;
		else
			pFlowPath3->out_user_pri_act = FALSE;
		pFlowPath3->out_user_priority = pPktHdr->remarkDec.outputQid;
		if(fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH34_TOS])
		{
			// remarking dscp only when dscp as flow key
			pFlowPath3->out_dscp_act = TRUE;
			pFlowPath3->out_dscp = ((pPktHdr->outer_iph->tos)>>2) & 0x3f;
		}

		pFlowPath3->out_stream_idx_act = pPktHdr->remarkDec.streamId_en;
		pFlowPath3->out_stream_idx = pPktHdr->remarkDec.streamId;

		pFlowPath3->out_share_meter_act = pPktHdr->remarkDec.meterIdx_en;
		pFlowPath3->out_share_meter_idx = pPktHdr->remarkDec.meterIdx;

		pFlowPath3->out_flow_counter_act = pPktHdr->remarkDec.mibIdx_en;
		pFlowPath3->out_flow_counter_idx = pPktHdr->remarkDec.mibIdx;

		pFlowPath3->out_extra_tag_index = 0;
		pFlowPath3->out_egress_port_to_vid_act = FALSE;
		pFlowPath3->out_drop = 0;
		pFlowPath3->lock = (pFlowPath3->out_share_meter_act) ? TRUE: FALSE;


	}
#elif defined(CONFIG_FC_CA8277B_SERIES)	
	{
		rtk_rg_asic_path5_entry_t *pFlowPath5 = (rtk_rg_asic_path5_entry_t *)pFlowEntry;
		

		bzero(pFlowPath5, sizeof(*pFlowPath5));
		
		pFlowPath5->valid = TRUE;

		pFlowPath5->in_path = FB_PATH_5;
		pFlowPath5->in_ipv4_or_ipv6 = 0;
		pFlowPath5->in_src_ipv4_addr = ntohl(pPktHdr->outer_iph->saddr);
		pFlowPath5->in_dst_ipv4_addr = ntohl(pPktHdr->outer_iph->daddr);
	
		pFlowPath5->in_l4_src_port = ntohs(pPktHdr->outer_udph->source);
		pFlowPath5->in_l4_dst_port = ntohs(pPktHdr->outer_udph->dest);
		pFlowPath5->in_l4proto_num = IPPROTO_UDP;



		//-------------------------egress action-------------------------

		pFlowPath5->out_l4_act = 0;														// Routing: 0, NAPT: 1
		
		pFlowPath5->out_l4_direction =1;
		
		
		barrier();
		
		//svlan
			
		//pFlowPath5->in_intf_idx = outer_igrNetifIdx;

		pFlowPath5->out_svid_format_act = outer_igrSVlanTagif?TRUE:FALSE;
		pFlowPath5->out_svlan_id = outer_igrSVlanTagif ? ntohs(pPktHdr->outer_svh->h_vlan_TCI)&VLAN_VID_MASK:0;

		pFlowPath5->out_spri = outer_igrSVlanTagif ? ((ntohs(pPktHdr->outer_svh->h_vlan_TCI)&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT):0;
		//cvlan

		
		pFlowPath5->out_cvid_format_act = outer_igrCVlanTagif?TRUE: FALSE;
		pFlowPath5->out_cvlan_id = outer_igrCVlanTagif ? ntohs(pPktHdr->outer_cvh->h_vlan_TCI)&VLAN_VID_MASK:0;
		pFlowPath5->out_cpri = outer_igrCVlanTagif ?  ((ntohs(pPktHdr->outer_cvh->h_vlan_TCI)&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT):0;

		if(pPktHdr->remarkDec.outputQid)
			pFlowPath5->out_user_pri_act = TRUE;

		pFlowPath5->out_user_priority = pPktHdr->remarkDec.outputQid;

		pFlowPath5->out_stream_idx_act = pPktHdr->remarkDec.streamId_en;
		pFlowPath5->out_stream_idx = pPktHdr->remarkDec.streamId;

		if(fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH5_TOS])
		{
			// remarking dscp only when dscp as flow key
			pFlowPath5->out_dscp_act = TRUE;
			pFlowPath5->out_dscp = ((pPktHdr->outer_iph->tos)>>2) & 0x3f;
		}
	
		pFlowPath5->out_share_meter_act = pPktHdr->remarkDec.meterIdx_en;
		pFlowPath5->out_share_meter_idx = pPktHdr->remarkDec.meterIdx;

		pFlowPath5->out_flow_counter_act = pPktHdr->remarkDec.mibIdx_en;
		pFlowPath5->out_flow_counter_idx = pPktHdr->remarkDec.mibIdx;


	}
#endif

	return SUCCESS;
}

void _rtk_fc_vxlan_8277b_reservedACL_del(void)
{
#if defined(CONFIG_FC_CA8277B_SERIES)
	int i;
	if(fc_db.vxlan_us_fastForward_num ==0 && fc_db.vxlan_ds_fastForward_num ==0)
	{

		TABLE("[VXLAN]Reset reserved acl for vxlan %s", fc_db.vxlan_specialFastFwd_setup_l3cls?"and reflash l2tp rsv acl":"");
		for(i = 0 ; i < 4 ; i++)
		{
			/*Upstream: cls-68 */
			if(fc_db.vxlan_l3cls_upstream_isSet[i] == 1){
				if(fc_db.vxlan_l3cls_info.pktlen_us_68_aclInfo[i] != CA_UINT32_INVALID )
				{
					assert_ok(_rtk_rg_aclAndCfReservedRuleDelSpecial(RTK_CA_CLS_TYPE_VXLAN_FFD_US, &fc_db.vxlan_l3cls_info.pktlen_us_68_aclInfo[i]));
					fc_db.vxlan_l3cls_info.pktlen_us_68_aclInfo[i] = CA_UINT32_INVALID ;
				}
						if(fc_db.vxlan_l3cls_info.pktlen_us_128_aclInfo[i] != CA_UINT32_INVALID )
						{
							assert_ok(_rtk_rg_aclAndCfReservedRuleDelSpecial(RTK_CA_CLS_TYPE_VXLAN_FFD_US, &fc_db.vxlan_l3cls_info.pktlen_us_128_aclInfo[i]));
							fc_db.vxlan_l3cls_info.pktlen_us_128_aclInfo[i] = CA_UINT32_INVALID ;
						}
				fc_db.vxlan_l3cls_upstream_isSet[i] = 0;
			}
			/*Down stream: cls - 68/others*/
			if(fc_db.vxlan_l3cls_downstream_isSet[i] == 1){
				if(fc_db.vxlan_l3cls_info.pktlen_ds_68_aclInfo[i] != CA_UINT32_INVALID )
				{
					assert_ok(_rtk_rg_aclAndCfReservedRuleDelSpecial(RTK_CA_CLS_TYPE_VXLAN_FFD_DS, &fc_db.vxlan_l3cls_info.pktlen_ds_68_aclInfo[i]));
					fc_db.vxlan_l3cls_info.pktlen_ds_68_aclInfo[i] = CA_UINT32_INVALID;
				}
				if(fc_db.vxlan_l3cls_info.pktlen_ds_others_aclInfo[i] != CA_UINT32_INVALID )
				{
					assert_ok(_rtk_rg_aclAndCfReservedRuleDelSpecial(RTK_CA_CLS_TYPE_VXLAN_FFD_DS, &fc_db.vxlan_l3cls_info.pktlen_ds_others_aclInfo[i]));
					fc_db.vxlan_l3cls_info.pktlen_ds_others_aclInfo[i] = CA_UINT32_INVALID;
				}
				fc_db.vxlan_l3cls_downstream_isSet[i] = 0;
			}					
		}
		if( fc_db.vxlan_ds_extraCLS_isSet) 
		{
			if(fc_db.vxlan_ds_extraCLS_Info != CA_UINT32_INVALID )
			{
				assert_ok(_rtk_rg_aclAndCfReservedRuleDelSpecial(RTK_CA_CLS_TYPE_VXLAN_FFD_EXTRA_DS, &fc_db.vxlan_ds_extraCLS_Info));
				fc_db.vxlan_ds_extraCLS_Info = CA_UINT32_INVALID ;
			}
			fc_db.vxlan_ds_extraCLS_isSet = 0;
		}
		if(fc_db.vxlan_specialFastFwd_setup_l3cls){
			TABLE("[VXLAN]Reset reserved acl & user acl!");
			fc_db.vxlan_specialFastFwd_setup_l3cls = 0;
			// readd reserved acl
			_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_TAIL_END, NULL);
		}

		
	}
#endif
}

int _rtk_fc_vxlan_down_stream_outer_flow_prepare(rtk_rg_asic_path3_entry_t *pFlowPath3, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, uint32 flowIdx,int igrNetifIdx,int egrNetifIdx, int is_checking_flow_can_add)
{

	rtk_fc_ingress_data_t *pFcIngressData = RTSKB_FCIGRDATA(rtskb);	
	int outer_igrSVlanTagif = 0, outer_igrCVlanTagif = 0, outer_igrPPPoETagif = 0;
#if defined(CONFIG_FC_RTL9607C_SERIES)
	int idx, ret;
#endif


	{
		if(pFcIngressData->ingressTagif & OUTER_SVLAN_TAGIF)
			outer_igrSVlanTagif = TRUE;
		if(pFcIngressData->ingressTagif & OUTER_CVLAN_TAGIF)
			outer_igrCVlanTagif = TRUE;
		if(pFcIngressData->ingressTagif & OUTER_PPPOE_TAGIF)
			outer_igrPPPoETagif = TRUE;
		//Outbound or Inbound
		TRACE("outer_igrSVlanTagif = %d outer_igrCVlanTagif = %d outer_igrPPPoETagif = %d",outer_igrSVlanTagif, outer_igrCVlanTagif,outer_igrPPPoETagif);
		bzero(pFlowPath3, sizeof(*pFlowPath3));
		
		pFlowPath3->valid = TRUE;

		pFlowPath3->in_path = FB_PATH_34;
		pFlowPath3->in_ipv4_or_ipv6 = 0;
		pFlowPath3->in_src_ipv4_addr = pPktHdr->vxlan_info.remote_v4_ip;// (pFcIngressData->vxlan_info.outer_srcIp);
		pFlowPath3->in_dst_ipv4_addr = pPktHdr->vxlan_info.gateway_v4_ip;
#if defined(CONFIG_FC_RTL9607C_SERIES)
		if((0x1<<pPktHdr->egressPort.macPortIdx)&fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraPmsk)
			pFlowPath3->in_l4_src_port = RTK_FC_VXLAN_ACC_EPHEMERAL_SPORT;
		else
			pFlowPath3->in_l4_src_port = (pFcIngressData->vxlan_info.outer_srcPort);
#else
		pFlowPath3->in_l4_src_port = (pFcIngressData->vxlan_info.outer_srcPort);
#endif
		pFlowPath3->in_l4_dst_port =  (4789);
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
		pFlowPath3->in_l4proto_num = IPPROTO_UDP;
#else
		pFlowPath3->in_l4proto = 0;
#endif
#if defined(CONFIG_RTK_L34_G3_PLATFORM)

		DEBUG("in_src_ipv4_addr: %x, in_dst_ipv4_addr: %x in_l4proto = %d, srcPort = %d dstPort = %d",pFlowPath3->in_src_ipv4_addr, pFlowPath3->in_dst_ipv4_addr, pFlowPath3->in_l4proto_num,
																				pFlowPath3->in_l4_src_port,pFlowPath3->in_l4_dst_port);
#else
		DEBUG("in_src_ipv4_addr: %x, in_dst_ipv4_addr: %x in_l4proto = %d, srcPort = %d dstPort = %d",pFlowPath3->in_src_ipv4_addr, pFlowPath3->in_dst_ipv4_addr, pFlowPath3->in_l4proto,
																				pFlowPath3->in_l4_src_port,pFlowPath3->in_l4_dst_port);
#endif
		pFlowPath3->in_intf_idx = igrNetifIdx;
		pFlowPath3->in_stagif = outer_igrSVlanTagif;
		pFlowPath3->in_ctagif = outer_igrCVlanTagif;
		pFlowPath3->in_pppoeif = outer_igrPPPoETagif;
		pFlowPath3->in_tos_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH5_TOS];
		pFlowPath3->in_tos = 0;//pFcIngressData->vxlan_info.outer_v4tos;//FIXME
		pFlowPath3->in_cvlan_pri = fc_db.netifTbl[NETIF_HWTOSW(igrNetifIdx)].dualUniInfo.vxlan_netif_info.outer_ctag_pri; //pFcIngressData->outer_srcCVlanPri;
		DEBUG("igrNetifIdx = %d(HW), sw:%d ",igrNetifIdx, NETIF_HWTOSW(igrNetifIdx));
		//-------------------------egress action-------------------------

		//pFlowPath3->out_l4_act = 0;														// Routing: 0, NAPT: 1
		
		//pFlowPath3->out_l4_direction =0;
		
		//pFlowPath3->out_l4_port = 0;													// don't care if routing

		pFlowPath3->out_intf_idx = egrNetifIdx;
#if defined(CONFIG_FC_RTL9607C_SERIES)

		if(is_checking_flow_can_add==0)
		{
			if((0x1<<pPktHdr->egressPort.macPortIdx)&fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraPmsk)
			{
				ret = rtk_fc_indMac_idx_get(fc_db.vxlan_accelerate_extra_downstreamL2Idx, &idx);
				if (ret != RTK_FC_RET_OK)
				{
					WARNING("ret = 0x%x! fc_db.vxlan_accelerate_extra_downstreamL2Idx = %d", ret, fc_db.vxlan_accelerate_extra_downstreamL2Idx);
				}
			}
			else
			{
				ret = rtk_fc_indMac_idx_get(fc_db.vxlan_accelerate_downstreamL2Idx, &idx);
				
				if (ret != RTK_FC_RET_OK)
				{
					WARNING("ret = 0x%x! fc_db.vxlan_accelerate_extra_downstreamL2Idx = %d", ret,fc_db.vxlan_accelerate_downstreamL2Idx);
				}
			}
			pFlowPath3->out_dmac_idx = idx;

		}
#else
		pFlowPath3->out_dmac_idx = fc_db.flowTbl[flowIdx].pFlowEntry->path5.out_dmac_idx;
#endif		
		
		barrier();
		
		//svlan

		pFlowPath3->out_stag_format_act = TRUE;
		
		pFlowPath3->out_egress_svid_act = TRUE;

		pFlowPath3->out_svid_format_act =FALSE;

		pFlowPath3->out_svlan_id = 0;

		pFlowPath3->out_spri_format_act = FALSE;
		pFlowPath3->out_spri = 0;

		//cvlan
		pFlowPath3->out_egress_cvid_act = TRUE;

		pFlowPath3->out_ctag_format_act = TRUE;

		pFlowPath3->out_cvid_format_act = FALSE;

		pFlowPath3->out_cvlan_id =	0;

		pFlowPath3->out_cpri_format_act = FALSE;

		pFlowPath3->out_cpri = 0;
		pFlowPath3->out_uc_lut_lookup = TRUE;
		pFlowPath3->out_dmac_trans = TRUE;

#if defined(CONFIG_FC_CA8277B_SERIES)
		pFlowPath3->out_user_pri_act = FALSE;
		pFlowPath3->out_user_priority = 0;
#elif defined(CONFIG_FC_RTL9607C_SERIES)
		pFlowPath3->out_user_pri_act = TRUE;
		if((0x1<<pPktHdr->egressPort.macPortIdx)&fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraPmsk)
			pFlowPath3->out_user_priority = fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamPriority;
		else
			pFlowPath3->out_user_priority = fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamPriority;
	

#endif
		pFlowPath3->out_dscp_act = TRUE;
		pFlowPath3->out_dscp =0;

		pFlowPath3->out_stream_idx_act = 0;
		pFlowPath3->out_stream_idx = 0;

		pFlowPath3->out_share_meter_act = 0;
		pFlowPath3->out_share_meter_idx = 0;

		pFlowPath3->out_flow_counter_act =0;
		pFlowPath3->out_flow_counter_idx = 0;

		pFlowPath3->out_extra_tag_index = 0;

		pFlowPath3->out_egress_port_to_vid_act = FALSE;
		pFlowPath3->out_drop = 0;
		pFlowPath3->lock = FALSE;
#if defined(CONFIG_FC_RTL9607C_SERIES)
		if(is_checking_flow_can_add==0)
		{
			unsigned char extraFwd,modified;
			
			struct rtl8686_hwnat_customized_entry customized_entry={0};
			
			if((0x1<<pPktHdr->egressPort.macPortIdx)&fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraPmsk)
			{
				extraFwd=1;
				customized_entry.gmac=fc_db_fastFwd_data.vxlan_acc.vxlan_extraGmac;
				customized_entry.rxRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamRxRingNum;
				customized_entry.txRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamTxRingNum;
				modified=fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamRing_modified;
				pFlowPath3->out_user_priority = fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamPriority;
				DEBUG("[VXLAN]Prepare for extra inbound VXLAN flow!!");
			}else{
				extraFwd=0;
				customized_entry.gmac=fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamGmac;
				customized_entry.rxRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamRxRingNum;
				customized_entry.txRingNum=fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamTxRingNum;
				modified=fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamRing_modified;
				pFlowPath3->out_user_priority = fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamPriority;
				DEBUG("[VXLAN]Prepare for inbound VXLAN flow!!");
			}

			if(fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamInfo_prepared && modified==0){
				customized_entry.valid=TRUE;
				//Prevent down stream add flow first, so fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length will not be updated(default updated by upstream)
				if(fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length == 0)
					fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length = 50;
				customized_entry.rxPreLen=-fc_db_fastFwd_data.vxlan_acc.vxlan_preallocated_outer_length;		//do it in rxhook when dynamic_sram_desc==0
				customized_entry.txInfo_addr_offset_v1=fc_db_fastFwd_data.vxlan_acc.vxlan_downstream_txAddrOffset;
				customized_entry.pCustomized_txInfo=(struct tx_info *)&fc_db_fastFwd_data.vxlan_acc.vxlan_downstream_txInfo;
				customized_entry.type=CUSTOMIZE_TYPE_VXLAN_DOWN;
				customized_entry.rx_ext_pmsk=RTK_FC_VXLAN_ACC_DOWNSTREAM_EXTPMAK;
				//DEBUG("Call re8686_customized_rx_and_tx");
				if(re8686_customized_rx_and_tx(customized_entry, NULL, NULL, rtk_fc_vxlan_customized_rxHook, NULL)==0){
					if(extraFwd)
						fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamRing_modified=1;
					else
						fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamRing_modified=1;
				}

			}		
		}
#endif

	}

	return SUCCESS;
}


int _rtk_fc_vxlan_extraVOQ_get(int cpuPortIdx, uint16 *voq)
{
#if defined(CONFIG_FC_CA8277B_SERIES)

	int i, found = 0;
	if(cpuPortIdx == -1 || cpuPortIdx >=4)
		return FAILED;

	for(i = 0 ; i < 2 ; i++)
	{
		if(fc_db.vxlan_extraVOQ_usage[cpuPortIdx][i] == 0)
		{
			found =1;
			fc_db.vxlan_extraVOQ_usage[cpuPortIdx][i] = 1;
			break;
		}
	}

	if(found)
	{
		if(i==0)
		{
			*voq = 5;
		}
		else
		{
			*voq = 6;
		}
		
		return SUCCESS;
	}
	else
		return FAILED;
#else
	return SUCCESS;

#endif
}

int _rtk_fc_vxlan_roundRobin_index_get(int vxlan_direction)
{
#if defined(CONFIG_FC_CA8277B_SERIES)

	int i ;
	if(vxlan_direction == 0 )//DOWNSTREAM
	{
		for(i = 0 ; i < 4; i++)
		{
			if(fc_db.vxlan_downStream_record[i].isSet == 0)
			{
				return i;
			}
		}
		return -1;
		
	}
	else // UPSTREAM
	{
		for(i = 0 ; i < 4; i++)
		{
			if(fc_db.vxlan_upStream_record[i].isSet == 0)
			{
				return i;
			}
		}
		return -1;
	}
#else
	return -1;
#endif	
}
void _rtk_fc_vxlan_8277b_reservedACL_del_by_idx(int index, int direction)
{
#if defined(CONFIG_FC_CA8277B_SERIES)

	if(direction == 1) // UPSTREAM
	{
		TABLE("[VXLAN][UPSTREAM]Reset reserved acl for vxlan_upstream[%d]", index);
		if(fc_db.vxlan_l3cls_upstream_isSet[index] == 1)
		{
			if(fc_db.vxlan_l3cls_info.pktlen_us_68_aclInfo[index] != CA_UINT32_INVALID )
			{
				assert_ok(_rtk_rg_aclAndCfReservedRuleDelSpecial(RTK_CA_CLS_TYPE_VXLAN_FFD_US, &fc_db.vxlan_l3cls_info.pktlen_us_68_aclInfo[index]));
				fc_db.vxlan_l3cls_info.pktlen_us_68_aclInfo[index] = CA_UINT32_INVALID ;
			}
			if(fc_db.vxlan_l3cls_info.pktlen_us_128_aclInfo[index] != CA_UINT32_INVALID )
			{
				assert_ok(_rtk_rg_aclAndCfReservedRuleDelSpecial(RTK_CA_CLS_TYPE_VXLAN_FFD_US, &fc_db.vxlan_l3cls_info.pktlen_us_128_aclInfo[index]));
				fc_db.vxlan_l3cls_info.pktlen_us_128_aclInfo[index] = CA_UINT32_INVALID ;
			}
			fc_db.vxlan_l3cls_upstream_isSet[index] = 0;
			
		}
	}
	else // DOWNSTREAM
	{
		TABLE("[VXLAN][DOWNSTREAM]Reset reserved acl for vxlan_downstream[%d]", index);
		/*Down stream: cls - 68/others*/
		if(fc_db.vxlan_l3cls_downstream_isSet[index] == 1)
		{
			if(fc_db.vxlan_l3cls_info.pktlen_ds_68_aclInfo[index] != CA_UINT32_INVALID )
			{
				assert_ok(_rtk_rg_aclAndCfReservedRuleDelSpecial(RTK_CA_CLS_TYPE_VXLAN_FFD_DS, &fc_db.vxlan_l3cls_info.pktlen_ds_68_aclInfo[index]));
				fc_db.vxlan_l3cls_info.pktlen_ds_68_aclInfo[index] = CA_UINT32_INVALID;
			}
			if(fc_db.vxlan_l3cls_info.pktlen_ds_others_aclInfo[index] != CA_UINT32_INVALID )
			{
				assert_ok(_rtk_rg_aclAndCfReservedRuleDelSpecial(RTK_CA_CLS_TYPE_VXLAN_FFD_DS, &fc_db.vxlan_l3cls_info.pktlen_ds_others_aclInfo[index]));
				fc_db.vxlan_l3cls_info.pktlen_ds_others_aclInfo[index] = CA_UINT32_INVALID;
			}
			fc_db.vxlan_l3cls_downstream_isSet[index] = 0;
			
		}		
	}
#endif	
}
#if defined(CONFIG_FC_CA8277B_SERIES)

void _rtk_fc_vxlan_hwIssue_workAround_setting(void)
{
	
	DMA_SEC_DMA_GLB_DMA_LSO_CTRL_t	  dma_lso_ctrl;
	NI_HV_GLB_NIRX_L3FE_DPQ_DEMUX_CFG0_t        l3fe_dpq_demux_cfg0;
	
	if(fc_db.vxlan_hw_issue_work_around_setting==0)
	{
		// don't delay l2tm schedule time, or l2tm port 10 will tail drop
		//rtk_ne_reg_write(0x000c00ff, L2TM_L2TM_ES_SCH_CFG0 + L2TM_L2TM_ES_SCH_CFG0_STRIDE * 10);	// port 10 - wan to L3FE
		//rtk_ne_reg_write(0x000c00ff, L2TM_L2TM_ES_SCH_CFG0 + L2TM_L2TM_ES_SCH_CFG0_STRIDE * 13);	// port 13 - lan to L3FE
		//rtk_ne_reg_write(0x000c00ff, L2TM_L2TM_ES_SCH_CFG0 + L2TM_L2TM_ES_SCH_CFG0_STRIDE * 8);		// port 8 - L2TM to L3QM
		//aal_port_arb_ldpid_pdpid_map_set(0, 0, 0, 0x19, 0xa);
		
		//Set dma-lso tx_burst_len to 2 (32 * 8 bytes )
		dma_lso_ctrl.wrd = rtk_dma_lso_reg_read(DMA_SEC_DMA_GLB_DMA_LSO_CTRL);
		dma_lso_ctrl.bf.tx_burst_len  = 2;
		rtk_dma_lso_reg_write(dma_lso_ctrl.wrd, DMA_SEC_DMA_GLB_DMA_LSO_CTRL);

		//Up-stream to Pon by-pass QM
		l3fe_dpq_demux_cfg0.wrd = 0xAAAA5500;
		rtk_ne_reg_write(l3fe_dpq_demux_cfg0.wrd, NI_HV_GLB_NIRX_L3FE_DPQ_DEMUX_CFG0);
		
		fc_db.vxlan_hw_issue_work_around_setting=1;
	}

}
#endif
int _rtk_fc_l2_vxlan_downstream_innerFlow_add_process(rtk_fc_pktHdr_t *pPktHdr, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, rtk_fc_ingress_data_t *pFcIngressData, uint32 *pFlowIdx,
												rtk_rg_asic_path1_entry_t *pFlowEntry, struct rt_nfconn *rtct, bool swOnly, rtk_fc_igrExtraInfo_t *pExtraInfo, uint8 staticEntry,
												struct rt_skbuff *rtskb)
{
#if defined(CONFIG_FC_CA8277B_SERIES)
	int inner_path3_flowHashIdx = 0;
	int ret = FC_VXLAN_FLOW_OK;

	inner_path3_flowHashIdx = _rtk_fc_flow_hashIndex_byPktHdr(pPktHdr);
	// 32K hash value point to 64K entry
	inner_path3_flowHashIdx <<= fc_db.flowHashWayShift;

	DEBUG("[L2 VXLAN][DS]Inner hash - %d",inner_path3_flowHashIdx);

	
	pFlowEntry->valid = TRUE;
	

	pG3IgrExtraInfo->vxlan_info.is_vxlanDS_innerFlow = 1;
	
	if(pFcIngressData->ingressTagif & SVLAN_TAGIF)
		pG3IgrExtraInfo->vxlan_info.ingress_inner_stag = 1;
	if(pFcIngressData->ingressTagif & CVLAN_TAGIF)
		pG3IgrExtraInfo->vxlan_info.ingress_inner_ctag = 1;
	if(pFcIngressData->ingressTagif & PPPOE_TAGIF)
		pG3IgrExtraInfo->vxlan_info.ingress_inner_pppoeTag = 1;

	ret = rtk_fc_flow_add(pFlowIdx, inner_path3_flowHashIdx, pFlowEntry, rtct,
						pExtraInfo, pPktHdr->isMulticast, pPktHdr->ingressPort, swOnly, staticEntry, pG3IgrExtraInfo, pPktHdr);
	if(ret!=RTK_FC_RET_OK)
	{
		if(ret==RTK_FC_RET_ERR_FLOW_FULL)
			TRACE("[Fail to add flow] Flow table is full.");
		else
			WARNING("[Fail to add flow] ret=%d", ret);
		
		return FC_VXLAN_FLOW_DOWNSTREAM_INNERFLOW_FAILED;
	}
	
#endif
	return FC_VXLAN_FLOW_OK;
}


int _rtk_fc_vxlan_downstream_innerFlow_add_process(rtk_fc_pktHdr_t *pPktHdr, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, rtk_fc_ingress_data_t *pFcIngressData, uint32 *pFlowIdx,
												rtk_rg_asic_path1_entry_t *pFlowEntry, struct rt_nfconn *rtct, bool swOnly, rtk_fc_igrExtraInfo_t *pExtraInfo, uint8 staticEntry,
												struct rt_skbuff *rtskb)
{
	// For VXLAN Down stream specail Fast forward:
	//   - recalculate inner hash
	// For VXLAN Down stream sw-only shortCut:
	//   - Use outter hash but the flow info is inner.
	int inner_path5_flowHashIdx = 0;
	int ret = FC_VXLAN_FLOW_OK;


	
#if defined(CONFIG_FC_CA8277B_SERIES)

	inner_path5_flowHashIdx = _rtk_fc_flow_hashIndex_byPktHdr(pPktHdr);
	// 32K hash value point to 64K entry
	inner_path5_flowHashIdx <<= fc_db.flowHashWayShift;

	DEBUG("[VXLAN][DS]Inner hash - %d",inner_path5_flowHashIdx);
#elif defined(CONFIG_FC_RTL9607C_SERIES)
	rtk_fc_tableFlowEntry_t tmp_flowPathEntry;
	uint16 igrSVID = 0, igrCVID = 0;
	//uint8 flow_can_add=1;
	//int by_pass_vxlan_add = 0;
	
	memcpy(&tmp_flowPathEntry.path5,pFlowEntry,sizeof(rtk_rg_asic_path5_entry_t));
	
	igrSVID = pFcIngressData->srcSVlanId;
	igrCVID = pFcIngressData->srcCVlanId;

	inner_path5_flowHashIdx = _rtk_fc_flow_hashIndex(tmp_flowPathEntry, igrSVID, igrCVID, FALSE);


	TRACE("[VXLAN][DS]Inner hash - %d",inner_path5_flowHashIdx);
/*
	//Check outer and inner can all be added to hw entry.
	rtk_fc_vxlan_check_flow_canBeAdded(&flow_can_add , RTK_FC_FLOW_DIRECTION_DOWNSTREAM, pFlowEntry, inner_path5_flowHashIdx, pFcIngressData->flowHashIdx, pG3IgrExtraInfo, pPktHdr, rtskb, outer_igrNetifIdx, &by_pass_vxlan_add);

	if(flow_can_add != 1)
	{
		WARNING("Some flow can't be add! swonly!");
		return FC_VXLAN_FLOW_NOT_VXLAN_CANT_BE_ADDED;
	}
	else if(by_pass_vxlan_add==1)
	{
		return FC_VXLAN_FLOW_BYPASS_VXLAN_ADD;
	}
	else
*/
#endif

	{
		pFlowEntry->valid = TRUE;
	}



	pG3IgrExtraInfo->vxlan_info.is_vxlanDS_innerFlow = 1;
	
	if(pFcIngressData->ingressTagif & SVLAN_TAGIF)
		pG3IgrExtraInfo->vxlan_info.ingress_inner_stag = 1;
	if(pFcIngressData->ingressTagif & CVLAN_TAGIF)
		pG3IgrExtraInfo->vxlan_info.ingress_inner_ctag = 1;
	if(pFcIngressData->ingressTagif & PPPOE_TAGIF)
		pG3IgrExtraInfo->vxlan_info.ingress_inner_pppoeTag = 1;

	ret = rtk_fc_flow_add(pFlowIdx, inner_path5_flowHashIdx, pFlowEntry, rtct,
						pExtraInfo, pPktHdr->isMulticast, pPktHdr->ingressPort, swOnly, staticEntry, pG3IgrExtraInfo, pPktHdr);
	if(ret!=RTK_FC_RET_OK)
	{
		if(ret==RTK_FC_RET_ERR_FLOW_FULL)
			TRACE("[Fail to add flow] Flow table is full.");
		else
			WARNING("[Fail to add flow] ret=%d", ret);
		
		return FC_VXLAN_FLOW_DOWNSTREAM_INNERFLOW_FAILED;
	}
	return FC_VXLAN_FLOW_OK;
						

}
int _rtk_fc_l2_vxlan_downstream_outerFlow_add_process( rtk_fc_ingress_data_t *pFcIngressData, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, 
																 uint32 innerFlowIdx, int igr_netifIdx, rtk_fc_igrExtraInfo_t *pExtraInfo, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo,
																 uint32 *outerFlowIdx, int cpuPortIdx)
{
#if defined(CONFIG_FC_CA8277B_SERIES)
	int ret = SUCCESS;
	rtk_fc_g3IgrExtraInfo_t * outer_pG3IgrExtraInfo = NULL;
	rtk_rg_asic_path1_entry_t outerFlowEntry;
	rtk_rg_asic_path3_entry_t *outer_pFlowPath3 = (rtk_rg_asic_path3_entry_t *)&outerFlowEntry;
	uint32 outer_flow_idx = 0;
	
	outer_pG3IgrExtraInfo = RTK_FC_HELPER_FC_KMALLOC(sizeof(rtk_fc_g3IgrExtraInfo_t), GFP_ATOMIC);
	_rtk_fc_g3IgrExtraInfo_init(outer_pG3IgrExtraInfo);
	//prepare info that needed for adding G3 HW entry
	
	outer_pG3IgrExtraInfo->lutIgrSaIdx = pExtraInfo->lutIgrSaIdx;
	outer_pG3IgrExtraInfo->lutEgrDaIdx = pExtraInfo->lutEgrDaIdx;
	DEBUG("hw netif idx:%d, sw netif idx:%d",igr_netifIdx,NETIF_HWTOSW(igr_netifIdx));
	outer_pG3IgrExtraInfo->svlan_id = fc_db.netifTbl[NETIF_HWTOSW(igr_netifIdx)].dualUniInfo.vxlan_netif_info.outer_svlan_id;//pFcIngressData->outer_srcSVlanId;
	outer_pG3IgrExtraInfo->cvlan_id = fc_db.netifTbl[NETIF_HWTOSW(igr_netifIdx)].dualUniInfo.vxlan_netif_info.outer_cvlan_id;//pFcIngressData->outer_srcCVlanId;


	outer_pG3IgrExtraInfo->ipv6_Saddr.s6_addr32[0] = fc_db.netifTbl[NETIF_HWTOSW(igr_netifIdx)].dualUniInfo.vxlan_netif_info.outer_remote_ip;
	outer_pG3IgrExtraInfo->ipv6_Daddr.s6_addr32[0] =fc_db.netifTbl[NETIF_HWTOSW(igr_netifIdx)].dualUniInfo.vxlan_netif_info.outer_ip;
	outer_pG3IgrExtraInfo->pon_stream_id = pG3IgrExtraInfo->pon_stream_id;
	
	_rtk_fc_l2_vxlan_down_stream_outer_flow_prepare(outer_pFlowPath3, rtskb, pPktHdr, innerFlowIdx, igr_netifIdx);
	

	outer_pG3IgrExtraInfo->vxlan_info.is_vxlanDS_outerFlow = 1;
	outer_pG3IgrExtraInfo->vxlan_info.is_l2_vxlan = 1;
	outer_pG3IgrExtraInfo->vxlan_info.cpuPortIdx = cpuPortIdx;
	outer_pG3IgrExtraInfo->dualHdrType		= RTK_FC_DUALTYPE_VXLAN;
	outer_pG3IgrExtraInfo->direction	= RTK_FC_FLOW_DIRECTION_DOWNSTREAM;
	outer_pG3IgrExtraInfo->vxlan_info.vxlan_downstream_inner_flow_idx = innerFlowIdx;
	if(((outerFlowEntry.in_path==FB_PATH_34)||(outerFlowEntry.in_path==FB_PATH_5)) && (outerFlowEntry.in_pppoeif)){
		outer_pG3IgrExtraInfo->pppoe_session_id = pFcIngressData->outer_sessionId;
	}

	if(!fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATHALL_SKIP_SPRI])
		outer_pG3IgrExtraInfo->svlan_pri = fc_db.netifTbl[NETIF_HWTOSW(igr_netifIdx)].dualUniInfo.vxlan_netif_info.outer_stag_pri;//pFcIngressData->outer_srcSVlanPri;

	if(!fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATHALL_SKIP_VLAN_DEICFI])
	{
		outer_pG3IgrExtraInfo->igr_svlan_dei = fc_db.netifTbl[NETIF_HWTOSW(igr_netifIdx)].dualUniInfo.vxlan_netif_info.outer_srcSVlanDei;//pFcIngressData->outer_srcSVlanDei;
		outer_pG3IgrExtraInfo->igr_cvlan_cfi = fc_db.netifTbl[NETIF_HWTOSW(igr_netifIdx)].dualUniInfo.vxlan_netif_info.outer_srcCVlanCfi;//pFcIngressData->outer_srcCVlanCfi;
	}
	outer_pG3IgrExtraInfo->egr_svlan_dei = fc_db.netifTbl[NETIF_HWTOSW(igr_netifIdx)].dualUniInfo.vxlan_netif_info.outer_srcSVlanDei;//pFcIngressData->outer_srcSVlanDei;
	outer_pG3IgrExtraInfo->egr_cvlan_cfi = fc_db.netifTbl[NETIF_HWTOSW(igr_netifIdx)].dualUniInfo.vxlan_netif_info.outer_srcCVlanCfi;//pFcIngressData->outer_srcCVlanCfi;
	outer_pG3IgrExtraInfo->igr_svlan_tpid_sel = fc_db.netifTbl[NETIF_HWTOSW(igr_netifIdx)].dualUniInfo.vxlan_netif_info.outer_srcSVlanTpid_sel;//pFcIngressData->outer_srcSVlanTpid_sel;
	outer_pG3IgrExtraInfo->egr_svlan_tpid_sel = fc_db.netifTbl[NETIF_HWTOSW(igr_netifIdx)].dualUniInfo.vxlan_netif_info.outer_srcSVlanTpid_sel;//pFcIngressData->outer_srcSVlanTpid_sel;

	outer_pG3IgrExtraInfo->vxlan_info.outer_tag_len = pPktHdr->vxlan_info.outerTag_off;
		
	
	ret = rtk_fc_flow_add(&outer_flow_idx, pFcIngressData->flowHashIdx, &outerFlowEntry, NULL,
												pExtraInfo, 0, pPktHdr->ingressPort, 0, 1, outer_pG3IgrExtraInfo, NULL);
	//if(outer_pG3IgrExtraInfo) 
		RTK_FC_HELPER_FC_KFREE(outer_pG3IgrExtraInfo, sizeof(rtk_fc_g3IgrExtraInfo_t));
		
	if(ret!=RTK_FC_RET_OK)
	{
		if(ret==RTK_FC_RET_ERR_FLOW_FULL)
			WARNING("[Fail to add flow] Flow table is full.");
		else
			WARNING("[Fail to add flow] ret=%d", ret);
		return FC_VXLAN_FLOW_DOWNSTREAM_OUTERFLOW_FAILED;
	}
	else
		*outerFlowIdx = outer_flow_idx;

#endif
	return FC_VXLAN_FLOW_OK;

}


int _rtk_fc_vxlan_downstream_outerFlow_add_process( rtk_fc_ingress_data_t *pFcIngressData, struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, 
																 uint32 innerFlowIdx, int igrNetifIdx, int egrNetifIdx, rtk_fc_igrExtraInfo_t *pExtraInfo, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo,
																 uint32 *outerFlowIdx, int cpuPortIdx)
{
	int ret = SUCCESS;
	rtk_fc_g3IgrExtraInfo_t * outer_pG3IgrExtraInfo = NULL;
	rtk_fc_tableFlowEntry_t flowPathEntry;
	rtk_rg_asic_path1_entry_t *outerFlowEntry = &flowPathEntry.path1;
	rtk_rg_asic_path3_entry_t *outer_pFlowPath3 = (rtk_rg_asic_path3_entry_t *)outerFlowEntry;
#if defined(CONFIG_FC_RTL9607C_SERIES)  || defined(CONFIG_FC_CA8277B_SERIES)

	uint32 outer_flow_idx = 0;
	uint32 outer_flow_hash = 0;
#endif 
	
	outer_pG3IgrExtraInfo = RTK_FC_HELPER_FC_KMALLOC(sizeof(rtk_fc_g3IgrExtraInfo_t), GFP_ATOMIC);
	_rtk_fc_g3IgrExtraInfo_init(outer_pG3IgrExtraInfo);
	//prepare info that needed for adding G3 HW entry
	
	outer_pG3IgrExtraInfo->lutIgrSaIdx = pExtraInfo->lutIgrSaIdx;
	outer_pG3IgrExtraInfo->lutEgrDaIdx = pExtraInfo->lutEgrDaIdx;
	DEBUG("hw netif: %d, sw netif:%d", igrNetifIdx,NETIF_HWTOSW(igrNetifIdx));
	outer_pG3IgrExtraInfo->svlan_id = fc_db.netifTbl[NETIF_HWTOSW(igrNetifIdx)].dualUniInfo.vxlan_netif_info.outer_svlan_id;//pFcIngressData->outer_srcSVlanId;
	outer_pG3IgrExtraInfo->cvlan_id = fc_db.netifTbl[NETIF_HWTOSW(igrNetifIdx)].dualUniInfo.vxlan_netif_info.outer_cvlan_id;//pFcIngressData->outer_srcCVlanId;


	outer_pG3IgrExtraInfo->ipv6_Saddr.s6_addr32[0] = fc_db.netifTbl[NETIF_HWTOSW(igrNetifIdx)].dualUniInfo.vxlan_netif_info.outer_remote_ip;
	outer_pG3IgrExtraInfo->ipv6_Daddr.s6_addr32[0] =fc_db.netifTbl[NETIF_HWTOSW(igrNetifIdx)].dualUniInfo.vxlan_netif_info.outer_ip;
	outer_pG3IgrExtraInfo->pon_stream_id = pG3IgrExtraInfo->pon_stream_id;
	
	_rtk_fc_vxlan_down_stream_outer_flow_prepare(outer_pFlowPath3, rtskb, pPktHdr, innerFlowIdx, igrNetifIdx, egrNetifIdx,0);
	
	
	

	outer_pG3IgrExtraInfo->vxlan_info.is_vxlanDS_outerFlow = 1;
#if defined(CONFIG_FC_CA8277B_SERIES)
	outer_pG3IgrExtraInfo->vxlan_info.cpuPortIdx = cpuPortIdx;
#endif
	outer_pG3IgrExtraInfo->dualHdrType		= RTK_FC_DUALTYPE_VXLAN;
	outer_pG3IgrExtraInfo->direction		= RTK_FC_FLOW_DIRECTION_DOWNSTREAM;
	outer_pG3IgrExtraInfo->vxlan_info.vxlan_downstream_inner_flow_idx = innerFlowIdx;
	if(((outerFlowEntry->in_path==FB_PATH_34)||(outerFlowEntry->in_path==FB_PATH_5)) && (outerFlowEntry->in_pppoeif)){
		outer_pG3IgrExtraInfo->pppoe_session_id = pFcIngressData->outer_sessionId;
	}

	if(!fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATHALL_SKIP_SPRI])
		outer_pG3IgrExtraInfo->svlan_pri =  fc_db.netifTbl[NETIF_HWTOSW(igrNetifIdx)].dualUniInfo.vxlan_netif_info.outer_stag_pri;//pFcIngressData->outer_srcSVlanPri;

	if(!fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATHALL_SKIP_VLAN_DEICFI])
	{
		outer_pG3IgrExtraInfo->igr_svlan_dei = fc_db.netifTbl[NETIF_HWTOSW(igrNetifIdx)].dualUniInfo.vxlan_netif_info.outer_srcSVlanDei;//pFcIngressData->outer_srcSVlanDei;
		outer_pG3IgrExtraInfo->igr_cvlan_cfi = fc_db.netifTbl[NETIF_HWTOSW(igrNetifIdx)].dualUniInfo.vxlan_netif_info.outer_srcCVlanCfi;// pFcIngressData->outer_srcCVlanCfi;
	}
	outer_pG3IgrExtraInfo->egr_svlan_dei = fc_db.netifTbl[NETIF_HWTOSW(igrNetifIdx)].dualUniInfo.vxlan_netif_info.outer_srcSVlanDei;//pFcIngressData->outer_srcSVlanDei;
	outer_pG3IgrExtraInfo->egr_cvlan_cfi = fc_db.netifTbl[NETIF_HWTOSW(igrNetifIdx)].dualUniInfo.vxlan_netif_info.outer_srcCVlanCfi;//pFcIngressData->outer_srcCVlanCfi;
	outer_pG3IgrExtraInfo->igr_svlan_tpid_sel = fc_db.netifTbl[NETIF_HWTOSW(igrNetifIdx)].dualUniInfo.vxlan_netif_info.outer_srcSVlanTpid_sel;//pFcIngressData->outer_srcSVlanTpid_sel;
	outer_pG3IgrExtraInfo->egr_svlan_tpid_sel = fc_db.netifTbl[NETIF_HWTOSW(igrNetifIdx)].dualUniInfo.vxlan_netif_info.outer_srcSVlanTpid_sel;//pFcIngressData->outer_srcSVlanTpid_sel;

	DEBUG("Outer flow pattern svlan: %d, cvlan_id: %d",fc_db.netifTbl[NETIF_HWTOSW(igrNetifIdx)].dualUniInfo.vxlan_netif_info.outer_svlan_id, fc_db.netifTbl[NETIF_HWTOSW(igrNetifIdx)].dualUniInfo.vxlan_netif_info.outer_cvlan_id);
	outer_flow_hash = _rtk_fc_flow_hashIndex(flowPathEntry, fc_db.netifTbl[NETIF_HWTOSW(igrNetifIdx)].dualUniInfo.vxlan_netif_info.outer_svlan_id, 
															  fc_db.netifTbl[NETIF_HWTOSW(igrNetifIdx)].dualUniInfo.vxlan_netif_info.outer_cvlan_id, FALSE);

#if defined(CONFIG_FC_RTL9607C_SERIES) 
	DEBUG("outer_flow_hash: %d, innerFlowIdx = %d",outer_flow_hash, innerFlowIdx);
	ret = rtk_fc_flow_add(&outer_flow_idx, outer_flow_hash, outerFlowEntry, NULL,
											NULL, 0, pPktHdr->ingressPort, 0, 0, outer_pG3IgrExtraInfo, NULL);

#elif defined(CONFIG_FC_CA8277B_SERIES)
	outer_pG3IgrExtraInfo->vxlan_info.outer_tag_len	=pPktHdr->vxlan_info.outerTag_off;
	

	ret = rtk_fc_flow_add(&outer_flow_idx, outer_flow_hash, outerFlowEntry, NULL,
											pExtraInfo, 0, pPktHdr->ingressPort, 0, 0, outer_pG3IgrExtraInfo, NULL);
#endif	
	//if(outer_pG3IgrExtraInfo) 
		RTK_FC_HELPER_FC_KFREE(outer_pG3IgrExtraInfo, sizeof(rtk_fc_g3IgrExtraInfo_t));
	
	if(ret!=RTK_FC_RET_OK)
	{
		if(ret==RTK_FC_RET_ERR_FLOW_FULL)
			WARNING("[Fail to add flow] Flow table is full.");
		else
			WARNING("[Fail to add flow] ret=%d", ret);
		return FC_VXLAN_FLOW_DOWNSTREAM_OUTERFLOW_FAILED;
	}
#if defined(CONFIG_FC_RTL9607C_SERIES)  || defined(CONFIG_FC_CA8277B_SERIES)
	else
		*outerFlowIdx = outer_flow_idx;
#endif

	
	return FC_VXLAN_FLOW_OK;
}
int _rtk_fc_L2_vxlan_g3IgrExtraInfo_setting(int vxlan_direction, rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr,
												   rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, int egrNetifIdx, rtk_rg_asic_path1_entry_t *pFlowEntry,
												   struct rt_skbuff *rtskb, int cpuPortIdx)
{
#if defined(CONFIG_FC_CA8277B_SERIES)

	if(vxlan_direction ==0 ) // DOWNSTREAM
	{
		pG3IgrExtraInfo->vxlan_info.useExtra = 1;
		pG3IgrExtraInfo->vxlan_info.is_l2_vxlan =1;
	}
	else if( vxlan_direction == 1)  // UPSTREAM
	{
		uint16 extraVOQ = 0;
		
		pG3IgrExtraInfo->vxlan_info.useExtra = 1;
		pG3IgrExtraInfo->vxlan_info.is_vxlanUS_innerFlow =1;
		pG3IgrExtraInfo->vxlan_info.is_l2_vxlan =1;

		{
			
			DEBUG("[VXLAN][US]Use Extra!");
			if(_rtk_fc_vxlan_extraVOQ_get(cpuPortIdx, &extraVOQ) != SUCCESS)
			{
				return FC_VXLAN_FLOW_EXTRA_GETVOQ_FAIL;
			}
			pG3IgrExtraInfo->vxlan_info.extraVOQ = extraVOQ;
		}
	}
#endif
	return FC_VXLAN_FLOW_OK;
}

int _rtk_fc_L3_vxlan_g3IgrExtraInfo_setting(int vxlan_direction, rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr,
												   rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, int egrNetifIdx, rtk_rg_asic_path1_entry_t *pFlowEntry,
												   struct rt_skbuff *rtskb, int useExtra, int cpuPortIdx)
{

	if(vxlan_direction ==0 ) // DOWNSTREAM
	{ 
		//VXLAN downstream
#if defined(CONFIG_FC_RTL9607C_SERIES)

		memcpy(&pG3IgrExtraInfo->vxlan_info.vxlan_DA[0], &pPktHdr->vxlan_info.vxlan_DA[0], 6);
#elif defined(CONFIG_FC_CA8277B_SERIES)
		if(useExtra)
		{
			DEBUG("[VXLAN][DS]Use Extra!");
			pG3IgrExtraInfo->vxlan_info.useExtra = 1;
		}
		pG3IgrExtraInfo->vxlan_info.innerExtra_len = pPktHdr->vxlan_info.inner_extra_len;
#endif

	}
	else if( vxlan_direction == 1)  // UPSTREAM
	{ 
		// VXLAN UPSTREAM
				
#if defined(CONFIG_FC_RTL9607C_SERIES)
		memcpy(&pG3IgrExtraInfo->vxlan_info.vxlan_DA[0], &pPktHdr->eth->h_dest[0], 6);

#elif defined(CONFIG_FC_CA8277B_SERIES)
		if(useExtra)
		{
			uint16 extraVOQ = 0;
			DEBUG("[VXLAN][US]Use Extra!");
			pG3IgrExtraInfo->vxlan_info.useExtra = 1;
			if(_rtk_fc_vxlan_extraVOQ_get(cpuPortIdx, &extraVOQ) != SUCCESS)
			{
				return FC_VXLAN_FLOW_EXTRA_GETVOQ_FAIL;
			}
			pG3IgrExtraInfo->vxlan_info.extraVOQ = extraVOQ;
		}

#endif

		pG3IgrExtraInfo->vxlan_info.is_vxlanUS_innerFlow =1;
		
	}
	return FC_VXLAN_FLOW_OK;
	
}
int _rtk_fc_l2_vxlan_upstream_innerFlow_add_process(rtk_fc_ingress_data_t *pFcIngressData, rtk_rg_asic_path1_entry_t *pFlowEntry, struct rt_nfconn *rtct, rtk_fc_igrExtraInfo_t *pExtraInfo,
															 rtk_fc_pktHdr_t *pPktHdr, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, uint32 swOnly_flowIdx, uint32 *pFlowIdx, int cpuPortIdx)
{	
#if defined(CONFIG_FC_CA8277B_SERIES)
	int ret;
	rtk_rg_asic_path3_entry_t *pFlowPath3 = (rtk_rg_asic_path3_entry_t *)pFlowEntry;
	uint32 flowIdx = 0;

	pFlowPath3->valid = TRUE;
	
	pG3IgrExtraInfo->vxlan_info.cpuPortIdx = cpuPortIdx;
	if( pFlowPath3->out_cvid_format_act && pPktHdr->cvh)  // Inner = 1
	{
		pFlowPath3->out_cvlan_id =	pPktHdr->cvlanid;
		pFlowPath3->out_cpri = pPktHdr->cvlanpri;
	}
	else if( pFlowPath3->out_cvid_format_act && !pPktHdr->cvh) // Inner = 0, outer = 1
	{
		pFlowPath3->out_cvid_format_act = FALSE;
		pFlowPath3->out_cpri_format_act = FALSE;
	}

	
	if( pFlowPath3->out_svid_format_act && pPktHdr->svh)  // Inner = 1
	{
		pFlowPath3->out_svlan_id =	pPktHdr->svlanid;
		pFlowPath3->out_spri = pPktHdr->svlanpri;
	}
	else if( pFlowPath3->out_svid_format_act && !pPktHdr->svh) // Inner = 0, outer = 1
	{
		pFlowPath3->out_svid_format_act = FALSE;
		pFlowPath3->out_spri_format_act = FALSE;
	}
	
	//Original flow add
	ret = rtk_fc_flow_add(&flowIdx, pFcIngressData->flowHashIdx, pFlowEntry, rtct,
						 pExtraInfo, pPktHdr->isMulticast, pPktHdr->ingressPort, 0, 0, pG3IgrExtraInfo, pPktHdr);

	if(ret!=RTK_FC_RET_OK)
	{
		if(ret==RTK_FC_RET_ERR_FLOW_FULL)
			TRACE("[Fail to add flow] Flow table is full.");
		else
			WARNING("[Fail to add flow] flowent idx=%d, ret=%d", pFcIngressData->flowHashIdx, ret);
		return FC_VXLAN_FLOW_UPSTREAM_INNER_ADD_FLOW_FAILED;
	}
	else
	{
		DEBUG("VXLAN upstream inner flow index: %d",flowIdx);
	}
	*pFlowIdx = flowIdx;

#endif
	return FC_VXLAN_FLOW_OK;
}

int _rtk_fc_vxlan_upstream_innerFlow_add_process(rtk_fc_ingress_data_t *pFcIngressData, rtk_rg_asic_path1_entry_t *pFlowEntry, struct rt_nfconn *rtct, rtk_fc_igrExtraInfo_t *pExtraInfo,
															 rtk_fc_pktHdr_t *pPktHdr, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, uint32 swOnly_flowIdx, uint32 *pFlowIdx, int cpuPortIdx, struct rt_skbuff *rtskb)
{
	int ret = SUCCESS;
	uint32 flowIdx = 0;

	
#if defined(CONFIG_FC_RTL9607C_SERIES)
	rtk_rg_asic_path5_entry_t *pFlowPath5 = (rtk_rg_asic_path5_entry_t *)pFlowEntry;

	int ret_val = 0;
	ret_val = _rtk_fc_vxlan_up_stream_innerFlow_prepare(pG3IgrExtraInfo, pPktHdr, pFlowPath5, rtskb);

	pFlowPath5->valid = TRUE;
	
	if(fc_db.vxlan_upStream_record[fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->spa].isSet ==1)
	{
		DEBUG("[VXLAN][FastFwd]Upstream spa:%d exceed supported number!",fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->spa);	
		return FC_VXLAN_FLOW_UPSTREAM_INNER_FLOW_EXCEED_PORT_LEARNING_LIMIT;
	}
	
#elif defined(CONFIG_FC_CA8277B_SERIES)
	rtk_rg_asic_path5_entry_t *pFlowPath5 = (rtk_rg_asic_path5_entry_t *)pFlowEntry;

	pFlowPath5->valid = TRUE;
/*
	if(fc_db.vxlan_upStream_record[fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->spa].isSet ==1 && 
		pFcIngressData->flowHashIdx == fc_db.vxlan_upStream_record[fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->spa].inner_flow_index )
	{
		DEBUG("[VXLAN][FastFwd]Flow exist!");	
	}
	else if(fc_db.vxlan_upStream_record[fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->spa].isSet ==1 && 
		pFcIngressData->flowHashIdx != fc_db.vxlan_upStream_record[fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->spa].inner_flow_index)
	{
		DEBUG("[VXLAN][FastFwd][mode 1]Upstream spa:%d exceed supported number!",fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->spa);	
		return FC_VXLAN_FLOW_UPSTREAM_INNER_FLOW_EXCEED_PORT_LEARNING_LIMIT;
	}
*/
	
	pG3IgrExtraInfo->vxlan_info.cpuPortIdx = cpuPortIdx;
	pG3IgrExtraInfo->vxlan_info.outer_srcPort = ntohs(pPktHdr->outer_udph->source);

#endif

#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL9607C_SERIES)
	if( pFlowPath5->out_cvid_format_act && pPktHdr->cvh)  // Inner = 1
	{
		pFlowPath5->out_cvlan_id =	pPktHdr->cvlanid;
		pFlowPath5->out_cpri = pPktHdr->cvlanpri;
	}
	else if( pFlowPath5->out_cvid_format_act && !pPktHdr->cvh) // Inner = 0, outer = 1
	{
		pFlowPath5->out_cvid_format_act = FALSE;
		pFlowPath5->out_cpri_format_act = FALSE;
	}

	
	if( pFlowPath5->out_svid_format_act && pPktHdr->svh)  // Inner = 1
	{
		pFlowPath5->out_svlan_id =	pPktHdr->svlanid;
		pFlowPath5->out_spri = pPktHdr->svlanpri;
	}
	else if( pFlowPath5->out_svid_format_act && !pPktHdr->svh) // Inner = 0, outer = 1
	{
		pFlowPath5->out_svid_format_act = FALSE;
		pFlowPath5->out_spri_format_act = FALSE;
	}
#endif

	
	//Original flow add
	ret = rtk_fc_flow_add(&flowIdx, pFcIngressData->flowHashIdx, pFlowEntry, rtct,
						 pExtraInfo, pPktHdr->isMulticast, pPktHdr->ingressPort, 0, 0, pG3IgrExtraInfo, pPktHdr);

	if(ret!=RTK_FC_RET_OK)
	{
		if(ret==RTK_FC_RET_ERR_FLOW_FULL)
			TRACE("[Fail to add flow] Flow table is full.");
		else
			WARNING("[Fail to add flow] flowent idx=%d, ret=%d", pFcIngressData->flowHashIdx, ret);
		return FC_VXLAN_FLOW_UPSTREAM_INNER_ADD_FLOW_FAILED;
	}
	else
	{
#if defined(CONFIG_FC_CA8277B_SERIES)
		
		_rtk_fc_vxlan_hwIssue_workAround_setting();
#endif
		DEBUG("VXLAN upstream inner flow index: %d",flowIdx);
	}
	*pFlowIdx = flowIdx;
	return FC_VXLAN_FLOW_OK;
}
int _rtk_fc_l2_vxlan_upstream_outerFlow_add_process(rtk_fc_pktHdr_t *pPktHdr, int16 lutIgrSaIdx, int16  outer_daIdx, int isIpv6, rtk_fc_ingress_data_t *pFcIngressData, rtk_rg_asic_path1_entry_t *pFlowEntry,
												  int igr_netifIdx, struct rt_skbuff *rtskb, uint32 flowIdx, uint32 *outerFlowIdx)
{
#if defined(CONFIG_FC_CA8277B_SERIES)
	uint32 flow_hash_index;
	rtk_fc_pktHdr_t *tmp_outer_pktHdr;
	rtk_fc_g3IgrExtraInfo_t * outer_pG3IgrExtraInfo = NULL;
	rtk_rg_asic_path1_entry_t outerFlowEntry;
	uint32 outer_flow_idx = 0;
	int ret= FC_VXLAN_FLOW_OK;

	
	tmp_outer_pktHdr = RTK_FC_HELPER_FC_KMALLOC(sizeof(rtk_fc_pktHdr_t), GFP_ATOMIC);
	outer_pG3IgrExtraInfo = RTK_FC_HELPER_FC_KMALLOC(sizeof(rtk_fc_g3IgrExtraInfo_t), GFP_ATOMIC);
	_rtk_fc_g3IgrExtraInfo_init(outer_pG3IgrExtraInfo);

	outer_pG3IgrExtraInfo->lutIgrSaIdx = lutIgrSaIdx;
	outer_pG3IgrExtraInfo->lutEgrDaIdx = outer_daIdx;
	
	if(pPktHdr->outer_svh)
		outer_pG3IgrExtraInfo->svlan_id = ntohs(pPktHdr->outer_svh->h_vlan_TCI)&VLAN_VID_MASK;// pFcIngressData->outer_srcSVlanId;
	if(pPktHdr->outer_cvh)
		outer_pG3IgrExtraInfo->cvlan_id = ntohs(pPktHdr->outer_cvh->h_vlan_TCI)&VLAN_VID_MASK;//pFcIngressData->outer_srcCVlanId;

	//prepare info that needed for adding G3 HW entry
	if(!isIpv6 && ((pFlowEntry->in_path==FB_PATH_34)||(pFlowEntry->in_path==FB_PATH_5)))
	{
		outer_pG3IgrExtraInfo->ipv6_Saddr.s6_addr32[0] = ntohl(pPktHdr->outer_iph->saddr);
		outer_pG3IgrExtraInfo->ipv6_Daddr.s6_addr32[0] = ntohl(pPktHdr->outer_iph->daddr);
	}

	
	_rtk_fc_l2_vxlan_up_stream_outer_flow_prepare(&outerFlowEntry, rtskb, pPktHdr, igr_netifIdx);

	outer_pG3IgrExtraInfo->vxlan_info.is_vxlanUS_outerFlow = 1;
	//outer_pG3IgrExtraInfo->vxlan_info.l2DualTableIndex = fc_db.flowTbl[flowIdx].l2Dual_table_info.l2Dual_table_index;
	outer_pG3IgrExtraInfo->vxlan_info.is_l2_vxlan = 1;
	outer_pG3IgrExtraInfo->dualHdrType			= RTK_FC_DUALTYPE_VXLAN;
	outer_pG3IgrExtraInfo->direction		= RTK_FC_FLOW_DIRECTION_UPSTREAM;
	outer_pG3IgrExtraInfo->vxlan_info.vxlan_upstream_inner_flow_idx = flowIdx;
	//DEBUG("Try to add VXLAN OUTER FLOW!");
	if(((outerFlowEntry.in_path==FB_PATH_34)||(outerFlowEntry.in_path==FB_PATH_5)) && (outerFlowEntry.in_pppoeif)){
		outer_pG3IgrExtraInfo->pppoe_session_id = (pPktHdr->outer_ppph)?(ntohs(pPktHdr->outer_ppph->sid)):0;
	}

	if(!fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATHALL_SKIP_SPRI])
		outer_pG3IgrExtraInfo->svlan_pri = pPktHdr->outer_svh?((ntohs(pPktHdr->outer_svh->h_vlan_TCI)&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT):0;

	if(!fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATHALL_SKIP_VLAN_DEICFI])
	{
		outer_pG3IgrExtraInfo->igr_svlan_dei = pPktHdr->outer_svh?((ntohs(pPktHdr->outer_svh->h_vlan_TCI)&VLAN_CFI_MASK)>>VLAN_CFI_SHIFT):0;
		outer_pG3IgrExtraInfo->igr_cvlan_cfi = pPktHdr->outer_cvh?((ntohs(pPktHdr->outer_cvh->h_vlan_TCI)&VLAN_CFI_MASK)>>VLAN_CFI_SHIFT):0;
	}
	outer_pG3IgrExtraInfo->egr_svlan_dei = pPktHdr->outer_svh?((ntohs(pPktHdr->outer_svh->h_vlan_TCI)&VLAN_CFI_MASK)>>VLAN_CFI_SHIFT):0;
	outer_pG3IgrExtraInfo->egr_cvlan_cfi = pPktHdr->outer_cvh?((ntohs(pPktHdr->outer_cvh->h_vlan_TCI)&VLAN_CFI_MASK)>>VLAN_CFI_SHIFT):0;
	outer_pG3IgrExtraInfo->igr_svlan_tpid_sel = pPktHdr->outer_stpid_sel;
	outer_pG3IgrExtraInfo->egr_svlan_tpid_sel = pPktHdr->outer_stpid_sel;

	memcpy(tmp_outer_pktHdr, pPktHdr, sizeof(rtk_fc_pktHdr_t));
	
	tmp_outer_pktHdr->eth = tmp_outer_pktHdr->outer_eth;
	tmp_outer_pktHdr->svh = tmp_outer_pktHdr->outer_svh;
	tmp_outer_pktHdr->cvh = tmp_outer_pktHdr->outer_cvh;
	tmp_outer_pktHdr->ppph = tmp_outer_pktHdr->outer_ppph;
	tmp_outer_pktHdr->iph = tmp_outer_pktHdr->outer_iph;
	tmp_outer_pktHdr->udph = tmp_outer_pktHdr->outer_udph;

	flow_hash_index = _rtk_fc_flow_hashIndex_byPktHdr(tmp_outer_pktHdr);
	TRACE("[VXLAN][US]Outer hash - %d",flow_hash_index);

	flow_hash_index <<= fc_db.flowHashWayShift;

	
	ret = rtk_fc_flow_add(&outer_flow_idx, flow_hash_index, &outerFlowEntry, NULL,
											NULL, 0, pPktHdr->ingressPort, 0, 1, outer_pG3IgrExtraInfo, NULL);

	
	//if(outer_pG3IgrExtraInfo) 
		RTK_FC_HELPER_FC_KFREE(outer_pG3IgrExtraInfo, sizeof(rtk_fc_g3IgrExtraInfo_t));
	//if(tmp_outer_pktHdr) 
		RTK_FC_HELPER_FC_KFREE(tmp_outer_pktHdr, sizeof(rtk_fc_pktHdr_t));
	
	if(ret!=RTK_FC_RET_OK)
	{
		if(ret==RTK_FC_RET_ERR_FLOW_FULL)
			TRACE("[Fail to add flow] Flow table is full.");
		else
			WARNING("[Fail to add flow] ret=%d", ret);
		return  FC_VXLAN_FLOW_UPSTREAM_OUTER_ADD_FLOW_FAILED;
	}
	else
		*outerFlowIdx = outer_flow_idx;
									
#endif

	
	return FC_VXLAN_FLOW_OK;
}

int _rtk_fc_vxlan_upstream_outerFlow_add_process(rtk_fc_pktHdr_t *pPktHdr, int16 lutIgrSaIdx, int16  outer_daIdx, int isIpv6, rtk_fc_ingress_data_t *pFcIngressData, rtk_rg_asic_path1_entry_t *pFlowEntry,
												  int igr_netifIdx, struct rt_skbuff *rtskb, uint32 flowIdx, uint32 *outerFlowIdx)
{
	uint32 flow_hash_index;
	rtk_fc_pktHdr_t *tmp_outer_pktHdr;
	rtk_fc_g3IgrExtraInfo_t * outer_pG3IgrExtraInfo = NULL;
	rtk_rg_asic_path1_entry_t outerFlowEntry = {0};
	//rtk_rg_asic_path5_entry_t *outer_pFlowPath5 = (rtk_rg_asic_path5_entry_t *)&outerFlowEntry;
#if defined(CONFIG_FC_RTL9607C_SERIES)  || defined(CONFIG_FC_CA8277B_SERIES)
	uint32 outer_flow_idx = 0;
	int ret= FC_VXLAN_FLOW_OK;
#endif

	rtk_fc_igrExtraInfo_t extraInfo = {0};
		
	DEBUG("Try to add VXLAN OUTER FLOW!");
	tmp_outer_pktHdr = RTK_FC_HELPER_FC_KMALLOC(sizeof(rtk_fc_pktHdr_t), GFP_ATOMIC);
	outer_pG3IgrExtraInfo = RTK_FC_HELPER_FC_KMALLOC(sizeof(rtk_fc_g3IgrExtraInfo_t), GFP_ATOMIC);
	_rtk_fc_g3IgrExtraInfo_init(outer_pG3IgrExtraInfo);
	//prepare info that needed for adding G3 HW entry
	
	outer_pG3IgrExtraInfo->lutIgrSaIdx = lutIgrSaIdx;
	outer_pG3IgrExtraInfo->lutEgrDaIdx = outer_daIdx;

	

	extraInfo.lutIgrSaIdx = lutIgrSaIdx;
	extraInfo.lutEgrDaIdx = outer_daIdx;	
	
	if(pPktHdr->outer_svh)
		outer_pG3IgrExtraInfo->svlan_id = ntohs(pPktHdr->outer_svh->h_vlan_TCI)&VLAN_VID_MASK;// pFcIngressData->outer_srcSVlanId;
	if(pPktHdr->outer_cvh)
		outer_pG3IgrExtraInfo->cvlan_id = ntohs(pPktHdr->outer_cvh->h_vlan_TCI)&VLAN_VID_MASK;//pFcIngressData->outer_srcCVlanId;
	
	//prepare info that needed for adding G3 HW entry
	if(!isIpv6 && ((pFlowEntry->in_path==FB_PATH_34)||(pFlowEntry->in_path==FB_PATH_5)))
	{
		outer_pG3IgrExtraInfo->ipv6_Saddr.s6_addr32[0] = ntohl(pPktHdr->outer_iph->saddr);
		outer_pG3IgrExtraInfo->ipv6_Daddr.s6_addr32[0] = ntohl(pPktHdr->outer_iph->daddr);
	}


	//_rtk_fc_vxlan_up_stream_outer_flow_prepare(outer_pFlowPath5, rtskb, pPktHdr, flowIdx);
	_rtk_fc_vxlan_up_stream_outer_flow_prepare(&outerFlowEntry, rtskb, pPktHdr, igr_netifIdx);

	outer_pG3IgrExtraInfo->vxlan_info.is_vxlanUS_outerFlow = 1;
	//outer_pG3IgrExtraInfo->vxlan_info.l2DualTableIndex = fc_db.flowTbl[flowIdx].l2Dual_table_info.l2Dual_table_index;
	outer_pG3IgrExtraInfo->dualHdrType			= RTK_FC_DUALTYPE_VXLAN;
	outer_pG3IgrExtraInfo->direction			= RTK_FC_FLOW_DIRECTION_UPSTREAM;
	outer_pG3IgrExtraInfo->vxlan_info.vxlan_upstream_inner_flow_idx = flowIdx;
	//DEBUG("Try to add VXLAN OUTER FLOW!");
	if(((outerFlowEntry.in_path==FB_PATH_34)||(outerFlowEntry.in_path==FB_PATH_5)) && (outerFlowEntry.in_pppoeif)){
		outer_pG3IgrExtraInfo->pppoe_session_id = (pPktHdr->outer_ppph)?(ntohs(pPktHdr->outer_ppph->sid)):0;
	}

	if(!fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATHALL_SKIP_SPRI])
		outer_pG3IgrExtraInfo->svlan_pri = pPktHdr->outer_svh?((ntohs(pPktHdr->outer_svh->h_vlan_TCI)&VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT):0;

	if(!fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATHALL_SKIP_VLAN_DEICFI])
	{
		outer_pG3IgrExtraInfo->igr_svlan_dei = pPktHdr->outer_svh?((ntohs(pPktHdr->outer_svh->h_vlan_TCI)&VLAN_CFI_MASK)>>VLAN_CFI_SHIFT):0;
		outer_pG3IgrExtraInfo->igr_cvlan_cfi = pPktHdr->outer_cvh?((ntohs(pPktHdr->outer_cvh->h_vlan_TCI)&VLAN_CFI_MASK)>>VLAN_CFI_SHIFT):0;
	}
	outer_pG3IgrExtraInfo->egr_svlan_dei = pPktHdr->outer_svh?((ntohs(pPktHdr->outer_svh->h_vlan_TCI)&VLAN_CFI_MASK)>>VLAN_CFI_SHIFT):0;
	outer_pG3IgrExtraInfo->egr_cvlan_cfi = pPktHdr->outer_cvh?((ntohs(pPktHdr->outer_cvh->h_vlan_TCI)&VLAN_CFI_MASK)>>VLAN_CFI_SHIFT):0;
	outer_pG3IgrExtraInfo->igr_svlan_tpid_sel = pPktHdr->outer_stpid_sel;
	outer_pG3IgrExtraInfo->egr_svlan_tpid_sel = pPktHdr->outer_stpid_sel;

	memcpy(tmp_outer_pktHdr, pPktHdr, sizeof(rtk_fc_pktHdr_t));
	
	tmp_outer_pktHdr->eth = tmp_outer_pktHdr->outer_eth;
	tmp_outer_pktHdr->svh = tmp_outer_pktHdr->outer_svh;
	tmp_outer_pktHdr->cvh = tmp_outer_pktHdr->outer_cvh;
	tmp_outer_pktHdr->ppph = tmp_outer_pktHdr->outer_ppph;
	tmp_outer_pktHdr->iph = tmp_outer_pktHdr->outer_iph;
	tmp_outer_pktHdr->udph = tmp_outer_pktHdr->outer_udph;
	tmp_outer_pktHdr->tcph = NULL;
	
#if defined(CONFIG_FC_RTL9607C_SERIES)
	if((0x1<<pPktHdr->ingressPort.macPortIdx)&fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraPmsk)
		tmp_outer_pktHdr->udph->source = htons(RTK_FC_VXLAN_ACC_EPHEMERAL_SPORT);

	tmp_outer_pktHdr->svh = NULL;
	tmp_outer_pktHdr->cvh = NULL;
	tmp_outer_pktHdr->ppph = NULL;
	tmp_outer_pktHdr->dmacToGatewayMAC = 0;
	DEBUG("sip: %x, dip: %x sport: %d, dport: %d", ntohl(tmp_outer_pktHdr->iph->saddr),ntohl(tmp_outer_pktHdr->iph->daddr), ntohs(tmp_outer_pktHdr->udph->source), ntohs(tmp_outer_pktHdr->udph->dest));
	
#endif			
	
	flow_hash_index = _rtk_fc_flow_hashIndex_byPktHdr(tmp_outer_pktHdr);

	TRACE("[VXLAN][US]Outer hash - %d",flow_hash_index);
#if defined(CONFIG_FC_CA8277B_SERIES)

	// 32K hash value point to 64K entry
	flow_hash_index <<= fc_db.flowHashWayShift;

	
	ret = rtk_fc_flow_add(&outer_flow_idx, flow_hash_index, &outerFlowEntry, NULL,
											&extraInfo, 0, pPktHdr->ingressPort, 0, 0, outer_pG3IgrExtraInfo, NULL);

#elif defined(CONFIG_FC_RTL9607C_SERIES)
	ret = rtk_fc_flow_add(&outer_flow_idx, flow_hash_index, &outerFlowEntry, NULL,
											&extraInfo, 0, pPktHdr->ingressPort, 0, 0, outer_pG3IgrExtraInfo, NULL);
#endif

	//if(outer_pG3IgrExtraInfo) 
		RTK_FC_HELPER_FC_KFREE(outer_pG3IgrExtraInfo, sizeof(rtk_fc_g3IgrExtraInfo_t));
	//if(tmp_outer_pktHdr) 
		RTK_FC_HELPER_FC_KFREE(tmp_outer_pktHdr, sizeof(rtk_fc_pktHdr_t));

#if defined(CONFIG_FC_RTL9607C_SERIES)  || defined(CONFIG_FC_CA8277B_SERIES)
	if(ret!=RTK_FC_RET_OK)
	{
		if(ret==RTK_FC_RET_ERR_FLOW_FULL)
			TRACE("[Fail to add flow] Flow table is full.");
		else
			WARNING("[Fail to add flow] ret=%d", ret);
		return  FC_VXLAN_FLOW_UPSTREAM_OUTER_ADD_FLOW_FAILED;
	}
	else
		*outerFlowIdx = outer_flow_idx;
#endif
	
	return FC_VXLAN_FLOW_OK;
	
}
int _rtk_fc_L2_vxlan_preChecking(int vxlan_direction, rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr,
 												   rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, int egrNetifIdx, rtk_rg_asic_path1_entry_t *pFlowEntry,
												   struct rt_skbuff *rtskb, int *cpuPortIdx)
{
#if defined(CONFIG_FC_CA8277B_SERIES)
	if(fc_db.controlFuc.special_fast_forward_mode==1)
	{

	
		if(vxlan_direction ==0 )//DOWNSTREAM
		{
			
			if(pFcIngressData->vxlan_info.outer_isV6)
			{
					return FC_VXLAN_FLOW_NOT_SUPPORT_OUTERV6;
			}
			
			if(fc_db.vxlan_ds_extra_fastForward_num >= 8)
			{
				TRACE("[VXLAN][DS]HW offload number is full, sw-only");
				return FC_VXLAN_FLOW_DOWNSTREAM_FULL;
			}
			*cpuPortIdx = fc_db.vxlan_ds_extra_RoundRobin_cpuPort_index;
						
		}
		else // UPSTREAM
		{
			
			if(pPktHdr->outer_ip6h)
			{
				return FC_VXLAN_FLOW_NOT_SUPPORT_OUTERV6;
			}
			if(fc_db.vxlan_us_fastForward_num >= 8)
			{
				TRACE("[VXLAN][US]HW offload number is full, sw-only");
				return FC_VXLAN_FLOW_UPSTREAM_FULL;
			}

			*cpuPortIdx = fc_db.vxlan_us_extra_RoundRobin_cpuPort_index;
		}
	}
#endif
	return FC_VXLAN_FLOW_OK;
}

int _rtk_fc_L3_vxlan_preChecking(int vxlan_direction, rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr,
 												   rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, int egrNetifIdx, rtk_rg_asic_path1_entry_t *pFlowEntry,
												   struct rt_skbuff *rtskb, int *cpuPortIdx, int *useExtra)

{
	if(vxlan_direction ==0 )//DOWNSTREAM
	{ 
		//VXLAN downstream
		
		if(fc_db.controlFuc.special_fast_forward_mode==1)
		{
			
			if(pFcIngressData->vxlan_info.outer_isV6)
			{
					return FC_VXLAN_FLOW_NOT_SUPPORT_OUTERV6;
			}
#if defined(CONFIG_FC_RTL9607C_SERIES)
			if(fc_db.vxlan_ds_fastForward_num >= RTK_FC_VXLAN_DS_FF_NUM_MAX)
			{
				TRACE("[VXLAN][DS]HW offload number is full, sw-only");
				return FC_VXLAN_FLOW_DOWNSTREAM_FULL;
			}

			if( (dynamic_sram_desc!=2 && dynamic_sram_desc!=0) || (dynamic_sram_desc==0 && fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_mechanism ==0) )
				return FC_VXLAN_FLOW_NOT_VXLAN_NIC_ACC_MODE;

			if(fc_db.lutTbl[pG3IgrExtraInfo->lutEgrDaIdx]->spa > 3)
			{	
				TRACE("[VXLAN][DOWNSTREAM]Temporary not support dport exceed 0~3, swOnly!");
				return FC_VXLAN_FLOW_BYPASS_VXLAN_ADD;
			}
#elif defined(CONFIG_FC_CA8277B_SERIES)

			//1. Checkout inner outer learned before?s
			if(fc_db.vxlan_ds_fastForward_num >= RTK_FC_VXLAN_DS_FF_NUM_MAX)
			{
				if(fc_db.vxlan_ds_extra_fastForward_num < RTK_FC_VXLAN_EXTRA_MAX_NUM)
				{
					*useExtra = 1;
				}
				else
				{
					TRACE("[VXLAN][DS]HW offload number is full, sw-only");
					return FC_VXLAN_FLOW_DOWNSTREAM_FULL;
				}
			}
			
			if(*useExtra != 1)
			{
				*cpuPortIdx = _rtk_fc_vxlan_roundRobin_index_get(vxlan_direction);
			}
			
			if(*cpuPortIdx==-1)
			{	
				//TODO: Should be extra
				*useExtra = 1;
			}
			
#endif
		}
		else
			return FC_VXLAN_FLOW_NOT_SPECIAL_FASTFWD_MODE;

	}
	else
	{
		// VXLAN UPSTREAM
		
		if(fc_db.controlFuc.special_fast_forward_mode==1)
		{
		
			if(pPktHdr->outer_ip6h)
			{
				return FC_VXLAN_FLOW_NOT_SUPPORT_OUTERV6;
			}
			
#if defined(CONFIG_FC_RTL9607C_SERIES)
			if(fc_db.vxlan_us_fastForward_num >= RTK_FC_VXLAN_US_FF_NUM_MAX)
			{
				TRACE("[VXLAN][US]HW offload number is full, sw-only");
				return FC_VXLAN_FLOW_UPSTREAM_FULL;
			}	
			
			if(fc_db.vxlan_us_fastForward_wan_intf_id ==-1)
			{
				fc_db.vxlan_us_fastForward_wan_intf_id = egrNetifIdx;
			}
			else if(fc_db.vxlan_us_fastForward_wan_intf_id !=-1 && fc_db.vxlan_us_fastForward_wan_intf_id !=egrNetifIdx)
			{
				WARNING("Upstream only support one vxlan interface to Fast Forward");
				return FC_VXLAN_FLOW_UPSTREAM_INTERFACE_ERR;
			}

			if( (dynamic_sram_desc!=2 && dynamic_sram_desc!=0) || (dynamic_sram_desc==0 && fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_mechanism ==0) ) 
				return FC_VXLAN_FLOW_NOT_VXLAN_NIC_ACC_MODE;

			if(fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->spa > 3)
			{	
				TRACE("[VXLAN][UPSTREAM]Temporary not support dport exceed 0~3, swOnly!");
				return FC_VXLAN_FLOW_BYPASS_VXLAN_ADD;
			}
#elif defined(CONFIG_FC_CA8277B_SERIES)
			
			if(fc_db.vxlan_us_fastForward_num >= RTK_FC_VXLAN_US_FF_NUM_MAX)
			{
				if(fc_db.vxlan_us_fastForward_num < RTK_FC_VXLAN_EXTRA_MAX_NUM)
				{
					*useExtra = 1;
				}
				else
				{
					TRACE("[VXLAN][US]HW offload number is full, sw-only");
					return FC_VXLAN_FLOW_UPSTREAM_FULL;
				}
			}
			
			if(fc_db.vxlan_us_fastForward_wan_intf_id ==-1)
			{
				fc_db.vxlan_us_fastForward_wan_intf_id = egrNetifIdx;
			}
			else if(fc_db.vxlan_us_fastForward_wan_intf_id !=-1 && fc_db.vxlan_us_fastForward_wan_intf_id !=egrNetifIdx)
			{
				/*
				WARNING("Upstream only support one vxlan interface to Fast Forward");
				return FC_VXLAN_FLOW_UPSTREAM_INTERFACE_ERR;
				*/
				*useExtra = 1;
			}

			if(*useExtra != 1)
			{	
				*cpuPortIdx = _rtk_fc_vxlan_roundRobin_index_get(vxlan_direction);
			}
			
			if(*cpuPortIdx==-1)
			{	
				*useExtra = 1;
			}
			
			
#endif		
		}
		else
			return FC_VXLAN_FLOW_NOT_SPECIAL_FASTFWD_MODE;

	}

#if defined(CONFIG_FC_CA8277B_SERIES)

	if(*useExtra == 1)
	{
		if(vxlan_direction==0)
		{
			*cpuPortIdx = fc_db.vxlan_ds_extra_RoundRobin_cpuPort_index;
		}
		else
		{
			*cpuPortIdx = fc_db.vxlan_us_extra_RoundRobin_cpuPort_index;
		}
	}

	if(*cpuPortIdx == -1)
	{
		return FC_VXLAN_FLOW_CPU_PORT_SEL_ERROR;
	}
	
#endif
	return FC_VXLAN_FLOW_OK;

}
int _rtk_fc_L2_vxlan_check_allFlow_canBeAdded(int vxlan_direction, rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr,
 												   rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, int egrNetifIdx, rtk_rg_asic_path1_entry_t *pFlowEntry,
												   struct rt_skbuff *rtskb, int igr_netifIdx)
{

#if defined(CONFIG_FC_CA8277B_SERIES)
	if(vxlan_direction ==0 ) // DOWN STREAM
	{
		
		uint32 inner_path3_flowHashIdx = 0;
		uint8 flow_can_add=1;
		int by_pass_vxlan_add = 0;

		inner_path3_flowHashIdx = _rtk_fc_flow_hashIndex_byPktHdr(pPktHdr);
		inner_path3_flowHashIdx <<= fc_db.flowHashWayShift;
		DEBUG("[VXLAN][DS]Inner hash - %d",inner_path3_flowHashIdx);

		//Check outer and inner can all be added to hw entry.
		rtk_fc_l2_vxlan_check_flow_canBeAdded(&flow_can_add , RTK_FC_FLOW_DIRECTION_DOWNSTREAM, pFlowEntry, inner_path3_flowHashIdx, pFcIngressData->flowHashIdx, pG3IgrExtraInfo, pPktHdr, rtskb, igr_netifIdx, &by_pass_vxlan_add);

		if(flow_can_add != 1)
		{
			WARNING("Some flow can't be add! swonly!");
			return FC_VXLAN_FLOW_NOT_VXLAN_CANT_BE_ADDED;
		}
		else if(by_pass_vxlan_add==1)
		{
			return FC_VXLAN_FLOW_BYPASS_VXLAN_ADD;
		}
	}
	else // UP STREAM
	{
		rtk_fc_pktHdr_t tmp_outer_pktHdr;
		int tmp_outer_hash=0;
		uint8 flow_can_add=1;
		int by_pass_vxlan_add= 0;
		
		memcpy(&tmp_outer_pktHdr, pPktHdr, sizeof(rtk_fc_pktHdr_t));
		
		tmp_outer_pktHdr.eth = tmp_outer_pktHdr.outer_eth;
		tmp_outer_pktHdr.svh = tmp_outer_pktHdr.outer_svh;
		tmp_outer_pktHdr.cvh = tmp_outer_pktHdr.outer_cvh;
		tmp_outer_pktHdr.ppph = tmp_outer_pktHdr.outer_ppph;
		tmp_outer_pktHdr.iph = tmp_outer_pktHdr.outer_iph;
		tmp_outer_pktHdr.udph =  RTK_FC_HELPER_FC_KMALLOC(sizeof(struct udphdr), GFP_ATOMIC);
		tmp_outer_pktHdr.tcph =  NULL;
		memcpy(tmp_outer_pktHdr.udph, pPktHdr->outer_udph, sizeof(struct udphdr));
		//tmp_outer_pktHdr.udph = tmp_outer_pktHdr.outer_udph;

		tmp_outer_hash = _rtk_fc_flow_hashIndex_byPktHdr(&tmp_outer_pktHdr);
		tmp_outer_hash <<= fc_db.flowHashWayShift;
		
		RTK_FC_HELPER_FC_KFREE(tmp_outer_pktHdr.udph, sizeof(struct udphdr));

		rtk_fc_l2_vxlan_check_flow_canBeAdded(&flow_can_add , RTK_FC_FLOW_DIRECTION_UPSTREAM, pFlowEntry, pFcIngressData->flowHashIdx, tmp_outer_hash, pG3IgrExtraInfo, pPktHdr, rtskb, igr_netifIdx, &by_pass_vxlan_add);

		if(flow_can_add==0)
			return FC_VXLAN_FLOW_NOT_VXLAN_CANT_BE_ADDED;
		if(by_pass_vxlan_add==1)
			return FC_VXLAN_FLOW_BYPASS_VXLAN_ADD;

	}
#endif
	return FC_VXLAN_FLOW_OK;
}

int _rtk_fc_L3_vxlan_check_allFlow_canBeAdded(int vxlan_direction, rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr,
 												   rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, int egrNetifIdx, rtk_rg_asic_path1_entry_t *pFlowEntry,
												   struct rt_skbuff *rtskb, int igr_netifIdx)
{
	if(vxlan_direction ==0 ) // DOWN STREAM
	{
#if defined(CONFIG_FC_RTL9607C_SERIES)
		rtk_fc_tableFlowEntry_t tmp_flowPathEntry;
		uint16 igrSVID = 0, igrCVID = 0;
		uint8 flow_can_add=1;
		int by_pass_vxlan_add = 0;
		uint32 inner_path5_flowHashIdx = 0;
		
		memcpy(&tmp_flowPathEntry.path5,pFlowEntry,sizeof(rtk_rg_asic_path5_entry_t));
		
		igrSVID = pFcIngressData->srcSVlanId;
		igrCVID = pFcIngressData->srcCVlanId;

		inner_path5_flowHashIdx = _rtk_fc_flow_hashIndex(tmp_flowPathEntry, igrSVID, igrCVID, FALSE);


		TRACE("[VXLAN][DS]Inner hash - %d",inner_path5_flowHashIdx);

		//Check outer and inner can all be added to hw entry.
		rtk_fc_vxlan_check_flow_canBeAdded(&flow_can_add , RTK_FC_FLOW_DIRECTION_DOWNSTREAM, pFlowEntry, inner_path5_flowHashIdx, pFcIngressData->flowHashIdx, pG3IgrExtraInfo, pPktHdr, rtskb, igr_netifIdx, egrNetifIdx, &by_pass_vxlan_add);

		if(flow_can_add != 1)
		{
			WARNING("Some flow can't be add! swonly!");
			return FC_VXLAN_FLOW_NOT_VXLAN_CANT_BE_ADDED;
		}
		else if(by_pass_vxlan_add==1)
		{
			return FC_VXLAN_FLOW_BYPASS_VXLAN_ADD;
		}
		
#elif defined(CONFIG_FC_CA8277B_SERIES)
		uint32 inner_path5_flowHashIdx = 0;
		uint8 flow_can_add=1;
		int by_pass_vxlan_add = 0;

		inner_path5_flowHashIdx = _rtk_fc_flow_hashIndex_byPktHdr(pPktHdr);
		inner_path5_flowHashIdx <<= fc_db.flowHashWayShift;


		DEBUG("[VXLAN][DS]Inner hash - %d",inner_path5_flowHashIdx);

		//Check outer and inner can all be added to hw entry.
		rtk_fc_vxlan_check_flow_canBeAdded(&flow_can_add , RTK_FC_FLOW_DIRECTION_DOWNSTREAM, pFlowEntry, inner_path5_flowHashIdx, pFcIngressData->flowHashIdx, pG3IgrExtraInfo, pPktHdr, rtskb, igr_netifIdx, egrNetifIdx, &by_pass_vxlan_add);

		if(flow_can_add != 1)
		{
			WARNING("Some flow can't be add! swonly!");
			return FC_VXLAN_FLOW_NOT_VXLAN_CANT_BE_ADDED;
		}
		else if(by_pass_vxlan_add==1)
		{
			return FC_VXLAN_FLOW_BYPASS_VXLAN_ADD;
		}
#endif
	}
	else 
	{ // UP STREAM
#if defined(CONFIG_FC_RTL9607C_SERIES)
		{
			rtk_fc_pktHdr_t tmp_outer_pktHdr;
			int tmp_outer_hash=0;
			uint8 flow_can_add=1;
			int by_pass_vxlan_add= 0;
			
			memcpy(&tmp_outer_pktHdr, pPktHdr, sizeof(rtk_fc_pktHdr_t));
			
			tmp_outer_pktHdr.eth = tmp_outer_pktHdr.outer_eth;
			tmp_outer_pktHdr.svh = tmp_outer_pktHdr.outer_svh;
			tmp_outer_pktHdr.cvh = tmp_outer_pktHdr.outer_cvh;
			tmp_outer_pktHdr.ppph = tmp_outer_pktHdr.outer_ppph;
			tmp_outer_pktHdr.iph = tmp_outer_pktHdr.outer_iph;
			tmp_outer_pktHdr.udph =  RTK_FC_HELPER_FC_KMALLOC(sizeof(struct udphdr), GFP_ATOMIC);
			tmp_outer_pktHdr.tcph =  NULL;
			memcpy(tmp_outer_pktHdr.udph, pPktHdr->outer_udph, sizeof(struct udphdr));
			//tmp_outer_pktHdr.udph = tmp_outer_pktHdr.outer_udph;
			
			if((0x1<<pPktHdr->ingressPort.macPortIdx)&fc_db_fastFwd_data.vxlan_acc.vxlan_acceleration_extraPmsk)
				tmp_outer_pktHdr.udph->source = RTK_FC_VXLAN_ACC_EPHEMERAL_SPORT;
			DEBUG("sport: %d, dport: %d", ntohs(tmp_outer_pktHdr.udph->source), ntohs(tmp_outer_pktHdr.udph->dest));
	
			tmp_outer_pktHdr.svh = NULL;
			tmp_outer_pktHdr.cvh = NULL;
			tmp_outer_pktHdr.ppph = NULL;
			tmp_outer_pktHdr.dmacToGatewayMAC = 0;

			tmp_outer_hash = _rtk_fc_flow_hashIndex_byPktHdr(&tmp_outer_pktHdr);
			RTK_FC_HELPER_FC_KFREE(tmp_outer_pktHdr.udph, sizeof(struct udphdr));

			rtk_fc_vxlan_check_flow_canBeAdded(&flow_can_add , RTK_FC_FLOW_DIRECTION_UPSTREAM, pFlowEntry, pFcIngressData->flowHashIdx, tmp_outer_hash, pG3IgrExtraInfo, pPktHdr, rtskb, igr_netifIdx, egrNetifIdx, &by_pass_vxlan_add);

			if(flow_can_add==0)
				return FC_VXLAN_FLOW_NOT_VXLAN_CANT_BE_ADDED;
			if(by_pass_vxlan_add==1)
				return FC_VXLAN_FLOW_BYPASS_VXLAN_ADD;
		}
#elif defined(CONFIG_FC_CA8277B_SERIES)
		{
			rtk_fc_pktHdr_t tmp_outer_pktHdr;
			int tmp_outer_hash=0;
			uint8 flow_can_add=1;
			int by_pass_vxlan_add= 0;
			
			memcpy(&tmp_outer_pktHdr, pPktHdr, sizeof(rtk_fc_pktHdr_t));
			
			tmp_outer_pktHdr.eth = tmp_outer_pktHdr.outer_eth;
			tmp_outer_pktHdr.svh = tmp_outer_pktHdr.outer_svh;
			tmp_outer_pktHdr.cvh = tmp_outer_pktHdr.outer_cvh;
			tmp_outer_pktHdr.ppph = tmp_outer_pktHdr.outer_ppph;
			tmp_outer_pktHdr.iph = tmp_outer_pktHdr.outer_iph;
			tmp_outer_pktHdr.udph =  RTK_FC_HELPER_FC_KMALLOC(sizeof(struct udphdr), GFP_ATOMIC);
			memcpy(tmp_outer_pktHdr.udph, pPktHdr->outer_udph, sizeof(struct udphdr));
			//tmp_outer_pktHdr.udph = tmp_outer_pktHdr.outer_udph;

			tmp_outer_hash = _rtk_fc_flow_hashIndex_byPktHdr(&tmp_outer_pktHdr);
			tmp_outer_hash <<= fc_db.flowHashWayShift;
			
			RTK_FC_HELPER_FC_KFREE(tmp_outer_pktHdr.udph, sizeof(struct udphdr));

			rtk_fc_vxlan_check_flow_canBeAdded(&flow_can_add , RTK_FC_FLOW_DIRECTION_UPSTREAM, pFlowEntry, pFcIngressData->flowHashIdx, tmp_outer_hash, pG3IgrExtraInfo, pPktHdr, rtskb, igr_netifIdx, egrNetifIdx, &by_pass_vxlan_add);

			if(flow_can_add==0)
				return FC_VXLAN_FLOW_NOT_VXLAN_CANT_BE_ADDED;
			if(by_pass_vxlan_add==1)
				return FC_VXLAN_FLOW_BYPASS_VXLAN_ADD;

		}
#endif

	}

	return FC_VXLAN_FLOW_OK;
}
int _rtk_fc_L2_vxlan_flow_processing(rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, int egrNetifIdx,
									rtk_rg_asic_path1_entry_t *pFlowEntry, struct rt_skbuff *rtskb, int igr_netifIdx, struct rt_nfconn *rtct, rtk_fc_igrExtraInfo_t *pExtraInfo, int16 outer_daIdx,
									uint32 swOnly_flowIdx)
{
#if defined(CONFIG_FC_CA8277B_SERIES)
	int ret = FC_VXLAN_FLOW_OK;
	int VXLAN_direction = 0; // 0: downstream, 1: upstream
	uint32 innerFlowIndex = 0, outerFlowIndex = 0;
	int cpuPortIdx;

	if(pFcIngressData->isDualHeader && pFcIngressData->ingressTagif & VXLAN_TAGIF && pPktHdr->dualHdrType == RTK_FC_DUALTYPE_NONE )
		VXLAN_direction = 0;
	else if( pFcIngressData->isDualHeader==0 && pPktHdr->dualHdrType == RTK_FC_DUALTYPE_VXLAN)	
		VXLAN_direction = 1;
	else//Not VXLAN
		return FC_VXLAN_FLOW_OK;

	ret = _rtk_fc_L2_vxlan_preChecking(VXLAN_direction, pFcIngressData, pPktHdr, pG3IgrExtraInfo, egrNetifIdx, pFlowEntry, rtskb, &cpuPortIdx);

	if(ret!=FC_VXLAN_FLOW_OK)
	{
		return ret;
	}
	
	ret = _rtk_fc_L2_vxlan_check_allFlow_canBeAdded(VXLAN_direction, pFcIngressData, pPktHdr, pG3IgrExtraInfo, egrNetifIdx, pFlowEntry, rtskb, igr_netifIdx);
	
	if(ret!=FC_VXLAN_FLOW_OK)
	{
		return ret;
	}
	
	ret = _rtk_fc_L2_vxlan_g3IgrExtraInfo_setting(VXLAN_direction, pFcIngressData, pPktHdr, pG3IgrExtraInfo, egrNetifIdx, pFlowEntry, rtskb, cpuPortIdx);
	
	if(ret!=FC_VXLAN_FLOW_OK)
	{
		return ret;
	}
	
	if(VXLAN_direction==1) // VXLAN UPSTREAM
	{
		//if(fc_db.flowTbl[swOnly_flowIdx].l2Dual_table_info.l2Dual_table_index != 0)
		{
			ret = _rtk_fc_l2_vxlan_special_fastFwd_upstream_setting(pPktHdr, pG3IgrExtraInfo);
			
			if(ret!= FC_VXLAN_FLOW_OK)
				return ret;
		}
		

		ret = _rtk_fc_l2_vxlan_upstream_outerFlow_add_process(pPktHdr, pExtraInfo->lutIgrSaIdx, outer_daIdx, 0, pFcIngressData, pFlowEntry, igr_netifIdx, rtskb, innerFlowIndex, &outerFlowIndex);
		if(ret!= FC_VXLAN_FLOW_OK)
		{
			rtk_fc_flow_delete(innerFlowIndex);
			return ret;
		}
		
		ret = _rtk_fc_l2_vxlan_upstream_innerFlow_add_process(pFcIngressData, pFlowEntry, NULL, pExtraInfo, pPktHdr, pG3IgrExtraInfo, swOnly_flowIdx, &innerFlowIndex, cpuPortIdx);

		if(ret!= FC_VXLAN_FLOW_OK)
		{	
			return ret;
		}
		fc_db.flowTbl[outerFlowIndex].l2Dual_table_info.vxlan_extra_index =pG3IgrExtraInfo->vxlan_info.extraIndex;
		fc_db.flowTbl[innerFlowIndex].l2Dual_table_info.vxlan_extra_index =pG3IgrExtraInfo->vxlan_info.extraIndex;
		fc_db.flowTbl[swOnly_flowIdx].l2Dual_table_info.vxlan_extra_index =pG3IgrExtraInfo->vxlan_info.extraIndex;
		// index start from 1
		if(fc_db.vxlan_extra_upStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex) ].isSet ==0)
		{
			fc_db.vxlan_extra_upStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex)].sw_flow_index = swOnly_flowIdx;
			fc_db.vxlan_extra_upStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex)].outer_flow_index = outerFlowIndex;
			fc_db.vxlan_extra_upStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex)].inner_flow_index = innerFlowIndex;
			fc_db.vxlan_extra_upStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex)].extraVoq_index = (pG3IgrExtraInfo->vxlan_info.extraVOQ==5)?0:1;
			fc_db.vxlan_extra_upStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex)].isSet =1;
			fc_db.vxlan_us_extra_fastForward_num++;
			DEBUG("[VXLAN][US][EXTRA]Inner mainhash: %d hwlookup mainhash: %d cpu-port:0x%x", fc_db.flowTbl[innerFlowIndex].mainHashIdx, fc_db.flowTbl[outerFlowIndex].mainHashIdx, fc_db.vxlan_extra_upStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex -1)].cpuPort  );

		}

		if(fc_db.vxlan_us_extra_RoundRobin_cpuPort_index ==3)
		{	
			fc_db.vxlan_us_extra_RoundRobin_cpuPort_index = 0;
		}
		else
		{
			fc_db.vxlan_us_extra_RoundRobin_cpuPort_index +=1;
		}
	}
	else		// VXLAN DOWNSTREAM
	{
		
		ca_ni_init_extra_ff_global_param_setting();
		
		ret = _rtk_fc_l2_vxlan_downstream_innerFlow_add_process(pPktHdr, pG3IgrExtraInfo, pFcIngressData, &innerFlowIndex, pFlowEntry, rtct, 0, pExtraInfo, 0, rtskb);

		if(ret!= FC_VXLAN_FLOW_OK)
		{
			return ret;
		}
		
		
		ret = _rtk_fc_l2_vxlan_downstream_outerFlow_add_process(pFcIngressData, rtskb, pPktHdr, innerFlowIndex, igr_netifIdx, pExtraInfo, pG3IgrExtraInfo, &outerFlowIndex, cpuPortIdx);

		if(ret!= FC_VXLAN_FLOW_OK)
		{
			DEBUG("Downstream outer flow add failed, delete innerFlow");
			rtk_fc_flow_delete(innerFlowIndex);
			return ret;
		}
		
		fc_db.flowTbl[outerFlowIndex].l2Dual_table_info.vxlan_extra_index =pG3IgrExtraInfo->vxlan_info.extraIndex;
		fc_db.flowTbl[innerFlowIndex].l2Dual_table_info.vxlan_extra_index =pG3IgrExtraInfo->vxlan_info.extraIndex;
		fc_db.flowTbl[swOnly_flowIdx].l2Dual_table_info.vxlan_extra_index =pG3IgrExtraInfo->vxlan_info.extraIndex;
		
		if(fc_db.vxlan_extra_downStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex)].isSet ==0)
		{
			fc_db.vxlan_extra_downStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex)].sw_flow_index = swOnly_flowIdx;
			fc_db.vxlan_extra_downStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex)].outer_flow_index = outerFlowIndex;
			fc_db.vxlan_extra_downStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex)].inner_flow_index = innerFlowIndex;
			fc_db.vxlan_extra_downStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex)].isSet =1;
			DEBUG("[VXLAN][DS][EXTRA]Inner mainhash: %d hwlookup mainhash: %d  cpu-port:0x%x", fc_db.flowTbl[outerFlowIndex].mainHashIdx, fc_db.flowTbl[innerFlowIndex].mainHashIdx, fc_db.vxlan_extra_downStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex -1)].cpuPort);
			fc_db.vxlan_ds_extra_fastForward_num++;
		}
		
		if(fc_db.vxlan_ds_extra_RoundRobin_cpuPort_index ==3)
		{	
			fc_db.vxlan_ds_extra_RoundRobin_cpuPort_index = 0;
		}
		else
		{
			fc_db.vxlan_ds_extra_RoundRobin_cpuPort_index +=1;
		}
	}
	
	
#endif	
	return FC_VXLAN_FLOW_OK;

}
int _rtk_fc_L3_vxlan_flow_processing(rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, int egrNetifIdx,
									rtk_rg_asic_path1_entry_t *pFlowEntry, struct rt_skbuff *rtskb, int igr_netifIdx, struct rt_nfconn *rtct, rtk_fc_igrExtraInfo_t *pExtraInfo, int16 outer_daIdx,
									uint32 swOnly_flowIdx)
{
	int ret = FC_VXLAN_FLOW_OK;
	int VXLAN_direction = 0; // 0: downstream, 1: upstream
	uint32 innerFlowIndex = 0, outerFlowIndex = 0;
	int cpuPortIdx, useExtra = 0;
	
	if(pFcIngressData->isDualHeader && pFcIngressData->ingressTagif & VXLAN_TAGIF && pPktHdr->dualHdrType == RTK_FC_DUALTYPE_NONE )
		VXLAN_direction = 0;
	else if( pFcIngressData->isDualHeader==0 && pPktHdr->dualHdrType == RTK_FC_DUALTYPE_VXLAN)	
		VXLAN_direction = 1;
	else//Not VXLAN
		return FC_VXLAN_FLOW_OK;

	ret = _rtk_fc_L3_vxlan_preChecking(VXLAN_direction, pFcIngressData, pPktHdr, pG3IgrExtraInfo, egrNetifIdx, pFlowEntry, rtskb, &cpuPortIdx, &useExtra);

	if(ret!=FC_VXLAN_FLOW_OK)
	{
		return ret;
	}
	
	ret = _rtk_fc_L3_vxlan_check_allFlow_canBeAdded(VXLAN_direction, pFcIngressData, pPktHdr, pG3IgrExtraInfo, egrNetifIdx, pFlowEntry, rtskb, igr_netifIdx);
	
	if(ret!=FC_VXLAN_FLOW_OK)
	{
		return ret;
	}
	
	ret = _rtk_fc_L3_vxlan_g3IgrExtraInfo_setting(VXLAN_direction, pFcIngressData, pPktHdr, pG3IgrExtraInfo, egrNetifIdx, pFlowEntry, rtskb, useExtra, cpuPortIdx);
	
	if(ret!=FC_VXLAN_FLOW_OK)
	{
		return ret;
	}
		
	if(VXLAN_direction==1) // VXLAN UPSTREAM
	{
		ret = _rtk_fc_vxlan_special_fastFwd_upstream_setting(pPktHdr, pG3IgrExtraInfo, useExtra );
		
		if(ret!= FC_VXLAN_FLOW_OK)
			return ret;
	
		
		
		ret = _rtk_fc_vxlan_upstream_outerFlow_add_process(pPktHdr, pExtraInfo->lutIgrSaIdx, outer_daIdx, 0, pFcIngressData, pFlowEntry, igr_netifIdx, rtskb, innerFlowIndex, &outerFlowIndex);
		if(ret!= FC_VXLAN_FLOW_OK)
		{
			rtk_fc_flow_delete(innerFlowIndex);
			return ret;
		}
		
		ret = _rtk_fc_vxlan_upstream_innerFlow_add_process(pFcIngressData, pFlowEntry, NULL, pExtraInfo, pPktHdr, pG3IgrExtraInfo, swOnly_flowIdx, &innerFlowIndex, cpuPortIdx,rtskb);

		if(ret!= FC_VXLAN_FLOW_OK)
			return ret;
		
		//For Upstream record
#if defined(CONFIG_FC_CA8277B_SERIES)
		if(useExtra)
		{
			
			
			fc_db.flowTbl[outerFlowIndex].l2Dual_table_info.vxlan_extra_index =pG3IgrExtraInfo->vxlan_info.extraIndex;
			
			fc_db.flowTbl[innerFlowIndex].l2Dual_table_info.vxlan_extra_index =pG3IgrExtraInfo->vxlan_info.extraIndex;
			
			fc_db.flowTbl[swOnly_flowIdx].l2Dual_table_info.vxlan_extra_index =pG3IgrExtraInfo->vxlan_info.extraIndex;
			// index start from 1
			if(fc_db.vxlan_extra_upStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex ) ].isSet ==0)
			{
				fc_db.vxlan_extra_upStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex )].sw_flow_index = swOnly_flowIdx;
				fc_db.vxlan_extra_upStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex )].outer_flow_index = outerFlowIndex;
				fc_db.vxlan_extra_upStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex )].inner_flow_index = innerFlowIndex;
				fc_db.vxlan_extra_upStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex )].extraVoq_index = (pG3IgrExtraInfo->vxlan_info.extraVOQ==5)?0:1;
				fc_db.vxlan_extra_upStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex )].isSet =1;
				fc_db.vxlan_us_extra_fastForward_num++;
				DEBUG("[VXLAN][US][EXTRA]Inner mainhash: %d hwlookup mainhash: %d cpu-port:0x%x", fc_db.flowTbl[innerFlowIndex].mainHashIdx, fc_db.flowTbl[outerFlowIndex].mainHashIdx, fc_db.vxlan_extra_upStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex -1)].cpuPort  );

			}
			if(fc_db.vxlan_us_extra_RoundRobin_cpuPort_index ==3)
			{	
				fc_db.vxlan_us_extra_RoundRobin_cpuPort_index = 0;
			}
			else
			{
				fc_db.vxlan_us_extra_RoundRobin_cpuPort_index +=1;
			}
		}
		else
		{
			if(fc_db.vxlan_upStream_record[cpuPortIdx].isSet ==0)
			{
				fc_db.vxlan_upStream_record[cpuPortIdx].sw_flow_index = swOnly_flowIdx;
				fc_db.vxlan_upStream_record[cpuPortIdx].outer_flow_index = outerFlowIndex;
				fc_db.vxlan_upStream_record[cpuPortIdx].inner_flow_index = innerFlowIndex;
				fc_db.vxlan_upStream_record[cpuPortIdx].isSet =1;
				fc_db.vxlan_us_fastForward_num++;
				DEBUG("[VXLAN][US]Inner mainhash: %d hwlookup mainhash: %d cpu-port:0x%x", fc_db.flowTbl[innerFlowIndex].mainHashIdx, fc_db.flowTbl[outerFlowIndex].mainHashIdx, fc_db.vxlan_upStream_record[cpuPortIdx].cpuPort  );

			}
		}
#elif defined(CONFIG_FC_RTL9607C_SERIES)
		if(fc_db.vxlan_upStream_record[fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->spa].isSet ==0)
		{
			fc_db.vxlan_upStream_record[fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->spa].outer_flow_index = outerFlowIndex;
			fc_db.vxlan_upStream_record[fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->spa].inner_flow_index = innerFlowIndex;
			fc_db.vxlan_upStream_record[fc_db.lutTbl[pG3IgrExtraInfo->lutIgrSaIdx]->spa].isSet =1;
			fc_db.vxlan_us_fastForward_num++;

		}
#endif	
	}
	else if(VXLAN_direction == 0) // VXLAN DOWNSTREAM
	{
		
		_rtk_fc_vxlan_special_fastFwd_downstream_setting(pG3IgrExtraInfo);
		
#if defined(CONFIG_FC_CA8277B_SERIES)		
		if(useExtra)
		{
			ca_ni_init_extra_ff_global_param_setting();
		}
#endif	
		ret = _rtk_fc_vxlan_downstream_innerFlow_add_process(pPktHdr, pG3IgrExtraInfo, pFcIngressData, &innerFlowIndex, pFlowEntry, rtct, 0, pExtraInfo, 0, rtskb);

		if(ret!= FC_VXLAN_FLOW_OK)
		{
			return ret;
		}
		
		
		ret = _rtk_fc_vxlan_downstream_outerFlow_add_process(pFcIngressData, rtskb, pPktHdr, innerFlowIndex, igr_netifIdx, egrNetifIdx, pExtraInfo, pG3IgrExtraInfo, &outerFlowIndex, cpuPortIdx);

		if(ret!= FC_VXLAN_FLOW_OK)
		{
			DEBUG("Downstream outer flow add failed, delete innerFlow");
			rtk_fc_flow_delete(innerFlowIndex);
			return ret;
		}
		
		//For Downstream record
#if defined(CONFIG_FC_CA8277B_SERIES)
		if(useExtra)
		{
			
			fc_db.flowTbl[outerFlowIndex].l2Dual_table_info.vxlan_extra_index =pG3IgrExtraInfo->vxlan_info.extraIndex;
			fc_db.flowTbl[innerFlowIndex].l2Dual_table_info.vxlan_extra_index =pG3IgrExtraInfo->vxlan_info.extraIndex;
			fc_db.flowTbl[swOnly_flowIdx].l2Dual_table_info.vxlan_extra_index =pG3IgrExtraInfo->vxlan_info.extraIndex;
			
			if(fc_db.vxlan_extra_downStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex )].isSet ==0)
			{
				fc_db.vxlan_extra_downStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex )].sw_flow_index = swOnly_flowIdx;
				fc_db.vxlan_extra_downStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex )].outer_flow_index = outerFlowIndex;
				fc_db.vxlan_extra_downStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex )].inner_flow_index = innerFlowIndex;
				fc_db.vxlan_extra_downStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex )].isSet =1;
				DEBUG("[VXLAN][DS][EXTRA]Inner mainhash: %d hwlookup mainhash: %d  cpu-port:0x%x", fc_db.flowTbl[outerFlowIndex].mainHashIdx, fc_db.flowTbl[innerFlowIndex].mainHashIdx, fc_db.vxlan_extra_downStream_record[(pG3IgrExtraInfo->vxlan_info.extraIndex -1)].cpuPort);
				fc_db.vxlan_ds_extra_fastForward_num++;
			}
			
			if(fc_db.vxlan_ds_extra_RoundRobin_cpuPort_index ==3)
			{	
				fc_db.vxlan_ds_extra_RoundRobin_cpuPort_index = 0;
			}
			else
			{
				fc_db.vxlan_ds_extra_RoundRobin_cpuPort_index +=1;
			}

		}
		else
		{
			if(fc_db.vxlan_downStream_record[cpuPortIdx].isSet ==0)
			{
				fc_db.vxlan_downStream_record[cpuPortIdx].sw_flow_index = swOnly_flowIdx;
				fc_db.vxlan_downStream_record[cpuPortIdx].outer_flow_index = outerFlowIndex;
				fc_db.vxlan_downStream_record[cpuPortIdx].inner_flow_index = innerFlowIndex;
				fc_db.vxlan_downStream_record[cpuPortIdx].isSet =1;
				DEBUG("[VXLAN][DS]Inner mainhash: %d hwlookup mainhash: %d  cpu-port:0x%x", fc_db.flowTbl[outerFlowIndex].mainHashIdx, fc_db.flowTbl[innerFlowIndex].mainHashIdx, fc_db.vxlan_downStream_record[cpuPortIdx].cpuPort);
				fc_db.vxlan_ds_fastForward_num++;
			}

		}
#elif defined(CONFIG_FC_RTL9607C_SERIES)
		if(fc_db.vxlan_downStream_record[fc_db.lutTbl[pG3IgrExtraInfo->lutEgrDaIdx]->spa].isSet ==0)
		{
			fc_db.vxlan_downStream_record[fc_db.lutTbl[pG3IgrExtraInfo->lutEgrDaIdx]->spa].outer_flow_index = outerFlowIndex;
			fc_db.vxlan_downStream_record[fc_db.lutTbl[pG3IgrExtraInfo->lutEgrDaIdx]->spa].inner_flow_index = innerFlowIndex;
			fc_db.vxlan_downStream_record[fc_db.lutTbl[pG3IgrExtraInfo->lutEgrDaIdx]->spa].isSet =1;
			fc_db.vxlan_ds_fastForward_num++;

		}
#endif		
	
	}

	return ret;
}
int _rtk_fc_vxlan_hwFlow_add_processing(void)
{
	int ret = FC_VXLAN_FLOW_OK; 
	return ret;
}

int _rtk_fc_vxlan_processing(rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, int egrNetifIdx,
									rtk_rg_asic_path1_entry_t *pFlowEntry, struct rt_skbuff *rtskb, int igr_netifIdx, struct rt_nfconn *rtct, rtk_fc_igrExtraInfo_t *pExtraInfo, int16 outer_daIdx,
									uint32 swOnly_flowIdx)
{
	int ret = FC_VXLAN_FLOW_OK; 

	if(fc_db.controlFuc.hwnat_mode == RT_FLOW_HWNAT_MODE_SW_ONLY)
	{
		return FC_VXLAN_FLOW_BYPASS_SWONLY_MODE;
	}
	if(fc_db.controlFuc.bridge_5tuple_flow_accelerate_by_2tuple==1)
	{
		TABLE("Please disable bridge_5tuple_flow_accelerate_by_2tuple to set special fast forward.");
		return FC_VXLAN_FLOW_BYPASS_VXLAN_ADD;
	}
	if(pFcIngressData->isDualHeader && pFcIngressData->ingressTagif & VXLAN_TAGIF && pPktHdr->dualHdrType == RTK_FC_DUALTYPE_NONE )
	{	DEBUG("[VXLAN]Find VXLAN downstream");}
	else if( pFcIngressData->isDualHeader==0 && pPktHdr->dualHdrType == RTK_FC_DUALTYPE_VXLAN)	
	{	DEBUG("[VXLAN]Find VXLAN upstream");}
	else//Not VXLAN
		return FC_VXLAN_FLOW_OK;

	if(pPktHdr->skip_special_fastFwd == 1)
		return FC_VXLAN_FLOW_BYPASS_VXLAN_ADD;
	/*
	if(!pFcIngressData->isDAGatewayMAC)
	{
		DEBUG("[VXLAN]None L3 VXLAN, currently only support L3 VXLAN!");
		return FC_VXLAN_FLOW_BYPASS_VXLAN_ADD;
	}
	*/

	if(!pFcIngressData->isDAGatewayMAC || pPktHdr->direction == RTK_FC_FLOW_DIRECTION_LANBRIDGE || pPktHdr->fwdType == RTK_FC_FWDTYPE_BRIDGE)
	{
		// L2 VXLAN processing
		DEBUG("[VXLAN] L2 Bridge VXLAN processing");
		ret = _rtk_fc_L2_vxlan_flow_processing(pFcIngressData, pPktHdr, pG3IgrExtraInfo, egrNetifIdx,
									pFlowEntry, rtskb, igr_netifIdx, rtct, pExtraInfo, outer_daIdx, swOnly_flowIdx);
	}
	else
	{
		DEBUG("[VXLAN] L3 VXLAN processing");
		ret = _rtk_fc_L3_vxlan_flow_processing(pFcIngressData, pPktHdr, pG3IgrExtraInfo, egrNetifIdx,
									pFlowEntry, rtskb, igr_netifIdx, rtct, pExtraInfo, outer_daIdx, swOnly_flowIdx);
	}
	return ret;
}

int _rtk_fc_l2Dual_vxlan_flow_del(rtk_fc_tableFlow_t *pFlowTable, int flowIdx)
{
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL9607C_SERIES)

	int i;


	if( pFlowTable->l2Dual_table_info.is_outer_flow==0)
	{
		// For Special fast forward
		if(fc_db.controlFuc.special_fast_forward_mode==1  && FLOW_INFO_IS_SET(pFlowTable, FLOW_INFO_SOFTWARE_ONLY) !=1)
		{
			
			if(pFlowTable->protoCtrl == FLOW_PROTO_CTRL_VXLAN_ADD && 
				FLOW_INFO_IS_SET(pFlowTable, FLOW_INFO_SOFTWARE_ONLY) != 1)
			{
				if(fc_db.vxlan_us_fastForward_num > 0 )
					fc_db.vxlan_us_fastForward_num--;
				
				if(fc_db.vxlan_us_fastForward_num==0)	
				{
					fc_db.vxlan_us_fastForward_wan_intf_id = -1;
					
#if defined(CONFIG_FC_CA8277B_SERIES)
					fc_db.vxlan_upstream_ca_extra_set = 0;

					fc_db.txBuffInitDone = 0;
					
					if(fc_db.vxlan_hw_issue_work_around_setting==1)
					{	
						DMA_SEC_DMA_GLB_DMA_LSO_CTRL_t	  dma_lso_ctrl;
						NI_HV_GLB_NIRX_L3FE_DPQ_DEMUX_CFG0_t        l3fe_dpq_demux_cfg0;
						
						
						rtk_ne_reg_write(0x000800ff, L2TM_L2TM_ES_SCH_CFG0 + L2TM_L2TM_ES_SCH_CFG0_STRIDE * 10);	// port 10 - wan to L3FE
						rtk_ne_reg_write(0x000800ff, L2TM_L2TM_ES_SCH_CFG0 + L2TM_L2TM_ES_SCH_CFG0_STRIDE * 13);	// port 13 - lan to L3FE
						rtk_ne_reg_write(0x000800ff, L2TM_L2TM_ES_SCH_CFG0 + L2TM_L2TM_ES_SCH_CFG0_STRIDE * 8);		// port 8 - L2TM to L3QM
						aal_port_arb_ldpid_pdpid_map_set(0, 0, 0, 0x19, 0xd);

						//Set dma-lso tx_burst_len to 2 (32 * 8 bytes )
						dma_lso_ctrl.wrd = rtk_dma_lso_reg_read(DMA_SEC_DMA_GLB_DMA_LSO_CTRL);
						dma_lso_ctrl.bf.tx_burst_len  = 3;
						rtk_dma_lso_reg_write(dma_lso_ctrl.wrd, DMA_SEC_DMA_GLB_DMA_LSO_CTRL);

						//Up-stream to Pon by-pass QM
						l3fe_dpq_demux_cfg0.wrd = 0xAAAA0000;
						rtk_ne_reg_write(l3fe_dpq_demux_cfg0.wrd, NI_HV_GLB_NIRX_L3FE_DPQ_DEMUX_CFG0);

						fc_db.vxlan_hw_issue_work_around_setting=0;
					}
#elif defined(CONFIG_FC_RTL9607C_SERIES)
					fc_db.vxlan_upstream_set_ok = 0;
					fc_db_fastFwd_data.vxlan_acc.vxlan_upstreamInfo_prepared = 0;
					fc_db_fastFwd_data.vxlan_acc.vxlan_extra_upstreamInfo_prepared = 0;
#endif
				}

#if defined(CONFIG_FC_CA8277B_SERIES)
				if(pFlowTable->l2Dual_table_info.vxlan_extra_index > 0)
				{
					ca_ni_init_tx_dma_lso_extra_ff_del(pFlowTable->l2Dual_table_info.vxlan_extra_index);
					if(fc_db.vxlan_extra_upStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index) ].isSet==1)
					{
						//TABLE("[EXTRA VXLAN][UPSTREAM]Now going to delete extra %d outer flow index: %d",(pFlowTable->l2Dual_table_info.vxlan_extra_index), fc_db.vxlan_extra_upStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index -1) ].outer_flow_index);
						rtk_fc_flow_delete(fc_db.vxlan_extra_upStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index) ].outer_flow_index);

						if(fc_db.vxlan_extra_upStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index)].cpuPort == 0x14)
						{
							fc_db.vxlan_extraVOQ_usage[0][fc_db.vxlan_extra_upStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index)].extraVoq_index] = 0;
						}
						else if(fc_db.vxlan_extra_upStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index)].cpuPort == 0x15)
						{
							fc_db.vxlan_extraVOQ_usage[1][fc_db.vxlan_extra_upStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index)].extraVoq_index] = 0;
						}
						else if(fc_db.vxlan_extra_upStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index)].cpuPort == 0x16)
						{
							fc_db.vxlan_extraVOQ_usage[2][fc_db.vxlan_extra_upStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index)].extraVoq_index] = 0;
						}
						else if(fc_db.vxlan_extra_upStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index)].cpuPort == 0x17)
						{
							fc_db.vxlan_extraVOQ_usage[3][fc_db.vxlan_extra_upStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index)].extraVoq_index] = 0;
						}
						fc_db.vxlan_extra_upStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index)].sw_flow_index = 0;
						fc_db.vxlan_extra_upStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index)].outer_flow_index = 0;
						fc_db.vxlan_extra_upStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index)].inner_flow_index = 0;
						fc_db.vxlan_extra_upStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index)].extraVoq_index = 0;
						fc_db.vxlan_extra_upStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index)].cpuPort = 0;
						fc_db.vxlan_extra_upStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index)].isSet =0;
						
					}
					if(fc_db.vxlan_us_extra_fastForward_num>0)
					{	
						fc_db.vxlan_us_extra_fastForward_num --;
					}
					
				}
#endif
			}
			else if(pFlowTable->protoCtrl == FLOW_PROTO_CTRL_VXLAN_REMOVE && 
				FLOW_INFO_IS_SET(pFlowTable, FLOW_INFO_SOFTWARE_ONLY) !=1)
			{
				if(fc_db.vxlan_ds_fastForward_num > 0 )
					fc_db.vxlan_ds_fastForward_num--;
				
				if(fc_db.vxlan_ds_fastForward_num==0)
				{
#if defined(CONFIG_FC_RTL9607C_SERIES)
				
					fc_db.vxlan_downstream_set_ok = 0;
					fc_db_fastFwd_data.vxlan_acc.vxlan_downstreamInfo_prepared = 0;
					fc_db_fastFwd_data.vxlan_acc.vxlan_extra_downstreamInfo_prepared = 0;
				
#elif defined(CONFIG_FC_CA8277B_SERIES)
				
					fc_db.vxlan_downstream_ca_extra_set = 0;
				
#endif
				}
#if defined(CONFIG_FC_CA8277B_SERIES)
				if( pFlowTable->l2Dual_table_info.vxlan_extra_index > 0)
				{
					if(fc_db.vxlan_extra_downStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index) ].isSet==1)
					{
						TABLE("[EXTRA VXLAN][DOWNSTREAM]Now going to delete extra %d outer flow index: %d",(pFlowTable->l2Dual_table_info.vxlan_extra_index), fc_db.vxlan_extra_downStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index) ].outer_flow_index);
						rtk_fc_flow_delete(fc_db.vxlan_extra_downStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index) ].outer_flow_index);
						fc_db.vxlan_extra_downStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index)].sw_flow_index = 0;
						fc_db.vxlan_extra_downStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index)].outer_flow_index = 0;
						fc_db.vxlan_extra_downStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index)].inner_flow_index = 0;
						fc_db.vxlan_extra_downStream_record[(pFlowTable->l2Dual_table_info.vxlan_extra_index)].isSet =0;
					}
					
					if(fc_db.vxlan_ds_extra_fastForward_num>0)
					{	
						fc_db.vxlan_ds_extra_fastForward_num --;
					}
				}
#endif				
			}
#if defined(CONFIG_FC_CA8277B_SERIES)	

			if(fc_db.vxlan_us_fastForward_num==0 && fc_db.vxlan_ds_fastForward_num==0)
			{	
				_rtk_fc_vxlan_8277b_reservedACL_del();
			}
#endif				
			//----------------------------------------UP/DOWN Stream related record delete-----------------------------
			for(i = 0 ; i < 4 ; i++)
			{
				if(flowIdx == fc_db.vxlan_upStream_record[i].inner_flow_index && fc_db.vxlan_upStream_record[i].isSet ==1)
				{
#if defined(CONFIG_FC_CA8277B_SERIES)					
					TABLE("DELETE VXLAN up stream inner flow");
					_rtk_fc_vxlan_8277b_reservedACL_del_by_idx(i, 1);

					fc_db.vxlan_upStream_record[i].isSet = 0;
					// delete outer flow
					if(fc_db.vxlan_upStream_record[i].outer_flow_index!=0)
						rtk_fc_flow_delete(fc_db.vxlan_upStream_record[i].outer_flow_index);
					
					fc_db.vxlan_upStream_record[i].inner_flow_index = 0;
					fc_db.vxlan_upStream_record[i].outer_flow_index = 0;
					fc_db.vxlan_upStream_record[i].cpuPort = 0;

					//memset(&fc_db.vxlan_us_SpecialFastFwdTbl[i], 0, sizeof(fc_db.vxlan_us_SpecialFastFwdTbl[i]));
					
					break;
#elif defined(CONFIG_FC_RTL9607C_SERIES)
					TABLE("DELETE VXLAN up stream inner flow");

					if(i==0)  // Normal
					{
						fc_db.vxlan_upStream_record[i].isSet = 0 ;
						// delete outer flow
						if(fc_db.vxlan_upStream_record[i].inner_flow_index!=0)
							rtk_fc_flow_delete(fc_db.vxlan_upStream_record[i].outer_flow_index);
						fc_db.vxlan_upStream_record[i].inner_flow_index = 0;
						fc_db.vxlan_upStream_record[i].outer_flow_index = 0;
						fc_db.vxlan_upStream_record[i].cpuPort = 0;
						fc_db.vxlan_port0_nicInfo_customized_entry_isSet = 0;
					}
					else	// extra, may share outer flow
					{
						TABLE("[VXLAN]Delete extra inner flow: %d from port: %d",fc_db.vxlan_upStream_record[i].inner_flow_index, i);
						fc_db.vxlan_upStream_record[i].inner_flow_index = 0;
						fc_db.vxlan_upStream_record[i].isSet = 0 ;
						if(fc_db.vxlan_upStream_record[1].isSet ==0 && fc_db.vxlan_upStream_record[2].isSet ==0 && fc_db.vxlan_upStream_record[3].isSet ==0)
						{
							TABLE("[VXLAN]EXTRA: All extra inner is deleted, now try to del share outer flow %d",fc_db.vxlan_upStream_record[i].outer_flow_index);
							rtk_fc_flow_delete(fc_db.vxlan_upStream_record[i].outer_flow_index);
							fc_db.vxlan_nicInfo_customized_entry_isSet = 0;
						}
						
					}
#endif

				}
				
			}
			
			

			for(i = 0 ; i < 4 ; i++)
			{
				if(fc_db.vxlan_downStream_record[i].isSet ==1 && flowIdx == fc_db.vxlan_downStream_record[i].inner_flow_index)
				{
#if defined(CONFIG_FC_CA8277B_SERIES)

					TABLE("DELETE VXLAN down stream inner flow");
					_rtk_fc_vxlan_8277b_reservedACL_del_by_idx(i, 0);

					fc_db.vxlan_downStream_record[i].isSet = 0 ;
					// delete outer flow
					if(fc_db.vxlan_downStream_record[i].outer_flow_index!=0)
						rtk_fc_flow_delete(fc_db.vxlan_downStream_record[i].outer_flow_index);
					fc_db.vxlan_downStream_record[i].inner_flow_index = 0;
					fc_db.vxlan_downStream_record[i].outer_flow_index = 0;
					fc_db.vxlan_downStream_record[i].cpuPort = 0;
				
					break;
#elif defined(CONFIG_FC_RTL9607C_SERIES)
					TABLE("DELETE VXLAN down stream inner flow");
					if(i==0)  // Normal
					{
						fc_db.vxlan_downStream_record[i].isSet = 0 ;
						// delete outer flow
						if(fc_db.vxlan_downStream_record[i].outer_flow_index!=0)
							rtk_fc_flow_delete(fc_db.vxlan_downStream_record[i].outer_flow_index);
						fc_db.vxlan_downStream_record[i].inner_flow_index = 0;
						fc_db.vxlan_downStream_record[i].outer_flow_index = 0;
						fc_db.vxlan_downStream_record[i].cpuPort = 0;
						fc_db.vxlan_downstream_port0_nicInfo_customized_entry_isSet = 0;
					}
					else	// extra, may share outer flow
					{
						// Don't delete flow, only when all down stream 's isSet is 0.
						TABLE("[VXLAN]Delete extra inner flow: %d to port: %d",fc_db.vxlan_downStream_record[i].inner_flow_index, i);
						
						fc_db.vxlan_downStream_record[i].isSet = 0 ;
						if(fc_db.vxlan_downStream_record[1].isSet ==0 && fc_db.vxlan_downStream_record[2].isSet ==0 && fc_db.vxlan_downStream_record[3].isSet ==0)
						{
							TABLE("[VXLAN]EXTRA: All extra inner isSet = 0, now try to del all downstream flow");
							rtk_fc_flow_delete(fc_db.vxlan_downStream_record[i].outer_flow_index);
							fc_db.vxlan_downstream_extra_nicInfo_customized_entry_isSet = 0;
							
							fc_db.vxlan_downStream_record[1].outer_flow_index=0;// 1 stands for port 1, port 1~3 = extra, port 0 = normal
							fc_db.vxlan_downStream_record[2].outer_flow_index=0;// 2 stands for port 2, port 1~3 = extra, port 0 = normal
							fc_db.vxlan_downStream_record[3].outer_flow_index=0;// 3 stands for port 3, port 1~3 = extra, port 0 = normal
							
							if(flowIdx != fc_db.vxlan_downStream_record[1].inner_flow_index && //prevent same index delete flow twice
								fc_db.vxlan_downStream_record[1].inner_flow_index!=0)
							{
								rtk_fc_flow_delete(fc_db.vxlan_downStream_record[1].inner_flow_index);
								fc_db.vxlan_downStream_record[1].inner_flow_index = 0;
							}
							else
								fc_db.vxlan_downStream_record[1].inner_flow_index = 0;

								
							if(flowIdx != fc_db.vxlan_downStream_record[2].inner_flow_index && //prevent same index delete flow twice
								fc_db.vxlan_downStream_record[2].inner_flow_index!=0)
							{
								rtk_fc_flow_delete(fc_db.vxlan_downStream_record[2].inner_flow_index);
								fc_db.vxlan_downStream_record[2].inner_flow_index = 0;
							}
							else
								fc_db.vxlan_downStream_record[2].inner_flow_index = 0;
							
							if(flowIdx != fc_db.vxlan_downStream_record[3].inner_flow_index && //prevent same index delete flow twice
								fc_db.vxlan_downStream_record[3].inner_flow_index!=0)
							{
								rtk_fc_flow_delete(fc_db.vxlan_downStream_record[3].inner_flow_index);
								fc_db.vxlan_downStream_record[3].inner_flow_index = 0;
							}
							else
								fc_db.vxlan_downStream_record[3].inner_flow_index = 0;
						}
						else
						{
							TABLE("[VXLAN][DS]Try to delete inner flow, Set isSet to 0 first!");
							return 1;
						}
						
					}
					
#endif

				}
			}
			//----------------------------------------------------------------------------------------------
		}

	}
	else
	{
		if(fc_db.controlFuc.special_fast_forward_mode==1 && FLOW_INFO_IS_SET(pFlowTable, FLOW_INFO_SOFTWARE_ONLY) !=1)
		{
			// In case of delete outer flow first
	   	
			for(i = 0 ; i < 4 ; i++)
			{
				if(fc_db.vxlan_downStream_record[i].isSet ==1 && flowIdx == fc_db.vxlan_downStream_record[i].outer_flow_index)
				{
#if defined(CONFIG_FC_CA8277B_SERIES)
					TABLE("DELETE VXLAN down stream outer flow");
					fc_db.vxlan_downStream_record[i].outer_flow_index = 0;
					if(fc_db.vxlan_downStream_record[i].inner_flow_index!=0)
						rtk_fc_flow_delete(fc_db.vxlan_downStream_record[i].inner_flow_index);
					break;
#elif defined(CONFIG_FC_RTL9607C_SERIES)
					TABLE("[VXLAN]Try to delete VXLAN down stream outer flow! But Don't delete! Must delete inner flow first!");
					return 1;
#endif


				}
			}
			for(i = 0 ; i < 4 ; i++)
			{
				if(fc_db.vxlan_upStream_record[i].isSet ==1 && flowIdx == fc_db.vxlan_upStream_record[i].outer_flow_index)
				{
#if defined(CONFIG_FC_CA8277B_SERIES)
					TABLE("DELETE VXLAN up stream outer flow");
					fc_db.vxlan_upStream_record[i].outer_flow_index = 0;
					if(fc_db.vxlan_upStream_record[i].inner_flow_index!=0)
						rtk_fc_flow_delete(fc_db.vxlan_upStream_record[i].inner_flow_index);
					break;
#elif defined(CONFIG_FC_RTL9607C_SERIES)
					TABLE("[VXLAN]Try to delete VXLAN up stream outer flow! But Don't delete! Must delete inner flow first!");
					return 1;
#endif

				}
			}
		}
	}
	
#endif	
	//---------------------------------------------- END OF VXLAN RELATED---------------------------------------------------
	return 0;
}

#endif
#if 0
int rtk_fc_vxlan_get_innerNetif_index(rtk_fc_pktHdr_t *pPktHdr, int direction)
{

	//int ret = SUCCESS;
	int i ,netifidx = -1;
	if(direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
	{
		for(i = 0; i< RTK_FC_TABLESIZE_INTF_SW; i++)
		{
			if(fc_db.netifTbl[i].intf.valid==FALSE)  continue;
			//printk("fc_db.netifTbl[%d].vxlan_vni = %d (%d) pPktHdr->cvlanid = %d (%d)\n", i , fc_db.netifTbl[i].vxlan_vni, ntohl(pPktHdr->vxlan_info.vxlanHdr->vx_vni)>>8, pPktHdr->cvlanid, fc_db.netifTbl[i].vlanId );
			if( (fc_db.netifTbl[i].dualUniInfo.vxlan_netif_info.vxlan_vni == ntohl(pPktHdr->vxlan_info.vxlanHdr->vx_vni)>>8) && pPktHdr->cvlanid == fc_db.netifTbl[i].vlanId )
			{
				netifidx = i;
				break;
			}
		}

	}
	else
	{
		for(i = 0; i< RTK_FC_TABLESIZE_INTF_SW; i++)
		{
			if(fc_db.netifTbl[i].intf.valid==FALSE)  continue;
			//printk("fc_db.netifTbl[%d].vxlan_vni = %d (%d) pPktHdr->cvlanid = %d (%d)\n", i , fc_db.netifTbl[i].vxlan_vni, ntohl(pPktHdr->vxlan_info.vxlanHdr->vx_vni)>>8, pPktHdr->cvlanid, fc_db.netifTbl[i].vlanId );
			if( (fc_db.netifTbl[i].dualUniInfo.vxlan_netif_info.vxlan_vni == ntohl(pPktHdr->vxlan_info.vxlanHdr->vx_vni)>>8) && pPktHdr->cvlanid == fc_db.netifTbl[i].vlanId )
			{
				netifidx = i;
				break;
			}
		}

	if(netifidx == SIGNED_INVALID)
		return RTK_FC_RET_ERR_ENTRY_NOT_FOUND;
	else {
		pPktHdr->vxlan_info.inner_intf_index = netifidx;
		return RTK_FC_RET_OK;
	}
}

rtk_fc_ret_t rtk_fc_vxlan_get_outerNetif_index(rtk_fc_pktHdr_t *pPktHdr, int direction)
{
	int i ,netifidx = SIGNED_INVALID;

	if(direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
	{
		for(i = 0; i< RTK_FC_TABLESIZE_INTF_SW; i++)
		{
			
			if(fc_db.netifTbl[i].intf.valid==FALSE)  continue;
			//printk("netif mac: %pM pkthdr mac: %pM  netif-ip: %x pkt-ip: %x\n",&fc_db.netifTbl[i].intf.gateway_mac_addr.octet, &pPktHdr->outer_eth->h_source, fc_db.netifTbl[i].intf.gateway_ipv4_addr,ntohl(pPktHdr->outer_iph->saddr));
			if( !memcmp(&fc_db.netifTbl[i].intf.gateway_mac_addr.octet, &pPktHdr->outer_eth->h_source, ETH_ALEN) )
			{
				if(pPktHdr->vxlan_info.outer_isV6)
				{
					if(RTK_FC_HELPER_NETDEV_COMPARE_V6ADDR_BY_V6ADDR(fc_db.netifTbl[i].dev, &pPktHdr->outer_ip6h->saddr)==SUCCESS )
					{	
						netifidx = i;
						break;
					}
				}
				else
				{
					if(fc_db.netifTbl[i].intf.gateway_ipv4_addr==ntohl(pPktHdr->outer_iph->saddr))
					{
						netifidx = i;
						break;
					}
				}
			}
		}
	}
	else
	{
		for(i = 0; i< RTK_FC_TABLESIZE_INTF_SW; i++)
		{
			
			if(fc_db.netifTbl[i].intf.valid==FALSE)  continue;
			//printk("netif mac: %pM pkthdr mac: %pM  netif-ip: %x pkt-ip: %x\n",&fc_db.netifTbl[i].intf.gateway_mac_addr.octet, &pPktHdr->outer_eth->h_source, fc_db.netifTbl[i].intf.gateway_ipv4_addr,ntohl(pPktHdr->outer_iph->saddr));
			
			if( !memcmp(&fc_db.netifTbl[i].intf.gateway_mac_addr.octet, &pPktHdr->outer_eth->h_dest, ETH_ALEN) )
			{
				if(pPktHdr->vxlan_info.outer_isV6)
				{
					if(RTK_FC_HELPER_NETDEV_COMPARE_V6ADDR_BY_V6ADDR(fc_db.netifTbl[i].dev, &pPktHdr->outer_ip6h->daddr)==SUCCESS )
					{	
						netifidx = i;
						break;
					}
				}
				else
				{
					if(fc_db.netifTbl[i].intf.gateway_ipv4_addr==ntohl(pPktHdr->outer_iph->daddr))
					{
						netifidx = i;
						break;
					}
				}
			}
		}
	}
	
	if(netifidx == SIGNED_INVALID)
		return RTK_FC_RET_ERR_ENTRY_NOT_FOUND;
	else {
		pPktHdr->vxlan_info.outer_intf_index = netifidx;
		return RTK_FC_RET_OK;
	}
}
#endif

int _rtk_fc_vxlan_downstream_pktHdr_recover(rtk_fc_pktHdr_t *pPktHdr, int sw_inner_inf_idx, int sw_outer_intf_idx)
{

	if(fc_db.netifTbl[sw_inner_inf_idx].dualType != RTK_FC_DUALTYPE_VXLAN)
	{
		DEBUG("interface[%d] is not vxlan dual type!",sw_inner_inf_idx);
		return FAILED;
	}
	
	//For shortCut usage
	pPktHdr->vxlan_info.outerTag_off = fc_db.netifTbl[sw_inner_inf_idx].dualUniInfo.vxlan_netif_info.outer_tag_len;
	pPktHdr->vxlan_info.outer_isV6 = fc_db.netifTbl[sw_inner_inf_idx].dualUniInfo.vxlan_netif_info.outer_is_v6;
	pPktHdr->vxlan_info.outer_svh = fc_db.netifTbl[sw_inner_inf_idx].dualUniInfo.vxlan_netif_info.outer_stag_if;
	pPktHdr->vxlan_info.outer_svlanid = fc_db.netifTbl[sw_inner_inf_idx].dualUniInfo.vxlan_netif_info.outer_svlan_id;
	pPktHdr->vxlan_info.outer_cvh = fc_db.netifTbl[sw_inner_inf_idx].dualUniInfo.vxlan_netif_info.outer_ctag_if;
	pPktHdr->vxlan_info.outer_cvlanid = fc_db.netifTbl[sw_inner_inf_idx].dualUniInfo.vxlan_netif_info.outer_cvlan_id;
	pPktHdr->vxlan_info.outer_ppph = fc_db.netifTbl[sw_inner_inf_idx].dualUniInfo.vxlan_netif_info.outer_pppoe_tag;
	pPktHdr->vxlan_info.vni = fc_db.netifTbl[sw_inner_inf_idx].dualUniInfo.vxlan_netif_info.vxlan_vni;
	
	memcpy(&pPktHdr->vxlan_info.vxlan_DA[0], &fc_db.netifTbl[sw_inner_inf_idx].dualUniInfo.vxlan_netif_info.vxlan_DA[0], 6);
	DEBUG("[VXLAN]pPktHdr->vxlan_info.outerTag_off = %d pPktHdr->vxlan_info.vni = %d",pPktHdr->vxlan_info.outerTag_off,pPktHdr->vxlan_info.vni);
	if(pPktHdr->vxlan_info.outer_isV6)
	{
		memcpy(&pPktHdr->vxlan_info.gateway_v6_ip , &fc_db.netifTbl[sw_inner_inf_idx].dualUniInfo.vxlan_netif_info.outer_v6Ip, sizeof(struct in6_addr));
		memcpy(&pPktHdr->vxlan_info.remote_v6_ip , &fc_db.netifTbl[sw_inner_inf_idx].dualUniInfo.vxlan_netif_info.outer_remote_v6Ip, sizeof(struct in6_addr));
	}
	else
	{
		pPktHdr->vxlan_info.gateway_v4_ip = fc_db.netifTbl[sw_inner_inf_idx].dualUniInfo.vxlan_netif_info.outer_ip;
		pPktHdr->vxlan_info.remote_v4_ip = fc_db.netifTbl[sw_inner_inf_idx].dualUniInfo.vxlan_netif_info.outer_remote_ip;
	}
	return SUCCESS;

}


#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
int _rtk_fc_vxlan_8277C_downStream_reservedACL_add(int *result_idx, rtk_fc_aclAndCf_reserved_dual_hdr_chk_t *dual_hdr_chk)
{

	int ret;
		
	ret = _rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CLS_TYPE_DUAL_HEADER_DS_CHECK, dual_hdr_chk);

	if(ret != RTK_FC_RET_OK)
	{
		WARNING("add special ACL DUAL_HEADER_DS_CHECK failed!!! ret = %x", ret);
		return RTK_FC_RET_ERR;
	}
	else
	{
		DEBUG("add special ACL DUAL_HEADER_DS_CHECK success, idx: %x",dual_hdr_chk->rslt_idx);
		*result_idx = dual_hdr_chk->rslt_idx;
	}
	//VXLAN-TODO: TTL -1 cls?

	return RTK_FC_RET_OK;
}


int rtk_fc_vxlan_get_outer_content(int outer_header_size, rtk_fc_pktHdr_t *pPktHdr)
{
	int offset = 0;
	struct udphdr tmp_outer_udph;
	u8 *outer_header_content = &pPktHdr->vxlan_info.outer_content[0];
	if(pPktHdr == NULL)
		return FAILED;
	


	// DMAC(6)+SMAC(6)+[SVLAN(4)+CVLAN(4)]+Ethertype(2)+[PPPoE(8)]+IP header(20/40)+UDP(8)+VxLAN(8)
	memset(outer_header_content, 0, outer_header_size);
	
	//outer l2
	memcpy(outer_header_content+offset, pPktHdr->outer_eth, sizeof(struct ethhdr));
	offset += sizeof(struct ethhdr);

	//outer svlan
	if(pPktHdr->outer_svh)
	{
		memcpy(outer_header_content+offset, pPktHdr->outer_svh, sizeof(struct vlan_hdr));
		offset += sizeof(struct vlan_hdr);
	}
	

	//outer cvlan
	if(pPktHdr->outer_cvh)
	{
		memcpy(outer_header_content+offset, pPktHdr->outer_cvh, sizeof(struct vlan_hdr));
		offset += sizeof(struct vlan_hdr);
	}
	

	//outer pppoe
	if(pPktHdr->outer_ppph)
	{
		memcpy(outer_header_content+offset, pPktHdr->outer_ppph, sizeof(struct pppoe_hdr));
		
		if(pPktHdr->outer_iph)
		{
				outer_header_content[offset + sizeof(struct pppoe_hdr)] = 0x00;
				outer_header_content[offset + sizeof(struct pppoe_hdr)+1] = 0x21;
		}
		else if(pPktHdr->outer_ip6h)
		{
				outer_header_content[offset + sizeof(struct pppoe_hdr)] = 0x00;
				outer_header_content[offset + sizeof(struct pppoe_hdr)+1] = 0x57;
		
		}
		offset += PPPOE_SES_HLEN;
	}

	//outer ip
	if(pPktHdr->outer_iph)
	{
		memcpy(outer_header_content + offset, pPktHdr->outer_iph, sizeof(struct iphdr));

		offset += sizeof(struct iphdr);
	}
	else if(pPktHdr->outer_ip6h)
	{
		memcpy(outer_header_content + offset, pPktHdr->outer_ip6h, sizeof(struct ipv6hdr));
		offset += sizeof(struct ipv6hdr);
	}
	
	//outer udp
	
	memcpy(&tmp_outer_udph, pPktHdr->outer_udph, sizeof(struct udphdr));
	tmp_outer_udph.check = 0;
	TRACE("Set outer udp checksum = 0!");
	//memcpy(outer_header_content+offset, pPktHdr->outer_udph, sizeof(struct udphdr));
	memcpy(outer_header_content+offset, &tmp_outer_udph, sizeof(struct udphdr));
	offset += sizeof(struct udphdr);

	//outer vxlan
	memcpy(outer_header_content+offset, pPktHdr->vxlan_info.vxlanHdr, sizeof(rtk_fc_vxlanhdr_t));
	offset += sizeof(rtk_fc_vxlanhdr_t);



	return SUCCESS;
}

int rtk_fc_vxlan_8277C_upstream_flow_setting(rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, int igrNetifIdx, int egrNetifIdx, rtk_fc_pktHdr_t *pPktHdr, bool swOnly)
{
	int ret=RTK_FC_RET_OK;
		
	//VXLAN-TODO:upstream
	//pG3IgrExtraInfo->dualHdrType					= RTK_FC_L2DUAL_ACT_ADD;
	pG3IgrExtraInfo->dualHdrType					= RTK_FC_DUALTYPE_VXLAN;
	pG3IgrExtraInfo->direction						= RTK_FC_FLOW_DIRECTION_UPSTREAM;
	pG3IgrExtraInfo->vxlan_info.outer_srcPort 		= ntohs(pPktHdr->outer_udph->source);

	return ret;

}

int rtk_fc_vxlan_8277C_downstream_flow_setting(rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, int igrNetifIdx, int egrNetifIdx, rtk_fc_ingress_data_t *pFcIngressData, rtk_fc_pktHdr_t *pPktHdr, bool swOnly)
{
	int ret=RTK_FC_RET_OK;

	if(fc_db.netifTbl[NETIF_HWTOSW(igrNetifIdx)].dualType != RTK_FC_DUALTYPE_VXLAN)
	{
		WARNING("VXLAN downstream ingress inner interface netif[%d] is not vxlan type!  ", NETIF_HWTOSW(igrNetifIdx));
		return FAILED;
	}
	else if(fc_db.netifTbl[NETIF_HWTOSW(igrNetifIdx)].dualUniInfo.vxlan_netif_info.valid != 1)
	{
		WARNING("VXLAN downstream vxlan interface netif[%d] info is not complete!  ", igrNetifIdx);
		return FAILED;
	}
	
	
	//VXLAN-TODO: pG3IgrExtraInfo setting

	pG3IgrExtraInfo->dualHdrType					= RTK_FC_DUALTYPE_VXLAN;
	pG3IgrExtraInfo->direction						= RTK_FC_FLOW_DIRECTION_DOWNSTREAM;

	

	return ret;
}



#endif



									

