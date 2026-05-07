#include <rtk_rg_g3_internal.h>
#include <flow.h>
#include <mcast.h>
#include <vlan.h>
#include <l2.h>
#include <nat.h>
#include <queue.h>
#include <port.h>
#include <aal_common.h>
#include <aal_fdb.h>
#include <aal_port.h>
#include <aal_ni_l2.h>
#include <aal_l2_qos.h>
#include <aal_l2_vlan.h>
#include <aal_l2_te.h>
#include <aal_hashlite.h>
#include <aal_l3_cls.h>
#include <aal_l3_te_cb.h>
#include <aal_l3_cam.h>

char printtmp[256];
#define G3INITPRINT( comment ,arg...) \
do {\
            int mt_trace_i;\
            sprintf( printtmp, comment,## arg);\
            for(mt_trace_i=1;mt_trace_i<1024;mt_trace_i++) \
            { \
                    if(printtmp[mt_trace_i]==0) \
                    { \
                            if(printtmp[mt_trace_i-1]=='\n') printtmp[mt_trace_i-1]=' '; \
                            else break; \
                    } \
            } \
            printk("[HWNAT] %s @%s(%d)\n", printtmp, __FUNCTION__, __LINE__);\
} while(0)

#define G3DEBUG G3INITPRINT

#if defined(CONFIG_CA_G3_G3LITE_SERIES)

#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
#define LAN_PORT_SHAPER_IPG_ID 3

uint32_t rtk_rg_g3_l2fe_hostpolicing_init(void)
{
	ca_status_t ret = CA_E_OK;
	int i = 0;

	{
		// enable l2fe tpid parsing for 0x88a8
		//Bit[3]: TPID == 8100;
		//Bit[2]: TPID == 9100;
		//Bit[1]: TPID == 88A8;
		//Bit[0]: TPID == Value of Register FE_PR_FE_TPID_CMP_C
		ASSERT_EQ(ca_l2_vlan_outer_tpid_add(G3_DEF_DEVID, CA_PORT_ID(CA_PORT_TYPE_ETHERNET,G3_LOOPBACK_P_NEWSPA), 1), CA_E_OK);
	}

	{
		// enable port 6 loopback
		aal_ni_xge_cmn_cfg_t xge_cmn_cfg;
		aal_ni_xge_cmn_cfg_mask_t xge_cmn_cfg_mask;

		xge_cmn_cfg_mask.u32 = 0;
		memset(&xge_cmn_cfg, 0, sizeof(xge_cmn_cfg));
		
		xge_cmn_cfg_mask.s.core_tx2rx_lb = 1;
		xge_cmn_cfg.core_tx2rx_lb = 1;
		aal_ni_xge_cmn_config_set(G3_DEF_DEVID, 0, xge_cmn_cfg_mask, &xge_cmn_cfg);


		aal_fdb_port_limit_set(G3_DEF_DEVID, G3_LOOPBACK_P_NEWSPA, 0);
	}

	{
		/* to prevent port 6 from congestion, make port 6 use the same L3QM eq profile selection with port 7*/
		QM_QM_DEST_PORT0_EQ_CFG_t dp_eq_cfg;
		// get port 7 selection
		dp_eq_cfg.wrd = rtk_ne_reg_read(QM_QM_DEST_PORT0_EQ_CFG + (QM_QM_DEST_PORT0_EQ_CFG_STRIDE * (CA_NI_PORT7 + CA_NI_TOTAL_CPU_PORT)));
		// set to port 6
		rtk_ne_reg_write(dp_eq_cfg.wrd , QM_QM_DEST_PORT0_EQ_CFG + (QM_QM_DEST_PORT0_EQ_CFG_STRIDE * (CA_NI_PORT6 + CA_NI_TOTAL_CPU_PORT)));
	}

	{
		// use VE(vlan engine) to redirect downstream pkt to {ldpid, cos} according to fake vid
		int queue_id = 0, fake_vid =0; 
		aal_l2_vlan_fib_map_t vlanfibmap;
		aal_l2_vlan_fib_map_mask_t  vlanfibmapmask;
		aal_l2_vlan_fib_mask_t vlanfibmask;
		aal_l2_vlan_fib_t vlanfib;
		aal_ilpb_cfg_msk_t ilpbcfg_msk;
		aal_ilpb_cfg_t ilpbcfg;

		// reset all vlan configuration before create new one
		ASSERT_EQ(ca_l2_vlan_delete_all(G3_DEF_DEVID), CA_E_OK);
				
		for(i = G3_LOOPBACK_P_START; i <= G3_LOOPBACK_P_END; i++) {
			for(queue_id = 0; queue_id <= 7; queue_id++) {
				
				fake_vid = G3_LOOPBACK_DOWNSTREAM_VID(i, queue_id);
				
				// create fake vlan but only works on lan ports
				ASSERT_EQ(ca_l2_vlan_create(G3_DEF_DEVID, fake_vid), CA_E_OK);

				vlanfibmapmask.u32 = 0;
				vlanfibmapmask.s.valid = 1;
				vlanfibmap.valid = 0;
				
				aal_l2_vlan_fib_map_set(G3_DEF_DEVID, AAL_L2_VLAN_TYPE_WAN_SVLAN, fake_vid, vlanfibmapmask, &vlanfibmap);
				aal_l2_vlan_fib_map_set(G3_DEF_DEVID, AAL_L2_VLAN_TYPE_WAN_CVLAN, fake_vid, vlanfibmapmask, &vlanfibmap);

				// enable vlan engine to pop fake vlan and redirect to specific dest port
				vlanfibmask.u32 = 0;
				vlanfibmask.s.dpid_field_vld = 1;
				vlanfibmask.s.dpid = 1;
				vlanfibmask.s.cos_field_vld = 1;
				vlanfibmask.s.cos = 1;
				vlanfibmask.s.top_vlan_cmd_field_vld = 1;
				vlanfibmask.s.top_vlan_cmd = 1;
				vlanfibmask.s.permit = 1;

				vlanfib.dpid_field_vld = 1;
				vlanfib.dpid = i;
				vlanfib.cos_field_vld = 1;
				vlanfib.cos = queue_id;
				vlanfib.top_vlan_cmd_field_vld = 1;
				vlanfib.top_vlan_cmd = 2;			//0:nop;1:push;2:pop;3:swap
				vlanfib.permit = 1;
		
				aal_l2_vlan_fib_map_get(G3_DEF_DEVID, AAL_L2_VLAN_TYPE_LAN_VLAN, fake_vid, &vlanfibmap);
				//G3INITPRINT("create fake vid %d, fibid = %d (valid:%d)", fake_vid, vlanfibmap.fib_id, vlanfibmap.valid);
				aal_l2_vlan_action_cfg_set(G3_DEF_DEVID, AAL_L2_VLAN_TYPE_LAN_VLAN, vlanfibmap.fib_id, vlanfibmask, &vlanfib);
			}

			// For upstream loopback, force port 0 ~ port 3 do not aware vlan tag to prevent incorrect vlan operation of ptag(priority tag).
			{
				memset(&ilpbcfg_msk, 0, sizeof(ilpbcfg_msk));
				memset(&ilpbcfg, 0, sizeof(ilpbcfg));
				ilpbcfg_msk.s.s_tpid_match = 1;
				ilpbcfg_msk.s.c_tpid_match = 1;
				ilpbcfg_msk.s.other_tpid_match = 1;
				ilpbcfg.s_tpid_match = 0x0;	
				ilpbcfg.c_tpid_match = 0x0;	
				ilpbcfg.other_tpid_match = 0x0;
				aal_port_ilpb_cfg_set(G3_DEF_DEVID, i, ilpbcfg_msk, &ilpbcfg);
			}
		}

		memset(&ilpbcfg_msk, 0, sizeof(ilpbcfg_msk));
		memset(&ilpbcfg, 0, sizeof(ilpbcfg));
		ilpbcfg_msk.s.cos_mode_sel = 1;
		ilpbcfg_msk.s.cos_control_bm = 1;
		ilpbcfg.cos_mode_sel = 2;				// remarking from cls/vlan
		ilpbcfg.cos_control_bm = 2;				// allow ve only
		aal_port_ilpb_cfg_set(G3_DEF_DEVID, G3_LOOPBACK_P_NEWSPA, ilpbcfg_msk, &ilpbcfg);

	}
#if !defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
	{
		//DMAC host policing: use egress packet lenth to do rate limiting
		//with fake vlan --> L2FE --> without fake vlan
		
		aal_l2_te_policer_cfg_t cfg;
		aal_l2_te_policer_cfg_msk_t msk;
		int flow_id;

		memset((void *)&cfg, 0, sizeof(cfg));
		memset((void *)&msk, 0, sizeof(msk));
		msk.s.pkt_len_sel = 1;
		cfg.pkt_len_sel = 1;
		for(flow_id = 0 + G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_TX ; flow_id < (G3_FLOW_POLICER_HOSTPOLICING_SIZE + G3_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR_TX) ; flow_id++)
			ASSERT_EQ(aal_l2_te_policer_flow_cfg_set(G3_DEF_DEVID, flow_id, msk, &cfg), CA_E_OK);
	}
#else
	{
		//DMAC host policing: use egress packet lenth to do rate limiting
		//with fake vlan --> L2FE --> without fake vlan

		aal_l2_te_policer_cfg_t cfg;
		aal_l2_te_policer_cfg_msk_t msk;
		int flow_id;

		memset((void *)&cfg, 0, sizeof(cfg));
		memset((void *)&msk, 0, sizeof(msk));
		msk.s.pkt_len_sel = 1;
		cfg.pkt_len_sel = 1;
		for(flow_id = 0 + G3_LOOPBACKMODE_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR ; flow_id < (G3_FLOW_POLICER_HOSTPOLICING_SIZE + G3_LOOPBACKMODE_FLOW_POLICER_IDXSHIFT_HOSTPOLMTR) ; flow_id++)
			ASSERT_EQ(aal_l2_te_policer_flow_cfg_set(G3_DEF_DEVID, flow_id, msk, &cfg), CA_E_OK);
	}
#endif

	{
		//disable port 6 L2/L3 port policer
		aal_l2_te_policer_cfg_msk_t	l2_policer_msk;
   		aal_l2_te_policer_cfg_t		l2_policer_cfg;
		aal_l3_te_policer_cfg_msk_t l3_policer_msk;
    	aal_l3_te_policer_cfg_t		l3_policer_cfg;

		
		memset((void *)&l2_policer_msk, 0, sizeof(l2_policer_msk));
		memset((void *)&l2_policer_cfg, 0, sizeof(l2_policer_cfg));
		l2_policer_msk.s.type = 1;
		l2_policer_cfg.type = CA_AAL_POLICER_TYPE_NONE;
        ASSERT_EQ(aal_l2_te_policer_port_cfg_set(G3_DEF_DEVID, G3_LOOPBACK_P_NEWSPA, l2_policer_msk, &l2_policer_cfg), CA_E_OK);

		memset((void *)&l3_policer_msk, 0, sizeof(l3_policer_msk));
		memset((void *)&l3_policer_cfg, 0, sizeof(l3_policer_cfg));
		l3_policer_msk.s.type = 1;
		l3_policer_cfg.type = CA_AAL_L3_POLICER_TYPE_NONE;
        ASSERT_EQ(aal_l3_te_policer_port_cfg_set(G3_DEF_DEVID, G3_LOOPBACK_P_NEWSPA, l3_policer_msk, &l3_policer_cfg), CA_E_OK);
	}
	return ret;
}
#else
uint32_t rtk_rg_g3_l2fe_hostpolicing_init(void)
{
	return 0 ;
}
#endif

#if defined(CONFIG_RG_G3_L3FE_MC_DEEPQ)
uint32_t rtk_rg_g3_l3fe_mcbuffering_init(void)
{

	{
		// enable port 6 loopback
		aal_ni_xge_cmn_cfg_t xge_cmn_cfg;
		aal_ni_xge_cmn_cfg_mask_t xge_cmn_cfg_mask;

		xge_cmn_cfg_mask.u32 = 0;
		memset(&xge_cmn_cfg, 0, sizeof(xge_cmn_cfg));
		
		xge_cmn_cfg_mask.s.core_tx2rx_lb = 1;
		xge_cmn_cfg.core_tx2rx_lb = 1;
		aal_ni_xge_cmn_config_set(0, 0, xge_cmn_cfg_mask, &xge_cmn_cfg);
	}

	{
		// enable L3TM sharper 1Gbps, sharping L3QM -> p6 traffic
		ca_port_id_t port_id = CA_NI_PORT6;
		aal_l3_te_shaper_tbc_cfg_msk_t shaper_msk;
		aal_l3_te_shaper_tbc_cfg_t shaper;

		shaper_msk.u32 = 0;
		memset(&shaper, 0, sizeof(shaper));

		shaper_msk.s.bs     = 1;
		shaper_msk.s.rate_k = 1;
		shaper_msk.s.rate_m = 1;
		shaper_msk.s.mode   = 1;
		shaper_msk.s.state  = 1;

		// ca_port_shaper_set
		shaper.bs		= 3;		//(1048568/1000 + 511) >> 9;
		shaper.rate_k 	= 256;
		shaper.rate_m 	= 1024;
		shaper.mode   	= CA_AAL_SHAPER_MODE_BPS;
		shaper.state  	= CA_AAL_SHAPER_ADMIN_STATE_SHAPER;
	
		ASSERT_EQ(aal_l3_te_shaper_port_tbc_set(G3_DEF_DEVID, port_id, shaper_msk, &shaper), CA_E_OK);
	}
	
	{
		// in order to buffer mc traffic by L3QM, enable deepq and tx to port 6(loopback port).
		// 	above changes could be found in aal_l3_cls_tbl_init()
		// enable port 6 DQ select here
		
		QM_QM_DEST_PORT0_EQ_CFG_t dp_eq_cfg;
		// get port 7 selection
		dp_eq_cfg.wrd = rtk_ne_reg_read(QM_QM_DEST_PORT0_EQ_CFG + (QM_QM_DEST_PORT0_EQ_CFG_STRIDE * (CA_NI_PORT7 + CA_NI_TOTAL_CPU_PORT)));
		// set to port 6
		rtk_ne_reg_write(dp_eq_cfg.wrd , QM_QM_DEST_PORT0_EQ_CFG + (QM_QM_DEST_PORT0_EQ_CFG_STRIDE * (CA_NI_PORT6 + CA_NI_TOTAL_CPU_PORT)));
	}
	
	return CA_E_OK;
}
#endif

#define HL_ACTGRP_UUC_GENERIC	(ACTION_GROUP_12 | ACTION_GROUP_13 | ACTION_GROUP_14 | ACTION_GROUP_15 | ACTION_GROUP_16 | ACTION_GROUP_18 | ACTION_GROUP_19)
#define HL_ACTGRP_PPPOEACT	(ACTION_GROUP_07 | ACTION_GROUP_12)

typedef struct {
	HASH_ACTION_GROUP_12;
	HASH_ACTION_GROUP_13;
	HASH_ACTION_GROUP_14;
	HASH_ACTION_GROUP_15;
	HASH_ACTION_GROUP_16;
	HASH_ACTION_GROUP_18;
	HASH_ACTION_GROUP_19;
} __attribute__((packed)) hl_act_uuc_generic_t;

static hl_act_uuc_generic_t HL_ACT_UUC = {
	.mdata_w_vld_2 = 0x8,
	.mdata_w_2 = (RXINFO_REF_TRAP_RSN_UNKNOWN_DA<<RXINFO_REF_TRAP_RSN_SHIFT_BIT),	//RXINFO_REF_TRAP_RSN_UNKNOWN_DA in FC/RG
	.permit = 1,
	.mcgid = ASIC_LPORT_CPU_0,
	.mc = 0,
	.dpid_pri = 1,
	.dpid_vld = 1,
	.keep_orig_pkt = 1,
	.keep_orig_pkt_vld = 1,
	.pol_vld = 1,
	.pol_en = 2,
	.pol_base = 256,		// the reserved entry for uuc storm control.
	.pol_sel = 2,
	.stage3_ctrl = 0,
	.stage3_ctrl_vld = 1,
	.t5_ctrl = 0xf,
	.t5_ctrl_vld = 1,
};

aal_hash_mask_t HL_KEYMSK_UUC;

static aal_hashlite_profile_t profile_uuc = {
	.type0_act_grp_mask_hit =	HL_ACTGRP_UUC_GENERIC,
	.type1_act_grp_mask_hit =	HL_ACTGRP_UUC_GENERIC,

	.miss = {
		.type0_act_grp_mask_empty_miss =	HL_ACTGRP_UUC_GENERIC,
		.type0_empty_miss =			&HL_ACT_UUC,
		.type0_empty_miss_len =			sizeof(HL_ACT_UUC),

		.type0_act_grp_mask_age_miss = 		HL_ACTGRP_UUC_GENERIC,
		.type0_age_miss =			&HL_ACT_UUC,
		.type0_age_miss_len =			sizeof(HL_ACT_UUC),

		.type1_act_grp_mask_empty_miss =	HL_ACTGRP_UUC_GENERIC,
		.type1_empty_miss =			&HL_ACT_UUC,
		.type1_empty_miss_len =			sizeof(HL_ACT_UUC),

		.type1_act_grp_mask_age_miss =		HL_ACTGRP_UUC_GENERIC,
		.type1_age_miss =			&HL_ACT_UUC,
		.type1_age_miss_len =			sizeof(HL_ACT_UUC),
	},

	.hit = {
		{ TUPLE_TYPE_FWD,	&HL_KEYMSK_UUC },
		{ TUPLE_TYPE_UNUSED,	NULL },
		{ TUPLE_TYPE_UNUSED,	NULL },
		{ TUPLE_TYPE_UNUSED,	NULL },
		{ TUPLE_TYPE_UNUSED,	NULL },
		{ TUPLE_TYPE_UNUSED,	NULL },
		{ TUPLE_TYPE_UNUSED,	NULL },
		{ TUPLE_TYPE_UNUSED,	NULL },
	},
};

#if defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)

typedef struct {
	HASH_ACTION_GROUP_07;
	HASH_ACTION_GROUP_12;
} __attribute__((packed)) hl_act_pppoeact_t;

static hl_act_pppoeact_t HL_ACT_PPPOEACT = {
	.mdata_w_vld_0 = 0xC,
	.mdata_w_0 = 0,
};

aal_hash_mask_t HL_KEYMSK_PPPOEACT;

static aal_hashlite_profile_t profile_pppoeact = {
	.type0_act_grp_mask_hit =	HL_ACTGRP_PPPOEACT,
	.type1_act_grp_mask_hit =	HL_ACTGRP_PPPOEACT,

	.miss = {
		.type0_act_grp_mask_empty_miss =	HL_ACTGRP_PPPOEACT,
		.type0_empty_miss =			&HL_ACT_PPPOEACT,
		.type0_empty_miss_len =			sizeof(HL_ACT_PPPOEACT),

		.type0_act_grp_mask_age_miss = 		HL_ACTGRP_PPPOEACT,
		.type0_age_miss =			&HL_ACT_PPPOEACT,
		.type0_age_miss_len =			sizeof(HL_ACT_PPPOEACT),

		.type1_act_grp_mask_empty_miss =	HL_ACTGRP_PPPOEACT,
		.type1_empty_miss =			&HL_ACT_PPPOEACT,
		.type1_empty_miss_len =			sizeof(HL_ACT_PPPOEACT),

		.type1_act_grp_mask_age_miss =		HL_ACTGRP_PPPOEACT,
		.type1_age_miss =			&HL_ACT_PPPOEACT,
		.type1_age_miss_len =			sizeof(HL_ACT_PPPOEACT),
	},

	.hit = {
		{ TUPLE_TYPE_FWD,	&HL_KEYMSK_PPPOEACT},
		{ TUPLE_TYPE_UNUSED,	NULL },
		{ TUPLE_TYPE_UNUSED,	NULL },
		{ TUPLE_TYPE_UNUSED,	NULL },
		{ TUPLE_TYPE_UNUSED,	NULL },
		{ TUPLE_TYPE_UNUSED,	NULL },
		{ TUPLE_TYPE_UNUSED,	NULL },
		{ TUPLE_TYPE_UNUSED,	NULL },
	},
};
#endif

int32 rtk_rg_g3_l3fe_unknownDAStormCtrl(unsigned int port, bool enable, int l3fe_pol_idx)
{
	int ret = AAL_E_OK;
	int hashlite_profile = HASHLITE_PROFILE_0 + port;

	if(hashlite_profile>HASHLITE_PROFILE_15) {
		G3INITPRINT("forbidden to enable uuc storm control for port %d", port);
		return AAL_E_OUTRANGE;
	}

	// 2K+32(G3) or 256+32(G3Lite) entries, 192 bits per each action fib
	
	//G3INITPRINT("turn %s port %d unknown DA storm control, policer idx is %d", enable?"ON":"OFF", port, l3fe_pol_idx);
	
	if(enable) {
		HL_ACT_UUC.stage3_ctrl = 1;	//Do NOT apply type-0 result of Hash Engine, keep hashlite mdata_w_2
		HL_ACT_UUC.pol_vld = 1;
		HL_ACT_UUC.pol_en = 2;
		HL_ACT_UUC.pol_base = l3fe_pol_idx;		// the reserved entry for uuc storm control.
		HL_ACT_UUC.pol_sel = 2;
		
		if ((ret = aal_hashlite_profile_set(G3_DEF_DEVID, hashlite_profile, &profile_uuc)) != AAL_E_OK) {
			G3INITPRINT("ERROR! Fail to initialize HashLite profile %d. (ret %d)\n", hashlite_profile, ret);
		}
	} else {
		HL_ACT_UUC.stage3_ctrl = 0;	//Apply type-0 result of Hash Engine
		HL_ACT_UUC.pol_vld = 0;
		HL_ACT_UUC.pol_en = 0;
		HL_ACT_UUC.pol_base = 0;
		HL_ACT_UUC.pol_sel = 0;
	
		if ((ret = aal_hashlite_profile_set(G3_DEF_DEVID, hashlite_profile, &profile_uuc)) != AAL_E_OK) {
			G3INITPRINT("ERROR! Fail to initialize HashLite profile %d. (ret %d)\n", hashlite_profile, ret);
		}
	}
	return ret;
}

int32 rtk_rg_g3_l3fe_unknownDA_wan_default_enable(unsigned int wan_port_idx)
{
	ca_int32_t ret = AAL_E_OK;
	cls_action_t action;

	/* set CLS WAN default fib for hashlite(unknowDA lookup)*/
	memset(&action, 0, sizeof(cls_action_t));
	/* set msk_ctrl */
	action.t2_ctrl_vld = 1;
	action.t2_ctrl = 0; 		// any one to trap packets
	action.t3_ctrl_vld = 1;
	action.t3_ctrl = T3_CTRL_BYPASS;
	action.t4_ctrl_vld = 1;
	action.t4_ctrl = wan_port_idx;
	action.t5_ctrl_vld = 1;
	action.t5_ctrl = T5_CTRL_BYPASS;
	action.dpid_vld = 1;
	action.permit = 1;
	action.mcgid = ASIC_LPORT_CPU_0,
	action.dpid_pri = 1;
	
	ret = aal_l3_cls_default_set(G3_DEF_DEVID, 0, CL_RSLT_TYPE_0, &action);
	if (ret != AAL_E_OK) {
		G3INITPRINT("L3 CLS WAN DEF FIB initialtion fail, ret = %d\n", ret);
	}

	return ret;
}

int32 rtk_rg_g3_l3fe_knownDA_add(unsigned char *dmac, unsigned int *entryidx)
{
	int ret = AAL_E_OK;
	aal_hash_key_t key;
	aal_hashlite_hash_action_entry_t action;
	unsigned int hashIdx = 0;

	memset(&key, 0, sizeof(key));
	memset(&action, 0, sizeof(action));
	*entryidx = 0;
		
	key.mac_da_0 = dmac[5];
	key.mac_da_1 = dmac[4];
	key.mac_da_2 = dmac[3];
	key.mac_da_3 = dmac[2];
	key.mac_da_4 = dmac[1];
	key.mac_da_5 = dmac[0];
	
	ret = aal_hashlite_hash_add(G3_DEF_DEVID, &key, &HL_KEYMSK_UUC, &action, HL_ACTGRP_UUC_GENERIC, HASHLITE_AGING_STATIC, &hashIdx);

	if(ret==AAL_E_EXIST || ret==AAL_E_OK) {
		if(hashIdx & HASHLITE_OVERFLOW_FLAG)
			*entryidx = (hashIdx & HASHLITE_INDEX_MASK) + HASHLITE_HASH_ACTION_ENTRY_MAX;
		else
			*entryidx = (hashIdx & HASHLITE_INDEX_MASK);
		
		ret = AAL_E_OK;
		//printk("add known da %pM to hashlite entry %d\n", dmac, *entryidx);
		
	}else if(ret == AAL_E_TBLFULL){
		printk(KERN_DEBUG "table full, fail to add known da hashlite entry, ret = %d\n", ret);
		
	}else{
		G3INITPRINT("ERROR! fail to add known da hashlite entry, ret = %d\n", ret);
		
	}

	return ret;
}

int32 rtk_rg_g3_l3fe_knownDA_del(unsigned int  entryidx)
{
	int ret = AAL_E_OK;

	ret = aal_hashlite_hash_delete(G3_DEF_DEVID, entryidx);
	
	if(ret) {
		G3INITPRINT("fail to del known da with hashlite entry idx %d, ret = %d\n", entryidx, ret);
	}
	
	return ret;
}

int32 rtk_rg_g3_l3fe_knownDA_flush(void)
{
	unsigned int hashIdx = 0;
	
	for(hashIdx = 0; hashIdx < (HASHLITE_HASH_TBL_ENTRY_MAX+HASHLITE_OVERFLOW_TBL_ENTRY_MAX); hashIdx++) {
		aal_hashlite_hash_delete(G3_DEF_DEVID, hashIdx);
	}

	return AAL_E_OK;
}


uint32_t rtk_rg_g3_hashlite_init(void)
{
	unsigned char bcMac[ETHER_ADDR_LEN];
	unsigned int wanportidx = ASIC_LPORT_ETH_NI7;
		
	// unmask all fields in hash mask, refer: hash_mask_unmask()
	memset(&HL_KEYMSK_UUC, -1, sizeof(aal_hash_mask_t));
	HL_KEYMSK_UUC.ip_sa = 0;
	HL_KEYMSK_UUC.ip_da = 0;
	HL_KEYMSK_UUC.ip_ttl = 0;

	// turn on interested key
	HL_KEYMSK_UUC.mac_da = 0;

	// disable
	rtk_rg_g3_l3fe_unknownDAStormCtrl(0, DISABLED, 0);
	rtk_rg_g3_l3fe_unknownDAStormCtrl(1, DISABLED, 0);
	rtk_rg_g3_l3fe_unknownDAStormCtrl(2, DISABLED, 0);
	rtk_rg_g3_l3fe_unknownDAStormCtrl(3, DISABLED, 0);
	rtk_rg_g3_l3fe_unknownDAStormCtrl(4, DISABLED, 0);
	
#if defined(CONFIG_ARCH_CORTINA_G3HGU)	
#if defined(CONFIG_RG_G3_WAN_PORT_INDEX)
	rtk_rg_g3_l3fe_unknownDAStormCtrl(CONFIG_RG_G3_WAN_PORT_INDEX, DISABLED, 0);
	wanportidx = CONFIG_RG_G3_WAN_PORT_INDEX;
#else
	rtk_rg_g3_l3fe_unknownDAStormCtrl(7, DISABLED, 0);
	wanportidx = ASIC_LPORT_ETH_NI7;
#endif
#endif

	rtk_rg_g3_l3fe_knownDA_flush();
	memset(bcMac, 0xff, ETHER_ADDR_LEN);
	{
		unsigned int idx;
		rtk_rg_g3_l3fe_knownDA_add(bcMac, &idx);
	}	
	
	// enable
	//rtk_rg_g3_l3fe_unknownDAStormCtrl(1, G3_FLOW_POLICER_IDXSHIFT_STORMCTL + 3);		// for test
	aal_hashlite_aging_timer_set(0);

	/* set CLS WAN default fib for hashlite(unknowDA lookup)*/
	ASSERT_EQ(rtk_rg_g3_l3fe_unknownDA_wan_default_enable(wanportidx), AAL_E_OK);

#if defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
	/*
	 * Enable hashlite profile 0xD to offload pppoe action
	 */
	{
		int ret = AAL_E_OK;
		aal_hash_key_t key;
		aal_hashlite_hash_action_entry_t action;
		hl_act_pppoeact_t *pPPPoEAct = (hl_act_pppoeact_t *)&action;
		unsigned int hashIdx = 0, entryidx = 0;
	
		memset(&HL_KEYMSK_PPPOEACT, -1, sizeof(aal_hash_mask_t));
		HL_KEYMSK_PPPOEACT.ip_sa = 0;
		HL_KEYMSK_PPPOEACT.ip_da = 0;
		HL_KEYMSK_PPPOEACT.ip_ttl = 0;

		// turn on interested key
		HL_KEYMSK_PPPOEACT.mdata = 0xffffffffffff00ff; /* mdata[15:8], 8 bits */

		if ((ret = aal_hashlite_profile_set(G3_DEF_DEVID, HASHLITE_PROFILE_13, &profile_pppoeact)) != AAL_E_OK) {
			G3INITPRINT("ERROR! Fail to initialize HashLite profile %d. (ret %d)\n", HASHLITE_PROFILE_13, ret);
		}


		memset(&key, 0, sizeof(key));
		memset(&action, 0, sizeof(action));

		// remove pppoe tag
		key.mdata = 1 <<8;	
		pPPPoEAct->pppoe_set = 1;	
		pPPPoEAct->mdata_w_vld_0 = 0xC;
		pPPPoEAct->mdata_w_0 = 0;
		
		ret = aal_hashlite_hash_add(G3_DEF_DEVID, &key, &HL_KEYMSK_PPPOEACT, &action, HL_ACTGRP_PPPOEACT, HASHLITE_AGING_STATIC, &hashIdx);

		G3INITPRINT("Hashlite PPPoE Act offload use entry[%d]", hashIdx);

		if(ret==AAL_E_EXIST || ret==AAL_E_OK) {
			if(hashIdx & HASHLITE_OVERFLOW_FLAG)
				entryidx = (hashIdx & HASHLITE_INDEX_MASK) + HASHLITE_HASH_ACTION_ENTRY_MAX;
			else
				entryidx = (hashIdx & HASHLITE_INDEX_MASK);
			
			ret = AAL_E_OK;
			
		}else if(ret == AAL_E_TBLFULL){
			printk(KERN_DEBUG "table full, fail to add hashlite pppoe action entry, ret = %d\n", ret);
			
		}else{
			G3INITPRINT("ERROR! fail to add hashlite pppoe action entry, ret = %d\n", ret);
			
		}
	}
#endif
	
	return CA_E_OK;
}
#endif


#if defined(CONFIG_CA8277B_SERIES)

#define HL_ACTGRP_SMACACT	(ACTION_GROUP_10)

typedef struct {
	HASH_ACTION_GROUP_10;
} __attribute__((packed)) hl_act_smacact_t;

static hl_act_smacact_t HL_ACT_SMACACT_DEF = {
	.mac_sa_vld = 0,
};

aal_hash_mask_t HL_KEYMSK_SMACACT;

static aal_hashlite_profile_t profile_smacact = {
	.type0_act_grp_mask_hit =	HL_ACTGRP_SMACACT,
	.type1_act_grp_mask_hit =	HL_ACTGRP_SMACACT,

	.miss = {
		.type0_act_grp_mask_empty_miss =	HL_ACTGRP_SMACACT,
		.type0_empty_miss =			&HL_ACT_SMACACT_DEF,
		.type0_empty_miss_len =			sizeof(HL_ACT_SMACACT_DEF),

		.type0_act_grp_mask_age_miss = 		HL_ACTGRP_SMACACT,
		.type0_age_miss =			&HL_ACT_SMACACT_DEF,
		.type0_age_miss_len =			sizeof(HL_ACT_SMACACT_DEF),

		.type1_act_grp_mask_empty_miss =	HL_ACTGRP_SMACACT,
		.type1_empty_miss =			&HL_ACT_SMACACT_DEF,
		.type1_empty_miss_len =			sizeof(HL_ACT_SMACACT_DEF),

		.type1_act_grp_mask_age_miss =		HL_ACTGRP_SMACACT,
		.type1_age_miss =			&HL_ACT_SMACACT_DEF,
		.type1_age_miss_len =			sizeof(HL_ACT_SMACACT_DEF),
	},

	.hit = {
		{ TUPLE_TYPE_FWD,	&HL_KEYMSK_SMACACT},
		{ TUPLE_TYPE_UNUSED,	NULL },
		{ TUPLE_TYPE_UNUSED,	NULL },
		{ TUPLE_TYPE_UNUSED,	NULL },
		{ TUPLE_TYPE_UNUSED,	NULL },
		{ TUPLE_TYPE_UNUSED,	NULL },
		{ TUPLE_TYPE_UNUSED,	NULL },
		{ TUPLE_TYPE_UNUSED,	NULL },
	},
};
uint32_t rtk_8277b_asic_hashlite_init(void)
{
	int ret = AAL_E_OK;		

	aal_hashlite_aging_timer_set(0);

	/*
	 * Enable hashlite profile 0xD to offload smac action
	 */
	{
		memset(&HL_KEYMSK_SMACACT, -1, sizeof(aal_hash_mask_t));
		HL_KEYMSK_SMACACT.ip_sa = 0;
		HL_KEYMSK_SMACACT.ip_da = 0;
		HL_KEYMSK_SMACACT.ip_ttl = 0;

		// turn on interested key
		HL_KEYMSK_SMACACT.mdata = 0xffffffffff00ffff; /* mdata[23:16], 8 bits */
		HL_KEYMSK_SMACACT.ip_vld = 0;					 /* must be enabled */

		if ((ret = aal_hashlite_profile_set(G3_DEF_DEVID, AAL_HL_PROFILE_SMAC_TRANS_OFFLOAD, &profile_smacact)) != AAL_E_OK) {
			G3INITPRINT("ERROR! Fail to initialize HashLite profile %d. (ret %d)\n", HASHLITE_PROFILE_13, ret);
		}
	}
	
	return CA_E_OK;
}

int32 rtk_8277b_asic_hl_egrsmac_trans_add(unsigned char egrNetifIdx, unsigned char *egrSMAC, unsigned int *entryidx)
{	
	int ret = AAL_E_OK;
	aal_hash_key_t key;
	aal_hashlite_hash_action_entry_t action;
	hl_act_smacact_t *pSMACAct = (hl_act_smacact_t *)&action;
	unsigned int hashIdx = 0;

	memset(&key, 0, sizeof(key));
	memset(&action, 0, sizeof(action));

	// request smac translation
	key.mdata = egrNetifIdx << (16);	// mdata_w1 bits[23:16]
	key.ip_vld = 1;
	pSMACAct->mac_sa_vld = 1;
	pSMACAct->mac_sa_0 = egrSMAC[5];
	pSMACAct->mac_sa_1 = egrSMAC[4];
	pSMACAct->mac_sa_2 = egrSMAC[3];
	pSMACAct->mac_sa_3 = egrSMAC[2];
	pSMACAct->mac_sa_4 = egrSMAC[1];
	pSMACAct->mac_sa_5 = egrSMAC[0];
	
	ret = aal_hashlite_hash_add(G3_DEF_DEVID, &key, &HL_KEYMSK_SMACACT, &action, HL_ACTGRP_SMACACT, HASHLITE_AGING_STATIC, &hashIdx);

	//G3INITPRINT("Hashlite smac offload use entry[%d]", hashIdx);

	if(ret==AAL_E_EXIST || ret==AAL_E_OK) {
		if(hashIdx & HASHLITE_OVERFLOW_FLAG)
			*entryidx = (hashIdx & HASHLITE_INDEX_MASK) + HASHLITE_HASH_ACTION_ENTRY_MAX;
		else
			*entryidx = (hashIdx & HASHLITE_INDEX_MASK);
		
		ret = AAL_E_OK;
		
	}else if(ret == AAL_E_TBLFULL){
		printk(KERN_DEBUG "table full, fail to add hashlite smac translation entry, ret = %d\n", ret);
		
	}else{
		G3INITPRINT("ERROR! fail to add hashlite smac translation entry, ret = %d\n", ret);
		
	}
	
	return ret;
}
int32 rtk_8277b_asic_hl_egrsmac_trans_del(unsigned int entryidx)
{
	int ret = AAL_E_OK;

	ret = aal_hashlite_hash_delete(G3_DEF_DEVID, entryidx);
	
	if(ret) {
		G3INITPRINT("fail to del smac translation with hashlite entry idx %d, ret = %d\n", entryidx, ret);
	}
	
	return ret;

}
int32 rtk_8277b_asic_ni_p7_rxsel_l3fe(unsigned char enable)
{
	int ret = AAL_E_OK;	
	ca_ni_rx_l3fe_demux_t demux={0};
	aal_ni_hv_glb_internal_port_id_cfg_mask_t niGlbCfgMask={0};
	aal_ni_hv_glb_internal_port_id_cfg_t niGlbCfg={0};

	if(enable) {
		/* 
		 * PON->L3FE->L2FE->L2TM->L3QM->CPU
		 * take care the configuration sequence: make sure l2 cls rule ready first.
		 */

		niGlbCfgMask.wrd = 0;
		niGlbCfgMask.bf.wan_rxsel = TRUE;
		niGlbCfg.wan_rxsel = 0;
		aal_ni_hv_glb_internal_port_id_cfg_set(CA_DEF_DEVID, niGlbCfgMask, &niGlbCfg);

		_rtk_rg_aclAndCfReservedRuleDelSpecial(RTK_CA_CLS_TYPE_L2_INGRESS_FORWARD_L3FE, NULL);
		_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_L2_INGRESS_FORWARD_L3FE, NULL);

		demux.wrd = rtk_ne_reg_read(NI_HV_GLB_NIRX_L3FE_DEMUX_CFG1);
		demux.bf.ldpid_1	= NIRX_L3FE_DEMUX_ID_L2FE;		
		rtk_ne_reg_write(demux.wrd, NI_HV_GLB_NIRX_L3FE_DEMUX_CFG1);
		
		demux.wrd = rtk_ne_reg_read(NI_HV_GLB_NIRX_L3FE_DPQ_DEMUX_CFG1);
		demux.bf.ldpid_1	= NIRX_L3FE_DEMUX_ID_L2FE;
		rtk_ne_reg_write(demux.wrd, NI_HV_GLB_NIRX_L3FE_DPQ_DEMUX_CFG1);
		
		rtk_ne_reg_write(0x000600ff, L2TM_L2TM_ES_SCH_CFG0 + L2TM_L2TM_ES_SCH_CFG0_STRIDE * 10);	// port 10 - wan to L3FE
		rtk_ne_reg_write(0x000600ff, L2TM_L2TM_ES_SCH_CFG0 + L2TM_L2TM_ES_SCH_CFG0_STRIDE * 13);	// port 13 - lan to L3FE
		rtk_ne_reg_write(0x000600ff, L2TM_L2TM_ES_SCH_CFG0 + L2TM_L2TM_ES_SCH_CFG0_STRIDE * 8);		// port 8 - L2TM to L3QM
	}else{
		/* 
		 * PON->L2FE->L2TM->L3FE->L3QM->CPU
		 * take care the configuration sequence: make sure l2 cls rule ready first.
		 */
		demux.wrd = rtk_ne_reg_read(NI_HV_GLB_NIRX_L3FE_DEMUX_CFG1);
		demux.bf.ldpid_1	= NIRX_L3FE_DEMUX_ID_L3QM;		
		rtk_ne_reg_write(demux.wrd, NI_HV_GLB_NIRX_L3FE_DEMUX_CFG1);
		
		demux.wrd = rtk_ne_reg_read(NI_HV_GLB_NIRX_L3FE_DPQ_DEMUX_CFG1);
		demux.bf.ldpid_1	= NIRX_L3FE_DEMUX_ID_L3QM;
		rtk_ne_reg_write(demux.wrd, NI_HV_GLB_NIRX_L3FE_DPQ_DEMUX_CFG1);
		
	
		_rtk_rg_aclAndCfReservedRuleDelSpecial(RTK_CA_CLS_TYPE_L2_INGRESS_FORWARD_L3FE, NULL);
		_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_L2_INGRESS_FORWARD_L3FE, NULL);

		niGlbCfgMask.wrd = 0;
		niGlbCfgMask.bf.wan_rxsel = TRUE;
		niGlbCfg.wan_rxsel = 2;
		aal_ni_hv_glb_internal_port_id_cfg_set(CA_DEF_DEVID, niGlbCfgMask, &niGlbCfg);
		
		rtk_ne_reg_write(0x000800ff, L2TM_L2TM_ES_SCH_CFG0 + L2TM_L2TM_ES_SCH_CFG0_STRIDE * 10);	// port 10 - wan to L3FE
		rtk_ne_reg_write(0x000800ff, L2TM_L2TM_ES_SCH_CFG0 + L2TM_L2TM_ES_SCH_CFG0_STRIDE * 13);	// port 13 - lan to L3FE
		rtk_ne_reg_write(0x000800ff, L2TM_L2TM_ES_SCH_CFG0 + L2TM_L2TM_ES_SCH_CFG0_STRIDE * 8);		// port 8 - L2TM to L3QM
	}
		
	return ret;
}
#endif

