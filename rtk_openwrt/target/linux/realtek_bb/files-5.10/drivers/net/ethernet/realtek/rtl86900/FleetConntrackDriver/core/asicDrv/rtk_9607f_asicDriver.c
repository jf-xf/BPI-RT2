 /*
  * Copyright (C) 2020 Realtek Semiconductor Corp.
  * All Rights Reserved.
  *
  * This program is the proprietary software of Realtek Semiconductor
  * Corporation and/or its licensors, and only be used, duplicated,
  * modified or distributed under the authorized license from Realtek.
  *
  * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
  * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 */


/*
 * Include Files
 */
#include <stdarg.h>

#include <generated/ca_ne_autoconf.h>
#include "rtk_9607f_asicDriver.h"

#include <aal_common.h>
#include <aal_fdb.h>
#include <aal_port.h>
#include <aal_ni_l2.h>
#include <aal_l2_qos.h>
#include <aal_l2_vlan.h>
#include <aal_l2_te.h>
#include <aal_l2_cls.h>
#include <aal_l2_classification.h>

#include <aal_l3_cls.h>
#include <aal_l3_te_cb.h>
#include <aal_l3_if.h>
#include <aal_l3_cam.h>
#include <aal_l3_pe.h>
#include <aal_l3_glb.h>
#include <aal_l3_specpkt.h>
#include <aal_hash.h>
#include <aal_hashlite.h>

#include <aal_ni.h>

#include <ca_ni.h>
#if 0 //FIXME: 9607F
#include <rt_pe_ext.h>
#endif

#if defined(CONFIG_RTK_L34_MODEL)
// model code
#include <modelAle.h>
#elif defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
// fpga or asic build with FC driver
#include <rtk_fc_internal.h>		// for ASSERT_EQ
#include <rtk_fc_acl.h>
#elif defined(CONFIG_CORTINA_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA_V8)
// fpga only
#include <rtl_glue.h>
#endif

/*
 * Symbol Definition
 */
#define MAX_PRINT_LEN	128


/*
 * Data Declaration
 */
uint32_t asic_debug=0;
rtk_9607f_asic_globalDatabase_t asic_db;
uint8_t l3fe_egress_mirrored_port = AAL_LPORT_INVALID;
uint8_t ldpid_to_pon_port_map_offset = 0x20;
char cpuport_init = 0;


rtk_9607f_asic_netif_ref_t asic_intf[FE_L3E_IF_TBL_ENTRY_MAX]={0};
uint8_t hashType2CacheCtrl[2] = {1, 2}; // TUPLE_TYPE_TYPE0: CacheCtrl set to 1, TUPLE_TYPE_TYPE1: CacheCtrl set to 2

static uint8 hashIntfIdx2HwIntfIdx_mapping[64] =	{	0,	1,	2,	3,	4,	5,	6,	7,	8,	9,	10,	11,	12,	13,	14,	15, /*	 0-15 :for single header, mapping to hw intf 0-15 */
														16,	17,	18,	19,	20,	21,	22,	23,	24,	25,	26,	27,	28,	29,	30,	31, /*	16-31 :for single header, mapping to hw intf 16-31 */
														16,	17,	18,	19,	20,	21,	22,	23,	24,	25,	26,	27,	28,	29,	30,	31,	/*	32-47 :for dual header with 1 Content header buffer, mapping to hw intf 16-31 */
														16,	17,	18,	19,	20,	21,	22,	23,	24,	25,	26,	27,	28,	29,	30,	31,	/*	48-63 :for dual header with 2 Content header buffer, mapping to hw intf 16-31 */
													};



/*
 * Macro Definition
 */
 
