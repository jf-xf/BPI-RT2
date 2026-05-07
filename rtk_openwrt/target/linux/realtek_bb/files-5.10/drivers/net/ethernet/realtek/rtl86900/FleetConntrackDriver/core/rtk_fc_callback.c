/*
 * Copyright (C) 2016 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*/

#include <linux/if_vlan.h>
#include <linux/if_pppox.h>

#include <linux/netfilter/nf_conntrack_tuple_common.h>
#include <linux/etherdevice.h>
#include <linux/inetdevice.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/neighbour.h>
#include <net/addrconf.h>
#include "../../../../net/bridge/br_private.h"
#include <common/rt_error.h>
#include <rtk/switch.h>
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
#include <rtk/classify.h>
#endif

#include "rtk_fc_driver.h"
#include "rtk_fc_callback.h"
#include "rtk_fc_internal.h"
#include "rtk_fc_debug.h"
#include "rtk_fc_dualHeader.h"
#include "rtk_fc_mappingAPI.h"
#include "rtk_fc_external.h"
#include "rtk_fc_acl.h"
#include "rtk_fc_helper.h"



#if defined(CONFIG_RTK_L34_XPON_PLATFORM) || defined(CONFIG_RTK_L34_G3_PLATFORM)
extern uint32 ASICDRIVERVER;
static int _rtl9607c_fc_global_state_init(void)
{
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	rtk_l34_hsdState_set(ENABLED);
	rtk_rg_asic_l2UcAct_set(FB_L2UCACT_FB);

#if defined(CONFIG_FC_RTL9607C_SERIES) && !defined(CONFIG_RTK_SOC_RTL8198D)
	// to carry pon sid to L34 module
	rtk_classify_cfSel_set(RTK_FC_MAC_PORT_PON, CLASSIFY_CF_SEL_ENABLE);
#endif

#if defined(CONFIG_FC_RTL9607C_RTL9603CVD_HYBRID)

	if(CHIP_ID_9607C) {
#if defined(CONFIG_SWITCH_SYS_CLOCK_TUNE)
		if(ASICDRIVERVER!=0x1){
			rtk_rg_asic_shareMeterGlobalConfig_set(0x16U, 0x3cU);
		}else
#endif
		{
			rtk_rg_asic_shareMeterGlobalConfig_set(0x57U, 0xbdU);
		}
	}else {
		rtk_rg_asic_shareMeterGlobalConfig_set(0x2bU, 0xbdU);
	}

#else

#if defined(CONFIG_SWITCH_SYS_CLOCK_TUNE)
	if(ASICDRIVERVER!=0x1){
		rtk_rg_asic_shareMeterGlobalConfig_set(0x16U, 0x3cU);
	}else
#endif
	{
		rtk_rg_asic_shareMeterGlobalConfig_set(0x57U, 0xbdU);
	}

#endif

	_rtk_rg_fb_hw_init();

	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_TTL_1, ENABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_TRAP_TCP_SYN_FIN_REST, ENABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_TRAP_TCP_SYN_ACK, ENABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_TRAP_FRAGMENT, ENABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_L3_CS_CHK, ENABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_L4_CS_CHK, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_L2_FLOW_LOOKUP_BY_MAC, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_CMP_TOS, ENABLED);

	// skip all optional hash pattern
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH12_SKIP_SVID, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH34_UCBC_SKIP_SVID, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH34_MC_SKIP_SVID, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH34_SKIP_DA, ENABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH5_SKIP_SVID, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH6_SKIP_SVID, DISABLED);

	//default add cvid to hash key
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH12_SKIP_CVID, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH34_UCBC_SKIP_CVID, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH34_MC_SKIP_CVID, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH5_SKIP_CVID, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH6_SKIP_CVID, DISABLED);


#if defined(CONFIG_FC_RTL9607C_SERIES) || defined(CONFIG_FC_RTL9607C_RTL9603CVD_HYBRID)
	if(ASICDRIVERVER!=0x1U)
#endif
	{
		RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH12_SKIP_CPRI, ENABLED);
		RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH34_UCBC_SKIP_CPRI, ENABLED);
		RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH34_MC_SKIP_CPRI, ENABLED);
		RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH5_SKIP_CPRI, ENABLED);
		RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH6_SKIP_CPRI, ENABLED);
		RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH6_SKIP_SA, ENABLED);
		RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH6_SKIP_DA, ENABLED);
#if defined(CONFIG_RTK_SOC_RTL8198D)
		RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATHALL_SKIP_DSCP, ENABLED);
#else
		RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATHALL_SKIP_DSCP, DISABLED);
#endif
	}

	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATHALL_SKIP_SPRI, ENABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATHALL_SKIP_VLAN_DEICFI, ENABLED);

	//init preHashPtn
	RTK_RG_ASIC_PREHASHPTN_SET(FB_PREHASH_PTN_SPORT,  0xB0000U);
#if defined(CONFIG_RTK_SOC_RTL8198D)
	RTK_RG_ASIC_PREHASHPTN_SET(FB_PREHASH_PTN_DPORT, 0x90000U);
#else
	RTK_RG_ASIC_PREHASHPTN_SET(FB_PREHASH_PTN_DPORT, 0x40000U);
#endif
	RTK_RG_ASIC_PREHASHPTN_SET(FB_PREHASH_PTN_SIP, 0xE00000U);
	RTK_RG_ASIC_PREHASHPTN_SET(FB_PREHASH_PTN_DIP, 0xF00000U);

	//set uc lookup mode
	//_rtk_rg_lut_ucLookupAction_init();

	//init flow ingress check
	//check tos
#if defined(CONFIG_RTK_SOC_RTL8198D)
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_TOS] = DISABLED; // this should follow FB_GLOBAL_PATHALL_SKIP_DSCP setting
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH34_TOS] = DISABLED; // this should follow FB_GLOBAL_PATHALL_SKIP_DSCP setting
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH5_TOS] = DISABLED; // this should follow FB_GLOBAL_PATHALL_SKIP_DSCP setting
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_TOS] = DISABLED; // this should follow FB_GLOBAL_PATHALL_SKIP_DSCP setting
#else
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_TOS] = ENABLED; // this should follow FB_GLOBAL_PATHALL_SKIP_DSCP setting
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH34_TOS] = ENABLED; // this should follow FB_GLOBAL_PATHALL_SKIP_DSCP setting
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH5_TOS] = ENABLED; // this should follow FB_GLOBAL_PATHALL_SKIP_DSCP setting
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_TOS] = ENABLED; // this should follow FB_GLOBAL_PATHALL_SKIP_DSCP setting
#endif
	//check protocol
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_PROTOCOL] = DISABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_PROTOCOL] = DISABLED;
	//check spa
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_SPA] = ENABLED;
	//check stream idx
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_STREAM_IDX] = ENABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH345_STREAM_IDX] = ENABLED; // support SW only
	//check path6
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_SMAC_IDX] = DISABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_DMAC_IDX] = ENABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_SIP] = ENABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_DIP] = ENABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_SPORT] = DISABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_DPORT] = DISABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_GRE_CALL_ID] = ENABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_L2TP_TUNNEL_ID] = ENABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_L2TP_SID] = ENABLED;

#elif defined(CONFIG_RTK_L34_G3_PLATFORM)

	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_CMP_TOS, ENABLED);

	// skip all optional hash pattern
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH12_SKIP_SVID, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH12_SKIP_CVID, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH34_UCBC_SKIP_SVID, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH34_UCBC_SKIP_CVID, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH34_MC_SKIP_SVID, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH34_MC_SKIP_CVID, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH34_SKIP_DA, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH5_SKIP_SVID, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH5_SKIP_CVID, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH6_SKIP_SVID, ENABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH6_SKIP_CVID, ENABLED);

	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH12_SKIP_CPRI, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH34_UCBC_SKIP_CPRI, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH34_MC_SKIP_CPRI, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH5_SKIP_CPRI, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH6_SKIP_CPRI, ENABLED);	// dual header: no need to care cpri
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH6_SKIP_SA, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATH6_SKIP_DA, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATHALL_SKIP_DSCP, DISABLED);

	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATHALL_SKIP_SPRI, DISABLED);
	RTK_RG_ASIC_GLOBALSTATE_SET(FB_GLOBAL_PATHALL_SKIP_VLAN_DEICFI, ENABLED);

	//init preHashPtn
	RTK_RG_ASIC_PREHASHPTN_SET(FB_PREHASH_PTN_SPORT,  0xB0000);
	RTK_RG_ASIC_PREHASHPTN_SET(FB_PREHASH_PTN_DPORT, 0x40000);
	RTK_RG_ASIC_PREHASHPTN_SET(FB_PREHASH_PTN_SIP, 0xF00000);
	RTK_RG_ASIC_PREHASHPTN_SET(FB_PREHASH_PTN_DIP, 0xF00000);

	//init flow ingress check
	//check tos
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_TOS] = ENABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH34_TOS] = ENABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH5_TOS] = ENABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_TOS] = DISABLED;
	//check protocol
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_PROTOCOL] = DISABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_PROTOCOL] = DISABLED;
	//check spa
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_SPA] = ENABLED;
	//check stream idx
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_STREAM_IDX] = ENABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH345_STREAM_IDX] = ENABLED; // support HW and SW
	//check path6
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_SMAC_IDX] = ENABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_DMAC_IDX] = ENABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_SIP] = ENABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_DIP] = ENABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_SPORT] = DISABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_DPORT] = DISABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_GRE_CALL_ID] = ENABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_L2TP_TUNNEL_ID] = ENABLED;
	fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH6_L2TP_SID] = ENABLED;
#endif

	return 0;
}

