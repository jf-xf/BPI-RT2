#include <rtk_rg_apolloPro_liteRomeDriver.h>
#include <rtk_rg_acl.h>
#include <rtk_rg_apolloPro_acl.h>
//#include <rtk_rg_apolloPro_liteRomeDriver.h>
#include <dal/rtl9607c/dal_rtl9607c_hwmisc.h>



/*H/W ACL debug string, check rtk_acl_field_type_t rather than rtl9607c_dal_acl_fieldTypes_t */
char *name_of_acl_field[]={
	"", "",
	"DMAC0[15:0]", // 2
	"DMAC1[31:16]",
	"DMAC2[47:32]",
	"",
	"SMAC0[15:0]",	//6
	"SMAC1[31:16]",
	"SMAC2[47:32]",
	"ETHERTYPE", //9
	"CTAG",	//a
	"STAG",	//b
	"GEMIDX/LLIDX", 	//0xc
	"FRAME/TAGS",	//0xd
	"",
	"IP4SIP[15:0]", //0xf
	"IP4SIP[31:16]", //0x10
	"",
	"IP4DIP[15:0]",
	"IP4DIP[31:16]",
	"IP4(TOS+PROTO)", //0x14
	"", //0x15
	"",
	"INNER_IP4SIP[15:0]",//0X17
	"INNER_IP4SIP[31:16]",
	"",
	"INNER_IP4DIP[15:0]", //0x1a
	"INNER_IP4DIP[31:16]",
	"INNER_IP4(TOS+PROTO)", //0X1c
	"",//0x1d
	"IP6SIP[15:0]", //0x1e
	"IP6SIP[31:16]",
	"IP6SIP[47:32]",
	"IP6SIP[63:48]",
	"IP6SIP[79:64]",
	"IP6SIP[95:80]",
	"IP6SIP[111:96]",
	"IP6SIP[127:112]", //0x25
	"",
	"IP6DIP[15:0]", //0x27
	"IP6DIP[31:16]",
	"IP6DIP[47:32]",
	"IP6DIP[63:48]",
	"IP6DIP[79:64]",
	"IP6DIP[95:80]",
	"IP6DIP[111:96]",
	"IP6DIP[127:112]",//0x2e
	"IP6(TC+NH)", //0x2f
	"TCPSPORT",//x0x30
	"TCPDPORT",
	"UDPSPORT",
	"UDPDPORT",
	"VIDRANGE",//x0x34
	"IPRANGE",
	"PORTRANGE",
	"PKTLENRANGE",
	"","","","",//0x38~0x3b
	"EXT_PORT_MASK",
	"FIELD_VALID",//3d
	"FS0(IPCP/IP6CP)",//0x3e
	"FS1(IPfrag)",
	"FS2(TCPflags)",
	"FS3(v6FlowLabel[0:3])",
	"FS4(v6FlowLabel[4:19])",
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
static int _rtk_rg_aclSWEntry_and_asic_reAdd(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx);
static int _rtk_rg_aclSWEntry_to_asic_add(rtk_rg_aclFilterAndQos_t *acl_filter,rtk_rg_aclFilterEntry_t* aclSWEntry,int shareHwAclWithSWAclIdx);

#if !defined(CONFIG_RG_G3_SERIES)	// CONFIG_RG_G3_SERIES_DEVELOPMENT
static int _rtk_rg_aclAndCfReservedRuleHeadReflash(void);
static int _rtk_rg_aclAndCfReservedRuleTailReflash(void);
#endif
int _rtk_rg_acl_user_part_rearrange(void);


/*(1)ACL init related APIs*/
int _rtk_rg_acl_asic_init(void)
{
	int i;
	rtk_filter_unmatch_action_type_t pAction;
	rtk_acl_template_t aclTemplate;
	rtk_acl_rangeCheck_ip_t iprangeEntry;
	rtk_acl_rangeCheck_l4Port_t prtRangeEntry;
#ifdef CONFIG_APOLLO_MODEL
#else
	rtk_acl_field_entry_t fieldSel;
#endif

	for(i=0; i<=RTK_RG_PORT_LASTCPU; i++)
	{
		/*set ACL_EN*/
		ASSERT_EQ(RTK_ACL_IGRSTATE_SET(i, ENABLED),RT_ERR_OK);
		/*set ACL_PERMIT*/
		pAction = FILTER_UNMATCH_PERMIT;
		ASSERT_EQ(RTK_ACL_IGRUNMATCHACTION_SET(i, pAction),RT_ERR_OK);
	}

#ifdef CONFIG_APOLLO_MODEL
#else
	/*init field selector: all FS is reserved in apolloPro*/
	//ApolloPro FS must defined by user own, no default setting as apollo/apolloFE
	for(i=0;i<8;i++)
	{
		bzero(&fieldSel,sizeof(fieldSel));
		fieldSel.index = i;
		fieldSel.format = ACL_FORMAT_RAW;
		fieldSel.offset = 0;
		ASSERT_EQ(rtk_acl_fieldSelect_set(&fieldSel),RT_ERR_OK);
	}

	//FS[0]: using for IPCP/IP6CP
	{
		bzero(&fieldSel,sizeof(fieldSel));
		fieldSel.index = 0;
		fieldSel.format = ACL_FORMAT_PPPOE;
		fieldSel.offset = 6;
		ASSERT_EQ(rtk_acl_fieldSelect_set(&fieldSel),RT_ERR_OK);
	}


	//FS[1]: using for IP fragment flag
	{
		bzero(&fieldSel,sizeof(fieldSel));
		fieldSel.index = 1;
		fieldSel.format = ACL_FORMAT_IPV4;
		fieldSel.offset = 6;
		ASSERT_EQ(rtk_acl_fieldSelect_set(&fieldSel),RT_ERR_OK);
	}


	//FS[2]: using for TCP flag
	{
		bzero(&fieldSel,sizeof(fieldSel));
		fieldSel.index = 2;
		fieldSel.format = ACL_FORMAT_IPPAYLOAD;
		fieldSel.offset = 12;
		ASSERT_EQ(rtk_acl_fieldSelect_set(&fieldSel),RT_ERR_OK);
	}


	//FS[3], using for ipv6 flowLabel[0:3]
	{
		bzero(&fieldSel,sizeof(fieldSel));
		fieldSel.index = 3;
		fieldSel.format = ACL_FORMAT_IPV6_OUTER;
		fieldSel.offset = 0;
		ASSERT_EQ(rtk_acl_fieldSelect_set(&fieldSel),RT_ERR_OK);
	}

	//FS[4], using for ipv6 flowLabel[4:19]
	{
		bzero(&fieldSel,sizeof(fieldSel));
		fieldSel.index = 4;
		fieldSel.format = ACL_FORMAT_IPV6_OUTER;
		fieldSel.offset = 2;
		ASSERT_EQ(rtk_acl_fieldSelect_set(&fieldSel),RT_ERR_OK);
	}



	/*init TEMPLATE, sync the pattern index by struct  rtk_rg_aclField_Teamplate_t*/

	//aclTemplate[0]
	bzero((void*) &aclTemplate, sizeof(aclTemplate));
	aclTemplate.index = 0;
	aclTemplate.fieldType[0] = ACL_FIELD_DMAC0;
	aclTemplate.fieldType[1] = ACL_FIELD_DMAC1;
	aclTemplate.fieldType[2] = ACL_FIELD_DMAC2;
	aclTemplate.fieldType[3] = ACL_FIELD_SMAC0;
	aclTemplate.fieldType[4] = ACL_FIELD_SMAC1;
	aclTemplate.fieldType[5] = ACL_FIELD_SMAC2;
	aclTemplate.fieldType[6] = ACL_FIELD_CTAG;
	aclTemplate.fieldType[7] = ACL_FIELD_STAG;
	ASSERT_EQ(rtk_acl_template_set(&aclTemplate),RT_ERR_OK);


	//aclTemplate[1]
	bzero((void*) &aclTemplate, sizeof(aclTemplate));
	aclTemplate.index = 1;
	aclTemplate.fieldType[0] = ACL_FIELD_IPV4_DIP0;
	aclTemplate.fieldType[1] = ACL_FIELD_IPV4_DIP1;
	aclTemplate.fieldType[2] = ACL_FIELD_IPV4_SIP0;
	aclTemplate.fieldType[3] = ACL_FIELD_IPV4_SIP1;
	aclTemplate.fieldType[4] = ACL_FIELD_IP_RANGE;
	aclTemplate.fieldType[5] = ACL_FIELD_ETHERTYPE;
	aclTemplate.fieldType[6] = ACL_FIELD_EXT_PORTMASK;
	aclTemplate.fieldType[7] = ACL_FIELD_GEMPORT;
	ASSERT_EQ(rtk_acl_template_set(&aclTemplate),RT_ERR_OK);



	//aclTemplate[2]
	bzero((void*) &aclTemplate, sizeof(aclTemplate));
	aclTemplate.index = 2;
	aclTemplate.fieldType[0] = ACL_FIELD_USER_DEFINED01; //l4 dport
	aclTemplate.fieldType[1] = ACL_FIELD_USER_DEFINED00; //l4 sport
	aclTemplate.fieldType[2] = ACL_FIELD_PORT_RANGE;
	aclTemplate.fieldType[3] = ACL_FIELD_IPV4_PROTOCOL;
	aclTemplate.fieldType[4] = ACL_FIELD_IPV6_NEXT_HEADER;
	aclTemplate.fieldType[5] = ACL_FIELD_FRAME_TYPE_TAGS; //tagIf
	aclTemplate.fieldType[6] = ACL_FIELD_VID_RANGE; //rsv
	aclTemplate.fieldType[7] = ACL_FIELD_PKT_LEN_RANGE; //rsv
	ASSERT_EQ(rtk_acl_template_set(&aclTemplate),RT_ERR_OK);


	//aclTemplate[3]:all rsv, filter userField
	bzero((void*) &aclTemplate, sizeof(aclTemplate));
	aclTemplate.index = 3;
	aclTemplate.fieldType[0] = ACL_FIELD_USER_DEFINED00;
	aclTemplate.fieldType[1] = ACL_FIELD_USER_DEFINED01;
	aclTemplate.fieldType[2] = ACL_FIELD_USER_DEFINED02;
	aclTemplate.fieldType[3] = ACL_FIELD_USER_DEFINED03;
	aclTemplate.fieldType[4] = ACL_FIELD_USER_DEFINED04;
	aclTemplate.fieldType[5] = ACL_FIELD_USER_DEFINED05;
	aclTemplate.fieldType[6] = ACL_FIELD_USER_DEFINED06;
	aclTemplate.fieldType[7] = ACL_FIELD_USER_DEFINED07;
	ASSERT_EQ(rtk_acl_template_set(&aclTemplate),RT_ERR_OK);

	//aclTemplate[4]: inner IP
	bzero((void*) &aclTemplate, sizeof(aclTemplate));
	aclTemplate.index = 4;
	aclTemplate.fieldType[0] = ACL_FIELD_IPV4_DIP0_INNER;
	aclTemplate.fieldType[1] = ACL_FIELD_IPV4_DIP1_INNER;
	aclTemplate.fieldType[2] = ACL_FIELD_IPV4_SIP0_INNER;
	aclTemplate.fieldType[3] = ACL_FIELD_IPV4_SIP1_INNER;
	aclTemplate.fieldType[4] = ACL_FIELD_IP_RANGE;
	aclTemplate.fieldType[5] = ACL_FIELD_ETHERTYPE;
	aclTemplate.fieldType[6] = ACL_FIELD_EXT_PORTMASK;
	aclTemplate.fieldType[7] = ACL_FIELD_IPV4_PROTOCOL_INNER;
	ASSERT_EQ(rtk_acl_template_set(&aclTemplate),RT_ERR_OK);

	//aclTemplate[5]:SIPv6
	bzero((void*) &aclTemplate, sizeof(aclTemplate));
	aclTemplate.index = 5;
	aclTemplate.fieldType[0] = ACL_FIELD_IPV6_SIP0;
	aclTemplate.fieldType[1] = ACL_FIELD_IPV6_SIP1;
	aclTemplate.fieldType[2] = ACL_FIELD_IPV6_SIP2;
	aclTemplate.fieldType[3] = ACL_FIELD_IPV6_SIP3;
	aclTemplate.fieldType[4] = ACL_FIELD_IPV6_SIP4;
	aclTemplate.fieldType[5] = ACL_FIELD_IPV6_SIP5;
	aclTemplate.fieldType[6] = ACL_FIELD_IPV6_SIP6;
	aclTemplate.fieldType[7] = ACL_FIELD_IPV6_SIP7;
	ASSERT_EQ(rtk_acl_template_set(&aclTemplate),RT_ERR_OK);

	//aclTemplate[6]:DIPv6
	bzero((void*) &aclTemplate, sizeof(aclTemplate));
	aclTemplate.index = 6;
	aclTemplate.fieldType[0] = ACL_FIELD_IPV6_DIP0;
	aclTemplate.fieldType[1] = ACL_FIELD_IPV6_DIP1;
	aclTemplate.fieldType[2] = ACL_FIELD_IPV6_DIP2;
	aclTemplate.fieldType[3] = ACL_FIELD_IPV6_DIP3;
	aclTemplate.fieldType[4] = ACL_FIELD_IPV6_DIP4;
	aclTemplate.fieldType[5] = ACL_FIELD_IPV6_DIP5;
	aclTemplate.fieldType[6] = ACL_FIELD_IPV6_DIP6;
	aclTemplate.fieldType[7] = ACL_FIELD_IPV6_DIP7;
	ASSERT_EQ(rtk_acl_template_set(&aclTemplate),RT_ERR_OK);

	//aclTemplate[7]
	bzero((void*) &aclTemplate, sizeof(aclTemplate));
	aclTemplate.index = 7;
	aclTemplate.fieldType[0] = ACL_FIELD_TCP_DPORT;
	aclTemplate.fieldType[1] = ACL_FIELD_TCP_SPORT;
	aclTemplate.fieldType[2] = ACL_FIELD_UDP_DPORT;
	aclTemplate.fieldType[3] = ACL_FIELD_UDP_SPORT;
	aclTemplate.fieldType[4] = ACL_FIELD_IPV4_SIP0_INNER;
	aclTemplate.fieldType[5] = ACL_FIELD_IPV4_SIP1_INNER;
	aclTemplate.fieldType[6] = ACL_FIELD_IPV4_DIP0_INNER;
	aclTemplate.fieldType[7] = ACL_FIELD_IPV4_DIP1_INNER;
	ASSERT_EQ(rtk_acl_template_set(&aclTemplate),RT_ERR_OK);

#endif

	/*init IP_RNG_TABLE*/
	bzero((void*) &iprangeEntry, sizeof(iprangeEntry));
	for(i =0; i<MAX_ACL_IPRANGETABLE_SIZE; i++)
	{
		iprangeEntry.index=i;
		ASSERT_EQ(rtk_acl_ipRange_set(&iprangeEntry),RT_ERR_OK);
	}

	/*init PORT_RNG_TABLE*/
	bzero((void*) &prtRangeEntry, sizeof(prtRangeEntry));
	for(i =0; i<MAX_ACL_PORTRANGETABLE_SIZE; i++)
	{
		prtRangeEntry.index=i;
		ASSERT_EQ(rtk_acl_portRange_set(&prtRangeEntry),RT_ERR_OK);
	}

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
	ACL_PTOOL_VERIFY_START;

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

	ACL_PTOOL_VERIFY_END;
	return RT_ERR_RG_OK;
}

static int _rtk_rg_rearrange_ACL_weight_for_l34_trap_drop_permit(int *accumulateIdx)
{
	int i,j;
	int temp,sp;
	int32 sort_start=0, sort_end=0;
	ACL_PTOOL_VERIFY_START;

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

	ACL_PTOOL_VERIFY_END;
	return RT_ERR_RG_OK;
}

static int _rtk_rg_rearrange_ACL_weight_for_l34_Qos(int *accumulateIdx)
{
	int i,j;
	int temp,sp;
	int32 sort_start=0, sort_end=0;
	ACL_PTOOL_VERIFY_START;

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

	ACL_PTOOL_VERIFY_END;
	return RT_ERR_RG_OK;
}


static int _rtk_rg_aclSWEntry_and_asic_reAdd(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx)
{
	int ret;
	ACL_PTOOL_VERIFY_START;
	
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
		if((acl_filter->action_type==ACL_ACTION_TYPE_TRAP_WITH_PRIORITY)||(acl_filter->action_type==ACL_ACTION_TYPE_TRAP2SLAVE_WITH_PRIORITY))
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
			}else if(acl_filter->action_type==ACL_ACTION_TYPE_QOS){//QoS rule both apply on HW and SW
				ACL_CTRL("RG_ACL[%d] QoS rule skip to HW, due to include H/W ACL not support pattern",*acl_filter_idx);
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
			}else if(acl_filter->action_type==ACL_ACTION_TYPE_QOS){//QoS rule both apply on HW and SW
				ACL_CTRL("RG_ACL[%d] QoS rule skip to HW, due to include not pattern",*acl_filter_idx);
				goto addHwAclEnd;
			}else{//drop rule will just by pass itself
				ACL_CTRL("RG_ACL[%d] drop rule skip to HW, due to include not pattern",*acl_filter_idx);
				goto addHwAclEnd;
			}
		}

		//need sub function to add to H/W ACL, and a clear API to call first.
		ret = _rtk_rg_aclSWEntry_to_asic_add(acl_filter,&rg_db.systemGlobal.aclSWEntry,*acl_filter_idx);
		if(ret != RT_ERR_RG_OK)
		{
			ACL_CTRL("RG_ACL[%d] add to H/W ACL fail(ret=0x%x), stop rest ACL rule with forwarding action add to H/W ACL",*acl_filter_idx, ret);
			rg_db.systemGlobal.stop_add_hw_acl=*acl_filter_idx+1;
			if((acl_filter->action_type==ACL_ACTION_TYPE_TRAP_WITH_PRIORITY)||(acl_filter->action_type==ACL_ACTION_TYPE_TRAP2SLAVE_WITH_PRIORITY))
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
	else if(rg_db.systemGlobal.stop_add_hw_acl>0 && ((acl_filter->action_type==ACL_ACTION_TYPE_TRAP_WITH_PRIORITY)||(acl_filter->action_type==ACL_ACTION_TYPE_TRAP2SLAVE_WITH_PRIORITY)))
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

	ACL_PTOOL_VERIFY_END;
    return (RT_ERR_RG_OK);
}




#if 1
static int _rtk_rg_search_acl_empty_Entry(int size, int* index)
{
    int i=0;
    int continue_size=0;
    rtk_acl_ingress_entry_t aclRule;
    bzero(&aclRule, sizeof(aclRule));

    if(size == 0) //no need acl entry
    {
        return (RT_ERR_RG_OK);
    }
    for(i=MIN_ACL_ENTRY_INDEX; i<=MAX_ACL_ENTRY_INDEX; i++)
    {
        aclRule.index = i;
        if(rtk_acl_igrRuleEntry_get(&aclRule))
        {
            ACL_CTRL("access HW_ACL[%d] failed",i);
            return (RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
        }
        if(aclRule.valid==DISABLED)
        {

            continue_size++;

            if(continue_size == size)
            {
                *index = ((i+1) - size);
                ACL_CTRL("found ACL[%d] is empty.  continue_size=%d",i,continue_size);
                return (RT_ERR_RG_OK);
            }
            continue;
        }
        else
        {
            continue_size =0;
            continue;
        }
    }

    return (RT_ERR_RG_ACL_ENTRY_FULL);
}


static int _rtk_rg_search_acl_exist_ipTableEntry(uint32* index,ipaddr_t upperIp,ipaddr_t lowerIp,rtk_acl_iprange_t type)
{

	int i;
	rtk_acl_rangeCheck_ip_t aclIpRangeEntry;
	bzero(&aclIpRangeEntry, sizeof(aclIpRangeEntry));


	for(i=0;i<MAX_ACL_IPRANGETABLE_SIZE;i++)
	{
	   aclIpRangeEntry.index = i;
	   if(rtk_acl_ipRange_get(&aclIpRangeEntry))
	   {
			ACL_CTRL("access HW ACL IP_RANGE table failed");
		   return RT_ERR_RG_ACL_IPTABLE_ACCESS_FAILED;
	   }
	   else
	   {
		   if(aclIpRangeEntry.type==type &&
		   		aclIpRangeEntry.upperIp==upperIp &&
		   		aclIpRangeEntry.lowerIp==lowerIp)
		   {
			   *index = i;
			   return (RT_ERR_RG_OK);
		   }
		   else
		   {
			   continue;
		   }
	   }
	}


	return RT_ERR_RG_ACL_IPTABLE_FULL;
}


static int _rtk_rg_search_acl_empty_ipTableEntry(uint32* index, uint32 type)
{
    int i;
    rtk_acl_rangeCheck_ip_t aclIpRangeEntry;
    bzero(&aclIpRangeEntry, sizeof(aclIpRangeEntry));

	if(type==RANGE_TABLE_SEARCH_FOR_ACL)
	{
		for(i=0;i<MAX_ACL_IPRANGETABLE_SIZE;i++)
	   {
		   aclIpRangeEntry.index = i;
		   if(rtk_acl_ipRange_get(&aclIpRangeEntry))
		   {
		   		ACL_CTRL("access HW ACL IP_RANGE table failed");
			   return RT_ERR_RG_ACL_IPTABLE_ACCESS_FAILED;
		   }
		   else
		   {
			   if(aclIpRangeEntry.type==IPRANGE_UNUSED)
			   {
				   *index = i;
				   return (RT_ERR_RG_OK);
			   }
			   else
			   {
				   continue;
			   }
		   }
	   }
	}

	ACL_CTRL("HW ACL IP_RANGE is full!");
    return RT_ERR_RG_ACL_IPTABLE_FULL;
}





static int _rtk_rg_search_acl_exist_portTableEntry(uint32* index,uint16 upper_bound,uint16 lower_bound,rtk_acl_portrange_type_t type)
{
	int i;
	rtk_acl_rangeCheck_l4Port_t aclPortRangeEntry;
    bzero(&aclPortRangeEntry, sizeof(aclPortRangeEntry));
	for(i=0;i<MAX_ACL_PORTRANGETABLE_SIZE;i++)
    {
        aclPortRangeEntry.index = i;
        if(rtk_acl_portRange_get(&aclPortRangeEntry))
        {
        	ACL_CTRL("access HW ACL PORT_RANGE table failed");
            return RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED;
        }
        else
        {
            if(aclPortRangeEntry.type==type &&
				aclPortRangeEntry.upper_bound==upper_bound &&
				aclPortRangeEntry.lower_bound==lower_bound)
            {
				 *index = i;
                return (RT_ERR_RG_OK);
            }
            else
            {
                continue;
            }
        }
    }

	//not found exist entry
    return RT_ERR_RG_ACL_PORTTABLE_FULL;

}




static int _rtk_rg_search_acl_empty_portTableEntry(uint32* index, uint32 type)
{
    int i;
    rtk_acl_rangeCheck_l4Port_t aclPortRangeEntry;
    bzero(&aclPortRangeEntry, sizeof(aclPortRangeEntry));

	//all Port_range table is used for ACL only in RTL9607C
	if(type==RANGE_TABLE_SEARCH_FOR_ACL)
	{
	    for(i=0;i<MAX_ACL_PORTRANGETABLE_SIZE;i++)
	    {
	        aclPortRangeEntry.index = i;
	        if(rtk_acl_portRange_get(&aclPortRangeEntry))
	        {
	        	ACL_CTRL("access HW ACL PORT_RANGE table failed");
	            return RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED;
	        }
	        else
	        {
	            if(aclPortRangeEntry.type==PORTRANGE_UNUSED)
	            {
					 *index = i;
	                return (RT_ERR_RG_OK);
	            }
	            else
	            {
	                continue;
	            }
	        }
	    }
	}

	ACL_CTRL("HW ACL PORT_RANGE is full!");
    return RT_ERR_RG_ACL_PORTTABLE_FULL;

}


// 0~USER_ACL_PKTLENRANGETABLE_START-1 use by reserve acl, USER_ACL_PKTLENRANGETABLE_START~MAX_ACL_PKTLENRANGETABLE_SIZE-1 used by user acl
static int _rtk_rg_search_acl_pktLenRangeTableEntry(uint16* index, uint8 is_rsv, uint16 upper_bound, uint16 lower_bound)
{
	int i = 0;
	int table_start, table_end;
	rtk_acl_rangeCheck_pktLength_t pktLenRange;
	bzero(&pktLenRange,sizeof(pktLenRange));

	if(is_rsv)
	{
		table_start = 0;
		table_end = USER_ACL_PKTLENRANGETABLE_START-1;
	}
	else
	{
		table_start = USER_ACL_PKTLENRANGETABLE_START;
		table_end = MAX_ACL_PKTLENRANGETABLE_SIZE-1;
	}

	if((upper_bound == 0) && (lower_bound == 0))	//reserve latest rule to range 0/0
		table_start = table_end;

	//reuse range table
	for(i=table_start;i<=table_end;i++)
	{
		pktLenRange.index = i;
		if(rtk_acl_packetLengthRange_get(&pktLenRange)==RT_ERR_RG_OK)
		{
			if((pktLenRange.lower_bound==lower_bound) && (pktLenRange.upper_bound==upper_bound))
			{
				*index = i;
				return (RT_ERR_RG_OK);
			}
			else
			{
				continue;
			}
		}
	}

	//add new range
	for(i=table_start;i<=table_end;i++)
	{
		pktLenRange.index = i;
		if(rtk_acl_packetLengthRange_get(&pktLenRange))
		{
			ACL_CTRL("access HW ACL PKTLEN_RANGE table failed");
			ACL_RSV("access HW ACL PKTLEN_RANGE table failed");
			return RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED;	//reuse error code
		}
		else
		{
			if((pktLenRange.lower_bound==0) && (pktLenRange.upper_bound==0))
			{
				*index = i;
				bzero(&pktLenRange,sizeof(pktLenRange));
				pktLenRange.index = i;
				pktLenRange.type = PKTLENRANGE_NOTREVISE;
				pktLenRange.upper_bound = upper_bound;
				pktLenRange.lower_bound = lower_bound;
				if(rtk_acl_packetLengthRange_set(&pktLenRange)){
					ACL_CTRL("update HW ACL PKTLEN_RANGE table[%d] failed", i);
					ACL_RSV("update HW ACL PKTLEN_RANGE table[%d] failed", i);
					return RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED;
				}else
					return (RT_ERR_RG_OK);
			}
			else
			{
				continue;
			}
		}
	}

	ACL_CTRL("HW ACL PKTLEN_RANGE is full!");
	ACL_RSV("HW ACL PKTLEN_RANGE is full!");
	return RT_ERR_RG_ACL_PORTTABLE_FULL;	//reuse error code
}



static int _rtk_rg_maskLength_get_by_ipv4_range(ipaddr_t ipv4_addr_start, ipaddr_t ipv4_addr_end, int* maskLength)
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



//9607C only support ACL pattern and drop action add to H/W.
static int _rtk_rg_aclSWEntry_to_asic_add(rtk_rg_aclFilterAndQos_t *acl_filter,rtk_rg_aclFilterEntry_t* aclSWEntry,int shareHwAclWithSWAclIdx)
{
	int i,j,index,ret;
	int aclLatchToCfIdx=FAIL;
	int aclActionRuleIdx,aclCountinueRuleSize;
	uint8 firstHitFlag;
	rtk_portmask_t mac_pmsk;
	rtk_portmask_t ext_pmsk;
	int portmask_need_double_hw_rule=0;
	int portmask_need_double_hw_rule_add_sencond=0;
	int ipv4_unmask_length=0;
	ACL_PTOOL_VERIFY_START;

	aclSWEntry->hw_aclEntry_start = 0;
	aclSWEntry->hw_aclEntry_size = 0;

    if(acl_filter->filter_fields == 0x0) //no patterns need to filter
        RETURN_ERR(RT_ERR_RG_INVALID_PARAM);

	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)//pure ACK type can not use CF patterns and actions
	{
		//check pattern that hw ACL can support
		if(acl_filter->action_type==ACL_ACTION_TYPE_TRAP_WITH_PRIORITY) {
			if(acl_filter->filter_fields & (~HW_ACL_SUPPORT_PATTERN_9607C)) {
				ACL_CTRL("rule contained HW ACL not support pattern, skip add to hw.");
				return (RT_ERR_RG_INVALID_PARAM);
			}
		}
		else if(acl_filter->filter_fields & (~HW_ACL_SUPPORT_PATTERN)){
			ACL_CTRL("rule contained HW ACL not support pattern, skip add to hw.");
			return (RT_ERR_RG_INVALID_PARAM);
		}

		//check the action only for drop
	}

	if(shareHwAclWithSWAclIdx >= 0)	//for debug acl trap by which rg acl index
		shareHwAclWithSWAclIdx = RTK_RG_ACLANDCF_RESERVED_TAIL_END + shareHwAclWithSWAclIdx;
	else
		shareHwAclWithSWAclIdx = FAIL;//pre-add test

	ASSERT_EQ(_rtk_rg_portmask_translator(acl_filter->ingress_port_mask, &mac_pmsk, &ext_pmsk),RT_ERR_RG_OK);


	//clean portmask_need_double_hw_rule record, if TRANSFROM need to add more then two set of ACLs.
	portmask_need_double_hw_rule=0;
	portmask_need_double_hw_rule_add_sencond=0;

	if(mac_pmsk.bits[0]!=0x0 && ext_pmsk.bits[0]!=0x0) //portmask include mac_ports and ext_ports, need double ACLs due to EXTPORT pattern
	{
		ACL_CTRL("rule include mac_port=0x%x and extport=0x%x",mac_pmsk.bits[0],ext_pmsk.bits[0]);
		portmask_need_double_hw_rule=1;
	}


PORTMASK_NEED_DOUBLE_RULE:
	aclCountinueRuleSize=0;

	//init global parameters
	bzero(rg_db.systemGlobal.aclRuleValid,sizeof(uint8)*GLOBAL_ACL_RULE_SIZE);
	bzero(&rg_db.systemGlobal.aclFieldEmpty,sizeof(rtk_acl_field_t));
	bzero(rg_db.systemGlobal.aclField,sizeof(rtk_acl_field_t)*GLOBAL_ACL_FIELD_SIZE);
	bzero(rg_db.systemGlobal.aclRule,sizeof(rtk_acl_ingress_entry_t)*GLOBAL_ACL_RULE_SIZE);
	bzero(&rg_db.systemGlobal.aclRuleEmpty,sizeof(rtk_acl_ingress_entry_t));


	/*set patterns*/
    if((acl_filter->filter_fields & INGRESS_INTF_BIT) && acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
    {
		WARNING("INGRESS_INTF_BIT is not support by HW_ACL!");
		return(RT_ERR_RG_INVALID_PARAM);
#if 0
        //use intf idex to get  Datastruct(for check is_wan or is_lan)
        ingress_intf_idx = acl_filter->ingress_intf_idx;
		bzero(&ingress_intf_info,sizeof(rtk_rg_intfInfo_t));

        ASSERT_EQ(rtk_rg_apollo_intfInfo_find(&ingress_intf_info, &ingress_intf_idx),RT_ERR_RG_OK);

        if(ingress_intf_info.is_wan)
        {
            //Wan Interface
            if(ingress_intf_info.wan_intf.wan_intf_conf.wan_type == RTK_RG_BRIDGE)//L2 wan
            {

                if(ingress_intf_info.wan_intf.wan_intf_conf.egress_vlan_tag_on)
                {
                    //this bridge wan have ctag => pkt must have ctag & compare CVID
                    for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
                    {
                        rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].value = 1;
                        rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].mask = 0xffff;
                    }
                    //setup Ctag
                    rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldType = ACL_FIELD_PATTERN_MATCH;
                    rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldUnion.pattern.fieldIdx = TEMPLATE_CTAG;
                    rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldUnion.data.value = ingress_intf_info.wan_intf.wan_intf_conf.egress_vlan_id;
                    rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldUnion.data.mask = 0x0fff; //do not compare with CPRI[15:3] & CFI[12]

                }
                else
                {
                    //this bridge wan do not have ctag =>  pkt must must not have ctag
                    for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
                    {
                        rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].value = 0;
                        rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].mask = 0xffff;
                    }
                }


            }
            else //L34 Wan
            {

                if(ingress_intf_info.wan_intf.wan_intf_conf.egress_vlan_tag_on)
                {
                    //this route wan have ctag
                    for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
                    {
                        rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].value = 1;
                        rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].mask = 0xffff;
                    }

                    //setup Ctag
                    rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldType = ACL_FIELD_PATTERN_MATCH;
                    rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldUnion.pattern.fieldIdx = TEMPLATE_CTAG;
                    rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldUnion.data.value = ingress_intf_info.wan_intf.wan_intf_conf.egress_vlan_id;
                    rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldUnion.data.mask = 0x0fff; //do not compare with CPRI[15:3] & CFI[12]


                    //setup gmac
                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC0].fieldType = ACL_FIELD_PATTERN_MATCH;
                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC0].fieldUnion.pattern.fieldIdx = TEMPLATE_DMAC0;
                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC0].fieldUnion.data.value = (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[4]<<8) | (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[5]);
                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC0].fieldUnion.data.mask = 0xffff;

                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC1].fieldType = ACL_FIELD_PATTERN_MATCH;
                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC1].fieldUnion.pattern.fieldIdx = TEMPLATE_DMAC1;
                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC1].fieldUnion.data.value = (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[2]<<8) | (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[3]);
                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC1].fieldUnion.data.mask = 0xffff;

                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC2].fieldType = ACL_FIELD_PATTERN_MATCH;
                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC2].fieldUnion.pattern.fieldIdx = TEMPLATE_DMAC2;
                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC2].fieldUnion.data.value = (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[0]<<8) | (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[1]);
                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC2].fieldUnion.data.mask = 0xffff;

                }
                else
                {
                    //this route wan do not have ctag
                    for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
                    {
                        rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].value = 0;
                        rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].mask = 0xffff;
                    }

                    //setup gmac
                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC0].fieldType = ACL_FIELD_PATTERN_MATCH;
                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC0].fieldUnion.pattern.fieldIdx = TEMPLATE_DMAC0;
                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC0].fieldUnion.data.value = (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[4]<<8) | (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[5]);
                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC0].fieldUnion.data.mask = 0xffff;

                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC1].fieldType = ACL_FIELD_PATTERN_MATCH;
                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC1].fieldUnion.pattern.fieldIdx = TEMPLATE_DMAC1;
                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC1].fieldUnion.data.value = (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[2]<<8) | (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[3]);
                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC1].fieldUnion.data.mask = 0xffff;

                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC2].fieldType = ACL_FIELD_PATTERN_MATCH;
                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC2].fieldUnion.pattern.fieldIdx = TEMPLATE_DMAC2;
                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC2].fieldUnion.data.value = (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[0]<<8) | (ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[1]);
                    rg_db.systemGlobal.aclField[TEMPLATE_DMAC2].fieldUnion.data.mask = 0xffff;
                }

            }

        }
        else
        {
            //lan Interface  => lan intf by judge active port
            for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
            {

            	for(j=0;j<RTK_RG_ALL_MAC_PORTMASK;j++){
					if((ingress_intf_info.lan_intf.port_mask.portmask&(1<<j)))
					{
						rg_db.systemGlobal.aclRule[i].activePorts.bits[0]|=(1<<j);
					}
				}
            }
        }