char printtmp[256];
#define ASICINITPRINT( comment ,arg...) \
do {\
            int mt_trace_i;\
            sprintf( printtmp, comment,## arg);\
            for(mt_trace_i=1;mt_trace_i<256;mt_trace_i++) \
            { \
                    if(printtmp[mt_trace_i]==0) \
                    { \
                            if(printtmp[mt_trace_i-1]=='\n') printtmp[mt_trace_i-1]=' '; \
                            else break; \
                    } \
            } \
            printk("[HWNAT] %s @%s(%d)\n", printtmp, __FUNCTION__, __LINE__);\
} while(0)
#define ERROR(comment, args...) \
do {\
	if(1) {\
		uint8_t dbg_buf[MAX_PRINT_LEN]; \
		snprintf(dbg_buf, MAX_PRINT_LEN-2, comment,## args);\
		strcat(dbg_buf, " "); \
		printk("\033[1;33;41m[ASIC] %s\033[1;30;40m@%s(%d))\033[0m\n", dbg_buf, __FUNCTION__, __LINE__); \
	}\
} while(0)

#define DEBUG(comment, args...) \
do {\
	if(asic_debug) {\
		uint8_t dbg_buf[MAX_PRINT_LEN]; \
		snprintf(dbg_buf, MAX_PRINT_LEN-2, comment,## args);\
		strcat(dbg_buf, " "); \
		printk("\033[1;36;40m[ASIC] %s\033[1;30;40m@%s(%d))\033[0m\n", dbg_buf, __FUNCTION__, __LINE__); \
	}\
} while(0)

#define PARAM_CHK(expr, errCode)\
do {\
	if ((uint32_t)(expr)) { \
		ERROR("invalid parameter, ret = 0x%x", errCode); \
        return errCode; \
    }\
} while (0)

/*
 * Function Implementation
 */


static asic_ret_t rtk_9607f_asic_ni_init(void)
{
	asic_ret_t ret = ASIC_RET_SUCCESS;
	{
		// l3 egress mirror
		aal_ni_hv_glb_internal_port_id_cfg_mask_t niGlbCfgMask={0};
		aal_ni_hv_glb_internal_port_id_cfg_t niGlbCfg={0};

		niGlbCfgMask.wrd = 0;
		niGlbCfgMask.bf.egr_mrr_sel = TRUE;
		niGlbCfgMask.bf.mrr_dsel = TRUE;
		niGlbCfgMask.bf.mrr_ldpid = TRUE;
		niGlbCfgMask.bf.mrr_cos = TRUE;
		niGlbCfgMask.bf.mrr_fe_bypass = TRUE;
		niGlbCfg.egr_mrr_sel = 1; // L3FE egress mirror enabled
		niGlbCfg.mrr_dsel = 0x3; // mirrored packet destination is L2TM(0x2) if mirror is enabled, destination is L2FE(0x3) if mirror is disabled
		niGlbCfg.mrr_ldpid = AAL_LPORT_BLACKHOLE;
		niGlbCfg.mrr_cos = 0;
		niGlbCfg.mrr_fe_bypass = 1;
		aal_ni_hv_glb_internal_port_id_cfg_set(RTK_ASIC_DEVID, niGlbCfgMask, &niGlbCfg);
	}
	return ret;
}

static asic_ret_t rtk_9607f_asic_l2fe_init(void)
{
	int i=0;
	asic_ret_t ret = ASIC_RET_SUCCESS;
	{
		// dpb tabl: to bypass l2fe if ldpid is ASIC_LPORT_L3FEMC (0x20 ) or ASIC_LPORT_PON_US (0x21~0x27)

		aal_dpb_cfg_msk_t dpbCfg_msk;
		aal_dpb_cfg_t dpbCfg;
		int lpid, cos;

		// change multicaast port to 0x20
		aal_port_dpb_ctrl_set(RTK_ASIC_DEVID, (ASIC_LPORT_L3FEMC >> 3)&0x7);

		dpbCfg_msk.s.bypass =1;
		dpbCfg_msk.s.ldpid =1;
		dpbCfg_msk.s.cos =1;
		dpbCfg.bypass=1;
		dpbCfg.ldpid=ASIC_LPORT_MC;
		dpbCfg.cos=0;
		/*
		From L3FE or QM ports traffic uses the ldpid[5:3] to match with this pattern. If
		matched, the {ldpid[2:0], cos[2:0]} will be used to search the DPB table to get
		the remapped ldpid and cos.
		*/
		lpid = ASIC_LPORT_L3FEMC;
		// mc data downstream ldpid = 0x20, bypass l2fe
		for(cos = 0 ; cos < 8 ; cos++)
		{
			if(aal_port_dpb_cfg_set(RTK_ASIC_DEVID, ((lpid & 0x7) <<3) | (cos&0x7) , dpbCfg_msk, &dpbCfg)) {
				ERROR("ERROR, aal api fail" );
			}
		}

	}
	
	{
		// FDB
		lrn_fwd_ctrl_mask_t fdbFwdCtrlMask={0};
		lrn_fwd_ctrl_t fdbFwdCtrl={0};
		aal_fdb_ctrl_t	ctrl={0};
		aal_fdb_ctrl_mask_t cmsk={0};

		fdbFwdCtrlMask.wrd = 0;
		fdbFwdCtrlMask.bf.sa_limit_excd_fwd_ctrl = TRUE;
		fdbFwdCtrlMask.bf.sa_hash_cllsn_fwd_ctrl = TRUE;
		fdbFwdCtrl.limit_excd_mode = SA_LRN_FWD_CTRL_FORWARD;	// forward if over limit
		fdbFwdCtrl.hash_cllsn_mode = SA_LRN_FWD_CTRL_FORWARD;	// prevent drop src: sa_mis_or_sa_limit_drp_14

		if(aal_fdb_lrn_fwd_ctrl_set(RTK_ASIC_DEVID, fdbFwdCtrlMask, &fdbFwdCtrl)) {
			ERROR("ERROR, aal api fail" );
		}

		// learning/aging/hashing mode - for HGU & Hybrid application
		cmsk.wrd = 0;
		cmsk.s.lrn_mode = 1;
		cmsk.s.aging_mode = 1;
		cmsk.s.hashing_mode = 1;
		ctrl.lrn_mode = CA_AAL_FDB_LRN_MODE_HW;	// allow PPTP port hw learning
		ctrl.aging_mode = 0;	// 1: software aging mode. do not timeout; 0: age decreased to zero(entry aged out) in hardware aging mode,
		ctrl.hashing_mode = 0;	// 2'b00, hashing algorithm is XOR.
		if(aal_fdb_ctrl_set(0, cmsk, &ctrl)){
			ERROR("ERROR, aal api fail" );
		}

		for(i = 0; i <= ASIC_LPORT_ETH_NI7; i++) {
			ASSERT_EQ(aal_fdb_port_limit_set(RTK_ASIC_DEVID, i, 0), CA_E_OK);
		}
	}
#if 0//FIXME: 9607F
	{
		/* for PE dbuf_flag*/
		ca_uint8_t dbuf_id = 7;
		aal_arb_port_dbuf_config_mask_t dbuf_config_mask;
		aal_arb_port_dbuf_config_t port_dbuf;

		dbuf_config_mask.wrd = 0;
		memset(&port_dbuf, 0, sizeof(port_dbuf));
		port_dbuf.dbuf_flg              = TRUE;
		port_dbuf.lspid_vld 			= TRUE;
		port_dbuf.lspid 				= RT_PE_HTTP_TEST_DMA_LSO_LSPID;
		dbuf_config_mask.bf.dbuf_flg	= TRUE;
		dbuf_config_mask.bf.lspid_vld	= TRUE;
		dbuf_config_mask.bf.lspid 		= TRUE;
		aal_arb_port_dbuf_set(RTK_ASIC_DEVID, dbuf_id, dbuf_config_mask, &port_dbuf);
	}
#endif
	{
		// ARB pdpid: L2TM physical port mapping
		uint32_t mymac = 0;
		uint32_t ldpid = 0;

		for(mymac = 0; mymac <= 1; mymac++) 
		{
			//aal_arb_pdpid_map_set

			/*
			 * multicast downstream: ldpid = 0x0~0x6, 0x12~0x17, deepq = 1
			 */
			for(ldpid = ASIC_LPORT_ETH_NI0; ldpid <= ASIC_LPORT_ETH_NI4; ldpid++) {
				if(aal_port_arb_ldpid_pdpid_map_set(RTK_ASIC_DEVID, mymac, 0, ldpid, AAL_PPORT_QM)){
					ERROR("ERROR, aal api fail" );
				}
				if(aal_port_arb_ldpid_pdpid_map_set(RTK_ASIC_DEVID, mymac, 1, ldpid, AAL_PPORT_QM)){
					ERROR("ERROR, aal api fail" );
				}
			}
			for(ldpid = ASIC_LPORT_ETH_NI5; ldpid < ASIC_LPORT_ETH_NI7; ldpid++) {
				if(aal_port_arb_ldpid_pdpid_map_set(RTK_ASIC_DEVID, mymac, 0, ldpid, AAL_PPORT_QM)){
					ERROR("ERROR, aal api fail" );
				}
				if(aal_port_arb_ldpid_pdpid_map_set(RTK_ASIC_DEVID, mymac, 1, ldpid, AAL_PPORT_QM)){
					ERROR("ERROR, aal api fail" );
				}
			}
			for(ldpid = ASIC_LPORT_CPU_0; ldpid <= ASIC_LPORT_CPU_7; ldpid++) {
				if(aal_port_arb_ldpid_pdpid_map_set(RTK_ASIC_DEVID, mymac, 0, ldpid, AAL_PPORT_CPU)){
					ERROR("ERROR, aal api fail" );
				}
				if(aal_port_arb_ldpid_pdpid_map_set(RTK_ASIC_DEVID, mymac, 1, ldpid, AAL_PPORT_CPU)){
					ERROR("ERROR, aal api fail" );
				}
			}
			// 0x18
			if(aal_port_arb_ldpid_pdpid_map_set(RTK_ASIC_DEVID, mymac, 0, ASIC_LPORT_L3_WAN, AAL_PPORT_L3_WAN)){
				ERROR("ERROR, aal api fail" );
			}
			if(aal_port_arb_ldpid_pdpid_map_set(RTK_ASIC_DEVID, mymac, 1, ASIC_LPORT_L3_WAN, AAL_PPORT_L3_WAN)){
				ERROR("ERROR, aal api fail" );
			}
			// 0x19
			if(aal_port_arb_ldpid_pdpid_map_set(RTK_ASIC_DEVID, mymac, 0, ASIC_LPORT_L3_LAN, AAL_PPORT_L3_LAN)){
				ERROR("ERROR, aal api fail" );
			}
			if(aal_port_arb_ldpid_pdpid_map_set(RTK_ASIC_DEVID, mymac, 1, ASIC_LPORT_L3_LAN, AAL_PPORT_L3_LAN)){
				ERROR("ERROR, aal api fail" );
			}

		}
	}
	{
		//Dual VXLAN/L2TP packet parser init
		int i;
		
		for(i=0; i<L3PP_L2TP_SPORT_TBL_SIZE_MAX; i++)
			aal_l3pp_l2tp_sport_set(i, L2TP_SPORT); // 0x06A5
		for(i=0; i<L3PP_VXLAN_DPORT_TBL_SIZE_MAX; i++)
			aal_l3pp_vxlan_dport_set(i, VXLAN_DPORT); // 0x12B5 
	}
	return ret;
}

static asic_ret_t rtk_9607f_asic_l3fe_init(void)
{
	asic_ret_t ret = ASIC_RET_SUCCESS;
	int aal_ret;
	int i = 0;
	L3FE_GLB_GLB_CFG_t glbctrl;
	l3fe_hash_aqm_cfg_t aqm_cfg;
	L3FE_PP_dual_dos_en_t pp_dual_dos_en;
	L3FE_PE_PE_CFG_t pe_cfg;
	L3FE_PE_PE_CFG2_t pe_cfg2;

	glbctrl.wrd = rtk_9607f_asic_ne_reg_read(L3FE_GLB_GLB_CFG);
	glbctrl.bf.l3fe_pon_mode = TRUE;
	glbctrl.bf.mc_port = ASIC_LPORT_L3FEMC;
	rtk_9607f_asic_ne_reg_write(glbctrl.wrd, L3FE_GLB_GLB_CFG);

	{
		// trap to cpu if TTL-1 <=0
		// aal_hash_ttl_zero_ctrl()
		L3FE_GLB_RES_CTRL_t res_ctrl;

		res_ctrl.bf.ttl_0_cpu_en = 1; /* 0: Drop, 1: Forward to cpu */
		res_ctrl.bf.ttl_0_cpu_ldpid = AAL_LPORT_CPU_0;
		res_ctrl.bf.ttl_0_cpu_deepq = 0;
		res_ctrl.bf.ttl_0_cpu_deepq_vld = 0;
		res_ctrl.bf.ttl_0_cpu_keep_org_pkt = 1;
		res_ctrl.bf.ttl_0_cpu_keep_org_pkt_vld = 1;
		res_ctrl.bf.ttl_0_cpu_ctrl_bypass = 1;

		rtk_9607f_asic_ne_reg_write(res_ctrl.wrd, L3FE_GLB_RES_CTRL);
	}
	{
		int aal_ret;
		memset(&aqm_cfg, 0, sizeof(l3fe_hash_aqm_cfg_t));
		aal_ret = aal_l3fe_hash_aqm_cfg_set(aqm_cfg);
		if(aal_ret)
		{
			printk("Fail to set aqm cfg, ret = %d", aal_ret);
			ret = ASIC_RET_FAIL;
		}
	}
	// enable DOS for dual header
	{
		pp_dual_dos_en.bf.en = 0xFFFF;
		rtk_9607f_asic_ne_reg_write(pp_dual_dos_en.wrd, L3FE_PP_dual_dos_en);
	}
	
	// to cpu ldpid remapping
	{
		aal_cpu_remap_cfg_mask_t remap_msk;
		aal_cpu_remap_cfg_t remap;
		aal_cpu_remap_hash_algo_t algo;
		
		memset(&remap_msk, 0, sizeof(remap_msk));
		memset(&remap, 0, sizeof(remap));
		memset(&algo, 0, sizeof(algo));		
		
		remap_msk.flow_miss_en = TRUE;
		remap_msk.olspid_hash_en = TRUE;
		remap_msk.smac_hash_en = TRUE;
		remap_msk.dmac_hash_en = TRUE;
		remap_msk.l3sip_hash_en = TRUE;
		remap_msk.l3dip_hash_en = TRUE;
		remap_msk.l4sport_hash_en = TRUE;
		remap_msk.l4dport_hash_en = TRUE;
		remap_msk.cpu_pid_mask = TRUE;

		remap.flow_miss_en = FALSE;		// init as disabled
		remap.olspid_hash_en = TRUE;
		remap.smac_hash_en = TRUE;
		remap.dmac_hash_en = TRUE;
		remap.l3sip_hash_en = TRUE;
		remap.l3dip_hash_en = TRUE;
		remap.l4sport_hash_en = TRUE;
		remap.l4dport_hash_en = TRUE;
		remap.cpu_pid_mask = 0x3;			// 0x10 and 0x11
		
		if((aal_ret = aal_l3_glb_cpu_remap_cfg_set(RTK_ASIC_DEVID, remap_msk, remap))) {	
			ERROR("Fail, ret = %d", aal_ret);
			return ASIC_RET_FAIL;
		}

		algo.left_rotation = 1;
		algo.shift_0_to_7 = 4;
		if((aal_ret = aal_l3_glb_cpu_remap_hashsmac_set(RTK_ASIC_DEVID, algo))) {	// 0xB0000
			ERROR("Fail, ret = %d", aal_ret);
			return ASIC_RET_FAIL;
		}
		
		algo.left_rotation = 0;
		algo.shift_0_to_7 = 4;
		if((aal_ret = aal_l3_glb_cpu_remap_hashdmac_set(RTK_ASIC_DEVID, algo))) {	// 0x40000
			ERROR("Fail, ret = %d", aal_ret);
			return ASIC_RET_FAIL;
		}

		algo.left_rotation = 1;
		algo.shift_0_to_7 = 7;
		if((aal_ret = aal_l3_glb_cpu_remap_hashl3sip_set(RTK_ASIC_DEVID, algo))) {	// 0xF00000
			ERROR("Fail, ret = %d", aal_ret);
			return ASIC_RET_FAIL;
		}
		
		algo.left_rotation = 1;
		algo.shift_0_to_7 = 7;
		if((aal_ret = aal_l3_glb_cpu_remap_hashl3dip_set(RTK_ASIC_DEVID, algo))) {	// 0xF00000
			ERROR("Fail, ret = %d", aal_ret);
			return ASIC_RET_FAIL;
		}
		
		algo.left_rotation = 1;
		algo.shift_0_to_7 = 4;
		if((aal_ret = aal_l3_glb_cpu_remap_hashl4sport_set(RTK_ASIC_DEVID, algo))) {	// 0xB0000
			ERROR("Fail, ret = %d", aal_ret);
			return ASIC_RET_FAIL;
		}
		
		algo.left_rotation = 0;
		algo.shift_0_to_7 = 4;
		if((aal_ret = aal_l3_glb_cpu_remap_hashl4dport_set(RTK_ASIC_DEVID, algo))) {	// 0x40000
			ERROR("Fail, ret = %d", aal_ret);
			return ASIC_RET_FAIL;
		}

		for(i=0; i<=CPU_LDPID_HASH_RESULT_15; i++) {
			if((aal_ret = aal_l3_glb_cpu_remap_dpid_set(RTK_ASIC_DEVID, i, i%L3QM_EQ_PROFILE_CPU_COUNT))) {
				ERROR("Fail, ret = %d", aal_ret);
				return ASIC_RET_FAIL;
			}
		}
	}
	
	// set vlan snap format
	{
		pe_cfg.wrd = rtk_9607f_asic_ne_reg_read(L3FE_PE_PE_CFG);
		pe_cfg.bf.snap_fmt_tra = 1; //snap and vlan format of trasform mode, 0: original format vlan + snap 1: new format snap + vlan
		pe_cfg.bf.snap_fmt_bri = 0; //snap and vlan format of trasform mode, 0: original format vlan + snap 1: new format snap + vlan
		rtk_9607f_asic_ne_reg_write(pe_cfg.wrd, L3FE_PE_PE_CFG);
	}
	// set PE EOP protection redirect port
	{
		pe_cfg2.bf.pe_prot_ldpid = AAL_LPORT_CPU_0;
		rtk_9607f_asic_ne_reg_write(pe_cfg2.wrd, L3FE_PE_PE_CFG2);
	}

	return ret;
}

static asic_ret_t rtk_9607f_asic_l3te_init(void)
{
	asic_ret_t ret = ASIC_RET_SUCCESS;
	TE_TE_GLB_CTRL_t ctrl;

	ctrl.wrd = rtk_9607f_asic_ne_reg_read(TE_TE_GLB_CTRL);
	ctrl.bf.cntrcfg_clear_on_read = 1;
	rtk_9607f_asic_ne_reg_write(ctrl.wrd, TE_TE_GLB_CTRL);

	return ret;
}

static asic_ret_t rtk_9607f_asic_l3qm_init(void)
{
	asic_ret_t ret = ASIC_RET_SUCCESS;

	/* Init L3QM to PON configuration */
	{
		// enable gemid map mode - (hdr_i.t1_ctrl[3]==0 &&  hdr_i.t1_ctrl[0]==1) => (hdr_a.ldpid = ldpid_base  + hdr_i.t2_ctrl[3:0])
		ret = aal_l3pe_pe_gemid_map_set(1);
		ret = aal_l3pe_pe_ldpid_base_set(ASIC_LPORT_PON_US_0);
	}

	{
		// QM_UPPER_LDPID_MAP.pon_offset = 0x20	=> scfg: CFG_ID_L3QM_LDPID_TO_PON_PORT_MAP_OFFSET
	}

	// NI_HV_GLB_INTERNAL_PORT_ID_CFG.wan_rxsel = 2	=> rtk_9607f_init


	
	{
		// Overwrite aal_ni_init_nirx_l3fe_demux()	
		/*
		 * 8277A: 0x20 mc data -> L2FE
		 *		 0x21~0x2F invalid ldpid
		 * 8277B: 0x20 mc data -> L2FE
		 *		 0x21~0x27 pon upstream -> L2FE (->L2TM->L3QM->WAN)
		 *		 0x28~0x2F pon upstream -> WAN
		  * 8277C: 0x20 mc data if !deepq, pon upstream if deepq
		 *		 0x21~0x2F pon upstream -> WAN
		 */
		ca_ni_rx_l3fe_demux_t demux;
		  
		demux.wrd = 0;
		demux.bf.ldpid_0	= NIRX_L3FE_DEMUX_ID_L2FE;
		demux.bf.ldpid_1	= NIRX_L3FE_DEMUX_ID_L3QM;
		demux.bf.ldpid_2	= NIRX_L3FE_DEMUX_ID_L3QM;
		demux.bf.ldpid_3	= NIRX_L3FE_DEMUX_ID_L3QM;
		demux.bf.ldpid_4	= NIRX_L3FE_DEMUX_ID_L3QM;
		demux.bf.ldpid_5	= NIRX_L3FE_DEMUX_ID_L3QM;
		demux.bf.ldpid_6	= NIRX_L3FE_DEMUX_ID_L3QM;
		demux.bf.ldpid_7	= NIRX_L3FE_DEMUX_ID_L3QM;
		demux.bf.ldpid_8	= NIRX_L3FE_DEMUX_ID_L3QM;
		demux.bf.ldpid_9	= NIRX_L3FE_DEMUX_ID_L3QM;
		demux.bf.ldpid_10	= NIRX_L3FE_DEMUX_ID_L3QM;
		demux.bf.ldpid_11	= NIRX_L3FE_DEMUX_ID_L3QM;
		demux.bf.ldpid_12	= NIRX_L3FE_DEMUX_ID_L3QM;
		demux.bf.ldpid_13	= NIRX_L3FE_DEMUX_ID_L3QM;
		demux.bf.ldpid_14	= NIRX_L3FE_DEMUX_ID_L3QM;
		demux.bf.ldpid_15	= NIRX_L3FE_DEMUX_ID_L3QM;

		// when header-A's DEEP_Q is NOT set
		demux.bf.ldpid_0	= NIRX_L3FE_DEMUX_ID_L2FE;
		rtk_ne_reg_write(demux.wrd, NI_HV_GLB_NIRX_L3FE_DEMUX_CFG2);
		// when header-A's DEEP_Q is set
		demux.bf.ldpid_0	= NIRX_L3FE_DEMUX_ID_L3QM;
		rtk_ne_reg_write(demux.wrd, NI_HV_GLB_NIRX_L3FE_DPQ_DEMUX_CFG2);
	}


	return ret;
}

asic_ret_t rtk_9607f_asic_netifTable_add(uint32_t idx, rtk_9607f_asic_netif_entry_t *pNetif)
{
	asic_ret_t ret = ASIC_RET_SUCCESS;
	fe_l3e_if_tbl_entry_t eif;
	l3_cam_mac_da_tbl_entry_t l3_cam_mac;
	uint32_t mtu_idx=CA_UINT32_INVALID, mac_idx=CA_UINT32_INVALID;
	uint8_t validMac = 1;

	PARAM_CHK(pNetif==NULL, ASIC_RET_NULL_POINTER);

	if((pNetif->gateway_mac_addr.octet[0] | pNetif->gateway_mac_addr.octet[1] | pNetif->gateway_mac_addr.octet[2] |
		pNetif->gateway_mac_addr.octet[3] | pNetif->gateway_mac_addr.octet[4] | pNetif->gateway_mac_addr.octet[5])==0) {
		validMac = 0;
	}

	// support update
	if(asic_intf[idx].valid==1) {
		rtk_9607f_asic_netifTable_del(idx);
	}

	// mtu
	if(pNetif->intf_mtu_check) {
		ret = aal_l3pe_mtu_size_add(pNetif->intf_mtu, &mtu_idx);
		if (ret!= AAL_E_OK) {
			ERROR("add netif[%d] mtu %d fail", idx, pNetif->intf_mtu);
			return ASIC_RET_FAIL;
		}
	}

	// mac
	if(validMac) {
		memset(&l3_cam_mac, 0x0, sizeof(l3_cam_mac_da_tbl_entry_t));

		l3_cam_mac.mac_0 = pNetif->gateway_mac_addr.octet[0];
		l3_cam_mac.mac_1 = pNetif->gateway_mac_addr.octet[1];
		l3_cam_mac.mac_2 = pNetif->gateway_mac_addr.octet[2];
		l3_cam_mac.mac_3 = pNetif->gateway_mac_addr.octet[3];
		l3_cam_mac.mac_4 = pNetif->gateway_mac_addr.octet[4];
		l3_cam_mac.mac_5 = pNetif->gateway_mac_addr.octet[5];
		l3_cam_mac.vld   = 1;

		ret = aal_l3_cam_mac_da_lookup(pNetif->gateway_mac_addr.octet, &mac_idx);
		if (ret == AAL_E_OK) {
			// increase reference count
			ret = aal_entry_add_by_idx(AAL_TABLE_L3_CAM_MAC_DA, &l3_cam_mac, mac_idx);
		}else {
			// add a new one
			ret = aal_entry_add(AAL_TABLE_L3_CAM_MAC_DA, &l3_cam_mac, &mac_idx);
		}

		if (ret != AAL_E_OK) {
			mac_idx = CA_UINT32_INVALID;
			if (mtu_idx != CA_UINT32_INVALID)
				aal_l3pe_mtu_size_del(mtu_idx);

			ERROR("add netif[%d] mac %pM fail ret=%d", idx, &pNetif->gateway_mac_addr.octet,ret);
			return ASIC_RET_FAIL;
		}
	}

	// if
	{
		memset(&eif, 0, sizeof(fe_l3e_if_tbl_entry_t));

		// In 8277, sa translation will be done by main hash fib.
		if(mac_idx != CA_UINT32_INVALID) {
			eif.mac_sa_vld = 1;
			eif.mac_sa_an_sel = (mac_idx+1);
		}

		if(pNetif->out_pppoe_act == FB_NETIFPPPOE_ACT_REMOVE) {
			// 8277B
			// for non-pppoe interface: remove tag if smac_trans == 0
			eif.pppoe_set = 1;
			eif.pppoe_vld = 0;
			eif.pppoe_session_id = 0;
		}else if((pNetif->out_pppoe_act == FB_NETIFPPPOE_ACT_ADD) || (pNetif->out_pppoe_act == FB_NETIFPPPOE_ACT_MODIFY)) {
			// for pppoe interace: add tag
			eif.pppoe_set = 1;
			eif.pppoe_vld = 1;
			eif.pppoe_session_id = pNetif->out_pppoe_sid;
		}else{
			// for non-pppoe interface: keep tag for passthrough
			eif.pppoe_set = 0;
			eif.pppoe_vld = 0;
			eif.pppoe_session_id = 0;
		}

		//default remove padding len from pppoe len
		eif.pppoe_len_control=1;
		if(pNetif->out_pppoe_len_include_padding)
			eif.pppoe_len_control = 0;
		
		//default remove padding len from SNAP L2 len
		eif.snap_tra_len_control=1;
		if(pNetif->out_snap_tra_len_include_padding)
			eif.snap_tra_len_control = 0;

		//default remove padding len from SNAP L2 len
		eif.snap_bri_len_control=1;
		if(pNetif->out_snap_bri_len_include_padding)
			eif.snap_bri_len_control = 0;

		ret = aal_entry_add_by_idx(AAL_TABLE_FE_L3E_IF_TBL, &eif, idx);
		if (ret != AAL_E_OK) {
			if (mtu_idx != CA_UINT32_INVALID)
				aal_l3pe_mtu_size_del(mtu_idx);

			if (mac_idx != CA_UINT32_INVALID)
				aal_entry_del(AAL_TABLE_L3_CAM_MAC_DA, mac_idx);

			ERROR("add netif[%d] mac %pM fail", idx, &pNetif->gateway_mac_addr.octet);
			return ASIC_RET_FAIL;
		}
	}

	asic_intf[idx].valid = 1;
	asic_intf[idx].mtu_check = pNetif->intf_mtu_check;
	asic_intf[idx].mtu_idx = mtu_idx;
	asic_intf[idx].mac_idx = mac_idx;

	DEBUG("Add netif[%d] mtuidx[%d] macidx[%d]", idx, mtu_idx, mac_idx);

	return ret;
}

asic_ret_t rtk_9607f_asic_netifTable_del(uint32_t idx)
{
	asic_ret_t ret = ASIC_RET_SUCCESS;

	PARAM_CHK(idx>=FE_L3E_IF_TBL_ENTRY_MAX, ASIC_RET_OUT_OF_RANGE);

	if(asic_intf[idx].valid == 0) {
		return ASIC_RET_SUCCESS;
	}

	if((ret = aal_entry_del(AAL_TABLE_FE_L3E_IF_TBL, idx)) != AAL_E_OK) {
		ERROR("fail to delete netif[%d], ret = %d", idx, ret);
	}

	if(asic_intf[idx].mtu_check && asic_intf[idx].mtu_idx <= 15){
		if((ret = aal_l3pe_mtu_size_del(asic_intf[idx].mtu_idx)) != AAL_E_OK) {
			ERROR("fail to delete netif[%d] mtuidx[%d], ret = %d", idx, asic_intf[idx].mtu_idx, ret);
		}
	}

	if(asic_intf[idx].mac_idx <= L3_CAM_MAC_DA_TBL_ENTRY_MAX) {
		if((ret = aal_entry_del(AAL_TABLE_L3_CAM_MAC_DA, asic_intf[idx].mac_idx)) != AAL_E_OK) {
			ERROR("fail to delete netif[%d] macidx[%d], ret = %d", idx, asic_intf[idx].mac_idx, ret);
		}
	}

	DEBUG("Del netif[%d] mtuidx[%d] macidx[%d]", idx, asic_intf[idx].mtu_idx, asic_intf[idx].mac_idx);

	memset(&asic_intf[idx], 0, sizeof(rtk_9607f_asic_netif_ref_t));

	return ret;
}

asic_ret_t rtk_9607f_flow_default_action_update(uint32_t profileIdx, rtk_9607f_asic_flow_defAct_type_t act)
{
	if(act == RTK_9607F_FLOW_DEFACT_TYPE_TRAP)
	{
		aal_hash_profile_defAct_update(profileIdx, asic_db.flow_defAct_idx[RTK_9607F_FLOW_DEFACT_TYPE_TRAP], asic_db.flow_defAct_idx[RTK_9607F_FLOW_DEFACT_TYPE_TRAP]);
	}
	else if(act == RTK_9607F_FLOW_DEFACT_TYPE_DROP)
	{
		aal_hash_profile_defAct_update(profileIdx, asic_db.flow_defAct_idx[RTK_9607F_FLOW_DEFACT_TYPE_DROP], asic_db.flow_defAct_idx[RTK_9607F_FLOW_DEFACT_TYPE_TRAP]);
	}
	else if(act == RTK_ASIC_FLOW_DEFACT_TYPE_TRAP_WITH_METER)
	{
		aal_hash_profile_defAct_update(profileIdx, asic_db.flow_defAct_idx[RTK_ASIC_FLOW_DEFACT_TYPE_TRAP_WITH_METER], asic_db.flow_defAct_idx[RTK_ASIC_FLOW_DEFACT_TYPE_TRAP_WITH_METER]);
	}
	else
	{
		ERROR("%s: [ERROR] unknown default action type (%d)\n", __func__, act);
		return ASIC_RET_NOT_SUPPORT;
	}
	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_asic_flow_default_action_meter_update(uint32_t profileIdx, uint32_t flow_meter_idx)
{
	hash_action_default_t defaultAct_trap_meter;
	hash_default_action_entry_t default_action;

	if(profileIdx != RTK_ASIC_FLOW_DEFACT_TYPE_TRAP_WITH_METER)
		return ASIC_RET_NOT_SUPPORT;
	
	aal_entry_get_by_idx(AAL_TABLE_HASH_DEFAULT_ACTION, asic_db.flow_defAct_idx[RTK_ASIC_FLOW_DEFACT_TYPE_TRAP_WITH_METER],  &default_action);
	memcpy(&defaultAct_trap_meter, &default_action, sizeof(defaultAct_trap_meter));
	defaultAct_trap_meter.pol_vld = 1;
	defaultAct_trap_meter.pol_grp_en = 1;
	defaultAct_trap_meter.pol_grp_id = (flow_meter_idx & 0x7);
	defaultAct_trap_meter.pol23_vld = 1;
	defaultAct_trap_meter.pol2_msb_en = 1;
	defaultAct_trap_meter.pol2_id_msb = (flow_meter_idx >> 3);
	memcpy(&default_action, &defaultAct_trap_meter, sizeof(defaultAct_trap_meter));
	aal_entry_set(AAL_TABLE_HASH_DEFAULT_ACTION, asic_db.flow_defAct_idx[RTK_ASIC_FLOW_DEFACT_TYPE_TRAP_WITH_METER],  &default_action);
	
	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_asic_mirrored_port_set(bool valid, uint8_t ldpid)
{
	if(!valid)
		l3fe_egress_mirrored_port = AAL_LPORT_INVALID;
	else
		l3fe_egress_mirrored_port = ldpid;

	return ASIC_RET_SUCCESS;
}

asic_ret_t _rtk_9607f_asic_flow_key_gen(rtk_9607f_asic_pathValue_t in_path, void* flow_key, aal_hash_key_t *hash_key)
{
	uint8_t tpid_idx;
	uint16_t vlan_tpid;

	// hash key init
	memset(hash_key, 0, sizeof(aal_hash_key_t));

	/* hash key generation*/
	if(in_path == FB_PATH_12)
	{
		rtk_9607f_asic_flow_config_key_path12_t *pPath12_key = (rtk_9607f_asic_flow_config_key_path12_t *)flow_key;
		// === key: L2 ===
		hash_key->orig_lspid = pPath12_key->orig_lspid;
		hash_key->mdata = pPath12_key->pon_streamId_or_wifi_devIdx;

		hash_key->vlan_cnt = pPath12_key->ctag_if + pPath12_key->stag_if;
		if(pPath12_key->ctag_if && pPath12_key->stag_if)
		{
			hash_key->top_vid = pPath12_key->svlan_id;
			hash_key->top_8021p = pPath12_key->svlan_pri;
			hash_key->top_dei = pPath12_key->svlan_dei;
			vlan_tpid = pPath12_key->svlan_tpid;
			if(aal_l3fe_pp_top_tpid_get(vlan_tpid, &tpid_idx) != AAL_E_OK)
			{
				DEBUG("Unknown TPID 0x%x", vlan_tpid);
				return ASIC_RET_NOT_SUPPORT;
			}
			hash_key->top_tpid_enc = tpid_idx;
			hash_key->inner_vid = pPath12_key->cvlan_id;
			hash_key->inner_8021p = pPath12_key->cvlan_pri;
			hash_key->inner_dei = pPath12_key->cvlan_cfi;
			vlan_tpid = pPath12_key->cvlan_tpid;
			if(aal_l3fe_pp_top_tpid_get(vlan_tpid, &tpid_idx) != AAL_E_OK)
			{
				DEBUG("Unknown TPID 0x%x", vlan_tpid);
				return ASIC_RET_NOT_SUPPORT;
			}
			hash_key->inner_tpid_enc = tpid_idx;
		}
		else if(pPath12_key->stag_if)
		{
			hash_key->top_vid = pPath12_key->svlan_id;
			hash_key->top_8021p = pPath12_key->svlan_pri;
			hash_key->top_dei = pPath12_key->svlan_dei;
			vlan_tpid = pPath12_key->svlan_tpid;
			if(aal_l3fe_pp_top_tpid_get(vlan_tpid, &tpid_idx) != AAL_E_OK)
			{
				DEBUG("Unknown TPID 0x%x", vlan_tpid);
				return ASIC_RET_NOT_SUPPORT;
			}
			hash_key->top_tpid_enc = tpid_idx;
		}
		else if(pPath12_key->ctag_if)
		{
			hash_key->top_vid = pPath12_key->cvlan_id;
			hash_key->top_8021p = pPath12_key->cvlan_pri;
			hash_key->top_dei = pPath12_key->cvlan_cfi;
			vlan_tpid = pPath12_key->cvlan_tpid;
			if(aal_l3fe_pp_top_tpid_get(vlan_tpid, &tpid_idx) != AAL_E_OK)
			{
				DEBUG("Unknown TPID 0x%x", vlan_tpid);
				return ASIC_RET_NOT_SUPPORT;
			}
			hash_key->top_tpid_enc = tpid_idx;
		}

		hash_key->mac_sa_5 = pPath12_key->src_mac[0];
		hash_key->mac_sa_4 = pPath12_key->src_mac[1];
		hash_key->mac_sa_3 = pPath12_key->src_mac[2];
		hash_key->mac_sa_2 = pPath12_key->src_mac[3];
		hash_key->mac_sa_1 = pPath12_key->src_mac[4];
		hash_key->mac_sa_0 = pPath12_key->src_mac[5];

		hash_key->mac_da_5 = pPath12_key->dst_mac[0];
		hash_key->mac_da_4 = pPath12_key->dst_mac[1];
		hash_key->mac_da_3 = pPath12_key->dst_mac[2];
		hash_key->mac_da_2 = pPath12_key->dst_mac[3];
		hash_key->mac_da_1 = pPath12_key->dst_mac[4];
		hash_key->mac_da_0 = pPath12_key->dst_mac[5];

		hash_key->ethertype_enc = pPath12_key->ethertype_enc;

		// === key: L3 ===
		hash_key->ip_vld = pPath12_key->ip_vld;
		hash_key->ip_dscp = pPath12_key->ip_dscp;
		hash_key->ip_ecn = pPath12_key->ip_ecn;
	}
	else if(in_path == FB_PATH_34)
	{
		rtk_9607f_asic_flow_config_key_path34_t *pPath34_key = (rtk_9607f_asic_flow_config_key_path34_t *)flow_key;

		// === key: L2 ===
		hash_key->orig_lspid = pPath34_key->orig_lspid;
		hash_key->mdata = pPath34_key->pon_streamId_or_wifi_devIdx;

		hash_key->vlan_cnt = pPath34_key->ctag_if + pPath34_key->stag_if;
		if(pPath34_key->ctag_if && pPath34_key->stag_if)
		{
			hash_key->top_vid = pPath34_key->svlan_id;
			hash_key->top_8021p = pPath34_key->svlan_pri;
			hash_key->top_dei = pPath34_key->svlan_dei;
			vlan_tpid = pPath34_key->svlan_tpid;
			if(aal_l3fe_pp_top_tpid_get(vlan_tpid, &tpid_idx) != AAL_E_OK)
			{
				DEBUG("Unknown TPID 0x%x", vlan_tpid);
				return ASIC_RET_NOT_SUPPORT;
			}
			hash_key->top_tpid_enc = tpid_idx;
			hash_key->inner_vid = pPath34_key->cvlan_id;
			hash_key->inner_8021p = pPath34_key->cvlan_pri;
			hash_key->inner_dei = pPath34_key->cvlan_cfi;
			vlan_tpid = pPath34_key->cvlan_tpid;
			if(aal_l3fe_pp_top_tpid_get(vlan_tpid, &tpid_idx) != AAL_E_OK)
			{
				DEBUG("Unknown TPID 0x%x", vlan_tpid);
				return ASIC_RET_NOT_SUPPORT;
			}
			hash_key->inner_tpid_enc = tpid_idx;
		}
		else if(pPath34_key->stag_if)
		{
			hash_key->top_vid = pPath34_key->svlan_id;
			hash_key->top_8021p = pPath34_key->svlan_pri;
			hash_key->top_dei = pPath34_key->svlan_dei;
			vlan_tpid = pPath34_key->svlan_tpid;
			if(aal_l3fe_pp_top_tpid_get(vlan_tpid, &tpid_idx) != AAL_E_OK)
			{
				DEBUG("Unknown TPID 0x%x", vlan_tpid);
				return ASIC_RET_NOT_SUPPORT;
			}
			hash_key->top_tpid_enc = tpid_idx;
		}
		else if(pPath34_key->ctag_if)
		{
			hash_key->top_vid = pPath34_key->cvlan_id;
			hash_key->top_8021p = pPath34_key->cvlan_pri;
			hash_key->top_dei = pPath34_key->cvlan_cfi;
			vlan_tpid = pPath34_key->cvlan_tpid;
			if(aal_l3fe_pp_top_tpid_get(vlan_tpid, &tpid_idx) != AAL_E_OK)
			{
				DEBUG("Unknown TPID 0x%x", vlan_tpid);
				return ASIC_RET_NOT_SUPPORT;
			}
			hash_key->top_tpid_enc = tpid_idx;
		}
		if(pPath34_key->pppoetag_if)
		{
			hash_key->pppoe_type = 2; //pppoe session
			hash_key->pppoe_session_id = pPath34_key->pppoe_sid;
		}

#if defined(CONFIG_REALTEK_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA_V8)
		// To compatible testcases. 5 tuple flow not care SMAC/DMAC.
#else
		hash_key->mac_sa_5 = pPath34_key->src_mac[0];
		hash_key->mac_sa_4 = pPath34_key->src_mac[1];
		hash_key->mac_sa_3 = pPath34_key->src_mac[2];
		hash_key->mac_sa_2 = pPath34_key->src_mac[3];
		hash_key->mac_sa_1 = pPath34_key->src_mac[4];
		hash_key->mac_sa_0 = pPath34_key->src_mac[5];

		hash_key->mac_da_5 = pPath34_key->dst_mac[0];
		hash_key->mac_da_4 = pPath34_key->dst_mac[1];
		hash_key->mac_da_3 = pPath34_key->dst_mac[2];
		hash_key->mac_da_2 = pPath34_key->dst_mac[3];
		hash_key->mac_da_1 = pPath34_key->dst_mac[4];
		hash_key->mac_da_0 = pPath34_key->dst_mac[5];
#endif

		// === key: L3 ===
		hash_key->ip_vld = TRUE;
		hash_key->ip_ver = pPath34_key->ipv4_or_ipv6;	// 1: IPv6, 0: IPv4
#if defined(CONFIG_REALTEK_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA_V8)
		if(pPath34_key->l4_proto_mode)
			hash_key->ip_protocol = pPath34_key->l4proto_num;
		else
			hash_key->ip_protocol = pPath34_key->l4proto? 0x6 : 0x11;	// 1: TCP, 0: UDP
#else
		hash_key->ip_protocol = pPath34_key->l4proto_num;
#endif
		hash_key->ip_dscp = pPath34_key->ip_dscp;
		hash_key->ip_ecn = pPath34_key->ip_ecn;

		hash_key->ip_sa_3 = pPath34_key->ip_sa[0]; /* MSB */
		hash_key->ip_sa_2 = pPath34_key->ip_sa[1];
		hash_key->ip_sa_1 = pPath34_key->ip_sa[2];
		hash_key->ip_sa_0 = pPath34_key->ip_sa[3];
		hash_key->ip_da_3 = pPath34_key->ip_da[0]; /* MSB */
		hash_key->ip_da_2 = pPath34_key->ip_da[1];
		hash_key->ip_da_1 = pPath34_key->ip_da[2];
		hash_key->ip_da_0 = pPath34_key->ip_da[3];

		// === key: L4 ===
		hash_key->l4_sp_exact_range = pPath34_key->l4_src_port;
		hash_key->l4_dp_exact_range = pPath34_key->l4_dst_port;
	}
	else if(in_path == FB_PATH_5)
	{
		rtk_9607f_asic_flow_config_key_path5_t *pPath5_key = (rtk_9607f_asic_flow_config_key_path5_t *)flow_key;

		// === key: L2 ===
		hash_key->orig_lspid = pPath5_key->orig_lspid;
		hash_key->mdata = pPath5_key->pon_streamId_or_wifi_devIdx;

		hash_key->vlan_cnt = pPath5_key->ctag_if + pPath5_key->stag_if;
		if(pPath5_key->ctag_if && pPath5_key->stag_if)
		{
			hash_key->top_vid = pPath5_key->svlan_id;
			hash_key->top_8021p = pPath5_key->svlan_pri;
			hash_key->top_dei = pPath5_key->svlan_dei;
			vlan_tpid = pPath5_key->svlan_tpid;
			if(aal_l3fe_pp_top_tpid_get(vlan_tpid, &tpid_idx) != AAL_E_OK)
			{
				DEBUG("Unknown TPID 0x%x", vlan_tpid);
				return ASIC_RET_NOT_SUPPORT;
			}
			hash_key->top_tpid_enc = tpid_idx;
			hash_key->inner_vid = pPath5_key->cvlan_id;
			hash_key->inner_8021p = pPath5_key->cvlan_pri;
			hash_key->inner_dei = pPath5_key->cvlan_cfi;
			vlan_tpid = pPath5_key->cvlan_tpid;
			if(aal_l3fe_pp_top_tpid_get(vlan_tpid, &tpid_idx) != AAL_E_OK)
			{
				DEBUG("Unknown TPID 0x%x", vlan_tpid);
				return ASIC_RET_NOT_SUPPORT;
			}
			hash_key->inner_tpid_enc = tpid_idx;
		}
		else if(pPath5_key->stag_if)
		{
			hash_key->top_vid = pPath5_key->svlan_id;
			hash_key->top_8021p = pPath5_key->svlan_pri;
			hash_key->top_dei = pPath5_key->svlan_dei;
			vlan_tpid = pPath5_key->svlan_tpid;
			if(aal_l3fe_pp_top_tpid_get(vlan_tpid, &tpid_idx) != AAL_E_OK)
			{
				DEBUG("Unknown TPID 0x%x", vlan_tpid);
				return ASIC_RET_NOT_SUPPORT;
			}
			hash_key->top_tpid_enc = tpid_idx;
		}
		else if(pPath5_key->ctag_if)
		{
			hash_key->top_vid = pPath5_key->cvlan_id;
			hash_key->top_8021p = pPath5_key->cvlan_pri;
			hash_key->top_dei = pPath5_key->cvlan_cfi;
			vlan_tpid = pPath5_key->cvlan_tpid;
			if(aal_l3fe_pp_top_tpid_get(vlan_tpid, &tpid_idx) != AAL_E_OK)
			{
				DEBUG("Unknown TPID 0x%x", vlan_tpid);
				return ASIC_RET_NOT_SUPPORT;
			}
			hash_key->top_tpid_enc = tpid_idx;
		}
		if(pPath5_key->pppoetag_if)
		{
			hash_key->pppoe_type = 2; //pppoe session
			hash_key->pppoe_session_id = pPath5_key->pppoe_sid;
		}


#if defined(CONFIG_REALTEK_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA_V8)
		// To compatible testcases. 5 tuple flow not care SMAC/DMAC.
#else
		hash_key->mac_sa_5 = pPath5_key->src_mac[0];
		hash_key->mac_sa_4 = pPath5_key->src_mac[1];
		hash_key->mac_sa_3 = pPath5_key->src_mac[2];
		hash_key->mac_sa_2 = pPath5_key->src_mac[3];
		hash_key->mac_sa_1 = pPath5_key->src_mac[4];
		hash_key->mac_sa_0 = pPath5_key->src_mac[5];

		hash_key->mac_da_5 = pPath5_key->dst_mac[0];
		hash_key->mac_da_4 = pPath5_key->dst_mac[1];
		hash_key->mac_da_3 = pPath5_key->dst_mac[2];
		hash_key->mac_da_2 = pPath5_key->dst_mac[3];
		hash_key->mac_da_1 = pPath5_key->dst_mac[4];
		hash_key->mac_da_0 = pPath5_key->dst_mac[5];
#endif

		// === key: L3 ===
		hash_key->ip_vld = TRUE;
		hash_key->ip_ver = pPath5_key->ipv4_or_ipv6;	// 1: IPv6, 0: IPv4
#if defined(CONFIG_REALTEK_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA_V8)
		if(pPath5_key->l4_proto_mode)
			hash_key->ip_protocol = pPath5_key->l4proto_num;
		else
			hash_key->ip_protocol = pPath5_key->l4proto? 0x6 : 0x11;	// 1: TCP, 0: UDP
#else
		hash_key->ip_protocol = pPath5_key->l4proto_num;
#endif
		hash_key->ip_dscp = pPath5_key->ip_dscp;
		hash_key->ip_ecn = pPath5_key->ip_ecn;

		hash_key->ip_sa_3 = pPath5_key->ip_sa[0]; /* MSB */
		hash_key->ip_sa_2 = pPath5_key->ip_sa[1];
		hash_key->ip_sa_1 = pPath5_key->ip_sa[2];
		hash_key->ip_sa_0 = pPath5_key->ip_sa[3];
		hash_key->ip_da_3 = pPath5_key->ip_da[0]; /* MSB */
		hash_key->ip_da_2 = pPath5_key->ip_da[1];
		hash_key->ip_da_1 = pPath5_key->ip_da[2];
		hash_key->ip_da_0 = pPath5_key->ip_da[3];

		// === key: L4 ===
		hash_key->l4_sp_exact_range = pPath5_key->l4_src_port;
		hash_key->l4_dp_exact_range = pPath5_key->l4_dst_port;
	}
	else if(in_path == FB_PATH_MC)
	{
		/*** This is for CRC calculation only ***/
		rtk_9607f_asic_flow_config_key_mc_t *pMc_key = (rtk_9607f_asic_flow_config_key_mc_t *)flow_key;

		hash_key->vlan_cnt = pMc_key->ctag_if + pMc_key->stag_if;
		// MC profile: care vid and 8021p only for VLAN fileds.
		if(pMc_key->ctag_if && pMc_key->stag_if)
		{
			hash_key->top_vid = pMc_key->svlan_id;
			hash_key->top_8021p = pMc_key->svlan_pri;
			hash_key->inner_vid = pMc_key->cvlan_id;
			hash_key->inner_8021p = pMc_key->cvlan_pri;
		}
		else if(pMc_key->stag_if)
		{
			hash_key->top_vid = pMc_key->svlan_id;
			hash_key->top_8021p = pMc_key->svlan_pri;
		}
		else if(pMc_key->ctag_if)
		{
			hash_key->top_vid = pMc_key->cvlan_id;
			hash_key->top_8021p = pMc_key->cvlan_pri;
		}

		hash_key->ip_vld = TRUE;
		hash_key->ip_ver = pMc_key->ipv4_or_ipv6;	// 1: IPv6, 0: IPv4

		hash_key->ip_sa_3 = pMc_key->ip_sa[0]; /* MSB */
		hash_key->ip_sa_2 = pMc_key->ip_sa[1];
		hash_key->ip_sa_1 = pMc_key->ip_sa[2];
		hash_key->ip_sa_0 = pMc_key->ip_sa[3];
		hash_key->ip_da_3 = pMc_key->ip_da[0]; /* MSB */
		hash_key->ip_da_2 = pMc_key->ip_da[1];
		hash_key->ip_da_1 = pMc_key->ip_da[2];
		hash_key->ip_da_0 = pMc_key->ip_da[3];
	}
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	else if(in_path == FB_PATH_MC_WIFI_AMSDU_TX)
	{
		rtk_9607f_asic_flow_config_key_pathMc_wifi_amsdu_tx_t *pPathMc_wifi_amsdu_tx_key = (rtk_9607f_asic_flow_config_key_pathMc_wifi_amsdu_tx_t *)flow_key;

		// === key: L2 ===
		hash_key->mdata = pPathMc_wifi_amsdu_tx_key->wifi_mac_id;

		hash_key->mac_da_5 = pPathMc_wifi_amsdu_tx_key->dst_mac[0];
		hash_key->mac_da_4 = pPathMc_wifi_amsdu_tx_key->dst_mac[1];
		hash_key->mac_da_3 = pPathMc_wifi_amsdu_tx_key->dst_mac[2];
		hash_key->mac_da_2 = pPathMc_wifi_amsdu_tx_key->dst_mac[3];
		hash_key->mac_da_1 = pPathMc_wifi_amsdu_tx_key->dst_mac[4];
		hash_key->mac_da_0 = pPathMc_wifi_amsdu_tx_key->dst_mac[5];
	}
#endif
	return ASIC_RET_SUCCESS;
}

asic_ret_t _rtk_9607f_asic_flow_action_gen(rtk_9607f_asic_pathValue_t in_path, void* flow_key, void* flow_act, aal_hash_action_t *hash_action, uint32_t profileIdx, uint32_t tupleIdx)
{
	uint8_t tpid_idx;
	uint8_t is_from_ponOrWifi = RTK_9607F_FLOW_FROM_OTHER;
	bool is_toWifi = FALSE;

	// hash action init
	memset(hash_action, 0, sizeof(aal_hash_action_t));
	hash_action->smac_trans = 1;

	/* hash action generation*/
	if(in_path == FB_PATH_12)
	{
		rtk_9607f_asic_flow_config_key_path12_t *pPath12_key = (rtk_9607f_asic_flow_config_key_path12_t *)flow_key;
		rtk_9607f_asic_flow_config_action_path12_t *pPath12_act = (rtk_9607f_asic_flow_config_action_path12_t *)flow_act;
		is_from_ponOrWifi = pPath12_key->is_from_ponRx_wifi_rx;
		// === action: L2 ===
		if(pPath12_act->out_smac_trans)
		{
			hash_action->smac_trans = 0; // use l3 interface to do SMAC trans
			hash_action->ip_ttl_dec1 = 1;
			hash_action->ip_ttl_zero_discard_en1 = 1;
			//mtu
			if(asic_intf[hashIntfIdx2HwIntfIdx_mapping[pPath12_act->egress_intf_idx]].valid && asic_intf[hashIntfIdx2HwIntfIdx_mapping[pPath12_act->egress_intf_idx]].mtu_check)
			{
				//follow flow action
				hash_action->ip_mtu_enc_vld1 = TRUE;
				hash_action->ip_mtu_enc1 = asic_intf[hashIntfIdx2HwIntfIdx_mapping[pPath12_act->egress_intf_idx]].mtu_idx;
			}
		}
		else if(pPath12_key->ip_vld == FALSE)
		{
			// for birdge packet: Modiy outer DMAC+SMAC+SVLAN+CVLAN according to main hash fib. Other packet contents keep as orinal one in PE module.
			hash_action->modify_vlan_only_vld1 = TRUE;
			hash_action->modify_vlan_only1 = TRUE;
		}

		if(pPath12_act->out_dmac_trans)
		{
			hash_action->mac_da_idx_vld = TRUE;
			hash_action->mac_da_idx = pPath12_act->mac_da_idx;
		}
		{
			//vlan: set tpid and vid only, no touch vlan dei
			if(pPath12_act->vlan_act)
			{
				//vlan set mode
				hash_action->vlan_vld1 = TRUE;
				hash_action->vlan_cnt1 = pPath12_act->vlan_cnt;
				if(hash_action->vlan_cnt1 == 2)
				{
					if(aal_l3fe_pp_top_tpid_get(pPath12_act->outer_tpid, &tpid_idx) != AAL_E_OK)
					{
						DEBUG("Unknown TPID 0x%x", pPath12_act->outer_tpid);
						return ASIC_RET_NOT_SUPPORT;
					}
					hash_action->top_tpid_enc1 = tpid_idx;
					hash_action->top_vid1 = pPath12_act->outer_vlanid;
					hash_action->top_dei1 = pPath12_act->outer_dei;
					hash_action->top_802_1p_update_en = TRUE;
					hash_action->top_802_1p1 = pPath12_act->outer_vlanpri;

					if(aal_l3fe_pp_top_tpid_get(pPath12_act->inner_tpid, &tpid_idx) != AAL_E_OK)
					{
						DEBUG("Unknown TPID 0x%x", pPath12_act->inner_tpid);
						return ASIC_RET_NOT_SUPPORT;
					}
					hash_action->inner_tpid_enc1 = tpid_idx;
					hash_action->inner_vid1 = pPath12_act->inner_vlanid;
					hash_action->inner_dei1 = pPath12_act->inner_dei;
					hash_action->inner_802_1p_update_en = TRUE;
					hash_action->inner_802_1p1 = pPath12_act->inner_vlanpri;
				}
				else if(hash_action->vlan_cnt1 == 1)
				{
					if(aal_l3fe_pp_top_tpid_get(pPath12_act->outer_tpid, &tpid_idx) != AAL_E_OK)
					{
						DEBUG("Unknown TPID 0x%x", pPath12_act->outer_tpid);
						return ASIC_RET_NOT_SUPPORT;
					}
					hash_action->top_tpid_enc1 = tpid_idx;
					hash_action->top_vid1 = pPath12_act->outer_vlanid;
					hash_action->top_dei1 = pPath12_act->outer_dei;
					hash_action->top_802_1p_update_en = TRUE;
					hash_action->top_802_1p1 = pPath12_act->outer_vlanpri;
				}
			}
		}

		// === action: L3 ===
		if(pPath12_act->ip_dscp_update_en)
		{
			hash_action->ip_dscp_update_en1 = TRUE;
			hash_action->ip_dscp1 = pPath12_act->ip_dscp;
		}

		// === action: others ===
		hash_action->l3_if_vld1 = TRUE;	// always enabled

		hash_action->cos_update_en = pPath12_act->cos_update_en;
		hash_action->cos1 = pPath12_act->cos;
		// forward by port
		hash_action->dpid_vld = TRUE;
		hash_action->permit = TRUE;
		hash_action->mcgid = pPath12_act->ldpid;
		hash_action->deepq = TRUE; //enable deepq
		hash_action->dpid_pri = 1;
		// mirror
		hash_action->mrr_vld = TRUE;
		hash_action->mrr_en = (l3fe_egress_mirrored_port == pPath12_act->ldpid) ? TRUE : FALSE;
		// stream ID mapping
		hash_action->t2_ctrl_vld1 = ((pPath12_act->gemId_mapping_mode & 0x8) >> 3); 		//FIB.t2_ctrl_vld reused as:			Keep_hdra_ldpid;
		hash_action->pop_l3_en1 = ((pPath12_act->gemId_mapping_mode & 0x4) >> 2);			//FIB.pop_l3_en reused as:			Keep_hdra_cos;
		hash_action->pop_l3_chk_ecn_en1 = ((pPath12_act->gemId_mapping_mode & 0x2) >> 1);	//FIB.pop_l3_chk_ecn_en reused as:	Keep_hdra_pol_id;
		hash_action->pop_l3_vld1 = ((pPath12_act->gemId_mapping_mode & 0x1) >> 0);			//FIB.pop_l3_vld reused as: 		hdra_polid_ldpid_upd_sel(upd_sel);

		if(pPath12_act->gemId_mapping_mode == 0x1)
		{	// CN2 mode[1]
			hash_action->mc = TRUE; // workaround to prevent gem id 0x1f in mcgid to identify as blackhole
			hash_action->dpid_vld = TRUE;
			hash_action->deepq = 1;
			hash_action->t2_ctrl1 = (pPath12_act->tcont_id - ldpid_to_pon_port_map_offset) & 0xF;			// tcont id. hdra.ldpid[5:0] <= CSR_ldpid_base[5:0] + {ldpid_offset_msb, hdri.t2_ctrl[3:0]};
			hash_action->ldpid_offset_msb = (pPath12_act->tcont_id - ldpid_to_pon_port_map_offset) >> 4;	// tcont id. hdra.ldpid[5:0] <= CSR_ldpid_base[5:0] + {ldpid_offset_msb, hdri.t2_ctrl[3:0]};
			hash_action->mcgid = pPath12_act->gem_id; //gem id
		}
		//wifi ssid
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
		if(pPath12_act->is_amsdu_pe_offload)
		{
			hash_action->mdata_byte = pPath12_act->wifi_ssid; //wifi_ssid is wifi priority 0-7
			hash_action->mdata_byte_vld = 1;

			if(pPath12_act->vxlan_sport_update_en)
			{
				DEBUG("To amsdu packet not support vxlan sport update");
				return ASIC_RET_NOT_SUPPORT;

			}
		}
		else
#endif
		{
			if(pPath12_act->wifi_ssid)
			{
				is_toWifi = TRUE;
				hash_action->mdata_byte = pPath12_act->wifi_ssid;
				hash_action->mdata_byte_vld = 1;
			}
			if(pPath12_act->vxlan_sport_update_en)
			{
				hash_action->vxlan_sport_update_en =			pPath12_act->vxlan_sport_update_en;
				hash_action->vxlan_sport_msb15 = 			(pPath12_act->vxlan_sport >> 10) & 0x3F;
				hash_action->six_rd_dip_remap_or_fmr_idx = 	(pPath12_act->vxlan_sport >> 8) & 0x3;
				if(hash_action->mdata_byte_vld == 0)
				{// mdata_byte is not used by wifi SSID
					hash_action->mdata_byte =				pPath12_act->vxlan_sport & 0xFF;
					hash_action->mdata_byte_vld = 1;
				}
			}
		}

		//intf index
		hash_action->igr_l3_if_idx1 = pPath12_act->ingress_intf_idx;
		hash_action->egr_l3_if_idx1 = pPath12_act->egress_intf_idx;
		hash_action->l3_if_counter_en = TRUE;

		//policer
		if(pPath12_act->pol_en || pPath12_act->pol2_en || pPath12_act->pol3_en)
		{
			hash_action->pol_vld1 = TRUE;
			hash_action->pol_en1 = pPath12_act->pol_en;
			hash_action->pol_id1 = pPath12_act->pol_id;
			hash_action->pol2_id_enable1 = pPath12_act->pol2_en;
			hash_action->pol2_id1 = pPath12_act->pol2_id;
			hash_action->pol3_id_enable1 = pPath12_act->pol3_en;
			hash_action->pol3_id1 = pPath12_act->pol3_id;
		}

		if(pPath12_act->sixRd_remap_vld)
		{
			hash_action->six_rd_dip_remap_or_fmr_idx_vld = TRUE;
			hash_action->six_rd_dip_remap_or_fmr_idx = pPath12_act->sixRd_remap_idx;
		}
		if(pPath12_act->mapeMapT_fmr_idx_vld)
		{
			hash_action->six_rd_dip_remap_or_fmr_idx_vld = TRUE;
			hash_action->six_rd_dip_remap_or_fmr_idx = pPath12_act->mapeMapT_fmr_idx;
		}
		if(pPath12_act->l2format_act_vld)
		{
			hash_action->l2_format_vld1 = TRUE;
			hash_action->l2_format1 = pPath12_act->l2format_act;
		}

	}
	else if(in_path == FB_PATH_34)
	{
		rtk_9607f_asic_flow_config_key_path34_t *pPath34_key = (rtk_9607f_asic_flow_config_key_path34_t *)flow_key;
		rtk_9607f_asic_flow_config_action_path34_t *pPath34_act = (rtk_9607f_asic_flow_config_action_path34_t *)flow_act;
		is_from_ponOrWifi = pPath34_key->is_from_ponRx_wifi_rx;
		// === action: L2 ===
		if(pPath34_act->out_smac_trans)
		{
			hash_action->smac_trans = 0; // use l3 interface to do SMAC trans
			hash_action->ip_ttl_dec1 = 1;
			hash_action->ip_ttl_zero_discard_en1 = 1;
			//mtu
			if(asic_intf[hashIntfIdx2HwIntfIdx_mapping[pPath34_act->egress_intf_idx]].valid && asic_intf[hashIntfIdx2HwIntfIdx_mapping[pPath34_act->egress_intf_idx]].mtu_check)
			{
				//follow flow action
				hash_action->ip_mtu_enc_vld1 = TRUE;
				hash_action->ip_mtu_enc1 = asic_intf[hashIntfIdx2HwIntfIdx_mapping[pPath34_act->egress_intf_idx]].mtu_idx;
			}
		}

		if(pPath34_act->out_dmac_trans)
		{
			hash_action->mac_da_idx_vld = TRUE;
			hash_action->mac_da_idx = pPath34_act->mac_da_idx;
		}
		{
			//vlan: set tpid and vid only, no touch vlan dei
			if(pPath34_act->vlan_act)
			{
				//vlan set mode
				hash_action->vlan_vld1 = TRUE;
				hash_action->vlan_cnt1 = pPath34_act->vlan_cnt;
				if(hash_action->vlan_cnt1 == 2)
				{
					if(aal_l3fe_pp_top_tpid_get(pPath34_act->outer_tpid, &tpid_idx) != AAL_E_OK)
					{
						DEBUG("Unknown TPID 0x%x", pPath34_act->outer_tpid);
						return ASIC_RET_NOT_SUPPORT;
					}
					hash_action->top_tpid_enc1 = tpid_idx;
					hash_action->top_vid1 = pPath34_act->outer_vlanid;
					hash_action->top_dei1 = pPath34_act->outer_dei;
					hash_action->top_802_1p_update_en = TRUE;
					hash_action->top_802_1p1 = pPath34_act->outer_vlanpri;

					if(aal_l3fe_pp_top_tpid_get(pPath34_act->inner_tpid, &tpid_idx) != AAL_E_OK)
					{
						DEBUG("Unknown TPID 0x%x", pPath34_act->inner_tpid);
						return ASIC_RET_NOT_SUPPORT;
					}
					hash_action->inner_tpid_enc1 = tpid_idx;
					hash_action->inner_vid1 = pPath34_act->inner_vlanid;
					hash_action->inner_dei1 = pPath34_act->inner_dei;
					hash_action->inner_802_1p_update_en = TRUE;
					hash_action->inner_802_1p1 = pPath34_act->inner_vlanpri;
				}
				else if(hash_action->vlan_cnt1 == 1)
				{
					if(aal_l3fe_pp_top_tpid_get(pPath34_act->outer_tpid, &tpid_idx) != AAL_E_OK)
					{
						DEBUG("Unknown TPID 0x%x", pPath34_act->outer_tpid);
						return ASIC_RET_NOT_SUPPORT;
					}
					hash_action->top_tpid_enc1 = tpid_idx;
					hash_action->top_vid1 = pPath34_act->outer_vlanid;
					hash_action->top_dei1 = pPath34_act->outer_dei;
					hash_action->top_802_1p_update_en = TRUE;
					hash_action->top_802_1p1 = pPath34_act->outer_vlanpri;
				}
			}
		}

		// === action: L3 ===
		if(pPath34_act->ip_dscp_update_en)
		{
			hash_action->ip_dscp_update_en1 = TRUE;
			hash_action->ip_dscp1 = pPath34_act->ip_dscp;
		}

		// === action: others ===
		hash_action->l3_if_vld1 = TRUE;	// always enabled
		hash_action->cos_update_en = pPath34_act->cos_update_en;
		hash_action->cos1 = pPath34_act->cos;
		// forward by port
		hash_action->dpid_vld = TRUE;
		hash_action->permit = TRUE;
		hash_action->mcgid = pPath34_act->ldpid;
		hash_action->deepq = TRUE; //enable deepq
		hash_action->dpid_pri = 1;
		// mirror
		hash_action->mrr_vld = TRUE;
		hash_action->mrr_en = (l3fe_egress_mirrored_port == pPath34_act->ldpid) ? TRUE : FALSE;
		// stream ID mapping
		hash_action->t2_ctrl_vld1 = ((pPath34_act->gemId_mapping_mode & 0x8) >> 3); 		//FIB.t2_ctrl_vld reused as:			Keep_hdra_ldpid;
		hash_action->pop_l3_en1 = ((pPath34_act->gemId_mapping_mode & 0x4) >> 2);			//FIB.pop_l3_en reused as:			Keep_hdra_cos;
		hash_action->pop_l3_chk_ecn_en1 = ((pPath34_act->gemId_mapping_mode & 0x2) >> 1);	//FIB.pop_l3_chk_ecn_en reused as:	Keep_hdra_pol_id;
		hash_action->pop_l3_vld1 = ((pPath34_act->gemId_mapping_mode & 0x1) >> 0);			//FIB.pop_l3_vld reused as: 		hdra_polid_ldpid_upd_sel(upd_sel);

		if(pPath34_act->gemId_mapping_mode == 0x1)
		{	// CN2 mode[1]
			hash_action->mc = TRUE; // workaround to prevent gem id 0x1f in mcgid to identify as blackhole
			hash_action->dpid_vld = TRUE;
			hash_action->deepq = 1;
			hash_action->t2_ctrl1 = (pPath34_act->tcont_id - ldpid_to_pon_port_map_offset) & 0xF;			// tcont id. hdra.ldpid[5:0] <= CSR_ldpid_base[5:0] + {ldpid_offset_msb, hdri.t2_ctrl[3:0]};
			hash_action->ldpid_offset_msb = (pPath34_act->tcont_id - ldpid_to_pon_port_map_offset) >> 4;	// tcont id. hdra.ldpid[5:0] <= CSR_ldpid_base[5:0] + {ldpid_offset_msb, hdri.t2_ctrl[3:0]};
			hash_action->mcgid = pPath34_act->gem_id; //gem id
		}
		//wifi ssid
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
		if(pPath34_act->is_amsdu_pe_offload)
		{
			hash_action->mdata_byte = pPath34_act->wifi_ssid; //wifi_ssid is wifi priority 0-7
			hash_action->mdata_byte_vld = 1;

			if(pPath34_act->vxlan_sport_update_en)
			{
				DEBUG("To amsdu packet not support vxlan sport update");
				return ASIC_RET_NOT_SUPPORT;

			}
		}
		else
#endif
		{
			if(pPath34_act->wifi_ssid)
			{
				is_toWifi = TRUE;
				hash_action->mdata_byte = pPath34_act->wifi_ssid;
				hash_action->mdata_byte_vld = 1;
			}
			if(pPath34_act->vxlan_sport_update_en)
			{
				hash_action->vxlan_sport_update_en = 		pPath34_act->vxlan_sport_update_en;
				hash_action->vxlan_sport_msb15 = 			(pPath34_act->vxlan_sport >> 10) & 0x3F;
				hash_action->six_rd_dip_remap_or_fmr_idx =	(pPath34_act->vxlan_sport >> 8) & 0x3;
				if(hash_action->mdata_byte_vld == 0)
				{	// mdata_byte is not used by wifi SSID
					hash_action->mdata_byte =				pPath34_act->vxlan_sport & 0xFF;
					hash_action->mdata_byte_vld = 1;
				}
			}
		}

		//intf index
		hash_action->igr_l3_if_idx1 = pPath34_act->ingress_intf_idx;
		hash_action->egr_l3_if_idx1 = pPath34_act->egress_intf_idx;
		hash_action->l3_if_counter_en = TRUE;
		//policer
		if(pPath34_act->pol_en || pPath34_act->pol2_en || pPath34_act->pol3_en)
		{
			hash_action->pol_vld1 = TRUE;
			hash_action->pol_en1 = pPath34_act->pol_en;
			hash_action->pol_id1 = pPath34_act->pol_id;
			hash_action->pol2_id_enable1 = pPath34_act->pol2_en;
			hash_action->pol2_id1 = pPath34_act->pol2_id;
			hash_action->pol3_id_enable1 = pPath34_act->pol3_en;
			hash_action->pol3_id1 = pPath34_act->pol3_id;
		}

		if(pPath34_act->sixRd_remap_vld)
		{
			hash_action->six_rd_dip_remap_or_fmr_idx_vld = TRUE;
			hash_action->six_rd_dip_remap_or_fmr_idx = pPath34_act->sixRd_remap_idx;
		}
		if(pPath34_act->mapeMapT_fmr_idx_vld)
		{
			hash_action->six_rd_dip_remap_or_fmr_idx_vld = TRUE;
			hash_action->six_rd_dip_remap_or_fmr_idx = pPath34_act->mapeMapT_fmr_idx;
		}
		if(pPath34_act->l2format_act_vld)
		{
			hash_action->l2_format_vld1 = TRUE;
			hash_action->l2_format1 = pPath34_act->l2format_act;
		}
	}
	else if(in_path == FB_PATH_5)
	{
		rtk_9607f_asic_flow_config_key_path5_t *pPath5_key = (rtk_9607f_asic_flow_config_key_path5_t *)flow_key;
		rtk_9607f_asic_flow_config_action_path5_t *pPath5_act = (rtk_9607f_asic_flow_config_action_path5_t *)flow_act;
		is_from_ponOrWifi = pPath5_key->is_from_ponRx_wifi_rx;
		// === action: L2 ===
		if(pPath5_act->disable_smac_pppoe_trans == FALSE)
			hash_action->smac_trans = 0; // use l3 interface to do SMAC trans
		hash_action->mac_da_idx_vld = TRUE;
		hash_action->mac_da_idx = pPath5_act->mac_da_idx;

		{
			//vlan: set tpid and vid only, no touch vlan dei
			if(pPath5_act->vlan_act)
			{
				//vlan set mode
				hash_action->vlan_vld1 = TRUE;
				hash_action->vlan_cnt1 = pPath5_act->vlan_cnt;
				if(hash_action->vlan_cnt1 == 2)
				{
					if(aal_l3fe_pp_top_tpid_get(pPath5_act->outer_tpid, &tpid_idx) != AAL_E_OK)
					{
						DEBUG("Unknown TPID 0x%x", pPath5_act->outer_tpid);
						return ASIC_RET_NOT_SUPPORT;
					}
					hash_action->top_tpid_enc1 = tpid_idx;
					hash_action->top_vid1 = pPath5_act->outer_vlanid;
					hash_action->top_dei1 = pPath5_act->outer_dei;
					hash_action->top_802_1p_update_en = TRUE;
					hash_action->top_802_1p1 = pPath5_act->outer_vlanpri;

					if(aal_l3fe_pp_top_tpid_get(pPath5_act->inner_tpid, &tpid_idx) != AAL_E_OK)
					{
						DEBUG("Unknown TPID 0x%x", pPath5_act->inner_tpid);
						return ASIC_RET_NOT_SUPPORT;
					}
					hash_action->inner_tpid_enc1 = tpid_idx;
					hash_action->inner_vid1 = pPath5_act->inner_vlanid;
					hash_action->inner_dei1 = pPath5_act->inner_dei;
					hash_action->inner_802_1p_update_en = TRUE;
					hash_action->inner_802_1p1 = pPath5_act->inner_vlanpri;
				}
				else if(hash_action->vlan_cnt1 == 1)
				{
					if(aal_l3fe_pp_top_tpid_get(pPath5_act->outer_tpid, &tpid_idx) != AAL_E_OK)
					{
						DEBUG("Unknown TPID 0x%x", pPath5_act->outer_tpid);
						return ASIC_RET_NOT_SUPPORT;
					}
					hash_action->top_tpid_enc1 = tpid_idx;
					hash_action->top_vid1 = pPath5_act->outer_vlanid;
					hash_action->top_dei1 = pPath5_act->outer_dei;
					hash_action->top_802_1p_update_en = TRUE;
					hash_action->top_802_1p1 = pPath5_act->outer_vlanpri;
				}
			}
		}

		// === action: L3 ===
		hash_action->ip_ttl_dec1 = TRUE;
		hash_action->ip_ttl_zero_discard_en1 = TRUE;

		if(pPath5_act->ip_dscp_update_en)
		{
			hash_action->ip_dscp_update_en1 = TRUE;
			hash_action->ip_dscp1 = pPath5_act->ip_dscp;
		}

		if(pPath5_act->ip_action_mode == RTK_9607F_IP_ACTION_MODE_V4_V6_NAPT)
		{
			hash_action->ip_addr_vld = TRUE;
			hash_action->ip_type = pPath5_act->ip_type;
			if(pPath5_key->ipv4_or_ipv6)
			{
				// IPv6
				hash_type0_flow_action_mode_t flow_act_mode;
				aal_hash_action_mode_get(RTK_ASIC_DEVID, &flow_act_mode);
				if(flow_act_mode == HASH_TYPE0_FLOW_ACTION_MODE_NORMAL)
				{
					ERROR("flow configuration do not support IPv6 NAPT HW acceleration.");
					return ASIC_RET_NOT_SUPPORT;
				}
				hash_action->ip_addr_napt6 = TRUE; // NAPT mode
				hash_action->ip_addr_h_96_vld = TRUE;
				hash_action->ip_addr_h_96_3 = pPath5_act->ip[0];
				hash_action->ip_addr_h_96_2 = pPath5_act->ip[1];
				hash_action->ip_addr_h_96_1 = pPath5_act->ip[2];
				hash_action->ip_addr =  pPath5_act->ip[3];
			}
			else
			{
				//IPv4
				hash_action->ip_addr = pPath5_act->ip[3];
			}
			hash_action->l4_port = pPath5_act->l4_port;
		}
		else if(pPath5_act->ip_action_mode == RTK_9607F_IP_ACTION_MODE_V6_NPT)
		{
			int ret;
			hash_npt6_prefix_addr_entry_t prefix_addr_entry;
			ret = aal_hash_npt6_prefix_addr_entry_get(RTK_ASIC_DEVID, pPath5_act->nptv6_cfg.ipv6_prefix_index_aft, &prefix_addr_entry);
			if(ret)
			{
				ERROR("Get npt6_prefix_adde[%d] failed, ret = %d", pPath5_act->nptv6_cfg.ipv6_prefix_index_aft, ret);
				return ASIC_RET_FAIL;
			}
			hash_action->ip_addr_vld = TRUE;
			hash_action->ip_type = pPath5_act->ip_type;
			hash_action->ip_addr_napt6 = FALSE; // NPT mode
			hash_action->ip_addr = (pPath5_act->nptv6_cfg.ipv6_prefix_index_aft & 0xf) | ((pPath5_act->nptv6_cfg.ipv6_prefix_len_aft & 0x7f) << 4) | ((pPath5_act->nptv6_cfg.ipv6_prefix_len_ori & 0x7f) << 11) | ((pPath5_act->nptv6_cfg.if_ipv6_add_hw_recal & 0x1) << 18);
			hash_action->l4_port = pPath5_act->l4_port;
		}
		else if(pPath5_act->ip_action_mode == RTK_9607F_IP_ACTION_MODE_V4_NAT_LOOPBACK)
		{
			hash_type0_flow_action_mode_t flow_act_mode;
			aal_hash_action_mode_get(RTK_ASIC_DEVID, &flow_act_mode);
			if(flow_act_mode == HASH_TYPE0_FLOW_ACTION_MODE_NORMAL)
			{
				ERROR("flow configuration do not support IPv4 NAT LOOPBACK HW acceleration.");
				return ASIC_RET_NOT_SUPPORT;
			}
			if(pPath5_key->ipv4_or_ipv6)
			{
				ERROR("Not support IPv6 NAT LOOPBACK.");
				return ASIC_RET_NOT_SUPPORT;
			}

			// IPv4 SNAT
			hash_action->ip_addr_vld = TRUE;
			hash_action->ip_type = FALSE;
			hash_action->ip_addr = pPath5_act->nat_loopback_cfg.ipv4_sip;
			hash_action->l4_port = pPath5_act->nat_loopback_cfg.l4_sport;

			// IPv4 DNAT
			hash_action->ip_da_l_vld = TRUE;
			hash_action->ip_da_0 = pPath5_act->nat_loopback_cfg.ipv4_dip;
			hash_action->l4_dp_vld = TRUE;
			hash_action->l4_dp = pPath5_act->nat_loopback_cfg.l4_dport;
		}

		// === action: others ===
		hash_action->l3_if_vld1 = TRUE;	// always enabled
		hash_action->cos_update_en = pPath5_act->cos_update_en;
		hash_action->cos1 = pPath5_act->cos;
		// forward by port
		hash_action->dpid_vld = TRUE;
		hash_action->permit = TRUE;
		hash_action->mcgid = pPath5_act->ldpid;
		hash_action->deepq = TRUE; //enable deepq
		hash_action->dpid_pri = 1;
		// mirror
		hash_action->mrr_vld = TRUE;
		hash_action->mrr_en = (l3fe_egress_mirrored_port == pPath5_act->ldpid) ? TRUE : FALSE;
		// stream ID mapping
		hash_action->t2_ctrl_vld1 = ((pPath5_act->gemId_mapping_mode & 0x8) >> 3);			//FIB.t2_ctrl_vld reused as:			Keep_hdra_ldpid;
		hash_action->pop_l3_en1 = ((pPath5_act->gemId_mapping_mode & 0x4) >> 2);			//FIB.pop_l3_en reused as:			Keep_hdra_cos;
		hash_action->pop_l3_chk_ecn_en1 = ((pPath5_act->gemId_mapping_mode & 0x2) >> 1);	//FIB.pop_l3_chk_ecn_en reused as:	Keep_hdra_pol_id;
		hash_action->pop_l3_vld1 = ((pPath5_act->gemId_mapping_mode & 0x1) >> 0);			//FIB.pop_l3_vld reused as: 		hdra_polid_ldpid_upd_sel(upd_sel);

		if(pPath5_act->gemId_mapping_mode == 0x1)
		{	// CN2 mode[1]
			hash_action->mc = TRUE; // workaround to prevent gem id 0x1f in mcgid to identify as blackhole
			hash_action->dpid_vld = TRUE;
			hash_action->deepq = 1;
			hash_action->t2_ctrl1 = (pPath5_act->tcont_id - ldpid_to_pon_port_map_offset) & 0xF;			// tcont id. hdra.ldpid[5:0] <= CSR_ldpid_base[5:0] + {ldpid_offset_msb, hdri.t2_ctrl[3:0]};
			hash_action->ldpid_offset_msb = (pPath5_act->tcont_id - ldpid_to_pon_port_map_offset) >> 4; // tcont id. hdra.ldpid[5:0] <= CSR_ldpid_base[5:0] + {ldpid_offset_msb, hdri.t2_ctrl[3:0]};
			hash_action->mcgid = pPath5_act->gem_id; //gem id
		}
		//wifi ssid
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
		if(pPath5_act->is_amsdu_pe_offload)
		{
			hash_action->mdata_byte = pPath5_act->wifi_ssid; //wifi_ssid is wifi priority 0-7
			hash_action->mdata_byte_vld = 1;

			if(pPath5_act->vxlan_sport_update_en)
			{
				DEBUG("To amsdu packet not support vxlan sport update");
				return ASIC_RET_NOT_SUPPORT;

			}
		}
		else
#endif
		{
			if(pPath5_act->wifi_ssid)
			{
				is_toWifi = TRUE;
				hash_action->mdata_byte = pPath5_act->wifi_ssid;
				hash_action->mdata_byte_vld = 1;
			}

			if(pPath5_act->vxlan_sport_update_en)
			{
				hash_action->vxlan_sport_update_en = 		pPath5_act->vxlan_sport_update_en;
				hash_action->vxlan_sport_msb15 = 			(pPath5_act->vxlan_sport >> 10) & 0x3F;
				hash_action->six_rd_dip_remap_or_fmr_idx =	(pPath5_act->vxlan_sport >> 8) & 0x3;
				if(hash_action->mdata_byte_vld == 0)
				{	// mdata_byte is not used by wifi SSID
					hash_action->mdata_byte =				pPath5_act->vxlan_sport & 0xFF;
					hash_action->mdata_byte_vld = 1;
				}
			}
		}

		//intf index
		hash_action->igr_l3_if_idx1 = pPath5_act->ingress_intf_idx;
		hash_action->egr_l3_if_idx1 = pPath5_act->egress_intf_idx;
		hash_action->l3_if_counter_en = TRUE;
		//policer
		if(pPath5_act->pol_en || pPath5_act->pol2_en || pPath5_act->pol3_en)
		{
			hash_action->pol_vld1 = TRUE;
			hash_action->pol_en1 = pPath5_act->pol_en;
			hash_action->pol_id1 = pPath5_act->pol_id;
			hash_action->pol2_id_enable1 = pPath5_act->pol2_en;
			hash_action->pol2_id1 = pPath5_act->pol2_id;
			hash_action->pol3_id_enable1 = pPath5_act->pol3_en;
			hash_action->pol3_id1 = pPath5_act->pol3_id;
		}
		//mtu
		if((pPath5_act->disable_mtu_check == FALSE) && asic_intf[hashIntfIdx2HwIntfIdx_mapping[pPath5_act->egress_intf_idx]].valid && asic_intf[hashIntfIdx2HwIntfIdx_mapping[pPath5_act->egress_intf_idx]].mtu_check)
		{
			//follow flow action
			hash_action->ip_mtu_enc_vld1 = TRUE;
			hash_action->ip_mtu_enc1 = asic_intf[hashIntfIdx2HwIntfIdx_mapping[pPath5_act->egress_intf_idx]].mtu_idx;
		}

		if(pPath5_act->sixRd_remap_vld)
		{
			hash_action->six_rd_dip_remap_or_fmr_idx_vld = TRUE;
			hash_action->six_rd_dip_remap_or_fmr_idx = pPath5_act->sixRd_remap_idx;
		}
		if(pPath5_act->mapeMapT_fmr_idx_vld)
		{
			hash_action->six_rd_dip_remap_or_fmr_idx_vld = TRUE;
			hash_action->six_rd_dip_remap_or_fmr_idx = pPath5_act->mapeMapT_fmr_idx;
		}
		if(pPath5_act->l2format_act_vld)
		{
			hash_action->l2_format_vld1 = TRUE;
			hash_action->l2_format1 = pPath5_act->l2format_act;
		}
	}
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	else if(in_path == FB_PATH_MC_WIFI_AMSDU_TX)
	{
		rtk_9607f_asic_flow_config_action_pathMc_wifi_amsdu_tx_t *pPthMc_wifi_amsdu_tx_act = (rtk_9607f_asic_flow_config_action_pathMc_wifi_amsdu_tx_t *)flow_act;
		hash_action->mac_da_idx_vld = pPthMc_wifi_amsdu_tx_act->mac_da_idx_act;
		hash_action->mac_da_idx = pPthMc_wifi_amsdu_tx_act->mac_da_idx;
		hash_action->cos_update_en = pPthMc_wifi_amsdu_tx_act->cos_update_en;
		hash_action->cos1 = pPthMc_wifi_amsdu_tx_act->cos;
		if(pPthMc_wifi_amsdu_tx_act->ldpid_act)
		{
			// forward by port
			hash_action->dpid_vld = TRUE;
			hash_action->permit = TRUE;
			hash_action->mcgid = pPthMc_wifi_amsdu_tx_act->ldpid;
			hash_action->deepq = TRUE; //enable deepq
			hash_action->dpid_pri = 1;
		}
		if(pPthMc_wifi_amsdu_tx_act->wifi_devId_macId_act)
		{
			hash_action->mdata_byte_vld = TRUE;
			hash_action->mdata_byte = pPthMc_wifi_amsdu_tx_act->wifi_devId_macId;
		}
		if(pPthMc_wifi_amsdu_tx_act->l2format_act_vld)
		{
			hash_action->l2_format_vld1 = TRUE;
			hash_action->l2_format1 = pPthMc_wifi_amsdu_tx_act->l2format_act;
		}
	}
#endif

#if 0 //chk_msk_ptr1 and cache_ctrl1 will be setup by aal hash API.
	/* set group[20] chk_msk_ptr to avoid double check fail */
	hash_action->chk_msk_ptr1 = asic_db.flow_profile_info[profileIdx].tuple_info[tupleIdx].hash_mask_idx;
	hash_action->cache_ctrl1 = hashType2CacheCtrl[TUPLE_TYPE_TYPE0]; // always use type 0
#endif

#if 0// 9607F MTU check is done before fib merge, no need this workaround
	if(hash_action->ip_mtu_enc_vld1)
	{
		/*
			from PON to wifi and has no pol1 action: enable MTU check, recover PON RX streamID from pol1_idx if MTU check failed.
			from PON to wifi and has pol1 action:    disable MTU check due to PON RX streamID can not be recovered.
		*/
		if((is_from_ponOrWifi == RTK_9607F_FLOW_FROM_PON) && is_toWifi && hash_action->pol_en1)
			hash_action->ip_mtu_enc_vld1 = FALSE;
	}
#endif
	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_asic_flow_hash_crc_cal(rtk_9607f_asic_flow_hash_cal_info_t *flowHashCalInfo, rtk_9607f_asic_flow_hash_crc_t *flow_hash_crc)
{
	int32_t ret;
	aal_hash_key_t hash_key;
	uint32_t profileIdx = 0;
	uint32_t tupleIdx = 0;
	aal_hash_value_t aal_hash_value;

	if(flowHashCalInfo->in_path == FB_PATH_12)
	{
		profileIdx = RTK_9607F_FLOW_PROFILE_FLOW_2TUPLE;
		/* hash key generation*/
		ret = _rtk_9607f_asic_flow_key_gen(FB_PATH_12, &flowHashCalInfo->path12_key, &hash_key);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash key for path12 failed!");
			return ret;
		}
	}
	else if(flowHashCalInfo->in_path == FB_PATH_34)
	{
		profileIdx = RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE;
		/* hash key generation*/
		ret = _rtk_9607f_asic_flow_key_gen(FB_PATH_34, &flowHashCalInfo->path34_key, &hash_key);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash key for path34 failed!");
			return ret;
		}
	}
	else if(flowHashCalInfo->in_path == FB_PATH_5)
	{
		profileIdx = RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE;
		/* hash key generation*/
		ret = _rtk_9607f_asic_flow_key_gen(FB_PATH_5, &flowHashCalInfo->path5_key, &hash_key);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash key for path5 failed!");
			return ret;
		}
	}
	else if(flowHashCalInfo->in_path == FB_PATH_MC)
	{
		/*** This is for CRC calculation only ***/
		profileIdx = RTK_9607F_FLOW_PROFILE_FLOW_MC;
		tupleIdx = 1;
		/* hash key generation*/
		ret = _rtk_9607f_asic_flow_key_gen(FB_PATH_MC, &flowHashCalInfo->path5_key, &hash_key);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash key for MC failed!");
			return ret;
		}
	}
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	else if(flowHashCalInfo->in_path == FB_PATH_MC_WIFI_AMSDU_TX)
	{
		profileIdx = RTK_9607F_FLOW_PROFILE_MC_WIFI_TX_AMSDU_OFLD;
		/* hash key generation*/
		ret = _rtk_9607f_asic_flow_key_gen(FB_PATH_MC_WIFI_AMSDU_TX, &flowHashCalInfo->pathMc_wifi_amsdu_tx_key, &hash_key);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash key for Mc_wifi_amsdu_tx failed!");
			return ret;
		}
	}
#endif
	else
	{
		DEBUG("Unknown flowHashCalInfo->in_path %d", flowHashCalInfo->in_path);
		return ASIC_RET_FAIL;
	}

	ret = aal_hash_value_cal(RTK_ASIC_DEVID, &hash_key, asic_db.flow_profile_info[profileIdx].tuple_info[tupleIdx].hash_mask_idx,
		profileIdx, &aal_hash_value);
	if(ret != CA_E_OK)
	{
		DEBUG("Calculate hash value failed! ret = %d", ret);
		return ASIC_RET_FAIL;
	}

	flow_hash_crc->crc16 = aal_hash_value.crc16;
	flow_hash_crc->crc32 = aal_hash_value.crc32;

	return ASIC_RET_SUCCESS;
}
#if defined(CONFIG_REALTEK_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA_V8)
asic_ret_t rtk_9607f_asic_flow_add_with_no_crc_calulate(rtk_9607f_asic_flow_config_t *flowConfig, int32_t *mainHashIdx, rtk_9607f_asic_flow_hash_crc_t flow_hash_crc)
{
	int32_t ret;
	aal_hash_key_t hash_key;
	aal_hash_action_t hash_action;
	aal_hash_aging_e hash_age;
	uint32_t rtk8277_hash_age_idle = HASH_AGING_1;
	aal_hash_value_t hash_value = {0};

	uint32_t profileIdx;
	uint32_t tupleIdx = 0;

	if(flowConfig->in_path == FB_PATH_12)
	{
		profileIdx = RTK_9607F_FLOW_PROFILE_FLOW_2TUPLE;
		/* hash key by CRC*/
		ret = _rtk_9607f_asic_flow_key_gen(FB_PATH_12, &(flowConfig->path12.key), &hash_key);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash key for path12 failed!");
			return ret;
		}

		/* hash action generation*/
		ret = _rtk_9607f_asic_flow_action_gen(FB_PATH_12, &(flowConfig->path12.key), &(flowConfig->path12.action), &hash_action, profileIdx, tupleIdx);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash action for path12 failed!");
			return ret;
		}
	}
	else if(flowConfig->in_path == FB_PATH_34)
	{
		profileIdx = RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE;
		/* hash key generation*/
		ret = _rtk_9607f_asic_flow_key_gen(FB_PATH_34, &(flowConfig->path34.key), &hash_key);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash key for path34 failed!");
			return ret;
		}

		/* hash action generation*/
		ret = _rtk_9607f_asic_flow_action_gen(FB_PATH_34, &(flowConfig->path34.key), &(flowConfig->path34.action), &hash_action, profileIdx, tupleIdx);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash action for path34 failed!");
			return ret;
		}
	}
	else if(flowConfig->in_path == FB_PATH_5)
	{
		profileIdx = RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE;
		/* hash key generation*/
		ret = _rtk_9607f_asic_flow_key_gen(FB_PATH_5, &(flowConfig->path5.key), &hash_key);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash key for path5 failed!");
			return ret;
		}

		/* hash action generation*/
		ret = _rtk_9607f_asic_flow_action_gen(FB_PATH_5, &(flowConfig->path5.key), &(flowConfig->path5.action), &hash_action, profileIdx, tupleIdx);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash action for path5 failed!");
			return ret;
		}
	}
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	else if(flowConfig->in_path == FB_PATH_MC_WIFI_AMSDU_TX)
	{
		profileIdx = RTK_9607F_FLOW_PROFILE_MC_WIFI_TX_AMSDU_OFLD;
		/* hash key generation*/
		ret = _rtk_9607f_asic_flow_key_gen(FB_PATH_MC_WIFI_AMSDU_TX, &(flowConfig->pathMc_wifi_amsdu_tx.key), &hash_key);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash key for pathMc_wifi_amsdu_tx failed!");
			return ret;
		}

		/* hash action generation*/
		ret = _rtk_9607f_asic_flow_action_gen(FB_PATH_MC_WIFI_AMSDU_TX, &(flowConfig->pathMc_wifi_amsdu_tx.key), &(flowConfig->pathMc_wifi_amsdu_tx.action), &hash_action, profileIdx, tupleIdx);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash action for pathMc_wifi_amsdu_tx failed!");
			return ret;
		}
	}