static int _rtl9607c_fc_port_init(void)
{
	int i=0, ret=0;
	rtk_enable_t enable;

	//get phyPort status and store in software
	for(i=RTK_PORT_UTP0;i<RTK_PORT_UTP11;i++)
	{
		if(rtk_switch_phyPortId_get(i, &ret)==RT_ERR_OK)fc_db.systemGlobal.phyPortStatus|=0x1ULL<<(RTK_FC_MAC_PORT0+ret);
	}

	if(rtk_switch_phyPortId_get(RTK_PORT_PON, &ret)==RT_ERR_OK)fc_db.systemGlobal.phyPortStatus|=0x1ULL<<(RTK_FC_MAC_PORT0+ret);
	if(rtk_switch_phyPortId_get(RTK_PORT_FIBER, &ret)==RT_ERR_OK)fc_db.systemGlobal.phyPortStatus|=0x1ULL<<(RTK_FC_MAC_PORT0+ret);
	if(rtk_switch_phyPortId_get(RTK_PORT_EXT0, &ret)==RT_ERR_OK)fc_db.systemGlobal.phyPortStatus|=0x1ULL<<(RTK_FC_MAC_PORT0+ret);
	if(rtk_switch_phyPortId_get(RTK_PORT_EXT1, &ret)==RT_ERR_OK)fc_db.systemGlobal.phyPortStatus|=0x1ULL<<(RTK_FC_MAC_PORT0+ret);
	if(rtk_switch_phyPortId_get(RTK_PORT_EXT2, &ret)==RT_ERR_OK)fc_db.systemGlobal.phyPortStatus|=0x1ULL<<(RTK_FC_MAC_PORT0+ret);
	if(rtk_switch_phyPortId_get(RTK_PORT_HSG0, &ret)==RT_ERR_OK)fc_db.systemGlobal.phyPortStatus|=0x1ULL<<(RTK_FC_MAC_PORT0+ret);
	if(rtk_switch_phyPortId_get(RTK_PORT_HSG1, &ret)==RT_ERR_OK)fc_db.systemGlobal.phyPortStatus|=0x1ULL<<(RTK_FC_MAC_PORT0+ret);


	if(rtk_switch_phyPortId_get(RTK_PORT_CPU, &ret)==RT_ERR_OK)fc_db.systemGlobal.phyPortStatus|=0x1ULL<<(RTK_FC_MAC_PORT0+ret);
#if defined(CONFIG_FC_RTL9607C_SERIES)
	if(rtk_switch_phyPortId_get(RTK_PORT_CPU1, &ret)==RT_ERR_OK)fc_db.systemGlobal.phyPortStatus|=0x1ULL<<(RTK_FC_MAC_PORT0+ret);
	if(rtk_switch_phyPortId_get(RTK_PORT_CPU2, &ret)==RT_ERR_OK)fc_db.systemGlobal.phyPortStatus|=0x1ULL<<(RTK_FC_MAC_PORT0+ret);
#endif	
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	if(rtk_switch_phyPortId_get(RTK_PORT_CPU1, &ret)==RT_ERR_OK)fc_db.systemGlobal.phyPortStatus|=0x1ULL<<(RTK_FC_MAC_PORT0+ret);
	if(rtk_switch_phyPortId_get(RTK_PORT_CPU2, &ret)==RT_ERR_OK)fc_db.systemGlobal.phyPortStatus|=0x1ULL<<(RTK_FC_MAC_PORT0+ret);
	if(rtk_switch_phyPortId_get(RTK_PORT_CPU3, &ret)==RT_ERR_OK)fc_db.systemGlobal.phyPortStatus|=0x1ULL<<(RTK_FC_MAC_PORT0+ret);
	if(rtk_switch_phyPortId_get(RTK_PORT_CPU4, &ret)==RT_ERR_OK)fc_db.systemGlobal.phyPortStatus|=0x1ULL<<(RTK_FC_MAC_PORT0+ret);
	if(rtk_switch_phyPortId_get(RTK_PORT_CPU5, &ret)==RT_ERR_OK)fc_db.systemGlobal.phyPortStatus|=0x1ULL<<(RTK_FC_MAC_PORT0+ret);
	if(rtk_switch_phyPortId_get(RTK_PORT_CPU6, &ret)==RT_ERR_OK)fc_db.systemGlobal.phyPortStatus|=0x1ULL<<(RTK_FC_MAC_PORT0+ret);
	if(rtk_switch_phyPortId_get(RTK_PORT_CPU7, &ret)==RT_ERR_OK)fc_db.systemGlobal.phyPortStatus|=0x1ULL<<(RTK_FC_MAC_PORT0+ret);

#endif

	// enable src port blocking but exclude cpu ports
	{
		rtk_portmask_t srcPortBlocking;
		rtk_l2_srcPortEgrFilterMask_get(&srcPortBlocking);
		srcPortBlocking.bits[0] &= (~RTK_FC_ALL_MAC_CPU_PORTMASK);
		rtk_l2_srcPortEgrFilterMask_set(&srcPortBlocking);
		rtk_l2_extPortEgrFilterMask_set(&(rtk_portmask_t){{0}});
	}

	enable = ENABLED;
#if defined(CONFIG_RTK_L34_G3_PLATFORM)

#if defined(CONFIG_RTK_FC_EXTERNAL_SWITCH_PORT)
	// to buffer traffic in SOC instead of external switch. each external lan port pkt should be transmitted by different qid.
	if(fc_db.controlFuc.external_switch_en == RTK_FC_EXTERNAL_PORT_SWITCH) {
		uint16 ipq = 16;
		int voqid = 0;
		aal_l2_te_shaper_tbc_cfg_msk_t profile_msk;
		aal_l2_te_shaper_tbc_cfg_t     profile;

		// decrease ipg because we always add 4B port-based vlan.
		aal_l2_te_shaper_ipq_set(0, CA_AAL_MAX_IPG_ID, ipq );
		aal_l2_te_shaper_port_ipq_set(0, CONFIG_RTK_FC_EXTERNAL_SWITCH_PORT, CA_AAL_MAX_IPG_ID);
		
		memset(&profile, 0, sizeof(profile));
		memset(&profile_msk, 0, sizeof(profile_msk));

		/* configure L2 shaper */
		profile_msk.s.rate_m 	= 1;
		profile_msk.s.rate_k	= 1;
		profile_msk.s.bs     	= 1;
		profile_msk.s.mode   	= 1;
		profile_msk.s.state  	= 1;

		/* VoQ shaper is based on 500kbps */
		profile.rate_m = 1000000 / 500;
		profile.rate_k 	= 128 / 2;
		profile.bs     	= (profile.rate_m + 512) >> 9;		//roundup (rate_m/512) in byte mode
		profile.mode   = CA_AAL_SHAPER_MODE_BPS;
		profile.state  	= CA_AAL_SHAPER_ADMIN_STATE_SHAPER;

		for(voqid = 0; voqid < 8; voqid++) {
			aal_l2_te_shaper_voq_tbc_set(0, (CONFIG_RTK_FC_EXTERNAL_SWITCH_PORT* 8 + voqid), profile_msk, &profile);
			aal_l2_tm_es_weight_ratio_set(0, CONFIG_RTK_FC_EXTERNAL_SWITCH_PORT, voqid, 1);
		}
	}
#endif

#else

#if defined(CONFIG_FC_RTL9607C_RTL9603CVD_HYBRID)

	if(CHIP_ID_9607C) {
		//switch padding
		ASSERT_EQ(reg_array_field_write(RTL9607C_P_MISCr, RTK_FC_MAC_PORT_MASTERCPU_CORE0, REG_ARRAY_INDEX_NONE, RTL9607C_PADDING_ENf, &enable), RT_ERR_OK);
		ASSERT_EQ(reg_array_field_write(RTL9607C_P_MISCr, RTK_FC_MAC_PORT_MASTERCPU_CORE1, REG_ARRAY_INDEX_NONE, RTL9607C_PADDING_ENf, &enable), RT_ERR_OK);
		//allow receiving packet which smaller than 64 bytes
		ASSERT_EQ(reg_array_field_write(RTL9607C_P_MISCr, RTK_FC_MAC_PORT_MASTERCPU_CORE0, REG_ARRAY_INDEX_NONE, RTL9607C_RX_SPCf, &enable), RT_ERR_OK);
		ASSERT_EQ(reg_array_field_write(RTL9607C_P_MISCr, RTK_FC_MAC_PORT_MASTERCPU_CORE1, REG_ARRAY_INDEX_NONE, RTL9607C_RX_SPCf, &enable), RT_ERR_OK);
		/*CONFIG_GMAC2_USABLE*/
	    	ASSERT_EQ(reg_array_field_write(RTL9607C_P_MISCr, RTK_FC_MAC_PORT_SLAVECPU, REG_ARRAY_INDEX_NONE, RTL9607C_PADDING_ENf, &enable), RT_ERR_OK);
	    	ASSERT_EQ(reg_array_field_write(RTL9607C_P_MISCr, RTK_FC_MAC_PORT_SLAVECPU, REG_ARRAY_INDEX_NONE, RTL9607C_RX_SPCf, &enable), RT_ERR_OK);
	}else{
		//switch padding
		ASSERT_EQ(reg_array_field_write(RTL9603CVD_P_MISCr, RTK_FC_MAC_PORT_CPU, REG_ARRAY_INDEX_NONE, RTL9603CVD_PADDING_ENf, &enable), RT_ERR_OK);
		//allow receiving packet which smaller than 64 bytes
		ASSERT_EQ(reg_array_field_write(RTL9603CVD_P_MISCr, RTK_FC_MAC_PORT_CPU, REG_ARRAY_INDEX_NONE, RTL9603CVD_RX_SPCf, &enable), RT_ERR_OK);
	}

#elif defined(CONFIG_FC_RTL9607C_SERIES)
	//switch padding
	ASSERT_EQ(reg_array_field_write(RTL9607C_P_MISCr, RTK_FC_MAC_PORT_MASTERCPU_CORE0, REG_ARRAY_INDEX_NONE, RTL9607C_PADDING_ENf, &enable), RT_ERR_OK);
	ASSERT_EQ(reg_array_field_write(RTL9607C_P_MISCr, RTK_FC_MAC_PORT_MASTERCPU_CORE1, REG_ARRAY_INDEX_NONE, RTL9607C_PADDING_ENf, &enable), RT_ERR_OK);
	//allow receiving packet which smaller than 64 bytes
	ASSERT_EQ(reg_array_field_write(RTL9607C_P_MISCr, RTK_FC_MAC_PORT_MASTERCPU_CORE0, REG_ARRAY_INDEX_NONE, RTL9607C_RX_SPCf, &enable), RT_ERR_OK);
	ASSERT_EQ(reg_array_field_write(RTL9607C_P_MISCr, RTK_FC_MAC_PORT_MASTERCPU_CORE1, REG_ARRAY_INDEX_NONE, RTL9607C_RX_SPCf, &enable), RT_ERR_OK);
	/*CONFIG_GMAC2_USABLE*/
    ASSERT_EQ(reg_array_field_write(RTL9607C_P_MISCr, RTK_FC_MAC_PORT_SLAVECPU, REG_ARRAY_INDEX_NONE, RTL9607C_PADDING_ENf, &enable), RT_ERR_OK);
    ASSERT_EQ(reg_array_field_write(RTL9607C_P_MISCr, RTK_FC_MAC_PORT_SLAVECPU, REG_ARRAY_INDEX_NONE, RTL9607C_RX_SPCf, &enable), RT_ERR_OK);
#elif defined(CONFIG_FC_RTL9603CVD_SERIES)
	//switch padding
	ASSERT_EQ(reg_array_field_write(RTL9603CVD_P_MISCr, RTK_FC_MAC_PORT_CPU, REG_ARRAY_INDEX_NONE, RTL9603CVD_PADDING_ENf, &enable), RT_ERR_OK);
	//allow receiving packet which smaller than 64 bytes
	ASSERT_EQ(reg_array_field_write(RTL9603CVD_P_MISCr, RTK_FC_MAC_PORT_CPU, REG_ARRAY_INDEX_NONE, RTL9603CVD_RX_SPCf, &enable), RT_ERR_OK);
#endif

#endif

	return 0;
}

static int _rtl9607c_fc_lut_init(void)
{
	int i;
	uint32 regValue;
	rtk_portmask_t mbpmsk;

#if defined(CONFIG_RTK_L34_XPON_PLATFORM) 
	ASSERT_EQ_CONT(rtk_l2_camState_set(ENABLED), RT_ERR_OK);
#endif

	rtk_l2_lookupMissAction_set(DLF_TYPE_BCAST,ACTION_FOLLOW_FB);

	// unknown DA flooding setting: trap to CPU
	mbpmsk.bits[0]=1U<<RTK_FC_MAC_PORT_MAINCPU;
	rtk_l2_lookupMissFloodPortMask_set(DLF_TYPE_MCAST,&mbpmsk);		//set multicast flooding port
	rtk_l2_lookupMissFloodPortMask_set(DLF_TYPE_BCAST,&mbpmsk);		//set broadcast
	rtk_l2_lookupMissFloodPortMask_set(DLF_TYPE_UCAST,&mbpmsk);		//set unicast unknown DA flooding mask
	rtk_l2_lookupMissAction_set(DLF_TYPE_UCAST, ACTION_FORWARD);		//set unicast unknown DA action as FORWARD to CPU, to prevent strom control fail issue

	//Turn off Lut auto-learning, set unknown SA and port-move trap
	for(i=0;i<RTK_FC_MAC_PORT_MAX;i++)
	{
		if(RTK_FC_ALL_MAC_CPU_PORTMASK & (1ULL<<i)) {
			//Turn off CPU port LUT auto-learning, and set Action to Forward
			ASSERT_EQ_CONT(RTK_L2_PORTLIMITLEARNINGCNT_SET(i,0),RT_ERR_OK);
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
			//ASSERT_EQ(RTK_L2_PORTLIMITLEARNINGCNTACTION_SET(i,LIMIT_LEARN_CNT_ACTION_FORWARD),RT_ERR_OK);
			ASSERT_EQ_CONT(RTK_L2_ILLEGALPORTMOVEACTION_SET(i,ACTION_FORWARD),RT_ERR_OK);
#else
			ASSERT_EQ_CONT(RTK_L2_PORTLIMITLEARNINGCNTACTION_SET(i,LIMIT_LEARN_CNT_ACTION_TO_CPU),RT_ERR_OK);
			// For VXLAN/NPTv6 Sram mode usage: To prevent portmoving from cpu-port.
			ASSERT_EQ_CONT(RTK_L2_ILLEGALPORTMOVEACTION_SET(i, ACTION_FORWARD), RT_ERR_OK);
#endif
		}else {
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
			if(fc_db.controlFuc.loopbackMode_is_enabled)
			{
				if((i == RTK_FC_MAC_PORT5) || (i == RTK_FC_MAC_PORT6))
					continue; // port 5 and port 6 is invalid in switch when loopback mode is enabled
			}
#endif
			ASSERT_EQ_CONT(RTK_L2_PORTLIMITLEARNINGCNT_SET(i,0),RT_ERR_OK);
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
			// G3 use L3FE CLS as ACL rule, so we expect no L2 trap happens before ACL.
			// in such design, adding hw l2 entry is no necessary. that means no unknown sa, no unmatch vlan, no port moving.
			ASSERT_EQ_CONT(RTK_L2_PORTLIMITLEARNINGCNTACTION_SET(i,LIMIT_LEARN_CNT_ACTION_FORWARD),RT_ERR_OK);
			ASSERT_EQ_CONT(RTK_L2_ILLEGALPORTMOVEACTION_SET(i,ACTION_FORWARD),RT_ERR_OK);
#else
			ASSERT_EQ_CONT(RTK_L2_PORTLIMITLEARNINGCNTACTION_SET(i,LIMIT_LEARN_CNT_ACTION_FORWARD),RT_ERR_OK);
			ASSERT_EQ_CONT(RTK_L2_ILLEGALPORTMOVEACTION_SET(i,ACTION_TRAP2CPU),RT_ERR_OK);
#endif
		}

#if defined(CONFIG_RTK_L34_XPON_PLATFORM) || defined(CONFIG_FC_CA8277AB_SERIES)
		// disable lut aging
		ASSERT_EQ_CONT(RTK_L2_PORTAGINGENABLE_SET(i, DISABLED), RT_ERR_OK);
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
		// enable lut aging because of hybrd mode requirement.
		ASSERT_EQ_CONT(RTK_L2_PORTAGINGENABLE_SET(i, ENABLED), RT_ERR_OK);
#endif

#if defined(CONFIG_RTK_L34_XPON_PLATFORM) || defined(CONFIG_FC_CA8277AB_SERIES)	// not support in 8277C
		ASSERT_EQ_CONT(RTK_L2_PORTLOOKUPMISSACTION_SET(i,DLF_TYPE_BCAST,ACTION_FORWARD),RT_ERR_RG_OK);
		ASSERT_EQ_CONT(RTK_L2_PORTLOOKUPMISSACTION_SET(i,DLF_TYPE_UCAST,ACTION_FORWARD),RT_ERR_RG_OK);

#if defined(CONFIG_RTK_L34_G3_PLATFORM)		
		ASSERT_EQ_CONT(RTK_L2_PORTLOOKUPMISSACTION_SET(i,DLF_TYPE_BCAST,ACTION_FORWARD),RT_ERR_RG_OK);
#else
		// trap by L34 flow miss instead of switch flood to cpu port
		ASSERT_EQ_CONT(RTK_L2_PORTLOOKUPMISSACTION_SET(i,DLF_TYPE_BCAST,ACTION_FOLLOW_FB),RT_ERR_RG_OK);
#endif
		ASSERT_EQ_CONT(RTK_L2_PORTLOOKUPMISSACTION_SET(i,DLF_TYPE_UCAST,ACTION_FORWARD),RT_ERR_RG_OK);
#endif
	}

	//Flush all unicast LUT may be learned by hardware
	ASSERT_EQ_CONT(rtk_l2_addr_delAll(ENABLED),RT_ERR_OK);

	// arp used as known
	regValue = 0x1U;
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)

#if defined(CONFIG_FC_RTL9607C_RTL9603CVD_HYBRID)
	if(CHIP_ID_9607C) {
		ASSERT_EQ_CONT(reg_field_write(RTL9607C_LUT_CFGr, RTL9607C_LUT_L34_ARP_USAGE_AS_KNOWNf, &regValue),RT_ERR_OK);	
	}else {
		ASSERT_EQ_CONT(reg_field_write(RTL9603CVD_LUT_CFGr, RTL9603CVD_LUT_L34_ARP_USAGE_AS_KNOWNf, &regValue),RT_ERR_OK);
	}
#elif defined(CONFIG_FC_RTL9607C_SERIES)
	ASSERT_EQ_CONT(reg_field_write(RTL9607C_LUT_CFGr, RTL9607C_LUT_L34_ARP_USAGE_AS_KNOWNf, &regValue),RT_ERR_OK);
#elif defined(CONFIG_FC_RTL9603CVD_SERIES)
	ASSERT_EQ_CONT(reg_field_write(RTL9603CVD_LUT_CFGr, RTL9603CVD_LUT_L34_ARP_USAGE_AS_KNOWNf, &regValue),RT_ERR_OK);
#endif

#endif

	return 0;
}

static int _rtl9607c_fc_svlan_init(void)
{

	int i;
	rtk_portmask_t svlanmbr;

	fc_db.systemGlobal.ifstagTPID1_enabled = FALSE;
	fc_db.systemGlobal.stagTPID[0] = ETH_P_8021AD;
	fc_db.systemGlobal.stagTPID[1] = 0x8888U;
	
	rtk_svlan_tpidEnable_set(0U, ENABLED);
	rtk_svlan_tpidEntry_set(0, fc_db.systemGlobal.stagTPID[0]);
	rtk_svlan_tpidEnable_set(1U, DISABLED);
	rtk_svlan_tpidEntry_set(1, fc_db.systemGlobal.stagTPID[1]);

	rtk_svlan_svlanFunctionEnable_set(DISABLED);

	for(i = RTK_FC_MAC_PORT0; i < RTK_FC_MAC_PORT_MAX; i++)
		RTK_SVLAN_SERVICEPORT_SET(i, ENABLED);

	// change default action from drop to port-based.
	rtk_svlan_untagAction_set(SVLAN_ACTION_PSVID, 0U);

	// keep pri
	rtk_svlan_priorityRef_set(REF_OUTER_TAG);

	 // for switch behavior: port base vlan transparent, used by hybrid mode lan-lan forwarding
	rtk_svlan_create(0U);
	svlanmbr.bits[0] = fc_db.systemGlobal.phyPortStatus;
	rtk_svlan_memberPort_set(0U, &svlanmbr, &svlanmbr);

	rtk_fc_gmac_svlan_tpid_enable(fc_db.systemGlobal.stagTPID[0], 0);
	rtk_fc_gmac_svlan_tpid_enable(fc_db.systemGlobal.stagTPID[1], 1);

#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	// DMA_LSO SVLAN TPID config
	rtk_dma_lso_reg_write(0x800088a8, DMA_SEC_DMA_GLB_DMA_LSO_VLAN_TAG_TYPE0);
#endif

	return 0;
}

