#include <rt_edp_acl.h>
#include <rt_edp_debug.h>
#include <rt_edp_internal.h>

#if defined(EDP_ACL_SUPPORT_HOOK_CHECK) && (EDP_ACL_SUPPORT_HOOK_CHECK==1)
#include <rt_edp_glue_struct.h>	//rt_edp_skb_t
#include <linux/if_vlan.h>	//vlan_dev_priv
#include <linux/ip.h>		//ip_hdr
#include <linux/ipv6.h>		//ipv6_hdr
#include <net/dsfield.h>	//ipv4_change_dsfield
#include <net/inet_ecn.h>	//ipv4_copy_dscp
#endif	//EDP_ACL_SUPPORT_HOOK_CHECK

struct table_info linux_table[] = {
    {RT_EDP_ACL_USED_IPTABLES_MANGLE_INPUT_IDX,		EDP_IPTABLES_UTILITY,	EDP_ACL_XTABLES_MANGLE,		EDP_ACL_XTABLES_INPUT},
    {RT_EDP_ACL_USED_IPTABLES_MANGLE_OUTPUT_IDX,	EDP_IPTABLES_UTILITY,	EDP_ACL_XTABLES_MANGLE,		EDP_ACL_XTABLES_OUTPUT},
    {RT_EDP_ACL_USED_IPTABLES_MANGLE_FORWARD_IDX,	EDP_IPTABLES_UTILITY,	EDP_ACL_XTABLES_MANGLE,		EDP_ACL_XTABLES_FORWARD},
    {RT_EDP_ACL_USED_IP6TABLES_MANGLE_INPUT_IDX,	EDP_IP6TABLES_UTILITY,	EDP_ACL_XTABLES_MANGLE,		EDP_ACL_XTABLES_INPUT},
    {RT_EDP_ACL_USED_IP6TABLES_MANGLE_OUTPUT_IDX,	EDP_IP6TABLES_UTILITY,	EDP_ACL_XTABLES_MANGLE,		EDP_ACL_XTABLES_OUTPUT},
	{RT_EDP_ACL_USED_IP6TABLES_MANGLE_FORWARD_IDX,	EDP_IP6TABLES_UTILITY,	EDP_ACL_XTABLES_MANGLE, 	EDP_ACL_XTABLES_FORWARD},
    {RT_EDP_ACL_USED_EBTABLES_FILTER_FORWARD_IDX,	EDP_EBTABLES_UTILITY,	EDP_ACL_EBTABLES_FILTER,	EDP_ACL_EBTABLES_FORWARD},
};

struct linux_table_info linux_table_map[] = {
    {RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL_IDX,		"all"},
    {RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_FILTER_IDX,	EDP_ACL_XTABLES_FILTER},
    {RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_NAT_IDX,		"-t nat"},
    {RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_MANGLE_IDX,	EDP_ACL_XTABLES_MANGLE},
    {RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL_IDX,		"all"},
	{RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_FILTER_IDX,	EDP_ACL_EBTABLES_FILTER},
    {RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_NAT_IDX,		"-t nat"},
};

struct linux_table_info linux_chain_map[] = {
    {RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_ALL_IDX,		"all"},
    {RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_IN_IDX,		EDP_ACL_XTABLES_INPUT},
    {RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_OUT_IDX,		EDP_ACL_XTABLES_OUTPUT},
    {RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_FWD_IDX,		EDP_ACL_XTABLES_FORWARD},
    {RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_PRE_IDX,		EDP_ACL_XTABLES_PREROUTING},
	{RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_POST_IDX,		EDP_ACL_XTABLES_POSTROUTING},
	{RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_CHAIN_ALL_IDX,		"all"},
	{RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_CHAIN_IN_IDX,		"ACL-IN"},
	{RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_CHAIN_OUT_IDX,		"ACL-OUT"},
	{RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_CHAIN_FWD_IDX,		EDP_ACL_EBTABLES_FORWARD},
	{RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_CHAIN_PRE_IDX,		"ACL-PRE"},
	{RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_CHAIN_POST_IDX,		"ACL-POST"},
};

/*RG CVLAN/SVLAN action debug string*/
char *name_of_rt_edp_cvlan_tagif_decision[]={ //mappint to rtk_rg_acl_cvlan_tagif_decision_t
	"NOP",
	"TAGGING",
	"C2S",
	"SP2C",
	"UNTAG",
	"TRANSPARENT",
};

char *name_of_rt_edp_cvlan_cvid_decision[]={ //mappint to rtk_rg_acl_cvlan_cvid_decision_t
	"ASSIGN",
	"FROM_1ST_TAG",
	"FROM_2ND_TAG",
	"FROM_INTERNAL_VID", //(upstream only)
	"FROM_DMAC2CVID", //(downstream only)
	"NOP", //apolloFE  (downstream only)
	"FROM_SP2C", //apolloFE
};

char *name_of_rt_edp_cvlan_cpri_decision[]={ //mappint to rtk_rg_acl_cvlan_cpri_decision_t
	"ASSIGN",
	"FROM_1ST_TAG",
	"FROM_2ND_TAG",
	"FROM_INTERNAL_PRI",
	"NOP",
	"FROM_DSCP_REMAP",//apolloFE
	"FROM_SP2C", //apolloFE (downstream only)
};


char *name_of_rt_edp_svlan_tagif_decision[]={ //mappint to rtk_rg_acl_svlan_tagif_decision_t
	"NOP",
	"TAGGING_WITH_VSTPID",
	"TAGGING_WITH_8100(not support)",
	"TAGGING_WITH_SP2C(not support)",
	"UNTAG",
	"TRANSPARENT",
	"TAGGING_WITH_VSTPID2",
	"TAGGING_WITH_ORIGINAL_STAG_TPID",
};


char *name_of_rt_edp_svlan_svid_decision[]={ //mappint to rtk_rg_acl_svlan_svid_decision_t
	"ASSIGN",
	"FROM_1ST_TAG",
	"FROM_2ND_TAG",
	"NOP", //apolloFE
	"SP2C", //apolloFE
};

char *name_of_rt_edp_svlan_spri_decision[]={ //mappint to rtk_rg_acl_svlan_spri_decision_t
	"ASSIGN",
	"FROM_1ST_TAG",
	"FROM_2ND_TAG",
	"FROM_INTERNAL_PRI",
	"NOP",//apolloFE
	"FROM_DSCP_REMAP", //apolloFE (downstream only)
	"FROM_SP2C", //apolloFE (downstream only)
};


int _dump_rt_edp_acl_entry_content(struct seq_file *s, rtk_rg_aclFilterAndQos_t *aclPara);
static int _rt_edp_aclSWEntry_not_support(rtk_rg_aclFilterAndQos_t *acl_filter);
static int _rt_edp_aclSWEntry_and_asic_reAdd(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx);
static int _rt_edp_aclSWEntry_to_linux_add_tables(rtk_rg_aclFilterAndQos_t *acl_filter,rt_edp_aclFilterEntry_t* aclSWEntry,int acl_filter_idx);
#if 0
static int _rtk_rg_aclSWEntry_to_asic_add(rtk_rg_aclFilterAndQos_t *acl_filter,rt_edp_aclFilterEntry_t* aclSWEntry,int shareHwAclWithSWAclIdx);

static int _rtk_rg_aclAndCfReservedRuleHeadReflash(void);
static int _rtk_rg_aclAndCfReservedRuleTailReflash(void);

int _rtk_rg_acl_user_part_rearrange(void);
#endif


/*(1)ACL init related APIs*/
#if 0
int _rt_edp_acl_asic_init(void)
{
	return (RT_EDP_ERR_OK);
}

int _rtk_rg_classify_asic_init(void)
{

	return (RT_EDP_ERR_OK);
}
#endif

int _rt_edp_acl_linux_init(void)
{
	int i;

	for(i=0; i<RT_EDP_ACL_USED_TABLE_END; i++) {
		_rt_edp_pipe_cmd("%s %s -N %s", linux_table[i].utility_buf,linux_table[i].table_buf,linux_table[i].chain_buf);
		if(!strcmp(linux_table[i].chain_buf, EDP_ACL_XTABLES_INPUT))
			_rt_edp_pipe_cmd("%s %s -I INPUT -j %s", linux_table[i].utility_buf,linux_table[i].table_buf,linux_table[i].chain_buf);
		else if(!strcmp(linux_table[i].chain_buf, EDP_ACL_XTABLES_OUTPUT))
			_rt_edp_pipe_cmd("%s %s -I OUTPUT -j %s", linux_table[i].utility_buf,linux_table[i].table_buf,linux_table[i].chain_buf);
		else if(!strcmp(linux_table[i].chain_buf, EDP_ACL_XTABLES_FORWARD) || !strcmp(linux_table[i].chain_buf, EDP_ACL_EBTABLES_FORWARD))
			_rt_edp_pipe_cmd("%s %s -I FORWARD -j %s", linux_table[i].utility_buf,linux_table[i].table_buf,linux_table[i].chain_buf);
	}

	return (RT_EDP_ERR_OK);
}

int _rt_edp_is_aclSWEntry_init(void)
{
    if(rt_edp_db.systemGlobal.acl_SW_table_entry==NULL || rt_edp_db.systemGlobal.acl_filter_temp==NULL)
        RT_EDP_RETURN_ERR(RT_EDP_ERR_NULL_POINTER);
    else
        return (RT_EDP_ERR_OK);
}


int _rt_edp_aclSWEntry_init(void)
{
	int j;

	for(j=0;j<MAX_ACL_SW_ENTRY_SIZE;j++){
		rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j]=-1;

		/*Skip ----------------- init acl_SWindex_sorting_by_weight_and_ingress_cvid_action for ingress_vid ----------------- Skip*/
	}

    bzero(rt_edp_db.systemGlobal.acl_SW_table_entry, sizeof(rt_edp_aclFilterEntry_t)*MAX_ACL_SW_ENTRY_SIZE);
    bzero(rt_edp_db.systemGlobal.acl_filter_temp, sizeof(rtk_rg_aclFilterAndQos_t)*MAX_ACL_SW_ENTRY_SIZE);
	rt_edp_db.systemGlobal.acl_SW_table_entry_size=0;



    return (RT_EDP_ERR_OK);
}

/*(2)RG_ACL APIs and internal APIs*/
int _rt_edp_aclSWEntry_get(int index, rt_edp_aclFilterEntry_t* aclSWEntry)
{
    //check the acl_SW_Entry has been allocate
    if(_rt_edp_is_aclSWEntry_init())
    {
        if(_rt_edp_aclSWEntry_init())
            RT_EDP_RETURN_ERR(RT_EDP_ERR_NULL_POINTER);
    }
    *aclSWEntry = rt_edp_db.systemGlobal.acl_SW_table_entry[index];
    return (RT_EDP_ERR_OK);

}

int _rt_edp_aclSWEntry_set(int index, rt_edp_aclFilterEntry_t aclSWEntry)
{
    //check the acl_SW_Entry has been allocate
    if(_rt_edp_is_aclSWEntry_init())
    {
        if(_rt_edp_aclSWEntry_init())
            RT_EDP_RETURN_ERR(RT_EDP_ERR_NULL_POINTER);
    }

	if(index<0 || index >= MAX_ACL_SW_ENTRY_SIZE){
		WARNING("rule index parameter is invalid! access RG_ACL[%d] rule fail.",index);
		return (RT_EDP_ERR_OK);
	}

    rt_edp_db.systemGlobal.acl_SW_table_entry[index] = aclSWEntry;
    return (RT_EDP_ERR_OK);
}

// rt_edp_aclFilterEntry_t aclSWEntry, empty_aclSWEntry;
int _rt_edp_aclSWEntry_empty_find(int* index)
{

    int i;
    bzero(&rt_edp_db.systemGlobal.aclSWEntry_temp, sizeof(rt_edp_db.systemGlobal.aclSWEntry_temp));	//aclSWEntry_for_find
    bzero(&rt_edp_db.systemGlobal.empty_aclSWEntry, sizeof(rt_edp_db.systemGlobal.empty_aclSWEntry));


    //check the acl_SW_Entry has been allocate
    if(_rt_edp_is_aclSWEntry_init())
    {
        if(_rt_edp_aclSWEntry_init())
            RT_EDP_RETURN_ERR(RT_EDP_ERR_NULL_POINTER);
    }

    for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
    {
        if(_rt_edp_aclSWEntry_get(i,&rt_edp_db.systemGlobal.aclSWEntry_temp))
            RT_EDP_RETURN_ERR(RT_EDP_ERR_ACL_SW_ENTRY_ACCESS_FAILED);

        if(!memcmp(&rt_edp_db.systemGlobal.aclSWEntry_temp,&rt_edp_db.systemGlobal.empty_aclSWEntry,sizeof(rt_edp_aclFilterEntry_t)))
        {
            *index = i;
            break;
        }

        //not found empty entry
        if(i==(MAX_ACL_SW_ENTRY_SIZE-1))
            return (RT_EDP_ERR_ACL_SW_ENTRY_FULL);
    }

    return (RT_EDP_ERR_OK);

}

//rt_edp_aclFilterEntry_t aclSWEntry, empty_aclSWEntry;
int _rt_edp_aclSWEntry_dump(void)
{
    int i;

    bzero(&rt_edp_db.systemGlobal.aclSWEntry,sizeof(rt_edp_db.systemGlobal.aclSWEntry));
    bzero(&rt_edp_db.systemGlobal.empty_aclSWEntry,sizeof(rt_edp_db.systemGlobal.empty_aclSWEntry));


  	ACL_CTRL_EDP("dump aclSWEntry");
    for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
    {
        _rt_edp_aclSWEntry_get(i,&rt_edp_db.systemGlobal.aclSWEntry);
        if(memcmp(&rt_edp_db.systemGlobal.aclSWEntry,&rt_edp_db.systemGlobal.empty_aclSWEntry,sizeof(rt_edp_aclFilterEntry_t)))
    	{
#if defined(CONFIG_RG_G3_SERIES)
			ACL_CTRL_EDP("aclSWEntry[%d]: aclindex=%s port=0x%x aclcount=%d\n",i,rt_edp_db.systemGlobal.aclSWEntry.hw_aclEntry_index,rt_edp_db.systemGlobal.aclSWEntry.hw_aclEntry_port.bits[0],rt_edp_db.systemGlobal.aclSWEntry.hw_aclEntry_count);
			ACL_CTRL_EDP("aclSWEntry[%d]: aclindex=%s port=0x%x aclcount=%d\n",i,rt_edp_db.systemGlobal.aclSWEntry.hw_aclEntry_index,rt_edp_db.systemGlobal.aclSWEntry.hw_aclEntry_port.bits[0],rt_edp_db.systemGlobal.aclSWEntry.hw_aclEntry_count);
#else
            ACL_CTRL_EDP("aclSWEntry[%d]: aclstart=%d aclsize=%d cfstart=%d cfsize=%d\n",i,rt_edp_db.systemGlobal.aclSWEntry.hw_aclEntry_start,rt_edp_db.systemGlobal.aclSWEntry.hw_aclEntry_size,rt_edp_db.systemGlobal.aclSWEntry.hw_cfEntry_start,rt_edp_db.systemGlobal.aclSWEntry.hw_cfEntry_size);
#endif
    	}
    }
    return (RT_EDP_ERR_OK);
}


static int _rt_edp_rearrange_ACL_weight(int *accumulateIdx)
{
	int i,j;
	int temp,sp;
	bzero(&rt_edp_db.systemGlobal.empty_aclFilter, sizeof(rtk_rg_aclFilterAndQos_t));

	//1-1. clean the sorting record of type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
	*accumulateIdx = 0;
	for(j=0;j<MAX_ACL_SW_ENTRY_SIZE;j++){
		rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j]=-1;
		/*Skip ----------------- reset acl_SWindex_sorting_by_weight_and_ingress_cvid_action for ingress_vid ----------------- Skip*/
	}

	//1-2. record the rule which type is ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
	for(i=0, j=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
	{
		if(memcmp(&rt_edp_db.systemGlobal.acl_filter_temp[i],&rt_edp_db.systemGlobal.empty_aclFilter,sizeof(rt_edp_db.systemGlobal.empty_aclFilter)))
		{
			if(rt_edp_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
			{
				//ASSERT_EQ(_rtk_rg_aclSWEntry_reAdd(&rt_edp_db.systemGlobal.acl_filter_temp[i], &i),RT_EDP_ERR_OK);
				rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j] = i; //record the rule which type is ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
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
			if(rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j]==-1 || rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1]==-1)
				break;

			if (rt_edp_db.systemGlobal.acl_filter_temp[(rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j])].acl_weight <
				rt_edp_db.systemGlobal.acl_filter_temp[(rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1])].acl_weight) //compare the weight
			{
				//swap the rule index
				temp = rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j];
				rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j] = rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1];
				rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1] = temp;
				sp=0;
			}
		}
		if (sp==1) //if no swap happened, then no need to check the lower array index(they are already sorted).
			break;
	}

	return RT_EDP_ERR_OK;
}

static int _rt_edp_rearrange_ACL_weight_for_l34_trap_drop_permit(int *accumulateIdx)
{
	int i,j;
	int temp,sp;
	int32 sort_start=0, sort_end=0;
    bzero(&rt_edp_db.systemGlobal.empty_aclFilter, sizeof(rtk_rg_aclFilterAndQos_t));

	//1-1. record the continue and sorting start index from rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[],
	sort_start = *accumulateIdx;


	// 1-2. find out the L34 trap/drop/permit rules, and put index to rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[] first
	for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
	{
		if(memcmp(&rt_edp_db.systemGlobal.acl_filter_temp[i],&rt_edp_db.systemGlobal.empty_aclFilter,sizeof(rt_edp_db.systemGlobal.empty_aclFilter)))
		{
			if(rt_edp_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP ||
				rt_edp_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP ||
				rt_edp_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP ||
				rt_edp_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP ||
				rt_edp_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT ||
				rt_edp_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_PERMIT)
			{
					rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[*accumulateIdx]=i;
					(*accumulateIdx)++;
			}
		}
	}

	//1-3. record the sorting end index,  the soring valied range is from rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[sort_start~sort_end]
	sort_end = *accumulateIdx;


	//1-4. sorting the rule from sort_start till end by weight: BubSort
	for (i=MAX_ACL_SW_ENTRY_SIZE-1; i>sort_start; i--){
		sp=1;
	   	for (j=sort_start; j<=i; j++){
			//check data is valid
			if(rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j]==-1 || rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1]==-1)
				break;

		  	if (rt_edp_db.systemGlobal.acl_filter_temp[(rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j])].acl_weight <
				rt_edp_db.systemGlobal.acl_filter_temp[(rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1])].acl_weight) //compare the weight
		 	{
		 		//swap the rule index
			   	temp = rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j];
			   	rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j] = rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1];
			   	rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1] = temp;
			   	sp=0;
		 	}
	   	}
		if (sp==1) //if no swap happened, then no need to check the lower array index(they are already sorted).
			break;
	}


	// 1-4. re-add the rules by sorted result
	for(j=sort_start;j<sort_end;j++)
	{
		i = rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j];
		ASSERT_EQ(_rt_edp_aclSWEntry_and_asic_reAdd(&rt_edp_db.systemGlobal.acl_filter_temp[i], &i),RT_EDP_ERR_OK);
	}


	return RT_EDP_ERR_OK;
}

static int _rt_edp_rearrange_ACL_weight_for_l34_Qos(int *accumulateIdx)
{
	int i,j;
	int temp,sp;
	int32 sort_start=0, sort_end=0;
	bzero(&rt_edp_db.systemGlobal.empty_aclFilter, sizeof(rtk_rg_aclFilterAndQos_t));

	//1-1. record the continue and sorting start index from rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[],
	sort_start = *accumulateIdx;


	// 1-2. find out the L34 QoS rules, and put index to rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[] first
	for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
	{
		if(memcmp(&rt_edp_db.systemGlobal.acl_filter_temp[i],&rt_edp_db.systemGlobal.empty_aclFilter,sizeof(rt_edp_db.systemGlobal.empty_aclFilter)))
		{
			if(rt_edp_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN ||
				rt_edp_db.systemGlobal.acl_filter_temp[i].fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN)
			{
					rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[*accumulateIdx]=i;
					(*accumulateIdx)++;
			}
		}
	}

	//1-3. record the sorting end index,  the soring valied range is from rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[sort_start~sort_end]
	sort_end = *accumulateIdx;


	//1-4. sorting the rule from sort_start till end by weight: BubSort
	for (i=MAX_ACL_SW_ENTRY_SIZE-1; i>sort_start; i--){
		sp=1;
		for (j=sort_start; j<=i; j++){
			//check data is valid
			if(rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j]==-1 || rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1]==-1)
				break;

			if (rt_edp_db.systemGlobal.acl_filter_temp[(rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j])].acl_weight <
				rt_edp_db.systemGlobal.acl_filter_temp[(rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1])].acl_weight) //compare the weight
			{
				//swap the rule index
				temp = rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j];
				rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j] = rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1];
				rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j+1] = temp;
				sp=0;
			}
		}
		if (sp==1) //if no swap happened, then no need to check the lower array index(they are already sorted).
			break;
	}


	// 1-4. re-add the rules by sorted result
	for(j=sort_start;j<sort_end;j++)
	{
		i = rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j];
		ASSERT_EQ(_rt_edp_aclSWEntry_and_asic_reAdd(&rt_edp_db.systemGlobal.acl_filter_temp[i], &i),RT_EDP_ERR_OK);
	}

	return RT_EDP_ERR_OK;
}


static int _rt_edp_aclSWEntry_and_asic_reAdd(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx)
{
	bzero(&rt_edp_db.systemGlobal.aclSWEntry,sizeof(rt_edp_db.systemGlobal.aclSWEntry));
	bzero(&rt_edp_db.systemGlobal.empty_aclSWEntry,sizeof(rt_edp_db.systemGlobal.empty_aclSWEntry));

	//check input parameter
	if(acl_filter == NULL
			|| acl_filter_idx==NULL)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_NULL_POINTER);

	if(acl_filter->filter_fields == 0x0)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INITPM_UNINIT);

	ACL_CTRL_EDP("========reAdd RG_ACL[%d]========",*acl_filter_idx);

	//check the rt_edp_db.systemGlobal.aclSWEntry has been used, reAdd must add aclSWEtry in the assigned acl_filter_idx
	ASSERT_EQ(_rt_edp_aclSWEntry_get(*acl_filter_idx, &rt_edp_db.systemGlobal.aclSWEntry),RT_EDP_ERR_OK);

	if(memcmp(&rt_edp_db.systemGlobal.aclSWEntry,&rt_edp_db.systemGlobal.empty_aclSWEntry,sizeof(rt_edp_aclFilterEntry_t)))
	{
		RT_EDP_RETURN_ERR(RT_EDP_ERR_ACL_SW_ENTRY_USED);
	}

	rt_edp_db.systemGlobal.aclSWEntry.acl_filter = *acl_filter;
	rt_edp_db.systemGlobal.aclSWEntry.valid = RTK_RG_ENABLED;

	/*Skip ----------------- check and add HW ACL ----------------- Skip*/

	//check and add ACL to Linux Table
#if defined(EDP_ACL_SUPPORT_HOOK_CHECK) && (EDP_ACL_SUPPORT_HOOK_CHECK==1)
	if(EDP_ACL_IS_ADD_LINUX_TABLE(acl_filter))
		ASSERT_EQ(_rt_edp_aclSWEntry_to_linux_add_tables(acl_filter, &rt_edp_db.systemGlobal.aclSWEntry, *acl_filter_idx),RT_EDP_ERR_OK);
#else
	ASSERT_EQ(_rt_edp_aclSWEntry_to_linux_add_tables(acl_filter, &rt_edp_db.systemGlobal.aclSWEntry, *acl_filter_idx),RT_EDP_ERR_OK);
#endif	//EDP_ACL_SUPPORT_HOOK_CHECK

	//reAdd must add aclSWEtry in the assigned acl_filter_idx
	ASSERT_EQ(_rt_edp_aclSWEntry_set(*acl_filter_idx,rt_edp_db.systemGlobal.aclSWEntry),RT_EDP_ERR_OK);

	//_rt_edp_aclSWEntry_dump();

	return (RT_EDP_ERR_OK);
}


static int _rt_edp_prefix_get_by_ipv6_mask(uint8* ipv6_mask, int* prefix)
{
	int i,j,k;
	*prefix = -1;

	for (i=0; i<16; i++) {
		if (ipv6_mask[i] != 0xff)
			break;
	}
	for(j=0; j<8; j++){ // j: get the first different bit of the byte
		if (ipv6_mask[i] == 0xff)
			break;
		if(((ipv6_mask[i]<<j)&0x80) != 0x80)
			break;
	}

	*prefix = (i*8)+j;

	/*make sure the IP mask is mask aligned*/
	for(k=0; k<8; k++){
		if (ipv6_mask[i] == 0xff)
			break;
		if(k>j){
			if((((ipv6_mask[i]<<k)&0x80)!=0x0)){WARNING("IP mask not in mask alignment(1) \n"); RT_EDP_RETURN_ERR(RT_EDP_ERR_FAILED);}//rest bit should be 0
		}
	}
	for(k=i+1; k<16; k++){
		if(ipv6_mask[k]!=0x0){WARNING("IP mask not in mask alignment(3) \n"); RT_EDP_RETURN_ERR(RT_EDP_ERR_FAILED);}//rest bytes should be 0x00
	}

	return (RT_EDP_ERR_OK);
}

static int _rt_edp_maskLength_get_by_ipv4_range(ipaddr_t ipv4_addr_start, ipaddr_t ipv4_addr_end, int* maskLength)
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
		if(((ipv4_addr_start<<i)&0x80000000)!=0){WARNING("IP Range not in mask alignment \n"); return(RT_EDP_ERR_FAILED);}
		if(((ipv4_addr_end<<i)&0x80000000)!=0x80000000){WARNING("IP Range not in mask alignment \n"); return(RT_EDP_ERR_FAILED);}
	}

	*maskLength = (32-length);

	return (RT_EDP_ERR_OK);

}

static int _rt_edp_maskLength_get_by_ipv6_range(uint8* ipv6_start, uint8* ipv6_end, int* maskLength)
{
	int i,j,k;
	int length;

	if(ipv6_start[0]==ipv6_end[0] && ipv6_start[1]==ipv6_end[1] &&
		ipv6_start[2]==ipv6_end[2] && ipv6_start[3]==ipv6_end[3] &&
		ipv6_start[4]==ipv6_end[4] && ipv6_start[5]==ipv6_end[5] &&
		ipv6_start[6]==ipv6_end[6] && ipv6_start[7]==ipv6_end[7] &&
		ipv6_start[8]==ipv6_end[8] && ipv6_start[9]==ipv6_end[9] &&
		ipv6_start[10]==ipv6_end[10] && ipv6_start[11]==ipv6_end[11] &&
		ipv6_start[12]==ipv6_end[12] && ipv6_start[13]==ipv6_end[13] &&
		ipv6_start[14]==ipv6_end[14] && ipv6_start[15]==ipv6_end[15]){
		//SINGLE IP
		*maskLength = 0;

	}else{ //RANGE IP
		for(i=0;i<16;i++){
			if(memcmp(&ipv6_start[i],&ipv6_end[i],1)){//i: get the first different byte
				break;
			}
		}
		for(j=0;j<8;j++){ // j: get the first different bit of the byte
			if(((ipv6_start[i]<<j)&0x80) != ((ipv6_end[i]<<j)&0x80))
				break;
		}
		length = (i*8)+j;
		(*maskLength) = 128 - length;




		/*make sure the IP range is mask aligned*/
		for(k=0;k<8;k++){
			if(k>j){
				if((((ipv6_start[i]<<k)&0x80)!=0x0)){WARNING("IP Range not in mask alignment(1) \n"); RT_EDP_RETURN_ERR(RT_EDP_ERR_FAILED);}//rest bit should be 0
				if((((ipv6_end[i]<<k)&0x80)!=0x80)){WARNING("IP Range not in mask alignment(2) \n"); RT_EDP_RETURN_ERR(RT_EDP_ERR_FAILED);}//rest bit should be 1
			}
		}

		for(k=i+1;k<16;k++){
			if(ipv6_start[k]!=0x0){WARNING("IP Range not in mask alignment(3) \n"); RT_EDP_RETURN_ERR(RT_EDP_ERR_FAILED);}//rest bytes should be 0x00
			if(ipv6_end[k]!=0xff){WARNING("IP Range not in mask alignment(4) \n"); RT_EDP_RETURN_ERR(RT_EDP_ERR_FAILED);}//rest bytes should be 0xff
		}
	}
	return (RT_EDP_ERR_OK);

}


#if 0
static int _rtk_rg_aclSWEntry_to_asic_add(rtk_rg_aclFilterAndQos_t *acl_filter,rt_edp_aclFilterEntry_t* aclSWEntry,int shareHwAclWithSWAclIdx)
{

	return (RT_EDP_ERR_OK);
}

int _rtk_rg_acl_user_part_rearrange(void)
{
	int ret;

	return ret;
}

#endif

int _rt_edp_aclSWEntry_and_asic_rearrange(void)
{
	int i,j,accumulateIdx;
	rtk_rg_aclFilterAndQos_t *acl_filter;



	bzero(&rt_edp_db.systemGlobal.aclSWEntry_temp, sizeof(rt_edp_aclFilterEntry_t));	//aclSWEntry_rearrange
	bzero(rt_edp_db.systemGlobal.acl_filter_temp, sizeof(rtk_rg_aclFilterAndQos_t)*MAX_ACL_SW_ENTRY_SIZE);

	/*Skip ----------------- Clear HW_ACL user used table ----------------- Skip*/

	//clear linux tables
	for(i=0; i<RT_EDP_ACL_USED_TABLE_END; i++) {
		_rt_edp_pipe_cmd("%s %s -F %s", linux_table[i].utility_buf,linux_table[i].table_buf,linux_table[i].chain_buf);
	}

	//clear the limitation for add ACL drop rule to HW.
	rt_edp_db.systemGlobal.stop_add_hw_acl = 0;

	//clear the information of rearrange status
	rt_edp_db.systemGlobal.stop_add_acl = FALSE;


	ACL_CTRL_EDP("================[do ACL rearrange]==================");
	//backup all acl_filter for reAdd, and clean all aclSWEntry
	for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
	{
		//backup acl_filter
		ASSERT_EQ(_rt_edp_aclSWEntry_get(i, &rt_edp_db.systemGlobal.aclSWEntry_temp),RT_EDP_ERR_OK);
		rt_edp_db.systemGlobal.acl_filter_temp[i] = rt_edp_db.systemGlobal.aclSWEntry_temp.acl_filter;
		//clean aclSWEntry
		bzero(&rt_edp_db.systemGlobal.aclSWEntry_temp, sizeof(rt_edp_aclFilterEntry_t));
		ASSERT_EQ(_rt_edp_aclSWEntry_set(i, rt_edp_db.systemGlobal.aclSWEntry_temp),RT_EDP_ERR_OK);
	}

	//reAdd all reserve aclSWEntry:  type priority as following
		//1. ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET (with acl_weight sorting)
		//2. ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP or ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP
		//3. ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID or ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN
	ASSERT_EQ(_rt_edp_rearrange_ACL_weight(&accumulateIdx),RT_EDP_ERR_OK);


	//1. readd the rules of type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
	for(j=0;j<MAX_ACL_SW_ENTRY_SIZE;j++){
		if(rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j]==-1)
			break;

		acl_filter = &rt_edp_db.systemGlobal.acl_filter_temp[(rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j])];
		ASSERT_EQ(_rt_edp_aclSWEntry_and_asic_reAdd(acl_filter, &rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[j]),RT_EDP_ERR_OK);
	}

	// 2. add rule of ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP or ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP
	_rt_edp_rearrange_ACL_weight_for_l34_trap_drop_permit(&accumulateIdx);


	// 3 add rule of type ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID or ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN
	_rt_edp_rearrange_ACL_weight_for_l34_Qos(&accumulateIdx);

	/*Skip ----------------- maintain acl_SWindex_sorting_by_weight_and_ingress_cvid_action for ingress_vid ----------------- Skip*/

	return (RT_EDP_ERR_OK);
}

