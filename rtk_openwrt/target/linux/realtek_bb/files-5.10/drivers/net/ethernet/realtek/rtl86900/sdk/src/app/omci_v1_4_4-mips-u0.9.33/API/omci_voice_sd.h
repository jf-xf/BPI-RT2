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
 * Purpose : Definition of Dual Management shared define
 */

#ifndef __OMCI_VOICE_DS_H__
#define __OMCI_VOICE_DS_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum voice_cfg_op_e
{
	VOICE_OP_RESET_ALL,
	VOICE_OP_SET_SHARE,
	VOICE_OP_SET_SHM,
	VOICE_OP_GET_LINE_STATE,
	VOICE_OP_INIT_VARS,
	VOICE_OP_GET_SHARE,
	VOICE_OP_GET_SHM,
	VOICE_OP_GET_RTP_STAT
}voice_cfg_op_t;

typedef struct voice_rtp_stat_info_s
{
	unsigned int			ch_id;
    unsigned int			session_id;
    unsigned int			reset_flag;
    void                    *pRtp_stat;                         //TstRtpRtcpStatistics 	*pRtp_stat;
}voice_rtk_stat_info_t;

typedef struct voice_cfg_s
{
	void                    *pFlash;                           //voip_flash_share_t  	*pFlash;
	void                    **ppCfg;                           //voipCfgParam_t			**ppCfg;
	void                    **ppSts;                           //voip_state_share_t     **ppSts;
	voice_rtk_stat_info_t	rtpStatInfo;
}voice_cfg_t;

typedef struct voice_cfg_msg_s
{
	voice_cfg_op_t	op_id;
	voice_cfg_t		cfg;
}voice_cfg_msg_t;


#ifdef __cplusplus
}
#endif

#endif
