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
 * Purpose : Definition of ME attribute: PPTP video ANI (90)
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) ME attribute: PPTP video ANI (90)
 */

#ifndef __MIB_PPTP_VIDEO_ANI_H__
#define __MIB_PPTP_VIDEO_ANI_H__

#ifdef __cplusplus
extern "C" {
#endif


#define MIB_TABLE_PPTP_VIDEO_ANI_ATTR_NUM (17)
#define MIB_TABLE_PPTP_VIDEO_ANI_ENTITY_ID_INDEX ((MIB_ATTR_INDEX)1)
#define MIB_TABLE_PPTP_VIDEO_ANI_ADMIN_STATE_INDEX ((MIB_ATTR_INDEX)2)
#define MIB_TABLE_PPTP_VIDEO_ANI_OP_STATE_INDEX ((MIB_ATTR_INDEX)3)
#define MIB_TABLE_PPTP_VIDEO_ANI_ARC_INDEX ((MIB_ATTR_INDEX)4)
#define MIB_TABLE_PPTP_VIDEO_ANI_ARC_INTVL_INDEX ((MIB_ATTR_INDEX)5)
#define MIB_TABLE_PPTP_VIDEO_ANI_FREQ_RANGE_LOW_INDEX ((MIB_ATTR_INDEX)6)
#define MIB_TABLE_PPTP_VIDEO_ANI_FREQ_RANGE_HIGH_INDEX ((MIB_ATTR_INDEX)7)
#define MIB_TABLE_PPTP_VIDEO_ANI_SIG_CAPABILITY_INDEX ((MIB_ATTR_INDEX)8)
#define MIB_TABLE_PPTP_VIDEO_ANI_OPTICAL_SIG_LVL_INDEX ((MIB_ATTR_INDEX)9)
#define MIB_TABLE_PPTP_VIDEO_ANI_PILOT_SIG_LVL_INDEX ((MIB_ATTR_INDEX)10)
#define MIB_TABLE_PPTP_VIDEO_ANI_SIG_LVL_MIN_INDEX ((MIB_ATTR_INDEX)11)
#define MIB_TABLE_PPTP_VIDEO_ANI_SIG_LVL_MAX_INDEX ((MIB_ATTR_INDEX)12)
#define MIB_TABLE_PPTP_VIDEO_ANI_PILOT_FREQ_INDEX ((MIB_ATTR_INDEX)13)
#define MIB_TABLE_PPTP_VIDEO_ANI_AGC_MODE_INDEX ((MIB_ATTR_INDEX)14)
#define MIB_TABLE_PPTP_VIDEO_ANI_AGC_SETTING_INDEX ((MIB_ATTR_INDEX)15)
#define MIB_TABLE_PPTP_VIDEO_ANI_VIDEO_LOW_THOLD_INDEX ((MIB_ATTR_INDEX)16)
#define MIB_TABLE_PPTP_VIDEO_ANI_VIDEO_HIGH_THOLD_INDEX ((MIB_ATTR_INDEX)17)


typedef enum {
    PPTP_VIDEO_ANI_FREQ_LOW_NO_LOW_BAND     = 0,
    PPTP_VIDEO_ANI_FREQ_LOW_50_TO_550_MHZ   = 1,
    PPTP_VIDEO_ANI_FREQ_LOW_50_TO_750_MHZ   = 2,
    PPTP_VIDEO_ANI_FREQ_LOW_50_TO_870_MHZ   = 3,
} pptp_video_ani_attr_freq_range_low_t;

typedef enum {
    PPTP_VIDEO_ANI_FREQ_HIGH_NO_HIGH_BAND       = 0,
    PPTP_VIDEO_ANI_FREQ_HIGH_550_TO_750_MHZ     = 1,
    PPTP_VIDEO_ANI_FREQ_HIGH_550_TO_870_MHZ     = 2,
    PPTP_VIDEO_ANI_FREQ_HIGH_950_TO_2050_MHZ    = 3,
    PPTP_VIDEO_ANI_FREQ_HIGH_2150_TO_3250_MHZ   = 4,
    PPTP_VIDEO_ANI_FREQ_HIGH_950_TO_3250_MHZ    = 5,
} pptp_video_ani_attr_freq_range_high_t;

typedef enum {
    PPTP_VIDEO_ANI_SIG_NO_SIG_LVL_MEASUREMENT                           = 0,
    PPTP_VIDEO_ANI_SIG_TOTAL_OPTICAL_PWR_LVL                            = 1,
    PPTP_VIDEO_ANI_SIG_FIX_FREQ_PILOT_TONE_PWR_LVL                      = 2,
    PPTP_VIDEO_ANI_SIG_TOTAL_OPTICAL_AND_FIX_FREQ_PILOT_TONE_PWR_LVL    = 3,
    PPTP_VIDEO_ANI_SIG_VAR_FREQ_PILOT_TONE_PWR_LVL                      = 4,
    PPTP_VIDEO_ANI_SIG_TOTAL_OPTICAL_AND_VAR_FREQ_PILOT_TONE_PWR_LVL    = 5,
    PPTP_VIDEO_ANI_SIG_BROADBAND_RF_PWR_LVL                             = 6,
    PPTP_VIDEO_ANI_SIG_TOTAL_OPTICAL_AND_BROADBAND_RF_PWR_LVL           = 7,
} pptp_video_ani_attr_sig_capability_t;

typedef enum {
    PPTP_VIDEO_ANI_AGC_MODE_NO_AGC              = 0,
    PPTP_VIDEO_ANI_AGC_MODE_BROADBAND_RF_AGC    = 1,
    PPTP_VIDEO_ANI_AGC_MODE_OPTICAL_AGC         = 2,
} pptp_video_ani_attr_agc_mode_t;

typedef struct {
	UINT16	EntityId;
	UINT8	AdminState;
	UINT8	OpState;
	UINT8	Arc;
	UINT8	ArcIntvl;
    UINT8   FreqRangeLow;
    UINT8   FreqRangeHigh;
    UINT8   SigCapability;
    UINT8   OpticalSigLvl;
    UINT8   PilotSigLvl;
    UINT8   SigLvlMin;
    UINT8   SigLvlMax;
    UINT32  PilotFreq;
    UINT8   AgcMode;
    UINT8   AgcSetting;
    UINT8   VideoLowThold;
    UINT8   VideoHighThold;
} __attribute__((packed)) MIB_TABLE_PPTP_VIDEO_ANI_T;


#ifdef __cplusplus
}
#endif

#endif