static int _rt_edp_linux_table_decision(rtk_rg_aclFilterAndQos_t *acl_filter, rt_edp_aclFilterEntry_t* aclSWEntry)
{
	rt_edp_aclSWEntry_internal_check_field_t tables_internal_check = 0;

	/*table decision by pattern (iptable/ip6table/ebtable).
		a. pattern ethertype only can use in ebtable.
		b. check use ipv4/ipv6 tag if or not, otherwise should configure in both xptable.
		c. xptable and ebtable should config protocol if user use l4 port as pattern.*/

	if(acl_filter->filter_fields & INGRESS_ETHERTYPE_BIT) {
		if(acl_filter->ingress_ethertype == 0x0800) {
			tables_internal_check = RT_EDP_ACL_INTERNAL_CHECK_XTABLES|RT_EDP_ACL_INTERNAL_CHECK_EBTABLES|RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV4;
		} else if (acl_filter->ingress_ethertype == 0x86dd) {
			tables_internal_check = RT_EDP_ACL_INTERNAL_CHECK_XTABLES|RT_EDP_ACL_INTERNAL_CHECK_EBTABLES|RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV6;
		} else {
			tables_internal_check = RT_EDP_ACL_INTERNAL_CHECK_EBTABLES|RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV4|RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV6;
		}
	}else if(((acl_filter->filter_fields & INGRESS_IPV4_TAGIF_BIT)&&(acl_filter->ingress_ipv4_tagif==1)) ||(acl_filter->filter_fields & INGRESS_ACL_PATTERN_IPV4_BITS) ||
			((acl_filter->filter_fields & INGRESS_IPV6_TAGIF_BIT)&&(acl_filter->ingress_ipv6_tagif==0))) {
		tables_internal_check = RT_EDP_ACL_INTERNAL_CHECK_XTABLES|RT_EDP_ACL_INTERNAL_CHECK_EBTABLES|RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV4;
	}else if(((acl_filter->filter_fields & INGRESS_IPV6_TAGIF_BIT)&&(acl_filter->ingress_ipv6_tagif==1)) || (acl_filter->filter_fields & INGRESS_ACL_PATTERN_IPV6_BITS) ||
			((acl_filter->filter_fields & INGRESS_IPV4_TAGIF_BIT)&&(acl_filter->ingress_ipv4_tagif==0))) {
		tables_internal_check = RT_EDP_ACL_INTERNAL_CHECK_XTABLES|RT_EDP_ACL_INTERNAL_CHECK_EBTABLES|RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV6;
	}else {
		tables_internal_check = RT_EDP_ACL_INTERNAL_CHECK_XTABLES|RT_EDP_ACL_INTERNAL_CHECK_EBTABLES|
							RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV4|RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV6;
	}
	if((acl_filter->filter_fields & INGRESS_L4_SPORT_RANGE_BIT) || (acl_filter->filter_fields & INGRESS_L4_DPORT_RANGE_BIT)) {
		if(acl_filter->filter_fields&INGRESS_L4_TCP_BIT)
			tables_internal_check |= RT_EDP_ACL_INTERNAL_CHECK_PROTO_TCP;
		else if(acl_filter->filter_fields&INGRESS_L4_UDP_BIT)
			tables_internal_check |= RT_EDP_ACL_INTERNAL_CHECK_PROTO_UDP;
		else
			tables_internal_check |= RT_EDP_ACL_INTERNAL_CHECK_PROTO_TCP|RT_EDP_ACL_INTERNAL_CHECK_PROTO_UDP;
	}
	else
		tables_internal_check |= RT_EDP_ACL_INTERNAL_CHECK_PROTO_NCARE;
	aclSWEntry->tables_internal_check = tables_internal_check;

	return (RT_EDP_ERR_OK);

}

static void _rt_edp_linux_debug_dump(int internal_check, int pattern_not_sup_table, int pattern_not_sup_chain, int action_not_sup_table, int action_not_sup_chain)
{
	unsigned char internal_check_buf[96];
	unsigned char ptr_not_sup_table_buf[96];
	unsigned char ptr_not_sup_chain_buf[96];
	unsigned char act_not_sup_table_buf[96];
	unsigned char act_not_sup_chain_buf[96];

	sprintf(internal_check_buf, "%s%s%s%s%s%s%s%s",
		internal_check&RT_EDP_ACL_INTERNAL_CHECK_XTABLES?"XTABLES,":"",
		internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES?"EBTABLES,":"",
		internal_check&RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_NCARE?"ETHERTYPE_NCARE,":"",
		internal_check&RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV4?"IPV4,":"",
		internal_check&RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV6?"IPV6,":"",
		internal_check&RT_EDP_ACL_INTERNAL_CHECK_PROTO_NCARE?"PROTO_NCARE,":"",
		internal_check&RT_EDP_ACL_INTERNAL_CHECK_PROTO_TCP?"TCP,":"",
		internal_check&RT_EDP_ACL_INTERNAL_CHECK_PROTO_UDP?"UDP,":""
	);
	sprintf(ptr_not_sup_table_buf, "%s%s%s%s%s%s%s",
		pattern_not_sup_table&RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL?"XT-A,":"",
		pattern_not_sup_table&RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_FILTER?"XT-F,":"",
		pattern_not_sup_table&RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_NAT?"XT-N,":"",
		pattern_not_sup_table&RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_MANGLE?"XT-M,":"",
		pattern_not_sup_table&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL?"EB-A,":"",
		pattern_not_sup_table&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_FILTER?"EB-F,,":"",
		pattern_not_sup_table&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_NAT?"EB-N,":""
	);
	sprintf(ptr_not_sup_chain_buf, "%s%s%s%s%s%s%s%s%s%s%s%s",
		pattern_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_ALL?"XT-A,":"",
		pattern_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_IN?"XT-IN,":"",
		pattern_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_OUT?"XT-OUT,":"",
		pattern_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_FWD?"XT-FWD,":"",
		pattern_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_PRE?"XT-PRE,":"",
		pattern_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_POST?"XT-POST,":"",
		pattern_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_CHAIN_ALL?"EB-A,":"",
		pattern_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_CHAIN_IN?"EB-IN,":"",
		pattern_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_CHAIN_OUT?"EB-OUT,":"",
		pattern_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_CHAIN_FWD?"EB-FWD,":"",
		pattern_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_CHAIN_PRE?"EB-PRE,":"",
		pattern_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_CHAIN_POST?"EB-POST,":""
	);
	sprintf(act_not_sup_table_buf, "%s%s%s%s%s%s%s",
		action_not_sup_table&RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL?"XT-A,":"",
		action_not_sup_table&RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_FILTER?"XT-F,":"",
		action_not_sup_table&RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_NAT?"XT-N,":"",
		action_not_sup_table&RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_MANGLE?"XT-M,":"",
		action_not_sup_table&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL?"EB-A,":"",
		action_not_sup_table&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_FILTER?"EB-F,,":"",
		action_not_sup_table&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_NAT?"EB-N,":""
	);
	sprintf(act_not_sup_chain_buf, "%s%s%s%s%s%s%s%s%s%s%s%s",
		action_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_ALL?"XT-A,":"",
		action_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_IN?"XT-IN,":"",
		action_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_OUT?"XT-OUT,":"",
		action_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_FWD?"XT-FWD,":"",
		action_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_PRE?"XT-PRE,":"",
		action_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_POST?"XT-POST,":"",
		action_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_CHAIN_ALL?"EB-A,":"",
		action_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_CHAIN_IN?"EB-IN,":"",
		action_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_CHAIN_OUT?"EB-OUT,":"",
		action_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_CHAIN_FWD?"EB-FWD,":"",
		action_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_CHAIN_PRE?"EB-PRE,":"",
		action_not_sup_chain&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_CHAIN_POST?"EB-POST,":""
	);

	ACL_CTRL_EDP("[%s]NSUP TB/CH by Pattern: %s/%s; Action: %s/%s", internal_check_buf, ptr_not_sup_table_buf, ptr_not_sup_chain_buf, act_not_sup_table_buf, act_not_sup_chain_buf);

	return;
}

static int _rt_edp_linux_pattern_translate(rtk_rg_aclFilterAndQos_t *acl_filter, rt_edp_aclFilterEntry_t* aclSWEntry, int tables_internal_check)
{
	rt_edp_aclSWEntry_internal_check_table_t ptr_nsup_table = 0;
	rt_edp_aclSWEntry_internal_check_chain_t ptr_nsup_chain = 0;
	char *pattern_buf = rt_edp_db.systemGlobal.acl_cmd_buff;
	unsigned char tmp_buf[96];
	unsigned char tmp_buf2[64];
	uint8   mac_tmp1[6*sizeof "123"];
	uint8   mac_tmp2[6*sizeof "123"];
	uint8   ipv4_addr[4*sizeof "123"];
	uint8   ipv4_addr_end[4*sizeof "123"];
	uint8   ipv6_addr[8*sizeof "FFFF:"];
	uint8   ipv6_addr_end[8*sizeof "FFFF:"];
	int ipv6_prefix_by_mask =0;
	int unmask_length=0;
	uint8 cmd_xptable_p_string = FALSE;	//0: not use; 1:tcp; 2:udp
	uint8 cmd_ebtable_p_string = FALSE;	//0: not use; 1:ipv4; 2:ipv6

	/*set patterns*/
	if((acl_filter->filter_fields & INGRESS_INTF_BIT) && acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
	{
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_XTABLES) {
			snprintf(tmp_buf, sizeof(tmp_buf), "-i eth0.2 ");
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			snprintf(tmp_buf, sizeof(tmp_buf), "-i eth0.2 ");
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
		//-i transform, under development
	}

	if(acl_filter->filter_fields & INGRESS_PORT_BIT)
	{
		//deal with in _rt_edp_linux_cmd_add();
	}

	if(acl_filter->filter_fields & INGRESS_WLANDEV_BIT){
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_XTABLES) {
			snprintf(tmp_buf, sizeof(tmp_buf), "-i eth0.2 ");
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			snprintf(tmp_buf, sizeof(tmp_buf), "-i eth0.2 ");
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
		//-i transform, under development
	}


	if(acl_filter->filter_fields & INGRESS_ETHERTYPE_BIT)
	{
		//ingress_ethertype=0x0800 for iptable or ingress_ethertype=0x86dd for ip6table
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			//not support acl_filter->ingress_ethertype_mask;
			snprintf(tmp_buf, sizeof(tmp_buf), "-p %s0x%x ", acl_filter->filter_fields_inverse&INGRESS_ETHERTYPE_BIT?"! ":"", acl_filter->ingress_ethertype);
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);

			if(acl_filter->ingress_ethertype_mask!=0xffff){
//				WARNING("Linux ebtables not support ingress_ethertype_mask");
//				return(RT_EDP_ERR_INVALID_PARAM);
				ptr_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
			}
		}
	}

	if(acl_filter->filter_fields & INGRESS_CTAG_PRI_BIT)
	{
		ptr_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL;
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			snprintf(tmp_buf, sizeof(tmp_buf), "-p 8021q %s--vlan-prio %d ", acl_filter->filter_fields_inverse&INGRESS_CTAG_PRI_BIT?"! ":"", acl_filter->ingress_ctag_pri);
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
	}

	if(acl_filter->filter_fields & INGRESS_CTAG_VID_BIT)
	{
		ptr_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL;
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			snprintf(tmp_buf, sizeof(tmp_buf), "-p 8021q %s--vlan-id %d ", acl_filter->filter_fields_inverse&INGRESS_CTAG_VID_BIT?"! ":"", acl_filter->ingress_ctag_vid);
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
	}

	if(acl_filter->filter_fields & INGRESS_CTAG_CFI_BIT )
	{
		WARNING("Linux tables not support ingress_ctag_cfi");
		return(RT_EDP_ERR_INVALID_PARAM);
	}

	if(acl_filter->filter_fields & INGRESS_SMAC_BIT)
	{
		//only valid in PREROUTING|FORWARD|INPUT Chains
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_XTABLES) {
			ptr_nsup_chain |= RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_OUT|RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_POST;
			if((acl_filter->ingress_smac_mask.octet[0]&acl_filter->ingress_smac_mask.octet[1]&acl_filter->ingress_smac_mask.octet[2]
				&acl_filter->ingress_smac_mask.octet[3]&acl_filter->ingress_smac_mask.octet[4]&acl_filter->ingress_smac_mask.octet[5])==0xff) {
				snprintf(tmp_buf, sizeof(tmp_buf), "-m mac %s--mac-s %s ", acl_filter->filter_fields_inverse&INGRESS_SMAC_BIT?"! ":"", _rt_edp_inet_mactoa_p(acl_filter->ingress_smac.octet, mac_tmp1));
			}else{
				snprintf(tmp_buf, sizeof(tmp_buf), "-m mac %s--mac-s %s --src-mask %s ", acl_filter->filter_fields_inverse&INGRESS_SMAC_BIT?"! ":"",
					_rt_edp_inet_mactoa_p(acl_filter->ingress_smac.octet, mac_tmp1), _rt_edp_inet_mactoa_p(acl_filter->ingress_smac_mask.octet, mac_tmp2));
			}
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			if((acl_filter->ingress_smac_mask.octet[0]&acl_filter->ingress_smac_mask.octet[1]&acl_filter->ingress_smac_mask.octet[2]
				&acl_filter->ingress_smac_mask.octet[3]&acl_filter->ingress_smac_mask.octet[4]&acl_filter->ingress_smac_mask.octet[5])==0xff) {
				snprintf(tmp_buf, sizeof(tmp_buf), "-s %s%s ", acl_filter->filter_fields_inverse&INGRESS_SMAC_BIT?"! ":"", _rt_edp_inet_mactoa_p(acl_filter->ingress_smac.octet, mac_tmp1));
			}else{
				snprintf(tmp_buf, sizeof(tmp_buf), "-s %s%s/%s ", acl_filter->filter_fields_inverse&INGRESS_SMAC_BIT?"! ":"",
					_rt_edp_inet_mactoa_p(acl_filter->ingress_smac.octet, mac_tmp1), _rt_edp_inet_mactoa_p(acl_filter->ingress_smac_mask.octet, mac_tmp2));
			}
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
	}

	if(acl_filter->filter_fields & INGRESS_DMAC_BIT)
	{
		//only valid in PREROUTING|FORWARD|INPUT Chains
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_XTABLES) {
			ptr_nsup_chain |= RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_OUT|RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_POST;
			if((acl_filter->ingress_dmac_mask.octet[0]&acl_filter->ingress_dmac_mask.octet[1]&acl_filter->ingress_dmac_mask.octet[2]
				&acl_filter->ingress_dmac_mask.octet[3]&acl_filter->ingress_dmac_mask.octet[4]&acl_filter->ingress_dmac_mask.octet[5])==0xff) {
				snprintf(tmp_buf, sizeof(tmp_buf), "-m mac %s--mac-d %s ", acl_filter->filter_fields_inverse&INGRESS_DMAC_BIT?"! ":"", _rt_edp_inet_mactoa_p(acl_filter->ingress_dmac.octet, mac_tmp1));
			}else{
				snprintf(tmp_buf, sizeof(tmp_buf), "-m mac %s--mac-d %s --dst-mask %s ", acl_filter->filter_fields_inverse&INGRESS_DMAC_BIT?"! ":"",
					_rt_edp_inet_mactoa_p(acl_filter->ingress_dmac.octet, mac_tmp1), _rt_edp_inet_mactoa_p(acl_filter->ingress_dmac_mask.octet, mac_tmp2));
			}
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			if((acl_filter->ingress_dmac_mask.octet[0]&acl_filter->ingress_dmac_mask.octet[1]&acl_filter->ingress_dmac_mask.octet[2]
				&acl_filter->ingress_dmac_mask.octet[3]&acl_filter->ingress_dmac_mask.octet[4]&acl_filter->ingress_dmac_mask.octet[5])==0xff) {
				snprintf(tmp_buf, sizeof(tmp_buf), "-d %s%s ", acl_filter->filter_fields_inverse&INGRESS_DMAC_BIT?"! ":"", _rt_edp_inet_mactoa_p(acl_filter->ingress_dmac.octet, mac_tmp1));
			}else{
				snprintf(tmp_buf, sizeof(tmp_buf), "-d %s%s/%s ", acl_filter->filter_fields_inverse&INGRESS_DMAC_BIT?"! ":"",
					_rt_edp_inet_mactoa_p(acl_filter->ingress_dmac.octet, mac_tmp1), _rt_edp_inet_mactoa_p(acl_filter->ingress_dmac_mask.octet, mac_tmp2));
			}
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
	}

	if(acl_filter->filter_fields & INGRESS_DSCP_BIT)
	{
		ptr_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_XTABLES) {
			snprintf(tmp_buf, sizeof(tmp_buf), "-m dscp %s--dscp %d ", acl_filter->filter_fields_inverse&INGRESS_DSCP_BIT?"! ":"", acl_filter->ingress_dscp);
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
	}

	if(acl_filter->filter_fields & INGRESS_TOS_BIT)
	{
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_XTABLES) {
			snprintf(tmp_buf, sizeof(tmp_buf), "-m tos %s--tos %d ", acl_filter->filter_fields_inverse&INGRESS_TOS_BIT?"! ":"", acl_filter->ingress_tos);
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip-tos %s%d ", cmd_ebtable_p_string==1?"":EDP_ACL_CMD_P_IPV4, acl_filter->filter_fields_inverse&INGRESS_TOS_BIT?"! ":"", acl_filter->ingress_tos);
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
			cmd_ebtable_p_string=1;
		}
	}

	if(acl_filter->filter_fields & INGRESS_L4_ICMP_BIT)
	{
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_XTABLES) {
			snprintf(tmp_buf, sizeof(tmp_buf), "%s-p icmp ", acl_filter->filter_fields_inverse&INGRESS_L4_ICMP_BIT?"! ":"");
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip-proto %sicmp ", cmd_ebtable_p_string==1?"":EDP_ACL_CMD_P_IPV4, acl_filter->filter_fields_inverse&INGRESS_L4_ICMP_BIT?"! ":"");
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
			cmd_ebtable_p_string=1;
		}
	}

	if(acl_filter->filter_fields & INGRESS_L4_TCP_BIT)
	{
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_XTABLES) {
			snprintf(tmp_buf, sizeof(tmp_buf), "%s%s", acl_filter->filter_fields_inverse&INGRESS_L4_TCP_BIT?"! ":"", cmd_xptable_p_string==1?"":EDP_ACL_CMD_P_TCP);
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
			cmd_xptable_p_string=1;
		}
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			if(tables_internal_check&ACL_INTERNAL_CHECK_ETHERTYPE) {
				if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV4) {
					snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip-proto %stcp ", cmd_ebtable_p_string==1?"":EDP_ACL_CMD_P_IPV4, acl_filter->filter_fields_inverse&INGRESS_L4_TCP_BIT?"! ":"");
					cmd_ebtable_p_string=1;
				}
				else {	//RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV6
					snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip6-proto %stcp ", cmd_ebtable_p_string==2?"":EDP_ACL_CMD_P_IPV6, acl_filter->filter_fields_inverse&INGRESS_L4_TCP_BIT?"! ":"");
					cmd_ebtable_p_string=2;
				}
				ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
			}
			else
				ptr_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
		}
	}

	if(acl_filter->filter_fields & INGRESS_L4_UDP_BIT)
	{
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_XTABLES) {
			snprintf(tmp_buf, sizeof(tmp_buf), "%s%s", acl_filter->filter_fields_inverse&INGRESS_L4_UDP_BIT?"! ":"", cmd_xptable_p_string==2?"":EDP_ACL_CMD_P_UDP);
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
			cmd_xptable_p_string=2;
		}
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			if(tables_internal_check&ACL_INTERNAL_CHECK_ETHERTYPE) {
				if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV4) {
					snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip-proto %sudp ", cmd_ebtable_p_string==1?"":EDP_ACL_CMD_P_IPV4, acl_filter->filter_fields_inverse&INGRESS_L4_UDP_BIT?"! ":"");
					cmd_ebtable_p_string=1;
				}
				else {	//RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV6
					snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip6-proto %sudp ", cmd_ebtable_p_string==2?"":EDP_ACL_CMD_P_IPV6, acl_filter->filter_fields_inverse&INGRESS_L4_UDP_BIT?"! ":"");
					cmd_ebtable_p_string=2;
				}
				ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
			}
			else
				ptr_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
		}
	}

	if(acl_filter->filter_fields & INGRESS_L4_NONE_TCP_NONE_UDP_BIT)
	{
		WARNING("Linux tables not support ingress_l4_protocal with 3(None-TCP and None-UDP)");
		return(RT_EDP_ERR_INVALID_PARAM);
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_DSCP_BIT)
	{
		ptr_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_XTABLES) {
			snprintf(tmp_buf, sizeof(tmp_buf), "-m dscp %s--dscp %d ", acl_filter->filter_fields_inverse&INGRESS_IPV6_DSCP_BIT?"! ":"", acl_filter->ingress_ipv6_dscp);
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_TC_BIT)
	{
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_XTABLES) {
			snprintf(tmp_buf, sizeof(tmp_buf), "-m tos %s--tos %d ", acl_filter->filter_fields_inverse&INGRESS_IPV6_TC_BIT?"! ":"", acl_filter->ingress_ipv6_tc);
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip6-tc %s%d ", cmd_ebtable_p_string==2?"":EDP_ACL_CMD_P_IPV6, acl_filter->filter_fields_inverse&INGRESS_IPV6_TC_BIT?"! ":"", acl_filter->ingress_ipv6_tc);
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
			cmd_ebtable_p_string=2;
		}
	}

	if(acl_filter->filter_fields & INGRESS_L4_ICMPV6_BIT)
	{
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_XTABLES) {
			snprintf(tmp_buf, sizeof(tmp_buf), "%s-p icmp ", acl_filter->filter_fields_inverse&INGRESS_L4_ICMPV6_BIT?"! ":"");
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip6-proto %sicmp ", cmd_ebtable_p_string==2?"":EDP_ACL_CMD_P_IPV6, acl_filter->filter_fields_inverse&INGRESS_L4_ICMPV6_BIT?"! ":"");
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
			cmd_ebtable_p_string=2;
		}
	}

	if(acl_filter->filter_fields & INGRESS_STREAM_ID_BIT)
	{
		WARNING("Linux tables not support ingress_stream_id and ingress_stream_id_mask");
		return(RT_EDP_ERR_INVALID_PARAM);
	}

	if(acl_filter->filter_fields & INGRESS_STAG_PRI_BIT)
	{
		WARNING("Linux tables not support ingress_stag_pri");
		return(RT_EDP_ERR_INVALID_PARAM);
	}

	if(acl_filter->filter_fields & INGRESS_STAG_VID_BIT)
	{
		WARNING("Linux tables not support ingress_stag_vid");
		return(RT_EDP_ERR_INVALID_PARAM);
	}

	if(acl_filter->filter_fields & INGRESS_STAG_DEI_BIT )
	{
		WARNING("Linux tables not support ingress_stag_dei");
		return(RT_EDP_ERR_INVALID_PARAM);
	}

	if(acl_filter->filter_fields & INGRESS_STAGIF_BIT)
	{
		WARNING("Linux tables not support ingress_stagIf");
		return(RT_EDP_ERR_INVALID_PARAM);
	}

	if(acl_filter->filter_fields & INGRESS_CTAGIF_BIT)
	{
		ptr_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL;
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			snprintf(tmp_buf, sizeof(tmp_buf), "-p %s8021q ", acl_filter->ingress_ctagIf?"":"! ");
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
	}

	if(acl_filter->filter_fields & INGRESS_L4_POROTCAL_VALUE_BIT)
	{
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_XTABLES) {
			snprintf(tmp_buf, sizeof(tmp_buf), "%s-p %d ", acl_filter->filter_fields_inverse&INGRESS_L4_POROTCAL_VALUE_BIT?"! ":"", acl_filter->ingress_l4_protocal);
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			if(tables_internal_check&ACL_INTERNAL_CHECK_ETHERTYPE) {
				if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV4) {
					snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip-proto %s%d ", cmd_ebtable_p_string==1?"":EDP_ACL_CMD_P_IPV4, acl_filter->filter_fields_inverse&INGRESS_L4_POROTCAL_VALUE_BIT?"! ":"", acl_filter->ingress_l4_protocal);
					cmd_ebtable_p_string=1;
				}
				else {	//RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV6
					snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip6-proto %s%d ", cmd_ebtable_p_string==2?"":EDP_ACL_CMD_P_IPV6, acl_filter->filter_fields_inverse&INGRESS_L4_POROTCAL_VALUE_BIT?"! ":"", acl_filter->ingress_l4_protocal);
					cmd_ebtable_p_string=2;
				}
				ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
			}
			else
				ptr_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_SIP_BIT )
	{
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_XTABLES) {
			if((acl_filter->ingress_src_ipv6_addr_mask[0]&acl_filter->ingress_src_ipv6_addr_mask[1]&acl_filter->ingress_src_ipv6_addr_mask[2]&acl_filter->ingress_src_ipv6_addr_mask[3]
				&acl_filter->ingress_src_ipv6_addr_mask[4]&acl_filter->ingress_src_ipv6_addr_mask[5]&acl_filter->ingress_src_ipv6_addr_mask[6]&acl_filter->ingress_src_ipv6_addr_mask[7]
				&acl_filter->ingress_src_ipv6_addr_mask[8]&acl_filter->ingress_src_ipv6_addr_mask[9]&acl_filter->ingress_src_ipv6_addr_mask[10]&acl_filter->ingress_src_ipv6_addr_mask[11]
				&acl_filter->ingress_src_ipv6_addr_mask[12]&acl_filter->ingress_src_ipv6_addr_mask[13]&acl_filter->ingress_src_ipv6_addr_mask[14]&acl_filter->ingress_src_ipv6_addr_mask[15])==0xff) {
				snprintf(tmp_buf, sizeof(tmp_buf), "%s-s %s ", acl_filter->filter_fields_inverse&INGRESS_IPV6_SIP_BIT?"! ":"",
					_rt_edp_inet_n6toa_p(acl_filter->ingress_src_ipv6_addr, ipv6_addr));
			}else{
				ASSERT_EQ(_rt_edp_prefix_get_by_ipv6_mask(acl_filter->ingress_src_ipv6_addr_mask,&ipv6_prefix_by_mask),RT_EDP_ERR_OK);
				snprintf(tmp_buf, sizeof(tmp_buf), "%s-s %s/%d ", acl_filter->filter_fields_inverse&INGRESS_IPV6_SIP_BIT?"! ":"",
					_rt_edp_inet_n6toa_p(acl_filter->ingress_src_ipv6_addr, ipv6_addr),ipv6_prefix_by_mask);
			}
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			if((acl_filter->ingress_src_ipv6_addr_mask[0]&acl_filter->ingress_src_ipv6_addr_mask[1]&acl_filter->ingress_src_ipv6_addr_mask[2]&acl_filter->ingress_src_ipv6_addr_mask[3]
				&acl_filter->ingress_src_ipv6_addr_mask[4]&acl_filter->ingress_src_ipv6_addr_mask[5]&acl_filter->ingress_src_ipv6_addr_mask[6]&acl_filter->ingress_src_ipv6_addr_mask[7]
				&acl_filter->ingress_src_ipv6_addr_mask[8]&acl_filter->ingress_src_ipv6_addr_mask[9]&acl_filter->ingress_src_ipv6_addr_mask[10]&acl_filter->ingress_src_ipv6_addr_mask[11]
				&acl_filter->ingress_src_ipv6_addr_mask[12]&acl_filter->ingress_src_ipv6_addr_mask[13]&acl_filter->ingress_src_ipv6_addr_mask[14]&acl_filter->ingress_src_ipv6_addr_mask[15])==0xff) {
				snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip6-src %s%s ", cmd_ebtable_p_string==2?"":EDP_ACL_CMD_P_IPV6, acl_filter->filter_fields_inverse&INGRESS_IPV6_SIP_BIT?"! ":"",
					_rt_edp_inet_n6toa_p(acl_filter->ingress_src_ipv6_addr, ipv6_addr));
				cmd_ebtable_p_string=2;
			}else{
				ASSERT_EQ(_rt_edp_prefix_get_by_ipv6_mask(acl_filter->ingress_src_ipv6_addr_mask,&ipv6_prefix_by_mask),RT_EDP_ERR_OK);
				snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip6-src %s%s/%d ", cmd_ebtable_p_string==2?"":EDP_ACL_CMD_P_IPV6, acl_filter->filter_fields_inverse&INGRESS_IPV6_SIP_BIT?"! ":"",
					_rt_edp_inet_n6toa_p(acl_filter->ingress_src_ipv6_addr, ipv6_addr),ipv6_prefix_by_mask);
				cmd_ebtable_p_string=2;
			}
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_DIP_BIT )
	{
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_XTABLES) {
			if((acl_filter->ingress_dest_ipv6_addr_mask[0]&acl_filter->ingress_dest_ipv6_addr_mask[1]&acl_filter->ingress_dest_ipv6_addr_mask[2]&acl_filter->ingress_dest_ipv6_addr_mask[3]
				&acl_filter->ingress_dest_ipv6_addr_mask[4]&acl_filter->ingress_dest_ipv6_addr_mask[5]&acl_filter->ingress_dest_ipv6_addr_mask[6]&acl_filter->ingress_dest_ipv6_addr_mask[7]
				&acl_filter->ingress_dest_ipv6_addr_mask[8]&acl_filter->ingress_dest_ipv6_addr_mask[9]&acl_filter->ingress_dest_ipv6_addr_mask[10]&acl_filter->ingress_dest_ipv6_addr_mask[11]
				&acl_filter->ingress_dest_ipv6_addr_mask[12]&acl_filter->ingress_dest_ipv6_addr_mask[13]&acl_filter->ingress_dest_ipv6_addr_mask[14]&acl_filter->ingress_dest_ipv6_addr_mask[15])==0xff) {
				snprintf(tmp_buf, sizeof(tmp_buf), "%s-d %s ", acl_filter->filter_fields_inverse&INGRESS_IPV6_DIP_BIT?"! ":"",
					_rt_edp_inet_n6toa_p(acl_filter->ingress_dest_ipv6_addr, ipv6_addr));
			}else{
				ASSERT_EQ(_rt_edp_prefix_get_by_ipv6_mask(acl_filter->ingress_dest_ipv6_addr_mask,&ipv6_prefix_by_mask),RT_EDP_ERR_OK);
				snprintf(tmp_buf, sizeof(tmp_buf), "%s-d %s/%d ", acl_filter->filter_fields_inverse&INGRESS_IPV6_DIP_BIT?"! ":"",
					_rt_edp_inet_n6toa_p(acl_filter->ingress_dest_ipv6_addr, ipv6_addr),ipv6_prefix_by_mask);
			}
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			if((acl_filter->ingress_dest_ipv6_addr_mask[0]&acl_filter->ingress_dest_ipv6_addr_mask[1]&acl_filter->ingress_dest_ipv6_addr_mask[2]&acl_filter->ingress_dest_ipv6_addr_mask[3]
				&acl_filter->ingress_dest_ipv6_addr_mask[4]&acl_filter->ingress_dest_ipv6_addr_mask[5]&acl_filter->ingress_dest_ipv6_addr_mask[6]&acl_filter->ingress_dest_ipv6_addr_mask[7]
				&acl_filter->ingress_dest_ipv6_addr_mask[8]&acl_filter->ingress_dest_ipv6_addr_mask[9]&acl_filter->ingress_dest_ipv6_addr_mask[10]&acl_filter->ingress_dest_ipv6_addr_mask[11]
				&acl_filter->ingress_dest_ipv6_addr_mask[12]&acl_filter->ingress_dest_ipv6_addr_mask[13]&acl_filter->ingress_dest_ipv6_addr_mask[14]&acl_filter->ingress_dest_ipv6_addr_mask[15])==0xff) {
				snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip6-dst %s%s ", cmd_ebtable_p_string==2?"":EDP_ACL_CMD_P_IPV6, acl_filter->filter_fields_inverse&INGRESS_IPV6_DIP_BIT?"! ":"",
					_rt_edp_inet_n6toa_p(acl_filter->ingress_dest_ipv6_addr, ipv6_addr));
				cmd_ebtable_p_string=2;
			}else{
				ASSERT_EQ(_rt_edp_prefix_get_by_ipv6_mask(acl_filter->ingress_dest_ipv6_addr_mask,&ipv6_prefix_by_mask),RT_EDP_ERR_OK);
				snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip6-dst %s%s/%d ", cmd_ebtable_p_string==2?"":EDP_ACL_CMD_P_IPV6, acl_filter->filter_fields_inverse&INGRESS_IPV6_DIP_BIT?"! ":"",
					_rt_edp_inet_n6toa_p(acl_filter->ingress_dest_ipv6_addr, ipv6_addr),ipv6_prefix_by_mask);
				cmd_ebtable_p_string=2;
			}
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV4_TAGIF_BIT )
	{
		//check table decision: INGRESS_IPV4_TAGIF_BIT and ingress_ipv4_tagif
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_TAGIF_BIT)
	{
		//check table decision: INGRESS_IPV6_TAGIF_BIT and ingress_ipv6_tagif
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_SIP_RANGE_BIT)
	{
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_XTABLES) {
			snprintf(tmp_buf, sizeof(tmp_buf), "-m iprange %s--src-range %s-%s ", acl_filter->filter_fields_inverse&INGRESS_IPV6_SIP_RANGE_BIT?"! ":"",
				_rt_edp_inet_n6toa_p(acl_filter->ingress_src_ipv6_addr_start, ipv6_addr),_rt_edp_inet_n6toa_p(acl_filter->ingress_src_ipv6_addr_end, ipv6_addr_end));
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			ASSERT_EQ(_rt_edp_maskLength_get_by_ipv6_range(acl_filter->ingress_src_ipv6_addr_start,acl_filter->ingress_src_ipv6_addr_end,&unmask_length),RT_EDP_ERR_OK);
			snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip6-src %s%s/%d ", cmd_ebtable_p_string==2?"":EDP_ACL_CMD_P_IPV6, acl_filter->filter_fields_inverse&INGRESS_IPV6_SIP_RANGE_BIT?"! ":"",
				_rt_edp_inet_n6toa_p(acl_filter->ingress_src_ipv6_addr_start, ipv6_addr),128-unmask_length);
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
			cmd_ebtable_p_string=2;
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_DIP_RANGE_BIT)
	{
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_XTABLES) {
			snprintf(tmp_buf, sizeof(tmp_buf), "-m iprange %s--dst-range %s-%s ", acl_filter->filter_fields_inverse&INGRESS_IPV6_DIP_RANGE_BIT?"! ":"",
				_rt_edp_inet_n6toa_p(acl_filter->ingress_dest_ipv6_addr_start, ipv6_addr),_rt_edp_inet_n6toa_p(acl_filter->ingress_dest_ipv6_addr_end, ipv6_addr_end));
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			ASSERT_EQ(_rt_edp_maskLength_get_by_ipv6_range(acl_filter->ingress_dest_ipv6_addr_start,acl_filter->ingress_dest_ipv6_addr_end,&unmask_length),RT_EDP_ERR_OK);
			snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip6-dst %s%s/%d ", cmd_ebtable_p_string==2?"":EDP_ACL_CMD_P_IPV6, acl_filter->filter_fields_inverse&INGRESS_IPV6_DIP_RANGE_BIT?"! ":"",
				_rt_edp_inet_n6toa_p(acl_filter->ingress_dest_ipv6_addr_start, ipv6_addr),128-unmask_length);
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
			cmd_ebtable_p_string=2;
		}
	}

	if(acl_filter->filter_fields & INGRESS_L4_SPORT_RANGE_BIT)
	{
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_XTABLES) {
			if(tables_internal_check&ACL_INTERNAL_CHECK_PROTO) {
				if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_PROTO_TCP) {
					snprintf(tmp_buf2, sizeof(tmp_buf2), "%s", cmd_xptable_p_string==1?"":EDP_ACL_CMD_P_TCP);
					cmd_xptable_p_string=1;
				}else if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_PROTO_UDP) {
					snprintf(tmp_buf2, sizeof(tmp_buf2), "%s", cmd_xptable_p_string==2?"":EDP_ACL_CMD_P_UDP);
					cmd_xptable_p_string=2;
				}
				if(acl_filter->ingress_src_l4_port_start == acl_filter->ingress_src_l4_port_end)	//SINGLE TCP/UDP Port
					snprintf(tmp_buf, sizeof(tmp_buf), "%s%s--sport %d ", tmp_buf2, acl_filter->filter_fields_inverse&INGRESS_L4_SPORT_RANGE_BIT?"! ":"", acl_filter->ingress_src_l4_port_start);
				else
					snprintf(tmp_buf, sizeof(tmp_buf), "%s%s--sport %d:%d ", tmp_buf2, acl_filter->filter_fields_inverse&INGRESS_L4_SPORT_RANGE_BIT?"! ":"", acl_filter->ingress_src_l4_port_start, acl_filter->ingress_src_l4_port_end);
				ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
			}
			else
				ptr_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL;
		}
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			//The source port or port range for the IP protocols 6 (TCP), 17 (UDP), 33 (DCCP) or 132 (SCTP).
			if(tables_internal_check&ACL_INTERNAL_CHECK_ETHERTYPE) {
				if(tables_internal_check&ACL_INTERNAL_CHECK_PROTO) {
					if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_PROTO_TCP) {
						snprintf(tmp_buf2, sizeof(tmp_buf2), "%s", cmd_xptable_p_string==1?"":EDP_ACL_CMD_P_TCP);
						cmd_xptable_p_string=1;
					}else if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_PROTO_UDP) {
						snprintf(tmp_buf2, sizeof(tmp_buf2), "%s", cmd_xptable_p_string==2?"":EDP_ACL_CMD_P_UDP);
						cmd_xptable_p_string=2;
					}
					if(acl_filter->ingress_src_l4_port_start == acl_filter->ingress_src_l4_port_end) {	//SINGLE TCP/UDP Port
						if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV4) {
							snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip-proto %s%s--ip-sport %d ", cmd_ebtable_p_string==1?"":EDP_ACL_CMD_P_IPV4, tmp_buf2, acl_filter->filter_fields_inverse&INGRESS_L4_SPORT_RANGE_BIT?"! ":"", acl_filter->ingress_src_l4_port_start);
							cmd_ebtable_p_string=1;
						}
						else {	//RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV6
							snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip6-proto %s%s--ip6-sport %d ", cmd_ebtable_p_string==2?"":EDP_ACL_CMD_P_IPV6, tmp_buf2, acl_filter->filter_fields_inverse&INGRESS_L4_SPORT_RANGE_BIT?"! ":"", acl_filter->ingress_src_l4_port_start);
							cmd_ebtable_p_string=2;
						}
					}else{
						if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV4) {
							snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip-proto %s%s--ip-sport %d:%d ", cmd_ebtable_p_string==1?"":EDP_ACL_CMD_P_IPV4, tmp_buf2, acl_filter->filter_fields_inverse&INGRESS_L4_SPORT_RANGE_BIT?"! ":"", acl_filter->ingress_src_l4_port_start, acl_filter->ingress_src_l4_port_end);
							cmd_ebtable_p_string=1;
						}
						else {	//RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV6
							snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip6-proto %s%s--ip6-sport %d:%d ", cmd_ebtable_p_string==2?"":EDP_ACL_CMD_P_IPV6, tmp_buf2, acl_filter->filter_fields_inverse&INGRESS_L4_SPORT_RANGE_BIT?"! ":"", acl_filter->ingress_src_l4_port_start, acl_filter->ingress_src_l4_port_end);
							cmd_ebtable_p_string=2;
						}
					}
					ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
				}
				else
					ptr_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
			}
			else
				ptr_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
		}
	}

	if(acl_filter->filter_fields & INGRESS_L4_DPORT_RANGE_BIT)
	{
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_XTABLES) {
			if(tables_internal_check&ACL_INTERNAL_CHECK_PROTO) {
				if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_PROTO_TCP) {
					snprintf(tmp_buf2, sizeof(tmp_buf2), "%s", cmd_xptable_p_string==1?"":EDP_ACL_CMD_P_TCP);
					cmd_xptable_p_string=1;
				}else if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_PROTO_UDP) {
					snprintf(tmp_buf2, sizeof(tmp_buf2), "%s", cmd_xptable_p_string==2?"":EDP_ACL_CMD_P_UDP);
					cmd_xptable_p_string=2;
				}
				if(acl_filter->ingress_dest_l4_port_start == acl_filter->ingress_dest_l4_port_end)	//SINGLE TCP/UDP Port
					snprintf(tmp_buf, sizeof(tmp_buf), "%s%s--dport %d ", tmp_buf2, acl_filter->filter_fields_inverse&INGRESS_L4_DPORT_RANGE_BIT?"! ":"", acl_filter->ingress_dest_l4_port_start);
				else
					snprintf(tmp_buf, sizeof(tmp_buf), "%s%s--dport %d:%d ", tmp_buf2, acl_filter->filter_fields_inverse&INGRESS_L4_DPORT_RANGE_BIT?"! ":"", acl_filter->ingress_dest_l4_port_start, acl_filter->ingress_dest_l4_port_end);
				ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
			}
			else
				ptr_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL;
		}
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			//The source port or port range for the IP protocols 6 (TCP), 17 (UDP), 33 (DCCP) or 132 (SCTP).
			if(tables_internal_check&ACL_INTERNAL_CHECK_ETHERTYPE) {
				if(tables_internal_check&ACL_INTERNAL_CHECK_PROTO) {
					if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_PROTO_TCP) {
						snprintf(tmp_buf2, sizeof(tmp_buf2), "%s", cmd_xptable_p_string==1?"":EDP_ACL_CMD_P_TCP);
						cmd_xptable_p_string=1;
					}else if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_PROTO_UDP) {
						snprintf(tmp_buf2, sizeof(tmp_buf2), "%s", cmd_xptable_p_string==2?"":EDP_ACL_CMD_P_UDP);
						cmd_xptable_p_string=2;
					}
					if(acl_filter->ingress_dest_l4_port_start == acl_filter->ingress_dest_l4_port_end) {	//SINGLE TCP/UDP Port
						if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV4) {
							snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip-proto %s%s--ip-dport %d ", cmd_ebtable_p_string==1?"":EDP_ACL_CMD_P_IPV4, tmp_buf2, acl_filter->filter_fields_inverse&INGRESS_L4_DPORT_RANGE_BIT?"! ":"", acl_filter->ingress_dest_l4_port_start);
							cmd_ebtable_p_string=1;
						}
						else {	//RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV6
							snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip6-proto %s%s--ip6-dport %d ", cmd_ebtable_p_string==2?"":EDP_ACL_CMD_P_IPV6, tmp_buf2, acl_filter->filter_fields_inverse&INGRESS_L4_DPORT_RANGE_BIT?"! ":"", acl_filter->ingress_dest_l4_port_start);
							cmd_ebtable_p_string=2;
						}
					}else{
						if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV4) {
							snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip-proto %s%s--ip-dport %d:%d ", cmd_ebtable_p_string==1?"":EDP_ACL_CMD_P_IPV4, tmp_buf2, acl_filter->filter_fields_inverse&INGRESS_L4_DPORT_RANGE_BIT?"! ":"", acl_filter->ingress_dest_l4_port_start, acl_filter->ingress_dest_l4_port_end);
							cmd_ebtable_p_string=1;
						}
						else {	//RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV6
							snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip6-proto %s%s--ip6-dport %d:%d ", cmd_ebtable_p_string==2?"":EDP_ACL_CMD_P_IPV6, tmp_buf2, acl_filter->filter_fields_inverse&INGRESS_L4_DPORT_RANGE_BIT?"! ":"", acl_filter->ingress_dest_l4_port_start, acl_filter->ingress_dest_l4_port_end);
							cmd_ebtable_p_string=2;
						}
					}
					ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
				}
				else
					ptr_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
			}
			else
				ptr_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV4_SIP_RANGE_BIT)
	{
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_XTABLES) {
			if(acl_filter->ingress_src_ipv4_addr_start == acl_filter->ingress_src_ipv4_addr_end ) {
				snprintf(tmp_buf, sizeof(tmp_buf), "%s-s %s ", acl_filter->filter_fields_inverse&INGRESS_IPV4_SIP_RANGE_BIT?"! ":"", _rt_edp_inet_ntoa_p(acl_filter->ingress_src_ipv4_addr_start, ipv4_addr));
			}else{
				snprintf(tmp_buf, sizeof(tmp_buf), "-m iprange %s--src-range %s-%s ", acl_filter->filter_fields_inverse&INGRESS_IPV4_SIP_RANGE_BIT?"! ":"",
					_rt_edp_inet_ntoa_p(acl_filter->ingress_src_ipv4_addr_start, ipv4_addr),_rt_edp_inet_ntoa_p(acl_filter->ingress_src_ipv4_addr_end, ipv4_addr_end));
			}
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			if(acl_filter->ingress_src_ipv4_addr_start == acl_filter->ingress_src_ipv4_addr_end ) {
				snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip-src %s%s ", cmd_ebtable_p_string==1?"":EDP_ACL_CMD_P_IPV4, acl_filter->filter_fields_inverse&INGRESS_IPV4_SIP_RANGE_BIT?"! ":"",
					_rt_edp_inet_ntoa_p(acl_filter->ingress_src_ipv4_addr_start, ipv4_addr));
				cmd_ebtable_p_string=1;
			}else{
				ASSERT_EQ(_rt_edp_maskLength_get_by_ipv4_range(acl_filter->ingress_src_ipv4_addr_start,acl_filter->ingress_src_ipv4_addr_end,&unmask_length),RT_EDP_ERR_OK);
				snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip-src %s%s/%d ", cmd_ebtable_p_string==1?"":EDP_ACL_CMD_P_IPV4, acl_filter->filter_fields_inverse&INGRESS_IPV4_SIP_RANGE_BIT?"! ":"",
					_rt_edp_inet_ntoa_p(acl_filter->ingress_src_ipv4_addr_start, ipv4_addr),32-unmask_length);
				cmd_ebtable_p_string=1;
			}
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV4_DIP_RANGE_BIT)
	{
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_XTABLES) {
			if(acl_filter->ingress_dest_ipv4_addr_start == acl_filter->ingress_dest_ipv4_addr_end ) {
				snprintf(tmp_buf, sizeof(tmp_buf), "%s-d %s ", acl_filter->filter_fields_inverse&INGRESS_IPV4_DIP_RANGE_BIT?"! ":"", _rt_edp_inet_ntoa_p(acl_filter->ingress_dest_ipv4_addr_start, ipv4_addr));
			}else{
				snprintf(tmp_buf, sizeof(tmp_buf), "-m iprange %s--dst-range %s-%s ", acl_filter->filter_fields_inverse&INGRESS_IPV4_DIP_RANGE_BIT?"! ":"",
					_rt_edp_inet_ntoa_p(acl_filter->ingress_dest_ipv4_addr_start, ipv4_addr),_rt_edp_inet_ntoa_p(acl_filter->ingress_dest_ipv4_addr_end, ipv4_addr_end));
			}
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
		if(tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_EBTABLES) {
			if(acl_filter->ingress_dest_ipv4_addr_start == acl_filter->ingress_dest_ipv4_addr_end ) {
				snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip-dst %s%s ", cmd_ebtable_p_string==1?"":EDP_ACL_CMD_P_IPV4, acl_filter->filter_fields_inverse&INGRESS_IPV4_DIP_RANGE_BIT?"! ":"",
					_rt_edp_inet_ntoa_p(acl_filter->ingress_dest_ipv4_addr_start, ipv4_addr));
				cmd_ebtable_p_string=1;
			}else{
				ASSERT_EQ(_rt_edp_maskLength_get_by_ipv4_range(acl_filter->ingress_dest_ipv4_addr_start,acl_filter->ingress_dest_ipv4_addr_end,&unmask_length),RT_EDP_ERR_OK);
				snprintf(tmp_buf, sizeof(tmp_buf), "%s--ip-dst %s%s/%d ", cmd_ebtable_p_string==1?"":EDP_ACL_CMD_P_IPV4, acl_filter->filter_fields_inverse&INGRESS_IPV4_DIP_RANGE_BIT?"! ":"",
					_rt_edp_inet_ntoa_p(acl_filter->ingress_dest_ipv4_addr_start, ipv4_addr),32-unmask_length);
				cmd_ebtable_p_string=1;
			}
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
		}
	}

	if(acl_filter->filter_fields & INGRESS_IPV6_FLOWLABEL_BIT)
	{
		WARNING("Linux tables not support ingress_ipv6_flow_label");
		return(RT_EDP_ERR_INVALID_PARAM);
	}

	aclSWEntry->ptr_nsup_chain |= ptr_nsup_chain;
	aclSWEntry->ptr_nsup_table |= ptr_nsup_table;

	return (RT_EDP_ERR_OK);
}