static int _rtl9607c_fc_cvlan_init(void)
{

#if !defined(CONFIG_RTK_L34_G3_PLATFORM)	// vlan disabling is not fully supported in CA platform
	rtk_portmask_t mbr, untag;
	int i;

	rtk_vlan_vlanFunctionEnable_set(DISABLED);

	/* Clear all vlan table setting */
	rtk_vlan_destroyAll(0U);
	/* Configure Vlan Table for LAN */
	mbr.bits[0] = (1U<<RTK_FC_MAC_PORT0) | (1U<<RTK_FC_MAC_PORT1) | (1U<<RTK_FC_MAC_PORT2) | (1U<<RTK_FC_MAC_PORT3) | (1U<<RTK_FC_MAC_PORT4) \
		| (1U<<RTK_FC_MAC_PORT_MASTERCPU_CORE0) | (1U<<RTK_FC_MAC_PORT_MASTERCPU_CORE1) | (1U<<RTK_FC_MAC_PORT_SLAVECPU);
	untag.bits[0] = (1U<<RTK_FC_MAC_PORT0) | (1U<<RTK_FC_MAC_PORT1) | (1U<<RTK_FC_MAC_PORT2) | (1U<<RTK_FC_MAC_PORT3) | (1U<<RTK_FC_MAC_PORT4) \
		| (1U<<RTK_FC_MAC_PORT_MASTERCPU_CORE0) | (1U<<RTK_FC_MAC_PORT_MASTERCPU_CORE1) | (1U<<RTK_FC_MAC_PORT_SLAVECPU);
	rtk_vlan_create(LAN_VID);
	rtk_vlan_port_set(LAN_VID,&mbr,&untag);
	mbr.bits[0] = (1U<<FC_EXT_PORT_LIST_MAX)-1U;
	//ASSERT_EQ(rtk_vlan_extPort_set(LAN_VID,&mbr);
	rtk_vlan_extPortmaskCfg_set(0U,&mbr);
	rtk_vlan_extPortmaskIndex_set(LAN_VID, 0U);
	rtk_vlan_fidMode_set(LAN_VID,VLAN_FID_SVL);
	rtk_vlan_fid_set(LAN_VID, DEFAULT_FID);

	/* Configure Vlan Table for WAN */
	mbr.bits[0] = (1U<<RTK_FC_MAC_PORT_PON) | (1U<<RTK_FC_MAC_PORT_RGMII) | (1U<<RTK_FC_MAC_PORT_MASTERCPU_CORE0) \
		| (1U<<RTK_FC_MAC_PORT_MASTERCPU_CORE1) | (1U<<RTK_FC_MAC_PORT_SLAVECPU);
	untag.bits[0] = (1U<<RTK_FC_MAC_PORT_MASTERCPU_CORE0) | (1U<<RTK_FC_MAC_PORT_MASTERCPU_CORE1) | (1U<<RTK_FC_MAC_PORT_SLAVECPU);
	//untag.bits[0] = 0x0;
	rtk_vlan_create(WAN_VID);
	rtk_vlan_port_set(WAN_VID,&mbr,&untag);
	mbr.bits[0] = (1U<<FC_EXT_PORT_LIST_MAX)-1U;
	//ASSERT_EQ(rtk_vlan_extPort_set(WAN_VID,&mbr);
	rtk_vlan_extPortmaskCfg_set(1U,&mbr);
	rtk_vlan_extPortmaskIndex_set(WAN_VID, 1U);
	rtk_vlan_fidMode_set(WAN_VID,VLAN_FID_SVL);
	rtk_vlan_fid_set(WAN_VID, DEFAULT_FID);


	/* Configure Port based VLAN ID register */

	RTK_VLAN_PORTPVID_SET(RTK_FC_MAC_PORT0,LAN_VID);
	RTK_VLAN_PORTPVID_SET(RTK_FC_MAC_PORT1,LAN_VID);
	RTK_VLAN_PORTPVID_SET(RTK_FC_MAC_PORT2,LAN_VID);
	RTK_VLAN_PORTPVID_SET(RTK_FC_MAC_PORT3,LAN_VID);
	RTK_VLAN_PORTPVID_SET(RTK_FC_MAC_PORT4,LAN_VID);
	RTK_VLAN_PORTPVID_SET(RTK_FC_MAC_PORT_PON,WAN_VID);
	RTK_VLAN_PORTPVID_SET(RTK_FC_MAC_PORT_RGMII,WAN_VID);
	RTK_VLAN_PORTPVID_SET(RTK_FC_MAC_PORT_MASTERCPU_CORE0,WAN_VID);
	RTK_VLAN_PORTPVID_SET(RTK_FC_MAC_PORT_MASTERCPU_CORE1,WAN_VID);
	for(i = 0; i < FC_EXT_PORT_LIST_MAX; i++)
		rtk_vlan_extPortPvid_set(i, LAN_VID);

	rtk_vlan_tagMode_set(RTK_FC_MAC_PORT_MASTERCPU_CORE0, VLAN_TAG_MODE_KEEP_FORMAT);
	rtk_vlan_tagMode_set(RTK_FC_MAC_PORT_MASTERCPU_CORE1, VLAN_TAG_MODE_KEEP_FORMAT);
	rtk_vlan_tagMode_set(RTK_FC_MAC_PORT_SLAVECPU, VLAN_TAG_MODE_KEEP_FORMAT);
	rtk_vlan_tagModeIp4mc_set(RTK_FC_MAC_PORT_MASTERCPU_CORE0, VLAN_TAG_MODE_KEEP_FORMAT);
	rtk_vlan_tagModeIp4mc_set(RTK_FC_MAC_PORT_MASTERCPU_CORE1, VLAN_TAG_MODE_KEEP_FORMAT);
	rtk_vlan_tagModeIp4mc_set(RTK_FC_MAC_PORT_SLAVECPU, VLAN_TAG_MODE_KEEP_FORMAT);
	rtk_vlan_tagModeIp6mc_set(RTK_FC_MAC_PORT_MASTERCPU_CORE0, VLAN_TAG_MODE_KEEP_FORMAT);
	rtk_vlan_tagModeIp6mc_set(RTK_FC_MAC_PORT_MASTERCPU_CORE1, VLAN_TAG_MODE_KEEP_FORMAT);
	rtk_vlan_tagModeIp6mc_set(RTK_FC_MAC_PORT_SLAVECPU, VLAN_TAG_MODE_KEEP_FORMAT);
#else
	rtk_vlan_vlanFunctionEnable_set(DISABLED);
#endif

	return 0;
}

/* Lut learning callback func */
int rtk_fc_rtl9607c_API_lut_add(void* input_data)
{
	int ret = RTK_FC_RET_OK;
	rtk_fc_lut_entry_t *pLutEntry = (rtk_fc_lut_entry_t *)input_data;

	if(pLutEntry->l2Addr[0]&0x1)
	{

		rtk_l2_mcastAddr_t mcastAddr;
		memset(&mcastAddr,0,sizeof(mcastAddr));
		mcastAddr.portmask.bits[0]=0U;
		mcastAddr.ext_portmask_idx=31U;
		mcastAddr.vid =pLutEntry->CVID;
		mcastAddr.fid = pLutEntry->svl_fid;
		memcpy(&mcastAddr.mac.octet, &pLutEntry->l2Addr, ETH_ALEN);
		ret=rtk_l2_mcastAddr_add(&mcastAddr);
		if(ret==RT_ERR_OK){
			TABLE("Add MC LUT[%d]",  mcastAddr.index);
			pLutEntry->lutIdx = mcastAddr.index;
		}
		else{
			TABLE("Fail to add mac %pM to LUT table, rtk_ret = 0x%x", pLutEntry->l2Addr, ret);
		}
	}
	else
	{
		rtk_l2_ucastAddr_t l2Addr;

		memset(&l2Addr, 0, sizeof(l2Addr));

		l2Addr.flags|=RTK_L2_UCAST_FLAG_ARP_USED;
		l2Addr.fid = pLutEntry->svl_fid;
		l2Addr.age = 7U;
		l2Addr.port = pLutEntry->spa;
		l2Addr.ext_port = pLutEntry->extspa;
		if(pLutEntry->CTAG_IF)
		{
			l2Addr.flags|=RTK_L2_UCAST_FLAG_CTAG_IF;
		}
		else
		{
			l2Addr.flags&=(~RTK_L2_UCAST_FLAG_CTAG_IF);
		}
		l2Addr.vid = pLutEntry->CVID;

		// Hw lut aging is enabled in hybrid mode, so we need to set lut to static
		l2Addr.flags|=RTK_L2_UCAST_FLAG_STATIC;

		memcpy(&l2Addr.mac.octet, &pLutEntry->l2Addr, ETH_ALEN);

		ret = rtk_l2_addr_add(&l2Addr);

		if(ret==RT_ERR_OK){			
			TABLE("Add LUT[%d] MAC %pM SPA %d EXTSPA %d CTAGIF %d VID %d", l2Addr.index, &l2Addr.mac.octet[0], l2Addr.port, l2Addr.ext_port, (l2Addr.flags&RTK_L2_UCAST_FLAG_CTAG_IF)?TRUE:FALSE, l2Addr.vid);
			pLutEntry->lutIdx = l2Addr.index;
		}
		else{
			TABLE("Fail to add mac %pM to LUT table, rtk_ret = 0x%x", pLutEntry->l2Addr, ret);
		}
	}
	if(ret==RT_ERR_OK)
		ret = RTK_FC_RET_OK;
	else if(ret==RT_ERR_ENTRY_FULL)
		ret = RTK_FC_RET_ERR_ENTRY_FULL;
	else
		ret = RTK_FC_RET_ERR;

	return ret;
}

int rtk_fc_rtl9607c_API_lut_del(void* input_data)
{
	rtk_fc_lut_entry_t *pLutEntry = (rtk_fc_lut_entry_t *)input_data;
	uint8 *mac = &pLutEntry->l2Addr[0];
	int ret = RT_ERR_OK, delLutDirectly=TRUE;
	rtk_l2_ucastAddr_t l2UcAddr;
	rtk_l2_mcastAddr_t l2McAddr;


	TABLE("Del LUT mac = %pM", mac);

	if((mac[0]&0x1)==0x0)
	{
		if(fc_db.hypridPPTP.portmask)
		{
			bzero(&l2UcAddr, sizeof(l2UcAddr));
			l2UcAddr.fid = pLutEntry->svl_fid;
			memcpy(&l2UcAddr.mac.octet, mac, ETH_ALEN);

			ret = rtk_l2_addr_get(&l2UcAddr);
			if(ret == RT_ERR_OK)
			{
				//Because this lut entry may be used by HW PPTP, we just disable its static and set its age to 2 for short timeout.
				if((0x1ULL<<l2UcAddr.port) & fc_db.wanPortMask.portmask)
				{
					TABLE("Disable static flag of lut[%d] and set its age to 2", l2UcAddr.index);
					l2UcAddr.flags &= (~RTK_L2_UCAST_FLAG_STATIC);
					l2UcAddr.flags &= (~RTK_L2_UCAST_FLAG_ARP_USED);
					l2UcAddr.age = 2U;
					ret = rtk_l2_addr_add(&l2UcAddr);
					delLutDirectly = FALSE;
				}
			}
			else
			{
				WARNING("[Fail to del hw lut] ret=%d MAC=%pM does not exist!", ret, mac);
				return RTK_FC_RET_ERR;
			}
		}
		if(delLutDirectly)
		{
			bzero(&l2UcAddr, sizeof(l2UcAddr));
			l2UcAddr.fid = pLutEntry->svl_fid;
			memcpy(&l2UcAddr.mac.octet, mac, ETH_ALEN);

			ret = rtk_l2_addr_del(&l2UcAddr);
		}
	}
	else
	{
		bzero(&l2McAddr, sizeof(l2McAddr));
		l2McAddr.fid = pLutEntry->svl_fid;
		memcpy(&l2McAddr.mac.octet, mac, ETH_ALEN);

		ret = rtk_l2_mcastAddr_del(&l2McAddr);
	}

	if(ret!=RT_ERR_OK)
	{
		WARNING("[Fail to del hw lut] ret=%d MAC=%pM", ret, mac);
		return RTK_FC_RET_ERR;
	}
	else
		return RTK_FC_RET_OK;
}

int rtk_fc_rtl9607c_API_lut_query(unsigned char *mac, int *lutIdx)
{
	int ret = 0;
	rtk_l2_ucastAddr_t l2UcAddr;
	rtk_l2_mcastAddr_t l2McAddr;


	if((mac[0]&0x1)==0x0)
	{
		bzero(&l2UcAddr, sizeof(l2UcAddr));
		l2UcAddr.fid = DEFAULT_FID;
		memcpy(&l2UcAddr.mac.octet, mac, ETH_ALEN);

		ret = rtk_l2_addr_get(&l2UcAddr);
		if(ret == RT_ERR_OK)
			*lutIdx = l2UcAddr.index;
	}
	else
	{
		bzero(&l2McAddr, sizeof(l2McAddr));
		l2McAddr.fid = DEFAULT_FID;
		memcpy(&l2McAddr.mac.octet, mac, ETH_ALEN);
		ret = rtk_l2_mcastAddr_get(&l2McAddr);
		if(ret == RT_ERR_OK)
			*lutIdx = l2McAddr.index;
	}

	return ret;
}



int rtk_fc_rtl9607c_API_init(void)
{
	int ret = 0;

	//printk("========== RTK FleetConntrack Driver Init ==========\n");

#ifdef __KERNEL__
	ASSERT_EQ(rtk_init_without_pon(),RT_ERR_OK);
#else
	ASSERT_EQ(rtk_init(),RT_ERR_OK);
#endif

	ASSERT_EQ(rtk_l2_init(), RT_ERR_OK);

	if((ret = _rtl9607c_fc_port_init()) != RTK_FC_RET_OK) {
		printk("HWNAT global state init failed: %x\n", ret);
		goto ERROR;
	}

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	{
		uint32 flowMode = FB_MODE_32K;
#if defined(CONFIG_FC_FLOWBASED_DYN_MAX_32K)
		flowMode = FB_MODE_32K;
#elif defined(CONFIG_FC_FLOWBASED_DYN_MAX_16K)
		flowMode = FB_MODE_16K;
#elif defined(CONFIG_FC_FLOWBASED_DYN_MAX_8K)
		flowMode = FB_MODE_8K;
#elif defined(CONFIG_FC_FLOWBASED_DYN_MAX_4K)
		flowMode = FB_MODE_4K;
#endif

		if((ret = _rtk_rg_fbMode_set(flowMode)) != RTK_FC_RET_OK) {
			printk("HWNAT init failed: %x\n", ret);
			goto ERROR;
		}
	}
#endif

	if((ret = _rtl9607c_fc_lut_init()) != RTK_FC_RET_OK) {
		printk("HWNAT init failed: %x\n", ret);
		goto ERROR;
	}

	if((ret = _rtl9607c_fc_global_state_init()) != RTK_FC_RET_OK) {
		printk("HWNAT global state init failed: %x\n", ret);
		goto ERROR;
	}

	if((ret = _rtl9607c_fc_cvlan_init()) != RTK_FC_RET_OK) {
		printk("HWNAT cvlan init failed: %x\n", ret);
		goto ERROR;
	}

	if((ret = _rtl9607c_fc_svlan_init()) != RTK_FC_RET_OK) {
		printk("HWNAT global state init failed: %x\n", ret);
		goto ERROR;
	}


#if !defined(CONFIG_RTK_L34_G3_PLATFORM)
	if((ret = rtk_rg_asic_hsbaMode_set(L34_HSBA_LOG_ALL)) != RTK_FC_RET_OK)
	{
		printk("HWNAT global state init failed: %x\n", ret);
		goto ERROR;
	}
#endif

	rtk_fc_tables_init();

ERROR:
	return ret;
}

#endif

