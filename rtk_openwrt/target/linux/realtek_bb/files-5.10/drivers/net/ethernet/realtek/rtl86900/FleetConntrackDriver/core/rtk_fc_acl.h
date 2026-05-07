/*
 * Copyright (C) 2017 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
*/

#include "rtk_fc_struct.h"

/*user defined ACL*/
int32 _rtk_fc_aclFilterAndQos_add(rt_acl_filterAndQos_t *acl_filter, int *acl_filter_idx);
int32 _rtk_fc_aclFilterAndQos_del(int acl_filter_idx);
int32 _rtk_fc_aclFilterAndQos_get(int acl_filter_idx, rt_acl_filterAndQos_t *acl_filter);
int32 _rtk_fc_acl_mib_get(int idx, rt_stat_acl_mib_t *paclMibCnt);
int32 _rtk_fc_acl_mib_set(int idx, rt_stat_acl_mib_t paclMibCnt);
int32 _rtk_fc_acl_mib_clear(int idx);
int32 _rtk_fc_acl_reserved_status_get(rt_acl_reserved_type_t reserved_type, int *status);
int32 _rtk_fc_acl_reserved_status_set(rt_acl_reserved_type_t reserved_type, int status);
int _rtk_fc_aclSWEntry_and_asic_rearrange(void);

/*reserved ACL*/
int _rtk_rg_aclAndCfReservedRuleAdd(rtk_rg_aclAndCf_reserved_type_t rsvType, void *parameter);
int _rtk_rg_aclAndCfReservedRuleDel(rtk_rg_aclAndCf_reserved_type_t rsvType);
int _rtk_rg_aclAndCfReservedRuleAddSpecial(rtk_rg_aclAndCf_reserved_type_t rsvType, void *parameter);
int _rtk_rg_aclAndCfReservedRuleDelSpecial(rtk_rg_aclAndCf_reserved_type_t rsvType, void *parameter);


// init
int rtk_fc_aclInit(void);

/*external APIs need for this ACl module*/
int _rtk_fc_aclSWEntry_get(int index, rtk_fc_aclFilterEntry_t* aclSWEntry);
int _rtk_fc_aclHeaderInfo_get(char *acl_dbg_info, int acl_dbg_info_size, int ref_info);
int _rtk_fc_aclAndCfReservedRule_intfRsvUpdate(bool is_add, int intf_swIdx);


