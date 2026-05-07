#include <rtk_rg_apolloPro_liteRomeDriver.h>
#include <rtk_rg_acl.h>
#include <rtk_rg_acl_g3.h>
//#include <rtk_rg_apolloPro_liteRomeDriver.h>
#include <dal/rtl9607c/dal_rtl9607c_hwmisc.h>

#include <aal_l3_specpkt.h>	//for AAL_L3_SPECPKT_TYPE_
#include <aal_l3_cls.h>	//for CLS_TBL_ID
#include <ca_aal.h>	//for AAL_TABLE_L3_CLS_KEY

/*H/W ACL debug string*/
char *name_of_acl_field[]={
	"",
	"DMAC0[15:0]", // 1
	"DMAC1[31:16]",
	"DMAC2[47:32]",
	"SMAC0[15:0]",
	"SMAC1[31:16]",
	"SMAC2[47:32]",
	"ETHERTYPE", //7
	"STAG",
	"CTAG",
	"GEMIDX/LLIDX", 	//0xa
	"FRAME/TAGS",	//0xb
	"",	"",	"",	"", //0xc~0xf
	"IP4SIP[15:0]", //0x10
	"IP4SIP[31:16]",
	"IP4DIP[15:0]",
	"IP4DIP[31:16]",
	"IP4(TOS+PROTO)", //0x14
	"IP6(TC+NH)", //0x15
	"INNER_IP4SIP[15:0]",//0X16
	"INNER_IP4SIP[31:16]",
	"INNER_IP4DIP[15:0]",
	"INNER_IP4DIP[31:16]",
	"INNER_IP4(TOS+PROTO)", //0X1a
	"TCPSPORT",
	"TCPDPORT",
	"UDPSPORT",
	"UDPDPORT",
	"",//0x1f
	"IP6SIP[15:0]", //0x20
	"IP6SIP[31:16]",
	"IP6SIP[47:32]",
	"IP6SIP[63:48]",
	"IP6SIP[79:64]",
	"IP6SIP[95:80]",
	"IP6SIP[111:96]",
	"IP6SIP[127:112]", //0x27
	"IP6DIP[15:0]", //0x28
	"IP6DIP[31:16]",
	"IP6DIP[47:32]",
	"IP6DIP[63:48]",
	"IP6DIP[79:64]",
	"IP6DIP[95:80]",
	"IP6DIP[111:96]",
	"IP6DIP[127:112]",//0x2f
	"VIDRANGE",//x0x30
	"IPRANGE",
	"PORTRANGE",
	"PKTLENRANGE",
	"FIELD_VALID",
	"EXT_PORT_MASK",
	"","","","","","","","","","",//0x36~0x3f
	"FIELD_SEL0",//0x40
	"FIELD_SEL1",
	"FIELD_SEL2",
	"FIELD_SEL3",
	"FIELD_SEL4",
	"FIELD_SEL5",
	"FIELD_SEL6",
	"FIELD_SEL7",
	"FIELD_SEL8",
	"FIELD_SEL9",
	"FIELD_SEL10",
	"FIELD_SEL11",
	"FIELD_SEL12",
	"FIELD_SEL13",
	"FIELD_SEL14",
	"FIELD_SEL15"
};


/*RG CVLAN/SVLAN action debug string*/
char *name_of_rg_cvlan_tagif_decision[]={ //mappint to rtk_rg_acl_cvlan_tagif_decision_t
	"NOP",
	"TAGGING",
	"C2S",
	"SP2C",
	"UNTAG",
	"TRANSPARENT",
};

char *name_of_rg_cvlan_cvid_decision[]={ //mappint to rtk_rg_acl_cvlan_cvid_decision_t
	"ASSIGN",
	"FROM_1ST_TAG",
	"FROM_2ND_TAG",
	"FROM_INTERNAL_VID", //(upstream only)
	"FROM_DMAC2CVID", //(downstream only)
	"NOP", //apolloFE  (downstream only)
	"FROM_SP2C", //apolloFE
};

char *name_of_rg_cvlan_cpri_decision[]={ //mappint to rtk_rg_acl_cvlan_cpri_decision_t
	"ASSIGN",
	"FROM_1ST_TAG",
	"FROM_2ND_TAG",
	"FROM_INTERNAL_PRI",
	"NOP",
	"FROM_DSCP_REMAP",//apolloFE
	"FROM_SP2C", //apolloFE (downstream only)
};


char *name_of_rg_svlan_tagif_decision[]={ //mappint to rtk_rg_acl_svlan_tagif_decision_t
	"NOP",
	"TAGGING_WITH_VSTPID",
	"TAGGING_WITH_8100(not support)",
	"TAGGING_WITH_SP2C(not support)",
	"UNTAG",
	"TRANSPARENT",
	"TAGGING_WITH_VSTPID2",
	"TAGGING_WITH_ORIGINAL_STAG_TPID",
};


char *name_of_rg_svlan_svid_decision[]={ //mappint to rtk_rg_acl_svlan_svid_decision_t
	"ASSIGN",
	"FROM_1ST_TAG",
	"FROM_2ND_TAG",
	"NOP", //apolloFE
	"SP2C", //apolloFE
};

char *name_of_rg_svlan_spri_decision[]={ //mappint to rtk_rg_acl_svlan_spri_decision_t
	"ASSIGN",
	"FROM_1ST_TAG",
	"FROM_2ND_TAG",
	"FROM_INTERNAL_PRI",
	"NOP",//apolloFE
	"FROM_DSCP_REMAP", //apolloFE (downstream only)
	"FROM_SP2C", //apolloFE (downstream only)
};


char *name_of_rg_cfpri_decision[]={ //rtk_rg_cfpri_decision_t
	"CFPRI_ASSIGN",
	"CFPRI_NOP",
};

char *name_of_rg_sid_decision[]={ //rtk_rg_sid_llid_decision_t
	"SID_LLID_ASSIGN",
	"SID_LLID_NOP",
};

char *name_of_rg_dscp_decision[]={ //rtk_rg_dscp_decision_t
	"DSCP_ASSIGN",
	"DSCP_NOP",
};

char *name_of_rg_fwd_decision[]={ //rtk_rg_acl_fwd_decision_t
	"ACL_FWD_NOP",
	"ACL_FWD_DROP",
	"ACL_FWD_TRAP_TO_CPU",
	"ACL_FWD_DROP_TO_PON",
};

char *name_of_rg_uni_decision[]={//rtk_rg_acl_uni_decision_t
	"ACL_UNI_FWD_TO_PORTMASK_ONLY",
	"ACL_UNI_FORCE_BY_MASK",
	"ACL_UNI_TRAP_TO_CPU",
	"AL_UNI_NOP",
};


int _dump_rg_acl_entry_content(struct seq_file *s, rtk_rg_aclFilterAndQos_t *aclPara);
static int _rtk_rg_aclSWEntry_not_support(rtk_rg_aclFilterAndQos_t *acl_filter);
static int _rtk_rg_aclHWEntry_not_support(rtk_rg_aclFilterAndQos_t *acl_filter);
static int _rtk_rg_aclSWEntry_and_asic_reAdd(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx);
static int _rtk_rg_aclSWEntry_to_asic_add(rtk_rg_aclFilterAndQos_t *acl_filter,rtk_rg_aclFilterEntry_t* aclSWEntry,int shareHwAclWithSWAclIdx);


static int _rtk_rg_aclAndCfReservedRuleHeadReflash(void);
static int _rtk_rg_aclAndCfReservedRuleTailReflash(void);

int _rtk_rg_aclAndCfReservedRuleAddSpecial(rtk_rg_aclAndCf_reserved_type_t rsvType, void *parameter);
int _rtk_rg_acl_user_part_rearrange(void);


/*(1)ACL init related APIs*/
int _rtk_rg_acl_asic_init(void)
{
	/*init TEMPLATE, ref: aal_l3_cls.c*/
	//aclTemplate[0]: CL_IF_ID_KEY_MSK
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IF_ID_KEY_MSK] |= (1<<ACL_FIELD_IP_VER);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IF_ID_KEY_MSK] |= (1<<ACL_FIELD_TOP_VID);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IF_ID_KEY_MSK] |= (1<<ACL_FIELD_VLAN_CNT);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IF_ID_KEY_MSK] |= (1<<ACL_FIELD_ETHERTYPE_ENC);

	//aclTemplate[1]: CL_IPV4_TUNNEL_ID_KEY_MSK
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK] |= (1<<ACL_FIELD_L4_PORT);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK] |= (1<<ACL_FIELD_L4_VLD);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK] |= (1<<ACL_FIELD_IPV4_DA);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK] |= (1<<ACL_FIELD_IPV4_SA);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK] |= (1<<ACL_FIELD_IP_PROTOCOL);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK] |= (1<<ACL_FIELD_IP_VER);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK] |= (1<<ACL_FIELD_TOP_VID);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK] |= (1<<ACL_FIELD_VLAN_CNT);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK] |= (1<<ACL_FIELD_ETHERTYPE_ENC);

	//aclTemplate[2]: CL_IPV4_SA_SHORT_KEY_MSK = CL_IPV4_SHORT_KEY_MSK + CLS_KEY_MSK_IPV4_SA
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_SA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_L4_PORT);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_SA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_L4_VLD);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_SA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_IP_PROTOCOL);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_SA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_DSCP);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_SA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_TOP_DEI);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_SA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_TOP_VL_802_1P);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_SA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_TOP_VID);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_SA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_VLAN_CNT);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_SA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_IPV4_SA);

	//aclTemplate[3]: CL_IPV4_DA_SHORT_KEY_MSK = CL_IPV4_SHORT_KEY_MSK + CLS_KEY_MSK_IPV4_DA
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_DA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_L4_PORT);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_DA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_L4_VLD);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_DA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_IP_PROTOCOL);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_DA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_DSCP);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_DA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_TOP_DEI);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_DA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_TOP_VL_802_1P);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_DA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_TOP_VID);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_DA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_VLAN_CNT);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV4_DA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_IPV4_DA);

	//aclTemplate[4]: CL_IPV6_SA_SHORT_KEY_MSK = CL_IPV6_SHORT_KEY_MSK + CLS_KEY_MSK_IPV6_SA
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV6_SA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_L4_PORT);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV6_SA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_L4_VLD);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV6_SA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_IPV6_FLOW_LBL);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV6_SA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_IP_PROTOCOL);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV6_SA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_DSCP);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV6_SA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_TOP_DEI);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV6_SA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_TOP_VL_802_1P);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV6_SA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_TOP_VID);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV6_SA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_VLAN_CNT);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV6_SA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_IPV6_SA);

	//aclTemplate[5]: CL_IPV6_DA_SHORT_KEY_MSK = CL_IPV6_SHORT_KEY_MSK + CLS_KEY_MSK_IPV6_DA
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV6_DA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_L4_PORT);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV6_DA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_L4_VLD);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV6_DA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_IPV6_FLOW_LBL);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV6_DA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_IP_PROTOCOL);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV6_DA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_DSCP);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV6_DA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_TOP_DEI);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV6_DA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_TOP_VL_802_1P);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV6_DA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_TOP_VID);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV6_DA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_VLAN_CNT);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_IPV6_DA_SHORT_KEY_MSK] |= (1<<ACL_FIELD_IPV6_DA);

	//aclTemplate[6]: CL_SPCL_PKT_KEY_MSK
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_SPCL_PKT_KEY_MSK] |= (1<<ACL_FIELD_IP_PROTOCOL);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_SPCL_PKT_KEY_MSK] |= (1<<ACL_FIELD_TOP_DEI);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_SPCL_PKT_KEY_MSK] |= (1<<ACL_FIELD_TOP_VL_802_1P);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_SPCL_PKT_KEY_MSK] |= (1<<ACL_FIELD_TOP_VID);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_SPCL_PKT_KEY_MSK] |= (1<<ACL_FIELD_VLAN_CNT);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_SPCL_PKT_KEY_MSK] |= (1<<ACL_FIELD_ETHERTYPE_ENC);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_SPCL_PKT_KEY_MSK] |= (1<<ACL_FIELD_MAC_DA);

	//aclTemplate[7]: CL_MCST_MAC_DA_KEY_MSK = CL_MCST_KEY_MSK + CLS_KEY_MSK_MAC_DA
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_MCST_MAC_DA_KEY_MSK] |= (1<<ACL_FIELD_IP_VER);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_MCST_MAC_DA_KEY_MSK] |= (1<<ACL_FIELD_TOP_DEI);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_MCST_MAC_DA_KEY_MSK] |= (1<<ACL_FIELD_TOP_VL_802_1P);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_MCST_MAC_DA_KEY_MSK] |= (1<<ACL_FIELD_TOP_VID);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_MCST_MAC_DA_KEY_MSK] |= (1<<ACL_FIELD_VLAN_CNT);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_MCST_MAC_DA_KEY_MSK] |= (1<<ACL_FIELD_MAC_DA);

	//aclTemplate[8]: CL_MCST_IP_DA_KEY_MSK = CL_MCST_KEY_MSK + CLS_KEY_MSK_IPV4_DA + CLS_KEY_MSK_IPV6_DA
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_MCST_IP_DA_KEY_MSK] |= (1<<ACL_FIELD_IP_VER);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_MCST_IP_DA_KEY_MSK] |= (1<<ACL_FIELD_TOP_DEI);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_MCST_IP_DA_KEY_MSK] |= (1<<ACL_FIELD_TOP_VL_802_1P);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_MCST_IP_DA_KEY_MSK] |= (1<<ACL_FIELD_TOP_VID);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_MCST_IP_DA_KEY_MSK] |= (1<<ACL_FIELD_VLAN_CNT);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_MCST_IP_DA_KEY_MSK] |= (1<<ACL_FIELD_IPV4_DA);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_MCST_IP_DA_KEY_MSK] |= (1<<ACL_FIELD_IPV6_DA);

	//aclTemplate[9]: CL_FULL_KEY_MSK
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<ACL_FIELD_L4_PORT);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<ACL_FIELD_L4_VLD);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<ACL_FIELD_IPV6_DA);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<ACL_FIELD_IPV6_SA);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<ACL_FIELD_IPV4_DA);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<ACL_FIELD_IPV4_SA);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<ACL_FIELD_IP_PROTOCOL);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<ACL_FIELD_DSCP);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<ACL_FIELD_IP_VER);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<ACL_FIELD_TOP_DEI);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<ACL_FIELD_TOP_VL_802_1P);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<ACL_FIELD_TOP_VID);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<ACL_FIELD_VLAN_CNT);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<ACL_FIELD_ETHERTYPE_ENC);
	rg_db.systemGlobal.ca_cls_template[ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<ACL_FIELD_MAC_SA);

	return (RT_ERR_RG_OK);

}

int _rtk_rg_classify_asic_init(void)
{

	//enabled PON port as CF port
	//postpone the enabled or not until add Wan interface.
	//ASSERT_EQ(RTK_CLASSIFY_CFSEL_SET(RTK_RG_MAC_PORT_PON, CLASSIFY_CF_SEL_ENABLE),RT_ERR_OK);

	return (RT_ERR_RG_OK);
}

/*(2)RG_ACL APIs and internal APIs*/
static int _rtk_rg_rearrange_ACL_weight(int *accumulateIdx)
{
	int i,j;
	int temp,sp;

	//1-1. clean the sorting record of type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
	*accumulateIdx = 0;
	memset(rg_db.systemGlobal.acl_SWindex_sorting_by_weight, -1, sizeof(rg_db.systemGlobal.acl_SWindex_sorting_by_weight));
	memset(rg_db.systemGlobal.acl_SWindex_sorting_by_weight_and_ingress_cvid_action, -1, sizeof(rg_db.systemGlobal.acl_SWindex_sorting_by_weight_and_ingress_cvid_action));

	//1-2. record the rule which type is ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
    for(i=0, *accumulateIdx=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
    {
		if(rg_db.systemGlobal.acl_filter_temp_valid[(i>>5)] == 0x0)
		{
			i+=31;
			continue;
		}
		else if((rg_db.systemGlobal.acl_filter_temp_valid[(i>>5)]&(1<<(i&0x1f))) == 0x0)
			continue;
		else
        {
        	if(rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
			{
            	//ASSERT_EQ(_rtk_rg_aclSWEntry_reAdd(&rg_db.systemGlobal.acl_filter_temp[i], &i),RT_ERR_RG_OK);
				rg_db.systemGlobal.acl_SWindex_sorting_by_weight[*accumulateIdx] = i; //record the rule which type is ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
				(*accumulateIdx)++;
        	}
		}
    }

	//1-3. sorting the rule by weight: BubSort
	for (i=MAX_ACL_SW_ENTRY_SIZE-1; i>0; i--){
		sp=1;
	   	for (j=0; j<=i; j++){
			//check data is valid
			if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]==-1 || rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1]==-1)
				break;

		  	if (rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j])].acl_weight <
				rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1])].acl_weight) //compare the weight
		 	{
		 		//swap the rule index
			   	temp = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j];
			   	rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j] = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1];
			   	rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1] = temp;
			   	sp=0;
		 	}
	   	}
		if (sp==1) //if no swap happened, then no need to check the lower array index(they are already sorted).
			break;
	}

	return RT_ERR_RG_OK;
}

static int _rtk_rg_rearrange_ACL_weight_for_l34_trap_drop_permit(int *accumulateIdx)
{
	int i,j;
	int temp,sp;
	int32 sort_start=0, sort_end=0;

	//1-1. record the continue and sorting start index from rg_db.systemGlobal.acl_SWindex_sorting_by_weight[],
	sort_start = *accumulateIdx;


	// 1-2. find out the L34 trap/drop/permit rules, and put index to rg_db.systemGlobal.acl_SWindex_sorting_by_weight[] first
	for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
	{
		if(rg_db.systemGlobal.acl_filter_temp_valid[(i>>5)] == 0x0)
		{
			i+=31;
			continue;
		}
		else if((rg_db.systemGlobal.acl_filter_temp_valid[(i>>5)]&(1<<(i&0x1f))) == 0x0)
			continue;
		else
		{
			if(rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP ||
				rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP ||
				rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP ||
				rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP ||
				rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT ||
				rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_PERMIT)
			{
					rg_db.systemGlobal.acl_SWindex_sorting_by_weight[*accumulateIdx]=i;
					(*accumulateIdx)++;
			}
		}
	}

	//1-3. record the sorting end index,  the soring valied range is from rg_db.systemGlobal.acl_SWindex_sorting_by_weight[sort_start~sort_end]
	sort_end = *accumulateIdx;


	//1-4. sorting the rule from sort_start till end by weight: BubSort
	for (i=(MAX_ACL_SW_ENTRY_SIZE-2); i>sort_start; i--){	//prevent Overrunning array
		sp=1;
	   	for (j=sort_start; j<=i; j++){
			//check data is valid
			if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]==-1 || rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1]==-1)
				break;

		  	if (rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j])].acl_weight <
				rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1])].acl_weight) //compare the weight
		 	{
		 		//swap the rule index
			   	temp = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j];
			   	rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j] = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1];
			   	rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1] = temp;
			   	sp=0;
		 	}
	   	}
		if (sp==1) //if no swap happened, then no need to check the lower array index(they are already sorted).
			break;
	}

	return RT_ERR_RG_OK;
}

static int _rtk_rg_rearrange_ACL_weight_for_l34_Qos(int *accumulateIdx)
{
	int i,j;
	int temp,sp;
	int32 sort_start=0, sort_end=0;

	//1-1. record the continue and sorting start index from rg_db.systemGlobal.acl_SWindex_sorting_by_weight[],
	sort_start = *accumulateIdx;


	// 1-2. find out the L34 QoS rules, and put index to rg_db.systemGlobal.acl_SWindex_sorting_by_weight[] first
	for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
	{
		if(rg_db.systemGlobal.acl_filter_temp_valid[(i>>5)] == 0x0)
		{
			i+=31;
			continue;
		}
		else if((rg_db.systemGlobal.acl_filter_temp_valid[(i>>5)]&(1<<(i&0x1f))) == 0x0)
			continue;
		else
		{
			if(rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN ||
				rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN)
			{
					rg_db.systemGlobal.acl_SWindex_sorting_by_weight[*accumulateIdx]=i;
					(*accumulateIdx)++;
			}
		}
	}

	//1-3. record the sorting end index,  the soring valied range is from rg_db.systemGlobal.acl_SWindex_sorting_by_weight[sort_start~sort_end]
	sort_end = *accumulateIdx;


	//1-4. sorting the rule from sort_start till end by weight: BubSort
	for (i=(MAX_ACL_SW_ENTRY_SIZE-2); i>sort_start; i--){	//prevent Overrunning array
		sp=1;
		for (j=sort_start; j<=i; j++){
			//check data is valid
			if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]==-1 || rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1]==-1)
				break;

			if (rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j])].acl_weight <
				rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1])].acl_weight) //compare the weight
			{
				//swap the rule index
				temp = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j];
				rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j] = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1];
				rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1] = temp;
				sp=0;
			}
		}
		if (sp==1) //if no swap happened, then no need to check the lower array index(they are already sorted).
			break;
	}

	return RT_ERR_RG_OK;
}


static int _rtk_rg_aclSWEntry_and_asic_reAdd(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx)
{
	int ret;
    bzero(&rg_db.systemGlobal.aclSWEntry,sizeof(rg_db.systemGlobal.aclSWEntry));

    //check input parameter
    if(acl_filter == NULL
            || acl_filter_idx==NULL)
        RETURN_ERR(RT_ERR_RG_NULL_POINTER);

    if(acl_filter->filter_fields == 0x0)
        RETURN_ERR(RT_ERR_RG_INITPM_UNINIT);

	ACL_CTRL("========reAdd RG_ACL[%d]========",*acl_filter_idx);

    //check the rg_db.systemGlobal.aclSWEntry has been used, reAdd must add aclSWEtry in the assigned acl_filter_idx
    ASSERT_EQ(_rtk_rg_aclSWEntry_get(*acl_filter_idx, &rg_db.systemGlobal.aclSWEntry),RT_ERR_RG_OK);

    if(memcmp(&rg_db.systemGlobal.aclSWEntry.acl_filter,acl_filter,sizeof(rtk_rg_aclFilterAndQos_t)))
    {
        RETURN_ERR(RT_ERR_RG_ACL_SW_ENTRY_USED);
    }

	//update HW info above COUNT_HWACL_LENGTH_FIELD
#if 1
	//try to add pure ACL H/W drop rule to ASIC for saving CPU resource
		//below case will stop the ACL H/W drop rule adding.
		//(a) any permit/trap/trap_to_PS rule with not pattern or with egress or H/W ACL not directly suppport pattern => stop any rest drop rule add to HW
		//(b) drop with not pattern => skip adding(H/W not support, keep S/W drop)
		//(c) drop with egress or H/W ACL not directly suppport pattern => skip adding(H/W not support, keep S/W drop)
	if(rg_db.systemGlobal.stop_add_hw_acl==0 && ACTION_CHECK_HW_NEED(acl_filter))
	{
		if(acl_filter->action_type==ACL_ACTION_TYPE_TRAP_WITH_PRIORITY)
		{
			//Only work on HW-ACL, so skip byPassAddHwAclCheck
		}
		else if(acl_filter->filter_fields & ~(HW_ACL_SUPPORT_PATTERN))
		{
			if(ACTION_CHECK_STOP_HW(acl_filter))
			{
				ACL_CTRL("RG_ACL[%d] stop rest ACL rule add to H/W ACL, due to include H/W ACL not support pattern",*acl_filter_idx);
				rg_db.systemGlobal.stop_add_hw_acl=*acl_filter_idx+1;
				goto addHwAclEnd;
			}else{//drop rule will just by pass itself
				ACL_CTRL("RG_ACL[%d] drop rule skip to HW, due to include H/W ACL not support pattern",*acl_filter_idx);
				goto addHwAclEnd;
			}
		}
		else if(acl_filter->filter_fields_inverse!=0x0)
		{
			if(ACTION_CHECK_STOP_HW(acl_filter))
			{
				ACL_CTRL("RG_ACL[%d] stop rest ACL rule add to H/W ACL, due to include not pattern",*acl_filter_idx);
				rg_db.systemGlobal.stop_add_hw_acl=*acl_filter_idx+1;
				goto addHwAclEnd;
			}else{//drop rule will just by pass itself
				ACL_CTRL("RG_ACL[%d] drop rule skip to HW, due to include not pattern",*acl_filter_idx);
				goto addHwAclEnd;
			}
		}
		else if(_rtk_rg_aclHWEntry_not_support(acl_filter))
		{
			if(ACTION_CHECK_STOP_HW(acl_filter))
			{
				ACL_CTRL("RG_ACL[%d] stop rest ACL rule add to H/W ACL, due to include H/W ACL not support case",*acl_filter_idx);
				rg_db.systemGlobal.stop_add_hw_acl=*acl_filter_idx+1;
				goto addHwAclEnd;
			}else{//drop rule will just by pass itself
				ACL_CTRL("RG_ACL[%d] drop rule skip to HW, due to include H/W ACL not support case",*acl_filter_idx);
				goto addHwAclEnd;
			}
		}


		//need sub function to add to H/W ACL, and a clear API to call first.
		//G3 should share SWAclIdx to HW so that it can maintan related information in ca_cls_rule_record structure
		ret = _rtk_rg_aclSWEntry_to_asic_add(acl_filter,&rg_db.systemGlobal.aclSWEntry,*acl_filter_idx);
		if(ret != RT_ERR_RG_OK)
		{
			ACL_CTRL("RG_ACL[%d] add to H/W ACL fail(ret=0x%x), stop rest ACL rule with forwarding action add to H/W ACL",*acl_filter_idx, ret);
			rg_db.systemGlobal.stop_add_hw_acl=*acl_filter_idx+1;
			if(acl_filter->action_type==ACL_ACTION_TYPE_TRAP_WITH_PRIORITY)
			{
				ACL_CTRL("RG_ACL[%d] Only work on HW-ACL, but this rule fail to add asic(ret=0x%x).",*acl_filter_idx, ret);
				WARNING("RG_ACL[%d] Only work on HW-ACL, but this rule fail to add asic(ret=0x%x).",*acl_filter_idx, ret);
				rg_db.systemGlobal.stop_add_acl = TRUE;
			}
		}
		else
		{
			ACL_CTRL("RG_ACL[%d] rule add to HW",*acl_filter_idx);
		}

	}
	else if(rg_db.systemGlobal.stop_add_hw_acl>0 && (acl_filter->action_type==ACL_ACTION_TYPE_TRAP_WITH_PRIORITY))
	{
		ACL_CTRL("RG_ACL[%d] ATTENTION: This rule only work on HW-ACL, but stop_add_hw_acl=1.",*acl_filter_idx);
		WARNING("RG_ACL[%d] ATTENTION: This rule only work on HW-ACL, but stop_add_hw_acl=1.",*acl_filter_idx);
		rg_db.systemGlobal.stop_add_acl = TRUE;
	}
addHwAclEnd:

#endif


    //reAdd must add aclSWEtry in the assigned acl_filter_idx
    ASSERT_EQ(_rtk_rg_aclSWEntry_set(*acl_filter_idx,rg_db.systemGlobal.aclSWEntry),RT_ERR_RG_OK);

    //_rtk_rg_aclSWEntry_dump();

    return (RT_ERR_RG_OK);
}


int _rtk_rg_g3_caClsAssignPortType(ca_classifier_rule_t *cfg, ca_port_id_t port)
{
	/* L2 CLS need to use different port type for different port */
	ca_classifier_key_t *key;
	ca_classifier_key_mask_t *key_mask;

	key = &(cfg->key);
	key_mask = &(cfg->key_mask);

	if(port >= AAL_LPORT_CPU_0) {
		key->src_port = CA_PORT_ID(CA_PORT_TYPE_CPU, port);
	}
#if defined(CONFIG_ARCH_CORTINA_G3HGU)
	else if(port == AAL_LPORT_ETH_NI7) {
		key->src_port = CA_PORT_ID(CA_PORT_TYPE_GPON, port);
		key->key_handle.flow_id = CA_UINT16_INVALID;
		key->key_handle.gem_index = CA_UINT16_INVALID;
		key->key_handle.llid_cos_index = CA_UINT16_INVALID;
		key_mask->key_handle = TRUE;
	}
#endif
	else {
		key->src_port = CA_PORT_ID(CA_PORT_TYPE_ETHERNET, port);
	}
	key_mask->src_port = TRUE;
	//ACL_RSV("port=%d, src_port=0x%x, mask=%d", port, cfg->key.src_port, cfg->key_mask.src_port);

	return (RT_ERR_RG_OK);
}

#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
int _rtk_rg_g3_caClsAssignDestPortType(ca_classifier_rule_t *cfg, ca_port_id_t port)
{
	/* L2 CLS need to use different port type for different port */
	ca_classifier_key_t *key;
	ca_classifier_key_mask_t *key_mask;

	key = &(cfg->key);
	key_mask = &(cfg->key_mask);

	if(port >= AAL_LPORT_CPU_0) {
		key->dest_port = CA_PORT_ID(CA_PORT_TYPE_CPU, port);
	}
	else {
		key->dest_port = CA_PORT_ID(CA_PORT_TYPE_ETHERNET, port);
	}
	key_mask->dest_port = TRUE;
	//ACL_RSV("port=%d, dest_port=0x%x, mask=%d", port, cfg->key.dest_port, cfg->key_mask.dest_port);

	return (RT_ERR_RG_OK);
}

int _rtk_rg_g3_caClsRulePortVlanTransform(ca_classifier_rule_t *cfg, ca_classifier_key_t *backup_key, ca_classifier_key_mask_t *backup_key_mask)
{
	//L3 CLS for port 0/1/2/3 should use fake vlan as pattern due to they will be loopback from port 6 with fake vlan.
	int port_idx;
	port_idx = PORT_ID(cfg->key.src_port);

	if((cfg->key_mask.src_port) && (PORT_TYPE(cfg->key.src_port)==CA_PORT_TYPE_L3) && ((0x1<<port_idx)&G3_LOOPBACK_P_MASK))
	{
		//back up original key and key_mask
		bzero(backup_key,sizeof(ca_classifier_key_t));
		bzero(backup_key_mask,sizeof(ca_classifier_key_mask_t));
		memcpy(backup_key,&cfg->key,sizeof(ca_classifier_key_t));
		memcpy(backup_key_mask,&cfg->key_mask,sizeof(ca_classifier_key_mask_t));

		//if original use vlan pattern, move it to inner vlan
		if(cfg->key_mask.l2 && cfg->key_mask.l2_mask.vlan_otag)
		{
			cfg->key_mask.l2_mask.vlan_itag = TRUE;
			memcpy(&cfg->key_mask.l2_mask.vlan_itag_mask,&cfg->key_mask.l2_mask.vlan_otag_mask,sizeof(ca_classifier_vlan_mask_t));
			memcpy(&cfg->key.l2.vlan_itag,&cfg->key.l2.vlan_otag,sizeof(ca_classifier_vlan_range_t));
			bzero(&cfg->key_mask.l2_mask.vlan_otag_mask,sizeof(ca_classifier_vlan_mask_t));
			bzero(&cfg->key.l2.vlan_otag,sizeof(ca_classifier_vlan_range_t));
		}

		//config outer fake vlan as key to regonize from which port
		cfg->key_mask.l2 = TRUE;
		cfg->key_mask.l2_mask.vlan_otag = TRUE;
		cfg->key_mask.l2_mask.vlan_otag_mask.vid = TRUE;
		switch(port_idx)
		{
			case AAL_LPORT_ETH_NI0:
				cfg->key.l2.vlan_otag.vlan_min.vid = G3_LOOPBACK_UPSTREAM_VID_P0;
				cfg->key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, G3_LOOPBACK_P_NEWSPA);
				break;
			case AAL_LPORT_ETH_NI1:
				cfg->key.l2.vlan_otag.vlan_min.vid = G3_LOOPBACK_UPSTREAM_VID_P1;
				cfg->key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, G3_LOOPBACK_P_NEWSPA);
				break;
			case AAL_LPORT_ETH_NI2:
				cfg->key.l2.vlan_otag.vlan_min.vid = G3_LOOPBACK_UPSTREAM_VID_P2;
				cfg->key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, G3_LOOPBACK_P_NEWSPA);
				break;
			case AAL_LPORT_ETH_NI3:
				cfg->key.l2.vlan_otag.vlan_min.vid = G3_LOOPBACK_UPSTREAM_VID_P3;
				cfg->key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, G3_LOOPBACK_P_NEWSPA);
				break;
			default:
				WARNING("unknown port index %d for transform POL OFFLOAD VLAN.", port_idx);
				break;
		}
		cfg->key.l2.vlan_otag.vlan_max = cfg->key.l2.vlan_otag.vlan_min;
		ACL_RSV("transform port %d to fake vlan 0x%x %s inner vlan", port_idx, cfg->key.l2.vlan_otag.vlan_max, cfg->key_mask.l2_mask.vlan_itag?"with":"without");
	}

	return (RT_ERR_RG_OK);
}

int _rtk_rg_g3_caClsRulePortVlanRollback(ca_classifier_rule_t *cfg, ca_classifier_key_t *backup_key, ca_classifier_key_mask_t *backup_key_mask)
{
	//Rollback original L3 CLS rule.
	int port_idx;
	port_idx = PORT_ID(cfg->key.src_port);

	if((cfg->key_mask.src_port) && (PORT_TYPE(cfg->key.src_port)==CA_PORT_TYPE_L3) && (port_idx==G3_LOOPBACK_P_NEWSPA))
	{
		memcpy(&cfg->key,backup_key,sizeof(ca_classifier_key_t));
		memcpy(&cfg->key_mask,backup_key_mask,sizeof(ca_classifier_key_mask_t));
		ACL_RSV("rollback from port 0x%x to port 0x%x", backup_key->src_port, cfg->key.src_port);
	}

	return (RT_ERR_RG_OK);
}
#endif

int _rtk_rg_g3_caClsAssignRuleRefIdx(ca_classifier_rule_t *cfg, int ruleType, char *debug_str, int debug_str_size)
{
	//reserve mdata_w_2 to ACL, ca api will transform sw_id[0] to mdata_w_2
	if(ruleType == FAIL){
		snprintf(debug_str, debug_str_size, "pre-add test");
	}else if(ruleType >= RTK_RG_ACLANDCF_RESERVED_TAIL_END){
		snprintf(debug_str, debug_str_size, "SW_ACL[%d]", ruleType - RTK_RG_ACLANDCF_RESERVED_TAIL_END);
		if((cfg->key_mask.src_port) && (PORT_TYPE(cfg->key.src_port)==CA_PORT_TYPE_L3))
		{
			cfg->action.options.sw_id[0] = ruleType - RTK_RG_ACLANDCF_RESERVED_TAIL_END;
			cfg->action.options.sw_id[0] |= RXINFO_REF_ACL_RSN_CTRL_USER;
			if((cfg->action.forward == CA_CLASSIFIER_FORWARD_PORT) && ((1<<(cfg->action.dest.port)) & RTK_RG_ALL_MAC_CPU_PORTMASK))
			{
				cfg->action.options.sw_id[0] |= RXINFO_REF_ACL_RSN_CTRL_TRAP;
				cfg->action.options.sw_id[0] |= RXINFO_REF_TRAP_RSN_ACL<<RXINFO_REF_TRAP_RSN_SHIFT_BIT;
			}
			cfg->action.options.masks.sw_id = TRUE;
		}
	}else{
		snprintf(debug_str, debug_str_size, "RSV_ACL[%d]", ruleType);
		if((cfg->key_mask.src_port) && (PORT_TYPE(cfg->key.src_port)==CA_PORT_TYPE_L3))
		{
			cfg->action.options.sw_id[0] = ruleType;
			cfg->action.options.sw_id[0] |= RXINFO_REF_ACL_RSN_CTRL_RSV;
			if((cfg->action.forward == CA_CLASSIFIER_FORWARD_PORT) && ((1<<(cfg->action.dest.port)) & RTK_RG_ALL_MAC_CPU_PORTMASK))
			{
				cfg->action.options.sw_id[0] |= RXINFO_REF_ACL_RSN_CTRL_TRAP;
				cfg->action.options.sw_id[0] |= RXINFO_REF_TRAP_RSN_ACL<<RXINFO_REF_TRAP_RSN_SHIFT_BIT;
			}
			cfg->action.options.masks.sw_id = TRUE;
		}
	}

	return (RT_ERR_RG_OK);
}

int _rtk_rg_g3_caClsRuleAdd(ca_classifier_rule_t *cfg, int ruleType)
{
	ca_status_t ret = CA_E_OK;
	char string_buf[16];

	_rtk_rg_g3_caClsAssignRuleRefIdx(cfg, ruleType, string_buf, sizeof(string_buf));

	if((cfg->key_mask.src_port) && (PORT_TYPE(cfg->key.src_port)==CA_PORT_TYPE_L3))
		WARNING("This API not support L3 CLS rule!!!(%s with ruleType=%d)", string_buf, ruleType);

	ret = ca_classifier_rule_add(G3_DEF_DEVID, cfg->priority, &(cfg->key), &(cfg->key_mask), &(cfg->action), &(cfg->index));
	if(ret != CA_E_OK)
	{
		if(ret == CA_E_FULL) {
			ACL_RSV("add CA CLS (%s) failed due to CA ACL FULL", string_buf);
		}else if(ret == CA_E_PARAM) {
			ACL_RSV("add CA CLS (%s) failed due to CA parsing parameter error", string_buf);
			WARNING("add CA CLS (%s) failed due to CA parsing parameter error", string_buf);
		} else if(ret == CA_E_NOT_SUPPORT) {
			ACL_RSV("add CA CLS (%s) failed due to CA NOT SUPPORT", string_buf);
			WARNING("add CA CLS (%s) failed due to CA NOT SUPPORT", string_buf);
		} else {
			ACL_RSV("add CA CLS (%s) failed!!!(ca_ret=0x%x)", string_buf, ret);
			WARNING("add CA CLS (%s) failed!!!(ca_ret=0x%x)", string_buf, ret);
		}
		return ret;
	}
	else if(cfg->index >= MAX_ACL_CA_CLS_RULE_SIZE)
	{
		ACL_RSV("add CA CLS (%s) failed!!!(cls_index=%d, ca_ret=%d)", string_buf, cfg->index, ret);
		ASSERT_EQ(ca_classifier_rule_delete(G3_DEF_DEVID, cfg->index), CA_E_OK);
		WARNING("add CA CLS (%s) failed!!!(cls_index=%d, ca_ret=%d)", string_buf, cfg->index, ret);
		return (RT_ERR_RG_FAILED);
	}

	rg_db.systemGlobal.ca_cls_rule_record[cfg->index].valid = ENABLED;
	rg_db.systemGlobal.ca_cls_rule_record[cfg->index].priority = cfg->priority;
	rg_db.systemGlobal.ca_cls_rule_record[cfg->index].ruleType = ruleType;
	rg_db.systemGlobal.ca_cls_used_count++;

	return (RT_ERR_RG_OK);
}

int _rtk_rg_g3_caClsRuleAALAdd(ca_classifier_rule_t *cfg, int ruleType, unsigned int aal_customize_reason)
{
	ca_status_t ret = CA_E_OK;
	char string_buf[16];
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
	ca_classifier_key_t backup_key;
	ca_classifier_key_mask_t backup_key_mask;
#endif

	_rtk_rg_g3_caClsAssignRuleRefIdx(cfg, ruleType, string_buf, sizeof(string_buf));

#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
	if((aal_customize_reason&CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT)==0x0)
		_rtk_rg_g3_caClsRulePortVlanTransform(cfg, &backup_key, &backup_key_mask);
#endif

	ret = ca_classifier_l3_cls_rule_add(G3_DEF_DEVID, cfg->priority, &(cfg->key), &(cfg->key_mask), &(cfg->action), &(cfg->index), aal_customize_reason);
	if(ret != CA_E_OK)
	{
		if(ret == CA_E_FULL) {
			ACL_RSV("add CA CLS (%s) failed due to CA ACL FULL", string_buf);
		}else if(ret == CA_E_PARAM) {
			ACL_RSV("add CA CLS (%s) failed due to CA parsing parameter error", string_buf);
			WARNING("add CA CLS (%s) failed due to CA parsing parameter error", string_buf);
		} else if(ret == CA_E_NOT_SUPPORT) {
			ACL_RSV("add CA CLS (%s) failed due to CA NOT SUPPORT", string_buf);
			WARNING("add CA CLS (%s) failed due to CA NOT SUPPORT", string_buf);
		} else {
			ACL_RSV("add CA CLS (%s) failed!!!(ca_ret=0x%x)", string_buf, ret);
			WARNING("add CA CLS (%s) failed!!!(ca_ret=0x%x)", string_buf, ret);
		}
		return ret;
	}
	else if(cfg->index >= MAX_ACL_CA_CLS_RULE_SIZE)
	{
		ACL_RSV("add CA CLS (%s) failed!!!(cls_index=%d)", string_buf, cfg->index);
		ASSERT_EQ(ca_classifier_rule_delete(G3_DEF_DEVID, cfg->index), CA_E_OK);
		WARNING("add CA CLS (%s) failed!!!(cls_index=%d)", string_buf, cfg->index);
		return (RT_ERR_RG_FAILED);
	}

#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
	_rtk_rg_g3_caClsRulePortVlanRollback(cfg, &backup_key, &backup_key_mask);
#endif

	rg_db.systemGlobal.ca_cls_rule_record[cfg->index].valid = ENABLED;
	rg_db.systemGlobal.ca_cls_rule_record[cfg->index].priority = cfg->priority;
	rg_db.systemGlobal.ca_cls_rule_record[cfg->index].ruleType = ruleType;
	rg_db.systemGlobal.ca_cls_rule_record[cfg->index].aal_customize = aal_customize_reason;
	rg_db.systemGlobal.ca_cls_used_count++;

	return (RT_ERR_RG_OK);
}

int _rtk_rg_g3_caClsRuleDelete(rtk_rg_g3_caRulePriority_t priority)
{
	int i;

	for(i = 0; i < MAX_ACL_CA_CLS_RULE_SIZE; i++)
	{
		if((rg_db.systemGlobal.ca_cls_rule_record[i].valid == ENABLED) && (rg_db.systemGlobal.ca_cls_rule_record[i].priority == priority))
		{
			ASSERT_EQ(ca_classifier_rule_delete(G3_DEF_DEVID, i), CA_E_OK);
			bzero(&rg_db.systemGlobal.ca_cls_rule_record[i],sizeof(rtk_rg_g3_cls_entry_t));
			rg_db.systemGlobal.ca_cls_used_count--;
		}
	}
	return (RT_ERR_RG_OK);
}

int _rtk_rg_g3_caClsRuleDeleteReserveType(int ruleType)
{
	int i;

	for(i = 0; i < MAX_ACL_CA_CLS_RULE_SIZE; i++)
	{
		if((rg_db.systemGlobal.ca_cls_rule_record[i].valid == ENABLED) && (rg_db.systemGlobal.ca_cls_rule_record[i].ruleType == ruleType))
		{
			ASSERT_EQ(ca_classifier_rule_delete(G3_DEF_DEVID, i), CA_E_OK);
			bzero(&rg_db.systemGlobal.ca_cls_rule_record[i],sizeof(rtk_rg_g3_cls_entry_t));
			rg_db.systemGlobal.ca_cls_used_count--;
		}
	}
	return (RT_ERR_RG_OK);
}

int _rtk_rg_g3_caSpecialPacketTrap(rtk_rg_g3_special_pkt_rule_t *cfg, int ruleType)
{
	ca_status_t ret = CA_E_OK;
	ca_port_id_t ca_cpu_port = CA_PORT_ID(CA_PORT_TYPE_CPU, RTK_RG_PORT_CPU);
	ca_special_packet_option_t act_opt;
	
	bzero(&act_opt, sizeof(act_opt));
	act_opt.mask.action_handle = FALSE;
	act_opt.action_handle.flow_id = CA_FLOW_ID_INVALID;
	//ACL_RSV("src_port=0x%x, spec=0x%x, cpu_port=0x%x, pri=%d", cfg->source_port, cfg->special_packet, ca_cpu_port, cfg->priority);

	ret = ca_special_packet_set(G3_DEF_DEVID, cfg->source_port, cfg->special_packet, TRUE, ca_cpu_port, cfg->priority, &act_opt);
	if(ret != CA_E_OK)
	{
		ACL_RSV("add CA Special Packet (RuleType=%d) failed!!!(ca_ret=%d)", ruleType, ret);
		WARNING("add CA Special Packet (RuleType=%d) failed!!!(ca_ret=%d)", ruleType, ret);
		return ret;
	}
	rg_db.systemGlobal.ca_special_pkt_record[cfg->special_packet].valid = ENABLED;
	rg_db.systemGlobal.ca_special_pkt_record[cfg->special_packet].ruleType = ruleType;

	return (RT_ERR_RG_OK);
}


static int _rtk_rg_aclHWEntry_maskLength_get_by_ipv4_range(ipaddr_t ipv4_addr_start, ipaddr_t ipv4_addr_end, int* maskLength)
{
	int i;
	int length=0;

	for(i=0;i<32;i++){
		if(((ipv4_addr_start<<i)&0x80000000)!=((ipv4_addr_end<<i)&0x80000000)){
			length = i;
			break;
		}
	}

	//check rest bit in range
	for(i=length;i<32;i++){
		if(((ipv4_addr_start<<i)&0x80000000)!=0){ACL_CTRL("IP Range not in mask alignment \n"); return(RT_ERR_RG_FAILED);}
		if(((ipv4_addr_end<<i)&0x80000000)!=0x80000000){ACL_CTRL("IP Range not in mask alignment \n"); return(RT_ERR_RG_FAILED);}
	}

	*maskLength = (32-length);

	return (RT_ERR_RG_OK);

}

static int _rtk_rg_aclHWEntry_maskLength_get_by_ipv6_mask(uint8* ipv6_mask, int* maskLength)
{
	int i,j,k;
	int length;

	if(ipv6_mask[0]==0xff && ipv6_mask[1]==0xff && ipv6_mask[2]==0xff && ipv6_mask[3]==0xff &&
		ipv6_mask[4]==0xff && ipv6_mask[5]==0xff && ipv6_mask[6]==0xff && ipv6_mask[7]==0xff &&
		ipv6_mask[8]==0xff && ipv6_mask[9]==0xff &&	ipv6_mask[10]==0xff && ipv6_mask[11]==0xff &&
		ipv6_mask[12]==0xff && ipv6_mask[13]==0xff && ipv6_mask[14]==0xff && ipv6_mask[15]==0xff){
		//SINGLE IP
		*maskLength = 0;

	}else{ //Multiple IP by Mask
		for(i=0;i<16;i++){
			if(ipv6_mask[i]!=0xff){//i: get the first non 0xff byte
				break;
			}
		}
		for(j=0;j<8;j++){ // j: get the first bit 0 of the byte
			if(((ipv6_mask[i]<<j)&0x80)!=0x80)
				break;
		}
		length = (i*8)+j;
		(*maskLength) = 128 - length;

		/*make sure the mask is aligned*/
		for(k=j+1;k<8;k++){
			if(((ipv6_mask[i]<<k)&0x80)!=0x0){ACL_CTRL("IPv6 Range not in mask alignment(Byte %d Bit %d) \n", i, k); return(RT_ERR_RG_FAILED);}//rest bit should be 0
		}
		for(k=i+1;k<16;k++){
			if(ipv6_mask[k]!=0x0){ACL_CTRL("IPv6 Range not in mask alignment(Byte %d) \n", k); return(RT_ERR_RG_FAILED);}//rest bytes should be 0x00
		}
	}

	return (RT_ERR_RG_OK);
}

static int _rtk_rg_aclHWEntry_support(rtk_rg_aclFilterAndQos_t *acl_filter)
{
    int i;
	unsigned long mask = 0;

	if(acl_filter->filter_fields & INGRESS_ETHERTYPE_BIT)
		mask |= (1<<ACL_FIELD_ETHERTYPE_ENC);

	if(acl_filter->filter_fields & INGRESS_CTAG_PRI_BIT)
		mask |= (1<<ACL_FIELD_TOP_VL_802_1P);

	if(acl_filter->filter_fields & INGRESS_CTAG_VID_BIT)
		mask |= (1<<ACL_FIELD_TOP_VID);

	if(acl_filter->filter_fields & INGRESS_CTAG_CFI_BIT )
		mask |= (1<<ACL_FIELD_TOP_DEI);

	if(acl_filter->filter_fields & INGRESS_SMAC_BIT)
		mask |= (1<<ACL_FIELD_MAC_SA);

	if(acl_filter->filter_fields & INGRESS_DMAC_BIT)
		mask |= (1<<ACL_FIELD_MAC_DA);

	if((acl_filter->filter_fields & INGRESS_DSCP_BIT)
		|| (acl_filter->filter_fields & INGRESS_IPV6_DSCP_BIT)) {
		mask |= (1<<ACL_FIELD_IP_VER);
		mask |= (1<<ACL_FIELD_DSCP);
	}

	if((acl_filter->filter_fields & INGRESS_L4_ICMP_BIT)
		|| (acl_filter->filter_fields & INGRESS_L4_ICMPV6_BIT)) {
		mask |= (1<<ACL_FIELD_IP_VER);
		mask |= (1<<ACL_FIELD_IP_PROTOCOL);
	}

	if((acl_filter->filter_fields & INGRESS_L4_TCP_BIT)
		|| (acl_filter->filter_fields & INGRESS_L4_UDP_BIT)
		|| (acl_filter->filter_fields & INGRESS_L4_POROTCAL_VALUE_BIT)) {
		mask |= (1<<ACL_FIELD_IP_PROTOCOL);
	}

	if(acl_filter->filter_fields & INGRESS_CTAGIF_BIT)
		mask |= (1<<ACL_FIELD_VLAN_CNT);

	if(acl_filter->filter_fields & INGRESS_IPV6_SIP_BIT )
		mask |= (1<<ACL_FIELD_IPV6_SA);

	if(acl_filter->filter_fields & INGRESS_IPV6_DIP_BIT )
		mask |= (1<<ACL_FIELD_IPV6_DA);

	if((acl_filter->filter_fields & INGRESS_IPV4_TAGIF_BIT )
		|| (acl_filter->filter_fields & INGRESS_IPV6_TAGIF_BIT)) {
		mask |= (1<<ACL_FIELD_IP_VER);
	}

	if((acl_filter->filter_fields & INGRESS_L4_SPORT_RANGE_BIT)
		|| (acl_filter->filter_fields & INGRESS_L4_DPORT_RANGE_BIT)) {
		mask |= (1<<ACL_FIELD_L4_PORT);
		mask |= (1<<ACL_FIELD_L4_VLD);
	}

	if(acl_filter->filter_fields & INGRESS_IPV4_SIP_RANGE_BIT)
		mask |= (1<<ACL_FIELD_IPV4_SA);

	if(acl_filter->filter_fields & INGRESS_IPV4_DIP_RANGE_BIT)
		mask |= (1<<ACL_FIELD_IPV4_DA);

	if(acl_filter->filter_fields & INGRESS_IPV6_FLOWLABEL_BIT)
		mask |= (1<<ACL_FIELD_IPV6_FLOW_LBL);

	for(i=0; i<ACL_TEMPLATE_CA_END; i++) {
		if (mask == (mask & rg_db.systemGlobal.ca_cls_template[i])) {
			ACL_CTRL("HW template mapping success: mask 0x%llx use template %d(0x%llx)", mask, i, rg_db.systemGlobal.ca_cls_template[i]);
			return (RT_ERR_RG_OK);
		}
	}
	ACL_CTRL("HW ACL not support these pattern(mask=0x%llx), and reset ACL will not add to hw.", mask);

	return(RT_ERR_RG_INVALID_PARAM);
}


static int _rtk_rg_aclHWEntry_not_support(rtk_rg_aclFilterAndQos_t *acl_filter)
{
	int ret;
	int value_check = 0;

	if(acl_filter->filter_fields & INGRESS_ETHERTYPE_BIT)
	{
		if(acl_filter->ingress_ethertype_mask!=0xffff)
		{
			ACL_CTRL("HW ACL not support INGRESS_ETHERTYPE_BIT with mask.");
			return(RT_ERR_RG_INVALID_PARAM);
		}
	}

	if(acl_filter->filter_fields & INGRESS_SMAC_BIT)
	{
		if(!(acl_filter->ingress_smac_mask.octet[0]==0xff &&
			acl_filter->ingress_smac_mask.octet[1]==0xff &&
			acl_filter->ingress_smac_mask.octet[2]==0xff &&
			acl_filter->ingress_smac_mask.octet[3]==0xff &&
			acl_filter->ingress_smac_mask.octet[4]==0xff &&
			acl_filter->ingress_smac_mask.octet[5]==0xff))
		{
			ACL_CTRL("HW ACL not support INGRESS_SMAC_BIT with mask.");
			return(RT_ERR_RG_INVALID_PARAM);
		}
	}

	if(acl_filter->filter_fields & INGRESS_DMAC_BIT)
	{
		if(PATTERN_CHECK_DMAC_IS_MC(acl_filter))
		{
			ACL_CTRL("dmac=dmac_mask=01:00:00:00:00:00 belongs to multicast.");
		}
		else if(PATTERN_CHECK_DMAC_IS_MC_IPV4(acl_filter))
		{
			ACL_CTRL("dmac=01:00:5E:00:00:00, dmac_mask=FF:FF:FF:00:00:00 belongs to ipv4 multicast.");
		}
		else if(PATTERN_CHECK_DMAC_IS_MC_IPV6(acl_filter))
		{
			ACL_CTRL("dmac=33:33:00:00:00:00, dmac_mask=FF:FF:00:00:00:00 belongs to ipv6 multicast.");
		}
		else if(PATTERN_CHECK_DMAC_IS_UC(acl_filter))
		{
			ACL_CTRL("dmac=00:00:00:00:00:00, dmac_mask=01:00:00:00:00:00 belongs to unicast.");
		}
		else if(PATTERN_CHECK_DMAC_IS_RSVD(acl_filter))
		{
			ACL_CTRL("dmac=01:80:C2:00:00:00, dmac_mask=FF:FF:FF:FF:FF:00 belongs to reserved mac.");
		}
		else if(!(acl_filter->ingress_dmac_mask.octet[0]==0xff &&
			acl_filter->ingress_dmac_mask.octet[1]==0xff &&
			acl_filter->ingress_dmac_mask.octet[2]==0xff &&
			acl_filter->ingress_dmac_mask.octet[3]==0xff &&
			acl_filter->ingress_dmac_mask.octet[4]==0xff &&
			acl_filter->ingress_dmac_mask.octet[5]==0xff))
		{
			ACL_CTRL("HW ACL not support INGRESS_DMAC_BIT with mask(only support 01/3333/01005E/0180C20000/UC).");
			return(RT_ERR_RG_INVALID_PARAM);
		}
	}

	if(acl_filter->filter_fields & INGRESS_CTAGIF_BIT)
	{
		if(acl_filter->ingress_ctagIf == 0){
			ACL_CTRL("HW ACL not support INGRESS_CTAGIF_BIT reverse check.");
			return(RT_ERR_RG_INVALID_PARAM);
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_SIP_BIT )
	{
		ret = _rtk_rg_aclHWEntry_maskLength_get_by_ipv6_mask(acl_filter->ingress_src_ipv6_addr_mask,&value_check);
		if(ret != RT_ERR_RG_OK)
		{
			ACL_CTRL("HW ACL not support INGRESS_IPV6_SIP_BIT with mask not aligned.");
			return(RT_ERR_RG_INVALID_PARAM);
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_DIP_BIT )
	{
		ret = _rtk_rg_aclHWEntry_maskLength_get_by_ipv6_mask(acl_filter->ingress_dest_ipv6_addr_mask,&value_check);
		if(ret != RT_ERR_RG_OK)
		{
			ACL_CTRL("HW ACL not support INGRESS_IPV6_DIP_BIT with mask not aligned.");
			return(RT_ERR_RG_INVALID_PARAM);
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV4_TAGIF_BIT )
	{
		if(acl_filter->ingress_ipv4_tagif==0)//must not IPv4
		{
			ACL_CTRL("HW ACL not support INGRESS_IPV4_TAGIF_BIT reverse check.");
			return(RT_ERR_RG_INVALID_PARAM);
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_TAGIF_BIT)
	{
		if(acl_filter->ingress_ipv6_tagif==0)//must not IPv6
		{
			ACL_CTRL("HW ACL not support INGRESS_IPV6_TAGIF_BIT reverse check.");
			return(RT_ERR_RG_INVALID_PARAM);
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV4_SIP_RANGE_BIT)
	{
		if(acl_filter->ingress_src_ipv4_addr_start != acl_filter->ingress_src_ipv4_addr_end)
		{
			ret = _rtk_rg_aclHWEntry_maskLength_get_by_ipv4_range(acl_filter->ingress_src_ipv4_addr_start,acl_filter->ingress_src_ipv4_addr_end,&value_check);
			if(ret != RT_ERR_RG_OK){
				ACL_CTRL("HW ACL not support INGRESS_IPV4_SIP_RANGE_BIT with range not in mask alignment.");
				return(RT_ERR_RG_INVALID_PARAM);
			}
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV4_DIP_RANGE_BIT)
	{
		if(acl_filter->ingress_dest_ipv4_addr_start != acl_filter->ingress_dest_ipv4_addr_end)
		{
			ret = _rtk_rg_aclHWEntry_maskLength_get_by_ipv4_range(acl_filter->ingress_dest_ipv4_addr_start,acl_filter->ingress_dest_ipv4_addr_end,&value_check);
			if(ret != RT_ERR_RG_OK){
				ACL_CTRL("HW ACL not support INGRESS_IPV4_DIP_RANGE_BIT with range not in mask alignment.");
				return(RT_ERR_RG_INVALID_PARAM);
			}
		}
	}

	ret = _rtk_rg_aclHWEntry_support(acl_filter);
	if(ret != RT_ERR_RG_OK)
		return(RT_ERR_RG_ACL_NOT_SUPPORT);

	return (RT_ERR_RG_OK);
}



//G3 only support ACL pattern and drop/permit/trap/ps/trap_with_priority action add to H/W.
static int _rtk_rg_aclSWEntry_to_asic_add(rtk_rg_aclFilterAndQos_t *acl_filter,rtk_rg_aclFilterEntry_t* aclSWEntry,int shareHwAclWithSWAclIdx)
{
	int i,index,ret;
	int aclCountinueRuleSize;
	rtk_portmask_t mac_pmsk;
	rtk_portmask_t ext_pmsk;
	rtk_portmask_t activePorts;
	rtk_portmask_t caCLSPort;
	ca_classifier_rule_t ca_cls_rule;
	char ca_index_tmp_buf[12];
	char ca_index_buffer[128];
	int unmask_length;
	unsigned int aal_customize;
	unsigned int ca_index_hw_info;

	aclSWEntry->hw_aclEntry_count = 0;
	bzero(&activePorts,sizeof(rtk_portmask_t));
	bzero(&caCLSPort,sizeof(rtk_portmask_t));
	bzero(&ca_cls_rule,sizeof(ca_cls_rule));
	bzero(&ca_index_buffer,sizeof(char));
	ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_USER;
	aclCountinueRuleSize=0;
	aal_customize = CA_CLASSIFIER_AAL_NA;

	if(shareHwAclWithSWAclIdx >= 0)
		index = RTK_RG_ACLANDCF_RESERVED_TAIL_END + shareHwAclWithSWAclIdx;
	else
		index = FAIL;//pre-add test

	if(acl_filter->filter_fields == 0x0) //no patterns need to filter
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);

	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)//pure ACK type can not use CF patterns and actions
	{
		//check pattern that hw ACL can support
		if(acl_filter->filter_fields & (~HW_ACL_SUPPORT_PATTERN)){
			ACL_CTRL("rule contained HW ACL not support pattern, skip add to hw.");
			return (RT_ERR_RG_INVALID_PARAM);
		}

		//check the action only for drop
	}
	else
	{
		WARNING("Only ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET support HW_ACL(fwding_type_and_direction %d )", acl_filter->fwding_type_and_direction);
		return(RT_ERR_RG_INVALID_PARAM);
	}

	ASSERT_EQ(_rtk_rg_portmask_translator(acl_filter->ingress_port_mask, &mac_pmsk, &ext_pmsk),RT_ERR_RG_OK);
	ACL_CTRL("PortMask 0x%x: physical port is 0x%x, extension port is 0x%x", acl_filter->ingress_port_mask, mac_pmsk.bits[0], ext_pmsk.bits[0]);

	if(acl_filter->filter_fields & INGRESS_PORT_BIT)
	{
		//G3 should always config activePorts, check activePort under below.
	}
	else if(shareHwAclWithSWAclIdx == FAIL)
	{
		if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
			WARNING("RG_ACL[%d] ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET should assign ingress_port_mask!", shareHwAclWithSWAclIdx);
		else
			WARNING("RG_ACL[%d] Recommend: you should config ingress_port_mask to make sure which port you care, otherwise, we will configure it as all physical port and wifi port!", shareHwAclWithSWAclIdx);
	}

	if(mac_pmsk.bits[0]==0x0 && ext_pmsk.bits[0]==0x0)
	{	//all physical port and wifi port: in L3 CLS, it means profile #lan and profile #wan
		activePorts.bits[0] = CA_L3_CLS_PROFILE_LAN;
		activePorts.bits[0] |= CA_L3_CLS_PROFILE_WAN;
		aal_customize |= CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT;
		ACL_CTRL("= both mac_pmsk/ext_pmsk is null, set acl rule for activePorts = 0x%x =",activePorts.bits[0]);
	}
	else
	{
		if((rg_db.systemGlobal.wanPortMask.portmask != 0x0) && (mac_pmsk.bits[0]&rg_db.systemGlobal.wanPortMask.portmask))
		{	//wan port + lan port
			activePorts.bits[0] = CA_L3_CLS_PROFILE_WAN;
			mac_pmsk.bits[0] &= ~rg_db.systemGlobal.wanPortMask.portmask;
			activePorts.bits[0] |= mac_pmsk.bits[0];
		}
		else if(mac_pmsk.bits[0]!=0x0)
		{	//only lan port
			activePorts = mac_pmsk;
		}
		if(ext_pmsk.bits[0]& RTK_RG_ALL_VLAN_MASTER_EXT_PORTMASK /*master CPU Ext port*/)
		{
			activePorts.bits[0] |= RTK_RG_ALL_REAL_MASTER_EXT_PORTMASK;	/*0x12~0x17 mask*/
		}
		ACL_CTRL("= set acl rule for activePorts = 0x%x =",activePorts.bits[0]);
	}

	if(acl_filter->filter_fields & INGRESS_ETHERTYPE_BIT)
	{
		//HW ACL not support "mask", for compitible of non-mask version (mask not set means all care)
		acl_filter->ingress_ethertype_mask = 0xffff;

		ca_cls_rule.key_mask.l2 = TRUE;
		ca_cls_rule.key_mask.l2_mask.ethertype = TRUE;
		ca_cls_rule.key.l2.ethertype = acl_filter->ingress_ethertype;
		if(acl_filter->ingress_ethertype == 0x8864)
			aal_customize |= CA_CLASSIFIER_AAL_L3_KEEP_ORIG;
	}

	if(acl_filter->filter_fields & INGRESS_CTAG_PRI_BIT)
	{
		ca_cls_rule.key_mask.l2 = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_otag = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.pri = TRUE;
		ca_cls_rule.key.l2.vlan_otag.vlan_min.pri = acl_filter->ingress_ctag_pri;
		ca_cls_rule.key.l2.vlan_otag.vlan_max = ca_cls_rule.key.l2.vlan_otag.vlan_min;
	}

	if(acl_filter->filter_fields & INGRESS_CTAG_VID_BIT)
	{
		ca_cls_rule.key_mask.l2 = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_otag = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.vid = TRUE;
		ca_cls_rule.key.l2.vlan_otag.vlan_min.vid = acl_filter->ingress_ctag_vid;
		ca_cls_rule.key.l2.vlan_otag.vlan_max = ca_cls_rule.key.l2.vlan_otag.vlan_min;
	}

	if(acl_filter->filter_fields & INGRESS_CTAG_CFI_BIT )
	{
		ca_cls_rule.key_mask.l2 = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_otag = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.dei = TRUE;
		ca_cls_rule.key.l2.vlan_otag.vlan_min.dei = acl_filter->ingress_ctag_cfi;
		ca_cls_rule.key.l2.vlan_otag.vlan_max = ca_cls_rule.key.l2.vlan_otag.vlan_min;
	}

	if(acl_filter->filter_fields & INGRESS_SMAC_BIT)
	{
		//HW ACL not support "mask", for compitible of non-mask version (mask not set means all care)
		acl_filter->ingress_smac_mask.octet[0]=0xff;
		acl_filter->ingress_smac_mask.octet[1]=0xff;
		acl_filter->ingress_smac_mask.octet[2]=0xff;
		acl_filter->ingress_smac_mask.octet[3]=0xff;
		acl_filter->ingress_smac_mask.octet[4]=0xff;
		acl_filter->ingress_smac_mask.octet[5]=0xff;

		ca_cls_rule.key_mask.l2 = TRUE;
		ca_cls_rule.key_mask.l2_mask.mac_sa = TRUE;
		memcpy(ca_cls_rule.key.l2.mac_sa.mac_min,acl_filter->ingress_smac.octet,sizeof(ca_mac_addr_t));
		memcpy(ca_cls_rule.key.l2.mac_sa.mac_max,acl_filter->ingress_smac.octet,sizeof(ca_mac_addr_t));
	}

	if(acl_filter->filter_fields & INGRESS_DMAC_BIT)
	{
		//HW ACL not support "mask", for compitible of non-mask version (mask not set means all care)
		if(PATTERN_CHECK_DMAC_IS_MC(acl_filter)){	//DA[40] = 1b1
			ca_cls_rule.key_mask.l2 = TRUE;
			ca_cls_rule.key_mask.l2_mask.is_multicast = TRUE;
			ca_cls_rule.key.l2.is_multicast = TRUE;
		}else if(PATTERN_CHECK_DMAC_IS_MC_IPV4(acl_filter)){
			//((MAC DA[47:24] == h01-00-5E) && (etherType == IPV4)) |((MAC DA[47:32] == h33-33) && (etherType == IPV6))
			ca_cls_rule.key_mask.ip = TRUE;
			ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;
			ca_cls_rule.key.ip.ip_version = 4;
			aal_customize |= CA_CLASSIFIER_AAL_L3_MAC_DA_IP_MC;
		}else if(PATTERN_CHECK_DMAC_IS_MC_IPV6(acl_filter)){
			//((MAC DA[47:24] == h01-00-5E) && (etherType == IPV4)) |((MAC DA[47:32] == h33-33) && (etherType == IPV6))
			ca_cls_rule.key_mask.ip = TRUE;
			ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;
			ca_cls_rule.key.ip.ip_version = 6;
			aal_customize |= CA_CLASSIFIER_AAL_L3_MAC_DA_IP_MC;
		}else if(PATTERN_CHECK_DMAC_IS_UC(acl_filter)){ //DA[40] = 0, DA_mask[40] = 1b1
			ca_cls_rule.key_mask.l2 = TRUE;
			ca_cls_rule.key_mask.l2_mask.is_multicast = TRUE;
			ca_cls_rule.key.l2.is_multicast = FALSE;
			aal_customize |= CA_CLASSIFIER_AAL_L3_NOT_BROADCAST;
		}else if(PATTERN_CHECK_DMAC_IS_RSVD(acl_filter)){	//DA[47:8] = 40h01_80_C2_00_00)
			aal_customize |= CA_CLASSIFIER_AAL_L3_MAC_DA_RSVD;
		}else{
			acl_filter->ingress_dmac_mask.octet[0]=0xff;
			acl_filter->ingress_dmac_mask.octet[1]=0xff;
			acl_filter->ingress_dmac_mask.octet[2]=0xff;
			acl_filter->ingress_dmac_mask.octet[3]=0xff;
			acl_filter->ingress_dmac_mask.octet[4]=0xff;
			acl_filter->ingress_dmac_mask.octet[5]=0xff;

			ca_cls_rule.key_mask.l2 = TRUE;
			ca_cls_rule.key_mask.l2_mask.mac_da = TRUE;
			memcpy(ca_cls_rule.key.l2.mac_da.mac_min,acl_filter->ingress_dmac.octet,sizeof(ca_mac_addr_t));
			memcpy(ca_cls_rule.key.l2.mac_da.mac_max,acl_filter->ingress_dmac.octet,sizeof(ca_mac_addr_t));
		}
	}

	if(acl_filter->filter_fields & INGRESS_DSCP_BIT)
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;
		ca_cls_rule.key.ip.ip_version = 4;
		ca_cls_rule.key_mask.ip_mask.dscp = TRUE;
		ca_cls_rule.key.ip.dscp = acl_filter->ingress_dscp;
	}

	if(acl_filter->filter_fields & INGRESS_L4_ICMP_BIT)
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;
		ca_cls_rule.key.ip.ip_version = 4;
		ca_cls_rule.key_mask.ip_mask.ip_protocol = TRUE;
		ca_cls_rule.key.ip.ip_protocol = 0x1;	//ICMP protocal value 1
	}

	if(acl_filter->filter_fields & INGRESS_L4_TCP_BIT)
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_protocol = TRUE;
		ca_cls_rule.key.ip.ip_protocol = 0x6;	//TCP protocal value 6
	}

	if(acl_filter->filter_fields & INGRESS_L4_UDP_BIT)
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_protocol = TRUE;
		ca_cls_rule.key.ip.ip_protocol = 0x11;	//UDP protocal value 17
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_DSCP_BIT)
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;
		ca_cls_rule.key.ip.ip_version = 6;
		ca_cls_rule.key_mask.ip_mask.dscp = TRUE;
		ca_cls_rule.key.ip.dscp = acl_filter->ingress_ipv6_dscp;
	}

	if(acl_filter->filter_fields & INGRESS_L4_ICMPV6_BIT )
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;
		ca_cls_rule.key.ip.ip_version = 6;
		ca_cls_rule.key_mask.ip_mask.ip_protocol = TRUE;
		ca_cls_rule.key.ip.ip_protocol = 0x1;	//ICMPv6 protocal value 1 in next-header
	}

	if(acl_filter->filter_fields & INGRESS_CTAGIF_BIT)
	{
		//HW ACL not support "reverse" check
		ca_cls_rule.key_mask.l2 = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_count = TRUE;
		ca_cls_rule.key.l2.vlan_count = 1;
	}

	if(acl_filter->filter_fields & INGRESS_L4_POROTCAL_VALUE_BIT)
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_protocol = TRUE;
		ca_cls_rule.key.ip.ip_protocol = acl_filter->ingress_l4_protocal;
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_SIP_BIT )
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_sa = TRUE;
		ca_cls_rule.key.ip.ip_sa.afi = CA_IPV6;
		ca_cls_rule.key.ip.ip_sa.addr_len = 128;
		for(i = 0; i < 4; i++)	//v6SIP: [0]=[31:0], [1]=[63:32], [2]=[95:64], [3]=[127:96]
			ca_cls_rule.key.ip.ip_sa.ip_addr.addr[i] = (acl_filter->ingress_src_ipv6_addr[0+i*4]<<24)|(acl_filter->ingress_src_ipv6_addr[1+i*4]<<16)|(acl_filter->ingress_src_ipv6_addr[2+i*4]<<8)|(acl_filter->ingress_src_ipv6_addr[3+i*4]);

		ret = _rtk_rg_aclHWEntry_maskLength_get_by_ipv6_mask(acl_filter->ingress_src_ipv6_addr_mask,&unmask_length);
		if(ret == RT_ERR_RG_OK)
			ca_cls_rule.key.ip.ip_sa.addr_len -= unmask_length;
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_DIP_BIT )
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_da = TRUE;
		ca_cls_rule.key.ip.ip_da.afi = CA_IPV6;
		ca_cls_rule.key.ip.ip_da.addr_len = 128;
		for(i = 0; i < 4; i++)	//v6DIP: [0]=[31:0], [1]=[63:32], [2]=[95:64], [3]=[127:96]
			ca_cls_rule.key.ip.ip_da.ip_addr.addr[i] = (acl_filter->ingress_dest_ipv6_addr[0+i*4]<<24)|(acl_filter->ingress_dest_ipv6_addr[1+i*4]<<16)|(acl_filter->ingress_dest_ipv6_addr[2+i*4]<<8)|(acl_filter->ingress_dest_ipv6_addr[3+i*4]);

		ret = _rtk_rg_aclHWEntry_maskLength_get_by_ipv6_mask(acl_filter->ingress_dest_ipv6_addr_mask,&unmask_length);
		if(ret == RT_ERR_RG_OK)
			ca_cls_rule.key.ip.ip_da.addr_len -= unmask_length;
	}

	if(acl_filter->filter_fields & INGRESS_IPV4_TAGIF_BIT )
	{
		//HW ACL not support "reverse" check
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;
		ca_cls_rule.key.ip.ip_version = 4;
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_TAGIF_BIT)
	{
		//HW ACL not support "reverse" check
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;
		ca_cls_rule.key.ip.ip_version = 6;
	}

	if(acl_filter->filter_fields & INGRESS_L4_SPORT_RANGE_BIT)
	{
		ca_cls_rule.key_mask.l4 = TRUE;
		ca_cls_rule.key_mask.l4_mask.src_port = TRUE;
		ca_cls_rule.key.l4.src_port.min = acl_filter->ingress_src_l4_port_start;
		ca_cls_rule.key.l4.src_port.max = acl_filter->ingress_src_l4_port_end;
	}

	if(acl_filter->filter_fields & INGRESS_L4_DPORT_RANGE_BIT)
	{
		ca_cls_rule.key_mask.l4 = TRUE;
		ca_cls_rule.key_mask.l4_mask.dst_port = TRUE;
		ca_cls_rule.key.l4.dst_port.min = acl_filter->ingress_dest_l4_port_start;
		ca_cls_rule.key.l4.dst_port.max = acl_filter->ingress_dest_l4_port_end;
	}

	if(acl_filter->filter_fields & INGRESS_IPV4_SIP_RANGE_BIT)
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_sa = TRUE;
		ca_cls_rule.key.ip.ip_sa.afi = CA_IPV4;
		ca_cls_rule.key.ip.ip_sa.addr_len = 32;
		ca_cls_rule.key.ip.ip_sa.ip_addr.addr[0] = acl_filter->ingress_src_ipv4_addr_start;	//acl_filter->ingress_src_ipv4_addr_end

		//HW ACL not support ip_sa_max, only support subnet
		if(acl_filter->ingress_src_ipv4_addr_start != acl_filter->ingress_src_ipv4_addr_end){
			ret = _rtk_rg_aclHWEntry_maskLength_get_by_ipv4_range(acl_filter->ingress_src_ipv4_addr_start,acl_filter->ingress_src_ipv4_addr_end,&unmask_length);
			if(ret == RT_ERR_RG_OK)
				ca_cls_rule.key.ip.ip_sa.addr_len -= unmask_length;
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV4_DIP_RANGE_BIT)
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_da = TRUE;
		ca_cls_rule.key.ip.ip_da.afi = CA_IPV4;
		ca_cls_rule.key.ip.ip_da.addr_len = 32;
		ca_cls_rule.key.ip.ip_da.ip_addr.addr[0] = acl_filter->ingress_dest_ipv4_addr_start;	//acl_filter->ingress_dest_ipv4_addr_end

		//HW ACL not support ip_sa_max, only support subnet
		if(acl_filter->ingress_dest_ipv4_addr_start != acl_filter->ingress_dest_ipv4_addr_end){
			ret = _rtk_rg_aclHWEntry_maskLength_get_by_ipv4_range(acl_filter->ingress_dest_ipv4_addr_start,acl_filter->ingress_dest_ipv4_addr_end,&unmask_length);
			if(ret == RT_ERR_RG_OK)
				ca_cls_rule.key.ip.ip_da.addr_len -= unmask_length;
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_FLOWLABEL_BIT)
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.flow_label = TRUE;
		ca_cls_rule.key.ip.flow_label = acl_filter->ingress_ipv6_flow_label;
	}


	/*set actions*/
	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)//use ACL action
	{
		switch(acl_filter->action_type)
		{
			case ACL_ACTION_TYPE_DROP:
				ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_DENY;
				break;

			case ACL_ACTION_TYPE_PERMIT:
				ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_FE;
				ca_cls_rule.action.dest.fe = CA_CLASSIFIER_FORWARD_FE_L3FE;	//only add L3 CLS, so permit to L3FE
				break;

			case ACL_ACTION_TYPE_TRAP:
				ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
				ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;
				ca_cls_rule.action.options.priority = 0;					//G3 not support HW priority decision, so configure as 0 to prevent l2tp data trap with high priority
				ca_cls_rule.action.options.masks.priority = TRUE;			//RTK_RG_ACLANDCF_RESERVED_L2TP_DATA_LOWER_PRIORITY
				aal_customize |= CA_CLASSIFIER_AAL_L3_KEEP_ORIG;
				break;

			case ACL_ACTION_TYPE_QOS:
				ACL_CTRL("ACL_ACTION_TYPE_QOS is not support by HW_ACL");
				break;

			case ACL_ACTION_TYPE_TRAP_TO_PS:
				ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
				ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;
				aal_customize |= CA_CLASSIFIER_AAL_L3_KEEP_ORIG;
				break;

			case ACL_ACTION_TYPE_POLICY_ROUTE:
				ACL_CTRL("ACL_ACTION_TYPE_POLICY_ROUTE is not support by HW_ACL");
				return(RT_ERR_RG_INVALID_PARAM);
				break;

			case ACL_ACTION_TYPE_TRAP_WITH_PRIORITY:
				ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
				ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;
				ca_cls_rule.action.options.priority = acl_filter->action_trap_with_priority;
				ca_cls_rule.action.options.masks.priority = TRUE;
				aal_customize |= CA_CLASSIFIER_AAL_L3_KEEP_ORIG;
				break;

			default:
			break;
		}
	}
	else
	{
		ACL_CTRL("fwding_type_and_direction not support HW_ACL");
		return(RT_ERR_RG_INVALID_PARAM);
	}


	/*add ACL rule*/
	CA_SCAN_PORTMASK(i, activePorts.bits[0])
	{
		ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
		ca_cls_rule.key_mask.src_port= 1;

		ret = _rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, index, aal_customize);
		if(ret == CA_E_FULL)
			return(RT_ERR_RG_ACL_ENTRY_FULL);
		else if(ret)
		{
			ACL_CTRL("add user ACL failed (ret=0x%x)!!!", ret);
			return(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
		}
		caCLSPort.bits[0] |= (0x1<<i);
		aclCountinueRuleSize++;

		ret = ca_classifier_l3_cls_rule_idx_get(G3_DEF_DEVID, ca_cls_rule.index, &ca_index_hw_info);
		if(ret == CA_E_OK)
			snprintf(ca_index_tmp_buf, sizeof(ca_index_tmp_buf), "%d-%d,", CLS_TBL_ID(ca_index_hw_info), CLS_KEY_ID(ca_index_hw_info));
		else
			snprintf(ca_index_tmp_buf, sizeof(ca_index_tmp_buf), "sw-%d,", ca_cls_rule.index);

		if((strlen(ca_index_buffer)+strlen(ca_index_tmp_buf)+1) < sizeof(ca_index_buffer))
			strncat(ca_index_buffer, ca_index_tmp_buf, strlen(ca_index_tmp_buf));
		else
			ACL_CTRL("Fail to generate ca index string!!(ca_index_buffer=%s, ca_index_tmp_buf=%s)", ca_index_buffer, ca_index_tmp_buf);
	}
	aclSWEntry->hw_aclEntry_count = aclCountinueRuleSize;
	aclSWEntry->hw_aclEntry_port = caCLSPort;
	memcpy(aclSWEntry->hw_aclEntry_index, ca_index_buffer, sizeof(aclSWEntry->hw_aclEntry_index));

	ACL_CTRL("using HW_ACL hw_aclEntry_index is %s (L3 CLS port = 0x%x, total %d rules)",aclSWEntry->hw_aclEntry_index,aclSWEntry->hw_aclEntry_port.bits[0],aclSWEntry->hw_aclEntry_count);

	return (RT_ERR_RG_OK);
}


void _rtk_rg_acl_for_multicast_temp_protection_add(void)
{
	int i;

	if(rg_db.systemGlobal.flow_flush_disable_by_feature&RTK_RG_FFD_BIT_ACL){
		//only add once when flow flush disable for saving time
		for(i = 0; i < MAX_ACL_CA_CLS_RULE_SIZE; i++){
			if((rg_db.systemGlobal.ca_cls_rule_record[i].valid == ENABLED) && (rg_db.systemGlobal.ca_cls_rule_record[i].ruleType == RTK_CA_CLS_TYPE_REARRANGE_PROTECTION)){
				ACL_CTRL("Skip ACL Rearrange Protection(MC permit, Others trap) Start");
				ACL_RSV("Skip ACL Rearrange Protection(MC permit, Others trap) Start");
				return;
			}
		}
	}

	_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_REARRANGE_PROTECTION, NULL);
	ACL_CTRL("ACL Rearrange Protection(MC permit, Others trap) Start");
	ACL_RSV("ACL Rearrange Protection(MC permit, Others trap) Start");
}

void _rtk_rg_acl_for_multicast_temp_protection_del(void)
{
	int i;

	if(rg_db.systemGlobal.flow_flush_disable_by_feature&RTK_RG_FFD_BIT_ACL){
		//not delete when flow flush disable for saving time, delete these rules when recover flow flush
		for(i = 0; i < MAX_ACL_CA_CLS_RULE_SIZE; i++){
			if((rg_db.systemGlobal.ca_cls_rule_record[i].valid == ENABLED) && (rg_db.systemGlobal.ca_cls_rule_record[i].ruleType == RTK_CA_CLS_TYPE_REARRANGE_PROTECTION)){
				ACL_CTRL("Skip ACL Rearrange Protection(MC permit, Others trap) Stop");
				ACL_RSV("Skip ACL Rearrange Protection(MC permit, Others trap) Stop");
				return;
			}
		}
	}

	ACL_CTRL("ACL Rearrange Protection(MC permit, Others trap) Stop");
	ACL_RSV("ACL Rearrange Protection(MC permit, Others trap) Stop");
	_rtk_rg_g3_caClsRuleDeleteReserveType(RTK_CA_CLS_TYPE_REARRANGE_PROTECTION);
}


int _rtk_rg_acl_user_part_rearrange(void)
{
	int ret;

	//using ACL[0] for multicast
	_rtk_rg_acl_for_multicast_temp_protection_add();

	ret = _rtk_rg_aclSWEntry_and_asic_rearrange();
	if(ret!=RT_ERR_RG_OK) WARNING("_rtk_rg_aclSWEntry_and_asic_rearrange failed!");

	_rtk_rg_acl_for_multicast_temp_protection_del();

	return ret;
}


int _rtk_rg_aclSWEntry_and_asic_rearrange(void)
{
	int i,j,accumulateIdx,ruleIdx;
	rtk_rg_aclFilterAndQos_t *acl_filter;



	bzero(&rg_db.systemGlobal.aclSWEntry_rearrange, sizeof(rtk_rg_aclFilterEntry_t));
	bzero(rg_db.systemGlobal.acl_filter_temp, sizeof(rtk_rg_aclFilterAndQos_t)*MAX_ACL_SW_ENTRY_SIZE);
	bzero(rg_db.systemGlobal.acl_filter_temp_valid, sizeof(rg_db.systemGlobal.acl_filter_temp_valid));

	//Clear HW_ACL user used table
	{
		//delete H/W ACL , not include reserved entries (G3 use priority to separate Reserve ACL head/User ACL/Reserve ACL tail)
		assert_ok(_rtk_rg_g3_caClsRuleDelete(RTK_CA_CLS_PRIORITY_L3_USER));
		ACL_CTRL("clear HW_ACL for all priority = RTK_CA_CLS_PRIORITY_L3_USER(%d)",RTK_CA_CLS_PRIORITY_L3_USER);
	}

	//clear the limitation for add ACL drop rule to HW.
	rg_db.systemGlobal.stop_add_hw_acl = 0;

	//clear the information of rearrange status
	rg_db.systemGlobal.stop_add_acl = FALSE;


	ACL_CTRL("================[do ACL rearrange]==================");
	//backup all acl_filter for reAdd, and clean all aclSWEntry
	for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
	{
		if(rg_db.systemGlobal.acl_SW_table_entry[i].valid!=RTK_RG_ENABLED)
			continue;
		//backup acl_filter
		ASSERT_EQ(_rtk_rg_aclSWEntry_get(i, &rg_db.systemGlobal.aclSWEntry_rearrange),RT_ERR_RG_OK);
		rg_db.systemGlobal.acl_filter_temp[i] = rg_db.systemGlobal.aclSWEntry_rearrange.acl_filter;
		rg_db.systemGlobal.acl_filter_temp_valid[(i>>5)] |= (1<<(i&0x1f));
		//clean aclSWEntry  hw information
		bzero(&rg_db.systemGlobal.aclSWEntry_rearrange, ((POINTER_CAST)(&rg_db.systemGlobal.aclSWEntry_rearrange.COUNT_HWACL_LENGTH_FIELD)-(POINTER_CAST)&rg_db.systemGlobal.aclSWEntry_rearrange));
		ASSERT_EQ(_rtk_rg_aclSWEntry_set(i, rg_db.systemGlobal.aclSWEntry_rearrange),RT_ERR_RG_OK);
	}

	//reAdd all reserve aclSWEntry:  type priority as following
		//1. ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET (with acl_weight sorting)
		//2. ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP or ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP
		//3. ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID or ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN
	ASSERT_EQ(_rtk_rg_rearrange_ACL_weight(&accumulateIdx),RT_ERR_RG_OK);


	//1. readd the rules of type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
	for(j=0;j<MAX_ACL_SW_ENTRY_SIZE;j++){
		if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]==-1)
			break;

		acl_filter = &rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j])];
		if(!ACTION_CHECK_HW_NEED(acl_filter))
			continue;
		ASSERT_EQ(_rtk_rg_aclSWEntry_and_asic_reAdd(acl_filter, &rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]),RT_ERR_RG_OK);
	}

	// 2. add rule of ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP or ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP
	_rtk_rg_rearrange_ACL_weight_for_l34_trap_drop_permit(&accumulateIdx);


	// 3 add rule of type ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID or ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN
	_rtk_rg_rearrange_ACL_weight_for_l34_Qos(&accumulateIdx);


	//record acl ingress_vid action valid rules for speed up internalVID decision in fwdEngine. (only type of ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET can use this action)
	//20160811Chuck: However, the dot1p remarking and ingress_cvid action both using CACT in H/W ACL, so need to check both action
	for(i=0,j=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
	{
		if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i]==-1)//no more rules need to check
			break;

		ruleIdx = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i];

		if(ACTION_CHECK_ACT_QOS_IGR_VID(rg_db.systemGlobal.acl_filter_temp[ruleIdx]))
		{
			rg_db.systemGlobal.acl_SWindex_sorting_by_weight_and_ingress_cvid_action[j]=ruleIdx;
			j++;
		}
	}

	if(rg_db.systemGlobal.stop_add_acl) {
		WARNING("add to RG ACL Faild due to ACL rearrange fail!");
		return (RT_ERR_RG_ACL_NOT_SUPPORT);
	}

	return (RT_ERR_RG_OK);
}

int _rtk_rg_aclSWEntry_need_asic_rearrange(rtk_rg_aclFilterAndQos_t *cur_acl_filter, int cur_filter_idx, int stop_hw_acl_filter_idx)
{
	//if the weight of current acl-filter is higher, then it need to try to add to asic
	bzero(&rg_db.systemGlobal.aclSWEntry_rearrange, sizeof(rtk_rg_aclFilterEntry_t));
	ASSERT_EQ(_rtk_rg_aclSWEntry_get(stop_hw_acl_filter_idx, &rg_db.systemGlobal.aclSWEntry_rearrange),RT_ERR_RG_OK);

	if(cur_acl_filter->acl_weight > rg_db.systemGlobal.aclSWEntry_rearrange.acl_filter.acl_weight)
		return TRUE;
	else if((cur_acl_filter->acl_weight == rg_db.systemGlobal.aclSWEntry_rearrange.acl_filter.acl_weight) && (cur_filter_idx < stop_hw_acl_filter_idx))
		return TRUE;

	return FALSE;
}

int _rtk_rg_aclSWEntry_sorting_by_weight(int is_add, int acl_filter_idx)
{
	int i, j, acl_idx;
	int replace_start, replace_rule_index;
	int act_igr_cvid = FALSE;
	rtk_rg_aclFilterEntry_t* pChkRule;

	ACL_CTRL("[%s] only sorting new order by weight for aclSWEntry[%d]", is_add?"ADD":"DELETE", acl_filter_idx);

	bzero(&rg_db.systemGlobal.aclSWEntry_rearrange, sizeof(rtk_rg_aclFilterEntry_t));
	ASSERT_EQ(_rtk_rg_aclSWEntry_get(acl_filter_idx, &rg_db.systemGlobal.aclSWEntry_rearrange),RT_ERR_RG_OK);

	if(is_add && ACTION_CHECK_ACT_QOS_IGR_VID(rg_db.systemGlobal.aclSWEntry_rearrange.acl_filter))
		act_igr_cvid = TRUE;

	if(is_add)
	{
		for(i=0, replace_start=FALSE; i<MAX_ACL_SW_ENTRY_SIZE; i++)
		{
			if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i]==-1)//no more rules need to check
			{
				if(replace_start)	//replace the latest rule
					rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i] = replace_rule_index;
				else
					rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i] = acl_filter_idx;
				//ACL_CTRL("update latest weight[%d] for RG_ACL[%d]", i, rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i]);
				break;
			}

			acl_idx = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i];
			pChkRule = &(rg_db.systemGlobal.acl_SW_table_entry[acl_idx]);
			if(replace_start)
			{
				//ACL_CTRL("push weight[%d] with RG_ACL[%d] for RG_ACL[%d]", i, acl_idx, replace_rule_index);
				rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i] = replace_rule_index;
				replace_rule_index = acl_idx;
			}
			else if((pChkRule->acl_filter.fwding_type_and_direction == rg_db.systemGlobal.aclSWEntry_rearrange.acl_filter.fwding_type_and_direction) &&
					((pChkRule->acl_filter.acl_weight < rg_db.systemGlobal.aclSWEntry_rearrange.acl_filter.acl_weight) ||
					((pChkRule->acl_filter.acl_weight == rg_db.systemGlobal.aclSWEntry_rearrange.acl_filter.acl_weight) && (acl_filter_idx < acl_idx))))
			{	//example: ACL[0]:w(500) > ACL[1]:w(400) > ACL[2]:w(200) > ACL[3]:w(200)
				//ACL_CTRL("insert weight[%d] with RG_ACL[%d] for RG_ACL[%d]", i, acl_idx, acl_filter_idx);
				replace_start = TRUE;
				replace_rule_index = acl_idx;
				rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i] = acl_filter_idx;
			}

		}
	}
	else
	{
		for(i=0, replace_start=FALSE; i<(MAX_ACL_SW_ENTRY_SIZE-1); i++)
		{
			if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i]==-1)//no more rules need to check
				break;

			acl_idx = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i];
			if(replace_start)
			{
				//ACL_CTRL("push weight[%d] with RG_ACL[%d] for RG_ACL[%d]", i, rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i], rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i+1]);
				rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i] = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i+1];
			}
			else if(acl_idx == acl_filter_idx)
			{
				replace_start = TRUE;
				rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i] = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i+1];
				//ACL_CTRL("replace weight[%d] with RG_ACL[%d] for RG_ACL[%d]", i, acl_filter_idx, rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i]);
			}
			if(replace_start && (i==(MAX_ACL_SW_ENTRY_SIZE-2)))	//prevent Overrunning array
				rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i+1] = -1;
		}
	}

	if(act_igr_cvid /*add ingress_cvid_action*/ || !is_add /*delete*/)
	{
		//ACL_CTRL("reflash ingress_cvid_action for RG_ACL[%d]", acl_filter_idx);
		memset(rg_db.systemGlobal.acl_SWindex_sorting_by_weight_and_ingress_cvid_action, -1, sizeof(rg_db.systemGlobal.acl_SWindex_sorting_by_weight_and_ingress_cvid_action));
		for(i=0,j=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
		{
			if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i]==-1)//no more rules need to check
				break;

			acl_idx = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i];
			bzero(&rg_db.systemGlobal.aclSWEntry_rearrange, sizeof(rtk_rg_aclFilterEntry_t));
			ASSERT_EQ(_rtk_rg_aclSWEntry_get(acl_idx, &rg_db.systemGlobal.aclSWEntry_rearrange),RT_ERR_RG_OK);

			if(ACTION_CHECK_ACT_QOS_IGR_VID(rg_db.systemGlobal.aclSWEntry_rearrange.acl_filter))
			{
				rg_db.systemGlobal.acl_SWindex_sorting_by_weight_and_ingress_cvid_action[j]=acl_idx;
				j++;
			}
		}
	}

	return (RT_ERR_RG_OK);
}

//int _rtk_rg_check_acl_committed_rule(rtk_acl_igr_log_act_ctrl_t mode){WARNING("Not implement yet!"); return RT_ERR_RG_OK;}

int32 _rtk_rg_acl_pattern_mask_check(rtk_rg_aclFilterAndQos_t *acl_filter)
{
	//Downward compatibility: if pattern with zero carebit, consider as all care.
	if(acl_filter->filter_fields & INGRESS_SMAC_BIT)
	{
		if((acl_filter->ingress_smac_mask.octet[0] |
				acl_filter->ingress_smac_mask.octet[1] |
				acl_filter->ingress_smac_mask.octet[2] |
				acl_filter->ingress_smac_mask.octet[3] |
				acl_filter->ingress_smac_mask.octet[4] |
				acl_filter->ingress_smac_mask.octet[5]) ==0x0 ){

				acl_filter->ingress_smac_mask.octet[0]=0xff;
				acl_filter->ingress_smac_mask.octet[1]=0xff;
				acl_filter->ingress_smac_mask.octet[2]=0xff;
				acl_filter->ingress_smac_mask.octet[3]=0xff;
				acl_filter->ingress_smac_mask.octet[4]=0xff;
				acl_filter->ingress_smac_mask.octet[5]=0xff;
			}
	}


	if(acl_filter->filter_fields & INGRESS_DMAC_BIT)
	{
		if((acl_filter->ingress_dmac_mask.octet[0]|
			acl_filter->ingress_dmac_mask.octet[1]|
			acl_filter->ingress_dmac_mask.octet[2]|
			acl_filter->ingress_dmac_mask.octet[3]|
			acl_filter->ingress_dmac_mask.octet[4]|
			acl_filter->ingress_dmac_mask.octet[5])==0x0 ){

			acl_filter->ingress_dmac_mask.octet[0]=0xff;
			acl_filter->ingress_dmac_mask.octet[1]=0xff;
			acl_filter->ingress_dmac_mask.octet[2]=0xff;
			acl_filter->ingress_dmac_mask.octet[3]=0xff;
			acl_filter->ingress_dmac_mask.octet[4]=0xff;
			acl_filter->ingress_dmac_mask.octet[5]=0xff;
		}

	}

	if(acl_filter->filter_fields & INGRESS_STREAM_ID_BIT)
	{
		if(acl_filter->ingress_stream_id_mask==0x0){
  			acl_filter->ingress_stream_id_mask = 0xffff;
		}
	}

	if(acl_filter->filter_fields & INGRESS_ETHERTYPE_BIT)
	{
 		if(acl_filter->ingress_ethertype_mask==0x0){
			acl_filter->ingress_ethertype_mask = 0xffff;
		}
	}

	if(acl_filter->filter_fields & INGRESS_EGRESS_PORTIDX_BIT)
	{
		//cf [2:0]uni pattern: ingress_port_idx for US, egress_port_idx for DS
		if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT)
		{
			if(acl_filter->ingress_port_idx_mask==0x0){
				acl_filter->ingress_port_idx_mask=CF_UNI_DEFAULT;
			}
		}
		else if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_PERMIT)
		{
			if(acl_filter->egress_port_idx_mask==0x0){
				acl_filter->egress_port_idx_mask=CF_UNI_DEFAULT;
			}
		}
		else if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
		{	//flow base support egress_port_idx for UP/DS, but ingress_port_idx_mask can use ingress_port_mask
			if(acl_filter->egress_port_idx_mask==0x0){
				acl_filter->egress_port_idx_mask=CF_UNI_DEFAULT;
			}
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_SIP_BIT )
	{
		//for compitible of non-mask version (mask not set means all care)
		if(acl_filter->ingress_src_ipv6_addr_mask[0]==0x0 &&
			acl_filter->ingress_src_ipv6_addr_mask[1]==0x0 &&
			acl_filter->ingress_src_ipv6_addr_mask[2]==0x0 &&
			acl_filter->ingress_src_ipv6_addr_mask[3]==0x0 &&
			acl_filter->ingress_src_ipv6_addr_mask[4]==0x0 &&
			acl_filter->ingress_src_ipv6_addr_mask[5]==0x0 &&
			acl_filter->ingress_src_ipv6_addr_mask[6]==0x0 &&
			acl_filter->ingress_src_ipv6_addr_mask[7]==0x0 &&
			acl_filter->ingress_src_ipv6_addr_mask[8]==0x0 &&
			acl_filter->ingress_src_ipv6_addr_mask[9]==0x0 &&
			acl_filter->ingress_src_ipv6_addr_mask[10]==0x0 &&
			acl_filter->ingress_src_ipv6_addr_mask[11]==0x0 &&
			acl_filter->ingress_src_ipv6_addr_mask[12]==0x0 &&
			acl_filter->ingress_src_ipv6_addr_mask[13]==0x0 &&
			acl_filter->ingress_src_ipv6_addr_mask[14]==0x0 &&
			acl_filter->ingress_src_ipv6_addr_mask[15]==0x0 )
		{
			acl_filter->ingress_src_ipv6_addr_mask[0] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[1] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[2] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[3] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[4] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[5] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[6] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[7] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[8] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[9] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[10] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[11] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[12] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[13] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[14] = 0xff;
			acl_filter->ingress_src_ipv6_addr_mask[15] = 0xff;
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_DIP_BIT )
	{
    	//for compitible of non-mask version (mask not set means all care)
		if(acl_filter->ingress_dest_ipv6_addr_mask[0]==0x0 &&
			acl_filter->ingress_dest_ipv6_addr_mask[1]==0x0 &&
			acl_filter->ingress_dest_ipv6_addr_mask[2]==0x0 &&
			acl_filter->ingress_dest_ipv6_addr_mask[3]==0x0 &&
			acl_filter->ingress_dest_ipv6_addr_mask[4]==0x0 &&
			acl_filter->ingress_dest_ipv6_addr_mask[5]==0x0 &&
			acl_filter->ingress_dest_ipv6_addr_mask[6]==0x0 &&
			acl_filter->ingress_dest_ipv6_addr_mask[7]==0x0 &&
			acl_filter->ingress_dest_ipv6_addr_mask[8]==0x0 &&
			acl_filter->ingress_dest_ipv6_addr_mask[9]==0x0 &&
			acl_filter->ingress_dest_ipv6_addr_mask[10]==0x0 &&
			acl_filter->ingress_dest_ipv6_addr_mask[11]==0x0 &&
			acl_filter->ingress_dest_ipv6_addr_mask[12]==0x0 &&
			acl_filter->ingress_dest_ipv6_addr_mask[13]==0x0 &&
			acl_filter->ingress_dest_ipv6_addr_mask[14]==0x0 &&
			acl_filter->ingress_dest_ipv6_addr_mask[15]==0x0 ){

				acl_filter->ingress_dest_ipv6_addr_mask[0] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[1] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[2] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[3] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[4] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[5] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[6] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[7] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[8] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[9] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[10] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[11] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[12] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[13] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[14] = 0xff;
				acl_filter->ingress_dest_ipv6_addr_mask[15] = 0xff;
		}
	}

	if(acl_filter->filter_fields & EGRESS_CTAG_VID_BIT)
	{
		if(acl_filter->egress_ctag_vid_mask==0x0)
		{
			acl_filter->egress_ctag_vid_mask = 0xfff;
		}
	}

	return (RT_ERR_RG_OK);
}

static int _rtk_rg_wlanDevFromDmacL2Idx_check(uint32 egress_wlanDevMask, rtk_rg_pktHdr_t *pPktHdr)
{
	int8 wlan_dev_idx=-1;

	if(pPktHdr->dmacL2Idx!=FAIL)
	{
		wlan_dev_idx = rg_db.lut[pPktHdr->dmacL2Idx].wlan_device_idx;
		ACL("[egress_wlanDevMask=0x%x] Get wlan_dev_idx=%d from LUT[%d]",egress_wlanDevMask,wlan_dev_idx,pPktHdr->dmacL2Idx);
		if((wlan_dev_idx >= 0) && (egress_wlanDevMask&(1<<wlan_dev_idx))){
			return ACL_PATTERN_HIT;
		}else{
			return ACL_PATTERN_UNHIT;
		}
	}
	ACL("invalid dmacL2Idx %d cause wlan_dev_idx unhit",pPktHdr->dmacL2Idx);

	return ACL_PATTERN_UNHIT;
}


/*(3)literomeDriver mapping APIs*/
int32 _rtk_rg_apollo_aclFilterAndQos_add(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx)
{
	int i,ret;
	int aclSWEntryIdx;
	int update_hw_acl = FALSE;
	bzero(&rg_db.systemGlobal.aclSWEntry,sizeof(rtk_rg_aclFilterEntry_t));


	//Check rg has been init
	if(rg_db.systemGlobal.vlanInit==0)
		RETURN_ERR(RT_ERR_RG_NOT_INIT);

	//check input parameter
	if(acl_filter == NULL || acl_filter_idx == NULL)
		RETURN_ERR(RT_ERR_RG_NULL_POINTER);

	if(rg_db.systemGlobal.acl_rg_add_parameter_dump){
		rtlglue_printf("RG ACL parameter check:\n");
		_dump_rg_acl_entry_content(NULL, acl_filter);
	}

	//ASSERT_EQ(_rtk_rg_aclSWEntry_warning(acl_filter),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_aclSWEntry_not_support(acl_filter),RT_ERR_RG_OK);

	//Downward compatibility: if pattern with zero carebit, consider as all care.
	ASSERT_EQ(_rtk_rg_acl_pattern_mask_check(acl_filter),RT_ERR_RG_OK);



	//using ACL[0] for multicast protextion
	_rtk_rg_acl_for_multicast_temp_protection_add();



	//pre-check HW ACL is enough or not, if the rule need to harwarelze and HW ACL is not enough, just retun fail.
	if(rg_db.systemGlobal.stop_add_hw_acl==0 && ACTION_CHECK_HW_NEED(acl_filter))
	{
		update_hw_acl = TRUE;
		if(acl_filter->action_type==ACL_ACTION_TYPE_TRAP_WITH_PRIORITY)
		{
			//Only work on HW-ACL, so skip byPassAddHwAclCheck
			if((acl_filter->filter_fields & ~(HW_ACL_SUPPORT_PATTERN)) || (acl_filter->filter_fields_inverse!=0x0) || (_rtk_rg_aclHWEntry_not_support(acl_filter)))
			{
				WARNING("This action only work on HW-ACL, but this rule include HW NOT SUPPORT PATTERN or INVERSE so fail to add asic.\n");
				ret = RT_ERR_RG_INVALID_PARAM;
				goto aclFailed;
			}
			if(acl_filter->acl_weight <= 0)
				WARNING("This action only work on HW-ACL, please assign a higher weight.\n");
		}
		else if(acl_filter->filter_fields & ~(HW_ACL_SUPPORT_PATTERN))
		{
			goto byPassAddHwAclCheck;
		}
		else if(acl_filter->filter_fields_inverse!=0x0)
		{
			goto byPassAddHwAclCheck;
		}
		else if((ret = _rtk_rg_aclHWEntry_not_support(acl_filter)) != RT_ERR_RG_OK)
		{
			if(ret == RT_ERR_RG_ACL_NOT_SUPPORT)
				WARNING("HW ACL not support these pattern, and reset ACL will not add to hw. Please check /proc/dump/acl_template for much more detail information.");
			goto byPassAddHwAclCheck;
		}

		//need sub function to add to H/W ACL, and a clear API to call first.
		ret = _rtk_rg_aclSWEntry_to_asic_add(acl_filter,&rg_db.systemGlobal.aclSWEntry,FAIL);
		if(ret == RT_ERR_RG_ACL_ENTRY_FULL)
		{
			bzero(&rg_db.systemGlobal.aclSWEntry, ((POINTER_CAST)(&rg_db.systemGlobal.aclSWEntry.COUNT_HWACL_LENGTH_FIELD)-(POINTER_CAST)&rg_db.systemGlobal.aclSWEntry));
			goto byPassAddHwAclCheck;
		}
		if(ret != RT_ERR_RG_OK)
		{
			goto aclFailed;
		}
	}
	else if(rg_db.systemGlobal.stop_add_hw_acl>0 && (acl_filter->action_type==ACL_ACTION_TYPE_TRAP_WITH_PRIORITY))
	{
		update_hw_acl = TRUE;
		ACL_CTRL("ATTENTION: This rule only work on HW-ACL, but stop_add_hw_acl=1.");
	}
byPassAddHwAclCheck:



	rg_db.systemGlobal.aclSWEntry.acl_filter = *acl_filter;
	rg_db.systemGlobal.aclSWEntry.valid=RTK_RG_ENABLED;

	ret=_rtk_rg_aclSWEntry_empty_find(&aclSWEntryIdx);
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	ret=_rtk_rg_aclSWEntry_set(aclSWEntryIdx,rg_db.systemGlobal.aclSWEntry);
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	*acl_filter_idx = aclSWEntryIdx;
	ACL_CTRL("add aclSWEntry[%d]",*acl_filter_idx);

	//count the total aclSWEntry size
	rg_db.systemGlobal.acl_SW_table_entry_size=0;
	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++)
	{
		if(rg_db.systemGlobal.acl_SW_table_entry[i].valid==RTK_RG_ENABLED)
			rg_db.systemGlobal.acl_SW_table_entry_size++;
	}

	if(!update_hw_acl && rg_db.systemGlobal.stop_add_hw_acl>0 && ACTION_CHECK_HW_NEED(acl_filter) && (_rtk_rg_aclSWEntry_need_asic_rearrange(acl_filter, aclSWEntryIdx, rg_db.systemGlobal.stop_add_hw_acl-1)))
		update_hw_acl = TRUE;

	//readd all acl rules for sorting: action  INGRESS_ALL(sort by weight) > L34 DROP/TRAP/PERMIT > L34 QoS
	if(update_hw_acl)
		ret = _rtk_rg_aclSWEntry_and_asic_rearrange();
	else	//if this rule not have to sync to asic, then only rearrange the sorting
		ret = _rtk_rg_aclSWEntry_sorting_by_weight(TRUE, *acl_filter_idx);
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	if((rg_db.systemGlobal.flow_flush_disable_by_feature&RTK_RG_FFD_BIT_ACL) == 0){
#if 0	// 20180521: enable flow hash of Cpri and DSCP by default
		ret = _rtk_rg_determind_fb_flow_hash_flexible_pattern();
		if(ret!=RT_ERR_RG_OK){
			WARNING("determind FB hash fail!");
		}
#endif
		ret = _rtk_rg_shortCut_clear();
		if(ret!=RT_ERR_RG_OK) goto aclFailed;
	}

	if(rg_db.systemGlobal.acl_rg_add_parameter_dump){
		rtlglue_printf("add to RG ACL[%d] success!\n",*acl_filter_idx);
	}

	_rtk_rg_acl_for_multicast_temp_protection_del();
	return (RT_ERR_RG_OK);

aclFailed:

	if(rg_db.systemGlobal.acl_rg_add_parameter_dump){
		rtlglue_printf("add to RG ACL Faild! ret=0x%x\n",ret);
	}

	_rtk_rg_acl_for_multicast_temp_protection_del();

	if(rg_db.systemGlobal.stop_add_acl) {
		ASSERT_EQ(rtk_rg_apollo_aclFilterAndQos_del(*acl_filter_idx),RT_ERR_RG_OK);
	}

	return ret;

}




int32 _rtk_rg_apollo_aclFilterAndQos_del(int acl_filter_idx)
{
	int i,ret=RT_ERR_RG_FAILED;
	int update_hw_acl = FALSE;
	bzero(&rg_db.systemGlobal.aclSWEntry,sizeof(rtk_rg_aclFilterEntry_t));

	//Check rg has been init
	if(rg_db.systemGlobal.vlanInit==0)
		RETURN_ERR(RT_ERR_RG_NOT_INIT);

	if(acl_filter_idx<0 || acl_filter_idx>=MAX_ACL_SW_ENTRY_SIZE){
		WARNING("invalid ACL index(%d)!",acl_filter_idx);
		RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);
	}

	//using ACL[0] for multicast protextion
	_rtk_rg_acl_for_multicast_temp_protection_add();

	ASSERT_EQ(_rtk_rg_aclSWEntry_get(acl_filter_idx, &rg_db.systemGlobal.aclSWEntry),RT_ERR_RG_OK);
	if((rg_db.systemGlobal.aclSWEntry.hw_aclEntry_count > 0) || (rg_db.systemGlobal.stop_add_hw_acl && ((rg_db.systemGlobal.stop_add_hw_acl-1) == acl_filter_idx)))
		update_hw_acl = TRUE;
	bzero(&rg_db.systemGlobal.aclSWEntry,sizeof(rtk_rg_aclFilterEntry_t));

	//clean aclSWEntry
	ret = _rtk_rg_aclSWEntry_set(acl_filter_idx, rg_db.systemGlobal.aclSWEntry);
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	//rearrange the ACL & CF ASIC to avoid discontinuous entry
	if(update_hw_acl)
		ret = _rtk_rg_aclSWEntry_and_asic_rearrange();
	else	//if this rule not have to sync to asic, then only rearrange the sorting
		ret = _rtk_rg_aclSWEntry_sorting_by_weight(FALSE, acl_filter_idx);
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	//count the total aclSWEntry size
	rg_db.systemGlobal.acl_SW_table_entry_size=0;
	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++)
	{
		if(rg_db.systemGlobal.acl_SW_table_entry[i].valid==RTK_RG_ENABLED)
			rg_db.systemGlobal.acl_SW_table_entry_size++;
	}

	ACL_CTRL("delete aclSWEntry[%d]",acl_filter_idx);

	if(acl_filter_idx < rg_db.systemGlobal.acl_SW_empty_start_index)	//update empty start idx when delete rule with smaller rule index
		rg_db.systemGlobal.acl_SW_empty_start_index = acl_filter_idx;

	if((rg_db.systemGlobal.flow_flush_disable_by_feature&RTK_RG_FFD_BIT_ACL) == 0){
#if 0	// 20180521: enable flow hash of Cpri and DSCP by default
		ret = _rtk_rg_determind_fb_flow_hash_flexible_pattern();
		if(ret!=RT_ERR_RG_OK){
			WARNING("determind FB hash fail!");
		}
#endif
		ret = _rtk_rg_shortCut_clear();
		if(ret!=RT_ERR_RG_OK) goto aclFailed;
	}

	//_rtk_rg_aclSWEntry_dump();


	_rtk_rg_acl_for_multicast_temp_protection_del();
	return (RT_ERR_RG_OK);

aclFailed:

	_rtk_rg_acl_for_multicast_temp_protection_del();
	return ret;

}


int32 _rtk_rg_apollo_aclFilterAndQos_find(rtk_rg_aclFilterAndQos_t *acl_filter, int *valid_idx)
{
	//search for the first not empty entry after valid_idx.
	int i;
	bzero(&rg_db.systemGlobal.aclSWEntry, sizeof(rg_db.systemGlobal.aclSWEntry));
	bzero(&rg_db.systemGlobal.empty_aclSWEntry, sizeof(rg_db.systemGlobal.empty_aclSWEntry));

	//Check rg has been init
	if(rg_db.systemGlobal.vlanInit==0)
		RETURN_ERR(RT_ERR_RG_NOT_INIT);
	if(acl_filter==NULL || valid_idx==NULL)
		RETURN_ERR(RT_ERR_RG_NULL_POINTER);

	if(*valid_idx==-1){
		for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++){
			ASSERT_EQ(_rtk_rg_aclSWEntry_get(i,&rg_db.systemGlobal.aclSWEntry),RT_ERR_RG_OK);
			if(!memcmp(&(rg_db.systemGlobal.aclSWEntry.acl_filter),acl_filter,sizeof(rtk_rg_aclFilterAndQos_t)))//search the same with acl_filter
			{
				*valid_idx=i;
				*acl_filter = rg_db.systemGlobal.aclSWEntry.acl_filter;
				ACL_CTRL("rtk_rg_aclFilterAndQos_find index=%d",*valid_idx);
				return (RT_ERR_RG_OK);
			}

		}
	}else if((*valid_idx>=0) && (*valid_idx<MAX_ACL_SW_ENTRY_SIZE)){
		for(i=*valid_idx; i<MAX_ACL_SW_ENTRY_SIZE; i++)
		{
			ASSERT_EQ(_rtk_rg_aclSWEntry_get(i,&rg_db.systemGlobal.aclSWEntry),RT_ERR_RG_OK);

			if(memcmp(&rg_db.systemGlobal.aclSWEntry,&rg_db.systemGlobal.empty_aclSWEntry,sizeof(rtk_rg_aclFilterEntry_t)))//search the different with empty_aclSWEntry
			{
				*valid_idx=i;
				*acl_filter = rg_db.systemGlobal.aclSWEntry.acl_filter;
				ACL_CTRL("rtk_rg_aclFilterAndQos_find index=%d",*valid_idx);
				return (RT_ERR_RG_OK);
			}
		}
		ACL_CTRL("rtk_rg_aclFilterAndQos_find failed");
	}else{
		RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);
	}
	//not found
	return (RT_ERR_RG_ACL_SW_ENTRY_NOT_FOUND);
}


int _rtk_rg_classifySWEntry_not_support(rtk_rg_classifyEntry_t *classifyFilter)
{

	//FB not support force redirect port. (path5 always lookup to LUT)
	if(classifyFilter->ds_action_field&CF_DS_ACTION_UNI_MASK_BIT)
	{
		if(classifyFilter->action_uni.uniActionDecision==ACL_UNI_FORCE_BY_MASK && classifyFilter->action_uni.assignedUniPortMask!=0x0){
			WARNING("apolloPro not support UNI force redirect.");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}

	if(classifyFilter->action_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2 ||
		classifyFilter->action_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_ORIGINAL_STAG_TPID )
	{
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}



	//SP2C/C2S actions is not belong to tagDecision layer in apolloPro, it should moved to vidDecision layer.
	if(classifyFilter->action_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C)
	{
		WARNING("SP2C action should be set at svlanSvidDecision layer");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
	if(classifyFilter->action_cvlan.cvlanTagIfDecision==ACL_CVLAN_TAGIF_TAGGING_WITH_C2S || classifyFilter->action_cvlan.cvlanTagIfDecision==ACL_CVLAN_TAGIF_TAGGING_WITH_SP2C)
	{
		WARNING("SP2C/C2S action should be set at cvlanCvidDecision layer");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	//******************downward compatibility patch*********************//
	if((classifyFilter->us_action_field & CF_US_ACTION_DROP_BIT) && (classifyFilter->us_action_field & CF_US_ACTION_FWD_BIT))
	{
		WARNING("CF_US_ACTION_DROP_BIT and CF_US_ACTION_FWD_BIT can not set at the same time");
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	}

	if((classifyFilter->ds_action_field & CF_DS_ACTION_DROP_BIT) && (classifyFilter->ds_action_field & CF_DS_ACTION_UNI_MASK_BIT))
	{
		WARNING("CF_DS_ACTION_DROP_BIT and CF_DS_ACTION_UNI_MASK_BIT can not set at the same time");
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	}

	if(classifyFilter->us_action_field & CF_US_ACTION_DROP_BIT)
	{
		//translate to CF_US_ACTION_FWD_BIT
		classifyFilter->us_action_field &= ~(CF_US_ACTION_DROP_BIT);
		classifyFilter->us_action_field |= CF_US_ACTION_FWD_BIT;
		classifyFilter->action_fwd.fwdDecision = ACL_FWD_DROP;
	}

	if(classifyFilter->ds_action_field & CF_DS_ACTION_DROP_BIT)
	{
		//translate to CF_DS_ACTION_UNI_MASK_BIT
		classifyFilter->ds_action_field &= ~(CF_DS_ACTION_DROP_BIT);
		classifyFilter->ds_action_field |= CF_DS_ACTION_UNI_MASK_BIT;
		classifyFilter->action_uni.uniActionDecision = ACL_UNI_FORCE_BY_MASK;
		classifyFilter->action_uni.assignedUniPortMask=0x0;
	}
	//******************downward compatibility patch end*********************//

	return RT_ERR_RG_OK;
}


int32 _rtk_rg_apollo_classifyEntry_add(rtk_rg_classifyEntry_t *classifyFilter)
{
	//int ret;

	//Check rg has been init
	if(rg_db.systemGlobal.vlanInit==0)
		RETURN_ERR(RT_ERR_RG_NOT_INIT);

	//check index range
	if(classifyFilter->index < 0 || classifyFilter->index > TOTAL_CF_ENTRY_SIZE){
		rtlglue_printf("Invalid classify rule index, range should be in %d~%d\n",0,TOTAL_CF_ENTRY_SIZE);
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	}

	//check flow & action
	if(classifyFilter->direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM){
		//ds should not have action
		if(classifyFilter->ds_action_field) RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	}else if(classifyFilter->direction==RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM){
		//us should not have action
		if(classifyFilter->us_action_field) RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	}else{
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	}

	ASSERT_EQ(_rtk_rg_classifySWEntry_not_support(classifyFilter),RT_ERR_RG_OK);


	if(classifyFilter->filter_fields & EGRESS_GEMIDX_BIT){
		if(classifyFilter->gemidx_mask==0x0){
			//for compitible of non-mask version (mask not set means all care)
			classifyFilter->gemidx_mask=0x7f;
		}else{
			//NA
		}
	}

	if(classifyFilter->filter_fields & EGRESS_UNI_BIT){
		if(classifyFilter->uni_mask==0x0){
			//for compitible of non-mask version (mask not set means all care)
			classifyFilter->uni_mask = CF_UNI_DEFAULT;
		}else{
			//NA
		}

	}


	assert_ok(_rtk_rg_classifySWEntry_set(classifyFilter->index, *classifyFilter));

#if 0	// 20180521: enable flow hash of Cpri and DSCP by default
	ret = _rtk_rg_determind_fb_flow_hash_flexible_pattern();
	if(ret!=RT_ERR_RG_OK){
		WARNING("determind FB hash fail!");
	}
#endif

	return (RT_ERR_RG_OK);

}


int32 _rtk_rg_apollo_classifyEntry_del(int index)
{
	//int ret;
	rtk_rg_classifyEntry_t empty_classifyFilter;

	//Check rg has been init
	if(rg_db.systemGlobal.vlanInit==0)
		RETURN_ERR(RT_ERR_RG_NOT_INIT);

	bzero(&empty_classifyFilter,sizeof(rtk_rg_classifyEntry_t));
	empty_classifyFilter.index=FAIL; //means invlaid

	assert_ok(_rtk_rg_classifySWEntry_set(index, empty_classifyFilter));

#if 0	// 20180521: enable flow hash of Cpri and DSCP by default
	ret = _rtk_rg_determind_fb_flow_hash_flexible_pattern();
	if(ret!=RT_ERR_RG_OK){
		WARNING("determind FB hash fail!");
	}
#endif

	return (RT_ERR_RG_OK);
}


int32 _rtk_rg_apollo_classifyEntry_find(int index, rtk_rg_classifyEntry_t *classifyFilter)
{
	//Check rg has been init
    if(rg_db.systemGlobal.vlanInit==0)
        RETURN_ERR(RT_ERR_RG_NOT_INIT);

	assert_ok(_rtk_rg_classifySWEntry_get(index, classifyFilter));
	return (RT_ERR_RG_OK);
}


/*(4)reserved ACL related APIs*/
int _rtk_rg_aclReservedEntry_init(void)
{
	rg_db.systemGlobal.aclAndCfReservedRule.aclLowerBoundary = 0;
	rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary = (MAX_ACL_ENTRY_SIZE-1);

	return (RT_ERR_RG_OK);
}

static int _rtk_rg_aclAndCfReservedRuleAddCheck(uint32 aclRsvSize, uint32 cfRsvSize, rtk_rg_aclAndCf_reserved_type_t rsvType)
{
#if 0		//[Care Asic ACL Full]
	//check rest empty acl rules in enough
	if(rg_db.systemGlobal.ca_cls_used_count+aclRsvSize >= MAX_ACL_CA_CLS_RULE_SIZE)
	{
		//flow base platform, when asic full, user acl can be S/W only, and rest user acl stop add to asic
		ACL_RSV("ACL rest rules for reserved[%d] is not enough! Some user acl will become S/W only. (used_count %d + aclRsvSize %d >= MAX_ACL_CA_CLS_RULE_SIZE %d)", rsvType, rg_db.systemGlobal.ca_cls_used_count, aclRsvSize, MAX_ACL_CA_CLS_RULE_SIZE);
		//WARNING("ACL rest rules for reserved[%d] is not enough! Some user acl will become S/W only. (used_count %d + aclRsvSize %d >= MAX_ACL_CA_CLS_RULE_SIZE %d)", rsvType, rg_db.systemGlobal.ca_cls_used_count, aclRsvSize, MAX_ACL_CA_CLS_RULE_SIZE);
		//return (RT_ERR_RG_FAILED);
	}
#endif
	return (RT_ERR_RG_OK);

}

int _rtk_rg_aclAndCfReservedRuleReflash(void)
{
#if defined(CONFIG_RG_FLOW_BASED_PLATFORM)
	{
		extern volatile int rg_api_aclReserveRearrangeAccelerate;
		extern atomic_t rg_api_entrance;

		if(atomic_read(&rg_api_entrance)>=1)
		{
			if(rg_api_aclReserveRearrangeAccelerate>=2)
			{
				//continue to rearrange reserve acl
			}
			else
			{
				rg_api_aclReserveRearrangeAccelerate=1;
				ACL_RSV("ingore rearrange reserve acl by API control path ");
				return RT_ERR_RG_OK;
			}
		}
	}
#endif

	//using ACL[0] for multicast
	_rtk_rg_acl_for_multicast_temp_protection_add();

	//[Care Asic ACL Full] force delete user acl for reserve acl
	assert_ok(_rtk_rg_g3_caClsRuleDelete(RTK_CA_CLS_PRIORITY_L3_USER));

	if((rg_db.systemGlobal.wanPortMask.portmask != 0x0)
		&& (rg_db.systemGlobal.aclAndCfReservedRule.acl_wanPortMask != rg_db.systemGlobal.wanPortMask.portmask))
		rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_CA_CLS_TYPE_L2_INGRESS_FORWARD_L3FE]=ENABLED;

	_rtk_rg_aclAndCfReservedRuleHeadReflash();
	_rtk_rg_aclAndCfReservedRuleTailReflash();

	//[Care Asic ACL Full] user acl can be S/W only, so reflash after reserve acl update finish
	assert_ok(_rtk_rg_aclSWEntry_and_asic_rearrange());

	_rtk_rg_acl_for_multicast_temp_protection_del();

	return RT_ERR_RG_OK;
}

static int _rtk_rg_aclAndCfReservedRuleHeadReflash(void)
{
	uint32 type;
	ca_classifier_rule_t ca_cls_rule;
	rtk_rg_g3_special_pkt_rule_t ca_specPkt_rule;
	rtk_portmask_t activePorts;
	int addRuleFailedFlag=0;
	uint32 i, j;
	unsigned int aal_customize = CA_CLASSIFIER_AAL_NA;

	//reflash th rules
	assert_ok(_rtk_rg_g3_caClsRuleDelete(RTK_CA_CLS_PRIORITY_L2_HEAD));
	assert_ok(_rtk_rg_g3_caClsRuleDelete(RTK_CA_CLS_PRIORITY_L3_HEAD));

	for(type=0;type<RTK_RG_ACLANDCF_RESERVED_HEAD_END;type++){
		if(rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[type]==ENABLED){
			switch(type){

				case RTK_CA_CLS_TYPE_L2_INGRESS_FORWARD_L3FE:
					ACL_RSV("update RSV[%d] L2_INGRESS_FORWARD_L3FE (current wanPortMask=0x%x, wanPortMask=0x%x)", type, rg_db.systemGlobal.aclAndCfReservedRule.acl_wanPortMask, rg_db.systemGlobal.wanPortMask.portmask);

					_rtk_rg_g3_caClsRuleDeleteReserveType(type);
					bzero(&ca_cls_rule,sizeof(ca_cls_rule));
					bzero(&activePorts,sizeof(rtk_portmask_t));

					//only need to care physical port and wifi port 0x12~0x16
					activePorts.bits[0] = RTK_RG_ALL_MAC_PORTMASK&(~(1<<RTK_RG_MAC_PORT_CPU))&(~(1<<RTK_RG_MAC_PORT_CPU_WLAN1_AND_OTHERS));
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
					activePorts.bits[0] = activePorts.bits[0] & (~(1<<G3_LOOPBACK_P_NEWSPA));
					activePorts.bits[0] = activePorts.bits[0] & (~(G3_LOOPBACK_P_MASK)); //packet from LAN port=>go to loopback port directly.
#endif
					CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
						ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L2_0;
						_rtk_rg_g3_caClsAssignPortType(&ca_cls_rule, i);

						ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
						if((0x1<<i) & rg_db.systemGlobal.wanPortMask.portmask)
							ca_cls_rule.action.dest.port = AAL_LPORT_L3_WAN;
						else
							ca_cls_rule.action.dest.port = AAL_LPORT_L3_LAN;

						if(_rtk_rg_g3_caClsRuleAdd(&ca_cls_rule, type)) {
							ACL_RSV("update special ACL RTK_CA_CLS_TYPE_L2_INGRESS_FORWARD_L3FE failed!!!");
							addRuleFailedFlag = 1;
							break;
						}
					}
					rg_db.systemGlobal.aclAndCfReservedRule.acl_wanPortMask = rg_db.systemGlobal.wanPortMask.portmask;
					rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[type]=DISABLED;
					break;

				case RTK_RG_ACLANDCF_RESERVED_STPBLOCKING:
					{
						ACL_RSV("add RSV[%d] STPBLOCKING", type);

						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						//L3 CLS not support MAC range, so create L2 igr Classifier for trap DA = 01:80:c2:xx:xx:xx
						activePorts.bits[0] = RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L2_HEAD;
							_rtk_rg_g3_caClsAssignPortType(&ca_cls_rule, i);

							ca_cls_rule.key_mask.l2 = TRUE;
							ca_cls_rule.key.l2.mac_da.mac_min[0] = ca_cls_rule.key.l2.mac_da.mac_max[0] = 0x01;
							ca_cls_rule.key.l2.mac_da.mac_min[1] = ca_cls_rule.key.l2.mac_da.mac_max[1] = 0x80;
							ca_cls_rule.key.l2.mac_da.mac_min[2] = ca_cls_rule.key.l2.mac_da.mac_max[2] = 0xc2;
							for(j=3; j<6; j++) {
								ca_cls_rule.key.l2.mac_da.mac_min[j] = 0;
								ca_cls_rule.key.l2.mac_da.mac_max[j] = 0xff;
							}
							ca_cls_rule.key_mask.l2_mask.mac_da = TRUE;

							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
							ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;

							if(_rtk_rg_g3_caClsRuleAdd(&ca_cls_rule, type)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_STPBLOCKING for trap failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}

						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						activePorts.bits[0] = rg_db.systemGlobal.stpBlockingPortmask.portmask;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L2_HEAD;
							_rtk_rg_g3_caClsAssignPortType(&ca_cls_rule, i);

							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_DENY;

							if(_rtk_rg_g3_caClsRuleAdd(&ca_cls_rule, type)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_STPBLOCKING for drop failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_ALL_TRAP:
					{
						ACL_RSV("add RSV[%d] ALL_TRAP (wanPortMask=0x%x)", type, rg_db.systemGlobal.wanPortMask.portmask);

						bzero(&ca_cls_rule,sizeof(ca_cls_rule));

						//Create L2 igr Classifier for forward the packet to L3FE
						CA_SCAN_ALL_PORT(i) {
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L2_HEAD;
							_rtk_rg_g3_caClsAssignPortType(&ca_cls_rule, i);

							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
							if(rg_db.systemGlobal.wanPortMask.portmask != 0x0) {
								if((0x1<<i) & rg_db.systemGlobal.wanPortMask.portmask)
									ca_cls_rule.action.dest.port = AAL_LPORT_L3_WAN;
								else
									ca_cls_rule.action.dest.port = AAL_LPORT_L3_LAN;
							}
							else {
								if(i == RTK_RG_PORT_PON)
									ca_cls_rule.action.dest.port = AAL_LPORT_L3_WAN;
								else
									ca_cls_rule.action.dest.port = AAL_LPORT_L3_LAN;
							}

							if(_rtk_rg_g3_caClsRuleAdd(&ca_cls_rule, type)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ALL_TRAP failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}

						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						//Create L3 Classifier for trap the packet to CPU
						activePorts.bits[0] = CA_L3_CLS_PROFILE_LAN;
						activePorts.bits[0] |= CA_L3_CLS_PROFILE_WAN;
						aal_customize = CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_HEAD;
							ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
							ca_cls_rule.key_mask.src_port= 1;

							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
							ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;

							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, aal_customize|CA_CLASSIFIER_AAL_L3_KEEP_ORIG)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ALL_TRAP failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}

						if(addRuleFailedFlag)
						{
							break;
						}
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_UNICAST_TRAP:
					{
						ACL_RSV("add RSV[%d] UNICAST_TRAP", type);

						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						activePorts.bits[0] = CA_L3_CLS_PROFILE_LAN;
						activePorts.bits[0] |= CA_L3_CLS_PROFILE_WAN;
						aal_customize = CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_HEAD;
							ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
							ca_cls_rule.key_mask.src_port= TRUE;

							ca_cls_rule.key_mask.l2 = TRUE;
							ca_cls_rule.key.l2.is_multicast = FALSE;	//is_multicast 1: MAC DA is multicast address(DA[40] = 1b1) and the packet is not broadcast DA
							ca_cls_rule.key_mask.l2_mask.is_multicast = TRUE;

							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
							ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;

							//not MC(DA[40] = 1b1) and not BC(DA = 48hffff_ffff_ffff)
							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, aal_customize|CA_CLASSIFIER_AAL_L3_NOT_BROADCAST)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UNICAST_TRAP failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP:
					{
						ACL_RSV("add RSV[%d] MULTICAST_SSDP_TRAP by special packet [SSDP(0x%x)]", type, AAL_L3_SPECPKT_TYPE_SSDP);

						bzero(&ca_specPkt_rule,sizeof(ca_specPkt_rule));

						//AAL_L3_SPECPKT_TYPE_SSDP: MC(MAC_DA[47:24] == 24'h01005E, or MAC_DA[47:32] == 16'h3333)+UDP(UDP lite) dport 1900/2869/5000
						ca_specPkt_rule.source_port = CA_PORT_ID(CA_PORT_TYPE_L2RP, AAL_LPORT_L3_LAN);
						ca_specPkt_rule.special_packet = CA_PKT_TYPE_SSDP;
						if(_rtk_rg_g3_caSpecialPacketTrap(&ca_specPkt_rule, type))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_DOT1X_EAPOL_TRAP:
					{
#if defined(CONFIG_RG_8021X_MAC_TABLE_SIZE) && (CONFIG_RG_8021X_MAC_TABLE_SIZE!=0)
						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						if(rg_db.systemGlobal.accessDot1xCfg.blockingPortmask.portmask == 0x0)
						{
							ACL_RSV("skip add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_DOT1X_EAPOL_TRAP due to blocking port is 0!!!");
							break;
						}

						ACL_RSV("add RSV[%d] DOT1X_EAPOL_TRAP [blockingPortMask=0x%x]", type, rg_db.systemGlobal.accessDot1xCfg.blockingPortmask);

						activePorts.bits[0] = rg_db.systemGlobal.accessDot1xCfg.blockingPortmask.portmask;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_HEAD;
							ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
							ca_cls_rule.key_mask.src_port= TRUE;
				
							ca_cls_rule.key_mask.l2 = TRUE;
							ca_cls_rule.key.l2.ethertype = RG_DOT1X_EAPOL_ETHERTYPE;
							ca_cls_rule.key_mask.l2_mask.ethertype = TRUE;

							ca_cls_rule.action.options.priority = 7;
							ca_cls_rule.action.options.masks.priority = TRUE;

							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
							ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;
				
							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, CA_CLASSIFIER_AAL_NA)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_DOT1X_EAPOL_TRAP failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}
#endif
					}
					break;

#if 0		//mark these function due to its still under development.
				case RTK_RG_ACLANDCF_RESERVED_INTF0_IPV4_FRAGMENT_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF0_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[0].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF1_IPV4_FRAGMENT_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF1_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[1].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF2_IPV4_FRAGMENT_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF2_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[2].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF3_IPV4_FRAGMENT_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF3_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[3].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF4_IPV4_FRAGMENT_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF4_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[4].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF5_IPV4_FRAGMENT_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF5_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[5].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF6_IPV4_FRAGMENT_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF6_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[6].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF7_IPV4_FRAGMENT_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF7_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[7].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF8_IPV4_FRAGMENT_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF8_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[8].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF9_IPV4_FRAGMENT_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF9_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[9].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF10_IPV4_FRAGMENT_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF10_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[10].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF11_IPV4_FRAGMENT_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF11_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[11].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF12_IPV4_FRAGMENT_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF12_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[12].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF13_IPV4_FRAGMENT_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF13_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[13].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF14_IPV4_FRAGMENT_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF14_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[14].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF15_IPV4_FRAGMENT_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF15_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[15].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;

fragmentTrapRule:
					{
						/*start to set ACL rule*/
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField_dmac0,sizeof(aclField_dmac0));
						bzero(&aclField_dmac1,sizeof(aclField_dmac1));
						bzero(&aclField_dmac2,sizeof(aclField_dmac2));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK;
						aclRule.templateIdx=0; // dmac

						//setup gmac
						aclField_dmac2.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField_dmac2.fieldUnion.pattern.fieldIdx = 0; //DA[15:0]: template[0] field[0]
						aclField_dmac2.fieldUnion.data.value = (mac.octet[4]<<8) | (mac.octet[5]);
						aclField_dmac2.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac2)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFn_IPV4_FRAGMENT_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclField_dmac1.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField_dmac1.fieldUnion.pattern.fieldIdx = 1; //DA[31:16]: template[0] field[1]
						aclField_dmac1.fieldUnion.data.value = (mac.octet[2]<<8) | (mac.octet[3]);
						aclField_dmac1.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac1)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFn_IPV4_FRAGMENT_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclField_dmac0.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField_dmac0.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
						aclField_dmac0.fieldUnion.data.value = (mac.octet[0]<<8) | (mac.octet[1]);
						aclField_dmac0.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac0)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFn_IPV4_FRAGMENT_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFn_IPV4_FRAGMENT_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;




						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK;
						aclRule.templateIdx=2; // tagif

						/*ipv4 tagif*/
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 5;/*template[2],field[5]*/
						aclField.fieldUnion.data.value = ACL_TAGIF_OUT_IP4_BIT; //ipv4 tagif
						aclField.fieldUnion.data.mask = ACL_TAGIF_OUT_IP4_BIT;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFn_IPV4_FRAGMENT_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFn_IPV4_FRAGMENT_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;



						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK;
						aclRule.templateIdx=3; //FS[1]: ipv4 fragment

						/*ip.MF*/
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 1;/*template[3],field[1]*/
						aclField.fieldUnion.data.value = 0x2000; //FS[1]: ipv4 fragment
						aclField.fieldUnion.data.mask = 0x2000;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFn_IPV4_FRAGMENT_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFn_IPV4_FRAGMENT_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;


					}
					break;

#endif	//mark these function due to its still under development.

				case RTK_RG_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY:
					{
						ACL_RSV("add RSV[%d] L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY", type);

						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						activePorts.bits[0] = CA_L3_CLS_PROFILE_WAN;
						aal_customize = CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT;
						aal_customize |= CA_CLASSIFIER_AAL_L3_RSLT_TYPE_0;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							/*Set ACL rule for higher rx prioirty for L2TP controal packet and LCP packet(Wan port + UDP + Sport(1701) + Dport(1701) + pktLen < 128)
								Due to CA L3 CLS not support these pattern in one rule, so seperate as below two rules:
									Rule 1: CLS Type0, key=udp + special header(type=2, key=sport/dport 1701), action=trap to CPU+assign higher priority
									Rule 2: CLS Type1, key=udp + packet length range 128~9216, action=assign priority 0
								Target: trap all l2tp packet, and l2tp data packet use lower priority. others data packet recover their priority by flow*/
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_HEAD;
							ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
							ca_cls_rule.key_mask.src_port= TRUE;

							ca_cls_rule.key_mask.ip = TRUE;
							ca_cls_rule.key.ip.ip_protocol = 0x11;
							ca_cls_rule.key_mask.ip_mask.ip_protocol = TRUE;

							/* l2tp packet not support sport/dport, so we use spacial header to do this
							ca_cls_rule.key_mask.l4 = TRUE;
							ca_cls_rule.key.l4.src_port.min = 1701;
							ca_cls_rule.key.l4.src_port.max = ca_cls_rule.key.l4.src_port.min;
							ca_cls_rule.key_mask.l4_mask.src_port = TRUE;
							ca_cls_rule.key.l4.dst_port.min = 1701;
							ca_cls_rule.key.l4.dst_port.max = ca_cls_rule.key.l4.dst_port.min;
							ca_cls_rule.key_mask.l4_mask.dst_port = TRUE;*/

							//force trap to CPU with priroity to avoid FB trap priroity replaced.
							ca_cls_rule.action.options.priority = rg_db.systemGlobal.aclAndCfReservedRule.l2tp_ctrl_lcp_assign_prioity.priority;
							ca_cls_rule.action.options.masks.priority = TRUE;

							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
							ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;

							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, aal_customize|CA_CLASSIFIER_AAL_L3_SPEC_PKT_L4_L2TP_PORT)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY:
					{
						ACL_RSV("add RSV[%d] PPPoE_LCP_PACKET_ASSIGN_PRIORITY", type);

						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						activePorts.bits[0] |= CA_L3_CLS_PROFILE_WAN;
						aal_customize = CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							//Set ACL rule for higher rx prioirty for PPPoE LCP packet(ppp protocol=0xc021, none-ipv4, none-ipv6)
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_HEAD;
							ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
							ca_cls_rule.key_mask.src_port= TRUE;

							ca_cls_rule.key_mask.l2 = TRUE;
							ca_cls_rule.key.l2.ethertype = 0x8864;
							ca_cls_rule.key_mask.l2_mask.ethertype = TRUE;

							ca_cls_rule.action.options.priority = rg_db.systemGlobal.aclAndCfReservedRule.pppoe_lcp_assign_prioity.priority;
							ca_cls_rule.action.options.masks.priority = TRUE;

							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
							ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;

							//PPP Protocol - LCP = 0xC021
							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, aal_customize|CA_CLASSIFIER_AAL_L3_PPP_LCP)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY for PPP LCP failed!!!");
								addRuleFailedFlag=1;
								break;
							}
							//PPP Protocol - IPCP = 0x8021
							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, aal_customize|CA_CLASSIFIER_AAL_L3_PPP_IPCP)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY for PPP IPCP failed!!!");
								addRuleFailedFlag=1;
								break;
							}
							//PPP Protocol - IP6CP = 0x8057
							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, aal_customize|CA_CLASSIFIER_AAL_L3_PPP_IP6CP)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY for PPP IP6CP failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY:
					{
						ACL_RSV("add RSV[%d] ACK_PACKET_ASSIGN_PRIORITY", type);
						//If enable this ACL, flow will disable priority bit to prevent acl priority replace by flow. However, GPON mode for stream id remapping still have problem.

						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						/* !!Attentation!! HW Known issue List as below:
							1. tcp_flags only support 'OR' instead of 'AND'. for example: configure tcp_flags as ACK only, CLS will hit as long as packet include 'ack'(ex: syn+ack).
							2. Only support one packet length, if we add two overlap packet length range, it will cause both unhit.
						*/

						activePorts.bits[0] = CA_L3_CLS_PROFILE_LAN;
						activePorts.bits[0] |= CA_L3_CLS_PROFILE_WAN;
						aal_customize = CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_HEAD;
							ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
							ca_cls_rule.key_mask.src_port= TRUE;

							ca_cls_rule.key_mask.l4 = TRUE;
							ca_cls_rule.key.l4.tcp_flags = 0x10;
							ca_cls_rule.key_mask.l4_mask.tcp_flags = 0x10;	//bit 4=ACK

							ca_cls_rule.key_mask.ip = TRUE;
							ca_cls_rule.key_mask.ip_mask.ip_protocol = TRUE;
							ca_cls_rule.key.ip.ip_protocol = 0x6;	//TCP protocal value 6

							ca_cls_rule.action.options.priority = rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_assign_priority.priority;
							ca_cls_rule.action.options.masks.priority = TRUE;

							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_FE;
							ca_cls_rule.action.dest.fe = CA_CLASSIFIER_FORWARD_FE_L3FE;

							//limit packet size is samller than 72
							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, aal_customize|CA_CLASSIFIER_AAL_L3_PKT_LEN_RANGE_IDX0)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_TRAP_OR_ASSIGN_PRIORITY:
					{
						ACL_RSV("add RSV[%d] ACK_PACKET_TRAP_OR_ASSIGN_PRIORITY", type);

						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						/* !!Attentation!! HW Known issue List as below:
							1. tcp_flags only support 'OR' instead of 'AND'. for example: configure tcp_flags as ACK only, CLS will hit as long as packet include 'ack'(ex: syn+ack).
							2. Only support one packet length, if we add two overlap packet length range, it will cause both unhit.
						*/

						if((rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_trap_or_assign_priority.pktLenStart != 0) ||
							(rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_trap_or_assign_priority.pktLenEnd != 72)) {
							WARNING("TCP ack only support packet length range 0~72 in G3.");
							ACL_RSV("TCP ack only support packet length range 0~72 in G3.");
							addRuleFailedFlag=1;
							break;
						}
						if(rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_trap_or_assign_priority.tcp_flag_data != rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_trap_or_assign_priority.tcp_flag_mask) {
							WARNING("TCP ack not support ack only in G3.");
							ACL_RSV("TCP ack not support ack only in G3.");
							addRuleFailedFlag=1;
							break;
						}

						activePorts.bits[0] = rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_trap_or_assign_priority.portmask;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_HEAD;
							ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
							ca_cls_rule.key_mask.src_port= TRUE;

							ca_cls_rule.key_mask.l4 = TRUE;
							ca_cls_rule.key.l4.tcp_flags = rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_trap_or_assign_priority.tcp_flag_data;
							ca_cls_rule.key_mask.l4_mask.tcp_flags = rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_trap_or_assign_priority.tcp_flag_mask;	//bit 4=ACK

							ca_cls_rule.key_mask.ip = TRUE;
							ca_cls_rule.key_mask.ip_mask.ip_protocol = TRUE;
							ca_cls_rule.key.ip.ip_protocol = 0x6;	//TCP protocal value 6

							switch(rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_trap_or_assign_priority.action) {
								case 0:
									ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
									ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;
									break;
								case 1:
									ca_cls_rule.action.options.priority = rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_trap_or_assign_priority.priority;
									ca_cls_rule.action.options.masks.priority = TRUE;
									ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_FE;
									ca_cls_rule.action.dest.fe = CA_CLASSIFIER_FORWARD_FE_L3FE;
									break;
								case 2:
									ca_cls_rule.action.options.priority = rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_trap_or_assign_priority.priority;
									ca_cls_rule.action.options.masks.priority = TRUE;
									ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
									ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;
									break;
								default:
									WARNING("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_TRAP_OR_ASSIGN_PRIORITY failed(only support action 0~2)!!!");
									break;
							}
							//limit packet size is samller than 72
							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, CA_CLASSIFIER_AAL_L3_PKT_LEN_RANGE_IDX0)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_TRAP_OR_ASSIGN_PRIORITY failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP_OR_ASSIGN_PRIORITY:
					{
						ACL_RSV("add RSV[%d] SYN_PACKET_TRAP_OR_ASSIGN_PRIORITY", type);

						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						/* !!Attentation!! HW Known issue List as below:
							1. tcp_flags only support 'OR' instead of 'AND'. for example: configure tcp_flags as ACK only, CLS will hit as long as packet include 'ack'(ex: syn+ack).
							2. Only support one packet length, if we add two overlap packet length range, it will cause both unhit.
						*/

						if((rg_db.systemGlobal.trapSpecificLengthSyn.pktLenStart != 0) ||
							(rg_db.systemGlobal.trapSpecificLengthSyn.pktLenEnd != 72)) {
							WARNING("TCP syn only support packet length range 0~72 in G3.");
							ACL_RSV("TCP syn only support packet length range 0~72 in G3.");
							addRuleFailedFlag=1;
							break;
						}
						if(rg_db.systemGlobal.trapSpecificLengthSyn.synOnly) {
							WARNING("TCP syn not support syn only in G3.");
							ACL_RSV("TCP syn not support syn only in G3.");
							addRuleFailedFlag=1;
							break;
						}

						activePorts.bits[0] = rg_db.systemGlobal.trapSpecificLengthSyn.portMask;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_HEAD;
							ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
							ca_cls_rule.key_mask.src_port= TRUE;

							ca_cls_rule.key_mask.l4 = TRUE;
							if(rg_db.systemGlobal.trapSpecificLengthSyn.synOnly){
								ca_cls_rule.key.l4.tcp_flags = 0x2;	//bit 1=ACK
								ca_cls_rule.key_mask.l4_mask.tcp_flags = 0x1ff;	//G3 only support 9 bits
							}else{
								ca_cls_rule.key.l4.tcp_flags = 0x2;	//bit 1=ACK
								ca_cls_rule.key_mask.l4_mask.tcp_flags = 0x2;
							}

							ca_cls_rule.key_mask.ip = TRUE;
							ca_cls_rule.key_mask.ip_mask.ip_protocol = TRUE;
							ca_cls_rule.key.ip.ip_protocol = 0x6;	//TCP protocal value 6

							if(rg_db.systemGlobal.trapSpecificLengthSyn.isTrap){
								ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
								ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;
							}else{
								ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_FE;
								ca_cls_rule.action.dest.fe = CA_CLASSIFIER_FORWARD_FE_L3FE;
							}
							if(rg_db.systemGlobal.trapSpecificLengthSyn.priority){
								ca_cls_rule.action.options.priority = rg_db.systemGlobal.trapSpecificLengthSyn.priority;
								ca_cls_rule.action.options.masks.priority = TRUE;
							}

							//limit packet size is samller than 72
							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, CA_CLASSIFIER_AAL_L3_PKT_LEN_RANGE_IDX0)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP_OR_ASSIGN_PRIORITY failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}
					}
					break;

#if 0		//mark these function due to its still under development.
				case RTK_RG_ACLANDCF_RESERVED_IGMP_MLD_DROP:
					{
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IGMP_MLD_DROP @ acl[%d]",aclIdx);

						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=rg_db.systemGlobal.aclAndCfReservedRule.igmp_mld_drop_portmask.portmask;
						aclRule.templateIdx=2; // tagif

						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 5;/*template[2],field[5]*/
						aclField.fieldUnion.data.value = (1<<11); //IGMP/MLD tagif
						aclField.fieldUnion.data.mask = (1<<11);
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_IGMP_MLD_DROP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act = ACL_IGR_FORWARD_DROP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_IGMP_MLD_DROP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;
					}
					break;
#endif	//mark these function due to its still under development.

				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_L2FE_FLOODING:
					{
						ACL_RSV("add RSV[%d] MULTICAST_L2FE_FLOODING(Step2:L2 CLS pop fake VLAN)", type);

						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						activePorts.bits[0] = RTK_RG_ALL_MAC_PORTMASK&(~(1<<RTK_RG_MAC_PORT_CPU))&(~(1<<RTK_RG_MAC_PORT_CPU_WLAN1_AND_OTHERS));
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L2_HEAD;
							_rtk_rg_g3_caClsAssignPortType(&ca_cls_rule, i);

							ca_cls_rule.key.l2.vlan_count = 1;
							ca_cls_rule.key.l2.vlan_otag.vlan_min.vid = 0xFF0;	//should hake CFG_ID_FAKE_WAN_MC_VID to always use the first one
							ca_cls_rule.key.l2.vlan_otag.vlan_max = ca_cls_rule.key.l2.vlan_otag.vlan_min;
							ca_cls_rule.key_mask.l2 = 1;
							ca_cls_rule.key_mask.l2_mask.vlan_count = 1;
							ca_cls_rule.key_mask.l2_mask.vlan_otag = 1;
							ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.vid = 1;

							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_FE;
							ca_cls_rule.action.dest.fe = CA_CLASSIFIER_FORWARD_FE_L2FE;

							ca_cls_rule.action.options.masks.outer_vlan_act = TRUE;
							ca_cls_rule.action.options.outer_vlan_act = CA_CLASSIFIER_VLAN_ACTION_POP;

							if(_rtk_rg_g3_caClsRuleAdd(&ca_cls_rule, type)) {
								ACL_RSV("add special ACL RTK_RG_ACLANDCF_RESERVED_MULTICAST_L2FE_FLOODING failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
						}
					}
					break;

				default:
					break;

			}

		}
	}

	if(addRuleFailedFlag==1)
		return (RT_ERR_RG_FAILED);

	return (RT_ERR_RG_OK);
}

static int _rtk_rg_aclAndCfReservedRuleTailReflash(void)
{
	uint32 type;
	ca_classifier_rule_t ca_cls_rule;
	rtk_rg_g3_special_pkt_rule_t ca_specPkt_rule;
	rtk_portmask_t activePorts;
	int addRuleFailedFlag=0;
	uint32 i, j;
	unsigned int aal_customize = CA_CLASSIFIER_AAL_NA;

	//reflash th rules
	assert_ok(_rtk_rg_g3_caClsRuleDelete(RTK_CA_CLS_PRIORITY_L2_TAIL));
	assert_ok(_rtk_rg_g3_caClsRuleDelete(RTK_CA_CLS_PRIORITY_L3_TAIL));

	for(type=RTK_RG_ACLANDCF_RESERVED_HEAD_END;type<RTK_RG_ACLANDCF_RESERVED_TAIL_END;type++){
		if(rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[type]==ENABLED){
			switch(type){

				case RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_ASSIGN_SHARE_METER:
					{
						ACL_RSV("add RSV[%d] SYN_PACKET_ASSIGN_SHARE_METER", type);

						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						/*!!Attentation!! HW Known issue: tcp_flags only support 'OR' instead of 'AND'. for example: configure tcp_flags as ACK only, CLS will hit as long as packet include 'ack'(ex: syn+ack).*/

						activePorts.bits[0] = CA_L3_CLS_PROFILE_WAN;
						aal_customize = CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
							ca_cls_rule.key_mask.src_port= TRUE;

							//TCP flag: SYN=1, xACK=0(G3 not support)x
							ca_cls_rule.key_mask.l4 = TRUE;
							ca_cls_rule.key.l4.tcp_flags = 0x2;
							ca_cls_rule.key_mask.l4_mask.tcp_flags = 0x2;	//bit 2=SYN

							//check TCP tagIf
							ca_cls_rule.key_mask.ip = TRUE;
							ca_cls_rule.key_mask.ip_mask.ip_protocol = TRUE;
							ca_cls_rule.key.ip.ip_protocol = 0x6;	//TCP protocal value 6

							//assign share meter
							ca_cls_rule.action.options.action_handle.flow_id = rg_db.systemGlobal.aclAndCfReservedRule.syn_packet_assign_share_meter.share_meter + G3_FLOW_POLICER_IDXSHIFT_PROC;
							ca_cls_rule.action.options.handle_type = CA_KEY_HANDLE_TYPE_FLOW_ID;
							ca_cls_rule.action.options.masks.action_handle = TRUE;

							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_FE;
							ca_cls_rule.action.dest.fe = CA_CLASSIFIER_FORWARD_FE_L3FE;

							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, aal_customize)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_ASSIGN_SHARE_METER failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_ARP_PACKET_ASSIGN_SHARE_METER:
					{
						ACL_RSV("add RSV[%d] ARP_PACKET_ASSIGN_SHARE_METER", type);

						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						activePorts.bits[0] = rg_db.systemGlobal.aclAndCfReservedRule.arp_packet_assign_share_meter.portmask;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
							ca_cls_rule.key_mask.src_port= TRUE;

							ca_cls_rule.key_mask.l2 = TRUE;
							ca_cls_rule.key.l2.ethertype = 0x0806;
							ca_cls_rule.key_mask.l2_mask.ethertype = TRUE;
							//broadcast mac to prevent impact arp response
							for(j=0; j<6; j++)	//broadcast mac
								ca_cls_rule.key.l2.mac_da.mac_min[j] = ca_cls_rule.key.l2.mac_da.mac_max[j] = 0xff;
							ca_cls_rule.key_mask.l2_mask.mac_da = TRUE;

							//assign share meter
							ca_cls_rule.action.options.action_handle.flow_id = rg_db.systemGlobal.aclAndCfReservedRule.arp_packet_assign_share_meter.share_meter + G3_FLOW_POLICER_IDXSHIFT_PROC;
							ca_cls_rule.action.options.handle_type = CA_KEY_HANDLE_TYPE_FLOW_ID;
							ca_cls_rule.action.options.masks.action_handle = TRUE;

							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_FE;
							ca_cls_rule.action.dest.fe = CA_CLASSIFIER_FORWARD_FE_L3FE;

							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, CA_CLASSIFIER_AAL_NA)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ARP_PACKET_ASSIGN_SHARE_METER failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER:
					{
						ACL_RSV("add RSV[%d] UDP_DOS_PACKET_ASSIGN_SHARE_METER(not support DMAC)", type);

						RTK_RG_DOS_RATE_LIMIT_VALID_SCAN(j){
							bzero(&ca_cls_rule,sizeof(ca_cls_rule));
							bzero(&activePorts,sizeof(rtk_portmask_t));

							/*!!Attentation!! HW Known issue: Only support one packet length, if we add two overlap packet length range, it will cause both unhit.*/

							if((rg_db.systemGlobal.dosRateLimit[j].pktLenStart != 124) ||
								(rg_db.systemGlobal.dosRateLimit[j].pktLenEnd != 128)) {
								WARNING("dos_rate_limit only support packet length range 124~128 in G3.");
								ACL_RSV("dos_rate_limit only support packet length range 124~128 in G3.");
							}else if(rg_db.systemGlobal.dosRateLimit[j].ctagif == 0) {
								WARNING("dos_rate_limit only support CtagIf=1 in G3.");
								ACL_RSV("dos_rate_limit only support CtagIf=1 in G3.");
							}

							activePorts.bits[0] = rg_db.systemGlobal.dosRateLimit[j].portmask;
							CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
								ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_TAIL;
								ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
								ca_cls_rule.key_mask.src_port= TRUE;

								ca_cls_rule.key_mask.l2 = TRUE;
								/* G3 not support DIP + DMAC at the same time
								ca_cls_rule.key.l2.mac_da v.s. rg_db.systemGlobal.dosRateLimit[j].ingressDmac
								ca_cls_rule.key_mask.l2_mask.mac_da = TRUE;*/
								//ctagif and ctag_vid
								if(rg_db.systemGlobal.dosRateLimit[j].ctagif) {
									ca_cls_rule.key_mask.l2_mask.vlan_otag = TRUE;
									ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.vid = TRUE;
									ca_cls_rule.key.l2.vlan_otag.vlan_min.vid = rg_db.systemGlobal.dosRateLimit[j].ctagVid;
									ca_cls_rule.key.l2.vlan_otag.vlan_max = ca_cls_rule.key.l2.vlan_otag.vlan_min;
								}

								//check TCP/UDP tagIf
								ca_cls_rule.key_mask.ip = TRUE;
								ca_cls_rule.key_mask.ip_mask.ip_protocol = TRUE;
								if(rg_db.systemGlobal.dosRateLimit[j].isTcp)
									ca_cls_rule.key.ip.ip_protocol = RG_IP_PROTO_TCP;	//TCP protocal value 6
								else
									ca_cls_rule.key.ip.ip_protocol = RG_IP_PROTO_UDP;	//UDP protocal value 17
								ca_cls_rule.key_mask.ip_mask.ip_da = 1;
								ca_cls_rule.key.ip.ip_da.afi = CA_IPV4;
								ca_cls_rule.key.ip.ip_da.ip_addr.ipv4_addr = rg_db.systemGlobal.dosRateLimit[j].ingressDip;
								ca_cls_rule.key.ip.ip_da.addr_len = 32;

								//assign share meter
								ca_cls_rule.action.options.action_handle.flow_id = rg_db.systemGlobal.dosRateLimit[j].shareMeterIdx_hw + G3_FLOW_POLICER_IDXSHIFT_PROC;
								ca_cls_rule.action.options.handle_type = CA_KEY_HANDLE_TYPE_FLOW_ID;
								ca_cls_rule.action.options.masks.action_handle = TRUE;

								ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_FE;
								ca_cls_rule.action.dest.fe = CA_CLASSIFIER_FORWARD_FE_L3FE;

								//limit packet size range 124~128
								if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, CA_CLASSIFIER_AAL_L3_PKT_LEN_RANGE_IDX2)) {
									ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER failed!!!");
									addRuleFailedFlag=1;
									break;
								}
							}
						}
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT:
					{
						ACL_RSV("add RSV[%d] MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT for FF0x/DHCPv6", type);

						//Due to G3 not support filter FFxE, so only trap control packet
						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						activePorts.bits[0] = CA_L3_CLS_PROFILE_LAN;
						activePorts.bits[0] |= CA_L3_CLS_PROFILE_WAN;
						aal_customize = CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
							ca_cls_rule.key_mask.src_port= TRUE;

							ca_cls_rule.key_mask.ip = TRUE;
							ca_cls_rule.key.ip.ip_version = 6;
							ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;

							//Trap multicast addresses in IPv6 use the prefix ff0X/12.
							ca_cls_rule.key_mask.ip_mask.ip_da = TRUE;
							bzero(&ca_cls_rule.key.ip.ip_da,sizeof(ca_cls_rule.key.ip.ip_da));
							ca_cls_rule.key.ip.ip_da.afi = CA_IPV6;
							ca_cls_rule.key.ip.ip_da.ip_addr.ipv6_addr[0] = 0xff000000;
							for(j = 1; j < 4; j++)
								ca_cls_rule.key.ip.ip_da.ip_addr.ipv6_addr[j] = 0x00000000;
							ca_cls_rule.key.ip.ip_da.addr_len = 12;	//ff0x:xxxx:xxx...

							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
							ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;

							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, aal_customize)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT for FF0x failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}

						//Trap v6 dhcp prevent by G3 unknown mc drop
						//Due to we need support trap dhcpv6 with higher priority by acl-filter, so use cls rather than special packet
						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						activePorts.bits[0] = CA_L3_CLS_PROFILE_LAN;
						activePorts.bits[0] |= CA_L3_CLS_PROFILE_WAN;
						aal_customize = CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
							ca_cls_rule.key_mask.src_port= TRUE;

							ca_cls_rule.key_mask.l2 = TRUE;
							ca_cls_rule.key.l2.is_multicast = TRUE;
							ca_cls_rule.key_mask.l2_mask.is_multicast = TRUE;

							ca_cls_rule.key_mask.ip = TRUE;
							ca_cls_rule.key.ip.ip_version = 6;
							ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;
							ca_cls_rule.key.ip.ip_protocol = 0x11;	//UDP protocal value 17
							ca_cls_rule.key_mask.ip_mask.ip_protocol = TRUE;

							ca_cls_rule.key_mask.l4 = TRUE;
							ca_cls_rule.key_mask.l4_mask.dst_port = TRUE;
							ca_cls_rule.key.l4.dst_port.min = 546;
							ca_cls_rule.key.l4.dst_port.max = 547;

							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
							ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;

							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, aal_customize)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT for DHCPv6 failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}

					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP:
					{
						if(rg_db.systemGlobal.icmp_trap_by_acl_disable == RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_ENABLE)
							ACL_RSV("add RSV[%d] CONTROL_PACKET_TRAP for PPPoE Control Packet/IGMP/ICMP/ICMPv6", type);
						else
							ACL_RSV("add RSV[%d] CONTROL_PACKET_TRAP for PPPoE Control Packet/IGMP/ICMPv6", type);

						//PPPoE Control Packet(Step1:permit ppp proto 0021/0057)
						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						activePorts.bits[0] |= CA_L3_CLS_PROFILE_LAN;
						activePorts.bits[0] |= CA_L3_CLS_PROFILE_WAN;
						aal_customize = CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
							ca_cls_rule.key_mask.src_port= TRUE;

							ca_cls_rule.key_mask.l2 = TRUE;
							ca_cls_rule.key.l2.ethertype = 0x8864;
							ca_cls_rule.key_mask.l2_mask.ethertype = TRUE;

							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_FE;
							ca_cls_rule.action.dest.fe = CA_CLASSIFIER_FORWARD_FE_L3FE;

							//PPP Protocol - IPv4 = 0x0021
							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, aal_customize|CA_CLASSIFIER_AAL_L3_PPP_IPV4)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP for PPPoE Control Packet(Step 1) failed!!!");
								addRuleFailedFlag=1;
								break;
							}
							//PPP Protocol - IPv6 = 0x0057
							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, aal_customize|CA_CLASSIFIER_AAL_L3_PPP_IPV6)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP for PPPoE Control Packet(Step 1) failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}

						//PPPoE Control Packet(Step2:trap ethertype=0x8864)
						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						activePorts.bits[0] |= CA_L3_CLS_PROFILE_LAN;
						activePorts.bits[0] |= CA_L3_CLS_PROFILE_WAN;
						aal_customize = CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							/*22969@aal_l3_cls fix this issue: if_id_key may in the lower HW entry than full_key, so make sure this if_id_key rule have lower priority than previous full_key rule*/
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
							ca_cls_rule.key_mask.src_port= TRUE;

							ca_cls_rule.key_mask.l2 = TRUE;
							ca_cls_rule.key.l2.ethertype = 0x8864;
							ca_cls_rule.key_mask.l2_mask.ethertype = TRUE;

							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
							ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;

							//Keep original due to L3FE Packet Editor cannot support the modification of PPPoE control packet
							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, aal_customize|CA_CLASSIFIER_AAL_L3_KEEP_ORIG)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP for PPPoE Control Packet(Step 2) failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}

						ACL_RSV("add RSV[%d] CONTROL_PACKET_TRAP by special packet [PPPOE_DIS(0x%x)/RIP(0x%x)]", type, AAL_L3_SPECPKT_TYPE_PPPOE_DISC, AAL_L3_SPECPKT_TYPE_RIP);

						bzero(&ca_specPkt_rule,sizeof(ca_specPkt_rule));
						//AAL_L3_SPECPKT_TYPE_PPPOE_DISC: Ethertype == 16'h8863
						ca_specPkt_rule.source_port = CA_PORT_ID(CA_PORT_TYPE_L2RP, AAL_LPORT_L3_WAN);
						ca_specPkt_rule.special_packet = CA_PKT_TYPE_PPPOE_DIS;
						if(_rtk_rg_g3_caSpecialPacketTrap(&ca_specPkt_rule, type))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						ca_specPkt_rule.source_port = CA_PORT_ID(CA_PORT_TYPE_L2RP, AAL_LPORT_L3_LAN);
						ca_specPkt_rule.special_packet = CA_PKT_TYPE_PPPOE_DIS;
						if(_rtk_rg_g3_caSpecialPacketTrap(&ca_specPkt_rule, type))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						bzero(&ca_specPkt_rule,sizeof(ca_specPkt_rule));
						//AAL_L3_SPECPKT_TYPE_RIP: UDP(UDP lite) dport 520
						ca_specPkt_rule.source_port = CA_PORT_ID(CA_PORT_TYPE_L2RP, AAL_LPORT_L3_WAN);
						ca_specPkt_rule.special_packet = CA_PKT_TYPE_RIP;
						if(_rtk_rg_g3_caSpecialPacketTrap(&ca_specPkt_rule, type))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						ca_specPkt_rule.source_port = CA_PORT_ID(CA_PORT_TYPE_L2RP, AAL_LPORT_L3_LAN);
						ca_specPkt_rule.special_packet = CA_PKT_TYPE_RIP;
						if(_rtk_rg_g3_caSpecialPacketTrap(&ca_specPkt_rule, type))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//IGMP
						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						activePorts.bits[0] |= CA_L3_CLS_PROFILE_LAN;
						activePorts.bits[0] |= CA_L3_CLS_PROFILE_WAN;
						aal_customize = CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
							ca_cls_rule.key_mask.src_port= TRUE;

							ca_cls_rule.key_mask.ip = TRUE;
							ca_cls_rule.key.ip.ip_protocol = RG_IP_PROTO_IGMP;
							ca_cls_rule.key_mask.ip_mask.ip_protocol = TRUE;

							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
							ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;

							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, aal_customize)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP for IGMP failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}

						if((rg_db.systemGlobal.icmp_trap_by_acl_disable == RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_ENABLE) ||
							(rg_db.systemGlobal.icmp_trap_by_acl_disable == RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_IPV6_DISABLE)){

							if(rg_db.systemGlobal.icmp_trap_by_acl_disable == RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_IPV6_DISABLE)
								ACL_RSV("add RSV[%d] CONTROL_PACKET_TRAP for ICMP", type);
							bzero(&ca_cls_rule,sizeof(ca_cls_rule));
							bzero(&activePorts,sizeof(rtk_portmask_t));

							activePorts.bits[0] |= CA_L3_CLS_PROFILE_LAN;
							activePorts.bits[0] |= CA_L3_CLS_PROFILE_WAN;
							aal_customize = CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT;
							CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
								ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_TAIL;
								ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
								ca_cls_rule.key_mask.src_port= TRUE;

								ca_cls_rule.key_mask.ip = TRUE;
								ca_cls_rule.key.ip.ip_protocol = RG_IP_PROTO_ICMP;
								ca_cls_rule.key_mask.ip_mask.ip_protocol = TRUE;

								ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
								ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;

								if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, aal_customize)) {
									ACL("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP for ICMP failed!!!");
									addRuleFailedFlag=1;
									break;
								}
							}
						}

						//G3 should trap ipv6 mld to prevent hit unknown multicast main hash default drop.
						{
							bzero(&ca_cls_rule,sizeof(ca_cls_rule));
							bzero(&activePorts,sizeof(rtk_portmask_t));

							activePorts.bits[0] |= CA_L3_CLS_PROFILE_LAN;
							activePorts.bits[0] |= CA_L3_CLS_PROFILE_WAN;
							aal_customize = CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT;
							CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
								ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_TAIL;
								ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
								ca_cls_rule.key_mask.src_port= TRUE;

								ca_cls_rule.key_mask.ip = TRUE;
								ca_cls_rule.key.ip.ip_protocol = RG_IP_PROTO_ICMPv6;
								ca_cls_rule.key_mask.ip_mask.ip_protocol = TRUE;

								ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
								ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;

								if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, aal_customize)) {
									ACL("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP for ICMPv6 failed!!!");
									addRuleFailedFlag=1;
									break;
								}
							}
						}

					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_L2TP_DATA_LOWER_PRIORITY:
					{
						ACL_RSV("add RSV[%d] L2TP_DATA_LOWER_PRIORITY", type);
						//this rule should higher than PPPoE Control Packet(Step1), otherwise l2tp data with pppoe tag will not hit this rule.

						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						activePorts.bits[0] = CA_L3_CLS_PROFILE_WAN;
						aal_customize = CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							/*Set ACL rule for higher rx prioirty for L2TP controal packet and LCP packet(Wan port + UDP + Sport(1701) + Dport(1701) + pktLen < 128)
								Due to CA L3 CLS not support these pattern in one rule, so seperate as below two rules:
									Rule 1: CLS Type0, key=udp + special header(type=2, key=sport/dport 1701), action=trap to CPU+assign higher priority
									Rule 2: CLS Type1, key=udp + packet length range 129~9216, action=assign priority 0
								Target: trap all l2tp packet, and l2tp data packet use lower priority. others data packet recover their priority by flow*/
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
							ca_cls_rule.key_mask.src_port= TRUE;

							ca_cls_rule.key_mask.ip = TRUE;
							ca_cls_rule.key.ip.ip_protocol = 0x11;
							ca_cls_rule.key_mask.ip_mask.ip_protocol = TRUE;

							ca_cls_rule.action.options.priority = 0;
							ca_cls_rule.action.options.masks.priority = TRUE;

							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_FE;
							ca_cls_rule.action.dest.fe = CA_CLASSIFIER_FORWARD_FE_L3FE;

							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, aal_customize|CA_CLASSIFIER_AAL_L3_PKT_LEN_RANGE_IDX1)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_L2TP_DATA_LOWER_PRIORITY failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_DSLITE_TRAP:
					{
						ACL_RSV("add RSV[%d] MULTICAST_DSLITE_TRAP", type);
						//HW not support Dslite, prevent dslit drop by unknown drop, and not impact bridge, so only trap v6 multicast dslite.

						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						activePorts.bits[0] = CA_L3_CLS_PROFILE_LAN;
						activePorts.bits[0] |= CA_L3_CLS_PROFILE_WAN;
						aal_customize = CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
							ca_cls_rule.key_mask.src_port= TRUE;

							ca_cls_rule.key_mask.ip = TRUE;
							ca_cls_rule.key.ip.ip_version = 6;
							ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;
							ca_cls_rule.key.ip.ip_protocol = 0x4;	//ipv6 next header
							ca_cls_rule.key_mask.ip_mask.ip_protocol = TRUE;

							//Trap multicast addresses in IPv6 use the prefix ffX/8.
							ca_cls_rule.key_mask.ip_mask.ip_da = TRUE;
							bzero(&ca_cls_rule.key.ip.ip_da,sizeof(ca_cls_rule.key.ip.ip_da));
							ca_cls_rule.key.ip.ip_da.afi = CA_IPV6;
							ca_cls_rule.key.ip.ip_da.ip_addr.ipv6_addr[0] = 0xff000000;
							for(j = 1; j < 4; j++)
								ca_cls_rule.key.ip.ip_da.ip_addr.ipv6_addr[j] = 0x00000000;
							ca_cls_rule.key.ip.ip_da.addr_len = 8; //ffxx:xxxx:xxx...

							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
							ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;

							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, aal_customize)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_DSLITE_TRAP failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_IPV6_FORWARD_L2FE:
					{	// not used
						ACL_RSV("add RSV[%d] MULTICAST_IPV6_FORWARD_L2FE(Step1:IPv6 MC back L2FE by L3 CLS)", type);

						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						activePorts.bits[0] = CA_L3_CLS_PROFILE_WAN;
						aal_customize = CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT;
						aal_customize |= CA_CLASSIFIER_AAL_L3_RSLT_TYPE_0;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
							ca_cls_rule.key_mask.src_port= TRUE;

							ca_cls_rule.key_mask.l2 = TRUE;
							ca_cls_rule.key.l2.is_multicast = TRUE;
							ca_cls_rule.key_mask.l2_mask.is_multicast = TRUE;

							ca_cls_rule.key_mask.ip = TRUE;
							ca_cls_rule.key.ip.ip_version = 6;
							ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;

							/* action: disable t2 ctrl and add fake vlan by CA_CLASSIFIER_AAL_L3_MCAST_FORWARD */
							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_FE;
							ca_cls_rule.action.dest.fe = CA_CLASSIFIER_FORWARD_FE_L3FE;

							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, aal_customize|CA_CLASSIFIER_AAL_L3_MCAST_FORWARD)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_IPV6_FORWARD_L2FE failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_IPV4_FORWARD_L2FE:
					{	// not used
						ACL_RSV("add RSV[%d] IPV4_FORWARD_L2FE(Step1:IPv4 MC back L2FE by L3 CLS)", type);

						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						activePorts.bits[0] = CA_L3_CLS_PROFILE_WAN;
						aal_customize = CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT;
						aal_customize |= CA_CLASSIFIER_AAL_L3_RSLT_TYPE_0;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
							ca_cls_rule.key_mask.src_port= TRUE;

							ca_cls_rule.key_mask.l2 = TRUE;
							ca_cls_rule.key.l2.is_multicast = TRUE;
							ca_cls_rule.key_mask.l2_mask.is_multicast = TRUE;

							ca_cls_rule.key_mask.ip = TRUE;
							ca_cls_rule.key.ip.ip_version = 4;
							ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;

							/* action: disable t2 ctrl and add fake vlan by CA_CLASSIFIER_AAL_L3_MCAST_FORWARD */
							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_FE;
							ca_cls_rule.action.dest.fe = CA_CLASSIFIER_FORWARD_FE_L3FE;

							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, aal_customize|CA_CLASSIFIER_AAL_L3_MCAST_FORWARD)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_IPV4_FORWARD_L2FE failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_PROTOCOL_TRAP:
					{
						ACL_RSV("add RSV[%d] MULTICAST_PROTOCOL_TRAP for %s", type, (rg_db.systemGlobal.multicastProtocol == RG_MC_IGMP_ONLY)?"ipv6":"ipv4");

						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						//trap ipv4 non pppoe multicast when MLD_ONLY
						activePorts.bits[0] = CA_L3_CLS_PROFILE_LAN | CA_L3_CLS_PROFILE_WAN;
						aal_customize = CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
							ca_cls_rule.key_mask.src_port= TRUE;

							ca_cls_rule.key_mask.ip = TRUE;
							if(rg_db.systemGlobal.multicastProtocol == RG_MC_IGMP_ONLY)
								ca_cls_rule.key.ip.ip_version = 6;	//RG_MC_IGMP_ONLY, trap ipv6
							else
								ca_cls_rule.key.ip.ip_version = 4;	//RG_MC_MLD_ONLY, trap ipv4
							ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;
							ca_cls_rule.key.ip.is_multicast = TRUE;	//IP DA is in MC IP range, ipv4_da[31:28]=4b1110, ipv6_da[127:120]=8hff
							ca_cls_rule.key_mask.ip_mask.is_multicast = TRUE;

							//aal_rule.key.pppoe_session_id = 0;
							//aal_rule.key.msk_pppoe_session = 0;

							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
							ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;

							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, aal_customize|CA_CLASSIFIER_AAL_L3_PPPOE_SESID_0)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_PROTOCOL_TRAP failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}

						bzero(&ca_cls_rule,sizeof(ca_cls_rule));
						bzero(&activePorts,sizeof(rtk_portmask_t));

						//trap ipv4 pppoe multicast with gw mac when MLD_ONLY
						activePorts.bits[0] = CA_L3_CLS_PROFILE_LAN | CA_L3_CLS_PROFILE_WAN;
						aal_customize = CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
							ca_cls_rule.key_mask.src_port= TRUE;

							ca_cls_rule.key_mask.l2 = TRUE;
							ca_cls_rule.key.l2.ethertype = 0x8864;
							ca_cls_rule.key_mask.l2_mask.ethertype = TRUE;

							ca_cls_rule.key_mask.ip = TRUE;
							if(rg_db.systemGlobal.multicastProtocol == RG_MC_IGMP_ONLY)
								ca_cls_rule.key.ip.ip_version = 6;	//RG_MC_IGMP_ONLY, trap ipv6
							else
								ca_cls_rule.key.ip.ip_version = 4;	//RG_MC_MLD_ONLY, trap ipv4
							ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;
							ca_cls_rule.key.ip.is_multicast = TRUE;	//IP DA is in MC IP range, ipv4_da[31:28]=4b1110, ipv6_da[127:120]=8hff
							ca_cls_rule.key_mask.ip_mask.is_multicast = TRUE;

							//aal_rule.key.mac_da_an_hit = 1;
							//aal_rule.key.msk_mac_da_an_hit = 0;

							ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
							ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;

							if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, type, aal_customize|CA_CLASSIFIER_AAL_L3_MAC_DA_AN_HIT)) {
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_PROTOCOL_TRAP failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}
					}
					break;

				default:
					break;
			}
		}
	}

	if(addRuleFailedFlag==1)
		return (RT_ERR_RG_FAILED);

	return (RT_ERR_RG_OK);

}

int _rtk_rg_aclAndCfReservedRuleAdd(rtk_rg_aclAndCf_reserved_type_t rsvType, void *parameter)
{
	ACL_RSV("#####Reserved ACL reflash!(add reserved ACL rsvType=%d)#####",rsvType);

	if(rsvType == RTK_RG_ACLANDCF_RESERVED_TAIL_END)
		goto forceReflash;

	if(rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]==ENABLED)
		return (RT_ERR_RG_OK);

	switch(rsvType){

		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP:	//reserved one more for MC temporary permit while ACL rearrange
		case RTK_RG_ACLANDCF_RESERVED_DOT1X_EAPOL_TRAP:
		//HEAD_END
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_UNICAST_TRAP:
		//HEAD_END
		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_DSLITE_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0,rsvType));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		//HEAD_END
		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_PROTOCOL_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(4,0,rsvType));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_STPBLOCKING:
		//HEAD_END
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(6,0,rsvType));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		//HEAD_END
		case RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(8,0,rsvType));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_ALL_TRAP:
		//HEAD_END
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(14,0,rsvType));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		//HEAD RSV

		case RTK_RG_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(3,0,rsvType));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_lcp_assign_prioity,(rtk_rg_aclAndCf_reserved_pppoe_lcp_assign_priority_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoe_lcp_assign_priority_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.l2tp_ctrl_lcp_assign_prioity,(rtk_rg_aclAndCf_reserved_l2tp_control_lcp_trap_and_assign_priority_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_l2tp_control_lcp_trap_and_assign_priority_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
		//HEAD_END
			/* fall through */
		case RTK_RG_ACLANDCF_RESERVED_L2TP_DATA_LOWER_PRIORITY:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,RTK_RG_ACLANDCF_RESERVED_L2TP_DATA_LOWER_PRIORITY));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_L2TP_DATA_LOWER_PRIORITY]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0,rsvType));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_assign_priority,(rtk_rg_aclAndCf_reserved_ack_packet_assign_priority_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_ack_packet_assign_priority_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_TRAP_OR_ASSIGN_PRIORITY:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0,rsvType));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_trap_or_assign_priority,(rtk_rg_aclAndCf_reserved_ack_packet_trap_or_assign_priority_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_ack_packet_trap_or_assign_priority_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP_OR_ASSIGN_PRIORITY:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0,rsvType));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		//TAIL RSV

		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_IPV4_FORWARD_L2FE:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
		/* case RTK_RG_ACLANDCF_RESERVED_MULTICAST_L2FE_FLOODING: */
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,RTK_RG_ACLANDCF_RESERVED_MULTICAST_L2FE_FLOODING));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_MULTICAST_L2FE_FLOODING]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_IPV6_FORWARD_L2FE:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
		/* case RTK_RG_ACLANDCF_RESERVED_MULTICAST_L2FE_FLOODING: */
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,RTK_RG_ACLANDCF_RESERVED_MULTICAST_L2FE_FLOODING));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_MULTICAST_L2FE_FLOODING]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(rg_db.systemGlobal.dosRateLimitCount,0,rsvType));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_ARP_PACKET_ASSIGN_SHARE_METER:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0,rsvType));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.arp_packet_assign_share_meter,(rtk_rg_aclAndCf_reserved_arp_packet_assign_share_meter_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_arp_packet_assign_share_meter_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_ASSIGN_SHARE_METER:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.syn_packet_assign_share_meter,(rtk_rg_aclAndCf_reserved_syn_packet_assign_share_meter_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_syn_packet_assign_share_meter_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

#if 0		//mark these function due to its still under development.
		case RTK_RG_ACLANDCF_RESERVED_IGMP_MLD_DROP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.igmp_mld_drop_portmask,(rtk_rg_aclAndCf_reserved_igmp_mld_drop_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_igmp_mld_drop_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_IGMP_MLD_DROP]=ENABLED;
			break;
		case RTK_RG_ACLANDCF_RESERVED_INTF0_IPV4_FRAGMENT_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF1_IPV4_FRAGMENT_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF2_IPV4_FRAGMENT_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF3_IPV4_FRAGMENT_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF4_IPV4_FRAGMENT_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF5_IPV4_FRAGMENT_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF6_IPV4_FRAGMENT_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF7_IPV4_FRAGMENT_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF8_IPV4_FRAGMENT_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF9_IPV4_FRAGMENT_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF10_IPV4_FRAGMENT_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF11_IPV4_FRAGMENT_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF12_IPV4_FRAGMENT_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF13_IPV4_FRAGMENT_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF14_IPV4_FRAGMENT_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF15_IPV4_FRAGMENT_TRAP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(3,0,rsvType));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[rsvType-RTK_RG_ACLANDCF_RESERVED_INTF0_IPV4_FRAGMENT_TRAP],(rtk_rg_aclAndCf_reserved_intf_ipv4_fragment_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_ipv4_fragment_trap_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;
#endif	//mark these function due to its still under development.

		default:
			ACL_RSV("#####Reserved ACL reflash!(add reserved ACL rsvType=%d) ... G3 Bypass!!!#####",rsvType);
			WARNING("#####Reserved ACL reflash!(add reserved ACL rsvType=%d) ... G3 Bypass!!!#####",rsvType);
			break;
	}

forceReflash:
	_rtk_rg_aclAndCfReservedRuleReflash();


	return (RT_ERR_RG_OK);

}


int _rtk_rg_aclAndCfReservedRuleDel(rtk_rg_aclAndCf_reserved_type_t rsvType)
{
	ACL_RSV("#####Reserved ACL reflash!(del reserved ACL rsvType=%d)#####",rsvType);

	if(rsvType == RTK_RG_ACLANDCF_RESERVED_TAIL_END)
		goto forceReflash;

	if(rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]==DISABLED)
		return (RT_ERR_RG_OK);

	switch(rsvType){

		case RTK_RG_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY:
		case RTK_RG_ACLANDCF_RESERVED_L2TP_DATA_LOWER_PRIORITY:
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_L2TP_DATA_LOWER_PRIORITY]=DISABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_IPV4_FORWARD_L2FE:
			if(rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_MULTICAST_IPV6_FORWARD_L2FE]==DISABLED)
				rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_MULTICAST_L2FE_FLOODING]=DISABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_IPV6_FORWARD_L2FE:
			if(rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_MULTICAST_IPV4_FORWARD_L2FE]==DISABLED)
				rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_MULTICAST_L2FE_FLOODING]=DISABLED;
			break;

		default:
			break;
	}

	rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=DISABLED;

forceReflash:
	_rtk_rg_aclAndCfReservedRuleReflash();

	return (RT_ERR_RG_OK);
}


int _rtk_rg_aclAndCfReservedRuleAddSpecial(rtk_rg_aclAndCf_reserved_type_t rsvType, void *parameter)
{
	ca_classifier_rule_t ca_cls_rule;
	uint32 i;
	int addRuleFailedFlag=0;
	rtk_portmask_t activePorts;

	ACL_RSV("#####Add special ACL!(rsvType=%d)#####",rsvType);

	bzero(&ca_cls_rule,sizeof(ca_cls_rule));

	switch(rsvType){

		case RTK_CA_CLS_TYPE_L2_INGRESS_FORWARD_L3FE:
			//Create L2 igr Classifier for forward the packet to L3FE - only need to care physical port and wifi port 0x12~0x16
			bzero(&activePorts,sizeof(rtk_portmask_t));
			activePorts.bits[0] = RTK_RG_ALL_MAC_PORTMASK&(~(1<<RTK_RG_MAC_PORT_CPU))&(~(1<<RTK_RG_MAC_PORT_CPU_WLAN1_AND_OTHERS));
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
			activePorts.bits[0] = activePorts.bits[0] & (~(1<<G3_LOOPBACK_P_NEWSPA));
			activePorts.bits[0] = activePorts.bits[0] & (~(G3_LOOPBACK_P_MASK)); //packet from LAN port=>go to loopback port directly.
#endif
			CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
				ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L2_0;
				_rtk_rg_g3_caClsAssignPortType(&ca_cls_rule, i);

				ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
				if(i == RTK_RG_PORT_PON)
					ca_cls_rule.action.dest.port = AAL_LPORT_L3_WAN;
				else
					ca_cls_rule.action.dest.port = AAL_LPORT_L3_LAN;

				if(_rtk_rg_g3_caClsRuleAdd(&ca_cls_rule, rsvType)) {
					ACL_RSV("add special ACL RTK_CA_CLS_TYPE_L2_INGRESS_FORWARD_L3FE failed!!!");
					addRuleFailedFlag = 1;
					break;
				}
			}
			rg_db.systemGlobal.aclAndCfReservedRule.acl_wanPortMask = (1<<RTK_RG_PORT_PON);
			break;

		case RTK_CA_CLS_TYPE_IPV4_WITH_OPTION_TRAP:
			//Trap the packet to CPU if it is ipv4 with option, this should not impact bridge packet
			ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_15;
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.ipv4_with_option_trap,(rtk_rg_aclAndCf_reserved_ipv4_with_option_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_ipv4_with_option_trap_t));
			ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, rg_db.systemGlobal.aclAndCfReservedRule.ipv4_with_option_trap.src_port);
			ca_cls_rule.key_mask.src_port = 1;

			ca_cls_rule.key_mask.l2 = 1;
			memcpy(&(ca_cls_rule.key.l2.mac_da.mac_min),rg_db.systemGlobal.aclAndCfReservedRule.ipv4_with_option_trap.dst_mac,sizeof(rg_db.systemGlobal.aclAndCfReservedRule.ipv4_with_option_trap.dst_mac));
			memcpy(&(ca_cls_rule.key.l2.mac_da.mac_max),rg_db.systemGlobal.aclAndCfReservedRule.ipv4_with_option_trap.dst_mac,sizeof(rg_db.systemGlobal.aclAndCfReservedRule.ipv4_with_option_trap.dst_mac));
			ca_cls_rule.key_mask.l2_mask.mac_da = TRUE;

			ca_cls_rule.key_mask.ip = 1;
			ca_cls_rule.key.ip.have_options = 1;
			ca_cls_rule.key_mask.ip_mask.have_options = 1;

			ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
			ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;

			if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, rsvType, CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT))
			{
				ACL_RSV("add special ACL RTK_CA_CLS_TYPE_IPV4_WITH_OPTION_TRAP failed!!!");
				addRuleFailedFlag = 1;
				break;
			}
			((rtk_rg_aclAndCf_reserved_ipv4_with_option_trap_t*)parameter)->cls_index = ca_cls_rule.index;
			break;

		case RTK_CA_CLS_TYPE_L2_INGRESS_MULTICAST_FORWARD_L2FE:
			//MC packet from L3FE will include fake vlan id, it should keep going
			//the priority should higher than MC_FORWARD_L3FE
			bzero(&activePorts,sizeof(rtk_portmask_t));
			activePorts.bits[0] = RTK_RG_ALL_MAC_PORTMASK&(~(1<<RTK_RG_MAC_PORT_CPU))&(~(1<<RTK_RG_MAC_PORT_CPU_WLAN1_AND_OTHERS));
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
			activePorts.bits[0] = activePorts.bits[0] & (~(1<<G3_LOOPBACK_P_NEWSPA));
#endif
			CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
				ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L2_7;
				_rtk_rg_g3_caClsAssignPortType(&ca_cls_rule, i);

				ca_cls_rule.key.l2.vlan_count = 1;
				ca_cls_rule.key.l2.vlan_otag.vlan_min.vid = 0xFEF;	//should hake CFG_ID_FAKE_WAN_MC_VID to always use the first one
				ca_cls_rule.key.l2.vlan_otag.vlan_max = ca_cls_rule.key.l2.vlan_otag.vlan_min;
				ca_cls_rule.key_mask.l2 = 1;
				ca_cls_rule.key_mask.l2_mask.vlan_count = 1;
				ca_cls_rule.key_mask.l2_mask.vlan_otag = 1;
				ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.vid = 1;

				ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_FE;
				ca_cls_rule.action.dest.fe = CA_CLASSIFIER_FORWARD_FE_L2FE;

				if(_rtk_rg_g3_caClsRuleAdd(&ca_cls_rule, rsvType)) {
					ACL_RSV("add special ACL RTK_CA_CLS_TYPE_L2_INGRESS_MULTICAST_FORWARD_L2FE failed!!!");
					addRuleFailedFlag = 1;
					break;
				}
			}
			break;
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
		case RTK_CA_CLS_TYPE_L2_UPSTREAM_LOOPBACK_INGRESS_FORWARD_L3FE:
			//Create L2 igr Classifier for forward the packet from loopback port with SVLAN tag (upstream) to L3FE
			ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L2_0;
			_rtk_rg_g3_caClsAssignPortType(&ca_cls_rule, G3_LOOPBACK_P_NEWSPA);

			{
				//ca_cls_rule.key.l2.vlan_otag.vlan_min.tpid = 0x88A8;
				//ca_cls_rule.key.l2.vlan_otag.vlan_max.tpid = 0x88A8;
				ca_cls_rule.key.l2.vlan_otag.vlan_min.vid = G3_LOOPBACK_UPSTREAM_VID_P0;
				ca_cls_rule.key.l2.vlan_otag.vlan_max.vid = G3_LOOPBACK_UPSTREAM_VID_P3;

				ca_cls_rule.key_mask.l2 = TRUE;
				ca_cls_rule.key_mask.l2_mask.vlan_otag = TRUE;
				//ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.tpid= TRUE;
				ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.vid= TRUE;

				ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
				ca_cls_rule.action.dest.port = AAL_LPORT_L3_LAN;

				if(_rtk_rg_g3_caClsRuleAdd(&ca_cls_rule, rsvType)) {
					ACL_RSV("add special ACL RTK_CA_CLS_TYPE_L2_UPSTREAM_LOOPBACK_INGRESS_FORWARD_L3FE failed!!!");
					addRuleFailedFlag = 1;
				}
			}
			break;
		case RTK_CA_CLS_TYPE_L2_UPSTREAM_LAN_INGRESS_FORWARD_LOOPBACK:
			//Create L2 igr Classifier for forward the packet from LAN port to loopback port with SVLAN tag
			bzero(&activePorts,sizeof(rtk_portmask_t));
			activePorts.bits[0] = G3_LOOPBACK_P_MASK; //packet from LAN port=>go to loopback port directly.
			CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
				ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L2_0;
				_rtk_rg_g3_caClsAssignPortType(&ca_cls_rule, i);

				ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
				ca_cls_rule.action.dest.port = G3_LOOPBACK_P_NEWSPA;

				ca_cls_rule.action.options.masks.outer_vlan_act = 1;
				ca_cls_rule.action.options.masks.outer_tpid = 1;
				ca_cls_rule.action.options.outer_vlan_act = CA_CLASSIFIER_VLAN_ACTION_PUSH;
				ca_cls_rule.action.options.outer_vid = G3_LOOPBACK_UPSTREAM_VID(i);
				//ca_cls_rule.action.options.outer_tpid = 0x88A8;

				if(_rtk_rg_g3_caClsRuleAdd(&ca_cls_rule, rsvType)) {
					ACL_RSV("add special ACL RTK_CA_CLS_TYPE_L2_UPSTREAM_LAN_INGRESS_FORWARD_LOOPBACK failed!!!");
					addRuleFailedFlag = 1;
					break;
				}
			}
			break;
		case RTK_CA_CLS_TYPE_L2_INGRESS_SA_HOSTPOLICING:
			//Create L2 ***egress*** Classifier for ingress SA host policing (using port 0x19 egress CLS)
			ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L2_3;
			_rtk_rg_g3_caClsAssignDestPortType(&ca_cls_rule, AAL_LPORT_L3_LAN);

			ca_cls_rule.key.l2.vlan_otag.vlan_min.vid = G3_LOOPBACK_UPSTREAM_VID_P0;
			ca_cls_rule.key.l2.vlan_otag.vlan_max.vid = G3_LOOPBACK_UPSTREAM_VID_P3;
			memcpy(ca_cls_rule.key.l2.mac_sa.mac_min, ((rtk_rg_aclAndCf_reserved_hostPoliceSaDa_t*)parameter)->hostMac.octet, sizeof(ca_mac_addr_t));
			memcpy(ca_cls_rule.key.l2.mac_sa.mac_max, ((rtk_rg_aclAndCf_reserved_hostPoliceSaDa_t*)parameter)->hostMac.octet, sizeof(ca_mac_addr_t));

			ca_cls_rule.key_mask.l2 = TRUE;
			ca_cls_rule.key_mask.l2_mask.vlan_otag = TRUE;
			ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.vid = TRUE;
			ca_cls_rule.key_mask.l2_mask.mac_sa = TRUE;

			ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_FE;
			ca_cls_rule.action.dest.fe= CA_CLASSIFIER_FORWARD_FE_L2FE;

			ca_cls_rule.action.options.masks.action_handle = TRUE;
			ca_cls_rule.action.options.handle_type = CA_KEY_HANDLE_TYPE_FLOW_ID;
			ca_cls_rule.action.options.action_handle.flow_id = ((rtk_rg_aclAndCf_reserved_hostPoliceSaDa_t*)parameter)->flow_id;

			if(_rtk_rg_g3_caClsRuleAdd(&ca_cls_rule, rsvType)) {
				ACL_RSV("add special ACL RTK_CA_CLS_TYPE_L2_UPSTREAM_LAN_INGRESS_FORWARD_LOOPBACK failed!!!");
				addRuleFailedFlag = 1;
			}
			else
				((rtk_rg_aclAndCf_reserved_hostPoliceSaDa_t*)parameter)->ca_cls_index = ca_cls_rule.index;
			break;

		case RTK_CA_CLS_TYPE_L2_EGRESS_DA_HOSTPOLICING:
			//Create L2 ***ingress*** Classifier for ingress SA host policing (using port 0x6 ingress CLS)
			ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L2_3;
			_rtk_rg_g3_caClsAssignPortType(&ca_cls_rule, G3_LOOPBACK_P_NEWSPA);

			ca_cls_rule.key.l2.vlan_otag.vlan_min.vid = G3_LOOPBACK_DOWNSTREAM_VID_MIN;
			ca_cls_rule.key.l2.vlan_otag.vlan_max.vid = G3_LOOPBACK_DOWNSTREAM_VID_MAX;
			memcpy(ca_cls_rule.key.l2.mac_da.mac_min, ((rtk_rg_aclAndCf_reserved_hostPoliceSaDa_t*)parameter)->hostMac.octet, sizeof(ca_mac_addr_t));
			memcpy(ca_cls_rule.key.l2.mac_da.mac_max, ((rtk_rg_aclAndCf_reserved_hostPoliceSaDa_t*)parameter)->hostMac.octet, sizeof(ca_mac_addr_t));

			ca_cls_rule.key_mask.l2 = TRUE;
			ca_cls_rule.key_mask.l2_mask.vlan_otag = TRUE;
			ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.vid = TRUE;
			ca_cls_rule.key_mask.l2_mask.mac_da = TRUE;

			ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_FE;
			ca_cls_rule.action.dest.fe= CA_CLASSIFIER_FORWARD_FE_L2FE;

			ca_cls_rule.action.options.masks.action_handle = TRUE;
			ca_cls_rule.action.options.handle_type = CA_KEY_HANDLE_TYPE_FLOW_ID;
			ca_cls_rule.action.options.action_handle.flow_id = ((rtk_rg_aclAndCf_reserved_hostPoliceSaDa_t*)parameter)->flow_id;

			if(_rtk_rg_g3_caClsRuleAdd(&ca_cls_rule, rsvType)) {
				ACL_RSV("add special ACL RTK_CA_CLS_TYPE_L2_UPSTREAM_LAN_INGRESS_FORWARD_LOOPBACK failed!!!");
				addRuleFailedFlag = 1;
			}
			else
				((rtk_rg_aclAndCf_reserved_hostPoliceSaDa_t*)parameter)->ca_cls_index = ca_cls_rule.index;
			break;

#endif

		case RTK_CA_CLS_TYPE_REARRANGE_PROTECTION:
			bzero(&activePorts,sizeof(rtk_portmask_t));
			activePorts.bits[0] = CA_L3_CLS_PROFILE_LAN|CA_L3_CLS_PROFILE_WAN;
			CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
				ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_15;
				ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
				ca_cls_rule.key_mask.src_port= TRUE;

				if(rg_db.systemGlobal.aclRearrangeProtectWithAllPermit)
				{
					ACL_CTRL("add ACL PROTECT for ALL permit");
				}
				else
				{
					//IP DA is in MC IP range, ipv4_da[31:28]=4b1110, ipv6_da[127:120]=8hff
					ca_cls_rule.key_mask.ip = TRUE;
					ca_cls_rule.key.ip.is_multicast = TRUE;
					ca_cls_rule.key_mask.ip_mask.is_multicast = TRUE;
				}

				ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_FE;
				ca_cls_rule.action.dest.fe = CA_CLASSIFIER_FORWARD_FE_L3FE;

				if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, rsvType, CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT)) {
					ACL_RSV("add special ACL RTK_CA_CLS_TYPE_REARRANGE_PROTECTION failed!!!");
					addRuleFailedFlag=1;
					break;
				}
			}

			bzero(&ca_cls_rule,sizeof(ca_cls_rule));
			bzero(&activePorts,sizeof(rtk_portmask_t));
			activePorts.bits[0] = CA_L3_CLS_PROFILE_LAN|CA_L3_CLS_PROFILE_WAN;
			CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
				ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_15;
				ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
				ca_cls_rule.key_mask.src_port= TRUE;

				ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
				ca_cls_rule.action.dest.port = RTK_RG_PORT_CPU;

				if(_rtk_rg_g3_caClsRuleAALAdd(&ca_cls_rule, rsvType, CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT)) {
					ACL_RSV("add special ACL RTK_CA_CLS_TYPE_REARRANGE_PROTECTION failed!!!");
					addRuleFailedFlag=1;
					break;
				}
			}
			break;

		default:
			ACL_RSV("#####Unknown special ACL (rsvType=%d)!!!#####",rsvType);
			WARNING("#####Unknown special ACL (rsvType=%d)!!!#####",rsvType);
			break;
	}

	if(addRuleFailedFlag==1)
		return (RT_ERR_RG_FAILED);

	return (RT_ERR_RG_OK);
}


int _rtk_rg_aclAndCfReservedRuleDelSpecial(rtk_rg_aclAndCf_reserved_type_t rsvType, int index, void *parameter)
{
	ACL_RSV("#####Delete special ACL!(index=%d)#####", index);

	if(ca_classifier_rule_delete(G3_DEF_DEVID, index) == CA_E_OK) {
		rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=DISABLED;
		rg_db.systemGlobal.ca_cls_rule_record[index].valid = DISABLED;
		rg_db.systemGlobal.ca_cls_rule_record[index].priority = RTK_CA_CLS_PRIORITY_INIT;
		rg_db.systemGlobal.ca_cls_used_count--;
	}
	else {
		ACL_RSV("Delete special ACL %d failed!!!", index);
		WARNING("#####Delete special ACL %d failed!!!#####", index);
		return (RT_ERR_RG_FAILED);
	}

	return (RT_ERR_RG_OK);
}


/*(5)fwdEngine datapath used APIs*/
int _rtk_rg_aclDecisionClear(rtk_rg_pktHdr_t *pPktHdr)
{
#ifdef CONFIG_GPON_FEATURE
	if(rg_db.systemGlobal.initParam.wanPortGponMode){
		pPktHdr->streamID=126; //SYNC to HW if no CF SID action hit.
	}else{
		pPktHdr->streamID=0; //EPON default LLID
	}
#endif
	bzero(&pPktHdr->aclDecision.ACL_DECISION_EGR_PART_CLEAR, sizeof(rtk_rg_aclHitAndAction_t)-((POINTER_CAST)(&pPktHdr->aclDecision.ACL_DECISION_EGR_PART_CLEAR)-(POINTER_CAST)(&pPktHdr->aclDecision)));

	return (RT_ERR_RG_OK);
}


int _rtk_rg_ingressACLPatternCheck(rtk_rg_pktHdr_t *pPktHdr, int ingressCvidRuleIdxArray[])
{

	int i,acl_sw_idx;
	uint8 port;
	rtk_rg_aclFilterEntry_t* pChkRule;
	int ipv6_upper_range_check,ipv6_lowwer_range_check;
	int port_check;
	uint8 tos;
	int handleValidRuleCounter=0;
	int hitRuleCounter=0;
	uint32 possitive_check=0;

	if(ingressCvidRuleIdxArray==NULL){
		if(pPktHdr->aclDecision.aclIgrRuleChecked==1){
			return RT_ERR_RG_OK;
		}
		else{
			//normal _rtk_rg_ingressACLPatternCheck has been called. the result is recored in pPktHdr->aclDecision, no need to check whole function again!
			pPktHdr->aclDecision.aclIgrRuleChecked = 1;
		}
	}
	else
	{
		ACL("Check ingress part of RG_ACL: (especial for ingress_cvid action rules) ");
	}

	pPktHdr->aclPriority = -1; //initial acl priority
	pPktHdr->aclDecision.aclEgrHaveToCheckRuleIdx[hitRuleCounter]=-1; //empty from first rule until the end

	if(rg_db.systemGlobal.acl_SW_table_entry_size<=0){//no rule need to verify
		ACL("no RG_ACL need to verify");
		return RT_ERR_RG_OK;
	}

	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){

		if(handleValidRuleCounter >= rg_db.systemGlobal.acl_SW_table_entry_size){//no valid rule need to check, skip rest for loop
			ACL("no more valid ACLRule need to verify");
			break;
		}

		if(ingressCvidRuleIdxArray==NULL){//normal check all aclSWRule ingress part.
			if(rg_db.systemGlobal.acl_SW_table_entry[i].valid!=RTK_RG_ENABLED)//skip empty rule
				continue;
			pChkRule = &(rg_db.systemGlobal.acl_SW_table_entry[i]);
			acl_sw_idx = i;

			ACL("Check ingress part of ACLRule[%d]:",acl_sw_idx);
		}else{//especial check aclSWRule with ingress cvid action.
			if(ingressCvidRuleIdxArray[i]==-1){
				ACL("no more ingressVid ACLRule need to verify");
				break;
			}
			pChkRule = &(rg_db.systemGlobal.acl_SW_table_entry[ingressCvidRuleIdxArray[i]]);
			acl_sw_idx = ingressCvidRuleIdxArray[i];
			ACL("Check ingress part of RG_ACL[%d]: (especial for ingress cvid action rules)",ingressCvidRuleIdxArray[i]);
		}

		handleValidRuleCounter++; //a valid rule is going to check


		if(RTK_RG_IS_WAN_PORT(pPktHdr->ingressPort)) //downstream
		{
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP)
			{
				ACL("RG_ACL[%d] FWD_TYPE UNHIT DOWNSTREAM ",acl_sw_idx);
				continue;
			}
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN)
			{
				ACL("RG_ACL[%d] FWD_TYPE UNHIT DOWNSTREAM ",acl_sw_idx);
				continue;
			}
		}
		else //upstream
		{
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP)
			{
				ACL("RG_ACL[%d] FWD_TYPE UNHIT UPSTREAM ",acl_sw_idx);
				continue;
			}
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN)
			{
				ACL("RG_ACL[%d] FWD_TYPE UNHIT UPSTREAM ",acl_sw_idx);
				continue;
			}
		}

		/*ingress rule check*/
		port_check = FAIL;
		if(pChkRule->acl_filter.filter_fields&INGRESS_PORT_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			for(port=0;port<RTK_RG_PORT_MAX;port++){//including extport
				if(RG_INVALID_PORT(port))
					continue;
				if(pChkRule->acl_filter.ingress_port_mask.portmask & (1<<port)){
					if(port==pPktHdr->ingressPort){
						port_check = SUCCESS;
					}
				}
			}
			if(port_check!=SUCCESS){
				//ACL("RG_ACL[%d] INGRESS_PORT UNHIT",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_PORT_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_PORT UNHIT");
		}



		if(pChkRule->acl_filter.filter_fields&INGRESS_INTF_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.ingress_intf_idx!=pPktHdr->srcNetifIdx){
				//ACL("RG_ACL[%d] INGRESS_INTF_BIT UNHIT",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_INTF_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_INTF_BIT UNHIT");
		}


		if(pChkRule->acl_filter.filter_fields&INGRESS_STREAM_ID_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if((pChkRule->acl_filter.ingress_stream_id&pChkRule->acl_filter.ingress_stream_id_mask)!=(pPktHdr->pRxDesc->rx_pon_stream_id&pChkRule->acl_filter.ingress_stream_id_mask)){
				//ACL("RG_ACL[%d] INGRESS_STREAM_ID_BIT UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_STREAM_ID_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_STREAM_ID_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV4_TAGIF_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.ingress_ipv4_tagif){//must have ip header
				if(pPktHdr->tagif&IPV4_TAGIF){
					//hit do nothing
				}else{
					//ACL("RG_ACL[%d] INGRESS_IPV4_TAGIF_BIT UNHIT ",acl_sw_idx);
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}else{//must not have IP header
				if(pPktHdr->tagif&IPV4_TAGIF){
					//ACL("RG_ACL[%d] INGRESS_IPV4_TAGIF_BIT UNHIT ",acl_sw_idx);
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}else{
					//hit do nothing
				}
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_IPV4_TAGIF_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_IPV4_TAGIF_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_TAGIF_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.ingress_ipv6_tagif){//must have ip header
				if(pPktHdr->tagif&IPV6_TAGIF){
					//hit do nothing
				}else{
					//ACL("RG_ACL[%d] INGRESS_IPV6_TAGIF_BIT UNHIT ",acl_sw_idx);
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}else{//must not have IP header
				if(pPktHdr->tagif&IPV6_TAGIF){
					//ACL("RG_ACL[%d] INGRESS_IPV6_TAGIF_BIT UNHIT ",acl_sw_idx);
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}else{
					//hit do nothing
				}
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_IPV6_TAGIF_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_IPV6_TAGIF_BIT UNHIT");
		}


		if(pChkRule->acl_filter.filter_fields&INGRESS_STAGIF_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.ingress_stagIf){//must have stag
				if((pPktHdr->tagif&SVLAN_TAGIF)==0x0){
					//ACL("RG_ACL[%d] INGRESS_STAGIF_BIT UNHIT ",acl_sw_idx);
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}else{//must not have stag
				if(pPktHdr->tagif&SVLAN_TAGIF){
					//ACL("RG_ACL[%d] INGRESS_STAGIF_BIT UNHIT ",acl_sw_idx);
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_STAGIF_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_STAGIF_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_CTAGIF_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.ingress_ctagIf){//must have ctag
				if((pPktHdr->tagif&CVLAN_TAGIF)==0x0){
					//ACL("RG_ACL[%d] INGRESS_CTAGIF_BIT UNHIT ",acl_sw_idx);
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}else{//must not have ctag
				if(pPktHdr->tagif&CVLAN_TAGIF){
					//ACL("RG_ACL[%d] INGRESS_CTAGIF_BIT UNHIT ",acl_sw_idx);
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_CTAGIF_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_CTAGIF_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_ETHERTYPE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if((pChkRule->acl_filter.ingress_ethertype & pChkRule->acl_filter.ingress_ethertype_mask)!=(pPktHdr->etherType & pChkRule->acl_filter.ingress_ethertype_mask)){
				//ACL("RG_ACL[%d] INGRESS_ETHERTYPE UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_ETHERTYPE_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_ETHERTYPE_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_STAG_PRI_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.ingress_stag_pri!=pPktHdr->stagPri){
				//ACL("RG_ACL[%d] INGRESS_STAG_PRI UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_STAG_PRI_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_STAG_PRI_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_STAG_VID_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.ingress_stag_vid!=pPktHdr->stagVid){
				//ACL("RG_ACL[%d] INGRESS_STAG_VID UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_STAG_VID_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_STAG_VID_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_STAG_DEI_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.ingress_stag_dei!=pPktHdr->stagDei){
				//ACL("RG_ACL[%d] INGRESS_STAG_DEI UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_STAG_DEI_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_STAG_DEI_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_CTAG_PRI_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.ingress_ctag_pri!=pPktHdr->ctagPri){
				//possitive_check = ACL_PATTERN_UNHIT; //continue;ACL("RG_ACL[%d] INGRESS_CTAG_PRI UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_CTAG_PRI_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_CTAG_PRI_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_CTAG_VID_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.ingress_ctag_vid!=pPktHdr->ctagVid){
				//ACL("RG_ACL[%d] INGRESS_CTAG_VID UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_CTAG_VID_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_CTAG_VID_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_CTAG_CFI_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.ingress_ctag_cfi!=pPktHdr->ctagCfi){
				//ACL("RG_ACL[%d] INGRESS_CTAG_CFI UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_CTAG_CFI_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_CTAG_CFI_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_SMAC_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if((pChkRule->acl_filter.ingress_smac.octet[0] & pChkRule->acl_filter.ingress_smac_mask.octet[0])!=(pPktHdr->pSmac[0]& pChkRule->acl_filter.ingress_smac_mask.octet[0]) ||
				(pChkRule->acl_filter.ingress_smac.octet[1] & pChkRule->acl_filter.ingress_smac_mask.octet[1])!=(pPktHdr->pSmac[1]& pChkRule->acl_filter.ingress_smac_mask.octet[1])||
				(pChkRule->acl_filter.ingress_smac.octet[2] & pChkRule->acl_filter.ingress_smac_mask.octet[2])!=(pPktHdr->pSmac[2]& pChkRule->acl_filter.ingress_smac_mask.octet[2])||
				(pChkRule->acl_filter.ingress_smac.octet[3] & pChkRule->acl_filter.ingress_smac_mask.octet[3])!=(pPktHdr->pSmac[3]& pChkRule->acl_filter.ingress_smac_mask.octet[3])||
				(pChkRule->acl_filter.ingress_smac.octet[4] & pChkRule->acl_filter.ingress_smac_mask.octet[4])!=(pPktHdr->pSmac[4]& pChkRule->acl_filter.ingress_smac_mask.octet[4])||
				(pChkRule->acl_filter.ingress_smac.octet[5] & pChkRule->acl_filter.ingress_smac_mask.octet[5])!=(pPktHdr->pSmac[5]& pChkRule->acl_filter.ingress_smac_mask.octet[5])
				){
				//memDump(pPktHdr->pSmac,6,"pktHdr pSmac");
				//memDump(pChkRule->acl_filter.ingress_smac.octet,6,"acl Smac");
				//ACL("RG_ACL[%d] INGRESS_SMAC UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_SMAC_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_SMAC_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_DMAC_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if((pChkRule->acl_filter.ingress_dmac.octet[0] & pChkRule->acl_filter.ingress_dmac_mask.octet[0])!=(pPktHdr->pDmac[0] & pChkRule->acl_filter.ingress_dmac_mask.octet[0]) ||
				(pChkRule->acl_filter.ingress_dmac.octet[1] & pChkRule->acl_filter.ingress_dmac_mask.octet[1])!=(pPktHdr->pDmac[1] & pChkRule->acl_filter.ingress_dmac_mask.octet[1])||
				(pChkRule->acl_filter.ingress_dmac.octet[2] & pChkRule->acl_filter.ingress_dmac_mask.octet[2])!=(pPktHdr->pDmac[2] & pChkRule->acl_filter.ingress_dmac_mask.octet[2])||
				(pChkRule->acl_filter.ingress_dmac.octet[3] & pChkRule->acl_filter.ingress_dmac_mask.octet[3])!=(pPktHdr->pDmac[3] & pChkRule->acl_filter.ingress_dmac_mask.octet[3])||
				(pChkRule->acl_filter.ingress_dmac.octet[4] & pChkRule->acl_filter.ingress_dmac_mask.octet[4])!=(pPktHdr->pDmac[4] & pChkRule->acl_filter.ingress_dmac_mask.octet[4])||
				(pChkRule->acl_filter.ingress_dmac.octet[5] & pChkRule->acl_filter.ingress_dmac_mask.octet[5])!=(pPktHdr->pDmac[5] & pChkRule->acl_filter.ingress_dmac_mask.octet[5])
				){
				//memDump(pPktHdr->pDmac,6,"pktHdr pDmac");
				//memDump(pChkRule->acl_filter.ingress_dmac.octet,6,"acl Dmac");
				//ACL("RG_ACL[%d] INGRESS_DMAC UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_DMAC_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_DMAC_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_DSCP_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pPktHdr->pTos==NULL){//pkt without DSCP
				//ACL("RG_ACL[%d] INGRESS_DSCP UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}else{
				tos = *(pPktHdr->pTos);
				if(pChkRule->acl_filter.ingress_dscp!=(tos>>2) || (pPktHdr->tagif&IPV4_TAGIF)==0x0 /*must be ipv4*/){
					//ACL("RG_ACL[%d] INGRESS_DSCP UNHIT ",acl_sw_idx);
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_DSCP_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_DSCP_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_TOS_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pPktHdr->pTos==NULL){//pkt without DSCP
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}else{
				tos = *(pPktHdr->pTos);
				if(pChkRule->acl_filter.ingress_tos!=tos || (pPktHdr->tagif&IPV4_TAGIF)==0x0 /*must be ipv4*/){
					//ACL("RG_ACL[%d] INGRESS_TOS UNHIT",acl_sw_idx);
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_TOS_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_TOS_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_DSCP_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pPktHdr->pTos==NULL){//pkt without DSCP
				//ACL("RG_ACL[%d] INGRESS_IPV6_DSCP_BIT UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}else{
				tos = (*(pPktHdr->pTos))<<4 & 0xf0;
				tos |= (*((pPktHdr->pTos)+1))>>4 & 0xf;
				if(pChkRule->acl_filter.ingress_ipv6_dscp!=(tos>>2) || (pPktHdr->tagif&IPV6_TAGIF)==0x0 /*must be ipv6*/){
					//ACL("RG_ACL[%d] INGRESS_IPV6_DSCP UNHIT ",acl_sw_idx);
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_IPV6_DSCP_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_IPV6_DSCP_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_TC_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pPktHdr->pTos==NULL){//pkt without DSCP
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}else{
				tos = (*(pPktHdr->pTos))<<4 & 0xf0;
				tos |= (*((pPktHdr->pTos)+1))>>4 & 0xf;
				if(pChkRule->acl_filter.ingress_ipv6_tc!=tos || (pPktHdr->tagif&IPV6_TAGIF)==0x0 /*must be ipv6*/){
					//ACL("RG_ACL[%d] INGRESS_IPV6_TC UNHIT",acl_sw_idx);
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_IPV6_TC_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_IPV6_TC_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_TCP_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if( !(pPktHdr->tagif&TCP_TAGIF)){
				//ACL("RG_ACL[%d] INGRESS_L4_TCP UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_L4_TCP_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_L4_TCP_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_UDP_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if( !(pPktHdr->tagif&UDP_TAGIF)){
				//ACL("RG_ACL[%d] INGRESS_L4_UDP UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_L4_UDP_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_L4_UDP_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_ICMP_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(!(pPktHdr->tagif&ICMP_TAGIF)){
				//ACL("RG_ACL[%d] INGRESS_L4_ICMP UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_L4_ICMP_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_L4_ICMP_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_ICMPV6_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(!(pPktHdr->tagif&ICMPV6_TAGIF)){
				//ACL("RG_ACL[%d] INGRESS_L4_ICMPV6_BIT UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_L4_ICMPV6_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_L4_ICMPV6_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_POROTCAL_VALUE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if((pPktHdr->ipProtocol)!=(pChkRule->acl_filter.ingress_l4_protocal)){
				//ACL("RG_ACL[%d] INGRESS_L4_POROTCAL_VALUE_BIT UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_L4_POROTCAL_VALUE_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_L4_POROTCAL_VALUE_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_SIP_RANGE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			ipv6_upper_range_check = -1;
			ipv6_lowwer_range_check = -1;
			if(pPktHdr->pIpv6Sip != NULL){
				ipv6_upper_range_check = memcmp(&(pChkRule->acl_filter.ingress_src_ipv6_addr_end[0]),pPktHdr->pIpv6Sip,16);
				 ipv6_lowwer_range_check = memcmp(pPktHdr->pIpv6Sip,&(pChkRule->acl_filter.ingress_src_ipv6_addr_start[0]),16);
			}
			if(!(ipv6_upper_range_check>=0 && ipv6_lowwer_range_check>=0 )){
				//ACL("RG_ACL[%d] INGRESS_IPV6_SIP_RANGE UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_IPV6_SIP_RANGE_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_IPV6_SIP_RANGE_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_SIP_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pPktHdr->pIpv6Sip != NULL){
				if((pPktHdr->pIpv6Sip[0]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[0])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[0]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[0]) ||
					(pPktHdr->pIpv6Sip[1]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[1])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[1]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[1]) ||
					(pPktHdr->pIpv6Sip[2]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[2])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[2]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[2]) ||
					(pPktHdr->pIpv6Sip[3]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[3])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[3]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[3]) ||
					(pPktHdr->pIpv6Sip[4]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[4])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[4]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[4]) ||
					(pPktHdr->pIpv6Sip[5]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[5])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[5]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[5]) ||
					(pPktHdr->pIpv6Sip[6]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[6])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[6]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[6]) ||
					(pPktHdr->pIpv6Sip[7]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[7])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[7]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[7]) ||
					(pPktHdr->pIpv6Sip[8]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[8])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[8]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[8]) ||
					(pPktHdr->pIpv6Sip[9]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[9])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[9]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[9]) ||
					(pPktHdr->pIpv6Sip[10]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[10])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[10]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[10]) ||
					(pPktHdr->pIpv6Sip[11]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[11])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[11]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[11]) ||
					(pPktHdr->pIpv6Sip[12]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[12])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[12]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[12]) ||
					(pPktHdr->pIpv6Sip[13]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[13])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[13]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[13]) ||
					(pPktHdr->pIpv6Sip[14]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[14])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[14]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[14]) ||
					(pPktHdr->pIpv6Sip[15]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[15])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[15]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[15])
					){
						//ACL("RG_ACL[%d] INGRESS_IPV6_SIP_BIT UNHIT ",acl_sw_idx);
						possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}else{
				//ACL("RG_ACL[%d] INGRESS_IPV6_SIP_BIT UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_IPV6_SIP_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_IPV6_SIP_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_DIP_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pPktHdr->pIpv6Dip != NULL){
				if((pPktHdr->pIpv6Dip[0]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[0])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[0]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[0]) ||
					(pPktHdr->pIpv6Dip[1]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[1])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[1]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[1]) ||
					(pPktHdr->pIpv6Dip[2]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[2])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[2]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[2]) ||
					(pPktHdr->pIpv6Dip[3]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[3])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[3]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[3]) ||
					(pPktHdr->pIpv6Dip[4]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[4])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[4]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[4]) ||
					(pPktHdr->pIpv6Dip[5]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[5])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[5]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[5]) ||
					(pPktHdr->pIpv6Dip[6]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[6])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[6]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[6]) ||
					(pPktHdr->pIpv6Dip[7]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[7])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[7]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[7]) ||
					(pPktHdr->pIpv6Dip[8]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[8])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[8]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[8]) ||
					(pPktHdr->pIpv6Dip[9]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[9])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[9]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[9]) ||
					(pPktHdr->pIpv6Dip[10]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[10])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[10]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[10]) ||
					(pPktHdr->pIpv6Dip[11]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[11])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[11]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[11]) ||
					(pPktHdr->pIpv6Dip[12]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[12])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[12]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[12]) ||
					(pPktHdr->pIpv6Dip[13]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[13])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[13]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[13]) ||
					(pPktHdr->pIpv6Dip[14]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[14])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[14]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[14]) ||
					(pPktHdr->pIpv6Dip[15]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[15])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[15]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[15])
					){
						//ACL("RG_ACL[%d] INGRESS_IPV6_DIP_BIT UNHIT ",acl_sw_idx);
						possitive_check = ACL_PATTERN_UNHIT; //continue;
				}


			}else{
				//ACL("RG_ACL[%d] INGRESS_IPV6_DIP_BIT UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_IPV6_DIP_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_IPV6_DIP_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_DIP_RANGE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			ipv6_upper_range_check = -1;
			ipv6_lowwer_range_check = -1;
			if(pPktHdr->pIpv6Dip!=NULL){
				ipv6_upper_range_check = memcmp(&(pChkRule->acl_filter.ingress_dest_ipv6_addr_end[0]),pPktHdr->pIpv6Dip,16);
				ipv6_lowwer_range_check = memcmp(pPktHdr->pIpv6Dip,&(pChkRule->acl_filter.ingress_dest_ipv6_addr_start[0]),16);
			}
			if(!(ipv6_upper_range_check>=0 && ipv6_lowwer_range_check>=0 )){
				//ACL("RG_ACL[%d] INGRESS_IPV6_DIP_RANGE UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_IPV6_DIP_RANGE_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_IPV6_DIP_RANGE_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV4_SIP_RANGE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if( !((pChkRule->acl_filter.ingress_src_ipv4_addr_end >= pPktHdr->ipv4Sip ) &&
				(pChkRule->acl_filter.ingress_src_ipv4_addr_start<= pPktHdr->ipv4Sip ))){
				//ACL("RG_ACL[%d] INGRESS_IPV4_SIP_RANGE UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_IPV4_SIP_RANGE_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_IPV4_SIP_RANGE_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV4_DIP_RANGE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if( !((pChkRule->acl_filter.ingress_dest_ipv4_addr_end >= pPktHdr->ipv4Dip ) &&
				(pChkRule->acl_filter.ingress_dest_ipv4_addr_start<= pPktHdr->ipv4Dip ))){
				//ACL("RG_ACL[%d] INGRESS_IPV4_DIP_RANGE UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_IPV4_DIP_RANGE_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_IPV4_DIP_RANGE_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_SPORT_RANGE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if( !((pChkRule->acl_filter.ingress_src_l4_port_end >= pPktHdr->sport ) &&
				(pChkRule->acl_filter.ingress_src_l4_port_start<= pPktHdr->sport ))){
				//ACL("RG_ACL[%d] INGRESS_L4_SPORT_RANGE UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_L4_SPORT_RANGE_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_L4_SPORT_RANGE_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_DPORT_RANGE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if( !((pChkRule->acl_filter.ingress_dest_l4_port_end >= pPktHdr->dport ) &&
				(pChkRule->acl_filter.ingress_dest_l4_port_start<= pPktHdr->dport ))){
				//ACL("RG_ACL[%d] INGRESS_L4_DPORT_RANGE UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_L4_DPORT_RANGE_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_L4_DPORT_RANGE_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_FLOWLABEL_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.ingress_ipv6_flow_label!=pPktHdr->ipv6FlowLebal){
				//possitive_check = ACL_PATTERN_UNHIT; //continue;ACL("RG_ACL[%d] INGRESS_IPV6_FLOWLABEL_BIT UNHIT ",acl_sw_idx);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_IPV6_FLOWLABEL_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_IPV6_FLOWLABEL_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_WLANDEV_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if((pPktHdr->wlan_dev_idx >= 0) && (pChkRule->acl_filter.ingress_wlanDevMask&(1<<pPktHdr->wlan_dev_idx))){
				//hit!
			}else{
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_WLANDEV_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_WLANDEV_BIT UNHIT");
		}


		if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_POLICY_ROUTE){
			PATTERN_CHECK_INIT(possitive_check);
			if(pPktHdr->isGatewayPacket==0)
				possitive_check = ACL_PATTERN_UNHIT;
			PATTERN_CHECK_RESULT(acl_sw_idx,0, possitive_check, "RG_ACL[%d] Policy Route not support DMAC != GMAC");
			if((1<<pPktHdr->ingressMacPort) & rg_db.systemGlobal.wanPortMask.portmask)
				possitive_check = ACL_PATTERN_UNHIT;
			PATTERN_CHECK_RESULT(acl_sw_idx,0, possitive_check, "RG_ACL[%d] Policy Route not support downstream");
		}



		//record ACL hit rule
		ACL("RG_ACL[%d] Ingress Part Hit!",acl_sw_idx);
		if(ingressCvidRuleIdxArray==NULL){
			pPktHdr->aclDecision.aclIgrHitMask[(acl_sw_idx>>5)]|=(1<<(acl_sw_idx&0x1f));
			pPktHdr->aclDecision.aclEgrHaveToCheckRuleIdx[hitRuleCounter]=acl_sw_idx; //record current rule
			pPktHdr->aclDecision.aclEgrHaveToCheckRuleIdx[hitRuleCounter+1]=-1;//empty next rule until the end
			hitRuleCounter++;
		}else{
			//ingress_cvid action can only be single hit
			if(pChkRule->acl_filter.qos_actions & ACL_ACTION_ACL_INGRESS_VID_BIT){
				pPktHdr->internalVlanID =  pChkRule->acl_filter.action_acl_ingress_vid;
				TRACE("RG_ACL[%d] HIT: internalVlanID modify to %d",acl_sw_idx,pPktHdr->internalVlanID);
			}
			break; //just check the first ACL CACT hit rule when ingressCvidRuleIdxArray!=NULL
		}

	}

	return RT_ERR_RG_OK;
}

rtk_rg_fwdEngineReturn_t _rtk_rg_ingressACLAction(rtk_rg_pktHdr_t *pPktHdr)
{
	/*
	*	This API is using for do pure ingress ACL actions such as:	acl_priority, drop, trap_to_PS
	*	remarking actions will be done after _rtk_rg_egressACLAction() & _rtk_rg_modifyPacketByACLAction()
	*/


	uint32 i, aclIdx=0;
	rtk_rg_aclFilterEntry_t* pChkRule;
	//TRACE("Ingress ACL Action:");

	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){
		aclIdx = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i];

		if(aclIdx==-1){
			// no more SW_acl rules
			break;
		}
		else
		{
			pChkRule= &rg_db.systemGlobal.acl_SW_table_entry[aclIdx];

			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
				&& (pPktHdr->aclDecision.aclIgrHitMask[(aclIdx>>5)]&(1<<(aclIdx&0x1f))))
			{

				if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_TRAP_WITH_PRIORITY &&
					((pPktHdr->aclDecision.aclIgrDoneAction & RG_IGR_PRIORITY_ACT_DONE_BIT)==0x0)){

					pPktHdr->aclPriority = pChkRule->acl_filter.action_trap_with_priority;	//also support high queue in tx
					ACL("RG_ACL[%d] do ACL_PRIORITY to %d", aclIdx, pPktHdr->aclPriority);
					pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_PRIORITY_ACT_DONE_BIT; //the ACL IGR_PRIORITY_ACT is done
				}

				if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_QOS &&
					(pChkRule->acl_filter.qos_actions & ACL_ACTION_ACL_PRIORITY_BIT) &&
					((pPktHdr->aclDecision.aclIgrDoneAction & RG_IGR_PRIORITY_ACT_DONE_BIT)==0x0)){

					if((pChkRule->acl_filter.filter_fields&EGRESS_ACL_PATTERN_BITS)==0x0)
					{	//due to supporting egress pattern (EGRESS_INTF_BIT) for fwdtype==0, so only update here when no egress pattern
						pPktHdr->aclPriority = pChkRule->acl_filter.action_acl_priority;
						ACL("RG_ACL[%d] do ACL_PRIORITY to %d", aclIdx, pPktHdr->aclPriority);
						pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_PRIORITY_ACT_DONE_BIT; //the ACL IGR_PRIORITY_ACT is done
					}
					else
					{
						ACL("RG_ACL[%d] postpone to update decision(acl_priority %d) until egressAction due to include egress pattern",aclIdx, pChkRule->acl_filter.action_acl_priority);
						TRACE("RG_ACL[%d] postpone the update decision(acl_priority %d) until egressAction due to include egress pattern",aclIdx, pChkRule->acl_filter.action_acl_priority);
					}
				}

				if(pPktHdr->aclPolicyRoute==FAIL && pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_POLICY_ROUTE)
				{
					ACL("RG_ACL[%d] HIT: Policy Route! use WAN[%d] as egress intf!!",aclIdx,pChkRule->acl_filter.action_policy_route_wan);
					//Keep policy route indicated egress WAN interface index in pktHdr
					pPktHdr->aclPolicyRoute=pChkRule->acl_filter.action_policy_route_wan;
				}

				if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_SW_MIRROR_WITH_UDP_ENCAP)
				{
					if(pChkRule->acl_filter.filter_fields&EGRESS_ACL_PATTERN_BITS){
						ACL("RG_ACL[%d] postpone the mirror until egressAction due to include egress pattern",aclIdx);
						TRACE("RG_ACL[%d] postpone the mirror until egressAction due to include egress pattern",aclIdx);
					}
					else if((pPktHdr->aclDecision.aclIgrDoneAction&RG_IGR_MIRROR_UDP_ENCAP_ACT_DONE_BIT)==0x0 && ((pPktHdr->tagif & IPV4_TAGIF) || (pPktHdr->tagif & IPV6_TAGIF)) /*support ipv4 and ipv6*/){
						pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_MIRROR_UDP_ENCAP_ACT_DONE_BIT;
						pPktHdr->aclDecision.action_igr_encap_udp=pChkRule->acl_filter.action_encap_udp;
						pPktHdr->aclDecision.igr_encap_acl_idx=aclIdx;
						ACL("RG_ACL[%d] MIRROR and UDP_ENCAPSULATION!",aclIdx);
						TRACE("RG_ACL[%d] MIRROR and UDP_ENCAPSULATION!",aclIdx);
					}
				}

				if((pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_DROP) || 
					(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_SW_DROP) || 
					(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_PERMIT) || 
					(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_TRAP_TO_PS))
				{
					if((pPktHdr->aclDecision.aclIgrDoneAction&RG_IGR_FORWARD_ACT_DONE_BIT)==0x0)
					{
						if(pPktHdr->aclDecision.aclIgrDoneAction&RG_IGR_FORWARD_ACT_POSTPONE_BIT)
						{	//postpone the lower priority fwd decision due to one of the higher priority rule should be.
							ACL("RG_ACL[%d] skip forward(action_type %d), because IGR_FORWARD_ACT should be postpone!", aclIdx, pChkRule->acl_filter.action_type);
							TRACE("RG_ACL[%d] skip forward(action_type %d), because IGR_FORWARD_ACT should be postpone!", aclIdx, pChkRule->acl_filter.action_type);
						}
						else if((pChkRule->acl_filter.filter_fields&EGRESS_ACL_PATTERN_BITS)==0x0)
						{	//due to supporting egress pattern for fwdtype==0, so permit/drop/trap only when no egress pattern
							if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_PERMIT)
							{
								ACL("RG_ACL[%d]: PERMIT",aclIdx);
								TRACE("ACL[%d] ACT:PERMIT",aclIdx);
								pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_FORWARD_ACT_DONE_BIT;
							}
							else if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_DROP)
							{
								ACL("RG_ACL[%d]: DROP",aclIdx);
								TRACE("[Drop] ACL[%d] ACT:DROP",aclIdx);
								pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_FORWARD_ACT_DONE_BIT;
								return RG_FWDENGINE_RET_DROP;
							}
							else if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_SW_DROP)
							{
								ACL("RG_ACL[%d]: SW DROP",aclIdx);
								TRACE("[Drop] ACL[%d] ACT:DROP",aclIdx);
								pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_FORWARD_ACT_DONE_BIT;
								return RG_FWDENGINE_RET_DROP;
							}
							else if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_TRAP_TO_PS)
							{
								ACL("RG_ACL[%d] TRAP to PS",aclIdx);
								TRACE("[To PS] ACL[%d] ACT:TRAP to PS",aclIdx);
								pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_FORWARD_ACT_DONE_BIT;
								pPktHdr->byPassToPsVlanAclDecision = 1;
								return RG_FWDENGINE_RET_TO_PS;
							}
						}
						else
						{
							ACL("RG_ACL[%d] postpone the decision(action_type=%d) until egressAction due to include egress pattern",aclIdx, pChkRule->acl_filter.action_type);
							TRACE("RG_ACL[%d] postpone the decision(action_type=%d) until egressAction due to include egress pattern",aclIdx, pChkRule->acl_filter.action_type);
							pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_FORWARD_ACT_POSTPONE_BIT;
						}
					}
					else
						ACL("RG_ACL[%d] skip forward(action_type %d), because IGR_FORWARD_ACT is already done!", aclIdx, pChkRule->acl_filter.action_type);
				}
			}
		}
	}

	return RG_FWDENGINE_RET_CONTINUE;

}

int _rtk_rg_egressACLAction_beforeCFcheck(rtk_rg_pktHdr_t *pPktHdr)
{
	uint32 i,acl_idx;
	rtk_rg_aclFilterEntry_t* pChkRule;

	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++)
	{
		if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i]==-1)
			break;	//no more SW_ACL rules

		acl_idx = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i];

		if((pPktHdr->aclDecision.aclIgrHitMask[(acl_idx>>5)]&(1<<(acl_idx&0x1f))) && (pPktHdr->aclDecision.aclEgrHitMask[(acl_idx>>5)]&(1<<(acl_idx&0x1f)))){

			pChkRule = &(rg_db.systemGlobal.acl_SW_table_entry[acl_idx]);

			if(pChkRule->acl_filter.fwding_type_and_direction!=ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
				continue;

			if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_QOS) {

				/*CF (fwdtype=1~4) pattern should reference for the result after ACL (fwdtype=0) modify
					After review ACL egress pattern, only support below action for this kind of case */

				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_ACL_PRIORITY_BIT){
					//aclPriority is used for internalPriority decision, but it also maybe reference by CF(fwd_type!=0), so we need to udpate this value in egress for fwd_type=0.

					DEBUG("RG_ACL[%d] Hit!!! pPktHdr->aclDecision.aclIgrDoneAction = 0x%x",acl_idx, pPktHdr->aclDecision.aclIgrDoneAction);

					if((pChkRule->acl_filter.filter_fields&EGRESS_ACL_PATTERN_BITS)==0x0 && (pPktHdr->aclDecision.aclIgrDoneAction & RG_IGR_PRIORITY_ACT_DONE_BIT)==0x0) {
						//due to aclIgrDoneAction will be reset before egress check, so update the action bit
						ACL("RG_ACL[%d] pure ingress rule, only update ACL_PRIORITY action bit", acl_idx);
						pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_PRIORITY_ACT_DONE_BIT; //the ACL IGR_PRIORITY_ACT is done
					}
					else if((pPktHdr->aclDecision.aclIgrDoneAction & RG_IGR_PRIORITY_ACT_DONE_BIT)==0x0) {
						pPktHdr->internalPriority = pChkRule->acl_filter.action_acl_priority;
						ACL("RG_ACL[%d] do ACL_PRIORITY to %d in egressAction for fwd_type=0", acl_idx, pPktHdr->internalPriority);
						TRACE("RG_ACL[%d] do ACL_PRIORITY to %d in egressAction for fwd_type=0", acl_idx, pPktHdr->internalPriority);
						pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_PRIORITY_ACT_DONE_BIT; //the ACL IGR_PRIORITY_ACT is done
					}
					else
					{
						ACL("RG_ACL[%d] skip ACL_PRIORITY to %d for fwd_type=0, because IGR_PRIORITY_ACT is already done!", acl_idx, pChkRule->acl_filter.action_acl_priority);
					}
				}

			}
		}
	}

	return RT_ERR_RG_OK;
}

int _rtk_rg_egressACLPatternCheck(int direct, int naptIdx, rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb,int l3Modify,int l4Modify, rtk_rg_port_idx_t egressPort)
{
	//egressPort used for confirm packet egress to CF port, egressPort==-1 represent BC to lan
	uint32 i,index;
	rtk_rg_aclFilterEntry_t* pChkRule;
	ipaddr_t sipModify,dipModify;
	uint16 sportModify,dportModify;
	uint8 dmacModify[ETHER_ADDR_LEN];
	uint8 smacModify[ETHER_ADDR_LEN];
	uint32 possitive_check;
	uint8 round_check = ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;	//check fwd_type=0 first, then check fwd_type!=0

	//int dipL3Idx,nexthopIdx;
	//int netifIdx=-1;
	uint32 flowDecision=CF_LAN_TO_LAN;
	rtk_rg_classifyEntry_t* pL2CFChkRule;
	uint32 is_ip4mc_check=0,is_ip6mc_check=0;
	int netifIdx=FAIL; //egress intf


	//reset the egressHitMask & final actions, because in broacast dataPath each Interfcaes will call this API individually

	for(i=0;i<((MAX_ACL_SW_ENTRY_SIZE/32)+1);i++){
		pPktHdr->aclDecision.aclEgrHitMask[i]=0;
	}

	for(i=0;i<((TOTAL_CF_ENTRY_SIZE/32)+1);i++){
		pPktHdr->aclDecision.aclEgrPattern1HitMask[i]=0;
	}

	_rtk_rg_aclDecisionClear(pPktHdr);


	//get original info
	sipModify = pPktHdr->ipv4Sip;
	dipModify = pPktHdr->ipv4Dip;
	sportModify = pPktHdr->sport;
	dportModify = pPktHdr->dport;

	//normal fwd L34, routing or bridge should be already decided by pPktHdr->pDmac & pPktHdr->pSmac
	memcpy(dmacModify, pPktHdr->pDmac,ETHER_ADDR_LEN);
	memcpy(smacModify, pPktHdr->pSmac,ETHER_ADDR_LEN);


	//20161020Chuck: the L34 modify information can be directly get from pPktHdr now.
	if(direct==RG_FWD_DECISION_NAPT)
	{
		// fill DA
		memcpy(dmacModify,rg_db.lut[pPktHdr->dmacL2Idx].rtk_lut.entry.l2UcEntry.mac.octet,ETHER_ADDR_LEN);

		//fill SA
		if(pPktHdr->netifIdx!=FAIL)
			memcpy(smacModify,rg_db.netif[pPktHdr->netifIdx].rtk_netif.gateway_mac.octet,ETHER_ADDR_LEN);
		else
			WARNING("[ERROR] invalid netifIdx=%d so fail to get egress smac!!",pPktHdr->netifIdx);

		//fill SIP
		if(l3Modify) sipModify=rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr;

		//fill SPORT
		if(l4Modify) sportModify=rg_db.naptOut[naptIdx].extPort;
	}
	else if(direct==RG_FWD_DECISION_NAPTR)
	{
		// fill DA
		memcpy(dmacModify,rg_db.lut[pPktHdr->dmacL2Idx].rtk_lut.entry.l2UcEntry.mac.octet,ETHER_ADDR_LEN);

		//fill SA
		if(pPktHdr->netifIdx!=FAIL)
			memcpy(smacModify,rg_db.netif[pPktHdr->netifIdx].rtk_netif.gateway_mac.octet,ETHER_ADDR_LEN);
		else
			WARNING("[ERROR] invalid netifIdx=%d so fail to get egress smac!!",pPktHdr->netifIdx);

		//fill DIP
		if(l3Modify) dipModify = ntohl(*pPktHdr->pIpv4Dip); //the packet content is changed for _rtk_rg_routingDecisionTablesLookup() temprorately

		//fill DPORT
		if(l4Modify) dportModify=rg_db.naptIn[naptIdx].rtk_naptIn.intPort;
	}
	else if (direct==RG_FWD_DECISION_V6ROUTING)
	{
		// fill DA
		memcpy(dmacModify,rg_db.lut[pPktHdr->dmacL2Idx].rtk_lut.entry.l2UcEntry.mac.octet,ETHER_ADDR_LEN);

		//fill SA
		if(pPktHdr->netifIdx!=FAIL)
			memcpy(smacModify,rg_db.netif[pPktHdr->netifIdx].rtk_netif.gateway_mac.octet,ETHER_ADDR_LEN);
		else
			WARNING("[ERROR] invalid netifIdx=%d so fail to get egress smac!!",pPktHdr->netifIdx);
	}


	if(pPktHdr->netifIdx==FAIL){ //Wan can not decide, consider as bridge followed apollo series. (apolloPro usually can have egress interface index even normal bridge.)
		netifIdx = 0;
	}else{
		netifIdx = pPktHdr->netifIdx;
	}


	ACL("parameter check: direct=%d naptIdx=%d l3Modify=%d l4Modify=%d netifIdx=%d fwdDecision=%d",direct,naptIdx,l3Modify,l4Modify,pPktHdr->netifIdx,pPktHdr->fwdDecision);
	ACL("decision check: netifIdx=%d, sipModify=0x%x, dipModify=0x%x, sportModify=%d, dportModify=%d \n dmacModify=%02x:%02x:%02x:%02x:%02x:%02x  smacModify=%02x:%02x:%02x:%02x:%02x:%02x",
	netifIdx,sipModify,dipModify,sportModify,dportModify,
	dmacModify[0],dmacModify[1],dmacModify[2],dmacModify[3],dmacModify[4],dmacModify[5],
	smacModify[0],smacModify[1],smacModify[2],smacModify[3],smacModify[4],smacModify[5]);

	if(egressPort == RG_ACL_EGRESS_BYPASS_PORT_ESPECIAL_TO_PS_CHECK){
		//bypass direction checking
	}
	else
	{
		//flow decision
		if(RTK_RG_IS_WAN_PORT(pPktHdr->ingressPort)) //downstream
		{
			flowDecision = CF_DOWNSTREAM;
			ACL("flowDecision: CF_DOWNSTREAM");
		}
		else //maybe upstream,
		{
			if(RTK_RG_IS_WAN_PORT(egressPort))//upstream
			{
				flowDecision = CF_UPSTREAM;
				ACL("flowDecision: CF_UPSTREAM");
			}
			else //lan to lan
			{
				flowDecision= CF_LAN_TO_LAN;
				ACL("flowDecision: CF_LAN_TO_LAN");
			}
		}
	}

	/*check CF pattern0*/
	for(index=0;index<MAX_ACL_SW_ENTRY_SIZE;index++){
		i = pPktHdr->aclDecision.aclEgrHaveToCheckRuleIdx[index];
		if(i==-1) { //no rest rule need to check
			if(round_check == ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET) {
				/*CF (fwdtype=1~4) pattern should reference for the result after ACL (fwdtype=0) modify */
				assert_ok(_rtk_rg_egressACLAction_beforeCFcheck(pPktHdr));
				/*check for fwd_type=0 finish, then check for fwd_type!=0 */
				round_check++;
				index = -1;	//next round will index++ so it will check rule idx 0
				continue;
			}
			else
				break;
		}

		pChkRule = &(rg_db.systemGlobal.acl_SW_table_entry[i]);

		/*CF (fwdtype=1~4) pattern should reference for the result after ACL (fwdtype=0) modify, so always check fwdtype=0 first */
		if(round_check == ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET) {	//this round only check fwd_type=0
			if(pChkRule->acl_filter.fwding_type_and_direction != ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
				continue;
		}
		else {		//this round only check fwd_type!=0
			if(pChkRule->acl_filter.fwding_type_and_direction == ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
				continue;
		}

		ACL("Check egress part of RG_ACL[%d]:",i);


		if(egressPort == RG_ACL_EGRESS_BYPASS_PORT_ESPECIAL_TO_PS_CHECK){
			//bypass direction checking


			//just chck PERMIT/TRAP_TO_PS rules for saving time.
			if((pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_PERMIT || pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_TRAP_TO_PS))
			{
				//go downstair for checking.
			}
			else
			{
				//just check trap_to_PS or Permit, others rule bypass
				continue;
			}
		}
		else
		{

			//direction check
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP ||
				pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN ||
				pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP ||
				pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT)
			{
				if(flowDecision != CF_UPSTREAM){
					ACL("RG_ACL[%d] fwding_type_and_direction UNHIT",i);
					continue;
				}
			}
			else if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP ||
				pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN ||
				pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP ||
				pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_PERMIT)
			{
				if(flowDecision != CF_DOWNSTREAM){
					ACL("RG_ACL[%d] fwding_type_and_direction UNHIT",i);
					continue;
				}

			}
			else
			{
				//lan-to-lan don't care, it can support all kind of pattern in apolloPro.
			}
		}


		if(pChkRule->acl_filter.filter_fields&EGRESS_INTF_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.egress_intf_idx!=netifIdx) //from PS, it's special pure software data path, the interface is determind as elder version which from pPktHdr->netifIdx
			{
				//ACL("pChkRule->acl_filter.egress_intf_idx=%d, netifIdx=%d, pPktHdr->netifIdx=%d",pChkRule->acl_filter.egress_intf_idx,netifIdx,pPktHdr->netifIdx);
				//ACL("RG_ACL[%d] EGRESS_INTF_BIT UNHIT ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_INTF_BIT)?1:0, possitive_check, "RG_ACL[%d] EGRESS_INTF_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&EGRESS_IPV4_SIP_RANGE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if( !((pChkRule->acl_filter.egress_src_ipv4_addr_end >= sipModify ) &&
				(pChkRule->acl_filter.egress_src_ipv4_addr_start<= sipModify ))){
				//DEBUG(" sipModify=0x%x",sipModify);
				//ACL("EGRESS_src_ipv4_addr_start=0x%x egress_src_ipv4_addr_end=0x%x",pChkRule->acl_filter.egress_src_ipv4_addr_start,pChkRule->acl_filter.egress_src_ipv4_addr_end);
				//ACL("RG_ACL[%d] EGRESS_IPV4_SIP_RANGE UNHIT ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_IPV4_SIP_RANGE_BIT)?1:0, possitive_check, "RG_ACL[%d] EGRESS_IPV4_SIP_RANGE_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&EGRESS_IPV4_DIP_RANGE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if( !((pChkRule->acl_filter.egress_dest_ipv4_addr_end >= dipModify ) &&
				(pChkRule->acl_filter.egress_dest_ipv4_addr_start<= dipModify))){
				//DEBUG(" dipModify=0x%x",dipModify);
				//ACL("EGRESS_dest_ipv4_addr_start=0x%x egress_dest_ipv4_addr_end=0x%x",pChkRule->acl_filter.egress_dest_ipv4_addr_start,pChkRule->acl_filter.egress_dest_ipv4_addr_end);
				//ACL("RG_ACL[%d] EGRESS_IPV4_DIP_RANGE UNHIT ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_IPV4_DIP_RANGE_BIT)?1:0, possitive_check, "RG_ACL[%d] EGRESS_IPV4_DIP_RANGE_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&EGRESS_L4_SPORT_RANGE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if( !((pChkRule->acl_filter.egress_src_l4_port_end >= sportModify ) &&
				(pChkRule->acl_filter.egress_src_l4_port_start<= sportModify ))){
				//ACL("RG_ACL[%d] EGRESS_L4_SPORT_RANGE UNHIT ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_L4_SPORT_RANGE_BIT)?1:0, possitive_check, "RG_ACL[%d] EGRESS_L4_SPORT_RANGE_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&EGRESS_L4_DPORT_RANGE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if( !((pChkRule->acl_filter.egress_dest_l4_port_end >= dportModify ) &&
				(pChkRule->acl_filter.egress_dest_l4_port_start<= dportModify))){
				//ACL("RG_ACL[%d] EGRESS_L4_DPORT_RANGE UNHIT ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_L4_DPORT_RANGE_BIT)?1:0, possitive_check, "RG_ACL[%d] EGRESS_L4_DPORT_RANGE_BIT UNHIT");
		}

		if(egressPort == RG_ACL_EGRESS_BYPASS_PORT_ESPECIAL_TO_PS_CHECK){
			if(pChkRule->acl_filter.filter_fields&EGRESS_CTAG_VID_BIT){
				//bypass EGRESS_CTAG_VID_BIT checking due to not support this pattern
				WARNING("not support EGRESS_CTAG_VID_BIT for egress trap_to_PS/permit ");
			}
		}
		else
		{
			if(pChkRule->acl_filter.filter_fields&EGRESS_CTAG_VID_BIT){//L34uptream only
				PATTERN_CHECK_INIT(possitive_check);

				//this pattern must egress with Ctag
				if(pPktHdr->egressVlanTagif==0){
					//ACL("RG_ACL[%d] EGRESS_CTAG_VID_BIT UNHIT  packet without Ctag ",i);
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}

				if((pChkRule->acl_filter.egress_ctag_vid & pChkRule->acl_filter.egress_ctag_vid_mask)!= (pPktHdr->internalVlanID & pChkRule->acl_filter.egress_ctag_vid_mask)){
					//ACL("RG_ACL[%d] EGRESS_CTAG_VID_BIT UNHIT  pChkRule->egress_ctag_vid=%d pPktHdr->egressVlanID=%d, pChkRule->acl_filter.egress_ctag_vid_mask=0x%x",i,pChkRule->acl_filter.egress_ctag_vid,pPktHdr->internalVlanID,pChkRule->acl_filter.egress_ctag_vid_mask);
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}

				PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_CTAG_VID_BIT)?1:0, possitive_check, "RG_ACL[%d] EGRESS_CTAG_VID_BIT UNHIT");
			}
		}

		if(egressPort == RG_ACL_EGRESS_BYPASS_PORT_ESPECIAL_TO_PS_CHECK){
			if(pChkRule->acl_filter.filter_fields&EGRESS_CTAG_PRI_BIT){
				//bypass EGRESS_CTAG_PRI_BIT checking due to not support this pattern
				WARNING("not support EGRESS_CTAG_PRI_BIT for egress trap_to_PS/permit ");
			}
		}
		else
		{
			if(pChkRule->acl_filter.filter_fields&EGRESS_CTAG_PRI_BIT){//L34uptream only
				PATTERN_CHECK_INIT(possitive_check);
				//this pattern must egress with Ctag
				if(pPktHdr->egressVlanTagif==0){
					//ACL("RG_ACL[%d] EGRESS_CTAG_PRI_BIT UNHIT  packet without Ctag ",i);
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}

				//[FIXME] broadcast datapath may not include ACL and Qos 1p-remarking decision in pPktHdr->egressPriority
				if(pChkRule->acl_filter.egress_ctag_pri!=pPktHdr->egressPriority)
				{
					//ACL("RG_ACL[%d] EGRESS_CTAG_PRI_BIT UNHIT ",i);
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}

				PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_CTAG_PRI_BIT)?1:0, possitive_check, "RG_ACL[%d] EGRESS_CTAG_PRI_BIT UNHIT");
			}
		}


		if(egressPort == RG_ACL_EGRESS_BYPASS_PORT_ESPECIAL_TO_PS_CHECK){
			if(pChkRule->acl_filter.filter_fields&INTERNAL_PRI_BIT){
				//bypass INTERNAL_PRI_BIT checking due to not support this pattern
				WARNING("not support INTERNAL_PRI_BIT for egress trap_to_PS/permit ");
			}
		}
		else
		{
			if(pChkRule->acl_filter.filter_fields&INTERNAL_PRI_BIT){
				PATTERN_CHECK_INIT(possitive_check);
				if(pChkRule->acl_filter.internal_pri!=pPktHdr->internalPriority){
					//ACL("RG_ACL[%d] INTERNAL_PRI_BIT UNHIT ",i);
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}

				PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&INTERNAL_PRI_BIT)?1:0, possitive_check, "RG_ACL[%d] INTERNAL_PRI_BIT UNHIT");
			}
		}


		if(egressPort == RG_ACL_EGRESS_BYPASS_PORT_ESPECIAL_TO_PS_CHECK){
			if(pChkRule->acl_filter.filter_fields&INGRESS_EGRESS_PORTIDX_BIT){
				//bypass INGRESS_EGRESS_PORTIDX_BIT checking due to not support this pattern
				WARNING("not support INGRESS_EGRESS_PORTIDX_BIT for egress trap_to_PS/permit ");
			}
		}
		else
		{

			if(pChkRule->acl_filter.filter_fields&INGRESS_EGRESS_PORTIDX_BIT){//uni pattern

				PATTERN_CHECK_INIT(possitive_check);

				//[FIXME] dowstream will following L34 DA lookup result
				if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP ||pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN)
				{
					//CF upstream(to PON), check spa
					if( (pChkRule->acl_filter.ingress_port_idx & pChkRule->acl_filter.ingress_port_idx_mask)!=(pPktHdr->ingressMacPort & pChkRule->acl_filter.ingress_port_idx_mask)){
						//ACL("RG_ACL[%d] INGRESS_EGRESS_PORTIDX_BIT UNHIT ",i);
						possitive_check = ACL_PATTERN_UNHIT; //continue;
					}
				}
				else if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP ||pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN)
				{	//CF downstream(from PON),check da
					if( (pChkRule->acl_filter.egress_port_idx & pChkRule->acl_filter.egress_port_idx_mask)!=(pPktHdr->egressMacPort & pChkRule->acl_filter.egress_port_idx_mask)){
						//ACL("RG_ACL[%d] INGRESS_EGRESS_PORTIDX_BIT UNHIT ",i);
						possitive_check = ACL_PATTERN_UNHIT; //continue;
					}
				}
				else if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)	//pattern egress_port_idx support upstream/downstream only in flow base platform
				{
					if( (pChkRule->acl_filter.egress_port_idx & pChkRule->acl_filter.egress_port_idx_mask)!=(pPktHdr->egressMacPort & pChkRule->acl_filter.egress_port_idx_mask)){
						ACL("RG_ACL[%d] INGRESS_EGRESS_PORTIDX_BIT UNHIT , egress_port_idx=%d, egress_port_idx_mask=0x%x, egressMacPort=%d",i,pChkRule->acl_filter.egress_port_idx,pChkRule->acl_filter.egress_port_idx_mask,pPktHdr->egressMacPort);
						possitive_check = ACL_PATTERN_UNHIT; //continue;
					}
				}

				PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_EGRESS_PORTIDX_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_EGRESS_PORTIDX_BIT UNHIT");
			}
		}


		if(pChkRule->acl_filter.filter_fields&EGRESS_IP4MC_IF){
			PATTERN_CHECK_INIT(possitive_check);
			if((pPktHdr->pDmac[0]==0x01&& pPktHdr->pDmac[1]==0x00 && pPktHdr->pDmac[2]==0x5e) && (pPktHdr->tagif&IGMP_TAGIF)==0x0){
				is_ip4mc_check = 1; //IP4MC(not include IGMP)
			}

			if(pChkRule->acl_filter.egress_ip4mc_if!=is_ip4mc_check){
				//ACL("RG_ACL[%d] EGRESS_IP4MC_IF UNHIT ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_IP4MC_IF)?1:0, possitive_check, "RG_ACL[%d] EGRESS_IP4MC_IF UNHIT");
		}


		if(pChkRule->acl_filter.filter_fields&EGRESS_IP6MC_IF){
			PATTERN_CHECK_INIT(possitive_check);
			if((pPktHdr->pDmac[0]==0x33 && pPktHdr->pDmac[1]==0x33) && (pPktHdr->tagif&IPV6_MLD_TAGIF)==0x0){
				is_ip6mc_check = 1; //IP6MC(not include MLD)
			}

			if(pChkRule->acl_filter.egress_ip6mc_if!=is_ip6mc_check){
				//ACL("RG_ACL[%d] EGRESS_IP6MC_IF UNHIT ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_IP6MC_IF)?1:0, possitive_check, "RG_ACL[%d] EGRESS_IP6MC_IF UNHIT");
		}


		if(pChkRule->acl_filter.filter_fields&EGRESS_SMAC_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if((pChkRule->acl_filter.egress_smac.octet[0] )!=(smacModify[0]) ||
				(pChkRule->acl_filter.egress_smac.octet[1] )!=(smacModify[1])||
				(pChkRule->acl_filter.egress_smac.octet[2] )!=(smacModify[2])||
				(pChkRule->acl_filter.egress_smac.octet[3] )!=(smacModify[3])||
				(pChkRule->acl_filter.egress_smac.octet[4] )!=(smacModify[4])||
				(pChkRule->acl_filter.egress_smac.octet[5] )!=(smacModify[5])
				){

				//ACL("RG_ACL[%d] EGRESS_SMAC_BIT UNHIT ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_SMAC_BIT)?1:0, possitive_check, "RG_ACL[%d] EGRESS_SMAC_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&EGRESS_DMAC_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if((pChkRule->acl_filter.egress_dmac.octet[0] )!=(dmacModify[0]) ||
				(pChkRule->acl_filter.egress_dmac.octet[1] )!=(dmacModify[1])||
				(pChkRule->acl_filter.egress_dmac.octet[2] )!=(dmacModify[2])||
				(pChkRule->acl_filter.egress_dmac.octet[3] )!=(dmacModify[3])||
				(pChkRule->acl_filter.egress_dmac.octet[4] )!=(dmacModify[4])||
				(pChkRule->acl_filter.egress_dmac.octet[5] )!=(dmacModify[5])
				){

				//ACL("RG_ACL[%d] EGRESS_DMAC_BIT UNHIT ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_DMAC_BIT)?1:0, possitive_check, "RG_ACL[%d] EGRESS_DMAC_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&EGRESS_WLANDEV_BIT){
			PATTERN_CHECK_INIT(possitive_check);

			if(_rtk_rg_wlanDevFromDmacL2Idx_check(pChkRule->acl_filter.egress_wlanDevMask, pPktHdr) == ACL_PATTERN_UNHIT)
			{
				//ACL("RG_ACL[%d] EGRESS_WLANDEV_BIT UNHIT ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_WLANDEV_BIT)?1:0, possitive_check, "RG_ACL[%d] EGRESS_WLANDEV_BIT UNHIT");
		}


		//record CF hit rule
		ACL("RG_ACL[%d] Egress Part Hit!",i);
		pPktHdr->aclDecision.aclEgrHitMask[i>>5]|=(1<<(i&0x1f));

	}

	if(egressPort == RG_ACL_EGRESS_BYPASS_PORT_ESPECIAL_TO_PS_CHECK){
		//bypass pure CF rules
	}
	else
	{

		/*check pure CF module*/
		for(i=0;i<TOTAL_CF_ENTRY_SIZE;i++){

			//CF pattern1 rule is valid if index is none zero
			if(rg_db.systemGlobal.classify_SW_table_entry[i].index!=FAIL){
				pL2CFChkRule = &(rg_db.systemGlobal.classify_SW_table_entry[i]);
				ACL("Check CFRule[%d]:",i);
			}else{
				continue;
			}

			//check upstream/downstream
			if(RTK_RG_IS_WAN_PORT(pPktHdr->ingressPort)){
				if(pL2CFChkRule->direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM){
					//spa is PON, just need to consider DS rules.
					ACL("RG_CF[%d] DIRECTION UNHIT UPSTREAM ",i);
					continue;
				}
			}else{
				//consider drop
				if((pL2CFChkRule->us_action_field&CF_US_ACTION_FWD_BIT) && (pL2CFChkRule->action_fwd.fwdDecision==ACL_FWD_DROP||pL2CFChkRule->action_fwd.fwdDecision==ACL_FWD_TRAP_TO_CPU))
				{
					//let it continue check. The Drop action will alwaye be Execute(if hit) even packet is not related to CF port.
				}else{

					if(!RTK_RG_IS_WAN_PORT(egressPort))
					{	//make sure its upstream, must egress to CF port
						ACL("RG_CF[%d] DIRECTION UNHIT UPSTREAM egressPort=%d ",i,egressPort);
						continue;
					}

					if(pL2CFChkRule->direction==RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM){
						//spa is none PON, just need to consider US rules.
						ACL("RG_CF[%d] DIRECTION UNHIT DOWNSTREAM ",i);
						continue;
					}
				}
			}

			if(pL2CFChkRule->filter_fields & EGRESS_ETHERTYPR_BIT){ //support mask
				if((pL2CFChkRule->etherType & pL2CFChkRule->etherType_mask)!= (pPktHdr->etherType& pL2CFChkRule->etherType_mask)){
					ACL("RG_CF[%d] EGRESS_ETHERTYPR_BIT UNHIT ",i);
					continue;
				}
			}

			if(pL2CFChkRule->filter_fields & EGRESS_GEMIDX_BIT){ //support mask
				/*where is the source? Rxdesc?*/
				if((pL2CFChkRule->gemidx & pL2CFChkRule->gemidx_mask)!= (pPktHdr->pRxDesc->rx_pon_stream_id & pL2CFChkRule->gemidx_mask)){
					ACL("RG_CF[%d] EGRESS_GEMIDX_BIT UNHIT ",i);
					continue;
				}
			}

			if(pL2CFChkRule->filter_fields & EGRESS_LLID_BIT){
				/*where is the source? Rxdesc?*/
				if(pL2CFChkRule->llid != pPktHdr->pRxDesc->rx_pon_stream_id){
					ACL("RG_CF[%d] EGRESS_LLID_BIT UNHIT ",i);
					continue;
				}
			}

			if(pL2CFChkRule->filter_fields & EGRESS_TAGVID_BIT){
				if(pPktHdr->tagif&SVLAN_TAGIF){
					if(pL2CFChkRule->outterTagVid != pPktHdr->stagVid){
						ACL("RG_CF[%d] EGRESS_TAGVID_BIT UNHIT ",i);
						continue;
					}
				}else if(pPktHdr->tagif&CVLAN_TAGIF){
					if(pL2CFChkRule->outterTagVid != pPktHdr->ctagVid){
						ACL("RG_CF[%d] EGRESS_TAGVID_BIT UNHIT ",i);
						continue;
					}
				}else{
					ACL("RG_CF[%d] EGRESS_TAGVID_BIT UNHIT ",i);
					continue;
				}
			}

			if(pL2CFChkRule->filter_fields & EGRESS_TAGPRI_BIT){
				if(pPktHdr->tagif&SVLAN_TAGIF){
					if(pL2CFChkRule->outterTagPri != pPktHdr->stagPri){
						ACL("RG_CF[%d] EGRESS_TAGPRI_BIT UNHIT ",i);
						continue;
					}
				}else if(pPktHdr->tagif&CVLAN_TAGIF){
					if(pL2CFChkRule->outterTagPri != pPktHdr->ctagPri){
						ACL("RG_CF[%d] EGRESS_TAGPRI_BIT UNHIT ",i);
						continue;
					}
				}else{
					ACL("RG_CF[%d] EGRESS_TAGPRI_BIT UNHIT ",i);
					continue;
				}
			}

			if(pL2CFChkRule->filter_fields & EGRESS_INTERNALPRI_BIT){
				if(pL2CFChkRule->internalPri != pPktHdr->internalPriority){
					ACL("RG_CF[%d] EGRESS_INTERNALPRI_BIT UNHIT ",i);
					continue;
				}
			}

			if(pL2CFChkRule->filter_fields & EGRESS_STAGIF_BIT){
				if((pL2CFChkRule->stagIf ==0 && (pPktHdr->tagif&SVLAN_TAGIF)) ||
					(pL2CFChkRule->stagIf ==1 && !(pPktHdr->tagif&SVLAN_TAGIF))){
					ACL("RG_CF[%d] EGRESS_STAGIF_BIT UNHIT ",i);
					continue;
				}
			}

			if(pL2CFChkRule->filter_fields & EGRESS_CTAGIF_BIT){
				if((pL2CFChkRule->ctagIf ==0 && (pPktHdr->tagif&CVLAN_TAGIF)) ||
					(pL2CFChkRule->ctagIf ==1 && !(pPktHdr->tagif&CVLAN_TAGIF))){
					ACL("RG_CF[%d] EGRESS_CTAGIF_BIT UNHIT ",i);
					continue;
				}
			}

			if(pL2CFChkRule->filter_fields & EGRESS_UNI_BIT){ //support mask
				//upstream
				if(pL2CFChkRule->direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM){
					if((pL2CFChkRule->uni & pL2CFChkRule->uni_mask)!= (pPktHdr->ingressMacPort & pL2CFChkRule->uni_mask)){
						ACL("RG_CF[%d] EGRESS_UNI_BIT UNHIT ",i);
						continue;
					}
				}
				//downstream : should decide by DMAC lookup
				if(pL2CFChkRule->direction==RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM){
					//TRACE("uni=%d uni_mask=%d egressPort=%d",pL2CFChkRule->uni,pL2CFChkRule->uni_mask,pPktHdr->egressMACPort);
					if((pL2CFChkRule->uni & pL2CFChkRule->uni_mask)!= (pPktHdr->egressMacPort & pL2CFChkRule->uni_mask)){
						ACL("RG_CF[%d] EGRESS_UNI_BIT UNHIT ",i);
						continue;
					}
				}
			}

			pPktHdr->aclDecision.direction = pL2CFChkRule->direction;
			pPktHdr->aclDecision.aclEgrPattern1HitMask[(i>>5)]|=(1<<(i&0x1f));
			ACL("RG_CF[%d] Hit!!!",i);

			break;
		}
	}
	return RT_ERR_RG_OK;

}



int _rtk_rg_egressACLAction(int direct, rtk_rg_pktHdr_t *pPktHdr)
{
	uint32 i,acl_idx;
	rtk_rg_aclFilterEntry_t* pChkRule;
	int permit_hit=0;

	uint32 pon_is_cfport;
	pon_is_cfport = rg_db.systemGlobal.pon_is_cfport;



	//TRACE("Egress ACL Action:");

	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++)
	{
		if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i]==-1)
			break;	//no more SW_ACL rules

		acl_idx = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i];
		if((pPktHdr->aclDecision.aclIgrHitMask[(acl_idx>>5)]&(1<<(acl_idx&0x1f))) && (pPktHdr->aclDecision.aclEgrHitMask[(acl_idx>>5)]&(1<<(acl_idx&0x1f)))){

			ACL("RG_ACL[%d] Hit!!!",acl_idx);
			DEBUG("pPktHdr->aclDecision.aclIgrDoneAction = 0x%x, aclEgrDoneAction = 0x%x",pPktHdr->aclDecision.aclIgrDoneAction,pPktHdr->aclDecision.aclEgrDoneAction);


			pChkRule = &(rg_db.systemGlobal.acl_SW_table_entry[acl_idx]);

			if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_DROP)
			{

				if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
				{
					if(pPktHdr->aclDecision.aclIgrDoneAction & RG_IGR_FORWARD_ACT_DONE_BIT)//permit is active
					{
						//do nothing because permit
					}
					else
					{
						TRACE("[Drop] RG_ACL[%d] DROP!",acl_idx);
						return RG_FWDENGINE_RET_DROP;
					}
				}
				else if((pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP || pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP))
				{
					if(pPktHdr->aclDecision.aclEgrDoneAction & RG_EGR_FWD_ACT_DONE_BIT)//permit is active
					{
						//do nothing because permit
					}
					else
					{
						TRACE("[Drop] RG_ACL[%d] DROP!",acl_idx);
						return RG_FWDENGINE_RET_DROP;
					}

				}

			}
			else if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_SW_DROP)
			{

				if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
				{
					if(pPktHdr->aclDecision.aclIgrDoneAction & RG_IGR_FORWARD_ACT_DONE_BIT)//permit is active
					{
						//do nothing because permit
					}
					else
					{
						TRACE("[Drop] RG_ACL[%d] SW DROP!",acl_idx);
						return RG_FWDENGINE_RET_DROP;
					}
				}
				else if((pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP || pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP))
				{
					if(pPktHdr->aclDecision.aclEgrDoneAction & RG_EGR_FWD_ACT_DONE_BIT)//permit is active
					{
						//do nothing because permit
					}
					else
					{
						TRACE("[Drop] RG_ACL[%d] SW DROP!",acl_idx);
						return RG_FWDENGINE_RET_DROP;
					}

				}

			}
			else if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_TRAP_TO_PS)
			{

				if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
				{
					if((pPktHdr->aclDecision.aclIgrDoneAction&RG_IGR_FORWARD_ACT_DONE_BIT)==0x0){
						pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_FORWARD_ACT_DONE_BIT;

						TRACE("[To PS] ACL[%d] ACT:TRAP to PS",acl_idx);
						pPktHdr->byPassToPsVlanAclDecision = 1;
						return RG_FWDENGINE_RET_TO_PS;

					}
				}
				else if((pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP || pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP))
				{
					if((pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_FWD_ACT_DONE_BIT)==0x0){
						pPktHdr->aclDecision.aclEgrDoneAction |= RG_EGR_FWD_ACT_DONE_BIT;

						TRACE("[To PS] ACL[%d] ACT:TRAP to PS",acl_idx);
						pPktHdr->byPassToPsVlanAclDecision = 1;
						return RG_FWDENGINE_RET_TO_PS;

					}
				}
			}
			else if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_TRAP)
			{
				if(permit_hit==0){
					if(pPktHdr->addPureL2Flow && rg_db.systemGlobal.acl_SW_table_entry[acl_idx].hw_aclEntry_count)
					{
						TRACE("[Trap] ACL[%d] packet add to hw/sw flow...path1 and include hw acl",acl_idx);
					}
					else
					{
						pPktHdr->swFlowOnlyByAcl = 1;
						TRACE("[Trap] ACL[%d] packet only add to sw flow",acl_idx);
					}
				}else{
					TRACE("[Permit] ACL[%d] bypass because already hit PERMIT rule",acl_idx);
				}
			}
			else if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_TRAP_WITH_PRIORITY)
			{
				if(permit_hit==0){
					pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_FORWARD_ACT_DONE_BIT;
					TRACE("[Trap] ACL[%d] TRAP with priority by HW",acl_idx);
				}else{
					TRACE("[Permit] ACL[%d] bypass because already hit PERMIT rule",acl_idx);
				}
			}
			else if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_PERMIT)
			{
				permit_hit = 1;
				if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
				{
					pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_FORWARD_ACT_DONE_BIT;
					TRACE("RG_ACL[%d] PERMIT!",acl_idx);

				}
				else if((pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT || pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_PERMIT))
				{
					pPktHdr->aclDecision.aclEgrDoneAction |= RG_EGR_FWD_ACT_DONE_BIT;
					TRACE("RG_ACL[%d] PERMIT!",acl_idx);

				}

			}
			else if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_SW_MIRROR_WITH_UDP_ENCAP)
			{	//this feature support two cases: mirror ingress packet if ACL pattern not include egress pattern; mirror egress packet if ACL pattern include egress pattern
				if((pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_MIRROR_UDP_ENCAP_ACT_DONE_BIT)==0x0 && (pChkRule->acl_filter.filter_fields&EGRESS_ACL_PATTERN_BITS) && ((pPktHdr->tagif & IPV4_TAGIF) || (pPktHdr->tagif & IPV6_TAGIF)) /*support ipv4 and ipv6*/){
					pPktHdr->aclDecision.aclEgrDoneAction |= RG_EGR_MIRROR_UDP_ENCAP_ACT_DONE_BIT;
					pPktHdr->aclDecision.action_igr_encap_udp=pChkRule->acl_filter.action_encap_udp;
					pPktHdr->aclDecision.egr_encap_acl_idx=acl_idx;
					TRACE("RG_ACL[%d] MIRROR and UDP_ENCAPSULATION!",acl_idx);
				}
			}
			else if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_FLOW_MIB)
			{
				if((pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_FLOWMIB_ACT_DONE_BIT)==0x0){
					pPktHdr->aclDecision.aclEgrDoneAction |= RG_EGR_FLOWMIB_ACT_DONE_BIT;
					pPktHdr->aclDecision.action_flowmib_counter_idx = pChkRule->acl_filter.action_flowmib_counter_idx;
					pPktHdr->flowMIBCounterIdx = pChkRule->acl_filter.action_flowmib_counter_idx;	// for shortcut/flow entry update
					TRACE("RG_ACL[%d] Accumulate Flow counters to FLOWMIB idx %d!", acl_idx, pPktHdr->aclDecision.action_flowmib_counter_idx);

					if(pPktHdr->aclDecision.action_flowmib_counter_idx >= PURE_SW_FLOWMIB_IDX_OFFSET) {
						pPktHdr->swFlowOnlyByAcl = 1;
						TRACE("[QoS] ACL[%d] packet only add to sw flow due to hit sw flow mib(idx >= %d)",acl_idx, PURE_SW_FLOWMIB_IDX_OFFSET);
					}
				}
			}
			else //Qos Type
			{
				pPktHdr->aclDecision.action_type = ACL_ACTION_TYPE_QOS;

				/*chuck:
				fwdtype=0 (ACL related rule, action control bit record in pPktHdr->aclDecision.aclIgrDoneAction )
				fwdtype=1~4 (CF related rule, action control bit record in pPktHdr->aclDecision.aclEgrDoneAction )
				  */

				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_1P_REMARKING_BIT){//belong to ACL action

					if((pPktHdr->aclDecision.aclIgrDoneAction&RG_IGR_CVLAN_ACT_DONE_BIT)==0x0) //ACL RG_IGR_CVLAN_ACT_DONE_BIT have not done
					{
							pPktHdr->aclDecision.qos_actions |= ACL_ACTION_1P_REMARKING_BIT;
							pPktHdr->aclDecision.action_dot1p_remarking_pri = pChkRule->acl_filter.action_dot1p_remarking_pri;
							pPktHdr->aclDecision.aclIgrDoneAction |=RG_IGR_CVLAN_ACT_DONE_BIT;
							ACL("RG_ACL[%d] do 1P_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_dot1p_remarking_pri);
							TRACE("RG_ACL[%d] do 1P_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_dot1p_remarking_pri);
					}
					else
					{
						ACL("RG_ACL[%d] skip 1P_REMARKING to %d, because IGR_CVLAN_ACT is already done!", acl_idx, pChkRule->acl_filter.action_dot1p_remarking_pri);
					}
				}

				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT){//belong to ACL action or CF action

					if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)//ACL rule
					{
						if((pPktHdr->aclDecision.aclIgrDoneAction&RG_IGR_SVLAN_ACT_DONE_BIT)==0x0) //ACL RG_IGR_SVLAN_ACT_DONE_BIT have not done
						{
							pPktHdr->aclDecision.qos_actions |= ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT;
							pPktHdr->aclDecision.action_ip_precedence_remarking_pri= pChkRule->acl_filter.action_ip_precedence_remarking_pri;
							pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_SVLAN_ACT_DONE_BIT;
							ACL("RG_ACL[%d] do IP_PRECEDENCE_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_ip_precedence_remarking_pri);
							TRACE("RG_ACL[%d] do IP_PRECEDENCE_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_ip_precedence_remarking_pri);
						}
						else
						{
							ACL("RG_ACL[%d] skip IP_PRECEDENCE_REMARKING to %d, because IGR_SVLAN_ACT is already done!", acl_idx, pChkRule->acl_filter.action_ip_precedence_remarking_pri);
						}
					}
					else//CF rule(fwdtype=1~4)
					{
						if((pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_DSCP_ACT_DONE_BIT)==0x0) //CF RG_EGR_DSCP_ACT_DONE_BIT have not done
						{
							pPktHdr->aclDecision.qos_actions |= ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT;
							pPktHdr->aclDecision.action_ip_precedence_remarking_pri= pChkRule->acl_filter.action_ip_precedence_remarking_pri;
							pPktHdr->aclDecision.aclEgrDoneAction |= RG_EGR_DSCP_ACT_DONE_BIT;
							ACL("RG_ACL[%d] do IP_PRECEDENCE_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_ip_precedence_remarking_pri);
							TRACE("RG_ACL[%d] do IP_PRECEDENCE_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_ip_precedence_remarking_pri);
						}
						else
						{
							ACL("RG_ACL[%d] skip IP_PRECEDENCE_REMARKING to %d, because EGR_DSCP_ACT is already done!", acl_idx, pChkRule->acl_filter.action_ip_precedence_remarking_pri);
						}
					}

				}


				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_DSCP_REMARKING_BIT){//belong to ACL action or CF action

					if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)//ACL rule
					{
						if((pPktHdr->aclDecision.aclIgrDoneAction&RG_IGR_SVLAN_ACT_DONE_BIT)==0x0) //ACL RG_IGR_SVLAN_ACT_DONE_BIT have not done
						{
							pPktHdr->aclDecision.qos_actions |= ACL_ACTION_DSCP_REMARKING_BIT;
							pPktHdr->aclDecision.action_dscp_remarking_pri= pChkRule->acl_filter.action_dscp_remarking_pri;
							pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_SVLAN_ACT_DONE_BIT;
							ACL("RG_ACL[%d] do DSCP_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_dscp_remarking_pri);
							TRACE("RG_ACL[%d] do DSCP_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_dscp_remarking_pri);
						}
						else
						{
							ACL("RG_ACL[%d] skip DSCP_REMARKING to %d, because IGR_SVLAN_ACT is already done!", acl_idx, pChkRule->acl_filter.action_dscp_remarking_pri);
						}
					}
					else//CF rule(fwdtype=1~4)
					{
						if((pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_DSCP_ACT_DONE_BIT)==0x0) //CF RG_EGR_DSCP_ACT_DONE_BIT have not done
						{
							pPktHdr->aclDecision.qos_actions |= ACL_ACTION_DSCP_REMARKING_BIT;
							pPktHdr->aclDecision.action_dscp_remarking_pri= pChkRule->acl_filter.action_dscp_remarking_pri;
							pPktHdr->aclDecision.aclEgrDoneAction |= RG_EGR_DSCP_ACT_DONE_BIT;
							ACL("RG_ACL[%d] do DSCP_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_dscp_remarking_pri);
							TRACE("RG_ACL[%d] do DSCP_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_dscp_remarking_pri);
						}
						else
						{
							ACL("RG_ACL[%d] skip DSCP_REMARKING to %d, because CF[0-64] is already done!", acl_idx, pChkRule->acl_filter.action_dscp_remarking_pri);
						}
					}

				}

				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_TOS_TC_REMARKING_BIT){//belong to ACL action or CF action
					//HW belong to ACL_ACTION_ACL_PRIORITY_BIT, but it will not happen HW PRIACT 0x0(acl trap with priority) and 0x6(tc_tos remarking), so SW make this action same effect as DSCP
					if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)//ACL rule
					{
						if((pPktHdr->aclDecision.aclIgrDoneAction&RG_IGR_SVLAN_ACT_DONE_BIT)==0x0) //ACL RG_IGR_SVLAN_ACT_DONE_BIT have not done
						{
							pPktHdr->aclDecision.qos_actions |= ACL_ACTION_TOS_TC_REMARKING_BIT;
							pPktHdr->aclDecision.action_tos_tc_remarking_pri= pChkRule->acl_filter.action_tos_tc_remarking_pri;
							pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_SVLAN_ACT_DONE_BIT;
							ACL("RG_ACL[%d] do TOS_TC_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_tos_tc_remarking_pri);
							TRACE("RG_ACL[%d] do TOS_TC_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_tos_tc_remarking_pri);
							/* G3 HW ACL not support ECN remarking by ACL, ECN will only udpate by TE */
							pPktHdr->swFlowOnlyByAcl = 1;
							TRACE("[QoS] ACL[%d] packet only add to sw flow",acl_idx);
						}
						else
						{
							ACL("RG_ACL[%d] skip TOS_TC_REMARKING to %d, because IGR_SVLAN_ACT is already done!", acl_idx, pChkRule->acl_filter.action_tos_tc_remarking_pri);
						}
					}
					else//CF rule(fwdtype=1~4)
					{
						if((pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_DSCP_ACT_DONE_BIT)==0x0) //CF RG_EGR_DSCP_ACT_DONE_BIT have not done
						{
							pPktHdr->aclDecision.qos_actions |= ACL_ACTION_TOS_TC_REMARKING_BIT;
							pPktHdr->aclDecision.action_tos_tc_remarking_pri= pChkRule->acl_filter.action_tos_tc_remarking_pri;
							pPktHdr->aclDecision.aclEgrDoneAction |= RG_EGR_DSCP_ACT_DONE_BIT;
							ACL("RG_ACL[%d] do TOS_TC_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_tos_tc_remarking_pri);
							TRACE("RG_ACL[%d] do TOS_TC_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_tos_tc_remarking_pri);
							/* G3 HW ACL not support ECN remarking by ACL, ECN will only udpate by TE */
							pPktHdr->swFlowOnlyByAcl = 1;
							TRACE("[QoS] ACL[%d] packet only add to sw flow",acl_idx);
						}
						else
						{
							ACL("RG_ACL[%d] skip TOS_TC_REMARKING to %d, because CF[0-64](EGR_DSCP_ACT) is already done!", acl_idx, pChkRule->acl_filter.action_tos_tc_remarking_pri);
						}
					}

				}

				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_QUEUE_ID_BIT){
					ACL("ACL_ACTION_QUEUE_ID_BIT is not supported in apolloPro!");
				}

				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_SHARE_METER_BIT){
					if((pPktHdr->aclDecision.aclIgrDoneAction&RG_IGR_POLICE_ACT_DONE_BIT)==0x0){//RG_IGR_POLICE_ACT_DONE_BIT have not been set
						pPktHdr->aclDecision.qos_actions |= ACL_ACTION_SHARE_METER_BIT;
						pPktHdr->aclDecision.action_share_meter= pChkRule->acl_filter.action_share_meter;
						pPktHdr->aclDecision.aclIgrDoneAction |=RG_IGR_POLICE_ACT_DONE_BIT;
						ACL("RG_ACL[%d] do ACL_ACTION_SHARE_METER to %d", acl_idx, pPktHdr->aclDecision.action_share_meter);
						TRACE("RG_ACL[%d] do ACL_ACTION_SHARE_METER to %d", acl_idx, pPktHdr->aclDecision.action_share_meter);

						if(PURE_SW_METER_IDX_OFFSET<=pPktHdr->aclDecision.action_share_meter && pPktHdr->aclDecision.action_share_meter<(PURE_SW_METER_IDX_OFFSET+PURE_SW_SHAREMETER_TABLE_SIZE)){
							pPktHdr->swFlowOnlyByAcl = 1;
							pPktHdr->swMeterOffsetIdx = pPktHdr->aclDecision.action_share_meter - PURE_SW_METER_IDX_OFFSET;
							TRACE("[QoS] ACL[%d] packet only add to sw flow due to hit sw share meter[%d](idx >= %d)", acl_idx, pPktHdr->aclDecision.action_share_meter, PURE_SW_METER_IDX_OFFSET);
						}
					}
					else
					{
						ACL("RG_ACL[%d] skip ACL_ACTION_SHARE_METER to %d, because IGR_POLICE_ACT is already done!", acl_idx, pChkRule->acl_filter.action_share_meter);
					}
				}

				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_LOG_COUNTER_BIT){
					ACL("ACL_ACTION_LOG_COUNTER_BIT is not supported in software!");
					//[FIXME]: need to development this feature to ApolloPro Asic setting by Jim
				}

				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_ACL_PRIORITY_BIT){
					//aclPriority is used for internalPriority decision, no need to use after this function anymore.
				}

				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_STREAM_ID_OR_LLID_BIT){//belong to CF action

					if((pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_SID_ACT_DONE_BIT)==0x0){//RG_EGR_SID_ACT_DONE_BIT have not been set
						pPktHdr->aclDecision.qos_actions |= ACL_ACTION_STREAM_ID_OR_LLID_BIT;
						pPktHdr->aclDecision.action_stream_id_or_llid= pChkRule->acl_filter.action_stream_id_or_llid;
						pPktHdr->aclDecision.aclEgrDoneAction |=RG_EGR_SID_ACT_DONE_BIT;
						ACL("RG_ACL[%d] do STREAM_ID_OR_LLID to %d", acl_idx, pPktHdr->aclDecision.action_stream_id_or_llid);
						TRACE("RG_ACL[%d] do STREAM_ID_OR_LLID to %d", acl_idx, pPktHdr->aclDecision.action_stream_id_or_llid);
					}
					else
					{
						ACL("RG_ACL[%d] skip STREAM_ID_OR_LLID to %d, because EGR_SID_ACT is already done!", acl_idx, pChkRule->acl_filter.action_stream_id_or_llid);
					}
				}

				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT){//belong to CF action
					if((pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_CFPRI_ACT_DONE_BIT)==0x0){//RG_EGR_CFPRI_ACT_DONE_BIT have not been set
						pPktHdr->aclDecision.qos_actions |= ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT;
						pPktHdr->aclDecision.action_acl_egress_internal_priority= pChkRule->acl_filter.egress_internal_priority;
						pPktHdr->aclDecision.aclEgrDoneAction |=RG_EGR_CFPRI_ACT_DONE_BIT;
						ACL("RG_ACL[%d] assign CFPRI to %d", acl_idx, pPktHdr->aclDecision.action_acl_egress_internal_priority);
						TRACE("RG_ACL[%d] assign CFPRI to %d", acl_idx, pPktHdr->aclDecision.action_acl_egress_internal_priority);
					}
					else
					{
						ACL("RG_ACL[%d] skip CFPRI to %d, because EGR_CFPRI_ACT is already done!", acl_idx, pChkRule->acl_filter.egress_internal_priority);
					}
				}

				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_REDIRECT_BIT){//belong to ACL action
#if 1
					ACL("ACL_ACTION_REDIRECT_BIT is not not support in apolloPro");
#else
					if((pPktHdr->aclDecision.aclIgrDoneAction&RG_IGR_FORWARD_ACT_DONE_BIT)==0x0){//RG_IGR_FORWARD_ACT_DONE_BIT have not been set
						pPktHdr->aclDecision.qos_actions |= ACL_ACTION_REDIRECT_BIT;
						pPktHdr->aclDecision.action_redirect_portmask= pChkRule->acl_filter.redirect_portmask;
						pPktHdr->aclDecision.aclIgrDoneAction|=RG_IGR_FORWARD_ACT_DONE_BIT;
						ACL("RG_ACL[%d] REDIRECT to portmask 0x%x", acl_idx, pPktHdr->aclDecision.action_redirect_portmask);
						TRACE("RG_ACL[%d] REDIRECT to portmask 0x%x", acl_idx, pPktHdr->aclDecision.action_redirect_portmask);
					}
					else
					{
						ACL("RG_ACL[%d] skip REDIRECT to portmask 0x%x, because IGR_FORWARD_ACT is already done!", acl_idx, pChkRule->acl_filter.redirect_portmask);
					}
#endif
				}

				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_ACL_CVLANTAG_BIT){//belong to CF action
					if((pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_CVLAN_ACT_DONE_BIT)==0x0){//RG_EGR_CVLAN_ACT_DONE_BIT have not been set

						pPktHdr->aclDecision.qos_actions|= ACL_ACTION_ACL_CVLANTAG_BIT;
						pPktHdr->aclDecision.action_acl_cvlan=pChkRule->acl_filter.action_acl_cvlan;
						pPktHdr->aclDecision.aclEgrDoneAction|=RG_EGR_CVLAN_ACT_DONE_BIT;

						ACL("RG_ACL[%d] do CVLANTAG to %s %s %s vid=%d pri=%d",
							acl_idx,
							name_of_rg_cvlan_tagif_decision[pPktHdr->aclDecision.action_acl_cvlan.cvlanTagIfDecision],
							name_of_rg_cvlan_cvid_decision[pPktHdr->aclDecision.action_acl_cvlan.cvlanCvidDecision],
							name_of_rg_cvlan_cpri_decision[pPktHdr->aclDecision.action_acl_cvlan.cvlanCpriDecision],
							pPktHdr->aclDecision.action_acl_cvlan.assignedCvid,
							pPktHdr->aclDecision.action_acl_cvlan.assignedCpri
							);
						TRACE("RG_ACL[%d] do CVLANTAG to %s %s %s vid=%d pri=%d",
							acl_idx,
							name_of_rg_cvlan_tagif_decision[pPktHdr->aclDecision.action_acl_cvlan.cvlanTagIfDecision],
							name_of_rg_cvlan_cvid_decision[pPktHdr->aclDecision.action_acl_cvlan.cvlanCvidDecision],
							name_of_rg_cvlan_cpri_decision[pPktHdr->aclDecision.action_acl_cvlan.cvlanCpriDecision],
							pPktHdr->aclDecision.action_acl_cvlan.assignedCvid,
							pPktHdr->aclDecision.action_acl_cvlan.assignedCpri
							);
					}
					else
					{
						ACL("RG_ACL[%d] skip CVLANTAG to %s %s %s vid=%d pri=%d, because EGR_CVLAN_ACT is already done!",
							acl_idx,
							name_of_rg_cvlan_tagif_decision[pChkRule->acl_filter.action_acl_cvlan.cvlanTagIfDecision],
							name_of_rg_cvlan_cvid_decision[pChkRule->acl_filter.action_acl_cvlan.cvlanCvidDecision],
							name_of_rg_cvlan_cpri_decision[pChkRule->acl_filter.action_acl_cvlan.cvlanCpriDecision],
							pChkRule->acl_filter.action_acl_cvlan.assignedCvid,
							pChkRule->acl_filter.action_acl_cvlan.assignedCpri
							);
					}
				}

				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_ACL_SVLANTAG_BIT){//belong CF action
					if((pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_SVLAN_ACT_DONE_BIT)==0x0){//RG_EGR_SVLAN_ACT_DONE_BIT have not been set

						pPktHdr->aclDecision.qos_actions|= ACL_ACTION_ACL_SVLANTAG_BIT;
						pPktHdr->aclDecision.action_acl_svlan=pChkRule->acl_filter.action_acl_svlan;
						pPktHdr->aclDecision.aclEgrDoneAction|=RG_EGR_SVLAN_ACT_DONE_BIT;


						ACL("RG_ACL[%d] do SVLANTAG to %s %s %s svid=%d spri=%d",
							acl_idx,
							name_of_rg_svlan_tagif_decision[pPktHdr->aclDecision.action_acl_svlan.svlanTagIfDecision],
							name_of_rg_svlan_svid_decision[pPktHdr->aclDecision.action_acl_svlan.svlanSvidDecision],
							name_of_rg_svlan_spri_decision[pPktHdr->aclDecision.action_acl_svlan.svlanSpriDecision],
							pPktHdr->aclDecision.action_acl_svlan.assignedSvid,
							pPktHdr->aclDecision.action_acl_svlan.assignedSpri
							);
						TRACE("RG_ACL[%d] do SVLANTAG to %s %s %s svid=%d spri=%d",
							acl_idx,
							name_of_rg_svlan_tagif_decision[pPktHdr->aclDecision.action_acl_svlan.svlanTagIfDecision],
							name_of_rg_svlan_svid_decision[pPktHdr->aclDecision.action_acl_svlan.svlanSvidDecision],
							name_of_rg_svlan_spri_decision[pPktHdr->aclDecision.action_acl_svlan.svlanSpriDecision],
							pPktHdr->aclDecision.action_acl_svlan.assignedSvid,
							pPktHdr->aclDecision.action_acl_svlan.assignedSpri
							);


					}
					else
					{
						ACL("RG_ACL[%d] skip SVLANTAG to %s %s %s svid=%d spri=%d, because EGR_SVLAN_ACT is already done!",
							acl_idx,
							name_of_rg_svlan_tagif_decision[pChkRule->acl_filter.action_acl_svlan.svlanTagIfDecision],
							name_of_rg_svlan_svid_decision[pChkRule->acl_filter.action_acl_svlan.svlanSvidDecision],
							name_of_rg_svlan_spri_decision[pChkRule->acl_filter.action_acl_svlan.svlanSpriDecision],
							pChkRule->acl_filter.action_acl_svlan.assignedSvid,
							pChkRule->acl_filter.action_acl_svlan.assignedSpri
							);

					}
				}

				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_DS_UNIMASK_BIT){ //belong to CF action
#if 0				//not support ACL_UNI_FORCE_BY_MASK, but ACL use ACL_UNI_FWD_TO_PORTMASK_ONLY
					ACL("ACL_ACTION_DS_UNIMASK_BIT is not not support in apolloPro");
#else
					if((pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_FWD_ACT_DONE_BIT)==0x0){//RG_EGR_FWD_ACT_DONE_BIT have not been set
						pPktHdr->aclDecision.action_acl_uni.uniActionDecision = ACL_UNI_FWD_TO_PORTMASK_ONLY;
						pPktHdr->aclDecision.action_acl_uni.assignedUniPortMask = pChkRule->acl_filter.downstream_uni_portmask;
						pPktHdr->aclDecision.qos_actions|=ACL_ACTION_DS_UNIMASK_BIT;
						pPktHdr->aclDecision.aclEgrDoneAction |=RG_EGR_FWD_ACT_DONE_BIT;
						ACL("RG_ACL[%d] do UNIPORT MASK=0x%x\n",acl_idx,pPktHdr->aclDecision.action_acl_uni.assignedUniPortMask);
						TRACE("RG_ACL[%d] do UNIPORT MASK=0x%x\n",acl_idx,pPktHdr->aclDecision.action_acl_uni.assignedUniPortMask);
					}
					else
					{
						ACL("RG_ACL[%d] skip UNIPORT MASK=0x%x, because EGR_FWD_ACT is already done!\n",acl_idx, pChkRule->acl_filter.downstream_uni_portmask);
					}
#endif
				}
			}

		}
	}

	//check CF pattern1.
	for(i=0;i<TOTAL_CF_ENTRY_SIZE;i++){
		//ACL(" Check CF[%d] actions",i);

		if(pPktHdr->aclDecision.aclEgrPattern1HitMask[i>>5]==0x0)
		{
			i+=31; //skip none valid rules for 32 rules per run.
			continue;
		}

		if((pPktHdr->aclDecision.aclEgrPattern1HitMask[i>>5]>>(((i>>3)&0x3)<<3))&0xff)
		{
			//there is valid rule need to check in these 8 rules
		}
		else
		{
			i+=7; //skip none valid rules for 8 rules per run.
			continue;
		}

		if(pPktHdr->aclDecision.aclEgrPattern1HitMask[(i>>5)]&(1<<(i&0x1f))){
			//ACL(" Do CF[%d] actions",i);
			//copy the rule action to final aclDecision
			//ACL("Copy CF[%d] to final action decision",i);

			pPktHdr->aclDecision.direction=rg_db.systemGlobal.classify_SW_table_entry[i].direction;

			if(rg_db.systemGlobal.classify_SW_table_entry[i].direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM) //upstream
			{
				//SVLAN ACT
				if(rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_STAG_BIT)
				{
					if((pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_SVLAN_ACT_DONE_BIT)==0x0)//RG_EGR_SVLAN_ACT_DONE_BIT have not been set
					{
						pPktHdr->aclDecision.us_action_field |= CF_US_ACTION_STAG_BIT;
						pPktHdr->aclDecision.aclEgrDoneAction|=RG_EGR_SVLAN_ACT_DONE_BIT;
						pPktHdr->aclDecision.action_svlan=rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan;

						ACL("RG_CF[%d] do VLAN %s  svid_act:%s	spri_act:%s  svid:%d  spri:%d",
							i,
							name_of_rg_svlan_tagif_decision[pPktHdr->aclDecision.action_svlan.svlanTagIfDecision],
							name_of_rg_svlan_svid_decision[pPktHdr->aclDecision.action_svlan.svlanSvidDecision],
							name_of_rg_svlan_spri_decision[pPktHdr->aclDecision.action_svlan.svlanSpriDecision],
							pPktHdr->aclDecision.action_svlan.assignedSvid,
							pPktHdr->aclDecision.action_svlan.assignedSpri
						);
						TRACE("RG_CF[%d] do VLAN %s  svid_act:%s  spri_act:%s  svid:%d	spri:%d",
							i,
							name_of_rg_svlan_tagif_decision[pPktHdr->aclDecision.action_svlan.svlanTagIfDecision],
							name_of_rg_svlan_svid_decision[pPktHdr->aclDecision.action_svlan.svlanSvidDecision],
							name_of_rg_svlan_spri_decision[pPktHdr->aclDecision.action_svlan.svlanSpriDecision],
							pPktHdr->aclDecision.action_svlan.assignedSvid,
							pPktHdr->aclDecision.action_svlan.assignedSpri
						);


					}
					else
					{
						ACL("RG_CF[%d] skip SVLAN , because EGR_SVLAN_ACT is already done!\n",i);
					}
				}


				//CVLAN ACT
				if(rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_CTAG_BIT)
				{
					if((pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_CVLAN_ACT_DONE_BIT)==0x0)//RG_EGR_CVLAN_ACT_DONE_BIT have not been set
					{
						pPktHdr->aclDecision.us_action_field |= CF_US_ACTION_CTAG_BIT;
						pPktHdr->aclDecision.aclEgrDoneAction|=RG_EGR_CVLAN_ACT_DONE_BIT;
						pPktHdr->aclDecision.action_cvlan=rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan;

						ACL("RG_CF[%d] do CVLAN %s	cvid_act:%s  cpri_act:%s  cvid:%d	cpri:%d",
							i,
							name_of_rg_cvlan_tagif_decision[pPktHdr->aclDecision.action_cvlan.cvlanTagIfDecision],
							name_of_rg_cvlan_cvid_decision[pPktHdr->aclDecision.action_cvlan.cvlanCvidDecision],
							name_of_rg_cvlan_cpri_decision[pPktHdr->aclDecision.action_cvlan.cvlanCpriDecision],
							pPktHdr->aclDecision.action_cvlan.assignedCvid,
							pPktHdr->aclDecision.action_cvlan.assignedCpri);
						TRACE("RG_CF[%d] do CVLAN %s  cvid_act:%s  cpri_act:%s	cvid:%d cpri:%d",
							i,
							name_of_rg_cvlan_tagif_decision[pPktHdr->aclDecision.action_cvlan.cvlanTagIfDecision],
							name_of_rg_cvlan_cvid_decision[pPktHdr->aclDecision.action_cvlan.cvlanCvidDecision],
							name_of_rg_cvlan_cpri_decision[pPktHdr->aclDecision.action_cvlan.cvlanCpriDecision],
							pPktHdr->aclDecision.action_cvlan.assignedCvid,
							pPktHdr->aclDecision.action_cvlan.assignedCpri);
					}
					else
					{
						ACL("RG_CF[%d] skip CVLAN , because EGR_CVLAN_ACT is already done!\n",i);
					}
				}


				//CFPRI ACT
				if(rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_CFPRI_BIT)
				{
					if((pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_CFPRI_ACT_DONE_BIT)==0x0)//RG_EGR_CFPRI_ACT_DONE_BIT have not been set
					{
						pPktHdr->aclDecision.us_action_field |= CF_US_ACTION_CFPRI_BIT;
						pPktHdr->aclDecision.aclEgrDoneAction|=RG_EGR_CFPRI_ACT_DONE_BIT;
						pPktHdr->aclDecision.action_cfpri=rg_db.systemGlobal.classify_SW_table_entry[i].action_cfpri;
						ACL("RG_CF[%d] do CFPRI %s, cfpri=%d\n",i, name_of_rg_cfpri_decision[pPktHdr->aclDecision.action_cfpri.cfPriDecision],pPktHdr->aclDecision.action_cfpri.assignedCfPri);
						TRACE("RG_CF[%d] do CFPRI %s, cfpri=%d\n",i, name_of_rg_cfpri_decision[pPktHdr->aclDecision.action_cfpri.cfPriDecision],pPktHdr->aclDecision.action_cfpri.assignedCfPri);
					}
					else
					{
						ACL("RG_CF[%d] skip CFPRI , because EGR_CFPRI_ACT is already done!\n",i);
					}
				}


				//DSCP ACT
				if(rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_DSCP_BIT)
				{
					if((pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_DSCP_ACT_DONE_BIT)==0x0)//RG_EGR_DSCP_ACT_DONE_BIT have not been set
					{
						pPktHdr->aclDecision.us_action_field|= CF_US_ACTION_DSCP_BIT;
						pPktHdr->aclDecision.aclEgrDoneAction|=RG_EGR_DSCP_ACT_DONE_BIT;
						pPktHdr->aclDecision.action_dscp=rg_db.systemGlobal.classify_SW_table_entry[i].action_dscp;
						ACL("RG_CF[%d] do DSCP %s, dscpVal=%d\n",i, name_of_rg_dscp_decision[pPktHdr->aclDecision.action_dscp.dscpDecision],pPktHdr->aclDecision.action_dscp.assignedDscp);
						TRACE("RG_CF[%d] do DSCP %s, dscpVal=%d\n",i, name_of_rg_dscp_decision[pPktHdr->aclDecision.action_dscp.dscpDecision],pPktHdr->aclDecision.action_dscp.assignedDscp);
					}
					else
					{
						ACL("RG_CF[%d] skip DSCP , because EGR_DSCP_ACT is already done!\n",i);
					}
				}

				//FWD ACT
				if(rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_FWD_BIT)
				{
					if((pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_FWD_ACT_DONE_BIT)==0x0)//RG_EGR_FWD_ACT_DONE_BIT have not been set
					{
						pPktHdr->aclDecision.us_action_field |= CF_US_ACTION_FWD_BIT;
						pPktHdr->aclDecision.aclEgrDoneAction|=RG_EGR_FWD_ACT_DONE_BIT;
						pPktHdr->aclDecision.action_fwd=rg_db.systemGlobal.classify_SW_table_entry[i].action_fwd;
						ACL("RG_CF[%d] do FWD %s\n",i, name_of_rg_fwd_decision[pPktHdr->aclDecision.action_fwd.fwdDecision]);
						TRACE("RG_CF[%d] do FWD %s\n",i, name_of_rg_fwd_decision[pPktHdr->aclDecision.action_fwd.fwdDecision]);
					}
					else
					{
						ACL("RG_CF[%d] skip FWD , because EGR_FWD_ACT is already done!\n",i);
					}
				}

				//SID ACT
				if(rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_SID_BIT)
				{
					if((pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_SID_ACT_DONE_BIT)==0x0)//RG_EGR_SID_ACT_DONE_BIT have not been set
					{
						pPktHdr->aclDecision.us_action_field|= CF_US_ACTION_SID_BIT;
						pPktHdr->aclDecision.aclEgrDoneAction|=RG_EGR_SID_ACT_DONE_BIT;
						pPktHdr->aclDecision.action_sid_or_llid=rg_db.systemGlobal.classify_SW_table_entry[i].action_sid_or_llid;
						ACL("RG_CF[%d] do SID %s, streamId=%d\n",i, name_of_rg_sid_decision[pPktHdr->aclDecision.action_sid_or_llid.sidDecision],pPktHdr->aclDecision.action_sid_or_llid.assignedSid_or_llid);
						TRACE("RG_CF[%d] do SID %s, streamId=%d\n",i, name_of_rg_sid_decision[pPktHdr->aclDecision.action_sid_or_llid.sidDecision],pPktHdr->aclDecision.action_sid_or_llid.assignedSid_or_llid);
					}
					else
					{
						ACL("RG_CF[%d] skip SID , because EGR_SID_ACT is already done!\n",i);
					}
				}

			}
			else //downstream
			{

				//SVLAN ACT
				if(rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field & CF_DS_ACTION_STAG_BIT)
				{
					if((pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_SVLAN_ACT_DONE_BIT)==0x0)//RG_EGR_SVLAN_ACT_DONE_BIT have not been set
					{
						pPktHdr->aclDecision.ds_action_field |= CF_DS_ACTION_STAG_BIT;
						pPktHdr->aclDecision.aclEgrDoneAction|=RG_EGR_SVLAN_ACT_DONE_BIT;
						pPktHdr->aclDecision.action_svlan=rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan;

						ACL("RG_CF[%d] do SVLAN %s  svid_act:%s	spri_act:%s  svid:%d  spri:%d",
							i,
							name_of_rg_svlan_tagif_decision[pPktHdr->aclDecision.action_svlan.svlanTagIfDecision],
							name_of_rg_svlan_svid_decision[pPktHdr->aclDecision.action_svlan.svlanSvidDecision],
							name_of_rg_svlan_spri_decision[pPktHdr->aclDecision.action_svlan.svlanSpriDecision],
							pPktHdr->aclDecision.action_svlan.assignedSvid,
							pPktHdr->aclDecision.action_svlan.assignedSpri
						);
						TRACE("RG_CF[%d] do SVLAN %s  svid_act:%s  spri_act:%s  svid:%d	spri:%d",
							i,
							name_of_rg_svlan_tagif_decision[pPktHdr->aclDecision.action_svlan.svlanTagIfDecision],
							name_of_rg_svlan_svid_decision[pPktHdr->aclDecision.action_svlan.svlanSvidDecision],
							name_of_rg_svlan_spri_decision[pPktHdr->aclDecision.action_svlan.svlanSpriDecision],
							pPktHdr->aclDecision.action_svlan.assignedSvid,
							pPktHdr->aclDecision.action_svlan.assignedSpri
						);


					}
					else
					{
						ACL("RG_CF[%d] skip SVLAN , because EGR_SVLAN_ACT is already done!\n",i);
					}
				}

				//CVLAN ACT
				if(rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field & CF_DS_ACTION_CTAG_BIT)
				{
					if((pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_CVLAN_ACT_DONE_BIT)==0x0)//RG_EGR_CVLAN_ACT_DONE_BIT have not been set
					{
						pPktHdr->aclDecision.ds_action_field |= CF_DS_ACTION_CTAG_BIT;
						pPktHdr->aclDecision.aclEgrDoneAction|=RG_EGR_CVLAN_ACT_DONE_BIT;
						pPktHdr->aclDecision.action_cvlan=rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan;

						ACL("RG_CF[%d] do CVLAN %s	cvid_act:%s  cpri_act:%s  cvid:%d	cpri:%d",
							i,
							name_of_rg_cvlan_tagif_decision[pPktHdr->aclDecision.action_cvlan.cvlanTagIfDecision],
							name_of_rg_cvlan_cvid_decision[pPktHdr->aclDecision.action_cvlan.cvlanCvidDecision],
							name_of_rg_cvlan_cpri_decision[pPktHdr->aclDecision.action_cvlan.cvlanCpriDecision],
							pPktHdr->aclDecision.action_cvlan.assignedCvid,
							pPktHdr->aclDecision.action_cvlan.assignedCpri);
						TRACE("RG_CF[%d] do CVLAN %s  cvid_act:%s  cpri_act:%s	cvid:%d cpri:%d",
							i,
							name_of_rg_cvlan_tagif_decision[pPktHdr->aclDecision.action_cvlan.cvlanTagIfDecision],
							name_of_rg_cvlan_cvid_decision[pPktHdr->aclDecision.action_cvlan.cvlanCvidDecision],
							name_of_rg_cvlan_cpri_decision[pPktHdr->aclDecision.action_cvlan.cvlanCpriDecision],
							pPktHdr->aclDecision.action_cvlan.assignedCvid,
							pPktHdr->aclDecision.action_cvlan.assignedCpri);


					}
					else
					{
						ACL("RG_CF[%d] skip CVLAN , because EGR_CVLAN_ACT is already done!\n",i);
					}
				}

				//CFPRI ACT
				if(rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field & CF_DS_ACTION_CFPRI_BIT)
				{
					if((pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_CFPRI_ACT_DONE_BIT)==0x0)//RG_EGR_CFPRI_ACT_DONE_BIT have not been set
					{
						pPktHdr->aclDecision.ds_action_field |= CF_DS_ACTION_CFPRI_BIT;
						pPktHdr->aclDecision.aclEgrDoneAction|=RG_EGR_CFPRI_ACT_DONE_BIT;
						pPktHdr->aclDecision.action_cfpri=rg_db.systemGlobal.classify_SW_table_entry[i].action_cfpri;
						ACL("RG_CF[%d] do CFPRI %s, cfpri=%d\n",i, name_of_rg_cfpri_decision[pPktHdr->aclDecision.action_cfpri.cfPriDecision],pPktHdr->aclDecision.action_cfpri.assignedCfPri);
						TRACE("RG_CF[%d] do CFPRI %s, cfpri=%d\n",i, name_of_rg_cfpri_decision[pPktHdr->aclDecision.action_cfpri.cfPriDecision],pPktHdr->aclDecision.action_cfpri.assignedCfPri);
					}
					else
					{
						ACL("RG_CF[%d] skip CFPRI , because EGR_CFPRI_ACT is already done!\n",i);
					}
				}


				//DSCP ACT
				if(rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field & CF_DS_ACTION_DSCP_BIT)
				{
					if((pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_DSCP_ACT_DONE_BIT)==0x0)//RG_EGR_DSCP_ACT_DONE_BIT have not been set
					{
						pPktHdr->aclDecision.ds_action_field|= CF_DS_ACTION_DSCP_BIT;
						pPktHdr->aclDecision.aclEgrDoneAction|=RG_EGR_DSCP_ACT_DONE_BIT;
						pPktHdr->aclDecision.action_dscp=rg_db.systemGlobal.classify_SW_table_entry[i].action_dscp;
						ACL("RG_CF[%d] do DSCP %s, dscpVal=%d\n",i, name_of_rg_dscp_decision[pPktHdr->aclDecision.action_dscp.dscpDecision],pPktHdr->aclDecision.action_dscp.assignedDscp);
						TRACE("RG_CF[%d] do DSCP %s, dscpVal=%d\n",i, name_of_rg_dscp_decision[pPktHdr->aclDecision.action_dscp.dscpDecision],pPktHdr->aclDecision.action_dscp.assignedDscp);
					}
					else
					{
						ACL("RG_CF[%d] skip DSCP , because EGR_DSCP_ACT is already done!\n",i);
					}
				}


				//FWD ACT
				if(rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field & CF_DS_ACTION_UNI_MASK_BIT)
				{
					if((pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_FWD_ACT_DONE_BIT)==0x0)//RG_EGR_FWD_ACT_DONE_BIT have not been set
					{
						if(rg_db.systemGlobal.classify_SW_table_entry[i].action_uni.uniActionDecision==ACL_UNI_FORCE_BY_MASK && rg_db.systemGlobal.classify_SW_table_entry[i].action_uni.assignedUniPortMask!=0x0){//ACL_UNI_FORCE_BY_MASK only support for drop, apolloPro can not support L34 egress forward to multi-port
							ACL("ACL_UNI_FORCE_BY_MASK is not not support in apolloPro, except using for drop(action_uni.assignedUniPortMask==0x0)");
						}else{	//only support ACL_UNI_FWD_TO_PORTMASK_ONLY
							pPktHdr->aclDecision.ds_action_field|= CF_DS_ACTION_UNI_MASK_BIT;
							pPktHdr->aclDecision.aclEgrDoneAction|=RG_EGR_FWD_ACT_DONE_BIT;
							pPktHdr->aclDecision.action_uni=rg_db.systemGlobal.classify_SW_table_entry[i].action_uni;
							ACL("RG_CF[%d] do UNI %s, portmask=0x%x\n",i, name_of_rg_uni_decision[pPktHdr->aclDecision.action_uni.uniActionDecision],pPktHdr->aclDecision.action_uni.assignedUniPortMask);
							TRACE("RG_CF[%d] do UNI %s, portmask=0x%x\n",i, name_of_rg_uni_decision[pPktHdr->aclDecision.action_uni.uniActionDecision],pPktHdr->aclDecision.action_uni.assignedUniPortMask);
						}
					}
					else
					{
						ACL("RG_CF[%d] skip UNI , because EGR_FWD_ACT is already done!\n",i);
					}
				}


			}

		}
	}

	return RG_FWDENGINE_RET_CONTINUE;
}



rtk_rg_fwdEngineReturn_t _rtk_rg_modifyPacketByACLAction(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr,rtk_rg_port_idx_t egressPort)
{
	uint8 tos;

	/* do RG ACL Qos actions */
	if(pPktHdr->aclDecision.action_type==ACL_ACTION_TYPE_QOS)
	{
		if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_1P_REMARKING_BIT)>0)
		{
			pPktHdr->egressPriority = pPktHdr->aclDecision.action_dot1p_remarking_pri;
			TRACE("Modify by RG_ACL ACT:1P_REMARKING egreesPri=%d",pPktHdr->aclDecision.action_dot1p_remarking_pri);
		}

		if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_TOS_TC_REMARKING_BIT)>0)
		{
			if(pPktHdr->pTos==NULL){ //packet may not have IP header
				TRACE("Modify by RG_ACL ACT: Failed! Due to packet has no pTos Field");
			}else if(!(pPktHdr->tagif&IPV4_TAGIF || pPktHdr->tagif&IPV6_TAGIF)){
				TRACE("Modify by RG_ACL ACT: Failed! Due to packet has no IP header");
			}else{
				if(pPktHdr->tagif&IPV6_TAGIF)
				{
					tos = pPktHdr->aclDecision.action_tos_tc_remarking_pri>>0x4;	//tos_tc MSB 4 bits
					tos |= (*pPktHdr->pTos)&0xf0;		//keep version 4 bits
					*(skb->data + (pPktHdr->pTos-pPktHdr->skb->data))=tos;

					tos = (pPktHdr->aclDecision.action_tos_tc_remarking_pri&0xf)<<0x4;	//tos_tc LSB 4 bits
					tos |= (*(pPktHdr->pTos+1))&0xf;		//keep flow label MSB 4 bits
					*((skb->data + (pPktHdr->pTos-pPktHdr->skb->data))+1)=tos;
				}
				else
				{
					tos = pPktHdr->aclDecision.action_tos_tc_remarking_pri;
					*(skb->data + (pPktHdr->pTos-pPktHdr->skb->data))=tos;		//remarking tos of packet
				}
				pPktHdr->egressDSCP = pPktHdr->aclDecision.action_tos_tc_remarking_pri>>0x2;	//tos_tc MSB 6 bits
				pPktHdr->egressECN = pPktHdr->aclDecision.action_tos_tc_remarking_pri&0x3;		//tos_tc LSB 2 bits
				TRACE("Modify by RG_ACL ACT:TOS_TC_REMARKING %s DSCP=%d, ECN=%d",(pPktHdr->tagif&IPV6_TAGIF)?"TC":"TOS",pPktHdr->egressDSCP, pPktHdr->egressECN);
			}
		}
		else if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT)>0)
		{
			if(pPktHdr->pTos==NULL){
				TRACE("Modify by RG_ACL ACT: Failed! Due to packet has no pTos Field");
			}		//packet may not have IP header
			else if(!(pPktHdr->tagif&IPV4_TAGIF || pPktHdr->tagif&IPV6_TAGIF)){
				TRACE("Modify by RG_ACL ACT: Failed! Due to packet has no IP header");
			}
			else{
				if(pPktHdr->tagif&IPV6_TAGIF)
				{
					//ip precedence is the MSB 3 bits of traffic class
					tos = pPktHdr->aclDecision.action_ip_precedence_remarking_pri<<0x1;
					tos |= (*pPktHdr->pTos)&0xf1;		//keep version 4 bits and original traffic class MSB 4th bit
					*(skb->data + (pPktHdr->pTos-pPktHdr->skb->data))=tos;
				}
				else
				{
					tos = pPktHdr->aclDecision.action_ip_precedence_remarking_pri<<0x5;
					tos |= (*pPktHdr->pTos)&0x1f;		//keep 5 bits from LSB
					*(skb->data + (pPktHdr->pTos-pPktHdr->skb->data))=tos;		//remarking tos of packet
				}
				TRACE("Modify by RG_ACL ACT:IP_PRECEDENCE_REMARKING ToS=%d",tos);
			}
		}
		else if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_DSCP_REMARKING_BIT)>0)
		{
			if(pPktHdr->pTos==NULL){ //packet may not have IP header
				TRACE("Modify by RG_ACL ACT: Failed! Due to packet has no pTos Field");
			}else if(!(pPktHdr->tagif&IPV4_TAGIF || pPktHdr->tagif&IPV6_TAGIF)){
				TRACE("Modify by RG_ACL ACT: Failed! Due to packet has no IP header");
			}else{
				if(pPktHdr->tagif&IPV6_TAGIF)
				{
					//dscp is the MSB 6 bits of traffic class
					tos = pPktHdr->aclDecision.action_dscp_remarking_pri>>0x2;	//dscp MSB 4 bits
					tos |= (*pPktHdr->pTos)&0xf0;		//keep version 4 bits
					*(skb->data + (pPktHdr->pTos-pPktHdr->skb->data))=tos;

					tos = (pPktHdr->aclDecision.action_dscp_remarking_pri&0x3)<<0x6;	//dscp LSB 2 bits
					tos |= (*(pPktHdr->pTos+1))&0x3f;		//keep original traffic label LSB 2 bits and flow label MSB 4 bits
					*((skb->data + (pPktHdr->pTos-pPktHdr->skb->data))+1)=tos;
				}
				else
				{
					tos = pPktHdr->aclDecision.action_dscp_remarking_pri<<0x2;
					tos |= (*pPktHdr->pTos)&0x3;		//keep 2 bits from LSB
					*(skb->data + (pPktHdr->pTos-pPktHdr->skb->data))=tos;		//remarking tos of packet
				}
				pPktHdr->egressDSCP = pPktHdr->aclDecision.action_dscp_remarking_pri;
				TRACE("Modify by RG_ACL ACT:DSCP_REMARKING DSCP=%d",pPktHdr->egressDSCP);
			}
		}

		if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT)>0){
			pPktHdr->internalPriority= pPktHdr->aclDecision.action_acl_egress_internal_priority;
			TRACE("Modify by RG_ACL ACT:CFPRI egress_internal_priority=%d",pPktHdr->internalPriority);
		}


		if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_STREAM_ID_OR_LLID_BIT)>0){
#ifdef CONFIG_GPON_FEATURE
			if(rg_db.systemGlobal.initParam.wanPortGponMode){
				pPktHdr->streamID = pPktHdr->aclDecision.action_stream_id_or_llid&0x7f;
				TRACE("Modify by RG_ACL ACT:STREAM_ID_OR_LLID StreamID=%d",pPktHdr->aclDecision.action_stream_id_or_llid&0x7f);
			}else{
				pPktHdr->streamID = pPktHdr->aclDecision.action_stream_id_or_llid&0x7f;
				TRACE("Modify by RG_ACL ACT:STREAM_ID_OR_LLID LLID=%d",pPktHdr->aclDecision.action_stream_id_or_llid&0x7f);
				//TRACE("Modify by RG_ACL Weired!!! wanPortGponMode=%d but assigned streamID action rule, skip this streamID action.",rg_db.systemGlobal.initParam.wanPortGponMode);
			}
#endif
		}

#if 0 //9607C not support egress to multi-port or redirect to none-lut port action in path5
		if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_REDIRECT_BIT)>0){
			pPktHdr->egressUniPortmask = pPktHdr->aclDecision.action_redirect_portmask;
			TRACE("Modify by RG_ACL ACT:REDIRCT egressPortMask=0x%x",pPktHdr->egressUniPortmask);

			if(pPktHdr->egressUniPortmask==0x0){
				ACL("Drop! because egressPmsk is change to 0x0 by UNI action");
				TRACE("[Drop] Modify by RG_ACL: Drop! finalPortMask is 0x0");
				return RG_FWDENGINE_RET_DROP;
			}
		}
#endif

		if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_DS_UNIMASK_BIT)>0){

			switch(pPktHdr->aclDecision.action_acl_uni.uniActionDecision){
				case ACL_UNI_FWD_TO_PORTMASK_ONLY:
					pPktHdr->egressUniPortmask = pPktHdr->aclDecision.action_acl_uni.assignedUniPortMask & (1<<pPktHdr->egressMacPort); //no matter unicast or broadcast, once only one port is sending.
					ACL(" RG_ACL egressMacPort %d filtered with egressPmsk 0x%x",pPktHdr->egressMacPort,pPktHdr->aclDecision.action_acl_uni.assignedUniPortMask);
					TRACE("Modify by RG_ACL ACT:UNI_FWD_TO_PORTMASK_ONLY finalPortMask=0x%x",pPktHdr->egressUniPortmask);

					if(pPktHdr->egressUniPortmask==0x0){
						ACL("Drop! because egressPmsk is change to 0x0 by UNI action");
						TRACE("[Drop] Modify by RG_ACL: Drop! finalPortMask is 0x0");
						return RG_FWDENGINE_RET_DROP;
					}
					break;
				case ACL_UNI_FORCE_BY_MASK:
					pPktHdr->egressUniPortmask = pPktHdr->aclDecision.action_acl_uni.assignedUniPortMask;
					ACL(" RG_ACL force egressPmsk change to 0x%x",pPktHdr->aclDecision.action_acl_uni.assignedUniPortMask);
					TRACE("Modify by RG_ACL ACT:UNI_FORCE_BY_MASK finalPortMask=0x%x",pPktHdr->egressUniPortmask);

					if(pPktHdr->egressUniPortmask==0x0){
						ACL("Drop! because egressPmsk is change to 0x0 by UNI action");
						TRACE("[Drop] Modify by RG_ACL: Drop! finalPortMask is 0x0");
						return RG_FWDENGINE_RET_DROP;
					}else{
						//FB-flow5 can not support egress to multi-port
						WARNING("ACL_UNI_FORCE_BY_MASK support egress to multi-port action, only support for drop purpose(egressUniPortmask=0x0)!");
					}
					break;

				default:
					break;
			}
		}


		if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_ACL_CVLANTAG_BIT)>0){
			pPktHdr->flow_cvid_with_egress_filter = FALSE;
			switch(pPktHdr->aclDecision.action_acl_cvlan.cvlanTagIfDecision){
				case ACL_CVLAN_TAGIF_NOP:
					/*do nothing, follow switch-core*/
					break;

				case ACL_CVLAN_TAGIF_TAGGING:
					{
						/*force tagging*/
						pPktHdr->egressVlanTagif = 1;

						//CVDI decision
						switch(pPktHdr->aclDecision.action_acl_cvlan.cvlanCvidDecision)
						{
							case ACL_CVLAN_CVID_NOP:
								//do nothing
								break;
							case ACL_CVLAN_CVID_ASSIGN:
								{
									pPktHdr->egressVlanID = pPktHdr->aclDecision.action_acl_cvlan.assignedCvid;
									pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
									TRACE("Modify by RG_ACL ACT:CVID_ASSIGN CVID=%d",pPktHdr->egressVlanID);
								}
								break;
							case ACL_CVLAN_CVID_COPY_FROM_1ST_TAG:
								{
									if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
										pPktHdr->egressVlanID = pPktHdr->stagVid;
										pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
										TRACE("Modify by RG_ACL ACT:CVID_COPY_FROM_1ST_TAG(S) CVID=%d",pPktHdr->egressVlanID);
									}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
										pPktHdr->egressVlanID = pPktHdr->ctagVid;
										pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
										TRACE("Modify by RG_ACL ACT:CVID_COPY_FROM_1ST_TAG(C) CVID=%d",pPktHdr->egressVlanID);
									}else{
										//no 1st tag, use assignedCvid as H/W
										pPktHdr->egressVlanID = pPktHdr->aclDecision.action_acl_cvlan.assignedCvid;
										pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
										TRACE("Modify by RG_ACL ACT:CVID_COPY_FROM_1ST_TAG(none tag) CVID=%d",pPktHdr->egressVlanID);
									}
								}
								break;
							case ACL_CVLAN_CVID_COPY_FROM_2ND_TAG:
								{
									if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
										pPktHdr->egressVlanID = pPktHdr->ctagVid;
										pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
										TRACE("Modify by RG_ACL ACT:CVID_COPY_FROM_2ND_TAG CVID=%d",pPktHdr->egressVlanID);
									}else{
										//no 2nd tag, use assignedCvid as H/W
										pPktHdr->egressVlanID = pPktHdr->aclDecision.action_acl_cvlan.assignedCvid;
										pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
										TRACE("Modify by RG_ACL ACT:CVID_COPY_FROM_2ND_TAG(none tag) CVID=%d",pPktHdr->egressVlanID);
									}
								}
								break;
							case ACL_CVLAN_CVID_COPY_FROM_INTERNAL_VID://upstream only
								{
									pPktHdr->egressVlanID = pPktHdr->internalVlanID;
									pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
									TRACE("Modify by RG_ACL ACT:CVID_COPY_FROM_INTERNAL_VID CVID=%d",pPktHdr->egressVlanID);
								}
								break;
							case ACL_CVLAN_CVID_CPOY_FROM_SP2C: //downstream only
								{
									WARNING("ACL_CVLAN_CVID_CPOY_FROM_SP2C is not supported in fwdEninge!");
								}
								break;
							case ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID: //downstream only
								{
									WARNING("ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID is not supported in fwdEninge!");
								}
								break;

							default:
								break;

						}


						//CPRI decision
						switch(pPktHdr->aclDecision.action_acl_cvlan.cvlanCpriDecision)
						{

							case ACL_CVLAN_CPRI_NOP:
								//do nothing
								break;
							case ACL_CVLAN_CPRI_ASSIGN:
								{
									pPktHdr->egressPriority = pPktHdr->aclDecision.action_acl_cvlan.assignedCpri;
									TRACE("Modify by RG_ACL ACT:CPRI_ASSIGN CPRI=%d",pPktHdr->egressPriority);
								}
								break;
							case ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG:
								{
									if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
										pPktHdr->egressPriority = pPktHdr->stagPri;
										TRACE("Modify by RG_ACL ACT:CPRI_COPY_FROM_1ST_TAG(S) CPRI=%d",pPktHdr->egressPriority);
									}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
										pPktHdr->egressPriority = pPktHdr->ctagPri;
										TRACE("Modify by RG_ACL ACT:CPRI_COPY_FROM_1ST_TAG(C) CPRI=%d",pPktHdr->egressPriority);
									}else{
										//no 1st tag, use assignedCpri as H/W
										pPktHdr->egressPriority = pPktHdr->aclDecision.action_acl_cvlan.assignedCpri;
										TRACE("Modify by RG_ACL ACT:CPRI_COPY_FROM_1ST_TAG(none tag) CPRI=%d",pPktHdr->egressPriority);
									}
								}
								break;
							case ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG:
								{
									if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
										pPktHdr->egressVlanID = pPktHdr->ctagVid;
										pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
										TRACE("Modify by RG_ACL ACT:CVID_COPY_FROM_2ND_TAG CVID=%d",pPktHdr->egressVlanID);
									}else{
										//no 2nd tag, use assignedCpri as H/W
										pPktHdr->egressPriority = pPktHdr->aclDecision.action_acl_cvlan.assignedCpri;
										TRACE("Modify by RG_ACL ACT:CPRI_COPY_FROM_1ST_TAG(none tag) CPRI=%d",pPktHdr->egressPriority);
									}
								}
								break;
							case ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI:
								{
									pPktHdr->egressPriority = pPktHdr->internalPriority;
									TRACE("Modify by RG_ACL ACT:CPRI_COPY_FROM_INTERNAL CPRI=%d",pPktHdr->egressPriority);
								}
								break;
							case ACL_CVLAN_CPRI_COPY_FROM_DSCP_REMAP:
								{
									WARNING("ACL_CVLAN_CPRI_COPY_FROM_DSCP_REMAP is not supported in fwdEninge!");
								}
								break;
							case ACL_CVLAN_CPRI_COPY_FROM_SP2C:
								{
									WARNING("ACL_CVLAN_CPRI_COPY_FROM_SP2C is not supported in fwdEninge!");
								}
								break; //downstream only

							default:
								break;
						}
					}
					break;

				case ACL_CVLAN_TAGIF_UNTAG:
					{
						pPktHdr->egressVlanTagif = 0;
						pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
						TRACE("Modify by RG_ACL ACT:UN-CTAG");
					}break;

				case ACL_CVLAN_TAGIF_TRANSPARENT:
					{
						pPktHdr->dmac2VlanID = FAIL;//avoid Dam2cvid change the cvlan
						if(pPktHdr->tagif & CVLAN_TAGIF){
							pPktHdr->egressVlanTagif = 1;
							pPktHdr->egressVlanID = pPktHdr->ctagVid;
							pPktHdr->egressPriority = pPktHdr->ctagPri;
						}else{//untag
							pPktHdr->egressVlanTagif = 0;
						}
						TRACE("Modify by RG_ACL ACT:CTAG TRANSPARENT");
					}
					break;

				default:
					pPktHdr->flow_cvid_with_egress_filter = TRUE;
					break;
			}

		}


		if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_ACL_SVLANTAG_BIT)>0){

			switch(pPktHdr->aclDecision.action_acl_svlan.svlanTagIfDecision){

				case ACL_SVLAN_TAGIF_NOP:
					//do nothing
					break;

				case ACL_SVLAN_TAGIF_UNTAG:
					{
						pPktHdr->egressServiceVlanTagif = 0;
						TRACE("Modify by RG_ACL ACT:UN-STAG");
					}
					break;

				case ACL_SVLAN_TAGIF_TRANSPARENT:
					{
						if(pPktHdr->tagif & SVLAN_TAGIF){
							pPktHdr->egressServiceVlanTagif = 1;
							pPktHdr->egressServiceVlanID = pPktHdr->stagVid;
							pPktHdr->egressServicePriority = pPktHdr->stagPri;
						}else{//untag
							pPktHdr->egressServiceVlanTagif = 0;
						}
						TRACE("Modify by RG_ACL ACT:STAG TRANSPARENT");
					}
					break;

				case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID:
					pPktHdr->egressServiceVlanTagif = 1; //force tagging with tpid
					TRACE("Modify by RG_ACL ACT:SVLANTAG with TPID");
					goto svidSpriDecision;
				case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2:
					pPktHdr->egressServiceVlanTagif = 2; //force tagging with tpid2
					TRACE("Modify by RG_ACL ACT:SVLANTAG with TPID2");
					goto svidSpriDecision;
				case ACL_SVLAN_TAGIF_TAGGING_WITH_ORIGINAL_STAG_TPID:
					pPktHdr->egressServiceVlanTagif = 3; //force tagging with original-stag-tpid
					TRACE("Modify by RG_ACL ACT:SVLANTAG with TPID2");
					goto svidSpriDecision;

svidSpriDecision:
				//SVID decision
				switch(pPktHdr->aclDecision.action_acl_svlan.svlanSvidDecision)
				{
					case ACL_SVLAN_SVID_NOP:
						//do nothing
						break;
					case ACL_SVLAN_SVID_ASSIGN:
						{
							pPktHdr->egressServiceVlanID = pPktHdr->aclDecision.action_acl_svlan.assignedSvid;
							TRACE("Modify by RG_ACL ACT:SVID_ASSIGN SVID=%d",pPktHdr->egressServiceVlanID);
						}
						break;
					case ACL_SVLAN_SVID_COPY_FROM_1ST_TAG:
						{
							if(pPktHdr->tagif & SVLAN_TAGIF){
								pPktHdr->egressServiceVlanID = pPktHdr->stagVid;
								TRACE("Modify by RG_ACL ACT:SVID_COPY_FROM_1ST_TAG(S) SVID=%d",pPktHdr->egressServiceVlanID);
							}else if (pPktHdr->tagif & CVLAN_TAGIF){
								pPktHdr->egressServiceVlanID = pPktHdr->ctagVid;
								TRACE("Modify by RG_ACL ACT:SVID_COPY_FROM_1ST_TAG(C) SVID=%d",pPktHdr->egressServiceVlanID);
							}else{
								//no 1st tag, use assignedSvid as H/W
								pPktHdr->egressServiceVlanID = pPktHdr->aclDecision.action_acl_svlan.assignedSvid;
								TRACE("Modify by RG_ACL ACT:SVID_COPY_FROM_1ST_TAG(none tag) SVID=%d",pPktHdr->egressServiceVlanID);
							}
						}
						break;
					case ACL_SVLAN_SVID_COPY_FROM_2ND_TAG:
						{
							if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){
								pPktHdr->egressServiceVlanID = pPktHdr->ctagVid;
								TRACE("Modify by RG_ACL ACT:SVID_COPY_FROM_2ND_TAG SVID=%d",pPktHdr->egressServiceVlanID);
							}else{
								//no 2nd tag, use assignedSvid as H/W
								pPktHdr->egressServiceVlanID = pPktHdr->aclDecision.action_acl_svlan.assignedSvid;
								TRACE("Modify by RG_ACL ACT:SVID_COPY_FROM_2ND_TAG(none tag) SVID=%d",pPktHdr->egressServiceVlanID);
							}
						}
						break;
					case ACL_SVLAN_SVID_SP2C: //downstream only
						{
							WARNING("ACL_SVLAN_SVID_SP2C is not supported in fwdEninge!");
						}
						break;
					default:
						break;
				}

				switch(pPktHdr->aclDecision.action_svlan.svlanSpriDecision)
				{
					case ACL_SVLAN_SPRI_NOP:
						//do nothing
						break;
					case ACL_SVLAN_SPRI_ASSIGN:
						{
							pPktHdr->egressServicePriority = pPktHdr->aclDecision.action_acl_svlan.assignedSpri;
							TRACE("Modify by RG_ACL ACT:SPRI_ASSIGN SPRI=%d",pPktHdr->egressServicePriority);
						}
						break;
					case ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG:
						{
							if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
								pPktHdr->egressServicePriority = pPktHdr->stagPri;
								TRACE("Modify by RG_ACL ACT:SPRI_COPY_FROM_1ST_TAG(S) SPRI=%d",pPktHdr->egressServicePriority);
							}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
								pPktHdr->egressServicePriority = pPktHdr->ctagPri;
								TRACE("Modify by RG_ACL ACT:SPRI_COPY_FROM_1ST_TAG(C) SPRI=%d",pPktHdr->egressServicePriority);
							}else{
								//no 1st tag, use assignedSpri as H/W
								pPktHdr->egressServicePriority = pPktHdr->aclDecision.action_acl_svlan.assignedSpri;
								TRACE("Modify by RG_ACL ACT:SPRI_COPY_FROM_1ST_TAG(none tag) SPRI=%d",pPktHdr->egressServicePriority);
							}
						}
						break;
					case ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG:
						{
							if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
								pPktHdr->egressServicePriority = pPktHdr->ctagPri;
								TRACE("Modify by RG_ACL ACT:SPRI_COPY_FROM_2ND_TAG SPRI=%d",pPktHdr->egressServicePriority);
							}else{
								//no 2nd tag, use assignedSpri as H/W
								pPktHdr->egressServicePriority = pPktHdr->aclDecision.action_acl_svlan.assignedSpri;
								TRACE("Modify by RG_ACL ACT:SPRI_COPY_FROM_2ND_TAG(none tag) SPRI=%d",pPktHdr->egressServicePriority);
							}

						}
						break;
					case ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI:
						{
							pPktHdr->egressServicePriority = pPktHdr->internalPriority;
							TRACE("Modify by RG_ACL ACT:SPRI_COPY_FROM_INTERNAL_PRI SPRI=%d",pPktHdr->egressServicePriority);
						}
						break;
					case ACL_SVLAN_SPRI_COPY_FROM_DSCP_REMAP: //downstream only
						{
							WARNING("APOLLOFE_ACL_SVLAN_SPRI_COPY_FROM_DSCP_REMAP is not supported in fwdEninge!");
						}
						break;
					case ACL_SVLAN_SPRI_COPY_FROM_SP2C: //downstream only
						{
							WARNING("APOLLOFE_ACL_SVLAN_SPRI_COPY_FROM_SP2C is not supported in fwdEninge!");
						}
						break;

					default:
						break;
				}
				default:
					break;
			}
		}
	}


	/* do RG CF pattern1 actions */
	{
		//ACL("Do CF64-511 actions:");
		if(pPktHdr->aclDecision.direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM){
			//check US action

			//Stag action
			if((pPktHdr->aclDecision.us_action_field&CF_US_ACTION_STAG_BIT)>0)
			{
				switch(pPktHdr->aclDecision.action_svlan.svlanTagIfDecision){

					case ACL_SVLAN_TAGIF_NOP:
						//do nothing
						break;

					case ACL_SVLAN_TAGIF_UNTAG:
						{
							pPktHdr->egressServiceVlanTagif = 0;
							TRACE("Modify by RG_CF ACT:UN-STAG");
						}
						break;

					case ACL_SVLAN_TAGIF_TRANSPARENT:
						{
							if(pPktHdr->tagif & SVLAN_TAGIF){
								pPktHdr->egressServiceVlanTagif = 1;
								pPktHdr->egressServiceVlanID = pPktHdr->stagVid;
								pPktHdr->egressServicePriority = pPktHdr->stagPri;
							}else{//untag
								pPktHdr->egressServiceVlanTagif = 0;
							}
							TRACE("Modify by RG_CF ACT:STAG TRANSPARENT");
						}
						break;

					case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID:
						pPktHdr->egressServiceVlanTagif = 1; //force tagging with tpid
						TRACE("Modify by RG_CF ACT:SVLANTAG with TPID");
						goto cfUssvidSpriDecision;
					case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2:
						pPktHdr->egressServiceVlanTagif = 2; //force tagging with tpid2
						TRACE("Modify by RG_CF ACT:SVLANTAG with TPID2");
						goto cfUssvidSpriDecision;
					case ACL_SVLAN_TAGIF_TAGGING_WITH_ORIGINAL_STAG_TPID:
						pPktHdr->egressServiceVlanTagif = 3; //force tagging with original-stag-tpid
						TRACE("Modify by RG_CF ACT:SVLANTAG with TPID2");
						goto cfUssvidSpriDecision;

cfUssvidSpriDecision:
					//SVID decision
					switch(pPktHdr->aclDecision.action_svlan.svlanSvidDecision)
					{
						case ACL_SVLAN_SVID_NOP:
							//do nothing
							break;
						case ACL_SVLAN_SVID_ASSIGN:
							{
								pPktHdr->egressServiceVlanID = pPktHdr->aclDecision.action_svlan.assignedSvid;
								TRACE("Modify by RG_CF ACT:SVID_ASSIGN SVID=%d",pPktHdr->egressServiceVlanID);
							}
							break;
						case ACL_SVLAN_SVID_COPY_FROM_1ST_TAG:
							{
								if(pPktHdr->tagif & SVLAN_TAGIF){
									pPktHdr->egressServiceVlanID = pPktHdr->stagVid;
									TRACE("Modify by RG_CF ACT:SVID_COPY_FROM_1ST_TAG(S) SVID=%d",pPktHdr->egressServiceVlanID);
								}else if (pPktHdr->tagif & CVLAN_TAGIF){
									pPktHdr->egressServiceVlanID = pPktHdr->ctagVid;
									TRACE("Modify by RG_CF ACT:SVID_COPY_FROM_1ST_TAG(C) SVID=%d",pPktHdr->egressServiceVlanID);
								}else{
									//no 1st tag, use assignedSvid as H/W
									pPktHdr->egressServiceVlanID = pPktHdr->aclDecision.action_svlan.assignedSvid;
									TRACE("Modify by RG_CF ACT:SVID_COPY_FROM_1ST_TAG(none tag) SVID=%d",pPktHdr->egressServiceVlanID);
								}
							}
							break;
						case ACL_SVLAN_SVID_COPY_FROM_2ND_TAG:
							{
								if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){
									pPktHdr->egressServiceVlanID = pPktHdr->ctagVid;
									TRACE("Modify by RG_CF ACT:SVID_COPY_FROM_2ND_TAG SVID=%d",pPktHdr->egressServiceVlanID);
								}else{
									//no 2nd tag, use assignedSvid as H/W
									pPktHdr->egressServiceVlanID = pPktHdr->aclDecision.action_svlan.assignedSvid;
									TRACE("Modify by RG_CF ACT:SVID_COPY_FROM_2ND_TAG(none tag) SVID=%d",pPktHdr->egressServiceVlanID);
								}
							}
							break;
						case ACL_SVLAN_SVID_SP2C: //downstream only
							{
								WARNING("ACL_SVLAN_SVID_SP2C is not supported in fwdEninge!");
							}
							break;
						default:
							break;
					}

					switch(pPktHdr->aclDecision.action_svlan.svlanSpriDecision)
					{
						case ACL_SVLAN_SPRI_NOP:
							//do nothing
							break;
						case ACL_SVLAN_SPRI_ASSIGN:
							{
								pPktHdr->egressServicePriority = pPktHdr->aclDecision.action_svlan.assignedSpri;
								TRACE("Modify by RG_CF ACT:SPRI_ASSIGN SPRI=%d",pPktHdr->egressServicePriority);
							}
							break;
						case ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG:
							{
								if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
									pPktHdr->egressServicePriority = pPktHdr->stagPri;
									TRACE("Modify by RG_CF ACT:SPRI_COPY_FROM_1ST_TAG(S) SPRI=%d",pPktHdr->egressServicePriority);
								}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
									pPktHdr->egressServicePriority = pPktHdr->ctagPri;
									TRACE("Modify by RG_CF ACT:SPRI_COPY_FROM_1ST_TAG(C) SPRI=%d",pPktHdr->egressServicePriority);
								}else{
									//no 1st tag, use assignedSpri as H/W
									pPktHdr->egressServicePriority = pPktHdr->aclDecision.action_svlan.assignedSpri;
									TRACE("Modify by RG_CF ACT:SPRI_COPY_FROM_1ST_TAG(none tag) SPRI=%d",pPktHdr->egressServicePriority);
								}
							}
							break;
						case ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG:
							{
								if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
									pPktHdr->egressServicePriority = pPktHdr->ctagPri;
									TRACE("Modify by RG_CF ACT:SPRI_COPY_FROM_2ND_TAG SPRI=%d",pPktHdr->egressServicePriority);
								}else{
									//no 2nd tag, use assignedSpri as H/W
									pPktHdr->egressServicePriority = pPktHdr->aclDecision.action_svlan.assignedSpri;
									TRACE("Modify by RG_CF ACT:SPRI_COPY_FROM_2ND_TAG(none tag) SPRI=%d",pPktHdr->egressServicePriority);
								}

							}
							break;
						case ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI:
							{
								pPktHdr->egressServicePriority = pPktHdr->internalPriority;
								TRACE("Modify by RG_CF ACT:SPRI_COPY_FROM_INTERNAL_PRI SPRI=%d",pPktHdr->egressServicePriority);
							}
							break;
						case ACL_SVLAN_SPRI_COPY_FROM_DSCP_REMAP: //downstream only
							{
								WARNING("ACL_SVLAN_SPRI_COPY_FROM_DSCP_REMAP is not supported in fwdEninge!");
							}
							break;
						case ACL_SVLAN_SPRI_COPY_FROM_SP2C: //downstream only
							{
								WARNING("ACL_SVLAN_SPRI_COPY_FROM_SP2C is not supported in fwdEninge!");
							}
							break;

						default:
							break;
					}

					default:
						break;
				}
			}

			//Ctag action
			if((pPktHdr->aclDecision.us_action_field&CF_US_ACTION_CTAG_BIT)>0)
			{
				pPktHdr->flow_cvid_with_egress_filter = FALSE;
				switch(pPktHdr->aclDecision.action_cvlan.cvlanTagIfDecision){
					case ACL_CVLAN_TAGIF_NOP:
						/*do nothing, follow switch-core*/
						break;

					case ACL_CVLAN_TAGIF_TAGGING:
						{
							/*force tagging*/
							pPktHdr->egressVlanTagif = 1;

							//CVDI decision
							switch(pPktHdr->aclDecision.action_cvlan.cvlanCvidDecision)
							{
								case ACL_CVLAN_CVID_NOP:
									//do nothing
									break;
								case ACL_CVLAN_CVID_ASSIGN:
									{
										pPktHdr->egressVlanID = pPktHdr->aclDecision.action_cvlan.assignedCvid;
										pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
										TRACE("Modify by RG_CF ACT:CVID_ASSIGN CVID=%d",pPktHdr->egressVlanID);
									}
									break;
								case ACL_CVLAN_CVID_COPY_FROM_1ST_TAG:
									{
										if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
											pPktHdr->egressVlanID = pPktHdr->stagVid;
											pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
											TRACE("Modify by RG_CF ACT:CVID_COPY_FROM_1ST_TAG(S) CVID=%d",pPktHdr->egressVlanID);
										}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
											pPktHdr->egressVlanID = pPktHdr->ctagVid;
											pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
											TRACE("Modify by RG_CF ACT:CVID_COPY_FROM_1ST_TAG(C) CVID=%d",pPktHdr->egressVlanID);
										}else{
											//no 1st tag, use assignedCvid as H/W
											pPktHdr->egressVlanID = pPktHdr->aclDecision.action_cvlan.assignedCvid;
											pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
											TRACE("Modify by RG_CF ACT:CVID_COPY_FROM_1ST_TAG(none tag) CVID=%d",pPktHdr->egressVlanID);
										}
									}
									break;
								case ACL_CVLAN_CVID_COPY_FROM_2ND_TAG:
									{
										if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
											pPktHdr->egressVlanID = pPktHdr->ctagVid;
											pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
											TRACE("Modify by RG_CF ACT:CVID_COPY_FROM_2ND_TAG CVID=%d",pPktHdr->egressVlanID);
										}else{
											//no 2nd tag, use assignedCvid as H/W
											pPktHdr->egressVlanID = pPktHdr->aclDecision.action_cvlan.assignedCvid;
											pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
											TRACE("Modify by RG_CF ACT:CVID_COPY_FROM_2ND_TAG(none tag) CVID=%d",pPktHdr->egressVlanID);
										}
									}
									break;
								case ACL_CVLAN_CVID_COPY_FROM_INTERNAL_VID://upstream only
									{
										pPktHdr->egressVlanID = pPktHdr->internalVlanID;
										pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
										TRACE("Modify by RG_CF ACT:CVID_COPY_FROM_INTERNAL_VID CVID=%d",pPktHdr->egressVlanID);
									}
									break;
								case ACL_CVLAN_CVID_CPOY_FROM_SP2C: //downstream only
									{
										WARNING("APOLLOFE_ACL_CVLAN_CVID_CPOY_FROM_SP2C is not supported in fwdEninge!");
									}
									break;
								case ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID: //downstream only
									{
										WARNING("APOLLOFE_ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID is not supported in fwdEninge!");
									}
									break;

								default:
									break;

							}


							//CPRI decision
							switch(pPktHdr->aclDecision.action_cvlan.cvlanCpriDecision)
							{

								case ACL_CVLAN_CPRI_NOP:
									//do nothing
									break;
								case ACL_CVLAN_CPRI_ASSIGN:
									{
										pPktHdr->egressPriority = pPktHdr->aclDecision.action_cvlan.assignedCpri;
										TRACE("Modify by RG_CF ACT:CPRI_ASSIGN CPRI=%d",pPktHdr->egressPriority);
									}
									break;
								case ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG:
									{
										if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
											pPktHdr->egressPriority = pPktHdr->stagPri;
											TRACE("Modify by RG_CF ACT:CPRI_COPY_FROM_1ST_TAG(S) CPRI=%d",pPktHdr->egressPriority);
										}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
											pPktHdr->egressPriority = pPktHdr->ctagPri;
											TRACE("Modify by RG_CF ACT:CPRI_COPY_FROM_1ST_TAG(C) CPRI=%d",pPktHdr->egressPriority);
										}else{
											//no 1st tag, use assignedCpri as H/W
											pPktHdr->egressPriority = pPktHdr->aclDecision.action_cvlan.assignedCpri;
											TRACE("Modify by RG_CF ACT:CPRI_COPY_FROM_1ST_TAG(none tag) CPRI=%d",pPktHdr->egressPriority);
										}
									}
									break;
								case ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG:
									{
										if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
											pPktHdr->egressVlanID = pPktHdr->ctagVid;
											pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
											TRACE("Modify by RG_CF ACT:CVID_COPY_FROM_2ND_TAG CVID=%d",pPktHdr->egressVlanID);
										}else{
											//no 2nd tag, use assignedCpri as H/W
											pPktHdr->egressPriority = pPktHdr->aclDecision.action_cvlan.assignedCpri;
											TRACE("Modify by RG_CF ACT:CPRI_COPY_FROM_1ST_TAG(none tag) CPRI=%d",pPktHdr->egressPriority);
										}
									}
									break;
								case ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI:
									{
										pPktHdr->egressPriority = pPktHdr->internalPriority;
										TRACE("Modify by RG_CF ACT:CPRI_COPY_FROM_INTERNAL CPRI=%d",pPktHdr->egressPriority);
									}
									break;
								case ACL_CVLAN_CPRI_COPY_FROM_DSCP_REMAP:
									{
										WARNING("ACL_CVLAN_CPRI_COPY_FROM_DSCP_REMAP is not supported in fwdEninge!");
									}
									break;
								case ACL_CVLAN_CPRI_COPY_FROM_SP2C:
									{
										WARNING("ACL_CVLAN_CPRI_COPY_FROM_SP2C is not supported in fwdEninge!");
									}
									break; //downstream only

								default:
									break;
							}
						}
						break;

					case ACL_CVLAN_TAGIF_UNTAG:
						{
							pPktHdr->egressVlanTagif = 0;
							pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
							TRACE("Modify by RG_CF ACT:UN-CTAG");
						}break;

					case ACL_CVLAN_TAGIF_TRANSPARENT:
						{
							pPktHdr->dmac2VlanID = FAIL;//avoid Dam2cvid change the cvlan
							if(pPktHdr->tagif & CVLAN_TAGIF){
								pPktHdr->egressVlanTagif = 1;
								pPktHdr->egressVlanID = pPktHdr->ctagVid;
								pPktHdr->egressPriority = pPktHdr->ctagPri;
							}else{//untag
								pPktHdr->egressVlanTagif = 0;
							}

						}
						TRACE("Modify by RG_CF ACT:CTAG TRANSPARENT");
						break;

					default:
						pPktHdr->flow_cvid_with_egress_filter = TRUE;
						break;
				}

			}

			//SID action
			if(pPktHdr->aclDecision.us_action_field & CF_US_ACTION_SID_BIT)
			{
#ifdef CONFIG_GPON_FEATURE
				switch(pPktHdr->aclDecision.action_sid_or_llid.sidDecision)
				{
					case ACL_SID_LLID_ASSIGN:
						pPktHdr->streamID = pPktHdr->aclDecision.action_sid_or_llid.assignedSid_or_llid;
						TRACE("Modify by RG_CF ACT:StreamID SID=%d",pPktHdr->streamID);
						break;
					case ACL_SID_LLID_NOP:
						break;
				}
#endif
			}

			//CFPRI ACT
			if(pPktHdr->aclDecision.us_action_field & CF_US_ACTION_CFPRI_BIT)
			{
				switch( pPktHdr->aclDecision.action_cfpri.cfPriDecision)
				{
					case ACL_CFPRI_ASSIGN:
						{
							pPktHdr->internalPriority= pPktHdr->aclDecision.action_cfpri.assignedCfPri;
							TRACE("Modify by RG_CF ACT:CFPRI cfpri=%d",pPktHdr->internalPriority);
						}
						break;
					case ACL_CFPRI_NOP:
						break;
				}
			}
			//DSCP ACT
			if(pPktHdr->aclDecision.us_action_field & CF_US_ACTION_DSCP_BIT)
			{
				//[FIXME]
				WARNING("CF_US_ACTION_DSCP_BIT is not supported in fwdEninge!");
			}
			//FWD ACT
			if(pPktHdr->aclDecision.us_action_field & CF_US_ACTION_FWD_BIT)
			{
				//[FIXME]
				WARNING("CF_US_ACTION_FWD_BIT is not supported in fwdEninge!");
			}
		}
		else //downstream
		{
			//check DS action
			if((pPktHdr->aclDecision.ds_action_field&CF_DS_ACTION_STAG_BIT)>0)
			{
				switch(pPktHdr->aclDecision.action_svlan.svlanTagIfDecision){

					case ACL_SVLAN_TAGIF_NOP:
						//do nothing
						break;

					case ACL_SVLAN_TAGIF_UNTAG:
						{
							pPktHdr->egressServiceVlanTagif = 0;
							TRACE("Modify by RG_CF ACT:UN-STAG");
						}
						break;

					case ACL_SVLAN_TAGIF_TRANSPARENT:
						{
							if(pPktHdr->tagif & SVLAN_TAGIF){
								pPktHdr->egressServiceVlanTagif = 1;
								pPktHdr->egressServiceVlanID = pPktHdr->stagVid;
								pPktHdr->egressServicePriority = pPktHdr->stagPri;
							}else{//untag
								pPktHdr->egressServiceVlanTagif = 0;
							}
							TRACE("Modify by RG_CF ACT:STAG TRANSPARENT");
						}
						break;

					case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID:
						pPktHdr->egressServiceVlanTagif = 1; //force tagging with tpid
						TRACE("Modify by RG_CF ACT:SVLANTAG with TPID");
						goto cfDssvidSpriDecision;
					case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2:
						pPktHdr->egressServiceVlanTagif = 2; //force tagging with tpid2
						TRACE("Modify by RG_CF ACT:SVLANTAG with TPID2");
						goto cfDssvidSpriDecision;
					case ACL_SVLAN_TAGIF_TAGGING_WITH_ORIGINAL_STAG_TPID:
						pPktHdr->egressServiceVlanTagif = 3; //force tagging with original-stag-tpid
						TRACE("Modify by RG_CF ACT:SVLANTAG with TPID2");
						goto cfDssvidSpriDecision;

cfDssvidSpriDecision:
					//SVID decision
					switch(pPktHdr->aclDecision.action_svlan.svlanSvidDecision)
					{
						case ACL_SVLAN_SVID_NOP:
							//do nothing
							break;
						case ACL_SVLAN_SVID_ASSIGN:
							{
								pPktHdr->egressServiceVlanID = pPktHdr->aclDecision.action_svlan.assignedSvid;
								TRACE("Modify by RG_CF ACT:SVID_ASSIGN SVID=%d",pPktHdr->egressServiceVlanID);
							}
							break;
						case ACL_SVLAN_SVID_COPY_FROM_1ST_TAG:
							{
								if(pPktHdr->tagif & SVLAN_TAGIF){
									pPktHdr->egressServiceVlanID = pPktHdr->stagVid;
									TRACE("Modify by RG_CF ACT:SVID_COPY_FROM_1ST_TAG(S) SVID=%d",pPktHdr->egressServiceVlanID);
								}else if (pPktHdr->tagif & CVLAN_TAGIF){
									pPktHdr->egressServiceVlanID = pPktHdr->ctagVid;
									TRACE("Modify by RG_CF ACT:SVID_COPY_FROM_1ST_TAG(C) SVID=%d",pPktHdr->egressServiceVlanID);
								}else{
									//no 1st tag, use assignedSvid as H/W
									pPktHdr->egressServiceVlanID = pPktHdr->aclDecision.action_svlan.assignedSvid;
									TRACE("Modify by RG_CF ACT:SVID_COPY_FROM_1ST_TAG(none tag) SVID=%d",pPktHdr->egressServiceVlanID);
								}
							}
							break;
						case  ACL_SVLAN_SVID_COPY_FROM_2ND_TAG:
							{
								if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){
									pPktHdr->egressServiceVlanID = pPktHdr->ctagVid;
									TRACE("Modify by RG_CF ACT:SVID_COPY_FROM_2ND_TAG SVID=%d",pPktHdr->egressServiceVlanID);
								}else{
									//no 2nd tag, use assignedSvid as H/W
									pPktHdr->egressServiceVlanID = pPktHdr->aclDecision.action_svlan.assignedSvid;
									TRACE("Modify by RG_CF ACT:SVID_COPY_FROM_2ND_TAG(none tag) SVID=%d",pPktHdr->egressServiceVlanID);
								}
							}
							break;
						case ACL_SVLAN_SVID_SP2C: //downstream only
							{
								WARNING("ACL_SVLAN_SVID_SP2C is not supported in fwdEninge!");
							}
							break;
						default:
							break;
					}

					switch(pPktHdr->aclDecision.action_svlan.svlanSpriDecision)
					{
						case  ACL_SVLAN_SPRI_NOP:
							//do nothing
							break;
						case ACL_SVLAN_SPRI_ASSIGN:
							{
								pPktHdr->egressServicePriority = pPktHdr->aclDecision.action_svlan.assignedSpri;
								TRACE("Modify by RG_CF ACT:SPRI_ASSIGN SPRI=%d",pPktHdr->egressServicePriority);
							}
							break;
						case  ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG:
							{
								if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
									pPktHdr->egressServicePriority = pPktHdr->stagPri;
									TRACE("Modify by RG_CF ACT:SPRI_COPY_FROM_1ST_TAG(S) SPRI=%d",pPktHdr->egressServicePriority);
								}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
									pPktHdr->egressServicePriority = pPktHdr->ctagPri;
									TRACE("Modify by RG_CF ACT:SPRI_COPY_FROM_1ST_TAG(C) SPRI=%d",pPktHdr->egressServicePriority);
								}else{
									//no 1st tag, use assignedSpri as H/W
									pPktHdr->egressServicePriority = pPktHdr->aclDecision.action_svlan.assignedSpri;
									TRACE("Modify by RG_CF ACT:SPRI_COPY_FROM_1ST_TAG(none tag) SPRI=%d",pPktHdr->egressServicePriority);
								}
							}
							break;
						case ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG:
							{
								if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
									pPktHdr->egressServicePriority = pPktHdr->ctagPri;
									TRACE("Modify by RG_CF ACT:SPRI_COPY_FROM_2ND_TAG SPRI=%d",pPktHdr->egressServicePriority);
								}else{
									//no 2nd tag, use assignedSpri as H/W
									pPktHdr->egressServicePriority = pPktHdr->aclDecision.action_svlan.assignedSpri;
									TRACE("Modify by RG_CF ACT:SPRI_COPY_FROM_2ND_TAG(none tag) SPRI=%d",pPktHdr->egressServicePriority);
								}

							}
							break;
						case ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI:
							{
								pPktHdr->egressServicePriority = pPktHdr->internalPriority;
								TRACE("Modify by RG_CF ACT:SPRI_COPY_FROM_INTERNAL_PRI SPRI=%d",pPktHdr->egressServicePriority);
							}
							break;
						case ACL_SVLAN_SPRI_COPY_FROM_DSCP_REMAP: //downstream only
							{
								WARNING("ACL_SVLAN_SPRI_COPY_FROM_DSCP_REMAP is not supported in fwdEninge!");
							}
							break;
						case ACL_SVLAN_SPRI_COPY_FROM_SP2C: //downstream only
							{
								WARNING("ACL_SVLAN_SPRI_COPY_FROM_SP2C is not supported in fwdEninge!");
							}
							break;

						default:
							break;
					}

					default:
						break;
				}
			}

			if((pPktHdr->aclDecision.ds_action_field&CF_DS_ACTION_CTAG_BIT)>0)
			{
				pPktHdr->flow_cvid_with_egress_filter = FALSE;
				switch(pPktHdr->aclDecision.action_cvlan.cvlanTagIfDecision){
					case ACL_CVLAN_TAGIF_NOP:
						/*do nothing, follow switch-core*/
						break;

					case ACL_CVLAN_TAGIF_TAGGING:
						{
							/*force tagging*/
							pPktHdr->egressVlanTagif = 1;

							//CVDI decision
							switch(pPktHdr->aclDecision.action_cvlan.cvlanCvidDecision)
							{
								case ACL_CVLAN_CVID_NOP:
									//do nothing
									break;
								case ACL_CVLAN_CVID_ASSIGN:
									{
										pPktHdr->egressVlanID = pPktHdr->aclDecision.action_cvlan.assignedCvid;
										pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
										TRACE("Modify by RG_CF ACT:CVID_ASSIGN CVID=%d",pPktHdr->egressVlanID);
									}
									break;
								case ACL_CVLAN_CVID_COPY_FROM_1ST_TAG:
									{
										if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
											pPktHdr->egressVlanID = pPktHdr->stagVid;
											pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
											TRACE("Modify by RG_CF ACT:CVID_COPY_FROM_1ST_TAG(S) CVID=%d",pPktHdr->egressVlanID);
										}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
											pPktHdr->egressVlanID = pPktHdr->ctagVid;
											pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
											TRACE("Modify by RG_CF ACT:CVID_COPY_FROM_1ST_TAG(C) CVID=%d",pPktHdr->egressVlanID);
										}else{
											//no 1st tag, use assignedCvid as H/W
											pPktHdr->egressVlanID = pPktHdr->aclDecision.action_cvlan.assignedCvid;
											pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
											TRACE("Modify by RG_CF ACT:CVID_COPY_FROM_1ST_TAG(none tag) CVID=%d",pPktHdr->egressVlanID);
										}
									}
									break;
								case ACL_CVLAN_CVID_COPY_FROM_2ND_TAG:
									{
										if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
											pPktHdr->egressVlanID = pPktHdr->ctagVid;
											pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
											TRACE("Modify by RG_CF ACT:CVID_COPY_FROM_2ND_TAG CVID=%d",pPktHdr->egressVlanID);
										}else{
											//no 2nd tag, use assignedCvid as H/W
											pPktHdr->egressVlanID = pPktHdr->aclDecision.action_cvlan.assignedCvid;
											pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
											TRACE("Modify by RG_CF ACT:CVID_COPY_FROM_2ND_TAG(none tag) CVID=%d",pPktHdr->egressVlanID);
										}
									}
									break;
								case ACL_CVLAN_CVID_COPY_FROM_INTERNAL_VID://upstream only
									{
										pPktHdr->egressVlanID = pPktHdr->internalVlanID;
										pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
										TRACE("Modify by RG_CF ACT:CVID_COPY_FROM_INTERNAL_VID CVID=%d",pPktHdr->egressVlanID);
									}
									break;
								case ACL_CVLAN_CVID_CPOY_FROM_SP2C: //downstream only
									{
										WARNING("APOLLOFE_ACL_CVLAN_CVID_CPOY_FROM_SP2C is not supported in fwdEninge!");
									}
									break;
								case ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID: //downstream only
									{
										WARNING("APOLLOFE_ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID is not supported in fwdEninge!");
									}
									break;

								default:
									break;

							}


							//CPRI decision
							switch(pPktHdr->aclDecision.action_cvlan.cvlanCpriDecision)
							{

								case ACL_CVLAN_CPRI_NOP:
									//do nothing
									break;
								case ACL_CVLAN_CPRI_ASSIGN:
									{
										pPktHdr->egressPriority = pPktHdr->aclDecision.action_cvlan.assignedCpri;
										TRACE("Modify by RG_CF ACT:CPRI_ASSIGN CPRI=%d",pPktHdr->egressPriority);
									}
									break;
								case ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG:
									{
										if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
											pPktHdr->egressPriority = pPktHdr->stagPri;
											TRACE("Modify by RG_CF ACT:CPRI_COPY_FROM_1ST_TAG(S) CPRI=%d",pPktHdr->egressPriority);
										}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
											pPktHdr->egressPriority = pPktHdr->ctagPri;
											TRACE("Modify by RG_CF ACT:CPRI_COPY_FROM_1ST_TAG(C) CPRI=%d",pPktHdr->egressPriority);
										}else{
											//no 1st tag, use assignedCpri as H/W
											pPktHdr->egressPriority = pPktHdr->aclDecision.action_cvlan.assignedCpri;
											TRACE("Modify by RG_CF ACT:CPRI_COPY_FROM_1ST_TAG(none tag) CPRI=%d",pPktHdr->egressPriority);
										}
									}
									break;
								case ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG:
									{
										if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
											pPktHdr->egressVlanID = pPktHdr->ctagVid;
											pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
											TRACE("Modify by RG_CF ACT:CVID_COPY_FROM_2ND_TAG CVID=%d",pPktHdr->egressVlanID);
										}else{
											//no 2nd tag, use assignedCpri as H/W
											pPktHdr->egressPriority = pPktHdr->aclDecision.action_cvlan.assignedCpri;
											TRACE("Modify by RG_CF ACT:CPRI_COPY_FROM_1ST_TAG(none tag) CPRI=%d",pPktHdr->egressPriority);
										}
									}
									break;
								case ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI:
									{
										pPktHdr->egressPriority = pPktHdr->internalPriority;
										TRACE("Modify by RG_CF ACT:CPRI_COPY_FROM_INTERNAL CPRI=%d",pPktHdr->egressPriority);
									}
									break;
								case ACL_CVLAN_CPRI_COPY_FROM_DSCP_REMAP:
									{
										WARNING("APOLLOFE_ACL_CVLAN_CPRI_COPY_FROM_DSCP_REMAP is not supported in fwdEninge!");
									}
									break;
								case ACL_CVLAN_CPRI_COPY_FROM_SP2C:
									{
										WARNING("APOLLOFE_ACL_CVLAN_CPRI_COPY_FROM_SP2C is not supported in fwdEninge!");
									}
									break; //downstream only

								default:
									break;
							}
						}
						break;

					case ACL_CVLAN_TAGIF_UNTAG:
						{
							pPktHdr->egressVlanTagif = 0;
							pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
							TRACE("Modify by RG_CF ACT:UN-CTAG");
						}break;

					case ACL_CVLAN_TAGIF_TRANSPARENT:
						{
							pPktHdr->dmac2VlanID = FAIL;//avoid Dam2cvid change the cvlan
							if(pPktHdr->tagif & CVLAN_TAGIF){
								pPktHdr->egressVlanTagif = 1;
								pPktHdr->egressVlanID = pPktHdr->ctagVid;
								pPktHdr->egressPriority = pPktHdr->ctagPri;
							}else{//untag
								pPktHdr->egressVlanTagif = 0;
							}

						}
						TRACE("Modify by RG_CF ACT:CTAG TRANSPARENT");
						break;

					default:
						pPktHdr->flow_cvid_with_egress_filter = TRUE;
						break;
				}

			}


			if((pPktHdr->aclDecision.ds_action_field&CF_DS_ACTION_UNI_MASK_BIT)>0)
			{
				switch(pPktHdr->aclDecision.action_uni.uniActionDecision){
					case ACL_UNI_FWD_TO_PORTMASK_ONLY:
						{
							pPktHdr->egressUniPortmask = ((1<<pPktHdr->egressMacPort) & pPktHdr->aclDecision.action_uni.assignedUniPortMask);
							ACL(" RG_CF egressMacPort %d filtered with egressPmsk 0x%x",pPktHdr->egressMacPort,pPktHdr->aclDecision.action_uni.assignedUniPortMask);
							TRACE("Modify by RG_CF ACT:UNI_FWD_TO_PORTMASK_ONLY finalPortMask=0x%x",pPktHdr->egressUniPortmask);

							if(pPktHdr->egressUniPortmask==0x0){
								ACL("Drop! because egressPmsk is change to 0x0 by UNI action");
								TRACE("[Drop] Modify by RG_CF ACT: Drop! finalPortMask is 0x0");
								return RG_FWDENGINE_RET_DROP;
							}

						}
						break;
					case ACL_UNI_FORCE_BY_MASK:
						{
							pPktHdr->egressUniPortmask = pPktHdr->aclDecision.action_uni.assignedUniPortMask;
							ACL(" RG_CF force egressPmsk change to 0x%x",pPktHdr->aclDecision.action_uni.assignedUniPortMask);
							TRACE("Modify by RG_CF ACT:UNI_FORCE_BY_MASK finalPortMask=0x%x",pPktHdr->egressUniPortmask);

							if(pPktHdr->egressUniPortmask==0x0){
								ACL("Drop! because egressPmsk is change to 0x0 by UNI action");
								TRACE("[Drop] Modify by RG_CF ACT: Drop! finalPortMask is 0x0");
								return RG_FWDENGINE_RET_DROP;
							}else{
								//FB-flow5 can not support egress to multi-port
								WARNING("ACL_UNI_FORCE_BY_MASK support egress to multi-port action, only support for drop purpose(egressUniPortmask=0x0)!");
							}

						}
						break;

					case ACL_UNI_TRAP_TO_CPU:
						{
							WARNING("ACL_UNI_TRAP_TO_CPU is not supported in fwdEngine.");
						}
						break;
					case AL_UNI_NOP:
						break;

					default:
						break;

				}
			}

			//CFPRI ACT
			if((pPktHdr->aclDecision.ds_action_field&CF_DS_ACTION_CFPRI_BIT)>0)
			{
				switch( pPktHdr->aclDecision.action_cfpri.cfPriDecision)
				{
					case ACL_CFPRI_ASSIGN:
						{
							pPktHdr->internalPriority= pPktHdr->aclDecision.action_cfpri.assignedCfPri;
							TRACE("Modify by RG_CF ACT:CFPRI cfpri=%d",pPktHdr->internalPriority);
						}
						break;
					case ACL_CFPRI_NOP:
						break;
				}
			}

			//DSCP ACT
			if((pPktHdr->aclDecision.ds_action_field&CF_DS_ACTION_DSCP_BIT)>0)
			{
				//[FIXME]
				WARNING("CF_DS_ACTION_DSCP_BIT is not supported in fwdEninge!");
			}
		}
	}


	return RG_FWDENGINE_RET_CONTINUE;
}




int _rtk_rg_acl_egressIntfIdx_precheck(rtk_rg_pktHdr_t *pPktHdr){WARNING("Not implement yet!"); return RT_ERR_RG_OK;}


/*(6)debug tool APIs*/
int _dump_rg_acl(struct seq_file *s)
{
	int i;
	rtk_rg_aclFilterAndQos_t *aclPara;

	PROC_PRINTF("acl_SW_table_entry_size:%d\n",rg_db.systemGlobal.acl_SW_table_entry_size);

#if 1
	PROC_PRINTF("aclSW rule index sorting:\n");
	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){
		if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i]==-1)
			break;
		PROC_PRINTF("ACL[%d]:w(%d)",rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i],
			rg_db.systemGlobal.acl_SW_table_entry[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i])].acl_filter.acl_weight);
		if(i+1!=MAX_ACL_SW_ENTRY_SIZE && rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i+1]>=0)
			PROC_PRINTF(" > ");
	}
	PROC_PRINTF("\n");
	if(rg_db.systemGlobal.stop_add_hw_acl>0)
		PROC_PRINTF("Stop rest ACL rule add to H/W ACL from index %d.\n", rg_db.systemGlobal.stop_add_hw_acl-1);
#endif

	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){
		//if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields!=0x0){
		//DEBUG("aclSWEntry[i],valid = %d",i,rg_db.systemGlobal.acl_SW_table_entry[i].valid);
		if(rg_db.systemGlobal.acl_SW_table_entry[i].valid==RTK_RG_ENABLED){
			aclPara = &(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter);
			PROC_PRINTF("========================RG_ACL[%d]===========================\n",i);
#if 1	//no need to use HW ACL
			PROC_PRINTF("[hw_acl_index:%s(port:0x%x, total:%d)] \n"
				,rg_db.systemGlobal.acl_SW_table_entry[i].hw_aclEntry_index
				,rg_db.systemGlobal.acl_SW_table_entry[i].hw_aclEntry_port.bits[0]
				,rg_db.systemGlobal.acl_SW_table_entry[i].hw_aclEntry_count);
#endif
			_dump_rg_acl_entry_content(s, aclPara);

		}

	}

	return RT_ERR_RG_OK;
}



int _dump_rg_cf(struct seq_file *s)
{
	int i;
	rtk_rg_classifyEntry_t *classifyFilter;

	for(i=0;i<TOTAL_CF_ENTRY_SIZE;i++){

		classifyFilter = &(rg_db.systemGlobal.classify_SW_table_entry[i]);

		if(classifyFilter->index==FAIL)
			continue;


		PROC_PRINTF("=============CF[%d]============= \n",classifyFilter->index);
		PROC_PRINTF("Direction: %s \n",classifyFilter->direction?"downstream":"upstream");
		PROC_PRINTF("Patterns: \n");
		if(classifyFilter->filter_fields & EGRESS_ETHERTYPR_BIT)
			PROC_PRINTF("etherType:0x%x (mask=0x%x)\n", classifyFilter->etherType,classifyFilter->etherType_mask);
		if(classifyFilter->filter_fields & EGRESS_GEMIDX_BIT)
			PROC_PRINTF("gemidx:%d (mask=0x%x)\n ", classifyFilter->gemidx,classifyFilter->gemidx_mask);
		if(classifyFilter->filter_fields & EGRESS_LLID_BIT)
			PROC_PRINTF("llid:%d\n", classifyFilter->llid);
		if(classifyFilter->filter_fields & EGRESS_TAGVID_BIT)
			PROC_PRINTF("outterTagVid:%d\n", classifyFilter->outterTagVid);
		if(classifyFilter->filter_fields & EGRESS_TAGPRI_BIT)
			PROC_PRINTF("outterTagPri:%d\n", classifyFilter->outterTagPri);
		if(classifyFilter->filter_fields & EGRESS_INTERNALPRI_BIT)
			PROC_PRINTF("internalPri:%d\n", classifyFilter->internalPri);
		if(classifyFilter->filter_fields & EGRESS_STAGIF_BIT)
			PROC_PRINTF("stagIf:%s\n", classifyFilter->stagIf?"Must Tag":"Must unTag");
		if(classifyFilter->filter_fields & EGRESS_CTAGIF_BIT)
			PROC_PRINTF("ctagIf:%s\n", classifyFilter->ctagIf?"Must Tag":"Must unTag");
		if(classifyFilter->filter_fields & EGRESS_UNI_BIT)
			PROC_PRINTF("uni:%d (mask=0x%x)\n", classifyFilter->uni,classifyFilter->uni_mask);

		PROC_PRINTF("Actions: \n");

		if(classifyFilter->direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM){
			PROC_PRINTF("US_ACTION_MASKBIT=0x%x \n",classifyFilter->us_action_field);


			if(classifyFilter->us_action_field&CF_US_ACTION_STAG_BIT)
			{
				PROC_PRINTF("[SVLAN] svlan: %s\n  svid_act:%s\n  spri_act:%s\n	svid:%d\n  spri:%d\n",
					name_of_rg_svlan_tagif_decision[classifyFilter->action_svlan.svlanTagIfDecision],
					name_of_rg_svlan_svid_decision[classifyFilter->action_svlan.svlanSvidDecision],
					name_of_rg_svlan_spri_decision[classifyFilter->action_svlan.svlanSpriDecision],
					classifyFilter->action_svlan.assignedSvid,
					classifyFilter->action_svlan.assignedSpri
				);
			}
			if(classifyFilter->us_action_field&CF_US_ACTION_CTAG_BIT)
			{
				PROC_PRINTF("[CVLAN] cvlan: %s\n  cvid_act:%s\n  cpri_act:%s\n	cvid:%d\n  cpri:%d\n",
					name_of_rg_cvlan_tagif_decision[classifyFilter->action_cvlan.cvlanTagIfDecision],
					name_of_rg_cvlan_cvid_decision[classifyFilter->action_cvlan.cvlanCvidDecision],
					name_of_rg_cvlan_cpri_decision[classifyFilter->action_cvlan.cvlanCpriDecision],
					classifyFilter->action_cvlan.assignedCvid,
					classifyFilter->action_cvlan.assignedCpri);
			}
			if(classifyFilter->us_action_field&CF_US_ACTION_CFPRI_BIT)
			{
				PROC_PRINTF("[CFPRI] act:%s cfpri:%d \n",name_of_rg_cfpri_decision[classifyFilter->action_cfpri.cfPriDecision],classifyFilter->action_cfpri.assignedCfPri);
			}
			if(classifyFilter->us_action_field&CF_US_ACTION_DSCP_BIT)
			{
				PROC_PRINTF("[DSCP] act:%s dscp:%d \n",name_of_rg_dscp_decision[classifyFilter->action_dscp.dscpDecision],classifyFilter->action_dscp.assignedDscp);
			}
			if(classifyFilter->us_action_field&CF_US_ACTION_SID_BIT)
			{
				PROC_PRINTF("[SID] act:%s sid:%d \n",name_of_rg_sid_decision[classifyFilter->action_sid_or_llid.sidDecision],classifyFilter->action_sid_or_llid.assignedSid_or_llid);
			}
			if(classifyFilter->us_action_field&CF_US_ACTION_FWD_BIT)
			{
				PROC_PRINTF("[FWD] act:%s \n",name_of_rg_fwd_decision[classifyFilter->action_fwd.fwdDecision]);
			}
			if(classifyFilter->us_action_field&CF_US_ACTION_DROP_BIT)
			{
				PROC_PRINTF("[DROP] act: DROP\n");
			}

		}else if(classifyFilter->direction==RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM){
			PROC_PRINTF("DS_ACTION_MASKBIT=0x%x \n",classifyFilter->ds_action_field);

			if(classifyFilter->ds_action_field&CF_DS_ACTION_STAG_BIT)
			{
				PROC_PRINTF("[SVLAN] svlan: %s\n  svid_act:%s\n  spri_act:%s\n	svid:%d\n  spri:%d\n",
					name_of_rg_svlan_tagif_decision[classifyFilter->action_svlan.svlanTagIfDecision],
					name_of_rg_svlan_svid_decision[classifyFilter->action_svlan.svlanSvidDecision],
					name_of_rg_svlan_spri_decision[classifyFilter->action_svlan.svlanSpriDecision],
					classifyFilter->action_svlan.assignedSvid,
					classifyFilter->action_svlan.assignedSpri
				);
			}
			if(classifyFilter->ds_action_field&CF_DS_ACTION_CTAG_BIT)
			{
				PROC_PRINTF("[CVLAN] cvlan: %s\n  cvid_act:%s\n  cpri_act:%s\n	cvid:%d\n  cpri:%d\n",
					name_of_rg_cvlan_tagif_decision[classifyFilter->action_cvlan.cvlanTagIfDecision],
					name_of_rg_cvlan_cvid_decision[classifyFilter->action_cvlan.cvlanCvidDecision],
					name_of_rg_cvlan_cpri_decision[classifyFilter->action_cvlan.cvlanCpriDecision],
					classifyFilter->action_cvlan.assignedCvid,
					classifyFilter->action_cvlan.assignedCpri);
			}
			if(classifyFilter->ds_action_field&CF_DS_ACTION_CFPRI_BIT)
			{
				PROC_PRINTF("[CFPRI] act:%s cfpri:%d \n",name_of_rg_cfpri_decision[classifyFilter->action_cfpri.cfPriDecision],classifyFilter->action_cfpri.assignedCfPri);
			}
			if(classifyFilter->ds_action_field&CF_DS_ACTION_DSCP_BIT)
			{
				PROC_PRINTF("[DSCP] act:%s dscp:%d \n",name_of_rg_dscp_decision[classifyFilter->action_dscp.dscpDecision],classifyFilter->action_dscp.assignedDscp);
			}
			if(classifyFilter->ds_action_field&CF_DS_ACTION_UNI_MASK_BIT)
			{
				PROC_PRINTF("[UNI] act: %s, portmask=0x%x \n",name_of_rg_uni_decision[classifyFilter->action_uni.uniActionDecision],classifyFilter->action_uni.assignedUniPortMask);
			}
			if(classifyFilter->ds_action_field&CF_DS_ACTION_DROP_BIT)
			{
				PROC_PRINTF("[DROP] act: DROP\n");
			}

		}
	}
	return RT_ERR_RG_OK;
}

int _dump_rg_acl_and_cf_diagshell(struct seq_file *s)
{
#if 1
	int i;
	uint8	unknownCmdFlag=0;
	int tagDecision=0, vidDecision=0, priDecision=0;//ther value should reference to rg_acl.cli

	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){
		//rtk_rg_acl_filter_fields_e check 1st
		if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields!=0x0){ 	//no more rules need to check
		//uint8 setFlag=0;
		//char tmpStr[255];
		//memset(tmpStr,0,255);

	PROC_PRINTF("rg clear acl-filter\n");
				if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.acl_weight!=0)	//not shown while zero
	PROC_PRINTF("rg set acl-filter acl_weight %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.acl_weight);

	PROC_PRINTF("rg set acl-filter fwding_type_and_direction %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction);

	//action
	PROC_PRINTF("rg set acl-filter action action_type %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_type);
	if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_type==ACL_ACTION_TYPE_POLICY_ROUTE)
	PROC_PRINTF("rg set acl-filter action policy-route egress_intf_idx %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_policy_route_wan);

	if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_type==ACL_ACTION_TYPE_FLOW_MIB)
	PROC_PRINTF("rg set acl-filter action flow-mib flow_counter_idx %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_flowmib_counter_idx);

	if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_type==ACL_ACTION_TYPE_SW_MIRROR_WITH_UDP_ENCAP) {
		if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.no_encap){
			PROC_PRINTF("rg set acl-filter action udp_encap no_encap 1 mirror_count %d",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.mirror_count);
			PROC_PRINTF("%s\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.remake_smac?" remake_smac":"");
		}else if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.mirror_count==FAIL){
			if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_ip_ipv6){
				PROC_PRINTF("rg set acl-filter action udp_encap assign_smac %x:%x:%x:%x:%x:%x assign_dmac %x:%x:%x:%x:%x:%x assign_sip_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x assign_dip_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x assign_dport %d\n",
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[5],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[5],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[5],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[6],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[7],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[8],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[9],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[10],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[11],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[12],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[13],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[14],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[15],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[5],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[6],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[7],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[8],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[9],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[10],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[11],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[12],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[13],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[14],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[15],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dport);
			}else{
				PROC_PRINTF("rg set acl-filter action udp_encap assign_smac %x:%x:%x:%x:%x:%x assign_dmac %x:%x:%x:%x:%x:%x assign_sip %d.%d.%d.%d assign_dip %d.%d.%d.%d assign_dport %d \n",
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[5],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[5],
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip & 0xff000000)>>24,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip & 0xff0000)>>16,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip & 0xff00)>>8,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip & 0xff),
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip & 0xff000000)>>24,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip & 0xff0000)>>16,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip & 0xff00)>>8,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip & 0xff),
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dport);
			}
		}
		else
		{
			if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_ip_ipv6){
				PROC_PRINTF("rg set acl-filter action udp_encap assign_smac %x:%x:%x:%x:%x:%x assign_dmac %x:%x:%x:%x:%x:%x assign_sip_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x assign_dip_ipv6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x assign_dport %d mirror_count %d\n",
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[5],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[5],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[5],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[6],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[7],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[8],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[9],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[10],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[11],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[12],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[13],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[14],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip_ipv6[15],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[5],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[6],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[7],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[8],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[9],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[10],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[11],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[12],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[13],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[14],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip_ipv6[15],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dport,
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.mirror_count);
			}else{
				PROC_PRINTF("rg set acl-filter action udp_encap assign_smac %x:%x:%x:%x:%x:%x assign_dmac %x:%x:%x:%x:%x:%x assign_sip %d.%d.%d.%d assign_dip %d.%d.%d.%d assign_dport %d mirror_count %d\n",
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[5],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[5],
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip & 0xff000000)>>24,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip & 0xff0000)>>16,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip & 0xff00)>>8,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip & 0xff),
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip & 0xff000000)>>24,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip & 0xff0000)>>16,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip & 0xff00)>>8,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip & 0xff),
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dport,
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.mirror_count);
			}
		}
	}

	if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_type==ACL_ACTION_TYPE_TRAP_WITH_PRIORITY)
		PROC_PRINTF("rg set acl-filter action trap_with_priority acl_priority %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_trap_with_priority);

	if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_type==ACL_ACTION_TYPE_QOS)
	{

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_ACL_PRIORITY_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_acl_priority %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_priority);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_1P_REMARKING_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_dot1p_remarking_pri %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_dot1p_remarking_pri);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_DSCP_REMARKING_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_dscp_remarking_pri %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_dscp_remarking_pri);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_TOS_TC_REMARKING_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_tos_tc_remarking_pri %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_tos_tc_remarking_pri);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_ip_precedence_remarking_pri %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_ip_precedence_remarking_pri);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_QUEUE_ID_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_queue_id %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_queue_id);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_egress_internal_priority %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_internal_priority);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_STREAM_ID_OR_LLID_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_stream_id %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_stream_id_or_llid);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_ACL_INGRESS_VID_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_ingress_vid %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_ingress_vid);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_REDIRECT_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_redirect 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.redirect_portmask);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_DS_UNIMASK_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_downstream_uni_portmask portmask 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.downstream_uni_portmask);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_SHARE_METER_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_share_meter %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_share_meter);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_LOG_COUNTER_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_log_counter %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_log_counter);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_ACL_CVLANTAG_BIT)!=0)
		{
			//init varible
			unknownCmdFlag=0;
			tagDecision=-1;
			vidDecision=-1;
			priDecision=-1;

			switch(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_cvlan.cvlanTagIfDecision){
			case ACL_CVLAN_TAGIF_NOP: tagDecision=0; break;
			case ACL_CVLAN_TAGIF_TAGGING: tagDecision=1;break;
			case ACL_CVLAN_TAGIF_UNTAG: tagDecision=2;break;
			case ACL_CVLAN_TAGIF_TRANSPARENT: tagDecision=3;break;
			default:
				unknownCmdFlag =1;
				break;
			}

			switch(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_cvlan.cvlanCvidDecision)
			{
				case ACL_CVLAN_CVID_ASSIGN: vidDecision=1; break;
				case ACL_CVLAN_CVID_COPY_FROM_1ST_TAG: vidDecision=2;break;
				case ACL_CVLAN_CVID_COPY_FROM_2ND_TAG: vidDecision=3;break;
				case ACL_CVLAN_CVID_COPY_FROM_INTERNAL_VID: vidDecision=4;break;
				case ACL_CVLAN_CVID_NOP: vidDecision=0; break;
				default:
					unknownCmdFlag =1;
					break;
			}

			switch(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_cvlan.cvlanCpriDecision)
			{

				case ACL_CVLAN_CPRI_ASSIGN: priDecision=1; break;
				case ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG: priDecision=2; break;
				case ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG: priDecision=3; break;
				case ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI: priDecision=4;break;
				case ACL_CVLAN_CPRI_NOP: priDecision=0; break;
				default:
					unknownCmdFlag =1;
					break;
			}

			if(unknownCmdFlag == 1)
			{
				PROC_PRINTF("rg set acl-filter action qos action_ctag [unknown action_ctag command]\n");
			}
			else
			{
				PROC_PRINTF("rg set acl-filter action qos action_ctag tagDecision %d cvidDecision %d cpriDecision %d cvid %d cpri %d\n",
				tagDecision,vidDecision,priDecision,
				rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_cvlan.assignedCvid,
				rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_cvlan.assignedCpri);
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_ACL_SVLANTAG_BIT)!=0)
		{

			//init varible
			unknownCmdFlag=0;
			tagDecision=-1;
			vidDecision=-1;
			priDecision=-1;

			switch(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_svlan.svlanTagIfDecision){
				case ACL_SVLAN_TAGIF_NOP: tagDecision=0; break;
				case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID: tagDecision=1; break;
				case ACL_SVLAN_TAGIF_UNTAG: tagDecision=3;break;
				case ACL_SVLAN_TAGIF_TRANSPARENT: tagDecision=4;break;
				case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2: tagDecision=2;break;
				case ACL_SVLAN_TAGIF_TAGGING_WITH_ORIGINAL_STAG_TPID: tagDecision=5;break;
				default:
					unknownCmdFlag =1;
					break;
			}

			switch(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_svlan.svlanSvidDecision)
			{
				case ACL_SVLAN_SVID_ASSIGN: vidDecision=1; break;
				case ACL_SVLAN_SVID_COPY_FROM_1ST_TAG: vidDecision=2; break;
				case ACL_SVLAN_SVID_COPY_FROM_2ND_TAG: vidDecision=3; break;
				case ACL_SVLAN_SVID_NOP: vidDecision=0; break;
				default:
					unknownCmdFlag =1;
					break;
			}
			switch(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_svlan.svlanSpriDecision)
			{
				case ACL_SVLAN_SPRI_ASSIGN: priDecision=1; break;
				case ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG: priDecision=2; break;
				case ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG: priDecision=3; break;
				case ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI: priDecision=4; break;
				case ACL_SVLAN_SPRI_NOP: priDecision=0; break;
				default:
					unknownCmdFlag =1;
					break;
			}



			if(unknownCmdFlag == 1)
			{
				PROC_PRINTF("rg set acl-filter action qos action_ctag [unknown action_stag command]\n");
			}
			else
			{
				PROC_PRINTF("rg set acl-filter action qos action_stag tagDecision %d svidDecision %d spriDecision %d svid %d spri %d\n",
				tagDecision,vidDecision,priDecision,
				rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_svlan.assignedSvid,
				rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_svlan.assignedSpri);

			}
		}
	}


		//pattern: egress
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_CTAG_PRI_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_ctag_pri %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_ctag_pri);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& EGRESS_CTAG_PRI_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_ctag_pri \n");
			}
		}
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_CTAG_VID_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_ctag_vid %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_ctag_vid);
			PROC_PRINTF("rg set acl-filter pattern egress_ctag_vid_mask %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_ctag_vid_mask);

			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& EGRESS_CTAG_VID_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_ctag_vid \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_IPV4_DIP_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_dest_ipv4_addr_start %d.%d.%d.%d engress_dest_ipv4_addr_end %d.%d.%d.%d\n",
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_start & 0xff000000)>>24,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_start & 0xff0000)>>16,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_start & 0xff00)>>8,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_start & 0xff),
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_end & 0xff000000)>>24,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_end & 0xff0000)>>16,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_end & 0xff00)>>8,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_end & 0xff));

			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& EGRESS_IPV4_DIP_RANGE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_dest_ipv4_addr_range \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_INTF_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_intf_idx %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_intf_idx);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& EGRESS_INTF_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_intf_idx \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_EGRESS_PORTIDX_BIT)!=0)
		{

			if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP ||
				rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN ||
				rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP ||
				rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_PERMIT)
			{
				PROC_PRINTF("rg set acl-filter pattern egress_port_idx %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_port_idx);
				PROC_PRINTF("rg set acl-filter pattern egress_port_idx_mask 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_port_idx_mask);

				if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& INGRESS_EGRESS_PORTIDX_BIT)!=0)
				{
					PROC_PRINTF("rg set acl-filter pattern not egress_port_idx \n");
				}
			}
			else if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP||
				rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN ||
				rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP ||
				rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT)
			{
				PROC_PRINTF("rg set acl-filter pattern ingress_port_idx %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_port_idx);
				PROC_PRINTF("rg set acl-filter pattern ingress_port_idx_mask 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_port_idx_mask);
				if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& INGRESS_EGRESS_PORTIDX_BIT)!=0)
				{
					PROC_PRINTF("rg set acl-filter pattern not ingress_port_idx \n");
				}
			}
			else if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
			{
				PROC_PRINTF("rg set acl-filter pattern egress_port_idx %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_port_idx);
				PROC_PRINTF("rg set acl-filter pattern egress_port_idx_mask 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_port_idx_mask);

				if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& INGRESS_EGRESS_PORTIDX_BIT)!=0)
				{
					PROC_PRINTF("rg set acl-filter pattern not egress_port_idx \n");
				}
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_IPV4_SIP_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_src_ipv4_addr_start %d.%d.%d.%d egress_src_ipv4_addr_end %d.%d.%d.%d\n",
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_start & 0xff000000)>>24,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_start & 0xff0000)>>16,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_start & 0xff00)>>8,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_start & 0xff),
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_end & 0xff000000)>>24,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_end & 0xff0000)>>16,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_end & 0xff00)>>8,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_end & 0xff));

			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& EGRESS_IPV4_SIP_RANGE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_src_ipv4_addr_range \n");
			}
		}


		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_L4_SPORT_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_src_l4_port_start %d egress_src_l4_port_end %d\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_l4_port_start,
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_l4_port_end);

			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& EGRESS_L4_SPORT_RANGE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_src_l4_port_range \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_L4_DPORT_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_dest_l4_port_start %d egress_dest_l4_port_end %d\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_l4_port_start,
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_l4_port_end);

			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& EGRESS_L4_DPORT_RANGE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_dest_l4_port_range \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_IP4MC_IF)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_ip4mc_if %d\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_ip4mc_if);

			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& EGRESS_IP4MC_IF)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_ip4mc_if \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_IP6MC_IF)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_ip6mc_if %d\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_ip6mc_if);

			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& EGRESS_IP6MC_IF)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_ip6mc_if \n");
			}
		}

		//pattern: ingress
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV4_TAGIF_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ipv4_tagif %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ipv4_tagif);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& INGRESS_IPV4_TAGIF_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_ipv4_tagif \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_TAGIF_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ipv6_tagif %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ipv6_tagif);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& INGRESS_IPV6_TAGIF_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_ipv6_tagif \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_CTAG_PRI_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ctag_pri %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ctag_pri);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_CTAG_PRI_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_ctag_pri \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_CTAG_VID_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ctag_vid %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ctag_vid);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_CTAG_VID_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_ctag_vid \n");
			}
		}
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_CTAG_CFI_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ctag_cfi %d \n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ctag_cfi);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_CTAG_CFI_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_ctag_cfi \n");
			}
		}
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV4_DIP_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_dest_ipv4_addr_start %d.%d.%d.%d ingress_dest_ipv4_addr_end %d.%d.%d.%d\n",
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_start & 0xff000000)>>24,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_start & 0xff0000)>>16,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_start & 0xff00)>>8,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_start & 0xff),
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_end & 0xff000000)>>24,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_end & 0xff0000)>>16,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_end & 0xff00)>>8,
					(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_end & 0xff));

			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_IPV4_DIP_RANGE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_dest_ipv4_addr_range \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_DIP_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_dest_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[5],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[6],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[7],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[8],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[9],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[10],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[11],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[12],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[13],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[14],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[15]);


			PROC_PRINTF("rg set acl-filter pattern ingress_dest_ipv6_addr_mask %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[5],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[6],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[7],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[8],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[9],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[10],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[11],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[12],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[13],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[14],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[15]);

			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& INGRESS_IPV6_DIP_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_dest_ipv6_addr \n");
			}


		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_DIP_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_dest_ipv6_addr_start %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ingress_dest_ipv6_addr_end %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[5],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[6],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[7],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[8],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[9],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[10],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[11],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[12],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[13],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[14],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[15],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[5],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[6],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[7],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[8],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[9],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[10],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[11],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[12],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[13],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[14],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[15]);


			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_IPV6_DIP_RANGE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_dest_ipv6_addr_range \n");
			}

		}


		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_DMAC_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_dmac %x:%x:%x:%x:%x:%x\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac.octet[0],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac.octet[1],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac.octet[2],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac.octet[3],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac.octet[4],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac.octet[5]);

			PROC_PRINTF("rg set acl-filter pattern ingress_dmac_mask %x:%x:%x:%x:%x:%x\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[0],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[1],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[2],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[3],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[4],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[5]);

			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& INGRESS_DMAC_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_dmac \n");
			}
		}


		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_DSCP_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_dscp %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dscp);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& INGRESS_DSCP_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_dscp \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_ETHERTYPE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ethertype 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ethertype);
			PROC_PRINTF("rg set acl-filter pattern ingress_ethertype_mask 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ethertype_mask);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& INGRESS_ETHERTYPE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_ethertype \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_INTF_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_intf_idx %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_intf_idx);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& INGRESS_INTF_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_intf_idx \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_DSCP_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ipv6_dscp %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ipv6_dscp);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& INGRESS_IPV6_DSCP_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_ipv6_dscp \n");
			}
		}


		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_UDP_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_l4_protocal 0\n");
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_L4_UDP_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_l4_protocal \n");
			}
		}
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_TCP_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_l4_protocal 1\n");
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_L4_TCP_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_l4_protocal \n");
			}
		}
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_ICMP_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_l4_protocal 2\n");
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_L4_ICMP_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_l4_protocal \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_NONE_TCP_NONE_UDP_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_l4_protocal 3\n");
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_L4_NONE_TCP_NONE_UDP_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_l4_protocal \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_ICMPV6_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_l4_protocal 4\n");
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_L4_ICMPV6_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_l4_protocal \n");
			}
		}


		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_POROTCAL_VALUE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_l4_protocal_value 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_l4_protocal);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_L4_POROTCAL_VALUE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_l4_protocal_value \n");
			}
		}


		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_PORT_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_port_mask 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_port_mask.portmask);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_PORT_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_port_mask \n");
			}
		}


		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_SMAC_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_smac %x:%x:%x:%x:%x:%x\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac.octet[0],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac.octet[1],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac.octet[2],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac.octet[3],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac.octet[4],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac.octet[5]);

			PROC_PRINTF("rg set acl-filter pattern ingress_smac_mask %x:%x:%x:%x:%x:%x\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac_mask.octet[0],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac_mask.octet[1],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac_mask.octet[2],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac_mask.octet[3],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac_mask.octet[4],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac_mask.octet[5]);

			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_SMAC_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_smac \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV4_SIP_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_src_ipv4_addr_start %d.%d.%d.%d ingress_src_ipv4_addr_end %d.%d.%d.%d\n",
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_start & 0xff000000)>>24,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_start & 0xff0000)>>16,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_start & 0xff00)>>8,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_start & 0xff),
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_end & 0xff000000)>>24,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_end & 0xff0000)>>16,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_end & 0xff00)>>8,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_end & 0xff));

			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_IPV4_SIP_RANGE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_src_ipv4_addr_range \n");
			}
		}
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_SIP_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_src_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[5],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[6],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[7],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[8],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[9],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[10],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[11],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[12],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[13],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[14],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[15]);

			PROC_PRINTF("rg set acl-filter pattern ingress_src_ipv6_addr_mask %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[0],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[1],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[2],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[3],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[4],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[5],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[6],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[7],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[8],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[9],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[10],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[11],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[12],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[13],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[14],
					rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[15]);

			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_IPV6_SIP_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_src_ipv6_addr \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_SIP_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_src_ipv6_addr_start %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ingress_src_ipv6_addr_end %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[0],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[1],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[2],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[3],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[4],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[5],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[6],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[7],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[8],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[9],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[10],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[11],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[12],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[13],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[14],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[15],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[0],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[1],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[2],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[3],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[4],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[5],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[6],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[7],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[8],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[9],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[10],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[11],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[12],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[13],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[14],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[15]);

			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_IPV6_SIP_RANGE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_src_ipv6_addr_range \n");
			}
		}


		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_SPORT_RANGE_BIT)!=0)
		{		PROC_PRINTF("rg set acl-filter pattern ingress_src_l4_port_start %d ingress_src_l4_port_end %d\n",
				rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_l4_port_start,
				rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_l4_port_end);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_L4_SPORT_RANGE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_src_l4_port_range \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_DPORT_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_dest_l4_port_start %d ingress_dest_l4_port_end %d\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_l4_port_start,
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_l4_port_end);

			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_L4_DPORT_RANGE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_dest_l4_port_range \n");
			}

		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_STAGIF_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_stagIf %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stagIf);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_STAGIF_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_stagIf \n");
			}
		}
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_CTAGIF_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ctagIf %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ctagIf);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_CTAGIF_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_ctagIf \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_STAG_PRI_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_stag_pri %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stag_pri);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_STAG_PRI_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_stag_pri \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_STAG_VID_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_stag_vid %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stag_vid);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_STAG_VID_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_stag_vid \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_STAG_DEI_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_stag_dei %d set\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stag_dei);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_STAG_DEI_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_stag_dei \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_STREAM_ID_BIT)!=0){
			PROC_PRINTF("rg set acl-filter pattern ingress_stream_id %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stream_id);
			PROC_PRINTF("rg set acl-filter pattern ingress_stream_id_mask 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stream_id_mask);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_STREAM_ID_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_stream_id \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_WLANDEV_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_wlanDevMask 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_wlanDevMask);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_WLANDEV_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_wlanDevMask \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_WLANDEV_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_wlanDevMask 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_wlanDevMask);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & EGRESS_WLANDEV_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_wlanDevMask \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INTERNAL_PRI_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern internal_pri %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.internal_pri);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INTERNAL_PRI_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not internal_pri \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_FLOWLABEL_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ipv6_flow_label  %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ipv6_flow_label);
			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_IPV6_FLOWLABEL_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_ipv6_flow_label  \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_SMAC_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_smac %x:%x:%x:%x:%x:%x\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_smac.octet[0],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_smac.octet[1],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_smac.octet[2],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_smac.octet[3],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_smac.octet[4],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_smac.octet[5]);

			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & EGRESS_SMAC_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_smac \n");
			}
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_DMAC_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_dmac %x:%x:%x:%x:%x:%x\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dmac.octet[0],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dmac.octet[1],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dmac.octet[2],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dmac.octet[3],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dmac.octet[4],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dmac.octet[5]);

			if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & EGRESS_DMAC_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_dmac \n");
			}
		}


		PROC_PRINTF("rg add acl-filter entry\n");
		}
	}


	PROC_PRINTF("#add classify-filter\n");	//action(*), entry and pattern(*)
	for(i=0;i<TOTAL_CF_ENTRY_SIZE;i++){
		if(rg_db.systemGlobal.classify_SW_table_entry[i].filter_fields!=0 ||
			rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field!=0 ||
			rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field!=0
			 ){
		//uint8 setFlag=0;
		//char tmpStr[255];
		//memset(tmpStr,0,255);
		PROC_PRINTF("rg clear classify-filter\n");

		if(rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field!=0x0){

			if((rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field & CF_DS_ACTION_STAG_BIT)!=0){

				//init varible
				unknownCmdFlag=0;
				tagDecision=-1;
				vidDecision=-1;
				priDecision=-1;

				switch(rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.svlanTagIfDecision)
				{
					case ACL_SVLAN_TAGIF_NOP: tagDecision=0; break;
					case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID: tagDecision=1; break;
					case ACL_SVLAN_TAGIF_UNTAG: tagDecision=3; break;
					case ACL_SVLAN_TAGIF_TRANSPARENT: tagDecision=4; break;
					case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2: tagDecision=2; break;
					case ACL_SVLAN_TAGIF_TAGGING_WITH_ORIGINAL_STAG_TPID: tagDecision=5; break;
					default:
						unknownCmdFlag =1;
						break;
				}

				switch(rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.svlanSvidDecision)
				{
					case ACL_SVLAN_SVID_ASSIGN: vidDecision=1; break;
					case ACL_SVLAN_SVID_COPY_FROM_1ST_TAG: vidDecision=2; break;
					case ACL_SVLAN_SVID_COPY_FROM_2ND_TAG: vidDecision=3; break;
					case ACL_SVLAN_SVID_NOP: vidDecision=0; break;
					default:
						unknownCmdFlag =1;
						break;
				}
				switch(rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.svlanSpriDecision)
				{
					case ACL_SVLAN_SPRI_ASSIGN: priDecision=1; break;
					case ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG: priDecision=2; break;
					case ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG: priDecision=3; break;
					case ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI: priDecision=4; break;
					case ACL_SVLAN_SPRI_NOP: priDecision=0; break;
					default:
						unknownCmdFlag =1;
						break;
				}

				if(unknownCmdFlag == 1)
				{
					PROC_PRINTF("rg set classify-filter action direction downstream action_stag [unknown action_stag decision]");
				}
				else
				{
					PROC_PRINTF("rg set classify-filter action direction downstream action_stag tagDecision %d svidDecision %d spriDecision %d svid %d spri %d\n",
						tagDecision,vidDecision,priDecision,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.assignedSvid,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.assignedSpri);
				}

			}
			if((rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field & CF_DS_ACTION_CTAG_BIT)!=0){
				switch(rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.cvlanTagIfDecision)
				{
					case ACL_CVLAN_TAGIF_NOP: tagDecision=0; break;
					case ACL_CVLAN_TAGIF_TAGGING: tagDecision=1; break;
					case ACL_CVLAN_TAGIF_UNTAG: tagDecision=2; break;
					case ACL_CVLAN_TAGIF_TRANSPARENT: tagDecision=3; break;
					default:
						unknownCmdFlag =1;
						break;
				}

				switch(rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.cvlanCvidDecision)
				{
					case ACL_CVLAN_CVID_ASSIGN: vidDecision=1; break;
					case ACL_CVLAN_CVID_COPY_FROM_1ST_TAG: vidDecision=2; break;
					case ACL_CVLAN_CVID_COPY_FROM_2ND_TAG: vidDecision=3; break;
					case ACL_CVLAN_CVID_NOP: vidDecision=0; break;
					default:
						unknownCmdFlag =1;
						break;
				}

				switch(rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.cvlanCpriDecision)
				{
					case ACL_CVLAN_CPRI_ASSIGN: priDecision=1; break;
					case ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG: priDecision=2; break;
					case ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG: priDecision=3; break;
					case ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI: priDecision=4; break;
					case ACL_CVLAN_CPRI_NOP: priDecision=0; break;
					default:
						unknownCmdFlag =1;
						break;
				}

				if(unknownCmdFlag == 1)
				{
					PROC_PRINTF("rg set classify-filter action direction downstream action_ctag [unknown action_ctag decision]");
				}
				else
				{
					PROC_PRINTF("rg set classify-filter action direction downstream action_ctag tagDecision %d cvidDecision %d cpriDecision %d cvid %d cpri %d\n",
						tagDecision,vidDecision,priDecision,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.assignedCvid,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.assignedCpri);
				}

			}

			if((rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field & CF_DS_ACTION_CFPRI_BIT)!=0){
				if(rg_db.systemGlobal.classify_SW_table_entry[i].action_cfpri.cfPriDecision==ACL_CFPRI_ASSIGN)
					PROC_PRINTF("rg set classify-filter action direction downstream action_cfpri cfpriDecision 0 cfpri %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].action_cfpri.assignedCfPri);
				else if(rg_db.systemGlobal.classify_SW_table_entry[i].action_cfpri.cfPriDecision==ACL_CFPRI_NOP)
					PROC_PRINTF("rg set classify-filter action direction downstream action_cfpri cfpriDecision 1 cfpri 0\n");
				else
					PROC_PRINTF("rg set classify-filter action direction downstream action_cfpri [unknown cfpriDecision]\n");
			}
			if((rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field & CF_DS_ACTION_DSCP_BIT)!=0){
				if(rg_db.systemGlobal.classify_SW_table_entry[i].action_dscp.dscpDecision==ACL_DSCP_ASSIGN)
					PROC_PRINTF("rg set classify-filter action direction downstream action_dscp dscpDecision 0 dscp %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].action_dscp.assignedDscp);
				else if(rg_db.systemGlobal.classify_SW_table_entry[i].action_dscp.dscpDecision==ACL_DSCP_NOP)
					PROC_PRINTF("rg set classify-filter action direction downstream action_dscp dscpDecision 1 dscp 0\n");
				else
					PROC_PRINTF("rg set classify-filter action direction downstream action_dscp [unknown dscpDecision]\n");

			}


			if((rg_db.systemGlobal.classify_SW_table_entry[i].ds_action_field & CF_DS_ACTION_UNI_MASK_BIT)!=0){
				if(rg_db.systemGlobal.classify_SW_table_entry[i].action_uni.uniActionDecision==ACL_UNI_FWD_TO_PORTMASK_ONLY){
					PROC_PRINTF("rg set classify-filter action direction downstream action_uni uniDecision 0 portmask 0x%x\n",
					rg_db.systemGlobal.classify_SW_table_entry[i].action_uni.assignedUniPortMask);
				}
				else if(rg_db.systemGlobal.classify_SW_table_entry[i].action_uni.uniActionDecision==ACL_UNI_FORCE_BY_MASK){
					PROC_PRINTF("rg set classify-filter action direction downstream action_uni uniDecision 1 portmask 0x%x\n",
					rg_db.systemGlobal.classify_SW_table_entry[i].action_uni.assignedUniPortMask);
				}
				else if(rg_db.systemGlobal.classify_SW_table_entry[i].action_uni.uniActionDecision==ACL_UNI_TRAP_TO_CPU){
					PROC_PRINTF("rg set classify-filter action direction downstream action_uni uniDecision 2 portmask 0x0\n");
				}
				else if(rg_db.systemGlobal.classify_SW_table_entry[i].action_uni.uniActionDecision==AL_UNI_NOP){
					PROC_PRINTF("rg set classify-filter action direction downstream action_uni uniDecision 3 portmask 0x0\n");
				}
				else{
					PROC_PRINTF("rg set classify-filter action direction downstream action_uni [unknown uniDecision]\n");
				}
			}

		}

		//upstreaming
		if(rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field != 0){

			if((rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_STAG_BIT)!=0)
			{

				//init varible
				unknownCmdFlag=0;
				tagDecision=-1;
				vidDecision=-1;
				priDecision=-1;

				switch(rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.svlanTagIfDecision)
				{
					case ACL_SVLAN_TAGIF_NOP: tagDecision=0; break;
					case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID: tagDecision=1; break;
					case ACL_SVLAN_TAGIF_UNTAG: tagDecision=3; break;
					case ACL_SVLAN_TAGIF_TRANSPARENT: tagDecision=4; break;
					case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2: tagDecision=2; break;
					case ACL_SVLAN_TAGIF_TAGGING_WITH_ORIGINAL_STAG_TPID: tagDecision=5; break;
					default:
						unknownCmdFlag =1;
						break;
				}

				switch(rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.svlanSvidDecision)
				{
					case ACL_SVLAN_SVID_ASSIGN: vidDecision=1; break;
					case ACL_SVLAN_SVID_COPY_FROM_1ST_TAG: vidDecision=2; break;
					case ACL_SVLAN_SVID_COPY_FROM_2ND_TAG: vidDecision=3; break;
					case ACL_SVLAN_SVID_NOP: vidDecision=0; break;
					default:
						unknownCmdFlag =1;
						break;
				}
				switch(rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.svlanSpriDecision)
				{
					case ACL_SVLAN_SPRI_ASSIGN: priDecision=1; break;
					case ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG: priDecision=2; break;
					case ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG: priDecision=3; break;
					case ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI: priDecision=4; break;
					case ACL_SVLAN_SPRI_NOP: priDecision=0; break;
					default:
						unknownCmdFlag =1;
						break;
				}

				if(unknownCmdFlag == 1)
				{
					PROC_PRINTF("rg set classify-filter action direction downstream action_stag [unknown action_stag decision]");
				}
				else
				{
					PROC_PRINTF("rg set classify-filter action direction upstream action_stag tagDecision %d svidDecision %d spriDecision %d svid %d spri %d\n",
						tagDecision,vidDecision,priDecision,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.assignedSvid,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_svlan.assignedSpri);
				}

			}

			if((rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_CTAG_BIT)!=0)
			{
				switch(rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.cvlanTagIfDecision)
				{
					case ACL_CVLAN_TAGIF_NOP: tagDecision=0; break;
					case ACL_CVLAN_TAGIF_TAGGING: tagDecision=1; break;
					case ACL_CVLAN_TAGIF_UNTAG: tagDecision=2; break;
					case ACL_CVLAN_TAGIF_TRANSPARENT: tagDecision=3; break;
					default:
						unknownCmdFlag =1;
						break;
				}

				switch(rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.cvlanCvidDecision)
				{
					case ACL_CVLAN_CVID_ASSIGN: vidDecision=1; break;
					case ACL_CVLAN_CVID_COPY_FROM_1ST_TAG: vidDecision=2; break;
					case ACL_CVLAN_CVID_COPY_FROM_2ND_TAG: vidDecision=3; break;
					case ACL_CVLAN_CVID_NOP: vidDecision=0; break;
					case ACL_CVLAN_CVID_COPY_FROM_INTERNAL_VID: vidDecision=4; break;
					default:
						unknownCmdFlag =1;
						break;
				}

				switch(rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.cvlanCpriDecision)
				{
					case ACL_CVLAN_CPRI_ASSIGN: priDecision=1; break;
					case ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG: priDecision=2; break;
					case ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG: priDecision=3; break;
					case ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI: priDecision=4; break;
					case ACL_CVLAN_CPRI_NOP: priDecision=0; break;
					default:
						unknownCmdFlag =1;
						break;
				}

				if(unknownCmdFlag == 1)
				{
					PROC_PRINTF("rg set classify-filter action direction downstream action_ctag [unknown action_ctag decision]");
				}
				else
				{
					PROC_PRINTF("rg set classify-filter action direction upstream action_ctag tagDecision %d cvidDecision %d cpriDecision %d cvid %d cpri %d\n",
						tagDecision,vidDecision,priDecision,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.assignedCvid,
						rg_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.assignedCpri);
				}

			}

/*
			//dop is supported by action_fwd in apolloFE
			if((rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_DROP_BIT)!=0)
				PROC_PRINTF("rg set classify-filter action direction upstream action_drop\n");
*/
			if((rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_FWD_BIT)!=0)
			{
				PROC_PRINTF("CF_US_ACTION_FWD_BIT");
				if(rg_db.systemGlobal.classify_SW_table_entry[i].action_fwd.fwdDecision==ACL_FWD_NOP){
					PROC_PRINTF("rg set classify-filter action direction upstream action_fwd fwdDecision 0\n");
				}
				else if(rg_db.systemGlobal.classify_SW_table_entry[i].action_fwd.fwdDecision==ACL_FWD_DROP){
					PROC_PRINTF("rg set classify-filter action direction upstream action_fwd fwdDecision 1\n");
				}
				else if(rg_db.systemGlobal.classify_SW_table_entry[i].action_fwd.fwdDecision==ACL_FWD_TRAP_TO_CPU){
					PROC_PRINTF("rg set classify-filter action direction upstream action_fwd fwdDecision 2\n");
				}
				else if(rg_db.systemGlobal.classify_SW_table_entry[i].action_fwd.fwdDecision==ACL_FWD_DROP_TO_PON){
					PROC_PRINTF("rg set classify-filter action direction upstream action_fwd fwdDecision 3\n");
				}
				else{
					PROC_PRINTF("rg set classify-filter action direction upstream action_fwd [unknown fwdDecision]\n");
				}
			}



			if((rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_CFPRI_BIT)!=0)
			{
				if(rg_db.systemGlobal.classify_SW_table_entry[i].action_cfpri.cfPriDecision==ACL_CFPRI_ASSIGN)
					PROC_PRINTF("rg set classify-filter action direction upstream action_cfpri cfpriDecision 0 cfpri %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].action_cfpri.assignedCfPri);
				else if(rg_db.systemGlobal.classify_SW_table_entry[i].action_cfpri.cfPriDecision==ACL_CFPRI_NOP)
					PROC_PRINTF("rg set classify-filter action direction upstream action_cfpri cfpriDecision 1 cfpri 0\n");
				else
					PROC_PRINTF("rg set classify-filter action direction upstream action_cfpri [unknown cfpriDecision]\n");
			}


			if((rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_DSCP_BIT)!=0)
			{
				if(rg_db.systemGlobal.classify_SW_table_entry[i].action_dscp.dscpDecision==ACL_DSCP_ASSIGN)
					PROC_PRINTF("rg set classify-filter action direction upstream action_dscp dscpDecision 0 dscp %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].action_dscp.assignedDscp);
				else if(rg_db.systemGlobal.classify_SW_table_entry[i].action_dscp.dscpDecision==ACL_DSCP_NOP)
					PROC_PRINTF("rg set classify-filter action direction upstream action_dscp dscpDecision 1 dscp 0\n");
				else
					PROC_PRINTF("rg set classify-filter action direction upstream action_dscp [unknown dscpDecision]\n");
			}

			if((rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_SID_BIT)!=0)
			{
				if(rg_db.systemGlobal.classify_SW_table_entry[i].action_sid_or_llid.sidDecision==ACL_SID_LLID_ASSIGN)
					PROC_PRINTF("rg set classify-filter action direction upstream action_sid sidDecision 0 sid %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].action_sid_or_llid.assignedSid_or_llid);
				else if(rg_db.systemGlobal.classify_SW_table_entry[i].action_sid_or_llid.sidDecision==ACL_SID_LLID_NOP)
					PROC_PRINTF("rg set classify-filter action direction upstream action_sid sidDecision 0 sid 0\n");
				else
					PROC_PRINTF("rg set classify-filter action direction upstream action_sid [unknown sidDecision]\n");
			}
/*
			if((rg_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_LOG_BIT)!=0)
				PROC_PRINTF("rg set classify-filter action direction upstream action_log %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].action_log.assignedCounterIdx);
*/
		}

		if((rg_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_CTAGIF_BIT)!=0)
			PROC_PRINTF("rg set classify-filter pattern ctagIf %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].ctagIf);

		if((rg_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_ETHERTYPR_BIT)!=0){
			PROC_PRINTF("rg set classify-filter pattern etherType 0x%x\n",rg_db.systemGlobal.classify_SW_table_entry[i].etherType);
			PROC_PRINTF("rg set classify-filter pattern etherType_mask 0x%x\n",rg_db.systemGlobal.classify_SW_table_entry[i].etherType_mask);
		}

		if((rg_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_GEMIDX_BIT)!=0){
			PROC_PRINTF("rg set classify-filter pattern gemidx %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].gemidx);
			PROC_PRINTF("rg set classify-filter pattern gemidx_mask 0x%x\n",rg_db.systemGlobal.classify_SW_table_entry[i].gemidx_mask);
		}

		if((rg_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_INTERNALPRI_BIT)!=0)
			PROC_PRINTF("rg set classify-filter pattern internalPri %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].internalPri);

		if((rg_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_LLID_BIT)!=0)
			PROC_PRINTF("rg set classify-filter pattern llid %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].llid);

		if((rg_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_TAGPRI_BIT)!=0)
			PROC_PRINTF("rg set classify-filter pattern outterTagPri %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].outterTagPri);

		if((rg_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_TAGVID_BIT)!=0)
			PROC_PRINTF("rg set classify-filter pattern outterTagVid %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].outterTagVid);

		if((rg_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_STAGIF_BIT)!=0)
			PROC_PRINTF("rg set classify-filter pattern stagIf %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].stagIf);

		if((rg_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_UNI_BIT)!=0){
			PROC_PRINTF("rg set classify-filter pattern uni %d\n",rg_db.systemGlobal.classify_SW_table_entry[i].uni);
			PROC_PRINTF("rg set classify-filter pattern uni_mask 0x%x\n",rg_db.systemGlobal.classify_SW_table_entry[i].uni_mask);
		}
			PROC_PRINTF("rg set classify-filter entry %d\n",i);
			PROC_PRINTF("rg add classify-filter entry\n");
		}
	}

#endif

	return RT_ERR_RG_OK;
}

void _rtk_ca_port_dump(struct seq_file *s, ca_port_id_t cfg, int ca_port_cls[][3], int ca_port_type)
{
	ca_uint32_t port_id;
	ca_uint32_t port_type;

	enum{
		CA_PORT_TYPE_ORIG_SRC_PORT=0,
		CA_PORT_TYPE_SRC_PORT=1,
		CA_PORT_TYPE_DEST_PORT=2,
		CA_SW_CLS_USAGE_L2_IGR=0,
		CA_SW_CLS_USAGE_L2_EGR=1,
		CA_SW_CLS_USAGE_L3=2,
	};

	port_id = PORT_ID(cfg);
	port_type = PORT_TYPE(cfg);

	PROC_PRINTF("0x%x", port_id);
	switch (port_type) {
		case CA_PORT_TYPE_GPON:
			PROC_PRINTF("(L2 rule, TYPE_GPON)");
			break;
		case CA_PORT_TYPE_ETHERNET:
			PROC_PRINTF("(L2 rule, TYPE_ETHERNET)");
			break;
		case CA_PORT_TYPE_CPU:
			PROC_PRINTF("(L2 rule, TYPE_CPU)");
			break;
		case CA_PORT_TYPE_L2RP:
			PROC_PRINTF("(L3 rule, TYPE_L2RP)");
			break;
		case CA_PORT_TYPE_L3:
			PROC_PRINTF("(L3 rule, TYPE_L3)");
			break;
		default:
			PROC_PRINTF("(port_type=0x%x)", port_type);
			break;
	}

	if(ca_port_type > 0)
	{	//only record src_port or dest_port
		switch (port_type) {
			case CA_PORT_TYPE_ETHERNET:
			case CA_PORT_TYPE_CPU:
				//src_port add at l2_igr; dest_port add at l2_egr
				ca_port_cls[port_id][ca_port_type-1]++;
				break;
			case CA_PORT_TYPE_L2RP:
			case CA_PORT_TYPE_L3:
				ca_port_cls[port_id][CA_SW_CLS_USAGE_L3]++;
				break;
		}
	}

	PROC_PRINTF(", ");
}

void _rtk_ca_vlan_tag_dump(struct seq_file *s, ca_classifier_vlan_mask_t cfg_mask, ca_classifier_vlan_t cfg)
{
	if(cfg_mask.tpid)	PROC_PRINTF("tpid = 0x%x, ", cfg.tpid);
	if(cfg_mask.vid)	PROC_PRINTF("vid = 0x%x, ", cfg.vid);
	if(cfg_mask.dei)	PROC_PRINTF("dei = 0x%x, ", cfg.dei);
	if(cfg_mask.pri)	PROC_PRINTF("pri = 0x%x, ", cfg.pri);
}

void _rtk_ca_ip_address_dump(struct seq_file *s, ca_ip_address_t cfg)
{
	//int i;

	//PROC_PRINTF("afi = %#x, ", cfg.afi);

	//for(i=0;i<4;i++)
	//	PROC_PRINTF("addr[%d] = 0x%x, ", i, cfg.ip_addr.addr[i]);

	if(cfg.afi == CA_IPV4)
		PROC_PRINTF("ipv4_addr = %pI4h, ", &cfg.ip_addr.ipv4_addr);
	else
		PROC_PRINTF("ipv6_addr = %x:%x:%x:%x, ", cfg.ip_addr.ipv6_addr[0], cfg.ip_addr.ipv6_addr[1], cfg.ip_addr.ipv6_addr[2], cfg.ip_addr.ipv6_addr[3]);

	PROC_PRINTF("addr_len = %#x", cfg.addr_len);
}

void _rtk_ca_rule_info_get(int index, char* debug_str, int debug_str_size)
{
	int sw_index;
	int ret;
	char *name_of_reserve_acl_type[]={	//ref: rtk_rg_aclAndCf_reserved_type_t	
		"L2_INGRESS_FORWARD_L3FE(not rearrange)",
		"IPV4_WITH_OPTION_TRAP(not rearrange)",
		"L2_INGRESS_MULTICAST_FORWARD_L2FE(not rearrange)",
#if defined(CONFIG_RG_G3_L2FE_POL_OFFLOAD)
		"L2_UPSTREAM_LOOPBACK_INGRESS_FORWARD_L3FE",
		"L2_UPSTREAM_LAN_INGRESS_FORWARD_LOOPBACK",
		"L2_INGRESS_SA_HOSTPOLICING",
		"L2_EGRESS_DA_HOSTPOLICING",
#endif
		"REARRANGE_PROTECTION(not rearrange)",

		"ALL_TRAP",
		"UNICAST_TRAP",
		"MULTICAST_SSDP_TRAP",
		"STPBLOCKING",
		"DOT1X_EAPOL_TRAP",
		"INTF0_IPV4_FRAGMENT_TRAP",
		"INTF1_IPV4_FRAGMENT_TRAP",
		"INTF2_IPV4_FRAGMENT_TRAP",
		"INTF3_IPV4_FRAGMENT_TRAP",
		"INTF4_IPV4_FRAGMENT_TRAP",
		"INTF5_IPV4_FRAGMENT_TRAP",
		"INTF6_IPV4_FRAGMENT_TRAP",
		"INTF7_IPV4_FRAGMENT_TRAP",
		"INTF8_IPV4_FRAGMENT_TRAP",
		"INTF9_IPV4_FRAGMENT_TRAP",
		"INTF10_IPV4_FRAGMENT_TRAP",
		"INTF11_IPV4_FRAGMENT_TRAP",
		"INTF12_IPV4_FRAGMENT_TRAP",
		"INTF13_IPV4_FRAGMENT_TRAP",
		"INTF14_IPV4_FRAGMENT_TRAP",
		"INTF15_IPV4_FRAGMENT_TRAP",
		"PPPoE_LCP_PACKET_ASSIGN_PRIORITY",
		"L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY",
		"ACK_PACKET_ASSIGN_PRIORITY",
		"ACK_PACKET_TRAP_OR_ASSIGN_PRIORITY",
		"SYN_PACKET_TRAP_OR_ASSIGN_PRIORITY",
		"IGMP_MLD_DROP",
		"MULTICAST_L2FE_FLOODING",

		"HEAD_END",

		"MULTICAST_PROTOCOL_TRAP",
		"MULTICAST_IPV4_FORWARD_L2FE",
		"MULTICAST_IPV6_FORWARD_L2FE",
		"MULTICAST_DSLITE_TRAP",
		"L2TP_DATA_LOWER_PRIORITY",
		"CONTROL_PACKET_TRAP",
		"MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT",
		"UDP_DOS_PACKET_ASSIGN_SHARE_METER",
		"ARP_PACKET_ASSIGN_SHARE_METER",
		"SYN_PACKET_ASSIGN_SHARE_METER",
		"TAIL_END",
	};

	memset(debug_str, 0, debug_str_size);
	if(rg_db.systemGlobal.ca_cls_rule_record[index].ruleType < RTK_RG_ACLANDCF_RESERVED_TAIL_END)
	{
		snprintf(debug_str, debug_str_size, "%s", name_of_reserve_acl_type[rg_db.systemGlobal.ca_cls_rule_record[index].ruleType]);
	}
	else
	{
		bzero(&rg_db.systemGlobal.aclSWEntry,sizeof(rg_db.systemGlobal.aclSWEntry));
		sw_index = rg_db.systemGlobal.ca_cls_rule_record[index].ruleType-RTK_RG_ACLANDCF_RESERVED_TAIL_END;
		ret = _rtk_rg_aclSWEntry_get(sw_index,&rg_db.systemGlobal.aclSWEntry);
		if(ret == RT_ERR_RG_OK)
			snprintf(debug_str, debug_str_size, "SW_ACL[%d],portmask=0x%x,total=%d", sw_index, rg_db.systemGlobal.aclSWEntry.hw_aclEntry_port.bits[0],rg_db.systemGlobal.aclSWEntry.hw_aclEntry_count);
		else
			snprintf(debug_str, debug_str_size, "SW_ACL[%d]", sw_index);
	}

}

int _dump_acl_ca(struct seq_file *s)
{
	int i, j;
	uint32 index;
	uint32 priority;
	ca_classifier_key_t key;
	ca_classifier_key_mask_t key_mask;
	ca_classifier_action_t action;
	char* string_print;
	char string_buf[64];
	int ca_port_cls[AAL_LPORT_L3_LAN+1][3];	//each port for L2 IGR/L2EGR/L3
	ca_port_id_t  ca_port;
	ca_boolean_t	 specPkt_forward_to_cpu;
	ca_port_id_t	 specPkt_cpu_port;
	ca_uint8_t		 specPkt_priority;
	ca_special_packet_option_t act_opt;
	int l3_cls_profile_lan = 0;
	int l3_cls_profile_wan = 0;

	enum{
		CA_PORT_TYPE_ORIG_SRC_PORT=0,
		CA_PORT_TYPE_SRC_PORT=1,
		CA_PORT_TYPE_DEST_PORT=2,
		CA_SW_CLS_USAGE_L2_IGR=0,
		CA_SW_CLS_USAGE_L2_EGR=1,
		CA_SW_CLS_USAGE_L3=2,
	};

	bzero(&ca_port_cls, sizeof(ca_port_cls));

	if(rg_db.systemGlobal.aclAndCfReservedRule.acl_wanPortMask != rg_db.systemGlobal.wanPortMask.portmask)
		PROC_PRINTF("+++ acl_wanPortMask = 0x%x, wanPortMask = 0x%x, lanPortMask = 0x%x +++\n", rg_db.systemGlobal.aclAndCfReservedRule.acl_wanPortMask, rg_db.systemGlobal.wanPortMask.portmask, rg_db.systemGlobal.lanPortMask.portmask);
	else
		PROC_PRINTF("+++ wanPortMask = 0x%x, lanPortMask = 0x%x +++\n", rg_db.systemGlobal.wanPortMask.portmask, rg_db.systemGlobal.lanPortMask.portmask);
	PROC_PRINTF("--------------- ACL TABLES ----------------(TOTAL: %d)\n", rg_db.systemGlobal.ca_cls_used_count);
	for(index=0; index<MAX_ACL_CA_CLS_RULE_SIZE; index++)
	{
		if(ca_classifier_rule_get(G3_DEF_DEVID, index, &priority, &key, &key_mask, &action) != CA_E_OK)
			continue;
		if(rg_db.systemGlobal.ca_cls_rule_record[index].valid == ENABLED)
		{
			_rtk_ca_rule_info_get(index, string_buf, sizeof(string_buf));
			PROC_PRINTF("  --- ACL TABLE[%d] ---(RTK-Priority=%d, RTK-RuleType=%d[%s])\n", index, rg_db.systemGlobal.ca_cls_rule_record[index].priority, rg_db.systemGlobal.ca_cls_rule_record[index].ruleType, string_buf);
		}
		else
			PROC_PRINTF("  --- ACL TABLE[%d] ---\n", index);

		PROC_PRINTF("\tPriority:0x%x\n", priority);
		PROC_PRINTF("\tKey: ");
		if(key_mask.orig_src_port) {
			PROC_PRINTF("orig_src_port = ");
			_rtk_ca_port_dump(s, key.orig_src_port, ca_port_cls, CA_PORT_TYPE_ORIG_SRC_PORT);
		}
		if(key_mask.src_port) {
			PROC_PRINTF("src_port = ");
			_rtk_ca_port_dump(s, key.src_port, ca_port_cls, CA_PORT_TYPE_SRC_PORT);
		}
		if(key_mask.dest_port) {
			PROC_PRINTF("dest_port = ");
			_rtk_ca_port_dump(s, key.dest_port, ca_port_cls, CA_PORT_TYPE_DEST_PORT);
		}
		if(key_mask.src_intf)				PROC_PRINTF("src_intf = 0x%x, ",key.src_intf);
		if(key_mask.dst_intf)				PROC_PRINTF("dst_intf = 0x%x, ",key.dst_intf);

		if(key_mask.merge_prio)				PROC_PRINTF("merge_prio = 0x%x, ",key.merge_prio);
		if(key_mask.packet_length)				PROC_PRINTF("packet_length_low = %d, packet_length_high = %d, ",key.packet_length_low, key.packet_length_high);
		if(key_mask.ingress_lan)				PROC_PRINTF("ingress_lan = 0x%x, ",key.ingress_lan);

		if((index < (MAX_ACL_CA_CLS_RULE_SIZE-1)) &&	//prevent Overrunning array
			(rg_db.systemGlobal.ca_cls_rule_record[index].ruleType == rg_db.systemGlobal.ca_cls_rule_record[index+1].ruleType) && rg_db.systemGlobal.ca_cls_rule_record[index+1].valid == ENABLED
			&& (rg_db.systemGlobal.ca_cls_rule_record[index].ruleType >= RTK_RG_ACLANDCF_RESERVED_TAIL_END))
		{
			PROC_PRINTF("\n");
			PROC_PRINTF("\t\t\t\t ...... Check key & action from next entry ...... \n");
			continue;
		}

		if(key_mask.l2) {
			PROC_PRINTF("\n");
			PROC_PRINTF("\tL2 Field: ");
			if(key_mask.l2_mask.ethertype)	PROC_PRINTF("ethertype = 0x%x, ",key.l2.ethertype);
			if(key_mask.l2_mask.subtype)	PROC_PRINTF("subtype = 0x%x, ",key.l2.subtype);
			if(key_mask.l2_mask.vlan_count)	PROC_PRINTF("vlan_count = 0x%x, ",key.l2.vlan_count);
			if(key_mask.l2_mask.is_multicast)	PROC_PRINTF("is_multicast = 0x%x, ",key.l2.is_multicast);
			if(key_mask.l2_mask.is_length)	PROC_PRINTF("is_length = 0x%x, ",key.l2.is_length);
			if(key_mask.l2_mask.cfm_opcode)	PROC_PRINTF("cfm_opcode = 0x%x, ",key.l2.cfm_opcode);
			if(key_mask.l2_mask.mac_sa) {
				PROC_PRINTF("\n");
				PROC_PRINTF("\t\tmac_sa(min): %pM; mac_sa(max): %pM", key.l2.mac_sa.mac_min, key.l2.mac_sa.mac_max);
			}
			if(key_mask.l2_mask.mac_da) {
				PROC_PRINTF("\n");
				PROC_PRINTF("\t\tmac_da(min): %pM; mac_da(max): %pM", key.l2.mac_da.mac_min, key.l2.mac_da.mac_max);
			}
			if(key_mask.l2_mask.vlan_otag) {
				PROC_PRINTF("\n");
				PROC_PRINTF("\t\tvlan_otag(min): ");
				_rtk_ca_vlan_tag_dump(s, key_mask.l2_mask.vlan_otag_mask, key.l2.vlan_otag.vlan_min);
				PROC_PRINTF(";vlan_otag(max): ");
				_rtk_ca_vlan_tag_dump(s, key_mask.l2_mask.vlan_otag_mask, key.l2.vlan_otag.vlan_max);
			}
			if(key_mask.l2_mask.vlan_itag) {
				PROC_PRINTF("\n");
				PROC_PRINTF("\t\tvlan_itag(min): ");
				_rtk_ca_vlan_tag_dump(s, key_mask.l2_mask.vlan_itag_mask, key.l2.vlan_itag.vlan_min);
				PROC_PRINTF(";vlan_itag(max): ");
				_rtk_ca_vlan_tag_dump(s, key_mask.l2_mask.vlan_itag_mask, key.l2.vlan_itag.vlan_max);
			}
		}

		if(key_mask.ip) {
			PROC_PRINTF("\n");
			PROC_PRINTF("\tIP Field: ");
			if(key_mask.ip_mask.ip_valid)			PROC_PRINTF("ip_valid = 0x%x, ",key.ip.ip_valid);
			if(key_mask.ip_mask.ip_version)		PROC_PRINTF("ip_version = 0x%x, ",key.ip.ip_version);
			if(key_mask.ip_mask.ip_protocol)		PROC_PRINTF("ip_protocol = 0x%x, ",key.ip.ip_protocol);
			if(key_mask.ip_mask.dscp)			PROC_PRINTF("dscp = 0x%x, ",key.ip.dscp);
			if(key_mask.ip_mask.ecn)			PROC_PRINTF("ecn = 0x%x, ",key.ip.ecn);
			if(key_mask.ip_mask.fragment)		PROC_PRINTF("fragment = 0x%x, ",key.ip.fragment);
			if(key_mask.ip_mask.have_options)	PROC_PRINTF("have_options = 0x%x, ",key.ip.have_options);
			if(key_mask.ip_mask.flow_label)		PROC_PRINTF("flow_label = 0x%x, ",key.ip.flow_label);
			if(key_mask.ip_mask.ext_header)		PROC_PRINTF("ext_header = 0x%x, ",key.ip.ext_header);
			if(key_mask.ip_mask.icmp_type)		PROC_PRINTF("icmp_type = 0x%x, ",key.ip.icmp_type);
			if(key_mask.ip_mask.igmp_type)		PROC_PRINTF("igmp_type = 0x%x, ",key.ip.igmp_type);
			if(key_mask.ip_mask.is_multicast)	PROC_PRINTF("is_multicast = 0x%x, ",key.ip.is_multicast);
			if(key_mask.ip_mask.ip_sa) {
				PROC_PRINTF("\n");
				PROC_PRINTF("\t\tip_sa: ");
				_rtk_ca_ip_address_dump(s, key.ip.ip_sa);
			}
			if(key_mask.ip_mask.ip_sa_max) {
				PROC_PRINTF("\n");
				PROC_PRINTF("\t\tip_sa_max: ");
				_rtk_ca_ip_address_dump(s, key.ip.ip_sa_max);
			}
			if(key_mask.ip_mask.ip_da) {
				PROC_PRINTF("\n");
				PROC_PRINTF("\t\tip_da: ");
				_rtk_ca_ip_address_dump(s, key.ip.ip_da);
			}
			if(key_mask.ip_mask.ip_da_max) {
				PROC_PRINTF("\n");
				PROC_PRINTF("\t\tip_da_max: ");
				_rtk_ca_ip_address_dump(s, key.ip.ip_da_max);
			}
		}

		if(key_mask.l4) {
			PROC_PRINTF("\n");
			PROC_PRINTF("\tL4 Field: ");
			if(key_mask.l4_mask.l4_valid)		PROC_PRINTF("l4_valid = 0x%x, ",key.l4.l4_valid);
			if(key_mask.l4_mask.tcp_flags)	PROC_PRINTF("tcp_flags = 0x%x, ",key.l4.tcp_flags);
			if(key_mask.l4_mask.src_port) {
				PROC_PRINTF("\n");
				PROC_PRINTF("\t\tsrc_port: min = %d, max = %d", key.l4.src_port.min, key.l4.src_port.max);
			}
			if(key_mask.l4_mask.dst_port) {
				PROC_PRINTF("\n");
				PROC_PRINTF("\t\tdst_port: min = %d, max = %d", key.l4.dst_port.min, key.l4.dst_port.max);
			}
		}

		if(key_mask.ext) {
			PROC_PRINTF("\n");
			PROC_PRINTF("\tEXT Field: ");
			PROC_PRINTF("offset = 0x%x, ", key.ext.offset);
			PROC_PRINTF("length = 0x%x, ", key.ext.length);
			PROC_PRINTF("\n");
			PROC_PRINTF("\t\tdata: ");
			for(i = 0; i < CA_MAX_DATA_EXTRACTION_LEN;i++) {
				PROC_PRINTF("[%d] = 0x%x, ", i, key.ext.data[i]);
			}
			PROC_PRINTF("\n");
			PROC_PRINTF("\t\tmask: ");
			for(i = 0; i < CA_MAX_DATA_EXTRACTION_LEN;i++) {
				PROC_PRINTF("[%d] = 0x%x, ", i, key.ext.mask[i]);
			}
		}

		if(rg_db.systemGlobal.ca_cls_rule_record[index].aal_customize > 0) {
			PROC_PRINTF("\n");
			PROC_PRINTF("\tHW Field: ");
			for(i=CA_CLASSIFIER_AAL_NA; (0x1<<i)<CA_CLASSIFIER_AAL_END; i++) {
				if(!(rg_db.systemGlobal.ca_cls_rule_record[index].aal_customize&(0x1<<i)))
					continue;
				switch(0x1<<i) {
					case CA_CLASSIFIER_AAL_L3_PPP_LCP:
						PROC_PRINTF("ppp_protocol_enc = 0xC021, ip_vld = 0, ");
						break;
					case CA_CLASSIFIER_AAL_L3_PPP_IPCP:
						PROC_PRINTF("ppp_protocol_enc = 0x8021, ip_vld = 0, ");
						break;
					case CA_CLASSIFIER_AAL_L3_PPP_IP6CP:
						PROC_PRINTF("ppp_protocol_enc = 0x8057, ip_vld = 0, ");
						break;
					case CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT:
						PROC_PRINTF("ignore src_port, ");
						if((1<<PORT_ID(key.src_port)) == CA_L3_CLS_PROFILE_LAN)
							l3_cls_profile_lan++;
						else if((1<<PORT_ID(key.src_port)) == CA_L3_CLS_PROFILE_WAN)
							l3_cls_profile_wan++;
						else
							PROC_PRINTF("<<Unknown Profile>>");
						break;
					case CA_CLASSIFIER_AAL_L3_NOT_BROADCAST:
						PROC_PRINTF("mac_da_bc = 0, ");
						break;
					case CA_CLASSIFIER_AAL_L3_PKT_LEN_RANGE_IDX0:
						PROC_PRINTF("pkt_len_range_idx = 0, ");
						break;
					case CA_CLASSIFIER_AAL_L3_PPP_IPV4:
						PROC_PRINTF("ppp_protocol_enc = 0x0021");
						break;
					case CA_CLASSIFIER_AAL_L3_PPP_IPV6:
						PROC_PRINTF("ppp_protocol_enc = 0x0057");
						break;
					case CA_CLASSIFIER_AAL_L3_KEEP_ORIG:
						PROC_PRINTF("keep_orig_pkt = 1, ");
						break;
					case CA_CLASSIFIER_AAL_L3_PKT_LEN_RANGE_IDX1:
						PROC_PRINTF("pkt_len_range_idx = 1, cls_rslt_type = 0");
						break;
					case CA_CLASSIFIER_AAL_L3_SPEC_PKT_L4_L2TP_PORT:
						PROC_PRINTF("(l2tp)l4_sport = 1701, l4_dport = 1701, ");
						break;
					case CA_CLASSIFIER_AAL_L3_MCAST_FORWARD:
						PROC_PRINTF("top_vid = 0xff0, t2_ctrl = 0xf, ldpid = L2FE, ");
						break;
					case CA_CLASSIFIER_AAL_L3_PKT_LEN_RANGE_IDX2:
						PROC_PRINTF("pkt_len_range_idx = 2, ");
						break;
					case CA_CLASSIFIER_AAL_L3_RSLT_TYPE_0:
						PROC_PRINTF("l3_rslt_type = 0, ");
						break;
					case CA_CLASSIFIER_AAL_L3_MAC_DA_IP_MC:
						PROC_PRINTF("mac_da_ip_mc = 1, ");
						break;
					case CA_CLASSIFIER_AAL_L3_MAC_DA_RSVD:
						PROC_PRINTF("mac_da_rsvd = 1, ");
						break;
					case CA_CLASSIFIER_AAL_L3_PPPOE_SESID_0:
						PROC_PRINTF("pppoe_session_id = 0, ");
						break;
					case CA_CLASSIFIER_AAL_L3_MAC_DA_AN_HIT:
						PROC_PRINTF("mac_da_an_hit = 1, ");
						break;
					default:
						break;
				}
			}
		}

		PROC_PRINTF("\n");
		switch(action.forward) {
			case CA_CLASSIFIER_FORWARD_DENY:
				PROC_PRINTF("\tAction: CA_CLASSIFIER_FORWARD_DENY, drop = 0x%x", action.dest.drop);
				break;
			case CA_CLASSIFIER_FORWARD_FE:
				if(action.dest.fe == CA_CLASSIFIER_FORWARD_FE_L2FE)
					string_print = "L2FE";
				else if(action.dest.fe == CA_CLASSIFIER_FORWARD_FE_L3FE)
					string_print = "L3FE";
				else
					string_print = "NONE";
				PROC_PRINTF("\tAction: CA_CLASSIFIER_FORWARD_FE, fe = 0x%x(%s)", action.dest.fe, string_print);
				break;
			case CA_CLASSIFIER_FORWARD_INTERFACE:
				PROC_PRINTF("\tAction: CA_CLASSIFIER_FORWARD_INTERFACE, intf = 0x%x", action.dest.intf);
				break;
			case CA_CLASSIFIER_FORWARD_PORT:
				PROC_PRINTF("\tAction: CA_CLASSIFIER_FORWARD_PORT, port = 0x%x", action.dest.port);
				break;
			default:
				PROC_PRINTF("\tAction: Unknown!!");
				break;
		}

		PROC_PRINTF("\n");
		PROC_PRINTF("\tAction Option: ");
		if(action.options.masks.action_handle) {
			if(action.options.handle_type == CA_KEY_HANDLE_TYPE_FLOW_ID)
				PROC_PRINTF("flow_id = 0x%x, ", action.options.action_handle.flow_id);
			if(action.options.handle_type == CA_KEY_HANDLE_TYPE_GEM_INDEX)
				PROC_PRINTF("gem_index = 0x%x, ", action.options.action_handle.gem_index);
			if(action.options.handle_type == CA_KEY_HANDLE_TYPE_LLID_COS)
				PROC_PRINTF("llid_cos_index = 0x%x, ", action.options.action_handle.llid_cos_index);
		}
		if(action.options.masks.priority)		PROC_PRINTF("priority = 0x%x, ", action.options.priority);
		if(action.options.masks.dscp)			PROC_PRINTF("dscp = 0x%x, ", action.options.dscp);
		if(action.options.masks.inner_dot1p)	PROC_PRINTF("inner_dot1p = 0x%x, ", action.options.inner_dot1p);
		if(action.options.masks.inner_tpid)		PROC_PRINTF("inner_tpid = 0x%x, ", action.options.inner_tpid);
		if(action.options.masks.inner_dei)		PROC_PRINTF("inner_dei = 0x%x, ", action.options.inner_dei);
		if(action.options.masks.inner_vlan_act) {
			PROC_PRINTF("inner_vlan_act = 0x%x, ", action.options.inner_vlan_act);
			PROC_PRINTF("inner_vid = 0x%x, ", action.options.inner_vid);
		}
		if(action.options.masks.outer_dot1p)	PROC_PRINTF("outer_dot1p = 0x%x, ", action.options.outer_dot1p);
		if(action.options.masks.outer_tpid)		PROC_PRINTF("outer_tpid = 0x%x, ", action.options.outer_tpid);
		if(action.options.masks.outer_dei)		PROC_PRINTF("outer_dei = 0x%x, ", action.options.outer_dei);
		if(action.options.masks.outer_vlan_act) {
			switch(action.options.outer_vlan_act) {
				case CA_CLASSIFIER_VLAN_ACTION_NOP:
					PROC_PRINTF("outer_vlan_act = bypass VLAN action, ");
					break;
				case CA_CLASSIFIER_VLAN_ACTION_PUSH:
					PROC_PRINTF("outer_vlan_act = push a VLAN tag, ");
					break;
				case CA_CLASSIFIER_VLAN_ACTION_POP:
					PROC_PRINTF("outer_vlan_act = pop the VLAN tag, ");
					break;
				case CA_CLASSIFIER_VLAN_ACTION_SWAP:
					PROC_PRINTF("outer_vlan_act = swap the VLAN ID with given, ");
					break;
				case CA_CLASSIFIER_VLAN_ACTION_SET:
					PROC_PRINTF("outer_vlan_act = set to specified value, ");
					break;
				default:
					PROC_PRINTF("outer_vlan_act: Unknown!!");
					break;
			}
			PROC_PRINTF("outer_vid = 0x%x, ", action.options.outer_vid);
		}
		if(action.options.masks.sw_shaper_id)	PROC_PRINTF("sw_shaper_id = 0x%x, ", action.options.sw_shaper_id);
		if(action.options.masks.sw_id) {
			PROC_PRINTF("\n");
			PROC_PRINTF("\t\tsw_id: ");
			for(i = 0; i < 4;i++)
				PROC_PRINTF("[%d] = 0x%x, ", i, action.options.sw_id[i]);
		}
		if(action.options.masks.mac_da) {
			PROC_PRINTF("\n");
			PROC_PRINTF("\t\tmac_da(mask=0x%x): %pM", action.options.masks.mac_da, action.options.mac_da);
		}

		PROC_PRINTF("\n");
	}

	PROC_PRINTF("--------------- CA SW CLS Usage ----------------\n");
	for(j = CA_SW_CLS_USAGE_L2_IGR; j <= CA_SW_CLS_USAGE_L3; j++)
	{
		if(j == CA_SW_CLS_USAGE_L2_IGR)
			PROC_PRINTF("L2 IGR:\t");
		else if(j == CA_SW_CLS_USAGE_L2_EGR)
			PROC_PRINTF("L2 EGR:\t");
		else
			PROC_PRINTF("L3 CLS:\t");
		for(i = 0; i < AAL_LPORT_L3_LAN+1; i++) {
			if(ca_port_cls[i][j] > 0) {
				if(j == CA_SW_CLS_USAGE_L3) {
					if((1<<i) == CA_L3_CLS_PROFILE_LAN) {
						PROC_PRINTF(" [LAN]%d ", l3_cls_profile_lan);
						PROC_PRINTF(" [%02x]%d ", i, ca_port_cls[i][j]-l3_cls_profile_lan);
					}
					else if((1<<i) == CA_L3_CLS_PROFILE_WAN) {
						PROC_PRINTF(" [%02x]%d ", i, ca_port_cls[i][j]-l3_cls_profile_wan);
						PROC_PRINTF(" [WAN]%d ", l3_cls_profile_wan);
					}
					else
						PROC_PRINTF(" [%02x]%d ", i, ca_port_cls[i][j]);
				}
				else
					PROC_PRINTF(" [%02x]%d ", i, ca_port_cls[i][j]);
			}
		}
		PROC_PRINTF("\n");
	}

	PROC_PRINTF("--------------- Special Packet Trap  ----------------\n");
	for(index=0; index<MAX_ACL_SPECIAL_PACKET_RULE_SIZE; index++)
	{
		ca_port = CA_PORT_ID(CA_PORT_TYPE_L2RP, AAL_LPORT_L3_LAN);
		if(ca_special_packet_get(G3_DEF_DEVID, ca_port, index, &specPkt_forward_to_cpu, &specPkt_cpu_port, &specPkt_priority, &act_opt) != CA_E_OK || (specPkt_forward_to_cpu != TRUE))
		{
			ca_port = CA_PORT_ID(CA_PORT_TYPE_L2RP, AAL_LPORT_L3_WAN);
			if(ca_special_packet_get(G3_DEF_DEVID, ca_port, index, &specPkt_forward_to_cpu, &specPkt_cpu_port, &specPkt_priority, &act_opt) != CA_E_OK || (specPkt_forward_to_cpu != TRUE))
				continue;
		}

		switch (index)
		{
			case CA_PKT_TYPE_BPDU:
				snprintf(string_buf, sizeof(string_buf), "BPDU(0x%x)\t:", AAL_L3_SPECPKT_TYPE_BPDU);
				string_print = string_buf; break;
			case CA_PKT_TYPE_IGMP:
				snprintf(string_buf, sizeof(string_buf), "IGMP(0x%x)\t:", AAL_L3_SPECPKT_TYPE_IGMP);
				string_print = string_buf; break;
			case CA_PKT_TYPE_ARP:
				snprintf(string_buf, sizeof(string_buf), "ARP(0x%x)\t:", AAL_L3_SPECPKT_TYPE_ARP);
				string_print = string_buf; break;
			case CA_PKT_TYPE_OAM:
				snprintf(string_buf, sizeof(string_buf), "OAM(0x%x)\t:", CA_PKT_TYPE_OAM);
				string_print = string_buf; break;
			case CA_PKT_TYPE_OMCI:
				snprintf(string_buf, sizeof(string_buf), "OMCI(0x%x)\t:", CA_PKT_TYPE_OMCI);
				string_print = string_buf; break;
			case CA_PKT_TYPE_PLOAM:
				snprintf(string_buf, sizeof(string_buf), "PLOAM(0x%x)\t:", CA_PKT_TYPE_PLOAM);
				string_print = string_buf; break;
			case CA_PKT_TYPE_DHCP:
				snprintf(string_buf, sizeof(string_buf), "DHCP(0x%x)\t:", AAL_L3_SPECPKT_TYPE_DHCPV4);
				string_print = string_buf; break;
			case CA_PKT_TYPE_DHCPV6:
				snprintf(string_buf, sizeof(string_buf), "DHCPV6(0x%x)\t:", AAL_L3_SPECPKT_TYPE_DHCPV6);
				string_print = string_buf; break;
			case CA_PKT_TYPE_PPPOE_DIS:
				snprintf(string_buf, sizeof(string_buf), "PPPOE_DIS(0x%x):", AAL_L3_SPECPKT_TYPE_PPPOE_DISC);
				string_print = string_buf; break;
			case CA_PKT_TYPE_ICMP:
				snprintf(string_buf, sizeof(string_buf), "ICMP(0x%x/0x%x)\t:", AAL_L3_SPECPKT_TYPE_IPV4_ICMP, AAL_L3_SPECPKT_TYPE_IPV6_ICMP);
				string_print = string_buf; break;
			case CA_PKT_TYPE_ICMPV6_MLD:
				snprintf(string_buf, sizeof(string_buf), "ICMPV6_MLD(0x%x):", AAL_L3_SPECPKT_TYPE_MLD);
				string_print = string_buf; break;
			case CA_PKT_TYPE_ICMPV6_NDP:
				snprintf(string_buf, sizeof(string_buf), "ICMPV6_NDP(0x%x):", AAL_L3_SPECPKT_TYPE_IPV6_NDP);
				string_print = string_buf; break;
			case CA_PKT_TYPE_CFM:
				snprintf(string_buf, sizeof(string_buf), "CFM(0x%x)\t:", CA_PKT_TYPE_CFM);
				string_print = string_buf; break;
			case CA_PKT_TYPE_RIP:
				snprintf(string_buf, sizeof(string_buf), "RIP(0x%x)\t:", AAL_L3_SPECPKT_TYPE_RIP);
				string_print = string_buf; break;
			case CA_PKT_TYPE_SSDP:
				snprintf(string_buf, sizeof(string_buf), "SSDP(0x%x)\t:", AAL_L3_SPECPKT_TYPE_SSDP);
				string_print = string_buf; break;
			case CA_PKT_TYPE_1AS:
				snprintf(string_buf, sizeof(string_buf), "1AS(0x%x)\t:", AAL_L3_SPECPKT_TYPE_USER_DEF_0);
				string_print = string_buf; break;
			case CA_PKT_TYPE_USER_DEFINED:
				snprintf(string_buf, sizeof(string_buf), "USER_DEFINED(0x%x)\t:", AAL_L3_SPECPKT_TYPE_USER_DEF_0);
				string_print = string_buf; break;
			default:
				snprintf(string_buf, sizeof(string_buf), "Others[%d]:", index);
				string_print = string_buf; break;
		}
		PROC_PRINTF("\t%s", string_print);

		ca_port = CA_PORT_ID(CA_PORT_TYPE_L2RP, AAL_LPORT_L3_LAN);
		if(ca_special_packet_get(G3_DEF_DEVID, ca_port, index, &specPkt_forward_to_cpu, &specPkt_cpu_port, &specPkt_priority, &act_opt) == CA_E_OK)
		{
			if(specPkt_forward_to_cpu)
			{
				if (rg_db.systemGlobal.ca_special_pkt_record[index].valid == ENABLED)
					PROC_PRINTF("<L2_RP>Port 0x%x/CPU 0x%x/Pri %d/RSV_ACL[%d], ", AAL_LPORT_L3_LAN, PORT_ID(specPkt_cpu_port), specPkt_priority, rg_db.systemGlobal.ca_special_pkt_record[index].ruleType);
				else
					PROC_PRINTF("<L2_RP>Port 0x%x/CPU 0x%x/Pri %d, ", AAL_LPORT_L3_LAN, PORT_ID(specPkt_cpu_port), specPkt_priority);
			}
		}

		ca_port = CA_PORT_ID(CA_PORT_TYPE_L2RP, AAL_LPORT_L3_WAN);
		if(ca_special_packet_get(G3_DEF_DEVID, ca_port, index, &specPkt_forward_to_cpu, &specPkt_cpu_port, &specPkt_priority, &act_opt) == CA_E_OK)
		{
			if(specPkt_forward_to_cpu)
			{
				if (rg_db.systemGlobal.ca_special_pkt_record[index].valid == ENABLED)
					PROC_PRINTF("<L2_RP>Port 0x%x/CPU 0x%x/Pri %d/RSV_ACL[%d], ", AAL_LPORT_L3_WAN, PORT_ID(specPkt_cpu_port), specPkt_priority, rg_db.systemGlobal.ca_special_pkt_record[index].ruleType);
				else
					PROC_PRINTF("<L2_RP>Port 0x%x/CPU 0x%x/Pri %d, ", AAL_LPORT_L3_WAN, PORT_ID(specPkt_cpu_port), specPkt_priority);
			}
		}

		PROC_PRINTF("\n");
	}

	return RT_ERR_RG_OK;
}

int _dump_acl_ca_by_index( struct file *filp, const char *buff,unsigned long len, void *data )
{
	int ca_cls_index=_rtk_rg_pasring_proc_string_to_integer(buff,len);
	uint32 rule_info = CA_UINT32_INVALID;
	char string_buf[64];
	int rule_priority = -1;

	if(rg_db.systemGlobal.ca_cls_rule_record[ca_cls_index].valid == ENABLED)
	{
		rule_priority = rg_db.systemGlobal.ca_cls_rule_record[ca_cls_index].priority;
		_rtk_ca_rule_info_get(ca_cls_index, string_buf, sizeof(string_buf));
		rtlglue_printf("--- RG Rule Type %d [ %s ] --- \n", rg_db.systemGlobal.ca_cls_rule_record[ca_cls_index].ruleType, string_buf);
	}
	ca_classifier_l3_cls_rule_idx_get(G3_DEF_DEVID, ca_cls_index, &rule_info);
	ca_classifier_l3_cls_rule_dump(rule_info, rule_priority, ca_cls_index);
	aal_used_entry_print(AAL_TABLE_L3_CLS_KEY, CLS_TBL_ID(rule_info), (CLS_KEY_TYPE(rule_info)==CL_FULL_KEY)?2:1);

	return len;
}

int _dump_acl(struct seq_file *s)
{
	ca_used_aal_entry_print(G3_DEF_DEVID);

	return RT_ERR_RG_OK;
}

int _dump_acl_by_index( struct file *filp, const char *buff,unsigned long len, void *data )
{
	int cls_hw_index=_rtk_rg_pasring_proc_string_to_integer(buff,len);
	int ca_cls_index=0;
	int ca_cls_count=2;
	uint32 rule_info = CA_UINT32_INVALID;
	char string_buf[64];

	for(ca_cls_index = 0; ca_cls_index < MAX_ACL_CA_CLS_RULE_SIZE; ca_cls_index++)
	{
		if(rg_db.systemGlobal.ca_cls_rule_record[ca_cls_index].valid != ENABLED)
			continue;
		ca_classifier_l3_cls_rule_idx_get(G3_DEF_DEVID, ca_cls_index, &rule_info);
		if(CLS_TBL_ID(rule_info) != cls_hw_index)
			continue;
		_rtk_ca_rule_info_get(ca_cls_index, string_buf, sizeof(string_buf));
		rtlglue_printf("--- RG Rule Type %d [ %s ] --- \n", rg_db.systemGlobal.ca_cls_rule_record[ca_cls_index].ruleType, string_buf);
		ca_classifier_l3_cls_rule_dump(rule_info, rg_db.systemGlobal.ca_cls_rule_record[ca_cls_index].priority, ca_cls_index);
		if(CLS_KEY_TYPE(rule_info) != CL_FULL_KEY)
			ca_cls_count = 1;
		break;
	}
	aal_used_entry_print(AAL_TABLE_L3_CLS_KEY, cls_hw_index, ca_cls_count);

	return RT_ERR_RG_OK;
}



int _dump_cf(struct seq_file *s)
{
	PROC_PRINTF("Chip Not Support.\n");
	return RT_ERR_RG_OK;
}


int32 _dump_acl_ipRangeTable(struct seq_file *s)
{
	PROC_PRINTF("Chip Not Support.\n");
	return RT_ERR_RG_OK;
}

int32 _dump_acl_portRangeTable(struct seq_file *s)
{
	PROC_PRINTF("Chip Not Support.\n");
	return RT_ERR_RG_OK;
}



int32 _dump_acl_template(struct seq_file *s)
{
    int i, j;

    PROC_PRINTF("--------------- ACL TEMPLATES ----------------\n");
	for(i=0; i<ACL_TEMPLATE_CA_END; i++)
    {
        PROC_PRINTF("\tTEMPLATE[%d]: 0x%05x[",i,rg_db.systemGlobal.ca_cls_template[i]);
		for(j=0; j<ACL_FIELD_END; j++) {
			if(rg_db.systemGlobal.ca_cls_template[i] & (1<<j)) {
				switch(j) {
					case ACL_FIELD_MAC_SA:
						PROC_PRINTF("smac,");
						break;
					case ACL_FIELD_MAC_DA:
						PROC_PRINTF("dmac,");
						break;
					case ACL_FIELD_IP_PROTOCOL:
						PROC_PRINTF("ip_protocal,");
						break;
					case ACL_FIELD_IPV4_SA:
						PROC_PRINTF("src_ipv4,");
						break;
					case ACL_FIELD_IPV4_DA:
						PROC_PRINTF("dest_ipv4,");
						break;
					case ACL_FIELD_IPV6_SA:
						PROC_PRINTF("src_ipv6,");
						break;
					case ACL_FIELD_IPV6_DA:
						PROC_PRINTF("dest_ipv6,");
						break;
					case ACL_FIELD_IP_VER:
						PROC_PRINTF("ip_version,");
						break;
					case ACL_FIELD_ETHERTYPE_ENC:
						PROC_PRINTF("ethertype,");
						break;
					case ACL_FIELD_L4_PORT:
						PROC_PRINTF("l4_port,");
						break;
					case ACL_FIELD_L4_VLD:
						PROC_PRINTF("l4_valid,");
						break;
					case ACL_FIELD_DSCP:
						PROC_PRINTF("dscp,");
						break;
					case ACL_FIELD_VLAN_CNT:
						PROC_PRINTF("ctagif,");
						break;
					case ACL_FIELD_TOP_VL_802_1P:
						PROC_PRINTF("ctag_pri,");
						break;
					case ACL_FIELD_TOP_VID:
						PROC_PRINTF("ctag_vid,");
						break;
					case ACL_FIELD_TOP_DEI:
						PROC_PRINTF("ctag_cfi,");
						break;
					case ACL_FIELD_IPV6_FLOW_LBL:
						PROC_PRINTF("v6_flow_label,");
						break;
					default:
						break;
				}
			}
		}
        PROC_PRINTF("]\n");
    }

    PROC_PRINTF("--------------- ACL Field ----------------\n");
	for(j=0; j<ACL_FIELD_END; j++) {
		switch(j) {
			case ACL_FIELD_IP_PROTOCOL:
				PROC_PRINTF("\tip_protocal: l4_protocol, l4_protocol_value\n");
				break;
			case ACL_FIELD_IP_VER:
				PROC_PRINTF("\tip_version: v4_tagif, v6_tagif, l4_protocal, dscp, v6_dscp\n");
				break;
			case ACL_FIELD_L4_PORT:
				PROC_PRINTF("\tl4_port: src_port, dst_port\n");
				break;
			case ACL_FIELD_L4_VLD:
				PROC_PRINTF("\tl4_valid: src_port, dst_port\n");
				break;
			default:
				break;
		}
	}

    PROC_PRINTF("--------------- HW ACL LIMITATION ----------------\n");
    PROC_PRINTF("1. Only support one of above template at the same time, otherwise, only use SW-ACL and reset ACL will not add to hw.\n");
    PROC_PRINTF("\t\t(example: not support DIP+dmac, smac+dmac)\n");
    PROC_PRINTF("2. Only support single ip address/mac address.\n");
    PROC_PRINTF("3. Not support reverse check. (example: ingress_ipv4_tagif 0, not ingress_dmac).\n");

    return 0;
}


//internal APIs
int _dump_rg_acl_entry_content(struct seq_file *s, rtk_rg_aclFilterAndQos_t *aclPara)
{
	rtk_rg_aclFilterAndQos_t *acl_parameter;

	//[FIXME] fwd_type can use name_of_string
	PROC_PRINTF(" fwd_type: %d\n",aclPara->fwding_type_and_direction);
	PROC_PRINTF(" acl_weight: %d\n",aclPara->acl_weight);


	PROC_PRINTF("[Patterns]: \n");
	PROC_PRINTF("filter_fields:0x%llx\n", aclPara->filter_fields);
	if(aclPara->filter_fields&INGRESS_PORT_BIT) PROC_PRINTF(" %s ingress_port_mask:0x%x\n", (aclPara->filter_fields_inverse&INGRESS_PORT_BIT)?"not":"",aclPara->ingress_port_mask.portmask);
	if(aclPara->filter_fields&INGRESS_EGRESS_PORTIDX_BIT) PROC_PRINTF(" %s ingress_port_idx:  %d  ingress_port_idx_mask:	0x%x\n",(aclPara->filter_fields_inverse&INGRESS_EGRESS_PORTIDX_BIT)?"not":"", aclPara->ingress_port_idx,aclPara->ingress_port_idx_mask);
	if(aclPara->filter_fields&INGRESS_EGRESS_PORTIDX_BIT) PROC_PRINTF(" %s egress_port_idx:  %d  egress_port_idx_mask: 0x%x\n",(aclPara->filter_fields_inverse&INGRESS_EGRESS_PORTIDX_BIT)?"not":"", aclPara->egress_port_idx,aclPara->egress_port_idx_mask);
	if(aclPara->filter_fields&INGRESS_DSCP_BIT) PROC_PRINTF(" %s ingress_dscp:    %d\n",(aclPara->filter_fields_inverse&INGRESS_DSCP_BIT)?"not":"", aclPara->ingress_dscp);
	if(aclPara->filter_fields&INGRESS_IPV6_DSCP_BIT) PROC_PRINTF(" %s ingress_ipv6_dscp:    %d\n",(aclPara->filter_fields_inverse&INGRESS_IPV6_DSCP_BIT)?"not":"", aclPara->ingress_ipv6_dscp);
	if(aclPara->filter_fields&INGRESS_TOS_BIT) PROC_PRINTF(" %s ingress_tos(ipv4):    %d\n",(aclPara->filter_fields_inverse&INGRESS_TOS_BIT)?"not":"", aclPara->ingress_tos);
	if(aclPara->filter_fields&INGRESS_IPV6_TC_BIT) PROC_PRINTF(" %s ingress_tc(ipv6):    %d\n",(aclPara->filter_fields_inverse&INGRESS_IPV6_TC_BIT)?"not":"", aclPara->ingress_ipv6_tc);
	if(aclPara->filter_fields&INGRESS_INTF_BIT) PROC_PRINTF(" %s ingress_intf_idx: %d\n",(aclPara->filter_fields_inverse&INGRESS_INTF_BIT)?"not":"", aclPara->ingress_intf_idx);
	if(aclPara->filter_fields&EGRESS_INTF_BIT) PROC_PRINTF(" %s egress_intf_idx:  %d\n",(aclPara->filter_fields_inverse&EGRESS_INTF_BIT)?"not":"", aclPara->egress_intf_idx);
	if(aclPara->filter_fields&INGRESS_STREAM_ID_BIT) PROC_PRINTF(" %s ingress_stream_id:  %d\n ingress_stream_id_mask:  0x%x\n",(aclPara->filter_fields_inverse&INGRESS_STREAM_ID_BIT)?"not":"", aclPara->ingress_stream_id, aclPara->ingress_stream_id_mask);
	if(aclPara->filter_fields&INGRESS_ETHERTYPE_BIT) PROC_PRINTF(" %s ingress_ethertype:0x%x  ingress_ethertype_mask:0x%x\n",(aclPara->filter_fields_inverse&INGRESS_ETHERTYPE_BIT)?"not":"", aclPara->ingress_ethertype, aclPara->ingress_ethertype_mask);
	if(aclPara->filter_fields&INGRESS_CTAG_VID_BIT) PROC_PRINTF(" %s ingress_ctag_vid: %d\n",(aclPara->filter_fields_inverse&INGRESS_CTAG_VID_BIT)?"not":"", aclPara->ingress_ctag_vid);
	if(aclPara->filter_fields&INGRESS_CTAG_PRI_BIT) PROC_PRINTF(" %s ingress_ctag_pri: %d\n",(aclPara->filter_fields_inverse&INGRESS_CTAG_PRI_BIT)?"not":"", aclPara->ingress_ctag_pri);
	if(aclPara->filter_fields&INGRESS_CTAG_CFI_BIT) PROC_PRINTF(" %s ingress_ctag_cfi: %d\n",(aclPara->filter_fields_inverse&INGRESS_CTAG_CFI_BIT)?"not":"", aclPara->ingress_ctag_cfi);
	if(aclPara->filter_fields&INGRESS_STAG_VID_BIT) PROC_PRINTF(" %s ingress_stag_vid: %d\n",(aclPara->filter_fields_inverse&INGRESS_STAG_VID_BIT)?"not":"", aclPara->ingress_stag_vid);
	if(aclPara->filter_fields&INGRESS_STAG_PRI_BIT) PROC_PRINTF(" %s ingress_stag_pri: %d\n",(aclPara->filter_fields_inverse&INGRESS_STAG_PRI_BIT)?"not":"", aclPara->ingress_stag_pri);
	if(aclPara->filter_fields&INGRESS_STAG_DEI_BIT) PROC_PRINTF(" %s ingress_stag_dei: %d\n",(aclPara->filter_fields_inverse&INGRESS_STAG_DEI_BIT)?"not":"", aclPara->ingress_stag_dei);
	if(aclPara->filter_fields&INGRESS_SMAC_BIT) PROC_PRINTF(" %s ingress_smac: %02X:%02X:%02X:%02X:%02X:%02X ingress_smac_mask: %02X:%02X:%02X:%02X:%02X:%02X\n", (aclPara->filter_fields_inverse&INGRESS_SMAC_BIT)?"not":"",
																	aclPara->ingress_smac.octet[0],
																	aclPara->ingress_smac.octet[1],
																	aclPara->ingress_smac.octet[2],
																	aclPara->ingress_smac.octet[3],
																	aclPara->ingress_smac.octet[4],
																	aclPara->ingress_smac.octet[5],
																	aclPara->ingress_smac_mask.octet[0],
																	aclPara->ingress_smac_mask.octet[1],
																	aclPara->ingress_smac_mask.octet[2],
																	aclPara->ingress_smac_mask.octet[3],
																	aclPara->ingress_smac_mask.octet[4],
																	aclPara->ingress_smac_mask.octet[5]);
	if(aclPara->filter_fields&INGRESS_DMAC_BIT) PROC_PRINTF(" %s ingress_dmac: %02X:%02X:%02X:%02X:%02X:%02X  ingress_dmac_mask: %02X:%02X:%02X:%02X:%02X:%02X\n",(aclPara->filter_fields_inverse&INGRESS_DMAC_BIT)?"not":"",
																	aclPara->ingress_dmac.octet[0],
																	aclPara->ingress_dmac.octet[1],
																	aclPara->ingress_dmac.octet[2],
																	aclPara->ingress_dmac.octet[3],
																	aclPara->ingress_dmac.octet[4],
																	aclPara->ingress_dmac.octet[5],
																	aclPara->ingress_dmac_mask.octet[0],
																	aclPara->ingress_dmac_mask.octet[1],
																	aclPara->ingress_dmac_mask.octet[2],
																	aclPara->ingress_dmac_mask.octet[3],
																	aclPara->ingress_dmac_mask.octet[4],
																	aclPara->ingress_dmac_mask.octet[5]);
	if(aclPara->filter_fields&EGRESS_SMAC_BIT) PROC_PRINTF(" %s egress_smac: %02X:%02X:%02X:%02X:%02X:%02X\n", (aclPara->filter_fields_inverse&EGRESS_SMAC_BIT)?"not":"",
																		aclPara->egress_smac.octet[0],
																		aclPara->egress_smac.octet[1],
																		aclPara->egress_smac.octet[2],
																		aclPara->egress_smac.octet[3],
																		aclPara->egress_smac.octet[4],
																		aclPara->egress_smac.octet[5]);
	if(aclPara->filter_fields&EGRESS_DMAC_BIT) PROC_PRINTF(" %s egress_dmac: %02X:%02X:%02X:%02X:%02X:%02X\n",(aclPara->filter_fields_inverse&EGRESS_DMAC_BIT)?"not":"",
																		aclPara->egress_dmac.octet[0],
																		aclPara->egress_dmac.octet[1],
																		aclPara->egress_dmac.octet[2],
																		aclPara->egress_dmac.octet[3],
																		aclPara->egress_dmac.octet[4],
																		aclPara->egress_dmac.octet[5]);



	//acl_parameter = &rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter;
	acl_parameter = aclPara;

	if(aclPara->filter_fields&INGRESS_IPV4_SIP_RANGE_BIT){
		PROC_PRINTF(" %s ingress_sip_low_bound: %s ",(aclPara->filter_fields_inverse&INGRESS_IPV4_SIP_RANGE_BIT)?"not":"", inet_ntoa(acl_parameter->ingress_src_ipv4_addr_start));
		PROC_PRINTF(" ingress_sip_up_bound: %s \n", inet_ntoa(acl_parameter->ingress_src_ipv4_addr_end));
	}
	if(aclPara->filter_fields&INGRESS_IPV4_DIP_RANGE_BIT){
		PROC_PRINTF(" %s ingress_dip_low_bound: %s ",(aclPara->filter_fields_inverse&INGRESS_IPV4_DIP_RANGE_BIT)?"not":"", inet_ntoa(acl_parameter->ingress_dest_ipv4_addr_start));
		PROC_PRINTF(" ingress_dip_up_bound: %s \n", inet_ntoa(acl_parameter->ingress_dest_ipv4_addr_end));
	}

	if(aclPara->filter_fields&INGRESS_IPV6_SIP_RANGE_BIT){
		PROC_PRINTF(" %s ingress_src_ipv6_addr_start: %02X%02X:",(aclPara->filter_fields_inverse&INGRESS_IPV6_SIP_RANGE_BIT)?"not":"", acl_parameter->ingress_src_ipv6_addr_start[0],acl_parameter->ingress_src_ipv6_addr_start[1]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_start[2],acl_parameter->ingress_src_ipv6_addr_start[3]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_start[4],acl_parameter->ingress_src_ipv6_addr_start[5]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_start[6],acl_parameter->ingress_src_ipv6_addr_start[7]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_start[8],acl_parameter->ingress_src_ipv6_addr_start[9]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_start[10],acl_parameter->ingress_src_ipv6_addr_start[11]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_start[12],acl_parameter->ingress_src_ipv6_addr_start[13]);
		PROC_PRINTF(" :%02X%02X: \n",acl_parameter->ingress_src_ipv6_addr_start[14],acl_parameter->ingress_src_ipv6_addr_start[15]);

		PROC_PRINTF(" ingress_src_ipv6_addr_end: %02X%02X:",acl_parameter->ingress_src_ipv6_addr_end[0],acl_parameter->ingress_src_ipv6_addr_end[1]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_end[2],acl_parameter->ingress_src_ipv6_addr_end[3]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_end[4],acl_parameter->ingress_src_ipv6_addr_end[5]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_end[6],acl_parameter->ingress_src_ipv6_addr_end[7]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_end[8],acl_parameter->ingress_src_ipv6_addr_end[9]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_end[10],acl_parameter->ingress_src_ipv6_addr_end[11]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_end[12],acl_parameter->ingress_src_ipv6_addr_end[13]);
		PROC_PRINTF(" :%02X%02X: \n",acl_parameter->ingress_src_ipv6_addr_end[14],acl_parameter->ingress_src_ipv6_addr_end[15]);
	}

	if(aclPara->filter_fields&INGRESS_IPV6_DIP_RANGE_BIT){
		PROC_PRINTF(" %s ingress_dest_ipv6_addr_start: %02X%02X:",(aclPara->filter_fields_inverse&INGRESS_IPV6_DIP_RANGE_BIT)?"not":"", acl_parameter->ingress_dest_ipv6_addr_start[0],acl_parameter->ingress_dest_ipv6_addr_start[1]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[2],acl_parameter->ingress_dest_ipv6_addr_start[3]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[4],acl_parameter->ingress_dest_ipv6_addr_start[5]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[6],acl_parameter->ingress_dest_ipv6_addr_start[7]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[8],acl_parameter->ingress_dest_ipv6_addr_start[9]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[10],acl_parameter->ingress_dest_ipv6_addr_start[11]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[12],acl_parameter->ingress_dest_ipv6_addr_start[13]);
		PROC_PRINTF(" :%02X%02X: \n",acl_parameter->ingress_dest_ipv6_addr_start[14],acl_parameter->ingress_dest_ipv6_addr_start[15]);

		PROC_PRINTF(" ingress_dest_ipv6_addr_end: %02X%02X:", acl_parameter->ingress_dest_ipv6_addr_end[0],acl_parameter->ingress_dest_ipv6_addr_end[1]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[2],acl_parameter->ingress_dest_ipv6_addr_end[3]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[4],acl_parameter->ingress_dest_ipv6_addr_end[5]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[6],acl_parameter->ingress_dest_ipv6_addr_end[7]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[8],acl_parameter->ingress_dest_ipv6_addr_end[9]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[10],acl_parameter->ingress_dest_ipv6_addr_end[11]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[12],acl_parameter->ingress_dest_ipv6_addr_end[13]);
		PROC_PRINTF(" :%02X%02X: \n",acl_parameter->ingress_dest_ipv6_addr_end[14],acl_parameter->ingress_dest_ipv6_addr_end[15]);
	}

	if(aclPara->filter_fields&INGRESS_IPV6_SIP_BIT){
		PROC_PRINTF(" %s ingress_src_ipv6_addr: %02X%02X:",(aclPara->filter_fields_inverse&INGRESS_IPV6_SIP_BIT)?"not":"", acl_parameter->ingress_src_ipv6_addr[0],acl_parameter->ingress_src_ipv6_addr[1]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr[2],acl_parameter->ingress_src_ipv6_addr[3]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr[4],acl_parameter->ingress_src_ipv6_addr[5]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr[6],acl_parameter->ingress_src_ipv6_addr[7]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr[8],acl_parameter->ingress_src_ipv6_addr[9]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr[10],acl_parameter->ingress_src_ipv6_addr[11]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr[12],acl_parameter->ingress_src_ipv6_addr[13]);
		PROC_PRINTF(" :%02X%02X: \n",acl_parameter->ingress_src_ipv6_addr[14],acl_parameter->ingress_src_ipv6_addr[15]);

		PROC_PRINTF(" ingress_src_ipv6_addr_mask: %02X%02X:", acl_parameter->ingress_src_ipv6_addr_mask[0],acl_parameter->ingress_src_ipv6_addr_mask[1]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[2],acl_parameter->ingress_src_ipv6_addr_mask[3]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[4],acl_parameter->ingress_src_ipv6_addr_mask[5]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[6],acl_parameter->ingress_src_ipv6_addr_mask[7]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[8],acl_parameter->ingress_src_ipv6_addr_mask[9]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[10],acl_parameter->ingress_src_ipv6_addr_mask[11]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[12],acl_parameter->ingress_src_ipv6_addr_mask[13]);
		PROC_PRINTF(" :%02X%02X: \n",acl_parameter->ingress_src_ipv6_addr_mask[14],acl_parameter->ingress_src_ipv6_addr_mask[15]);
	}

	if(aclPara->filter_fields&INGRESS_IPV6_DIP_BIT){
		PROC_PRINTF(" %s ingress_dest_ipv6_addr: %02X%02X:",(aclPara->filter_fields_inverse&INGRESS_IPV6_DIP_BIT)?"not":"", acl_parameter->ingress_dest_ipv6_addr[0],acl_parameter->ingress_dest_ipv6_addr[1]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[2],acl_parameter->ingress_dest_ipv6_addr[3]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[4],acl_parameter->ingress_dest_ipv6_addr[5]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[6],acl_parameter->ingress_dest_ipv6_addr[7]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[8],acl_parameter->ingress_dest_ipv6_addr[9]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[10],acl_parameter->ingress_dest_ipv6_addr[11]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[12],acl_parameter->ingress_dest_ipv6_addr[13]);
		PROC_PRINTF(" :%02X%02X: \n",acl_parameter->ingress_dest_ipv6_addr[14],acl_parameter->ingress_dest_ipv6_addr[15]);

		PROC_PRINTF(" ingress_dest_ipv6_addr_mask: %02X%02X:", acl_parameter->ingress_dest_ipv6_addr_mask[0],acl_parameter->ingress_dest_ipv6_addr_mask[1]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[2],acl_parameter->ingress_dest_ipv6_addr_mask[3]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[4],acl_parameter->ingress_dest_ipv6_addr_mask[5]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[6],acl_parameter->ingress_dest_ipv6_addr_mask[7]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[8],acl_parameter->ingress_dest_ipv6_addr_mask[9]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[10],acl_parameter->ingress_dest_ipv6_addr_mask[11]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[12],acl_parameter->ingress_dest_ipv6_addr_mask[13]);
		PROC_PRINTF(" :%02X%02X: \n",acl_parameter->ingress_dest_ipv6_addr_mask[14],acl_parameter->ingress_dest_ipv6_addr_mask[15]);
	}

	if(aclPara->filter_fields&INGRESS_L4_SPORT_RANGE_BIT){
		PROC_PRINTF(" %s ingress_src_l4_port_low_bound: %d ", (aclPara->filter_fields_inverse&INGRESS_L4_SPORT_RANGE_BIT)?"not":"", acl_parameter->ingress_src_l4_port_start);
		PROC_PRINTF(" ingress_src_l4_port_up_bound: %d \n", acl_parameter->ingress_src_l4_port_end);
	}
	if(aclPara->filter_fields&INGRESS_L4_DPORT_RANGE_BIT){
		PROC_PRINTF(" %s ingress_dest_l4_port_low_bound: %d  ", (aclPara->filter_fields_inverse&INGRESS_L4_DPORT_RANGE_BIT)?"not":"", acl_parameter->ingress_dest_l4_port_start);
		PROC_PRINTF(" ingress_dest_l4_port_up_bound:  %d \n", acl_parameter->ingress_dest_l4_port_end);
	}

	if(aclPara->filter_fields&EGRESS_IPV4_SIP_RANGE_BIT){
		PROC_PRINTF(" %s egress_sip_low_bound: %s	", (aclPara->filter_fields_inverse&EGRESS_IPV4_SIP_RANGE_BIT)?"not":"", diag_util_inet_ntoa(acl_parameter->egress_src_ipv4_addr_start));
		PROC_PRINTF(" egress_sip_up_bound:	%s \n", diag_util_inet_ntoa(acl_parameter->egress_src_ipv4_addr_end));
	}
	if(aclPara->filter_fields&EGRESS_IPV4_DIP_RANGE_BIT){
		PROC_PRINTF(" %s egress_dip_low_bound: %s	", (aclPara->filter_fields_inverse&EGRESS_IPV4_DIP_RANGE_BIT)?"not":"", diag_util_inet_ntoa(acl_parameter->egress_dest_ipv4_addr_start));
		PROC_PRINTF(" egress_dip_up_bound:	%s \n", diag_util_inet_ntoa(acl_parameter->egress_dest_ipv4_addr_end));
	}
	if(aclPara->filter_fields&EGRESS_L4_SPORT_RANGE_BIT){
		PROC_PRINTF(" %s egress_src_l4_port_low_bound: %d	", (aclPara->filter_fields_inverse&EGRESS_L4_SPORT_RANGE_BIT)?"not":"", acl_parameter->egress_src_l4_port_start);
		PROC_PRINTF(" egress_src_l4_port_up_bound:	%d \n", acl_parameter->egress_src_l4_port_end);
	}
	if(aclPara->filter_fields&EGRESS_L4_DPORT_RANGE_BIT){
		PROC_PRINTF(" %s egress_dest_l4_port_low_bound: %d ", (aclPara->filter_fields_inverse&EGRESS_L4_DPORT_RANGE_BIT)?"not":"", acl_parameter->egress_dest_l4_port_start);
		PROC_PRINTF(" egress_dest_l4_port_up_bound: %d \n", acl_parameter->egress_dest_l4_port_end);
	}

	if(aclPara->filter_fields&EGRESS_CTAG_VID_BIT){
		PROC_PRINTF(" %s egress_ctag_vid:	%d \n", (aclPara->filter_fields_inverse&EGRESS_CTAG_VID_BIT)?"not":"",acl_parameter->egress_ctag_vid);
		PROC_PRINTF(" egress_ctag_vid_mask: 0x%x \n", acl_parameter->egress_ctag_vid_mask);
	}

	if(aclPara->filter_fields&EGRESS_CTAG_PRI_BIT) PROC_PRINTF(" %s egress_ctag_pri:  %d \n",(aclPara->filter_fields_inverse&EGRESS_CTAG_PRI_BIT)?"not":"", acl_parameter->egress_ctag_pri);


	if(acl_parameter->filter_fields & INGRESS_IPV4_TAGIF_BIT){
		PROC_PRINTF(" %s ipv4_tagIf: %s \n",(aclPara->filter_fields_inverse&INGRESS_IPV4_TAGIF_BIT)?"not":"",acl_parameter->ingress_ipv4_tagif?"Must be IPv4":"Must not be IPv4");
	}

	if(acl_parameter->filter_fields & INGRESS_IPV6_TAGIF_BIT){
		PROC_PRINTF(" %s ipv6_tagIf: %s \n",(aclPara->filter_fields_inverse&INGRESS_IPV6_TAGIF_BIT)?"not":"",acl_parameter->ingress_ipv6_tagif?"Must be IPv6":"Must not be IPv6");
	}

	if(acl_parameter->filter_fields & EGRESS_IP4MC_IF){
		PROC_PRINTF(" %s egress_ip4mc_if: %s \n",(aclPara->filter_fields_inverse&EGRESS_IP4MC_IF)?"not":"",acl_parameter->egress_ip4mc_if?"Must be IP4MC":"Must not be IP4MC");
	}
	if(acl_parameter->filter_fields & EGRESS_IP6MC_IF){
		PROC_PRINTF(" %s egress_ip6mc_if: %s \n",(aclPara->filter_fields_inverse&EGRESS_IP6MC_IF)?"not":"",acl_parameter->egress_ip6mc_if?"Must be IP6MC":"Must not be IP6MC");
	}

	if(acl_parameter->filter_fields & INGRESS_L4_TCP_BIT){
		PROC_PRINTF(" %s l4-protocal: tcp \n",(aclPara->filter_fields_inverse&INGRESS_L4_TCP_BIT)?"not":"");
	}else if(acl_parameter->filter_fields & INGRESS_L4_UDP_BIT){
		PROC_PRINTF(" %s l4-protocal: udp \n",(aclPara->filter_fields_inverse&INGRESS_L4_UDP_BIT)?"not":"");
	}else if(acl_parameter->filter_fields & INGRESS_L4_ICMP_BIT){
		PROC_PRINTF(" %s l4-protocal: icmp \n",(aclPara->filter_fields_inverse&INGRESS_L4_ICMP_BIT)?"not":"");
	}else if(acl_parameter->filter_fields & INGRESS_L4_NONE_TCP_NONE_UDP_BIT){
		PROC_PRINTF(" %s l4-protocal: none-tcp & none-udp \n",(aclPara->filter_fields_inverse&INGRESS_L4_NONE_TCP_NONE_UDP_BIT)?"not":"");
	}else if(acl_parameter->filter_fields & INGRESS_L4_ICMPV6_BIT){
		PROC_PRINTF(" %s l4-protocal: icmpv6 \n",(aclPara->filter_fields_inverse&INGRESS_L4_ICMPV6_BIT)?"not":"");
	}else{
		//PROC_PRINTF(" l4-protocal: not care \n");
	}

	if(acl_parameter->filter_fields & INGRESS_L4_POROTCAL_VALUE_BIT){
		PROC_PRINTF(" %s l4-protocal-value: %d \n",(aclPara->filter_fields_inverse&INGRESS_L4_POROTCAL_VALUE_BIT)?"not":"",acl_parameter->ingress_l4_protocal);
	}

	if(acl_parameter->filter_fields & INGRESS_STAGIF_BIT){
		if(aclPara->ingress_stagIf){
			PROC_PRINTF(" %s stagIf: must have Stag \n",(aclPara->filter_fields_inverse&INGRESS_STAGIF_BIT)?"not":"");
		}else{
			PROC_PRINTF(" %s stagIf: must not have Stag \n",(aclPara->filter_fields_inverse&INGRESS_STAGIF_BIT)?"not":"");
		}
	}
	if(acl_parameter->filter_fields & INGRESS_CTAGIF_BIT){
		if(aclPara->ingress_ctagIf){
			PROC_PRINTF(" %s ctagIf: must have Ctag \n",(aclPara->filter_fields_inverse&INGRESS_CTAGIF_BIT)?"not":"");
		}else{
			PROC_PRINTF(" %s ctagIf: must not have Ctag \n",(aclPara->filter_fields_inverse&INGRESS_CTAGIF_BIT)?"not":"");
		}
	}

	if(acl_parameter->filter_fields & INTERNAL_PRI_BIT){
		PROC_PRINTF(" %s internal priroity: %d \n",(aclPara->filter_fields_inverse&INTERNAL_PRI_BIT)?"not":"",acl_parameter->internal_pri);
	}

	if(acl_parameter->filter_fields & INGRESS_IPV6_FLOWLABEL_BIT){
		PROC_PRINTF(" %s ingress_ipv6_flow_label: %d \n",(aclPara->filter_fields_inverse&INGRESS_IPV6_FLOWLABEL_BIT)?"not":"",acl_parameter->ingress_ipv6_flow_label);
	}

	if(acl_parameter->filter_fields & INGRESS_WLANDEV_BIT){
		PROC_PRINTF(" %s ingress_wlanDevMask: 0x%x \n",(aclPara->filter_fields_inverse&INGRESS_WLANDEV_BIT)?"not":"",acl_parameter->ingress_wlanDevMask);
	}

	if(acl_parameter->filter_fields & EGRESS_WLANDEV_BIT){
		PROC_PRINTF(" %s egress_wlanDevMask: 0x%x \n",(aclPara->filter_fields_inverse&EGRESS_WLANDEV_BIT)?"not":"",acl_parameter->egress_wlanDevMask);
	}

	PROC_PRINTF("[Actions]: \n");
	switch(acl_parameter->action_type){
		case ACL_ACTION_TYPE_DROP:
			PROC_PRINTF("action_type: ACL_ACTION_TYPE_DROP \n");
			break;
		case ACL_ACTION_TYPE_PERMIT:
			PROC_PRINTF("action type: ACL_ACTION_TYPE_PERMIT \n");
			break;
		case ACL_ACTION_TYPE_TRAP:
			PROC_PRINTF("action type: ACL_ACTION_TYPE_TRAP \n");
			break;
		case ACL_ACTION_TYPE_TRAP_TO_PS:
			PROC_PRINTF("action type: ACL_ACTION_TYPE_TRAP_TO_PS \n");
			break;
		case ACL_ACTION_TYPE_QOS:
			PROC_PRINTF("action type: ACL_ACTION_TYPE_QOS \n");
			PROC_PRINTF("qos_actions_bits: 0x%x\n",acl_parameter->qos_actions);
			if(acl_parameter->qos_actions&ACL_ACTION_1P_REMARKING_BIT) PROC_PRINTF(" dot1p_remarking: %d \n",acl_parameter->action_dot1p_remarking_pri);
			if(acl_parameter->qos_actions&ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT) PROC_PRINTF(" ip_precedence_remarking: %d\n",acl_parameter->action_ip_precedence_remarking_pri);
			if(acl_parameter->qos_actions&ACL_ACTION_DSCP_REMARKING_BIT) PROC_PRINTF(" dscp_remarking: %d\n",acl_parameter->action_dscp_remarking_pri);
			if(acl_parameter->qos_actions&ACL_ACTION_TOS_TC_REMARKING_BIT) PROC_PRINTF(" tos_tc_remarking: %d\n",acl_parameter->action_tos_tc_remarking_pri);
			if(acl_parameter->qos_actions&ACL_ACTION_QUEUE_ID_BIT) PROC_PRINTF(" queue_id: %d\n",acl_parameter->action_queue_id);
			if(acl_parameter->qos_actions&ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT) PROC_PRINTF(" egress_internal_priority(cfpri): %d\n",acl_parameter->egress_internal_priority);
			if(acl_parameter->qos_actions&ACL_ACTION_SHARE_METER_BIT) PROC_PRINTF(" share_meter: %d\n",acl_parameter->action_share_meter);
			if(acl_parameter->qos_actions&ACL_ACTION_LOG_COUNTER_BIT) PROC_PRINTF(" logging_counter: %d\n",acl_parameter->action_log_counter);
			if(acl_parameter->qos_actions&ACL_ACTION_STREAM_ID_OR_LLID_BIT) PROC_PRINTF(" stream id: %d\n",acl_parameter->action_stream_id_or_llid);
			if(acl_parameter->qos_actions&ACL_ACTION_ACL_PRIORITY_BIT) PROC_PRINTF(" acl priority: %d\n",acl_parameter->action_acl_priority);
			if(acl_parameter->qos_actions&ACL_ACTION_ACL_INGRESS_VID_BIT) PROC_PRINTF(" ingress cvid: %d\n",acl_parameter->action_acl_ingress_vid);
			if(acl_parameter->qos_actions&ACL_ACTION_DS_UNIMASK_BIT) PROC_PRINTF(" ds uni portmask: 0x%x\n",acl_parameter->downstream_uni_portmask);
			if(acl_parameter->qos_actions&ACL_ACTION_REDIRECT_BIT) PROC_PRINTF(" redirect to portmaks: 0x%x\n",acl_parameter->redirect_portmask);
			if(acl_parameter->qos_actions&ACL_ACTION_ACL_CVLANTAG_BIT)
			{
				PROC_PRINTF(" cvlan: %s\n  cvid_act:%s\n  cpri_act:%s\n  cvid:%d\n  cpri:%d\n",
					name_of_rg_cvlan_tagif_decision[acl_parameter->action_acl_cvlan.cvlanTagIfDecision],
					name_of_rg_cvlan_cvid_decision[acl_parameter->action_acl_cvlan.cvlanCvidDecision],
					name_of_rg_cvlan_cpri_decision[acl_parameter->action_acl_cvlan.cvlanCpriDecision],
					acl_parameter->action_acl_cvlan.assignedCvid,
					acl_parameter->action_acl_cvlan.assignedCpri);
			}
			if(acl_parameter->qos_actions&ACL_ACTION_ACL_SVLANTAG_BIT)
			{
				PROC_PRINTF(" svlan: %s\n  svid_act:%s\n  spri_act:%s\n  svid:%d\n  spri:%d\n",
					name_of_rg_svlan_tagif_decision[acl_parameter->action_acl_svlan.svlanTagIfDecision],
					name_of_rg_svlan_svid_decision[acl_parameter->action_acl_svlan.svlanSvidDecision],
					name_of_rg_svlan_spri_decision[acl_parameter->action_acl_svlan.svlanSpriDecision],
					acl_parameter->action_acl_svlan.assignedSvid,
					acl_parameter->action_acl_svlan.assignedSpri
				);

			}
			break;
		case ACL_ACTION_TYPE_POLICY_ROUTE:
			PROC_PRINTF("action type: ACL_ACTION_TYPE_POLICY_ROUTE \n");
			PROC_PRINTF("policy_route_wan: 0x%x\n",acl_parameter->action_policy_route_wan);
			break;

		case ACL_ACTION_TYPE_SW_MIRROR_WITH_UDP_ENCAP:
			PROC_PRINTF("action type: ACL_ACTION_TYPE_SW_MIRROR_WITH_UDP_ENCAP \n");
			if(acl_parameter->action_encap_udp.no_encap){
				PROC_PRINTF(" no_encap\n");
				if(acl_parameter->action_encap_udp.remake_smac)PROC_PRINTF(" remake_smac\n");
			}else{
				PROC_PRINTF(" encap_smac: %02X:%02X:%02X:%02X:%02X:%02X\n",
					acl_parameter->action_encap_udp.encap_smac.octet[0],
					acl_parameter->action_encap_udp.encap_smac.octet[1],
					acl_parameter->action_encap_udp.encap_smac.octet[2],
					acl_parameter->action_encap_udp.encap_smac.octet[3],
					acl_parameter->action_encap_udp.encap_smac.octet[4],
					acl_parameter->action_encap_udp.encap_smac.octet[5]);
				PROC_PRINTF(" encap_dmac: %02X:%02X:%02X:%02X:%02X:%02X\n",
					acl_parameter->action_encap_udp.encap_dmac.octet[0],
					acl_parameter->action_encap_udp.encap_dmac.octet[1],
					acl_parameter->action_encap_udp.encap_dmac.octet[2],
					acl_parameter->action_encap_udp.encap_dmac.octet[3],
					acl_parameter->action_encap_udp.encap_dmac.octet[4],
					acl_parameter->action_encap_udp.encap_dmac.octet[5]);
				PROC_PRINTF(" encap_ip_ipv6:%d\n",acl_parameter->action_encap_udp.encap_ip_ipv6);
				if(acl_parameter->action_encap_udp.encap_ip_ipv6){
					PROC_PRINTF(" encap_sip_ipv6: %02X%02X:",acl_parameter->action_encap_udp.encap_Sip_ipv6[0],acl_parameter->action_encap_udp.encap_Sip_ipv6[1]);
					PROC_PRINTF(" :%02X%02X:",acl_parameter->action_encap_udp.encap_Sip_ipv6[2],acl_parameter->action_encap_udp.encap_Sip_ipv6[3]);
					PROC_PRINTF(" :%02X%02X:",acl_parameter->action_encap_udp.encap_Sip_ipv6[4],acl_parameter->action_encap_udp.encap_Sip_ipv6[5]);
					PROC_PRINTF(" :%02X%02X:",acl_parameter->action_encap_udp.encap_Sip_ipv6[6],acl_parameter->action_encap_udp.encap_Sip_ipv6[7]);
					PROC_PRINTF(" :%02X%02X:",acl_parameter->action_encap_udp.encap_Sip_ipv6[8],acl_parameter->action_encap_udp.encap_Sip_ipv6[9]);
					PROC_PRINTF(" :%02X%02X:",acl_parameter->action_encap_udp.encap_Sip_ipv6[10],acl_parameter->action_encap_udp.encap_Sip_ipv6[11]);
					PROC_PRINTF(" :%02X%02X:",acl_parameter->action_encap_udp.encap_Sip_ipv6[12],acl_parameter->action_encap_udp.encap_Sip_ipv6[13]);
					PROC_PRINTF(" :%02X%02X: \n",acl_parameter->action_encap_udp.encap_Sip_ipv6[14],acl_parameter->action_encap_udp.encap_Sip_ipv6[15]);

					PROC_PRINTF(" encap_dip_ipv6: %02X%02X:", acl_parameter->action_encap_udp.encap_Dip_ipv6[0],acl_parameter->action_encap_udp.encap_Dip_ipv6[1]);
					PROC_PRINTF(" :%02X%02X:",acl_parameter->action_encap_udp.encap_Dip_ipv6[2],acl_parameter->action_encap_udp.encap_Dip_ipv6[3]);
					PROC_PRINTF(" :%02X%02X:",acl_parameter->action_encap_udp.encap_Dip_ipv6[4],acl_parameter->action_encap_udp.encap_Dip_ipv6[5]);
					PROC_PRINTF(" :%02X%02X:",acl_parameter->action_encap_udp.encap_Dip_ipv6[6],acl_parameter->action_encap_udp.encap_Dip_ipv6[7]);
					PROC_PRINTF(" :%02X%02X:",acl_parameter->action_encap_udp.encap_Dip_ipv6[8],acl_parameter->action_encap_udp.encap_Dip_ipv6[9]);
					PROC_PRINTF(" :%02X%02X:",acl_parameter->action_encap_udp.encap_Dip_ipv6[10],acl_parameter->action_encap_udp.encap_Dip_ipv6[11]);
					PROC_PRINTF(" :%02X%02X:",acl_parameter->action_encap_udp.encap_Dip_ipv6[12],acl_parameter->action_encap_udp.encap_Dip_ipv6[13]);
					PROC_PRINTF(" :%02X%02X: \n",acl_parameter->action_encap_udp.encap_Dip_ipv6[14],acl_parameter->action_encap_udp.encap_Dip_ipv6[15]);
				}else{
					PROC_PRINTF(" encap_sip: %d.%d.%d.%d\n",
						(acl_parameter->action_encap_udp.encap_Sip & 0xff000000)>>24,
						(acl_parameter->action_encap_udp.encap_Sip & 0xff0000)>>16,
						(acl_parameter->action_encap_udp.encap_Sip & 0xff00)>>8,
						(acl_parameter->action_encap_udp.encap_Sip & 0xff));
					PROC_PRINTF(" encap_dip: %d.%d.%d.%d\n",
						(acl_parameter->action_encap_udp.encap_Dip & 0xff000000)>>24,
						(acl_parameter->action_encap_udp.encap_Dip & 0xff0000)>>16,
						(acl_parameter->action_encap_udp.encap_Dip & 0xff00)>>8,
						(acl_parameter->action_encap_udp.encap_Dip & 0xff));
				}
				PROC_PRINTF(" encap_dport:%d\n",acl_parameter->action_encap_udp.encap_Dport);
			}
			PROC_PRINTF(" mirror_count:%d\n",acl_parameter->action_encap_udp.mirror_count);
			break;

		case ACL_ACTION_TYPE_FLOW_MIB:
			PROC_PRINTF("action type: ACL_ACTION_TYPE_FLOW_MIB \n");
			PROC_PRINTF("flowmib_counter_index: %d\n",acl_parameter->action_flowmib_counter_idx);
			break;

		case ACL_ACTION_TYPE_TRAP_WITH_PRIORITY:
			PROC_PRINTF("action type: ACL_ACTION_TYPE_TRAP_WITH_PRIORITY \n");
			PROC_PRINTF("acl_priority: %d\n",acl_parameter->action_trap_with_priority);
			break;

		case ACL_ACTION_TYPE_SW_DROP:
			PROC_PRINTF("action_type: ACL_ACTION_TYPE_SW_DROP \n");
			break;

		default:
			break;
	}

	return 0;
}


static int _rtk_rg_aclSWEntry_not_support(rtk_rg_aclFilterAndQos_t *acl_filter)
{
	//int acl_SWEntry_index;
    rtk_portmask_t mac_pmsk;
	rtk_portmask_t ext_pmsk;

	//rtk_rg_aclFilterEntry_t aclSWEntry;


	ASSERT_EQ(_rtk_rg_portmask_translator(acl_filter->ingress_port_mask, &mac_pmsk, &ext_pmsk),RT_ERR_RG_OK);

	//SVLAN action not support feature
	if((acl_filter->action_type==ACL_ACTION_TYPE_QOS) && (acl_filter->qos_actions & ACL_ACTION_ACL_SVLANTAG_BIT))
	{
		if(acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_8100 ||acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C)
			RETURN_ERR(RT_ERR_RG_ACL_NOT_SUPPORT);

		if(acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2 ||acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_ORIGINAL_STAG_TPID)
			RETURN_ERR(RT_ERR_RG_ACL_NOT_SUPPORT);

		if(acl_filter->action_acl_svlan.svlanSvidDecision==ACL_SVLAN_SVID_SP2C)
			RETURN_ERR(RT_ERR_RG_ACL_NOT_SUPPORT);

		if(acl_filter->action_acl_svlan.svlanSpriDecision==ACL_SVLAN_SPRI_COPY_FROM_DSCP_REMAP || acl_filter->action_acl_svlan.svlanSpriDecision==ACL_SVLAN_SPRI_COPY_FROM_SP2C)
			RETURN_ERR(RT_ERR_RG_ACL_NOT_SUPPORT);

		if(!((acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN) ||
			(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN)))
			RETURN_ERR(RT_ERR_RG_ACL_NOT_SUPPORT);
	}

	//CVLAN action not support feature
	if((acl_filter->action_type==ACL_ACTION_TYPE_QOS) && (acl_filter->qos_actions & ACL_ACTION_ACL_CVLANTAG_BIT))
	{
		if(acl_filter->action_acl_cvlan.cvlanTagIfDecision==ACL_CVLAN_TAGIF_TAGGING_WITH_C2S || acl_filter->action_acl_cvlan.cvlanTagIfDecision==ACL_CVLAN_TAGIF_TAGGING_WITH_SP2C)
			RETURN_ERR(RT_ERR_RG_ACL_NOT_SUPPORT);

		if(acl_filter->action_acl_cvlan.cvlanCvidDecision==ACL_CVLAN_CVID_CPOY_FROM_SP2C || acl_filter->action_acl_cvlan.cvlanCvidDecision==ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID)
			RETURN_ERR(RT_ERR_RG_ACL_NOT_SUPPORT);

		if(acl_filter->action_acl_cvlan.cvlanCpriDecision==ACL_CVLAN_CPRI_COPY_FROM_DSCP_REMAP || acl_filter->action_acl_cvlan.cvlanCpriDecision==ACL_CVLAN_CPRI_COPY_FROM_SP2C)
			RETURN_ERR(RT_ERR_RG_ACL_NOT_SUPPORT);

		if(!((acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN) ||
			(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN)))
			RETURN_ERR(RT_ERR_RG_ACL_NOT_SUPPORT);
	}

#if 0	//not support ACL_UNI_FORCE_BY_MASK, but ACL use ACL_UNI_FWD_TO_PORTMASK_ONLY
	//FB not support uni action to multi-port
	if((acl_filter->qos_actions & ACL_ACTION_DS_UNIMASK_BIT))
	{
		if(acl_filter->downstream_uni_portmask!=0x0)
		{
			WARNING("apolloPro testchip not support ACL_ACTION_DS_UNIMASK_BIT action to redirect or to multi-port");
			RETURN_ERR(RT_ERR_RG_ACL_NOT_SUPPORT);
		}
	}
#endif

	//FB not support egress to multi-port action (path5 can only forward to single port)
	if((acl_filter->qos_actions & ACL_ACTION_REDIRECT_BIT))
	{
			WARNING("apolloPro not support ction ACL_ACTION_REDIRECT_BIT and ACL_ACTION_DS_UNIMASK_BIT");
			RETURN_ERR(RT_ERR_RG_ACL_NOT_SUPPORT);
	}

	if((acl_filter->qos_actions & ACL_ACTION_SHARE_METER_BIT))
	{
		//only support share meter index 0~31(flow), 48~79(software)
		if(acl_filter->action_share_meter >= PURE_SW_METER_IDX_OFFSET+PURE_SW_SHAREMETER_TABLE_SIZE)
		{
			WARNING("apolloPro ACL action share meter must less than %d", PURE_SW_METER_IDX_OFFSET+PURE_SW_SHAREMETER_TABLE_SIZE);
			RETURN_ERR(RT_ERR_RG_ACL_NOT_SUPPORT);
		}
		else if((acl_filter->action_share_meter >= FLOWBASED_TABLESIZE_SHAREMTR) && (acl_filter->action_share_meter < PURE_SW_METER_IDX_OFFSET))
		{
			WARNING("apolloPro ACL action not support share meter index from %d to %d", FLOWBASED_TABLESIZE_SHAREMTR, PURE_SW_METER_IDX_OFFSET-1);
			RETURN_ERR(RT_ERR_RG_ACL_NOT_SUPPORT);
		}
	}


	//SP2C/C2S actions is not belong to tagDecision layer in apolloFE, it should moved to vidDecision layer.
	if(acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C)
	{
		WARNING("SP2C action should be set at svlanSvidDecision layer");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
	if(acl_filter->action_acl_cvlan.cvlanTagIfDecision==ACL_CVLAN_TAGIF_TAGGING_WITH_C2S || acl_filter->action_acl_cvlan.cvlanTagIfDecision==ACL_CVLAN_TAGIF_TAGGING_WITH_SP2C)
	{
		WARNING("SP2C/C2S action should be set at cvlanCvidDecision layer");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	if(acl_filter->qos_actions & ACL_ACTION_QUEUE_ID_BIT)
	{
		WARNING("ACL_ACTION_QUEUE_ID_BIT is not support, please use ACL_ACTION_ACL_PRIORITY_BIT");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	if(acl_filter->qos_actions & ACL_ACTION_LOG_COUNTER_BIT)
	{
		WARNING("ACL_ACTION_LOG_COUNTER_BIT is not support, please use FB counter");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}


#if 0 //20170512: Apollo Pro should support IPv6 range check
	if(acl_filter->filter_fields & INGRESS_IPV6_SIP_RANGE_BIT)
	{
		WARNING("INGRESS_IPV6_SIP_RANGE_BIT is not support, please use INGRESS_IPV6_SIP_BIT");
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_DIP_RANGE_BIT)
	{
		WARNING("INGRESS_IPV6_DIP_RANGE_BIT is not support, please use INGRESS_IPV6_DIP_BIT");
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	}
#endif

#if 0 	//G3 HW ACL use the same field for ip_protocol, so remove this limition
	//HW ACL set ipv4/ipv6 l4_protocal in different template field, so need to indicate ipv4 or ipv6. (supporting HW ACL drop)
	if(acl_filter->filter_fields & INGRESS_L4_POROTCAL_VALUE_BIT)
	{
		if(((acl_filter->filter_fields & INGRESS_IPV4_TAGIF_BIT)==0x0 && (acl_filter->filter_fields & INGRESS_IPV6_TAGIF_BIT)==0x0)
			 || (acl_filter->ingress_ipv4_tagif==0 && acl_filter->ingress_ipv6_tagif==0)){
			WARNING("INGRESS_L4_POROTCAL_VALUE_BIT should indicate ipv4 or ipv6. if H/W drop is need, please also assign INGRESS_IPV4_TAGIF_BIT or INGRESS_IPV6_TAGIF_BIT.");
		}
	}
#endif

	if((acl_filter->filter_fields&INGRESS_WLANDEV_BIT) && (acl_filter->filter_fields&INGRESS_PORT_BIT))
	{
		if(acl_filter->ingress_port_mask.portmask & ~RTK_RG_ALL_EXT_PORTMASK){
			WARNING("INGRESS_PORT_BIT only support EXT port when config INGRESS_WLANDEV_BIT at the same time");
		}
	}

	if(acl_filter->filter_fields&INGRESS_TCP_FLAGS_BIT)
	{
		WARNING("INGRESS_TCP_FLAGS_BIT still under development");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	if(acl_filter->filter_fields&INGRESS_PKT_LEN_RANGE_BIT)
	{
		WARNING("INGRESS_PKT_LEN_RANGE_BIT still under development");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	//acl-priroity is decided when ingress, not support with egress pattern
	if(acl_filter->qos_actions & ACL_ACTION_ACL_PRIORITY_BIT)
	{
		if((acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET) && (acl_filter->filter_fields & EGRESS_INTF_BIT))
		{
			//Support this pattern to configure acl priority action
		}
		else if(acl_filter->fwding_type_and_direction!=ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
		{
			WARNING("ACL_ACTION_ACL_PRIORITY_BIT is only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET, please use ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT!");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
		else if((acl_filter->filter_fields & EGRESS_ACL_PATTERN_BITS)!=0x0)
		{
			WARNING("ACL_ACTION_ACL_PRIORITY_BIT decision is in ingress, rule can not support such action with egress pattern!");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}

	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
	{
		if(acl_filter->filter_fields & (
			INTERNAL_PRI_BIT|
			EGRESS_CTAG_PRI_BIT|
			EGRESS_CTAG_VID_BIT) )
		{
			WARNING("fwdtype==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET with not supported pattern.");
			RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
		}
	}

	if(acl_filter->action_type==ACL_ACTION_TYPE_POLICY_ROUTE){
		if(rg_db.systemGlobal.interfaceInfo[acl_filter->action_policy_route_wan].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_BRIDGE)
		{
			WARNING("ACL_ACTION_TYPE_POLICY_ROUTE not support Bridge WAN interface!");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
		else if((acl_filter->filter_fields & EGRESS_ACL_PATTERN_BITS)!=0x0)
		{
			WARNING("ACL_ACTION_TYPE_POLICY_ROUTE decision is in ingress, rule can not support such action with egress pattern!");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}
	//20170310CHUCK: supporting egress using permit/trap_to_ps but limited egress pattern
	if(acl_filter->action_type==ACL_ACTION_TYPE_TRAP_TO_PS || acl_filter->action_type==ACL_ACTION_TYPE_PERMIT)
	{
		if(acl_filter->filter_fields & ~(
			INGRESS_PORT_BIT|
			INGRESS_INTF_BIT|
			INGRESS_ETHERTYPE_BIT|
			INGRESS_CTAG_PRI_BIT|
			INGRESS_CTAG_VID_BIT|
			INGRESS_SMAC_BIT|
			INGRESS_DMAC_BIT|
			INGRESS_DSCP_BIT|
			INGRESS_L4_TCP_BIT|
			INGRESS_L4_UDP_BIT|
			INGRESS_IPV6_SIP_RANGE_BIT|
			INGRESS_IPV6_DIP_RANGE_BIT|
			INGRESS_IPV4_SIP_RANGE_BIT|
			INGRESS_IPV4_DIP_RANGE_BIT|
			INGRESS_L4_SPORT_RANGE_BIT|
			INGRESS_L4_DPORT_RANGE_BIT|
			INGRESS_L4_ICMP_BIT|
			INGRESS_IPV6_DSCP_BIT|
			INGRESS_STREAM_ID_BIT|
			INGRESS_STAG_PRI_BIT|
			INGRESS_STAG_VID_BIT|
			INGRESS_STAGIF_BIT|
			INGRESS_CTAGIF_BIT|
			INGRESS_L4_POROTCAL_VALUE_BIT|
			INGRESS_TOS_BIT|
			INGRESS_IPV6_TC_BIT|
			INGRESS_IPV6_SIP_BIT|
			INGRESS_IPV6_DIP_BIT|
			INGRESS_WLANDEV_BIT|
			INGRESS_IPV4_TAGIF_BIT|
			INGRESS_IPV6_TAGIF_BIT|
			INGRESS_L4_ICMPV6_BIT |
			INGRESS_CTAG_CFI_BIT|
			INGRESS_STAG_DEI_BIT|
			EGRESS_INTF_BIT|
			EGRESS_IPV4_SIP_RANGE_BIT|
			EGRESS_IPV4_DIP_RANGE_BIT|
			EGRESS_L4_SPORT_RANGE_BIT|
			EGRESS_L4_DPORT_RANGE_BIT|
			EGRESS_IP4MC_IF|
			EGRESS_IP6MC_IF|
			EGRESS_DMAC_BIT|
			EGRESS_SMAC_BIT|
			EGRESS_WLANDEV_BIT) )
		{
			WARNING("fwdtype==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET with not supported pattern.");
			RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
		}
		else
		{
			//only aboved pattern can be support when ACL_ACTION_TYPE_TRAP_TO_PS/ACL_ACTION_TYPE_PERMIT
		}



	}


	//check supported actions
	if(acl_filter->action_type==ACL_ACTION_TYPE_DROP ||
		acl_filter->action_type==ACL_ACTION_TYPE_PERMIT ||
		acl_filter->action_type==ACL_ACTION_TYPE_TRAP ||
		acl_filter->action_type==ACL_ACTION_TYPE_QOS ||
		acl_filter->action_type==ACL_ACTION_TYPE_TRAP_TO_PS ||
		acl_filter->action_type==ACL_ACTION_TYPE_POLICY_ROUTE ||
		acl_filter->action_type==ACL_ACTION_TYPE_SW_MIRROR_WITH_UDP_ENCAP||
		acl_filter->action_type==ACL_ACTION_TYPE_TRAP_WITH_PRIORITY||
		acl_filter->action_type==ACL_ACTION_TYPE_FLOW_MIB||
		acl_filter->action_type==ACL_ACTION_TYPE_SW_DROP
		)
	{
		//action can be support.
	}
	else
	{
		WARNING("ACL action_type is not suported by this platform!");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}


	return (RT_ERR_RG_OK);
}

