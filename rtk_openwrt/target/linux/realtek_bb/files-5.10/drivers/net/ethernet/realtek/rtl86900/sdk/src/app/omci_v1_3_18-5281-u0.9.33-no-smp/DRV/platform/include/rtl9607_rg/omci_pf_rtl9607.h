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
 *
 * $Revision: 39101 $
 * $Date: 2013-06-24 04:35:27 -0500 (Fri, 03 May 2013) $
 *
 * Purpose : OMCI driver layer module defination
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI (G.984.4)
 *
 */

#ifndef __OMCI_RF_RTL9607_H__
#define __OMCI_RF_RTL9607_H__

#include <linux/workqueue.h>
#include <rtk/classify.h>
#define __LINUX_KERNEL__
#include <module/gpon/gpon.h>
#include <module/gpon/gpon_defs.h>
#include <DRV/omci_drv.h>
#include "omci_drv_ext.h"

/*
MACRO
*/



/*
DEFINE
*/

/*for handle ioctl handler*/
typedef struct omci_work_s {
	rtk_gpon_omci_msg_t omci;
	struct work_struct work;
}omci_work_t;



#endif
