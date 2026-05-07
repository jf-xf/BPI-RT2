/*
 * Copyright (C) 2018 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: $
 * $Date: $
 *
 * Purpose : Definition of GPON Global API
 *
 * Feature : The file have include the following module and sub-modules
 *           (1) Initialization
 *           (2) GPON parameter settings
 *           (3) Data path configuration.
 *           (4) Counter information
 *
 */
#ifndef __RT_GPON_H__
#define __RT_GPON_H__

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/rt_type.h>
#include <hal/chipdef/chip.h>


/*
 * Symbol Definition                                            
 */

#define RT_GPON_PON_QUEUE_PIR_UNLIMITED (0xFFFFFFFF)
#define RT_GPON_OMCI_MIC_DIR_DS 0x01
#define RT_GPON_OMCI_MIC_DIR_US 0x02

#define RT_GPON_OMCC_TCONT_ID (0xFFFF)


/*
 * GPON MAC Serial Number Structure.
 */
#define RT_GPON_VENDOR_ID_LEN           4
#define RT_GPON_VENDOR_SPECIFIC_LEN     4

typedef struct rt_gpon_serialNumber_s{
    uint8 vendor[RT_GPON_VENDOR_ID_LEN];         /*verdor ID*/
    uint8 specific[RT_GPON_VENDOR_SPECIFIC_LEN]; /*verdor specific*/
}rt_gpon_serialNumber_t;

/*
 * GPON MAC Registration ID Structure.
 */
#define RT_GPON_PASSWORD_LEN           10     /*G.984*/
#define RT_GPON_REGISTRATION_ID_LEN    36     /*G.987 and G.989*/

typedef struct rt_gpon_registrationId_s{
    uint8 regId[RT_GPON_REGISTRATION_ID_LEN]; /* registraration id in 987.3 or password in 984.3 */
}rt_gpon_registrationId_t;

/*
 * GPON MAC FSM State.
 */
/* for G.984 only, G.987, G.989 merged O2 and O3 into O2_3 */
#define RT_GPON_ONU_STATES_O1  RT_NGP2_ONU_STATE_O1_1
#define RT_GPON_ONU_STATES_O2  RT_NGP2_ONU_STATE_O1_2
#define RT_GPON_ONU_STATES_O3  RT_NGP2_ONU_STATE_O2_3
#define RT_GPON_ONU_STATES_O4  RT_NGP2_ONU_STATE_O4
#define RT_GPON_ONU_STATES_O5  RT_NGP2_ONU_STATE_O5_1
#define RT_GPON_ONU_STATES_O6  RT_NGP2_ONU_STATE_O6
#define RT_GPON_ONU_STATES_O7  RT_NGP2_ONU_STATE_O7

typedef enum rt_gpon_onuState_e{
    RT_NGP2_ONU_STATE_O1_1      = 0,  /* RT_GPON_ONU_STATES_O1 */
    RT_NGP2_ONU_STATE_O1_2      = 1,  /* RT_GPON_ONU_STATES_O2 */
    RT_NGP2_ONU_STATE_O2_3      = 2,  /* RT_GPON_ONU_STATES_O3 */
    RT_NGP2_ONU_STATE_O4        = 3,  /* RT_GPON_ONU_STATES_O4 */
    RT_NGP2_ONU_STATE_O5_1      = 4,  /* RT_GPON_ONU_STATES_O5 */
    RT_NGP2_ONU_STATE_O5_2      = 5,  /* NG-PON2 only */
    RT_NGP2_ONU_STATE_O6        = 6,  /* RT_GPON_ONU_STATES_O6 */
    RT_NGP2_ONU_STATE_O7        = 7,  /* RT_GPON_ONU_STATES_O7 */
    RT_NGP2_ONU_STATE_O8_1      = 8,  /* NG-PON2 only */
    RT_NGP2_ONU_STATE_O8_2      = 9,  /* NG-PON2 only */
    RT_NGP2_ONU_STATE_O9        = 10, /* NG-PON2 only */
    RT_NGP2_ONU_STATE_END
} rt_gpon_onuState_t;

/*
 * GPON MAC Initial FSM State.
 */
typedef enum rt_gpon_initialState_e{
    RT_GPON_ONU_INIT_STATE_O1 = 0,          /* GPON ONU FSM start from Initial(O1) state */
    RT_GPON_ONU_INIT_STATE_O7 = 1,          /* GPON ONU FSM start from Emergency Stop(O7) state */
}rt_gpon_initialState_t;


/*
 * GPON MAC Flow Structure
 */
typedef struct rt_gpon_usFlow_s{
    uint32                      gemPortId; /* gem port */
    uint32                      tcontId;   /* logical tcont ID */
    uint32                      tcQueueId; /* logical tcont queue ID */
    rt_enable_t                 aesEn;     /* aes encryption state, only valid in xgpon/xgspon/ngpon2 */
}rt_gpon_usFlow_t;

typedef struct rt_gpon_dsFlow_s{
    uint32                      gemPortId; /* gem port */
    rt_enable_t                 aesEn;     /* aes encryption state */
    rt_enable_t                 isMcGem;   /* multicast gem, only valid in xgpon/xgspon/ngpon2 */
}rt_gpon_dsFlow_t;

typedef enum rt_gpon_tcont_scheduler_mode_e{
    RT_GPON_TCONT_QUEUE_SCHEDULER_SP = 0, /* Strict Priority */
    RT_GPON_TCONT_QUEUE_SCHEDULER_WRR = 1, /* Weighted Round Robin */
    RT_GPON_TCONT_QUEUE_SCHEDULER_SP_WRR = 2 /* SP + WRR */
} rt_gpon_tcont_scheduler_mode_t;

typedef struct rt_gpon_queueCfg_s{
    rt_gpon_tcont_scheduler_mode_t      scheduleType;
    uint32                              cir; //unit: 8kbps
    uint32                              pir; //unit: 8kbps
    uint32                              weight; /* weight value of WRR mode */
}rt_gpon_queueCfg_t;

typedef struct rt_gpon_schedule_info_s{
    uint32      max_tcont;       /* maximum tcont number */
    uint32      max_tcon_queue;  /* maximum tcont queue number */
    uint32      max_flow;        /* maximum flow number */
    uint32      max_pon_queue;   /* maximum pon queue number */
}rt_gpon_schedule_info_t;

/*
 * GPON MAC D/S Flow level Performance Counter
 */
