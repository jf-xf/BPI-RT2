/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*/

#ifndef __RTK_FC_DEBUG__
#define __RTK_FC_DEBUG__

#include "rtk_fc_struct.h"

void _rtk_fc_dump_stack(void);
void _rtk_fc_dump_mem(unsigned char *ptitle, unsigned char *pbuf, int len);

void dump_packet(unsigned char *pkt,unsigned int size,char *memo);


#define HWNAT_DEBUG_LEVEL fc_db.debugLevel

void _rtk_fc_rtlglue_printf(char* fmt, ...);
char *proc_printf(struct seq_file *s, char *fmt, ...);
#define PROC_PRINTF( comment ,arg...)	proc_printf(s,comment,##arg)
#define rtlglue_print_hex_dump(arg...) 	print_hex_dump(KERN_EMERG, ##arg)
void _rtk_fc_tracelog_printf(int bitmask, int color, int bgcolor, char *string, const char *func, int line, char *dbgtmp, ...);


#define DUMP_PACKET(pkt,size,memo) \
do {\
	uint32 show=FC_DEBUG_LEVEL_TRACE_DUMP; \
	show&=HWNAT_DEBUG_LEVEL;	\
	if(fc_db.filterLevel&FC_DEBUG_LEVEL_TRACE_DUMP)\
	{ \
		show=fc_db.tracefilterShow; \
	} \
	if(show) { \
		dump_packet(pkt, size, memo);\
	}\
} while(0)

