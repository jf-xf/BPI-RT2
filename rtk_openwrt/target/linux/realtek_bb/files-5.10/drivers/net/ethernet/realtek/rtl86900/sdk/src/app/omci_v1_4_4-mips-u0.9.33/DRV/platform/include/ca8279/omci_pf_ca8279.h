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

#ifndef __OMCI_PF_CA8279_H__
#define __OMCI_PF_CA8279_H__

#include <linux/workqueue.h>
#include <rtk/classify.h>
#define __LINUX_KERNEL__
#include <module/gpon/gpon.h>
#include <module/gpon/gpon_defs.h>
#include <DRV/omci_drv.h>



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



/****************************************************************/
/* Type Definition                                              */
/****************************************************************/
typedef struct ca8279_gpon_usFlow_s{
    unsigned int    gemPortId;
    unsigned int    tcontId;
    unsigned char   tcQueueId;
    unsigned char   isExpand;
    unsigned char   aesState;
}ca8279_gpon_usFlow_t;


typedef struct ca8279_gpon_dsFlow_s{
    unsigned int    gemPortId;
}ca8279_gpon_dsFlow_t;

typedef struct ca8279_ponmac_queueCfg_s
{
    unsigned int cir;//unit,8Kbps
    unsigned int pir;//unit,8Kbps
    unsigned int scheduleType;
    unsigned int weight;
    //rtk_enable_t egrssDrop;

} ca8279_ponmac_queueCfg_t;


typedef struct ca8279_gpon_schedule_info_s{
    unsigned int      omcc_tcont;
    unsigned int      omcc_queue;
    unsigned int      omcc_flow;
    unsigned int      max_tcont;
    unsigned int      max_tcon_queue;
    unsigned int      max_flow;
    unsigned int      max_pon_queue;
}ca8279_gpon_schedule_info_t;

typedef struct ca8279_gpon_gem_stat_s{
    unsigned int        gemBlock;
    unsigned long long  gemByte;
} ca8279_gpon_gem_stat_t;

/*for handle ioctl handler*/
typedef struct omci_work_s {
	rtk_gpon_omci_msg_t omci;
	struct work_struct work;
}omci_work_t;




int pf_ca8279_gpon_init(unsigned int maxGemFlow);
int pf_ca8279_gpon_scheInfo_get(ca8279_gpon_schedule_info_t *pScheInfo);
int pf_ca8279_gpon_dump(unsigned int maxGemFlow);
int pf_ca8279_gpon_tcont_create(unsigned int tcontId, unsigned int allocId);
int pf_ca8279_gpon_tcont_delete(unsigned int tcontId, unsigned int allocId);

int pf_ca8279_gpon_usFlow_set(unsigned int usFlowId, const ca8279_gpon_usFlow_t *pUsFlow);
int pf_ca8279_gpon_usFlow_del(unsigned int usFlowId);
int pf_ca8279_gpon_usFlow_delAll(void);
int pf_ca8279_gpon_dsFlow_set(unsigned int dsFlowId, const ca8279_gpon_dsFlow_t *pDsFlow);
int pf_ca8279_gpon_dsFlow_del(unsigned int dsFlowId);
int pf_ca8279_gpon_dsFlow_delAll(void);

int pf_ca8279_gpon_pon_queue_add(
    unsigned int tcontId, 
    unsigned int tcQueueId, 
    unsigned int phyQueueId, 
    const ca8279_ponmac_queueCfg_t *pQueuecfg
);
int pf_ca8279_gpon_pon_queue_del(    
    unsigned int tcontId, 
    unsigned int tcQueueId, 
    unsigned int phyQueueId, 
    const ca8279_ponmac_queueCfg_t *pQueuecfg
);

int pf_ca8279_gpon_usFlowStat_get(unsigned int usFlowId, ca8279_gpon_gem_stat_t *stat);
int pf_ca8279_gpon_dsFlowStat_get(unsigned int dsFlowId, ca8279_gpon_gem_stat_t *stat);

#endif
