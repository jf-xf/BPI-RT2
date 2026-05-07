#ifndef __RT_EDP_GLUE_STRUCT_H__
#define __RT_EDP_GLUE_STRUCT_H__

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++
This file only works when EDP_ACL_SUPPORT_HOOK_CHECK=1

Patch below information to linux-3.18.x/include/linux/skbuff.h

#if defined(CONFIG_RTL_NEW_FLOW_BASE_HWNAT_DRIVER) || defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
#include "../../drivers/net/ethernet/realtek/rtl86900/FleetConntrackDriver/rtk_rg_fc_glue_struct.h"
#endif
+#if defined(CONFIG_RT_EDP_API)
+#include "../../drivers/net/ethernet/realtek/rtl86900/romeDriver/rt_edp_glue_struct.h"
+#endif

struct sk_buff {
...
#if defined(CONFIG_RTL_NEW_FLOW_BASE_HWNAT_DRIVER) || defined(CONFIG_RTK_L34_FLEETCONNTRACK_ENABLE)
        rtk_rg_fc_ingress_data_t fcIngressData;
#endif
+#if defined(CONFIG_RT_EDP_API)
+	rt_edp_skb_t edpSkbData;
+#endif
...
};
++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#define EDP_ACL_MAX_ACL_SW_ENTRY_SIZE 128		//ref to MAX_ACL_SW_ENTRY_SIZE

typedef struct rt_edp_aclHit_cvlan_tag_action_s{
	unsigned int 	cvlanTagIfDecision;	//ref to rt_edp_acl_cvlan_tagif_decision_t
	unsigned int	cvlanCvidDecision;	//ref to rt_edp_acl_cvlan_cvid_decision_t
	unsigned int	cvlanCpriDecision;	//ref to rt_edp_acl_cvlan_cpri_decision_t
	unsigned int 	assignedCvid;
	unsigned char 	assignedCpri;
}rt_edp_aclHit_cvlan_tag_action_t;

typedef struct rt_edp_aclHit_svlan_tag_action_s{
	unsigned int 	svlanTagIfDecision;	//ref to rt_edp_acl_svlan_tagif_decision_t
	unsigned int	svlanSvidDecision;	//ref to rt_edp_acl_svlan_svid_decision_t
	unsigned int	svlanSpriDecision;	//ref to rt_edp_acl_svlan_spri_decision_t
	unsigned int 	assignedSvid;
	unsigned char 	assignedSpri;
}rt_edp_aclHit_svlan_tag_action_t;

typedef struct rt_edp_aclHitAndAction_s
{
	unsigned int aclIgrHitMask[(EDP_ACL_MAX_ACL_SW_ENTRY_SIZE/32)+1]; //check RG_ACL 0~MAX_ACL_SW_ENTRY_SIZE ingressPart hit or not:  1ULL & ruleNo is 0: not hit  1: hit
	unsigned int aclEgrHitMask[(EDP_ACL_MAX_ACL_SW_ENTRY_SIZE/32)+1]; //check RG_ACL 0~MAX_ACL_SW_ENTRY_SIZE egressPart hit or not:   1ULL & ruleNo is 0: not hit  1: hit
	int aclEgrHaveToCheckRuleIdx[EDP_ACL_MAX_ACL_SW_ENTRY_SIZE];//record which ingress part hit rule index, for speeding up egress pattern check. (-1 means invalid index, and rest array doen't need to check)

	/* _rt_edp_aclDecisionClear will only reset below parameter before mark decision */

	unsigned int aclIgrDoneAction;	//ref to rt_edp_igrAction_controlBit_t
	unsigned int aclEgrDoneAction;	//ref to rt_edp_egrAction_controlBit_t

	/*RG ACL final decided actions*/
	unsigned int qos_actions; 		//ref to rt_edp_acl_qos_action_t

	unsigned char action_dot1p_remarking_pri;
	unsigned char action_ip_precedence_remarking_pri;
	unsigned char action_dscp_remarking_pri;
	unsigned char action_tos_tc_remarking_pri;
	rt_edp_aclHit_cvlan_tag_action_t action_acl_cvlan;	//ref to rt_edp_cvlan_tag_action_t
	rt_edp_aclHit_svlan_tag_action_t action_acl_svlan;	//ref to rt_edp_svlan_tag_action_t
}rt_edp_aclHitAndAction_t;

typedef struct rt_edp_skb_data_s
{
	rt_edp_aclHitAndAction_t aclDecision;
}rt_edp_skb_t;

#endif