typedef struct rt_gpon_dsFlow_gem_counter_s{
    uint32 gem_block;                   /* D/S GEM Block Counter */
    uint64 gem_byte;                    /* D/S GEM Byte Counter */
}rt_gpon_dsFlow_gem_counter_t;

typedef struct rt_gpon_dsFlow_eth_counter_s{
    uint32 eth_pkt_rx;                  /* D/S Ethernet Packet Counter */
    uint32 eth_pkt_fwd;                 /* D/S Forward Ethernet Packet Counter */
}rt_gpon_dsFlow_eth_counter_t;

/*
 * GPON MAC U/S Flow level Performance Counter
 */
typedef struct rt_gpon_usFlow_gem_counter_s{
    uint32 gem_block;                   /* U/S GEM Block Counter */
    uint64 gem_byte;                    /* U/S GEM Byte Counter */
}rt_gpon_usFlow_gem_counter_t;

typedef struct rt_gpon_usFlow_eth_counter_s{
    uint32 eth_cnt;                    /* U/S GEM Byte Counter */
}rt_gpon_usFlow_eth_counter_t;

/*
 * GPON MAC Flow Performance Type
 */
typedef enum rt_gpon_flow_counter_type_e{
    RT_GPON_CNT_TYPE_FLOW_DS_GEM,         /* D/S Flow GEM Statistics */
    RT_GPON_CNT_TYPE_FLOW_DS_ETH,         /* D/S Flow ETH Statistics */
    RT_GPON_CNT_TYPE_FLOW_US_GEM,         /* U/S Flow GEM Statistics */
    RT_GPON_CNT_TYPE_FLOW_US_ETH,         /* U/S Flow ETH Statistics */
    RT_GPON_CNT_TYPE_FLOW_MAX
}rt_gpon_flow_counter_type_t;

/*
 * Flow Performance Counter
 */
typedef union rt_gpon_flow_counter_u{
    rt_gpon_dsFlow_gem_counter_t dsgem; /* D/S flow gem counter */
    rt_gpon_dsFlow_eth_counter_t dseth; /* D/S flow eth counter */
    rt_gpon_usFlow_gem_counter_t usgem; /* U/S flow gem counter */
    rt_gpon_usFlow_eth_counter_t useth; /* U/S flow eth counter */
}rt_gpon_flow_counter_t;

/*
 * GPON MAC Performance Type
 */
typedef enum rt_gpon_pm_type_e{
    RT_GPON_PM_TYPE_PHY_LOS,            /*PHY/LODS PM counters (989.3 table 14-1, VZOpenOMCI:7.3.1)*/
    RT_GPON_PM_TYPE_FEC,                /*FEC PM counters (988 9.2.22, 989.3 table 14-1, VZOpenOMCI:7.3.2)*/
    RT_GPON_PM_TYPE_XGEM,               /*XGEM PM counters (989.3 table 14-1, VZOpenOMCI: 7.3.3)*/
    RT_GPON_PM_TYPE_PLOAM_PM1,          /*PLOAM1 PM counters (989.3 table 14-1, VZOpenOMCI: 7.3.4)*/
    RT_GPON_PM_TYPE_PLOAM_PM2,          /*PLOAM2 PM counters (989.3 table 14-1, VZOpenOMCI: 7.3.5)*/
    RT_GPON_PM_TYPE_PLOAM_PM3,          /*PLOAM3 PM counters (989.3 table 14-1, VZOpenOMCI: 7.3.6)*/
    RT_GPON_PM_TYPE_CHAN_TUNING_PM1,    /*TUNING1 PM counters (989.3 table 14-1, VZOpenOMCI: 7.3.7)*/
    RT_GPON_PM_TYPE_CHAN_TUNING_PM2,    /*TUNING2 PM counters (989.3 table 14-1, VZOpenOMCI: 7.3.8)*/
    RT_GPON_PM_TYPE_CHAN_TUNING_PM3,    /*TUNING3 PM counters (989.3 table 14-1, VZOpenOMCI: 7.3.9)*/
    RT_GPON_PM_TYPE_OMCI,               /*OMCI PM counters (989.3 table 14-1, VZOpenOMCI: 7.3.10)*/
    RT_GPON_PM_TYPE_MAX
}rt_gpon_pm_type_t;

/*PHY LODS PM counters*/
typedef struct rt_gpon_pm_phy_los_s{
    uint64 total_words_protected_by_bip;                /* total BIP codewords(D/S frames) counter */
    uint32 bip_error_count;                             /* BIP error counter */ 
    uint32 psbd_hec_errors_corrected;                   /* PSBD HEC corrected counter */
    uint32 psbd_hec_errors_uncorrectable;               /* PSBD HEC uncorrectable error counter */
    uint32 fs_header_errors_corrected;                  /* FS header corrected counter */
    uint32 fs_header_errors_uncorrectable;              /* FS header uncorrectable error counter */
    uint32 total_lods_event_count;                  /* The count of the state transitions from O5.1/O5.2 to O6, referring to the ONU activation cycle state machine */
    uint32 lods_restored_in_operating_twdm_ch;         /* The count of LODS events cleared automatically without retuning */
    uint32 lods_restored_in_protection_twdm_ch;        /* The count of LODS events resolved by retuning to a pre-configured protection TWDM channel */
    uint32 lods_restored_in_discretionary_twdm_ch;      /* The count of LODS events resolved by retuning to a TWDM channel chosen by the ONU, without retuning */
    uint32 lods_reactivations;                  /* The count of LODS events resolved through ONU reactivation; that is, either TO2 (without WLCP) or TO3 + TO4 (with WLCP) expires before the downstream channel is reacquired */
    uint32 lods_handshake_failure_in_protection_ch;      /* The count of LODS events resolved through ONU reactivation after attempted protection switching, which turns unsuccessful due to a handshake failure. */
    uint32 lods_handshake_failure_in_discretionary_ch;    /* The count of LODS events resolved through ONU reactivation after attempted retuning to a discretionary channel, which turns unsuccessful due to a handshake failure */
}rt_gpon_pm_phy_los_t;

/*FEC PM counters*/
typedef struct rt_gpon_pm_counter_fec_s{
    uint32 corrected_fec_bytes;         /* FEC corrected bytes */
    uint32 corrected_fec_codewords;     /* FEC corrected codewords */
    uint32 uncorrected_fec_codewords;   /* FEC uncorrectable codewords */
    uint32 total_fec_codewords;         /* FEC total codewords */
    uint32 fec_seconds;                 /* FEC seconds */
} rt_gpon_pm_counter_fec_t;


