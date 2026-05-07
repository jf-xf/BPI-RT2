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
 * Purpose : Definition of OMCI outgoing event define
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI outgoing event define
 */

#ifndef __OMCI_EVENT_H__
#define __OMCI_EVENT_H__

#ifdef  __cplusplus
extern "C" {
#endif


#include "gos_type.h"


// define event type
typedef enum {
	OMCI_EVENT_TYPE_CFG_CHANGE	= 0,
	OMCI_EVENT_TYPE_SW_DOWNLOAD
} omci_event_type_t;


#ifdef  __cplusplus
}
#endif

#endif