#endif

	hash_age = rtk8277_hash_age_idle;

	hash_value.crc16 = flow_hash_crc.crc16;
	hash_value.crc32 = flow_hash_crc.crc32;

	/*Note. If hash exists in main hash/overflow hash table, the hash_age will be updated*/
	ret = aal_hash_add_with_no_crc_calulate(RTK_ASIC_DEVID, &hash_key, hash_value, FALSE, &hash_action, TUPLE_TYPE_TYPE0,
			hash_age, asic_db.flow_profile_info[profileIdx].tuple_info[tupleIdx].hash_mask_idx, profileIdx, mainHashIdx);

	if(ret == CA_E_FULL)
	{
		*mainHashIdx = INVALID_IDX;
		return ASIC_RET_FULL;
	}
	else if(ret == CA_E_EXISTS)
	{
		return ASIC_RET_EXIST;
	}
	else if (ret != CA_E_OK) {
		*mainHashIdx = INVALID_IDX;
		ERROR("fail to add main hash entry. (ret=%d)", ret);
		return ASIC_RET_FAIL;
	}

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_asic_flow_add(rtk_9607f_asic_flow_config_t *flowConfig, int32_t *mainHashIdx)
{
	int32_t ret;
	aal_hash_key_t hash_key;
	aal_hash_action_t hash_action;
	aal_hash_aging_e hash_age;
	uint32_t rtk8277_hash_age_idle = HASH_AGING_1;

	uint32_t profileIdx;
	uint32_t tupleIdx = 0;

	if(flowConfig->in_path == FB_PATH_12)
	{
		//compatible to CONFIG_REALTEK_BOARD_FPGA : set TPID key here
		{
			if(flowConfig->path12.key.stag_if)
				flowConfig->path12.key.svlan_tpid = 0x88a8;
			if(flowConfig->path12.key.ctag_if)
				flowConfig->path12.key.cvlan_tpid = 0x8100;
		}

		profileIdx = RTK_9607F_FLOW_PROFILE_FLOW_2TUPLE;
		/* hash key generation*/
		ret = _rtk_9607f_asic_flow_key_gen(FB_PATH_12, &(flowConfig->path12.key), &hash_key);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash key for path12 failed!");
			return ret;
		}

		/* hash action generation*/
		ret = _rtk_9607f_asic_flow_action_gen(FB_PATH_12, &(flowConfig->path12.key), &(flowConfig->path12.action), &hash_action, profileIdx, tupleIdx);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash action for path12 failed!");
			return ret;
		}
	}
	else if(flowConfig->in_path == FB_PATH_34)
	{
		//compatible to CONFIG_REALTEK_BOARD_FPGA : set TPID key here
		{
			if(flowConfig->path34.key.stag_if)
				flowConfig->path34.key.svlan_tpid = 0x88a8;
			if(flowConfig->path34.key.ctag_if)
				flowConfig->path34.key.cvlan_tpid = 0x8100;
		}

		profileIdx = RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE;
		/* hash key generation*/
		ret = _rtk_9607f_asic_flow_key_gen(FB_PATH_34, &(flowConfig->path34.key), &hash_key);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash key for path34 failed!");
			return ret;
		}

		/* hash action generation*/
		ret = _rtk_9607f_asic_flow_action_gen(FB_PATH_34, &(flowConfig->path34.key), &(flowConfig->path34.action), &hash_action, profileIdx, tupleIdx);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash action for path34 failed!");
			return ret;
		}
	}
	else if(flowConfig->in_path == FB_PATH_5)
	{
		//compatible to CONFIG_REALTEK_BOARD_FPGA : set TPID key here
		{
			if(flowConfig->path5.key.stag_if)
				flowConfig->path5.key.svlan_tpid = 0x88a8;
			if(flowConfig->path5.key.ctag_if)
				flowConfig->path5.key.cvlan_tpid = 0x8100;
		}

		profileIdx = RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE;
		/* hash key generation*/
		ret = _rtk_9607f_asic_flow_key_gen(FB_PATH_5, &(flowConfig->path5.key), &hash_key);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash key for path5 failed!");
			return ret;
		}

		/* hash action generation*/
		ret = _rtk_9607f_asic_flow_action_gen(FB_PATH_5, &(flowConfig->path5.key), &(flowConfig->path5.action), &hash_action, profileIdx, tupleIdx);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash action for path5 failed!");
			return ret;
		}
	}
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	else if(flowConfig->in_path == FB_PATH_MC_WIFI_AMSDU_TX)
	{
		profileIdx = RTK_9607F_FLOW_PROFILE_MC_WIFI_TX_AMSDU_OFLD;
		/* hash key generation*/
		ret = _rtk_9607f_asic_flow_key_gen(FB_PATH_MC_WIFI_AMSDU_TX, &(flowConfig->pathMc_wifi_amsdu_tx.key), &hash_key);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash key for pathMc_wifi_amsdu_tx failed!");
			return ret;
		}

		/* hash action generation*/
		ret = _rtk_9607f_asic_flow_action_gen(FB_PATH_MC_WIFI_AMSDU_TX, &(flowConfig->pathMc_wifi_amsdu_tx.key), &(flowConfig->pathMc_wifi_amsdu_tx.action), &hash_action, profileIdx, tupleIdx);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash action for pathMc_wifi_amsdu_tx failed!");
			return ret;
		}
	}
