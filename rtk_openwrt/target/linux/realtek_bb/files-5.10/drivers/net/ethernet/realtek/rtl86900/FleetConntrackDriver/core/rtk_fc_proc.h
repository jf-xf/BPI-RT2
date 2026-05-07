#include <linux/seq_file.h>
#include <linux/inet.h>

#include "rtk_fc_define.h"
#include "rtk_fc_struct.h"
#include "rtk_fc_debug.h"
#include "rtk_fc_internal.h"
#include "rtk_fc_driver.h"

extern char *name_of_dispatch_mode[];

extern char *name_of_rg_remarkingBits[];

enum rtk_rg_parseDataType
{
   MAC=0,
   V4IP,
   V6IP,
   STRING_EN_DIS, //warning : only use for uint32* parameterGetAddr
   UNSIGNED_LONG, //uint32
   SINGED_LONG,	  //int32
   STRING,		  //warning : should sure string non-overflow
};
int32 _rtk_fc_parseGetValue(char* stringBuff,void *parameterGetAddr,char* parameterName,enum rtk_rg_parseDataType dataType);   

int _rtk_fc_proc_parsing_string_to_integer(const char *buff,unsigned long len);

void rtk_fc_proc_init(void);
void rtk_fc_proc_exit(void);

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
int rtk_fc_proc_mapeNetifFmr_write(struct file *file, const char *buffer, unsigned long count, void *data);
int rtk_fc_proc_mapeNetifFmr_get(struct seq_file *s, void *v);
#endif
int rtk_fc_proc_maptNetifDmr_write(struct file *file, const char *buffer, unsigned long count, void *data);
int rtk_fc_proc_maptNetifDmr_get(struct seq_file *s, void *v);


