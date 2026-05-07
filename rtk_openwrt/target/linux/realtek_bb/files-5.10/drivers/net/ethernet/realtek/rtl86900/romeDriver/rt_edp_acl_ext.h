#ifndef _RT_EDP_ACL_EXT_H
#define _RT_EDP_ACL_EXT_H

/*(1)ACL init related APIs*/
int _rt_edp_acl_linux_init(void);

/*(3)literomeDriver mapping APIs*/
//1 ===== FIXME: To prevent the data structure of parameter of RG API is changed, use orginal RG data structure temporarily. =====
int32 _rt_edp_aclFilterAndQos_add(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx);
int32 _rt_edp_aclFilterAndQos_del(int acl_filter_idx);
int32 _rt_edp_aclFilterAndQos_find(rtk_rg_aclFilterAndQos_t *acl_filter, int *valid_idx);
//1 ===== end FIXME =====

/*(5)linux datapath used APIs*/
#if defined(EDP_ACL_SUPPORT_HOOK_CHECK) && (EDP_ACL_SUPPORT_HOOK_CHECK==1)
int _rt_edp_ingressACLPatternCheckAndAction(struct sk_buff *skb);
int _rt_edp_egressACLPatternCheckAndAction(struct sk_buff *skb);
int _rt_edp_modifyPacketByACLAction(struct sk_buff *skb);
#endif	//EDP_ACL_SUPPORT_HOOK_CHECK

/*(6)debug tool APIs*/
int _dump_rt_edp_acl(struct seq_file *s);
int _dump_rt_edp_acl_and_cf_diagshell(struct seq_file *s);

#endif //end of #ifndef _RT_EDP_ACL_EXT_H