uint32_t rtk_rg_g3_cpuport_init(void) 
{
	ca_status_t ret = CA_E_OK;
	int i;

	i = 0;
	
#if defined(CONFIG_CA_NE_L2FP) || defined(CONFIG_NE_L2FP)	// if defined, L2FP module allocate netdev for those cpuports. ref: ca_l2fp_init

	// enable CPU port 0x12~0x17 for wifi acceleration
	for(i=ASIC_LPORT_CPU_2; i <= ASIC_LPORT_CPU_7; i++) {

		if(!(ret = rtk_ni_virtual_cpuport_open(i)))
			G3INITPRINT("Enable CPU port 0x%x", i);	
		else
			G3INITPRINT("ERROR - Enable CPU port 0x%x FAIL, ret = 0x%x !!!!!", i ,ret);
	}
	
#else
	G3INITPRINT("CA wifi acceleration (L2FP) in off!!!!!\n");

#endif

#if defined(CONFIG_ARCH_CORTINA_G3HGU)
	// init DMA_LSO shaper for wifi veriwave Rx test.
	{
		unsigned int regValue = 0;

		// shp_en=1, global shaper enable
		regValue = rtk_dma_lso_reg_read(DMA_SEC_SS_CTRL);
		regValue |= 0x40000000; 
		rtk_dma_lso_reg_write(regValue, DMA_SEC_SS_CTRL);

		// enb=2'b10, port shaper enable
		rtk_dma_lso_reg_write(0x00000080, DMA_SEC_SS_SHP_PORT_TBC_MEM_DATA1);
		
		// burst size=16, 1Gbps shaper rate (after take IPG into consideration)
		rtk_dma_lso_reg_write(0x400F6400, DMA_SEC_SS_SHP_PORT_TBC_MEM_DATA0);

		// indirect register write to DMA_LSO VP 2~7 shaper
		rtk_dma_lso_reg_write(0xc0000002, DMA_SEC_SS_SHP_PORT_TBC_MEM_ACCESS);
		rtk_dma_lso_reg_write(0xc0000003, DMA_SEC_SS_SHP_PORT_TBC_MEM_ACCESS);
		rtk_dma_lso_reg_write(0xc0000004, DMA_SEC_SS_SHP_PORT_TBC_MEM_ACCESS);
		rtk_dma_lso_reg_write(0xc0000005, DMA_SEC_SS_SHP_PORT_TBC_MEM_ACCESS);
		rtk_dma_lso_reg_write(0xc0000006, DMA_SEC_SS_SHP_PORT_TBC_MEM_ACCESS);
		rtk_dma_lso_reg_write(0xc0000007, DMA_SEC_SS_SHP_PORT_TBC_MEM_ACCESS);
		
	}
#endif

	return ret;
}