#endif
    }



	if(acl_filter->filter_fields & INGRESS_PORT_BIT)
	{

#if 1
		if(portmask_need_double_hw_rule) //acl_filter->ingress_port_mask include mac_port + ext_port
		{

			if(portmask_need_double_hw_rule_add_sencond==1)
			{
				//add mac port rules
				for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
				{
					rg_db.systemGlobal.aclRule[i].activePorts = mac_pmsk;
				}
				ACL_CTRL("= set acl rule for mac_port=0x%x =",mac_pmsk.bits[0]);
			}
			else
			{
				if(ext_pmsk.bits[0]){
					rg_db.systemGlobal.aclField[TEMPLATE_EXTPORTMASK].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField[TEMPLATE_EXTPORTMASK].fieldUnion.pattern.fieldIdx = (TEMPLATE_EXTPORTMASK%ACL_PER_RULE_FIELD_SIZE);;
					rg_db.systemGlobal.aclField[TEMPLATE_EXTPORTMASK].fieldUnion.data.value = 0x0;
					for(i=0;i<RTK_RG_MAX_EXT_PORT;i++){
						if(!(ext_pmsk.bits[0]&(1<<i))){
							rg_db.systemGlobal.aclField[TEMPLATE_EXTPORTMASK].fieldUnion.data.mask |= (1<<i); //band not allowed ext_port
						}
					}


					//add CPU port
					if(ext_pmsk.bits[0]& RTK_RG_ALL_VLAN_MASTER_EXT_PORTMASK /*master CPU Ext port*/)
					{
						for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
						{
							rg_db.systemGlobal.aclRule[i].activePorts.bits[0] |= RTK_RG_ALL_MAC_MASTER_CPU_PORTMASK/*core0 + core1 mask*/;
						}

					}
					if(ext_pmsk.bits[0]& RTK_RG_ALL_VLAN_SLAVE_EXT_PORTMASK /*slave CPU Ext port*/)
					{
						for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
						{
							rg_db.systemGlobal.aclRule[i].activePorts.bits[0] |= RTK_RG_ALL_MAC_SLAVE_CPU_PORTMASK /*slave(mac7) mask*/;
						}
					}
				}
				ACL_CTRL("= set acl rule for ext_port=0x%x (pattern_mask=0x%x) =",ext_pmsk.bits[0],rg_db.systemGlobal.aclField[TEMPLATE_EXTPORTMASK].fieldUnion.data.mask);

			}

		}
		else//acl_filter->ingress_port_mask include mac_port only or ext_port only
		{
			//add active port in all related(0~3) rg_db.systemGlobal.aclRule
			for(i=0; i<GLOBAL_ACL_RULE_SIZE; i++)
			{
				rg_db.systemGlobal.aclRule[i].activePorts = mac_pmsk;
			}
			//if there are any ext_port add ext_port pattern
			if(ext_pmsk.bits[0]){
				rg_db.systemGlobal.aclField[TEMPLATE_EXTPORTMASK].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField[TEMPLATE_EXTPORTMASK].fieldUnion.pattern.fieldIdx = (TEMPLATE_EXTPORTMASK%ACL_PER_RULE_FIELD_SIZE);
				rg_db.systemGlobal.aclField[TEMPLATE_EXTPORTMASK].fieldUnion.data.value = 0x0;
				for(i=0;i<RTK_RG_MAX_EXT_PORT;i++){
					if(!(ext_pmsk.bits[0]&(1<<i))){
						rg_db.systemGlobal.aclField[TEMPLATE_EXTPORTMASK].fieldUnion.data.mask |= (1<<i); //band not allowed ext_port
					}
				}
			}
		}
#else
	  	//add active port in all related(0~3) rg_db.systemGlobal.aclRule
        for(i=0; i<GLOBAL_ACL_RULE_SIZE; i++)
        {
            rg_db.systemGlobal.aclRule[i].activePorts = mac_pmsk;
        }
		//if there are any ext_port add ext_port pattern
		if(ext_pmsk.bits[0]){
			rg_db.systemGlobal.aclField[TEMPLATE_EXTPORTMASK].fieldType = ACL_FIELD_PATTERN_MATCH;
			rg_db.systemGlobal.aclField[TEMPLATE_EXTPORTMASK].fieldUnion.pattern.fieldIdx = (TEMPLATE_EXTPORTMASK%ACL_PER_RULE_FIELD_SIZE);
			rg_db.systemGlobal.aclField[TEMPLATE_EXTPORTMASK].fieldUnion.data.value = 0x0;
			for(i=0;i<RTK_RG_MAX_EXT_PORT;i++){
				if(!(ext_pmsk.bits[0]&(1<<i))){
					rg_db.systemGlobal.aclField[TEMPLATE_EXTPORTMASK].fieldUnion.data.mask |= (1<<i); //band not allowed ext_port
				}
			}
		}
#endif

	}
	else if(shareHwAclWithSWAclIdx == FAIL)
	{
		if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
			WARNING("ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET should assign ingress_port_mask!");
	}

	if(acl_filter->filter_fields & INGRESS_ETHERTYPE_BIT)
	{
        rg_db.systemGlobal.aclField[TEMPLATE_ETHERTYPE].fieldType = ACL_FIELD_PATTERN_MATCH;
	    rg_db.systemGlobal.aclField[TEMPLATE_ETHERTYPE].fieldUnion.pattern.fieldIdx = (TEMPLATE_ETHERTYPE%ACL_PER_RULE_FIELD_SIZE);
	    rg_db.systemGlobal.aclField[TEMPLATE_ETHERTYPE].fieldUnion.data.value = acl_filter->ingress_ethertype;
		rg_db.systemGlobal.aclField[TEMPLATE_ETHERTYPE].fieldUnion.data.mask = acl_filter->ingress_ethertype_mask;

		//for compitible of non-mask version (mask not set means all care)
		if(acl_filter->ingress_ethertype_mask==0x0){
	    	rg_db.systemGlobal.aclField[TEMPLATE_ETHERTYPE].fieldUnion.data.mask = 0xffff;
			acl_filter->ingress_ethertype_mask = 0xffff;
		}
	}

	if(acl_filter->filter_fields & INGRESS_CTAG_PRI_BIT)
	{
		rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldUnion.pattern.fieldIdx = (TEMPLATE_CTAG%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldUnion.data.value |= ((acl_filter->ingress_ctag_pri)<<13);
		rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldUnion.data.mask |= 0xe000;

	}

	if(acl_filter->filter_fields & INGRESS_CTAG_VID_BIT)
	{
		rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldUnion.pattern.fieldIdx = (TEMPLATE_CTAG%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldUnion.data.value |= (acl_filter->ingress_ctag_vid);
		rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldUnion.data.mask |= 0x0fff;
	}

	if(acl_filter->filter_fields & INGRESS_CTAG_CFI_BIT )
	{
		rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldUnion.pattern.fieldIdx = (TEMPLATE_CTAG%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldUnion.data.value |= ((acl_filter->ingress_ctag_cfi)<<12);
		rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldUnion.data.mask |= 0x1000;
	}

	if(acl_filter->filter_fields & INGRESS_SMAC_BIT)
	{
		rg_db.systemGlobal.aclField[TEMPLATE_SMAC0].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_SMAC0].fieldUnion.pattern.fieldIdx = (TEMPLATE_SMAC0%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_SMAC0].fieldUnion.data.value = ((acl_filter->ingress_smac.octet[4]<<8) | (acl_filter->ingress_smac.octet[5]));
		rg_db.systemGlobal.aclField[TEMPLATE_SMAC0].fieldUnion.data.mask = ((acl_filter->ingress_smac_mask.octet[4]<<8) | (acl_filter->ingress_smac_mask.octet[5]));

		rg_db.systemGlobal.aclField[TEMPLATE_SMAC1].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_SMAC1].fieldUnion.pattern.fieldIdx = (TEMPLATE_SMAC1%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_SMAC1].fieldUnion.data.value =((acl_filter->ingress_smac.octet[2]<<8) | (acl_filter->ingress_smac.octet[3]));
		rg_db.systemGlobal.aclField[TEMPLATE_SMAC1].fieldUnion.data.mask = ((acl_filter->ingress_smac_mask.octet[2]<<8) | (acl_filter->ingress_smac_mask.octet[3]));

		rg_db.systemGlobal.aclField[TEMPLATE_SMAC2].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_SMAC2].fieldUnion.pattern.fieldIdx = (TEMPLATE_SMAC2%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_SMAC2].fieldUnion.data.value =((acl_filter->ingress_smac.octet[0]<<8) | (acl_filter->ingress_smac.octet[1]));
		rg_db.systemGlobal.aclField[TEMPLATE_SMAC2].fieldUnion.data.mask = ((acl_filter->ingress_smac_mask.octet[0]<<8) | (acl_filter->ingress_smac_mask.octet[1]));

		//for compitible of non-mask version (mask not set means all care)
		if((acl_filter->ingress_smac_mask.octet[0] |
			acl_filter->ingress_smac_mask.octet[1] |
			acl_filter->ingress_smac_mask.octet[2] |
			acl_filter->ingress_smac_mask.octet[3] |
			acl_filter->ingress_smac_mask.octet[4] |
			acl_filter->ingress_smac_mask.octet[5]) ==0x0 ){
			rg_db.systemGlobal.aclField[TEMPLATE_SMAC0].fieldUnion.data.mask = 0xffff;
			rg_db.systemGlobal.aclField[TEMPLATE_SMAC1].fieldUnion.data.mask = 0xffff;
			rg_db.systemGlobal.aclField[TEMPLATE_SMAC2].fieldUnion.data.mask = 0xffff;

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
		rg_db.systemGlobal.aclField[TEMPLATE_DMAC0].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_DMAC0].fieldUnion.pattern.fieldIdx = (TEMPLATE_DMAC0%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_DMAC0].fieldUnion.data.value =((acl_filter->ingress_dmac.octet[4]<<8) | (acl_filter->ingress_dmac.octet[5]));
		rg_db.systemGlobal.aclField[TEMPLATE_DMAC0].fieldUnion.data.mask = ((acl_filter->ingress_dmac_mask.octet[4]<<8) | (acl_filter->ingress_dmac_mask.octet[5]));

		rg_db.systemGlobal.aclField[TEMPLATE_DMAC1].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_DMAC1].fieldUnion.pattern.fieldIdx = (TEMPLATE_DMAC1%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_DMAC1].fieldUnion.data.value =((acl_filter->ingress_dmac.octet[2]<<8) | (acl_filter->ingress_dmac.octet[3]));
		rg_db.systemGlobal.aclField[TEMPLATE_DMAC1].fieldUnion.data.mask = ((acl_filter->ingress_dmac_mask.octet[2]<<8) | (acl_filter->ingress_dmac_mask.octet[3]));

		rg_db.systemGlobal.aclField[TEMPLATE_DMAC2].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_DMAC2].fieldUnion.pattern.fieldIdx = (TEMPLATE_DMAC2%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_DMAC2].fieldUnion.data.value =((acl_filter->ingress_dmac.octet[0]<<8) | (acl_filter->ingress_dmac.octet[1]));
		rg_db.systemGlobal.aclField[TEMPLATE_DMAC2].fieldUnion.data.mask =((acl_filter->ingress_dmac_mask.octet[0]<<8) | (acl_filter->ingress_dmac_mask.octet[1]));

		//for compitible of non-mask version (mask not set means all care)
		if((acl_filter->ingress_dmac_mask.octet[0]|
			acl_filter->ingress_dmac_mask.octet[1]|
			acl_filter->ingress_dmac_mask.octet[2]|
			acl_filter->ingress_dmac_mask.octet[3]|
			acl_filter->ingress_dmac_mask.octet[4]|
			acl_filter->ingress_dmac_mask.octet[5])==0x0 ){
			rg_db.systemGlobal.aclField[TEMPLATE_DMAC0].fieldUnion.data.mask = 0xffff;
			rg_db.systemGlobal.aclField[TEMPLATE_DMAC1].fieldUnion.data.mask = 0xffff;
			rg_db.systemGlobal.aclField[TEMPLATE_DMAC2].fieldUnion.data.mask = 0xffff;

			acl_filter->ingress_dmac_mask.octet[0]=0xff;
			acl_filter->ingress_dmac_mask.octet[1]=0xff;
			acl_filter->ingress_dmac_mask.octet[2]=0xff;
			acl_filter->ingress_dmac_mask.octet[3]=0xff;
			acl_filter->ingress_dmac_mask.octet[4]=0xff;
			acl_filter->ingress_dmac_mask.octet[5]=0xff;
		}
	}

	if(acl_filter->filter_fields & INGRESS_DSCP_BIT)
	{
		rg_db.systemGlobal.aclField[TEMPLATE_IP4_TOS_PROTO].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_IP4_TOS_PROTO].fieldUnion.pattern.fieldIdx = (TEMPLATE_IP4_TOS_PROTO%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_IP4_TOS_PROTO].fieldUnion.data.value &=0xff; //clear formet pattern
		rg_db.systemGlobal.aclField[TEMPLATE_IP4_TOS_PROTO].fieldUnion.data.value |= (acl_filter->ingress_dscp)<<(2+8); //ToS[15:8] + protocal[7:0]
		rg_db.systemGlobal.aclField[TEMPLATE_IP4_TOS_PROTO].fieldUnion.data.mask |= 0xfc00;
	}

	if(acl_filter->filter_fields & INGRESS_TOS_BIT)
	{
		rg_db.systemGlobal.aclField[TEMPLATE_IP4_TOS_PROTO].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_IP4_TOS_PROTO].fieldUnion.pattern.fieldIdx = (TEMPLATE_IP4_TOS_PROTO%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_IP4_TOS_PROTO].fieldUnion.data.value &=0xff; //clear formet pattern
		rg_db.systemGlobal.aclField[TEMPLATE_IP4_TOS_PROTO].fieldUnion.data.value |= acl_filter->ingress_tos<<8;//ToS[15:8] + protocal[7:0]
		rg_db.systemGlobal.aclField[TEMPLATE_IP4_TOS_PROTO].fieldUnion.data.mask |= 0xff00;
	}

	if(acl_filter->filter_fields & INGRESS_L4_ICMP_BIT)
	{
		rg_db.systemGlobal.aclField[TEMPLATE_IP4_TOS_PROTO].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_IP4_TOS_PROTO].fieldUnion.pattern.fieldIdx = (TEMPLATE_IP4_TOS_PROTO%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_IP4_TOS_PROTO].fieldUnion.data.value &=0xff00; //clear formet pattern
		rg_db.systemGlobal.aclField[TEMPLATE_IP4_TOS_PROTO].fieldUnion.data.value |=0x01; //ICMP protocal value
		rg_db.systemGlobal.aclField[TEMPLATE_IP4_TOS_PROTO].fieldUnion.data.mask |=0xff; //ToS[15:8] + protocal[7:0]
	}

	if(acl_filter->filter_fields & INGRESS_L4_TCP_BIT)
	{
		rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.pattern.fieldIdx = (TEMPLATE_FRAME_TYPE_TAGS%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.value |= (1<<10); //TCP bit
		rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.mask |=(1<<10); //TCP bit
	}

	if(acl_filter->filter_fields & INGRESS_L4_UDP_BIT)
	{
		rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.pattern.fieldIdx = (TEMPLATE_FRAME_TYPE_TAGS%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.value |= (1<<9); //TCP bit
		rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.mask |=(1<<9); //TCP bit
 	}

	if(acl_filter->filter_fields & INGRESS_L4_NONE_TCP_NONE_UDP_BIT)
	{
		rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.pattern.fieldIdx = (TEMPLATE_FRAME_TYPE_TAGS%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.value &= ~((1<<9)|(1<<10)); //UDP & TCP bit
		rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.mask |=((1<<9)|(1<<10)); //UDP & TCP bit
	}



	if(acl_filter->filter_fields & INGRESS_IPV6_DSCP_BIT)
	{
        rg_db.systemGlobal.aclField[TEMPLATE_IP6_TC_NH].fieldType = ACL_FIELD_PATTERN_MATCH;
        rg_db.systemGlobal.aclField[TEMPLATE_IP6_TC_NH].fieldUnion.pattern.fieldIdx = (TEMPLATE_IP6_TC_NH%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_IP6_TC_NH].fieldUnion.data.value &=0x00ff;//clear former pattern
        rg_db.systemGlobal.aclField[TEMPLATE_IP6_TC_NH].fieldUnion.data.value |= (acl_filter->ingress_ipv6_dscp)<<(2+8); //TC[15:8] + NH[7:0]
        rg_db.systemGlobal.aclField[TEMPLATE_IP6_TC_NH].fieldUnion.data.mask |= 0xfc00;
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_TC_BIT)
	{
		rg_db.systemGlobal.aclField[TEMPLATE_IP6_TC_NH].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_IP6_TC_NH].fieldUnion.pattern.fieldIdx = (TEMPLATE_IP6_TC_NH%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_IP6_TC_NH].fieldUnion.data.value &=0x00ff;//clear former pattern
		rg_db.systemGlobal.aclField[TEMPLATE_IP6_TC_NH].fieldUnion.data.value |= (acl_filter->ingress_ipv6_tc)<<8; //TC[15:8] + NH[7:0]
		rg_db.systemGlobal.aclField[TEMPLATE_IP6_TC_NH].fieldUnion.data.mask |= 0xff00;
	}

	if(acl_filter->filter_fields & INGRESS_L4_ICMPV6_BIT )
	{
		rg_db.systemGlobal.aclField[TEMPLATE_IP6_TC_NH].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_IP6_TC_NH].fieldUnion.pattern.fieldIdx = (TEMPLATE_IP6_TC_NH%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_IP6_TC_NH].fieldUnion.data.value &=0xff00;//clear former pattern
		rg_db.systemGlobal.aclField[TEMPLATE_IP6_TC_NH].fieldUnion.data.value |=0x3a; //ICMPv6 nextheader value
		rg_db.systemGlobal.aclField[TEMPLATE_IP6_TC_NH].fieldUnion.data.mask |=0xff;//mask 8 bit only
	}

	if(acl_filter->filter_fields & INGRESS_STREAM_ID_BIT)
	{
        rg_db.systemGlobal.aclField[TEMPLATE_GEMPORT].fieldType = ACL_FIELD_PATTERN_MATCH;
        rg_db.systemGlobal.aclField[TEMPLATE_GEMPORT].fieldUnion.pattern.fieldIdx = (TEMPLATE_GEMPORT%ACL_PER_RULE_FIELD_SIZE);
        rg_db.systemGlobal.aclField[TEMPLATE_GEMPORT].fieldUnion.data.value = acl_filter->ingress_stream_id;
		rg_db.systemGlobal.aclField[TEMPLATE_GEMPORT].fieldUnion.data.mask = acl_filter->ingress_stream_id_mask;
		//for compitible of non-mask version (mask not set means all care)
		if(acl_filter->ingress_stream_id_mask==0x0){
        	rg_db.systemGlobal.aclField[TEMPLATE_GEMPORT].fieldUnion.data.mask = 0xffff;
			acl_filter->ingress_stream_id_mask = 0xffff;
		}
	}


	if(acl_filter->filter_fields & INGRESS_STAG_PRI_BIT)
	{
		rg_db.systemGlobal.aclField[TEMPLATE_STAG].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_STAG].fieldUnion.pattern.fieldIdx = (TEMPLATE_STAG%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_STAG].fieldUnion.data.value |= ((acl_filter->ingress_stag_pri)<<13);
		rg_db.systemGlobal.aclField[TEMPLATE_STAG].fieldUnion.data.mask |= 0xe000;
	}

	if(acl_filter->filter_fields & INGRESS_STAG_VID_BIT)
	{
		rg_db.systemGlobal.aclField[TEMPLATE_STAG].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_STAG].fieldUnion.pattern.fieldIdx = (TEMPLATE_STAG%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_STAG].fieldUnion.data.value |= (acl_filter->ingress_stag_vid);
		rg_db.systemGlobal.aclField[TEMPLATE_STAG].fieldUnion.data.mask |= 0x0fff;
	}

	if(acl_filter->filter_fields & INGRESS_STAG_DEI_BIT )
	{
		rg_db.systemGlobal.aclField[TEMPLATE_STAG].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_STAG].fieldUnion.pattern.fieldIdx = (TEMPLATE_STAG%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_STAG].fieldUnion.data.value |= ((acl_filter->ingress_stag_dei)<<12);
		rg_db.systemGlobal.aclField[TEMPLATE_STAG].fieldUnion.data.mask |= 0x1000;
	}

	if(acl_filter->filter_fields & INGRESS_STAGIF_BIT)
	{
		if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)//pure ACL
		{
			for(i=0; i<GLOBAL_ACL_RULE_SIZE; i++)
			{
				if(acl_filter->ingress_stagIf)
					rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_STAG].value=1;
				else
					rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_STAG].value=0;

				rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_STAG].mask=0xffff;
			}
		}
	}

	if(acl_filter->filter_fields & INGRESS_CTAGIF_BIT)
	{
		if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)//pure ACL
		{
			for(i=0; i<GLOBAL_ACL_RULE_SIZE; i++)
			{
				if(acl_filter->ingress_ctagIf)
					rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].value=1;
				else
					rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].value=0;

				rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].mask=0xffff;
			}
		}
	}

	if(acl_filter->filter_fields & INGRESS_L4_POROTCAL_VALUE_BIT)
	{
		if((acl_filter->filter_fields & INGRESS_IPV4_TAGIF_BIT)&&acl_filter->ingress_ipv4_tagif==1)
		{
			rg_db.systemGlobal.aclField[TEMPLATE_IP4_TOS_PROTO].fieldType = ACL_FIELD_PATTERN_MATCH;
			rg_db.systemGlobal.aclField[TEMPLATE_IP4_TOS_PROTO].fieldUnion.pattern.fieldIdx = (TEMPLATE_IP4_TOS_PROTO%ACL_PER_RULE_FIELD_SIZE);
			rg_db.systemGlobal.aclField[TEMPLATE_IP4_TOS_PROTO].fieldUnion.data.value |=acl_filter->ingress_l4_protocal;
			rg_db.systemGlobal.aclField[TEMPLATE_IP4_TOS_PROTO].fieldUnion.data.mask |=0xff; //ToS[15:8] + protocal[7:0]
		}
		else if((acl_filter->filter_fields & INGRESS_IPV6_TAGIF_BIT)&&acl_filter->ingress_ipv6_tagif==1)
		{
			rg_db.systemGlobal.aclField[TEMPLATE_IP6_TC_NH].fieldType = ACL_FIELD_PATTERN_MATCH;
			rg_db.systemGlobal.aclField[TEMPLATE_IP6_TC_NH].fieldUnion.pattern.fieldIdx = (TEMPLATE_IP6_TC_NH%ACL_PER_RULE_FIELD_SIZE);
			rg_db.systemGlobal.aclField[TEMPLATE_IP6_TC_NH].fieldUnion.data.value |= (acl_filter->ingress_l4_protocal); //TC[15:8] + NH[7:0]
			rg_db.systemGlobal.aclField[TEMPLATE_IP6_TC_NH].fieldUnion.data.mask |= 0xff;
		}
		else
		{
			WARNING("INGRESS_L4_POROTCAL_VALUE_BIT must set with INGRESS_IPV4_TAGIF_BIT or INGRESS_IPV6_TAGIF_BIT for H/W ACL");
			RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
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

		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP0].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv6SIP0%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP0].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[14]<<8)|(acl_filter->ingress_src_ipv6_addr[15]);//v6SIP[15:0]
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP0].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[14]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[15]);//v6SIP[15:0];

		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP1].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv6SIP1%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP1].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[12]<<8)|(acl_filter->ingress_src_ipv6_addr[13]);//v6SIP[31:16]
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP1].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[12]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[13]);

		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP2].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP2].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv6SIP2%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP2].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[10]<<8)|(acl_filter->ingress_src_ipv6_addr[11]);//v6SIP[47:32]
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP2].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[10]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[11]);;

		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP3].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP3].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv6SIP3%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP3].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[8]<<8)|(acl_filter->ingress_src_ipv6_addr[9]);//v6SIP[63:48]
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP3].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[8]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[9]);;

		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP4].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP4].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv6SIP4%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP4].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[6]<<8)|(acl_filter->ingress_src_ipv6_addr[7]);//v6SIP[79:64]
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP4].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[6]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[7]);;

		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP5].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP5].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv6SIP5%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP5].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[4]<<8)|(acl_filter->ingress_src_ipv6_addr[5]);//v6SIP[95:80]
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP5].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[4]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[5]);;

		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP6].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP6].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv6SIP6%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP6].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[2]<<8)|(acl_filter->ingress_src_ipv6_addr[3]);//v6SIP[111:96]
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP6].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[2]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[3]);;

		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP7].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP7].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv6SIP7%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP7].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[0]<<8)|(acl_filter->ingress_src_ipv6_addr[1]);//v6SIP[127:112]
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6SIP7].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[0]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[1]);

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

        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP0].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv6DIP0%ACL_PER_RULE_FIELD_SIZE);
        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP0].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[14]<<8)|(acl_filter->ingress_dest_ipv6_addr[15]);//v6DIP[15:0]
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP0].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[14]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[15]);

        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP1].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv6DIP1%ACL_PER_RULE_FIELD_SIZE);
        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP1].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[12]<<8)|(acl_filter->ingress_dest_ipv6_addr[13]);//v6DIP[31:16]
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP1].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[12]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[13]);

        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP2].fieldType = ACL_FIELD_PATTERN_MATCH;
        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP2].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv6DIP2%ACL_PER_RULE_FIELD_SIZE);
        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP2].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[10]<<8)|(acl_filter->ingress_dest_ipv6_addr[11]);//v6DIP[47:32]
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP2].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[10]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[11]);

        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP3].fieldType = ACL_FIELD_PATTERN_MATCH;
        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP3].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv6DIP3%ACL_PER_RULE_FIELD_SIZE);
        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP3].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[8]<<8)|(acl_filter->ingress_dest_ipv6_addr[9]);//v6DIP[63:48]
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP3].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[8]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[9]);

        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP4].fieldType = ACL_FIELD_PATTERN_MATCH;
        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP4].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv6DIP4%ACL_PER_RULE_FIELD_SIZE);
        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP4].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[6]<<8)|(acl_filter->ingress_dest_ipv6_addr[7]);//v6DIP[79:64]
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP4].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[6]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[7]);

        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP5].fieldType = ACL_FIELD_PATTERN_MATCH;
        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP5].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv6DIP5%ACL_PER_RULE_FIELD_SIZE);
        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP5].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[4]<<8)|(acl_filter->ingress_dest_ipv6_addr[5]);//v6DIP[95:80]
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP5].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[4]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[5]);

        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP6].fieldType = ACL_FIELD_PATTERN_MATCH;
        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP6].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv6DIP6%ACL_PER_RULE_FIELD_SIZE);
        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP6].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[2]<<8)|(acl_filter->ingress_dest_ipv6_addr[3]);//v6DIP[111:96]
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP6].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[2]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[3]);

        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP7].fieldType = ACL_FIELD_PATTERN_MATCH;
        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP7].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv6DIP7%ACL_PER_RULE_FIELD_SIZE);
        rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP7].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[0]<<8)|(acl_filter->ingress_dest_ipv6_addr[1]);//v6DIP[127:112]
		rg_db.systemGlobal.aclField[TEMPLATE_IPv6DIP7].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[0]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[1]);

	}

	if(acl_filter->filter_fields & INGRESS_IPV4_TAGIF_BIT )
	{
		rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.pattern.fieldIdx = (TEMPLATE_FRAME_TYPE_TAGS%ACL_PER_RULE_FIELD_SIZE);
		if(acl_filter->ingress_ipv4_tagif==0)//must not IPv4
			rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.value &= ~(1<<5); //out IP4 bit
		else//must be IPv4
			rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.value |= (1<<5); //out IP4 bit

		rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.mask |=(1<<5); //out IP4 bit
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_TAGIF_BIT)
	{
		rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.pattern.fieldIdx = (TEMPLATE_FRAME_TYPE_TAGS%ACL_PER_RULE_FIELD_SIZE);
		if(acl_filter->ingress_ipv6_tagif==0)//must not IPv6
			rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.value &= ~(1<<6); //ipv6 bit
		else//must be IPv6
			rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.value |= (1<<6); //ipv6 bit

		rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.mask |=(1<<6); //ipv6 bit
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_SIP_RANGE_BIT)
	{
		WARNING("INGRESS_IPV6_SIP_RANGE_BIT is not support yet!");
		return(RT_ERR_RG_INVALID_PARAM);
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_DIP_RANGE_BIT)
	{
		WARNING("INGRESS_IPV6_DIP_RANGE_BIT is not support yet!");
		return(RT_ERR_RG_INVALID_PARAM);
	}


	if(acl_filter->filter_fields & INGRESS_L4_SPORT_RANGE_BIT)
	{
		if((acl_filter->ingress_src_l4_port_start == acl_filter->ingress_src_l4_port_end) && (acl_filter->filter_fields & INGRESS_L4_TCP_BIT)){
			//SINGLE TCP Port
			rg_db.systemGlobal.aclField[TEMPLATE_TCP_SPORT].fieldType = ACL_FIELD_PATTERN_MATCH;
			rg_db.systemGlobal.aclField[TEMPLATE_TCP_SPORT].fieldUnion.pattern.fieldIdx = (TEMPLATE_TCP_SPORT%ACL_PER_RULE_FIELD_SIZE);
			rg_db.systemGlobal.aclField[TEMPLATE_TCP_SPORT].fieldUnion.data.value = acl_filter->ingress_src_l4_port_start;
			rg_db.systemGlobal.aclField[TEMPLATE_TCP_SPORT].fieldUnion.data.mask = 0xffff;

		}else if((acl_filter->ingress_src_l4_port_start == acl_filter->ingress_src_l4_port_end) && (acl_filter->filter_fields & INGRESS_L4_UDP_BIT)){
			//SINGLE UDP Port
			rg_db.systemGlobal.aclField[TEMPLATE_UDP_SPORT].fieldType = ACL_FIELD_PATTERN_MATCH;
			rg_db.systemGlobal.aclField[TEMPLATE_UDP_SPORT].fieldUnion.pattern.fieldIdx = (TEMPLATE_UDP_SPORT%ACL_PER_RULE_FIELD_SIZE);
			rg_db.systemGlobal.aclField[TEMPLATE_UDP_SPORT].fieldUnion.data.value = acl_filter->ingress_src_l4_port_start;
			rg_db.systemGlobal.aclField[TEMPLATE_UDP_SPORT].fieldUnion.data.mask = 0xffff;

		}else{
			uint32 pattern_idx=0;
			if(_rtk_rg_search_acl_exist_portTableEntry(&pattern_idx,acl_filter->ingress_src_l4_port_end,acl_filter->ingress_src_l4_port_start,PORTRANGE_SPORT)==RT_ERR_RG_OK)
			{
				//found there is exist the same port_range entry can be share
				ACL_CTRL("Found acl SportRange[%d] can be share with this rule.",pattern_idx);
			}
			else
			{
				ret = _rtk_rg_search_acl_empty_portTableEntry(&pattern_idx,RANGE_TABLE_SEARCH_FOR_ACL);
				if(ret!=RT_ERR_RG_OK) return ret;
			}


			bzero(&rg_db.systemGlobal.aclSportRangeEntry, sizeof(rg_db.systemGlobal.aclSportRangeEntry));
			rg_db.systemGlobal.aclSportRangeEntry.index = pattern_idx;
			rg_db.systemGlobal.aclSportRangeEntry.upper_bound=acl_filter->ingress_src_l4_port_end;
			rg_db.systemGlobal.aclSportRangeEntry.lower_bound=acl_filter->ingress_src_l4_port_start;
			rg_db.systemGlobal.aclSportRangeEntry.type = PORTRANGE_SPORT;


			rg_db.systemGlobal.aclField[TEMPLATE_L4PORT_RANGE].fieldType = ACL_FIELD_PATTERN_MATCH;
			rg_db.systemGlobal.aclField[TEMPLATE_L4PORT_RANGE].fieldUnion.pattern.fieldIdx = (TEMPLATE_L4PORT_RANGE%ACL_PER_RULE_FIELD_SIZE);
			rg_db.systemGlobal.aclField[TEMPLATE_L4PORT_RANGE].fieldUnion.data.value |= (1<<(pattern_idx));
			rg_db.systemGlobal.aclField[TEMPLATE_L4PORT_RANGE].fieldUnion.data.mask |= (1<<(pattern_idx));

			if(rtk_acl_portRange_set(&rg_db.systemGlobal.aclSportRangeEntry))
			{
				ACL_CTRL("access acl SportRange table failed");
				return(RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
			}
			else
			{
				ACL_CTRL("use HW ACL PORT_RANGE[%d]",pattern_idx);
				aclSWEntry->hw_used_table |= APOLLOFE_RG_ACL_USED_INGRESS_SPORTTABLE;
				aclSWEntry->hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_SPORTTABLE_INDEX] = pattern_idx;
			}
		}
	}

	if(acl_filter->filter_fields & INGRESS_L4_DPORT_RANGE_BIT)
	{
		if((acl_filter->ingress_dest_l4_port_start == acl_filter->ingress_dest_l4_port_end) && (acl_filter->filter_fields & INGRESS_L4_TCP_BIT)){
			//SINGLE TCP DPort
			rg_db.systemGlobal.aclField[TEMPLATE_TCP_DPORT].fieldType = ACL_FIELD_PATTERN_MATCH;
			rg_db.systemGlobal.aclField[TEMPLATE_TCP_DPORT].fieldUnion.pattern.fieldIdx = (TEMPLATE_TCP_DPORT%ACL_PER_RULE_FIELD_SIZE);
			rg_db.systemGlobal.aclField[TEMPLATE_TCP_DPORT].fieldUnion.data.value = acl_filter->ingress_dest_l4_port_start;
			rg_db.systemGlobal.aclField[TEMPLATE_TCP_DPORT].fieldUnion.data.mask = 0xffff;

		}else if((acl_filter->ingress_dest_l4_port_start == acl_filter->ingress_dest_l4_port_end) && (acl_filter->filter_fields & INGRESS_L4_UDP_BIT)){
			//SINGLE UDP DPort
			rg_db.systemGlobal.aclField[TEMPLATE_UDP_DPORT].fieldType = ACL_FIELD_PATTERN_MATCH;
			rg_db.systemGlobal.aclField[TEMPLATE_UDP_DPORT].fieldUnion.pattern.fieldIdx = (TEMPLATE_UDP_DPORT%ACL_PER_RULE_FIELD_SIZE);
			rg_db.systemGlobal.aclField[TEMPLATE_UDP_DPORT].fieldUnion.data.value = acl_filter->ingress_dest_l4_port_start;
			rg_db.systemGlobal.aclField[TEMPLATE_UDP_DPORT].fieldUnion.data.mask = 0xffff;

		}else{
			uint32 pattern_idx=0;
			if(_rtk_rg_search_acl_exist_portTableEntry(&pattern_idx,acl_filter->ingress_dest_l4_port_end,acl_filter->ingress_dest_l4_port_start,PORTRANGE_DPORT)==RT_ERR_RG_OK)
			{
				//found there is exist the same port_range entry can be share
				ACL_CTRL("Found acl DportRange[%d] can be share with this rule.",pattern_idx);
			}
			else
			{
				ret = _rtk_rg_search_acl_empty_portTableEntry(&pattern_idx,RANGE_TABLE_SEARCH_FOR_ACL);
				if(ret!=RT_ERR_RG_OK) return ret;
			}


			bzero(&rg_db.systemGlobal.aclDportRangeEntry, sizeof(rg_db.systemGlobal.aclDportRangeEntry));
			rg_db.systemGlobal.aclDportRangeEntry.index = pattern_idx;
			rg_db.systemGlobal.aclDportRangeEntry.upper_bound=acl_filter->ingress_dest_l4_port_end;
			rg_db.systemGlobal.aclDportRangeEntry.lower_bound=acl_filter->ingress_dest_l4_port_start;
			rg_db.systemGlobal.aclDportRangeEntry.type = PORTRANGE_DPORT;

			rg_db.systemGlobal.aclField[TEMPLATE_L4PORT_RANGE].fieldType = ACL_FIELD_PATTERN_MATCH;
			rg_db.systemGlobal.aclField[TEMPLATE_L4PORT_RANGE].fieldUnion.pattern.fieldIdx = (TEMPLATE_L4PORT_RANGE%ACL_PER_RULE_FIELD_SIZE);
			rg_db.systemGlobal.aclField[TEMPLATE_L4PORT_RANGE].fieldUnion.data.value |= (1<<(pattern_idx));
			rg_db.systemGlobal.aclField[TEMPLATE_L4PORT_RANGE].fieldUnion.data.mask |= (1<<(pattern_idx));

			if(rtk_acl_portRange_set(&rg_db.systemGlobal.aclDportRangeEntry))
			{
				ACL_CTRL("access HW ACL PORT_RANGE table failed");
				return(RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
			}
			else
			{
				ACL_CTRL("use HW ACL PORT_RANGE[%d]",pattern_idx);
				aclSWEntry->hw_used_table |= APOLLOFE_RG_ACL_USED_INGRESS_DPORTTABLE;
				aclSWEntry->hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_DPORTTABLE_INDEX] = pattern_idx;
			}
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV4_SIP_RANGE_BIT)
	{
		if(acl_filter->ingress_src_ipv4_addr_start == acl_filter->ingress_src_ipv4_addr_end ){
			//SINGLE IP
			rg_db.systemGlobal.aclField[TEMPLATE_IPv4SIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
			rg_db.systemGlobal.aclField[TEMPLATE_IPv4SIP0].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv4SIP0%ACL_PER_RULE_FIELD_SIZE);
			rg_db.systemGlobal.aclField[TEMPLATE_IPv4SIP0].fieldUnion.data.value = (acl_filter->ingress_src_ipv4_addr_start & 0xffff);//SIP[15:0]
			rg_db.systemGlobal.aclField[TEMPLATE_IPv4SIP0].fieldUnion.data.mask = 0xffff;

			rg_db.systemGlobal.aclField[TEMPLATE_IPv4SIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
			rg_db.systemGlobal.aclField[TEMPLATE_IPv4SIP1].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv4SIP1%ACL_PER_RULE_FIELD_SIZE);
			rg_db.systemGlobal.aclField[TEMPLATE_IPv4SIP1].fieldUnion.data.value = (acl_filter->ingress_src_ipv4_addr_start & 0xffff0000)>>16;//SIP[31:16]
			rg_db.systemGlobal.aclField[TEMPLATE_IPv4SIP1].fieldUnion.data.mask = 0xffff;

		}else{
			//IP RANGE
			int i;
			uint32 mask=0;
			ret = _rtk_rg_maskLength_get_by_ipv4_range(acl_filter->ingress_src_ipv4_addr_start,acl_filter->ingress_src_ipv4_addr_end,&ipv4_unmask_length);
			if(ret == RT_ERR_RG_OK)
			{
				//build IPv4 Range mask
				for(i=0;i<32;i++){
					if(i>=ipv4_unmask_length){mask|=(1<<i);}
				}

				rg_db.systemGlobal.aclField[TEMPLATE_IPv4SIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField[TEMPLATE_IPv4SIP0].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv4SIP0%ACL_PER_RULE_FIELD_SIZE);
				rg_db.systemGlobal.aclField[TEMPLATE_IPv4SIP0].fieldUnion.data.value = (acl_filter->ingress_src_ipv4_addr_start & 0xffff);//SIP[15:0]
				rg_db.systemGlobal.aclField[TEMPLATE_IPv4SIP0].fieldUnion.data.mask = (mask&0x0000ffff);

				rg_db.systemGlobal.aclField[TEMPLATE_IPv4SIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField[TEMPLATE_IPv4SIP1].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv4SIP1%ACL_PER_RULE_FIELD_SIZE);
				rg_db.systemGlobal.aclField[TEMPLATE_IPv4SIP1].fieldUnion.data.value = (acl_filter->ingress_src_ipv4_addr_start & 0xffff0000)>>16;//SIP[31:16]
				rg_db.systemGlobal.aclField[TEMPLATE_IPv4SIP1].fieldUnion.data.mask = (mask >> 16);
			}
			else
			{
				uint32 pattern_idx=0;
				if(_rtk_rg_search_acl_exist_ipTableEntry(&pattern_idx,acl_filter->ingress_src_ipv4_addr_end,acl_filter->ingress_src_ipv4_addr_start,IPRANGE_IPV4_SIP)==RT_ERR_RG_OK)
				{
					//found there is exist the same port_range entry can be share
					ACL_CTRL("Found acl IpRange[%d] can be share with this rule.",pattern_idx);
				}
				else
				{
					ret = _rtk_rg_search_acl_empty_ipTableEntry(&pattern_idx,RANGE_TABLE_SEARCH_FOR_ACL);
					if(ret!=RT_ERR_RG_OK) return ret;
				}

				bzero(&rg_db.systemGlobal.aclSIPRangeEntry, sizeof(rg_db.systemGlobal.aclSIPRangeEntry));
				rg_db.systemGlobal.aclSIPRangeEntry.index = pattern_idx;
				rg_db.systemGlobal.aclSIPRangeEntry.upperIp=acl_filter->ingress_src_ipv4_addr_end;
				rg_db.systemGlobal.aclSIPRangeEntry.lowerIp=acl_filter->ingress_src_ipv4_addr_start;
				rg_db.systemGlobal.aclSIPRangeEntry.type = IPRANGE_IPV4_SIP;

				rg_db.systemGlobal.aclField[TEMPLATE_IP_RANGE].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField[TEMPLATE_IP_RANGE].fieldUnion.pattern.fieldIdx = (TEMPLATE_IP_RANGE%ACL_PER_RULE_FIELD_SIZE);
				rg_db.systemGlobal.aclField[TEMPLATE_IP_RANGE].fieldUnion.data.value |= (1<<(pattern_idx));
				rg_db.systemGlobal.aclField[TEMPLATE_IP_RANGE].fieldUnion.data.mask |= (1<<(pattern_idx));

				if(rtk_acl_ipRange_set(&rg_db.systemGlobal.aclSIPRangeEntry))
				{
					ACL_CTRL("access HW ACL IP_RANGE table failed");
					return(RT_ERR_RG_ACL_IPTABLE_ACCESS_FAILED);
				}
				else
				{
					ACL_CTRL("use HW ACL IP_RANGE[%d]",pattern_idx);
					aclSWEntry->hw_used_table |= APOLLOFE_RG_ACL_USED_INGRESS_SIP4TABLE;
					aclSWEntry->hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_SIP4TABLE_INDEX] = pattern_idx;
				}
			}
		}

	}

	if(acl_filter->filter_fields & INGRESS_IPV4_DIP_RANGE_BIT)
	{
		if(acl_filter->ingress_dest_ipv4_addr_start == acl_filter->ingress_dest_ipv4_addr_end ){
			//SINGLE IP
			rg_db.systemGlobal.aclField[TEMPLATE_IPv4DIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
			rg_db.systemGlobal.aclField[TEMPLATE_IPv4DIP0].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv4DIP0%ACL_PER_RULE_FIELD_SIZE);
			rg_db.systemGlobal.aclField[TEMPLATE_IPv4DIP0].fieldUnion.data.value = (acl_filter->ingress_dest_ipv4_addr_start & 0xffff);//DIP[15:0]
			rg_db.systemGlobal.aclField[TEMPLATE_IPv4DIP0].fieldUnion.data.mask = 0xffff;

			rg_db.systemGlobal.aclField[TEMPLATE_IPv4DIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
			rg_db.systemGlobal.aclField[TEMPLATE_IPv4DIP1].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv4DIP1%ACL_PER_RULE_FIELD_SIZE);
			rg_db.systemGlobal.aclField[TEMPLATE_IPv4DIP1].fieldUnion.data.value = (acl_filter->ingress_dest_ipv4_addr_start & 0xffff0000)>>16;//DIP[31:16]
			rg_db.systemGlobal.aclField[TEMPLATE_IPv4DIP1].fieldUnion.data.mask = 0xffff;

		}else{
			//IP RANGE
			int i;
			uint32 mask=0;
			ret = _rtk_rg_maskLength_get_by_ipv4_range(acl_filter->ingress_dest_ipv4_addr_start,acl_filter->ingress_dest_ipv4_addr_end,&ipv4_unmask_length);
			if(ret == RT_ERR_RG_OK)
			{
				//build IPv4 Range mask
				for(i=0;i<32;i++){
					if(i>=ipv4_unmask_length){mask|=(1<<i);}
				}

				rg_db.systemGlobal.aclField[TEMPLATE_IPv4DIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField[TEMPLATE_IPv4DIP0].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv4DIP0%ACL_PER_RULE_FIELD_SIZE);
				rg_db.systemGlobal.aclField[TEMPLATE_IPv4DIP0].fieldUnion.data.value = (acl_filter->ingress_dest_ipv4_addr_start & 0xffff);//DIP[15:0]
				rg_db.systemGlobal.aclField[TEMPLATE_IPv4DIP0].fieldUnion.data.mask = (mask&0x0000ffff);

				rg_db.systemGlobal.aclField[TEMPLATE_IPv4DIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField[TEMPLATE_IPv4DIP1].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPv4DIP1%ACL_PER_RULE_FIELD_SIZE);
				rg_db.systemGlobal.aclField[TEMPLATE_IPv4DIP1].fieldUnion.data.value = (acl_filter->ingress_dest_ipv4_addr_start & 0xffff0000)>>16;//DIP[31:16]
				rg_db.systemGlobal.aclField[TEMPLATE_IPv4DIP1].fieldUnion.data.mask = (mask >> 16);
			}
			else
			{
				uint32 pattern_idx=0;
				if(_rtk_rg_search_acl_exist_ipTableEntry(&pattern_idx,acl_filter->ingress_dest_ipv4_addr_end,acl_filter->ingress_dest_ipv4_addr_start,IPRANGE_IPV4_DIP)==RT_ERR_RG_OK)
				{
					//found there is exist the same port_range entry can be share
					ACL_CTRL("Found acl IpRange[%d] can be share with this rule.",pattern_idx);
				}
				else
				{
					ret = _rtk_rg_search_acl_empty_ipTableEntry(&pattern_idx,RANGE_TABLE_SEARCH_FOR_ACL);
					if(ret!=RT_ERR_RG_OK) return ret;
				}

				bzero(&rg_db.systemGlobal.aclDIPRangeEntry, sizeof(rg_db.systemGlobal.aclDIPRangeEntry));
				rg_db.systemGlobal.aclDIPRangeEntry.index = pattern_idx;
				rg_db.systemGlobal.aclDIPRangeEntry.upperIp=acl_filter->ingress_dest_ipv4_addr_end;
				rg_db.systemGlobal.aclDIPRangeEntry.lowerIp=acl_filter->ingress_dest_ipv4_addr_start;
				rg_db.systemGlobal.aclDIPRangeEntry.type = IPRANGE_IPV4_DIP;

				rg_db.systemGlobal.aclField[TEMPLATE_IP_RANGE].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField[TEMPLATE_IP_RANGE].fieldUnion.pattern.fieldIdx = (TEMPLATE_IP_RANGE%ACL_PER_RULE_FIELD_SIZE);
				rg_db.systemGlobal.aclField[TEMPLATE_IP_RANGE].fieldUnion.data.value |= (1<<(pattern_idx));
				rg_db.systemGlobal.aclField[TEMPLATE_IP_RANGE].fieldUnion.data.mask |= (1<<(pattern_idx));

				if(rtk_acl_ipRange_set(&rg_db.systemGlobal.aclDIPRangeEntry))
				{
					ACL_CTRL("access HW ACL IP_RANGE table failed");
					return(RT_ERR_RG_ACL_IPTABLE_ACCESS_FAILED);
				}
				else
				{
					ACL_CTRL("use HW ACL IP_RANGE[%d]",pattern_idx);
					aclSWEntry->hw_used_table |= APOLLOFE_RG_ACL_USED_INGRESS_DIP4TABLE;
					aclSWEntry->hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_DIP4TABLE_INDEX] = pattern_idx;
				}
			}
		}

	}


	if(acl_filter->filter_fields & INGRESS_IPV6_FLOWLABEL_BIT)
	{
		//ipv6_flow_label[0:3]
		rg_db.systemGlobal.aclField[TEMPLATE_IPV6_FLOW_LABEL_0].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_IPV6_FLOW_LABEL_0].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPV6_FLOW_LABEL_0%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_IPV6_FLOW_LABEL_0].fieldUnion.data.value = (((acl_filter->ingress_ipv6_flow_label)>>16) & 0xf);
		rg_db.systemGlobal.aclField[TEMPLATE_IPV6_FLOW_LABEL_0].fieldUnion.data.mask = 0xf;

		//ipv6_flow_label[4:19]
		rg_db.systemGlobal.aclField[TEMPLATE_IPV6_FLOW_LABEL_1].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_IPV6_FLOW_LABEL_1].fieldUnion.pattern.fieldIdx = (TEMPLATE_IPV6_FLOW_LABEL_1%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_IPV6_FLOW_LABEL_1].fieldUnion.data.value = ((acl_filter->ingress_ipv6_flow_label) & 0xffff);
		rg_db.systemGlobal.aclField[TEMPLATE_IPV6_FLOW_LABEL_1].fieldUnion.data.mask = 0xffff;
	}

	if(acl_filter->filter_fields & INGRESS_TCP_FLAGS_BIT)
	{
		rg_db.systemGlobal.aclField[TEMPLATE_TCP_FLAGS].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_TCP_FLAGS].fieldUnion.pattern.fieldIdx = (TEMPLATE_TCP_FLAGS%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_TCP_FLAGS].fieldUnion.data.value = acl_filter->ingress_tcp_flags&0x1ff;
		rg_db.systemGlobal.aclField[TEMPLATE_TCP_FLAGS].fieldUnion.data.mask = acl_filter->ingress_tcp_flags_mask&0x1ff;

		rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.pattern.fieldIdx = (TEMPLATE_FRAME_TYPE_TAGS%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.value |= (1<<10); //TCP bit
		rg_db.systemGlobal.aclField[TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.mask |=(1<<10); //TCP bit
	}

	if(acl_filter->filter_fields & INGRESS_PKT_LEN_RANGE_BIT)
	{
		uint16 pattern_idx=0;

		ret = _rtk_rg_search_acl_pktLenRangeTableEntry(&pattern_idx,FALSE,acl_filter->ingress_packet_length_end,acl_filter->ingress_packet_length_start);
		if(ret!=RT_ERR_RG_OK){
			ACL_CTRL("access HW ACL PKTLEN_RANGE table failed");
			return ret;
		}

		rg_db.systemGlobal.aclField[TEMPLATE_PKT_LEN_RANGE].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_PKT_LEN_RANGE].fieldUnion.pattern.fieldIdx = (TEMPLATE_PKT_LEN_RANGE%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_PKT_LEN_RANGE].fieldUnion.data.value |= (1<<(pattern_idx));
		rg_db.systemGlobal.aclField[TEMPLATE_PKT_LEN_RANGE].fieldUnion.data.mask |= (1<<(pattern_idx));

		ACL_CTRL("use HW ACL PKTLEN_RANGE[%d]",pattern_idx);
		aclSWEntry->hw_used_table |= APOLLOFE_RG_ACL_USED_INGRESS_PKTLENTABLE;
		aclSWEntry->hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_PKTLENTABLE_INDEX] = pattern_idx;
	}


    //fieldadd to rg_db.systemGlobal.aclRule, and record the valid ACL rule.
	for(j=0;j<GLOBAL_ACL_RULE_SIZE;j++)
	{
	    for(i=(j*ACL_PER_RULE_FIELD_SIZE); i<((j+1)*ACL_PER_RULE_FIELD_SIZE); i++)
	    {
	        if(memcmp(&rg_db.systemGlobal.aclField[i],&rg_db.systemGlobal.aclFieldEmpty,sizeof(rtk_acl_field_t)))
	        {
	            rg_db.systemGlobal.aclRuleValid[j] = ENABLED;
	            if(rtk_acl_igrRuleField_add(&rg_db.systemGlobal.aclRule[j], &rg_db.systemGlobal.aclField[i]))
	            {
	                 ACL_CTRL("set field[%d] to aclTmpRule[%d] Fail!",i,j);
	                return(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
	            }
				else
				{
					 //ACL_CTRL("set field[%d] to aclTmpRule[%d]",i,j);
				}
	        }
	    }
	}


	aclActionRuleIdx = 0; //if no pattern is need, default use the rg_db.systemGlobal.aclRule[0] to assign action.
	aclCountinueRuleSize = 0;
	firstHitFlag=0;
	for(i=0;i<GLOBAL_ACL_RULE_SIZE;i++) //find first valid rule to assign action.
	{
		if(rg_db.systemGlobal.aclRuleValid[i]==ENABLED){
			//ACL_CTRL("aclTmpRule[%d] is valid, and will add to HW_ACL",aclActionRuleIdx);
			aclCountinueRuleSize++;

			if(firstHitFlag==0)
			{
				aclActionRuleIdx = i; //record the first aclTmpRule to assign actions.
				firstHitFlag=1;
			}
		}
	}


	//For case: although no rg_db.systemGlobal.aclField is need, but still need one rg_db.systemGlobal.aclRule to filter basic pattern,such as ingress_portmask, stagif, ctagif ...etc.  and do actions
	if(memcmp(&rg_db.systemGlobal.aclRule[0],&rg_db.systemGlobal.aclRuleEmpty,sizeof(rtk_acl_ingress_entry_t)) && aclCountinueRuleSize==0)//use rg_db.systemGlobal.aclRule[0] to contant the pattern and action, and add to HW.
	{
		aclCountinueRuleSize = 1;
		aclActionRuleIdx = 0;
		rg_db.systemGlobal.aclRuleValid[0]=ENABLED;
	}

	//ACL_CTRL("the action is set in aclTmpRule[%d]",aclActionRuleIdx);

	/*set actions*/
	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)//use ACL action
	{
		switch(acl_filter->action_type)
		{
			case ACL_ACTION_TYPE_DROP:
				rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLED;
				rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.forwardAct.act= ACL_IGR_FORWARD_EGRESSMASK_ACT;
				rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.forwardAct.portMask.bits[0]=0x0;
				break;

			case ACL_ACTION_TYPE_PERMIT:
		        rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLED;
		        rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.forwardAct.act= ACL_IGR_FORWARD_EGRESSMASK_ACT;
				rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.forwardAct.portMask.bits[0]= RTK_RG_ALL_MAC_PORTMASK;
				break;

			case ACL_ACTION_TYPE_TRAP:
				rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLED;
		        rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.forwardAct.act= ACL_IGR_FORWARD_TRAP_ACT;
				break;

			case ACL_ACTION_TYPE_QOS:
		        for(i=0; (0x1<<i)<ACL_ACTION_QOS_END; i++)
		        {
		            switch((acl_filter->qos_actions&(1<<i)))
		            {
						case ACL_ACTION_NOP_BIT: break;
						case ACL_ACTION_1P_REMARKING_BIT:
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_CVLAN_ACT] = ENABLED;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.cvlanAct.act= ACL_IGR_CVLAN_1P_REMARK_ACT;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.cvlanAct.dot1p=acl_filter->action_dot1p_remarking_pri;
							break;

						case ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT:
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_SVLAN_ACT] = ENABLED;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.svlanAct.act = ACL_IGR_SVLAN_DSCP_REMARK_ACT;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.svlanAct.dscp = (acl_filter->action_ip_precedence_remarking_pri << 3);
							break;

						case ACL_ACTION_DSCP_REMARKING_BIT:
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_SVLAN_ACT] = ENABLED;
		                    rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.svlanAct.act = ACL_IGR_SVLAN_DSCP_REMARK_ACT;
		                    rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.svlanAct.dscp = acl_filter->action_dscp_remarking_pri;
			 				break;

						case ACL_ACTION_TOS_TC_REMARKING_BIT:
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_PRI_ACT] = ENABLED;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.priAct.act = ACL_IGR_PRI_TOS_REMARK_ACT;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.priAct.tos = acl_filter->action_tos_tc_remarking_pri;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.priAct.tosMask = 0xff;	//reference action bit for tos
							break;

						case ACL_ACTION_QUEUE_ID_BIT:
							WARNING("ACL_ACTION_QUEUE_ID_BIT is not support, please use ACL_ACTION_ACL_PRIORITY_BIT!");
							return(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_SHARE_METER_BIT:
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_LOG_ACT] = ENABLED;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.logAct.act = ACL_IGR_LOG_POLICING_ACT;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.logAct.meter = acl_filter->action_share_meter;
							break;

						case ACL_ACTION_LOG_COUNTER_BIT:
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_LOG_ACT] = ENABLED;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.logAct.act = ACL_IGR_LOG_MIB_ACT;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.logAct.mib = acl_filter->action_log_counter;
							break;

						case ACL_ACTION_STREAM_ID_OR_LLID_BIT:
							ACL_CTRL("ACL_ACTION_QUEUE_ID_BIT is not support with fwdtype ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET");
							return(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_ACL_PRIORITY_BIT:
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_PRI_ACT] = ENABLED;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.priAct.act = ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.priAct.aclPri = acl_filter->action_acl_priority;
							break;

						case ACL_ACTION_ACL_CVLANTAG_BIT:
							ACL_CTRL("ACL_ACTION_ACL_CVLANTAG_BIT is not support by HW_ACL");
							return(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_ACL_SVLANTAG_BIT:
							ACL_CTRL("ACL_ACTION_ACL_SVLANTAG_BIT is not support by HW_ACL");
							return(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_ACL_INGRESS_VID_BIT:
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_CVLAN_ACT] = ENABLED;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.cvlanAct.act = ACL_IGR_CVLAN_IGR_CVLAN_ACT;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.cvlanAct.cvid = (acl_filter->action_acl_ingress_vid & 0xfff);
							break;

						case ACL_ACTION_DS_UNIMASK_BIT:
							ACL_CTRL("ACL_ACTION_ACL_SVLANTAG_BIT is not support by HW_ACL");
							return(RT_ERR_RG_INVALID_PARAM);
							break;
						case ACL_ACTION_REDIRECT_BIT:
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLED;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.forwardAct.act = ACL_IGR_FORWARD_REDIRECT_ACT;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.forwardAct.portMask.bits[0]= acl_filter->redirect_portmask;
							break;

						case ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT:
							ACL_CTRL("ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT is not support by HW_ACL");
							return(RT_ERR_RG_INVALID_PARAM);
							break;
						case ACL_ACTION_QOS_END:
							break;
						default:
							break;
					}
		        }
				break;

			case ACL_ACTION_TYPE_TRAP_TO_PS:
				rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLED;
		        rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.forwardAct.act= ACL_IGR_FORWARD_TRAP_ACT;
				break;

			case ACL_ACTION_TYPE_POLICY_ROUTE:
				ACL_CTRL("ACL_ACTION_TYPE_POLICY_ROUTE is not support by HW_ACL");
				return(RT_ERR_RG_INVALID_PARAM);
				break;

			case ACL_ACTION_TYPE_TRAP_WITH_PRIORITY:
				rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLED;
				rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.forwardAct.act= ACL_IGR_FORWARD_TRAP_ACT;
				rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_PRI_ACT] = ENABLED;
				rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.priAct.act = ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
				rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.priAct.aclPri = acl_filter->action_trap_with_priority;
				break;

			case ACL_ACTION_TYPE_TRAP2SLAVE_WITH_PRIORITY: 
				rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLED;
				rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.forwardAct.act= ACL_IGR_FORWARD_TRAP2SLAVE_ACT;
				rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_PRI_ACT] = ENABLED;
				rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.priAct.act = ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
				rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.priAct.aclPri = acl_filter->action_trap_with_priority;
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
	ret = _rtk_rg_search_acl_empty_Entry(aclCountinueRuleSize, &aclLatchToCfIdx);
	if(ret!=RT_ERR_RG_OK){
		ACL_PTOOL_VERIFY_END;
		return ret;
	}
	//ACL_CTRL("aclTmpRule will add to ACL[%d] (continue:%d)",aclLatchToCfIdx,aclCountinueRuleSize);

	index = aclLatchToCfIdx;//index is used for HW ACL countinuous rule.
    for(i=0; i<GLOBAL_ACL_RULE_SIZE; i++)
    {
    	if((i==aclActionRuleIdx) && (aclCountinueRuleSize>1))
		{
			ACL_CTRL("postpone add aclTmpRule[%d] with action to HW ACL[%d] untill all pattern add finish",i,aclLatchToCfIdx);
			index++;
			continue;
		}
        if(rg_db.systemGlobal.aclRuleValid[i]==ENABLED)
        {
            rg_db.systemGlobal.aclRule[i].index = index;
            rg_db.systemGlobal.aclRule[i].valid = ENABLED;
            rg_db.systemGlobal.aclRule[i].templateIdx = i;
			ret = RTK_ACL_IGRRULEENTRY_ADD(&rg_db.systemGlobal.aclRule[i], shareHwAclWithSWAclIdx);
            if(ret)
            {
                ACL_CTRL("adding aclTmpRule[%d] to HW ACL[%d] failed. ret=0x%x",i,index,ret);
                return(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
            }

			ACL_CTRL("adding aclTmpRule[%d] to HW ACL[%d] success. ret=0x%x",i,rg_db.systemGlobal.aclRule[i].index,ret);
			index++;
        }
    }

    if(aclCountinueRuleSize>1) {	//add action untill all pattern add finish prevent mis-hit
	    rg_db.systemGlobal.aclRule[aclActionRuleIdx].index = aclLatchToCfIdx;
	    rg_db.systemGlobal.aclRule[aclActionRuleIdx].valid = ENABLED;
	    rg_db.systemGlobal.aclRule[aclActionRuleIdx].templateIdx = aclActionRuleIdx;
		ret = RTK_ACL_IGRRULEENTRY_ADD(&rg_db.systemGlobal.aclRule[aclActionRuleIdx], shareHwAclWithSWAclIdx);
	    if(ret)
	    {
	        ACL_CTRL("adding aclTmpRule[%d] with action to HW ACL[%d] failed. ret=0x%x",aclActionRuleIdx,aclLatchToCfIdx,ret);
	        return(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
	    }
		ACL_CTRL("adding aclTmpRule[%d] with action to HW ACL[%d] success. ret=0x%x",aclActionRuleIdx,rg_db.systemGlobal.aclRule[aclActionRuleIdx].index,ret);
	}

	if(	portmask_need_double_hw_rule==1 && portmask_need_double_hw_rule_add_sencond==1)//the second dummy H/W ACL rule for mac_port
	{
		aclSWEntry->hw_aclEntry_size += aclCountinueRuleSize;
	}
	else
	{
		aclSWEntry->hw_aclEntry_start = aclLatchToCfIdx;
		aclSWEntry->hw_aclEntry_size = aclCountinueRuleSize;
	}

	ACL_CTRL("using HW_ACL hw_aclEntry_start is %d, hw_aclEntry_size is %d",aclSWEntry->hw_aclEntry_start,aclSWEntry->hw_aclEntry_size);

	if( portmask_need_double_hw_rule==1 && portmask_need_double_hw_rule_add_sencond==0){	//rule add for first time
		//DEBUG("ext_port rule added: hw_aclEntry_start=%d, hw_aclEntry_size=%d",aclSWEntry->hw_aclEntry_start,aclSWEntry->hw_aclEntry_size);
		portmask_need_double_hw_rule_add_sencond = 1;
		goto PORTMASK_NEED_DOUBLE_RULE;
	}

	if( portmask_need_double_hw_rule==1 && portmask_need_double_hw_rule_add_sencond==1){	//rule add for second time, restore origianl hw_aclEntry_start and accumulate hw_aclEntry_size
		//DEBUG("mac_port rule added: hw_aclEntry_start=%d, hw_aclEntry_size=%d",aclSWEntry->hw_aclEntry_start,aclSWEntry->hw_aclEntry_size);
	}


	ACL_PTOOL_VERIFY_END;
	return (RT_ERR_RG_OK);
}


#endif


static int _rtk_rg_free_acl_portTableEntry(int index)
{
	int ret;
    rtk_acl_rangeCheck_l4Port_t aclPortRangeEntry;
    bzero(&aclPortRangeEntry,sizeof(aclPortRangeEntry));
    aclPortRangeEntry.index=index;
	ret = rtk_acl_portRange_set(&aclPortRangeEntry);
    if(ret!= RT_ERR_RG_OK)
    {
        ACL_CTRL("free acl porttable failed, ret(rtk)=%d",ret);
        RETURN_ERR(RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
    }
    return (RT_ERR_RG_OK);
}

static int _rtk_rg_free_acl_ipTableEntry(int index)
{
	int ret;
    rtk_acl_rangeCheck_ip_t aclIpRangeEntry;
    bzero(&aclIpRangeEntry,sizeof(aclIpRangeEntry));
    aclIpRangeEntry.index=index;
	ret = rtk_acl_ipRange_set(&aclIpRangeEntry);
    if(ret!= RT_ERR_RG_OK)
    {
        ACL_CTRL("free acl iptable failed, ret(rtk)=%d",ret);
        RETURN_ERR(RT_ERR_RG_ACL_IPTABLE_ACCESS_FAILED);
    }
    return (RT_ERR_RG_OK);
}

static int _rtk_rg_free_acl_pktLenTableEntry(int index)
{
	int ret;
	rtk_acl_rangeCheck_pktLength_t pktLenRange;
	bzero(&pktLenRange,sizeof(pktLenRange));
    pktLenRange.index=index;
	ret = rtk_acl_packetLengthRange_set(&pktLenRange);
    if(ret!= RT_ERR_RG_OK)
    {
        ACL_CTRL("free acl pktLenRangetable failed, ret(rtk)=%d",ret);
        ACL_RSV("free acl pktLenRangetable failed, ret(rtk)=%d",ret);
        RETURN_ERR(RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
    }
    return (RT_ERR_RG_OK);
}

static int _rtk_rg_reset_asic_TableEntry(int reset_all, int acl_start, int acl_count)
{
	int i;
	int acl_end = 0;
	ACL_PTOOL_VERIFY_START;

	if(acl_count)
	{	
		acl_end = acl_start+acl_count-1;
		//delete H/W ACL , not include reserved entries
		for(i=acl_start; i<=acl_end; i++)
		{
			if(rtk_acl_igrRuleEntry_del(i))
			{
				RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
			}
		}
		ACL_CTRL("clear HW_ACL[%d~%d]",acl_start,acl_end);
	}

	if(reset_all)
	{
		//delete ACL IP range table
		for(i=0; i<MAX_ACL_IPRANGETABLE_SIZE; i++)
		{
			if(_rtk_rg_free_acl_ipTableEntry(i))
			{
				RETURN_ERR(RT_ERR_RG_ACL_IPTABLE_ACCESS_FAILED);
			}
		}
		ACL_CTRL("clear all HW_ACL IP_RANGE[%d~%d]",0,MAX_ACL_IPRANGETABLE_SIZE);
		
		//delete ACL Port range table
		for(i=0; i<MAX_ACL_PORTRANGETABLE_SIZE; i++)
		{
			if(_rtk_rg_free_acl_portTableEntry(i))
			{
				RETURN_ERR(RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
			}
		}
		ACL_CTRL("clear all HW_ACL PORT_RANGE[%d~%d]",0,MAX_ACL_PORTRANGETABLE_SIZE);

		//delete ACL Packet length range table
		for(i=USER_ACL_PKTLENRANGETABLE_START; i<MAX_ACL_PKTLENRANGETABLE_SIZE; i++)
		{
			if(_rtk_rg_free_acl_pktLenTableEntry(i))
			{
				RETURN_ERR(RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
			}
		}
		ACL_CTRL("clear all HW_ACL PKTLEN_RANGE[%d~%d]",USER_ACL_PKTLENRANGETABLE_START,MAX_ACL_PKTLENRANGETABLE_SIZE-1);
	}

	ACL_PTOOL_VERIFY_END;
	return (RT_ERR_RG_OK);
}


void _rtk_rg_acl_for_multicast_temp_protection_add(void)
{
#if defined(HW_ACL_REARRANGE_PROTECT_VERSION) && (HW_ACL_REARRANGE_PROTECT_VERSION==2)
	rtk_acl_ingress_entry_t aclRule;
	rtk_acl_field_t aclField;

	if(rg_db.systemGlobal.flow_flush_disable_by_feature&RTK_RG_FFD_BIT_ACL){
		//only add once when flow flush disable for saving time
		bzero(&aclRule, sizeof(aclRule));
		aclRule.index = HW_ACL_REARRANGE_PROTECT_V4MC_PERMIT;
		if(rtk_acl_igrRuleEntry_get(&aclRule))
		{
			ACL_CTRL("access HW_ACL[%d] failed",aclRule.index);
		}else if(aclRule.valid!=DISABLED){
			ACL_CTRL("Skip ACL Rearrange Protection(MC permit) Start @ acl[0]");
			ACL_RSV("Skip ACL Rearrange Protection(MC permit) Start @ acl[0]");
			return;
		}
	}

	/***set IPv4 Multicast Permit rule***/
	{
		bzero(&aclRule,sizeof(aclRule));
		bzero(&aclField,sizeof(aclField));

		if(rg_db.systemGlobal.aclRearrangeProtectWithAllPermit)
		{
			ACL_CTRL("add ACL PROTECT for ALL permit");
		}
		else
		{
			//setup MC IPv4 DIP 224.0.0.0 to 239.255.255.255
			aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
			aclField.fieldUnion.pattern.fieldIdx = 1; //dipv4[31:16]: template[1] field[1]
			aclField.fieldUnion.data.value = 0xe000;
			aclField.fieldUnion.data.mask = 0xf000;
			if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
			{
				WARNING("setup ACL for MC IPv4 permit failed!!!");
			}
		}

		aclRule.valid=ENABLED;
		aclRule.index=HW_ACL_REARRANGE_PROTECT_V4MC_PERMIT;
		aclRule.templateIdx = 1;//dipv4 in template[1]
		aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
		aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
		aclRule.act.forwardAct.act= ACL_IGR_FORWARD_EGRESSMASK_ACT; //permit action
		aclRule.act.forwardAct.portMask.bits[0]=RTK_RG_ALL_MAC_PORTMASK;
		if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, FAIL))
		{
			WARNING("setup ACL for MC IPv4 permit failed!!!");
		}
	}

	/***set IPv6 Multicast Permit rule***/
	{
		bzero(&aclRule,sizeof(aclRule));
		bzero(&aclField,sizeof(aclField));

		//setup MC IPv6 DIP FFxE:xx..
		aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField.fieldUnion.pattern.fieldIdx = 7; //dipv6[127:112] in template[6]:field[7]
		aclField.fieldUnion.data.value = 0xff0e; //dipv6[127:112]
		aclField.fieldUnion.data.mask = 0xff0f;
		if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
		{
			WARNING("setup ACL for MC IPv6 permit failed!!!");
		}

		aclRule.valid=ENABLED;
		aclRule.index=HW_ACL_REARRANGE_PROTECT_V6MC_PERMIT;
		aclRule.templateIdx = 6;//dipv6 in template[6]
		aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
		aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
		aclRule.act.forwardAct.act= ACL_IGR_FORWARD_EGRESSMASK_ACT; //permit action
		aclRule.act.forwardAct.portMask.bits[0]=RTK_RG_ALL_MAC_PORTMASK;
		if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, FAIL))
		{
			WARNING("setup ACL for MC IPv6 permit failed!!!");
		}
	}

	/***set rest trap rule***/
	{
		bzero(&aclRule,sizeof(aclRule));
		bzero(&aclField,sizeof(aclField));

		aclRule.valid=ENABLED;
		aclRule.index=HW_ACL_REARRANGE_PROTECT_OTHERS_TRAP;
		aclRule.templateIdx=0;
		aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
		aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
		aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
		if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, FAIL))
		{
			WARNING("setup ACL for rest all trap failed!!!");
		}
	}
	ACL_CTRL("ACL Rearrange Protection(MC permit, Others trap) Start @ acl[%d] & acl[%d] & acl[%d]", HW_ACL_REARRANGE_PROTECT_V4MC_PERMIT, HW_ACL_REARRANGE_PROTECT_V6MC_PERMIT, HW_ACL_REARRANGE_PROTECT_OTHERS_TRAP);
	ACL_RSV("ACL Rearrange Protection(MC permit, Others trap) Start @ acl[%d] & acl[%d] & acl[%d]", HW_ACL_REARRANGE_PROTECT_V4MC_PERMIT, HW_ACL_REARRANGE_PROTECT_V6MC_PERMIT, HW_ACL_REARRANGE_PROTECT_OTHERS_TRAP);
