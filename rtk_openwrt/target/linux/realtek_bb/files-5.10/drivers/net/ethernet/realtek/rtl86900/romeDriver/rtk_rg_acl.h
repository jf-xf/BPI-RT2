#ifndef RTK_RG_ACL_H
#define RTK_RG_ACL_H

#include <rtk_rg_internal.h>
#include <rtk_rg_mappingAPI.h>
//#include <rtk_rg_apollo_liteRomeDriver.h>	//FIXME: temporary incldue for calling rtk_rg_apollo_*

#define NEED_CF_ASIC_RULE_ONLY_TRUE 1
#define NEED_CF_ASIC_RULE_ONLY_FLASE 0

/*(1)ACL init related APIs*/
int _rtk_rg_acl_asic_init(void);
int _rtk_rg_classify_asic_init(void);

/*(2)RG_ACL APIs and internal APIs*/
#if defined(CONFIG_RG_RTL9607C_SERIES) || defined(CONFIG_RG_RTL9603CVD_SERIES)
int _rtk_rg_aclSWEntry_and_asic_rearrange(int acl_filter_idx);
#else
int _rtk_rg_aclSWEntry_and_asic_rearrange(void);
#endif
int _rtk_rg_acl_user_part_rearrange(void);
int _rtk_rg_check_acl_committed_rule(rtk_acl_igr_log_act_ctrl_t mode);


/*(3)literomeDriver mapping APIs*/
int32 _rtk_rg_apollo_aclFilterAndQos_add(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx);
int32 _rtk_rg_apollo_aclFilterAndQos_del(int acl_filter_idx);
int32 _rtk_rg_apollo_aclFilterAndQos_find(rtk_rg_aclFilterAndQos_t *acl_filter, int *valid_idx);
int32 _rtk_rg_apollo_classifyEntry_add(rtk_rg_classifyEntry_t *classifyFilter);
int32 _rtk_rg_apollo_classifyEntry_del(int index);
int32 _rtk_rg_apollo_classifyEntry_find(int index, rtk_rg_classifyEntry_t *classifyFilter);


/*(4)reserved ACL related APIs*/
int _rtk_rg_aclReservedEntry_init(void);
int _rtk_rg_aclAndCfReservedRuleAdd(rtk_rg_aclAndCf_reserved_type_t rsvType, void *parameter);
int _rtk_rg_aclAndCfReservedRuleDel(rtk_rg_aclAndCf_reserved_type_t rsvType);


/*(5)fwdEngine datapath used APIs*/
int _rtk_rg_aclDecisionClear(rtk_rg_pktHdr_t *pPktHdr);
int _rtk_rg_ingressACLPatternCheck(rtk_rg_pktHdr_t *pPktHdr, int ingressCvidRuleIdxArray[]);
rtk_rg_fwdEngineReturn_t _rtk_rg_ingressACLAction(rtk_rg_pktHdr_t *pPktHdr);
int _rtk_rg_egressACLPatternCheck(int direct, int naptIdx, rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb,int l3Modify,int l4Modify, rtk_rg_port_idx_t egressPort);
int _rtk_rg_egressACLAction(int direct, rtk_rg_pktHdr_t *pPktHdr);
rtk_rg_fwdEngineReturn_t _rtk_rg_modifyPacketByACLAction(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr,rtk_rg_port_idx_t egressPort);
int _rtk_rg_acl_egressIntfIdx_precheck(rtk_rg_pktHdr_t *pPktHdr);


/*(6)debug tool APIs*/
int _dump_rg_acl(struct seq_file *s);
int _dump_rg_cf(struct seq_file *s);
int _dump_rg_acl_and_cf_diagshell(struct seq_file *s);
#if defined(CONFIG_RG_G3_SERIES)
int _dump_acl_ca(struct seq_file *s);
int _dump_acl_ca_by_index( struct file *filp, const char *buff,unsigned long len, void *data );
int _dump_acl_by_index( struct file *filp, const char *buff,unsigned long len, void *data );
#endif
int _dump_acl(struct seq_file *s);
int _dump_cf(struct seq_file *s);
int32 _dump_acl_ipRangeTable(struct seq_file *s);
int32 _dump_acl_portRangeTable(struct seq_file *s);
int32 _dump_acl_template(struct seq_file *s);



/*(7)external APIs need for this ACl module*/
extern rtk_rg_err_code_t rtk_rg_apollo_intfInfo_find(rtk_rg_intfInfo_t *intf_info, int *valid_lan_or_wan_intf_idx);
extern rtk_rg_err_code_t rtk_rg_apollo_qosDot1pPriRemarkByInternalPriEgressPortEnable_get(rtk_rg_mac_port_idx_t rmk_port, rtk_rg_enable_t *pRmk_enable);
extern rtk_rg_err_code_t rtk_rg_apollo_qosPortBasedPriority_get(rtk_rg_mac_port_idx_t port_idx,uint32 *pInt_pri);
extern rtk_rg_err_code_t rtk_rg_apollo_aclFilterAndQos_del(int acl_filter_idx);
extern rtk_rg_err_code_t rtk_rg_apollo_aclFilterAndQos_add(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx);
extern rtk_rg_fwdEngineReturn_t rtk_rg_dropBySwRateLimt_Check(uint32 meterIndex, uint32 byteCount, uint32 packetCount, rtk_rg_meter_type_t funcbasedMeterType);


#endif //end of #ifndef RTK_RG_ACL_H


