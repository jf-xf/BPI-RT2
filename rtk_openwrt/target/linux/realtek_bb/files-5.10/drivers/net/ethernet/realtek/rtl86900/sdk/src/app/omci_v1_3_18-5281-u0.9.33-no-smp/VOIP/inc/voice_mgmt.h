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
 * Purpose : Definition of rtk/rg or customized snooping mode related define
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1)
 */
#ifndef __VOICE_MGMT_H__
#define __VOICE_MGMT_H__

#ifdef  __cplusplus
extern "C" {
#endif

#include "voice_wrapper.h"
#include "omci_voip_basic.h"

typedef GOS_ERROR_CODE (*omci_voice_vendor_sr_t)(void  *);

typedef enum omci_voice_vendor_s
{
    VOICE_VENDOR_NONE = 0,
    VOICE_VENDOR_RTK,
	VOICE_VENDOR_END
} omci_voice_vendor_t;

typedef struct omci_voice_wrapper_info_s
{
    omci_voice_vendor_t         mode_id;
    omci_voice_wrapper_t 	    *pWrapper;
    omci_voice_vendor_sr_t      pHandler;
    void                        *pWrapSoHandle;
    void                        *pUtilSoHandle;
} omci_voice_wrapper_info_t;

extern omci_voice_wrapper_t* omci_voice_wrapper_find(omci_voice_vendor_t voice_vendor);
extern int omci_voice_wrapper_register(omci_voice_vendor_t v, omci_voice_wrapper_t *p);
extern int omci_voice_wrapper_unregister(omci_voice_vendor_t v);
extern void omci_voice_vendor_init(omci_voice_vendor_t vendor_id);
extern void omci_voice_vendor_deinit(omci_voice_vendor_t vendor_id);
extern void omci_voice_vendor_service_cb(omci_voice_vendor_t v, void *p);

#ifdef  __cplusplus
}
#endif

#endif


