 /*
  * Copyright (C) 2019 Realtek Semiconductor Corp.
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

#include "rtk_rg_ca8277_asicDriver.h"
#if defined(CONFIG_CA_G3_G3LITE_SERIES) || defined(CONFIG_CA8277B_SERIES)
#include "rtk_rg_g3_internal.h"
#endif
#include "ca_ext.h"

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
#include <aal_l3_if.h>
#include <aal_l3_cam.h>
#include <aal_l3_pe.h>


/*
 * Symbol Definition
 */
#define MAX_PRINT_LEN	128


/*
 * Data Declaration
 */
uint32_t asic_debug=0;

rtk_rg_asic_netif_ref_t asic_intf[FE_L3E_IF_TBL_ENTRY_MAX]={0};


/*
 * Macro Definition
 */
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
* Supported Compiler Flags
*	- 8277/G3: CONFIG_CA8279_SERIES
*	- 8277B/Venus: CONFIG_CA8277B_SERIES
*/


/*
 * Function Implementation
 */


static asic_ret_t rtk_rg_asic_ni_init(void)
{
	asic_ret_t ret = ASIC_RET_SUCCESS;

#if defined(CONFIG_CA8277B_SERIES)
	{
		// l3 egress mirror
		aal_ni_hv_glb_internal_port_id_cfg_mask_t niGlbCfgMask;
		aal_ni_hv_glb_internal_port_id_cfg_t niGlbCfg;

		niGlbCfgMask.wrd = 0;
		niGlbCfgMask.bf.egr_mrr_sel = TRUE;
		niGlbCfgMask.bf.mrr_dsel = TRUE;
		niGlbCfgMask.bf.mrr_ldpid = TRUE;
		niGlbCfgMask.bf.mrr_cos = TRUE;
		niGlbCfgMask.bf.mrr_fe_bypass = TRUE;
		niGlbCfg.egr_mrr_sel = 1; // L3FE egress mirror enabled
		niGlbCfg.mrr_dsel = 0x2; // mirrored packet destination is L2TM
		niGlbCfg.mrr_ldpid = AAL_LPORT_BLACKHOLE;
		niGlbCfg.mrr_cos = 0;
		niGlbCfg.mrr_fe_bypass = 1;
		aal_ni_hv_glb_internal_port_id_cfg_set(CA_DEF_DEVID, niGlbCfgMask, &niGlbCfg);
	}
#endif
	
	return ret;
}

