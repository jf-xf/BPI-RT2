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


#include <linux/if_pppox.h>
#include <net/ip.h>

#include "rtk_fc_dualHeader.h"
#include "rtk_fc_internal.h"
#include "rtk_fc_callback.h"
#include "rtk_fc_debug.h"
#include "rtk_fc_mappingAPI.h"
#include "rtk_fc_external.h"
#include "rtk_fc_vxlan.h"
#include "rtk_fc_acl.h"
#include <net/ip6_tunnel.h>

#define EXTRATAG_BUFFER_OFFSET 		RTK_FC_DUAL_CONTENT_BUF_ENT_OFFSET	// offset per each content extra tag action list
#define EXTRATAG_LEN				40	// outer header length for PPTP/L2TP/DSLite
#define V6RD_EXTRATAG_LEN			20	// outer header length for 6RD

int _rtk_fc_fragIPID_save(int swNetifIdx, uint16 oriIPID, uint16 newIPID)
{
	int i = 0, freeEntIdx = -1, hitEntIdx = -1, lruEntIdx = -1;
	uint16 lruIPID = 65535;
	rtk_fc_devGwMac_t *egressIntf = &fc_db.devGwMacTbl[swNetifIdx];

	for(i = 0; i < MAX_SW_FRAGIPID_SIZE; i++){
		if((freeEntIdx==-1)&& (!egressIntf->psFragIPID[i].valid))
			freeEntIdx = i;

		if((egressIntf->psFragIPID[i].valid) && (egressIntf->psFragIPID[i].oriIPID == oriIPID)){
			hitEntIdx = i;
			break;
		}

		if((egressIntf->psFragIPID[i].valid) && (lruIPID>egressIntf->psFragIPID[i].oriIPID)){
			lruEntIdx = i;
			lruIPID = egressIntf->psFragIPID[i].oriIPID;
		}

	}

	if(hitEntIdx!=-1){
		TRACE("netif[%d] duplicate ipid was found, entryidx: %d, IPID: %d, new IPID %d", swNetifIdx, hitEntIdx, egressIntf->psFragIPID[hitEntIdx].oriIPID, newIPID);
	}else if(freeEntIdx!=-1){
		hitEntIdx = freeEntIdx;
		TRACE("netif[%d] new frag packet, entryidx: %d, IPID: %d, new IPID %d", swNetifIdx, hitEntIdx, egressIntf->psFragIPID[hitEntIdx].oriIPID, newIPID);
	}else{
		hitEntIdx = lruEntIdx;
		TRACE("netif[%d] ipid records are full, replace entryidx %d, IPID: %d, new IPID %d", swNetifIdx, lruEntIdx, egressIntf->psFragIPID[hitEntIdx].oriIPID, newIPID);
	}

	egressIntf->psFragIPID[hitEntIdx].valid = TRUE;
	egressIntf->psFragIPID[hitEntIdx].oriIPID = oriIPID;
	egressIntf->psFragIPID[hitEntIdx].newIPID = newIPID;

	return SUCCESS;
}


int _rtk_fc_fragIPID_delete(int swNetifIdx, uint16 oriIPID, uint16 *newIPID)
{
	int i = 0, hitEntIdx = -1;
	rtk_fc_devGwMac_t *egressIntf = &fc_db.devGwMacTbl[swNetifIdx];

	for(i = 0; i < MAX_SW_FRAGIPID_SIZE; i++){
		if((egressIntf->psFragIPID[i].valid) && (egressIntf->psFragIPID[i].oriIPID == oriIPID)){
			hitEntIdx = i;
			break;
		}
	}

	if(hitEntIdx!=-1){
		TRACE("netif[%d] duplicate ipid was found, idx: %d, IPID: %d", swNetifIdx, hitEntIdx, egressIntf->psFragIPID[hitEntIdx].oriIPID);
	}else{
		TRACE("netif[%d] ipid records was not found, IPID: %d", swNetifIdx, oriIPID);
		return FAILED;
	}

	*newIPID = egressIntf->psFragIPID[hitEntIdx].newIPID;

	egressIntf->psFragIPID[hitEntIdx].valid = FALSE;
	egressIntf->psFragIPID[hitEntIdx].oriIPID = 0;
	egressIntf->psFragIPID[hitEntIdx].newIPID = 0;

	return SUCCESS;

}

int _rtk_fc_fragIPID_get(int swNetifIdx, uint16 oriIPID, uint16 *newIPID)
{
	int i = 0, hitEntIdx = -1;
	rtk_fc_devGwMac_t *egressIntf = &fc_db.devGwMacTbl[swNetifIdx];

	for(i = 0; i < MAX_SW_FRAGIPID_SIZE; i++){
		if((egressIntf->psFragIPID[i].valid) && (egressIntf->psFragIPID[i].oriIPID == oriIPID)){
			hitEntIdx = i;
			break;
		}
	}

	if(hitEntIdx!=-1){
		TRACE("netif[%d] duplicate ipid was found, idx: %d, IPID: %d", swNetifIdx, hitEntIdx, egressIntf->psFragIPID[hitEntIdx].oriIPID);
	}else{
		TRACE("netif[%d] ipid records was not found, IPID: %d", swNetifIdx, oriIPID);
		return FAILED;
	}

	*newIPID = egressIntf->psFragIPID[hitEntIdx].newIPID;

	return SUCCESS;
}


static int _rtk_fc_extraTagActionList_empty_find(int intfHwIdx)
{
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	/* return dual control index */
	if ((intfHwIdx < RTK_FC_DUAL_NETIF_START_IDX) || (RTK_FC_TABLESIZE_INTF <= intfHwIdx))
	{
		WARNING("Invalid HW netif index: %d", intfHwIdx);
		return SIGNED_INVALID;
	}
	// one to one mapping to netif
	return RTK_FC_DUAL_CONTROL_IDX(intfHwIdx);
#else
	//NOT (CONFIG_FC_RTL8277C_SERIES || CONFIG_FC_RTL9607F_SERIES)
	/* return 1~7 for action list idx */
	int i ;
	for(i = RTK_FC_TABLESIZE_EXTRATAG_LISTMIN; i < RTK_FC_TABLESIZE_EXTRATAG; i++)
	{
		if(fc_db.extraTagList[i].valid==0)
			return i;
	}
	WARNING("NO empty action list could be used!");
	return SIGNED_INVALID;
#endif
}



static int _rtk_fc_extraTag_setupPPTP(int wan_intf_idx, uint8 *outerHdr, rtk_fc_tunnelInfo_t *tunnelInfo, int *extraTagActListIdx)
{
	rtk_rg_asic_extraTagAction_t extraTagAction;
	int extraTagIdx = 0, actSeq = 0;
	int TUNN_tag_len=EXTRATAG_LEN;	//IP(20)+GRE(16)+PPP(4) for PPTP

	//correct GRE header length here
	if((tunnelInfo->tunnelTag&GRESEQ_TAGIF)==0)TUNN_tag_len-=4;
	if((tunnelInfo->tunnelTag&GREACK_TAGIF)==0)TUNN_tag_len-=4;
	TUNN_tag_len-=4;
	TUNN_tag_len+=tunnelInfo->pppMode;

	// get extraTagIdx by interface
	extraTagIdx = _rtk_fc_extraTagActionList_empty_find(wan_intf_idx);
	if(extraTagIdx==SIGNED_INVALID) 
	{
		*extraTagActListIdx=SIGNED_INVALID;
		return FAIL;
	}

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type1.act_bit = FB_EXTG_ACTBIT_1;
	extraTagAction.type1.length = TUNN_tag_len;
	extraTagAction.type1.src_addr_offset = 0 + (EXTRATAG_BUFFER_OFFSET * (RTK_FC_EXTRATAG_IDX_TO_SRC_ADDR_OFFSET(extraTagIdx)));
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type2.act_bit = FB_EXTG_ACTBIT_2;
	extraTagAction.type2.ethertype = 0x0800;
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type3.act_bit = FB_EXTG_ACTBIT_3;
	extraTagAction.type3.pkt_buff_offset = 2;
	extraTagAction.type3.length = 2;
	extraTagAction.type3.value = TUNN_tag_len;
	extraTagAction.type3.operation = 0;
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type4.act_bit = FB_EXTG_ACTBIT_4;
	extraTagAction.type4.pkt_buff_offset = 4;
	extraTagAction.type4.data_src_type = 1;						// 1:IP ID;
	extraTagAction.type4.seq_ack_reg_idx = wan_intf_idx;
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type5.act_bit = FB_EXTG_ACTBIT_5;
	extraTagAction.type6.pkt_buff_offset = 10;
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type3.act_bit = FB_EXTG_ACTBIT_3;
	extraTagAction.type3.pkt_buff_offset = 24;
	extraTagAction.type3.length = 2;
	extraTagAction.type3.value = tunnelInfo->pppMode;
	extraTagAction.type3.operation = 0;
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type4.act_bit = FB_EXTG_ACTBIT_4;
	extraTagAction.type4.pkt_buff_offset = 28;
	extraTagAction.type4.data_src_type = 0;
	extraTagAction.type4.reduce_seq = !((tunnelInfo->tunnelTag&GRESEQ_TAGIF)>0);
	extraTagAction.type4.reduce_ack = !((tunnelInfo->tunnelTag&GREACK_TAGIF)>0);
	extraTagAction.type4.seq_ack_reg_idx = wan_intf_idx;
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	//memDump(outerHdr, TUNN_tag_len, "Content Buffer - Before");
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGCONTENTBUFFER_SET(extraTagIdx, outerHdr), SUCCESS);
	//memDump(outerHdr, TUNN_tag_len, "Content Buffer - After");

	*extraTagActListIdx = extraTagIdx;

	return SUCCESS;
}

static int _rtk_fc_extraTag_setupL2TP(int wan_intf_idx, uint8 *outerHdr, int *extraTagActListIdx)
{
	rtk_rg_asic_extraTagAction_t extraTagAction;
	int extraTagIdx = 0, actSeq = 0;
	int TUNN_tag_len=EXTRATAG_LEN;	//IP(20)+UDP(8)+L2TP(8)+PPP(4) for L2TP

	// get extraTagIdx by interface
	extraTagIdx = _rtk_fc_extraTagActionList_empty_find(wan_intf_idx);
	if(extraTagIdx==SIGNED_INVALID) 
	{
		*extraTagActListIdx=SIGNED_INVALID;
		return FAIL;
	}


	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type1.act_bit = FB_EXTG_ACTBIT_1;
	extraTagAction.type1.length = TUNN_tag_len;
	extraTagAction.type1.src_addr_offset = 0 + (EXTRATAG_BUFFER_OFFSET * (RTK_FC_EXTRATAG_IDX_TO_SRC_ADDR_OFFSET(extraTagIdx)));
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type2.act_bit = FB_EXTG_ACTBIT_2;
	extraTagAction.type2.ethertype = 0x0800;
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type3.act_bit = FB_EXTG_ACTBIT_3;
	extraTagAction.type3.pkt_buff_offset = 2;
	extraTagAction.type3.length = 2;
	extraTagAction.type3.value = TUNN_tag_len;
	extraTagAction.type3.operation = 0;
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type4.act_bit = FB_EXTG_ACTBIT_4;
	extraTagAction.type4.pkt_buff_offset = 4;
	extraTagAction.type4.data_src_type = 1;						// 1:IP ID;
	extraTagAction.type4.seq_ack_reg_idx = wan_intf_idx;
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type5.act_bit = FB_EXTG_ACTBIT_5;
	extraTagAction.type6.pkt_buff_offset = 10;
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type3.act_bit = FB_EXTG_ACTBIT_3;
	extraTagAction.type3.pkt_buff_offset = 24;
	extraTagAction.type3.length = 2;
	extraTagAction.type3.value = TUNN_tag_len-20;		// 20: ipv4 header length
	extraTagAction.type3.operation = 0;
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type3.act_bit = FB_EXTG_ACTBIT_3;
	extraTagAction.type3.pkt_buff_offset = 30;
	extraTagAction.type3.length = 2;
	extraTagAction.type3.value = TUNN_tag_len-20-8;		//20: ipv4 header length; 8: udp header length
	extraTagAction.type3.operation = 0;
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type6.act_bit = FB_EXTG_ACTBIT_6;
	extraTagAction.type6.pkt_buff_offset = 26;
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);


	//memDump(outerHdr, TUNN_tag_len, "Content Buffer - Before");
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGCONTENTBUFFER_SET(extraTagIdx, outerHdr), SUCCESS);
	//memDump(outerHdr, TUNN_tag_len, "Content Buffer - After");

	*extraTagActListIdx = extraTagIdx;

	return SUCCESS;
}

static int _rtk_fc_extraTag_setupIP4InIP6(int wan_intf_idx, uint8 *outerHdr, int *extraTagActListIdx)
{
	rtk_rg_asic_extraTagAction_t extraTagAction;
	int extraTagIdx = 0, actSeq = 0;
	int TUNN_tag_len=EXTRATAG_LEN;	//IPv6(40)

	// get extraTagIdx by interface
	extraTagIdx = _rtk_fc_extraTagActionList_empty_find(wan_intf_idx);
	if(extraTagIdx==SIGNED_INVALID) 
	{
		*extraTagActListIdx=SIGNED_INVALID;
		return FAIL;
	}


	//insert 40 length
	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type1.act_bit = FB_EXTG_ACTBIT_1;
	extraTagAction.type1.length = TUNN_tag_len;
	extraTagAction.type1.src_addr_offset = 0 + (EXTRATAG_BUFFER_OFFSET * (RTK_FC_EXTRATAG_IDX_TO_SRC_ADDR_OFFSET(extraTagIdx)));
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	//change ethertype to 0x86dd
	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type2.act_bit = FB_EXTG_ACTBIT_2;
	extraTagAction.type2.ethertype = 0x86dd;
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	//
	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type3.act_bit = FB_EXTG_ACTBIT_3;
	extraTagAction.type3.pkt_buff_offset = 4;
	extraTagAction.type3.length = 2;
	extraTagAction.type3.value = 0;
	extraTagAction.type3.operation = 0;
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	//memDump(outerHdr, TUNN_tag_len, "Content Buffer - Before");
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGCONTENTBUFFER_SET(extraTagIdx, outerHdr), SUCCESS);
	//memDump(outerHdr, TUNN_tag_len, "Content Buffer - After");

	*extraTagActListIdx = extraTagIdx;

	return SUCCESS;
}

static int _rtk_fc_extraTag_setup6RD(int wan_intf_idx, uint8 *outerHdr, int *extraTagActListIdx)
{
	rtk_rg_asic_extraTagAction_t extraTagAction;
	int extraTagIdx = 0, actSeq = 0;
	int TUNN_tag_len=V6RD_EXTRATAG_LEN;	//IPv4(20)

	// get extraTagIdx by interface
	extraTagIdx = _rtk_fc_extraTagActionList_empty_find(wan_intf_idx);
	if(extraTagIdx==SIGNED_INVALID) 
	{
		*extraTagActListIdx=SIGNED_INVALID;
		return FAIL;
	}


	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type1.act_bit = FB_EXTG_ACTBIT_1;
	extraTagAction.type1.length = TUNN_tag_len;
	extraTagAction.type1.src_addr_offset = 0 + (EXTRATAG_BUFFER_OFFSET * (RTK_FC_EXTRATAG_IDX_TO_SRC_ADDR_OFFSET(extraTagIdx)));
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type2.act_bit = FB_EXTG_ACTBIT_2;
	extraTagAction.type2.ethertype = 0x0800;
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type3.act_bit = FB_EXTG_ACTBIT_3;
	extraTagAction.type3.pkt_buff_offset = 2;
	extraTagAction.type3.length = 2;
	extraTagAction.type3.value = 20; //IPv4 header length
	extraTagAction.type3.operation = 0;
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type4.act_bit = FB_EXTG_ACTBIT_4;
	extraTagAction.type4.pkt_buff_offset = 4;
	extraTagAction.type4.data_src_type = 1;						// 1:IP ID;
	extraTagAction.type4.seq_ack_reg_idx = wan_intf_idx;
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	//memDump(outerHdr, TUNN_tag_len, "Content Buffer - Before");
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGCONTENTBUFFER_SET(extraTagIdx, outerHdr), SUCCESS);
	//memDump(outerHdr, TUNN_tag_len, "Content Buffer - After");

	*extraTagActListIdx = extraTagIdx;

	return SUCCESS;
}

static int _rtk_fc_extraTag_setupVXLAN(int hw_intf_idx, uint8 *outerHdr, int *extraTagActListIdx, int outerHdrLen, int outer_pppoe_tag_off, int outer_ip_tag_off, int outer_udp_tag_off, bool outer_is_v6)
{
	rtk_rg_asic_extraTagAction_t extraTagAction;
	int extraTagIdx = 0, actSeq = 0;
	
	// get extraTagIdx by interface
	extraTagIdx = _rtk_fc_extraTagActionList_empty_find(hw_intf_idx);
	if(extraTagIdx==SIGNED_INVALID) 
	{
		*extraTagActListIdx=SIGNED_INVALID;
		return FAIL;
	}


	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type1.act_bit = FB_EXTG_ACTBIT_1;
	extraTagAction.type1.length = outerHdrLen;
	extraTagAction.type1.src_addr_offset = 0 + (EXTRATAG_BUFFER_OFFSET * (RTK_FC_EXTRATAG_IDX_TO_SRC_ADDR_OFFSET(extraTagIdx)));
	
	
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	if(outer_pppoe_tag_off)
	{
		// GRE eth BR outer contain PPPoE header, change PPP len
		bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
		extraTagAction.type3.act_bit = FB_EXTG_ACTBIT_3;
		extraTagAction.type3.pkt_buff_offset = outer_pppoe_tag_off + 4; //PPPoE len field offset
		extraTagAction.type3.length = 2;
		extraTagAction.type3.value = outerHdrLen - outer_pppoe_tag_off - PPPOE_SES_HLEN +2; // payload length includes p2p protocol(2 bytes)
		extraTagAction.type3.operation = 0;
		ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);
	}

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	if(outer_is_v6)
	{
		extraTagAction.type3.pkt_buff_offset = outer_ip_tag_off + 4; // IPv4 total_len field offset
		extraTagAction.type3.value = outerHdrLen - outer_udp_tag_off;
	}
	else
	{
		extraTagAction.type3.pkt_buff_offset = outer_ip_tag_off + 2; // IPv4 total_len field offset
		extraTagAction.type3.value = outerHdrLen - outer_ip_tag_off;
	}
	extraTagAction.type3.act_bit = FB_EXTG_ACTBIT_3;
	extraTagAction.type3.length = 2;
	extraTagAction.type3.operation = 0;
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type3.act_bit = FB_EXTG_ACTBIT_3;
	extraTagAction.type3.pkt_buff_offset = outer_udp_tag_off + 4; // UDP len field offset
	extraTagAction.type3.length = 2;
	extraTagAction.type3.value = outerHdrLen - outer_udp_tag_off;
	extraTagAction.type3.operation = 0;
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type7.act_bit = FB_EXTG_ACTBIT_7;
	extraTagAction.type7.outer_udp_offset = outer_udp_tag_off;
	extraTagAction.type7.outer_ppp_offset = outer_pppoe_tag_off;
	extraTagAction.type7.is_l2dual = 1;
	extraTagAction.type7.outer_ip_offset = outer_ip_tag_off;
	extraTagAction.type7.outer_tag_len = outerHdrLen;



	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	// No need to set IPv4 checksum update

	//memDump(outerHdr, outerHdrLen, "Content Buffer - Before");
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGCONTENTBUFFER_SET(extraTagIdx, outerHdr), SUCCESS);
	//memDump(outerHdr, outerHdrLen, "Content Buffer - After");
	
	

	*extraTagActListIdx = extraTagIdx;

	return SUCCESS;
}

static int _rtk_fc_extraTag_setupGreEthBr(int hw_intf_idx, uint8 *outerHdr, int *extraTagActListIdx, int outerHdrLen, int outer_pppoe_tag_off, int outer_ip_tag_off, bool outer_iph_is_v6)
{
	rtk_rg_asic_extraTagAction_t extraTagAction;
	int extraTagIdx = 0, actSeq = 0;

	// get extraTagIdx by interface
	extraTagIdx = _rtk_fc_extraTagActionList_empty_find(hw_intf_idx);
	if(extraTagIdx==SIGNED_INVALID) 
	{
		*extraTagActListIdx=SIGNED_INVALID;
		return FAIL;
	}

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type1.act_bit = FB_EXTG_ACTBIT_1;
	extraTagAction.type1.length = outerHdrLen;
	extraTagAction.type1.src_addr_offset = 0 + (EXTRATAG_BUFFER_OFFSET * (RTK_FC_EXTRATAG_IDX_TO_SRC_ADDR_OFFSET(extraTagIdx)));

	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	if(outer_pppoe_tag_off)
	{
		// GRE eth BR outer contain PPPoE header, change PPP len
		bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
		extraTagAction.type3.act_bit = FB_EXTG_ACTBIT_3;
		extraTagAction.type3.pkt_buff_offset = outer_pppoe_tag_off + 4; //PPPoE len field offset
		extraTagAction.type3.length = 2;
		extraTagAction.type3.value = outerHdrLen - outer_pppoe_tag_off - PPPOE_SES_HLEN +2; // payload length includes p2p protocol(2 bytes)
		extraTagAction.type3.operation = 0;
		ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);
	}

	if(outer_iph_is_v6)
	{
		bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
		extraTagAction.type3.act_bit = FB_EXTG_ACTBIT_3;
		extraTagAction.type3.pkt_buff_offset = outer_ip_tag_off + 4; // IPv6 Payload_len field offset
		extraTagAction.type3.length = 2;
		extraTagAction.type3.value = outerHdrLen - outer_ip_tag_off - 40;
		extraTagAction.type3.operation = 0;
		ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);
	}
	else
	{
		bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
		extraTagAction.type3.act_bit = FB_EXTG_ACTBIT_3;
		extraTagAction.type3.pkt_buff_offset = outer_ip_tag_off + 2; // IPv4 total_len field offset
		extraTagAction.type3.length = 2;
		extraTagAction.type3.value = outerHdrLen - outer_ip_tag_off;
		extraTagAction.type3.operation = 0;
		ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

		bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
		extraTagAction.type4.act_bit = FB_EXTG_ACTBIT_4;
		extraTagAction.type4.pkt_buff_offset = outer_ip_tag_off + 4;	// IPv4 IP_ID field offset
		extraTagAction.type4.data_src_type = 1;							// 1:IP ID;
		extraTagAction.type4.seq_ack_reg_idx = hw_intf_idx;
		ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

		// No need to set IPv4 checksum update
	}

	bzero(&extraTagAction, sizeof(rtk_rg_asic_extraTagAction_t));
	extraTagAction.type7.act_bit = FB_EXTG_ACTBIT_7;
	extraTagAction.type7.outer_ppp_offset = outer_pppoe_tag_off;
	extraTagAction.type7.outer_ip_offset = outer_ip_tag_off;
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_ADD(extraTagIdx, actSeq++, &extraTagAction), SUCCESS);

	//memDump(outerHdr, outerHdrLen, "Content Buffer - Before");
	ASSERT_EQ(RTK_RG_ASIC_EXTRATAGCONTENTBUFFER_SET(extraTagIdx, outerHdr), SUCCESS);
	//memDump(outerHdr, outerHdrLen, "Content Buffer - After");

	*extraTagActListIdx = extraTagIdx;

	return SUCCESS;
}

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
/* Setup ingress ACL for 6RD downstream*/
int rtk_fc_8277C_v6rd_downStream_reservedACL_add(int igrIntfIdx, rtk_fc_ingress_data_t *pFcIngressData, int *result_idx)
{
	rtk_fc_aclAndCf_reserved_dual_hdr_chk_t dual_hdr_chk = {0};
	int ret;
	// set downstream cls for ingress interface
	dual_hdr_chk.packet_type = RTK_FC_CLS_DUAL_6RD;
	dual_hdr_chk.igr_intf_idx		= igrIntfIdx;
	dual_hdr_chk.hash_tuple_idx = RTK_ASIC_FLOW_PROFILE_FLOW_5TUPLE;
	dual_hdr_chk.six_rd.src_ip = pFcIngressData->srcIp;
	dual_hdr_chk.six_rd.dst_ip = pFcIngressData->dstIp;

	ret = _rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CLS_TYPE_DUAL_HEADER_DS_CHECK, &dual_hdr_chk);
	if(ret != RTK_FC_RET_OK)
	{
		DEBUG("add special ACL DUAL_HEADER_DS_CHECK failed!!! ret = %x",ret);
		return RTK_FC_RET_ERR;
	}
	else
	{
		DEBUG("add special ACL DUAL_HEADER_DS_CHECK success, idx: 0x%x",dual_hdr_chk.rslt_idx);
		//_rtk_ca_cls_rule_dump(dual_hdr_chk.rslt_idx, 0, 0-RTK_CLS_TYPE_DUAL_HEADER_DS_CHECK, FALSE);
		*result_idx = dual_hdr_chk.rslt_idx;
	}

	return RTK_FC_RET_OK;
}

int _rtk_fc_dualHeader_ctrl_setupPPTP(int dualControlIdx, rtk_fc_pktHdr_t *pPktHdr, int isIPv4)
{
	int32 ret=SUCCESS;
	l3pe_dual_control_tbl_entry_t dual_ctrl_entry;
	uint32 gre_len=8, ppp_len=0;

	//set dual hdr control for PPTP
	memset(&dual_ctrl_entry, 0, sizeof(l3pe_dual_control_tbl_entry_t));

	if(pPktHdr->direction==RTK_FC_FLOW_DIRECTION_UPSTREAM){
		if(pPktHdr->tunnelInfo.tunnelTag&GRESEQ_TAGIF){
			gre_len+=4;
			dual_ctrl_entry.dual_ctrl_gre_mod_fld += 1;	// only seq
		}
		if(pPktHdr->tunnelInfo.tunnelTag&GREACK_TAGIF){
			gre_len+=4;
			dual_ctrl_entry.dual_ctrl_gre_mod_fld += 2;	// only ack || both seq/ack
		}

		ppp_len=pPktHdr->tunnelInfo.pppMode;
	}

	dual_ctrl_entry.dual_ctrl_hdr_fmt = 0;
	if(isIPv4){
		//v4
		dual_ctrl_entry.dual_ctrl_hdr_len 				= 20+gre_len+ppp_len; 	// IPv4(20)+GRE(8~16)+PPP(1~4)

		//FIXME: because egress netif always has small MTU already, therefore we do not use this diff value to adjust MTU comparison.
		//dual_ctrl_entry.dual_ctrl_mtu_diff 			= 20+gre_len+ppp_len; 	// IPv4(20)+GRE(8~16)+PPP(1~4)

		dual_ctrl_entry.dual_ctrl_gre_seq_ack_ost 		= 28;	// IPv4(20)+GRE seq offset(8)

		dual_ctrl_entry.dual_ctrl_rep_ip_len 			= 1;
		dual_ctrl_entry.dual_ctrl_rep_ip_len_diff 		= 20+gre_len+ppp_len;	// IPv4(20)+GRE(8~16)+PPP(1~4)

		dual_ctrl_entry.dual_ctrl_rep_len_ost1 			= 24;	// IPv4(20)+GRE length field offset(4)
		dual_ctrl_entry.dual_ctrl_rep_len_diff1 		= ppp_len;	// PPP(1~4)

		dual_ctrl_entry.dual_ctrl_rep_len_ost2 			= 127;	// X
		dual_ctrl_entry.dual_ctrl_rep_len_diff2 		= 0;	// X
#if defined(CONFIG_FC_RTL9607F_SERIES)
		dual_ctrl_entry.dual_ctrl_rep_len_ost3 			= 127;	// X
		dual_ctrl_entry.dual_ctrl_rep_len_diff3 		= 0;	// X
#endif

		dual_ctrl_entry.dual_ctrl_ip4_ost 				= 0;
		dual_ctrl_entry.dual_ctrl_ip6_oft 				= 127;

		dual_ctrl_entry.dual_ctrl_rep_ip_en				= 1;	// replace IP version
		dual_ctrl_entry.dual_ctrl_rep_ip_ver 			= 0;	// IPv4
	}else{
		//v6
		dual_ctrl_entry.dual_ctrl_hdr_len 				= 40+gre_len+ppp_len; 	// IPv6(40)+GRE(8~16)+PPP(1~4)

		//FIXME: because egress netif always has small MTU already, therefore we do not use this diff value to adjust MTU comparison.
		//dual_ctrl_entry.dual_ctrl_mtu_diff 			= 40+gre_len+ppp_len; 	// IPv6(40)+GRE(8~16)+PPP(1~4)

		dual_ctrl_entry.dual_ctrl_gre_seq_ack_ost 		= 48;	// IPv6(40)+GRE seq offset(8)

		dual_ctrl_entry.dual_ctrl_rep_ip_len 			= 1;
		dual_ctrl_entry.dual_ctrl_rep_ip_len_diff 		= gre_len+ppp_len;	// GRE(8~16)+PPP(1~4)

		dual_ctrl_entry.dual_ctrl_rep_len_ost1 			= 44;	// IPv6(40)+GRE length field offset(4)
		dual_ctrl_entry.dual_ctrl_rep_len_diff1 		= ppp_len;	// PPP(1~4)

		dual_ctrl_entry.dual_ctrl_rep_len_ost2 			= 127;	// X
		dual_ctrl_entry.dual_ctrl_rep_len_diff2 		= 0;	// X
#if defined(CONFIG_FC_RTL9607F_SERIES)
		dual_ctrl_entry.dual_ctrl_rep_len_ost3 			= 127;	// X
		dual_ctrl_entry.dual_ctrl_rep_len_diff3 		= 0;	// X
#endif
		dual_ctrl_entry.dual_ctrl_ip4_ost 				= 127;
		dual_ctrl_entry.dual_ctrl_ip6_oft 				= 0;

		dual_ctrl_entry.dual_ctrl_rep_ip_en				= 1;	// replace IP version
		dual_ctrl_entry.dual_ctrl_rep_ip_ver 			= 1;	// IPv6
	}
	//dual_ctrl_entry.dual_ctrl_rep_ip_dscp_en 		= (TOS_hash_replace_ctrl&PPTP_TOS_REPLACE_DSCP_MASK)>0;
	//dual_ctrl_entry.dual_ctrl_rep_ip_ecn_en 		= (TOS_hash_replace_ctrl&PPTP_TOS_REPLACE_ECN_MASK)>0;
	//dual_ctrl_entry.dual_ctrl_rep_ip_ptc 			= 0;
	//dual_ctrl_entry.dual_ctrl_rep_ip_ttl 			= 0;
	//dual_ctrl_entry.dual_ctrl_mapt_pre_len 		= 0;
	//dual_ctrl_entry.dual_ctrl_map_drf_ver 		= 0;
	//dual_ctrl_entry.dual_ctrl_6rd_dip_pre_len 	= 0;
	//dual_ctrl_entry.dual_ctrl_6rd_dip_suf_str_bit = 0;
	//dual_ctrl_entry.dual_ctrl_dsl_uc_chk 			= 0;
	//dual_ctrl_entry.dual_ctrl_dsl_mc_chk 			= 0;
	//dual_ctrl_entry.dual_ctrl_l4sum_rec 			= 0;
	dual_ctrl_entry.dual_ctrl_vxlan_sp_ost 			= 127;
#if defined(CONFIG_FC_RTL9607F_SERIES)
	dual_ctrl_entry.dual_ctrl_inr_to_out_fld 		= 0xb8;	// bit [3,4,5,7]
#else
	dual_ctrl_entry.dual_ctrl_inr_to_out_fld 		= 0xb0;	// bit [4,5,7]
#endif
	dual_ctrl_entry.dual_ctrl_tos_hsh_sel 			= 0;	//hashed by outer TOS(1) or by inner TOS(0)
	dual_ctrl_entry.dual_ctrl_mc_mac_map 			= 1;
	dual_ctrl_entry.dual_ctrl_ppp_ip_ver_ost		= dual_ctrl_entry.dual_ctrl_hdr_len-1;		//modify ppp ip version based on inner packet ip version

	ret = aal_pe_dual_control_entry_add(dualControlIdx, &dual_ctrl_entry);

	return ret;
}

int _rtk_fc_dualHeader_ctrl_setupL2TP(int dualControlIdx, rtk_fc_pktHdr_t *pPktHdr)
{
	int32 ret=SUCCESS;
	l3pe_dual_control_tbl_entry_t dual_ctrl_entry;
	uint32 l2tpHdrLen=8, pppLen=pPktHdr->tunnelInfo.pppMode;
	uint32 /*l2tp_length_bit=1,*/ outer_isIpv6=(pPktHdr->outer_iph)?0:1;

	//set dual hdr control for L2TP
	{
		memset(&dual_ctrl_entry, 0, sizeof(l3pe_dual_control_tbl_entry_t));
		dual_ctrl_entry.dual_ctrl_hdr_fmt 				= 0;
		dual_ctrl_entry.dual_ctrl_hdr_len 				= (outer_isIpv6)?(40+8+l2tpHdrLen+pppLen):(20+8+l2tpHdrLen+pppLen);	// IP header(20/40)+UDP(8)+L2TP(8)+PPP(4)
		dual_ctrl_entry.dual_ctrl_mtu_diff 				= 0; //(outer_isIpv6)?(40+8+l2tpHdrLen+pppLen):(20+8+l2tpHdrLen+pppLen);	// IP header(20/40)+UDP(8)+L2TP(8)+PPP(4)
		dual_ctrl_entry.dual_ctrl_ppp_ip_ver_ost		= dual_ctrl_entry.dual_ctrl_hdr_len - 1;
		dual_ctrl_entry.dual_ctrl_rep_ip_en				= TRUE;
		dual_ctrl_entry.dual_ctrl_rep_ip_ver 			= outer_isIpv6;							// IPv4 or IPv6
		dual_ctrl_entry.dual_ctrl_rep_len_ost1 			= (outer_isIpv6)?(40+4):(20+4);			// IP header(20/40)+UDP length field offset(4)
		dual_ctrl_entry.dual_ctrl_rep_len_diff1 		= 8+l2tpHdrLen+pppLen;					// UDP(8)+L2TP(8)+PPP(4)
		/*if(l2tp_length_bit)*/	// coverity defect
		{
			dual_ctrl_entry.dual_ctrl_rep_len_ost2 		= (outer_isIpv6)?(40+8+2):(20+8+2);		// IP header(20/40)+UDP(8)+L2TP length field offset(2)
			dual_ctrl_entry.dual_ctrl_rep_len_diff2 	= l2tpHdrLen+pppLen;					// L2TP(8)+PPP(4)
		}
		/*else
		{
			dual_ctrl_entry.dual_ctrl_rep_len_ost2		= 127;
		}*/
		dual_ctrl_entry.dual_ctrl_ip4_ost 				= (outer_isIpv6)?127:0;
		dual_ctrl_entry.dual_ctrl_ip6_oft 				= (outer_isIpv6)?0:127;
		dual_ctrl_entry.dual_ctrl_gre_seq_ack_ost 		= 127;
		//dual_ctrl_entry.dual_ctrl_gre_mod_fld 		= 0;
		dual_ctrl_entry.dual_ctrl_rep_ip_dscp_en 		= FALSE;
		dual_ctrl_entry.dual_ctrl_rep_ip_ecn_en 		= FALSE;
		dual_ctrl_entry.dual_ctrl_rep_ip_len 			= 1;
		dual_ctrl_entry.dual_ctrl_rep_ip_len_diff 		= (outer_isIpv6)?(8+l2tpHdrLen+pppLen):(20+8+l2tpHdrLen+pppLen);	// IP header(20/40)+UDP(8)+L2TP(8)+PPP(4)
		//dual_ctrl_entry.dual_ctrl_rep_ip_ptc 			= 0;
		//dual_ctrl_entry.dual_ctrl_rep_ip_ttl 			= 0;
		//dual_ctrl_entry.dual_ctrl_mapt_pre_len 		= 0;
		//dual_ctrl_entry.dual_ctrl_map_drf_ver 		= 0;
		//dual_ctrl_entry.dual_ctrl_6rd_dip_pre_len 	= 0;
		//dual_ctrl_entry.dual_ctrl_6rd_dip_suf_str_bit = 0;
		//dual_ctrl_entry.dual_ctrl_dsl_uc_chk 			= 0;
		//dual_ctrl_entry.dual_ctrl_dsl_mc_chk 			= 0;
		dual_ctrl_entry.dual_ctrl_l4sum_rec 			= 0;
		dual_ctrl_entry.dual_ctrl_vxlan_sp_ost 			= 127;
#if defined(CONFIG_FC_RTL8277C_SERIES)		
		dual_ctrl_entry.dual_ctrl_inr_to_out_fld 		= 0xf0;	// bit [4,5,6,7]
#else
		dual_ctrl_entry.dual_ctrl_inr_to_out_fld 		= 0xf8;	// bit [3,4,5,6,7]
		dual_ctrl_entry.dual_ctrl_rep_len_ost3			= 127;
#endif
		dual_ctrl_entry.dual_ctrl_tos_hsh_sel 			= 0;	//hashed by outer TOS(1) or by inner TOS(0)
		dual_ctrl_entry.dual_ctrl_mc_mac_map 			= 1;

		aal_pe_dual_control_entry_del(dualControlIdx);
		ret = aal_pe_dual_control_entry_add(dualControlIdx, &dual_ctrl_entry);
		if(ret)
			WARNING("Add dual control failed, ret = %d", ret);
	}

	return ret;
}


int _rtk_fc_dualHeader_ctrl_setupMAPT(int dualControlIdx, uint32 mapt_draft_version, uint32 dmr_prefix_len)
{
	int32 ret=SUCCESS;
	l3pe_dual_control_tbl_entry_t dual_ctrl_entry;
	
	//set dual hdr control for mapT
	memset(&dual_ctrl_entry, 0, sizeof(l3pe_dual_control_tbl_entry_t));
	dual_ctrl_entry.dual_ctrl_hdr_fmt				= 2;
	dual_ctrl_entry.dual_ctrl_hdr_len				= 40;	// IPv6(40)
	dual_ctrl_entry.dual_ctrl_mtu_diff				= 40;	// IPv6(40)
	dual_ctrl_entry.dual_ctrl_rep_ip_en 			= 1;	// replace IP version
	dual_ctrl_entry.dual_ctrl_rep_ip_ver			= 1;	// IPv6
	dual_ctrl_entry.dual_ctrl_rep_len_ost1			= 127;	// X
	dual_ctrl_entry.dual_ctrl_rep_len_diff1 		= 0;	// X
	dual_ctrl_entry.dual_ctrl_rep_len_ost2			= 127;	// X
	dual_ctrl_entry.dual_ctrl_rep_len_diff2 		= 0;	// X
#if defined(CONFIG_FC_RTL9607F_SERIES)
	dual_ctrl_entry.dual_ctrl_rep_len_ost3			= 127;	// X
	dual_ctrl_entry.dual_ctrl_rep_len_diff3 		= 0;	// X
#endif
	dual_ctrl_entry.dual_ctrl_ip4_ost				= 127;
	dual_ctrl_entry.dual_ctrl_gre_seq_ack_ost		= 127;	// X
	dual_ctrl_entry.dual_ctrl_gre_mod_fld			= 0;	// X
	dual_ctrl_entry.dual_ctrl_ip6_oft				= 0;
	dual_ctrl_entry.dual_ctrl_rep_ip_len			= 1;
	dual_ctrl_entry.dual_ctrl_rep_ip_len_diff		= 0;
	//dual_ctrl_entry.dual_ctrl_rep_ip_len_diff_pad_ctrl=(ip_len_ctrl_disable==0);
	//dual_ctrl_entry.dual_ctrl_rep_ip_dscp_en		= (TOS_hash_replace_ctrl&MAPT_TOS_REPLACE_DSCP_MASK)>0;
	//dual_ctrl_entry.dual_ctrl_rep_ip_ecn_en 		= (TOS_hash_replace_ctrl&MAPT_TOS_REPLACE_ECN_MASK)>0;
	dual_ctrl_entry.dual_ctrl_rep_ip_ptc			= 1;
	dual_ctrl_entry.dual_ctrl_rep_ip_ttl			= 1;
	switch(dmr_prefix_len){
		case 32:
		case 40:
		case 48:
		case 56:
		case 64:
			dual_ctrl_entry.dual_ctrl_mapt_pre_len	= (dmr_prefix_len>>3)-3;
			break;
		case 96:
			dual_ctrl_entry.dual_ctrl_mapt_pre_len	= (dmr_prefix_len>>3)-6;
			break;
		default:
			break;
	}
	dual_ctrl_entry.dual_ctrl_map_drf_ver			= mapt_draft_version;
	//dual_ctrl_entry.dual_ctrl_6rd_dip_pre_len 	= 0;
	//dual_ctrl_entry.dual_ctrl_6rd_dip_suf_str_bit = 0;
	//dual_ctrl_entry.dual_ctrl_dsl_uc_chk			= 0;
	//dual_ctrl_entry.dual_ctrl_dsl_mc_chk			= 0;
	dual_ctrl_entry.dual_ctrl_l4sum_rec 			= 1;	// pop IPv4, recalculate L4 checksum by IPV6 pseudo header
	dual_ctrl_entry.dual_ctrl_vxlan_sp_ost			= 127;
	dual_ctrl_entry.dual_ctrl_inr_to_out_fld		= 0x80; // bit [7]
	dual_ctrl_entry.dual_ctrl_tos_hsh_sel			= 0; 	//enable or disable won't make any difference
	dual_ctrl_entry.dual_ctrl_mc_mac_map			= 1;
	dual_ctrl_entry.dual_ctrl_ppp_ip_ver_ost		= 127;	//disable
	ret = aal_pe_dual_control_entry_add(dualControlIdx, &dual_ctrl_entry);
	if(ret)
		WARNING("Add dual control failed, ret = %d", ret);

	return ret;
}

#if defined(CONFIG_FC_RTL9607F_SERIES)

int _rtk_fc_dualHeader_ctrl_setupXLAT(int dualControlIdx, rtk_fc_dual_ipv6HashMask_t remotePrefix,rtk_fc_dual_ipv6HashMask_t  localPrefix)
{
	int32 ret=SUCCESS;
	l3pe_dual_control_tbl_entry_t dual_ctrl_entry;
	
	//set dual hdr control for mapT
	memset(&dual_ctrl_entry, 0, sizeof(l3pe_dual_control_tbl_entry_t));
	dual_ctrl_entry.dual_ctrl_hdr_fmt				= 2;
	dual_ctrl_entry.dual_ctrl_hdr_len				= 40;	// IPv6(40)
	dual_ctrl_entry.dual_ctrl_mtu_diff				= 40;	// IPv6(40)
	dual_ctrl_entry.dual_ctrl_rep_ip_en 			= 1;	// replace IP version
	dual_ctrl_entry.dual_ctrl_rep_ip_ver			= 1;	// IPv6
	dual_ctrl_entry.dual_ctrl_rep_len_ost1			= 127;	// X
	dual_ctrl_entry.dual_ctrl_rep_len_diff1 		= 0;	// X
	dual_ctrl_entry.dual_ctrl_rep_len_ost2			= 127;	// X
	dual_ctrl_entry.dual_ctrl_rep_len_diff2 		= 0;	// X
	dual_ctrl_entry.dual_ctrl_ip4_ost				= 127;
	dual_ctrl_entry.dual_ctrl_gre_seq_ack_ost		= 127;	// X			
	dual_ctrl_entry.dual_ctrl_gre_mod_fld			= 0;	// X
	dual_ctrl_entry.dual_ctrl_ip6_oft				= 0;
	dual_ctrl_entry.dual_ctrl_rep_ip_len			= 1;
	dual_ctrl_entry.dual_ctrl_rep_ip_len_diff		= 0;
	//dual_ctrl_entry.dual_ctrl_rep_ip_len_diff_pad_ctrl=(ip_len_ctrl_disable==0);
	//dual_ctrl_entry.dual_ctrl_rep_ip_dscp_en		= (TOS_hash_replace_ctrl&MAPT_TOS_REPLACE_DSCP_MASK)>0;
	//dual_ctrl_entry.dual_ctrl_rep_ip_ecn_en 		= (TOS_hash_replace_ctrl&MAPT_TOS_REPLACE_ECN_MASK)>0;
	dual_ctrl_entry.dual_ctrl_rep_ip_ptc			= 1;
	dual_ctrl_entry.dual_ctrl_rep_ip_ttl			= 1;
	//dual_ctrl_entry.dual_ctrl_map_drf_ver			= 0;
	//dual_ctrl_entry.dual_ctrl_6rd_dip_pre_len 	= 0;
	//dual_ctrl_entry.dual_ctrl_6rd_dip_suf_str_bit = 0;
	//dual_ctrl_entry.dual_ctrl_dsl_uc_chk			= 0;
	//dual_ctrl_entry.dual_ctrl_dsl_mc_chk			= 0;
	dual_ctrl_entry.dual_ctrl_l4sum_rec 			= 1;	// pop IPv4, recalculate L4 checksum by IPV6 pseudo header
	dual_ctrl_entry.dual_ctrl_vxlan_sp_ost			= 127;
	dual_ctrl_entry.dual_ctrl_inr_to_out_fld		= 0x80; // bit [7]
	dual_ctrl_entry.dual_ctrl_tos_hsh_sel			= 0; 	//enable or disable won't make any difference
	dual_ctrl_entry.dual_ctrl_mc_mac_map			= 1;
	dual_ctrl_entry.dual_ctrl_ppp_ip_ver_ost		= 127;	//disable

	dual_ctrl_entry.dual_ctrl_xlat464_sip_dip_trans = TRUE;
	
	dual_ctrl_entry.dual_ctrl_rep_len_ost3 			= 127;	// X
	dual_ctrl_entry.dual_ctrl_rep_len_diff3 		= 0;	// X

	if(remotePrefix&DUAL_IPV6_HASHMASK_PREFIX_96)
		dual_ctrl_entry.dual_ctrl_mapt_pre_len=6;
	else if (remotePrefix&DUAL_IPV6_HASHMASK_PREFIX_64)
		dual_ctrl_entry.dual_ctrl_mapt_pre_len=5;
	else if (remotePrefix&DUAL_IPV6_HASHMASK_PREFIX_56)
		dual_ctrl_entry.dual_ctrl_mapt_pre_len=4;
	else if (remotePrefix&DUAL_IPV6_HASHMASK_PREFIX_48)
		dual_ctrl_entry.dual_ctrl_mapt_pre_len=3;
	else if (remotePrefix&DUAL_IPV6_HASHMASK_PREFIX_40)
		dual_ctrl_entry.dual_ctrl_mapt_pre_len=2;
	else if (remotePrefix&DUAL_IPV6_HASHMASK_PREFIX_32)
		dual_ctrl_entry.dual_ctrl_mapt_pre_len=1;
	else if (remotePrefix&DUAL_IPV6_HASHMASK_ALL)
		dual_ctrl_entry.dual_ctrl_mapt_pre_len=0;

	if(localPrefix&DUAL_IPV6_HASHMASK_PREFIX_96)
		dual_ctrl_entry.dual_ctrl_xlat464_sip_prefix_len=6;
	else if (localPrefix&DUAL_IPV6_HASHMASK_PREFIX_64)
		dual_ctrl_entry.dual_ctrl_xlat464_sip_prefix_len=5;
	else if (localPrefix&DUAL_IPV6_HASHMASK_PREFIX_56)
		dual_ctrl_entry.dual_ctrl_xlat464_sip_prefix_len=4;
	else if (localPrefix&DUAL_IPV6_HASHMASK_PREFIX_48)
		dual_ctrl_entry.dual_ctrl_xlat464_sip_prefix_len=3;
	else if (localPrefix&DUAL_IPV6_HASHMASK_PREFIX_40)
		dual_ctrl_entry.dual_ctrl_xlat464_sip_prefix_len=2;
	else if (localPrefix&DUAL_IPV6_HASHMASK_PREFIX_32)
		dual_ctrl_entry.dual_ctrl_xlat464_sip_prefix_len=1;
	else if (localPrefix&DUAL_IPV6_HASHMASK_ALL)
		dual_ctrl_entry.dual_ctrl_xlat464_sip_prefix_len=0;


	ret = aal_pe_dual_control_entry_add(dualControlIdx, &dual_ctrl_entry);
	if(ret)
		WARNING("Add dual control failed, ret = %d", ret);

	return ret;
}
#endif

int _rtk_fc_dualHeader_ctrl_setupIP4InIP6(int dualControlIdx, rtk_fc_pktHdr_t *pPktHdr)
{
	int32 ret=SUCCESS;
	l3pe_dual_control_tbl_entry_t dual_ctrl_entry;

	//set dual hdr control for Dslite/Mape
	{
		memset(&dual_ctrl_entry, 0, sizeof(l3pe_dual_control_tbl_entry_t));
		dual_ctrl_entry.dual_ctrl_hdr_fmt				= 0;
		dual_ctrl_entry.dual_ctrl_hdr_len				= 40;	// IPv6 Hdr Len(40)
		dual_ctrl_entry.dual_ctrl_mtu_diff				= 0;	// interface already using 1500-40=1460 so do not config this field
		dual_ctrl_entry.dual_ctrl_ppp_ip_ver_ost		= 127;
		dual_ctrl_entry.dual_ctrl_rep_ip_en 			= 1;	//chnage L2 to v6 0x86dd/0x0057
		dual_ctrl_entry.dual_ctrl_rep_ip_ver			= 1;	//chnage L2 to v6 0x86dd/0x0057
		dual_ctrl_entry.dual_ctrl_rep_len_ost1			= 127;	
		//dual_ctrl_entry.dual_ctrl_rep_len_diff1 		= 0;
		dual_ctrl_entry.dual_ctrl_rep_len_ost2			= 127;	
		//dual_ctrl_entry.dual_ctrl_rep_len_diff2 		= 0;	
		dual_ctrl_entry.dual_ctrl_ip4_ost				= 127;
		dual_ctrl_entry.dual_ctrl_gre_seq_ack_ost		= 127;
#if defined(CONFIG_FC_RTL8277C_SERIES)
#else
		dual_ctrl_entry.dual_ctrl_rep_len_ost3 			= 127;	// X
		dual_ctrl_entry.dual_ctrl_rep_len_diff3 		= 0;	// X
#endif		
		dual_ctrl_entry.dual_ctrl_rep_ip_dscp_en		= FALSE;
		dual_ctrl_entry.dual_ctrl_rep_ip_ecn_en 		= FALSE;
		dual_ctrl_entry.dual_ctrl_rep_ip_len			= 1;
		dual_ctrl_entry.dual_ctrl_rep_ip_len_diff		= 0;	// IPv6 Hdr Len(40)
		dual_ctrl_entry.dual_ctrl_rep_ip_ttl			= FALSE;
		dual_ctrl_entry.dual_ctrl_dsl_uc_chk			= FALSE;
		dual_ctrl_entry.dual_ctrl_dsl_mc_chk			= TRUE;
		dual_ctrl_entry.dual_ctrl_vxlan_sp_ost			= 127;
		/*0:L2 1:Vlan 2:802.1p 3:l2+pppoe 4 L3 5:DSCP 6:L4 7:ehtertype+pppoe_enc*/
		dual_ctrl_entry.dual_ctrl_inr_to_out_fld		= 0x30; // bit [4,5]
		dual_ctrl_entry.dual_ctrl_tos_hsh_sel			= 0;
		dual_ctrl_entry.dual_ctrl_mc_mac_map			= 1;
		dual_ctrl_entry.dual_ctrl_rep_ip_len_diff_pad_ctrl = 0;

		ret = aal_pe_dual_control_entry_add(dualControlIdx, &dual_ctrl_entry);
		if(ret)
			WARNING("Add dual control failed, ret = %d", ret);
	}


	return ret;
}
int _rtk_fc_dualHeader_dual_content_setupVXLAN(int hwNetifIdx, rtk_fc_pktHdr_t *pPktHdr )
{
	// Dual content table set
	int ret = RTK_FC_RET_OK, dual_header_outer_content_size;
	int dual_control_entry_idx 			= RTK_FC_DUAL_CONTROL_IDX(hwNetifIdx);
	
	dual_header_outer_content_size = pPktHdr->vxlan_info.outerTag_off;
	ret = rtk_fc_vxlan_get_outer_content(dual_header_outer_content_size, pPktHdr);
	if(ret !=SUCCESS)
		return ret;

	
	if(((fc_db.netifTbl[NETIF_HWTOSW(hwNetifIdx)].dualHdr_HWState&FC_DUALHDR_HW_CONTENT_BUF_SETUP)==0))
	{
		//rtk_fc_dualHeader_outerContent_get_VXLAN(dual_header_outer_content, dual_header_outer_content_size, RTK_FC_DUALTYPE_VXLAN, pPktHdr);
		ret = rtk_fc_dual_control_content_setVXLAN(dual_control_entry_idx, &pPktHdr->vxlan_info.outer_content[0], dual_header_outer_content_size);
		if(ret!=RTK_FC_RET_OK)
		{
			WARNING("DUAL CONTENT ENTRY[%d] Set failed!",dual_control_entry_idx);
			return ret;
		}

		fc_db.netifTbl[NETIF_HWTOSW(hwNetifIdx)].dualHdr_HWState|=FC_DUALHDR_HW_CONTENT_BUF_SETUP;
	}

	return ret;
}

int _rtk_fc_dualHeader_dual_ctrl_setupVXLAN(int hwNetifIdx, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_dev_type_t devType )
{
	int dual_control_entry_idx 			= RTK_FC_DUAL_CONTROL_IDX(hwNetifIdx);
	int real_vxlan_interface_index		= _rtk_fc_flow_intf_mapping_idx_get(hwNetifIdx);
	int ret = RTK_FC_RET_OK;
	
	DEBUG("dual_control_entry_idx = %d, hwNetifIdx = %d(sw:%d)",dual_control_entry_idx, hwNetifIdx, NETIF_HWTOSW(hwNetifIdx));

	
	if((fc_db.netifTbl[NETIF_HWTOSW(hwNetifIdx)].dualHdr_HWState&FC_DUALHDR_HW_CTRL_UP_REG_SETUP)==0)
	{
		TRACE("Try to Add dual control reg");
		// Dual control table set
		ret = rtk_fc_dual_control_entry_setVXLAN(dual_control_entry_idx, pPktHdr);
		if(ret!=RTK_FC_RET_OK)
		{
			WARNING("DUAL CONTROL ENTRY[%d] Set failed!",dual_control_entry_idx);
			return ret;
		}
		fc_db.netifTbl[NETIF_HWTOSW(hwNetifIdx)].dualHdr_HWState|=FC_DUALHDR_HW_CTRL_UP_REG_SETUP;
	}

	
	// For l2 Dual table setting (sw)
	
	

	//VXLAN: set l3 reserved cls for down stream
	if(fc_db.netifTbl[NETIF_HWTOSW(hwNetifIdx)].dualHdr_ds_clsIdx==SIGNED_INVALID)
	{
		rtk_fc_aclAndCf_reserved_dual_hdr_chk_t dual_hdr_chk = {0};
		int result_idx;
		
		dual_hdr_chk.packet_type					= RTK_FC_CLS_DUAL_VXLAN;
		dual_hdr_chk.profile_idx					= 0; //WAN
		dual_hdr_chk.igr_intf_idx					= real_vxlan_interface_index;
		dual_hdr_chk.hash_tuple_idx 				= RTK_ASIC_FLOW_PROFILE_FLOW_5TUPLE;
		dual_hdr_chk.vxlan.vlan.has_ctag			= (pPktHdr->vxlan_info.outer_cvh)?1:0;
		dual_hdr_chk.vxlan.vlan.cvlan_id			= (pPktHdr->vxlan_info.outer_cvlanid)?(pPktHdr->vxlan_info.outer_cvlanid):0;
		dual_hdr_chk.vxlan.vlan.has_stag			= (pPktHdr->vxlan_info.outer_svh)?1:0;
		dual_hdr_chk.vxlan.vlan.svlan_id			= (pPktHdr->vxlan_info.outer_svlanid)?(pPktHdr->vxlan_info.outer_svlanid):0;
		dual_hdr_chk.vxlan.vlan.vlan_check_en		= 1;
		dual_hdr_chk.vxlan.vni						= (pPktHdr->vxlan_info.vni);
		if(pPktHdr->outer_ip6h)
		{	
			dual_hdr_chk.vxlan.dst_ip[0]	= ntohl(pPktHdr->vxlan_info.gateway_v6_ip.in6_u.u6_addr32[0]);
			dual_hdr_chk.vxlan.dst_ip[1]	= ntohl(pPktHdr->vxlan_info.gateway_v6_ip.in6_u.u6_addr32[1]);
			dual_hdr_chk.vxlan.dst_ip[2]	= ntohl(pPktHdr->vxlan_info.gateway_v6_ip.in6_u.u6_addr32[2]);
			dual_hdr_chk.vxlan.dst_ip[3]	= ntohl(pPktHdr->vxlan_info.gateway_v6_ip.in6_u.u6_addr32[3]);
		}
		else
		{
			dual_hdr_chk.vxlan.dst_ip[3]	= ntohl(pPktHdr->vxlan_info.gateway_v4_ip);
			if(dual_hdr_chk.vxlan.dst_ip[3] == 0x0)
				WARNING("[VXLAN]Try to set strange downstream VXLAN cls ip = 0x0 (pPktHdr->outer_iph: %x) ",pPktHdr->vxlan_info.gateway_v4_ip);
		}
		
		ret = _rtk_fc_vxlan_8277C_downStream_reservedACL_add(&result_idx, &dual_hdr_chk);
		
		if(ret!=RTK_FC_RET_OK)
		{
			WARNING("Set Reserved ACL failed!");
			return ret;
		}
		else
		{
			fc_db.netifTbl[NETIF_HWTOSW(hwNetifIdx)].dualHdr_ds_clsIdx = result_idx;
			
			memset(&dual_hdr_chk,0, sizeof(rtk_fc_aclAndCf_reserved_dual_hdr_chk_t));
			dual_hdr_chk.packet_type					= RTK_FC_CLS_DUAL_VXLAN_NON_IP;
			dual_hdr_chk.profile_idx					= 0; //WAN
			dual_hdr_chk.igr_intf_idx					= real_vxlan_interface_index;
			dual_hdr_chk.hash_tuple_idx 				= RTK_ASIC_FLOW_PROFILE_FLOW_2TUPLE;
			dual_hdr_chk.vxlan.vlan.has_ctag			= (pPktHdr->vxlan_info.outer_cvh)?1:0;
			dual_hdr_chk.vxlan.vlan.cvlan_id			= (pPktHdr->vxlan_info.outer_cvlanid)?(pPktHdr->vxlan_info.outer_cvlanid):0;
			dual_hdr_chk.vxlan.vlan.has_stag			= (pPktHdr->vxlan_info.outer_svh)?1:0;
			dual_hdr_chk.vxlan.vlan.svlan_id			= (pPktHdr->vxlan_info.outer_svlanid)?(pPktHdr->vxlan_info.outer_svlanid):0;
			dual_hdr_chk.vxlan.vlan.vlan_check_en		= 1;
			dual_hdr_chk.vxlan.vni						= pPktHdr->vxlan_info.vni;
			if(pPktHdr->outer_ip6h)
			{	
				dual_hdr_chk.vxlan.dst_ip[0]	= ntohl(pPktHdr->vxlan_info.gateway_v6_ip.in6_u.u6_addr32[0]);
				dual_hdr_chk.vxlan.dst_ip[1]	= ntohl(pPktHdr->vxlan_info.gateway_v6_ip.in6_u.u6_addr32[1]);
				dual_hdr_chk.vxlan.dst_ip[2]	= ntohl(pPktHdr->vxlan_info.gateway_v6_ip.in6_u.u6_addr32[2]);
				dual_hdr_chk.vxlan.dst_ip[3]	= ntohl(pPktHdr->vxlan_info.gateway_v6_ip.in6_u.u6_addr32[3]);
			}
			else
			{
				dual_hdr_chk.vxlan.dst_ip[3]	= ntohl(pPktHdr->vxlan_info.gateway_v4_ip);
				if(dual_hdr_chk.vxlan.dst_ip[3] == 0x0)
					WARNING("[VXLAN]Try to set strange downstream VXLAN cls ip = 0x0 (pPktHdr->outer_iph: %x) ",pPktHdr->vxlan_info.gateway_v4_ip);
			}
			
			ret = _rtk_fc_vxlan_8277C_downStream_reservedACL_add(&result_idx, &dual_hdr_chk);
			
			if(ret!=RTK_FC_RET_OK)
			{
				WARNING("Set extra Reserved ACL failed!");
				return ret;
			}
			else
			{
				fc_db.netifTbl[NETIF_HWTOSW(hwNetifIdx)].dualHdr_ds_extra_clsIdx = result_idx;
			}
			
		}
		ACL_RSV("VXLAN down stream L3 cls index: %x extra index:%x, netifHwTbl interface index: %d", fc_db.netifTbl[NETIF_HWTOSW(hwNetifIdx)].dualHdr_ds_clsIdx, fc_db.netifTbl[NETIF_HWTOSW(hwNetifIdx)].dualHdr_ds_extra_clsIdx, hwNetifIdx);
	}

	return ret;
}

#if defined(CONFIG_FC_RTL9607F_SERIES)
int _rtk_fc_dualHeader_ctrl_setupGreEthBr(int dualControlIdx, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_tableNetif_t *pNetif, uint outer_pppoe_tag_off, uint outer_ip_tag_off)
{
	int32 ret=SUCCESS;
	l3pe_dual_control_tbl_entry_t dual_ctrl_entry;

	//set dual hdr control for GRE_ETH_BR
	memset(&dual_ctrl_entry, 0, sizeof(l3pe_dual_control_tbl_entry_t));
	dual_ctrl_entry.dual_ctrl_hdr_fmt			= 1;
	dual_ctrl_entry.dual_ctrl_hdr_len			= pNetif->dualUniInfo.greEthBrInfo.outerTag_len;
	if(pNetif->dualUniInfo.greEthBrInfo.outer_ipversion)
		dual_ctrl_entry.dual_ctrl_mtu_diff		= sizeof(struct ipv6hdr) + sizeof(struct gre_base_hdr);	// IPv6(40)+L2GRE(4)
	else
		dual_ctrl_entry.dual_ctrl_mtu_diff		= sizeof(struct iphdr) + sizeof(struct gre_base_hdr);	// IPv4(20)+L2GRE(4)

	dual_ctrl_entry.dual_ctrl_ppp_ip_ver_ost	= 127;
	dual_ctrl_entry.dual_ctrl_rep_ip_en			= FALSE;												// don't care
	dual_ctrl_entry.dual_ctrl_rep_ip_ver		= 0;													// don't care
	if(outer_pppoe_tag_off)
	{
		dual_ctrl_entry.dual_ctrl_rep_len_ost1	= outer_pppoe_tag_off + 4; //pppoe payload_len
		if(pNetif->dualUniInfo.greEthBrInfo.outer_ipversion)
			dual_ctrl_entry.dual_ctrl_rep_len_diff1 = 2 + sizeof(struct ipv6hdr) + sizeof(struct gre_base_hdr); // PPP(2) + IPv6 header(40) + L2GRE(4)
		else
			dual_ctrl_entry.dual_ctrl_rep_len_diff1	= 2 + sizeof(struct iphdr) + sizeof(struct gre_base_hdr); // PPP(2) + IPv4 header(20) + L2GRE(4
	}
	else
		dual_ctrl_entry.dual_ctrl_rep_len_ost1		= 127;

	dual_ctrl_entry.dual_ctrl_rep_len_ost2			= 127;
	dual_ctrl_entry.dual_ctrl_rep_len_ost3			= 127;

	dual_ctrl_entry.dual_ctrl_rep_ip_dscp_en		= FALSE;
	dual_ctrl_entry.dual_ctrl_rep_ip_ecn_en			= FALSE;
	dual_ctrl_entry.dual_ctrl_rep_ip_len			= 1;

	if(pNetif->dualUniInfo.greEthBrInfo.outer_ipversion)
	{
		dual_ctrl_entry.dual_ctrl_ip4_ost			= 127;
		dual_ctrl_entry.dual_ctrl_ip6_oft			= outer_ip_tag_off;
		dual_ctrl_entry.dual_ctrl_rep_ip_len_diff	= sizeof(struct gre_base_hdr);
	}
	else
	{
		dual_ctrl_entry.dual_ctrl_ip4_ost			= outer_ip_tag_off;
		dual_ctrl_entry.dual_ctrl_ip6_oft			= 127;
		dual_ctrl_entry.dual_ctrl_rep_ip_len_diff	= sizeof(struct iphdr) + sizeof(struct gre_base_hdr);
	}
	dual_ctrl_entry.dual_ctrl_gre_seq_ack_ost		= 127;
	//dual_ctrl_entry.dual_ctrl_gre_mod_fld			= 0;
	//dual_ctrl_entry.dual_ctrl_rep_ip_ptc			= 0;
	//dual_ctrl_entry.dual_ctrl_rep_ip_ttl			= 0;
	//dual_ctrl_entry.dual_ctrl_mapt_pre_len		= 0;
	//dual_ctrl_entry.dual_ctrl_map_drf_ver 		= 0;
	//dual_ctrl_entry.dual_ctrl_6rd_dip_pre_len 	= 0;
	//dual_ctrl_entry.dual_ctrl_6rd_dip_suf_str_bit = 0;
	//dual_ctrl_entry.dual_ctrl_dsl_uc_chk			= 0;
	//dual_ctrl_entry.dual_ctrl_dsl_mc_chk			= 0;
	dual_ctrl_entry.dual_ctrl_l4sum_rec 			= 0;
	dual_ctrl_entry.dual_ctrl_inr_to_out_fld		= 0x3f; // bit [0,1,2,3,4,5]
	dual_ctrl_entry.dual_ctrl_tos_hsh_sel 			= 0;	//hashed by outer TOS(1) or by inner TOS(0);
	//dual_ctrl_entry.dual_ctrl_mc_mac_map			= 0;

	ret = aal_pe_dual_control_entry_add(dualControlIdx, &dual_ctrl_entry);
	return ret;
}
#endif
#endif



int _rtk_fc_flow_setupPPTP(rtk_fc_pktHdr_t *pPktHdr,struct sk_buff *skb, int wan_intf_idx)
{
	int ret=RTK_FC_RET_OK;
	rtk_fc_tableNetif_t *pNetif = &fc_db.netifTbl[NETIF_HWTOSW(wan_intf_idx)];

	if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
	{
		if(pNetif->outerHdrExtratagIdx == SIGNED_INVALID)
		{
			//1 1: Add flow extra tag
			// Generate IPv4+GRE+PPP header
			int32 extraTagActListIdx = 0;
			u8 contenBuffer[64]={0};
			u8 *outerHdr = &contenBuffer[0];
			u8 off;
			u32 flag;
			TRACE("[Dual] Gen outer header and add to SRAM content buffer");

			// Prepare IP(20)+GRE(16)+PPP(4) for PPTP
			// L3 header --
			if(pNetif->dualUniInfo.psGreInfo.outer_ipversion)
			{
				TRACE("[Dual][PPTP] Do not support outer non-ipv4 hdr");
				return FAIL;
			}
			else
			{
				*((uint16 *)(outerHdr + 0)) = htons(0x4500);	// ver, IHL
				//*((uint16 *)(outerHdr + 2)) = htons(0x0);	// total length	: updated by extra tag actions
				//*((uint16 *)(outerHdr + 4)) = htons(0x0);	// ipid		: updated by extra tag actions
				//*((uint16 *)(outerHdr + 6)) = htons(0x0);	// flags, frag offset
				*((uint16 *)(outerHdr + 8)) = htons(0x402f);	// ttl, protocol
				//*((uint16 *)(outerHdr + 10)) = htons(0x0);	// hdr checksum
				*((uint32 *)(outerHdr + 12)) = htonl(pNetif->dualUniInfo.psGreInfo.localIp);	// src ip
				*((uint32 *)(outerHdr + 16)) = htonl(pNetif->dualUniInfo.psGreInfo.remoteIp);	// dst ip
			}

			// GRE --
			flag=0x2001880b;
			//*(uint16 *)(outerHdr + 24) = htons(0x0);									//Payload length
			*(uint16 *)(outerHdr + 26) = htons(pNetif->dualUniInfo.psGreInfo.peer_call_id);		//Peer CallID
			//*(uint32 *)(outerHdr + 28) = htonl(0x0);										//Sequence Num
			//*(uint32 *)(outerHdr + 32) = htonl(0x0);										//Acknowledgment Num
			off=28;
			if(pPktHdr->tunnelInfo.tunnelTag&GRESEQ_TAGIF){
				flag|=0x10000000;
				off+=4;
			}
			if(pPktHdr->tunnelInfo.tunnelTag&GREACK_TAGIF){
				flag|=0x00800000;
				off+=4;
			}
			*(uint32 *)(outerHdr + 20) = htonl(flag);									//Flags, version, Protocol Type=PPP

			// PPP --
			{
				switch(pPktHdr->tunnelInfo.pppMode){
					case RTK_FC_COMP_PPP_FF0300XX:
						*(uint32 *)(outerHdr + off) = htonl(0xff030021);							// Address, Control, Protocol=IPv4
						break;
					case RTK_FC_COMP_PPP_FF03XX:
						*(uint32 *)(outerHdr + off) = htonl(0xff032100);							// Address, Control, Protocol=IPv4
						break;
					case RTK_FC_COMP_PPP_00XX:
						*(uint32 *)(outerHdr + off) = htonl(0x00210000);							// Address, Control, Protocol=IPv4
						break;
					case RTK_FC_COMP_PPP_XX:
						*(uint32 *)(outerHdr + off) = htonl(0x21000000);							// Address, Control, Protocol=IPv4
						break;
					default:
						break;
				}
			}

			if((ret = _rtk_fc_extraTag_setupPPTP(wan_intf_idx, outerHdr, &(pPktHdr->tunnelInfo), &extraTagActListIdx)) == SUCCESS)
				pNetif->outerHdrExtratagIdx = extraTagActListIdx;
			else
				return FAIL;

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
			if(aal_pe_dual_content_entry_add(RTK_FC_DUAL_CONTROL_IDX(wan_intf_idx), (l3pe_dual_tbl_entry_t *)outerHdr))
				DEBUG("[Dual][PPTP]Dual content add failed.");
			//if downstream already add dual control before, delete once to clear it up!
			if(pNetif->dualHdr_HWState&FC_DUALHDR_HW_CTRL_DOWN_REG_SETUP)
				aal_pe_dual_control_entry_del(RTK_FC_DUAL_CONTROL_IDX(wan_intf_idx));
			if(_rtk_fc_dualHeader_ctrl_setupPPTP(RTK_FC_DUAL_CONTROL_IDX(wan_intf_idx), pPktHdr, pPktHdr->outer_iph?1:0))
				DEBUG("[Dual][PPTP]Dual contrl entry add failed.");
			pNetif->dualHdr_HWState|=FC_DUALHDR_HW_CTRL_UP_REG_SETUP;
#endif

			//1 3: GRE SEQ starts from 1
			if((ret = rtk_fc_dualHdrInfo_set(RTK_FC_FB_DUALHDR_OUTER_IPV4ID, wan_intf_idx, ntohs(pPktHdr->outer_iph->id)/*network endian*/)) != RT_ERR_RG_OK)
				WARNING("set ipid was fail, ret = %d", ret);

			//if(pPktHdr->tunnelInfo.pptp.greseqTagif){
			if(pPktHdr->tunnelInfo.tunnelTag & GRESEQ_TAGIF){
				if((ret = rtk_fc_dualHdrInfo_set(RTK_FC_FB_DUALHDR_GRESEQ, wan_intf_idx, pPktHdr->tunnelInfo.pptp.greseq/*host endian*/)) != RT_ERR_RG_OK)
					WARNING("set gre sequence was fail, ret = %d", ret);
			}

			//if(pPktHdr->tunnelInfo.pptp.greackTagif){
			if(pPktHdr->tunnelInfo.tunnelTag & GREACK_TAGIF){
				if((ret = rtk_fc_dualHdrInfo_set(RTK_FC_FB_DUALHDR_GREACK, wan_intf_idx, pPktHdr->tunnelInfo.pptp.greack/*host endian*/)) != RT_ERR_RG_OK)
					WARNING("set gre ack was fail, ret = %d", ret);
			}
		}
	}

	if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM)
	{
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
		if (pNetif->outerHdrFlowIdx==SIGNED_INVALID)
		{
			rtk_fc_tableFlowEntry_t flowPathEntry;
			rtk_rg_asic_path6_entry_t *flowPath6 = &flowPathEntry.path6;
			rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo = NULL;
			rtk_fc_g3IgrExtraInfo_t g3IgrExtraInfo;
			rtk_fc_ingress_data_t *pFcIngressData = &skb->fcIngressData;
			rtk_fc_pmap_t portmap;
			uint32 flow_idx;
			int32 flowHashIdx = 0;
			uint8 addSwOnly = FALSE;
			int16 remoteGwMac = 0;

			ret=_rtk_fc_lut_find(pPktHdr->eth->h_dest, &remoteGwMac);
			if(ret != RTK_FC_RET_OK)
			{
				WARNING("find remoteGwMac index fail!!! ret = %x", ret);
				return FAIL;
			}

			TRACE("[Dual] prepare PPTP path6");

			//1 2: Add flow path6
			bzero(flowPath6, sizeof(rtk_rg_asic_path6_entry_t));
			flowPath6->valid = TRUE;
			flowPath6->in_path = FB_PATH_6;
			flowPath6->in_src_ip_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_SIP];
			flowPath6->in_src_ipv4_addr = ntohl(pNetif->dualUniInfo.psGreInfo.remoteIp);
			flowPath6->in_dst_ip_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_DIP];
			flowPath6->in_dst_ipv4_addr = ntohl(pNetif->dualUniInfo.psGreInfo.localIp);
			flowPath6->in_l4_src_port_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_SPORT];
			flowPath6->in_l4_src_port = 0;
			flowPath6->in_l4_dst_port_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_DPORT];
			flowPath6->in_l4_dst_port = 0;
			flowPath6->in_intf_idx = wan_intf_idx;
			flowPath6->in_ctagif = (pFcIngressData->ingressTagif & CVLAN_TAGIF) ? TRUE : FALSE;
			flowPath6->in_stagif = (pFcIngressData->ingressTagif & SVLAN_TAGIF) ? TRUE : FALSE;
			flowPath6->in_pppoeif= (pFcIngressData->ingressTagif & PPPOE_TAGIF) ? TRUE : FALSE;
			flowPath6->in_pppoe_sid = (pFcIngressData->ingressTagif & PPPOE_TAGIF) ? pFcIngressData->sessionId : 0;
			flowPath6->in_src_mac_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_SMAC_IDX];
			flowPath6->in_smac_lut_idx = remoteGwMac;
			flowPath6->in_dst_mac_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_DMAC_IDX];
			flowPath6->in_dmac_lut_idx = pNetif->lutIdx;
			flowPath6->in_pptpif = TRUE;
			flowPath6->in_gre_call_id_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_GRE_CALL_ID];
			flowPath6->in_gre_call_id = pFcIngressData->ingressCallid;
			flowPath6->in_protocol = FB_INPROTOCOL_ALL_ACCEPT;
			flowPath6->lock = TRUE;

#if defined(CONFIG_RTK_SOC_RTL8198D)
			// addSwOnly may be changed here
			if (rtk_fc_flow_limit_check((bool *)&addSwOnly, flowPath6->in_path) != SUCCESS) {
				return FAIL;
			}
#endif

			//prepare info that needed for adding G3 HW flow entry. Add pure sw flow here so don't care the content of g3IgrExtraInfo
			_rtk_fc_g3IgrExtraInfo_init(&g3IgrExtraInfo);
			pG3IgrExtraInfo = &g3IgrExtraInfo;
		
			flowHashIdx = _rtk_fc_flow_hashIndex(flowPathEntry, pFcIngressData->srcSVlanId, pFcIngressData->srcCVlanId, FALSE);
			portmap.macPortIdx = RTK_FC_MAC_PORT_PON;
		
			ret = rtk_fc_flow_add(&flow_idx, flowHashIdx, flowPath6, NULL,
									NULL, FALSE, portmap, addSwOnly, TRUE,
									pG3IgrExtraInfo, pPktHdr);
			if(ret != RT_ERR_RG_OK)
			{
				WARNING("path6 flow add fail!!! ret = %x", ret);
				return FAIL;
			}
			else
			{
				DEBUG("add PATH6 flow success, idx: %d, intf_idx=%d", flow_idx, NETIF_HWTOSW(wan_intf_idx));
				pNetif->outerHdrFlowIdx = flow_idx;
			}
		}
#endif
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		if((pNetif->dualHdr_HWState&(FC_DUALHDR_HW_CTRL_UP_REG_SETUP|FC_DUALHDR_HW_CTRL_DOWN_REG_SETUP))==0)
		{
			if(_rtk_fc_dualHeader_ctrl_setupPPTP(RTK_FC_DUAL_CONTROL_IDX(wan_intf_idx), pPktHdr, !pNetif->dualUniInfo.psGreInfo.outer_ipversion))
				DEBUG("[Dual][PPTP]Dual contrl entry add failed.");
			else
				pNetif->dualHdr_HWState|=FC_DUALHDR_HW_CTRL_DOWN_REG_SETUP;
		}

		if(pNetif->dualHdr_ds_clsIdx == CA_UINT32_INVALID)
		{
			rtk_fc_aclAndCf_reserved_dual_hdr_chk_t dual_hdr_chk = {0};
			rtk_fc_ingress_data_t *pFcIngressData = &skb->fcIngressData;

			dual_hdr_chk.packet_type			= RTK_FC_CLS_DUAL_PPTP_GRE;
			dual_hdr_chk.profile_idx			= 0; //WAN
			dual_hdr_chk.igr_intf_idx			= _rtk_fc_flow_intf_mapping_idx_get(pNetif->hwIdx);
			dual_hdr_chk.hash_tuple_idx 		= RTK_ASIC_FLOW_PROFILE_FLOW_5TUPLE;
			dual_hdr_chk.pptp_gre.gre_call_id	= pNetif->dualUniInfo.psGreInfo.call_id;
			//check ip header
			dual_hdr_chk.pptp_gre.is_ipv4 = !pNetif->dualUniInfo.psGreInfo.outer_ipversion;
			if(dual_hdr_chk.pptp_gre.is_ipv4)
			{
				dual_hdr_chk.pptp_gre.ipv4_dst_ip = pNetif->dualUniInfo.psGreInfo.localIp;
			}
			else
			{
				dual_hdr_chk.pptp_gre.ipv6_dst_ip[0] = pNetif->dualUniInfo.psGreInfo.localV6Ip.s6_addr32[0];
				dual_hdr_chk.pptp_gre.ipv6_dst_ip[1] = pNetif->dualUniInfo.psGreInfo.localV6Ip.s6_addr32[1];
				dual_hdr_chk.pptp_gre.ipv6_dst_ip[2] = pNetif->dualUniInfo.psGreInfo.localV6Ip.s6_addr32[2];
				dual_hdr_chk.pptp_gre.ipv6_dst_ip[3] = pNetif->dualUniInfo.psGreInfo.localV6Ip.s6_addr32[3];
			}
			//check vlan
			dual_hdr_chk.pptp_gre.vlan.vlan_check_en	= 1;
			dual_hdr_chk.pptp_gre.vlan.has_stag 		= (pFcIngressData->ingressTagif&SVLAN_TAGIF)?1:0;
			dual_hdr_chk.pptp_gre.vlan.svlan_id 		= (pFcIngressData->ingressTagif&SVLAN_TAGIF)?pFcIngressData->srcSVlanId:0;
			dual_hdr_chk.pptp_gre.vlan.has_ctag 		= (pFcIngressData->ingressTagif&CVLAN_TAGIF)?1:0;
			dual_hdr_chk.pptp_gre.vlan.cvlan_id 		= (pFcIngressData->ingressTagif&CVLAN_TAGIF)?pFcIngressData->srcCVlanId:0;
			ret = _rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CLS_TYPE_DUAL_HEADER_DS_CHECK, &dual_hdr_chk);
			if(ret != RTK_FC_RET_OK)
			{
				WARNING("add special ACL DUAL_HEADER_DS_CHECK failed!!! ret = %x", ret);
				return FAIL;
			}
			else
			{
				DEBUG("add special ACL DUAL_HEADER_DS_CHECK success, idx: %d, intf_idx=%d", dual_hdr_chk.rslt_idx, dual_hdr_chk.igr_intf_idx);
				pNetif->dualHdr_ds_clsIdx=dual_hdr_chk.rslt_idx;
			}
		}
#endif
	}

	return ret;
}


int _rtk_fc_flow_setupL2TP(rtk_fc_pktHdr_t *pPktHdr,struct sk_buff *skb, int wan_intf_idx)
{
	int ret=RTK_FC_RET_OK;
	rtk_fc_tableNetif_t *pNetif = &fc_db.netifTbl[NETIF_HWTOSW(wan_intf_idx)];
	
	if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
	{
		if(pNetif->outerHdrExtratagIdx == SIGNED_INVALID)
		{
			//1 1: Add flow extra tag
			// Generate IPv4+UDP+L2TP header
			int32 extraTagActListIdx = 0;
			u32 bufLen = 0;
			u8 contenBuffer[64]={0};
			u8 *outerHdr = &contenBuffer[0];
			TRACE("[Dual] Gen outer header and add to SRAM content buffer");

			// Prepare IP(20/40)+UDP(8)+L2TP(8)+PPP(4) for L2TP
			// L3 header --
			if(pNetif->dualUniInfo.l2tpInfo.outer_ipversion)
			{
				TRACE("[Dual][L2TP] Do not support outer non-ipv4 hdr");
				return FAIL;
			}
			else
			{
				*((uint16 *)(outerHdr + 0)) = htons(0x4500);	// ver, IHL
				//*((uint16 *)(outerHdr + 2)) = htons(0x0);	// total length	: updated by extra tag actions
				//*((uint16 *)(outerHdr + 4)) = htons(0x0);	// ipid		: updated by extra tag actions
				//*((uint16 *)(outerHdr + 6)) = htons(0x0);	// flags, frag offset
				*((uint16 *)(outerHdr + 8)) = htons(0x4011);	// ttl, protocol
				//*((uint16 *)(outerHdr + 10)) = htons(0x0);	// hdr checksum
				*((uint32 *)(outerHdr + 12)) = htonl(pNetif->dualUniInfo.l2tpInfo.localIp);		// src ip
				*((uint32 *)(outerHdr + 16)) = htonl(pNetif->dualUniInfo.l2tpInfo.remoteIp);	// dst ip
				bufLen += 20;
			}

			// L4 UDP --
			{
				*(uint16 *)(outerHdr + bufLen) = htons(pNetif->dualUniInfo.l2tpInfo.local_l4port);		// src port
				*(uint16 *)(outerHdr + bufLen + 2) = htons(pNetif->dualUniInfo.l2tpInfo.remote_l4port);	// dst port
				//*(uint16 *)(outerHdr + bufLen + 4) = htons(0x0);	// len: updated by extra tag actions
				//*(uint16 *)(outerHdr + bufLen + 6) = htons(0x0);	// checksum: updated by extra tag actions
				bufLen += 8;
			}
			// L2TP --
			{
				*(uint16 *)(outerHdr + bufLen) = htons(0x4002);	// Type(0), Length(1), Sequence(0), Offset(0), Priority(0), Version(2)
				//*(uint16 *)(outerHdr + bufLen +2) = htons(0x0);	//Total length: updated by extra tag actions
				*(uint16 *)(outerHdr + bufLen +4) = htons(pNetif->dualUniInfo.l2tpInfo.peer_tunnel_id);		//Peer's Tunnel ID
				*(uint16 *)(outerHdr + bufLen +6) = htons(pNetif->dualUniInfo.l2tpInfo.peer_session_id);	//Peer's Session ID
				bufLen += 8;
			}
			// PPP --
			{
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
				switch(pPktHdr->tunnelInfo.pppMode) // Address, Control, Protocol=IPv4
				{
					case RTK_FC_COMP_PPP_FF0300XX:
						outerHdr[bufLen]	= 0xff ;
						outerHdr[bufLen+1]	= 0x3 ;
						outerHdr[bufLen+2]	= 0x0 ;
						outerHdr[bufLen+3]	= 0x21 ;					
						break;
					case RTK_FC_COMP_PPP_FF03XX:
						outerHdr[bufLen]	= 0xff ;
						outerHdr[bufLen+1]	= 0x3 ;
						outerHdr[bufLen+2]	= 0x21 ;							
						break;
					case RTK_FC_COMP_PPP_00XX:
						outerHdr[bufLen]	= 0x0 ;
						outerHdr[bufLen+1]	= 0x21 ;							
						break;
					case RTK_FC_COMP_PPP_XX:
						outerHdr[bufLen]	= 0x21 ;						
						break;
					default:
						break;
				}
				bufLen += pPktHdr->tunnelInfo.pppMode;
#else
				*(uint32 *)(outerHdr + bufLen) = htonl(0xff030021);							// Address, Control, Protocol=IPv4
				bufLen += 4;
#endif
			}
			
			if((ret = _rtk_fc_extraTag_setupL2TP(wan_intf_idx, outerHdr, &extraTagActListIdx)) == SUCCESS){
				pNetif->outerHdrExtratagIdx = extraTagActListIdx;
				TRACE("[L2TP] extra tag index: %d", extraTagActListIdx);
			}
			else
				return FAIL;
			
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
			aal_pe_dual_content_entry_del(RTK_FC_DUAL_CONTROL_IDX(wan_intf_idx));
			if(aal_pe_dual_content_entry_add(RTK_FC_DUAL_CONTROL_IDX(wan_intf_idx), (l3pe_dual_tbl_entry_t *)outerHdr))
				WARNING("[Dual][L2TP] Dual content add failed.");
			if(_rtk_fc_dualHeader_ctrl_setupL2TP(RTK_FC_DUAL_CONTROL_IDX(wan_intf_idx), pPktHdr))
				WARNING("[Dual][L2TP] Dual contrl entry add failed.");
			pNetif->dualHdr_HWState |= FC_DUALHDR_HW_CTRL_UP_REG_SETUP;
#endif	

			if((ret = rtk_fc_dualHdrInfo_set(RTK_FC_FB_DUALHDR_OUTER_IPV4ID, wan_intf_idx, ntohs(pPktHdr->outer_iph->id)/*network endian*/)) != RT_ERR_RG_OK)
				WARNING("set ipid was fail, ret = %d", ret);
		}
	}

	if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM)
	{
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
		if(pNetif->outerHdrFlowIdx == SIGNED_INVALID)
		{
			rtk_fc_ingress_data_t *pFcIngressData = &skb->fcIngressData;
			rtk_fc_tableFlowEntry_t flowPathEntry;
			rtk_rg_asic_path6_entry_t *flowPath6 = &flowPathEntry.path6;
			rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo = NULL;
			rtk_fc_g3IgrExtraInfo_t g3IgrExtraInfo;
			rtk_fc_pmap_t portmap;
			uint32 flow_idx;
			int32 flowHashIdx = 0;
			uint8 addSwOnly = FALSE;
			int16 remoteGwMac = 0;

			if(_rtk_fc_lut_find(pFcIngressData->sa, &remoteGwMac) != RTK_FC_RET_OK)
				return FAIL;

			TRACE("[Dual][L2TP] prepare path6");

			//1 2: Add flow path6
			bzero(flowPath6, sizeof(rtk_rg_asic_path6_entry_t));
			flowPath6->valid = TRUE;
			flowPath6->in_path = FB_PATH_6;
			flowPath6->in_src_ip_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_SIP];
			flowPath6->in_src_ipv4_addr = ntohl(pNetif->dualUniInfo.l2tpInfo.remoteIp);
			flowPath6->in_dst_ip_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_DIP];
			flowPath6->in_dst_ipv4_addr = ntohl(pNetif->dualUniInfo.l2tpInfo.localIp);
			flowPath6->in_l4_src_port_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_SPORT];
			flowPath6->in_l4_src_port = ntohs(pNetif->dualUniInfo.l2tpInfo.remote_l4port);
			flowPath6->in_l4_dst_port_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_DPORT];
			flowPath6->in_l4_dst_port = ntohs(pNetif->dualUniInfo.l2tpInfo.local_l4port);
			flowPath6->in_intf_idx = wan_intf_idx;
			flowPath6->in_ctagif = (pFcIngressData->ingressTagif&CVLAN_TAGIF) ? TRUE : FALSE;
			flowPath6->in_stagif = (pFcIngressData->ingressTagif&SVLAN_TAGIF) ? TRUE : FALSE;
			flowPath6->in_pppoeif= (pFcIngressData->ingressTagif&PPPOE_TAGIF) ? TRUE : FALSE;
			flowPath6->in_pppoe_sid = (pFcIngressData->ingressTagif&PPPOE_TAGIF) ? pFcIngressData->sessionId : 0;
			flowPath6->in_src_mac_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_SMAC_IDX];
			flowPath6->in_smac_lut_idx = remoteGwMac;
			flowPath6->in_dst_mac_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_DMAC_IDX];
			flowPath6->in_dmac_lut_idx = pNetif->lutIdx;
			flowPath6->in_l2tpif = TRUE;
			flowPath6->in_l2tp_tunnel_id_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_L2TP_TUNNEL_ID];
			flowPath6->in_l2tp_tunnel_id = pNetif->dualUniInfo.l2tpInfo.tunnel_id;
			flowPath6->in_l2tp_session_id_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_L2TP_SID];
			flowPath6->in_l2tp_session_id = pNetif->dualUniInfo.l2tpInfo.session_id;
			flowPath6->in_protocol = FB_INPROTOCOL_ALL_ACCEPT;
			flowPath6->lock = TRUE;
		
#if defined(CONFIG_RTK_SOC_RTL8198D)
			// addSwOnly may be changed here
			if (rtk_fc_flow_limit_check((bool *)&addSwOnly, flowPath6->in_path) != SUCCESS) {
				return FAIL;
			}
#endif
			//prepare info that needed for adding G3 HW flow entry. Add pure sw flow here so don't care the content of g3IgrExtraInfo
			_rtk_fc_g3IgrExtraInfo_init(&g3IgrExtraInfo);
			pG3IgrExtraInfo = &g3IgrExtraInfo;
		
			flowHashIdx = _rtk_fc_flow_hashIndex(flowPathEntry, pFcIngressData->srcSVlanId, pFcIngressData->srcCVlanId, FALSE);
		
			portmap.macPortIdx = RTK_FC_MAC_PORT_PON;
		
			ret = rtk_fc_flow_add(&flow_idx, flowHashIdx, flowPath6, NULL,
								NULL, FALSE, portmap, addSwOnly, TRUE,
								pG3IgrExtraInfo, pPktHdr);
			if(ret == RT_ERR_RG_OK)
				pNetif->outerHdrFlowIdx = flow_idx;
			else
				WARNING("[L2TP] Fail to add path6 !!");
		
			TRACE("[L2TP] intf idx[%d], path 6 idx[%d]", wan_intf_idx, flowHashIdx);

		}
#endif
		//Add downstream dual header CLS
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		if((pNetif->dualHdr_HWState & (FC_DUALHDR_HW_CTRL_UP_REG_SETUP | FC_DUALHDR_HW_CTRL_DOWN_REG_SETUP))==0)
		{
			if(_rtk_fc_dualHeader_ctrl_setupL2TP(RTK_FC_DUAL_CONTROL_IDX(wan_intf_idx), pPktHdr))
				WARNING("[Dual][L2TP] Dual contrl entry add failed.");
			pNetif->dualHdr_HWState |= FC_DUALHDR_HW_CTRL_DOWN_REG_SETUP;
		}
		if(pNetif->dualHdr_ds_clsIdx == CA_UINT32_INVALID)
		{
			rtk_fc_ingress_data_t *pFcIngressData = &skb->fcIngressData;
			rtk_fc_aclAndCf_reserved_dual_hdr_chk_t dual_hdr_chk = {0};
			int ret;

			dual_hdr_chk.packet_type		= RTK_FC_CLS_DUAL_L2TP;
			dual_hdr_chk.profile_idx		= 0; //WAN
			dual_hdr_chk.igr_intf_idx		= _rtk_fc_flow_intf_mapping_idx_get(pNetif->hwIdx);
			dual_hdr_chk.hash_tuple_idx 	= RTK_ASIC_FLOW_PROFILE_FLOW_5TUPLE;
			dual_hdr_chk.l2tp.tunnel_id 	= pNetif->dualUniInfo.l2tpInfo.tunnel_id;
			dual_hdr_chk.l2tp.session_id	= pNetif->dualUniInfo.l2tpInfo.session_id;
			//check ip header
			dual_hdr_chk.l2tp.is_ipv4 = !pNetif->dualUniInfo.l2tpInfo.outer_ipversion;
			if(dual_hdr_chk.l2tp.is_ipv4)
			{
				dual_hdr_chk.l2tp.ipv4_dst_ip = pNetif->dualUniInfo.l2tpInfo.localIp;
			}
			else
			{
				dual_hdr_chk.l2tp.ipv6_dst_ip[0] = pNetif->dualUniInfo.l2tpInfo.localV6Ip.s6_addr32[0];
				dual_hdr_chk.l2tp.ipv6_dst_ip[1] = pNetif->dualUniInfo.l2tpInfo.localV6Ip.s6_addr32[1];
				dual_hdr_chk.l2tp.ipv6_dst_ip[2] = pNetif->dualUniInfo.l2tpInfo.localV6Ip.s6_addr32[2];
				dual_hdr_chk.l2tp.ipv6_dst_ip[3] = pNetif->dualUniInfo.l2tpInfo.localV6Ip.s6_addr32[3];
			}
			//check vlan
			dual_hdr_chk.l2tp.vlan.vlan_check_en	= 1;
			dual_hdr_chk.l2tp.vlan.has_stag 		= (pFcIngressData->ingressTagif&SVLAN_TAGIF) ? 1 : 0;
			dual_hdr_chk.l2tp.vlan.svlan_id 		= (pFcIngressData->ingressTagif&SVLAN_TAGIF) ? pFcIngressData->srcSVlanId : 0;
			dual_hdr_chk.l2tp.vlan.has_ctag 		= (pFcIngressData->ingressTagif&CVLAN_TAGIF) ? 1 : 0;
			dual_hdr_chk.l2tp.vlan.cvlan_id 		= (pFcIngressData->ingressTagif&CVLAN_TAGIF) ? pFcIngressData->srcCVlanId : 0;
			
			ret = _rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CLS_TYPE_DUAL_HEADER_DS_CHECK, &dual_hdr_chk);
			if(ret != RTK_FC_RET_OK)
			{
				WARNING("[L2TP] add special ACL DUAL_HEADER_DS_CHECK failed!!! ret = %x", ret);
				return FAIL;
			}
			else
			{
				DEBUG("[L2TP] add special ACL DUAL_HEADER_DS_CHECK success, idx: %d, intf_idx=%d", dual_hdr_chk.rslt_idx, dual_hdr_chk.igr_intf_idx);
				pNetif->dualHdr_ds_clsIdx = dual_hdr_chk.rslt_idx;
			}
		}
#endif
	}
	
	return ret;
}

int _rtk_fc_flow_setupIP4InIP6(rtk_fc_pktHdr_t *pPktHdr,struct sk_buff *skb, int wan_intf_idx)
{
	int ret=RTK_FC_RET_OK;

	rtk_fc_tableNetif_t *pNetif = &fc_db.netifTbl[NETIF_HWTOSW(wan_intf_idx)];
	int32  extraTagActListIdx = 0;
	uint32 ihl_tc_flow = 0x60000000;	// version: 6
	u8 contenBuffer[64]={0};
	u8 *outerHdr = &contenBuffer[0];

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	//setting pkt_fmr_idx
	if(pNetif->dualUniInfo.ip4Inip6_info.isMape)
	{
		if(pPktHdr->remarkDec.mapet_fmr)
		{
			if(pNetif->hwFmrIdx==FAIL)
				{WARNING("FMR TABLE NOT READY");return FAIL;}
			else
				pPktHdr->pkt_fmr_idx = pNetif->hwFmrIdx;
		}
		else
		{
			pPktHdr->pkt_fmr_idx = L3PE_DUAL_FMR_TBL_ENTRY_MAX;
		}
	}
#endif

	TRACE("[Dual] prepare DsLite tag");
#if defined(CONFIG_RTK_L34_XPON_PLATFORM) 
	if (pNetif->outerHdrFlowIdx==SIGNED_INVALID)	
	{
		uint8 addSwOnly = FALSE;
		rtk_fc_pmap_t portmap;
		rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo = NULL;
		rtk_fc_g3IgrExtraInfo_t g3IgrExtraInfo;
		rtk_fc_tableFlowEntry_t flowPathEntry;
		rtk_rg_asic_path6_entry_t *flowPath6 = &flowPathEntry.path6;
		int32 flowHashIdx = 0;
		uint32 flow_idx;
		int16 remoteGwMac = 0;

		if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
		{
			if(_rtk_fc_lut_find(pPktHdr->eth->h_dest, &remoteGwMac) != RTK_FC_RET_OK)
				return FAIL;
		}
		else if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM) 
		{
			if(_rtk_fc_lut_find(skb->fcIngressData.sa, &remoteGwMac) != RTK_FC_RET_OK)
				return FAIL;
		}		
	
		//1 2: Add flow path6
		bzero(flowPath6, sizeof(rtk_rg_asic_path6_entry_t));
		flowPath6->valid = TRUE;		
		flowPath6->in_path = FB_PATH_6;
		flowPath6->in_src_ip_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_SIP];
		flowPath6->in_src_ipv6_addr_hash = _rtk_rg_flowHashIPv6SrcAddr_get(pNetif->dualUniInfo.ip4Inip6_info.remote6Addr.s6_addr);
		flowPath6->in_dst_ip_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_DIP];
		flowPath6->in_dst_ipv6_addr_hash = _rtk_rg_flowHashIPv6DstAddr_get(pNetif->dualUniInfo.ip4Inip6_info.local6Addr.s6_addr);
		flowPath6->in_l4_src_port_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_SPORT];
		flowPath6->in_l4_src_port = 0;
		flowPath6->in_l4_dst_port_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_DPORT];
		flowPath6->in_l4_dst_port = 0;
		flowPath6->in_intf_idx = wan_intf_idx;
		flowPath6->in_src_mac_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_SMAC_IDX];
		flowPath6->in_smac_lut_idx = remoteGwMac;
		flowPath6->in_dst_mac_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_DMAC_IDX];
		flowPath6->in_dmac_lut_idx = pNetif->lutIdx;
		flowPath6->in_dsliteif = TRUE;
		flowPath6->in_protocol = FB_INPROTOCOL_ALL_ACCEPT;
		flowPath6->lock = TRUE;
		if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
		{
			flowPath6->in_ctagif = pPktHdr->cvh ? TRUE : FALSE;
			flowPath6->in_stagif = pPktHdr->svh ? TRUE : FALSE;
			flowPath6->in_pppoeif= pPktHdr->ppph ? TRUE : FALSE;
			flowPath6->in_pppoe_sid = pPktHdr->ppph ? ntohs(pPktHdr->ppph->sid) : 0;
		}
		else if (pPktHdr->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM)
		{
			flowPath6->in_ctagif = (skb->fcIngressData.ingressTagif&CVLAN_TAGIF) ? TRUE : FALSE;
			flowPath6->in_stagif = (skb->fcIngressData.ingressTagif&SVLAN_TAGIF) ? TRUE : FALSE;
			flowPath6->in_pppoeif= (skb->fcIngressData.ingressTagif&PPPOE_TAGIF) ? TRUE : FALSE;
			flowPath6->in_pppoe_sid = (skb->fcIngressData.ingressTagif&PPPOE_TAGIF) ? skb->fcIngressData.sessionId : 0;
		}

		
		//prepare info that needed for adding G3 HW flow entry. Add pure sw flow here so don't care the content of g3IgrExtraInfo
		_rtk_fc_g3IgrExtraInfo_init(&g3IgrExtraInfo);
		pG3IgrExtraInfo = &g3IgrExtraInfo;

		if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
		{
			flowHashIdx = _rtk_fc_flow_hashIndex(flowPathEntry, pPktHdr->svlanid, pPktHdr->cvlanid, FALSE);
		}
		else if (pPktHdr->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM)
		{
			flowHashIdx = _rtk_fc_flow_hashIndex(flowPathEntry, skb->fcIngressData.srcSVlanId, skb->fcIngressData.srcCVlanId, FALSE);
		}
		portmap.macPortIdx = RTK_FC_MAC_PORT_PON;
	
		ret = rtk_fc_flow_add(&flow_idx, flowHashIdx, flowPath6, NULL,
							NULL, FALSE, portmap, addSwOnly, TRUE,
							pG3IgrExtraInfo, pPktHdr);
		if(ret == RTK_FC_RET_OK){
			pNetif->outerHdrFlowIdx = flow_idx;
		}
		TABLE("Add IP4InIP6 Path6 Idx:%d",flow_idx);
	}
#endif	

	if(pNetif->outerHdrExtratagIdx ==SIGNED_INVALID)
	{
		//1 1: Add flow extra tag
		// Generate IPv6 header
		TRACE("[Dual] Gen outer header and add to SRAM content buffer");

		// Prepare IPv6(40) for DSLite
		// L3 header --
		*(uint32 *)(outerHdr + 0) = htonl(ihl_tc_flow); 				//ver, tc, flowlabel
		//*(uint16 *)(pData + 4) = htons(0x0);							//Payload length: updated by extra tag actions
		*(uint16 *)(outerHdr + 6) = htons(0x0400 | 0xff);					//Next header(IPIP:4), Hop limit
		memcpy(outerHdr + 8, &pNetif->dualUniInfo.ip4Inip6_info.local6Addr, IPV6_ADDR_LEN); // src ip
		memcpy(outerHdr + 24, &pNetif->dualUniInfo.ip4Inip6_info.remote6Addr, IPV6_ADDR_LEN);	// dst ip
		
		if((ret = _rtk_fc_extraTag_setupIP4InIP6(wan_intf_idx, outerHdr, &extraTagActListIdx)) == SUCCESS)
		{
			TRACE("Set up extraTag[%d]", extraTagActListIdx);
			pNetif->outerHdrExtratagIdx = extraTagActListIdx;
		}
		else
		{
			TRACE("extra tag setting fail");
			return FAIL;
		}
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		if(aal_pe_dual_content_entry_add(RTK_FC_DUAL_CONTROL_IDX(wan_intf_idx), (l3pe_dual_tbl_entry_t *)outerHdr))
			DEBUG("[Dual][DSlite/MAPE] Dual content add failed.");
		if(_rtk_fc_dualHeader_ctrl_setupIP4InIP6(RTK_FC_DUAL_CONTROL_IDX(wan_intf_idx), pPktHdr))
			DEBUG("[Dual][DSlite/MAPE] Dual contrl entry add failed.");
#endif
	}


#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)

	if(pNetif->dualHdr_ds_clsIdx == CA_UINT32_INVALID)
	{
		rtk_fc_aclAndCf_reserved_dual_hdr_chk_t dual_hdr_chk = {0};
	
		if(!pNetif->dualUniInfo.ip4Inip6_info.isMape)
		{
			TRACE("ADD Dslite Reserved ACL");
			//dslite
			dual_hdr_chk.packet_type		= RTK_FC_CLS_DUAL_DSLITE;
			dual_hdr_chk.profile_idx		= 0; //WAN
			dual_hdr_chk.igr_intf_idx		= _rtk_fc_flow_intf_mapping_idx_get(pNetif->hwIdx);
			dual_hdr_chk.hash_tuple_idx 	= RTK_ASIC_FLOW_PROFILE_FLOW_5TUPLE;

			dual_hdr_chk.dslite.src_ip[0] = ntohl(pNetif->dualUniInfo.ip4Inip6_info.remote6Addr.s6_addr32[0]);
			dual_hdr_chk.dslite.src_ip[1] = ntohl(pNetif->dualUniInfo.ip4Inip6_info.remote6Addr.s6_addr32[1]);
			dual_hdr_chk.dslite.src_ip[2] = ntohl(pNetif->dualUniInfo.ip4Inip6_info.remote6Addr.s6_addr32[2]);
			dual_hdr_chk.dslite.src_ip[3] = ntohl(pNetif->dualUniInfo.ip4Inip6_info.remote6Addr.s6_addr32[3]);
			//check vlan
			if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
			{
				dual_hdr_chk.dslite.vlan.vlan_check_en	= 1;
				dual_hdr_chk.dslite.vlan.has_stag			= (pPktHdr->svh) ? 1 : 0 ;
				dual_hdr_chk.dslite.vlan.svlan_id			= (pPktHdr->svh) ? pPktHdr->svlanid : 0 ;
				dual_hdr_chk.dslite.vlan.has_ctag			= (pPktHdr->cvh) ? 1 : 0 ;
				dual_hdr_chk.dslite.vlan.cvlan_id			= (pPktHdr->cvh) ? pPktHdr->cvlanid : 0 ;
			}
			else
			{
				dual_hdr_chk.dslite.vlan.vlan_check_en	= 1;
				dual_hdr_chk.dslite.vlan.has_stag			= (skb->fcIngressData.ingressTagif&SVLAN_TAGIF)?1 : 0 ;
				dual_hdr_chk.dslite.vlan.svlan_id			= (skb->fcIngressData.ingressTagif&SVLAN_TAGIF) ? skb->fcIngressData.srcSVlanId : 0 ;
				dual_hdr_chk.dslite.vlan.has_ctag			= (skb->fcIngressData.ingressTagif&CVLAN_TAGIF) ? 1 : 0 ;
				dual_hdr_chk.dslite.vlan.cvlan_id			= (skb->fcIngressData.ingressTagif&CVLAN_TAGIF) ? skb->fcIngressData.srcCVlanId : 0 ;
			}
		}
		else
		{
			TRACE("ADD MAPE Reserved ACL");
			//mape
			dual_hdr_chk.packet_type		= RTK_FC_CLS_DUAL_MAP_E;
			dual_hdr_chk.profile_idx		= 0; //WAN
			dual_hdr_chk.igr_intf_idx		= _rtk_fc_flow_intf_mapping_idx_get(pNetif->hwIdx);
			dual_hdr_chk.hash_tuple_idx 	= RTK_ASIC_FLOW_PROFILE_FLOW_5TUPLE;
	
			dual_hdr_chk.map_e.dst_ip[0] = ntohl(pNetif->dualUniInfo.ip4Inip6_info.local6Addr.s6_addr32[0]);
			dual_hdr_chk.map_e.dst_ip[1] = ntohl(pNetif->dualUniInfo.ip4Inip6_info.local6Addr.s6_addr32[1]);
			dual_hdr_chk.map_e.dst_ip[2] = ntohl(pNetif->dualUniInfo.ip4Inip6_info.local6Addr.s6_addr32[2]);
			dual_hdr_chk.map_e.dst_ip[3] = ntohl(pNetif->dualUniInfo.ip4Inip6_info.local6Addr.s6_addr32[3]);
			
			//check vlan
			if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
			{
				dual_hdr_chk.map_e.vlan.vlan_check_en	= 1;
				dual_hdr_chk.map_e.vlan.has_stag			= (pPktHdr->svh) ? 1 : 0 ;
				dual_hdr_chk.map_e.vlan.svlan_id			= (pPktHdr->svh) ? pPktHdr->svlanid : 0 ;
				dual_hdr_chk.map_e.vlan.has_ctag			= (pPktHdr->cvh) ? 1 : 0 ;
				dual_hdr_chk.map_e.vlan.cvlan_id			= (pPktHdr->cvh) ? pPktHdr->cvlanid : 0 ;
			}
			else
			{
				dual_hdr_chk.dslite.vlan.vlan_check_en	= 1;
				dual_hdr_chk.dslite.vlan.has_stag			= (skb->fcIngressData.ingressTagif&SVLAN_TAGIF) ?1 : 0 ;
				dual_hdr_chk.dslite.vlan.svlan_id			= (skb->fcIngressData.ingressTagif&SVLAN_TAGIF) ? skb->fcIngressData.srcSVlanId : 0 ;
				dual_hdr_chk.dslite.vlan.has_ctag			= (skb->fcIngressData.ingressTagif&CVLAN_TAGIF) ? 1 : 0 ;
				dual_hdr_chk.dslite.vlan.cvlan_id			= (skb->fcIngressData.ingressTagif&CVLAN_TAGIF) ? skb->fcIngressData.srcCVlanId : 0 ;			
			}
		}
		ret = _rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CLS_TYPE_DUAL_HEADER_DS_CHECK, &dual_hdr_chk);
		if(ret)
		{
			WARNING("add special ACL DUAL_HEADER_DS_CHECK failed!!! ret = %x", ret);
			return RTK_FC_RET_ERR;
		}
		else
		{
			DEBUG("add special ACL DUAL_HEADER_DS_CHECK success, idx: %d, intf_idx=%d", dual_hdr_chk.rslt_idx, dual_hdr_chk.igr_intf_idx);
			pNetif->dualHdr_ds_clsIdx=dual_hdr_chk.rslt_idx;
		}
	}	
#endif

	return ret;
}

extern 	rtk_fc_ret_t _rtk_fc_dualHeader_ctrl_setup6RD(int dualControlIdx, rtk_fc_pktHdr_t *pPktHdr);
int _rtk_fc_flow_setup6RD(rtk_fc_pktHdr_t *pPktHdr,struct sk_buff *skb, int wan_intf_idx)
{
	rtk_rg_err_code_t ret=RT_ERR_RG_OK;
	rtk_fc_tableNetif_t *pNetif = &fc_db.netifTbl[NETIF_HWTOSW(wan_intf_idx)];
	int32 extraTagActListIdx = 0;
	uint32 ihvl = 0x45000000;	// version: 4, hlen: 5
	int16 remoteGwMac = 0;

	if(fc_db.controlFuc.v6RDSwAcceleration_disable)
	{
		pNetif->outerHdrFlowIdx = SIGNED_INVALID;
		pNetif->outerHdrExtratagIdx = SIGNED_INVALID;
		return FAIL;
	}

	if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
	{
		if(_rtk_fc_lut_find(pPktHdr->eth->h_dest, &remoteGwMac) != RTK_FC_RET_OK)
			return FAIL;
	}
	else if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM) 
	{
		if(_rtk_fc_lut_find(skb->fcIngressData.sa, &remoteGwMac) != RTK_FC_RET_OK)
			return FAIL;
	}

	if(pNetif->outerHdrExtratagIdx ==SIGNED_INVALID)
	{
		//1 1: Add flow extra tag
		// Generate IPv4 header
		u8 contenBuffer[64]={0};
		u8 *outerHdr = &contenBuffer[0];

		// Prepare IPv4(20) for 6RD
		// L3 header --
		{
			*(uint32 *)(outerHdr + 0) = htonl(ihvl);					//ver, hlen
			*(uint16 *)(outerHdr + 8) = htons(0xff00 | 0x29);					//Hop limit,protocol(41)
			*(uint32 *)(outerHdr + 12) = htonl(pNetif->dualUniInfo.ip6Inip4_info.localAddr.s_addr);
			*(uint32 *)(outerHdr + 16) = htonl(pNetif->dualUniInfo.ip6Inip4_info.remoteAddr.s_addr);
		}

		if((ret = _rtk_fc_extraTag_setup6RD(wan_intf_idx, outerHdr, &extraTagActListIdx)) == SUCCESS)
		{
			TRACE("Set up extraTag[%d]", extraTagActListIdx);
			pNetif->outerHdrExtratagIdx = extraTagActListIdx;
		}

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		if(aal_pe_dual_content_entry_add(RTK_FC_DUAL_CONTROL_IDX(wan_intf_idx), (l3pe_dual_tbl_entry_t *)outerHdr))
			DEBUG("[Dual][6RD] Dual content add failed.");
		if(_rtk_fc_dualHeader_ctrl_setup6RD(RTK_FC_DUAL_CONTROL_IDX(wan_intf_idx), pPktHdr))
			DEBUG("[Dual][6RD] Dual contrl entry add failed.");
#endif		
	}

	//IPID
	if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM && !pNetif->dualUniInfo.ip6Inip4_info.upstream_seen)
	{
		//IPID 
		if(pPktHdr->outer_iph)
		{
			TRACE("Try to keep IP_ID:0x%x in netif info.",ntohs(pPktHdr->outer_iph->id));
			if((ret = rtk_fc_dualHdrInfo_set(RTK_FC_FB_DUALHDR_OUTER_IPV4ID, wan_intf_idx, ntohs(pPktHdr->outer_iph->id)/*network endian*/)) != RT_ERR_RG_OK)
				WARNING("set ipid was fail, ret = %d", ret);
		}
		pNetif->dualUniInfo.ip6Inip4_info.upstream_seen = 1;
	}

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	// 6RD: set l3 reserved cls
	//WARNING("move from _rtk_fc_flow_update6RD_by_igrPktHdr please update netif at rtk_fc_netif_update_dualinfo_by_dualHashKey and get patten from netif");
	if(pNetif->dualHdr_ds_clsIdx == CA_UINT32_INVALID)
	{
		rtk_fc_aclAndCf_reserved_dual_hdr_chk_t dual_hdr_chk = {0};

		// set downstream cls for ingress interface
		dual_hdr_chk.packet_type = RTK_FC_CLS_DUAL_6RD;
		dual_hdr_chk.profile_idx		= 0; //WAN
		dual_hdr_chk.igr_intf_idx	= _rtk_fc_flow_intf_mapping_idx_get(pNetif->hwIdx);
		dual_hdr_chk.hash_tuple_idx = RTK_ASIC_FLOW_PROFILE_FLOW_5TUPLE;
		dual_hdr_chk.six_rd.src_ip = pNetif->dualUniInfo.ip6Inip4_info.remoteAddr.s_addr;
		dual_hdr_chk.six_rd.dst_ip = pNetif->dualUniInfo.ip6Inip4_info.localAddr.s_addr;

		ret = _rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CLS_TYPE_DUAL_HEADER_DS_CHECK, &dual_hdr_chk);
		if(ret)
		{
			DEBUG("add special ACL DUAL_HEADER_DS_CHECK failed!!! ret = %x",ret);
			return RTK_FC_RET_ERR;
		}
		else
		{
			DEBUG("add special ACL DUAL_HEADER_DS_CHECK success, idx: 0x%x",dual_hdr_chk.rslt_idx);
			//_rtk_ca_cls_rule_dump(dual_hdr_chk.rslt_idx, 0, 0-RTK_CLS_TYPE_DUAL_HEADER_DS_CHECK, FALSE);
			pNetif->dualHdr_ds_clsIdx=dual_hdr_chk.rslt_idx;
		}
	}
	else
		DEBUG("Dual header netif downstream cls index:%d",pNetif->dualHdr_ds_clsIdx);
#endif

	return ret;
}

int _rtk_fc_flow_setupMAPT(rtk_fc_pktHdr_t *pPktHdr,struct sk_buff *skb, int wan_intf_idx)
{
	int ret = RTK_FC_RET_OK;
	
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	rtk_fc_tableNetif_t *pNetif = &fc_db.netifTbl[NETIF_HWTOSW(wan_intf_idx)];
	rtk_fc_ingress_data_t *pFcIngressData = &skb->fcIngressData;

	if(pPktHdr->ip6h)
	{
		//egress with ipv6 header, upstream
		int aal_ret;
		int i,prefix,longestMask=0,matchIdx=-1;
		unsigned short idx,bitMask;
		l3pe_dual_FMR_tbl_entry_t exist_entry;
		unsigned int ihl_tc_flow = 0x60000000;	// version: 6
		unsigned char contenBuffer[64]={0};
		unsigned char *outerHdr = &contenBuffer[0];

		if((pNetif->dualHdr_HWState&FC_DUALHDR_HW_CONTENT_BUF_SETUP)==0){
			TRACE("[MAPT][up] Gen outer header and add to SRAM content buffer");
			// Prepare IPv6(40) for MAPT sip translation
			// L3 header --
			*(uint32 *)(outerHdr + 0) = htonl(ihl_tc_flow);					//ver, tc, flowlabel
			//*(uint16 *)(pData + 4) = htons(0x0);							//Payload length
			//*(uint16 *)(outerHdr + 6) = htons(0x0400 | 0xff);				//Next header, Hop limit
			memcpy(outerHdr + 8, pNetif->dualUniInfo.maptInfo.localV6Ip.s6_addr, IPV6_ADDR_LEN);		// src ip
			memcpy(outerHdr + 24, pNetif->dualUniInfo.maptInfo.remoteV6Ip.s6_addr, IPV6_ADDR_LEN);	// dst ip
			aal_ret=aal_pe_dual_content_entry_add(RTK_FC_DUAL_CONTROL_IDX(wan_intf_idx), (l3pe_dual_tbl_entry_t *)outerHdr);
			if(aal_ret){
				DEBUG("[MAPT][up]Dual content add failed.(aal_ret=%d)",aal_ret);
				return FAIL;
			}
			//indicate this hw interface had already setup.
			pNetif->dualHdr_HWState|=FC_DUALHDR_HW_CONTENT_BUF_SETUP;
		}

		if(pPktHdr->remarkDec.mapet_fmr){
			//to CE, dip translation need fmr rule.
			for(i=0;i<L3PE_DUAL_FMR_TBL_ENTRY_MAX;i++){
				aal_ret=rtk_rg_l3_pe_dual_fmr_get(i, &exist_entry);
				if((aal_ret == ASIC_RET_ENTRYNOTRSVD) || (aal_ret == ASIC_RET_NOT_FOUND) || (aal_ret == ASIC_RET_NULL_POINTER))
					continue;

				prefix=exist_entry.fmr_pref_len;
				idx=(prefix>>3)&0xff;

				if((prefix&0x7)==0)
					bitMask=0;
				else
					bitMask=(0xff<<(8-(prefix&0x7)))&0xff;
				DEBUG("prefix = %d, idx = %d, bitMask =%02x, daddr=%pI6",prefix,idx,bitMask,pNetif->dualUniInfo.maptInfo.remoteV6Ip.s6_addr);
				if(prefix>longestMask && memcmp(exist_entry.fmr_pre_addr,pNetif->dualUniInfo.maptInfo.remoteV6Ip.s6_addr,idx)==0 &&
					((exist_entry.fmr_pre_addr[idx]&bitMask)==(pNetif->dualUniInfo.maptInfo.remoteV6Ip.s6_addr[idx]&bitMask)))
				{
					DEBUG("Match %d!!",i);
					longestMask=prefix;
					matchIdx=i;
				}
			}
			if(matchIdx<0){
				DEBUG("[MAPT][up]FMR rule compare fail..");
				return FAIL;
			}
			pPktHdr->pkt_fmr_idx=matchIdx;
			TRACE("[MAPT][up] to CE ready, fmr_idx=%d",pPktHdr->pkt_fmr_idx);
		}else{
			//to BR, dip translation need dual control setting only.
			pPktHdr->pkt_fmr_idx=L3PE_DUAL_FMR_TBL_ENTRY_MAX;
			if((pNetif->dualHdr_HWState&FC_DUALHDR_HW_CTRL_UP_REG_SETUP)==0){
				TRACE("[MAPT][up] setup control register");
				if(_rtk_fc_dualHeader_ctrl_setupMAPT(RTK_FC_DUAL_CONTROL_IDX(wan_intf_idx), pNetif->dualUniInfo.maptInfo.hwDraftVer, pNetif->dualUniInfo.maptInfo.hwPrefxLen)){
					DEBUG("[MAPT][up]Dual contrl entry add failed.");
					return FAIL;
				}
				//indicate this hw interface had already setup.
				pNetif->dualHdr_HWState|=FC_DUALHDR_HW_CTRL_UP_REG_SETUP;
			}
			TRACE("[MAPT][up] to BR ready, fmr_idx=%d, draft_ver=%d, prefix_len=%d",pPktHdr->pkt_fmr_idx,pNetif->dualUniInfo.maptInfo.hwDraftVer,pNetif->dualUniInfo.maptInfo.hwPrefxLen);
		}
	}
	else
	{
		//otherwise, downstream don't care fmr idx
		if(pPktHdr->remarkDec.mapet_fmr){
			//from CE sip translation have to lookup fmr table for longest prefix match, dip translation by dual control setting.
			TRACE("[MAPT][down] from CE ready, fmr_idx=%d",pPktHdr->pkt_fmr_idx);
		}else{
			//pPktHdr->fmr_idx=L3PE_DUAL_FMR_TBL_ENTRY_MAX;
			//from BR need dual control setting for both sip and dip translation.
			if((pNetif->dualHdr_HWState&FC_DUALHDR_HW_CTRL_UP_REG_SETUP)==0){
				TRACE("[MAPT][down] setup control register");
				if(_rtk_fc_dualHeader_ctrl_setupMAPT(RTK_FC_DUAL_CONTROL_IDX(wan_intf_idx), pNetif->dualUniInfo.maptInfo.hwDraftVer, pNetif->dualUniInfo.maptInfo.hwPrefxLen)){
					DEBUG("[MAPT][down]Dual contrl entry add failed.");
					return FAIL;
				}
				//indicate this hw interface had already setup.
				pNetif->dualHdr_HWState|=FC_DUALHDR_HW_CTRL_UP_REG_SETUP;
			}
			TRACE("[MAPT][down] from BR ready, fmr_idx=%d, draft_ver=%d, prefix_len=%d",pPktHdr->pkt_fmr_idx,pNetif->dualUniInfo.maptInfo.hwDraftVer,pNetif->dualUniInfo.maptInfo.hwPrefxLen);
		}
	}

	//setup downstream CLS
	if(pNetif->dualHdr_ds_clsIdx == CA_UINT32_INVALID)
	{
		rtk_fc_aclAndCf_reserved_dual_hdr_chk_t dual_hdr_chk = {0};
		int cls_ret;

		dual_hdr_chk.packet_type			= RTK_FC_CLS_DUAL_MAP_T;
		dual_hdr_chk.profile_idx			= 0; //WAN
		dual_hdr_chk.igr_intf_idx			= _rtk_fc_flow_intf_mapping_idx_get(wan_intf_idx);
		dual_hdr_chk.hash_tuple_idx			= RTK_ASIC_FLOW_PROFILE_FLOW_5TUPLE;
		dual_hdr_chk.map_t.dst_ip[0]		= ntohl(pNetif->dualUniInfo.maptInfo.localV6Ip.s6_addr32[0]);
		dual_hdr_chk.map_t.dst_ip[1]		= ntohl(pNetif->dualUniInfo.maptInfo.localV6Ip.s6_addr32[1]);
		dual_hdr_chk.map_t.dst_ip[2]		= ntohl(pNetif->dualUniInfo.maptInfo.localV6Ip.s6_addr32[2]);
		dual_hdr_chk.map_t.dst_ip[3]		= ntohl(pNetif->dualUniInfo.maptInfo.localV6Ip.s6_addr32[3]);

		//check vlan
		if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
		{
			dual_hdr_chk.map_t.vlan.vlan_check_en	= 1;
			dual_hdr_chk.map_t.vlan.has_stag			= (pPktHdr->svh) ? 1 : 0 ;
			dual_hdr_chk.map_t.vlan.svlan_id			= (pPktHdr->svh) ? pPktHdr->svlanid : 0 ;
			dual_hdr_chk.map_t.vlan.has_ctag			= (pPktHdr->cvh) ? 1 : 0 ;
			dual_hdr_chk.map_t.vlan.cvlan_id			= (pPktHdr->cvh) ? pPktHdr->cvlanid : 0 ;
		}
		else
		{
			dual_hdr_chk.map_t.vlan.vlan_check_en	= 1;
			dual_hdr_chk.map_t.vlan.has_stag			= (pFcIngressData->ingressTagif&SVLAN_TAGIF)?1 : 0 ;
			dual_hdr_chk.map_t.vlan.svlan_id			= (pFcIngressData->ingressTagif&SVLAN_TAGIF) ? pFcIngressData->srcSVlanId : 0 ;
			dual_hdr_chk.map_t.vlan.has_ctag			= (pFcIngressData->ingressTagif&CVLAN_TAGIF) ? 1 : 0 ;
			dual_hdr_chk.map_t.vlan.cvlan_id			= (pFcIngressData->ingressTagif&CVLAN_TAGIF) ? pFcIngressData->srcCVlanId : 0 ;
		}

		cls_ret = _rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CLS_TYPE_DUAL_HEADER_DS_CHECK, &dual_hdr_chk);
		if(cls_ret != RTK_FC_RET_OK)
		{
			WARNING("[MAPT][down] add special ACL DUAL_HEADER_DS_CHECK failed!!! acl_ret = %x", cls_ret);
			return FAIL;
		}
		else
		{
			DEBUG("[MAPT][down] add special ACL DUAL_HEADER_DS_CHECK success, idx: %x, intf_idx=%d", dual_hdr_chk.rslt_idx, dual_hdr_chk.igr_intf_idx);
			pNetif->dualHdr_ds_clsIdx = dual_hdr_chk.rslt_idx;
		}
#if defined(CONFIG_FC_RTL8277C_SERIES)
		//77c only, 07f no need
		//add another CLS for TCP flag packets using different hash profile
		dual_hdr_chk.packet_type			= RTK_FC_CLS_DUAL_MAP_T_TCP_FLAG0;
		dual_hdr_chk.hash_tuple_idx			= RTK_ASIC_FLOW_PROFILE_FLOW_5TUPLE_TCP_FLAG0;

		cls_ret = _rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CLS_TYPE_DUAL_HEADER_DS_CHECK, &dual_hdr_chk);
		if(cls_ret != RTK_FC_RET_OK)
		{
			WARNING("[MAPT][down] add TCP flag0 special ACL DUAL_HEADER_DS_CHECK failed!!! acl_ret = %x", cls_ret);
			assert_ok(_rtk_rg_aclAndCfReservedRuleDelSpecial(RTK_CLS_TYPE_DUAL_HEADER_DS_CHECK, &pNetif->dualHdr_ds_clsIdx));
			pNetif->dualHdr_ds_clsIdx = RTK_FC_UINT32_INVALID;
			return FAIL;
		}
		else
		{
			DEBUG("[MAPT][down] add TCP flag0 special ACL DUAL_HEADER_DS_CHECK success, idx: %x, intf_idx=%d", dual_hdr_chk.rslt_idx, dual_hdr_chk.igr_intf_idx);
			pNetif->dualHdr_ds_extra_clsIdx = dual_hdr_chk.rslt_idx;
		}
#endif
	}
#else
	ret = FAIL;
#endif

	return ret;
}

int _rtk_fc_flow_setupXLAT(rtk_fc_pktHdr_t *pPktHdr,struct sk_buff *skb, int wan_intf_idx)
{
	int ret = RTK_FC_RET_OK;
	
#if defined(CONFIG_FC_RTL9607F_SERIES)
	rtk_fc_tableNetif_t *pNetif = &fc_db.netifTbl[NETIF_HWTOSW(wan_intf_idx)];
	rtk_fc_ingress_data_t *pFcIngressData = &skb->fcIngressData;


	if(pPktHdr->ip6h)
	{
		//egress with ipv6 header, upstream
		int aal_ret;
#if 0
		int i,prefix,longestMask=0,matchIdx=-1;
		unsigned short idx,bitMask;
		l3pe_dual_FMR_tbl_entry_t exist_entry;
#endif
		unsigned int ihl_tc_flow = 0x60000000;	// version: 6
		unsigned char contenBuffer[64]={0};
		unsigned char *outerHdr = &contenBuffer[0];

		if((pNetif->dualHdr_HWState&FC_DUALHDR_HW_CONTENT_BUF_SETUP)==0)
		{
			TRACE("[XLAT] Gen outer header and add to SRAM content buffer");
			// Prepare IPv6(40) for MAPT sip translation
			// L3 header --
			*(uint32 *)(outerHdr + 0) = htonl(ihl_tc_flow);					//ver, tc, flowlabel
			//*(uint16 *)(pData + 4) = htons(0x0);							//Payload length
			//*(uint16 *)(outerHdr + 6) = htons(0x0400 | 0xff);				//Next header, Hop limit
			memcpy(outerHdr + 8, pNetif->dualUniInfo.xlatInfo.Ipv6Addr_local.s6_addr, IPV6_ADDR_LEN);		// src ip
			memcpy(outerHdr + 24, pNetif->dualUniInfo.xlatInfo.Ipv6Addr_remote.s6_addr, IPV6_ADDR_LEN);	// dst ip
			aal_ret=aal_pe_dual_content_entry_add(RTK_FC_DUAL_CONTROL_IDX(wan_intf_idx), (l3pe_dual_tbl_entry_t *)outerHdr);
			if(aal_ret){
				DEBUG("[XLAT]Dual content add failed.(aal_ret=%d)",aal_ret);
				return FAIL;
			}
			//indicate this hw interface had already setup.
			pNetif->dualHdr_HWState|=FC_DUALHDR_HW_CONTENT_BUF_SETUP;
		}


		//to BR, dip translation need dual control setting only.
		if((pNetif->dualHdr_HWState&FC_DUALHDR_HW_CTRL_UP_REG_SETUP)==0)
		{
			if(_rtk_fc_dualHeader_ctrl_setupXLAT(RTK_FC_DUAL_CONTROL_IDX(wan_intf_idx), pNetif->dualUniInfo.xlatInfo.Ipv6Addr_remotePreifx, pNetif->dualUniInfo.xlatInfo.Ipv6Addr_localPreifx))
			{
				DEBUG("[XLAT][up] Dual contrl entry add failed.");
				return FAIL;
			}
			//indicate this hw interface had already setup.
			pNetif->dualHdr_HWState|=FC_DUALHDR_HW_CTRL_UP_REG_SETUP;
			TRACE("[XLAT][up] setup control register");			
		}
	}
	else
	{
		//from BR need dual control setting for both sip and dip translation.
		if((pNetif->dualHdr_HWState&FC_DUALHDR_HW_CTRL_UP_REG_SETUP)==0){
			TRACE("[XLAT][down] setup control register");
			if(_rtk_fc_dualHeader_ctrl_setupXLAT(RTK_FC_DUAL_CONTROL_IDX(wan_intf_idx), pNetif->dualUniInfo.xlatInfo.Ipv6Addr_remotePreifx, pNetif->dualUniInfo.xlatInfo.Ipv6Addr_localPreifx)){
				DEBUG("[XLAT][down]Dual contrl entry add failed.");
				return FAIL;
			}
			//indicate this hw interface had already setup.
			pNetif->dualHdr_HWState|=FC_DUALHDR_HW_CTRL_UP_REG_SETUP;
		}	
	}

	//setup downstream CLS
	if(pNetif->dualHdr_ds_clsIdx == CA_UINT32_INVALID)
	{
		rtk_fc_aclAndCf_reserved_dual_hdr_chk_t dual_hdr_chk = {0};
		int cls_ret;

		dual_hdr_chk.packet_type			= RTK_FC_CLS_DUAL_XLAT;
		dual_hdr_chk.profile_idx			= 0; //WAN
		dual_hdr_chk.igr_intf_idx			= _rtk_fc_flow_intf_mapping_idx_get(wan_intf_idx);
		dual_hdr_chk.hash_tuple_idx			= RTK_ASIC_FLOW_PROFILE_FLOW_5TUPLE;
		dual_hdr_chk.xlat.dst_ip[0]		= ntohl(pNetif->dualUniInfo.xlatInfo.Ipv6Addr_local.s6_addr32[0]);
		dual_hdr_chk.xlat.dst_ip[1]		= ntohl(pNetif->dualUniInfo.xlatInfo.Ipv6Addr_local.s6_addr32[1]);
		dual_hdr_chk.xlat.dst_ip[2]		= ntohl(pNetif->dualUniInfo.xlatInfo.Ipv6Addr_local.s6_addr32[2]);
		dual_hdr_chk.xlat.dst_ip[3]		= ntohl(pNetif->dualUniInfo.xlatInfo.Ipv6Addr_local.s6_addr32[3]);
		if(pNetif->dualUniInfo.xlatInfo.Ipv6Addr_localPreifx&DUAL_IPV6_HASHMASK_PREFIX_96)
			dual_hdr_chk.xlat.dst_ip_subnet_size=96;
		else if(pNetif->dualUniInfo.xlatInfo.Ipv6Addr_localPreifx&DUAL_IPV6_HASHMASK_PREFIX_64)
			dual_hdr_chk.xlat.dst_ip_subnet_size=64;
		else if(pNetif->dualUniInfo.xlatInfo.Ipv6Addr_localPreifx&DUAL_IPV6_HASHMASK_PREFIX_56)
			dual_hdr_chk.xlat.dst_ip_subnet_size=56;
		else if(pNetif->dualUniInfo.xlatInfo.Ipv6Addr_localPreifx&DUAL_IPV6_HASHMASK_PREFIX_48)
			dual_hdr_chk.xlat.dst_ip_subnet_size=48;
		else if(pNetif->dualUniInfo.xlatInfo.Ipv6Addr_localPreifx&DUAL_IPV6_HASHMASK_PREFIX_40)
			dual_hdr_chk.xlat.dst_ip_subnet_size=40;
		else if(pNetif->dualUniInfo.xlatInfo.Ipv6Addr_localPreifx&DUAL_IPV6_HASHMASK_PREFIX_32)
			dual_hdr_chk.xlat.dst_ip_subnet_size=32;


		dual_hdr_chk.xlat.src_ip[0]		= ntohl(pNetif->dualUniInfo.xlatInfo.Ipv6Addr_remote.s6_addr32[0]);
		dual_hdr_chk.xlat.src_ip[1]		= ntohl(pNetif->dualUniInfo.xlatInfo.Ipv6Addr_remote.s6_addr32[1]);
		dual_hdr_chk.xlat.src_ip[2]		= ntohl(pNetif->dualUniInfo.xlatInfo.Ipv6Addr_remote.s6_addr32[2]);
		dual_hdr_chk.xlat.src_ip[3]		= ntohl(pNetif->dualUniInfo.xlatInfo.Ipv6Addr_remote.s6_addr32[3]);
		if(pNetif->dualUniInfo.xlatInfo.Ipv6Addr_remotePreifx&DUAL_IPV6_HASHMASK_PREFIX_96)
			dual_hdr_chk.xlat.src_ip_subnet_size=96;
		else if(pNetif->dualUniInfo.xlatInfo.Ipv6Addr_remotePreifx&DUAL_IPV6_HASHMASK_PREFIX_64)
			dual_hdr_chk.xlat.src_ip_subnet_size=64;
		else if(pNetif->dualUniInfo.xlatInfo.Ipv6Addr_remotePreifx&DUAL_IPV6_HASHMASK_PREFIX_56)
			dual_hdr_chk.xlat.src_ip_subnet_size=56;
		else if(pNetif->dualUniInfo.xlatInfo.Ipv6Addr_remotePreifx&DUAL_IPV6_HASHMASK_PREFIX_48)
			dual_hdr_chk.xlat.src_ip_subnet_size=48;
		else if(pNetif->dualUniInfo.xlatInfo.Ipv6Addr_remotePreifx&DUAL_IPV6_HASHMASK_PREFIX_40)
			dual_hdr_chk.xlat.src_ip_subnet_size=40;
		else if(pNetif->dualUniInfo.xlatInfo.Ipv6Addr_remotePreifx&DUAL_IPV6_HASHMASK_PREFIX_32)
			dual_hdr_chk.xlat.src_ip_subnet_size=32;


		//check vlan
		if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
		{
			dual_hdr_chk.xlat.vlan.vlan_check_en	= 1;
			dual_hdr_chk.xlat.vlan.has_stag			= (pPktHdr->svh) ? 1 : 0 ;
			dual_hdr_chk.xlat.vlan.svlan_id			= (pPktHdr->svh) ? pPktHdr->svlanid : 0 ;
			dual_hdr_chk.xlat.vlan.has_ctag			= (pPktHdr->cvh) ? 1 : 0 ;
			dual_hdr_chk.xlat.vlan.cvlan_id			= (pPktHdr->cvh) ? pPktHdr->cvlanid : 0 ;
		}
		else
		{
			dual_hdr_chk.xlat.vlan.vlan_check_en	= 1;
			dual_hdr_chk.xlat.vlan.has_stag			= (pFcIngressData->ingressTagif&SVLAN_TAGIF)?1 : 0 ;
			dual_hdr_chk.xlat.vlan.svlan_id			= (pFcIngressData->ingressTagif&SVLAN_TAGIF) ? pFcIngressData->srcSVlanId : 0 ;
			dual_hdr_chk.xlat.vlan.has_ctag			= (pFcIngressData->ingressTagif&CVLAN_TAGIF) ? 1 : 0 ;
			dual_hdr_chk.xlat.vlan.cvlan_id			= (pFcIngressData->ingressTagif&CVLAN_TAGIF) ? pFcIngressData->srcCVlanId : 0 ;
		}

		cls_ret = _rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CLS_TYPE_DUAL_HEADER_DS_CHECK, &dual_hdr_chk);
		if(cls_ret != RTK_FC_RET_OK)
		{
			WARNING("[XLAT] add special ACL DUAL_HEADER_DS_CHECK failed!!! acl_ret = %x", cls_ret);
			return FAIL;
		}
		else
		{
			DEBUG("[XLAT] add special ACL DUAL_HEADER_DS_CHECK success, idx: %x, intf_idx=%d", dual_hdr_chk.rslt_idx, dual_hdr_chk.igr_intf_idx);
			pNetif->dualHdr_ds_clsIdx = dual_hdr_chk.rslt_idx;
		}
	}
#else
	ret = FAIL;
#endif

	return ret;
}


int _rtk_fc_flow_setupVXLAN(rtk_fc_pktHdr_t *pPktHdr,struct sk_buff *skb, int hw_intf_idx)
{
	rtk_fc_ret_t ret=RT_ERR_RG_FAILED;
	rtk_fc_tableNetif_t *pNetif = &fc_db.netifTbl[NETIF_HWTOSW(hw_intf_idx)];
	int32 extraTagActListIdx = 0;
	int32 swNetifIdx = NETIF_HWTOSW(hw_intf_idx);

	//FIXME("WEN FIXME LATTER DUAL_MODIFY");
#if 0//defined(CONFIG_FC_RTL9607C_SERIES) || defined(CONFIG_FC_CA8277B_SERIES)		
		if( fc_db.controlFuc.special_fast_forward_mode==1U &&
			skb->fcIngressData.isDualHeader && 
			(skb->fcIngressData.ingressTagif & VXLAN_TAGIF) && 
			pPktHdr->dualHdrType == RTK_FC_DUALTYPE_NONE && 
			fc_db.vxlan_ds_fastForward_num < RTK_FC_VXLAN_DS_FF_NUM_MAX)
		{	
			//	For VXLAN down stream special fast forward
			//	Because need to add outter header's flow
			//	So we need to get outer header's netif index
			
			//outer_dev = rtk_fc_vxlan_getOuterHdrDestDev(pPktHdr, rtskb, RTK_FC_VXLAN_DOWNSTREAM);
			outer_dev = fc_db.netifTbl[pFcIngressData->vxlan_info.outer_intf_index].dev; //rtk_fc_vxlan_getOuterHdrDestDev(pPktHdr, rtskb, RTK_FC_L2DUAL_ACT_REMOVE);
			outer_netifIdx = rtk_fc_decideNetifIndex(RTK_FC_DEV_TYPE_INGRESS, outer_dev, rtskb, pPktHdr, FALSE);
			if(outer_dev!=NULL)
				DEBUG("[VXLAN][DS]Outer interface %s index %d",outer_dev->name,outer_netifIdx);
			else
			{
				DEBUG("[VXLAN][DS]Get outer interface failed!! sw-Only!");
				return FAILED;
			}
			
		}
			
		if( fc_db.controlFuc.special_fast_forward_mode==1U &&
			skb->fcIngressData.isDualHeader==0U && 
			pPktHdr->dualHdrType == RTK_FC_DUALTYPE_VXLAN &&
			fc_db.vxlan_us_fastForward_num < RTK_FC_VXLAN_US_FF_NUM_MAX)
		{	
			//	For VXLAN down stream special fast forward
			//	Because need to add outter header's flow
			//	So we need to get outer header's netif index
			/*
			outer_dev = fc_db.netifTbl[pPktHdr->vxlan_info.outer_intf_index].dev;//rtk_fc_vxlan_getOuterHdrSourceDev(pPktHdr, rtskb, RTK_FC_L2DUAL_ACT_ADD);
			outer_netifIdx = rtk_fc_decideNetifIndex(RTK_FC_DEV_TYPE_EGRESS, outer_dev, rtskb, pPktHdr, FALSE);
			if(outer_dev!=NULL)
				DEBUG("[VXLAN][US]Outer interface %s index %d",outer_dev->name,outer_netifIdx);
			else
			{
				DEBUG("[VXLAN][US]Get outer interface failed!! sw-Only!");
				swOnly = TRUE;
			}
			*/
		}
	
#endif

	if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM)
	{
		DEBUG(" VXLAN downstream add hardward table, sw index = %d",swNetifIdx);
		if(fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.valid)
		{
#if defined(CONFIG_FC_RTL9607C_SERIES) || defined(CONFIG_FC_CA8277B_SERIES)	
			DEBUG("VXLAN_DA = %pM",&fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.vxlan_DA[0]);
			memcpy(&pPktHdr->vxlan_info.vxlan_DA[0], &fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.vxlan_DA[0], 6);
#endif
			pPktHdr->vxlan_info.outer_cvlanid 	= fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_cvlan_id;
			pPktHdr->vxlan_info.outer_svlanid 	= fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_svlan_id;
			pPktHdr->vxlan_info.outer_cvh 		= fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_ctag_if;
			pPktHdr->vxlan_info.outer_svh 		= fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_stag_if;
			pPktHdr->vxlan_info.vni			  	= fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.vxlan_vni;
			pPktHdr->vxlan_info.outer_isV6 		= fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_is_v6;
			pPktHdr->vxlan_info.outer_ppph		= fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_pppoe_tag;
			pPktHdr->vxlan_info.outerTag_off	= fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_tag_len;
			if(pPktHdr->outer_ip6h)
			{	
				memcpy(&pPktHdr->vxlan_info.gateway_v6_ip, &fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_v6Ip, sizeof(struct in6_addr));
				memcpy(&pPktHdr->vxlan_info.remote_v6_ip, &fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_remote_v6Ip, sizeof(struct in6_addr));
			}
			else
			{
				DEBUG("Get gateway ip %x from netif[%d]", fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_ip,swNetifIdx);
				pPktHdr->vxlan_info.gateway_v4_ip = htonl(fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_ip);
				pPktHdr->vxlan_info.remote_v4_ip  = htonl(fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_remote_ip);
			}
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)			
			_rtk_fc_dualHeader_dual_ctrl_setupVXLAN(hw_intf_idx, pPktHdr, RTK_FC_DEV_TYPE_EGRESS);
#endif			

		}else
			DEBUG("[VXLAN][DS]Netif[%d] vxlan info is not ready yet.",NETIF_HWTOSW(hw_intf_idx));
		ret = RTK_FC_RET_OK;
	}
	else if (pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)	
	{
		//1 1: Add flow extra tag
		u8 contenBuffer[86]={0};//14 + 4 + 4 + 8 + 40 + 8 + 8 =  86
		u8 *outerHdr = &contenBuffer[0];
		int offset = 0, outer_pppoe_tag_off = 0, outer_ip_tag_off = 0, outer_udp_tag_off = 0;
		struct udphdr tmp_outer_udph;
/*
		// Prepare outer header for GRE eth br
		if(pPktHdr->greEthBr_info.outerTag_off > 64)
		{
			WARNING("[GRE_ETH_BR] Outer header Over size!!");
			return RTK_FC_RET_ERR_BUF_OVERFLOW;
		}
*/
		memcpy(outerHdr+offset, pPktHdr->outer_eth, sizeof(struct ethhdr));
		offset += sizeof(struct ethhdr);

		if(pPktHdr->outer_svh){
			memcpy(outerHdr+offset, pPktHdr->outer_svh, sizeof(struct vlan_hdr));
			offset += sizeof(struct vlan_hdr);
		}
		if(pPktHdr->outer_cvh){
			memcpy(outerHdr+offset, pPktHdr->outer_cvh, sizeof(struct vlan_hdr));
			offset += sizeof(struct vlan_hdr);
		}
		if(pPktHdr->outer_ppph){
			memcpy(outerHdr+offset, pPktHdr->outer_ppph, sizeof(struct pppoe_hdr)+2);
			outer_pppoe_tag_off = offset;
			offset += sizeof(struct pppoe_hdr) + 2;
		}
		
		if(pPktHdr->outer_iph)
		{
			memcpy(outerHdr+offset, pPktHdr->outer_iph, sizeof(struct iphdr));
			outer_ip_tag_off = offset;
			offset += sizeof(struct iphdr);

		}
		else if(pPktHdr->outer_ip6h)
		{
		
			memcpy(outerHdr+offset, pPktHdr->outer_ip6h, sizeof(struct ipv6hdr));
			outer_ip_tag_off = offset;
			offset += sizeof(struct ipv6hdr);

		}
		
		memcpy(&tmp_outer_udph, pPktHdr->outer_udph, sizeof(struct udphdr));
		tmp_outer_udph.check = 0;
		TRACE("Set outer udp checksum = 0!");
		
		memcpy(outerHdr+offset, &tmp_outer_udph, sizeof(struct udphdr));
		outer_udp_tag_off = offset;
		offset += sizeof(struct udphdr);
	
		//outer vxlan
		memcpy(outerHdr+offset, pPktHdr->vxlan_info.vxlanHdr, sizeof(rtk_fc_vxlanhdr_t));
		offset += sizeof(rtk_fc_vxlanhdr_t);


		// double check
		if(offset != pPktHdr->vxlan_info.outerTag_off)
		{
			WARNING("[GRE_ETH_BR] packet format length (%d) unmatch outer_tag_len (%d)!", offset, pPktHdr->vxlan_info.outerTag_off);
			return RTK_FC_RET_ERR_BUF_OVERFLOW;
		}

		if((ret = _rtk_fc_extraTag_setupVXLAN(hw_intf_idx, outerHdr, &extraTagActListIdx, pPktHdr->vxlan_info.outerTag_off, outer_pppoe_tag_off, outer_ip_tag_off, outer_udp_tag_off, pPktHdr->vxlan_info.outer_isV6)) == SUCCESS)
		{
			TRACE("Set up extraTag[%d]", extraTagActListIdx);
			pNetif->outerHdrExtratagIdx = extraTagActListIdx;
			if(pPktHdr->vxlan_info.outerTag_off > 64 )
				pNetif->use_two_extraTag = 1;
		}


#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		
		if( _rtk_fc_dualHeader_dual_content_setupVXLAN(hw_intf_idx, pPktHdr)!= SUCCESS)
			DEBUG("[Dual][VXLAN] Dual content add failed.");

		_rtk_fc_dualHeader_dual_ctrl_setupVXLAN(hw_intf_idx, pPktHdr, RTK_FC_DEV_TYPE_EGRESS);

#elif defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL9607C_SERIES)
		//1. content buffer without vlan/pppoe storage
		//2. Inner extra_len for 8277B
		if(fc_db.controlFuc.special_fast_forward_mode==1)
			_rtk_fc_dualHeader_vxlan_get_fastFwd_outer_content(pPktHdr);
#endif

	}

	return ret;
}

int _rtk_fc_flow_setupGreEthBr(rtk_fc_pktHdr_t *pPktHdr,struct sk_buff *skb, int hw_intf_idx)
{
	/*
		GRE_ETH_BR:
		9607C/8277B/8277C: SW only
		9607F: HW supported
	*/

	rtk_fc_ret_t ret = RT_ERR_RG_OK;
	rtk_fc_tableNetif_t *pNetif = &fc_db.netifTbl[NETIF_HWTOSW(hw_intf_idx)];
	int32 extraTagActListIdx = 0;
	uint offset = 0, outer_pppoe_tag_off = 0, outer_ip_tag_off = 0;
	bool outer_iph_is_v6 = FALSE;
#if defined(CONFIG_FC_RTL9607F_SERIES)
	rtk_fc_ingress_data_t *pFcIngressData = &skb->fcIngressData;
	bool if_need_to_set_dual_ctrl = (pNetif->dualHdr_ds_clsIdx == SIGNED_INVALID)?TRUE:FALSE;
#endif

	if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
	{
		//1 1: Add flow extra tag
		if(pNetif->outerHdrExtratagIdx == SIGNED_INVALID)
		{
			u8 contenBuffer[(RTK_FC_DUAL_CONTENT_BUF_ENT_OFFSET<<1)]={0};
			u8 *outerHdr = &(contenBuffer[0]);

			if(pNetif->dualUniInfo.greEthBrInfo.outerTag_len > (pNetif->hwEntryNum * RTK_FC_DUAL_CONTENT_BUF_ENT_OFFSET))
			{
				WARNING("[GRE_ETH_BR] Outer header Over size!! netif[%u]: dualUniInfo.greEthBrInfo.outerTag_len %u, hwEntryNum %u", NETIF_HWTOSW(hw_intf_idx), pNetif->dualUniInfo.greEthBrInfo.outerTag_len, pNetif->hwEntryNum);
				return RTK_FC_RET_ERR_BUF_OVERFLOW;
			}

			// Prepare outer header for GRE eth br
			memcpy(outerHdr+offset, pPktHdr->outer_eth, sizeof(struct ethhdr));
			offset += sizeof(struct ethhdr);

			if(pPktHdr->outer_svh){
				memcpy(outerHdr+offset, pPktHdr->outer_svh, sizeof(struct vlan_hdr));
				offset += sizeof(struct vlan_hdr);
			}
			if(pPktHdr->outer_cvh){
				memcpy(outerHdr+offset, pPktHdr->outer_cvh, sizeof(struct vlan_hdr));
				offset += sizeof(struct vlan_hdr);
			}
			if(pPktHdr->outer_ppph){
				memcpy(outerHdr+offset, pPktHdr->outer_ppph, sizeof(struct pppoe_hdr)+2);
				outer_pppoe_tag_off = offset;
				offset += sizeof(struct pppoe_hdr) + 2;
			}

			if(pPktHdr->outer_iph)
			{
				memcpy(outerHdr+offset, pPktHdr->outer_iph, sizeof(struct iphdr));
				outer_ip_tag_off = offset;
				offset += sizeof(struct iphdr);
			}
			else if(pPktHdr->outer_ip6h)
			{
				memcpy(outerHdr+offset, pPktHdr->outer_ip6h, sizeof(struct ipv6hdr));
				outer_ip_tag_off = offset;
				offset += sizeof(struct ipv6hdr);
				outer_iph_is_v6 = TRUE;
			}

			//gre tag
			memcpy(outerHdr+offset, pPktHdr->greh, sizeof(struct gre_base_hdr));
			offset += sizeof(struct gre_base_hdr);

			// double check
			if(offset != pNetif->dualUniInfo.greEthBrInfo.outerTag_len)
			{
				WARNING("[GRE_ETH_BR][upstream] packet format length (%d) unmatch outer_tag_len (%d)!", offset, pNetif->dualUniInfo.greEthBrInfo.outerTag_len);
				return RTK_FC_RET_ERR_BUF_OVERFLOW;
			}

			if((ret = _rtk_fc_extraTag_setupGreEthBr(hw_intf_idx, outerHdr, &extraTagActListIdx, pPktHdr->greEthBr_info.outerTag_off, outer_pppoe_tag_off, outer_ip_tag_off, outer_iph_is_v6)) == SUCCESS)
			{
				TRACE("Set up extraTag[%d]", extraTagActListIdx);
				pNetif->outerHdrExtratagIdx = extraTagActListIdx;
			}
			else
			{
				TRACE("Fail to set up extraTag");
				return RTK_FC_RET_ERR;
			}

#if defined(CONFIG_FC_RTL9607F_SERIES)
			if(aal_pe_dual_content_entry_add(RTK_FC_DUAL_CONTROL_IDX(hw_intf_idx), (l3pe_dual_tbl_entry_t *)outerHdr))
			{
				DEBUG("[Dual][GRE_ETH_BR] Dual content add failed.");
				return RTK_FC_RET_ERR;
			}
			if(pNetif->hwEntryNum == 2)
			{
				if(aal_pe_dual_content_entry_add(RTK_FC_DUAL_CONTROL_IDX(hw_intf_idx)+1, (l3pe_dual_tbl_entry_t *)(outerHdr+RTK_FC_DUAL_CONTENT_BUF_ENT_OFFSET)))
				{
					DEBUG("[Dual][GRE_ETH_BR] The second Dual content add failed.");
					return RTK_FC_RET_ERR;
				}
			}
			//IPID
			if((ret = rtk_fc_dualHdrInfo_set(RTK_FC_FB_DUALHDR_OUTER_IPV4ID, hw_intf_idx, ntohs(pPktHdr->outer_iph->id)/*network endian*/)) != RTK_FC_RET_OK)
				WARNING("set ipid was fail, ret = %d", ret);
#endif

			DEBUG("[Dual][GRE_ETH_BR] outer_pppoe_tag_off: %u, outer_ip_tag_off: %u", outer_pppoe_tag_off, outer_ip_tag_off);
		}
#if defined(CONFIG_FC_RTL9607F_SERIES)
		else
		{
			// pNetif->outerHdrExtratagIdx != SIGNED_INVALID
			if(if_need_to_set_dual_ctrl)
			{
				// prepare dual ctrl info
				offset += sizeof(struct ethhdr);

				if(pPktHdr->outer_svh)
					offset += sizeof(struct vlan_hdr);
				if(pPktHdr->outer_cvh)
					offset += sizeof(struct vlan_hdr);

				if(pPktHdr->outer_ppph){
					outer_pppoe_tag_off = offset;
					offset += sizeof(struct pppoe_hdr) + 2;
				}

				if(pPktHdr->outer_iph){
					outer_ip_tag_off = offset;
					offset += sizeof(struct iphdr);
				}
				else if(pPktHdr->outer_ip6h)
				{
					outer_ip_tag_off = offset;
					offset += sizeof(struct ipv6hdr);
					outer_iph_is_v6 = TRUE;
				}

				//gre tag
				offset += sizeof(struct gre_base_hdr);

				// double check
				if(offset != pNetif->dualUniInfo.greEthBrInfo.outerTag_len)
				{
					WARNING("[GRE_ETH_BR][upstream] packet format length (%d) unmatch outer_tag_len (%d)!", offset, pNetif->dualUniInfo.greEthBrInfo.outerTag_len);
					return RTK_FC_RET_ERR_BUF_OVERFLOW;
				}
				DEBUG("[Dual][GRE_ETH_BR] outer_pppoe_tag_off: %u, outer_ip_tag_off: %u", outer_pppoe_tag_off, outer_ip_tag_off);
			}
		}
#endif
	}
	else if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_DOWNSTREAM)
	{
#if defined(CONFIG_FC_RTL9607F_SERIES)
		if(if_need_to_set_dual_ctrl)
		{
			// prepare dual ctrl info
			offset += sizeof(struct ethhdr);

			if(pFcIngressData->ingressTagif & OUTER_SVLAN_TAGIF)
				offset += sizeof(struct vlan_hdr);
			if(pFcIngressData->ingressTagif & OUTER_CVLAN_TAGIF)
				offset += sizeof(struct vlan_hdr);

			if(pFcIngressData->ingressTagif & OUTER_PPPOE_TAGIF){
				outer_pppoe_tag_off = offset;
				offset += sizeof(struct pppoe_hdr) + 2;
			}

			if(pNetif->dualUniInfo.greEthBrInfo.outer_ipversion == 0){
				outer_ip_tag_off = offset;
				offset += sizeof(struct iphdr);
			}
			else
			{
				outer_ip_tag_off = offset;
				offset += sizeof(struct ipv6hdr);
				outer_iph_is_v6 = TRUE;
			}

			//gre tag
			offset += sizeof(struct gre_base_hdr);

			// double check
			if(offset != pNetif->dualUniInfo.greEthBrInfo.outerTag_len)
			{
				WARNING("[GRE_ETH_BR][downstream] packet format length (%d) unmatch outer_tag_len (%d)!", offset, pNetif->dualUniInfo.greEthBrInfo.outerTag_len);
				return RTK_FC_RET_ERR_BUF_OVERFLOW;
			}
			DEBUG("[Dual][GRE_ETH_BR] outer_pppoe_tag_off: %u, outer_ip_tag_off: %u", outer_pppoe_tag_off, outer_ip_tag_off);
		}
#endif
	}
	else
	{
		WARNING("packet direction (%u) not support!", pPktHdr->direction);
		return RTK_FC_RET_ERR;
	}


#if defined(CONFIG_FC_RTL9607F_SERIES)
	if(pNetif->dualHdr_ds_clsIdx == SIGNED_INVALID)
	{
		//RTK_FC_CLS_DUAL_L2GRE
		rtk_fc_aclAndCf_reserved_dual_hdr_chk_t dual_hdr_chk = {0};
		TRACE("ADD GRE_ETH_BR Reserved ACL for RTK_FC_CLS_DUAL_L2GRE");
		{
			dual_hdr_chk.packet_type		= RTK_FC_CLS_DUAL_L2GRE;
			dual_hdr_chk.profile_idx		= 0; //WAN
			dual_hdr_chk.igr_intf_idx		= _rtk_fc_flow_intf_mapping_idx_get(pNetif->hwIdx);
			dual_hdr_chk.hash_tuple_idx 	= RTK_ASIC_FLOW_PROFILE_FLOW_5TUPLE;

			dual_hdr_chk.l2_gre.is_ipv4 = !pNetif->dualUniInfo.greEthBrInfo.outer_ipversion;

			if(dual_hdr_chk.l2_gre.is_ipv4)
			{
				dual_hdr_chk.l2_gre.ipv4_src_ip = ntohl(pNetif->dualUniInfo.greEthBrInfo.remoteIp);
				dual_hdr_chk.l2_gre.ipv4_dst_ip = ntohl(pNetif->dualUniInfo.greEthBrInfo.localIp);
			}
			else
			{
				dual_hdr_chk.l2_gre.ipv6_src_ip[0] = ntohl(pNetif->dualUniInfo.greEthBrInfo.remoteV6Ip.s6_addr32[0]);
				dual_hdr_chk.l2_gre.ipv6_src_ip[1] = ntohl(pNetif->dualUniInfo.greEthBrInfo.remoteV6Ip.s6_addr32[1]);
				dual_hdr_chk.l2_gre.ipv6_src_ip[2] = ntohl(pNetif->dualUniInfo.greEthBrInfo.remoteV6Ip.s6_addr32[2]);
				dual_hdr_chk.l2_gre.ipv6_src_ip[3] = ntohl(pNetif->dualUniInfo.greEthBrInfo.remoteV6Ip.s6_addr32[3]);
				dual_hdr_chk.l2_gre.ipv6_dst_ip[0] = ntohl(pNetif->dualUniInfo.greEthBrInfo.localV6Ip.s6_addr32[0]);
				dual_hdr_chk.l2_gre.ipv6_dst_ip[1] = ntohl(pNetif->dualUniInfo.greEthBrInfo.localV6Ip.s6_addr32[1]);
				dual_hdr_chk.l2_gre.ipv6_dst_ip[2] = ntohl(pNetif->dualUniInfo.greEthBrInfo.localV6Ip.s6_addr32[2]);
				dual_hdr_chk.l2_gre.ipv6_dst_ip[3] = ntohl(pNetif->dualUniInfo.greEthBrInfo.localV6Ip.s6_addr32[3]);
			}
			ret = _rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CLS_TYPE_DUAL_HEADER_DS_CHECK, &dual_hdr_chk);
			if(ret != RTK_FC_RET_OK)
			{
				WARNING("add special ACL DUAL_HEADER_DS_CHECK failed!!! ret = %x", ret);
				return RTK_FC_RET_ERR;
			}
			else
			{
				DEBUG("add special ACL DUAL_HEADER_DS_CHECK success, idx: %d, intf_idx: %d", dual_hdr_chk.rslt_idx, dual_hdr_chk.igr_intf_idx);
				pNetif->dualHdr_ds_clsIdx = dual_hdr_chk.rslt_idx;
			}
		}

		if(_rtk_fc_dualHeader_ctrl_setupGreEthBr(RTK_FC_DUAL_CONTROL_IDX(hw_intf_idx), pPktHdr, pNetif, outer_pppoe_tag_off, outer_ip_tag_off))
		{
			DEBUG("[Dual][GRE_ETH_BR] Dual contrl entry add failed.");
			return RTK_FC_RET_ERR;
		}
	}
	if(pNetif->dualHdr_ds_extra_clsIdx == SIGNED_INVALID)
	{
		//RTK_FC_CLS_DUAL_L2GRE_NON_IP
		rtk_fc_aclAndCf_reserved_dual_hdr_chk_t dual_hdr_chk = {0};
		TRACE("ADD GRE_ETH_BR Reserved ACL for RTK_FC_CLS_DUAL_L2GRE_NON_IP");
		{
			dual_hdr_chk.packet_type		= RTK_FC_CLS_DUAL_L2GRE_NON_IP;
			dual_hdr_chk.profile_idx		= 0; //WAN
			dual_hdr_chk.igr_intf_idx		= _rtk_fc_flow_intf_mapping_idx_get(pNetif->hwIdx);
			dual_hdr_chk.hash_tuple_idx 	= RTK_ASIC_FLOW_PROFILE_FLOW_2TUPLE;

			dual_hdr_chk.l2_gre.is_ipv4 = !pNetif->dualUniInfo.greEthBrInfo.outer_ipversion;

			if(dual_hdr_chk.l2_gre.is_ipv4)
			{
				dual_hdr_chk.l2_gre.ipv4_src_ip = ntohl(pNetif->dualUniInfo.greEthBrInfo.remoteIp);
				dual_hdr_chk.l2_gre.ipv4_dst_ip = ntohl(pNetif->dualUniInfo.greEthBrInfo.localIp);
			}
			else
			{
				dual_hdr_chk.l2_gre.ipv6_src_ip[0] = ntohl(pNetif->dualUniInfo.greEthBrInfo.remoteV6Ip.s6_addr32[0]);
				dual_hdr_chk.l2_gre.ipv6_src_ip[1] = ntohl(pNetif->dualUniInfo.greEthBrInfo.remoteV6Ip.s6_addr32[1]);
				dual_hdr_chk.l2_gre.ipv6_src_ip[2] = ntohl(pNetif->dualUniInfo.greEthBrInfo.remoteV6Ip.s6_addr32[2]);
				dual_hdr_chk.l2_gre.ipv6_src_ip[3] = ntohl(pNetif->dualUniInfo.greEthBrInfo.remoteV6Ip.s6_addr32[3]);
				dual_hdr_chk.l2_gre.ipv6_dst_ip[0] = ntohl(pNetif->dualUniInfo.greEthBrInfo.localV6Ip.s6_addr32[0]);
				dual_hdr_chk.l2_gre.ipv6_dst_ip[1] = ntohl(pNetif->dualUniInfo.greEthBrInfo.localV6Ip.s6_addr32[1]);
				dual_hdr_chk.l2_gre.ipv6_dst_ip[2] = ntohl(pNetif->dualUniInfo.greEthBrInfo.localV6Ip.s6_addr32[2]);
				dual_hdr_chk.l2_gre.ipv6_dst_ip[3] = ntohl(pNetif->dualUniInfo.greEthBrInfo.localV6Ip.s6_addr32[3]);
			}
			ret = _rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CLS_TYPE_DUAL_HEADER_DS_CHECK, &dual_hdr_chk);
			if(ret != RTK_FC_RET_OK)
			{
				WARNING("add special ACL DUAL_HEADER_DS_CHECK failed!!! ret = %x", ret);
				return RTK_FC_RET_ERR;
			}
			else
			{
				DEBUG("add special ACL DUAL_HEADER_DS_CHECK success, idx: %d, intf_idx: %d", dual_hdr_chk.rslt_idx, dual_hdr_chk.igr_intf_idx);
				pNetif->dualHdr_ds_extra_clsIdx = dual_hdr_chk.rslt_idx;
			}
		}
	}
#endif
	return ret;
}

#if 0
/* FCv3_0 WARNING:DO NOT USING THIS FUNCTION ,REMOVE LATTER */
static int _rtk_fc_flow_updateL2TP_by_igrPktHdr(rtk_fc_pktHdr_t *pPktHdr, uint32 path6Idx)
{
	rtk_rg_asic_path6_entry_t *flowPath6 = &fc_db.flowTbl[path6Idx].pFlowEntry->path6;
	rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo = NULL;
	rtk_fc_g3IgrExtraInfo_t g3IgrExtraInfo;

	//prepare info that needed for adding G3 HW flow entry. Add pure sw flow here so don't care the content of g3IgrExtraInfo
#if defined(CONFIG_RTK_L34_G3_PLATFORM)	
	if(!FLOW_INFO_IS_SET(&fc_db.flowTbl[path6Idx], FLOW_INFO_SOFTWARE_ONLY))
		WARNING("Please check! Path 6 flow entry should be SW entry.");
#endif	
	_rtk_fc_g3IgrExtraInfo_init(&g3IgrExtraInfo);
	pG3IgrExtraInfo = &g3IgrExtraInfo;


	flowPath6->valid = TRUE;
	flowPath6->in_l2tpif = TRUE;
	flowPath6->in_l2tp_tunnel_id_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_L2TP_TUNNEL_ID];
	flowPath6->in_l2tp_tunnel_id = pPktHdr->tunnelInfo.l2tp.tunnelid;
	flowPath6->in_l2tp_session_id_check = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_L2TP_SID];
	flowPath6->in_l2tp_session_id = pPktHdr->tunnelInfo.l2tp.sessionid;
	flowPath6->lock = TRUE;

	ASSERT_EQ(RTK_RG_ASIC_FLOWPATH_SET(&path6Idx, flowPath6, pG3IgrExtraInfo, FLOW_INFO_IS_SET(&fc_db.flowTbl[path6Idx], FLOW_INFO_SOFTWARE_ONLY)), RTK_FC_RET_OK);
	
	return SUCCESS;
}

/* FCv3_0 WARNING:DO NOT USING THIS FUNCTION ,REMOVE LATTER */
static int _rtk_fc_flow_updateIP4InIP6_by_igrPktHdr(rtk_fc_pktHdr_t *pPktHdr, uint32 path6Idx)
{
	rtk_rg_asic_path6_entry_t *flowPath6 = &fc_db.flowTbl[path6Idx].pFlowEntry->path6;
	rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo = NULL;
	rtk_fc_g3IgrExtraInfo_t g3IgrExtraInfo;

	TRACE("dslite path6Idx=%d flowPath6=%p",path6Idx,flowPath6);
	//prepare info that needed for adding G3 HW flow entry. Add pure sw flow here so don't care the content of g3IgrExtraInfo

	_rtk_fc_g3IgrExtraInfo_init(&g3IgrExtraInfo);
	pG3IgrExtraInfo = &g3IgrExtraInfo;

	flowPath6->valid = TRUE;
	flowPath6->in_dsliteif = TRUE;
	flowPath6->lock = TRUE;

	ASSERT_EQ(RTK_RG_ASIC_FLOWPATH_SET(&path6Idx, flowPath6, pG3IgrExtraInfo, FLOW_INFO_IS_SET(&fc_db.flowTbl[path6Idx], FLOW_INFO_SOFTWARE_ONLY)), RTK_FC_RET_OK);

	return SUCCESS;
}

/* FCv3_0 WARNING:DO NOT USING THIS FUNCTION ,REMOVE LATTER */
static int _rtk_fc_flow_update6RD_by_igrPktHdr(rtk_fc_pktHdr_t *pPktHdr, uint32 path6Idx)
{
	rtk_rg_asic_path6_entry_t *flowPath6 = &fc_db.flowTbl[path6Idx].pFlowEntry->path6;
	rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo = NULL;
	rtk_fc_g3IgrExtraInfo_t g3IgrExtraInfo;

	//prepare info that needed for adding G3 HW flow entry. Add pure sw flow here so don't care the content of g3IgrExtraInfo
#if defined(CONFIG_RTK_L34_G3_PLATFORM)	
	if(!FLOW_INFO_IS_SET(&fc_db.flowTbl[path6Idx], FLOW_INFO_SOFTWARE_ONLY))
		WARNING("Please check! Path 6 flow entry should be SW entry.");
#endif	
	_rtk_fc_g3IgrExtraInfo_init(&g3IgrExtraInfo);
	pG3IgrExtraInfo = &g3IgrExtraInfo;


	flowPath6->valid = TRUE;
	flowPath6->rsvd_in_6rdif = TRUE;
	flowPath6->lock = TRUE;

	ASSERT_EQ(RTK_RG_ASIC_FLOWPATH_SET(&path6Idx, flowPath6, pG3IgrExtraInfo, FLOW_INFO_IS_SET(&fc_db.flowTbl[path6Idx], FLOW_INFO_SOFTWARE_ONLY)), RTK_FC_RET_OK);

	return SUCCESS;
}
#endif

static int _rtk_fc_applyExtraTagAction(int extraTagIdx, rtk_rg_asic_extraTagAction_t *pExtAct, struct rt_skbuff *rtskb, u32 pktBufOff, rtk_fc_pktHdr_t *pPktHdr)
{
	u32 value = 0;
	u8 actBit = pExtAct->type1.act_bit;
	int TUNN_tag_len = fc_db.extraTagList[extraTagIdx].contentLen;
	u8 *pData = RTSKB_DATA(rtskb);
	
	if(actBit == FB_EXTG_ACTBIT_1)
	{
		u8 *insertBuf = &fc_db.extraTagContentBuffer[pExtAct->type1.src_addr_offset];
		// insert_location: Insert outer header after PPP header(PPPoE+PPP).
				// if without PPP header, insert outer header after Ethertype(IPoE).
		RTK_FC_HOOK_PS_SKB_SKB_PUSH(RTSKB_SKB(rtskb), TUNN_tag_len, &pData);
		memcpy(pData, pData + TUNN_tag_len, pktBufOff);
		RTSKB_MAC_HEADER(rtskb) -= (TUNN_tag_len);
		// reset DMAC and SMAC pointer
		pPktHdr->eth=(struct ethhdr *)pData;
		
		// copy outer hdr buffer
		memcpy(pData+pktBufOff, insertBuf, TUNN_tag_len);

		pPktHdr->outer_iph = (struct iphdr *)(pData+pktBufOff);	//record outer header pointer

		DEBUG("[Dual] actionbit %d - insert outer hdr buffer", actBit);
		//if(fc_db.debugLevel & FC_DEBUG_LEVEL_DEBUG){
		//	memDump(pData, pktBufOff+TUNN_tag_len+20, "insert outer hdr");
		//}
	}

	if(actBit == FB_EXTG_ACTBIT_2)
	{
		uint16 replaceType = pExtAct->type2.ethertype;
		u16 *pEthType =(u16 *)(pData + pktBufOff - 2);

		*pEthType = htons(replaceType);
		DEBUG("[Dual] actionbit %d - replace etherType to 0x%04x", actBit, pExtAct->type2.ethertype);
	}

	if(actBit == FB_EXTG_ACTBIT_3)
	{
		u16 *tmplen = NULL;
		value = RTSKB_LEN(rtskb) - TUNN_tag_len - pktBufOff;	// reset to original l2 payload length.
		if(pExtAct->type3.operation==0)	// +
			value += pExtAct->type3.value;
		else{
			WARNING("Not support!! FIXME");
		}

		if(pExtAct->type3.length == 2){
			 tmplen = (u16 *)(pData+pktBufOff+pExtAct->type3.pkt_buff_offset);
			*tmplen = htons((u16)value);
		}else{
			WARNING("Not support!! FIXME");
		}
		DEBUG("[Dual] actionbit %d - skb len: %d, tunnTagLen: %d, pktoffset: %d", actBit, RTSKB_LEN(rtskb), TUNN_tag_len, pktBufOff);
		if(tmplen) {
			DEBUG("[Dual] actionbit %d - update offset %d to value 0x%x", actBit, pExtAct->type3.pkt_buff_offset, htons(*tmplen));
		}
	}

	if(actBit == FB_EXTG_ACTBIT_4)
	{
		u32 counter = 0;

		if(pExtAct->type4.data_src_type == 0)
		{
			u32 *target = NULL;
			u32 seqackOffset = 0;
			if(pExtAct->type4.reduce_seq != 1)
			{
				target = (u32 *)(pData+pktBufOff+pExtAct->type4.pkt_buff_offset);
				rtk_fc_dualHdrInfo_get(RTK_FC_FB_DUALHDR_GRESEQ, pExtAct->type4.seq_ack_reg_idx, &counter);
				*target = htonl(counter);
				seqackOffset+=4;
				DEBUG("[Dual] actionbit %d - use gre SEQ# 0x%x", actBit, counter);
			}
			if(pExtAct->type4.reduce_ack != 1)
			{
				target = (u32 *)(pData+pktBufOff+pExtAct->type4.pkt_buff_offset+seqackOffset);
				rtk_fc_dualHdrInfo_get(RTK_FC_FB_DUALHDR_GREACK, pExtAct->type4.seq_ack_reg_idx, &counter);
				*target = htonl(counter);
				DEBUG("[Dual] actionbit %d - use gre ACK# 0x%x", actBit, counter);
			}
		}
		else{
			u16 *target = NULL;
			target = (u16 *)(pData+pktBufOff+pExtAct->type4.pkt_buff_offset);
			rtk_fc_dualHdrInfo_get(RTK_FC_FB_DUALHDR_OUTER_IPV4ID, pExtAct->type4.seq_ack_reg_idx, &counter);
			*target = htons(counter);
			DEBUG("[Dual] actionbit %d - use IPv4 ID# 0x%x", actBit, counter);
		}
	}

	if(actBit == FB_EXTG_ACTBIT_5)
	{
		struct iphdr *iph = (struct iphdr *)(pData+pktBufOff);
		//iph->check = ip_fast_csum((unsigned char *)iph, iph->ihl);	// done by nic checksum offload
		DEBUG("[Dual] actionbit %d - ip checksum 0x%x", actBit, ntohs(iph->check));
	}

	if(actBit == FB_EXTG_ACTBIT_6)
	{
		struct iphdr *iph = (struct iphdr *)(pData+pktBufOff);
		struct udphdr *udph = (struct udphdr *)(pData+pktBufOff+(iph->ihl<<2));
		if(iph->protocol == 0x11)
		{
			//udph->check = inet_chksum_pseudo((u8 *)udph, udph->len, iph->saddr, iph->daddr, iph->protocol);	// done by nic checksum offload
		}
		else
		{
			WARNING("l4 protocol is not supported");
		}

		DEBUG("[Dual] actionbit %d - l4 checksum 0x%x", actBit, ntohs(udph->check));
	}
	if(actBit == FB_EXTG_ACTBIT_7)
	{
		if(pExtAct->type7.outer_udp_offset!=0)
		{
			pPktHdr->outer_udph = (struct udphdr *)(pData+pktBufOff+pExtAct->type7.outer_udp_offset);	//record outer header pointer
			TRACE("[EXTRA TAG]update outer udp: %p",pPktHdr->outer_udph);
		}
		
		if(pExtAct->type7.outer_ppp_offset!=0)
		{
			pPktHdr->outer_ppph = (struct pppoe_hdr *)(pData+pktBufOff+pExtAct->type7.outer_ppp_offset);	//record outer header pointer
			TRACE("[EXTRA TAG]update outer ppp: %p", pPktHdr->outer_ppph);
		}
		if(pExtAct->type7.is_l2dual)
		{
			pPktHdr->outer_eth = (struct ethhdr *)pData;
			pPktHdr->eth = (struct ethhdr *)(pData+pExtAct->type7.outer_tag_len);
			pPktHdr->vxlan_info.outerTag_off = pExtAct->type7.outer_tag_len;
			DEBUG("pPktHdr->eth = %p is_l2dual = %d",pPktHdr->eth,pExtAct->type7.is_l2dual);
		}
		if(pExtAct->type7.outer_is_v6)
		{
			pPktHdr->outer_ip6h = (struct ipv6hdr *)(pData+pktBufOff+pExtAct->type7.outer_ip_offset);	//record outer header pointer
		}
		else
			pPktHdr->outer_iph = (struct iphdr *)(pData+pktBufOff+pExtAct->type7.outer_ip_offset);	//record outer header pointer

		DEBUG("[Dual] actionbit %d outer_udp_offset:%d, outer_ppp_offset: %d, is_l2dual: %d", actBit, pExtAct->type7.outer_udp_offset, pExtAct->type7.outer_ppp_offset, pExtAct->type7.is_l2dual);
	}
	return SUCCESS;

}

/*
	for dual upstream and downstream hardware config
	1.setting path6/extraTag 
	2.G3:dual_content/dual_control
	3.maintain fc_db.netifTbl[NETIF_HWTOSW(hwNetifIdx)].dualHdr_HWState if you need update info to hardward
*/
int rtk_fc_dual_hw_setting(rtk_fc_pktHdr_t *pPktHdr,struct sk_buff *skb, uint32 hwNetifIdx, int32 *extraTagIdx)
{
	int ret = SUCCESS;

	rtk_fc_tableNetif_t *p_netif = &fc_db.netifTbl[NETIF_HWTOSW(hwNetifIdx)];

	FC_PARAM_CHK((p_netif->intf.valid == FALSE), RTK_FC_RET_ERR_ENTRY_NOT_FOUND);
	
	//reference to rtk_fc_dualInfoInit
	if(fc_db.dualCallBackFunc[p_netif->dualType].dual_hw_setting)
	{
		ret = fc_db.dualCallBackFunc[p_netif->dualType].dual_hw_setting(pPktHdr,skb, hwNetifIdx);
		if(ret)
			DEBUG("dualCallBackFunc Error %d",ret);
	}

	_rtk_fc_netif_dummy_packet_set(hwNetifIdx); //initial for PPTP/L2TP dummy packet

	if(pPktHdr->direction == RTK_FC_FLOW_DIRECTION_UPSTREAM)
		*extraTagIdx = p_netif->outerHdrExtratagIdx;
	else
		*extraTagIdx=0;


	return ret;
}

// for mapt and xlat
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
void _rtk_fc_4to6_L4ChecksumUpdate(rtk_fc_pktHdr_t *pPktHdr, uint16 *ipAddr, rtk_fc_tableFlow_t *pFlowTable)
{
	uint32 i,tmp_chksum,chksumDiff=0;
	uint16 *new_pseudoHdr,*origPort, newPort=pFlowTable->pFlowEntry->path5.out_l4_port;

	//HC' = ~(~HC + ~m + m')
	if(pPktHdr->udph){
		tmp_chksum = ((~(pPktHdr->udph->check))&0xffff);
		origPort = &pPktHdr->udph->source;
	}else if(pPktHdr->tcph){
		tmp_chksum = ((~(pPktHdr->tcph->check))&0xffff);
		origPort = &pPktHdr->tcph->source;
	}else
		return;	//do nothing

	if(pFlowTable->maptInfo.l4checksum_valid){
		if(*origPort != newPort){
			*origPort = htons(newPort);
		}

		tmp_chksum += pFlowTable->maptInfo.l4checksum_diff;
	}else{
		new_pseudoHdr = (uint16 *)(&pPktHdr->ip6h->saddr.s6_addr16[0]);

		for(i=0;i<4;i++){
			if(*ipAddr != *new_pseudoHdr){
				chksumDiff += (((~(*ipAddr))&0xffff) + (*new_pseudoHdr));
			}
			ipAddr++;
			new_pseudoHdr++;
		}

		for(i=0;i<12;i++){
			if(*new_pseudoHdr != 0){
				chksumDiff += (0xffff + (*new_pseudoHdr));
			}
			new_pseudoHdr++;
		}

		//update new sport
		if(*origPort != newPort){
			chksumDiff += ((~(*origPort))&0xffff);
			*origPort = htons(newPort);
			chksumDiff += *origPort;
		}

		tmp_chksum += chksumDiff;

		while (chksumDiff >> 16){
			chksumDiff = (chksumDiff & 0xffffUL) + (chksumDiff >> 16);
		}
		pFlowTable->maptInfo.l4checksum_diff = chksumDiff;
		pFlowTable->maptInfo.l4checksum_valid = 1;
	}

	while (tmp_chksum >> 16){
		tmp_chksum = (tmp_chksum & 0xffffUL) + (tmp_chksum >> 16);
	}

	if(pPktHdr->udph)
		pPktHdr->udph->check = ((~tmp_chksum)&0xffff);
	else if(pPktHdr->tcph)
		pPktHdr->tcph->check = ((~tmp_chksum)&0xffff);
}

void _rtk_fc_6to4_L4ChecksumUpdate(rtk_fc_pktHdr_t *pPktHdr, uint16 *ipAddr, rtk_fc_tableFlow_t *pFlowTable)
{
	uint32 i,tmp_chksum,chksumDiff=0;
	uint16 *ori_pseudoHdr,*origPort,newPort=pFlowTable->pFlowEntry->path5.out_l4_port;

	//HC' = ~(~HC + ~m + m')
	if(pPktHdr->udph){
		tmp_chksum = ((~(pPktHdr->udph->check))&0xffff);
		origPort = &pPktHdr->udph->dest;
	}else if(pPktHdr->tcph){
		tmp_chksum = ((~(pPktHdr->tcph->check))&0xffff);
		origPort = &pPktHdr->tcph->dest;
	}else
		return;	//do nothing

	if(pFlowTable->maptInfo.l4checksum_valid){
		if(*origPort != newPort){
			*origPort = htons(newPort);
		}

		tmp_chksum += pFlowTable->maptInfo.l4checksum_diff;
	}else{
		ori_pseudoHdr = (uint16 *)(&pPktHdr->ip6h->saddr.s6_addr16[0]);

		for(i=0;i<4;i++){
			if(*ori_pseudoHdr != *ipAddr){
				chksumDiff += (((~(*ori_pseudoHdr))&0xffff) + (*ipAddr));
			}
			ori_pseudoHdr++;
			ipAddr++;
		}

		for(i=0;i<12;i++){
			if(*ori_pseudoHdr != 0){
				chksumDiff += ((~(*ori_pseudoHdr))&0xffff);
			}
			ori_pseudoHdr++;
		}

		//update new dport
		if(*origPort != newPort){
			chksumDiff += ((~(*origPort))&0xffff);
			*origPort = htons(newPort);
			chksumDiff += *origPort;
		}

		tmp_chksum += chksumDiff;

		while (chksumDiff >> 16){
			chksumDiff = (chksumDiff & 0xffffUL) + (chksumDiff >> 16);
		}
		pFlowTable->maptInfo.l4checksum_diff = chksumDiff;
		pFlowTable->maptInfo.l4checksum_valid = 1;
	}

	while (tmp_chksum >> 16){
		tmp_chksum = (tmp_chksum & 0xffffUL) + (tmp_chksum >> 16);
	}

	if(pPktHdr->udph)
		pPktHdr->udph->check = ((~tmp_chksum)&0xffff);
	else if(pPktHdr->tcph)
		pPktHdr->tcph->check = ((~tmp_chksum)&0xffff);
}
#endif
int rtk_fc_mapt_translate(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_tableFlow_t *pFlowTable)		//enlarge skb from IPv4 to IPv6 or shrink skb from IPv6 to IPv4
{
	uint32 tmpOffset=0;
	uint32 moveLen = 0;
	uint16 ipv4Addr[4];
	u8 *origDataStart = NULL;
	struct iphdr *iph;
	struct ipv6hdr *ip6h;

	if(!rtskb || !pPktHdr || !pFlowTable)
		return RT_ERR_RG_FAILED;

	if(pPktHdr->isFragment)
		return RT_ERR_RG_FAILED;

	//if(fc_db.debugLevel & FC_DEBUG_LEVEL_DEBUG)
		//dump_packet(RTSKB_DATA(rtskb),RTSKB_LEN(rtskb),"before translate packet");

	origDataStart = RTSKB_DATA(rtskb);
	if(pPktHdr->iph){
		iph=pPktHdr->iph;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		//keep ipv4 address
		ipv4Addr[0]=iph->saddr>>16;
		ipv4Addr[1]=iph->saddr&0xffff;
		ipv4Addr[2]=iph->daddr>>16;
		ipv4Addr[3]=iph->daddr&0xffff;
		iph->ttl--;
#endif
		tmpOffset = (unsigned char *)iph - origDataStart;
		moveLen = sizeof(struct ipv6hdr) - (iph->ihl<<2);		//push
		skb_push(RTSKB_SKB(rtskb), moveLen);
		memmove(RTSKB_DATA(rtskb),origDataStart,tmpOffset);
		ip6h=(struct ipv6hdr *)(RTSKB_DATA(rtskb)+tmpOffset);

		//copy original v4 header field to v6 header
		ip6h->version = 6;
		ip6h->priority = ((iph->tos>>4)&0xf);
		ip6h->flow_lbl[0] = (iph->tos&0xf)<<4;
		ip6h->flow_lbl[1] = 0;
		ip6h->flow_lbl[2] = 0;
		ip6h->payload_len = htons(ntohs(iph->tot_len) - (iph->ihl<<2));
		ip6h->nexthdr = iph->protocol;
		ip6h->hop_limit = iph->ttl;
		memcpy(&ip6h->saddr.s6_addr32[0],&fc_db.ipv6_mapt_mappingTbl[pFlowTable->maptInfo.sip_indirect_mapping_index].addr.s6_addr32[0],sizeof(struct in6_addr));
		memcpy(&ip6h->daddr.s6_addr32[0],&fc_db.ipv6_mapt_mappingTbl[pFlowTable->maptInfo.dip_indirect_mapping_index].addr.s6_addr32[0],sizeof(struct in6_addr));

		// update eth_type/ppp_protocol
		if(pPktHdr->ppph)
			*(unsigned short *)((unsigned char *)ip6h-2) = htons(0x0057);
		else
			*(unsigned short *)((unsigned char *)ip6h-2) = htons(ETH_P_IPV6);

		pPktHdr->iph=NULL;
		pPktHdr->ip6h=ip6h;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		//update l4checksum and l4port here
		_rtk_fc_4to6_L4ChecksumUpdate(pPktHdr, (uint16 *)(&ipv4Addr[0]), pFlowTable);
#endif
	}else if(pPktHdr->ip6h){
		ip6h=pPktHdr->ip6h;
		tmpOffset=htonl(pFlowTable->pFlowEntry->path5.in_src_ipv4_addr);
		ipv4Addr[0]=tmpOffset>>16;
		ipv4Addr[1]=tmpOffset&0xffff;
		tmpOffset=htonl(pFlowTable->pFlowEntry->path5.out_dst_ipv4_addr);
		ipv4Addr[2]=tmpOffset>>16;
		ipv4Addr[3]=tmpOffset&0xffff;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		//update l4checksum and l4port here
		_rtk_fc_6to4_L4ChecksumUpdate(pPktHdr, (uint16 *)(&ipv4Addr[0]), pFlowTable);
		ip6h->hop_limit--;
#endif
		tmpOffset = (unsigned char *)ip6h - origDataStart;
		moveLen = sizeof(struct ipv6hdr) - sizeof(struct iphdr);		//pull
		skb_pull(RTSKB_SKB(rtskb), moveLen);

		iph=(struct iphdr *)(RTSKB_DATA(rtskb)+tmpOffset);

		//copy original v6 header field to v4 header
		iph->version = 4;
		iph->ihl = 5;
		iph->tos = (((ip6h->priority<<4)&0xf0)|((ip6h->flow_lbl[0]>>4)&0xf));
		iph->tot_len = htons(ntohs(ip6h->payload_len) + 20);
		iph->id = htons(jiffies&0xffff);
		iph->frag_off = htons(IP_DF);
		iph->ttl = ip6h->hop_limit;
		iph->protocol = ip6h->nexthdr;
		iph->saddr = ((ipv4Addr[0]<<16) | ipv4Addr[1]);
		iph->daddr = ((ipv4Addr[2]<<16) | ipv4Addr[3]);
		iph->check = 0;
		iph->check = inet_chksum((u8 *)iph,20);

		memmove(RTSKB_DATA(rtskb),origDataStart,tmpOffset);

		// update eth_type/ppp_protocol
		if(pPktHdr->ppph)
			*(unsigned short *)((unsigned char *)iph-2) = htons(0x0021);
		else
			*(unsigned short *)((unsigned char *)iph-2) = htons(ETH_P_IP);

		pPktHdr->iph=iph;
		pPktHdr->ip6h=NULL;
	}else
		return RT_ERR_RG_FAILED;

	RTK_FC_HOOK_PS_SKB_SKB_RESET_MAC_HEADER(RTSKB_SKB(rtskb));

	//reset header pointer
 	if(pPktHdr->eth) pPktHdr->eth = (struct ethhdr *)RTSKB_DATA(rtskb);
	if(pPktHdr->svh) pPktHdr->svh = (struct vlan_hdr *)((uint8 *)pPktHdr->svh + tmpOffset);
	if(pPktHdr->cvh) pPktHdr->cvh = (struct vlan_hdr *)((uint8 *)pPktHdr->cvh + tmpOffset);
	if(pPktHdr->ppph) pPktHdr->ppph = (struct pppoe_hdr *)((uint8 *)pPktHdr->ppph + tmpOffset);
	//if(fc_db.debugLevel & FC_DEBUG_LEVEL_DEBUG)
		//dump_packet(RTSKB_DATA(rtskb),RTSKB_LEN(rtskb),"after translate packet");

	return (RT_ERR_RG_OK);
}

int rtk_fc_outerHdr_insert(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_tableFlow_t *pFlowTblEnt, bool *ifSkipNicL2Offload)
{
	int TUNN_tag_len=0;				//IP(20)+GRE(16)+PPP(4) for PPTP, IP(20)+UDP(8)+L2TP(8)+PPP(4) for L2TP
	int i = 0, tmpOffset = 0;
	uint8 extraTagIdx=pFlowTblEnt->pFlowEntry->path5.out_extra_tag_index;

	rtk_fc_dualHdrtype_t wan_type = fc_db.netifTbl[NETIF_HWTOSW(pFlowTblEnt->pFlowEntry->path5.out_intf_idx)].dualType;

	TUNN_tag_len = fc_db.extraTagList[extraTagIdx].contentLen;

	if(fc_db.netifTbl[NETIF_HWTOSW(pFlowTblEnt->pFlowEntry->path5.out_intf_idx)].dualHdr_buf_insert_pos == FC_DUALHDR_HW_CTRL_PKT_BEGINIG)
	{
		tmpOffset = 0; // insert dual buffer content at the begining of the packet
		*ifSkipNicL2Offload = TRUE; // skip NIC VLAN/PPPoE offload to prevet outer l2 header to be modified
	}
	else
	{
		if(pPktHdr->eth) tmpOffset+=ETH_HLEN;
		if(pPktHdr->svh) tmpOffset+=4;
		if(pPktHdr->cvh) tmpOffset+=4;
		if(pPktHdr->ppph) tmpOffset+=8;
	}

	if(_rtk_fc_skb_cow_head_and_pktHdr_update(RTSKB_SKB(rtskb), TUNN_tag_len, rtskb, pPktHdr)<0)
	{
		WARNING("skb head room is not enough..return without insert %s tag",wan_type==RTK_FC_DUALTYPE_PPTP?"PPTP":"L2TP");
	}
	else
	{
		//if(fc_db.debugLevel & FC_DEBUG_LEVEL_DEBUG)
		//	dump_packet(skb->data,skb->len,"before adding outer hdr");
		for(i=0; i<RTK_FC_TABLESIZE_EXTRATAG_ACTIONS; i++)
		{
			if(fc_db.extraTagList[extraTagIdx].actions[i].type1.act_bit== FB_EXTG_ACTBIT_NOACTION)
				break;
			else
			{
				_rtk_fc_applyExtraTagAction(extraTagIdx, &fc_db.extraTagList[extraTagIdx].actions[i], rtskb, tmpOffset, pPktHdr);
			}
		}


		if(wan_type==RTK_FC_DUALTYPE_PPTP)
		{
			TRACE("add PPTP tag");
			pPktHdr->dualHdrType = RTK_FC_DUALTYPE_PPTP; //G3 need this info to fill nic tx info
		}
		else if(wan_type==RTK_FC_DUALTYPE_L2TP)
		{
			TRACE("add L2TP tag");
			pPktHdr->dualHdrType = RTK_FC_DUALTYPE_L2TP; //G3 need this info to fill nic tx info
		}
		else if(wan_type==RTK_FC_DUALTYPE_IP4INIP6)
		{
			TRACE("add OuterIpv6 tag (DUALTYPE_IP4INIP6)");
			pPktHdr->dualHdrType = RTK_FC_DUALTYPE_IP4INIP6; //G3 need this info to fill nic tx info
			pPktHdr->outer_ip6h = (struct ipv6hdr *)pPktHdr->outer_iph;
			pPktHdr->outer_iph = NULL;
		}
		else if(wan_type==RTK_FC_DUALTYPE_6RD)
		{
			TRACE("add 6RD tag");
			pPktHdr->dualHdrType = RTK_FC_DUALTYPE_6RD; //G3 need this info to fill nic tx info
		}
		else if(wan_type==RTK_FC_DUALTYPE_GRE_ETH_BR)
		{
			TRACE("add GRE_ETH_BR tag");
			pPktHdr->dualHdrType = RTK_FC_DUALTYPE_GRE_ETH_BR; //G3 need this info to fill nic tx info
		}
		else if(wan_type==RTK_FC_DUALTYPE_VXLAN)
		{
			TRACE("add VXLAN tag");
			pPktHdr->dualHdrType = RTK_FC_DUALTYPE_VXLAN; //G3 need this info to fill nic tx info
		}


		if (wan_type == RTK_FC_DUALTYPE_IP4INIP6 && fc_db.controlFuc.dsliteV6TosFromV4)
		{
			struct ipv6hdr *ip6h = (struct ipv6hdr *)pPktHdr->outer_ip6h;
			uint8 tos1, tos2;

			tos1 = (pPktHdr->iph->tos >> 4) & 0x0F;
			tos2 = (pPktHdr->iph->tos) & 0x0F;

			ip6h->priority = tos1;
			ip6h->flow_lbl[0] &= 0xF;
			ip6h->flow_lbl[0] |= (tos2<<4);

			DSLITE("%pI4 => %pI4, tos=0x%x\n", &pPktHdr->iph->saddr, &pPktHdr->iph->daddr, pPktHdr->iph->tos);
		}
		
		if ((wan_type == RTK_FC_DUALTYPE_IP4INIP6) && (pFlowTblEnt->protoCtrl == FLOW_PROTO_CTRL_MAPE_ACC))
		{	//MAP-E upstream may have different outHdr for FMR
			uint32 outif_idx = pFlowTblEnt->pFlowEntry->path5.out_intf_idx;
			uint16 fmr_idx = pFlowTblEnt->mapeInfo.mape_out_dst6_idx;
			rtk_fc_mape_dst6_update(pPktHdr, outif_idx, fmr_idx);
		}		

		//if(fc_db.debugLevel & FC_DEBUG_LEVEL_DEBUG)
		//	dump_packet(skb->data,skb->len,"after adding outer hdr");
	}
	return (RT_ERR_RG_OK);
}


int rtk_fc_outerHdr_remove(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_tableFlow_t *pFlowTblEnt)
{
	uint32 tmpOffset=0;
	uint32 rmLen = 0;
	u8 *innerDataStart = NULL;
	char innerV4Hdr = FALSE;


	//if(fc_db.debugLevel & FC_DEBUG_LEVEL_DEBUG)
		//dump_packet(skb->data,skb->len,"before remove packet");

	if(fc_db.netifTbl[NETIF_HWTOSW(pFlowTblEnt->pFlowEntry->path5.in_intf_idx)].dualHdr_buf_insert_pos == FC_DUALHDR_HW_CTRL_PKT_BEGINIG)
	{
		tmpOffset = 0;
		if(pPktHdr->eth) 
			innerDataStart = (char *)pPktHdr->eth;
		else {
			WARNING("innerDataStart (dualHdr_buf_insert_pos is from the begining of packet) NULL!\n");
			return RT_ERR_RG_FAILED;
		}
	}
	else
	{
		if(pPktHdr->eth) tmpOffset+=ETH_HLEN;
		if(pPktHdr->svh) tmpOffset+=4;
		if(pPktHdr->cvh) tmpOffset+=4;
		if(pPktHdr->ppph) tmpOffset+=8;

		//since the length we remove must longer than 2 MACs address, we use memcpy to replace memmove for speed.
		if((pPktHdr->dualHdrType == RTK_FC_DUALTYPE_PPTP) || (pPktHdr->dualHdrType == RTK_FC_DUALTYPE_L2TP)){

			if(pPktHdr->iph){
				innerV4Hdr = TRUE;
				innerDataStart = (char *)pPktHdr->iph;
			}else{
				innerDataStart = (char *)pPktHdr->ip6h;
			}
		}
		else if(pPktHdr->dualHdrType == RTK_FC_DUALTYPE_IP4INIP6){
			innerV4Hdr = TRUE;
			innerDataStart = (char *)pPktHdr->iph;
			pPktHdr->outer_ip6h = NULL;
		}
		else if(pPktHdr->dualHdrType == RTK_FC_DUALTYPE_6RD){
			innerDataStart = (char *)pPktHdr->ip6h;
		}
		else {
			WARNING("innerDataStart NULL!\n");
			return RT_ERR_RG_FAILED;
		}
	}
	rmLen = innerDataStart - RTSKB_DATA(rtskb)- tmpOffset;

	if(fc_db.netifTbl[NETIF_HWTOSW(pFlowTblEnt->pFlowEntry->path5.out_intf_idx)].dualHdr_buf_insert_pos == FC_DUALHDR_HW_CTRL_PKT_BEGINIG)
		;// no need memcpy
	else
		memcpy(innerDataStart - tmpOffset, RTSKB_DATA(rtskb), tmpOffset);		// remove outer headers

	RTSKB_DATA(rtskb)+=rmLen;
	RTSKB_LEN(rtskb)-=rmLen;
	RTK_FC_HOOK_PS_SKB_SKB_RESET_MAC_HEADER(RTSKB_SKB(rtskb));


	if(fc_db.netifTbl[NETIF_HWTOSW(pFlowTblEnt->pFlowEntry->path5.out_intf_idx)].dualHdr_buf_insert_pos == FC_DUALHDR_HW_CTRL_PKT_BEGINIG)
	{}// no need update ethertype and header pointer
	else
	{
		// update eth_type/ppp_protocol
		if(innerV4Hdr){
			if(pPktHdr->ppph)
				*(unsigned short *)(innerDataStart-2) = htons(0x0021);
			else
				*(unsigned short *)(innerDataStart-2) = htons(ETH_P_IP);
		}else {
			if(pPktHdr->ppph)
				*(unsigned short *)(innerDataStart-2) = htons(0x0057);
			else
				*(unsigned short *)(innerDataStart-2) = htons(ETH_P_IPV6);
		}

		//reset header pointer
		if(pPktHdr->eth) pPktHdr->eth = (struct ethhdr *)RTSKB_DATA(rtskb);
		if(pPktHdr->svh) pPktHdr->svh = (struct vlan_hdr *)((uint8 *)pPktHdr->svh + rmLen);
		if(pPktHdr->cvh) pPktHdr->cvh = (struct vlan_hdr *)((uint8 *)pPktHdr->cvh + rmLen);
		if(pPktHdr->ppph) pPktHdr->ppph = (struct pppoe_hdr *)((uint8 *)pPktHdr->ppph + rmLen);
	}
	//if(fc_db.debugLevel & FC_DEBUG_LEVEL_DEBUG)
	//dump_packet(skb->data,skb->len,"after remove packet");

	return (RT_ERR_RG_OK);
}
#if 0 
int rtk_fc_l2Dual_outerHdr_remove(struct rt_skbuff *rtskb, uint8 outer_tag_len, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_tableFlow_t *pFlowTable)
{
	//uint8 *pData = RTSKB_DATA(rtskb);
	
	RTSKB_DATA(rtskb)+=outer_tag_len;
	RTSKB_LEN(rtskb)-=outer_tag_len;
	RTK_FC_HOOK_PS_SKB_SKB_RESET_MAC_HEADER(RTSKB_SKB(rtskb));
	//pData = RTSKB_DATA(rtskb);

	if(fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].type == RTK_FC_L2DUAL_TYPE_VXLAN)
	{
		_rtk_fc_swPktHdrModify(rtskb, pPktHdr, pFlowTable);

		pPktHdr->skbLen_egress = RTSKB_LEN(rtskb);			// egress length is updated already by _rtk_fc_swPktHdrModify
	}
	return (SUCCESS);

}
int rtk_fc_l2Dual_outerHdr_insert(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_tableFlow_t *pFlowTable)
{
	uint8 *pData = RTSKB_DATA(rtskb);

	if(atomic_read(&fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].refCount)==0)
	{
		WARNING("Invalid l2DualTbl index");
		return FAIL;
	}

	if(fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].action != RTK_FC_L2DUAL_ACT_ADD)
	{
		WARNING("Invalid l2DualTbl type %d", fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].action != RTK_FC_L2DUAL_ACT_ADD);
		return FAIL;
	}

	if(fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].type == RTK_FC_L2DUAL_TYPE_VXLAN)
	{
		if(_rtk_fc_skb_cow_head_and_pktHdr_update(RTSKB_SKB(rtskb), fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].vxlan_us.outer_tag_len, rtskb, pPktHdr)<0)
		{
			WARNING("skb head room is not enough");
			return FAIL;
		}

		RTK_FC_HOOK_PS_SKB_SKB_PUSH(RTSKB_SKB(rtskb), fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].vxlan_us.outer_tag_len, &pData);
		memcpy(pData, fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].vxlan_us.contentBuffer, fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].vxlan_us.outer_tag_len);

		if(fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].vxlan_us.outer_ppph_tag_off)
		{
			DEBUG("[VXLAN]Outer pppoe offset = %d",fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].vxlan_us.outer_ppph_tag_off);
			pPktHdr->ppph = (struct pppoe_hdr *)(pData+fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].vxlan_us.outer_ppph_tag_off);
			pPktHdr->ppph->length = htons(RTSKB_LEN(rtskb) - fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].vxlan_us.outer_ppph_tag_off - PPPOE_SES_HLEN +2); // payload length includes p2p protocol(2 bytes)
		}

		RTK_FC_HOOK_PS_SKB_SKB_RESET_MAC_HEADER(RTSKB_SKB(rtskb));
		pPktHdr->outer_eth = (struct ethhdr *)(pData);
		if(fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].vxlan_us.outer_is_ipv6)
		{
			pPktHdr->outer_ip6h = (struct ipv6hdr *)(pData+fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].vxlan_us.outer_iph_tag_off);
			//The ipv6 header is not included in payload_len
			pPktHdr->outer_ip6h->payload_len =  htons(RTSKB_LEN(rtskb) - fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].vxlan_us.outer_udph_tag_off);
		}
		else
		{
			pPktHdr->outer_iph = (struct iphdr *)(pData+fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].vxlan_us.outer_iph_tag_off);
			pPktHdr->outer_iph->tot_len =  htons(RTSKB_LEN(rtskb) - fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].vxlan_us.outer_iph_tag_off);

		}

		pPktHdr->outer_udph = (struct udphdr *)(pData+fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].vxlan_us.outer_udph_tag_off);
		pPktHdr->outer_udph->len =	htons(RTSKB_LEN(rtskb) - fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].vxlan_us.outer_udph_tag_off);
		
	
		if(pFlowTable->pFlowEntry->path5.out_svid_format_act == FALSE)
		{
			if(pPktHdr->svh)
			{
				rtk_fc_vxlan_inner_skbVlanTag_remove(rtskb, pPktHdr, TRUE, pFlowTable->l2Dual_table_info.l2Dual_table_index);
			}	
		}

		// For inner cvlan
		if(pFlowTable->pFlowEntry->path5.out_cvid_format_act && 
			fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].vxlan_us.inner_cvlan_tagif ==1 )
		{
			uint16 vlanTCI = 0;
			vlanTCI = fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].vxlan_us.inner_cvlan_cvid | ( fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].vxlan_us.inner_cvlan_pri<<VLAN_PRIO_SHIFT | (fc_db.l2DualTbl[pFlowTable->l2Dual_table_info.l2Dual_table_index].vxlan_us.inner_cvlan_cfi<<VLAN_CFI_SHIFT));
			if(pPktHdr->cvh)
			{
				pPktHdr->cvh->h_vlan_TCI = htons(vlanTCI);
			}
			else
			{
				rtk_fc_vxlan_inner_skbVlanTag_insert(rtskb, pPktHdr, vlanTCI, FALSE, pFlowTable->l2Dual_table_info.l2Dual_table_index);
			}
		}

		RTK_FC_HOOK_PS_SKB_SKB_RESET_MAC_HEADER(RTSKB_SKB(rtskb));
		
	}

	return (SUCCESS);
}
#endif
int rtk_fc_getTunnelEgressDevIndex(rtk_fc_pktHdr_t *pPktHdr)
{
	int i;
	int index=FAIL;

	if(pPktHdr->eth==NULL || (pPktHdr->iph==NULL && pPktHdr->ip6h==NULL))
		return index;

	for(i = 0; i < RTK_FC_TABLESIZE_INTF; i++)
	{
		if(fc_db.netifTbl[i].intf.valid==FALSE)
			continue;
		if(pPktHdr->dualHdrType==RTK_FC_DUALTYPE_PPTP &&
			fc_db.netifTbl[i].dualType==RTK_FC_DUALTYPE_PPTP &&
			(fc_db.netifTbl[i].dualHashKey == pPktHdr->dualHdrHashKey) &&
			(fc_db.netifTbl[i].dualUniInfo.psGreInfo.peer_call_id == pPktHdr->tunnelInfo.pptp.grecallid)
		)
		{
			index=i;
			DEBUG("the PPTP netif match!! %d",i);
			break;
		}
		else if(pPktHdr->dualHdrType==RTK_FC_DUALTYPE_L2TP && 
				fc_db.netifTbl[i].dualType==pPktHdr->dualHdrType && fc_db.netifTbl[i].dualHashKey==pPktHdr->dualHdrHashKey && 
				(fc_db.netifTbl[i].dualUniInfo.l2tpInfo.peer_tunnel_id == pPktHdr->tunnelInfo.l2tp.tunnelid) &&
				(fc_db.netifTbl[i].dualUniInfo.l2tpInfo.peer_session_id == pPktHdr->tunnelInfo.l2tp.sessionid)
		)
		{
			index=i;
			DEBUG("the L2TP netif match!! %d",i);
			break;
		}
	}

	return index;
}

int rtk_fc_getTunnelEgressBasedDevIndex(int swNetifIdx)
{
	int i,count,baseWanIdx=-1;
	struct net_device *p_PScacheDev[DEV_STACK_MAX]={NULL,NULL,NULL,NULL};
	count=rtk_fc_psIfidxStackKey_to_dev(fc_db.netifTbl[swNetifIdx].psIfidxStackKey,p_PScacheDev);

	for(i=DEVIFIDX_VALID_MIN;i<RTK_FC_DEV_GW_MAC_TBL;i++)
	{
		if(!fc_db.devGwMacTbl[i].dev)
			continue;
		if(fc_db.devGwMacTbl[i].dev == p_PScacheDev[count-1])
		{
			baseWanIdx=i;
			break;
		}
	}

	return baseWanIdx;
}

int rtk_fc_getTunnelEgressBasedDevIndex_byPktHdr(rtk_fc_pktHdr_t *pPktHdr)
{
	int i,j;

	for(i=DEVIFIDX_VALID_MIN;i<RTK_FC_DEV_GW_MAC_TBL;i++){
		if(!fc_db.devGwMacTbl[i].dev)
			continue;
		if(!memcmp(fc_db.devGwMacTbl[i].dev->dev_addr,pPktHdr->eth->h_source,ETH_ALEN)){
			for(j=0;j<MAX_SW_FRAGIPID_SIZE;j++){
				if(!fc_db.devGwMacTbl[i].psFragIPID[j].valid)
					continue;
				if(fc_db.devGwMacTbl[i].psFragIPID[j].oriIPID==ntohs(pPktHdr->iph->id)){
					DEBUG("IPID match!! baseIntfIdx=%d",i);
					return i;
				}
			}
		}
	}

	return FAIL;
}

/*
* Sync PPTP/L2TP IPID/GRESEQ/ACK
*/
int rtk_fc_egress_tunnelInfoSync(struct rt_skbuff *rtskb, rtk_fc_pktHdr_t *pPktHdr)
{

	uint32 tmpval = 0;
	uint8 frag = FALSE, frag_first = FALSE, frag_last = FALSE;
	int baseIntfIdx = FAIL, egressIntfIdx = FAIL;
	uint16 fragIPID = 0;

	if(pPktHdr->outer_iph && (RTSKB_FCIGRDATA(rtskb)->isDualHeader == FALSE) && (pPktHdr->dualHdrType == RTK_FC_DUALTYPE_GRE_ETH_BR))
	{
		int i;
		if(RTSKB_FCIGRDATA(rtskb)->doLearning)
		{
			egressIntfIdx = rtk_fc_getEgressSwNetifIdx(pPktHdr,rtskb->skb,RTK_FC_FWDTYPE_ROUTING,RTK_FC_FLOW_DIRECTION_UPSTREAM);
		}
		else
		{
			if(pPktHdr->dualHdrHashKey == 0)
			{
				TRACE("pPktHdr->dualHdrType is GRE_ETH_BR, but pPktHdr->dualHashKey is 0, skip tunnelInfoSync");
				return (FAILED);
			}
			else
			{
				for(i = 0 ; i < RTK_FC_TABLESIZE_INTF_SW ; i++)
				{
					if(!fc_db.netifTbl[i].intf.valid )
						continue;

					if((fc_db.netifTbl[i].dualType == pPktHdr->dualHdrType) && (fc_db.netifTbl[i].dualHashKey == pPktHdr->dualHdrHashKey))
					{
						egressIntfIdx = i;
						break;
					}
				}
				TRACE("Search dualHashKey:0x%x netif %d", pPktHdr->dualHdrHashKey, egressIntfIdx);
			}
		}

		if((egressIntfIdx==FAIL) || (fc_db.netifTbl[egressIntfIdx].dualType!=RTK_FC_DUALTYPE_GRE_ETH_BR))
		{
			DEBUG("incorrect/unnecessary egress interface %d\n", egressIntfIdx);
			return (FAILED);
		}
		else
		{

			fc_db.netifTbl[egressIntfIdx].ipid = ntohs(pPktHdr->outer_iph->id);
			ASSERT_EQ(rtk_fc_dualHdrInfo_get(RTK_FC_FB_DUALHDR_OUTER_IPV4ID, fc_db.netifTbl[egressIntfIdx].hwIdx, &tmpval), SUCCESS);
			pPktHdr->outer_iph->id = htons(tmpval);

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
			RTK_FC_HOOK_PS_SKB_IP_SUMMED_SET(RTSKB_SKB(rtskb), CHECKSUM_PARTIAL); // for 8277 series, HW need to do checksum offload
#endif
		}
	}


	// common field: IPID, the packet may be dual hdr or single hdr.
	if(pPktHdr->outer_iph && ((pPktHdr->dualHdrType == RTK_FC_DUALTYPE_PPTP) || (pPktHdr->dualHdrType == RTK_FC_DUALTYPE_L2TP)) &&
		ip_is_fragment(pPktHdr->outer_iph) )
	{
		//2 Dual hdr: 1st frag pkt
		frag = TRUE;
		frag_first = FALSE; frag_last = FALSE;

		// find the inner interface
		egressIntfIdx=rtk_fc_getTunnelEgressDevIndex(pPktHdr);

		if((egressIntfIdx==FAIL) ||
			((fc_db.netifTbl[egressIntfIdx].dualType!=RTK_FC_DUALTYPE_PPTP)&&(fc_db.netifTbl[egressIntfIdx].dualType!=RTK_FC_DUALTYPE_L2TP))){
			DEBUG("incorrect/unnecessary egress interface %d", egressIntfIdx);
			return (FAILED);
		}

		// find the outer interface
		baseIntfIdx=rtk_fc_getTunnelEgressBasedDevIndex(egressIntfIdx);

		if(baseIntfIdx==FAIL){
			WARNING("incorrect egress interface %d", baseIntfIdx);
			return (FAILED);
		}
		if(!(pPktHdr->outer_iph->frag_off & htons(IP_OFFSET)) && (pPktHdr->outer_iph->frag_off & htons(IP_MF))){
			frag_first = TRUE;
		}else {
			// error case
			WARNING("dual hdr fmt but without ipmf flag");
			return (FAILED);
		}
		DEBUG("[FRAG] %s packet, ori ipid: %d, skb len: %d, egress to intf[%d] base on devGwMacTbl[%d]",
			(pPktHdr->dualHdrType== RTK_FC_DUALTYPE_PPTP)?"PPTP":"L2TP", ntohs(pPktHdr->outer_iph->id), RTSKB_LEN(rtskb), egressIntfIdx, baseIntfIdx);

		// Get new identifier
		fc_db.netifTbl[egressIntfIdx].ipid = ntohs(pPktHdr->outer_iph->id);
		ASSERT_EQ(rtk_fc_dualHdrInfo_get(RTK_FC_FB_DUALHDR_OUTER_IPV4ID, fc_db.netifTbl[egressIntfIdx].hwIdx, &tmpval), SUCCESS);
		pPktHdr->outer_iph->id = htons(tmpval);

		// maintain fragment ipid
		if(frag_first){
			// record the ipid because of first fragment packet
			_rtk_fc_fragIPID_save(baseIntfIdx,fc_db.netifTbl[egressIntfIdx].ipid, ntohs(pPktHdr->outer_iph->id));
		}

		DEBUG("[FRAG] %s frag packet, use ipid: %d, skb len: %d",
			(pPktHdr->dualHdrType== RTK_FC_DUALTYPE_PPTP)?"PPTP":"L2TP", ntohs(pPktHdr->outer_iph->id), RTSKB_LEN(rtskb));

	}
	else if((pPktHdr->outer_iph==NULL) && (pPktHdr->iph!=NULL) && ip_is_fragment(pPktHdr->iph)){
		//2 Dual hdr: 2nd~N frag pkt
		frag = TRUE;
		baseIntfIdx=rtk_fc_getTunnelEgressBasedDevIndex_byPktHdr(pPktHdr);
		if(baseIntfIdx==FAIL){
			DEBUG("incorrect egress interface %d", baseIntfIdx);
			return (FAILED);
		}

		if(!(pPktHdr->iph->frag_off & htons(IP_OFFSET)) && (pPktHdr->iph->frag_off & htons(IP_MF))){
			frag_first = TRUE;
		}else if((pPktHdr->iph->frag_off & htons(IP_OFFSET)) && !(pPktHdr->iph->frag_off & htons(IP_MF))){
			frag_last = TRUE;
 		}

		if(frag_first){
			// from wan to lan, it may fragment packet and without outer header, ignore such case.
		}
		if(!frag_last){
			// get the ipid record
			if(_rtk_fc_fragIPID_get(baseIntfIdx, ntohs(pPktHdr->iph->id), &fragIPID) == SUCCESS)
				pPktHdr->iph->id = htons(fragIPID);
		}else{
			// remove the ipid record because of last fragment packet
			if(_rtk_fc_fragIPID_delete(baseIntfIdx, ntohs(pPktHdr->iph->id), &fragIPID) == SUCCESS)
				pPktHdr->iph->id = htons(fragIPID);
		}

		DEBUG("[FRAG] single hdr packet, last_frag: %d, use ipid: %d, skb len: %d", frag_last, ntohs(pPktHdr->iph->id), RTSKB_LEN(rtskb));

	}
	else if(pPktHdr->outer_iph && ((pPktHdr->dualHdrType == RTK_FC_DUALTYPE_PPTP) || (pPktHdr->dualHdrType == RTK_FC_DUALTYPE_L2TP))){
		//2 Dual hdr: not frag pkt
		// no fragment, consider normal dual hdr ipid sync.
		egressIntfIdx=rtk_fc_getTunnelEgressDevIndex(pPktHdr);

		if((egressIntfIdx==FAIL) ||
			((fc_db.netifTbl[egressIntfIdx].dualType!=RTK_FC_DUALTYPE_PPTP)&&(fc_db.netifTbl[egressIntfIdx].dualType!=RTK_FC_DUALTYPE_L2TP))){
			DEBUG("incorrect/unnecessary egress interface %d !!!", egressIntfIdx);
			return (FAILED);
		}

		ASSERT_EQ(rtk_fc_dualHdrInfo_get(RTK_FC_FB_DUALHDR_OUTER_IPV4ID, fc_db.netifTbl[egressIntfIdx].hwIdx, &tmpval), SUCCESS);
		pPktHdr->outer_iph->id = htons(tmpval);

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		RTK_FC_HOOK_PS_SKB_IP_SUMMED_SET(RTSKB_SKB(rtskb), CHECKSUM_PARTIAL); // for 8277 series, HW need to do checksum offload
#endif

		DEBUG("[FRAG] %s not frag packet, use ipid: %d, skb len: %d", (pPktHdr->dualHdrType== RTK_FC_DUALTYPE_PPTP)?"PPTP":"L2TP", ntohs(pPktHdr->outer_iph->id), RTSKB_LEN(rtskb));
	}

	if((pPktHdr->dualHdrType == RTK_FC_DUALTYPE_PPTP) || (pPktHdr->tunnelInfo.tunnelTag & (GRESEQ_TAGIF|GREACK_TAGIF))){

		if(egressIntfIdx == FAIL)
			egressIntfIdx=rtk_fc_getTunnelEgressDevIndex(pPktHdr);

		if(egressIntfIdx == FAIL)
			return (FAILED);	

		// gre seq/ack
		//if(pPktHdr->tunnelInfo.pptp.greseqTagif){
		if((pPktHdr->tunnelInfo.tunnelTag & GRESEQ_TAGIF) && (egressIntfIdx >= 0)){
			fc_db.netifTbl[egressIntfIdx].dualUniInfo.psGreInfo.tx_greseq = pPktHdr->tunnelInfo.pptp.greseq;

			rtk_fc_dualHdrInfo_get(RTK_FC_FB_DUALHDR_GRESEQ, fc_db.netifTbl[egressIntfIdx].hwIdx, &tmpval);
			DEBUG("[Dual] PPTP GRE seq: hw(%d) ps(%d)", tmpval, pPktHdr->tunnelInfo.pptp.greseq);
			if(pPktHdr->tunnelInfo.pptp.greseq > tmpval){
				rtk_fc_dualHdrInfo_set(RTK_FC_FB_DUALHDR_GRESEQ, fc_db.netifTbl[egressIntfIdx].hwIdx, pPktHdr->tunnelInfo.pptp.greseq+1);
			}
			else{
				*pPktHdr->tunnelInfo.pptp.pGRESeq = htonl(tmpval);
			}
		}
		//if(pPktHdr->tunnelInfo.pptp.greackTagif){
		if((pPktHdr->tunnelInfo.tunnelTag & GREACK_TAGIF) && (egressIntfIdx >= 0)){
			fc_db.netifTbl[egressIntfIdx].dualUniInfo.psGreInfo.tx_greack = pPktHdr->tunnelInfo.pptp.greack;
			rtk_fc_dualHdrInfo_get(RTK_FC_FB_DUALHDR_GREACK, fc_db.netifTbl[egressIntfIdx].hwIdx, &tmpval);
			*pPktHdr->tunnelInfo.pptp.pGREAck = htonl(tmpval);
			DEBUG("[Dual] PPTP GRE ack: hw(%d) ps(%d)", tmpval, pPktHdr->tunnelInfo.pptp.greack);
		}

	}
	else if(pPktHdr->dualHdrType == RTK_FC_DUALTYPE_L2TP){
		// no requirement
	}

	return (SUCCESS);
}


/*
* To delete dual-config related configuration
* 1. Extra tag actions and content buffer
* 2. Path6 flow entry
*/
int rtk_fc_netif_dualConfig_del(int swIntfIdx)
{
	rtk_fc_tableNetif_t *pNetif = &fc_db.netifTbl[swIntfIdx];

	FC_PARAM_CHK(pNetif==NULL, RTK_FC_RET_ERR_ENTRY_NOT_FOUND);
	FC_PARAM_CHK(pNetif->intf.valid==FALSE, RTK_FC_RET_ERR_ENTRY_NOT_FOUND);

	TRACE("clear intf config: dual type[%d] path6_idx[%d] extratag_idx[%d] ", pNetif->dualType, pNetif->outerHdrFlowIdx, pNetif->outerHdrExtratagIdx);

	// Del flow entra tag action
	if(pNetif->outerHdrExtratagIdx!= FAIL)
	{
		ASSERT_EQ(RTK_RG_ASIC_EXTRATAGACTION_DEL(pNetif->outerHdrExtratagIdx, 0), RT_ERR_RG_OK);
		//TODO: clear extra tag content buffer
	}

	// Del flow path6 entry (MAP-T/GRE_ETH_BR do not have path6 flow)
	if(pNetif->outerHdrFlowIdx!=-SIGNED_INVALID)
	{
		rtk_fc_flow_delete(pNetif->outerHdrFlowIdx);		// delete and ignore return fail (e.g. flow not found) because flow entry may be deleded already.
		pNetif->outerHdrFlowIdx = SIGNED_INVALID;
	}
	
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	//clear DualHeader control and content register
	while(aal_pe_dual_control_entry_del(RTK_FC_DUAL_CONTROL_IDX(pNetif->hwIdx)) == AAL_E_ENTRY_USED);
	do{
		int i;
		for(i=pNetif->hwEntryNum;i>0;i--)
		{
			while(aal_pe_dual_content_entry_del(RTK_FC_DUAL_CONTROL_IDX(pNetif->hwIdx)+pNetif->hwEntryNum-i) == AAL_E_ENTRY_USED);	
		}
	}while(0);

	//Del downstream dual header CLS
	if(pNetif->dualHdr_ds_clsIdx != RTK_FC_UINT32_INVALID)
	{
		assert_ok(_rtk_rg_aclAndCfReservedRuleDelSpecial(RTK_CLS_TYPE_DUAL_HEADER_DS_CHECK, &pNetif->dualHdr_ds_clsIdx));
		pNetif->dualHdr_ds_clsIdx = RTK_FC_UINT32_INVALID;
	}
	if(pNetif->dualHdr_ds_extra_clsIdx != RTK_FC_UINT32_INVALID)
	{
		assert_ok(_rtk_rg_aclAndCfReservedRuleDelSpecial(RTK_CLS_TYPE_DUAL_HEADER_DS_CHECK, &pNetif->dualHdr_ds_extra_clsIdx));
		pNetif->dualHdr_ds_extra_clsIdx = RTK_FC_UINT32_INVALID;
	}

	//delete fmrtable
	if(pNetif->hwFmrIdx!=SIGNED_INVALID)
	{
		rtk_fc_mapet_hwfmrInfo_t fmrhwInfo;
		bzero(&fmrhwInfo,sizeof(fmrhwInfo));
		fmrhwInfo.hwFmrIdx=pNetif->hwFmrIdx;
		rtk_fc_mapet_hwfmr_del(&fmrhwInfo);
		pNetif->hwFmrIdx=SIGNED_INVALID;
	}

	
#endif

	return (RTK_FC_RET_OK);
}


void rtk_fc_dualHash_ip4Inip6(rtk_fc_dualHashKey_t* patten,rtk_fc_pktHdr_t *pPktHdr,rtk_fc_flow_dual_direction_t dual_direct)
{
	if(pPktHdr->outer_ip6h)
	{
		if(dual_direct)
		{
			memcpy(&patten->local_ip_addr,pPktHdr->outer_ip6h->daddr.s6_addr,sizeof(struct in6_addr));
			memcpy(&patten->remote_ip_addr,pPktHdr->outer_ip6h->saddr.s6_addr,sizeof(struct in6_addr));
		}
		else
		{
			//to wan Egr
			memcpy(&patten->local_ip_addr,pPktHdr->outer_ip6h->saddr.s6_addr,sizeof(struct in6_addr));
			memcpy(&patten->remote_ip_addr,pPktHdr->outer_ip6h->daddr.s6_addr,sizeof(struct in6_addr));
		}
	}

	return;
}
void rtk_fc_dualHash_PPTP(rtk_fc_dualHashKey_t* pattern,rtk_fc_pktHdr_t *pPktHdr,rtk_fc_flow_dual_direction_t dual_direct)
{
	if(dual_direct)
	{
		//downstream
		if(pPktHdr->outer_ip6h){
			memcpy(&pattern->local_ip_addr,pPktHdr->outer_ip6h->daddr.s6_addr,sizeof(struct in6_addr));
			memcpy(&pattern->remote_ip_addr,pPktHdr->outer_ip6h->saddr.s6_addr,sizeof(struct in6_addr));
			TRACE("downstream outer v6 daddr = %pI6 saddr = %pI6",&pPktHdr->outer_ip6h->daddr,&pPktHdr->outer_ip6h->saddr);
		}
		else if(pPktHdr->outer_iph)
		{
			pattern->local_ip_addr[0]=ntohl(pPktHdr->outer_iph->daddr);
			pattern->remote_ip_addr[0]=ntohl(pPktHdr->outer_iph->saddr);
			TRACE("downstream outer v4 daddr = %pI4 saddr = %pI4",&pPktHdr->outer_iph->daddr,&pPktHdr->outer_iph->saddr);
		}
		pattern->otherParameter[0]=pPktHdr->dualHdrType;	//should be RTK_FC_DUALTYPE_PPTP
	}
	else
	{
		//upstream
		if(pPktHdr->outer_ip6h){
			memcpy(&pattern->local_ip_addr,pPktHdr->outer_ip6h->saddr.s6_addr,sizeof(struct in6_addr));
			memcpy(&pattern->remote_ip_addr,pPktHdr->outer_ip6h->daddr.s6_addr,sizeof(struct in6_addr));
			TRACE("upstream outer v6 saddr = %pI6 daddr = %pI6",&pPktHdr->outer_ip6h->saddr,&pPktHdr->outer_ip6h->daddr);
		}
		else if(pPktHdr->outer_iph)
		{
			pattern->local_ip_addr[0]=ntohl(pPktHdr->outer_iph->saddr);
			pattern->remote_ip_addr[0]=ntohl(pPktHdr->outer_iph->daddr);
			TRACE("upstream outer v4 saddr = %pI4 daddr = %pI4",&pPktHdr->outer_iph->saddr,&pPktHdr->outer_iph->daddr);
		}
		pattern->otherParameter[0]=pPktHdr->dualHdrType;	//should be RTK_FC_DUALTYPE_PPTP
	}

	return;
}
void rtk_fc_dualHash_L2TP(rtk_fc_dualHashKey_t* pattern, rtk_fc_pktHdr_t *pPktHdr, rtk_fc_flow_dual_direction_t dual_direct)
{
	if(dual_direct == RTK_FC_FLOW_DUAL_DIRECTION_DOWNSTREAM_IGR)
	{
		//downstream
		if(pPktHdr->outer_ip6h){
			memcpy(&pattern->local_ip_addr, pPktHdr->outer_ip6h->daddr.s6_addr, sizeof(struct in6_addr));
			memcpy(&pattern->remote_ip_addr, pPktHdr->outer_ip6h->saddr.s6_addr, sizeof(struct in6_addr));
			TRACE("[L2TP hash][DS] outer v6 daddr = %pI6 saddr = %pI6", &pPktHdr->outer_ip6h->daddr, &pPktHdr->outer_ip6h->saddr);
		}
		else if(pPktHdr->outer_iph)
		{
			pattern->local_ip_addr[0] = ntohl(pPktHdr->outer_iph->daddr);
			pattern->remote_ip_addr[0] = ntohl(pPktHdr->outer_iph->saddr);
			TRACE("[L2TP hash][DS] outer v4 daddr = %pI4 saddr = %pI4", &pPktHdr->outer_iph->daddr, &pPktHdr->outer_iph->saddr);
		}
		pattern->otherParameter[0]=RTK_FC_DUALTYPE_L2TP;
	}
	else if(dual_direct == RTK_FC_FLOW_DUAL_DIRECTION_UPSTREAM_EGR)
	{
		//upstream
		if(pPktHdr->outer_ip6h){
			memcpy(&pattern->local_ip_addr, pPktHdr->outer_ip6h->saddr.s6_addr, sizeof(struct in6_addr));
			memcpy(&pattern->remote_ip_addr, pPktHdr->outer_ip6h->daddr.s6_addr, sizeof(struct in6_addr));
			TRACE("[L2TP hash][UP] outer v6 saddr = %pI6 daddr = %pI6", &pPktHdr->outer_ip6h->saddr, &pPktHdr->outer_ip6h->daddr);
		}
		else if(pPktHdr->outer_iph)
		{
			pattern->local_ip_addr[0]=ntohl(pPktHdr->outer_iph->saddr);
			pattern->remote_ip_addr[0]=ntohl(pPktHdr->outer_iph->daddr);
			TRACE("[L2TP hash][UP] outer v4 saddr = %pI4 daddr = %pI4", &pPktHdr->outer_iph->saddr, &pPktHdr->outer_iph->daddr);
		}
		pattern->otherParameter[0]=RTK_FC_DUALTYPE_L2TP;
	}
	else
		WARNING("[L2TP] dual_direct %d is unknown !!", dual_direct);
	
	return;
}

void rtk_fc_dualHash_IPSEC(rtk_fc_dualHashKey_t* patten,rtk_fc_pktHdr_t *pPktHdr,rtk_fc_flow_dual_direction_t dual_direct)
{
	WARNING("please add dual-hdr IPSEC hash function ");
	return;
}
void rtk_fc_dualHash_GRE_ETH_BR(rtk_fc_dualHashKey_t* patten,rtk_fc_pktHdr_t *pPktHdr,rtk_fc_flow_dual_direction_t dual_direct)
{
	if(dual_direct == RTK_FC_FLOW_DUAL_DIRECTION_DOWNSTREAM_IGR)
	{
		// RTK_FC_FLOW_DUAL_DIRECTION_DOWNSTREAM_IGR
		if(pPktHdr->outer_iph)
		{
			patten->local_ip_addr[0] = pPktHdr->outer_iph->daddr;
			patten->remote_ip_addr[0] = pPktHdr->outer_iph->saddr;
			DEBUG("dual hash pattern for GRE_ETH_BR: local_ip_addr (%pI4n), remote_ip_addr (%pI4n)", &(patten->local_ip_addr[0]), &(patten->remote_ip_addr[0]));
		}
		else if(pPktHdr->outer_ip6h)
		{
			memcpy(&patten->local_ip_addr, pPktHdr->outer_ip6h->daddr.s6_addr, sizeof(struct in6_addr));
			memcpy(&patten->remote_ip_addr, pPktHdr->outer_ip6h->saddr.s6_addr, sizeof(struct in6_addr));
			DEBUG("dual hash pattern for GRE_ETH_BR: local_ip_addr (%pI6c), remote_ip_addr (%pI6c)", patten->local_ip_addr, patten->remote_ip_addr);
		}
		else
			WARNING("pPktHdr->outer_iph and pPktHdr->outer_ip6h are both NULL!");
	}
	else
	{
		// RTK_FC_FLOW_DUAL_DIRECTION_UPSTREAM_EGR
		if(pPktHdr->outer_iph)
		{
			patten->local_ip_addr[0] = pPktHdr->outer_iph->saddr;
			patten->remote_ip_addr[0] = pPktHdr->outer_iph->daddr;
			DEBUG("dual hash pattern for GRE_ETH_BR: local_ip_addr (%pI4n), remote_ip_addr (%pI4n)", &(patten->local_ip_addr[0]), &(patten->remote_ip_addr[0]));
		}
		else if(pPktHdr->outer_ip6h)
		{
			memcpy(&patten->local_ip_addr, pPktHdr->outer_ip6h->saddr.s6_addr, sizeof(struct in6_addr));
			memcpy(&patten->remote_ip_addr, pPktHdr->outer_ip6h->daddr.s6_addr, sizeof(struct in6_addr));
			DEBUG("dual hash pattern for GRE_ETH_BR: local_ip_addr (%pI6c), remote_ip_addr (%pI6c)", patten->local_ip_addr, patten->remote_ip_addr);
		}
		else
			WARNING("pPktHdr->outer_iph and pPktHdr->outer_ip6h are both NULL!");
	}

	if(pPktHdr->outer_svh)
		patten->svlanId = pPktHdr->outer_svlanid;
	else
		patten->svlanId = SIGNED_INVALID;

	if(pPktHdr->outer_cvh)
		patten->cvlanId = pPktHdr->outer_cvlanid;
	else
		patten->cvlanId = SIGNED_INVALID;

	DEBUG("dual hash pattern for GRE_ETH_BR: svlanId(0x%x), cvlanId(0x%x)", patten->svlanId, patten->cvlanId);

	return;
}
void rtk_fc_dualHash_VXLAN(rtk_fc_dualHashKey_t* patten,rtk_fc_pktHdr_t *pPktHdr,rtk_fc_flow_dual_direction_t dual_direct)
{
	patten->otherParameter[0] = pPktHdr->vxlan_info.vni;
	if(pPktHdr->cvh)
		patten->otherParameter[1] = pPktHdr->cvlanid;
	if(pPktHdr->svh)
		patten->otherParameter[2] = pPktHdr->svlanid;
	DEBUG("add dual-hdr VXLAN VNI: %d to pattern",pPktHdr->vxlan_info.vni);
	return;
}
void rtk_fc_dualHash_6RD(rtk_fc_dualHashKey_t* patten,rtk_fc_pktHdr_t *pPktHdr,rtk_fc_flow_dual_direction_t dual_direct)
{
	if(pPktHdr->outer_iph)
	{
		if(dual_direct==RTK_FC_FLOW_DUAL_DIRECTION_DOWNSTREAM_IGR)
		{
			patten->local_ip_addr[0] = pPktHdr->outer_iph->daddr;
			patten->remote_ip_addr[0] = pPktHdr->outer_iph->saddr;
		}
		else
		{
			//to wan Egr
			patten->local_ip_addr[0] = pPktHdr->outer_iph->saddr;
			patten->remote_ip_addr[0] = pPktHdr->outer_iph->daddr;
		}
	}

	return;
}

void rtk_fc_dualHash_MAPT(rtk_fc_dualHashKey_t* pattern,rtk_fc_pktHdr_t *pPktHdr,rtk_fc_flow_dual_direction_t dual_direct)
{
	if(dual_direct)
	{
		//downstream
		if(pPktHdr->ip6h)
		{
			memcpy(&pattern->local_ip_addr,pPktHdr->ip6h->daddr.s6_addr,sizeof(struct in6_addr));
			DEBUG("downstream v6 daddr = %pI6 saddr = %pI6",&pPktHdr->ip6h->daddr,&pPktHdr->ip6h->saddr);
		}		
	}
	else
	{
		//upstream
		if(pPktHdr->ip6h)
		{
			memcpy(&pattern->local_ip_addr,pPktHdr->ip6h->saddr.s6_addr,sizeof(struct in6_addr));
			DEBUG("upstream v6 saddr = %pI6 daddr = %pI6",&pPktHdr->ip6h->saddr,&pPktHdr->ip6h->daddr);
		}
	}
	pattern->otherParameter[0]=128;	//compare IP address 128bits
	return;
}

static void rtk_fc_dualHash_XLAT_ipMasked(uint8 *ip6addr,rtk_fc_dual_ipv6HashMask_t prefixBits)
{

	switch(prefixBits)
	{
		case DUAL_IPV6_HASHMASK_PREFIX_32:
			ip6addr[4] = 0;
			ip6addr[5] = 0;
			ip6addr[6] = 0;
			ip6addr[7] = 0;
			ip6addr[8] = 0;
			ip6addr[9] = 0;
			ip6addr[10] = 0;
			ip6addr[11] = 0;
			ip6addr[12] = 0;
			ip6addr[13] = 0;
			ip6addr[14] = 0;
			ip6addr[15] = 0;
			break;
		case DUAL_IPV6_HASHMASK_PREFIX_40:
			ip6addr[5] = 0;
			ip6addr[6] = 0;
			ip6addr[7] = 0;
			ip6addr[8] = 0;
			ip6addr[9] = 0;
			ip6addr[10] = 0;
			ip6addr[11] = 0;
			ip6addr[12] = 0;
			ip6addr[13] = 0;
			ip6addr[14] = 0;
			ip6addr[15] = 0;
			break;
		case DUAL_IPV6_HASHMASK_PREFIX_48:
			ip6addr[6] = 0;
			ip6addr[7] = 0;
			ip6addr[8] = 0;
			ip6addr[9] = 0;
			ip6addr[10] = 0;
			ip6addr[11] = 0;
			ip6addr[12] = 0;
			ip6addr[13] = 0;
			ip6addr[14] = 0;
			ip6addr[15] = 0;
			break;
		case DUAL_IPV6_HASHMASK_PREFIX_56:
			ip6addr[7] = 0;
			ip6addr[8] = 0;
			ip6addr[9] = 0;
			ip6addr[10] = 0;
			ip6addr[11] = 0;
			ip6addr[12] = 0;
			ip6addr[13] = 0;
			ip6addr[14] = 0;
			ip6addr[15] = 0;
			break;
		case DUAL_IPV6_HASHMASK_PREFIX_64:
			ip6addr[8] = 0;
			ip6addr[9] = 0;
			ip6addr[10] = 0;
			ip6addr[11] = 0;
			ip6addr[12] = 0;
			ip6addr[13] = 0;
			ip6addr[14] = 0;
			ip6addr[15] = 0;
			break;
		case DUAL_IPV6_HASHMASK_PREFIX_96:
			ip6addr[12] = 0;
			ip6addr[13] = 0;
			ip6addr[14] = 0;
			ip6addr[15] = 0;
			break;
		default:
			WARNING("Error Input prefixBits:%x",prefixBits);
	}
	return;
}
void rtk_fc_dualHash_XLAT(rtk_fc_dualHashKey_t* pattern,rtk_fc_pktHdr_t *pPktHdr,rtk_fc_flow_dual_direction_t dual_direct)
{
	if(dual_direct)
	{
		//downstream
		if(pPktHdr->ip6h)
		{
			memcpy(&pattern->local_ip_addr,pPktHdr->ip6h->daddr.s6_addr,sizeof(struct in6_addr));
			DEBUG("downstream v6 daddr = %pI6 saddr = %pI6",&pPktHdr->ip6h->daddr,&pPktHdr->ip6h->saddr);
			rtk_fc_dualHash_XLAT_ipMasked((uint8 *)&pattern->local_ip_addr,fc_db.dualIpv6HashMask& (~DUAL_IPV6_HASHMASK_ALL));
			DEBUG("local_ip_addr Masked v6 addr :%pI6 ",&pattern->local_ip_addr);
		}		
	}
	else
	{
		//upstream
		if(pPktHdr->ip6h)
		{
			memcpy(&pattern->local_ip_addr,pPktHdr->ip6h->saddr.s6_addr,sizeof(struct in6_addr));
			DEBUG("upstream v6 saddr = %pI6 daddr = %pI6",&pPktHdr->ip6h->saddr,&pPktHdr->ip6h->daddr);
			rtk_fc_dualHash_XLAT_ipMasked((uint8 *)&pattern->local_ip_addr,fc_db.dualIpv6HashMask& (~DUAL_IPV6_HASHMASK_ALL));
			DEBUG("local_ip_addr Masked v6 addr :%pI6 ",&pattern->local_ip_addr);
		}
	}
	pattern->otherParameter[0]=fc_db.dualIpv6HashMask& (~DUAL_IPV6_HASHMASK_ALL);	//compare IP address 128bits
	return;
}




/*
* init extra tag action list
*/
int rtk_fc_dualInfoInit(void)
{

	/* RTK_FC_DUALTYPE_IP4INIP6 */
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_IP4INIP6].dual_hash_patten_fill = &rtk_fc_dualHash_ip4Inip6;
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_IP4INIP6].dual_netifInfo_collect = &rtk_fc_netif_dual_set_4in6;
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_IP4INIP6].dual_hw_setting = &_rtk_fc_flow_setupIP4InIP6;

	/* RTK_FC_DUALTYPE_PPTP */
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_PPTP].dual_hash_patten_fill = &rtk_fc_dualHash_PPTP;
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_PPTP].dual_netifInfo_collect = &rtk_fc_netif_dual_set_PPTP ;
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_PPTP].dual_hw_setting = &_rtk_fc_flow_setupPPTP;

	/* RTK_FC_DUALTYPE_L2TP */
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_L2TP].dual_hash_patten_fill = &rtk_fc_dualHash_L2TP;
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_L2TP].dual_netifInfo_collect = &rtk_fc_netif_dual_set_L2TP ;
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_L2TP].dual_hw_setting = &_rtk_fc_flow_setupL2TP;

	/* RTK_FC_DUALTYPE_IPSEC */
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_IPSEC].dual_hash_patten_fill = NULL;
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_IPSEC].dual_netifInfo_collect = NULL ;
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_IPSEC].dual_hw_setting = NULL;

	/* RTK_FC_DUALTYPE_GRE_ETH_BR */
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_GRE_ETH_BR].dual_hash_patten_fill = &rtk_fc_dualHash_GRE_ETH_BR;
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_GRE_ETH_BR].dual_netifInfo_collect = &rtk_fc_netif_dual_set_GRE_ETH_BR ;
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_GRE_ETH_BR].dual_hw_setting = &_rtk_fc_flow_setupGreEthBr;

	/* RTK_FC_DUALTYPE_VXLAN */
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_VXLAN].dual_hash_patten_fill = &rtk_fc_dualHash_VXLAN;
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_VXLAN].dual_netifInfo_collect = &rtk_fc_netif_dual_set_VXLAN ;
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_VXLAN].dual_hw_setting = &_rtk_fc_flow_setupVXLAN;

	/* RTK_FC_DUALTYPE_6RD */
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_6RD].dual_hash_patten_fill = &rtk_fc_dualHash_6RD;
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_6RD].dual_netifInfo_collect = &rtk_fc_netif_dual_set_6RD ;
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_6RD].dual_hw_setting =   &_rtk_fc_flow_setup6RD;

	/* RTK_FC_DUALTYPE_MAPT */
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_MAPT].dual_hash_patten_fill = &rtk_fc_dualHash_MAPT;
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_MAPT].dual_netifInfo_collect = &rtk_fc_netif_dual_set_MAPT;
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_MAPT].dual_hw_setting = &_rtk_fc_flow_setupMAPT;

	/* RTK_FC_DUALTYPE_XLAT464 */
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_XLAT464].dual_hash_patten_fill = &rtk_fc_dualHash_XLAT;
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_XLAT464].dual_netifInfo_collect = &rtk_fc_netif_dual_set_XLAT;
	fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_XLAT464].dual_hw_setting = &_rtk_fc_flow_setupXLAT;


	return SUCCESS;
}


/*=====================================================
* MAP-E:
*	In addition to DMR, MAP-E can also support FMR at the same time. 
* 		1. (DMR) CE1 -> BR
*		2. (FMR) CE1 -> CE2
*			2.1 CE1 -> CE2(Support FMR) -> CE1
*			2.2 CE1 -> CE2(Not Support FMR) -> BR -> CE1
*	- For case 1 and 2, the IPv6 dst addresses of upstream packets forwarded through DMR/FMR are different.
*	So the IPv6 dstAddr needs to be updated after inserting thr outHdr for MAP-E upstream .
*	- For case 2.2, the IPv6 dstAddr of upstream packets is different from the IPv6 srcAddr of downstream
*	So the check of IPv6 srcAddr in downstream should be bypassed
*=====================================================
*/

/*
* Init the MAP-E IPv6 dst addresses
*/
int rtk_fc_mape_dst6Info_init(void){
	int i=0, j=0;
	for (i = 0; i < RTK_FC_TABLESIZE_INTF; i++){
		memset(&(fc_db.mapeDst6s[i]), 0x00, sizeof(rtk_fc_mape_dst6_t));		
		INIT_LIST_HEAD(&(fc_db.mapeDst6s[i].free_list));
		INIT_LIST_HEAD(&(fc_db.mapeDst6s[i].used_list));
		for (j = 0; j < RTK_FC_MAPE_FMR_DST6_MAX_NUMS; j++){
			fc_db.mapeDst6s[i].dst6_info[j].index = j;
			list_add_tail(&(fc_db.mapeDst6s[i].dst6_info[j].list), &(fc_db.mapeDst6s[i].free_list));
		}
	}
	return SUCCESS;
}



int rtk_fc_is_mape_upstream_flow(uint32 flowIdx)
{
	rtk_fc_tableFlow_t *pFlowTable = &fc_db.flowTbl[flowIdx];
	rtk_rg_asic_path5_entry_t *pFlowPath5 = (rtk_rg_asic_path5_entry_t *)pFlowTable->pFlowEntry;

	if (fc_db.netifTbl[NETIF_HWTOSW(pFlowPath5->out_intf_idx)].dualType == RTK_FC_DUALTYPE_IP4INIP6 && fc_db.netifTbl[NETIF_HWTOSW(pFlowPath5->out_intf_idx)].dualUniInfo.ip4Inip6_info.isMape)
	{
		return TRUE;
	}
	return FALSE;

}

/*
* Record the IPv6 dst address carried in pPktHdr, when adding the MAP-E upstream flow5 in egress.
* And hook the index of dst6 address to the corresponding pFlowTable->mapeInfo.mape_out_dst6_idx.
*/
int rtk_fc_mape_dst6_decision(rtk_fc_pktHdr_t *pPktHdr, uint32 flowIdx){
	rtk_fc_mape_fmr_dst6_t *mape_fmr;
	rtk_fc_tableFlow_t *pFlowTable = &fc_db.flowTbl[flowIdx];
	rtk_rg_asic_path5_entry_t *pFlowPath5 = (rtk_rg_asic_path5_entry_t *)pFlowTable->pFlowEntry;		
	uint32 netif_idx = pFlowPath5->out_intf_idx;
	struct list_head *p=NULL, *n=NULL;

	if (!pPktHdr || !(pPktHdr->outer_ip6h))
		return (SUCCESS);
	
	if (fc_db.mapeDst6s[netif_idx].in_used == 0)
		fc_db.mapeDst6s[netif_idx].in_used = 1;

	if (!list_empty(&(fc_db.mapeDst6s[netif_idx].used_list))){	
		list_for_each_safe(p, n, &(fc_db.mapeDst6s[netif_idx].used_list)) {
			mape_fmr = list_entry(p, rtk_fc_mape_fmr_dst6_t, list);
			if (memcmp(&(mape_fmr->d_addr6), &(pPktHdr->outer_ip6h->daddr), sizeof(struct in6_addr)) == 0){
				mape_fmr->ref_cnt++;
	
				pFlowTable->mapeInfo.mape_out_dst6_idx = mape_fmr->index;
	
				DEBUG("[flowIdx=%d] netif_idx=%d, fmr_idx=%d, mape_fmr->ref_cnt=%d\n", flowIdx, netif_idx, mape_fmr->index, mape_fmr->ref_cnt);

				return (SUCCESS);
			}else{
				continue;
			}
		}		
	}
	
	if (!list_empty(&(fc_db.mapeDst6s[netif_idx].free_list))){
		mape_fmr = list_first_entry(&(fc_db.mapeDst6s[netif_idx].free_list), rtk_fc_mape_fmr_dst6_t, list);	
		list_del(&(mape_fmr->list));
		list_add_tail(&(mape_fmr->list), &(fc_db.mapeDst6s[netif_idx].used_list));
		
		memcpy(&(mape_fmr->d_addr6), &(pPktHdr->outer_ip6h->daddr), sizeof(struct in6_addr));
		mape_fmr->ref_cnt = 1;

		pFlowTable->mapeInfo.mape_out_dst6_idx = mape_fmr->index;

		DEBUG("[flowIdx=%d] netif_idx=%d, fmr_idx=%d, mape_fmr->ref_cnt=%d\n", flowIdx, netif_idx, mape_fmr->index, mape_fmr->ref_cnt);

		return (SUCCESS);
	}	
	return (FAILED);	
}

/*
* After inserting the out header into the MAP-E upstream packet in shortcut, 
* update the IPv6 dst address based on the pFlowTable->mapeInfo.mape_out_dst6_idx.
*/
void rtk_fc_mape_dst6_update(rtk_fc_pktHdr_t *pPktHdr, uint32 outif_idx, uint16 fmr_idx){
	struct ipv6hdr *outer_ip6h = (struct ipv6hdr *)pPktHdr->outer_ip6h;
	rtk_fc_mape_fmr_dst6_t *mape_fmr;
	
	if (outif_idx >= RTK_FC_TABLESIZE_INTF)
		return;
		
	if (fmr_idx >= RTK_FC_MAPE_FMR_DST6_MAX_NUMS)
		return;

	mape_fmr = &(fc_db.mapeDst6s[outif_idx].dst6_info[fmr_idx]);
	if (mape_fmr->ref_cnt > 0)
		memcpy(&outer_ip6h->daddr, &mape_fmr->d_addr6, sizeof(struct in6_addr));	
}


static void _rtk_fc_mape_dst6_refcnt_put(uint32 netif_idx, uint16 fmr_idx){
	rtk_fc_mape_fmr_dst6_t *mape_fmr;
	struct list_head *list;

	mape_fmr = &(fc_db.mapeDst6s[netif_idx].dst6_info[fmr_idx]);
	mape_fmr->ref_cnt--;

	DEBUG("netif_idx=%d, fmr_idx=%d, mape_fmr->ref_cnt=%u\n", netif_idx, fmr_idx, mape_fmr->ref_cnt);	
	if (mape_fmr->ref_cnt <= 0)	{
		list = &(mape_fmr->list);
		list_del(list);
		list_add_tail(list, &(fc_db.mapeDst6s[netif_idx].free_list));
	}
}

/*
* When the flow5 which representing the MAP-E upstream timeout,
* the ref_cnt of IPv6 dst address referenced by it needs to be decreased
*/
int rtk_fc_mape_dst6_refcnt_put(uint32 flowIdx){
	rtk_fc_tableFlow_t *pFlowTable = &fc_db.flowTbl[flowIdx];
	rtk_rg_asic_path5_entry_t *pFlowPath5 = (rtk_rg_asic_path5_entry_t *)pFlowTable->pFlowEntry;

	//if (!(pFlowPath5->out_intf_idx>=0 && pFlowPath5->out_intf_idx<RTK_FC_TABLESIZE_INTF))
		//return FAILED;
	
	if (pFlowTable->mapeInfo.mape_out_dst6_idx >= RTK_FC_MAPE_FMR_DST6_MAX_NUMS)
		return FAILED;
	
	DEBUG("[flowIdx=%d] ", flowIdx);

	_rtk_fc_mape_dst6_refcnt_put(pFlowPath5->out_intf_idx, pFlowTable->mapeInfo.mape_out_dst6_idx);
	return (SUCCESS);
}

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
rtk_fc_ret_t rtk_fc_dual_control_content_set6RD(uint32 dual_content_tbl_idx, rtk_fc_g3IgrExtraInfo_t *pG3IgrExtraInfo, u8 *outer_header_content, int outer_header_size, rtk_fc_pktHdr_t *pPktHdr)
{
	rtk_fc_ret_t ret=RTK_FC_RET_OK;
	//l3pe_dual_tbl_entry_t dual_buffer_entry;
	//int aal_ret, offset = 0;

	if(pG3IgrExtraInfo->dualHdrType == RTK_FC_DUALTYPE_6RD)
	{
		// DMAC(6)+SMAC(6)+[SVLAN(4)+CVLAN(4)]+Ethertype(2)+[PPPoE(8)]+IP header(20/40)+UDP(8)+VxLAN(8)
		memset(outer_header_content, 0, outer_header_size);
		
		//6RD-TODO: get Outer content add set to content table
		
	}

	
	return ret;
}


rtk_fc_ret_t _rtk_fc_dualHeader_ctrl_setup6RD(int dualControlIdx, rtk_fc_pktHdr_t *pPktHdr)
{
	rtk_fc_ret_t ret=RTK_FC_RET_OK;
	l3pe_dual_control_tbl_entry_t dual_ctrl_entry;
	int aal_ret;
	memset(&dual_ctrl_entry, 0, sizeof(l3pe_dual_control_tbl_entry_t));
	
	{
		dual_ctrl_entry.dual_ctrl_hdr_fmt			= 0;
		dual_ctrl_entry.dual_ctrl_hdr_len			= 20;	//dual_buffer_size; IPv4 header 
		/* Usually 6RD tunnel device MTU is configured as 1480, ASIC does not have to plus mtu_diff to check packet length */
		//dual_ctrl_entry.dual_ctrl_mtu_diff			= 20; 	//IPv4 header(20)
		dual_ctrl_entry.dual_ctrl_ppp_ip_ver_ost		= 127;
		dual_ctrl_entry.dual_ctrl_rep_ip_en 			= 1;
		dual_ctrl_entry.dual_ctrl_rep_ip_ver			= 0;	//IPv4
		dual_ctrl_entry.dual_ctrl_rep_len_ost1			= 2;
		dual_ctrl_entry.dual_ctrl_rep_len_diff1 		= 20;
		dual_ctrl_entry.dual_ctrl_rep_len_ost2			= 127;
		dual_ctrl_entry.dual_ctrl_rep_len_diff2 		= 16;
		dual_ctrl_entry.dual_ctrl_ip4_ost			= 0;
		dual_ctrl_entry.dual_ctrl_ip6_oft			= 127;
		dual_ctrl_entry.dual_ctrl_gre_seq_ack_ost		= 127;
		dual_ctrl_entry.dual_ctrl_gre_mod_fld 		= 0;
		dual_ctrl_entry.dual_ctrl_rep_ip_dscp_en 		= 1;
		dual_ctrl_entry.dual_ctrl_rep_ip_ecn_en 		= 1;
		dual_ctrl_entry.dual_ctrl_rep_ip_len			= 1;
		dual_ctrl_entry.dual_ctrl_rep_ip_len_diff		= 20;
		dual_ctrl_entry.dual_ctrl_rep_ip_ptc			= 0;
		dual_ctrl_entry.dual_ctrl_rep_ip_ttl			= 0;
		dual_ctrl_entry.dual_ctrl_mapt_pre_len			= 0;
		dual_ctrl_entry.dual_ctrl_map_drf_ver 			= 0;
		dual_ctrl_entry.dual_ctrl_6rd_dip_pre_len 		= 0;
		dual_ctrl_entry.dual_ctrl_6rd_dip_suf_str_bit 		= 32;
		dual_ctrl_entry.dual_ctrl_dsl_uc_chk			= 0;
		dual_ctrl_entry.dual_ctrl_dsl_mc_chk			= 0;
		dual_ctrl_entry.dual_ctrl_l4sum_rec 			= 0;
		dual_ctrl_entry.dual_ctrl_vxlan_sp_ost			= 127;
		dual_ctrl_entry.dual_ctrl_inr_to_out_fld		= 0xb0;
		dual_ctrl_entry.dual_ctrl_tos_hsh_sel 			= 0;
		dual_ctrl_entry.dual_ctrl_mc_mac_map			= 0;
	}
	aal_ret = aal_pe_dual_control_entry_add(dualControlIdx, &dual_ctrl_entry);  // VXLAN-TODO:Idx getting
	if(aal_ret)
	{
		WARNING("Add dual control failed, ret = %d", aal_ret);
		ret = RTK_FC_RET_ERR;
	}
	return ret;
}

rtk_fc_ret_t rtk_fc_dual_control_entry_setVXLAN(int idx, rtk_fc_pktHdr_t *pPktHdr)
{
	rtk_fc_ret_t ret=RTK_FC_RET_OK;
	l3pe_dual_control_tbl_entry_t dual_ctrl_entry;
	int aal_ret;
	memset(&dual_ctrl_entry, 0, sizeof(l3pe_dual_control_tbl_entry_t));
	DEBUG("VXLAN set dual header control reg!");
	


	dual_ctrl_entry.dual_ctrl_hdr_fmt				= 1;
	dual_ctrl_entry.dual_ctrl_hdr_len				= pPktHdr->vxlan_info.outerTag_off;//dual_buffer_size;						// DMAC(6)+SMAC(6)+[SVLAN(4)+CVLAN(4)]+Ethertype(2)+[PPPoE(8)]+IP header(20/40)+UDP(8)+VxLAN(8)
	dual_ctrl_entry.dual_ctrl_mtu_diff				= 0;//(pPktHdr->vxlan_info.outer_isV6)?(40+8+8):(20+8+8); 	// IP header(20/40)+UDP(8)+VxLAN(8)
	dual_ctrl_entry.dual_ctrl_ppp_ip_ver_ost		= 127;
	dual_ctrl_entry.dual_ctrl_rep_ip_en 			= FALSE;								// don't care
	dual_ctrl_entry.dual_ctrl_rep_ip_ver			= 0;									// don't care
	if(pPktHdr->outer_ppph)
	{
		// DMAC(6)+SMAC(6)+[SVLAN(4)+CVLAN(4)]+Ethertype(2)+pppoe payload length field(4)
		dual_ctrl_entry.dual_ctrl_rep_len_ost1		= 14+((pPktHdr->vxlan_info.outer_svh)?4:0)+((pPktHdr->vxlan_info.outer_cvh)?4:0)+4;
		// PPP(2)+IP header(20/40)+UDP(8)+VxLAN(8)
		dual_ctrl_entry.dual_ctrl_rep_len_diff1 	= (pPktHdr->vxlan_info.outer_isV6)?(2+40+8+8):(2+20+8+8);
	}
	else
	{
		dual_ctrl_entry.dual_ctrl_rep_len_ost1		= 127;
		dual_ctrl_entry.dual_ctrl_rep_len_diff1 	= 0;
	}
	// DMAC(6)+SMAC(6)+[SVLAN(4)+CVLAN(4)]+Ethertype(2)+[PPPoE(8)]+IP header(20/40)+UDP length field offset(4)
	dual_ctrl_entry.dual_ctrl_rep_len_ost2			= 14+((pPktHdr->vxlan_info.outer_svh)?4:0)+((pPktHdr->vxlan_info.outer_cvh)?4:0)+((pPktHdr->vxlan_info.outer_ppph)?8:0)+((pPktHdr->vxlan_info.outer_isV6)?40:20)+4;
	dual_ctrl_entry.dual_ctrl_rep_len_diff2 		= 16;										// UDP(8)+VxLAN(8)
#if defined(CONFIG_FC_RTL9607F_SERIES)	
	dual_ctrl_entry.dual_ctrl_rep_len_ost3			= 127;
	dual_ctrl_entry.dual_ctrl_rep_len_diff3 		= 0;										// UDP(8)+VxLAN(8)
#endif	
	// DMAC(6)+SMAC(6)+[SVLAN(4)+CVLAN(4)]+Ethertype(2)+[PPPoE(8)]
	dual_ctrl_entry.dual_ctrl_ip4_ost				= (pPktHdr->vxlan_info.outer_isV6)?127:(14+((pPktHdr->vxlan_info.outer_svh)?4:0)+((pPktHdr->vxlan_info.outer_cvh)?4:0)+((pPktHdr->vxlan_info.outer_ppph)?8:0));
	dual_ctrl_entry.dual_ctrl_ip6_oft				= (pPktHdr->vxlan_info.outer_isV6)?(14+((pPktHdr->vxlan_info.outer_svh)?4:0)+((pPktHdr->vxlan_info.outer_cvh)?4:0)+((pPktHdr->vxlan_info.outer_ppph)?8:0)):127;
	dual_ctrl_entry.dual_ctrl_gre_seq_ack_ost		= 127;
	//dual_ctrl_entry.dual_ctrl_gre_mod_fld 		= 0;
	dual_ctrl_entry.dual_ctrl_rep_ip_dscp_en 		= FALSE;
	dual_ctrl_entry.dual_ctrl_rep_ip_ecn_en 		= FALSE;
	dual_ctrl_entry.dual_ctrl_rep_ip_len			= 1;
	dual_ctrl_entry.dual_ctrl_rep_ip_len_diff		= (pPktHdr->vxlan_info.outer_isV6)?(8+8):(20+8+8); 		// IP header(20/40)+UDP(8)+VxLAN(8)
	dual_ctrl_entry.dual_ctrl_rep_len_diff2_pad_ctrl	= 1;
	dual_ctrl_entry.dual_ctrl_rep_len_diff1_pad_ctrl	= 1;
	dual_ctrl_entry.dual_ctrl_rep_ip_len_diff_pad_ctrl	= 1;
	//dual_ctrl_entry.dual_ctrl_rep_ip_ptc			= 0;
	//dual_ctrl_entry.dual_ctrl_rep_ip_ttl			= 0;
	//dual_ctrl_entry.dual_ctrl_mapt_pre_len		= 0;
	//dual_ctrl_entry.dual_ctrl_map_drf_ver 		= 0;
	//dual_ctrl_entry.dual_ctrl_6rd_dip_pre_len 	= 0;
	//dual_ctrl_entry.dual_ctrl_6rd_dip_suf_str_bit = 0;
	//dual_ctrl_entry.dual_ctrl_dsl_uc_chk			= 0;
	//dual_ctrl_entry.dual_ctrl_dsl_mc_chk			= 0;
	dual_ctrl_entry.dual_ctrl_l4sum_rec 			= 0;
	// DMAC(6)+SMAC(6)+[SVLAN(4)+CVLAN(4)]+Ethertype(2)+[PPPoE(8)]+IP header(20/40)
	dual_ctrl_entry.dual_ctrl_vxlan_sp_ost			= 14+((pPktHdr->vxlan_info.outer_svh)?4:0)+((pPktHdr->vxlan_info.outer_cvh)?4:0)+((pPktHdr->vxlan_info.outer_ppph)?8:0)+((pPktHdr->vxlan_info.outer_isV6)?40:20);
	dual_ctrl_entry.dual_ctrl_inr_to_out_fld		= 0x7f; // bit [0,1,2,3,4,5,6]
	dual_ctrl_entry.dual_ctrl_tos_hsh_sel 			= 0; //VXLAN-TODO: FIXME
	//dual_ctrl_entry.dual_ctrl_mc_mac_map			= 0;

	
	aal_ret = aal_pe_dual_control_entry_add(idx, &dual_ctrl_entry);  // VXLAN-TODO:Idx getting
	if(aal_ret)
	{
		WARNING("Add dual control failed, ret = %d", aal_ret);
		ret = RTK_FC_RET_ERR;
	}
	return ret;
}


rtk_fc_ret_t rtk_fc_dual_control_content_setVXLAN(uint32 dual_content_tbl_idx, u8 *outer_header_content, int outer_header_size)
{
	rtk_fc_ret_t ret=RTK_FC_RET_OK;
	l3pe_dual_tbl_entry_t dual_buffer_entry;
	int aal_ret;

	
	// DMAC(6)+SMAC(6)+[SVLAN(4)+CVLAN(4)]+Ethertype(2)+[PPPoE(8)]+IP header(20/40)+UDP(8)+VxLAN(8)
	
	memset(dual_buffer_entry.buff, 0, L3PE_DUAL_CONTENT_TBL_BUFF_SIZE_MAX);
	memcpy(dual_buffer_entry.buff, outer_header_content, (outer_header_size<=L3PE_DUAL_CONTENT_TBL_BUFF_SIZE_MAX) ? outer_header_size : L3PE_DUAL_CONTENT_TBL_BUFF_SIZE_MAX); 

	aal_ret = aal_pe_dual_content_entry_add(dual_content_tbl_idx, &dual_buffer_entry);
	if(aal_ret == (-4) ) //AAL_E_ENTRY_USED
		DEBUG("Outer is added before.");
	else if(aal_ret !=RTK_FC_RET_OK)
		WARNING("Add dual content failed, ret = %d", aal_ret);
	else{
		ret = aal_ret;
		//return ret;
	}

	if(outer_header_size > L3PE_DUAL_CONTENT_TBL_BUFF_SIZE_MAX)
	{
		memset(dual_buffer_entry.buff, 0, L3PE_DUAL_CONTENT_TBL_BUFF_SIZE_MAX);
		
		if((outer_header_size-L3PE_DUAL_CONTENT_TBL_BUFF_SIZE_MAX) > L3PE_DUAL_CONTENT_TBL_BUFF_SIZE_MAX)
			aal_ret = RTK_FC_RET_ERR_BUF_OVERFLOW;
		else {
			memcpy(dual_buffer_entry.buff, outer_header_content+L3PE_DUAL_CONTENT_TBL_BUFF_SIZE_MAX, outer_header_size-L3PE_DUAL_CONTENT_TBL_BUFF_SIZE_MAX);
		
			aal_ret = aal_pe_dual_content_entry_add(dual_content_tbl_idx+1, &dual_buffer_entry);
		}

		if(aal_ret == (-4) ) //AAL_E_ENTRY_USED
			DEBUG("Outer is added before.");
		else if(aal_ret !=RTK_FC_RET_OK)
			WARNING("Add dual content failed, ret = %d", aal_ret);
		else{
			ret = aal_ret;
			//return ret;
		}

	}


	
	return ret;
}


#endif

void rtk_fc_dual_4in6NetifInfo_update_by_netdev_priv(uint32 swNetifIdx,struct net_device *dev)
{
	struct ip6_tnl *ip6Tnl_priv=NULL;
	struct __ip6_tnl_parm *parm = NULL;

	ip6Tnl_priv = (struct ip6_tnl*)netdev_priv(dev);
	parm = &ip6Tnl_priv->parms;

	TRACE("dev->name:%s(%d)  ip6Tnl_priv->dev->name:%s(%d) ",dev->name,dev->ifindex,ip6Tnl_priv->dev->name,ip6Tnl_priv->dev->ifindex);
	TRACE("parm name:%s link:%d(ifidx) proto:%d encap_limit:%d  hop_limit:%d flowinfo:%d flags:0x%x i_flags:0x%x o_flags:0x%x i_key:0x%x o_key:0x%x",
		parm->name,parm->link,parm->proto,parm->encap_limit,parm->hop_limit,parm->flowinfo,parm->flags,parm->i_flags,parm->o_flags,parm->i_key,parm->o_key);
	TRACE("laddr:%pI6 raddr:%pI6",&parm->laddr,&parm->raddr);

	memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.ip4Inip6_info.local6Addr,&parm->laddr,sizeof(parm->laddr));
	memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.ip4Inip6_info.remote6Addr,&parm->raddr,sizeof(parm->raddr));

	return;
}



static unsigned int _rtk_fc_dualHashKey(rtk_fc_dualHashKey_t *key)
{
	uint32 dualHashKey=0;
	uint32 rdmLoIp0=0;
	uint8 *p_randomLoIp0=(uint8 *)&rdmLoIp0;
	uint8 *p_local_ip_addr=(uint8 *)&key->local_ip_addr[0];
	p_randomLoIp0[0]=p_local_ip_addr[3];
	p_randomLoIp0[1]=p_local_ip_addr[0];
	p_randomLoIp0[2]=p_local_ip_addr[2];
	p_randomLoIp0[3]=p_local_ip_addr[1];
	
	dualHashKey ^= rdmLoIp0^key->local_ip_addr[1]^key->local_ip_addr[2]^key->local_ip_addr[3];
	dualHashKey ^= (key->remote_ip_addr[0]+key->remote_ip_addr[1])^(key->remote_ip_addr[2]+key->remote_ip_addr[3]);	
	dualHashKey ^= ((key->cvlanId) | (key->svlanId<<16) );
	dualHashKey ^= key->otherParameter[0]^key->otherParameter[1]^key->otherParameter[2]^key->otherParameter[3];
	dualHashKey ^= (key->dualType <<24);
	return dualHashKey;
}


unsigned int rtk_fc_dualHashKey_remap_MAPT_XLAT(unsigned int originHashKey,uint8 usingMsb)
{
	uint16 tmpKey;
	if(usingMsb)
	{
		//using MSB 16bits
		tmpKey=(originHashKey>>16)&0xffff;
		return ((originHashKey&0xffff)^(tmpKey))<<16;	//high 16bits
	}
	else
	{
		//using LSB 16bits
		tmpKey=(originHashKey>>16)&0xffff;
		return ((originHashKey&0xffff)^(tmpKey));	//low 16bits
	}
}
unsigned int rtk_fc_dualHashKey(rtk_fc_dualHdrtype_t dualType ,rtk_fc_dualHashKey_t *key)
{
	
	uint32 dualHashKey =0;

	key->dualType = dualType;
	dualHashKey = _rtk_fc_dualHashKey(key);
	//remapping MAPT & XLAT
	if(dualType==RTK_FC_DUALTYPE_MAPT)
		dualHashKey=rtk_fc_dualHashKey_remap_MAPT_XLAT(dualHashKey,TRUE);
	else if (dualType==RTK_FC_DUALTYPE_XLAT464)
		dualHashKey=rtk_fc_dualHashKey_remap_MAPT_XLAT(dualHashKey,FALSE);

	return dualHashKey;
}


/*
	imagine dualHashKeyPatten is ingresCLS outer patten or path 6 keys
	Careful: should reference isFromWanIgr to get local_ip_addr/remote_ip_addr
*/
unsigned int rtk_fc_netif_dualHashKey_config_byPktHdr(rtk_fc_dualHdrtype_t dualType, rtk_fc_pktHdr_t *pPktHdr,rtk_fc_flow_dual_direction_t dual_direct)
{
	uint32 dualHashKey=0;
	uint32 ip4toip6_downstream=0;
	rtk_fc_dualHashKey_t dualHashKeyPatten;

	if(dualType == RTK_FC_DUALTYPE_NONE){
		if(fc_db.dualIpv6HashMask && (dual_direct==RTK_FC_FLOW_DUAL_DIRECTION_DOWNSTREAM_IGR) && pPktHdr->ip6h){
			TRACE("Maybe single v6 or ip4toip6 downstream...prepare hashkey by dualIpv6HashMask 0x%x",fc_db.dualIpv6HashMask);
			ip4toip6_downstream=1;
		}else
			return 0;
	}
	
	bzero(&dualHashKeyPatten,sizeof(dualHashKeyPatten));
	if(pPktHdr->cvh)
		dualHashKeyPatten.cvlanId = pPktHdr->cvlanid;
	else
		dualHashKeyPatten.cvlanId = SIGNED_INVALID;
	if(pPktHdr->svh)
		dualHashKeyPatten.svlanId = pPktHdr->svlanid;
	else
		dualHashKeyPatten.svlanId = SIGNED_INVALID;
	/* 
		only mapt/xlat464 dualType can't decision by packet parser
		We assume that ipv6 only may be mapt/xlat464 calculate the dualHash
	*/
	if(ip4toip6_downstream){

		if(fc_db.dualIpv6HashMask&DUAL_IPV6_HASHMASK_ALL)
		{
			//MAPT
			if(fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_MAPT].dual_hash_patten_fill)
				fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_MAPT].dual_hash_patten_fill(&dualHashKeyPatten,pPktHdr,dual_direct);
			dualHashKey|=rtk_fc_dualHashKey(RTK_FC_DUALTYPE_MAPT,&dualHashKeyPatten);
		}
		if (fc_db.dualIpv6HashMask >= DUAL_IPV6_HASHMASK_PREFIX_96)
		{
			//XLAT464
			if(fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_XLAT464].dual_hash_patten_fill)
				fc_db.dualCallBackFunc[RTK_FC_DUALTYPE_XLAT464].dual_hash_patten_fill(&dualHashKeyPatten,pPktHdr,dual_direct);
			dualHashKey|=rtk_fc_dualHashKey(RTK_FC_DUALTYPE_XLAT464,&dualHashKeyPatten);
		}
		
	}else{
		//refer to rtk_fc_dualInfoInit 
		if(fc_db.dualCallBackFunc[dualType].dual_hash_patten_fill)
			fc_db.dualCallBackFunc[dualType].dual_hash_patten_fill(&dualHashKeyPatten,pPktHdr,dual_direct);

		dualHashKey=rtk_fc_dualHashKey(dualType,&dualHashKeyPatten);
	}

	return dualHashKey;

}

/*
	upstream egr dual netif and downstream Igr dual netif setting
	WARNING: if(downstream Igr) the skb->fcIngressData is uninitialized
*/
int rtk_fc_netif_dual_set_4in6(uint32 swNetifIdx,rtk_fc_flow_dual_direction_t dual_direct, struct sk_buff *skb, rtk_fc_pktHdr_t *pPktHdr)
{
	if(dual_direct == RTK_FC_FLOW_DUAL_DIRECTION_UPSTREAM_EGR)
	{
		if(pPktHdr->outer_ip6h)
		{
			memcpy(fc_db.netifTbl[swNetifIdx].dualUniInfo.ip4Inip6_info.local6Addr.s6_addr,pPktHdr->outer_ip6h->saddr.s6_addr,sizeof(struct in6_addr));
			memcpy(fc_db.netifTbl[swNetifIdx].dualUniInfo.ip4Inip6_info.remote6Addr.s6_addr,pPktHdr->outer_ip6h->daddr.s6_addr,sizeof(struct in6_addr));
		}
	}
	else if (dual_direct == RTK_FC_FLOW_DUAL_DIRECTION_DOWNSTREAM_IGR)
	{
		if(pPktHdr->outer_ip6h)
		{
			memcpy(fc_db.netifTbl[swNetifIdx].dualUniInfo.ip4Inip6_info.local6Addr.s6_addr,pPktHdr->outer_ip6h->daddr.s6_addr,sizeof(struct in6_addr));
			memcpy(fc_db.netifTbl[swNetifIdx].dualUniInfo.ip4Inip6_info.remote6Addr.s6_addr,pPktHdr->outer_ip6h->saddr.s6_addr,sizeof(struct in6_addr));
		}

		//for early check support
		fc_db.netifTbl[swNetifIdx].dualHdr_downstream_earlyChk=1;
		fc_db.netifTbl[swNetifIdx].dualHdr_outer_ipversion=1;
		fc_db.netifTbl[swNetifIdx].dualHdr_outer_L3_Offset=0;  //only valid when FC_DUALHDR_HW_CTRL_PKT_BEGINIG
		fc_db.netifTbl[swNetifIdx].dualHdr_outer_length = (void*)pPktHdr->iph -(void*)pPktHdr->outer_ip6h;
	}

	if (pPktHdr->fwdType == RTK_FC_FWDTYPE_NAPT)
	{
		fc_db.netifTbl[swNetifIdx].dualUniInfo.ip4Inip6_info.isMape=1;
	}
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	{
		int i,j;
		struct net_device *p_PScacheDev[DEV_STACK_MAX]={NULL,NULL,NULL,NULL};

		rtk_fc_psIfidxStackKey_to_dev(fc_db.netifTbl[swNetifIdx].psIfidxStackKey,p_PScacheDev);
		for(i=0;i<DEV_STACK_MAX;i++)
		{
			if(p_PScacheDev[i])
			{
				for(j=DEVIFIDX_VALID_MIN;j<RTK_FC_DEV_GW_MAC_TBL;j++)
				{
					if(!fc_db.devGwMacTbl[j].dev)
						continue;

					if(fc_db.devGwMacTbl[j].dev == p_PScacheDev[i] && fc_db.devGwMacTbl[j].mapet_fmr_hwidx_valid)
					{
						fc_db.netifTbl[swNetifIdx].hwFmrIdx =  fc_db.devGwMacTbl[j].mapet_fmr_hwidx;
						break;
					}
				}
			}
		}
	}
#endif

	return SUCCESS;
}

/*
	upstream egr dual netif and downstream Igr dual netif setting
	WARNING: if(downstream Igr) the skb->fcIngressData is uninitialized
*/
int rtk_fc_netif_dual_set_PPTP(uint32 swNetifIdx,rtk_fc_flow_dual_direction_t dual_direc, struct sk_buff *skb, rtk_fc_pktHdr_t *pPktHdr)
{
	/*
	*  For PPTP, pptp interface should record call id to identify the correct interface.
	*/
	if(dual_direc == RTK_FC_FLOW_DUAL_DIRECTION_UPSTREAM_EGR)
	{
		DEBUG("[Dual][PPTP]Need to add peer call id : %d to interface table.", pPktHdr->tunnelInfo.pptp.grecallid);

		if(fc_db.netifTbl[swNetifIdx].dualUniInfo.psGreInfo.peer_call_id != pPktHdr->tunnelInfo.pptp.grecallid){
			TRACE("[PPTP]Find pptp device! netif, ready to set peer call id = %d", pPktHdr->tunnelInfo.pptp.grecallid);
			fc_db.netifTbl[swNetifIdx].dualUniInfo.psGreInfo.peer_call_id = pPktHdr->tunnelInfo.pptp.grecallid;
		}
		if(pPktHdr->outer_ip6h)
		{
			fc_db.netifTbl[swNetifIdx].dualUniInfo.psGreInfo.outer_ipversion=1;
			memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.psGreInfo.localV6Ip,pPktHdr->ip6h->saddr.s6_addr,sizeof(struct in6_addr));
			memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.psGreInfo.remoteV6Ip,pPktHdr->ip6h->daddr.s6_addr,sizeof(struct in6_addr));
		}
		else if(pPktHdr->outer_iph)
		{
			fc_db.netifTbl[swNetifIdx].dualUniInfo.psGreInfo.outer_ipversion=0;
			fc_db.netifTbl[swNetifIdx].dualUniInfo.psGreInfo.localIp=ntohl(pPktHdr->outer_iph->saddr);
			fc_db.netifTbl[swNetifIdx].dualUniInfo.psGreInfo.remoteIp=ntohl(pPktHdr->outer_iph->daddr);
		}
	}
	else if (dual_direc == RTK_FC_FLOW_DUAL_DIRECTION_DOWNSTREAM_IGR)
	{
		DEBUG("[Dual][PPTP]Need to add call id : %d to interface table.", pPktHdr->tunnelInfo.pptp.grecallid);

		if(fc_db.netifTbl[swNetifIdx].dualUniInfo.psGreInfo.call_id != pPktHdr->tunnelInfo.pptp.grecallid){
			TRACE("[PPTP]Find pptp device! netif, ready to set call id = %d", pPktHdr->tunnelInfo.pptp.grecallid);
			fc_db.netifTbl[swNetifIdx].dualUniInfo.psGreInfo.call_id = pPktHdr->tunnelInfo.pptp.grecallid;
		}
		fc_db.netifTbl[swNetifIdx].dualHdr_downstream_earlyChk = 1;
		fc_db.netifTbl[swNetifIdx].dualHdr_outer_L3_Offset = 0;  //only valid when FC_DUALHDR_HW_CTRL_PKT_BEGINIG
		if(pPktHdr->outer_ip6h)
		{
			fc_db.netifTbl[swNetifIdx].dualUniInfo.psGreInfo.outer_ipversion=1;
			memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.psGreInfo.localV6Ip,pPktHdr->ip6h->daddr.s6_addr,sizeof(struct in6_addr));
			memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.psGreInfo.remoteV6Ip,pPktHdr->ip6h->saddr.s6_addr,sizeof(struct in6_addr));

			//for early check support
			fc_db.netifTbl[swNetifIdx].dualHdr_outer_ipversion = 1;
			if(pPktHdr->iph)
				fc_db.netifTbl[swNetifIdx].dualHdr_outer_length = (void*)pPktHdr->iph - (void*)pPktHdr->outer_ip6h;
			else
				fc_db.netifTbl[swNetifIdx].dualHdr_outer_length = (void*)pPktHdr->ip6h - (void*)pPktHdr->outer_ip6h;
		}
		else if(pPktHdr->outer_iph)
		{
			fc_db.netifTbl[swNetifIdx].dualUniInfo.psGreInfo.outer_ipversion=0;
			fc_db.netifTbl[swNetifIdx].dualUniInfo.psGreInfo.localIp=ntohl(pPktHdr->outer_iph->daddr);
			fc_db.netifTbl[swNetifIdx].dualUniInfo.psGreInfo.remoteIp=ntohl(pPktHdr->outer_iph->saddr);
			//for early check support
			fc_db.netifTbl[swNetifIdx].dualHdr_outer_ipversion = 0;
			if(pPktHdr->iph)
				fc_db.netifTbl[swNetifIdx].dualHdr_outer_length = (void*)pPktHdr->iph - (void*)pPktHdr->outer_iph;
			else
				fc_db.netifTbl[swNetifIdx].dualHdr_outer_length = (void*)pPktHdr->ip6h - (void*)pPktHdr->outer_iph;
		}
	}	
	return SUCCESS;
}

/*
	upstream egr dual netif and downstream Igr dual netif setting
	WARNING: if(downstream Igr) the skb->fcIngressData is uninitialized
*/
int rtk_fc_netif_dual_set_L2TP(uint32 swNetifIdx, rtk_fc_flow_dual_direction_t dual_direc, struct sk_buff *skb, rtk_fc_pktHdr_t *pPktHdr)
{
	/*
	*  For L2TP, l2tp interface should record tunnel id and session id to identify the correct interface.
	*/
	if(dual_direc == RTK_FC_FLOW_DUAL_DIRECTION_UPSTREAM_EGR)
	{
		DEBUG("[Dual][L2TP][UP] Add peer tunnel id %d / session id %d to sw interface %d", pPktHdr->tunnelInfo.l2tp.tunnelid, pPktHdr->tunnelInfo.l2tp.sessionid, swNetifIdx);

		if(fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.peer_tunnel_id != pPktHdr->tunnelInfo.l2tp.tunnelid){
			TRACE("[L2TP][UP] Find l2tp device! swNetifIdx %d, ready to set peer tunnel id = %d", swNetifIdx, pPktHdr->tunnelInfo.l2tp.tunnelid);
			fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.peer_tunnel_id = pPktHdr->tunnelInfo.l2tp.tunnelid;
		}
		if(fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.peer_session_id != pPktHdr->tunnelInfo.l2tp.sessionid){
			TRACE("[L2TP][UP] Find l2tp device! swNetifIdx %d, ready to set peer session id = %d", swNetifIdx, pPktHdr->tunnelInfo.l2tp.sessionid);
			fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.peer_session_id = pPktHdr->tunnelInfo.l2tp.sessionid;
		}
		if(pPktHdr->outer_ip6h)
		{
			fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.outer_ipversion = 1;
			memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.localV6Ip, pPktHdr->ip6h->saddr.s6_addr, sizeof(struct in6_addr));
			memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.remoteV6Ip, pPktHdr->ip6h->daddr.s6_addr, sizeof(struct in6_addr));
		}
		else if(pPktHdr->outer_iph)
		{
			fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.outer_ipversion = 0;
			fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.localIp = ntohl(pPktHdr->outer_iph->saddr);
			fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.remoteIp = ntohl(pPktHdr->outer_iph->daddr);
		}
		if(pPktHdr->outer_udph)
		{
			fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.local_l4port = ntohs(pPktHdr->outer_udph->source);
			fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.remote_l4port = ntohs(pPktHdr->outer_udph->dest);
		}
	}
	else if (dual_direc == RTK_FC_FLOW_DUAL_DIRECTION_DOWNSTREAM_IGR)
	{
		DEBUG("[Dual][L2TP][DS] Add tunnel id %d / session id %d to sw interface %d", pPktHdr->tunnelInfo.l2tp.tunnelid, pPktHdr->tunnelInfo.l2tp.sessionid, swNetifIdx);

		if(fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.tunnel_id != pPktHdr->tunnelInfo.l2tp.tunnelid){
			TRACE("[L2TP][DS] Find l2tp device! swNetifIdx %d, ready to set tunnel id = %d", swNetifIdx, pPktHdr->tunnelInfo.l2tp.tunnelid);
			fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.tunnel_id = pPktHdr->tunnelInfo.l2tp.tunnelid;
		}
		if(fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.session_id != pPktHdr->tunnelInfo.l2tp.sessionid){
			TRACE("[L2TP][DS] Find l2tp device! swNetifIdx %d, ready to set session id = %d", swNetifIdx, pPktHdr->tunnelInfo.l2tp.sessionid);
			fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.session_id = pPktHdr->tunnelInfo.l2tp.sessionid;
		}
		fc_db.netifTbl[swNetifIdx].dualHdr_downstream_earlyChk = 1;
		fc_db.netifTbl[swNetifIdx].dualHdr_outer_L3_Offset = 0;  //only valid when FC_DUALHDR_HW_CTRL_PKT_BEGINIG
		if(pPktHdr->outer_ip6h)
		{
			fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.outer_ipversion = 1;
			memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.localV6Ip, pPktHdr->ip6h->daddr.s6_addr, sizeof(struct in6_addr));
			memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.remoteV6Ip, pPktHdr->ip6h->saddr.s6_addr, sizeof(struct in6_addr));
			//for early check support
			fc_db.netifTbl[swNetifIdx].dualHdr_outer_ipversion = 1;
			if(pPktHdr->iph)
				fc_db.netifTbl[swNetifIdx].dualHdr_outer_length = (void*)pPktHdr->iph - (void*)pPktHdr->outer_ip6h;
			else
				fc_db.netifTbl[swNetifIdx].dualHdr_outer_length = (void*)pPktHdr->ip6h - (void*)pPktHdr->outer_ip6h;
		}
		else if(pPktHdr->outer_iph)
		{
			fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.outer_ipversion = 0;
			fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.localIp = ntohl(pPktHdr->outer_iph->daddr);
			fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.remoteIp = ntohl(pPktHdr->outer_iph->saddr);
			//for early check support
			fc_db.netifTbl[swNetifIdx].dualHdr_outer_ipversion = 0;
			if(pPktHdr->iph)
				fc_db.netifTbl[swNetifIdx].dualHdr_outer_length = (void*)pPktHdr->iph - (void*)pPktHdr->outer_iph;
			else
				fc_db.netifTbl[swNetifIdx].dualHdr_outer_length = (void*)pPktHdr->ip6h - (void*)pPktHdr->outer_iph;
		}
		if(pPktHdr->outer_udph)
		{
			fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.local_l4port = ntohs(pPktHdr->outer_udph->dest);
			fc_db.netifTbl[swNetifIdx].dualUniInfo.l2tpInfo.remote_l4port = ntohs(pPktHdr->outer_udph->source);
		}
	}	
	else
		WARNING("[L2TP] dual_direct %d is unknown !!", dual_direc);

	return SUCCESS;
}


/*
	upstream egr dual netif and downstream Igr dual netif setting
	WARNING: if(downstream Igr) the skb->fcIngressData is uninitialized
*/
int rtk_fc_netif_dual_set_IPSEC(uint32 swNetifIdx,rtk_fc_flow_dual_direction_t dual_direc, struct sk_buff *skb, rtk_fc_pktHdr_t *pPktHdr)
{
	WARNING("please add RTK_FC_DUALTYPE_IPSEC  store dual-info by packet");
	if(dual_direc == RTK_FC_FLOW_DUAL_DIRECTION_UPSTREAM_EGR)
	{
		/* add by owner*/
	}
	else if (dual_direc == RTK_FC_FLOW_DUAL_DIRECTION_DOWNSTREAM_IGR)
	{
		/* add by owner*/
	}	
	return SUCCESS;
}


/*
	upstream egr dual netif and downstream Igr dual netif setting
	WARNING: if(downstream Igr) the skb->fcIngressData is uninitialized
*/
int rtk_fc_netif_dual_set_GRE_ETH_BR(uint32 swNetifIdx,rtk_fc_flow_dual_direction_t dual_direc, struct sk_buff *skb, rtk_fc_pktHdr_t *pPktHdr)
{
	fc_db.netifTbl[swNetifIdx].dualHdr_buf_insert_pos = FC_DUALHDR_HW_CTRL_PKT_BEGINIG;
	if(dual_direc == RTK_FC_FLOW_DUAL_DIRECTION_UPSTREAM_EGR)
	{
		if(pPktHdr->outer_iph)
		{
			fc_db.netifTbl[swNetifIdx].dualUniInfo.greEthBrInfo.outer_ipversion = 0;
			fc_db.netifTbl[swNetifIdx].dualUniInfo.greEthBrInfo.localIp = pPktHdr->outer_iph->saddr;
			fc_db.netifTbl[swNetifIdx].dualUniInfo.greEthBrInfo.remoteIp = pPktHdr->outer_iph->daddr;
		}
		else if(pPktHdr->outer_ip6h)
		{
			fc_db.netifTbl[swNetifIdx].dualUniInfo.greEthBrInfo.outer_ipversion = 1;
			memcpy(&(fc_db.netifTbl[swNetifIdx].dualUniInfo.greEthBrInfo.localV6Ip), pPktHdr->outer_ip6h->saddr.s6_addr, sizeof(struct in6_addr));
			memcpy(&(fc_db.netifTbl[swNetifIdx].dualUniInfo.greEthBrInfo.remoteV6Ip), pPktHdr->outer_ip6h->daddr.s6_addr, sizeof(struct in6_addr));
		}
		else
		{
			WARNING("pPktHdr->outer_iph and pPktHdr->outer_ip6h are both NULL!");
			return FAIL;
		}
	}
	else if (dual_direc == RTK_FC_FLOW_DUAL_DIRECTION_DOWNSTREAM_IGR)
	{
		if(pPktHdr->outer_iph)
		{
			fc_db.netifTbl[swNetifIdx].dualUniInfo.greEthBrInfo.outer_ipversion = 0;
			fc_db.netifTbl[swNetifIdx].dualUniInfo.greEthBrInfo.localIp = pPktHdr->outer_iph->daddr;
			fc_db.netifTbl[swNetifIdx].dualUniInfo.greEthBrInfo.remoteIp = pPktHdr->outer_iph->saddr;
		}
		else if(pPktHdr->outer_ip6h)
		{
			fc_db.netifTbl[swNetifIdx].dualUniInfo.greEthBrInfo.outer_ipversion = 1;
			memcpy(&(fc_db.netifTbl[swNetifIdx].dualUniInfo.greEthBrInfo.localV6Ip), pPktHdr->outer_ip6h->daddr.s6_addr, sizeof(struct in6_addr));
			memcpy(&(fc_db.netifTbl[swNetifIdx].dualUniInfo.greEthBrInfo.remoteV6Ip), pPktHdr->outer_ip6h->saddr.s6_addr, sizeof(struct in6_addr));
		}
		else
		{
			WARNING("pPktHdr->outer_iph and pPktHdr->outer_ip6h are both NULL!");
			return FAIL;
		}

		//for early check support
#if defined(CONFIG_FC_RTL9607F_SERIES)
		fc_db.netifTbl[swNetifIdx].dualHdr_downstream_earlyChk = 1;
		fc_db.netifTbl[swNetifIdx].dualHdr_outer_ipversion = fc_db.netifTbl[swNetifIdx].dualUniInfo.greEthBrInfo.outer_ipversion;
		fc_db.netifTbl[swNetifIdx].dualHdr_outer_L3_Offset = (pPktHdr->outer_iph)?((uint8*)pPktHdr->outer_iph-(uint8*)pPktHdr->outer_eth):((uint8*)pPktHdr->outer_ip6h-(uint8*)pPktHdr->outer_eth);  //only valid when FC_DUALHDR_HW_CTRL_PKT_BEGINIG
		fc_db.netifTbl[swNetifIdx].dualHdr_outer_length = pPktHdr->greEthBr_info.outerTag_off;
#else
		//need to recalculate inner hash by pktHdr
		fc_db.netifTbl[swNetifIdx].dualHdr_downstream_earlyChk = 0;
#endif
	}
	fc_db.netifTbl[swNetifIdx].dualUniInfo.greEthBrInfo.outerTag_len = pPktHdr->greEthBr_info.outerTag_off;

	return SUCCESS;
}

/*
	upstream egr dual netif and downstream Igr dual netif setting
	WARNING: if(downstream Igr) the skb->fcIngressData is uninitialized
*/
int rtk_fc_netif_dual_set_VXLAN(uint32 swNetifIdx,rtk_fc_flow_dual_direction_t dual_direc, struct sk_buff *skb, rtk_fc_pktHdr_t *pPktHdr)
{

	fc_db.netifTbl[swNetifIdx].dualHdr_buf_insert_pos = FC_DUALHDR_HW_CTRL_PKT_BEGINIG;
	DEBUG("RTK_FC_DUALTYPE_VXLAN setting dual-info by packet");
	if(dual_direc == RTK_FC_FLOW_DUAL_DIRECTION_UPSTREAM_EGR)
	{
		/* add by owner*/
		
		fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.valid			= 1;
		fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.vxlan_vni		= pPktHdr->vxlan_info.vni;		
		if(&pPktHdr->outer_eth)
			memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.vxlan_DA[0], &pPktHdr->outer_eth->h_source[0], 6);
		//DEBUG("vxlan DA = %pM",&fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.vxlan_DA[0]);
			
		if(pPktHdr->vxlan_info.outer_isV6)
		{
			memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_v6Ip, &pPktHdr->outer_ip6h->saddr, sizeof(struct in6_addr));
			memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_remote_v6Ip, &pPktHdr->outer_ip6h->daddr, sizeof(struct in6_addr));
		}
		else
		{
			fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_ip		= ntohl(pPktHdr->outer_iph->saddr);
			fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_remote_ip	= ntohl(pPktHdr->outer_iph->daddr);
		}
		
		fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_pppoe_tag		= (pPktHdr->outer_ppph)?1:0;
		if(pPktHdr->outer_ppph)
			fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_pppoe_sid		= ntohs(pPktHdr->outer_ppph->sid);
		fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_tag_len		= (pPktHdr->vxlan_info.outerTag_off);
		
		fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_ctag_if		= (pPktHdr->outer_cvh)?1:0;
		fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_ctag_pri		= (pPktHdr->outer_cvlanpri);
		fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_cvlan_id		= (pPktHdr->outer_cvlanid);
		
		fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_stag_if		= (pPktHdr->outer_svh)?1:0;
		fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_stag_pri		= (pPktHdr->outer_svlanpri);
		fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_svlan_id		= (pPktHdr->outer_svlanid);
		fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_is_v6			= pPktHdr->vxlan_info.outer_isV6;
		/*For special fast forward (07C/77B)usage*/
		fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_srcCVlanCfi			= (pPktHdr->outer_cvh)? ((ntohs(pPktHdr->outer_cvh->h_vlan_TCI)&VLAN_CFI_MASK)>>VLAN_CFI_SHIFT):0;
		fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_srcSVlanDei			= (pPktHdr->outer_svh)? ((ntohs(pPktHdr->outer_svh->h_vlan_TCI)&VLAN_CFI_MASK)>>VLAN_CFI_SHIFT):0;
		fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_srcSVlanTpid_sel		= pPktHdr->outer_stpid_sel;
			
	}
	else if (dual_direc == RTK_FC_FLOW_DUAL_DIRECTION_DOWNSTREAM_IGR)
	{
		/* add by owner*/
		if(fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.valid)
		{
			DEBUG("[VXLAN][DS]netifTbl[%d] vxlan netif info is valid! Set to packet header.",swNetifIdx);
			/*Add CLS related info*/
			pPktHdr->vxlan_info.outer_cvh		= fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_ctag_if;
			pPktHdr->vxlan_info.outer_cvlanid	= fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_cvlan_id;
			pPktHdr->vxlan_info.outer_svh		= fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_stag_if;	
			pPktHdr->vxlan_info.outer_svlanid	= fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_svlan_id;
			pPktHdr->vxlan_info.outer_isV6		= fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_is_v6;
			
			if(pPktHdr->vxlan_info.outer_isV6)
			{
				
					
				memcpy(&pPktHdr->vxlan_info.gateway_v6_ip, &fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_v6Ip, sizeof(struct in6_addr));
				memcpy(&pPktHdr->vxlan_info.remote_v6_ip, &fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_remote_v6Ip, sizeof(struct in6_addr));
			}
			else
			{
				pPktHdr->vxlan_info.gateway_v4_ip	=	fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_ip;
				pPktHdr->vxlan_info.remote_v4_ip	=	fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_remote_ip;
				
			}
			pPktHdr->vxlan_info.vni				= fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.vxlan_vni;
			memcpy(&pPktHdr->vxlan_info.vxlan_DA[0], &fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.vxlan_DA[0], 6);
		}
		else
		{
			TABLE("Netif [%d] is VXLAN!!", swNetifIdx);
			/* add by owner*/
		
			fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.valid			= 1;
			fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.vxlan_vni		= pPktHdr->vxlan_info.vni;	
			if(pPktHdr->outer_eth)
				memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.vxlan_DA[0], &pPktHdr->outer_eth->h_dest[0], 6);
			DEBUG("vxlan DA = %pM",&fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.vxlan_DA[0]);
			if(pPktHdr->vxlan_info.outer_isV6)
			{
				memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_v6Ip, &pPktHdr->outer_ip6h->daddr, sizeof(struct in6_addr));
				memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_remote_v6Ip, &pPktHdr->outer_ip6h->saddr, sizeof(struct in6_addr));
			}
			else
			{
				fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_ip		= ntohl(pPktHdr->outer_iph->daddr);
				fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_remote_ip	= ntohl(pPktHdr->outer_iph->saddr);
				DEBUG("outer IP = 0x%x, remote ip = 0x%x",fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_ip, fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_remote_ip);
			}
			
			fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_pppoe_tag		= (pPktHdr->outer_ppph)?1:0;
			if(pPktHdr->outer_ppph)
				fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_pppoe_sid		= ntohs(pPktHdr->outer_ppph->sid);
			fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_tag_len		= (pPktHdr->vxlan_info.outerTag_off);
			
			fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_ctag_if		= (pPktHdr->outer_cvh)?1:0;
			fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_ctag_pri		= (pPktHdr->outer_cvlanpri);
			fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_cvlan_id		= (pPktHdr->outer_cvlanid);
			
			fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_stag_if		= (pPktHdr->outer_svh)?1:0;
			fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_stag_pri		= (pPktHdr->outer_svlanpri);
			fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_svlan_id		= (pPktHdr->outer_svlanid);
			fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_is_v6			= pPktHdr->vxlan_info.outer_isV6;

			/*For special fast forward (07C/77B)usage*/
			fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_srcCVlanCfi			= (pPktHdr->outer_cvh)? ((ntohs(pPktHdr->outer_cvh->h_vlan_TCI)&VLAN_CFI_MASK)>>VLAN_CFI_SHIFT):0;
			fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_srcSVlanDei			= (pPktHdr->outer_svh)? ((ntohs(pPktHdr->outer_svh->h_vlan_TCI)&VLAN_CFI_MASK)>>VLAN_CFI_SHIFT):0;
			fc_db.netifTbl[swNetifIdx].dualUniInfo.vxlan_netif_info.outer_srcSVlanTpid_sel		= pPktHdr->outer_stpid_sel;
		
			//return FAILED;
		}
		
		//for early check support
		fc_db.netifTbl[swNetifIdx].dualHdr_downstream_earlyChk=1;
		if(pPktHdr->vxlan_info.outer_isV6)
			fc_db.netifTbl[swNetifIdx].dualHdr_outer_ipversion=1;
		else
			fc_db.netifTbl[swNetifIdx].dualHdr_outer_ipversion=0;
		fc_db.netifTbl[swNetifIdx].dualHdr_outer_L3_Offset= 14 +((pPktHdr->outer_ppph)? 8 : 0) + ((pPktHdr->outer_cvh)?4:0)+ ((pPktHdr->outer_svh)?4:0);  //only valid when FC_DUALHDR_HW_CTRL_PKT_BEGINIG
		fc_db.netifTbl[swNetifIdx].dualHdr_outer_length = (pPktHdr->vxlan_info.outerTag_off);	
	}

	
	return SUCCESS;
}


/*
	upstream egr dual netif and downstream Igr dual netif setting
	WARNING: if(downstream Igr) the skb->fcIngressData is uninitialized
*/
int rtk_fc_netif_dual_set_6RD(uint32 swNetifIdx,rtk_fc_flow_dual_direction_t dual_direc, struct sk_buff *skb, rtk_fc_pktHdr_t *pPktHdr)
{
	//WARNING("please add RTK_FC_DUALTYPE_6RD  store dual-info by packet");
	if(pPktHdr->outer_iph)
		TRACE("Store dual-info by %s packet ([sip:%pI4] [dip:%pI4]).\n",(dual_direc == RTK_FC_FLOW_DUAL_DIRECTION_UPSTREAM_EGR)?"upstream":"downstream",&pPktHdr->outer_iph->saddr,&pPktHdr->outer_iph->daddr);
	
	if(dual_direc == RTK_FC_FLOW_DUAL_DIRECTION_UPSTREAM_EGR)
	{
		if(pPktHdr->outer_iph)
		{
			fc_db.netifTbl[swNetifIdx].dualUniInfo.ip6Inip4_info.localAddr.s_addr= ntohl(pPktHdr->outer_iph->saddr);
			fc_db.netifTbl[swNetifIdx].dualUniInfo.ip6Inip4_info.remoteAddr.s_addr= ntohl(pPktHdr->outer_iph->daddr);
		}
	}
	else if (dual_direc == RTK_FC_FLOW_DUAL_DIRECTION_DOWNSTREAM_IGR)
	{
		if(pPktHdr->outer_iph)
		{
			fc_db.netifTbl[swNetifIdx].dualUniInfo.ip6Inip4_info.localAddr.s_addr = ntohl(pPktHdr->outer_iph->daddr);
			fc_db.netifTbl[swNetifIdx].dualUniInfo.ip6Inip4_info.remoteAddr.s_addr = ntohl(pPktHdr->outer_iph->saddr);
		}

		#if defined(CONFIG_RTK_L34_G3_PLATFORM)
		{
			//for early check support
			fc_db.netifTbl[swNetifIdx].dualHdr_downstream_earlyChk=1;
			fc_db.netifTbl[swNetifIdx].dualHdr_outer_ipversion=0; //0:v4 1:v6
			fc_db.netifTbl[swNetifIdx].dualHdr_outer_L3_Offset=0;  //only valid when FC_DUALHDR_HW_CTRL_PKT_BEGINIG
			fc_db.netifTbl[swNetifIdx].dualHdr_outer_length = (void*)pPktHdr->ip6h -(void*)pPktHdr->outer_iph;
		}
		#endif
	}

	return SUCCESS;
}


/*
	upstream egr dual netif and downstream Igr dual netif setting
	WARNING: if(downstream Igr) the skb->fcIngressData is uninitialized
*/
int rtk_fc_netif_dual_set_MAPT(uint32 swNetifIdx,rtk_fc_flow_dual_direction_t dual_direc, struct sk_buff *skb, rtk_fc_pktHdr_t *pPktHdr)
{
	if(dual_direc == RTK_FC_FLOW_DUAL_DIRECTION_UPSTREAM_EGR)
	{
		if(pPktHdr->ip6h)
		{
			memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.maptInfo.localV6Ip,pPktHdr->ip6h->saddr.s6_addr,sizeof(struct in6_addr));
			memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.maptInfo.remoteV6Ip,pPktHdr->ip6h->daddr.s6_addr,sizeof(struct in6_addr));
			DEBUG("upstream!! localV6Ip as %pI6, remoteV6Ip as %pI6",&fc_db.netifTbl[swNetifIdx].dualUniInfo.maptInfo.localV6Ip,&fc_db.netifTbl[swNetifIdx].dualUniInfo.maptInfo.remoteV6Ip);
		}
	}
	else if (dual_direc == RTK_FC_FLOW_DUAL_DIRECTION_DOWNSTREAM_IGR)
	{
		if(pPktHdr->ip6h)
		{
			memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.maptInfo.localV6Ip,pPktHdr->ip6h->daddr.s6_addr,sizeof(struct in6_addr));
			memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.maptInfo.remoteV6Ip,pPktHdr->ip6h->saddr.s6_addr,sizeof(struct in6_addr));
			DEBUG("downstream!! localV6Ip as %pI6, remoteV6Ip as %pI6",&fc_db.netifTbl[swNetifIdx].dualUniInfo.maptInfo.localV6Ip,&fc_db.netifTbl[swNetifIdx].dualUniInfo.maptInfo.remoteV6Ip);
		}

		//for early check support
		fc_db.netifTbl[swNetifIdx].dualHdr_downstream_earlyChk = 1;
		fc_db.netifTbl[swNetifIdx].dualHdr_outer_L3_Offset = 0;  //only valid when FC_DUALHDR_HW_CTRL_PKT_BEGINIG
		fc_db.netifTbl[swNetifIdx].dualHdr_outer_ipversion = 1;
		fc_db.netifTbl[swNetifIdx].dualHdr_outer_length = pPktHdr->l4Offset - pPktHdr->l3Offset;
	}
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	{
		int i,j;
		struct net_device *p_PScacheDev[DEV_STACK_MAX]={NULL,NULL,NULL,NULL};

		rtk_fc_psIfidxStackKey_to_dev(fc_db.netifTbl[swNetifIdx].psIfidxStackKey,p_PScacheDev);
		for(i=0;i<DEV_STACK_MAX;i++)
		{
			if(p_PScacheDev[i])
			{
				for(j=DEVIFIDX_VALID_MIN;j<RTK_FC_DEV_GW_MAC_TBL;j++)
				{
					if(!fc_db.devGwMacTbl[j].dev)
						continue;
					
					if(fc_db.devGwMacTbl[j].dev == p_PScacheDev[i] && fc_db.devGwMacTbl[j].maptInfoValid)
					{
						fc_db.netifTbl[swNetifIdx].dualUniInfo.maptInfo.hwDraftVer=fc_db.devGwMacTbl[j].maptDraftVer;
						fc_db.netifTbl[swNetifIdx].dualUniInfo.maptInfo.hwPrefxLen=fc_db.devGwMacTbl[j].maptPrefxLen;
						DEBUG("hwDraftVer=%d, hwPrefxLen=%d",fc_db.netifTbl[swNetifIdx].dualUniInfo.maptInfo.hwDraftVer,fc_db.netifTbl[swNetifIdx].dualUniInfo.maptInfo.hwPrefxLen);
						i=DEV_STACK_MAX;
						break;
					}
					if(fc_db.devGwMacTbl[j].dev == p_PScacheDev[i] && fc_db.devGwMacTbl[j].mapet_fmr_hwidx_valid)
					{
						fc_db.netifTbl[swNetifIdx].hwFmrIdx =  fc_db.devGwMacTbl[j].mapet_fmr_hwidx;

					}
				}
			}
		}
	}
#endif
	return SUCCESS;
}
/*
	upstream egr dual netif and downstream Igr dual netif setting
	WARNING: if(downstream Igr) the skb->fcIngressData is uninitialized
*/
int rtk_fc_netif_dual_set_XLAT(uint32 swNetifIdx,rtk_fc_flow_dual_direction_t dual_direc, struct sk_buff *skb, rtk_fc_pktHdr_t *pPktHdr)
{

	if(dual_direc == RTK_FC_FLOW_DUAL_DIRECTION_UPSTREAM_EGR)
	{
		if(pPktHdr->ip6h)
		{
			memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.xlatInfo.Ipv6Addr_local.s6_addr,pPktHdr->ip6h->saddr.s6_addr,sizeof(struct in6_addr));
			memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.xlatInfo.Ipv6Addr_remote.s6_addr,pPktHdr->ip6h->daddr.s6_addr,sizeof(struct in6_addr));
			rtk_fc_dualHash_XLAT_ipMasked((uint8 *)&fc_db.netifTbl[swNetifIdx].dualUniInfo.xlatInfo.Ipv6Addr_local.s6_addr[0],fc_db.dualIpv6HashMask& (~DUAL_IPV6_HASHMASK_ALL));
			rtk_fc_dualHash_XLAT_ipMasked((uint8 *)&fc_db.netifTbl[swNetifIdx].dualUniInfo.xlatInfo.Ipv6Addr_remote.s6_addr[0],fc_db.dualIpv6HashMask& (~DUAL_IPV6_HASHMASK_ALL));
			DEBUG("upstream!! Masked localV6Ip as %pI6, remoteV6Ip as %pI6",&fc_db.netifTbl[swNetifIdx].dualUniInfo.xlatInfo.Ipv6Addr_local.s6_addr,&fc_db.netifTbl[swNetifIdx].dualUniInfo.xlatInfo.Ipv6Addr_remote.s6_addr);
		}
	}
	else if (dual_direc == RTK_FC_FLOW_DUAL_DIRECTION_DOWNSTREAM_IGR)
	{
		if(pPktHdr->ip6h)
		{
			memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.xlatInfo.Ipv6Addr_local.s6_addr,pPktHdr->ip6h->daddr.s6_addr,sizeof(struct in6_addr));
			memcpy(&fc_db.netifTbl[swNetifIdx].dualUniInfo.xlatInfo.Ipv6Addr_remote.s6_addr,pPktHdr->ip6h->saddr.s6_addr,sizeof(struct in6_addr));
			rtk_fc_dualHash_XLAT_ipMasked((uint8 *)&fc_db.netifTbl[swNetifIdx].dualUniInfo.xlatInfo.Ipv6Addr_local.s6_addr,fc_db.dualIpv6HashMask& (~DUAL_IPV6_HASHMASK_ALL));
			rtk_fc_dualHash_XLAT_ipMasked((uint8 *)&fc_db.netifTbl[swNetifIdx].dualUniInfo.xlatInfo.Ipv6Addr_remote.s6_addr,fc_db.dualIpv6HashMask& (~DUAL_IPV6_HASHMASK_ALL));			
			DEBUG("downstream!! Masked localV6Ip as %pI6, remoteV6Ip as %pI6",&fc_db.netifTbl[swNetifIdx].dualUniInfo.xlatInfo.Ipv6Addr_local.s6_addr,&fc_db.netifTbl[swNetifIdx].dualUniInfo.xlatInfo.Ipv6Addr_remote.s6_addr);
		}
#if defined(CONFIG_FC_RTL9607F_SERIES)
		//for early check support
		fc_db.netifTbl[swNetifIdx].dualHdr_downstream_earlyChk = 1;
		fc_db.netifTbl[swNetifIdx].dualHdr_outer_L3_Offset = 0;  //only valid when FC_DUALHDR_HW_CTRL_PKT_BEGINIG
		fc_db.netifTbl[swNetifIdx].dualHdr_outer_ipversion = 1;
		fc_db.netifTbl[swNetifIdx].dualHdr_outer_length = pPktHdr->l4Offset - pPktHdr->l3Offset;
#endif		
	}
	fc_db.netifTbl[swNetifIdx].dualUniInfo.xlatInfo.Ipv6Addr_localPreifx = fc_db.dualIpv6HashMask&(~DUAL_IPV6_HASHMASK_ALL);
	fc_db.netifTbl[swNetifIdx].dualUniInfo.xlatInfo.Ipv6Addr_remotePreifx = fc_db.dualIpv6HashMask&(~DUAL_IPV6_HASHMASK_ALL);


	return SUCCESS;
}



void rtk_fc_netif_dualinfo_setting(rtk_fc_dev_type_t devType,rtk_fc_flow_direction_t dir ,uint32 swNetifIdx, struct sk_buff *skb, rtk_fc_pktHdr_t *pPktHdr,int devStackingCnt,struct net_device**  devArray)
{
	uint8 hwEntryNum = 1;
	rtk_fc_dualHdrtype_t dualType = RTK_FC_DUALTYPE_NONE;
	rtk_fc_ingress_data_t *pFcIngressData = &skb->fcIngressData;

	if(devType==RTK_FC_DEV_TYPE_INGRESS)
	{
		if(pFcIngressData->isDualHeader && pPktHdr->dualHdrType==RTK_FC_DUALTYPE_NONE)	//downstream dual
		{
			if(pFcIngressData->ingressTagif & IP4_IN_IP6)
				dualType = RTK_FC_DUALTYPE_IP4INIP6;
			else if(pFcIngressData->ingressTagif & PPTP_TAGIF)
				dualType = RTK_FC_DUALTYPE_PPTP;
			else if(pFcIngressData->ingressTagif & L2TP_TAGIF)
				dualType = RTK_FC_DUALTYPE_L2TP;
			else if(pFcIngressData->ingressTagif & IPSEC_ESP_TAGIF)
				dualType = RTK_FC_DUALTYPE_IPSEC;
			else if(pFcIngressData->ingressTagif & GRE_ETH_BR_TAGIF)
			{
				dualType = RTK_FC_DUALTYPE_GRE_ETH_BR;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
				if(fc_db.netifTbl[swNetifIdx].dualUniInfo.greEthBrInfo.outerTag_len == 0)
					WARNING("[GRE_ETH_BR] Invalid outerTag_len(0)");
				hwEntryNum = (fc_db.netifTbl[swNetifIdx].dualUniInfo.greEthBrInfo.outerTag_len >= RTK_FC_DUAL_CONTENT_BUFFER_SIZE) ? 2 : 1 ;
#endif
			}
			else if(pFcIngressData->ingressTagif & VXLAN_TAGIF)
			{
				dualType = RTK_FC_DUALTYPE_VXLAN;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)				
				hwEntryNum = (pFcIngressData->vxlan_info.outer_tag_len >= RTK_FC_DUAL_CONTENT_BUFFER_SIZE) ? 2 : 1 ;
#endif
			}
			else if(pFcIngressData->ingressTagif & V6RD_TAGIF)
				dualType = RTK_FC_DUALTYPE_6RD;
			else
				dualType = RTK_FC_DUALTYPE_NONE;

		}
		else if(pPktHdr->isMAPT && pPktHdr->iph)
		{
			dualType = RTK_FC_DUALTYPE_MAPT;
		}
		else if (pPktHdr->isXLAT && pPktHdr->iph)
		{
			dualType = RTK_FC_DUALTYPE_XLAT464;
		}
	}
	else if(devType==RTK_FC_DEV_TYPE_EGRESS)
	{
		if(pFcIngressData->isDualHeader==0 && pPktHdr->dualHdrType) //upstream dual
		{
			dualType = pPktHdr->dualHdrType;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
			if(dualType == RTK_FC_DUALTYPE_VXLAN)
				hwEntryNum = (pPktHdr->vxlan_info.outerTag_off >= RTK_FC_DUAL_CONTENT_BUFFER_SIZE) ? 2 : 1 ;
			else if(dualType == RTK_FC_DUALTYPE_GRE_ETH_BR)
				hwEntryNum = (pPktHdr->greEthBr_info.outerTag_off >= RTK_FC_DUAL_CONTENT_BUFFER_SIZE) ? 2 : 1 ;
#endif
		}
		else if(pPktHdr->isMAPT && pPktHdr->ip6h)
		{
			dualType = RTK_FC_DUALTYPE_MAPT;
		}
		else if(pPktHdr->isXLAT && pPktHdr->ip6h)
		{
			dualType = RTK_FC_DUALTYPE_XLAT464;
		}
	}

	fc_db.netifTbl[swNetifIdx].dualType = dualType;
	fc_db.netifTbl[swNetifIdx].hwEntryNum = hwEntryNum;

	if( dir==RTK_FC_FLOW_DIRECTION_UPSTREAM && (devType==RTK_FC_DEV_TYPE_EGRESS) && fc_db.netifTbl[swNetifIdx].dualType!=RTK_FC_DUALTYPE_NONE)
	{
		/* 
			1.(by dual_netifInfo_collect )in upstream egress we have all dual-info, please update to netif and udpate netif dual-info by dev->netdev_priv 
			2.save dualHashKey
		*/
		if(fc_db.dualCallBackFunc[dualType].dual_netifInfo_collect)
			fc_db.dualCallBackFunc[dualType].dual_netifInfo_collect(swNetifIdx,RTK_FC_FLOW_DUAL_DIRECTION_UPSTREAM_EGR,skb,pPktHdr);

		fc_db.netifTbl[swNetifIdx].dualHashKey = pPktHdr->dualHdrHashKey;
		fc_db.netifTbl[swNetifIdx].dualUniInfo_syncedUP=1;
	}
	else if( dir==RTK_FC_FLOW_DIRECTION_DOWNSTREAM 	&& (devType==RTK_FC_DEV_TYPE_INGRESS) && fc_db.netifTbl[swNetifIdx].dualType!=RTK_FC_DUALTYPE_NONE)
	{
		/* 
			just need save dualHashKey to netif
		*/
		if(skb->fcIngressData.dualPattenHashKey)
			fc_db.netifTbl[swNetifIdx].dualHashKey=skb->fcIngressData.dualPattenHashKey;

		//special treat!!
		if(dualType==RTK_FC_DUALTYPE_MAPT)
		{
			fc_db.netifTbl[swNetifIdx].dualHashKey&=DUAL_MAPT_HASH_MASK;
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
			fc_db.netifTbl[swNetifIdx].dualUniInfo_syncedDOWN=1;
#endif
		}
		else if(dualType==RTK_FC_DUALTYPE_XLAT464)
		{
			fc_db.netifTbl[swNetifIdx].dualHashKey&=DUAL_XLAT_HASH_MASK;
		}
	}


	if(dualType == RTK_FC_DUALTYPE_NONE){
		fc_db.netifTbl[swNetifIdx].dualUniInfo_syncedUP=1;
		fc_db.netifTbl[swNetifIdx].dualUniInfo_syncedDOWN=1;
	}
	{
		extern	char *name_of_dualHdrType[];
		TRACE("update %s netif[%d] to dual-type:[%s] dualHdrHashKey:%x",(devType==RTK_FC_DEV_TYPE_INGRESS)?"Igr":"Egr",swNetifIdx,name_of_dualHdrType[dualType],pPktHdr->dualHdrHashKey);
	}
	return;
}


//for ingress flow learning slow path prepare,update dual-info by match dualHashKey
//if hit return netifHitIdx else return FAIL
int rtk_fc_netif_update_dualinfo_by_dualHashKey(rtk_fc_pktHdr_t *pPktHdr,struct sk_buff *skb)
{
	int i;
	int netifHitIdx=FAIL;

	if(pPktHdr->dualHdrHashKey == 0)
		return netifHitIdx;

	TRACE("Search dualHashKey:%x netif",pPktHdr->dualHdrHashKey);

	for(i=0;i<RTK_FC_TABLESIZE_INTF_SW;i++)
	{
		if(!fc_db.netifTbl[i].intf.valid )
			continue;

		if( (fc_db.netifTbl[i].dualType==pPktHdr->dualHdrType && fc_db.netifTbl[i].dualHashKey==pPktHdr->dualHdrHashKey)||
			(fc_db.netifTbl[i].dualType==RTK_FC_DUALTYPE_MAPT && fc_db.netifTbl[i].dualHashKey==(pPktHdr->dualHdrHashKey&DUAL_MAPT_HASH_MASK)) ||
			(fc_db.netifTbl[i].dualType==RTK_FC_DUALTYPE_XLAT464 && fc_db.netifTbl[i].dualHashKey==(pPktHdr->dualHdrHashKey&DUAL_XLAT_HASH_MASK)))
		{
			netifHitIdx=i;
			break;
		}
	}
	//not find match netif 
	if(netifHitIdx==FAIL){
		DEBUG("Didn't Find match netif by dualHdrHash key %x",pPktHdr->dualHdrHashKey);
		return netifHitIdx;
	}
	if(fc_db.netifTbl[netifHitIdx].dualUniInfo_syncedDOWN){
		DEBUG("netif[%d] downstream info has been synced.",netifHitIdx);
		return netifHitIdx;
	}

	TABLE("downstream hash match ingress swNetif[%d] update dual-info(%d)",netifHitIdx,fc_db.netifTbl[netifHitIdx].dualType);
/*
	downstream Igr dual netif setting
	WARNING:the skb->fcIngressData is uninitialized
*/
	if(fc_db.dualCallBackFunc[fc_db.netifTbl[netifHitIdx].dualType].dual_netifInfo_collect)
		fc_db.dualCallBackFunc[fc_db.netifTbl[netifHitIdx].dualType].dual_netifInfo_collect(netifHitIdx,RTK_FC_FLOW_DUAL_DIRECTION_DOWNSTREAM_IGR,skb,pPktHdr);

	fc_db.netifTbl[netifHitIdx].dualUniInfo_syncedDOWN=1;

	return netifHitIdx;
}