static int _rt_edp_linux_action_translate(rtk_rg_aclFilterAndQos_t *acl_filter, rt_edp_aclFilterEntry_t* aclSWEntry)
{
	int i;
	char *pattern_buf = rt_edp_db.systemGlobal.acl_cmd_buff;
	rt_edp_aclSWEntry_internal_check_table_t act_nsup_table = 0;
	rt_edp_aclSWEntry_internal_check_chain_t act_nsup_chain = 0;
	unsigned char tmp_buf[96];

	/*set actions*/
	switch(acl_filter->action_type)
	{
		case ACL_ACTION_TYPE_DROP:
			snprintf(tmp_buf, sizeof(tmp_buf), "-j DROP");
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
			break;

		case ACL_ACTION_TYPE_PERMIT:
		case ACL_ACTION_TYPE_TRAP:
			snprintf(tmp_buf, sizeof(tmp_buf), "-j RETURN");
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
			break;

		case ACL_ACTION_TYPE_QOS:
			act_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
			for(i=0; (0x1<<i)<ACL_ACTION_QOS_END; i++)
			{
				switch((acl_filter->qos_actions&(1<<i)))
				{
					case ACL_ACTION_NOP_BIT: break;
					case ACL_ACTION_1P_REMARKING_BIT:
						//WARNING("ACL_ACTION_1P_REMARKING_BIT is not support by linux table");
						//return(RT_EDP_ERR_INVALID_PARAM);
						act_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL|RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
						break;

					case ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT:
						//WARNING("ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT is not support by linux table");
						//return(RT_EDP_ERR_INVALID_PARAM);
						act_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL|RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
						break;

					case ACL_ACTION_DSCP_REMARKING_BIT:	//only can use in MANGLE TABLE
						snprintf(tmp_buf, sizeof(tmp_buf), "-j DSCP --set-dscp %d ", acl_filter->action_dscp_remarking_pri);
						ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
						break;

					case ACL_ACTION_TOS_TC_REMARKING_BIT: //only can use in MANGLE TABLE
						snprintf(tmp_buf, sizeof(tmp_buf), "-j TOS --set-tos %d ", acl_filter->action_tos_tc_remarking_pri);
						ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
						break;

					case ACL_ACTION_QUEUE_ID_BIT:
						//WARNING("ACL_ACTION_QUEUE_ID_BIT is not support by linux table");
						//return(RT_EDP_ERR_INVALID_PARAM);
						act_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL|RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
						break;

					case ACL_ACTION_SHARE_METER_BIT:
						//WARNING("ACL_ACTION_SHARE_METER_BIT is not support by linux table");
						//return(RT_EDP_ERR_INVALID_PARAM);
						act_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL|RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
						break;

					case ACL_ACTION_LOG_COUNTER_BIT:
						//WARNING("ACL_ACTION_LOG_COUNTER_BIT is not support by linux table");
						//return(RT_EDP_ERR_INVALID_PARAM);
						act_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL|RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
						break;

					case ACL_ACTION_STREAM_ID_OR_LLID_BIT:
						//WARNING("ACL_ACTION_STREAM_ID_OR_LLID_BIT is not support by linux table");
						//return(RT_EDP_ERR_INVALID_PARAM);
						act_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL|RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
						break;

					case ACL_ACTION_ACL_PRIORITY_BIT:	//only can use in FORWARD/OUTPUT/POSTROUTING Chain
						act_nsup_chain |= RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_IN;
						snprintf(tmp_buf, sizeof(tmp_buf), "-j CLASSIFY --set-class %d:%d ", acl_filter->action_acl_priority, acl_filter->action_acl_priority);
						ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
						break;

					case ACL_ACTION_ACL_CVLANTAG_BIT:
						//WARNING("ACL_ACTION_ACL_CVLANTAG_BIT is not support by linux table");
						//return(RT_EDP_ERR_INVALID_PARAM);
						act_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL|RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
						break;

					case ACL_ACTION_ACL_SVLANTAG_BIT:
						//WARNING("ACL_ACTION_ACL_SVLANTAG_BIT is not support by linux table");
						//return(RT_EDP_ERR_INVALID_PARAM);
						act_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL|RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
						break;

					case ACL_ACTION_ACL_INGRESS_VID_BIT:
						//WARNING("ACL_ACTION_ACL_INGRESS_VID_BIT is not support by linux table");
						//return(RT_EDP_ERR_INVALID_PARAM);
						act_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL|RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
						break;

					case ACL_ACTION_DS_UNIMASK_BIT:
						//WARNING("ACL_ACTION_DS_UNIMASK_BIT is not support by linux table");
						//return(RT_EDP_ERR_INVALID_PARAM);
						act_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL|RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
						break;
					case ACL_ACTION_REDIRECT_BIT:
						//WARNING("ACL_ACTION_REDIRECT_BIT is not support by linux table");
						//return(RT_EDP_ERR_INVALID_PARAM);
						act_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL|RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
						break;

					case ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT:
						snprintf(tmp_buf, sizeof(tmp_buf), "-j CLASSIFY --set-class %d:%d ", acl_filter->egress_internal_priority, acl_filter->egress_internal_priority);
						ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
						break;
					case ACL_ACTION_QOS_END:
						break;
					default:
						break;
				}
			}
			break;

		case ACL_ACTION_TYPE_TRAP_TO_PS:
			snprintf(tmp_buf, sizeof(tmp_buf), "-j ACCEPT");
			ACL_CMD_STRNCAT(pattern_buf, tmp_buf);
			break;

		case ACL_ACTION_TYPE_POLICY_ROUTE:
			//WARNING("ACL_ACTION_TYPE_POLICY_ROUTE is under development");
			//return(RT_EDP_ERR_INVALID_PARAM);
			act_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL|RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
			break;

		case ACL_ACTION_TYPE_TRAP_WITH_PRIORITY:
			//WARNING("ACL_ACTION_TYPE_TRAP_WITH_PRIORITY is not support by linux table");
			//return(RT_EDP_ERR_INVALID_PARAM);
			act_nsup_table |= RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL|RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL;
			break;

		default:
		break;
	}

	aclSWEntry->act_nsup_chain |= act_nsup_chain;
	aclSWEntry->act_nsup_table |= act_nsup_table;

	return (RT_EDP_ERR_OK);
}

int _rt_edp_pipe_cmd_acl(rt_edp_aclFilterEntry_t* aclSWEntry, int idx, char *cmd)
{
	int ret;

	ACL_CTRL_EDP("CMD=%s", cmd);
	ret = _rt_edp_pipe_cmd(cmd);
	if(ret == 0) {
		aclSWEntry->linux_used_table_index[idx][1]++; //count
		aclSWEntry->linux_used_table |= (0x1<<idx);
	}

	return (RT_EDP_ERR_OK);
}

static int _rt_edp_linux_cmd_add(rtk_rg_aclFilterAndQos_t *acl_filter, rt_edp_aclFilterEntry_t* aclSWEntry, int tb_idx, int check_tb_idx)
{
	int i, pre_idx;
	unsigned char tmp_buf[96];
	char cmd_buff[EDP_CB_CMD_BUFF_SIZE];
	int dev_port_map_max = sizeof(rt_edp_db.dev_port_map) / sizeof(rt_edp_db.dev_port_map[0]);

	if(acl_filter->filter_fields & INGRESS_PORT_BIT) {
		for(i=0; i<dev_port_map_max; i++) {
			if((acl_filter->ingress_port_mask.portmask & (1<<i))==0x0)
				continue;
			if((i>1) && (strcmp(rt_edp_db.dev_port_map[pre_idx].dev_name, rt_edp_db.dev_port_map[i].dev_name)==0x0))
				continue;

			if((i <= RT_EDP_MAC_PORT_PON) &&
				((strcmp(linux_table[check_tb_idx].chain_buf, EDP_ACL_XTABLES_INPUT)==0x0) ||
				(strcmp(linux_table[check_tb_idx].chain_buf, EDP_ACL_XTABLES_FORWARD)==0x0) ||
				(strcmp(linux_table[check_tb_idx].chain_buf, EDP_ACL_XTABLES_PREROUTING)==0x0))) {
				//lan port/wan port should add in input/forward/prerouting
				if(tb_idx==RT_EDP_ACL_INTERNAL_CHECK_XTABLES_IDX)
					snprintf(tmp_buf, sizeof(tmp_buf), "-m physdev --physdev-in %s+", rt_edp_db.dev_port_map[i].dev_name);
				else if(tb_idx==RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_IDX)
					snprintf(tmp_buf, sizeof(tmp_buf), "-i %s+", rt_edp_db.dev_port_map[i].dev_name);

				snprintf(cmd_buff, sizeof(cmd_buff), "%s %s -A %s %s %s", linux_table[check_tb_idx].utility_buf, linux_table[check_tb_idx].table_buf, linux_table[check_tb_idx].chain_buf, tmp_buf, rt_edp_db.systemGlobal.acl_cmd_buff);
				ASSERT_EQ(_rt_edp_pipe_cmd_acl(aclSWEntry, check_tb_idx, cmd_buff),RT_EDP_ERR_OK);
			}else if((i > RT_EDP_MAC_PORT_PON) &&
				((strcmp(linux_table[check_tb_idx].chain_buf, EDP_ACL_XTABLES_OUTPUT)==0x0) ||
				(strcmp(linux_table[check_tb_idx].chain_buf, EDP_ACL_XTABLES_POSTROUTING)==0x0))) {
				//cpu port rule should add in output/postrouting
				snprintf(cmd_buff, sizeof(cmd_buff), "%s %s -A %s %s", linux_table[check_tb_idx].utility_buf, linux_table[check_tb_idx].table_buf, linux_table[check_tb_idx].chain_buf, rt_edp_db.systemGlobal.acl_cmd_buff);
				ASSERT_EQ(_rt_edp_pipe_cmd_acl(aclSWEntry, check_tb_idx, cmd_buff),RT_EDP_ERR_OK);
			}else
				continue;

			//prevent to add same rule again
			pre_idx = i;
		}
	}else {
		snprintf(cmd_buff, sizeof(cmd_buff), "%s %s -A %s %s", linux_table[check_tb_idx].utility_buf, linux_table[check_tb_idx].table_buf, linux_table[check_tb_idx].chain_buf, rt_edp_db.systemGlobal.acl_cmd_buff);
		ASSERT_EQ(_rt_edp_pipe_cmd_acl(aclSWEntry, check_tb_idx, cmd_buff),RT_EDP_ERR_OK);
	}

	return (RT_EDP_ERR_OK);
}