#else
	//add multicast trap and addsign VID=1 at ACL[0] when rearrange period.

	rtk_acl_ingress_entry_t aclRule;
	rtk_acl_field_t aclField_dmac0;

	if(rg_db.systemGlobal.flow_flush_disable_by_feature&RTK_RG_FFD_BIT_ACL){
		//only add once when flow flush disable for saving time
		bzero(&aclRule, sizeof(aclRule));
		aclRule.index = 0;
		if(rtk_acl_igrRuleEntry_get(&aclRule))
		{
			ACL_CTRL("access HW_ACL[%d] failed",aclRule.index);
		}else if(aclRule.valid!=DISABLED){
			ACL_CTRL("Skip ACL Rearrange Protection(MC permit) Start @ acl[0]");
			ACL_RSV("Skip ACL Rearrange Protection(MC permit) Start @ acl[0]");
			return;
		}
	}

	//reserved acl[0] for permit multicast packet
	{
		bzero(&aclRule,sizeof(aclRule));
		bzero(&aclField_dmac0,sizeof(aclField_dmac0));

		if(rg_db.systemGlobal.aclRearrangeProtectWithAllPermit)
		{
			ACL_CTRL("add ACL PROTECT for ALL permit");
		}
		else
		{
			//setup MC DMAC care bit 0x01
			ACL_CTRL("add ACL PROTECT for MC temp permit");
			aclField_dmac0.fieldType = ACL_FIELD_PATTERN_MATCH;
			aclField_dmac0.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
			aclField_dmac0.fieldUnion.data.value = 0x100;
			aclField_dmac0.fieldUnion.data.mask = 0x100;
			if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac0))
			{
				WARNING("setup ACL for MC temporary permit failed!!!");
			}
		}

		aclRule.valid=ENABLED;
		aclRule.index=0;
		aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
		aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
		aclRule.act.forwardAct.act= ACL_IGR_FORWARD_EGRESSMASK_ACT;
		aclRule.act.forwardAct.portMask.bits[0]=0xffff;

		if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, FAIL))
		{
			WARNING("setup ACL for MC temporary permit failed!!!");
		}
	}
	ACL_CTRL("ACL Rearrange Protection(MC permit) Start @ acl[0]");
	ACL_RSV("ACL Rearrange Protection(MC permit) Start @ acl[0]");