static asic_ret_t rtk_rg_asic_l2fe_init(void)
{
	asic_ret_t ret = ASIC_RET_SUCCESS;
	
	{
		// dpb tabl: to bypass l2fe if ldpid is ASIC_LPORT_MC (0x1B )

		aal_dpb_cfg_msk_t dpbCfg_msk;
		aal_dpb_cfg_t dpbCfg;
		int lpid, cos;

		aal_port_dpb_ctrl_set(CA_DEF_DEVID, (ASIC_LPORT_MC >> 3)&0x7);

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
		lpid = ASIC_LPORT_MC;
		// mc data downstream, bypass l2fe
		for(cos = 0 ; cos < 8 ; cos++)
		{
			aal_port_dpb_cfg_set(CA_DEF_DEVID, ((lpid & 0x7) <<3) | (cos&0x7) , dpbCfg_msk, &dpbCfg) ;
		}
	}

	{
		// FDB
		lrn_fwd_ctrl_mask_t fdbFwdCtrlMask;
		lrn_fwd_ctrl_t fdbFwdCtrl;
		aal_fdb_ctrl_t	ctrl;
		aal_fdb_ctrl_mask_t cmsk;

		fdbFwdCtrlMask.wrd = 0;
		fdbFwdCtrlMask.bf.sa_limit_excd_fwd_ctrl = TRUE;
		fdbFwdCtrlMask.bf.sa_hash_cllsn_fwd_ctrl = TRUE;
		fdbFwdCtrl.limit_excd_mode = SA_LRN_FWD_CTRL_FORWARD;	// forward if over limit
		fdbFwdCtrl.hash_cllsn_mode = SA_LRN_FWD_CTRL_FORWARD;	// prevent drop src: sa_mis_or_sa_limit_drp_14

		aal_fdb_lrn_fwd_ctrl_set(CA_DEF_DEVID, fdbFwdCtrlMask, &fdbFwdCtrl);

		// learning/aging/hashing mode - HGU application
		cmsk.wrd = 0;
		cmsk.s.lrn_mode = 1;
		cmsk.s.aging_mode = 1;
		cmsk.s.hashing_mode = 1;
		ctrl.lrn_mode = CA_AAL_FDB_LRN_MODE_HW;	// allow PPTP port hw learning
		ctrl.aging_mode = 1;	// 1: software aging mode. do not timeout
		ctrl.hashing_mode = 0;	// 2'b00, hashing algorithm is XOR.
		aal_fdb_ctrl_set(0, cmsk, &ctrl);
	}

	{
		// DBUF and ARB pdpid: L2TM physical port mapping
		uint32_t mymac = 0;
		uint32_t ldpid = 0;

		for(mymac = 0; mymac <= 1; mymac++) {
			//aal_arb_pdpid_map_set

#if defined(CONFIG_CA8277B_SERIES)
			/*
			 * pon upstream ldpid = 0x0~0x7, 0x28~0x3
			 */
			{
				/* CPU direct TX: there are 2 cases of ldpid 0~6
					lspid RTK_FC_MAC_PORT_CPU0: to physical port 0~6 
					lspid RTK_FC_MAC_PORT_CPU1: to WAN port, ldpid is TCONT id here (dbuf_flg will be 1: CPU -> L2FE -> L2TM -> [L3QM ->] WAN port)
				*/
				ca_uint8_t dbuf_id = 0;
				aal_arb_port_dbuf_config_mask_t dbuf_config_mask;
				aal_arb_port_dbuf_config_t port_dbuf;

				dbuf_config_mask.wrd = 0;
				memset(&port_dbuf, 0, sizeof(port_dbuf));
				port_dbuf.dbuf_flg              = TRUE;
				port_dbuf.lspid_vld 			= TRUE;
				port_dbuf.lspid 				= RTK_FC_MAC_PORT_CPU1;
				dbuf_config_mask.bf.dbuf_flg	= TRUE;
				dbuf_config_mask.bf.lspid_vld	= TRUE;
				dbuf_config_mask.bf.lspid 		= TRUE;
				aal_arb_port_dbuf_set(CA_DEF_DEVID, dbuf_id, dbuf_config_mask, &port_dbuf);
			}

			// port 0x0 ~ port 0x6
			for(ldpid = ASIC_LPORT_ETH_NI0; ldpid <= ASIC_LPORT_ETH_NI6; ldpid++)
			{
				uint32_t stat = 0;		
				
				aal_port_physical_port_to_dq_get(CA_DEF_DEVID, ldpid, &stat);
				if(stat) {
					// QM is used by PHY ports	
					// case: deep_q = 0 (tx to lan/phy): QM
					aal_port_arb_ldpid_pdpid_map_set(CA_DEF_DEVID, mymac, 0, ldpid, AAL_PPORT_QM);
					// case: deep_q = 1 (tx to wan/puc): !QM
					aal_port_arb_ldpid_pdpid_map_set(CA_DEF_DEVID, mymac, 1, ldpid, AAL_PPORT_QM);
				}else {
					// QM is used by WAN TCONTs
					// case: deep_q = 0 (tx to lan/phy): !QM
					aal_port_arb_ldpid_pdpid_map_set(CA_DEF_DEVID, mymac, 0, ldpid, ldpid);
					// case: deep_q = 1 (tx to wan/puc): QM
					aal_port_arb_ldpid_pdpid_map_set(CA_DEF_DEVID, mymac, 1, ldpid, AAL_PPORT_QM);
				}
			}

			// port 0x20 ~ port 0x3F
			for(ldpid = ASIC_LPORT_PON_US_0; ldpid <= ASIC_LPORT_PON_US_31; ldpid++)
			{
				aal_port_arb_ldpid_pdpid_map_set(CA_DEF_DEVID, mymac, 0, ldpid, AAL_PPORT_NI7);
				aal_port_arb_ldpid_pdpid_map_set(CA_DEF_DEVID, mymac, 1, ldpid, AAL_PPORT_NI7);
			}

#endif

			// port 0x7
			aal_port_arb_ldpid_pdpid_map_set(CA_DEF_DEVID, mymac, 0, ASIC_LPORT_ETH_NI7, AAL_PPORT_QM);
			aal_port_arb_ldpid_pdpid_map_set(CA_DEF_DEVID, mymac, 1, ASIC_LPORT_ETH_NI7, AAL_PPORT_QM);

			// port 0x10 ~ port 0x17
			for(ldpid = ASIC_LPORT_CPU_0; ldpid <= ASIC_LPORT_CPU_7; ldpid++) {
				aal_port_arb_ldpid_pdpid_map_set(CA_DEF_DEVID, mymac, 0, ldpid, AAL_PPORT_CPU);
				aal_port_arb_ldpid_pdpid_map_set(CA_DEF_DEVID, mymac, 1, ldpid, AAL_PPORT_CPU);
			}

		}
	}

	
	return ret;
}