uint32_t rtk_rg_g3_cpuport_exit(void) 
{
	int i = 0;

	G3INITPRINT("unregister cpu port...");
		
	for(i=ASIC_LPORT_CPU_2; i <= ASIC_LPORT_CPU_7; i++) {
		rtk_ni_virtual_cpuport_close(i);	
		G3INITPRINT("close cpu port...0x%x", i);
	}
	
	return CA_E_OK;
}

uint32_t rtk_rg_g3_flow_init(rtk_rg_flow_key_mask_t flowKeyMask)
{
	ca_flow_key_type_config_t flow_key;
	ca_flow_key_type_t keyType;


	ca_flow_key_profile_mapping_set(G3_DEF_DEVID, RG_CA_FLOW_UC5TUPLE_DS,			HASH_PROFILE_0);
	ca_flow_key_profile_mapping_set(G3_DEF_DEVID, RG_CA_FLOW_UC5TUPLE_US,			HASH_PROFILE_1);
	ca_flow_key_profile_mapping_set(G3_DEF_DEVID, RG_CA_FLOW_MC, 					HASH_PROFILE_2);
	ca_flow_key_profile_mapping_set(G3_DEF_DEVID, RG_CA_FLOW_UC2TUPLE_BRIDGE,		HASH_PROFILE_3);
	ca_flow_key_profile_mapping_set(G3_DEF_DEVID, RG_CA_FLOW_MC_FORWARD,			HASH_PROFILE_2);
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
	ca_flow_key_profile_mapping_set(G3_DEF_DEVID, RG_CA_FLOW_CLS_DOS_POL,			HASH_PROFILE_4);
#elif defined(CONFIG_CA8277B_SERIES)
	ca_flow_key_profile_mapping_set(G3_DEF_DEVID, RG_CA_FLOW_CLS_TRAP_GRPPOL,		HASH_PROFILE_4);
#endif

	ca_flow_delete_all(G3_DEF_DEVID);
	for(keyType = CA_FLOW_TYPE_0; keyType<CA_FLOW_TYPE_MAX; keyType++){
		ca_flow_key_type_delete(G3_DEF_DEVID, keyType);
	}
	
	/****************************************************** 
	* profile 0: path345 pattern mask for ds
	******************************************************/
	memset(&flow_key, 0, sizeof(flow_key));
	flow_key.key_type = RG_CA_FLOW_UC5TUPLE_DS;		//FB_PATH_34 & 5
	flow_key.prio = RG_CA_FLOW_TUPPLE_PRI_0;
#if defined(CONFIG_CA8277B_SERIES)
	flow_key.key_mask.flow_id = (flowKeyMask.p345_stream_id) ? TRUE : FALSE;
#endif
	flow_key.key_mask.l2_keys = TRUE;
	flow_key.key_mask.l3_keys = TRUE;
	flow_key.key_mask.l4_keys = TRUE;
	flow_key.key_mask.l2_mask.o_lspid = TRUE;												// force enabled to support unknown sa trap by main hash
	flow_key.key_mask.l2_mask.outer_vlan.tpid = TRUE;
	flow_key.key_mask.l2_mask.outer_vlan.vid = (flowKeyMask.P345_vlanId) ? TRUE : FALSE;
	flow_key.key_mask.l2_mask.outer_vlan.pri = (flowKeyMask.P345_vlanPri) ? TRUE : FALSE;
	flow_key.key_mask.l2_mask.outer_vlan.dei = (flowKeyMask.pall_vlan_deicfi) ? TRUE : FALSE;
	flow_key.key_mask.l2_mask.inner_vlan.tpid = TRUE;
	flow_key.key_mask.l2_mask.inner_vlan.vid = (flowKeyMask.P345_vlanId) ? TRUE : FALSE;
	flow_key.key_mask.l2_mask.inner_vlan.pri = (flowKeyMask.P345_vlanPri) ? TRUE : FALSE;
	flow_key.key_mask.l2_mask.inner_vlan.dei = (flowKeyMask.pall_vlan_deicfi) ? TRUE : FALSE;
	flow_key.key_mask.l2_mask.pppoe_type = TRUE;
	flow_key.key_mask.l2_mask.pppoe_session_id = TRUE;
	flow_key.key_mask.l3_mask.ip_valid = TRUE;
	flow_key.key_mask.l3_mask.ip_version = TRUE;
	flow_key.key_mask.l3_mask.ip_protocol = TRUE;
	flow_key.key_mask.l3_mask.dscp = (flowKeyMask.P345_dscp) ? TRUE : FALSE;
	flow_key.key_mask.l3_mask.ecn = (flowKeyMask.P345_ecn) ? TRUE : FALSE;
	flow_key.key_mask.l3_mask.ip_sa = TRUE;
	flow_key.key_mask.l3_mask.ip_da = TRUE;
	
	flow_key.key_mask.l3_mask.fragment = TRUE;	
	flow_key.key_mask.l3_mask.hbh_header = TRUE;	//also enable hash_mask->ip_options by cortina-api
	flow_key.key_mask.l3_mask.routing_header = TRUE;
	flow_key.key_mask.l3_mask.dest_opt_header = TRUE;

	flow_key.key_mask.l4_mask.src_l4_port = TRUE;
	flow_key.key_mask.l4_mask.dst_l4_port = TRUE;
	
	flow_key.key_mask.l4_mask.tcp_flags= RTK_ASIC_5TUPLE_HASH_KEY_TCP_FLAG;	// rst, fin

	ASSERT_EQ(ca_flow_key_type_add(G3_DEF_DEVID, &flow_key), CA_E_OK);
	
	/****************************************************** 
	* profile 1: path345 pattern mask for  up
	******************************************************/
	flow_key.key_type = RG_CA_FLOW_UC5TUPLE_US;		//FB_PATH_34 & 5
	// all the other fields in flow_key are the same with profile 0
	ASSERT_EQ(ca_flow_key_type_add(G3_DEF_DEVID, &flow_key), CA_E_OK);
	
	/****************************************************** 
	* profile 3: path12 pattern mask
	******************************************************/
	memset(&flow_key, 0, sizeof(flow_key));
	flow_key.key_type = RG_CA_FLOW_UC2TUPLE_BRIDGE;	//FB_PATH_12
	flow_key.prio = RG_CA_FLOW_TUPPLE_PRI_0;
#if defined(CONFIG_CA8277B_SERIES)
	flow_key.key_mask.flow_id = (flowKeyMask.p12_stream_id) ? TRUE : FALSE;
#endif
	flow_key.key_mask.l2_keys = TRUE;
	
	flow_key.key_mask.l2_mask.o_lspid = TRUE;		// force enabled to support unknown sa trap by main hash
	flow_key.key_mask.l2_mask.mac_sa = TRUE;
	flow_key.key_mask.l2_mask.mac_da = TRUE;
	flow_key.key_mask.l2_mask.outer_vlan.tpid = TRUE;
	flow_key.key_mask.l2_mask.outer_vlan.vid = (flowKeyMask.P12_vlanId) ? TRUE : FALSE;
	flow_key.key_mask.l2_mask.outer_vlan.pri = (flowKeyMask.P12_vlanPri) ? TRUE : FALSE;
	flow_key.key_mask.l2_mask.outer_vlan.dei = (flowKeyMask.pall_vlan_deicfi) ? TRUE : FALSE;
	flow_key.key_mask.l2_mask.inner_vlan.tpid = TRUE;
	flow_key.key_mask.l2_mask.inner_vlan.vid = (flowKeyMask.P12_vlanId) ? TRUE : FALSE;
	flow_key.key_mask.l2_mask.inner_vlan.pri = (flowKeyMask.P12_vlanPri) ? TRUE : FALSE;
	flow_key.key_mask.l2_mask.inner_vlan.dei = (flowKeyMask.pall_vlan_deicfi) ? TRUE : FALSE;
	flow_key.key_mask.l3_mask.dscp = (flowKeyMask.P12_dscp) ? TRUE : FALSE;
	flow_key.key_mask.l3_mask.ecn = (flowKeyMask.P12_ecn) ? TRUE : FALSE;

	ASSERT_EQ(ca_flow_key_type_add(G3_DEF_DEVID, &flow_key), CA_E_OK);

#if defined(CONFIG_FC_G3_G3LITE_SERIES)
	/******************************************************
	* profile 4: need Hash to check DOS Attack Prevent and do policing if necessary: syn flood deny
	******************************************************/
	memset(&flow_key, 0, sizeof(flow_key));
	flow_key.key_type = RG_CA_FLOW_CLS_DOS_POL;
	flow_key.prio = RG_CA_FLOW_TUPPLE_PRI_0;
	flow_key.key_mask.l2_keys = TRUE;
	flow_key.key_mask.l3_keys = TRUE;
	flow_key.key_mask.l4_keys = TRUE;

	flow_key.key_mask.l2_mask.o_lspid = TRUE;	// per port configuration
	flow_key.key_mask.l3_mask.ip_protocol = TRUE;	// tcp or icmp
	flow_key.key_mask.l4_mask.tcp_flags= 0x12;	// ack, syn

	ASSERT_EQ(ca_flow_key_type_add(G3_DEF_DEVID, &flow_key), CA_E_OK);
#elif defined(CONFIG_CA8277B_SERIES)

	/******************************************************
	* profile 4: CLS trap and group policer
	******************************************************/
	memset(&flow_key, 0, sizeof(flow_key));
	flow_key.key_type = RG_CA_FLOW_CLS_TRAP_GRPPOL;
	flow_key.prio = RG_CA_FLOW_TUPPLE_PRI_0;
	flow_key.key_mask.l2_keys = TRUE;

	flow_key.key_mask.l2_mask.o_lspid = TRUE;

	ASSERT_EQ(ca_flow_key_type_add(G3_DEF_DEVID, &flow_key), CA_E_OK);
#endif
	

	/****************************************************** 
	* profile 2: MC
	******************************************************/
	/* Initialized by CA driver. ref: aal_hash_init() -> aal_hash_profile_set() */


#if 0 // one profile one tuple, no TTL checking	anymore
	/****************************************************** 
	* TTL pattern mask
	******************************************************/
	memset(&flow_key, 0, sizeof(flow_key));
	flow_key.key_type = RG_CA_FLOW_TYPE_TTL;	//TTL
	flow_key.prio = RG_CA_FLOW_TYPE_TTL;
	flow_key.key_mask.l3_keys = TRUE;
	flow_key.key_mask.l3_mask.ip_ttl = 0xFE;

	ASSERT_EQ(ca_flow_key_type_add(G3_DEF_DEVID, &flow_key), CA_E_OK);


	/****************************************************** 
	* reserved Flow entry
	******************************************************/
	{
		ca_status_t ca_ret = CA_E_OK;
		ca_flow_t flow_config;

		memset(&flow_config, 0, sizeof(flow_config));
		flow_config.key_type = RG_CA_FLOW_TYPE_TTL;
		flow_config.key.l3_key.ip_ttl = 0;
		flow_config.actions.forward = CA_CLASSIFIER_FORWARD_PORT;
		flow_config.actions.dest.port = ASIC_LPORT_CPU_0;

		ca_ret = ca_flow_add(0, &flow_config);

		if(ca_ret == CA_E_OK){
			printk(">>>>> Add flow[%d]\n", flow_config.index);
		}else{
			printk("\033[1;33;41m[WARNING] Add flow type[%d] fail, ca_ret=0x%x \033[0m\n", flow_config.key_type, ca_ret);
		}
		

	}
#endif	
	return CA_E_OK;
}
/*====================
	rtk_rg_g3_policer_init(): Init L2 and L3 policer here.
	dal_ca8279_rate_init():	Init L2 policer only
====================*/
int32 rtk_rg_g3_policer_init(void)
{	
	ca_uint16_t flow_id;
	ca_policer_t policer;
#if defined(CONFIG_CA8277B_SERIES)
	aal_l3_te_glb_cfg_msk_t glb_cfg_msk;
	aal_l3_te_glb_cfg_t glb_cfg;
	TE_POL_FLOW_CFG_t flow_cfg;
	L3FE_GLB_TE_OPTION_t te_option;

	/*Enable 3 policrers*/
	memset(&glb_cfg_msk, 0, sizeof(glb_cfg_msk));
	memset(&glb_cfg, 0, sizeof(glb_cfg));
	glb_cfg_msk.s.sel3flows = 1;
	glb_cfg.sel3flows = 1;
	ASSERT_EQ(aal_l3_te_glb_cfg_set(G3_DEF_DEVID, glb_cfg_msk, &glb_cfg), CA_E_OK);

	/*
	TE_POL_AGRFLOW_CTRL0: policer2[0-31]
	TE_POL_AGRFLOW_CTRL0: policer2[32-63]
	TE_POL_FLOW3_CTRL0: policer3[0-31]
	TE_POL_FLOW3_CTRL1: policer3[32-63]
	*/
	flow_cfg.wrd = rtk_ne_reg_read(TE_POL_FLOW_CFG);
	flow_cfg.bf.watermark_flow2 = 0x1f; //31
	flow_cfg.bf.watermark_flow3 = 0x1f; //31
	rtk_ne_reg_write(flow_cfg.wrd, TE_POL_FLOW_CFG);

	/*CRC length is not included in packet length. extra +4 to take CRC into count for TE.*/
	te_option.wrd = rtk_ne_reg_read(L3FE_GLB_TE_OPTION);
	te_option.bf.bits |= (1<<8); //l3fe_l3te_egr_pktle = original +4 (4 for CRC length)
	te_option.bf.bits |= (1<<9); //l3fe_l3te_ingr_pktlen = original +4 (4 for CRC length)
	te_option.bf.bits |= (1<<10); //set hdri.mcgid to black-hole if l3te_l3fe_drop_code is 6 (pol_drop)
	rtk_ne_reg_write(te_option.wrd, L3FE_GLB_TE_OPTION);
#endif

	/*init l2 and L3 policer here*/
	policer.mode = CA_POLICER_MODE_DISABLE;
	policer.pps = 0;
	policer.cir = RT_RATE_HW_IDX_L2_METER_RATE_MAX;
	policer.cbs = RT_RATE_HW_IDX_L2_METER_BURST_MAX;
	policer.pir = RT_RATE_HW_IDX_L2_METER_RATE_MAX;
	policer.pbs = RT_RATE_HW_IDX_L2_METER_BURST_MAX;
	
	for(flow_id=0;flow_id <= CA_AAL_MAX_FLOW_ID;flow_id++)
	{
		ASSERT_EQ(ca_l2_flow_policer_set(G3_DEF_DEVID,flow_id,&policer), CA_E_OK);
	}

	policer.cir = POL_RATE_MAX;
	policer.cbs = POL_BURST_MAX;
	policer.pir = POL_RATE_MAX;
	policer.pbs = POL_BURST_MAX;
	for(flow_id=0; flow_id <= CA_AAL_L3_MAX_FLOW_ID;flow_id++)
	{
		ASSERT_EQ(ca_l3_flow_policer_set(G3_DEF_DEVID,flow_id,&policer), CA_E_OK);
#if defined(CONFIG_FC_CAG3_SERIES)
		{
			/*For 8277, policer count will be increased no matter if pol_en is enabled. Clear all counter*/
			aal_l3_te_pm_policer_t l3_pm_data;
			ASSERT_EQ(aal_l3_te_pm_policer_flow_get(G3_DEF_DEVID, flow_id, &l3_pm_data), CA_E_OK); //read clear
		}
#endif
	}
	
#if defined(CONFIG_CA8277B_SERIES)
	/*inital policer 2 & policer 3*/
	policer.cir = POL_RATE_MAX;	// for policer 2 & policer 3, rate_m mean 1000 rate_k, follow policer 1 here
	policer.cbs = POL_BURST_MAX;
	policer.pir = POL_RATE_MAX;	// for policer 2 & policer 3, rate_m mean 1000 rate_k, follow policer 1 here
	policer.pbs = POL_BURST_MAX;
	for(flow_id=0; flow_id <= CA_AAL_L3_MAX_FLOW2_ID;flow_id++)
	{
		ASSERT_EQ(ca_l3_flow_policer2_set(G3_DEF_DEVID,flow_id,&policer), CA_E_OK);
	}
	for(flow_id=0; flow_id <= CA_AAL_L3_MAX_FLOW3_ID;flow_id++)
	{
		ASSERT_EQ(ca_l3_flow_policer3_set(G3_DEF_DEVID,flow_id,&policer), CA_E_OK);
	}
#endif

	return CA_E_OK;
}