/*XGEM PM counters*/
typedef struct rt_gpon_pm_counter_xgem_s{
    uint32 total_transmitted_xgem_frames;                   /* total trasmitted XGEM frames */
    uint32 transmitted_xgem_frames_with_lf;                 /* trasmitted XGEM frames with last flag */
    uint32 total_received_xgem_frames;                      /* total received XGEM frames */
    uint32 received_xgem_header_hec_errors;                 /* received XGEM header HEC errors */
    uint32 fs_words_lost_to_xgem_header_hec_errors;         /* FS words lost due to xgem header HEC error */
    uint32 xgem_encryption_key_errors;                      /* xgem encryption key errors */
    uint32 total_transmitted_bytes_in_non_idle_xgem_frames; /* total transmitted bytes in non idle xgem frames */
    uint32 total_received_bytes_in_non_idle_xgem_frames;    /* total received bytes in non idle xgem frames */
} rt_gpon_pm_counter_xgem_t;

/*PLOAM PM counters - Part1*/
typedef struct rt_gpon_pm_counter_ploam1_s{
    uint32 ploam_mic_errors;                    /* ploam MIC errors */
    uint32 downstream_ploam_message_count;      /* D/S ploam message count */
    uint32 ranging_time_message_count;          /* ranging_time ploam message count */
    uint32 protection_control_message_count;    /* protection_control ploam message count */
    uint32 adjust_tx_wavelength_message_count;  /* adjust_tx_wavelength ploam message count */
    uint32 adjust_tx_wavelength_amplitude;      /* adjust_rx_wavelength ploam message count */
} rt_gpon_pm_counter_ploam1_t;

/*PLOAM PM counters - Part2*/
typedef struct rt_gpon_pm_counter_ploam2_s{
    uint32 system_profile_message_count;                /* system_profile ploam message count */
    uint32 channel_profile_message_count;               /* channel_profile ploam message count */
    uint32 burst_profile_message_count;                 /* burst_profile ploam message count */
    uint32 assign_onu_id_message_count;                 /* assign_onu_id ploam message count */
    uint32 unsatisfied_adjust_tx_wavelength_requests;   /* unsatisfied adjust_tx_wavelength request count */
    uint32 deactivate_onu_id_message_count;             /* deactivate ploam message count */
    uint32 disable_serial_number_message_count;         /* disable_sn ploam message count */
    uint32 request_registration_message_count;          /* request_registration ploam message count */
    uint32 assign_alloc_id_message_count;               /* assign_alloc_id ploam message count */
    uint32 key_control_message_count;                   /* key_control ploam message count */
    uint32 sleep_allow_message_count;                   /* sleep_allow ploam message count */
    uint32 enctyped_port_id_message_count;              /* encrypted_port ploam message count */
    uint32 no_message_count;                            /* no_message ploam message count */
    uint32 popup_message_count;                         /* popup ploam message count */
    uint32 request_key_message_count;                   /* request_key ploam message count */
    uint32 config_port_id_message_count;                /* configure_port ploam message count */
    uint32 pee_message_count;                           /* pee ploam message count */
    uint32 change_power_level_message_count;            /* change_power_level ploam message count */
    uint32 pst_message_count;                           /* pst ploam message count */
    uint32 ber_interval_message_count;                  /* ber_interval ploam message count */
    uint32 key_switch_time_message_count;               /* key_switch_time ploam message count */
    uint32 extend_burst_len_message_count;              /* extend_burst_len ploam message count */
}rt_gpon_pm_counter_ploam2_t;

/*PLOAM PM counters - Part3*/
typedef struct rt_gpon_pm_counter_ploam3_s{
    uint32 upstream_ploam_message_count;                    /* U/S ploam message count */
    uint32 serial_number_onu_in_band_message_count;         /* inband sn ploam message count */
    uint32 serial_number_onu_amcc_message_count;            /* amcc sn ploam message count */
    uint32 registration_message_count;                      /* registration ploam message count */
    uint32 acknowledgement_message_count;                   /* ack ploam message count */
    uint32 sleep_request_message_count;                     /* sleep_request ploam message count */
    uint32 tuning_response_ack_nack_msg_count;              /* tuning_response ploam message with ack/nack opcode */
    uint32 tuning_response_complete_u_rollback_msg_count;   /* tuning_response ploam message with complete_u/rollback opcode */
    uint32 power_consumption_report_message_count;          /* power_consumption_report ploam message count */
    uint32 change_power_level_parameter_error_count;        /* change power level failed count */
    uint32 dying_gasp_message_count;                        /* dying_gasp ploam message count */
    uint32 no_message_count;                                /* no_message ploam message count */
    uint32 encrypt_key_message_count;                       /* encrypt_key ploam message count */
    uint32 pee_message_count;                               /* pee ploam message count */
    uint32 pst_message_count;                               /* pst ploam message count */
    uint32 rei_message_count;                               /* rei ploam message count */
}rt_gpon_pm_counter_ploam3_t;