static asic_ret_t rtk_rg_asic_l3fe_init(void)
{
	asic_ret_t ret = ASIC_RET_SUCCESS;
	L3FE_GLB_GLB_CFG_t glbctrl;
	
	glbctrl.wrd = rtk_ne_reg_read(L3FE_GLB_GLB_CFG);
	glbctrl.bf.l3fe_pon_mode = TRUE;
	glbctrl.bf.mc_port = ASIC_LPORT_MC;
	rtk_ne_reg_write(glbctrl.wrd, L3FE_GLB_GLB_CFG);


#if defined(CONFIG_CA8277B_SERIES)
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
		
		rtk_ne_reg_write(res_ctrl.wrd, L3FE_GLB_RES_CTRL);
	}
#endif
	
	return ret;
}

static asic_ret_t rtk_rg_asic_l3te_init(void)
{
	asic_ret_t ret = ASIC_RET_SUCCESS;
#if defined(CONFIG_CA8277B_SERIES)
	TE_TE_GLB_CTRL_t ctrl;

	ctrl.wrd = rtk_ne_reg_read(TE_TE_GLB_CTRL);
	ctrl.bf.cntrcfg_clear_on_read = 1;
	rtk_ne_reg_write(ctrl.wrd, TE_TE_GLB_CTRL);
#endif

	return ret;
}

asic_ret_t rtk_rg_asic_l3qm_p4_init(void)
{
	asic_ret_t ret = ASIC_RET_SUCCESS;

#if defined(CONFIG_CA8279_SERIES)
	{
		// reuse port 7 QM resource
		QM_QM_DEST_PORT0_EQ_CFG_t dp_eq_cfg;
		dp_eq_cfg.wrd = rtk_ne_reg_read(QM_QM_DEST_PORT0_EQ_CFG + (QM_QM_DEST_PORT0_EQ_CFG_STRIDE * (CA_NI_PORT7 + CA_NI_TOTAL_CPU_PORT)));
		rtk_ne_reg_write(dp_eq_cfg.wrd , QM_QM_DEST_PORT0_EQ_CFG + (QM_QM_DEST_PORT0_EQ_CFG_STRIDE * (CA_NI_PORT4 + CA_NI_TOTAL_CPU_PORT)));
	}
#endif
	return ret;
}