/*
 * overwrite QM voq profile selection in aal_l3_te_cb_init()
 */
uint32_t rtk_rg_g3_qm_init(void)
{

	return CA_E_OK;
}

uint32_t rtk_rg_g3_acl_init(void)
{
	int i;
	aal_ilpb_cfg_msk_t ilpb_cfg_msk={0};
	aal_ilpb_cfg_t  ilpb_cfg;
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
	aal_elpb_cfg_msk_t elpb_cfg_msk={0};
	aal_elpb_cfg_t	elpb_cfg;
#endif

	//reset all CA cls
	ASSERT_EQ(ca_classifier_rule_delete_all(G3_DEF_DEVID), CA_E_OK);

	//re-configure L2 IGR CLS per port start/length
	memset(&ilpb_cfg_msk, 0, sizeof(aal_ilpb_cfg_msk_t));
	memset(&ilpb_cfg, 0, sizeof(aal_ilpb_cfg_t));
	ilpb_cfg_msk.s.igr_cls_lookup_en = TRUE;
	ilpb_cfg_msk.s.cls_start = TRUE;
	ilpb_cfg_msk.s.cls_length = TRUE;
	ilpb_cfg.igr_cls_lookup_en = 1;

#if defined(CONFIG_ARCH_CORTINA_G3HGU) || defined(CONFIG_ARCH_CORTINA_VENUS)
	/*each hw entry = eth_niX * 8 + eth_ni(X+1) * 8.
		PORT IDX	|	HW IDX(start)	|	LENGTH
		0~7(x)			x*8 				8
		0x10~0x17(y)	64+(y-0x10)*8		8	*/
	for(i = ASIC_LPORT_ETH_NI0; i <= ASIC_LPORT_ETH_NI7; i++){
		ilpb_cfg.cls_start = AAL_PORT_START_FOR_CLS + i*AAL_ILPB_CLS_LENGTH;
		ilpb_cfg.cls_length = AAL_ILPB_CLS_LENGTH;

#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
		//reserve port 5 resource to port 6
		if(i == G3_LOOPBACK_P_RSVSPA){
			ilpb_cfg.cls_start = 0;
			ilpb_cfg.cls_length = 0;
		}
		else if(i == G3_LOOPBACK_P_NEWSPA){
			ilpb_cfg.cls_start = AAL_PORT_START_FOR_CLS + (i-1)*AAL_ILPB_CLS_LENGTH;
			ilpb_cfg.cls_length = AAL_ILPB_CLS_LENGTH*2;
		}
#endif
		
		ASSERT_EQ(aal_port_ilpb_cfg_set(G3_DEF_DEVID, i, ilpb_cfg_msk, &ilpb_cfg), CA_E_OK);
	}
	for(i = ASIC_LPORT_CPU_0; i <= ASIC_LPORT_CPU_7; i++){
		ilpb_cfg.cls_start = AAL_PORT_START_FOR_CLS_CPU + (i-ASIC_LPORT_CPU_0)*AAL_ILPB_CLS_LENGTH_CPU;
		ilpb_cfg.cls_length = AAL_ILPB_CLS_LENGTH_CPU;
		ASSERT_EQ(aal_port_ilpb_cfg_set(G3_DEF_DEVID, i, ilpb_cfg_msk, &ilpb_cfg), CA_E_OK);
	}	
#else	//note CONFIG_ARCH_CORTINA_G3HGU
	for(i = ASIC_LPORT_CPU_2; i <= ASIC_LPORT_CPU_7; i++){		// wifi hwlookup from port 0x17 should bypass l2FE to keep pol_id value
		ilpb_cfg.cls_start = AAL_PORT_START_FOR_CLS_CPU + (i-ASIC_LPORT_CPU_0)*AAL_ILPB_CLS_LENGTH_CPU;
		ilpb_cfg.cls_length = AAL_ILPB_CLS_LENGTH_CPU;
		ASSERT_EQ(aal_port_ilpb_cfg_set(G3_DEF_DEVID, i, ilpb_cfg_msk, &ilpb_cfg), CA_E_OK);
	}
#endif

#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
	//re-configure L2 EGR CLS per port start/length
	memset(&elpb_cfg_msk, 0, sizeof(aal_elpb_cfg_msk_t));
	memset(&elpb_cfg, 0, sizeof(aal_elpb_cfg_t));
	elpb_cfg_msk.s.egr_cls_lookup_en = TRUE;
	elpb_cfg_msk.s.egr_cls_start = TRUE;
	elpb_cfg_msk.s.egr_cls_length = TRUE;
	elpb_cfg.egr_cls_lookup_en = TRUE;

	/*each hw entry = eth_niX * 8 + eth_ni(X+1) * 8.
		PORT IDX	|	HW IDX(start)	|	LENGTH
		0~5(x)			x*8 				8
		6(x)			x*8 				24
		7(x)			(x-1)*8+24			8
		0x10~0x17(y)	0					0
		0x18~0x19(y)	80+(y-0x18)*8		24	*/
	for(i = ASIC_LPORT_ETH_NI0; i <= ASIC_LPORT_ETH_NI7; i++){
		elpb_cfg.egr_cls_start = AAL_PORT_START_FOR_CLS + i*AAL_ELPB_CLS_LENGTH;
		elpb_cfg.egr_cls_length = AAL_ELPB_CLS_LENGTH;

		if(i == G3_LOOPBACK_P_NEWSPA){
			elpb_cfg.egr_cls_start = AAL_PORT_START_FOR_CLS + i*AAL_ELPB_CLS_LENGTH;
			elpb_cfg.egr_cls_length = AAL_ELPB_CLS_MAX_LENGTH;
		}
		else if(i == ASIC_LPORT_ETH_NI7){
			elpb_cfg.egr_cls_start = AAL_PORT_START_FOR_CLS + (i-1)*AAL_ELPB_CLS_LENGTH + AAL_ELPB_CLS_MAX_LENGTH;
			elpb_cfg.egr_cls_length = AAL_ELPB_CLS_LENGTH;
		}

		ASSERT_EQ(aal_port_elpb_cfg_set(G3_DEF_DEVID, i, elpb_cfg_msk, &elpb_cfg), CA_E_OK);
	}
	for(i = ASIC_LPORT_CPU_0; i <= ASIC_LPORT_CPU_7; i++){
		elpb_cfg.egr_cls_start = 0;
		elpb_cfg.egr_cls_length = 0;
		ASSERT_EQ(aal_port_elpb_cfg_set(G3_DEF_DEVID, i, elpb_cfg_msk, &elpb_cfg), CA_E_OK);
	}
	for(i = ASIC_LPORT_L3_WAN; i <= ASIC_LPORT_L3_LAN; i++){
		elpb_cfg.egr_cls_start = (AAL_ELPB_CLS_LENGTH*7 + AAL_ELPB_CLS_MAX_LENGTH) + (i-ASIC_LPORT_L3_WAN)*AAL_ELPB_CLS_MAX_LENGTH;
		elpb_cfg.egr_cls_length = AAL_ELPB_CLS_MAX_LENGTH;
		ASSERT_EQ(aal_port_elpb_cfg_set(G3_DEF_DEVID, i, elpb_cfg_msk, &elpb_cfg), CA_E_OK);
	}
#endif

	//Create L2 igr Classifier with priority 0 for forward all packet to L3FE
	ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_L2_INGRESS_FORWARD_L3FE, NULL), RT_ERR_OK);