/* Proc control functions */
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
int rtk_fc_proc_ldma_test_set(struct file *filp, const char *buff,unsigned long count, void *data);
#endif
#if defined(CONFIG_RTK_PTOOL_CPU_PERF)
#if defined(CONFIG_FC_RTL9607C_SERIES)
int rtk_fc_proc_test_gdma_set(struct file *filp, const char *buff,unsigned long count, void *data, unsigned int burst_size);
int rtk_fc_proc_test_gdma_4WORD_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_test_gdma_8WORD_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_test_gdma_16WORD_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_test_gdma_32WORD_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_test_gdma_32WORD_0_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_test_gdma_32WORD_1_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_test_gdma_32WORD_2_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_test_gdma_32WORD_3_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_test_gdma_32WORD_01_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_test_gdma_32WORD_02_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_test_gdma_32WORD_03_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_test_gdma_32WORD_12_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_test_gdma_32WORD_13_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_test_gdma_32WORD_23_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_test_gdma_32WORD_012_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_test_gdma_32WORD_013_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_test_gdma_32WORD_023_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_test_gdma_32WORD_123_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_test_gdma_get(struct seq_file *s, void *v);
int rtk_fc_proc_test_memcpy_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_test_memcpy_get(struct seq_file *s, void *v);
#endif
#endif
int rtk_fc_proc_global_ctrl_get(struct seq_file *s, void *v);
int rtk_fc_proc_global_ctrl_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_acl_multiple_hit_cfg_get(struct seq_file *s, void *v);
int rtk_fc_proc_acl_multiple_hit_cfg_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_l2_macEgrLearning_get(struct seq_file *s, void *v);
int rtk_fc_proc_l2_macEgrLearning_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_l2_mac_portGroup_get(struct seq_file *s, void *v);
int rtk_fc_proc_l2_mac_portGroup_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_epon_llid_format_get(struct seq_file *s, void *v);
int rtk_fc_proc_epon_llid_format_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_flow_tcp_learning_state_get(struct seq_file *s, void *v);
int rtk_fc_proc_flow_tcp_learning_state_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_flow_tcp_bidirection_learning_get(struct seq_file *s, void *v);
int rtk_fc_proc_flow_tcp_bidirection_learning_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_pppoe_connectionAutoExtend_period_get(struct seq_file *s, void *v);
int rtk_fc_proc_pppoe_connectionAutoExtend_period_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_bridge_5tuple_flow_accelerate_by_2tuple_get(struct seq_file *s, void *v);
int rtk_fc_proc_bridge_5tuple_flow_accelerate_by_2tuple_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_bridge_2tuple_flow_ethtype_check_get(struct seq_file *s, void *v);
int rtk_fc_proc_bridge_2tuple_flow_ethtype_check_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_bc_hwflow_accelerate_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_bc_hwflow_accelerate_get(struct seq_file *s, void *v);
int rtk_fc_proc_abstrSwFlowTemplateType_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_abstrSwFlowTemplateType_get(struct seq_file *s, void *v);
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES)
int rtk_fc_proc_xgspon_ds_to_l3fe_get(struct seq_file *s, void *v);
int rtk_fc_proc_xgspon_ds_to_l3fe_set(struct file *filp, const char *buff,unsigned long len, void *data);
#endif
int rtk_fc_proc_flow_meter_mib_conf_dependence_get(struct seq_file *s, void *v);
int rtk_fc_proc_flow_meter_mib_conf_dependence_set(struct file *filp, const char *buff,unsigned long len, void *data);
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
int rtk_fc_proc_hw_error_pkt_check_get(struct seq_file *s, void *v);
int rtk_fc_proc_hw_error_pkt_check_set(struct file *filp, const char *buff,unsigned long len, void *data);
#endif
int rtk_fc_headeri_latchMode_set(struct file *filp, const char *buff,unsigned long len, void *data);
#endif
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
int rtk_fc_proc_dsliteHwAcceleration_disable_get(struct seq_file *s, void *v);
int rtk_fc_proc_dsliteHwAcceleration_disable_set(struct file *filp, const char *buff,unsigned long len, void *data);
#endif
int rtk_fc_proc_dsliteV6TosFromV4_get(struct seq_file *s, void *v);
int rtk_fc_proc_dsliteV6TosFromV4_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_v6RDSwAcceleration_disable_get(struct seq_file *s, void *v);
int rtk_fc_proc_v6RDSwAcceleration_disable_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_skbmark_streamId_enable_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_streamId_enable_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_skbmark_swshaper_enable_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_swshaper_enable_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_skbmark_streamId_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_streamId_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_skbmark_psFloodFwdAcc_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_psFloodFwdAcc_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_skbmark_meterId_enable_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_meterId_enable_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_skbmark_meterId_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_meterId_set(struct file *filp, const char *buff,unsigned long len, void *data);
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
#if !defined(CONFIG_FC_RTL9607F_SERIES)
//TODO: 9607F flow cache mib
int rtk_fc_proc_skbmark_flow_cache_mib_enable_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_flow_cache_mib_enable_set(struct file *filp, const char *buff,unsigned long count, void *data);
#endif
int rtk_fc_proc_skbmark_mapet_fmr_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_mapet_fmr_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_vxlan_v6_en_hwacc_get(struct seq_file *s, void *v);
int rtk_fc_proc_vxlan_v6_en_hwacc_set(struct file *filp, const char *buff, unsigned long count, void *data);
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)

int rtk_fc_proc_ipsec_en_shortCut_get(struct seq_file *s, void *v);
int rtk_fc_proc_ipsec_en_shortCut_set(struct file *filp, const char *buff, unsigned long count, void *data);

int rtk_fc_proc_ipsec_en_pe_offload_get(struct seq_file *s, void *v);
int rtk_fc_proc_ipsec_en_pe_offload_set(struct file *filp, const char *buff, unsigned long count, void *data);
int rtk_fc_proc_ipsec_key_dump_get(struct seq_file *s, void *v);

int rtk_fc_proc_ipsec_debug_get(struct seq_file *s, void *v);
int rtk_fc_proc_ipsec_debug_set(struct file *filp, const char *buff, unsigned long count, void *data);
#endif