#endif
}

void _rtk_rg_acl_for_multicast_temp_protection_del(void)
{
#if defined(HW_ACL_REARRANGE_PROTECT_VERSION) && (HW_ACL_REARRANGE_PROTECT_VERSION==2)
	rtk_acl_ingress_entry_t aclRule;
	int i;

	if(rg_db.systemGlobal.flow_flush_disable_by_feature&RTK_RG_FFD_BIT_ACL){
		//not delete when flow flush disable for saving time, delete these rules when recover flow flush
		bzero(&aclRule, sizeof(aclRule));
		aclRule.index = HW_ACL_REARRANGE_PROTECT_V4MC_PERMIT;
		if(rtk_acl_igrRuleEntry_get(&aclRule))
		{
			ACL_CTRL("access HW_ACL[%d] failed",aclRule.index);
		}else if(aclRule.valid!=DISABLED){
			ACL_CTRL("Skip ACL Rearrange Protection(MC permit) Stop @ acl[0]");
			ACL_RSV("Skip ACL Rearrange Protection(MC permit) Stop @ acl[0]");
			return;
		}
	}

	for(i=(HW_ACL_REARRANGE_PROTECT_RSV_HEAD-1); i>=HW_ACL_REARRANGE_PROTECT_V4MC_PERMIT; i--){
		if(rtk_acl_igrRuleEntry_del(i))
		{
			WARNING("setup ACL for MC temporary trap failed!!!");
		}
	}
	ACL_CTRL("ACL Rearrange Protection(MC permit, Others trap) Stop @ acl[%d] & acl[%d] & acl[%d]", HW_ACL_REARRANGE_PROTECT_V4MC_PERMIT, HW_ACL_REARRANGE_PROTECT_V6MC_PERMIT, HW_ACL_REARRANGE_PROTECT_OTHERS_TRAP);
	ACL_RSV("ACL Rearrange Protection(MC permit, Others trap) Stop @ acl[%d] & acl[%d] & acl[%d]", HW_ACL_REARRANGE_PROTECT_V4MC_PERMIT, HW_ACL_REARRANGE_PROTECT_V6MC_PERMIT, HW_ACL_REARRANGE_PROTECT_OTHERS_TRAP);
#else
	//add multicast trap and addsign VID=1 at ACL[0] when rearrange period.

	rtk_acl_ingress_entry_t aclRule;

	if(rg_db.systemGlobal.flow_flush_disable_by_feature&RTK_RG_FFD_BIT_ACL){
		//not delete when flow flush disable for saving time, delete these rules when recover flow flush
		bzero(&aclRule, sizeof(aclRule));
		aclRule.index = HW_ACL_REARRANGE_PROTECT_V4MC_PERMIT;
		if(rtk_acl_igrRuleEntry_get(&aclRule))
		{
			ACL_CTRL("access HW_ACL[%d] failed",aclRule.index);
		}else if(aclRule.valid!=DISABLED){
			ACL_CTRL("Skip ACL Rearrange Protection(MC permit) Stop @ acl[0]");
			ACL_RSV("Skip ACL Rearrange Protection(MC permit) Stop @ acl[0]");
			return;
		}
	}

	//remove reserved acl[0] for permit multicast
	{
		if(rtk_acl_igrRuleEntry_del(0))
		{
			WARNING("setup ACL for MC temporary trap failed!!!");
		}
	}
	ACL_CTRL("ACL Rearrange Protection(MC permit) Stop @ acl[0]");
	ACL_RSV("ACL Rearrange Protection(MC permit) Stop @ acl[0]");
#endif
}


int _rtk_rg_acl_user_part_rearrange(void)
{
	int ret;

	//using ACL[0] for multicast
	_rtk_rg_acl_for_multicast_temp_protection_add();

	ret = _rtk_rg_aclSWEntry_and_asic_rearrange(FAIL);
	if(ret!=RT_ERR_RG_OK) WARNING("_rtk_rg_aclSWEntry_and_asic_rearrange failed!");

	_rtk_rg_acl_for_multicast_temp_protection_del();

	return ret;
}