#if 0
asic_ret_t rtk_rg_asic_l3qm_p3EQ_used_by_p4(void)
{
	asic_ret_t ret = ASIC_RET_SUCCESS;

#if defined(CONFIG_CA8277B_SERIES)
	// HW Acc.: if dest port is p4, change hash fib ldpid to p3 in order to use p3 QM buffer but remap L2TM p4 here.
	{
		L2TM_L2TM_BM_DQ_TO_TM_PORT_MAP_t map;
		map.wrd = rtk_ne_reg_read(L2TM_L2TM_BM_DQ_TO_TM_PORT_MAP);
		map.bf.tm_dpid_sel_3 = ASIC_LPORT_ETH_NI4;
		rtk_ne_reg_write(map.wrd, L2TM_L2TM_BM_DQ_TO_TM_PORT_MAP);
	}

	// CPU Dirtx: if dest port is p4, redirect to p3 inorder to use p3 QM buffer.
	{
		aal_arb_redir_config_mask_t redirCfg_msk;
		aal_arb_redir_config_t redirCfg;

		redirCfg_msk.wrd = 0;
		memset(&redirCfg, 0, sizeof(redirCfg));

		redirCfg_msk.bf.en = TRUE;
		redirCfg_msk.bf.ldpid = TRUE;

		redirCfg.en = TRUE;
		redirCfg.ldpid = ASIC_LPORT_ETH_NI3;
		
		aal_arb_redir_ldpid_config_set(CA_DEF_DEVID, ASIC_LPORT_ETH_NI4, redirCfg_msk, &redirCfg);
	}
#endif

	return ret;
}
#endif
static asic_ret_t rtk_rg_asic_l3qm_init(void)
{
	asic_ret_t ret = ASIC_RET_SUCCESS;
	
#if defined(CONFIG_CA8277B_SERIES)
	/* Init L3QM to PON configuration */
	{
		// enable gemid map mode - (hdr_i.t1_ctrl[3]==0 &&  hdr_i.t1_ctrl[0]==1) => (hdr_a.ldpid = ldpid_base  + hdr_i.t2_ctrl[3:0])
		ret = aal_l3pe_pe_gemid_map_set(1);	
		ret = aal_l3pe_pe_ldpid_base_set(ASIC_LPORT_PON_US_0);
	}

	{		
		uint32_t i = 0;
		uint32_t stat = 0;
	
		// QM demux sel
		// ldpid 0x0-0x7: set 1 if to LAN (TM), else to WAN
		aal_ni_hv_glb_internal_port_id_cfg_mask_t demux_mask;
		aal_ni_hv_glb_internal_port_id_cfg_t demux;

		demux_mask.wrd = 0;
		memset(&demux, 0, sizeof(demux));
		
		demux_mask.bf.l3qmrx_demux_sel = 1;
		
		for(i = ASIC_LPORT_ETH_NI0; i <= ASIC_LPORT_ETH_NI7; i++) {
			aal_port_physical_port_to_dq_get(CA_DEF_DEVID, i, &stat);
			if(stat) demux.l3qmrx_demux_sel |= (1<<i);
		}
#if defined(CONFIG_RG_G3_WAN_PORT_INDEX)&&CONFIG_RG_G3_WAN_PORT_INDEX!=7
		demux.l3qmrx_demux_sel |= (1<<CONFIG_RG_G3_WAN_PORT_INDEX);
#endif
		ret = aal_ni_hv_glb_internal_port_id_cfg_set(CA_DEF_DEVID, demux_mask, &demux);
	}

	{
		// QM_UPPER_LDPID_MAP.pon_offset = 0x20	=> scfg: CFG_ID_L3QM_LDPID_TO_PON_PORT_MAP_OFFSET
	}
	

	// NI_HV_GLB_INTERNAL_PORT_ID_CFG.wan_rxsel = 2	=> rtk_rg_g3_init
	
#endif

	{
		// Overwrite aal_ni_init_nirx_l3fe_demux() 	
		/*
		 * 8277A: 0x20 mc data -> L2FE
		 * 		 0x21~0x2F invalid ldpid
		 * 8277B:
		 * 		 0x4~0x7 pon upstream -> L3QM -> WAN
		 *		 0x28~0x2F pon upstream -> WAN
		 * 		 0x4~0x6 (deepq=0) -> L2TM -> LAN port
		 */
		ca_ni_rx_l3fe_demux_t demux;
		
#if defined(CONFIG_CA8277B_SERIES)
		/* 0x0 ~ 0xF */
		demux.wrd = rtk_ne_reg_read(NI_HV_GLB_NIRX_L3FE_DEMUX_CFG0);
		demux.bf.ldpid_0        = NIRX_L3FE_DEMUX_ID_L2TM;
		demux.bf.ldpid_1        = NIRX_L3FE_DEMUX_ID_L2TM;
		demux.bf.ldpid_2        = NIRX_L3FE_DEMUX_ID_L2TM;
		demux.bf.ldpid_3        = NIRX_L3FE_DEMUX_ID_L2TM;
		demux.bf.ldpid_4        = NIRX_L3FE_DEMUX_ID_L2TM;
		demux.bf.ldpid_5        = NIRX_L3FE_DEMUX_ID_L2TM;
		demux.bf.ldpid_6        = NIRX_L3FE_DEMUX_ID_L2TM;
		demux.bf.ldpid_7        = NIRX_L3FE_DEMUX_ID_L3QM;
		
		// when header-A's DEEP_Q is NOT set
		rtk_ne_reg_write(demux.wrd, NI_HV_GLB_NIRX_L3FE_DEMUX_CFG0);
		
		demux.wrd = rtk_ne_reg_read(NI_HV_GLB_NIRX_L3FE_DPQ_DEMUX_CFG0);
		demux.bf.ldpid_0        = NIRX_L3FE_DEMUX_ID_L3QM;
		demux.bf.ldpid_1        = NIRX_L3FE_DEMUX_ID_L3QM;
		demux.bf.ldpid_2        = NIRX_L3FE_DEMUX_ID_L3QM;
		demux.bf.ldpid_3        = NIRX_L3FE_DEMUX_ID_L3QM;
		demux.bf.ldpid_4        = NIRX_L3FE_DEMUX_ID_L3QM;
		demux.bf.ldpid_5        = NIRX_L3FE_DEMUX_ID_L3QM;
		demux.bf.ldpid_6        = NIRX_L3FE_DEMUX_ID_L3QM;
		demux.bf.ldpid_7        = NIRX_L3FE_DEMUX_ID_L3QM;
		
		// when header-A's DEEP_Q is set
		rtk_ne_reg_write(demux.wrd, NI_HV_GLB_NIRX_L3FE_DPQ_DEMUX_CFG0);
#endif

		/* 0x20 ~ 0x2F */
		demux.wrd = 0;
		demux.bf.ldpid_0		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_1		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_2		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_3		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_4		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_5		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_6		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_7		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_8		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_9		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_10		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_11		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_12		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_13		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_14		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_15		= NIRX_L3FE_DEMUX_ID_WAN;

		// when header-A's DEEP_Q is NOT set
		rtk_ne_reg_write(demux.wrd, NI_HV_GLB_NIRX_L3FE_DEMUX_CFG2);
		// when header-A's DEEP_Q is set
		rtk_ne_reg_write(demux.wrd, NI_HV_GLB_NIRX_L3FE_DPQ_DEMUX_CFG2);

		/* 0x30 ~ 0x3F */
		demux.wrd = 0;
		demux.bf.ldpid_0		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_1		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_2		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_3		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_4		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_5		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_6		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_7		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_8		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_9		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_10		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_11		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_12		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_13		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_14		= NIRX_L3FE_DEMUX_ID_WAN;
		demux.bf.ldpid_15		= NIRX_L3FE_DEMUX_ID_WAN;

		// when header-A's DEEP_Q is NOT set
		rtk_ne_reg_write(demux.wrd, NI_HV_GLB_NIRX_L3FE_DEMUX_CFG3);
		// when header-A's DEEP_Q is set
		rtk_ne_reg_write(demux.wrd, NI_HV_GLB_NIRX_L3FE_DPQ_DEMUX_CFG3);
	}


	return ret;
}
 