#endif

	hash_age = rtk8277_hash_age_idle;

	/*Note. If hash exists in main hash/overflow hash table, the hash_age will be updated*/
	ret = aal_hash_add(RTK_ASIC_DEVID, &hash_key, &hash_action, TUPLE_TYPE_TYPE0,
			hash_age, asic_db.flow_profile_info[profileIdx].tuple_info[tupleIdx].hash_mask_idx, profileIdx, mainHashIdx);

	if(ret == CA_E_FULL)
	{
		*mainHashIdx = INVALID_IDX;
		return ASIC_RET_FULL;
	}
	else if(ret == CA_E_EXISTS)
	{
		return ASIC_RET_EXIST;
	}
	else if (ret != CA_E_OK) {
		*mainHashIdx = INVALID_IDX;
		ERROR("fail to add main hash entry. (ret=%d)", ret);
		return ASIC_RET_FAIL;
	}

	return ASIC_RET_SUCCESS;
}
#endif
asic_ret_t rtk_9607f_asic_flow_add_with_no_crc_calulate_by_idx(rtk_9607f_asic_flow_config_t *flowConfig, int32_t mainHashIdx, rtk_9607f_asic_flow_hash_crc_t flow_hash_crc)
{
	int32_t ret;
	aal_hash_key_t hash_key;
	aal_hash_action_t hash_action;
	aal_hash_aging_e hash_age;
	uint32_t rtk8277_hash_age_idle = HASH_AGING_1;
	aal_hash_value_t hash_value = {0};

	uint32_t profileIdx = 0;
	uint32_t tupleIdx = 0;

	if(flowConfig->in_path == FB_PATH_12)
	{
		profileIdx = RTK_9607F_FLOW_PROFILE_FLOW_2TUPLE;
		/* hash key by CRC*/
		ret = _rtk_9607f_asic_flow_key_gen(FB_PATH_12, &(flowConfig->path12.key), &hash_key);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash key for path12 failed!");
			return ret;
		}

		/* hash action generation*/
		ret = _rtk_9607f_asic_flow_action_gen(FB_PATH_12, &(flowConfig->path12.key), &(flowConfig->path12.action), &hash_action, profileIdx, tupleIdx);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash action for path12 failed!");
			return ret;
		}
	}
	else if(flowConfig->in_path == FB_PATH_34)
	{
		profileIdx = RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE;
		/* hash key generation*/
		ret = _rtk_9607f_asic_flow_key_gen(FB_PATH_34, &(flowConfig->path34.key), &hash_key);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash key for path34 failed!");
			return ret;
		}

		/* hash action generation*/
		ret = _rtk_9607f_asic_flow_action_gen(FB_PATH_34, &(flowConfig->path34.key), &(flowConfig->path34.action), &hash_action, profileIdx, tupleIdx);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash action for path34 failed!");
			return ret;
		}
	}
	else if(flowConfig->in_path == FB_PATH_5)
	{
		profileIdx = RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE;
		/* hash key generation*/
		ret = _rtk_9607f_asic_flow_key_gen(FB_PATH_5, &(flowConfig->path5.key), &hash_key);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash key for path5 failed!");
			return ret;
		}

		/* hash action generation*/
		ret = _rtk_9607f_asic_flow_action_gen(FB_PATH_5, &(flowConfig->path5.key), &(flowConfig->path5.action), &hash_action, profileIdx, tupleIdx);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash action for path5 failed!");
			return ret;
		}
	}
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	else if(flowConfig->in_path == FB_PATH_MC_WIFI_AMSDU_TX)
	{
		profileIdx = RTK_9607F_FLOW_PROFILE_MC_WIFI_TX_AMSDU_OFLD;
		/* hash key generation*/
		ret = _rtk_9607f_asic_flow_key_gen(FB_PATH_MC_WIFI_AMSDU_TX, &(flowConfig->pathMc_wifi_amsdu_tx.key), &hash_key);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash key for pathMc_wifi_amsdu_tx failed!");
			return ret;
		}

		/* hash action generation*/
		ret = _rtk_9607f_asic_flow_action_gen(FB_PATH_MC_WIFI_AMSDU_TX, &(flowConfig->pathMc_wifi_amsdu_tx.key), &(flowConfig->pathMc_wifi_amsdu_tx.action), &hash_action, profileIdx, tupleIdx);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash action for pathMc_wifi_amsdu_tx failed!");
			return ret;
		}
	}
#endif
	else
	{
		DEBUG("Unknown flowConfig->in_path %d", flowConfig->in_path);
		return ASIC_RET_FAIL;
	}

	hash_age = rtk8277_hash_age_idle;

	hash_value.crc16 = flow_hash_crc.crc16;
	hash_value.crc32 = flow_hash_crc.crc32;

	/*Note. If hash exists in main hash/overflow hash table, the hash_age will be updated*/
	ret = aal_hash_add_with_no_crc_calulate_by_idx(RTK_ASIC_DEVID, &hash_key, hash_value, FALSE, &hash_action, TUPLE_TYPE_TYPE0,
			hash_age, asic_db.flow_profile_info[profileIdx].tuple_info[tupleIdx].hash_mask_idx, profileIdx, mainHashIdx);

	if(ret == CA_E_EXISTS)
	{
		return ASIC_RET_EXIST;
	}
	else if (ret != CA_E_OK) {
		ERROR("fail to add main hash entry. (ret=%d)", ret);
		return ASIC_RET_FAIL;
	}

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_asic_flow_add_by_idx(rtk_9607f_asic_flow_config_t *flowConfig, int32_t mainHashIdx, rtk_9607f_asic_flow_hash_crc_t *flow_hash_crc)
{
	int32_t ret;
	aal_hash_key_t hash_key;
	aal_hash_action_t hash_action;
	aal_hash_aging_e hash_age;
	aal_hash_value_t hash_value;
	uint32_t rtk8277_hash_age_idle = HASH_AGING_1;

	uint32_t profileIdx = 0;
	uint32_t tupleIdx = 0;

	if(flowConfig->in_path == FB_PATH_12)
	{
		profileIdx = RTK_9607F_FLOW_PROFILE_FLOW_2TUPLE;
		/* hash key generation*/
		ret = _rtk_9607f_asic_flow_key_gen(FB_PATH_12, &(flowConfig->path12.key), &hash_key);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash key for path12 failed!");
			return ret;
		}

		/* hash action generation*/
		ret = _rtk_9607f_asic_flow_action_gen(FB_PATH_12, &(flowConfig->path12.key), &(flowConfig->path12.action), &hash_action, profileIdx, tupleIdx);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash action for path12 failed!");
			return ret;
		}
	}
	else if(flowConfig->in_path == FB_PATH_34)
	{
		profileIdx = RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE;
		/* hash key generation*/
		ret = _rtk_9607f_asic_flow_key_gen(FB_PATH_34, &(flowConfig->path34.key), &hash_key);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash key for path34 failed!");
			return ret;
		}

		/* hash action generation*/
		ret = _rtk_9607f_asic_flow_action_gen(FB_PATH_34, &(flowConfig->path34.key), &(flowConfig->path34.action), &hash_action, profileIdx, tupleIdx);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash action for path34 failed!");
			return ret;
		}
	}
	else if(flowConfig->in_path == FB_PATH_5)
	{
		profileIdx = RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE;
		/* hash key generation*/
		ret = _rtk_9607f_asic_flow_key_gen(FB_PATH_5, &(flowConfig->path5.key), &hash_key);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash key for path5 failed!");
			return ret;
		}

		/* hash action generation*/
		ret = _rtk_9607f_asic_flow_action_gen(FB_PATH_5, &(flowConfig->path5.key), &(flowConfig->path5.action), &hash_action, profileIdx, tupleIdx);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash action for path5 failed!");
			return ret;
		}
	}
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	else if(flowConfig->in_path == FB_PATH_MC_WIFI_AMSDU_TX)
	{
		profileIdx = RTK_9607F_FLOW_PROFILE_MC_WIFI_TX_AMSDU_OFLD;
		/* hash key generation*/
		ret = _rtk_9607f_asic_flow_key_gen(FB_PATH_MC_WIFI_AMSDU_TX, &(flowConfig->pathMc_wifi_amsdu_tx.key), &hash_key);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash key for pathMc_wifi_amsdu_tx failed!");
			return ret;
		}

		/* hash action generation*/
		ret = _rtk_9607f_asic_flow_action_gen(FB_PATH_MC_WIFI_AMSDU_TX, &(flowConfig->pathMc_wifi_amsdu_tx.key), &(flowConfig->pathMc_wifi_amsdu_tx.action), &hash_action, profileIdx, tupleIdx);
		if(ret != ASIC_RET_SUCCESS)
		{
			DEBUG("generate hash action for pathMc_wifi_amsdu_tx failed!");
			return ret;
		}
	}
#endif
	else
	{
		DEBUG("Unknown flowConfig->in_path %d", flowConfig->in_path);
		return ASIC_RET_FAIL;
	}

	hash_age = rtk8277_hash_age_idle;

	/*Note. If hash exists in main hash/overflow hash table, the hash_age will be updated*/
	ret = aal_hash_add_by_idx(RTK_ASIC_DEVID, &hash_key, &hash_action, TUPLE_TYPE_TYPE0,
			hash_age, asic_db.flow_profile_info[profileIdx].tuple_info[tupleIdx].hash_mask_idx, profileIdx, mainHashIdx, &hash_value);

	flow_hash_crc->crc16 = hash_value.crc16;
	flow_hash_crc->crc32 = hash_value.crc32;

	if(ret == CA_E_EXISTS)
	{
		return ASIC_RET_EXIST;
	}
	else if (ret != CA_E_OK) {
		ERROR("fail to add main hash entry. (ret=%d)", ret);
		return ASIC_RET_FAIL;
	}

	return ASIC_RET_SUCCESS;
}