#if defined(CONFIG_RTK_L34_G3_PLATFORM)

int rtk_fc_flow_tuple_init(void)
{
	ca_status_t ca_ret;
	rtk_rg_flow_key_mask_t flowKeyMask;

	bzero(&flowKeyMask, sizeof(flowKeyMask));

	flowKeyMask.P12_vlanId = fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATH12_SKIP_CVID] ? FALSE : TRUE;
	flowKeyMask.P12_vlanPri = (fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATH12_SKIP_CPRI] && fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATHALL_SKIP_SPRI]) ? FALSE : TRUE;
	flowKeyMask.P345_vlanId = fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATH5_SKIP_CVID] ? FALSE : TRUE;
	flowKeyMask.P345_vlanPri = (fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATH5_SKIP_CPRI] && fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATHALL_SKIP_SPRI]) ? FALSE : TRUE;
	flowKeyMask.pall_vlan_deicfi = fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_PATHALL_SKIP_VLAN_DEICFI] ? FALSE : TRUE;
	flowKeyMask.p12_stream_id = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_STREAM_IDX];
	flowKeyMask.p345_stream_id = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH345_STREAM_IDX];
	flowKeyMask.p12_ethtype = fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_PROTOCOL];
	if(fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH12_TOS]==ENABLED)
	{
		flowKeyMask.P12_dscp = TRUE;
		flowKeyMask.P12_ecn = fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_CMP_TOS];
	}
	else
	{
		flowKeyMask.P12_dscp = FALSE;
		flowKeyMask.P12_ecn = FALSE;
	}
	if(fc_db.systemGlobal.flowCheckState[FB_FLOW_CHECK_PATH5_TOS]==ENABLED)
	{
		flowKeyMask.P345_dscp = TRUE;
		flowKeyMask.P345_ecn = fc_db.systemGlobal.fbGlobalState[FB_GLOBAL_CMP_TOS];
	}
	else
	{
		flowKeyMask.P345_dscp = FALSE;
		flowKeyMask.P345_ecn = FALSE;
	}
	ca_ret = rtk_rg_g3_flow_init(flowKeyMask);
	if(ca_ret!=CA_E_OK)
	{
		WARNING("Fail to initialize G3 flow, ca_ret=0x%x", ca_ret);
		return RT_ERR_RG_FAILED;
	}

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	{
		bool if_cache_age0_hit = FALSE;
		hash_type0_flow_action_mode_t flow_act_mode;
		ca_ret = aal_hash_cache_age0_hit_get(&if_cache_age0_hit);
		if(ca_ret != AAL_E_OK)
		{
			WARNING("Fail to get hash cache age0 hit state, ca_ret=0x%x", ca_ret);
			return RT_ERR_RG_FAILED;
		}
		if(if_cache_age0_hit)
			fc_db.controlFuc.if_hash_cache_age0_hit = TRUE;
		else
			fc_db.controlFuc.if_hash_cache_age0_hit = FALSE;

		ca_ret = aal_hash_action_mode_get(RTK_ASIC_DEVID, &flow_act_mode);
		if(ca_ret != AAL_E_OK)
		{
			WARNING("Fail to get hash action mode, ca_ret=0x%x", ca_ret);
			return RT_ERR_RG_FAILED;
		}
		if(flow_act_mode == HASH_TYPE0_FLOW_ACTION_MODE_NORMAL)
			fc_db.controlFuc.if_hash_fib_384_bits = FALSE;
		else
			fc_db.controlFuc.if_hash_fib_384_bits = TRUE;
	}
#endif
	return ca_ret;
}
int rtk_fc_ca_API_lut_add(void* input_data)
{
	int ret = RTK_FC_RET_OK;
	rtk_fc_lut_entry_t *pLutEntry = (rtk_fc_lut_entry_t *)input_data;

#if defined(CONFIG_FC_CA8277AB_SERIES)
	uint32 hwL2Idx;
	ca_l2_addr_entry_t addr;

	if(pLutEntry->l2Addr[0]&0x1) {
		// pure software lut for multicast mac
		return RTK_FC_RET_OK;
	}
	
	memset(&addr, 0, sizeof(addr));
	memcpy(addr.mac_addr, &pLutEntry->l2Addr, ETH_ALEN);

	addr.static_flag = 1;	// prevent hw overwriting
	
	addr.vid = pLutEntry->CVID;
	addr.fwd_vid = pLutEntry->CVID;

	addr.sa_permit = 1;
	addr.da_permit = 1;
	addr.port_id = pLutEntry->spa;
	
	if(pLutEntry->l2Addr[0]&0x1)
		addr.mc_group_id = 0;
	else
		addr.mc_group_id = CA_UINT32_INVALID;

	ret = ca_l2_addr_add(CA_DEF_DEVID,&addr);

		
	if(ret==CA_E_OK){
	
		if(_rtk_fc_hw_lut_idx_get(&pLutEntry->l2Addr[0], pLutEntry->svl_fid, &hwL2Idx)!=RT_ERR_OK) {
			WARNING("FIXME: Fail to get mac %pM from LUT table", pLutEntry->l2Addr);
			ret = RTK_FC_RET_ERR;
		} else {
			pLutEntry->lutIdx = hwL2Idx;
			
			TABLE("Add LUT[%d] MAC %pM SPA %d CTAGIF %d VID %d", hwL2Idx, &pLutEntry->l2Addr[0], pLutEntry->spa, pLutEntry->CTAG_IF, pLutEntry->CVID);

			ret = RTK_FC_RET_OK;
		}
	}else if(ret == CA_E_FULL) {
		TABLE("RTK_FC_RET_ERR_ENTRY_FULL");
		ret = RTK_FC_RET_ERR_ENTRY_FULL;
	}else {
		WARNING("Fail to add mac %pM, ret = 0x%x, hashidx = %d", pLutEntry->l2Addr, ret, _rtk_fc_hash_mac_fid(&addr.mac_addr[0], 0) << RTK_FC_LUT_HASH_WAY_SHIFT);
	} 
#else

	if(pLutEntry->l2Addr[0]&0x1)
	{
		ca_uint32_t addr=0;
		aal_fdb_entry_cfg_t   fdbEntry;
		memset(&fdbEntry, 0, sizeof(aal_fdb_entry_cfg_t));
		memcpy(fdbEntry.mac,pLutEntry->l2Addr,sizeof(ca_mac_addr_t));
		
		fdbEntry.static_flag	= 1;
		fdbEntry.sa_permit		= 1;
		fdbEntry.da_permit		= 1;
		//for hybird case avoid cf drop (cf only bypass ldpid = (RTK_FC_MAC_PORT_L3_WAN/RTK_FC_MAC_PORT_L3_LAN))
		fdbEntry.port_id		= RTK_FC_MAC_PORT_L3_WAN;
		fdbEntry.aging_sts		= 1;
		fdbEntry.aging_time		= 7;
		if((ret = aal_fdb_entry_add(0,&fdbEntry))!=CA_E_OK)
		{
			TABLE("table aal_fdb_entry_add Error ret=%d",ret);
			return RTK_FC_RET_ERR;
		}
		/*Get FDB table address*/
		if(CA_E_OK == aal_fdb_entry_offset_get(0, fdbEntry.mac, 
			fdbEntry.key_vid, fdbEntry.dot1p, &addr))
		{
			pLutEntry->lutIdx  = addr;
		}


	}
	else
	{
		rtk_l2_ucastAddr_t l2Addr;

		memset(&l2Addr, 0, sizeof(l2Addr));

		l2Addr.flags|=RTK_L2_UCAST_FLAG_ARP_USED;
		l2Addr.fid = pLutEntry->svl_fid;
		l2Addr.age = 7;
		l2Addr.port = pLutEntry->spa;
		l2Addr.ext_port = pLutEntry->extspa;
		if(pLutEntry->CTAG_IF)
		{
			l2Addr.flags|=RTK_L2_UCAST_FLAG_CTAG_IF;
		}
		else
		{
			l2Addr.flags&=(~RTK_L2_UCAST_FLAG_CTAG_IF);
		}
		l2Addr.vid = pLutEntry->CVID;

		// Hw lut aging is enabled in hybrid mode, so we need to set lut to static
		l2Addr.flags|=RTK_L2_UCAST_FLAG_STATIC;

		memcpy(&l2Addr.mac.octet, &pLutEntry->l2Addr, ETH_ALEN);

		/*dal_rtl8277c_l2_addr_add*/
		ret = rtk_l2_addr_add(&l2Addr); 
		
		if(ret==RT_ERR_OK){ 		
			TABLE("Add LUT[%d] MAC %pM SPA %d EXTSPA %d CTAGIF %d VID %d", l2Addr.index, &l2Addr.mac.octet[0], l2Addr.port, l2Addr.ext_port, (l2Addr.flags&RTK_L2_UCAST_FLAG_CTAG_IF)?TRUE:FALSE, l2Addr.vid);
			pLutEntry->lutIdx = l2Addr.index;
		}
		else{
			TABLE("Fail to add mac %pM to LUT table, rtk_ret = 0x%x", pLutEntry->l2Addr, ret);
		}

	}

		
	if(ret==RT_ERR_OK)
		ret = RTK_FC_RET_OK;
	else if(ret==RT_ERR_ENTRY_FULL)
		ret = RTK_FC_RET_ERR_ENTRY_FULL;
	else
		ret = RTK_FC_RET_ERR;
#endif

	return ret;
}
int rtk_fc_ca_API_lut_del(void* input_data)
{	
	rtk_fc_lut_entry_t *pLutEntry = (rtk_fc_lut_entry_t *)input_data;
	uint8 *mac = &pLutEntry->l2Addr[0];
	int ret = RTK_FC_RET_OK;
	
#if defined(CONFIG_FC_CA8277AB_SERIES)
	ca_l2_addr_entry_t addr;

	if(mac[0]&0x1) {
		// pure software lut for multicast mac
		return RTK_FC_RET_OK;
	}

	memset(&addr, 0, sizeof(addr));
	memcpy(addr.mac_addr, mac, ETH_ALEN);

	if((ret = ca_l2_addr_get(CA_DEF_DEVID, addr.mac_addr, pLutEntry->svl_fid, &addr))!=CA_E_OK)
	{
		WARNING("Fail to get mac %pM, ret = 0x%x", mac, ret);
		return RTK_FC_RET_ERR;
	}
	
	TABLE("Del LUT mac = %pM", mac);
	
	if((ret = ca_l2_addr_delete(CA_DEF_DEVID, &addr))!=CA_E_OK)
	{
		WARNING("Fail to del mac %pM, ret = 0x%x", mac, ret);
		return RTK_FC_RET_ERR;
	}
#else
	if(mac[0]&0x1) 
	{
		aal_fdb_entry_cfg_t   fdb_entry;
		memset(&fdb_entry,0,sizeof(aal_fdb_entry_cfg_t));

		ret = aal_fdb_entry_get(0,mac,0,&fdb_entry);
        if(ret)
        {
			TABLE("mac = %pM aal_fdb_entry_get fail ret=%d",mac,ret);
			return RTK_FC_RET_ERR;
        }
		ret = aal_fdb_entry_del(0, &fdb_entry);
        if(ret)
        {
			TABLE("mac = %pM aal_fdb_entry_del fail ret=%d",mac,ret);
			return RTK_FC_RET_ERR;
        }		
	}
	else
	{
		rtk_l2_ucastAddr_t l2UcAddr;

		bzero(&l2UcAddr, sizeof(l2UcAddr));
		l2UcAddr.fid = pLutEntry->svl_fid;
		memcpy(&l2UcAddr.mac.octet, mac, ETH_ALEN);

		//dal_rtl8277c_l2_addr_del
		ret = rtk_l2_addr_del(&l2UcAddr);
	}
#endif

	return ret;
}
int rtk_fc_ca_API_lut_query(unsigned char *mac, int *lutIdx)
{
	uint32 hwL2Idx;

	if(_rtk_fc_hw_lut_idx_get(mac, DEFAULT_FID, &hwL2Idx)!=RT_ERR_OK) {
		WARNING("FIXME: Fail to get mac %pM from LUT table", mac);
		return RTK_FC_RET_ERR;
	} else {
		*lutIdx = hwL2Idx;
		
		TABLE("Find mac %pM from LUT[%d]", mac, hwL2Idx);
	}

	return RTK_FC_RET_OK;
}
int rtk_fc_ca_API_init(void)
{
	rtk_fc_rtl9607c_API_init();

	rtk_asic_init();

#if defined(CONFIG_FC_CA8277AB_SERIES)
#if defined(CONFIG_FC_CAG3_SERIES)
	if(fc_db.controlFuc.loopbackMode_is_enabled)
		ASSERT_EQ_CONT(rtk_rg_g3_l2fe_hostpolicing_init(), RT_ERR_RG_OK);
#endif
	if(fc_db.controlFuc.port4_hsgmii_en) {
		rtk_rg_asic_l3qm_p4_init();
	}
#endif
	
	rtk_fc_flow_tuple_init();

	/*************************
	 * CPU Port 0x12~0x17 for wifi
	 *************************/
	ASSERT_EQ_CONT(rtk_rg_g3_cpuport_init(), CA_E_OK);


	//rtk_ne_reg_write(0xa0000000, L3FE_FE_L3E_HS_CACHE_MISC);

	return RTK_FC_RET_OK;
}

int rtk_fc_pf_exit(void)
{
	rtk_asic_exit();

	return SUCCESS;
}
#endif


static int _rtk_fc_extPMask_init(void)
{
	int i;

	// Init table
	for(i = 0; i < RTK_FC_TABLESIZE_EXTPORT; i++)
	{
		bzero(&fc_db.extPortTbl[i], sizeof(rtk_fc_tableExtPort_t));
	}

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	rtk_rg_asic_table_reset(FB_RST_EXT_PMASK_TYPE);
#endif

	return SUCCESS;
}

#if !(defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)) // 8277C/9607F no need indMac
static int _rtk_fc_indMac_addr_init(void)
{
	int i;

	// Init table
	for(i = 0; i < RTK_FC_TABLESIZE_INDMAC; i++)
	{
		bzero(&fc_db.indMacTbl[i], sizeof(rtk_fc_tableIndMac_t));
	}

	// Init list
	for(i=0; i<RTK_FLOWBASE_BUCKETSIZE_INDMAC; i++)
		INIT_LIST_HEAD(&fc_db.listHead_indMacHash[i]);


#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	rtk_rg_asic_table_reset(FB_RST_MAC_INDEX_TBL);
#endif

	return SUCCESS;
}

// input: l2 address
// output: indMac table index
static int _rtk_fc_indMac_addr_ref(rtk_l2_ucastAddr_t *l2Addr, int *index)
{
	int ret=RTK_FC_RET_OK;
	int i;
	rtk_fc_tableIndMac_t *entry = NULL;


	for(i=0;i<RTK_FC_TABLESIZE_INDMAC;i++)
	{
		if((fc_db.indMacTbl[i].valid==0U) || (fc_db.indMacTbl[i].indMacRefCount == 0U))
		{
			entry = &fc_db.indMacTbl[i];
			break;
		}
	}

	if(i == RTK_FC_TABLESIZE_INDMAC)
	{
		DEBUG("Indirect MAC table full!");
		return RTK_FC_RET_ERR_ENTRY_FULL;
	}

	if(entry != NULL)
	{
		if(entry->valid==0){
			// new empty entry
			memset(entry,0,sizeof(rtk_fc_tableIndMac_t));
			entry->hwIdx = i;
		}else if(fc_db.indMacTbl[i].indMacRefCount == 0U){
			// replace old entry
			list_del(&entry->hashList);
			memset(entry,0,sizeof(rtk_fc_tableIndMac_t));
			entry->hwIdx = i;
		}
		INIT_LIST_HEAD(&entry->hashList);
		list_add_tail(&entry->hashList, &fc_db.listHead_indMacHash[l2Addr->mac.octet[5]&(RTK_FLOWBASE_BUCKETSIZE_INDMAC-1)]);
		// Sync. to H/W
		entry->indMac.l2_idx = l2Addr->index;

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
		ret = rtk_rg_asic_indirectMacTable_add(i, &entry->indMac);
#endif
		if(ret!=RTK_FC_RET_OK)
		{
			DEBUG("Add H/W indirect mac table failed!");
			return ret;
		}else
			TABLE("Add indMac[%d] -> l2: %d", entry->hwIdx, entry->indMac.l2_idx);
		entry->valid = TRUE;
		*index = i;
	}

	return RTK_FC_RET_OK;
}