/*TUNING PM counters - Part1*/
typedef struct rt_gpon_pm_counter_chan_tuning1_s{
    uint32 tuning_control_requests_for_rx_only_or_rx_tx;  /* G.988#9.16.8, The counter of received Tuning_Control PLOAM messages with Request operation code that contain tuning instructions either for receiver only or for both receiver and transmitter */
    uint32 tuning_control_requests_for_tx_only;  /* G.988#9.16.8, The counter of received Tuning_Control PLOAM messages with Request operation code that contain tuning instructions for transmitter only */
    uint32 tuning_control_requests_rejected_int_spc;  /* G.988#9.16.8, The counter of transmitted Tuning_Response PLOAM messages with NACK operation code and INT_SFC response code */
    uint32 tuning_control_requests_rejected_ds_all;  /* G.988#9.16.8, The aggregate counter of transmitted Tuning_Response PLOAM messages with NACK operation code and any DS_xxx response code */
    uint32 tuning_control_requests_rejected_us_all;  /* G.988#9.16.8, The aggregate counter of transmitted Tuning_Response PLOAM messages with NACK operation code and any US_xxx response code */
    uint32 tuning_control_requests_ok_target_channel;  /* G.988#9.16.8, The counter of controlled tuning attempts for which an upstream tuning confirmation has been obtained in the target channel */
    uint32 tuning_control_requests_failed_to4_exp;  /* G.988#9.16.8, The counter of controlled tuning attempts that failed due to timer TO4 expiration in the DS Tuning state (O8) in the target channel */
    uint32 tuning_control_requests_failed_to5_exp;  /* G.988#9.16.8, The counter of controlled tuning attempts that failed due to timer TO5 expiration in the US Tuning state (O9) in the target channel */
    uint32 tuning_control_requests_resolved_discret_ch;  /* G.988#9.16.8, The counter of controlled tuning attempts for which an upstream tuning confirmation has been obtained in the discretionary channel */
    uint32 tuning_control_requests_rollback_com_ds;  /* G.988#9.16.8, The counter of controlled tuning attempts that failed due to communication condition in the target channel */
    uint32 tuning_control_requests_rollback_ds_all;  /* G.988#9.16.8, The aggregate counter of controlled tuning attempts that failed due to target downstream wavelength channel inconsistency */
    uint32 tuning_control_requests_rollback_us_all;  /* G.988#9.16.8, The aggregate counter of controlled tuning attempts that failed due to target upstream wavelength channel parameter violation */
    uint32 tuning_control_requests_failed_reactivation;  /* G.988#9.16.8, The counter of controlled tuning attempts that failed on any reason, with expiration of timers TO4 or TO5 causing the ONU transition into state O1 */
}rt_gpon_pm_counter_chan_tuning1_t;

/*TUNING PM counters - Part2*/
typedef struct rt_gpon_pm_counter_chan_tuning2_s{
    uint32 tuning_control_requests_rejected_ds_albl;  /* G.988#9.16.9, The counter of transmitted Tuning_Response PLOAM messages with NACK operation code and DS_ALBL response code */
    uint32 tuning_control_requests_rejected_ds_void;  /* G.988#9.16.9, The counter of transmitted Tuning_Response PLOAM messages with NACK operation code and DS_VOID response code */
    uint32 tuning_control_requests_rejected_ds_part;  /* G.988#9.16.9, The counter of transmitted Tuning_Response PLOAM messages with NACK operation code and DS_PART response code */
    uint32 tuning_control_requests_rejected_ds_tunr;  /* G.988#9.16.9, The counter of transmitted Tuning_Response PLOAM messages with NACK operation code and DS_TUNR response code */
    uint32 tuning_control_requests_rejected_ds_lnrt;  /* G.988#9.16.9, The counter of transmitted Tuning_Response PLOAM messages with NACK operation code and DS_LNRT response code */
    uint32 tuning_control_requests_rejected_ds_lncd;  /* G.988#9.16.9, The counter of transmitted Tuning_Response PLOAM messages with NACK operation code and DS_LNCD response code */
    uint32 tuning_control_requests_rejected_us_albl;  /* G.988#9.16.9, The counter of transmitted Tuning_Response PLOAM messages with NACK operation code and US_ALBL response code */
    uint32 tuning_control_requests_rejected_us_void;  /* G.988#9.16.9, The counter of transmitted Tuning_Response PLOAM messages with NACK operation code and US_VOID response code */
    uint32 tuning_control_requests_rejected_us_tunr;  /* G.988#9.16.9, The counter of transmitted Tuning_Response PLOAM messages with NACK operation code and US_TUNR response code */
    uint32 tuning_control_requests_rejected_us_clbr;  /* G.988#9.16.9, The counter of transmitted Tuning_Response PLOAM messages with NACK operation code and US_CLBR response code */
    uint32 tuning_control_requests_rejected_us_lktp;  /* G.988#9.16.9, The counter of transmitted Tuning_Response PLOAM messages with NACK operation code and US_LKTP response code */
    uint32 tuning_control_requests_rejected_us_lnrt;  /* G.988#9.16.9, The counter of transmitted Tuning_Response PLOAM messages with NACK operation code and US_LNRT response code */
    uint32 tuning_control_requests_rejected_us_lncd;  /* G.988#9.16.9, The counter of transmitted Tuning_Response PLOAM messages with NACK operation code and US_LNCD response code */
}rt_gpon_pm_counter_chan_tuning2_t;

/*TUNING PM counters - Part3*/
typedef struct rt_gpon_pm_counter_chan_tuning3_s{
    uint32 tuning_control_requests_rollback_ds_albl;  /* G.988#9.16.10, The counter of controlled tuning attempts that failed due to downstream administrative label inconsistency */
    uint32 tuning_control_requests_rollback_ds_lktp;  /* G.988#9.16.10, The counter of controlled tuning attempts that failed due to downstream optical link type inconsistency */
    uint32 tuning_control_requests_rollback_us_albl;  /* G.988#9.16.10, The counter of controlled tuning attempts that failed due to upstream administrative label violation */
    uint32 tuning_control_requests_rollback_us_void;  /* G.988#9.16.10, The counter of controlled tuning attempts that failed due to the target upstream wavelength channel descriptor being void */
    uint32 tuning_control_requests_rollback_us_tunr;  /* G.988#9.16.10, The counter of controlled tuning attempts that failed due to the transmitter tuning range violation */
    uint32 tuning_control_requests_rollback_us_lktp;  /* G.988#9.16.10, The counter of controlled tuning attempts that failed due to the upstream optical link type violation */
    uint32 tuning_control_requests_rollback_us_lnrt;  /* G.988#9.16.10, The counter of controlled tuning attempts that failed due to the upstream line rate violation */
    uint32 tuning_control_requests_rollback_us_lncd;  /* G.988#9.16.10, The counter of controlled tuning attempts that failed due to the upstream line code violation */
}rt_gpon_pm_counter_chan_tuning3_t;

/*OMCI PM counters*/
typedef struct rt_gpon_pm_counter_omci_s{
    uint32 omci_tx_msg_count;       /* omci message tx count */
    uint32 omci_rx_msg_count;       /* omci message rx count */
    uint32 omci_rx_baseline_count;  /* omci baseline message rx count */
    uint32 omci_rx_extended_count;  /* omci extended message rx count */
    uint32 omci_rx_mic_error_count; /* omci message rx mic_error count */
}rt_gpon_pm_counter_omci_t;

/*
 * Global Performance Counter
 */