#endif
int rtk_fc_proc_skbmark_mibId_enable_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_mibId_enable_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_skbmark_mibId_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_mibId_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_skbmark_qid_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_qid_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_skbmark_fwdByPs_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_fwdByPs_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_skbmark_skipFcFunc_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_skipFcFunc_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_skbmark_wanDsLoopback_en_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_wanDsLoopback_en_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_skbmark_swIntfIdx_assign_en_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_swIntfIdx_assign_en_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_skbmark_swIntfIdx_assign_inOut_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_swIntfIdx_assign_inOut_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_skbmark_swIntfIdx_assign_idx_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_swIntfIdx_assign_idx_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_skbmark_skip_fc_alg_check_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_skip_fc_alg_check_set(struct file *filp, const char *buff,unsigned long count, void *data);
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
int rtk_fc_proc_skbmark_cvlan_vid_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_cvlan_vid_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_skbmark_cvlan_pri_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_cvlan_pri_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_skbmark_cvlan_action_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_cvlan_action_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_skbmark_svlan_vid_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_svlan_vid_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_skbmark_svlan_pri_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_svlan_pri_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_skbmark_svlan_action_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_svlan_action_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_skbmark_svlan_tpid_get(struct seq_file *s, void *v);
int rtk_fc_proc_skbmark_svlan_tpid_set(struct file *filp, const char *buff,unsigned long count, void *data);
#endif
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
int rtk_fc_wifi_amsdu_pe_offload_mode_macId_get(struct seq_file *s, void *v);
int rtk_fc_wifi_amsdu_pe_offload_mode_macId_set(struct file *file, const char *buffer, unsigned long count, void *data);
#endif
int rtk_fc_igmp_mcExtFlowIdxTbl_write(struct file *file, const char *buffer, unsigned long count, void *data);


int rtk_fc_proc_flow_hashInputConfig_get(struct seq_file *s, void *v);
int rtk_fc_proc_flow_hashInputConfig_set(struct file *filp, const char *buff,unsigned long len, void *data);