int rtk_fc_indMac_idx_get(unsigned int l2Idx, int *indMacIdx)
{
	int ret = RTK_FC_RET_OK;
	int index=SIGNED_INVALID;
	rtk_fc_tableIndMac_t *pIndMacEntry;
	rtk_l2_addr_table_t l2Addr;
	int hashing;

	FC_PARAM_CHK(l2Idx>=RTK_FC_TABLESIZE_LUT, RTK_FC_RET_ERR_INDEX_OUT_OF_RANGE);
	
	if(unlikely(fc_db.lutTbl[l2Idx] == NULL)){
		WARNING("lut entry[%d] is not exist!\n", l2Idx);
		return RTK_FC_RET_ERR_NULL_POINTER;
	}

	// search exist entry
	hashing = (fc_db.lutTbl[l2Idx]->l2Addr[5]&(RTK_FLOWBASE_BUCKETSIZE_INDMAC-1));
	// find exist entry
	list_for_each_entry(pIndMacEntry, &fc_db.listHead_indMacHash[hashing], hashList)
	{
		if(l2Idx == pIndMacEntry->indMac.l2_idx)
		{
			index = pIndMacEntry->hwIdx;
			break;
		}
	}
	
	// miss: try to add a new one
	if(index == SIGNED_INVALID) {
		
		bzero(&l2Addr, sizeof(l2Addr));

		if(fc_db.lutTbl[l2Idx]){
			memcpy(&l2Addr.entry.l2UcEntry.mac.octet[0], fc_db.lutTbl[l2Idx]->l2Addr, ETH_ALEN);
			l2Addr.entry.l2UcEntry.index = l2Idx;
		}
		
		if((ret = _rtk_fc_indMac_addr_ref(&l2Addr.entry.l2UcEntry, &index)) != RTK_FC_RET_OK)
			return ret;
	}

	*indMacIdx = index;

	return ret;
}
#endif

//return old entry or free entry 
int rtk_fc_netif_get_freeEntry(uint32 psIfidxStackKey,int16 *swNetifIdx)
{
	int i = 0;
	int firtInvalid=FAILED;
	*swNetifIdx = SIGNED_INVALID;


	for(i = 0; i< RTK_FC_TABLESIZE_INTF_SW; i++)
	{
		if(fc_db.netifTbl[i].intf.valid)
		{
			if(fc_db.netifTbl[i].psIfidxStackKey == psIfidxStackKey)
			{
				*swNetifIdx = i;
				return RTK_FC_RET_ERR_ENTRY_EXIST;
			}
		}
		else if(firtInvalid==FAILED)
			firtInvalid=i;
	}

	if(firtInvalid==FAIL)
		return RTK_FC_RET_ERR_ENTRY_FULL;

	*swNetifIdx=firtInvalid;

	//init netif table
	bzero(&fc_db.netifTbl[firtInvalid],sizeof(fc_db.netifTbl[firtInvalid]));

	fc_db.netifTbl[firtInvalid].intf.valid=1;
	fc_db.netifTbl[firtInvalid].psIfidxStackKey=psIfidxStackKey;

	fc_db.netifTbl[firtInvalid].lutIdx = SIGNED_INVALID;
	fc_db.netifTbl[firtInvalid].hwIdx = SIGNED_INVALID;
	fc_db.netifTbl[firtInvalid].pppoeGwLutIdx = SIGNED_INVALID; 
	fc_db.netifTbl[firtInvalid].outerHdrFlowIdx = SIGNED_INVALID; 
	fc_db.netifTbl[firtInvalid].outerHdrExtratagIdx = SIGNED_INVALID; 
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	fc_db.netifTbl[firtInvalid].dualHdr_ds_clsIdx = CA_UINT32_INVALID;
	fc_db.netifTbl[firtInvalid].dualHdr_ds_extra_clsIdx = CA_UINT32_INVALID;
	fc_db.netifTbl[firtInvalid].hwFmrIdx = SIGNED_INVALID;
#endif

	//the fixed value
	fc_db.netifTbl[firtInvalid].intf.ingress_action = FB_ACTION_FORWARD;
	fc_db.netifTbl[firtInvalid].intf.egress_action = FB_ACTION_FORWARD;
	fc_db.netifTbl[firtInvalid].intf.deny_ipv4 = 0;
	fc_db.netifTbl[firtInvalid].intf.deny_ipv6 = 0;
	fc_db.netifTbl[firtInvalid].intf.allow_ingress_portmask.bits[0]=0;
	fc_db.netifTbl[firtInvalid].intf.allow_ingress_ext_portmask.bits[0]=0;
	fc_db.netifTbl[firtInvalid].intf.out_pppoe_act = FB_NETIFPPPOE_ACT_REMOVE;
	fc_db.netifTbl[firtInvalid].intf.intf_mtu_check = 1U;
	fc_db.netifTbl[firtInvalid].hwEntryNum = 1U;

	return RTK_FC_RET_OK;
}


int rtk_fc_netif_sw_del_by_fcDevIdx(int32 fcDevIdx)
{
	int i,j;
	int stackingCount;
	int32 p_fcDevIdx[DEV_STACK_MAX]={0};

	if(fcDevIdx==DEVIFIDX_INVALID_MIN)
		return RTK_FC_RET_ERR_INVALID_PARAM;

	TABLE("delete netif by devGwMacTbl[%d]",fcDevIdx);

	for(i=0;i<RTK_FC_TABLESIZE_INTF_SW;i++)
	{
		if(!fc_db.netifTbl[i].intf.valid)
			continue;
		
		stackingCount=rtk_fc_psIfidxStackKey_to_fcDevIdx(fc_db.netifTbl[i].psIfidxStackKey,p_fcDevIdx);
		for(j=0;j<stackingCount;j++)
		{

			if(p_fcDevIdx[j]== fcDevIdx)
			{
				rtk_fc_netif_sw_del(i);
				break;
			}
		}

	}
	return RTK_FC_RET_OK;
}


// del sw netif
int rtk_fc_netif_sw_del(int swEntIdx)
{
	int ret = 0 ;

	TABLE("delete netif[%d] hwIdx[%d]",swEntIdx,fc_db.netifTbl[swEntIdx].hwIdx);
	FC_PARAM_CHK(swEntIdx>=RTK_FC_TABLESIZE_INTF_SW, FAIL);
	FC_PARAM_CHK(swEntIdx<0, FAIL);
	FC_PARAM_CHK((fc_db.netifTbl[swEntIdx].intf.valid==0), SUCCESS);

	if(fc_db.netifTbl[swEntIdx].lutIdx != SIGNED_INVALID)
	{
		ret = _rtk_fc_lut_staticEntry_del(fc_db.netifTbl[swEntIdx].lutIdx, FALSE);
		fc_db.netifTbl[swEntIdx].lutIdx=SIGNED_INVALID;
	}
	
	if(fc_db.netifTbl[swEntIdx].hwIdx!=SIGNED_INVALID)
	{
		rtk_fc_netif_dummy_packet_clear(fc_db.netifTbl[swEntIdx].hwIdx);		
		rtk_fc_hwNetif_del_by_swNetif(swEntIdx);
		fc_db.netifTbl[swEntIdx].hwIdx=SIGNED_INVALID;
	}

	fc_db.netifTbl[swEntIdx].intf.valid=0;

	//should after valid set to zero
	if(fc_db.netifTbl[swEntIdx].dualType==RTK_FC_DUALTYPE_IP4INIP6)
		_rtk_fc_aclAndCfReservedRule_intfRsvUpdate(FALSE, swEntIdx);

	return SUCCESS;
}


static int _rtk_fc_devGwMac_init(void)
{
	int i;
	for(i = DEVIFIDX_VALID_MIN; i < RTK_FC_DEV_GW_MAC_TBL; i++)
	{
		fc_db.devGwMacTbl[i].dev=NULL;
		fc_db.devGwMacTbl[i].hwlutIdx = SIGNED_INVALID;
		fc_db.devGwMacTbl[i].myMacIdx = SIGNED_INVALID;
	}
	return SUCCESS;
}

static int _rtk_fc_devGwMac_reset(void)
{
	int i;
	for(i = DEVIFIDX_VALID_MIN; i < RTK_FC_DEV_GW_MAC_TBL; i++)
	{
		if(fc_db.devGwMacTbl[i].dev)
			rtk_fc_devGwMac_del(fc_db.devGwMacTbl[i].dev);
	}
	return SUCCESS;
}


int rtk_fc_gwMac_lut_add( uint8* mac,int16* index,int isWanGwMac,int forceLearningPort,int macportidx,int macextportidx,int wlanidx)
{
	
	rtk_fc_pmap_t portmap;
	int wlanIdx=RTK_FC_WLANX_NOT_FOUND;
	int ret=RTK_FC_RET_OK;

	portmap.macExtPortIdx = 0;
	portmap.macPortIdx = RTK_FC_MAC_PORT_MAINCPU;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)				
	if(isWanGwMac)
		portmap.macPortIdx = ASIC_LPORT_L3_WAN;
	else
		portmap.macPortIdx = ASIC_LPORT_L3_LAN;
#endif
	
	//if mac has exited in l2 entry, delete (mac clone case)
	if(_rtk_fc_lut_find(mac, index) == RTK_FC_RET_OK) 
	{
		if(forceLearningPort)
		{
			fc_db.lutTbl[*index]->wlan_dev_idx = wlanidx;
			fc_db.lutTbl[*index]->spa = macportidx;
			fc_db.lutTbl[*index]->extspa = macextportidx;
			RTK_FC_LUT_ADD(fc_db.lutTbl[*index], TRUE); //update entry to hardware
		}
		else
		{
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)			
			//For the case: WAN interface and LAN interface with same mac, we should always set lut spa to 0x19
			//Use L2 classification rule 0 to set ldpid = 0x18 when packet is from WAN and with the same mac.
			if(fc_db.lutTbl[*index]->spa == ASIC_LPORT_L3_WAN && (int)portmap.macPortIdx  == (int)ASIC_LPORT_L3_LAN)
			{
				fc_db.lutTbl[*index]->spa = ASIC_LPORT_L3_LAN; 
				RTK_FC_LUT_ADD(fc_db.lutTbl[*index], TRUE); //update entry to hardware
			}
#endif
			if(!fc_db.lutTbl[*index]->isStatic && !fc_db.lutTbl[*index]->isStaticWiFi)
				RTK_FC_LUT_DEL(fc_db.lutTbl[*index], FALSE);
		}
	}

	if(forceLearningPort)
	{
		portmap.macExtPortIdx = macextportidx;
		portmap.macPortIdx = macportidx;
		wlanIdx=  wlanidx;
	}
	DEBUG("Learning Gw Mac %pM",mac);

#if defined(CONFIG_RTK_SOC_RTL8198D)
	ret = _rtk_fc_lut_learning(mac, portmap, 0, wlanIdx,
										FALSE, (TRUE | FORCE_STATIC),index, NULL, LUT_DIR_DONT_CARE);
#else
	ret = _rtk_fc_lut_learning(mac, portmap, 0, wlanIdx,
										FALSE, TRUE,index, NULL, LUT_DIR_DONT_CARE);
#endif

	if(ret!=RTK_FC_RET_OK)
		return ret;

	fc_db.lutTbl[*index]->isGMAC=1;

	return ret;
}