asic_ret_t rtk_9607f_asic_flow_del(int32_t mainHashIdx)
{
	int ret;

#if defined(CONFIG_REALTEK_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA_V8)
	PARAM_CHK(mainHashIdx >= HASH_HASH_TBL_ENTRY_MAX, ASIC_RET_OUT_OF_RANGE);
#else
	PARAM_CHK(mainHashIdx >= fc_db.flowHwTableSize, ASIC_RET_OUT_OF_RANGE);
#endif

	DEBUG("Del flow ((mainHashIdx=%d)", mainHashIdx);
	ret = aal_hash_delete(RTK_ASIC_DEVID, mainHashIdx);
	if (ret == AAL_E_ENTRYNOTFOUND)
	{
		return ASIC_RET_SUCCESS;
	}
	else if (ret != AAL_E_OK)
	{
		ERROR("fail to delete main hash entry. (mainHashIdx=%d, ret=%d)", mainHashIdx, ret);
		return ASIC_RET_FAIL;
	}
	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_asic_flow_flush(void)
{
	int ret;
	ret = aal_hash_flush(RTK_ASIC_DEVID);
	if(ret)
		ERROR("%s: aal_hash_flush failed, ret = %d", __func__, ret);
	aal_table_flush(AAL_TABLE_HASH_NPT6_PREFIX_ADDR);

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_asic_flow_traffic_get(ca_uint32_t mainHashIdx, uint32_t *trfStatus)
{
	int ret;

	// get traffic bit of 32 flow entries at a time
	ret = aal_hash_traffic_status_get(AAL_TABLE_HASH_HASH_TBL, mainHashIdx, trfStatus);

	if(ret != AAL_E_OK)
	{
		ERROR("fail to set flow traffic. (mainHashIdx=%d, ret=%d)\n", mainHashIdx, ret);
		return ASIC_RET_SUCCESS;
	}
	return ASIC_RET_SUCCESS;

}

asic_ret_t rtk_9607f_asic_flow_age_set(ca_uint32_t mainHashIdx, uint32_t age)
{
	int ret;

	ret = aal_hash_age_set(AAL_TABLE_HASH_HASH_TBL, mainHashIdx, age);

	if(ret != AAL_E_OK)
	{
		ERROR("fail to set main hash age. (mainHashIdx=%d, ret=%d)\n", mainHashIdx, ret);
		return ASIC_RET_SUCCESS;
	}
	return ASIC_RET_SUCCESS;

}

asic_ret_t rtk_9607f_asic_flow_age_get(ca_uint32_t mainHashIdx, uint32_t *age)
{
	int ret = 0, time = 0;

	ret = aal_hash_age_get(AAL_TABLE_HASH_HASH_TBL, mainHashIdx, age, &time);

	if(ret != AAL_E_OK)
	{
		ERROR("fail to set main hash age. (mainHashIdx=%d, ret=%d)", mainHashIdx, ret);
		return ASIC_RET_SUCCESS;
	}

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_asic_dmalso_backpressure_rule_add(rtk_asic_dmalso_bp_conf_t *pVP_bp_conf)
{
	uint8 i = 0;
	uint16 bp_vp_mask = 0;
	uint32 regAddr = 0, regValue = 0;
	TE_CB_WIFI_CTRL_t wifi_bp_ctrl;
	TE_CB_WIFI_BUF_THRSH_G0_t vpThreshold;
	TE_CB_VP_MAP0_t vpMask;
	DMA_SEC_DMA_LSO_VP_CONTROL_t vp_ctrl;

	PARAM_CHK(pVP_bp_conf->rule_idx >= ASIC_DMALSO_BP_RULE_END, ASIC_RET_OUT_OF_RANGE);

	if(pVP_bp_conf->rule_idx == ASIC_DMALSO_BP_RULE_NONE) {
		for(i = ASIC_DMALSO_BP_RULE_1; i < ASIC_DMALSO_BP_RULE_END; i++) {
			if(asic_db.dmalso_bp_rule_vld[i] == FALSE) {
				pVP_bp_conf->rule_idx = i;
				break;
			}
		}
		if(i>=ASIC_DMALSO_BP_RULE_END)
			return ASIC_RET_FULL;
	}
	
	asic_db.dmalso_bp_rule_vld[pVP_bp_conf->rule_idx] = TRUE;

	// enable
	wifi_bp_ctrl.wrd = rtk_9607f_asic_ne_reg_read(TE_CB_WIFI_CTRL);
	if(wifi_bp_ctrl.bf.backpressure_en==0) {
		wifi_bp_ctrl.bf.backpressure_en = 1;
		rtk_9607f_asic_ne_reg_write(wifi_bp_ctrl.wrd, TE_CB_WIFI_CTRL);
	}

	// config TE threshold
	vpThreshold.bf.hth = pVP_bp_conf->threshold_hi;
	vpThreshold.bf.lth = pVP_bp_conf->threshold_lo;
	rtk_9607f_asic_ne_reg_write(vpThreshold.wrd, TE_CB_WIFI_BUF_THRSH_G0 + (pVP_bp_conf->rule_idx * 4));

	// sel VP rule group per lspid
	for(i = 0; i < ASIC_DMALSO_BP_SPID_MAX; i++) {
		if(pVP_bp_conf->lspid_vld[i] == TRUE) {
			regAddr = QM_QM_VP_BACK_PRES_CTRL0 + ((pVP_bp_conf->lspid[i] / 8) * 4);
			regValue = rtk_9607f_asic_ne_reg_read(regAddr);
			regValue &= ~(0xF << ((pVP_bp_conf->lspid[i] & 0x7) * 4));
			regValue |= (pVP_bp_conf->rule_idx << ((pVP_bp_conf->lspid[i] & 0x7) * 4));
			rtk_9607f_asic_ne_reg_write(regValue, regAddr);
		}
	}

	// config backpressure target VP
	for(i = 0; i < ASIC_DMALSO_BP_SPID_MAX; i++) {
		if((pVP_bp_conf->vpId_vld[i] == TRUE) && (pVP_bp_conf->vpId[i] < DMA_SEC_DMA_LSO_VP_COUNT)) {	// CA_NI_DMA_LSO_VP_NUM
			bp_vp_mask |= (1<<pVP_bp_conf->vpId[i]);
			// enable VP 
			regAddr = DMA_SEC_DMA_LSO_VP_CONTROL + (pVP_bp_conf->vpId[i] * DMA_SEC_DMA_LSO_VP_STRIDE);
			vp_ctrl.wrd = rtk_9607f_asic_dma_lso_reg_read(regAddr);
			vp_ctrl.bf.te_dma_bp_en = 1;
			rtk_9607f_asic_dma_lso_reg_write(vp_ctrl.wrd, regAddr);
		}
	}
	regAddr = TE_CB_VP_MAP0 + (pVP_bp_conf->rule_idx / 2) * 4;
	vpMask.wrd = rtk_9607f_asic_ne_reg_read(regAddr);
	if(pVP_bp_conf->rule_idx%2 == 0)
		vpMask.bf.g0 = bp_vp_mask;
	else
		vpMask.bf.g1 = bp_vp_mask;
	rtk_9607f_asic_ne_reg_write(vpMask.wrd, regAddr);

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_asic_dmalso_backpressure_rule_del(rtk_asic_dmalso_bp_rule_t rule_idx)
{
	uint8 i = 0, j = 0, mapRule = 0;
	asic_ret_t ret = ASIC_RET_SUCCESS;
	uint16 bp_vp_mask = 0;
	uint32 regAddr = 0, regValue = 0;
	TE_CB_VP_MAP0_t vpMask;
	rtk_asic_dmalso_bp_conf_t vp_bp_conf = {0};
	DMA_SEC_DMA_LSO_VP_CONTROL_t vp_ctrl;

	PARAM_CHK(rule_idx >= ASIC_DMALSO_BP_RULE_END, ASIC_RET_OUT_OF_RANGE);

	// delete backpressure threshold and  vp mask
	vp_bp_conf.rule_idx = rule_idx;
	vp_bp_conf.threshold_hi = 0xFFFF;
	vp_bp_conf.threshold_lo = 0xFFFF;

	if((ret = rtk_asic_dmalso_backpressure_rule_add(&vp_bp_conf)) != ASIC_RET_SUCCESS)
		return ret;

	asic_db.dmalso_bp_rule_vld[rule_idx] = FALSE;

	// find all TE en setting
	for(i = 0; i < ASIC_DMALSO_BP_RULE_END/2; i++) {
		
		vpMask.wrd = rtk_9607f_asic_ne_reg_read(TE_CB_VP_MAP0 + (i * 4));
		bp_vp_mask |= vpMask.bf.g0;
		bp_vp_mask |= vpMask.bf.g1;
	}
	// adjust VP backpressure en bit
	for(i = 0; i < DMA_SEC_DMA_LSO_VP_COUNT; i++) {
		regAddr = DMA_SEC_DMA_LSO_VP_CONTROL + (i * DMA_SEC_DMA_LSO_VP_STRIDE);
		vp_ctrl.wrd = rtk_9607f_asic_dma_lso_reg_read(regAddr);
		vp_ctrl.bf.te_dma_bp_en = ((1<<i) & bp_vp_mask) ? 1 : 0;
		rtk_9607f_asic_dma_lso_reg_write(vp_ctrl.wrd, regAddr);
	}

	// clear all lspid mapping rule which is matched with rule_idx
	for(i = 0; i < ASIC_LPORT_MAX/8; i++) {
		uint8 hit = FALSE;
		regAddr = QM_QM_VP_BACK_PRES_CTRL0 + (i * 4);
		regValue = rtk_9607f_asic_ne_reg_read(regAddr);
		for(j = 0; j < 8; j++) {
			mapRule = (regValue>>(j*4)) & 0x7;
			if(mapRule == rule_idx) {
				hit = TRUE;
				regValue &= ~(0x7 << (j*4));
			}
		}
		if(hit)
			rtk_9607f_asic_ne_reg_write(regValue, regAddr);
	}
	
	return ret;
}

asic_ret_t rtk_9607f_asic_dmaLso_lspid_from_hdra(bool enable)
{	//this function for 9607F should useless

	DMA_SEC_DMA_GLB_DMA_LSO_CTRL_t    dma_lso_ctrl;

	dma_lso_ctrl.wrd = rtk_9607f_asic_dma_lso_reg_read(DMA_SEC_DMA_GLB_DMA_LSO_CTRL);
	if(enable){
		DEBUG("Enable force direct tx by default for pon mirror\n");
		dma_lso_ctrl.bf.lspid_keep = 1;			/* keep lspid by tx desc or hdrA */
		rtk_9607f_asic_dma_lso_reg_write(dma_lso_ctrl.wrd, DMA_SEC_DMA_GLB_DMA_LSO_CTRL);
		fc_db.controlFuc.tx_with_hdr_debug = TRUE;
	}else{
		DEBUG("Disable force direct tx by default\n");
		dma_lso_ctrl.bf.lspid_keep = 0;			/* keep lspid by VP */
		rtk_9607f_asic_dma_lso_reg_write(dma_lso_ctrl.wrd, DMA_SEC_DMA_GLB_DMA_LSO_CTRL);
		fc_db.controlFuc.tx_with_hdr_debug = FALSE;
	}

	return ASIC_RET_SUCCESS;
	
}

asic_ret_t rtk_9607f_asic_dmaLso_reg_get(rtk_9607f_asic_dmaLso_reg_t *dmaLso_reg)
{
	DMA_SEC_DMA_GLB_DMA_LSO_CTRL_t	  dma_lso_ctrl;

	dma_lso_ctrl.wrd = rtk_9607f_asic_dma_lso_reg_read(DMA_SEC_DMA_GLB_DMA_LSO_CTRL);

	if(dmaLso_reg->lspid_keep_vld){
		dmaLso_reg->lspid_keep = dma_lso_ctrl.bf.lspid_keep;
	}

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_asic_dmaAftFib_set(uint32_t idx, rtk_9607f_asic_dmaAftFib_t dmaAftFib)
{
	DMA_SEC_DMA_AFT_L2FIB_DATA0_t fib0;
	DMA_SEC_DMA_AFT_L2FIB_DATA1_t fib1;
	DMA_SEC_DMA_AFT_L2FIB_DATA2_t fib2;
	asic_ret_t ret = ASIC_RET_SUCCESS;

	PARAM_CHK(idx >= DMA_AFT_FIB_SIZE, ASIC_RET_OUT_OF_RANGE);

	fib0.bf.session_id = dmaAftFib.session_id;
	fib0.bf.pppoe_cmd = dmaAftFib.pppoe_cmd;
	fib0.bf.inner_1p_sel = dmaAftFib.inner_1p_sel;
	fib0.bf.inner_802_1p = dmaAftFib.inner_802_1p;
	fib0.bf.inner_dei_sel = dmaAftFib.inner_dei_sel;
	fib0.bf.inner_dei = dmaAftFib.inner_dei;
	fib0.bf.inner_vid = dmaAftFib.inner_vid & ((1<<6)-1);

	fib1.bf.inner_vid = dmaAftFib.inner_vid >> 6;
	fib1.bf.inner_tpid_sel = dmaAftFib.inner_tpid_sel;
	fib1.bf.inner_tpid_enc = dmaAftFib.inner_tpid_enc;
	fib1.bf.top_1p_sel = dmaAftFib.top_1p_sel;
	fib1.bf.top_802_1p = dmaAftFib.top_802_1p;
	fib1.bf.top_dei_sel = dmaAftFib.top_dei_sel;
	fib1.bf.top_dei = dmaAftFib.top_dei;
	fib1.bf.top_vid = dmaAftFib.top_vid;
	fib1.bf.top_tpid_sel = dmaAftFib.top_tpid_sel & 1;

	fib2.bf.top_tpid_sel = dmaAftFib.top_tpid_sel >> 1;
	fib2.bf.top_tpid_enc = dmaAftFib.top_tpid_enc;
	fib2.bf.inner_vlan_cmd = dmaAftFib.inner_vlan_cmd;
	fib2.bf.vlan_cnt = dmaAftFib.vlan_cnt;
	fib2.bf.vlan_vld = dmaAftFib.vlan_vld;

	ret = aal_ni_dma_lso_set_aft_l2fib(idx, fib0, fib1, fib2);

	if (ret!= AAL_E_OK)
	{
		ERROR("set dmaAftFib[%d], ret = %d", idx, ret);
		return ASIC_RET_FAIL;
	}

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_asic_dmaAftFib_get(uint32_t idx, rtk_9607f_asic_dmaAftFib_t *pDmaAftFib)
{
	DMA_SEC_DMA_AFT_L2FIB_DATA0_t fib0;
	DMA_SEC_DMA_AFT_L2FIB_DATA1_t fib1;
	DMA_SEC_DMA_AFT_L2FIB_DATA2_t fib2;
	asic_ret_t ret = ASIC_RET_SUCCESS;

	PARAM_CHK(idx >= DMA_AFT_FIB_SIZE, ASIC_RET_OUT_OF_RANGE);

	memset(pDmaAftFib, 0, sizeof(rtk_9607f_asic_dmaAftFib_t));

	ret = aal_ni_dma_lso_get_aft_l2fib(idx, &fib0, &fib1, &fib2);

	if (ret!= AAL_E_OK)
	{
		ERROR("get dmaAftFib[%d], ret = %d", idx, ret);
		return ASIC_RET_FAIL;
	}

	pDmaAftFib->session_id = fib0.bf.session_id;
	pDmaAftFib->pppoe_cmd = fib0.bf.pppoe_cmd;
	pDmaAftFib->inner_1p_sel = fib0.bf.inner_1p_sel;
	pDmaAftFib->inner_802_1p = fib0.bf.inner_802_1p;
	pDmaAftFib->inner_dei_sel = fib0.bf.inner_dei_sel;
	pDmaAftFib->inner_dei = fib0.bf.inner_dei;
	pDmaAftFib->inner_vid = fib0.bf.inner_vid | (fib1.bf.inner_vid << 6);
	pDmaAftFib->inner_tpid_sel = fib1.bf.inner_tpid_sel;
	pDmaAftFib->inner_tpid_enc = fib1.bf.inner_tpid_enc;
	pDmaAftFib->top_1p_sel = fib1.bf.top_1p_sel;
	pDmaAftFib->top_802_1p = fib1.bf.top_802_1p;
	pDmaAftFib->top_dei_sel = fib1.bf.top_dei_sel;
	pDmaAftFib->top_dei = fib1.bf.top_dei;
	pDmaAftFib->top_vid = fib1.bf.top_vid;
	pDmaAftFib->top_tpid_sel = fib1.bf.top_tpid_sel | fib2.bf.top_tpid_sel;
	pDmaAftFib->top_tpid_enc = fib2.bf.top_tpid_enc;
	pDmaAftFib->inner_vlan_cmd = fib2.bf.inner_vlan_cmd;
	pDmaAftFib->vlan_cnt = fib2.bf.vlan_cnt;
	pDmaAftFib->vlan_vld = fib2.bf.vlan_vld;

	return ASIC_RET_SUCCESS;
}
asic_ret_t rtk_9607f_asic_dmaAftTpid_set(char tpid_set_msk, rtk_9607f_asic_dmaAftTpid_t aftTpid)
{
	DMA_SEC_DMA_AFT_TPID_0_t	tpid01;
	DMA_SEC_DMA_AFT_TPID_1_t	tpid23;

	tpid01.wrd = rtk_9607f_asic_dma_lso_reg_read(DMA_SEC_DMA_AFT_TPID_0);
	tpid23.wrd = rtk_9607f_asic_dma_lso_reg_read(DMA_SEC_DMA_AFT_TPID_1);

	if(tpid_set_msk & (1 << 0))
		tpid01.bf.tpid_0 = aftTpid.tpid_0;
	if(tpid_set_msk & (1 << 1))
		tpid01.bf.tpid_1 = aftTpid.tpid_1;
	if(tpid_set_msk & (1 << 2))
		tpid23.bf.tpid_2 = aftTpid.tpid_2;
	if(tpid_set_msk & (1 << 3))
		tpid23.bf.tpid_3 = aftTpid.tpid_3;

	rtk_9607f_asic_dma_lso_reg_write(tpid01.wrd, DMA_SEC_DMA_AFT_TPID_0);
	rtk_9607f_asic_dma_lso_reg_write(tpid23.wrd, DMA_SEC_DMA_AFT_TPID_1);

	return ASIC_RET_SUCCESS;
}
asic_ret_t rtk_9607f_asic_dmaAftTpid_get(rtk_9607f_asic_dmaAftTpid_t *pAftTpid)
{
	DMA_SEC_DMA_AFT_TPID_0_t	tpid01;
	DMA_SEC_DMA_AFT_TPID_1_t	tpid23;

	tpid01.wrd = rtk_9607f_asic_dma_lso_reg_read(DMA_SEC_DMA_AFT_TPID_0);
	tpid23.wrd = rtk_9607f_asic_dma_lso_reg_read(DMA_SEC_DMA_AFT_TPID_1);

	pAftTpid->tpid_0 = tpid01.bf.tpid_0;
	pAftTpid->tpid_1 = tpid01.bf.tpid_1;
	pAftTpid->tpid_2 = tpid23.bf.tpid_2;
	pAftTpid->tpid_3 = tpid23.bf.tpid_3;

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_fc_asic_l2te_policer_enable_set(uint8_t enable)
{
	asic_ret_t ret = ASIC_RET_SUCCESS;
	L2TM_L2TE_GLB_CTRL_t ctrl;

	ctrl.wrd = rtk_9607f_asic_ne_reg_read(L2TM_L2TE_GLB_CTRL);
	ctrl.bf.pol_en = enable;
	rtk_9607f_asic_ne_reg_write(ctrl.wrd, L2TM_L2TE_GLB_CTRL);

	return ret;
}

/* ifg: global configuration */
asic_ret_t rtk_9607f_l3_policer_ifg_config_set(bool ifgInclude)
{
	aal_l3_te_policer_cfg_t l3_pol_cfg;
	aal_l3_te_policer_cfg_msk_t l3_pol_msk;
	uint32_t i = 0;

	/* L3 */
	memset((void *)&l3_pol_cfg, 0, sizeof(l3_pol_cfg));
	memset((void *)&l3_pol_msk, 0, sizeof(l3_pol_msk));

	/*setup L3FE_GLB_TE_OPTION[8-9] when intial, it will add 4 CRC lenth to L3TE ingress and egress packet count*/
	if(ifgInclude)
		l3_pol_cfg.ipg = 20;
	else
		l3_pol_cfg.ipg = 0;

	l3_pol_msk.s.ipg = 1;

	for(i = 0 ; i <= CA_AAL_L3_POLICER_PKT_TYPE_UBC ; i++)
	{
		if(aal_l3_te_policer_pkt_type_cfg_set(CA_DEF_DEVID, i, l3_pol_msk, &l3_pol_cfg) != CA_E_OK)
		{
			ERROR("%s: error - failed to set ipg for packet type %u\n", __func__, i);
			return ASIC_RET_FAIL;
		}
	}

	for(i = 0 ; i <= CA_AAL_L3_MAX_PORT_ID ; i++)
	{
		if(aal_l3_te_policer_port_cfg_set(CA_DEF_DEVID, i, l3_pol_msk, &l3_pol_cfg) != CA_E_OK)
		{
			ERROR("%s: error - failed to set ipg for port %d\n", __func__, i);
			return ASIC_RET_FAIL;
		}
	}

	for(i = 0 ; i <= CA_AAL_L3_MAX_FLOW_ID ; i++)
	{
		if(aal_l3_te_policer_flow_cfg_set(CA_DEF_DEVID, i, l3_pol_msk, &l3_pol_cfg) != CA_E_OK)
		{
			ERROR("%s: error - failed to set ipg for flow %d\n", __func__, i);
			return ASIC_RET_FAIL;
		}
	}

	//ipg configuration is regarded as a global setting, set to policer2 and policer3 at the same time
	{
		ca_uint32_t maxIdx = 0;
		TE_TE_GLB_CTRL_t glb_ctrl;

		glb_ctrl.wrd = rtk_ne_reg_read(TE_TE_GLB_CTRL);

		if(glb_ctrl.bf.sel3flows == 0)
			maxIdx = CA_AAL_L3_MAX_AGR_FLOW_ID;
		else if(glb_ctrl.bf.sel3flows == 1)
			maxIdx = CA_AAL_L3_MAX_FLOW2_ID;

		for(i = 0 ; i <= maxIdx ; i++)
		{
			if(aal_l3_te_policer_agr_flow_cfg_set(CA_DEF_DEVID, i, l3_pol_msk, &l3_pol_cfg) != CA_E_OK)
			{
				ERROR("%s: error - failed to set ipg for flow policer2 %d\n", __func__, i);
				return ASIC_RET_FAIL;
			}
		}
	}

	for(i = 0 ; i <= CA_AAL_L3_MAX_FLOW3_ID ; i++)
	{
		if(aal_l3_te_policer_flow3_cfg_set(CA_DEF_DEVID, i, l3_pol_msk, &l3_pol_cfg) != CA_E_OK)
		{
			ERROR("%s: error - failed to set ipg for flow policer3 %d\n", __func__, i);
			return ASIC_RET_FAIL;
		}
	}
	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_l3_policer_ifg_config_get(    bool *pIfgInclude)
{
	aal_l3_te_policer_cfg_t l3_pol_cfg;

	PARAM_CHK(pIfgInclude == NULL, ASIC_RET_NULL_POINTER);

	memset((void *)&l3_pol_cfg, 0, sizeof(l3_pol_cfg));

	/* ipg configurations for ports, flows, flow groups, and pkt type are all the same. get any of them */
	if(aal_l3_te_policer_port_cfg_get(CA_DEF_DEVID, 0, &l3_pol_cfg) != CA_E_OK)
	{
		ERROR("%s: error - failed to get ipg for port %d\n", __func__, 0);
		return ASIC_RET_FAIL;
	}

	if(l3_pol_cfg.ipg == 0)
		*pIfgInclude = FALSE;
	else
		*pIfgInclude = TRUE;

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_l3_flow_policer_mode_set(uint32_t pol_idx, bool if_enable)
{
	aal_l3_te_policer_cfg_t l3_pol_cfg;
	aal_l3_te_policer_cfg_msk_t l3_pol_msk;
	aal_l3_te_policer_profile_t l3_pol_profile_cfg;
	aal_l3_te_policer_profile_msk_t l3_pol_profile_msk;
	uint32_t group_base, i;
	uint32_t tmp_idx;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW_ID, ASIC_RET_OUT_OF_RANGE);

	if(if_enable)
	{
		memset((void *)&l3_pol_cfg, 0, sizeof(l3_pol_cfg));
		memset((void *)&l3_pol_msk, 0, sizeof(l3_pol_msk));
		l3_pol_msk.s.type = 1;
		l3_pol_cfg.type = CA_AAL_L3_POLICER_TYPE_RFC2697;
		if (aal_l3_te_policer_flow_cfg_set(CA_DEF_DEVID, pol_idx, l3_pol_msk, &l3_pol_cfg) != CA_E_OK)
		{
			ERROR("%s: failed to set configurations for l3 flow %d\n", __func__, pol_idx);
			return ASIC_RET_FAIL;
		}
	}
	else
	{
		group_base = (pol_idx/32) * 32;

		for(i = 0 ; i < 32 ; i++) /*Policer is 32 set of 1 group*/
		{
			tmp_idx = group_base + i;

			if(tmp_idx == pol_idx)
				continue;

			if (aal_l3_te_policer_flow_cfg_get(CA_DEF_DEVID, tmp_idx, &l3_pol_cfg) != CA_E_OK)
			{
				ERROR("%s: failed to get policing profile for L3 flow %d\n", __func__, tmp_idx);
				return ASIC_RET_FAIL;
			}

			if (aal_l3_te_policer_flow_profile_get(CA_DEF_DEVID, tmp_idx, &l3_pol_profile_cfg) != CA_E_OK)
			{
				ERROR("%s: failed to get policing profile for L3 flow %d\n", __func__, tmp_idx);
				return ASIC_RET_FAIL;
			}

			if ((l3_pol_profile_cfg.cir_k != CA_AAL_L3_TE_POL_MAX_RATE_K) || (l3_pol_profile_cfg.cir_m != CA_AAL_L3_TE_POL_MAX_RATE_M) ||
				(l3_pol_profile_cfg.pir_k != CA_AAL_L3_TE_POL_MAX_RATE_K) || (l3_pol_profile_cfg.pir_m != CA_AAL_L3_TE_POL_MAX_RATE_M) ||
				(l3_pol_cfg.pol_update_mode != CA_AAL_L3_POLICER_UPDATE_MODE_BYTE))
				break; //not in disable mode
		}

		if(i >= 32)
		{
			// all other 31 policers are set to maximum rate, there 32 policers can be set to disable mode
			memset((void *)&l3_pol_cfg, 0, sizeof(l3_pol_cfg));
			memset((void *)&l3_pol_msk, 0, sizeof(l3_pol_msk));
			l3_pol_msk.s.type = 1;
			l3_pol_cfg.type = CA_AAL_L3_POLICER_TYPE_NONE;
			if (aal_l3_te_policer_flow_cfg_set(CA_DEF_DEVID, pol_idx, l3_pol_msk, &l3_pol_cfg) != CA_E_OK)
			{
				ERROR("%s: failed to set configurations for l3 flow %d\n", __func__, pol_idx);
				return ASIC_RET_FAIL;
			}
		}

		// set rate to maximun value.
		memset((void *)&l3_pol_profile_cfg, 0, sizeof(l3_pol_profile_cfg));
		memset((void *)&l3_pol_profile_msk, 0, sizeof(l3_pol_profile_msk));
		l3_pol_profile_msk.s.cir_k = 1;
		l3_pol_profile_msk.s.cir_m = 1;
		l3_pol_profile_msk.s.pir_k = 1;
		l3_pol_profile_msk.s.pir_m = 1;
		l3_pol_profile_cfg.cir_k = CA_AAL_L3_TE_POL_MAX_RATE_K;
		l3_pol_profile_cfg.cir_m = CA_AAL_L3_TE_POL_MAX_RATE_M;
		l3_pol_profile_cfg.pir_k = CA_AAL_L3_TE_POL_MAX_RATE_K;
		l3_pol_profile_cfg.pir_m = CA_AAL_L3_TE_POL_MAX_RATE_M;

		if(aal_l3_te_policer_flow_profile_set(CA_DEF_DEVID, pol_idx, l3_pol_profile_msk, &l3_pol_profile_cfg) != CA_E_OK)
		{
			ERROR("%s: failed to set policing profile for l3 flow %d\n", __func__, pol_idx);
			return CA_E_ERROR;
		}
	}

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_l3_flow_policer_mode_get(uint32_t pol_idx, bool *if_enable)
{
	aal_l3_te_policer_cfg_t l3_pol_cfg;
	aal_l3_te_policer_profile_t l3_pol_profile_cfg;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW_ID, ASIC_RET_OUT_OF_RANGE);
	PARAM_CHK(if_enable == NULL, ASIC_RET_NULL_POINTER);


	if(aal_l3_te_policer_flow_cfg_get(CA_DEF_DEVID, pol_idx, &l3_pol_cfg) != CA_E_OK)
	{
		ERROR("%s: failed to get configurations for l3 flow %d\n", __func__, pol_idx);
		return ASIC_RET_FAIL;
	}

	if(l3_pol_cfg.type == CA_AAL_L3_POLICER_TYPE_NONE)
		*if_enable = FALSE;
	else
	{
		if(aal_l3_te_policer_flow_profile_get(CA_DEF_DEVID, pol_idx, &l3_pol_profile_cfg) != CA_E_OK)
		{
			ERROR("%s: failed to get policing profile for L3 flow %d\n", __func__, pol_idx);
			return ASIC_RET_FAIL;
		}
		if((l3_pol_profile_cfg.cir_k == CA_AAL_L3_TE_POL_MAX_RATE_K) && (l3_pol_profile_cfg.cir_m == CA_AAL_L3_TE_POL_MAX_RATE_M) &&
			(l3_pol_profile_cfg.pir_k == CA_AAL_L3_TE_POL_MAX_RATE_K) && (l3_pol_profile_cfg.pir_m == CA_AAL_L3_TE_POL_MAX_RATE_M) &&
			(l3_pol_cfg.pol_update_mode == CA_AAL_L3_POLICER_UPDATE_MODE_BYTE))
			*if_enable = FALSE;
		else
			*if_enable = TRUE;
	}

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_l3_flow_policer_rate_set(uint32_t pol_idx, uint32_t committed_rate, uint32_t peak_rate)
{
	aal_l3_te_policer_profile_t l3_pol_profile_cfg;
	aal_l3_te_policer_profile_msk_t l3_pol_profile_msk;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW_ID, ASIC_RET_OUT_OF_RANGE);

	memset((void *)&l3_pol_profile_cfg, 0, sizeof(l3_pol_profile_cfg));
	memset((void *)&l3_pol_profile_msk, 0, sizeof(l3_pol_profile_msk));

	l3_pol_profile_msk.s.cir_k = 1;
	l3_pol_profile_msk.s.cir_m = 1;
	l3_pol_profile_msk.s.pir_k = 1;
	l3_pol_profile_msk.s.pir_m = 1;

	l3_pol_profile_cfg.cir_k = committed_rate % 500;
	l3_pol_profile_cfg.cir_m = committed_rate / 500;
	l3_pol_profile_cfg.pir_k = peak_rate % 500;
	l3_pol_profile_cfg.pir_m = peak_rate / 500;

	if(aal_l3_te_policer_flow_profile_set(CA_DEF_DEVID, pol_idx, l3_pol_profile_msk, &l3_pol_profile_cfg) != CA_E_OK)
	{
		ERROR("%s: failed to set policing profile for l3 flow %d\n", __func__, pol_idx);
		return CA_E_ERROR;
	}

	return CA_E_OK;
}

asic_ret_t rtk_9607f_l3_flow_policer_rate_get(uint32_t pol_idx, uint32_t *committed_rate, uint32_t *peak_rate)
{
	aal_l3_te_policer_profile_t l3_pol_profile_cfg;
	uint16_t rate_k;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW_ID, ASIC_RET_OUT_OF_RANGE);
	PARAM_CHK(committed_rate == NULL, ASIC_RET_NULL_POINTER);
	PARAM_CHK(peak_rate == NULL, ASIC_RET_NULL_POINTER);

	if(aal_l3_te_policer_flow_profile_get(CA_DEF_DEVID, pol_idx, &l3_pol_profile_cfg) != CA_E_OK)
	{
		ERROR("%s: failed to set policing profile for l3 flow %d\n", __func__, pol_idx);
		return CA_E_ERROR;
	}

	rate_k = l3_pol_profile_cfg.cir_k % 500; //The valid value is 0~499.
	*committed_rate = l3_pol_profile_cfg.cir_m * 500 + rate_k;
	rate_k = l3_pol_profile_cfg.pir_k % 500; //The valid value is 0~499.
	*peak_rate = l3_pol_profile_cfg.pir_m * 500 + rate_k;

	return CA_E_OK;
}

asic_ret_t rtk_9607f_l3_flow_policer_burstSize_set(uint32_t pol_idx, uint32_t committed_burstSize, uint32_t peak_burstSize)
{
	aal_l3_te_policer_profile_t l3_pol_profile_cfg;
	aal_l3_te_policer_profile_msk_t l3_pol_profile_msk;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW_ID, ASIC_RET_OUT_OF_RANGE);

	memset((void *)&l3_pol_profile_cfg, 0, sizeof(l3_pol_profile_cfg));
	memset((void *)&l3_pol_profile_msk, 0, sizeof(l3_pol_profile_msk));

	l3_pol_profile_msk.s.cbs = 1;
	l3_pol_profile_msk.s.pbs = 1;

	l3_pol_profile_cfg.cbs = committed_burstSize;
	l3_pol_profile_cfg.pbs = peak_burstSize;

	if(aal_l3_te_policer_flow_profile_set(CA_DEF_DEVID, pol_idx, l3_pol_profile_msk, &l3_pol_profile_cfg) != CA_E_OK)
	{
		ERROR("%s: failed to set policing profile for l3 flow %d\n", __func__, pol_idx);
		return CA_E_ERROR;
	}

	return CA_E_OK;
}

asic_ret_t rtk_9607f_l3_flow_policer_burstSize_get(uint32_t pol_idx, uint32_t *committed_burstSize, uint32_t *peak_burstSize)
{
	aal_l3_te_policer_profile_t l3_pol_profile_cfg;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW_ID, ASIC_RET_OUT_OF_RANGE);
	PARAM_CHK(committed_burstSize == NULL, ASIC_RET_NULL_POINTER);
	PARAM_CHK(peak_burstSize == NULL, ASIC_RET_NULL_POINTER);

	if(aal_l3_te_policer_flow_profile_get(CA_DEF_DEVID, pol_idx, &l3_pol_profile_cfg) != CA_E_OK)
	{
		ERROR("%s: failed to set policing profile for l3 flow %d\n", __func__, pol_idx);
		return CA_E_ERROR;
	}

	*committed_burstSize = l3_pol_profile_cfg.cbs;
	*peak_burstSize = l3_pol_profile_cfg.pbs;

	return CA_E_OK;
}

asic_ret_t rtk_9607f_l3_flow_policer_rateMode_set(uint32_t pol_idx, asic_pol_rateMode_t rate_mode)
{
	aal_l3_te_policer_cfg_t l3_pol_cfg;
	aal_l3_te_policer_cfg_msk_t l3_pol_msk;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW_ID, ASIC_RET_OUT_OF_RANGE);

	memset((void *)&l3_pol_cfg, 0, sizeof(l3_pol_cfg));
	memset((void *)&l3_pol_msk, 0, sizeof(l3_pol_msk));

	l3_pol_msk.s.pol_update_mode = 1;

	if(rate_mode == ASIC_POL_RATEMODE_PPS)
		l3_pol_cfg.pol_update_mode = CA_AAL_L3_POLICER_UPDATE_MODE_PKT;
	else
		l3_pol_cfg.pol_update_mode = CA_AAL_L3_POLICER_UPDATE_MODE_BYTE;

	if (aal_l3_te_policer_flow_cfg_set(CA_DEF_DEVID, pol_idx, l3_pol_msk, &l3_pol_cfg) != CA_E_OK)
	{
		ERROR("%s: failed to set configurations for l3 flow %d\n", __func__, pol_idx);
		return ASIC_RET_FAIL;
	}

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_l3_flow_policer_rateMode_get(uint32_t pol_idx, asic_pol_rateMode_t *rate_mode)
{
	aal_l3_te_policer_cfg_t l3_pol_cfg;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW_ID, ASIC_RET_OUT_OF_RANGE);
	PARAM_CHK(rate_mode == NULL, ASIC_RET_NULL_POINTER);

	if(aal_l3_te_policer_flow_cfg_get(CA_DEF_DEVID, pol_idx, &l3_pol_cfg) != CA_E_OK)
	{
		ERROR("%s: failed to get configurations for l3 flow %d\n", __func__, pol_idx);
		return ASIC_RET_FAIL;
	}

	if(l3_pol_cfg.pol_update_mode == CA_AAL_L3_POLICER_UPDATE_MODE_PKT)
		*rate_mode = ASIC_POL_RATEMODE_PPS;
	else
		*rate_mode = ASIC_POL_RATEMODE_BPS;

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_l3_flow_policer2_mode_set(uint32_t pol_idx, bool if_enable)
{
	aal_l3_te_policer_cfg_t l3_pol_cfg;
	aal_l3_te_policer_cfg_msk_t l3_pol_msk;
	aal_l3_te_policer_profile_t l3_pol_profile_cfg;
	aal_l3_te_policer_profile_msk_t l3_pol_profile_msk;
	uint32_t group_base, i;
	uint32_t tmp_idx;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW2_ID, ASIC_RET_OUT_OF_RANGE);

	if(if_enable)
	{
		memset((void *)&l3_pol_cfg, 0, sizeof(l3_pol_cfg));
		memset((void *)&l3_pol_msk, 0, sizeof(l3_pol_msk));
		l3_pol_msk.s.type = 1;
		l3_pol_cfg.type = CA_AAL_L3_POLICER_TYPE_RFC2697;
		if (aal_l3_te_policer_agr_flow_cfg_set(CA_DEF_DEVID, pol_idx, l3_pol_msk, &l3_pol_cfg) != CA_E_OK)
		{
			ERROR("%s: failed to set configurations for l3 flow2 %d\n", __func__, pol_idx);
			return ASIC_RET_FAIL;
		}
	}
	else
	{
		group_base = (pol_idx/32) * 32;

		for(i = 0 ; i < 32 ; i++) /*Policer is 32 set of 1 group*/
		{
			tmp_idx = group_base+ i;

			if(tmp_idx == pol_idx)
				continue;

			if (aal_l3_te_policer_agr_flow_cfg_get(CA_DEF_DEVID, tmp_idx, &l3_pol_cfg) != CA_E_OK)
			{
				ERROR("%s: failed to get policing profile for L3 flow2 %d\n", __func__, tmp_idx);
				return ASIC_RET_FAIL;
			}

			if (aal_l3_te_policer_agr_flow_profile_get(CA_DEF_DEVID, tmp_idx, &l3_pol_profile_cfg) != CA_E_OK)
			{
				ERROR("%s: failed to get policing profile for L3 flow2 %d\n", __func__, tmp_idx);
				return ASIC_RET_FAIL;
			}

			if ((l3_pol_profile_cfg.cir_k != CA_AAL_L3_TE_POL_MAX_RATE_K) || (l3_pol_profile_cfg.cir_m != CA_AAL_L3_TE_POL_MAX_RATE_M) ||
				(l3_pol_profile_cfg.pir_k != CA_AAL_L3_TE_POL_MAX_RATE_K) || (l3_pol_profile_cfg.pir_m != CA_AAL_L3_TE_POL_MAX_RATE_M) ||
				(l3_pol_cfg.pol_update_mode != CA_AAL_L3_POLICER_UPDATE_MODE_BYTE))
				break; //not in disable mode
		}

		if(i >= 32)
		{
			// all other 31 policers are set to maximum rate, there 32 policers can be set to disable mode
			memset((void *)&l3_pol_cfg, 0, sizeof(l3_pol_cfg));
			memset((void *)&l3_pol_msk, 0, sizeof(l3_pol_msk));
			l3_pol_msk.s.type = 1;
			l3_pol_cfg.type = CA_AAL_L3_POLICER_TYPE_NONE;
			if (aal_l3_te_policer_agr_flow_cfg_set(CA_DEF_DEVID, pol_idx, l3_pol_msk, &l3_pol_cfg) != CA_E_OK)
			{
				ERROR("%s: failed to set configurations for l3 flow2 %d\n", __func__, pol_idx);
				return ASIC_RET_FAIL;
			}
		}

		// set rate to maximun value.
		memset((void *)&l3_pol_profile_cfg, 0, sizeof(l3_pol_profile_cfg));
		memset((void *)&l3_pol_profile_msk, 0, sizeof(l3_pol_profile_msk));
		l3_pol_profile_msk.s.cir_k = 1;
		l3_pol_profile_msk.s.cir_m = 1;
		l3_pol_profile_msk.s.pir_k = 1;
		l3_pol_profile_msk.s.pir_m = 1;
		l3_pol_profile_cfg.cir_k = CA_AAL_L3_TE_POL_MAX_RATE_K;
		l3_pol_profile_cfg.cir_m = CA_AAL_L3_TE_POL_MAX_RATE_M;
		l3_pol_profile_cfg.pir_k = CA_AAL_L3_TE_POL_MAX_RATE_K;
		l3_pol_profile_cfg.pir_m = CA_AAL_L3_TE_POL_MAX_RATE_M;

		if(aal_l3_te_policer_agr_flow_profile_set(CA_DEF_DEVID, pol_idx, l3_pol_profile_msk, &l3_pol_profile_cfg) != CA_E_OK)
		{
			ERROR("%s: failed to set policing profile for l3 flow2 %d\n", __func__, pol_idx);
			return CA_E_ERROR;
		}
	}

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_l3_flow_policer2_mode_get(uint32_t pol_idx, bool *if_enable)
{
	aal_l3_te_policer_cfg_t l3_pol_cfg;
	aal_l3_te_policer_profile_t l3_pol_profile_cfg;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW2_ID, ASIC_RET_OUT_OF_RANGE);
	PARAM_CHK(if_enable == NULL, ASIC_RET_NULL_POINTER);

	if(aal_l3_te_policer_agr_flow_cfg_get(CA_DEF_DEVID, pol_idx, &l3_pol_cfg) != CA_E_OK)
	{
		ERROR("%s: failed to get configurations for l3 flow2 %d\n", __func__, pol_idx);
		return ASIC_RET_FAIL;
	}

	if(l3_pol_cfg.type == CA_AAL_L3_POLICER_TYPE_NONE)
		*if_enable = FALSE;
	else
	{
		if(aal_l3_te_policer_agr_flow_profile_get(CA_DEF_DEVID, pol_idx, &l3_pol_profile_cfg) != CA_E_OK)
		{
			ERROR("%s: failed to get policing profile for L3 flow2 %d\n", __func__, pol_idx);
			return ASIC_RET_FAIL;
		}
		// maximum rate
		if((l3_pol_profile_cfg.cir_k == CA_AAL_L3_TE_POL_MAX_RATE_K) && (l3_pol_profile_cfg.cir_m == CA_AAL_L3_TE_POL_MAX_RATE_M) &&
			(l3_pol_profile_cfg.pir_k == CA_AAL_L3_TE_POL_MAX_RATE_K) && (l3_pol_profile_cfg.pir_m == CA_AAL_L3_TE_POL_MAX_RATE_M) &&
			(l3_pol_cfg.pol_update_mode == CA_AAL_L3_POLICER_UPDATE_MODE_BYTE))
			*if_enable = FALSE;
		else
			*if_enable = TRUE;
	}

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_l3_flow_policer2_rate_set(uint32_t pol_idx, uint32_t committed_rate, uint32_t peak_rate)
{
	aal_l3_te_policer_profile_t l3_pol_profile_cfg;
	aal_l3_te_policer_profile_msk_t l3_pol_profile_msk;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW2_ID, ASIC_RET_OUT_OF_RANGE);

	memset((void *)&l3_pol_profile_cfg, 0, sizeof(l3_pol_profile_cfg));
	memset((void *)&l3_pol_profile_msk, 0, sizeof(l3_pol_profile_msk));

	l3_pol_profile_msk.s.cir_k = 1;
	l3_pol_profile_msk.s.cir_m = 1;
	l3_pol_profile_msk.s.pir_k = 1;
	l3_pol_profile_msk.s.pir_m = 1;

	l3_pol_profile_cfg.cir_k = committed_rate % 1000;
	l3_pol_profile_cfg.cir_m = committed_rate / 1000;
	l3_pol_profile_cfg.pir_k = peak_rate % 1000;
	l3_pol_profile_cfg.pir_m = peak_rate / 1000;

	if(aal_l3_te_policer_agr_flow_profile_set(CA_DEF_DEVID, pol_idx, l3_pol_profile_msk, &l3_pol_profile_cfg) != CA_E_OK)
	{
		ERROR("%s: failed to set policing profile for l3 flow2 %d\n", __func__, pol_idx);
		return CA_E_ERROR;
	}

	return CA_E_OK;
}

asic_ret_t rtk_9607f_l3_flow_policer2_rate_get(uint32_t pol_idx, uint32_t *committed_rate, uint32_t *peak_rate)
{
	aal_l3_te_policer_profile_t l3_pol_profile_cfg;
	uint16_t rate_k;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW2_ID, ASIC_RET_OUT_OF_RANGE);
	PARAM_CHK(committed_rate == NULL, ASIC_RET_NULL_POINTER);
	PARAM_CHK(peak_rate == NULL, ASIC_RET_NULL_POINTER);

	if(aal_l3_te_policer_agr_flow_profile_get(CA_DEF_DEVID, pol_idx, &l3_pol_profile_cfg) != CA_E_OK)
	{
		ERROR("%s: failed to set policing profile for l3 flow2 %d\n", __func__, pol_idx);
		return CA_E_ERROR;
	}

	rate_k =  l3_pol_profile_cfg.cir_k % 1000; //The valid value is 0~999.
	*committed_rate = l3_pol_profile_cfg.cir_m * 1000 + rate_k;
	rate_k = l3_pol_profile_cfg.pir_k % 1000; //The valid value is 0~999.
	*peak_rate = l3_pol_profile_cfg.pir_m * 1000 + l3_pol_profile_cfg.pir_k;

	return CA_E_OK;
}

asic_ret_t rtk_9607f_l3_flow_policer2_burstSize_set(uint32_t pol_idx, uint32_t committed_burstSize, uint32_t peak_burstSize)
{
	aal_l3_te_policer_profile_t l3_pol_profile_cfg;
	aal_l3_te_policer_profile_msk_t l3_pol_profile_msk;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW2_ID, ASIC_RET_OUT_OF_RANGE);

	memset((void *)&l3_pol_profile_cfg, 0, sizeof(l3_pol_profile_cfg));
	memset((void *)&l3_pol_profile_msk, 0, sizeof(l3_pol_profile_msk));

	l3_pol_profile_msk.s.cbs = 1;
	l3_pol_profile_msk.s.pbs = 1;

	l3_pol_profile_cfg.cbs = committed_burstSize;
	l3_pol_profile_cfg.pbs = peak_burstSize;

	if(aal_l3_te_policer_agr_flow_profile_set(CA_DEF_DEVID, pol_idx, l3_pol_profile_msk, &l3_pol_profile_cfg) != CA_E_OK)
	{
		ERROR("%s: failed to set policing profile for l3 flow2 %d\n", __func__, pol_idx);
		return CA_E_ERROR;
	}

	return CA_E_OK;
}

asic_ret_t rtk_9607f_l3_flow_policer2_burstSize_get(uint32_t pol_idx, uint32_t *committed_burstSize, uint32_t *peak_burstSize)
{
	aal_l3_te_policer_profile_t l3_pol_profile_cfg;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW2_ID, ASIC_RET_OUT_OF_RANGE);
	PARAM_CHK(committed_burstSize == NULL, ASIC_RET_NULL_POINTER);
	PARAM_CHK(peak_burstSize == NULL, ASIC_RET_NULL_POINTER);


	if(aal_l3_te_policer_agr_flow_profile_get(CA_DEF_DEVID, pol_idx, &l3_pol_profile_cfg) != CA_E_OK)
	{
		ERROR("%s: failed to set policing profile for l3 flow2 %d\n", __func__, pol_idx);
		return CA_E_ERROR;
	}

	*committed_burstSize = l3_pol_profile_cfg.cbs;
	*peak_burstSize = l3_pol_profile_cfg.pbs;

	return CA_E_OK;
}

asic_ret_t rtk_9607f_l3_flow_policer2_rateMode_set(uint32_t pol_idx, asic_pol_rateMode_t rate_mode)
{
	aal_l3_te_policer_cfg_t l3_pol_cfg;
	aal_l3_te_policer_cfg_msk_t l3_pol_msk;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW2_ID, ASIC_RET_OUT_OF_RANGE);

	memset((void *)&l3_pol_cfg, 0, sizeof(l3_pol_cfg));
	memset((void *)&l3_pol_msk, 0, sizeof(l3_pol_msk));

	l3_pol_msk.s.pol_update_mode = 1;

	if(rate_mode == ASIC_POL_RATEMODE_PPS)
		l3_pol_cfg.pol_update_mode = CA_AAL_L3_POLICER_UPDATE_MODE_PKT;
	else
		l3_pol_cfg.pol_update_mode = CA_AAL_L3_POLICER_UPDATE_MODE_BYTE;

	if (aal_l3_te_policer_agr_flow_cfg_set(CA_DEF_DEVID, pol_idx, l3_pol_msk, &l3_pol_cfg) != CA_E_OK)
	{
		ERROR("%s: failed to set configurations for l3 flow2 %d\n", __func__, pol_idx);
		return ASIC_RET_FAIL;
	}

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_l3_flow_policer2_rateMode_get(uint32_t pol_idx, asic_pol_rateMode_t *rate_mode)
{
	aal_l3_te_policer_cfg_t l3_pol_cfg;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW2_ID, ASIC_RET_OUT_OF_RANGE);
	PARAM_CHK(rate_mode == NULL, ASIC_RET_NULL_POINTER);

	if(aal_l3_te_policer_agr_flow_cfg_get(CA_DEF_DEVID, pol_idx, &l3_pol_cfg) != CA_E_OK)
	{
		ERROR("%s: failed to get configurations for l3 flow2 %d\n", __func__, pol_idx);
		return ASIC_RET_FAIL;
	}

	if(l3_pol_cfg.pol_update_mode == CA_AAL_L3_POLICER_UPDATE_MODE_PKT)
		*rate_mode = ASIC_POL_RATEMODE_PPS;
	else
		*rate_mode = ASIC_POL_RATEMODE_BPS;

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_l3_flow_policer3_mode_set(uint32_t pol_idx, bool if_enable)
{
	aal_l3_te_policer_cfg_t l3_pol_cfg;
	aal_l3_te_policer_cfg_msk_t l3_pol_msk;
	aal_l3_te_policer_profile_t l3_pol_profile_cfg;
	aal_l3_te_policer_profile_msk_t l3_pol_profile_msk;
	uint32_t group_base, i;
	uint32_t tmp_idx;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW3_ID, ASIC_RET_OUT_OF_RANGE);

	if(if_enable)
	{
		memset((void *)&l3_pol_cfg, 0, sizeof(l3_pol_cfg));
		memset((void *)&l3_pol_msk, 0, sizeof(l3_pol_msk));
		l3_pol_msk.s.type = 1;
		l3_pol_cfg.type = CA_AAL_L3_POLICER_TYPE_RFC2697;
		if (aal_l3_te_policer_flow3_cfg_set(CA_DEF_DEVID, pol_idx, l3_pol_msk, &l3_pol_cfg) != CA_E_OK)
		{
			ERROR("%s: failed to set configurations for l3 flow3 %d\n", __func__, pol_idx);
			return ASIC_RET_FAIL;
		}
	}
	else
	{
		group_base = (pol_idx/32) * 32;

		for(i = 0 ; i < 32 ; i++) /*Policer is 32 set of 1 group*/
		{
			tmp_idx = group_base + i;

			if(tmp_idx == pol_idx)
				continue;

			if (aal_l3_te_policer_flow3_cfg_get(CA_DEF_DEVID, tmp_idx, &l3_pol_cfg) != CA_E_OK)
			{
				ERROR("%s: failed to get policing profile for L3 flow3 %d\n", __func__, tmp_idx);
				return ASIC_RET_FAIL;
			}

			if(aal_l3_te_policer_flow3_profile_get(CA_DEF_DEVID, tmp_idx, &l3_pol_profile_cfg) != CA_E_OK)
			{
				ERROR("%s: failed to get policing profile for L3 flow3 %d\n", __func__, tmp_idx);
				return ASIC_RET_FAIL;
			}

			if ((l3_pol_profile_cfg.cir_k != CA_AAL_L3_TE_POL_MAX_RATE_K) || (l3_pol_profile_cfg.cir_m != CA_AAL_L3_TE_POL_MAX_RATE_M) ||
				(l3_pol_profile_cfg.pir_k != CA_AAL_L3_TE_POL_MAX_RATE_K) || (l3_pol_profile_cfg.pir_m != CA_AAL_L3_TE_POL_MAX_RATE_M) ||
				(l3_pol_cfg.pol_update_mode != CA_AAL_L3_POLICER_UPDATE_MODE_BYTE))
				break; //not in disable mode
		}

		if(i >= 32)
		{
			// all other 31 policers are set to maximum rate, there 32 policers can be set to disable mode
			memset((void *)&l3_pol_cfg, 0, sizeof(l3_pol_cfg));
			memset((void *)&l3_pol_msk, 0, sizeof(l3_pol_msk));
			l3_pol_msk.s.type = 1;
			l3_pol_cfg.type = CA_AAL_L3_POLICER_TYPE_NONE;
			if(aal_l3_te_policer_flow3_cfg_set(CA_DEF_DEVID, pol_idx, l3_pol_msk, &l3_pol_cfg) != CA_E_OK)
			{
				ERROR("%s: failed to set configurations for l3 flow3 %d\n", __func__, pol_idx);
				return ASIC_RET_FAIL;
			}
		}

		// set rate to maximun value.
		memset((void *)&l3_pol_profile_cfg, 0, sizeof(l3_pol_profile_cfg));
		memset((void *)&l3_pol_profile_msk, 0, sizeof(l3_pol_profile_msk));
		l3_pol_profile_msk.s.cir_k = 1;
		l3_pol_profile_msk.s.cir_m = 1;
		l3_pol_profile_msk.s.pir_k = 1;
		l3_pol_profile_msk.s.pir_m = 1;
		l3_pol_profile_cfg.cir_k = CA_AAL_L3_TE_POL_MAX_RATE_K;
		l3_pol_profile_cfg.cir_m = CA_AAL_L3_TE_POL_MAX_RATE_M;
		l3_pol_profile_cfg.pir_k = CA_AAL_L3_TE_POL_MAX_RATE_K;
		l3_pol_profile_cfg.pir_m = CA_AAL_L3_TE_POL_MAX_RATE_M;

		if (aal_l3_te_policer_flow3_profile_set(CA_DEF_DEVID, pol_idx, l3_pol_profile_msk, &l3_pol_profile_cfg) != CA_E_OK)
		{
			ERROR("%s: failed to set policing profile for l3 flow3 %d\n", __func__, pol_idx);
			return CA_E_ERROR;
		}
	}

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_l3_flow_policer3_mode_get(uint32_t pol_idx, bool *if_enable)
{
	aal_l3_te_policer_cfg_t l3_pol_cfg;
	aal_l3_te_policer_profile_t l3_pol_profile_cfg;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW3_ID, ASIC_RET_OUT_OF_RANGE);
	PARAM_CHK(if_enable == NULL, ASIC_RET_NULL_POINTER);

	if(aal_l3_te_policer_flow3_cfg_get(CA_DEF_DEVID, pol_idx, &l3_pol_cfg) != CA_E_OK)
	{
		ERROR("%s: failed to get configurations for l3 flow3 %d\n", __func__, pol_idx);
		return ASIC_RET_FAIL;
	}

	if(l3_pol_cfg.type == CA_AAL_L3_POLICER_TYPE_NONE)
	*if_enable = FALSE;
	else
	{
		if (aal_l3_te_policer_flow3_profile_get(CA_DEF_DEVID, pol_idx, &l3_pol_profile_cfg) != CA_E_OK)
		{
			ERROR("%s: failed to get policing profile for L3 flow3 %d\n", __func__, pol_idx);
			return ASIC_RET_FAIL;
		}
		if ((l3_pol_profile_cfg.cir_k == CA_AAL_L3_TE_POL_MAX_RATE_K) && (l3_pol_profile_cfg.cir_m == CA_AAL_L3_TE_POL_MAX_RATE_M) &&
			(l3_pol_profile_cfg.pir_k == CA_AAL_L3_TE_POL_MAX_RATE_K) && (l3_pol_profile_cfg.pir_m == CA_AAL_L3_TE_POL_MAX_RATE_M) &&
			(l3_pol_cfg.pol_update_mode == CA_AAL_L3_POLICER_UPDATE_MODE_BYTE))
			*if_enable = FALSE;
		else
		*if_enable = TRUE;
	}

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_l3_flow_policer3_rate_set(uint32_t pol_idx, uint32_t committed_rate, uint32_t peak_rate)
{
	aal_l3_te_policer_profile_t l3_pol_profile_cfg;
	aal_l3_te_policer_profile_msk_t l3_pol_profile_msk;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW3_ID, ASIC_RET_OUT_OF_RANGE);

	memset((void *)&l3_pol_profile_cfg, 0, sizeof(l3_pol_profile_cfg));
	memset((void *)&l3_pol_profile_msk, 0, sizeof(l3_pol_profile_msk));

	l3_pol_profile_msk.s.cir_k = 1;
	l3_pol_profile_msk.s.cir_m = 1;
	l3_pol_profile_msk.s.pir_k = 1;
	l3_pol_profile_msk.s.pir_m = 1;

	l3_pol_profile_cfg.cir_k = committed_rate % 1000;
	l3_pol_profile_cfg.cir_m = committed_rate / 1000;
	l3_pol_profile_cfg.pir_k = peak_rate % 1000;
	l3_pol_profile_cfg.pir_m = peak_rate / 1000;

	if(aal_l3_te_policer_flow3_profile_set(CA_DEF_DEVID, pol_idx, l3_pol_profile_msk, &l3_pol_profile_cfg) != CA_E_OK)
	{
		ERROR("%s: failed to set policing profile for l3 flow3 %d\n", __func__, pol_idx);
		return CA_E_ERROR;
	}

	return CA_E_OK;
}