static int _rt_edp_aclSWEntry_to_linux_add_tables(rtk_rg_aclFilterAndQos_t *acl_filter,rt_edp_aclFilterEntry_t* aclSWEntry,int acl_filter_idx)
{
	int ret, i, j, k, m, n;
	uint8 tb_start, tb_end, ch_start, ch_end;
	rt_edp_aclSWEntry_internal_check_field_t tables_internal_check = 0;
	unsigned char utility_buf[32];

	ret = RT_EDP_ERR_OK;
	aclSWEntry->linux_used_table = 0;
	aclSWEntry->tables_internal_check = 0;
	aclSWEntry->ptr_nsup_chain = 0;
	aclSWEntry->ptr_nsup_table = 0;
	aclSWEntry->act_nsup_chain = 0;
	aclSWEntry->act_nsup_table = 0;

	for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++) {
		if(rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[i]==-1)
			break;
		if(rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[i] == acl_filter_idx) {
			if(i>0) {
				for(j=0; j<RT_EDP_ACL_USED_TABLE_END; j++) {
					aclSWEntry->linux_used_table_index[j][0]= rt_edp_db.systemGlobal.acl_SW_table_entry[(rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[i-1])].linux_used_table_index[j][0] + rt_edp_db.systemGlobal.acl_SW_table_entry[(rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[i-1])].linux_used_table_index[j][1];
				}
			}
			break;
		}
	}

	//update aclSWEntry->tables_internal_check
	ASSERT_EQ(_rt_edp_linux_table_decision(acl_filter, aclSWEntry),RT_EDP_ERR_OK);

	//pattern transform to string base on different table: ex iptable+tcp & iptable+udp & ip6table+tcp & ip6table+udp & ebtable+v4+tcp & ebtable+v6+udp
	for(i=RT_EDP_ACL_INTERNAL_CHECK_XTABLES_IDX; i<=RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_IDX; i++) {	//iptable or ebtable
		if((aclSWEntry->tables_internal_check & (1<<i)) == 0x0)
			continue;

		for(j=RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV4_IDX; j<=RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV6_IDX; j++) {	//ipv4 or ipv6
			if((aclSWEntry->tables_internal_check & (1<<j)) == 0x0)
				continue;

			for(k=RT_EDP_ACL_INTERNAL_CHECK_PROTO_NCARE_IDX; k<=RT_EDP_ACL_INTERNAL_CHECK_PROTO_UDP_IDX; k++) {	//tcp or udp or not care
				if((aclSWEntry->tables_internal_check & (1<<k)) == 0x0)
					continue;
				if((i==RT_EDP_ACL_INTERNAL_CHECK_XTABLES_IDX) && ((aclSWEntry->ptr_nsup_table & RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL) || (aclSWEntry->act_nsup_table & RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL)))
					continue;
				else if((i==RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_IDX) && ((aclSWEntry->ptr_nsup_table & RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL) || (aclSWEntry->act_nsup_table & RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL)))
					continue;

				bzero(&rt_edp_db.systemGlobal.acl_cmd_buff,sizeof(rt_edp_db.systemGlobal.acl_cmd_buff));
				tables_internal_check = aclSWEntry->tables_internal_check & ((1<<i) | (1<<j) | (1<<k));
				if(tables_internal_check==0x0)
					continue;

				//for ebtable, only some of pattern(list in ACL_INTERNAL_CHECK_EBTABLE_DEFAULT_IPV4) need different pattern for ipv4 & ipv6
				if((i==RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_IDX) && (tables_internal_check&RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV6) && !(acl_filter->filter_fields & ACL_INTERNAL_CHECK_EBTABLE_DEFAULT_IPV4))
					continue;

				switch(i) {
					case RT_EDP_ACL_INTERNAL_CHECK_XTABLES_IDX:
						if(j==RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV4_IDX)
							snprintf(utility_buf, sizeof(utility_buf), "%s ", EDP_IPTABLES_UTILITY);
						else
							snprintf(utility_buf, sizeof(utility_buf), "%s ", EDP_IP6TABLES_UTILITY);
						break;
					case RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_IDX:
						snprintf(utility_buf, sizeof(utility_buf), "%s ", EDP_EBTABLES_UTILITY);
						break;
					default: break;
				}

				ret = _rt_edp_linux_pattern_translate(acl_filter, aclSWEntry, tables_internal_check);
				if(ret != RT_EDP_ERR_OK) {
					ACL_CTRL_EDP("transform ACL pattern to %slinux command fail...(ret=0x%x, tables_internal_check=0x%x)", utility_buf, ret, tables_internal_check);
					continue;
				}

				ret = _rt_edp_linux_action_translate(acl_filter, aclSWEntry);
				if(ret != RT_EDP_ERR_OK) {
					ACL_CTRL_EDP("transform ACL action to %slinux command fail...(ret=0x%x, tables_internal_check=0x%x)", utility_buf, ret, tables_internal_check);
					continue;
				}

				_rt_edp_linux_debug_dump(tables_internal_check, aclSWEntry->ptr_nsup_table, aclSWEntry->ptr_nsup_chain, aclSWEntry->act_nsup_table, aclSWEntry->act_nsup_chain);

				/*add ACL rule*/
				for(m=0; m<RT_EDP_ACL_USED_TABLE_END; m++) {
					if(i==RT_EDP_ACL_INTERNAL_CHECK_XTABLES_IDX) {
						if((j==RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV4_IDX) && (strcmp(linux_table[m].utility_buf, EDP_IPTABLES_UTILITY)))
							continue;
						else if((j==RT_EDP_ACL_INTERNAL_CHECK_ETHERTYPE_IPV6_IDX) && (strcmp(linux_table[m].utility_buf, EDP_IP6TABLES_UTILITY)))
							continue;
						tb_start = RT_EDP_ACL_INTERNAL_CHECK_XTABLES_TABLE_ALL_IDX;
						tb_end = RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL_IDX;
						ch_start = RT_EDP_ACL_INTERNAL_CHECK_XTABLES_CHAIN_ALL_IDX;
						ch_end = RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_CHAIN_ALL_IDX;
					}
					else if(i==RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_IDX) {
						if(strcmp(linux_table[m].utility_buf, EDP_EBTABLES_UTILITY))
							continue;
						tb_start = RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_TABLE_ALL_IDX;
						tb_end = RT_EDP_ACL_INTERNAL_CHECK_TABLE_MAX_IDX;
						ch_start = RT_EDP_ACL_INTERNAL_CHECK_EBTABLES_CHAIN_ALL_IDX;
						ch_end = RT_EDP_ACL_INTERNAL_CHECK_CHAIN_MAX_IDX;
					}

					//this rule include pattern/action that not support in some of linux table or chain
					if((aclSWEntry->ptr_nsup_table & (1<<tb_start)) || (aclSWEntry->act_nsup_table & (1<<tb_start)))
						continue;
					if((aclSWEntry->ptr_nsup_chain & (1<<ch_start)) || (aclSWEntry->act_nsup_chain & (1<<ch_start)))
						continue;

					for(n=tb_start; n<tb_end; n++) {
						if(strcmp(linux_table[m].table_buf, linux_table_map[n].name_buf)==0x0)
							break;
					}
					if((aclSWEntry->ptr_nsup_table & (1<<n)) || (aclSWEntry->act_nsup_table & (1<<n)))
						continue;
					for(n=ch_start; n<ch_end; n++) {
						if(strcmp(linux_table[m].chain_buf, linux_chain_map[n].name_buf)==0x0)
							break;
					}
					if((aclSWEntry->ptr_nsup_chain & (1<<n)) || (aclSWEntry->act_nsup_chain & (1<<n)))
						continue;

					ASSERT_EQ(_rt_edp_linux_cmd_add(acl_filter, aclSWEntry, i, m),RT_EDP_ERR_OK);
				}
			}
		}
	}

	ACL_CTRL_EDP("transform result: %s", rt_edp_db.systemGlobal.acl_cmd_buff);
	_rt_edp_linux_debug_dump(aclSWEntry->tables_internal_check, aclSWEntry->ptr_nsup_table, aclSWEntry->ptr_nsup_chain, aclSWEntry->act_nsup_table, aclSWEntry->act_nsup_chain);

	return (RT_EDP_ERR_OK);
}


int32 _rt_edp_acl_pattern_mask_check(rtk_rg_aclFilterAndQos_t *acl_filter)
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

	return (RT_EDP_ERR_OK);
}


/*(3)literomeDriver mapping APIs*/
int32 _rt_edp_aclFilterAndQos_add(rtk_rg_aclFilterAndQos_t *acl_filter, int *acl_filter_idx)
{
	int i,ret;
	int aclSWEntryIdx;
	bzero(&rt_edp_db.systemGlobal.aclSWEntry,sizeof(rt_edp_aclFilterEntry_t));

	/*Skip ----------------- rg has been init check ----------------- Skip*/

	//check input parameter
	if(acl_filter == NULL || acl_filter_idx == NULL)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_NULL_POINTER);

	if(rt_edp_db.systemGlobal.acl_rg_add_parameter_dump){
		rtlglue_printf("RG ACL parameter check:\n");
		_dump_rt_edp_acl_entry_content(NULL, acl_filter);
	}

	ASSERT_EQ(_rt_edp_aclSWEntry_not_support(acl_filter),RT_EDP_ERR_OK);

	//Downward compatibility: if pattern with zero carebit, consider as all care.
	ASSERT_EQ(_rt_edp_acl_pattern_mask_check(acl_filter),RT_EDP_ERR_OK);


	/*Skip ----------------- multicast protection add ----------------- Skip*/

	/*Skip ----------------- pre add HW ACL check ----------------- Skip*/

	rt_edp_db.systemGlobal.aclSWEntry.acl_filter = *acl_filter;
	rt_edp_db.systemGlobal.aclSWEntry.valid=RT_EDP_ENABLED;

	ret=_rt_edp_aclSWEntry_empty_find(&aclSWEntryIdx);
	if(ret!=RT_EDP_ERR_OK) goto aclFailed;

	ret=_rt_edp_aclSWEntry_set(aclSWEntryIdx,rt_edp_db.systemGlobal.aclSWEntry);
	if(ret!=RT_EDP_ERR_OK) goto aclFailed;

	*acl_filter_idx = aclSWEntryIdx;
	ACL_CTRL_EDP("add aclSWEntry[%d]",*acl_filter_idx);

	//count the total aclSWEntry size
	rt_edp_db.systemGlobal.acl_SW_table_entry_size=0;
	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++)
	{
		if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].valid==RT_EDP_ENABLED)
			rt_edp_db.systemGlobal.acl_SW_table_entry_size++;
	}

	//readd all acl rules for sorting: action  INGRESS_ALL(sort by weight) > L34 DROP/TRAP/PERMIT > L34 QoS
	ret = _rt_edp_aclSWEntry_and_asic_rearrange();
	if(rt_edp_db.systemGlobal.stop_add_acl) {
		WARNING("add to RG ACL Faild due to ACL rearrange fail!");
		ret = RT_EDP_ERR_ACL_NOT_SUPPORT;
		goto aclFailed;
	}
	if(ret!=RT_EDP_ERR_OK) goto aclFailed;

	/*Skip ----------------- short cut clear ----------------- Skip*/

	if(rt_edp_db.systemGlobal.acl_rg_add_parameter_dump){
		rtlglue_printf("add to RG ACL[%d] success!\n",*acl_filter_idx);
	}


	/*Skip ----------------- update sw share meter counter ----------------- Skip*/

	/*Skip ----------------- multicast protection del ----------------- Skip*/

	return (RT_EDP_ERR_OK);

aclFailed:

	if(rt_edp_db.systemGlobal.acl_rg_add_parameter_dump){
		rtlglue_printf("add to RG ACL Faild! ret=0x%x\n",ret);
	}

	/*Skip ----------------- multicast protection del ----------------- Skip*/

	if(rt_edp_db.systemGlobal.stop_add_acl) {
		ASSERT_EQ(_rt_edp_aclFilterAndQos_del(*acl_filter_idx),RT_EDP_ERR_OK);
	}

	return ret;

}




int32 _rt_edp_aclFilterAndQos_del(int acl_filter_idx)
{
	int i,ret=RT_EDP_ERR_FAILED;
	bzero(&rt_edp_db.systemGlobal.aclSWEntry,sizeof(rt_edp_aclFilterEntry_t));

	/*Skip ----------------- rg has been init check ----------------- Skip*/

	if(acl_filter_idx<0 || acl_filter_idx>=MAX_ACL_SW_ENTRY_SIZE){
		WARNING("invalid ACL index(%d)!",acl_filter_idx);
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INDEX_OUT_OF_RANGE);
	}

	/*Skip ----------------- multicast protection add ----------------- Skip*/

	/*Skip ----------------- update sw share meter counter ----------------- Skip*/

	//clean aclSWEntry
	ret = _rt_edp_aclSWEntry_set(acl_filter_idx, rt_edp_db.systemGlobal.aclSWEntry);
	if(ret!=RT_EDP_ERR_OK) goto aclFailed;

	//rearrange the ACL & CF ASIC to avoid discontinuous entry
	ret = _rt_edp_aclSWEntry_and_asic_rearrange();
	if(ret!=RT_EDP_ERR_OK) goto aclFailed;

	//count the total aclSWEntry size
	rt_edp_db.systemGlobal.acl_SW_table_entry_size=0;
	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++)
	{
		if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].valid==RT_EDP_ENABLED)
			rt_edp_db.systemGlobal.acl_SW_table_entry_size++;
	}

	ACL_CTRL_EDP("delete aclSWEntry[%d]",acl_filter_idx);

	/*Skip ----------------- short cut clear ----------------- Skip*/

	if(rt_edp_db.systemGlobal.acl_rg_add_parameter_dump){
		rtlglue_printf("RG ACL parameter check:\n");
		_rt_edp_aclSWEntry_dump();
	}

	/*Skip ----------------- multicast protection del ----------------- Skip*/
	return (RT_EDP_ERR_OK);

aclFailed:

	/*Skip ----------------- multicast protection del ----------------- Skip*/
	return ret;


}


int32 _rt_edp_aclFilterAndQos_find(rtk_rg_aclFilterAndQos_t *acl_filter, int *valid_idx)
{
	//search for the first not empty entry after valid_idx.
	int i;
	bzero(&rt_edp_db.systemGlobal.aclSWEntry, sizeof(rt_edp_db.systemGlobal.aclSWEntry));
	bzero(&rt_edp_db.systemGlobal.empty_aclSWEntry,sizeof(rt_edp_db.systemGlobal.empty_aclSWEntry));	//aclSWEntryEmpty

	/*Skip ----------------- rg has been init check ----------------- Skip*/

	if(acl_filter==NULL || valid_idx==NULL)
		RT_EDP_RETURN_ERR(RT_EDP_ERR_NULL_POINTER);

	if(*valid_idx==-1){
		for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++){
			ASSERT_EQ(_rt_edp_aclSWEntry_get(i,&rt_edp_db.systemGlobal.aclSWEntry),RT_EDP_ERR_OK);
			if(!memcmp(&(rt_edp_db.systemGlobal.aclSWEntry.acl_filter),acl_filter,sizeof(rtk_rg_aclFilterAndQos_t)))//search the same with acl_filter
			{
				*valid_idx=i;
				*acl_filter = rt_edp_db.systemGlobal.aclSWEntry.acl_filter;
				ACL_CTRL_EDP("rtk_rg_aclFilterAndQos_find index=%d",*valid_idx);
				return (RT_EDP_ERR_OK);
			}

		}
	}else if((*valid_idx>=0) && (*valid_idx<MAX_ACL_SW_ENTRY_SIZE)){
		for(i=*valid_idx; i<MAX_ACL_SW_ENTRY_SIZE; i++)
		{
			ASSERT_EQ(_rt_edp_aclSWEntry_get(i,&rt_edp_db.systemGlobal.aclSWEntry),RT_EDP_ERR_OK);

			if(memcmp(&rt_edp_db.systemGlobal.aclSWEntry,&rt_edp_db.systemGlobal.empty_aclSWEntry,sizeof(rt_edp_aclFilterEntry_t)))//search the different with empty_aclSWEntry
			{
				*valid_idx=i;
				*acl_filter = rt_edp_db.systemGlobal.aclSWEntry.acl_filter;
				ACL_CTRL_EDP("rtk_rg_aclFilterAndQos_find index=%d",*valid_idx);
				return (RT_EDP_ERR_OK);
			}
		}
		ACL_CTRL_EDP("rtk_rg_aclFilterAndQos_find failed");
	}else{
		RT_EDP_RETURN_ERR(RT_EDP_ERR_INDEX_OUT_OF_RANGE);
	}
	//not found
	return (RT_EDP_ERR_ACL_SW_ENTRY_NOT_FOUND);
}



/*(4)reserved ACL related APIs*/
#if 0
int _rtk_rg_aclReservedEntry_init(void)
{

	return (RT_EDP_ERR_OK);
}

static int _rtk_rg_aclAndCfReservedRuleAddCheck(uint32 aclRsvSize, uint32 cfRsvSize)
{

	return (RT_EDP_ERR_OK);

}

int _rtk_rg_aclAndCfReservedRuleReflash(void)
{

	return RT_EDP_ERR_OK;
}

static int _rtk_rg_aclAndCfReservedRuleHeadReflash(void)
{


	return (RT_EDP_ERR_OK);
}

static int _rtk_rg_aclAndCfReservedRuleTailReflash(void)
{

	return (RT_EDP_ERR_OK);

}

int _rtk_rg_aclAndCfReservedRuleAdd(rtk_rg_aclAndCf_reserved_type_t rsvType, void *parameter)
{

	return (RT_EDP_ERR_OK);

}


int _rtk_rg_aclAndCfReservedRuleDel(rtk_rg_aclAndCf_reserved_type_t rsvType)
{

	return (RT_EDP_ERR_OK);
}
#endif


/*(5)linux datapath used APIs*/
#if defined(EDP_ACL_SUPPORT_HOOK_CHECK) && (EDP_ACL_SUPPORT_HOOK_CHECK==1)
int _rt_edp_aclDecisionClear(struct sk_buff *skb)
{
	rt_edp_skb_t *pPktHdr = &skb->edpSkbData;

	/*Skip ----------------- initialize streamID in CONFIG_GPON_FEATURE ----------------- Skip*/

	bzero(&pPktHdr->aclDecision.aclIgrDoneAction, sizeof(rt_edp_aclHitAndAction_t)-((POINTER_CAST)(&pPktHdr->aclDecision.aclIgrDoneAction)-(POINTER_CAST)(&pPktHdr->aclDecision)));

	return (RT_EDP_ERR_OK);
}

int _rt_edp_acl_pattern_dev_port_mapping(char *dev_name, int *port_idx)
{
	int i, port_cpu;

	if(strstr(dev_name, "nas")) {
		for(i = 0; i < EDP_MAX_NETIF_HW_TABLE_SIZE; i++)
		{
			if(rt_edp_db.systemGlobal.interfaceInfo[i].valid==RT_EDP_IF_INVALID_ENTRY)
				continue;
			if((rt_edp_db.systemGlobal.interfaceInfo[i].storedInfo.is_wan) && (strcmp(dev_name, rt_edp_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.devName)==0x0))
			{
				*port_idx = rt_edp_db.systemGlobal.interfaceInfo[i].storedInfo.wan_intf.wan_intf_conf.wan_port_idx;
				return (RT_EDP_ERR_OK);
			}
		}
	}

#if defined(CONFIG_RG_RTL9607C_SERIES)
	port_cpu = RT_EDP_MAC_PORT_MASTERCPU_CORE0;
#else
	port_cpu = RT_EDP_MAC_PORT_CPU;
#endif

	for(i = 0; i < RT_EDP_MAC_PORT_MAX; i++) {
		if(strstr(dev_name /*ex: eth0.2.9*/, rt_edp_db.dev_port_map[i].dev_name /*ex: eth0.2*/)) {
			if(rt_edp_db.dev_port_map[i].port_idx > RT_EDP_MAC_PORT_PON)
				*port_idx = port_cpu;
			else
				*port_idx = rt_edp_db.dev_port_map[i].port_idx;
			return (RT_EDP_ERR_OK);
		}
	}

	return (RT_EDP_ERR_OK);
}

int _rt_edp_acl_pattern_get(struct sk_buff *skb, rt_edp_aclCheck_data_t *pData)
{
	struct ethhdr *eth = NULL;
	struct iphdr *ip = NULL;
	struct ipv6hdr *ipv6h = NULL;
	struct tcphdr *tcp = NULL;
	struct udphdr *udp = NULL;
	struct vlan_dev_priv *vlan = NULL;
	char dbg_buffer1[64];
	char dbg_buffer2[64];

	pData->devPort = -1;
	pData->fromDevPort = -1;
	pData->wlan_dev_idx = -1;
	pData->stream_id = -1;
	pData->intf_idx = -1;

	if(skb->dev) {
		vlan = vlan_dev_priv(skb->dev);
		memcpy(pData->dev_name, skb->dev->name, sizeof(pData->dev_name));
		ASSERT_EQ(_rt_edp_acl_pattern_dev_port_mapping(pData->dev_name, &pData->devPort),RT_EDP_ERR_OK);
	}
	else
		ACL_EDP("Warning!! fail to parsing everything due to no skb dev...");

	if(skb->from_dev) {
		memcpy(pData->fromDev_name, skb->from_dev->name, sizeof(pData->fromDev_name));
		ASSERT_EQ(_rt_edp_acl_pattern_dev_port_mapping(pData->fromDev_name, &pData->fromDevPort),RT_EDP_ERR_OK);
	}

	if(vlan) {
		//fail to parsing outer vlan priority
		if((vlan->vlan_proto) && (skb->vlan_tci & VLAN_TAG_PRESENT)) {
			pData->tagif |= RT_EDP_SVLAN_TAGIF;
			pData->stagVid = vlan->vlan_id;
			//pData->stagPri
			pData->tagif |= RT_EDP_CVLAN_TAGIF;
			pData->ctagVid = skb->vlan_tci & VLAN_VID_MASK;
			pData->ctagPri = (skb->vlan_tci & VLAN_PRIO_MASK)>>VLAN_PRIO_SHIFT;
		}
		else if(vlan->vlan_proto) {
			pData->tagif |= RT_EDP_CVLAN_TAGIF;
			pData->ctagVid = vlan->vlan_id;
			//pData->ctagPri
		}
	}

	eth = eth_hdr(skb);
	if(eth) {
		memcpy(pData->smac, eth->h_source, ETHER_ADDR_LEN);
		memcpy(pData->dmac, eth->h_dest, ETHER_ADDR_LEN);
	}

	pData->ethertype = skb->protocol;
	if(pData->ethertype == ETH_P_IP) {
		ip = ip_hdr(skb);
		if(ip) {
			pData->tagif |= RT_EDP_IPV4_TAGIF;
			pData->ipv4Sip = ip->saddr;
			pData->ipv4Dip = ip->daddr;
			if(ip->protocol == IPPROTO_TCP)
				tcp = (struct tcphdr *)((u32)ip+(ip->ihl<<2));
			else if(ip->protocol == IPPROTO_UDP)
				udp = (struct udphdr *)((u32)ip+(ip->ihl<<2));
			else if(ip->protocol == IPPROTO_IGMP)
				pData->tagif |= RT_EDP_IGMP_TAGIF;
			else if(ip->protocol == IPPROTO_ICMP)
				pData->tagif |= RT_EDP_ICMP_TAGIF;
			pData->ipProtocol = ip->protocol;
			pData->ipv4_tos = ip->tos;
		}
	}else if(pData->ethertype == ETH_P_IPV6) {
		ipv6h = ipv6_hdr(skb);
		if(ipv6h) {
			pData->tagif |= RT_EDP_IPV6_TAGIF;
			memcpy(pData->ipv6Sip, &ipv6h->saddr, sizeof(pData->ipv6Sip));
			memcpy(pData->ipv6Dip, &ipv6h->daddr, sizeof(pData->ipv6Dip));
			if(ipv6h->nexthdr == IPPROTO_TCP)
				tcp = (struct tcphdr *)((u32)ipv6h+sizeof(struct ipv6hdr));
			else if(ipv6h->nexthdr == IPPROTO_UDP)
				udp = (struct udphdr *)((u32)ipv6h+sizeof(struct ipv6hdr));
			else if(ipv6h->nexthdr == 0x3a)
				pData->tagif |= RT_EDP_ICMPV6_TAGIF;
			pData->ipProtocol = ipv6h->nexthdr;
			pData->ipv6_tc = ((ipv6h->priority << 4) | ((ipv6h->flow_lbl[0] & 0xF0) >> 4));
			pData->ipv6FlowLebal = (((ipv6h->flow_lbl[0] & 0x0F) << 16 ) | (ipv6h->flow_lbl[1] << 8) | ipv6h->flow_lbl[2]);
		}
	}

	if(tcp) {
		pData->tagif |= RT_EDP_TCP_TAGIF;
		pData->sport = ntohs(tcp->source);
		pData->dport = ntohs(tcp->dest);
	}
	else if(udp) {
		pData->tagif |= RT_EDP_UDP_TAGIF;
		pData->sport = ntohs(udp->source);
		pData->dport = ntohs(udp->dest);
	}

	//pData->wlan_dev_idx
	//pData->stream_id
	//pData->intf_idx

	bzero(dbg_buffer1, sizeof(dbg_buffer1));
	bzero(dbg_buffer2, sizeof(dbg_buffer2));
	if(pData->tagif&RT_EDP_SVLAN_TAGIF)
		snprintf(dbg_buffer1, sizeof(dbg_buffer1), "stagIf=%d(VID=%d, PRI=%d, DEI=%d)", pData->tagif&RT_EDP_SVLAN_TAGIF?1:0, pData->stagVid, pData->stagPri, pData->stagDei);
	if(pData->tagif&RT_EDP_CVLAN_TAGIF)
		snprintf(dbg_buffer2, sizeof(dbg_buffer2), "ctagIf=%d(VID=%d, PRI=%d, CFI=%d)", pData->tagif&RT_EDP_CVLAN_TAGIF?1:0, pData->ctagVid, pData->ctagPri, pData->ctagCfi);
	ACL_EDP("Intf/Port=%s/%d DA=%02x:%02x:%02x:%02x:%02x:%02x SA=%02x:%02x:%02x:%02x:%02x:%02x ETH=%04x %s %s WLANidx=%d StrID=%d IntfIdx=%d",
		pData->dev_name, pData->devPort,
		pData->dmac[0], pData->dmac[1], pData->dmac[2], pData->dmac[3], pData->dmac[4], pData->dmac[5],
		pData->smac[0], pData->smac[1], pData->smac[2], pData->smac[3], pData->smac[4], pData->smac[5],
		pData->ethertype,
		(pData->tagif&RT_EDP_SVLAN_TAGIF)?dbg_buffer1:"stagIf=0",
		(pData->tagif&RT_EDP_CVLAN_TAGIF)?dbg_buffer2:"ctagIf=0",
		pData->wlan_dev_idx,
		pData->stream_id,
		pData->intf_idx
		);

	bzero(dbg_buffer1, sizeof(dbg_buffer1));
	bzero(dbg_buffer2, sizeof(dbg_buffer2));
	if(pData->tagif&RT_EDP_IPV4_TAGIF) {
		snprintf(dbg_buffer1, sizeof(dbg_buffer1), "%pI4", &pData->ipv4Sip);
		snprintf(dbg_buffer2, sizeof(dbg_buffer2), "%pI4", &pData->ipv4Dip);
	}
	else if(pData->tagif&RT_EDP_IPV6_TAGIF) {
		snprintf(dbg_buffer1, sizeof(dbg_buffer1), "%pI6", &pData->ipv6Sip);
		snprintf(dbg_buffer2, sizeof(dbg_buffer2), "%pI6", &pData->ipv6Dip);
	}
	if((pData->tagif&RT_EDP_IPV4_TAGIF) || (pData->tagif&RT_EDP_IPV6_TAGIF)) {
		ACL_EDP("Src=%s(%d) Dst=%s(%d) L4_PROTO=0x%x TOS/TC,FLOWLEBEL=0x%x,0x%x",
			dbg_buffer1,
			pData->sport,
			dbg_buffer2,
			pData->dport,
			pData->ipProtocol,
			(pData->tagif&RT_EDP_IPV4_TAGIF)?pData->ipv4_tos:pData->ipv6_tc,
			(pData->tagif&RT_EDP_IPV4_TAGIF)?0:pData->ipv6FlowLebal
			);
	}

	return (RT_EDP_ERR_OK);
}