typedef union rt_gpon_pm_counter_u{
    rt_gpon_pm_phy_los_t phyLos;                    /* gpon physical los counter */
    rt_gpon_pm_counter_fec_t fec;                   /* gpon fec counter */
    rt_gpon_pm_counter_xgem_t xgem;                 /* gpon xgem counter */
    rt_gpon_pm_counter_ploam1_t ploam1;             /* gpon pm_ploam1 counter */
    rt_gpon_pm_counter_ploam2_t ploam2;             /* gpon pm_ploam2 counter */
    rt_gpon_pm_counter_ploam3_t ploam3;             /* gpon pm_ploam3 counter */
    rt_gpon_pm_counter_chan_tuning1_t chanTuning1;  /* gpon pm_tuning1 counter */
    rt_gpon_pm_counter_chan_tuning2_t chanTuning2;  /* gpon pm_tuning2 counter */
    rt_gpon_pm_counter_chan_tuning3_t chanTuning3;  /* gpon pm_tuning3 counter */
    rt_gpon_pm_counter_omci_t omci;                 /* gpon omci counter */
}rt_gpon_pm_counter_t;

/*TCONT PM counters*/
typedef struct rt_gpon_tcont_counter_s{
    uint32 tcont_id;                /* tcont ID */
    uint64 tcont_idle_byte_count;   /* tcont idle byte count */
    uint32 tcont_eth_pkt_count;     /* tcont eth packet count */
    uint32 tcont_gem_pkt_count;     /* tcont gem count */
}rt_gpon_tcont_counter_t;

/*
 * GPON MAC FEC Status Structure
 */
typedef struct rt_gpon_fec_status_s{
    uint32    ds_fec_status;    /* D/S FEC status */
    uint32    us_fec_status;    /* U/S FEC status */
}rt_gpon_fec_status_t;

/*
 * GPON PON TAG Structure.
 */
#define RT_GPON_PONTAG_LEN           8     /*G.984*/

typedef struct rt_gpon_ponTag_s{
    uint8 tagData[RT_GPON_PONTAG_LEN];  /* gpon pon tag */
}rt_gpon_ponTag_t;

/*
 * GPON MSK Structure.
 */
#define RT_GPON_MSK_LEN           16

typedef struct rt_gpon_msk_s{
    uint8 mskData[RT_GPON_MSK_LEN]; /* gpon master session key */
}rt_gpon_msk_t;

/*
 * GPON Multicast Key Structure.
 */
#define RT_GPON_MCKEY_LEN         16

typedef enum rt_gpon_mcKeyIdx_e{
    RT_GPON_MC_KEY_IDX0 = 1,          /* GPON Multicast Key Index #0 */
    RT_GPON_MC_KEY_IDX1 = 2,          /* GPON Multicast Key Index #1 */
    RT_GPON_MC_KEY_IDX_MAX
}rt_gpon_mcKeyIdx_t;

typedef enum rt_gpon_mcKeyAction_e{
    RT_GPON_MC_KEY_ACTION_SET_ENTRY = 0,          /* GPON Multicast Key ACTION: Set the specified entry */
    RT_GPON_MC_KEY_ACTION_CLEAR_ENTRY = 1,        /* GPON Multicast Key ACTION: Clear the specified entry */
    RT_GPON_MC_KEY_ACTION_CLEAR_TABLE = 2,        /* GPON Multicast Key ACTION: Clear the entire table */
    RT_GPON_MC_KEY_ACTION_MAX
}rt_gpon_mcKeyAction_t;

typedef struct rt_gpon_mc_key_s{
    uint32 idx;     
    uint32 action;
    uint8  key[RT_GPON_MCKEY_LEN];
}rt_gpon_mc_key_t;

#define RT_GPON_DELIMITER_LEN 3

typedef struct rt_gpon_gtc_info_s{
    uint8  guard_bits;                          /* guard time bits number */
    uint8  type1_bits;                          /* type1 preamble bits number */
    uint8  type2_bits;                          /* type2 preamble bits number */
    uint8  type3_bits;                          /* type3 preamble bits number */
    uint8  type3_ptn;                           /* type3 pattern */
    uint8  delimiter[RT_GPON_DELIMITER_LEN];    /* delimiter pattern */
    uint32 eqd;                                 /* equalization delay */
    uint8  powerLevel;                          /* power level */
    uint32 responTime; /*ns*/
}rt_gpon_gtc_info_t;

typedef enum rt_gpon_attribute_type_e{
    RT_GPON_ATTRIBUTE_TYPE_DBRU_STATUS,         /* DBRu status */
    RT_GPON_ATTRIBUTE_TYPE_GEM_BLOCK_LENGTH,    /* gem block length, only valid in gpon mode */
    RT_GPON_ATTRIBUTE_TYPE_SD_THRESHOLD,        /* Signal Degraded threshold */
    RT_GPON_ATTRIBUTE_TYPE_SF_THRESHOLD,        /* Signal Fail threshold */
	RT_GPON_ATTRIBUTE_TYPE_GTC_INFO,            /* GTC information */
    RT_GPON_ATTRIBUTE_TYPE_ONUID,               /* onu_id */
    RT_GPON_ATTRIBUTE_TYPE_ADMINSTATE,          /* admin state */
    RT_GPON_ATTRIBUTE_TYPE_MAX
}rt_gpon_attribute_type_t;

typedef union rt_gpon_attribute_u{
    uint8 dbruStatus;               /* DBRu status */
    uint32 gemBlockLength;          /* gem block length, only valid in gpon mode */
    uint32 sdThreshold;             /* Signal Degraded threshold */
    uint32 sfThreshold;             /* Signal Fail threshold */
	rt_gpon_gtc_info_t gtc_info;    /* GTC information */
    uint32 onuId;                   /* onu_id */
    rt_enable_t adminState;         /* admin state */
}rt_gpon_attribute_t;

typedef struct rt_gpon_omcc_s{
    uint32 allocId; /* omcc tcont alloc id */
    uint32 gemId;   /* omcc tcont gem port */
}rt_gpon_omcc_t;


/*OMCI packet format*/
typedef struct
{
    uint16    transId;      /* Transaction correlation id */
    uint8     msgType;      /* message type */
    uint8     devId;        /* device id */
    uint16    classId;      /* managed entity id */
    uint16    instId;       /* managed entity instance */
}rt_gpon_omci_pkt_hdr_t;