asic_ret_t rtk_9607f_l3_flow_policer3_rate_get(uint32_t pol_idx, uint32_t *committed_rate, uint32_t *peak_rate)
{
	aal_l3_te_policer_profile_t l3_pol_profile_cfg;
	uint16_t rate_k;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW3_ID, ASIC_RET_OUT_OF_RANGE);
	PARAM_CHK(committed_rate == NULL, ASIC_RET_NULL_POINTER);
	PARAM_CHK(peak_rate == NULL, ASIC_RET_NULL_POINTER);

	if (aal_l3_te_policer_flow3_profile_get(CA_DEF_DEVID, pol_idx, &l3_pol_profile_cfg) != CA_E_OK)
	{
		ERROR("%s: failed to set policing profile for l3 flow3 %d\n", __func__, pol_idx);
		return CA_E_ERROR;
	}

	rate_k =  l3_pol_profile_cfg.cir_k % 1000; //The valid value is 0~999.
	*committed_rate = l3_pol_profile_cfg.cir_m * 1000 + l3_pol_profile_cfg.cir_k;
	rate_k =  l3_pol_profile_cfg.pir_k % 1000; //The valid value is 0~999.
	*peak_rate = l3_pol_profile_cfg.pir_m * 1000 + l3_pol_profile_cfg.pir_k;

	return CA_E_OK;
}

asic_ret_t rtk_9607f_l3_flow_policer3_burstSize_set(uint32_t pol_idx, uint32_t committed_burstSize, uint32_t peak_burstSize)
{
	aal_l3_te_policer_profile_t l3_pol_profile_cfg;
	aal_l3_te_policer_profile_msk_t l3_pol_profile_msk;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW3_ID, ASIC_RET_OUT_OF_RANGE);

	memset((void *)&l3_pol_profile_cfg, 0, sizeof(l3_pol_profile_cfg));
	memset((void *)&l3_pol_profile_msk, 0, sizeof(l3_pol_profile_msk));

	l3_pol_profile_msk.s.cbs = 1;
	l3_pol_profile_msk.s.pbs = 1;

	l3_pol_profile_cfg.cbs = committed_burstSize;
	l3_pol_profile_cfg.pbs = peak_burstSize;

	if(aal_l3_te_policer_flow3_profile_set(CA_DEF_DEVID, pol_idx, l3_pol_profile_msk, &l3_pol_profile_cfg) != CA_E_OK)
	{
		ERROR("%s: failed to set policing profile for l3 flow3 %d\n", __func__, pol_idx);
		return CA_E_ERROR;
	}

	return CA_E_OK;
}

asic_ret_t rtk_9607f_l3_flow_policer3_burstSize_get(uint32_t pol_idx, uint32_t *committed_burstSize, uint32_t *peak_burstSize)
{
	aal_l3_te_policer_profile_t l3_profile_cfg;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW3_ID, ASIC_RET_OUT_OF_RANGE);
	PARAM_CHK(committed_burstSize == NULL, ASIC_RET_NULL_POINTER);
	PARAM_CHK(peak_burstSize == NULL, ASIC_RET_NULL_POINTER);

	if (aal_l3_te_policer_flow3_profile_get(CA_DEF_DEVID, pol_idx, &l3_profile_cfg) != CA_E_OK)
	{
		ERROR("%s: failed to set policing profile for l3 flow3 %d\n", __func__, pol_idx);
		return CA_E_ERROR;
	}

	*committed_burstSize = l3_profile_cfg.cbs;
	*peak_burstSize = l3_profile_cfg.pbs;

	return CA_E_OK;
}