#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
 	ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_L2_UPSTREAM_LOOPBACK_INGRESS_FORWARD_L3FE, NULL), RT_ERR_OK);
 	ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_L2_UPSTREAM_LAN_INGRESS_FORWARD_LOOPBACK, NULL), RT_ERR_OK);
#endif

#if defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
	//Create L3 Classifier for hash profile decision
	ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_GENERIC_INTF_HASH_PROFILE_DECISION, NULL), RT_ERR_OK);
#else
	//Create L3 Classifier to trap ipv6 multicast dslite
	ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAdd(RTK_RG_ACLANDCF_RESERVED_MULTICAST_DSLITE_TRAP, NULL), RT_ERR_OK);
#endif

	G3INITPRINT("%s: ##### CA L2/L3 CLS initialization success.#####\n", __func__);

	return CA_E_OK;
}

uint32 rtk_g3_igr_egr_port_vlanFilter_disable(uint32 portId)
{
	aal_ilpb_cfg_t ilpb_cfg;
	aal_ilpb_cfg_msk_t ilpb_msk;
	aal_elpb_cfg_t elpb_cfg;
	aal_elpb_cfg_msk_t elpb_msk;

	memset(&ilpb_cfg,0,sizeof(ilpb_cfg));
	memset(&ilpb_msk,0,sizeof(ilpb_msk));
	memset(&elpb_cfg,0,sizeof(elpb_cfg));
	memset(&elpb_msk,0,sizeof(elpb_msk));

	ilpb_msk.s.ing_fltr_ena=1;
	ilpb_cfg.ing_fltr_ena=0;
	aal_port_ilpb_cfg_set(G3_DEF_DEVID,portId,ilpb_msk,&ilpb_cfg);

	elpb_msk.s.egr_ve_srch_en=1;
	elpb_cfg.egr_ve_srch_en=0;
	aal_port_elpb_cfg_set(G3_DEF_DEVID,portId,elpb_msk,&elpb_cfg);

	return CA_E_OK;
}

#define ILPB_CMP_TPID_CMP_S 			(1<<0)
#define ILPB_CMP_TPID_CMP_C 			(1<<0)
#define ILPB_CMP_TPID_CMP_OTHER 		(1<<0)
#define ILPB_CMP_TPID_CMP_88A8 			(1<<1)
#define ILPB_CMP_TPID_CMP_9100 			(1<<2)
#define ILPB_CMP_TPID_CMP_8100 			(1<<3)

