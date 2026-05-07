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
 * Purpose :  Definition of rtk snooping mode related define
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1)
 */
#ifndef __VOICE_RTK_WRAPPER_H__
#define __VOICE_RTK_WRAPPER_H__

#ifdef  __cplusplus
extern "C" {
#endif

#include "voice_mgmt.h"

typedef struct codec_preced_s
{
    UINT8 chid;
    UINT8 CodecSelection1stOrder;
    UINT8 PacketPeriodSelection1stOrder;
    UINT8 SilenceSuppression1stOrder;
    UINT8 CodecSelection2ndOrder;
    UINT8 PacketPeriodSelection2ndOrder;
    UINT8 SilenceSuppression2ndOrder;
    UINT8 CodecSelection3rdOrder;
    UINT8 PacketPeriodSelection3rdOrder;
    UINT8 SilenceSuppression3rdOrder;
    UINT8 CodecSelection4thOrder;
    UINT8 PacketPeriodSelection4thOrder;
    UINT8 SilenceSuppression4thOrder;
}CODEC_PREDED_T, *CODEC_Preced_Tp;


extern omci_voice_wrapper_t rtk_voice_wrapper;


#ifdef  __cplusplus
}
#endif

#endif


