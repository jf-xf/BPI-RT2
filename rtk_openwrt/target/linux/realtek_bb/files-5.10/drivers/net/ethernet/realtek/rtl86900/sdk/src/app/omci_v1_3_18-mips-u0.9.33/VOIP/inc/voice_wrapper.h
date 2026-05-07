/*
 * Copyright (C) 2014 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : Definition of OMCI internal APIs
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1)
 */
#ifndef __VOICE_WRAPPER_H__
#define __VOICE_WRAPPER_H__

#ifdef  __cplusplus
extern "C" {
#endif


#include "gos_linux.h"
#include "omci_util.h"
#include "mib_table_defs.h"

typedef struct omci_voice_wrapper_s
{
	/* rtk proprietary action */
	GOS_ERROR_CODE   (*omci_voice_config_init)(void);
	GOS_ERROR_CODE   (*omci_voice_config_save)(void);
	GOS_ERROR_CODE   (*omci_voice_service_restart)(void);
    GOS_ERROR_CODE   (*omci_voice_config_reset)(void);
    /* set */
    GOS_ERROR_CODE   (*omci_voice_fax_mode_set)(UINT16, UINT8);
    GOS_ERROR_CODE   (*omci_voice_oob_dtmf_set)(UINT16, UINT8);
    GOS_ERROR_CODE   (*omci_voice_codec_sel_order_set)(UINT16, UINT16, UINT8, UINT8);
    GOS_ERROR_CODE   (*omci_voice_codec_sel_order_set_all)(UINT16, UINT8 *, UINT8);
    GOS_ERROR_CODE   (*omci_voice_min_local_port_set)(UINT16,UINT16);
    GOS_ERROR_CODE   (*omci_voice_dscp_mark_set)(UINT8);
    GOS_ERROR_CODE   (*omci_voice_echo_cancel_ind_set)(UINT16, UINT8);
    GOS_ERROR_CODE   (*omci_voice_user_part_aor_set)(UINT16, UINT32, CHAR *, UINT8, UINT32);
    GOS_ERROR_CODE   (*omci_voice_sip_display_name_set)(UINT16, UINT32, CHAR*, UINT32);
    GOS_ERROR_CODE   (*omci_voice_user_name_set)(UINT16, UINT32, CHAR*, CHAR*,UINT32);
    GOS_ERROR_CODE   (*omci_voice_password_set)(UINT16, UINT32, CHAR*, UINT32);
    GOS_ERROR_CODE   (*omci_voice_call_wait_feature_set)(UINT16, UINT8);
    GOS_ERROR_CODE   (*omci_voice_direct_connect_feature_set)(UINT16, UINT8);
    GOS_ERROR_CODE   (*omci_voice_proxy_server_ip_set)(UINT16, UINT32, CHAR *, UINT8, UINT32);
    GOS_ERROR_CODE   (*omci_voice_outbound_proxy_ip_set)(UINT16, UINT32, CHAR *, UINT8, UINT32);
    GOS_ERROR_CODE   (*omci_voice_sip_reg_exp_time_set)(UINT16, UINT32, UINT32);
    GOS_ERROR_CODE   (*omci_voice_reg_head_start_time_set)(UINT16, UINT32, UINT32);
    GOS_ERROR_CODE   (*omci_voice_proxy_tcp_udp_port_set)(UINT16, UINT32, UINT16);
    GOS_ERROR_CODE   (*omci_voice_proxy_domain_name_set)(UINT16, UINT32, CHAR *, UINT8, UINT32);
    GOS_ERROR_CODE   (*omci_voice_rtp_stat_clear)(UINT16);
    GOS_ERROR_CODE   (*omci_voice_CCPM_stat_clear)(UINT16);
    GOS_ERROR_CODE   (*omci_voice_SIPAGPM_stat_clear)(UINT16);
    GOS_ERROR_CODE   (*omci_voice_SIPCIPM_stat_clear)(UINT16);

    GOS_ERROR_CODE   (*omci_voice_pots_state_set)(UINT16, UINT8);
    GOS_ERROR_CODE   (*omci_voice_jitter_set)(UINT16, UINT32, UINT16);
    GOS_ERROR_CODE   (*omci_voice_hook_flash_time_set)(UINT16, UINT32, UINT16);
    GOS_ERROR_CODE   (*omci_voice_progress_or_transfer_feature_set)(UINT16, UINT16);
    GOS_ERROR_CODE   (*omci_voice_dial_plan_set)(UINT16, UINT8 *, UINT32);
    GOS_ERROR_CODE   (*omci_voice_config_method_set)(UINT8 );	
    /* get */
    GOS_ERROR_CODE   (*omci_voice_hook_state_get)(UINT16,UINT8 *);
    GOS_ERROR_CODE   (*omci_voice_VOIP_line_status_get)(omci_VoIP_line_status_t *);
    GOS_ERROR_CODE   (*omci_voice_CCPM_history_data_get)(omci_CCPM_history_data_t *);
    GOS_ERROR_CODE   (*omci_voice_RTPPM_history_data_get)(omci_RTPPM_history_data_t *);
    GOS_ERROR_CODE   (*omci_voice_SIPAGPM_history_data_get)(omci_SIPAGPM_history_data_t *);
    GOS_ERROR_CODE   (*omci_voice_SIPCIPM_history_data_get)(omci_SIPCIPM_history_data_t *);

} omci_voice_wrapper_t;

#ifdef  __cplusplus
}
#endif

#endif