uint32_t rtk_rg_g3_init(void)
{
	int i;
	ca_uint8_t keep=1;
	ca_uint32_t l3fe_tbl_count=4;
	aal_ni_hv_glb_internal_port_id_cfg_t portid_cfg;
	aal_ni_hv_glb_internal_port_id_cfg_mask_t portid_mask={0};
	aal_l3fe_lpb_tbl_cfg_t lpb_cfg;
	aal_l3fe_lpb_tbl_cfg_mask_t lpb_mask={0};
	aal_ilpb_cfg_msk_t ilpb_cfg_msk={0};
	aal_ilpb_cfg_t  ilpb_cfg;
	//ca_gen_intf_attrib_t gen_intf_attrib;

#if 0 //defined(CONFIG_ARCH_CORTINA_G3LITE)
	ca_uint32_t port, queue;
	ca_port_id_t port_id;
	ca_queue_wred_profile_t profile;
	ca_status_t ret;
#endif	

	for(i=0 ; i <=AAL_LPORT_LLID_GEM_INDEX_31 ;i ++)
	{
		//default vlan detection 
		bzero(&ilpb_cfg_msk,sizeof(ilpb_cfg_msk));
		bzero(&ilpb_cfg,sizeof(ilpb_cfg));
		ilpb_cfg_msk.s.top_s_tpid_enable		=1;
		ilpb_cfg_msk.s.top_c_tpid_enable		=1;
		ilpb_cfg_msk.s.inner_s_tpid_enable	=1;
		ilpb_cfg_msk.s.inner_c_tpid_enable	=1;
		ilpb_cfg_msk.s.s_tpid_match			=1;
		ilpb_cfg_msk.s.c_tpid_match			=1;
		
		ilpb_cfg.top_s_tpid_enable 		=1;
		ilpb_cfg.top_c_tpid_enable 		=1;
		ilpb_cfg.inner_s_tpid_enable		=0;
		ilpb_cfg.inner_c_tpid_enable		=1;
		//0x88A8  0x9100 
		ilpb_cfg.s_tpid_match			=ILPB_CMP_TPID_CMP_88A8 | ILPB_CMP_TPID_CMP_9100;	/* 1<<0:PP_TPID_CMP_S 1<<1:0x88A8  1<<2:0x9100 1<<3:0x8100*/
		//0x8100
		ilpb_cfg.c_tpid_match			=ILPB_CMP_TPID_CMP_8100;	/* 1<<0:PP_TPID_CMP_C 1<<1:0x88A8  1<<2:0x9100 1<<3:0x8100*/
		aal_port_ilpb_cfg_set(0,i,ilpb_cfg_msk,&ilpb_cfg);
	}

	//keep lspid unchange
	ASSERT_EQ(aal_l3fe_keep_lspid_unchange_set(0, &keep),CA_E_OK);
	G3INITPRINT("%s: #####Set keep_lspid_unchange to %u#####\n", __func__,keep);

	//let packet from port 7 could enter l2fe
	ASSERT_EQ(aal_ni_hv_glb_internal_port_id_cfg_get(0, &portid_cfg),CA_E_OK);
	portid_mask.bf.wan_rxsel=1;
	portid_cfg.wan_rxsel=2;
	ASSERT_EQ(aal_ni_hv_glb_internal_port_id_cfg_set(0, portid_mask, &portid_cfg),CA_E_OK);
	G3INITPRINT("%s: #####Set wan_rxsel to %u#####\n", __func__,portid_cfg.wan_rxsel);

	//let packet from port 7 act as other LAN ports
	ilpb_cfg_msk.u32[0] = 0;
	ilpb_cfg_msk.u32[1] = 0;
	ilpb_cfg_msk.s.wan_ind = 1;
	ilpb_cfg.wan_ind = 1;
#if defined (CONFIG_RG_G3_SERIES) && defined(CONFIG_RG_G3_WAN_PORT_INDEX)
	ASSERT_EQ(aal_port_ilpb_cfg_set(0, CONFIG_RG_G3_WAN_PORT_INDEX, ilpb_cfg_msk, &ilpb_cfg),CA_E_OK);
#else
	ASSERT_EQ(aal_port_ilpb_cfg_set(0, ASIC_LPORT_ETH_NI7, ilpb_cfg_msk, &ilpb_cfg),CA_E_OK);//FC driver
#endif
	G3INITPRINT("%s: #####Set wan_ind to %u#####\n", __func__,ilpb_cfg.wan_ind);

	//Disable MAC check in L3FE
	lpb_mask.s.mac_da_match_en=1;
	for(i=0;i<l3fe_tbl_count;i++)
	{
		ASSERT_EQ(aal_l3_stg0_lpb_tbl_get(G3_DEF_DEVID, i, &lpb_cfg),CA_E_OK);
		lpb_cfg.mac_da_match_en=0;
		ASSERT_EQ(aal_l3_stg0_lpb_tbl_set(G3_DEF_DEVID, i, lpb_mask, &lpb_cfg),CA_E_OK);
		G3INITPRINT("%s: #####Set stg0tbl[%d]mac_da_match_en to %u#####\n", __func__,i,lpb_cfg.mac_da_match_en);
	}

#if defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
	rtk_rg_asic_fb_init();	// include 8277 and 8277B asic init
#endif

	/******************************************************
	 * L2/L3 Classifier
	 ******************************************************/
	rtk_rg_g3_acl_init();

	/******************************************************
	 * L2/L3 QoS Decision
	 ******************************************************/
	{
		memset(&ilpb_cfg_msk, 0, sizeof(aal_ilpb_cfg_msk_t));
		memset(&ilpb_cfg, 0, sizeof(aal_ilpb_cfg_t));
		ilpb_cfg_msk.s.cos_mode_sel = TRUE;
		ilpb_cfg.cos_mode_sel = AAL_COS_MODE_COS_SEL_BMP;	//default is AAL_COS_MODE_MAP_RX_TOP_DOT1P
		ilpb_cfg_msk.s.cos_control_bm = TRUE;
		ilpb_cfg.cos_control_bm = 0x7;		// bit map(default 0x6), 1: ILPB default setting, 2: VLAN Engine FIB, 4: Ingress CLS FIB
		ilpb_cfg_msk.s.default_cos = TRUE;
		ilpb_cfg.default_cos = 0;

		for(i = ASIC_LPORT_ETH_NI0; i <= ASIC_LPORT_CPU_7; i++){	//port 0~7, port 0x12~0x17
			ASSERT_EQ(aal_port_ilpb_cfg_set(G3_DEF_DEVID, i, ilpb_cfg_msk, &ilpb_cfg), CA_E_OK);
			if(i == ASIC_LPORT_ETH_NI7)
				i = ASIC_LPORT_CPU_2-1;
		}

		memset(&lpb_cfg, 0, sizeof(aal_l3fe_lpb_tbl_cfg_t));
		memset(&lpb_mask, 0, sizeof(aal_l3fe_lpb_tbl_cfg_mask_t));
		lpb_mask.s.default_cos_sel = TRUE;
		lpb_mask.s.default_cos = TRUE;
		lpb_cfg.default_cos_sel = 1; 		//default is 2, 1: LPB default setting, 2: Dot1p mapping, 3: DSCP mapping
		lpb_cfg.default_cos = 0;
			
		for(i = 0; i < l3fe_tbl_count; i++)
			ASSERT_EQ(aal_l3_stg0_lpb_tbl_set(G3_DEF_DEVID, i, lpb_mask, &lpb_cfg),CA_E_OK);

		G3INITPRINT("%s: #####Disable L2FE/L3FE QoS Decision#####\n", __func__);
	}

	/**********
	 * Main Hash
	 **********/
	{
		ca_nat_config_t natConfig;
		
		// call flow tuple init by RG/FC driver
	/*
		rtk_rg_flow_key_mask_t flowKeyMask;
		flowKeyMask.P12_vlanId = TRUE;
		flowKeyMask.P12_vlanPri = TRUE;
		flowKeyMask.P345_vlanId = TRUE;
		flowKeyMask.P345_vlanPri = TRUE;
		flowKeyMask.P345_dscp = TRUE;
		flowKeyMask.P345_ecn = TRUE;
		ASSERT_EQ(rtk_rg_g3_flow_init(flowKeyMask), CA_E_OK);
	*/
		// force disable aging countdown
		ASSERT_EQ(ca_nat_config_get(G3_DEF_DEVID, &natConfig), CA_E_OK);
		natConfig.aging_time = 0;
		ASSERT_EQ(ca_nat_config_set(G3_DEF_DEVID, &natConfig), CA_E_OK);
	}
	
	/**********
	 * Hash Lite
	 **********/
	{
#if defined(CONFIG_CA_G3_G3LITE_SERIES)
		rtk_rg_g3_hashlite_init();
#elif defined(CONFIG_CA8277B_SERIES)
		rtk_8277b_asic_hashlite_init();
#endif
	}

	/**********
	 * VLAN
	 **********/
	{
		int i;
		ca_uint32_t outerTPID[4] = {0x8100, 0x88a8, 0, 0};
		ca_uint32_t innerTPID[4] = {0x8100, 0x88a8, 0, 0};
		ASSERT_EQ(aal_l3fe_pp_tpid_set(2, outerTPID, 2, innerTPID), AAL_E_OK);
		//disable all ilpb/elpb hardware vlan function 
		for(i=ASIC_LPORT_ETH_NI0 ; i< ASIC_LPORT_PON_US_31 ;i++)
		{
			rtk_g3_igr_egr_port_vlanFilter_disable(i);
		}
	}
	{
		aal_l2_vlan_default_cfg_t  cfg;
		aal_l2_vlan_default_cfg_mask_t mask;
		bzero(&cfg,sizeof(cfg));
		bzero(&mask,sizeof(mask));
		mask.s.tx_tpid0=1;
		mask.s.tx_tpid1=1;
		mask.s.tx_tpid2=1;
		mask.s.tx_tpid3=1;
		cfg.tx_tpid0=0x88a8;
		cfg.tx_tpid1=0x9100;
		cfg.tx_tpid2=0x8100;
		cfg.tx_tpid3=0x8100;
		mask.s.cmp_tpid_svlan=1;
		mask.s.cmp_tpid_cvlan=1;
		mask.s.cmp_tpid_other=1;
		//mask.s.cmp_tpid_user=1;
		cfg.cmp_tpid_svlan	=0x88a8;
		cfg.cmp_tpid_cvlan	=0x8100;
		cfg.cmp_tpid_other	=0x8100;
		//cfg.cmp_tpid_user	=0x9100;
		aal_l2_vlan_default_cfg_set(0,mask,&cfg);
	}
	
	/**********
	 * L2
	 **********/
	ASSERT_EQ(ca_l2_aging_mode_set(0, CA_L2_AGING_SOFTWARE), CA_E_OK);
	{
		// set L2 hash mode
		aal_fdb_ctrl_t	ctrl;
		aal_fdb_ctrl_mask_t cmsk;
		aal_fdb_ctrl_get(0,&ctrl);
		ctrl.hashing_mode=0;
		cmsk.wrd = ~0U;
		aal_fdb_ctrl_set(0,cmsk,&ctrl);
	}


	/********************
	*  l3fe_pon_mode which will take the pol_id from hdr-a to l3fe hdr-i
	********************/
	{
		L3FE_GLB_GLB_CFG_t l3fe_glb;
		l3fe_glb.wrd = rtk_ne_reg_read(L3FE_GLB_GLB_CFG);
		l3fe_glb.bf.l3fe_pon_mode = TRUE;
		rtk_ne_reg_write(l3fe_glb.wrd, L3FE_GLB_GLB_CFG);
	}

	/********************
	*  clear mcgroup
	********************/
	{
		ca_l2_mcast_group_delete_all(G3_DEF_DEVID);
		ca_l3_mcast_group_delete_all(G3_DEF_DEVID);
	}

	/**********
	 * L3 policer
	 **********/
	rtk_rg_g3_policer_init();

	/**********
	 * L3 QM
	 **********/
	rtk_rg_g3_qm_init();

#if defined(CONFIG_ARCH_CORTINA_G3HGU) || defined(CONFIG_ARCH_CORTINA_VENUS)
	/*
	 * For 8277A/8277B: remove CA patch for packet stop if random packet size 
	 * the original solution is "Combine TM->L3FE WAN and TM->L3FE LAN into the same port." in aal_port_init()
	 * now CA has new solution to patch L2TM
	 */
	 // rollback
	for(i = 0; i <= 1; i++) {
		aal_port_arb_ldpid_pdpid_map_set(G3_DEF_DEVID, i, 0, ASIC_LPORT_L3_WAN, ASIC_PPORT_L3_WAN);
	}
	// new solution
	// Reg_op w 0xf430237c 0x000800ff
	// Reg_op w 0xf43023a0  0x000800ff
	rtk_ne_reg_write(0x000800ff, L2TM_L2TM_ES_SCH_CFG0 + L2TM_L2TM_ES_SCH_CFG0_STRIDE * 10);	// port 10 - wan to L3FE
	rtk_ne_reg_write(0x000800ff, L2TM_L2TM_ES_SCH_CFG0 + L2TM_L2TM_ES_SCH_CFG0_STRIDE * 13);	// port 13 - lan to L3FE
	rtk_ne_reg_write(0x000800ff, L2TM_L2TM_ES_SCH_CFG0 + L2TM_L2TM_ES_SCH_CFG0_STRIDE * 8);		// port 8 - L2TM to L3QM
	rtk_ne_reg_write(0x000800ff, L2TM_L2TM_ES_SCH_CFG0 + L2TM_L2TM_ES_SCH_CFG0_STRIDE * 9);		// port 9 - L2TM to L3QM

#if defined(CONFIG_ARCH_CORTINA_VENUS)
	{	
		L2TM_L2TM_ES_CTRL_t l2tm_es_ctrl;
		l2tm_es_ctrl.wrd =  rtk_ne_reg_read(L2TM_L2TM_ES_CTRL);
		l2tm_es_ctrl.bf.ni_port_rdy_10_deglitch = 1;
		l2tm_es_ctrl.bf.ni_port_rdy_13_deglitch = 1;
		rtk_ne_reg_write(l2tm_es_ctrl.wrd, L2TM_L2TM_ES_CTRL);
	}
#endif

#endif
	
#if 0 //defined(CONFIG_ARCH_CORTINA_G3LITE)
	memset(&profile, 0, sizeof(ca_queue_wred_profile_t));
	profile.unmarked_dp[0] = 100;
	profile.marked_dp[0] = 100;

	for (port = CA_PORT_ID_NI0; port <= CA_PORT_ID_NI4; port++) 
	{
		port_id = CA_PORT_ID(CA_PORT_TYPE_ETHERNET, port);
		for (queue = 0; queue < 8; queue++) 
		{
			ret = ca_queue_wred_set(0, port_id, queue, &profile);

			if (ret != CA_E_OK)
				printk("%s error set port %x\n", __func__, port_id);
		}
	}
	for (port = CA_PORT_ID_CPU0; port <= CA_PORT_ID_CPU7; port++)
	{
		port_id = CA_PORT_ID(CA_PORT_TYPE_CPU, port);
		for (queue = 0; queue < 8; queue++) 
		{
			ret = ca_queue_wred_set(0, port_id, queue, &profile);

			if (ret != CA_E_OK)
				printk("%s error set port %x\n", __func__, port_id);
		}
	}
#endif

#if defined(CONFIG_ARCH_CORTINA_VENUS)
	/**********
	 * DMALSO, DMA AFT padding init
	 **********/

	{	
		DMA_SEC_DMA_GLB_DMA_LSO_CTRL_t    dma_lso_ctrl;
		DMA_SEC_DMA_AFT_CTRL_t dma_aft_ctrl;
		
		dma_lso_ctrl.wrd = rtk_dma_lso_reg_read(DMA_SEC_DMA_GLB_DMA_LSO_CTRL);
		dma_lso_ctrl.bf.lso_padding_en = 0;
		dma_lso_ctrl.bf.lspid_keep = 1;
		rtk_dma_lso_reg_write(dma_lso_ctrl.wrd, DMA_SEC_DMA_GLB_DMA_LSO_CTRL);
		
		dma_aft_ctrl.wrd = rtk_dma_lso_reg_read(DMA_SEC_DMA_AFT_CTRL);
		dma_aft_ctrl.bf.padding_en	 = 1;
		dma_aft_ctrl.bf.padding_size = 64;
		rtk_dma_lso_reg_write(dma_aft_ctrl.wrd, DMA_SEC_DMA_AFT_CTRL);	

		G3INITPRINT("%s: #####Disable DMALSO padding, Enable DMA AFT padding#####\n", __func__);
	}
#endif

	return CA_E_OK;
}

uint32_t rtk_rg_g3_exit(void)
{
	rtk_rg_g3_cpuport_exit();
	
	return CA_E_OK;
}

#if defined(CONFIG_LUNA_G3_SERIES)	

#include <ca_mut.h>


extern ca_uint8_t l3fe_main_hash_profile_mc;
extern unsigned int hash_mask_multicast_idx;
extern unsigned int hash_mask_multicast_forward_idx;

void array_reverse_copy(unsigned char *dst, unsigned char *src, unsigned int length)
{
	int i;

	for (i = 0; i < length; i++) {
		dst[i] = src[length - i - 1];
	}
}

static inline ca_boolean_t vlan_is_valid(ca_uint16_t vlan)
{
	return !(vlan & 0xF000);
}

#define VID(vlan) (vlan & 0xFFF)

static inline ca_uint16_t vlan_to_hw_vid(ca_uint16_t vlan)
{
	ca_uint16_t hw_vid;

	if (vlan_is_valid(vlan))
		hw_vid = VID(vlan);
	else
		hw_vid = 0;

	return hw_vid;
}

static inline ca_boolean_t mac_is_mcast(ca_mac_addr_t addr)
{
	return (addr[0] & 0x1) == 0x1;
}

static inline ca_boolean_t ipv4_is_mcast(ca_l3_ip_addr_t addr)
{
	return (addr.ipv4_addr & 0xF0000000) == 0xE0000000;
}