int _rt_edp_ingressACLPatternCheck(struct sk_buff *skb, rt_edp_aclCheck_data_t *pData)
{

	int i,acl_sw_idx;
	uint8 port;
	rt_edp_aclFilterEntry_t* pChkRule;
	int ipv6_upper_range_check,ipv6_lowwer_range_check;
	int port_check;
	uint8 tos;
	int handleValidRuleCounter=0;
	int hitRuleCounter=0;
	uint32 possitive_check=0;
	rt_edp_skb_t *pSkbData = &skb->edpSkbData;
	rt_edp_aclCheck_data_t *pPktHdr = pData;

	/*Skip ----------------- ingressCvidRuleIdxArray ----------------- Skip*/

	/*Skip ----------------- aclIgrRuleChecked ----------------- Skip*/

	/*Skip ----------------- initial acl priority ----------------- Skip*/
	pSkbData->aclDecision.aclEgrHaveToCheckRuleIdx[hitRuleCounter]=-1; //empty from first rule until the end

	if(rt_edp_db.systemGlobal.acl_SW_table_entry_size<=0){//no rule need to verify
		ACL_EDP("no RG_ACL need to verify");
		return RT_ERR_RG_OK;
	}

	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){

		if(handleValidRuleCounter >= rt_edp_db.systemGlobal.acl_SW_table_entry_size){//no valid rule need to check, skip rest for loop
			//ACL_EDP("no more valid ACLRule need to verify");
			break;
		}

		//normal check all aclSWRule ingress part.
		if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].valid!=RT_EDP_ENABLED)//skip empty rule
			continue;
		pChkRule = &(rt_edp_db.systemGlobal.acl_SW_table_entry[i]);
		acl_sw_idx = i;

		//ACL_EDP("Check ingress part of ACLRule[%d]:",acl_sw_idx);
		/*Skip ----------------- especial check aclSWRule with ingress cvid action. ----------------- Skip*/

		handleValidRuleCounter++; //a valid rule is going to check


		if(RT_EDP_IS_WAN_PORT(pPktHdr->devPort)) //downstream
		{
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP)
			{
				ACL_EDP("RG_ACL[%d] FWD_TYPE UNHIT DOWNSTREAM ",acl_sw_idx);
				continue;
			}
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN)
			{
				ACL_EDP("RG_ACL[%d] FWD_TYPE UNHIT DOWNSTREAM ",acl_sw_idx);
				continue;
			}
		}
		else //upstream
		{
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP)
			{
				ACL_EDP("RG_ACL[%d] FWD_TYPE UNHIT UPSTREAM ",acl_sw_idx);
				continue;
			}
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN)
			{
				ACL_EDP("RG_ACL[%d] FWD_TYPE UNHIT UPSTREAM ",acl_sw_idx);
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
					if(port==pPktHdr->devPort){
						port_check = SUCCESS;
					}
				}
			}
			if(port_check!=SUCCESS){
				ACL_EDP("RG_ACL[%d] INGRESS_PORT UNHIT, devPort=%d, portmask=0x%x ",acl_sw_idx, pPktHdr->devPort, pChkRule->acl_filter.ingress_port_mask.portmask);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_PORT_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_PORT UNHIT");
		}



		if(pChkRule->acl_filter.filter_fields&INGRESS_INTF_BIT){
			WARNING("Pattern ingress_intf_idx still under development...");
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.ingress_intf_idx!=pPktHdr->intf_idx){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_INTF_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_INTF_BIT UNHIT");
		}


		if(pChkRule->acl_filter.filter_fields&INGRESS_STREAM_ID_BIT){
			WARNING("Pattern ingress_stream_id still under development...");
			PATTERN_CHECK_INIT(possitive_check);
			if((pChkRule->acl_filter.ingress_stream_id&pChkRule->acl_filter.ingress_stream_id_mask)!=(pPktHdr->stream_id&pChkRule->acl_filter.ingress_stream_id_mask)){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_STREAM_ID_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_STREAM_ID_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV4_TAGIF_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.ingress_ipv4_tagif){//must have ip header
				if(pPktHdr->tagif&RT_EDP_IPV4_TAGIF){
					//hit do nothing
				}else{
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}else{//must not have IP header
				if(pPktHdr->tagif&RT_EDP_IPV4_TAGIF){
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
				if(pPktHdr->tagif&RT_EDP_IPV6_TAGIF){
					//hit do nothing
				}else{
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}else{//must not have IP header
				if(pPktHdr->tagif&RT_EDP_IPV6_TAGIF){
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
				if((pPktHdr->tagif&RT_EDP_SVLAN_TAGIF)==0x0){
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}else{//must not have stag
				if(pPktHdr->tagif&RT_EDP_SVLAN_TAGIF){
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_STAGIF_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_STAGIF_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_CTAGIF_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.ingress_ctagIf){//must have ctag
				if((pPktHdr->tagif&RT_EDP_CVLAN_TAGIF)==0x0){
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}else{//must not have ctag
				if(pPktHdr->tagif&RT_EDP_CVLAN_TAGIF){
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_CTAGIF_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_CTAGIF_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_ETHERTYPE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if((pChkRule->acl_filter.ingress_ethertype & pChkRule->acl_filter.ingress_ethertype_mask)!=(pPktHdr->ethertype & pChkRule->acl_filter.ingress_ethertype_mask)){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_ETHERTYPE_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_ETHERTYPE_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_STAG_PRI_BIT){
			WARNING("Pattern ingress_stag_pri still under development...");
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.ingress_stag_pri!=pPktHdr->stagPri){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_STAG_PRI_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_STAG_PRI_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_STAG_VID_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.ingress_stag_vid!=pPktHdr->stagVid){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_STAG_VID_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_STAG_VID_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_STAG_DEI_BIT){
			//EDP not support
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.ingress_stag_dei!=pPktHdr->stagDei){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_STAG_DEI_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_STAG_DEI_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_CTAG_PRI_BIT){
			WARNING("Pattern ingress_ctag_pri still under development...");
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.ingress_ctag_pri!=pPktHdr->ctagPri){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_CTAG_PRI_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_CTAG_PRI_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_CTAG_VID_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.ingress_ctag_vid!=pPktHdr->ctagVid){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_CTAG_VID_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_CTAG_VID_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_CTAG_CFI_BIT){
			//EDP not support
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.ingress_ctag_cfi!=pPktHdr->ctagCfi){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_CTAG_CFI_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_CTAG_CFI_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_SMAC_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if((pChkRule->acl_filter.ingress_smac.octet[0] & pChkRule->acl_filter.ingress_smac_mask.octet[0])!=(pPktHdr->smac[0]& pChkRule->acl_filter.ingress_smac_mask.octet[0]) ||
				(pChkRule->acl_filter.ingress_smac.octet[1] & pChkRule->acl_filter.ingress_smac_mask.octet[1])!=(pPktHdr->smac[1]& pChkRule->acl_filter.ingress_smac_mask.octet[1])||
				(pChkRule->acl_filter.ingress_smac.octet[2] & pChkRule->acl_filter.ingress_smac_mask.octet[2])!=(pPktHdr->smac[2]& pChkRule->acl_filter.ingress_smac_mask.octet[2])||
				(pChkRule->acl_filter.ingress_smac.octet[3] & pChkRule->acl_filter.ingress_smac_mask.octet[3])!=(pPktHdr->smac[3]& pChkRule->acl_filter.ingress_smac_mask.octet[3])||
				(pChkRule->acl_filter.ingress_smac.octet[4] & pChkRule->acl_filter.ingress_smac_mask.octet[4])!=(pPktHdr->smac[4]& pChkRule->acl_filter.ingress_smac_mask.octet[4])||
				(pChkRule->acl_filter.ingress_smac.octet[5] & pChkRule->acl_filter.ingress_smac_mask.octet[5])!=(pPktHdr->smac[5]& pChkRule->acl_filter.ingress_smac_mask.octet[5])
				){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_SMAC_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_SMAC_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_DMAC_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if((pChkRule->acl_filter.ingress_dmac.octet[0] & pChkRule->acl_filter.ingress_dmac_mask.octet[0])!=(pPktHdr->dmac[0] & pChkRule->acl_filter.ingress_dmac_mask.octet[0]) ||
				(pChkRule->acl_filter.ingress_dmac.octet[1] & pChkRule->acl_filter.ingress_dmac_mask.octet[1])!=(pPktHdr->dmac[1] & pChkRule->acl_filter.ingress_dmac_mask.octet[1])||
				(pChkRule->acl_filter.ingress_dmac.octet[2] & pChkRule->acl_filter.ingress_dmac_mask.octet[2])!=(pPktHdr->dmac[2] & pChkRule->acl_filter.ingress_dmac_mask.octet[2])||
				(pChkRule->acl_filter.ingress_dmac.octet[3] & pChkRule->acl_filter.ingress_dmac_mask.octet[3])!=(pPktHdr->dmac[3] & pChkRule->acl_filter.ingress_dmac_mask.octet[3])||
				(pChkRule->acl_filter.ingress_dmac.octet[4] & pChkRule->acl_filter.ingress_dmac_mask.octet[4])!=(pPktHdr->dmac[4] & pChkRule->acl_filter.ingress_dmac_mask.octet[4])||
				(pChkRule->acl_filter.ingress_dmac.octet[5] & pChkRule->acl_filter.ingress_dmac_mask.octet[5])!=(pPktHdr->dmac[5] & pChkRule->acl_filter.ingress_dmac_mask.octet[5])
				){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_DMAC_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_DMAC_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_DSCP_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pPktHdr->ipv4_tos==0){//pkt without DSCP
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}else{
				tos = pPktHdr->ipv4_tos;
				if(pChkRule->acl_filter.ingress_dscp!=(tos>>2) || (pPktHdr->tagif&RT_EDP_IPV4_TAGIF)==0x0 /*must be ipv4*/){
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_DSCP_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_DSCP_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_TOS_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pPktHdr->ipv4_tos==0){//pkt without DSCP
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}else{
				tos = pPktHdr->ipv4_tos;
				if(pChkRule->acl_filter.ingress_tos!=tos || (pPktHdr->tagif&RT_EDP_IPV4_TAGIF)==0x0 /*must be ipv4*/){
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_TOS_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_TOS_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_DSCP_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pPktHdr->ipv6_tc==0){//pkt without DSCP
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}else{
				tos = pPktHdr->ipv6_tc;
				if(pChkRule->acl_filter.ingress_ipv6_dscp!=(tos>>2) || (pPktHdr->tagif&RT_EDP_IPV6_TAGIF)==0x0 /*must be ipv6*/){
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_IPV6_DSCP_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_IPV6_DSCP_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_TC_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pPktHdr->ipv6_tc==0){//pkt without DSCP
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}else{
				tos = pPktHdr->ipv6_tc;
				if(pChkRule->acl_filter.ingress_ipv6_tc!=tos || (pPktHdr->tagif&RT_EDP_IPV6_TAGIF)==0x0 /*must be ipv6*/){
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_IPV6_TC_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_IPV6_TC_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_TCP_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if( !(pPktHdr->tagif&RT_EDP_TCP_TAGIF)){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_L4_TCP_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_L4_TCP_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_UDP_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if( !(pPktHdr->tagif&RT_EDP_UDP_TAGIF)){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_L4_UDP_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_L4_UDP_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_ICMP_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(!(pPktHdr->tagif&RT_EDP_ICMP_TAGIF)){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_L4_ICMP_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_L4_ICMP_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_ICMPV6_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(!(pPktHdr->tagif&RT_EDP_ICMPV6_TAGIF)){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_L4_ICMPV6_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_L4_ICMPV6_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_POROTCAL_VALUE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if((pPktHdr->ipProtocol)!=(pChkRule->acl_filter.ingress_l4_protocal)){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_L4_POROTCAL_VALUE_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_L4_POROTCAL_VALUE_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_SIP_RANGE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			ipv6_upper_range_check = -1;
			ipv6_lowwer_range_check = -1;
			if(pPktHdr->tagif&RT_EDP_IPV6_TAGIF){
				ipv6_upper_range_check = memcmp(&(pChkRule->acl_filter.ingress_src_ipv6_addr_end[0]),pPktHdr->ipv6Sip,16);
				 ipv6_lowwer_range_check = memcmp(pPktHdr->ipv6Sip,&(pChkRule->acl_filter.ingress_src_ipv6_addr_start[0]),16);
			}
			if(!(ipv6_upper_range_check>=0 && ipv6_lowwer_range_check>=0 )){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_IPV6_SIP_RANGE_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_IPV6_SIP_RANGE_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_SIP_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pPktHdr->tagif&RT_EDP_IPV6_TAGIF){
				if((pPktHdr->ipv6Sip[0]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[0])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[0]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[0]) ||
					(pPktHdr->ipv6Sip[1]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[1])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[1]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[1]) ||
					(pPktHdr->ipv6Sip[2]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[2])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[2]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[2]) ||
					(pPktHdr->ipv6Sip[3]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[3])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[3]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[3]) ||
					(pPktHdr->ipv6Sip[4]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[4])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[4]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[4]) ||
					(pPktHdr->ipv6Sip[5]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[5])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[5]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[5]) ||
					(pPktHdr->ipv6Sip[6]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[6])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[6]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[6]) ||
					(pPktHdr->ipv6Sip[7]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[7])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[7]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[7]) ||
					(pPktHdr->ipv6Sip[8]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[8])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[8]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[8]) ||
					(pPktHdr->ipv6Sip[9]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[9])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[9]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[9]) ||
					(pPktHdr->ipv6Sip[10]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[10])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[10]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[10]) ||
					(pPktHdr->ipv6Sip[11]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[11])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[11]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[11]) ||
					(pPktHdr->ipv6Sip[12]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[12])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[12]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[12]) ||
					(pPktHdr->ipv6Sip[13]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[13])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[13]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[13]) ||
					(pPktHdr->ipv6Sip[14]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[14])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[14]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[14]) ||
					(pPktHdr->ipv6Sip[15]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[15])!=(pChkRule->acl_filter.ingress_src_ipv6_addr[15]&pChkRule->acl_filter.ingress_src_ipv6_addr_mask[15])
					){
						possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}else{
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_IPV6_SIP_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_IPV6_SIP_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_DIP_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pPktHdr->ipv6Dip != NULL){
				if((pPktHdr->ipv6Dip[0]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[0])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[0]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[0]) ||
					(pPktHdr->ipv6Dip[1]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[1])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[1]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[1]) ||
					(pPktHdr->ipv6Dip[2]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[2])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[2]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[2]) ||
					(pPktHdr->ipv6Dip[3]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[3])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[3]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[3]) ||
					(pPktHdr->ipv6Dip[4]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[4])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[4]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[4]) ||
					(pPktHdr->ipv6Dip[5]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[5])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[5]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[5]) ||
					(pPktHdr->ipv6Dip[6]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[6])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[6]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[6]) ||
					(pPktHdr->ipv6Dip[7]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[7])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[7]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[7]) ||
					(pPktHdr->ipv6Dip[8]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[8])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[8]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[8]) ||
					(pPktHdr->ipv6Dip[9]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[9])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[9]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[9]) ||
					(pPktHdr->ipv6Dip[10]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[10])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[10]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[10]) ||
					(pPktHdr->ipv6Dip[11]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[11])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[11]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[11]) ||
					(pPktHdr->ipv6Dip[12]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[12])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[12]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[12]) ||
					(pPktHdr->ipv6Dip[13]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[13])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[13]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[13]) ||
					(pPktHdr->ipv6Dip[14]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[14])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[14]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[14]) ||
					(pPktHdr->ipv6Dip[15]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[15])!=(pChkRule->acl_filter.ingress_dest_ipv6_addr[15]&pChkRule->acl_filter.ingress_dest_ipv6_addr_mask[15])
					){
						possitive_check = ACL_PATTERN_UNHIT; //continue;
				}

			}else{
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_IPV6_DIP_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_IPV6_DIP_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_DIP_RANGE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			ipv6_upper_range_check = -1;
			ipv6_lowwer_range_check = -1;
			if(pPktHdr->ipv6Dip!=NULL){
				ipv6_upper_range_check = memcmp(&(pChkRule->acl_filter.ingress_dest_ipv6_addr_end[0]),pPktHdr->ipv6Dip,16);
				ipv6_lowwer_range_check = memcmp(pPktHdr->ipv6Dip,&(pChkRule->acl_filter.ingress_dest_ipv6_addr_start[0]),16);
			}
			if(!(ipv6_upper_range_check>=0 && ipv6_lowwer_range_check>=0 )){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_IPV6_DIP_RANGE_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_IPV6_DIP_RANGE_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV4_SIP_RANGE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if( !((pChkRule->acl_filter.ingress_src_ipv4_addr_end >= pPktHdr->ipv4Sip ) &&
				(pChkRule->acl_filter.ingress_src_ipv4_addr_start<= pPktHdr->ipv4Sip ))){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_IPV4_SIP_RANGE_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_IPV4_SIP_RANGE_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV4_DIP_RANGE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if( !((pChkRule->acl_filter.ingress_dest_ipv4_addr_end >= pPktHdr->ipv4Dip ) &&
				(pChkRule->acl_filter.ingress_dest_ipv4_addr_start<= pPktHdr->ipv4Dip ))){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_IPV4_DIP_RANGE_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_IPV4_DIP_RANGE_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_SPORT_RANGE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if( !((pChkRule->acl_filter.ingress_src_l4_port_end >= pPktHdr->sport ) &&
				(pChkRule->acl_filter.ingress_src_l4_port_start<= pPktHdr->sport ))){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_L4_SPORT_RANGE_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_L4_SPORT_RANGE_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_L4_DPORT_RANGE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if( !((pChkRule->acl_filter.ingress_dest_l4_port_end >= pPktHdr->dport ) &&
				(pChkRule->acl_filter.ingress_dest_l4_port_start<= pPktHdr->dport ))){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_L4_DPORT_RANGE_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_L4_DPORT_RANGE_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_IPV6_FLOWLABEL_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.ingress_ipv6_flow_label!=pPktHdr->ipv6FlowLebal){
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_IPV6_FLOWLABEL_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_IPV6_FLOWLABEL_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&INGRESS_WLANDEV_BIT){
			WARNING("Pattern ingress_wlanDevMask still under development...");
			PATTERN_CHECK_INIT(possitive_check);
			if((pPktHdr->wlan_dev_idx >= 0) && (pChkRule->acl_filter.ingress_wlanDevMask&(1<<pPktHdr->wlan_dev_idx))){
				//hit!
			}else{
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(acl_sw_idx,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_WLANDEV_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_WLANDEV_BIT UNHIT");
		}



		//record ACL hit rule
		ACL_EDP("RG_ACL[%d] Ingress Part Hit!",acl_sw_idx);
		pSkbData->aclDecision.aclIgrHitMask[(acl_sw_idx>>5)]|=(1<<(acl_sw_idx&0x1f));
		pSkbData->aclDecision.aclEgrHaveToCheckRuleIdx[hitRuleCounter]=acl_sw_idx; //record current rule
		pSkbData->aclDecision.aclEgrHaveToCheckRuleIdx[hitRuleCounter+1]=-1;//empty next rule until the end
		hitRuleCounter++;
		/*Skip ----------------- ingressCvidRuleIdxArray ----------------- Skip*/

	}

	return RT_EDP_ERR_OK;
}

int _rt_edp_ingressACLAction(struct sk_buff *skb)
{
	return RT_EDP_ERR_OK;
}


int _rt_edp_ingressACLPatternCheckAndAction(struct sk_buff *skb)
{
	int i;
	rtk_rg_aclFilterAndQos_t *acl_filter;
	rt_edp_aclCheck_data_t pktData;
	rt_edp_aclCheck_data_t *pData = &pktData;

	bzero(&skb->edpSkbData.aclDecision, sizeof(skb->edpSkbData.aclDecision));

	skb->edpSkbData.aclDecision.aclEgrHaveToCheckRuleIdx[0]=-1; //empty from first rule until the end

	if(rt_edp_db.systemGlobal.acl_SW_table_entry_size<=0){//no rule need to verify
		ACL_EDP("no RG_ACL need to verify");
		return RT_ERR_RG_OK;
	}

	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){
		if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].valid!=RT_EDP_ENABLED)//skip empty rule
			continue;
		acl_filter = &(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter);
		if(EDP_ACL_IS_QOS_REMARKING(acl_filter)) {
			i = -1;
			break;
		}
	}
	if(i != -1) {
		ACL_EDP("no QoS RG_ACL need to verify");
		return RT_EDP_ERR_OK;
	}

	bzero(&pktData, sizeof(pktData));

	ASSERT_EQ(_rt_edp_acl_pattern_get(skb, pData),RT_EDP_ERR_OK);

	ASSERT_EQ(_rt_edp_ingressACLPatternCheck(skb, pData),RT_EDP_ERR_OK);

	ASSERT_EQ(_rt_edp_ingressACLAction(skb),RT_EDP_ERR_OK);

	return RT_EDP_ERR_OK;
}

int _rt_edp_egressACLPatternCheck(struct sk_buff *skb, rt_edp_aclCheck_data_t *pData)
{
	uint32 i,index;
	rt_edp_aclFilterEntry_t* pChkRule;
	uint32 possitive_check;
	uint8 round_check = ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET;	//check fwd_type=0 first, then check fwd_type!=0
	uint32 flowDecision = RT_EDP_CF_LAN_TO_LAN;
	uint32 is_ip4mc_check=0,is_ip6mc_check=0;

	rt_edp_skb_t *pSkbData = &skb->edpSkbData;
	rt_edp_aclCheck_data_t *pPktHdr = pData;

	//reset the egressHitMask & final actions, because in broacast dataPath each Interfcaes will call this API individually


	/*Skip ----------------- reset aclEgrPattern1HitMask ----------------- Skip*/

	_rt_edp_aclDecisionClear(skb);

	/*Skip ----------------- RG_ACL_EGRESS_BYPASS_PORT_ESPECIAL_TO_PS_CHECK ----------------- Skip*/
	//flow decision
	if(RT_EDP_IS_WAN_PORT(pPktHdr->fromDevPort)) //downstream
	{
		flowDecision = RT_EDP_CF_DOWNSTREAM;
		ACL_EDP("flowDecision: CF_DOWNSTREAM (ingressPort=%d,egressPort=%d)", pPktHdr->fromDevPort, pPktHdr->devPort);
	}
	else //maybe upstream,
	{
		if(RT_EDP_IS_WAN_PORT(pPktHdr->devPort))//upstream
		{
			flowDecision = RT_EDP_CF_UPSTREAM;
			ACL_EDP("flowDecision: CF_UPSTREAM (ingressPort=%d,egressPort=%d)", pPktHdr->fromDevPort, pPktHdr->devPort);
		}
		else //lan to lan
		{
			flowDecision= RT_EDP_CF_LAN_TO_LAN;
			ACL_EDP("flowDecision: CF_LAN_TO_LAN (ingressPort=%d,egressPort=%d)", pPktHdr->fromDevPort, pPktHdr->devPort);
		}
	}

	/*check CF pattern0*/
	for(index=0;index<MAX_ACL_SW_ENTRY_SIZE;index++){
		i = pSkbData->aclDecision.aclEgrHaveToCheckRuleIdx[index];
		if(i==-1) { //no rest rule need to check
			if(round_check == ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET) {
				/*CF (fwdtype=1~4) pattern should reference for the result after ACL (fwdtype=0) modify */
				/*Skip ----------------- _rtk_rg_egressACLAction_beforeCFcheck ----------------- Skip*/
				/*check for fwd_type=0 finish, then check for fwd_type!=0 */
				round_check++;
				index = -1;	//next round will index++ so it will check rule idx 0
				continue;
			}
			else
				break;
		}


		pChkRule = &(rt_edp_db.systemGlobal.acl_SW_table_entry[i]);

		/*CF (fwdtype=1~4) pattern should reference for the result after ACL (fwdtype=0) modify, so always check fwdtype=0 first */
		if(round_check == ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET) {	//this round only check fwd_type=0
			if(pChkRule->acl_filter.fwding_type_and_direction != ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
				continue;
		}
		else {		//this round only check fwd_type!=0
			if(pChkRule->acl_filter.fwding_type_and_direction == ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
				continue;
		}

		//ACL_EDP("Check egress part of RG_ACL[%d]:",i);


		/*Skip ----------------- RG_ACL_EGRESS_BYPASS_PORT_ESPECIAL_TO_PS_CHECK ----------------- Skip*/
		//direction check
		if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP ||
			pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN ||
			pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP ||
			pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT)
		{
			if(flowDecision != RT_EDP_CF_UPSTREAM){
				ACL_EDP("RG_ACL[%d] fwding_type_and_direction UNHIT",i);
				continue;
			}
		}
		else if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP ||
			pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN ||
			pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP ||
			pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_PERMIT)
		{
			if(flowDecision != RT_EDP_CF_DOWNSTREAM){
				ACL_EDP("RG_ACL[%d] fwding_type_and_direction UNHIT",i);
				continue;
			}

		}
		else
		{
			//lan-to-lan don't care, it can support all kind of pattern in apolloPro.
		}


		if(pChkRule->acl_filter.filter_fields&EGRESS_INTF_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.egress_intf_idx!=pPktHdr->intf_idx) //from PS, it's special pure software data path, the interface is determind as elder version which from pPktHdr->netifIdx
			{
				//ACL_EDP("pChkRule->acl_filter.egress_intf_idx=%d, netifIdx=%d, pPktHdr->netifIdx=%d",pChkRule->acl_filter.egress_intf_idx,netifIdx,pPktHdr->netifIdx);
				//ACL_EDP("RG_ACL[%d] EGRESS_INTF_BIT UNHIT ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}
			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_INTF_BIT)?1:0, possitive_check, "RG_ACL[%d] EGRESS_INTF_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&EGRESS_IPV4_SIP_RANGE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if( !((pChkRule->acl_filter.egress_src_ipv4_addr_end >= pPktHdr->ipv4Sip ) &&
				(pChkRule->acl_filter.egress_src_ipv4_addr_start<= pPktHdr->ipv4Sip ))){
				//DEBUG(" sipModify=0x%x",sipModify);
				//ACL_EDP("EGRESS_src_ipv4_addr_start=0x%x egress_src_ipv4_addr_end=0x%x",pChkRule->acl_filter.egress_src_ipv4_addr_start,pChkRule->acl_filter.egress_src_ipv4_addr_end);
				//ACL_EDP("RG_ACL[%d] EGRESS_IPV4_SIP_RANGE UNHIT ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_IPV4_SIP_RANGE_BIT)?1:0, possitive_check, "RG_ACL[%d] EGRESS_IPV4_SIP_RANGE_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&EGRESS_IPV4_DIP_RANGE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if( !((pChkRule->acl_filter.egress_dest_ipv4_addr_end >= pPktHdr->ipv4Dip ) &&
				(pChkRule->acl_filter.egress_dest_ipv4_addr_start<= pPktHdr->ipv4Dip))){
				//DEBUG(" dipModify=0x%x",dipModify);
				//ACL_EDP("EGRESS_dest_ipv4_addr_start=0x%x egress_dest_ipv4_addr_end=0x%x",pChkRule->acl_filter.egress_dest_ipv4_addr_start,pChkRule->acl_filter.egress_dest_ipv4_addr_end);
				//ACL_EDP("RG_ACL[%d] EGRESS_IPV4_DIP_RANGE UNHIT ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_IPV4_DIP_RANGE_BIT)?1:0, possitive_check, "RG_ACL[%d] EGRESS_IPV4_DIP_RANGE_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&EGRESS_L4_SPORT_RANGE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if( !((pChkRule->acl_filter.egress_src_l4_port_end >= pPktHdr->sport ) &&
				(pChkRule->acl_filter.egress_src_l4_port_start<= pPktHdr->sport ))){
				//ACL_EDP("RG_ACL[%d] EGRESS_L4_SPORT_RANGE UNHIT ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_L4_SPORT_RANGE_BIT)?1:0, possitive_check, "RG_ACL[%d] EGRESS_L4_SPORT_RANGE_BIT UNHIT");
		}
		if(pChkRule->acl_filter.filter_fields&EGRESS_L4_DPORT_RANGE_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if( !((pChkRule->acl_filter.egress_dest_l4_port_end >= pPktHdr->dport ) &&
				(pChkRule->acl_filter.egress_dest_l4_port_start<= pPktHdr->dport))){
				//ACL_EDP("RG_ACL[%d] EGRESS_L4_DPORT_RANGE UNHIT ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_L4_DPORT_RANGE_BIT)?1:0, possitive_check, "RG_ACL[%d] EGRESS_L4_DPORT_RANGE_BIT UNHIT");
		}

#if 0	//under development
		/*Skip ----------------- RG_ACL_EGRESS_BYPASS_PORT_ESPECIAL_TO_PS_CHECK ----------------- Skip*/
		if(pChkRule->acl_filter.filter_fields&EGRESS_CTAG_VID_BIT){//L34uptream only
			PATTERN_CHECK_INIT(possitive_check);

			//this pattern must egress with Ctag
			if(pPktHdr->egressVlanTagif==0){
				//ACL_EDP("RG_ACL[%d] EGRESS_CTAG_VID_BIT UNHIT  packet without Ctag ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			if((pChkRule->acl_filter.egress_ctag_vid & pChkRule->acl_filter.egress_ctag_vid_mask)!= (pPktHdr->internalVlanID & pChkRule->acl_filter.egress_ctag_vid_mask)){
				//ACL_EDP("RG_ACL[%d] EGRESS_CTAG_VID_BIT UNHIT  pChkRule->egress_ctag_vid=%d pPktHdr->egressVlanID=%d, pChkRule->acl_filter.egress_ctag_vid_mask=0x%x",i,pChkRule->acl_filter.egress_ctag_vid,pPktHdr->internalVlanID,pChkRule->acl_filter.egress_ctag_vid_mask);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_CTAG_VID_BIT)?1:0, possitive_check, "RG_ACL[%d] EGRESS_CTAG_VID_BIT UNHIT");
		}


		/*Skip ----------------- RG_ACL_EGRESS_BYPASS_PORT_ESPECIAL_TO_PS_CHECK ----------------- Skip*/
		if(pChkRule->acl_filter.filter_fields&EGRESS_CTAG_PRI_BIT){//L34uptream only
			PATTERN_CHECK_INIT(possitive_check);
			//this pattern must egress with Ctag
			if(pPktHdr->egressVlanTagif==0){
				//ACL_EDP("RG_ACL[%d] EGRESS_CTAG_PRI_BIT UNHIT  packet without Ctag ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			//[FIXME] broadcast datapath may not include ACL and Qos 1p-remarking decision in pPktHdr->egressPriority
			if(pChkRule->acl_filter.egress_ctag_pri!=pPktHdr->egressPriority)
			{
				//ACL_EDP("RG_ACL[%d] EGRESS_CTAG_PRI_BIT UNHIT ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_CTAG_PRI_BIT)?1:0, possitive_check, "RG_ACL[%d] EGRESS_CTAG_PRI_BIT UNHIT");
		}


		/*Skip ----------------- RG_ACL_EGRESS_BYPASS_PORT_ESPECIAL_TO_PS_CHECK ----------------- Skip*/
		if(pChkRule->acl_filter.filter_fields&INTERNAL_PRI_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if(pChkRule->acl_filter.internal_pri!=pPktHdr->internalPriority){
				//ACL_EDP("RG_ACL[%d] INTERNAL_PRI_BIT UNHIT ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&INTERNAL_PRI_BIT)?1:0, possitive_check, "RG_ACL[%d] INTERNAL_PRI_BIT UNHIT");
		}
#endif



		/*Skip ----------------- RG_ACL_EGRESS_BYPASS_PORT_ESPECIAL_TO_PS_CHECK ----------------- Skip*/
		if(pChkRule->acl_filter.filter_fields&INGRESS_EGRESS_PORTIDX_BIT){//uni pattern

			PATTERN_CHECK_INIT(possitive_check);

			//[FIXME] dowstream will following L34 DA lookup result
			if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP ||pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN)
			{
				//CF upstream(to PON), check spa
				if( (pChkRule->acl_filter.ingress_port_idx & pChkRule->acl_filter.ingress_port_idx_mask)!=(pPktHdr->fromDevPort & pChkRule->acl_filter.ingress_port_idx_mask)){
					//ACL_EDP("RG_ACL[%d] INGRESS_EGRESS_PORTIDX_BIT UNHIT ",i);
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}
			else if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP ||pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN)
			{	//CF downstream(from PON),check da
				if( (pChkRule->acl_filter.egress_port_idx & pChkRule->acl_filter.egress_port_idx_mask)!=(pPktHdr->devPort & pChkRule->acl_filter.egress_port_idx_mask)){
					//ACL_EDP("RG_ACL[%d] INGRESS_EGRESS_PORTIDX_BIT UNHIT ",i);
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}
			else if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)	//pattern egress_port_idx support upstream/downstream only in flow base platform
			{
				if( (pChkRule->acl_filter.egress_port_idx & pChkRule->acl_filter.egress_port_idx_mask)!=(pPktHdr->devPort & pChkRule->acl_filter.egress_port_idx_mask)){
					ACL_EDP("RG_ACL[%d] INGRESS_EGRESS_PORTIDX_BIT UNHIT , egress_port_idx=%d, egress_port_idx_mask=0x%x, devPort=%d",i,pChkRule->acl_filter.egress_port_idx,pChkRule->acl_filter.egress_port_idx_mask,pPktHdr->devPort);
					possitive_check = ACL_PATTERN_UNHIT; //continue;
				}
			}

			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&INGRESS_EGRESS_PORTIDX_BIT)?1:0, possitive_check, "RG_ACL[%d] INGRESS_EGRESS_PORTIDX_BIT UNHIT");
		}


		if(pChkRule->acl_filter.filter_fields&EGRESS_IP4MC_IF){
			PATTERN_CHECK_INIT(possitive_check);
			if((pPktHdr->dmac[0]==0x01&& pPktHdr->dmac[1]==0x00 && pPktHdr->dmac[2]==0x5e) && (pPktHdr->tagif&RT_EDP_IGMP_TAGIF)==0x0){
				is_ip4mc_check = 1; //IP4MC(not include IGMP)
			}

			if(pChkRule->acl_filter.egress_ip4mc_if!=is_ip4mc_check){
				//ACL_EDP("RG_ACL[%d] EGRESS_IP4MC_IF UNHIT ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_IP4MC_IF)?1:0, possitive_check, "RG_ACL[%d] EGRESS_IP4MC_IF UNHIT");
		}


		if(pChkRule->acl_filter.filter_fields&EGRESS_IP6MC_IF){
			PATTERN_CHECK_INIT(possitive_check);
			if((pPktHdr->dmac[0]==0x33 && pPktHdr->dmac[1]==0x33) && (pPktHdr->tagif&RT_EDP_IPV6_MLD_TAGIF)==0x0){
				is_ip6mc_check = 1; //IP6MC(not include MLD)
			}

			if(pChkRule->acl_filter.egress_ip6mc_if!=is_ip6mc_check){
				//ACL_EDP("RG_ACL[%d] EGRESS_IP6MC_IF UNHIT ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_IP6MC_IF)?1:0, possitive_check, "RG_ACL[%d] EGRESS_IP6MC_IF UNHIT");
		}


		if(pChkRule->acl_filter.filter_fields&EGRESS_SMAC_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if((pChkRule->acl_filter.egress_smac.octet[0] )!=(pPktHdr->smac[0]) ||
				(pChkRule->acl_filter.egress_smac.octet[1] )!=(pPktHdr->smac[1])||
				(pChkRule->acl_filter.egress_smac.octet[2] )!=(pPktHdr->smac[2])||
				(pChkRule->acl_filter.egress_smac.octet[3] )!=(pPktHdr->smac[3])||
				(pChkRule->acl_filter.egress_smac.octet[4] )!=(pPktHdr->smac[4])||
				(pChkRule->acl_filter.egress_smac.octet[5] )!=(pPktHdr->smac[5])
				){

				//ACL_EDP("RG_ACL[%d] EGRESS_SMAC_BIT UNHIT ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_SMAC_BIT)?1:0, possitive_check, "RG_ACL[%d] EGRESS_SMAC_BIT UNHIT");
		}

		if(pChkRule->acl_filter.filter_fields&EGRESS_DMAC_BIT){
			PATTERN_CHECK_INIT(possitive_check);
			if((pChkRule->acl_filter.egress_dmac.octet[0] )!=(pPktHdr->dmac[0]) ||
				(pChkRule->acl_filter.egress_dmac.octet[1] )!=(pPktHdr->dmac[1])||
				(pChkRule->acl_filter.egress_dmac.octet[2] )!=(pPktHdr->dmac[2])||
				(pChkRule->acl_filter.egress_dmac.octet[3] )!=(pPktHdr->dmac[3])||
				(pChkRule->acl_filter.egress_dmac.octet[4] )!=(pPktHdr->dmac[4])||
				(pChkRule->acl_filter.egress_dmac.octet[5] )!=(pPktHdr->dmac[5])
				){

				//ACL_EDP("RG_ACL[%d] EGRESS_DMAC_BIT UNHIT ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_DMAC_BIT)?1:0, possitive_check, "RG_ACL[%d] EGRESS_DMAC_BIT UNHIT");
		}

#if 0	//under development
		if(pChkRule->acl_filter.filter_fields&EGRESS_WLANDEV_BIT){
			PATTERN_CHECK_INIT(possitive_check);

			if(_rtk_rg_wlanDevFromDmacL2Idx_check(pChkRule->acl_filter.egress_wlanDevMask, pPktHdr) == ACL_PATTERN_UNHIT)
			{
				//ACL_EDP("RG_ACL[%d] EGRESS_WLANDEV_BIT UNHIT ",i);
				possitive_check = ACL_PATTERN_UNHIT; //continue;
			}

			PATTERN_CHECK_RESULT(i,(pChkRule->acl_filter.filter_fields_inverse&EGRESS_WLANDEV_BIT)?1:0, possitive_check, "RG_ACL[%d] EGRESS_WLANDEV_BIT UNHIT");
		}
#endif

		//record CF hit rule
		ACL_EDP("RG_ACL[%d] Egress Part Hit!",i);
		pSkbData->aclDecision.aclEgrHitMask[i>>5]|=(1<<(i&0x1f));

	}

	/*Skip ----------------- check pure CF module ----------------- Skip*/

	return RT_EDP_ERR_OK;
}

int _rt_edp_egressACLAction(struct sk_buff *skb)
{
	uint32 i,acl_idx;
	rt_edp_aclFilterEntry_t* pChkRule;
	rt_edp_skb_t *pPktHdr = &skb->edpSkbData;

	/*Skip ----------------- pon_is_cfport ----------------- Skip*/

	//TRACE("Egress ACL Action:");

	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++)
	{
		if(rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[i]==-1)
			break;	//no more SW_ACL rules

		acl_idx = rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[i];
		if((pPktHdr->aclDecision.aclIgrHitMask[(acl_idx>>5)]&(1<<(acl_idx&0x1f))) && (pPktHdr->aclDecision.aclEgrHitMask[(acl_idx>>5)]&(1<<(acl_idx&0x1f)))){

			ACL_EDP("RG_ACL[%d] Hit!!!",acl_idx);
			ACL_EDP("pPktHdr->aclDecision.aclIgrDoneAction = 0x%x, aclEgrDoneAction = 0x%x",pPktHdr->aclDecision.aclIgrDoneAction,pPktHdr->aclDecision.aclEgrDoneAction);


			pChkRule = &(rt_edp_db.systemGlobal.acl_SW_table_entry[acl_idx]);

			if(pChkRule->acl_filter.action_type==ACL_ACTION_TYPE_QOS)
			{
				/*Skip ----------------- pPktHdr->aclDecision.action_type ----------------- Skip*/

				/*attentation:
				fwdtype=0 (ACL related rule, action control bit record in pPktHdr->aclDecision.aclIgrDoneAction )
				fwdtype=1~4 (CF related rule, action control bit record in pPktHdr->aclDecision.aclEgrDoneAction )
				  */

				if(pChkRule->acl_filter.qos_actions & RT_EDP_ACL_ACTION_1P_REMARKING_BIT){//belong to ACL action

					if((pPktHdr->aclDecision.aclIgrDoneAction&RT_EDP_ACL_IGR_CVLAN_ACT_DONE_BIT)==0x0) //ACL RT_EDP_ACL_IGR_CVLAN_ACT_DONE_BIT have not done
					{
							pPktHdr->aclDecision.qos_actions |= RT_EDP_ACL_ACTION_1P_REMARKING_BIT;
							pPktHdr->aclDecision.action_dot1p_remarking_pri = pChkRule->acl_filter.action_dot1p_remarking_pri;
							pPktHdr->aclDecision.aclIgrDoneAction |=RT_EDP_ACL_IGR_CVLAN_ACT_DONE_BIT;
							ACL_EDP("RG_ACL[%d] do 1P_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_dot1p_remarking_pri);
					}
					else
					{
						ACL_EDP("RG_ACL[%d] skip 1P_REMARKING to %d, because IGR_CVLAN_ACT is already done!", acl_idx, pChkRule->acl_filter.action_dot1p_remarking_pri);
					}
				}

				if(pChkRule->acl_filter.qos_actions & RT_EDP_ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT){//belong to ACL action or CF action

					if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)//ACL rule
					{
						if((pPktHdr->aclDecision.aclIgrDoneAction&RT_EDP_ACL_IGR_SVLAN_ACT_DONE_BIT)==0x0) //ACL RT_EDP_ACL_IGR_SVLAN_ACT_DONE_BIT have not done
						{
							pPktHdr->aclDecision.qos_actions |= RT_EDP_ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT;
							pPktHdr->aclDecision.action_ip_precedence_remarking_pri= pChkRule->acl_filter.action_ip_precedence_remarking_pri;
							pPktHdr->aclDecision.aclIgrDoneAction |= RT_EDP_ACL_IGR_SVLAN_ACT_DONE_BIT;
							ACL_EDP("RG_ACL[%d] do IP_PRECEDENCE_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_ip_precedence_remarking_pri);
						}
						else
						{
							ACL_EDP("RG_ACL[%d] skip IP_PRECEDENCE_REMARKING to %d, because IGR_SVLAN_ACT is already done!", acl_idx, pChkRule->acl_filter.action_ip_precedence_remarking_pri);
						}
					}
					else//CF rule(fwdtype=1~4)
					{
						if((pPktHdr->aclDecision.aclEgrDoneAction&RT_EDP_ACL_EGR_DSCP_ACT_DONE_BIT)==0x0) //CF RT_EDP_ACL_EGR_DSCP_ACT_DONE_BIT have not done
						{
							pPktHdr->aclDecision.qos_actions |= RT_EDP_ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT;
							pPktHdr->aclDecision.action_ip_precedence_remarking_pri= pChkRule->acl_filter.action_ip_precedence_remarking_pri;
							pPktHdr->aclDecision.aclEgrDoneAction |= RT_EDP_ACL_EGR_DSCP_ACT_DONE_BIT;
							ACL_EDP("RG_ACL[%d] do IP_PRECEDENCE_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_ip_precedence_remarking_pri);
						}
						else
						{
							ACL_EDP("RG_ACL[%d] skip IP_PRECEDENCE_REMARKING to %d, because EGR_DSCP_ACT is already done!", acl_idx, pChkRule->acl_filter.action_ip_precedence_remarking_pri);
						}
					}

				}


				if(pChkRule->acl_filter.qos_actions & RT_EDP_ACL_ACTION_DSCP_REMARKING_BIT){//belong to ACL action or CF action

					if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)//ACL rule
					{
						if((pPktHdr->aclDecision.aclIgrDoneAction&RT_EDP_ACL_IGR_SVLAN_ACT_DONE_BIT)==0x0) //ACL RT_EDP_ACL_IGR_SVLAN_ACT_DONE_BIT have not done
						{
							pPktHdr->aclDecision.qos_actions |= RT_EDP_ACL_ACTION_DSCP_REMARKING_BIT;
							pPktHdr->aclDecision.action_dscp_remarking_pri= pChkRule->acl_filter.action_dscp_remarking_pri;
							pPktHdr->aclDecision.aclIgrDoneAction |= RT_EDP_ACL_IGR_SVLAN_ACT_DONE_BIT;
							ACL_EDP("RG_ACL[%d] do DSCP_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_dscp_remarking_pri);
						}
						else
						{
							ACL_EDP("RG_ACL[%d] skip DSCP_REMARKING to %d, because IGR_SVLAN_ACT is already done!", acl_idx, pChkRule->acl_filter.action_dscp_remarking_pri);
						}
					}
					else//CF rule(fwdtype=1~4)
					{
						if((pPktHdr->aclDecision.aclEgrDoneAction&RT_EDP_ACL_EGR_DSCP_ACT_DONE_BIT)==0x0) //CF RT_EDP_ACL_EGR_DSCP_ACT_DONE_BIT have not done
						{
							pPktHdr->aclDecision.qos_actions |= RT_EDP_ACL_ACTION_DSCP_REMARKING_BIT;
							pPktHdr->aclDecision.action_dscp_remarking_pri= pChkRule->acl_filter.action_dscp_remarking_pri;
							pPktHdr->aclDecision.aclEgrDoneAction |= RT_EDP_ACL_EGR_DSCP_ACT_DONE_BIT;
							ACL_EDP("RG_ACL[%d] do DSCP_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_dscp_remarking_pri);
						}
						else
						{
							ACL_EDP("RG_ACL[%d] skip DSCP_REMARKING to %d, because CF[0-64] is already done!", acl_idx, pChkRule->acl_filter.action_dscp_remarking_pri);
						}
					}

				}

				if(pChkRule->acl_filter.qos_actions & RT_EDP_ACL_ACTION_TOS_TC_REMARKING_BIT){//belong to ACL action or CF action
					//HW belong to RT_EDP_ACL_ACTION_ACL_PRIORITY_BIT, but it will not happen HW PRIACT 0x0(acl trap with priority) and 0x6(tc_tos remarking), so SW make this action same effect as DSCP
					if(pChkRule->acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)//ACL rule
					{
						if((pPktHdr->aclDecision.aclIgrDoneAction&RT_EDP_ACL_IGR_SVLAN_ACT_DONE_BIT)==0x0) //ACL RT_EDP_ACL_IGR_SVLAN_ACT_DONE_BIT have not done
						{
							pPktHdr->aclDecision.qos_actions |= RT_EDP_ACL_ACTION_TOS_TC_REMARKING_BIT;
							pPktHdr->aclDecision.action_tos_tc_remarking_pri= pChkRule->acl_filter.action_tos_tc_remarking_pri;
							pPktHdr->aclDecision.aclIgrDoneAction |= RT_EDP_ACL_IGR_SVLAN_ACT_DONE_BIT;
							ACL_EDP("RG_ACL[%d] do TOS_TC_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_tos_tc_remarking_pri);
							/*Skip ----------------- swFlowOnlyByAcl ----------------- Skip*/
						}
						else
						{
							ACL_EDP("RG_ACL[%d] skip TOS_TC_REMARKING to %d, because IGR_SVLAN_ACT is already done!", acl_idx, pChkRule->acl_filter.action_tos_tc_remarking_pri);
						}
					}
					else//CF rule(fwdtype=1~4)
					{
						if((pPktHdr->aclDecision.aclEgrDoneAction&RT_EDP_ACL_EGR_DSCP_ACT_DONE_BIT)==0x0) //CF RT_EDP_ACL_EGR_DSCP_ACT_DONE_BIT have not done
						{
							pPktHdr->aclDecision.qos_actions |= RT_EDP_ACL_ACTION_TOS_TC_REMARKING_BIT;
							pPktHdr->aclDecision.action_tos_tc_remarking_pri= pChkRule->acl_filter.action_tos_tc_remarking_pri;
							pPktHdr->aclDecision.aclEgrDoneAction |= RT_EDP_ACL_EGR_DSCP_ACT_DONE_BIT;
							ACL_EDP("RG_ACL[%d] do TOS_TC_REMARKING to %d", acl_idx, pPktHdr->aclDecision.action_tos_tc_remarking_pri);
							/*Skip ----------------- swFlowOnlyByAcl ----------------- Skip*/
						}
						else
						{
							ACL_EDP("RG_ACL[%d] skip TOS_TC_REMARKING to %d, because CF[0-64](EGR_DSCP_ACT) is already done!", acl_idx, pChkRule->acl_filter.action_tos_tc_remarking_pri);
						}
					}

				}

				if(pChkRule->acl_filter.qos_actions & RT_EDP_ACL_ACTION_QUEUE_ID_BIT){
					ACL_EDP("ACL_ACTION_QUEUE_ID_BIT is not supported in EDP!");
				}

				if(pChkRule->acl_filter.qos_actions & RT_EDP_ACL_ACTION_SHARE_METER_BIT){
					ACL_EDP("ACL_ACTION_SHARE_METER_BIT is supported by linux in EDP!");
				}

				if(pChkRule->acl_filter.qos_actions & RT_EDP_ACL_ACTION_LOG_COUNTER_BIT){
					ACL_EDP("ACL_ACTION_LOG_COUNTER_BIT is not supported in EDP!");
				}

				if(pChkRule->acl_filter.qos_actions & RT_EDP_ACL_ACTION_ACL_PRIORITY_BIT){
					//aclPriority is used for internalPriority decision, no need to use after this function anymore.
					ACL_EDP("ACL_ACTION_ACL_PRIORITY_BIT is supported by linux in EDP!");
				}

				if(pChkRule->acl_filter.qos_actions & RT_EDP_ACL_ACTION_STREAM_ID_OR_LLID_BIT){//belong to CF action
					ACL_EDP("ACL_ACTION_STREAM_ID_OR_LLID_BIT is supported by linux in EDP!");
				}

				if(pChkRule->acl_filter.qos_actions & RT_EDP_ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT){//belong to CF action
					ACL_EDP("ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT is supported by linux in EDP!");
				}

				if(pChkRule->acl_filter.qos_actions & RT_EDP_ACL_ACTION_REDIRECT_BIT){//belong to ACL action
					ACL_EDP("ACL_ACTION_REDIRECT_BIT is not supported in EDP!");
				}

				if(pChkRule->acl_filter.qos_actions & RT_EDP_ACL_ACTION_ACL_CVLANTAG_BIT){//belong to CF action
					if((pPktHdr->aclDecision.aclEgrDoneAction&RT_EDP_ACL_EGR_CVLAN_ACT_DONE_BIT)==0x0){//RT_EDP_ACL_EGR_CVLAN_ACT_DONE_BIT have not been set

						pPktHdr->aclDecision.qos_actions|= RT_EDP_ACL_ACTION_ACL_CVLANTAG_BIT;
						pPktHdr->aclDecision.action_acl_cvlan.cvlanTagIfDecision=pChkRule->acl_filter.action_acl_cvlan.cvlanTagIfDecision;
						pPktHdr->aclDecision.action_acl_cvlan.cvlanCvidDecision=pChkRule->acl_filter.action_acl_cvlan.cvlanCvidDecision;
						pPktHdr->aclDecision.action_acl_cvlan.cvlanCpriDecision=pChkRule->acl_filter.action_acl_cvlan.cvlanCpriDecision;
						pPktHdr->aclDecision.action_acl_cvlan.assignedCvid=pChkRule->acl_filter.action_acl_cvlan.assignedCvid;
						pPktHdr->aclDecision.action_acl_cvlan.assignedCpri=pChkRule->acl_filter.action_acl_cvlan.assignedCpri;
						pPktHdr->aclDecision.aclEgrDoneAction|=RT_EDP_ACL_EGR_CVLAN_ACT_DONE_BIT;

						ACL_EDP("RG_ACL[%d] do CVLANTAG to %s %s %s vid=%d pri=%d",
							acl_idx,
							name_of_rt_edp_cvlan_tagif_decision[pPktHdr->aclDecision.action_acl_cvlan.cvlanTagIfDecision],
							name_of_rt_edp_cvlan_cvid_decision[pPktHdr->aclDecision.action_acl_cvlan.cvlanCvidDecision],
							name_of_rt_edp_cvlan_cpri_decision[pPktHdr->aclDecision.action_acl_cvlan.cvlanCpriDecision],
							pPktHdr->aclDecision.action_acl_cvlan.assignedCvid,
							pPktHdr->aclDecision.action_acl_cvlan.assignedCpri
							);
					}
					else
					{
						ACL_EDP("RG_ACL[%d] skip CVLANTAG to %s %s %s vid=%d pri=%d, because EGR_CVLAN_ACT is already done!",
							acl_idx,
							name_of_rt_edp_cvlan_tagif_decision[pChkRule->acl_filter.action_acl_cvlan.cvlanTagIfDecision],
							name_of_rt_edp_cvlan_cvid_decision[pChkRule->acl_filter.action_acl_cvlan.cvlanCvidDecision],
							name_of_rt_edp_cvlan_cpri_decision[pChkRule->acl_filter.action_acl_cvlan.cvlanCpriDecision],
							pChkRule->acl_filter.action_acl_cvlan.assignedCvid,
							pChkRule->acl_filter.action_acl_cvlan.assignedCpri
							);
					}
				}

				if(pChkRule->acl_filter.qos_actions & RT_EDP_ACL_ACTION_ACL_SVLANTAG_BIT){//belong CF action
					if((pPktHdr->aclDecision.aclEgrDoneAction&RT_EDP_ACL_EGR_SVLAN_ACT_DONE_BIT)==0x0){//RT_EDP_ACL_EGR_SVLAN_ACT_DONE_BIT have not been set

						pPktHdr->aclDecision.qos_actions|= RT_EDP_ACL_ACTION_ACL_SVLANTAG_BIT;
						pPktHdr->aclDecision.action_acl_svlan.svlanTagIfDecision=pChkRule->acl_filter.action_acl_svlan.svlanTagIfDecision;
						pPktHdr->aclDecision.action_acl_svlan.svlanSvidDecision=pChkRule->acl_filter.action_acl_svlan.svlanSvidDecision;
						pPktHdr->aclDecision.action_acl_svlan.svlanSpriDecision=pChkRule->acl_filter.action_acl_svlan.svlanSpriDecision;
						pPktHdr->aclDecision.action_acl_svlan.assignedSvid=pChkRule->acl_filter.action_acl_svlan.assignedSvid;
						pPktHdr->aclDecision.action_acl_svlan.assignedSpri=pChkRule->acl_filter.action_acl_svlan.assignedSpri;
						pPktHdr->aclDecision.aclEgrDoneAction|=RT_EDP_ACL_EGR_SVLAN_ACT_DONE_BIT;

						ACL_EDP("RG_ACL[%d] do SVLANTAG to %s %s %s svid=%d spri=%d",
							acl_idx,
							name_of_rt_edp_svlan_tagif_decision[pPktHdr->aclDecision.action_acl_svlan.svlanTagIfDecision],
							name_of_rt_edp_svlan_svid_decision[pPktHdr->aclDecision.action_acl_svlan.svlanSvidDecision],
							name_of_rt_edp_svlan_spri_decision[pPktHdr->aclDecision.action_acl_svlan.svlanSpriDecision],
							pPktHdr->aclDecision.action_acl_svlan.assignedSvid,
							pPktHdr->aclDecision.action_acl_svlan.assignedSpri
							);

					}
					else
					{
						ACL_EDP("RG_ACL[%d] skip SVLANTAG to %s %s %s svid=%d spri=%d, because EGR_SVLAN_ACT is already done!",
							acl_idx,
							name_of_rt_edp_svlan_tagif_decision[pChkRule->acl_filter.action_acl_svlan.svlanTagIfDecision],
							name_of_rt_edp_svlan_svid_decision[pChkRule->acl_filter.action_acl_svlan.svlanSvidDecision],
							name_of_rt_edp_svlan_spri_decision[pChkRule->acl_filter.action_acl_svlan.svlanSpriDecision],
							pChkRule->acl_filter.action_acl_svlan.assignedSvid,
							pChkRule->acl_filter.action_acl_svlan.assignedSpri
							);

					}
				}

				if(pChkRule->acl_filter.qos_actions & RT_EDP_ACL_ACTION_DS_UNIMASK_BIT){ //belong to CF action
					ACL_EDP("ACL_ACTION_DS_UNIMASK_BIT is not supported in EDP!");
				}
			}

		}
	}

	/*Skip ----------------- check CF pattern1 ----------------- Skip*/

	return RT_EDP_ERR_OK;
}

int _rt_edp_egressACLPatternCheckAndAction(struct sk_buff *skb)
{
	rt_edp_aclCheck_data_t pktData;
	rt_edp_aclCheck_data_t *pData = &pktData;

	if(skb->edpSkbData.aclDecision.aclEgrHaveToCheckRuleIdx[0] == -1) {
		ACL_EDP("no ingress RG_ACL need to verify");
		return RT_EDP_ERR_OK;
	}

	bzero(&pktData, sizeof(pktData));

	ASSERT_EQ(_rt_edp_acl_pattern_get(skb, pData),RT_EDP_ERR_OK);

	ASSERT_EQ(_rt_edp_egressACLPatternCheck(skb, pData),RT_EDP_ERR_OK);

	ASSERT_EQ(_rt_edp_egressACLAction(skb),RT_EDP_ERR_OK);

	return RT_EDP_ERR_OK;
}

int _rt_edp_modifyPacketByACLAction(struct sk_buff *skb)
{
	rt_edp_skb_t *pPktHdr = &skb->edpSkbData;
	rt_edp_aclCheck_data_t pktData;
	rt_edp_aclCheck_data_t *pData = &pktData;

	int value;
	struct iphdr *ip = NULL;
	struct ipv6hdr *ipv6h = NULL;

	if(pPktHdr->aclDecision.qos_actions == 0x0)
		return RT_EDP_ERR_OK;

	bzero(&pktData, sizeof(pktData));

	ASSERT_EQ(_rt_edp_acl_pattern_get(skb, pData),RT_EDP_ERR_OK);

	/* do RG ACL Qos actions */
	if(pPktHdr->aclDecision.qos_actions != 0x0) {
#if 0	//under development
		if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_1P_REMARKING_BIT)>0)
		{
			pPktHdr->egressPriority = pPktHdr->aclDecision.action_dot1p_remarking_pri;
			ACL_EDP("Modify by RG_ACL ACT:1P_REMARKING egreesPri=%d",pPktHdr->aclDecision.action_dot1p_remarking_pri);
		}
#endif

		if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_TOS_TC_REMARKING_BIT)>0)
		{
			if(!(pData->tagif&IPV4_TAGIF || pData->tagif&IPV6_TAGIF)){
				ACL_EDP("Modify by RG_ACL ACT: Failed! Due to packet has no IP header");
			}else{
				value = -1;
				if(pData->tagif&IPV6_TAGIF)
				{
					ipv6h = ipv6_hdr(skb);
					if(ipv6h) {
						ipv6_change_dsfield(ipv6h, 0, pPktHdr->aclDecision.action_tos_tc_remarking_pri);
						value = ((ipv6h->priority << 4) | ((ipv6h->flow_lbl[0] & 0xF0) >> 4));
					}
				}
				else
				{
					ip = ip_hdr(skb);
					if(ip) {
						ipv4_change_dsfield(ip, 0, pPktHdr->aclDecision.action_tos_tc_remarking_pri);
						value = ip->tos;
					}
				}
				ACL_EDP("Modify by RG_ACL ACT:TOS_TC_REMARKING %s %d",(pData->tagif&IPV6_TAGIF)?"TC":"TOS",value);
			}
		}
#if 0	//under development

		else if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT)>0)
		{
			if(pPktHdr->pTos==NULL){
				ACL_EDP("Modify by RG_ACL ACT: Failed! Due to packet has no pTos Field");
			}		//packet may not have IP header
			else if(!(pPktHdr->tagif&IPV4_TAGIF || pPktHdr->tagif&IPV6_TAGIF)){
				ACL_EDP("Modify by RG_ACL ACT: Failed! Due to packet has no IP header");
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
				ACL_EDP("Modify by RG_ACL ACT:IP_PRECEDENCE_REMARKING ToS=%d",tos);
			}
		}
#endif
		else if((pPktHdr->aclDecision.qos_actions&ACL_ACTION_DSCP_REMARKING_BIT)>0)
		{
			if(!(pData->tagif&IPV4_TAGIF || pData->tagif&IPV6_TAGIF)){
				ACL_EDP("Modify by RG_ACL ACT: Failed! Due to packet has no IP header");
			}else{
				value = -1;
				if(pData->tagif&IPV6_TAGIF)
				{
					ipv6h = ipv6_hdr(skb);
					if(ipv6h) {
						value = pPktHdr->aclDecision.action_dscp_remarking_pri << 0x2;
						ipv6_change_dsfield(ipv6h, INET_ECN_MASK, value );
						value = pPktHdr->aclDecision.action_dscp_remarking_pri;
					}
				}
				else
				{
					ip = ip_hdr(skb);
					if(ip) {
						value = pPktHdr->aclDecision.action_dscp_remarking_pri << 0x2;
						ipv4_change_dsfield(ip, INET_ECN_MASK, value);
						value = pPktHdr->aclDecision.action_dscp_remarking_pri;
					}
				}
				ACL_EDP("Modify by RG_ACL ACT:DSCP_REMARKING DSCP=%d", value);
			}
		}

#if 0	//under development
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
									ACL_EDP("Modify by RG_ACL ACT:CVID_ASSIGN CVID=%d",pPktHdr->egressVlanID);
								}
								break;
							case ACL_CVLAN_CVID_COPY_FROM_1ST_TAG:
								{
									if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
										pPktHdr->egressVlanID = pPktHdr->stagVid;
										pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
										ACL_EDP("Modify by RG_ACL ACT:CVID_COPY_FROM_1ST_TAG(S) CVID=%d",pPktHdr->egressVlanID);
									}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
										pPktHdr->egressVlanID = pPktHdr->ctagVid;
										pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
										ACL_EDP("Modify by RG_ACL ACT:CVID_COPY_FROM_1ST_TAG(C) CVID=%d",pPktHdr->egressVlanID);
									}else{
										//no 1st tag, use assignedCvid as H/W
										pPktHdr->egressVlanID = pPktHdr->aclDecision.action_acl_cvlan.assignedCvid;
										pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
										ACL_EDP("Modify by RG_ACL ACT:CVID_COPY_FROM_1ST_TAG(none tag) CVID=%d",pPktHdr->egressVlanID);
									}
								}
								break;
							case ACL_CVLAN_CVID_COPY_FROM_2ND_TAG:
								{
									if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
										pPktHdr->egressVlanID = pPktHdr->ctagVid;
										pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
										ACL_EDP("Modify by RG_ACL ACT:CVID_COPY_FROM_2ND_TAG CVID=%d",pPktHdr->egressVlanID);
									}else{
										//no 2nd tag, use assignedCvid as H/W
										pPktHdr->egressVlanID = pPktHdr->aclDecision.action_acl_cvlan.assignedCvid;
										pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
										ACL_EDP("Modify by RG_ACL ACT:CVID_COPY_FROM_2ND_TAG(none tag) CVID=%d",pPktHdr->egressVlanID);
									}
								}
								break;
							case ACL_CVLAN_CVID_COPY_FROM_INTERNAL_VID://upstream only
								{
									pPktHdr->egressVlanID = pPktHdr->internalVlanID;
									pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
									ACL_EDP("Modify by RG_ACL ACT:CVID_COPY_FROM_INTERNAL_VID CVID=%d",pPktHdr->egressVlanID);
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
									ACL_EDP("Modify by RG_ACL ACT:CPRI_ASSIGN CPRI=%d",pPktHdr->egressPriority);
								}
								break;
							case ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG:
								{
									if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
										pPktHdr->egressPriority = pPktHdr->stagPri;
										ACL_EDP("Modify by RG_ACL ACT:CPRI_COPY_FROM_1ST_TAG(S) CPRI=%d",pPktHdr->egressPriority);
									}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
										pPktHdr->egressPriority = pPktHdr->ctagPri;
										ACL_EDP("Modify by RG_ACL ACT:CPRI_COPY_FROM_1ST_TAG(C) CPRI=%d",pPktHdr->egressPriority);
									}else{
										//no 1st tag, use assignedCpri as H/W
										pPktHdr->egressPriority = pPktHdr->aclDecision.action_acl_cvlan.assignedCpri;
										ACL_EDP("Modify by RG_ACL ACT:CPRI_COPY_FROM_1ST_TAG(none tag) CPRI=%d",pPktHdr->egressPriority);
									}
								}
								break;
							case ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG:
								{
									if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
										pPktHdr->egressVlanID = pPktHdr->ctagVid;
										pPktHdr->dmac2VlanID = FAIL; //avoid mac2cvid change the egressVID again.
										ACL_EDP("Modify by RG_ACL ACT:CVID_COPY_FROM_2ND_TAG CVID=%d",pPktHdr->egressVlanID);
									}else{
										//no 2nd tag, use assignedCpri as H/W
										pPktHdr->egressPriority = pPktHdr->aclDecision.action_acl_cvlan.assignedCpri;
										ACL_EDP("Modify by RG_ACL ACT:CPRI_COPY_FROM_1ST_TAG(none tag) CPRI=%d",pPktHdr->egressPriority);
									}
								}
								break;
							case ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI:
								{
									pPktHdr->egressPriority = pPktHdr->internalPriority;
									ACL_EDP("Modify by RG_ACL ACT:CPRI_COPY_FROM_INTERNAL CPRI=%d",pPktHdr->egressPriority);
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
						ACL_EDP("Modify by RG_ACL ACT:UN-CTAG");
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
						ACL_EDP("Modify by RG_ACL ACT:CTAG TRANSPARENT");
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
						ACL_EDP("Modify by RG_ACL ACT:UN-STAG");
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
						ACL_EDP("Modify by RG_ACL ACT:STAG TRANSPARENT");
					}
					break;

				case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID:
					pPktHdr->egressServiceVlanTagif = 1; //force tagging with tpid
					ACL_EDP("Modify by RG_ACL ACT:SVLANTAG with TPID");
					goto svidSpriDecision;
				case ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2:
					pPktHdr->egressServiceVlanTagif = 2; //force tagging with tpid2
					ACL_EDP("Modify by RG_ACL ACT:SVLANTAG with TPID2");
					goto svidSpriDecision;
				case ACL_SVLAN_TAGIF_TAGGING_WITH_ORIGINAL_STAG_TPID:
					pPktHdr->egressServiceVlanTagif = 3; //force tagging with original-stag-tpid
					ACL_EDP("Modify by RG_ACL ACT:SVLANTAG with TPID2");
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
							ACL_EDP("Modify by RG_ACL ACT:SVID_ASSIGN SVID=%d",pPktHdr->egressServiceVlanID);
						}
						break;
					case ACL_SVLAN_SVID_COPY_FROM_1ST_TAG:
						{
							if(pPktHdr->tagif & SVLAN_TAGIF){
								pPktHdr->egressServiceVlanID = pPktHdr->stagVid;
								ACL_EDP("Modify by RG_ACL ACT:SVID_COPY_FROM_1ST_TAG(S) SVID=%d",pPktHdr->egressServiceVlanID);
							}else if (pPktHdr->tagif & CVLAN_TAGIF){
								pPktHdr->egressServiceVlanID = pPktHdr->ctagVid;
								ACL_EDP("Modify by RG_ACL ACT:SVID_COPY_FROM_1ST_TAG(C) SVID=%d",pPktHdr->egressServiceVlanID);
							}else{
								//no 1st tag, use assignedSvid as H/W
								pPktHdr->egressServiceVlanID = pPktHdr->aclDecision.action_acl_svlan.assignedSvid;
								ACL_EDP("Modify by RG_ACL ACT:SVID_COPY_FROM_1ST_TAG(none tag) SVID=%d",pPktHdr->egressServiceVlanID);
							}
						}
						break;
					case ACL_SVLAN_SVID_COPY_FROM_2ND_TAG:
						{
							if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){
								pPktHdr->egressServiceVlanID = pPktHdr->ctagVid;
								ACL_EDP("Modify by RG_ACL ACT:SVID_COPY_FROM_2ND_TAG SVID=%d",pPktHdr->egressServiceVlanID);
							}else{
								//no 2nd tag, use assignedSvid as H/W
								pPktHdr->egressServiceVlanID = pPktHdr->aclDecision.action_acl_svlan.assignedSvid;
								ACL_EDP("Modify by RG_ACL ACT:SVID_COPY_FROM_2ND_TAG(none tag) SVID=%d",pPktHdr->egressServiceVlanID);
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
							ACL_EDP("Modify by RG_ACL ACT:SPRI_ASSIGN SPRI=%d",pPktHdr->egressServicePriority);
						}
						break;
					case ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG:
						{
							if(pPktHdr->tagif & SVLAN_TAGIF){ //outter tag is Stag
								pPktHdr->egressServicePriority = pPktHdr->stagPri;
								ACL_EDP("Modify by RG_ACL ACT:SPRI_COPY_FROM_1ST_TAG(S) SPRI=%d",pPktHdr->egressServicePriority);
							}else if (pPktHdr->tagif & CVLAN_TAGIF){ //outter tag is Ctag
								pPktHdr->egressServicePriority = pPktHdr->ctagPri;
								ACL_EDP("Modify by RG_ACL ACT:SPRI_COPY_FROM_1ST_TAG(C) SPRI=%d",pPktHdr->egressServicePriority);
							}else{
								//no 1st tag, use assignedSpri as H/W
								pPktHdr->egressServicePriority = pPktHdr->aclDecision.action_acl_svlan.assignedSpri;
								ACL_EDP("Modify by RG_ACL ACT:SPRI_COPY_FROM_1ST_TAG(none tag) SPRI=%d",pPktHdr->egressServicePriority);
							}
						}
						break;
					case ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG:
						{
							if((pPktHdr->tagif & SVLAN_TAGIF) && (pPktHdr->tagif & CVLAN_TAGIF)){ //have double tag
								pPktHdr->egressServicePriority = pPktHdr->ctagPri;
								ACL_EDP("Modify by RG_ACL ACT:SPRI_COPY_FROM_2ND_TAG SPRI=%d",pPktHdr->egressServicePriority);
							}else{
								//no 2nd tag, use assignedSpri as H/W
								pPktHdr->egressServicePriority = pPktHdr->aclDecision.action_acl_svlan.assignedSpri;
								ACL_EDP("Modify by RG_ACL ACT:SPRI_COPY_FROM_2ND_TAG(none tag) SPRI=%d",pPktHdr->egressServicePriority);
							}

						}
						break;
					case ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI:
						{
							pPktHdr->egressServicePriority = pPktHdr->internalPriority;
							ACL_EDP("Modify by RG_ACL ACT:SPRI_COPY_FROM_INTERNAL_PRI SPRI=%d",pPktHdr->egressServicePriority);
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
#endif
	}

	/* do RG CF pattern1 actions */
	/*Skip ----------------- do RG CF pattern1 actions ----------------- Skip*/

	return RT_EDP_ERR_OK;
}
#endif	//EDP_ACL_SUPPORT_HOOK_CHECK

