/*
 * Copyright (C) 2019 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: $
 * $Date: $
 *
 * Purpose : Definition of classify rule global API
 *
 * Feature : The file have include the following module and sub-modules
 *           (1) classify rule add/delete
 *
 */

#ifndef __RT_CLS_H__
#define __RT_CLS_H__

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/rt_type.h>
#include <hal/chipdef/chip.h>

/*
 * Symbol Definition
 */
 
#define RT_CLS_HW_INDEX_MAX 10

typedef enum rt_cls_get_op_e{
    RT_CLS_GET_OP_FIRST,
    RT_CLS_GET_OP_NEXT,
    RT_CLS_GET_OP_INDEX
} rt_cls_get_op_t;

typedef enum rt_cls_dir_e{
    RT_CLS_DIR_DS,   /* Downstream */
    RT_CLS_DIR_US    /* Upstream */
} rt_cls_dir_t;


typedef enum rt_cls_filter_mask_e
{
    RT_CLS_FILTER_FLOWID_BIT       =(1<<0),  /* Downstream flow ID */
    RT_CLS_FILTER_OUTER_TAGIf_BIT  =(1<<1),  /* Outer tag interface */
    RT_CLS_FILTER_INNER_TAGIf_BIT  =(1<<2),  /* Inner tag interface */
    RT_CLS_FILTER_OUTER_TPID_BIT   =(1<<3),
    RT_CLS_FILTER_OUTER_VID_BIT    =(1<<4),
    RT_CLS_FILTER_OUTER_PRI_BIT    =(1<<5),
    RT_CLS_FILTER_OUTER_DEI_BIT    =(1<<6),
    RT_CLS_FILTER_INNER_TPID_BIT   =(1<<7),
    RT_CLS_FILTER_INNER_VID_BIT    =(1<<8),
    RT_CLS_FILTER_INNER_PRI_BIT    =(1<<9),
    RT_CLS_FILTER_INNER_DEI_BIT    =(1<<10),
    RT_CLS_FILTER_ETHERTYPE        =(1<<11),
    RT_CLS_FILTER_DSCP_BIT         =(1<<12),
    RT_CLS_FILTER_EGRESS_PORT_BIT  =(1<<13)
}rt_cls_filter_mask_t;

typedef enum rt_cls_tag_action_e
{
    RT_CLS_TAG_TRANSPARENT,   /* Keep original VLAN tag */
    RT_CLS_TAG_TAGGING,       /* Add a tag */
    RT_CLS_TAG_MODIFY,        /* Modify original VLAN tag content */
    RT_CLS_TAG_REMOVE,        /* Delete original VLAN tag */
    RT_CLS_TAG_END
}rt_cls_tag_action_t;

typedef enum rt_cls_tpid_action_e{
    RT_CLS_TPID_ASSIGN,
    RT_CLS_TPID_COPY_FROM_1ST_TAG,        /* TPID copy from outer tag, if no outer tag then as assian */
    RT_CLS_TPID_COPY_FROM_2ND_TAG,        /* TPID copy from inner tag, if no inner tag then as assian */
    RT_CLS_TPID_END
}rt_cls_tpid_action_t;

typedef enum rt_cls_vid_action_e
{
    RT_CLS_VID_ASSIGN,
    RT_CLS_VID_COPY_FROM_1ST_TAG,        /* VID copy from outer tag, if no outer tag then as assian */
    RT_CLS_VID_COPY_FROM_2ND_TAG,        /* VID copy from inner tag, if no inner tag then as assian */
    RT_CLS_VID_TRANSPARENT,
    RT_CLS_VID_END,
}rt_cls_vid_action_t;

typedef enum rt_cls_pri_action_e{
    RT_CLS_PRI_ASSIGN,
    RT_CLS_PRI_COPY_FROM_1ST_TAG,        /* PRI copy from outer tag, if no outer tag then as assian */
    RT_CLS_PRI_COPY_FROM_2ND_TAG,        /* PRI copy from inner tag, if no inner tag then as assian */
    RT_CLS_PRI_COPY_FROM_DSCP_REMAP,     /* PRI assian from DSCP remappin table */
    RT_CLS_PRI_TRANSPARENT,
    RT_CLS_PRI_END,
}rt_cls_pri_action_t;

typedef enum rt_cls_dei_action_e{
    RT_CLS_DEI_ASSIGN,
    RT_CLS_DEI_COPY_FROM_1ST_TAG,        /* DEI/CFI copy from outer tag, if no outer tag then as assian */
    RT_CLS_DEI_COPY_FROM_2ND_TAG,        /* DEI/CFI copy from inner tag, if no inner tag then as assian */
    RT_CLS_DEI_END,
}rt_cls_dei_action_t;