typedef struct
{
    rt_gpon_omci_pkt_hdr_t  hdr;    /* omci header */
    uint8                 pdu[32];  /* message contents */
    uint8                 rsvd[2];  /* reserved */
    uint16                sduLen;   /* The length of the CPCS-SDU field is set to 0x0028 */
    uint32                crc;      /* 32-bit CRC */
}rt_gpon_omci_base_pkt_t;

typedef struct
{
    rt_gpon_omci_pkt_hdr_t  hdr;    /* omci header of extended type */
    uint16                  len;    /* omci length of extended type */
}rt_gpon_omci_ext_pkt_t;



/*
 * OMCI RX callback function.
 */
typedef uint32 (*rt_gpon_omci_rx_callback)(uint32 msgLen,uint8 *pMsg);

/*
 * PLOAM RX callback function.
 */
typedef uint32 (*rt_gpon_ploam_rx_callback)(uint8 *pMsg, uint32 len);

/*
 * Function Declaration
 */

/* Module Name    : Gpon     */
/* Sub-module Name: Gpon parameter settings */

/* Function Name:
 *      rt_gpon_init
 * Description:
 *      Initialize GPON MAC.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 *      Module must be initialized before using all of APIs in this module.
 */
extern int32
rt_gpon_init(void);

/* Function Name:
 *      rt_gpon_serialNumber_set
 * Description:
 *      Set GPON serial number
 * Input:
 *      pSN   - pointer to the GPON serial number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      This API set the serial number of GPON MAC .
 */
extern int32
rt_gpon_serialNumber_set(rt_gpon_serialNumber_t *pSN);

/* Function Name:
 *      rt_gpon_serialNumber_get
 * Description:
 *      Get GPON serial number
 * Input:
 *      None
 * Output:
 *      pSN   - pointer to the GPON serial number
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      This API get the serial number of GPON MAC .
 */
extern int32
rt_gpon_serialNumber_get(rt_gpon_serialNumber_t *pSN);

/* Function Name:
 *      rt_gpon_registrationId_set
 * Description:
 *      Set GPON registration ID (password in 984.3).
 * Input:
 *      pRegId   - pointer to the GPON registration id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      This API get the registration id of GPON MAC .
 */
extern int32
rt_gpon_registrationId_set(rt_gpon_registrationId_t *pRegId);

/* Function Name:
 *      rt_gpon_registrationId_get
 * Description:
 *      Get GPON registration ID (password in 984.3).
 * Input:
 *      None
 * Output:
 *      pRegId   - pointer to the GPON registration id
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      The registration ID is the password in 984.3 .
 */
extern int32
rt_gpon_registrationId_get(rt_gpon_registrationId_t *pRegId);

/* Function Name:
 *      rt_gpon_activate
 * Description:
 *      GPON MAC Activating.
 * Input:
 *      initState   - the initial state when ONU active
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The GPON MAC is starting to work now.
 */
extern int32
rt_gpon_activate(rt_gpon_initialState_t initState);

/* Function Name:
 *      rt_gpon_deactivate
 * Description:
 *      GPON MAC Deactivate.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 * Note:
 *      The GPON MAC is out of work now.
*/
extern int32
rt_gpon_deactivate(void);

/* Function Name:
 *      rt_gpon_onuState_get
 * Description:
 *      Get the GPON ONU state.
 * Input:
 *      None
 * Output:
 *      pOnuState  - pointer to the ONU state
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      This API get the onu state of GPON MAC.
 */
extern int32
rt_gpon_onuState_get(rt_gpon_onuState_t *pOnuState);

/* Function Name:
 *      rt_gpon_omci_tx
 * Description:
 *      Transmit OMCI message.
 * Input:
 *      msgLen   - length of the OMCI message to be transmitted
 *      pMsg     - pointer to the OMCI message data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      This API is used for transmit OMCI message. 
 */
extern int32
rt_gpon_omci_tx(uint32 msgLen,uint8 *pMsg);

/* Function Name:
 *      rt_gpon_omci_rx_callback_register
 * Description:
 *      Register OMCI RX callback function.
 * Input:
 *      omciRx   - pointer to the callback function for OMCI RX
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      This API is used for receive OMCI message.  
 */
extern int32
rt_gpon_omci_rx_callback_register(rt_gpon_omci_rx_callback omciRx);

/* Function Name:
 *      rt_gpon_tcont_set
 * Description:
 *      Set TCONT by assigning an allocation id.
 * Input:
 *      allocId   - allocation id 
 * Output:
 *      pTcontId  - pointer to the logic TCONT id
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      Caller should keep the output pTcontId. 
 */
extern int32 
rt_gpon_tcont_set(uint32 *pTcontId, uint32 allocId);

/* Function Name:
 *      rt_gpon_tcont_get
 * Description:
 *      Get allocation id by assigned logic TCONT id.
 * Input:
 *      tcontId  -  logic TCONT id
 * Output:
 *      pAllocId   -  pointer to the allocation id 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      This API get the alloc id for the input tcont id.  
 */
extern int32 
rt_gpon_tcont_get(uint32 tcontId, uint32 *pAllocId);

/* Function Name:
 *      rt_gpon_tcont_del
 * Description:
 *      Delete the TCONT id
 * Input:
 *      tcontId   - TCONT id 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note: 
 *      This API del the tcont id.  
 */
extern int32 
rt_gpon_tcont_del(uint32 tcontId);

/* Function Name:
 *      rt_gpon_usFlow_set
 * Description:
 *      Set the upstream flow for gem port, tcont and queue id
 * Input:
 *      usFlowId  - logical flow id
 *      pUsFlow   - pointer to the upstream flow configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      This API is used for mapping flow to gem port, and tcont/queue.  
 */
extern int32 
rt_gpon_usFlow_set(uint32 usFlowId, rt_gpon_usFlow_t *pUsFlow);

/* Function Name:
 *      rt_gpon_usFlow_get
 * Description:
 *      Get the upstream flow for gem port, tcont and queue id
 * Input:
 *      usFlowId  - logical flow id
 * Output:
 *      pUsFlow   - pointer to the upstream flow configuration
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      This API get the mapping of flow and gem port, tcont/queue.  
 */
extern int32 
rt_gpon_usFlow_get(uint32 usFlowId, rt_gpon_usFlow_t *pUsFlow);

/* Function Name:
 *      rt_gpon_usFlow_del
 * Description:
 *      Delete the upstream flow
 * Input:
 *      usFlowId  - logical flow id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note: 
 *      This API delete the us flow mapping.  
 */
extern int32 
rt_gpon_usFlow_del(uint32 usFlowId);