asic_ret_t rtk_9607f_l3_flow_policer3_rateMode_set(uint32_t pol_idx, asic_pol_rateMode_t rate_mode)
{
	aal_l3_te_policer_cfg_t l3_pol_cfg;
	aal_l3_te_policer_cfg_msk_t l3_pol_msk;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW3_ID, ASIC_RET_OUT_OF_RANGE);

	memset((void *)&l3_pol_cfg, 0, sizeof(l3_pol_cfg));
	memset((void *)&l3_pol_msk, 0, sizeof(l3_pol_msk));

	l3_pol_msk.s.pol_update_mode = 1;

	if(rate_mode == ASIC_POL_RATEMODE_PPS)
		l3_pol_cfg.pol_update_mode = CA_AAL_L3_POLICER_UPDATE_MODE_PKT;
	else
		l3_pol_cfg.pol_update_mode = CA_AAL_L3_POLICER_UPDATE_MODE_BYTE;

	if (aal_l3_te_policer_flow3_cfg_set(CA_DEF_DEVID, pol_idx, l3_pol_msk, &l3_pol_cfg) != CA_E_OK)
	{
		ERROR("%s: failed to set configurations for l3 flow3 %d\n", __func__, pol_idx);
		return ASIC_RET_FAIL;
	}

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_l3_flow_policer3_rateMode_get(uint32_t pol_idx, asic_pol_rateMode_t *rate_mode)
{
	aal_l3_te_policer_cfg_t l3_pol_cfg;

	PARAM_CHK(pol_idx > CA_AAL_L3_MAX_FLOW3_ID, ASIC_RET_OUT_OF_RANGE);
	PARAM_CHK(rate_mode == NULL, ASIC_RET_NULL_POINTER);

	if(aal_l3_te_policer_flow3_cfg_get(CA_DEF_DEVID, pol_idx, &l3_pol_cfg) != CA_E_OK)
	{
		ERROR("%s: failed to get configurations for l3 flow3 %d\n", __func__, pol_idx);
		return ASIC_RET_FAIL;
	}

	if(l3_pol_cfg.pol_update_mode == CA_AAL_L3_POLICER_UPDATE_MODE_PKT)
		*rate_mode = ASIC_POL_RATEMODE_PPS;
	else
		*rate_mode = ASIC_POL_RATEMODE_BPS;

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_l3_pe_dual_fmr_add(uint32 fmr_idx, void *fmr_entry)
{
	int ret;
	l3pe_dual_FMR_tbl_entry_t *pEntry=(l3pe_dual_FMR_tbl_entry_t *)fmr_entry;
	
	ret=aal_pe_dual_fmr_entry_add(fmr_idx, pEntry);
	if(ret!=AAL_E_OK){
		ERROR("%s: failed to add fmr entry %d\n", __func__, fmr_idx);
		return ASIC_RET_FAIL;
	}
	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_l3_pe_dual_fmr_get(uint32 fmr_idx, void *fmr_entry)
{
	int ret;
	l3pe_dual_FMR_tbl_entry_t *pEntry=(l3pe_dual_FMR_tbl_entry_t *)fmr_entry;
	
	ret=aal_pe_dual_fmr_entry_get(fmr_idx, pEntry);
	if(ret == AAL_E_OK )
		return ASIC_RET_SUCCESS;
	else if(ret == AAL_E_NULLPTR)
		return ASIC_RET_NULL_POINTER;
	else if(ret == AAL_E_ENTRYNOTRSVD)
		return ASIC_RET_ENTRYNOTRSVD;
	else if(ret == AAL_E_ENTRYNOTFOUND)
		return ASIC_RET_NOT_FOUND;
	else
		return ASIC_RET_FAIL;
}

asic_ret_t rtk_9607f_l3_pe_dual_fmr_del(uint32 fmr_idx)
{
	int ret;
	ret=aal_pe_dual_fmr_entry_del(fmr_idx);
	if(ret!=AAL_E_OK){
		ERROR("%s: failed to del fmr entry %d\n", __func__, fmr_idx);
		return ASIC_RET_FAIL;
	}
	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_9607f_asic_fb_init(void)
{
	int i=0;

	DEBUG("ASIC INIT");

	/*
	 * Table init
	 */
	for(i=0; i<FE_L3E_IF_TBL_ENTRY_MAX; i++) {
		rtk_9607f_asic_netifTable_del(i);
	}
	{
		rtk_9607f_asic_dmaAftFib_t fib;
		rtk_9607f_asic_dmaAftTpid_t tpidConf;

		memset(&fib, 0, sizeof(fib));
		for(i=0; i<DMA_AFT_FIB_SIZE; i++)
			rtk_9607f_asic_dmaAftFib_set(i, fib);

		tpidConf.tpid_0 = 0x8100;
		tpidConf.tpid_1 = 0x88a8;
		tpidConf.tpid_2 = 0x9100;
		tpidConf.tpid_3 = 0x9200;
		rtk_9607f_asic_dmaAftTpid_set(0xf, tpidConf);
	}

	rtk_9607f_asic_l2fe_init();

	rtk_9607f_asic_l3fe_init();

	rtk_9607f_asic_l3te_init();

	rtk_9607f_asic_l3qm_init();

	rtk_9607f_asic_ni_init();

	rtk_9607f_asic_flow_flush();

	return ASIC_RET_SUCCESS;
}

/*==================== Fom ca_ext.c START====================*/
uint32_t rtk_9607f_asic_ne_reg_read(uint32_t addr)
{
	return rtk_ne_reg_read(addr);
}

void rtk_9607f_asic_ne_reg_write(uint32_t data, uint32_t addr)
{
	rtk_ne_reg_write(data, addr);
}
uint32_t rtk_9607f_asic_dma_lso_reg_read(uint32_t addr)
{
	return rtk_dma_lso_reg_read(addr);
}

void rtk_9607f_asic_dma_lso_reg_write(uint32_t data, uint32_t addr)
{
	rtk_dma_lso_reg_write(data, addr);
}
#if !defined(CONFIG_REALTEK_BOARD_FPGA)
#if !defined(CONFIG_FC_RTL9607F_SERIES)
//TODO: 9607F FBM
uint32_t rtk_9607f_asic_fbm_glb_reg_read(uint32_t addr)
{
	return rtk_fbm_glb_reg_read(addr);
}

void rtk_9607f_asic_fbm_glb_reg_write(uint32_t data, uint32_t addr)
{
	rtk_fbm_glb_reg_write(data, addr);
}

uint32_t rtk_9607f_asic_fbm_axi_reg_read(uint32_t addr)
{
	return rtk_fbm_axi_reg_read(addr);
}

void rtk_9607f_asic_fbm_axi_reg_write(uint32_t data, uint32_t addr)
{
	rtk_fbm_axi_reg_write(data, addr);
}

uint32_t rtk_9607f_asic_fbm_cpu_reg_read(uint32_t addr)
{
	return rtk_fbm_cpu_reg_read(addr);
}

void rtk_9607f_asic_fbm_cpu_reg_write(uint32_t data, uint32_t addr)
{
	rtk_fbm_cpu_reg_write(data, addr);
}

uint32_t rtk_9607f_asic_fbm_pool_reg_read(uint32_t addr)
{
	return rtk_fbm_pool_reg_read(addr);
}

void rtk_9607f_asic_fbm_pool_reg_write(uint32_t data, uint32_t addr)
{
	rtk_fbm_pool_reg_write(data, addr);
}
#endif

#endif
int rtk_9607f_asic_l3fe_keep_lspid_unchange_set(ca_device_id_t device_id, ca_uint8_t *config)
{
	int ret;
	L3FE_STG0_CTRL_t stg0_ctrl;

	if ((ret = aal_l3fe_stg0_ctrl_get(device_id, &stg0_ctrl)) != AAL_E_OK) {
		ERROR("%s: Fail to get STG0 ctrl. (ret=%d)\n", __func__, ret);
		return ret;
	}
	DEBUG("=== device_id %u ===\n", device_id);
	DEBUG("=== L3FE_STG0_CTRL : 0x%08x ===\n", stg0_ctrl.wrd);
	DEBUG("keep_lspid_unchange=%u\n", stg0_ctrl.bf.keep_lspid_unchange);

	stg0_ctrl.bf.keep_lspid_unchange=*config;

	if ((ret = aal_l3fe_stg0_ctrl_set(0, &stg0_ctrl)) != AAL_E_OK) {
		ERROR("%s: Fail to set STG0 ctrl. (ret=%d)\n", __func__, ret);
		return ret;
	}

	return CA_E_OK;
}

/*==================== Fom ca_ext.c END====================*/


int _rtk_9607f_dataBase_init(void)
{
	int i, j;
	memset(&asic_db, 0, sizeof(asic_db));

	for(i = 0 ; i < RTK_9607F_FLOW_PROFILE_MAX ; i++)
		for(j = 0 ; j < RTK_9607F_MAX_HASH_TUPLE_PER_PROFILE ; j++)
			asic_db.flow_profile_info[i].tuple_info[j].hash_mask_idx = FAIL;

	for(i = 0 ; i < RTK_9607F_FLOW_DEFACT_TYPE_MAX ; i++)
		asic_db.flow_defAct_idx[i] = FAIL;

	return ASIC_RET_SUCCESS;
}

static uint32 _rtk_9607f_cpuport_hash_preprocess_mac(uint8 *mac, aal_cpu_remap_hash_algo_t cf_ptn, uint8 *mac_aftr)
{
	uint32 shiftdir = cf_ptn.left_rotation;
	uint32 shiftbits = cf_ptn.shift_0_to_7;
	uint32 xoroperatedvalue = cf_ptn.xor_process  & 0xfffff;
	uint32 msb_28bits = ((mac[0]<<20) | (mac[1]<<12) | (mac[2]<<4) | ((mac[3]>>4)&0xF));
	uint32 lsb_20bits =(((mac[3]&0xF)<<16) | (mac[4]<<8) | (mac[5]));
	uint32 mac_tmp = 0;
	
	if(shiftdir){
		mac_tmp = ((lsb_20bits << shiftbits) & 0xfffff) | (lsb_20bits>> (20 - shiftbits));
		//printk("left shift - mac before %pM, after = 0x%x\n", mac, mac_tmp);
	}else{
		mac_tmp = ((lsb_20bits << (20 - shiftbits)) & 0xfffff) | (lsb_20bits>> shiftbits);
		//printk("right shift - mac before %pM, after = 0x%x\n", mac, mac_tmp);
	}

	mac_aftr[5] = mac_tmp & 0xFF;
	mac_aftr[4] = (mac_tmp >> 8) & 0xFF;
	mac_aftr[3] = (((msb_28bits & 0xF) << 4) | ((mac_tmp >> 16) & 0xF));
	mac_aftr[2] = (msb_28bits >> 4) & 0xFF;
	mac_aftr[1] = (msb_28bits >> 12) & 0xFF;
	mac_aftr[0] = (msb_28bits >> 20) & 0xFF;

	mac_aftr[5] ^= xoroperatedvalue & 0xFF;
	mac_aftr[4] ^= (xoroperatedvalue >> 8) & 0xFF;
	mac_aftr[3] ^=  ((xoroperatedvalue >> 16) & 0xF);
	
	return SUCCESS;
}

static uint32 _rtk_9607f_cpuport_hash_preprocess_ip(uint32 ip, aal_cpu_remap_hash_algo_t cf_ptn)
{
	uint32 shiftdir = cf_ptn.left_rotation;
	uint32 shiftbits = cf_ptn.shift_0_to_7;
	uint32 xoroperatedvalue = cf_ptn.xor_process  & 0xfffff;
	uint32 msb_12bits = ip & 0xfff00000;
	uint32 lsb_20bits = ip & 0x000fffff;
	uint32 ip_tmp = 0;
	
		
	if(shiftdir){
		ip_tmp = ((lsb_20bits << shiftbits) & 0xfffff) | (lsb_20bits>> (20 - shiftbits));
	}else{
		ip_tmp = ((lsb_20bits << (20 - shiftbits)) & 0xfffff) | (lsb_20bits>> shiftbits);
	}
	
	//printk("ip before 0x%x, %s shift %d bits, after = 0x%x\n", ip, shiftdir?"left":"right", shiftbits, ip_tmp);
	
	return (msb_12bits |( (ip_tmp ^ xoroperatedvalue) & 0xfffff));
}

static uint32 _rtk_9607f_cpuport_hash_preprocess_port(uint16 port, aal_cpu_remap_hash_algo_t cf_ptn)
{
	uint32 shiftdir = cf_ptn.left_rotation;
	uint32 shiftbits = cf_ptn.shift_0_to_7;
	uint32 xoroperatedvalue = cf_ptn.xor_process  & 0xffff;
	uint32 port_tmp = 0;
	
	if(shiftdir){
		port_tmp = ((port << shiftbits) & 0xffff) | (port >> (16 - shiftbits));
	}else{
		port_tmp = ((port << (16 - shiftbits)) & 0xffff) | (port >> shiftbits);
	}
	
	return (port_tmp ^ xoroperatedvalue) & 0xffff;
}

/*
 * IP/IP6[31:0]
*/
uint32_t rtk_9607f_cpuport_hash_get(uint32 o_lspid, uint8 *o_smac, uint8 *o_dmac, uint32 o_sip, uint32 o_dip, uint16 o_sport, uint16 o_dport)
{	
	#define CPUPORT_HASH_BITS 4
	#define CPUPORT_HASH_BITMASK  ((1<<CPUPORT_HASH_BITS)-1)

	aal_cpu_remap_cfg_t cfg;
	aal_cpu_remap_hash_algo_t algo;
	uint32 lspid, sip, dip;
	uint8 smac[6], dmac[6];
	uint16 sport, dport; 
	
	uint32 hash = 0, i =0;
	uint8 macBytePosi, macByteMSB, macByteValue;

	aal_l3_glb_cpu_remap_cfg_get(RTK_ASIC_DEVID, &cfg);

	/* hash - spa */
	if(cfg.olspid_hash_en) {
		lspid = o_lspid & 0x3F;
		hash = (lspid & CPUPORT_HASH_BITMASK) ^ 
				((lspid>>CPUPORT_HASH_BITS) & CPUPORT_HASH_BITMASK);
	}

	/* hash - mac */
	if(o_smac && o_dmac) {
		aal_l3_glb_cpu_remap_hashsmac_get(RTK_ASIC_DEVID, &algo);
		_rtk_9607f_cpuport_hash_preprocess_mac(o_smac, algo, &smac[0]);
		
		aal_l3_glb_cpu_remap_hashdmac_get(RTK_ASIC_DEVID, &algo);
		_rtk_9607f_cpuport_hash_preprocess_mac(o_dmac, algo, &dmac[0]);
	
		for(i  = 0; i < 48/CPUPORT_HASH_BITS ; i++) {
			macBytePosi = i / (8/CPUPORT_HASH_BITS);
			macByteMSB = i % (8/CPUPORT_HASH_BITS);

			if(cfg.smac_hash_en) {
				macByteValue = smac[macBytePosi];
				hash ^= ((macByteValue >>  (CPUPORT_HASH_BITS*macByteMSB)) & CPUPORT_HASH_BITMASK);
			}

			if(cfg.dmac_hash_en) {
				macByteValue = dmac[macBytePosi];
				hash ^= ((macByteValue >>  (CPUPORT_HASH_BITS*macByteMSB)) & CPUPORT_HASH_BITMASK);
			}
		}
	}

	/* hash - ip */
	if(o_sip || o_dip) {
		aal_l3_glb_cpu_remap_hashl3sip_get(RTK_ASIC_DEVID, &algo);
		sip = _rtk_9607f_cpuport_hash_preprocess_ip(o_sip, algo);
		
		aal_l3_glb_cpu_remap_hashl3dip_get(RTK_ASIC_DEVID, &algo);
		dip = _rtk_9607f_cpuport_hash_preprocess_ip(o_dip, algo);
		
		for(i  = 0; i < 32/CPUPORT_HASH_BITS ; i++) {
			
			if(cfg.l3sip_hash_en) {
				hash ^= ((sip >> (CPUPORT_HASH_BITS*i)) & CPUPORT_HASH_BITMASK);
			}
			
			if(cfg.l3dip_hash_en) {
				hash ^= ((dip >> (CPUPORT_HASH_BITS*i)) & CPUPORT_HASH_BITMASK);
			}
		}
	}


	/* hash - port */
	if(o_sport || o_dport) {
		aal_l3_glb_cpu_remap_hashl4sport_get(RTK_ASIC_DEVID, &algo);
		sport = _rtk_9607f_cpuport_hash_preprocess_port(o_sport, algo);

		aal_l3_glb_cpu_remap_hashl4dport_get(RTK_ASIC_DEVID, &algo);
		dport = _rtk_9607f_cpuport_hash_preprocess_port(o_dport, algo);
		
		for(i  = 0; i < 16/CPUPORT_HASH_BITS ; i++) {
			
			if(cfg.l4sport_hash_en) {
				hash ^= ((sport >> (CPUPORT_HASH_BITS*i)) & CPUPORT_HASH_BITMASK);
			}
			
			if(cfg.l4dport_hash_en) {
				hash ^= ((dport >> (CPUPORT_HASH_BITS*i)) & CPUPORT_HASH_BITMASK);
			}
		}
	}

	return (hash&CPUPORT_HASH_BITMASK);
}

uint32_t rtk_9607f_ni_virtual_cpuport_open(int cpuPort)
{
	if(cpuPort<AAL_LPORT_CPU_2 || cpuPort>AAL_LPORT_CPU_7)
		return CA_E_PORT;
	else{
		
		nic_register_rxfunc_by_port(cpuPort-AAL_LPORT_CPU_0, &nic_rx_list);
			
		return ca_ni_virtual_instance_open(cpuPort-AAL_LPORT_CPU_0);
	}
}
uint32_t rtk_9607f_ni_virtual_cpuport_close(int cpuPort)
{
	if(cpuPort<AAL_LPORT_CPU_2 || cpuPort>AAL_LPORT_CPU_7)
		return CA_E_PORT;
	else{
		
		return ca_ni_virtual_instance_close(cpuPort-AAL_LPORT_CPU_0);
	}
}

uint32_t rtk_9607f_cpuport_init(void)
{
	ca_status_t ret = CA_E_OK;
	int i;

	i = 0;

	if(!cpuport_init) {
#if defined(CONFIG_CA_NE_L2FP) || defined(CONFIG_NE_L2FP)	
		// if defined, L2FP module allocate netdev for those cpuports. ref: ca_l2fp_init

		// enable CPU port 0x12~0x17 for wifi acceleration
		for(i=ASIC_LPORT_CPU_2; i <= ASIC_LPORT_CPU_7; i++) {

			if(!(ret = rtk_9607f_ni_virtual_cpuport_open(i)))
				ASICINITPRINT("Enable CPU port 0x%x", i);
			else
				ASICINITPRINT("ERROR - Enable CPU port 0x%x FAIL, ret = 0x%x !!!!!", i ,ret);
		}

		cpuport_init = 1;

#else
		ASICINITPRINT("CA wifi acceleration (L2FP) in off!!!!!\n");

#endif
	}

	return ret;
}


uint32_t rtk_9607f_dmalso_init(void)
{
	ca_status_t ret = CA_E_OK;

	rtk_ni_init_tx_dma_lso();

	return ret;
}

uint32_t rtk_9607f_cpuport_exit(void)
{

	int i = 0;


	if(cpuport_init) {
		ASICINITPRINT("unregister cpu port...");

		for(i=ASIC_LPORT_CPU_2; i <= ASIC_LPORT_CPU_7; i++) {
			rtk_9607f_ni_virtual_cpuport_close(i);
			ASICINITPRINT("close cpu port...0x%x", i);
		}
		
		cpuport_init = 0;
	}
	return CA_E_OK;
}

uint32_t rtk_9607f_flow_default_action_init(uint32_t actGrpMsk_default)
{
	hash_action_default_t defaultAct_trap, defaultAct_drop, defaultAct_trap_meter;
	hash_default_action_entry_t default_action;
	unsigned int default_action_idx;

	/*default action size check for internal SRAM*/
	//internal SRAM for default action only 96 bits per entry

	//3 [0]/*default action entry add: TRAP*/
	memset(&defaultAct_trap, 0, sizeof(defaultAct_trap));
#if 0 // use CLS to decide whitch CPU to trap, no need to touch ldpid here
	defaultAct_trap.permit = 1;	// forward the packet
	defaultAct_trap.dpid_vld = 1;
	defaultAct_trap.dpid_pri = 1;
	defaultAct_trap.mcgid = AAL_LPORT_CPU_0;
	defaultAct_trap.mc = 0;	// mc = 0 means "mcgid is LDPID"
#endif
	defaultAct_trap.keep_orig_pkt = 1;
	defaultAct_trap.keep_orig_pkt_vld = 1;

	memset(&default_action, 0, sizeof(default_action));
	memcpy(&default_action, &defaultAct_trap, sizeof(defaultAct_trap));
	ASSERT_EQ(aal_hash_default_action_add(&default_action, &default_action_idx, actGrpMsk_default), AAL_E_OK);
	asic_db.flow_defAct_idx[RTK_9607F_FLOW_DEFACT_TYPE_TRAP] = default_action_idx;

	//3 [1]/*default action entry add: DROP*/
	memset(&defaultAct_drop, 0, sizeof(defaultAct_drop));
#if defined(CONFIG_REALTEK_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA_V8)
	defaultAct_drop.permit = 0;
	defaultAct_drop.dpid_vld = 1;
	defaultAct_drop.dpid_pri = 1;
#else // use policer to drop packet

	// set policer rate 0. Set burstSize minimum value 1
	if(rtk_9607f_l3_flow_policer_mode_set(G3_FLOW_POLICER_IDXSHIFT_HASH_DFT_DROP, 1) != ASIC_RET_SUCCESS)
		ASICINITPRINT("[Error] set policer[%d] mode for MC hash default action DROP failed.", G3_FLOW_POLICER_IDXSHIFT_HASH_DFT_DROP);
	if(rtk_9607f_l3_flow_policer_rate_set(G3_FLOW_POLICER_IDXSHIFT_HASH_DFT_DROP, 0, 0) != ASIC_RET_SUCCESS)
		ASICINITPRINT("[Error] set policer[%d] rate for MC hash default action DROP failed.", G3_FLOW_POLICER_IDXSHIFT_HASH_DFT_DROP);
	if(rtk_9607f_l3_flow_policer_burstSize_set(G3_FLOW_POLICER_IDXSHIFT_HASH_DFT_DROP, 1, 1) != ASIC_RET_SUCCESS)
		ASICINITPRINT("[Error] set policer[%d] burst size for MC hash default action DROP failed.", G3_FLOW_POLICER_IDXSHIFT_HASH_DFT_DROP);
	// set policer counter 0. This will initial policer token count to 0.
	if(_aal_l3_te_policer_flow_counter_clear(RTK_ASIC_DEVID, G3_FLOW_POLICER_IDXSHIFT_HASH_DFT_DROP) != CA_E_OK)
		ASICINITPRINT("[Error] set policer[%d] counter to 0 failed.", G3_FLOW_POLICER_IDXSHIFT_HASH_DFT_DROP);

	defaultAct_drop.pol_vld = 1;
	defaultAct_drop.pol_en = 2;
	defaultAct_drop.pol_base = G3_FLOW_POLICER_IDXSHIFT_HASH_DFT_DROP;
	defaultAct_drop.pol_sel = 2;
#endif
	memset(&default_action, 0, sizeof(default_action));
	memcpy(&default_action, &defaultAct_drop, sizeof(defaultAct_drop));
	ASSERT_EQ(aal_hash_default_action_add(&default_action, &default_action_idx, actGrpMsk_default), AAL_E_OK);
	asic_db.flow_defAct_idx[RTK_9607F_FLOW_DEFACT_TYPE_DROP] = default_action_idx;

	//3 [2]/*default action entry add: TRAP with merter*/
	memset(&defaultAct_trap_meter, 0, sizeof(defaultAct_trap_meter));
#if 0 // use CLS to decide whitch CPU to trap, no need to touch ldpid here
	defaultAct_trap_meter.permit = 1;	// forward the packet
	defaultAct_trap_meter.dpid_vld = 1;
	defaultAct_trap_meter.dpid_pri = 1;
	defaultAct_trap_meter.mcgid = AAL_LPORT_CPU_0;
	defaultAct_trap_meter.mc = 0;	// mc = 0 means "mcgid is LDPID"
#endif
	defaultAct_trap_meter.keep_orig_pkt = 1;
	defaultAct_trap_meter.keep_orig_pkt_vld = 1;
	defaultAct_trap_meter.pol_vld = 1;
	defaultAct_trap_meter.pol_grp_en = 1;
	defaultAct_trap_meter.pol_grp_id = 0;
	defaultAct_trap_meter.pol23_vld = 1;
	defaultAct_trap_meter.pol2_msb_en = 1;
	defaultAct_trap_meter.pol2_id_msb = 0;

	memset(&default_action, 0, sizeof(default_action));
	memcpy(&default_action, &defaultAct_trap_meter, sizeof(defaultAct_trap));
	ASSERT_EQ(aal_hash_default_action_add(&default_action, &default_action_idx, actGrpMsk_default), AAL_E_OK);
	asic_db.flow_defAct_idx[RTK_9607F_FLOW_DEFACT_TYPE_TRAP_WITH_METER] = default_action_idx;

	return ASIC_RET_SUCCESS;
}
uint32_t rtk_9607f_flow_hash_profile_init(uint32 actGrpMsk_flow,uint32 actGrpMsk_type1, rtk_9607f_flow_key_mask_t flowKeyMask)
{

	aal_hash_mask_t hash_mask;
	uint32 mask = 0;
	unsigned int hash_mask_idx;
	uint8 tuple_index;
	char *string_of_hash_profile[] = {
			"RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE",
			"RTK_9607F_FLOW_PROFILE_FLOW_2TUPLE",
			"RTK_9607F_FLOW_PROFILE_FLOW_MC",
#if defined(CONFIG_REALTEK_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA_V8) // for testing Hash double check failed
			"RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE_TCP_FLAG0",
#endif
			"RTK_9607F_FLOW_PROFILE_DEFAULT_DROP",
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
			"RTK_9607F_FLOW_PROFILE_MC_WIFI_TX_AMSDU_OFLD",
#endif
	};
	L3FE_FE_L3E_HS_PROFILE0_OTHER_t profile0_other_config;

	/* hash profile initial */
	/******************************************************
	* profile 0: path345 pattern mask (5 tuple): use 1 hash tuple
	******************************************************/
	// profile basic init: default action "TRAP", tuple number clear to 0, hash key algorithm selection
	aal_hash_profile_init(RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE, asic_db.flow_defAct_idx[RTK_9607F_FLOW_DEFACT_TYPE_TRAP], asic_db.flow_defAct_idx[RTK_9607F_FLOW_DEFACT_TYPE_TRAP], actGrpMsk_flow, actGrpMsk_type1);
	// profile tuple add: 5 tuple
	aal_hash_mask_unmask(&hash_mask);
	hash_mask.orig_lspid = mask;	// force enabled to support unknown sa trap by main hash
	hash_mask.mdata = (flowKeyMask.p345_stream_id)?0xFFFFFFFFFFFFFF00:0xFFFFFFFFFFFFFFFF; // stream ID (PON rx) or wifi dev idx (wifi rx) as flow key
#if defined(CONFIG_REALTEK_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA_V8)
	// To compatible testcases. 5 tuple flow not care SMAC/DMAC.
#else
	hash_mask.mac_sa = mask; // for MC passthrough (5 tuple the same, DMAC may be different)
	hash_mask.mac_da = mask; // for MC passthrough (5 tuple the same, DMAC may be different)
#endif
	hash_mask.top_vid = (flowKeyMask.P345_vlanId) ? mask : (~mask);
	hash_mask.top_8021p = (flowKeyMask.P345_vlanPri) ? mask : (~mask);
	hash_mask.top_dei = (flowKeyMask.pall_vlan_deicfi) ? mask : (~mask);
	hash_mask.top_tpid_enc = mask;
	hash_mask.inner_vid = (flowKeyMask.P345_vlanId) ? mask : (~mask);
	hash_mask.inner_8021p = (flowKeyMask.P345_vlanPri) ? mask : (~mask);
	hash_mask.inner_dei = (flowKeyMask.pall_vlan_deicfi) ? mask : (~mask);
	hash_mask.inner_tpid_enc = mask;
	hash_mask.vlan_cnt = mask;
	hash_mask.pppoe_type = mask;
	hash_mask.pppoe_session_id = mask;
	hash_mask.ip_vld = mask;
	hash_mask.ip_ver = mask;
	hash_mask.ip_protocol = mask;
	hash_mask.ip_dscp = (flowKeyMask.P345_dscp) ? mask : (~mask);
	hash_mask.ip_ecn = (flowKeyMask.P345_ecn) ? mask : (~mask);
	hash_mask.ip_sa = 128;
	hash_mask.ip_da = 128;
	hash_mask.ip_fragment_flag = mask;
	hash_mask.ipv6_hbh = mask;
	hash_mask.ip_options = mask;
	hash_mask.ipv6_rh = mask;
	hash_mask.ipv6_doh = mask;
	hash_mask.l4_sp_exact_range = mask;
	hash_mask.l4_dp_exact_range = mask;
#if defined(CONFIG_REALTEK_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA_V8)
	// compatile to FPGA test case
	hash_mask.tcp_rdp_ctrl = ~(0x2);	//  syn
#endif

	ASSERT_EQ(aal_hash_profile_tuple_add(RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE, TUPLE_TYPE_TYPE0, RTK_9607F_FLOW_TUPPLE_PRI_0, hash_mask, &hash_mask_idx, &tuple_index), AAL_E_OK);
	if(tuple_index != 0)
		ASICINITPRINT("%s: [ERROR] strange tuple_index (%d) of hash profile %d (%s)\n", __func__, RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE, tuple_index, string_of_hash_profile[RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE]);
	asic_db.flow_profile_info[RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE].tuple_info[0].hash_mask_idx = hash_mask_idx;
	ASICINITPRINT("%s: ##### init hash profile %d (%s), profile 0 hash_mask_idx = %d\n", __func__, RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE, string_of_hash_profile[RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE], hash_mask_idx);

	// set tcp flag trap mask to trap TCP rst/fin packets
	profile0_other_config.wrd = rtk_9607f_asic_ne_reg_read(L3FE_FE_L3E_HS_PROFILE0_OTHER);
	profile0_other_config.bf.tcp_flag_mask = 0x5;
	rtk_9607f_asic_ne_reg_write(profile0_other_config.wrd, L3FE_FE_L3E_HS_PROFILE0_OTHER);

	/******************************************************
	* profile 1: path12 pattern mask (2 tuple): use 1 hash tuple
	******************************************************/
	// profile basic init: default action "TRAP", tuple number clear to 0, hash key algorithm selection
	aal_hash_profile_init(RTK_9607F_FLOW_PROFILE_FLOW_2TUPLE, asic_db.flow_defAct_idx[RTK_9607F_FLOW_DEFACT_TYPE_TRAP], asic_db.flow_defAct_idx[RTK_9607F_FLOW_DEFACT_TYPE_TRAP], actGrpMsk_flow, actGrpMsk_type1);
	// profile tuple add: 2 tuple
	aal_hash_mask_unmask(&hash_mask);
	hash_mask.orig_lspid = mask;	// force enabled to support unknown sa trap by main hash
	hash_mask.mdata = (flowKeyMask.p12_stream_id)?0xFFFFFFFFFFFFFF00:0xFFFFFFFFFFFFFFFF; // stream ID (PON rx) or wifi dev idx (wifi rx) as flow key
	hash_mask.mac_sa = mask;
	hash_mask.mac_da = mask;
	hash_mask.top_vid = (flowKeyMask.P12_vlanId) ? mask : (~mask);
	hash_mask.top_8021p = (flowKeyMask.P12_vlanPri) ? mask : (~mask);
	hash_mask.top_dei = (flowKeyMask.pall_vlan_deicfi) ? mask : (~mask);
	hash_mask.top_tpid_enc = mask;
	hash_mask.inner_vid = (flowKeyMask.P12_vlanId) ? mask : (~mask);
	hash_mask.inner_8021p = (flowKeyMask.P12_vlanPri) ? mask : (~mask);
	hash_mask.inner_dei = (flowKeyMask.pall_vlan_deicfi) ? mask : (~mask);
	hash_mask.inner_tpid_enc = mask;
	hash_mask.vlan_cnt = mask;
	hash_mask.ethertype_enc = (flowKeyMask.p12_ethtype) ? mask : (~mask);
	hash_mask.ip_dscp = (flowKeyMask.P12_dscp) ? mask : (~mask);
	hash_mask.ip_ecn = (flowKeyMask.P12_ecn) ? mask : (~mask);
	/*
		pure L2: modify_vlan_only set to 1 (to prevent non IP PPPoE PE issue)
		IP: modify_vlan_only set to 0. (If modify_vlan_only is 1, ip_dscp can not be modified)
	*/
	hash_mask.ip_vld = mask;
	//hash_mask.l4_sp_exact_range = mask; /* exactly match */
	//hash_mask.l4_dp_exact_range = mask; /* exactly match */
	ASSERT_EQ(aal_hash_profile_tuple_add(RTK_9607F_FLOW_PROFILE_FLOW_2TUPLE, TUPLE_TYPE_TYPE0, RTK_9607F_FLOW_TUPPLE_PRI_0, hash_mask, &hash_mask_idx, &tuple_index), AAL_E_OK);
	if(tuple_index != 0)
		ASICINITPRINT("%s: [ERROR] strange tuple_index (%d) of hash profile %d (%s)\n", __func__, RTK_9607F_FLOW_PROFILE_FLOW_2TUPLE, tuple_index, string_of_hash_profile[RTK_9607F_FLOW_PROFILE_FLOW_2TUPLE]);
	asic_db.flow_profile_info[RTK_9607F_FLOW_PROFILE_FLOW_2TUPLE].tuple_info[0].hash_mask_idx = hash_mask_idx;
	ASICINITPRINT("%s: ##### init hash profile %d (%s), profile 0 hash_mask_idx = %d\n", __func__, RTK_9607F_FLOW_PROFILE_FLOW_2TUPLE, string_of_hash_profile[RTK_9607F_FLOW_PROFILE_FLOW_2TUPLE], hash_mask_idx);

	/******************************************************
	* profile 2: MC: use 2 hash tuple
	******************************************************/
	// profile basic init: default action "TRAP", tuple number clear to 0, hash key algorithm selection
	aal_hash_profile_init(RTK_9607F_FLOW_PROFILE_FLOW_MC, asic_db.flow_defAct_idx[RTK_9607F_FLOW_DEFACT_TYPE_DROP], asic_db.flow_defAct_idx[RTK_9607F_FLOW_DEFACT_TYPE_TRAP], actGrpMsk_flow, actGrpMsk_type1);
	// profile tuple add: 2 tuple
	// tuple 0
	aal_hash_mask_unmask(&hash_mask);
	hash_mask.ip_da = 128;
	hash_mask.ip_ver = mask;
	hash_mask.ip_vld = mask;
	// no possible to expect vlan id of mc data flow, so we disable vlan pattern in mc hash tuple.
	hash_mask.top_vid = (~mask);
	ASSERT_EQ(aal_hash_profile_tuple_add(RTK_9607F_FLOW_PROFILE_FLOW_MC, TUPLE_TYPE_TYPE0, RTK_9607F_FLOW_TUPPLE_PRI_0, hash_mask, &hash_mask_idx, &tuple_index), AAL_E_OK);
	if(tuple_index != 0)
		ASICINITPRINT("%s: [ERROR] strange tuple_index (%d) of hash profile %d (%s)\n", __func__, RTK_9607F_FLOW_PROFILE_FLOW_MC, tuple_index, string_of_hash_profile[RTK_9607F_FLOW_PROFILE_FLOW_MC]);
	asic_db.flow_profile_info[RTK_9607F_FLOW_PROFILE_FLOW_MC].tuple_info[MC_HASH_GRP_TRAP_TUPLE].hash_mask_idx = hash_mask_idx;
	ASICINITPRINT("%s: ##### init hash profile %d (%s), profile 0 hash_mask_idx = %d\n", __func__, RTK_9607F_FLOW_PROFILE_FLOW_MC, string_of_hash_profile[RTK_9607F_FLOW_PROFILE_FLOW_MC], hash_mask_idx);


	aal_hash_mask_unmask(&hash_mask);
	hash_mask.ip_da = 128;
	hash_mask.ip_sa =128;
	hash_mask.ip_ver = 0;
	hash_mask.ip_vld = 0;
	hash_mask.vlan_cnt = mask;
#if defined(CONFIG_REALTEK_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA_V8)
	// To compatible testcases.
#else
	hash_mask.top_dei = (flowKeyMask.pall_vlan_deicfi) ? mask : (~mask);
	hash_mask.top_tpid_enc = mask;
#endif
	hash_mask.top_vid = (flowKeyMask.P345_vlanId) ? mask : (~mask);
	hash_mask.top_8021p = (flowKeyMask.P345_vlanPri) ? mask : (~mask);
	hash_mask.inner_vid = (flowKeyMask.P345_vlanId) ? mask : (~mask);
	hash_mask.inner_8021p = (flowKeyMask.P345_vlanPri) ? mask : (~mask);
#if defined(CONFIG_REALTEK_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA_V8)
	// compatile to FPGA test case
#else
	hash_mask.inner_dei = (flowKeyMask.pall_vlan_deicfi) ? mask : (~mask);
	hash_mask.inner_tpid_enc = mask;
	hash_mask.ip_dscp = (flowKeyMask.P345_dscp) ? mask : (~mask);
	hash_mask.ip_ecn = (flowKeyMask.P345_ecn) ? mask : (~mask);
	hash_mask.mdata = (flowKeyMask.p345_stream_id)?0xFFFFFFFFFFFFFF00:0xFFFFFFFFFFFFFFFF; // stream ID (PON rx) or wifi dev idx (wifi rx) as flow key
#endif
	ASSERT_EQ(aal_hash_profile_tuple_add(RTK_9607F_FLOW_PROFILE_FLOW_MC, TUPLE_TYPE_TYPE0, RTK_9607F_FLOW_TUPPLE_PRI_1, hash_mask, &hash_mask_idx, &tuple_index), AAL_E_OK);
	if(tuple_index != 1)
		ASICINITPRINT("%s: [ERROR] strange tuple_index (%d) of hash profile %d (%s)\n", __func__, RTK_9607F_FLOW_PROFILE_FLOW_MC, tuple_index, string_of_hash_profile[RTK_9607F_FLOW_PROFILE_FLOW_MC]);
	asic_db.flow_profile_info[RTK_9607F_FLOW_PROFILE_FLOW_MC].tuple_info[MC_HASH_FLOW_FORWARD_TUPLE].hash_mask_idx = hash_mask_idx;
	ASICINITPRINT("%s: ##### init hash profile %d (%s), profile 1 hash_mask_idx = %d\n", __func__, RTK_9607F_FLOW_PROFILE_FLOW_MC, string_of_hash_profile[RTK_9607F_FLOW_PROFILE_FLOW_MC], hash_mask_idx);

#if defined(CONFIG_REALTEK_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA_V8) // for testing Hash double check failed
	/******************************************************
	* profile 3: path345 pattern mask (5 tuple) but no care TCP flag (purpose: SW no need to recalculate CRC of TCP RST/FIN): use 1 hash tuple
	******************************************************/
	// profile basic init: default action "TRAP", tuple number clear to 0, hash key algorithm selection
	aal_hash_profile_init(RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE_TCP_FLAG0, asic_db.flow_defAct_idx[RTK_9607F_FLOW_DEFACT_TYPE_TRAP], asic_db.flow_defAct_idx[RTK_9607F_FLOW_DEFACT_TYPE_TRAP], actGrpMsk_flow, actGrpMsk_type1);
	// profile tuple add: 5 tuple but no care TCP flag
	aal_hash_mask_unmask(&hash_mask);
	hash_mask.orig_lspid = mask;	// force enabled to support unknown sa trap by main hash
	hash_mask.mdata = (flowKeyMask.p345_stream_id)?0xFFFFFFFFFFFFFF00:0xFFFFFFFFFFFFFFFF; // stream ID (PON rx) or wifi dev idx (wifi rx) as flow key
#if defined(CONFIG_REALTEK_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA_V8)
	// To compatible testcases. 5 tuple flow not care SMAC/DMAC.
#else
	hash_mask.mac_sa = mask; // for MC passthrough (5 tuple the same, DMAC may be different)
	hash_mask.mac_da = mask; // for MC passthrough (5 tuple the same, DMAC may be different)
#endif
	hash_mask.top_vid = (flowKeyMask.P345_vlanId) ? mask : (~mask);
	hash_mask.top_8021p = (flowKeyMask.P345_vlanPri) ? mask : (~mask);
	hash_mask.top_dei = (flowKeyMask.pall_vlan_deicfi) ? mask : (~mask);
	hash_mask.top_tpid_enc = mask;
	hash_mask.inner_vid = (flowKeyMask.P345_vlanId) ? mask : (~mask);
	hash_mask.inner_8021p = (flowKeyMask.P345_vlanPri) ? mask : (~mask);
	hash_mask.inner_dei = (flowKeyMask.pall_vlan_deicfi) ? mask : (~mask);
	hash_mask.inner_tpid_enc = mask;
	hash_mask.vlan_cnt = mask;
	hash_mask.pppoe_type = mask;
	hash_mask.pppoe_session_id = mask;
	hash_mask.ip_vld = mask;
	hash_mask.ip_ver = mask;
	hash_mask.ip_protocol = mask;
	hash_mask.ip_dscp = (flowKeyMask.P345_dscp) ? mask : (~mask);
	hash_mask.ip_ecn = (flowKeyMask.P345_ecn) ? mask : (~mask);
	hash_mask.ip_sa = 128;
	hash_mask.ip_da = 128;
	hash_mask.ip_fragment_flag = mask;
	hash_mask.ipv6_hbh = mask;
	hash_mask.ip_options = mask;
	hash_mask.ipv6_rh = mask;
	hash_mask.ipv6_doh = mask;
	hash_mask.l4_sp_exact_range = mask;
	hash_mask.l4_dp_exact_range = mask;

	ASSERT_EQ(aal_hash_profile_tuple_add(RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE_TCP_FLAG0, TUPLE_TYPE_TYPE0, RTK_9607F_FLOW_TUPPLE_PRI_0, hash_mask, &hash_mask_idx, &tuple_index), AAL_E_OK);
	if(tuple_index != 0)
		ASICINITPRINT("%s: [ERROR] strange tuple_index (%d) of hash profile %d (%s)\n", __func__, RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE_TCP_FLAG0, tuple_index, string_of_hash_profile[RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE_TCP_FLAG0]);
	asic_db.flow_profile_info[RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE_TCP_FLAG0].tuple_info[0].hash_mask_idx = hash_mask_idx;
	ASICINITPRINT("%s: ##### init hash profile %d (%s), profile 0 hash_mask_idx = %d\n", __func__, RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE_TCP_FLAG0, string_of_hash_profile[RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE_TCP_FLAG0], hash_mask_idx);
#endif

	/******************************************************
	* profile 3: default drop (purpose: for ACL DROP action. ACL mib and drop action take effect at the same time)
	******************************************************/
	// profile basic init: default action "DROP", tuple number clear to 0, hash key algorithm selection
	aal_hash_profile_init(RTK_9607F_FLOW_PROFILE_DEFAULT_DROP, asic_db.flow_defAct_idx[RTK_9607F_FLOW_DEFACT_TYPE_DROP], asic_db.flow_defAct_idx[RTK_9607F_FLOW_DEFACT_TYPE_DROP], actGrpMsk_flow, actGrpMsk_type1);
	// profile tuple add: use 5 tuple but no care TCP flag
	hash_mask.mdata = 0; // flow care mdata 64 bits to make sure CRC16/CRC32 be different with normal flow. (prevent hash double check fail, this will make default action policer drop not work.)
	ASSERT_EQ(aal_hash_profile_tuple_add(RTK_9607F_FLOW_PROFILE_DEFAULT_DROP, TUPLE_TYPE_TYPE0, RTK_9607F_FLOW_TUPPLE_PRI_0, hash_mask, &hash_mask_idx, &tuple_index), AAL_E_OK);
	if(tuple_index != 0)
		ASICINITPRINT("%s: [ERROR] strange tuple_index (%d) of hash profile %d (%s)\n", __func__, RTK_9607F_FLOW_PROFILE_DEFAULT_DROP, tuple_index, string_of_hash_profile[RTK_9607F_FLOW_PROFILE_DEFAULT_DROP]);
	asic_db.flow_profile_info[RTK_9607F_FLOW_PROFILE_DEFAULT_DROP].tuple_info[0].hash_mask_idx = hash_mask_idx;
	ASICINITPRINT("%s: ##### init hash profile %d (%s), profile 0 hash_mask_idx = %d\n", __func__, RTK_9607F_FLOW_PROFILE_DEFAULT_DROP, string_of_hash_profile[RTK_9607F_FLOW_PROFILE_DEFAULT_DROP], hash_mask_idx);

#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
	/******************************************************
	* profile 4: DMAC update for MC wifi TX amsdu offload: use 1 hash tuple
	******************************************************/
	// profile basic init: default action "TRAP", tuple number clear to 0, hash key algorithm selection
	aal_hash_profile_init(RTK_9607F_FLOW_PROFILE_MC_WIFI_TX_AMSDU_OFLD, asic_db.flow_defAct_idx[RTK_9607F_FLOW_DEFACT_TYPE_DROP], asic_db.flow_defAct_idx[RTK_9607F_FLOW_DEFACT_TYPE_DROP], actGrpMsk_flow, actGrpMsk_type1);
	// profile tuple add: 5 tuple but no care TCP flag
	aal_hash_mask_unmask(&hash_mask);
	hash_mask.mdata = 0xFFFFFFFFFFFFFF00; //wifi mac_idx (wifi tx) as flow key
	hash_mask.mac_da = mask; // for MC passthrough (5 tuple the same, DMAC may be different)
	ASSERT_EQ(aal_hash_profile_tuple_add(RTK_9607F_FLOW_PROFILE_MC_WIFI_TX_AMSDU_OFLD, TUPLE_TYPE_TYPE0, RTK_9607F_FLOW_TUPPLE_PRI_0, hash_mask, &hash_mask_idx, &tuple_index), AAL_E_OK);
	if(tuple_index != 0)
		ASICINITPRINT("%s: [ERROR] strange tuple_index (%d) of hash profile %d (%s)\n", __func__, RTK_9607F_FLOW_PROFILE_MC_WIFI_TX_AMSDU_OFLD, tuple_index, string_of_hash_profile[RTK_9607F_FLOW_PROFILE_MC_WIFI_TX_AMSDU_OFLD]);
	asic_db.flow_profile_info[RTK_9607F_FLOW_PROFILE_MC_WIFI_TX_AMSDU_OFLD].tuple_info[0].hash_mask_idx = hash_mask_idx;
	ASICINITPRINT("%s: ##### init hash profile %d (%s), profile 0 hash_mask_idx = %d\n", __func__, RTK_9607F_FLOW_PROFILE_MC_WIFI_TX_AMSDU_OFLD, string_of_hash_profile[RTK_9607F_FLOW_PROFILE_MC_WIFI_TX_AMSDU_OFLD], hash_mask_idx);
#endif
	return ASIC_RET_SUCCESS;
}
uint32_t rtk_9607f_flow_init(rtk_9607f_flow_key_mask_t flowKeyMask)
{
	
	uint32 actGrpMsk_flow, actGrpMsk_type1, actGrpMsk_default;
	uint32 hashTbl_size, hashAction_width, cacheTbl_size, cacheAction_width, hashBucket_size, defAction_width;
	uint32 type0_fibBits, type1_fibBits, def_fibBits;
	hash_cache_fib_mode_select_t cache_fib_mode;
	int ret = 0;
	L3FE_FE_L3E_HS_INV_MAC_t hash_invalid_dmac_act;
	L3FE_FE_L3E_HS_OTEHR_FAIL_t other_fail_act; // tcp flag mask action

	
	/* flow DMAC lookup miss TRAP */
	hash_invalid_dmac_act.wrd = rtk_9607f_asic_ne_reg_read(L3FE_FE_L3E_HS_INV_MAC);
	hash_invalid_dmac_act.bf.keep_ori_vld = TRUE;
	hash_invalid_dmac_act.bf.keep_ori = TRUE;
	hash_invalid_dmac_act.bf.deepq_vld = TRUE;
	hash_invalid_dmac_act.bf.deepq = FALSE;
	hash_invalid_dmac_act.bf.ldpid = 0x10;
	rtk_9607f_asic_ne_reg_write(hash_invalid_dmac_act.wrd, L3FE_FE_L3E_HS_INV_MAC);

	/* get action group mask */
	if(aal_hash_actionGrpBitmask_get(HASH_ACTGROUP_TYPE_FLOW, &actGrpMsk_flow))
	{
		ASICINITPRINT("%s: [ERROR] get action group mask failed (flow)\n", __func__);
		return ASIC_RET_FAIL;
	}

	if(aal_hash_actionGrpBitmask_get(HASH_ACTGROUP_TYPE_HASHTYPE1, &actGrpMsk_type1))
	{
		ASICINITPRINT("%s: [ERROR] get action group mask failed (hash type1)\n", __func__);
		return ASIC_RET_FAIL;
	}

	if(aal_hash_actionGrpBitmask_get(HASH_ACTGROUP_TYPE_DEFAULT, &actGrpMsk_default))
	{
		ASICINITPRINT("%s: [ERROR] get action group mask failed (default)\n", __func__);
		return ASIC_RET_FAIL;
	}

	/* validate hash fib length */
	if(aal_hash_hash_table_size_get(&hashTbl_size, &hashBucket_size, &hashAction_width))
	{
		ASICINITPRINT("%s: [ERROR] get hash table size failed\n", __func__);
		return ASIC_RET_FAIL;
	}
	if(aal_hash_hash_dafAction_size_get(&defAction_width))
	{
		ASICINITPRINT("%s: [ERROR] get hash default action size failed\n", __func__);
		return ASIC_RET_FAIL;
	}
	if(aal_hash_cache_table_size_get(&cacheTbl_size, &cacheAction_width, &cache_fib_mode))
	{
		ASICINITPRINT("%s: [ERROR] get hash cache action size failed\n", __func__);
		return ASIC_RET_FAIL;
	}

	aal_hash_actionGrpBitmask_length_get(actGrpMsk_flow, &type0_fibBits);
	aal_hash_actionGrpBitmask_length_get(actGrpMsk_type1, &type1_fibBits);
	aal_hash_actionGrpBitmask_length_get(actGrpMsk_default, &def_fibBits);

	ASICINITPRINT("%s: ##### hash fib bits width: flow(type-0) act_mask 0x%x (%d bits), type-1 act_mask 0x%x (%d bits), default act_mask 0x%x (%d bits)\n", __func__, actGrpMsk_flow, type0_fibBits, actGrpMsk_type1, type1_fibBits, actGrpMsk_default, def_fibBits);
	if((type0_fibBits > hashAction_width) || (type1_fibBits > hashAction_width) || (type0_fibBits > cacheAction_width) || (type1_fibBits > cacheAction_width) || (def_fibBits > defAction_width))
	{
		ASICINITPRINT("%s: [ERROR] fib length over fib width configuration (hashAction_width: %d, cacheAction_width: %d, defAction_width: %d)\n", __func__, hashAction_width, cacheAction_width, defAction_width);
		return ASIC_RET_FAIL;
	}

	/* default action setup */
	ASSERT_EQ(rtk_9607f_flow_default_action_init(actGrpMsk_default), ASIC_RET_SUCCESS);

	/*set tcp flag trap mask action*/
	other_fail_act.wrd = rtk_9607f_asic_ne_reg_read(L3FE_FE_L3E_HS_OTEHR_FAIL);
	other_fail_act.bf.tcp_flag_act = (0x10 & 0x3F) << 0;	//[5:0]: ldpid
	other_fail_act.bf.tcp_flag_act |= (0 & 0x1) << 6;		//[6]: deepq
	other_fail_act.bf.tcp_flag_act |= (1 & 0x1) << 7;		//[7]: deepq_vld
	other_fail_act.bf.tcp_flag_act |= (1 & 0x1) << 8;		//[8]: keep_org_pkt
	other_fail_act.bf.tcp_flag_act |= (1 & 0x1) << 9;		//[9]: keep_org_pkt_vld
	rtk_9607f_asic_ne_reg_write(other_fail_act.wrd, L3FE_FE_L3E_HS_OTEHR_FAIL);

	ret = rtk_9607f_flow_hash_profile_init(actGrpMsk_flow, actGrpMsk_type1, flowKeyMask);
	if(ret != ASIC_RET_SUCCESS)
		return ASIC_RET_FAIL;

	return ASIC_RET_SUCCESS;
}


/*====================
	rtk_9607f_policer_init(): Init L2 and L3 policer here.
====================*/
int32 rtk_9607f_policer_init(void)
{

	ca_uint16_t flow_id;
	aal_l3_te_policer_cfg_msk_t policer_cfg_msk;
	aal_l3_te_policer_cfg_t policer_cfg;
	aal_l3_te_policer_profile_msk_t policer_profile_msk;
	aal_l3_te_policer_profile_t policer_profile;
	aal_l3_te_glb_cfg_msk_t glb_cfg_msk;
	aal_l3_te_glb_cfg_t glb_cfg;

	TE_POL_FLOW_CFG_t flow_cfg;
	L3FE_GLB_TE_OPTION_t te_option;

	/*Enable 3 policrers*/
	memset(&glb_cfg_msk, 0, sizeof(glb_cfg_msk));
	memset(&glb_cfg, 0, sizeof(glb_cfg));
	glb_cfg_msk.s.sel3flows = 1;
	glb_cfg.sel3flows = 1;
	ASSERT_EQ(aal_l3_te_glb_cfg_set(RTK_ASIC_DEVID, glb_cfg_msk, &glb_cfg), CA_E_OK);

	/*
	TE_POL_AGRFLOW_CTRL0: policer2[0-31]
	TE_POL_AGRFLOW_CTRL0: policer2[32-63]
	TE_POL_FLOW3_CTRL0: policer3[0-31]
	TE_POL_FLOW3_CTRL1: policer3[32-63]
	*/
	flow_cfg.wrd = rtk_9607f_asic_ne_reg_read(TE_POL_FLOW_CFG);
	flow_cfg.bf.watermark_flow2 = 0x1f; //31
	flow_cfg.bf.watermark_flow3 = 0x1f; //31
	rtk_9607f_asic_ne_reg_write(flow_cfg.wrd, TE_POL_FLOW_CFG);

	/*CRC length is not included in packet length. extra +4 to take CRC into count for TE.*/
	te_option.wrd = rtk_9607f_asic_ne_reg_read(L3FE_GLB_TE_OPTION);
	te_option.bf.bits |= (1<<8); //l3fe_l3te_egr_pktle = original +4 (4 for CRC length)
	te_option.bf.bits |= (1<<9); //l3fe_l3te_ingr_pktlen = original +4 (4 for CRC length)
	rtk_9607f_asic_ne_reg_write(te_option.wrd, L3FE_GLB_TE_OPTION);

	/*init L3 policer here*/

	memset(&policer_cfg_msk, 0, sizeof(policer_cfg_msk));
	memset(&policer_cfg, 0, sizeof(policer_cfg));
	policer_cfg_msk.s.type = 1;
	policer_cfg.type = CA_AAL_L3_POLICER_TYPE_NONE;
	policer_cfg_msk.s.pol_update_mode = 1;
	policer_cfg.pol_update_mode = CA_AAL_L3_POLICER_UPDATE_MODE_BYTE;

	memset(&policer_profile_msk, 0, sizeof(policer_profile_msk));
	memset(&policer_profile, 0, sizeof(policer_profile));
	policer_profile_msk.s.cir_k = 1;
	policer_profile.cir_k = 0x3ff;
	policer_profile_msk.s.cir_m = 1;
	policer_profile.cir_m = 0xffff;
	policer_profile_msk.s.pir_k = 1;
	policer_profile.pir_k = 0x3ff;
	policer_profile_msk.s.pir_m = 1;
	policer_profile.pir_m = 0xffff;
	policer_profile_msk.s.cbs = 1;
	policer_profile.cbs = 0xfff;
	policer_profile_msk.s.pbs = 1;
	policer_profile.pbs = 0xfff;

	for(flow_id = 0 ; flow_id <= CA_AAL_L3_MAX_FLOW_ID ; flow_id++)
	{
		ASSERT_EQ(aal_l3_te_policer_flow_cfg_set(RTK_ASIC_DEVID, flow_id, policer_cfg_msk, &policer_cfg), CA_E_OK);
		ASSERT_EQ(aal_l3_te_policer_flow_profile_set(RTK_ASIC_DEVID, flow_id, policer_profile_msk, &policer_profile), CA_E_OK);
	}

	for(flow_id = 0 ; flow_id <= CA_AAL_L3_MAX_FLOW2_ID ; flow_id++)
	{
		ASSERT_EQ(aal_l3_te_policer_agr_flow_cfg_set(RTK_ASIC_DEVID, flow_id, policer_cfg_msk, &policer_cfg), CA_E_OK);
		ASSERT_EQ(aal_l3_te_policer_agr_flow_profile_set(RTK_ASIC_DEVID, flow_id, policer_profile_msk, &policer_profile), CA_E_OK);
	}

	for(flow_id = 0 ; flow_id <= CA_AAL_L3_MAX_FLOW3_ID ; flow_id++)
	{
		ASSERT_EQ(aal_l3_te_policer_flow3_cfg_set(RTK_ASIC_DEVID, flow_id, policer_cfg_msk, &policer_cfg), CA_E_OK);
		ASSERT_EQ(aal_l3_te_policer_flow3_profile_set(RTK_ASIC_DEVID, flow_id, policer_profile_msk, &policer_profile), CA_E_OK);
	}

	return CA_E_OK;
}

/*
 * overwrite QM voq profile selection in aal_l3_te_cb_init()
 */
uint32_t rtk_9607f_qm_init(void)
{
#if 0 // init by aal
	uint8 voq_id;
	uint8 profile_sel = 0;

	for(voq_id=0; voq_id<CA_AAL_MAX_VOQ_ID; voq_id++){
		if(voq_id < 16){
			// 0x10,0x11 cpu port voq
                    profile_sel = 2;
		}else if(voq_id > (CA_AAL_MAX_VOQ_ID>>1)){
			// phy port voq =>  keep ca initialized setting.
                    //profile_sel = 3;
                    continue;
		}else{
			// 0x12~0x17 wifi cpu port voq
                    profile_sel = 0;
		}
            aal_l3_te_cb_voq_profile_sel_set(RTK_ASIC_DEVID, voq_id, profile_sel);
	}
#endif
	return CA_E_OK;
}

int rtk_9607f_acl_add(rtk_9607f_asic_rsv_acl_t type)
{
	uint32_t ca_aal_rslt_idx=0;
	aal_l2_cf_rule_t ruleCfg;
	l3_cls_rule_t ca_aal_rule;
	uint32 index;
	uint8 default_device_id = 0;
	uint8 aal_idx_offset=0;
	char aal_idx_buf[64];

	switch(type){
		case ASIC_RSV_ACL_RESET_L2_L3_CLS:
			{
				for(index = 0; index <= L2_CLS_KEY_TBL_ENTRY_MAX; index++){
					ca_aal_rslt_idx = index<<1;
					aal_l2_cls_del(RTK_ASIC_DEVID, (ca_aal_rslt_idx|0));
					aal_l2_cls_del(RTK_ASIC_DEVID, (ca_aal_rslt_idx|1));	//sub entry
				}
				aal_table_flush(AAL_TABLE_L3_CLS_KEY);
				aal_table_flush(AAL_TABLE_L3_CLS_FIB);
			}
			break;
		case ASIC_RSV_ACL_L2_EGRESS_FORWARD_L3FE:	//ref: dal_rt_rtl9607f_cls_init
			{
				//FPGA only support 16 entry due to limit resource
				for(index=0;index<16/*__AAL_L2_CF_MAX_RULE_ID*/;index++)
					aal_l2_cf_rule_delete(default_device_id, index);

				index = 0;

				memset(&ruleCfg,0,sizeof(aal_l2_cf_rule_t));
				ruleCfg.key.care_bits.valid = 1;
				ruleCfg.key.data_bits.valid = 1;
				ruleCfg.key.care_bits.lspid = 0x3f;
				ruleCfg.key.data_bits.lspid = 7;
				ruleCfg.action.fwd_act = AAL_L2_CF_ACTION_FWD_ACT_LDPID;
				ruleCfg.action.cf_fwd_id = 0x18;
				aal_l2_cf_rule_add(default_device_id, index ,&ruleCfg);
				index++;

				memset(&ruleCfg,0,sizeof(aal_l2_cf_rule_t));
				ruleCfg.key.care_bits.valid = 1;
				ruleCfg.key.data_bits.valid = 1;
				ruleCfg.action.fwd_act = AAL_L2_CF_ACTION_FWD_ACT_LDPID;
				ruleCfg.action.cf_fwd_id = 0x19;
				aal_l2_cf_rule_add(default_device_id, index ,&ruleCfg);
				index++;

				ASICINITPRINT("%s: ##### L2 EGR CLS initialization success.#####\n", __func__);
			}
			break;
		case ASIC_RSV_ACL_GENERIC_INTF_HASH_PROFILE_DECISION:
			{
#if !defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)	
				enum{
					CLS_T2CTRL_MAINHASH_PROFILE_WAN=RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE,
					CLS_T2CTRL_MAINHASH_PROFILE_LAN=RTK_9607F_FLOW_PROFILE_FLOW_5TUPLE,
					CLS_T2CTRL_MAINHASH_PROFILE_MC=RTK_9607F_FLOW_PROFILE_FLOW_MC,
					CLS_T2CTRL_MAINHASH_PROFILE_2TUPLE_BRIDGE=RTK_9607F_FLOW_PROFILE_FLOW_2TUPLE,
					CA_L3_AAL_KEY_MSK_ENABLE = 0,
					RTK_FC_MAC_PORT_CPU = 0x10,
					RTK_FC_MAC_PORT_CPU1 = 0x11,
				};
#endif

				memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));

				/*
				 * Default rules of Wan/Lan profile
				 */

				for(index = CL_RUL_PROFILE_WAN; index <= CL_RUL_PROFILE_LAN; index++) {
					aal_idx_offset = 0;

					/** mc rule **/
					ca_aal_rule.pri = 9 /*RTK_CA_CLS_PRIORITY_L3_9*/;
					ca_aal_rule.rslt_type = CL_RSLT_TYPE_0;

					aal_l3_cls_key_clean(&ca_aal_rule.key);
					memset(&ca_aal_rule.action, 0, sizeof(cls_action_t));

					ca_aal_rule.key.msk_mac_da_ip_mc = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.mac_da_ip_mc = 1;	//((MAC DA[47:24] == 01-00-5E) && (etherType == IPV4)) | ((MAC DA[47:32] == 33-33) && (etherType == IPV6))
					ca_aal_rule.key.msk_ip_vld = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.ip_vld = 1;

					//_rtk_fc_g3_caAalClsAssignHashProfileAction(&ca_aal_rule, ca_aal_profile_idx, FALSE, 0);
					ca_aal_rule.action.dpid_vld = 1;
					ca_aal_rule.action.dpid_pri = 1;
					ca_aal_rule.action.permit = 1;
					ca_aal_rule.action.mc = 0; //1:mcgid=mcgid;0:mcgid=ldpid
					if(index==CL_RUL_PROFILE_WAN)
						ca_aal_rule.action.mcgid = RTK_FC_MAC_PORT_CPU1;
					else
						ca_aal_rule.action.mcgid = RTK_FC_MAC_PORT_CPU;
					ca_aal_rule.action.t2_ctrl_vld = 1;
					ca_aal_rule.action.t2_ctrl = CLS_T2CTRL_MAINHASH_PROFILE_MC;
					ca_aal_rule.action.stage2_ctrl_vld = 1; // Merge type-0 and type-1 result of Hash Engine
					ca_aal_rule.action.stage2_ctrl = 0;

					ASSERT_EQ(aal_l3_cls_add(RTK_ASIC_DEVID, index, &ca_aal_rule, &ca_aal_rslt_idx), CA_E_OK);
					aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%d-%d,", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));

					/** mc rule **/
					ca_aal_rule.pri = 9 /*RTK_CA_CLS_PRIORITY_L3_9*/;	//reuse same entry with ca mc rule
					ca_aal_rule.rslt_type = CL_RSLT_TYPE_0;

					aal_l3_cls_key_clean(&ca_aal_rule.key);
					memset(&ca_aal_rule.action, 0, sizeof(cls_action_t));
					ca_aal_rule.key.msk_mac_da_an_hit = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.mac_da_an_hit = 1;

					ca_aal_rule.key.msk_ip_vld = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.ip_vld = 1;
					ca_aal_rule.key.msk_ip_da_mc = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.ip_da_mc = 1;	//IP DA is in MC IP range,  IPv4: ip_da[31:28] == 4'b1110; IPv6: ip_da[127:120] == 8'hff

					//_rtk_fc_g3_caAalClsAssignHashProfileAction(&ca_aal_rule, ca_aal_profile_idx, FALSE, 0);
					ca_aal_rule.action.dpid_vld = 1;
					ca_aal_rule.action.dpid_pri = 1;
					ca_aal_rule.action.permit = 1;
					ca_aal_rule.action.mc = 0; //1:mcgid=mcgid;0:mcgid=ldpid
					if(index==CL_RUL_PROFILE_WAN)
						ca_aal_rule.action.mcgid = RTK_FC_MAC_PORT_CPU1;
					else
						ca_aal_rule.action.mcgid = RTK_FC_MAC_PORT_CPU;
					ca_aal_rule.action.t2_ctrl_vld = 1;
					ca_aal_rule.action.t2_ctrl = CLS_T2CTRL_MAINHASH_PROFILE_MC;
					ca_aal_rule.action.stage2_ctrl_vld = 1; // Merge type-0 and type-1 result of Hash Engine
					ca_aal_rule.action.stage2_ctrl = 0;

					ASSERT_EQ(aal_l3_cls_add(RTK_ASIC_DEVID, index, &ca_aal_rule, &ca_aal_rslt_idx), CA_E_OK);
					aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%d-%d,", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));

					{
						/** with L4 header (UC5 tuple) **/
						ca_aal_rule.pri = 1 /*RTK_CA_CLS_PRIORITY_L3_1*/;
						ca_aal_rule.rslt_type = CL_RSLT_TYPE_0;

						aal_l3_cls_key_clean(&ca_aal_rule.key);
						memset(&ca_aal_rule.action, 0, sizeof(cls_action_t));

						ca_aal_rule.key.msk_ip_da_mc = CA_L3_AAL_KEY_MSK_ENABLE;
						ca_aal_rule.key.ip_da_mc = 0;//non multicast
						ca_aal_rule.key.msk_l4_vld = CA_L3_AAL_KEY_MSK_ENABLE;	// tcp udp rdp
						ca_aal_rule.key.l4_vld = 1;

						//_rtk_fc_g3_caAalClsAssignHashProfileAction(&ca_aal_rule, ca_aal_profile_idx, care_lspid, i);
						ca_aal_rule.action.dpid_vld = 1;
						ca_aal_rule.action.dpid_pri = 1;
						ca_aal_rule.action.permit = 1;
						ca_aal_rule.action.mc = 0; //1:mcgid=mcgid;0:mcgid=ldpid
						if(index==CL_RUL_PROFILE_WAN)
							ca_aal_rule.action.mcgid = RTK_FC_MAC_PORT_CPU1;
						else
							ca_aal_rule.action.mcgid = RTK_FC_MAC_PORT_CPU;
						ca_aal_rule.action.t2_ctrl_vld = 1;
						if(index==CL_RUL_PROFILE_WAN)
							ca_aal_rule.action.t2_ctrl = CLS_T2CTRL_MAINHASH_PROFILE_WAN;
						else
							ca_aal_rule.action.t2_ctrl = CLS_T2CTRL_MAINHASH_PROFILE_LAN; 
						ca_aal_rule.action.stage2_ctrl_vld = 1; // Merge type-0 and type-1 result of Hash Engine
						ca_aal_rule.action.stage2_ctrl = 0;

						ASSERT_EQ(aal_l3_cls_add(RTK_ASIC_DEVID, index, &ca_aal_rule, &ca_aal_rslt_idx), CA_E_OK);
						aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%d-%d,", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));

						/** without L4 header (UC2 tuple) **/
						ca_aal_rule.pri = 0 /*RTK_CA_CLS_PRIORITY_L3_0*/;
						ca_aal_rule.rslt_type = CL_RSLT_TYPE_0;

						aal_l3_cls_key_clean(&ca_aal_rule.key);
						memset(&ca_aal_rule.action, 0, sizeof(cls_action_t));

						//_rtk_fc_g3_caAalClsAssignHashProfileAction(&ca_aal_rule, ca_aal_profile_idx, care_lspid, i);
						ca_aal_rule.action.dpid_vld = 1;
						ca_aal_rule.action.dpid_pri = 1;
						ca_aal_rule.action.permit = 1;
						ca_aal_rule.action.mc = 0; //1:mcgid=mcgid;0:mcgid=ldpid
						if(index==CL_RUL_PROFILE_WAN)
							ca_aal_rule.action.mcgid = RTK_FC_MAC_PORT_CPU1;
						else
							ca_aal_rule.action.mcgid = RTK_FC_MAC_PORT_CPU;
						ca_aal_rule.action.t2_ctrl_vld = 1;
						ca_aal_rule.action.t2_ctrl = CLS_T2CTRL_MAINHASH_PROFILE_2TUPLE_BRIDGE; 
						ca_aal_rule.action.stage2_ctrl_vld = 1; // Merge type-0 and type-1 result of Hash Engine
						ca_aal_rule.action.stage2_ctrl = 0;

						ASSERT_EQ(aal_l3_cls_add(RTK_ASIC_DEVID, index, &ca_aal_rule, &ca_aal_rslt_idx), CA_E_OK);
						aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%d-%d,", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
					}
					ASICINITPRINT("%s: ##### CLS initialization for HASH success @ %s[%s] #####\n", __func__, index?"LAN":"WAN", aal_idx_buf);
				}

			}
			break;
		default:
			ASICINITPRINT("%s: ##### CLS initialization fail due to not support type %d.#####\n", __func__, type);
			break;
	}

    return 0;
}

uint32_t rtk_9607f_acl_init(void)
{
	int i;
	aal_ilpb_cfg_msk_t ilpb_cfg_msk={0};
	aal_ilpb_cfg_t  ilpb_cfg;

	#define RTK_ASIC_ILPB_CLS_START 		0
	#define RTK_ASIC_ILPB_CLS_LENGTH		6

	//reset all L2/L3 CLS
#if defined(CONFIG_CORTINA_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA_V8)
	ASSERT_EQ(rtk_9607f_acl_add(ASIC_RSV_ACL_RESET_L2_L3_CLS), CA_E_OK);
#elif defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
	ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CLS_TYPE_RESET_L2_L3_CLS, NULL), CA_E_OK);
