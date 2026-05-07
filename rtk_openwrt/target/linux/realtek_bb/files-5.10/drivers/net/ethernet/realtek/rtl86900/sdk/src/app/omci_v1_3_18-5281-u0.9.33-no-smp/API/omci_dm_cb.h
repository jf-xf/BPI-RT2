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
 * Purpose : Definition of Dual Management callbacks
 */

#ifndef __OMCI_DM_CB_H__
#define __OMCI_DM_CB_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "omci_dm_sd.h"


typedef struct
{
	int (*omci_dm_pon_wan_info_set)(omci_dm_pon_wan_info_t *pPonWanInfo);
	int (*omci_dm_pon_wan_info_del)(unsigned int wanIdx);
    	int (*omci_dm_ds_mc_bc_info_set)(omci_dm_ds_bc_mc_info_t *pDsBcMcInfo);
	int	(*omci_dm_ds_mc_bc_info_del)(unsigned int relatedId);
} omci_dmm_cb_t;


void omci_dmm_cb_register(omci_dmm_cb_t *p);
void omci_dmm_cb_unregister(void);
int omci_dmm_pon_wan_rule_xlate(omci_dm_pon_wan_info_t *pRule, omci_pon_wan_rule_position_t pos, void *p);
int omci_dmm_rule_available_check(int rule_idx);

#ifdef __cplusplus
}
#endif

#endif