/*(6)debug tool APIs*/
int _dump_rt_edp_acl(struct seq_file *s)
{
	int i, j;
	rtk_rg_aclFilterAndQos_t *aclPara;

	PROC_PRINTF("acl_SW_table_entry_size:%d\n",rt_edp_db.systemGlobal.acl_SW_table_entry_size);

	PROC_PRINTF("aclSW rule index sorting:\n");
	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){
		if(rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[i]==-1)
			break;
		PROC_PRINTF("ACL[%d]:w(%d)",rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[i],
			rt_edp_db.systemGlobal.acl_SW_table_entry[(rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[i])].acl_filter.acl_weight);
		if(i+1!=MAX_ACL_SW_ENTRY_SIZE && rt_edp_db.systemGlobal.acl_SWindex_sorting_by_weight[i+1]>=0)
			PROC_PRINTF(" > ");
	}
	PROC_PRINTF("\n");
	if(rt_edp_db.systemGlobal.stop_add_hw_acl>0)
		PROC_PRINTF("Stop rest ACL rule add to H/W ACL from index %d.\n", rt_edp_db.systemGlobal.stop_add_hw_acl-1);

	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){
		//if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields!=0x0){
		//DEBUG("aclSWEntry[i],valid = %d",i,rt_edp_db.systemGlobal.acl_SW_table_entry[i].valid);
		if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].valid==RT_EDP_ENABLED){
			aclPara = &(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter);
			PROC_PRINTF("========================RG_ACL[%d]===========================\n",i);
#if defined(CONFIG_RG_RTL9607C_SERIES)	//no need to use HW ACL
			PROC_PRINTF("[hw_acl_start:%d(continue:%d) hw_cf_start:%d(continue:%d)] \n"
				,rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_aclEntry_start
				,rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_aclEntry_size
				,rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_cfEntry_start
				,rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_cfEntry_size);

			PROC_PRINTF("[Using range tables]: \n");
			if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_INGRESS_SIP4TABLE) PROC_PRINTF("ACL_SIP4_RANGE_TABLE[%d]  \n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_SIP4TABLE_INDEX]);
			if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_INGRESS_DIP4TABLE) PROC_PRINTF("ACL_DIP4_RANGE_TABLE[%d]  \n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_DIP4TABLE_INDEX]);
			if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_INGRESS_SIP6TABLE) PROC_PRINTF("ACL_SIP6_RANGE_TABLE[%d]  \n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_SIP6TABLE_INDEX]);
			if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_INGRESS_DIP6TABLE) PROC_PRINTF("ACL_DIP6_RANGE_TABLE[%d]  \n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_DIP6TABLE_INDEX]);
			if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_INGRESS_SPORTTABLE) PROC_PRINTF("ACL_SPORT_RANGE_TABLE[%d]  \n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_SPORTTABLE_INDEX]);
			if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_INGRESS_DPORTTABLE) PROC_PRINTF("ACL_DPORT_RANGE_TABLE[%d]  \n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_INGRESS_DPORTTABLE_INDEX]);
			if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_EGRESS_SIP4TABLE) PROC_PRINTF("CF_SIP4_RANGE_TABLE[%d] \n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_EGRESS_SIP4TABLE_INDEX]);
			if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_EGRESS_DIP4TABLE) PROC_PRINTF("CF_DIP4_RANGE_TABLE[%d] \n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_EGRESS_DIP4TABLE_INDEX]);
			if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_EGRESS_SIP6TABLE) PROC_PRINTF("CF_DSCP_TABLE[%d] \n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_EGRESS_SIP6TABLE_INDEX]);
			if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_EGRESS_DIP6TABLE) PROC_PRINTF("CF_DSCP_TABLE[%d] \n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_EGRESS_DIP6TABLE_INDEX]);
			if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_EGRESS_SPORTTABLE) PROC_PRINTF("CF_SPORT_RANGE_TABLE[%d] \n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_EGRESS_SPORTTABLE_INDEX]);
			if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table&APOLLOFE_RG_ACL_USED_EGRESS_DPORTTABLE) PROC_PRINTF("CF_DPORT_RANGE_TABLE[%d] \n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_used_table_index[APOLLOFE_RG_ACL_USED_EGRESS_DPORTTABLE_INDEX]);
#elif defined(CONFIG_RG_G3_SERIES)
			PROC_PRINTF("[hw_acl_index:%s(port:0x%x, total:%d)] \n"
				,rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_aclEntry_index
				,rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_aclEntry_port.bits[0]
				,rt_edp_db.systemGlobal.acl_SW_table_entry[i].hw_aclEntry_count);
#endif

			PROC_PRINTF("[Using linux tables]: \n");
			_rt_edp_linux_debug_dump(rt_edp_db.systemGlobal.acl_SW_table_entry[i].tables_internal_check, rt_edp_db.systemGlobal.acl_SW_table_entry[i].ptr_nsup_table, rt_edp_db.systemGlobal.acl_SW_table_entry[i].ptr_nsup_chain, rt_edp_db.systemGlobal.acl_SW_table_entry[i].act_nsup_table, rt_edp_db.systemGlobal.acl_SW_table_entry[i].act_nsup_chain);
			for(j=0; j<RT_EDP_ACL_USED_TABLE_END; j++) {
				if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].linux_used_table&(0x1<<j))
					PROC_PRINTF("%s/%s/chain %s - start:%d(continue:%d)\n",
						linux_table[j].utility_buf,linux_table[j].table_buf,linux_table[j].chain_buf,
						rt_edp_db.systemGlobal.acl_SW_table_entry[i].linux_used_table_index[j][0],
						rt_edp_db.systemGlobal.acl_SW_table_entry[i].linux_used_table_index[j][1]);
			}

			_dump_rt_edp_acl_entry_content(s, aclPara);

		}

	}

	return RT_EDP_ERR_OK;
}