/* Function Name:
 *      rt_gpon_usFlow_delAll
 * Description:
 *      Delete all the upstream flows
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 * Note: 
 *      This API is used for delete all us flow.  
 */
extern int32 
rt_gpon_usFlow_delAll(void);

/* Function Name:
 *      rt_gpon_dsFlow_set
 * Description:
 *      Set the downstream flow configuration
 * Input:
 *      dsFlowId  - logical flow id
 *      pDsFlow   - pointer to the downstream flow configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      This API set mapping of flow and ds gem, and set AES state.  
 */
extern int32 
rt_gpon_dsFlow_set(uint32 dsFlowId, rt_gpon_dsFlow_t *pDsFlow);

/* Function Name:
 *      rt_gpon_dsFlow_get
 * Description:
 *      Get the downstream flow configuration
 * Input:
 *      dsFlowId  - logical flow id
 * Output:
 *      pDsFlow   - pointer to the downstream flow configuration
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      This API get ds flow configuration.  
 */
extern int32 
rt_gpon_dsFlow_get(uint32 dsFlowId, rt_gpon_dsFlow_t *pDsFlow);

/* Function Name:
 *      rt_gpon_dsFlow_del
 * Description:
 *      Delete the downstream flow 
 * Input:
 *      dsFlowId  - logical flow id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note: 
 *      This API is used for delete ds flow.  
 */
extern int32 
rt_gpon_dsFlow_del(uint32 dsFlowId);

/* Function Name:
 *      rt_gpon_dsFlow_delAll
 * Description:
 *      Delete all the downstream flows
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 * Note: 
 *      This API is used for delete all ds flow 
 */
extern int32 
rt_gpon_dsFlow_delAll(void);

/* Function Name:
 *      rt_gpon_loop_gemport_set
 * Description:
 *      Set the downstream gem port for loopback function
 * Input:
 *      gemPortId - gem port Index
 *      enable - enable/disable loop function of specified GEM 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_DRIVER_NOT_FOUND     - chip not support 
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_INPUT                - Invalid input parameters.
 *      RT_ERR_ENTRY_NOTFOUND       - specified entry not found
 * Note: 
 *      The packet received from the loopback gem port will be
 *      loopback with the same gem port as upstream gem port.
 *      And the DA SA will be swapped.
 */
extern int32 
rt_gpon_loop_gemport_set(uint32 gemPortId, rt_enable_t enable);

/* Function Name:
 *      rt_gpon_loop_gemport_get
 * Description:
 *      Get the loopback function state of the downstream gem port
 * Input:
 *      gemPortId - gem port Index
 * Output:
 *      pEnable    - get Enable/disble loop of specified GEM
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_DRIVER_NOT_FOUND     - chip not support 
 *      RT_ERR_NOT_INIT             - The module is not initial
 *      RT_ERR_INPUT                - Invalid input parameters.
 *      RT_ERR_ENTRY_NOTFOUND       - specified entry not found
 * Note: 
 *      This API get loopback state for specific ds gem port 
 */
extern int32 
rt_gpon_loop_gemport_get(uint32 gemPortId, rt_enable_t *pEnable);


/* Function Name:
 *      rt_gpon_ponQueue_set
 * Description:
 *      Set the pon queue
 * Input:
 *      tcontId    - logical tcont index
 *      tcQueueId  - logical tcont queue id
 *      pQueuecfg - pointer to the queue configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      Support RT_GPON_TCONT_QUEUE_SCHEDULER_SP or
 *      RT_GPON_TCONT_QUEUE_SCHEDULER_WRR.
 *      CIR and PIR use RT_MAX_RATE to specify the maximum rate. 
 *      The real maximum rate might be different by chip.
 *      Unit is 8Kbps.
 */
extern int32 
rt_gpon_ponQueue_set(uint32 tcontId, uint32 tcQueueId, rt_gpon_queueCfg_t *pQueuecfg);

/* Function Name:
 *      rt_gpon_ponQueue_get
 * Description:
 *      Get the pon queue
 * Input:
 *      tcontId    - logical tcont index
 *      tcQueueId  - logical tcont queue id
 * Output:
 *      pQueuecfg - pointer to the queue configuration
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      This API get pon queue configuration 
 */
extern int32 
rt_gpon_ponQueue_get(uint32 tcontId, uint32 tcQueueId, rt_gpon_queueCfg_t *pQueuecfg);

/* Function Name:
 *      rt_gpon_ponQueue_del
 * Description:
 *      Delete the pon queue
 * Input:
 *      tcontId    - logical tcont index
 *      tcQueueId  - logical tcont queue id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note: 
 *      This API is used for delete pon queue 
 */
extern int32 
rt_gpon_ponQueue_del(uint32 tcontId, uint32 tcQueueId);

/* Function Name:
 *      rt_gpon_scheInfo_get
 * Description:
 *      Get scheduling information
 * Input:
 *      None
 * Output:
 *      pScheInfo  - pointer to the scheduling information
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      This API get max_tcont, max_tcon_queue, max_flow, and max_pon_queue
 */
extern int32 
rt_gpon_scheInfo_get(rt_gpon_schedule_info_t *pScheInfo);

/* Function Name:
 *      rt_gpon_flowCounter_get
 * Description:
 *      Get the flow counters by types.
 * Input:
 *      flowId   - logical flow ID
 *      type     - counter type
 * Output:
 *      pFlowCnt - pointer to the flow counters
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      RT_GPON_CNT_TYPE_FLOW_DS_GEM, RT_GPON_CNT_TYPE_FLOW_DS_ETH
 *      RT_GPON_CNT_TYPE_FLOW_US_GEM, RT_GPON_CNT_TYPE_FLOW_US_ETH
 */
extern int32 
rt_gpon_flowCounter_get(uint32 flowId, rt_gpon_flow_counter_type_t flowCntType, rt_gpon_flow_counter_t *pFlowCnt);

/* Function Name:
 *      rt_gpon_pmCounter_get
 * Description:
 *      Get the pm counters by types.
 * Input:
 *      pmCntType  - counter type
 * Output:
 *      pPmCnt       - pointer to the global counters
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      This API get performance monitoring counter 
 */
extern int32 
rt_gpon_pmCounter_get(rt_gpon_pm_type_t pmCntType, rt_gpon_pm_counter_t *pPmCnt);