int rtk_fc_devGwMac_update_and_add(struct net_device *dev,int updatePortBySw,int32 *index)
{
	int i;
	int firstFree=FAIL;
	int usingEntry=FAIL;
	int ret;
	int16 lutIdx;
	uint32 gateway_ipv4_addr=0,global_ipv6_addr_set=0;
	struct inet6_dev *in6_dev=NULL;

	*index=DEVIFIDX_INVALID_MIN;

	for(i = DEVIFIDX_VALID_MIN; i < RTK_FC_DEV_GW_MAC_TBL; i++)
	{
		if(fc_db.devGwMacTbl[i].dev==dev)
		{
			usingEntry=i;
			goto UPDATE_GW_MAC;
		}
		if(fc_db.devGwMacTbl[i].dev==NULL && firstFree==FAIL)
			firstFree=i;
	}
	
	if(usingEntry==FAIL && firstFree==FAIL)
		{WARNING("tbl FULl ");	return FAIL;}


	//new etnry 
	dev_hold(dev);
	usingEntry=firstFree;
	memset(&fc_db.devGwMacTbl[usingEntry],0,sizeof(rtk_fc_devGwMac_t));
	fc_db.devGwMacTbl[usingEntry].dev=dev;
	fc_db.devGwMacTbl[usingEntry].hwlutIdx=SIGNED_INVALID;
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	fc_db.devGwMacTbl[usingEntry].myMacIdx=SIGNED_INVALID;
#endif

UPDATE_GW_MAC:
	*index = usingEntry;

	//delete old entry
	if(fc_db.devGwMacTbl[usingEntry].hwlutIdx!=SIGNED_INVALID)
	{
		_rtk_fc_lut_staticEntry_del(fc_db.devGwMacTbl[usingEntry].hwlutIdx, FALSE);
		fc_db.devGwMacTbl[usingEntry].hwlutIdx=SIGNED_INVALID;
	}
#if defined(CONFIG_RTK_L34_G3_PLATFORM)		
	if(fc_db.devGwMacTbl[usingEntry].myMacIdx!=SIGNED_INVALID)
	{
		aal_entry_del(AAL_TABLE_L3_CAM_MAC_DA,fc_db.devGwMacTbl[usingEntry].myMacIdx);
		fc_db.devGwMacTbl[usingEntry].myMacIdx=SIGNED_INVALID;
	}
#endif


	RTK_FC_HELPER_RCU_READ_LOCK();
	RTK_FC_HELPER_RCU_INDEV_GET_IPADDR(dev, &gateway_ipv4_addr);
	RTK_FC_HELPER_RCU_IN6_DEV_GET(dev, &in6_dev);
	if(in6_dev != NULL)
	{
		RTK_FC_HELPER_NETDEV_IFA_GLOBAL_ADDR6_SET(in6_dev, &global_ipv6_addr_set);
	}
	RTK_FC_HELPER_RCU_READ_UNLOCK();

	if(RTK_FC_HOOK_PS_DEV_IS_PRIV_FLAGS_SET(dev, RTK_FC_NETDEV_PRIV_FLAG_TYPE_EBRIDGE))
	{
		DEBUG("Ready to add bridge interface %s to hw netif table. (temp entry)", dev->name);
	}
	else if(RTK_FC_HOOK_PS_DEV_IS_PRIV_FLAGS_SET(dev, RTK_FC_NETDEV_PRIV_FLAG_TYPE_DOMAIN_WAN))
	{
		if(RTK_FC_HELPER_GET_BR_PORT_EXIST(dev))
		{
			struct net_device *br_intf=NULL;
			br_intf = rtk_fc_getBridgedDevbyNetDev(dev);
			if(br_intf)
			{
				DEBUG("WAN interface %s is the member of bridge intf %s, no need to add temp entry", dev->name, br_intf->name);
				return RTK_FC_RET_OK;
			}
		}
		DEBUG("Ready to add WAN interface %s to hw netif table. (temp entry)", dev->name);
	}
	else if((gateway_ipv4_addr!=0 )|| (global_ipv6_addr_set!= 0))
	{
		DEBUG("Ready to add interface %s to hw netif table.", dev->name);
	}
	else
		return RTK_FC_RET_OK;



	if(((dev->dev_addr[0]|dev->dev_addr[1]|dev->dev_addr[2]|dev->dev_addr[3]|dev->dev_addr[4]|dev->dev_addr[5])!=0x0) &&   
		((dev->type==ARPHRD_PPP || (!(dev->flags & UPDATE_IFF_FLAG)))))
	{
#if defined(CONFIG_RTK_L34_G3_PLATFORM)		
		{
			l3_cam_mac_da_tbl_entry_t l3_cam_mac;
			unsigned int camMacIdx;
			memset(&l3_cam_mac, 0x0, sizeof(l3_cam_mac_da_tbl_entry_t));

			l3_cam_mac.mac_0 = dev->dev_addr[0];
			l3_cam_mac.mac_1 = dev->dev_addr[1];
			l3_cam_mac.mac_2 = dev->dev_addr[2];
			l3_cam_mac.mac_3 = dev->dev_addr[3];
			l3_cam_mac.mac_4 = dev->dev_addr[4];
			l3_cam_mac.mac_5 = dev->dev_addr[5];
			l3_cam_mac.vld   = 1;
			
			ret = aal_l3_cam_mac_da_lookup(dev->dev_addr, &camMacIdx);
			if (ret == AAL_E_OK) {
				// increase reference count
				ret = aal_entry_add_by_idx(AAL_TABLE_L3_CAM_MAC_DA, &l3_cam_mac, camMacIdx);
			}else {
				// add a new one
				ret = aal_entry_add(AAL_TABLE_L3_CAM_MAC_DA, &l3_cam_mac, &camMacIdx);
			}
			fc_db.devGwMacTbl[usingEntry].myMacIdx = camMacIdx;
		}
#endif

		{
			int isWanDev=0;
			if(RTK_FC_HOOK_PS_DEV_IS_PRIV_FLAGS_SET(dev, RTK_FC_NETDEV_PRIV_FLAG_TYPE_DOMAIN_WAN))
				isWanDev=1;
			ret = rtk_fc_gwMac_lut_add(dev->dev_addr,&lutIdx,isWanDev,updatePortBySw,fc_db.devGwMacTbl[usingEntry].macportidx,fc_db.devGwMacTbl[usingEntry].macextportidx,fc_db.devGwMacTbl[usingEntry].wlanidx);
			if(ret!=RTK_FC_RET_OK)
			{
				fc_db.devGwMacTbl[usingEntry].hwlutIdx=SIGNED_INVALID;
				return ret;
			}
			fc_db.devGwMacTbl[usingEntry].hwlutIdx = lutIdx;
			if(updatePortBySw)
				fc_db.devGwMacTbl[usingEntry].forcePortConfig=1;
		}

	}

#ifdef CONFIG_RTK_SOC_RTL8198D
#ifdef CONFIG_OPENWRT_SDK
	if (!strncmp(dev->name, "br-lan", IFNAMSIZ))
#else
	if (!strncmp(dev->name, "br0", IFNAMSIZ))
#endif
	{
		int ip_addr;
		RTK_FC_HELPER_RCU_INDEV_GET_IPADDR(dev, &ip_addr);
		RTK_FC_HELPER_MGR_BR0_IP_MAC_SET(ip_addr, dev->dev_addr, 0U);
	}
#endif

	return SUCCESS;
}

//if not found return DEVIFIDX_INVALID_MIN
int rtk_fc_devGwMacIdx_get(struct net_device *dev)
{
	int getEntryIdx=FAIL;
	int i;
	for(i = DEVIFIDX_VALID_MIN; i < RTK_FC_DEV_GW_MAC_TBL; i++)
	{
		if(fc_db.devGwMacTbl[i].dev==dev)
		{
			getEntryIdx=i;
			break;
		}
	}
	
	if(getEntryIdx==FAIL )
		{TABLE("[%s]entry not found ",dev->name);	return DEVIFIDX_INVALID_MIN;}
	else
		return getEntryIdx;
}



int rtk_fc_devGwMac_del(struct net_device *dev)
{
	int usingEntry=FAIL;
	int i;
	for(i = DEVIFIDX_VALID_MIN; i < RTK_FC_DEV_GW_MAC_TBL; i++)
	{
		if(fc_db.devGwMacTbl[i].dev==dev)
		{
			usingEntry=i;
			break;
		}
	}
	
	if(usingEntry==FAIL )
		{TABLE("[%s]entry not found ",dev->name);	return FAIL;}

	rtk_fc_netif_sw_del_by_fcDevIdx(usingEntry);

	//delete lut/myMac entry
	if(fc_db.devGwMacTbl[usingEntry].hwlutIdx!=SIGNED_INVALID)
	{
		_rtk_fc_lut_staticEntry_del(fc_db.devGwMacTbl[usingEntry].hwlutIdx, FALSE);
		fc_db.devGwMacTbl[usingEntry].hwlutIdx=SIGNED_INVALID;
	}
#if defined(CONFIG_RTK_L34_G3_PLATFORM)		
	if(fc_db.devGwMacTbl[usingEntry].myMacIdx!=SIGNED_INVALID)
	{
		aal_entry_del(AAL_TABLE_L3_CAM_MAC_DA,fc_db.devGwMacTbl[usingEntry].myMacIdx);
		fc_db.devGwMacTbl[usingEntry].myMacIdx=SIGNED_INVALID;
	}
#endif
	dev_put(dev);
	fc_db.devGwMacTbl[usingEntry].dev=NULL;


#ifdef CONFIG_RTK_SOC_RTL8198D
#ifdef CONFIG_OPENWRT_SDK
	if (!strncmp(dev->name, "br-lan", IFNAMSIZ))
#else
	if (!strncmp(dev->name, "br0", IFNAMSIZ))
#endif
	{
		uint8 mac_addr[ETHER_ADDR_LEN] = {0};
		RTK_FC_HELPER_MGR_BR0_IP_MAC_SET(0U, mac_addr, 0U);
	}
#endif

	return SUCCESS;


}



static int _rtk_fc_netif_init(void)
{
	int i;

	for(i = 0; i < RTK_FC_TABLESIZE_INTF_SW; i++)
	{
		//init netif table
		bzero(&fc_db.netifTbl[i],sizeof(fc_db.netifTbl[i]));
	
		fc_db.netifTbl[i].lutIdx = SIGNED_INVALID;
		fc_db.netifTbl[i].hwIdx = SIGNED_INVALID;
		fc_db.netifTbl[i].pppoeGwLutIdx = SIGNED_INVALID; 
		fc_db.netifTbl[i].outerHdrFlowIdx = SIGNED_INVALID; 
		fc_db.netifTbl[i].outerHdrExtratagIdx = SIGNED_INVALID; 
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		fc_db.netifTbl[i].dualHdr_ds_clsIdx = CA_UINT32_INVALID;
		fc_db.netifTbl[i].dualHdr_ds_extra_clsIdx = CA_UINT32_INVALID;
		fc_db.netifTbl[i].hwFmrIdx = SIGNED_INVALID;
#endif
	
		//the fixed value
		fc_db.netifTbl[i].intf.ingress_action = FB_ACTION_FORWARD;
		fc_db.netifTbl[i].intf.egress_action = FB_ACTION_FORWARD;
		fc_db.netifTbl[i].intf.deny_ipv4 = 0;
		fc_db.netifTbl[i].intf.deny_ipv6 = 0;
		fc_db.netifTbl[i].intf.allow_ingress_portmask.bits[0]=0;
		fc_db.netifTbl[i].intf.allow_ingress_ext_portmask.bits[0]=0;
		fc_db.netifTbl[i].intf.out_pppoe_act = FB_NETIFPPPOE_ACT_REMOVE;
		fc_db.netifTbl[i].intf.intf_mtu_check = 1U;
		fc_db.netifTbl[i].hwEntryNum = 1U;

	}

	for(i = 0; i < RTK_FC_TABLESIZE_INTF; i++)
	{
		RTK_RG_ASIC_NETIFTABLE_DEL(i); //clear HW and SW fields
	}

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	rtk_rg_asic_table_reset(FB_RST_IF_TBL);
#endif

	return SUCCESS;
}

// reset exist hw entry
static int _rtk_fc_netif_reset(void)
{
	int i;

	for(i = 0; i < RTK_FC_TABLESIZE_INTF_SW; i++)
	{
		if(fc_db.netifTbl[i].intf.valid)
			rtk_fc_netif_sw_del(i);
	}

	for(i = 0; i < RTK_FC_TABLESIZE_INTF; i++)
	{
		RTK_RG_ASIC_NETIFTABLE_DEL(i); //clear HW and SW fields
	}

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	rtk_rg_asic_table_reset(FB_RST_IF_TBL);
#endif

	return SUCCESS;
}


int rtk_fc_tableReset(void)
{

#if !(defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)) // 8277C/9607F no need indMac, pFlowPath5->out_dmac_idx is meaningless
	_rtk_fc_indMac_addr_init();
#endif
	_rtk_fc_extPMask_init();

	_rtk_fc_devGwMac_reset();
	_rtk_fc_netif_reset();

	return SUCCESS;
}

int rtk_fc_tables_init(void)
{
#if !(defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)) // 8277C/9607F no need indMac, pFlowPath5->out_dmac_idx is meaningless
	_rtk_fc_indMac_addr_init();
#endif
	_rtk_fc_extPMask_init();

	_rtk_fc_devGwMac_init();
	_rtk_fc_netif_init();

	return SUCCESS;
}


#include <linux/if_pppox.h>

int rtk_fc_hwNetif_add_update_by_swNetif(uint8 swEntIdx)
{

	int i = 0,hwIdx=SIGNED_INVALID;
	rtk_fc_ret_t ret = RTK_FC_RET_ERR;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	int netif_idx_offset, netif_search_start_idx;
	uint8 hwEntryNum=1;
#endif

	if(swEntIdx>=RTK_FC_TABLESIZE_INTF_SW)
	{
		WARNING("FIXME: try to write sw netif[%d]", swEntIdx);
		goto RET_FAILED;
	}

	if(fc_db.netifTbl[swEntIdx].intf.valid!=TRUE )
	{
		WARNING("config was incomplete netif[%d]:invalid ",swEntIdx);
		goto RET_FAILED;
	}

	if(fc_db.netifTbl[swEntIdx].hwIdx!=SIGNED_INVALID)
	{
		//update
		if((fc_db.netifHwTblVaild[fc_db.netifTbl[swEntIdx].hwIdx].hwNetifValid==0) || fc_db.netifHwTblVaild[fc_db.netifTbl[swEntIdx].hwIdx].swNetifIdx!=swEntIdx)
			{WARNING("netif[%d] unexpect Error",swEntIdx);goto RET_FAILED;}

		for(i=0;i<fc_db.netifTbl[swEntIdx].hwEntryNum;i++)
		{
			ret=RTK_RG_ASIC_NETIFTABLE_ADD(fc_db.netifTbl[swEntIdx].hwIdx+i, &(fc_db.netifTbl[swEntIdx].intf));
			if(ret != RTK_FC_RET_OK){
				WARNING("netif update fail");
				goto RET_FAILED;
			}
			else{
				TABLE("update sw intf[%d] to hw netif[%d] ", swEntIdx, fc_db.netifTbl[swEntIdx].hwIdx+i);
			}

		}
		return SUCCESS;

	}
	else
	{
		//add new
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		netif_search_start_idx = (fc_db.netifTbl[swEntIdx].dualType==RTK_FC_DUALTYPE_NONE) ? RTK_FC_NETIF_START_IDX : RTK_FC_DUAL_NETIF_START_IDX;	
		hwEntryNum = fc_db.netifTbl[swEntIdx].hwEntryNum;
		
#if defined(CONFIG_FC_RTL8277C_SERIES)// for HW issue of 8277C interface index 48 & dual buffer length >64	
		if(fc_db.netifTbl[swEntIdx].dualType!=RTK_FC_DUALTYPE_NONE && hwEntryNum>1)
		{
			netif_search_start_idx = RTK_FC_DUAL_NETIF_START_IDX + 2;
		}
#endif

		switch(hwEntryNum)
		{
			case 1:
			case 2:	
				for(i=netif_search_start_idx; i<RTK_FC_TABLESIZE_INTF; i+=hwEntryNum)
				{
					// find first consecutive n free netif entry
					if(hwIdx==SIGNED_INVALID)
					{
						for(netif_idx_offset=0; netif_idx_offset<hwEntryNum; netif_idx_offset++)
						{
							if(fc_db.netifHwTblVaild[ i + netif_idx_offset].hwNetifValid)
								break;
						}
						if(netif_idx_offset==hwEntryNum)
						{
							hwIdx = i;
						}
					}
				}
				break;
			default:
				WARNING("Do not support hwEntryNum=%d, swNetifIdx=%d", hwEntryNum, swEntIdx);
				goto RET_FAILED;
		}	
#else
		for(i=RTK_FC_NETIF_START_IDX; i<RTK_FC_TABLESIZE_INTF; i++)
		{
			// find first free netif entry
			if((fc_db.netifHwTblVaild[i].hwNetifValid == FALSE) && (hwIdx==SIGNED_INVALID))
			{
				hwIdx = i;
			}
		}
#endif		

		if(hwIdx==SIGNED_INVALID)
		{
			WARNING("hw netif table was full");
			goto RET_FAILED;
		}


		ret = RTK_RG_ASIC_NETIFTABLE_ADD(hwIdx, &(fc_db.netifTbl[swEntIdx].intf));

		if(ret != RTK_FC_RET_OK){
			fc_db.netifTbl[swEntIdx].hwIdx = SIGNED_INVALID;
	 		RTK_RG_ASIC_NETIFTABLE_DEL(hwIdx);
			goto RET_FAILED;
		}
		else{
			TABLE("Add sw intf[%d] to hw netif[%d] ", swEntIdx, hwIdx );
		}

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		for(netif_idx_offset=1; netif_idx_offset<hwEntryNum; netif_idx_offset++)
		{		
			int hwNetifIdx = hwIdx + netif_idx_offset;

			// using sw netif intf to add hw
			ret = RTK_RG_ASIC_NETIFTABLE_ADD(hwNetifIdx, &(fc_db.netifTbl[swEntIdx].intf));

			if(ret != RTK_FC_RET_OK){

				fc_db.netifTbl[swEntIdx].hwIdx = SIGNED_INVALID;

				//recover netif
				for(i=hwNetifIdx; i>=hwIdx; i--)
				{
					RTK_RG_ASIC_NETIFTABLE_DEL(i);
				}

				WARNING("add sw netif[%d] failed: %x\n", swEntIdx, ret);
				goto RET_FAILED;
			}
			else{
				TABLE("[Dual virtual netif] Add sw intf[%d] to hw netif[%d] ", swEntIdx, hwNetifIdx);

			}

			fc_db.netifHwTblVaild[hwNetifIdx].hwNetifValid=TRUE;
			fc_db.netifHwTblVaild[hwNetifIdx].swNetifIdx = swEntIdx;

			
		}
#endif

		/*reserved ACL*/
		_rtk_fc_aclAndCfReservedRule_intfRsvUpdate(TRUE, swEntIdx);


		fc_db.netifHwTblVaild[hwIdx].hwNetifValid=TRUE;
		fc_db.netifHwTblVaild[hwIdx].swNetifIdx = swEntIdx;

		fc_db.netifTbl[swEntIdx].hwIdx = hwIdx;

		return ret;
	}
RET_FAILED:
	return FAILED;



}