int _dump_rt_edp_acl_and_cf_diagshell(struct seq_file *s)
{
	int i;
	uint8	unknownCmdFlag=0;
	int tagDecision=0, vidDecision=0, priDecision=0;//ther value should reference to rg_acl.cli

	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++){
		//rtk_rg_acl_filter_fields_e check 1st
		if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields!=0x0){ 	//no more rules need to check
		//uint8 setFlag=0;
		//char tmpStr[255];
		//memset(tmpStr,0,255);

	PROC_PRINTF("rg clear acl-filter\n");
				if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.acl_weight!=0)	//not shown while zero
	PROC_PRINTF("rg set acl-filter acl_weight %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.acl_weight);

	PROC_PRINTF("rg set acl-filter fwding_type_and_direction %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction);

	//action
	PROC_PRINTF("rg set acl-filter action action_type %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_type);
	if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_type==ACL_ACTION_TYPE_POLICY_ROUTE)
	PROC_PRINTF("rg set acl-filter action policy-route egress_intf_idx %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_policy_route_wan);

	if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_type==ACL_ACTION_TYPE_FLOW_MIB)
	PROC_PRINTF("rg set acl-filter action flow-mib flow_counter_idx %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_flowmib_counter_idx);

	if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_type==ACL_ACTION_TYPE_SW_MIRROR_WITH_UDP_ENCAP)
	PROC_PRINTF("rg set acl-filter action udp_encap assign_smac %x:%x:%x:%x:%x:%x assign_dmac %x:%x:%x:%x:%x:%x assign_sip %d.%d.%d.%d assign_dip %d.%d.%d.%d assign_dport %d \n",
		rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[0],
		rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[1],
		rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[2],
		rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[3],
		rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[4],
		rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_smac.octet[5],
		rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[0],
		rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[1],
		rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[2],
		rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[3],
		rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[4],
		rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_dmac.octet[5],
		(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip & 0xff000000)>>24,
		(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip & 0xff0000)>>16,
		(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip & 0xff00)>>8,
		(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Sip & 0xff),
		(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip & 0xff000000)>>24,
		(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip & 0xff0000)>>16,
		(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip & 0xff00)>>8,
		(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dip & 0xff),
		rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_encap_udp.encap_Dport);

	if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_type==ACL_ACTION_TYPE_TRAP_WITH_PRIORITY)
		PROC_PRINTF("rg set acl-filter action trap_with_priority acl_priority %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_trap_with_priority);

	if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_type==ACL_ACTION_TYPE_QOS)
	{

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_ACL_PRIORITY_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_acl_priority %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_priority);
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_1P_REMARKING_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_dot1p_remarking_pri %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_dot1p_remarking_pri);
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_DSCP_REMARKING_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_dscp_remarking_pri %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_dscp_remarking_pri);
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_TOS_TC_REMARKING_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_tos_tc_remarking_pri %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_tos_tc_remarking_pri);
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_ip_precedence_remarking_pri %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_ip_precedence_remarking_pri);
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_QUEUE_ID_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_queue_id %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_queue_id);
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_egress_internal_priority %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_internal_priority);
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_STREAM_ID_OR_LLID_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_stream_id %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_stream_id_or_llid);
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_ACL_INGRESS_VID_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_ingress_vid %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_ingress_vid);
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_REDIRECT_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_redirect 0x%x\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.redirect_portmask);
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_DS_UNIMASK_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_downstream_uni_portmask portmask 0x%x\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.downstream_uni_portmask);
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_SHARE_METER_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_share_meter %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_share_meter);
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_LOG_COUNTER_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter action qos action_log_counter %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_log_counter);
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_ACL_CVLANTAG_BIT)!=0)
		{
			//init varible
			unknownCmdFlag=0;
			tagDecision=-1;
			vidDecision=-1;
			priDecision=-1;

			switch(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_cvlan.cvlanTagIfDecision){
			case ACL_CVLAN_TAGIF_NOP: tagDecision=0; break;
			case ACL_CVLAN_TAGIF_TAGGING: tagDecision=1;break;
			case ACL_CVLAN_TAGIF_UNTAG: tagDecision=2;break;
			case ACL_CVLAN_TAGIF_TRANSPARENT: tagDecision=3;break;
			default:
				unknownCmdFlag =1;
				break;
			}

			switch(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_cvlan.cvlanCvidDecision)
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

			switch(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_cvlan.cvlanCpriDecision)
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
				rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_cvlan.assignedCvid,
				rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_cvlan.assignedCpri);
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.qos_actions & ACL_ACTION_ACL_SVLANTAG_BIT)!=0)
		{

			//init varible
			unknownCmdFlag=0;
			tagDecision=-1;
			vidDecision=-1;
			priDecision=-1;

			switch(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_svlan.svlanTagIfDecision){
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

			switch(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_svlan.svlanSvidDecision)
			{
				case ACL_SVLAN_SVID_ASSIGN: vidDecision=1; break;
				case ACL_SVLAN_SVID_COPY_FROM_1ST_TAG: vidDecision=2; break;
				case ACL_SVLAN_SVID_COPY_FROM_2ND_TAG: vidDecision=3; break;
				case ACL_SVLAN_SVID_NOP: vidDecision=0; break;
				default:
					unknownCmdFlag =1;
					break;
			}
			switch(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_svlan.svlanSpriDecision)
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
				rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_svlan.assignedSvid,
				rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.action_acl_svlan.assignedSpri);

			}
		}
	}


		//pattern: egress
		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_CTAG_PRI_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_ctag_pri %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_ctag_pri);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& EGRESS_CTAG_PRI_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_ctag_pri \n");
			}
		}
		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_CTAG_VID_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_ctag_vid %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_ctag_vid);
			PROC_PRINTF("rg set acl-filter pattern egress_ctag_vid_mask %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_ctag_vid_mask);

			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& EGRESS_CTAG_VID_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_ctag_vid \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_IPV4_DIP_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_dest_ipv4_addr_start %d.%d.%d.%d engress_dest_ipv4_addr_end %d.%d.%d.%d\n",
			(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_start & 0xff000000)>>24,
			(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_start & 0xff0000)>>16,
			(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_start & 0xff00)>>8,
			(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_start & 0xff),
			(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_end & 0xff000000)>>24,
			(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_end & 0xff0000)>>16,
			(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_end & 0xff00)>>8,
			(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_ipv4_addr_end & 0xff));

			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& EGRESS_IPV4_DIP_RANGE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_dest_ipv4_addr_range \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_INTF_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_intf_idx %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_intf_idx);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& EGRESS_INTF_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_intf_idx \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_EGRESS_PORTIDX_BIT)!=0)
		{

			if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP ||
				rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN ||
				rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_TRAP ||
				rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_PERMIT)
			{
				PROC_PRINTF("rg set acl-filter pattern egress_port_idx %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_port_idx);
				PROC_PRINTF("rg set acl-filter pattern egress_port_idx_mask 0x%x\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_port_idx_mask);

				if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& INGRESS_EGRESS_PORTIDX_BIT)!=0)
				{
					PROC_PRINTF("rg set acl-filter pattern not egress_port_idx \n");
				}
			}
			else if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP||
				rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN ||
				rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_TRAP ||
				rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_PERMIT)
			{
				PROC_PRINTF("rg set acl-filter pattern ingress_port_idx %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_port_idx);
				PROC_PRINTF("rg set acl-filter pattern ingress_port_idx_mask 0x%x\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_port_idx_mask);
				if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& INGRESS_EGRESS_PORTIDX_BIT)!=0)
				{
					PROC_PRINTF("rg set acl-filter pattern not ingress_port_idx \n");
				}
			}
			else if(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET)
			{
				PROC_PRINTF("rg set acl-filter pattern egress_port_idx %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_port_idx);
				PROC_PRINTF("rg set acl-filter pattern egress_port_idx_mask 0x%x\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_port_idx_mask);

				if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& INGRESS_EGRESS_PORTIDX_BIT)!=0)
				{
					PROC_PRINTF("rg set acl-filter pattern not egress_port_idx \n");
				}
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_IPV4_SIP_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_src_ipv4_addr_start %d.%d.%d.%d egress_src_ipv4_addr_end %d.%d.%d.%d\n",
					(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_start & 0xff000000)>>24,
					(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_start & 0xff0000)>>16,
					(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_start & 0xff00)>>8,
					(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_start & 0xff),
					(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_end & 0xff000000)>>24,
					(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_end & 0xff0000)>>16,
					(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_end & 0xff00)>>8,
					(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_ipv4_addr_end & 0xff));

			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& EGRESS_IPV4_SIP_RANGE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_src_ipv4_addr_range \n");
			}
		}


		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_L4_SPORT_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_src_l4_port_start %d egress_src_l4_port_end %d\n",
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_l4_port_start,
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_src_l4_port_end);

			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& EGRESS_L4_SPORT_RANGE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_src_l4_port_range \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_L4_DPORT_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_dest_l4_port_start %d egress_dest_l4_port_end %d\n",
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_l4_port_start,
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dest_l4_port_end);

			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& EGRESS_L4_DPORT_RANGE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_dest_l4_port_range \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_IP4MC_IF)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_ip4mc_if %d\n",
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_ip4mc_if);

			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& EGRESS_IP4MC_IF)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_ip4mc_if \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_IP6MC_IF)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_ip6mc_if %d\n",
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_ip6mc_if);

			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& EGRESS_IP6MC_IF)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_ip6mc_if \n");
			}
		}

		//pattern: ingress
		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV4_TAGIF_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ipv4_tagif %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ipv4_tagif);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& INGRESS_IPV4_TAGIF_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_ipv4_tagif \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_TAGIF_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ipv6_tagif %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ipv6_tagif);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& INGRESS_IPV6_TAGIF_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_ipv6_tagif \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_CTAG_PRI_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ctag_pri %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ctag_pri);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_CTAG_PRI_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_ctag_pri \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_CTAG_VID_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ctag_vid %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ctag_vid);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_CTAG_VID_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_ctag_vid \n");
			}
		}
		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_CTAG_CFI_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ctag_cfi %d \n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ctag_cfi);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_CTAG_CFI_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_ctag_cfi \n");
			}
		}
		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV4_DIP_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_dest_ipv4_addr_start %d.%d.%d.%d ingress_dest_ipv4_addr_end %d.%d.%d.%d\n",
					(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_start & 0xff000000)>>24,
					(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_start & 0xff0000)>>16,
					(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_start & 0xff00)>>8,
					(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_start & 0xff),
					(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_end & 0xff000000)>>24,
					(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_end & 0xff0000)>>16,
					(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_end & 0xff00)>>8,
					(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv4_addr_end & 0xff));

			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_IPV4_DIP_RANGE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_dest_ipv4_addr_range \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_DIP_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_dest_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[0],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[1],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[2],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[3],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[4],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[5],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[6],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[7],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[8],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[9],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[10],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[11],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[12],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[13],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[14],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr[15]);


			PROC_PRINTF("rg set acl-filter pattern ingress_dest_ipv6_addr_mask %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[0],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[1],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[2],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[3],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[4],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[5],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[6],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[7],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[8],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[9],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[10],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[11],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[12],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[13],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[14],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_mask[15]);

			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& INGRESS_IPV6_DIP_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_dest_ipv6_addr \n");
			}


		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_DIP_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_dest_ipv6_addr_start %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ingress_dest_ipv6_addr_end %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[0],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[1],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[2],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[3],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[4],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[5],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[6],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[7],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[8],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[9],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[10],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[11],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[12],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[13],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[14],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_start[15],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[0],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[1],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[2],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[3],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[4],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[5],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[6],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[7],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[8],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[9],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[10],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[11],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[12],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[13],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[14],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_ipv6_addr_end[15]);


			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_IPV6_DIP_RANGE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_dest_ipv6_addr_range \n");
			}

		}


		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_DMAC_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_dmac %x:%x:%x:%x:%x:%x\n",
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac.octet[0],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac.octet[1],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac.octet[2],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac.octet[3],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac.octet[4],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac.octet[5]);

			PROC_PRINTF("rg set acl-filter pattern ingress_dmac_mask %x:%x:%x:%x:%x:%x\n",
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[0],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[1],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[2],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[3],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[4],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dmac_mask.octet[5]);

			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& INGRESS_DMAC_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_dmac \n");
			}
		}


		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_DSCP_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_dscp %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dscp);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& INGRESS_DSCP_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_dscp \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_ETHERTYPE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ethertype 0x%x\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ethertype);
			PROC_PRINTF("rg set acl-filter pattern ingress_ethertype_mask 0x%x\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ethertype_mask);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& INGRESS_ETHERTYPE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_ethertype \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_INTF_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_intf_idx %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_intf_idx);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& INGRESS_INTF_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_intf_idx \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_DSCP_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ipv6_dscp %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ipv6_dscp);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse& INGRESS_IPV6_DSCP_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_ipv6_dscp \n");
			}
		}


		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_UDP_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_l4_protocal 0\n");
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_L4_UDP_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_l4_protocal \n");
			}
		}
		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_TCP_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_l4_protocal 1\n");
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_L4_TCP_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_l4_protocal \n");
			}
		}
		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_ICMP_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_l4_protocal 2\n");
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_L4_ICMP_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_l4_protocal \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_NONE_TCP_NONE_UDP_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_l4_protocal 3\n");
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_L4_NONE_TCP_NONE_UDP_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_l4_protocal \n");
			}
		}


		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_POROTCAL_VALUE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_l4_protocal_value 0x%x\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_l4_protocal);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_L4_POROTCAL_VALUE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_l4_protocal_value \n");
			}
		}


		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_PORT_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_port_mask 0x%x\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_port_mask.portmask);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_PORT_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_port_mask \n");
			}
		}


		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_SMAC_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_smac %x:%x:%x:%x:%x:%x\n",
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac.octet[0],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac.octet[1],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac.octet[2],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac.octet[3],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac.octet[4],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac.octet[5]);

			PROC_PRINTF("rg set acl-filter pattern ingress_smac_mask %x:%x:%x:%x:%x:%x\n",
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac_mask.octet[0],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac_mask.octet[1],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac_mask.octet[2],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac_mask.octet[3],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac_mask.octet[4],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_smac_mask.octet[5]);

			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_SMAC_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_smac \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV4_SIP_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_src_ipv4_addr_start %d.%d.%d.%d ingress_src_ipv4_addr_end %d.%d.%d.%d\n",
			(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_start & 0xff000000)>>24,
			(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_start & 0xff0000)>>16,
			(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_start & 0xff00)>>8,
			(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_start & 0xff),
			(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_end & 0xff000000)>>24,
			(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_end & 0xff0000)>>16,
			(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_end & 0xff00)>>8,
			(rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv4_addr_end & 0xff));

			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_IPV4_SIP_RANGE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_src_ipv4_addr_range \n");
			}
		}
		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_SIP_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_src_ipv6_addr %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[0],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[1],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[2],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[3],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[4],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[5],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[6],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[7],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[8],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[9],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[10],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[11],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[12],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[13],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[14],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr[15]);

			PROC_PRINTF("rg set acl-filter pattern ingress_src_ipv6_addr_mask %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[0],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[1],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[2],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[3],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[4],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[5],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[6],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[7],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[8],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[9],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[10],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[11],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[12],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[13],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[14],
					rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_mask[15]);

			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_IPV6_SIP_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_src_ipv6_addr \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_SIP_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_src_ipv6_addr_start %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ingress_src_ipv6_addr_end %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[0],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[1],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[2],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[3],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[4],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[5],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[6],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[7],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[8],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[9],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[10],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[11],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[12],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[13],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[14],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_start[15],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[0],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[1],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[2],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[3],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[4],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[5],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[6],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[7],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[8],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[9],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[10],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[11],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[12],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[13],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[14],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_ipv6_addr_end[15]);

			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_IPV6_SIP_RANGE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_src_ipv6_addr_range \n");
			}
		}


		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_SPORT_RANGE_BIT)!=0)
		{		PROC_PRINTF("rg set acl-filter pattern ingress_src_l4_port_start %d ingress_src_l4_port_end %d\n",
				rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_l4_port_start,
				rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_src_l4_port_end);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_L4_SPORT_RANGE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_src_l4_port_range \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_L4_DPORT_RANGE_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_dest_l4_port_start %d ingress_dest_l4_port_end %d\n",
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_l4_port_start,
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_dest_l4_port_end);

			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_L4_DPORT_RANGE_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_dest_l4_port_range \n");
			}

		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_STAGIF_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_stagIf %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stagIf);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_STAGIF_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_stagIf \n");
			}
		}
		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_CTAGIF_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ctagIf %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ctagIf);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_CTAGIF_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_ctagIf \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_STAG_PRI_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_stag_pri %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stag_pri);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_STAG_PRI_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_stag_pri \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_STAG_VID_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_stag_vid %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stag_vid);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_STAG_VID_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_stag_vid \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_STAG_DEI_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_stag_dei %d set\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stag_dei);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_STAG_DEI_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_stag_dei \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_STREAM_ID_BIT)!=0){
			PROC_PRINTF("rg set acl-filter pattern ingress_stream_id %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stream_id);
			PROC_PRINTF("rg set acl-filter pattern ingress_stream_id_mask 0x%x\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_stream_id_mask);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_STREAM_ID_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_stream_id \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_WLANDEV_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_wlanDevMask 0x%x\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_wlanDevMask);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_WLANDEV_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_wlanDevMask \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_WLANDEV_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_wlanDevMask 0x%x\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_wlanDevMask);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & EGRESS_WLANDEV_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_wlanDevMask \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INTERNAL_PRI_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern internal_pri %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.internal_pri);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INTERNAL_PRI_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not internal_pri \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & INGRESS_IPV6_FLOWLABEL_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern ingress_ipv6_flow_label  %d\n",rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.ingress_ipv6_flow_label);
			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & INGRESS_IPV6_FLOWLABEL_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not ingress_ipv6_flow_label  \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_SMAC_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_smac %x:%x:%x:%x:%x:%x\n",
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_smac.octet[0],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_smac.octet[1],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_smac.octet[2],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_smac.octet[3],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_smac.octet[4],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_smac.octet[5]);

			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & EGRESS_SMAC_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_smac \n");
			}
		}

		if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields & EGRESS_DMAC_BIT)!=0)
		{
			PROC_PRINTF("rg set acl-filter pattern egress_dmac %x:%x:%x:%x:%x:%x\n",
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dmac.octet[0],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dmac.octet[1],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dmac.octet[2],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dmac.octet[3],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dmac.octet[4],
			rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.egress_dmac.octet[5]);

			if((rt_edp_db.systemGlobal.acl_SW_table_entry[i].acl_filter.filter_fields_inverse & EGRESS_DMAC_BIT)!=0)
			{
				PROC_PRINTF("rg set acl-filter pattern not egress_dmac \n");
			}
		}


		PROC_PRINTF("rg add acl-filter entry\n");
		}
	}


	PROC_PRINTF("#add classify-filter\n");	//action(*), entry and pattern(*)
	PROC_PRINTF("Under development...\n");