asic_ret_t rtk_rg_asic_netifTable_add(uint32_t idx, rtk_rg_asic_netif_entry_t *pNetif)
{
	asic_ret_t ret = ASIC_RET_SUCCESS;
	fe_l3e_if_tbl_entry_t eif;
	uint32_t mtu_idx=CA_UINT32_INVALID, mac_idx=CA_UINT32_INVALID;
	uint8_t validMac = 1;
	rtk_mac_t netif_mac_addr;
#if defined(CONFIG_CA_G3_G3LITE_SERIES) || defined(CONFIG_CA8277B_SERIES)
#if defined(CONFIG_CA8277B_SERIES)
	uint32_t hash_idx=CA_UINT32_INVALID;
	uint8_t need_netif_acl_extend = FALSE;
#endif	//CONFIG_CA8277B_SERIES
	rtk_fc_aclAndCf_reserved_generic_intf_mc_dmac_t netif_dmac;
	netif_dmac.acl_info = CA_UINT32_INVALID;
#endif

	PARAM_CHK(pNetif==NULL, ASIC_RET_NULL_POINTER);

	if((pNetif->gateway_mac_addr.octet[0] | pNetif->gateway_mac_addr.octet[1] | pNetif->gateway_mac_addr.octet[2] |
		pNetif->gateway_mac_addr.octet[3] | pNetif->gateway_mac_addr.octet[4] | pNetif->gateway_mac_addr.octet[5])==0) {
		validMac = 0;
	}

	PARAM_CHK((validMac && idx==0), ASIC_RET_NOT_SUPPORT);

	// support update
	if(asic_intf[idx].valid==1) {
		rtk_rg_asic_netifTable_del(idx);
	}

	// mtu
	if(pNetif->intf_mtu_check) {
		ret = aal_l3pe_mtu_size_add(pNetif->intf_mtu, &mtu_idx);
		if (ret!= AAL_E_OK) {
			ERROR("add netif[%d] mtu %d fail", idx, pNetif->intf_mtu);
			return ASIC_RET_FAIL;
		}
	}

	memset(&netif_mac_addr, 0x0, sizeof(netif_mac_addr));

	// mac
	if(validMac) {
		l3_cam_mac_da_tbl_entry_t l3_cam_mac;
		
		memcpy(&netif_mac_addr, pNetif->gateway_mac_addr.octet, ETH_ALEN);
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

#if defined(CONFIG_CA_G3_G3LITE_SERIES) || defined(CONFIG_CA8277B_SERIES)
			if(ret != AAL_E_OK){
				mac_idx = CA_UINT32_INVALID;
				//hash profile for mc should include this dmac in new acl rule
				if((pNetif->out_pppoe_act == FB_NETIFPPPOE_ACT_ADD) || (pNetif->out_pppoe_act == FB_NETIFPPPOE_ACT_MODIFY)) {
					netif_dmac.netif_idx = idx;	//for debug information
					memcpy(&netif_dmac.gateway_mac_addr, pNetif->gateway_mac_addr.octet, ETH_ALEN);
					ret = _rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_GENERIC_INTF_MC_PROFILE_WITH_DMAC, &netif_dmac);
					if (ret != AAL_E_OK)
						ERROR("fail to add reserve acl for netif[%d] fail (mac[%pM] aclInfo[0x%x] ret[%d])", idx, &pNetif->gateway_mac_addr.octet, netif_dmac.acl_info, ret);
				}
#if defined(CONFIG_CA8277B_SERIES)
				//hash profile for BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE function should include this dmac in new acl rule
				need_netif_acl_extend = TRUE;
#endif
			}
#endif

		}
		
#if defined(CONFIG_CA8277B_SERIES)
		
		ret = rtk_8277b_asic_hl_egrsmac_trans_add(idx, &pNetif->gateway_mac_addr.octet[0], &hash_idx);
				