int rtk_fc_hwNetif_del_by_swNetif(int swEntIdx)
{


	if(fc_db.netifTbl[swEntIdx].hwIdx!=SIGNED_INVALID)
	{
		rtk_fc_flow_delete_by_intfIdx(fc_db.netifTbl[swEntIdx].hwIdx);
		fc_db.netifHwTblVaild[fc_db.netifTbl[swEntIdx].hwIdx].hwNetifValid=0;
		fc_db.netifHwTblVaild[fc_db.netifTbl[swEntIdx].hwIdx].swNetifIdx=0;
		RTK_RG_ASIC_NETIFTABLE_DEL(fc_db.netifTbl[swEntIdx].hwIdx);
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
{
		int netif_idx_offset;
		for(netif_idx_offset=1; netif_idx_offset<fc_db.netifTbl[swEntIdx].hwEntryNum; netif_idx_offset++)
		{
			RTK_RG_ASIC_NETIFTABLE_DEL(fc_db.netifTbl[swEntIdx].hwIdx + netif_idx_offset);
		}
}
#endif	
		if(fc_db.netifTbl[swEntIdx].dualType)
		{
			/*	clear:dual hw config
				outerHdrFlowIdx/dualHdr_ds_clsIdx/dualHdr_ds_extra_clsIdx/hwFmrIdx
			*/			
			rtk_fc_netif_dualConfig_del(swEntIdx);
		}
	}
	
	return SUCCESS;
}

int rtk_fc_netif_update_by_flow(uint8 swEntIdx,rtk_fc_pktHdr_t *pPktHdr, struct sk_buff *skb, rtk_fc_flow_direction_t dir,rtk_fc_dev_type_t devType)
{
	int stackingCount;
	struct net_device *p_PScacheDev[DEV_STACK_MAX]={NULL,NULL,NULL,NULL};
	rtk_fc_tableNetif_t *pNetif = &fc_db.netifTbl[swEntIdx];
	uint8 *pNetifMac = NULL;
	int ret;
	int updateHwNetif=0;
		
	if(!fc_db.netifTbl[swEntIdx].intf.valid)
		{WARNING("invalid netif[%d]",swEntIdx); return FAILED;}

	pNetifMac = &fc_db.netifTbl[swEntIdx].intf.gateway_mac_addr.octet[0];

	stackingCount=rtk_fc_psIfidxStackKey_to_dev(fc_db.netifTbl[swEntIdx].psIfidxStackKey,p_PScacheDev);

	/* only valid from 1 ~ 3 */
	if(stackingCount<=0 || DEV_STACK_MAX<=stackingCount || p_PScacheDev[0]==NULL || p_PScacheDev[stackingCount-1]==NULL)
		{WARNING("invalid stackingCount[%d]",stackingCount); return FAILED;}

	/*
		========================================================================================================
		setting netif by net_device 
		========================================================================================================
	*/

	//update mtu by device using first dev (expect is dual-dev)
	//(dual)user configuration should minus dual-hdr N bytes by itself. so FC change to follow interfce original setting.
	if(fc_db.netifTbl[swEntIdx].intf.intf_mtu_check)
	{
		if(fc_db.netifTbl[swEntIdx].intf.intf_mtu!=0 )
		{
			if(fc_db.netifTbl[swEntIdx].intf.intf_mtu!=p_PScacheDev[0]->mtu)
			{
				WARNING("netif[%d] Mtu from %d to %d by %s ",swEntIdx,fc_db.netifTbl[swEntIdx].intf.intf_mtu,p_PScacheDev[0]->mtu,p_PScacheDev[0]->name);
				fc_db.netifTbl[swEntIdx].intf.intf_mtu = p_PScacheDev[0]->mtu;
				updateHwNetif=1;
			}
		}
		else if(p_PScacheDev[0]->mtu!=0)
		{
			TRACE("assign netif[%d] mtu:%d by %s",swEntIdx,p_PScacheDev[0]->mtu,p_PScacheDev[0]->name);
			fc_db.netifTbl[swEntIdx].intf.intf_mtu = p_PScacheDev[0]->mtu;
			updateHwNetif=1;
		}
	}

	//update gateway mac by device
	if( (!(p_PScacheDev[stackingCount-1]->flags&UPDATE_IFF_FLAG))  &&  p_PScacheDev[stackingCount-1]->dev_addr!=NULL)
	{
		if(((pNetifMac[0]|pNetifMac[1]|pNetifMac[2]|pNetifMac[3]|pNetifMac[4]|pNetifMac[5] )!=0x0)  )
		{
			if(memcmp(pNetifMac,p_PScacheDev[stackingCount-1]->dev_addr,6)!=0)
			{
				if( !(pPktHdr->dualHdrType == RTK_FC_DUALTYPE_VXLAN || (skb->fcIngressData.ingressTagif&VXLAN_TAGIF)) )
					WARNING("netif[%d] gateway mac change from %pM to %pM By Dev:%s by dev",swEntIdx,pNetifMac,p_PScacheDev[stackingCount-1]->dev_addr,p_PScacheDev[stackingCount-1]->name);
				else
					DEBUG("netif[%d] gateway mac change from %pM to %pM By Dev:%s by dev",swEntIdx,pNetifMac,p_PScacheDev[stackingCount-1]->dev_addr,p_PScacheDev[stackingCount-1]->name);
				memcpy(pNetifMac, p_PScacheDev[stackingCount-1]->dev_addr, 6);
				updateHwNetif=1;
			}			
		}
		else
		{
			TRACE("netif[%d] assign gateway mac %pM by device[%s]",swEntIdx,p_PScacheDev[stackingCount-1]->dev_addr,p_PScacheDev[stackingCount-1]->name);
			memcpy(pNetifMac, p_PScacheDev[stackingCount-1]->dev_addr, 6);
			updateHwNetif=1;
		}
	}

	//update ipv4 address by device
	{
		int ipAddr = 0;
		RTK_FC_HELPER_RCU_INDEV_GET_IPADDR(p_PScacheDev[stackingCount-1], &ipAddr);
		//only assign first time ,we like update ipv4 addr by packets
		if(fc_db.netifTbl[swEntIdx].intf.gateway_ipv4_addr==0 && ipAddr!=0)
		{
			TRACE("assign netif[%d] ipv4Addr:%pI4h",swEntIdx,&ipAddr);
			fc_db.netifTbl[swEntIdx].intf.gateway_ipv4_addr = ipAddr;
			updateHwNetif=1;
		}
	}



	/*
		========================================================================================================
		setting netif by packets
		========================================================================================================
	*/
	//udpate lan/wan ineterface gatewaymac by packets (smac/dmac change ,routing case)
	if( !(skb->fcIngressData.da[0]&1) && !(pPktHdr->eth->h_dest[0]&1) && memcmp(pPktHdr->eth->h_dest,skb->fcIngressData.da,6) && devType==RTK_FC_DEV_TYPE_INGRESS)
	{
		//not mc/bc  dmac change update ingress netif
		if( (pNetifMac[0]|pNetifMac[1]|pNetifMac[2]|pNetifMac[3]|pNetifMac[4]|pNetifMac[5] )!=0x0 )
		{
			if(memcmp(pNetifMac,skb->fcIngressData.da,6)!=0)
			{
				if( !(pPktHdr->dualHdrType == RTK_FC_DUALTYPE_VXLAN || (skb->fcIngressData.ingressTagif&VXLAN_TAGIF)) )
					WARNING("netif[%d] gateway mac change from %pM to %pM by pkt",swEntIdx,pNetifMac,skb->fcIngressData.da);
				else
					DEBUG("netif[%d] gateway mac change from %pM to %pM by pkt",swEntIdx,pNetifMac,skb->fcIngressData.da);
				if(fc_db.netifTbl[swEntIdx].lutIdx != SIGNED_INVALID)
				{
					_rtk_fc_lut_staticEntry_del(fc_db.netifTbl[swEntIdx].lutIdx, FALSE);
					fc_db.netifTbl[swEntIdx].lutIdx = SIGNED_INVALID;
				}
				memcpy(fc_db.netifTbl[swEntIdx].intf.gateway_mac_addr.octet,skb->fcIngressData.da,6);
				updateHwNetif=1;
			}	
		}
		else
		{
			TRACE("netif[%d] gateway mac assign %pM by ingress packets DA",swEntIdx,skb->fcIngressData.da);
			memcpy(fc_db.netifTbl[swEntIdx].intf.gateway_mac_addr.octet,skb->fcIngressData.da,6);
			updateHwNetif=1;
		}
	}
	else if( !(skb->fcIngressData.sa[0]&1) && !(pPktHdr->eth->h_source[0]&1) && memcmp(pPktHdr->eth->h_source,skb->fcIngressData.sa,6) && devType==RTK_FC_DEV_TYPE_EGRESS)
	{
		//not mc/bc smac change update egress netif
		if( (pNetifMac[0]|pNetifMac[1]|pNetifMac[2]|pNetifMac[3]|pNetifMac[4]|pNetifMac[5] )!=0x0 )
		{
			if(memcmp(pNetifMac,pPktHdr->eth->h_source,6)!=0)
			{
				if( !(pPktHdr->dualHdrType == RTK_FC_DUALTYPE_VXLAN || (skb->fcIngressData.ingressTagif&VXLAN_TAGIF)) )
					WARNING("netif[%d] gateway mac change from %pM to %pM by pkt",swEntIdx,pNetifMac,pPktHdr->eth->h_source);
				else
					DEBUG("netif[%d] gateway mac change from %pM to %pM by pkt",swEntIdx,pNetifMac,pPktHdr->eth->h_source);
				if(fc_db.netifTbl[swEntIdx].lutIdx != SIGNED_INVALID)
				{
					_rtk_fc_lut_staticEntry_del(fc_db.netifTbl[swEntIdx].lutIdx, FALSE);
					fc_db.netifTbl[swEntIdx].lutIdx = SIGNED_INVALID;
				}
				memcpy(fc_db.netifTbl[swEntIdx].intf.gateway_mac_addr.octet,pPktHdr->eth->h_source,6);
				updateHwNetif=1;
			}
		}
		else
		{
			TRACE("netif[%d] gateway mac assign %pM by egress packets SA",swEntIdx,pPktHdr->eth->h_source);
			memcpy(fc_db.netifTbl[swEntIdx].intf.gateway_mac_addr.octet,pPktHdr->eth->h_source,6);
			updateHwNetif=1;
		}
	}


	//Update Wan netif PPPoE SessionId by packets (intf.out_pppoe_sid/out_pppoe_act)
	if(dir==RTK_FC_FLOW_DIRECTION_UPSTREAM && devType==RTK_FC_DEV_TYPE_EGRESS && !(skb->fcIngressData.ingressTagif&PPPOE_TAGIF) && pPktHdr->ppph)
	{
		fc_db.netifTbl[swEntIdx].intf.out_pppoe_act = FB_NETIFPPPOE_ACT_ADD;
		if(fc_db.netifTbl[swEntIdx].intf.out_pppoe_sid !=0 )
		{
			if(fc_db.netifTbl[swEntIdx].intf.out_pppoe_sid!=ntohs(pPktHdr->ppph->sid))
			{
				WARNING("netif[%d] PPPoE sessionid change from %d to %d",swEntIdx,fc_db.netifTbl[swEntIdx].intf.out_pppoe_sid,ntohs(pPktHdr->ppph->sid) );
				fc_db.netifTbl[swEntIdx].intf.out_pppoe_sid = ntohs(pPktHdr->ppph->sid);		
				updateHwNetif=1;
			}
		}
		else if(ntohs(pPktHdr->ppph->sid) != 0)
		{
			TRACE("netif[%d] PPPoE sessionid[%d] assign by egress packets",swEntIdx,ntohs(pPktHdr->ppph->sid) );
			fc_db.netifTbl[swEntIdx].intf.out_pppoe_sid = ntohs(pPktHdr->ppph->sid) ;	
			updateHwNetif=1;
		}
		
		if(pPktHdr->dmacL2Idx !=FAIL)
		{
			if(fc_db.netifTbl[swEntIdx].pppoeGwLutIdx != FAIL)
			{
				if(fc_db.netifTbl[swEntIdx].pppoeGwLutIdx != pPktHdr->dmacL2Idx)
				{
					WARNING("PPPoE packet's dmacL2Idx:%d is diffenent from its pppoeGwLutIdx(%d), update it", fc_db.netifTbl[swEntIdx].pppoeGwLutIdx,pPktHdr->smacL2Idx);
					fc_db.netifTbl[swEntIdx].pppoeGwLutIdx = pPktHdr->dmacL2Idx;
				}
			}
			else
			{
				fc_db.netifTbl[swEntIdx].pppoeGwLutIdx = pPktHdr->dmacL2Idx;
				TRACE("Update pppoeGwLutIdx %d to PPPoE interface (sw interface index: %d)", pPktHdr->dmacL2Idx, swEntIdx);
			}
		}
		
	}
	else if(dir==RTK_FC_FLOW_DIRECTION_DOWNSTREAM     && devType==RTK_FC_DEV_TYPE_INGRESS && (skb->fcIngressData.ingressTagif&PPPOE_TAGIF) && !pPktHdr->ppph)
	{
		fc_db.netifTbl[swEntIdx].intf.out_pppoe_act = FB_NETIFPPPOE_ACT_ADD;
		
		if(fc_db.netifTbl[swEntIdx].intf.out_pppoe_sid !=0 )
		{
			if(fc_db.netifTbl[swEntIdx].intf.out_pppoe_sid!=skb->fcIngressData.sessionId)
			{
				WARNING("netif[%d] PPPoE sessionid change from %d to %d",swEntIdx,fc_db.netifTbl[swEntIdx].intf.out_pppoe_sid,skb->fcIngressData.sessionId);
				fc_db.netifTbl[swEntIdx].intf.out_pppoe_sid = skb->fcIngressData.sessionId;
				updateHwNetif=1;
			}
		}
		else if(skb->fcIngressData.sessionId != 0)
		{
			TRACE("netif[%d] PPPoE sessionid[%d] assign by ingress packets",swEntIdx,skb->fcIngressData.sessionId );
			fc_db.netifTbl[swEntIdx].intf.out_pppoe_sid = skb->fcIngressData.sessionId;	
			updateHwNetif=1;
		}
		if(pPktHdr->smacL2Idx !=FAIL)
		{
			if(fc_db.netifTbl[swEntIdx].pppoeGwLutIdx != FAIL)
			{
				if(fc_db.netifTbl[swEntIdx].pppoeGwLutIdx != pPktHdr->smacL2Idx)
				{
					WARNING("PPPoE packet's smacL2Idx:%d is diffenent from its pppoeGwLutIdx(%d), update it", fc_db.netifTbl[swEntIdx].pppoeGwLutIdx,pPktHdr->smacL2Idx);
					fc_db.netifTbl[swEntIdx].pppoeGwLutIdx = pPktHdr->smacL2Idx;
				}
			}
			else
			{
				fc_db.netifTbl[swEntIdx].pppoeGwLutIdx = pPktHdr->smacL2Idx;
				TRACE("Update pppoeGwLutIdx %d to PPPoE interface (sw interface index: %d)", pPktHdr->smacL2Idx, swEntIdx);
			}
		}		
	}

#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	//Update Vxd client mode netif PPPoE action - keep.
	if(dir==RTK_FC_FLOW_DIRECTION_UPSTREAM && 
			devType==RTK_FC_DEV_TYPE_EGRESS && 
			pPktHdr->egrWlanDevIdx <= RTK_FC_WLANX_END_INTF &&
			RTK_FC_HELPER_WLAN_IS_CLIENT_MODE(pPktHdr->egrWlanDevIdx)) {
		fc_db.netifTbl[swEntIdx].intf.out_pppoe_act = FB_NETIFPPPOE_ACT_KEEP;
		DEBUG("egress to wlandevidx %d sw netif idx %d, set pppoe act as keep because of client mode.", pPktHdr->egrWlanDevIdx, swEntIdx);
	}
	if(dir==RTK_FC_FLOW_DIRECTION_DOWNSTREAM && 
			devType==RTK_FC_DEV_TYPE_INGRESS &&
			pPktHdr->igrWlanDevIdx <= RTK_FC_WLANX_END_INTF &&
			RTK_FC_HELPER_WLAN_IS_CLIENT_MODE(pPktHdr->igrWlanDevIdx)) {
		fc_db.netifTbl[swEntIdx].intf.out_pppoe_act = FB_NETIFPPPOE_ACT_KEEP;
		DEBUG("ingress from wlandevidx %d sw netif idx %d, set pppoe act as keep because of client mode.", pPktHdr->igrWlanDevIdx, swEntIdx);
	}
#endif


	//update ipv4 address by packets
	if(dir==RTK_FC_FLOW_DIRECTION_UPSTREAM&& (pPktHdr->fwdType==RTK_FC_FWDTYPE_NAPT || pPktHdr->fwdType==RTK_FC_FWDTYPE_NATLOOPBACK) && devType==RTK_FC_DEV_TYPE_EGRESS && pPktHdr->iph &&(skb->fcIngressData.srcIp!=ntohl(pPktHdr->iph->saddr)))
	{
		uint32 ipAddr = ntohl(pPktHdr->iph->saddr);
		if(fc_db.netifTbl[swEntIdx].intf.gateway_ipv4_addr!=0 )
		{
			if(fc_db.netifTbl[swEntIdx].intf.gateway_ipv4_addr!=ipAddr)
			{
				if(pPktHdr->dualHdrType != RTK_FC_DUALTYPE_NONE)	
					TABLE("netif[%d] gw Ipv4 address change form %pI4h to %pI4h",swEntIdx,&fc_db.netifTbl[swEntIdx].intf.gateway_ipv4_addr,&ipAddr);
				else
					WARNING("netif[%d] gw Ipv4 address change form %pI4h to %pI4h",swEntIdx,&fc_db.netifTbl[swEntIdx].intf.gateway_ipv4_addr,&ipAddr);
				fc_db.netifTbl[swEntIdx].intf.gateway_ipv4_addr = ipAddr;
				updateHwNetif=1;
			}
		}
		else
		{
			TRACE("assign netif[%d] ipv4Addr:%pI4h by egress packet",swEntIdx,&ipAddr);
			fc_db.netifTbl[swEntIdx].intf.gateway_ipv4_addr = ipAddr;
			updateHwNetif=1;
		}

	}
	else if(dir==RTK_FC_FLOW_DIRECTION_DOWNSTREAM  && (pPktHdr->fwdType==RTK_FC_FWDTYPE_NAPT || pPktHdr->fwdType==RTK_FC_FWDTYPE_NATLOOPBACK) && devType==RTK_FC_DEV_TYPE_INGRESS && pPktHdr->iph && (skb->fcIngressData.dstIp!=ntohl(pPktHdr->iph->daddr)))
	{
		uint32 ipAddr = skb->fcIngressData.dstIp;
		if(fc_db.netifTbl[swEntIdx].intf.gateway_ipv4_addr!=0 )
		{
			if(fc_db.netifTbl[swEntIdx].intf.gateway_ipv4_addr!=ipAddr)
			{
				if(skb->fcIngressData.ingressTagif&(PPTP_TAGIF|L2TP_TAGIF|VXLAN_TAGIF))
					TABLE("netif[%d] gw Ipv4 address change form %pI4h to %pI4h",swEntIdx,&fc_db.netifTbl[swEntIdx].intf.gateway_ipv4_addr,&ipAddr);
				else
					WARNING("netif[%d] gw Ipv4 address change form %pI4h to %pI4h",swEntIdx,&fc_db.netifTbl[swEntIdx].intf.gateway_ipv4_addr,&ipAddr);
				fc_db.netifTbl[swEntIdx].intf.gateway_ipv4_addr = ipAddr;
				updateHwNetif=1;
			}
		}
		else
		{
			TRACE("assign netif[%d] ipv4Addr:%pI4h by egress packet",swEntIdx,&ipAddr);
			fc_db.netifTbl[swEntIdx].intf.gateway_ipv4_addr = ipAddr;
			updateHwNetif=1;
		}		
	}


	/*
		update downstream ingress and upstream egress netif (wan netif)
	*/
	if( (dir==RTK_FC_FLOW_DIRECTION_UPSTREAM && devType==RTK_FC_DEV_TYPE_EGRESS && !fc_db.netifTbl[swEntIdx].dualUniInfo_syncedUP) ||
		(dir==RTK_FC_FLOW_DIRECTION_DOWNSTREAM && devType==RTK_FC_DEV_TYPE_INGRESS && !fc_db.netifTbl[swEntIdx].dualUniInfo_syncedDOWN))
	{
		rtk_fc_netif_dualinfo_setting(devType,dir,swEntIdx,skb,pPktHdr,stackingCount,p_PScacheDev);
	}



/*
	========================================================================================================
	setting hardward entry
	========================================================================================================
*/
	if(fc_db.netifTbl[swEntIdx].lutIdx==SIGNED_INVALID)
	{
		int isWanGw=0;
		if( (dir==RTK_FC_FLOW_DIRECTION_UPSTREAM     && devType==RTK_FC_DEV_TYPE_EGRESS) || (dir==RTK_FC_FLOW_DIRECTION_DOWNSTREAM     && devType==RTK_FC_DEV_TYPE_INGRESS))		
		{
			isWanGw=1;
		}
		ret = rtk_fc_gwMac_lut_add(fc_db.netifTbl[swEntIdx].intf.gateway_mac_addr.octet,&fc_db.netifTbl[swEntIdx].lutIdx,isWanGw,0,0,0,0);
		if(ret!=RTK_FC_RET_OK)
			{WARNING("add netifgwMac lut fail ret=%d",ret);goto  NETIF_FAIL;}
	}

	if(fc_db.netifTbl[swEntIdx].hwIdx==SIGNED_INVALID || updateHwNetif)
	{
		//assign fc_db.netifTbl[swEntIdx].hwIdx in the function
		ret = rtk_fc_hwNetif_add_update_by_swNetif(swEntIdx);
		if(ret!=RTK_FC_RET_OK)
			{WARNING("add netifgwMac lut fail ret=%d",ret);goto  NETIF_FAIL;}
	}
	
	if(fc_db.netifTbl[swEntIdx].hwIdx!=SIGNED_INVALID)
		_rtk_fc_netif_dummy_packet_set(pNetif->hwIdx); //initial for PPPoE dummy packet



	

	return SUCCESS;
NETIF_FAIL:

	if(fc_db.netifTbl[swEntIdx].lutIdx!=SIGNED_INVALID)
	{
		RTK_FC_LUT_DEL(fc_db.lutTbl[pNetif->lutIdx], FALSE);
		fc_db.netifTbl[swEntIdx].lutIdx=SIGNED_INVALID;
	}
	
	if(fc_db.netifTbl[swEntIdx].hwIdx!=SIGNED_INVALID)
	{
		rtk_fc_netif_dummy_packet_clear(pNetif->hwIdx);
		rtk_fc_hwNetif_del_by_swNetif(swEntIdx);
		fc_db.netifTbl[swEntIdx].hwIdx=SIGNED_INVALID;
	}
	
	
	return FAILED;
}