int _rtk_rg_aclSWEntry_and_asic_rearrange(int acl_filter_idx)
{
	int i,j,accumulateIdx,ruleIdx;
	rtk_rg_aclFilterAndQos_t *acl_filter;
	int rearrange_hw_acl_all = TRUE;
	int acl_filter_weight = 0;
	ACL_PTOOL_VERIFY_START;



	bzero(&rg_db.systemGlobal.aclSWEntry_rearrange, sizeof(rtk_rg_aclFilterEntry_t));
	bzero(rg_db.systemGlobal.acl_filter_temp, sizeof(rtk_rg_aclFilterAndQos_t)*MAX_ACL_SW_ENTRY_SIZE);
	bzero(rg_db.systemGlobal.acl_filter_temp_valid, sizeof(rg_db.systemGlobal.acl_filter_temp_valid));

	if(acl_filter_idx != FAIL)
	{
		bzero(&rg_db.systemGlobal.aclSWEntry,sizeof(rtk_rg_aclFilterEntry_t));
		ASSERT_EQ(_rtk_rg_aclSWEntry_get(acl_filter_idx,&rg_db.systemGlobal.aclSWEntry),RT_ERR_RG_OK);
		if(rg_db.systemGlobal.aclSWEntry.valid != RTK_RG_ENABLED){
			rearrange_hw_acl_all = TRUE;
			ACL_CTRL("================[ACL[%d] rearrange scope: all (invalid acl idx=%d)]==================", acl_filter_idx, acl_filter_idx);			
		}else if(rg_db.systemGlobal.stop_add_hw_acl){
			rearrange_hw_acl_all = TRUE;
			ACL_CTRL("================[ACL[%d] rearrange scope: all (stop hw acl idx=%d)]==================", acl_filter_idx, (rg_db.systemGlobal.stop_add_hw_acl-1));			
		}else if(rg_db.systemGlobal.aclSWEntry.hw_used_table){
			rearrange_hw_acl_all = TRUE;
			ACL_CTRL("================[ACL[%d] rearrange scope: all (hw used tbl=0x%x)]==================", acl_filter_idx, rg_db.systemGlobal.aclSWEntry.hw_used_table);
		}else{
			rearrange_hw_acl_all = FALSE;
			acl_filter_weight = rg_db.systemGlobal.aclSWEntry.acl_filter.acl_weight;
			ACL_CTRL("================[ACL[%d] rearrange scope: priority lower than %d ]==================", acl_filter_idx, acl_filter_weight);
		}		
	}

forceHWRearrange:
	//Clear HW_ACL user used table
	if(rearrange_hw_acl_all){
		ASSERT_EQ(_rtk_rg_reset_asic_TableEntry(TRUE, MIN_ACL_ENTRY_INDEX, (MAX_ACL_ENTRY_INDEX-MIN_ACL_ENTRY_INDEX+1)),RT_ERR_RG_OK);

		//clear the limitation for add ACL rule to HW.
		rg_db.systemGlobal.stop_add_hw_acl = 0;
	}

	//clear the information of rearrange status
	rg_db.systemGlobal.stop_add_acl = FALSE;


	ACL_CTRL("================[do ACL rearrange]==================");
	//backup all acl_filter for reAdd, and clean aclSWEntry/HW_ACL if necessary
	for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
	{
		if(rg_db.systemGlobal.acl_SW_table_entry[i].valid!=RTK_RG_ENABLED)
			continue;
		//backup acl_filter
		ASSERT_EQ(_rtk_rg_aclSWEntry_get(i, &rg_db.systemGlobal.aclSWEntry_rearrange),RT_ERR_RG_OK);
		rg_db.systemGlobal.acl_filter_temp[i] = rg_db.systemGlobal.aclSWEntry_rearrange.acl_filter;
		rg_db.systemGlobal.acl_filter_temp_valid[(i>>5)] |= (1<<(i&0x1f));
		
		if(rearrange_hw_acl_all || (rg_db.systemGlobal.acl_filter_temp[i].acl_weight < acl_filter_weight) || (i == acl_filter_idx) ||
								((rg_db.systemGlobal.acl_filter_temp[i].acl_weight == acl_filter_weight) && (i > acl_filter_idx))){
			if(!rearrange_hw_acl_all && rg_db.systemGlobal.aclSWEntry_rearrange.hw_used_table){
				rearrange_hw_acl_all = TRUE;
				ACL_CTRL("================[ACL[%d] rearrange scope: all (hw used tbl=0x%x)]==================", i, rg_db.systemGlobal.aclSWEntry_rearrange.hw_used_table);
				goto forceHWRearrange;
			}
			//clean aclSWEntry hw information
			if(!rearrange_hw_acl_all && rg_db.systemGlobal.aclSWEntry_rearrange.hw_aclEntry_size)
				ASSERT_EQ(_rtk_rg_reset_asic_TableEntry(FALSE, rg_db.systemGlobal.aclSWEntry_rearrange.hw_aclEntry_start, rg_db.systemGlobal.aclSWEntry_rearrange.hw_aclEntry_size),RT_ERR_RG_OK);
			bzero(&rg_db.systemGlobal.aclSWEntry_rearrange, ((POINTER_CAST)(&rg_db.systemGlobal.aclSWEntry_rearrange.COUNT_HWACL_LENGTH_FIELD)-(POINTER_CAST)&rg_db.systemGlobal.aclSWEntry_rearrange));
			ASSERT_EQ(_rtk_rg_aclSWEntry_set(i, rg_db.systemGlobal.aclSWEntry_rearrange),RT_ERR_RG_OK);
		}
	}

	//reAdd all reserve aclSWEntry:  type priority as following
		//1. ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET (with acl_weight sorting)
		//2. ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP or ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP
		//3. ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID or ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN
	ASSERT_EQ(_rtk_rg_rearrange_ACL_weight(&accumulateIdx),RT_ERR_RG_OK);


	//1. readd the rules of type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET and reset the asic entry with lower priority
	for(j=0;j<MAX_ACL_SW_ENTRY_SIZE;j++){
		if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]==-1)
			break;
		else if(!rearrange_hw_acl_all && (rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j] != acl_filter_idx))
			continue;
		else if(!rearrange_hw_acl_all && (rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j] == acl_filter_idx))
			rearrange_hw_acl_all = TRUE;
		
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

	ACL_PTOOL_VERIFY_END;
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
	ACL_PTOOL_VERIFY_START;

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
			if(replace_start && (i==(MAX_ACL_SW_ENTRY_SIZE-2))) //prevent Overrunning array
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

	ACL_PTOOL_VERIFY_END;
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

	if(acl_filter->filter_fields & INGRESS_TCP_FLAGS_BIT)
	{
 		if(acl_filter->ingress_tcp_flags_mask==0x0){
			acl_filter->ingress_tcp_flags_mask = 0xfff;
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
	ACL_PTOOL_VERIFY_START;
	
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
		if((acl_filter->action_type==ACL_ACTION_TYPE_TRAP_WITH_PRIORITY)||(acl_filter->action_type==ACL_ACTION_TYPE_TRAP2SLAVE_WITH_PRIORITY))
		{
			//Only work on HW-ACL, so skip byPassAddHwAclCheck
			if((acl_filter->filter_fields & ~(HW_ACL_SUPPORT_PATTERN_9607C)) || (acl_filter->filter_fields_inverse!=0x0))
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

		//need sub function to add to H/W ACL, and a clear API to call first.
		ret = _rtk_rg_aclSWEntry_to_asic_add(acl_filter,&rg_db.systemGlobal.aclSWEntry,FAIL);
		if((ret == RT_ERR_RG_ACL_ENTRY_FULL) || (ret == RT_ERR_RG_ACL_IPTABLE_FULL) || (ret == RT_ERR_RG_ACL_PORTTABLE_FULL))
		{
			bzero(&rg_db.systemGlobal.aclSWEntry, ((POINTER_CAST)(&rg_db.systemGlobal.aclSWEntry.COUNT_HWACL_LENGTH_FIELD)-(POINTER_CAST)&rg_db.systemGlobal.aclSWEntry));
			goto byPassAddHwAclCheck;
		}
		if(ret != RT_ERR_RG_OK)
		{
			goto aclFailed;
		}
	}
	else if(rg_db.systemGlobal.stop_add_hw_acl>0 && ((acl_filter->action_type==ACL_ACTION_TYPE_TRAP_WITH_PRIORITY)||(acl_filter->action_type==ACL_ACTION_TYPE_TRAP2SLAVE_WITH_PRIORITY)))
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
		ret = _rtk_rg_aclSWEntry_and_asic_rearrange(*acl_filter_idx);
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

	ACL_PTOOL_VERIFY_END;
	return (RT_ERR_RG_OK);

aclFailed:

	if(rg_db.systemGlobal.acl_rg_add_parameter_dump){
		rtlglue_printf("add to RG ACL Faild! ret=0x%x\n",ret);
	}

	_rtk_rg_acl_for_multicast_temp_protection_del();

	if(rg_db.systemGlobal.stop_add_acl) {
		ASSERT_EQ(rtk_rg_apollo_aclFilterAndQos_del(*acl_filter_idx),RT_ERR_RG_OK);
	}

	ACL_PTOOL_VERIFY_END;
	return ret;

}




int32 _rtk_rg_apollo_aclFilterAndQos_del(int acl_filter_idx)
{
	int i,ret=RT_ERR_RG_FAILED;
	int update_hw_acl = FALSE;
	int acl_filter_idx_delete = FAIL;
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
	if(rg_db.systemGlobal.aclSWEntry.hw_aclEntry_size || (rg_db.systemGlobal.stop_add_hw_acl && ((rg_db.systemGlobal.stop_add_hw_acl-1) == acl_filter_idx))){
		update_hw_acl = TRUE;
		if(rg_db.systemGlobal.aclSWEntry.hw_used_table || rg_db.systemGlobal.stop_add_hw_acl){
			acl_filter_idx_delete = FAIL;
		}else{
			for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++){
				if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i]==-1)//no more rules need to check
					break;
				else if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i] != acl_filter_idx)
					continue;		
				acl_filter_idx_delete = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i-1];
				break;
			}
		}
		if((acl_filter_idx_delete != FAIL) && rg_db.systemGlobal.aclSWEntry.hw_aclEntry_size)
			ASSERT_EQ(_rtk_rg_reset_asic_TableEntry(FALSE, rg_db.systemGlobal.aclSWEntry.hw_aclEntry_start, rg_db.systemGlobal.aclSWEntry.hw_aclEntry_size),RT_ERR_RG_OK);
	}
	bzero(&rg_db.systemGlobal.aclSWEntry,sizeof(rtk_rg_aclFilterEntry_t));

	//clean aclSWEntry
	ret = _rtk_rg_aclSWEntry_set(acl_filter_idx, rg_db.systemGlobal.aclSWEntry);
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	//rearrange the ACL & CF ASIC to avoid discontinuous entry
	if(update_hw_acl)
		ret = _rtk_rg_aclSWEntry_and_asic_rearrange(acl_filter_idx_delete);
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

	if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0)){
		//FB testchip not support patterns (even flow is different, it will be hash into the same way)
		if(classifyFilter->filter_fields&EGRESS_TAGPRI_BIT)
		{
			WARNING("apolloPro testchip not support pattern EGRESS_TAGPRI_BIT, due to FB will hash in the same way.");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}


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

	if(classifyFilter->filter_fields & EGRESS_GEMIDX_BIT)
	{
		if(classifyFilter->direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM)
		{
			WARNING("CF pattern EGRESS_GEMIDX_BIT not support upstream.");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
		else if(classifyFilter->gemidx_mask == SW_CF_GEMIDX_MASK)
		{	//flow based platform all cf is sw only
			//WARNING("CF pattern EGRESS_GEMIDX_BIT mask is 0x%x for SW CF only", classifyFilter->gemidx_mask);
		}
		else if((classifyFilter->gemidx > MAX_CF_GEMIDX_VALUE) || (classifyFilter->gemidx_mask > MAX_CF_GEMIDX_VALUE))
		{
			WARNING("CF pattern EGRESS_GEMIDX_BIT max value is %d.", MAX_CF_GEMIDX_VALUE);
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}
	if(classifyFilter->filter_fields & EGRESS_LLID_BIT)
	{
		if(classifyFilter->direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM)
		{
			WARNING("CF pattern EGRESS_LLID_BIT not support upstream.");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
		else if(classifyFilter->llid > MAX_CF_LLID_VALUE)
		{
			WARNING("CF pattern EGRESS_LLID_BIT max value is %d.", MAX_CF_LLID_VALUE);
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
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
			classifyFilter->gemidx_mask=MAX_CF_GEMIDX_VALUE;
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

#if !defined(CONFIG_RG_G3_SERIES)	// CONFIG_RG_G3_SERIES_DEVELOPMENT
//G3 implement rsvACL in file rtk_rg_acl_g3.c / rtk_rg_acl_g3.h

/*(4)reserved ACL related APIs*/
int _rtk_rg_aclReservedEntry_init(void)
{
	rg_db.systemGlobal.aclAndCfReservedRule.acl_MC_temp_permit_idx = 0;// This value is assgined by reserved ACL, just for temp initial now.

	rg_db.systemGlobal.aclAndCfReservedRule.aclLowerBoundary = 0;
	rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary = (MAX_ACL_ENTRY_SIZE-1);

	return (RT_ERR_RG_OK);
}

static int _rtk_rg_aclAndCfReservedRuleAddCheck(uint32 aclRsvSize, uint32 cfRsvSize, rtk_rg_aclAndCf_reserved_type_t rsvType)
{
#if 0		//[Care Asic ACL Full]
	uint32 i;
	rtk_acl_ingress_entry_t aclRule;
	//check rest empty acl rules in enough
	for(i=rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary;i>(rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary-aclRsvSize);i--){
		aclRule.index=i;
		assert_ok(rtk_acl_igrRuleEntry_get(&aclRule));
		if(aclRule.valid!=DISABLED){
			//flow base platform, when asic full, user acl can be S/W only, and rest user acl stop add to asic
			ACL_RSV("ACL rest rules for reserved[%d] is not enough!Some user acl will become S/W only. (i=%d aclUpperBoundary=%d aclRsvSize=%d)",rsvType,i,rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary,aclRsvSize);
			//WARNING("ACL rest rules for reserved[%d] is not enough!Some user acl will become S/W only. (i=%d aclUpperBoundary=%d aclRsvSize=%d)",rsvType,i,rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary,aclRsvSize);
			//return (RT_ERR_RG_FAILED);
		}
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

	_rtk_rg_aclAndCfReservedRuleHeadReflash();
	_rtk_rg_aclAndCfReservedRuleTailReflash();

	//[Care Asic ACL Full] user acl can be S/W only, so reflash after reserve acl update finish
	assert_ok(_rtk_rg_aclSWEntry_and_asic_rearrange(FAIL));

	_rtk_rg_acl_for_multicast_temp_protection_del();

	return RT_ERR_RG_OK;
}

static int _rtk_rg_aclAndCfReservedRuleHeadReflash(void)
{
	uint32 type;
#if defined(HW_ACL_REARRANGE_PROTECT_VERSION) && (HW_ACL_REARRANGE_PROTECT_VERSION==2)
	uint32 aclIdx=HW_ACL_REARRANGE_PROTECT_RSV_HEAD; //ACL[0],ACL[1],ACL[2] reserved for _rtk_rg_acl_for_multicast_temp_protection_add()
#else
	uint32 aclIdx=1; //ACL[0] reserved for _rtk_rg_acl_for_multicast_temp_protection_add()
#endif
	int addRuleFailedFlag=0;

	rtk_acl_ingress_entry_t aclRule;
	rtk_acl_field_t aclField,aclField2;
	rtk_acl_field_t	aclField_dmac0,aclField_dmac1,aclField_dmac2;
	rtk_classify_field_t classifyField;
	uint32 i, aclValue0;
	rtk_mac_t mac;
	uint16 pktLenEntryIdx;
	uint32 cfIdx=0;
	rtk_classify_cfg_t cfRule;
	rtk_classify_ds_act_t dsAct;
	uint8 aclreverseIdx=0;	//Multiple HIT should add all pattern first and then add action(reverse add)

	//reflash th rules
	for(i=aclIdx;i<rg_db.systemGlobal.aclAndCfReservedRule.aclLowerBoundary;i++){
		assert_ok(rtk_acl_igrRuleEntry_del(i));
	}
	for(i=0;i<rg_db.systemGlobal.aclAndCfReservedRule.cfLowerBoundary;i++){
		assert_ok(rtk_classify_cfgEntry_del(i));
	}
	//delete ACL Packet Length range table
	for(i=0; i<USER_ACL_PKTLENRANGETABLE_START; i++){
		assert_ok(_rtk_rg_free_acl_pktLenTableEntry(i));
	}

	for(type=0;type<RTK_RG_ACLANDCF_RESERVED_HEAD_END;type++){
		if(rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[type]==ENABLED){
			switch(type){
				case RTK_RG_ACLANDCF_RESERVED_STPBLOCKING:
					{
						ACL_RSV("add RSV[%d] STPBLOCKING @ acl[%d] & acl[%d]",type,aclIdx,aclIdx+1);
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField_dmac1,sizeof(aclField_dmac1));
						bzero(&aclField_dmac2,sizeof(aclField_dmac2));

						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField_dmac2.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField_dmac2.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[0]
						aclField_dmac2.fieldUnion.data.value = 0x0180;
						aclField_dmac2.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac2))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_STPBLOCKING failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						aclField_dmac1.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField_dmac1.fieldUnion.pattern.fieldIdx = 1; //DA[31:16]: template[0] field[1]
						aclField_dmac1.fieldUnion.data.value = 0xc200;
						aclField_dmac1.fieldUnion.data.mask = 0xff00;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac1))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_STPBLOCKING failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						aclRule.valid = ENABLE;
						aclRule.index = aclIdx;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;

						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_STPBLOCKING failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						//point to next ruleIdx
						aclIdx++;

						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid = ENABLE;
						aclRule.index = aclIdx;
						aclRule.activePorts.bits[0]=rg_db.systemGlobal.stpBlockingPortmask.portmask;
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
						aclRule.act.forwardAct.act= ACL_IGR_FORWARD_EGRESSMASK_ACT;
						aclRule.act.forwardAct.portMask.bits[0]= 0x0;	//drop
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_STPBLOCKING failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;
					}
					break;


				case RTK_RG_ACLANDCF_RESERVED_ALL_TRAP:
					{
						ACL_RSV("add RSV[%d] ALL_TRAP @ acl[%d]",type,aclIdx);
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;

						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ALL_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						//point to next ruleIdx
						aclIdx++;

						//keep this index in rg_db. while user ACL rearrange this index will be used.
						//rg_db.systemGlobal.aclAndCfReservedRule.acl_MC_temp_permit_idx = aclIdx;
						//point to next ruleIdx
						//aclIdx++;
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_UNICAST_TRAP:
					{
						ACL_RSV("add RSV[%d] UNICAST_TRAP @ acl[%d]",type,aclIdx);
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField_dmac0,sizeof(aclField_dmac0));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;

						aclField_dmac0.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField_dmac0.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
						aclField_dmac0.fieldUnion.data.value = 0x0000; //unicast mac byte[0],bit[0]==0
						aclField_dmac0.fieldUnion.data.mask = 0x0100;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac0))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UNICAST_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}


						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UNICAST_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						//point to next ruleIdx
						aclIdx++;

					}
					break;


				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP:
					{
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						bzero(&aclField2,sizeof(aclField2));

						//trap 239.255.255.250
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 0; //DIP[15:0]
						aclField.fieldUnion.data.value = 0xfffa;
						aclField.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField2.fieldUnion.pattern.fieldIdx = 1; //DIP[31:16]
						aclField2.fieldUnion.data.value = 0xefff;
						aclField2.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField2))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.templateIdx=1;
						if(rg_db.systemGlobal.lanPortMask.portmask != 0x0)
							aclRule.activePorts.bits[0]=(rg_db.systemGlobal.lanPortMask.portmask & RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU);
						else
							aclRule.activePorts.bits[0]=RTK_RG_ALL_LAN_PORTMASK;
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						ACL_RSV("add RSV[%d] MULTICAST_SSDP_TRAP [activePorts=lanPortMask(0x%x)&ALL_MAC_WITHOUT_CPU(0x%x)] @ acl[%d]",type,rg_db.systemGlobal.lanPortMask.portmask,RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU,aclIdx);

						//point to next ruleIdx
						aclIdx++;
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_DOT1X_EAPOL_TRAP:
					{
#if defined(CONFIG_RG_8021X_MAC_TABLE_SIZE) && (CONFIG_RG_8021X_MAC_TABLE_SIZE!=0)
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						
						if(rg_db.systemGlobal.accessDot1xCfg.blockingPortmask.portmask == 0x0)
						{
							ACL_RSV("skip add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_DOT1X_EAPOL_TRAP due to blocking port is 0!!!");
							break;
						}

						//trap ethertype 0x888e
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 5;/*template[1],field[5]*/
						aclField.fieldUnion.data.value = RG_DOT1X_EAPOL_ETHERTYPE; //ethertype=0x888E
						aclField.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_DOT1X_EAPOL_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.templateIdx=1;
						aclRule.activePorts.bits[0]=rg_db.systemGlobal.accessDot1xCfg.blockingPortmask.portmask;
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
						aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
						aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
						aclRule.act.priAct.aclPri=7;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_DOT1X_EAPOL_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						ACL_RSV("add RSV[%d] DOT1X_EAPOL_TRAP [blockingPortMask=0x%x] @ acl[%d]",type,rg_db.systemGlobal.accessDot1xCfg.blockingPortmask,aclIdx);

						//point to next ruleIdx
						aclIdx++;
#endif
					}
					break;
					
				case RTK_RG_ACLANDCF_RESERVED_INTF0_IPV4_FRAGMENT_TRAP:
					ACL_RSV("add RSV[%d] INTF0_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",type,aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[0].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF1_IPV4_FRAGMENT_TRAP:
					ACL_RSV("add RSV[%d] INTF1_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",type,aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[1].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF2_IPV4_FRAGMENT_TRAP:
					ACL_RSV("add RSV[%d] INTF2_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",type,aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[2].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF3_IPV4_FRAGMENT_TRAP:
					ACL_RSV("add RSV[%d] INTF3_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",type,aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[3].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF4_IPV4_FRAGMENT_TRAP:
					ACL_RSV("add RSV[%d] INTF4_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",type,aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[4].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF5_IPV4_FRAGMENT_TRAP:
					ACL_RSV("add RSV[%d] INTF5_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",type,aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[5].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF6_IPV4_FRAGMENT_TRAP:
					ACL_RSV("add RSV[%d] INTF6_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",type,aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[6].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF7_IPV4_FRAGMENT_TRAP:
					ACL_RSV("add RSV[%d] INTF7_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",type,aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[7].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF8_IPV4_FRAGMENT_TRAP:
					ACL_RSV("add RSV[%d] INTF8_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",type,aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[8].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF9_IPV4_FRAGMENT_TRAP:
					ACL_RSV("add RSV[%d] INTF9_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",type,aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[9].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF10_IPV4_FRAGMENT_TRAP:
					ACL_RSV("add RSV[%d] INTF10_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",type,aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[10].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF11_IPV4_FRAGMENT_TRAP:
					ACL_RSV("add RSV[%d] INTF11_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",type,aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[11].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF12_IPV4_FRAGMENT_TRAP:
					ACL_RSV("add RSV[%d] INTF12_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",type,aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[12].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF13_IPV4_FRAGMENT_TRAP:
					ACL_RSV("add RSV[%d] INTF13_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",type,aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[13].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF14_IPV4_FRAGMENT_TRAP:
					ACL_RSV("add RSV[%d] INTF14_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",type,aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[14].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF15_IPV4_FRAGMENT_TRAP:
					ACL_RSV("add RSV[%d] INTF15_IPV4_FRAGMENT_TRAP @ acl[%d] & acl[%d] & acl[%d]",type,aclIdx,aclIdx+1,aclIdx+2);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_ipv4_trap[15].gmac.octet[0],ETHER_ADDR_LEN);
					goto fragmentTrapRule;

fragmentTrapRule:
					{
						/* Multiple HIT should add all pattern first and then add action */
						aclreverseIdx = aclIdx+2;

						/*start to set ACL rule*/
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField_dmac0,sizeof(aclField_dmac0));
						bzero(&aclField_dmac1,sizeof(aclField_dmac1));
						bzero(&aclField_dmac2,sizeof(aclField_dmac2));
						aclRule.valid=ENABLED;
						aclRule.index=aclreverseIdx;
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

						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFn_IPV4_FRAGMENT_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclreverseIdx--;
						aclIdx++;




						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid=ENABLED;
						aclRule.index=aclreverseIdx;
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

						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFn_IPV4_FRAGMENT_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclreverseIdx--;
						aclIdx++;



						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid=ENABLED;
						aclRule.index=aclreverseIdx;
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

						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFn_IPV4_FRAGMENT_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;


					}
					break;




				case RTK_RG_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY:
					{
						ACL_RSV("add RSV[%d] L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY (wanPortMask=0x%x)   @ acl[%d]",type,rg_db.systemGlobal.wanPortMask.portmask,aclIdx);

						//Set ACL rule for higher rx prioirty for L2TP controal packet and LCP packet(L2TP, none-ipv4, none-ipv6)
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));

						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						if(rg_db.systemGlobal.wanPortMask.portmask != 0x0)
							aclRule.activePorts.bits[0]=rg_db.systemGlobal.wanPortMask.portmask;
						else
							aclRule.activePorts.bits[0]=(1<<RTK_RG_PORT_PON);
						aclRule.templateIdx=2; /*use :tagIf=> template[2],field[5]*/


						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 5;/*template[2],field[5]*/
						aclField.fieldUnion.data.value = (ACL_TAGIF_L2TP_BIT & (~ACL_TAGIF_IN_IP4_BIT) & (~ACL_TAGIF_IP6_BIT));
						aclField.fieldUnion.data.mask = (ACL_TAGIF_L2TP_BIT | ACL_TAGIF_IN_IP4_BIT | ACL_TAGIF_IP6_BIT);
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//force trap to CPU with priroity to avoid FB trap priroity replaced.
						aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
						aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
						aclRule.act.priAct.aclPri=rg_db.systemGlobal.aclAndCfReservedRule.l2tp_ctrl_lcp_assign_prioity.priority;
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;

					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE0_TRAP:
					ACL_RSV("add RSV[%d] MULTICAST_PPPOE_RULE0_TRAP (wanPortMask=0x%x) @ acl[%d] & acl[%d]",type,rg_db.systemGlobal.wanPortMask.portmask,aclIdx,aclIdx+1);
					memcpy(mac.octet,rg_db.systemGlobal.aclAndCfReservedRule.mc_pppoe_trap[0].dmac.octet,ETHER_ADDR_LEN);
					aclValue0 = rg_db.systemGlobal.aclAndCfReservedRule.mc_pppoe_trap[0].dip;
					goto mcPppoeTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE1_TRAP:
					ACL_RSV("add RSV[%d] MULTICAST_PPPOE_RULE1_TRAP (wanPortMask=0x%x) @ acl[%d] & acl[%d]",type,rg_db.systemGlobal.wanPortMask.portmask,aclIdx,aclIdx+1);
					memcpy(mac.octet,rg_db.systemGlobal.aclAndCfReservedRule.mc_pppoe_trap[1].dmac.octet,ETHER_ADDR_LEN);
					aclValue0 = rg_db.systemGlobal.aclAndCfReservedRule.mc_pppoe_trap[1].dip;
					goto mcPppoeTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE2_TRAP:
					ACL_RSV("add RSV[%d] MULTICAST_PPPOE_RULE2_TRAP (wanPortMask=0x%x) @ acl[%d] & acl[%d]",type,rg_db.systemGlobal.wanPortMask.portmask,aclIdx,aclIdx+1);
					memcpy(mac.octet,rg_db.systemGlobal.aclAndCfReservedRule.mc_pppoe_trap[2].dmac.octet,ETHER_ADDR_LEN);
					aclValue0 = rg_db.systemGlobal.aclAndCfReservedRule.mc_pppoe_trap[2].dip;
					goto mcPppoeTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE3_TRAP:
					ACL_RSV("add RSV[%d] MULTICAST_PPPOE_RULE3_TRAP (wanPortMask=0x%x) @ acl[%d] & acl[%d]",type,rg_db.systemGlobal.wanPortMask.portmask,aclIdx,aclIdx+1);
					memcpy(mac.octet,rg_db.systemGlobal.aclAndCfReservedRule.mc_pppoe_trap[3].dmac.octet,ETHER_ADDR_LEN);
					aclValue0 = rg_db.systemGlobal.aclAndCfReservedRule.mc_pppoe_trap[3].dip;
					goto mcPppoeTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE4_TRAP:
					ACL_RSV("add RSV[%d] MULTICAST_PPPOE_RULE4_TRAP (wanPortMask=0x%x) @ acl[%d] & acl[%d]",type,rg_db.systemGlobal.wanPortMask.portmask,aclIdx,aclIdx+1);
					memcpy(mac.octet,rg_db.systemGlobal.aclAndCfReservedRule.mc_pppoe_trap[4].dmac.octet,ETHER_ADDR_LEN);
					aclValue0 = rg_db.systemGlobal.aclAndCfReservedRule.mc_pppoe_trap[4].dip;
					goto mcPppoeTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE5_TRAP:
					ACL_RSV("add RSV[%d] MULTICAST_PPPOE_RULE5_TRAP (wanPortMask=0x%x) @ acl[%d] & acl[%d]",type,rg_db.systemGlobal.wanPortMask.portmask,aclIdx,aclIdx+1);
					memcpy(mac.octet,rg_db.systemGlobal.aclAndCfReservedRule.mc_pppoe_trap[5].dmac.octet,ETHER_ADDR_LEN);
					aclValue0 = rg_db.systemGlobal.aclAndCfReservedRule.mc_pppoe_trap[5].dip;
					goto mcPppoeTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE6_TRAP:
					ACL_RSV("add RSV[%d] MULTICAST_PPPOE_RULE6_TRAP (wanPortMask=0x%x) @ acl[%d] & acl[%d]",type,rg_db.systemGlobal.wanPortMask.portmask,aclIdx,aclIdx+1);
					memcpy(mac.octet,rg_db.systemGlobal.aclAndCfReservedRule.mc_pppoe_trap[6].dmac.octet,ETHER_ADDR_LEN);
					aclValue0 = rg_db.systemGlobal.aclAndCfReservedRule.mc_pppoe_trap[6].dip;
					goto mcPppoeTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE7_TRAP:
					ACL_RSV("add RSV[%d] MULTICAST_PPPOE_RULE7_TRAP (wanPortMask=0x%x) @ acl[%d] & acl[%d]",type,rg_db.systemGlobal.wanPortMask.portmask,aclIdx,aclIdx+1);
					memcpy(mac.octet,rg_db.systemGlobal.aclAndCfReservedRule.mc_pppoe_trap[7].dmac.octet,ETHER_ADDR_LEN);
					aclValue0 = rg_db.systemGlobal.aclAndCfReservedRule.mc_pppoe_trap[7].dip;
					goto mcPppoeTrapRule;

mcPppoeTrapRule:
					{
						/*start to set ACL rule*/
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField_dmac0,sizeof(aclField_dmac0));
						bzero(&aclField_dmac1,sizeof(aclField_dmac1));
						bzero(&aclField_dmac2,sizeof(aclField_dmac2));
						aclRule.valid=ENABLED;
						aclRule.index = aclIdx;
						aclRule.activePorts.bits[0]=rg_db.systemGlobal.wanPortMask.portmask;
						aclRule.templateIdx=0; //dmac

						//setup gmac
						aclField_dmac0.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField_dmac0.fieldUnion.pattern.fieldIdx = 0; //DA[15:0]: template[0] field[0]
						aclField_dmac0.fieldUnion.data.value = (mac.octet[4]<<8) | (mac.octet[5]);
						aclField_dmac0.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac0)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULEn_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclField_dmac1.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField_dmac1.fieldUnion.pattern.fieldIdx = 1; //DA[31:16]: template[0] field[1]
						aclField_dmac1.fieldUnion.data.value = (mac.octet[2]<<8) | (mac.octet[3]);
						aclField_dmac1.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac1)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULEn_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclField_dmac2.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField_dmac2.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
						aclField_dmac2.fieldUnion.data.value = (mac.octet[0]<<8) | (mac.octet[1]);
						aclField_dmac2.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac2)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULEn_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}


						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULEn_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;

						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						bzero(&aclField2,sizeof(aclField2));
						aclRule.valid=ENABLED;
						aclRule.index = aclIdx;
						aclRule.activePorts.bits[0]=rg_db.systemGlobal.wanPortMask.portmask;
						aclRule.templateIdx=1; //dip

						//setup DIP
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 1; //DIP[0:15]
						aclField.fieldUnion.data.value = ((aclValue0&0xffff0000)>>16);
						aclField.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULEn_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField2.fieldUnion.pattern.fieldIdx = 0; //DIP[16:31]
						aclField2.fieldUnion.data.value = (aclValue0&0xffff);
						aclField2.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField2)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULEn_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULEn_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY:
					{
						ACL_RSV("add RSV[%d] PPPoE_LCP_PACKET_ASSIGN_PRIORITY (wanPortMask=0x%x) @ acl[%d]",type,rg_db.systemGlobal.wanPortMask.portmask,aclIdx);

						//Set ACL rule for higher rx prioirty for PPPoE LCP packet(eth=0x8864, none-ipv4, none-ipv6)
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));

						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						if(rg_db.systemGlobal.wanPortMask.portmask != 0x0)
							aclRule.activePorts.bits[0]=rg_db.systemGlobal.wanPortMask.portmask;
						else
							aclRule.activePorts.bits[0]=(1<<RTK_RG_PORT_PON);
						aclRule.templateIdx=2; /*use :tagIf=> template[2],field[5]*/


						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 5;/*template[2],field[5]*/
						aclField.fieldUnion.data.value = ACL_TAGIF_PPPoE_8864_BIT;
						aclField.fieldUnion.data.mask = (ACL_TAGIF_PPPoE_8864_BIT | ACL_TAGIF_OUT_IP4_BIT | ACL_TAGIF_IP6_BIT);
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//force trap to CPU with priroity to avoid FB trap priroity replaced.
						aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
						aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
						aclRule.act.priAct.aclPri=rg_db.systemGlobal.aclAndCfReservedRule.pppoe_lcp_assign_prioity.priority;
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;

					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY:
					{
						ACL_RSV("add RSV[%d] ACK_PACKET_ASSIGN_PRIORITY @ acl[%d] & acl[%d]",type,aclIdx,aclIdx+1);
						//If enable this ACL, flow will disable priority bit to prevent acl priority replace by flow. However, GPON mode for stream id remapping still have problem.

						/* Multiple HIT should add all pattern first and then add action */
						aclreverseIdx = aclIdx+1;

						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));

						aclRule.valid=ENABLED;
						aclRule.index=aclreverseIdx;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK;
						aclRule.templateIdx=3; /*use :FS[2] => template[3],field[2]*/

						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 2;/*template[3],field[2]*/
						aclField.fieldUnion.data.value = 0x10; //ACK flag
						aclField.fieldUnion.data.mask = 0x10;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclreverseIdx--;
						aclIdx++;

						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));

						aclRule.valid=ENABLED;
						aclRule.index=aclreverseIdx;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK;
						aclRule.templateIdx=2; /*pktLenRange => template[2],field[7]*/

						//limit packet size is samller than 72
						pktLenEntryIdx = 0;
						if(_rtk_rg_search_acl_pktLenRangeTableEntry(&pktLenEntryIdx, TRUE, 72, 0)!=RT_ERR_RG_OK)
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 7;/*template[2],field[7]*/
						aclField.fieldUnion.data.value = 0x1<<pktLenEntryIdx; //pktLenRange index
						aclField.fieldUnion.data.mask = 0x1<<pktLenEntryIdx;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//check TCP tagIf
						bzero(&aclField2,sizeof(aclField2));
						aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField2.fieldUnion.pattern.fieldIdx = 5;/*template[2],field[5]*/
						aclField2.fieldUnion.data.value = ACL_TAGIF_TCP_BIT; //TCP tagIf
						aclField2.fieldUnion.data.mask = ACL_TAGIF_TCP_BIT;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField2)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//force fwd to CPU port (avoid egress vlan filter useless)
						aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
						aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
						aclRule.act.priAct.aclPri=rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_assign_priority.priority;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						//point to next ruleIdx
						aclIdx++;



					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_TRAP_OR_ASSIGN_PRIORITY:
					{
						ACL_RSV("add RSV[%d] ACK_PACKET_TRAP_OR_ASSIGN_PRIORITY @ acl[%d] & acl[%d]",type,aclIdx,aclIdx+1);

						/* Multiple HIT should add all pattern first and then add action */
						aclreverseIdx = aclIdx+1;

						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));

						aclRule.valid=ENABLED;
						aclRule.index=aclreverseIdx;
						aclRule.activePorts.bits[0]=rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_trap_or_assign_priority.portmask;
						aclRule.templateIdx=3; /*use :FS[2] => template[3],field[2]*/

						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 2;/*template[3],field[2]*/
						aclField.fieldUnion.data.value = rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_trap_or_assign_priority.tcp_flag_data; //ACK flag
						aclField.fieldUnion.data.mask = rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_trap_or_assign_priority.tcp_flag_mask;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_TRAP_OR_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_TRAP_OR_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclreverseIdx--;
						aclIdx++;

						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));

						aclRule.valid=ENABLED;
						aclRule.index=aclreverseIdx;
						aclRule.activePorts.bits[0]=rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_trap_or_assign_priority.portmask;
						aclRule.templateIdx=2; /*pktLenRange => template[2],field[7]*/

						pktLenEntryIdx = 0;
						if(_rtk_rg_search_acl_pktLenRangeTableEntry(&pktLenEntryIdx, TRUE, rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_trap_or_assign_priority.pktLenEnd, rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_trap_or_assign_priority.pktLenStart)!=RT_ERR_RG_OK)
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_TRAP_OR_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 7;/*template[2],field[7]*/
						aclField.fieldUnion.data.value = 0x1<<pktLenEntryIdx; //pktLenRange index
						aclField.fieldUnion.data.mask = 0x1<<pktLenEntryIdx;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_TRAP_OR_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//check TCP tagIf
						bzero(&aclField2,sizeof(aclField2));
						aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField2.fieldUnion.pattern.fieldIdx = 5;/*template[2],field[5]*/
						aclField2.fieldUnion.data.value = ACL_TAGIF_TCP_BIT; //TCP tagIf
						aclField2.fieldUnion.data.mask = ACL_TAGIF_TCP_BIT;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField2)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_TRAP_OR_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						switch(rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_trap_or_assign_priority.action) {
							case 0:
								aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
								aclRule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;
								break;
							case 1:
								aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
								aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
								aclRule.act.priAct.aclPri=rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_trap_or_assign_priority.priority;
								break;
							case 2:
								aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
								aclRule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;
								aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
								aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
								aclRule.act.priAct.aclPri=rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_trap_or_assign_priority.priority;
								break;
							default:
								WARNING("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_TRAP_OR_ASSIGN_PRIORITY failed(only support action 0~2)!!!");
								break;
						}
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_TRAP_OR_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						//point to next ruleIdx
						aclIdx++;
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP_OR_ASSIGN_PRIORITY:
					{
						ACL_RSV("add RSV[%d] SYN_PACKET_TRAP_OR_ASSIGN_PRIORITY @ acl[%d] & acl[%d]",type,aclIdx,aclIdx+1);

						/* Multiple HIT should add all pattern first and then add action */
						aclreverseIdx = aclIdx+1;

						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));

						aclRule.valid=ENABLED;
						aclRule.index=aclreverseIdx;
						aclRule.activePorts.bits[0]=rg_db.systemGlobal.trapSpecificLengthSyn.portMask;
						aclRule.templateIdx=3; /*use :FS[2] => template[3],field[2]*/

						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 2;/*template[3],field[2]*/
						if(rg_db.systemGlobal.trapSpecificLengthSyn.synOnly){
							aclField.fieldUnion.data.value = 0x2; //SYN flag
							aclField.fieldUnion.data.mask = 0xfff;
						}else{
							aclField.fieldUnion.data.value = 0x2; //SYN flag
							aclField.fieldUnion.data.mask = 0x2;
						}
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP_OR_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP_OR_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclreverseIdx--;
						aclIdx++;

						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));

						aclRule.valid=ENABLED;
						aclRule.index=aclreverseIdx;
						aclRule.activePorts.bits[0]=rg_db.systemGlobal.trapSpecificLengthSyn.portMask;
						aclRule.templateIdx=2; /*pktLenRange => template[2],field[7]*/

						pktLenEntryIdx = 0;
						if(_rtk_rg_search_acl_pktLenRangeTableEntry(&pktLenEntryIdx, TRUE, rg_db.systemGlobal.trapSpecificLengthSyn.pktLenEnd, rg_db.systemGlobal.trapSpecificLengthSyn.pktLenStart)!=RT_ERR_RG_OK)
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP_OR_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 7;/*template[2],field[7]*/
						aclField.fieldUnion.data.value = 0x1<<pktLenEntryIdx; //pktLenRange index
						aclField.fieldUnion.data.mask = 0x1<<pktLenEntryIdx;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP_OR_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//check TCP tagIf
						bzero(&aclField2,sizeof(aclField2));
						aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField2.fieldUnion.pattern.fieldIdx = 5;/*template[2],field[5]*/
						aclField2.fieldUnion.data.value = ACL_TAGIF_TCP_BIT; //TCP tagIf
						aclField2.fieldUnion.data.mask = ACL_TAGIF_TCP_BIT;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField2)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP_OR_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						if(rg_db.systemGlobal.trapSpecificLengthSyn.isTrap){
							aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
							aclRule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;
						}
						if(rg_db.systemGlobal.trapSpecificLengthSyn.priority){
							aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
							aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
							aclRule.act.priAct.aclPri=rg_db.systemGlobal.trapSpecificLengthSyn.priority;
						}

						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP_OR_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						//point to next ruleIdx
						aclIdx++;
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_DSLITE_TRAP:
					{
						ACL_RSV("add RSV[%d] DSLITE_TRAP @ acl[%d]",type,aclIdx);
						//HW have dslite bug will cause use page leakage, so trap v6 multicast dslite.

						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						bzero(&aclField2,sizeof(aclField2));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK;
						aclRule.templateIdx=2; /*use :TC_NH, FRAME_TAG*/

						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 4;/*template[2],field[4]*/
						aclField.fieldUnion.data.value = RG_IP_PROTO_IPinIP; //NH protocal value
						aclField.fieldUnion.data.mask = 0xff;//mask 8 bit only
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_DSLITE_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField2.fieldUnion.pattern.fieldIdx = 5;/*template[2],field[5]*/
						aclField2.fieldUnion.data.value = ACL_TAGIF_IP6_BIT ;
						aclField2.fieldUnion.data.mask = ACL_TAGIF_IP6_BIT;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField2)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_DSLITE_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_DSLITE_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_IGMP_MLD_DROP:
					{
						ACL_RSV("add RSV[%d] IGMP_MLD_DROP @ acl[%d]",type,aclIdx);

						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=rg_db.systemGlobal.aclAndCfReservedRule.igmp_mld_drop_portmask.portmask;
						aclRule.templateIdx=2; // tagif

						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 5;/*template[2],field[5]*/
						aclField.fieldUnion.data.value = ACL_TAGIF_IGMP_MLD_BIT; //IGMP/MLD tagif
						aclField.fieldUnion.data.mask = ACL_TAGIF_IGMP_MLD_BIT;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_IGMP_MLD_DROP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act = ACL_IGR_FORWARD_EGRESSMASK_ACT;
						aclRule.act.forwardAct.portMask.bits[0]= 0x0;	//drop
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_IGMP_MLD_DROP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_UNICAST_REDIRECT:
#if defined(CONFIG_RG_RTL9603CVD_SERIES)
					{
						ACL_RSV("Not support due to this reserve acl include CF rule");
						break;
					}
#endif
					{
						ACL_RSV("add RSV[%d] UNICAST_REDIRECT @ acl[%d] & cf[%d]",type,aclIdx, cfIdx);

						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField_dmac0,sizeof(aclField_dmac0));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						if(rg_db.systemGlobal.wanPortMask.portmask != 0x0)
							aclRule.activePorts.bits[0]=rg_db.systemGlobal.wanPortMask.portmask;
						else
							aclRule.activePorts.bits[0]=(1<<RTK_RG_PORT_PON);
						aclRule.templateIdx=0;

						aclField_dmac0.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField_dmac0.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
						aclField_dmac0.fieldUnion.data.value = 0x0000; //unicast mac byte[0],bit[0]==0
						aclField_dmac0.fieldUnion.data.mask = 0x0100;

						if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac0))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UNICAST_REDIRECT failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_REDIRECT_ACT;
						aclRule.act.forwardAct.portMask.bits[0]=(1<<rg_db.systemGlobal.aclAndCfReservedRule.unicast_redirect.redirect_to_port);
						aclRule.act.enableAct[ACL_IGR_CVLAN_ACT]=ENABLED;
						aclRule.act.cvlanAct.act=ACL_IGR_CVLAN_IGR_CVLAN_ACT;
						aclRule.act.cvlanAct.cvid=rg_db.systemGlobal.aclAndCfReservedRule.unicast_redirect.cvlan_cvid;

						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UNICAST_REDIRECT failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						bzero(&cfRule,sizeof(cfRule));
						bzero(&dsAct,sizeof(dsAct));
						//CF(from RGMII) diabled all actions

						dsAct.csAct=CLASSIFY_DS_CSACT_TRANSPARENT;
						dsAct.cAct=CLASSIFY_DS_CACT_TRANSPARENT;
						dsAct.interPriAct=CLASSIFY_CF_PRI_ACT_NOP;
						dsAct.uniAct=CLASSIFY_DS_UNI_ACT_NOP;
						dsAct.dscp=CLASSIFY_DSCP_ACT_DISABLE;

						cfRule.index=cfIdx;
						cfRule.direction=CLASSIFY_DIRECTION_DS;
						cfRule.valid=ENABLED;
						cfRule.act.dsAct=dsAct;

						if(RTK_CLASSIFY_CFGENTRY_ADD(&cfRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UNICAST_REDIRECT failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;
						cfIdx++;

					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_UNTAG_UNICAST_REDIRECT:
#if defined(CONFIG_RG_RTL9603CVD_SERIES)
					{
						ACL_RSV("Not support due to this reserve acl include CF rule");
						break;
					}
#endif
					{
						ACL_RSV("add RSV[%d] UNTAG_UNICAST_REDIRECT @ acl[%d] & cf[%d]",type,aclIdx, cfIdx);

						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField_dmac0,sizeof(aclField_dmac0));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=rg_db.systemGlobal.aclAndCfReservedRule.untag_unicast_redirect.ingress_portmask;
						aclRule.templateIdx=0;

						aclField_dmac0.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField_dmac0.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
						aclField_dmac0.fieldUnion.data.value = 0x0000; //unicast mac byte[0],bit[0]==0
						aclField_dmac0.fieldUnion.data.mask = 0x0100;

						if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac0))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UNTAG_UNICAST_REDIRECT failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.careTag.tags[ACL_CARE_TAG_STAG].value=0;	//untag
						aclRule.careTag.tags[ACL_CARE_TAG_STAG].mask=0xffff;
						aclRule.careTag.tags[ACL_CARE_TAG_CTAG].value=0;
						aclRule.careTag.tags[ACL_CARE_TAG_CTAG].mask=0xffff;

						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_REDIRECT_ACT;
						aclRule.act.forwardAct.portMask.bits[0]=(1<<rg_db.systemGlobal.aclAndCfReservedRule.untag_unicast_redirect.redirect_to_port);
						aclRule.act.enableAct[ACL_IGR_CVLAN_ACT]=ENABLED;
						aclRule.act.cvlanAct.act=ACL_IGR_CVLAN_IGR_CVLAN_ACT;
						aclRule.act.cvlanAct.cvid=rg_db.systemGlobal.aclAndCfReservedRule.untag_unicast_redirect.acl_igr_cvlan_cvid;
						aclRule.act.enableAct[ACL_IGR_INTR_ACT]=ENABLED;
						aclRule.act.aclInterrupt=ENABLED;
						aclRule.act.aclLatch=ENABLED;

						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UNTAG_UNICAST_REDIRECT failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						bzero(&cfRule,sizeof(cfRule));
						bzero(&dsAct,sizeof(dsAct));
						//CF assign ctag id

						classifyField.fieldType =CLASSIFY_FIELD_ACL_HIT;
						classifyField.classify_pattern.fieldData.value=aclIdx;
						classifyField.classify_pattern.fieldData.mask=0x7f;

						if(rtk_classify_field_add(&cfRule, &classifyField))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UNTAG_UNICAST_REDIRECT failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						dsAct.csAct=CLASSIFY_DS_CSACT_TRANSPARENT;
						dsAct.cAct=CLASSIFY_DS_CACT_ADD_CTAG_8100;
						dsAct.cVidAct=CLASSIFY_DS_VID_ACT_ASSIGN;
						dsAct.cTagVid=rg_db.systemGlobal.aclAndCfReservedRule.untag_unicast_redirect.cf_egr_cvlan_cvid;
						dsAct.interPriAct=CLASSIFY_CF_PRI_ACT_NOP;
						dsAct.uniAct=CLASSIFY_DS_UNI_ACT_NOP;
						dsAct.dscp=CLASSIFY_DSCP_ACT_DISABLE;

						cfRule.index=cfIdx;
						cfRule.direction=CLASSIFY_DIRECTION_DS;
						cfRule.valid=ENABLED;
						cfRule.act.dsAct=dsAct;

						if(RTK_CLASSIFY_CFGENTRY_ADD(&cfRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UNTAG_UNICAST_REDIRECT failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;
						cfIdx++;

					}
					break;


				default:
					break;

			}

		}
	}

	rg_db.systemGlobal.aclAndCfReservedRule.aclLowerBoundary=aclIdx;
	rg_db.systemGlobal.aclAndCfReservedRule.cfLowerBoundary=cfIdx;

	if(addRuleFailedFlag==1)
		return (RT_ERR_RG_FAILED);

	//[Care Asic ACL Full] user acl can be S/W only, so reflash after reserve acl update finish
	//ASSERT_EQ(_rtk_rg_aclSWEntry_and_asic_rearrange(FAIL),RT_ERR_RG_OK);


	return (RT_ERR_RG_OK);
}