#endif

	//re-configure L2 IGR CLS per port start/length
	memset(&ilpb_cfg_msk, 0, sizeof(aal_ilpb_cfg_msk_t));
	memset(&ilpb_cfg, 0, sizeof(aal_ilpb_cfg_t));
	ilpb_cfg_msk.s.igr_cls_lookup_en = TRUE;
	ilpb_cfg_msk.s.cls_start = TRUE;
	ilpb_cfg_msk.s.cls_length = TRUE;
	ilpb_cfg.igr_cls_lookup_en = 1;

	/*each hw entry = LAN * 6 + WAN * 6 + CPU * 6 + Reserve * 6
		PORT IDX	|	HW IDX(start)	|	LENGTH
		0~6					0 					6
		7					6 					6
		0x10~0x17			12					6	*/
	for(i = ASIC_LPORT_ETH_NI0; i < ASIC_LPORT_ETH_NI7; i++){
		ilpb_cfg.cls_start = RTK_ASIC_ILPB_CLS_START;
		ilpb_cfg.cls_length = RTK_ASIC_ILPB_CLS_LENGTH;
		ASSERT_EQ(aal_port_ilpb_cfg_set(RTK_ASIC_DEVID, i, ilpb_cfg_msk, &ilpb_cfg), CA_E_OK);
	}
	for(i = ASIC_LPORT_ETH_NI7; i <= ASIC_LPORT_ETH_NI7; i++){
		ilpb_cfg.cls_start = RTK_ASIC_ILPB_CLS_START + 1*RTK_ASIC_ILPB_CLS_LENGTH;
		ilpb_cfg.cls_length = RTK_ASIC_ILPB_CLS_LENGTH;
		ASSERT_EQ(aal_port_ilpb_cfg_set(RTK_ASIC_DEVID, i, ilpb_cfg_msk, &ilpb_cfg), CA_E_OK);
	}
	for(i = ASIC_LPORT_CPU_0; i <= ASIC_LPORT_CPU_7; i++){
		ilpb_cfg.cls_start = RTK_ASIC_ILPB_CLS_START + 2*RTK_ASIC_ILPB_CLS_LENGTH;
		ilpb_cfg.cls_length = RTK_ASIC_ILPB_CLS_LENGTH;
		ASSERT_EQ(aal_port_ilpb_cfg_set(RTK_ASIC_DEVID, i, ilpb_cfg_msk, &ilpb_cfg), CA_E_OK);
	}

#if defined(CONFIG_CORTINA_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA) || defined(CONFIG_REALTEK_BOARD_FPGA_V8)
	//Create L2 Classifier with priority 0 for forward all packet to L3FE;
	ASSERT_EQ(rtk_9607f_acl_add(ASIC_RSV_ACL_L2_EGRESS_FORWARD_L3FE), CA_E_OK);

	//Create L3 Classifier for hash profile decision
	ASSERT_EQ(rtk_9607f_acl_add(ASIC_RSV_ACL_GENERIC_INTF_HASH_PROFILE_DECISION), CA_E_OK);
#elif defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
	ASSERT_EQ(aal_port_ilpb_cfg_get(RTK_ASIC_DEVID, ASIC_LPORT_ETH_NI0, &ilpb_cfg), CA_E_OK);
	fc_db.cls_l2_port_info[0][0] = ilpb_cfg.cls_start;
	fc_db.cls_l2_port_info[0][1] = ilpb_cfg.cls_length;
	ASSERT_EQ(aal_port_ilpb_cfg_get(RTK_ASIC_DEVID, ASIC_LPORT_ETH_NI7, &ilpb_cfg), CA_E_OK);
	fc_db.cls_l2_port_info[1][0] = ilpb_cfg.cls_start;
	fc_db.cls_l2_port_info[1][1] = ilpb_cfg.cls_length;
	ASSERT_EQ(aal_port_ilpb_cfg_get(RTK_ASIC_DEVID, ASIC_LPORT_CPU_0, &ilpb_cfg), CA_E_OK);
	fc_db.cls_l2_port_info[2][0] = ilpb_cfg.cls_start;
	fc_db.cls_l2_port_info[2][1] = ilpb_cfg.cls_length;

	fc_db.cls_l2_port_info[3][0] = fc_db.cls_l2_port_info[2][0] + fc_db.cls_l2_port_info[2][1];
	fc_db.cls_l2_port_info[3][1] = fc_db.cls_l2_port_info[2][1];

	//Create L2 Classifier with priority 3 for forward mc packet to L3FE;
	ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CLS_TYPE_L2_INGRESS_MC_FORWARD_L3FE, NULL), CA_E_OK);
	ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CLS_TYPE_L2_INGRESS_FORWARD_PORT, NULL), CA_E_OK);

	//Create L2 Classifier with priority 0 for forward all packet to L3FE;
	ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CLS_TYPE_L2_EGRESS_FORWARD_L3FE, NULL), CA_E_OK);

	//Create L3 Classifier for hash profile decision
	ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_GENERIC_INTF_HASH_PROFILE_DECISION, NULL), CA_E_OK);

	//Create L3 Classifier to trap ipv6 multicast dslite
	ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_MULTICAST_DSLITE_TRAP, NULL), CA_E_OK);

	//Create L3 Classifier to drop multicast jumbo packet to prevent hw stuck
	ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_MC_JUMBO_DROP, NULL), CA_E_OK);

	//Create L3 Classifier to trap ttl<=1 prevent dual header outter header ttl invalid
	ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_TTL_LESS_ONE_TRAP, NULL), CA_E_OK);
#endif

	ASICINITPRINT("%s: ##### L2/L3 CLS initialization success.#####\n", __func__);

	return CA_E_OK;
}

uint32 rtk_9607f_igr_egr_port_vlanFilter_disable(uint32 portId)
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
	if(aal_port_ilpb_cfg_set(RTK_ASIC_DEVID,portId,ilpb_msk,&ilpb_cfg)){
		ERROR("ERROR, aal api fail" );
	}

	elpb_msk.s.egr_ve_srch_en=1;
	elpb_cfg.egr_ve_srch_en=0;
	if(aal_port_elpb_cfg_set(RTK_ASIC_DEVID,portId,elpb_msk,&elpb_cfg)){
		ERROR("ERROR, aal api fail" );
	}

	return CA_E_OK;
}

static inline ca_boolean_t ipv4_is_mcast(ca_l3_ip_addr_t addr)
{
	return (addr.ipv4_addr & 0xF0000000) == 0xE0000000;
}

static inline ca_boolean_t ipv6_is_mcast(ca_l3_ip_addr_t addr)
{
	return (addr.ipv6_addr[0] & 0xFF000000) == 0xFF000000;
}

asic_ret_t rtk_9607f_flow_mcast_group_asknown (ca_ip_address_t groupAddr,ca_int32_t *hwIdx)
{

	aal_hash_key_t key = {0}, *hash_key = &key;
	aal_hash_action_t action = {0}, *hash_action = &action;
	ca_uint32_t h_idx, *hash_idx = &h_idx;
	asic_ret_t rc = ASIC_RET_SUCCESS;

	
	if (groupAddr.afi == CA_IPV4) 
	{
		if (groupAddr.addr_len != 32 ||!ipv4_is_mcast(groupAddr.ip_addr))
			return ASIC_RET_OUT_OF_RANGE;
		if ((groupAddr.addr_len >0) && (groupAddr.afi != CA_IPV4 ||groupAddr.addr_len != 32))
			return ASIC_RET_OUT_OF_RANGE;
	} 
	else 
	{
		if (groupAddr.addr_len != 128 ||!ipv6_is_mcast(groupAddr.ip_addr))
			return ASIC_RET_OUT_OF_RANGE;
		if ((groupAddr.addr_len >0) && (groupAddr.afi != CA_IPV6 ||groupAddr.addr_len != 128)) 
			return ASIC_RET_OUT_OF_RANGE;
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
	hash_action->chk_msk_ptr = asic_db.flow_profile_info[RTK_9607F_FLOW_PROFILE_FLOW_MC].tuple_info[MC_HASH_GRP_TRAP_TUPLE].hash_mask_idx;
	if (ASIC_RET_SUCCESS != (rc = aal_hash_add(RTK_ASIC_DEVID, hash_key, hash_action, TUPLE_TYPE_TYPE0, HASH_AGING_INVALID, asic_db.flow_profile_info[RTK_9607F_FLOW_PROFILE_FLOW_MC].tuple_info[MC_HASH_GRP_TRAP_TUPLE].hash_mask_idx, RTK_9607F_FLOW_PROFILE_FLOW_MC, hash_idx)))
	{
		ASICINITPRINT("Hash add failed rc=%d \n",rc);
		goto ERROR;	
	}
	*hwIdx = *hash_idx;

	return ASIC_RET_SUCCESS;

	ERROR:
		return ASIC_RET_FAIL;


}


asic_ret_t rtk_9607f_flow_mcast_group_asunknown(ca_ip_address_t groupAddr,ca_int32_t *hwIdx)
{
	asic_ret_t rc = ASIC_RET_SUCCESS;

	rc = aal_hash_delete(RTK_ASIC_DEVID, *hwIdx);
	if(rc != ASIC_RET_SUCCESS)
		ASICINITPRINT("delete group as known Error idx=%d",*hwIdx);	

	return rc;
}


uint32_t rtk_9607f_init(void)
{
	int i;
	ca_uint8_t keep=1;
	ca_uint32_t l3fe_tbl_count=4;
	aal_ni_hv_glb_internal_port_id_cfg_t portid_cfg={0};
	aal_ni_hv_glb_internal_port_id_cfg_mask_t portid_mask={0};
	aal_l3fe_lpb_tbl_cfg_t lpb_cfg={0};
	aal_l3fe_lpb_tbl_cfg_mask_t lpb_mask={0};
	aal_ilpb_cfg_msk_t ilpb_cfg_msk={0};
	aal_ilpb_cfg_t  ilpb_cfg={0};
	//ca_gen_intf_attrib_t gen_intf_attrib;

	_rtk_9607f_dataBase_init();

	//keep lspid unchange
	ASSERT_EQ(rtk_9607f_asic_l3fe_keep_lspid_unchange_set(0, &keep),CA_E_OK);
	ASICINITPRINT("%s: #####Set keep_lspid_unchange to %u#####\n", __func__,keep);

	//let packet from port 7 could enter l2fe
	ASSERT_EQ(aal_ni_hv_glb_internal_port_id_cfg_get(0, &portid_cfg),CA_E_OK);
	portid_mask.bf.wan_rxsel=1;
	portid_cfg.wan_rxsel=2;
	ASSERT_EQ(aal_ni_hv_glb_internal_port_id_cfg_set(0, portid_mask, &portid_cfg),CA_E_OK);
	ASICINITPRINT("%s: #####Set wan_rxsel to %u#####\n", __func__,portid_cfg.wan_rxsel);

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
	ASICINITPRINT("%s: #####Set wan_ind to %u#####\n", __func__,ilpb_cfg.wan_ind);

	//Disable MAC check in L3FE
	lpb_mask.s.mac_da_match_en=1;
	for(i=0;i<l3fe_tbl_count;i++)
	{
		ASSERT_EQ(aal_l3_stg0_lpb_tbl_get(RTK_ASIC_DEVID, i, &lpb_cfg),CA_E_OK);
		lpb_cfg.mac_da_match_en=0;
		ASSERT_EQ(aal_l3_stg0_lpb_tbl_set(RTK_ASIC_DEVID, i, lpb_mask, &lpb_cfg),CA_E_OK);
		ASICINITPRINT("%s: #####Set stg0tbl[%d]mac_da_match_en to %u#####\n", __func__,i,lpb_cfg.mac_da_match_en);
	}

	// L3 PP work around for HW bug
	{
		static aal_l3_specpkt_err_fwd_ctrl_t err_fwd_ctrl;
		aal_l3_specpkt_err_fwd_ctrl_mask_t err_fwd_ctrl_mask;

		ASSERT_EQ(aal_l3_specpkt_err_fwd_ctrl_get(RTK_ASIC_DEVID, &err_fwd_ctrl),CA_E_OK);
		memset(&err_fwd_ctrl_mask, 0, sizeof(err_fwd_ctrl_mask));

		// for l2tp S=1 packets should not bypass CLS, due to CLS should trap L2tp lcp & L2tp control even S=1
		err_fwd_ctrl_mask.s.l2tp_hdr_err = 1;
		err_fwd_ctrl.l2tp_hdr_err = 2;	//"Normal forward"

		// for VXLAN packets may be incorrectly hit rdp_ver_err
		err_fwd_ctrl_mask.s.rdp_ver_err = 1;
		err_fwd_ctrl.rdp_ver_err = 2;	//"Normal forward"
		err_fwd_ctrl.udp_zero_chksum_err = 2;	//"Normal forward"

		ASSERT_EQ(aal_l3_specpkt_err_fwd_ctrl_set(RTK_ASIC_DEVID, err_fwd_ctrl_mask, &err_fwd_ctrl),CA_E_OK);
	}

	rtk_9607f_asic_fb_init();

	/******************************************************
	 * L2/L3 Classifier
	 ******************************************************/
	rtk_9607f_acl_init();

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
			ASSERT_EQ(aal_port_ilpb_cfg_set(RTK_ASIC_DEVID, i, ilpb_cfg_msk, &ilpb_cfg), CA_E_OK);
			if(i == ASIC_LPORT_ETH_NI7)
				i = ASIC_LPORT_CPU_2-1;
		}

		memset(&lpb_cfg, 0, sizeof(aal_l3fe_lpb_tbl_cfg_t));
		memset(&lpb_mask, 0, sizeof(aal_l3fe_lpb_tbl_cfg_mask_t));
		lpb_mask.s.default_cos_sel = TRUE;
		lpb_mask.s.default_cos = TRUE;
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
		lpb_cfg.default_cos_sel = 0;        //for tx amsdu hdr-a cos
#else
		lpb_cfg.default_cos_sel = 1; 		//default is 2, 1: LPB default setting, 2: Dot1p mapping, 3: DSCP mapping
#endif	
		lpb_cfg.default_cos = 0;

		for(i = 0; i < l3fe_tbl_count; i++)
			ASSERT_EQ(aal_l3_stg0_lpb_tbl_set(RTK_ASIC_DEVID, i, lpb_mask, &lpb_cfg),CA_E_OK);

		ASICINITPRINT("%s: #####Disable L2FE/L3FE QoS Decision#####\n", __func__);
	}


	/**********
	 * Main Hash
	 **********/
	{
#if 0	// call flow tuple init by RG/FC driver
		rtk_9607f_flow_key_mask_t flowKeyMask;
		flowKeyMask.P12_vlanId = TRUE;
		flowKeyMask.P12_vlanPri = TRUE;
		flowKeyMask.P345_vlanId = TRUE;
		flowKeyMask.P345_vlanPri = TRUE;
		flowKeyMask.P345_dscp = TRUE;
		flowKeyMask.P345_ecn = TRUE;
		flowKeyMask.pall_vlan_deicfi = FALSE;
		ASSERT_EQ(rtk_9607f_flow_init(flowKeyMask), ASIC_RET_SUCCESS);
#endif
		// force disable aging countdown
		ASSERT_EQ(aal_hash_aging_timer_set(0), AAL_E_OK);
	}

	/**********
	 * VLAN
	 **********/
	{
		ca_uint32_t outerTPID[2] = {0x8100, 0x88a8};
		ca_uint32_t innerTPID[2] = {0x8100, 0x88a8};
		ASSERT_EQ(aal_l3fe_pp_tpid_set(2, outerTPID, 2, innerTPID), AAL_E_OK);
		for(i=ASIC_LPORT_ETH_NI0 ; i< ASIC_LPORT_PON_US_31 ;i++)
		{
			rtk_9607f_igr_egr_port_vlanFilter_disable(i);
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
			cfg.tx_tpid3=0x9100;
			mask.s.cmp_tpid_svlan=1;
			mask.s.cmp_tpid_cvlan=1;
			mask.s.cmp_tpid_other=1;
			mask.s.cmp_tpid_user=1;
			cfg.cmp_tpid_svlan	= 0x88a8;
			cfg.cmp_tpid_cvlan	= 0x8100;
			cfg.cmp_tpid_other	= 0x9100;
			cfg.cmp_tpid_user	= 0x9100;
			aal_l2_vlan_default_cfg_set(0,mask,&cfg);
		}		
	}

	/********************
	*  l3fe_pon_mode which will take the pol_id from hdr-a to l3fe hdr-i
	********************/
	{
		L3FE_GLB_GLB_CFG_t l3fe_glb;
		l3fe_glb.wrd = rtk_9607f_asic_ne_reg_read(L3FE_GLB_GLB_CFG);
		l3fe_glb.bf.l3fe_pon_mode = TRUE;
		rtk_9607f_asic_ne_reg_write(l3fe_glb.wrd, L3FE_GLB_GLB_CFG);
	}
#if 0 //1 FIXME: [9607F] temporary remove (need to remove CA API)

	/********************
	*  clear mcgroup
	********************/
	{
		ca_l2_mcast_group_delete_all(RTK_ASIC_DEVID);
		ca_l3_mcast_group_delete_all(RTK_ASIC_DEVID);
	}
#endif
	/**********
	 * L3 policer
	 **********/
	rtk_9607f_policer_init();

	/**********
	 * L3 QM
	 **********/
	rtk_9607f_qm_init();

	rtk_9607f_cpuport_init();
	rtk_9607f_dmalso_init();
	return CA_E_OK;
}

uint32_t rtk_9607f_exit(void)
{
	rtk_9607f_cpuport_exit();

	return CA_E_OK;
}