#endif		

		if (ret != AAL_E_OK) {
			if (mtu_idx != CA_UINT32_INVALID)
				aal_l3pe_mtu_size_del(mtu_idx);

			ERROR("add netif[%d] mac %pM fail", idx, &pNetif->gateway_mac_addr.octet);
			return ASIC_RET_FAIL;
		}
	}


	// if
	{
		memset(&eif, 0, sizeof(fe_l3e_if_tbl_entry_t));

		eif.deep_q = 1;

		if(mtu_idx != CA_UINT32_INVALID){
			eif.mtu_vld = 1;
			eif.mtu_enc = mtu_idx;
		}

#if defined(CONFIG_RTL8277C_SERIES)
		// In 8277, sa translation will be done by main hash fib.
		// In 8277B, sa translation will be done by hashlite.
		if(mac_idx != CA_UINT32_INVALID) {
			eif.mac_sa_vld = 1;
			eif.mac_sa_an_sel = (mac_idx+1);
		}
#endif

	
		if(pNetif->out_pppoe_act == FB_NETIFPPPOE_ACT_REMOVE) {
#if defined(CONFIG_CA_G3_G3LITE_SERIES)
			// PATCH: keep pppoe tag by default. Remove pppoe tag by setting mdata bits[15:8] to indicate hashlite to cover remove action.
			// for non-pppoe interface: keep tag for passthrough
			eif.pppoe_set = 0;
			eif.pppoe_vld = 0;
			eif.pppoe_session_id = 0;
#else
			// 8277B
			// for non-pppoe interface: remove tag if smac_trans == 0
			eif.pppoe_set = 1;
			eif.pppoe_vld = 0;
			eif.pppoe_session_id = 0;
#endif
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
	memcpy(&asic_intf[idx].netif_mac_addr, &netif_mac_addr, ETH_ALEN);
	asic_intf[idx].mtu_idx = mtu_idx;
	asic_intf[idx].mac_idx = mac_idx;
#if defined(CONFIG_CA_G3_G3LITE_SERIES) || defined(CONFIG_CA8277B_SERIES)
	asic_intf[idx].acl_info = netif_dmac.acl_info;
#if defined(CONFIG_CA8277B_SERIES)
	asic_intf[idx].acl_info_flow_acc = CA_UINT32_INVALID;
	asic_intf[idx].acl_info_flow_acc_wan = CA_UINT32_INVALID;
	asic_intf[idx].hash_idx = hash_idx;

	if(need_netif_acl_extend){
		ret = _rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE_GW_MAC_UPDATE, &idx);
		if (ret != AAL_E_OK)	//if update success, it will update acl_info_flow_acc, acl_info_flow_acc_wan
			ERROR("fail to add 5to2 flow acc reserve acl for netif[%d] fail (mac[%pM] ret[%d])", idx, &asic_intf[idx].netif_mac_addr.octet, ret);
	}
#endif	//CONFIG_CA8277B_SERIES
#endif

#if defined(CONFIG_CA_G3_G3LITE_SERIES)
	DEBUG("Add netif[%d] mtuidx[%d] macidx[%d] aclMc[0x%x]", idx, mtu_idx, mac_idx, netif_dmac.acl_info);
#elif defined(CONFIG_CA8277B_SERIES)
	DEBUG("Add netif[%d] mtuidx[%d] macidx[%d] aclMc[0x%x] acl5to2[Lan 0x%x Wan 0x%x] hashidx[%d]", idx, mtu_idx, mac_idx, netif_dmac.acl_info, asic_intf[idx].acl_info_flow_acc, asic_intf[idx].acl_info_flow_acc_wan, hash_idx);
#else
	DEBUG("Add netif[%d] mtuidx[%d] macidx[%d]", idx, mtu_idx, mac_idx);
#endif

	return ret;
}