static int _rtk_rg_aclAndCfReservedRuleTailReflash(void)
{
	uint32 type;
	uint32 aclIdx=(MAX_ACL_ENTRY_SIZE-1);
	rtk_acl_ingress_entry_t aclRule;
	rtk_acl_field_t aclField,aclField2,aclField_dmac0,aclField_dmac1,aclField_dmac2;
	uint16 pktLenEntryIdx;

	int addRuleFailedFlag=0;
	uint32 i;


	//reflash th rules
	for(i=rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary;i<MAX_ACL_ENTRY_SIZE;i++){
		assert_ok(rtk_acl_igrRuleEntry_del(i));
	}

	for(type=RTK_RG_ACLANDCF_RESERVED_HEAD_END;type<RTK_RG_ACLANDCF_RESERVED_TAIL_END;type++){
		if(rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[type]==ENABLED){
			switch(type){

				case RTK_RG_ACLANDCF_RESERVED_DHCP_PACKET_ASSIGN_SHARE_METER:
					ACL_RSV("add RSV[%d] DHCP_PACKET_ASSIGN_SHARE_METER @ acl[%d] & acl[%d] &  acl[%d] & acl[%d]",type,aclIdx,aclIdx-1,aclIdx-2,aclIdx-3);

					//rule1: UDP_l4_sport (67) + ipv4
					bzero(&aclRule,sizeof(aclRule));
					bzero(&aclField,sizeof(aclField));

					aclRule.valid=ENABLED;
					aclRule.index=aclIdx;
					aclRule.activePorts.bits[0]=rg_db.systemGlobal.aclAndCfReservedRule.dhcp_packet_assign_share_meter.portmask;
					aclRule.templateIdx=7; /*use :UDP_SPORT  => template[7],field[3]*/

					aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField.fieldUnion.pattern.fieldIdx = 3;/*template[7],field[3]*/
					aclField.fieldUnion.data.value = 67; //UDP sport
					aclField.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_DHCP_PACKET_ASSIGN_SHARE_METER failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_DHCP_PACKET_ASSIGN_SHARE_METER failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					//point to next ruleIdx
					aclIdx--;

					//continue rule: ipv4
					bzero(&aclRule,sizeof(aclRule));
					bzero(&aclField,sizeof(aclField));

					aclRule.valid=ENABLED;
					aclRule.index=aclIdx;
					aclRule.activePorts.bits[0]=rg_db.systemGlobal.aclAndCfReservedRule.dhcp_packet_assign_share_meter.portmask;
					aclRule.templateIdx=2; /*use :FRAMETAG	=> template[2],field[5]*/

					aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField.fieldUnion.pattern.fieldIdx = 5;/*template[2],field[5]*/
					aclField.fieldUnion.data.value = ACL_TAGIF_OUT_IP4_BIT; //ipv4
					aclField.fieldUnion.data.mask = ACL_TAGIF_OUT_IP4_BIT;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_DHCP_PACKET_ASSIGN_SHARE_METER failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					//assign share meter
					aclRule.act.enableAct[ACL_IGR_LOG_ACT]=ENABLED;
					aclRule.act.logAct.act=ACL_IGR_LOG_POLICING_ACT;
					aclRule.act.logAct.meter=rg_db.systemGlobal.aclAndCfReservedRule.dhcp_packet_assign_share_meter.share_meter;
					if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_DHCP_PACKET_ASSIGN_SHARE_METER failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					//point to next ruleIdx
					aclIdx--;



					//rule2: UDP_l4_sport (68) + ipv4
					bzero(&aclRule,sizeof(aclRule));
					bzero(&aclField,sizeof(aclField));

					aclRule.valid=ENABLED;
					aclRule.index=aclIdx;
					aclRule.activePorts.bits[0]=rg_db.systemGlobal.aclAndCfReservedRule.dhcp_packet_assign_share_meter.portmask;
					aclRule.templateIdx=7; /*use :UDP_SPORT  => template[7],field[3]*/

					aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField.fieldUnion.pattern.fieldIdx = 3;/*template[7],field[3]*/
					aclField.fieldUnion.data.value = 68; //UDP sport
					aclField.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_DHCP_PACKET_ASSIGN_SHARE_METER failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_DHCP_PACKET_ASSIGN_SHARE_METER failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					//point to next ruleIdx
					aclIdx--;

					//continue rule: ipv4
					bzero(&aclRule,sizeof(aclRule));
					bzero(&aclField,sizeof(aclField));

					aclRule.valid=ENABLED;
					aclRule.index=aclIdx;
					aclRule.activePorts.bits[0]=rg_db.systemGlobal.aclAndCfReservedRule.dhcp_packet_assign_share_meter.portmask;
					aclRule.templateIdx=2; /*use :FRAMETAG	=> template[2],field[5]*/

					aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField.fieldUnion.pattern.fieldIdx = 5;/*template[2],field[5]*/
					aclField.fieldUnion.data.value = ACL_TAGIF_OUT_IP4_BIT; //ipv4
					aclField.fieldUnion.data.mask = ACL_TAGIF_OUT_IP4_BIT;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_DHCP_PACKET_ASSIGN_SHARE_METER failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					//assign share meter
					aclRule.act.enableAct[ACL_IGR_LOG_ACT]=ENABLED;
					aclRule.act.logAct.act=ACL_IGR_LOG_POLICING_ACT;
					aclRule.act.logAct.meter=rg_db.systemGlobal.aclAndCfReservedRule.dhcp_packet_assign_share_meter.share_meter;
					if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_DHCP_PACKET_ASSIGN_SHARE_METER failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					//point to next ruleIdx
					aclIdx--;
					break;

				case RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_ASSIGN_SHARE_METER:
					ACL_RSV("add RSV[%d] SYN_PACKET_ASSIGN_SHARE_METER @ acl[%d] & acl[%d]",type,aclIdx,aclIdx-1);

					bzero(&aclRule,sizeof(aclRule));
					bzero(&aclField,sizeof(aclField));

					aclRule.valid=ENABLED;
					aclRule.index=aclIdx;
					if(rg_db.systemGlobal.wanPortMask.portmask != 0x0)
						aclRule.activePorts.bits[0]=rg_db.systemGlobal.wanPortMask.portmask;
					else
						aclRule.activePorts.bits[0]=(1<<RTK_RG_PORT_PON);
					aclRule.templateIdx=2; /*use :FRAME_TAG => template[2],field[5]*/

					//check TCP tagIf
					bzero(&aclField2,sizeof(aclField2));
					aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField2.fieldUnion.pattern.fieldIdx = 5;/*template[2],field[5]*/
					aclField2.fieldUnion.data.value = ACL_TAGIF_TCP_BIT; //TCP tagIf
					aclField2.fieldUnion.data.mask = ACL_TAGIF_TCP_BIT;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField2)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_ASSIGN_SHARE_METER failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_ASSIGN_SHARE_METER failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					//point to next ruleIdx
					aclIdx--;

					bzero(&aclRule,sizeof(aclRule));
					bzero(&aclField,sizeof(aclField));

					aclRule.valid=ENABLED;
					aclRule.index=aclIdx;
					if(rg_db.systemGlobal.wanPortMask.portmask != 0x0)
						aclRule.activePorts.bits[0]=rg_db.systemGlobal.wanPortMask.portmask;
					else
						aclRule.activePorts.bits[0]=(1<<RTK_RG_PORT_PON);
					aclRule.templateIdx=3; /*use :FS[2] => template[3],field[2]*/

					aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField.fieldUnion.pattern.fieldIdx = 2;/*template[3],field[2]*/
					aclField.fieldUnion.data.value = 0x2; //TCP flag: SYN=1, ACK=0
					aclField.fieldUnion.data.mask = 0x12;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_ASSIGN_SHARE_METER failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					//assign share meter
					aclRule.act.enableAct[ACL_IGR_LOG_ACT]=ENABLED;
					aclRule.act.logAct.act=ACL_IGR_LOG_POLICING_ACT;
					aclRule.act.logAct.meter=rg_db.systemGlobal.aclAndCfReservedRule.syn_packet_assign_share_meter.share_meter;

					if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_ASSIGN_SHARE_METER failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					//point to next ruleIdx
					aclIdx--;
					break;

				case RTK_RG_ACLANDCF_RESERVED_ARP_PACKET_ASSIGN_SHARE_METER:
					ACL_RSV("add RSV[%d] ARP_PACKET_ASSIGN_SHARE_METER @ acl[%d] & acl[%d]",type,aclIdx,aclIdx-1);

					bzero(&aclRule,sizeof(aclRule));

					aclRule.valid=ENABLED;
					aclRule.index=aclIdx;
					aclRule.activePorts.bits[0]=rg_db.systemGlobal.aclAndCfReservedRule.arp_packet_assign_share_meter.portmask;
					aclRule.templateIdx=0; /*use :DMAC => template[0],field[0]-field[2]*/

					//broadcast mac to prevent impact arp response
					bzero(&aclField_dmac2,sizeof(aclField_dmac2));
					aclField_dmac2.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_dmac2.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
					aclField_dmac2.fieldUnion.data.value = 0xffff; //broadcast mac
					aclField_dmac2.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac2))
					{
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ARP_PACKET_ASSIGN_SHARE_METER failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					bzero(&aclField_dmac1,sizeof(aclField_dmac1));
					aclField_dmac1.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_dmac1.fieldUnion.pattern.fieldIdx = 1; //DA[31:16]: template[0] field[1]
					aclField_dmac1.fieldUnion.data.value = 0xffff; //broadcast mac
					aclField_dmac1.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac1))
					{
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ARP_PACKET_ASSIGN_SHARE_METER failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					bzero(&aclField_dmac0,sizeof(aclField_dmac0));
					aclField_dmac0.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_dmac0.fieldUnion.pattern.fieldIdx = 0; //DA[15:0]: template[0] field[0]
					aclField_dmac0.fieldUnion.data.value = 0xffff; //broadcast mac
					aclField_dmac0.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac0))
					{
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ARP_PACKET_ASSIGN_SHARE_METER failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ARP_PACKET_ASSIGN_SHARE_METER failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					//point to next ruleIdx
					aclIdx--;

					bzero(&aclRule,sizeof(aclRule));
					bzero(&aclField,sizeof(aclField));

					aclRule.valid=ENABLED;
					aclRule.index=aclIdx;
					aclRule.activePorts.bits[0]=rg_db.systemGlobal.aclAndCfReservedRule.arp_packet_assign_share_meter.portmask;
					aclRule.templateIdx=1; /*use :ETHERTYPE => template[1],field[5]*/

					aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField.fieldUnion.pattern.fieldIdx = 5;/*template[1],field[5]*/
					aclField.fieldUnion.data.value = RG_ARP_ETHERTYPE; //ethertype=0x806
					aclField.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ARP_PACKET_ASSIGN_SHARE_METER failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					//assign share meter
					aclRule.act.enableAct[ACL_IGR_LOG_ACT]=ENABLED;
					aclRule.act.logAct.act=ACL_IGR_LOG_POLICING_ACT;
					aclRule.act.logAct.meter=rg_db.systemGlobal.aclAndCfReservedRule.arp_packet_assign_share_meter.share_meter;

					if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ARP_PACKET_ASSIGN_SHARE_METER failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					//point to next ruleIdx
					aclIdx--;
					break;

				case RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER:
					ACL_RSV("add RSV[%d] UDP_DOS_PACKET_ASSIGN_SHARE_METER @ acl[%d] - acl[%d]",type,(aclIdx-rg_db.systemGlobal.dosRateLimitCount*3+1),aclIdx);

					RTK_RG_DOS_RATE_LIMIT_VALID_SCAN(i){
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						bzero(&aclField2,sizeof(aclField2));

						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=rg_db.systemGlobal.dosRateLimit[i].portmask;
						aclRule.templateIdx=2; /*use :FRAME_TAG/PKTELNRANGE => template[2],field[5] and template[2],field[7]*/

						//check TCP tagIf
						aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField2.fieldUnion.pattern.fieldIdx = 5;/*template[2],field[5]*/
						if(rg_db.systemGlobal.dosRateLimit[i].isTcp) {
							aclField2.fieldUnion.data.value = ACL_TAGIF_TCP_BIT; //TCP tagIf
							aclField2.fieldUnion.data.mask = ACL_TAGIF_TCP_BIT;
						}
						else {
							aclField2.fieldUnion.data.value = ACL_TAGIF_UDP_BIT; //UDP tagIf
							aclField2.fieldUnion.data.mask = ACL_TAGIF_UDP_BIT;
						}
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField2)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//limit packet size is samller than pktLen, SW rate limit not include crc
						pktLenEntryIdx = 0;
						if(_rtk_rg_search_acl_pktLenRangeTableEntry(&pktLenEntryIdx, TRUE, (rg_db.systemGlobal.dosRateLimit[i].pktLenEnd+4), (rg_db.systemGlobal.dosRateLimit[i].pktLenStart+4))!=RT_ERR_RG_OK)
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 7;/*template[2],field[7]*/
						aclField.fieldUnion.data.value = 0x1<<pktLenEntryIdx; //pktLenRange index
						aclField.fieldUnion.data.mask = 0x1<<pktLenEntryIdx;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx--;

						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						bzero(&aclField2,sizeof(aclField2));

						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=rg_db.systemGlobal.dosRateLimit[i].portmask;
						aclRule.templateIdx=1; /*use :IP4DIP => template[1],field[0]-field[1]*/

						aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField2.fieldUnion.pattern.fieldIdx = 0;/*template[1],field[0]*/
						aclField2.fieldUnion.data.value = (rg_db.systemGlobal.dosRateLimit[i].ingressDip & 0xffff);//SIP[15:0]
						aclField2.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField2)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 1;/*template[1],field[1]*/
						aclField.fieldUnion.data.value = (rg_db.systemGlobal.dosRateLimit[i].ingressDip & 0xffff0000)>>16;//SIP[31:16]
						aclField.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx--;

						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));

						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=rg_db.systemGlobal.dosRateLimit[i].portmask;
						aclRule.templateIdx=0; /*use :DMAC/CTAG => template[0],field[0]-field[2],field[6]*/

						bzero(&aclField_dmac2,sizeof(aclField_dmac2));
						aclField_dmac2.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField_dmac2.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
						aclField_dmac2.fieldUnion.data.value = ((rg_db.systemGlobal.dosRateLimit[i].ingressDmac.octet[0]<<8)
																|(rg_db.systemGlobal.dosRateLimit[i].ingressDmac.octet[1]));
						aclField_dmac2.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac2))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						bzero(&aclField_dmac1,sizeof(aclField_dmac1));
						aclField_dmac1.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField_dmac1.fieldUnion.pattern.fieldIdx = 1; //DA[31:16]: template[0] field[1]
						aclField_dmac1.fieldUnion.data.value = ((rg_db.systemGlobal.dosRateLimit[i].ingressDmac.octet[2]<<8)
																|(rg_db.systemGlobal.dosRateLimit[i].ingressDmac.octet[3]));
						aclField_dmac1.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac1))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						bzero(&aclField_dmac0,sizeof(aclField_dmac0));
						aclField_dmac0.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField_dmac0.fieldUnion.pattern.fieldIdx = 0; //DA[15:0]: template[0] field[0]
						aclField_dmac0.fieldUnion.data.value = ((rg_db.systemGlobal.dosRateLimit[i].ingressDmac.octet[4]<<8)
																|(rg_db.systemGlobal.dosRateLimit[i].ingressDmac.octet[5]));
						aclField_dmac0.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac0))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						if(rg_db.systemGlobal.dosRateLimit[i].ctagif)
						{
							aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
							aclField.fieldUnion.pattern.fieldIdx = 6;/*template[0],field[6]*/
							aclField.fieldUnion.data.value = rg_db.systemGlobal.dosRateLimit[i].ctagVid;
							aclField.fieldUnion.data.mask = 0x0fff;
							if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER failed!!!");
								addRuleFailedFlag=1;
								break;
							}
						}

						//ctagif
						aclRule.careTag.tags[ACL_CARE_TAG_CTAG].value=rg_db.systemGlobal.dosRateLimit[i].ctagif;
						aclRule.careTag.tags[ACL_CARE_TAG_CTAG].mask=0xffff;

						//assign share meter
						aclRule.act.enableAct[ACL_IGR_LOG_ACT]=ENABLED;
						aclRule.act.logAct.act=ACL_IGR_LOG_POLICING_ACT;
						aclRule.act.logAct.meter=rg_db.systemGlobal.dosRateLimit[i].shareMeterIdx_hw;

						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx--;
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_EGRESS_VLAN_FILTER_DISABLE:
					//translate egress cvlan to 1, to avoid FB HW drop when CF cvlan remarking to none-exist vlan_id(Downward compatibility as apollo, apolloFE)
					ACL_RSV("add RSV[%d] EGRESS_VLAN_FILTER_DISABLE @ acl[%d]",type,aclIdx);
					bzero(&aclRule,sizeof(aclRule));
					aclRule.valid=ENABLED;
					aclRule.index=aclIdx;
					aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
					aclRule.act.enableAct[ACL_IGR_CVLAN_ACT]=ENABLED;
					aclRule.act.cvlanAct.act=ACL_IGR_CVLAN_EGR_CVLAN_ACT;
					aclRule.act.cvlanAct.cvid=rg_db.systemGlobal.initParam.fwdVLAN_CPU; //Vlan-1, consider as none vlan filter

					if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type))
					{
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_EGRESS_VLAN_FILTER_DISABLE failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					//point to next ruleIdx
					aclIdx--;
					break;

				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT:
					{

					/*new default policy, drop unknownDA UDP multicast, trap else multicast*/
						ACL_RSV("add RSV[%d] MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT @ acl[%d] & acl[%d]",type,aclIdx,aclIdx-1);

						//rule for trap else multicast packet (such as unknownDA ICMP...etc)
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 7; //dipv6[127:112] in template[6]:field[7]
						aclField.fieldUnion.data.value = 0xff00; //dipv6[127:112]
						aclField.fieldUnion.data.mask = 0xff00;

						if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_URLFILTER_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.valid = ENABLE;
						aclRule.index = aclIdx;
						aclRule.templateIdx = 6; //dipv6 in template[6]
						aclRule.activePorts.bits[0] = RTK_RG_ALL_MAC_PORTMASK & (~(1<<RTK_RG_MAC_PORT_CPU));
						//trap to cpu action
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
						aclRule.act.forwardAct.act= ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx--;


						//rule for drop unknownDA UDP multicast (use permit action, if unknownDA will dropped by HW reason 207)
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 7; //dipv6[127:112] in template[6]:field[7]
						aclField.fieldUnion.data.value = 0xff0e; //dipv6[127:112]
						aclField.fieldUnion.data.mask = 0xff0f;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_URLFILTER_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.valid = ENABLE;
						aclRule.index = aclIdx;
						aclRule.templateIdx = 6;//dipv6 in template[6]
						aclRule.activePorts.bits[0] = RTK_RG_ALL_MAC_PORTMASK & (~(1<<RTK_RG_MAC_PORT_CPU));
						//aclRule.careTag.tags[ACL_CARE_TAG_UDP].value=ENABLED;
						//aclRule.careTag.tags[ACL_CARE_TAG_UDP].mask=0xffff;
						//permit, used to avoid next trap action
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
						//aclRule.act.forwardAct.act= ACL_IGR_FORWARD_COPY_ACT;
						aclRule.act.forwardAct.act= ACL_IGR_FORWARD_EGRESSMASK_ACT; //permit action
						aclRule.act.forwardAct.portMask.bits[0]=RTK_RG_ALL_MAC_PORTMASK;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx--;
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP:
					{
						if(rg_db.systemGlobal.icmp_trap_by_acl_disable == RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_ENABLE)
							ACL_RSV("add RSV[%d] CONTROL_PACKET_TRAP @ acl[%d] & acl[%d] & acl[%d] & acl[%d] & acl[%d]",type,aclIdx,aclIdx-1,aclIdx-2,aclIdx-3,aclIdx-4);
						else if(rg_db.systemGlobal.icmp_trap_by_acl_disable == RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_DISABLE)
							ACL_RSV("add RSV[%d] CONTROL_PACKET_TRAP @ acl[%d] & acl[%d] & acl[%d]",type,aclIdx,aclIdx-1,aclIdx-2);
						else
							ACL_RSV("add RSV[%d] CONTROL_PACKET_TRAP @ acl[%d] & acl[%d] & acl[%d] & acl[%d]",type,aclIdx,aclIdx-1,aclIdx-2,aclIdx-3);
						if(rg_db.systemGlobal.icmp_trap_by_acl_disable != RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_ENABLE)
							ACL_RSV("\t Attention: take care icmp/icmpv6 by proc/rg/icmp_trap_by_acl_disable");
						// 5 ACL is added, trap control packet(ex:ICMP or IPCP/IP6CP)which  could be forwarded by PATH1 flow which is created by other packet.


						//rule5:PPPoE tagif=1, none-ipv4, none-ipv6, action=trap
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK;
						aclRule.templateIdx=2; /*use :FRAME_TAG*/


						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 5;/*template[2],field[5]*/
						aclField.fieldUnion.data.value = ACL_TAGIF_PPPoE_8863_8864_BIT ;
						aclField.fieldUnion.data.mask = (ACL_TAGIF_PPPoE_8863_8864_BIT|ACL_TAGIF_OUT_IP4_BIT|ACL_TAGIF_IP6_BIT);
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclIdx--;


						//rule4: IPv4 tagif=1 + IGMP, action=trap
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK;
						aclRule.templateIdx=2; /*use :FRAME_TAG*/


						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 5;/*template[2],field[5]*/
						aclField.fieldUnion.data.value = (ACL_TAGIF_OUT_IP4_BIT|ACL_TAGIF_IGMP_MLD_BIT) ;
						aclField.fieldUnion.data.mask = (ACL_TAGIF_OUT_IP4_BIT|ACL_TAGIF_IGMP_MLD_BIT);
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclIdx--;


						//rule3: IPv6 tagif=1 + MLD, action=trap
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK;
						aclRule.templateIdx=2; /*use :FRAME_TAG*/


						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 5;/*template[2],field[5]*/
						aclField.fieldUnion.data.value = (ACL_TAGIF_IP6_BIT|ACL_TAGIF_IGMP_MLD_BIT) ;
						aclField.fieldUnion.data.mask = (ACL_TAGIF_IP6_BIT|ACL_TAGIF_IGMP_MLD_BIT);
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclIdx--;



						//rule2:	IPv4 tagif=1and l4protocol=0x0001(ICMP), action=trap
						if((rg_db.systemGlobal.icmp_trap_by_acl_disable == RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_ENABLE) || (rg_db.systemGlobal.icmp_trap_by_acl_disable == RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_IPV6_DISABLE)) {
							bzero(&aclRule,sizeof(aclRule));
							bzero(&aclField,sizeof(aclField));
							bzero(&aclField2,sizeof(aclField2));
							aclRule.valid=ENABLED;
							aclRule.index=aclIdx;
							aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK;
							aclRule.templateIdx=2; /*use :TOS_PROTO and FRAME_TAG*/

							aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
							aclField.fieldUnion.pattern.fieldIdx = 3;/*template[2],field[3]*/
							aclField.fieldUnion.data.value = RG_IP_PROTO_ICMP; //IP protocal value
							aclField.fieldUnion.data.mask = 0xff;
							if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP failed!!!");
								addRuleFailedFlag=1;
								break;
							}
							aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
							aclField2.fieldUnion.pattern.fieldIdx = 5;/*template[2],field[5]*/
							aclField2.fieldUnion.data.value = ACL_TAGIF_OUT_IP4_BIT; //outter IPv4 tagIf
							aclField2.fieldUnion.data.mask = ACL_TAGIF_OUT_IP4_BIT;
							if(rtk_acl_igrRuleField_add(&aclRule, &aclField2)){
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
							aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT; //trap action
							aclRule.act.forwardAct.portMask.bits[0] = RTK_RG_ALL_MAC_PORTMASK;
							if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							aclIdx--;
						}

						//rule1:	IPv6 tagif=1 and l4protocol=0x003a (ICMPv6), action=trap
						if((rg_db.systemGlobal.icmp_trap_by_acl_disable == RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_ENABLE) || (rg_db.systemGlobal.icmp_trap_by_acl_disable == RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_IPV4_DISABLE)) {
							bzero(&aclRule,sizeof(aclRule));
							bzero(&aclField,sizeof(aclField));
							bzero(&aclField2,sizeof(aclField2));
							aclRule.valid=ENABLED;
							aclRule.index=aclIdx;
							aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK;
							aclRule.templateIdx=2; /*use :TC_NH and FRAME_TAG*/

							aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
							aclField.fieldUnion.pattern.fieldIdx = 4;/*template[2],field[4]*/
							aclField.fieldUnion.data.value = RG_IP_PROTO_ICMPv6; //NH protocal value
							aclField.fieldUnion.data.mask = 0xff;
							if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP failed!!!");
								addRuleFailedFlag=1;
								break;
							}
							aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
							aclField2.fieldUnion.pattern.fieldIdx = 5;/*template[2],field[5]*/
							aclField2.fieldUnion.data.value = ACL_TAGIF_IP6_BIT; //outter IPv4 tagIf
							aclField2.fieldUnion.data.mask = ACL_TAGIF_IP6_BIT;
							if(rtk_acl_igrRuleField_add(&aclRule, &aclField2)){
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
							aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT; //trap action
							//aclRule.act.forwardAct.portMask.bits[0] = RTK_RG_ALL_MAC_PORTMASK;
							if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							aclIdx--;
						}

					}
					break;

				default:
					break;
			}
		}
	}

	rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary=aclIdx;
	if(addRuleFailedFlag==1)
		return (RT_ERR_RG_FAILED);

	return (RT_ERR_RG_OK);

}

int _rtk_rg_aclAndCfReservedRuleAdd(rtk_rg_aclAndCf_reserved_type_t rsvType, void *parameter)
{
	uint check_num = 0;

	ACL_RSV("#####Reserved ACL reflash!(add reserved ACL rsvType=%d)#####",rsvType);

	if(rsvType == RTK_RG_ACLANDCF_RESERVED_TAIL_END)
		goto forceReflash;

	if(rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]==ENABLED)
		return (RT_ERR_RG_OK);

	switch(rsvType){

		case RTK_RG_ACLANDCF_RESERVED_UNICAST_TRAP:	//reserved one more for MC temporary permit while ACL rearrange
		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP:	//reserved one more for MC temporary permit while ACL rearrange
		case RTK_RG_ACLANDCF_RESERVED_DOT1X_EAPOL_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_DSLITE_TRAP:
		//HEAD_END
		case RTK_RG_ACLANDCF_RESERVED_EGRESS_VLAN_FILTER_DISABLE:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_ALL_TRAP:	//reserved one more for MC temporary permit while ACL rearrange
		case RTK_RG_ACLANDCF_RESERVED_STPBLOCKING:
		//HEAD_END
		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0,rsvType));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		//HEAD RSV

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

		case RTK_RG_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_lcp_assign_prioity,(rtk_rg_aclAndCf_reserved_pppoe_lcp_assign_priority_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoe_lcp_assign_priority_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.l2tp_ctrl_lcp_assign_prioity,(rtk_rg_aclAndCf_reserved_l2tp_control_lcp_trap_and_assign_priority_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_l2tp_control_lcp_trap_and_assign_priority_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE0_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE1_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE2_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE3_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE4_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE5_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE6_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE7_TRAP:		
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0,rsvType));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.mc_pppoe_trap[rsvType-RTK_RG_ACLANDCF_RESERVED_MULTICAST_PPPOE_RULE0_TRAP],(rtk_rg_aclAndCf_multicast_pppoe_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_multicast_pppoe_trap_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
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

		case RTK_RG_ACLANDCF_RESERVED_IGMP_MLD_DROP:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.igmp_mld_drop_portmask,(rtk_rg_aclAndCf_reserved_igmp_mld_drop_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_igmp_mld_drop_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_UNICAST_REDIRECT:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,1,rsvType));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.unicast_redirect,(rtk_rg_aclAndCf_reserved_unicast_redirect_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_unicast_redirect_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_UNTAG_UNICAST_REDIRECT:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,1,rsvType));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.untag_unicast_redirect,(rtk_rg_aclAndCf_reserved_untag_unicast_redirect_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_untag_unicast_redirect_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		//TAIL RSV

		case RTK_RG_ACLANDCF_RESERVED_DHCP_PACKET_ASSIGN_SHARE_METER:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(4,0,rsvType));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.dhcp_packet_assign_share_meter,(rtk_rg_aclAndCf_reserved_dhcp_packet_assign_share_meter_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_dhcp_packet_assign_share_meter_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_ASSIGN_SHARE_METER:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.syn_packet_assign_share_meter,(rtk_rg_aclAndCf_reserved_syn_packet_assign_share_meter_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_syn_packet_assign_share_meter_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_ARP_PACKET_ASSIGN_SHARE_METER:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0,rsvType));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.arp_packet_assign_share_meter,(rtk_rg_aclAndCf_reserved_arp_packet_assign_share_meter_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_arp_packet_assign_share_meter_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_UDP_DOS_PACKET_ASSIGN_SHARE_METER:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(3*rg_db.systemGlobal.dosRateLimitCount,0,rsvType));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP:
			if(rg_db.systemGlobal.icmp_trap_by_acl_disable == RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_ENABLE)
				check_num = 5;
			else if(rg_db.systemGlobal.icmp_trap_by_acl_disable == RTK_RG_ACL_CONTROL_PACKET_TRAP_RSV_ICMP_TRAP_DISABLE)
				check_num = 3;	//disable icmpv4 and icmpv6
			else
				check_num = 4;	//disable icmpv4 or icmpv6
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(check_num,0,rsvType));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;
		case RTK_RG_ACLANDCF_RESERVED_NPTV6_ACC_UPSTREAM_FROM_LAN0_ASSIGN_SHARE_METER:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.nptv6_acc_upstream_assign_share_meter[0],(rtk_rg_aclAndCf_reserved_nptv6_acc_upstream_assign_share_meter_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_nptv6_acc_upstream_assign_share_meter_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_NPTV6_ACC_UPSTREAM_FROM_LAN0_ASSIGN_SHARE_METER]=ENABLED; 
			break;
		case RTK_RG_ACLANDCF_RESERVED_NPTV6_ACC_UPSTREAM_FROM_LAN1_ASSIGN_SHARE_METER:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.nptv6_acc_upstream_assign_share_meter[1],(rtk_rg_aclAndCf_reserved_nptv6_acc_upstream_assign_share_meter_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_nptv6_acc_upstream_assign_share_meter_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_NPTV6_ACC_UPSTREAM_FROM_LAN1_ASSIGN_SHARE_METER]=ENABLED; 
			break;	
		case RTK_RG_ACLANDCF_RESERVED_NPTV6_ACC_UPSTREAM_FROM_LAN2_ASSIGN_SHARE_METER:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.nptv6_acc_upstream_assign_share_meter[2],(rtk_rg_aclAndCf_reserved_nptv6_acc_upstream_assign_share_meter_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_nptv6_acc_upstream_assign_share_meter_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_NPTV6_ACC_UPSTREAM_FROM_LAN2_ASSIGN_SHARE_METER]=ENABLED; 
			break;	
		case RTK_RG_ACLANDCF_RESERVED_NPTV6_ACC_UPSTREAM_FROM_LAN3_ASSIGN_SHARE_METER:
			assert_ok(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType));
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.nptv6_acc_upstream_assign_share_meter[3],(rtk_rg_aclAndCf_reserved_nptv6_acc_upstream_assign_share_meter_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_nptv6_acc_upstream_assign_share_meter_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[RTK_RG_ACLANDCF_RESERVED_NPTV6_ACC_UPSTREAM_FROM_LAN3_ASSIGN_SHARE_METER]=ENABLED; 
			break;
		default:
			break;
	}

forceReflash:
	_rtk_rg_aclAndCfReservedRuleReflash();

	ACL_RSV("RESERVED_ACL_BEFORE=%d, RESERVED_ACL_AFTER=%d",RESERVED_ACL_BEFORE,RESERVED_ACL_AFTER);

	return (RT_ERR_RG_OK);

}


int _rtk_rg_aclAndCfReservedRuleDel(rtk_rg_aclAndCf_reserved_type_t rsvType)
{
	ACL_RSV("#####Reserved ACL reflash!(del reserved ACL rsvType=%d)#####",rsvType);

	if(rsvType == RTK_RG_ACLANDCF_RESERVED_TAIL_END)
		goto forceReflash;

	if(rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]==DISABLED)
		return (RT_ERR_RG_OK);

	rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=DISABLED;

forceReflash:
	_rtk_rg_aclAndCfReservedRuleReflash();

	return (RT_ERR_RG_OK);
}