/* Function Name:
 *      rt_gpon_ponTag_get
 * Description:
 *      Get the PON Tag.
 * Input:
 *      None
 * Output:
 *      pPonTag       - pointer to the PON TAG
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      This API get PON tag information 
 */
extern int32 
rt_gpon_ponTag_get(rt_gpon_ponTag_t *pPonTag);

/* Function Name:
 *      rt_gpon_msk_set
 * Description:
 *      Set the Master Session Key.
 * Input:
 *      pMsk - pointer to the Master Session Key
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      This API get Master Session Key 
 */
extern int32 
rt_gpon_msk_set(rt_gpon_msk_t *pMsk);

/* Function Name:
 *      rt_gpon_omci_mic_generate
 * Description:
 *      Generate the OMCI MIC value according to the inputs.
 * Input:
 *      dir     - the direction of the OMCI msg, 0x01 for Downstream and 0x02 for Upstream
 *      pMsg    - pointer to the OMCI message data
 *      msgLen  - length of the OMCI message, not include MIC
 * Output:
 *      mic     - pointer to the generated MIC
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      This API is used to generate the OMCI MIC value 
 */
extern int32
rt_gpon_omci_mic_generate(uint32 dir, uint8 *pMsg, uint32 msgLen, uint32 *mic);

/* Function Name:
 *      rt_gpon_mcKey_set
 * Description:
 *      Set the Multicast Key.
 * Input:
 *      pMcKey - pointer to the Multicast Key
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      This API is used to set muticast key, only for XG/XGS/NG PON 
 */
extern int32 
rt_gpon_mcKey_set(rt_gpon_mc_key_t *pMcKey);

/* Function Name:
 *      rt_gpon_egrBandwidthCtrlRate_get
 * Description:
 *      Get the egress bandwidth control rate.
 * Input:
 *      None
 * Output:
 *      pRate - egress bandwidth control rate
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - Invalid port id
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      The unit of granularity  is 1Kbps.
 */
extern int32
rt_gpon_egrBandwidthCtrlRate_get(uint32 *pRate);

/* Function Name:
 *      rt_gpon_egrBandwidthCtrlRate_set
 * Description:
 *      Set the pon port egress bandwidth control rate.
 * Input:
 *      rate - egress bandwidth control rate
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK      - OK
 *      RT_ERR_FAILED  - Failed
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_RATE    - Invalid input rate
 * Note:
 *      The unit of granularity  is 1Kbps.
 *      Use RT_MAX_RATE to specify the maximum rate.
 *      The real maximum rate might be different by chip.
 */
extern int32
rt_gpon_egrBandwidthCtrlRate_set(uint32 rate);

/* Function Name:
 *      rt_gpon_attribute_get
 * Description:
 *      Get the GPON related attribute.
 * Input:
 *      attributeType       - attribute type
 * Output:
 *      pAttributeValue     - pointer to the attribute value
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      This API get GPON misc attribute.
 */
extern int32
rt_gpon_attribute_get(rt_gpon_attribute_type_t attributeType,rt_gpon_attribute_t *pAttributeValue);

/* Function Name:
 *      rt_gpon_attribute_set
 * Description:
 *      Set the GPON related attribute.
 * Input:
 *      attributeType       - attribute type
 *      pAttributeValue     - pointer to the attribute value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      This API set GPON misc attribute 
 */
extern int32
rt_gpon_attribute_set(rt_gpon_attribute_type_t attributeType,rt_gpon_attribute_t *pAttributeValue);

/* Function Name:
 *      rt_gpon_omci_mirror_set
 * Description:
 *      Set the GPON OMCI mirror function.
 * Input:
 *      enable          - enable/disable mirro function 
 *      mirroringPort   - mirroring port 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK      - OK
 *      RT_ERR_FAILED  - Failed
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_INPUT   - Invalid input 
 * Note: 
 *      This API provide the omci message mirror function. 
 */
extern int32
rt_gpon_omci_mirror_set(rt_enable_t enable, rt_port_t mirroringPort);

/* Function Name:
 *      rt_gpon_omci_mirror_get
 * Description:
 *      Get the GPON OMCI mirror function.
 * Input:
 *      None
 * Output:
 *      pEnable         - get Enable/disble mirroring OMCI status
 *      pMirroringPort  - get mirroring port 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_INPUT        - Invalid input 
 * Note: 
 *      The API get the state of OMCI mirror function. 
 */
extern int32
rt_gpon_omci_mirror_get(rt_enable_t *pEnable, rt_port_t *pMirroringPort);

/* Function Name:
 *      rt_gpon_forceEmergencyStop_set
 * Description:
 *      Set the GPON force emergency Stop state.
 * Input:
 *      State  - enable/disable emergency stop state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note: 
 *      This API set GPON force emergency stop state. 
 */
extern int32
rt_gpon_forceEmergencyStop_set(rt_enable_t state);

/* Function Name:
 *      rt_gpon_tcontCounter_get
 * Description:
 *      Get the tcont counters.
 * Input:
 *      pTcontCnt->tconId  - tcont ID
 * Output:
 *      pTcontCnt - pointer to the flow counters
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      This API get the tcont counters. 
 */
extern int32 
rt_gpon_tcontCounter_get(rt_gpon_tcont_counter_t *pTcontCnt);

/* Function Name:
 *      rt_gpon_omcc_get
 * Description:
 *      get omcc information include allocid and gem port.
 * Input:
 *      None
 * Output:
 *      pOmcc                 -   omcc information
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_ENTRY_NOTFOUND - specified entry not found
 * Note: 
 *      This API get OMCC information. 
 */
extern int32 
rt_gpon_omcc_get(rt_gpon_omcc_t *pOmcc);

/* Function Name:
 *      rt_gpon_fec_get
 * Description:
 *      Get the GPON FEC status function.
 * Input:
 *      None
 * Output:
 *      pFecStatus          -   fec status information
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      This API get GPON US/DS FEC status. 
 */
extern int32
rt_gpon_fec_get(rt_gpon_fec_status_t *pFecStatus);

/* Function Name:
 *      dal_rt_rtl8277c_gpon_ploam_rx_register
 * Description:
 *      register ploam rx callback
 * Input:
 *      func            - callback func
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - OK
 *      RT_ERR_FAILED   - Failed
 *      RT_ERR_INPUT    - null callback func
 * Note:
 */
extern int32
rt_gpon_ploam_rx_register(rt_gpon_ploam_rx_callback func);
#endif