static inline ca_boolean_t ipv6_is_mcast(ca_l3_ip_addr_t addr)
{
	return (addr.ipv6_addr[0] & 0xFF000000) == 0xFF000000;
}

static ca_int16_t ip_addr_compare(ca_ip_address_t *a, ca_ip_address_t *b)
{
	if (a->afi == CA_IPV4 && b->afi == CA_IPV4) {
		if (a->ip_addr.ipv4_addr == b->ip_addr.ipv4_addr)
			return 0;
	} else if (a->afi == CA_IPV6 && a->afi == CA_IPV6){
		if (memcmp(a->ip_addr.ipv6_addr, b->ip_addr.ipv6_addr, 16) == 0)
			return 0;
	}

	return -1;
}


static LIST_HEAD(l2_mcg_list);

static inline ca_boolean_t api_level_vlan_range_valid(ca_uint16_t vlan)
{
	return (vlan == CA_UINT16_INVALID || !(vlan & 0xF000));
}

struct sw_mcgid_entry_s {
	ca_uint8_t valid;
	rtl_l2_mcg_t *l2ptr;
};

//software mcgid table
struct sw_mcgid_entry_s sw_mcgid_pool[SW_MCGID_TBL_ENTRY_MAX]={0};


rtl_l2_mcg_t* rtl_l2_mcg_node_alloc(ca_uint16_t mc_llid,ca_uint16_t vlan, ca_ip_address_t grp_addr,ca_ip_address_t src_addr);


static rtl_port_t* rtl_l2_port_node_get(rtl_l2_mcg_t *node, portForwardAction_t *fake_key)
{
	rtl_port_t *port_node;

	if (!node)
		return NULL;
	
	if (!list_empty(&node->port_list))
		list_for_each_entry(port_node, &node->port_list, list)
	{
			if (port_node->key.ldpid == fake_key->ldpid &&
				memcmp(&port_node->key, fake_key, sizeof(portForwardAction_t)) == 0)
				return port_node;
	}

	return NULL;
}

static ca_status_t rtl_l2_port_node_delete(ca_int16_t swMcgid ,rtl_port_t *port_node)
{
	if (port_node == NULL)
		return CA_E_PARAM;

	list_del(&port_node->list);
	aal_port_forward_action_del(swMcgid,&port_node->key);
	kfree(port_node);

	return CA_E_OK;
}

static rtl_port_t* rtl_l2_port_node_alloc(ca_int16_t swMcgid ,portForwardAction_t *portAction)
{
	rtl_port_t *port_node;
	ca_int16_t bit;

	bit = aal_port_forward_action_add(swMcgid,portAction);
	if (bit != CA_E_OK)
		return NULL;
	port_node = kmalloc(sizeof(rtl_port_t), GFP_ATOMIC);
	if (port_node == NULL)
		return NULL;
	memcpy(&port_node->key, portAction, sizeof(portForwardAction_t));

	return port_node;
}


ca_int16_t rtl_swMcgid_alloc(ca_uint16_t *swMcgid)
{

	int ret;

	ret =aal_sw_mcgid_alloc(swMcgid);
	if(CA_E_OK != ret)
		return ret;

	if(sw_mcgid_pool[*swMcgid].valid)
		return CA_E_EXISTS;
	else
		sw_mcgid_pool[*swMcgid].valid=1;
	
	return CA_E_OK;
}

ca_status_t rtl_swMcgid_free(ca_int16_t swMcgid)
{
	int ret;
	
	if(swMcgid < 0 ||  SW_MCGID_TBL_ENTRY_MAX <=swMcgid)
		return CA_E_PARAM;
	
	if(sw_mcgid_pool[swMcgid].valid==0)
		return CA_E_PARAM;

	ret = aal_sw_mcgid_free(swMcgid);
	if(CA_E_OK != ret)
		return ret;

	sw_mcgid_pool[swMcgid].valid=0;
	sw_mcgid_pool[swMcgid].l2ptr=NULL;
	return CA_E_OK;
}



rtl_l2_mcg_t* rtl_l2_mcg_node_alloc(ca_uint16_t mc_llid,ca_uint16_t vlan, ca_ip_address_t grp_addr,ca_ip_address_t src_addr)
{

	rtl_l2_mcg_t *mcg_node=NULL;
	ca_uint16_t  swMcgid=CA_UINT16_INVALID,mce_indx=CA_UINT16_INVALID;

	if(rtl_swMcgid_alloc(&swMcgid)!=CA_E_OK)
		goto ADDL_L2NODE_FAIL;

	mcg_node = kmalloc(sizeof(rtl_l2_mcg_t), GFP_ATOMIC);
	if (mcg_node == NULL)
		goto ADDL_L2NODE_FAIL;		
	
	memset(mcg_node,0, sizeof(rtl_l2_mcg_t));
	
	mcg_node->vlan = vlan;
	mcg_node->llid = mc_llid;
	mcg_node->grp_addr = grp_addr;
	mcg_node->src_addr = src_addr;
	mcg_node->sw_mcgid = swMcgid;
	mcg_node->mce_indx = mce_indx;
	mcg_node->is_knownMcGroup=0;
	mcg_node->hash_idx = CA_UINT32_INVALID;

	//set to pool table
	sw_mcgid_pool[swMcgid].l2ptr=mcg_node;
	
	INIT_LIST_HEAD(&mcg_node->port_list);

	list_add(&mcg_node->list, &l2_mcg_list);

	return mcg_node;
	
ADDL_L2NODE_FAIL:
	if(swMcgid!=CA_UINT16_INVALID)
		rtl_swMcgid_free(swMcgid);

	return NULL;
	
}



ca_status_t rtl_l2_mcast_group_asknown (
	CA_IN ca_device_id_t         device_id,
	CA_IN ca_ip_address_t groupAddr,
	CA_OUT ca_int32_t *hwIdx
	)
{

	aal_hash_key_t key = {0}, *hash_key = &key;
	aal_hash_action_t action = {0}, *hash_action = &action;
	ca_uint32_t h_idx, *hash_idx = &h_idx;
	ca_status_t rc = CA_E_OK;

	
	if (groupAddr.afi == CA_IPV4) 
	{
		if (groupAddr.addr_len != 32 ||!ipv4_is_mcast(groupAddr.ip_addr))
			return CA_E_PARAM;
		if ((groupAddr.addr_len >0) && (groupAddr.afi != CA_IPV4 ||groupAddr.addr_len != 32))
			return CA_E_PARAM;
	} 
	else 
	{
		if (groupAddr.addr_len != 128 ||!ipv6_is_mcast(groupAddr.ip_addr))
			return CA_E_PARAM;
		if ((groupAddr.addr_len >0) && (groupAddr.afi != CA_IPV6 ||groupAddr.addr_len != 128)) 
			return CA_E_PARAM;
	}

	//hash add and assign mcg_node->mce_indx to hash table trao to cpu
	hash_key->ip_vld = 1;
	if (groupAddr.afi == CA_IPV4) 
	{
		hash_key->ip_ver = 0;
		hash_key->ip_da_0 = groupAddr.ip_addr.ipv4_addr;
		hash_key->ip_da_1 = 0;
		hash_key->ip_da_2 = 0;
		hash_key->ip_da_3 = 0;
	} 
	else 
	{
		hash_key->ip_ver = 1;
		hash_key->ip_da_0 = groupAddr.ip_addr.ipv6_addr[3];
		hash_key->ip_da_1 = groupAddr.ip_addr.ipv6_addr[2];
		hash_key->ip_da_2 = groupAddr.ip_addr.ipv6_addr[1];
		hash_key->ip_da_3 = groupAddr.ip_addr.ipv6_addr[0];
	}

	// Note: KEEP ORIGINAL to CPU !!!
	hash_action->dpid_vld = 1;
	hash_action->mcgid = 0x10;
	hash_action->dpid_pri = 1;			// overwrite ldpid decision of previous stage
	hash_action->permit = 1;
	hash_action->no_drop = 1;
	hash_action->no_drop_vld = 1;
//	hash_action->stage3_ctrl = 0;		// no impact
//	hash_action->stage3_ctrl_vld = 1;
	hash_action->chk_msk_ptr = hash_mask_multicast_idx;
	if (CA_E_OK != (rc = aal_hash_add(0, hash_key, hash_action, TUPLE_TYPE_FWD, HASH_AGING_STATIC, hash_mask_multicast_idx, l3fe_main_hash_profile_mc, hash_idx)))
	{
		G3INITPRINT("Hash add failed rc=%d \n",rc);
		goto ERROR;	
	}
	*hwIdx = *hash_idx;

	//G3INITPRINT("Hash add known group at [%d]\n",*hash_idx);

	return CA_E_OK;

	ERROR:
		return CA_E_ERROR;


}


ca_status_t rtl_l2_mcast_group_asunknown (
	CA_IN ca_device_id_t		 device_id,
	CA_IN ca_ip_address_t groupAddr,
	CA_OUT ca_int32_t *hwIdx)
{
	ca_status_t rc = CA_E_OK;

	rc = aal_hash_delete(device_id, *hwIdx);
	if(rc != CA_E_OK)
		G3INITPRINT("group as known Error");	

	return rc;
}


ca_status_t rtl_l2_mcast_group_add (
	CA_IN ca_device_id_t         device_id,
	CA_OUT ca_l2_mcast_entry_t *entry)
{

	rtl_l2_mcg_t *mcg_node=NULL;
	rtl_l2_mcg_t *cur_node;
	struct list_head *tmp;

	if (!entry)
		return CA_E_PARAM;

	if (!api_level_vlan_range_valid(entry->mcast_vlan))
		return CA_E_PARAM;
	
	if (entry->group_ip_addr.afi == CA_IPV4) 
	{
		if (entry->group_ip_addr.addr_len != 32 ||!ipv4_is_mcast(entry->group_ip_addr.ip_addr))
			return CA_E_PARAM;
		if ((entry->src_ip_address.addr_len >0) && (entry->src_ip_address.afi != CA_IPV4 ||entry->src_ip_address.addr_len != 32))
			return CA_E_PARAM;
	} 
	else 
	{
		if (entry->group_ip_addr.addr_len != 128 ||!ipv6_is_mcast(entry->group_ip_addr.ip_addr))
			return CA_E_PARAM;
		if ((entry->src_ip_address.addr_len >0) && (entry->src_ip_address.afi != CA_IPV6 ||entry->src_ip_address.addr_len != 128)) 
			return CA_E_PARAM;
	}


	//node same entry check (key by  group_ip_addr/mcast_vlan/mc_llid/src_ip_address)
	list_for_each(tmp, &l2_mcg_list) 
	{
		cur_node = list_entry(tmp, rtl_l2_mcg_t, list);
	
		if (cur_node->llid == entry->replication_only 
			&& cur_node->vlan == entry->mcast_vlan 
			&& ip_addr_compare(&cur_node->grp_addr, &entry->group_ip_addr) == 0
			&& ip_addr_compare(&cur_node->src_addr, &entry->src_ip_address) == 0 )
		{
			mcg_node = (rtl_l2_mcg_t *)tmp;
		}
	}


	if(mcg_node)
	{
		//MC_DEBUG("Find a exist entry entry->mcg_id=%d\n",entry->mcg_id);
	}
	else
	{
		mcg_node = rtl_l2_mcg_node_alloc(entry->replication_only,entry->mcast_vlan, entry->group_ip_addr, entry->src_ip_address);
		if (mcg_node == NULL)
			return CA_E_FULL;

		//MC_DEBUG("l2_rtl_mcg_node_alloc a new entry \n");

	}
	entry->mcg_id = mcg_node->sw_mcgid;

	return CA_E_OK;


}


ca_status_t rtl_l2_mcast_group_delete (
		CA_IN ca_device_id_t		 device_id,
		CA_IN ca_uint32_t		  swMcgid)
{

	rtl_l2_mcg_t *mcg_node;
	ca_status_t ret;

	if(sw_mcgid_pool[swMcgid].valid==0)
		return CA_E_NOT_FOUND;
	mcg_node = sw_mcgid_pool[swMcgid].l2ptr;

	
	ret = aal_hash_delete(0, mcg_node->hash_idx);
	if(ret != CA_E_OK)
		G3INITPRINT("aal_hash_delete Error");

	ret = aal_hw_mcgid_free(swMcgid,mcg_node->mce_indx);
	if(ret != CA_E_OK)
		G3INITPRINT("aal_hw_mcgid_free Error");

	ret=rtl_swMcgid_free(swMcgid);
	if(ret != CA_E_OK)
		G3INITPRINT("rtl_swMcgid_del Error");

	list_del(&mcg_node->list);
	
	ret = CA_E_OK;
	//MC_DEBUG("free mce_indx=%d hash_idx=%d mcg_id=%d \n",mcg_node->mce_indx,mcg_node->hash_idx,mcg_id);
	kfree(mcg_node);
	
	return ret;

}


ca_status_t rtl_l2_mcast_group_dump (
			CA_IN ca_device_id_t		 device_id,
			CA_IN ca_uint32_t		  swMcgid)
{

	rtl_l2_mcg_t *mcg_node;
	rtl_port_t *port_node, *tmp;
	ca_status_t ret=CA_E_OK;
	char *vlancmd[4]={"NOP","PUSH ","POP","SWAP"};

	if(sw_mcgid_pool[swMcgid].valid==0)
		return CA_E_NOT_FOUND;	
	mcg_node = sw_mcgid_pool[swMcgid].l2ptr;
	
	if (mcg_node)
	{
		printk("====== CA Internal Tbl Info for mcg_id:[%d]======\n",swMcgid);
		printk(" mce_indx=%d hash_idx=%d llid=%d\n",mcg_node->mce_indx,mcg_node->hash_idx,mcg_node->llid);
		printk("PortLists:\n");
		if (!list_empty(&mcg_node->port_list))
			list_for_each_entry_safe(port_node, tmp, &mcg_node->port_list, list) 
			{
				printk("\t\tPortID:%d  egccmd/cvlan/cpri[%s/%d/%d] cos:%d wififlow_id:%d sa_trans:%d Mac=%pM\n",port_node->key.ldpid,vlancmd[port_node->key.vlan_cmd],port_node->key.vlan,port_node->key.vpri,port_node->key.cos,port_node->key.wifi_flowid,port_node->key.mac_sa_cmd,port_node->key.mac_sa);
			}
		printk("====== CA Internal Tbl End ======\n");

	}
	return ret;



}


