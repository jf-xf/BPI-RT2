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


typedef int (*OMCI_SET_CF_US_ACT_PTR)(OMCI_VLAN_OPER_ts *, unsigned int, rtk_classify_us_act_t *);
typedef int (*OMCI_SET_CF_US_RULE_PTR)(OMCI_VLAN_OPER_ts *, unsigned int, rtk_classify_cfg_t *, omci_rule_pri_t *);
typedef int (*ASSIGN_NONUSED_CF_IDX_PTR)(unsigned int, omci_rule_pri_t, unsigned int *);
typedef int (*OMCI_CREATE_US_DP_CF_PTR)(uint32,	l2_service_t *, rtk_classify_cfg_t *, uint8, omci_rule_pri_t, unsigned int);
typedef void (*SHOW_CF_FIELD)(rtk_classify_cfg_t *);
typedef void (*REMOVE_USED_CF_IDX)(unsigned int);
typedef void (*SAVE_CFCFG_TO_DB_PTR)(unsigned int, rtk_classify_cfg_t *);

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