asic_ret_t rtk_rg_asic_netifTable_del(uint32_t idx)
{
	asic_ret_t ret = ASIC_RET_SUCCESS;

	PARAM_CHK(idx>=FE_L3E_IF_TBL_ENTRY_MAX, ASIC_RET_OUT_OF_RANGE);
	
	if(asic_intf[idx].valid == 0) {
		return ASIC_RET_SUCCESS;
	}

	memset(&asic_intf[idx].netif_mac_addr, 0x0, ETH_ALEN);
	if((ret = aal_entry_del(AAL_TABLE_FE_L3E_IF_TBL, idx)) != AAL_E_OK) {
		ERROR("fail to delete netif[%d], ret = %d", idx, ret);
	}

	if(asic_intf[idx].mtu_idx <= 3){
		if((ret = aal_l3pe_mtu_size_del(asic_intf[idx].mtu_idx)) != AAL_E_OK) {
			ERROR("fail to delete netif[%d] mtuidx[%d], ret = %d", idx, asic_intf[idx].mtu_idx, ret);
		}
	}
	if(asic_intf[idx].mac_idx < L3_CAM_MAC_DA_TBL_ENTRY_MAX) {
		if((ret = aal_entry_del(AAL_TABLE_L3_CAM_MAC_DA, asic_intf[idx].mac_idx)) != AAL_E_OK) {
			ERROR("fail to delete netif[%d] macidx[%d], ret = %d", idx, asic_intf[idx].mac_idx, ret);
		}
	}
#if defined(CONFIG_CA_G3_G3LITE_SERIES) || defined(CONFIG_CA8277B_SERIES)
	if(asic_intf[idx].acl_info != CA_UINT32_INVALID) {
		if((ret = _rtk_rg_aclAndCfReservedRuleDelSpecial(RTK_CA_CLS_TYPE_GENERIC_INTF_MC_PROFILE_WITH_DMAC, &asic_intf[idx].acl_info)) != AAL_E_OK) {
			ERROR("fail to delete netif[%d] aclMc[0x%x](TBL/KEY_IDX: %d / %d), ret = %d", idx, asic_intf[idx].acl_info, CLS_TBL_ID(asic_intf[idx].acl_info), CLS_KEY_ID(asic_intf[idx].acl_info), ret);
		}
	}
#if defined(CONFIG_CA8277B_SERIES)
	if(asic_intf[idx].acl_info_flow_acc != CA_UINT32_INVALID) {	//will also update acl_info_flow_acc_wan
		if((ret = _rtk_rg_aclAndCfReservedRuleDelSpecial(RTK_CA_CLS_TYPE_BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE_GW_MAC_UPDATE, &idx)) != AAL_E_OK) {
			ERROR("fail to delete netif[%d] acl5to2[Wan 0x%x](TBL/KEY_IDX: %d / %d)[Lan 0x%x](TBL/KEY_IDX: %d / %d), ret = %d", idx, asic_intf[idx].acl_info_flow_acc_wan, CLS_TBL_ID(asic_intf[idx].acl_info_flow_acc_wan), CLS_KEY_ID(asic_intf[idx].acl_info_flow_acc_wan),
				asic_intf[idx].acl_info_flow_acc, CLS_TBL_ID(asic_intf[idx].acl_info_flow_acc), CLS_KEY_ID(asic_intf[idx].acl_info_flow_acc), ret);
		}
	}
	if(asic_intf[idx].hash_idx != CA_UINT32_INVALID) {
		if((ret = rtk_8277b_asic_hl_egrsmac_trans_del(asic_intf[idx].hash_idx)) != AAL_E_OK) {
			ERROR("fail to delete netif[%d] hashidx[%d], ret = %d", idx, asic_intf[idx].hash_idx, ret);
		}
	}
#endif	//CONFIG_CA8277B_SERIES
#endif

#if defined(CONFIG_CA_G3_G3LITE_SERIES)
	DEBUG("Del netif[%d] mtuidx[%d] macidx[%d] aclMc[0x%x]", idx, asic_intf[idx].mtu_idx, asic_intf[idx].mac_idx, asic_intf[idx].acl_info);
#elif defined(CONFIG_CA8277B_SERIES)
	DEBUG("Del netif[%d] mtuidx[%d] macidx[%d] aclMc[0x%x] acl5to2[0x%x/0x%x] hashidx[%d]", idx, asic_intf[idx].mtu_idx, asic_intf[idx].mac_idx, asic_intf[idx].acl_info, asic_intf[idx].acl_info_flow_acc_wan, asic_intf[idx].acl_info_flow_acc, asic_intf[idx].hash_idx);
#else
	DEBUG("Del netif[%d] mtuidx[%d] macidx[%d]", idx, asic_intf[idx].mtu_idx, asic_intf[idx].mac_idx);
#endif
	
	memset(&asic_intf[idx], 0, sizeof(rtk_rg_asic_netif_ref_t));
	
	return ret;
}