int _rtl_update_hwMcgidxToHash(rtl_l2_mcg_t *mcg_node)
{

	ca_int16_t mce_indx;
	aal_hash_key_t key = {0}, *hash_key = &key;
	aal_hash_action_t action = {0}, *hash_action = &action;
	ca_uint32_t h_idx;
	ca_status_t rc = CA_E_OK;

	h_idx = mcg_node->hash_idx;
	mce_indx = mcg_node->mce_indx;

				
	hash_key->top_vid = vlan_to_hw_vid(mcg_node->vlan);
	hash_key->ip_vld = 1;


	if (mcg_node->grp_addr.afi == CA_IPV4) {
		hash_key->ip_ver = 0;
		hash_key->ip_da_0 = mcg_node->grp_addr.ip_addr.ipv4_addr;
		hash_key->ip_da_1 = 0;
		hash_key->ip_da_2 = 0;
		hash_key->ip_da_3 = 0;
		
		hash_key->ip_sa_0 = mcg_node->src_addr.ip_addr.ipv4_addr;
		hash_key->ip_sa_1 = 0;
		hash_key->ip_sa_2 = 0;
		hash_key->ip_sa_3 = 0;			
	} else {
		hash_key->ip_ver = 1;
		hash_key->ip_da_0 = mcg_node->grp_addr.ip_addr.ipv6_addr[3];
		hash_key->ip_da_1 = mcg_node->grp_addr.ip_addr.ipv6_addr[2];
		hash_key->ip_da_2 = mcg_node->grp_addr.ip_addr.ipv6_addr[1];
		hash_key->ip_da_3 = mcg_node->grp_addr.ip_addr.ipv6_addr[0];
		
		hash_key->ip_sa_0 = mcg_node->src_addr.ip_addr.ipv6_addr[3];
		hash_key->ip_sa_1 = mcg_node->src_addr.ip_addr.ipv6_addr[2];
		hash_key->ip_sa_2 = mcg_node->src_addr.ip_addr.ipv6_addr[1];
		hash_key->ip_sa_3 = mcg_node->src_addr.ip_addr.ipv6_addr[0];		
	}
	//G3INITPRINT("sip=%x dip=%x vlan=%d \n",hash_key->ip_sa_0,hash_key->ip_da_0,mcg_node->vlan);

#if defined(CONFIG_ARCH_CORTINA_G3HGU)	
{
	hash_action->mac_da_vld = 1;
	if (mcg_node->grp_addr.afi == CA_IPV4) {
		hash_action->mac_da_5 = 0x01;
		hash_action->mac_da_4 = 0x00;
		hash_action->mac_da_3 = 0x5E;
		hash_action->mac_da_2 = (mcg_node->grp_addr.ip_addr.ipv4_addr & 0x7F0000) >> 16;
		hash_action->mac_da_1 = (mcg_node->grp_addr.ip_addr.ipv4_addr & 0xFF00) >> 8;
		hash_action->mac_da_0 = mcg_node->grp_addr.ip_addr.ipv4_addr & 0xFF;
	} else {
		hash_action->mac_da_5 = 0x33;
		hash_action->mac_da_4 = 0x33;
		hash_action->mac_da_3 = (mcg_node->grp_addr.ip_addr.ipv6_addr[3] & 0xFF000000) >> 24;
		hash_action->mac_da_2 = (mcg_node->grp_addr.ip_addr.ipv6_addr[3] & 0xFF0000) >> 16;
		hash_action->mac_da_1 = (mcg_node->grp_addr.ip_addr.ipv6_addr[3] & 0xFF00) >> 8;
		hash_action->mac_da_0 = mcg_node->grp_addr.ip_addr.ipv6_addr[3] & 0xFF;
	}
}
#elif defined(CONFIG_ARCH_CORTINA_VENUS)
{
	//set mac index table at first
	idx_intf_mac_entry_t mac_entry;
	ca_uint8_t mac_da[6];
	int da_rslt_idx = 0;
	int ret;
	memset(&mac_entry, 0, sizeof(mac_entry));
	if (mcg_node->grp_addr.afi == CA_IPV4) {
		mac_da[0] = 0x01;
		mac_da[1] = 0x00;
		mac_da[2] = 0x5E;
		mac_da[3] = (mcg_node->grp_addr.ip_addr.ipv4_addr & 0x7F0000) >> 16;
		mac_da[4] = (mcg_node->grp_addr.ip_addr.ipv4_addr & 0xFF00) >> 8;
		mac_da[5] = mcg_node->grp_addr.ip_addr.ipv4_addr & 0xFF;
	} else {
		mac_da[0] = 0x33;
		mac_da[1] = 0x33;
		mac_da[2] = (mcg_node->grp_addr.ip_addr.ipv6_addr[3] & 0xFF000000) >> 24;
		mac_da[3] = (mcg_node->grp_addr.ip_addr.ipv6_addr[3] & 0xFF0000) >> 16;
		mac_da[4] = (mcg_node->grp_addr.ip_addr.ipv6_addr[3] & 0xFF00) >> 8;
		mac_da[5] = mcg_node->grp_addr.ip_addr.ipv6_addr[3] & 0xFF;
	}
	array_reverse_copy(mac_entry.mac_da, mac_da, 6);
	ret = aal_entry_add(AAL_TABLE_IDX_INTF_MAC, &mac_entry, &da_rslt_idx);
	if(ret != CA_E_OK) {
		return CA_E_ERROR;
	}
	hash_action->mac_da_idx = da_rslt_idx;
	hash_action->mac_da_idx_vld = 1;
}
#else
	G3INITPRINT("\n WARNING: FIXME IF NEW FLATFORM !!!!!!!!!!\n");
#endif


	if(vlan_is_valid(mcg_node->vlan))
	{
		hash_action->vlan_cnt = 1;
		hash_action->top_tpid_enc = 1;
		hash_action->top_dei_sel = 1;
		hash_action->top_vid = vlan_to_hw_vid(mcg_node->vlan);
		hash_action->vlan_vld = 1;
	}
	//assign mce_index		
	hash_action->mc=1;
	hash_action->mcgid = mce_indx;
	hash_action->dpid_vld = 1;
	hash_action->permit = 1;
	hash_action->dpid_pri = 1;	 
	
	hash_action->pppoe_set = 1;
	hash_action->pppoe_vld = 0;
	hash_action->chk_msk_ptr = hash_mask_multicast_forward_idx;

	if(mcg_node->is_knownMcGroup==0)
	{
		if (CA_E_OK != (rc = aal_hash_add(0, hash_key, hash_action, TUPLE_TYPE_FWD, HASH_AGING_STATIC, hash_mask_multicast_forward_idx, l3fe_main_hash_profile_mc, &h_idx)))
		{
			G3INITPRINT("Hash add failed rc=%d \n",rc);
			return rc; 
		}
		mcg_node->hash_idx = h_idx;
		//G3DEBUG("add hash_idx=%d\n",mcg_node->hash_idx);
		mcg_node->is_knownMcGroup=1;

	}
	else
	{
		if (CA_E_OK != (rc = aal_hash_action_update(0, hash_action, TUPLE_TYPE_FWD, hash_mask_multicast_forward_idx, l3fe_main_hash_profile_mc, h_idx)))
		{
			G3INITPRINT("Hash update failed rc=%d \n",rc);
			return rc; 
		}
		//G3DEBUG("add hash_idx=%d update mceIdx to mce_indx=%d \n",mcg_node->hash_idx,mce_indx);

	}

	return CA_E_OK;


}



ca_status_t rtl_l2_port_member_add(ca_device_id_t         device_id, rtl_l2_mcast_group_members_t *members)
{

	rtl_l2_mcg_t *mcg_node=NULL;
	rtl_port_t *port_node = NULL;
	portForwardAction_t action;
	ca_uint32_t i;
	ca_uint32_t sameNodeHit=0;
	ca_uint16_t keepOldMceIdx=CA_UINT16_INVALID;
	int32 ret;


	if(sw_mcgid_pool[members->mcg_id].valid==0)
		return CA_E_NOT_FOUND;
	mcg_node = sw_mcgid_pool[members->mcg_id].l2ptr;

	//hardware ARB_FIB table setting
	for (i = 0; i < members->member_count; i++) 
	{
		memset(&action, 0, sizeof(action));

		action.ldpid = members->member[i].member_port;
		action.wifi_flowid =  members->member[i].wifi_flowid;
		action.cos_en=1;
		action.cos = members->member[i].cos;

		if(members->member[i].action_mask.mac_sa_trans)
		{
			action.mac_sa_cmd = members->member[i].action_mask.mac_sa_trans;
			memcpy(&action.mac_sa, members->member[i].new_mac_sa,sizeof(action.mac_sa));
		}
		
		if (members->member[i].action_mask.vlan_action==RTL_L2_VLAN_TAG_ACTION_NOP || members->member[i].action_mask.vlan_action==RTL_L2_VLAN_TAG_ACTION_POP || members->member[i].action_mask.vlan_action==RTL_L2_VLAN_TAG_ACTION_POP_ALL) 
		{
			//do not assign vlan/vpri/dei 
			action.vlan_cmd = members->member[i].vlan_action;
		}
		else
		{
			action.vlan_cmd = members->member[i].vlan_action;
			action.vpri_cmd = 1;
			action.dei_cmd = 1;
			action.vlan = members->member[i].vid;
			action.vpri =  members->member[i].vpri;
			action.dei = 0;
		}


		//check same action node 
		if (!list_empty(&mcg_node->port_list))
		{
			list_for_each_entry(port_node, &mcg_node->port_list, list) 
			{
				if (port_node->key.ldpid == action.ldpid &&memcmp(&port_node->key, &action, sizeof(portForwardAction_t)) == 0)
				{
					sameNodeHit=1;
					break;
				}
			}
		}
		if(sameNodeHit)
			continue;

		port_node = rtl_l2_port_node_alloc(members->mcg_id,&action);
		if (port_node == NULL)
			return CA_E_FULL;

		list_add(&port_node->list, &mcg_node->port_list);

	}


	keepOldMceIdx=mcg_node->mce_indx;
	ret = aal_hw_mcgid_new(mcg_node->sw_mcgid,&mcg_node->mce_indx);
	if(ret !=AAL_E_OK)
		goto DEL_ADN_FREE;;

	//we should setting hash lastest
	if(_rtl_update_hwMcgidxToHash(mcg_node) != CA_E_OK)
		goto DEL_ADN_FREE;

	//release keepOldMceIdx
	if(keepOldMceIdx != CA_UINT16_INVALID)
	{
		ret=aal_hw_mcgid_free(mcg_node->sw_mcgid,keepOldMceIdx);
		if(ret!= CA_E_OK)
			G3INITPRINT("aal_hw_mcgid_free Error ret:%d keepOldMceIdx=%d",ret,keepOldMceIdx);
	}

	members->mcHwIdx = mcg_node->hash_idx;

	return CA_E_OK;

DEL_ADN_FREE:

	G3INITPRINT("table rtl_l2_port_member_add add hw entry \n");

	return CA_E_OK;

// free hash first


}


ca_status_t rtl_l2_port_member_delete(ca_device_id_t device_id,rtl_l2_mcast_group_members_t *members)
{
	rtl_l2_mcg_t *mcg_node=NULL;
	rtl_port_t *port_node=NULL;
	portForwardAction_t action;
	ca_uint32_t i;
	ca_uint16_t keepOldMceIdx=CA_UINT16_INVALID;
	int32 ret;

	if(sw_mcgid_pool[members->mcg_id].valid==0)
		return CA_E_NOT_FOUND;
	mcg_node = sw_mcgid_pool[members->mcg_id].l2ptr;


	for (i = 0; i < members->member_count; i++) 
	{
		memset(&action, 0, sizeof(action));

		action.ldpid = members->member[i].member_port;
		action.wifi_flowid =  members->member[i].wifi_flowid;
		action.cos_en=1;
		action.cos = members->member[i].cos;

		if(members->member[i].action_mask.mac_sa_trans)
		{
			action.mac_sa_cmd = members->member[i].action_mask.mac_sa_trans;
			memcpy(&action.mac_sa, members->member[i].new_mac_sa,sizeof(action.mac_sa));
		}
		
		if (members->member[i].action_mask.vlan_action==RTL_L2_VLAN_TAG_ACTION_NOP || members->member[i].action_mask.vlan_action==RTL_L2_VLAN_TAG_ACTION_POP || members->member[i].action_mask.vlan_action==RTL_L2_VLAN_TAG_ACTION_POP_ALL) 
		{
			//do not assign vlan/vpri/dei 
			action.vlan_cmd = members->member[i].vlan_action;
		}
		else
		{
			action.vlan_cmd = members->member[i].vlan_action;
			action.vpri_cmd = 1;
			action.dei_cmd = 1;
			action.vlan = members->member[i].vid;
			action.vpri =  members->member[i].vpri;
			action.dei = 0;
		}

		port_node = rtl_l2_port_node_get(mcg_node, &action);
		
		if (port_node != NULL)
			rtl_l2_port_node_delete(mcg_node->sw_mcgid,port_node);
		else{
			G3INITPRINT("search by following key fail: PortID:%d  egccmd/cvlan/cpri[%d/%d/%d] cos:%d wififlow_id:%d sa_trans:%d mac_sa=%pM\n", action.ldpid, action.vlan_cmd, action.vlan, action.vpri, action.cos, action.wifi_flowid, action.mac_sa_cmd,action.mac_sa);
			G3INITPRINT("mcg_node  sw_mcgid %d mce_indx %d", mcg_node->sw_mcgid, mcg_node->mce_indx);
		}
	}

	//we release keepOldMceIdx after update hash 
	keepOldMceIdx=mcg_node->mce_indx;
	ret = aal_hw_mcgid_new(mcg_node->sw_mcgid,&mcg_node->mce_indx);
	if(ret !=AAL_E_OK)
		goto DEL_ADN_FREE;;

	//	we should setting hash lastest
	if(_rtl_update_hwMcgidxToHash(mcg_node) != CA_E_OK)
		goto DEL_ADN_FREE;

	//release oldMceIdx after change hash
	if(keepOldMceIdx != CA_UINT16_INVALID)
	{
		ret=aal_hw_mcgid_free(mcg_node->sw_mcgid,keepOldMceIdx);
		if(ret!= CA_E_OK)
			G3INITPRINT("aal_hw_mcgid_free Error ret:%d keepOldMceIdx=%d",ret,keepOldMceIdx);
	}

	members->mcHwIdx = mcg_node->hash_idx;
	
	return CA_E_OK;

DEL_ADN_FREE:

	G3INITPRINT("table full rtl_l2_port_member_delete hw entry \n");

	return CA_E_OK;
}


EXPORT_SYMBOL(rtl_l2_mcast_group_asknown);
EXPORT_SYMBOL(rtl_l2_mcast_group_asunknown);
EXPORT_SYMBOL(rtl_l2_mcast_group_add);
EXPORT_SYMBOL(rtl_l2_mcast_group_delete);
EXPORT_SYMBOL(rtl_l2_mcast_group_dump);
EXPORT_SYMBOL(rtl_l2_port_member_add);
EXPORT_SYMBOL(rtl_l2_port_member_delete);

#endif