#if 0
	for(i=0;i<TOTAL_CF_ENTRY_SIZE;i++){
		if(rt_edp_db.systemGlobal.classify_SW_table_entry[i].filter_fields!=0 ||
			rt_edp_db.systemGlobal.classify_SW_table_entry[i].ds_action_field!=0 ||
			rt_edp_db.systemGlobal.classify_SW_table_entry[i].us_action_field!=0
			 ){
		//uint8 setFlag=0;
		//char tmpStr[255];
		//memset(tmpStr,0,255);
		PROC_PRINTF("rg clear classify-filter\n");

		if(rt_edp_db.systemGlobal.classify_SW_table_entry[i].ds_action_field!=0x0){

			if((rt_edp_db.systemGlobal.classify_SW_table_entry[i].ds_action_field & CF_DS_ACTION_STAG_BIT)!=0){

				//init varible
				unknownCmdFlag=0;
				tagDecision=-1;
				vidDecision=-1;
				priDecision=-1;

				switch(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_svlan.svlanTagIfDecision)
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

				switch(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_svlan.svlanSvidDecision)
				{
					case ACL_SVLAN_SVID_ASSIGN: vidDecision=1; break;
					case ACL_SVLAN_SVID_COPY_FROM_1ST_TAG: vidDecision=2; break;
					case ACL_SVLAN_SVID_COPY_FROM_2ND_TAG: vidDecision=3; break;
					case ACL_SVLAN_SVID_NOP: vidDecision=0; break;
					default:
						unknownCmdFlag =1;
						break;
				}
				switch(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_svlan.svlanSpriDecision)
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
						rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_svlan.assignedSvid,
						rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_svlan.assignedSpri);
				}

			}
			if((rt_edp_db.systemGlobal.classify_SW_table_entry[i].ds_action_field & CF_DS_ACTION_CTAG_BIT)!=0){
				switch(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.cvlanTagIfDecision)
				{
					case ACL_CVLAN_TAGIF_NOP: tagDecision=0; break;
					case ACL_CVLAN_TAGIF_TAGGING: tagDecision=1; break;
					case ACL_CVLAN_TAGIF_UNTAG: tagDecision=2; break;
					case ACL_CVLAN_TAGIF_TRANSPARENT: tagDecision=3; break;
					default:
						unknownCmdFlag =1;
						break;
				}

				switch(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.cvlanCvidDecision)
				{
					case ACL_CVLAN_CVID_ASSIGN: vidDecision=1; break;
					case ACL_CVLAN_CVID_COPY_FROM_1ST_TAG: vidDecision=2; break;
					case ACL_CVLAN_CVID_COPY_FROM_2ND_TAG: vidDecision=3; break;
					case ACL_CVLAN_CVID_NOP: vidDecision=0; break;
					default:
						unknownCmdFlag =1;
						break;
				}

				switch(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.cvlanCpriDecision)
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
						rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.assignedCvid,
						rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.assignedCpri);
				}

			}

			if((rt_edp_db.systemGlobal.classify_SW_table_entry[i].ds_action_field & CF_DS_ACTION_CFPRI_BIT)!=0){
				if(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_cfpri.cfPriDecision==ACL_CFPRI_ASSIGN)
					PROC_PRINTF("rg set classify-filter action direction downstream action_cfpri cfpriDecision 0 cfpri %d\n",rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_cfpri.assignedCfPri);
				else if(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_cfpri.cfPriDecision==ACL_CFPRI_NOP)
					PROC_PRINTF("rg set classify-filter action direction downstream action_cfpri cfpriDecision 1 cfpri 0\n");
				else
					PROC_PRINTF("rg set classify-filter action direction downstream action_cfpri [unknown cfpriDecision]\n");
			}
			if((rt_edp_db.systemGlobal.classify_SW_table_entry[i].ds_action_field & CF_DS_ACTION_DSCP_BIT)!=0){
				if(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_dscp.dscpDecision==ACL_DSCP_ASSIGN)
					PROC_PRINTF("rg set classify-filter action direction downstream action_dscp dscpDecision 0 dscp %d\n",rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_dscp.assignedDscp);
				else if(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_dscp.dscpDecision==ACL_DSCP_NOP)
					PROC_PRINTF("rg set classify-filter action direction downstream action_dscp dscpDecision 1 dscp 0\n");
				else
					PROC_PRINTF("rg set classify-filter action direction downstream action_dscp [unknown dscpDecision]\n");

			}


			if((rt_edp_db.systemGlobal.classify_SW_table_entry[i].ds_action_field & CF_DS_ACTION_UNI_MASK_BIT)!=0){
				if(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_uni.uniActionDecision==ACL_UNI_FWD_TO_PORTMASK_ONLY){
					PROC_PRINTF("rg set classify-filter action direction downstream action_uni uniDecision 0 portmask 0x%x\n",
					rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_uni.assignedUniPortMask);
				}
				else if(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_uni.uniActionDecision==ACL_UNI_FORCE_BY_MASK){
					PROC_PRINTF("rg set classify-filter action direction downstream action_uni uniDecision 1 portmask 0x%x\n",
					rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_uni.assignedUniPortMask);
				}
				else if(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_uni.uniActionDecision==ACL_UNI_TRAP_TO_CPU){
					PROC_PRINTF("rg set classify-filter action direction downstream action_uni uniDecision 2 portmask 0x0\n");
				}
				else if(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_uni.uniActionDecision==AL_UNI_NOP){
					PROC_PRINTF("rg set classify-filter action direction downstream action_uni uniDecision 3 portmask 0x0\n");
				}
				else{
					PROC_PRINTF("rg set classify-filter action direction downstream action_uni [unknown uniDecision]\n");
				}
			}

		}

		//upstreaming
		if(rt_edp_db.systemGlobal.classify_SW_table_entry[i].us_action_field != 0){

			if((rt_edp_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_STAG_BIT)!=0)
			{

				//init varible
				unknownCmdFlag=0;
				tagDecision=-1;
				vidDecision=-1;
				priDecision=-1;

				switch(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_svlan.svlanTagIfDecision)
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

				switch(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_svlan.svlanSvidDecision)
				{
					case ACL_SVLAN_SVID_ASSIGN: vidDecision=1; break;
					case ACL_SVLAN_SVID_COPY_FROM_1ST_TAG: vidDecision=2; break;
					case ACL_SVLAN_SVID_COPY_FROM_2ND_TAG: vidDecision=3; break;
					case ACL_SVLAN_SVID_NOP: vidDecision=0; break;
					default:
						unknownCmdFlag =1;
						break;
				}
				switch(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_svlan.svlanSpriDecision)
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
						rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_svlan.assignedSvid,
						rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_svlan.assignedSpri);
				}

			}

			if((rt_edp_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_CTAG_BIT)!=0)
			{
				switch(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.cvlanTagIfDecision)
				{
					case ACL_CVLAN_TAGIF_NOP: tagDecision=0; break;
					case ACL_CVLAN_TAGIF_TAGGING: tagDecision=1; break;
					case ACL_CVLAN_TAGIF_UNTAG: tagDecision=2; break;
					case ACL_CVLAN_TAGIF_TRANSPARENT: tagDecision=3; break;
					default:
						unknownCmdFlag =1;
						break;
				}

				switch(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.cvlanCvidDecision)
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

				switch(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.cvlanCpriDecision)
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
						rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.assignedCvid,
						rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_cvlan.assignedCpri);
				}

			}

/*
			//dop is supported by action_fwd in apolloFE
			if((rt_edp_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_DROP_BIT)!=0)
				PROC_PRINTF("rg set classify-filter action direction upstream action_drop\n");
*/
			if((rt_edp_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_FWD_BIT)!=0)
			{
				PROC_PRINTF("CF_US_ACTION_FWD_BIT");
				if(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_fwd.fwdDecision==ACL_FWD_NOP){
					PROC_PRINTF("rg set classify-filter action direction upstream action_fwd fwdDecision 0\n");
				}
				else if(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_fwd.fwdDecision==ACL_FWD_DROP){
					PROC_PRINTF("rg set classify-filter action direction upstream action_fwd fwdDecision 1\n");
				}
				else if(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_fwd.fwdDecision==ACL_FWD_TRAP_TO_CPU){
					PROC_PRINTF("rg set classify-filter action direction upstream action_fwd fwdDecision 2\n");
				}
				else if(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_fwd.fwdDecision==ACL_FWD_DROP_TO_PON){
					PROC_PRINTF("rg set classify-filter action direction upstream action_fwd fwdDecision 3\n");
				}
				else{
					PROC_PRINTF("rg set classify-filter action direction upstream action_fwd [unknown fwdDecision]\n");
				}
			}



			if((rt_edp_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_CFPRI_BIT)!=0)
			{
				if(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_cfpri.cfPriDecision==ACL_CFPRI_ASSIGN)
					PROC_PRINTF("rg set classify-filter action direction upstream action_cfpri cfpriDecision 0 cfpri %d\n",rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_cfpri.assignedCfPri);
				else if(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_cfpri.cfPriDecision==ACL_CFPRI_NOP)
					PROC_PRINTF("rg set classify-filter action direction upstream action_cfpri cfpriDecision 1 cfpri 0\n");
				else
					PROC_PRINTF("rg set classify-filter action direction upstream action_cfpri [unknown cfpriDecision]\n");
			}


			if((rt_edp_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_DSCP_BIT)!=0)
			{
				if(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_dscp.dscpDecision==ACL_DSCP_ASSIGN)
					PROC_PRINTF("rg set classify-filter action direction upstream action_dscp dscpDecision 0 dscp %d\n",rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_dscp.assignedDscp);
				else if(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_dscp.dscpDecision==ACL_DSCP_NOP)
					PROC_PRINTF("rg set classify-filter action direction upstream action_dscp dscpDecision 1 dscp 0\n");
				else
					PROC_PRINTF("rg set classify-filter action direction upstream action_dscp [unknown dscpDecision]\n");
			}

			if((rt_edp_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_SID_BIT)!=0)
			{
				if(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_sid_or_llid.sidDecision==ACL_SID_LLID_ASSIGN)
					PROC_PRINTF("rg set classify-filter action direction upstream action_sid sidDecision 0 sid %d\n",rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_sid_or_llid.assignedSid_or_llid);
				else if(rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_sid_or_llid.sidDecision==ACL_SID_LLID_NOP)
					PROC_PRINTF("rg set classify-filter action direction upstream action_sid sidDecision 0 sid 0\n");
				else
					PROC_PRINTF("rg set classify-filter action direction upstream action_sid [unknown sidDecision]\n");
			}
/*
			if((rt_edp_db.systemGlobal.classify_SW_table_entry[i].us_action_field & CF_US_ACTION_LOG_BIT)!=0)
				PROC_PRINTF("rg set classify-filter action direction upstream action_log %d\n",rt_edp_db.systemGlobal.classify_SW_table_entry[i].action_log.assignedCounterIdx);
*/
		}

		if((rt_edp_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_CTAGIF_BIT)!=0)
			PROC_PRINTF("rg set classify-filter pattern ctagIf %d\n",rt_edp_db.systemGlobal.classify_SW_table_entry[i].ctagIf);

		if((rt_edp_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_ETHERTYPR_BIT)!=0){
			PROC_PRINTF("rg set classify-filter pattern etherType 0x%x\n",rt_edp_db.systemGlobal.classify_SW_table_entry[i].etherType);
			PROC_PRINTF("rg set classify-filter pattern etherType_mask 0x%x\n",rt_edp_db.systemGlobal.classify_SW_table_entry[i].etherType_mask);
		}

		if((rt_edp_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_GEMIDX_BIT)!=0){
			PROC_PRINTF("rg set classify-filter pattern gemidx %d\n",rt_edp_db.systemGlobal.classify_SW_table_entry[i].gemidx);
			PROC_PRINTF("rg set classify-filter pattern gemidx_mask 0x%x\n",rt_edp_db.systemGlobal.classify_SW_table_entry[i].gemidx_mask);
		}

		if((rt_edp_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_INTERNALPRI_BIT)!=0)
			PROC_PRINTF("rg set classify-filter pattern internalPri %d\n",rt_edp_db.systemGlobal.classify_SW_table_entry[i].internalPri);

		if((rt_edp_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_LLID_BIT)!=0)
			PROC_PRINTF("rg set classify-filter pattern llid %d\n",rt_edp_db.systemGlobal.classify_SW_table_entry[i].llid);

		if((rt_edp_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_TAGPRI_BIT)!=0)
			PROC_PRINTF("rg set classify-filter pattern outterTagPri %d\n",rt_edp_db.systemGlobal.classify_SW_table_entry[i].outterTagPri);

		if((rt_edp_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_TAGVID_BIT)!=0)
			PROC_PRINTF("rg set classify-filter pattern outterTagVid %d\n",rt_edp_db.systemGlobal.classify_SW_table_entry[i].outterTagVid);

		if((rt_edp_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_STAGIF_BIT)!=0)
			PROC_PRINTF("rg set classify-filter pattern stagIf %d\n",rt_edp_db.systemGlobal.classify_SW_table_entry[i].stagIf);

		if((rt_edp_db.systemGlobal.classify_SW_table_entry[i].filter_fields & EGRESS_UNI_BIT)!=0){
			PROC_PRINTF("rg set classify-filter pattern uni %d\n",rt_edp_db.systemGlobal.classify_SW_table_entry[i].uni);
			PROC_PRINTF("rg set classify-filter pattern uni_mask 0x%x\n",rt_edp_db.systemGlobal.classify_SW_table_entry[i].uni_mask);
		}
			PROC_PRINTF("rg set classify-filter entry %d\n",i);
			PROC_PRINTF("rg add classify-filter entry\n");
		}
	}

#endif

	return RT_EDP_ERR_OK;
}

#if 0
int _dump_acl(struct seq_file *s)
{
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

	return RT_EDP_ERR_OK;
}

#endif


//internal APIs
int _dump_rt_edp_acl_entry_content(struct seq_file *s, rtk_rg_aclFilterAndQos_t *aclPara)
{
	rtk_rg_aclFilterAndQos_t *acl_parameter;
	uint8   ipv4_addr[4*sizeof "123"];

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
		PROC_PRINTF(" %s ingress_sip_low_bound: %s ",(aclPara->filter_fields_inverse&INGRESS_IPV4_SIP_RANGE_BIT)?"not":"", _rt_edp_inet_ntoa_p(acl_parameter->ingress_src_ipv4_addr_start, ipv4_addr));
		PROC_PRINTF(" ingress_sip_up_bound: %s \n", _rt_edp_inet_ntoa_p(acl_parameter->ingress_src_ipv4_addr_end, ipv4_addr));
	}
	if(aclPara->filter_fields&INGRESS_IPV4_DIP_RANGE_BIT){
		PROC_PRINTF(" %s ingress_dip_low_bound: %s ",(aclPara->filter_fields_inverse&INGRESS_IPV4_DIP_RANGE_BIT)?"not":"", _rt_edp_inet_ntoa_p(acl_parameter->ingress_dest_ipv4_addr_start, ipv4_addr));
		PROC_PRINTF(" ingress_dip_up_bound: %s \n", _rt_edp_inet_ntoa_p(acl_parameter->ingress_dest_ipv4_addr_end, ipv4_addr));
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
		PROC_PRINTF(" %s egress_sip_low_bound: %s	", (aclPara->filter_fields_inverse&EGRESS_IPV4_SIP_RANGE_BIT)?"not":"", _rt_edp_inet_ntoa_p(acl_parameter->egress_src_ipv4_addr_start, ipv4_addr));
		PROC_PRINTF(" egress_sip_up_bound:	%s \n", _rt_edp_inet_ntoa_p(acl_parameter->egress_src_ipv4_addr_end, ipv4_addr));
	}
	if(aclPara->filter_fields&EGRESS_IPV4_DIP_RANGE_BIT){
		PROC_PRINTF(" %s egress_dip_low_bound: %s	", (aclPara->filter_fields_inverse&EGRESS_IPV4_DIP_RANGE_BIT)?"not":"", _rt_edp_inet_ntoa_p(acl_parameter->egress_dest_ipv4_addr_start, ipv4_addr));
		PROC_PRINTF(" egress_dip_up_bound:	%s \n", _rt_edp_inet_ntoa_p(acl_parameter->egress_dest_ipv4_addr_end, ipv4_addr));
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
			if(acl_parameter->qos_actions&ACL_ACTION_REDIRECT_BIT) PROC_PRINTF(" redirect to portmaks: 0x%x\n",acl_parameter->redirect_portmask);
			if(acl_parameter->qos_actions&ACL_ACTION_ACL_CVLANTAG_BIT)
			{
				PROC_PRINTF(" cvlan: %s\n  cvid_act:%s\n  cpri_act:%s\n  cvid:%d\n  cpri:%d\n",
					name_of_rt_edp_cvlan_tagif_decision[acl_parameter->action_acl_cvlan.cvlanTagIfDecision],
					name_of_rt_edp_cvlan_cvid_decision[acl_parameter->action_acl_cvlan.cvlanCvidDecision],
					name_of_rt_edp_cvlan_cpri_decision[acl_parameter->action_acl_cvlan.cvlanCpriDecision],
					acl_parameter->action_acl_cvlan.assignedCvid,
					acl_parameter->action_acl_cvlan.assignedCpri);
			}
			if(acl_parameter->qos_actions&ACL_ACTION_ACL_SVLANTAG_BIT)
			{
				PROC_PRINTF(" svlan: %s\n  svid_act:%s\n  spri_act:%s\n  svid:%d\n  spri:%d\n",
					name_of_rt_edp_svlan_tagif_decision[acl_parameter->action_acl_svlan.svlanTagIfDecision],
					name_of_rt_edp_svlan_svid_decision[acl_parameter->action_acl_svlan.svlanSvidDecision],
					name_of_rt_edp_svlan_spri_decision[acl_parameter->action_acl_svlan.svlanSpriDecision],
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
			PROC_PRINTF(" encap_dport:%d\n",acl_parameter->action_encap_udp.encap_Dport);
			break;

		case ACL_ACTION_TYPE_FLOW_MIB:
			PROC_PRINTF("action type: ACL_ACTION_TYPE_FLOW_MIB \n");
			PROC_PRINTF("flowmib_counter_index: %d\n",acl_parameter->action_flowmib_counter_idx);
			break;

		case ACL_ACTION_TYPE_TRAP_WITH_PRIORITY:
			PROC_PRINTF("action type: ACL_ACTION_TYPE_TRAP_WITH_PRIORITY \n");
			PROC_PRINTF("acl_priority: %d\n",acl_parameter->action_trap_with_priority);
			break;

		default:
			break;
	}

	return 0;
}


static int _rt_edp_aclSWEntry_not_support(rtk_rg_aclFilterAndQos_t *acl_filter)
{

	//EDP not support
	if(acl_filter->filter_fields&INGRESS_STAG_DEI_BIT) {
		WARNING("EDP not support pattern: INGRESS_STAG_DEI_BIT");
		RT_EDP_RETURN_ERR(RT_EDP_ERR_ACL_NOT_SUPPORT);
	}else if(acl_filter->filter_fields&INGRESS_CTAG_CFI_BIT)
	{
		WARNING("EDP not support pattern: INGRESS_CTAG_CFI_BIT");
		RT_EDP_RETURN_ERR(RT_EDP_ERR_ACL_NOT_SUPPORT);
	}


	//SVLAN action not support feature
	if((acl_filter->action_type==ACL_ACTION_TYPE_QOS) && (acl_filter->qos_actions & ACL_ACTION_ACL_SVLANTAG_BIT))
	{
		if(acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_8100 ||acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C)
			RT_EDP_RETURN_ERR(RT_EDP_ERR_ACL_NOT_SUPPORT);

		if(acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2 ||acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_ORIGINAL_STAG_TPID)
			RT_EDP_RETURN_ERR(RT_EDP_ERR_ACL_NOT_SUPPORT);

		if(acl_filter->action_acl_svlan.svlanSvidDecision==ACL_SVLAN_SVID_SP2C)
			RT_EDP_RETURN_ERR(RT_EDP_ERR_ACL_NOT_SUPPORT);

		if(acl_filter->action_acl_svlan.svlanSpriDecision==ACL_SVLAN_SPRI_COPY_FROM_DSCP_REMAP || acl_filter->action_acl_svlan.svlanSpriDecision==ACL_SVLAN_SPRI_COPY_FROM_SP2C)
			RT_EDP_RETURN_ERR(RT_EDP_ERR_ACL_NOT_SUPPORT);

		if(!((acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN) ||
			(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN)))
			RT_EDP_RETURN_ERR(RT_EDP_ERR_ACL_NOT_SUPPORT);
	}

	//CVLAN action not support feature
	if((acl_filter->action_type==ACL_ACTION_TYPE_QOS) && (acl_filter->qos_actions & ACL_ACTION_ACL_CVLANTAG_BIT))
	{
		if(acl_filter->action_acl_cvlan.cvlanTagIfDecision==ACL_CVLAN_TAGIF_TAGGING_WITH_C2S || acl_filter->action_acl_cvlan.cvlanTagIfDecision==ACL_CVLAN_TAGIF_TAGGING_WITH_SP2C)
			RT_EDP_RETURN_ERR(RT_EDP_ERR_ACL_NOT_SUPPORT);

		if(acl_filter->action_acl_cvlan.cvlanCvidDecision==ACL_CVLAN_CVID_CPOY_FROM_SP2C || acl_filter->action_acl_cvlan.cvlanCvidDecision==ACL_CVLAN_CVID_CPOY_FROM_DMAC2CVID)
			RT_EDP_RETURN_ERR(RT_EDP_ERR_ACL_NOT_SUPPORT);

		if(acl_filter->action_acl_cvlan.cvlanCpriDecision==ACL_CVLAN_CPRI_COPY_FROM_DSCP_REMAP || acl_filter->action_acl_cvlan.cvlanCpriDecision==ACL_CVLAN_CPRI_COPY_FROM_SP2C)
			RT_EDP_RETURN_ERR(RT_EDP_ERR_ACL_NOT_SUPPORT);

		if(!((acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN) ||
			(acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN)))
			RT_EDP_RETURN_ERR(RT_EDP_ERR_ACL_NOT_SUPPORT);
	}

	//FB not support uni action to multi-port
	if((acl_filter->qos_actions & ACL_ACTION_DS_UNIMASK_BIT))
	{
		if(acl_filter->downstream_uni_portmask!=0x0)
		{
			WARNING("EDP not support ACL_ACTION_DS_UNIMASK_BIT action to redirect or to multi-port");
			RT_EDP_RETURN_ERR(RT_EDP_ERR_ACL_NOT_SUPPORT);
		}
	}

	//FB not support egress to multi-port action (path5 can only forward to single port)
	if((acl_filter->qos_actions & ACL_ACTION_REDIRECT_BIT))
	{
			WARNING("EDP not support ction ACL_ACTION_REDIRECT_BIT and ACL_ACTION_DS_UNIMASK_BIT");
			RT_EDP_RETURN_ERR(RT_EDP_ERR_ACL_NOT_SUPPORT);
	}

	//FB not support share meter index 32~47
	if((acl_filter->qos_actions & ACL_ACTION_SHARE_METER_BIT))
	{
		if(acl_filter->action_share_meter >= PURE_SW_METER_IDX_OFFSET+PURE_SW_SHAREMETER_TABLE_SIZE)
		{
			WARNING("EDP ACL action share meter must less than %d", PURE_SW_METER_IDX_OFFSET+PURE_SW_SHAREMETER_TABLE_SIZE);
			RT_EDP_RETURN_ERR(RT_EDP_ERR_ACL_NOT_SUPPORT);
		}
		else if((acl_filter->action_share_meter >= FLOWBASED_TABLESIZE_SHAREMTR) && (acl_filter->action_share_meter < PURE_SW_METER_IDX_OFFSET))
		{
			WARNING("EDP ACL action not support share meter index from %d to %d", FLOWBASED_TABLESIZE_SHAREMTR, PURE_SW_METER_IDX_OFFSET-1);
			RT_EDP_RETURN_ERR(RT_EDP_ERR_ACL_NOT_SUPPORT);
		}
	}

#if 0
	//FB not support patterns (even flow is different, it will be hash into the same way)
	if(rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT0)
	{
		if(acl_filter->filter_fields & (INGRESS_CTAG_PRI_BIT|INGRESS_STAG_PRI_BIT))
		{
			//WARNING("EDP not support pattern (INGRESS_CTAG_PRI_BIT|INGRESS_STAG_PRI_BIT), due to FB will hash in the same way.");
			//RT_EDP_RETURN_ERR(RT_EDP_ERR_ACL_NOT_SUPPORT);

			//20170515:tysu suggest let customer can added it for one flow support. different cpri consider as the same flow case should not happened!
			WARNING("[Customer Aware!!!]INGRESS_CTAG_PRI_BIT/INGRESS_STAG_PRI_BIT is not support with flow hash in 9607C testChip, the rule can be added but all cpri/spri is considered as the same flow(cpri/spri 0~7 do the same action).");
		}

		if(acl_filter->filter_fields & (INGRESS_CTAG_CFI_BIT|INGRESS_STAG_DEI_BIT))
		{
			WARNING("EDP pattern (INGRESS_CTAG_CFI_BIT|INGRESS_STAG_DEI_BIT) only filter for first slow path packet, suppose smae flow will not have different dei/cfi.");
		}
	}
	else
	{
		if(acl_filter->filter_fields & (INGRESS_CTAG_CFI_BIT|INGRESS_STAG_DEI_BIT|INGRESS_STAG_PRI_BIT))
		{
			WARNING("EDP pattern (INGRESS_CTAG_CFI_BIT|INGRESS_STAG_DEI_BIT|INGRESS_STAG_PRI_BIT) only filter for first slow path packet, suppose smae flow will not have different dei/cfi.");
		}
	}
#endif


	//SP2C/C2S actions is not belong to tagDecision layer in apolloFE, it should moved to vidDecision layer.
	if(acl_filter->action_acl_svlan.svlanTagIfDecision==ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C)
	{
		WARNING("SP2C action should be set at svlanSvidDecision layer");
		RT_EDP_RETURN_ERR(RT_EDP_ERR_CHIP_NOT_SUPPORT);
	}
	if(acl_filter->action_acl_cvlan.cvlanTagIfDecision==ACL_CVLAN_TAGIF_TAGGING_WITH_C2S || acl_filter->action_acl_cvlan.cvlanTagIfDecision==ACL_CVLAN_TAGIF_TAGGING_WITH_SP2C)
	{
		WARNING("SP2C/C2S action should be set at cvlanCvidDecision layer");
		RT_EDP_RETURN_ERR(RT_EDP_ERR_CHIP_NOT_SUPPORT);
	}

	if(acl_filter->qos_actions & ACL_ACTION_QUEUE_ID_BIT)
	{
		WARNING("ACL_ACTION_QUEUE_ID_BIT is not support, please use ACL_ACTION_ACL_PRIORITY_BIT");
		RT_EDP_RETURN_ERR(RT_EDP_ERR_CHIP_NOT_SUPPORT);
	}

	if(acl_filter->qos_actions & ACL_ACTION_LOG_COUNTER_BIT)
	{
		WARNING("ACL_ACTION_LOG_COUNTER_BIT is not support, please use FB counter");
		RT_EDP_RETURN_ERR(RT_EDP_ERR_CHIP_NOT_SUPPORT);
	}


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

	//acl-priroity is decided when ingress, not support with egress pattern
	if(acl_filter->qos_actions & ACL_ACTION_ACL_PRIORITY_BIT)
	{
		if((acl_filter->fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET) && (acl_filter->filter_fields & EGRESS_INTF_BIT))
		{
			//Support this pattern to configure acl priority action
		}
		else if((acl_filter->filter_fields & EGRESS_ACL_PATTERN_BITS)!=0x0)
		{
			WARNING("ACL_ACTION_ACL_PRIORITY_BIT decision is in ingress, rule can not support such action with egress pattern!");
			RT_EDP_RETURN_ERR(RT_EDP_ERR_CHIP_NOT_SUPPORT);
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
			RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);
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
			RT_EDP_RETURN_ERR(RT_EDP_ERR_INVALID_PARAM);
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
		acl_filter->action_type==ACL_ACTION_TYPE_FLOW_MIB
		)
	{
		//action can be support.
#if 0
		if((acl_filter->action_type==ACL_ACTION_TYPE_FLOW_MIB) && (rg_db.systemGlobal.internalSupportMask&RTK_RG_INTERNAL_SUPPORT_BIT0 /*test chip*/) && (acl_filter->action_flowmib_counter_idx < MAX_FLOWMIB_TABLE_SIZE)) {
			WARNING("EDP only support sw flow mib range %d to %d", MAX_FLOWMIB_TABLE_SIZE, MAX_FLOWMIB_TABLE_SIZE+PURE_SW_FLOWMIB_TABLE_SIZE-1);
			RETURN_ERR(RT_EDP_ERR_CHIP_NOT_SUPPORT);
		}
#endif
	}
	else
	{
		WARNING("ACL action_type is not suported by this platform!");
		RT_EDP_RETURN_ERR(RT_EDP_ERR_CHIP_NOT_SUPPORT);
	}


	return (RT_EDP_ERR_OK);
}