void rtk_fc_netif_port_info_set(int swEntIdx, char macportidx, char macextportidx, char wlanidx)
{
	fc_db.devGwMacTbl[swEntIdx].macportidx= macportidx;
	fc_db.devGwMacTbl[swEntIdx].macextportidx= macextportidx;
	fc_db.devGwMacTbl[swEntIdx].wlanidx= wlanidx;
}





int rtk_fc_eventHandler_inetAddr(void *ptr, unsigned long event)
{
	int ret = RTK_FC_RET_OK;
	struct net_device *dev = NULL;
	int ipAddr = 0;
	unsigned int mtu;
	
	RTK_FC_HELPER_RCU_READ_LOCK();

	RTK_FC_HELPER_NETDEV_IFA_TO_DEV(ptr, &dev);
	RTK_FC_HELPER_RCU_INDEV_GET_IPADDR(dev, &ipAddr);
	RTK_FC_HELPER_RCU_READ_UNLOCK();

	if(unlikely(fc_db.rtk_fc_mgr_init==0)) return RTK_FC_RET_OK;

	if(unlikely(fc_db.controlFuc.hwnat_mode==RT_FLOW_HWNAT_MODE_SPEC_TEST)) return RTK_FC_RET_OK;

	if(RTK_FC_HELPER_NETDEV_IGNORE_LOOKUP(dev,event)) return RTK_FC_RET_OK;

	EVENT("[INETADDR EVENT] netdevice:%s event:%lu\n", dev->name, event);

	switch (event) {
		case NETDEV_UP:
			break;
		default:
			return RTK_FC_RET_OK;
	}

	EVENT("netdev:%s ip:%pI4h", dev->name, (&ipAddr));

#ifdef CONFIG_SYSCTL // disable netfilter checksum checking
	{
		RTK_FC_HELPER_NETDEV_SET_NETFILTER_CHECKSUM(&dev);
		/*
		struct net *net = dev_net(dev);
		net->ct.sysctl_checksum = 0;
		*/
	}
#endif

	RTK_FC_HELPER_NETDEV_GET_DEV_DATA(dev, &mtu, RTK_FC_NETDEV_GET_MTU);

	EVENT("devname = %s, ip = %pI4h, mtu = %d", dev->name, &ipAddr, mtu);	
		
	return ret;

}

int rtk_fc_eventHandler_inet6Addr(void *ptr, unsigned long event)
{
	// sync ipv6 addr to sw netif table
	int ret = RTK_FC_RET_OK;
	struct net_device *dev = NULL;
	unsigned int mtu;

	RTK_FC_HELPER_NETDEV_IFAV6_TO_DEV(ptr, &dev);
	if(unlikely(fc_db.rtk_fc_mgr_init==0)) return RTK_FC_RET_OK;

	if(unlikely(fc_db.controlFuc.hwnat_mode==RT_FLOW_HWNAT_MODE_SPEC_TEST)) return RTK_FC_RET_OK;

	if(RTK_FC_HELPER_NETDEV_IGNORE_LOOKUP(dev,event)) return RTK_FC_RET_OK;

	EVENT("[INET6ADDR EVENT] netdevice:%s event:%lu\n", dev->name, event);

	switch (event) {
		case NETDEV_UP:
			break;
		default:
			return RTK_FC_RET_OK;
	}

#ifdef CONFIG_SYSCTL // disable netfilter checksum checking
	{
		RTK_FC_HELPER_NETDEV_SET_NETFILTER_CHECKSUM(&dev);
		/*
		struct net *net = dev_net(dev);
		net->ct.sysctl_checksum = 0;
		*/
	}
#endif
	RTK_FC_HELPER_NETDEV_GET_DEV_DATA(dev, &mtu, RTK_FC_NETDEV_GET_MTU);

	EVENT("devname = %s, mtu = %d", dev->name, mtu);	

	return ret;

}


int rtk_fc_eventHandler_netDev(void *ptr, unsigned long event)
{
	struct net_device *dev = NULL;
	bool iswlan = FALSE;
	int32 fcDevIdx=DEVIFIDX_INVALID_MIN;
	
	RTK_FC_HELPER_NETDEV_NOTIFIERINFO_TO_DEV(ptr, &dev);

	if(unlikely(fc_db.rtk_fc_mgr_init==0)) return RTK_FC_RET_OK;
	
	if(unlikely(fc_db.controlFuc.hwnat_mode==RT_FLOW_HWNAT_MODE_SPEC_TEST)) return RTK_FC_RET_OK;

	if(RTK_FC_HELPER_NETDEV_IGNORE_LOOKUP(dev,event)) return RTK_FC_RET_OK;
	
	EVENT("[NETDEVICE EVENT] netdevice:%s(ifindex:%d) netSpace[ifindex:%d] net_init[ifindex:%d] net event:%lu features:0x%llx\n", dev->name, dev->ifindex,dev_net(dev)->ifindex,init_net.ifindex, event, dev->features);

	switch (event) {		
		case NETDEV_REGISTER:
			RTK_FC_HELPER_WLAN_SET_USBNAME(dev);
			/* fall through */
		case NETDEV_UP:
			//wlan init
			//For G3, below operation is done in rtk_rg_wlan_netDevice_set().
			RTK_FC_HOOK_PS_DEV_IS_WLAN_DEV(dev, &iswlan);
			if(iswlan)
			{
				RTK_FC_HELPER_SET_WFO_PORTID(dev);
				RTK_FC_HELPER_WLAN_REGISTER(RTK_FC_WLANX_NOT_FOUND, dev);
			}
			/* fall through */
		case NETDEV_CHANGEMTU:
			/* fall through */
		case NETDEV_CHANGEADDR:
			//add or update devGwMacTbl
			rtk_fc_devGwMac_update_and_add(dev,FALSE,&fcDevIdx);
			if(fcDevIdx!=DEVIFIDX_INVALID_MIN)
				rtk_fc_netif_sw_del_by_fcDevIdx(fcDevIdx);		
			break;
		case NETDEV_CHANGEUPPER:
			break;
		case NETDEV_DOWN:
			/* fall through */
		case NETDEV_UNREGISTER:
			RTK_FC_HOOK_PS_DEV_IS_WLAN_DEV(dev, &iswlan);
			if(iswlan)
			{
				// wlan devnot ready, so flush related flow to prevent wifi fast forward.
				rtk_fc_wlan_devidx_t wlan_dev_idx;
				RTK_FC_HELPER_WLAN_DEV_TO_DEVID(dev, &wlan_dev_idx);
				rtk_fc_flow_delete_by_wlanIdx(wlan_dev_idx);
				RTK_FC_HELPER_WLAN_UNREGISTER(dev);
			}
			rtk_fc_devGwMac_del(dev);
			break;
		case NETDEV_CHANGE:
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)			
			if(fc_db.controlFuc.bc_hwflow_accelerate && fc_db.bcMacIdx!=FAIL)
			{
				TABLE("link change delete abstr bc-flow delete by lut:%d",fc_db.bcMacIdx);
				rtk_fc_abstrSwFlow_BC_UUC_UMC_floodingDomain_delete();
			}
#endif			
			if(fc_db.igmp_unknown_flood)
			{
				extern int32 rtk_fc_igmp_sendAllMcDummyPktDetector(unsigned long task_priv);
				rtk_fc_igmp_sendAllMcDummyPktDetector(0UL);
				IGMP("dev %s state is change ",dev->name);
			}
			EVENT("NETDEV_CHANGE");
			break;
		default:
			return RTK_FC_RET_OK;
	}


	return RTK_FC_RET_OK;

}


EXPORT_SYMBOL(rtk_fc_devGwMacIdx_get);