#if defined(CONFIG_CA8277B_SERIES)
asic_ret_t rtk_rg_asic_netifTable_get(uint32_t idx, rtk_rg_asic_netif_ref_t *pAsicNetif)
{
	PARAM_CHK(pAsicNetif==NULL, ASIC_RET_NULL_POINTER);
	PARAM_CHK(idx >= FE_L3E_IF_TBL_ENTRY_MAX, ASIC_RET_OUT_OF_RANGE);

	memcpy(pAsicNetif, &asic_intf[idx], sizeof(asic_intf[0]));

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_rg_asic_netifTable_set(uint32_t idx, rtk_rg_asic_netif_ref_t *pAsicNetif)
{	//only update acl related info
	PARAM_CHK(pAsicNetif==NULL, ASIC_RET_NULL_POINTER);
	PARAM_CHK(idx >= FE_L3E_IF_TBL_ENTRY_MAX, ASIC_RET_OUT_OF_RANGE);

	asic_intf[idx].acl_info_flow_acc = pAsicNetif->acl_info_flow_acc;
	asic_intf[idx].acl_info_flow_acc_wan = pAsicNetif->acl_info_flow_acc_wan;

	return ASIC_RET_SUCCESS;
}

asic_ret_t rtk_rg_asic_dmaAftFib_set(uint32_t idx, rtk_rg_asic_dmaAftFib_t dmaAftFib)
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

asic_ret_t rtk_rg_asic_dmaAftFib_get(uint32_t idx, rtk_rg_asic_dmaAftFib_t *pDmaAftFib)
{
	DMA_SEC_DMA_AFT_L2FIB_DATA0_t fib0;
	DMA_SEC_DMA_AFT_L2FIB_DATA1_t fib1;
	DMA_SEC_DMA_AFT_L2FIB_DATA2_t fib2;
	asic_ret_t ret = ASIC_RET_SUCCESS;

	PARAM_CHK(idx >= DMA_AFT_FIB_SIZE, ASIC_RET_OUT_OF_RANGE);

	memset(pDmaAftFib, 0, sizeof(rtk_rg_asic_dmaAftFib_t));

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
asic_ret_t rtk_rg_asic_dmaAftTpid_set(char tpid_set_msk, rtk_rg_asic_dmaAftTpid_t aftTpid)
{
	DMA_SEC_DMA_AFT_TPID_0_t	tpid01;
	DMA_SEC_DMA_AFT_TPID_1_t	tpid23;

	tpid01.wrd = rtk_dma_lso_reg_read(DMA_SEC_DMA_AFT_TPID_0);
	tpid23.wrd = rtk_dma_lso_reg_read(DMA_SEC_DMA_AFT_TPID_1);

	if(tpid_set_msk & (1 << 0))
		tpid01.bf.tpid_0 = aftTpid.tpid_0;
	if(tpid_set_msk & (1 << 1))
		tpid01.bf.tpid_1 = aftTpid.tpid_1;
	if(tpid_set_msk & (1 << 2))
		tpid23.bf.tpid_2 = aftTpid.tpid_2;
	if(tpid_set_msk & (1 << 3))
		tpid23.bf.tpid_3 = aftTpid.tpid_3;

	rtk_dma_lso_reg_write(tpid01.wrd, DMA_SEC_DMA_AFT_TPID_0);
	rtk_dma_lso_reg_write(tpid23.wrd, DMA_SEC_DMA_AFT_TPID_1);

	return ASIC_RET_SUCCESS;
}
asic_ret_t rtk_rg_asic_dmaAftTpid_get(rtk_rg_asic_dmaAftTpid_t *pAftTpid)
{
	DMA_SEC_DMA_AFT_TPID_0_t	tpid01;
	DMA_SEC_DMA_AFT_TPID_1_t	tpid23;

	tpid01.wrd = rtk_dma_lso_reg_read(DMA_SEC_DMA_AFT_TPID_0);
	tpid23.wrd = rtk_dma_lso_reg_read(DMA_SEC_DMA_AFT_TPID_1);

	pAftTpid->tpid_0 = tpid01.bf.tpid_0;
	pAftTpid->tpid_1 = tpid01.bf.tpid_1;
	pAftTpid->tpid_2 = tpid23.bf.tpid_2;
	pAftTpid->tpid_3 = tpid23.bf.tpid_3;

	return ASIC_RET_SUCCESS;
}

#endif

asic_ret_t rtk_rg_asic_fb_init(void)
{
	int i=0;
	
	DEBUG("ASIC INIT");
	
	/*
	 * Table init
	 */
	for(i=0; i<FE_L3E_IF_TBL_ENTRY_MAX; i++) {
		rtk_rg_asic_netifTable_del(i);
	}
#if defined(CONFIG_CA8277B_SERIES)
	{
		rtk_rg_asic_dmaAftFib_t fib;
		rtk_rg_asic_dmaAftTpid_t tpidConf;

		memset(&fib, 0, sizeof(fib));
		for(i=0; i<DMA_AFT_FIB_SIZE; i++)
			rtk_rg_asic_dmaAftFib_set(i, fib);

		tpidConf.tpid_0 = 0x8100;
		tpidConf.tpid_1 = 0x88a8;
		tpidConf.tpid_2 = 0x9100;
		tpidConf.tpid_3 = 0x9200;
		rtk_rg_asic_dmaAftTpid_set(0xf, tpidConf);
	}
#endif
	
	rtk_rg_asic_l2fe_init();

	rtk_rg_asic_l3fe_init();

	rtk_rg_asic_l3te_init();
	
	rtk_rg_asic_l3qm_init();
	
	rtk_rg_asic_ni_init();

	return ASIC_RET_SUCCESS;
}