int rtk_fc_proc_host_policing_config_get(struct seq_file *s, void *v);
int rtk_fc_proc_host_policing_config_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_hw_host_policing_mib_reset(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_sw_host_policing_mib_reset(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_wanPortMask_get(struct seq_file *s, void *v);
int rtk_fc_proc_wanPortMask_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_wlan_wanPortMask_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_macPortMaskWithoutCpu_get(struct seq_file *s, void *v);
int rtk_fc_proc_macPortMaskWithoutCpu_set(struct file *filp, const char *buff,unsigned long len, void *data);

int rtk_fc_proc_wan_access_limit_probeInterval_get(struct seq_file *s, void *v);
int rtk_fc_proc_wan_access_limit_probeInterval_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_flowSyncPeriod_get(struct seq_file *s, void *v);
int rtk_fc_proc_flowSyncPeriod_set(struct file *filp, const char *buff,unsigned long len, void *data);
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
int rtk_fc_proc_amsdu_pkt_get(struct seq_file *s, void *v);
int rtk_fc_proc_amsdu_pkt_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_amsdu_pkt_size_get(struct seq_file *s, void *v);
int rtk_fc_proc_amsdu_pkt_size_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_amsdu_time_out_get(struct seq_file *s, void *v);
int rtk_fc_proc_amsdu_time_out_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_pe_offload_wifi_amsdu_en_get(struct seq_file *s, void *v);
int rtk_fc_proc_pe_offload_wifi_amsdu_en_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_pe_offload_wifi_amsdu_func_switching_get(struct seq_file *s, void *v);
int rtk_fc_proc_pe_offload_wifi_amsdu_func_switching_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_wifi_amsdu_wlan_idx_table_get(struct seq_file *s, void *v);
#endif
int rtk_fc_proc_nonfdbUc_timeout_get(struct seq_file *s, void *v);
int rtk_fc_proc_nonfdbUc_timeout_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_nonStaticMc_timeout_get(struct seq_file *s, void *v);
int rtk_fc_proc_nonStaticMc_timeout_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_nonCtFlowTimeoutSec_get(struct seq_file *s, void *v);
int rtk_fc_proc_nonCtFlowTimeoutSec_set(struct file *filp, const char *buff,unsigned long len, void *data);
int _rtk_fc_proc_sendFromCpu_uasge(struct seq_file *s, void *v);
int _rtk_fc_proc_sendFromCpu(struct file *filp, const char *buff,unsigned long len, void *data );

int rtk_fc_proc_flow_operation_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_flow_operation_get(struct seq_file *s, void *v);

int rtk_fc_proc_rstconntrack(struct file *file, const char *buffer, unsigned long count, void *data);
int rtk_fc_proc_fbmode_get(struct seq_file *s, void *v);
int rtk_fc_proc_fbmode_set(struct file *file, const char *buffer, unsigned long count, void *data);
int rtk_fc_proc_flowFlush_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_ct_hash_info_list_flush(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_reset_ipv6_nat_mapping_table(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_reset_ipv6_mapt_mapping_table(struct file *filp, const char *buff,unsigned long len, void *data);

int rtk_fc_proc_flow_not_update_in_real_time_get(struct seq_file *s, void *v);
int rtk_fc_proc_flow_not_update_in_real_time_set(struct file *filp, const char *buff,unsigned long len, void *data);
#if !(defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES))
int rtk_fc_proc_prehashptn_sport_get(struct seq_file *s, void *v);
int rtk_fc_proc_prehashptn_sport_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_prehashptn_dport_get(struct seq_file *s, void *v);
int rtk_fc_proc_prehashptn_dport_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_prehashptn_sip_get(struct seq_file *s, void *v);
int rtk_fc_proc_prehashptn_sip_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_prehashptn_dip_get(struct seq_file *s, void *v);
int rtk_fc_proc_prehashptn_dip_set(struct file *filp, const char *buff,unsigned long len, void *data );
#endif
int rtk_fc_proc_flowSessionAutoExtend_get(struct seq_file *s, void *v);
int rtk_fc_proc_flowSessionAutoExtend_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_flow_l2_skipPsTracking_get(struct seq_file *s, void *v);
int rtk_fc_proc_flow_l2_skipPsTracking_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_flowSkipAllPsTracking_get(struct seq_file *s, void *v);
int rtk_fc_proc_flowSkipAllPsTracking_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_wifiSlowPathRxbyNic_get(struct seq_file *s, void *v);
int rtk_fc_proc_wifiSlowPathRxbyNic_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_prevent_control_packet_drop_get(struct seq_file *s, void *v);
int rtk_fc_proc_prevent_control_packet_drop_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_wifi_flow_ctrl_threshold_get(struct seq_file *s, void *v);
int rtk_fc_proc_wlan0_flow_ctrl_on_threshold_mbps_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_wlan0_flow_ctrl_off_threshold_mbps_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_wlan1_flow_ctrl_on_threshold_mbps_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_wlan1_flow_ctrl_off_threshold_mbps_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_wifi_tx_qos_mapping_get(struct seq_file *s, void *v);
int rtk_fc_proc_wifi_tx_qos_mapping_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_stagTPID_get(struct seq_file *s, void *v);
int rtk_fc_proc_stagTPID_set(struct file *filp, const char *buff,unsigned long len, void *data );

int rtk_fc_proc_remarkrule_get(struct seq_file *s, void *v);

int rtk_fc_proc_fb_meter_get(struct seq_file *s, void *v);
int rtk_fc_proc_fb_meter_set(struct file *file, const char *buffer, unsigned long count, void *data);

int rtk_fc_proc_hwnat_mode_selection_set(struct file *filp, const char *buff, unsigned long count, void *data);
int rtk_fc_proc_hwnat_mode_selection_get(struct seq_file *s, void *v);

int rtk_fc_proc_shortcut_flow_count_get(struct seq_file *s, void *v);
int rtk_fc_proc_shortcut_flow_count_set(struct file *filp, const char *buff, unsigned long count, void *data);

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
int rtk_fc_proc_flow_miss_meter_get(struct seq_file *s, void *v);
int rtk_fc_proc_flow_miss_meter_set(struct file *filp, const char *buff,unsigned long count, void *data);
int rtk_fc_proc_wan_ingress_shaper_get(struct seq_file *s, void *v);
int rtk_fc_proc_wan_ingress_shaper_set(struct file *filp, const char *buff,unsigned long count, void *data);
#endif

/* swdump*/
int rtk_fc_proc_flow_natloopback(struct seq_file *s, void *v);


/* trace */
int rtk_fc_proc_debug_level_write(struct file *file, const char *buffer, unsigned long count, void *data);
int rtk_fc_proc_debug_level_read(struct seq_file *s, void *data);
int rtk_fc_proc_filter_level_read(struct seq_file *s, void *v);
int rtk_fc_proc_filter_level_write(struct file *file, const char *buffer, unsigned long count, void *data);
int rtk_fc_proc_traceFilter_read(struct seq_file *s, void *data);
int rtk_fc_proc_traceFilter_write(struct file *file, const char *buffer, unsigned long count, void *data);
int tracefilterRULE0_dump_hs_timer( struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_debug_message_display_to_tty_enable_read(struct seq_file *s, void *v);
int rtk_fc_proc_debug_message_display_to_tty_enable_write(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_manager_debug_message_read(struct seq_file *s, void *v);
int rtk_fc_proc_manager_debug_message_write(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_flow_delete_track_read(struct seq_file *s, void *v);
int rtk_fc_proc_flow_delete_track_write(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_hash_test(struct seq_file *s, void *v);

#if defined(CONFIG_SMP)
int rtk_fc_proc_smp_dispatch_nic_rx_get(struct seq_file *s, void *v);
int rtk_fc_proc_smp_dispatch_nic_rx_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_smp_dispatch_nic_tx_get(struct seq_file *s, void *v);
int rtk_fc_proc_smp_dispatch_nic_tx_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_smp_dispatch_wifi_rx_get(struct seq_file *s, void *v);
int rtk_fc_proc_smp_dispatch_wifi_rx_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_smp_dispatch_wifi_tx_get(struct seq_file *s, void *v);
int rtk_fc_proc_smp_dispatch_wifi_tx_set(struct file *filp, const char *buff,unsigned long len, void *data );

#endif
#if defined(CONFIG_FC_RTL9607C_SERIES)
int _rtk_fc_vxlan_acceleration_mechanism_get(struct seq_file *s, void *v);
int _rtk_fc_vxlan_acceleration_mechanism_proc_set(struct file *filp, const char *buff,unsigned long len, void *data );
int _rtk_fc_nptv6_acceleration_mechanism_get(struct seq_file *s, void *v);
int _rtk_fc_nptv6_acceleration_mechanism_proc_set(struct file *filp, const char *buff,unsigned long len, void *data );
int _rtk_fc_vxlan_acceleration_extraPmsk_get(struct seq_file *s, void *v);
int _rtk_fc_vxlan_acceleration_extraPmsk_proc_set(struct file *filp, const char *buff,unsigned long len, void *data );
int _rtk_fc_nptv6_acc_mechanism_lanMeter_get(struct seq_file *s, void *v);
int _rtk_fc_nptv6_acc_mechanism_lanMeter_set(struct file *filp, const char *buff,unsigned long len, void *data );

#endif
int rtk_fc_proc_flow_delay_get(struct seq_file *s, void *v);
int rtk_fc_proc_flow_delay_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_ethertype_bypass_get(struct seq_file *s, void *v);
int rtk_fc_proc_ethertype_bypass_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_sw_shaperMib_update_mode_flow_get(struct seq_file *s, void *v);
int rtk_fc_proc_sw_shaperMib_update_mode_flow_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_sw_shaperMib_update_mode_host_get(struct seq_file *s, void *v);
int rtk_fc_proc_sw_shaperMib_update_mode_host_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_sw_ackDelay_ctrl_get(struct seq_file *s, void *v);
int rtk_fc_proc_sw_ackDelay_ctrl_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_flow_swFwded_aclTrapPri_mask_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_flow_swFwded_aclTrapPri_mask_get(struct seq_file *s, void *v);
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
int rtk_fc_proc_dynamic_prehashPattern_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_dynamic_prehashPattern_get(struct seq_file *s, void *v);
#endif
int rtk_fc_proc_house_keeping_flow_delete_immediatly_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_house_keeping_flow_delete_immediatly_get(struct seq_file *s, void *v);

int rtk_fc_proc_l2_port_moving_check_with_vlan_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_l2_port_moving_check_with_vlan_get(struct seq_file *s, void *v);

int rtk_fc_proc_disable_port_port_moving_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_disable_port_port_moving_get(struct seq_file *s, void *v);


int rtk_fc_proc_external_switch_info_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_external_switch_info_get(struct seq_file *s, void *v);





/* multicast control */
int rtk_fc_proc_mc_set_write(struct file *file, const char *buffer, unsigned long count, void *data);
int rtk_fc_proc_mc_set_get(struct seq_file *s, void *v);
int rtk_fc_proc_mcSetMode_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_mcSetMode_get(struct seq_file *s, void *v);
int rtk_fc_proc_igmp_unknown_flood_get(struct seq_file *s, void *v);
int rtk_fc_proc_igmp_unknown_flood_set(struct file *filp, const char *buff,unsigned long len, void *data);
#if defined(CONFIG_RTK_FC_IPSEC_FASTFWD)
int rtk_fc_proc_ipsec_hook_table_get(struct seq_file *s, void *v);
int rtk_fc_proc_ipsec_hook_table_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_ipsec_ipi_info_get(struct seq_file *s, void *v);
#endif
//int rtk_fc_proc_ipsec_hash_padding_table_get(struct seq_file *s, void *v);

int rtk_fc_proc_devGwMac_get(struct seq_file *s, void *v);
int rtk_fc_proc_devGwMac_set(struct file *filp, const char *buff,unsigned long len, void *data);

int rtk_fc_proc_version_get(struct seq_file *s, void *v);
int rtk_fc_proc_wlan_devmap_dump(struct seq_file *s, void *v);
int rtk_fc_proc_wlan_devmap_set(struct file *filp, const char *buff,unsigned long len, void *data);
int rtk_fc_proc_gmac_auto_sel_get(struct seq_file *s, void *v);
int rtk_fc_proc_gmac_auto_sel_set(struct file *filp, const char *buff,unsigned long len, void *data);
int32 rtk_fc_smpStatistic_get(struct seq_file *s, void *v);
int rtk_fc_smpStatistic_set(struct file *filp, const char *buff,unsigned long len, void *data);

/* reset */
int rtk_fc_proc_netif_mib_reset(struct file *file, const char *buffer, unsigned long count, void *data);
int rtk_fc_proc_hw_flow_mib_reset(struct file *file, const char *buffer, unsigned long count, void *data);
int rtk_fc_proc_sw_flow_mib_reset(struct file *file, const char *buffer, unsigned long count, void *data);
int rtk_fc_proc_wan_access_limit_IP_list_reset(struct file *filp, const char *buff,unsigned long len, void *data);

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
int rtk_fc_wifi_flow_ctrl_onOff_check(void);
#endif

#if defined(CONFIG_FC_RTL8198F_SERIES)
int rtk_fc_proc_dsliteUdpFrag_get(struct seq_file *s, void *v);
int rtk_fc_proc_dsliteUdpFrag_set(struct file *filp, const char *buff,unsigned long len, void *data);
#endif
#if defined(CONFIG_RTK_SOC_RTL8198D)
int rtk_fc_proc_user_group_get(struct seq_file *s, void *v);
int rtk_fc_proc_user_group_set(struct file *file, const char *buffer, unsigned long count, void *data);

int rtk_fc_proc_flow_limit_set(struct file *file, const char *buffer, unsigned long count, void *data);
#endif
#if defined(CONFIG_RTK_SOC_RTL8198D) || defined(CONFIG_RTL8198XB_SERIES)
int rtk_fc_proc_ext_flow_mib_set(struct file *file, const char *buffer, unsigned long count, void *data);
#endif

int rtk_fc_proc_ip_frag_shortcut_get(struct seq_file *s, void *v);
int rtk_fc_proc_ip_frag_shortcut_set(struct file *filp, const char *buff,unsigned long len, void *data );
int rtk_fc_proc_pe_crypto_test_set(struct file *filp, const char *buff, unsigned long count, void *data);
int rtk_fc_proc_pe_func_on_pe_cpu_num_get(struct seq_file *s, void *v);
int rtk_fc_proc_pe_func_on_pe_cpu_num_set(struct file *filp, const char *buff, unsigned long count, void *data);