#endif //end of !defined(CONFIG_RG_G3_SERIES_DEVELOPMENT)



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

		if(pChkRule->acl_filter.filter_fields&INGRESS_TCP_FLAGS_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if((pChkRule->acl_filter.ingress_tcp_flags&pChkRule->acl_filter.ingress_tcp_flags_mask)!=((*((u8*)(&pPktHdr->tcpFlags)))&pChkRule->acl_filter.ingress_tcp_flags_mask)){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_TCP_FLAGS_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_TCP_FLAGS_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_PKT_LEN_RANGE_BIT)
		{
			PATTERN_CHECK_INIT(possitive_check);
			if( !((pChkRule->acl_filter.ingress_packet_length_end >= (pPktHdr->skbLen+4) ) &&	//follow asic packet range behavior: includer CRC 4byte
				(pChkRule->acl_filter.ingress_packet_length_start <= (pPktHdr->skbLen+4) ))){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_PKT_LEN_RANGE_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_PKT_LEN_RANGE_BIT UNHIT");
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

				if(((pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_TRAP_WITH_PRIORITY)||(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_TRAP2SLAVE_WITH_PRIORITY)) &&
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

		if( rg_db.systemGlobal.urlHiPri_En &&  naptIdx>0 && rg_db.naptOut[naptIdx].urlPriEn )
		{
			TRACE("Using URL PRIORIY to internalPriority [%d]->[%d]",pPktHdr->internalPriority,rg_db.systemGlobal.urlHiPri_table_entry[rg_db.naptOut[naptIdx].urlPriIdx].urlHighPriEt.urlpri);
			pPktHdr->internalPriority =  rg_db.systemGlobal.urlHiPri_table_entry[rg_db.naptOut[naptIdx].urlPriIdx].urlHighPriEt.urlpri;
		}
				
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
					if(pPktHdr->addPureL2Flow && (rg_db.systemGlobal.acl_SW_table_entry[acl_idx].hw_aclEntry_start || rg_db.systemGlobal.acl_SW_table_entry[acl_idx].hw_aclEntry_size))
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
			else if((pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_TRAP_WITH_PRIORITY)||(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_TRAP2SLAVE_WITH_PRIORITY))
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
					pPktHdr->aclDecision.action_egr_encap_udp=pChkRule->acl_filter.action_encap_udp;
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
							if(!(rg_db.systemGlobal.acl_SW_table_entry[acl_idx].hw_aclEntry_start || rg_db.systemGlobal.acl_SW_table_entry[acl_idx].hw_aclEntry_size)) {
								pPktHdr->swFlowOnlyByAcl = 1;
								TRACE("[QoS] ACL[%d] packet only add to sw flow",acl_idx);
							}
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
							if(!(rg_db.systemGlobal.acl_SW_table_entry[acl_idx].hw_aclEntry_start || rg_db.systemGlobal.acl_SW_table_entry[acl_idx].hw_aclEntry_size)) {
								pPktHdr->swFlowOnlyByAcl = 1;
								TRACE("[QoS] ACL[%d] packet only add to sw flow",acl_idx);
							}
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
					if(rg_db.systemGlobal.forcedlyCFpattern1Override || (pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_SVLAN_ACT_DONE_BIT)==0x0)//RG_EGR_SVLAN_ACT_DONE_BIT have not been set
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
					if(rg_db.systemGlobal.forcedlyCFpattern1Override || (pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_CVLAN_ACT_DONE_BIT)==0x0)//RG_EGR_CVLAN_ACT_DONE_BIT have not been set
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
					if(rg_db.systemGlobal.forcedlyCFpattern1Override || (pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_CFPRI_ACT_DONE_BIT)==0x0)//RG_EGR_CFPRI_ACT_DONE_BIT have not been set
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
					if(rg_db.systemGlobal.forcedlyCFpattern1Override || (pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_DSCP_ACT_DONE_BIT)==0x0)//RG_EGR_DSCP_ACT_DONE_BIT have not been set
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
					if(rg_db.systemGlobal.forcedlyCFpattern1Override || (pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_FWD_ACT_DONE_BIT)==0x0)//RG_EGR_FWD_ACT_DONE_BIT have not been set
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
					if(rg_db.systemGlobal.forcedlyCFpattern1Override || (pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_SID_ACT_DONE_BIT)==0x0)//RG_EGR_SID_ACT_DONE_BIT have not been set
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
					if(rg_db.systemGlobal.forcedlyCFpattern1Override || (pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_SVLAN_ACT_DONE_BIT)==0x0)//RG_EGR_SVLAN_ACT_DONE_BIT have not been set
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
					if(rg_db.systemGlobal.forcedlyCFpattern1Override || (pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_CVLAN_ACT_DONE_BIT)==0x0)//RG_EGR_CVLAN_ACT_DONE_BIT have not been set
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
					if(rg_db.systemGlobal.forcedlyCFpattern1Override || (pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_CFPRI_ACT_DONE_BIT)==0x0)//RG_EGR_CFPRI_ACT_DONE_BIT have not been set
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
					if(rg_db.systemGlobal.forcedlyCFpattern1Override || (pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_DSCP_ACT_DONE_BIT)==0x0)//RG_EGR_DSCP_ACT_DONE_BIT have not been set
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
					if(rg_db.systemGlobal.forcedlyCFpattern1Override || (pPktHdr->aclDecision.aclEgrDoneAction&RG_EGR_FWD_ACT_DONE_BIT)==0x0)//RG_EGR_FWD_ACT_DONE_BIT have not been set
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
										pPktHdr->egressPriority = pPktHdr->ctagPri;
										TRACE("Modify by RG_ACL ACT:CPRI_COPY_FROM_2ND_TAG CPRI=%d",pPktHdr->egressPriority);
									}else{
										//no 2nd tag, use assignedCpri as H/W
										pPktHdr->egressPriority = pPktHdr->aclDecision.action_acl_cvlan.assignedCpri;
										TRACE("Modify by RG_ACL ACT:CPRI_COPY_FROM_2ND_TAG(none tag) CPRI=%d",pPktHdr->egressPriority);
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
											pPktHdr->egressPriority = pPktHdr->ctagPri;
											TRACE("Modify by RG_CF ACT:CPRI_COPY_FROM_2ND_TAG CPRI=%d",pPktHdr->egressPriority);
										}else{
											//no 2nd tag, use assignedCpri as H/W
											pPktHdr->egressPriority = pPktHdr->aclDecision.action_cvlan.assignedCpri;
											TRACE("Modify by RG_CF ACT:CPRI_COPY_FROM_2ND_TAG(none tag) CPRI=%d",pPktHdr->egressPriority);
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
											pPktHdr->egressPriority = pPktHdr->ctagPri;
											TRACE("Modify by RG_CF ACT:CPRI_COPY_FROM_2ND_TAG CPRI=%d",pPktHdr->egressPriority);
										}else{
											//no 2nd tag, use assignedCpri as H/W
											pPktHdr->egressPriority = pPktHdr->aclDecision.action_cvlan.assignedCpri;
											TRACE("Modify by RG_CF ACT:CPRI_COPY_FROM_2ND_TAG(none tag) CPRI=%d",pPktHdr->egressPriority);
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
			PROC_PRINTF("[hw_acl_start:%d(continue:%d) \n"
				,rg_db.systemGlobal.acl_SW_table_entry[i].hw_aclEntry_start
				,rg_db.systemGlobal.acl_SW_table_entry[i].hw_aclEntry_size);

			PROC_PRINTF("[Using range tables]: \n");
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_INGRESS_SIP4TABLE) PROC_PRINTF("ACL_SIP4_RANGE_TABLE[%d]  \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_SIP4TABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_INGRESS_DIP4TABLE) PROC_PRINTF("ACL_DIP4_RANGE_TABLE[%d]  \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_DIP4TABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_INGRESS_SIP6TABLE) PROC_PRINTF("ACL_SIP6_RANGE_TABLE[%d]  \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_SIP6TABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_INGRESS_DIP6TABLE) PROC_PRINTF("ACL_DIP6_RANGE_TABLE[%d]  \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_DIP6TABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_INGRESS_SPORTTABLE) PROC_PRINTF("ACL_SPORT_RANGE_TABLE[%d]  \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_SPORTTABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_INGRESS_DPORTTABLE) PROC_PRINTF("ACL_DPORT_RANGE_TABLE[%d]  \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_DPORTTABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_EGRESS_SIP4TABLE) PROC_PRINTF("CF_SIP4_RANGE_TABLE[%d] \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_EGRESS_SIP4TABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_EGRESS_DIP4TABLE) PROC_PRINTF("CF_DIP4_RANGE_TABLE[%d] \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_EGRESS_DIP4TABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_EGRESS_SIP6TABLE) PROC_PRINTF("CF_DSCP_TABLE[%d] \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_EGRESS_SIP6TABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_EGRESS_DIP6TABLE) PROC_PRINTF("CF_DSCP_TABLE[%d] \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_EGRESS_DIP6TABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_EGRESS_SPORTTABLE) PROC_PRINTF("CF_SPORT_RANGE_TABLE[%d] \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_EGRESS_SPORTTABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_EGRESS_DPORTTABLE) PROC_PRINTF("CF_DPORT_RANGE_TABLE[%d] \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_EGRESS_DPORTTABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_INGRESS_PKTLENTABLE) PROC_PRINTF(" ACL_PKTLEN_RANGE_TABLE[%d]  \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_PKTLENTABLE_INDEX]);
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
				PROC_PRINTF("[SVLAN] svlan: %s\n  svid_act:%s\n  spri_act:%s\n  svid:%d\n  spri:%d\n",
					name_of_rg_svlan_tagif_decision[classifyFilter->action_svlan.svlanTagIfDecision],
					name_of_rg_svlan_svid_decision[classifyFilter->action_svlan.svlanSvidDecision],
					name_of_rg_svlan_spri_decision[classifyFilter->action_svlan.svlanSpriDecision],
					classifyFilter->action_svlan.assignedSvid,
					classifyFilter->action_svlan.assignedSpri
				);
			}
			if(classifyFilter->us_action_field&CF_US_ACTION_CTAG_BIT)
			{
				PROC_PRINTF("[CVLAN] cvlan: %s\n  cvid_act:%s\n  cpri_act:%s\n  cvid:%d\n  cpri:%d\n",
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
				PROC_PRINTF("[SVLAN] svlan: %s\n  svid_act:%s\n  spri_act:%s\n  svid:%d\n  spri:%d\n",
					name_of_rg_svlan_tagif_decision[classifyFilter->action_svlan.svlanTagIfDecision],
					name_of_rg_svlan_svid_decision[classifyFilter->action_svlan.svlanSvidDecision],
					name_of_rg_svlan_spri_decision[classifyFilter->action_svlan.svlanSpriDecision],
					classifyFilter->action_svlan.assignedSvid,
					classifyFilter->action_svlan.assignedSpri
				);
			}
			if(classifyFilter->ds_action_field&CF_DS_ACTION_CTAG_BIT)
			{
				PROC_PRINTF("[CVLAN] cvlan: %s\n  cvid_act:%s\n  cpri_act:%s\n  cvid:%d\n  cpri:%d\n",
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

	if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_type==ACL_ACTION_TYPE_SW_MIRROR_WITH_UDP_ENCAP) 
	{
		if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.no_encap){
			PROC_PRINTF("rg set acl-filter action udp_encap no_encap 1 mirror_count %d",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.mirror_count);
			PROC_PRINTF("%s\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.remake_smac?" remake_smac":"");
		}else if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.mirror_count==FAIL){			
			if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_ip_ipv6)
			{
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
			}
			else
			{
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
			PROC_PRINTF("rg set acl-filter action udp_encap tcp_header_only %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.tcp_header_only);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.tcp_rule_append)
				PROC_PRINTF("rg set acl-filter action udp_encap tcp_rule_idx %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.tcp_rule_idx);
		}
		else
		{
			if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_ip_ipv6)
			{
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
			}
			else
			{
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
			PROC_PRINTF("rg set acl-filter action udp_encap tcp_header_only %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.tcp_header_only);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.tcp_rule_append)
				PROC_PRINTF("rg set acl-filter action udp_encap tcp_rule_idx %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.tcp_rule_idx);
		}
	}

	if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_type==ACL_ACTION_TYPE_TRAP_WITH_PRIORITY)
		PROC_PRINTF("rg set acl-filter action trap_with_priority acl_priority %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_trap_with_priority);

	if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_type==ACL_ACTION_TYPE_TRAP2SLAVE_WITH_PRIORITY)
		PROC_PRINTF("rg set acl-filter action trap2slave_with_priority acl_priority %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_trap_with_priority);

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

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_TCP_FLAGS_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_tcp_flags  0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_tcp_flags);
			PROC_PRINTF("rg set acl-filter pattern ingress_tcp_flags_mask  0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_tcp_flags_mask);
			//hardwarelize pattern, not support reverse check
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_PKT_LEN_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_packet_length_start %d ingress_packet_length_end %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_packet_length_start, rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_packet_length_end);
			//hardwarelize pattern, not support reverse check
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


int _dump_acl(struct seq_file *s)
{
#if !defined(CONFIG_RG_G3_SERIES)
	int i,j;
	rtk_acl_ingress_entry_t aclRule;
	char* actionString;
    rtk_acl_template_t aclTemplate;
	rtk_acl_debug_reason_t pDbgReason;

	i=0;j=0;

	PROC_PRINTF("+++ wanPortMask = 0x%x, lanPortMask = 0x%x +++\n", rg_db.systemGlobal.wanPortMask.portmask, rg_db.systemGlobal.lanPortMask.portmask);
	PROC_PRINTF("--------------- ACL TABLES ----------------\n");
	for(i=0; i<MAX_ACL_ENTRY_SIZE; i++)
	{
		memset(&aclRule,0,sizeof(aclRule));
		aclRule.index=i;
		rtk_acl_igrRuleEntry_get(&aclRule);
		if(aclRule.valid)
		{
			int tag_care=0;
			PROC_PRINTF("  --- ACL TABLE[%d] ---\n",i);
			PROC_PRINTF("\tvalid:%x\n",aclRule.valid);

			for(j=0; j<8; j++)
			{
				if(aclRule.readField.fieldRaw[j].mask)
				{
					PROC_PRINTF("\tfield[%d]:0x%04x  mask[%d]:0x%04x\t",j,aclRule.readField.fieldRaw[j].value,j,aclRule.readField.fieldRaw[j].mask);
					memset(&aclTemplate,0,sizeof(aclTemplate));
					aclTemplate.index=aclRule.templateIdx;
					rtk_acl_template_get(&aclTemplate);
					PROC_PRINTF("{0x%02x:%s}\n",aclTemplate.fieldType[j],name_of_acl_field[aclTemplate.fieldType[j]]);
				}
			}

			PROC_PRINTF("\tactive portmask:0x%x\n",aclRule.activePorts.bits[0]);

			if(aclRule.careTag.tags[2].mask) tag_care|=(1<<2);
			if(aclRule.careTag.tags[1].mask) tag_care|=(1<<1);
			if(tag_care)
			{
				PROC_PRINTF("\ttag_care:");
				if(tag_care&(1<<2))PROC_PRINTF("%s",aclRule.careTag.tags[2].value?"[Stag:O]":"[Stag:X]");
				if(tag_care&(1<<1))PROC_PRINTF("%s",aclRule.careTag.tags[1].value?"[Ctag:O]":"[Ctag:X]");
				PROC_PRINTF("\n");
			}

			PROC_PRINTF("\ttemplateIdx:%x\n",aclRule.templateIdx);

			{
				PROC_PRINTF("\taction bits:");

				if(aclRule.act.enableAct[5]) PROC_PRINTF("[INT/CF]");
				if(aclRule.act.enableAct[4]) PROC_PRINTF("[FWD]");
				if(aclRule.act.enableAct[3]) PROC_PRINTF("[POLICY/LOG]");
				if(aclRule.act.enableAct[2]) PROC_PRINTF("[PRI]");
				if(aclRule.act.enableAct[1]) PROC_PRINTF("[SVLAN]");
				if(aclRule.act.enableAct[0]) PROC_PRINTF("[CVLAN]");
				PROC_PRINTF("\n");

				if(aclRule.act.enableAct[0])
				{
					switch(aclRule.act.cvlanAct.act)
					{
						case ACL_IGR_CVLAN_IGR_CVLAN_ACT: actionString="Ingress CVLAN action"; break;
						case ACL_IGR_CVLAN_EGR_CVLAN_ACT: actionString="Egress CVLAN action";break;
						case ACL_IGR_CVLAN_DS_SVID_ACT: actionString="Using SVID";break;
						case ACL_IGR_CVLAN_POLICING_ACT: actionString="Policing";break;
						case ACL_IGR_CVLAN_1P_REMARK_ACT: actionString="1P remark";break;
						case ACL_IGR_CVLAN_BW_METER_ACT: actionString="Bandwidth Metering"; break;
						default: actionString="unKnown action"; break;
					}
					PROC_PRINTF("\t[CVLAN_ACTIDX:%x(%s)] cvid:%d dot1p:%x\n",aclRule.act.cvlanAct.act,actionString,aclRule.act.cvlanAct.cvid,aclRule.act.cvlanAct.dot1p);
				}

				if(aclRule.act.enableAct[1])
				{
					switch(aclRule.act.svlanAct.act)
					{
						case ACL_IGR_SVLAN_IGR_SVLAN_ACT: actionString="Ingress SVLAN action"; break;
						case ACL_IGR_SVLAN_EGR_SVLAN_ACT:  actionString="Egress SVLAN action"; break;
						case ACL_IGR_SVLAN_US_CVID_ACT:  actionString="Using CVID"; break;
						case ACL_IGR_SVLAN_POLICING_ACT:  actionString="Policing"; break;
						case ACL_IGR_SVLAN_MIB_ACT: actionString="Logging";break;
						case ACL_IGR_SVLAN_1P_REMARK_ACT:  actionString="1P remark"; break;
						case ACL_IGR_SVLAN_DSCP_REMARK_ACT:
							actionString="DSCP remark";
							//pPktHdr->egressDSCPRemarking = ENABLED_DSCP_REMARK_AND_SRC_FROM_ACL;
							//pPktHdr->egressDSCP = pPktHdr->aclDecision.action_dscp_remarking_pri;
							break;
						case ACL_IGR_SVLAN_BW_METER_ACT:  actionString="Bandwidth Metering"; break;
						default: actionString="unKnown action"; break;
					}
					PROC_PRINTF("\t[SVLAN_ACTIDX:%x(%s)] svid:%d dot1p:%x dscp:%d nexthop:%x\n",aclRule.act.svlanAct.act,actionString,aclRule.act.svlanAct.svid,aclRule.act.svlanAct.dot1p,aclRule.act.svlanAct.dscp,aclRule.act.svlanAct.nexthop);
				}

				if(aclRule.act.enableAct[4])
				{
					switch(aclRule.act.forwardAct.act)
					{
						case ACL_IGR_FORWARD_EGRESSMASK_ACT: actionString="Forward frame with ACLPMSK only (& filtering)"; break;
						case ACL_IGR_FORWARD_REDIRECT_ACT: actionString="Redirect frame with ACLPMSK"; break;
						case ACL_IGR_FORWARD_IGR_MIRROR_ACT: actionString="Ingress mirror to ACLPMSK"; break;
						case ACL_IGR_FORWARD_TRAP_ACT: actionString="Trap to CPU"; break;
						default: actionString="unKnown action"; break;
					}

					PROC_PRINTF("\t[FWD_ACTIDX:%x(%s)] portMask:0x%x\n",aclRule.act.forwardAct.act,actionString,aclRule.act.forwardAct.portMask.bits[0]);
				}

				if(aclRule.act.enableAct[2])
				{
					switch(aclRule.act.priAct.act)
					{
						case ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT: actionString="ACL Priority";break;
						case ACL_IGR_PRI_DSCP_REMARK_ACT:
							actionString="DSCP Remarking";
							//pPktHdr->egressDSCPRemarking = ENABLED_DSCP_REMARK_AND_SRC_FROM_ACL;
							//pPktHdr->egressDSCP = pPktHdr->aclDecision.action_dscp_remarking_pri;
							break;
						case ACL_IGR_PRI_1P_REMARK_ACT: actionString="1P Remarking";break;
						case ACL_IGR_PRI_POLICING_ACT: actionString="Policing";break;
						case ACL_IGR_PRI_MIB_ACT: actionString="Logging";break;
						case ACL_IGR_PRI_BW_METER_ACT: actionString="Bandwidth Metering";break;
						case ACL_IGR_PRI_TOS_REMARK_ACT: actionString="ToS remarking";break;
						default: actionString="unKnown action"; break;
					}
					PROC_PRINTF("\t[PRI_ACTIDX:%x(%s)] aclPri:%x dot1p:%x dscp:%d Tos:%d(TosMask:0x%x)\n",aclRule.act.priAct.act,actionString,aclRule.act.priAct.aclPri,aclRule.act.priAct.dot1p,aclRule.act.priAct.dscp,aclRule.act.priAct.tos,aclRule.act.priAct.tosMask);
				}

				if(aclRule.act.enableAct[3])
				{
					switch(aclRule.act.logAct.act)
					{
						case ACL_IGR_LOG_POLICING_ACT: actionString="Policing"; break;
						case ACL_IGR_LOG_MIB_ACT: actionString="Logging"; break;
						case ACL_IGR_LOG_BW_METER_ACT: actionString="Bandwidth Metering"; break;
						case ACL_IGR_LOG_1P_REMARK_ACT: actionString="1P remark"; break;
						default: actionString="unKnown action"; break;
					}
					PROC_PRINTF("\t[POLICY/LOG_ACTIDX:%x(%s)] meteridx:%d\n",aclRule.act.logAct.act,actionString, aclRule.act.logAct.act==ACL_IGR_LOG_MIB_ACT? aclRule.act.logAct.mib:aclRule.act.logAct.meter);
				}

				if(aclRule.act.enableAct[5])
				{
					switch(aclRule.act.extendAct.act)
					{
						case ACL_IGR_EXTEND_NONE_ACT: actionString="None"; break;
						case ACL_IGR_EXTEND_SID_ACT: actionString="Stream ID assign"; break;
						case ACL_IGR_EXTEND_LLID_ACT: actionString="LLID"; break;
						case ACL_IGR_EXTEND_EXT_ACT: actionString="Ext Act(not support in apolloFE)"; break;
						case ACL_IGR_EXTEND_1P_REMARK_ACT: actionString="1P Remarking"; break;
						default: actionString="unKnown action"; break;
					}
					PROC_PRINTF("\t[INT/CF_ACTIDX:%x(%s)] CFHITLATCH:%x INT:%x index(stream_id or llid):0x%x, pmask:0x%x\n",aclRule.act.extendAct.act,actionString,aclRule.act.aclLatch, aclRule.act.aclInterrupt,aclRule.act.extendAct.index,aclRule.act.extendAct.portMask.bits[0]);
				}

			}

	   }
	}


	//use the ASIC API
	PROC_PRINTF("--------------- ACL HIT OINFO----------------\n");

	memset(&pDbgReason,0,sizeof(pDbgReason));
	rtk_acl_dbgHitReason_get(&pDbgReason);

	PROC_PRINTF("[CACT:%s]: hit rule %d\n",pDbgReason.hitAct[ACL_IGR_CVLAN_ACT]?"O":"X",pDbgReason.index[ACL_IGR_CVLAN_ACT]);
	PROC_PRINTF("[SACT:%s]: hit rule %d\n",pDbgReason.hitAct[ACL_IGR_SVLAN_ACT]?"O":"X",pDbgReason.index[ACL_IGR_SVLAN_ACT]);
	PROC_PRINTF("[PRI:%s]: hit rule %d\n",pDbgReason.hitAct[ACL_IGR_PRI_ACT]?"O":"X",pDbgReason.index[ACL_IGR_PRI_ACT]);
	PROC_PRINTF("[POLICE:%s]: hit rule %d\n",pDbgReason.hitAct[ACL_IGR_LOG_ACT]?"O":"X",pDbgReason.index[ACL_IGR_LOG_ACT]);
	PROC_PRINTF("[FWD:%s]: hit rule %d\n",pDbgReason.hitAct[ACL_IGR_FORWARD_ACT]?"O":"X",pDbgReason.index[ACL_IGR_FORWARD_ACT]);
	PROC_PRINTF("[INT:%s]: hit rule %d\n",pDbgReason.hitAct[ACL_IGR_INTR_ACT]?"O":"X",pDbgReason.index[ACL_IGR_INTR_ACT]);
#endif
	return RT_ERR_RG_OK;
}



int _dump_cf(struct seq_file *s)
{
	uint32 val;

	int i;
	//rtk_dscp_t dscp_entry;
	int dscp=0;
	rtk_classify_cfg_t classifyCfg;
	//rtk_classify_rangeCheck_l4Port_t prtRangeEntry;
	//rtk_classify_rangeCheck_ip_t ipRangeEntry;
	char *decisionString, *vidDecisionString, *priDecisionString;
	val=0;

#if 0	//ApolloPro not used
	PROC_PRINTF("------------ CF PORT RANGE TABLE -------------\n");
	for(i=0; i<8; i++)
	{
		memset(&prtRangeEntry,0,sizeof(prtRangeEntry));
		prtRangeEntry.index=i;
		rtk_classify_portRange_get(&prtRangeEntry);
		PROC_PRINTF("\tPORTRANGE[%d] upper:%d lower:%d type:0x%x\n",i,prtRangeEntry.upperPort,prtRangeEntry.lowerPort,prtRangeEntry.type);

	}

	PROC_PRINTF("------------ CF IP RANGE TABLE -------------\n");
	for(i=0; i<8; i++)
	{
		memset(&ipRangeEntry,0,sizeof(ipRangeEntry));
		ipRangeEntry.index=i;
		rtk_classify_ipRange_get(&ipRangeEntry);
		PROC_PRINTF("\tIPRANGE[%d] upper:0x%x lower:0x%x type:0x%x\n",i,ipRangeEntry.upperIp,ipRangeEntry.lowerIp,ipRangeEntry.type);
	}

	PROC_PRINTF("------------ CF DSCP TABLE -------------\n");
	for(i=0; i<8; i++)
	{
		rtk_classify_cfPri2Dscp_get(i,&dscp_entry);
		PROC_PRINTF("\tDSCP[%d]:0x%x\n",i,dscp_entry);
	}
#endif

	PROC_PRINTF("------------ CF TABLE -------------\n");
	for(i=0; i<512; i++)
	{
		memset(&classifyCfg,0,sizeof(classifyCfg));
		classifyCfg.index=i;
		rtk_classify_cfgEntry_get(&classifyCfg);
		if(classifyCfg.valid)
		{
			PROC_PRINTF("  ------ CF TABLE[%d] ------\n",i);
			//PROC_PRINTF("\tvalid:%x\n",classifyCfg.valid);


			//parse U/D
			PROC_PRINTF("\tDIRECTION: %s\n",classifyCfg.direction?"DOWNSTREAM":"UPSTREAM");

			//PROC_PRINTF("\tfield[2]:%x mask:%x\n",classifyCfg.field.readField.dataFieldRaw[2],classifyCfg.field.readField.careFieldRaw[2]);
			//PROC_PRINTF("\tfield[1]:%x mask:%x\n",classifyCfg.field.readField.dataFieldRaw[1],classifyCfg.field.readField.careFieldRaw[1]);
			//PROC_PRINTF("\tfield[0]:%x mask:%x\n",classifyCfg.field.readField.dataFieldRaw[0],classifyCfg.field.readField.careFieldRaw[0]);


			if(i<64){//L34 patterns
				//parse PortRange
				if((classifyCfg.field.readField.dataFieldRaw[2]>>12)&0x8)
				{
					PROC_PRINTF("\tPORT: must hit TABLE[%d]\n",(classifyCfg.field.readField.dataFieldRaw[2]>>12)&0x7);
				}


				//parse IpRange
				if((classifyCfg.field.readField.dataFieldRaw[2]>>8)&0x8)
				{
					PROC_PRINTF("\tIP: must hit TABLE[%d]\n",(classifyCfg.field.readField.dataFieldRaw[2]>>8)&0x7);
				}

				//parse Acl Hit
				if((classifyCfg.field.readField.dataFieldRaw[2]>>0)&0x80)
				{
					PROC_PRINTF("\tACL: must hit ACL[%d]\n",(classifyCfg.field.readField.dataFieldRaw[2]>>0)&0x7f);
				}

				//parse Wan Interface
				PROC_PRINTF("\tWAN_IF:%d \t(Mask:0x%x)\n",(classifyCfg.field.readField.dataFieldRaw[1]>>12)&0x7,(classifyCfg.field.readField.careFieldRaw[1]>>12)&0x7);


				//parse IPv6_MC
				if((classifyCfg.field.readField.careFieldRaw[1]>>11)&0x1)
					PROC_PRINTF("\tIPv6_MC:%s\n", ((classifyCfg.field.readField.dataFieldRaw[1]>>11)&0x1)?"Must":"Must Not");
				//parse IPv4_MC
				if( (classifyCfg.field.readField.careFieldRaw[1]>>10)&0x1)
					PROC_PRINTF("\tIPv4_MC:%s\n", ((classifyCfg.field.readField.dataFieldRaw[1]>>10)&0x1)?"Must":"Must Not");
				//parse MLD
				if( (classifyCfg.field.readField.careFieldRaw[1]>>9)&0x1)
					PROC_PRINTF("\tMLD:%s\n", ((classifyCfg.field.readField.dataFieldRaw[1]>>9)&0x1)?"Must":"Must Not");
				//parse IGMP
				if((classifyCfg.field.readField.careFieldRaw[1]>>8)&0x1)
					PROC_PRINTF("\tIGMP:%s\n", ((classifyCfg.field.readField.dataFieldRaw[1]>>8)&0x1)?"Must":"Must Not");
				//parse DEI
				if((classifyCfg.field.readField.careFieldRaw[1]>>7)&0x1){
					PROC_PRINTF("\tDEI:%d \t(Mask:0x%x)\n",(classifyCfg.field.readField.dataFieldRaw[1]>>7)&0x1,(classifyCfg.field.readField.careFieldRaw[1]>>7)&0x1);
				}
			}else{//L2 patterns
				//parse Ethertype
				if(classifyCfg.field.readField.careFieldRaw[2]){
					 PROC_PRINTF("\tEhtertype:0x%x \t(Mask:0x%x)\n",classifyCfg.field.readField.dataFieldRaw[2],classifyCfg.field.readField.careFieldRaw[2]);

				}
				//parse GemIdx or LLID
				if((classifyCfg.field.readField.careFieldRaw[1]>>7)&0xff){
					 PROC_PRINTF("\tGemIdx/LLID/TOS:0x%x \t(Mask:0x%x)\n",(classifyCfg.field.readField.dataFieldRaw[1]>>7)&0xff,(classifyCfg.field.readField.careFieldRaw[1]>>7)&0xff);
				}

			}


			//parse VID
			if((((classifyCfg.field.readField.careFieldRaw[1]>>0)&0x7f)<<5) | ((classifyCfg.field.readField.careFieldRaw[0])>>11 &0x1f)){
				PROC_PRINTF("\tVID:%d \t(Mask:0x%x)\n",(((classifyCfg.field.readField.dataFieldRaw[1]>>0)&0x7f)<<5) | ((classifyCfg.field.readField.dataFieldRaw[0])>>11 &0x1f),(((classifyCfg.field.readField.careFieldRaw[1]>>0)&0x7f)<<5) | ((classifyCfg.field.readField.careFieldRaw[0])>>11 &0x1f));
			}

			//parse PRI
			if((classifyCfg.field.readField.careFieldRaw[0]>>8)&0x7){
				PROC_PRINTF("\tPRI:%d \t\t(Mask:0x%x)\n",(classifyCfg.field.readField.dataFieldRaw[0]>>8)&0x7,(classifyCfg.field.readField.careFieldRaw[0]>>8)&0x7);
			}

			//parse INTER_PRI
			if((classifyCfg.field.readField.careFieldRaw[0]>>5)&0x7){
				PROC_PRINTF("\tINTER_PRI:%d \t(Mask:0x%x)\n",(classifyCfg.field.readField.dataFieldRaw[0]>>5)&0x7,(classifyCfg.field.readField.careFieldRaw[0]>>5)&0x7);
			}

			//parse STAG, CTAG
			if((classifyCfg.field.readField.careFieldRaw[0]>>4)&0x1)
				PROC_PRINTF("\tSTAG_IF:%s \n",((classifyCfg.field.readField.dataFieldRaw[0]>>4)&0x1)?"Must Stagged":"Must don't have Stag");

			if((classifyCfg.field.readField.careFieldRaw[0]>>3)&0x1)
				PROC_PRINTF("\tCTAG_IF:%s \n",((classifyCfg.field.readField.dataFieldRaw[0]>>3)&0x1)?"Must Ctagged":"Must don't have Ctag");


			//parse UNI
			if((classifyCfg.field.readField.careFieldRaw[0]>>0)&CF_UNI_DEFAULT){
				PROC_PRINTF("\tUNI:%d \t\t(Mask:0x%x)\n",(classifyCfg.field.readField.dataFieldRaw[0]>>0)&CF_UNI_DEFAULT,(classifyCfg.field.readField.careFieldRaw[0]>>0)&CF_UNI_DEFAULT);
			}

#if 1
			if(classifyCfg.direction) //downstream
			{

				//[SACT]
				switch(classifyCfg.act.dsAct.csAct)
				{
					case CLASSIFY_DS_CSACT_NOP: decisionString="NOP"; break;
					case CLASSIFY_DS_CSACT_ADD_TAG_VS_TPID: decisionString="ADD_TAG_VS_TPID"; break;
					case CLASSIFY_DS_CSACT_ADD_TAG_8100: decisionString="ADD_TAG_8100"; break;
					case CLASSIFY_DS_CSACT_DEL_STAG: decisionString="DEL_STAG"; break;
					case CLASSIFY_DS_CSACT_TRANSPARENT: decisionString="TRANSPARENT"; break;
					case CLASSIFY_DS_CSACT_SP2C: decisionString="SP2C"; break;
					case CLASSIFY_DS_CSACT_ADD_TAG_STAG_TPID: decisionString="ADD_TAG_STAG_TPID"; break;
					case CLASSIFY_DS_CSACT_ACTCTRL_DISABLE: decisionString="ACTCTRL_DISABLE"; break;
					case CLASSIFY_DS_CSACT_ADD_TAG_VS_TPID2: decisionString="ADD_TAG_VS_TPID2"; break;
					default:
						decisionString="unKnown decision";
						break;
				}
				switch(classifyCfg.act.dsAct.csVidAct)
				{
					case CLASSIFY_DS_VID_ACT_ASSIGN: vidDecisionString="ASSIGN"; break;
					case CLASSIFY_DS_VID_ACT_FROM_1ST_TAG: vidDecisionString="FROM_1ST_TAG"; break;
					case CLASSIFY_DS_VID_ACT_FROM_2ND_TAG: vidDecisionString="FROM_2ND_TAG"; break;
					case CLASSIFY_DS_VID_ACT_FROM_LUT: vidDecisionString="FROM_LUT"; break;
					case CLASSIFY_DS_VID_ACT_TRANSLATION_SP2C: vidDecisionString="FROM_SP2C"; break;
					case CLASSIFY_DS_VID_ACT_NOP: vidDecisionString="NOP"; break;
					default:
						vidDecisionString="unKnown decision";
						break;
				}
				switch(classifyCfg.act.dsAct.csPriAct)
				{
					case CLASSIFY_DS_PRI_ACT_ASSIGN: priDecisionString="ASSIGN"; break;
					case CLASSIFY_DS_PRI_ACT_FROM_1ST_TAG: priDecisionString="FROM_1ST_TAG"; break;
					case CLASSIFY_DS_PRI_ACT_FROM_2ND_TAG: priDecisionString="FROM_2ND_TAG"; break;
					case CLASSIFY_DS_PRI_ACT_FROM_INTERNAL: priDecisionString="FROM_INTERNAL"; break;
					case CLASSIFY_DS_PRI_ACT_TRANSLATION_SP2C: priDecisionString="FROM_SP2C"; break;
					case CLASSIFY_DS_PRI_ACT_FROM_DSCP: priDecisionString="FROM_DSCP"; break;
					case CLASSIFY_DS_PRI_ACT_NOP: priDecisionString="NOP"; break;
					default:
						priDecisionString="unKnown decision";
						break;

				}

				if(classifyCfg.act.dsAct.csAct!=CLASSIFY_DS_CSACT_ACTCTRL_DISABLE){
					PROC_PRINTF("\t[SACT]:%s \n\t\tSVID_ACT:%s \tSVID:%d \n\t\tSPRI_ACT:%s \tSPRI:%d \n",
					decisionString,
					vidDecisionString,classifyCfg.act.dsAct.sTagVid,
					priDecisionString,classifyCfg.act.dsAct.sTagPri);
				}


				//[CACT]
				switch(classifyCfg.act.dsAct.cAct)
				{
					case CLASSIFY_DS_CACT_NOP: decisionString="NOP"; break;
					case CLASSIFY_DS_CACT_ADD_CTAG_8100: decisionString="ADD_CTAG_8100"; break;
					case CLASSIFY_DS_CACT_TRANSLATION_SP2C: decisionString="SP2C"; break;
					case CLASSIFY_DS_CACT_DEL_CTAG: decisionString="DEL_CTAG"; break;
					case CLASSIFY_DS_CACT_TRANSPARENT: decisionString="TRANSPARENT"; break;
					case CLASSIFY_DS_CACT_ACTCTRL_DISABLE: decisionString="TRANSPARENT"; break;
					default:
						decisionString="unKnown decision";
						break;
				}
				switch(classifyCfg.act.dsAct.cVidAct)
				{
					case CLASSIFY_DS_VID_ACT_ASSIGN: vidDecisionString="ASSIGN"; break;
					case CLASSIFY_DS_VID_ACT_FROM_1ST_TAG: vidDecisionString="FROM_1ST_TAG"; break;
					case CLASSIFY_DS_VID_ACT_FROM_2ND_TAG: vidDecisionString="FROM_2ND_TAG"; break;
					case CLASSIFY_DS_VID_ACT_FROM_LUT: vidDecisionString="FROM_LUT"; break;
					case CLASSIFY_DS_VID_ACT_TRANSLATION_SP2C: vidDecisionString="SP2C"; break;
					case CLASSIFY_DS_VID_ACT_NOP: vidDecisionString="NOP"; break;
					default:
						vidDecisionString="unKnown decision";
						break;
				}
				switch(classifyCfg.act.dsAct.cPriAct)
				{
					case CLASSIFY_DS_PRI_ACT_ASSIGN: priDecisionString="ASSIGN"; break;
					case CLASSIFY_DS_PRI_ACT_FROM_1ST_TAG: priDecisionString="FROM_1ST_TAG"; break;
					case CLASSIFY_DS_PRI_ACT_FROM_2ND_TAG: priDecisionString="FROM_2ND_TAG"; break;
					case CLASSIFY_DS_PRI_ACT_FROM_INTERNAL: priDecisionString="FROM_INTERNAL"; break;
					case CLASSIFY_DS_PRI_ACT_TRANSLATION_SP2C: priDecisionString="SP2C"; break;
					case CLASSIFY_DS_PRI_ACT_FROM_DSCP: priDecisionString="FROM_DSCP"; break;
					case CLASSIFY_DS_PRI_ACT_NOP: priDecisionString="NOP"; break;
					default:
						priDecisionString="unKnown decision";
						break;
				}

				if(classifyCfg.act.dsAct.cAct!=CLASSIFY_DS_CACT_ACTCTRL_DISABLE)
				{
					PROC_PRINTF("\t[CACT]:%s \n\t\tCVID_ACT:%s \tVID:%d \n\t\tCPRI_ACT:%s \tPRI:%d \n",
					decisionString,
					vidDecisionString,classifyCfg.act.dsAct.cTagVid,
					priDecisionString,classifyCfg.act.dsAct.cTagPri);
				}

				//[CFPRI]
				switch(classifyCfg.act.dsAct.interPriAct)
				{
					case CLASSIFY_CF_PRI_ACT_NOP: decisionString="NOP"; break;
					case CLASSIFY_CF_PRI_ACT_ASSIGN: decisionString="ASSIGN"; break;
					case CLASSIFY_CF_PRI_ACT_ACTCTRL_DISABLE: decisionString="ACTCTRL_DISABLE"; break;
					default:
						decisionString="unKnown decision";
						break;
				}

				if(classifyCfg.act.dsAct.interPriAct!=CLASSIFY_CF_PRI_ACT_ACTCTRL_DISABLE)
				{
				PROC_PRINTF("\t[CFPRI]:%s \n\t\tCFPRI:%d\n",
				decisionString,
				classifyCfg.act.dsAct.cfPri);
			}

			//[DSCP]
			switch(classifyCfg.act.dsAct.dscp)
			{
				case CLASSIFY_DSCP_ACT_DISABLE: decisionString="DISABLE"; break;
				case CLASSIFY_DSCP_ACT_ENABLE: decisionString="ENABLE"; break;
				case CLASSIFY_DSCP_ACT_ACTCTRL_DISABLE: decisionString="ACTCTRL_DISABLE"; break;
				default:
					decisionString="unKnown decision";
					break;
			}

			if(classifyCfg.act.dsAct.dscp!=CLASSIFY_DSCP_ACT_ACTCTRL_DISABLE)
			{
				rtk_classify_cfPri2Dscp_get(classifyCfg.act.usAct.cfPri, &dscp);
				PROC_PRINTF("\t[DSCP]:%s \n\t\tDSCP:%d \n",
				decisionString,
				dscp);
			}

			//[UNI]
			switch(classifyCfg.act.dsAct.uniAct)
			{
				case CLASSIFY_DS_UNI_ACT_NOP: decisionString="NOP"; break;
				case CLASSIFY_DS_UNI_ACT_MASK_BY_UNIMASK: decisionString="MASK_BY_UNIMASK (& filtering)"; break;
				case CLASSIFY_DS_UNI_ACT_FORCE_FORWARD: decisionString="FORCE_FORWARD"; break;
				case CLASSIFY_DS_UNI_ACT_TRAP: decisionString="TRAP"; break;
				case CLASSIFY_DS_UNI_ACT_ACTCTRL_DISABLE: decisionString="ACTCTRL_DISABLE"; break;
				default:
					decisionString="unKnown decision";
					break;
			}
			if(classifyCfg.act.dsAct.uniAct!=CLASSIFY_DS_UNI_ACT_ACTCTRL_DISABLE)
			{
				PROC_PRINTF("\t[UNI]:%s \n\t\tUNI_MASK:0x%x\n",
				decisionString,
				classifyCfg.act.dsAct.uniMask.bits[0]);
			}
		}
		else
		{
			//[SACT]
			switch(classifyCfg.act.usAct.csAct)
			{
				case CLASSIFY_US_CSACT_NOP: decisionString="NOP"; break;
				case CLASSIFY_US_CSACT_ADD_TAG_VS_TPID: decisionString="ADD_TAG_VS_TPID"; break;
				case CLASSIFY_US_CSACT_ADD_TAG_8100: decisionString="ADD_TAG_8100"; break;
				case CLASSIFY_US_CSACT_DEL_STAG: decisionString="DEL_STAG"; break;
				case CLASSIFY_US_CSACT_TRANSPARENT: decisionString="TRANSPARENT"; break;
				case CLASSIFY_US_CSACT_ADD_TAG_STAG_TPID: decisionString="ADD_TAG_STAG_TPID"; break;
				case CLASSIFY_US_CSACT_ACTCTRL_DISABLE: decisionString="ACTCTRL_DISABLE"; break;
				case CLASSIFY_US_CSACT_ADD_TAG_VS_TPID2: decisionString="ADD_TAG_VS_TPID2"; break;
				default:
					decisionString="unKnown decision";
					break;
			}
			switch(classifyCfg.act.usAct.csVidAct)
			{
				case CLASSIFY_US_VID_ACT_ASSIGN: vidDecisionString="ASSIGN"; break;
				case CLASSIFY_US_VID_ACT_FROM_1ST_TAG: vidDecisionString="FROM_1ST_TAG"; break;
				case CLASSIFY_US_VID_ACT_FROM_2ND_TAG: vidDecisionString="FROM_2ND_TAG"; break;
				case CLASSIFY_US_VID_ACT_FROM_INTERNAL: vidDecisionString="FROM_INTERNAL"; break;
				case CLASSIFY_US_VID_ACT_NOP: vidDecisionString="NOP"; break;
				default:
					vidDecisionString="unKnown decision";
					break;
			}
			switch(classifyCfg.act.usAct.csPriAct)
			{
				case CLASSIFY_US_PRI_ACT_ASSIGN: priDecisionString="ASSIGN"; break;
				case CLASSIFY_US_PRI_ACT_FROM_1ST_TAG: priDecisionString="FROM_1ST_TAG"; break;
				case CLASSIFY_US_PRI_ACT_FROM_2ND_TAG: priDecisionString="FROM_2ND_TAG"; break;
				case CLASSIFY_US_PRI_ACT_FROM_INTERNAL: priDecisionString="FROM_INTERNAL"; break;
				case CLASSIFY_US_PRI_ACT_FROM_DSCP: priDecisionString="FROM_DSCP"; break;
				case CLASSIFY_US_PRI_ACT_NOP: priDecisionString="NOP"; break;
				default:
					priDecisionString="unKnown decision";
					break;
			}

			if(classifyCfg.act.usAct.csAct!=CLASSIFY_US_CSACT_ACTCTRL_DISABLE)
			{
				PROC_PRINTF("\t[SACT]:%s \n\t\tSVID_ACT:%s	\tSVID:%d \n\t\tSPRI_ACT:%s \tSPRI:%d \n",
				decisionString,
				vidDecisionString,classifyCfg.act.usAct.sTagVid,
				priDecisionString,classifyCfg.act.usAct.sTagPri);
			}


			//[CACT]
			switch(classifyCfg.act.usAct.cAct)
			{
				case CLASSIFY_US_CACT_NOP: decisionString="NOP"; break;
				case CLASSIFY_US_CACT_ADD_CTAG_8100: decisionString="ADD_CTAG_8100"; break;
				case CLASSIFY_US_CACT_TRANSLATION_C2S: decisionString="C2S"; break;
				case CLASSIFY_US_CACT_DEL_CTAG: decisionString="DEL_CTAG"; break;
				case CLASSIFY_US_CACT_TRANSPARENT: decisionString="TRANSPARENT"; break;
				case CLASSIFY_US_CACT_ACTCTRL_DISABLE: decisionString="ACTCTRL_DISABLE"; break;
				default:
					decisionString="unKnown decision";
					break;
			}

			switch(classifyCfg.act.usAct.cVidAct)
			{
				case CLASSIFY_US_VID_ACT_ASSIGN: vidDecisionString="ASSIGN"; break;
				case CLASSIFY_US_VID_ACT_FROM_1ST_TAG: vidDecisionString="FROM_1ST_TAG"; break;
				case CLASSIFY_US_VID_ACT_FROM_2ND_TAG: vidDecisionString="FROM_2ND_TAG"; break;
				case CLASSIFY_US_VID_ACT_FROM_INTERNAL: vidDecisionString="FROM_INTERNAL"; break;
				case CLASSIFY_US_VID_ACT_NOP: vidDecisionString="NOP"; break;
				default:
					vidDecisionString="unKnown decision";
					break;
			}

			switch(classifyCfg.act.usAct.cPriAct)
			{
				case CLASSIFY_US_PRI_ACT_ASSIGN: priDecisionString="ASSIGN"; break;
				case CLASSIFY_US_PRI_ACT_FROM_1ST_TAG: priDecisionString="FROM_1ST_TAG"; break;
				case CLASSIFY_US_PRI_ACT_FROM_2ND_TAG: priDecisionString="FROM_2ND_TAG"; break;
				case CLASSIFY_US_PRI_ACT_FROM_INTERNAL: priDecisionString="FROM_INTERNAL"; break;
				case CLASSIFY_US_PRI_ACT_FROM_DSCP: priDecisionString="FROM_DSCP"; break;
				case CLASSIFY_US_PRI_ACT_NOP: priDecisionString="NOP"; break;
				default:
					priDecisionString="unKnown decision";
					break;
			}

			if(classifyCfg.act.usAct.cAct!=CLASSIFY_US_CACT_ACTCTRL_DISABLE){
				PROC_PRINTF("\t[CACT]:%s \n\t\tCVID_ACT:%s \tVID:%d \n\t\tCPRI_ACT:%s \tPRI:%d \n",
				decisionString,
				vidDecisionString,classifyCfg.act.usAct.cTagVid,
				priDecisionString,classifyCfg.act.usAct.cTagPri);
			}

			//[CFACT]
			switch(classifyCfg.act.usAct.interPriAct)
			{
				case CLASSIFY_CF_PRI_ACT_NOP: decisionString="NOP"; break;
				case CLASSIFY_CF_PRI_ACT_ASSIGN: decisionString="ASSIGN"; break;
				case CLASSIFY_CF_PRI_ACT_ACTCTRL_DISABLE: decisionString="ACTCTRL_DISABLE"; break;
				default:
					decisionString="unKnown decision";
					break;
			}
			if(classifyCfg.act.usAct.interPriAct!=CLASSIFY_CF_PRI_ACT_ACTCTRL_DISABLE)
			{
				PROC_PRINTF("\t[CFPRI]:%s \n\t\tCFPRI:%d\n",
					decisionString,
					classifyCfg.act.usAct.cfPri);
			}

			//[DSCP]
			switch(classifyCfg.act.usAct.dscp)
			{
				case CLASSIFY_DSCP_ACT_DISABLE: decisionString="DISABLE"; break;
				case CLASSIFY_DSCP_ACT_ENABLE: decisionString="ENABLE"; break;
				case CLASSIFY_DSCP_ACT_ACTCTRL_DISABLE: decisionString="ACTCTRL_DISABLE"; break;
				default:
					decisionString="unKnown decision";
					break;
			}
			if(classifyCfg.act.usAct.dscp!=CLASSIFY_DSCP_ACT_ACTCTRL_DISABLE)
			{
				rtk_classify_cfPri2Dscp_get(classifyCfg.act.usAct.cfPri, &dscp);
				PROC_PRINTF("\t[DSCP]:%s \tDSCP:%d \n",
				decisionString,
				dscp);
			}


			//[DROP]
			switch(classifyCfg.act.usAct.drop)
			{
				case CLASSIFY_DROP_ACT_NONE: decisionString="NONE"; break;
				case CLASSIFY_DROP_ACT_ENABLE: decisionString="DROP"; break;
				case CLASSIFY_DROP_ACT_TRAP: decisionString="TRAP"; break;
				case CLASSIFY_DROP_ACT_DROP_PON: decisionString="DROP_PON"; break;
				case CLASSIFY_DROP_ACT_ACTCTRL_DISABLE: decisionString="ACTCTRL_DISABLE"; break;
				default:
					decisionString="unKnown decision";
					break;
			}
			if(classifyCfg.act.usAct.drop!=CLASSIFY_DROP_ACT_ACTCTRL_DISABLE)
			{
				PROC_PRINTF("\t[DROP]:%s\n",
				decisionString);
			}

			//[SID]
			switch(classifyCfg.act.usAct.sidQidAct)
			{
				case CLASSIFY_US_SQID_ACT_ASSIGN_NOP: decisionString="NOP"; break;
				case CLASSIFY_US_SQID_ACT_ASSIGN_SID: decisionString="ASSIGN_SID"; break;
				case CLASSIFY_US_SQID_ACT_ASSIGN_QID: decisionString="ASSIGN_QID"; break;
				case CLASSIFY_US_SQID_ACT_ACTCTRL_DISABLE: decisionString="ACTCTRL_DISABLE"; break;
				default:
					decisionString="unKnown decision";
					break;
			}
			if(classifyCfg.act.usAct.sidQidAct!=CLASSIFY_US_SQID_ACT_ACTCTRL_DISABLE)
			{
				PROC_PRINTF("\t[SID]:%s \n\t\tstream_id=%d \n",
					decisionString,
					classifyCfg.act.usAct.sidQid);
			}
		}

#else
			if(classifyCfg.direction) //downstream
			{
				rtk_classify_cfPri2Dscp_get(classifyCfg.act.dsAct.cfPri, &dscp);

				//[SACT]
				if(classifyCfg.act.dsAct.csAct==CLASSIFY_DS_CSACT_ADD_TAG_VS_TPID || classifyCfg.act.dsAct.csAct==CLASSIFY_DS_CSACT_ADD_TAG_8100 ){
					PROC_PRINTF("\t[SACT]:%s \n\t\tSVID_ACT:%s \tSVID:%d \n\t\tSPRI_ACT:%s \tSPRI:%d \n",
					name_of_ds_csAct_idx[classifyCfg.act.dsAct.csAct],
					name_of_ds_csVid_idx[classifyCfg.act.dsAct.csVidAct],classifyCfg.act.dsAct.sTagVid,
					name_of_ds_csPri_idx[classifyCfg.act.dsAct.csPriAct],classifyCfg.act.dsAct.sTagPri);
				}else if (classifyCfg.act.dsAct.csAct==CLASSIFY_DS_CSACT_DEL_STAG || classifyCfg.act.dsAct.csAct==CLASSIFY_DS_CSACT_TRANSPARENT || classifyCfg.act.dsAct.csAct==CLASSIFY_DS_CSACT_SP2C){
					PROC_PRINTF("\t[SACT]:%s \n",name_of_ds_csAct_idx[classifyCfg.act.dsAct.csAct]);

				}

				//[CACT]
				if(classifyCfg.act.dsAct.cAct==CLASSIFY_DS_CACT_ADD_CTAG_8100){
					PROC_PRINTF("\t[CACT]:%s \n\t\tCVID_ACT:%s \tVID:%d \n\t\tCPRI_ACT:%s \tPRI:%d \n",
						name_of_ds_cAct_idx[classifyCfg.act.dsAct.cAct],
						name_of_ds_cVid_idx[classifyCfg.act.dsAct.cVidAct],classifyCfg.act.dsAct.cTagVid,
						name_of_ds_cPri_idx[classifyCfg.act.dsAct.cPriAct],classifyCfg.act.dsAct.cTagPri);
				}else if(classifyCfg.act.dsAct.cAct==CLASSIFY_DS_CACT_TRANSLATION_SP2C || classifyCfg.act.dsAct.cAct==CLASSIFY_DS_CACT_DEL_CTAG || CLASSIFY_DS_CACT_TRANSPARENT){
					PROC_PRINTF("\t[CACT]:%s\n",name_of_ds_cAct_idx[classifyCfg.act.dsAct.cAct]);
				}

				//[CFPRI]
				if(classifyCfg.act.dsAct.interPriAct==CLASSIFY_CF_PRI_ACT_ASSIGN)
					PROC_PRINTF("\t[CFPRI]:CFPRI ASSIGN \n\t\tCFPRI:%d\n",classifyCfg.act.dsAct.cfPri);

				//[DSCP]
				if(classifyCfg.act.dsAct.dscp==CLASSIFY_DSCP_ACT_ENABLE)
					PROC_PRINTF("\t[DSCP]:DSCP ASSIGN \n\t\tDSCP:%d \n",dscp);

				//[UNI]
				if(classifyCfg.act.dsAct.uniAct!=CLASSIFY_DS_UNI_ACT_NOP)
					PROC_PRINTF("\t[UNI]:%s \n\t\tUNI_MASK:0x%x\n",name_of_ds_uni_idx[classifyCfg.act.dsAct.uniAct],classifyCfg.act.dsAct.uniMask.bits[0]);
			}
			else
			{
				rtk_classify_cfPri2Dscp_get(classifyCfg.act.usAct.cfPri, &dscp);

				//[SACT]
				if(classifyCfg.act.usAct.csAct==CLASSIFY_US_CSACT_ADD_TAG_VS_TPID ||classifyCfg.act.usAct.csAct==CLASSIFY_US_CSACT_ADD_TAG_8100){
					PROC_PRINTF("\t[SACT]:%s \n\t\tSVID_ACT:%s	\tSVID:%d \n\t\tSPRI_ACT:%s \tSPRI:%d \n",
					name_of_us_csAct_idx[classifyCfg.act.usAct.csAct],
					name_of_us_csVid_idx[classifyCfg.act.usAct.csVidAct],classifyCfg.act.usAct.sTagVid,
					name_of_us_csPri_idx[classifyCfg.act.usAct.csPriAct],classifyCfg.act.usAct.sTagPri);
				}else if (classifyCfg.act.usAct.csAct==CLASSIFY_US_CSACT_DEL_STAG || classifyCfg.act.usAct.csAct==CLASSIFY_US_CSACT_TRANSPARENT){
					PROC_PRINTF("\t[SACT]:%s \n",name_of_us_csAct_idx[classifyCfg.act.usAct.csAct]);
				}

				//[CACT]
				if(classifyCfg.act.usAct.cAct==CLASSIFY_US_CACT_ADD_CTAG_8100){
					PROC_PRINTF("\t[CACT]:%s \n\t\tCVID_ACT:%s \tVID:%d \n\t\tCPRI_ACT:%s \tPRI:%d \n",
						name_of_us_cAct_idx[classifyCfg.act.usAct.cAct],
						name_of_us_cVid_idx[classifyCfg.act.usAct.cVidAct],classifyCfg.act.usAct.cTagVid,
						name_of_us_cPri_idx[classifyCfg.act.usAct.cPriAct],classifyCfg.act.usAct.cTagPri);
				}else if(classifyCfg.act.usAct.cAct==CLASSIFY_US_CACT_TRANSLATION_C2S||classifyCfg.act.usAct.cAct==CLASSIFY_US_CACT_DEL_CTAG||classifyCfg.act.usAct.cAct==CLASSIFY_US_CACT_TRANSPARENT){
					PROC_PRINTF("\t[CACT]:%s\n",name_of_us_cAct_idx[classifyCfg.act.usAct.cAct]);
				}

				//[CFACT]
				if(classifyCfg.act.usAct.interPriAct==CLASSIFY_CF_PRI_ACT_ASSIGN)
					PROC_PRINTF("\t[CFPRI]:CFPRI ASSIGN \n\t\tCFPRI:%d\n",classifyCfg.act.usAct.cfPri);

				//[DSCP]
				if(classifyCfg.act.usAct.dscp==CLASSIFY_DSCP_ACT_ENABLE)
					PROC_PRINTF("\t[DSCP]:DSCP ASSIGN \tDSCP:%d \n",dscp);

				//[DROP]
				if(classifyCfg.act.usAct.drop==CLASSIFY_DROP_ACT_ENABLE)
					PROC_PRINTF("\t[DROP]\n");

				//[DROP]
				if(classifyCfg.act.usAct.sidQidAct==CLASSIFY_US_SQID_ACT_ASSIGN_SID || classifyCfg.act.usAct.sidQidAct==CLASSIFY_US_SQID_ACT_ASSIGN_QID)
					PROC_PRINTF("\t[SID]:%s \n\t\tSidQid=%d \n",name_of_us_sidQid_idx[classifyCfg.act.usAct.sidQidAct],classifyCfg.act.usAct.sidQid);

				//[LOG]
				if(classifyCfg.act.usAct.log==CLASSIFY_US_LOG_ACT_ENABLE)
					PROC_PRINTF("\t[LOG] \tConterIdx:%d\n",classifyCfg.act.usAct.logCntIdx);
			}
#endif
		}
	}

#if 1
//#ifdef FORCE_PROBE_APOLLOMP

	PROC_PRINTF("--------------- CF HIT INFO----------------\n");
	switch(rg_kernel.apolloChipId)
	{
#if defined(CONFIG_RG_RTL9600_SERIES)
		case APOLLOMP_CHIP_ID:
			for(i=0; i<2; i++)
			{
				reg_array_field_read(APOLLOMP_STAT_CF_REASONr,REG_ARRAY_INDEX_NONE,i, APOLLOMP_CF_HIT_INFOf, &val);
				switch(i)
				{
				case 0:
					if(val&0x200)
						{PROC_PRINTF("[RULE(64-511):%s]: hit rule %d\n",val&0x200?"O":"X",val&0x1ff);}
					else
						{PROC_PRINTF("[RULE(64-511):%s]: unhit\n",val&0x200?"O":"X");}
					break;
				case 1:
					if(val&0x200)
						{PROC_PRINTF("[RULE(0-63):%s]: hit rule %d\n",val&0x200?"O":"X",val&0x1ff);}
					else
						{PROC_PRINTF("[RULE(0-63):%s]: unhit\n",val&0x200?"O":"X");}
					break;
				}
			}
			break;
#endif
#if defined(CONFIG_RTL9601B_SERIES)
		case RTL9601B_CHIP_ID:
			break;
#endif
#if defined(CONFIG_RG_RTL9602C_SERIES)
		case RTL9602C_CHIP_ID:
			break;
#endif
		default:
			PROC_PRINTF("Chip Not Support.\n");
	}
#endif

	return RT_ERR_RG_OK;
}


char *name_of_acl_iprange_type[]={
	"UNUSED",
	"IPV4_SIP",
	"IPV4_DIP",
	"IPV6_SIP",
	"IPV6_DIP",
	"IPV4_SIP_INNER",
	"IPV4_DIP_INNER",
};
int32 _dump_acl_ipRangeTable(struct seq_file *s)
{
    int i;
    rtk_acl_rangeCheck_ip_t ipRangeEntry;

    PROC_PRINTF("------------ ACL IP RANGE TABLES -------------\n");
    for(i=0; i<MAX_ACL_IPRANGETABLE_SIZE; i++)
    {
        memset(&ipRangeEntry,0,sizeof(ipRangeEntry));
        ipRangeEntry.index=i;
        rtk_acl_ipRange_get(&ipRangeEntry);
        PROC_PRINTF("\tIPRANGE[%d] upper:0x%x lower:0x%x type:%s\n",i,ipRangeEntry.upperIp,ipRangeEntry.lowerIp,name_of_acl_iprange_type[ipRangeEntry.type]);
    }

	return RT_ERR_RG_OK;
}

char *name_of_acl_portrange_type[]={
	"UNUSED",
	"SPORT",
	"DPORT",
};
int32 _dump_acl_portRangeTable(struct seq_file *s)
{
    int i;
    rtk_acl_rangeCheck_l4Port_t portRangeEntry;

    PROC_PRINTF("------------ ACL PORT RANGE TABLES -------------\n");
    for(i=0; i<MAX_ACL_PORTRANGETABLE_SIZE; i++)
    {
        memset(&portRangeEntry,0,sizeof(portRangeEntry));
        portRangeEntry.index=i;
        rtk_acl_portRange_get(&portRangeEntry);
        PROC_PRINTF("\tPORTRANGE[%d] upper:%d lower:%d type:%s\n",i,portRangeEntry.upper_bound,portRangeEntry.lower_bound,name_of_acl_portrange_type[portRangeEntry.type]);
    }

	return RT_ERR_RG_OK;
}



int32 _dump_acl_template(struct seq_file *s)
{
    int i;
    rtk_acl_template_t aclTemplate;

    PROC_PRINTF("--------------- ACL TEMPLATES(RTK API VALUE) ----------------\n");
	for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
    {
        memset(&aclTemplate,0,sizeof(aclTemplate));
        aclTemplate.index=i;
        rtk_acl_template_get(&aclTemplate);
        PROC_PRINTF("\tTEMPLATE[%d]: [0:0x%x][1:0x%x][2:0x%x][3:0x%x][4:0x%x][5:0x%x][6:0x%x][7:0x%x]\n",i,aclTemplate.fieldType[0],aclTemplate.fieldType[1],aclTemplate.fieldType[2],aclTemplate.fieldType[3],aclTemplate.fieldType[4],aclTemplate.fieldType[5],aclTemplate.fieldType[6],aclTemplate.fieldType[7]);
    }
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

	if(aclPara->filter_fields&INGRESS_TCP_FLAGS_BIT){
		PROC_PRINTF(" %s ingress_tcp_flags:  0x%x  ingress_tcp_flags_mask:  0x%x\n",(aclPara->filter_fields_inverse&INGRESS_TCP_FLAGS_BIT)?"not":"", acl_parameter->ingress_tcp_flags, acl_parameter->ingress_tcp_flags_mask);
	}

	if(aclPara->filter_fields&INGRESS_PKT_LEN_RANGE_BIT){
		PROC_PRINTF(" %s ingress_packet_length_start:  %d  ingress_packet_length_end:  %d\n",(aclPara->filter_fields_inverse&INGRESS_PKT_LEN_RANGE_BIT)?"not":"", acl_parameter->ingress_packet_length_start, acl_parameter->ingress_packet_length_end);
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
			PROC_PRINTF(" tcp_header_only:%d\n",acl_parameter->action_encap_udp.tcp_header_only);
			if(acl_parameter->action_encap_udp.tcp_rule_append)
				PROC_PRINTF(" tcp_rule_idx:%d\n",acl_parameter->action_encap_udp.tcp_rule_idx);
			break;

		case ACL_ACTION_TYPE_FLOW_MIB:
			PROC_PRINTF("action type: ACL_ACTION_TYPE_FLOW_MIB \n");
			PROC_PRINTF("flowmib_counter_index: %d\n",acl_parameter->action_flowmib_counter_idx);
			break;

		case ACL_ACTION_TYPE_TRAP_WITH_PRIORITY:
			PROC_PRINTF("action type: ACL_ACTION_TYPE_TRAP_WITH_PRIORITY \n");
			PROC_PRINTF("acl_priority: %d\n",acl_parameter->action_trap_with_priority);
			break;

		case ACL_ACTION_TYPE_TRAP2SLAVE_WITH_PRIORITY:
			PROC_PRINTF("action type: ACL_ACTION_TYPE_TRAP2SLAVE_WITH_PRIORITY \n");
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

	//FB not support patterns (even flow is different, it will be hash into the same way)
	if(rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0)
	{
		if(acl_filter->filter_fields & (INGRESS_CTAG_PRI_BIT|INGRESS_STAG_PRI_BIT))
		{
			//WARNING("apolloPro not support pattern (INGRESS_CTAG_PRI_BIT|INGRESS_STAG_PRI_BIT), due to FB will hash in the same way.");
			//RETURN_ERR(RT_ERR_RG_ACL_NOT_SUPPORT);

			//20170515:tysu suggest let customer can added it for one flow support. different cpri consider as the same flow case should not happened!
			WARNING("[Customer Aware!!!]INGRESS_CTAG_PRI_BIT/INGRESS_STAG_PRI_BIT is not support with flow hash in 9607C testChip, the rule can be added but all cpri/spri is considered as the same flow(cpri/spri 0~7 do the same action).");
		}

		if(acl_filter->filter_fields & (INGRESS_CTAG_CFI_BIT|INGRESS_STAG_DEI_BIT))
		{
			WARNING("apolloPropattern (INGRESS_CTAG_CFI_BIT|INGRESS_STAG_DEI_BIT) only filter for first slow path packet, suppose smae flow will not have different dei/cfi.");
		}
	}
	else
	{
		if(acl_filter->filter_fields & (INGRESS_CTAG_CFI_BIT|INGRESS_STAG_DEI_BIT|INGRESS_STAG_PRI_BIT))
		{
			WARNING("apolloPropattern (INGRESS_CTAG_CFI_BIT|INGRESS_STAG_DEI_BIT|INGRESS_STAG_PRI_BIT) only filter for first slow path packet, suppose smae flow will not have different dei/cfi.");
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

	//HW ACL set ipv4/ipv6 l4_protocal in different template field, so need to indicate ipv4 or ipv6. (supporting HW ACL drop)
	if(acl_filter->filter_fields & INGRESS_L4_POROTCAL_VALUE_BIT)
	{
		if(((acl_filter->filter_fields & INGRESS_IPV4_TAGIF_BIT)==0x0 && (acl_filter->filter_fields & INGRESS_IPV6_TAGIF_BIT)==0x0)
			 || (acl_filter->ingress_ipv4_tagif==0 && acl_filter->ingress_ipv6_tagif==0)){
			WARNING("INGRESS_L4_POROTCAL_VALUE_BIT should indicate ipv4 or ipv6. if H/W trap with priority is need, please also assign INGRESS_IPV4_TAGIF_BIT or INGRESS_IPV6_TAGIF_BIT.");
		}
	}

	if((acl_filter->filter_fields&INGRESS_WLANDEV_BIT) && (acl_filter->filter_fields&INGRESS_PORT_BIT))
	{
		if(acl_filter->ingress_port_mask.portmask & ~RTK_RG_ALL_EXT_PORTMASK){
			WARNING("INGRESS_PORT_BIT only support EXT port when config INGRESS_WLANDEV_BIT at the same time");
		}
	}

#if defined(CONFIG_GPON_FEATURE)
	if((acl_filter->filter_fields&INGRESS_STREAM_ID_BIT) && (rg_db.systemGlobal.initParam.wanPortGponMode))
	{
		if((acl_filter->ingress_stream_id < 0) || (acl_filter->ingress_stream_id > 126)){
			WARNING("INGRESS_STREAM_ID_BIT only support range 0~126, not support %d", acl_filter->ingress_stream_id);
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}
#endif
#if 0	//support egress pattern
	if((acl_filter->filter_fields&INGRESS_TCP_FLAGS_BIT) && (!ACTION_CHECK_HW_NEED(acl_filter)))
	{
		WARNING("INGRESS_TCP_FLAGS_BIT only support hardwarelize action. (forward action, rather than qos action in flow based platform.)");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	if((acl_filter->filter_fields&INGRESS_PKT_LEN_RANGE_BIT) && (!ACTION_CHECK_HW_NEED(acl_filter)))
	{
		WARNING("INGRESS_PKT_LEN_RANGE_BIT only support hardwarelize action. (forward action, rather than qos action in flow based platform.)");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
#endif
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
			INGRESS_IPV6_FLOWLABEL_BIT|
			INGRESS_TCP_FLAGS_BIT|
			INGRESS_PKT_LEN_RANGE_BIT|
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
		acl_filter->action_type==ACL_ACTION_TYPE_TRAP2SLAVE_WITH_PRIORITY||
		acl_filter->action_type==ACL_ACTION_TYPE_FLOW_MIB||
		acl_filter->action_type==ACL_ACTION_TYPE_SW_DROP
		)
	{
		//action can be support.
		if((acl_filter->action_type==ACL_ACTION_TYPE_FLOW_MIB) && (rg_db.systemGlobal.internalSupportMask&RTK_RG_INTERNAL_SUPPORT_BIT0 /*test chip*/) && (acl_filter->action_flowmib_counter_idx < MAX_FLOWMIB_TABLE_SIZE)) {
			WARNING("apolloPro testchip only support sw flow mib range %d to %d", MAX_FLOWMIB_TABLE_SIZE, MAX_FLOWMIB_TABLE_SIZE+PURE_SW_FLOWMIB_TABLE_SIZE-1);
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}
	else
	{
		WARNING("ACL action_type is not suported by this platform!");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}


	return (RT_ERR_RG_OK);
}

