#include <rtk_rg_acl.h>
#include <rtk_rg_acl_apolloFE.h>
#include <rtk_rg_apolloFE_liteRomeDriver.h>
#include <dal/rtl9602c/dal_rtl9602c_hwmisc.h>



int _dump_rg_acl_entry_content(struct seq_file *s, rtk_rg_aclFilterAndQos_t *aclPara);


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
	"GEMIDX/LLIDX", //0xa
	"",	"",	"",	"",	"", //0xb~0xf
	"IP4SIP[15:0]", //0x10
	"IP4SIP[31:16]",
	"IP4DIP[15:0]",
	"IP4DIP[31:16]",
	"IP4(TOS+PROTO)", //0x14
	"IP6(TC+NH)", //0x15
	"","","","","","","","","","",//0x16~0x1f
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

/*H/W CF debug sting*/

char *name_of_us_vid_cfg[]={
	"internal cvid",
	"internal cvid",
};

char *name_of_ds_vid_cfg[]={
	"outter vid",
	"ingress ctag vid",
};

char *name_of_us_pri_cfg[]={
	"internal pri",
	"remarked cpri",
};

char *name_of_ds_pri_cfg[]={
	"ingress ctag pri",
	"ingress ctag pri",
};

char *diagStr_cfActBit[] = {
	"[CVlan]",
	"[SVlan]",
	"[Dscp]",
	"[Forward]",
	"[CfPri]",
	"[Sid]"
};


/* internal APIs*/

#define RANGE_TABLE_SEARCH_FOR_ACL 0
#define RANGE_TABLE_SEARCH_FOR_CF 1

static int _rtk_rg_aclSWEntry_to_asic_add(rtk_rg_aclFilterAndQos_t *acl_filter,rtk_rg_aclFilterEntry_t* aclSWEntry,int shareHwAclWithSWAclIdx);
static int _rtk_rg_aclSWEntry_and_asic_reAdd(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx);
void _rtk_rg_acl_for_multicast_temp_protection_add(void);
void _rtk_rg_acl_for_multicast_temp_protection_del(void);



#ifndef HW_ACL_REARRANGE_PROTECT_VERSION
static int32 _rtk_rg_acl_multicastTempPermit_enable(void){
	//we reserved acl[rg_db.systemGlobal.aclAndCfReservedRule.acl_MC_temp_permit_idx] for permit multicast packet
	rtk_acl_ingress_entry_t aclRule;
	rtk_acl_field_t aclField_dmac0;

	bzero(&aclRule,sizeof(aclRule));
	bzero(&aclField_dmac0,sizeof(aclField_dmac0));

	DEBUG("Enabled ACL for MC temp permit");

	//setup MC DMAC care bit 0x01
	aclField_dmac0.fieldType = ACL_FIELD_PATTERN_MATCH;
	aclField_dmac0.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
	aclField_dmac0.fieldUnion.data.value = 0x100;
	aclField_dmac0.fieldUnion.data.mask = 0x100;
	if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac0))
	{
		DEBUG("setup ACL for MC temporary trap failed!!!");
		return RT_ERR_RG_FAILED;
	}

	aclRule.valid=ENABLED;
	aclRule.index=rg_db.systemGlobal.aclAndCfReservedRule.acl_MC_temp_permit_idx;
	aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
	aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
	aclRule.act.forwardAct.act= ACL_IGR_FORWARD_IGR_MIRROR_ACT;
	aclRule.act.forwardAct.portMask.bits[0]=0x0;

	aclRule.act.enableAct[ACL_IGR_CVLAN_ACT] = ENABLE;
	aclRule.act.cvlanAct.act = ACL_IGR_CVLAN_IGR_CVLAN_ACT;
	if(rg_db.systemGlobal.acl_rearrange_force_mc_ingress_cvid==-1){
		aclRule.act.cvlanAct.cvid=rg_db.systemGlobal.initParam.fwdVLAN_CPU;
	}else{
		aclRule.act.cvlanAct.cvid=rg_db.systemGlobal.acl_rearrange_force_mc_ingress_cvid;
	}

	if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule))
	{
		DEBUG("setup ACL for MC temporary trap failed!!!");
		return RT_ERR_RG_FAILED;
	}

	return RT_ERR_RG_OK;
}

static int32 _rtk_rg_acl_multicastTempPermit_disable(void){

	DEBUG("Disabled ACL for MC temp permit");

	if(rtk_acl_igrRuleEntry_del(rg_db.systemGlobal.aclAndCfReservedRule.acl_MC_temp_permit_idx))
	{
		WARNING("setup ACL for MC temporary trap failed!!!");
		return RT_ERR_RG_FAILED;
	}
	return RT_ERR_RG_OK;
}
#endif

static int32 _rtk_rg_acl_multicastTempPermit_and_rest_trap_enable(void){
	//we reserved acl[0] for permit multicast packet
	rtk_acl_ingress_entry_t aclRule;
	rtk_acl_field_t aclField,aclField2;

	/***set IPv4 Multicast Permit rule***/
	{
		bzero(&aclRule,sizeof(aclRule));
		bzero(&aclField,sizeof(aclField));
		bzero(&aclField2,sizeof(aclField2));
		//setup MC DMAC 01:00:5e:xx:xx:xx
		aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
		aclField.fieldUnion.data.value = 0x0100;
		aclField.fieldUnion.data.mask = 0xffff;
		if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
		{
			DEBUG("setup ACL for MC IPv4 permit failed!!!");
			return RT_ERR_RG_FAILED;
		}

		aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField2.fieldUnion.pattern.fieldIdx = 1; //DA[47:32]: template[0] field[1]
		aclField2.fieldUnion.data.value = 0x5e00;
		aclField2.fieldUnion.data.mask = 0xff00;
		if(rtk_acl_igrRuleField_add(&aclRule, &aclField2))
		{
			DEBUG("setup ACL for MC IPv4 permit failed!!!");
			return RT_ERR_RG_FAILED;
		}

		aclRule.valid=ENABLED;
		aclRule.index=rg_db.systemGlobal.aclAndCfReservedRule.aclSkipRearrangeIPv4MCPermitIdx;
		aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
		aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
		aclRule.act.forwardAct.act= ACL_IGR_FORWARD_IGR_MIRROR_ACT;	//PERMIT
		aclRule.act.forwardAct.portMask.bits[0]=0x0;
		if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule))
		{
			DEBUG("setup ACL for MC IPv4 permit failed!!!");
			return RT_ERR_RG_FAILED;
		}
	}

	/***set IPv6 Multicast Permit rule***/
	{
		bzero(&aclRule,sizeof(aclRule));
		bzero(&aclField,sizeof(aclField));
		bzero(&aclField2,sizeof(aclField2));
		//setup MC DMAC 33:33:xx:xx:xx:xx
		aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
		aclField.fieldUnion.data.value = 0x3333;
		aclField.fieldUnion.data.mask = 0xffff;
		if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
		{
			DEBUG("setup ACL for MC IPv6 permit failed!!!");
			return RT_ERR_RG_FAILED;
		}

		aclRule.valid=ENABLED;
		aclRule.index=rg_db.systemGlobal.aclAndCfReservedRule.aclSkipRearrangeIPv6MCPermitIdx;
		aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
		aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
		aclRule.act.forwardAct.act= ACL_IGR_FORWARD_IGR_MIRROR_ACT;	//PERMIT
		aclRule.act.forwardAct.portMask.bits[0]=0x0;
		if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule))
		{
			DEBUG("setup ACL for MC IPv6 permit failed!!!");
			return RT_ERR_RG_FAILED;
		}
	}

	/***set rest trap rule***/
	{
		bzero(&aclRule,sizeof(aclRule));
		bzero(&aclField,sizeof(aclField));
		bzero(&aclField2,sizeof(aclField2));
		aclRule.valid=ENABLED;
		aclRule.index=rg_db.systemGlobal.aclAndCfReservedRule.aclSkipRearrangeAlltTrapIdx;
		aclRule.templateIdx=0;
		aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
		aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
		aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
#ifdef CONFIG_DUALBAND_CONCURRENT
		//force internal-priority(CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI-1) to avoid trap to EXT-1
		aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
		aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
		aclRule.act.priAct.aclPri=((CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI-1)&0x7);
#endif
		if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule))
		{
			DEBUG("setup ACL for rest all trap failed!!!");
			return RT_ERR_RG_FAILED;
		}

	}
	return RT_ERR_RG_OK;
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
    int i,start_idx;
    rtk_acl_rangeCheck_l4Port_t aclPortRangeEntry;
    bzero(&aclPortRangeEntry, sizeof(aclPortRangeEntry));

	if(type==RANGE_TABLE_SEARCH_FOR_ACL)
	{
		start_idx=31; //ACL use table entry 16~31, search from 31 to 16
	    for(i=start_idx;i>=16;i--)
	    {
	        aclPortRangeEntry.index = i;
	        if(rtk_acl_portRange_get(&aclPortRangeEntry))
	        {
	            RETURN_ERR(RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
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
	else
	{
		start_idx=0;
	    for(i=start_idx;i<(start_idx+16);i++)
	    {
	        aclPortRangeEntry.index = i;
	        if(rtk_acl_portRange_get(&aclPortRangeEntry))
	        {
	            RETURN_ERR(RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
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
    RETURN_ERR(RT_ERR_RG_ACL_PORTTABLE_FULL);

}

static int _rtk_rg_search_acl_empty_ipTableEntry(uint32* index, uint32 type)
{
    int i,start_idx;
    rtk_acl_rangeCheck_ip_t aclIpRangeEntry;
    bzero(&aclIpRangeEntry, sizeof(aclIpRangeEntry));

	if(type==RANGE_TABLE_SEARCH_FOR_ACL)
	{
		start_idx=31; //ACL use table entry 16~31, search from 31 to 16
		for(i=start_idx;i>=16;i--)
	   {
		   aclIpRangeEntry.index = i;
		   if(rtk_acl_ipRange_get(&aclIpRangeEntry))
		   {
			   RETURN_ERR(RT_ERR_RG_ACL_IPTABLE_ACCESS_FAILED);
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
	else{
		start_idx=0; //ACL use table entry 0~15, search from 0 to 15

	    for(i=start_idx;i<16;i++)
	    {
	        aclIpRangeEntry.index = i;
	        if(rtk_acl_ipRange_get(&aclIpRangeEntry))
	        {
	            RETURN_ERR(RT_ERR_RG_ACL_IPTABLE_ACCESS_FAILED);
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
    RETURN_ERR(RT_ERR_RG_ACL_IPTABLE_FULL);
}

static int _rtk_rg_search_acl_empty_Entry(int size, int* index)
{
    int i=0;
    int continue_size=0;
    rtk_acl_ingress_entry_t aclRule;
    bzero(&aclRule, sizeof(aclRule));


	DEBUG("size = %d",size);


    if(size == 0) //no need acl entry
    {
        return (RT_ERR_RG_OK);
    }
    for(i=MIN_ACL_ENTRY_INDEX; i<=MAX_ACL_ENTRY_INDEX; i++)
    {
        aclRule.index = i;
		DEBUG("check ACL[%d] is empty or not",i);
        if(rtk_acl_igrRuleEntry_get(&aclRule))
        {
            DEBUG("get acl[%d] failed",i);
            RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
        }
        if(aclRule.valid==DISABLED)
        {

            continue_size++;
			DEBUG("get acl[%d] aclRule.valid is %d.  continue_size=%d",i,aclRule.valid,continue_size);
            if(continue_size == size)
            {
                *index = ((i+1) - size);
                //DEBUG("get empty acl entries start_index=%d continue_size=%d ",*index,continue_size);
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

    RETURN_ERR(RT_ERR_RG_ACL_ENTRY_FULL);
}

static int _rtk_rg_search_cf_empty_Entry(int size,int* index)
{
    int i,j;
    rtk_classify_cfg_t cfRule;
    bzero(&cfRule, sizeof(cfRule));
    for(i=RESERVED_CF_BEFORE; i<RESERVED_CF_AFTER; i++)
    {
        cfRule.index = i;
        if(rtk_classify_cfgEntry_get(&cfRule))
        {
            RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
        }

        if(cfRule.valid==DISABLED)
        {
            *index = i;
			for(j=0;j<size;j++){
				cfRule.index = i+j;
				if(rtk_classify_cfgEntry_get(&cfRule))
				{
					RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
				}
				if(cfRule.valid==DISABLED){
					//DEBUG("cf need size=%d get cf[%d] is empty",size,cfRule.index);
				   continue;
				}else{
					RETURN_ERR(RT_ERR_RG_CF_ENTRY_FULL);
				}
			}
			return (RT_ERR_RG_OK);
        }

    }
   	return (RT_ERR_RG_CF_ENTRY_FULL);
}

static int _rtk_rg_free_acl_portTableEntry(int index)
{
	int ret;
    rtk_acl_rangeCheck_l4Port_t aclPortRangeEntry;
    bzero(&aclPortRangeEntry,sizeof(aclPortRangeEntry));
    aclPortRangeEntry.index=index;
	ret = rtk_acl_portRange_set(&aclPortRangeEntry);
    if(ret!= RT_ERR_RG_OK)
    {
        DEBUG("free acl porttable failed, ret(rtk)=%d",ret);
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
        DEBUG("free acl iptable failed, ret(rtk)=%d",ret);
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

#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
static int _rtk_rg_rearrange_ACL_weight_for_egress_wan(void)
{
	int i,j,ret;
	int temp,sp,p_idx,n_idx,p_type,n_type,p_weight,n_weight;

	//1-4. sorting the rule by weight: BubSort
	for (i=MAX_ACL_SW_ENTRY_SIZE-1; i>0; i--){
		sp=1;
		for (j=0; j<=i; j++){
			//check data is valid
			if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]==-1 || rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1]==-1)
				break;

			//we change the order if the weight is the same but the type has different
			p_weight=rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j])].acl_weight;
			n_weight=rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1])].acl_weight;
			if(p_weight!=n_weight)continue;

			p_idx=rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j])].egress_intf_idx;
			p_type=RG_ACL_TRANS_NONE;
			n_idx=rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1])].egress_intf_idx;
			n_type=RG_ACL_TRANS_NONE;

			//compare decision:vlanB_l34>portB_l34>vlanB_l2>portB_l2>intf_ro>df_ro>l2
			if(rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j])].filter_fields&EGRESS_INTF_BIT){
				ret=rtk_rg_apollo_intfInfo_find(&rg_db.systemGlobal.egress_intf_info, &p_idx);
				if((ret==RT_ERR_RG_OK)&&(rg_db.systemGlobal.egress_intf_info.is_wan)&&(p_idx==rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j])].egress_intf_idx)){
					if(rg_db.systemGlobal.egress_intf_info.wan_intf.wan_intf_conf.vlan_binding_mask.portmask)
						p_type=RG_ACL_TRANS_L2_VLAN_BIND;
					else if(rg_db.systemGlobal.egress_intf_info.wan_intf.wan_intf_conf.port_binding_mask.portmask)
						p_type=RG_ACL_TRANS_L2_PORT_BIND;

					if(rg_db.systemGlobal.egress_intf_info.wan_intf.wan_intf_conf.wan_type!=RTK_RG_BRIDGE){
						if(p_type!=RG_ACL_TRANS_NONE)
							p_type+=2;	//L34
						else{
							switch(rg_db.systemGlobal.interfaceInfo[p_idx].p_wanStaticInfo->ip_version){
								case IPVER_V4ONLY:
									if(rg_db.systemGlobal.interfaceInfo[p_idx].p_wanStaticInfo->ipv4_default_gateway_on)
										p_type=RG_ACL_TRANS_v4_OTHER;
									else
										p_type=RG_ACL_TRANS_v4_SUBNET;
									break;
								case IPVER_V6ONLY:
									if(rg_db.systemGlobal.interfaceInfo[p_idx].p_wanStaticInfo->ipv6_default_gateway_on)
										p_type=RG_ACL_TRANS_v6_OTHER;
									else
										p_type=RG_ACL_TRANS_v6_SUBNET;
									break;
								default:
									if(rg_db.systemGlobal.interfaceInfo[p_idx].p_wanStaticInfo->ipv4_default_gateway_on)
										p_type=RG_ACL_TRANS_v4_OTHER_v6_OTHER;
									else
										p_type=RG_ACL_TRANS_v4_SUBNET_v6_OTHER;
									if(!rg_db.systemGlobal.interfaceInfo[p_idx].p_wanStaticInfo->ipv6_default_gateway_on)
										p_type+=1;
									break;
							}
						}
					}else if(p_type==RG_ACL_TRANS_NONE)
						p_type=RG_ACL_TRANS_L2; //non-binding bridge interface
				}else
					p_type=RG_ACL_TRANS_L2; //non-exist interface or non-wan interface
			}
			if(rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1])].filter_fields&EGRESS_INTF_BIT){
				ret=rtk_rg_apollo_intfInfo_find(&rg_db.systemGlobal.egress_intf_info, &n_idx);
				if((ret==RT_ERR_RG_OK)&&(rg_db.systemGlobal.egress_intf_info.is_wan)&& (n_idx==rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1])].egress_intf_idx)){
					if(rg_db.systemGlobal.egress_intf_info.wan_intf.wan_intf_conf.vlan_binding_mask.portmask)
						n_type=RG_ACL_TRANS_L2_VLAN_BIND;
					else if(rg_db.systemGlobal.egress_intf_info.wan_intf.wan_intf_conf.port_binding_mask.portmask)
						n_type=RG_ACL_TRANS_L2_PORT_BIND;

					if(rg_db.systemGlobal.egress_intf_info.wan_intf.wan_intf_conf.wan_type!=RTK_RG_BRIDGE){
						if(n_type!=RG_ACL_TRANS_NONE)
							n_type+=2;	//L34
						else{
							switch(rg_db.systemGlobal.interfaceInfo[n_idx].p_wanStaticInfo->ip_version){
								case IPVER_V4ONLY:
									if(rg_db.systemGlobal.interfaceInfo[n_idx].p_wanStaticInfo->ipv4_default_gateway_on)
										n_type=RG_ACL_TRANS_v4_OTHER;
									else
										n_type=RG_ACL_TRANS_v4_SUBNET;
									break;
								case IPVER_V6ONLY:
									if(rg_db.systemGlobal.interfaceInfo[n_idx].p_wanStaticInfo->ipv6_default_gateway_on)
										n_type=RG_ACL_TRANS_v6_OTHER;
									else
										n_type=RG_ACL_TRANS_v6_SUBNET;
									break;
								default:
									if(rg_db.systemGlobal.interfaceInfo[n_idx].p_wanStaticInfo->ipv4_default_gateway_on)
										n_type=RG_ACL_TRANS_v4_OTHER_v6_OTHER;
									else
										n_type=RG_ACL_TRANS_v4_SUBNET_v6_OTHER;
									if(!rg_db.systemGlobal.interfaceInfo[n_idx].p_wanStaticInfo->ipv6_default_gateway_on)
										n_type+=1;
									break;
							}
						}
					}else if(n_type==RG_ACL_TRANS_NONE)
						n_type=RG_ACL_TRANS_L2; //non-binding bridge interface
				}else
					n_type=RG_ACL_TRANS_L2; //non-exist interface or non-wan interface
			}
			//20150826LUKE: Mix egress WAN pattern rule with non-egress-WAN pattern rule at same weight should return fail.
			if((p_type==RG_ACL_TRANS_NONE && n_type>p_type) || (n_type==RG_ACL_TRANS_NONE && p_type>n_type))
				return RT_ERR_RG_ACL_EGRESS_WAN_MIX;

			//compare the type while acl_weight is the same
			if (p_type<n_type){
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


#endif

static int _rtk_rg_rearrange_ACL_weight(int *accumulateIdx)
{
	int i,j;
	int temp,sp;
    bzero(&rg_db.systemGlobal.empty_aclFilter, sizeof(rtk_rg_aclFilterAndQos_t));

	//1-1. clean the sorting record of type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
	*accumulateIdx = 0;
	for(j=0;j<MAX_ACL_SW_ENTRY_SIZE;j++){
		rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]=-1;
		rg_db.systemGlobal.acl_SWindex_sorting_by_weight_and_ingress_cvid_action[j]=-1;
	}

	//1-2. record the rule which type is ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
    for(i=0, j=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
    {
        if(memcmp(&rg_db.systemGlobal.acl_filter_temp[i],&rg_db.systemGlobal.empty_aclFilter,sizeof(rg_db.systemGlobal.empty_aclFilter)))
        {
        	if(rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
			{
            	//ASSERT_EQ(_rtk_rg_aclSWEntry_reAdd(&rg_db.systemGlobal.acl_filter_temp[i], &i),RT_ERR_RG_OK);
				rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j] = i; //record the rule which type is ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
				j++;
				*accumulateIdx=j;
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
    bzero(&rg_db.systemGlobal.empty_aclFilter, sizeof(rtk_rg_aclFilterAndQos_t));

	//1-1. record the continue and sorting start index from rg_db.systemGlobal.acl_SWindex_sorting_by_weight[],
	sort_start = *accumulateIdx;


	// 1-2. find out the L34 trap/drop/permit rules, and put index to rg_db.systemGlobal.acl_SWindex_sorting_by_weight[] first
	for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
	{
		if(memcmp(&rg_db.systemGlobal.acl_filter_temp[i],&rg_db.systemGlobal.empty_aclFilter,sizeof(rg_db.systemGlobal.empty_aclFilter)))
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
	for (i=MAX_ACL_SW_ENTRY_SIZE-1; i>sort_start; i--){
		sp=1;
	   	for (j=sort_start; j<=i; j++){
			//check data is valid
			if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]==-1)
				break;

			if(j<i){
				if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1]==-1)
					break;
			}

			if(j==(MAX_ACL_SW_ENTRY_SIZE-1))
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


	// 1-4. re-add the rules by sorted result
	for(j=sort_start;j<sort_end;j++)
	{
		i = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j];
		ASSERT_EQ(_rtk_rg_aclSWEntry_and_asic_reAdd(&rg_db.systemGlobal.acl_filter_temp[i], &i),RT_ERR_RG_OK);
	}


	return RT_ERR_RG_OK;
}

static int _rtk_rg_rearrange_ACL_weight_for_l34_Qos(int *accumulateIdx)
{
	int i,j;
	int temp,sp;
	int32 sort_start=0, sort_end=0;
	bzero(&rg_db.systemGlobal.empty_aclFilter, sizeof(rtk_rg_aclFilterAndQos_t));

	//1-1. record the continue and sorting start index from rg_db.systemGlobal.acl_SWindex_sorting_by_weight[],
	sort_start = *accumulateIdx;


	// 1-2. find out the L34 QoS rules, and put index to rg_db.systemGlobal.acl_SWindex_sorting_by_weight[] first
	for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
	{
		if(memcmp(&rg_db.systemGlobal.acl_filter_temp[i],&rg_db.systemGlobal.empty_aclFilter,sizeof(rg_db.systemGlobal.empty_aclFilter)))
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
	for (i=MAX_ACL_SW_ENTRY_SIZE-1; i>sort_start; i--){
		sp=1;
		for (j=sort_start; j<=i; j++){
			//check data is valid
			if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]==-1)
				break;
			if(j<i){
				if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1]==-1)
					break;
			}
			if(j==(MAX_ACL_SW_ENTRY_SIZE-1))
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


	// 1-4. re-add the rules by sorted result
	for(j=sort_start;j<sort_end;j++)
	{
		i = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j];
		ASSERT_EQ(_rtk_rg_aclSWEntry_and_asic_reAdd(&rg_db.systemGlobal.acl_filter_temp[i], &i),RT_ERR_RG_OK);
	}

	return RT_ERR_RG_OK;
}


static int _rtk_rg_aclDuplicatedIngressRuleCheck(int chkAclSWEntryIdx,rtk_rg_aclFilterAndQos_t *chkAclSWEntry,int *duIdx)
{
	/*for cheching is there any duplicated pure ingress rule.
	If the duplicated rule exist it doesn't need to add HW ACL again.
	We just shared the same HW ACL rule index*/

	int i,cmpIdx;
	rtk_rg_aclFilterEntry_t *cmpAclSWEntry;

	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){
		//get elder rules
		cmpIdx = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i];
		if(cmpIdx==-1){//rest rules haven't set.
			ACL("no rest rule need to check!\n");
			break;
		}

		cmpAclSWEntry = (&rg_db.systemGlobal.acl_SW_table_entry[cmpIdx]);
		ACL("Current RG_ACL cmp with RG_ACL[%d] cmpAclSWEntry.filter_fields=0x%llx chkAclSWEntry.filter_fields=0x%llx \n",cmpIdx,cmpAclSWEntry->acl_filter.filter_fields,chkAclSWEntry->filter_fields);

		if(chkAclSWEntryIdx==cmpIdx){//this is chkAclSWEntry self rule.
			ACL("self rule, skip!\n");
			continue;
		}

		if(cmpAclSWEntry->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET){//pure acl without cf_latch action, can not be shared.
			ACL("pure acl rule, skip!\n");
			continue;
		}

		//compared the ingress part with elder rule
		if((chkAclSWEntry->filter_fields&PURE_ACL_PATTERN_BITS)==(cmpAclSWEntry->acl_filter.filter_fields&PURE_ACL_PATTERN_BITS)){

			if(chkAclSWEntry->filter_fields&INGRESS_PORT_BIT){
				if(chkAclSWEntry->ingress_port_mask.portmask!=cmpAclSWEntry->acl_filter.ingress_port_mask.portmask){
					ACL("INGRESS_PORT_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_INTF_BIT){
				if(chkAclSWEntry->ingress_intf_idx!=cmpAclSWEntry->acl_filter.ingress_intf_idx){
					ACL("INGRESS_INTF_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_ETHERTYPE_BIT){
				if(chkAclSWEntry->ingress_ethertype!=cmpAclSWEntry->acl_filter.ingress_ethertype){
					ACL("INGRESS_ETHERTYPE_BIT not the same\n");
					continue;
				}
				if(chkAclSWEntry->ingress_ethertype_mask!=cmpAclSWEntry->acl_filter.ingress_ethertype_mask){
					ACL("INGRESS_ETHERTYPE_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_CTAG_PRI_BIT){
				if(chkAclSWEntry->ingress_ctag_pri!=cmpAclSWEntry->acl_filter.ingress_ctag_pri){
					ACL("INGRESS_CTAG_PRI_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_CTAG_VID_BIT){
				if(chkAclSWEntry->ingress_ctag_vid!=cmpAclSWEntry->acl_filter.ingress_ctag_vid){
					ACL("INGRESS_CTAG_VID_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_CTAG_CFI_BIT){
				if(chkAclSWEntry->ingress_ctag_cfi!=cmpAclSWEntry->acl_filter.ingress_ctag_cfi){
					ACL("INGRESS_CTAG_CFI_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_SMAC_BIT){
				if(memcmp(chkAclSWEntry->ingress_smac.octet,cmpAclSWEntry->acl_filter.ingress_smac.octet,ETHER_ADDR_LEN)){
					ACL("INGRESS_SMAC_BIT not the same\n");
					continue;
				}
				if(memcmp(chkAclSWEntry->ingress_smac_mask.octet,cmpAclSWEntry->acl_filter.ingress_smac_mask.octet,ETHER_ADDR_LEN)){
					ACL("INGRESS_SMAC_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_DMAC_BIT){
				if(memcmp(chkAclSWEntry->ingress_dmac.octet,cmpAclSWEntry->acl_filter.ingress_dmac.octet,ETHER_ADDR_LEN)){
					ACL("INGRESS_DMAC_BIT not the same\n");
					continue;
				}
				if(memcmp(chkAclSWEntry->ingress_dmac_mask.octet,cmpAclSWEntry->acl_filter.ingress_dmac_mask.octet,ETHER_ADDR_LEN)){
					ACL("INGRESS_DMAC_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_DSCP_BIT){
				if(chkAclSWEntry->ingress_dscp!=cmpAclSWEntry->acl_filter.ingress_dscp){
					ACL("INGRESS_DSCP_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_IPV6_SIP_RANGE_BIT){
				if(memcmp(chkAclSWEntry->ingress_src_ipv6_addr_start,cmpAclSWEntry->acl_filter.ingress_src_ipv6_addr_start,16)){
					ACL("INGRESS_IPV6_SIP_RANGE_BIT not the same\n");
					continue;
				}
				if(memcmp(chkAclSWEntry->ingress_src_ipv6_addr_end,cmpAclSWEntry->acl_filter.ingress_src_ipv6_addr_end,16)){
					ACL("INGRESS_IPV6_SIP_RANGE_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_IPV6_DIP_RANGE_BIT){
#if 0
				if(memcmp(chkAclSWEntry->ingress_dest_ipv6_addr_start,cmpAclSWEntry->acl_filter.ingress_dest_ipv6_addr_start,16)){
					ACL("INGRESS_IPV6_DIP_RANGE_BIT not the same\n");
					continue;
				}
				if(memcmp(chkAclSWEntry->ingress_dest_ipv6_addr_end,cmpAclSWEntry->acl_filter.ingress_dest_ipv6_addr_end,16)){
					ACL("INGRESS_IPV6_DIP_RANGE_BIT not the same\n");
					continue;
				}
#endif
			}
			if(chkAclSWEntry->filter_fields&INGRESS_IPV4_SIP_RANGE_BIT){
				if(chkAclSWEntry->ingress_src_ipv4_addr_start!=cmpAclSWEntry->acl_filter.ingress_src_ipv4_addr_start){
					ACL("INGRESS_IPV4_SIP_RANGE_BIT not the same\n");
					continue;
				}
				if(chkAclSWEntry->ingress_src_ipv4_addr_end!=cmpAclSWEntry->acl_filter.ingress_src_ipv4_addr_end){
					ACL("INGRESS_IPV4_SIP_RANGE_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_IPV4_DIP_RANGE_BIT){
				if(chkAclSWEntry->ingress_dest_ipv4_addr_start!=cmpAclSWEntry->acl_filter.ingress_dest_ipv4_addr_start){
					ACL("INGRESS_IPV4_DIP_RANGE_BIT not the same\n");
					//ACL("chkAclSWEntry->ingress_dest_ipv4_addr_start=0x%x , cmpAclSWEntry->acl_filter.ingress_dest_ipv4_addr_start=0x%x",chkAclSWEntry->ingress_dest_ipv4_addr_start,cmpAclSWEntry->acl_filter.ingress_dest_ipv4_addr_start);
					continue;
				}
				if(chkAclSWEntry->ingress_dest_ipv4_addr_end!=cmpAclSWEntry->acl_filter.ingress_dest_ipv4_addr_end){
					ACL("INGRESS_IPV4_DIP_RANGE_BIT not the same\n");
					//ACL("chkAclSWEntry->ingress_dest_ipv4_addr_end=0x%x, cmpAclSWEntry->acl_filter.ingress_dest_ipv4_addr_end=0x%x",chkAclSWEntry->ingress_dest_ipv4_addr_end,cmpAclSWEntry->acl_filter.ingress_dest_ipv4_addr_end);
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_L4_SPORT_RANGE_BIT){
				if(chkAclSWEntry->ingress_src_l4_port_start!=cmpAclSWEntry->acl_filter.ingress_src_l4_port_start){
					ACL("INGRESS_L4_SPORT_RANGE_BIT not the same\n");
					continue;
				}
				if(chkAclSWEntry->ingress_src_l4_port_end!=cmpAclSWEntry->acl_filter.ingress_src_l4_port_end){
					ACL("INGRESS_L4_SPORT_RANGE_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_L4_DPORT_RANGE_BIT){
				if(chkAclSWEntry->ingress_dest_l4_port_start!=cmpAclSWEntry->acl_filter.ingress_dest_l4_port_start){
					ACL("INGRESS_L4_DPORT_RANGE_BIT not the same\n");
					continue;
				}
				if(chkAclSWEntry->ingress_dest_l4_port_end!=cmpAclSWEntry->acl_filter.ingress_dest_l4_port_end){
					ACL("INGRESS_L4_DPORT_RANGE_BIT not the same\n");
					continue;
				}
			}

			if(chkAclSWEntry->filter_fields&INGRESS_IPV6_DSCP_BIT){
				if(chkAclSWEntry->ingress_ipv6_dscp!=cmpAclSWEntry->acl_filter.ingress_ipv6_dscp){
					ACL("INGRESS_IPV6_DSCP_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_STREAM_ID_BIT){
				if(chkAclSWEntry->ingress_stream_id!=cmpAclSWEntry->acl_filter.ingress_stream_id){
					ACL("INGRESS_STREAM_ID_BIT not the same\n");
					continue;
				}
				if(chkAclSWEntry->ingress_stream_id_mask!=cmpAclSWEntry->acl_filter.ingress_stream_id_mask){
					ACL("INGRESS_STREAM_ID_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_STAG_PRI_BIT){
				if(chkAclSWEntry->ingress_stag_pri!=cmpAclSWEntry->acl_filter.ingress_stag_pri){
					ACL("INGRESS_STAG_PRI_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_STAG_VID_BIT){
				if(chkAclSWEntry->ingress_stag_vid!=cmpAclSWEntry->acl_filter.ingress_stag_vid){
					ACL("INGRESS_STAG_VID_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_STAG_DEI_BIT){
				if(chkAclSWEntry->ingress_stag_dei!=cmpAclSWEntry->acl_filter.ingress_stag_dei){
					ACL("INGRESS_STAG_DEI_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_STAGIF_BIT){
				if(chkAclSWEntry->ingress_stagIf!=cmpAclSWEntry->acl_filter.ingress_stagIf){
					ACL("INGRESS_STAGIF_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_CTAGIF_BIT){
				if(chkAclSWEntry->ingress_ctagIf!=cmpAclSWEntry->acl_filter.ingress_ctagIf){
					ACL("INGRESS_CTAGIF_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_L4_POROTCAL_VALUE_BIT){
				if(chkAclSWEntry->ingress_l4_protocal!=cmpAclSWEntry->acl_filter.ingress_l4_protocal){
					ACL("INGRESS_L4_POROTCAL_VALUE_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_TOS_BIT){
				if(chkAclSWEntry->ingress_tos!=cmpAclSWEntry->acl_filter.ingress_tos){
					ACL("INGRESS_TOS_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_IPV6_TC_BIT){
				if(chkAclSWEntry->ingress_ipv6_tc!=cmpAclSWEntry->acl_filter.ingress_ipv6_tc){
					ACL("INGRESS_IPV6_TC_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_IPV6_SIP_BIT){
				if(memcmp(chkAclSWEntry->ingress_src_ipv6_addr,cmpAclSWEntry->acl_filter.ingress_src_ipv6_addr,16)){
					ACL("INGRESS_IPV6_SIP_BIT not the same\n");
					continue;
				}
				if(memcmp(chkAclSWEntry->ingress_src_ipv6_addr_mask,cmpAclSWEntry->acl_filter.ingress_src_ipv6_addr_mask,16)){
					ACL("INGRESS_IPV6_SIP_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_IPV6_DIP_BIT){
				if(memcmp(chkAclSWEntry->ingress_dest_ipv6_addr,cmpAclSWEntry->acl_filter.ingress_dest_ipv6_addr,16)){
					ACL("INGRESS_IPV6_DIP_BIT not the same\n");
					continue;
				}
				if(memcmp(chkAclSWEntry->ingress_dest_ipv6_addr_mask,cmpAclSWEntry->acl_filter.ingress_dest_ipv6_addr_mask,16)){
					ACL("INGRESS_IPV6_DIP_BIT not the same\n");
					continue;
				}
			}
			if(chkAclSWEntry->filter_fields&INGRESS_WLANDEV_BIT){
				if(chkAclSWEntry->ingress_wlanDevMask!=cmpAclSWEntry->acl_filter.ingress_wlanDevMask){
					ACL("INGRESS_WLANDEV_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_IPV4_TAGIF_BIT){
				if(chkAclSWEntry->ingress_ipv4_tagif!=cmpAclSWEntry->acl_filter.ingress_ipv4_tagif){
					ACL("INGRESS_IPV4_TAGIF_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_IPV6_TAGIF_BIT){
				if(chkAclSWEntry->ingress_ipv6_tagif!=cmpAclSWEntry->acl_filter.ingress_ipv6_tagif){
					ACL("INGRESS_IPV6_TAGIF_BIT not the same\n");
					continue;
				}

			}
			if(chkAclSWEntry->filter_fields&INGRESS_PKT_LEN_RANGE_BIT){
				if(chkAclSWEntry->ingress_packet_length_start!=cmpAclSWEntry->acl_filter.ingress_packet_length_start){
					ACL("INGRESS_PKT_LEN_RANGE_BIT not the same\n");
					continue;
				}
				if(chkAclSWEntry->ingress_packet_length_end!=cmpAclSWEntry->acl_filter.ingress_packet_length_end){
					ACL("INGRESS_PKT_LEN_RANGE_BIT not the same\n");
					continue;
				}
			}

			ACL("Ingress Part is the same with RG_ACL[%d], share the HW ACL[%d] \n",cmpIdx,cmpAclSWEntry->hw_aclEntry_start);
			*duIdx = cmpIdx;
			break;
		}
	}

	return (RT_ERR_RG_OK);
}

static int _rtk_rg_aclSWEntry_and_asic_reAdd(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx)
{
	int duIdx;
	int ret=RT_ERR_RG_OK;
    bzero(&rg_db.systemGlobal.aclSWEntry,sizeof(rg_db.systemGlobal.aclSWEntry));
    bzero(&rg_db.systemGlobal.empty_aclSWEntry,sizeof(rg_db.systemGlobal.empty_aclSWEntry));

    //check input parameter
    if(acl_filter == NULL
            || acl_filter_idx==NULL)
        RETURN_ERR(RT_ERR_RG_NULL_POINTER);

    if(acl_filter->filter_fields == 0x0)
        RETURN_ERR(RT_ERR_RG_INITPM_UNINIT);

	ACL("reAdd RG_ACL[%d]",*acl_filter_idx);

    //check the aclSWEntry has been used, reAdd must add aclSWEtry in the assigned acl_filter_idx
    ASSERT_EQ(_rtk_rg_aclSWEntry_get(*acl_filter_idx, &rg_db.systemGlobal.aclSWEntry),RT_ERR_RG_OK);

    if(memcmp(&rg_db.systemGlobal.aclSWEntry,&rg_db.systemGlobal.empty_aclSWEntry,sizeof(rtk_rg_aclFilterEntry_t)))
    {
        RETURN_ERR(RT_ERR_RG_ACL_SW_ENTRY_USED);
    }


    //parse acl_filter field to setup aclSWEntry & ASIC

	if(acl_filter->action_type==ACL_ACTION_TYPE_POLICY_ROUTE)
	{
    	//skip add to HWNAT
	}
	else if(acl_filter->filter_fields & INGRESS_WLANDEV_BIT)
	{
		//skip add to HWNAT
		rg_db.systemGlobal.wlanDevPatternValidInACL = 1;
	}
	else{
		if(rg_db.systemGlobal.aclSkipRearrangeHWAclCf==1)
		{
			//skip rearrange H/W
		}else{
			duIdx = FAIL;//init with none-duplicated acl rule index
			if(acl_filter->fwding_type_and_direction!=ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET){ //only latch to cf needs to check.
				_rtk_rg_aclDuplicatedIngressRuleCheck(*acl_filter_idx,acl_filter,&duIdx);
			}

			ret = _rtk_rg_aclSWEntry_to_asic_add(acl_filter,&rg_db.systemGlobal.aclSWEntry,duIdx);
			if(ret!=RT_ERR_RG_OK){
				WARNING("reAdd RG_ACL[%d] fail(error code=%d), discard this rule as below!",*acl_filter_idx,ret);
				_dump_rg_acl_entry_content(NULL, acl_filter);
			}
		}
	}
    rg_db.systemGlobal.aclSWEntry.acl_filter = *acl_filter;
	rg_db.systemGlobal.aclSWEntry.valid = RTK_RG_ENABLED;

    //reAdd must add aclSWEtry in the assigned acl_filter_idx
    ASSERT_EQ(_rtk_rg_aclSWEntry_set(*acl_filter_idx,rg_db.systemGlobal.aclSWEntry),RT_ERR_RG_OK);

    //_rtk_rg_aclSWEntry_dump();

    return (RT_ERR_RG_OK);
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



static int _rtk_rg_aclSWEntry_to_asic_add(rtk_rg_aclFilterAndQos_t *acl_filter,rtk_rg_aclFilterEntry_t* aclSWEntry,int shareHwAclWithSWAclIdx)
{
	int i,j,index,ret,ingress_intf_idx=0;
	int aclLatchToCfIdx=FAIL,cfLatchToAclIdx=FAIL;
	int aclActionRuleIdx,aclCountinueRuleSize,cfRuleIdx;
	uint8 firstHitFlag,aclLatchToCfFlag=DISABLED;
	rtk_portmask_t mac_pmsk;
	rtk_portmask_t ext_pmsk;

	int portmask_need_double_hw_rule=0;
	int portmask_need_double_hw_rule_add_sencond=0;

#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
	int first_keep=0, binding_mask=0,temp_mask=0,LAN_num=0,LAN_count=0;
	unsigned int bit_mask=0;
	rtk_rg_acl_transform_type_t transform_type=RG_ACL_TRANS_NONE;
	rtk_rg_portmask_t egress_port_binding_mask;
	unsigned long long int saved_filter_fields=acl_filter->filter_fields;
	int egress_intf_idx=0;
#endif

	aclSWEntry->hw_aclEntry_start = 0;
	aclSWEntry->hw_aclEntry_size = 0;


	if(acl_filter->filter_fields == 0x0) //no patterns need to filter
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);

	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)//pure ACK type can not use CF patterns and actions
	{
		if(acl_filter->filter_fields & RG_PURE_CF_PATTERNS)
			RETURN_ERR(RT_ERR_RG_INVALID_PARAM);

		if(acl_filter->action_type==ACL_ACTION_TYPE_QOS && (acl_filter->qos_actions& RG_PURE_CF_QOS_ACTION))
			RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	}


	ASSERT_EQ(_rtk_rg_portmask_translator(acl_filter->ingress_port_mask, &mac_pmsk, &ext_pmsk),RT_ERR_RG_OK);

#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
	if((acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET) && (acl_filter->filter_fields&EGRESS_INTF_BIT))
	{
		//Check which WAN model we have:
		//if wanType is bridge:
		// 1: with VLAN-binding, ingress SPA+VID
		// 2: with Port-binding, ingress SPA
		//else
		// 1: with VLAN-binding, ingress SPA+VID+DMAC=GMAC
		// 2: with Port-binding, ingress SPA+DMAC=GMAC
		// 3: with non-default route interface, DIP sunbnet+DMAC=GMAC
		// 4: other, ingress SPA without WAN port+DMAC=GMAC
		rg_db.systemGlobal.acl_SW_egress_intf_type_zero_num++;
		egress_intf_idx = acl_filter->egress_intf_idx;
		ret=rtk_rg_apollo_intfInfo_find(&rg_db.systemGlobal.egress_intf_info, &egress_intf_idx);
		if((ret==RT_ERR_RG_OK)&&(egress_intf_idx==acl_filter->egress_intf_idx)&&(rg_db.systemGlobal.egress_intf_info.is_wan))
		{
			memcpy(&egress_port_binding_mask,&rg_db.systemGlobal.egress_intf_info.wan_intf.wan_intf_conf.port_binding_mask,sizeof(rtk_rg_portmask_t));
			DEBUG("WAN intf[%d] vlanbpmsk is %x, portbpmsk is %x!",egress_intf_idx,rg_db.systemGlobal.egress_intf_info.wan_intf.wan_intf_conf.vlan_binding_mask.portmask,egress_port_binding_mask.portmask);
			if(rg_db.systemGlobal.egress_intf_info.wan_intf.wan_intf_conf.vlan_binding_mask.portmask)
			{
				transform_type=RG_ACL_TRANS_L2_VLAN_BIND;
				for(i=0;i<MAX_BIND_SW_TABLE_SIZE;i++)
				{
					if(rg_db.bind[i].valid && rg_db.bind[i].rtk_bind.vidLan!=0 && egress_intf_idx==rg_db.nexthop[rg_db.wantype[rg_db.bind[i].rtk_bind.wanTypeIdx].rtk_wantype.nhIdx].rtk_nexthop.ifIdx)
						binding_mask|=0x1<<i;
				}
				temp_mask=binding_mask;
			}
			else if(egress_port_binding_mask.portmask)
				transform_type=RG_ACL_TRANS_L2_PORT_BIND;

			if(rg_db.systemGlobal.egress_intf_info.wan_intf.wan_intf_conf.wan_type!=RTK_RG_BRIDGE)
			{
				LAN_num=rg_db.systemGlobal.lanIntfTotalNum;
				LAN_count=0;
				if(transform_type!=RG_ACL_TRANS_NONE)
					transform_type+=2;	//L34
				else
				{
					switch(rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ip_version)
					{
						case IPVER_V4ONLY:
							if(rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv4_default_gateway_on)
								transform_type=RG_ACL_TRANS_v4_OTHER;
							else
								transform_type=RG_ACL_TRANS_v4_SUBNET;
							break;
						case IPVER_V6ONLY:
							if(rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_default_gateway_on)
								transform_type=RG_ACL_TRANS_v6_OTHER;
							else
								transform_type=RG_ACL_TRANS_v6_SUBNET;
							break;
						default:
							if(rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv4_default_gateway_on)
								transform_type=RG_ACL_TRANS_v4_OTHER_v6_OTHER;
							else
								transform_type=RG_ACL_TRANS_v4_SUBNET_v6_OTHER;
							if(!rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_default_gateway_on)
								transform_type+=2;
							break;
					}
				}

				if(rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ip_addr==0x0 &&
					rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_addr.ipv6_addr[0]==0 &&
					rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_addr.ipv6_addr[1]==0 &&
					rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_addr.ipv6_addr[2]==0 &&
					rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_addr.ipv6_addr[3]==0 &&
					rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_addr.ipv6_addr[4]==0 &&
					rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_addr.ipv6_addr[5]==0 &&
					rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_addr.ipv6_addr[6]==0 &&
					rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_addr.ipv6_addr[7]==0 &&
					rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_addr.ipv6_addr[8]==0 &&
					rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_addr.ipv6_addr[9]==0 &&
					rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_addr.ipv6_addr[10]==0 &&
					rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_addr.ipv6_addr[11]==0 &&
					rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_addr.ipv6_addr[12]==0 &&
					rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_addr.ipv6_addr[13]==0 &&
					rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_addr.ipv6_addr[14]==0 &&
					rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_addr.ipv6_addr[15]==0 )
				{
						transform_type=RG_ACL_TRANS_L34_NOT_READY;	//IP addr has not been set.
				}

			}else if(transform_type==RG_ACL_TRANS_NONE)
					transform_type=RG_ACL_TRANS_L2; //non-binding bridge interface
TRANSFORM_BEGIN:
			DEBUG("transform begin, type is %d, filter_fields:%llx, LAN_num is %d, LAN_count is %d",transform_type,acl_filter->filter_fields,LAN_num,LAN_count);
			switch(transform_type)
			{
				case RG_ACL_TRANS_L34_VLAN_BIND:
					//DMAC=LGMAC
					if(LAN_num>0)
					{
						acl_filter->filter_fields|=INGRESS_DMAC_BIT;
						memcpy(acl_filter->ingress_dmac.octet,rg_db.systemGlobal.lanIntfGroup[LAN_count].p_intfInfo->p_lanIntfConf->gmac.octet,ETHER_ADDR_LEN);
						acl_filter->ingress_dmac_mask.octet[0]=0xff;
						acl_filter->ingress_dmac_mask.octet[1]=0xff;
						acl_filter->ingress_dmac_mask.octet[2]=0xff;
						acl_filter->ingress_dmac_mask.octet[3]=0xff;
						acl_filter->ingress_dmac_mask.octet[4]=0xff;
						acl_filter->ingress_dmac_mask.octet[5]=0xff;
					}
					else
					{
						temp_mask=0;
						break;
					}
				case RG_ACL_TRANS_L2_VLAN_BIND:
					//ingress SPA+VID
					for(i=0;i<MAX_BIND_SW_TABLE_SIZE;i++)
					{
						if(temp_mask&(0x1<<i))
						{
							temp_mask&=(~(0x1<<i)); //turn off one bit while all LANGMAC had added
							acl_filter->filter_fields|=INGRESS_PORT_BIT;
							acl_filter->ingress_port_mask.portmask=0x0;
							acl_filter->ingress_port_mask.portmask|=rg_db.bind[i].rtk_bind.portMask.bits[0];
							acl_filter->ingress_port_mask.portmask|=rg_db.bind[i].rtk_bind.extPortMask.bits[0]<<RTK_RG_PORT_CPU;
							if(transform_type==RG_ACL_TRANS_L34_VLAN_BIND && !(acl_filter->ingress_port_mask.portmask&rg_db.systemGlobal.lanIntfGroup[LAN_count].p_intfInfo->p_lanIntfConf->port_mask.portmask))
								goto skipAclAsicSetting;
							acl_filter->filter_fields|=INGRESS_CTAG_VID_BIT;
							acl_filter->ingress_ctag_vid=rg_db.bind[i].rtk_bind.vidLan;
							DEBUG("add spa %x vid %d",acl_filter->ingress_port_mask.portmask,acl_filter->ingress_ctag_vid);
							break;
						}
					}
					break;
				case RG_ACL_TRANS_L34_PORT_BIND:
					//DMAC=LGMAC
					if(LAN_num>0)
					{
						acl_filter->filter_fields|=INGRESS_DMAC_BIT;
						memcpy(acl_filter->ingress_dmac.octet,rg_db.systemGlobal.lanIntfGroup[LAN_count].p_intfInfo->p_lanIntfConf->gmac.octet,ETHER_ADDR_LEN);
						acl_filter->ingress_dmac_mask.octet[0]=0xff;
						acl_filter->ingress_dmac_mask.octet[1]=0xff;
						acl_filter->ingress_dmac_mask.octet[2]=0xff;
						acl_filter->ingress_dmac_mask.octet[3]=0xff;
						acl_filter->ingress_dmac_mask.octet[4]=0xff;
						acl_filter->ingress_dmac_mask.octet[5]=0xff;
						if(!(egress_port_binding_mask.portmask&rg_db.systemGlobal.lanIntfGroup[LAN_count].p_intfInfo->p_lanIntfConf->port_mask.portmask))goto skipAclAsicSetting;
					}
					else
						break;
				case RG_ACL_TRANS_L2_PORT_BIND:
					//ingress SPA
					acl_filter->filter_fields|=INGRESS_PORT_BIT;
					acl_filter->ingress_port_mask.portmask=egress_port_binding_mask.portmask;
					break;
				case RG_ACL_TRANS_v6_SUBNET:
				case RG_ACL_TRANS_v4_OTHER_v6_SUBNET:
					//IPv6 DIP sunbnet
#if 0
					acl_filter->filter_fields|=INGRESS_IPV6_DIP_RANGE_BIT;
					memcpy(acl_filter->ingress_dest_ipv6_addr_start,rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_addr.ipv6_addr,IPV6_ADDR_LEN);
					memcpy(acl_filter->ingress_dest_ipv6_addr_end,rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_addr.ipv6_addr,IPV6_ADDR_LEN);
					if((rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_mask_length&0x7)==0)
						bit_mask=0;
					else
						bit_mask=(0xff<<(8-(rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_mask_length&0x7)))&0xff;
					i=rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_mask_length>>3;
					acl_filter->ingress_dest_ipv6_addr_start[i]&=bit_mask;
					acl_filter->ingress_dest_ipv6_addr_end[i]|=((~bit_mask)&0xff);
					memset(&acl_filter->ingress_dest_ipv6_addr_start[i+1],0,IPV6_ADDR_LEN-i-1);
					memset(&acl_filter->ingress_dest_ipv6_addr_end[i+1],0xff,IPV6_ADDR_LEN-i-1);
					DEBUG("ipv6 start %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
						acl_filter->ingress_dest_ipv6_addr_start[0],acl_filter->ingress_dest_ipv6_addr_start[1],acl_filter->ingress_dest_ipv6_addr_start[2],acl_filter->ingress_dest_ipv6_addr_start[3],
						acl_filter->ingress_dest_ipv6_addr_start[4],acl_filter->ingress_dest_ipv6_addr_start[5],acl_filter->ingress_dest_ipv6_addr_start[6],acl_filter->ingress_dest_ipv6_addr_start[7],
						acl_filter->ingress_dest_ipv6_addr_start[8],acl_filter->ingress_dest_ipv6_addr_start[9],acl_filter->ingress_dest_ipv6_addr_start[10],acl_filter->ingress_dest_ipv6_addr_start[11],
						acl_filter->ingress_dest_ipv6_addr_start[12],acl_filter->ingress_dest_ipv6_addr_start[13],acl_filter->ingress_dest_ipv6_addr_start[14],acl_filter->ingress_dest_ipv6_addr_start[15]);
					DEBUG("ipv6 end %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
						acl_filter->ingress_dest_ipv6_addr_end[0],acl_filter->ingress_dest_ipv6_addr_end[1],acl_filter->ingress_dest_ipv6_addr_end[2],acl_filter->ingress_dest_ipv6_addr_end[3],
						acl_filter->ingress_dest_ipv6_addr_end[4],acl_filter->ingress_dest_ipv6_addr_end[5],acl_filter->ingress_dest_ipv6_addr_end[6],acl_filter->ingress_dest_ipv6_addr_end[7],
						acl_filter->ingress_dest_ipv6_addr_end[8],acl_filter->ingress_dest_ipv6_addr_end[9],acl_filter->ingress_dest_ipv6_addr_end[10],acl_filter->ingress_dest_ipv6_addr_end[11],
						acl_filter->ingress_dest_ipv6_addr_end[12],acl_filter->ingress_dest_ipv6_addr_end[13],acl_filter->ingress_dest_ipv6_addr_end[14],acl_filter->ingress_dest_ipv6_addr_end[15]);
#else
				//replace using INGRESS_IPV6_DIP_RANGE_BIT(not support in apolloFE) to INGRESS_IPV6_DIP_BIT
				acl_filter->filter_fields|=INGRESS_IPV6_DIP_BIT;
				memcpy(acl_filter->ingress_dest_ipv6_addr,rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_addr.ipv6_addr,IPV6_ADDR_LEN);


				if(rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_mask_length!=128){ //need to concern mask
					i=rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_mask_length>>3; //all 0xff bytes

					if((rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_mask_length&0x7)==0)
					{
						bit_mask=0;
						memset(acl_filter->ingress_dest_ipv6_addr_mask,0xff,i);
					}
					else
					{
						bit_mask=(0xff<<(8-(rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ipv6_mask_length&0x7)))&0xff;
						memset(acl_filter->ingress_dest_ipv6_addr_mask,0xff,i);
						acl_filter->ingress_dest_ipv6_addr_mask[i]=bit_mask;
					}
				}else{//maksLength is 128
					memset(acl_filter->ingress_dest_ipv6_addr_mask,0xff,16);
				}

				DEBUG("ipv6  %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
					acl_filter->ingress_dest_ipv6_addr[0],acl_filter->ingress_dest_ipv6_addr[1],acl_filter->ingress_dest_ipv6_addr[2],acl_filter->ingress_dest_ipv6_addr[3],
					acl_filter->ingress_dest_ipv6_addr[4],acl_filter->ingress_dest_ipv6_addr[5],acl_filter->ingress_dest_ipv6_addr[6],acl_filter->ingress_dest_ipv6_addr[7],
					acl_filter->ingress_dest_ipv6_addr[8],acl_filter->ingress_dest_ipv6_addr[9],acl_filter->ingress_dest_ipv6_addr[10],acl_filter->ingress_dest_ipv6_addr[11],
					acl_filter->ingress_dest_ipv6_addr[12],acl_filter->ingress_dest_ipv6_addr[13],acl_filter->ingress_dest_ipv6_addr[14],acl_filter->ingress_dest_ipv6_addr[15]);
				DEBUG("ipv6 mask %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
					acl_filter->ingress_dest_ipv6_addr_mask[0],acl_filter->ingress_dest_ipv6_addr_mask[1],acl_filter->ingress_dest_ipv6_addr_mask[2],acl_filter->ingress_dest_ipv6_addr_mask[3],
					acl_filter->ingress_dest_ipv6_addr_mask[4],acl_filter->ingress_dest_ipv6_addr_mask[5],acl_filter->ingress_dest_ipv6_addr_mask[6],acl_filter->ingress_dest_ipv6_addr_mask[7],
					acl_filter->ingress_dest_ipv6_addr_mask[8],acl_filter->ingress_dest_ipv6_addr_mask[9],acl_filter->ingress_dest_ipv6_addr_mask[10],acl_filter->ingress_dest_ipv6_addr_mask[11],
					acl_filter->ingress_dest_ipv6_addr_mask[12],acl_filter->ingress_dest_ipv6_addr_mask[13],acl_filter->ingress_dest_ipv6_addr_mask[14],acl_filter->ingress_dest_ipv6_addr_mask[15]);

#endif
				case RG_ACL_TRANS_v4_SUBNET:
				case RG_ACL_TRANS_v4_SUBNET_v6_OTHER:
				case RG_ACL_TRANS_v4_SUBNET_v6_SUBNET:
					if(transform_type!=RG_ACL_TRANS_v6_SUBNET && transform_type!=RG_ACL_TRANS_v4_OTHER_v6_SUBNET)
					{
						//IPv4 DIP sunbnet
						acl_filter->filter_fields|=INGRESS_IPV4_DIP_RANGE_BIT;
						acl_filter->ingress_dest_ipv4_addr_start=(rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ip_addr&rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ip_network_mask);
						acl_filter->ingress_dest_ipv4_addr_end=(rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ip_addr|(~rg_db.systemGlobal.interfaceInfo[egress_intf_idx].p_wanStaticInfo->ip_network_mask));
						DEBUG("ipv4 start %x",acl_filter->ingress_dest_ipv4_addr_start);
						DEBUG("ipv4 end %x",acl_filter->ingress_dest_ipv4_addr_end);
					}
				case RG_ACL_TRANS_v4_OTHER:
				case RG_ACL_TRANS_v6_OTHER:
				case RG_ACL_TRANS_v4_OTHER_v6_OTHER:
					if(transform_type==RG_ACL_TRANS_v4_OTHER)
					{
						acl_filter->filter_fields|=INGRESS_IPV4_TAGIF_BIT;
						acl_filter->ingress_ipv4_tagif=1;
					}
					if(transform_type==RG_ACL_TRANS_v6_OTHER)
					{
						acl_filter->filter_fields|=INGRESS_IPV6_TAGIF_BIT;
						acl_filter->ingress_ipv6_tagif=1;
					}
					if(LAN_num-->0)
					{
						//ingress SPA without WAN port+DMAC=LGMAC
						acl_filter->filter_fields|=INGRESS_PORT_BIT;
						acl_filter->ingress_port_mask.portmask=rg_db.systemGlobal.lanIntfGroup[LAN_count].p_intfInfo->p_lanIntfConf->port_mask.portmask;
						acl_filter->filter_fields|=INGRESS_DMAC_BIT;
						memcpy(acl_filter->ingress_dmac.octet,rg_db.systemGlobal.lanIntfGroup[LAN_count++].p_intfInfo->p_lanIntfConf->gmac.octet,ETHER_ADDR_LEN);
						acl_filter->ingress_dmac_mask.octet[0]=0xff;
						acl_filter->ingress_dmac_mask.octet[1]=0xff;
						acl_filter->ingress_dmac_mask.octet[2]=0xff;
						acl_filter->ingress_dmac_mask.octet[3]=0xff;
						acl_filter->ingress_dmac_mask.octet[4]=0xff;
						acl_filter->ingress_dmac_mask.octet[5]=0xff;
						DEBUG("ingress pmsk=%x, dmac is %02x:%02x:%02x:%02x:%02x:%02x",acl_filter->ingress_port_mask.portmask,
							acl_filter->ingress_dmac.octet[0],acl_filter->ingress_dmac.octet[1],acl_filter->ingress_dmac.octet[2],
							acl_filter->ingress_dmac.octet[3],acl_filter->ingress_dmac.octet[4],acl_filter->ingress_dmac.octet[5]);
					}
					break;
				case RG_ACL_TRANS_L2:
				case RG_ACL_TRANS_L34_NOT_READY:
				default:
					//Do nothing
					break;
			}

			if(!(acl_filter->filter_fields & (INGRESS_PORT_BIT|INGRESS_DMAC_BIT|INGRESS_IPV4_TAGIF_BIT|INGRESS_IPV6_TAGIF_BIT|INGRESS_IPV4_DIP_RANGE_BIT|INGRESS_IPV6_DIP_BIT|INGRESS_CTAG_VID_BIT))){
				DEBUG("EGRESS_INTF_BIT transform to null pattern, so enable all lan port.");
				acl_filter->filter_fields|=INGRESS_PORT_BIT;
				acl_filter->ingress_port_mask.portmask=RTK_RG_ALL_LAN_PORTMASK;
			}

			ASSERT_EQ(_rtk_rg_portmask_translator(acl_filter->ingress_port_mask, &mac_pmsk, &ext_pmsk),RT_ERR_RG_OK);
		}
		else
		{
			WARNING("INTF[%d] is not exist when using WAN_INTF_TRANSFORM(fwdtype = ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET), this rule will be discard!",acl_filter->egress_intf_idx);
			return RT_ERR_RG_ENTRY_NOT_EXIST;
		}


	}
#endif

	//clean portmask_need_double_hw_rule record, if TRANSFROM need to add more then two set of ACLs.
	portmask_need_double_hw_rule=0;
	portmask_need_double_hw_rule_add_sencond=0;

	if((acl_filter->ingress_port_mask.portmask&((1<<RTK_RG_PORT_CPU)|(1<<RTK_RG_EXT_PORT0)))==(((1<<RTK_RG_PORT_CPU)|(1<<RTK_RG_EXT_PORT0)))){ //apolloFE series using at most all_mac_port + ext0
		DEBUG("ingress_port_mask include all ext ports, just need one ACL that include cpu port");
		ext_pmsk.bits[0] = 0x0;
		portmask_need_double_hw_rule=0;
	}
	else if((acl_filter->ingress_port_mask.portmask &RTK_RG_ALL_MAC_PORTMASK) &&(acl_filter->ingress_port_mask.portmask&RTK_RG_ALL_EXT_PORTMASK)) //portmask include mac_ports and ext_ports, need double ACLs due to EXTPORT pattern
	{
		DEBUG("rule include mac_port=0x%x and extport=0x%x",mac_pmsk.bits[0],ext_pmsk.bits[0]);
		portmask_need_double_hw_rule=1;
	}
	//DEBUG("##TRANSFROMED acl_filter->ingress_port_mask=0x%x mac_pmsk=0x%x  ext_pmsk=0x%x###",acl_filter->ingress_port_mask,mac_pmsk.bits[0],ext_pmsk.bits[0]);


PORTMASK_NEED_DOUBLE_RULE:
	aclCountinueRuleSize=0;


	//init global parameters
	bzero(rg_db.systemGlobal.aclRuleValid,sizeof(uint8)*GLOBAL_ACL_RULE_SIZE);
	bzero(&rg_db.systemGlobal.aclFieldEmpty,sizeof(rtk_acl_field_t));
	bzero(rg_db.systemGlobal.aclField,sizeof(rtk_acl_field_t)*GLOBAL_ACL_FIELD_SIZE);
	bzero(rg_db.systemGlobal.aclRule,sizeof(rtk_acl_ingress_entry_t)*GLOBAL_ACL_RULE_SIZE);
	bzero(&rg_db.systemGlobal.aclRuleEmpty,sizeof(rtk_acl_ingress_entry_t));
	bzero(&rg_db.systemGlobal.classifyFieldEmpty,sizeof(rtk_classify_field_t));
	bzero(rg_db.systemGlobal.classifyField,sizeof(rtk_classify_field_t)*GLOBAL_CF_FIELD_SIZE);
	bzero(rg_db.systemGlobal.classifyEntry,sizeof(rtk_classify_cfg_t)*GLOBAL_CF_RULE_SIZE);

	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP
		|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN
		|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP
		|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT)
	{
		//init rg_db.systemGlobal.classifyEntry without occupied action control bit.
		rg_db.systemGlobal.classifyEntry[0].act.usAct.csAct = CLASSIFY_US_CSACT_ACTCTRL_DISABLE;
		rg_db.systemGlobal.classifyEntry[0].act.usAct.cAct = CLASSIFY_US_CACT_ACTCTRL_DISABLE;
		rg_db.systemGlobal.classifyEntry[0].act.usAct.sidQidAct = CLASSIFY_US_SQID_ACT_ACTCTRL_DISABLE;
		rg_db.systemGlobal.classifyEntry[0].act.usAct.interPriAct = CLASSIFY_CF_PRI_ACT_ACTCTRL_DISABLE;
		rg_db.systemGlobal.classifyEntry[0].act.usAct.dscp = CLASSIFY_DSCP_ACT_ACTCTRL_DISABLE;
		rg_db.systemGlobal.classifyEntry[0].act.usAct.drop = CLASSIFY_DROP_ACT_ACTCTRL_DISABLE;

		//init for conflict template pattern check
		rg_db.systemGlobal.classifyEntry[0].templateIdx = TEMPLATE_US_CFG_END;
	}
	else if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP
			||acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN
			||acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP
			||acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_PERMIT)
	{
		//init rg_db.systemGlobal.classifyEntry without occupied action control bit.
		rg_db.systemGlobal.classifyEntry[0].act.dsAct.csAct = CLASSIFY_DS_CSACT_ACTCTRL_DISABLE;
		rg_db.systemGlobal.classifyEntry[0].act.dsAct.cAct = CLASSIFY_DS_CACT_ACTCTRL_DISABLE;
		rg_db.systemGlobal.classifyEntry[0].act.dsAct.interPriAct = CLASSIFY_CF_PRI_ACT_ACTCTRL_DISABLE;
		rg_db.systemGlobal.classifyEntry[0].act.dsAct.uniAct = CLASSIFY_DS_UNI_ACT_ACTCTRL_DISABLE;
		rg_db.systemGlobal.classifyEntry[0].act.dsAct.dscp = CLASSIFY_DSCP_ACT_ACTCTRL_DISABLE;

		rg_db.systemGlobal.classifyEntry[0].templateIdx = TEMPLATE_DS_CFG_END;	//init for conflict template pattern check
	}

	/*set patterns*/
	//acl patterns
	//ASSERT_EQ(_rtk_rg_portmask_translator(acl_filter->ingress_port_mask, &mac_pmsk, &ext_pmsk),RT_ERR_RG_OK);


	//transformed when fwdtype is ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
	if((acl_filter->filter_fields & INGRESS_INTF_BIT) && acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
	{
		//use intf idex to get	Datastruct(for check is_wan or is_lan)
		ingress_intf_idx = acl_filter->ingress_intf_idx;
		bzero(&rg_db.systemGlobal.ingress_intf_info,sizeof(rtk_rg_intfInfo_t));

		ASSERT_EQ(rtk_rg_apollo_intfInfo_find(&rg_db.systemGlobal.ingress_intf_info, &ingress_intf_idx),RT_ERR_RG_OK);

		if(rg_db.systemGlobal.ingress_intf_info.is_wan)
		{
			//Wan Interface
			if(rg_db.systemGlobal.ingress_intf_info.wan_intf.wan_intf_conf.wan_type == RTK_RG_BRIDGE)//L2 wan
			{

				if(rg_db.systemGlobal.ingress_intf_info.wan_intf.wan_intf_conf.egress_vlan_tag_on)
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
					rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldUnion.data.value = rg_db.systemGlobal.ingress_intf_info.wan_intf.wan_intf_conf.egress_vlan_id;
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

				if(rg_db.systemGlobal.ingress_intf_info.wan_intf.wan_intf_conf.egress_vlan_tag_on)
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
					rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldUnion.data.value = rg_db.systemGlobal.ingress_intf_info.wan_intf.wan_intf_conf.egress_vlan_id;
					rg_db.systemGlobal.aclField[TEMPLATE_CTAG].fieldUnion.data.mask = 0x0fff; //do not compare with CPRI[15:3] & CFI[12]


					//setup gmac
					rg_db.systemGlobal.aclField[TEMPLATE_DMAC0].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField[TEMPLATE_DMAC0].fieldUnion.pattern.fieldIdx = TEMPLATE_DMAC0;
					rg_db.systemGlobal.aclField[TEMPLATE_DMAC0].fieldUnion.data.value = (rg_db.systemGlobal.ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[4]<<8) | (rg_db.systemGlobal.ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[5]);
					rg_db.systemGlobal.aclField[TEMPLATE_DMAC0].fieldUnion.data.mask = 0xffff;

					rg_db.systemGlobal.aclField[TEMPLATE_DMAC1].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField[TEMPLATE_DMAC1].fieldUnion.pattern.fieldIdx = TEMPLATE_DMAC1;
					rg_db.systemGlobal.aclField[TEMPLATE_DMAC1].fieldUnion.data.value = (rg_db.systemGlobal.ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[2]<<8) | (rg_db.systemGlobal.ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[3]);
					rg_db.systemGlobal.aclField[TEMPLATE_DMAC1].fieldUnion.data.mask = 0xffff;

					rg_db.systemGlobal.aclField[TEMPLATE_DMAC2].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField[TEMPLATE_DMAC2].fieldUnion.pattern.fieldIdx = TEMPLATE_DMAC2;
					rg_db.systemGlobal.aclField[TEMPLATE_DMAC2].fieldUnion.data.value = (rg_db.systemGlobal.ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[0]<<8) | (rg_db.systemGlobal.ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[1]);
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
					rg_db.systemGlobal.aclField[TEMPLATE_DMAC0].fieldUnion.data.value = (rg_db.systemGlobal.ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[4]<<8) | (rg_db.systemGlobal.ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[5]);
					rg_db.systemGlobal.aclField[TEMPLATE_DMAC0].fieldUnion.data.mask = 0xffff;

					rg_db.systemGlobal.aclField[TEMPLATE_DMAC1].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField[TEMPLATE_DMAC1].fieldUnion.pattern.fieldIdx = TEMPLATE_DMAC1;
					rg_db.systemGlobal.aclField[TEMPLATE_DMAC1].fieldUnion.data.value = (rg_db.systemGlobal.ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[2]<<8) | (rg_db.systemGlobal.ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[3]);
					rg_db.systemGlobal.aclField[TEMPLATE_DMAC1].fieldUnion.data.mask = 0xffff;

					rg_db.systemGlobal.aclField[TEMPLATE_DMAC2].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField[TEMPLATE_DMAC2].fieldUnion.pattern.fieldIdx = TEMPLATE_DMAC2;
					rg_db.systemGlobal.aclField[TEMPLATE_DMAC2].fieldUnion.data.value = (rg_db.systemGlobal.ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[0]<<8) | (rg_db.systemGlobal.ingress_intf_info.wan_intf.wan_intf_conf.gmac.octet[1]);
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
					if((rg_db.systemGlobal.ingress_intf_info.lan_intf.port_mask.portmask&(1<<j)))
					{
						rg_db.systemGlobal.aclRule[i].activePorts.bits[0]|=(1<<j);
					}
				}
			}
		}
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
				//DEBUG("set acl rule for mac_port=0x%x",mac_pmsk.bits[0]);
			}
			else
			{
				//add EXT port rules
				for(i=0; i<MAX_ACL_TEMPLATE_SIZE; i++)
				{
					rg_db.systemGlobal.aclRule[i].activePorts.bits[0] = (1<<RTK_RG_PORT_CPU);
				}

				if(ext_pmsk.bits[0]){
					rg_db.systemGlobal.aclField[TEMPLATE_EXTPORTMASK].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField[TEMPLATE_EXTPORTMASK].fieldUnion.pattern.fieldIdx = (TEMPLATE_EXTPORTMASK%ACL_PER_RULE_FIELD_SIZE);;
					rg_db.systemGlobal.aclField[TEMPLATE_EXTPORTMASK].fieldUnion.data.value = 0x0;
					for(i=0;i<RTK_RG_MAX_EXT_PORT;i++){
						if(!(ext_pmsk.bits[0]&(1<<i))){
							rg_db.systemGlobal.aclField[TEMPLATE_EXTPORTMASK].fieldUnion.data.mask |= (1<<i); //band not allowed ext_port
						}
					}
				}
				//DEBUG("set acl rule for ext_port=0x%x  (pattern_mask=0x%x)",ext_pmsk.bits[0],rg_db.systemGlobal.aclField[TEMPLATE_EXTPORTMASK].fieldUnion.data.mask);

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
	else
	{

#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
		if((acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET) && (acl_filter->filter_fields&EGRESS_INTF_BIT)==0x0)
			WARNING("ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET should assign ingress_port_mask!");
#else
		if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
			WARNING("ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET should assign ingress_port_mask!");
#endif
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
		for(i=0; i<GLOBAL_ACL_RULE_SIZE; i++)
		{
			rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_TCP].value=1;
			rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_TCP].mask=0xffff;
		}
	}

	if(acl_filter->filter_fields & INGRESS_L4_UDP_BIT)
	{
		for(i=0; i<GLOBAL_ACL_RULE_SIZE; i++)
		{
			rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_UDP].value=1;
			rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_UDP].mask=0xffff;
		}
	}


	if(acl_filter->filter_fields & INGRESS_L4_NONE_TCP_NONE_UDP_BIT)
	{
		for(i=0; i<GLOBAL_ACL_RULE_SIZE; i++)
		{
			rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_UDP].value=0;
			rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_UDP].mask=0xffff;

			rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_TCP].value=0;
			rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_TCP].mask=0xffff;
		}
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
		rg_db.systemGlobal.aclField[TEMPLATE_IP6_TC_NH].fieldUnion.data.value |= (acl_filter->ingress_ipv6_tc); //TC[15:8] + NH[7:0]
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
		else//must latch to CF
		{
			rg_db.systemGlobal.classifyField[PATTERN_CF_STAG_IF].fieldType =CLASSIFY_FIELD_IS_STAG;
			rg_db.systemGlobal.classifyField[PATTERN_CF_STAG_IF].classify_pattern.fieldData.value=acl_filter->ingress_stagIf;
			rg_db.systemGlobal.classifyField[PATTERN_CF_STAG_IF].classify_pattern.fieldData.mask=0x1;
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
		else//must latch to CF
		{
			rg_db.systemGlobal.classifyField[PATTERN_CF_CTAG_IF].fieldType =CLASSIFY_FIELD_IS_CTAG;
			rg_db.systemGlobal.classifyField[PATTERN_CF_CTAG_IF].classify_pattern.fieldData.value=acl_filter->ingress_ctagIf;
			rg_db.systemGlobal.classifyField[PATTERN_CF_CTAG_IF].classify_pattern.fieldData.mask=0x1;
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
			WARNING("INGRESS_L4_POROTCAL_VALUE_BIT must set with INGRESS_IPV4_TAGIF_BIT or INGRESS_IPV6_TAGIF_BIT");
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
		for(i=0; i<GLOBAL_ACL_RULE_SIZE; i++)
		{
			rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_IPV4].value = acl_filter->ingress_ipv4_tagif;
			rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_IPV4].mask = 0xffff;
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_TAGIF_BIT)
	{
		for(i=0; i<GLOBAL_ACL_RULE_SIZE; i++)
		{
			rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_IPV6].value = acl_filter->ingress_ipv6_tagif;
			rg_db.systemGlobal.aclRule[i].careTag.tags[ACL_CARE_TAG_IPV6].mask = 0xffff;
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_SIP_RANGE_BIT)
	{
		if(!((acl_filter->filter_fields&INGRESS_IPV6_TAGIF_BIT)&&(acl_filter->ingress_ipv6_tagif==1)))
		{
			WARNING("INGRESS_IPV6_SIP_RANGE_BIT must set with INGRESS_IPV6_TAGIF_BIT");
			RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
		}

		WARNING("INGRESS_IPV6_SIP_RANGE_BIT is not support yet!");
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_DIP_RANGE_BIT)
	{
		if(!((acl_filter->filter_fields&INGRESS_IPV6_TAGIF_BIT)&&(acl_filter->ingress_ipv6_tagif==1)))
		{
			WARNING("INGRESS_IPV6_DIP_RANGE_BIT must set with INGRESS_IPV6_TAGIF_BIT");
			RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
		}

		WARNING("INGRESS_IPV6_DIP_RANGE_BIT is not support yet!");
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	}


	if(acl_filter->filter_fields & INGRESS_L4_SPORT_RANGE_BIT)
	{
		uint32 pattern_idx;

		if(_rtk_rg_search_acl_exist_portTableEntry(&pattern_idx,acl_filter->ingress_src_l4_port_end,acl_filter->ingress_src_l4_port_start,PORTRANGE_SPORT)==RT_ERR_RG_OK)
		{
			//found there is exist the same port_range entry can be share
			DEBUG("Found acl SportRange[%d] can be share with this rule.",pattern_idx);
		}
		else
		{
			ASSERT_EQ(_rtk_rg_search_acl_empty_portTableEntry(&pattern_idx,RANGE_TABLE_SEARCH_FOR_ACL),RT_ERR_RG_OK);
		}

		bzero(&rg_db.systemGlobal.aclSportRangeEntry, sizeof(rg_db.systemGlobal.aclSportRangeEntry));
		rg_db.systemGlobal.aclSportRangeEntry.index = pattern_idx;
		rg_db.systemGlobal.aclSportRangeEntry.upper_bound=acl_filter->ingress_src_l4_port_end;
		rg_db.systemGlobal.aclSportRangeEntry.lower_bound=acl_filter->ingress_src_l4_port_start;
		rg_db.systemGlobal.aclSportRangeEntry.type = PORTRANGE_SPORT;


		rg_db.systemGlobal.aclField[TEMPLATE_L4PORT_RANGE_16_31].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_L4PORT_RANGE_16_31].fieldUnion.pattern.fieldIdx = (TEMPLATE_L4PORT_RANGE_16_31%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_L4PORT_RANGE_16_31].fieldUnion.data.value |= (1<<(pattern_idx-16)); //ACL actually used H/W range table 16~31
		rg_db.systemGlobal.aclField[TEMPLATE_L4PORT_RANGE_16_31].fieldUnion.data.mask |= (1<<(pattern_idx-16));

		if(rtk_acl_portRange_set(&rg_db.systemGlobal.aclSportRangeEntry))
		{
			//DEBUG("adding acl SportRange table failed");
			RETURN_ERR(RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
		}
		else
		{
			//DEBUG("adding acl SportRange to porttable[%d]",rg_db.systemGlobal.aclSportRangeEntry.index);
			//record the information to aclSWEntry
			aclSWEntry->hw_used_table |= APOLLOFE_RG_ACL_USED_INGRESS_SPORTTABLE;
			aclSWEntry->hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_SPORTTABLE_INDEX] = pattern_idx;
		}
	}

	if(acl_filter->filter_fields & INGRESS_L4_DPORT_RANGE_BIT)
	{
		uint32 pattern_idx;

		if(_rtk_rg_search_acl_exist_portTableEntry(&pattern_idx,acl_filter->ingress_dest_l4_port_end,acl_filter->ingress_dest_l4_port_start,PORTRANGE_DPORT)==RT_ERR_RG_OK)
		{
			//found there is exist the same port_range entry can be share
			DEBUG("Found acl DportRange[%d] can be share with this rule.",pattern_idx);
		}
		else
		{
			ASSERT_EQ(_rtk_rg_search_acl_empty_portTableEntry(&pattern_idx,RANGE_TABLE_SEARCH_FOR_ACL),RT_ERR_RG_OK);
		}

		bzero(&rg_db.systemGlobal.aclDportRangeEntry, sizeof(rg_db.systemGlobal.aclDportRangeEntry));
		rg_db.systemGlobal.aclDportRangeEntry.index = pattern_idx;
		rg_db.systemGlobal.aclDportRangeEntry.upper_bound=acl_filter->ingress_dest_l4_port_end;
		rg_db.systemGlobal.aclDportRangeEntry.lower_bound=acl_filter->ingress_dest_l4_port_start;
		rg_db.systemGlobal.aclDportRangeEntry.type = PORTRANGE_DPORT;

		rg_db.systemGlobal.aclField[TEMPLATE_L4PORT_RANGE_16_31].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_L4PORT_RANGE_16_31].fieldUnion.pattern.fieldIdx = (TEMPLATE_L4PORT_RANGE_16_31%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_L4PORT_RANGE_16_31].fieldUnion.data.value |= (1<<(pattern_idx-16));//ACL actually used H/W range table 16~31
		rg_db.systemGlobal.aclField[TEMPLATE_L4PORT_RANGE_16_31].fieldUnion.data.mask |= (1<<(pattern_idx-16));

		if(rtk_acl_portRange_set(&rg_db.systemGlobal.aclDportRangeEntry))
		{
			//DEBUG("adding acl DportRange table failed");
			RETURN_ERR(RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
		}
		else
		{
			//DEBUG("adding acl DportRange to porttable[%d]",rg_db.systemGlobal.aclDportRangeEntry.index);
			aclSWEntry->hw_used_table |= APOLLOFE_RG_ACL_USED_INGRESS_DPORTTABLE;
			aclSWEntry->hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_DPORTTABLE_INDEX] = pattern_idx;
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV4_SIP_RANGE_BIT)
	{
		uint32 pattern_idx;
		if(!((acl_filter->filter_fields&INGRESS_IPV4_TAGIF_BIT)&&(acl_filter->ingress_ipv4_tagif==1)))
		{
			//WARNING("INGRESS_IPV4_SIP_RANGE_BIT should set with INGRESS_IPV4_TAGIF_BIT");
			//RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
		}

		ASSERT_EQ(_rtk_rg_search_acl_empty_ipTableEntry(&pattern_idx,RANGE_TABLE_SEARCH_FOR_ACL),RT_ERR_RG_OK);

		bzero(&rg_db.systemGlobal.aclSIPRangeEntry, sizeof(rg_db.systemGlobal.aclSIPRangeEntry));
		rg_db.systemGlobal.aclSIPRangeEntry.index = pattern_idx;
		rg_db.systemGlobal.aclSIPRangeEntry.upperIp=acl_filter->ingress_src_ipv4_addr_end;
		rg_db.systemGlobal.aclSIPRangeEntry.lowerIp=acl_filter->ingress_src_ipv4_addr_start;
		rg_db.systemGlobal.aclSIPRangeEntry.type = IPRANGE_IPV4_SIP;

		rg_db.systemGlobal.aclField[TEMPLATE_IP_RANGE_16_31].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_IP_RANGE_16_31].fieldUnion.pattern.fieldIdx = (TEMPLATE_IP_RANGE_16_31%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_IP_RANGE_16_31].fieldUnion.data.value |= (1<<(pattern_idx-16));//ACL actually used H/W range table 16~31
		rg_db.systemGlobal.aclField[TEMPLATE_IP_RANGE_16_31].fieldUnion.data.mask |= (1<<(pattern_idx-16));

		if(rtk_acl_ipRange_set(&rg_db.systemGlobal.aclSIPRangeEntry))
		{
			//DEBUG("adding acl DportRange table failed");
			RETURN_ERR(RT_ERR_RG_ACL_IPTABLE_ACCESS_FAILED);
		}
		else
		{
			//DEBUG("adding acl DportRange to porttable[%d]",rg_db.systemGlobal.aclDportRangeEntry.index);
			aclSWEntry->hw_used_table |= APOLLOFE_RG_ACL_USED_INGRESS_SIP4TABLE;
			aclSWEntry->hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_SIP4TABLE_INDEX] = pattern_idx;
		}

	}

	if(acl_filter->filter_fields & INGRESS_IPV4_DIP_RANGE_BIT)
	{
		uint32 pattern_idx;

		if(!((acl_filter->filter_fields&INGRESS_IPV4_TAGIF_BIT)&&(acl_filter->ingress_ipv4_tagif==1)))
		{
			//WARNING("INGRESS_IPV4_DIP_RANGE_BIT should set with INGRESS_IPV4_TAGIF_BIT");
			//RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
		}

		ASSERT_EQ(_rtk_rg_search_acl_empty_ipTableEntry(&pattern_idx,RANGE_TABLE_SEARCH_FOR_ACL),RT_ERR_RG_OK);

		bzero(&rg_db.systemGlobal.aclDIPRangeEntry, sizeof(rg_db.systemGlobal.aclDIPRangeEntry));
		rg_db.systemGlobal.aclDIPRangeEntry.index = pattern_idx;
		rg_db.systemGlobal.aclDIPRangeEntry.upperIp=acl_filter->ingress_dest_ipv4_addr_end;
		rg_db.systemGlobal.aclDIPRangeEntry.lowerIp=acl_filter->ingress_dest_ipv4_addr_start;
		rg_db.systemGlobal.aclDIPRangeEntry.type = IPRANGE_IPV4_DIP;

		rg_db.systemGlobal.aclField[TEMPLATE_IP_RANGE_16_31].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_IP_RANGE_16_31].fieldUnion.pattern.fieldIdx = (TEMPLATE_IP_RANGE_16_31%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_IP_RANGE_16_31].fieldUnion.data.value |= (1<<(pattern_idx-16));//ACL actually used H/W range table 16~31
		rg_db.systemGlobal.aclField[TEMPLATE_IP_RANGE_16_31].fieldUnion.data.mask |= (1<<(pattern_idx-16));

		if(rtk_acl_ipRange_set(&rg_db.systemGlobal.aclDIPRangeEntry))
		{
			//DEBUG("adding acl DportRange table failed");
			RETURN_ERR(RT_ERR_RG_ACL_IPTABLE_ACCESS_FAILED);
		}
		else
		{
			//DEBUG("adding acl DportRange to porttable[%d]",rg_db.systemGlobal.aclDportRangeEntry.index);
			aclSWEntry->hw_used_table |= APOLLOFE_RG_ACL_USED_INGRESS_DIP4TABLE;
			aclSWEntry->hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_DIP4TABLE_INDEX] = pattern_idx;
		}

	}

	//cf patterns
	if(acl_filter->filter_fields & INGRESS_INTF_BIT)
	{
		if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP
			||acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN
			||acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP
			||acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_PERMIT)
		{
			rg_db.systemGlobal.classifyField[PATTERN_CF_WANINTF].fieldType =CLASSIFY_FIELD_WAN_IF;
			rg_db.systemGlobal.classifyField[PATTERN_CF_WANINTF].classify_pattern.fieldData.value=acl_filter->ingress_intf_idx;
			rg_db.systemGlobal.classifyField[PATTERN_CF_WANINTF].classify_pattern.fieldData.mask=0xf;
		}
		else if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
		{
			//when fwdtype==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET, supported by ACL pattern transform.
		}
		else
		{
			WARNING("INGRESS_INTF_BIT only supported in fwdtype ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_XXXX.");
			RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
		}
	}

#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
	if((acl_filter->fwding_type_and_direction!=ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET) && (acl_filter->filter_fields&EGRESS_INTF_BIT))
#else
	if(acl_filter->filter_fields & EGRESS_INTF_BIT)
#endif
	{
		if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT)
		{
			rg_db.systemGlobal.classifyField[PATTERN_CF_WANINTF].fieldType =CLASSIFY_FIELD_WAN_IF;
			rg_db.systemGlobal.classifyField[PATTERN_CF_WANINTF].classify_pattern.fieldData.value=acl_filter->egress_intf_idx;
			rg_db.systemGlobal.classifyField[PATTERN_CF_WANINTF].classify_pattern.fieldData.mask=0xf;
		}
		else
		{
			WARNING("EGRESS_INTF_BIT only supported in fwdtype ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_XXXX.");
			RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
		}
	}

	if(acl_filter->filter_fields & EGRESS_CTAG_PRI_BIT)
	{
		if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN
			||acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP
			||acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP
			||acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT)
		{
			rg_db.systemGlobal.classifyField[PATTERN_CF_PRI_CFG].fieldType =CLASSIFY_FIELD_INTER_PRI;
			rg_db.systemGlobal.classifyField[PATTERN_CF_PRI_CFG].classify_pattern.fieldData.value=acl_filter->egress_ctag_pri;
			rg_db.systemGlobal.classifyField[PATTERN_CF_PRI_CFG].classify_pattern.fieldData.mask=0x7;

			if(rg_db.systemGlobal.classifyEntry[0].templateIdx==TEMPLATE_US_CFG_INTERNAL_CVID_AND_INTERNAL_PRI)
			{
				WARNING("EGRESS_CTAG_PRI_BIT's template index is conflict to INTERNAL_PRI_BIT(choose one in one rule)");
				RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
			}
			else
			{
				//choose US template[1]
				rg_db.systemGlobal.classifyEntry[0].templateIdx = TEMPLATE_US_CFG_INTERNAL_CVID_AND_EMARKED_PRI;
			}
		}
		else
		{
			WARNING("EGRESS_INTF_BIT only supported in fwdtype ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_XXXX.");
			RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
		}
	}


	if(acl_filter->filter_fields & EGRESS_CTAG_VID_BIT)
	{
		if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN
			||acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP
			||acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP
			||acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT)
		{
			//for compitible of non-mask version (mask not set means all care)
			if(acl_filter->egress_ctag_vid_mask==0x0)
			{
				acl_filter->egress_ctag_vid_mask = 0xfff;
			}
			rg_db.systemGlobal.classifyField[PATTERN_CF_VID_CFG].fieldType =CLASSIFY_FIELD_TAG_VID;
			rg_db.systemGlobal.classifyField[PATTERN_CF_VID_CFG].classify_pattern.fieldData.value=acl_filter->egress_ctag_vid;
			rg_db.systemGlobal.classifyField[PATTERN_CF_VID_CFG].classify_pattern.fieldData.mask=acl_filter->egress_ctag_vid_mask;
		}
		else
		{
			//DS do not have internal ctag vid pattern
			WARNING("EGRESS_CTAG_VID_BIT only supported in fwdtype ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_XXXX.");
			RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
		}
	}


	if(acl_filter->filter_fields & EGRESS_IP4MC_IF){
		//multicast
		rg_db.systemGlobal.classifyField[PATTERN_CF_IPMC_IF].fieldType =CLASSIFY_FIELD_IPMC;
		rg_db.systemGlobal.classifyField[PATTERN_CF_IPMC_IF].classify_pattern.fieldData.value=acl_filter->egress_ip4mc_if;
		rg_db.systemGlobal.classifyField[PATTERN_CF_IPMC_IF].classify_pattern.fieldData.mask=0x1;

		//must be ipv4
		rg_db.systemGlobal.classifyField[PATTERN_CF_IPV4_IF].fieldType =CLASSIFY_FIELD_IPV4;
		rg_db.systemGlobal.classifyField[PATTERN_CF_IPV4_IF].classify_pattern.fieldData.value=0x1;
		rg_db.systemGlobal.classifyField[PATTERN_CF_IPV4_IF].classify_pattern.fieldData.mask=0x1;
	}

	if(acl_filter->filter_fields & EGRESS_IP6MC_IF){
		//multicast
		rg_db.systemGlobal.classifyField[PATTERN_CF_IPMC_IF].fieldType =CLASSIFY_FIELD_IPMC;
		rg_db.systemGlobal.classifyField[PATTERN_CF_IPMC_IF].classify_pattern.fieldData.value=acl_filter->egress_ip6mc_if;
		rg_db.systemGlobal.classifyField[PATTERN_CF_IPMC_IF].classify_pattern.fieldData.mask=0x1;

		//must be ipv6
		rg_db.systemGlobal.classifyField[PATTERN_CF_IPV6_IF].fieldType =CLASSIFY_FIELD_IPV6;
		rg_db.systemGlobal.classifyField[PATTERN_CF_IPV6_IF].classify_pattern.fieldData.value=0x1;
		rg_db.systemGlobal.classifyField[PATTERN_CF_IPV6_IF].classify_pattern.fieldData.mask=0x1;

	}


	if(acl_filter->filter_fields & INGRESS_EGRESS_PORTIDX_BIT)
	{
		//cf [2:0]uni pattern: ingress_port_idx for US, egress_port_idx for DS
		if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT)
		{
			rg_db.systemGlobal.classifyField[PATTERN_CF_UNI].fieldType =CLASSIFY_FIELD_UNI;
			rg_db.systemGlobal.classifyField[PATTERN_CF_UNI].classify_pattern.fieldData.value=acl_filter->ingress_port_idx;
			if(acl_filter->ingress_port_idx_mask==0x0){
				acl_filter->ingress_port_idx_mask=CF_UNI_DEFAULT;
				rg_db.systemGlobal.classifyField[PATTERN_CF_UNI].classify_pattern.fieldData.mask=CF_UNI_DEFAULT;
			}else{
				rg_db.systemGlobal.classifyField[PATTERN_CF_UNI].classify_pattern.fieldData.mask=acl_filter->ingress_port_idx_mask;
			}
		}
		else if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_PERMIT)
		{
			rg_db.systemGlobal.classifyField[PATTERN_CF_UNI].fieldType =CLASSIFY_FIELD_UNI;
			rg_db.systemGlobal.classifyField[PATTERN_CF_UNI].classify_pattern.fieldData.value=acl_filter->egress_port_idx;
			if(acl_filter->egress_port_idx_mask==0x0){
				acl_filter->egress_port_idx_mask=CF_UNI_DEFAULT;
				rg_db.systemGlobal.classifyField[PATTERN_CF_UNI].classify_pattern.fieldData.mask=CF_UNI_DEFAULT;
			}else{
				rg_db.systemGlobal.classifyField[PATTERN_CF_UNI].classify_pattern.fieldData.mask=acl_filter->egress_port_idx_mask;
			}
		}
		else
		{
			WARNING("INGRESS_EGRESS_PORTIDX_BIT only supported in fwdtype ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_XXX_XXXX.");
			RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
		}
	}


	if(acl_filter->filter_fields & INTERNAL_PRI_BIT)
	{
		//cf [7:5]IntPri pattern
		if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT)
		{
			rg_db.systemGlobal.classifyField[PATTERN_CF_PRI_CFG].fieldType =CLASSIFY_FIELD_INTER_PRI;
			rg_db.systemGlobal.classifyField[PATTERN_CF_PRI_CFG].classify_pattern.fieldData.value=acl_filter->internal_pri;
			rg_db.systemGlobal.classifyField[PATTERN_CF_PRI_CFG].classify_pattern.fieldData.mask=0x7;
			if(rg_db.systemGlobal.classifyEntry[0].templateIdx==TEMPLATE_US_CFG_INTERNAL_CVID_AND_EMARKED_PRI)
			{
				WARNING("INTERNAL_PRI_BIT's template index is conflict to EGRESS_CTAG_PRI_BIT (choose one in one rule)");
				RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
			}
			else
			{
				//choose US template[0]
				rg_db.systemGlobal.classifyEntry[0].templateIdx = TEMPLATE_US_CFG_INTERNAL_CVID_AND_INTERNAL_PRI;
			}
		}
		else
		{
			//DS do not have internal pri pattern
			WARNING("INTERNAL_PRI_BIT only supported in fwdtype ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_XXXX.");
			RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
		}
	}

	if(acl_filter->filter_fields & EGRESS_IPV4_SIP_RANGE_BIT)
	{
		uint32 pattern_idx;

		if(!((acl_filter->filter_fields&INGRESS_IPV4_TAGIF_BIT)&&(acl_filter->ingress_ipv4_tagif==1)))
		{
			WARNING("EGRESS_IPV4_SIP_RANGE_BIT should set with INGRESS_IPV4_TAGIF_BIT for safty");
			//RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
		}

		ASSERT_EQ(_rtk_rg_search_acl_empty_ipTableEntry(&pattern_idx,RANGE_TABLE_SEARCH_FOR_CF),RT_ERR_RG_OK);

		bzero(&rg_db.systemGlobal.aclSIPRangeEntry, sizeof(rg_db.systemGlobal.aclSIPRangeEntry));
		rg_db.systemGlobal.aclSIPRangeEntry.index = pattern_idx;
		rg_db.systemGlobal.aclSIPRangeEntry.upperIp=acl_filter->egress_src_ipv4_addr_end;
		rg_db.systemGlobal.aclSIPRangeEntry.lowerIp=acl_filter->egress_src_ipv4_addr_start;
		rg_db.systemGlobal.aclSIPRangeEntry.type = IPRANGE_IPV4_SIP;

		rg_db.systemGlobal.aclField[TEMPLATE_EGR_IP_RANGE_0_15].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_EGR_IP_RANGE_0_15].fieldUnion.pattern.fieldIdx = (TEMPLATE_EGR_IP_RANGE_0_15%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_EGR_IP_RANGE_0_15].fieldUnion.data.value |= (1<<pattern_idx);
		rg_db.systemGlobal.aclField[TEMPLATE_EGR_IP_RANGE_0_15].fieldUnion.data.mask |= (1<<pattern_idx);

		if(rtk_acl_ipRange_set(&rg_db.systemGlobal.aclSIPRangeEntry))
		{
			//DEBUG("adding acl DportRange table failed");
			RETURN_ERR(RT_ERR_RG_ACL_IPTABLE_ACCESS_FAILED);
		}
		else
		{
			//DEBUG("adding acl DportRange to porttable[%d]",rg_db.systemGlobal.aclDportRangeEntry.index);
			aclSWEntry->hw_used_table |= APOLLOFE_RG_ACL_USED_EGRESS_SIP4TABLE;
			aclSWEntry->hw_used_table_index[APOLLOFE_RG_ACL_USED_EGRESS_SIP4TABLE_INDEX] = pattern_idx;
		}
	}

	if(acl_filter->filter_fields & EGRESS_IPV4_DIP_RANGE_BIT)
	{
		uint32 pattern_idx;
		if(!((acl_filter->filter_fields&INGRESS_IPV4_TAGIF_BIT)&&(acl_filter->ingress_ipv4_tagif==1)))
		{
			WARNING("EGRESS_IPV4_DIP_RANGE_BIT should set with INGRESS_IPV4_TAGIF_BITfor safty");
			//RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
		}

		ASSERT_EQ(_rtk_rg_search_acl_empty_ipTableEntry(&pattern_idx,RANGE_TABLE_SEARCH_FOR_CF),RT_ERR_RG_OK);

		bzero(&rg_db.systemGlobal.aclDIPRangeEntry, sizeof(rg_db.systemGlobal.aclDIPRangeEntry));
		rg_db.systemGlobal.aclDIPRangeEntry.index = pattern_idx;
		rg_db.systemGlobal.aclDIPRangeEntry.upperIp=acl_filter->egress_dest_ipv4_addr_end;
		rg_db.systemGlobal.aclDIPRangeEntry.lowerIp=acl_filter->egress_dest_ipv4_addr_start;
		rg_db.systemGlobal.aclDIPRangeEntry.type = IPRANGE_IPV4_DIP;

		rg_db.systemGlobal.aclField[TEMPLATE_EGR_IP_RANGE_0_15].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_EGR_IP_RANGE_0_15].fieldUnion.pattern.fieldIdx = (TEMPLATE_EGR_IP_RANGE_0_15%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_EGR_IP_RANGE_0_15].fieldUnion.data.value |= (1<<pattern_idx);
		rg_db.systemGlobal.aclField[TEMPLATE_EGR_IP_RANGE_0_15].fieldUnion.data.mask |= (1<<pattern_idx);

		if(rtk_acl_ipRange_set(&rg_db.systemGlobal.aclDIPRangeEntry))
		{
			//DEBUG("adding acl DportRange table failed");
			RETURN_ERR(RT_ERR_RG_ACL_IPTABLE_ACCESS_FAILED);
		}
		else
		{
			//DEBUG("adding acl DportRange to porttable[%d]",rg_db.systemGlobal.aclDportRangeEntry.index);
			aclSWEntry->hw_used_table |= APOLLOFE_RG_ACL_USED_EGRESS_DIP4TABLE;
			aclSWEntry->hw_used_table_index[APOLLOFE_RG_ACL_USED_EGRESS_DIP4TABLE_INDEX] = pattern_idx;
		}
	}

	if(acl_filter->filter_fields & EGRESS_L4_SPORT_RANGE_BIT)
	{
		uint32 pattern_idx;
		ASSERT_EQ(_rtk_rg_search_acl_empty_portTableEntry(&pattern_idx,RANGE_TABLE_SEARCH_FOR_CF),RT_ERR_RG_OK);
		DEBUG("get empty range-port entry[%d]",pattern_idx);


		bzero(&rg_db.systemGlobal.aclSportRangeEntry, sizeof(rg_db.systemGlobal.aclSportRangeEntry));
		rg_db.systemGlobal.aclSportRangeEntry.index = pattern_idx;
		rg_db.systemGlobal.aclSportRangeEntry.upper_bound=acl_filter->egress_src_l4_port_end;
		rg_db.systemGlobal.aclSportRangeEntry.lower_bound=acl_filter->egress_src_l4_port_start;
		rg_db.systemGlobal.aclSportRangeEntry.type = PORTRANGE_SPORT;


		rg_db.systemGlobal.aclField[TEMPLATE_EGR_L4PORT_RANGE_0_15].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_EGR_L4PORT_RANGE_0_15].fieldUnion.pattern.fieldIdx = (TEMPLATE_EGR_L4PORT_RANGE_0_15%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_EGR_L4PORT_RANGE_0_15].fieldUnion.data.value |= (1<<pattern_idx);
		rg_db.systemGlobal.aclField[TEMPLATE_EGR_L4PORT_RANGE_0_15].fieldUnion.data.mask |= (1<<pattern_idx);

		if(rtk_acl_portRange_set(&rg_db.systemGlobal.aclSportRangeEntry))
		{
			//DEBUG("adding acl SportRange table failed");
			RETURN_ERR(RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
		}
		else
		{
			//DEBUG("adding acl SportRange to porttable[%d]",rg_db.systemGlobal.aclSportRangeEntry.index);
			//record the information to aclSWEntry
			aclSWEntry->hw_used_table |= APOLLOFE_RG_ACL_USED_EGRESS_SPORTTABLE;
			aclSWEntry->hw_used_table_index[APOLLOFE_RG_ACL_USED_EGRESS_SPORTTABLE_INDEX] = pattern_idx;
		}

	}

	if(acl_filter->filter_fields & EGRESS_L4_DPORT_RANGE_BIT)
	{
		uint32 pattern_idx;
		ASSERT_EQ(_rtk_rg_search_acl_empty_portTableEntry(&pattern_idx,RANGE_TABLE_SEARCH_FOR_CF),RT_ERR_RG_OK);
		DEBUG("get empty range-port entry[%d]",pattern_idx);

		bzero(&rg_db.systemGlobal.aclDportRangeEntry, sizeof(rg_db.systemGlobal.aclDportRangeEntry));
		rg_db.systemGlobal.aclDportRangeEntry.index = pattern_idx;
		rg_db.systemGlobal.aclDportRangeEntry.upper_bound=acl_filter->egress_dest_l4_port_end;
		rg_db.systemGlobal.aclDportRangeEntry.lower_bound=acl_filter->egress_dest_l4_port_start;
		rg_db.systemGlobal.aclDportRangeEntry.type = PORTRANGE_DPORT;

		rg_db.systemGlobal.aclField[TEMPLATE_EGR_L4PORT_RANGE_0_15].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_EGR_L4PORT_RANGE_0_15].fieldUnion.pattern.fieldIdx = (TEMPLATE_EGR_L4PORT_RANGE_0_15%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_EGR_L4PORT_RANGE_0_15].fieldUnion.data.value |= (1<<pattern_idx);
		rg_db.systemGlobal.aclField[TEMPLATE_EGR_L4PORT_RANGE_0_15].fieldUnion.data.mask |= (1<<pattern_idx);

		if(rtk_acl_portRange_set(&rg_db.systemGlobal.aclDportRangeEntry))
		{
			//DEBUG("adding acl DportRange table failed");
			RETURN_ERR(RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
		}
		else
		{
			//DEBUG("adding acl DportRange to porttable[%d]",rg_db.systemGlobal.aclDportRangeEntry.index);
			aclSWEntry->hw_used_table |= APOLLOFE_RG_ACL_USED_EGRESS_DPORTTABLE;
			aclSWEntry->hw_used_table_index[APOLLOFE_RG_ACL_USED_EGRESS_DPORTTABLE_INDEX] = pattern_idx;
		}
	}

	if(acl_filter->filter_fields & INGRESS_PKT_LEN_RANGE_BIT)
	{
		uint16 pattern_idx=0;

		ret = _rtk_rg_search_acl_pktLenRangeTableEntry(&pattern_idx,FALSE,acl_filter->ingress_packet_length_end,acl_filter->ingress_packet_length_start);
		if(ret!=RT_ERR_RG_OK){
			WARNING("access HW ACL PKTLEN_RANGE table failed");
			RETURN_ERR(RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
		}

		rg_db.systemGlobal.aclField[TEMPLATE_PKT_LEN_RANGE].fieldType = ACL_FIELD_PATTERN_MATCH;
		rg_db.systemGlobal.aclField[TEMPLATE_PKT_LEN_RANGE].fieldUnion.pattern.fieldIdx = (TEMPLATE_PKT_LEN_RANGE%ACL_PER_RULE_FIELD_SIZE);
		rg_db.systemGlobal.aclField[TEMPLATE_PKT_LEN_RANGE].fieldUnion.data.value |= (1<<(pattern_idx));
		rg_db.systemGlobal.aclField[TEMPLATE_PKT_LEN_RANGE].fieldUnion.data.mask |= (1<<(pattern_idx));

		DEBUG("use HW ACL PKTLEN_RANGE[%d]",pattern_idx);
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
				DEBUG("adding field[%d] to rg_db.systemGlobal.aclRule[%d]",i,j);
				rg_db.systemGlobal.aclRuleValid[j] = ENABLED;
				DEBUG("rg_db.systemGlobal.aclRuleValid[%d] = ENBABLED",j);
				if(rtk_acl_igrRuleField_add(&rg_db.systemGlobal.aclRule[j], &rg_db.systemGlobal.aclField[i]))
				{
					//DEBUG("adding rg_db.systemGlobal.aclRule[0] field[%d] failed",i);
					RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
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
			aclCountinueRuleSize++;

			if(firstHitFlag==0)
			{
				aclActionRuleIdx = i; //record the first rg_db.systemGlobal.aclRule to assign actions.
				firstHitFlag=1;
			}
		}
	}
	DEBUG("aclActionRuleIdx=%d",aclActionRuleIdx);

	//For case: although no rg_db.systemGlobal.aclField is need, but still need one rg_db.systemGlobal.aclRule to filter basic pattern,such as ingress_portmask, tagif ...etc.	and do actions, such as latch_to_cf
	if(memcmp(&rg_db.systemGlobal.aclRule[0],&rg_db.systemGlobal.aclRuleEmpty,sizeof(rtk_acl_ingress_entry_t)) && aclCountinueRuleSize==0)//use rg_db.systemGlobal.aclRule[0] to contant the pattern and action, and add to HW.
	{
		aclCountinueRuleSize = 1;
		rg_db.systemGlobal.aclRuleValid[0]=ENABLED;
	}



	//auto-enabled ipv4_tagif/ipv6_tagif to avoid rule miss-hit (some ipv6-addr rule last 32bit is the same as ipv4-addr)
	if((acl_filter->filter_fields & INGRESS_IPV4_SIP_RANGE_BIT) || (acl_filter->filter_fields & INGRESS_IPV4_DIP_RANGE_BIT)){
		rg_db.systemGlobal.aclRule[aclActionRuleIdx].careTag.tags[ACL_CARE_TAG_IPV4].value=ENABLED;
		rg_db.systemGlobal.aclRule[aclActionRuleIdx].careTag.tags[ACL_CARE_TAG_IPV4].mask=0xffff;
	}
	if((acl_filter->filter_fields & INGRESS_IPV6_SIP_RANGE_BIT) || (acl_filter->filter_fields & INGRESS_IPV6_DIP_RANGE_BIT)||(acl_filter->filter_fields & INGRESS_IPV6_SIP_BIT)||(acl_filter->filter_fields & INGRESS_IPV6_DIP_BIT)){
		rg_db.systemGlobal.aclRule[aclActionRuleIdx].careTag.tags[ACL_CARE_TAG_IPV6].value=ENABLED;
		rg_db.systemGlobal.aclRule[aclActionRuleIdx].careTag.tags[ACL_CARE_TAG_IPV6].mask=0xffff;
	}


	/*set actions*/
	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)//use ACL action
	{
		switch(acl_filter->action_type)
		{
			case ACL_ACTION_TYPE_DROP:
				rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLED;
				rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.forwardAct.act= ACL_IGR_FORWARD_REDIRECT_ACT;
				rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.forwardAct.portMask.bits[0]=0x0;
				break;

			case ACL_ACTION_TYPE_PERMIT:
				rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLED;
				rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.forwardAct.act= ACL_IGR_FORWARD_IGR_MIRROR_ACT;
				rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.forwardAct.portMask.bits[0]= 0x0;
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
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_ROUTE_ACT] = ENABLED;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.routeAct.act= ACL_IGR_ROUTE_1P_REMARK_ACT;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.routeAct.dot1p=acl_filter->action_dot1p_remarking_pri;

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

						case ACL_ACTION_QUEUE_ID_BIT:
							WARNING("ACL_ACTION_QUEUE_ID_BIT is not support, please use ACL_ACTION_ACL_PRIORITY_BIT!");
							RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_SHARE_METER_BIT:
							if(PURE_SW_METER_IDX_OFFSET<=acl_filter->action_share_meter && acl_filter->action_share_meter<(PURE_SW_METER_IDX_OFFSET+PURE_SW_SHAREMETER_TABLE_SIZE)){
								//trap to CPU
								rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLED;
								rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.forwardAct.act= ACL_IGR_FORWARD_TRAP_ACT;
								DEBUG("use trap rather than sharemeter action due to should care sw share meter[%d](idx >= %d)", acl_filter->action_share_meter, PURE_SW_METER_IDX_OFFSET);
							}else{
								rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_LOG_ACT] = ENABLED;
								rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.logAct.act = ACL_IGR_LOG_POLICING_ACT;
								rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.logAct.meter = acl_filter->action_share_meter;
							}
							break;

						case ACL_ACTION_LOG_COUNTER_BIT:
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_LOG_ACT] = ENABLED;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.logAct.act = ACL_IGR_LOG_MIB_ACT;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.logAct.mib = acl_filter->action_log_counter;
							break;

						case ACL_ACTION_STREAM_ID_OR_LLID_BIT:
							WARNING("ACL_ACTION_QUEUE_ID_BIT is not support with fwdtype ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET");
							RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_ACL_PRIORITY_BIT:
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_PRI_ACT] = ENABLED;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.priAct.act = ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.priAct.aclPri = acl_filter->action_acl_priority;
							break;

						case ACL_ACTION_ACL_CVLANTAG_BIT:
							WARNING("ACL_ACTION_ACL_CVLANTAG_BIT is not support with fwdtype ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET");
							RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_ACL_SVLANTAG_BIT:
							WARNING("ACL_ACTION_ACL_SVLANTAG_BIT is not support with fwdtype ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET");
							RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_ACL_INGRESS_VID_BIT:
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_CVLAN_ACT] = ENABLED;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.cvlanAct.act = ACL_IGR_CVLAN_IGR_CVLAN_ACT;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.cvlanAct.cvid = (acl_filter->action_acl_ingress_vid & 0xfff);
							break;

						case ACL_ACTION_DS_UNIMASK_BIT:
							WARNING("ACL_ACTION_ACL_SVLANTAG_BIT is not support with fwdtype ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET");
							RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							break;
						case ACL_ACTION_REDIRECT_BIT:
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLED;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.forwardAct.act = ACL_IGR_FORWARD_REDIRECT_ACT;
							rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.forwardAct.portMask.bits[0]= acl_filter->redirect_portmask;
							break;
						case ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT:
							WARNING("ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT is not support with fwdtype ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET");
							RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
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
				//The L4 flow policy route is supported by software. using NAPT point to nexthop.
				break;

			default:
			break;
		}
	}
	else if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP)
	{
		//enabled acl latch
		rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_INTR_ACT] = ENABLED;
		rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.aclLatch=ENABLED;

		switch(acl_filter->action_type)
		{
			case ACL_ACTION_TYPE_DROP:
				rg_db.systemGlobal.classifyEntry[0].act.usAct.drop = CLASSIFY_DROP_ACT_DROP_PON;
				break;
			case ACL_ACTION_TYPE_PERMIT:
			case ACL_ACTION_TYPE_TRAP:
			case ACL_ACTION_TYPE_QOS:
			case ACL_ACTION_TYPE_TRAP_TO_PS:
			case ACL_ACTION_TYPE_POLICY_ROUTE:
			default:
				WARNING("ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP only support ACL_ACTION_TYPE_DROP action");
				RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
				break;
		}
	}
	else if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP)
	{
		//enabled acl latch
		rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_INTR_ACT] = ENABLED;
		rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.aclLatch=ENABLED;

		switch(acl_filter->action_type)
		{
			case ACL_ACTION_TYPE_DROP:
				rg_db.systemGlobal.classifyEntry[0].act.dsAct.uniAct = CLASSIFY_DS_UNI_ACT_FORCE_FORWARD;
				rg_db.systemGlobal.classifyEntry[0].act.dsAct.uniMask.bits[0]= 0x0;
				break;
			case ACL_ACTION_TYPE_PERMIT:
			case ACL_ACTION_TYPE_TRAP:
			case ACL_ACTION_TYPE_QOS:
			case ACL_ACTION_TYPE_TRAP_TO_PS:
			case ACL_ACTION_TYPE_POLICY_ROUTE:
			default:
				WARNING("ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP only support ACL_ACTION_TYPE_DROP action");
				RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
			break;
		}
	}
	else if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN)
	{
		//enabled acl latch
		rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_INTR_ACT] = ENABLED;
		rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.aclLatch=ENABLED;

		switch(acl_filter->action_type)
		{
			case ACL_ACTION_TYPE_DROP:
			case ACL_ACTION_TYPE_PERMIT:
			case ACL_ACTION_TYPE_TRAP:
			case ACL_ACTION_TYPE_TRAP_TO_PS:
				WARNING("ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN only support ACL_ACTION_TYPE_QOS action");
				RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
				break;
			case ACL_ACTION_TYPE_QOS:
				for(i=0; (0x1<<i)<ACL_ACTION_QOS_END; i++)
				{
					switch((acl_filter->qos_actions&(1<<i)))
					{
						case ACL_ACTION_NOP_BIT: break;
						case ACL_ACTION_1P_REMARKING_BIT:
							WARNING("ACL_ACTION_1P_REMARKING_BIT is only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET, please use ACL_ACTION_ACL_CVLANTAG_BIT!");
							RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT:
							rg_db.systemGlobal.classifyEntry[0].act.usAct.dscp = CLASSIFY_DSCP_ACT_ENABLE;
							rg_db.systemGlobal.classifyEntry[0].act.usAct.dscpVal= (acl_filter->action_ip_precedence_remarking_pri<<3);
							break;

						case ACL_ACTION_DSCP_REMARKING_BIT:
							rg_db.systemGlobal.classifyEntry[0].act.usAct.dscp = CLASSIFY_DSCP_ACT_ENABLE;
							rg_db.systemGlobal.classifyEntry[0].act.usAct.dscpVal= acl_filter->action_dscp_remarking_pri;
							break;

						case ACL_ACTION_QUEUE_ID_BIT:
							WARNING("ACL_ACTION_QUEUE_ID_BIT is not support, please use ACL_ACTION_ACL_PRIORITY_BIT!");
							RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_SHARE_METER_BIT:
							WARNING("ACL_ACTION_SHARE_METER_BIT is only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET!");
							RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_LOG_COUNTER_BIT:
							WARNING("ACL_ACTION_LOG_COUNTER_BIT is only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET!");
							RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							break;


						case ACL_ACTION_STREAM_ID_OR_LLID_BIT:
							rg_db.systemGlobal.classifyEntry[0].act.usAct.sidQidAct = CLASSIFY_US_SQID_ACT_ASSIGN_SID;
							rg_db.systemGlobal.classifyEntry[0].act.usAct.sidQid= acl_filter->action_stream_id_or_llid;
							break;

						case ACL_ACTION_ACL_PRIORITY_BIT:
							WARNING("ACL_ACTION_ACL_PRIORITY_BIT is only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET, please use ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT!");
							RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_ACL_CVLANTAG_BIT:
							//ctag decision
							if(acl_filter->action_acl_cvlan.cvlanTagIfDecision==ACL_CVLAN_TAGIF_NOP){rg_db.systemGlobal.classifyEntry[0].act.usAct.cAct=CLASSIFY_US_CACT_NOP;}
							else if(acl_filter->action_acl_cvlan.cvlanTagIfDecision==ACL_CVLAN_TAGIF_TAGGING){rg_db.systemGlobal.classifyEntry[0].act.usAct.cAct=CLASSIFY_US_CACT_ADD_CTAG_8100;}
							else if(acl_filter->action_acl_cvlan.cvlanTagIfDecision==ACL_CVLAN_TAGIF_UNTAG){rg_db.systemGlobal.classifyEntry[0].act.usAct.cAct=CLASSIFY_US_CACT_DEL_CTAG;}
							else if(acl_filter->action_acl_cvlan.cvlanTagIfDecision==ACL_CVLAN_TAGIF_TRANSPARENT){rg_db.systemGlobal.classifyEntry[0].act.usAct.cAct=CLASSIFY_US_CACT_TRANSPARENT;}
							else {
								WARNING("invalid cvlanTagIfDecision");
								RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							}
							//cvid decision
							if(acl_filter->action_acl_cvlan.cvlanCvidDecision==ACL_CVLAN_CVID_NOP){rg_db.systemGlobal.classifyEntry[0].act.usAct.cVidAct=CLASSIFY_US_VID_ACT_NOP;}
							else if(acl_filter->action_acl_cvlan.cvlanCvidDecision==ACL_CVLAN_CVID_ASSIGN){rg_db.systemGlobal.classifyEntry[0].act.usAct.cVidAct=CLASSIFY_US_VID_ACT_ASSIGN;}
							else if(acl_filter->action_acl_cvlan.cvlanCvidDecision==ACL_CVLAN_CVID_COPY_FROM_1ST_TAG){rg_db.systemGlobal.classifyEntry[0].act.usAct.cVidAct=CLASSIFY_US_VID_ACT_FROM_1ST_TAG;}
							else if(acl_filter->action_acl_cvlan.cvlanCvidDecision==ACL_CVLAN_CVID_COPY_FROM_2ND_TAG){rg_db.systemGlobal.classifyEntry[0].act.usAct.cVidAct=CLASSIFY_US_VID_ACT_FROM_2ND_TAG;}
							else if(acl_filter->action_acl_cvlan.cvlanCvidDecision==ACL_CVLAN_CVID_COPY_FROM_INTERNAL_VID){rg_db.systemGlobal.classifyEntry[0].act.usAct.cVidAct=CLASSIFY_US_VID_ACT_FROM_INTERNAL;}
							else if(acl_filter->action_acl_cvlan.cvlanCvidDecision==ACL_CVLAN_CVID_CPOY_FROM_SP2C){WARNING("APOLLOFE_ACL_CVLAN_CVID_CPOY_FROM_SP2C is only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN"); RETURN_ERR(RT_ERR_RG_INVALID_PARAM);}
							else if(acl_filter->action_acl_cvlan.cvlanCvidDecision==ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID){WARNING("APOLLOFE_ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID is only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN"); RETURN_ERR(RT_ERR_RG_INVALID_PARAM);}
							else {
								WARNING("invalid cvlanCvidDecision");
								RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							}
							//cpri decision
							if(acl_filter->action_acl_cvlan.cvlanCpriDecision==ACL_CVLAN_CPRI_NOP){rg_db.systemGlobal.classifyEntry[0].act.usAct.cPriAct=CLASSIFY_US_PRI_ACT_NOP;}
							else if(acl_filter->action_acl_cvlan.cvlanCpriDecision==ACL_CVLAN_CPRI_ASSIGN){rg_db.systemGlobal.classifyEntry[0].act.usAct.cPriAct=CLASSIFY_US_PRI_ACT_ASSIGN;}
							else if(acl_filter->action_acl_cvlan.cvlanCpriDecision==ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG){rg_db.systemGlobal.classifyEntry[0].act.usAct.cPriAct=CLASSIFY_US_PRI_ACT_FROM_1ST_TAG;}
							else if(acl_filter->action_acl_cvlan.cvlanCpriDecision==ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG){rg_db.systemGlobal.classifyEntry[0].act.usAct.cPriAct=CLASSIFY_US_PRI_ACT_FROM_2ND_TAG;}
							else if(acl_filter->action_acl_cvlan.cvlanCpriDecision==ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI){rg_db.systemGlobal.classifyEntry[0].act.usAct.cPriAct=CLASSIFY_US_PRI_ACT_FROM_INTERNAL;}
							else if(acl_filter->action_acl_cvlan.cvlanCpriDecision==ACL_CVLAN_CPRI_COPY_FROM_DSCP_REMAP){rg_db.systemGlobal.classifyEntry[0].act.usAct.cPriAct=CLASSIFY_US_PRI_ACT_FROM_DSCP;}
							else if(acl_filter->action_acl_cvlan.cvlanCpriDecision==ACL_CVLAN_CPRI_COPY_FROM_SP2C){WARNING("APOLLOFE_ACL_CVLAN_CPRI_COPY_FROM_SP2C is only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN"); RETURN_ERR(RT_ERR_RG_INVALID_PARAM);}
							else {
								WARNING("invalid cvlanCpriDecision");
								RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							}
							//cvid
							rg_db.systemGlobal.classifyEntry[0].act.usAct.cTagVid = acl_filter->action_acl_cvlan.assignedCvid;
							//cpri
							rg_db.systemGlobal.classifyEntry[0].act.usAct.cTagPri= acl_filter->action_acl_cvlan.assignedCpri;
							break;

						case ACL_ACTION_ACL_SVLANTAG_BIT:
							//stag decision
							if(acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_NOP){rg_db.systemGlobal.classifyEntry[0].act.usAct.csAct=CLASSIFY_US_CSACT_NOP;}
							else if(acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID){rg_db.systemGlobal.classifyEntry[0].act.usAct.csAct=CLASSIFY_US_CSACT_ADD_TAG_VS_TPID;}
							else if(acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2){rg_db.systemGlobal.classifyEntry[0].act.usAct.csAct=CLASSIFY_US_CSACT_ADD_TAG_VS_TPID2;}
							else if(acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_UNTAG){rg_db.systemGlobal.classifyEntry[0].act.usAct.csAct=CLASSIFY_US_CSACT_DEL_STAG;}
							else if(acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TRANSPARENT){rg_db.systemGlobal.classifyEntry[0].act.usAct.csAct=CLASSIFY_US_CSACT_TRANSPARENT;}
							else if(acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_ORIGINAL_STAG_TPID){rg_db.systemGlobal.classifyEntry[0].act.usAct.csAct=CLASSIFY_US_CSACT_ADD_TAG_STAG_TPID;}
							else {
								WARNING("invalid svlanTagIfDecision");
								RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							}
							//svid decision
							if(acl_filter->action_acl_svlan.svlanSvidDecision==ACL_SVLAN_SVID_NOP){rg_db.systemGlobal.classifyEntry[0].act.usAct.csVidAct=CLASSIFY_US_VID_ACT_NOP;}
							else if(acl_filter->action_acl_svlan.svlanSvidDecision==ACL_SVLAN_SVID_ASSIGN){rg_db.systemGlobal.classifyEntry[0].act.usAct.csVidAct=CLASSIFY_US_VID_ACT_ASSIGN;}
							else if(acl_filter->action_acl_svlan.svlanSvidDecision==ACL_SVLAN_SVID_COPY_FROM_1ST_TAG){rg_db.systemGlobal.classifyEntry[0].act.usAct.csVidAct=CLASSIFY_US_VID_ACT_FROM_1ST_TAG;}
							else if(acl_filter->action_acl_svlan.svlanSvidDecision==ACL_SVLAN_SVID_COPY_FROM_2ND_TAG){rg_db.systemGlobal.classifyEntry[0].act.usAct.csVidAct=CLASSIFY_US_VID_ACT_FROM_2ND_TAG;}
							else if(acl_filter->action_acl_svlan.svlanSvidDecision==ACL_SVLAN_SVID_SP2C){WARNING("APOLLOFE_ACL_SVLAN_SVID_SP2C is only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN"); RETURN_ERR(RT_ERR_RG_INVALID_PARAM);}
							else {
								WARNING("invalid svlanSvidDecision");
								RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							}
							//spri decision
							if(acl_filter->action_acl_svlan.svlanSpriDecision==ACL_SVLAN_SPRI_NOP){rg_db.systemGlobal.classifyEntry[0].act.usAct.csPriAct=CLASSIFY_US_PRI_ACT_NOP;}
							else if(acl_filter->action_acl_svlan.svlanSpriDecision==ACL_SVLAN_SPRI_ASSIGN){rg_db.systemGlobal.classifyEntry[0].act.usAct.csPriAct=CLASSIFY_US_PRI_ACT_ASSIGN;}
							else if(acl_filter->action_acl_svlan.svlanSpriDecision==ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG){rg_db.systemGlobal.classifyEntry[0].act.usAct.csPriAct=CLASSIFY_US_PRI_ACT_FROM_1ST_TAG;}
							else if(acl_filter->action_acl_svlan.svlanSpriDecision==ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG){rg_db.systemGlobal.classifyEntry[0].act.usAct.csPriAct=CLASSIFY_US_PRI_ACT_FROM_2ND_TAG;}
							else if(acl_filter->action_acl_svlan.svlanSpriDecision==ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI){rg_db.systemGlobal.classifyEntry[0].act.usAct.csPriAct=CLASSIFY_US_PRI_ACT_FROM_INTERNAL;}
							else if(acl_filter->action_acl_svlan.svlanSpriDecision==ACL_SVLAN_SPRI_COPY_FROM_DSCP_REMAP){WARNING("APOLLOFE_ACL_SVLAN_SPRI_COPY_FROM_DSCP_REMAP is only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN"); RETURN_ERR(RT_ERR_RG_INVALID_PARAM);}
							else if(acl_filter->action_acl_svlan.svlanSpriDecision==ACL_SVLAN_SPRI_COPY_FROM_SP2C){WARNING("APOLLOFE_ACL_SVLAN_SPRI_COPY_FROM_SP2C is only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN"); RETURN_ERR(RT_ERR_RG_INVALID_PARAM);}
							else {
								WARNING("invalid svlanSpriDecision");
								RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							}
							//svid
							rg_db.systemGlobal.classifyEntry[0].act.usAct.sTagVid=acl_filter->action_acl_svlan.assignedSvid;
							//spri
							rg_db.systemGlobal.classifyEntry[0].act.usAct.sTagPri=acl_filter->action_acl_svlan.assignedSpri;
							break;

						case ACL_ACTION_ACL_INGRESS_VID_BIT:
							WARNING("ACL_ACTION_ACL_PRIORITY_BIT is only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET!");
							RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_DS_UNIMASK_BIT:
							WARNING("ACL_ACTION_ACL_PRIORITY_BIT is only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN!");
							RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_REDIRECT_BIT:
							WARNING("ACL_ACTION_ACL_PRIORITY_BIT is only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET!");
							RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT:
							rg_db.systemGlobal.classifyEntry[0].act.usAct.interPriAct = CLASSIFY_CF_PRI_ACT_ASSIGN;
							rg_db.systemGlobal.classifyEntry[0].act.usAct.cfPri= acl_filter->egress_internal_priority;
							break;

						case ACL_ACTION_QOS_END:
							break;
						default:
							break;
					}
				}
				break;
			case ACL_ACTION_TYPE_POLICY_ROUTE:
				WARNING("ACL_ACTION_TYPE_POLICY_ROUTE only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET");
				RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
				break;
			default:
			break;
		}

	}
	else if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN)
	{

		//enabled acl latch
		rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_INTR_ACT] = ENABLED;
		rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.aclLatch=ENABLED;

		switch(acl_filter->action_type)
		{
			case ACL_ACTION_TYPE_DROP:
			case ACL_ACTION_TYPE_PERMIT:
			case ACL_ACTION_TYPE_TRAP:
			case ACL_ACTION_TYPE_TRAP_TO_PS:
				WARNING("ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN only support ACL_ACTION_TYPE_QOS action");
				RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
				break;

			case ACL_ACTION_TYPE_QOS:
				for(i=0; (0x1<<i)<ACL_ACTION_QOS_END; i++)
				{
					switch((acl_filter->qos_actions&(1<<i)))
					{
						case ACL_ACTION_NOP_BIT: break;
						case ACL_ACTION_1P_REMARKING_BIT:
							WARNING("ACL_ACTION_1P_REMARKING_BIT is only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET, please use ACL_ACTION_ACL_CVLANTAG_BIT!");
							RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT:
							rg_db.systemGlobal.classifyEntry[0].act.dsAct.dscp = CLASSIFY_DSCP_ACT_ENABLE;
							rg_db.systemGlobal.classifyEntry[0].act.dsAct.dscpVal= (acl_filter->action_ip_precedence_remarking_pri<<3);
							break;

						case ACL_ACTION_DSCP_REMARKING_BIT:
							rg_db.systemGlobal.classifyEntry[0].act.dsAct.dscp = CLASSIFY_DSCP_ACT_ENABLE;
							rg_db.systemGlobal.classifyEntry[0].act.dsAct.dscpVal= acl_filter->action_dscp_remarking_pri;
							break;

						case ACL_ACTION_QUEUE_ID_BIT:
							WARNING("ACL_ACTION_QUEUE_ID_BIT is not support, please use ACL_ACTION_ACL_PRIORITY_BIT!");
							RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_SHARE_METER_BIT:
							WARNING("ACL_ACTION_SHARE_METER_BIT is only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET!");
							RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_LOG_COUNTER_BIT:
							WARNING("ACL_ACTION_LOG_COUNTER_BIT is only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET!");
							RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_STREAM_ID_OR_LLID_BIT:
							WARNING("ACL_ACTION_STREAM_ID_OR_LLID_BIT is only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN!");
							RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_ACL_PRIORITY_BIT:
							WARNING("ACL_ACTION_ACL_PRIORITY_BIT is only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET, please use ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT!");
							RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_ACL_CVLANTAG_BIT:
							//ctag decision
							if(acl_filter->action_acl_cvlan.cvlanTagIfDecision==ACL_CVLAN_TAGIF_NOP){rg_db.systemGlobal.classifyEntry[0].act.dsAct.cAct=CLASSIFY_DS_CACT_NOP;}
							else if(acl_filter->action_acl_cvlan.cvlanTagIfDecision==ACL_CVLAN_TAGIF_TAGGING){rg_db.systemGlobal.classifyEntry[0].act.dsAct.cAct=CLASSIFY_DS_CACT_ADD_CTAG_8100;}
							else if(acl_filter->action_acl_cvlan.cvlanTagIfDecision==ACL_CVLAN_TAGIF_UNTAG){rg_db.systemGlobal.classifyEntry[0].act.dsAct.cAct=CLASSIFY_DS_CACT_DEL_CTAG;}
							else if(acl_filter->action_acl_cvlan.cvlanTagIfDecision==ACL_CVLAN_TAGIF_TRANSPARENT){rg_db.systemGlobal.classifyEntry[0].act.dsAct.cAct=CLASSIFY_DS_CACT_TRANSPARENT;}
							else {
								WARNING("invalid cvlanTagIfDecision");
								RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							}
							//cvid decision
							if(acl_filter->action_acl_cvlan.cvlanCvidDecision==ACL_CVLAN_CVID_NOP){rg_db.systemGlobal.classifyEntry[0].act.dsAct.cVidAct=CLASSIFY_DS_VID_ACT_NOP;}
							else if(acl_filter->action_acl_cvlan.cvlanCvidDecision==ACL_CVLAN_CVID_ASSIGN){rg_db.systemGlobal.classifyEntry[0].act.dsAct.cVidAct=CLASSIFY_DS_VID_ACT_ASSIGN;}
							else if(acl_filter->action_acl_cvlan.cvlanCvidDecision==ACL_CVLAN_CVID_COPY_FROM_1ST_TAG){rg_db.systemGlobal.classifyEntry[0].act.dsAct.cVidAct=CLASSIFY_DS_VID_ACT_FROM_1ST_TAG;}
							else if(acl_filter->action_acl_cvlan.cvlanCvidDecision==ACL_CVLAN_CVID_COPY_FROM_2ND_TAG){rg_db.systemGlobal.classifyEntry[0].act.dsAct.cVidAct=CLASSIFY_DS_VID_ACT_FROM_2ND_TAG;}
							else if(acl_filter->action_acl_cvlan.cvlanCvidDecision==ACL_CVLAN_CVID_COPY_FROM_INTERNAL_VID){WARNING("ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN is only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN"); RETURN_ERR(RT_ERR_RG_INVALID_PARAM);}
							else if(acl_filter->action_acl_cvlan.cvlanCvidDecision==ACL_CVLAN_CVID_CPOY_FROM_SP2C){rg_db.systemGlobal.classifyEntry[0].act.dsAct.cVidAct=CLASSIFY_DS_VID_ACT_TRANSLATION_SP2C;}
							else if(acl_filter->action_acl_cvlan.cvlanCvidDecision==ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID){rg_db.systemGlobal.classifyEntry[0].act.dsAct.cVidAct=CLASSIFY_DS_VID_ACT_FROM_LUT;}
							else {
								WARNING("invalid cvlanCvidDecision");
								RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							}
							//cpri decision
							if(acl_filter->action_acl_cvlan.cvlanCpriDecision==ACL_CVLAN_CPRI_NOP){rg_db.systemGlobal.classifyEntry[0].act.dsAct.cPriAct=CLASSIFY_DS_PRI_ACT_NOP;}
							else if(acl_filter->action_acl_cvlan.cvlanCpriDecision==ACL_CVLAN_CPRI_ASSIGN){rg_db.systemGlobal.classifyEntry[0].act.dsAct.cPriAct=CLASSIFY_DS_PRI_ACT_ASSIGN;}
							else if(acl_filter->action_acl_cvlan.cvlanCpriDecision==ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG){rg_db.systemGlobal.classifyEntry[0].act.dsAct.cPriAct=CLASSIFY_DS_PRI_ACT_FROM_1ST_TAG;}
							else if(acl_filter->action_acl_cvlan.cvlanCpriDecision==ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG){rg_db.systemGlobal.classifyEntry[0].act.dsAct.cPriAct=CLASSIFY_DS_PRI_ACT_FROM_2ND_TAG;}
							else if(acl_filter->action_acl_cvlan.cvlanCpriDecision==ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI){rg_db.systemGlobal.classifyEntry[0].act.dsAct.cPriAct=CLASSIFY_DS_PRI_ACT_FROM_INTERNAL;}
							else if(acl_filter->action_acl_cvlan.cvlanCpriDecision==ACL_CVLAN_CPRI_COPY_FROM_DSCP_REMAP){rg_db.systemGlobal.classifyEntry[0].act.dsAct.cPriAct=CLASSIFY_DS_PRI_ACT_FROM_DSCP;}
							else if(acl_filter->action_acl_cvlan.cvlanCpriDecision==ACL_CVLAN_CPRI_COPY_FROM_SP2C){rg_db.systemGlobal.classifyEntry[0].act.dsAct.cPriAct=CLASSIFY_DS_PRI_ACT_TRANSLATION_SP2C;}
							else {
								WARNING("invalid cvlanCpriDecision");
								RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							}
							//cvid
							rg_db.systemGlobal.classifyEntry[0].act.dsAct.cTagVid = acl_filter->action_acl_cvlan.assignedCvid;
							//cpri
							rg_db.systemGlobal.classifyEntry[0].act.dsAct.cTagPri= acl_filter->action_acl_cvlan.assignedCpri;
							break;

						case ACL_ACTION_ACL_SVLANTAG_BIT:
							//stag decision
							if(acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_NOP){rg_db.systemGlobal.classifyEntry[0].act.dsAct.csAct=CLASSIFY_DS_CSACT_NOP;}
							else if(acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID){rg_db.systemGlobal.classifyEntry[0].act.dsAct.csAct=CLASSIFY_DS_CSACT_ADD_TAG_VS_TPID;}
							else if(acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2){rg_db.systemGlobal.classifyEntry[0].act.dsAct.csAct=CLASSIFY_DS_CSACT_ADD_TAG_VS_TPID2;}
							else if(acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_UNTAG){rg_db.systemGlobal.classifyEntry[0].act.dsAct.csAct=CLASSIFY_DS_CSACT_DEL_STAG;}
							else if(acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TRANSPARENT){rg_db.systemGlobal.classifyEntry[0].act.dsAct.csAct=CLASSIFY_DS_CSACT_TRANSPARENT;}
							else if(acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_ORIGINAL_STAG_TPID){rg_db.systemGlobal.classifyEntry[0].act.dsAct.csAct=CLASSIFY_DS_CSACT_ADD_TAG_STAG_TPID;}
							else {
								WARNING("invalid svlanTagIfDecision");
								RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							}
							//svid decision
							if(acl_filter->action_acl_svlan.svlanSvidDecision==ACL_SVLAN_SVID_NOP){rg_db.systemGlobal.classifyEntry[0].act.dsAct.csVidAct=CLASSIFY_DS_VID_ACT_NOP;}
							else if(acl_filter->action_acl_svlan.svlanSvidDecision==ACL_SVLAN_SVID_ASSIGN){rg_db.systemGlobal.classifyEntry[0].act.dsAct.csVidAct=CLASSIFY_DS_VID_ACT_ASSIGN;}
							else if(acl_filter->action_acl_svlan.svlanSvidDecision==ACL_SVLAN_SVID_COPY_FROM_1ST_TAG){rg_db.systemGlobal.classifyEntry[0].act.dsAct.csVidAct=CLASSIFY_DS_VID_ACT_FROM_1ST_TAG;}
							else if(acl_filter->action_acl_svlan.svlanSvidDecision==ACL_SVLAN_SVID_COPY_FROM_2ND_TAG){rg_db.systemGlobal.classifyEntry[0].act.dsAct.csVidAct=CLASSIFY_DS_VID_ACT_FROM_2ND_TAG;}
							else if(acl_filter->action_acl_svlan.svlanSvidDecision==ACL_SVLAN_SVID_SP2C){rg_db.systemGlobal.classifyEntry[0].act.dsAct.csVidAct=CLASSIFY_DS_VID_ACT_TRANSLATION_SP2C;}
							else {
								WARNING("invalid svlanSvidDecision");
								RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							}
							//spri decision
							if(acl_filter->action_acl_svlan.svlanSpriDecision==ACL_SVLAN_SPRI_NOP){rg_db.systemGlobal.classifyEntry[0].act.dsAct.csPriAct=CLASSIFY_DS_PRI_ACT_NOP;}
							else if(acl_filter->action_acl_svlan.svlanSpriDecision==ACL_SVLAN_SPRI_ASSIGN){rg_db.systemGlobal.classifyEntry[0].act.dsAct.csPriAct=CLASSIFY_DS_PRI_ACT_ASSIGN;}
							else if(acl_filter->action_acl_svlan.svlanSpriDecision==ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG){rg_db.systemGlobal.classifyEntry[0].act.dsAct.csPriAct=CLASSIFY_DS_PRI_ACT_FROM_1ST_TAG;}
							else if(acl_filter->action_acl_svlan.svlanSpriDecision==ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG){rg_db.systemGlobal.classifyEntry[0].act.dsAct.csPriAct=CLASSIFY_DS_PRI_ACT_FROM_2ND_TAG;}
							else if(acl_filter->action_acl_svlan.svlanSpriDecision==ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI){rg_db.systemGlobal.classifyEntry[0].act.dsAct.csPriAct=CLASSIFY_DS_PRI_ACT_FROM_INTERNAL;}
							else if(acl_filter->action_acl_svlan.svlanSpriDecision==ACL_SVLAN_SPRI_COPY_FROM_DSCP_REMAP){rg_db.systemGlobal.classifyEntry[0].act.dsAct.csPriAct=CLASSIFY_DS_PRI_ACT_FROM_DSCP;}
							else if(acl_filter->action_acl_svlan.svlanSpriDecision==ACL_SVLAN_SPRI_COPY_FROM_SP2C){rg_db.systemGlobal.classifyEntry[0].act.dsAct.csPriAct=CLASSIFY_DS_PRI_ACT_TRANSLATION_SP2C;}
							else {
								WARNING("invalid svlanSpriDecision");
								RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							}
							//svid
							rg_db.systemGlobal.classifyEntry[0].act.dsAct.sTagVid=acl_filter->action_acl_svlan.assignedSvid;
							//spri
							rg_db.systemGlobal.classifyEntry[0].act.dsAct.sTagPri=acl_filter->action_acl_svlan.assignedSpri;
							break;

						case ACL_ACTION_ACL_INGRESS_VID_BIT:
							WARNING("ACL_ACTION_ACL_PRIORITY_BIT is only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET!");
							RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_DS_UNIMASK_BIT:
							rg_db.systemGlobal.classifyEntry[0].act.dsAct.uniAct=CLASSIFY_DS_UNI_ACT_MASK_BY_UNIMASK;
							rg_db.systemGlobal.classifyEntry[0].act.dsAct.uniMask.bits[0]=acl_filter->downstream_uni_portmask;
							break;

						case ACL_ACTION_REDIRECT_BIT:
							WARNING("ACL_ACTION_ACL_PRIORITY_BIT is only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET!");
							RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
							break;

						case ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT:
							rg_db.systemGlobal.classifyEntry[0].act.dsAct.interPriAct=CLASSIFY_CF_PRI_ACT_ASSIGN;
							rg_db.systemGlobal.classifyEntry[0].act.dsAct.cfPri=acl_filter->egress_internal_priority;
							break;

						case ACL_ACTION_QOS_END:
							break;
						default:
							break;
					}
				}
				break;
			case ACL_ACTION_TYPE_POLICY_ROUTE:
				WARNING("ACL_ACTION_TYPE_POLICY_ROUTE only supported by fwdtype ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET");
				RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
				break;
			default:
			break;
		}

	}
	else if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP)
	{
		//enabled acl latch
		rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_INTR_ACT] = ENABLED;
		rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.aclLatch=ENABLED;

		switch(acl_filter->action_type)
		{
			case ACL_ACTION_TYPE_TRAP:
			case ACL_ACTION_TYPE_TRAP_TO_PS:
				//up trap
				rg_db.systemGlobal.classifyEntry[0].act.usAct.drop = CLASSIFY_DROP_ACT_TRAP;
				break;
			case ACL_ACTION_TYPE_DROP:
			case ACL_ACTION_TYPE_PERMIT:
			case ACL_ACTION_TYPE_QOS:
			case ACL_ACTION_TYPE_POLICY_ROUTE:
			default:
				WARNING("ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP only support ACL_ACTION_TYPE_TRAP/ACL_ACTION_TYPE_TRAP_TO_PS action");
				RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
				break;
		}
	}
	else if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP)
	{
		//enabled acl latch
		rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_INTR_ACT] = ENABLED;
		rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.aclLatch=ENABLED;

		switch(acl_filter->action_type)
		{
			case ACL_ACTION_TYPE_TRAP:
			case ACL_ACTION_TYPE_TRAP_TO_PS:
				//up trap
				rg_db.systemGlobal.classifyEntry[0].act.dsAct.uniAct = CLASSIFY_DS_UNI_ACT_TRAP;
				break;
			case ACL_ACTION_TYPE_DROP:
			case ACL_ACTION_TYPE_PERMIT:
			case ACL_ACTION_TYPE_QOS:
			case ACL_ACTION_TYPE_POLICY_ROUTE:
			default:
				WARNING("ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP only support ACL_ACTION_TYPE_TRAP/ACL_ACTION_TYPE_TRAP_TO_PS action");
				RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
				break;
		}
	}
	else if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT)
	{
		//enabled acl latch
		rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_INTR_ACT] = ENABLED;
		rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.aclLatch=ENABLED;
		switch(acl_filter->action_type)
		{
			case ACL_ACTION_TYPE_PERMIT:
				//occupied trap
				rg_db.systemGlobal.classifyEntry[0].act.usAct.drop = CLASSIFY_DROP_ACT_NONE;
				break;
			case ACL_ACTION_TYPE_TRAP:
			case ACL_ACTION_TYPE_TRAP_TO_PS:
			case ACL_ACTION_TYPE_DROP:
			case ACL_ACTION_TYPE_QOS:
			case ACL_ACTION_TYPE_POLICY_ROUTE:
			default:
				WARNING("ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT only support ACL_ACTION_TYPE_PERMIT action");
				RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
				break;
		}
	}
	else if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_PERMIT)
	{
		//enabled acl latch
		rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_INTR_ACT] = ENABLED;
		rg_db.systemGlobal.aclRule[aclActionRuleIdx].act.aclLatch=ENABLED;
		switch(acl_filter->action_type)
		{
			case ACL_ACTION_TYPE_PERMIT:
				//occupied trap
				rg_db.systemGlobal.classifyEntry[0].act.dsAct.uniAct = CLASSIFY_DS_UNI_ACT_NOP;
				break;
			case ACL_ACTION_TYPE_TRAP:
			case ACL_ACTION_TYPE_TRAP_TO_PS:
			case ACL_ACTION_TYPE_DROP:
			case ACL_ACTION_TYPE_QOS:
			case ACL_ACTION_TYPE_POLICY_ROUTE:
			default:
				WARNING("ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_PERMIT only support ACL_ACTION_TYPE_PERMIT action");
				RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
				break;
		}
	}
	else
	{
		WARNING("invalid fwding_type_and_direction");
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	}


	/*add ACL rule*/
	//setup ACL ASIC
	//the same acl patterns has set before
	if(shareHwAclWithSWAclIdx!=FAIL) {
		aclSWEntry->hw_aclEntry_start = rg_db.systemGlobal.acl_SW_table_entry[shareHwAclWithSWAclIdx].hw_aclEntry_start;
		aclSWEntry->hw_aclEntry_size = rg_db.systemGlobal.acl_SW_table_entry[shareHwAclWithSWAclIdx].hw_aclEntry_size;
		aclLatchToCfIdx = rg_db.systemGlobal.acl_SW_table_entry[shareHwAclWithSWAclIdx].hw_aclEntry_start;
#if 1
		if (rg_db.systemGlobal.aclRuleValid[7]==ENABLED)
		{
			ASSERT_EQ(_rtk_rg_search_acl_empty_Entry(aclCountinueRuleSize, &cfLatchToAclIdx),RT_ERR_RG_OK);
			DEBUG("Get empty ACL[%d] (continue:%d)",cfLatchToAclIdx,aclCountinueRuleSize);

			rg_db.systemGlobal.aclRule[7].activePorts.bits[0] = RTK_RG_ALL_MAC_PORTMASK;
			rg_db.systemGlobal.aclRule[7].act.enableAct[ACL_IGR_CVLAN_ACT]=DISABLED;
			rg_db.systemGlobal.aclRule[7].act.enableAct[ACL_IGR_SVLAN_ACT]=DISABLED;
			rg_db.systemGlobal.aclRule[7].act.enableAct[ACL_IGR_PRI_ACT]=DISABLED;
			rg_db.systemGlobal.aclRule[7].act.enableAct[ACL_IGR_LOG_ACT]=DISABLED;
			rg_db.systemGlobal.aclRule[7].act.enableAct[ACL_IGR_FORWARD_ACT]=DISABLED;
			rg_db.systemGlobal.aclRule[7].act.enableAct[ACL_IGR_ROUTE_ACT]=DISABLED;
			rg_db.systemGlobal.aclRule[7].act.enableAct[ACL_IGR_INTR_ACT]=ENABLED;
			rg_db.systemGlobal.aclRule[7].act.aclLatch = ENABLED;

			rg_db.systemGlobal.aclRule[7].index = cfLatchToAclIdx;
			rg_db.systemGlobal.aclRule[7].valid = ENABLED;
			rg_db.systemGlobal.aclRule[7].templateIdx = 7;
			ret = RTK_ACL_IGRRULEENTRY_ADD(&rg_db.systemGlobal.aclRule[7]);
			if (ret)
			{
				DEBUG("adding rg_db.systemGlobal.aclRule[7] to H/W aclEntry[%d] failed. ret=0x%x",rg_db.systemGlobal.aclRule[7].index,ret);
				RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
			}

			DEBUG("adding rg_db.systemGlobal.aclRule[7] to H/W ACL[%d] success. ret=0x%x",rg_db.systemGlobal.aclRule[7].index,ret);
		}else
			aclLatchToCfFlag=ENABLED;	//for acl to CF latch
#endif
		goto skipAclAsicSetting;
	}



	ASSERT_EQ(_rtk_rg_search_acl_empty_Entry(aclCountinueRuleSize, &aclLatchToCfIdx),RT_ERR_RG_OK);
	DEBUG("Get empty ACL[%d] (continue:%d)",aclLatchToCfIdx,aclCountinueRuleSize);

	index = aclLatchToCfIdx;//index is used for HW ACL countinuous rule.
	for(i=0; i<GLOBAL_ACL_RULE_SIZE; i++)
	{
		if(rg_db.systemGlobal.aclRuleValid[i]==ENABLED)
		{
			if(i<7)
			{
				aclLatchToCfFlag=ENABLED;//rg_db.systemGlobal.aclRule[0~6] is acl latch to cf, but rg_db.systemGlobal.aclRule[7] is cf latch to acl
				if((rg_db.systemGlobal.aclRule[i].act.aclLatch==ENABLED) && (rg_db.systemGlobal.aclRule[i].activePorts.bits[0]==0x0)){
					WARNING("ACL latch to CF without assigning ingress_port_mask! The rule has no meaning.");
				}
			}

			if(i==7) /*cf latch to ACl rule*/
			{
				cfLatchToAclIdx = index;
				rg_db.systemGlobal.aclRule[i].activePorts.bits[0] = RTK_RG_ALL_MAC_PORTMASK;
				rg_db.systemGlobal.aclRule[i].act.enableAct[ACL_IGR_CVLAN_ACT]=DISABLED;
				rg_db.systemGlobal.aclRule[i].act.enableAct[ACL_IGR_SVLAN_ACT]=DISABLED;
				rg_db.systemGlobal.aclRule[i].act.enableAct[ACL_IGR_PRI_ACT]=DISABLED;
				rg_db.systemGlobal.aclRule[i].act.enableAct[ACL_IGR_LOG_ACT]=DISABLED;
				rg_db.systemGlobal.aclRule[i].act.enableAct[ACL_IGR_FORWARD_ACT]=DISABLED;
				rg_db.systemGlobal.aclRule[i].act.enableAct[ACL_IGR_ROUTE_ACT]=DISABLED;
				//enabled latch
				rg_db.systemGlobal.aclRule[i].act.enableAct[ACL_IGR_INTR_ACT]=ENABLED;
				rg_db.systemGlobal.aclRule[i].act.aclLatch = ENABLED;
			}

			rg_db.systemGlobal.aclRule[i].index = index;
			rg_db.systemGlobal.aclRule[i].valid = ENABLED;
			rg_db.systemGlobal.aclRule[i].templateIdx = i;
			ret = RTK_ACL_IGRRULEENTRY_ADD(&rg_db.systemGlobal.aclRule[i]);
			if(ret)
			{
				DEBUG("adding rg_db.systemGlobal.aclRule[%d] to H/W aclEntry[%d] failed. ret=0x%x",i,index,ret);
				RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
			}

			DEBUG("adding rg_db.systemGlobal.aclRule[%d] to H/W ACL[%d] success. ret=0x%x",i,rg_db.systemGlobal.aclRule[i].index,ret);
			index++;
		}
	}
	//record used H/W ACL entry to aclSWEntry


	if(!first_keep)
	{
		aclSWEntry->hw_aclEntry_start = aclLatchToCfIdx;
		aclSWEntry->hw_aclEntry_size = aclCountinueRuleSize;
		first_keep=1;
	}
	else
		aclSWEntry->hw_aclEntry_size += aclCountinueRuleSize;

	DEBUG("hw_aclEntry_start is %d, hw_aclEntry_size is %d",aclSWEntry->hw_aclEntry_start,aclSWEntry->hw_aclEntry_size);


	if( portmask_need_double_hw_rule==1 && portmask_need_double_hw_rule_add_sencond==0){	//rule add for first time
		//DEBUG("ext_port rule added: hw_aclEntry_start=%d, hw_aclEntry_size=%d",aclSWEntry->hw_aclEntry_start,aclSWEntry->hw_aclEntry_size);
		portmask_need_double_hw_rule_add_sencond = 1;
		goto PORTMASK_NEED_DOUBLE_RULE;
	}

	if( portmask_need_double_hw_rule==1 && portmask_need_double_hw_rule_add_sencond==1){	//rule add for second time, restore origianl hw_aclEntry_start and accumulate hw_aclEntry_size
		//DEBUG("mac_port rule added: hw_aclEntry_start=%d, hw_aclEntry_size=%d",aclSWEntry->hw_aclEntry_start,aclSWEntry->hw_aclEntry_size);
	}





#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
	aclCountinueRuleSize=0;
	if(temp_mask)goto TRANSFORM_BEGIN;	//if more bind have to add

	acl_filter->filter_fields=saved_filter_fields;
	switch(transform_type)
	{
		case RG_ACL_TRANS_v4_OTHER_v6_SUBNET:
		case RG_ACL_TRANS_v4_SUBNET_v6_OTHER:
		case RG_ACL_TRANS_v4_SUBNET_v6_SUBNET:
			transform_type-=5;
			LAN_num=rg_db.systemGlobal.lanIntfTotalNum;
			LAN_count=0;
			goto TRANSFORM_BEGIN;
		case RG_ACL_TRANS_L34_VLAN_BIND:
			temp_mask=binding_mask;
		case RG_ACL_TRANS_L34_PORT_BIND:
			LAN_num--;
			LAN_count++;
			if(LAN_num>0)goto TRANSFORM_BEGIN;		//if more LAN have to add
			break;
		default:
			break;
	}
#endif

skipAclAsicSetting:

	/*add CF rule*/
	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP ||
		acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP ||
		acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN ||
		acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN ||
		acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP ||
		acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP ||
		acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT ||
		acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_PERMIT)
	{
		if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT)
		{
			rg_db.systemGlobal.classifyEntry[0].direction = CLASSIFY_DIRECTION_US;
		}
		else if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP
			|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_PERMIT)
		{
			rg_db.systemGlobal.classifyEntry[0].direction = CLASSIFY_DIRECTION_DS;
		}

		//fieldadd to rg_db.systemGlobal.aclRule, and record the valid ACL rule.
		for(i=0; i<GLOBAL_CF_FIELD_SIZE; i++)
		{
			//handle the aclLatchToCf or cfLatchToAcl
			if(i==PATTERN_CF_ACL_LATCH_TO_CF)
			{
				DEBUG("aclLatchToCfIdx=%d",aclLatchToCfIdx);
				if(aclLatchToCfFlag==ENABLED && aclLatchToCfIdx!=FAIL)//some acl is added and need to latch
				{
					rg_db.systemGlobal.classifyField[PATTERN_CF_ACL_LATCH_TO_CF].fieldType =CLASSIFY_FIELD_ACL_HIT;
					rg_db.systemGlobal.classifyField[PATTERN_CF_ACL_LATCH_TO_CF].classify_pattern.fieldData.value=aclLatchToCfIdx;
					rg_db.systemGlobal.classifyField[PATTERN_CF_ACL_LATCH_TO_CF].classify_pattern.fieldData.mask=0x7f;
				}
			}
			if(i==PATTERN_CF_CF_LATCH_TO_ACL)
			{
				DEBUG("cfLatchToAclIdx=%d",cfLatchToAclIdx);
				if(rg_db.systemGlobal.aclRuleValid[7]==ENABLED)//acl[7] is added means cf need to latch back to acl
				{
					rg_db.systemGlobal.classifyField[PATTERN_CF_CF_LATCH_TO_ACL].fieldType =CLASSIFY_FIELD_CF_ACL_HIT;
					rg_db.systemGlobal.classifyField[PATTERN_CF_CF_LATCH_TO_ACL].classify_pattern.fieldData.value=cfLatchToAclIdx;
					rg_db.systemGlobal.classifyField[PATTERN_CF_CF_LATCH_TO_ACL].classify_pattern.fieldData.mask=0x7f;
				}
			}

			if(memcmp(&rg_db.systemGlobal.classifyField[i],&rg_db.systemGlobal.classifyFieldEmpty,sizeof(rtk_classify_field_t)))
			{
				//DEBUG("rg_db.systemGlobal.classifyField[%d] is added",i);
				if(rtk_classify_field_add(&rg_db.systemGlobal.classifyEntry[0], &rg_db.systemGlobal.classifyField[i]))
				{
					DEBUG("adding rg_db.systemGlobal.classifyField[%d] to rg_db.systemGlobal.classifyEntry failed",i);
					RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
				}
			}

		}

		ASSERT_EQ(_rtk_rg_search_cf_empty_Entry(1,&cfRuleIdx),RT_ERR_RG_OK);
		DEBUG("Get empty CF[%d]",cfRuleIdx);
		rg_db.systemGlobal.classifyEntry[0].valid = 1;
		rg_db.systemGlobal.classifyEntry[0].index = cfRuleIdx;
		if(rg_db.systemGlobal.classifyEntry[0].templateIdx==TEMPLATE_US_CFG_END || rg_db.systemGlobal.classifyEntry[0].templateIdx==TEMPLATE_DS_CFG_END)
		{	//The used pattern is not cared template idx. just assign a reasonable value.
			rg_db.systemGlobal.classifyEntry[0].templateIdx = 0;
		}

		ret = RTK_CLASSIFY_CFGENTRY_ADD(&rg_db.systemGlobal.classifyEntry[0]);
		if(ret)
		{
			DEBUG("adding rg_db.systemGlobal.classifyEntry[%d] failed, ret=0x%x",cfRuleIdx,ret);
			DEBUG("dataFieldRaw[0]=0x%x  careFieldRaw[0]=0x%x",rg_db.systemGlobal.classifyEntry[0].field.readField.dataFieldRaw[0],rg_db.systemGlobal.classifyEntry[0].field.readField.careFieldRaw[0]);
			DEBUG("dataFieldRaw[1]=0x%x  careFieldRaw[1]=0x%x",rg_db.systemGlobal.classifyEntry[0].field.readField.dataFieldRaw[1],rg_db.systemGlobal.classifyEntry[0].field.readField.careFieldRaw[1]);
			DEBUG("dataFieldRaw[2]=0x%x  careFieldRaw[2]=0x%x",rg_db.systemGlobal.classifyEntry[0].field.readField.dataFieldRaw[2],rg_db.systemGlobal.classifyEntry[0].field.readField.careFieldRaw[2]);

			RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
		}
		DEBUG("dataFieldRaw[0]=0x%x  careFieldRaw[0]=0x%x",rg_db.systemGlobal.classifyEntry[0].field.readField.dataFieldRaw[0],rg_db.systemGlobal.classifyEntry[0].field.readField.careFieldRaw[0]);
		DEBUG("dataFieldRaw[1]=0x%x  careFieldRaw[1]=0x%x",rg_db.systemGlobal.classifyEntry[0].field.readField.dataFieldRaw[1],rg_db.systemGlobal.classifyEntry[0].field.readField.careFieldRaw[1]);
		DEBUG("dataFieldRaw[2]=0x%x  careFieldRaw[2]=0x%x",rg_db.systemGlobal.classifyEntry[0].field.readField.dataFieldRaw[2],rg_db.systemGlobal.classifyEntry[0].field.readField.careFieldRaw[2]);
		DEBUG("adding rg_db.systemGlobal.classifyEntry to H/W CF[%d] success. ret=0x%x",rg_db.systemGlobal.classifyEntry[0].index,ret);
		aclSWEntry->hw_cfEntry_start = cfRuleIdx;
		aclSWEntry->hw_cfEntry_size = 1;
	}



	return (RT_ERR_RG_OK);
}




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

#if 0
//9602C demo bug, to flush acl table by yourself
	for (i=0; i<MAX_ACL_ENTRY_SIZE; i++)
	{
		rtk_acl_igrRuleEntry_del(i);
	}
#endif

    /*set ACL_EN*/
    ASSERT_EQ(RTK_ACL_IGRSTATE_SET(RTK_RG_PORT0,ENABLED),RT_ERR_OK);
    ASSERT_EQ(RTK_ACL_IGRSTATE_SET(RTK_RG_PORT1,ENABLED),RT_ERR_OK);
    ASSERT_EQ(RTK_ACL_IGRSTATE_SET(RTK_RG_PORT_PON,ENABLED),RT_ERR_OK);
    ASSERT_EQ(RTK_ACL_IGRSTATE_SET(RTK_RG_PORT_CPU,ENABLED),RT_ERR_OK);

    /*set ACL_PERMIT*/
    pAction = FILTER_UNMATCH_PERMIT;
    ASSERT_EQ(RTK_ACL_IGRUNMATCHACTION_SET(RTK_RG_PORT0,pAction),RT_ERR_OK);
    ASSERT_EQ(RTK_ACL_IGRUNMATCHACTION_SET(RTK_RG_PORT1,pAction),RT_ERR_OK);
    ASSERT_EQ(RTK_ACL_IGRUNMATCHACTION_SET(RTK_RG_PORT_PON,pAction),RT_ERR_OK);
    ASSERT_EQ(RTK_ACL_IGRUNMATCHACTION_SET(RTK_RG_PORT_CPU,pAction),RT_ERR_OK);

#ifdef CONFIG_APOLLO_MODEL
#else
    /*init field selector*/
	for(i=0;i<16;i++)
	{
		bzero(&fieldSel,sizeof(fieldSel));
		fieldSel.index = i;
		fieldSel.format = ACL_FORMAT_DEFAULT;
		fieldSel.offset = 0;
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
	aclTemplate.fieldType[5] = ACL_FIELD_USER_VALID; //rsv
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


	//aclTemplate[4]:all rsv, filter userField
    bzero((void*) &aclTemplate, sizeof(aclTemplate));
    aclTemplate.index = 4;
    aclTemplate.fieldType[0] = ACL_FIELD_USER_DEFINED08;
    aclTemplate.fieldType[1] = ACL_FIELD_USER_DEFINED09;
    aclTemplate.fieldType[2] = ACL_FIELD_USER_DEFINED10;
    aclTemplate.fieldType[3] = ACL_FIELD_USER_DEFINED11;
    aclTemplate.fieldType[4] = ACL_FIELD_USER_DEFINED12;
    aclTemplate.fieldType[5] = ACL_FIELD_USER_DEFINED13;
    aclTemplate.fieldType[6] = ACL_FIELD_USER_DEFINED14;
    aclTemplate.fieldType[7] = ACL_FIELD_USER_DEFINED15;
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
	//ACL/CF  IP,PORT range tables is shared in apolloFE, and is initialed by _rtk_rg_acl_asic_init

	int32 i;
	rtk_classify_default_wanIf_t defaultWanIf;
	rtk_classify_template_cfg_pattern0_t p0TemplateCfg;

	//DEBUG("cf_pattern0_size=%d,mib_l2_wanif_idx=%d,mib_mc_wanif_idx=%d",rg_db.systemGlobal.initParam.cf_pattern0_size,rg_db.systemGlobal.initParam.mib_l2_wanif_idx,rg_db.systemGlobal.initParam.mib_mc_wanif_idx );


	//defined L2 and MC default interfcae index
	rg_db.systemGlobal.mib_l2_wanif_idx = DEFAULT_L2_WANIF_IDX;
	rg_db.systemGlobal.mib_mc_wanif_idx = DEFAULT_MC_WANIF_IDX;
	bzero(&defaultWanIf,sizeof(rtk_classify_default_wanIf_t));
	defaultWanIf.l2WanIf = rg_db.systemGlobal.mib_l2_wanif_idx;
	defaultWanIf.mcastWanIf = rg_db.systemGlobal.mib_mc_wanif_idx;
	ASSERT_EQ(rtk_classify_defaultWanIf_set(&defaultWanIf),RT_ERR_OK);

	//defined CF pattern0 size and pattern1 size
	if(rg_kernel.force_cf_pattern0_size_enable==1)
	{
		rg_db.systemGlobal.cf_pattern0_size = rg_kernel.cf_pattern0_size;
		DEBUG("init cf pattern0 size by proc(%d)",rg_db.systemGlobal.cf_pattern0_size);
		ASSERT_EQ(rtk_classify_entryNumPattern1_set(TOTAL_CF_ENTRY_SIZE-rg_db.systemGlobal.cf_pattern0_size),RT_ERR_OK);
	}
	else
	{
		rg_db.systemGlobal.cf_pattern0_size = DEFAULT_CF_PATTERN0_ENTRY_SIZE;
		DEBUG("init cf pattern0 size by DEFAULT_CF_PATTERN0_ENTRY_SIZE(%d)",rg_db.systemGlobal.cf_pattern0_size);
		ASSERT_EQ(rtk_classify_entryNumPattern1_set(TOTAL_CF_ENTRY_SIZE-rg_db.systemGlobal.cf_pattern0_size),RT_ERR_OK);\
	}

	//set CF pattern0 template
	//US template[0]: internalVID + internalPri
	bzero(&p0TemplateCfg,sizeof(p0TemplateCfg));
	p0TemplateCfg.index=0;
	p0TemplateCfg.direction=CLASSIFY_DIRECTION_US;
	p0TemplateCfg.vidCfg=CLASSIFY_TEMPLATE_VID_CFG_INTERNAL_CVID;
	p0TemplateCfg.priCfg=CLASSIFY_TEMPLATE_PRI_CFG_INTERNAL_PRI;
	rtk_classify_templateCfgPattern0_set(&p0TemplateCfg);

	//US template[1]: internalVID + remarkedPri
	bzero(&p0TemplateCfg,sizeof(p0TemplateCfg));
	p0TemplateCfg.index=1;
	p0TemplateCfg.direction=CLASSIFY_DIRECTION_US;
	p0TemplateCfg.vidCfg=CLASSIFY_TEMPLATE_VID_CFG_INTERNAL_CVID;
	p0TemplateCfg.priCfg=CLASSIFY_TEMPLATE_PRI_CFG_REMARK_CPRI;
	rtk_classify_templateCfgPattern0_set(&p0TemplateCfg);


	//DS template[0]: outterVID + ingress ctag Pri
	bzero(&p0TemplateCfg,sizeof(p0TemplateCfg));
	p0TemplateCfg.index=0;
	p0TemplateCfg.direction=CLASSIFY_DIRECTION_DS;
	p0TemplateCfg.vidCfg=CLASSIFY_TEMPLATE_VID_CFG_INGRESS_OUTERTAG_VID;
	p0TemplateCfg.priCfg=CLASSIFY_TEMPLATE_PRI_CFG_INGRESS_CPRI;
	rtk_classify_templateCfgPattern0_set(&p0TemplateCfg);

	//DS template[1]: ingress ctag VID + ingress ctag Pri
	bzero(&p0TemplateCfg,sizeof(p0TemplateCfg));
	p0TemplateCfg.index=1;
	p0TemplateCfg.direction=CLASSIFY_DIRECTION_DS;
	p0TemplateCfg.vidCfg=CLASSIFY_TEMPLATE_VID_CFG_INGRESS_CVID;
	p0TemplateCfg.priCfg=CLASSIFY_TEMPLATE_PRI_CFG_INGRESS_CPRI;
	rtk_classify_templateCfgPattern0_set(&p0TemplateCfg);


	//clear CF pattern 0 entries
	for(i=0;i<rg_db.systemGlobal.cf_pattern0_size;i++){
		ASSERT_EQ(rtk_classify_cfgEntry_del(i),RT_ERR_OK);
	}

	//enabled PON port as CF port,
	//pospond the enabled or not until add Wan interface.
	//ASSERT_EQ(RTK_CLASSIFY_CFSEL_SET(RTK_RG_MAC_PORT_PON, CLASSIFY_CF_SEL_ENABLE),RT_ERR_OK);


	return (RT_ERR_RG_OK);
}


/*(2)RG_ACL APIs and internal APIs*/

int _rtk_rg_aclSWEntry_and_asic_rearrange(void)
{
	int i,j,accumulateIdx,ruleIdx;
	rtk_rg_aclFilterEntry_t aclSWEntry;


	bzero(&aclSWEntry, sizeof(rtk_rg_aclFilterEntry_t));
	bzero(&rg_db.systemGlobal.empty_aclFilter, sizeof(rtk_rg_aclFilterAndQos_t));
	bzero(rg_db.systemGlobal.acl_filter_temp, sizeof(rtk_rg_aclFilterAndQos_t)*MAX_ACL_SW_ENTRY_SIZE);

	DEBUG("================[do ACL rearrange]==================");

	//clear gloable settings, if there is wlanDev pattern rule, the wlanDevPatternValidInACL will be valid by _rtk_rg_aclSWEntry_reAdd()
	rg_db.systemGlobal.wlanDevPatternValidInACL = 0;

#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
	//20141224LUKE: clear egress intf idx
	rg_db.systemGlobal.acl_SW_egress_intf_type_zero_num = 0;
#endif

	//backup all acl_filter for reAdd, and clean all aclSWEntry
	for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
	{
		//backup acl_filter
		ASSERT_EQ(_rtk_rg_aclSWEntry_get(i, &aclSWEntry),RT_ERR_RG_OK);
		rg_db.systemGlobal.acl_filter_temp[i] = aclSWEntry.acl_filter;
		//clean aclSWEntry
		bzero(&aclSWEntry, sizeof(rtk_rg_aclFilterEntry_t));
		ASSERT_EQ(_rtk_rg_aclSWEntry_set(i, aclSWEntry),RT_ERR_RG_OK);
	}

	if(rg_db.systemGlobal.aclSkipRearrangeHWAclCf==1)
	{
		//skip rearrange H/W
	}else{
		//delete H/W ACL , not include reserved entries
		for(i=MIN_ACL_ENTRY_INDEX; i<=MAX_ACL_ENTRY_INDEX; i++)
		{
			if(rtk_acl_igrRuleEntry_del(i))
			{
				RETURN_ERR(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
			}
		}

		//delete H/W  CF(pattern0) , not include reserved entries
		for(i=RESERVED_CF_BEFORE; i<RESERVED_CF_AFTER; i++)
		{
			if(rtk_classify_cfgEntry_del(i))
			{
				RETURN_ERR(RT_ERR_RG_CF_ENTRY_ACCESS_FAILED);
			}
		}

		//delete ACL/CF IP range table
		for(i=0; i<MAX_ACL_IPRANGETABLE_SIZE; i++)
		{
			if(_rtk_rg_free_acl_ipTableEntry(i))
			{
				RETURN_ERR(RT_ERR_RG_ACL_IPTABLE_ACCESS_FAILED);
			}
		}

		//delete ACL/CF Port range table
		for(i=0; i<MAX_ACL_PORTRANGETABLE_SIZE; i++)
		{
			if(_rtk_rg_free_acl_portTableEntry(i))
			{
				RETURN_ERR(RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
			}
		}

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
	//reAdd all reserve aclSWEntry:  type priority as following
		//1. ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET (with acl_weight sorting)
		//2. ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP or ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP
		//3. ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID or ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN
	//ASSERT_EQ(_rtk_rg_rearrange_ACL_weight(&accumulateIdx),RT_ERR_RG_OK);
#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
	ASSERT_EQ(_rtk_rg_rearrange_ACL_weight(&accumulateIdx),RT_ERR_RG_OK);
	ASSERT_EQ(_rtk_rg_rearrange_ACL_weight_for_egress_wan(),RT_ERR_RG_OK);

	//1-5. readd the rules of type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
	for(j=0;j<MAX_ACL_SW_ENTRY_SIZE;j++){
		if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]==-1)
			break;
		ASSERT_EQ(_rtk_rg_aclSWEntry_and_asic_reAdd(&rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j])], &rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]),RT_ERR_RG_OK);
	}
#else
	ASSERT_EQ(_rtk_rg_rearrange_ACL_weight(&accumulateIdx),RT_ERR_RG_OK);

	//1-4. readd the rules of type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
	for(j=0;j<MAX_ACL_SW_ENTRY_SIZE;j++){
		if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]==-1)
			break;
		ASSERT_EQ(_rtk_rg_aclSWEntry_and_asic_reAdd(&rg_db.systemGlobal.acl_filter_temp[(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j])], &rg_db.systemGlobal.acl_SWindex_sorting_by_weight[j]),RT_ERR_RG_OK);
	}
#endif



	// 2. add rule of ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP or ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP
#if 1
	_rtk_rg_rearrange_ACL_weight_for_l34_trap_drop_permit(&accumulateIdx);

#else
	for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
	{
		if(memcmp(&rg_db.systemGlobal.acl_filter_temp[i],&empty_aclFilter,sizeof(empty_aclFilter)))
		{
			if(rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP ||
				rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP ||
				rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP ||
				rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP ||
				rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT ||
				rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_PERMIT)
			{
					ASSERT_EQ(_rtk_rg_aclSWEntry_and_asic_reAdd(&rg_db.systemGlobal.acl_filter_temp[i], &i),RT_ERR_RG_OK);
					rg_db.systemGlobal.acl_SWindex_sorting_by_weight[accumulateIdx]=i;
					accumulateIdx++;
			}
		}
	}
#endif
	// 3 add rule of type ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID or ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN
#if 1
	_rtk_rg_rearrange_ACL_weight_for_l34_Qos(&accumulateIdx);
#else

	for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
	{
		if(memcmp(&rg_db.systemGlobal.acl_filter_temp[i],&empty_aclFilter,sizeof(empty_aclFilter)))
		{
			if( rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN||
				rg_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN)
			{
					ASSERT_EQ(_rtk_rg_aclSWEntry_and_asic_reAdd(&rg_db.systemGlobal.acl_filter_temp[i], &i),RT_ERR_RG_OK);
					rg_db.systemGlobal.acl_SWindex_sorting_by_weight[accumulateIdx]=i;
					accumulateIdx++;
			}
		}
	}
#endif

	//record acl ingress_vid action valid rules for speed up internalVID decision in fwdEngine. (only type of ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET can use this action)
	//20160811Chuck: However, the dot1p remarking and ingress_cvid action both using CACT in H/W ACL, so need to check both action
	for(i=0,j=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
	{
		if(rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i]==-1)//no more rules need to check
			break;

		ruleIdx = rg_db.systemGlobal.acl_SWindex_sorting_by_weight[i];

		if(rg_db.systemGlobal.acl_filter_temp[ruleIdx].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
		{
			if( (rg_db.systemGlobal.acl_filter_temp[ruleIdx].action_type==ACL_ACTION_TYPE_QOS) &&
				((rg_db.systemGlobal.acl_filter_temp[ruleIdx].qos_actions&ACL_ACTION_ACL_INGRESS_VID_BIT)))
			{
				rg_db.systemGlobal.acl_SWindex_sorting_by_weight_and_ingress_cvid_action[j]=ruleIdx;
				j++;
			}
		}

	}

	return (RT_ERR_RG_OK);
}


int _rtk_rg_aclSWEntry_warning(rtk_rg_aclFilterAndQos_t *acl_filter)
{

	if(acl_filter->fwding_type_and_direction == ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
	{
		if(acl_filter->action_type==ACL_ACTION_TYPE_QOS && (acl_filter->qos_actions & ACL_ACTION_REDIRECT_BIT))
		{
			WARNING("ACL_ACTION_REDIRECT_BIT could affect to DROP/TRAP/TRAP_TO_PS action.");
		}
	}


	return (RT_ERR_RG_OK);
}

int _rtk_rg_aclSWEntry_not_support(rtk_rg_aclFilterAndQos_t *acl_filter)
{
	int i,ret,ingress_intf_idx;
	//int acl_SWEntry_index;
    rtk_portmask_t mac_pmsk;
	rtk_portmask_t ext_pmsk;
	rtk_rg_intfInfo_t ingress_intf_info;
	rtk_rg_intfInfo_t egress_intf_info;
	//rtk_rg_aclFilterEntry_t aclSWEntry;


	ASSERT_EQ(_rtk_rg_portmask_translator(acl_filter->ingress_port_mask, &mac_pmsk, &ext_pmsk),RT_ERR_RG_OK);


	//20160701LUKE: disable CF modify STAG if layer2LookupMissFlood2CPU is enabled.
	if(rg_kernel.layer2LookupMissFlood2CPU==RTK_RG_ENABLED &&
		(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN ||
		acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP ||
		acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_PERMIT)&&
		(acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID||
		acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_8100||
		acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C||
		acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2||
		acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_ORIGINAL_STAG_TPID)&&
		(acl_filter->action_acl_svlan.svlanSvidDecision==ACL_SVLAN_SVID_ASSIGN||
		acl_filter->action_acl_svlan.svlanSvidDecision==ACL_SVLAN_SVID_COPY_FROM_1ST_TAG||
		acl_filter->action_acl_svlan.svlanSvidDecision==ACL_SVLAN_SVID_COPY_FROM_2ND_TAG)){
		WARNING("svlanSvidDecision not support when layer2LookupMissFlood2CPU in apollo");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
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


	//check supported actions
	if(acl_filter->action_type==ACL_ACTION_TYPE_DROP ||
		acl_filter->action_type==ACL_ACTION_TYPE_PERMIT ||
		acl_filter->action_type==ACL_ACTION_TYPE_TRAP ||
		acl_filter->action_type==ACL_ACTION_TYPE_QOS ||
		acl_filter->action_type==ACL_ACTION_TYPE_TRAP_TO_PS ||
		acl_filter->action_type==ACL_ACTION_TYPE_POLICY_ROUTE)
	{
		//action can be suport.
	}
	else
	{
		WARNING("ACL action_type is not suported by this platform!");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

    //INGRESS_INTF_BIT trasform to ACL pattern, ctag_if, ingress_cvid, dmac could be limited.
    if((acl_filter->filter_fields & INGRESS_INTF_BIT) && (acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET))
    {
        ingress_intf_idx = acl_filter->ingress_intf_idx;
		bzero(&ingress_intf_info,sizeof(rtk_rg_intfInfo_t));
        ret = rtk_rg_apollo_intfInfo_find(&ingress_intf_info, &ingress_intf_idx);
        if(ret!=RT_ERR_RG_OK)
        {
            return ret;
        }
        if(ingress_intf_idx != acl_filter->ingress_intf_idx)
        {
            //did not get the assigned interface
            RETURN_ERR(RT_ERR_RG_INTF_GET_FAIL);
        }

        if(ingress_intf_info.is_wan) //ingress interface is wan
        {
            //Wan Interface
            if(ingress_intf_info.wan_intf.wan_intf_conf.wan_type == RTK_RG_BRIDGE)
            {
                //wan bridge(L2) mode: judge CVID
                if(ingress_intf_info.wan_intf.wan_intf_conf.egress_vlan_tag_on)
                {
                    if(acl_filter->filter_fields & INGRESS_CTAG_VID_BIT)
                    {
                        //prevent error setting: user set CVID is not same with ingress_wan_interface VID=> conflict!!!
                        if(acl_filter->ingress_ctag_vid!=ingress_intf_info.wan_intf.wan_intf_conf.egress_vlan_id)
                            RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);
                    }
                }
                else
                {
                    //prevent error setting: user set CVID but infress_wan_interface do not need to tag on=> conflict!!!
                    if(acl_filter->filter_fields & INGRESS_CTAG_VID_BIT)
                        RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);
                	if(acl_filter->filter_fields & INGRESS_CTAG_CFI_BIT)
                        RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);
                }
            }
            else
            {
                //wan route(L34) mode: judge CVID + DMAC
                if(ingress_intf_info.wan_intf.wan_intf_conf.egress_vlan_tag_on)
                {
                    if(acl_filter->filter_fields & INGRESS_CTAG_VID_BIT)
                    {
                        //prevent error setting: user set CVID is not same with ingress_wan_interface VID=> conflict!!!
                        if(acl_filter->ingress_ctag_vid!=ingress_intf_info.wan_intf.wan_intf_conf.egress_vlan_id)
                            RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);
                    }

                    if(acl_filter->filter_fields & INGRESS_DMAC_BIT)
                    {
                        //prevent error setting: user set DMAC is not same with ingress_wan_interface gmac=> conflict!!!
                        if(memcmp(&(acl_filter->ingress_dmac),&(ingress_intf_info.wan_intf.wan_intf_conf.gmac),sizeof(rtk_mac_t)))
                            RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);
                    }
                }
                else
                {
                    //prevent error setting: user set CVID but ingress_wan_interface do not need to tag on=> conflict!!!
                    if(acl_filter->filter_fields & INGRESS_CTAG_VID_BIT)
                        RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);
                    if(acl_filter->filter_fields & INGRESS_CTAG_CFI_BIT)
                        RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);

                    if(acl_filter->filter_fields & INGRESS_DMAC_BIT)
                    {
                        //prevent error setting: user set DMAC is not same with ingress_wan_interface gmac=> conflict!!!
                        if(memcmp(&(acl_filter->ingress_dmac),&(ingress_intf_info.wan_intf.wan_intf_conf.gmac),sizeof(rtk_mac_t)))
                            RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);
                    }
                }
            }
        }
        else//ingress interface is lan
        {
            //prevent error setting: user set portmask is not in lan_intf.port_mask=> conflict!!!
            if(acl_filter->filter_fields & INGRESS_PORT_BIT)
            {
                for(i=0; i<RTK_RG_MAC_PORT_MAX; i++)
                {
                	if(RG_INVALID_MAC_PORT(i)) continue;
                    if( ( ((mac_pmsk.bits[0]>>i)&0x1)==0x1)  &&  (((ingress_intf_info.lan_intf.port_mask.portmask>>i)&0x1)==0x0) )
                        RETURN_ERR(RT_ERR_RG_ACL_CF_FIELD_CONFLICT);
                }
            }
        }
    }


	//not support null qos action to prevent multiple hit
	if((acl_filter->action_type==ACL_ACTION_TYPE_QOS) && (acl_filter->qos_actions == 0))
	{
		WARNING("ACL action_type ACL_ACTION_TYPE_QOS should assign one of qos action.");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	//SVLAN action not support feature
	if((acl_filter->action_type==ACL_ACTION_TYPE_QOS) && (acl_filter->qos_actions & ACL_ACTION_ACL_SVLANTAG_BIT))
	{
		if(acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_8100 ||acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C)
			RETURN_ERR(RT_ERR_RG_ACL_NOT_SUPPORT);

		if(acl_filter->action_acl_svlan.svlanSvidDecision==ACL_SVLAN_SVID_SP2C)
			RETURN_ERR(RT_ERR_RG_ACL_NOT_SUPPORT);

		if(acl_filter->action_acl_svlan.svlanSpriDecision==ACL_SVLAN_SPRI_COPY_FROM_DSCP_REMAP || acl_filter->action_acl_svlan.svlanSpriDecision==ACL_SVLAN_SPRI_COPY_FROM_SP2C)
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
	}


	if( (acl_filter->action_type==ACL_ACTION_TYPE_QOS) && (acl_filter->qos_actions & ACL_ACTION_SHARE_METER_BIT) ){
		if(rg_db.systemGlobal.aclPolicingMode==ACL_IGR_LOG_MIB_ACT){
			WARNING("ACL log action and policing action are mutual exclusive, please check /proc/rg/turn_on_acl_counter");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
		//only support share meter index 0~15(switch), 48~79(software)
		if(acl_filter->action_share_meter >= PURE_SW_METER_IDX_OFFSET+PURE_SW_SHAREMETER_TABLE_SIZE)
		{
			WARNING("ACL action share meter must less than %d", PURE_SW_METER_IDX_OFFSET+PURE_SW_SHAREMETER_TABLE_SIZE);
			RETURN_ERR(RT_ERR_RG_ACL_NOT_SUPPORT);
		}
		else if((acl_filter->action_share_meter >= MAX_SHAREMETER_TABLE_SIZE) && (acl_filter->action_share_meter < PURE_SW_METER_IDX_OFFSET))
		{
			WARNING("ACL action not support share meter index from %d to %d", MAX_SHAREMETER_TABLE_SIZE, PURE_SW_METER_IDX_OFFSET-1);
			RETURN_ERR(RT_ERR_RG_ACL_NOT_SUPPORT);
		}
	}

	if( (acl_filter->action_type==ACL_ACTION_TYPE_QOS) && (acl_filter->qos_actions & ACL_ACTION_LOG_COUNTER_BIT) ){
		if(rg_db.systemGlobal.aclPolicingMode==ACL_IGR_LOG_POLICING_ACT){
			WARNING("ACL policing action and log action are mutual exclusive, please check /proc/rg/turn_on_acl_counter");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}


#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
	//limit CF ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET to the ACL only pattern, policy_route can assign egress_intf only
	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET){
		if((acl_filter->filter_fields&EGRESS_IPV4_SIP_RANGE_BIT)||
			(acl_filter->filter_fields&EGRESS_IPV4_DIP_RANGE_BIT)||
			(acl_filter->filter_fields&EGRESS_L4_SPORT_RANGE_BIT)||
			(acl_filter->filter_fields&EGRESS_L4_DPORT_RANGE_BIT)||
			(acl_filter->filter_fields&EGRESS_CTAG_PRI_BIT)||
			(acl_filter->filter_fields&EGRESS_CTAG_VID_BIT))
		{
			WARNING("type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET support ingress pattern only!");
			RETURN_ERR(RT_ERR_RG_ACL_NOT_SUPPORT);
		}
		else if(acl_filter->filter_fields&EGRESS_INTF_BIT)
		{
			int intf_idx,ret;
			if((acl_filter->filter_fields&INGRESS_CTAG_VID_BIT)||
				(acl_filter->filter_fields&INGRESS_PORT_BIT)||
				(acl_filter->filter_fields&INGRESS_IPV4_DIP_RANGE_BIT)||
				(acl_filter->filter_fields&INGRESS_IPV6_DIP_BIT)||
				(acl_filter->filter_fields&INGRESS_DMAC_BIT))
			{
				WARNING("type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET for Egress Intf conflict ingress pattern!");
				RETURN_ERR(RT_ERR_RG_ACL_NOT_SUPPORT);
			}
			//20160303LUKE: check for intf index, should exist and WAN interface.
			intf_idx=acl_filter->egress_intf_idx;
			ret=rtk_rg_apollo_intfInfo_find(&egress_intf_info, &intf_idx);
			if(ret!=RT_ERR_RG_OK||intf_idx!=acl_filter->egress_intf_idx||!egress_intf_info.is_wan){
				WARNING("type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET for Egress Intf Index error!");
				RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
			}

			//20160722CHUCK: ApolloFE not support egress trap to PS! (SKB may be changed by L34)
			if(acl_filter->action_type==ACL_ACTION_TYPE_TRAP_TO_PS)
			{
				WARNING("type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET for Egress Intf not support TRAP_TO_PS!");
				RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
			}


		}
	}
#if 0
	//20160223LUKE: check if the egress rules are mix with non-egress rules!!
	bzero(rg_db.systemGlobal.acl_filter_temp, sizeof(rtk_rg_aclFilterAndQos_t)*MAX_ACL_SW_ENTRY_SIZE);
	bzero(&aclSWEntry,sizeof(aclSWEntry));

	aclSWEntry.acl_filter = *acl_filter;
	aclSWEntry.valid = RTK_RG_ENABLED;

	ASSERT_EQ(_rtk_rg_aclSWEntry_empty_find(&acl_SWEntry_index),RT_ERR_RG_OK);
   	ASSERT_EQ(_rtk_rg_aclSWEntry_set(acl_SWEntry_index,aclSWEntry),RT_ERR_RG_OK);

    //backup all acl_filter for sorting
    for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++){
        //backup acl_filter
        ASSERT_EQ(_rtk_rg_aclSWEntry_get(i, &aclSWEntry),RT_ERR_RG_OK);
        rg_db.systemGlobal.acl_filter_temp[i] = aclSWEntry.acl_filter;
    }

	ASSERT_EQ(_rtk_rg_rearrange_ACL_weight(&i),RT_ERR_RG_OK);
	i=_rtk_rg_rearrange_ACL_weight_for_egress_wan();

	//Clear the temporary SWEntry
	bzero(&aclSWEntry,sizeof(aclSWEntry));
	ASSERT_EQ(_rtk_rg_aclSWEntry_set(acl_SWEntry_index,aclSWEntry),RT_ERR_RG_OK);
	if(i==RT_ERR_RG_ACL_EGRESS_WAN_MIX){
		WARNING("Mix egress WAN pattern rule with non-egress-WAN pattern rule at same weight!");
		RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

#else
	//20161130: check is there any TRANSFORM/NONE_TRANSFORM rule using the same acl_weight (avoid call to _rtk_rg_rearrange_ACL_weight() that may change internal datastructure)
	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
	{
		for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++){
			if(rg_db.systemGlobal.acl_SW_table_entry[i].valid==1
				&& acl_filter->acl_weight==rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.acl_weight)//acl_weight is the same with compared rule
			{
				if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
					&& (acl_filter->filter_fields&EGRESS_INTF_BIT)!=(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_INTF_BIT)) //checked rule and compared rule is different type, one is TRANSFORM, the other is not.
				{
					WARNING("Mix egress WAN pattern rule with non-egress-WAN pattern rule at same weight!");
					RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
				}
			}
		}
	}
#endif


#else
	//limit CF ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET to the ACL only pattern, policy_route can assign egress_intf only
	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET){
		if((acl_filter->filter_fields&EGRESS_INTF_BIT)||
			(acl_filter->filter_fields&EGRESS_IPV4_SIP_RANGE_BIT)||
			(acl_filter->filter_fields&EGRESS_IPV4_DIP_RANGE_BIT)||
			(acl_filter->filter_fields&EGRESS_L4_SPORT_RANGE_BIT)||
			(acl_filter->filter_fields&EGRESS_L4_DPORT_RANGE_BIT)||
			(acl_filter->filter_fields&EGRESS_CTAG_PRI_BIT)||
			(acl_filter->filter_fields&EGRESS_CTAG_VID_BIT))
		{
			WARNING("type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET support ingress pattern only!");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}
#endif


	//20160722CHUCK: ApolloFE not support egress trap to PS! (SKB may be changed by L34)
	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP ||
		acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP)
	{
		if(acl_filter->action_type==ACL_ACTION_TYPE_TRAP_TO_PS)
		{
			WARNING("type ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP/ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP not support TRAP_TO_PS!");
			RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
		}
	}

	//CF template pattern is limited by direction
	if((acl_filter->filter_fields&EGRESS_CTAG_PRI_BIT) || (acl_filter->filter_fields&INTERNAL_PRI_BIT)) //egress_ctag_pri and internal-pri only support when upstream
	{
		if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP|| acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN)
		{
		}
		else
		{
			WARNING("EGRESS_CTAG_PRI_BIT only supported when upstream!");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}

#if defined(CONFIG_GPON_FEATURE)
	if((acl_filter->filter_fields&INGRESS_STREAM_ID_BIT) && (rg_db.systemGlobal.initParam.wanPortGponMode))
	{
		if((acl_filter->ingress_stream_id < 0) || (acl_filter->ingress_stream_id > 63)){
			WARNING("INGRESS_STREAM_ID_BIT only support range 0~63, not support %d", acl_filter->ingress_stream_id);
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}
#endif

	#if 0 //20151221, Cheney: remove this limitation because some customers may use MacBasedDecision + force forward.
	//not support force forward ports when macBasedDecision enabled.
	//Because downstream binding may have more than one packet to lan port, the force forward can not define the behavior.
	if(rg_db.systemGlobal.initParam.macBasedTagDecision==1)
	{
		if(acl_filter->qos_actions&ACL_ACTION_DS_UNIMASK_BIT)
		{
			WARNING("ACL_ACTION_DS_UNIMASK_BIT is not supported when macBasedTagDecision enabled");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}

		if(acl_filter->qos_actions&ACL_ACTION_REDIRECT_BIT)
		{
			WARNING("ACL_ACTION_REDIRECT_BIT is not supported when macBasedTagDecision enabled");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}
	#endif


	if(acl_filter->filter_fields_inverse != 0x0)
	{
		WARNING("apollo not suport ACL with inverse pattern!");
		RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
	}

	//20170310CHUCK: due to apolloPro could support egress pattern, avoid pattern not limited.  limit egress pattern when fwdtype=0
	if(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
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
			INGRESS_PKT_LEN_RANGE_BIT|
			EGRESS_INTF_BIT) )
		{
			WARNING("fwdtype==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET with not supported pattern.");
			RETURN_ERR(RT_ERR_RG_INVALID_PARAM);
		}
		else
		{
			//only aboved pattern can be support when fwdtype==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
		}

	}



	return (RT_ERR_RG_OK);
}



/*(3)literomeDriver mapping APIs*/
int32 _rtk_rg_apollo_aclFilterAndQos_add(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx)
{
	int i,ret;
	int aclSWEntryIdx;
	rtk_rg_aclFilterEntry_t aclSWEntry;
    bzero(&aclSWEntry,sizeof(aclSWEntry));


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

	if(rg_db.systemGlobal.aclSkipRearrangeHWAclCf==1)
	{
		ret = _rtk_rg_acl_multicastTempPermit_and_rest_trap_enable();
		if(ret!=RT_ERR_RG_OK) goto aclFailed;
	}
	else
	{
#if defined(HW_ACL_REARRANGE_PROTECT_VERSION) && (HW_ACL_REARRANGE_PROTECT_VERSION==2)
		_rtk_rg_acl_for_multicast_temp_protection_add();
#else
		//let multicast packet permit while H/W ACL rearrange to make sure IPTV flow smooth
		ret = _rtk_rg_acl_multicastTempPermit_enable();
		if(ret!=RT_ERR_RG_OK) goto aclFailed;
#endif
	}



	if(acl_filter->action_type==ACL_ACTION_TYPE_POLICY_ROUTE)
	{
		DEBUG("Policy Route to WAN%d! do not add to ASIC!!",acl_filter->action_policy_route_wan);

	}
	else if(acl_filter->filter_fields & INGRESS_WLANDEV_BIT)
	{
		rg_db.systemGlobal.wlanDevPatternValidInACL = 1;//it will skip HWLOOKUP
		DEBUG("wlanDev pattern vlaid! do not rearrange ASIC!!");
	}
	else
	{
		if(rg_db.systemGlobal.aclSkipRearrangeHWAclCf==1)
		{
			//skip rearrange H/W
		}else{
			ret=_rtk_rg_aclSWEntry_to_asic_add(acl_filter,&aclSWEntry,FAIL);
			if(ret!=RT_ERR_RG_OK) goto aclFailed;
		}
	}

	aclSWEntry.acl_filter = *acl_filter;
	aclSWEntry.valid=RTK_RG_ENABLED;

    ret=_rtk_rg_aclSWEntry_empty_find(&aclSWEntryIdx);
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

   	ret=_rtk_rg_aclSWEntry_set(aclSWEntryIdx,aclSWEntry);
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	*acl_filter_idx = aclSWEntryIdx;
	DEBUG("add aclSWEntry[%d]",*acl_filter_idx);

	//count the total aclSWEntry size
	rg_db.systemGlobal.acl_SW_table_entry_size=0;
	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++)
	{
		if(rg_db.systemGlobal.acl_SW_table_entry[i].valid==RTK_RG_ENABLED)
			rg_db.systemGlobal.acl_SW_table_entry_size++;
	}

	//rearrange all HW ACL/CF and aclSWEntry.
	if(acl_filter->action_type==ACL_ACTION_TYPE_POLICY_ROUTE)
	{
		DEBUG("Policy Route to WAN%d! do not add to ASIC!!",acl_filter->action_policy_route_wan);
	}
	else if(acl_filter->filter_fields & INGRESS_WLANDEV_BIT)
	{
		rg_db.systemGlobal.wlanDevPatternValidInACL = 1;//it will skip HWLOOKUP
		DEBUG("wlanDev pattern vlaid! do not rearrange ASIC!!");
	}

	//readd all acl rules for sorting: action  INGRESS_ALL(sort by weight) > L34 DROP/TRAP/PERMIT > L34 QoS
    ret = _rtk_rg_aclSWEntry_and_asic_rearrange();
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	ret = _rtk_rg_shortCut_clear();
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	if(rg_db.systemGlobal.aclSkipRearrangeHWAclCf==1)
	{
		//rserved the MC permit and Trap all until proc rg_db.systemGlobal.aclSkipRearrangeHWAclCf disabled
	}
	else
	{
#if defined(HW_ACL_REARRANGE_PROTECT_VERSION) && (HW_ACL_REARRANGE_PROTECT_VERSION==2)
		_rtk_rg_acl_for_multicast_temp_protection_del();
#else
		//delete the MC temp permit
		_rtk_rg_acl_multicastTempPermit_disable();
#endif
	}

	if(rg_db.systemGlobal.acl_rg_add_parameter_dump){
		rtlglue_printf("add to RG ACL[%d] success!\n",*acl_filter_idx);
	}
	return (RT_ERR_RG_OK);

aclFailed:
	if(rg_db.systemGlobal.aclSkipRearrangeHWAclCf==1)
	{
		//rserved the MC permit and Trap all until proc rg_db.systemGlobal.aclSkipRearrangeHWAclCf disabled
	}
	else
	{
#if defined(HW_ACL_REARRANGE_PROTECT_VERSION) && (HW_ACL_REARRANGE_PROTECT_VERSION==2)
		_rtk_rg_acl_for_multicast_temp_protection_del();
#else
		//delete the MC temp permit
		_rtk_rg_acl_multicastTempPermit_disable();
#endif
	}

	if(rg_db.systemGlobal.acl_rg_add_parameter_dump){
		rtlglue_printf("add to RG ACL Faild! ret=0x%x\n",ret);
	}

	return ret;

}

int32 _rtk_rg_apollo_aclFilterAndQos_del(int acl_filter_idx)
{
	int i,ret=RT_ERR_RG_FAILED;
	rtk_rg_aclFilterEntry_t aclSWEntryEmpty;
	bzero(&aclSWEntryEmpty,sizeof(aclSWEntryEmpty));

	//Check rg has been init
	if(rg_db.systemGlobal.vlanInit==0)
		RETURN_ERR(RT_ERR_RG_NOT_INIT);
	if(acl_filter_idx<0 || acl_filter_idx>=MAX_ACL_SW_ENTRY_SIZE){
		WARNING("invalid ACL index(%d)!",acl_filter_idx);
		RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);
	}

	if(rg_db.systemGlobal.aclSkipRearrangeHWAclCf==1)
	{
		ret = _rtk_rg_acl_multicastTempPermit_and_rest_trap_enable();
		if(ret!=RT_ERR_RG_OK) goto aclFailed;
	}
	else
	{
#if defined(HW_ACL_REARRANGE_PROTECT_VERSION) && (HW_ACL_REARRANGE_PROTECT_VERSION==2)
		_rtk_rg_acl_for_multicast_temp_protection_add();
#else
		//let multicast packet permit while H/W ACL rearrange to make sure IPTV flow smooth
		ret = _rtk_rg_acl_multicastTempPermit_enable();
		if(ret!=RT_ERR_RG_OK) goto aclFailed;
#endif
	}


	//clean aclSWEntry
	ret = _rtk_rg_aclSWEntry_set(acl_filter_idx, aclSWEntryEmpty);
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	//rearrange the ACL & CF ASIC to avoid discontinuous entry
	ret = _rtk_rg_aclSWEntry_and_asic_rearrange();
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	//count the total aclSWEntry size
	rg_db.systemGlobal.acl_SW_table_entry_size=0;
	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++)
	{
		if(rg_db.systemGlobal.acl_SW_table_entry[i].valid==RTK_RG_ENABLED)
			rg_db.systemGlobal.acl_SW_table_entry_size++;
	}

	DEBUG("delete aclSWEntry[%d]",acl_filter_idx);

	if(acl_filter_idx < rg_db.systemGlobal.acl_SW_empty_start_index)	//update empty start idx when delete rule with smaller rule index
		rg_db.systemGlobal.acl_SW_empty_start_index = acl_filter_idx;

	ret = _rtk_rg_shortCut_clear();
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	//_rtk_rg_aclSWEntry_dump();

	if(rg_db.systemGlobal.aclSkipRearrangeHWAclCf==1)
	{
		//rserved the MC permit and Trap all until proc rg_db.systemGlobal.aclSkipRearrangeHWAclCf disabled
	}
	else
	{
#if defined(HW_ACL_REARRANGE_PROTECT_VERSION) && (HW_ACL_REARRANGE_PROTECT_VERSION==2)
		_rtk_rg_acl_for_multicast_temp_protection_del();
#else
		//delete the MC temp permit
		_rtk_rg_acl_multicastTempPermit_disable();
#endif
	}

	return (RT_ERR_RG_OK);

aclFailed:
	if(rg_db.systemGlobal.aclSkipRearrangeHWAclCf==1)
	{
		//rserved the MC permit and Trap all until proc rg_db.systemGlobal.aclSkipRearrangeHWAclCf disabled
	}
	else
	{
#if defined(HW_ACL_REARRANGE_PROTECT_VERSION) && (HW_ACL_REARRANGE_PROTECT_VERSION==2)
		_rtk_rg_acl_for_multicast_temp_protection_del();
#else
		//delete the MC temp permit
		_rtk_rg_acl_multicastTempPermit_disable();
#endif
	}

	return ret;

}


int32 _rtk_rg_apollo_aclFilterAndQos_find(rtk_rg_aclFilterAndQos_t *acl_filter, int *valid_idx)
{
	//search for the first not empty entry after valid_idx.
	int i;
	bzero(&rg_db.systemGlobal.aclSWEntry, sizeof(rg_db.systemGlobal.aclSWEntry));
	bzero(&rg_db.systemGlobal.aclSWEntryEmpty, sizeof(rg_db.systemGlobal.aclSWEntryEmpty));

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
				DEBUG("rtk_rg_aclFilterAndQos_find index=%d",*valid_idx);
				return (RT_ERR_RG_OK);
			}

		}
	}else if((*valid_idx>=0) && (*valid_idx<MAX_ACL_SW_ENTRY_SIZE)){
		for(i=*valid_idx; i<MAX_ACL_SW_ENTRY_SIZE; i++)
		{
			ASSERT_EQ(_rtk_rg_aclSWEntry_get(i,&rg_db.systemGlobal.aclSWEntry),RT_ERR_RG_OK);

			if(memcmp(&rg_db.systemGlobal.aclSWEntry,&rg_db.systemGlobal.aclSWEntryEmpty,sizeof(rtk_rg_aclFilterEntry_t)))//search the different with aclSWEntryEmpty
			{
				*valid_idx=i;
				*acl_filter = rg_db.systemGlobal.aclSWEntry.acl_filter;
				DEBUG("rtk_rg_aclFilterAndQos_find index=%d",*valid_idx);
				return (RT_ERR_RG_OK);
			}
		}
		DEBUG("rtk_rg_aclFilterAndQos_find failed");
	}else{
		RETURN_ERR(RT_ERR_RG_INDEX_OUT_OF_RANGE);
	}
	//not found
	return (RT_ERR_RG_ACL_SW_ENTRY_NOT_FOUND);
}






int _rtk_rg_classifySWEntry_not_support(rtk_rg_classifyEntry_t *classifyFilter)
{

	if(classifyFilter->filter_fields & EGRESS_GEMIDX_BIT)
	{
		if(classifyFilter->direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM)
		{
			WARNING("CF pattern EGRESS_GEMIDX_BIT not support upstream.");
			RETURN_ERR(RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
		else if(classifyFilter->gemidx_mask == SW_CF_GEMIDX_MASK)
		{	//this configuration will cause RTK CF range check fail, so it will sw cf only
			WARNING("CF pattern EGRESS_GEMIDX_BIT mask is 0x%x for SW CF only", classifyFilter->gemidx_mask);
		}
		else if((classifyFilter->gemidx > MAX_CF_GEMIDX_VALUE) || (classifyFilter->gemidx_mask > MAX_CF_GEMIDX_MASK))
		{
			WARNING("CF pattern EGRESS_GEMIDX_BIT max value is %d/mask is 0x%x.", MAX_CF_GEMIDX_VALUE, MAX_CF_GEMIDX_MASK);
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

	//20160701LUKE: disable CF modify STAG if layer2LookupMissFlood2CPU is enabled.
	if(rg_kernel.layer2LookupMissFlood2CPU==RTK_RG_ENABLED && classifyFilter->direction==RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM &&
		(classifyFilter->action_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID||
		classifyFilter->action_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_8100||
		classifyFilter->action_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C||
		classifyFilter->action_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2||
		classifyFilter->action_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_ORIGINAL_STAG_TPID)&&
		(classifyFilter->action_svlan.svlanSvidDecision==ACL_SVLAN_SVID_ASSIGN||
		classifyFilter->action_svlan.svlanSvidDecision==ACL_SVLAN_SVID_COPY_FROM_1ST_TAG||
		classifyFilter->action_svlan.svlanSvidDecision==ACL_SVLAN_SVID_COPY_FROM_2ND_TAG)){
		WARNING("svlanSvidDecision not support when layer2LookupMissFlood2CPU in apolloFE");
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


//rtk_classify_field_t classifyField[CF_PATTERN_END];
int32 _rtk_rg_apollo_classifyEntry_add(rtk_rg_classifyEntry_t *classifyFilter)
{

	int i;
	int ret;
	rtk_classify_cfg_t cfRule;
	rtk_classify_field_t empty_classifyField;

	//Check rg has been init
	if(rg_db.systemGlobal.vlanInit==0)
		RETURN_ERR(RT_ERR_RG_NOT_INIT);


	bzero(&cfRule,sizeof(rtk_classify_cfg_t));
	bzero(&empty_classifyField,sizeof(rtk_classify_field_t));
	bzero(rg_db.systemGlobal.classifyField,sizeof(rtk_classify_field_t)*CF_PATTERN_END);


	//check index range
	if(classifyFilter->index < rg_db.systemGlobal.cf_pattern0_size || classifyFilter->index >= TOTAL_CF_ENTRY_SIZE){
		rtlglue_printf("Invalid classify rule index, range should be in %d~%d\n",rg_db.systemGlobal.cf_pattern0_size,TOTAL_CF_ENTRY_SIZE);
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


	//init each H/W action control bit
	if(classifyFilter->direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM)
	{
		//init classifyEntry without occupied action control bit.
		cfRule.act.usAct.csAct = CLASSIFY_US_CSACT_ACTCTRL_DISABLE;
		cfRule.act.usAct.cAct = CLASSIFY_US_CACT_ACTCTRL_DISABLE;
		cfRule.act.usAct.sidQidAct = CLASSIFY_US_SQID_ACT_ACTCTRL_DISABLE;
		cfRule.act.usAct.interPriAct = CLASSIFY_CF_PRI_ACT_ACTCTRL_DISABLE;
		cfRule.act.usAct.dscp = CLASSIFY_DSCP_ACT_ACTCTRL_DISABLE;
		cfRule.act.usAct.drop = CLASSIFY_DROP_ACT_ACTCTRL_DISABLE;
	}
	else if(classifyFilter->direction==RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM)
	{
		//init classifyEntry without occupied action control bit.
		cfRule.act.dsAct.csAct = CLASSIFY_DS_CSACT_ACTCTRL_DISABLE;
		cfRule.act.dsAct.cAct = CLASSIFY_DS_CACT_ACTCTRL_DISABLE;
		cfRule.act.dsAct.interPriAct = CLASSIFY_CF_PRI_ACT_ACTCTRL_DISABLE;
		cfRule.act.dsAct.uniAct = CLASSIFY_DS_UNI_ACT_ACTCTRL_DISABLE;
		cfRule.act.dsAct.dscp = CLASSIFY_DSCP_ACT_ACTCTRL_DISABLE;
	}


	//set valid,index,template
	cfRule.valid = ENABLED;
	cfRule.index = classifyFilter->index;
	cfRule.templateIdx = 0; //[FIXME] For now, only support template[0]. templtae[1] should be enhanced.

	//set direction
	if(classifyFilter->direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM){
		cfRule.direction = CLASSIFY_DIRECTION_US;
	}else{
		cfRule.direction = CLASSIFY_DIRECTION_DS;
	}

	//set patterns
	if(classifyFilter->filter_fields & EGRESS_ETHERTYPR_BIT){
		rg_db.systemGlobal.classifyField[CF_PATTERN_ETHERTYPE].fieldType=CLASSIFY_FIELD_ETHERTYPE;
		rg_db.systemGlobal.classifyField[CF_PATTERN_ETHERTYPE].classify_pattern.fieldData.value=(classifyFilter->etherType & 0xffff);
		if(classifyFilter->etherType_mask==0x0){
			//for compitible of non-mask version (mask not set means all care)
			rg_db.systemGlobal.classifyField[CF_PATTERN_ETHERTYPE].classify_pattern.fieldData.mask=0xffff;
			classifyFilter->etherType_mask=0xffff;
		}else{
			rg_db.systemGlobal.classifyField[CF_PATTERN_ETHERTYPE].classify_pattern.fieldData.mask=(classifyFilter->etherType_mask & 0xffff);
		}
	}
	if(classifyFilter->filter_fields & EGRESS_GEMIDX_BIT){
		rg_db.systemGlobal.classifyField[CF_PATTERN_GEMIDX_OR_LLID].fieldType=CLASSIFY_FIELD_TOS_DSIDX;
		rg_db.systemGlobal.classifyField[CF_PATTERN_GEMIDX_OR_LLID].classify_pattern.fieldData.value=(classifyFilter->gemidx & MAX_CF_GEMIDX_MASK);
		if(classifyFilter->gemidx_mask==0x0){
			//for compitible of non-mask version (mask not set means all care)
			rg_db.systemGlobal.classifyField[CF_PATTERN_GEMIDX_OR_LLID].classify_pattern.fieldData.mask=MAX_CF_GEMIDX_MASK;
			classifyFilter->gemidx_mask=MAX_CF_GEMIDX_MASK;
		}else if(classifyFilter->gemidx_mask==SW_CF_GEMIDX_MASK){
			rg_db.systemGlobal.classifyField[CF_PATTERN_GEMIDX_OR_LLID].classify_pattern.fieldData.mask=(classifyFilter->gemidx_mask);
		}else{
			rg_db.systemGlobal.classifyField[CF_PATTERN_GEMIDX_OR_LLID].classify_pattern.fieldData.mask=(classifyFilter->gemidx_mask&MAX_CF_GEMIDX_MASK);
		}
	}

	if(classifyFilter->filter_fields & EGRESS_LLID_BIT){
		rg_db.systemGlobal.classifyField[CF_PATTERN_GEMIDX_OR_LLID].fieldType=CLASSIFY_FIELD_TOS_DSIDX;
		rg_db.systemGlobal.classifyField[CF_PATTERN_GEMIDX_OR_LLID].classify_pattern.fieldData.value=(classifyFilter->llid & MAX_CF_LLID_VALUE); //llid 4 bits only
		rg_db.systemGlobal.classifyField[CF_PATTERN_GEMIDX_OR_LLID].classify_pattern.fieldData.mask=MAX_CF_LLID_VALUE;
	}
	if(classifyFilter->filter_fields & EGRESS_TAGVID_BIT){
		rg_db.systemGlobal.classifyField[CF_PATTERN_OUTTERTAGVID].fieldType=CLASSIFY_FIELD_TAG_VID;
		rg_db.systemGlobal.classifyField[CF_PATTERN_OUTTERTAGVID].classify_pattern.fieldData.value=(classifyFilter->outterTagVid & 0xfff);
		rg_db.systemGlobal.classifyField[CF_PATTERN_OUTTERTAGVID].classify_pattern.fieldData.mask=0xfff;
	}
	if(classifyFilter->filter_fields & EGRESS_TAGPRI_BIT){
		rg_db.systemGlobal.classifyField[CF_PATTERN_OUTTERTAGPRI].fieldType=CLASSIFY_FIELD_TAG_PRI;
		rg_db.systemGlobal.classifyField[CF_PATTERN_OUTTERTAGPRI].classify_pattern.fieldData.value=(classifyFilter->outterTagPri & 0x7);
		rg_db.systemGlobal.classifyField[CF_PATTERN_OUTTERTAGPRI].classify_pattern.fieldData.mask=0x7;
	}
	if(classifyFilter->filter_fields & EGRESS_INTERNALPRI_BIT){
		rg_db.systemGlobal.classifyField[CF_PATTERN_INTERNALPRI].fieldType=CLASSIFY_FIELD_INTER_PRI;
		rg_db.systemGlobal.classifyField[CF_PATTERN_INTERNALPRI].classify_pattern.fieldData.value=(classifyFilter->internalPri & 0x7);
		rg_db.systemGlobal.classifyField[CF_PATTERN_INTERNALPRI].classify_pattern.fieldData.mask=0x7;
	}
	if(classifyFilter->filter_fields & EGRESS_STAGIF_BIT){
		rg_db.systemGlobal.classifyField[CF_PATTERN_STAGFLAG].fieldType=CLASSIFY_FIELD_IS_STAG;
		rg_db.systemGlobal.classifyField[CF_PATTERN_STAGFLAG].classify_pattern.fieldData.value=(classifyFilter->stagIf & 0x1);
		rg_db.systemGlobal.classifyField[CF_PATTERN_STAGFLAG].classify_pattern.fieldData.mask=0x1;
	}
	if(classifyFilter->filter_fields & EGRESS_CTAGIF_BIT){
		rg_db.systemGlobal.classifyField[CF_PATTERN_CTAGFLAG].fieldType=CLASSIFY_FIELD_IS_CTAG;
		rg_db.systemGlobal.classifyField[CF_PATTERN_CTAGFLAG].classify_pattern.fieldData.value=(classifyFilter->ctagIf & 0x1);
		rg_db.systemGlobal.classifyField[CF_PATTERN_CTAGFLAG].classify_pattern.fieldData.mask=0x1;
	}
	if(classifyFilter->filter_fields & EGRESS_UNI_BIT){
		rg_db.systemGlobal.classifyField[CF_PATTERN_UNI].fieldType=CLASSIFY_FIELD_UNI;
		rg_db.systemGlobal.classifyField[CF_PATTERN_UNI].classify_pattern.fieldData.value=(classifyFilter->uni & CF_UNI_DEFAULT);
		if(classifyFilter->uni_mask==0x0){
			//for compitible of non-mask version (mask not set means all care)
			rg_db.systemGlobal.classifyField[CF_PATTERN_UNI].classify_pattern.fieldData.mask=CF_UNI_DEFAULT;
			classifyFilter->uni_mask = CF_UNI_DEFAULT;
		}else{
			rg_db.systemGlobal.classifyField[CF_PATTERN_UNI].classify_pattern.fieldData.mask=(classifyFilter->uni_mask&CF_UNI_DEFAULT);
		}

	}

	for(i=0;i<CF_PATTERN_END;i++){
		if(memcmp(&rg_db.systemGlobal.classifyField[i],&empty_classifyField,sizeof(rtk_classify_field_t))){
			ret = rtk_classify_field_add(&cfRule,&rg_db.systemGlobal.classifyField[i]);
			if(ret!=RT_ERR_OK){
				DEBUG("add classify_field[%d] failed!",i);
				RETURN_ERR(ret);
			}
		}
	}

	//set action
	if(classifyFilter->direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM){
		//Upstream actions
		for(i=0;i<64;i++){
			switch((classifyFilter->us_action_field&(1ULL<<i))){
				//Stag
				case CF_US_ACTION_STAG_BIT:
				{
					//StagIf decision
					switch(classifyFilter->action_svlan.svlanTagIfDecision)
					{
						case ACL_SVLAN_TAGIF_NOP: cfRule.act.usAct.csAct = CLASSIFY_US_CSACT_NOP; break;
						case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID: cfRule.act.usAct.csAct = CLASSIFY_US_CSACT_ADD_TAG_VS_TPID; break;
						case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2: cfRule.act.usAct.csAct = CLASSIFY_US_CSACT_ADD_TAG_VS_TPID2; break;
						case ACL_SVLAN_TAGIF_UNTAG: cfRule.act.usAct.csAct = CLASSIFY_US_CSACT_DEL_STAG; break;
						case ACL_SVLAN_TAGIF_TRANSPARENT: cfRule.act.usAct.csAct = CLASSIFY_US_CSACT_TRANSPARENT; break;
						case ACL_SVLAN_TAGIF_TAGGING_WITH_ORIGINAL_STAG_TPID: cfRule.act.usAct.csAct = CLASSIFY_US_CSACT_ADD_TAG_STAG_TPID; break;
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
					//Svid decision
					switch(classifyFilter->action_svlan.svlanSvidDecision)
					{
						case ACL_SVLAN_SVID_NOP: cfRule.act.usAct.csVidAct = CLASSIFY_US_VID_ACT_NOP; break;
						case ACL_SVLAN_SVID_ASSIGN: cfRule.act.usAct.csVidAct = CLASSIFY_US_VID_ACT_ASSIGN; break;
						case ACL_SVLAN_SVID_COPY_FROM_1ST_TAG: cfRule.act.usAct.csVidAct = CLASSIFY_US_VID_ACT_FROM_1ST_TAG; break;
						case ACL_SVLAN_SVID_COPY_FROM_2ND_TAG: cfRule.act.usAct.csVidAct = CLASSIFY_US_VID_ACT_FROM_2ND_TAG; break;
						case ACL_SVLAN_SVID_SP2C: WARNING("APOLLOFE_ACL_SVLAN_SVID_SP2C only supported by downstream"); RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT); break; //downstream only
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
					cfRule.act.usAct.sTagVid = classifyFilter->action_svlan.assignedSvid;

					//Spri decision
					switch(classifyFilter->action_svlan.svlanSpriDecision)
					{
						case ACL_SVLAN_SPRI_NOP: cfRule.act.usAct.csPriAct = CLASSIFY_US_PRI_ACT_NOP; break;
						case ACL_SVLAN_SPRI_ASSIGN: cfRule.act.usAct.csPriAct = CLASSIFY_US_PRI_ACT_ASSIGN; break;
						case ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG: cfRule.act.usAct.csPriAct = CLASSIFY_US_PRI_ACT_FROM_1ST_TAG; break;
						case ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG: cfRule.act.usAct.csPriAct = CLASSIFY_US_PRI_ACT_FROM_2ND_TAG; break;
						case ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI: cfRule.act.usAct.csPriAct = CLASSIFY_US_PRI_ACT_FROM_INTERNAL; break;
						case ACL_SVLAN_SPRI_COPY_FROM_DSCP_REMAP: WARNING("APOLLOFE_ACL_SVLAN_SPRI_COPY_FROM_DSCP_REMAP only supported by downstream"); RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT); break; //downstream only
						case ACL_SVLAN_SPRI_COPY_FROM_SP2C: WARNING("APOLLOFE_ACL_SVLAN_SPRI_COPY_FROM_SP2C only supported by downstream"); RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT); break; //downstream only
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
					cfRule.act.usAct.sTagPri = classifyFilter->action_svlan.assignedSpri;
				}
				break;

				//Ctag
				case CF_US_ACTION_CTAG_BIT:
				{
					//CtagIf decision
					switch(classifyFilter->action_cvlan.cvlanTagIfDecision){
						case ACL_CVLAN_TAGIF_NOP: cfRule.act.usAct.cAct = CLASSIFY_US_CACT_NOP; break;
						case ACL_CVLAN_TAGIF_TAGGING: cfRule.act.usAct.cAct = CLASSIFY_US_CACT_ADD_CTAG_8100; break;
						case ACL_CVLAN_TAGIF_UNTAG: cfRule.act.usAct.cAct = CLASSIFY_US_CACT_DEL_CTAG; break;
						case ACL_CVLAN_TAGIF_TRANSPARENT: cfRule.act.usAct.cAct = CLASSIFY_US_CACT_TRANSPARENT; break;
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
					//Cvid decision
					switch(classifyFilter->action_cvlan.cvlanCvidDecision){
						case ACL_CVLAN_CVID_NOP: cfRule.act.usAct.cVidAct = CLASSIFY_US_VID_ACT_NOP; break;
						case ACL_CVLAN_CVID_ASSIGN: cfRule.act.usAct.cVidAct = CLASSIFY_US_VID_ACT_ASSIGN; break;
						case ACL_CVLAN_CVID_COPY_FROM_1ST_TAG: cfRule.act.usAct.cVidAct = CLASSIFY_US_VID_ACT_FROM_1ST_TAG; break;
						case ACL_CVLAN_CVID_COPY_FROM_2ND_TAG: cfRule.act.usAct.cVidAct = CLASSIFY_US_VID_ACT_FROM_2ND_TAG; break;
						case ACL_CVLAN_CVID_COPY_FROM_INTERNAL_VID: cfRule.act.usAct.cVidAct = CLASSIFY_US_VID_ACT_FROM_INTERNAL; break;//upstream only
						case ACL_CVLAN_CVID_CPOY_FROM_SP2C: WARNING("APOLLOFE_ACL_CVLAN_CVID_CPOY_FROM_SP2C only supported by downstream"); RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT); break;//downstream only
						case ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID: WARNING("APOLLOFE_ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID only supported by downstream"); RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT); break; //downstream only
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
					cfRule.act.usAct.cTagVid = classifyFilter->action_cvlan.assignedCvid;

					//Cpri decision
					switch(classifyFilter->action_cvlan.cvlanCpriDecision){
						case ACL_CVLAN_CPRI_NOP: cfRule.act.usAct.cPriAct = CLASSIFY_US_PRI_ACT_NOP; break;
						case ACL_CVLAN_CPRI_ASSIGN: cfRule.act.usAct.cPriAct = CLASSIFY_US_PRI_ACT_ASSIGN; break;
						case ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG: cfRule.act.usAct.cPriAct = CLASSIFY_US_PRI_ACT_FROM_1ST_TAG; break;
						case ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG: cfRule.act.usAct.cPriAct = CLASSIFY_US_PRI_ACT_FROM_2ND_TAG; break;
						case ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI: cfRule.act.usAct.cPriAct = CLASSIFY_US_PRI_ACT_FROM_INTERNAL; break;
						case ACL_CVLAN_CPRI_COPY_FROM_DSCP_REMAP: cfRule.act.usAct.cPriAct = CLASSIFY_US_PRI_ACT_FROM_DSCP; break;
						case ACL_CVLAN_CPRI_COPY_FROM_SP2C: WARNING("APOLLOFE_ACL_CVLAN_CPRI_COPY_FROM_SP2C only supported by downstream"); RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT); break; //downstream only
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
					cfRule.act.usAct.cTagPri = classifyFilter->action_cvlan.assignedCpri;
				}
				break;


				//CFPRI
				case CF_US_ACTION_CFPRI_BIT:
				{
					switch(classifyFilter->action_cfpri.cfPriDecision){
						case ACL_CFPRI_ASSIGN: cfRule.act.usAct.interPriAct = CLASSIFY_CF_PRI_ACT_ASSIGN; break;
						case ACL_CFPRI_NOP: cfRule.act.usAct.interPriAct = CLASSIFY_CF_PRI_ACT_NOP; break;
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
					cfRule.act.usAct.cfPri = classifyFilter->action_cfpri.assignedCfPri;
				}
				break;

				//DSCP
				case CF_US_ACTION_DSCP_BIT:
				{
					switch(classifyFilter->action_dscp.dscpDecision){
						case ACL_DSCP_ASSIGN: cfRule.act.usAct.dscp = CLASSIFY_DSCP_ACT_ENABLE; break;
						case ACL_DSCP_NOP: cfRule.act.usAct.dscp = CLASSIFY_DSCP_ACT_DISABLE; break;
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
					cfRule.act.usAct.dscpVal = classifyFilter->action_dscp.assignedDscp;
				}
				break;

				//SID
				case CF_US_ACTION_SID_BIT:
				{
					switch(classifyFilter->action_sid_or_llid.sidDecision){
						case ACL_SID_LLID_ASSIGN: cfRule.act.usAct.sidQidAct = CLASSIFY_US_SQID_ACT_ASSIGN_SID; break;
						case ACL_SID_LLID_NOP: cfRule.act.usAct.sidQidAct = CLASSIFY_US_SQID_ACT_ASSIGN_NOP; break;
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
					cfRule.act.usAct.sidQid = classifyFilter->action_sid_or_llid.assignedSid_or_llid;
				}
				break;

				//FWD
				case CF_US_ACTION_FWD_BIT:
				{
					switch(classifyFilter->action_fwd.fwdDecision){
						case ACL_FWD_NOP: cfRule.act.usAct.drop = CLASSIFY_DROP_ACT_NONE; break;
						case ACL_FWD_DROP: cfRule.act.usAct.drop = CLASSIFY_DROP_ACT_ENABLE; break;
						case ACL_FWD_TRAP_TO_CPU: cfRule.act.usAct.drop = CLASSIFY_DROP_ACT_TRAP; break;
						case ACL_FWD_DROP_TO_PON: cfRule.act.usAct.drop = CLASSIFY_DROP_ACT_DROP_PON; break;
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
				}
				break;

				default:
					break;
			}
		}
	}else{
		//Downstream actions
		for(i=0;i<64;i++){
			switch((classifyFilter->ds_action_field&(1ULL<<i))){
				//Stag
				case CF_DS_ACTION_STAG_BIT:
				{
					//StagIf decision
					switch(classifyFilter->action_svlan.svlanTagIfDecision){
						case ACL_SVLAN_TAGIF_NOP: cfRule.act.dsAct.csAct = CLASSIFY_DS_CSACT_NOP; break;
						case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID: cfRule.act.dsAct.csAct = CLASSIFY_DS_CSACT_ADD_TAG_VS_TPID; break;
						case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2: cfRule.act.dsAct.csAct = CLASSIFY_DS_CSACT_ADD_TAG_VS_TPID2; break;
						case ACL_SVLAN_TAGIF_UNTAG: cfRule.act.dsAct.csAct = CLASSIFY_DS_CSACT_DEL_STAG; break;
						case ACL_SVLAN_TAGIF_TRANSPARENT: cfRule.act.dsAct.csAct = CLASSIFY_DS_CSACT_TRANSPARENT; break;
						case ACL_SVLAN_TAGIF_TAGGING_WITH_ORIGINAL_STAG_TPID: cfRule.act.dsAct.csAct = CLASSIFY_DS_CSACT_ADD_TAG_STAG_TPID; break;
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}

					//Svid decision
					switch(classifyFilter->action_svlan.svlanSvidDecision){
						case ACL_SVLAN_SVID_NOP: cfRule.act.dsAct.csVidAct = CLASSIFY_DS_VID_ACT_NOP; break;
						case ACL_SVLAN_SVID_ASSIGN: cfRule.act.dsAct.csVidAct = CLASSIFY_DS_VID_ACT_ASSIGN; break;
						case ACL_SVLAN_SVID_COPY_FROM_1ST_TAG: cfRule.act.dsAct.csVidAct = CLASSIFY_DS_VID_ACT_FROM_1ST_TAG; break;
						case ACL_SVLAN_SVID_COPY_FROM_2ND_TAG: cfRule.act.dsAct.csVidAct = CLASSIFY_DS_VID_ACT_FROM_2ND_TAG; break;
						case ACL_SVLAN_SVID_SP2C: cfRule.act.dsAct.csVidAct = CLASSIFY_DS_VID_ACT_TRANSLATION_SP2C; break;//downstream only
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;

					}
					cfRule.act.dsAct.sTagVid= classifyFilter->action_svlan.assignedSvid;

					//Spri decision
					switch(classifyFilter->action_svlan.svlanSpriDecision){
						case ACL_SVLAN_SPRI_NOP: cfRule.act.dsAct.csPriAct = CLASSIFY_DS_PRI_ACT_NOP; break;
						case ACL_SVLAN_SPRI_ASSIGN: cfRule.act.dsAct.csPriAct = CLASSIFY_DS_PRI_ACT_ASSIGN; break;
						case ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG: cfRule.act.dsAct.csPriAct = CLASSIFY_DS_PRI_ACT_FROM_1ST_TAG; break;
						case ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG: cfRule.act.dsAct.csPriAct = CLASSIFY_DS_PRI_ACT_FROM_2ND_TAG; break;
						case ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI: cfRule.act.dsAct.csPriAct = CLASSIFY_DS_PRI_ACT_FROM_INTERNAL; break;
						case ACL_SVLAN_SPRI_COPY_FROM_DSCP_REMAP:  cfRule.act.dsAct.csPriAct = CLASSIFY_DS_PRI_ACT_FROM_DSCP; break; //downstream only
						case ACL_SVLAN_SPRI_COPY_FROM_SP2C: cfRule.act.dsAct.csPriAct = CLASSIFY_DS_PRI_ACT_TRANSLATION_SP2C; break;  //downstream only
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
					cfRule.act.dsAct.sTagPri= classifyFilter->action_svlan.assignedSpri;
				}
				break;

				//Ctag
				case CF_DS_ACTION_CTAG_BIT:
				{
					//CtagIf decision
					switch(classifyFilter->action_cvlan.cvlanTagIfDecision){
						case ACL_CVLAN_TAGIF_NOP: cfRule.act.dsAct.cAct = CLASSIFY_DS_CACT_NOP; break;
						case ACL_CVLAN_TAGIF_TAGGING: cfRule.act.dsAct.cAct = CLASSIFY_DS_CACT_ADD_CTAG_8100; break;
						case ACL_CVLAN_TAGIF_UNTAG: cfRule.act.dsAct.cAct = CLASSIFY_DS_CACT_DEL_CTAG; break;
						case ACL_CVLAN_TAGIF_TRANSPARENT: cfRule.act.dsAct.cAct = CLASSIFY_DS_CACT_TRANSPARENT; break;
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
					//Cvid decision
					switch(classifyFilter->action_cvlan.cvlanCvidDecision){
						case ACL_CVLAN_CVID_NOP: cfRule.act.dsAct.cVidAct = CLASSIFY_DS_VID_ACT_NOP; break;
						case ACL_CVLAN_CVID_ASSIGN: cfRule.act.dsAct.cVidAct = CLASSIFY_DS_VID_ACT_ASSIGN; break;
						case ACL_CVLAN_CVID_COPY_FROM_1ST_TAG: cfRule.act.dsAct.cVidAct = CLASSIFY_DS_VID_ACT_FROM_1ST_TAG; break;
						case ACL_CVLAN_CVID_COPY_FROM_2ND_TAG: cfRule.act.dsAct.cVidAct = CLASSIFY_DS_VID_ACT_FROM_2ND_TAG; break;
						case ACL_CVLAN_CVID_COPY_FROM_INTERNAL_VID: RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT); break;//upstream only
						case ACL_CVLAN_CVID_CPOY_FROM_SP2C: cfRule.act.dsAct.cVidAct = CLASSIFY_DS_VID_ACT_TRANSLATION_SP2C; break;//downstream only
						case ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID: cfRule.act.dsAct.cVidAct = CLASSIFY_DS_VID_ACT_FROM_LUT; break; //downstream only
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;

					}
					cfRule.act.dsAct.cTagVid = classifyFilter->action_cvlan.assignedCvid;

					//Cpri decision
					switch(classifyFilter->action_cvlan.cvlanCpriDecision){
						case ACL_CVLAN_CPRI_NOP: cfRule.act.dsAct.cPriAct = CLASSIFY_DS_PRI_ACT_NOP; break;
						case ACL_CVLAN_CPRI_ASSIGN: cfRule.act.dsAct.cPriAct = CLASSIFY_DS_PRI_ACT_ASSIGN; break;
						case ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG: cfRule.act.dsAct.cPriAct = CLASSIFY_DS_PRI_ACT_FROM_1ST_TAG; break;
						case ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG: cfRule.act.dsAct.cPriAct = CLASSIFY_DS_PRI_ACT_FROM_2ND_TAG; break;
						case ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI: cfRule.act.dsAct.cPriAct = CLASSIFY_DS_PRI_ACT_FROM_INTERNAL; break;
						case ACL_CVLAN_CPRI_COPY_FROM_DSCP_REMAP: cfRule.act.dsAct.cPriAct = CLASSIFY_DS_PRI_ACT_FROM_DSCP; break;
						case ACL_CVLAN_CPRI_COPY_FROM_SP2C: cfRule.act.dsAct.cPriAct = CLASSIFY_DS_PRI_ACT_TRANSLATION_SP2C; break; //downstream only
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
					cfRule.act.dsAct.cTagPri = classifyFilter->action_cvlan.assignedCpri;
				}
				break;

				//CFPRI
				case CF_DS_ACTION_CFPRI_BIT:
				{
					switch(classifyFilter->action_cfpri.cfPriDecision){
						case ACL_CFPRI_ASSIGN: cfRule.act.dsAct.interPriAct = CLASSIFY_CF_PRI_ACT_ASSIGN; break;
						case ACL_CFPRI_NOP: cfRule.act.dsAct.interPriAct = CLASSIFY_CF_PRI_ACT_NOP; break;
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
				}
				break;

				//DSCP
				case CF_DS_ACTION_DSCP_BIT:
				{
					switch(classifyFilter->action_dscp.dscpDecision){
						case ACL_DSCP_ASSIGN: cfRule.act.dsAct.dscp = CLASSIFY_DSCP_ACT_ENABLE; break;
						case ACL_DSCP_NOP: cfRule.act.dsAct.dscp = CLASSIFY_DSCP_ACT_DISABLE; break;
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
				}
				break;

				//UNI
				case CF_DS_ACTION_UNI_MASK_BIT:
				{
					switch(classifyFilter->action_uni.uniActionDecision){
						case ACL_UNI_FWD_TO_PORTMASK_ONLY: cfRule.act.dsAct.uniAct=CLASSIFY_DS_UNI_ACT_MASK_BY_UNIMASK; break;
						case ACL_UNI_FORCE_BY_MASK: cfRule.act.dsAct.uniAct=CLASSIFY_DS_UNI_ACT_FORCE_FORWARD; break;
						case ACL_UNI_TRAP_TO_CPU: cfRule.act.dsAct.uniAct=CLASSIFY_DS_UNI_ACT_TRAP; break;
						case AL_UNI_NOP: cfRule.act.dsAct.uniAct=CLASSIFY_DS_UNI_ACT_NOP; break;
						default:
							RETURN_ERR(RT_ERR_RG_CF_NOT_SUPPORT);
							break;
					}
					cfRule.act.dsAct.uniMask.bits[0] = classifyFilter->action_uni.assignedUniPortMask;
				}
				break;

				default:
					break;
			}

		}
	}

	assert_ok(RTK_CLASSIFY_CFGENTRY_ADD(&cfRule));
	assert_ok(_rtk_rg_classifySWEntry_set(cfRule.index, *classifyFilter));

	return (RT_ERR_RG_OK);

}



int32 _rtk_rg_apollo_classifyEntry_del(int index)
{
	rtk_rg_classifyEntry_t empty_classifyFilter;

	//Check rg has been init
    if(rg_db.systemGlobal.vlanInit==0)
        RETURN_ERR(RT_ERR_RG_NOT_INIT);

	bzero(&empty_classifyFilter,sizeof(rtk_rg_classifyEntry_t));

	assert_ok(rtk_classify_cfgEntry_del(index));
	assert_ok(_rtk_rg_classifySWEntry_set(index, empty_classifyFilter));
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

int _rtk_rg_vlanExistInRgCheck(int vid)
{
	/*check the VLAN has been created in rg_db.
	every acl ingress cvid action should do this check for avoiding unsync between H/W & rg_db,
	because RTK ACL api will created H/W vlan automatically*/

	if(rg_db.vlan[vid].valid==ENABLED)
		return SUCCESS;
	else
		return FAIL;
}

int _rtk_rg_aclReservedEntry_init(void)
{
	rg_db.systemGlobal.aclAndCfReservedRule.acl_MC_temp_permit_idx = 0;// This value is assgined by reserved ACL, just for temp initial now.

	rg_db.systemGlobal.aclAndCfReservedRule.aclLowerBoundary = 0;
	rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary = (MAX_ACL_ENTRY_SIZE-1);

	rg_db.systemGlobal.aclAndCfReservedRule.cfLowerBoundary = 0;
	rg_db.systemGlobal.aclAndCfReservedRule.cfUpperBoundary = (rg_db.systemGlobal.cf_pattern0_size-1);

	//the multicast vlan protection initial value, releated to /proc/rg/acl_force_mc_cvid_when_rearrange
	rg_db.systemGlobal.acl_rearrange_force_mc_ingress_cvid = -1;


	return (RT_ERR_RG_OK);
}

static int _rtk_rg_aclAndCfReservedRuleHeadReflash(void)
{
	uint32 type;
#if defined(HW_ACL_REARRANGE_PROTECT_VERSION) && (HW_ACL_REARRANGE_PROTECT_VERSION==2)
	uint32 aclIdx=HW_ACL_REARRANGE_PROTECT_RSV_HEAD; //ACL[0],ACL[1],ACL[2] reserved for _rtk_rg_acl_for_multicast_temp_protection_add()
#else
	uint32 aclIdx=1;
#endif
	uint32 cfIdx=0;
	int addRuleFailedFlag=0;
	rtk_mac_t gmac,mac;
	uint16 pktLenEntryIdx;

	rtk_ipv6_addr_t ipv6_addr,ipv6_addr_mask;
	rtk_acl_ingress_entry_t aclRule;
	rtk_acl_field_t aclField,aclField2;
	rtk_acl_field_t aclField_dmac0,aclField_dmac1,aclField_dmac2;
	uint32 i;
	uint32 dip=0,mask=0,vid=0;

	//reflash th rules, ACL[0] is reserved for multicast temporary protection.
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
						ACL_RSV("rsv RTK_RG_ACLANDCF_RESERVED_STPBLOCKING @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
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
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule))
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
						aclRule.act.forwardAct.act= ACL_IGR_FORWARD_REDIRECT_ACT;
						aclRule.act.forwardAct.portMask.bits[0]= 0x0;	//drop
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_STPBLOCKING failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_ACL_SKIP_HW_REARRANGE_PERMIT_AND_TRAP_RESERV:
					ACL_RSV("rsv RTK_RG_ACLANDCF_RESERVED_ACL_SKIP_HW_REARRANGE_PERMIT_AND_TRAP_RESERV @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					//pospond the ASIC trap until first time call _rtk_rg_apollo_aclFilterAndQos_add()

					//ruleIdx rsv for IPv4 multicast permit
					rg_db.systemGlobal.aclAndCfReservedRule.aclSkipRearrangeIPv4MCPermitIdx = aclIdx;
					//point to next
					aclIdx++;

					//ruleIdx rsv for IPv6 multicast permit
					rg_db.systemGlobal.aclAndCfReservedRule.aclSkipRearrangeIPv6MCPermitIdx = aclIdx;
					//point to next
					aclIdx++;

					//ruleIdx rsv for rest trap
					rg_db.systemGlobal.aclAndCfReservedRule.aclSkipRearrangeAlltTrapIdx = aclIdx;
					//point to next
					aclIdx++;
					break;

				case RTK_RG_ACLANDCF_RESERVED_INTF15_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF15_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[15].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[15]=aclIdx; //record former rule index
					goto dhcpTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF14_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF14_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[14].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[14]=aclIdx; //record former rule index
					goto dhcpTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF13_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF13_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[13].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[13]=aclIdx; //record former rule index
					goto dhcpTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF12_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF12_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[12].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[12]=aclIdx; //record former rule index
					goto dhcpTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF11_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF11_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[11].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[11]=aclIdx; //record former rule index
					goto dhcpTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF10_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF10_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[10].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[10]=aclIdx; //record former rule index
					goto dhcpTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF9_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF9_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[9].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[9]=aclIdx; //record former rule index
					goto dhcpTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF8_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF8_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[8].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[8]=aclIdx; //record former rule index
					goto dhcpTrapRule;

				case RTK_RG_ACLANDCF_RESERVED_INTF7_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF7_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[7].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[7]=aclIdx; //record former rule index
					goto dhcpTrapRule;

				case RTK_RG_ACLANDCF_RESERVED_INTF6_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF6_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[6].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[6]=aclIdx; //record former rule index
					goto dhcpTrapRule;

				case RTK_RG_ACLANDCF_RESERVED_INTF5_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF5_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[5].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[5]=aclIdx; //record former rule index
					goto dhcpTrapRule;

				case RTK_RG_ACLANDCF_RESERVED_INTF4_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF4_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[4].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[4]=aclIdx; //record former rule index
					goto dhcpTrapRule;

				case RTK_RG_ACLANDCF_RESERVED_INTF3_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF3_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[3].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[3]=aclIdx; //record former rule index
					goto dhcpTrapRule;

				case RTK_RG_ACLANDCF_RESERVED_INTF2_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF2_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[2].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[2]=aclIdx; //record former rule index
					goto dhcpTrapRule;

				case RTK_RG_ACLANDCF_RESERVED_INTF1_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF1_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[1].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[1]=aclIdx; //record former rule index
					goto dhcpTrapRule;

				case RTK_RG_ACLANDCF_RESERVED_INTF0_DHCP_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF0_DHCP_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[0].gmac.octet[0],ETHER_ADDR_LEN);
					rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[0]=aclIdx; //record former rule index
					goto dhcpTrapRule;

dhcpTrapRule:
					/*start to set ACL rule*/
					bzero(&aclRule,sizeof(aclRule));
					bzero(&aclField_dmac0,sizeof(aclField_dmac0));
					bzero(&aclField_dmac1,sizeof(aclField_dmac1));
					bzero(&aclField_dmac2,sizeof(aclField_dmac2));
					aclRule.valid=ENABLED;
					aclRule.index = aclIdx;
					aclRule.activePorts.bits[0]=(1<<RTK_RG_MAC_PORT_PON);
					aclRule.templateIdx=0; //dmac
					//setup gmac
					aclField_dmac2.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_dmac2.fieldUnion.pattern.fieldIdx = 0; //DA[15:0]: template[0] field[0]
					aclField_dmac2.fieldUnion.data.value = (gmac.octet[4]<<8) | (gmac.octet[5]);
					aclField_dmac2.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac2)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFX_DHCP_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					aclField_dmac1.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_dmac1.fieldUnion.pattern.fieldIdx = 1; //DA[31:16]: template[0] field[1]
					aclField_dmac1.fieldUnion.data.value = (gmac.octet[2]<<8) | (gmac.octet[3]);
					aclField_dmac1.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac1)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFX_DHCP_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					aclField_dmac0.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_dmac0.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
					aclField_dmac0.fieldUnion.data.value = (gmac.octet[0]<<8) | (gmac.octet[1]);
					aclField_dmac0.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac0)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFX_DHCP_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
					//other mode trap to CPU to keep original
					aclRule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;

					if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFX_DHCP_TRAP failed!!!");
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
					aclRule.activePorts.bits[0]=(1<<RTK_RG_MAC_PORT_PON);
					aclRule.templateIdx=2; //l4_sport, l4_dport: Template[2]
					//setup l4_sport
					aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField.fieldUnion.pattern.fieldIdx = 1; //sport: template[2] field[1]
					aclField.fieldUnion.data.value = 67; //DHCP l4_port
					aclField.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFX_DHCP_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					//setup l4_dport
					aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField2.fieldUnion.pattern.fieldIdx = 0; //sport: template[2] field[0]
					aclField2.fieldUnion.data.value = 68; //DHCP l4_port
					aclField2.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField2)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFX_DHCP_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFX_DHCP_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					//point to next ruleIdx
					aclIdx++;

					break;


				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_RMA_TRAP:
					{
						ACL_RSV("add RTK_RG_ACLANDCF_RESERVED_MULTICAST_RMA_TRAP @ acl[%d]",aclIdx);


						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						bzero(&aclField2,sizeof(aclField2));

						//trap 224.0.0.X
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 0; //DIP[15:0]
						aclField.fieldUnion.data.value = 0x0;
						aclField.fieldUnion.data.mask = 0xff00;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_RMA_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField2.fieldUnion.pattern.fieldIdx = 1; //DIP[31:16]
						aclField2.fieldUnion.data.value = 0xe000;
						aclField2.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField2))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_RMA_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.templateIdx=1;
						aclRule.careTag.tags[ACL_CARE_TAG_IPV4].value= 1;
						aclRule.careTag.tags[ACL_CARE_TAG_IPV4].mask=0xffff;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_RMA_TRAP failed!!!");
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
						aclRule.careTag.tags[ACL_CARE_TAG_IPV4].value= 1;
						aclRule.careTag.tags[ACL_CARE_TAG_IPV4].mask=0xffff;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_LAN_PORTMASK;
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						ACL_RSV("add RTK_RG_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP @ acl[%d]",aclIdx);

						//point to next ruleIdx
						aclIdx++;
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_WIFI2_IPC_ROUTING_TRAP:
					{
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						bzero(&aclField2,sizeof(aclField2));

						//trap 10.253.253.0 / 29
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 0; //DIP[15:0]
						aclField.fieldUnion.data.value = 0xfd00;
						aclField.fieldUnion.data.mask = 0xfffc;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_WIFI2_IPC_ROUTING_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField2.fieldUnion.pattern.fieldIdx = 1; //DIP[31:16]
						aclField2.fieldUnion.data.value = 0x0afd;
						aclField2.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField2))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_WIFI2_IPC_ROUTING_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.templateIdx=1;
						aclRule.careTag.tags[ACL_CARE_TAG_IPV4].value= 1;
						aclRule.careTag.tags[ACL_CARE_TAG_IPV4].mask=0xffff;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_LAN_PORTMASK;
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_WIFI2_IPC_ROUTING_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						ACL_RSV("add RTK_RG_ACLANDCF_RESERVED_WIFI2_IPC_ROUTING_TRAP @ acl[%d]",aclIdx);

						//point to next ruleIdx
						aclIdx++;
					}
					break;


				case RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP:
					{

						if(rg_kernel.layer2LookupMissFlood2CPU==RTK_RG_ENABLED){//support proc/rg/layer2LookupMissFlood2CPU
							ACL_RSV("skip add RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP because enabled proc/rg/layer2LookupMissFlood2CPU");
						}
						else
						{
							ACL_RSV("add RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP @ acl[%d]",aclIdx);
							bzero(&aclRule,sizeof(aclRule));
							bzero(&aclField_dmac0,sizeof(aclField_dmac0));
							bzero(&aclField_dmac1,sizeof(aclField_dmac1));
							bzero(&aclField_dmac2,sizeof(aclField_dmac2));

							//setup broadcast DMAC
							aclField_dmac2.fieldType = ACL_FIELD_PATTERN_MATCH;
							aclField_dmac2.fieldUnion.pattern.fieldIdx = 0; //DA[15:0]: template[0] field[0]
							aclField_dmac2.fieldUnion.data.value = 0xffff;
							aclField_dmac2.fieldUnion.data.mask = 0xffff;
							if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac2))
							{
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP failed!!!");
								addRuleFailedFlag=1;
								break;
							}
							aclField_dmac1.fieldType = ACL_FIELD_PATTERN_MATCH;
							aclField_dmac1.fieldUnion.pattern.fieldIdx = 1; //DA[31:16]: template[0] field[1]
							aclField_dmac1.fieldUnion.data.value = 0xffff;
							aclField_dmac1.fieldUnion.data.mask = 0xffff;
							if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac1))
							{
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							aclField_dmac0.fieldType = ACL_FIELD_PATTERN_MATCH;
							aclField_dmac0.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
							aclField_dmac0.fieldUnion.data.value = 0xffff;
							aclField_dmac0.fieldUnion.data.mask = 0xffff;
							if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac0))
							{
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							aclRule.valid=ENABLED;
							aclRule.index=aclIdx;
							aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
							aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
							aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;

							if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule))
							{
								ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							//point to next ruleIdx
							aclIdx++;
						}
#ifndef HW_ACL_REARRANGE_PROTECT_VERSION
						//keep this index in rg_db. while user ACL rearrange this index will be used.
						rg_db.systemGlobal.aclAndCfReservedRule.acl_MC_temp_permit_idx = aclIdx;
						//point to next ruleIdx
						aclIdx++;
#endif
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_ALL_TRAP:
					{
						ACL_RSV("add RTK_RG_ACLANDCF_RESERVED_ALL_TRAP @ acl[%d]",aclIdx);
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;

						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ALL_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						//point to next ruleIdx
						aclIdx++;
#ifndef HW_ACL_REARRANGE_PROTECT_VERSION
						//keep this index in rg_db. while user ACL rearrange this index will be used.
						rg_db.systemGlobal.aclAndCfReservedRule.acl_MC_temp_permit_idx = aclIdx;
						//point to next ruleIdx
						aclIdx++;
#endif
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_UNICAST_TRAP:
					{
						ACL_RSV("add RTK_RG_ACLANDCF_RESERVED_UNICAST_TRAP @ acl[%d]",aclIdx);
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


						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_UNICAST_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						//point to next ruleIdx
						aclIdx++;

					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP:
					{
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP @ acl[%d]",aclIdx);

						//Set ACL rule for trap all SYN packet
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));

						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
						aclRule.templateIdx=3; /*use :FS[2] => template[3],field[2]*/
						aclRule.careTag.tags[ACL_CARE_TAG_TCP].value=ENABLED;
						aclRule.careTag.tags[ACL_CARE_TAG_TCP].mask=0xffff;

						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 2;/*template[3],field[2]*/
						aclField.fieldUnion.data.value = 0x2; //SYN flag
						aclField.fieldUnion.data.mask = 0x2;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//force fwd to CPU port (avoid egress vlan filter useless)
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;

						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;

					}
					break;
				case RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY:
					{
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);

						//Set ACL rule for trap all SYN packet
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));

						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
						aclRule.templateIdx=3; /*use :FS[2] => template[3],field[2]*/
						aclRule.careTag.tags[ACL_CARE_TAG_TCP].value=ENABLED;
						aclRule.careTag.tags[ACL_CARE_TAG_TCP].mask=0xffff;

						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 2;/*template[3],field[2]*/
						aclField.fieldUnion.data.value = 0x10; //ACK flag
						aclField.fieldUnion.data.mask = 0x10;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//force fwd to CPU port (avoid egress vlan filter useless)
						aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
						aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
						aclRule.act.priAct.aclPri=rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_assign_priority.priority;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;

						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));

						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
						aclRule.templateIdx=2; /*template[2],field[7]*/

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
						aclField.fieldUnion.data.value = 0x1<<pktLenEntryIdx; //using pktLen[1]
						aclField.fieldUnion.data.mask = 0x1<<pktLenEntryIdx;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY:
					{
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY @ acl[%d]",aclIdx);

						//Set ACL rule for higher rx prioirty for PPPoE LCP packet(eth=0x8864, none-ipv4, none-ipv6)
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));

						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=(1<<RTK_RG_PORT_PON);
						aclRule.templateIdx=1; /*use :ethertype=> template[1],field[5]*/
						aclRule.careTag.tags[ACL_CARE_TAG_IPV4].value=DISABLED;
						aclRule.careTag.tags[ACL_CARE_TAG_IPV4].mask=0xffff;
						aclRule.careTag.tags[ACL_CARE_TAG_IPV6].value=DISABLED;
						aclRule.careTag.tags[ACL_CARE_TAG_IPV6].mask=0xffff;


						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 5;/*template[1],field[5]*/
						aclField.fieldUnion.data.value = 0x8864;
						aclField.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//force fwd to CPU port (avoid egress vlan filter useless)
						aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
						aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
						aclRule.act.priAct.aclPri=rg_db.systemGlobal.aclAndCfReservedRule.pppoe_lcp_assign_prioity.priority;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;
					}
					break;
				case RTK_RG_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY:
					{
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY @ acl[%d]",aclIdx);

						//Set ACL rule for higher rx prioirty for L2TP LCP/Control packet(UDP=1701, pktLen < 128 assumed control packet usually is small packet)
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						bzero(&aclField2,sizeof(aclField2));
						bzero(&aclField_dmac0,sizeof(aclField_dmac0));

						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=(1<<RTK_RG_PORT_PON);
						aclRule.templateIdx=2; /*use :ethertype=> template[2],field[0]&field[1]*/
						aclRule.careTag.tags[ACL_CARE_TAG_UDP].value=ENABLED;
						aclRule.careTag.tags[ACL_CARE_TAG_UDP].mask=0xffff;

						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 0;/*template[2],field[0]*/
						aclField.fieldUnion.data.value = 1701;
						aclField.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField2.fieldUnion.pattern.fieldIdx = 1;/*template[2],field[1]*/
						aclField2.fieldUnion.data.value = 1701;
						aclField2.fieldUnion.data.mask = 0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField2)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//limit packet size is samller than 128
						pktLenEntryIdx = 0;
						if(_rtk_rg_search_acl_pktLenRangeTableEntry(&pktLenEntryIdx, TRUE, 128, 0)!=RT_ERR_RG_OK)
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclField_dmac0.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField_dmac0.fieldUnion.pattern.fieldIdx = 7;/*template[2],field[7]*/
						aclField_dmac0.fieldUnion.data.value = 0x1<<pktLenEntryIdx; //using pktLen[0]
						aclField_dmac0.fieldUnion.data.mask = 0x1<<pktLenEntryIdx;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac0)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
						aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
						aclRule.act.priAct.aclPri=rg_db.systemGlobal.aclAndCfReservedRule.l2tp_ctrl_lcp_assign_prioity.priority;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;

					}
					break;



				case RTK_RG_ACLANDCF_RESERVED_PORTBINDING_FAIL_TRAP:
					{
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PORTBINDING_FAIL_TRAP @ acl[%d] & acl[%d] & acl[%d]",aclIdx,aclIdx+1,aclIdx+2);
						//trap none-IPv4, none-IPv6 ctag packets (none-IP fail with port binding in apolloFE)
						bzero(&aclRule,sizeof(aclRule));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.templateIdx=0;
						aclRule.careTag.tags[ACL_CARE_TAG_CTAG].value= 1; //ctag
						aclRule.careTag.tags[ACL_CARE_TAG_CTAG].mask=0xffff;
						aclRule.careTag.tags[ACL_CARE_TAG_IPV4].value= 0; //none-ipv4
						aclRule.careTag.tags[ACL_CARE_TAG_IPV4].mask=0xffff;
						aclRule.careTag.tags[ACL_CARE_TAG_IPV6].value= 0; //none-ipv6
						aclRule.careTag.tags[ACL_CARE_TAG_IPV6].mask=0xffff;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_LAN_PORTMASK;
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PORTBINDING_FAIL_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;

						//trap mutlicast IPv4 (PPPoE multicast fail with port binding in apolloFE)
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));

						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 1; //DIP[31:16]	DIP : 224.0.0.0 ~ 239.255.255.255 (just care 0xeX.XX.XX.XX)
						aclField.fieldUnion.data.value = 0xe000;
						aclField.fieldUnion.data.mask = 0xf000;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PORTBINDING_FAIL_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.templateIdx=1;
						aclRule.careTag.tags[ACL_CARE_TAG_IPV4].value= 1;
						aclRule.careTag.tags[ACL_CARE_TAG_IPV4].mask=0xffff;
						aclRule.activePorts.bits[0]=RTK_RG_ALL_LAN_PORTMASK;
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_RMA_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;

						//trap LAN mutlicast IPv6 (PPPoE multicast fail with port binding in apolloFE)
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
						aclRule.activePorts.bits[0] = RTK_RG_ALL_LAN_PORTMASK;
						//trap to cpu action
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
						aclRule.act.forwardAct.act= ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_URLFILTER_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF0_NONE_IP_VLAN_TRANSLATE:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF0_NONE_IP_VLAN_TRANSLATE @ acl[%d]",aclIdx);
					mask=rg_db.systemGlobal.aclAndCfReservedRule.none_ip_bind_translate_vid[0].portmask;
					vid=rg_db.systemGlobal.aclAndCfReservedRule.none_ip_bind_translate_vid[0].ingress_vid;
					goto noneIpBindTranlateVidRule;
				case RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF1_NONE_IP_VLAN_TRANSLATE:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF1_NONE_IP_VLAN_TRANSLATE @ acl[%d]",aclIdx);
					mask=rg_db.systemGlobal.aclAndCfReservedRule.none_ip_bind_translate_vid[1].portmask;
					vid=rg_db.systemGlobal.aclAndCfReservedRule.none_ip_bind_translate_vid[1].ingress_vid;
					goto noneIpBindTranlateVidRule;
				case RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF2_NONE_IP_VLAN_TRANSLATE:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF2_NONE_IP_VLAN_TRANSLATE @ acl[%d]",aclIdx);
					mask=rg_db.systemGlobal.aclAndCfReservedRule.none_ip_bind_translate_vid[2].portmask;
					vid=rg_db.systemGlobal.aclAndCfReservedRule.none_ip_bind_translate_vid[2].ingress_vid;
					goto noneIpBindTranlateVidRule;
				case RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF3_NONE_IP_VLAN_TRANSLATE:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF3_NONE_IP_VLAN_TRANSLATE @ acl[%d]",aclIdx);
					mask=rg_db.systemGlobal.aclAndCfReservedRule.none_ip_bind_translate_vid[3].portmask;
					vid=rg_db.systemGlobal.aclAndCfReservedRule.none_ip_bind_translate_vid[3].ingress_vid;
					goto noneIpBindTranlateVidRule;
				case RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF4_NONE_IP_VLAN_TRANSLATE:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF4_NONE_IP_VLAN_TRANSLATE @ acl[%d]",aclIdx);
					mask=rg_db.systemGlobal.aclAndCfReservedRule.none_ip_bind_translate_vid[4].portmask;
					vid=rg_db.systemGlobal.aclAndCfReservedRule.none_ip_bind_translate_vid[4].ingress_vid;
					goto noneIpBindTranlateVidRule;
				case RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF5_NONE_IP_VLAN_TRANSLATE:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF5_NONE_IP_VLAN_TRANSLATE @ acl[%d]",aclIdx);
					mask=rg_db.systemGlobal.aclAndCfReservedRule.none_ip_bind_translate_vid[5].portmask;
					vid=rg_db.systemGlobal.aclAndCfReservedRule.none_ip_bind_translate_vid[5].ingress_vid;
					goto noneIpBindTranlateVidRule;
				case RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF6_NONE_IP_VLAN_TRANSLATE:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF6_NONE_IP_VLAN_TRANSLATE @ acl[%d]",aclIdx);
					mask=rg_db.systemGlobal.aclAndCfReservedRule.none_ip_bind_translate_vid[6].portmask;
					vid=rg_db.systemGlobal.aclAndCfReservedRule.none_ip_bind_translate_vid[6].ingress_vid;
					goto noneIpBindTranlateVidRule;
				case RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF7_NONE_IP_VLAN_TRANSLATE:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF7_NONE_IP_VLAN_TRANSLATE @ acl[%d]",aclIdx);
					mask=rg_db.systemGlobal.aclAndCfReservedRule.none_ip_bind_translate_vid[7].portmask;
					vid=rg_db.systemGlobal.aclAndCfReservedRule.none_ip_bind_translate_vid[7].ingress_vid;
					goto noneIpBindTranlateVidRule;

noneIpBindTranlateVidRule:
					{
						//none-IPv4, none-IPv6 packets without VlanTag, translate to the binding BridgeWan VID.
						bzero(&aclRule,sizeof(aclRule));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.templateIdx=0;
						aclRule.careTag.tags[ACL_CARE_TAG_IPV4].value= 0; //none-ipv4
						aclRule.careTag.tags[ACL_CARE_TAG_IPV4].mask=0xffff;
						aclRule.careTag.tags[ACL_CARE_TAG_IPV6].value= 0; //none-ipv6
						aclRule.careTag.tags[ACL_CARE_TAG_IPV6].mask=0xffff;
						aclRule.careTag.tags[ACL_CARE_TAG_CTAG].value= 0; //none-ctag
						aclRule.careTag.tags[ACL_CARE_TAG_CTAG].mask=0xffff;
						aclRule.activePorts.bits[0]=mask;
						aclRule.act.enableAct[ACL_IGR_CVLAN_ACT]=ENABLED;
						aclRule.act.cvlanAct.act=ACL_IGR_CVLAN_IGR_CVLAN_ACT;
						aclRule.act.cvlanAct.cvid=vid;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTFn_NONE_IP_VLAN_TRANSLATE failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx++;
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[0].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[0].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF1_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[1].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[1].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF2_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[2].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[2].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF3_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[3].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[3].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF4_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[4].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[4].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF5_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[5].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[5].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF6_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[6].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[6].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF7_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[7].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[7].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF8_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF8_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[8].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[8].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF9_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF9_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[9].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[9].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF10_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF10_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[10].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[10].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF11_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF11_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[11].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[11].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF12_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF12_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[12].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[12].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF13_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF13_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[13].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[13].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF14_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF14_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[14].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[14].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_INTF15_DSLITE_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_INTF15_DSLITE_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&mac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[15].smac.octet[0],ETHER_ADDR_LEN);
					memcpy(&ipv6_addr.ipv6_addr[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[15].ipv6_dip.ipv6_addr[0],IPV6_ADDR_LEN);
					goto dsliteTrapRule;

dsliteTrapRule:
					/*start to set ACL rule*/
					bzero(&aclRule,sizeof(aclRule));
					bzero(&rg_db.systemGlobal.aclField_ar[0],sizeof(rtk_acl_field_t));
					bzero(&rg_db.systemGlobal.aclField_ar[1],sizeof(rtk_acl_field_t));
					bzero(&rg_db.systemGlobal.aclField_ar[2],sizeof(rtk_acl_field_t));
					aclRule.valid=ENABLED;
					aclRule.index = aclIdx;
					aclRule.activePorts.bits[0]=(1<<RTK_RG_MAC_PORT_PON);
					aclRule.templateIdx=0; //smac
					//setup smac
					rg_db.systemGlobal.aclField_ar[2].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField_ar[2].fieldUnion.pattern.fieldIdx = 3; //SA[15:0]: template[0] field[3]
					rg_db.systemGlobal.aclField_ar[2].fieldUnion.data.value = (mac.octet[4]<<8) | (mac.octet[5]);
					rg_db.systemGlobal.aclField_ar[2].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &rg_db.systemGlobal.aclField_ar[2])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					rg_db.systemGlobal.aclField_ar[1].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField_ar[1].fieldUnion.pattern.fieldIdx = 4; //SA[31:16]: template[0] field[4]
					rg_db.systemGlobal.aclField_ar[1].fieldUnion.data.value = (mac.octet[2]<<8) | (mac.octet[3]);
					rg_db.systemGlobal.aclField_ar[1].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &rg_db.systemGlobal.aclField_ar[1])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					rg_db.systemGlobal.aclField_ar[0].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField_ar[0].fieldUnion.pattern.fieldIdx = 5; //SA[47:32]: template[0] field[5]
					rg_db.systemGlobal.aclField_ar[0].fieldUnion.data.value = (mac.octet[0]<<8) | (mac.octet[1]);
					rg_db.systemGlobal.aclField_ar[0].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &rg_db.systemGlobal.aclField_ar[0])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
					aclRule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;

					if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					//point to next ruleIdx
					aclIdx++;


					bzero(&aclRule,sizeof(aclRule));
					bzero(rg_db.systemGlobal.aclField_ar,sizeof(rtk_acl_field_t)*8);
					aclRule.valid=ENABLED;
					aclRule.index = aclIdx;
					aclRule.activePorts.bits[0]=(1<<RTK_RG_MAC_PORT_PON);
					aclRule.templateIdx=6; //ipv6_dip template[6]

					//setup DIPv6
					rg_db.systemGlobal.aclField_ar[0].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField_ar[0].fieldUnion.pattern.fieldIdx = 7;
					rg_db.systemGlobal.aclField_ar[0].fieldUnion.data.value = (ipv6_addr.ipv6_addr[0]<<8|ipv6_addr.ipv6_addr[1]); //DIP[]
					rg_db.systemGlobal.aclField_ar[0].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &rg_db.systemGlobal.aclField_ar[0])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					rg_db.systemGlobal.aclField_ar[1].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField_ar[1].fieldUnion.pattern.fieldIdx = 6;
					rg_db.systemGlobal.aclField_ar[1].fieldUnion.data.value = (ipv6_addr.ipv6_addr[2]<<8|ipv6_addr.ipv6_addr[3]); //DIP[]
					rg_db.systemGlobal.aclField_ar[1].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &rg_db.systemGlobal.aclField_ar[1])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					rg_db.systemGlobal.aclField_ar[2].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField_ar[2].fieldUnion.pattern.fieldIdx = 5;
					rg_db.systemGlobal.aclField_ar[2].fieldUnion.data.value = (ipv6_addr.ipv6_addr[4]<<8|ipv6_addr.ipv6_addr[5]); //DIP[]
					rg_db.systemGlobal.aclField_ar[2].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &rg_db.systemGlobal.aclField_ar[2])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					rg_db.systemGlobal.aclField_ar[3].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField_ar[3].fieldUnion.pattern.fieldIdx = 4;
					rg_db.systemGlobal.aclField_ar[3].fieldUnion.data.value = (ipv6_addr.ipv6_addr[6]<<8|ipv6_addr.ipv6_addr[7]); //DIP[]
					rg_db.systemGlobal.aclField_ar[3].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &rg_db.systemGlobal.aclField_ar[3])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					rg_db.systemGlobal.aclField_ar[4].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField_ar[4].fieldUnion.pattern.fieldIdx = 3;
					rg_db.systemGlobal.aclField_ar[4].fieldUnion.data.value = (ipv6_addr.ipv6_addr[8]<<8|ipv6_addr.ipv6_addr[9]); //DIP[]
					rg_db.systemGlobal.aclField_ar[4].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &rg_db.systemGlobal.aclField_ar[4])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					rg_db.systemGlobal.aclField_ar[5].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField_ar[5].fieldUnion.pattern.fieldIdx = 2;
					rg_db.systemGlobal.aclField_ar[5].fieldUnion.data.value = (ipv6_addr.ipv6_addr[10]<<8|ipv6_addr.ipv6_addr[11]); //DIP[]
					rg_db.systemGlobal.aclField_ar[5].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &rg_db.systemGlobal.aclField_ar[5])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					rg_db.systemGlobal.aclField_ar[6].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField_ar[6].fieldUnion.pattern.fieldIdx = 1;
					rg_db.systemGlobal.aclField_ar[6].fieldUnion.data.value = (ipv6_addr.ipv6_addr[12]<<8|ipv6_addr.ipv6_addr[13]); //DIP[]
					rg_db.systemGlobal.aclField_ar[6].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &rg_db.systemGlobal.aclField_ar[6])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					rg_db.systemGlobal.aclField_ar[7].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField_ar[7].fieldUnion.pattern.fieldIdx = 0;
					rg_db.systemGlobal.aclField_ar[7].fieldUnion.data.value = (ipv6_addr.ipv6_addr[14]<<8|ipv6_addr.ipv6_addr[15]); //DIP[]
					rg_db.systemGlobal.aclField_ar[7].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &rg_db.systemGlobal.aclField_ar[7])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_INTFx_DSLITE_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					//point to next ruleIdx
					aclIdx++;
					break;


			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF0_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF0_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[0].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF1_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF1_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[1].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF2_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF2_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[2].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF3_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF3_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[3].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF4_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF4_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[4].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF5_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF5_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[5].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF6_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF6_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[6].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF7_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF7_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[7].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;


			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF8_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF8_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[8].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF9_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF9_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[9].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF10_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF10_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[10].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF11_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF11_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[11].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF12_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF12_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[12].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF13_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF13_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[13].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF14_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF14_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[14].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF15_MC_ROUTING_TRAP:
				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF15_MC_ROUTING_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[15].gmac.octet[0],ETHER_ADDR_LEN);
				}
				goto pppoeMcRoutingTrapRule;

pppoeMcRoutingTrapRule:

				if(rg_db.systemGlobal.pppoe_mc_routing_trap==RTK_RG_ENABLED){//rule add  only when proc enabled

					/*start to set ACL rule*/
					bzero(&aclRule,sizeof(aclRule));
					bzero(rg_db.systemGlobal.aclField_ar,sizeof(rtk_acl_field_t)*8);
					aclRule.valid=ENABLED;
					aclRule.index = aclIdx;
					aclRule.activePorts.bits[0]=(1<<RTK_RG_MAC_PORT_PON);
					aclRule.careTag.tags[ACL_CARE_TAG_IPV4].value=1;
					aclRule.careTag.tags[ACL_CARE_TAG_IPV4].mask=0xffff;
					aclRule.templateIdx=0; //dmac
					//setup gmac
					rg_db.systemGlobal.aclField_ar[2].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField_ar[2].fieldUnion.pattern.fieldIdx = 0; //DA[15:0]: template[0] field[0]
					rg_db.systemGlobal.aclField_ar[2].fieldUnion.data.value = (gmac.octet[4]<<8) | (gmac.octet[5]);
					rg_db.systemGlobal.aclField_ar[2].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &rg_db.systemGlobal.aclField_ar[2])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPOE_INTFx_MC_ROUTING_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					rg_db.systemGlobal.aclField_ar[1].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField_ar[1].fieldUnion.pattern.fieldIdx = 1; //DA[31:16]: template[0] field[1]
					rg_db.systemGlobal.aclField_ar[1].fieldUnion.data.value = (gmac.octet[2]<<8) | (gmac.octet[3]);
					rg_db.systemGlobal.aclField_ar[1].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &rg_db.systemGlobal.aclField_ar[1])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPOE_INTFx_MC_ROUTING_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					rg_db.systemGlobal.aclField_ar[0].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField_ar[0].fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
					rg_db.systemGlobal.aclField_ar[0].fieldUnion.data.value = (gmac.octet[0]<<8) | (gmac.octet[1]);
					rg_db.systemGlobal.aclField_ar[0].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &rg_db.systemGlobal.aclField_ar[0])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPOE_INTFx_MC_ROUTING_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
					aclRule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;

					if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPOE_INTFx_MC_ROUTING_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					//point to next ruleIdx
					aclIdx++;



					bzero(&aclRule,sizeof(aclRule));
					bzero(rg_db.systemGlobal.aclField_ar,sizeof(rtk_acl_field_t)*8);
					aclRule.valid=ENABLED;
					aclRule.index = aclIdx;
					aclRule.activePorts.bits[0]=(1<<RTK_RG_MAC_PORT_PON);
					aclRule.careTag.tags[ACL_CARE_TAG_IPV4].value=1;
					aclRule.careTag.tags[ACL_CARE_TAG_IPV4].mask=0xffff;
					aclRule.templateIdx=1; //ethertype
					//setup ethertype
					rg_db.systemGlobal.aclField_ar[3].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField_ar[3].fieldUnion.pattern.fieldIdx = 5; //ethertype: template[1] field[5]
					rg_db.systemGlobal.aclField_ar[3].fieldUnion.data.value = 0x8864;
					rg_db.systemGlobal.aclField_ar[3].fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &rg_db.systemGlobal.aclField_ar[3])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPOE_INTFx_MC_ROUTING_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					//setup DIP
					rg_db.systemGlobal.aclField_ar[0].fieldType = ACL_FIELD_PATTERN_MATCH;
					rg_db.systemGlobal.aclField_ar[0].fieldUnion.pattern.fieldIdx = 1;
					rg_db.systemGlobal.aclField_ar[0].fieldUnion.data.value = 0xe000; //multicast IP leading with 1110 ...
					rg_db.systemGlobal.aclField_ar[0].fieldUnion.data.mask = 0xf000;
					if(rtk_acl_igrRuleField_add(&aclRule, &rg_db.systemGlobal.aclField_ar[0])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPOE_INTFx_MC_ROUTING_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
					if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPOE_INTFx_MC_ROUTING_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					//point to next ruleIdx
					aclIdx++;
				}
				break;

			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF0_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF0_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[0].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF1_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF1_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[1].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF2_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF2_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[2].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF3_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF3_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[3].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF4_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF4_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[4].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF5_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF5_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[5].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF6_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF6_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[6].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF7_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF7_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[7].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule;


			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF8_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF8_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[8].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF9_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF9_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[9].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF10_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF10_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[10].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF11_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF11_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[11].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF12_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF12_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[12].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF13_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF13_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[13].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF14_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF14_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[14].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF15_LINK_LOCAL_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IPV6_INTF15_LINK_LOCAL_TRAP @ acl[%d] & acl[%d]",aclIdx,aclIdx+1);
				memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[15].gmac.octet[0],ETHER_ADDR_LEN);
				goto linkLocalTrapRule;

linkLocalTrapRule:
				bzero(&aclRule,sizeof(aclRule));
				bzero(&aclField_dmac2,sizeof(aclField_dmac2));
				bzero(&aclField_dmac1,sizeof(aclField_dmac1));
				bzero(&aclField_dmac0,sizeof(aclField_dmac0));

				/*set rule for gateway mac*/
				aclRule.valid=ENABLED;
				aclRule.index=aclIdx;
				aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
				aclRule.careTag.tags[ACL_CARE_TAG_IPV6].value=ENABLED;
				aclRule.careTag.tags[ACL_CARE_TAG_IPV6].mask=0xffff;
				aclRule.templateIdx=0; /*IPv6 DIP[127:112]: template[3],field[5]*/

				//setup gmac
				aclField_dmac2.fieldType = ACL_FIELD_PATTERN_MATCH;
				aclField_dmac2.fieldUnion.pattern.fieldIdx = 0; //DA[15:0]: template[0] field[0]
				aclField_dmac2.fieldUnion.data.value = (gmac.octet[4]<<8) | (gmac.octet[5]);
				aclField_dmac2.fieldUnion.data.mask = 0xffff;
				if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac2)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_IPV6_INTFX_LINK_LOCAL_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}

				aclField_dmac1.fieldType = ACL_FIELD_PATTERN_MATCH;
				aclField_dmac1.fieldUnion.pattern.fieldIdx = 1; //DA[31:16]: template[0] field[1]
				aclField_dmac1.fieldUnion.data.value = (gmac.octet[2]<<8) | (gmac.octet[3]);
				aclField_dmac1.fieldUnion.data.mask = 0xffff;
				if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac1)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_IPV6_INTFX_LINK_LOCAL_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}

				aclField_dmac0.fieldType = ACL_FIELD_PATTERN_MATCH;
				aclField_dmac0.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
				aclField_dmac0.fieldUnion.data.value = (gmac.octet[0]<<8) | (gmac.octet[1]);
				aclField_dmac0.fieldUnion.data.mask = 0xffff;
				if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac0)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_IPV6_INTFX_LINK_LOCAL_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}

				//trap to cpu action
				aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
				aclRule.act.forwardAct.act= ACL_IGR_FORWARD_TRAP_ACT;

				//continuous hit, no need to set action
				if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_IPV6_INTFX_LINK_LOCAL_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}

				//point to next ruleIdx
				aclIdx++;

				/*set rule for link local IP*/
				bzero(&aclRule,sizeof(aclRule));
				bzero(&aclField,sizeof(aclField));
				aclRule.valid=ENABLED;
				aclRule.index=aclIdx;
				aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
				aclRule.careTag.tags[ACL_CARE_TAG_IPV6].value=ENABLED;
				aclRule.careTag.tags[ACL_CARE_TAG_IPV6].mask=0xffff;
				aclRule.templateIdx=6; /*IPv6 DIP[127:112]: template[6],field[7]*/

				aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
				aclField.fieldUnion.pattern.fieldIdx = 7;//template[6],field[7] = IPv6 DIP[127:112]
				aclField.fieldUnion.data.value=0xfe80; //IPv6 DIP[127:112]
				aclField.fieldUnion.data.mask=0xffff;
				if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_IPV6_INTFX_LINK_LOCAL_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}
				if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_IPV6_INTFX_LINK_LOCAL_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}
				//point to next ruleIdx
				aclIdx++;
				break;


			case RTK_RG_ACLANDCF_RESERVED_RULE0_DIP_MASK_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE0_DIP_MASK_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[0].dip;
				mask = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[0].mask;
				goto dipMaskTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_RULE1_DIP_MASK_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE1_DIP_MASK_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[1].dip;
				mask = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[1].mask;
				goto dipMaskTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_RULE2_DIP_MASK_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE2_DIP_MASK_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[2].dip;
				mask = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[2].mask;
				goto dipMaskTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_RULE3_DIP_MASK_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE3_DIP_MASK_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[3].dip;
				mask = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[3].mask;
				goto dipMaskTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_RULE4_DIP_MASK_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE4_DIP_MASK_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[4].dip;
				mask = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[4].mask;
				goto dipMaskTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_RULE5_DIP_MASK_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE5_DIP_MASK_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[5].dip;
				mask = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[5].mask;
				goto dipMaskTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_RULE6_DIP_MASK_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE6_DIP_MASK_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[6].dip;
				mask = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[6].mask;
				goto dipMaskTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_RULE7_DIP_MASK_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE7_DIP_MASK_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[7].dip;
				mask = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[7].mask;
				goto dipMaskTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_RULE8_DIP_MASK_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE8_DIP_MASK_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[8].dip;
				mask = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[8].mask;
				goto dipMaskTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_RULE9_DIP_MASK_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE9_DIP_MASK_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[9].dip;
				mask = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[9].mask;
				goto dipMaskTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_RULE10_DIP_MASK_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE10_DIP_MASK_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[10].dip;
				mask = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[10].mask;
				goto dipMaskTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_RULE11_DIP_MASK_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE11_DIP_MASK_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[11].dip;
				mask = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[11].mask;
				goto dipMaskTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_RULE12_DIP_MASK_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE12_DIP_MASK_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[12].dip;
				mask = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[12].mask;
				goto dipMaskTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_RULE13_DIP_MASK_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE13_DIP_MASK_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[13].dip;
				mask = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[13].mask;
				goto dipMaskTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_RULE14_DIP_MASK_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE14_DIP_MASK_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[14].dip;
				mask = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[14].mask;
				goto dipMaskTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_RULE15_DIP_MASK_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE15_DIP_MASK_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[15].dip;
				mask = rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[15].mask;
				goto dipMaskTrapRule;

dipMaskTrapRule:
				bzero(&aclRule,sizeof(aclRule));
				bzero(&aclField,sizeof(aclField));
				bzero(&aclField2,sizeof(aclField2));

				aclRule.valid=ENABLED;
				aclRule.index=aclIdx;
				aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
				aclRule.templateIdx=1; //DIP

				//DIP[0:15]
				aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
				aclField.fieldUnion.pattern.fieldIdx = 1;//DIP[31:16]
				aclField.fieldUnion.data.value= ((dip&0xffff0000)>>16);
				aclField.fieldUnion.data.mask=((mask&0xffff0000)>>16);
				if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_RULEn_DIP_MASK_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}

				//DIP[16:31]
				aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
				aclField2.fieldUnion.pattern.fieldIdx = 0;//DIP[15:0]
				aclField2.fieldUnion.data.value= (dip&0xffff);
				aclField2.fieldUnion.data.mask=(mask&0xffff);
				if(rtk_acl_igrRuleField_add(&aclRule, &aclField2)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_RULEn_DIP_MASK_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}


				aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
				aclRule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;
#ifdef CONFIG_DUALBAND_CONCURRENT
				aclRule.act.enableAct[ACL_IGR_PRI_ACT] = ENABLE;
				aclRule.act.priAct.act= ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
				aclRule.act.priAct.aclPri = ((CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI-1)&0x7);
#endif

				if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_RULEn_DIP_MASK_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}
				//point to next ruleIdx
				aclIdx++;
				break;


				case RTK_RG_ACLANDCF_RESERVED_RULE0_DIPv6_MASK_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE0_DIPv6_MASK_TRAP @ acl[%d]",aclIdx);
					memcpy(&ipv6_addr,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[0].dipv6.ipv6_addr,IPV6_ADDR_LEN);
					memcpy(&ipv6_addr_mask,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[0].dipv6_mask.ipv6_addr,IPV6_ADDR_LEN);
					goto dipv6MaskTrapRule;

				case RTK_RG_ACLANDCF_RESERVED_RULE1_DIPv6_MASK_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE1_DIPv6_MASK_TRAP @ acl[%d]",aclIdx);
					memcpy(&ipv6_addr,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[1].dipv6.ipv6_addr,IPV6_ADDR_LEN);
					memcpy(&ipv6_addr_mask,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[1].dipv6_mask.ipv6_addr,IPV6_ADDR_LEN);
					goto dipv6MaskTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_RULE2_DIPv6_MASK_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE2_DIPv6_MASK_TRAP @ acl[%d]",aclIdx);
					memcpy(&ipv6_addr,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[2].dipv6.ipv6_addr,IPV6_ADDR_LEN);
					memcpy(&ipv6_addr_mask,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[2].dipv6_mask.ipv6_addr,IPV6_ADDR_LEN);
					goto dipv6MaskTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_RULE3_DIPv6_MASK_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE3_DIPv6_MASK_TRAP @ acl[%d]",aclIdx);
					memcpy(&ipv6_addr,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[3].dipv6.ipv6_addr,IPV6_ADDR_LEN);
					memcpy(&ipv6_addr_mask,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[3].dipv6_mask.ipv6_addr,IPV6_ADDR_LEN);
					goto dipv6MaskTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_RULE4_DIPv6_MASK_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE4_DIPv6_MASK_TRAP @ acl[%d]",aclIdx);
					memcpy(&ipv6_addr,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[4].dipv6.ipv6_addr,IPV6_ADDR_LEN);
					memcpy(&ipv6_addr_mask,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[4].dipv6_mask.ipv6_addr,IPV6_ADDR_LEN);
					goto dipv6MaskTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_RULE5_DIPv6_MASK_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE5_DIPv6_MASK_TRAP @ acl[%d]",aclIdx);
					memcpy(&ipv6_addr,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[5].dipv6.ipv6_addr,IPV6_ADDR_LEN);
					memcpy(&ipv6_addr_mask,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[5].dipv6_mask.ipv6_addr,IPV6_ADDR_LEN);
					goto dipv6MaskTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_RULE6_DIPv6_MASK_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE6_DIPv6_MASK_TRAP @ acl[%d]",aclIdx);
					memcpy(&ipv6_addr,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[6].dipv6.ipv6_addr,IPV6_ADDR_LEN);
					memcpy(&ipv6_addr_mask,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[6].dipv6_mask.ipv6_addr,IPV6_ADDR_LEN);
					goto dipv6MaskTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_RULE7_DIPv6_MASK_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE7_DIPv6_MASK_TRAP @ acl[%d]",aclIdx);
					memcpy(&ipv6_addr,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[7].dipv6.ipv6_addr,IPV6_ADDR_LEN);
					memcpy(&ipv6_addr_mask,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[7].dipv6_mask.ipv6_addr,IPV6_ADDR_LEN);
					goto dipv6MaskTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_RULE8_DIPv6_MASK_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE8_DIPv6_MASK_TRAP @ acl[%d]",aclIdx);
					memcpy(&ipv6_addr,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[8].dipv6.ipv6_addr,IPV6_ADDR_LEN);
					memcpy(&ipv6_addr_mask,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[8].dipv6_mask.ipv6_addr,IPV6_ADDR_LEN);
					goto dipv6MaskTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_RULE9_DIPv6_MASK_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE9_DIPv6_MASK_TRAP @ acl[%d]",aclIdx);
					memcpy(&ipv6_addr,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[9].dipv6.ipv6_addr,IPV6_ADDR_LEN);
					memcpy(&ipv6_addr_mask,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[9].dipv6_mask.ipv6_addr,IPV6_ADDR_LEN);
					goto dipv6MaskTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_RULE10_DIPv6_MASK_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE10_DIPv6_MASK_TRAP @ acl[%d]",aclIdx);
					memcpy(&ipv6_addr,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[10].dipv6.ipv6_addr,IPV6_ADDR_LEN);
					memcpy(&ipv6_addr_mask,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[10].dipv6_mask.ipv6_addr,IPV6_ADDR_LEN);
					goto dipv6MaskTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_RULE11_DIPv6_MASK_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE11_DIPv6_MASK_TRAP @ acl[%d]",aclIdx);
					memcpy(&ipv6_addr,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[11].dipv6.ipv6_addr,IPV6_ADDR_LEN);
					memcpy(&ipv6_addr_mask,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[11].dipv6_mask.ipv6_addr,IPV6_ADDR_LEN);
					goto dipv6MaskTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_RULE12_DIPv6_MASK_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE12_DIPv6_MASK_TRAP @ acl[%d]",aclIdx);
					memcpy(&ipv6_addr,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[12].dipv6.ipv6_addr,IPV6_ADDR_LEN);
					memcpy(&ipv6_addr_mask,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[12].dipv6_mask.ipv6_addr,IPV6_ADDR_LEN);
					goto dipv6MaskTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_RULE13_DIPv6_MASK_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE13_DIPv6_MASK_TRAP @ acl[%d]",aclIdx);
					memcpy(&ipv6_addr,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[13].dipv6.ipv6_addr,IPV6_ADDR_LEN);
					memcpy(&ipv6_addr_mask,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[13].dipv6_mask.ipv6_addr,IPV6_ADDR_LEN);
					goto dipv6MaskTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_RULE14_DIPv6_MASK_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE14_DIPv6_MASK_TRAP @ acl[%d]",aclIdx);
					memcpy(&ipv6_addr,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[14].dipv6.ipv6_addr,IPV6_ADDR_LEN);
					memcpy(&ipv6_addr_mask,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[14].dipv6_mask.ipv6_addr,IPV6_ADDR_LEN);
					goto dipv6MaskTrapRule;
				case RTK_RG_ACLANDCF_RESERVED_RULE15_DIPv6_MASK_TRAP:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_RULE15_DIPv6_MASK_TRAP @ acl[%d]",aclIdx);
					memcpy(&ipv6_addr,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[15].dipv6.ipv6_addr,IPV6_ADDR_LEN);
					memcpy(&ipv6_addr_mask,rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[15].dipv6_mask.ipv6_addr,IPV6_ADDR_LEN);
					goto dipv6MaskTrapRule;

	dipv6MaskTrapRule:

				bzero(&aclRule,sizeof(aclRule));
				for(i=0;i<8;i++)
					bzero(&rg_db.systemGlobal.aclField_ar[i],sizeof(rtk_acl_field_t));


				aclRule.valid=ENABLED;
				aclRule.index=aclIdx;
				aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
				aclRule.templateIdx=6; //DIPv6


				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP0%8].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP0%8].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP0%8;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP0%8].fieldUnion.data.value = (ipv6_addr.ipv6_addr[14]<<8)|(ipv6_addr.ipv6_addr[15]);//v6DIP[15:0]
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP0%8].fieldUnion.data.mask = (ipv6_addr_mask.ipv6_addr[14]<<8)|(ipv6_addr_mask.ipv6_addr[15]);

				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP1%8].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP1%8].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP1%8;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP1%8].fieldUnion.data.value = (ipv6_addr.ipv6_addr[12]<<8)|(ipv6_addr.ipv6_addr[13]);//v6DIP[31:16]
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP1%8].fieldUnion.data.mask = (ipv6_addr_mask.ipv6_addr[12]<<8)|(ipv6_addr_mask.ipv6_addr[13]);

				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP2%8].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP2%8].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP2%8;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP2%8].fieldUnion.data.value = (ipv6_addr.ipv6_addr[10]<<8)|(ipv6_addr.ipv6_addr[11]);//v6DIP[47:32]
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP2%8].fieldUnion.data.mask = (ipv6_addr_mask.ipv6_addr[10]<<8)|(ipv6_addr_mask.ipv6_addr[11]);

				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP3%8].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP3%8].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP3%8;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP3%8].fieldUnion.data.value = (ipv6_addr.ipv6_addr[8]<<8)|(ipv6_addr.ipv6_addr[9]);//v6DIP[63:48]
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP3%8].fieldUnion.data.mask = (ipv6_addr_mask.ipv6_addr[8]<<8)|(ipv6_addr_mask.ipv6_addr[9]);

				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP4%8].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP4%8].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP4%8;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP4%8].fieldUnion.data.value = (ipv6_addr.ipv6_addr[6]<<8)|(ipv6_addr.ipv6_addr[7]);//v6DIP[79:64]
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP4%8].fieldUnion.data.mask = (ipv6_addr_mask.ipv6_addr[6]<<8)|(ipv6_addr_mask.ipv6_addr[7]);

				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP5%8].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP5%8].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP5%8;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP5%8].fieldUnion.data.value = (ipv6_addr.ipv6_addr[4]<<8)|(ipv6_addr.ipv6_addr[5]);//v6DIP[95:80]
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP5%8].fieldUnion.data.mask = (ipv6_addr_mask.ipv6_addr[4]<<8)|(ipv6_addr_mask.ipv6_addr[5]);

				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP6%8].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP6%8].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP6%8;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP6%8].fieldUnion.data.value = (ipv6_addr.ipv6_addr[2]<<8)|(ipv6_addr.ipv6_addr[3]);//v6DIP[111:96]
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP6%8].fieldUnion.data.mask = (ipv6_addr_mask.ipv6_addr[2]<<8)|(ipv6_addr_mask.ipv6_addr[3]);

				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP7%8].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP7%8].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP7%8;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP7%8].fieldUnion.data.value = (ipv6_addr.ipv6_addr[0]<<8)|(ipv6_addr.ipv6_addr[1]);//v6DIP[127:112]
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP7%8].fieldUnion.data.mask = (ipv6_addr_mask.ipv6_addr[0]<<8)|(ipv6_addr_mask.ipv6_addr[1]);

				for(i=0;i<8;i++){
					if(rtk_acl_igrRuleField_add(&aclRule, &rg_db.systemGlobal.aclField_ar[i])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_RULEn_DIPv6_MASK_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
				}

				aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
				aclRule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;
				if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_RULEn_DIPv6_MASK_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}
				//point to next ruleIdx
				aclIdx++;
				break;


			case RTK_RG_ACLANDCF_RESERVED_PORT0_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PORT0_TRAP @ acl[%d]",aclIdx);
				mask = (1<<RTK_RG_MAC_PORT0);
				goto portTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PORT1_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PORT1_TRAP @ acl[%d]",aclIdx);
				mask = (1<<RTK_RG_MAC_PORT1);
				goto portTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_PORT2_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PORT2_TRAP @ acl[%d]",aclIdx);
				mask = (1<<RTK_RG_MAC_PORT_PON);
				goto portTrapRule;

portTrapRule:
				bzero(&aclRule,sizeof(aclRule));
				aclRule.valid=ENABLED;
				aclRule.index=aclIdx;
				aclRule.activePorts.bits[0]=mask;
				aclRule.templateIdx=0;

				aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
				aclRule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;
#ifdef CONFIG_DUALBAND_CONCURRENT
				aclRule.act.enableAct[ACL_IGR_PRI_ACT] = ENABLE;
				aclRule.act.priAct.act= ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
				aclRule.act.priAct.aclPri = ((CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI-1)&0x7);
#endif
				if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PORTn_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}
				//point to next ruleIdx
				aclIdx++;
				break;




			case RTK_RG_ACLANDCF_RESERVED_VLANBIND0_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND0_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[0].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[0].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND1_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND1_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[1].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[1].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND2_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND2_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[2].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[2].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND3_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND3_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[3].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[3].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND4_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND4_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[4].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[4].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND5_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND5_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[5].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[5].vid;
				goto vlanBindTrapRule;

			case RTK_RG_ACLANDCF_RESERVED_VLANBIND6_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND6_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[6].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[6].vid;
				goto vlanBindTrapRule;

			case RTK_RG_ACLANDCF_RESERVED_VLANBIND7_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND7_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[7].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[7].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND8_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND8_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[8].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[8].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND9_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND9_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[9].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[9].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND10_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND10_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[10].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[10].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND11_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND11_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[11].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[11].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND12_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND12_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[12].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[12].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND13_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND13_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[13].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[13].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND14_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND14_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[14].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[14].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND15_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND15_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[15].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[15].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND16_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND16_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[16].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[16].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND17_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND17_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[17].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[17].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND18_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND18_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[18].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[18].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND19_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND19_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[19].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[19].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND20_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND20_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[20].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[20].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND21_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND21_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[21].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[21].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND22_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND22_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[22].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[22].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND23_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND23_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[23].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[23].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND24_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND24_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[24].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[24].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND25_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND25_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[25].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[25].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND26_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND26_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[26].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[26].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND27_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND27_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[27].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[27].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND28_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND28_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[28].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[28].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND29_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND29_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[29].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[29].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND30_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND30_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[30].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[30].vid;
				goto vlanBindTrapRule;
			case RTK_RG_ACLANDCF_RESERVED_VLANBIND31_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_VLANBIND31_TRAP @ acl[%d]",aclIdx);
				mask = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[31].portmask;
				vid = rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[31].vid;
				goto vlanBindTrapRule;

vlanBindTrapRule:
				bzero(&aclRule,sizeof(aclRule));
				bzero(&aclField,sizeof(aclField));

				aclRule.valid=ENABLED;
				aclRule.index=aclIdx;
				aclRule.activePorts.bits[0]=mask;
				aclRule.templateIdx=0; //VID

				aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
				aclField.fieldUnion.pattern.fieldIdx = 6;/*template[0],field[6]*/
				aclField.fieldUnion.data.value = vid;
				aclField.fieldUnion.data.mask = 0xfff;
				if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_VLANBINDn_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}

				aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
				aclRule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;
#ifdef CONFIG_DUALBAND_CONCURRENT
				aclRule.act.enableAct[ACL_IGR_PRI_ACT] = ENABLE;
				aclRule.act.priAct.act= ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
				aclRule.act.priAct.aclPri = ((CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI-1)&0x7);
#endif
				if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_VLANBINDn_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}
				//point to next ruleIdx
				aclIdx++;
				break;

			case RTK_RG_ACLANDCF_RESERVED_NETIF0_GATEWAY_IPv4_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF0_GATEWAY_IPv4_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.interfaceInfo[0].storedInfo.wan_intf.static_info.ip_addr;
				goto addGipv4TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF1_GATEWAY_IPv4_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF1_GATEWAY_IPv4_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.interfaceInfo[1].storedInfo.wan_intf.static_info.ip_addr;
				goto addGipv4TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF2_GATEWAY_IPv4_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF2_GATEWAY_IPv4_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.interfaceInfo[2].storedInfo.wan_intf.static_info.ip_addr;
				goto addGipv4TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF3_GATEWAY_IPv4_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF3_GATEWAY_IPv4_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.interfaceInfo[3].storedInfo.wan_intf.static_info.ip_addr;
				goto addGipv4TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF4_GATEWAY_IPv4_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF4_GATEWAY_IPv4_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.interfaceInfo[4].storedInfo.wan_intf.static_info.ip_addr;
				goto addGipv4TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF5_GATEWAY_IPv4_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF5_GATEWAY_IPv4_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.interfaceInfo[5].storedInfo.wan_intf.static_info.ip_addr;
				goto addGipv4TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF6_GATEWAY_IPv4_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF6_GATEWAY_IPv4_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.interfaceInfo[6].storedInfo.wan_intf.static_info.ip_addr;
				goto addGipv4TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF7_GATEWAY_IPv4_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF7_GATEWAY_IPv4_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.interfaceInfo[7].storedInfo.wan_intf.static_info.ip_addr;
				goto addGipv4TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF8_GATEWAY_IPv4_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF8_GATEWAY_IPv4_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.interfaceInfo[8].storedInfo.wan_intf.static_info.ip_addr;
				goto addGipv4TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF9_GATEWAY_IPv4_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF9_GATEWAY_IPv4_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.interfaceInfo[9].storedInfo.wan_intf.static_info.ip_addr;
				goto addGipv4TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF10_GATEWAY_IPv4_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF10_GATEWAY_IPv4_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.interfaceInfo[10].storedInfo.wan_intf.static_info.ip_addr;
				goto addGipv4TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF11_GATEWAY_IPv4_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF11_GATEWAY_IPv4_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.interfaceInfo[11].storedInfo.wan_intf.static_info.ip_addr;
				goto addGipv4TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF12_GATEWAY_IPv4_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF12_GATEWAY_IPv4_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.interfaceInfo[12].storedInfo.wan_intf.static_info.ip_addr;
				goto addGipv4TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF13_GATEWAY_IPv4_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF13_GATEWAY_IPv4_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.interfaceInfo[13].storedInfo.wan_intf.static_info.ip_addr;
				goto addGipv4TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF14_GATEWAY_IPv4_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF14_GATEWAY_IPv4_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.interfaceInfo[14].storedInfo.wan_intf.static_info.ip_addr;
				goto addGipv4TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF15_GATEWAY_IPv4_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF15_GATEWAY_IPv4_TRAP @ acl[%d]",aclIdx);
				dip = rg_db.systemGlobal.interfaceInfo[15].storedInfo.wan_intf.static_info.ip_addr;
				goto addGipv4TrapRule;

addGipv4TrapRule:
				bzero(&aclRule,sizeof(aclRule));
				bzero(&aclField,sizeof(aclField));
				bzero(&aclField2,sizeof(aclField2));

				aclRule.valid=ENABLED;
				aclRule.index=aclIdx;
				aclRule.activePorts.bits[0]= RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
				aclRule.templateIdx=1;//DIPv4

				//DIP[0:15]
				aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
				aclField.fieldUnion.pattern.fieldIdx = 1;//DIP[31:16]
				aclField.fieldUnion.data.value= ((dip&0xffff0000)>>16);
				aclField.fieldUnion.data.mask=0xffff;
				if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_NETIFn_GATEWAY_IPv4_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}

				//DIP[16:31]
				aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
				aclField2.fieldUnion.pattern.fieldIdx = 0;//DIP[15:0]
				aclField2.fieldUnion.data.value= (dip&0xffff);
				aclField2.fieldUnion.data.mask=0xffff;
				if(rtk_acl_igrRuleField_add(&aclRule, &aclField2)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_NETIFn_GATEWAY_IPv4_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}

				aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
				aclRule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;
				if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_NETIFn_GATEWAY_IPv4_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}
				//point to next ruleIdx
				aclIdx++;
				break;


			case RTK_RG_ACLANDCF_RESERVED_NETIF0_GATEWAY_IPv6_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF0_GATEWAY_IPv6_TRAP @ acl[%d]",aclIdx);
				memcpy(&ipv6_addr.ipv6_addr[0], &rg_db.systemGlobal.interfaceInfo[0].storedInfo.wan_intf.static_info.ipv6_addr.ipv6_addr[0],IPV6_ADDR_LEN);
				goto addGipv6TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF1_GATEWAY_IPv6_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF1_GATEWAY_IPv6_TRAP @ acl[%d]",aclIdx);
				memcpy(&ipv6_addr.ipv6_addr[0], &rg_db.systemGlobal.interfaceInfo[1].storedInfo.wan_intf.static_info.ipv6_addr.ipv6_addr[0],IPV6_ADDR_LEN);
				goto addGipv6TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF2_GATEWAY_IPv6_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF2_GATEWAY_IPv6_TRAP @ acl[%d]",aclIdx);
				memcpy(&ipv6_addr.ipv6_addr[0], &rg_db.systemGlobal.interfaceInfo[2].storedInfo.wan_intf.static_info.ipv6_addr.ipv6_addr[0],IPV6_ADDR_LEN);
				goto addGipv6TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF3_GATEWAY_IPv6_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF3_GATEWAY_IPv6_TRAP @ acl[%d]",aclIdx);
				memcpy(&ipv6_addr.ipv6_addr[0], &rg_db.systemGlobal.interfaceInfo[3].storedInfo.wan_intf.static_info.ipv6_addr.ipv6_addr[0],IPV6_ADDR_LEN);
				goto addGipv6TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF4_GATEWAY_IPv6_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF4_GATEWAY_IPv6_TRAP @ acl[%d]",aclIdx);
				memcpy(&ipv6_addr.ipv6_addr[0], &rg_db.systemGlobal.interfaceInfo[4].storedInfo.wan_intf.static_info.ipv6_addr.ipv6_addr[0],IPV6_ADDR_LEN);
				goto addGipv6TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF5_GATEWAY_IPv6_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF5_GATEWAY_IPv6_TRAP @ acl[%d]",aclIdx);
				memcpy(&ipv6_addr.ipv6_addr[0], &rg_db.systemGlobal.interfaceInfo[5].storedInfo.wan_intf.static_info.ipv6_addr.ipv6_addr[0],IPV6_ADDR_LEN);
				goto addGipv6TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF6_GATEWAY_IPv6_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF6_GATEWAY_IPv6_TRAP @ acl[%d]",aclIdx);
				memcpy(&ipv6_addr.ipv6_addr[0], &rg_db.systemGlobal.interfaceInfo[6].storedInfo.wan_intf.static_info.ipv6_addr.ipv6_addr[0],IPV6_ADDR_LEN);
				goto addGipv6TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF7_GATEWAY_IPv6_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF7_GATEWAY_IPv6_TRAP @ acl[%d]",aclIdx);
				memcpy(&ipv6_addr.ipv6_addr[0], &rg_db.systemGlobal.interfaceInfo[7].storedInfo.wan_intf.static_info.ipv6_addr.ipv6_addr[0],IPV6_ADDR_LEN);
				goto addGipv6TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF8_GATEWAY_IPv6_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF8_GATEWAY_IPv6_TRAP @ acl[%d]",aclIdx);
				memcpy(&ipv6_addr.ipv6_addr[0], &rg_db.systemGlobal.interfaceInfo[8].storedInfo.wan_intf.static_info.ipv6_addr.ipv6_addr[0],IPV6_ADDR_LEN);
				goto addGipv6TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF9_GATEWAY_IPv6_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF9_GATEWAY_IPv6_TRAP @ acl[%d]",aclIdx);
				memcpy(&ipv6_addr.ipv6_addr[0], &rg_db.systemGlobal.interfaceInfo[9].storedInfo.wan_intf.static_info.ipv6_addr.ipv6_addr[0],IPV6_ADDR_LEN);
				goto addGipv6TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF10_GATEWAY_IPv6_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF10_GATEWAY_IPv6_TRAP @ acl[%d]",aclIdx);
				memcpy(&ipv6_addr.ipv6_addr[0], &rg_db.systemGlobal.interfaceInfo[10].storedInfo.wan_intf.static_info.ipv6_addr.ipv6_addr[0],IPV6_ADDR_LEN);
				goto addGipv6TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF11_GATEWAY_IPv6_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF11_GATEWAY_IPv6_TRAP @ acl[%d]",aclIdx);
				memcpy(&ipv6_addr.ipv6_addr[0], &rg_db.systemGlobal.interfaceInfo[11].storedInfo.wan_intf.static_info.ipv6_addr.ipv6_addr[0],IPV6_ADDR_LEN);
				goto addGipv6TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF12_GATEWAY_IPv6_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF12_GATEWAY_IPv6_TRAP @ acl[%d]",aclIdx);
				memcpy(&ipv6_addr.ipv6_addr[0], &rg_db.systemGlobal.interfaceInfo[12].storedInfo.wan_intf.static_info.ipv6_addr.ipv6_addr[0],IPV6_ADDR_LEN);
				goto addGipv6TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF13_GATEWAY_IPv6_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF13_GATEWAY_IPv6_TRAP @ acl[%d]",aclIdx);
				memcpy(&ipv6_addr.ipv6_addr[0], &rg_db.systemGlobal.interfaceInfo[13].storedInfo.wan_intf.static_info.ipv6_addr.ipv6_addr[0],IPV6_ADDR_LEN);
				goto addGipv6TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF14_GATEWAY_IPv6_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF14_GATEWAY_IPv6_TRAP @ acl[%d]",aclIdx);
				memcpy(&ipv6_addr.ipv6_addr[0], &rg_db.systemGlobal.interfaceInfo[14].storedInfo.wan_intf.static_info.ipv6_addr.ipv6_addr[0],IPV6_ADDR_LEN);
				goto addGipv6TrapRule;
			case RTK_RG_ACLANDCF_RESERVED_NETIF15_GATEWAY_IPv6_TRAP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_NETIF15_GATEWAY_IPv6_TRAP @ acl[%d]",aclIdx);
				memcpy(&ipv6_addr.ipv6_addr[0], &rg_db.systemGlobal.interfaceInfo[15].storedInfo.wan_intf.static_info.ipv6_addr.ipv6_addr[0],IPV6_ADDR_LEN);
				goto addGipv6TrapRule;

addGipv6TrapRule:

				bzero(&aclRule,sizeof(aclRule));
				for(i=0;i<8;i++)
					bzero(&rg_db.systemGlobal.aclField_ar[i],sizeof(rtk_acl_field_t));


				aclRule.valid=ENABLED;
				aclRule.index=aclIdx;
				aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
				aclRule.templateIdx=6; //DIPv6


				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP0%8].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP0%8].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP0%8;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP0%8].fieldUnion.data.value = (ipv6_addr.ipv6_addr[14]<<8)|(ipv6_addr.ipv6_addr[15]);//v6DIP[15:0]
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP0%8].fieldUnion.data.mask = 0xffff;

				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP1%8].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP1%8].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP1%8;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP1%8].fieldUnion.data.value = (ipv6_addr.ipv6_addr[12]<<8)|(ipv6_addr.ipv6_addr[13]);//v6DIP[31:16]
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP1%8].fieldUnion.data.mask = 0xffff;

				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP2%8].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP2%8].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP2%8;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP2%8].fieldUnion.data.value = (ipv6_addr.ipv6_addr[10]<<8)|(ipv6_addr.ipv6_addr[11]);//v6DIP[47:32]
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP2%8].fieldUnion.data.mask = 0xffff;

				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP3%8].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP3%8].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP3%8;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP3%8].fieldUnion.data.value = (ipv6_addr.ipv6_addr[8]<<8)|(ipv6_addr.ipv6_addr[9]);//v6DIP[63:48]
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP3%8].fieldUnion.data.mask = 0xffff;

				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP4%8].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP4%8].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP4%8;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP4%8].fieldUnion.data.value = (ipv6_addr.ipv6_addr[6]<<8)|(ipv6_addr.ipv6_addr[7]);//v6DIP[79:64]
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP4%8].fieldUnion.data.mask = 0xffff;

				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP5%8].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP5%8].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP5%8;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP5%8].fieldUnion.data.value = (ipv6_addr.ipv6_addr[4]<<8)|(ipv6_addr.ipv6_addr[5]);//v6DIP[95:80]
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP5%8].fieldUnion.data.mask = 0xffff;

				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP6%8].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP6%8].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP6%8;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP6%8].fieldUnion.data.value = (ipv6_addr.ipv6_addr[2]<<8)|(ipv6_addr.ipv6_addr[3]);//v6DIP[111:96]
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP6%8].fieldUnion.data.mask = 0xffff;

				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP7%8].fieldType = ACL_FIELD_PATTERN_MATCH;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP7%8].fieldUnion.pattern.fieldIdx = TEMPLATE_IPv6DIP7%8;
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP7%8].fieldUnion.data.value = (ipv6_addr.ipv6_addr[0]<<8)|(ipv6_addr.ipv6_addr[1]);//v6DIP[127:112]
				rg_db.systemGlobal.aclField_ar[TEMPLATE_IPv6DIP7%8].fieldUnion.data.mask = 0xffff;

				for(i=0;i<8;i++){
					if(rtk_acl_igrRuleField_add(&aclRule, &rg_db.systemGlobal.aclField_ar[i])){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_NETIFn_GATEWAY_IPv6_TRAP failed!!!");
						addRuleFailedFlag=1;
						break;
					}
				}

				aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
				aclRule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;
				if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
					ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_NETIFn_GATEWAY_IPv6_TRAP failed!!!");
					addRuleFailedFlag=1;
					break;
				}
				//point to next ruleIdx
				aclIdx++;
				break;

			case RTK_RG_ACLANDCF_RESERVED_IGMP_MLD_DROP:
				ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_IGMP_MLD_DROP @ acl[%d]",aclIdx);

				bzero(&aclRule,sizeof(aclRule));
				bzero(&aclField,sizeof(aclField));

				aclRule.valid=ENABLED;
				aclRule.index=aclIdx;
				aclRule.activePorts.bits[0]=rg_db.systemGlobal.aclAndCfReservedRule.igmp_mld_drop_portmask.portmask;
				aclRule.templateIdx=4; //FS[15] : tagif

				aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
				aclField.fieldUnion.pattern.fieldIdx = 7;/*template[4],field[7]*/
				aclField.fieldUnion.data.value = (1<<6); //IGMP/MLD tagif
				aclField.fieldUnion.data.mask = (1<<6);
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

	//reflash user ACL
	ASSERT_EQ(_rtk_rg_aclSWEntry_and_asic_rearrange(),RT_ERR_RG_OK);


	return (RT_ERR_RG_OK);
}


static int _rtk_rg_aclAndCfReservedRuleTailReflash(void)
{
	uint32 type;
	uint32 aclIdx=(MAX_ACL_ENTRY_SIZE-1), cfIdx=(rg_db.systemGlobal.cf_pattern0_size-1);
	uint32 vlan,port,prioirty;
	//rule used varibles
	rtk_acl_ingress_entry_t aclRule;
	rtk_acl_field_t aclField;

#if 1 //PPPOE passthrought
	rtk_classify_cfg_t cfRule;
	rtk_classify_field_t cfField;
	rtk_classify_ds_act_t dsAct;
	rtk_mac_t gmac;
	rtk_acl_field_t aclField_dmac0,aclField_dmac1,aclField_dmac2;
#endif

	int addRuleFailedFlag=0;
	uint32 i;

	//reflash th rules
	for(i=rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary;i<MAX_ACL_ENTRY_SIZE;i++){
		assert_ok(rtk_acl_igrRuleEntry_del(i));
	}
	for(i=rg_db.systemGlobal.aclAndCfReservedRule.cfUpperBoundary;i<rg_db.systemGlobal.cf_pattern0_size;i++){
		assert_ok(rtk_classify_cfgEntry_del(i));
	}


	for(type=RTK_RG_ACLANDCF_RESERVED_HEAD_END;type<RTK_RG_ACLANDCF_RESERVED_TAIL_END;type++){
		if(rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[type]==ENABLED){
			switch(type){

				case RTK_RG_ACLANDCF_RESERVED_LAN_TO_INTERNET_BRIDGEWAN_VLAN_TRANSLATE:
					{
						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_LAN_TO_INTERNET_BRIDGEWAN_VLAN_TRANSLATE @ acl[%d]",aclIdx);
						//translate ipv6 multicast to assigned vid
						bzero(&aclRule,sizeof(aclRule));

						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;


						aclRule.activePorts.bits[0]=rg_db.systemGlobal.aclAndCfReservedRule.lan_pask_and_wan_vid.portmask;
						aclRule.templateIdx=0;
						aclRule.act.enableAct[ACL_IGR_CVLAN_ACT]=ENABLED;
						aclRule.act.cvlanAct.act=ACL_IGR_CVLAN_IGR_CVLAN_ACT;
						aclRule.act.cvlanAct.cvid=rg_db.systemGlobal.aclAndCfReservedRule.lan_pask_and_wan_vid.wan_vid;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_LAN_TO_INTERNET_BRIDGEWAN_VLAN_TRANSLATE failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx--;
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT:
					{

					/*new default policy, drop unknownDA UDP multicast, trap else multicast*/
						ACL_RSV("add RTK_RG_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT @ acl[%d] & acl[%d]",aclIdx,aclIdx-1);

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
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule))
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
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx--;
					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV6_PASSTHROUGHT:
					{

						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV6_PASSTHROUGHT @ acl[%d]",aclIdx);
						//translate ipv6 multicast to assigned vid
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));

						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;

						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 7;//v6DIP[127:112]: template[6],field[7]
						aclField.fieldUnion.data.value=0xff00; //v6DIP start with 1111 1111 ...
						aclField.fieldUnion.data.mask=0xff00;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV6_PASSTHROUGHT failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.activePorts.bits[0]=(1<<RTK_RG_PORT_PON);//PON only
						aclRule.careTag.tags[ACL_CARE_TAG_IPV6].value=ENABLED;
						aclRule.careTag.tags[ACL_CARE_TAG_IPV6].mask=0xffff;
						aclRule.templateIdx=6; /*use DIP[127:112]: template[6],field[7]*/
						aclRule.act.enableAct[ACL_IGR_CVLAN_ACT]=ENABLED;
						aclRule.act.cvlanAct.act=ACL_IGR_CVLAN_IGR_CVLAN_ACT;
						aclRule.act.cvlanAct.cvid=rg_db.systemGlobal.initParam.fwdVLAN_CPU;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV6_PASSTHROUGHT failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx--;
					}
					break;



#if 1	//PPPOE passthrought
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864:
					{
						if(_rtk_rg_vlanExistInRgCheck(rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtDefaulTrapRulePatchPara.remarkVid)!=SUCCESS){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864 failed!!! Assigned Vid[%d] not exist",rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtDefaulTrapRulePatchPara.remarkVid);
							addRuleFailedFlag=1;
							break;
						}

						ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864 @ acl[%d~%d] & cf[%d]",aclIdx,(aclIdx-2),cfIdx);

						//for remarking ethertype 0x8864 to bridged vid.(downstream)
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
							//just filter pppoe packet, do not care sessionID
						aclRule.activePorts.bits[0]=rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtDefaulTrapRulePatchPara.wanPmsk;		//filter packets from WAN port only
						aclRule.careTag.tags[ACL_CARE_TAG_PPPOE].value=ENABLED;
						aclRule.careTag.tags[ACL_CARE_TAG_PPPOE].mask=0xffff;

						//because per PPPoEWAN no need to check sessionID, so it doesnt have to concern vlan tag makes FS[15] parsing sessionID problem.
						//aclRule.careTag.tags[ACL_CARE_TAG_CTAG].value=DISABLE;
						//aclRule.careTag.tags[ACL_CARE_TAG_CTAG].mask=0xffff;
						aclRule.templateIdx=3; /*use fieldSelector[15]: template[3],field[7]*/
						aclRule.act.enableAct[ACL_IGR_CVLAN_ACT]=ENABLED;
						aclRule.act.cvlanAct.act=ACL_IGR_CVLAN_IGR_CVLAN_ACT;
						aclRule.act.cvlanAct.cvid=rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtDefaulTrapRulePatchPara.remarkVid;
						aclRule.act.enableAct[ACL_IGR_INTR_ACT] = ENABLE; //latch to cf
						aclRule.act.aclLatch = ENABLED;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864 failed!!!");
							addRuleFailedFlag=1;
							break;
						}



						//add cf[RESERVED_CF_PPPOE_PASSTHROUGH_DOWNSTREAM_DMAC2CVID_ENTRY] for force all downstream packet DMAC=>CVID action (downstream)
						bzero(&cfRule,sizeof(cfRule));
						bzero(&dsAct,sizeof(dsAct));
						dsAct.cAct=CLASSIFY_DS_CACT_ADD_CTAG_8100;
						dsAct.cVidAct=CLASSIFY_DS_VID_ACT_FROM_LUT;
						dsAct.cPriAct=CLASSIFY_DS_PRI_ACT_FROM_INTERNAL;
						cfRule.index=cfIdx;
						cfRule.direction=CLASSIFY_DIRECTION_DS;
						cfRule.valid=ENABLED;
						cfRule.act.dsAct=dsAct;
						//latch by ACL[RESERVED_ACL_PPPoE_SESSIONID_REMARKING_ENTRY_BASE_FOR_ETHTER_8864]
						cfField.fieldType = CLASSIFY_FIELD_ACL_HIT;
						cfField.classify_pattern.fieldData.value = ((1<<7) | aclIdx); //(1<<7) is the valid bit
						cfField.classify_pattern.fieldData.mask=0xff;
						if(rtk_classify_field_add(&cfRule, &cfField))
						{
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864 failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						if(RTK_CLASSIFY_CFGENTRY_ADD(&cfRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864 failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						//point to next ruleIdx
						aclIdx--;
						cfIdx--;


						//add acl[59] for trap all ethertype=0x8864 to fwdEngine (upstream)
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
							//just filter pppoeTag packet
						aclRule.activePorts.bits[0]=rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtDefaulTrapRulePatchPara.lanPmsk&(~(0x1<<RTK_RG_MAC_PORT_CPU)); 	//filter packets from LAN port only, exclude CPU port
						aclRule.careTag.tags[ACL_CARE_TAG_PPPOE].value=ENABLED;
						aclRule.careTag.tags[ACL_CARE_TAG_PPPOE].mask=0xffff;
						aclRule.templateIdx=3; //unuse any field.
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;//ACL_IGR_FORWARD_COPY_ACT;
						//aclRule.act.forwardAct.portMask.bits[0]=wan_pmsk;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864 failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						//point to next ruleIdx
						aclIdx--;

						//add acl[60] for trap all ethertype=0x8863 to fwdEngine (upstream and downstream)
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
							//Ethertype 0x8863
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 7;//template[0],field[7]
						aclField.fieldUnion.data.value=0x8863;
						aclField.fieldUnion.data.mask=0xffff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864 failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK&(~(0x1<<RTK_RG_MAC_PORT_CPU));		//from CPU port should not trap again
						aclRule.templateIdx=0; /*use Ethertype: template[0],field[7]*/
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act = ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
							ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864 failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						//point to next ruleIdx
						aclIdx--;

					}
					break;

				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF15ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF15ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[15].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[15].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF14ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF14ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[14].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[14].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF13ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF13ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[13].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[13].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF12ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF12ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[12].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[12].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF11ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF11ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[11].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[11].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF10ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF10ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[10].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[10].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF9ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF9ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[9].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[9].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF8ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF8ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[8].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[8].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;

				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF7ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF7ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[7].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[7].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF6ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF6ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[6].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[6].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF5ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF5ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[5].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[5].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF4ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF4ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[4].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[4].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF3ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF3ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[3].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[3].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF2ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF2ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[2].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[2].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF1ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF1ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[1].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[1].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;
				case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF0ISPPPOEWAN:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF0ISPPPOEWAN @ acl[%d]",aclIdx);
					memcpy(&gmac.octet[0],&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[0].gmac.octet[0],ETHER_ADDR_LEN);
					//memcpy(&sessionId,&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[0].sessionId,2); //sizeof(uint16)=2
					goto setPPPoEwanRule;

setPPPoEwanRule:
					/*start to set ACL rule*/
					bzero(&aclRule,sizeof(aclRule));
					bzero(&aclField_dmac0,sizeof(aclField_dmac0));
					bzero(&aclField_dmac1,sizeof(aclField_dmac1));
					bzero(&aclField_dmac2,sizeof(aclField_dmac2));
					aclRule.valid=ENABLED;
					aclRule.index = aclIdx;
					aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK;
					aclRule.careTag.tags[ACL_CARE_TAG_PPPOE].value=ENABLED; //filter 0x8864
					aclRule.careTag.tags[ACL_CARE_TAG_PPPOE].mask=0xffff;
					aclRule.templateIdx=0;

					//setup gmac
					aclField_dmac2.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_dmac2.fieldUnion.pattern.fieldIdx = 0; //DA[15:0]: template[0] field[0]
					aclField_dmac2.fieldUnion.data.value = (gmac.octet[4]<<8) | (gmac.octet[5]);
					aclField_dmac2.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac2)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTFxISPPPOEWAN failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					aclField_dmac1.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_dmac1.fieldUnion.pattern.fieldIdx = 1; //DA[31:16]: template[0] field[1]
					aclField_dmac1.fieldUnion.data.value = (gmac.octet[2]<<8) | (gmac.octet[3]);
					aclField_dmac1.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac1)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTFxISPPPOEWAN failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					aclField_dmac0.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField_dmac0.fieldUnion.pattern.fieldIdx = 2; //DA[47:32]: template[0] field[2]
					aclField_dmac0.fieldUnion.data.value = (gmac.octet[0]<<8) | (gmac.octet[1]);
					aclField_dmac0.fieldUnion.data.mask = 0xffff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac0)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTFxISPPPOEWAN failed!!!");
						addRuleFailedFlag=1;
						break;
					}


					aclRule.act.enableAct[ACL_IGR_CVLAN_ACT]=ENABLED;
					aclRule.act.cvlanAct.act = ACL_IGR_CVLAN_MIB_ACT;
					aclRule.act.cvlanAct.mib = 32; //assigned a unused counter

					if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTFxISPPPOEWAN failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					//point to next ruleIdx
					aclIdx--;

					break;

#endif

				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT0_WITHOUT_FILTER:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT0_WITHOUT_FILTER @ acl[%d]",aclIdx);
					port = rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[RTK_RG_PORT0].service_port;
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[RTK_RG_PORT0].assigned_vid;
					goto portBasedAclRule;
				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT1_WITHOUT_FILTER:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT1_WITHOUT_FILTER @ acl[%d]",aclIdx);
					port = rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[RTK_RG_PORT1].service_port;
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[RTK_RG_PORT1].assigned_vid;
					goto portBasedAclRule;


				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT4_WITHOUT_FILTER:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT4_WITHOUT_FILTER @ acl[%d]",aclIdx);
					port = rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[RTK_RG_PORT_PON].service_port;
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[RTK_RG_PORT_PON].assigned_vid;
					goto portBasedAclRule;


				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT6_WITHOUT_FILTER:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT6_WITHOUT_FILTER @ acl[%d]",aclIdx);
					port = rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[RTK_RG_PORT_CPU].service_port;
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[RTK_RG_PORT_CPU].assigned_vid;
					goto portBasedAclRule;

portBasedAclRule:
					bzero(&aclRule,sizeof(aclRule));
					bzero(&aclField,sizeof(aclField));

					aclRule.valid=ENABLED;
					aclRule.index=aclIdx;
					aclRule.activePorts.bits[0]=(1<<port);
					aclRule.templateIdx=0;
					aclRule.careTag.tags[ACL_CARE_TAG_CTAG].value=DISABLED;
					aclRule.careTag.tags[ACL_CARE_TAG_CTAG].mask=0xffff;

					//translate CVID
					aclRule.act.enableAct[ACL_IGR_CVLAN_ACT]=ENABLED;
					aclRule.act.cvlanAct.act=ACL_IGR_CVLAN_IGR_CVLAN_ACT;
					aclRule.act.cvlanAct.cvid=vlan;
					if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORTn_WITHOUT_FILTER failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					//point to next ruleIdx
					aclIdx--;

					break;



				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF0:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF0 @ acl[%d]",aclIdx);
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[0].ingress_vlan;
					prioirty = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[0].assigned_priority;
					goto assignVlanBasedPriorityForInterface;
				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF1:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF1 @ acl[%d]",aclIdx);
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[1].ingress_vlan;
					prioirty = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[1].assigned_priority;
					goto assignVlanBasedPriorityForInterface;

				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF2:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF2 @ acl[%d]",aclIdx);
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[2].ingress_vlan;
					prioirty = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[2].assigned_priority;
					goto assignVlanBasedPriorityForInterface;

				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF3:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF3 @ acl[%d]",aclIdx);
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[3].ingress_vlan;
					prioirty = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[3].assigned_priority;
					goto assignVlanBasedPriorityForInterface;

				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF4:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF4 @ acl[%d]",aclIdx);
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[4].ingress_vlan;
					prioirty = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[4].assigned_priority;
					goto assignVlanBasedPriorityForInterface;

				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF5:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF5 @ acl[%d]",aclIdx);
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[5].ingress_vlan;
					prioirty = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[5].assigned_priority;
					goto assignVlanBasedPriorityForInterface;

				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF6:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF6 @ acl[%d]",aclIdx);
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[6].ingress_vlan;
					prioirty = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[6].assigned_priority;
					goto assignVlanBasedPriorityForInterface;

				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF7:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF7 @ acl[%d]",aclIdx);
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[7].ingress_vlan;
					prioirty = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[7].assigned_priority;
					goto assignVlanBasedPriorityForInterface;

				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF8:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF8 @ acl[%d]",aclIdx);
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[8].ingress_vlan;
					prioirty = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[8].assigned_priority;
					goto assignVlanBasedPriorityForInterface;

				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF9:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF9 @ acl[%d]",aclIdx);
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[9].ingress_vlan;
					prioirty = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[9].assigned_priority;
					goto assignVlanBasedPriorityForInterface;

				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF10:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF10 @ acl[%d]",aclIdx);
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[10].ingress_vlan;
					prioirty = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[10].assigned_priority;
					goto assignVlanBasedPriorityForInterface;

				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF11:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF11 @ acl[%d]",aclIdx);
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[11].ingress_vlan;
					prioirty = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[11].assigned_priority;
					goto assignVlanBasedPriorityForInterface;

				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF12:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF12 @ acl[%d]",aclIdx);
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[12].ingress_vlan;
					prioirty = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[12].assigned_priority;
					goto assignVlanBasedPriorityForInterface;

				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF13:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF13 @ acl[%d]",aclIdx);
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[13].ingress_vlan;
					prioirty = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[13].assigned_priority;
					goto assignVlanBasedPriorityForInterface;

				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF14:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF14 @ acl[%d]",aclIdx);
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[14].ingress_vlan;
					prioirty = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[14].assigned_priority;
					goto assignVlanBasedPriorityForInterface;

				case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF15:
					ACL_RSV("do RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF15 @ acl[%d]",aclIdx);
					vlan = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[15].ingress_vlan;
					prioirty = rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[15].assigned_priority;
					goto assignVlanBasedPriorityForInterface;

assignVlanBasedPriorityForInterface:
					bzero(&aclRule,sizeof(aclRule));
					bzero(&aclField,sizeof(aclField));

					aclRule.valid=ENABLED;
					aclRule.index=aclIdx;
					aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK;
					aclRule.templateIdx=0;
					aclRule.careTag.tags[ACL_CARE_TAG_CTAG].value=ENABLED;
					aclRule.careTag.tags[ACL_CARE_TAG_CTAG].mask=0xffff;

					//compare CVID
					aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
					aclField.fieldUnion.pattern.fieldIdx = 6;//template[0],field[6]
					aclField.fieldUnion.data.value=vlan; //just care VID
					aclField.fieldUnion.data.mask=0x0fff;
					if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTFn failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					//assigned priority
					aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
					aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
					aclRule.act.priAct.aclPri=prioirty;
					if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule)){
						ACL_RSV("add reserved ACL&CF RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTFn failed!!!");
						addRuleFailedFlag=1;
						break;
					}

					//point to next ruleIdx
					aclIdx--;
					break;

				default:
					break;
			}
		}
	}

	rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary=aclIdx;
	rg_db.systemGlobal.aclAndCfReservedRule.cfUpperBoundary=cfIdx;

	if(addRuleFailedFlag==1)
		return (RT_ERR_RG_FAILED);

	return (RT_ERR_RG_OK);

}

void _rtk_rg_acl_for_multicast_temp_protection_add(void)
{
#if defined(HW_ACL_REARRANGE_PROTECT_VERSION) && (HW_ACL_REARRANGE_PROTECT_VERSION==2)
	rtk_acl_ingress_entry_t aclRule;
	rtk_acl_field_t aclField;

	/***set IPv4 Multicast Permit rule***/
	if(rg_db.systemGlobal.rgInit) {
		bzero(&aclRule,sizeof(aclRule));
		bzero(&aclField,sizeof(aclField));

		//setup MC IPv4 DIP 224.0.0.0 to 239.255.255.255
		aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField.fieldUnion.pattern.fieldIdx = 1; //dipv4[31:16]: template[1] field[1]
		aclField.fieldUnion.data.value = 0xe000;
		aclField.fieldUnion.data.mask = 0xf000;
		if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
		{
			WARNING("setup ACL for MC IPv4 permit failed!!!");
		}

		aclRule.valid=ENABLED;
		aclRule.index=HW_ACL_REARRANGE_PROTECT_V4MC_PERMIT;
		aclRule.templateIdx = 1;//dipv4 in template[1]
		aclRule.activePorts.bits[0]=RTK_RG_ALL_MAC_PORTMASK_WITHOUT_CPU;
		aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
		aclRule.act.forwardAct.act= ACL_IGR_FORWARD_IGR_MIRROR_ACT;
		aclRule.act.forwardAct.portMask.bits[0]=0x0;

		aclRule.act.enableAct[ACL_IGR_CVLAN_ACT] = ENABLE;
		aclRule.act.cvlanAct.act = ACL_IGR_CVLAN_IGR_CVLAN_ACT;
		if(rg_db.systemGlobal.acl_rearrange_force_mc_ingress_cvid==-1){
			aclRule.act.cvlanAct.cvid=rg_db.systemGlobal.initParam.fwdVLAN_CPU;
		}else{
			aclRule.act.cvlanAct.cvid=rg_db.systemGlobal.acl_rearrange_force_mc_ingress_cvid;
		}

		if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule))
		{
			WARNING("setup ACL for MC IPv4 permit failed!!!");
		}
	}

	/***set IPv6 Multicast Permit rule***/
	if(rg_db.systemGlobal.rgInit) {
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
		aclRule.act.forwardAct.act= ACL_IGR_FORWARD_IGR_MIRROR_ACT;
		aclRule.act.forwardAct.portMask.bits[0]=0x0;

		aclRule.act.enableAct[ACL_IGR_CVLAN_ACT] = ENABLE;
		aclRule.act.cvlanAct.act = ACL_IGR_CVLAN_IGR_CVLAN_ACT;
		if(rg_db.systemGlobal.acl_rearrange_force_mc_ingress_cvid==-1){
			aclRule.act.cvlanAct.cvid=rg_db.systemGlobal.initParam.fwdVLAN_CPU;
		}else{
			aclRule.act.cvlanAct.cvid=rg_db.systemGlobal.acl_rearrange_force_mc_ingress_cvid;
		}

		if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule))
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
		if(rg_db.systemGlobal.aclRearrangeProtectWithAllPermit)
		{
			ACL_CTRL("add ACL PROTECT for ALL permit");
			aclRule.act.forwardAct.act= ACL_IGR_FORWARD_IGR_MIRROR_ACT;
			aclRule.act.forwardAct.portMask.bits[0]=0x0;
		}
		else
		{
			aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
		}
		if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule))
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
		aclRule.act.forwardAct.act= ACL_IGR_FORWARD_IGR_MIRROR_ACT;
		aclRule.act.forwardAct.portMask.bits[0]=0x0;

		aclRule.act.enableAct[ACL_IGR_CVLAN_ACT] = ENABLE;
		aclRule.act.cvlanAct.act = ACL_IGR_CVLAN_IGR_CVLAN_ACT;
		if(rg_db.systemGlobal.acl_rearrange_force_mc_ingress_cvid==-1){
			aclRule.act.cvlanAct.cvid=rg_db.systemGlobal.initParam.fwdVLAN_CPU;
		}else{
			aclRule.act.cvlanAct.cvid=rg_db.systemGlobal.acl_rearrange_force_mc_ingress_cvid;
		}

		if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule))
		{
			WARNING("setup ACL for MC temporary permit failed!!!");
		}
	}
	DEBUG("ACL Rearrange Protection(MC permit) Start @ acl[0]");
	ACL_RSV("ACL Rearrange Protection(MC permit) Start @ acl[0]");
#endif
}

void _rtk_rg_acl_for_multicast_temp_protection_del(void)
{
#if defined(HW_ACL_REARRANGE_PROTECT_VERSION) && (HW_ACL_REARRANGE_PROTECT_VERSION==2)
	int i;

	for(i=HW_ACL_REARRANGE_PROTECT_V4MC_PERMIT; i<HW_ACL_REARRANGE_PROTECT_RSV_HEAD; i++){
		if(rtk_acl_igrRuleEntry_del(i))
		{
			WARNING("setup ACL for MC temporary trap failed!!!");
		}
	}
	DEBUG("ACL Rearrange Protection(MC permit, Others trap) Stop @ acl[%d] & acl[%d] & acl[%d]", HW_ACL_REARRANGE_PROTECT_V4MC_PERMIT, HW_ACL_REARRANGE_PROTECT_V6MC_PERMIT, HW_ACL_REARRANGE_PROTECT_OTHERS_TRAP);
	ACL_RSV("ACL Rearrange Protection(MC permit, Others trap) Stop @ acl[%d] & acl[%d] & acl[%d]", HW_ACL_REARRANGE_PROTECT_V4MC_PERMIT, HW_ACL_REARRANGE_PROTECT_V6MC_PERMIT, HW_ACL_REARRANGE_PROTECT_OTHERS_TRAP);
#else
	//add multicast trap and addsign VID=1 at ACL[0] when rearrange period.

	rtk_acl_ingress_entry_t aclRule;
	bzero(&aclRule,sizeof(aclRule));
	//remove reserved acl[0] for permit multicast
	{
		if(rtk_acl_igrRuleEntry_del(0))
		{
			WARNING("setup ACL for MC temporary trap failed!!!");
		}
	}
	DEBUG("ACL Rearrange Protection(MC permit) Stop @ acl[0]");
	ACL_RSV("ACL Rearrange Protection(MC permit) Stop @ acl[0]");
#endif
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



int _rtk_rg_aclAndCfReservedRuleReflash(void)
{

	_rtk_rg_acl_for_multicast_temp_protection_add();

	_rtk_rg_aclAndCfReservedRuleHeadReflash();
	_rtk_rg_aclAndCfReservedRuleTailReflash();

	_rtk_rg_acl_for_multicast_temp_protection_del();

	return RT_ERR_RG_OK;
}

static int _rtk_rg_aclAndCfReservedRuleAddCheck(uint32 aclRsvSize, uint32 cfRsvSize, rtk_rg_aclAndCf_reserved_type_t rsvType)
{
	uint32 i;
	rtk_acl_ingress_entry_t aclRule;
	rtk_classify_cfg_t cfRule;;

	//check rest empty acl rules in enough
	for(i=rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary;i>(rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary-aclRsvSize);i--){
		aclRule.index=i;
		assert_ok(rtk_acl_igrRuleEntry_get(&aclRule));
		if(aclRule.valid!=DISABLED){
			WARNING("ACL rest rules for reserved[%d] is not enough! i=%d aclUpperBoundary=%d aclRsvSize=%d",rsvType,i,rg_db.systemGlobal.aclAndCfReservedRule.aclUpperBoundary,aclRsvSize);
			return (RT_ERR_RG_FAILED);
		}
	}

	//check rest empty cf rules in enough
	for(i=rg_db.systemGlobal.aclAndCfReservedRule.cfUpperBoundary;i>(rg_db.systemGlobal.aclAndCfReservedRule.cfUpperBoundary-cfRsvSize);i--){
		cfRule.index=i;
		assert_ok(rtk_classify_cfgEntry_get(&cfRule));
		if(cfRule.valid!=DISABLED){
			WARNING("CF rest rules for reserved[%d] is not enough!",rsvType);
			return (RT_ERR_RG_FAILED);
		}
	}

	return (RT_ERR_RG_OK);

}


int _rtk_rg_aclAndCfReservedRuleAdd(rtk_rg_aclAndCf_reserved_type_t rsvType, void *parameter)
{
	int i;
	ACL_RSV("#####Reserved ACL reflash!(add reserved ACL rsvType=%d)#####",rsvType);

	//init used parameter in fwdEngine dataPath, all rules will be readd again
	for(i=0;i<MAX_NETIF_SW_TABLE_SIZE;i++){
		rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[i]=FAIL;
	}

	switch(rsvType){

		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_RMA_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_WIFI2_IPC_ROUTING_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_UNICAST_TRAP: //reserved one more for MC temporary permit while ACL rearrange
		case RTK_RG_ACLANDCF_RESERVED_SYN_PACKET_TRAP:
		//HEAD_END
		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_VID_TRANSLATE_FOR_IPV6_PASSTHROUGHT:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType),RT_ERR_RG_OK);
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;		

		case RTK_RG_ACLANDCF_RESERVED_STPBLOCKING:
		case RTK_RG_ACLANDCF_RESERVED_BROADCAST_TRAP:	//reserved one more for MC temporary permit while ACL rearrange
		case RTK_RG_ACLANDCF_RESERVED_ALL_TRAP:	//reserved one more for MC temporary permit while ACL rearrange
		//HEAD_END
		case RTK_RG_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0,rsvType),RT_ERR_RG_OK);
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;


		case RTK_RG_ACLANDCF_RESERVED_ACL_SKIP_HW_REARRANGE_PERMIT_AND_TRAP_RESERV:
		//HEAD_END
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(3,0,rsvType),RT_ERR_RG_OK);
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		//HEAD RSV

		case RTK_RG_ACLANDCF_RESERVED_INTF0_DHCP_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF1_DHCP_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF2_DHCP_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF3_DHCP_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF4_DHCP_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF5_DHCP_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF6_DHCP_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF7_DHCP_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF8_DHCP_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF9_DHCP_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF10_DHCP_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF11_DHCP_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF12_DHCP_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF13_DHCP_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF14_DHCP_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF15_DHCP_TRAP:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0,rsvType),RT_ERR_RG_OK);
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_dhcp_trap_para[rsvType-RTK_RG_ACLANDCF_RESERVED_INTF0_DHCP_TRAP],(rtk_rg_aclAndCf_reserved_intf_dhcp_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_dhcp_trap_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_ACK_PACKET_ASSIGN_PRIORITY:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0,rsvType),RT_ERR_RG_OK);
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.ack_packet_assign_priority,(rtk_rg_aclAndCf_reserved_ack_packet_assign_priority_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_ack_packet_assign_priority_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType),RT_ERR_RG_OK);
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_lcp_assign_prioity,(rtk_rg_aclAndCf_reserved_pppoe_lcp_assign_priority_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoe_lcp_assign_priority_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType),RT_ERR_RG_OK);
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.l2tp_ctrl_lcp_assign_prioity,(rtk_rg_aclAndCf_reserved_l2tp_control_lcp_trap_and_assign_priority_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_l2tp_control_lcp_trap_and_assign_priority_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_PORTBINDING_FAIL_TRAP:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(3,0,rsvType),RT_ERR_RG_OK);
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;
		case RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF0_NONE_IP_VLAN_TRANSLATE:
		case RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF1_NONE_IP_VLAN_TRANSLATE:
		case RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF2_NONE_IP_VLAN_TRANSLATE:
		case RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF3_NONE_IP_VLAN_TRANSLATE:
		case RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF4_NONE_IP_VLAN_TRANSLATE:
		case RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF5_NONE_IP_VLAN_TRANSLATE:
		case RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF6_NONE_IP_VLAN_TRANSLATE:
		case RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF7_NONE_IP_VLAN_TRANSLATE:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType),RT_ERR_RG_OK);
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.none_ip_bind_translate_vid[rsvType-RTK_RG_ACLANDCF_RESERVED_PORTBINDING_INTF0_NONE_IP_VLAN_TRANSLATE],(rtk_rg_aclAndCf_reserved_none_ip_bind_translate_vid_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_none_ip_bind_translate_vid_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF1_DSLITE_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF2_DSLITE_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF3_DSLITE_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF4_DSLITE_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF5_DSLITE_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF6_DSLITE_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF7_DSLITE_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF8_DSLITE_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF9_DSLITE_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF10_DSLITE_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF11_DSLITE_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF12_DSLITE_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF13_DSLITE_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF14_DSLITE_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_INTF15_DSLITE_TRAP:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0,rsvType),RT_ERR_RG_OK);
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_dslite_trap_para[rsvType-RTK_RG_ACLANDCF_RESERVED_INTF0_DSLITE_TRAP],(rtk_rg_aclAndCf_reserved_intf_dslite_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_dslite_trap_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF0_MC_ROUTING_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF1_MC_ROUTING_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF2_MC_ROUTING_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF3_MC_ROUTING_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF4_MC_ROUTING_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF5_MC_ROUTING_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF6_MC_ROUTING_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF7_MC_ROUTING_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF8_MC_ROUTING_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF9_MC_ROUTING_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF10_MC_ROUTING_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF11_MC_ROUTING_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF12_MC_ROUTING_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF13_MC_ROUTING_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF14_MC_ROUTING_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF15_MC_ROUTING_TRAP:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0,rsvType),RT_ERR_RG_OK);
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoe_intf_multicast_routing_trap_para[rsvType-RTK_RG_ACLANDCF_RESERVED_PPPOE_INTF0_MC_ROUTING_TRAP],(rtk_rg_aclAndCf_reserved_pppoe_intf_multicast_routing_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoe_intf_multicast_routing_trap_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;
			
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF0_LINK_LOCAL_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF1_LINK_LOCAL_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF2_LINK_LOCAL_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF3_LINK_LOCAL_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF4_LINK_LOCAL_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF5_LINK_LOCAL_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF6_LINK_LOCAL_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF7_LINK_LOCAL_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF8_LINK_LOCAL_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF9_LINK_LOCAL_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF10_LINK_LOCAL_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF11_LINK_LOCAL_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF12_LINK_LOCAL_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF13_LINK_LOCAL_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF14_LINK_LOCAL_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_IPV6_INTF15_LINK_LOCAL_TRAP:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(2,0,rsvType),RT_ERR_RG_OK);
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.intf_link_local_trap_para[rsvType-RTK_RG_ACLANDCF_RESERVED_IPV6_INTF0_LINK_LOCAL_TRAP],(rtk_rg_aclAndCf_reserved_intf_linkLocal_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_intf_linkLocal_trap_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;
		
		case RTK_RG_ACLANDCF_RESERVED_RULE0_DIP_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE1_DIP_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE2_DIP_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE3_DIP_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE4_DIP_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE5_DIP_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE6_DIP_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE7_DIP_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE8_DIP_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE9_DIP_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE10_DIP_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE11_DIP_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE12_DIP_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE13_DIP_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE14_DIP_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE15_DIP_MASK_TRAP:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType),RT_ERR_RG_OK);
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.dip_mask_trap[rsvType-RTK_RG_ACLANDCF_RESERVED_RULE0_DIP_MASK_TRAP],(rtk_rg_aclAndCf_reserved_dip_mask_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_dip_mask_trap_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;
		
		case RTK_RG_ACLANDCF_RESERVED_RULE0_DIPv6_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE1_DIPv6_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE2_DIPv6_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE3_DIPv6_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE4_DIPv6_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE5_DIPv6_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE6_DIPv6_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE7_DIPv6_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE8_DIPv6_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE9_DIPv6_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE10_DIPv6_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE11_DIPv6_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE12_DIPv6_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE13_DIPv6_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE14_DIPv6_MASK_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_RULE15_DIPv6_MASK_TRAP:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType),RT_ERR_RG_OK);
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.dipv6_mask_trap[rsvType-RTK_RG_ACLANDCF_RESERVED_RULE0_DIPv6_MASK_TRAP],(rtk_rg_aclAndCf_reserved_dipv6_mask_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_dipv6_mask_trap_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;
		
		case RTK_RG_ACLANDCF_RESERVED_PORT0_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_PORT1_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_PORT2_TRAP:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType),RT_ERR_RG_OK);
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;
		
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND0_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND1_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND2_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND3_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND4_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND5_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND6_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND7_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND8_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND9_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND10_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND11_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND12_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND13_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND14_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND15_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND16_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND17_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND18_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND19_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND20_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND21_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND22_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND23_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND24_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND25_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND26_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND27_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND28_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND29_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND30_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_VLANBIND31_TRAP:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType),RT_ERR_RG_OK);
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.vlan_bind_trap[rsvType-RTK_RG_ACLANDCF_RESERVED_VLANBIND0_TRAP],(rtk_rg_aclAndCf_reserved_vlan_bind_trap_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_vlan_bind_trap_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;
		
		case RTK_RG_ACLANDCF_RESERVED_NETIF0_GATEWAY_IPv4_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF1_GATEWAY_IPv4_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF2_GATEWAY_IPv4_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF3_GATEWAY_IPv4_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF4_GATEWAY_IPv4_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF5_GATEWAY_IPv4_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF6_GATEWAY_IPv4_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF7_GATEWAY_IPv4_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF8_GATEWAY_IPv4_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF9_GATEWAY_IPv4_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF10_GATEWAY_IPv4_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF11_GATEWAY_IPv4_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF12_GATEWAY_IPv4_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF13_GATEWAY_IPv4_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF14_GATEWAY_IPv4_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF15_GATEWAY_IPv4_TRAP:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType),RT_ERR_RG_OK);
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;
		
		case RTK_RG_ACLANDCF_RESERVED_NETIF0_GATEWAY_IPv6_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF1_GATEWAY_IPv6_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF2_GATEWAY_IPv6_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF3_GATEWAY_IPv6_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF4_GATEWAY_IPv6_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF5_GATEWAY_IPv6_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF6_GATEWAY_IPv6_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF7_GATEWAY_IPv6_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF8_GATEWAY_IPv6_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF9_GATEWAY_IPv6_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF10_GATEWAY_IPv6_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF11_GATEWAY_IPv6_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF12_GATEWAY_IPv6_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF13_GATEWAY_IPv6_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF14_GATEWAY_IPv6_TRAP:
		case RTK_RG_ACLANDCF_RESERVED_NETIF15_GATEWAY_IPv6_TRAP:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType),RT_ERR_RG_OK);
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;
		
		case RTK_RG_ACLANDCF_RESERVED_IGMP_MLD_DROP:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType),RT_ERR_RG_OK);
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.igmp_mld_drop_portmask,(rtk_rg_aclAndCf_reserved_igmp_mld_drop_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_igmp_mld_drop_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		//TAIL RSV

		case RTK_RG_ACLANDCF_RESERVED_LAN_TO_INTERNET_BRIDGEWAN_VLAN_TRANSLATE:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType),RT_ERR_RG_OK);
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.lan_pask_and_wan_vid,(rtk_rg_aclAndCf_reserved_lan_to_internet_bridgeWan_translate_vid_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_lan_to_internet_bridgeWan_translate_vid_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_TRAP_ALL8863_US8864_DMAC2CVID_DS8864:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(3,1,rsvType),RT_ERR_RG_OK);
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtDefaulTrapRulePatchPara,(rtk_rg_aclAndCf_reserved_pppoepassthroughtDefaulTrapRuletPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoepassthroughtDefaulTrapRuletPatch_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF15ISPPPOEWAN:
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF14ISPPPOEWAN:
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF13ISPPPOEWAN:
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF12ISPPPOEWAN:
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF11ISPPPOEWAN:
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF10ISPPPOEWAN:
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF9ISPPPOEWAN:
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF8ISPPPOEWAN:
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF7ISPPPOEWAN:
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF6ISPPPOEWAN:
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF5ISPPPOEWAN:
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF4ISPPPOEWAN:
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF3ISPPPOEWAN:
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF2ISPPPOEWAN:
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF1ISPPPOEWAN:
		case RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF0ISPPPOEWAN:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType),RT_ERR_RG_OK);		//rsv index larger = intf index smaller
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.pppoepassthroughtIntfIsPppoewanpara[RTK_RG_ACLANDCF_RESERVED_PPPoEPASSTHROUGHT_INTF0ISPPPOEWAN-rsvType],(rtk_rg_aclAndCf_reserved_pppoepassthroughtIntfIsPppoewanPatch_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_pppoepassthroughtIntfIsPppoewanPatch_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT0_WITHOUT_FILTER:
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT1_WITHOUT_FILTER:
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT2_WITHOUT_FILTER:
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT3_WITHOUT_FILTER:
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT4_WITHOUT_FILTER:
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT5_WITHOUT_FILTER:
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT6_WITHOUT_FILTER:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType),RT_ERR_RG_OK);
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.assignCvlanForServicePortWithoutFilterPara[rsvType-RTK_RG_ACLANDCF_RESERVED_ASSIGN_INGRESS_CVLAN_FOR_SERVICE_PORT0_WITHOUT_FILTER],(rtk_rg_aclAndCf_reserved_AssignCvlanForServicePortWithoutFilter_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_AssignCvlanForServicePortWithoutFilter_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF0:
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF1:
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF2:
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF3:
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF4:
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF5:
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF6:
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF7:
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF8:
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF9:
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF10:
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF11:
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF12:
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF13:
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF14:
		case RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF15:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddCheck(1,0,rsvType),RT_ERR_RG_OK);
			memcpy(&rg_db.systemGlobal.aclAndCfReservedRule.assignVlanBasedPriorityForInterfacePara[rsvType-RTK_RG_ACLANDCF_RESERVED_ASSIGN_VLAN_BASED_RRIORITY_FOR_INTF0],(rtk_rg_aclAndCf_reserved_AssignVlanBasedPriorityForInterface_t*)parameter,sizeof(rtk_rg_aclAndCf_reserved_AssignVlanBasedPriorityForInterface_t));
			rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_RG_ACLANDCF_RESERVED_TAIL_END:
			break;

		default:
			break;
	}

	_rtk_rg_aclAndCfReservedRuleReflash();

	ACL_RSV("RESERVED_ACL_BEFORE=%d, RESERVED_ACL_AFTER=%d",RESERVED_ACL_BEFORE,RESERVED_ACL_AFTER);


	return (RT_ERR_RG_OK);

}


int _rtk_rg_aclAndCfReservedRuleDel(rtk_rg_aclAndCf_reserved_type_t rsvType)
{

	ACL_RSV("#####Reserved ACL reflash!(del reserved ACL rsvType=%d)#####",rsvType);

	if(rsvType < RTK_RG_ACLANDCF_RESERVED_TAIL_END)
		rg_db.systemGlobal.aclAndCfReservedRule.reservedMask[rsvType]=DISABLED;

	_rtk_rg_aclAndCfReservedRuleReflash();

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
	bzero(&pPktHdr->aclDecision.ACL_DECISION_EGR_PART_CLEAR, sizeof(rtk_rg_aclHitAndAction_t)-((uint32)(&pPktHdr->aclDecision.ACL_DECISION_EGR_PART_CLEAR)-(uint32)(&pPktHdr->aclDecision)));

	return (RT_ERR_RG_OK);
}

int _rtk_rg_acl_egressIntfIdx_precheck(rtk_rg_pktHdr_t *pPktHdr)
{
	//this API is using packet and Lan, Wan, Binding information to guess egress_intf_index when ingress stage for ACL TRANSFROM type.

	/*
	    egress_intf_idx predecision:
	    1. check vlan-binding =>egress_intf_idx from bind->wantype->nexthop->intf
	    2. check port-binding =>egress_intf_idx from bind->wantype->nexthop->intf
	    3. check Interface route(DIP + DA_gmac) =>egress_intf_idx from route -> ARP/Nexthop -> intf
	    4. check default roue(DA_gmac) =>egress_intf_idx from route[default] -> Nexthop -> intf
	    5. check normal bridge => egress_intf_idx is zero
	*/

	int dipL3Idx=0,nexthopIdx=0,wanGroupIdx=0;;


	pPktHdr->egressIntfIdxPreTrans = FAIL;


	if((1<<pPktHdr->ingressPort)& rg_db.systemGlobal.wanPortMask.portmask){
		pPktHdr->egressIntfIdxPreTrans = FAIL;
		ACL("egress_intf_idx pre-check fail, downstream packet.");
		return SUCCESS;
	}



	// 1. check vlan-binding
	// 2. check port-binding
	if(rg_db.systemGlobal.initParam.macBasedTagDecision && (_rtk_rg_bindingRuleCheck(pPktHdr, &wanGroupIdx)==RG_FWDENGINE_RET_HIT_BINDING))
	{
		pPktHdr->egressIntfIdxPreTrans= rg_db.systemGlobal.wanIntfGroup[wanGroupIdx].index;
		ACL("egress_intf_idx as WAN[%d], pre-check by binding",pPktHdr->egressIntfIdxPreTrans);
		return SUCCESS;
	}

	//3. check Interface route(DIP + DA_gmac)
	//4. check default roue(DA_gmac)
	{

		if((pPktHdr->tagif&IPV4_TAGIF)&&pPktHdr->isGatewayPacket)
		{

			//check DIP
			dipL3Idx=_rtk_rg_l3lookup(pPktHdr->ipv4Dip);

			/* arp table decision */
			if(rg_db.l3[dipL3Idx].rtk_l3.process==L34_PROCESS_ARP)
			{
				pPktHdr->egressIntfIdxPreTrans=rg_db.l3[dipL3Idx].rtk_l3.netifIdx; //for Normal Route SMAC
				ACL("egress_intf_idx as WAN[%d], pre-check by ARP route",pPktHdr->egressIntfIdxPreTrans);
				return SUCCESS;
			}
			/* nexthop table decision */
			else if(rg_db.l3[dipL3Idx].rtk_l3.process==L34_PROCESS_NH)
			{
				nexthopIdx=rg_db.l3[dipL3Idx].rtk_l3.nhStart;
				pPktHdr->egressIntfIdxPreTrans=rg_db.nexthop[nexthopIdx].rtk_nexthop.ifIdx;
				ACL("egress_intf_idx as WAN[%d], pre-check by nexthop route",pPktHdr->egressIntfIdxPreTrans);
				return SUCCESS;
			}
			else if(rg_db.l3[dipL3Idx].rtk_l3.process==L34_PROCESS_CPU)
			{
				if(rg_db.systemGlobal.interfaceInfo[rg_db.l3[dipL3Idx].rtk_l3.netifIdx].valid &&
					rg_db.systemGlobal.interfaceInfo[rg_db.l3[dipL3Idx].rtk_l3.netifIdx].storedInfo.is_wan &&
					(rg_db.systemGlobal.interfaceInfo[rg_db.l3[dipL3Idx].rtk_l3.netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPTP ||
					rg_db.systemGlobal.interfaceInfo[rg_db.l3[dipL3Idx].rtk_l3.netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_L2TP ||
					rg_db.systemGlobal.interfaceInfo[rg_db.l3[dipL3Idx].rtk_l3.netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_DSLITE ||
					rg_db.systemGlobal.interfaceInfo[rg_db.l3[dipL3Idx].rtk_l3.netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE_DSLITE))
				{
					nexthopIdx=rg_db.l3[dipL3Idx].rtk_l3.nhStart; //for Normal Route DMAC
					pPktHdr->egressIntfIdxPreTrans=rg_db.nexthop[nexthopIdx].rtk_nexthop.ifIdx;
					ACL("egress_intf_idx as WAN[%d], pre-check by routing CPU proceess(could be PPTP/L2TP/DS-Lite/PPPoE_DS-lite Wan)",pPktHdr->egressIntfIdxPreTrans);
					return SUCCESS;
				}
				else
				{
					pPktHdr->egressIntfIdxPreTrans = FAIL;
					ACL("egress_intf_idx pre-check fail by IPv4 route type is CPU process.");
					return SUCCESS;
				}

			}
			else
			{
				pPktHdr->egressIntfIdxPreTrans = FAIL;
				ACL("egress_intf_idx pre-check fail by IPv4 route type not found.");
				return SUCCESS;
			}
		}
		else if((pPktHdr->tagif&IPV6_TAGIF)&&pPktHdr->isGatewayPacket)
		{

				dipL3Idx=_rtk_rg_v6L3lookup(pPktHdr->pIpv6Dip);
				if(dipL3Idx==-1)
				{
					pPktHdr->egressIntfIdxPreTrans = FAIL;
					ACL("egress_intf_idx pre-check by IPv6 route not found.");
					return SUCCESS;
				}

				//hit! check process column
				switch (rg_db.v6route[dipL3Idx].rtk_v6route.type)
				{

					rtk_ipv6Routing_entry_t *routeEntry;
					rtk_l34_nexthop_entry_t	*nexthopEntry;

					case L34_IPV6_ROUTE_TYPE_DROP:
					case L34_IPV6_ROUTE_TYPE_TRAP:
						pPktHdr->egressIntfIdxPreTrans = FAIL;
						ACL("egress_intf_idx pre-check fail by IPv6 route(TRAP or DROP).");
						return SUCCESS;

					case L34_IPV6_ROUTE_TYPE_GLOBAL:
						/* Read NextHop */
						routeEntry=&rg_db.v6route[dipL3Idx].rtk_v6route;
						/* Read NextHop (Routing) */
						nexthopIdx=routeEntry->nhOrIfidIdx;
						nexthopEntry=&rg_db.nexthop[nexthopIdx].rtk_nexthop;
						pPktHdr->egressIntfIdxPreTrans=nexthopEntry->ifIdx;
						ACL("egress_intf_idx as WAN[%d] pre-check by IPv6 global route.",pPktHdr->egressIntfIdxPreTrans);
						return SUCCESS;

					case L34_IPV6_ROUTE_TYPE_LOCAL:
						routeEntry=&rg_db.v6route[dipL3Idx].rtk_v6route;
						pPktHdr->egressIntfIdxPreTrans=routeEntry->nhOrIfidIdx&0x7;
						ACL("egress_intf_idx as WAN[%d] pre-check by IPv6 local route.",pPktHdr->egressIntfIdxPreTrans);
						return SUCCESS;

					default:
						pPktHdr->egressIntfIdxPreTrans = FAIL;
						ACL("egress_intf_idx pre-check fail by IPv6 route type not found.");
						return SUCCESS;
			}

		}
	}

	//5. check normal bridge
	if(pPktHdr->isGatewayPacket==0){
		pPktHdr->egressIntfIdxPreTrans = 0;
		ACL("egress_intf_idx as WAN[%d](bridge) pre-check by final.",pPktHdr->egressIntfIdxPreTrans);
		return SUCCESS;
	}


	//not found
	pPktHdr->egressIntfIdxPreTrans = FAIL;
	ACL("egress_intf_idx pre-check fail by IPv6 route type not found.");

	return SUCCESS;

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
		ACL("Check ingress part of ACLRule: (especial for ingress cvid action rules) ");
	}

	pPktHdr->aclPriority = -1; //initial acl priority
	pPktHdr->aclDecision.aclEgrHaveToCheckRuleIdx[hitRuleCounter]=-1; //empty from first rule until the end

	if(rg_db.systemGlobal.acl_SW_table_entry_size<=0){//no rule need to verify
		ACL("no ACLRule need to verify");
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
			ACL("Check ingress part of ACLRule[%d]: (especial for ingress cvid action rules)",acl_sw_idx);
		}

		handleValidRuleCounter++; //a valid rule is going to check

		if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_TRAP)//trap to fwdEngine ignor
			continue;

		if(pPktHdr->ingressPort==RTK_RG_PORT_PON) //downstream
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
				ACL("RG_ACL[%d] INGRESS_PORT UNHIT",acl_sw_idx);
				continue;
			}
		}


#if 1	//checked when fwdtype is 0, otherwise supported by CF wan_intf pattern
		if(pChkRule->acl_filter.filter_fields&INGRESS_INTF_BIT){
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
			{
				//FIXME:judge rule is same as HWNAT, however may be not enought for ever case.
				//check this intf exist
				if(rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].valid==IF_INVALID_ENTRY){
					ACL("RG_ACL[%d] INGRESS_INTF UNHIT ",acl_sw_idx);
					continue;
				}


				if(rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.is_wan==1){//wan intf
					if(rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_BRIDGE){//bridge wan
						//check vlan
						if(rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.egress_vlan_tag_on==1){//vlan should tagged
							if(pPktHdr->ctagVid!=rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.egress_vlan_id){
								ACL("RG_ACL[%d] INGRESS_INTF UNHIT ",acl_sw_idx);
								continue;
							}
						}else{//vlan should untagged
							if(pPktHdr->tagif & CVLAN_TAGIF){
								ACL("RG_ACL[%d] INGRESS_INTF UNHIT ",acl_sw_idx);
								continue;
							}
						}
					}else{//L34 Wan
						if(rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.egress_vlan_tag_on==1){//vlan should tagged
							//vid==gw_vid && da==gw_mac
							if(pPktHdr->ctagVid!=rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.egress_vlan_id){
								ACL("RG_ACL[%d] INGRESS_INTF UNHIT ",acl_sw_idx);
								continue;
							}
							if(rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[0]!=pPktHdr->pDmac[0] ||
								rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[1]!=pPktHdr->pDmac[1]||
								rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[2]!=pPktHdr->pDmac[2]||
								rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[3]!=pPktHdr->pDmac[3]||
								rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[4]!=pPktHdr->pDmac[4]||
								rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[5]!=pPktHdr->pDmac[5]
								){
									ACL("RG_ACL[%d] INGRESS_INTF UNHIT ",acl_sw_idx);
									continue;
							}
						}else{//vlan should untagged & da==gw_mac
							if(pPktHdr->tagif & CVLAN_TAGIF){
								ACL("RG_ACL[%d] INGRESS_INTF UNHIT ",acl_sw_idx);
								continue;
							}
							if(rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[0]!=pPktHdr->pDmac[0] ||
								rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[1]!=pPktHdr->pDmac[1]||
								rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[2]!=pPktHdr->pDmac[2]||
								rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[3]!=pPktHdr->pDmac[3]||
								rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[4]!=pPktHdr->pDmac[4]||
								rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.wan_intf.wan_intf_conf.gmac.octet[5]!=pPktHdr->pDmac[5]
								){
									ACL("RG_ACL[%d] INGRESS_INTF UNHIT ",acl_sw_idx);
									continue;
							}
						}

					}

				}else{//lan intf
					//check spa is in lan_port_mask
					port_check = FAIL;
					for(port=0;port<RTK_RG_MAC_PORT_MAX;port++){
						if(RG_INVALID_MAC_PORT(port)) continue;
						if(rg_db.systemGlobal.interfaceInfo[pChkRule->acl_filter.ingress_intf_idx].storedInfo.lan_intf.port_mask.portmask & (1<<port)){
							if(port==pPktHdr->ingressMacPort){
								port_check = SUCCESS;
							}
						}
					}
					if(port_check!=SUCCESS){
						ACL("RG_ACL[%d] INGRESS_INTF UNHIT ",acl_sw_idx);
						continue;
					}
				}
			}
		}
#endif

		if(pChkRule->acl_filter.filter_fields&INGRESS_STREAM_ID_BIT){
			if((pChkRule->acl_filter.ingress_stream_id&pChkRule->acl_filter.ingress_stream_id_mask)!=(pPktHdr->pRxDesc->rx_pon_stream_id&pChkRule->acl_filter.ingress_stream_id_mask)){
				ACL("RG_ACL[%d] INGRESS_STREAM_ID_BIT UNHIT ",acl_sw_idx);
				continue;
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV4_TAGIF_BIT){
			if(pChkRule->acl_filter.ingress_ipv4_tagif){//must have ip header
				if(pPktHdr->tagif&IPV4_TAGIF){
					//hit do nothing
					//ACL("RG_ACL[%d] INGRESS_IPV4_TAGIF_BIT must tag hit ",i);
				}else{
					ACL("RG_ACL[%d] INGRESS_IPV4_TAGIF_BIT UNHIT ",acl_sw_idx);
					continue;
				}
			}else{//must not have IP header
				if(pPktHdr->tagif&IPV4_TAGIF){
					ACL("RG_ACL[%d] INGRESS_IPV4_TAGIF_BIT UNHIT ",acl_sw_idx);
					continue;
				}else{
					//hit do nothing
					//ACL("RG_ACL[%d] INGRESS_IPV4_TAGIF_BIT must untag hit ",i);
				}
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_TAGIF_BIT){
			if(pChkRule->acl_filter.ingress_ipv6_tagif){//must have ip header
				if(pPktHdr->tagif&IPV6_TAGIF){
					//hit do nothing
					//ACL("RG_ACL[%d] INGRESS_IPV6_TAGIF_BIT must tag hit ",i);
				}else{
					ACL("RG_ACL[%d] INGRESS_IPV6_TAGIF_BIT UNHIT ",acl_sw_idx);
					continue;
				}
			}else{//must not have IP header
				if(pPktHdr->tagif&IPV6_TAGIF){
					ACL("RG_ACL[%d] INGRESS_IPV6_TAGIF_BIT UNHIT ",acl_sw_idx);
					continue;
				}else{
					//hit do nothing
					//ACL("RG_ACL[%d] INGRESS_IPV6_TAGIF_BIT must untag hit ",i);
				}
			}
		}


		if(pChkRule->acl_filter.filter_fields&INGRESS_STAGIF_BIT){
			if(pChkRule->acl_filter.ingress_stagIf){//must have stag
				if((pPktHdr->tagif&SVLAN_TAGIF)==0x0){
					ACL("RG_ACL[%d] INGRESS_STAGIF_BIT UNHIT ",acl_sw_idx);
					continue;
				}
			}else{//must not have stag
				if(pPktHdr->tagif&SVLAN_TAGIF){
					ACL("RG_ACL[%d] INGRESS_STAGIF_BIT UNHIT ",acl_sw_idx);
					continue;
				}
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_CTAGIF_BIT){
			if(pChkRule->acl_filter.ingress_ctagIf){//must have ctag
				if((pPktHdr->tagif&CVLAN_TAGIF)==0x0){
					ACL("RG_ACL[%d] INGRESS_CTAGIF_BIT UNHIT ",acl_sw_idx);
					continue;
				}
			}else{//must not have ctag
				if(pPktHdr->tagif&CVLAN_TAGIF){
					ACL("RG_ACL[%d] INGRESS_CTAGIF_BIT UNHIT ",acl_sw_idx);
					continue;
				}
			}
		}


		if(pChkRule->acl_filter.filter_fields&INGRESS_ETHERTYPE_BIT){
			if((pChkRule->acl_filter.ingress_ethertype & pChkRule->acl_filter.ingress_ethertype_mask)!=(pPktHdr->etherType & pChkRule->acl_filter.ingress_ethertype_mask)){
				ACL("RG_ACL[%d] INGRESS_ETHERTYPE UNHIT ",acl_sw_idx);
				continue;
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_STAG_PRI_BIT){
			if(pChkRule->acl_filter.ingress_stag_pri!=pPktHdr->stagPri){
				ACL("RG_ACL[%d] INGRESS_STAG_PRI UNHIT ",acl_sw_idx);
				continue;
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_STAG_VID_BIT){
			if(pChkRule->acl_filter.ingress_stag_vid!=pPktHdr->stagVid){
				ACL("RG_ACL[%d] INGRESS_STAG_VID UNHIT ",acl_sw_idx);
				continue;
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_STAG_DEI_BIT){
			if(pChkRule->acl_filter.ingress_stag_dei!=pPktHdr->stagDei){
				ACL("RG_ACL[%d] INGRESS_STAG_DEI UNHIT ",acl_sw_idx);
				continue;
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_CTAG_PRI_BIT){
			if(pChkRule->acl_filter.ingress_ctag_pri!=pPktHdr->ctagPri){
				ACL("RG_ACL[%d] INGRESS_CTAG_PRI UNHIT ",acl_sw_idx);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_CTAG_VID_BIT){
			if(pChkRule->acl_filter.ingress_ctag_vid!=pPktHdr->ctagVid){
				ACL("RG_ACL[%d] INGRESS_CTAG_VID UNHIT ",acl_sw_idx);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_CTAG_CFI_BIT){
			if(pChkRule->acl_filter.ingress_ctag_cfi!=pPktHdr->ctagCfi){
				ACL("RG_ACL[%d] INGRESS_CTAG_CFI UNHIT ",acl_sw_idx);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_SMAC_BIT){
			if((pChkRule->acl_filter.ingress_smac.octet[0] & pChkRule->acl_filter.ingress_smac_mask.octet[0])!=(pPktHdr->pSmac[0]& pChkRule->acl_filter.ingress_smac_mask.octet[0]) ||
				(pChkRule->acl_filter.ingress_smac.octet[1] & pChkRule->acl_filter.ingress_smac_mask.octet[1])!=(pPktHdr->pSmac[1]& pChkRule->acl_filter.ingress_smac_mask.octet[1])||
				(pChkRule->acl_filter.ingress_smac.octet[2] & pChkRule->acl_filter.ingress_smac_mask.octet[2])!=(pPktHdr->pSmac[2]& pChkRule->acl_filter.ingress_smac_mask.octet[2])||
				(pChkRule->acl_filter.ingress_smac.octet[3] & pChkRule->acl_filter.ingress_smac_mask.octet[3])!=(pPktHdr->pSmac[3]& pChkRule->acl_filter.ingress_smac_mask.octet[3])||
				(pChkRule->acl_filter.ingress_smac.octet[4] & pChkRule->acl_filter.ingress_smac_mask.octet[4])!=(pPktHdr->pSmac[4]& pChkRule->acl_filter.ingress_smac_mask.octet[4])||
				(pChkRule->acl_filter.ingress_smac.octet[5] & pChkRule->acl_filter.ingress_smac_mask.octet[5])!=(pPktHdr->pSmac[5]& pChkRule->acl_filter.ingress_smac_mask.octet[5])
				){
				//memDump(pPktHdr->pSmac,6,"pktHdr pSmac");
				//memDump(pChkRule->acl_filter.ingress_smac.octet,6,"acl Smac");
				ACL("RG_ACL[%d] INGRESS_SMAC UNHIT ",acl_sw_idx);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_DMAC_BIT){
			if((pChkRule->acl_filter.ingress_dmac.octet[0] & pChkRule->acl_filter.ingress_dmac_mask.octet[0])!=(pPktHdr->pDmac[0] & pChkRule->acl_filter.ingress_dmac_mask.octet[0]) ||
				(pChkRule->acl_filter.ingress_dmac.octet[1] & pChkRule->acl_filter.ingress_dmac_mask.octet[1])!=(pPktHdr->pDmac[1] & pChkRule->acl_filter.ingress_dmac_mask.octet[1])||
				(pChkRule->acl_filter.ingress_dmac.octet[2] & pChkRule->acl_filter.ingress_dmac_mask.octet[2])!=(pPktHdr->pDmac[2] & pChkRule->acl_filter.ingress_dmac_mask.octet[2])||
				(pChkRule->acl_filter.ingress_dmac.octet[3] & pChkRule->acl_filter.ingress_dmac_mask.octet[3])!=(pPktHdr->pDmac[3] & pChkRule->acl_filter.ingress_dmac_mask.octet[3])||
				(pChkRule->acl_filter.ingress_dmac.octet[4] & pChkRule->acl_filter.ingress_dmac_mask.octet[4])!=(pPktHdr->pDmac[4] & pChkRule->acl_filter.ingress_dmac_mask.octet[4])||
				(pChkRule->acl_filter.ingress_dmac.octet[5] & pChkRule->acl_filter.ingress_dmac_mask.octet[5])!=(pPktHdr->pDmac[5] & pChkRule->acl_filter.ingress_dmac_mask.octet[5])
				){
				//memDump(pPktHdr->pDmac,6,"pktHdr pDmac");
				//memDump(pChkRule->acl_filter.ingress_dmac.octet,6,"acl Dmac");
				ACL("RG_ACL[%d] INGRESS_DMAC UNHIT ",acl_sw_idx);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_DSCP_BIT){
			if(pPktHdr->pTos==NULL){//pkt without DSCP
				ACL("RG_ACL[%d] INGRESS_DSCP UNHIT ",acl_sw_idx);
				continue;
			}else{
				if((pPktHdr->tagif&IPV4_TAGIF)==0x0 /*must be ipv4*/)	{
					ACL("RG_ACL[%d] INGRESS_DSCP UNHIT");
					continue;
				}
				tos = *(pPktHdr->pTos);
				if(pChkRule->acl_filter.ingress_dscp!=(tos>>2)){
					ACL("RG_ACL[%d] INGRESS_DSCP UNHIT ",acl_sw_idx);
					continue;
				}
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_TOS_BIT){
			if(pPktHdr->pTos==NULL){//pkt without DSCP
				continue;
			}else{
				if((pPktHdr->tagif&IPV4_TAGIF)==0x0 /*must be ipv4*/)	{
					ACL("RG_ACL[%d] INGRESS_TOS UNHIT");
					continue;
				}
				tos = *(pPktHdr->pTos);
				if(pChkRule->acl_filter.ingress_tos!=tos){
					ACL("RG_ACL[%d] INGRESS_TOS UNHIT",acl_sw_idx);
					continue;
				}
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_DSCP_BIT){
			if(pPktHdr->pTos==NULL){//pkt without DSCP
				ACL("RG_ACL[%d] INGRESS_IPV6_DSCP_BIT UNHIT ",acl_sw_idx);
				continue;
			}else{
				if((pPktHdr->tagif&IPV6_TAGIF)==0x0 /*must be ipv6*/)	{
					ACL("RG_ACL[%d] INGRESS_IPV6_DSCP_BIT UNHIT");
					continue;
				}
				tos = (*(pPktHdr->pTos))<<4 & 0xf0;
				tos |= (*((pPktHdr->pTos)+1))>>4 & 0xf;
				if(pChkRule->acl_filter.ingress_ipv6_dscp!=(tos>>2)){
					ACL("RG_ACL[%d] INGRESS_IPV6_DSCP UNHIT ",acl_sw_idx);
					continue;
				}
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_TC_BIT){
			if(pPktHdr->pTos==NULL){//pkt without DSCP
				continue;
			}else{
				if((pPktHdr->tagif&IPV6_TAGIF)==0x0 /*must be ipv6*/)	{
					ACL("RG_ACL[%d] INGRESS_IPV6_TC UNHIT");
					continue;
				}

				tos = (*(pPktHdr->pTos))<<4 & 0xf0;
				tos |= (*((pPktHdr->pTos)+1))>>4 & 0xf;
				if(pChkRule->acl_filter.ingress_ipv6_tc!=tos){
					ACL("RG_ACL[%d] INGRESS_IPV6_TC UNHIT");
					continue;
				}
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_TCP_BIT){
			if( !(pPktHdr->tagif&TCP_TAGIF)){
				ACL("RG_ACL[%d] INGRESS_L4_TCP UNHIT ",acl_sw_idx);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_UDP_BIT){
			if( !(pPktHdr->tagif&UDP_TAGIF)){
				ACL("RG_ACL[%d] INGRESS_L4_UDP UNHIT ",acl_sw_idx);
				continue;
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_NONE_TCP_NONE_UDP_BIT){
			if( (pPktHdr->tagif&UDP_TAGIF)||(pPktHdr->tagif&TCP_TAGIF)){
				ACL("RG_ACL[%d] INGRESS_L4_NONE_TCP_NONE_UDP_BIT UNHIT ",acl_sw_idx);
				continue;
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_ICMP_BIT){
			if(!(pPktHdr->tagif&ICMP_TAGIF)){
				ACL("RG_ACL[%d] INGRESS_L4_ICMP UNHIT ",acl_sw_idx);
				continue;
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_ICMPV6_BIT){
			if(!(pPktHdr->tagif&ICMPV6_TAGIF)){
				ACL("RG_ACL[%d] INGRESS_L4_ICMPV6_BIT UNHIT ",acl_sw_idx);
				continue;
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_POROTCAL_VALUE_BIT){
			if((pPktHdr->ipProtocol)!=(pChkRule->acl_filter.ingress_l4_protocal)){
				ACL("RG_ACL[%d] INGRESS_L4_POROTCAL_VALUE_BIT UNHIT ",acl_sw_idx);
				continue;
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_SIP_RANGE_BIT){
			ipv6_upper_range_check = -1;
			ipv6_lowwer_range_check = -1;
			if(pPktHdr->pIpv6Sip != NULL){
				ipv6_upper_range_check = memcmp(&(pChkRule->acl_filter.ingress_src_ipv6_addr_end[0]),pPktHdr->pIpv6Sip,16);
				 ipv6_lowwer_range_check = memcmp(pPktHdr->pIpv6Sip,&(pChkRule->acl_filter.ingress_src_ipv6_addr_start[0]),16);
			}
			if(!(ipv6_upper_range_check>=0 && ipv6_lowwer_range_check>=0 )){
				ACL("RG_ACL[%d] INGRESS_IPV6_SIP_RANGE UNHIT ",acl_sw_idx);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_SIP_BIT){
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
						ACL("RG_ACL[%d] INGRESS_IPV6_SIP_BIT UNHIT ",acl_sw_idx);
						continue;
				}
			}else{
				ACL("RG_ACL[%d] INGRESS_IPV6_SIP_BIT UNHIT ",acl_sw_idx);
				continue;
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_DIP_BIT){
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
						ACL("RG_ACL[%d] INGRESS_IPV6_DIP_BIT UNHIT ",acl_sw_idx);
						continue;
				}


			}else{
				ACL("RG_ACL[%d] INGRESS_IPV6_DIP_BIT UNHIT ",acl_sw_idx);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_DIP_RANGE_BIT){
			ipv6_upper_range_check = -1;
			ipv6_lowwer_range_check = -1;
			if(pPktHdr->pIpv6Dip!=NULL){
				ipv6_upper_range_check = memcmp(&(pChkRule->acl_filter.ingress_dest_ipv6_addr_end[0]),pPktHdr->pIpv6Dip,16);
				ipv6_lowwer_range_check = memcmp(pPktHdr->pIpv6Dip,&(pChkRule->acl_filter.ingress_dest_ipv6_addr_start[0]),16);
			}
			if(!(ipv6_upper_range_check>=0 && ipv6_lowwer_range_check>=0 )){
				ACL("RG_ACL[%d] INGRESS_IPV6_DIP_RANGE UNHIT ",acl_sw_idx);
				continue;
			}

		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV4_SIP_RANGE_BIT){
			if( !((pChkRule->acl_filter.ingress_src_ipv4_addr_end >= pPktHdr->ipv4Sip ) &&
				(pChkRule->acl_filter.ingress_src_ipv4_addr_start<= pPktHdr->ipv4Sip ))){
				ACL("RG_ACL[%d] INGRESS_IPV4_SIP_RANGE UNHIT ",acl_sw_idx);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV4_DIP_RANGE_BIT){
			if( !((pChkRule->acl_filter.ingress_dest_ipv4_addr_end >= pPktHdr->ipv4Dip ) &&
				(pChkRule->acl_filter.ingress_dest_ipv4_addr_start<= pPktHdr->ipv4Dip ))){
				ACL("RG_ACL[%d] INGRESS_IPV4_DIP_RANGE UNHIT ",acl_sw_idx);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_SPORT_RANGE_BIT){
			if( !((pChkRule->acl_filter.ingress_src_l4_port_end >= pPktHdr->sport ) &&
				(pChkRule->acl_filter.ingress_src_l4_port_start<= pPktHdr->sport ))){
				ACL("RG_ACL[%d] INGRESS_L4_SPORT_RANGE UNHIT ",acl_sw_idx);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_DPORT_RANGE_BIT){
			if( !((pChkRule->acl_filter.ingress_dest_l4_port_end >= pPktHdr->dport ) &&
				(pChkRule->acl_filter.ingress_dest_l4_port_start<= pPktHdr->dport ))){
				ACL("RG_ACL[%d] INGRESS_L4_DPORT_RANGE UNHIT ",acl_sw_idx);
				continue;
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_PKT_LEN_RANGE_BIT)
		{
			if( !((pChkRule->acl_filter.ingress_packet_length_end >= (pPktHdr->skbLen+4) ) &&	//follow asic packet range behavior: include CRC 4byte
				(pChkRule->acl_filter.ingress_packet_length_start <= (pPktHdr->skbLen+4) ))){
				ACL("RG_ACL[%d] INGRESS_PKT_LEN_RANGE_BIT UNHIT ",acl_sw_idx);
				continue;
			}
		}


		if(pChkRule->acl_filter.filter_fields&INGRESS_WLANDEV_BIT){

			if((pPktHdr->wlan_dev_idx >= 0) && (pChkRule->acl_filter.ingress_wlanDevMask&(1<<pPktHdr->wlan_dev_idx))){
				//hit!
			}else{
				ACL("RG_ACL[%d] INGRESS_WLANDEV_BIT UNHIT ingress_wlanDevMask=0x%x pPktHdr->wlan_dev_idx=%d",acl_sw_idx,pChkRule->acl_filter.ingress_wlanDevMask,pPktHdr->wlan_dev_idx);
				continue;
			}
		}


#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
		if((pChkRule->acl_filter.filter_fields&EGRESS_INTF_BIT)
			&& (pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)){

			if(pChkRule->acl_filter.egress_intf_idx!=pPktHdr->egressIntfIdxPreTrans){
				ACL("RG_ACL[%d] EGRESS_INTF_BIT UNHIT",acl_sw_idx);
				continue;
			}

		}
#endif



		//record ACL hit rule
		ACL("RG_ACL[%d] Ingress Part Hit!",acl_sw_idx);
		if(ingressCvidRuleIdxArray==NULL){
			pPktHdr->aclDecision.aclIgrHitMask[(i>>5)]|=(1<<(i&0x1f));
			pPktHdr->aclDecision.aclEgrHaveToCheckRuleIdx[hitRuleCounter]=i; //record current rule
			pPktHdr->aclDecision.aclEgrHaveToCheckRuleIdx[hitRuleCounter+1]=-1;//empty next rule until the end
			hitRuleCounter++;
		}else{
			//ingress_cvid action can only be single hit
			if(pChkRule->acl_filter.qos_actions & ACL_ACTION_ACL_INGRESS_VID_BIT){
				pPktHdr->internalVlanID =  pChkRule->acl_filter.action_acl_ingress_vid;
				TRACE("RG_ACL[%d] HIT: internalVlanID modify to %d",acl_sw_idx,pPktHdr->internalVlanID);
				pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_CVLAN_ACT_DONE_BIT; //the ACL IGR_CVLAN_ACT is done
			}
			else
			{
				TRACE("RG_ACL[%d] HIT: dot1p remarking action will cause ingress_cvid action invalid",acl_sw_idx,pPktHdr->internalVlanID);
			}
			break; //just check the first ACL CACT hit rule when ingressCvidRuleIdxArray!=NULL
		}

	}

	return RT_ERR_RG_OK;
}



rtk_rg_fwdEngineReturn_t _rtk_rg_ingressACLAction(rtk_rg_pktHdr_t *pPktHdr)
{
	/*
	*	This API is using for do pure ingress ACL actions such as:  acl_priority, drop, trap_to_PS
	*	remarking actions will be done after _rtk_rg_egressACLAction() & _rtk_rg_modifyPacketByACLAction()
	*/

	//ingress_cvid,  ingress_svid have not supported right now.
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

				if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_QOS &&
					(pChkRule->acl_filter.qos_actions & ACL_ACTION_ACL_PRIORITY_BIT) &&
					((pPktHdr->aclDecision.aclIgrDoneAction & RG_IGR_PRIORITY_ACT_DONE_BIT)==0x0)){

					pPktHdr->aclPriority = pChkRule->acl_filter.action_acl_priority;
					ACL("RG_ACL[%d] do ACL_PRIORITY to %d", aclIdx, pPktHdr->aclPriority);
					pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_PRIORITY_ACT_DONE_BIT; //the ACL IGR_PRIORITY_ACT is done
				}

				if(pPktHdr->aclPolicyRoute==FAIL && pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_POLICY_ROUTE)
				{
					ACL("RG_ACL[%d] HIT: Policy Route! use WAN[%d] as egress intf!!",aclIdx,pChkRule->acl_filter.action_policy_route_wan);
					//Keep policy route indicated egress WAN interface index in pktHdr
					pPktHdr->aclPolicyRoute=pChkRule->acl_filter.action_policy_route_wan;
				}

				if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_PERMIT)
				{
					ACL("RG_ACL[%d]: PERMIT",aclIdx);
					TRACE("ACL[%d] ACT:PERMIT",i);
					pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_FORWARD_ACT_DONE_BIT;
				}

				if(((pPktHdr->aclDecision.aclIgrDoneAction&RG_IGR_FORWARD_ACT_DONE_BIT)==0x0)&&
					(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_DROP)
#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
					&& ((pChkRule->acl_filter.filter_fields&EGRESS_INTF_BIT)==0x0) //make sure not transform case! if transformed, it should drop in _rtk_rg_egressACLAction() after interface checked!
#endif
				)
				{
					ACL(" RG_ACL[%d]: DROP",aclIdx);
					TRACE("[Drop] ACL[%d] ACT:DROP",i);
					pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_FORWARD_ACT_DONE_BIT;
					return RG_FWDENGINE_RET_DROP;
				}

				if(((pPktHdr->aclDecision.aclIgrDoneAction&RG_IGR_FORWARD_ACT_DONE_BIT)==0x0)&&
					(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_TRAP_TO_PS)&&
					(pPktHdr->pRxDesc->rx_dst_port_mask!=0x20)
#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
					&& ((pChkRule->acl_filter.filter_fields&EGRESS_INTF_BIT)==0x0) //make sure not transform case! if transformed, it should trap in _rtk_rg_egressACLAction() after interface checked!
#endif
				)
				{
					ACL(" RG_ACL[%d] TRAP to PS",aclIdx);
					TRACE("[To PS] ACL[%d] ACT:TRAP to PS",i);
					pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_FORWARD_ACT_DONE_BIT;
					pPktHdr->byPassToPsVlanAclDecision = 1;
					return RG_FWDENGINE_RET_TO_PS;
				}
			}
		}
	}

	return RG_FWDENGINE_RET_CONTINUE;

}


int _rtk_rg_egressACLPatternCheck(int direct, int naptIdx, rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb,int l3Modify,int l4Modify, rtk_rg_port_idx_t egressPort)
{
	//egressPort used for confirm packet egress to CF port, egressPort==-1 represent BC to lan
	uint32 i,index;
	rtk_rg_aclFilterEntry_t* pChkRule;
	ipaddr_t sipModify,dipModify;
	uint16 sportModify,dportModify;
	int dipL3Idx,netifIdx=-1,nexthopIdx;
	uint32 flowDecision;
	rtk_rg_classifyEntry_t* pL2CFChkRule;
	uint32 is_ip4mc_check=0,is_ip6mc_check=0;
	uint32 uni_check_port;
	//reset the egressHitMask & final actions, because in broacast dataPath each Interfcaes will call this API individually

	for(i=0;i<((MAX_ACL_SW_ENTRY_SIZE/32)+1);i++){
		pPktHdr->aclDecision.aclEgrHitMask[i]=0;
	}

	for(i=0;i<((TOTAL_CF_ENTRY_SIZE/32)+1);i++){
		pPktHdr->aclDecision.aclEgrPattern1HitMask[i]=0;
	}

	_rtk_rg_aclDecisionClear(pPktHdr);



	if((egressPort == RTK_RG_PORT_PON)&&(rg_db.systemGlobal.ponPortUnmatchCfDrop))
		pPktHdr->unmatched_cf_act = IDX_UNHIT_DROP; //to PON port, only upstream; not need to check if any downstream pkt
	else
		pPktHdr->unmatched_cf_act = IDX_UNHIT_PASS;

	//mib counter and cf decision
	_rtk_rg_IntfIdxDecisionForCF(pPktHdr);

/*
	if(rg_db.systemGlobal.acl_SW_table_entry_size<=0)//no rule need to verify
		return RT_ERR_RG_OK;
*/
	//get original info
	sipModify = pPktHdr->ipv4Sip;
	dipModify = pPktHdr->ipv4Dip;
	sportModify = pPktHdr->sport;
	dportModify = pPktHdr->dport;
	//ACL("Egress Pattern check: direct=%d naptIdx=%d l3Modify=%d l4Modify=%d netifIdx=%d  fwdDecision=%d",direct,naptIdx,l3Modify,l4Modify,pPktHdr->netifIdx,pPktHdr->fwdDecision);

	//get L34 info (translate SIP/DIP  SPORT/DPORT)
	if(direct==RG_FWD_DECISION_NAPT)//NAPT OUTBOUND
	{

		//fill SIP
		if(l3Modify)
			sipModify=rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr;

		//fill SPORT
		if(l4Modify)
			sportModify=rg_db.naptOut[naptIdx].extPort;

		//pPktHdr->netifIdx has been set by _rtk_rg_routingDecisionTablesLookup(pPktHdr,SIP_DIP_CLASS_NAPT);
		netifIdx = pPktHdr->netifIdx;

	}
	else if(direct==RG_FWD_DECISION_NAPTR)//NAPTr INBOUND
	{
		//fill DIP
		if(l3Modify)
			dipModify=rg_db.naptIn[naptIdx].rtk_naptIn.intIp;

		//fill DPORT
		if(l4Modify)
			dportModify=rg_db.naptIn[naptIdx].rtk_naptIn.intPort;

		//set egress intf index to pktHdr
		//_rtk_rg_routingDecisionTablesLookup(pPktHdr,SIP_DIP_CLASS_NAPTR); => calling this API call will make some of pktHdr information changed!we just need to decide  egress netifIdx.
		dipL3Idx = _rtk_rg_l3lookup(dipModify);
		if(rg_db.l3[dipL3Idx].rtk_l3.process==L34_PROCESS_ARP)
		{
			netifIdx=rg_db.l3[dipL3Idx].rtk_l3.netifIdx;
		}
		else if(rg_db.l3[dipL3Idx].rtk_l3.process==L34_PROCESS_NH)
		{
			nexthopIdx=rg_db.l3[dipL3Idx].rtk_l3.nhStart;
			netifIdx=rg_db.nexthop[nexthopIdx].rtk_nexthop.ifIdx;
		}
		else if(rg_db.l3[dipL3Idx].rtk_l3.process==L34_PROCESS_CPU)
		{

			if(rg_db.l3[dipL3Idx].rtk_l3.ipAddr > 0)
			{
				netifIdx=rg_db.l3[dipL3Idx].rtk_l3.netifIdx;
			}
			else
			{
				netifIdx = -1; //netifIdx not found
			}
		}

	}else{
		//bridge mode or IPv6 routing mode: will be Fail(-1) => can not hit egress_CVID, or egress_CPRI pattern.
		//or binding: pPktHdr->netifIdx will be the wanIntf index
		//or from protocal-stack:  pPktHdr->netifIdx will be the wanIntf index.
		netifIdx = pPktHdr->netifIdx;
	}


	//flow decision
	if(pPktHdr->ingressPort==RTK_RG_PORT_PON) //downstream
	{
		flowDecision = CF_DOWNSTREAM;
		ACL("flowDecision: CF_DOWNSTREAM");
	}
	else //maybe upstream,
	{
		if(egressPort==RTK_RG_PORT_PON)//upstream
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

	/*check CF pattern0*/
	for(index=0;index<MAX_ACL_SW_ENTRY_SIZE;index++){
		i = pPktHdr->aclDecision.aclEgrHaveToCheckRuleIdx[index];
		if(i==-1)//no rest rule need to check
			break;

		pChkRule = &(rg_db.systemGlobal.acl_SW_table_entry[i]);
		ACL("Check egress part of ACLRule[%d]:",i);

		//direction check
		if(flowDecision == CF_DOWNSTREAM) //downstream
		{
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP)
			{
				ACL("RG_ACL[%d] INGRESS_PORT UNHIT DOWNSTREAM ",i);
				continue;
			}
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN)
			{
				ACL("RG_ACL[%d] INGRESS_PORT UNHIT DOWNSTREAM ",i);
				continue;
			}
		}
		else if(flowDecision == CF_UPSTREAM)//maybe upstream,
		{
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP)
			{
				ACL("RG_ACL[%d] INGRESS_PORT UNHIT UPSTREAM ",i);
				continue;
			}
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN)
			{
				ACL("RG_ACL[%d] INGRESS_PORT UNHIT UPSTREAM ",i);
				continue;
			}

		}
		else //lan-to-lan
		{

#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET && (pChkRule->acl_filter.filter_fields&EGRESS_INTF_BIT)==0x0) /*Transform should check egress_intf_idx*/
#else
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
#endif
			{
				ACL("RG_ACL[%d] fwdtype = ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET, speedup by skip egress pattern check ",i);
				goto skipEgressPatternCheck;
			}
			else //fwdtype = 1~4
			{
				//consider drop
				if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_DROP){
					//let it continue check. The Drop action will alwaye be Execute(if hit) even packet is not related to CF port.
				}else{
					ACL("RG_ACL[%d] Lan-to-Lan, none-related to CF port",i);
					continue;
				}
			}
		}

		if(pChkRule->acl_filter.filter_fields&EGRESS_INTF_BIT){

			if(pPktHdr->ingressLocation==RG_IGR_PROTOCOL_STACK) //from PS,
			{

#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
				if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP || pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN || pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET/*could be transform*/)
#else
				if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP || pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN)
#endif
				{
					if(pChkRule->acl_filter.egress_intf_idx!=pPktHdr->netifIdx) //from PS, it's special pure software data path, the interface is determind as elder version which from pPktHdr->netifIdx
					{
						DEBUG("pPktHdr->netifIdx=%d",pPktHdr->netifIdx);
						ACL("RG_CF[%d] EGRESS_INTF_BIT UNHIT ",i);
						continue;
					}
				}
				else
				{
					ACL("RG_ACL[%d] EGRESS_INTF_BIT UNHIT (not upstream rule)",i);
					continue;
				}

			}
			else //normal forwarding
			{
#if CONFIG_ACL_EGRESS_WAN_INTF_TRANSFORM
				if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP || pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN || pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET/*could be transform*/)
#else
				if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP || pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN)
#endif
				{
					if(pChkRule->acl_filter.egress_intf_idx!=pPktHdr->mibNetifIdx)
					{
						//DEBUG("pPktHdr->mibNetifIdx=%d",pPktHdr->mibNetifIdx);
						ACL("RG_CF[%d] EGRESS_INTF_BIT UNHIT ",i);
						continue;
					}
				}
				else
				{
					ACL("RG_ACL[%d] EGRESS_INTF_BIT UNHIT (not upstream rule)",i);
					continue;
				}
			}
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_INTF_BIT){
			//[FIXME] pPktHdr->ingressMibNetifIdx is not assigned value yet (wait for MIB module).
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP || pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN) //downstream check ingress MibInterface
			{
				if(pChkRule->acl_filter.ingress_intf_idx!=pPktHdr->mibNetifIdx)
				{
					DEBUG("pPktHdr->mibNetifIdx=%d",pPktHdr->mibNetifIdx);
					ACL("RG_CF[%d] INGRESS_INTF_BIT UNHIT ",i);
					continue;
				}
			}
			else if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
			{
				//pure ACL, don't care this pattern.  the ingress interface is checked by _rtk_rg_ingressACLPatternCheck()
			}
			else //upstream doen't have ingress MibInterface
			{
				ACL("RG_ACL[%d] INGRESS_INTF_BIT UNHIT (not downstream rule)",i);
				continue;
			}

		}


		if(pChkRule->acl_filter.filter_fields&EGRESS_IPV4_SIP_RANGE_BIT){
			if( !((pChkRule->acl_filter.egress_src_ipv4_addr_end >= sipModify ) &&
				(pChkRule->acl_filter.egress_src_ipv4_addr_start<= sipModify ))){
				//DEBUG(" sipModify=0x%x",sipModify);
				//ACL("EGRESS_src_ipv4_addr_start=0x%x egress_src_ipv4_addr_end=0x%x",pChkRule->acl_filter.egress_src_ipv4_addr_start,pChkRule->acl_filter.egress_src_ipv4_addr_end);
				ACL("RG_ACL[%d] EGRESS_IPV4_SIP_RANGE UNHIT ",i);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&EGRESS_IPV4_DIP_RANGE_BIT){
			if( !((pChkRule->acl_filter.egress_dest_ipv4_addr_end >= dipModify ) &&
				(pChkRule->acl_filter.egress_dest_ipv4_addr_start<= dipModify))){
				//DEBUG(" dipModify=0x%x",dipModify);
				//ACL("EGRESS_dest_ipv4_addr_start=0x%x egress_dest_ipv4_addr_end=0x%x",pChkRule->acl_filter.egress_dest_ipv4_addr_start,pChkRule->acl_filter.egress_dest_ipv4_addr_end);
				ACL("RG_ACL[%d] EGRESS_IPV4_DIP_RANGE UNHIT ",i);
				continue;
			}
		}
		if(pChkRule->acl_filter.filter_fields&EGRESS_L4_SPORT_RANGE_BIT){
			if( !((pChkRule->acl_filter.egress_src_l4_port_end >= sportModify ) &&
				(pChkRule->acl_filter.egress_src_l4_port_start<= sportModify ))){
				ACL("RG_ACL[%d] EGRESS_L4_SPORT_RANGE UNHIT ",i);
				continue;
			}

		}
		if(pChkRule->acl_filter.filter_fields&EGRESS_L4_DPORT_RANGE_BIT){
			if( !((pChkRule->acl_filter.egress_dest_l4_port_end >= dportModify ) &&
				(pChkRule->acl_filter.egress_dest_l4_port_start<= dportModify))){
				ACL("RG_ACL[%d] EGRESS_L4_DPORT_RANGE UNHIT ",i);
				continue;
			}
		}


		if(pChkRule->acl_filter.filter_fields&EGRESS_CTAG_VID_BIT){//L34uptream only
			//[FIXME] apolloFE upstream H/W direstly use internalCVID (H/W include L34, binding, ACL ingress/egress cvid and DMAC2CVID result. However, pPktHdr->internalVlanID just include L34, binding, ACL ingressCVID,  it should fix to include DMAC2CVID at least)

			//this pattern must egress with Ctag
			if(pPktHdr->egressVlanTagif==0){
				ACL("RG_ACL[%d] EGRESS_CTAG_VID_BIT UNHIT  packet without Ctag ",i);
				continue;
			}

			if((pChkRule->acl_filter.egress_ctag_vid & pChkRule->acl_filter.egress_ctag_vid_mask)!= (pPktHdr->internalVlanID & pChkRule->acl_filter.egress_ctag_vid_mask)){
				ACL("RG_ACL[%d] EGRESS_CTAG_VID_BIT UNHIT  pChkRule->egress_ctag_vid=%d pPktHdr->egressVlanID=%d, pChkRule->acl_filter.egress_ctag_vid_mask=0x%x",i,pChkRule->acl_filter.egress_ctag_vid,pPktHdr->internalVlanID,pChkRule->acl_filter.egress_ctag_vid_mask);
				continue;
			}

		}

		if(pChkRule->acl_filter.filter_fields&EGRESS_CTAG_PRI_BIT){//L34uptream only
			//pPktHdr->egressPriority should include 1Q-tag, port-based, Qos 1p-remarking decision.

			//this pattern must egress with Ctag
			if(pPktHdr->egressVlanTagif==0){
				ACL("RG_ACL[%d] EGRESS_CTAG_PRI_BIT UNHIT  packet without Ctag ",i);
				continue;
			}

			//[FIXME] broadcast datapath may not include ACL and Qos 1p-remarking decision in pPktHdr->egressPriority
			if(pChkRule->acl_filter.egress_ctag_pri!=pPktHdr->egressPriority)
			{
				ACL("RG_ACL[%d] EGRESS_CTAG_PRI_BIT UNHIT ",i);
				continue;
			}
		}

		if(pChkRule->acl_filter.filter_fields&INTERNAL_PRI_BIT){
			if(pChkRule->acl_filter.internal_pri!=pPktHdr->internalPriority){
				ACL("RG_ACL[%d] INTERNAL_PRI_BIT UNHIT ",i);
				continue;
			}
		}


		if(pChkRule->acl_filter.filter_fields&INGRESS_EGRESS_PORTIDX_BIT){//uni pattern
			//[FIXME] dowstream will following L34 DA lookup result

			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP ||pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN)
			{
				if(pPktHdr->ingressPort > RTK_RG_PORT_CPU){ //from EXT port should check as CPU port
					uni_check_port = RTK_RG_PORT_CPU;
				}else{
					uni_check_port = pPktHdr->ingressPort;
				}

				//CF upstream(to PON), check spa
				if( (pChkRule->acl_filter.ingress_port_idx & pChkRule->acl_filter.ingress_port_idx_mask)!=(uni_check_port & pChkRule->acl_filter.ingress_port_idx_mask)){
					ACL("RG_ACL[%d] INGRESS_EGRESS_PORTIDX_BIT UNHIT ",i);
					continue;
				}
			}
			else if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP ||pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN)
			{	//CF downstream(from PON),check da
				if( (pChkRule->acl_filter.egress_port_idx & pChkRule->acl_filter.egress_port_idx_mask)!=(pPktHdr->egressMacPort & pChkRule->acl_filter.egress_port_idx_mask)){
					ACL("RG_ACL[%d] INGRESS_EGRESS_PORTIDX_BIT UNHIT ",i);
					continue;
				}
			}
		}

		if(pChkRule->acl_filter.filter_fields&EGRESS_IP4MC_IF){
			if((pPktHdr->pDmac[0]==0x01&& pPktHdr->pDmac[1]==0x00 && pPktHdr->pDmac[2]==0x5e) && (pPktHdr->tagif&IGMP_TAGIF)==0x0){
				is_ip4mc_check = 1; //IP4MC(not include IGMP)
			}

			if(pChkRule->acl_filter.egress_ip4mc_if!=is_ip4mc_check){
				ACL("RG_ACL[%d] EGRESS_IP4MC_IF UNHIT ",i);
						continue;
			}
		}


		if(pChkRule->acl_filter.filter_fields&EGRESS_IP6MC_IF){
			if((pPktHdr->pDmac[0]==0x33 && pPktHdr->pDmac[1]==0x33) && (pPktHdr->tagif&IPV6_MLD_TAGIF)==0x0){
				is_ip6mc_check = 1; //IP6MC(not include MLD)
			}

			if(pChkRule->acl_filter.egress_ip6mc_if!=is_ip6mc_check){
				ACL("RG_ACL[%d] EGRESS_IP6MC_IF UNHIT ",i);
						continue;
			}

		}



		//record CF hit rule
		//ACL("HIT CF0-63[%d]",i);
		pPktHdr->unmatched_cf_act = IDX_UNHIT_PASS;
skipEgressPatternCheck:
		ACL("RG_ACL[%d] Egress Part Hit!",i);
		pPktHdr->aclDecision.aclEgrHitMask[i>>5]|=(1<<(i&0x1f));

	}


	/*check CF pattern1*/
	for(i=rg_db.systemGlobal.cf_pattern0_size;i<TOTAL_CF_ENTRY_SIZE;i++){

		//CF pattern1 rule is valid if index is none zero
		if(rg_db.systemGlobal.classify_SW_table_entry[i].index!=0){
			pL2CFChkRule = &(rg_db.systemGlobal.classify_SW_table_entry[i]);
			ACL("Check CFRule[%d]:",i);
		}else{
			continue;
		}

		//check upstream/downstream
		if(pPktHdr->ingressPort==RTK_RG_PORT_PON){
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

				if(egressPort!=RTK_RG_PORT_PON)
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

				if(pPktHdr->ingressPort > RTK_RG_PORT_CPU){ //from EXT port should check as CPU port
					uni_check_port = RTK_RG_PORT_CPU;
				}else{
					uni_check_port = pPktHdr->ingressPort;
				}

				if((pL2CFChkRule->uni & pL2CFChkRule->uni_mask)!= (uni_check_port & pL2CFChkRule->uni_mask)){
					ACL("RG_CF[%d] EGRESS_UNI_BIT UNHIT ",i);
					continue;
				}
			}
			//downstream : CF0 should decide by DMAC lookup, CF1 should decide by ingress DMAC
			if(pL2CFChkRule->direction==RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM){
				ACL("uni=%d uni_mask=0x%x direct=%d (cpuPort=%d egressPort=%d)",pL2CFChkRule->uni,pL2CFChkRule->uni_mask,direct,RTK_RG_MAC_PORT_CPU,pPktHdr->egressMacPort);
				if (direct!=RG_FWD_DECISION_BRIDGING){//NAPT or V4V6Routing, ingress DA lookup should always be CPU port because DA==Gateway MAC
					if((pL2CFChkRule->uni & pL2CFChkRule->uni_mask)!=RTK_RG_MAC_PORT_CPU){
						ACL("RG_CF[%d] EGRESS_UNI_BIT UNHIT ",i);
						continue;
					}
				}
				else{//Bridge
					if((pL2CFChkRule->uni & pL2CFChkRule->uni_mask)!= (pPktHdr->egressMacPort & pL2CFChkRule->uni_mask)){
						ACL("RG_CF[%d] EGRESS_UNI_BIT UNHIT ",i);
						continue;
					}
				}
			}
		}

		pPktHdr->aclDecision.direction = pL2CFChkRule->direction;
		pPktHdr->aclDecision.aclEgrPattern1HitMask[(i>>5)]|=(1<<(i&0x1f));
		ACL("RG_CF[%d] Hit!!!",i);

		pPktHdr->unmatched_cf_act = IDX_UNHIT_PASS;
		//FIXME:CF can just hit one rule! So we just check until first hit.
		break;
	}

	return RT_ERR_RG_OK;

}



int _rtk_rg_egressACLAction(int direct, rtk_rg_pktHdr_t *pPktHdr)
{
	uint32 i,acl_idx;
	rtk_rg_aclFilterEntry_t* pChkRule;

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
			else if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_TRAP_TO_PS)
			{

				if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
				{
					//ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET trap is already done in _rtk_rg_ingressACLAction()
				}
				else if((pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP || pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP))
				{
					pPktHdr->aclDecision.aclEgrDoneAction |= RG_EGR_FWD_ACT_DONE_BIT;
					TRACE("[To PS] ACL[%d] ACT:TRAP to PS",acl_idx);
					pPktHdr->byPassToPsVlanAclDecision = 1;
					return RG_FWDENGINE_RET_TO_PS;
				}
			}
			else if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_PERMIT)
			{

				if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
				{
					pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_FORWARD_ACT_DONE_BIT;
					TRACE("RG_ACL[%d] PERMIT!",acl_idx);

				}
				else if((pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP || pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP))
				{
					pPktHdr->aclDecision.aclEgrDoneAction |= RG_EGR_FWD_ACT_DONE_BIT;
					TRACE("RG_ACL[%d] PERMIT!",acl_idx);

				}

			}
			else //Qos Type
			{
				pPktHdr ->aclDecision.action_type = ACL_ACTION_TYPE_QOS;

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


				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_QUEUE_ID_BIT){
					ACL("ACL_ACTION_QUEUE_ID_BIT is not supported in apolloFE!");
				}

				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_SHARE_METER_BIT){

					if((pPktHdr->aclDecision.aclIgrDoneAction&RG_IGR_POLICE_ACT_DONE_BIT)==0x0) //ACL RG_IGR_POLICE_ACT_DONE_BIT have not done
					{
						pPktHdr->aclDecision.qos_actions |= ACL_ACTION_SHARE_METER_BIT;
						pPktHdr->aclDecision.action_share_meter= pChkRule->acl_filter.action_share_meter;
						pPktHdr->aclDecision.aclIgrDoneAction |= RG_IGR_POLICE_ACT_DONE_BIT;
						ACL("RG_ACL[%d] do SHARE_METER to %d", acl_idx, pPktHdr->aclDecision.action_share_meter);
						TRACE("RG_ACL[%d] do SHARE_METER to %d", acl_idx, pPktHdr->aclDecision.action_share_meter);

						if(PURE_SW_METER_IDX_OFFSET<=pChkRule->acl_filter.action_share_meter && pChkRule->acl_filter.action_share_meter<(PURE_SW_METER_IDX_OFFSET+PURE_SW_SHAREMETER_TABLE_SIZE)){
							pPktHdr->swMeterOffsetIdx = pPktHdr->aclDecision.action_share_meter - PURE_SW_METER_IDX_OFFSET;
							TRACE("[QoS] ACL[%d] packet always trap due to should care sw share meter[%d](idx >= %d)", acl_idx, pPktHdr->aclDecision.action_share_meter, PURE_SW_METER_IDX_OFFSET);
						}
					}
					else
					{
						ACL("RG_ACL[%d] skip SHARE_METER to %d, because IGR_POLICE_ACT is already done!", acl_idx, pChkRule->acl_filter.action_share_meter);
					}

				}

				if(pChkRule->acl_filter.qos_actions & ACL_ACTION_LOG_COUNTER_BIT){
					ACL("ACL_ACTION_LOG_COUNTER_BIT is not supported in software!");
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
				}
			}

		}
	}



	//check CF pattern1.
	for(i=rg_db.systemGlobal.cf_pattern0_size;i<TOTAL_CF_ENTRY_SIZE;i++){
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

						ACL("RG_CF[%d] do VLAN %s  svid_act:%s  spri_act:%s  svid:%d  spri:%d",
							i,
							name_of_rg_svlan_tagif_decision[pPktHdr->aclDecision.action_svlan.svlanTagIfDecision],
							name_of_rg_svlan_svid_decision[pPktHdr->aclDecision.action_svlan.svlanSvidDecision],
							name_of_rg_svlan_spri_decision[pPktHdr->aclDecision.action_svlan.svlanSpriDecision],
							pPktHdr->aclDecision.action_svlan.assignedSvid,
							pPktHdr->aclDecision.action_svlan.assignedSpri
						);
						TRACE("RG_CF[%d] do VLAN %s  svid_act:%s  spri_act:%s  svid:%d  spri:%d",
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

						ACL("RG_CF[%d] do CVLAN %s  cvid_act:%s  cpri_act:%s  cvid:%d	cpri:%d",
							i,
							name_of_rg_cvlan_tagif_decision[pPktHdr->aclDecision.action_cvlan.cvlanTagIfDecision],
							name_of_rg_cvlan_cvid_decision[pPktHdr->aclDecision.action_cvlan.cvlanCvidDecision],
							name_of_rg_cvlan_cpri_decision[pPktHdr->aclDecision.action_cvlan.cvlanCpriDecision],
							pPktHdr->aclDecision.action_cvlan.assignedCvid,
							pPktHdr->aclDecision.action_cvlan.assignedCpri);
						TRACE("RG_CF[%d] do CVLAN %s  cvid_act:%s  cpri_act:%s  cvid:%d	cpri:%d",
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

						ACL("RG_CF[%d] do VLAN %s  svid_act:%s  spri_act:%s  svid:%d  spri:%d",
							i,
							name_of_rg_svlan_tagif_decision[pPktHdr->aclDecision.action_svlan.svlanTagIfDecision],
							name_of_rg_svlan_svid_decision[pPktHdr->aclDecision.action_svlan.svlanSvidDecision],
							name_of_rg_svlan_spri_decision[pPktHdr->aclDecision.action_svlan.svlanSpriDecision],
							pPktHdr->aclDecision.action_svlan.assignedSvid,
							pPktHdr->aclDecision.action_svlan.assignedSpri
						);
						TRACE("RG_CF[%d] do VLAN %s  svid_act:%s  spri_act:%s  svid:%d  spri:%d",
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

						ACL("RG_CF[%d] do CVLAN %s  cvid_act:%s  cpri_act:%s  cvid:%d	cpri:%d",
							i,
							name_of_rg_cvlan_tagif_decision[pPktHdr->aclDecision.action_cvlan.cvlanTagIfDecision],
							name_of_rg_cvlan_cvid_decision[pPktHdr->aclDecision.action_cvlan.cvlanCvidDecision],
							name_of_rg_cvlan_cpri_decision[pPktHdr->aclDecision.action_cvlan.cvlanCpriDecision],
							pPktHdr->aclDecision.action_cvlan.assignedCvid,
							pPktHdr->aclDecision.action_cvlan.assignedCpri);
						TRACE("RG_CF[%d] do CVLAN %s  cvid_act:%s  cpri_act:%s  cvid:%d	cpri:%d",
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
						pPktHdr->aclDecision.ds_action_field|= CF_DS_ACTION_UNI_MASK_BIT;
						pPktHdr->aclDecision.aclEgrDoneAction|=RG_EGR_FWD_ACT_DONE_BIT;
						pPktHdr->aclDecision.action_uni=rg_db.systemGlobal.classify_SW_table_entry[i].action_uni;
						ACL("RG_CF[%d] do UNI %s, portmask=0x%x\n",i, name_of_rg_uni_decision[pPktHdr->aclDecision.action_uni.uniActionDecision],pPktHdr->aclDecision.action_uni.assignedUniPortMask);
						TRACE("RG_CF[%d] do UNI %s, portmask=0x%x\n",i, name_of_rg_uni_decision[pPktHdr->aclDecision.action_uni.uniActionDecision],pPktHdr->aclDecision.action_uni.assignedUniPortMask);
					}
					else
					{
						ACL("RG_CF[%d] skip UNI , because EGR_FWD_ACT is already done!\n",i);
					}
				}


			}

		}
	}

	if(pPktHdr->unmatched_cf_act == IDX_UNHIT_DROP)
	{
		if((pPktHdr->ingressPort==RTK_RG_PORT_PON && pon_is_cfport))
		{
			///downstream, no need to check cf un-match-drop
		}
		else
		{
			//drop L2 un-matched-CF pkt
			TRACE("[Drop] RG_CF Un-matched Drop");
			return RG_FWDENGINE_RET_DROP;
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

		if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT)>0)
		{
			if(pPktHdr->pTos==NULL)goto ACL_RET;		//packet may not have IP header
			if(!(pPktHdr->tagif&IPV4_TAGIF || pPktHdr->tagif&IPV6_TAGIF)) goto ACL_RET;

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
		else if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_DSCP_REMARKING_BIT)>0)
		{
			if(pPktHdr->pTos==NULL)goto ACL_RET;		//packet may not have IP header
			if(!(pPktHdr->tagif&IPV4_TAGIF || pPktHdr->tagif&IPV6_TAGIF)) goto ACL_RET;

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
				TRACE("Modify by RG_ACL Weired!!! wanPortGponMode=%d but assigned streamID action rule, skip this streamID action.",rg_db.systemGlobal.initParam.wanPortGponMode);
			}
#endif
		}

		if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_REDIRECT_BIT)>0){
			pPktHdr->egressUniPortmask = pPktHdr->aclDecision.action_redirect_portmask;
			TRACE("Modify by RG_ACL ACT:REDIRCT egressPortMask=0x%x",pPktHdr->egressUniPortmask);

			if(pPktHdr->egressUniPortmask==0x0){
				ACL("Drop! because egressPmsk is change to 0x0 by UNI action");
				TRACE("[Drop] Modify by RG_ACL: Drop! finalPortMask is 0x0");
				return RG_FWDENGINE_RET_DROP;
			}
		}

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
					}
					break;

				default:
					break;
			}
		}


		if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_ACL_CVLANTAG_BIT)>0){

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

ACL_RET:

	return RG_FWDENGINE_RET_CONTINUE;
}




/*(6)debug tool APIs*/

int _dump_rg_acl_entry_content(struct seq_file *s, rtk_rg_aclFilterAndQos_t *aclPara)
{
	rtk_rg_aclFilterAndQos_t *acl_parameter;

	//[FIXME] fwd_type can use name_of_string
	PROC_PRINTF(" fwd_type: %d\n",aclPara->fwding_type_and_direction);
	PROC_PRINTF(" acl_weight: %d\n",aclPara->acl_weight);


	PROC_PRINTF("[Patterns]: \n");
	PROC_PRINTF("filter_fields:0x%llx\n", aclPara->filter_fields);
	if(aclPara->filter_fields&INGRESS_PORT_BIT) PROC_PRINTF(" ingress_port_mask:0x%x\n", aclPara->ingress_port_mask.portmask);
	if(aclPara->filter_fields&INGRESS_EGRESS_PORTIDX_BIT) PROC_PRINTF(" ingress_port_idx:  %d  ingress_port_idx_mask:	0x%x\n", aclPara->ingress_port_idx,aclPara->ingress_port_idx_mask);
	if(aclPara->filter_fields&INGRESS_EGRESS_PORTIDX_BIT) PROC_PRINTF(" egress_port_idx:  %d  egress_port_idx_mask: 0x%x\n", aclPara->egress_port_idx,aclPara->egress_port_idx_mask);
	if(aclPara->filter_fields&INGRESS_DSCP_BIT) PROC_PRINTF(" ingress_dscp:    %d\n", aclPara->ingress_dscp);
	if(aclPara->filter_fields&INGRESS_IPV6_DSCP_BIT) PROC_PRINTF(" ingress_ipv6_dscp:	 %d\n", aclPara->ingress_ipv6_dscp);
	if(aclPara->filter_fields&INGRESS_TOS_BIT) PROC_PRINTF(" ingress_tos(ipv4):    %d\n", aclPara->ingress_tos);
	if(aclPara->filter_fields&INGRESS_IPV6_TC_BIT) PROC_PRINTF(" ingress_tc(ipv6):	  %d\n", aclPara->ingress_ipv6_tc);
	if(aclPara->filter_fields&INGRESS_INTF_BIT) PROC_PRINTF(" ingress_intf_idx: %d\n", aclPara->ingress_intf_idx);
	if(aclPara->filter_fields&EGRESS_INTF_BIT) PROC_PRINTF(" egress_intf_idx:  %d\n", aclPara->egress_intf_idx);
	if(aclPara->filter_fields&INGRESS_STREAM_ID_BIT) PROC_PRINTF(" ingress_stream_id:  %d\n ingress_stream_id_mask:  0x%x\n", aclPara->ingress_stream_id, aclPara->ingress_stream_id_mask);
	if(aclPara->filter_fields&INGRESS_ETHERTYPE_BIT) PROC_PRINTF(" ingress_ethertype:0x%x  ingress_ethertype_mask:0x%x\n", aclPara->ingress_ethertype, aclPara->ingress_ethertype_mask);
	if(aclPara->filter_fields&INGRESS_CTAG_VID_BIT) PROC_PRINTF(" ingress_ctag_vid: %d\n", aclPara->ingress_ctag_vid);
	if(aclPara->filter_fields&INGRESS_CTAG_PRI_BIT) PROC_PRINTF(" ingress_ctag_pri: %d\n", aclPara->ingress_ctag_pri);
	if(aclPara->filter_fields&INGRESS_CTAG_CFI_BIT) PROC_PRINTF(" ingress_ctag_cfi: %d\n", aclPara->ingress_ctag_cfi);
	if(aclPara->filter_fields&INGRESS_STAG_VID_BIT) PROC_PRINTF(" ingress_stag_vid: %d\n", aclPara->ingress_stag_vid);
	if(aclPara->filter_fields&INGRESS_STAG_PRI_BIT) PROC_PRINTF(" ingress_stag_pri: %d\n", aclPara->ingress_stag_pri);
	if(aclPara->filter_fields&INGRESS_STAG_DEI_BIT) PROC_PRINTF(" ingress_stag_dei: %d\n", aclPara->ingress_stag_dei);
	if(aclPara->filter_fields&INGRESS_SMAC_BIT) PROC_PRINTF(" ingress_smac: %02X:%02X:%02X:%02X:%02X:%02X ingress_smac_mask: %02X:%02X:%02X:%02X:%02X:%02X\n",
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
	if(aclPara->filter_fields&INGRESS_DMAC_BIT) PROC_PRINTF(" ingress_dmac: %02X:%02X:%02X:%02X:%02X:%02X  ingress_dmac_mask: %02X:%02X:%02X:%02X:%02X:%02X\n",
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

	//acl_parameter = &rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter;
	acl_parameter = aclPara;

	if(aclPara->filter_fields&INGRESS_IPV4_SIP_RANGE_BIT){
		PROC_PRINTF(" ingress_sip_low_bound: %s ", inet_ntoa(acl_parameter->ingress_src_ipv4_addr_start));
		PROC_PRINTF(" ingress_sip_up_bound: %s \n", inet_ntoa(acl_parameter->ingress_src_ipv4_addr_end));
	}
	if(aclPara->filter_fields&INGRESS_IPV4_DIP_RANGE_BIT){
		PROC_PRINTF(" ingress_dip_low_bound: %s ", inet_ntoa(acl_parameter->ingress_dest_ipv4_addr_start));
		PROC_PRINTF(" ingress_dip_up_bound: %s \n", inet_ntoa(acl_parameter->ingress_dest_ipv4_addr_end));
	}

	if(aclPara->filter_fields&INGRESS_IPV6_SIP_RANGE_BIT){
		PROC_PRINTF(" ingress_src_ipv6_addr_start: %02X%02X:",acl_parameter->ingress_src_ipv6_addr_start[0],acl_parameter->ingress_src_ipv6_addr_start[1]);
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
		PROC_PRINTF(" ingress_dest_ipv6_addr_start: %02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[0],acl_parameter->ingress_dest_ipv6_addr_start[1]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[2],acl_parameter->ingress_dest_ipv6_addr_start[3]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[4],acl_parameter->ingress_dest_ipv6_addr_start[5]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[6],acl_parameter->ingress_dest_ipv6_addr_start[7]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[8],acl_parameter->ingress_dest_ipv6_addr_start[9]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[10],acl_parameter->ingress_dest_ipv6_addr_start[11]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[12],acl_parameter->ingress_dest_ipv6_addr_start[13]);
		PROC_PRINTF(" :%02X%02X: \n",acl_parameter->ingress_dest_ipv6_addr_start[14],acl_parameter->ingress_dest_ipv6_addr_start[15]);

		PROC_PRINTF(" ingress_dest_ipv6_addr_end: %02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[0],acl_parameter->ingress_dest_ipv6_addr_end[1]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[2],acl_parameter->ingress_dest_ipv6_addr_end[3]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[4],acl_parameter->ingress_dest_ipv6_addr_end[5]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[6],acl_parameter->ingress_dest_ipv6_addr_end[7]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[8],acl_parameter->ingress_dest_ipv6_addr_end[9]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[10],acl_parameter->ingress_dest_ipv6_addr_end[11]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[12],acl_parameter->ingress_dest_ipv6_addr_end[13]);
		PROC_PRINTF(" :%02X%02X: \n",acl_parameter->ingress_dest_ipv6_addr_end[14],acl_parameter->ingress_dest_ipv6_addr_end[15]);
	}

	if(aclPara->filter_fields&INGRESS_IPV6_SIP_BIT){
		PROC_PRINTF(" ingress_src_ipv6_addr: %02X%02X:",acl_parameter->ingress_src_ipv6_addr[0],acl_parameter->ingress_src_ipv6_addr[1]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr[2],acl_parameter->ingress_src_ipv6_addr[3]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr[4],acl_parameter->ingress_src_ipv6_addr[5]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr[6],acl_parameter->ingress_src_ipv6_addr[7]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr[8],acl_parameter->ingress_src_ipv6_addr[9]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr[10],acl_parameter->ingress_src_ipv6_addr[11]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr[12],acl_parameter->ingress_src_ipv6_addr[13]);
		PROC_PRINTF(" :%02X%02X: \n",acl_parameter->ingress_src_ipv6_addr[14],acl_parameter->ingress_src_ipv6_addr[15]);

		PROC_PRINTF(" ingress_src_ipv6_addr_mask: %02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[0],acl_parameter->ingress_src_ipv6_addr_mask[1]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[2],acl_parameter->ingress_src_ipv6_addr_mask[3]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[4],acl_parameter->ingress_src_ipv6_addr_mask[5]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[6],acl_parameter->ingress_src_ipv6_addr_mask[7]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[8],acl_parameter->ingress_src_ipv6_addr_mask[9]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[10],acl_parameter->ingress_src_ipv6_addr_mask[11]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[12],acl_parameter->ingress_src_ipv6_addr_mask[13]);
		PROC_PRINTF(" :%02X%02X: \n",acl_parameter->ingress_src_ipv6_addr_mask[14],acl_parameter->ingress_src_ipv6_addr_mask[15]);
	}

	if(aclPara->filter_fields&INGRESS_IPV6_DIP_BIT){
		PROC_PRINTF(" ingress_dest_ipv6_addr: %02X%02X:",acl_parameter->ingress_dest_ipv6_addr[0],acl_parameter->ingress_dest_ipv6_addr[1]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[2],acl_parameter->ingress_dest_ipv6_addr[3]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[4],acl_parameter->ingress_dest_ipv6_addr[5]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[6],acl_parameter->ingress_dest_ipv6_addr[7]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[8],acl_parameter->ingress_dest_ipv6_addr[9]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[10],acl_parameter->ingress_dest_ipv6_addr[11]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[12],acl_parameter->ingress_dest_ipv6_addr[13]);
		PROC_PRINTF(" :%02X%02X: \n",acl_parameter->ingress_dest_ipv6_addr[14],acl_parameter->ingress_dest_ipv6_addr[15]);

		PROC_PRINTF(" ingress_dest_ipv6_addr_mask: %02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[0],acl_parameter->ingress_dest_ipv6_addr_mask[1]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[2],acl_parameter->ingress_dest_ipv6_addr_mask[3]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[4],acl_parameter->ingress_dest_ipv6_addr_mask[5]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[6],acl_parameter->ingress_dest_ipv6_addr_mask[7]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[8],acl_parameter->ingress_dest_ipv6_addr_mask[9]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[10],acl_parameter->ingress_dest_ipv6_addr_mask[11]);
		PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[12],acl_parameter->ingress_dest_ipv6_addr_mask[13]);
		PROC_PRINTF(" :%02X%02X: \n",acl_parameter->ingress_dest_ipv6_addr_mask[14],acl_parameter->ingress_dest_ipv6_addr_mask[15]);
	}

	if(aclPara->filter_fields&INGRESS_L4_SPORT_RANGE_BIT){
		PROC_PRINTF(" ingress_src_l4_port_low_bound: %d ", acl_parameter->ingress_src_l4_port_start);
		PROC_PRINTF(" ingress_src_l4_port_up_bound: %d \n", acl_parameter->ingress_src_l4_port_end);
	}
	if(aclPara->filter_fields&INGRESS_L4_DPORT_RANGE_BIT){
		PROC_PRINTF(" ingress_dest_l4_port_low_bound: %d  ", acl_parameter->ingress_dest_l4_port_start);
		PROC_PRINTF(" ingress_dest_l4_port_up_bound:  %d \n", acl_parameter->ingress_dest_l4_port_end);
	}

	if(aclPara->filter_fields&EGRESS_IPV4_SIP_RANGE_BIT){
		PROC_PRINTF(" egress_sip_low_bound: %s	", diag_util_inet_ntoa(acl_parameter->egress_src_ipv4_addr_start));
		PROC_PRINTF(" egress_sip_up_bound:	%s \n", diag_util_inet_ntoa(acl_parameter->egress_src_ipv4_addr_end));
	}
	if(aclPara->filter_fields&EGRESS_IPV4_DIP_RANGE_BIT){
		PROC_PRINTF(" egress_dip_low_bound: %s	", diag_util_inet_ntoa(acl_parameter->egress_dest_ipv4_addr_start));
		PROC_PRINTF(" egress_dip_up_bound:	%s \n", diag_util_inet_ntoa(acl_parameter->egress_dest_ipv4_addr_end));
	}
	if(aclPara->filter_fields&EGRESS_L4_SPORT_RANGE_BIT){
		PROC_PRINTF(" egress_src_l4_port_low_bound: %d	", acl_parameter->egress_src_l4_port_start);
		PROC_PRINTF(" egress_src_l4_port_up_bound:	%d \n", acl_parameter->egress_src_l4_port_end);
	}
	if(aclPara->filter_fields&EGRESS_L4_DPORT_RANGE_BIT){
		PROC_PRINTF(" egress_dest_l4_port_low_bound: %d ", acl_parameter->egress_dest_l4_port_start);
		PROC_PRINTF(" egress_dest_l4_port_up_bound: %d \n", acl_parameter->egress_dest_l4_port_end);
	}

	if(aclPara->filter_fields&EGRESS_CTAG_VID_BIT){
		PROC_PRINTF(" egress_ctag_vid:	%d \n", acl_parameter->egress_ctag_vid);
		PROC_PRINTF(" egress_ctag_vid_mask: 0x%x \n", acl_parameter->egress_ctag_vid_mask);
	}

	if(aclPara->filter_fields&EGRESS_CTAG_PRI_BIT) PROC_PRINTF("egress_ctag_pri:  %d \n", acl_parameter->egress_ctag_pri);


	if(acl_parameter->filter_fields & INGRESS_IPV4_TAGIF_BIT){
		PROC_PRINTF(" ipv4_tagIf: %s \n",acl_parameter->ingress_ipv4_tagif?"Must be IPv4":"Must not be IPv4");
	}

	if(acl_parameter->filter_fields & INGRESS_IPV6_TAGIF_BIT){
		PROC_PRINTF(" ipv6_tagIf: %s \n",acl_parameter->ingress_ipv6_tagif?"Must be IPv6":"Must not be IPv6");
	}

	if(acl_parameter->filter_fields & EGRESS_IP4MC_IF){
		PROC_PRINTF("egress_ip4mc_if: %s \n",acl_parameter->egress_ip4mc_if?"Must be IP4MC":"Must not be IP4MC");
	}
	if(acl_parameter->filter_fields & EGRESS_IP6MC_IF){
		PROC_PRINTF("egress_ip6mc_if: %s \n",acl_parameter->egress_ip6mc_if?"Must be IP6MC":"Must not be IP6MC");
	}

	if(acl_parameter->filter_fields & INGRESS_L4_TCP_BIT){
		PROC_PRINTF(" l4-protocal: tcp \n");
	}else if(acl_parameter->filter_fields & INGRESS_L4_UDP_BIT){
		PROC_PRINTF(" l4-protocal: udp \n");
	}else if(acl_parameter->filter_fields & INGRESS_L4_ICMP_BIT){
		PROC_PRINTF(" l4-protocal: icmp \n");
	}else if(acl_parameter->filter_fields & INGRESS_L4_NONE_TCP_NONE_UDP_BIT){
		PROC_PRINTF(" l4-protocal: none-tcp and none-udp \n");
	}else if(acl_parameter->filter_fields & INGRESS_L4_ICMPV6_BIT){
		PROC_PRINTF(" %s l4-protocal: icmpv6 \n",(aclPara->filter_fields_inverse&INGRESS_L4_ICMPV6_BIT)?"not":"");
	}else{
		//PROC_PRINTF(" l4-protocal: not care \n");
	}

	if(acl_parameter->filter_fields & INGRESS_L4_POROTCAL_VALUE_BIT){
		PROC_PRINTF(" l4-protocal-value: %d \n",acl_parameter->ingress_l4_protocal);
	}

	if(acl_parameter->filter_fields & INGRESS_STAGIF_BIT){
		if(aclPara->ingress_stagIf){
			PROC_PRINTF(" stagIf: must have Stag \n");
		}else{
			PROC_PRINTF(" stagIf: must not have Stag \n");
		}
	}
	if(acl_parameter->filter_fields & INGRESS_CTAGIF_BIT){
		if(aclPara->ingress_ctagIf){
			PROC_PRINTF(" ctagIf: must have Ctag \n");
		}else{
			PROC_PRINTF(" ctagIf: must not have Ctag \n");
		}
	}

	if(aclPara->filter_fields&INGRESS_PKT_LEN_RANGE_BIT){
		PROC_PRINTF(" ingress_packet_length_start:  %d  ingress_packet_length_end:  %d\n", acl_parameter->ingress_packet_length_start, acl_parameter->ingress_packet_length_end);
	}

	if(acl_parameter->filter_fields & INGRESS_WLANDEV_BIT){
		PROC_PRINTF(" ingress_wlanDevMask: 0x%x \n",acl_parameter->ingress_wlanDevMask);
	}

	if(acl_parameter->filter_fields & INTERNAL_PRI_BIT){
		PROC_PRINTF("internal priroity: %d \n",acl_parameter->internal_pri);
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
		default:
			break;
	}

	return 0;
}



int _dump_rg_acl(struct seq_file *s)
{

#if 1
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
#endif

	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){
		//if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields!=0x0){
		//DEBUG("aclSWEntry[i],valid = %d",i,rg_db.systemGlobal.acl_SW_table_entry[i].valid);
		if(rg_db.systemGlobal.acl_SW_table_entry[i].valid==RTK_RG_ENABLED){
			aclPara = &(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter);
			PROC_PRINTF("========================RG_ACL[%d]===========================\n",i);
			PROC_PRINTF("[hw_acl_start:%d(continue:%d) hw_cf_start:%d(continue:%d)] \n"
				,rg_db.systemGlobal.acl_SW_table_entry[i].hw_aclEntry_start
				,rg_db.systemGlobal.acl_SW_table_entry[i].hw_aclEntry_size
				,rg_db.systemGlobal.acl_SW_table_entry[i].hw_cfEntry_start
				,rg_db.systemGlobal.acl_SW_table_entry[i].hw_cfEntry_size);

			PROC_PRINTF("[Using range tables]: \n");
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_INGRESS_SIP4TABLE) PROC_PRINTF("ACL_SIP4_RANGE_TABLE[%d]  \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_SIP4TABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_INGRESS_DIP4TABLE) PROC_PRINTF("ACL_DIP4_RANGE_TABLE[%d]  \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_DIP4TABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_INGRESS_SIP6TABLE) PROC_PRINTF("ACL_SIP6_RANGE_TABLE[%d]  \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_SIP6TABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_INGRESS_DIP6TABLE) PROC_PRINTF("ACL_DIP6_RANGE_TABLE[%d]  \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_DIP6TABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_INGRESS_SPORTTABLE) PROC_PRINTF("ACL_SPORT_RANGE_TABLE[%d]  \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_SPORTTABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_INGRESS_DPORTTABLE) PROC_PRINTF("ACL_DPORT_RANGE_TABLE[%d]  \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_DPORTTABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_INGRESS_PKTLENTABLE) PROC_PRINTF(" ACL_PKTLEN_RANGE_TABLE[%d]  \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_PKTLENTABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_EGRESS_SIP4TABLE) PROC_PRINTF("CF_SIP4_RANGE_TABLE[%d] \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_EGRESS_SIP4TABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_EGRESS_DIP4TABLE) PROC_PRINTF("CF_DIP4_RANGE_TABLE[%d] \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_EGRESS_DIP4TABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_EGRESS_SIP6TABLE) PROC_PRINTF("CF_DSCP_TABLE[%d] \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_EGRESS_SIP6TABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_EGRESS_DIP6TABLE) PROC_PRINTF("CF_DSCP_TABLE[%d] \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_EGRESS_DIP6TABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_EGRESS_SPORTTABLE) PROC_PRINTF("CF_SPORT_RANGE_TABLE[%d] \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_EGRESS_SPORTTABLE_INDEX]);
			if(rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_EGRESS_DPORTTABLE) PROC_PRINTF("CF_DPORT_RANGE_TABLE[%d] \n",rg_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_EGRESS_DPORTTABLE_INDEX]);

			_dump_rg_acl_entry_content(s, aclPara);

#if 0
			//[FIXME] fwd_type can use name_of_string
			PROC_PRINTF(" fwd_type: %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction);
			PROC_PRINTF(" acl_weight: %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.acl_weight);


			PROC_PRINTF("[Patterns]: \n");
			PROC_PRINTF("filter_fields:0x%llx\n", aclPara->filter_fields);
			if(aclPara->filter_fields&INGRESS_PORT_BIT) PROC_PRINTF(" ingress_port_mask:0x%x\n", aclPara->ingress_port_mask.portmask);
			if(aclPara->filter_fields&INGRESS_EGRESS_PORTIDX_BIT) PROC_PRINTF(" ingress_port_idx:  %d  ingress_port_idx_mask:	%d\n", aclPara->ingress_port_idx,aclPara->ingress_port_idx_mask);
			if(aclPara->filter_fields&INGRESS_EGRESS_PORTIDX_BIT) PROC_PRINTF(" egress_port_idx:  %d  egress_port_idx_mask:	%d\n", aclPara->egress_port_idx,aclPara->egress_port_idx_mask);
			if(aclPara->filter_fields&INGRESS_DSCP_BIT) PROC_PRINTF(" ingress_dscp:	   %d\n", aclPara->ingress_dscp);
			if(aclPara->filter_fields&INGRESS_INTF_BIT) PROC_PRINTF(" ingress_intf_idx: %d\n", aclPara->ingress_intf_idx);
			if(aclPara->filter_fields&EGRESS_INTF_BIT) PROC_PRINTF(" egress_intf_idx:  %d\n", aclPara->egress_intf_idx);
			if(aclPara->filter_fields&INGRESS_STREAM_ID_BIT) PROC_PRINTF(" ingress_stream_id:  %d\n	ingress_stream_id_mask:  0x%x\n", aclPara->ingress_stream_id, aclPara->ingress_stream_id_mask);
			if(aclPara->filter_fields&INGRESS_ETHERTYPE_BIT) PROC_PRINTF(" ingress_ethertype:0x%x  ingress_ethertype_mask:0x%x\n", aclPara->ingress_ethertype, aclPara->ingress_ethertype_mask);
			if(aclPara->filter_fields&INGRESS_CTAG_VID_BIT) PROC_PRINTF(" ingress_ctag_vid: %d\n", aclPara->ingress_ctag_vid);
			if(aclPara->filter_fields&INGRESS_CTAG_PRI_BIT) PROC_PRINTF(" ingress_ctag_pri: %d\n", aclPara->ingress_ctag_pri);
			if(aclPara->filter_fields&INGRESS_CTAG_CFI_BIT) PROC_PRINTF(" ingress_ctag_cfi: %d\n", aclPara->ingress_ctag_cfi);
			if(aclPara->filter_fields&INGRESS_STAG_VID_BIT) PROC_PRINTF(" ingress_stag_vid: %d\n", aclPara->ingress_stag_vid);
			if(aclPara->filter_fields&INGRESS_STAG_PRI_BIT) PROC_PRINTF(" ingress_stag_pri: %d\n", aclPara->ingress_stag_pri);
			if(aclPara->filter_fields&INGRESS_STAG_DEI_BIT) PROC_PRINTF(" ingress_stag_dei: %d\n", aclPara->ingress_stag_dei);
			if(aclPara->filter_fields&INGRESS_SMAC_BIT) PROC_PRINTF(" ingress_smac: %02X:%02X:%02X:%02X:%02X:%02X ingress_smac_mask: %02X:%02X:%02X:%02X:%02X:%02X\n",
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
			if(aclPara->filter_fields&INGRESS_DMAC_BIT) PROC_PRINTF(" ingress_dmac: %02X:%02X:%02X:%02X:%02X:%02X  ingress_dmac_mask: %02X:%02X:%02X:%02X:%02X:%02X\n",
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

			acl_parameter = &rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter;
			if(aclPara->filter_fields&INGRESS_IPV4_SIP_RANGE_BIT){
				PROC_PRINTF(" ingress_sip_low_bound: %s	", inet_ntoa(acl_parameter->ingress_src_ipv4_addr_start));
				PROC_PRINTF(" ingress_sip_up_bound:	%s \n", inet_ntoa(acl_parameter->ingress_src_ipv4_addr_end));
			}
			if(aclPara->filter_fields&INGRESS_IPV4_DIP_RANGE_BIT){
				PROC_PRINTF(" ingress_dip_low_bound: %s	", inet_ntoa(acl_parameter->ingress_dest_ipv4_addr_start));
				PROC_PRINTF(" ingress_dip_up_bound:	%s \n", inet_ntoa(acl_parameter->ingress_dest_ipv4_addr_end));
			}

			if(aclPara->filter_fields&INGRESS_IPV6_SIP_RANGE_BIT){
				PROC_PRINTF(" ingress_src_ipv6_addr_start: %02X%02X:",acl_parameter->ingress_src_ipv6_addr_start[0],acl_parameter->ingress_src_ipv6_addr_start[1]);
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
				PROC_PRINTF(" ingress_dest_ipv6_addr_start: %02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[0],acl_parameter->ingress_dest_ipv6_addr_start[1]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[2],acl_parameter->ingress_dest_ipv6_addr_start[3]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[4],acl_parameter->ingress_dest_ipv6_addr_start[5]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[6],acl_parameter->ingress_dest_ipv6_addr_start[7]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[8],acl_parameter->ingress_dest_ipv6_addr_start[9]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[10],acl_parameter->ingress_dest_ipv6_addr_start[11]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_start[12],acl_parameter->ingress_dest_ipv6_addr_start[13]);
				PROC_PRINTF(" :%02X%02X: \n",acl_parameter->ingress_dest_ipv6_addr_start[14],acl_parameter->ingress_dest_ipv6_addr_start[15]);

				PROC_PRINTF(" ingress_dest_ipv6_addr_end: %02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[0],acl_parameter->ingress_dest_ipv6_addr_end[1]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[2],acl_parameter->ingress_dest_ipv6_addr_end[3]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[4],acl_parameter->ingress_dest_ipv6_addr_end[5]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[6],acl_parameter->ingress_dest_ipv6_addr_end[7]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[8],acl_parameter->ingress_dest_ipv6_addr_end[9]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[10],acl_parameter->ingress_dest_ipv6_addr_end[11]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_end[12],acl_parameter->ingress_dest_ipv6_addr_end[13]);
				PROC_PRINTF(" :%02X%02X: \n",acl_parameter->ingress_dest_ipv6_addr_end[14],acl_parameter->ingress_dest_ipv6_addr_end[15]);
			}

			if(aclPara->filter_fields&INGRESS_IPV6_SIP_BIT){
				PROC_PRINTF(" ingress_src_ipv6_addr: %02X%02X:",acl_parameter->ingress_src_ipv6_addr[0],acl_parameter->ingress_src_ipv6_addr[1]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr[2],acl_parameter->ingress_src_ipv6_addr[3]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr[4],acl_parameter->ingress_src_ipv6_addr[5]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr[6],acl_parameter->ingress_src_ipv6_addr[7]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr[8],acl_parameter->ingress_src_ipv6_addr[9]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr[10],acl_parameter->ingress_src_ipv6_addr[11]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr[12],acl_parameter->ingress_src_ipv6_addr[13]);
				PROC_PRINTF(" :%02X%02X: \n",acl_parameter->ingress_src_ipv6_addr[14],acl_parameter->ingress_src_ipv6_addr[15]);

				PROC_PRINTF(" ingress_src_ipv6_addr_mask: %02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[0],acl_parameter->ingress_src_ipv6_addr_mask[1]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[2],acl_parameter->ingress_src_ipv6_addr_mask[3]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[4],acl_parameter->ingress_src_ipv6_addr_mask[5]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[6],acl_parameter->ingress_src_ipv6_addr_mask[7]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[8],acl_parameter->ingress_src_ipv6_addr_mask[9]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[10],acl_parameter->ingress_src_ipv6_addr_mask[11]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_src_ipv6_addr_mask[12],acl_parameter->ingress_src_ipv6_addr_mask[13]);
				PROC_PRINTF(" :%02X%02X: \n",acl_parameter->ingress_src_ipv6_addr_mask[14],acl_parameter->ingress_src_ipv6_addr_mask[15]);
			}

			if(aclPara->filter_fields&INGRESS_IPV6_DIP_BIT){
				PROC_PRINTF(" ingress_dest_ipv6_addr: %02X%02X:",acl_parameter->ingress_dest_ipv6_addr[0],acl_parameter->ingress_dest_ipv6_addr[1]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[2],acl_parameter->ingress_dest_ipv6_addr[3]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[4],acl_parameter->ingress_dest_ipv6_addr[5]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[6],acl_parameter->ingress_dest_ipv6_addr[7]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[8],acl_parameter->ingress_dest_ipv6_addr[9]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[10],acl_parameter->ingress_dest_ipv6_addr[11]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr[12],acl_parameter->ingress_dest_ipv6_addr[13]);
				PROC_PRINTF(" :%02X%02X: \n",acl_parameter->ingress_dest_ipv6_addr[14],acl_parameter->ingress_dest_ipv6_addr[15]);

				PROC_PRINTF(" ingress_dest_ipv6_addr_mask: %02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[0],acl_parameter->ingress_dest_ipv6_addr_mask[1]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[2],acl_parameter->ingress_dest_ipv6_addr_mask[3]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[4],acl_parameter->ingress_dest_ipv6_addr_mask[5]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[6],acl_parameter->ingress_dest_ipv6_addr_mask[7]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[8],acl_parameter->ingress_dest_ipv6_addr_mask[9]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[10],acl_parameter->ingress_dest_ipv6_addr_mask[11]);
				PROC_PRINTF(" :%02X%02X:",acl_parameter->ingress_dest_ipv6_addr_mask[12],acl_parameter->ingress_dest_ipv6_addr_mask[13]);
				PROC_PRINTF(" :%02X%02X: \n",acl_parameter->ingress_dest_ipv6_addr_mask[14],acl_parameter->ingress_dest_ipv6_addr_mask[15]);
			}

			if(aclPara->filter_fields&INGRESS_L4_SPORT_RANGE_BIT){
				PROC_PRINTF(" ingress_src_l4_port_low_bound: %d	", acl_parameter->ingress_src_l4_port_start);
				PROC_PRINTF(" ingress_src_l4_port_up_bound:	%d \n", acl_parameter->ingress_src_l4_port_end);
			}
			if(aclPara->filter_fields&INGRESS_L4_DPORT_RANGE_BIT){
				PROC_PRINTF(" ingress_dest_l4_port_low_bound: %d  ", acl_parameter->ingress_dest_l4_port_start);
				PROC_PRINTF(" ingress_dest_l4_port_up_bound:  %d \n", acl_parameter->ingress_dest_l4_port_end);
			}

			if(aclPara->filter_fields&EGRESS_IPV4_SIP_RANGE_BIT){
				PROC_PRINTF(" egress_sip_low_bound: %s  ", diag_util_inet_ntoa(acl_parameter->egress_src_ipv4_addr_start));
				PROC_PRINTF(" egress_sip_up_bound:  %s \n", diag_util_inet_ntoa(acl_parameter->egress_src_ipv4_addr_end));
			}
			if(aclPara->filter_fields&EGRESS_IPV4_DIP_RANGE_BIT){
				PROC_PRINTF(" egress_dip_low_bound: %s  ", diag_util_inet_ntoa(acl_parameter->egress_dest_ipv4_addr_start));
				PROC_PRINTF(" egress_dip_up_bound:  %s \n", diag_util_inet_ntoa(acl_parameter->egress_dest_ipv4_addr_end));
			}
			if(aclPara->filter_fields&EGRESS_L4_SPORT_RANGE_BIT){
				PROC_PRINTF(" egress_src_l4_port_low_bound: %d  ", acl_parameter->egress_src_l4_port_start);
				PROC_PRINTF(" egress_src_l4_port_up_bound:  %d \n", acl_parameter->egress_src_l4_port_end);
			}
			if(aclPara->filter_fields&EGRESS_L4_DPORT_RANGE_BIT){
				PROC_PRINTF(" egress_dest_l4_port_low_bound: %d	", acl_parameter->egress_dest_l4_port_start);
				PROC_PRINTF(" egress_dest_l4_port_up_bound:	%d \n", acl_parameter->egress_dest_l4_port_end);
			}

			if(aclPara->filter_fields&EGRESS_CTAG_VID_BIT){
				PROC_PRINTF(" egress_ctag_vid:  %d \n", acl_parameter->egress_ctag_vid);
				PROC_PRINTF(" egress_ctag_vid_mask:	%d \n", acl_parameter->egress_ctag_vid_mask);
			}

			if(aclPara->filter_fields&EGRESS_CTAG_PRI_BIT) PROC_PRINTF("egress_ctag_pri:  %d \n", acl_parameter->egress_ctag_pri);


			if(acl_parameter->filter_fields & INGRESS_IPV4_TAGIF_BIT){
				PROC_PRINTF(" ipv4_tagIf: %s \n",acl_parameter->ingress_ipv4_tagif?"Must be IPv4":"Must not be IPv4");
			}

			if(acl_parameter->filter_fields & INGRESS_IPV6_TAGIF_BIT){
				PROC_PRINTF(" ipv6_tagIf: %s \n",acl_parameter->ingress_ipv6_tagif?"Must be IPv6":"Must not be IPv6");
			}

			if(acl_parameter->filter_fields & EGRESS_IP4MC_IF){
				PROC_PRINTF("egress_ip4mc_if: %s \n",acl_parameter->egress_ip4mc_if?"Must be IP4MC":"Must not be IP4MC");
			}
			if(acl_parameter->filter_fields & EGRESS_IP6MC_IF){
				PROC_PRINTF("egress_ip6mc_if: %s \n",acl_parameter->egress_ip6mc_if?"Must be IP6MC":"Must not be IP6MC");
			}

			if(acl_parameter->filter_fields & INGRESS_L4_TCP_BIT){
				PROC_PRINTF(" l4-protocal: tcp \n");
			}else if(acl_parameter->filter_fields & INGRESS_L4_UDP_BIT){
				PROC_PRINTF(" l4-protocal: udp \n");
			}else if(acl_parameter->filter_fields & INGRESS_L4_ICMP_BIT){
				PROC_PRINTF(" l4-protocal: icmp \n");
			}else{
				PROC_PRINTF(" l4-protocal: not care \n");
			}

			if(acl_parameter->filter_fields & INGRESS_L4_POROTCAL_VALUE_BIT){
				PROC_PRINTF(" l4-protocal-value: %d \n",acl_parameter->ingress_l4_protocal);
			}

			if(acl_parameter->filter_fields & INGRESS_STAGIF_BIT){
				if(aclPara->ingress_stagIf){
					PROC_PRINTF(" stagIf: must have Stag \n");
				}else{
					PROC_PRINTF(" stagIf: must not have Stag \n");
				}
			}
			if(acl_parameter->filter_fields & INGRESS_CTAGIF_BIT){
				if(aclPara->ingress_ctagIf){
					PROC_PRINTF(" ctagIf: must have Ctag \n");
				}else{
					PROC_PRINTF(" ctagIf: must not have Ctag \n");
				}
			}

			if(acl_parameter->filter_fields & INGRESS_WLANDEV_BIT){
				PROC_PRINTF(" ingress_wlanDevMask: 0x%x \n",acl_parameter->ingress_wlanDevMask);
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
					if(acl_parameter->qos_actions&ACL_ACTION_QUEUE_ID_BIT) PROC_PRINTF(" queue_id: %d\n",acl_parameter->action_queue_id);
					if(acl_parameter->qos_actions&ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT) PROC_PRINTF(" egress_internal_priority(cfpri): %d\n",acl_parameter->egress_internal_priority);
					if(acl_parameter->qos_actions&ACL_ACTION_SHARE_METER_BIT) PROC_PRINTF(" share_meter: %d\n",acl_parameter->action_share_meter);
					if(acl_parameter->qos_actions&ACL_ACTION_STREAM_ID_OR_LLID_BIT) PROC_PRINTF(" stream id: %d\n",acl_parameter->action_stream_id_or_llid);
					if(acl_parameter->qos_actions&ACL_ACTION_ACL_PRIORITY_BIT) PROC_PRINTF(" acl priority: %d\n",acl_parameter->action_acl_priority);
					if(acl_parameter->qos_actions&ACL_ACTION_ACL_INGRESS_VID_BIT) PROC_PRINTF(" ingress cvid: %d\n",acl_parameter->action_acl_ingress_vid);
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
				default:
					break;
			}
#endif
		}

	}
#endif
	if(rg_db.systemGlobal.aclSkipRearrangeHWAclCf==1){
		PROC_PRINTF("proc/rg/acl_skip_hw_rearrange is enabled, the H/W information may have no meaning");
	}

	return RT_ERR_RG_OK;
}

int _dump_rg_cf(struct seq_file *s)
{
	/*dump CF[64-511]*/

#if 1
	int i;
	rtk_rg_classifyEntry_t* cf_rule;
	for(i=rg_db.systemGlobal.cf_pattern0_size; i<TOTAL_CF_ENTRY_SIZE; i++){
		if(rg_db.systemGlobal.classify_SW_table_entry[i].index==i){//if empty, the index should be zero
			cf_rule = &rg_db.systemGlobal.classify_SW_table_entry[i];
			PROC_PRINTF("========cfRule[%d]=========\n",i);
			PROC_PRINTF("direction: %s \n",(cf_rule->direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM)?"upstream":"downstream");

			PROC_PRINTF("[patterns]: \n");

			if(cf_rule->filter_fields & EGRESS_ETHERTYPR_BIT)
				PROC_PRINTF("ETHERTYPR:0x%x \tMASK:0x%x \n",cf_rule->etherType,cf_rule->etherType_mask);
			if(cf_rule->filter_fields & EGRESS_GEMIDX_BIT)
				PROC_PRINTF("GEMIDX:%d \tMASK:0x%x \n",cf_rule->gemidx,cf_rule->gemidx_mask);
			if(cf_rule->filter_fields & EGRESS_LLID_BIT)
				PROC_PRINTF("LLID:%d \n",cf_rule->llid);
			if(cf_rule->filter_fields & EGRESS_TAGVID_BIT)
				PROC_PRINTF("TAGVID:%d \n",cf_rule->outterTagVid);
			if(cf_rule->filter_fields & EGRESS_TAGPRI_BIT)
				PROC_PRINTF("TAGPRI:%d \n",cf_rule->outterTagPri);
			if(cf_rule->filter_fields & EGRESS_INTERNALPRI_BIT)
				PROC_PRINTF("INTERNALPRI:%d \n",cf_rule->internalPri);
			if(cf_rule->filter_fields & EGRESS_STAGIF_BIT)
				PROC_PRINTF("STAGIF:%s \n",cf_rule->stagIf?"must have Stag":"must not have Stag");
			if(cf_rule->filter_fields & EGRESS_CTAGIF_BIT)
				PROC_PRINTF("CTAGIF:%s \n",cf_rule->ctagIf?"must have Ctag":"must not have Ctag");
			if(cf_rule->filter_fields & EGRESS_UNI_BIT)
				PROC_PRINTF("UNI:%d \tMASK:0x%x \n",cf_rule->uni,cf_rule->uni_mask);


			PROC_PRINTF("[valid actions]: \n");
			if(cf_rule->direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM){//upstream
				if(cf_rule->us_action_field & CF_US_ACTION_STAG_BIT) PROC_PRINTF("SVLAN \n");
				if(cf_rule->us_action_field & CF_US_ACTION_CTAG_BIT) PROC_PRINTF("CVLAN \n");
				if(cf_rule->us_action_field & CF_US_ACTION_CFPRI_BIT) PROC_PRINTF("CFPRI \n");
				if(cf_rule->us_action_field & CF_US_ACTION_DSCP_BIT) PROC_PRINTF("DSCP \n");
				if(cf_rule->us_action_field & CF_US_ACTION_SID_BIT) PROC_PRINTF("SID \n");
				if(cf_rule->us_action_field & CF_US_ACTION_DROP_BIT) PROC_PRINTF("DROP \n");
				if(cf_rule->us_action_field & CF_US_ACTION_LOG_BIT) PROC_PRINTF("LOG \n");

			}else{//downstream
				if(cf_rule->ds_action_field & CF_DS_ACTION_STAG_BIT) PROC_PRINTF("SVLAN \n");
				if(cf_rule->ds_action_field & CF_DS_ACTION_CTAG_BIT) PROC_PRINTF("CVLAN \n");
				if(cf_rule->ds_action_field & CF_DS_ACTION_CFPRI_BIT) PROC_PRINTF("CFPRI \n");
				if(cf_rule->ds_action_field & CF_DS_ACTION_DSCP_BIT) PROC_PRINTF("DSCP \n");
				if(cf_rule->ds_action_field & CF_DS_ACTION_UNI_MASK_BIT) PROC_PRINTF("UNI \n");
				if(cf_rule->ds_action_field & CF_DS_ACTION_DROP_BIT) PROC_PRINTF("DROP \n");
			}

		}
	}
#endif


	return RT_ERR_RG_OK;
}

int32 _dump_acl_template(struct seq_file *s)
{
    int i;
    rtk_acl_template_t aclTemplate;

    PROC_PRINTF("--------------- ACL TEMPLATES(API VALUE) ----------------\n");
	for(i=0; i<8; i++)
    {
        memset(&aclTemplate,0,sizeof(aclTemplate));
        aclTemplate.index=i;
        rtk_acl_template_get(&aclTemplate);
        PROC_PRINTF("\tTEMPLATE[%d]: [0:0x%x][1:0x%x][2:0x%x][3:0x%x][4:0x%x][5:0x%x][6:0x%x][7:0x%x]\n",i,aclTemplate.fieldType[0],aclTemplate.fieldType[1],aclTemplate.fieldType[2],aclTemplate.fieldType[3],aclTemplate.fieldType[4],aclTemplate.fieldType[5],aclTemplate.fieldType[6],aclTemplate.fieldType[7]);
    }
    return 0;
}

int _dump_acl(struct seq_file *s)
{
	int i,j;
	uint32 val;
	rtk_acl_ingress_entry_t aclRule;
	char* actionString;

	i=0;j=0;val=0;

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
					reg_array_field_read(RTL9602C_ACL_TEMPLATE_CTRLr, aclRule.templateIdx,j, RTL9602C_FIELDf, &val);
					PROC_PRINTF("{0x%02x:%s}\n",val&0x7f,name_of_acl_field[val&0x7f]);
				}
			}

			PROC_PRINTF("\tactive portmask:0x%x\n",aclRule.activePorts.bits[0]);

			if(aclRule.careTag.tags[6].mask) tag_care|=(1<<6);
			if(aclRule.careTag.tags[5].mask) tag_care|=(1<<5);
			if(aclRule.careTag.tags[4].mask) tag_care|=(1<<4);
			if(aclRule.careTag.tags[3].mask) tag_care|=(1<<3);
			if(aclRule.careTag.tags[2].mask) tag_care|=(1<<2);
			if(aclRule.careTag.tags[1].mask) tag_care|=(1<<1);
			if(aclRule.careTag.tags[0].mask) tag_care|=(1<<0);
			if(tag_care)
			{
				PROC_PRINTF("\ttag_care:");
				if(tag_care&(1<<6))PROC_PRINTF("%s",aclRule.careTag.tags[6].value?"[UDP:O]":"[UDP:X]");
				if(tag_care&(1<<5))PROC_PRINTF("%s",aclRule.careTag.tags[5].value?"[TCP:O]":"[TCP:X]");
				if(tag_care&(1<<4))PROC_PRINTF("%s",aclRule.careTag.tags[4].value?"[IPv6:O]":"[IPv6:X]");
				if(tag_care&(1<<3))PROC_PRINTF("%s",aclRule.careTag.tags[3].value?"[IPv4:O]":"[IPv4:X]");
				if(tag_care&(1<<2))PROC_PRINTF("%s",aclRule.careTag.tags[2].value?"[Stag:O]":"[Stag:X]");
				if(tag_care&(1<<1))PROC_PRINTF("%s",aclRule.careTag.tags[1].value?"[Ctag:O]":"[Ctag:X]");
				if(tag_care&(1<<0))PROC_PRINTF("%s",aclRule.careTag.tags[0].value?"[PPPoE:O]":"[PPPoE:X]");
				PROC_PRINTF("\n");
			}

			PROC_PRINTF("\ttemplateIdx:%x\n",aclRule.templateIdx);

			if(aclRule.templateIdx!=7)
			{
				PROC_PRINTF("\taction bits:");

				if(aclRule.act.enableAct[6]) PROC_PRINTF("[POLICY ROUTE]");
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
						case ACL_IGR_SVLAN_1P_REMARK_ACT:  actionString="1P remark"; break;
						case ACL_IGR_SVLAN_DSCP_REMARK_ACT:  actionString="DSCP remark"; break;
						case ACL_IGR_SVLAN_ROUTE_ACT:  actionString="policy Route(not support in apolloFE)"; break;
						case ACL_IGR_SVLAN_BW_METER_ACT:  actionString="Bandwidth Metering"; break;
						default: actionString="unKnown action"; break;
					}
					PROC_PRINTF("\t[SVLAN_ACTIDX:%x(%s)] svid:%d dot1p:%x dscp:%d nexthop:%x\n",aclRule.act.svlanAct.act,actionString,aclRule.act.svlanAct.svid,aclRule.act.svlanAct.dot1p,aclRule.act.svlanAct.dscp,aclRule.act.svlanAct.nexthop);
				}

				if(aclRule.act.enableAct[4])
				{
					switch(aclRule.act.forwardAct.act)
					{
						case ACL_IGR_FORWARD_COPY_ACT: actionString="Forward frame with ACLPMSK only (& filtering)"; break;
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
						case ACL_IGR_PRI_DSCP_REMARK_ACT: actionString="DSCP Remarking";break;
						case ACL_IGR_PRI_1P_REMARK_ACT: actionString="1P Remarking";break;
						case ACL_IGR_PRI_POLICING_ACT: actionString="Policing";break;
						case ACL_IGR_PRI_MIB_ACT: actionString="Logging";break;
						case ACL_IGR_PRI_ROUTE_ACT: actionString="policy Route(not support in apolloFE)";break;
						case ACL_IGR_PRI_BW_METER_ACT: actionString="Bandwidth Metering";break;
						default: actionString="unKnown action"; break;
					}
					PROC_PRINTF("\t[PRI_ACTIDX:%x(%s)] aclPri:%x dot1p:%x dscp:%d nexthop:%x\n",aclRule.act.priAct.act,actionString,aclRule.act.priAct.aclPri,aclRule.act.priAct.dot1p,aclRule.act.priAct.dscp,aclRule.act.priAct.nexthop);
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

				if(aclRule.act.enableAct[6])
				{
					switch(aclRule.act.routeAct.act)
					{
						 case ACL_IGR_ROUTE_ROUTE_ACT: actionString="POlicy route"; break;
   						 case ACL_IGR_ROUTE_1P_REMARK_ACT: actionString="1P remark"; break;
						 default: actionString="unKnown action"; break;
					}

					PROC_PRINTF("\t[ROUTE:%x(%s)] Nexthop:%d dot1p:%d \n",aclRule.act.routeAct.act,actionString,aclRule.act.routeAct.nexthop,aclRule.act.routeAct.dot1p);
				}
			}
			else
			{
				PROC_PRINTF("\taction bits: (CF latch to ACL) \n");
			}

	   }
	}


	//use the ASIC API
	PROC_PRINTF("--------------- ACL HIT OINFO----------------\n");

	for(i=0; i<7; i++)
	{
		reg_array_field_read(RTL9602C_STAT_ACL_REASONr,REG_ARRAY_INDEX_NONE,i, RTL9602C_ACL_HIT_INFOf, &val);
		switch(i)
		{
		case 0:
			PROC_PRINTF("%-11s %-3s index %d\n","[CACT]",((val&0x7F)!=0x7F)?"yes":"no",val&0x7f);
			break;
		case 1:
			PROC_PRINTF("%-11s %-3s index %d\n","[SACT]",((val&0x7F)!=0x7F)?"yes":"no",val&0x7f);
			break;
		case 2:
			PROC_PRINTF("%-11s %-3s index %d\n","[PRI]",((val&0x7F)!=0x7F)?"yes":"no",val&0x7f);
			break;
		case 3:
			PROC_PRINTF("%-11s %-3s index %d\n","[POLICE]",((val&0x7F)!=0x7F)?"yes":"no",val&0x7f);
			break;
		case 4:
			PROC_PRINTF("%-11s %-3s index %d\n","[FWD]",((val&0x7F)!=0x7F)?"yes":"no",val&0x7f);
			break;
		case 5:
			PROC_PRINTF("%-11s %-3s index %d\n","[EXT]",((val&0x7F)!=0x7F)?"yes":"no",val&0x7f);
			break;
		case 6:
			PROC_PRINTF("%-11s %-3s index %d\n","[ROUTE]",((val&0x7F)!=0x7F)?"yes":"no",val&0x7f);
			break;

		}
	}

	if(rg_db.systemGlobal.aclSkipRearrangeHWAclCf==1){
		PROC_PRINTF("proc/rg/acl_skip_hw_rearrange is enabled, the H/W ACL information may have no meaning");
	}

	return RT_ERR_RG_OK;
}


int _dump_cf(struct seq_file *s)
{
	int i;
	uint32 arrayIndex,arraySize=0;
	uint32 value=0;
	rtk_classify_cfg_t classifyCfg;
	char *decisionString, *vidDecisionString, *priDecisionString;

	PROC_PRINTF("------------ CF TABLE -------------\n");
	for(i=0; i<rg_db.systemGlobal.cf_pattern0_size; i++) //only list pattern0
	{
		memset(&classifyCfg,0,sizeof(classifyCfg));
		classifyCfg.index=i;
		rtk_classify_cfgEntry_get(&classifyCfg);
		if(classifyCfg.valid)
		{
			PROC_PRINTF("  ------ CF [%d] ------\n",i);
			//PROC_PRINTF("\tvalid:%x\n",classifyCfg.valid);

			//U/D
			PROC_PRINTF("\tDIRECTION: %s\n",classifyCfg.direction?"DOWNSTREAM":"UPSTREAM");

			//PROC_PRINTF("\tfield[2]:%x mask:%x\n",classifyCfg.field.readField.dataFieldRaw[2],classifyCfg.field.readField.careFieldRaw[2]);
			//PROC_PRINTF("\tfield[1]:%x mask:%x\n",classifyCfg.field.readField.dataFieldRaw[1],classifyCfg.field.readField.careFieldRaw[1]);
			//PROC_PRINTF("\tfield[0]:%x mask:%x\n",classifyCfg.field.readField.dataFieldRaw[0],classifyCfg.field.readField.careFieldRaw[0]);


			//wan_intf
			if((classifyCfg.field.readField.careFieldRaw[2]>>12)&0xf)
				PROC_PRINTF("\twan_intf:%d (Mask 0x%x)\n", ((classifyCfg.field.readField.dataFieldRaw[2]>>12)&0xf),(classifyCfg.field.readField.careFieldRaw[2]>>12)&0xf);

			//pppoe_if
			if((classifyCfg.field.readField.careFieldRaw[2]>>11)&0x1)
				PROC_PRINTF("\tpppoe_if:%d (Mask 0x%x)\n", ((classifyCfg.field.readField.dataFieldRaw[2]>>11)&0x1),(classifyCfg.field.readField.careFieldRaw[2]>>11)&0x1);

			//ipv4_if
			if((classifyCfg.field.readField.careFieldRaw[2]>>10)&0x1)
				PROC_PRINTF("\tipv4_if:%d (Mask 0x%x)\n", ((classifyCfg.field.readField.dataFieldRaw[2]>>10)&0x1),(classifyCfg.field.readField.careFieldRaw[2]>>10)&0x1);

			//ipv6_if
			if((classifyCfg.field.readField.careFieldRaw[2]>>9)&0x1)
				PROC_PRINTF("\tipv6_if:%d (Mask 0x%x)\n", ((classifyCfg.field.readField.dataFieldRaw[2]>>9)&0x1),(classifyCfg.field.readField.careFieldRaw[2]>>9)&0x1);

			//ipmc_if
			if((classifyCfg.field.readField.careFieldRaw[2]>>8)&0x1)
				PROC_PRINTF("\tipmc_if:%d (Mask 0x%x)\n", ((classifyCfg.field.readField.dataFieldRaw[2]>>8)&0x1),(classifyCfg.field.readField.careFieldRaw[2]>>8)&0x1);


			//igmp_mld_if
			if((classifyCfg.field.readField.careFieldRaw[2]>>7)&0x1)
				PROC_PRINTF("\tigmp_mld_if:%d (Mask 0x%x)\n", ((classifyCfg.field.readField.dataFieldRaw[2]>>7)&0x1),(classifyCfg.field.readField.careFieldRaw[2]>>7)&0x1);

			//acl_latch_to_cf
			if((classifyCfg.field.readField.careFieldRaw[2])&0x7f)
				PROC_PRINTF("\tacl_latch_to_cf:%d (Mask 0x%x)\n", ((classifyCfg.field.readField.dataFieldRaw[2])&0x7f),(classifyCfg.field.readField.careFieldRaw[2])&0x7f);

			//int_vid_tagif
			if((classifyCfg.field.readField.careFieldRaw[1]>>14)&0x1)
				PROC_PRINTF("\tint_vid_tagif:%d (Mask 0x%x)\n", ((classifyCfg.field.readField.dataFieldRaw[1]>>14)&0x1),(classifyCfg.field.readField.careFieldRaw[1]>>14)&0x1);

			//cf_latch_to_acl
			if((classifyCfg.field.readField.careFieldRaw[1]>>7)&0x7f)
				PROC_PRINTF("\tcf_latch_to_acl:%d (Mask 0x%x)\n", ((classifyCfg.field.readField.dataFieldRaw[1]>>7)&0x7f),(classifyCfg.field.readField.careFieldRaw[1]>>7)&0x7f);

			//vid_cfg
			if( ((classifyCfg.field.readField.careFieldRaw[1]&0x3f)<<5)|((classifyCfg.field.readField.careFieldRaw[0]>>11)&0x1f)){
				PROC_PRINTF("\tvid_cfg[%s]:%d (Mask 0x%x)\n",
					classifyCfg.direction?(/*downstream*/name_of_ds_vid_cfg[classifyCfg.templateIdx] ):(/*upstream*/name_of_us_vid_cfg[classifyCfg.templateIdx]),
					(((classifyCfg.field.readField.dataFieldRaw[1]&0x3f)<<5)|((classifyCfg.field.readField.dataFieldRaw[0]>>11)&0x1f)),
					(((classifyCfg.field.readField.careFieldRaw[1]&0x3f)<<5)|((classifyCfg.field.readField.careFieldRaw[0]>>11)&0x1f)));
			}

			//tag_pri
			if((classifyCfg.field.readField.careFieldRaw[0]>>8)&0x7)
				PROC_PRINTF("\ttag_pri:%d (Mask 0x%x)\n", ((classifyCfg.field.readField.dataFieldRaw[0]>>8)&0x7),(classifyCfg.field.readField.careFieldRaw[0]>>8)&0x7);


			//pri_cfg
			if((classifyCfg.field.readField.careFieldRaw[0]>>5)&0x7)
			{
				PROC_PRINTF("\tpri_cfg[%s]:%d (Mask 0x%x)\n",
					classifyCfg.direction?(/*downstream*/name_of_ds_pri_cfg[classifyCfg.templateIdx]):(/*upstream*/name_of_us_pri_cfg[classifyCfg.templateIdx]),
				((classifyCfg.field.readField.dataFieldRaw[0]>>5)&0x7),
				(classifyCfg.field.readField.careFieldRaw[0]>>5)&0x7);
			}

			//stag_if
			if((classifyCfg.field.readField.careFieldRaw[0]>>4)&0x1)
				PROC_PRINTF("\tstag_if:%d (Mask 0x%x)\n", ((classifyCfg.field.readField.dataFieldRaw[0]>>4)&0x1),(classifyCfg.field.readField.careFieldRaw[0]>>4)&0x1);

			//ctag_if
			if((classifyCfg.field.readField.careFieldRaw[0]>>3)&0x1)
				PROC_PRINTF("\tctag_if:%d (Mask 0x%x)\n", ((classifyCfg.field.readField.dataFieldRaw[0]>>3)&0x1),(classifyCfg.field.readField.careFieldRaw[0]>>3)&0x1);

			//uni
			if((classifyCfg.field.readField.careFieldRaw[0])&CF_UNI_DEFAULT)
				PROC_PRINTF("\tuni:%d (Mask 0x%x)\n", ((classifyCfg.field.readField.dataFieldRaw[0])&CF_UNI_DEFAULT),(classifyCfg.field.readField.careFieldRaw[0])&CF_UNI_DEFAULT);

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
					PROC_PRINTF("\t[DSCP]:%s \n\t\tDSCP:%d \n",
					decisionString,
					classifyCfg.act.dsAct.dscpVal);
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
					PROC_PRINTF("\t[DSCP]:%s \tDSCP:%d \n",
					decisionString,
					classifyCfg.act.usAct.dscpVal);
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
		}
	}


	for(i=rg_db.systemGlobal.cf_pattern0_size; i<TOTAL_CF_ENTRY_SIZE; i++) //list pattern1
	{
		memset(&classifyCfg,0,sizeof(classifyCfg));
		classifyCfg.index=i;
		rtk_classify_cfgEntry_get(&classifyCfg);
		if(classifyCfg.valid)
		{

			//patterns
			PROC_PRINTF("  ------ CF [%d] ------\n",i);
			//PROC_PRINTF("\tvalid:%x\n",classifyCfg.valid);
			if(classifyCfg.templateIdx==1)
			{
				PROC_PRINTF("RG not support CF pattern_1 with template[1]");
				continue;
			}

			//U/D
			PROC_PRINTF("\tDIRECTION: %s\n",classifyCfg.direction?"DOWNSTREAM":"UPSTREAM");

			//ether type
			if((classifyCfg.field.readField.careFieldRaw[2])&0xffff)
				PROC_PRINTF("\tether_type:0x%x (Mask 0x%x)\n", ((classifyCfg.field.readField.dataFieldRaw[2])&0xffff),(classifyCfg.field.readField.careFieldRaw[2])&0xffff);

			//tos/tc/gemidx/llid
			if((classifyCfg.field.readField.careFieldRaw[1]>>7)&0xff)
				PROC_PRINTF("\ttos_tc_gemidx_llid:%d (Mask 0x%x)\n", ((classifyCfg.field.readField.dataFieldRaw[1]>>7)&0xff),(classifyCfg.field.readField.careFieldRaw[1]>>7)&0xff);


			//outter_vid
			if( ((classifyCfg.field.readField.careFieldRaw[1]&0x3f)<<5)|((classifyCfg.field.readField.careFieldRaw[0]>>11)&0x1f)){
				PROC_PRINTF("\toutter_vid:%d (Mask 0x%x)\n",
					(((classifyCfg.field.readField.dataFieldRaw[1]&0x3f)<<5)|((classifyCfg.field.readField.dataFieldRaw[0]>>11)&0x1f)),
					(((classifyCfg.field.readField.careFieldRaw[1]&0x3f)<<5)|((classifyCfg.field.readField.careFieldRaw[0]>>11)&0x1f)));
			}

			//outter_pri
			if((classifyCfg.field.readField.careFieldRaw[0]>>8)&0x7)
				PROC_PRINTF("\toutter_pri:%d (Mask 0x%x)\n", ((classifyCfg.field.readField.dataFieldRaw[0]>>8)&0x7),(classifyCfg.field.readField.careFieldRaw[0]>>8)&0x7);


			//int_pri
			if((classifyCfg.field.readField.careFieldRaw[0]>>5)&0x7)
			{
				PROC_PRINTF("\tint_pri:%d (Mask 0x%x)\n",((classifyCfg.field.readField.dataFieldRaw[0]>>5)&0x7),(classifyCfg.field.readField.careFieldRaw[0]>>5)&0x7);
			}

			//stag_if
			if((classifyCfg.field.readField.careFieldRaw[0]>>4)&0x1)
				PROC_PRINTF("\tstag_if:%d (Mask 0x%x)\n", ((classifyCfg.field.readField.dataFieldRaw[0]>>4)&0x1),(classifyCfg.field.readField.careFieldRaw[0]>>4)&0x1);

			//ctag_if
			if((classifyCfg.field.readField.careFieldRaw[0]>>3)&0x1)
				PROC_PRINTF("\tctag_if:%d (Mask 0x%x)\n", ((classifyCfg.field.readField.dataFieldRaw[0]>>3)&0x1),(classifyCfg.field.readField.careFieldRaw[0]>>3)&0x1);

			//uni
			if((classifyCfg.field.readField.careFieldRaw[0])&CF_UNI_DEFAULT)
				PROC_PRINTF("\tuni:%d (Mask 0x%x)\n", ((classifyCfg.field.readField.dataFieldRaw[0])&CF_UNI_DEFAULT),(classifyCfg.field.readField.careFieldRaw[0])&CF_UNI_DEFAULT);


			//actions
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
					PROC_PRINTF("\t[DSCP]:%s \n\t\tDSCP:%d \n",
					decisionString,
					classifyCfg.act.dsAct.dscpVal);
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
					PROC_PRINTF("\t[DSCP]:%s \tDSCP:%d \n",
					decisionString,
					classifyCfg.act.usAct.dscpVal);
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

		}
	}


#if 1
	PROC_PRINTF("--------------- CF HIT INFO----------------\n");
	/*For CF pattern 0 hit check*/
	PROC_PRINTF("Pattern 0: \n");
	PROC_PRINTF(" Action	   Hit Index\n");
	arraySize = 6;
	for(arrayIndex = 0; arrayIndex<arraySize; arrayIndex++)
	{
		reg_array_field_read(RTL9602C_STAT_CF_PTN0_REASONr, REG_ARRAY_INDEX_NONE, arrayIndex, RTL9602C_CF_PTN0_HIT_INFOf, &value);
		PROC_PRINTF(" %-11s %-3s %-d\n", diagStr_cfActBit[arrayIndex], (value & 0x100)? "yes":"no", value & 0xff);
	}
#endif

	if(rg_db.systemGlobal.aclSkipRearrangeHWAclCf==1){
		PROC_PRINTF("proc/rg/acl_skip_hw_rearrange is enabled, the H/W CF information may have no meaning");
	}

	return RT_ERR_RG_OK;
}

int _rtk_rg_check_acl_committed_rule(rtk_acl_igr_log_act_ctrl_t mode)
{
	int i, ret = 0;
    rtk_rg_aclFilterEntry_t aclSWEntry;

	for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
	{
	    if(_rtk_rg_aclSWEntry_get(i,&aclSWEntry))
	        RETURN_ERR(RT_ERR_RG_ACL_SW_ENTRY_ACCESS_FAILED);
		if( ((mode == ACL_IGR_LOG_MIB_ACT) && (aclSWEntry.acl_filter.qos_actions & ACL_ACTION_LOG_COUNTER_BIT)) ||
			( (mode == ACL_IGR_LOG_POLICING_ACT) && (aclSWEntry.acl_filter.qos_actions & ACL_ACTION_SHARE_METER_BIT) )
		)
			ret = 1;
	}

	return ret;
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
		}
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_CTAG_VID_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_ctag_vid %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_ctag_vid);
			PROC_PRINTF("rg set acl-filter pattern egress_ctag_vid_mask %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_ctag_vid_mask);
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
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_INTF_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_intf_idx %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_intf_idx);
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
			}
			else if(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP||
				rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN ||
				rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP ||
				rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT)
			{
				PROC_PRINTF("rg set acl-filter pattern ingress_port_idx %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_port_idx);
				PROC_PRINTF("rg set acl-filter pattern ingress_port_idx_mask 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_port_idx_mask);
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
		}


		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_L4_SPORT_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_src_l4_port_start %d egress_src_l4_port_end %d\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_l4_port_start,
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_l4_port_end);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_L4_DPORT_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_dest_l4_port_start %d egress_dest_l4_port_end %d\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_l4_port_start,
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_l4_port_end);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_IP4MC_IF)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_ip4mc_if %d\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_ip4mc_if);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_IP6MC_IF)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_ip6mc_if %d\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_ip6mc_if);
		}

		//pattern: ingress
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV4_TAGIF_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ipv4_tagif %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ipv4_tagif);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_TAGIF_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ipv6_tagif %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ipv6_tagif);
		}
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_CTAG_PRI_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ctag_pri %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ctag_pri);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_CTAG_VID_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ctag_vid %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ctag_vid);
		}
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_CTAG_CFI_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ctag_cfi %d set\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ctag_cfi);
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
			}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_DIP_BIT)!=0)	//mask for IPv6; all zero = all "1"(all mask)
		{
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
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_DMAC_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_dmac_mask %x:%x:%x:%x:%x:%x\n",
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[0],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[1],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[2],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[3],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[4],
			rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[5]);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_DSCP_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_dscp %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dscp);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_ETHERTYPE_BIT)!=0){
			PROC_PRINTF("rg set acl-filter pattern ingress_ethertype 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ethertype);
			PROC_PRINTF("rg set acl-filter pattern ingress_ethertype_mask 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ethertype_mask);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_INTF_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_intf_idx %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_intf_idx);
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_DSCP_BIT)!=0)
			PROC_PRINTF("rg set acl-filter pattern ingress_ipv6_dscp %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ipv6_dscp);

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_UDP_BIT)!=0)
			PROC_PRINTF("rg set acl-filter pattern ingress_l4_protocal 0\n");

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_TCP_BIT)!=0)
			PROC_PRINTF("rg set acl-filter pattern ingress_l4_protocal 1\n");

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_ICMP_BIT)!=0)
			PROC_PRINTF("rg set acl-filter pattern ingress_l4_protocal 2\n");

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_NONE_TCP_NONE_UDP_BIT)!=0)
			PROC_PRINTF("rg set acl-filter pattern ingress_l4_protocal 3\n");

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_ICMPV6_BIT)!=0)
			PROC_PRINTF("rg set acl-filter pattern ingress_l4_protocal 4\n");

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_POROTCAL_VALUE_BIT)!=0)
			PROC_PRINTF("rg set acl-filter pattern ingress_l4_protocal_value 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_l4_protocal);

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_PORT_BIT)!=0)
			PROC_PRINTF("rg set acl-filter pattern ingress_port_mask 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_port_mask.portmask);

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_SMAC_BIT)!=0){
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
		}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV4_SIP_RANGE_BIT)!=0)
			PROC_PRINTF("rg set acl-filter pattern ingress_src_ipv4_addr_start %d.%d.%d.%d ingress_src_ipv4_addr_end %d.%d.%d.%d\n",
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_start & 0xff000000)>>24,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_start & 0xff0000)>>16,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_start & 0xff00)>>8,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_start & 0xff),
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_end & 0xff000000)>>24,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_end & 0xff0000)>>16,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_end & 0xff00)>>8,
			(rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_end & 0xff));

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_SIP_BIT)!=0){
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
				}

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_SIP_RANGE_BIT)!=0)
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


		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_SPORT_RANGE_BIT)!=0)
			PROC_PRINTF("rg set acl-filter pattern ingress_src_l4_port_start %d ingress_src_l4_port_end %d\n",
		rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_l4_port_start,
		rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_l4_port_end);

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_DPORT_RANGE_BIT)!=0)
			PROC_PRINTF("rg set acl-filter pattern ingress_dest_l4_port_start %d ingress_dest_l4_port_end %d\n",
		rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_l4_port_start,
		rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_l4_port_end);


		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_STAGIF_BIT)!=0)
			PROC_PRINTF("rg set acl-filter pattern ingress_stagIf %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stagIf);

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_CTAGIF_BIT)!=0)
			PROC_PRINTF("rg set acl-filter pattern ingress_ctagIf %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ctagIf);

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_STAG_PRI_BIT)!=0)
			PROC_PRINTF("rg set acl-filter pattern ingress_stag_pri %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stag_pri);

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_STAG_VID_BIT)!=0)
			PROC_PRINTF("rg set acl-filter pattern ingress_stag_vid %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stag_vid);
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_STAG_DEI_BIT)!=0)
			PROC_PRINTF("rg set acl-filter pattern ingress_stag_dei %d set\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stag_dei);

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_STREAM_ID_BIT)!=0){
			PROC_PRINTF("rg set acl-filter pattern ingress_stream_id %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stream_id);
			PROC_PRINTF("rg set acl-filter pattern ingress_stream_id_mask 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stream_id_mask);
		}
		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_WLANDEV_BIT)!=0)
			PROC_PRINTF("rg set acl-filter pattern ingress_wlanDevMask 0x%x\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_wlanDevMask);

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INTERNAL_PRI_BIT)!=0)
			PROC_PRINTF("rg set acl-filter pattern internal_pri %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.internal_pri);

		if((rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_PKT_LEN_RANGE_BIT)!=0)
			PROC_PRINTF("rg set acl-filter pattern ingress_packet_length_start %d ingress_packet_length_end %d\n",rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_packet_length_start, rg_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_packet_length_end);

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


char *name_of_acl_iprange_type[]={
	"UNUSED",
	"IPV4_SIP",
	"IPV4_DIP",
	"IPV6_SIP",
	"IPV6_DIP",
};
int32 _dump_acl_ipRangeTable(struct seq_file *s)
{
    int i;
    rtk_acl_rangeCheck_ip_t ipRangeEntry;

    PROC_PRINTF("------------ ACL IP RANGE TABLES -------------\n");
    for(i=0; i<32; i++)
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
    for(i=0; i<32; i++)
    {
        memset(&portRangeEntry,0,sizeof(portRangeEntry));
        portRangeEntry.index=i;
        rtk_acl_portRange_get(&portRangeEntry);
        PROC_PRINTF("\tPORTRANGE[%d] upper:%d lower:%d type:%s\n",i,portRangeEntry.upper_bound,portRangeEntry.lower_bound,name_of_acl_portrange_type[portRangeEntry.type]);
    }

	return RT_ERR_RG_OK;
}