typedef struct rt_cls_filter_rule_s
{
    /* filter mask*/
    rt_cls_filter_mask_t filterMask;     /* Filter mask, if bit equal to 1 means filter corresponging field */
    /* filter values */
    uint32               flowId;         /* Downstream flow ID */
    uint32               outerTagIf;     /* Outer tag interface, 1: packet has an outer tag, 0: packet has no outer tag */
    uint32               innerTagIf;     /* Inner tag interface, 1: packet has an inner tag, 0: packet has no inner tag */
    uint32               outerTagTpid;   
    uint32               outerTagVid;
    uint32               outerTagPri;
    uint32               outerTagDei;
    uint32               innerTagTpid;
    uint32               innerTagVid;
    uint32               innerTagPri;
    uint32               innerTagDei;
    uint32               etherType;
    uint8                dscp[8];        /* DSCP bitmask, if bit equal to 1 means filter corresponding DSCP value */
    uint32               egressPortMask; /* Egress UNI port mask, Only for DS only rule to filter egress port */
} rt_cls_filter_rule_t;

typedef struct rt_cls_rule_action_s
{
    rt_cls_tag_action_t     tagAction;      /* VLAN tag action */
    rt_cls_tpid_action_t    tagTpidAction;  /* TPID action */
    rt_cls_vid_action_t     tagVidAction;   /* VID action */
    rt_cls_pri_action_t     tagPriAction;   /* PRI action */
    rt_cls_dei_action_t     tagDeiAction;   /* VLAN DEI/CFI action */
    int32 assignedTpid;                     /* TPID value*/
    int32 assignedVid;                      /* VID value */
    int32 assignedPri;                      /* PRI value */
    int32 assignedDei;                      /* DEI/CFI value */
}rt_cls_rule_action_t;

typedef enum rt_cls_ds_pri_act_e
{
     RT_CLS_DS_PRI_ACT_NOP = 0,
     RT_CLS_DS_PRI_ACT_ASSIGN = 1,
} rt_cls_ds_pri_act_t;


typedef struct rt_cls_rule_s
{
    uint32               index;            /* Rule index */
    uint32               flowId;           /* Upstream flow ID */
    uint32               rulePriority;     /* Rule priority, a higher value means higher priority */
    rt_cls_dir_t         direction;        /* Rule direction, upstream or downstream*/
    uint32               ingressPortMask;  /* UNI port mask */
    rt_cls_filter_rule_t filterRule;       /* Rule filter pattetn */
    rt_cls_rule_action_t outerTagAct;      /* Outer VLAN tag action */
    rt_cls_rule_action_t innerTagAct;      /* Inner VLAN tag action */
    rt_cls_ds_pri_act_t  dsPriAct;         /* Downstream internal priority action */
    uint32               dsPri;            /* Downstream internal priority*/
} rt_cls_rule_t;

typedef struct rt_cls_rule_info_s
{
    rt_cls_rule_t        rule;                          /* CLS Rule*/
    uint16               hwIndex[RT_CLS_HW_INDEX_MAX];  /* CLS hardware rule index, 0xff mean not in use*/
} rt_cls_rule_info_t;


/*
 * Function Declaration
 */

/* Module Name    : Classify rule     */
/* Sub-module Name: Add/delete classify rule */

/* Function Name:
 *      rt_cls_init
 * Description:
 *      Initialize cls setting and database
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                               - OK
 *      RT_ERR_FAILED                           - Failed
 * Note:
 *      None
 */
extern int32 rt_cls_init(void);

/* Function Name:
 *      rt_cls_rule_add
 * Description:
 *      Add classify rule and apply to ASIC.
 * Input:
 *      pClsRule - The classify rule configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                               - OK
 *      RT_ERR_FAILED                           - Failed
 *      RT_ERR_NULL_POINTER                     - Pointer pClassifyCfg point to NULL.
 *      RT_ERR_INPUT                            - Invalid input parameters.
 * Note:
 *      None
 */
extern int32 rt_cls_rule_add(rt_cls_rule_t *pClsRule);

/* Function Name:
 *      rt_cls_rule_delete
 * Description:
 *      Delete  classify rule.
 * Input:
 *      index     - index of classify rule entry.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                               - OK
 *      RT_ERR_FAILED                           - Failed
 *      RT_ERR_INPUT                            - Invalid input parameters.
 * Note:
 *      None.
 */
extern int32 rt_cls_rule_delete(uint32 index);

/* Function Name:
 *      rt_cls_rule_get
 * Description:
 *      Get  classify rule.
 * Input:
 *      op        - operation type of get (first/next/index)
 *      index     - index of classify rule entry.
 *      dir       - direction of classify rule
 * Output:
 *      pClsRuleInfo  - classify rule information
 * Return:
 *      RT_ERR_OK                               - OK
 *      RT_ERR_FAILED                           - Rule not found
 * Note:
 *      None.
 */
extern int32 rt_cls_rule_get(rt_cls_get_op_t op, uint32 index, rt_cls_dir_t dir, rt_cls_rule_info_t *pClsRuleInfo);


/* Function Name:
 *      rt_cls_fwdPort_set
 * Description:
 *      Set VEIP port default forward port.
 * Input:
 *      port     - port index.
 *      fwdPort  - defalt forward port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                               - OK
 *      RT_ERR_FAILED                           - Failed
 * Note:
 *      For the device type, the WAN port is not the PON port, ex. ethernet WAN mode,
 *      it needs to change the default forward port of the WAN port from the LAN side to WAN side  
 */
extern int32 rt_cls_fwdPort_set(rt_port_t port, rt_port_t fwdPort);

#endif