#ifndef FBDEBUG_PRK
#define FBDEBUG_PRK( bitmask, string, color,bgcolor,comment ,arg...) \
do {\
	uint32 show_mask=bitmask&HWNAT_DEBUG_LEVEL; \
	if(fc_db.filterLevel > FC_DEBUG_LEVEL_NONE) \
	{ \
		if(show_mask & fc_db.filterLevel) \
			show_mask=fc_db.tracefilterShow; \
		else \
			show_mask=0U;\
	}\
	if(show_mask)\
	{\
		_rtk_fc_tracelog_printf(bitmask, color, bgcolor, string, __FUNCTION__, __LINE__ , comment,## arg);\
	} \
} while(0)
#endif


#define DEBUG( comment ,arg...) 	FBDEBUG_PRK(FC_DEBUG_LEVEL_DEBUG,"DEBUG",33,40,comment,##arg)
#define FIXME( comment ,arg...) 	FBDEBUG_PRK(FC_DEBUG_LEVEL_FIXME,"FIXME",34,40,comment,##arg)
#define CBACK( comment ,arg...) 	FBDEBUG_PRK(FC_DEBUG_LEVEL_CALLBACK,"CALLBACK",34,40,comment,##arg)
#define TRACE( comment ,arg...) 	FBDEBUG_PRK(FC_DEBUG_LEVEL_TRACE,"TRACE",35,40,comment,##arg)
#define TRACE_RXINFO( comment ,arg...) 	FBDEBUG_PRK(FC_DEBUG_LEVEL_TRACE,"TRACE",37,45,comment,##arg)
#define TRACE_TXINFO( comment ,arg...) 	FBDEBUG_PRK(FC_DEBUG_LEVEL_TRACE,"TRACE",37,45,comment,##arg)
#define WARNING( comment ,arg...) 	FBDEBUG_PRK(FC_DEBUG_LEVEL_WARN,"WARNING",33,41,comment,##arg)
// TRACE_DUMP
#define EVENT( comment ,arg...) 	FBDEBUG_PRK(FC_DEBUG_LEVEL_EVENT,"EVENT",32,40,comment,##arg)
#define TABLE( comment ,arg...) 	FBDEBUG_PRK(FC_DEBUG_LEVEL_TABLE,"TABLE",31,40,comment,##arg)
#define ALG( comment ,arg...) 		FBDEBUG_PRK(FC_DEBUG_LEVEL_ALG,"ALG",31,40,comment,##arg)
#define IGMP( comment ,arg...) 		FBDEBUG_PRK(FC_DEBUG_LEVEL_IGMP,"IGMP",31,40,comment,##arg)
#define ACL_RSV( comment ,arg...) 	FBDEBUG_PRK(FC_DEBUG_LEVEL_ACL_RRESERVED,"ACL_RSV",36,40,comment,##arg)
#define API( comment ,arg...) 		FBDEBUG_PRK(FC_DEBUG_LEVEL_API,"API",36,40,comment,##arg)
#define TIMER( comment ,arg...) 	FBDEBUG_PRK(FC_DEBUG_LEVEL_TIMER,"TIMER",34,40,comment,##arg)
#define TMP( comment ,arg...) 		FBDEBUG_PRK(FC_DEBUG_LEVEL_TMP,"TMP",32,40,comment,##arg)
#define IPSEC( comment ,arg...) 		FBDEBUG_PRK(FC_DEBUG_LEVEL_IPSEC,"IPSEC",32,40,comment,##arg)
#define IPSEC_HOOK( comment ,arg...) 		FBDEBUG_PRK(FC_DEBUG_LEVEL_IPSEC_HOOK,"IPSEC_HK",31,40,comment,##arg)


#define PSTACK( comment ,arg...) 	FBDEBUG_PRK(FC_DEBUG_LEVEL_PS,"PSTACK",36,40,comment,##arg)
//#define IGR( comment ,arg...) 		FBDEBUG_PRK(FC_DEBUG_LEVEL_IGR,"IGR",36,40,comment,##arg)
//#define EGR( comment ,arg...) 		FBDEBUG_PRK(FC_DEBUG_LEVEL_EGR,"EGR",36,40,comment,##arg)
#define WIFI( comment ,arg...) 		FBDEBUG_PRK(FC_DEBUG_LEVEL_WIFI,"WIFI",36,40,comment,##arg)
#define ACL_CTRL( comment ,arg...) 	FBDEBUG_PRK(FC_DEBUG_LEVEL_ACL_CONTROL_PATH,"ACL_CTRL",36,40,comment,##arg)

#define DSLITE( comment ,arg...) 	FBDEBUG_PRK(FC_DEBUG_LEVEL_DSLITE,"DS-LITE",36,40,comment,##arg)
#define TOPS( comment ,arg...) 		FBDEBUG_PRK(FC_DEBUG_LEVEL_TOPS,"TOPS",36,40,comment,##arg)
#define FRAGMENT( comment ,arg...) 	FBDEBUG_PRK(FC_DEBUG_LEVEL_FRAGMENT,"FRAGMENT",33,40,comment,##arg)

#define PROC_PRINT_REG(reg) \
do {\
	uint32 regvalue = rtk_ne_reg_read(reg);\
	if(regvalue!=0)\
		PROC_PRINTF("%-28s(0x%x): %u\n", #reg, reg, regvalue);\
}while(0)

#define PROC_PRINT_REG_WITH_NAME(reg, name) \
do {\
	uint32 regvalue = rtk_ne_reg_read(reg);\
	if(regvalue!=0)\
		PROC_PRINTF("%-28s(0x%x): %u\n", name, reg, regvalue);\
}while(0)



int dump_lut_table(struct seq_file *s, void *v);
int dump_flow_table(struct seq_file *s, void *v);
int dump_flow_table_by_filter(struct file *file, const char *buffer, unsigned long count, void *data);
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
int dump_flow_key_conf(void);
#endif
int dump_netif(struct seq_file *s, void *v);
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
int dump_hs(struct seq_file *s, void *v);
int dump_l34hs(struct seq_file *s, void *v);
#endif
#if defined(CONFIG_RTK_L34_G3_PLATFORM) && defined(CONFIG_FC_CA8277AB_SERIES)
int dump_acl_ca(struct seq_file *s, void *v);
int dump_acl_ca_by_index(struct file *file, const char *buffer, unsigned long len, void *data);
#endif
int dump_acl(struct seq_file *s, void *v);
int dump_acl_by_index(struct file *file, const char *buffer, unsigned long len, void *data);
int dump_acl_reserved_info(struct seq_file *s, void *v);
int dump_acl_range_table(struct seq_file *s, void *v);
int dump_extratag_table(struct seq_file *s, void *v);
int dump_ethtype_table(struct seq_file *s, void *v);
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
int dump_flowdram_table(struct seq_file *s, void *v);
int dump_flowdram_table_by_filter(struct file *file, const char *buffer, unsigned long len, void *data);
int dump_sw_flowTcam_list(struct seq_file *s, void *v);
int dump_flowtag_table(struct seq_file *s, void *v);
int dump_camtag_table(struct seq_file *s, void *v);
int dump_flowtrf_table(struct seq_file *s, void *v);
int dump_indmac_table(struct seq_file *s, void *v);
int dump_extpmask_table(struct seq_file *s, void *v);
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
void _rtk_ca_cls_rule_dump(ca_uint32_t hw_index, ca_uint32_t priority, ca_int32_t index, ca_uint8_t is_l2);
int dump_headera(struct seq_file *s, void *v);
int dump_headeri(struct seq_file *s, void *v);
int dump_dropcount(struct seq_file *s, void *v);
int dump_dropcount_opcode(struct file *file, const char *buffer, unsigned long count, void *data);
int dump_ponStreamid_table(struct seq_file *s, void *v);
int rtk_fc_headeri_latchMode_set(struct file *filp, const char *buff,unsigned long length, void *data);

#if defined(CONFIG_FC_RTL8277C_SERIES) && RTK_FC_TABLESIZE_OVERFLOW_FLOW
int dump_sw_flowOverFlow_list_list(struct seq_file *s, void *v);
#endif
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
int32 dump_dmaAftAction_table(struct seq_file *s, void *v);
int32 dump_dmaAftAction_table_by_idx(struct file *file, const char *buffer, unsigned long count, void *data);
int32 dump_sw_dmaAftAction_table(struct seq_file *s, void *v);
int32 dump_sw_dmaAftAction_table_by_idx(struct file *file, const char *buffer, unsigned long count, void *data);
#if defined(CONFIG_FC_RTL9607F_SERIES)
int32 dump_dmaAftMap_table(struct seq_file *s, void *v);
int32 dump_sw_dmaAftMap_table(struct seq_file *s, void *v);
#endif
#endif
#endif

// common func for different platform //
int32 dump_flow_by_rawdata(struct seq_file *s, int32 idx, void *pFlowData);
int dump_flow_p1Rawdata(struct seq_file *s, int32 flowIdx, void *pFlowData);
int dump_flow_p2Rawdata(struct seq_file *s, int32 flowIdx, void *pFlowData);
int dump_flow_p3Rawdata(struct seq_file *s, int32 flowIdx, void *pFlowData);
int dump_flow_p4Rawdata(struct seq_file *s, int32 flowIdx, void *pFlowData);
int dump_flow_p5Rawdata(struct seq_file *s, int32 flowIdx, void *pFlowData);
int dump_flow_p6Rawdata(struct seq_file *s, int32 flowIdx, void *pFlowData);
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
int32 dump_flow_pMcWifiAmsduTxRawdata(struct seq_file *s, int32 flowIdx, void *pFlowData);
#endif
#endif

int dump_sw_netif(struct seq_file *s, void *v);
int dump_sw_netifmib(struct seq_file *s, void *v);
int rtk_fc_dump_fc_acl_entry_content(struct seq_file *s, rtk_fc_aclFilterEntry_t *acl_entry);
int dump_sw_acl(struct seq_file *s, void *v);
int32 dump_sw_flow_mib(struct seq_file *s, void *v);
int dump_flow_mib(struct seq_file *s, void *v);
int dump_abstrSwFlow_table(struct seq_file *s, void *v);
void dump_sw_flow_fields(struct seq_file *s, void *v, rtk_fc_tableFlow_t *pFlowTbl, uint32 flowIdx);
int dump_sw_flow_table(struct seq_file *s, void *v);
int dump_sw_flow_table_by_filter(struct file *file, const char *buffer, unsigned long count, void *data);

int dump_sw_flow_list(struct seq_file *s, void *v);
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
int dump_mc_macId_table(struct seq_file *s, void *v);
int dump_ct_hash_info_list(struct seq_file *s, void *v);
#endif
int dump_pe_crypto_connection(struct seq_file *s, void *v);
int dump_sw_mape_dst6_info(struct seq_file *s, void *v);
int dump_sw_memAllocStat(struct seq_file *s, void *v);
int dump_sw_memInfo(struct seq_file *s, void *v);
int dump_vlanGroupMacLimitStatistics(struct seq_file *s, void *v);
int rtk_fc_dump_collisionFlows(rtk_fc_tableFlowEntry_t *flow1, rtk_fc_tableFlowEntry_t *flow2);
int32 dump_hw_host_policing_mib(struct seq_file *s, void *v);
int32 dump_sw_host_policing_mib(struct seq_file *s, void *v);
int dump_rtMeter_table(struct seq_file *s, void *v);
int dump_rtMeter_debugging_table(struct file *file, const char *buffer, unsigned long count, void *data);
int dump_swShaper_table(struct seq_file *s, void *v);
int dump_sw_ackDelay(struct seq_file *s, void *v);
int dump_dual_passthrough_flowMapping_table(struct seq_file *s, void *v);
int dump_ipv6_nat_mapping_table(struct seq_file *s, void *v);
int dump_ipv6_mapt_mapping_table(struct seq_file *s, void *v);
int dump_l2dual_info_table(struct seq_file *s, void *v);
#if defined(CONFIG_FC_CA8277B_SERIES)
int dump_vxlan_extra_record(struct seq_file *s, void *v);
#endif
int dump_flow_connection(struct seq_file *s, void *v);
int dump_wan_access_limit_statistic(struct seq_file *s, void *v);
int dump_abstrSwFlowPatten_table(struct seq_file *s, void *v,rtk_fc_abstrSwFlowList_entry_t *pAbstrSwFlowEt);
int rtk_fc_proc_fwdStatistic_get(struct seq_file *s, void *v);
int rtk_fc_proc_fwdStatistic_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_fwdStatistic_drop_get(struct seq_file *s, void *v);
int rtk_fc_proc_flowLut_read(struct seq_file *s, void *v);
int rtk_fc_proc_flowLut_write(struct file *file, const char *buffer, unsigned long count, void *data);
int rtk_fc_proc_l2ivl_read(struct seq_file *s, void *v);
int dump_lutCam_list(struct seq_file *s, void *v);
int rtk_fc_proc_flowStatistic_read(struct seq_file *s, void *v);
int rtk_fc_proc_flowStatistic_write(struct file *file, const char *buffer, unsigned long count, void *data);
int rtk_fc_proc_fbMatch_read(struct seq_file *s, void *v);
int rtk_fc_proc_fbMatch_write(struct file *file, const char *buffer, unsigned long count, void *data);
int rtk_fc_proc_sw_flowStatistic_get(struct seq_file *f, void *data);
int rtk_fc_proc_sw_flowStatistic_set(struct file *filp, const char *buff,unsigned long len, void *data);
int dump_abstrSwflowType_table(struct seq_file *s, void *v,int type);

#if defined(CONFIG_RTK_SOC_RTL8198D)
int dump_flow_limit(struct seq_file *s, void *v);
#endif
#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_RTL8198XB_SERIES)
int dump_ext_flow_mib(struct seq_file *s, void *v);
#endif
#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_FC_RTL8198F_SERIES) || defined(CONFIG_RTL8198X_SERIES)
int rtk_fc_tcp_in_window_get(struct seq_file *s, void *v);
int rtk_fc_tcp_in_window_set(struct file *file, const char *buffer, unsigned long count, void *data);
int rtk_fc_default_route_info_get(struct seq_file *s, void *v);
#endif
#endif //__RTK_FC_DEBUG__
