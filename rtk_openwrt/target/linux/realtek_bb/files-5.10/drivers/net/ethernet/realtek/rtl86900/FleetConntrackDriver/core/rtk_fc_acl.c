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

#include "rtk_fc_acl.h"
#include "rtk_fc_internal.h"
#include "rtk_fc_callback.h"
#include "rtk_fc_debug.h"
#include "rtk_fc_external.h"

#ifdef CONFIG_RTK_L34_XPON_PLATFORM
#include "rtk_fc_mappingAPI.h"
#include <rtk/acl.h>
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
#include <aal_l2_cls.h>		//for aal_l2_cls_add
#include <aal_l3_cls.h>		//for aal_l3_cls_add
#include <aal_l3_cam.h>		//for l3_cam_xxx
#include <aal_l3_specpkt.h>
#endif
#if defined(CONFIG_FC_RTL9607C_SERIES)
extern uint32 dynamic_sram_desc;
#endif

int _rtk_fc_acl_asic_init(void);
int _rtk_fc_aclSWEntry_init(void);
static int _rtk_fc_aclHWEntry_not_support(rt_acl_filterAndQos_t *acl_filter);
static int _rtk_fc_aclSWEntry_to_asic_add(rt_acl_filterAndQos_t *acl_filter,rtk_fc_aclFilterEntry_t* aclSWEntry,int shareHwAclWithSWAclIdx);
static int _rtk_fc_reset_asic_TableEntry(void);
void _rtk_fc_acl_for_multicast_temp_protection_add(void);
void _rtk_fc_acl_for_multicast_temp_protection_del(void);
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
int _rtk_fc_aclReservedEntry_init(void);
#endif

#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
static int _rtk_fc_aclHWEntry_meter_remapping(void);
#endif


/*(1)ACL init related APIs*/
int rtk_fc_aclInit(void)
{
	rtk_fc_aclAndCf_reserved_assign_priority_t rsv_acl_assign_priority;

	//Init ACL Template & Field Selector & RangeTable Value
	assert_ok(_rtk_fc_acl_asic_init());

	//init RT API ACL for SW maintain info
	assert_ok(_rtk_fc_aclSWEntry_init());

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	//init reserved ACL SW maintain info
	assert_ok(_rtk_fc_aclReservedEntry_init());
#endif

	/*reserved ACL*/
	{
		if(fc_db.controlFuc.rt_api_is_enabled){
			fc_db.aclAndCfReservedRule.reservedMap[RT_ACL_RESERVED_TYPE_ICMPV4_TRAP] = RTK_FC_ACLANDCF_RESERVED_ICMPV4_TRAP;
			fc_db.aclAndCfReservedRule.reservedMap[RT_ACL_RESERVED_TYPE_ICMPV6_TRAP] = RTK_FC_ACLANDCF_RESERVED_ICMPV6_TRAP;
		}

#if defined(CONFIG_FC_RTL8277C_SERIES) && defined(CONFIG_RTK_NIC_HWLOOKUP_SNAP_TRANSFORM_BY_HW)
		assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_WIFI_SNAP_TRANS, NULL));
#endif
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
		assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_WIFI_TXAMSDU_HW_LOOKUP, NULL));
		assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_WIFI_TXAMSDU_MC_TO_UC, NULL));
#endif
		assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP, NULL));

		assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT, NULL));

		assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_ICMPV4_TRAP, NULL));
		assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_ICMPV6_TRAP, NULL));

		assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP, NULL));

		bzero(&rsv_acl_assign_priority,sizeof(rsv_acl_assign_priority));
		rsv_acl_assign_priority.priority=7;

		assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY, &rsv_acl_assign_priority));
		assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY, &rsv_acl_assign_priority));
		assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_PPTP_GRE_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY, &rsv_acl_assign_priority));
	}


	return (RTK_FC_RET_OK);
}


int _rtk_fc_aclSWEntry_init(void)
{
	int j;

	for(j=0;j<MAX_ACL_SW_ENTRY_SIZE;j++){
		fc_db.acl_SWindex_sorting_by_weight[j]=-1;

		/*Skip ----------------- init acl_SWindex_sorting_by_weight_and_ingress_cvid_action for ingress_vid ----------------- Skip*/
	}

    bzero(fc_db.acl_SW_table_entry, sizeof(rtk_fc_aclFilterEntry_t)*MAX_ACL_SW_ENTRY_SIZE);
    bzero(fc_db.acl_filter_temp, sizeof(rt_acl_filterAndQos_t)*MAX_ACL_SW_ENTRY_SIZE);
	bzero(fc_db.acl_filter_temp_valid, sizeof(fc_db.acl_filter_temp_valid));
	fc_db.acl_SW_table_entry_size=0;



    return (RT_ERR_RG_OK);
}

/*(2)FC_ACL APIs and internal APIs*/
int _rtk_fc_aclSWEntry_get(int index, rtk_fc_aclFilterEntry_t* aclSWEntry)
{
    *aclSWEntry = fc_db.acl_SW_table_entry[index];
    return (RT_ERR_RG_OK);

}

int _rtk_fc_aclSWEntry_set(int index, rtk_fc_aclFilterEntry_t *aclSWEntry)
{
	if(index<0 || index >= MAX_ACL_SW_ENTRY_SIZE){
		WARNING("rule index parameter is invalid! access RG_ACL[%d] rule fail.",index);
		return (RT_ERR_RG_OK);
	}

    memcpy(&fc_db.acl_SW_table_entry[index], aclSWEntry, sizeof(rtk_fc_aclFilterEntry_t));
    return (RT_ERR_RG_OK);
}

// rtk_fc_aclFilterEntry_t aclSWEntry, empty_aclSWEntry;
int _rtk_fc_aclSWEntry_empty_find(int* index)
{

    int i;
    bzero(&fc_db.aclSWEntry_temp, sizeof(fc_db.aclSWEntry_temp));
    bzero(&fc_db.empty_aclSWEntry, sizeof(fc_db.empty_aclSWEntry));


    for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
    {
    	if(fc_db.acl_SW_table_entry[i].valid==ENABLED)
			continue;

        if(_rtk_fc_aclSWEntry_get(i,&fc_db.aclSWEntry_temp))
            return (RT_ERR_RG_ACL_SW_ENTRY_ACCESS_FAILED);

        if(!memcmp(&fc_db.aclSWEntry_temp,&fc_db.empty_aclSWEntry,sizeof(rtk_fc_aclFilterEntry_t)))
        {
            *index = i;
            break;
        }
	}
    //not found empty entry
    if(i==(MAX_ACL_SW_ENTRY_SIZE-1))
        return (RT_ERR_RG_ACL_SW_ENTRY_FULL);

    return (RT_ERR_RG_OK);

}

//rtk_fc_aclFilterEntry_t aclSWEntry, empty_aclSWEntry;
int _rtk_fc_aclSWEntry_dump(void)
{
    int i;

    bzero(&fc_db.aclSWEntry,sizeof(fc_db.aclSWEntry));
    bzero(&fc_db.empty_aclSWEntry,sizeof(fc_db.empty_aclSWEntry));


  	DEBUG("dump aclSWEntry");
    for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
    {
        _rtk_fc_aclSWEntry_get(i,&fc_db.aclSWEntry);
        if(memcmp(&fc_db.aclSWEntry,&fc_db.empty_aclSWEntry,sizeof(rtk_fc_aclFilterEntry_t)))
    	{
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
			DEBUG("aclSWEntry[%d]: aclindex=%s port=0x%x aclcount=%d\n",i,fc_db.aclSWEntry.hw_aclEntry_index,fc_db.aclSWEntry.hw_aclEntry_port.bits[0],fc_db.aclSWEntry.hw_aclEntry_count);
			ACL_CTRL("aclSWEntry[%d]: aclindex=%s port=0x%x aclcount=%d\n",i,fc_db.aclSWEntry.hw_aclEntry_index,fc_db.aclSWEntry.hw_aclEntry_port.bits[0],fc_db.aclSWEntry.hw_aclEntry_count);
#else
            DEBUG("aclSWEntry[%d]: aclstart=%d aclsize=%d\n",i,fc_db.aclSWEntry.hw_aclEntry_start,fc_db.aclSWEntry.hw_aclEntry_size);
#endif
    	}
    }
    return (RT_ERR_RG_OK);
}

static int _rtk_fc_aclSWEntry_rearrange_ACL_weight(int *accumulateIdx)
{
	int i,j;
	int temp,sp;

	//1-1. clean the sorting record of type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
	*accumulateIdx = 0;
	memset(fc_db.acl_SWindex_sorting_by_weight, -1, sizeof(fc_db.acl_SWindex_sorting_by_weight));
	/*Skip ----------------- reset acl_SWindex_sorting_by_weight_and_ingress_cvid_action for ingress_vid ----------------- Skip*/

	//1-2. record the rule which type is ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
	for(i=0, j=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
	{
		if((fc_db.acl_filter_temp_valid[(i>>5)]&(1<<(i&0x1f))) == 0x0)
			continue;
		else
		{
			/*Skip ----------------- fwding_type_and_direction==ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET ----------------- Skip*/
			//ASSERT_EQ(_rtk_fc_aclSWEntry_reAdd(&fc_db.acl_filter_temp[i], &i),RT_ERR_RG_OK);
			fc_db.acl_SWindex_sorting_by_weight[j] = i; //record the rule which type is ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
			j++;
			*accumulateIdx=j;
		}
	}

	//1-3. sorting the rule by weight: BubSort
	for (i=MAX_ACL_SW_ENTRY_SIZE-1; i>0; i--){
		sp=1;
	   	for (j=0; j<=i; j++){
			//check data is valid
			if(fc_db.acl_SWindex_sorting_by_weight[j]==-1 || fc_db.acl_SWindex_sorting_by_weight[j+1]==-1)
				break;

		  	if (fc_db.acl_filter_temp[(fc_db.acl_SWindex_sorting_by_weight[j])].acl_weight <
				fc_db.acl_filter_temp[(fc_db.acl_SWindex_sorting_by_weight[j+1])].acl_weight) //compare the weight
		 	{
		 		//swap the rule index
			   	temp = fc_db.acl_SWindex_sorting_by_weight[j];
			   	fc_db.acl_SWindex_sorting_by_weight[j] = fc_db.acl_SWindex_sorting_by_weight[j+1];
			   	fc_db.acl_SWindex_sorting_by_weight[j+1] = temp;
			   	sp=0;
		 	}
	   	}
		if (sp==1) //if no swap happened, then no need to check the lower array index(they are already sorted).
			break;
	}

	return RT_ERR_RG_OK;
}

static int _rtk_fc_aclSWEntry_and_asic_reAdd(rt_acl_filterAndQos_t *acl_filter, int *acl_filter_idx)
{
	int ret;
    bzero(&fc_db.aclSWEntry,sizeof(fc_db.aclSWEntry));
    bzero(&fc_db.empty_aclSWEntry,sizeof(fc_db.empty_aclSWEntry));

    //check input parameter
    if(acl_filter == NULL
            || acl_filter_idx==NULL) {
        return (RT_ERR_RG_NULL_POINTER);
	}

	ACL_CTRL("========reAdd RG_ACL[%d]========",*acl_filter_idx);

    //check the fc_db.aclSWEntry has been used, reAdd must add aclSWEtry in the assigned acl_filter_idx
    ASSERT_EQ(_rtk_fc_aclSWEntry_get(*acl_filter_idx, &fc_db.aclSWEntry),RT_ERR_RG_OK);

    if(memcmp(&fc_db.aclSWEntry,&fc_db.empty_aclSWEntry,sizeof(rtk_fc_aclFilterEntry_t)))
    {
        return (RT_ERR_RG_ACL_SW_ENTRY_USED);
    }

    fc_db.aclSWEntry.acl_filter = *acl_filter;
	fc_db.aclSWEntry.valid = ENABLED;


	//try to add pure ACL H/W rule to ASIC
	ret = _rtk_fc_aclSWEntry_to_asic_add(acl_filter,&fc_db.aclSWEntry,*acl_filter_idx);
	if(ret != RT_ERR_RG_OK)
	{
		ACL_CTRL("RG_ACL[%d] Only work on HW-ACL, but this rule fail to add asic(ret=0x%x).",*acl_filter_idx, ret);
		WARNING("RG_ACL[%d] Only work on HW-ACL, but this rule fail to add asic(ret=0x%x).",*acl_filter_idx, ret);
		fc_db.stop_add_acl = TRUE;
	}
	else
	{
		ACL_CTRL("RG_ACL[%d] rule add to HW",*acl_filter_idx);
	}


    //reAdd must add aclSWEtry in the assigned acl_filter_idx
    ASSERT_EQ(_rtk_fc_aclSWEntry_set(*acl_filter_idx, &fc_db.aclSWEntry),RT_ERR_RG_OK);

    //_rtk_fc_aclSWEntry_dump();

    return (RT_ERR_RG_OK);
}

int _rtk_fc_aclSWEntry_and_asic_rearrange(void)
{
	int i,j,accumulateIdx;
	rt_acl_filterAndQos_t *acl_filter;



	bzero(&fc_db.aclSWEntry_temp, sizeof(rtk_fc_aclFilterEntry_t));
	bzero(fc_db.acl_filter_temp, sizeof(rt_acl_filterAndQos_t)*MAX_ACL_SW_ENTRY_SIZE);
	bzero(fc_db.acl_filter_temp_valid, sizeof(fc_db.acl_filter_temp_valid));

	//Clear HW_ACL user used table
	ASSERT_EQ(_rtk_fc_reset_asic_TableEntry(),RT_ERR_RG_OK);

	/*Skip ----------------- stop_add_hw_acl ----------------- Skip*/

	//clear the information of rearrange status
	fc_db.stop_add_acl = FALSE;
	fc_db.controlFuc.acl_arp_targetip_offset = 0;

	ACL_CTRL("================[do ACL rearrange]==================");
	//backup all acl_filter for reAdd, and clean all aclSWEntry
	for(i=0; i<MAX_ACL_SW_ENTRY_SIZE; i++)
	{
		if(fc_db.acl_SW_table_entry[i].valid!=ENABLED)
			continue;
		//backup acl_filter
		ASSERT_EQ(_rtk_fc_aclSWEntry_get(i, &fc_db.aclSWEntry_temp),RT_ERR_RG_OK);
		fc_db.acl_filter_temp[i] = fc_db.aclSWEntry_temp.acl_filter;
		fc_db.acl_filter_temp_valid[(i>>5)] |= (1<<(i&0x1f));
		//clean aclSWEntry
		bzero(&fc_db.aclSWEntry_temp, sizeof(rtk_fc_aclFilterEntry_t));
		ASSERT_EQ(_rtk_fc_aclSWEntry_set(i, &fc_db.aclSWEntry_temp),RT_ERR_RG_OK);
	}

	//reAdd all reserve aclSWEntry:  type priority as following
		//1. ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET (with acl_weight sorting)
	ASSERT_EQ(_rtk_fc_aclSWEntry_rearrange_ACL_weight(&accumulateIdx),RT_ERR_RG_OK);

#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	ASSERT_EQ(_rtk_fc_aclHWEntry_meter_remapping(),RT_ERR_RG_OK);
#endif
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	if(fc_db.acl_lan_portmask && (fc_db.acl_lan_portmask != RTK_FC_RT_ACL_PORTMASK_INVALID))
	{
		fc_db.acl_lan_portmask = 0;
		ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleDel(RTK_FC_ACLANDCF_RESERVED_ACL_LAN_PORTMASK_RESET),RT_ERR_RG_OK);
	}
#endif

	//1. readd the rules of type ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET
	for(j=0;j<MAX_ACL_SW_ENTRY_SIZE;j++){
		if(fc_db.acl_SWindex_sorting_by_weight[j]==-1)
			break;

		acl_filter = &fc_db.acl_filter_temp[(fc_db.acl_SWindex_sorting_by_weight[j])];
		ASSERT_EQ(_rtk_fc_aclSWEntry_and_asic_reAdd(acl_filter, &fc_db.acl_SWindex_sorting_by_weight[j]),RT_ERR_RG_OK);
	}

	/*Skip ----------------- sorting for other fwd_type ----------------- Skip*/

	/*Skip ----------------- maintain acl_SWindex_sorting_by_weight_and_ingress_cvid_action for ingress_vid ----------------- Skip*/

	return (RT_ERR_RG_OK);
}

static int _rtk_fc_aclSWEntry_not_support(rt_acl_filterAndQos_t *acl_filter)
{
	int i = 0;
	unsigned long long int action_fields[RT_ACL_ACTION_GROUP_END];
	int bit_count[RT_ACL_ACTION_GROUP_END];
	int ret = RT_ERR_RG_OK;
	rt_rate_meter_mapping_t meterMapping;
	int meterIdxHwRange_min, meterIdxHwRange_max;


	if(acl_filter->action_fields == 0x0)
	{
		WARNING("Not support add ACL without any action.");
		return (RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	if(((acl_filter->filter_fields & RT_ACL_INGRESS_PORT_MASK_BIT) == 0x0) && ((acl_filter->filter_fields & RT_ACL_INGRESS_WLAN_MBSSID_MASK_BIT) == 0x0))
	{	//user not care any port, so asic default care all lan/wan/cpu port
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
		if(acl_filter->action_fields & RT_ACL_ACTION_METER_GROUP_SHARE_METER_BIT){	//not impact pol_id for wifi ssid
			acl_filter->ingress_port_mask = RT_ACL_PORT_ALL_LAN_BIT|RT_ACL_PORT_ALL_WAN_BIT;
			WARNING("ingress_port_mask is null, reconfigure it as all lan/wan port(0x%x)!", acl_filter->ingress_port_mask);
		}else
#endif
		{
			acl_filter->ingress_port_mask = RTK_FC_ALL_MAC_PORTMASK;
			ACL_CTRL("ingress_port_mask is null, reconfigure it as all port(0x%x)!", acl_filter->ingress_port_mask);
		}
	}

	if((acl_filter->filter_fields & RT_ACL_INGRESS_STAG_DEI_BIT) && (acl_filter->ingress_stag_dei > 1))
	{
		WARNING("ingress_stag_dei only support value 0 or 1, not support %d!", acl_filter->ingress_stag_dei);
		return (RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	if((acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_CFI_BIT) && (acl_filter->ingress_ctag_cfi > 1))
	{
		WARNING("ingress_ctag_cfi only support value 0 or 1, not support %d!", acl_filter->ingress_ctag_cfi);
		return (RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	if(fc_db.controlFuc.rt_api_is_enabled){
		if(acl_filter->action_fields & RT_ACL_ACTION_METER_GROUP_SHARE_METER_BIT)
			{
				ret = RTK_FC_HELPER_RT_RATE_SHAREMETER_MAPPING_HW_GET(acl_filter->action_meter_group_share_meter_index, &meterMapping);
				if((ret == RT_ERR_RG_OK) && (meterMapping.type == RT_METER_TYPE_ACL))
				{
					//check if hw index is valid
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
					meterIdxHwRange_min = 0;
					meterIdxHwRange_max = (RTK_FC_TABLESIZE_SHAREMTR-1);
#elif defined(CONFIG_RTK_L34_G3_PLATFORM)
					meterIdxHwRange_min = G3_FLOW_POLICER_FLOWMTR_SIZE; //flow_policer[0:31] reserved for flow meter
					meterIdxHwRange_max = ((G3_L3_FLOW_POLICER_SIZE - G3_L3_FLOW_POLICER_SIZE_MIB)-1);
#endif
					if((meterIdxHwRange_min > meterMapping.hw_index) || (meterMapping.hw_index > meterIdxHwRange_max))
					{
						WARNING("Invalid hw meter index (%d) for ACL rate limiting, please add meter for ACL first.", meterMapping.hw_index);
						return (RT_ERR_RG_INVALID_PARAM);
					}

#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
					{
						for(i = 0; i < RTK_FC_TABLESIZE_POLREMAP; i++)
						{
							if(!fc_db.hwPolRemap[i].valid)
								break;
							if(fc_db.hwPolRemap[i].pol_id == meterMapping.hw_index)
								break;
						}
						if(i == RTK_FC_TABLESIZE_POLREMAP){
							WARNING("Fail to remapping hw meter index (%d) due to limited resource(%d), please check /proc/fc/sw_dump/rt_meter.", meterMapping.hw_index, RTK_FC_TABLESIZE_POLREMAP);
							return (RT_ERR_RG_INVALID_PARAM);
						}
					}
#endif
				}
				else
				{
					WARNING("get ACL meter hw index Failed, return value: 0x%x, virtual meter type: %d", ret, meterMapping.type);
					return (RT_ERR_RG_INVALID_PARAM);
				}
			}

	}

	if(acl_filter->action_fields & RT_ACL_ACTION_PRIORITY_GROUP_TRAP_PRIORITY_BIT)
	{
		if(acl_filter->action_priority_group_trap_priority > 7)
		{
			WARNING("Only support priority range 0~7, not support %d.", acl_filter->action_priority_group_trap_priority);
			return (RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}

	if(acl_filter->action_fields & RT_ACL_ACTION_PRIORITY_GROUP_ACL_PRIORITY_BIT)
	{
		if(acl_filter->action_priority_group_acl_priority > 7)
		{
			WARNING("Only support priority range 0~7, not support %d.", acl_filter->action_priority_group_acl_priority);
			return (RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	if((acl_filter->action_fields&RT_ACL_ACTION_LOGGING_GROUP_MIB_BIT) && (acl_filter->action_logging_group_mib_index >= MAX_ACL_MIB_TABLE_SIZE))
	{	//reserve 1 mib to trap to slave cpu function
		WARNING("Only support mib index 0~%d, not support %d.", MAX_ACL_MIB_TABLE_SIZE-1, acl_filter->action_logging_group_mib_index);
		return (RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
#else
	if((acl_filter->action_fields&RT_ACL_ACTION_LOGGING_GROUP_MIB_BIT) && (acl_filter->action_logging_group_mib_index >= RT_STAT_ACLMIB_TABLE_SIZE))
	{
		WARNING("Only support mib index 0~%d, not support %d.", RT_STAT_ACLMIB_TABLE_SIZE-1, acl_filter->action_logging_group_mib_index);
		return (RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
#endif

	//check supported actions, each group only support one bit
	bzero(bit_count, sizeof(int)*RT_ACL_ACTION_GROUP_END);
	action_fields[RT_ACL_ACTION_GROUP_FORWARD] = acl_filter->action_fields & RT_ACL_ACTION_FORWARD_GROUP_BITS;
	action_fields[RT_ACL_ACTION_GROUP_PRIORITY] = acl_filter->action_fields & RT_ACL_ACTION_PRIORITY_GROUP_BITS;
	action_fields[RT_ACL_ACTION_GROUP_METER] = acl_filter->action_fields & RT_ACL_ACTION_METER_GROUP_BITS;
	action_fields[RT_ACL_ACTION_GROUP_LOGGING] = acl_filter->action_fields & RT_ACL_ACTION_LOGGING_GROUP_BITS;
	for(i=0; i<RT_ACL_ACTION_GROUP_END; i++) {
		while(action_fields[i]) {
			bit_count[i]++;
			action_fields[i] &= (action_fields[i] - 1ULL);
		}
		if(bit_count[i] > 1) {
			WARNING("Only support one %s action at the same time.", 
				(i<RT_ACL_ACTION_GROUP_PRIORITY)?"FORWARD":
					(i>RT_ACL_ACTION_GROUP_PRIORITY)?"METER":"PRIORITY");
			return (RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}

	//only trap with priority is available
	if((acl_filter->action_fields&RT_ACL_ACTION_PRIORITY_GROUP_TRAP_PRIORITY_BIT) && ((acl_filter->action_fields&RT_ACL_ACTION_FORWARD_GROUP_BITS) == 0x0))
	{
		WARNING("RT_ACL_ACTION_PRIORITY_GROUP_TRAP_PRIORITY_BIT only work along with FORWARD+trap action.");
		return (RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
#if defined(CONFIG_FC_RTL9607C_SERIES)

	if(fc_db.controlFuc.special_fast_forward_mode && dynamic_sram_desc ==0)
	{
		//printk("DRAM mode on acl_filter->action_fields = %x acl_filter->action_priority_group_trap_priority = %d ~~~~ acl_filter->action_fields&RT_ACL_ACTION_PRIORITY_GROUP_TRAP_PRIORITY_BIT = %d\n",acl_filter->action_fields, acl_filter->action_priority_group_trap_priority,acl_filter->action_fields&RT_ACL_ACTION_PRIORITY_GROUP_TRAP_PRIORITY_BIT);
		if(acl_filter->action_fields&RT_ACL_ACTION_PRIORITY_GROUP_TRAP_PRIORITY_BIT)
		{
			//printk("acl_filter->action_priority_group_trap_priority = %d acl_filter->action_priority_group_acl_priority = %d\n",acl_filter->action_priority_group_trap_priority, acl_filter->action_priority_group_acl_priority);
			if(acl_filter->action_priority_group_trap_priority <= 5 && acl_filter->action_priority_group_trap_priority >= 4)
			{
				printk("dram mode is on, try to re-arrange priority to 6\n");
				acl_filter->action_priority_group_trap_priority = 6;
			}
		}

	}
#endif

	return (RT_ERR_RG_OK);
}

static int32 _rtk_fc_aclSWEntry_pattern_mask_check(rt_acl_filterAndQos_t *acl_filter)
{
	//Downward compatibility: if pattern with zero carebit, consider as all care.
	if(acl_filter->filter_fields & RT_ACL_INGRESS_SMAC_BIT)
	{
		if((acl_filter->ingress_smac_mask[0] |
				acl_filter->ingress_smac_mask[1] |
				acl_filter->ingress_smac_mask[2] |
				acl_filter->ingress_smac_mask[3] |
				acl_filter->ingress_smac_mask[4] |
				acl_filter->ingress_smac_mask[5]) ==0x0 ){

				acl_filter->ingress_smac_mask[0]=0xffU;
				acl_filter->ingress_smac_mask[1]=0xffU;
				acl_filter->ingress_smac_mask[2]=0xffU;
				acl_filter->ingress_smac_mask[3]=0xffU;
				acl_filter->ingress_smac_mask[4]=0xffU;
				acl_filter->ingress_smac_mask[5]=0xffU;
			}
	}


	if(acl_filter->filter_fields & RT_ACL_INGRESS_DMAC_BIT)
	{
		if((acl_filter->ingress_dmac_mask[0]|
			acl_filter->ingress_dmac_mask[1]|
			acl_filter->ingress_dmac_mask[2]|
			acl_filter->ingress_dmac_mask[3]|
			acl_filter->ingress_dmac_mask[4]|
			acl_filter->ingress_dmac_mask[5])==0x0 ){

			acl_filter->ingress_dmac_mask[0]=0xffU;
			acl_filter->ingress_dmac_mask[1]=0xffU;
			acl_filter->ingress_dmac_mask[2]=0xffU;
			acl_filter->ingress_dmac_mask[3]=0xffU;
			acl_filter->ingress_dmac_mask[4]=0xffU;
			acl_filter->ingress_dmac_mask[5]=0xffU;
		}

	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_STREAM_ID_BIT)
	{
		if(acl_filter->ingress_stream_id_mask==0x0U){
			acl_filter->ingress_stream_id_mask = 0xffffU;
		}
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_ETHERTYPE_BIT)
	{
 		if(acl_filter->ingress_ethertype_mask==0x0U){
			acl_filter->ingress_ethertype_mask = 0xffffU;
		}
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_SIP_BIT )
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
			acl_filter->ingress_src_ipv6_addr_mask[0] = 0xffU;
			acl_filter->ingress_src_ipv6_addr_mask[1] = 0xffU;
			acl_filter->ingress_src_ipv6_addr_mask[2] = 0xffU;
			acl_filter->ingress_src_ipv6_addr_mask[3] = 0xffU;
			acl_filter->ingress_src_ipv6_addr_mask[4] = 0xffU;
			acl_filter->ingress_src_ipv6_addr_mask[5] = 0xffU;
			acl_filter->ingress_src_ipv6_addr_mask[6] = 0xffU;
			acl_filter->ingress_src_ipv6_addr_mask[7] = 0xffU;
			acl_filter->ingress_src_ipv6_addr_mask[8] = 0xffU;
			acl_filter->ingress_src_ipv6_addr_mask[9] = 0xffU;
			acl_filter->ingress_src_ipv6_addr_mask[10] = 0xffU;
			acl_filter->ingress_src_ipv6_addr_mask[11] = 0xffU;
			acl_filter->ingress_src_ipv6_addr_mask[12] = 0xffU;
			acl_filter->ingress_src_ipv6_addr_mask[13] = 0xffU;
			acl_filter->ingress_src_ipv6_addr_mask[14] = 0xffU;
			acl_filter->ingress_src_ipv6_addr_mask[15] = 0xffU;
		}
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_DIP_BIT )
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

				acl_filter->ingress_dest_ipv6_addr_mask[0] = 0xffU;
				acl_filter->ingress_dest_ipv6_addr_mask[1] = 0xffU;
				acl_filter->ingress_dest_ipv6_addr_mask[2] = 0xffU;
				acl_filter->ingress_dest_ipv6_addr_mask[3] = 0xffU;
				acl_filter->ingress_dest_ipv6_addr_mask[4] = 0xffU;
				acl_filter->ingress_dest_ipv6_addr_mask[5] = 0xffU;
				acl_filter->ingress_dest_ipv6_addr_mask[6] = 0xffU;
				acl_filter->ingress_dest_ipv6_addr_mask[7] = 0xffU;
				acl_filter->ingress_dest_ipv6_addr_mask[8] = 0xffU;
				acl_filter->ingress_dest_ipv6_addr_mask[9] = 0xffU;
				acl_filter->ingress_dest_ipv6_addr_mask[10] = 0xffU;
				acl_filter->ingress_dest_ipv6_addr_mask[11] = 0xffU;
				acl_filter->ingress_dest_ipv6_addr_mask[12] = 0xffU;
				acl_filter->ingress_dest_ipv6_addr_mask[13] = 0xffU;
				acl_filter->ingress_dest_ipv6_addr_mask[14] = 0xffU;
				acl_filter->ingress_dest_ipv6_addr_mask[15] = 0xffU;
		}
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_TCP_FLAGS_BIT)
	{
 		if(acl_filter->ingress_tcp_flags_mask==0x0U){
			acl_filter->ingress_tcp_flags_mask = 0xfffU;
		}
	}

	return (RT_ERR_RG_OK);
}

static int _rtk_fc_aclHWEntry_maskLength_get_by_ipv4_range(ipaddr_t ipv4_addr_start, ipaddr_t ipv4_addr_end, int* maskLength)
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
		if(((ipv4_addr_start<<i)&0x80000000)!=0U){ACL_CTRL("IP Range not in mask alignment \n"); return(RT_ERR_RG_FAILED);}
		if(((ipv4_addr_end<<i)&0x80000000)!=0x80000000){ACL_CTRL("IP Range not in mask alignment \n"); return(RT_ERR_RG_FAILED);}
	}

	*maskLength = (32-length);

	return (RT_ERR_RG_OK);

}

int _rtk_fc_aclHeaderInfo_get(char *acl_dbg_info, int acl_dbg_info_size, int ref_info)
{
	bzero(&fc_db.aclSWEntry_temp, sizeof(fc_db.aclSWEntry_temp));

	if(ref_info&RXINFO_REF_ACL_RSN_CTRL_RSV)
	{
		if(ref_info&RXINFO_REF_ACL_RSN_CTRL_TRAP)
			snprintf(acl_dbg_info, acl_dbg_info_size, "(trap by RSV_ACL[%d])", (ref_info&RXINFO_REF_ACL_RSN_VAL));
		else
			snprintf(acl_dbg_info, acl_dbg_info_size, "(hit RSV_ACL[%d])", (ref_info&RXINFO_REF_ACL_RSN_VAL));
		return RT_ERR_RG_OK;
	}
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	//work around mdata[16:63] will be overrite by hash CRC16/32, sw debug only
	else if((ref_info&RXINFO_REF_ACL_RSN_CTRL_TRAP) == 0)
	{
		if(ref_info < RTK_FC_ACLANDCF_RESERVED_TAIL_END){
			snprintf(acl_dbg_info, acl_dbg_info_size, "(trap by RSV_ACL[%d])", (ref_info&RXINFO_REF_ACL_RSN_VAL));
			return RT_ERR_RG_OK;
		}else{
			ref_info -= RTK_FC_ACLANDCF_RESERVED_TAIL_END;
		}
	}
#endif

	ref_info &= RXINFO_REF_ACL_RSN_VAL;
	if(_rtk_fc_aclSWEntry_get(ref_info, &fc_db.aclSWEntry_temp) == RT_ERR_RG_OK)
	{
		if(((fc_db.aclSWEntry_temp.acl_filter.action_fields)&RT_ACL_ACTION_PRIORITY_GROUP_TRAP_PRIORITY_BIT) && ((fc_db.aclSWEntry_temp.acl_filter.action_fields)&RT_ACL_ACTION_FORWARD_GROUP_TRAP_BIT))
			snprintf(acl_dbg_info, acl_dbg_info_size, "(trap+pri%d by RT_ACL[%d])", fc_db.aclSWEntry_temp.acl_filter.action_priority_group_trap_priority, ref_info);
		else if((fc_db.aclSWEntry_temp.acl_filter.action_fields)&RT_ACL_ACTION_FORWARD_GROUP_TRAP_BIT)
			snprintf(acl_dbg_info, acl_dbg_info_size, "(trap by RT_ACL[%d])", ref_info);
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
		else if((fc_db.aclSWEntry_temp.acl_filter.action_fields)&RT_ACL_ACTION_FORWARD_GROUP_TRAP_TO_PS_BIT)
			snprintf(acl_dbg_info, acl_dbg_info_size, "(trap to ps by RT_ACL[%d])", ref_info);
#if defined(CONFIG_FC_CA8277AB_SERIES)
		else
			snprintf(acl_dbg_info, acl_dbg_info_size, "(hit RT_ACL[%d])", ref_info);
#endif
#endif
	}

	return RT_ERR_RG_OK;
}

/*(3)RT APIs*/
int32 _rtk_fc_aclFilterAndQos_add(rt_acl_filterAndQos_t *acl_filter, int *acl_filter_idx)
{
	int i,ret;
	int aclSWEntryIdx;
	rt_acl_filterAndQos_t *p_acl_filter = acl_filter;
	bzero(&fc_db.aclSWEntry,sizeof(rtk_fc_aclFilterEntry_t));

	/*Skip ----------------- fc has been init check ----------------- Skip*/

	//check input parameter
	if(acl_filter_idx == NULL)
		return (RT_ERR_RG_NULL_POINTER);

	if(fc_db.controlFuc.acl_para_dump)
	{
		rtk_fc_aclFilterEntry_t tmp_aclSWEntry={0};
		rtlglue_printf("ACL parameter dump:\n");
		memcpy(&tmp_aclSWEntry.acl_filter, acl_filter, sizeof(tmp_aclSWEntry.acl_filter));
		rtk_fc_dump_fc_acl_entry_content(NULL,&tmp_aclSWEntry);
	}

	ASSERT_EQ(_rtk_fc_aclSWEntry_not_support(p_acl_filter),RT_ERR_RG_OK);

	//Downward compatibility: if pattern with zero carebit, consider as all care.
	ASSERT_EQ(_rtk_fc_aclSWEntry_pattern_mask_check(p_acl_filter),RT_ERR_RG_OK);

	ASSERT_EQ(_rtk_fc_aclHWEntry_not_support(p_acl_filter),RT_ERR_RG_OK);

	//using ACL[0] for multicast protextion
	_rtk_fc_acl_for_multicast_temp_protection_add();


	//pre-check HW ACL is enough or not, if the rule fail to harwarelze, just retun fail.
	ret = _rtk_fc_aclSWEntry_to_asic_add(p_acl_filter,&fc_db.aclSWEntry,FAIL);
	if(ret != RT_ERR_RG_OK)
	{
		goto aclFailed;
	}


	memcpy(&fc_db.aclSWEntry.acl_filter, acl_filter, sizeof(fc_db.aclSWEntry.acl_filter));
	fc_db.aclSWEntry.valid=ENABLED;

	ret=_rtk_fc_aclSWEntry_empty_find(&aclSWEntryIdx);
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	ret=_rtk_fc_aclSWEntry_set(aclSWEntryIdx, &fc_db.aclSWEntry);
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	*acl_filter_idx = aclSWEntryIdx;
	ACL_CTRL("add aclSWEntry[%d]",*acl_filter_idx);

	//count the total aclSWEntry size
	fc_db.acl_SW_table_entry_size=0;
	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++)
	{
		if(fc_db.acl_SW_table_entry[i].valid==ENABLED)
			fc_db.acl_SW_table_entry_size++;
	}

	//readd all acl rules for sorting: action  INGRESS_ALL(sort by weight)
	ret = _rtk_fc_aclSWEntry_and_asic_rearrange();
	if(fc_db.stop_add_acl) {
		WARNING("add to RG ACL Faild due to ACL rearrange fail!");
		ret = RT_ERR_RG_ACL_NOT_SUPPORT;
		goto aclFailed;
	}
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	/*Skip ----------------- _rtk_rg_determind_fb_flow_hash_flexible_pattern due to enable flow hash of Cpri and DSCP by default ----------------- Skip*/

	/*Skip ----------------- _rtk_rg_shortCut_clear ----------------- Skip*/

	if(fc_db.controlFuc.acl_para_dump)
	{
		rtlglue_printf("RT_ACL[%d] add success!\n", *acl_filter_idx);
	}

	_rtk_fc_acl_for_multicast_temp_protection_del();
	return (RT_ERR_RG_OK);

aclFailed:

	if(fc_db.controlFuc.acl_para_dump)
	{
		rtlglue_printf("RT_ACL add fail with ret=0x%x\n", ret);
	}

	_rtk_fc_acl_for_multicast_temp_protection_del();

	if(fc_db.stop_add_acl) {
		ASSERT_EQ(_rtk_fc_aclFilterAndQos_del(*acl_filter_idx),RT_ERR_RG_OK);
	}

	return ret;

}




int32 _rtk_fc_aclFilterAndQos_del(int acl_filter_idx)
{
	int i,ret=RT_ERR_RG_FAILED;
	bzero(&fc_db.aclSWEntry,sizeof(rtk_fc_aclFilterEntry_t));

	/*Skip ----------------- rg has been init check ----------------- Skip*/

	if(acl_filter_idx<0 || acl_filter_idx>=MAX_ACL_SW_ENTRY_SIZE){
		WARNING("invalid ACL index(%d)!",acl_filter_idx);
		return (RT_ERR_RG_INDEX_OUT_OF_RANGE);
	}

	if(fc_db.controlFuc.acl_para_dump)
	{
		rtlglue_printf("ACL parameter dump:\n");
		ASSERT_EQ(_rtk_fc_aclSWEntry_get(acl_filter_idx, &fc_db.aclSWEntry),RT_ERR_RG_OK);
		rtk_fc_dump_fc_acl_entry_content(NULL,&fc_db.aclSWEntry);
		bzero(&fc_db.aclSWEntry,sizeof(rtk_fc_aclFilterEntry_t));
	}

	//using ACL[0] for multicast protextion
	_rtk_fc_acl_for_multicast_temp_protection_add();

	//clean aclSWEntry
	ret = _rtk_fc_aclSWEntry_set(acl_filter_idx, &fc_db.aclSWEntry);
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	//rearrange the ACL & CF ASIC to avoid discontinuous entry
	ret = _rtk_fc_aclSWEntry_and_asic_rearrange();
	if(ret!=RT_ERR_RG_OK) goto aclFailed;

	//count the total aclSWEntry size
	fc_db.acl_SW_table_entry_size=0;
	for(i=0;i<MAX_ACL_SW_ENTRY_SIZE;i++)
	{
		if(fc_db.acl_SW_table_entry[i].valid==ENABLED)
			fc_db.acl_SW_table_entry_size++;
	}

	ACL_CTRL("delete aclSWEntry[%d]",acl_filter_idx);

	/*Skip ----------------- _rtk_rg_determind_fb_flow_hash_flexible_pattern due to enable flow hash of Cpri and DSCP by default ----------------- Skip*/

	/*Skip ----------------- _rtk_rg_shortCut_clear ----------------- Skip*/

	if(fc_db.controlFuc.acl_para_dump)
	{
		rtlglue_printf("RT_ACL[%d] del success!\n", acl_filter_idx);
	}

	_rtk_fc_acl_for_multicast_temp_protection_del();
	return (RT_ERR_RG_OK);

aclFailed:

	if(fc_db.controlFuc.acl_para_dump)
	{
		rtlglue_printf("RT_ACL del fail with ret=0x%x\n", ret);
	}

	_rtk_fc_acl_for_multicast_temp_protection_del();
	return ret;

}


int32 _rtk_fc_aclFilterAndQos_get(int acl_filter_idx, rt_acl_filterAndQos_t *acl_filter)
{
	bzero(&fc_db.aclSWEntry, sizeof(fc_db.aclSWEntry));

	/*Skip ----------------- rg has been init check ----------------- Skip*/

	if(acl_filter==NULL)
		return (RT_ERR_RG_NULL_POINTER);

	if((acl_filter_idx>=0) && (acl_filter_idx<MAX_ACL_SW_ENTRY_SIZE)){
		ASSERT_EQ(_rtk_fc_aclSWEntry_get(acl_filter_idx,&fc_db.aclSWEntry),RT_ERR_RG_OK);
		if(fc_db.aclSWEntry.valid == ENABLED) {
			*acl_filter = fc_db.aclSWEntry.acl_filter;
			ACL_CTRL("_rtk_fc_aclFilterAndQos_get index=%d",acl_filter_idx);
			return (RT_ERR_RG_OK);
		}
		ACL_CTRL("_rtk_fc_aclFilterAndQos_get failed");
	}else{
		return (RT_ERR_RG_INDEX_OUT_OF_RANGE);
	}
	//not found
	return (RT_ERR_RG_ACL_SW_ENTRY_NOT_FOUND);
}

int32 _rtk_fc_acl_mib_get(int idx, rt_stat_acl_mib_t *paclMibCnt)
{
	int ret = RT_ERR_RG_OK;
	uint64 byte_count;
	uint32 packet_count;
	uint8 type = 0;	//0:byte count 1:packet count
	byte_count = 0ULL;
	packet_count = 0U;

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	{
		rtk_stat_log_ctrl_t logCtrl;

		bzero((void*) &logCtrl, sizeof(logCtrl));
		ASSERT_EQ(rtk_stat_logCtrl_get(idx*2, &logCtrl), RT_ERR_RG_OK);

		if(logCtrl.type == STAT_LOG_TYPE_BYTECNT){	//64bits byte count or 32bits packet count
			type = 0;
			ret = rtk_stat_log_get(idx*2, &byte_count);
		}else{
			type = 1;
			ret = rtk_stat_log_get(idx*2, &byte_count);
			packet_count = byte_count;
			byte_count = 0;
		}
	}
#endif
#if defined(CONFIG_RTK_L34_G3_PLATFORM) && (defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES))
	{	//read clear
		aal_l3_te_pm_policer_t l3_pm_data;

		ret = aal_l3_te_pm_policer_agr_flow_get(G3_DEF_DEVID, idx + G3_FLOW_POLICER_IDXSHIFT_ACLMIB, &l3_pm_data);
		if(ret == CA_E_OK){
			fc_db.acl_mib[idx].bytes_cnt += l3_pm_data.total_bytes;
			fc_db.acl_mib[idx].pkt_cnt += l3_pm_data.total_pkt;
			type = fc_db.acl_mib[idx].mib_type;
			if(fc_db.acl_mib[idx].mib_type == 0)
				byte_count = fc_db.acl_mib[idx].bytes_cnt;
			else
				packet_count = fc_db.acl_mib[idx].pkt_cnt;
		}
	}
#endif

	if(ret != RT_ERR_OK)
		WARNING("fail to get acl mib counter %d", idx);

	paclMibCnt->type = type;
	paclMibCnt->byte_count = byte_count;
	paclMibCnt->packet_count = packet_count;

	return ret;
}

int32 _rtk_fc_acl_mib_set(int idx, rt_stat_acl_mib_t paclMibCnt)
{
	int ret = RT_ERR_RG_OK;
	//paclMibCnt.type ----> 0:byte count 1:packet count

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	{
		rtk_stat_log_ctrl_t logCtrl;

		bzero((void*) &logCtrl, sizeof(logCtrl));
		ASSERT_EQ(rtk_stat_logCtrl_get(idx*2, &logCtrl), RT_ERR_RG_OK);

		//reset counter if change to different type
		if(((logCtrl.type == 0) && (paclMibCnt.type == STAT_LOG_TYPE_PKTCNT)) || 
			((logCtrl.type == 1) && (paclMibCnt.type == STAT_LOG_TYPE_BYTECNT))){
			ASSERT_EQ(rtk_stat_log_reset(idx*2), RT_ERR_RG_OK);
		}
		
		if(paclMibCnt.type == 0){
			logCtrl.type = STAT_LOG_TYPE_BYTECNT;
			logCtrl.mode = STAT_LOG_MODE_64BITS;
		}else{
			logCtrl.type = STAT_LOG_TYPE_PKTCNT;
			logCtrl.mode = STAT_LOG_MODE_32BITS;
		}
		ret = rtk_stat_logCtrl_set(idx*2, logCtrl);
	}
#elif defined(CONFIG_RTK_L34_G3_PLATFORM) && (defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES))
	{
		aal_l3_te_pm_policer_t l3_pm_data;

		//reset counter if change to different type
		if(fc_db.acl_mib[idx].mib_type != paclMibCnt.type){
			//read clear
			ret = aal_l3_te_pm_policer_agr_flow_get(G3_DEF_DEVID, idx + G3_FLOW_POLICER_IDXSHIFT_ACLMIB, &l3_pm_data);
			fc_db.acl_mib[idx].bytes_cnt = 0;
			fc_db.acl_mib[idx].pkt_cnt = 0;
		}

		fc_db.acl_mib[idx].mib_type = paclMibCnt.type;
	}
#else
	WARNING("This chip not support this function.");
#endif

	if(ret != RT_ERR_OK)
		WARNING("fail to set acl mib counter %d as type %d", idx, paclMibCnt.type);

	return ret;
}

int32 _rtk_fc_acl_mib_clear(int idx)
{
	int ret = RT_ERR_RG_OK;

#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	ret = rtk_stat_log_reset(idx*2);
#endif
#if defined(CONFIG_RTK_L34_G3_PLATFORM) && (defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES))
	{	//read clear
		aal_l3_te_pm_policer_t l3_pm_data;

		ret = aal_l3_te_pm_policer_agr_flow_get(G3_DEF_DEVID, idx + G3_FLOW_POLICER_IDXSHIFT_ACLMIB, &l3_pm_data);
		fc_db.acl_mib[idx].bytes_cnt = 0;
		fc_db.acl_mib[idx].pkt_cnt = 0;
	}
#endif

	if(ret != RT_ERR_OK)
		WARNING("fail to reset acl mib counter %d", idx);

	return ret;
}

int32 _rtk_fc_acl_reserved_status_get(rt_acl_reserved_type_t reserved_type, int *status)
{
	int ret = RT_ERR_RG_OK;
	bool dump = FALSE;
	int loop, loop_start, loop_end, hw_rsv_type;
	char *type_name[RT_ACL_RESERVED_TYPE_MAX]={"ICMPV4_TRAP","ICMPV6_TRAP"};

	if(!fc_db.controlFuc.rt_api_is_enabled)
		return ret;

	if(reserved_type == RT_ACL_RESERVED_TYPE_MAX){
		loop_start = RT_ACL_RESERVED_TYPE_ICMPV4_TRAP;
		loop_end = RT_ACL_RESERVED_TYPE_MAX;
		dump = TRUE;
	}else{
		loop_start = reserved_type;
		loop_end = reserved_type+1;
	}

	for(loop = loop_start; loop < loop_end; loop++){
		hw_rsv_type = fc_db.aclAndCfReservedRule.reservedMap[loop];
		*status = fc_db.aclAndCfReservedRule.reservedMask[hw_rsv_type];
		if(dump)
			rtlglue_printf("ACL software reserved type[%d](%s) status is %s (HW RSV[%d]).\n", loop, type_name[loop], *status?"enable":"disable", hw_rsv_type);
	}

	return ret;
}

int32 _rtk_fc_acl_reserved_status_set(rt_acl_reserved_type_t reserved_type, int status)
{
	int ret = RT_ERR_RG_OK;
	int hw_rsv_type;

	if(!fc_db.controlFuc.rt_api_is_enabled)
		return ret;

	hw_rsv_type = fc_db.aclAndCfReservedRule.reservedMap[reserved_type];

	if(status)
		ret = _rtk_rg_aclAndCfReservedRuleAdd(hw_rsv_type, NULL);
	else
		ret = _rtk_rg_aclAndCfReservedRuleDel(hw_rsv_type);

	return ret;
}

/*(4)reserved ACL related APIs*/
int _rtk_fc_aclAndCfReservedRule_intfRsvUpdate(bool is_add, int intf_swIdx)
{
#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
	int i, j, k;
	rtk_fc_aclAndCf_reserved_hybrid_pptp_func_t local_update;

	bzero(&local_update,sizeof(local_update));

	if(fc_db.hypridPPTP.portmask){
		//Get different netif mac OO:OO:OO:OO:OO:XX (XX means not care)
		for(i=DEVIFIDX_VALID_MIN; i<RTK_FC_DEV_GW_MAC_TBL; i++){
			if(fc_db.devGwMacTbl[i].dev == NULL)
				continue;
			if(fc_db.devGwMacTbl[i].dev)
			{
				if(!RTK_FC_HOOK_PS_DEV_IS_PRIV_FLAGS_SET(fc_db.devGwMacTbl[i].dev, RTK_FC_NETDEV_PRIV_FLAG_TYPE_EBRIDGE))	//skip non bridge interface
					continue;
			}

			if(local_update.intf_count == 0U){
				memcpy(&local_update.gateway_mac_addr[local_update.intf_count], &fc_db.devGwMacTbl[i].dev->dev_addr, ETH_ALEN);
				local_update.intf_count++;
			}else{
				for(j=0; j<local_update.intf_count; j++){
					for(k=0; k<(ETH_ALEN-1); k++){	//mac[0]~mac[4] should be the same
						if(local_update.gateway_mac_addr[j].octet[k] != fc_db.devGwMacTbl[i].dev->dev_addr[k])
							break;
					}
				}
				if((j != local_update.intf_count) || (k != (ETH_ALEN-1))){
					memcpy(&local_update.gateway_mac_addr[local_update.intf_count], &fc_db.devGwMacTbl[i].dev->dev_addr, ETH_ALEN);
					local_update.intf_count++;	//collect different gw mac
				}
			}
		}
	}

	if(memcmp(&fc_db.aclAndCfReservedRule.hybrid_pptp_func, &local_update, sizeof(fc_db.aclAndCfReservedRule.hybrid_pptp_func))){
		memcpy(&fc_db.aclAndCfReservedRule.hybrid_pptp_func, &local_update, sizeof(fc_db.aclAndCfReservedRule.hybrid_pptp_func));
		//for RTK_FC_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY & RTK_FC_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY
		_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_TAIL_END, NULL);
	}
#endif
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
	int i;

	if(is_add)
	{
		if(fc_db.netifTbl[intf_swIdx].intf.valid==FALSE)
			return (RT_ERR_RG_OK);

		if(fc_db.netifTbl[intf_swIdx].dualType==RTK_FC_DUALTYPE_IP4INIP6){
			assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_MULTICAST_DSLITE_TRAP, NULL));
			TRACE("enable dslite intf reserve acl for hw intf[%d]", intf_swIdx);
		}
	}
	else
	{
		fc_db.aclAndCfReservedRule.dslite_intf_count = 0;
		for(i=0; i<RTK_FC_TABLESIZE_INTF_SW; i++){
			if(fc_db.netifTbl[i].intf.valid == FALSE)
				continue;
			if(fc_db.netifTbl[i].dualType==RTK_FC_DUALTYPE_IP4INIP6)
				fc_db.aclAndCfReservedRule.dslite_intf_count++;
		}

		if(fc_db.aclAndCfReservedRule.dslite_intf_count == 0){
			assert_ok(_rtk_rg_aclAndCfReservedRuleDel(RTK_FC_ACLANDCF_RESERVED_MULTICAST_DSLITE_TRAP));
			TRACE("disable dslite intf reserve acl after delete sw intf[%d]", intf_swIdx);
		}
	}
#endif

	return (RT_ERR_RG_OK);
}



#if defined(CONFIG_RTK_L34_XPON_PLATFORM)
/*(1)ACL init related APIs*/
int _rtk_fc_acl_asic_init(void)
{
	int i;
	rtk_filter_unmatch_action_type_t pAction;
	rtk_acl_template_t aclTemplate;
	rtk_acl_rangeCheck_ip_t iprangeEntry;
	rtk_acl_rangeCheck_l4Port_t prtRangeEntry;
	rtk_acl_rangeCheck_pktLength_t pktlenRangeEntry;
	rtk_acl_field_entry_t fieldSel;
	rtk_stat_log_ctrl_t logCtrl;

	pAction = FILTER_UNMATCH_PERMIT;
	for(i=0;i<RTK_FC_MAC_PORT_MAX;i++)
	{
		/*set ACL_EN*/
		if(RTK_ACL_IGRSTATE_SET(i,ENABLED))WARNING("init ACL for port %d failed!!!\n", i);
		/*set ACL_PERMIT*/
		if(RTK_ACL_IGRUNMATCHACTION_SET(i,pAction))WARNING("init ACL for port %d failed!!!\n", i);
	}

	/*init field selector: all FS is reserved in apolloPro*/
	//ApolloPro FS must defined by user own, no default setting as apollo/apolloFE
	for(i=0;i<8;i++)
	{
		bzero(&fieldSel,sizeof(fieldSel));
		fieldSel.index = i;
		fieldSel.format = ACL_FORMAT_RAW;
		fieldSel.offset = 0U;
		if(rtk_acl_fieldSelect_set(&fieldSel))WARNING("init ACL failed!!!\n");
	}

	//FS[1]: using for IP fragment flag
	{
		bzero(&fieldSel,sizeof(fieldSel));
		fieldSel.index = 1U;
		fieldSel.format = ACL_FORMAT_IPV4;
		fieldSel.offset = 6U;
		if(rtk_acl_fieldSelect_set(&fieldSel))WARNING("init ACL failed!!!\n");
	}


	//FS[2]: using for TCP flag
	{
		bzero(&fieldSel,sizeof(fieldSel));
		fieldSel.index = 2U;
		fieldSel.format = ACL_FORMAT_IPPAYLOAD;
		fieldSel.offset = 12U;
		if(rtk_acl_fieldSelect_set(&fieldSel))WARNING("init ACL failed!!!\n");
	}

	
	//FS[6]: using for ARP target ip[31:16], pattern: RT_ACL_INGRESS_ARP_TARGET_IP_BIT
	{
		bzero(&fieldSel,sizeof(fieldSel));
		fieldSel.index = 6;
		fieldSel.format = ACL_FORMAT_RAW;
		fieldSel.offset = ARP_TARGET_IP_BYTE_OFFSET;	// no stag/ctag
		if(rtk_acl_fieldSelect_set(&fieldSel))WARNING("init ACL failed!!!\n");
	}
	//FS[7]: using for ARP target ip[15:0], pattern: RT_ACL_INGRESS_ARP_TARGET_IP_BIT
	{
		bzero(&fieldSel,sizeof(fieldSel));
		fieldSel.index = 7;
		fieldSel.format = ACL_FORMAT_RAW;
		fieldSel.offset = ARP_TARGET_IP_BYTE_OFFSET+2;	// no stag/ctag
		if(rtk_acl_fieldSelect_set(&fieldSel))WARNING("init ACL failed!!!\n");
	}




	/*init TEMPLATE, sync the pattern index by struct  rtk_fc_aclField_Teamplate_t*/

	//aclTemplate[0]
	bzero((void*) &aclTemplate, sizeof(aclTemplate));
	aclTemplate.index = 0U;
	aclTemplate.fieldType[0] = ACL_FIELD_DMAC0;
	aclTemplate.fieldType[1] = ACL_FIELD_DMAC1;
	aclTemplate.fieldType[2] = ACL_FIELD_DMAC2;
	aclTemplate.fieldType[3] = ACL_FIELD_SMAC0;
	aclTemplate.fieldType[4] = ACL_FIELD_SMAC1;
	aclTemplate.fieldType[5] = ACL_FIELD_SMAC2;
	aclTemplate.fieldType[6] = ACL_FIELD_CTAG;
	aclTemplate.fieldType[7] = ACL_FIELD_STAG;
	if(rtk_acl_template_set(&aclTemplate))WARNING("init ACL failed!!!\n");


	//aclTemplate[1]
	bzero((void*) &aclTemplate, sizeof(aclTemplate));
	aclTemplate.index = 1U;
	aclTemplate.fieldType[0] = ACL_FIELD_IPV4_DIP0;
	aclTemplate.fieldType[1] = ACL_FIELD_IPV4_DIP1;
	aclTemplate.fieldType[2] = ACL_FIELD_IPV4_SIP0;
	aclTemplate.fieldType[3] = ACL_FIELD_IPV4_SIP1;
	aclTemplate.fieldType[4] = ACL_FIELD_IP_RANGE;
	aclTemplate.fieldType[5] = ACL_FIELD_ETHERTYPE;
	aclTemplate.fieldType[6] = ACL_FIELD_EXT_PORTMASK;
	aclTemplate.fieldType[7] = ACL_FIELD_GEMPORT;
	if(rtk_acl_template_set(&aclTemplate))WARNING("init ACL failed!!!\n");



	//aclTemplate[2]
	bzero((void*) &aclTemplate, sizeof(aclTemplate));
	aclTemplate.index = 2U;
	aclTemplate.fieldType[0] = ACL_FIELD_USER_DEFINED01; //l4 dport
	aclTemplate.fieldType[1] = ACL_FIELD_USER_DEFINED00; //l4 sport
	aclTemplate.fieldType[2] = ACL_FIELD_PORT_RANGE;
	aclTemplate.fieldType[3] = ACL_FIELD_IPV4_PROTOCOL;
	aclTemplate.fieldType[4] = ACL_FIELD_IPV6_NEXT_HEADER;
	aclTemplate.fieldType[5] = ACL_FIELD_FRAME_TYPE_TAGS; //tagIf
	aclTemplate.fieldType[6] = ACL_FIELD_VID_RANGE; //rsv
	aclTemplate.fieldType[7] = ACL_FIELD_PKT_LEN_RANGE; //rsv
	if(rtk_acl_template_set(&aclTemplate))WARNING("init ACL failed!!!\n");


	//aclTemplate[3]:all rsv, filter userField
	bzero((void*) &aclTemplate, sizeof(aclTemplate));
	aclTemplate.index = 3U;
	aclTemplate.fieldType[0] = ACL_FIELD_USER_DEFINED00;
	aclTemplate.fieldType[1] = ACL_FIELD_USER_DEFINED01;
	aclTemplate.fieldType[2] = ACL_FIELD_USER_DEFINED02;
	aclTemplate.fieldType[3] = ACL_FIELD_USER_DEFINED03;
	aclTemplate.fieldType[4] = ACL_FIELD_USER_DEFINED04;
	aclTemplate.fieldType[5] = ACL_FIELD_USER_DEFINED05;
	aclTemplate.fieldType[6] = ACL_FIELD_USER_DEFINED06;
	aclTemplate.fieldType[7] = ACL_FIELD_USER_DEFINED07;
	if(rtk_acl_template_set(&aclTemplate))WARNING("init ACL failed!!!\n");

	//aclTemplate[4]: inner IP
	bzero((void*) &aclTemplate, sizeof(aclTemplate));
	aclTemplate.index = 4U;
	aclTemplate.fieldType[0] = ACL_FIELD_IPV4_DIP0_INNER;
	aclTemplate.fieldType[1] = ACL_FIELD_IPV4_DIP1_INNER;
	aclTemplate.fieldType[2] = ACL_FIELD_IPV4_SIP0_INNER;
	aclTemplate.fieldType[3] = ACL_FIELD_IPV4_SIP1_INNER;
	aclTemplate.fieldType[4] = ACL_FIELD_IP_RANGE;
	aclTemplate.fieldType[5] = ACL_FIELD_ETHERTYPE;
	aclTemplate.fieldType[6] = ACL_FIELD_EXT_PORTMASK;
	aclTemplate.fieldType[7] = ACL_FIELD_IPV4_PROTOCOL_INNER;
	if(rtk_acl_template_set(&aclTemplate))WARNING("init ACL failed!!!\n");

	//aclTemplate[5]:SIPv6
	bzero((void*) &aclTemplate, sizeof(aclTemplate));
	aclTemplate.index = 5U;
	aclTemplate.fieldType[0] = ACL_FIELD_IPV6_SIP0;
	aclTemplate.fieldType[1] = ACL_FIELD_IPV6_SIP1;
	aclTemplate.fieldType[2] = ACL_FIELD_IPV6_SIP2;
	aclTemplate.fieldType[3] = ACL_FIELD_IPV6_SIP3;
	aclTemplate.fieldType[4] = ACL_FIELD_IPV6_SIP4;
	aclTemplate.fieldType[5] = ACL_FIELD_IPV6_SIP5;
	aclTemplate.fieldType[6] = ACL_FIELD_IPV6_SIP6;
	aclTemplate.fieldType[7] = ACL_FIELD_IPV6_SIP7;
	if(rtk_acl_template_set(&aclTemplate))WARNING("init ACL failed!!!\n");

	//aclTemplate[6]:DIPv6
	bzero((void*) &aclTemplate, sizeof(aclTemplate));
	aclTemplate.index = 6U;
	aclTemplate.fieldType[0] = ACL_FIELD_IPV6_DIP0;
	aclTemplate.fieldType[1] = ACL_FIELD_IPV6_DIP1;
	aclTemplate.fieldType[2] = ACL_FIELD_IPV6_DIP2;
	aclTemplate.fieldType[3] = ACL_FIELD_IPV6_DIP3;
	aclTemplate.fieldType[4] = ACL_FIELD_IPV6_DIP4;
	aclTemplate.fieldType[5] = ACL_FIELD_IPV6_DIP5;
	aclTemplate.fieldType[6] = ACL_FIELD_IPV6_DIP6;
	aclTemplate.fieldType[7] = ACL_FIELD_IPV6_DIP7;
	if(rtk_acl_template_set(&aclTemplate))WARNING("init ACL failed!!!\n");

	//aclTemplate[7]
	bzero((void*) &aclTemplate, sizeof(aclTemplate));
	aclTemplate.index = 7U;
	aclTemplate.fieldType[0] = ACL_FIELD_TCP_DPORT;
	aclTemplate.fieldType[1] = ACL_FIELD_TCP_SPORT;
	aclTemplate.fieldType[2] = ACL_FIELD_UDP_DPORT;
	aclTemplate.fieldType[3] = ACL_FIELD_UDP_SPORT;
	aclTemplate.fieldType[4] = ACL_FIELD_IPV4_SIP0_INNER;
	aclTemplate.fieldType[5] = ACL_FIELD_IPV4_SIP1_INNER;
	aclTemplate.fieldType[6] = ACL_FIELD_IPV4_DIP0_INNER;
	aclTemplate.fieldType[7] = ACL_FIELD_IPV4_DIP1_INNER;
	ASSERT_EQ(rtk_acl_template_set(&aclTemplate),RT_ERR_OK);

	/*init IP_RNG_TABLE*/
	bzero((void*) &iprangeEntry, sizeof(iprangeEntry));
	for(i =0; i<MAX_ACL_IPRANGETABLE_SIZE; i++)
	{
		iprangeEntry.index=i;
		if(rtk_acl_ipRange_set(&iprangeEntry))WARNING("init ACL failed!!!\n");
	}

	/*init PORT_RNG_TABLE*/
	bzero((void*) &prtRangeEntry, sizeof(prtRangeEntry));
	for(i =0; i<MAX_ACL_PORTRANGETABLE_SIZE; i++)
	{
		prtRangeEntry.index=i;
		if(rtk_acl_portRange_set(&prtRangeEntry))WARNING("init ACL failed!!!\n");
	}

	/*init PKTLEN_RNG_TABLE*/
	bzero((void*) &pktlenRangeEntry, sizeof(pktlenRangeEntry));
	for(i =0; i<MAX_ACL_PACKETRANGETABLE_SIZE; i++)
	{
		pktlenRangeEntry.index=i;
		if(rtk_acl_packetLengthRange_set(&pktlenRangeEntry))WARNING("init ACL failed!!!\n");
	}

	/*init STAT_ACL_CNT_MODE, STAT_ACL_CNT_TYPE*/
	bzero((void*) &logCtrl, sizeof(logCtrl));
	logCtrl.type = STAT_LOG_TYPE_BYTECNT;
	logCtrl.mode = STAT_LOG_MODE_64BITS;
	for(i = 0; i < MAX_ACL_LOG_MIB_NUM; i+=2)
		if(rtk_stat_logCtrl_set(i, logCtrl))WARNING("init ACL failed!!!\n");

	/*init STAT_ACL_CNT_RST*/
	for(i = 0; i < MAX_ACL_LOG_MIB_NUM; i+=2)
		if(rtk_stat_log_reset(i))WARNING("init ACL failed!!!\n");

	return (RT_ERR_RG_OK);
}

int _rtk_fc_aclReservedEntry_init(void)
{
	fc_db.aclAndCfReservedRule.aclValidTableSize = (MAX_ACL_ENTRY_SIZE);			/* fc acl rule = acl total size */

	fc_db.aclAndCfReservedRule.aclProtectLowerBoundary = 0U;										/* protection rules start idx */
	fc_db.aclAndCfReservedRule.aclProtectUpperBoundary = 0U;										/* protection rules end idx */
																								/* acl reserve rule - head */
	fc_db.aclAndCfReservedRule.aclLowerBoundary = 0U;											/* acl reserve rule - user start idx */
	fc_db.aclAndCfReservedRule.aclUpperBoundary = (fc_db.aclAndCfReservedRule.aclValidTableSize-1);	/* acl reserve rule - user end idx */
																								/* acl reserve rule - tail */

	return (RT_ERR_RG_OK);
}

/*(2)RG_ACL APIs and internal APIs*/
static int _rtk_fc_aclHWEntry_not_support(rt_acl_filterAndQos_t *acl_filter)
{	
	//HW ACL set ipv4/ipv6 l4_protocal in different template field, so need to indicate ipv4 or ipv6. (supporting HW ACL drop)
	if(acl_filter->filter_fields & RT_ACL_INGRESS_L4_POROTCAL_VALUE_BIT)
	{
		if(((acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_TAGIF_BIT)==0x0 && (acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_TAGIF_BIT)==0x0)
			 || (acl_filter->ingress_ipv4_tagif==0 && acl_filter->ingress_ipv6_tagif==0)){
			WARNING("RT_ACL_INGRESS_L4_POROTCAL_VALUE_BIT should indicate ipv4 or ipv6, please also assign RT_ACL_INGRESS_IPV4_TAGIF_BIT or RT_ACL_INGRESS_IPV6_TAGIF_BIT.");
			return (RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}

	//only support one format(untag/stag,catg/sctag) at the same time (by field selector 6&7)
	if((acl_filter->filter_fields & RT_ACL_INGRESS_ARP_TARGET_IP_BIT) && (fc_db.controlFuc.acl_arp_targetip_offset))
	{
		uint32 arpOffset = 1;
		if((acl_filter->filter_fields & RT_ACL_INGRESS_STAGIF_BIT) && (acl_filter->ingress_stagif == 0) &&
			(acl_filter->filter_fields & RT_ACL_INGRESS_CTAGIF_BIT) && (acl_filter->ingress_ctagif == 0)){
			//untag
		}else{
			if(acl_filter->filter_fields & (RT_ACL_INGRESS_STAGIF_BIT|RT_ACL_INGRESS_STAG_VID_BIT|RT_ACL_INGRESS_STAG_PRI_BIT|RT_ACL_INGRESS_STAG_DEI_BIT))
				arpOffset += 4;
			if(acl_filter->filter_fields & (RT_ACL_INGRESS_CTAGIF_BIT|RT_ACL_INGRESS_CTAG_VID_BIT|RT_ACL_INGRESS_CTAG_PRI_BIT|RT_ACL_INGRESS_CTAG_CFI_BIT))
				arpOffset += 4;
		}
		if(arpOffset != fc_db.controlFuc.acl_arp_targetip_offset){
			WARNING("RT_ACL_INGRESS_ARP_TARGET_IP_BIT only support one format(untag/stag or catg/sctag) at the same time, already have %s rule.",
				((fc_db.controlFuc.acl_arp_targetip_offset==1)?"untag":((fc_db.controlFuc.acl_arp_targetip_offset==(8+1))?"2tags":("1tag"))));
			return (RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}

	//Packet parser not support due to stag parsing earler than ctag
	if((acl_filter->filter_fields & RT_ACL_INGRESS_STAGIF_BIT) && (acl_filter->ingress_stagif == 0) && RT_ACL_PATTERN_IS_STPID_EQ_CTPID &&
		(((acl_filter->filter_fields & RT_ACL_INGRESS_CTAGIF_BIT) && (acl_filter->ingress_ctagif != 0)) ||
		(acl_filter->filter_fields & (RT_ACL_INGRESS_CTAG_VID_BIT|RT_ACL_INGRESS_CTAG_PRI_BIT|RT_ACL_INGRESS_CTAG_CFI_BIT))))
	{
		WARNING("RT_ACL pattern ingress_stagif=0 + ctag_xxx is not support when stag tpid the same as ctag tpid.");
		return (RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	//ACL trap > ACL policing drop
	if((acl_filter->action_fields&RT_ACL_ACTION_FORWARD_GROUP_TRAP_BIT) && (acl_filter->action_fields&RT_ACL_ACTION_METER_GROUP_BITS))
	{
		WARNING("METER action should not work along with trap action(ACL trap > ACL policing drop).");
		return (RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	if((acl_filter->action_fields&RT_ACL_ACTION_FORWARD_GROUP_TRAP_TO_PS_BIT))
	{
		WARNING("HW ACL not support TRAP TO PS action.");
		return (RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	//FLOW hit/miss > ACL redirect
	if(acl_filter->action_fields&RT_ACL_ACTION_FORWARD_GROUP_REDIRECT_BIT)
	{
		if((acl_filter->filter_fields&RT_ACL_CARE_PORT_UNITYPE_PPTP_BIT)==0x0)
		{
			WARNING("REDIRECT action should enable CARE_PORT_UNITYPE_PPTP at the same time(flow decision > ACL decision).");
			return (RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
		if(acl_filter->ingress_port_mask & ~fc_db.hypridPPTP.portmask)
		{
			WARNING("REDIRECT action only support uni pptp portmask(ingress_port_mask=0x%x, hypridPPTP=0x%llx).", acl_filter->ingress_port_mask, fc_db.hypridPPTP.portmask);
			return (RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}

	if((acl_filter->action_fields&RT_ACL_ACTION_LOGGING_GROUP_MIB_BIT) && (acl_filter->action_logging_group_mib_index*2 >= MAX_ACL_LOG_MIB_NUM))
	{
		WARNING("Logging action only support mib index 0~%d, not support %d.", MAX_ACL_LOG_MIB_NUM-1, acl_filter->action_logging_group_mib_index*2);
		return (RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
	return (RT_ERR_RG_OK);
}

static int _rtk_fc_acl_portmask_translator(rt_acl_port_mask_t port_mask, rtk_portmask_t* mac_pmsk)
{
	int i;

	if(mac_pmsk==NULL )
		return (RT_ERR_RG_NULL_POINTER);

	bzero(mac_pmsk,sizeof(rtk_portmask_t));

	for(i=0;i<RT_PORT_MAX;i++){
		if((port_mask & (1<<i))==0x0)
			continue;
		mac_pmsk->bits[0] |= (1<<i);
	}

	return (RT_ERR_RG_OK);
}

static int _rtk_fc_acl_wlan_mbssid_mask_translator(rt_acl_wlan_mbssid_mask_t mbssid_mask[], int size, rtk_portmask_t* ext_pmsk, rtk_portmask_t* acl_ext_pmsk)
{
	int i, j, band = 0, sharedSSID = 0;
	rtk_fc_wlan_devidx_t wlanDevIdx;
	rtk_fc_wlan_devmask_t wlanDevMask, checkDoneMask = 0ULL;
	rtk_fc_wlan_devmap_t *wlanDevMap = NULL;
	rtk_fc_mac_port_idx_t macPortIdx;
	rtk_fc_mac_ext_port_idx_t macExtPortIdx;

	if(ext_pmsk==NULL )
		return (RT_ERR_RG_NULL_POINTER);

	bzero(ext_pmsk,sizeof(rtk_portmask_t)*size);
	bzero(acl_ext_pmsk,sizeof(rtk_portmask_t)*size);

	for(j=0;j<size;j++){
		for(i=0;i<RT_WLAN_MBSSID_MAX;i++){
			if((mbssid_mask[j] & (1<<i))==0x0)
				continue;

			wlanDevIdx = rtk_fc_wlan_rtmbssid_to_devidx(j, i);

			if(checkDoneMask & (1ULL<<wlanDevIdx))
				continue;

			if(RTK_FC_HELPER_WLAN_DEVMAP_GET(wlanDevIdx, &wlanDevMap) == SUCCESS) {
				macPortIdx = RTK_FC_HELPER_WLAN_DEVMAP_MACPORTIDX_GET(wlanDevMap);
				macExtPortIdx = RTK_FC_HELPER_WLAN_DEVMAP_MACEXTPORTIDX_GET(wlanDevMap);
				if(RTK_FC_HELPER_WLAN_DEVMAP_SHAREEXTPORT_GET(wlanDevMap) == TRUE) {
					RTK_FC_HELPER_WLAN_PORT_TO_DEVID(macPortIdx, macExtPortIdx, &wlanDevIdx, &wlanDevMask);
					checkDoneMask |= wlanDevMask;

					// TODO: consider different band but share same cpu/ext port.
					// check all wlan dev is configured, return fail if not.
					for(sharedSSID=0; sharedSSID<RT_WLAN_MBSSID_MAX; sharedSSID++) {
						if((mbssid_mask[j] & (1<<sharedSSID))==0x0)
							continue;

						wlanDevIdx = rtk_fc_wlan_rtmbssid_to_devidx(j, sharedSSID);

						if(wlanDevIdx == RTK_FC_WLANX_NOT_FOUND) {
							WARNING("This chip not support mbssid %d", sharedSSID);
							return (RT_ERR_RG_CHIP_NOT_SUPPORT);
						}
						
						wlanDevMask &= ~(1ULL<<wlanDevIdx);
					}

					if(wlanDevMask!=0ULL) {
						WARNING("Some mbssid (%llx) in wlan%d belongs to others, you should config all", wlanDevMask, j);
						return (RT_ERR_RG_CHIP_NOT_SUPPORT);
					}
				}

				if(macPortIdx== RTK_FC_MAC_PORT_MASTERCPU_CORE0)
					band = 0;
				else if(macPortIdx == RTK_FC_MAC_PORT_MASTERCPU_CORE1)
					band = 1;
				else if(macPortIdx == RTK_FC_MAC_PORT_SLAVECPU)
					band = 2;

				ext_pmsk[band].bits[0] |= (1<<macExtPortIdx);
			}else {

				WARNING("This chip not support wlan%d mbssid %d", j, i);
				return (RT_ERR_RG_CHIP_NOT_SUPPORT);
			}

		}

		for(i=RTK_FC_MAC_EXT_PORT0;i<RTK_FC_MAC_EXT_PORT_MAX;i++){
			if(!(ext_pmsk[j].bits[0]&(1<<i))){
				acl_ext_pmsk[j].bits[0] |= (1<<i); //band not allowed ext_port
			}
		}
	}

	return (RT_ERR_RG_OK);
}


static int _rtk_fc_search_acl_exist_ipTableEntry(uint32* index,ipaddr_t upperIp,ipaddr_t lowerIp,rtk_acl_iprange_t type)
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

static int _rtk_fc_search_acl_exist_tableEntry(uint32* index,uint32 table_type,uint16 upper_bound,uint16 lower_bound,uint32 type)
{
	int i;
	int ret = -1;
	rtk_acl_rangeCheck_l4Port_t aclPortRangeEntry;
	rtk_acl_rangeCheck_pktLength_t aclPktLengthRangeEntry;

    bzero(&aclPortRangeEntry, sizeof(aclPortRangeEntry));
    bzero(&aclPktLengthRangeEntry, sizeof(aclPktLengthRangeEntry));

	switch(table_type) {
		case RTK_FC_ACL_USED_INGRESS_SPORTTABLE_INDEX:
		case RTK_FC_ACL_USED_INGRESS_DPORTTABLE_INDEX:
			for(i=0;i<MAX_ACL_PORTRANGETABLE_SIZE;i++)
		    {
		        aclPortRangeEntry.index = i;
		        if(rtk_acl_portRange_get(&aclPortRangeEntry)){
		        	ACL_CTRL("access HW ACL PORT_RANGE table failed");
		            return RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED;
		        }else{
		            if(aclPortRangeEntry.type==type &&
						aclPortRangeEntry.upper_bound==upper_bound &&
						aclPortRangeEntry.lower_bound==lower_bound){
						 *index = i;
		                return (RT_ERR_RG_OK);
		            }else{
		                continue;
		            }
		        }
		    }
			ret = RT_ERR_RG_ACL_PORTTABLE_FULL;
			break;
		case RTK_FC_ACL_USED_INGRESS_PKTLENTABLE_INDEX:
			for(i=0;i<MAX_ACL_PACKETRANGETABLE_SIZE;i++)
		    {
		        aclPktLengthRangeEntry.index = i;
		        if(rtk_acl_packetLengthRange_get(&aclPktLengthRangeEntry)){
		        	ACL_CTRL("access HW ACL PACKET_LENGTH_RANGE table failed");
		            return RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED;
		        }else{
		            if(aclPktLengthRangeEntry.type==type &&
						aclPktLengthRangeEntry.upper_bound==upper_bound &&
						aclPktLengthRangeEntry.lower_bound==lower_bound){
						 *index = i;
		                return (RT_ERR_RG_OK);
		            }else{
		                continue;
		            }
		        }
		    }
			ret = RT_ERR_RG_ACL_PORTTABLE_FULL;
			break;
		default:
			break;
	}

	//not found exist entry
    return ret;
}

static int _rtk_fc_search_acl_empty_tableEntry(uint32* index, uint32 type)
{
    int i;
	int ret = RT_ERR_RG_OK;
    rtk_acl_rangeCheck_ip_t aclIpRangeEntry;
    rtk_acl_rangeCheck_l4Port_t aclPortRangeEntry;
	rtk_acl_rangeCheck_pktLength_t aclPktLengthRangeEntry;

    bzero(&aclIpRangeEntry, sizeof(aclIpRangeEntry));
    bzero(&aclPortRangeEntry, sizeof(aclPortRangeEntry));
    bzero(&aclPktLengthRangeEntry, sizeof(aclPktLengthRangeEntry));

	switch(type) {
		case RTK_FC_ACL_USED_INGRESS_SIP4TABLE_INDEX:
		case RTK_FC_ACL_USED_INGRESS_DIP4TABLE_INDEX:
			for(i=0;i<MAX_ACL_IPRANGETABLE_SIZE;i++){
				aclIpRangeEntry.index = i;
				if(rtk_acl_ipRange_get(&aclIpRangeEntry)){
					ACL_CTRL("access HW ACL IP_RANGE table failed");
					return RT_ERR_RG_ACL_IPTABLE_ACCESS_FAILED;
				}else{
					if(aclIpRangeEntry.type==IPRANGE_UNUSED){
						*index = i;
						return (RT_ERR_RG_OK);
					}else{
						continue;
					}
				}
			}
			ACL_CTRL("HW ACL IP_RANGE is full!");
			ret = RT_ERR_RG_ACL_IPTABLE_FULL;
			break;
		case RTK_FC_ACL_USED_INGRESS_SPORTTABLE_INDEX:
		case RTK_FC_ACL_USED_INGRESS_DPORTTABLE_INDEX:
			for(i=0;i<MAX_ACL_PORTRANGETABLE_SIZE;i++){
				aclPortRangeEntry.index = i;
				if(rtk_acl_portRange_get(&aclPortRangeEntry)){
					ACL_CTRL("access HW ACL PORT_RANGE table failed");
					return RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED;
				}else{
					if(aclPortRangeEntry.type==PORTRANGE_UNUSED){
						*index = i;
						return (RT_ERR_RG_OK);
					}else{
						continue;
					}
				}
			}
			ACL_CTRL("HW ACL PORT_RANGE is full!");
			ret = RT_ERR_RG_ACL_PORTTABLE_FULL;
			break;
		case RTK_FC_ACL_USED_INGRESS_PKTLENTABLE_INDEX:
			for(i=0;i<MAX_ACL_PACKETRANGETABLE_SIZE;i++){
				aclPktLengthRangeEntry.index = i;
				if(rtk_acl_packetLengthRange_get(&aclPktLengthRangeEntry)){
					ACL_CTRL("access HW ACL PACKET_LENGTH_RANGE table failed");
					return RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED;
				}else{
					if((aclPktLengthRangeEntry.type==PKTLENRANGE_NOTREVISE) &&
						(aclPktLengthRangeEntry.upper_bound==0) &&
						(aclPktLengthRangeEntry.lower_bound==0)){
						*index = i;
						return (RT_ERR_RG_OK);
					}else{
						continue;
					}
				}
			}
			break;
		default:
			break;
	}

    return ret;
}

static int _rtk_fc_search_acl_empty_Entry(int size, int* index)
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


static int _rtk_fc_aclSWEntry_to_asic_add(rt_acl_filterAndQos_t *acl_filter,rtk_fc_aclFilterEntry_t* aclSWEntry,int shareHwAclWithSWAclIdx)
{
	int i,j,index,ret;
	int aclIdx=FAIL;
	uint8 aclActionRuleIdx,aclCountinueRuleSize;
	uint8 firstHitFlag;
	rtk_portmask_t mac_pmsk;
	rtk_portmask_t ext_pmsk[RT_WLAN_DEVICE_MAX];	//[0] for master cpu core0, [1] for master cpu core1, [2] for slave cpu
	rtk_portmask_t acl_ext_pmsk[RT_WLAN_DEVICE_MAX];	//transform ext_pmsk to HW ACL format
	rtk_portmask_t cpu_ext_pmsk[RT_WLAN_DEVICE_MAX];	//cpu port mapping with ext_pmsk
	uint8 wlan_count=RT_WLAN_DEVICE_MAX;
	uint8 portmask_need_double_hw_rule=0U;
	uint8 portmask_need_double_hw_rule_wlan_index=wlan_count;
	uint8 portmask_need_double_hw_rule_add_sencond=0U;
	int ipv4_unmask_length=0;
	rt_rate_meter_mapping_t meterMapping;


	uint8 aclRuleValid[GLOBAL_ACL_RULE_SIZE];
	rtk_acl_field_t aclFieldEmpty;
	rtk_acl_ingress_entry_t aclRuleEmpty;
	rtk_acl_rangeCheck_ip_t aclSIPRangeEntry,aclDIPRangeEntry;
	rtk_acl_rangeCheck_l4Port_t aclSportRangeEntry,aclDportRangeEntry;
	rtk_acl_rangeCheck_pktLength_t aclPktLengthRangeEntry;	

	aclSWEntry->hw_aclEntry_start = 0U;
	aclSWEntry->hw_aclEntry_size = 0U;

	if(shareHwAclWithSWAclIdx >= 0)	//for debug acl trap by which rt acl index
		shareHwAclWithSWAclIdx = RTK_FC_ACLANDCF_RESERVED_TAIL_END + shareHwAclWithSWAclIdx;
	else
		shareHwAclWithSWAclIdx = FAIL;//pre-add test

	/*Skip ----------------- fwding_type_and_direction/HW_ACL_SUPPORT_PATTERN check ----------------- Skip*/

	ASSERT_EQ(_rtk_fc_acl_portmask_translator(acl_filter->ingress_port_mask, &mac_pmsk),RT_ERR_RG_OK);
	if(acl_filter->filter_fields & RT_ACL_INGRESS_WLAN_MBSSID_MASK_BIT) {
		ASSERT_EQ(_rtk_fc_acl_wlan_mbssid_mask_translator(acl_filter->ingress_wlan_mbssid_mask, wlan_count, ext_pmsk, acl_ext_pmsk),RT_ERR_RG_OK);
		for(i=0; i<wlan_count; i++) {	//Get the first none zero wlan mbssid mask
			if(i==RT_WLAN_DEVICE0_INDEX)
				cpu_ext_pmsk[i].bits[0] = (1U<<RTK_FC_MAC_PORT_MASTERCPU_CORE0);
			else if(i==RT_WLAN_DEVICE1_INDEX)
				cpu_ext_pmsk[i].bits[0] = (1U<<RTK_FC_MAC_PORT_MASTERCPU_CORE1);
			else if(i==RT_WLAN_DEVICE2_INDEX){
				cpu_ext_pmsk[i].bits[0] = (1U<<RTK_FC_MAC_PORT_SLAVECPU);
				if((acl_filter->ingress_wlan_mbssid_mask[i]!=RT_WLAN_MBSSID_ALL_BIT) && (ext_pmsk[i].bits[0]) && (acl_filter->ingress_port_mask&(1<<RT_PORT7)))
					WARNING("[Attentation] Please not use port %d if you also want to control wlan device index %d", RT_PORT7, i);
			}else
				WARNING("Not support wlan device index %d", i);
		}
		for(i=0; i<wlan_count; i++) {	//Get the first (none zero)&(not all) wlan mbssid mask
			if((acl_filter->ingress_wlan_mbssid_mask[i]!=RT_WLAN_MBSSID_ALL_BIT) && (ext_pmsk[i].bits[0])) {
				portmask_need_double_hw_rule_wlan_index = i;
				break;
			}
		}
	}
	else
		bzero(ext_pmsk,sizeof(rtk_portmask_t)*wlan_count);

	//clean portmask_need_double_hw_rule record, if TRANSFROM need to add more then two set of ACLs.
	portmask_need_double_hw_rule=0U;
	portmask_need_double_hw_rule_add_sencond=0U;

	//portmask include mac_ports and ext_ports, need double ACLs due to EXTPORT pattern
	if((mac_pmsk.bits[0]!=0x0U) && (portmask_need_double_hw_rule_wlan_index < wlan_count))
	{
		ACL_CTRL("rule include ingress_port_mask=0x%x and ingress_wlan_mbssid_mask(wlan0/1/2)=0x%x/0x%x/0x%x",acl_filter->ingress_port_mask,acl_filter->ingress_wlan_mbssid_mask[RT_WLAN_DEVICE0_INDEX],acl_filter->ingress_wlan_mbssid_mask[RT_WLAN_DEVICE1_INDEX],acl_filter->ingress_wlan_mbssid_mask[RT_WLAN_DEVICE2_INDEX]);
		portmask_need_double_hw_rule=1U;
	}

PORTMASK_NEED_DOUBLE_RULE:
	aclCountinueRuleSize=0;

	//init global parameters
	bzero(aclRuleValid,sizeof(uint8)*GLOBAL_ACL_RULE_SIZE);
	bzero(&aclFieldEmpty,sizeof(rtk_acl_field_t));
	bzero(fc_db.aclField,sizeof(rtk_acl_field_t)*GLOBAL_ACL_FIELD_SIZE);
	bzero(fc_db.aclRule,sizeof(rtk_acl_ingress_entry_t)*GLOBAL_ACL_RULE_SIZE);
	bzero(&aclRuleEmpty,sizeof(rtk_acl_ingress_entry_t));



	/*set patterns*/

	if((/*mac_port + ext_port, second time add mac_port only*/ portmask_need_double_hw_rule && portmask_need_double_hw_rule_add_sencond)
		|| (/*OR mac_port only*/ (portmask_need_double_hw_rule==0) && (mac_pmsk.bits[0])))
	{
		//add active port in all related fc_db.aclRule
		for(i=0; i<GLOBAL_ACL_RULE_SIZE; i++)
		{
			if(acl_filter->ingress_port_mask == RTK_FC_ALL_MAC_PORTMASK){
				fc_db.aclRule[i].activePorts.bits[0] = RTK_FC_ALL_MAC_PORTMASK;
				continue;
			}

			fc_db.aclRule[i].activePorts = mac_pmsk;
			for(j=0; j<wlan_count; j++)
			{
				if(acl_filter->ingress_wlan_mbssid_mask[j]==RT_WLAN_MBSSID_ALL_BIT)
					fc_db.aclRule[i].activePorts.bits[0] |= cpu_ext_pmsk[j].bits[0];
			}
		}
		ACL_CTRL("= set acl rule for mac_port=0x%x =",fc_db.aclRule[0].activePorts.bits[0]);
	}

	if((/*mac_port + ext_port, first time add ext_port only*/portmask_need_double_hw_rule && (portmask_need_double_hw_rule_add_sencond==0))
		|| (/*OR ext_port only*/ (portmask_need_double_hw_rule==0) && (portmask_need_double_hw_rule_wlan_index < wlan_count)))
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_EXTPORTMASK].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_EXTPORTMASK].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_EXTPORTMASK%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_EXTPORTMASK].fieldUnion.data.value = 0x0U;
		fc_db.aclField[RTK_ACL_TEMPLATE_EXTPORTMASK].fieldUnion.data.mask = acl_ext_pmsk[portmask_need_double_hw_rule_wlan_index].bits[0]; //band not allowed ext_port

		for(i=0; i<GLOBAL_ACL_RULE_SIZE; i++)
		{
			fc_db.aclRule[i].activePorts.bits[0] = cpu_ext_pmsk[portmask_need_double_hw_rule_wlan_index].bits[0];
			for(j=(portmask_need_double_hw_rule_wlan_index+1); j<wlan_count; j++)
			{
				if(ext_pmsk[j].bits[0] && (ext_pmsk[j].bits[0] == ext_pmsk[portmask_need_double_hw_rule_wlan_index].bits[0]))
					fc_db.aclRule[i].activePorts.bits[0] |= cpu_ext_pmsk[j].bits[0];
			}
		}
		ACL_CTRL("= set acl rule for not allow ext_port=0x%x, and mac_port=0x%x =",fc_db.aclField[RTK_ACL_TEMPLATE_EXTPORTMASK].fieldUnion.data.mask, fc_db.aclRule[0].activePorts.bits[0]);
	}

	if(acl_filter->filter_fields & RT_ACL_CARE_PORT_UNITYPE_PPTP_BIT) {
		if(fc_db.hypridPPTP.portmask)
			ACL_CTRL("= care pptp_port=0x%llx, so mac_port=0x%x =",fc_db.hypridPPTP.portmask,fc_db.aclRule[0].activePorts.bits[0]);
	}else if(fc_db.hypridPPTP.portmask){
		//disable pptp port configuration in all related fc_db.aclRule
		for(i=0; i<GLOBAL_ACL_RULE_SIZE; i++)
		{
			fc_db.aclRule[i].activePorts.bits[0] &= (~fc_db.hypridPPTP.portmask);
		}
		ACL_CTRL("= disable acl rule for pptp_port=0x%llx, so mac_port=0x%x =",fc_db.hypridPPTP.portmask,fc_db.aclRule[0].activePorts.bits[0]);
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_SMAC_BIT)
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_SMAC0].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_SMAC0].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_SMAC0%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_SMAC0].fieldUnion.data.value = ((acl_filter->ingress_smac[4]<<8) | (acl_filter->ingress_smac[5]));
		fc_db.aclField[RTK_ACL_TEMPLATE_SMAC0].fieldUnion.data.mask = ((acl_filter->ingress_smac_mask[4]<<8) | (acl_filter->ingress_smac_mask[5]));

		fc_db.aclField[RTK_ACL_TEMPLATE_SMAC1].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_SMAC1].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_SMAC1%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_SMAC1].fieldUnion.data.value =((acl_filter->ingress_smac[2]<<8) | (acl_filter->ingress_smac[3]));
		fc_db.aclField[RTK_ACL_TEMPLATE_SMAC1].fieldUnion.data.mask = ((acl_filter->ingress_smac_mask[2]<<8) | (acl_filter->ingress_smac_mask[3]));

		fc_db.aclField[RTK_ACL_TEMPLATE_SMAC2].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_SMAC2].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_SMAC2%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_SMAC2].fieldUnion.data.value =((acl_filter->ingress_smac[0]<<8) | (acl_filter->ingress_smac[1]));
		fc_db.aclField[RTK_ACL_TEMPLATE_SMAC2].fieldUnion.data.mask = ((acl_filter->ingress_smac_mask[0]<<8) | (acl_filter->ingress_smac_mask[1]));
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_DMAC_BIT)
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_DMAC0].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_DMAC0].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_DMAC0%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_DMAC0].fieldUnion.data.value =((acl_filter->ingress_dmac[4]<<8) | (acl_filter->ingress_dmac[5]));
		fc_db.aclField[RTK_ACL_TEMPLATE_DMAC0].fieldUnion.data.mask = ((acl_filter->ingress_dmac_mask[4]<<8) | (acl_filter->ingress_dmac_mask[5]));

		fc_db.aclField[RTK_ACL_TEMPLATE_DMAC1].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_DMAC1].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_DMAC1%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_DMAC1].fieldUnion.data.value =((acl_filter->ingress_dmac[2]<<8) | (acl_filter->ingress_dmac[3]));
		fc_db.aclField[RTK_ACL_TEMPLATE_DMAC1].fieldUnion.data.mask = ((acl_filter->ingress_dmac_mask[2]<<8) | (acl_filter->ingress_dmac_mask[3]));

		fc_db.aclField[RTK_ACL_TEMPLATE_DMAC2].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_DMAC2].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_DMAC2%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_DMAC2].fieldUnion.data.value =((acl_filter->ingress_dmac[0]<<8) | (acl_filter->ingress_dmac[1]));
		fc_db.aclField[RTK_ACL_TEMPLATE_DMAC2].fieldUnion.data.mask =((acl_filter->ingress_dmac_mask[0]<<8) | (acl_filter->ingress_dmac_mask[1]));
	}


	if(acl_filter->filter_fields & RT_ACL_INGRESS_ETHERTYPE_BIT)
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_ETHERTYPE].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_ETHERTYPE].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_ETHERTYPE%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_ETHERTYPE].fieldUnion.data.value = acl_filter->ingress_ethertype;
		fc_db.aclField[RTK_ACL_TEMPLATE_ETHERTYPE].fieldUnion.data.mask = acl_filter->ingress_ethertype_mask;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_STAGIF_BIT)
	{
		for(i=0; i<GLOBAL_ACL_RULE_SIZE; i++)
		{
			if(acl_filter->ingress_stagif)
				fc_db.aclRule[i].careTag.tags[ACL_CARE_TAG_STAG].value=1U;
			else
				fc_db.aclRule[i].careTag.tags[ACL_CARE_TAG_STAG].value=0U;

			fc_db.aclRule[i].careTag.tags[ACL_CARE_TAG_STAG].mask=0xffffU;
		}
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_STAG_VID_BIT)
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_STAG].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_STAG].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_STAG%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_STAG].fieldUnion.data.value |= (acl_filter->ingress_stag_vid);
		fc_db.aclField[RTK_ACL_TEMPLATE_STAG].fieldUnion.data.mask |= 0x0fffU;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_STAG_PRI_BIT)
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_STAG].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_STAG].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_STAG%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_STAG].fieldUnion.data.value |= ((acl_filter->ingress_stag_pri)<<13);
		fc_db.aclField[RTK_ACL_TEMPLATE_STAG].fieldUnion.data.mask |= 0xe000U;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_STAG_DEI_BIT)
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_STAG].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_STAG].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_STAG%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_STAG].fieldUnion.data.value |= ((acl_filter->ingress_stag_dei)<<12);
		fc_db.aclField[RTK_ACL_TEMPLATE_STAG].fieldUnion.data.mask |= 0x1000U;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAGIF_BIT)
	{
		for(i=0; i<GLOBAL_ACL_RULE_SIZE; i++)
		{
			if(acl_filter->ingress_ctagif)
				fc_db.aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].value=1U;
			else
				fc_db.aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].value=0U;

			fc_db.aclRule[i].careTag.tags[ACL_CARE_TAG_CTAG].mask=0xffffU;
		}
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_VID_BIT)
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_CTAG].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_CTAG].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_CTAG%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_CTAG].fieldUnion.data.value |= (acl_filter->ingress_ctag_vid);
		fc_db.aclField[RTK_ACL_TEMPLATE_CTAG].fieldUnion.data.mask |= 0x0fffU;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_PRI_BIT)
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_CTAG].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_CTAG].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_CTAG%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_CTAG].fieldUnion.data.value |= ((acl_filter->ingress_ctag_pri)<<13);
		fc_db.aclField[RTK_ACL_TEMPLATE_CTAG].fieldUnion.data.mask |= 0xe000U;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_CFI_BIT)
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_CTAG].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_CTAG].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_CTAG%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_CTAG].fieldUnion.data.value |= ((acl_filter->ingress_ctag_cfi)<<12);
		fc_db.aclField[RTK_ACL_TEMPLATE_CTAG].fieldUnion.data.mask |= 0x1000U;
	}


	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_TAGIF_BIT )
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS%ACL_PER_RULE_FIELD_SIZE);
		if(acl_filter->ingress_ipv4_tagif==0)//must not IPv4
			fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.value &= ~ACL_TAGIF_OUT_IP4_BIT; //out IP4 bit
		else//must be IPv4
			fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.value |= ACL_TAGIF_OUT_IP4_BIT; //out IP4 bit

		fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.mask |=ACL_TAGIF_OUT_IP4_BIT; //out IP4 bit
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_SIP_RANGE_BIT)
	{
		if(acl_filter->ingress_src_ipv4_addr_start == acl_filter->ingress_src_ipv4_addr_end ){
			//SINGLE IP
			fc_db.aclField[RTK_ACL_TEMPLATE_IPv4SIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
			fc_db.aclField[RTK_ACL_TEMPLATE_IPv4SIP0].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv4SIP0%ACL_PER_RULE_FIELD_SIZE);
			fc_db.aclField[RTK_ACL_TEMPLATE_IPv4SIP0].fieldUnion.data.value = (acl_filter->ingress_src_ipv4_addr_start & 0xffff);//SIP[15:0]
			fc_db.aclField[RTK_ACL_TEMPLATE_IPv4SIP0].fieldUnion.data.mask = 0xffffU;

			fc_db.aclField[RTK_ACL_TEMPLATE_IPv4SIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
			fc_db.aclField[RTK_ACL_TEMPLATE_IPv4SIP1].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv4SIP1%ACL_PER_RULE_FIELD_SIZE);
			fc_db.aclField[RTK_ACL_TEMPLATE_IPv4SIP1].fieldUnion.data.value = (acl_filter->ingress_src_ipv4_addr_start & 0xffff0000)>>16;//SIP[31:16]
			fc_db.aclField[RTK_ACL_TEMPLATE_IPv4SIP1].fieldUnion.data.mask = 0xffffU;

		}else{
			//IP RANGE
			int i;
			uint32 mask=0;
			ret = _rtk_fc_aclHWEntry_maskLength_get_by_ipv4_range(acl_filter->ingress_src_ipv4_addr_start,acl_filter->ingress_src_ipv4_addr_end,&ipv4_unmask_length);
			if(ret == RT_ERR_RG_OK)
			{
				//build IPv4 Range mask
				for(i=0;i<32;i++){
					if(i>=ipv4_unmask_length){mask|=(1<<i);}
				}

				fc_db.aclField[RTK_ACL_TEMPLATE_IPv4SIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
				fc_db.aclField[RTK_ACL_TEMPLATE_IPv4SIP0].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv4SIP0%ACL_PER_RULE_FIELD_SIZE);
				fc_db.aclField[RTK_ACL_TEMPLATE_IPv4SIP0].fieldUnion.data.value = (acl_filter->ingress_src_ipv4_addr_start & 0xffff);//SIP[15:0]
				fc_db.aclField[RTK_ACL_TEMPLATE_IPv4SIP0].fieldUnion.data.mask = (mask&0x0000ffff);

				fc_db.aclField[RTK_ACL_TEMPLATE_IPv4SIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
				fc_db.aclField[RTK_ACL_TEMPLATE_IPv4SIP1].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv4SIP1%ACL_PER_RULE_FIELD_SIZE);
				fc_db.aclField[RTK_ACL_TEMPLATE_IPv4SIP1].fieldUnion.data.value = (acl_filter->ingress_src_ipv4_addr_start & 0xffff0000)>>16;//SIP[31:16]
				fc_db.aclField[RTK_ACL_TEMPLATE_IPv4SIP1].fieldUnion.data.mask = (mask >> 16);
			}
			else
			{
				uint32 pattern_idx=0;
				if(_rtk_fc_search_acl_exist_ipTableEntry(&pattern_idx,acl_filter->ingress_src_ipv4_addr_end,acl_filter->ingress_src_ipv4_addr_start,IPRANGE_IPV4_SIP)==RT_ERR_RG_OK)
				{
					//found there is exist the same port_range entry can be share
					ACL_CTRL("Found acl IpRange[%d] can be share with this rule.",pattern_idx);
				}
				else
				{
					ret = _rtk_fc_search_acl_empty_tableEntry(&pattern_idx,RTK_FC_ACL_USED_INGRESS_SIP4TABLE_INDEX);
					if(ret!=RT_ERR_RG_OK) return ret;
				}

				bzero(&aclSIPRangeEntry, sizeof(aclSIPRangeEntry));
				aclSIPRangeEntry.index = pattern_idx;
				aclSIPRangeEntry.upperIp=acl_filter->ingress_src_ipv4_addr_end;
				aclSIPRangeEntry.lowerIp=acl_filter->ingress_src_ipv4_addr_start;
				aclSIPRangeEntry.type = IPRANGE_IPV4_SIP;

				fc_db.aclField[RTK_ACL_TEMPLATE_IP_RANGE].fieldType = ACL_FIELD_PATTERN_MATCH;
				fc_db.aclField[RTK_ACL_TEMPLATE_IP_RANGE].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IP_RANGE%ACL_PER_RULE_FIELD_SIZE);
				fc_db.aclField[RTK_ACL_TEMPLATE_IP_RANGE].fieldUnion.data.value |= (1<<(pattern_idx));
				fc_db.aclField[RTK_ACL_TEMPLATE_IP_RANGE].fieldUnion.data.mask |= (1<<(pattern_idx));

				if(rtk_acl_ipRange_set(&aclSIPRangeEntry))
				{
					ACL_CTRL("access HW ACL IP_RANGE table failed");
					return(RT_ERR_RG_ACL_IPTABLE_ACCESS_FAILED);
				}
				else
				{
					ACL_CTRL("use HW ACL IP_RANGE[%d]",pattern_idx);
					aclSWEntry->hw_used_table |= RTK_FC_ACL_USED_INGRESS_SIP4TABLE;
					aclSWEntry->hw_used_table_index[RTK_FC_ACL_USED_INGRESS_SIP4TABLE_INDEX] = pattern_idx;
				}
			}
		}

	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_DIP_RANGE_BIT)
	{
		if(acl_filter->ingress_dest_ipv4_addr_start == acl_filter->ingress_dest_ipv4_addr_end ){
			//SINGLE IP
			fc_db.aclField[RTK_ACL_TEMPLATE_IPv4DIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
			fc_db.aclField[RTK_ACL_TEMPLATE_IPv4DIP0].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv4DIP0%ACL_PER_RULE_FIELD_SIZE);
			fc_db.aclField[RTK_ACL_TEMPLATE_IPv4DIP0].fieldUnion.data.value = (acl_filter->ingress_dest_ipv4_addr_start & 0xffff);//DIP[15:0]
			fc_db.aclField[RTK_ACL_TEMPLATE_IPv4DIP0].fieldUnion.data.mask = 0xffffU;

			fc_db.aclField[RTK_ACL_TEMPLATE_IPv4DIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
			fc_db.aclField[RTK_ACL_TEMPLATE_IPv4DIP1].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv4DIP1%ACL_PER_RULE_FIELD_SIZE);
			fc_db.aclField[RTK_ACL_TEMPLATE_IPv4DIP1].fieldUnion.data.value = (acl_filter->ingress_dest_ipv4_addr_start & 0xffff0000)>>16;//DIP[31:16]
			fc_db.aclField[RTK_ACL_TEMPLATE_IPv4DIP1].fieldUnion.data.mask = 0xffffU;

		}else{
			//IP RANGE
			int i;
			uint32 mask=0U;
			ret = _rtk_fc_aclHWEntry_maskLength_get_by_ipv4_range(acl_filter->ingress_dest_ipv4_addr_start,acl_filter->ingress_dest_ipv4_addr_end,&ipv4_unmask_length);
			if(ret == RT_ERR_RG_OK)
			{
				//build IPv4 Range mask
				for(i=0;i<32;i++){
					if(i>=ipv4_unmask_length){mask|=(1<<i);}
				}

				fc_db.aclField[RTK_ACL_TEMPLATE_IPv4DIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
				fc_db.aclField[RTK_ACL_TEMPLATE_IPv4DIP0].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv4DIP0%ACL_PER_RULE_FIELD_SIZE);
				fc_db.aclField[RTK_ACL_TEMPLATE_IPv4DIP0].fieldUnion.data.value = (acl_filter->ingress_dest_ipv4_addr_start & 0xffff);//DIP[15:0]
				fc_db.aclField[RTK_ACL_TEMPLATE_IPv4DIP0].fieldUnion.data.mask = (mask&0x0000ffff);

				fc_db.aclField[RTK_ACL_TEMPLATE_IPv4DIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
				fc_db.aclField[RTK_ACL_TEMPLATE_IPv4DIP1].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv4DIP1%ACL_PER_RULE_FIELD_SIZE);
				fc_db.aclField[RTK_ACL_TEMPLATE_IPv4DIP1].fieldUnion.data.value = (acl_filter->ingress_dest_ipv4_addr_start & 0xffff0000)>>16;//DIP[31:16]
				fc_db.aclField[RTK_ACL_TEMPLATE_IPv4DIP1].fieldUnion.data.mask = (mask >> 16);
			}
			else
			{
				uint32 pattern_idx=0U;
				if(_rtk_fc_search_acl_exist_ipTableEntry(&pattern_idx,acl_filter->ingress_dest_ipv4_addr_end,acl_filter->ingress_dest_ipv4_addr_start,IPRANGE_IPV4_DIP)==RT_ERR_RG_OK)
				{
					//found there is exist the same port_range entry can be share
					ACL_CTRL("Found acl IpRange[%d] can be share with this rule.",pattern_idx);
				}
				else
				{
					ret = _rtk_fc_search_acl_empty_tableEntry(&pattern_idx,RTK_FC_ACL_USED_INGRESS_DIP4TABLE_INDEX);
					if(ret!=RT_ERR_RG_OK) return ret;
				}

				bzero(&aclDIPRangeEntry, sizeof(aclDIPRangeEntry));
				aclDIPRangeEntry.index = pattern_idx;
				aclDIPRangeEntry.upperIp = acl_filter->ingress_dest_ipv4_addr_end;
				aclDIPRangeEntry.lowerIp = acl_filter->ingress_dest_ipv4_addr_start;
				aclDIPRangeEntry.type = IPRANGE_IPV4_DIP;

				fc_db.aclField[RTK_ACL_TEMPLATE_IP_RANGE].fieldType = ACL_FIELD_PATTERN_MATCH;
				fc_db.aclField[RTK_ACL_TEMPLATE_IP_RANGE].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IP_RANGE%ACL_PER_RULE_FIELD_SIZE);
				fc_db.aclField[RTK_ACL_TEMPLATE_IP_RANGE].fieldUnion.data.value |= (1<<(pattern_idx));
				fc_db.aclField[RTK_ACL_TEMPLATE_IP_RANGE].fieldUnion.data.mask |= (1<<(pattern_idx));

				if(rtk_acl_ipRange_set(&aclDIPRangeEntry))
				{
					ACL_CTRL("access HW ACL IP_RANGE table failed");
					return(RT_ERR_RG_ACL_IPTABLE_ACCESS_FAILED);
				}
				else
				{
					ACL_CTRL("use HW ACL IP_RANGE[%d]",pattern_idx);
					aclSWEntry->hw_used_table |= RTK_FC_ACL_USED_INGRESS_DIP4TABLE;
					aclSWEntry->hw_used_table_index[RTK_FC_ACL_USED_INGRESS_DIP4TABLE_INDEX] = pattern_idx;
				}
			}
		}

	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_DSCP_BIT)
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_IP4_TOS_PROTO].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_IP4_TOS_PROTO].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IP4_TOS_PROTO%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_IP4_TOS_PROTO].fieldUnion.data.value &=0xffU; //clear formet pattern
		fc_db.aclField[RTK_ACL_TEMPLATE_IP4_TOS_PROTO].fieldUnion.data.value |= (acl_filter->ingress_ipv4_dscp)<<(2+8); //ToS[15:8] + protocal[7:0]
		fc_db.aclField[RTK_ACL_TEMPLATE_IP4_TOS_PROTO].fieldUnion.data.mask |= 0xfc00U;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_TOS_BIT)
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_IP4_TOS_PROTO].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_IP4_TOS_PROTO].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IP4_TOS_PROTO%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_IP4_TOS_PROTO].fieldUnion.data.value &=0xffU; //clear formet pattern
		fc_db.aclField[RTK_ACL_TEMPLATE_IP4_TOS_PROTO].fieldUnion.data.value |= acl_filter->ingress_ipv4_tos<<8;//ToS[15:8] + protocal[7:0]
		fc_db.aclField[RTK_ACL_TEMPLATE_IP4_TOS_PROTO].fieldUnion.data.mask |= 0xff00U;
	}


	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_TAGIF_BIT)
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS%ACL_PER_RULE_FIELD_SIZE);
		if(acl_filter->ingress_ipv6_tagif==0)//must not IPv6
			fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.value &= ~ACL_TAGIF_IP6_BIT; //ipv6 bit
		else//must be IPv6
			fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.value |= ACL_TAGIF_IP6_BIT; //ipv6 bit

		fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.mask |= ACL_TAGIF_IP6_BIT; //ipv6 bit
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_SIP_BIT )
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP0].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv6SIP0%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP0].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[14]<<8)|(acl_filter->ingress_src_ipv6_addr[15]);//v6SIP[15:0]
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP0].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[14]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[15]);//v6SIP[15:0];

		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP1].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv6SIP1%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP1].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[12]<<8)|(acl_filter->ingress_src_ipv6_addr[13]);//v6SIP[31:16]
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP1].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[12]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[13]);

		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP2].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP2].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv6SIP2%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP2].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[10]<<8)|(acl_filter->ingress_src_ipv6_addr[11]);//v6SIP[47:32]
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP2].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[10]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[11]);;

		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP3].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP3].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv6SIP3%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP3].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[8]<<8)|(acl_filter->ingress_src_ipv6_addr[9]);//v6SIP[63:48]
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP3].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[8]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[9]);;

		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP4].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP4].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv6SIP4%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP4].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[6]<<8)|(acl_filter->ingress_src_ipv6_addr[7]);//v6SIP[79:64]
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP4].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[6]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[7]);;

		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP5].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP5].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv6SIP5%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP5].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[4]<<8)|(acl_filter->ingress_src_ipv6_addr[5]);//v6SIP[95:80]
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP5].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[4]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[5]);;

		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP6].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP6].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv6SIP6%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP6].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[2]<<8)|(acl_filter->ingress_src_ipv6_addr[3]);//v6SIP[111:96]
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP6].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[2]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[3]);;

		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP7].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP7].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv6SIP7%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP7].fieldUnion.data.value = (acl_filter->ingress_src_ipv6_addr[0]<<8)|(acl_filter->ingress_src_ipv6_addr[1]);//v6SIP[127:112]
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6SIP7].fieldUnion.data.mask = (acl_filter->ingress_src_ipv6_addr_mask[0]<<8)|(acl_filter->ingress_src_ipv6_addr_mask[1]);

	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_DIP_BIT )
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP0].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP0].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv6DIP0%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP0].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[14]<<8)|(acl_filter->ingress_dest_ipv6_addr[15]);//v6DIP[15:0]
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP0].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[14]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[15]);

		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP1].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP1].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv6DIP1%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP1].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[12]<<8)|(acl_filter->ingress_dest_ipv6_addr[13]);//v6DIP[31:16]
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP1].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[12]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[13]);

		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP2].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP2].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv6DIP2%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP2].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[10]<<8)|(acl_filter->ingress_dest_ipv6_addr[11]);//v6DIP[47:32]
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP2].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[10]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[11]);

		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP3].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP3].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv6DIP3%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP3].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[8]<<8)|(acl_filter->ingress_dest_ipv6_addr[9]);//v6DIP[63:48]
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP3].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[8]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[9]);

		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP4].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP4].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv6DIP4%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP4].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[6]<<8)|(acl_filter->ingress_dest_ipv6_addr[7]);//v6DIP[79:64]
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP4].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[6]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[7]);

		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP5].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP5].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv6DIP5%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP5].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[4]<<8)|(acl_filter->ingress_dest_ipv6_addr[5]);//v6DIP[95:80]
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP5].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[4]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[5]);

		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP6].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP6].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv6DIP6%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP6].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[2]<<8)|(acl_filter->ingress_dest_ipv6_addr[3]);//v6DIP[111:96]
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP6].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[2]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[3]);

		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP7].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP7].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IPv6DIP7%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP7].fieldUnion.data.value = (acl_filter->ingress_dest_ipv6_addr[0]<<8)|(acl_filter->ingress_dest_ipv6_addr[1]);//v6DIP[127:112]
		fc_db.aclField[RTK_ACL_TEMPLATE_IPv6DIP7].fieldUnion.data.mask = (acl_filter->ingress_dest_ipv6_addr_mask[0]<<8)|(acl_filter->ingress_dest_ipv6_addr_mask[1]);

	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_DSCP_BIT)
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_IP6_TC_NH].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_IP6_TC_NH].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IP6_TC_NH%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_IP6_TC_NH].fieldUnion.data.value &=0x00ffU;//clear former pattern
		fc_db.aclField[RTK_ACL_TEMPLATE_IP6_TC_NH].fieldUnion.data.value |= (acl_filter->ingress_ipv6_dscp)<<(2+8); //TC[15:8] + NH[7:0]
		fc_db.aclField[RTK_ACL_TEMPLATE_IP6_TC_NH].fieldUnion.data.mask |= 0xfc00U;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_TC_BIT)
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_IP6_TC_NH].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_IP6_TC_NH].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IP6_TC_NH%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_IP6_TC_NH].fieldUnion.data.value &=0x00ffU;//clear former pattern
		fc_db.aclField[RTK_ACL_TEMPLATE_IP6_TC_NH].fieldUnion.data.value |= (acl_filter->ingress_ipv6_tc)<<8; //TC[15:8] + NH[7:0]
		fc_db.aclField[RTK_ACL_TEMPLATE_IP6_TC_NH].fieldUnion.data.mask |= 0xff00U;
	}


	if(acl_filter->filter_fields & RT_ACL_INGRESS_L4_POROTCAL_VALUE_BIT)
	{
		if((acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_TAGIF_BIT)&&acl_filter->ingress_ipv4_tagif==1)
		{
			fc_db.aclField[RTK_ACL_TEMPLATE_IP4_TOS_PROTO].fieldType = ACL_FIELD_PATTERN_MATCH;
			fc_db.aclField[RTK_ACL_TEMPLATE_IP4_TOS_PROTO].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IP4_TOS_PROTO%ACL_PER_RULE_FIELD_SIZE);
			fc_db.aclField[RTK_ACL_TEMPLATE_IP4_TOS_PROTO].fieldUnion.data.value |=acl_filter->ingress_l4_protocal;
			fc_db.aclField[RTK_ACL_TEMPLATE_IP4_TOS_PROTO].fieldUnion.data.mask |=0xffU; //ToS[15:8] + protocal[7:0]
		}
		else if((acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_TAGIF_BIT)&&acl_filter->ingress_ipv6_tagif==1)
		{
			fc_db.aclField[RTK_ACL_TEMPLATE_IP6_TC_NH].fieldType = ACL_FIELD_PATTERN_MATCH;
			fc_db.aclField[RTK_ACL_TEMPLATE_IP6_TC_NH].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IP6_TC_NH%ACL_PER_RULE_FIELD_SIZE);
			fc_db.aclField[RTK_ACL_TEMPLATE_IP6_TC_NH].fieldUnion.data.value |= (acl_filter->ingress_l4_protocal); //TC[15:8] + NH[7:0]
			fc_db.aclField[RTK_ACL_TEMPLATE_IP6_TC_NH].fieldUnion.data.mask |= 0xffU;
		}
		else
		{
			WARNING("RT_ACL_INGRESS_L4_POROTCAL_VALUE_BIT must set with RT_ACL_INGRESS_IPV4_TAGIF_BIT or RT_ACL_INGRESS_IPV6_TAGIF_BIT for H/W ACL");
			return(RT_ERR_RG_INVALID_PARAM);
		}
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_L4_TCP_BIT)
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.value |= ACL_TAGIF_TCP_BIT; //TCP bit
		fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.mask |= ACL_TAGIF_TCP_BIT; //TCP bit
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_L4_UDP_BIT)
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.value |= ACL_TAGIF_UDP_BIT; //UDP bit
		fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.mask |= ACL_TAGIF_UDP_BIT; //UDP bit
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_L4_ICMP_BIT)
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_IP4_TOS_PROTO].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_IP4_TOS_PROTO].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IP4_TOS_PROTO%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_IP4_TOS_PROTO].fieldUnion.data.value &=0xff00; //clear formet pattern
		fc_db.aclField[RTK_ACL_TEMPLATE_IP4_TOS_PROTO].fieldUnion.data.value |=0x01U; //ICMP protocal value
		fc_db.aclField[RTK_ACL_TEMPLATE_IP4_TOS_PROTO].fieldUnion.data.mask |=0xffU; //ToS[15:8] + protocal[7:0]
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_L4_ICMPV6_BIT )
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_IP6_TC_NH].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_IP6_TC_NH].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_IP6_TC_NH%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_IP6_TC_NH].fieldUnion.data.value &=0xff00U;//clear former pattern
		fc_db.aclField[RTK_ACL_TEMPLATE_IP6_TC_NH].fieldUnion.data.value |=0x3aU; //ICMPv6 nextheader value
		fc_db.aclField[RTK_ACL_TEMPLATE_IP6_TC_NH].fieldUnion.data.mask |=0xffU;//mask 8 bit only
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_L4_SPORT_RANGE_BIT)
	{
		if((acl_filter->ingress_src_l4_port_start == acl_filter->ingress_src_l4_port_end) && (acl_filter->filter_fields & RT_ACL_INGRESS_L4_TCP_BIT)){
			//SINGLE TCP Port
			fc_db.aclField[RTK_ACL_TEMPLATE_TCP_SPORT].fieldType = ACL_FIELD_PATTERN_MATCH;
			fc_db.aclField[RTK_ACL_TEMPLATE_TCP_SPORT].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_TCP_SPORT%ACL_PER_RULE_FIELD_SIZE);
			fc_db.aclField[RTK_ACL_TEMPLATE_TCP_SPORT].fieldUnion.data.value = acl_filter->ingress_src_l4_port_start;
			fc_db.aclField[RTK_ACL_TEMPLATE_TCP_SPORT].fieldUnion.data.mask = 0xffffU;

		}else if((acl_filter->ingress_src_l4_port_start == acl_filter->ingress_src_l4_port_end) && (acl_filter->filter_fields & RT_ACL_INGRESS_L4_UDP_BIT)){
			//SINGLE UDP Port
			fc_db.aclField[RTK_ACL_TEMPLATE_UDP_SPORT].fieldType = ACL_FIELD_PATTERN_MATCH;
			fc_db.aclField[RTK_ACL_TEMPLATE_UDP_SPORT].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_UDP_SPORT%ACL_PER_RULE_FIELD_SIZE);
			fc_db.aclField[RTK_ACL_TEMPLATE_UDP_SPORT].fieldUnion.data.value = acl_filter->ingress_src_l4_port_start;
			fc_db.aclField[RTK_ACL_TEMPLATE_UDP_SPORT].fieldUnion.data.mask = 0xffffU;

		}else{
			uint32 pattern_idx=0;
			if(_rtk_fc_search_acl_exist_tableEntry(&pattern_idx,RTK_FC_ACL_USED_INGRESS_SPORTTABLE_INDEX,acl_filter->ingress_src_l4_port_end,acl_filter->ingress_src_l4_port_start,PORTRANGE_SPORT)==RT_ERR_RG_OK)
			{
				//found there is exist the same port_range entry can be share
				ACL_CTRL("Found acl SportRange[%d] can be share with this rule.",pattern_idx);
			}
			else
			{
				ret = _rtk_fc_search_acl_empty_tableEntry(&pattern_idx,RTK_FC_ACL_USED_INGRESS_SPORTTABLE_INDEX);
				if(ret!=RT_ERR_RG_OK) return ret;
			}

			bzero(&aclSportRangeEntry, sizeof(aclSportRangeEntry));
			aclSportRangeEntry.index = pattern_idx;
			aclSportRangeEntry.upper_bound=acl_filter->ingress_src_l4_port_end;
			aclSportRangeEntry.lower_bound=acl_filter->ingress_src_l4_port_start;
			aclSportRangeEntry.type = PORTRANGE_SPORT;

			fc_db.aclField[RTK_ACL_TEMPLATE_L4PORT_RANGE].fieldType = ACL_FIELD_PATTERN_MATCH;
			fc_db.aclField[RTK_ACL_TEMPLATE_L4PORT_RANGE].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_L4PORT_RANGE%ACL_PER_RULE_FIELD_SIZE);
			fc_db.aclField[RTK_ACL_TEMPLATE_L4PORT_RANGE].fieldUnion.data.value |= (1<<(pattern_idx));
			fc_db.aclField[RTK_ACL_TEMPLATE_L4PORT_RANGE].fieldUnion.data.mask |= (1<<(pattern_idx));

			if(rtk_acl_portRange_set(&aclSportRangeEntry))
			{
				ACL_CTRL("access acl SportRange table failed");
				return(RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
			}
			else
			{
				ACL_CTRL("use HW ACL PORT_RANGE[%d]",pattern_idx);
				aclSWEntry->hw_used_table |= RTK_FC_ACL_USED_INGRESS_SPORTTABLE;
				aclSWEntry->hw_used_table_index[RTK_FC_ACL_USED_INGRESS_SPORTTABLE_INDEX] = pattern_idx;
			}
		}
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_L4_DPORT_RANGE_BIT)
	{
		if((acl_filter->ingress_dest_l4_port_start == acl_filter->ingress_dest_l4_port_end) && (acl_filter->filter_fields & RT_ACL_INGRESS_L4_TCP_BIT)){
			//SINGLE TCP DPort
			fc_db.aclField[RTK_ACL_TEMPLATE_TCP_DPORT].fieldType = ACL_FIELD_PATTERN_MATCH;
			fc_db.aclField[RTK_ACL_TEMPLATE_TCP_DPORT].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_TCP_DPORT%ACL_PER_RULE_FIELD_SIZE);
			fc_db.aclField[RTK_ACL_TEMPLATE_TCP_DPORT].fieldUnion.data.value = acl_filter->ingress_dest_l4_port_start;
			fc_db.aclField[RTK_ACL_TEMPLATE_TCP_DPORT].fieldUnion.data.mask = 0xffffU;

		}else if((acl_filter->ingress_dest_l4_port_start == acl_filter->ingress_dest_l4_port_end) && (acl_filter->filter_fields & RT_ACL_INGRESS_L4_UDP_BIT)){
			//SINGLE UDP DPort
			fc_db.aclField[RTK_ACL_TEMPLATE_UDP_DPORT].fieldType = ACL_FIELD_PATTERN_MATCH;
			fc_db.aclField[RTK_ACL_TEMPLATE_UDP_DPORT].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_UDP_DPORT%ACL_PER_RULE_FIELD_SIZE);
			fc_db.aclField[RTK_ACL_TEMPLATE_UDP_DPORT].fieldUnion.data.value = acl_filter->ingress_dest_l4_port_start;
			fc_db.aclField[RTK_ACL_TEMPLATE_UDP_DPORT].fieldUnion.data.mask = 0xffffU;

		}else{
			uint32 pattern_idx=0;
			if(_rtk_fc_search_acl_exist_tableEntry(&pattern_idx,RTK_FC_ACL_USED_INGRESS_DPORTTABLE_INDEX,acl_filter->ingress_dest_l4_port_end,acl_filter->ingress_dest_l4_port_start,PORTRANGE_DPORT)==RT_ERR_RG_OK)
			{
				//found there is exist the same port_range entry can be share
				ACL_CTRL("Found acl DportRange[%d] can be share with this rule.",pattern_idx);
			}
			else
			{
				ret = _rtk_fc_search_acl_empty_tableEntry(&pattern_idx,RTK_FC_ACL_USED_INGRESS_DPORTTABLE_INDEX);
				if(ret!=RT_ERR_RG_OK) return ret;
			}

			bzero(&aclDportRangeEntry, sizeof(aclDportRangeEntry));
			aclDportRangeEntry.index = pattern_idx;
			aclDportRangeEntry.upper_bound=acl_filter->ingress_dest_l4_port_end;
			aclDportRangeEntry.lower_bound=acl_filter->ingress_dest_l4_port_start;
			aclDportRangeEntry.type = PORTRANGE_DPORT;

			fc_db.aclField[RTK_ACL_TEMPLATE_L4PORT_RANGE].fieldType = ACL_FIELD_PATTERN_MATCH;
			fc_db.aclField[RTK_ACL_TEMPLATE_L4PORT_RANGE].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_L4PORT_RANGE%ACL_PER_RULE_FIELD_SIZE);
			fc_db.aclField[RTK_ACL_TEMPLATE_L4PORT_RANGE].fieldUnion.data.value |= (1<<(pattern_idx));
			fc_db.aclField[RTK_ACL_TEMPLATE_L4PORT_RANGE].fieldUnion.data.mask |= (1<<(pattern_idx));

			if(rtk_acl_portRange_set(&aclDportRangeEntry))
			{
				ACL_CTRL("access HW ACL PORT_RANGE table failed");
				return(RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
			}
			else
			{
				ACL_CTRL("use HW ACL PORT_RANGE[%d]",pattern_idx);
				aclSWEntry->hw_used_table |= RTK_FC_ACL_USED_INGRESS_DPORTTABLE;
				aclSWEntry->hw_used_table_index[RTK_FC_ACL_USED_INGRESS_DPORTTABLE_INDEX] = pattern_idx;
			}
		}
	}


	if(acl_filter->filter_fields & RT_ACL_INGRESS_STREAM_ID_BIT)
	{
		fc_db.aclField[RTK_ACL_TEMPLATE_GEMPORT].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_GEMPORT].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_GEMPORT%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_GEMPORT].fieldUnion.data.value = acl_filter->ingress_stream_id;
		fc_db.aclField[RTK_ACL_TEMPLATE_GEMPORT].fieldUnion.data.mask = acl_filter->ingress_stream_id_mask;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_TCP_FLAGS_BIT)
	{
		//FS[2] for tcp flags
		fc_db.aclField[RTK_ACL_TEMPLATE_TCP_FLAGS].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_TCP_FLAGS].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_TCP_FLAGS%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_TCP_FLAGS].fieldUnion.data.value = acl_filter->ingress_tcp_flags&0x1ff;
		fc_db.aclField[RTK_ACL_TEMPLATE_TCP_FLAGS].fieldUnion.data.mask = acl_filter->ingress_tcp_flags_mask&0x1ff;

		fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.value |= ACL_TAGIF_TCP_BIT; //TCP bit
		fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.mask |= ACL_TAGIF_TCP_BIT; //TCP bit
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_PKT_LEN_RANGE_BIT)
	{
		uint32 pattern_idx=0;
		if(_rtk_fc_search_acl_exist_tableEntry(&pattern_idx,RTK_FC_ACL_USED_INGRESS_PKTLENTABLE_INDEX,acl_filter->ingress_packet_length_end,acl_filter->ingress_packet_length_start,PKTLENRANGE_NOTREVISE)==RT_ERR_RG_OK)
		{
			//found there is exist the same packet_length_range entry can be share
			ACL_CTRL("Found acl PacketLengthRange[%d] can be share with this rule.",pattern_idx);
		}
		else
		{
			ret = _rtk_fc_search_acl_empty_tableEntry(&pattern_idx,RTK_FC_ACL_USED_INGRESS_PKTLENTABLE_INDEX);
			if(ret!=RT_ERR_RG_OK) return ret;
		}

		bzero(&aclPktLengthRangeEntry, sizeof(aclPktLengthRangeEntry));
		aclPktLengthRangeEntry.index = pattern_idx;
		aclPktLengthRangeEntry.upper_bound=acl_filter->ingress_packet_length_end;
		aclPktLengthRangeEntry.lower_bound=acl_filter->ingress_packet_length_start;
		aclPktLengthRangeEntry.type = PKTLENRANGE_NOTREVISE;

		fc_db.aclField[RTK_ACL_TEMPLATE_PKTLEN_RANGE].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_PKTLEN_RANGE].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_PKTLEN_RANGE%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_PKTLEN_RANGE].fieldUnion.data.value |= (1<<(pattern_idx));
		fc_db.aclField[RTK_ACL_TEMPLATE_PKTLEN_RANGE].fieldUnion.data.mask |= (1<<(pattern_idx));

		if(rtk_acl_packetLengthRange_set(&aclPktLengthRangeEntry))
		{
			ACL_CTRL("access HW ACL PKTLEN_RANGE table failed");
			return(RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
		}
		else
		{
			ACL_CTRL("use HW ACL PKTLEN_RANGE[%d]",pattern_idx);
			aclSWEntry->hw_used_table |= RTK_FC_ACL_USED_INGRESS_PKTLENTABLE;
			aclSWEntry->hw_used_table_index[RTK_FC_ACL_USED_INGRESS_PKTLENTABLE_INDEX] = pattern_idx;
		}
	}

	// check arp pattern after stag/ctag
	if(acl_filter->filter_fields & RT_ACL_INGRESS_ARP_TARGET_IP_BIT)
	{
		rtk_acl_field_entry_t fieldSel={0};
		uint32 arpOffset = ARP_TARGET_IP_BYTE_OFFSET;
		
		fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.value |= ACL_TAGIF_ARP_BIT; //ARP bit
		fc_db.aclField[RTK_ACL_TEMPLATE_FRAME_TYPE_TAGS].fieldUnion.data.mask |= ACL_TAGIF_ARP_BIT; //ARP bit

		fc_db.aclField[RTK_ACL_TEMPLATE_ARP_TARGET_IP_UPPER].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_ARP_TARGET_IP_UPPER].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_ARP_TARGET_IP_UPPER%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_ARP_TARGET_IP_UPPER].fieldUnion.data.value = (acl_filter->ingress_arp_target_ip>>16)&0xffff;
		fc_db.aclField[RTK_ACL_TEMPLATE_ARP_TARGET_IP_UPPER].fieldUnion.data.mask = 0xffff;
		
		fc_db.aclField[RTK_ACL_TEMPLATE_ARP_TARGET_IP_LOWER].fieldType = ACL_FIELD_PATTERN_MATCH;
		fc_db.aclField[RTK_ACL_TEMPLATE_ARP_TARGET_IP_LOWER].fieldUnion.pattern.fieldIdx = (RTK_ACL_TEMPLATE_ARP_TARGET_IP_LOWER%ACL_PER_RULE_FIELD_SIZE);
		fc_db.aclField[RTK_ACL_TEMPLATE_ARP_TARGET_IP_LOWER].fieldUnion.data.value = acl_filter->ingress_arp_target_ip&0xffff;
		fc_db.aclField[RTK_ACL_TEMPLATE_ARP_TARGET_IP_LOWER].fieldUnion.data.mask = 0xffff;

		// re-init field sel since raw offset must consider stag/ctag.		
		if(acl_filter->ingress_stagif || 
			fc_db.aclField[RTK_ACL_TEMPLATE_STAG].fieldType == ACL_FIELD_PATTERN_MATCH)
			arpOffset += 4;

		if(acl_filter->ingress_ctagif || 
			fc_db.aclField[RTK_ACL_TEMPLATE_CTAG].fieldType == ACL_FIELD_PATTERN_MATCH)
			arpOffset += 4;

		//FS[6]: using for ARP target ip[31:16]
		{
			bzero(&fieldSel,sizeof(fieldSel));
			fieldSel.index = 6;
			fieldSel.format = ACL_FORMAT_RAW;
			fieldSel.offset = arpOffset;
			if(rtk_acl_fieldSelect_set(&fieldSel))WARNING("init ACL failed!!!\n");
		}
		//FS[7]: using for ARP target ip[15:0]
		{
			bzero(&fieldSel,sizeof(fieldSel));
			fieldSel.index = 7;
			fieldSel.format = ACL_FORMAT_RAW;
			fieldSel.offset = arpOffset+2;
			if(rtk_acl_fieldSelect_set(&fieldSel))WARNING("init ACL failed!!!\n");
		}

		fc_db.controlFuc.acl_arp_targetip_offset = (arpOffset - ARP_TARGET_IP_BYTE_OFFSET + 1);
	}


	//fieldadd to fc_db.aclRule, and record the valid ACL rule.
	for(j=0;j<GLOBAL_ACL_RULE_SIZE;j++)
	{
		for(i=(j*ACL_PER_RULE_FIELD_SIZE); i<((j+1)*ACL_PER_RULE_FIELD_SIZE); i++)
		{
			if(memcmp(&fc_db.aclField[i],&aclFieldEmpty,sizeof(rtk_acl_field_t)))
			{
				aclRuleValid[j] = ENABLED;
				if(rtk_acl_igrRuleField_add(&fc_db.aclRule[j], &fc_db.aclField[i]))
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


	aclActionRuleIdx = 0U; //if no pattern is need, default use the fc_db.aclRule[0] to assign action.
	aclCountinueRuleSize = 0U;
	firstHitFlag=0U;
	for(i=0;i<GLOBAL_ACL_RULE_SIZE;i++) //find first valid rule to assign action.
	{
		if(aclRuleValid[i]==ENABLED){
			//ACL_CTRL("aclTmpRule[%d] is valid, and will add to HW_ACL",aclActionRuleIdx);
			aclCountinueRuleSize++;

			if(firstHitFlag==0U)
			{
				aclActionRuleIdx = i; //record the first aclTmpRule to assign actions.
				firstHitFlag=1U;
			}
		}
	}


	//For case: although no fc_db.aclField is need, but still need one fc_db.aclRule to filter basic pattern,such as ingress_portmask, stagif, ctagif ...etc.  and do actions
	if(memcmp(&fc_db.aclRule[0],&aclRuleEmpty,sizeof(rtk_acl_ingress_entry_t)) && aclCountinueRuleSize==0)//use fc_db.aclRule[0] to contant the pattern and action, and add to HW.
	{
		aclCountinueRuleSize = 1U;
		aclActionRuleIdx = 0U;
		aclRuleValid[0]=ENABLED;
	}

	if(aclCountinueRuleSize > MAX_ACL_TEMPLATE_MULTIPLE_HIT_SIZE)
	{
		WARNING("HW ACL  maximum support %d templates, now may use %d templates.",MAX_ACL_TEMPLATE_MULTIPLE_HIT_SIZE,aclCountinueRuleSize);
		return(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
	}

	//ACL_CTRL("the action is set in aclTmpRule[%d]",aclActionRuleIdx);

	/*set actions*/
	for(i=0; i<RT_ACL_ACTION_FIELDS_INDEX_MAX; i++)
	{
		switch((acl_filter->action_fields)&(0x1<<i))
		{
			case RT_ACL_ACTION_FORWARD_GROUP_DROP_BIT:
				fc_db.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLED;
				fc_db.aclRule[aclActionRuleIdx].act.forwardAct.act= ACL_IGR_FORWARD_EGRESSMASK_ACT;
				fc_db.aclRule[aclActionRuleIdx].act.forwardAct.portMask.bits[0]=0x0U;
				break;

			case RT_ACL_ACTION_FORWARD_GROUP_TRAP_BIT:
				fc_db.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLED;
				fc_db.aclRule[aclActionRuleIdx].act.forwardAct.act= ACL_IGR_FORWARD_TRAP_ACT;
				break;
				
			case RT_ACL_ACTION_FORWARD_GROUP_TRAP_SLAVECPU_BIT:
				fc_db.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLED;
				fc_db.aclRule[aclActionRuleIdx].act.forwardAct.act= ACL_IGR_FORWARD_TRAP2SLAVE_ACT;
				fc_db.aclRule[aclActionRuleIdx].act.forwardAct.portMask.bits[0]= 0x1;

				//DS qos will loopback to slave cpu(pon_ds_p7_loopback_en), try to keep original by disable CVLAN (prevent impact by other trap rule that use CVLAN as debug info)
				fc_db.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_CVLAN_ACT] = ENABLED;
				fc_db.aclRule[aclActionRuleIdx].act.cvlanAct.act = ACL_IGR_CVLAN_MIB_ACT;
				aclSWEntry->hw_mib_index = MAX_ACL_MIB_TABLE_SIZE;
				fc_db.aclRule[aclActionRuleIdx].act.cvlanAct.mib = aclSWEntry->hw_mib_index;
				break;

			case RT_ACL_ACTION_FORWARD_GROUP_PERMIT_BIT:
				fc_db.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLED;
				fc_db.aclRule[aclActionRuleIdx].act.forwardAct.act= ACL_IGR_FORWARD_EGRESSMASK_ACT;
				fc_db.aclRule[aclActionRuleIdx].act.forwardAct.portMask.bits[0]= RTK_FC_ALL_MAC_PORTMASK;
				break;

			case RT_ACL_ACTION_FORWARD_GROUP_REDIRECT_BIT:
				fc_db.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLED;
				fc_db.aclRule[aclActionRuleIdx].act.forwardAct.act= ACL_IGR_FORWARD_REDIRECT_ACT;
				fc_db.aclRule[aclActionRuleIdx].act.forwardAct.portMask.bits[0] = 0x1 << acl_filter->action_forward_group_redirect_port_idx;
				break;

			case RT_ACL_ACTION_PRIORITY_GROUP_TRAP_PRIORITY_BIT:
				fc_db.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_PRI_ACT] = ENABLED;
				fc_db.aclRule[aclActionRuleIdx].act.priAct.act = ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
				fc_db.aclRule[aclActionRuleIdx].act.priAct.aclPri = acl_filter->action_priority_group_trap_priority;
				break;

			case RT_ACL_ACTION_PRIORITY_GROUP_ACL_PRIORITY_BIT:
				fc_db.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_PRI_ACT] = ENABLED;
				fc_db.aclRule[aclActionRuleIdx].act.priAct.act = ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
				fc_db.aclRule[aclActionRuleIdx].act.priAct.aclPri = acl_filter->action_priority_group_acl_priority;
				break;

			case RT_ACL_ACTION_METER_GROUP_SHARE_METER_BIT:
				fc_db.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_LOG_ACT] = ENABLED;
				fc_db.aclRule[aclActionRuleIdx].act.logAct.act = ACL_IGR_LOG_POLICING_ACT;

				if(fc_db.controlFuc.rt_api_is_enabled){

					ret = RTK_FC_HELPER_RT_RATE_SHAREMETER_MAPPING_HW_GET(acl_filter->action_meter_group_share_meter_index, &meterMapping);
					if((ret == RT_ERR_RG_OK) && (meterMapping.type == RT_METER_TYPE_ACL))
					{
						aclSWEntry->hw_share_meter_index = meterMapping.hw_index;
					}
					else
					{
						WARNING("get ACL meter hw index Failed, return value: 0x%x, virtual meter type: %d", ret, meterMapping.type);
						return (RT_ERR_RG_INVALID_PARAM);
					}
				}else
					aclSWEntry->hw_share_meter_index = acl_filter->action_meter_group_share_meter_index;


				fc_db.aclRule[aclActionRuleIdx].act.logAct.meter = aclSWEntry->hw_share_meter_index;
				break;

			case RT_ACL_ACTION_LOGGING_GROUP_MIB_BIT:
				fc_db.aclRule[aclActionRuleIdx].act.enableAct[ACL_IGR_CVLAN_ACT] = ENABLED;
				fc_db.aclRule[aclActionRuleIdx].act.cvlanAct.act = ACL_IGR_CVLAN_MIB_ACT;
				aclSWEntry->hw_mib_index = acl_filter->action_logging_group_mib_index * 2;
				fc_db.aclRule[aclActionRuleIdx].act.cvlanAct.mib = aclSWEntry->hw_mib_index;
				break;

			default:
				break;
		}
	}


WLAN_MBSSID_MASK_NEED_DOUBLE_RULE:

	/*add ACL rule*/
	ret = _rtk_fc_search_acl_empty_Entry(aclCountinueRuleSize, &aclIdx);
	if(ret!=RT_ERR_RG_OK)
		return ret;
	//ACL_CTRL("aclTmpRule will add to ACL[%d] (continue:%d)",aclIdx,aclCountinueRuleSize);

	index = aclIdx;//index is used for HW ACL countinuous rule.
	for(i=0; i<GLOBAL_ACL_RULE_SIZE; i++)
	{
		if((i==aclActionRuleIdx) && (aclCountinueRuleSize>1))
		{
			ACL_CTRL("pospond add aclTmpRule[%d] with action to HW ACL[%d] untill all pattern add finish",i,aclIdx);
			index++;
			continue;
		}
		if(aclRuleValid[i]==ENABLED)
		{
			fc_db.aclRule[i].index = index;
			fc_db.aclRule[i].valid = ENABLED;
			fc_db.aclRule[i].templateIdx = i;
			ret = RTK_ACL_IGRRULEENTRY_ADD(&fc_db.aclRule[i], shareHwAclWithSWAclIdx);
			if(ret)
			{
				ACL_CTRL("adding aclTmpRule[%d] to HW ACL[%d] failed. ret=0x%x",i,index,ret);
				return(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
			}

			ACL_CTRL("adding aclTmpRule[%d] to HW ACL[%d] success. ret=0x%x",i,fc_db.aclRule[i].index,ret);
			index++;
		}
	}

	if(aclCountinueRuleSize>1) {	//add action untill all pattern add finish prevent mis-hit
		fc_db.aclRule[aclActionRuleIdx].index = aclIdx;
		fc_db.aclRule[aclActionRuleIdx].valid = ENABLED;
		fc_db.aclRule[aclActionRuleIdx].templateIdx = aclActionRuleIdx;
		ret = RTK_ACL_IGRRULEENTRY_ADD(&fc_db.aclRule[aclActionRuleIdx], shareHwAclWithSWAclIdx);
		if(ret)
		{
			ACL_CTRL("adding aclTmpRule[%d] with action to HW ACL[%d] failed. ret=0x%x",aclActionRuleIdx,aclIdx,ret);
			return(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
		}
		ACL_CTRL("adding aclTmpRule[%d] with action to HW ACL[%d] success. ret=0x%x",aclActionRuleIdx,fc_db.aclRule[aclActionRuleIdx].index,ret);
	}

	if( portmask_need_double_hw_rule==1U && portmask_need_double_hw_rule_add_sencond==1)//the second dummy H/W ACL rule for mac_port
	{
		aclSWEntry->hw_aclEntry_size += aclCountinueRuleSize;
	}
	else
	{
		if(aclSWEntry->hw_aclEntry_size) {
			aclSWEntry->hw_aclEntry_size += aclCountinueRuleSize;
		}
		else {
			aclSWEntry->hw_aclEntry_start = aclIdx;
			aclSWEntry->hw_aclEntry_size = aclCountinueRuleSize;
		}
	}

	ACL_CTRL("using HW_ACL hw_aclEntry_start is %d, hw_aclEntry_size is %d",aclSWEntry->hw_aclEntry_start,aclSWEntry->hw_aclEntry_size);

	if( portmask_need_double_hw_rule==1U && portmask_need_double_hw_rule_add_sencond==0U){	//rule add for first time
		//DEBUG("ext_port rule added: hw_aclEntry_start=%d, hw_aclEntry_size=%d",aclSWEntry->hw_aclEntry_start,aclSWEntry->hw_aclEntry_size);

		/* If wlan mbssid mask is different for each wlan index, each mbssid mask should add one group by it's self */
		for(i=(portmask_need_double_hw_rule_wlan_index+1); i<wlan_count; i++)
		{	//example: current wlan index=0, check wlan index1 and index2
			if(ext_pmsk[i].bits[0] && (ext_pmsk[i].bits[0] != ext_pmsk[portmask_need_double_hw_rule_wlan_index].bits[0]))
			{	//example: if wlan index2 mbssid mask is different from wlan index0 mbssid mask, then it may need to add another group acl
				for(j=0; j<(portmask_need_double_hw_rule_wlan_index+1); j++) {
					if(ext_pmsk[j].bits[0] && (ext_pmsk[j].bits[0] == ext_pmsk[i].bits[0])) {
						j = -1;	//wlan0_mbssid_mask=0x1, wlan1=0x2, wlan2=0x1, then wlan2 will config with wlan0, so only care (wlan0+wlan2) and (wlan1)
						break;	//example: event wlan index2 mbssid mask is different from wlan index0 mbssid mask, it should also different from wlan0/wlan1
					}
				}
				if(j != -1)
				{	//this ext_port is different from previous
					portmask_need_double_hw_rule_wlan_index = i;
					fc_db.aclField[RTK_ACL_TEMPLATE_EXTPORTMASK].fieldUnion.data.mask = acl_ext_pmsk[portmask_need_double_hw_rule_wlan_index].bits[0]; //band not allowed ext_port
					for(i=0; i<GLOBAL_ACL_RULE_SIZE; i++)
					{
						fc_db.aclRule[i].activePorts.bits[0] = cpu_ext_pmsk[portmask_need_double_hw_rule_wlan_index].bits[0];
						for(j=(portmask_need_double_hw_rule_wlan_index+1); j<wlan_count; j++)
						{
							if(ext_pmsk[j].bits[0] && (ext_pmsk[j].bits[0] == ext_pmsk[portmask_need_double_hw_rule_wlan_index].bits[0]))
								fc_db.aclRule[i].activePorts.bits[0] |= cpu_ext_pmsk[j].bits[0];
						}
					}
					ACL_CTRL("= set another group rule for wlan%d (not allow ext_port=0x%x, and mac_port=0x%x) =",portmask_need_double_hw_rule_wlan_index,fc_db.aclField[RTK_ACL_TEMPLATE_EXTPORTMASK].fieldUnion.data.mask, fc_db.aclRule[0].activePorts.bits[0]);
					goto WLAN_MBSSID_MASK_NEED_DOUBLE_RULE;
				}
			}
		}

		portmask_need_double_hw_rule_add_sencond = 1U;
		goto PORTMASK_NEED_DOUBLE_RULE;
	}

	if( portmask_need_double_hw_rule==1U && portmask_need_double_hw_rule_add_sencond==1U){	//rule add for second time, restore origianl hw_aclEntry_start and accumulate hw_aclEntry_size
		//DEBUG("mac_port rule added: hw_aclEntry_start=%d, hw_aclEntry_size=%d",aclSWEntry->hw_aclEntry_start,aclSWEntry->hw_aclEntry_size);
	}

	return (RT_ERR_RG_OK);
}

static int _rtk_fc_free_acl_portTableEntry(int index)
{
	int ret;
    rtk_acl_rangeCheck_l4Port_t aclPortRangeEntry;
    bzero(&aclPortRangeEntry,sizeof(aclPortRangeEntry));
    aclPortRangeEntry.index=index;
	ret = rtk_acl_portRange_set(&aclPortRangeEntry);
    if(ret!= RT_ERR_RG_OK)
    {
        ACL_CTRL("free acl porttable failed, ret(rtk)=%d",ret);
        return (RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
    }
    return (RT_ERR_RG_OK);
}

static int _rtk_fc_free_acl_ipTableEntry(int index)
{
	int ret;
    rtk_acl_rangeCheck_ip_t aclIpRangeEntry;
    bzero(&aclIpRangeEntry,sizeof(aclIpRangeEntry));
    aclIpRangeEntry.index=index;
	ret = rtk_acl_ipRange_set(&aclIpRangeEntry);
    if(ret!= RT_ERR_RG_OK)
    {
        ACL_CTRL("free acl iptable failed, ret(rtk)=%d",ret);
        return (RT_ERR_RG_ACL_IPTABLE_ACCESS_FAILED);
    }
    return (RT_ERR_RG_OK);
}

static int _rtk_fc_free_acl_pktLenTableEntry(int index)
{
	int ret;
    rtk_acl_rangeCheck_pktLength_t aclPktLenRangeEntry;
    bzero(&aclPktLenRangeEntry,sizeof(aclPktLenRangeEntry));
    aclPktLenRangeEntry.index=index;
	ret = rtk_acl_packetLengthRange_set(&aclPktLenRangeEntry);
    if(ret!= RT_ERR_RG_OK)
    {
        ACL_CTRL("free acl packet lengh range table failed, ret(rtk)=%d",ret);
        return (RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
    }
    return (RT_ERR_RG_OK);
}

static int _rtk_fc_reset_asic_TableEntry(void)
{
	int i;

	//delete H/W ACL , not include reserved entries
	for(i=MIN_ACL_ENTRY_INDEX; i<=MAX_ACL_ENTRY_INDEX; i++)
	{
		if(rtk_acl_igrRuleEntry_del(i))
		{
			return (RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
		}
	}
	ACL_CTRL("clear HW_ACL[%d~%d]",MIN_ACL_ENTRY_INDEX,MAX_ACL_ENTRY_INDEX);

	//delete ACL IP range table
	for(i=0; i<MAX_ACL_IPRANGETABLE_SIZE; i++)
	{
		if(_rtk_fc_free_acl_ipTableEntry(i))
		{
			return (RT_ERR_RG_ACL_IPTABLE_ACCESS_FAILED);
		}
	}
	ACL_CTRL("clear all HW_ACL IP_RANGE[%d~%d]",0,MAX_ACL_IPRANGETABLE_SIZE);

	//delete ACL Port range table
	for(i=0; i<MAX_ACL_PORTRANGETABLE_SIZE; i++)
	{
		if(_rtk_fc_free_acl_portTableEntry(i))
		{
			return (RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
		}
	}
	ACL_CTRL("clear all HW_ACL PORT_RANGE[%d~%d]",0,MAX_ACL_PORTRANGETABLE_SIZE);

	//delete ACL Packet Lengh range table
	for(i=0; i<MAX_ACL_PACKETRANGETABLE_SIZE; i++)
	{
		if(_rtk_fc_free_acl_pktLenTableEntry(i))
		{
			return (RT_ERR_RG_ACL_PORTTABLE_ACCESS_FAILED);
		}
	}
	ACL_CTRL("clear all HW_ACL PKT_LEN_RANGE[%d~%d]",0,MAX_ACL_PACKETRANGETABLE_SIZE);

    return (RT_ERR_RG_OK);
}


void _rtk_fc_acl_for_multicast_temp_protection_add(void)
{
	uint32 aclIdx = 0U;
	rtk_acl_ingress_entry_t aclRule;
	rtk_acl_field_t aclField;

	aclIdx = fc_db.aclAndCfReservedRule.aclProtectLowerBoundary;

	/***set IPv4 Multicast Permit rule***/
	{
		bzero(&aclRule,sizeof(aclRule));
		bzero(&aclField,sizeof(aclField));

		//setup MC IPv4 DIP 224.0.0.0 to 239.255.255.255
		aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
		aclField.fieldUnion.pattern.fieldIdx = 1U; //dipv4[31:16]: template[1] field[1]
		aclField.fieldUnion.data.value = 0xe000U;
		aclField.fieldUnion.data.mask = 0xf000U;
		if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
		{
			WARNING("setup ACL for MC IPv4 permit failed!!!");
		}

		aclRule.valid=ENABLED;
		aclRule.index=aclIdx;
		aclRule.templateIdx = 1U;//dipv4 in template[1]
		aclRule.activePorts.bits[0]=RTK_FC_ALL_MAC_PORTMASK_WITHOUT_CPU;
		aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
		aclRule.act.forwardAct.act= ACL_IGR_FORWARD_EGRESSMASK_ACT; //permit action
		aclRule.act.forwardAct.portMask.bits[0]=RTK_FC_ALL_MAC_PORTMASK;
		if(rtk_acl_igrRuleEntry_add(&aclRule))
		{
			WARNING("setup ACL for MC IPv4 permit failed!!!");
		}

		aclIdx++;
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
		aclRule.index=aclIdx;
		aclRule.templateIdx = 6U;//dipv6 in template[6]
		aclRule.activePorts.bits[0]=RTK_FC_ALL_MAC_PORTMASK_WITHOUT_CPU;
		aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
		aclRule.act.forwardAct.act= ACL_IGR_FORWARD_EGRESSMASK_ACT; //permit action
		aclRule.act.forwardAct.portMask.bits[0]=RTK_FC_ALL_MAC_PORTMASK;
		if(rtk_acl_igrRuleEntry_add(&aclRule))
		{
			WARNING("setup ACL for MC IPv6 permit failed!!!");
		}

		aclIdx++;
	}

	/***set rest trap rule***/
	if(0){	//Due to FC already apply "add action untill all pattern add finish prevent mis-hit", so remove this rule to prevent hw acc fail
		bzero(&aclRule,sizeof(aclRule));
		bzero(&aclField,sizeof(aclField));

		aclRule.valid=ENABLED;
		aclRule.index=aclIdx;
		aclRule.templateIdx=0U;
		aclRule.activePorts.bits[0]=RTK_FC_ALL_MAC_PORTMASK_WITHOUT_CPU;
		aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
		aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
		if(rtk_acl_igrRuleEntry_add(&aclRule))
		{
			WARNING("setup ACL for rest all trap failed!!!");
		}

		aclIdx++;
	}

	if(fc_db.aclAndCfReservedRule.aclProtectUpperBoundary==fc_db.aclAndCfReservedRule.aclProtectLowerBoundary)
		fc_db.aclAndCfReservedRule.aclProtectUpperBoundary = aclIdx - 1;
	ACL_CTRL("ACL Rearrange Protection(MC permit, Others trap) Start @ acl[%d] to acl[%d]", fc_db.aclAndCfReservedRule.aclProtectLowerBoundary, fc_db.aclAndCfReservedRule.aclProtectUpperBoundary);
	ACL_RSV("ACL Rearrange Protection(MC permit, Others trap) Start @ acl[%d] to acl[%d]", fc_db.aclAndCfReservedRule.aclProtectLowerBoundary, fc_db.aclAndCfReservedRule.aclProtectUpperBoundary);
}

void _rtk_fc_acl_for_multicast_temp_protection_del(void)
{
	int i;

	for(i=fc_db.aclAndCfReservedRule.aclProtectLowerBoundary; i<=fc_db.aclAndCfReservedRule.aclProtectUpperBoundary; i++){
		if(rtk_acl_igrRuleEntry_del(i))
		{
			WARNING("setup ACL for MC temporary trap failed!!!");
		}
	}
	ACL_CTRL("ACL Rearrange Protection(MC permit, Others trap) Stop @ acl[%d] to acl[%d]", fc_db.aclAndCfReservedRule.aclProtectLowerBoundary, fc_db.aclAndCfReservedRule.aclProtectUpperBoundary);
	ACL_RSV("ACL Rearrange Protection(MC permit, Others trap) Stop @ acl[%d] to acl[%d]", fc_db.aclAndCfReservedRule.aclProtectLowerBoundary, fc_db.aclAndCfReservedRule.aclProtectUpperBoundary);
}

/*(4)reserved ACL related APIs*/
static int _rtk_fc_aclAndCfReservedRuleAddCheck(uint32 aclRsvSize, uint32 cfRsvSize)
{
	uint32 i;
	rtk_acl_ingress_entry_t aclRule;
	//check rest empty acl rules in enough
	for(i=fc_db.aclAndCfReservedRule.aclUpperBoundary;i>(fc_db.aclAndCfReservedRule.aclUpperBoundary-aclRsvSize);i--){
		aclRule.index=i;
		assert_ok(rtk_acl_igrRuleEntry_get(&aclRule));
		if(aclRule.valid!=DISABLED){
			WARNING("ACL rest rules for reserved is not enough! i=%d aclUpperBoundary=%d aclRsvSize=%d",i,fc_db.aclAndCfReservedRule.aclUpperBoundary,aclRsvSize);
			return (RT_ERR_RG_FAILED);
		}
	}
	return (RT_ERR_RG_OK);

}

static int _rtk_fc_aclAndCfReservedRule_hybridIntfRsvAdd(int aclIdx, rtk_mac_t mac)
{
	rtk_acl_ingress_entry_t aclRule;
	rtk_acl_field_t aclField_dmac0,aclField_dmac1,aclField_dmac2;

	bzero(&aclRule,sizeof(aclRule));
	bzero(&aclField_dmac0,sizeof(aclField_dmac0));
	bzero(&aclField_dmac1,sizeof(aclField_dmac1));
	bzero(&aclField_dmac2,sizeof(aclField_dmac2));

	aclRule.valid=ENABLED;
	aclRule.index=aclIdx;
	aclRule.activePorts.bits[0]=RTK_FC_ALL_MAC_PORTMASK;
	aclRule.templateIdx=0U; /*use :DMAC => template[0],field[0]-field[2]*/

	//setup gmac
	aclField_dmac2.fieldType = ACL_FIELD_PATTERN_MATCH;
	aclField_dmac2.fieldUnion.pattern.fieldIdx = 0U; //DA[15:0]: template[0] field[0]
	aclField_dmac2.fieldUnion.data.value = (mac.octet[4]<<8) | (mac.octet[5]);
	aclField_dmac2.fieldUnion.data.mask = 0xff00U;	//only care MAC prefix
	if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac2)){
		return (RT_ERR_RG_FAILED);
	}

	aclField_dmac1.fieldType = ACL_FIELD_PATTERN_MATCH;
	aclField_dmac1.fieldUnion.pattern.fieldIdx = 1U; //DA[31:16]: template[0] field[1]
	aclField_dmac1.fieldUnion.data.value = (mac.octet[2]<<8) | (mac.octet[3]);
	aclField_dmac1.fieldUnion.data.mask = 0xffff;
	if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac1)){
		return (RT_ERR_RG_FAILED);
	}

	aclField_dmac0.fieldType = ACL_FIELD_PATTERN_MATCH;
	aclField_dmac0.fieldUnion.pattern.fieldIdx = 2U; //DA[47:32]: template[0] field[2]
	aclField_dmac0.fieldUnion.data.value = (mac.octet[0]<<8) | (mac.octet[1]);
	aclField_dmac0.fieldUnion.data.mask = 0xffffU;
	if(rtk_acl_igrRuleField_add(&aclRule, &aclField_dmac0)){
		return (RT_ERR_RG_FAILED);
	}

	if(rtk_acl_igrRuleEntry_add(&aclRule)){
		return (RT_ERR_RG_FAILED);
	}

	return (RT_ERR_RG_OK);
}

static int _rtk_fc_aclAndCfReservedRuleHeadReflash(void)
{
	uint32 type;
	uint32 aclIdx=fc_db.aclAndCfReservedRule.aclProtectUpperBoundary+1; //ACL[0],ACL[1],ACL[2] reserved for _rtk_fc_acl_for_multicast_temp_protection_add()
	int addRuleFailedFlag=0;
	int loop_end = 0;

	rtk_acl_ingress_entry_t aclRule;
	rtk_acl_field_t aclField;
	rtk_acl_field_t aclField2;
#if 0	//not verify yet	
	rtk_acl_field_t aclField3;
	rtk_acl_rangeCheck_pktLength_t aclPktLengthRangeEntry;
#endif
	uint32 i;
	rtk_mac_t mac;

	//reflash th rules
	for(i=aclIdx;i<fc_db.aclAndCfReservedRule.aclLowerBoundary;i++){
		assert_ok(rtk_acl_igrRuleEntry_del(i));
	}


	for(type=0;type<RTK_FC_ACLANDCF_RESERVED_HEAD_END;type++){
		if(fc_db.aclAndCfReservedRule.reservedMask[type]==ENABLED){
			switch(type){
				
				case RTK_FC_ACLANDCF_RESERVED_ALL_TRAP:
					{
						ACL_RSV("add RSV[%d] ALL_TRAP @ acl[%d]",type,aclIdx);

						bzero(&aclRule,sizeof(aclRule));

						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=RTK_FC_ALL_MAC_PORTMASK_WITHOUT_CPU;
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
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP:
					{
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						bzero(&aclField2,sizeof(aclField2));

						//trap 239.255.255.250
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 0U; //DIP[15:0]
						aclField.fieldUnion.data.value = 0xfffaU;
						aclField.fieldUnion.data.mask = 0xffffU;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
						{
							ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField2.fieldUnion.pattern.fieldIdx = 1U; //DIP[31:16]
						aclField2.fieldUnion.data.value = 0xefffU;
						aclField2.fieldUnion.data.mask = 0xffffU;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField2))
						{
							ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.templateIdx=1U;
						#if defined(CONFIG_RTK_NIC_HWLOOKUP_DEAMSDU_OFFLOAD)
						aclRule.activePorts.bits[0]=(RTK_FC_ALL_MAC_CPU_PORTMASK|RTK_FC_ALL_MAC_PORTMASK_WITHOUT_CPU)&(~fc_db.hypridPPTP.portmask);
						#else
						aclRule.activePorts.bits[0]=RTK_FC_ALL_MAC_PORTMASK_WITHOUT_CPU&(~fc_db.hypridPPTP.portmask);

						#endif
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type))
						{
							ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						ACL_RSV("add RSV[%d] MULTICAST_SSDP_TRAP @ acl[%d]",type,aclIdx);

						//point to next ruleIdx
						aclIdx++;
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY:
					{
						if(fc_db.hypridPPTP.portmask){
							ACL_RSV("add RSV[%d] L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY @ acl[%d] - acl[%d]",type,aclIdx,(aclIdx+fc_db.aclAndCfReservedRule.hybrid_pptp_func.intf_count*2-1));
							loop_end = fc_db.aclAndCfReservedRule.hybrid_pptp_func.intf_count;
						}
						else{
							ACL_RSV("add RSV[%d] L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY @ acl[%d]",type,aclIdx);
							loop_end = 1;
						}

						for(i=0; i<loop_end; i++)
						{

							if(fc_db.hypridPPTP.portmask){
								if(!(fc_db.aclAndCfReservedRule.hybrid_pptp_func.gateway_mac_addr[i].octet[0]|fc_db.aclAndCfReservedRule.hybrid_pptp_func.gateway_mac_addr[i].octet[1]|
									fc_db.aclAndCfReservedRule.hybrid_pptp_func.gateway_mac_addr[i].octet[2]|fc_db.aclAndCfReservedRule.hybrid_pptp_func.gateway_mac_addr[i].octet[3]|
									fc_db.aclAndCfReservedRule.hybrid_pptp_func.gateway_mac_addr[i].octet[4]|fc_db.aclAndCfReservedRule.hybrid_pptp_func.gateway_mac_addr[i].octet[5]))
									continue;

								memcpy(&mac.octet, fc_db.aclAndCfReservedRule.hybrid_pptp_func.gateway_mac_addr[i].octet, ETH_ALEN);

								addRuleFailedFlag = _rtk_fc_aclAndCfReservedRule_hybridIntfRsvAdd(aclIdx, mac);
								if(addRuleFailedFlag){
									ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY failed!!!");
									addRuleFailedFlag=1;
									break;
								}

								//point to next ruleIdx
								aclIdx++;
							}

							//Set ACL rule for higher rx prioirty for L2TP controal packet and LCP packet(L2TP, none-ipv4, none-ipv6)
							bzero(&aclRule,sizeof(aclRule));
							bzero(&aclField,sizeof(aclField));

							aclRule.valid=ENABLED;
							aclRule.index=aclIdx;
							aclRule.activePorts.bits[0]=RTK_FC_ALL_MAC_PORTMASK;
							aclRule.templateIdx=2U; /*use :tagIf=> template[2],field[5]*/


							aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
							aclField.fieldUnion.pattern.fieldIdx = 5U;/*template[2],field[5]*/
							aclField.fieldUnion.data.value = (ACL_TAGIF_L2TP_BIT & (~ACL_TAGIF_IN_IP4_BIT) & (~ACL_TAGIF_IP6_BIT));
							aclField.fieldUnion.data.mask = (ACL_TAGIF_L2TP_BIT | ACL_TAGIF_IN_IP4_BIT | ACL_TAGIF_IP6_BIT);
							if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
								ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							//force trap to CPU with priroity to avoid FB trap priroity replaced.
							aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
							aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
							aclRule.act.priAct.aclPri=fc_db.aclAndCfReservedRule.l2tp_ctrl_lcp_assign_prioity.priority;
							aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
							aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
							if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
								ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							//point to next ruleIdx
							aclIdx++;

						}

					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY:
					{
						if(fc_db.hypridPPTP.portmask){
							ACL_RSV("add RSV[%d] PPPoE_LCP_PACKET_ASSIGN_PRIORITY @ acl[%d] - acl[%d]",type,aclIdx,(aclIdx+fc_db.aclAndCfReservedRule.hybrid_pptp_func.intf_count*2-1));
							loop_end = fc_db.aclAndCfReservedRule.hybrid_pptp_func.intf_count;
						}
						else{
							ACL_RSV("add RSV[%d] PPPoE_LCP_PACKET_ASSIGN_PRIORITY @ acl[%d]",type,aclIdx);
							loop_end = 1;
						}

						for(i=0; i<loop_end; i++)
						{

							if(fc_db.hypridPPTP.portmask){
								if(!(fc_db.aclAndCfReservedRule.hybrid_pptp_func.gateway_mac_addr[i].octet[0]|fc_db.aclAndCfReservedRule.hybrid_pptp_func.gateway_mac_addr[i].octet[1]|
									fc_db.aclAndCfReservedRule.hybrid_pptp_func.gateway_mac_addr[i].octet[2]|fc_db.aclAndCfReservedRule.hybrid_pptp_func.gateway_mac_addr[i].octet[3]|
									fc_db.aclAndCfReservedRule.hybrid_pptp_func.gateway_mac_addr[i].octet[4]|fc_db.aclAndCfReservedRule.hybrid_pptp_func.gateway_mac_addr[i].octet[5]))
									continue;

								memcpy(&mac.octet, fc_db.aclAndCfReservedRule.hybrid_pptp_func.gateway_mac_addr[i].octet, ETH_ALEN);

								addRuleFailedFlag = _rtk_fc_aclAndCfReservedRule_hybridIntfRsvAdd(aclIdx, mac);
								if(addRuleFailedFlag){
									ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY failed!!!");
									addRuleFailedFlag=1;
									break;
								}

								//point to next ruleIdx
								aclIdx++;
							}

							//Set ACL rule for higher rx prioirty for PPPoE LCP packet(eth=0x8864, none-ipv4, none-ipv6)
							bzero(&aclRule,sizeof(aclRule));
							bzero(&aclField,sizeof(aclField));

							aclRule.valid=ENABLED;
							aclRule.index=aclIdx;
							aclRule.activePorts.bits[0]=RTK_FC_ALL_MAC_PORTMASK;
							aclRule.templateIdx=2U; /*use :tagIf=> template[2],field[5]*/


							aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
							aclField.fieldUnion.pattern.fieldIdx = 5U;/*template[2],field[5]*/
							aclField.fieldUnion.data.value = ACL_TAGIF_PPPoE_8864_BIT;
							aclField.fieldUnion.data.mask = (ACL_TAGIF_PPPoE_8864_BIT | ACL_TAGIF_OUT_IP4_BIT | ACL_TAGIF_IP6_BIT);
							if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
								ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							//force trap to CPU with priroity to avoid FB trap priroity replaced.
							aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
							aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
							aclRule.act.priAct.aclPri=fc_db.aclAndCfReservedRule.pppoe_lcp_assign_prioity.priority;
							aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
							aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
							if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
								ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							//point to next ruleIdx
							aclIdx++;

						}

					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_PPTP_GRE_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY:
					{
#if 0	//not verify yet
						if(fc_db.hypridPPTP.portmask){
							ACL_RSV("add RSV[%d] PPTP_GRE_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY @ acl[%d] - acl[%d]",type,aclIdx,(aclIdx+fc_db.aclAndCfReservedRule.hybrid_pptp_func.intf_count*3-1));
							loop_end = fc_db.aclAndCfReservedRule.hybrid_pptp_func.intf_count;
						}
						else{
							ACL_RSV("add RSV[%d] PPTP_GRE_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY @ acl[%d] - acl[%d]",type,aclIdx,aclIdx++);
							loop_end = 1;
						}

						for(i=0; i<loop_end; i++)
						{

							if(fc_db.hypridPPTP.portmask){
								if(!(fc_db.aclAndCfReservedRule.hybrid_pptp_func.gateway_mac_addr[i].octet[0]|fc_db.aclAndCfReservedRule.hybrid_pptp_func.gateway_mac_addr[i].octet[1]|
									fc_db.aclAndCfReservedRule.hybrid_pptp_func.gateway_mac_addr[i].octet[2]|fc_db.aclAndCfReservedRule.hybrid_pptp_func.gateway_mac_addr[i].octet[3]|
									fc_db.aclAndCfReservedRule.hybrid_pptp_func.gateway_mac_addr[i].octet[4]|fc_db.aclAndCfReservedRule.hybrid_pptp_func.gateway_mac_addr[i].octet[5]))
									continue;

								memcpy(&mac.octet, fc_db.aclAndCfReservedRule.hybrid_pptp_func.gateway_mac_addr[i].octet, ETH_ALEN);

								addRuleFailedFlag = _rtk_fc_aclAndCfReservedRule_hybridIntfRsvAdd(aclIdx, mac);
								if(addRuleFailedFlag){
									ACL_RSV("add reserved ACL&CF PPTP_GRE_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY failed!!!");
									addRuleFailedFlag=1;
									break;
								}

								//point to next ruleIdx
								aclIdx++;
							}

							//Set ACL rule for higher rx prioirty for PPTP GRE controal packet and LCP packet(IP protocol=47, inner none-ipv4 & none-ipv6, packet length range 0~80)
							bzero(&aclRule,sizeof(aclRule));
							bzero(&aclField,sizeof(aclField));
							bzero(&aclField2,sizeof(aclField2));
							bzero(&aclField3,sizeof(aclField3));

							bzero(&aclPktLengthRangeEntry, sizeof(aclPktLengthRangeEntry));
							aclPktLengthRangeEntry.index = RSV_ACL_PACKETRANGETABLE_IDX;
							aclPktLengthRangeEntry.upper_bound=80;
							aclPktLengthRangeEntry.lower_bound=0;
							aclPktLengthRangeEntry.type = PKTLENRANGE_NOTREVISE;

							if(rtk_acl_packetLengthRange_set(&aclPktLengthRangeEntry)){
								ACL_CTRL("access HW ACL PKTLEN_RANGE table failed");
							}

							aclRule.valid=ENABLED;
							aclRule.index=aclIdx;
							aclRule.activePorts.bits[0]=RTK_FC_ALL_MAC_PORTMASK;
							aclRule.templateIdx=2; /*use :TOS_PROTO and FRAME_TAG; tagIf=> template[2],field[5]*/

							aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
							aclField.fieldUnion.pattern.fieldIdx = 3;/*template[2],field[3]*/
							aclField.fieldUnion.data.value = 0x2f; //IPv4 protocal value = GRE 47
							aclField.fieldUnion.data.mask = 0xff;
							if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
								ACL_RSV("add reserved ACL&CF PPTP_GRE_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
							aclField2.fieldUnion.pattern.fieldIdx = 5;/*template[2],field[5]*/
							aclField2.fieldUnion.data.value = (~ACL_TAGIF_IN_IP4_BIT) & (~ACL_TAGIF_IP6_BIT);
							aclField2.fieldUnion.data.mask = (ACL_TAGIF_IN_IP4_BIT | ACL_TAGIF_IP6_BIT);
							if(rtk_acl_igrRuleField_add(&aclRule, &aclField2)){
								ACL_RSV("add reserved ACL&CF PPTP_GRE_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							aclField3.fieldType = ACL_FIELD_PATTERN_MATCH;
							aclField3.fieldUnion.pattern.fieldIdx = 7;/*template[2],field[7]*/
							aclField3.fieldUnion.data.value = (1<<(RSV_ACL_PACKETRANGETABLE_IDX));
							aclField3.fieldUnion.data.mask = (1<<(RSV_ACL_PACKETRANGETABLE_IDX));
							if(rtk_acl_igrRuleField_add(&aclRule, &aclField3)){
								ACL_RSV("add reserved ACL&CF PPTP_GRE_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							//force trap to CPU with priroity to avoid FB trap priroity replaced.
							aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
							aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
							aclRule.act.priAct.aclPri=fc_db.aclAndCfReservedRule.pptp_gre_ctrl_lcp_assign_prioity.priority;
							aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
							aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
							if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
								ACL_RSV("add reserved ACL&CF PPTP_GRE_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							//point to next ruleIdx
							aclIdx++;

							bzero(&aclRule,sizeof(aclRule));
							bzero(&aclField,sizeof(aclField));
							bzero(&aclField2,sizeof(aclField2));
							bzero(&aclField3,sizeof(aclField3));

							aclRule.valid=ENABLED;
							aclRule.index=aclIdx;
							aclRule.activePorts.bits[0]=RTK_FC_ALL_MAC_PORTMASK;
							aclRule.templateIdx=2; /*use :TC_NH and FRAME_TAG; tagIf=> template[2],field[5]*/

							aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
							aclField.fieldUnion.pattern.fieldIdx = 4;/*template[2],field[4]*/
							aclField.fieldUnion.data.value = 0x2f; //IPv6 next header = GRE 47
							aclField.fieldUnion.data.mask = 0xff;
							if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
								ACL_RSV("add reserved ACL&CF PPTP_GRE_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
							aclField2.fieldUnion.pattern.fieldIdx = 5;/*template[2],field[5]*/
							aclField2.fieldUnion.data.value = (~ACL_TAGIF_IN_IP4_BIT) & (~ACL_TAGIF_IP6_BIT);
							aclField2.fieldUnion.data.mask = (ACL_TAGIF_IN_IP4_BIT | ACL_TAGIF_IP6_BIT);
							if(rtk_acl_igrRuleField_add(&aclRule, &aclField2)){
								ACL_RSV("add reserved ACL&CF PPTP_GRE_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							aclField3.fieldType = ACL_FIELD_PATTERN_MATCH;
							aclField3.fieldUnion.pattern.fieldIdx = 7;/*template[2],field[7]*/
							aclField3.fieldUnion.data.value = (1<<(RSV_ACL_PACKETRANGETABLE_IDX));
							aclField3.fieldUnion.data.mask = (1<<(RSV_ACL_PACKETRANGETABLE_IDX));
							if(rtk_acl_igrRuleField_add(&aclRule, &aclField3)){
								ACL_RSV("add reserved ACL&CF PPTP_GRE_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							//force trap to CPU with priroity to avoid FB trap priroity replaced.
							aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
							aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
							aclRule.act.priAct.aclPri=fc_db.aclAndCfReservedRule.pptp_gre_ctrl_lcp_assign_prioity.priority;
							aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
							aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
							if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
								ACL_RSV("add reserved ACL&CF PPTP_GRE_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							//point to next ruleIdx
							aclIdx++;
						}
#endif
					}
					/* fall through */
				default:
					break;

			}

		}
	}

	fc_db.aclAndCfReservedRule.aclLowerBoundary=aclIdx;

	if(addRuleFailedFlag==1)
		return (RT_ERR_RG_FAILED);

	//reflash user ACL
	ASSERT_EQ(_rtk_fc_aclSWEntry_and_asic_rearrange(),RT_ERR_RG_OK);


	return (RT_ERR_RG_OK);
}

static int _rtk_fc_aclAndCfReservedRuleTailReflash(void)
{
	uint32 type;
	uint32 aclIdx=(fc_db.aclAndCfReservedRule.aclValidTableSize-1);
	rtk_acl_ingress_entry_t aclRule;
	rtk_acl_field_t aclField,aclField2;

	int addRuleFailedFlag=0;
	uint32 i;


	//reflash th rules
	for(i=fc_db.aclAndCfReservedRule.aclUpperBoundary;i<fc_db.aclAndCfReservedRule.aclValidTableSize;i++){
		assert_ok(rtk_acl_igrRuleEntry_del(i));
	}

	for(type=RTK_FC_ACLANDCF_RESERVED_HEAD_END;type<RTK_FC_ACLANDCF_RESERVED_TAIL_END;type++){
		if(fc_db.aclAndCfReservedRule.reservedMask[type]==ENABLED){
			switch(type){

				case RTK_FC_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT:
					{

					/*new default policy, drop unknownDA UDP multicast, trap else multicast*/
						ACL_RSV("add RSV[%d] MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT @ acl[%d] & acl[%d]",type,aclIdx,aclIdx-1);

						//rule for trap else multicast packet (such as unknownDA ICMP...etc)
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 7U; //dipv6[127:112] in template[6]:field[7]
						aclField.fieldUnion.data.value = 0xff00U; //dipv6[127:112]
						aclField.fieldUnion.data.mask = 0xff00U;

						if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
						{
							ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.valid = ENABLE;
						aclRule.index = aclIdx;
						aclRule.templateIdx = 6U; //dipv6 in template[6]
						aclRule.activePorts.bits[0] = RTK_FC_ALL_MAC_PORTMASK & (~(1<<RTK_FC_MAC_PORT_CPU));
						//trap to cpu action
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
						aclRule.act.forwardAct.act= ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type))
						{
							ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx--;


						//rule for drop unknownDA UDP multicast (use permit action, if unknownDA will dropped by HW reason 207)
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 7U; //dipv6[127:112] in template[6]:field[7]
						aclField.fieldUnion.data.value = 0xff0eU; //dipv6[127:112]
						aclField.fieldUnion.data.mask = 0xff0fU;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField))
						{
							ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.valid = ENABLE;
						aclRule.index = aclIdx;
						aclRule.templateIdx = 6U;//dipv6 in template[6]
						aclRule.activePorts.bits[0] = RTK_FC_ALL_MAC_PORTMASK & (~(1<<RTK_FC_MAC_PORT_CPU));
						//aclRule.careTag.tags[ACL_CARE_TAG_UDP].value=ENABLED;
						//aclRule.careTag.tags[ACL_CARE_TAG_UDP].mask=0xffff;
						//permit, used to avoid next trap action
						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT] = ENABLE;
						//aclRule.act.forwardAct.act= ACL_IGR_FORWARD_COPY_ACT;
						aclRule.act.forwardAct.act= ACL_IGR_FORWARD_EGRESSMASK_ACT; //permit action
						aclRule.act.forwardAct.portMask.bits[0]=RTK_FC_ALL_MAC_PORTMASK;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type))
						{
							ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						//point to next ruleIdx
						aclIdx--;
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP:
					{
						if(fc_db.hypridPPTP.portmask)	// 2 ACL is added, trap control packet(IGMP/MLD)to make sure multicase feature.
							ACL_RSV("add RSV[%d] CONTROL_PACKET_TRAP @ acl[%d] & acl[%d]",type,aclIdx,aclIdx-1);
						else							// 3 ACL is added, trap control packet(ex:IPCP/IP6CP) which could be forwarded by PATH1 flow which is created by other packet.
							ACL_RSV("add RSV[%d] CONTROL_PACKET_TRAP @ acl[%d] & acl[%d] & acl[%d]",type,aclIdx,aclIdx-1,aclIdx-2);


						//rule3:PPPoE tagif=1, action=trap
						if(fc_db.hypridPPTP.portmask == 0x0ULL){
							bzero(&aclRule,sizeof(aclRule));
							bzero(&aclField,sizeof(aclField));
							aclRule.valid=ENABLED;
							aclRule.index=aclIdx;
							aclRule.activePorts.bits[0]=RTK_FC_ALL_MAC_PORTMASK;
							aclRule.templateIdx=2U; /*use :FRAME_TAG*/


							aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
							aclField.fieldUnion.pattern.fieldIdx = 5U;/*template[2],field[5]*/
							aclField.fieldUnion.data.value = ACL_TAGIF_PPPoE_8863_8864_BIT ;
							aclField.fieldUnion.data.mask = (ACL_TAGIF_PPPoE_8863_8864_BIT|ACL_TAGIF_OUT_IP4_BIT|ACL_TAGIF_IP6_BIT);
							if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
								ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
							aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
							if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
								ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP failed!!!");
								addRuleFailedFlag=1;
								break;
							}

							aclIdx--;
						}


						//rule2: IPv4 tagif=1 + IGMP, action=trap
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=RTK_FC_ALL_MAC_PORTMASK;
						aclRule.templateIdx=2U; /*use :FRAME_TAG*/


						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 5U;/*template[2],field[5]*/
						aclField.fieldUnion.data.value = (ACL_TAGIF_OUT_IP4_BIT|ACL_TAGIF_IGMP_MLD_BIT) ;
						aclField.fieldUnion.data.mask = (ACL_TAGIF_OUT_IP4_BIT|ACL_TAGIF_IGMP_MLD_BIT);
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclIdx--;


						//rule1: IPv6 tagif=1 + MLD, action=trap
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=RTK_FC_ALL_MAC_PORTMASK;
						aclRule.templateIdx=2U; /*use :FRAME_TAG*/


						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 5U;/*template[2],field[5]*/
						aclField.fieldUnion.data.value = (ACL_TAGIF_IP6_BIT|ACL_TAGIF_IGMP_MLD_BIT) ;
						aclField.fieldUnion.data.mask = (ACL_TAGIF_IP6_BIT|ACL_TAGIF_IGMP_MLD_BIT);
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT;
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclIdx--;
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_ICMPV6_TRAP:
					{
						if(fc_db.hypridPPTP.portmask){
							ACL_RSV("skip RSV[%d] ICMPV6_TRAP due to hyprid function enable (portmask 0x%llx)",type,fc_db.hypridPPTP.portmask);
							break;
						}else	//which could be forwarded by PATH1 flow which is created by other packet.
							ACL_RSV("add RSV[%d] ICMPV6_TRAP @ acl[%d]",type,aclIdx);

						//IPv6 tagif=1 and l4protocol=0x003a (ICMPv6), action=trap
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						bzero(&aclField2,sizeof(aclField2));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=RTK_FC_ALL_MAC_PORTMASK;
						aclRule.templateIdx=2U; /*use :TC_NH and FRAME_TAG*/

						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 4U;/*template[2],field[4]*/
						aclField.fieldUnion.data.value = 0x3a; //NH protocal value
						aclField.fieldUnion.data.mask = 0xff;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_ICMPV6_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField2.fieldUnion.pattern.fieldIdx = 5U;/*template[2],field[5]*/
						aclField2.fieldUnion.data.value = ACL_TAGIF_IP6_BIT; //outter IPv4 tagIf
						aclField2.fieldUnion.data.mask = ACL_TAGIF_IP6_BIT;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField2)){
							ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_ICMPV6_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

					#if defined(CONFIG_RTK_SOC_RTL8198D)
						aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
						aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
						aclRule.act.priAct.aclPri = 7U;
					#endif

						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT; //trap action
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_ICMPV6_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclIdx--;
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_ICMPV4_TRAP:
					{
						if(fc_db.hypridPPTP.portmask){
							ACL_RSV("skip RSV[%d] ICMPV4_TRAP due to hyprid function enable (portmask 0x%llx)",type,fc_db.hypridPPTP.portmask);
							break;
						}else	//which could be forwarded by PATH1 flow which is created by other packet.
							ACL_RSV("add RSV[%d] ICMPV4_TRAP @ acl[%d]",type,aclIdx);

						//IPv4 tagif=1and l4protocol=0x0001(ICMP), action=trap
						bzero(&aclRule,sizeof(aclRule));
						bzero(&aclField,sizeof(aclField));
						bzero(&aclField2,sizeof(aclField2));
						aclRule.valid=ENABLED;
						aclRule.index=aclIdx;
						aclRule.activePorts.bits[0]=RTK_FC_ALL_MAC_PORTMASK;
						aclRule.templateIdx=2U; /*use :TOS_PROTO and FRAME_TAG*/

						aclField.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField.fieldUnion.pattern.fieldIdx = 3U;/*template[2],field[3]*/
						aclField.fieldUnion.data.value = 0x01U; //IP protocal value
						aclField.fieldUnion.data.mask = 0xffU;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField)){
							ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_ICMPV4_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}
						aclField2.fieldType = ACL_FIELD_PATTERN_MATCH;
						aclField2.fieldUnion.pattern.fieldIdx = 5U;/*template[2],field[5]*/
						aclField2.fieldUnion.data.value = ACL_TAGIF_OUT_IP4_BIT; //outter IPv4 tagIf
						aclField2.fieldUnion.data.mask = ACL_TAGIF_OUT_IP4_BIT;
						if(rtk_acl_igrRuleField_add(&aclRule, &aclField2)){
							ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_ICMPV4_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

					#if defined(CONFIG_RTK_SOC_RTL8198D)
						aclRule.act.enableAct[ACL_IGR_PRI_ACT]=ENABLED;
						aclRule.act.priAct.act=ACL_IGR_PRI_ACL_PRI_ASSIGN_ACT;
						aclRule.act.priAct.aclPri = 7U;
					#endif

						aclRule.act.enableAct[ACL_IGR_FORWARD_ACT]=ENABLED;
						aclRule.act.forwardAct.act=ACL_IGR_FORWARD_TRAP_ACT; //trap action
						if(RTK_ACL_IGRRULEENTRY_ADD(&aclRule, type)){
							ACL_RSV("add reserved ACL&CF RTK_FC_ACLANDCF_RESERVED_ICMPV4_TRAP failed!!!");
							addRuleFailedFlag=1;
							break;
						}

						aclIdx--;
					}
					break;

				default:
					break;
			}
		}
	}

	fc_db.aclAndCfReservedRule.aclUpperBoundary=aclIdx;
	if(addRuleFailedFlag==1)
		return (RT_ERR_RG_FAILED);

	return (RT_ERR_RG_OK);

}



int _rtk_fc_aclAndCfReservedRuleReflash(void)
{

	//using ACL[0] for multicast
	_rtk_fc_acl_for_multicast_temp_protection_add();

	_rtk_fc_aclAndCfReservedRuleHeadReflash();
	_rtk_fc_aclAndCfReservedRuleTailReflash();

	_rtk_fc_acl_for_multicast_temp_protection_del();

	return RT_ERR_RG_OK;
}

int _rtk_rg_aclAndCfReservedRuleAdd(rtk_rg_aclAndCf_reserved_type_t rsvType, void *parameter)
{
	ACL_RSV("#####Reserved ACL reflash!(add reserved ACL rsvType=%d)#####",rsvType);

	if((rsvType < RTK_FC_ACLANDCF_RESERVED_TAIL_END) && (fc_db.aclAndCfReservedRule.reservedMask[rsvType]==ENABLED))
		return (RT_ERR_RG_OK);

	switch(rsvType){

		case RTK_FC_ACLANDCF_RESERVED_ALL_TRAP:
		case RTK_FC_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP:
			assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(1U,0U));
			fc_db.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_FC_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY:
			if(fc_db.hypridPPTP.portmask)
				assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(fc_db.aclAndCfReservedRule.hybrid_pptp_func.intf_count*2U,0U));
			else
				assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(1U,0U));
			memcpy(&fc_db.aclAndCfReservedRule.pppoe_lcp_assign_prioity,(rtk_fc_aclAndCf_reserved_assign_priority_t*)parameter,sizeof(rtk_fc_aclAndCf_reserved_assign_priority_t));
			fc_db.aclAndCfReservedRule.reservedMask[RTK_FC_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY]=ENABLED;
			break;

		case RTK_FC_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY:
			if(fc_db.hypridPPTP.portmask)
				assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(fc_db.aclAndCfReservedRule.hybrid_pptp_func.intf_count*2U,0U));
			else
				assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(1U,0U));
			memcpy(&fc_db.aclAndCfReservedRule.l2tp_ctrl_lcp_assign_prioity,(rtk_fc_aclAndCf_reserved_assign_priority_t*)parameter,sizeof(rtk_fc_aclAndCf_reserved_assign_priority_t));
			fc_db.aclAndCfReservedRule.reservedMask[RTK_FC_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY]=ENABLED;
			break;

		case RTK_FC_ACLANDCF_RESERVED_PPTP_GRE_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY:
			if(fc_db.hypridPPTP.portmask)
				assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(fc_db.aclAndCfReservedRule.hybrid_pptp_func.intf_count*3U,0U));
			else
				assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(1U,0U));
			memcpy(&fc_db.aclAndCfReservedRule.pptp_gre_ctrl_lcp_assign_prioity,(rtk_fc_aclAndCf_reserved_assign_priority_t*)parameter,sizeof(rtk_fc_aclAndCf_reserved_assign_priority_t));
			fc_db.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_FC_ACLANDCF_RESERVED_ICMPV4_TRAP:
		case RTK_FC_ACLANDCF_RESERVED_ICMPV6_TRAP:
			assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(1U,0U));
			fc_db.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_FC_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP:
			if(fc_db.hypridPPTP.portmask)
				assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(2U,0U));
			else
				assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(3U,0U));
			fc_db.aclAndCfReservedRule.reservedMask[RTK_FC_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP]=ENABLED;
			break;

		case RTK_FC_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT:
			assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(2U,0U));
			fc_db.aclAndCfReservedRule.reservedMask[RTK_FC_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT]=ENABLED;
			break;

		default:
			break;
	}

	_rtk_fc_aclAndCfReservedRuleReflash();

	ACL_RSV("RESERVED_ACL_BEFORE=%d, RESERVED_ACL_AFTER=%d",fc_db.aclAndCfReservedRule.aclLowerBoundary,fc_db.aclAndCfReservedRule.aclUpperBoundary);


	return (RT_ERR_RG_OK);

}


int _rtk_rg_aclAndCfReservedRuleDel(rtk_rg_aclAndCf_reserved_type_t rsvType)
{

	ACL_RSV("#####Reserved ACL reflash!(del reserved ACL rsvType=%d)#####",rsvType);

	if(fc_db.aclAndCfReservedRule.reservedMask[rsvType]==DISABLED)
		return (RT_ERR_RG_OK);

	fc_db.aclAndCfReservedRule.reservedMask[rsvType]=DISABLED;

	_rtk_fc_aclAndCfReservedRuleReflash();

	return (RT_ERR_RG_OK);
}
#endif //CONFIG_RTK_L34_XPON_PLATFORM
#if defined(CONFIG_RTK_L34_G3_PLATFORM)
/*(0)Internal Definition*/
#define PATTERN_CHECK_DMAC_IS_MC(acl_filter) \
	((acl_filter->ingress_dmac[0]==0x1) && (acl_filter->ingress_dmac_mask[0]==0x1) &&  \
		((acl_filter->ingress_dmac[1]|acl_filter->ingress_dmac[2]|acl_filter->ingress_dmac[3]| \
			acl_filter->ingress_dmac[4]|acl_filter->ingress_dmac[5])==0x0) && \
		((acl_filter->ingress_dmac_mask[1]|acl_filter->ingress_dmac_mask[2]|acl_filter->ingress_dmac_mask[3]| \
			acl_filter->ingress_dmac_mask[4]|acl_filter->ingress_dmac_mask[5])==0x0))

#define PATTERN_CHECK_DMAC_IS_MC_IPV4(acl_filter) \
	((acl_filter->ingress_dmac[0]==0x1) && (acl_filter->ingress_dmac[1]==0x0) && (acl_filter->ingress_dmac[2]==0x5E) && \
		(acl_filter->ingress_dmac_mask[0]==0xff) && (acl_filter->ingress_dmac_mask[1]==0xff) && (acl_filter->ingress_dmac_mask[2]==0xff) &&  \
		((acl_filter->ingress_dmac[3]|acl_filter->ingress_dmac[4]|acl_filter->ingress_dmac[5])==0x0) && \
		((acl_filter->ingress_dmac_mask[3]|acl_filter->ingress_dmac_mask[4]|acl_filter->ingress_dmac_mask[5])==0x0))

#define PATTERN_CHECK_DMAC_IS_MC_IPV6(acl_filter) \
	((acl_filter->ingress_dmac[0]==0x33) && (acl_filter->ingress_dmac[1]==0x33) && \
		(acl_filter->ingress_dmac_mask[0]==0xff) && (acl_filter->ingress_dmac_mask[1]==0xff) && \
		((acl_filter->ingress_dmac[2]|acl_filter->ingress_dmac[3]|acl_filter->ingress_dmac[4]|acl_filter->ingress_dmac[5])==0x0) && \
		((acl_filter->ingress_dmac_mask[2]|acl_filter->ingress_dmac_mask[3]|acl_filter->ingress_dmac_mask[4]|acl_filter->ingress_dmac_mask[5])==0x0))

#define PATTERN_CHECK_DMAC_IS_UC(acl_filter) \
	((acl_filter->ingress_dmac[0]==0x0) && (acl_filter->ingress_dmac_mask[0]==0x1) &&  \
		((acl_filter->ingress_dmac[1]|acl_filter->ingress_dmac[2]|acl_filter->ingress_dmac[3]| \
			acl_filter->ingress_dmac[4]|acl_filter->ingress_dmac[5])==0x0) && \
		((acl_filter->ingress_dmac_mask[1]|acl_filter->ingress_dmac_mask[2]|acl_filter->ingress_dmac_mask[3]| \
			acl_filter->ingress_dmac_mask[4]|acl_filter->ingress_dmac_mask[5])==0x0))

#define PATTERN_CHECK_DMAC_IS_RSVD(acl_filter) \
	((acl_filter->ingress_dmac[0]==0x1) && (acl_filter->ingress_dmac[1]==0x80) && (acl_filter->ingress_dmac[2]==0xC2) && \
		(acl_filter->ingress_dmac_mask[0]==0xff) && (acl_filter->ingress_dmac_mask[1]==0xff) && (acl_filter->ingress_dmac_mask[2]==0xff) && \
		(acl_filter->ingress_dmac_mask[3]==0xff) && (acl_filter->ingress_dmac_mask[4]==0xff) &&  \
		((acl_filter->ingress_dmac[3]|acl_filter->ingress_dmac[4]|acl_filter->ingress_dmac[5]|acl_filter->ingress_dmac_mask[5])==0x0))

#if defined(CONFIG_FC_G3_G3LITE_SERIES)
#define ACL_CHECK_HASH_NEED(acl_filter) \
	( ((acl_filter->ingress_tcp_flags_mask == 0xfff) && (acl_filter->ingress_tcp_flags == 0x2 /*syn only*/)) || \
		((acl_filter->ingress_tcp_flags_mask == 0xfff) && (acl_filter->ingress_tcp_flags == 0x12 /*syn ack*/)) )
#endif

/*(1)ACL init related APIs*/
int _rtk_fc_acl_asic_init(void)
{
	/*init TEMPLATE, ref: aal_l3_cls.c*/

	//aclTemplate[0]: CL_IF_ID_KEY_MSK
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IF_ID_KEY_MSK] |= (1<<RTK_ACL_FIELD_IP_VER);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IF_ID_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VID);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IF_ID_KEY_MSK] |= (1<<RTK_ACL_FIELD_VLAN_CNT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IF_ID_KEY_MSK] |= (1<<RTK_ACL_FIELD_ETHERTYPE_ENC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IF_ID_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_IP_MC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IF_ID_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_BC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IF_ID_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_MC);

	//aclTemplate[1]: CL_IPV4_TUNNEL_ID_KEY_MSK
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK] |= (1<<RTK_ACL_FIELD_L4_PORT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK] |= (1<<RTK_ACL_FIELD_L4_VLD);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV4_DA);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV4_SA);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK] |= (1<<RTK_ACL_FIELD_IP_PROTOCOL);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK] |= (1<<RTK_ACL_FIELD_IP_VER);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VID);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK] |= (1<<RTK_ACL_FIELD_VLAN_CNT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK] |= (1<<RTK_ACL_FIELD_ETHERTYPE_ENC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_IP_MC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_BC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_TUNNEL_ID_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_MC);

	//aclTemplate[2]: CL_IPV4_SA_SHORT_KEY_MSK = CL_IPV4_SHORT_KEY_MSK + CLS_KEY_MSK_IPV4_SA
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_SA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_L4_PORT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_SA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_L4_VLD);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_SA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_IP_PROTOCOL);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_SA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_DSCP);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_SA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_DEI);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_SA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VL_802_1P);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_SA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VID);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_SA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_VLAN_CNT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_SA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV4_SA);

	//aclTemplate[3]: CL_IPV4_DA_SHORT_KEY_MSK = CL_IPV4_SHORT_KEY_MSK + CLS_KEY_MSK_IPV4_DA
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_DA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_L4_PORT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_DA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_L4_VLD);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_DA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_IP_PROTOCOL);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_DA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_DSCP);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_DA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_DEI);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_DA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VL_802_1P);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_DA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VID);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_DA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_VLAN_CNT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV4_DA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV4_DA);

	//aclTemplate[4]: CL_IPV6_SA_SHORT_KEY_MSK = CL_IPV6_SHORT_KEY_MSK + CLS_KEY_MSK_IPV6_SA
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV6_SA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_L4_PORT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV6_SA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_L4_VLD);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV6_SA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV6_FLOW_LBL);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV6_SA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_IP_PROTOCOL);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV6_SA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_DSCP);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV6_SA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_DEI);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV6_SA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VL_802_1P);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV6_SA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VID);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV6_SA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_VLAN_CNT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV6_SA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV6_SA);

	//aclTemplate[5]: CL_IPV6_DA_SHORT_KEY_MSK = CL_IPV6_SHORT_KEY_MSK + CLS_KEY_MSK_IPV6_DA
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV6_DA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_L4_PORT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV6_DA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_L4_VLD);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV6_DA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV6_FLOW_LBL);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV6_DA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_IP_PROTOCOL);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV6_DA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_DSCP);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV6_DA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_DEI);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV6_DA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VL_802_1P);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV6_DA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VID);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV6_DA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_VLAN_CNT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_IPV6_DA_SHORT_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV6_DA);

	//aclTemplate[6]: CL_SPCL_PKT_KEY_MSK
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_SPCL_PKT_KEY_MSK] |= (1<<RTK_ACL_FIELD_IP_PROTOCOL);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_SPCL_PKT_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_DEI);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_SPCL_PKT_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VL_802_1P);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_SPCL_PKT_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VID);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_SPCL_PKT_KEY_MSK] |= (1<<RTK_ACL_FIELD_VLAN_CNT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_SPCL_PKT_KEY_MSK] |= (1<<RTK_ACL_FIELD_ETHERTYPE_ENC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_SPCL_PKT_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_SPCL_PKT_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_IP_MC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_SPCL_PKT_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_BC);	
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_SPCL_PKT_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_MC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_SPCL_PKT_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_RSVD);

	//aclTemplate[7]: CL_MCST_MAC_DA_KEY_MSK = CL_MCST_KEY_MSK + CLS_KEY_MSK_MAC_DA
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_MCST_MAC_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IP_VER);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_MCST_MAC_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_DEI);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_MCST_MAC_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VL_802_1P);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_MCST_MAC_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VID);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_MCST_MAC_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_VLAN_CNT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_MCST_MAC_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_MC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_MCST_MAC_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_IP_MC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_MCST_MAC_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_BC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_MCST_MAC_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA);

	//aclTemplate[8]: CL_MCST_IP_DA_KEY_MSK = CL_MCST_KEY_MSK + CLS_KEY_MSK_IPV4_DA + CLS_KEY_MSK_IPV6_DA
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_MCST_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IP_VER);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_MCST_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_DEI);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_MCST_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VL_802_1P);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_MCST_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VID);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_MCST_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_VLAN_CNT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_MCST_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_MC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_MCST_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_IP_MC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_MCST_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_BC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_MCST_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV4_DA);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_MCST_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV6_DA);

	//aclTemplate[9]: CL_FULL_KEY_MSK
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<RTK_ACL_FIELD_L4_PORT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<RTK_ACL_FIELD_L4_VLD);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV6_DA);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV6_SA);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV4_DA);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV4_SA);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<RTK_ACL_FIELD_IP_PROTOCOL);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<RTK_ACL_FIELD_DSCP);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<RTK_ACL_FIELD_IP_VER);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_DEI);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VL_802_1P);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VID);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<RTK_ACL_FIELD_VLAN_CNT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<RTK_ACL_FIELD_ETHERTYPE_ENC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_SA);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_MC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_IP_MC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_BC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_RSVD);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<RTK_ACL_FIELD_TCP_RDP_CTRL);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<RTK_ACL_FIELD_PKTLEN_RNG_MATCH_VEC);
#if !defined(CONFIG_FC_G3_G3LITE_SERIES)
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FULL_KEY_MSK] |= (1<<RTK_ACL_FIELD_IP_RANGE);

	//aclTemplate[10]: RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK = CL_FLOW_KEY_MSK + CLS_KEY_MSK_MAC_SA + CLS_KEY_MSK_MAC_DA + CLS_KEY_MSK_IPV4_SA + CLS_KEY_MSK_IPV6_SA
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_TCP_RDP_CTRL);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_L4_PORT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_L4_VLD);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV6_FLOW_LBL);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IP_PROTOCOL);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_DSCP);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IP_VER);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_DEI);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VL_802_1P);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VID);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_VLAN_CNT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_PKTLEN_RNG_MATCH_VEC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_ETHERTYPE_ENC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_MC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_RSVD);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_IP_MC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_BC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_SA);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV4_SA);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV6_SA);

	//aclTemplate[11]: RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK = CL_FLOW_KEY_MSK + CLS_KEY_MSK_MAC_SA + CLS_KEY_MSK_MAC_DA + CLS_KEY_MSK_IPV4_DA + CLS_KEY_MSK_IPV6_DA
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_TCP_RDP_CTRL);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_L4_PORT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_L4_VLD);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV6_FLOW_LBL);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IP_PROTOCOL);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_DSCP);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IP_VER);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_DEI);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VL_802_1P);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VID);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_VLAN_CNT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_PKTLEN_RNG_MATCH_VEC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_ETHERTYPE_ENC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_MC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_RSVD);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_IP_MC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_BC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_SA);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV4_DA);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV6_DA);

	//aclTemplate[12]: RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_SA_KEY_MSK = CL_FLOW_KEY_MSK + CLS_KEY_MSK_MAC_RANGE + CLS_KEY_MSK_IPV4_SA + CLS_KEY_MSK_IPV6_SA
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_TCP_RDP_CTRL);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_L4_PORT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_L4_VLD);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV6_FLOW_LBL);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IP_PROTOCOL);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_DSCP);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IP_VER);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_DEI);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VL_802_1P);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VID);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_VLAN_CNT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_PKTLEN_RNG_MATCH_VEC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_ETHERTYPE_ENC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_MC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_RSVD);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_IP_MC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_BC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_RANGE);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV4_SA);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_SA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV6_SA);

	//aclTemplate[13]: RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_DA_KEY_MSK = CL_FLOW_KEY_MSK + CLS_KEY_MSK_MAC_RANGE + CLS_KEY_MSK_IPV4_DA + CLS_KEY_MSK_IPV6_DA
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_TCP_RDP_CTRL);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_L4_PORT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_L4_VLD);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV6_FLOW_LBL);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IP_PROTOCOL);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_DSCP);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IP_VER);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_DEI);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VL_802_1P);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_TOP_VID);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_VLAN_CNT);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_PKTLEN_RNG_MATCH_VEC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_ETHERTYPE_ENC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_MC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_RSVD);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_IP_MC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_DA_BC);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_MAC_RANGE);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV4_DA);
	fc_db.ca_cls_template[RTK_ACL_TEMPLATE_CL_FLOW_MAC_RANGE_IP_DA_KEY_MSK] |= (1<<RTK_ACL_FIELD_IPV6_DA);
#endif

	{
		uint8 rsv_type;
		for(rsv_type = 0; rsv_type<RTK_FC_ACLANDCF_RESERVED_TAIL_END; rsv_type++)
			snprintf(fc_db.nameRsvAclType[rsv_type], sizeof(fc_db.nameRsvAclType[0]), "%s", "UNDEFINE");
	}
	snprintf(fc_db.nameRsvAclType[RTK_CA_CLS_TYPE_L2_INGRESS_FORWARD_L3FE], sizeof(fc_db.nameRsvAclType[0]), "%s", "L2_INGRESS_FORWARD_L3FE");
	snprintf(fc_db.nameRsvAclType[RTK_CA_CLS_TYPE_GENERIC_INTF_HASH_PROFILE_DECISION], sizeof(fc_db.nameRsvAclType[0]), "%s", "GENERIC_INTF_HASH_PROFILE_DECISION");
	snprintf(fc_db.nameRsvAclType[RTK_CA_CLS_TYPE_GENERIC_INTF_MC_PROFILE_WITH_DMAC], sizeof(fc_db.nameRsvAclType[0]), "%s", "GENERIC_INTF_MC_PROFILE_WITH_DMAC");
	snprintf(fc_db.nameRsvAclType[RTK_CA_CLS_TYPE_L2_UPSTREAM_LOOPBACK_INGRESS_FORWARD_L3FE], sizeof(fc_db.nameRsvAclType[0]), "%s", "L2_UPSTREAM_LOOPBACK_INGRESS_FORWARD_L3FE");
	snprintf(fc_db.nameRsvAclType[RTK_CA_CLS_TYPE_L2_UPSTREAM_LAN_INGRESS_FORWARD_LOOPBACK], sizeof(fc_db.nameRsvAclType[0]), "%s", "L2_UPSTREAM_LAN_INGRESS_FORWARD_LOOPBACK");
	snprintf(fc_db.nameRsvAclType[RTK_CA_CLS_TYPE_L2_INGRESS_SA_HOSTPOLICING], sizeof(fc_db.nameRsvAclType[0]), "%s", "L2_INGRESS_SA_HOSTPOLICING");
	snprintf(fc_db.nameRsvAclType[RTK_CA_CLS_TYPE_L2_EGRESS_DA_HOSTPOLICING], sizeof(fc_db.nameRsvAclType[0]), "%s", "L2_EGRESS_DA_HOSTPOLICING");
	snprintf(fc_db.nameRsvAclType[RTK_CA_CLS_TYPE_REARRANGE_PROTECTION], sizeof(fc_db.nameRsvAclType[0]), "%s", "REARRANGE_PROTECTION");
	snprintf(fc_db.nameRsvAclType[RTK_CA_CLS_TYPE_BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE], sizeof(fc_db.nameRsvAclType[0]), "%s", "BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE");
	snprintf(fc_db.nameRsvAclType[RTK_CA_CLS_TYPE_BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE_GW_MAC_UPDATE], sizeof(fc_db.nameRsvAclType[0]), "%s", "BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE_GW_MAC_UPDATE");
	snprintf(fc_db.nameRsvAclType[RTK_CA_CLS_TYPE_VXLAN_FFD_US], sizeof(fc_db.nameRsvAclType[0]), "%s", "VXLAN_FFD_US");
	snprintf(fc_db.nameRsvAclType[RTK_CA_CLS_TYPE_VXLAN_FFD_DS], sizeof(fc_db.nameRsvAclType[0]), "%s", "VXLAN_FFD_DS");
	snprintf(fc_db.nameRsvAclType[RTK_CA_CLS_TYPE_VXLAN_FFD_EXTRA_DS], sizeof(fc_db.nameRsvAclType[0]), "%s", "VXLAN_FFD_EXTRA_DS");
	snprintf(fc_db.nameRsvAclType[RTK_CLS_TYPE_USER_ACL_REARRANGE], sizeof(fc_db.nameRsvAclType[0]), "%s", "USER_ACL_REARRANGE");
	snprintf(fc_db.nameRsvAclType[RTK_CLS_TYPE_HTTP_ACCELERATE_BY_PE], sizeof(fc_db.nameRsvAclType[0]), "%s", "HPPT_ACCELERATE_BY_PE");
	snprintf(fc_db.nameRsvAclType[RTK_CLS_TYPE_SRV6_ACCELERATE_BY_PE], sizeof(fc_db.nameRsvAclType[0]), "%s", "SRV6_ACCELERATE_BY_PE");
	snprintf(fc_db.nameRsvAclType[RTK_CLS_TYPE_RESET_L2_L3_CLS], sizeof(fc_db.nameRsvAclType[0]), "%s", "RESET_L2_L3_CLS");
	snprintf(fc_db.nameRsvAclType[RTK_CLS_TYPE_L2_INGRESS_FORWARD_PORT], sizeof(fc_db.nameRsvAclType[0]), "%s", "L2_INGRESS_FORWARD_PORT");
	snprintf(fc_db.nameRsvAclType[RTK_CLS_TYPE_L2_INGRESS_MC_FORWARD_L3FE], sizeof(fc_db.nameRsvAclType[0]), "%s", "L2_INGRESS_MC_FORWARD_L3FE");
	snprintf(fc_db.nameRsvAclType[RTK_CLS_TYPE_L2_EGRESS_FORWARD_L3FE], sizeof(fc_db.nameRsvAclType[0]), "%s", "L2_EGRESS_FORWARD_L3FE");
	snprintf(fc_db.nameRsvAclType[RTK_CLS_TYPE_DUAL_HEADER_DS_CHECK], sizeof(fc_db.nameRsvAclType[0]), "%s", "DUAL_HEADER_DS_CHECK");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_SPECIAL_END], sizeof(fc_db.nameRsvAclType[0]), "%s", "RESERVED_SPECIAL_END");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_ALL_TRAP], sizeof(fc_db.nameRsvAclType[0]), "%s", "ALL_TRAP");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_SNAP_OTHER_TRAP], sizeof(fc_db.nameRsvAclType[0]), "%s", "SNAP_OTHER_TRAP");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_BURST_PACKET_SEND_MCE], sizeof(fc_db.nameRsvAclType[0]), "%s", "BURST_PACKET_SEND_MCE");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_WIFI_SNAP_TRANS], sizeof(fc_db.nameRsvAclType[0]), "%s", "WIFI_SNAP_TRANS");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_WIFI_TXAMSDU_HW_LOOKUP], sizeof(fc_db.nameRsvAclType[0]), "%s", "WIFI_TXAMSDU_HW_LOOKUP");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_WIFI_TXAMSDU_MC_TO_UC], sizeof(fc_db.nameRsvAclType[0]), "%s", "WIFI_TXAMSDU_MC_TO_UC");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_IPSEC_ACCELERATE_BY_PE], sizeof(fc_db.nameRsvAclType[0]), "%s", "IPSEC_ACCELERATE_BY_PE");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_ACL_LAN_PORTMASK_RESET], sizeof(fc_db.nameRsvAclType[0]), "%s", "ACL_LAN_PORTMASK_RESET");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP], sizeof(fc_db.nameRsvAclType[0]), "%s", "MULTICAST_SSDP_TRAP");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_MULTICAST_RIP_TRAP], sizeof(fc_db.nameRsvAclType[0]), "%s", "MULTICAST_RIP_TRAP");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY], sizeof(fc_db.nameRsvAclType[0]), "%s", "PPPoE_LCP_PACKET_ASSIGN_PRIORITY");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY], sizeof(fc_db.nameRsvAclType[0]), "%s", "L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_PPTP_GRE_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY], sizeof(fc_db.nameRsvAclType[0]), "%s", "PPTP_GRE_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_HEAD_END], sizeof(fc_db.nameRsvAclType[0]), "%s", "HEAD_END");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_MC_JUMBO_DROP], sizeof(fc_db.nameRsvAclType[0]), "%s", "MC_JUMBO_DROP");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_TTL_LESS_ONE_TRAP], sizeof(fc_db.nameRsvAclType[0]), "%s", "TTL_LESS_ONE_TRAP");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_SMP_DISPATCH_NIC_RX_BY_ACL], sizeof(fc_db.nameRsvAclType[0]), "%s", "SMP_DISPATCH_NIC_RX_BY_ACL");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_MULTICAST_DSLITE_TRAP], sizeof(fc_db.nameRsvAclType[0]), "%s", "MULTICAST_DSLITE_TRAP");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_L2TP_DATA_LOWER_PRIORITY], sizeof(fc_db.nameRsvAclType[0]), "%s", "L2TP_DATA_LOWER_PRIORITY");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_ICMPV4_TRAP], sizeof(fc_db.nameRsvAclType[0]), "%s", "ICMPV4_TRAP");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_ICMPV6_TRAP], sizeof(fc_db.nameRsvAclType[0]), "%s", "ICMPV6_TRAP");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_DHCPV4_TRAP], sizeof(fc_db.nameRsvAclType[0]), "%s", "DHCPV4_TRAP");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP], sizeof(fc_db.nameRsvAclType[0]), "%s", "CONTROL_PACKET_TRAP");
	snprintf(fc_db.nameRsvAclType[RTK_FC_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT], sizeof(fc_db.nameRsvAclType[0]), "%s", "MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT");

	return (RT_ERR_RG_OK);
}

/*(2)RG_ACL APIs and internal APIs*/
#if defined(FC_USER_ACL_CA_CLS_SUPPORT)
void _rtk_fc_g3_caClsAssignRuleRefIdx(ca_classifier_rule_t *cfg, int ruleType, char *debug_str, int debug_str_size)
{
	//reserve mdata_w_2 to ACL, ca api will transform sw_id[0] to mdata_w_2
	if(ruleType == FAIL){
		snprintf(debug_str, debug_str_size, "pre-add test");
	}else if(ruleType >= RTK_FC_ACLANDCF_RESERVED_TAIL_END){
		snprintf(debug_str, debug_str_size, "SW_ACL[%d]", ruleType - RTK_FC_ACLANDCF_RESERVED_TAIL_END);
#if	!defined(CONFIG_ARCH_CORTINA_G3LITE)
		//this feature will cause CL_IF_ID_KEY(0.25 entry size) unused, so g3 lite not support this feature due to limite cls resource
		if((cfg->key_mask.src_port) && (PORT_TYPE(cfg->key.src_port)==CA_PORT_TYPE_L3))
		{
			cfg->action.options.sw_id[0] |= ruleType - RTK_FC_ACLANDCF_RESERVED_TAIL_END;
			cfg->action.options.sw_id[0] |= RXINFO_REF_ACL_RSN_CTRL_USER;
			if((cfg->action.forward == CA_CLASSIFIER_FORWARD_PORT) && ((1<<(cfg->action.dest.port)) & RTK_FC_ALL_MAC_PURECPU_PORTMASK))
			{
				cfg->action.options.sw_id[0] |= RXINFO_REF_ACL_RSN_CTRL_TRAP;
				if((cfg->action.options.sw_id[0] >> RXINFO_REF_TRAP_RSN_SHIFT_BIT) != RXINFO_REF_TRAP_RSN_ACL_TO_PS)
					cfg->action.options.sw_id[0] |= RXINFO_REF_TRAP_RSN_ACL<<RXINFO_REF_TRAP_RSN_SHIFT_BIT;
			}
			cfg->action.options.masks.sw_id = TRUE;
		}
#endif
	}else{
		snprintf(debug_str, debug_str_size, "RSV_ACL[%d]", ruleType);
#if	!defined(CONFIG_ARCH_CORTINA_G3LITE)
		//this feature will cause CL_IF_ID_KEY(0.25 entry size) unused, so g3 lite not support this feature due to limite cls resource
		if((cfg->key_mask.src_port) && (PORT_TYPE(cfg->key.src_port)==CA_PORT_TYPE_L3))
		{
			cfg->action.options.sw_id[0] |= ruleType;
			cfg->action.options.sw_id[0] |= RXINFO_REF_ACL_RSN_CTRL_RSV;
			if((cfg->action.forward == CA_CLASSIFIER_FORWARD_PORT) && ((1<<(cfg->action.dest.port)) & RTK_FC_ALL_MAC_PURECPU_PORTMASK))
			{
				cfg->action.options.sw_id[0] |= RXINFO_REF_ACL_RSN_CTRL_TRAP;
				cfg->action.options.sw_id[0] |= RXINFO_REF_TRAP_RSN_ACL<<RXINFO_REF_TRAP_RSN_SHIFT_BIT;
			}
			cfg->action.options.masks.sw_id = TRUE;
		}
#endif
	}
	return;
}

void _rtk_fc_g3_caClsRulePortVlanTransform(ca_classifier_rule_t *cfg, ca_classifier_key_t *backup_key, ca_classifier_key_mask_t *backup_key_mask)
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
		ACL_RSV("transform port %d to fake vlan 0x%x %s inner vlan", port_idx, cfg->key.l2.vlan_otag.vlan_max.vid, cfg->key_mask.l2_mask.vlan_itag?"with":"without");
	}

	return;
}

void _rtk_fc_g3_caClsRulePortVlanRollback(ca_classifier_rule_t *cfg, ca_classifier_key_t *backup_key, ca_classifier_key_mask_t *backup_key_mask)
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

	return;
}


int _rtk_fc_g3_caClsRuleAALAdd(ca_classifier_rule_t *cfg, int ruleType, unsigned int aal_customize_reason)
{
	ca_status_t ret = CA_E_OK;
	char string_buf[16];
	ca_classifier_key_t backup_key;
	ca_classifier_key_mask_t backup_key_mask;


	_rtk_fc_g3_caClsAssignRuleRefIdx(cfg, ruleType, string_buf, sizeof(string_buf));
	
	if(fc_db.controlFuc.loopbackMode_is_enabled && (aal_customize_reason&CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT)==0x0)
		_rtk_fc_g3_caClsRulePortVlanTransform(cfg, &backup_key, &backup_key_mask);

	ret = ca_classifier_l3_cls_rule_add(G3_DEF_DEVID, cfg->priority, &(cfg->key), &(cfg->key_mask), &(cfg->action), &(cfg->index), aal_customize_reason);
	if(ret != CA_E_OK)
	{
		if(ret == CA_E_FULL) {
			ACL_RSV("add CA CLS (%s) failed due to CA ACL FULL", string_buf);
			WARNING("add CA CLS (%s) failed due to CA ACL FULL", string_buf);
		}else if(ret == CA_E_PARAM) {
			ACL_RSV("add CA CLS (%s) failed due to CA parsing parameter error", string_buf);
			WARNING("add CA CLS (%s) failed due to CA parsing parameter error", string_buf);
		} else if(ret == CA_E_NOT_SUPPORT) {
			ACL_RSV("add CA CLS (%s) failed due to CA NOT SUPPORT", string_buf);
			WARNING("add CA CLS (%s) failed due to CA NOT SUPPORT", string_buf);
		} else {
			ACL_RSV("add CA CLS (%s) failed!!!(ca_ret=%d)", string_buf, ret);
			WARNING("add CA CLS (%s) failed!!!(ca_ret=%d)", string_buf, ret);
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
	
	if(fc_db.controlFuc.loopbackMode_is_enabled && (aal_customize_reason&CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT)==0x0)
		_rtk_fc_g3_caClsRulePortVlanRollback(cfg, &backup_key, &backup_key_mask);


	fc_db.ca_cls_rule_record[cfg->index].valid = ENABLED;
	fc_db.ca_cls_rule_record[cfg->index].priority = cfg->priority;
	fc_db.ca_cls_rule_record[cfg->index].ruleType = ruleType;
	fc_db.ca_cls_rule_record[cfg->index].aal_customize = aal_customize_reason;
	fc_db.ca_cls_used_count++;

	return (RT_ERR_RG_OK);
}

int _rtk_fc_g3_caClsRuleDelete(rtk_fc_g3_caRulePriority_t priority)
{
	int i;

	for(i = 0; i < MAX_ACL_CA_CLS_RULE_SIZE; i++)
	{
		if((fc_db.ca_cls_rule_record[i].valid == ENABLED) && (fc_db.ca_cls_rule_record[i].priority == priority))
		{
			ASSERT_EQ(ca_classifier_rule_delete(G3_DEF_DEVID, i), CA_E_OK);
			bzero(&fc_db.ca_cls_rule_record[i],sizeof(rtk_fc_g3_cls_entry_t));
			fc_db.ca_cls_used_count--;
		}
	}
	return (RT_ERR_RG_OK);
}
#endif

void _rtk_fc_g3_caAalClsAssignRuleRefIdx(l3_cls_rule_t *cfg, int ruleType, char *debug_str, int debug_str_size)
{
	if(ruleType == FAIL){
		snprintf(debug_str, debug_str_size, "pre-add test");
	}else{
		if(ruleType >= RTK_FC_ACLANDCF_RESERVED_TAIL_END)
			snprintf(debug_str, debug_str_size, "SW_ACL[%d]", ruleType - RTK_FC_ACLANDCF_RESERVED_TAIL_END);
		else
			snprintf(debug_str, debug_str_size, "RSV_ACL[%d]", ruleType);


#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		if((ruleType == RTK_FC_ACLANDCF_RESERVED_WIFI_TXAMSDU_HW_LOOKUP) || 
			(ruleType == RTK_CLS_TYPE_HTTP_ACCELERATE_BY_PE) ||
			(ruleType == RTK_CLS_TYPE_SRV6_ACCELERATE_BY_PE) ||
			(ruleType == RTK_FC_ACLANDCF_RESERVED_IPSEC_ACCELERATE_BY_PE))
		{
			// no reference code
		}
		else if(((1<<(cfg->action.mcgid)) & RTK_FC_ALL_MAC_PURECPU_PORTMASK) && (cfg->action.keep_orig_pkt == TRUE) && (cfg->action.t2_ctrl == 0xf))
		{
			if(ruleType >= RTK_FC_ACLANDCF_RESERVED_TAIL_END){
				cfg->action.mdata_w_1 = ruleType - RTK_FC_ACLANDCF_RESERVED_TAIL_END;
				cfg->action.mdata_w_1 |= RXINFO_REF_ACL_RSN_CTRL_USER;
			}else{
				cfg->action.mdata_w_1 = ruleType;
				cfg->action.mdata_w_1 |= RXINFO_REF_ACL_RSN_CTRL_RSV;
			}
			cfg->action.mdata_w_1 |= RXINFO_REF_ACL_RSN_CTRL_TRAP;
			cfg->action.mdata_w_vld_1 = 0xf;
			cfg->action.pol_vld = TRUE;						//work around mdata[16:63] will be overrite by hash CRC16/32, sw debug only
			cfg->action.pol_sel = 2;
			cfg->action.pol_base = ruleType;
#if defined(CONFIG_FC_RTL8277C_SERIES)
			if((cfg->action.mcgid == RTK_FC_MAC_PORT_CPU1) && (cfg->action.mdata_w_vld_0 == 0)){
				cfg->action.mdata_w_vld_0 = 0x4;				// reason: mdata0[11:8], keep acl trap for mcgid=0x11
				cfg->action.mdata_w_0 = CPU_REASON_ACL << RXINFO_REF_HW_RSN_SHIFT_BIT;	// hw resson (02): acl trap. (cls hit && ldpid = 0x10 && keep_orig_pkt = 1)
			}else if((cfg->action.mcgid == RTK_FC_MAC_PORT_CPU) && (cfg->action.mdata_w_0 == CPU_REASON_ACL << RXINFO_REF_HW_RSN_SHIFT_BIT)){
				cfg->action.mdata_w_vld_0 = 0;					//remove reason due to hw offload this
			}
			if(fc_db.controlFuc.acl_multiple_hit_cfg == 0){	//work around acl trap should keep crc, so not bypass hash
				//cfg->action.ip_ttl_vld = TRUE;
				//cfg->action.ip_ttl_cmd = 2;	//set ip_ttl = 0
				//cfg->action.ip_ttl = 0;
				//cfg->action.no_drop_vld = TRUE;	//mc profile will use hash policer drop
				//cfg->action.no_drop = TRUE;
				cfg->action.mdata_w_vld_0 |= 0x4; // reason: mdata0[11:8]
				cfg->action.mdata_w_0 |= CPU_REASON_FLOWMISS << RXINFO_REF_HW_RSN_SHIFT_BIT;// resson3: flow miss
				cfg->action.mdata_w_vld_0 |= 0x8;
				if((cfg->action.mdata_w_0 >> RXINFO_REF_TRAP_RSN_SHIFT_BIT) != RXINFO_REF_TRAP_RSN_ACL_TO_PS){	//keep RXINFO_REF_TRAP_RSN_ACL_TO_PS
					cfg->action.mdata_w_0 |= RXINFO_REF_TRAP_RSN_ACL << RXINFO_REF_TRAP_RSN_SHIFT_BIT;	//sw reason for MDATAL_REASON_SW
					cfg->action.t2_ctrl = RTK_ASIC_FLOW_PROFILE_FLOW_5TUPLE_TCP_FLAG0;	//make sure flow miss for 5 tuple check
				}
			}
#elif defined(CONFIG_FC_RTL9607F_SERIES)
			if(MDATAL_REASON(cfg->action.mdata_w_0) != CPU_REASON_ACL_TRAP_PS){
				cfg->action.t2_ctrl_vld = 0;
				cfg->action.mdata_w_vld_0 = 0x4;				// hw reason: mdata0[11:8]
				cfg->action.mdata_w_0 = CPU_REASON_ACL << RXINFO_REF_HW_RSN_SHIFT_BIT;	// hw resson (14): acl trap with hash crc
			}
#endif
			if(ruleType == RTK_FC_ACLANDCF_RESERVED_ALL_TRAP){	//force bypass hash case
				cfg->action.t2_ctrl_vld = 1;
				cfg->action.t2_ctrl = T2_CTRL_BYPASS;
			}
		}
#elif !defined(CONFIG_ARCH_CORTINA_G3LITE)
		//this will use fib_mode_2, only fib mode 0/1 can use CL_IF_ID_KEY(0.25 entry size), so remove this debug information for G3 LITE.
		//fib mode 0/1 only support mdata CLS_FIB_MSK_MDATA_W0/CLS_FIB_MSK_MDATA_W1
		//reserve mdata_w_2 to ACL in 8077A
		if(ruleType != RTK_CA_CLS_TYPE_GENERIC_INTF_HASH_PROFILE_DECISION)	//keep flow miss reason
		{
			cfg->action.mdata_w_2 = ruleType;
			cfg->action.mdata_w_2 |= RXINFO_REF_ACL_RSN_CTRL_RSV;
			if((cfg->action.mc == 0) && ((1<<(cfg->action.mcgid)) & RTK_FC_ALL_MAC_PURECPU_PORTMASK))
			{
				cfg->action.mdata_w_2 |= RXINFO_REF_ACL_RSN_CTRL_TRAP;
				if((cfg->action.mdata_w_2 >> RXINFO_REF_TRAP_RSN_SHIFT_BIT) != RXINFO_REF_TRAP_RSN_ACL_TO_PS)	//keep RXINFO_REF_TRAP_RSN_ACL_TO_PS
					cfg->action.mdata_w_2 |= RXINFO_REF_TRAP_RSN_ACL<<RXINFO_REF_TRAP_RSN_SHIFT_BIT;
			}
			cfg->action.mdata_w_vld_2 = 0xf;
		}
#endif
	}

	return;
}

void _rtk_fc_g3_caAalClsAssignHashProfileAction(l3_cls_rule_t *cfg, uint8 profile_id, uint8 care_lspid, int port)
{
	cfg->action.dpid_vld = 1;
	cfg->action.dpid_pri = 1;
	cfg->action.permit = 1;
	cfg->action.mc = 0; //1:mcgid=mcgid;0:mcgid=ldpid
#if defined(RTK_FC_MAINCPU_ONE_CPU_PORT)
	cfg->action.mcgid = RTK_FC_MAINCPU_ONE_CPU_PORT;
#else
	if(profile_id==CA_L3_AAL_CLS_PROFILE_WAN)
		cfg->action.mcgid = RTK_FC_MAC_PORT_CPU1;
	else
		cfg->action.mcgid = RTK_FC_MAC_PORT_CPU;
#endif

	cfg->action.t2_ctrl_vld = 1;
	//cfg->action.t2_ctrl 		//update by caller
	
#if !(defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES))
	cfg->action.t3_ctrl_vld = 1;
	cfg->action.t3_ctrl = 0xf;
	cfg->action.t4_ctrl_vld = 1;
	if(care_lspid && (port<RTK_FC_MAC_PORT_CPU))
		cfg->action.t4_ctrl = port;
	else
		cfg->action.t4_ctrl = 0xf;
	cfg->action.t5_ctrl_vld = 1;
	cfg->action.t5_ctrl = 0xf;
	cfg->action.stage3_ctrl_vld = 1;
	cfg->action.stage3_ctrl = 1;
#endif
	cfg->action.stage2_ctrl_vld = 1; // Merge type-0 and type-1 result of Hash Engine
	cfg->action.stage2_ctrl = 0;

	if(cfg->action.mrr_en){	//enable CLS mirror for cpu port mirror (CLS enable mirror, flow hit disable)
		cfg->action.mrr_vld = 1;
	}

	return;
}

void _rtk_fc_g3_caAalClsAssignTrapAction(l3_cls_rule_t *cfg, uint8 profile_idx)
{
	//ref: cls_fib_entry_update

	cfg->action.keep_orig_pkt = 1;
	cfg->action.keep_orig_pkt_vld = 1;
	cfg->action.mc = 0;	//1:mcgid=mcgid;0:mcgid=ldpid
#if defined(RTK_FC_MAINCPU_ONE_CPU_PORT)
	cfg->action.mcgid = RTK_FC_MAINCPU_ONE_CPU_PORT;
#else
	if(profile_idx == CA_L3_AAL_CLS_PROFILE_WAN)
		cfg->action.mcgid = RTK_FC_MAC_PORT_CPU1;
	else
		cfg->action.mcgid = RTK_FC_MAC_PORT_CPU;
#endif
	cfg->action.dpid_pri = 1;
	cfg->action.permit = 1;
	cfg->action.dpid_vld = 1;
	cfg->action.t2_ctrl_vld = 1;
	cfg->action.t2_ctrl = T2_CTRL_BYPASS;
#if !(defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES))
	cfg->action.t3_ctrl_vld = 1;
	cfg->action.t3_ctrl = T3_CTRL_BYPASS;
	cfg->action.t4_ctrl_vld = 1;
	cfg->action.t4_ctrl = T4_CTRL_BYPASS;
	cfg->action.t5_ctrl_vld = 1;
	cfg->action.t5_ctrl = T5_CTRL_BYPASS;
	if (!(cfg->action.mcgid >= AAL_LPORT_CPU_0 && cfg->action.mcgid <= AAL_LPORT_CPU_7)) {
		cfg->action.deepq = 1;
	}
#if defined(CONFIG_LUNA_G3_SERIES) && !(defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES))
	if (cfg->action.mcgid >= AAL_LPORT_CPU_0 && cfg->action.mcgid <= AAL_LPORT_CPU_1) {
		cfg->action.mdata_w_1 = 0;
		cfg->action.mdata_w_vld_1 = 0xf;
	}
#endif
#endif

	return;
}

void _rtk_fc_g3_caAalClsRulePortVlanTransform(l3_cls_rule_t *cfg, cls_key_t *backup_key)
{
	//L3 CLS for port 0/1/2/3 should use fake vlan as pattern due to they will be loopback from port 6 with fake vlan.
	int port_idx;
	port_idx = PORT_ID(cfg->key.o_lspid);

	if((cfg->key.msk_o_lspid == CA_L3_AAL_KEY_MSK_ENABLE) && ((G3_LOOPBACK_P_START <= port_idx) && (port_idx <= G3_LOOPBACK_P_END)))
	{
		//back up original key and key_mask
		bzero(backup_key,sizeof(cls_key_t));
		memcpy(backup_key,&cfg->key,sizeof(cls_key_t));

		//if original use vlan pattern, move it to inner vlan
		if(cfg->key.msk_top_vid == CA_L3_AAL_KEY_MSK_ENABLE)
		{
			cfg->key.msk_inner_vid = CA_L3_AAL_KEY_MSK_ENABLE;
			cfg->key.inner_vid = cfg->key.top_vid;
			cfg->key.msk_top_vid = CA_L3_AAL_KEY_MSK_DISABLE;
			cfg->key.top_vid = 0;
		}

		//config outer fake vlan as key to regonize from which port
		cfg->key.msk_top_vid = CA_L3_AAL_KEY_MSK_ENABLE;
		cfg->key.top_vid = G3_LOOPBACK_UPSTREAM_VID(PORT_ID(port_idx));
		cfg->key.o_lspid = G3_LOOPBACK_P_NEWSPA;

		ACL_RSV("transform port %d to fake vlan 0x%x %s inner vlan", port_idx, cfg->key.top_vid, (cfg->key.msk_inner_vid == CA_L3_AAL_KEY_MSK_ENABLE)?"with":"without");
	}

	return;
}


void _rtk_fc_g3_caAalClsRulePortVlanRollback(l3_cls_rule_t *cfg, cls_key_t *backup_key)
{
	//Rollback original L3 CLS rule.
	int port_idx;
	port_idx = PORT_ID(cfg->key.o_lspid);

	if((cfg->key.msk_o_lspid == CA_L3_AAL_KEY_MSK_ENABLE) && (port_idx==G3_LOOPBACK_P_NEWSPA))
	{
		ACL_RSV("rollback from port 0x%x to port 0x%x", backup_key->o_lspid, cfg->key.o_lspid);
		memcpy(&cfg->key,backup_key,sizeof(cls_key_t));
	}

	return;
}


int _rtk_fc_g3_caAalClsL2RuleAdd(aal_l2_cls_rule_t *l2_entry, uint32 *rslt_idx, int ruleType)
{
	ca_status_t ret = CA_E_OK;
	int index = 0;

	ret = aal_l2_cls_add(G3_DEF_DEVID, l2_entry, rslt_idx);
	if(ret != CA_E_OK)
		WARNING("add L2 CLS failed!!!(ret=%d)", ret);

	for(index = 0; index < MAX_ACL_CA_AAL_CLS_RULE_SIZE; index++)
		if(!fc_db.ca_aal_cls_rule_record[index].valid)
			break;
	if(index == MAX_ACL_CA_AAL_CLS_RULE_SIZE)
	{
		WARNING("CA AAL entry full, please enlarge sw rule size %d.", MAX_ACL_CA_AAL_CLS_RULE_SIZE);
		return (RT_ERR_RG_FAILED);
	}

	fc_db.ca_aal_cls_rule_record[index].valid = ENABLED;
	fc_db.ca_aal_cls_rule_record[index].ruleType = ruleType;
	fc_db.ca_aal_cls_rule_record[index].priority = l2_entry->priority;
	fc_db.ca_aal_cls_rule_record[index].rslt_idx = *rslt_idx;
	fc_db.ca_aal_cls_rule_record[index].is_l2_rule = TRUE;

	return (RT_ERR_RG_OK);
}

int _rtk_fc_g3_caAalClsRuleAdd(uint32 profile_idx, l3_cls_rule_t *rule, uint32 *rslt_idx, int ruleType)
{
	ca_status_t ret = AAL_E_OK;
	char string_buf[16];
	int index = 0;
	cls_key_t backup_key;


	_rtk_fc_g3_caAalClsAssignRuleRefIdx(rule, ruleType, string_buf, sizeof(string_buf));

	if(fc_db.controlFuc.loopbackMode_is_enabled)
		_rtk_fc_g3_caAalClsRulePortVlanTransform(rule, &backup_key);

	ret = aal_l3_cls_add(G3_DEF_DEVID, profile_idx, rule, rslt_idx);
	if(ret != AAL_E_OK)
	{
		if(ret == AAL_E_TBLFULL) {
			ACL_RSV("add CA AAL CLS (RSV_ACL[%s]) failed due to CA ACL FULL", string_buf);
			WARNING("add CA AAL CLS (RSV_ACL[%s]) failed due to CA ACL FULL", string_buf);
		}else if(ret == AAL_E_INVALID) {
			ACL_RSV("add CA AAL CLS (RSV_ACL[%s]) failed due to CA parsing parameter error", string_buf);
			WARNING("add CA AAL CLS (RSV_ACL[%s]) failed due to CA parsing parameter error", string_buf);
		}else {
			ACL_RSV("add CA AAL CLS (RSV_ACL[%s]) failed!!!(ca_ret=%d)", string_buf, ret);
			WARNING("add CA AAL CLS (RSV_ACL[%s]) failed!!!(ca_ret=%d)", string_buf, ret);
		}
		return ret;
	}

	for(index = 0; index < MAX_ACL_CA_AAL_CLS_RULE_SIZE; index++)
		if(!fc_db.ca_aal_cls_rule_record[index].valid)
			break;
	if(index == MAX_ACL_CA_AAL_CLS_RULE_SIZE)
	{
		WARNING("CA AAL entry full, please enlarge sw rule size %d.", MAX_ACL_CA_AAL_CLS_RULE_SIZE);
		return (RT_ERR_RG_FAILED);
	}

	if(fc_db.controlFuc.loopbackMode_is_enabled)
		_rtk_fc_g3_caAalClsRulePortVlanRollback(rule, &backup_key);


	fc_db.ca_aal_cls_rule_record[index].valid = ENABLED;
	fc_db.ca_aal_cls_rule_record[index].ruleType = ruleType;
	fc_db.ca_aal_cls_rule_record[index].priority = rule->pri;
	fc_db.ca_aal_cls_rule_record[index].rslt_idx = *rslt_idx;
	fc_db.ca_aal_cls_rule_record[index].profile_id = profile_idx;
	if(rule->key.pktlen_rng_match_mask == CA_L3_AAL_KEY_MSK_ENABLE){
		fc_db.ca_aal_cls_rule_record[index].cam_info.cam_type |= (0x1 << RTK_ACL_CLS_RUL_INFO_CAM_PKT_LEN);
		fc_db.ca_aal_cls_rule_record[index].cam_info.pkt_len_vec = rule->key.pktlen_rng_match_vec;
		//ACL_RSV("index %d, profile %d, ruleType %d, pkt cam 0x%x", index, profile_idx, ruleType, fc_db.ca_aal_cls_rule_record[index].cam_info.pkt_len_vec);
	}
	if(rule->key.spcl_pkt_hdr_mtch_msk == CA_L3_AAL_KEY_MSK_ENABLE){
		fc_db.ca_aal_cls_rule_record[index].cam_info.cam_type |= (0x1 << RTK_ACL_CLS_RUL_INFO_CAM_SPCL_HDR);
		fc_db.ca_aal_cls_rule_record[index].cam_info.spec_hdr_vec = rule->key.spcl_pkt_hdr_mtch;
		//ACL_RSV("index %d, profile %d, ruleType %d, speck hdr cam 0x%x", index, profile_idx, ruleType, fc_db.ca_aal_cls_rule_record[index].cam_info.spec_hdr_vec);
	}
	if(rule->key.msk_ethertype_enc == CA_L3_AAL_KEY_MSK_ENABLE){
		fc_db.ca_aal_cls_rule_record[index].cam_info.cam_type |= (0x1 << RTK_ACL_CLS_RUL_INFO_CAM_ETHERTYPE);
		fc_db.ca_aal_cls_rule_record[index].cam_info.ethertype_idx = rule->key.ethertype_enc-1;
		//ACL_RSV("index %d, profile %d, ruleType %d, ethertype cam %d", index, profile_idx, ruleType, fc_db.ca_aal_cls_rule_record[index].cam_info.ethertype_idx);
	}
	if(rule->key.msk_ppp_protocol == CA_L3_AAL_KEY_MSK_ENABLE){
		fc_db.ca_aal_cls_rule_record[index].cam_info.cam_type |= (0x1 << RTK_ACL_CLS_RUL_INFO_CAM_PPP_PROTO);
		fc_db.ca_aal_cls_rule_record[index].cam_info.ppp_proto_idx = rule->key.ppp_protocol_enc-1;
		//ACL_RSV("index %d, profile %d, ruleType %d, ppp proto cam %d", index, profile_idx, ruleType, fc_db.ca_aal_cls_rule_record[index].cam_info.ppp_proto_idx);
	}
	if(rule->key.msk_l4_port == 3){
		fc_db.ca_aal_cls_rule_record[index].cam_info.cam_type |= (0x1 << RTK_ACL_CLS_RUL_INFO_CAM_L4_SPORT);
		fc_db.ca_aal_cls_rule_record[index].cam_info.cam_type |= (0x1 << RTK_ACL_CLS_RUL_INFO_CAM_L4_DPORT);
		fc_db.ca_aal_cls_rule_record[index].cam_info.l4_sport_vec = rule->key.l4_sp;
		fc_db.ca_aal_cls_rule_record[index].cam_info.l4_dport_vec = rule->key.l4_dp;
		//ACL_RSV("index %d, profile %d, ruleType %d, l4 sport/dport cam 0x%x/0x%x", index, profile_idx, ruleType, fc_db.ca_aal_cls_rule_record[index].cam_info.l4_sport_vec, fc_db.ca_aal_cls_rule_record[index].cam_info.l4_dport_vec);
	}

	return (RT_ERR_RG_OK);
}

int _rtk_fc_g3_caAalClsRuleDeleteCam(int index)
{
	uint8 cam_idx;

	if(fc_db.ca_aal_cls_rule_record[index].cam_info.cam_type == 0)
		return (RT_ERR_RG_OK);

	if(fc_db.ca_aal_cls_rule_record[index].cam_info.cam_type & (0x1 << RTK_ACL_CLS_RUL_INFO_CAM_PKT_LEN)){
		//ACL_RSV("index %d, profile_id %d, rslt type %d, pkt cam 0x%x", index, fc_db.ca_aal_cls_rule_record[index].profile_id, fc_db.ca_aal_cls_rule_record[index].ruleType, fc_db.ca_aal_cls_rule_record[index].cam_info.pkt_len_vec);
		cam_idx = 0;
		while (fc_db.ca_aal_cls_rule_record[index].cam_info.pkt_len_vec) {
			if (fc_db.ca_aal_cls_rule_record[index].cam_info.pkt_len_vec & 0x1)
				ASSERT_EQ(aal_entry_del(AAL_TABLE_L3_CAM_PKT_LEN, cam_idx), AAL_E_OK);
			fc_db.ca_aal_cls_rule_record[index].cam_info.pkt_len_vec >>= 1;
			cam_idx++;
		}
	}
	if(fc_db.ca_aal_cls_rule_record[index].cam_info.cam_type & (0x1 << RTK_ACL_CLS_RUL_INFO_CAM_SPCL_HDR)){
		//ACL_RSV("index %d, profile_id %d, rslt type %d, speck hdr cam 0x%x", index, fc_db.ca_aal_cls_rule_record[index].profile_id, fc_db.ca_aal_cls_rule_record[index].ruleType, fc_db.ca_aal_cls_rule_record[index].cam_info.spec_hdr_vec);
		cam_idx = 0;
		while (fc_db.ca_aal_cls_rule_record[index].cam_info.spec_hdr_vec) {
			if (fc_db.ca_aal_cls_rule_record[index].cam_info.spec_hdr_vec & 0x1)
				ASSERT_EQ(aal_entry_del(AAL_TABLE_L3_CAM_SPCL_HDR, cam_idx), AAL_E_OK);
			fc_db.ca_aal_cls_rule_record[index].cam_info.spec_hdr_vec >>= 1;
			cam_idx++;
		}
	}
	if(fc_db.ca_aal_cls_rule_record[index].cam_info.cam_type & (0x1 << RTK_ACL_CLS_RUL_INFO_CAM_ETHERTYPE)){
		//ACL_RSV("index %d, profile_id %d, rslt type %d, ethertype cam %d", index, fc_db.ca_aal_cls_rule_record[index].profile_id, fc_db.ca_aal_cls_rule_record[index].ruleType, fc_db.ca_aal_cls_rule_record[index].cam_info.ethertype_idx);
		//ASSERT_EQ(aal_entry_del(AAL_TABLE_L3_CAM_ETHERTYPE, fc_db.ca_aal_cls_rule_record[index].cam_info.ethertype_idx), AAL_E_OK);
		ASSERT_EQ(_rtk_fc_ethTypeEntry_acl_deref(fc_db.ca_aal_cls_rule_record[index].cam_info.ethertype_idx), RTK_FC_RET_OK);
	}
	if(fc_db.ca_aal_cls_rule_record[index].cam_info.cam_type & (0x1 << RTK_ACL_CLS_RUL_INFO_CAM_PPP_PROTO)){
		//ACL_RSV("index %d, profile_id %d, rslt type %d, ppp proto cam %d", index, fc_db.ca_aal_cls_rule_record[index].profile_id, fc_db.ca_aal_cls_rule_record[index].ruleType, fc_db.ca_aal_cls_rule_record[index].cam_info.ppp_proto_idx);
		ASSERT_EQ(aal_entry_del(AAL_TABLE_L3_CAM_PPP_PROT, fc_db.ca_aal_cls_rule_record[index].cam_info.ppp_proto_idx), AAL_E_OK);
	}
	if(fc_db.ca_aal_cls_rule_record[index].cam_info.cam_type & (0x1 << RTK_ACL_CLS_RUL_INFO_CAM_L4_SPORT)){
		//ACL_RSV("index %d, profile_id %d, rslt type %d, l4 sport cam 0x%x", index, fc_db.ca_aal_cls_rule_record[index].profile_id, fc_db.ca_aal_cls_rule_record[index].ruleType, fc_db.ca_aal_cls_rule_record[index].cam_info.l4_sport_vec);
		cam_idx = 0;
		while (fc_db.ca_aal_cls_rule_record[index].cam_info.l4_sport_vec) {
			if (fc_db.ca_aal_cls_rule_record[index].cam_info.l4_sport_vec & 0x1)
				ASSERT_EQ(aal_entry_del(AAL_TABLE_L3_CAM_SPORT, cam_idx), AAL_E_OK);
			fc_db.ca_aal_cls_rule_record[index].cam_info.l4_sport_vec >>= 1;
			cam_idx++;
		}
	}
	if(fc_db.ca_aal_cls_rule_record[index].cam_info.cam_type & (0x1 << RTK_ACL_CLS_RUL_INFO_CAM_L4_DPORT)){
		//ACL_RSV("index %d, profile_id %d, rslt type %d, dport cam 0x%x", index, fc_db.ca_aal_cls_rule_record[index].profile_id, fc_db.ca_aal_cls_rule_record[index].ruleType, fc_db.ca_aal_cls_rule_record[index].cam_info.l4_dport_vec);
		cam_idx = 0;
		while (fc_db.ca_aal_cls_rule_record[index].cam_info.l4_dport_vec) {
			if (fc_db.ca_aal_cls_rule_record[index].cam_info.l4_dport_vec & 0x1)
				ASSERT_EQ(aal_entry_del(AAL_TABLE_L3_CAM_DPORT, cam_idx), AAL_E_OK);
			fc_db.ca_aal_cls_rule_record[index].cam_info.l4_dport_vec >>= 1;
			cam_idx++;
		}
	}

	return (RT_ERR_RG_OK);
}

int _rtk_fc_g3_caAalClsRuleDelete(rtk_rg_aclAndCf_reserved_type_t rsvType_start, rtk_rg_aclAndCf_reserved_type_t rsvType_end)
{
	int i;

	for(i = 0; i < MAX_ACL_CA_AAL_CLS_RULE_SIZE; i++)
	{
		if(fc_db.ca_aal_cls_rule_record[i].valid != ENABLED)
			continue;
		if((fc_db.ca_aal_cls_rule_record[i].ruleType < rsvType_start) || (fc_db.ca_aal_cls_rule_record[i].ruleType > rsvType_end))
			continue;

		if(fc_db.ca_aal_cls_rule_record[i].is_l2_rule == TRUE){
			ASSERT_EQ(aal_l2_cls_del(G3_DEF_DEVID, fc_db.ca_aal_cls_rule_record[i].rslt_idx), AAL_E_OK);
			bzero(&fc_db.ca_aal_cls_rule_record[i],sizeof(rtk_fc_ca_aal_cls_entry_t));
			continue;
		}
		ASSERT_EQ(_rtk_fc_g3_caAalClsRuleDeleteCam(i), RT_ERR_RG_OK);
		ASSERT_EQ(aal_l3_cls_delete(G3_DEF_DEVID, fc_db.ca_aal_cls_rule_record[i].profile_id, fc_db.ca_aal_cls_rule_record[i].rslt_idx), AAL_E_OK);
		bzero(&fc_db.ca_aal_cls_rule_record[i],sizeof(rtk_fc_ca_aal_cls_entry_t));
	}

	return (RT_ERR_RG_OK);
}

int _rtk_fc_g3_caAalClsRuleDeleteRsltIdx(int ruleType, uint32 rslt_idx)
{
	int i;

	for(i = 0; i < MAX_ACL_CA_AAL_CLS_RULE_SIZE; i++)
	{
		if(fc_db.ca_aal_cls_rule_record[i].valid != ENABLED)
			continue;
		if(fc_db.ca_aal_cls_rule_record[i].ruleType != ruleType)
			continue;
		if(fc_db.ca_aal_cls_rule_record[i].rslt_idx != rslt_idx)
			continue;

		if(fc_db.ca_aal_cls_rule_record[i].is_l2_rule == TRUE){
			ASSERT_EQ(aal_l2_cls_del(G3_DEF_DEVID, fc_db.ca_aal_cls_rule_record[i].rslt_idx), AAL_E_OK);
			bzero(&fc_db.ca_aal_cls_rule_record[i],sizeof(rtk_fc_ca_aal_cls_entry_t));
			break;
		}

		ASSERT_EQ(_rtk_fc_g3_caAalClsRuleDeleteCam(i), RT_ERR_RG_OK);
		ASSERT_EQ(aal_l3_cls_delete(G3_DEF_DEVID, fc_db.ca_aal_cls_rule_record[i].profile_id, fc_db.ca_aal_cls_rule_record[i].rslt_idx), AAL_E_OK);
		bzero(&fc_db.ca_aal_cls_rule_record[i],sizeof(rtk_fc_g3_cls_entry_t));
		break;
	}

	if(ruleType == RTK_CA_CLS_TYPE_GENERIC_INTF_MC_PROFILE_WITH_DMAC){
		for(i = 0; i < RTK_FC_TABLESIZE_INTF_MC_ACL; i++){
			if(fc_db.aclAndCfReservedRule.generic_intf_mc_dmac[i].acl_info == rslt_idx){
				bzero(&fc_db.aclAndCfReservedRule.generic_intf_mc_dmac[i],sizeof(rtk_fc_aclAndCf_reserved_generic_intf_mc_dmac_t));
				break;
			}
		}
	}

	return (RT_ERR_RG_OK);
}

int _rtk_fc_g3_caAalClsRuleDeleteReserveType(int ruleType)
{
	int i;

	for(i = 0; i < MAX_ACL_CA_AAL_CLS_RULE_SIZE; i++)
	{
		if(fc_db.ca_aal_cls_rule_record[i].valid != ENABLED)
			continue;
		if(fc_db.ca_aal_cls_rule_record[i].ruleType != ruleType)
			continue;
		if(fc_db.ca_aal_cls_rule_record[i].is_l2_rule == TRUE)
			ASSERT_EQ(aal_l2_cls_del(G3_DEF_DEVID, fc_db.ca_aal_cls_rule_record[i].rslt_idx), AAL_E_OK);
		else{
			ASSERT_EQ(_rtk_fc_g3_caAalClsRuleDeleteCam(i), RT_ERR_RG_OK);
			ASSERT_EQ(aal_l3_cls_delete(G3_DEF_DEVID, fc_db.ca_aal_cls_rule_record[i].profile_id, fc_db.ca_aal_cls_rule_record[i].rslt_idx), AAL_E_OK);
		}
		bzero(&fc_db.ca_aal_cls_rule_record[i],sizeof(rtk_fc_ca_aal_cls_entry_t));
	}

	return (RT_ERR_RG_OK);
}


#if defined(CONFIG_FC_G3_G3LITE_SERIES)
int _rtk_fc_g3_caClsRuleAdd(ca_classifier_rule_t *cfg, int ruleType)
{
	ca_status_t ret = CA_E_OK;
	char string_buf[16];

	snprintf(string_buf, sizeof(string_buf), "RSV_ACL[%d]", ruleType);

	ret = ca_classifier_rule_add(G3_DEF_DEVID, cfg->priority, &(cfg->key), &(cfg->key_mask), &(cfg->action), &(cfg->index));
	if(ret != CA_E_OK)
	{
		if(ret == CA_E_FULL) {
			ACL_RSV("add CA CLS (%s) failed due to CA ACL FULL", string_buf);
			WARNING("add CA CLS (%s) failed due to CA ACL FULL", string_buf);
		}else if(ret == CA_E_PARAM) {
			ACL_RSV("add CA CLS (%s) failed due to CA parsing parameter error", string_buf);
			WARNING("add CA CLS (%s) failed due to CA parsing parameter error", string_buf);
		} else if(ret == CA_E_NOT_SUPPORT) {
			ACL_RSV("add CA CLS (%s) failed due to CA NOT SUPPORT", string_buf);
			WARNING("add CA CLS (%s) failed due to CA NOT SUPPORT", string_buf);
		} else {
			ACL_RSV("add CA CLS (%s) failed!!!(ca_ret=%d)", string_buf, ret);
			WARNING("add CA CLS (%s) failed!!!(ca_ret=%d)", string_buf, ret);
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

	fc_db.ca_cls_rule_record[cfg->index].valid = ENABLED;
	fc_db.ca_cls_rule_record[cfg->index].priority = cfg->priority;
	fc_db.ca_cls_rule_record[cfg->index].ruleType = ruleType;
	fc_db.ca_cls_used_count++;

	return (RT_ERR_RG_OK);
}

static int _rtk_fc_aclHWEntry_hash_remapping(int port_idx, rt_acl_filterAndQos_t *acl_filter, rtk_fc_aclFilterEntry_t* aclSWEntry)
{
	int assign_port_idx, assign_hash_type, ret = RT_ERR_RG_OK;
	ca_flow_t flow_config;

	if(ACL_CHECK_HASH_NEED(acl_filter)){
		if(acl_filter->ingress_tcp_flags == 0x2 /*syn only*/)
			assign_hash_type = RTK_FC_ACL_HASH_TCP_TYPE_SYN_ONLY;
		else
			assign_hash_type = RTK_FC_ACL_HASH_TCP_TYPE_SYN_ACK;

		if(fc_db.controlFuc.loopbackMode_is_enabled){
			if((0x1<<port_idx)&G3_LOOPBACK_P_MASK)
				assign_port_idx = G3_LOOPBACK_P_NEWSPA;
			else
				assign_port_idx = port_idx;
		}else
			assign_port_idx = port_idx;

		if(fc_db.acl_remap_hash_idx[assign_hash_type][assign_port_idx]){
			aclSWEntry->hw_hash_index = 1;
			ACL_CTRL("skip add hash entry for port[%d to %d] due to hash[%d] already exist.", port_idx, assign_port_idx, fc_db.acl_remap_hash_idx[assign_hash_type][assign_port_idx]-1);
			return ret;
		}

		memset(&flow_config, 0, sizeof(flow_config));
		flow_config.actions.forward = CA_CLASSIFIER_FORWARD_PORT;
		flow_config.key_type = RG_CA_FLOW_CLS_DOS_POL;

		flow_config.key.l2_key.o_lspid = assign_port_idx;
		flow_config.key.l3_key.ip_protocol = 0x6;
		flow_config.key.l4_key.tcp_flags= acl_filter->ingress_tcp_flags;	//syn only 0x2 or syn ack 0x12

		flow_config.actions.dest.port = RTK_FC_MAC_PORT_CPU;
		if(acl_filter->action_fields & RT_ACL_ACTION_METER_GROUP_SHARE_METER_BIT){
			flow_config.actions.options.masks.flow_id = TRUE;
			flow_config.actions.options.flow_id = aclSWEntry->hw_share_meter_index;
		}
		if(acl_filter->action_fields & RT_ACL_ACTION_PRIORITY_GROUP_TRAP_PRIORITY_BIT){	//attention: non-8277A will cause FC skip learning
			flow_config.actions.options.masks.cos = TRUE;
			flow_config.actions.options.cos = acl_filter->action_priority_group_trap_priority;
		}
		if(acl_filter->action_fields & RT_ACL_ACTION_PRIORITY_GROUP_ACL_PRIORITY_BIT){	//attention: 9607C flow miss priority > acl priority
			flow_config.actions.options.masks.cos = TRUE;
			flow_config.actions.options.cos = acl_filter->action_priority_group_acl_priority;

			if(flow_config.actions.options.cos == 7)
				flow_config.actions.dest.no_drop = 1;
		}

		ret = ca_flow_add(G3_DEF_DEVID, &flow_config);
		if(ret != CA_E_OK){
			ACL_CTRL("add hash entry for %s packet failed!!!(ca_ret=%d)", (assign_hash_type==RTK_FC_ACL_HASH_TCP_TYPE_SYN_ONLY)?"syn only":"syn ack", ret);
			WARNING("add hash entry for %s packet failed!!!(ca_ret=%d)", (assign_hash_type==RTK_FC_ACL_HASH_TCP_TYPE_SYN_ONLY)?"syn only":"syn ack", ret);
			return ret;
		}else{
			aclSWEntry->hw_hash_index = flow_config.index + 1;
			fc_db.acl_remap_hash_idx[assign_hash_type][assign_port_idx] = aclSWEntry->hw_hash_index;
		}
	}

	return ret;
}
#endif

#if defined(CONFIG_FC_CA8277B_SERIES) || defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
static int _rtk_fc_aclHWEntry_meter_remapping(void)
{
	int aclIdx, loopIdx, mapIdx, ret;
	rt_acl_filterAndQos_t *acl_filter;
	rt_rate_meter_mapping_t meterMapping;
	rtk_fc_hw_pol_id_remap_t localRempMeter[RTK_FC_TABLESIZE_POLREMAP];

	if(!fc_db.controlFuc.rt_api_is_enabled)
		return (RT_ERR_RG_OK);

	memset(localRempMeter, 0, sizeof(localRempMeter));

	for(aclIdx=0, mapIdx=0; aclIdx<MAX_ACL_SW_ENTRY_SIZE; aclIdx++){
		if(fc_db.acl_SWindex_sorting_by_weight[aclIdx]==-1)
			break;

		acl_filter = &fc_db.acl_filter_temp[(fc_db.acl_SWindex_sorting_by_weight[aclIdx])];
		//ACL rule with meter action from lan/wan port, due to pol_id should reserve to wlan id, so this should remap to pol2 in 77B/pol_3 in 77C
		if((acl_filter->action_fields & RT_ACL_ACTION_METER_GROUP_SHARE_METER_BIT) == 0x0)
			continue;

		ret = RTK_FC_HELPER_RT_RATE_SHAREMETER_MAPPING_HW_GET(acl_filter->action_meter_group_share_meter_index, &meterMapping);
		if((ret != RT_ERR_RG_OK) || (meterMapping.type != RT_METER_TYPE_ACL)){
			ACL_CTRL("fail to update meter remapping, ret=%d, rt meter idx=%d, meter type=%d", ret, acl_filter->action_meter_group_share_meter_index, meterMapping.type);
			continue;
		}

		//reuse same entry with same hw meter idx
		for(loopIdx=0; loopIdx<mapIdx; loopIdx++){
			if(!localRempMeter[loopIdx].valid)
				continue;
			if(localRempMeter[loopIdx].pol_id == meterMapping.hw_index)
				break;
		}
		if(loopIdx != mapIdx)
			continue;

		if(mapIdx==RTK_FC_TABLESIZE_POLREMAP){
			ACL_CTRL("fail to update meter remapping due to limited resource %d", RTK_FC_TABLESIZE_POLREMAP);
			break;
		}
		localRempMeter[mapIdx].valid = TRUE;
		localRempMeter[mapIdx].pol_id = meterMapping.hw_index;
		mapIdx++;
	}

	if(memcmp(fc_db.hwPolRemap, localRempMeter, sizeof(fc_db.hwPolRemap))){
		ACL_CTRL("update hw pol1 remapping table");
		memcpy(fc_db.hwPolRemap, localRempMeter, sizeof(fc_db.hwPolRemap));
		ret = _rtk_fc_g3PolRemap_sync(-1);
		if(ret != RT_ERR_RG_OK)
			ACL_CTRL("fail to update meter remapping return value 0x%x", ret);
	}

	return (RT_ERR_RG_OK);
}
#endif

#if defined(CONFIG_FC_CA8277B_SERIES)
static int _rtk_fc_aclHWEntry_vlan_readd(uint8 need_vlan_rule, ca_classifier_rule_t *ca_cls_rule_vlan, int port, rt_acl_filterAndQos_t *acl_filter, int index, unsigned int aal_customize)
{
	//check C only: C will be outter with 1 vlan at here
	ca_classifier_rule_t ca_cls_rule;
	int ret;

	memcpy(&ca_cls_rule,ca_cls_rule_vlan,sizeof(ca_cls_rule));

	ca_cls_rule.key_mask.l2_mask.vlan_itag = FALSE;
	bzero(&ca_cls_rule.key_mask.l2_mask.vlan_otag_mask,sizeof(ca_classifier_vlan_mask_t));
	bzero(&ca_cls_rule.key_mask.l2_mask.vlan_itag_mask,sizeof(ca_classifier_vlan_mask_t));

	if(need_vlan_rule == 1){	//untag rule
		ca_cls_rule.key_mask.l2 = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_count = TRUE;
		ca_cls_rule.key.l2.vlan_count = 0;
	}else{						//c is outter
		ca_cls_rule.key_mask.l2 = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_count = TRUE;
		ca_cls_rule.key.l2.vlan_count = 1;
		ca_cls_rule.key_mask.l2_mask.vlan_otag = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.tpid = TRUE;
		ca_cls_rule.key.l2.vlan_otag.vlan_min.tpid = RT_ACL_PATTERN_CVLAN_TPID;
		if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_VID_BIT){
			ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.vid = TRUE;
			ca_cls_rule.key.l2.vlan_otag.vlan_min.vid = acl_filter->ingress_ctag_vid;
		}
		if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_PRI_BIT){
			ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.pri = TRUE;
			ca_cls_rule.key.l2.vlan_otag.vlan_min.pri = acl_filter->ingress_ctag_pri;
		}
		if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_CFI_BIT){
			ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.dei = TRUE;
			ca_cls_rule.key.l2.vlan_otag.vlan_min.dei = acl_filter->ingress_ctag_cfi;
		}
		ca_cls_rule.key.l2.vlan_otag.vlan_max = ca_cls_rule.key.l2.vlan_otag.vlan_min;
	}

	ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, port);
	ca_cls_rule.key_mask.src_port= 1;

	ret = _rtk_fc_g3_caClsRuleAALAdd(&ca_cls_rule, index, aal_customize);
	if(ret == CA_E_FULL)
		return(RT_ERR_RG_ACL_ENTRY_FULL);
	else if(ret)
	{
		ACL_CTRL("add user ACL failed (ret=0x%x)!!!", ret);
		return(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
	}

	ca_cls_rule_vlan->index = ca_cls_rule.index;

	return ret;
}
#endif

static int _rtk_fc_aclHWEntry_maskLength_get_by_mask(uint8* maskValue, int maskSize, int* maskLength)
{
	int i,j,k;
	int length;

	for(i=0; i<maskSize; i++){
		if(maskValue[i] != 0xff)
			break;
	}
	if(i == maskSize){
		//SINGLE IP or MAC, mask=all 0xff
		*maskLength = 0;
		return (RT_ERR_RG_OK);
	}

	{ //Multiple IP or MAC by Mask
		for(i=0;i<maskSize;i++){
			if(maskValue[i]!=0xff){//i: get the first non 0xff byte
				break;
			}
		}
		for(j=0;j<8;j++){ // j: get the first bit 0 of the byte
			if(((maskValue[i]<<j)&0x80)!=0x80)
				break;
		}
		length = (i*8)+j;
		(*maskLength) = (8*maskSize) - length;

		/*make sure the mask is aligned*/
		for(k=j+1;k<8;k++){
			if(((maskValue[i]<<k)&0x80)!=0x0){ACL_CTRL("%s Range not in mask alignment(Byte %d Bit %d) \n", (maskSize==IPV6_ADDR_LEN)?"IPv6":"MAC", i, k); return(RT_ERR_RG_FAILED);}//rest bit should be 0
		}
		for(k=i+1;k<maskSize;k++){
			if(maskValue[k]!=0x0){ACL_CTRL("%s Range not in mask alignment(Byte %d) \n", (maskSize==IPV6_ADDR_LEN)?"IPv6":"MAC", k); return(RT_ERR_RG_FAILED);}//rest bytes should be 0x00
		}
	}

	return (RT_ERR_RG_OK);
}

static int _rtk_fc_aclHWEntry_support(rt_acl_filterAndQos_t *acl_filter)
{
    int i;
	uint32 mask = 0;
#if !defined(CONFIG_FC_G3_G3LITE_SERIES)
	int ret;
	int value_check = 0;
#endif

	if(acl_filter->filter_fields & RT_ACL_INGRESS_ETHERTYPE_BIT)
		mask |= (1<<RTK_ACL_FIELD_ETHERTYPE_ENC);

	if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_PRI_BIT)
		mask |= (1<<RTK_ACL_FIELD_TOP_VL_802_1P);

	if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_VID_BIT)
		mask |= (1<<RTK_ACL_FIELD_TOP_VID);

	if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_CFI_BIT)
		mask |= (1<<RTK_ACL_FIELD_TOP_DEI);

	if(acl_filter->filter_fields & RT_ACL_INGRESS_SMAC_BIT) {
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
		mask |= (1<<RTK_ACL_FIELD_MAC_SA);
#else
		ret = _rtk_fc_aclHWEntry_maskLength_get_by_mask(acl_filter->ingress_smac_mask,ETHER_ADDR_LEN,&value_check);
		if(value_check == 0)
			mask |= (1<<RTK_ACL_FIELD_MAC_SA);
		else
			mask |= (1<<RTK_ACL_FIELD_MAC_RANGE);
#endif
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_DMAC_BIT) {
		if(PATTERN_CHECK_DMAC_IS_MC(acl_filter))
			mask |= (1<<RTK_ACL_FIELD_MAC_DA_MC);
		else if(PATTERN_CHECK_DMAC_IS_MC_IPV4(acl_filter) || PATTERN_CHECK_DMAC_IS_MC_IPV6(acl_filter))
			mask |= (1<<RTK_ACL_FIELD_MAC_DA_IP_MC);
		else if(PATTERN_CHECK_DMAC_IS_UC(acl_filter))
			mask |= (1<<RTK_ACL_FIELD_MAC_DA_BC);
		else if(PATTERN_CHECK_DMAC_IS_RSVD(acl_filter))
			mask |= (1<<RTK_ACL_FIELD_MAC_DA_RSVD);
		else{
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
			mask |= (1<<RTK_ACL_FIELD_MAC_DA);
#else
			ret = _rtk_fc_aclHWEntry_maskLength_get_by_mask(acl_filter->ingress_dmac_mask,ETHER_ADDR_LEN,&value_check);
			if(value_check == 0)
				mask |= (1<<RTK_ACL_FIELD_MAC_DA);
			else
				mask |= (1<<RTK_ACL_FIELD_MAC_RANGE);
#endif
		}
	}

	if((acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_DSCP_BIT)
		|| (acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_DSCP_BIT)) {
		mask |= (1<<RTK_ACL_FIELD_IP_VER);
		mask |= (1<<RTK_ACL_FIELD_DSCP);
	}

	if((acl_filter->filter_fields & RT_ACL_INGRESS_L4_ICMP_BIT)
		|| (acl_filter->filter_fields & RT_ACL_INGRESS_L4_ICMPV6_BIT)) {
		mask |= (1<<RTK_ACL_FIELD_IP_VER);
		mask |= (1<<RTK_ACL_FIELD_IP_PROTOCOL);
	}

	if((acl_filter->filter_fields & RT_ACL_INGRESS_L4_TCP_BIT)
		|| (acl_filter->filter_fields & RT_ACL_INGRESS_L4_UDP_BIT)
		|| (acl_filter->filter_fields & RT_ACL_INGRESS_L4_POROTCAL_VALUE_BIT)) {
		mask |= (1<<RTK_ACL_FIELD_IP_PROTOCOL);
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAGIF_BIT)
		mask |= (1<<RTK_ACL_FIELD_VLAN_CNT);

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_SIP_BIT )
		mask |= (1<<RTK_ACL_FIELD_IPV6_SA);

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_DIP_BIT )
		mask |= (1<<RTK_ACL_FIELD_IPV6_DA);

	if((acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_TAGIF_BIT )
		|| (acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_TAGIF_BIT)) {
		mask |= (1<<RTK_ACL_FIELD_IP_VER);
	}

	if((acl_filter->filter_fields & RT_ACL_INGRESS_L4_SPORT_RANGE_BIT)
		|| (acl_filter->filter_fields & RT_ACL_INGRESS_L4_DPORT_RANGE_BIT)) {
		mask |= (1<<RTK_ACL_FIELD_L4_PORT);
		mask |= (1<<RTK_ACL_FIELD_L4_VLD);
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_SIP_RANGE_BIT) {
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
		mask |= (1<<RTK_ACL_FIELD_IPV4_SA);
#else
		if((acl_filter->ingress_src_ipv4_addr_start != acl_filter->ingress_src_ipv4_addr_end) &&
			((ret=_rtk_fc_aclHWEntry_maskLength_get_by_ipv4_range(acl_filter->ingress_src_ipv4_addr_start,acl_filter->ingress_src_ipv4_addr_end,&value_check)) != RT_ERR_RG_OK))
		{
			mask |= (1<<RTK_ACL_FIELD_IP_RANGE);
		}
		else
		{
			mask |= (1<<RTK_ACL_FIELD_IPV4_SA);
		}
#endif
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_DIP_RANGE_BIT) {
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
		mask |= (1<<RTK_ACL_FIELD_IPV4_DA);
#else
		/* not support sa ip range + da ip range,  one sa ip + da ip range,  sa ip range + one da ip */
		if(mask & (1<<RTK_ACL_FIELD_IP_RANGE)){
			WARNING("HW ACL not support RT_ACL_INGRESS_IPV4_DIP_RANGE_BIT/RT_ACL_INGRESS_IPV4_SIP_RANGE_BIT at the same time.");
			return(RT_ERR_RG_INVALID_PARAM);
		}
		if((acl_filter->ingress_dest_ipv4_addr_start != acl_filter->ingress_dest_ipv4_addr_end) &&
			((ret=_rtk_fc_aclHWEntry_maskLength_get_by_ipv4_range(acl_filter->ingress_dest_ipv4_addr_start,acl_filter->ingress_dest_ipv4_addr_end,&value_check)) != RT_ERR_RG_OK))
		{
			mask |= (1<<RTK_ACL_FIELD_IP_RANGE);
			if(mask & (1<<RTK_ACL_FIELD_IPV4_SA)){
				WARNING("HW ACL not support RT_ACL_INGRESS_IPV4_DIP_RANGE_BIT/RT_ACL_INGRESS_IPV4_SIP_RANGE_BIT at the same time.");
				return(RT_ERR_RG_INVALID_PARAM);
			}
		}
		else
		{
			mask |= (1<<RTK_ACL_FIELD_IPV4_DA);
		}
#endif
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_TCP_FLAGS_BIT)
		mask |= (1<<RTK_ACL_FIELD_TCP_RDP_CTRL);

	if(acl_filter->filter_fields & RT_ACL_INGRESS_PKT_LEN_RANGE_BIT)
		mask |= (1<<RTK_ACL_FIELD_PKTLEN_RNG_MATCH_VEC);

	for(i=0; i<RTK_ACL_TEMPLATE_CA_END; i++) {
		if (mask == (mask & fc_db.ca_cls_template[i])) {
			ACL_CTRL("HW template mapping success: mask 0x%x use template %d(0x%x)", mask, i, fc_db.ca_cls_template[i]);
			return (RT_ERR_RG_OK);
		}
	}
	ACL_CTRL("HW ACL not support these pattern(mask=0x%x), and reset ACL will not add to hw.", mask);

	return(RT_ERR_RG_INVALID_PARAM);
}

static int _rtk_fc_aclHWEntry_not_support(rt_acl_filterAndQos_t *acl_filter)
{
	int ret;
	int value_check = 0;

#if defined(CONFIG_FC_G3_G3LITE_SERIES)
	if(acl_filter->filter_fields&RT_ACL_INGRESS_STAGIF_BIT)
	{
		WARNING("RT_ACL_INGRESS_STAGIF_BIT is not support.");
		return (RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	if(acl_filter->filter_fields&RT_ACL_INGRESS_STAG_VID_BIT)
	{
		WARNING("RT_ACL_INGRESS_STAG_VID_BIT is not support.");
		return (RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	if(acl_filter->filter_fields&RT_ACL_INGRESS_STAG_PRI_BIT)
	{
		WARNING("RT_ACL_INGRESS_STAG_PRI_BIT is not support.");
		return (RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	if(acl_filter->filter_fields&RT_ACL_INGRESS_STAG_DEI_BIT)
	{
		WARNING("RT_ACL_INGRESS_STAG_DEI_BIT is not support.");
		return (RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
#endif

	//Packet parser not support due to stag parsing earler than ctag
	if((acl_filter->filter_fields & RT_ACL_INGRESS_STAGIF_BIT) && (acl_filter->ingress_stagif == 0) && RT_ACL_PATTERN_IS_STPID_EQ_CTPID &&
		(((acl_filter->filter_fields & RT_ACL_INGRESS_CTAGIF_BIT) && (acl_filter->ingress_ctagif != 0)) ||
		(acl_filter->filter_fields & (RT_ACL_INGRESS_CTAG_VID_BIT|RT_ACL_INGRESS_CTAG_PRI_BIT|RT_ACL_INGRESS_CTAG_CFI_BIT))))
	{
		WARNING("RT_ACL pattern ingress_stagif=0 + ctag_xxx is not support when stag tpid the same as ctag tpid.");
		return (RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

#if !(defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES))
	if(acl_filter->filter_fields&RT_ACL_INGRESS_STREAM_ID_BIT)
	{
		WARNING("RT_ACL_INGRESS_STREAM_ID_BIT is not support.");
		return (RT_ERR_RG_CHIP_NOT_SUPPORT);
	}
#endif

#if defined(CONFIG_FC_G3_G3LITE_SERIES)
	if(acl_filter->filter_fields & RT_ACL_INGRESS_TCP_FLAGS_BIT)
	{
		if(ACL_CHECK_HASH_NEED(acl_filter)){
			//support syn related function by CLS + hash
			if(acl_filter->filter_fields & ~(RT_ACL_INGRESS_PORT_MASK_BIT|RT_ACL_INGRESS_L4_POROTCAL_VALUE_BIT|RT_ACL_INGRESS_L4_TCP_BIT|RT_ACL_INGRESS_TCP_FLAGS_BIT)){
				WARNING("ACL for TCP SYN only/SYN-ACK only support pattern: port/protocol=tcp/tcp_flag.");
				return (RT_ERR_RG_INVALID_PARAM);
			}
			if((acl_filter->filter_fields & RT_ACL_INGRESS_L4_POROTCAL_VALUE_BIT) && (acl_filter->ingress_l4_protocal != 0x6)){
				WARNING("ACL for TCP SYN only/SYN-ACK of pattern protocol value should be tcp(0x6).");
				return (RT_ERR_RG_INVALID_PARAM);
			}
			if(((acl_filter->action_fields & RT_ACL_ACTION_METER_GROUP_SHARE_METER_BIT) == 0x0) &&
				((acl_filter->action_fields & RT_ACL_ACTION_PRIORITY_GROUP_TRAP_PRIORITY_BIT) == 0x0) && 
				((acl_filter->action_fields & RT_ACL_ACTION_PRIORITY_GROUP_ACL_PRIORITY_BIT) == 0x0)){
				WARNING("ACL for TCP SYN only/SYN-ACK only support higher priority, trap with priority or assign share meter.");
				return (RT_ERR_RG_CHIP_NOT_SUPPORT);
			}
		}else if((acl_filter->ingress_tcp_flags_mask == 0xfff) || (acl_filter->ingress_tcp_flags != acl_filter->ingress_tcp_flags_mask)){
			//[G3 Known Issue] tcp flag and mask is OR check (ex: config TCP SYN only, TCP SYN+ACK also hit)
			WARNING("Please assign ingress_tcp_flags_mask with only one bit enable(ex: only support has ack check rather than ack only).");
			return (RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_PKT_LEN_RANGE_BIT)
	{
		{	//8277A limitation: overlap packet length range cause not hit, so only support limit range at this step
			uint8 loop_idx;
			l3_cam_pkt_len_tbl_entry_t cam_pkt_len;

			for(loop_idx = 0; loop_idx < L3_CAM_PKT_LEN_TBL_ENTRY_MAX; loop_idx++){
				if (AAL_E_OK != aal_entry_get_by_idx(AAL_TABLE_L3_CAM_PKT_LEN, loop_idx, &cam_pkt_len))
					continue;
				if(cam_pkt_len.vld == 0)
					continue;
				if(((acl_filter->ingress_packet_length_start >= cam_pkt_len.pkt_len_low) && (acl_filter->ingress_packet_length_start <= cam_pkt_len.pkt_len_hi)) ||
					((acl_filter->ingress_packet_length_end >= cam_pkt_len.pkt_len_low) && (acl_filter->ingress_packet_length_end <= cam_pkt_len.pkt_len_hi)))
				{
					WARNING("RT_ACL_INGRESS_PKT_LEN_RANGE_BIT not support packet length range %d~%d due to it overlap with entry %d (%d~%d).", acl_filter->ingress_packet_length_start, acl_filter->ingress_packet_length_end, loop_idx, cam_pkt_len.pkt_len_low, cam_pkt_len.pkt_len_hi);
					return (RT_ERR_RG_CHIP_NOT_SUPPORT);
				}
			}
		}
		if (acl_filter->ingress_packet_length_start > 0x3fff || acl_filter->ingress_packet_length_end > 0x3fff)
		{
			WARNING("HW ACL not support packet length range larger than 0x3fff.");
			return (RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}
#else
	if(acl_filter->filter_fields & RT_ACL_INGRESS_TCP_FLAGS_BIT)
	{
		if((acl_filter->ingress_tcp_flags & 0x2) &&
			(acl_filter->action_fields & RT_ACL_ACTION_FORWARD_GROUP_TRAP_BIT) && (acl_filter->action_fields & RT_ACL_ACTION_PRIORITY_GROUP_TRAP_PRIORITY_BIT)){
			acl_filter->action_fields &= ~RT_ACL_ACTION_FORWARD_GROUP_TRAP_BIT;
			acl_filter->action_fields &= ~RT_ACL_ACTION_PRIORITY_GROUP_TRAP_PRIORITY_BIT;
			acl_filter->action_fields |= RT_ACL_ACTION_PRIORITY_GROUP_ACL_PRIORITY_BIT;
			acl_filter->action_priority_group_acl_priority = acl_filter->action_priority_group_trap_priority;
			ACL_CTRL("ACL for TCP has SYN, try to hw acc in most case.");
		}
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_PKT_LEN_RANGE_BIT)
	{
		if (acl_filter->ingress_packet_length_start > 0x3fff || acl_filter->ingress_packet_length_end > 0x3fff)
		{
			WARNING("HW ACL not support packet length range larger than 0x3fff.");
			return (RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
	}
#endif

	if(acl_filter->filter_fields & RT_ACL_INGRESS_ETHERTYPE_BIT)
	{
		if(acl_filter->ingress_ethertype_mask!=0xffff)
		{
			WARNING("HW ACL not support RT_ACL_INGRESS_ETHERTYPE_BIT with mask.");
			return(RT_ERR_RG_INVALID_PARAM);
		}
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_SMAC_BIT)
	{
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
		if(!(acl_filter->ingress_smac_mask[0]==0xff &&
			acl_filter->ingress_smac_mask[1]==0xff &&
			acl_filter->ingress_smac_mask[2]==0xff &&
			acl_filter->ingress_smac_mask[3]==0xff &&
			acl_filter->ingress_smac_mask[4]==0xff &&
			acl_filter->ingress_smac_mask[5]==0xff))
		{
			WARNING("HW ACL not support RT_ACL_INGRESS_SMAC_BIT with mask.");
			return(RT_ERR_RG_INVALID_PARAM);
		}
#else
		ret = _rtk_fc_aclHWEntry_maskLength_get_by_mask(acl_filter->ingress_smac_mask,ETHER_ADDR_LEN,&value_check);
		if(ret != RT_ERR_RG_OK)
		{
			WARNING("HW ACL not support RT_ACL_INGRESS_SMAC_BIT with mask not aligned.");
			return(RT_ERR_RG_INVALID_PARAM);
		}
#endif
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_DMAC_BIT)
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
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
		else if(!(acl_filter->ingress_dmac_mask[0]==0xff &&
			acl_filter->ingress_dmac_mask[1]==0xff &&
			acl_filter->ingress_dmac_mask[2]==0xff &&
			acl_filter->ingress_dmac_mask[3]==0xff &&
			acl_filter->ingress_dmac_mask[4]==0xff &&
			acl_filter->ingress_dmac_mask[5]==0xff))
		{
			WARNING("HW ACL not support RT_ACL_INGRESS_DMAC_BIT with mask(only support 01/3333/01005E/0180C20000/UC).");
			return(RT_ERR_RG_INVALID_PARAM);
		}
#else
		else if((ret=_rtk_fc_aclHWEntry_maskLength_get_by_mask(acl_filter->ingress_dmac_mask,ETHER_ADDR_LEN,&value_check)) != RT_ERR_RG_OK)
		{
			WARNING("HW ACL not support RT_ACL_INGRESS_DMAC_BIT with mask not aligned.");
			return(RT_ERR_RG_INVALID_PARAM);
		}
#endif
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_SIP_BIT )
	{
		ret = _rtk_fc_aclHWEntry_maskLength_get_by_mask(acl_filter->ingress_src_ipv6_addr_mask,IPV6_ADDR_LEN,&value_check);
		if(ret != RT_ERR_RG_OK)
		{
			WARNING("HW ACL not support RT_ACL_INGRESS_IPV6_SIP_BIT with mask not aligned.");
			return(RT_ERR_RG_INVALID_PARAM);
		}
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_DIP_BIT )
	{
		ret = _rtk_fc_aclHWEntry_maskLength_get_by_mask(acl_filter->ingress_dest_ipv6_addr_mask,IPV6_ADDR_LEN,&value_check);
		if(ret != RT_ERR_RG_OK)
		{
			WARNING("HW ACL not support RT_ACL_INGRESS_IPV6_DIP_BIT with mask not aligned.");
			return(RT_ERR_RG_INVALID_PARAM);
		}
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_TAGIF_BIT )
	{
		if(acl_filter->ingress_ipv4_tagif==0)//must not IPv4
		{
			WARNING("HW ACL not support RT_ACL_INGRESS_IPV4_TAGIF_BIT reverse check.");
			return(RT_ERR_RG_INVALID_PARAM);
		}
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_TAGIF_BIT)
	{
		if(acl_filter->ingress_ipv6_tagif==0)//must not IPv6
		{
			WARNING("HW ACL not support RT_ACL_INGRESS_IPV6_TAGIF_BIT reverse check.");
			return(RT_ERR_RG_INVALID_PARAM);
		}
	}

#if defined(CONFIG_FC_G3_G3LITE_SERIES)
	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_SIP_RANGE_BIT)
	{
		if(acl_filter->ingress_src_ipv4_addr_start != acl_filter->ingress_src_ipv4_addr_end)
		{
			ret = _rtk_fc_aclHWEntry_maskLength_get_by_ipv4_range(acl_filter->ingress_src_ipv4_addr_start,acl_filter->ingress_src_ipv4_addr_end,&value_check);
			if(ret != RT_ERR_RG_OK){
				WARNING("HW ACL not support RT_ACL_INGRESS_IPV4_SIP_RANGE_BIT with range not in mask alignment.");
				return(RT_ERR_RG_INVALID_PARAM);
			}
		}
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_DIP_RANGE_BIT)
	{
		if(acl_filter->ingress_dest_ipv4_addr_start != acl_filter->ingress_dest_ipv4_addr_end)
		{
			ret = _rtk_fc_aclHWEntry_maskLength_get_by_ipv4_range(acl_filter->ingress_dest_ipv4_addr_start,acl_filter->ingress_dest_ipv4_addr_end,&value_check);
			if(ret != RT_ERR_RG_OK){
				WARNING("HW ACL not support RT_ACL_INGRESS_IPV4_DIP_RANGE_BIT with range not in mask alignment.");
				return(RT_ERR_RG_INVALID_PARAM);
			}
		}
	}
#endif

	if(acl_filter->action_fields & RT_ACL_ACTION_METER_GROUP_SHARE_METER_BIT)
	{
		if(acl_filter->filter_fields & RT_ACL_INGRESS_WLAN_MBSSID_MASK_BIT)
		{
			WARNING("HW ACL not support METER action with wlan mbssid pattern.");
			return (RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
		//Flow meter will overwrite ACL meter, so ACL should trap to keep meter index. However, hw forwarding with ACL meter still can work if flow meter disable.
		//FC Purpose: acl policing + hw forwarding without policing in most case. If user want to ACL polcing > flow policing, then user can enable ACL trap by themself at the same time.
		//Known issue: 8277B to keep stream id from lan/wan port(hwPolRemap), so will be always acl policing + sw forwarding ==> use pol2 to prevent this issue, so remove the limitation
		//if((acl_filter->action_fields & RT_ACL_ACTION_FORWARD_GROUP_TRAP_BIT) == 0x0)
		//{
			//WARNING("HW ACL not support METER action without TRAP action(Flow policing > ACL policing).");
			//return (RT_ERR_RG_CHIP_NOT_SUPPORT);
		//}
	}

	if((acl_filter->action_fields&RT_ACL_ACTION_LOGGING_GROUP_MIB_BIT) && (acl_filter->action_logging_group_mib_index >= MAX_ACL_LOG_MIB_NUM))
	{
		WARNING("Logging action only support mib index 0~%d, not support %d.", MAX_ACL_LOG_MIB_NUM-1, acl_filter->action_logging_group_mib_index);
		return (RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	if((acl_filter->action_fields&RT_ACL_ACTION_FORWARD_GROUP_TRAP_SLAVECPU_BIT))
	{
		WARNING("HW ACL not support TRAO TO SLAVE CPU action.");
		return (RT_ERR_RG_CHIP_NOT_SUPPORT);
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_ARP_TARGET_IP_BIT)
	{
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
		WARNING("RT_ACL_INGRESS_ARP_TARGET_IP_BIT is not support.");
		return (RT_ERR_RG_CHIP_NOT_SUPPORT);
#else
		//only support one format(untag/stag,catg/sctag) at the same time (by special header L3FE_PP_OFFSET_HDR.offset)
		if(fc_db.controlFuc.acl_arp_targetip_offset){
			uint32 arpOffset = 1;
			if((acl_filter->filter_fields & RT_ACL_INGRESS_STAGIF_BIT) && (acl_filter->ingress_stagif == 0) &&
				(acl_filter->filter_fields & RT_ACL_INGRESS_CTAGIF_BIT) && (acl_filter->ingress_ctagif == 0)){
				//untag
			}else{
				if(acl_filter->filter_fields & (RT_ACL_INGRESS_STAGIF_BIT|RT_ACL_INGRESS_STAG_VID_BIT|RT_ACL_INGRESS_STAG_PRI_BIT|RT_ACL_INGRESS_STAG_DEI_BIT))
					arpOffset += 4;
				if(acl_filter->filter_fields & (RT_ACL_INGRESS_CTAGIF_BIT|RT_ACL_INGRESS_CTAG_VID_BIT|RT_ACL_INGRESS_CTAG_PRI_BIT|RT_ACL_INGRESS_CTAG_CFI_BIT))
					arpOffset += 4;
			}
			if(arpOffset != fc_db.controlFuc.acl_arp_targetip_offset){
				WARNING("RT_ACL_INGRESS_ARP_TARGET_IP_BIT only support one format(untag/stag or catg/sctag) at the same time, already have %s rule.",
					((fc_db.controlFuc.acl_arp_targetip_offset==1)?"untag":((fc_db.controlFuc.acl_arp_targetip_offset==(8+1))?"2tags":("1tag"))));
				return (RT_ERR_RG_CHIP_NOT_SUPPORT);
			}
		}
#endif
	}

	ret = _rtk_fc_aclHWEntry_support(acl_filter);
	if(ret != RT_ERR_RG_OK)
		return(RT_ERR_RG_ACL_NOT_SUPPORT);

	return (RT_ERR_RG_OK);
}


static int _rtk_fc_acl_portmask_translator(uint32 port_mask, rtk_portmask_t* mac_pmsk)
{
	int i;

	if(mac_pmsk==NULL )
		return (RT_ERR_RG_NULL_POINTER);

	bzero(mac_pmsk,sizeof(rtk_portmask_t));

	for(i=0;i<RT_PORT_MAX;i++){
		if((port_mask & (1<<i))==0x0)
			continue;
		if(RT_ACL_PORT_ALL_WAN_BIT & (1<<i)){	//G3 default RT_PORT_PON=7, but may use port 4 as wan port
			mac_pmsk->bits[0] |= (1<<i);
			continue;
		}
		mac_pmsk->bits[0] |= (1<<i);
	}

	return (RT_ERR_RG_OK);
}

#if defined(CONFIG_FC_CA8277AB_SERIES)
static int _rtk_fc_acl_wlan_mbssid_mask_translator(rt_acl_wlan_mbssid_mask_t mbssid_mask[], int size, rtk_portmask_t* ext_pmsk)
{
	int i, j, sharedSSID = 0;
	rtk_fc_wlan_devidx_t wlanDevIdx;
	rtk_fc_wlan_devmask_t wlanDevMask, checkDoneMask = 0;
	rtk_fc_wlan_devmap_t *wlanDevMap = NULL;
	rtk_fc_mac_port_idx_t macPortIdx;

	if(ext_pmsk==NULL )
		return (RT_ERR_RG_NULL_POINTER);

	bzero(ext_pmsk,sizeof(rtk_portmask_t)*size);

	for(j=0;j<size;j++){
		for(i=0;i<RT_WLAN_MBSSID_MAX;i++){
			if(((mbssid_mask[j] & (1<<i))==0x0) && (mbssid_mask[j] != RT_WLAN_MBSSID_ALL_BIT))
				continue;

			wlanDevIdx = rtk_fc_wlan_rtmbssid_to_devidx(j, i);
			
			if(checkDoneMask & (1LL<<wlanDevIdx))
				continue;

			if(RTK_FC_HELPER_WLAN_DEVMAP_GET(wlanDevIdx, &wlanDevMap) == SUCCESS) {
				macPortIdx = RTK_FC_HELPER_WLAN_DEVMAP_MACPORTIDX_GET(wlanDevMap);
				RTK_FC_HELPER_WLAN_CPUPORT_TO_DEVID(macPortIdx, &wlanDevIdx, &wlanDevMask);
				checkDoneMask |= wlanDevMask;
				wlanDevMask &= ~(1LL<<wlanDevIdx);
				if(wlanDevIdx == RTK_FC_WLANX_MULTI_INTF)
				{
					// TODO: consider different band but share same cpu/ext port.
					// check all wlan dev is configured, return fail if not.
					for(sharedSSID=0; sharedSSID<RT_WLAN_MBSSID_MAX; sharedSSID++) {
						if((mbssid_mask[j] & (1<<sharedSSID))==0x0)
							continue;

						wlanDevIdx = rtk_fc_wlan_rtmbssid_to_devidx(j, sharedSSID);

						if(wlanDevIdx == RTK_FC_WLANX_NOT_FOUND) {
							WARNING("This chip not support mbssid %d", sharedSSID);
							return (RT_ERR_RG_CHIP_NOT_SUPPORT);
						}

						wlanDevMask &= ~(1LL<<wlanDevIdx);
					}

				}

				if(wlanDevMask!=0ULL) {
					WARNING("Some mbssid (%llx) in wlan%d belongs to others, you should config all", wlanDevMask, j);
					return (RT_ERR_RG_CHIP_NOT_SUPPORT);
				}else {
					//write all mac port into ext_pmsk[RT_WLAN_DEVICE0_INDEX]
					ext_pmsk[RT_WLAN_DEVICE0_INDEX].bits[0] |= (1<<macPortIdx);
				}
			}else if(mbssid_mask[j] != RT_WLAN_MBSSID_ALL_BIT) {

				WARNING("This chip not support wlan%d mbssid %d", j, i);
				return (RT_ERR_RG_CHIP_NOT_SUPPORT);
			}

		}
	}

	return (RT_ERR_RG_OK);
}
#else
static int _rtk_fc_acl_wlan_mbssid_mask_translator(rt_acl_wlan_mbssid_mask_t mbssid_mask[], int size, rtk_portmask_t* ext_pmsk, uint8* wlan_dev_id, int wlan_dev_cnt)
{
	int i, j;
	rtk_fc_wlan_devidx_t wlanDevIdx;
	rtk_fc_wlan_devmap_t *wlanDevMap = NULL;
	rtk_fc_mac_port_idx_t macPortIdx;

	if(ext_pmsk==NULL )
		return (RT_ERR_RG_NULL_POINTER);

	bzero(ext_pmsk,sizeof(rtk_portmask_t)*size);
	bzero(wlan_dev_id,sizeof(uint8)*wlan_dev_cnt);

	macPortIdx = RTK_FC_MAC_PORT_CPU;

	for(j=0;j<size;j++){
		for(i=0;i<RT_WLAN_MBSSID_MAX;i++){
			if(((mbssid_mask[j] & (1<<i))==0x0) && (mbssid_mask[j] != RT_WLAN_MBSSID_ALL_BIT))
				continue;

			wlanDevIdx = rtk_fc_wlan_rtmbssid_to_devidx(j, i);

			if(RTK_FC_HELPER_WLAN_DEVMAP_GET(wlanDevIdx, &wlanDevMap) == SUCCESS) {

				if(macPortIdx >= RTK_FC_MAC_PORT_MAX){
					WARNING("Current design not support configure wlan count more than %d in one rule.", (RTK_FC_MAC_PORT_MAX-RTK_FC_MAC_PORT_CPU));
					return (RT_ERR_RG_INVALID_PARAM);
				}
				wlan_dev_id[macPortIdx-RTK_FC_MAC_PORT_CPU] = wlanDevIdx;		//wlan0 in PORT_CPU, wlan1 in PORT_CPU1, ..., wlan7 in PORT_WLAN_CPU5, max wlan count 8
				ext_pmsk[RT_WLAN_DEVICE0_INDEX].bits[0] |= (1<<macPortIdx);		//valid bit
				macPortIdx++;

			}else if(mbssid_mask[j] != RT_WLAN_MBSSID_ALL_BIT) {

				WARNING("This chip not support wlan%d mbssid %d", j, i);
				return (RT_ERR_RG_CHIP_NOT_SUPPORT);
			}

		}
	}

	return (RT_ERR_RG_OK);
}
#endif

#if defined(FC_USER_ACL_CA_CLS_SUPPORT)
static int _rtk_fc_aclSWEntry_to_asic_add(rt_acl_filterAndQos_t *acl_filter,rtk_fc_aclFilterEntry_t* aclSWEntry,int shareHwAclWithSWAclIdx)
{
	int i,index,ret;
	int aclCountinueRuleSize;
	rtk_portmask_t mac_pmsk;
	rtk_portmask_t ext_pmsk[RT_WLAN_DEVICE_MAX];	//only support wlan[0]
	rtk_portmask_t activePorts;
	rtk_portmask_t caCLSPort;
	uint8 wlan_count=RT_WLAN_DEVICE_MAX;
	ca_classifier_rule_t ca_cls_rule;
	uint8 ca_index_offset;
	char ca_index_buffer[128];
	uint32 ca_index_hw_info = CA_UINT32_INVALID;
	int unmask_length;
	rt_rate_meter_mapping_t meterMapping;
	unsigned int aal_customize;
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
	bool is_hash = FALSE;
#else
	uint8 need_vlan_rule = 0;
	uint16 outter_tpid, inner_tpid;
#endif

	aclSWEntry->hw_aclEntry_count = 0;
	bzero(&activePorts,sizeof(rtk_portmask_t));
	bzero(&caCLSPort,sizeof(rtk_portmask_t));
	bzero(&ca_cls_rule,sizeof(ca_cls_rule));
	bzero(&ca_index_buffer,sizeof(char));
	ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L3_USER;
	aclCountinueRuleSize=0;
	aal_customize = CA_CLASSIFIER_AAL_NA;

	if(shareHwAclWithSWAclIdx >= 0)
		index = RTK_FC_ACLANDCF_RESERVED_TAIL_END + shareHwAclWithSWAclIdx;
	else
		index = FAIL;//pre-add test

	/*Skip ----------------- fwding_type_and_direction/HW_ACL_SUPPORT_PATTERN check ----------------- Skip*/

	ASSERT_EQ(_rtk_fc_acl_portmask_translator(acl_filter->ingress_port_mask, &mac_pmsk),RT_ERR_RG_OK);
	if(acl_filter->filter_fields & RT_ACL_INGRESS_WLAN_MBSSID_MASK_BIT)
		ASSERT_EQ(_rtk_fc_acl_wlan_mbssid_mask_translator(acl_filter->ingress_wlan_mbssid_mask, wlan_count, ext_pmsk),RT_ERR_RG_OK);
	else
		bzero(ext_pmsk,sizeof(rtk_portmask_t)*wlan_count);

	/*set patterns*/

	if(acl_filter->filter_fields & RT_ACL_CARE_PORT_UNITYPE_PPTP_BIT) {
		if(fc_db.hypridPPTP.portmask)
			ACL_CTRL("= care pptp_port=0x%llx, so mac_port=0x%x =",fc_db.hypridPPTP.portmask,mac_pmsk.bits[0]);
	}else if(fc_db.hypridPPTP.portmask){
		//disable pptp port configuration in all related fc_db.aclRule
		mac_pmsk.bits[0] &= (~fc_db.hypridPPTP.portmask);
		ACL_CTRL("= disable acl rule for pptp_port=0x%llx, so mac_port=0x%x =",fc_db.hypridPPTP.portmask,mac_pmsk.bits[0]);
	}
	if((((mac_pmsk.bits[0]&RT_ACL_PORT_ALL_LAN_BIT)==RT_ACL_PORT_ALL_LAN_BIT) && ((acl_filter->filter_fields & RT_ACL_INGRESS_WLAN_MBSSID_MASK_BIT)==0x0)
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
			&& ((acl_filter->action_fields & RT_ACL_ACTION_METER_GROUP_SHARE_METER_BIT) == 0x0)
#endif
		) || ((acl_filter->ingress_port_mask == RTK_FC_ALL_MAC_PORTMASK) && ((acl_filter->filter_fields & RT_ACL_CARE_PORT_UNITYPE_PPTP_BIT) || (fc_db.hypridPPTP.portmask == 0x0))) )
	{
		//[optimize] all physical port and wifi port: in L3 CLS, it means profile #lan and profile #wan
		activePorts.bits[0] = CA_L3_CLS_PROFILE_LAN;
		if((mac_pmsk.bits[0]&RT_ACL_PORT_ALL_WAN_BIT)==RT_ACL_PORT_ALL_WAN_BIT)
			activePorts.bits[0] |= CA_L3_CLS_PROFILE_WAN;
		aal_customize |= CA_CLASSIFIER_AAL_L3_IGNORE_SRC_PORT;
		ACL_CTRL("= set acl rule for activePorts = 0x%x due to both mac_pmsk/ext_pmsk is config as all port =",activePorts.bits[0]);
	}
	else
	{
		if((mac_pmsk.bits[0]!=0x0) && ((mac_pmsk.bits[0]&RT_ACL_PORT_ALL_WAN_BIT)==RT_ACL_PORT_ALL_WAN_BIT))
		{	//wan port + lan port
			activePorts.bits[0] = CA_L3_CLS_PROFILE_WAN;
			activePorts.bits[0] |= (mac_pmsk.bits[0]&(~RT_ACL_PORT_ALL_WAN_BIT));
		}
		else if(mac_pmsk.bits[0]!=0x0)
		{	//only lan port
			activePorts = mac_pmsk;
		}
		if(ext_pmsk[RT_WLAN_DEVICE0_INDEX].bits[0]!=0x0)
		{
			activePorts.bits[0] |= ext_pmsk[RT_WLAN_DEVICE0_INDEX].bits[0];
		}
		ACL_CTRL("= set acl rule for activePorts = 0x%x =",activePorts.bits[0]);
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_SMAC_BIT)
	{
		//HW ACL not support "mask", acl_filter->ingress_smac_mask
		ca_cls_rule.key_mask.l2 = TRUE;
		ca_cls_rule.key_mask.l2_mask.mac_sa = TRUE;
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
		memcpy(ca_cls_rule.key.l2.mac_sa.mac_min,acl_filter->ingress_smac,sizeof(ca_mac_addr_t));
		memcpy(ca_cls_rule.key.l2.mac_sa.mac_max,acl_filter->ingress_smac,sizeof(ca_mac_addr_t));
#else
		for(i=0; i<ETHER_ADDR_LEN; i++)
			ca_cls_rule.key.l2.mac_sa.mac_min[i] = (acl_filter->ingress_smac[i]&acl_filter->ingress_smac_mask[i]);
		for(i=0; i<ETHER_ADDR_LEN; i++)
			ca_cls_rule.key.l2.mac_sa.mac_max[i] = (acl_filter->ingress_smac[i]&acl_filter->ingress_smac_mask[i]) + ~(acl_filter->ingress_smac_mask[i]);
#endif
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_DMAC_BIT)
	{
		//HW ACL not support "mask", acl_filter->ingress_dmac_mask
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
		}else if(PATTERN_CHECK_DMAC_IS_UC(acl_filter)){	//DA[40] = 0, DA_mask[40] = 1b1
			ca_cls_rule.key_mask.l2 = TRUE;
			ca_cls_rule.key_mask.l2_mask.is_multicast = TRUE;
			ca_cls_rule.key.l2.is_multicast = FALSE;
			aal_customize |= CA_CLASSIFIER_AAL_L3_NOT_BROADCAST;
		}else if(PATTERN_CHECK_DMAC_IS_RSVD(acl_filter)){	//DA[47:8] = 40h01_80_C2_00_00)
			aal_customize |= CA_CLASSIFIER_AAL_L3_MAC_DA_RSVD;
		}else{
			ca_cls_rule.key_mask.l2 = TRUE;
			ca_cls_rule.key_mask.l2_mask.mac_da = TRUE;
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
			memcpy(ca_cls_rule.key.l2.mac_da.mac_min,acl_filter->ingress_dmac,sizeof(ca_mac_addr_t));
			memcpy(ca_cls_rule.key.l2.mac_da.mac_max,acl_filter->ingress_dmac,sizeof(ca_mac_addr_t));
#else
			for(i=0; i<ETHER_ADDR_LEN; i++)
				ca_cls_rule.key.l2.mac_da.mac_min[i] = (acl_filter->ingress_dmac[i]&acl_filter->ingress_dmac_mask[i]);
			for(i=0; i<ETHER_ADDR_LEN; i++)
				ca_cls_rule.key.l2.mac_da.mac_max[i] = (acl_filter->ingress_dmac[i]&acl_filter->ingress_dmac_mask[i]) + ~(acl_filter->ingress_dmac_mask[i]);
#endif
		}
	}


	if(acl_filter->filter_fields & RT_ACL_INGRESS_ETHERTYPE_BIT)
	{
		//HW ACL not support "mask", acl_filter->ingress_ethertype_mask
		ca_cls_rule.key_mask.l2 = TRUE;
		ca_cls_rule.key_mask.l2_mask.ethertype = TRUE;
		ca_cls_rule.key.l2.ethertype = acl_filter->ingress_ethertype;

		//Bug fix CA HW fail to keep original for ethertype=0x8864
		if(acl_filter->ingress_ethertype == 0x8864)
			aal_customize |= CA_CLASSIFIER_AAL_L3_KEEP_ORIG;
	}

#if defined(CONFIG_FC_G3_G3LITE_SERIES)
	if(acl_filter->filter_fields & RT_ACL_INGRESS_STAGIF_BIT)
	{
		//not support yet
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_STAG_VID_BIT)
	{
		//not support yet
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_STAG_PRI_BIT)
	{
		//not support yet
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_STAG_DEI_BIT)
	{
		//not support yet
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAGIF_BIT)
	{
		ca_cls_rule.key_mask.l2 = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_count = TRUE;
		if(acl_filter->ingress_ctagif == 0)
			ca_cls_rule.key.l2.vlan_count = 0;
		else
			ca_cls_rule.key.l2.vlan_count = 1;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_VID_BIT)
	{
		ca_cls_rule.key_mask.l2 = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_otag = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.vid = TRUE;
		ca_cls_rule.key.l2.vlan_otag.vlan_min.vid = acl_filter->ingress_ctag_vid;
		ca_cls_rule.key.l2.vlan_otag.vlan_max = ca_cls_rule.key.l2.vlan_otag.vlan_min;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_PRI_BIT)
	{
		ca_cls_rule.key_mask.l2 = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_otag = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.pri = TRUE;
		ca_cls_rule.key.l2.vlan_otag.vlan_min.pri = acl_filter->ingress_ctag_pri;
		ca_cls_rule.key.l2.vlan_otag.vlan_max = ca_cls_rule.key.l2.vlan_otag.vlan_min;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_CFI_BIT)
	{
		ca_cls_rule.key_mask.l2 = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_otag = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.dei = TRUE;
		ca_cls_rule.key.l2.vlan_otag.vlan_min.dei = acl_filter->ingress_ctag_cfi;
		ca_cls_rule.key.l2.vlan_otag.vlan_max = ca_cls_rule.key.l2.vlan_otag.vlan_min;
	}
#else
	outter_tpid = 0;
	inner_tpid = 0;
	if((acl_filter->filter_fields & RT_ACL_INGRESS_STAGIF_BIT) && (acl_filter->ingress_stagif == 0) &&
		(acl_filter->filter_fields & RT_ACL_INGRESS_CTAGIF_BIT) && (acl_filter->ingress_ctagif == 0))
	{	//untag
		ca_cls_rule.key_mask.l2 = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_count = TRUE;
		ca_cls_rule.key.l2.vlan_count = 0;
	}else if(((acl_filter->filter_fields & RT_ACL_INGRESS_STAGIF_BIT) && (acl_filter->ingress_stagif == 0)) ||
			((acl_filter->filter_fields & RT_ACL_INGRESS_CTAGIF_BIT) && (acl_filter->ingress_ctagif == 0))){
		//vlan count = 1 here, and untag at _rtk_fc_aclHWEntry_vlan_readd
		need_vlan_rule = 1;
		ca_cls_rule.key_mask.l2 = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_count = TRUE;
		ca_cls_rule.key.l2.vlan_count = 1;
		ca_cls_rule.key_mask.l2_mask.vlan_otag = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.tpid = TRUE;
		if((acl_filter->filter_fields & RT_ACL_INGRESS_STAGIF_BIT) && (acl_filter->ingress_stagif == 0)){
			ca_cls_rule.key.l2.vlan_otag.vlan_min.tpid = RT_ACL_PATTERN_CVLAN_TPID;	//ctag only
			//if(stag_tpid=ctag_tpid & check C also) _rtk_fc_aclHWEntry_not_support
			//if(stag_tpid=ctag_tpid) check untag rule only
			//if(check C also) c is outter and skip untag rule
			//else (c is outter) here and (untag) rule later
			if(RT_ACL_PATTERN_IS_STPID_EQ_CTPID){
				need_vlan_rule = 0;
				ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.tpid = FALSE;
				ca_cls_rule.key.l2.vlan_count = 0;
				ca_cls_rule.key_mask.l2_mask.vlan_count = TRUE;
			}
			else if(acl_filter->filter_fields & (RT_ACL_INGRESS_CTAGIF_BIT|RT_ACL_INGRESS_CTAG_VID_BIT|RT_ACL_INGRESS_CTAG_PRI_BIT|RT_ACL_INGRESS_CTAG_CFI_BIT)){
				need_vlan_rule = 0;
				outter_tpid = ca_cls_rule.key.l2.vlan_otag.vlan_min.tpid;
			}
		}else{	//ingress_ctagif == 0
			ca_cls_rule.key.l2.vlan_otag.vlan_min.tpid = RT_ACL_PATTERN_SVLAN_TPID;	//stag only
			//if(check S also) s is outter and skip untag rule
			//else (s is outter) here and (untag) rule later
			if(acl_filter->filter_fields & (RT_ACL_INGRESS_STAGIF_BIT|RT_ACL_INGRESS_STAG_VID_BIT|RT_ACL_INGRESS_STAG_PRI_BIT|RT_ACL_INGRESS_STAG_DEI_BIT)){
				need_vlan_rule = 0;
				outter_tpid = ca_cls_rule.key.l2.vlan_otag.vlan_min.tpid;
			}
		}
		ca_cls_rule.key.l2.vlan_otag.vlan_max = ca_cls_rule.key.l2.vlan_otag.vlan_min;
	}else{
		//S: outter
		//C: inner [if(check C only) + vlan count 2]
		//if(check C only): C will be outter with 1 vlan at _rtk_fc_aclHWEntry_vlan_readd
		if(acl_filter->filter_fields & (RT_ACL_INGRESS_STAGIF_BIT|RT_ACL_INGRESS_STAG_VID_BIT|RT_ACL_INGRESS_STAG_PRI_BIT|RT_ACL_INGRESS_STAG_DEI_BIT))
			outter_tpid = RT_ACL_PATTERN_SVLAN_TPID;
		if(acl_filter->filter_fields & (RT_ACL_INGRESS_CTAGIF_BIT|RT_ACL_INGRESS_CTAG_VID_BIT|RT_ACL_INGRESS_CTAG_PRI_BIT|RT_ACL_INGRESS_CTAG_CFI_BIT))
			inner_tpid = RT_ACL_PATTERN_CVLAN_TPID;
	}

	if(outter_tpid){
		ca_cls_rule.key_mask.l2 = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_otag = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.tpid = TRUE;
		ca_cls_rule.key.l2.vlan_otag.vlan_min.tpid = outter_tpid;
		if((outter_tpid == RT_ACL_PATTERN_SVLAN_TPID) && (acl_filter->filter_fields & (RT_ACL_INGRESS_STAG_VID_BIT|RT_ACL_INGRESS_STAG_PRI_BIT|RT_ACL_INGRESS_STAG_DEI_BIT))){
			if(acl_filter->filter_fields & RT_ACL_INGRESS_STAG_VID_BIT){
				ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.vid = TRUE;
				ca_cls_rule.key.l2.vlan_otag.vlan_min.vid = acl_filter->ingress_stag_vid;
			}
			if(acl_filter->filter_fields & RT_ACL_INGRESS_STAG_PRI_BIT){
				ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.pri = TRUE;
				ca_cls_rule.key.l2.vlan_otag.vlan_min.pri = acl_filter->ingress_stag_pri;
			}
			if(acl_filter->filter_fields & RT_ACL_INGRESS_STAG_DEI_BIT){
				ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.dei = TRUE;
				ca_cls_rule.key.l2.vlan_otag.vlan_min.dei = acl_filter->ingress_stag_dei;
			}
		}else{	//RT_ACL_PATTERN_CVLAN_TPID
			if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_VID_BIT){
				ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.vid = TRUE;
				ca_cls_rule.key.l2.vlan_otag.vlan_min.vid = acl_filter->ingress_ctag_vid;
			}
			if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_PRI_BIT){
				ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.pri = TRUE;
				ca_cls_rule.key.l2.vlan_otag.vlan_min.pri = acl_filter->ingress_ctag_pri;
			}
			if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_CFI_BIT){
				ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.dei = TRUE;
				ca_cls_rule.key.l2.vlan_otag.vlan_min.dei = acl_filter->ingress_ctag_cfi;
			}
		}
		ca_cls_rule.key.l2.vlan_otag.vlan_max = ca_cls_rule.key.l2.vlan_otag.vlan_min;
	}else if(inner_tpid){	//ACL check C only (outter_tpid = 0)
		need_vlan_rule = 2;
		ca_cls_rule.key_mask.l2 = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_count = TRUE;
		ca_cls_rule.key.l2.vlan_count = 2;
		//if(stag_tpid=ctag_tpid) check inner tag only
		//else check (outter tag) here and (inner tag) later
		if(RT_ACL_PATTERN_IS_STPID_EQ_CTPID){
			need_vlan_rule = 0;
		}
	}
	if(inner_tpid){
		ca_cls_rule.key_mask.l2 = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_itag = TRUE;
		ca_cls_rule.key_mask.l2_mask.vlan_itag_mask.tpid = TRUE;
		ca_cls_rule.key.l2.vlan_itag.vlan_min.tpid = inner_tpid;
		if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_VID_BIT){
			ca_cls_rule.key_mask.l2_mask.vlan_itag_mask.vid = TRUE;
			ca_cls_rule.key.l2.vlan_itag.vlan_min.vid = acl_filter->ingress_ctag_vid;
		}
		if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_PRI_BIT){
			ca_cls_rule.key_mask.l2_mask.vlan_itag_mask.pri = TRUE;
			ca_cls_rule.key.l2.vlan_itag.vlan_min.pri = acl_filter->ingress_ctag_pri;
		}
		if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_CFI_BIT){
			ca_cls_rule.key_mask.l2_mask.vlan_itag_mask.dei = TRUE;
			ca_cls_rule.key.l2.vlan_itag.vlan_min.dei = acl_filter->ingress_ctag_cfi;
		}
		ca_cls_rule.key.l2.vlan_itag.vlan_max = ca_cls_rule.key.l2.vlan_itag.vlan_min;
	}
#endif


	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_TAGIF_BIT )
	{
		//HW ACL not support "reverse" check
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;
		ca_cls_rule.key.ip.ip_version = 4;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_SIP_RANGE_BIT)
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_sa = TRUE;
		ca_cls_rule.key.ip.ip_sa.afi = CA_IPV4;
		ca_cls_rule.key.ip.ip_sa.addr_len = 32;
		ca_cls_rule.key.ip.ip_sa.ip_addr.addr[0] = acl_filter->ingress_src_ipv4_addr_start;	//acl_filter->ingress_src_ipv4_addr_end

		//8277A: HW ACL not support ip_sa_max, only support subnet
		if(acl_filter->ingress_src_ipv4_addr_start != acl_filter->ingress_src_ipv4_addr_end){
			ret = _rtk_fc_aclHWEntry_maskLength_get_by_ipv4_range(acl_filter->ingress_src_ipv4_addr_start,acl_filter->ingress_src_ipv4_addr_end,&unmask_length);
			if(ret == RT_ERR_RG_OK){
				ca_cls_rule.key.ip.ip_sa.addr_len -= unmask_length;
			}
#if !defined(CONFIG_FC_G3_G3LITE_SERIES)
			else{
				ca_cls_rule.key_mask.ip_mask.ip_sa_max = TRUE;
				ca_cls_rule.key.ip.ip_sa_max.afi = CA_IPV4;
				ca_cls_rule.key.ip.ip_sa_max.addr_len = 32;
				ca_cls_rule.key.ip.ip_sa_max.ip_addr.addr[0] = acl_filter->ingress_src_ipv4_addr_end;
			}
#endif
		}
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_DIP_RANGE_BIT)
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_da = TRUE;
		ca_cls_rule.key.ip.ip_da.afi = CA_IPV4;
		ca_cls_rule.key.ip.ip_da.addr_len = 32;
		ca_cls_rule.key.ip.ip_da.ip_addr.addr[0] = acl_filter->ingress_dest_ipv4_addr_start;	//acl_filter->ingress_dest_ipv4_addr_end

		//8277A: HW ACL not support ip_da_max, only support subnet
		if(acl_filter->ingress_dest_ipv4_addr_start != acl_filter->ingress_dest_ipv4_addr_end){
			ret = _rtk_fc_aclHWEntry_maskLength_get_by_ipv4_range(acl_filter->ingress_dest_ipv4_addr_start,acl_filter->ingress_dest_ipv4_addr_end,&unmask_length);
			if(ret == RT_ERR_RG_OK){
				ca_cls_rule.key.ip.ip_da.addr_len -= unmask_length;
			}
#if !defined(CONFIG_FC_G3_G3LITE_SERIES)
			else{
				ca_cls_rule.key_mask.ip_mask.ip_da_max = TRUE;
				ca_cls_rule.key.ip.ip_da_max.afi = CA_IPV4;
				ca_cls_rule.key.ip.ip_da_max.addr_len = 32;
				ca_cls_rule.key.ip.ip_da_max.ip_addr.addr[0] = acl_filter->ingress_dest_ipv4_addr_end;
			}
#endif
		}
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_DSCP_BIT)
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;
		ca_cls_rule.key.ip.ip_version = 4;
		ca_cls_rule.key_mask.ip_mask.dscp = TRUE;
		ca_cls_rule.key.ip.dscp = acl_filter->ingress_ipv4_dscp;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_TOS_BIT)
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;
		ca_cls_rule.key.ip.ip_version = 4;
		ca_cls_rule.key_mask.ip_mask.dscp = TRUE;
		ca_cls_rule.key.ip.dscp = (acl_filter->ingress_ipv4_tos >> 2);
		ca_cls_rule.key_mask.ip_mask.ecn = TRUE;
		ca_cls_rule.key.ip.ecn = (acl_filter->ingress_ipv4_tos & 0x3);
	}


	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_TAGIF_BIT)
	{
		//HW ACL not support "reverse" check
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;
		ca_cls_rule.key.ip.ip_version = 6;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_SIP_BIT )
	{
		//8277A: HW ACL not support "mask", but can support aligned mask
		//8277B: HW ACL support range, but API only support mask, so also only support aligned mask
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_sa = TRUE;
		ca_cls_rule.key.ip.ip_sa.afi = CA_IPV6;
		ca_cls_rule.key.ip.ip_sa.addr_len = 128;
		for(i = 0; i < 4; i++)	//v6SIP: [0]=[31:0], [1]=[63:32], [2]=[95:64], [3]=[127:96]
			ca_cls_rule.key.ip.ip_sa.ip_addr.addr[i] = (acl_filter->ingress_src_ipv6_addr[0+i*4]<<24)|(acl_filter->ingress_src_ipv6_addr[1+i*4]<<16)|(acl_filter->ingress_src_ipv6_addr[2+i*4]<<8)|(acl_filter->ingress_src_ipv6_addr[3+i*4]);
		
		ret = _rtk_fc_aclHWEntry_maskLength_get_by_mask(acl_filter->ingress_src_ipv6_addr_mask,IPV6_ADDR_LEN,&unmask_length);
		if(ret == RT_ERR_RG_OK)
			ca_cls_rule.key.ip.ip_sa.addr_len -= unmask_length;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_DIP_BIT )
	{
		//8277A: HW ACL not support "mask", but can support aligned mask
		//8277B: HW ACL support range, but API only support mask, so also only support aligned mask
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_da = TRUE;
		ca_cls_rule.key.ip.ip_da.afi = CA_IPV6;
		ca_cls_rule.key.ip.ip_da.addr_len = 128;
		for(i = 0; i < 4; i++)	//v6DIP: [0]=[31:0], [1]=[63:32], [2]=[95:64], [3]=[127:96]
			ca_cls_rule.key.ip.ip_da.ip_addr.addr[i] = (acl_filter->ingress_dest_ipv6_addr[0+i*4]<<24)|(acl_filter->ingress_dest_ipv6_addr[1+i*4]<<16)|(acl_filter->ingress_dest_ipv6_addr[2+i*4]<<8)|(acl_filter->ingress_dest_ipv6_addr[3+i*4]);
		
		ret = _rtk_fc_aclHWEntry_maskLength_get_by_mask(acl_filter->ingress_dest_ipv6_addr_mask,IPV6_ADDR_LEN,&unmask_length);
		if(ret == RT_ERR_RG_OK)
			ca_cls_rule.key.ip.ip_da.addr_len -= unmask_length;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_DSCP_BIT)
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;
		ca_cls_rule.key.ip.ip_version = 6;
		ca_cls_rule.key_mask.ip_mask.dscp = TRUE;
		ca_cls_rule.key.ip.dscp = acl_filter->ingress_ipv6_dscp;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_TC_BIT)
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;
		ca_cls_rule.key.ip.ip_version = 6;
		ca_cls_rule.key_mask.ip_mask.dscp = TRUE;
		ca_cls_rule.key.ip.dscp = (acl_filter->ingress_ipv6_tc >> 2);
		ca_cls_rule.key_mask.ip_mask.ecn = TRUE;
		ca_cls_rule.key.ip.ecn = (acl_filter->ingress_ipv6_tc & 0x3);
	}


	if(acl_filter->filter_fields & RT_ACL_INGRESS_L4_POROTCAL_VALUE_BIT)
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_protocol = TRUE;
		ca_cls_rule.key.ip.ip_protocol = acl_filter->ingress_l4_protocal;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_L4_TCP_BIT)
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_protocol = TRUE;
		ca_cls_rule.key.ip.ip_protocol = 0x6;	//TCP protocal value 6
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_L4_UDP_BIT)
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_protocol = TRUE;
		ca_cls_rule.key.ip.ip_protocol = 0x11;	//UDP protocal value 17
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_L4_ICMP_BIT)
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;
		ca_cls_rule.key.ip.ip_version = 4;
		ca_cls_rule.key_mask.ip_mask.ip_protocol = TRUE;
		ca_cls_rule.key.ip.ip_protocol = 0x1;	//ICMP protocal value 1
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_L4_ICMPV6_BIT )
	{
		ca_cls_rule.key_mask.ip = TRUE;
		ca_cls_rule.key_mask.ip_mask.ip_version = TRUE;
		ca_cls_rule.key.ip.ip_version = 6;
		ca_cls_rule.key_mask.ip_mask.ip_protocol = TRUE;
		ca_cls_rule.key.ip.ip_protocol = 0x3a;	//ICMPv6 protocal value 0x3a in next-header
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_L4_SPORT_RANGE_BIT)
	{
		ca_cls_rule.key_mask.l4 = TRUE;
		ca_cls_rule.key_mask.l4_mask.src_port = TRUE;
		ca_cls_rule.key.l4.src_port.min = acl_filter->ingress_src_l4_port_start;
		ca_cls_rule.key.l4.src_port.max = acl_filter->ingress_src_l4_port_end;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_L4_DPORT_RANGE_BIT)
	{
		ca_cls_rule.key_mask.l4 = TRUE;
		ca_cls_rule.key_mask.l4_mask.dst_port = TRUE;
		ca_cls_rule.key.l4.dst_port.min = acl_filter->ingress_dest_l4_port_start;
		ca_cls_rule.key.l4.dst_port.max = acl_filter->ingress_dest_l4_port_end;
	}


	if(acl_filter->filter_fields & RT_ACL_INGRESS_STREAM_ID_BIT)
	{
		//not support yet
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_TCP_FLAGS_BIT)
	{
		//8277A limitation: only support OR check, so display warning
		//8277B: (tcp_flags=0xfff or tcp_flags != tcp_flags_mask) AND mode, otherwise OR mode

#if defined(CONFIG_FC_G3_G3LITE_SERIES)
		//8277A support below case by CLS + HASH from _rtk_fc_aclHWEntry_hash_remapping
		// - Pattern can be: TCP SYN only, TCP SYN+ACK
		// - Action can be: policing, trap with priority
		if(ACL_CHECK_HASH_NEED(acl_filter))
			is_hash = TRUE;
#endif

		ca_cls_rule.key_mask.l4 = TRUE;
		ca_cls_rule.key.l4.tcp_flags = acl_filter->ingress_tcp_flags&0x1ff;
		ca_cls_rule.key_mask.l4_mask.tcp_flags = acl_filter->ingress_tcp_flags_mask&0x1ff;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_PKT_LEN_RANGE_BIT)
	{
		//8277A limitation: overlap packet length range cause not hit, so only support limit range at _rtk_fc_aclHWEntry_not_support
		ca_cls_rule.key_mask.packet_length = TRUE;
		ca_cls_rule.key.packet_length_low = acl_filter->ingress_packet_length_start;
		ca_cls_rule.key.packet_length_high = acl_filter->ingress_packet_length_end;
	}


	/*set actions*/
	for(i=0; i<RT_ACL_ACTION_FIELDS_INDEX_MAX; i++)
	{
		switch((acl_filter->action_fields)&(0x1<<i))
		{
			case RT_ACL_ACTION_FORWARD_GROUP_DROP_BIT:
				ca_cls_rule.key_mask.merge_prio = TRUE;
				ca_cls_rule.key.merge_prio = CL_RSLT_TYPE_1;
				ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_DENY;
				break;

			case RT_ACL_ACTION_FORWARD_GROUP_TRAP_BIT:
				ca_cls_rule.key_mask.merge_prio = TRUE;
				ca_cls_rule.key.merge_prio = CL_RSLT_TYPE_1;
				ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
				ca_cls_rule.action.dest.port = RTK_FC_MAC_PORT_CPU;
				ca_cls_rule.action.options.priority = 0;					//Default configure as 0 to prevent l2tp data trap with high priority
				ca_cls_rule.action.options.masks.priority = TRUE;			//RTK_RG_ACLANDCF_RESERVED_L2TP_DATA_LOWER_PRIORITY
				aal_customize |= CA_CLASSIFIER_AAL_L3_KEEP_ORIG;
				break;

			case RT_ACL_ACTION_FORWARD_GROUP_TRAP_TO_PS_BIT:
				ca_cls_rule.key_mask.merge_prio = TRUE;
				ca_cls_rule.key.merge_prio = CL_RSLT_TYPE_1;
				ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
				ca_cls_rule.action.dest.port = RTK_FC_MAC_PORT_CPU;
				ca_cls_rule.action.options.priority = 0;					//Default configure as 0 to prevent l2tp data trap with high priority
				ca_cls_rule.action.options.masks.priority = TRUE;			//RTK_RG_ACLANDCF_RESERVED_L2TP_DATA_LOWER_PRIORITY
				aal_customize |= CA_CLASSIFIER_AAL_L3_KEEP_ORIG;
				ca_cls_rule.action.options.sw_id[0] |= RXINFO_REF_TRAP_RSN_ACL_TO_PS << RXINFO_REF_TRAP_RSN_SHIFT_BIT;
				break;

			case RT_ACL_ACTION_FORWARD_GROUP_PERMIT_BIT:
				ca_cls_rule.key_mask.merge_prio = TRUE;
				ca_cls_rule.key.merge_prio = CL_RSLT_TYPE_1;
				ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_FE;
				ca_cls_rule.action.dest.fe = CA_CLASSIFIER_FORWARD_FE_L3FE;	//only add L3 CLS, so permit to L3FE
				break;

			case RT_ACL_ACTION_FORWARD_GROUP_REDIRECT_BIT:
				ca_cls_rule.key_mask.merge_prio = TRUE;
				ca_cls_rule.key.merge_prio = CL_RSLT_TYPE_1;
				ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
				ca_cls_rule.action.dest.port = acl_filter->action_forward_group_redirect_port_idx;
				break;

			case RT_ACL_ACTION_PRIORITY_GROUP_TRAP_PRIORITY_BIT:
				ca_cls_rule.key_mask.merge_prio = TRUE;
				ca_cls_rule.key.merge_prio = CL_RSLT_TYPE_1;
				ca_cls_rule.action.options.priority = acl_filter->action_priority_group_trap_priority;
				ca_cls_rule.action.options.masks.priority = TRUE;
				break;

			case RT_ACL_ACTION_PRIORITY_GROUP_ACL_PRIORITY_BIT:
				ca_cls_rule.key_mask.merge_prio = TRUE;
				ca_cls_rule.key.merge_prio = CL_RSLT_TYPE_1;
				ca_cls_rule.action.options.priority = acl_filter->action_priority_group_acl_priority;
				ca_cls_rule.action.options.masks.priority = TRUE;

				if(ca_cls_rule.action.options.priority == 7){
					aal_customize |= CA_CLASSIFIER_AAL_L3_NO_DROP;
				}

				if((acl_filter->action_fields & RT_ACL_ACTION_FORWARD_GROUP_BITS) == 0x0){
					ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_FE;
					ca_cls_rule.action.dest.fe = CA_CLASSIFIER_FORWARD_FE_L3FE;
				}
				break;

			case RT_ACL_ACTION_METER_GROUP_SHARE_METER_BIT:
				ca_cls_rule.key_mask.merge_prio = TRUE;
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
				ca_cls_rule.key.merge_prio = CL_RSLT_TYPE_1;	//8277A: different rules, meter & forward action according to acl weight due to limit resource
#else
				if(fc_db.controlFuc.acl_multiple_hit_cfg == 1)
					ca_cls_rule.key.merge_prio = CL_RSLT_TYPE_2;	//8277B: different rules, meter & forward action both work at the same time
				else
					ca_cls_rule.key.merge_prio = CL_RSLT_TYPE_1;	//disable multiple hit function
#endif

				if((acl_filter->action_fields & RT_ACL_ACTION_FORWARD_GROUP_TRAP_BIT) == 0x0){
					ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_FE;
					ca_cls_rule.action.dest.fe = CA_CLASSIFIER_FORWARD_FE_L3FE;
				}

				if(fc_db.controlFuc.rt_api_is_enabled){

					ret = RTK_FC_HELPER_RT_RATE_SHAREMETER_MAPPING_HW_GET(acl_filter->action_meter_group_share_meter_index, &meterMapping);
					if((ret == RT_ERR_RG_OK) && (meterMapping.type == RT_METER_TYPE_ACL))
					{
						aclSWEntry->hw_share_meter_index = meterMapping.hw_index;
#if defined(CONFIG_FC_CA8277B_SERIES)
						for(ret = 0; ret < RTK_FC_TABLESIZE_POLREMAP; ret++){
							if(!fc_db.hwPolRemap[ret].valid)
								break;	//speed up, due to valid start from 0~n
							if(fc_db.hwPolRemap[ret].pol_id == aclSWEntry->hw_share_meter_index){
								aclSWEntry->hw_share_meter_index = ret;
								aal_customize |= CA_CLASSIFIER_AAL_L3_POL_REMAP_GRP;
								ACL_CTRL("pol_id %d remap to pol_grp_id %d", aclSWEntry->hw_share_meter_index, ret);
								break;
							}
						}
#endif
					}
					else
					{
						WARNING("get ACL meter hw index Failed, return value: 0x%x, virtual meter type: %d", ret, meterMapping.type);
						return (RT_ERR_RG_INVALID_PARAM);
					}
				}else
					aclSWEntry->hw_share_meter_index = acl_filter->action_meter_group_share_meter_index;


				ca_cls_rule.action.options.action_handle.flow_id = aclSWEntry->hw_share_meter_index;
				ca_cls_rule.action.options.masks.action_handle = TRUE;
				break;

			default:
				break;
		}
	}


	/*add ACL rule*/
	ca_index_offset = 0;
	CA_SCAN_PORTMASK(i, activePorts.bits[0])
	{
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
		if(is_hash){
			ret = _rtk_fc_aclHWEntry_hash_remapping(i, acl_filter, aclSWEntry);
			if(ret){
				ACL_CTRL("add user ACL failed (ret=0x%x)!!!", ret);
				return(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
			}
			ca_index_offset += snprintf(ca_index_buffer+ca_index_offset, sizeof(ca_index_buffer)-ca_index_offset, "hash-%d,", aclSWEntry->hw_hash_index-1);
		}
		else
#else
		if(need_vlan_rule){	//add one more rule
			ret = _rtk_fc_aclHWEntry_vlan_readd(need_vlan_rule, &ca_cls_rule, i, acl_filter, index, aal_customize);	//if update below algorithm, should also update this api.
			if(ret == CA_E_OK){
				aclCountinueRuleSize++;
				ret = ca_classifier_l3_cls_rule_idx_get(G3_DEF_DEVID, ca_cls_rule.index, &ca_index_hw_info);
				if(ret == CA_E_OK)
					ca_index_offset += snprintf(ca_index_buffer+ca_index_offset, sizeof(ca_index_buffer)-ca_index_offset, "%d-%d,", CLS_TBL_ID(ca_index_hw_info), CLS_KEY_ID(ca_index_hw_info));
				else
					ca_index_offset += snprintf(ca_index_buffer+ca_index_offset, sizeof(ca_index_buffer)-ca_index_offset, "sw-%d,", ca_cls_rule.index);
				ca_cls_rule.index = 0;
			}else{
				return ret;
			}
		}
#endif
		{
			ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_L3, i);
			ca_cls_rule.key_mask.src_port= 1;

			ret = _rtk_fc_g3_caClsRuleAALAdd(&ca_cls_rule, index, aal_customize);
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
				ca_index_offset += snprintf(ca_index_buffer+ca_index_offset, sizeof(ca_index_buffer)-ca_index_offset, "%d-%d,", CLS_TBL_ID(ca_index_hw_info), CLS_KEY_ID(ca_index_hw_info));
			else
				ca_index_offset += snprintf(ca_index_buffer+ca_index_offset, sizeof(ca_index_buffer)-ca_index_offset, "sw-%d,", ca_cls_rule.index);
		}
	}
	aclSWEntry->hw_aclEntry_count = aclCountinueRuleSize;
	aclSWEntry->hw_aclEntry_port = caCLSPort;
	memcpy(aclSWEntry->hw_aclEntry_index, ca_index_buffer, sizeof(aclSWEntry->hw_aclEntry_index));

	ACL_CTRL("using HW_ACL hw_aclEntry_index is %s (L3 CLS port = 0x%x, total %d rules)",aclSWEntry->hw_aclEntry_index,aclSWEntry->hw_aclEntry_port.bits[0],aclSWEntry->hw_aclEntry_count);

	return (RT_ERR_RG_OK);
}
#else
static int _rtk_fc_aclSWEntry_to_asic_convert(rt_acl_filterAndQos_t *acl_filter,rtk_fc_aclFilterEntry_t *aclSWEntry,l3_cls_rule_t *aclHWEntry, uint8 *need_more_rule)
{
	int i, ret;
	uint8 tpid_idx;
	uint8 mac_min[ETHER_ADDR_LEN];
	uint8 mac_max[ETHER_ADDR_LEN];
	uint32 address[4];
	uint16 outter_tpid, inner_tpid;
	int unmask_length, mask_length;
	rt_rate_meter_mapping_t meterMapping;
	cls_key_t *aal_key = &aclHWEntry->key;
	cls_action_t *aal_action = &aclHWEntry->action;

	bzero(aclHWEntry,sizeof(l3_cls_rule_t));
	aal_l3_cls_key_clean(aal_key);

	aclHWEntry->pri = RTK_CA_CLS_PRIORITY_L3_USER;
	aclHWEntry->rslt_type = CL_RSLT_TYPE_1;

	if(acl_filter->filter_fields & RT_ACL_INGRESS_SMAC_BIT)
	{
		for(i=0; i<ETHER_ADDR_LEN; i++)
			mac_min[i] = (acl_filter->ingress_smac[i]&acl_filter->ingress_smac_mask[i]);
		for(i=0; i<ETHER_ADDR_LEN; i++)
			mac_max[i] = (acl_filter->ingress_smac[i]&acl_filter->ingress_smac_mask[i]) + ~(acl_filter->ingress_smac_mask[i]);
		if(memcmp(&mac_min[0], &mac_max[0], ETHER_ADDR_LEN)) {	//mac range
			for(i=0; i<ETHER_ADDR_LEN; i++)
				aal_key->mac_da[i] = mac_min[5-i];
			for(i=0; i<ETHER_ADDR_LEN; i++)
				aal_key->mac_sa[i] = mac_max[5-i];
			aal_key->mac_addr_ctrl = 1;
		}else{
			for(i=0; i<ETHER_ADDR_LEN; i++)
				aal_key->mac_sa[i] = mac_min[5-i];
			aal_key->msk_mac_sa = CA_L3_AAL_KEY_MSK_ENABLE;
			aal_key->mac_addr_ctrl = 5;
		}
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_DMAC_BIT)
	{
		//HW ACL not support "mask", acl_filter->ingress_dmac_mask
		if(PATTERN_CHECK_DMAC_IS_MC(acl_filter)){	//DA[40] = 1b1
			aal_key->mac_da_mc = TRUE;
			aal_key->msk_mac_da_mc = CA_L3_AAL_KEY_MSK_ENABLE;
		}else if(PATTERN_CHECK_DMAC_IS_MC_IPV4(acl_filter)){
			//((MAC DA[47:24] == h01-00-5E) && (etherType == IPV4)) |((MAC DA[47:32] == h33-33) && (etherType == IPV6))
			aal_key->ip_ver = 0;	//ipv4
			aal_key->msk_ip_ver= CA_L3_AAL_KEY_MSK_ENABLE;
			aal_key->mac_da_ip_mc = 1;
			aal_key->msk_mac_da_ip_mc = CA_L3_AAL_KEY_MSK_ENABLE;
		}else if(PATTERN_CHECK_DMAC_IS_MC_IPV6(acl_filter)){
			//((MAC DA[47:24] == h01-00-5E) && (etherType == IPV4)) |((MAC DA[47:32] == h33-33) && (etherType == IPV6))
			aal_key->ip_ver = 1;	//ipv6
			aal_key->msk_ip_ver= CA_L3_AAL_KEY_MSK_ENABLE;
			aal_key->mac_da_ip_mc = 1;
			aal_key->msk_mac_da_ip_mc = CA_L3_AAL_KEY_MSK_ENABLE;
		}else if(PATTERN_CHECK_DMAC_IS_UC(acl_filter)){ //DA[40] = 0, DA_mask[40] = 1b1
			aal_key->mac_da_mc = FALSE;
			aal_key->msk_mac_da_mc = CA_L3_AAL_KEY_MSK_ENABLE;
			aal_key->mac_da_bc = FALSE; //Set if the MAC DA is broadcast address (48hffff_ffff_ffff)
			aal_key->msk_mac_da_bc = CA_L3_AAL_KEY_MSK_ENABLE;
		}else if(PATTERN_CHECK_DMAC_IS_RSVD(acl_filter)){	//DA[47:8] = 40h01_80_C2_00_00)
			aal_key->mac_da_rsvd = TRUE;	//Set if the MAC DA[47:8] = 40h01_80_C2_00_00
			aal_key->msk_mac_da_rsvd = CA_L3_AAL_KEY_MSK_ENABLE;
		}else{
			for(i=0; i<ETHER_ADDR_LEN; i++)
				mac_min[i] = (acl_filter->ingress_dmac[i]&acl_filter->ingress_dmac_mask[i]);
			for(i=0; i<ETHER_ADDR_LEN; i++)
				mac_max[i] = (acl_filter->ingress_dmac[i]&acl_filter->ingress_dmac_mask[i]) + ~(acl_filter->ingress_dmac_mask[i]);
			if(memcmp(&mac_min[0], &mac_max[0], ETHER_ADDR_LEN)) {	//mac range
				if (aal_key->mac_addr_ctrl == 1 || aal_key->mac_addr_ctrl == 5){
					WARNING("not support smac and dmac range at the same time!!!");
					return (RT_ERR_RG_CHIP_NOT_SUPPORT);
				}
				for(i=0; i<ETHER_ADDR_LEN; i++)
					aal_key->mac_da[i] = mac_min[5-i];
				for(i=0; i<ETHER_ADDR_LEN; i++)
					aal_key->mac_sa[i] = mac_max[5-i];
				aal_key->mac_addr_ctrl = 2;
			}else{
				if (aal_key->mac_addr_ctrl == 1){
					WARNING("not support smac range and dmac at the same time!!!");
					return (RT_ERR_RG_CHIP_NOT_SUPPORT);
				}
				for(i=0; i<ETHER_ADDR_LEN; i++)
					aal_key->mac_da[i] = mac_min[5-i];
				aal_key->msk_mac_da = CA_L3_AAL_KEY_MSK_ENABLE;
				aal_key->mac_addr_ctrl = (aal_key->mac_addr_ctrl == 5) ? 4 : 6;
				for(i=0; i<ETHER_ADDR_LEN; i++)
					aal_key->dest_addr.dest_mac_addr[i] = mac_min[5-i];
				aal_key->dest_addr_type = 0;
				aal_key->msk_dest_addr = CA_L3_AAL_KEY_MSK_ENABLE;
			}
		}
	}


	if(acl_filter->filter_fields & RT_ACL_INGRESS_ETHERTYPE_BIT)
	{
		//update ethertype_enc in _rtk_fc_aclSWEntry_to_asic_cam
		aal_key->msk_ethertype_enc = CA_L3_AAL_KEY_MSK_ENABLE;

		//Bug fix CA HW fail to keep original for ethertype=0x8864
		if(acl_filter->ingress_ethertype == 0x8864){
			aal_action->keep_orig_pkt_vld = 1;
			aal_action->keep_orig_pkt = 1;
		}
	}


	if(*need_more_rule == 0){
		outter_tpid = 0;
		inner_tpid = 0;
		if((acl_filter->filter_fields & RT_ACL_INGRESS_STAGIF_BIT) && (acl_filter->ingress_stagif == 0) &&
			(acl_filter->filter_fields & RT_ACL_INGRESS_CTAGIF_BIT) && (acl_filter->ingress_ctagif == 0))
		{	//untag
			aal_key->vlan_cnt = 0;
			aal_key->msk_vlan_cnt = CA_L3_AAL_KEY_MSK_ENABLE;
		}else if(((acl_filter->filter_fields & RT_ACL_INGRESS_STAGIF_BIT) && (acl_filter->ingress_stagif == 0)) ||
				((acl_filter->filter_fields & RT_ACL_INGRESS_CTAGIF_BIT) && (acl_filter->ingress_ctagif == 0))){
			//vlan count = 1 here, and untag at case (*need_more_rule == 1)
			*need_more_rule = 1;
			aal_key->vlan_cnt = 1;
			aal_key->msk_vlan_cnt = CA_L3_AAL_KEY_MSK_ENABLE;
			if((acl_filter->filter_fields & RT_ACL_INGRESS_STAGIF_BIT) && (acl_filter->ingress_stagif == 0)){
				if (AAL_E_OK != aal_l3fe_pp_top_tpid_get(RT_ACL_PATTERN_CVLAN_TPID, &tpid_idx)){
					WARNING("fail to support cvlan tpid 0x%x due to Packet Parser not support!!!", RT_ACL_PATTERN_CVLAN_TPID);
					return (RT_ERR_RG_CHIP_NOT_SUPPORT);
				}
				aal_key->top_tpid_enc = tpid_idx;
				aal_key->msk_top_tpid_enc = CA_L3_AAL_KEY_MSK_ENABLE;
				//if(stag_tpid=ctag_tpid & check C also) _rtk_fc_aclHWEntry_not_support
				//if(stag_tpid=ctag_tpid) check untag rule only
				//if(check C also) c is outter and skip untag rule
				//else (c is outter) here and (untag) rule later
				if(RT_ACL_PATTERN_IS_STPID_EQ_CTPID){
					*need_more_rule = 0;
					aal_key->msk_top_tpid_enc = CA_L3_AAL_KEY_MSK_DISABLE;
					aal_key->vlan_cnt = 0;
					aal_key->msk_vlan_cnt = CA_L3_AAL_KEY_MSK_ENABLE;
				}
				else if(acl_filter->filter_fields & (RT_ACL_INGRESS_CTAGIF_BIT|RT_ACL_INGRESS_CTAG_VID_BIT|RT_ACL_INGRESS_CTAG_PRI_BIT|RT_ACL_INGRESS_CTAG_CFI_BIT)){
					*need_more_rule = 0;
					outter_tpid = RT_ACL_PATTERN_CVLAN_TPID;
				}
			}else{	//ingress_ctagif == 0
				if (AAL_E_OK != aal_l3fe_pp_top_tpid_get(RT_ACL_PATTERN_SVLAN_TPID, &tpid_idx)){
					WARNING("fail to support svlan tpid 0x%x due to Packet Parser not support!!!", RT_ACL_PATTERN_SVLAN_TPID);
					return (RT_ERR_RG_CHIP_NOT_SUPPORT);
				}
				aal_key->top_tpid_enc = tpid_idx;
				aal_key->msk_top_tpid_enc = CA_L3_AAL_KEY_MSK_ENABLE;
				//if(check S also) s is outter and skip untag rule
				//else (s is outter) here and (untag) rule later
				if(acl_filter->filter_fields & (RT_ACL_INGRESS_STAGIF_BIT|RT_ACL_INGRESS_STAG_VID_BIT|RT_ACL_INGRESS_STAG_PRI_BIT|RT_ACL_INGRESS_STAG_DEI_BIT)){
					*need_more_rule = 0;
					outter_tpid = RT_ACL_PATTERN_SVLAN_TPID;
				}
			}
		}else{
			//S: outter
			//C: inner [if(check C only) + vlan count 2]
			//if(check C only): C will be outter with 1 vlan at _rtk_fc_acl_add_acl_rule
			if(acl_filter->filter_fields & (RT_ACL_INGRESS_STAGIF_BIT|RT_ACL_INGRESS_STAG_VID_BIT|RT_ACL_INGRESS_STAG_PRI_BIT|RT_ACL_INGRESS_STAG_DEI_BIT))
				outter_tpid = RT_ACL_PATTERN_SVLAN_TPID;
			if(acl_filter->filter_fields & (RT_ACL_INGRESS_CTAGIF_BIT|RT_ACL_INGRESS_CTAG_VID_BIT|RT_ACL_INGRESS_CTAG_PRI_BIT|RT_ACL_INGRESS_CTAG_CFI_BIT))
				inner_tpid = RT_ACL_PATTERN_CVLAN_TPID;
		}

		if(outter_tpid){
			if (AAL_E_OK != aal_l3fe_pp_top_tpid_get(outter_tpid, &tpid_idx)){
				WARNING("fail to support top vlan tpid 0x%x due to Packet Parser not support!!!", outter_tpid);
				return (RT_ERR_RG_CHIP_NOT_SUPPORT);
			}
			aal_key->top_tpid_enc = tpid_idx;
			aal_key->msk_top_tpid_enc = CA_L3_AAL_KEY_MSK_ENABLE;
			if((outter_tpid == RT_ACL_PATTERN_SVLAN_TPID) && (acl_filter->filter_fields & (RT_ACL_INGRESS_STAG_VID_BIT|RT_ACL_INGRESS_STAG_PRI_BIT|RT_ACL_INGRESS_STAG_DEI_BIT))){
				if(acl_filter->filter_fields & RT_ACL_INGRESS_STAG_VID_BIT){
					aal_key->top_vid = acl_filter->ingress_stag_vid;
					if (inner_tpid && ((acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_CFI_BIT) || (acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_PRI_BIT)))
						aal_key->vid_ctl = 0;
					else
						aal_key->msk_top_vid = CA_L3_AAL_KEY_MSK_ENABLE;
				}
				if(acl_filter->filter_fields & RT_ACL_INGRESS_STAG_PRI_BIT){
					aal_key->top_802_1p = acl_filter->ingress_stag_pri;
					aal_key->msk_top_vl_802_1p = CA_L3_AAL_KEY_MSK_ENABLE;
				}
				if(acl_filter->filter_fields & RT_ACL_INGRESS_STAG_DEI_BIT){
					aal_key->top_dei= acl_filter->ingress_stag_dei;
					aal_key->msk_top_dei = CA_L3_AAL_KEY_MSK_ENABLE;
				}
			}else{	//RT_ACL_PATTERN_CVLAN_TPID
				if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_VID_BIT){
					aal_key->top_vid = acl_filter->ingress_ctag_vid;
					if (inner_tpid && ((acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_CFI_BIT) || (acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_PRI_BIT)))
						aal_key->vid_ctl = 0;
					else
						aal_key->msk_top_vid = CA_L3_AAL_KEY_MSK_ENABLE;
				}
				if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_PRI_BIT){
					aal_key->top_802_1p = acl_filter->ingress_ctag_pri;
					aal_key->msk_top_vl_802_1p = CA_L3_AAL_KEY_MSK_ENABLE;
				}
				if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_CFI_BIT){
					aal_key->top_dei= acl_filter->ingress_ctag_cfi;
					aal_key->msk_top_dei = CA_L3_AAL_KEY_MSK_ENABLE;
				}
			}
		}else if(inner_tpid){	//ACL check C only (outter_tpid = 0) in case (*need_more_rule == 2)
			*need_more_rule = 2;
			aal_key->vlan_cnt = 2;
			aal_key->msk_vlan_cnt = CA_L3_AAL_KEY_MSK_ENABLE;
			//if(stag_tpid=ctag_tpid) check inner tag only
			//else check (outter tag) here and (inner tag) later
			if(RT_ACL_PATTERN_IS_STPID_EQ_CTPID){
				*need_more_rule = 0;
			}
		}
		if(inner_tpid){
			if (AAL_E_OK != aal_l3fe_pp_inner_tpid_get(inner_tpid, &tpid_idx)){
				WARNING("fail to support inner vlan tpid 0x%x due to Packet Parser not support!!!", inner_tpid);
				return (RT_ERR_RG_CHIP_NOT_SUPPORT);
			}
			aal_key->inner_tpid_enc = tpid_idx;
			aal_key->msk_inner_tpid_enc = CA_L3_AAL_KEY_MSK_ENABLE;
			if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_VID_BIT){
				aal_key->inner_vid = acl_filter->ingress_ctag_vid;
				aal_key->msk_inner_vid = CA_L3_AAL_KEY_MSK_ENABLE;
			}
			if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_PRI_BIT){
				aal_key->inner_802_1p = acl_filter->ingress_ctag_pri;
				aal_key->msk_inner_802_1p = CA_L3_AAL_KEY_MSK_ENABLE;
			}
			if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_CFI_BIT){
				aal_key->inner_dei= acl_filter->ingress_ctag_cfi;
				aal_key->msk_inner_dei = CA_L3_AAL_KEY_MSK_ENABLE;
			}
		}
	}
	else if(*need_more_rule == 1)
	{	//ACL check untag at here (ref: _rtk_fc_aclHWEntry_vlan_readd case 1 in 77B)
		ACL_CTRL("add one more ACL for untag!!!");
		*need_more_rule = 0;
		aal_key->vlan_cnt = 0;
		aal_key->msk_vlan_cnt = CA_L3_AAL_KEY_MSK_ENABLE;
	}
	else if(*need_more_rule == 2)
	{	//ACL check C will be outter with 1 vlan only at here (ref: _rtk_fc_aclHWEntry_vlan_readd case 2 in 77B)
		ACL_CTRL("add one more ACL for outter CVLAN!!!");
		*need_more_rule = 0;
		aal_key->vlan_cnt = 1;
		aal_key->msk_vlan_cnt = CA_L3_AAL_KEY_MSK_ENABLE;
		if (AAL_E_OK != aal_l3fe_pp_top_tpid_get(RT_ACL_PATTERN_CVLAN_TPID, &tpid_idx)){
			WARNING("fail to support outter cvlan tpid 0x%x due to Packet Parser not support!!!", RT_ACL_PATTERN_CVLAN_TPID);
			return (RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
		aal_key->top_tpid_enc = tpid_idx;
		aal_key->msk_top_tpid_enc = CA_L3_AAL_KEY_MSK_ENABLE;
		if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_VID_BIT){
			aal_key->top_vid = acl_filter->ingress_ctag_vid;
			aal_key->msk_top_vid = CA_L3_AAL_KEY_MSK_ENABLE;
		}
		if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_PRI_BIT){
			aal_key->top_802_1p = acl_filter->ingress_ctag_pri;
			aal_key->msk_top_vl_802_1p = CA_L3_AAL_KEY_MSK_ENABLE;
		}
		if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_CFI_BIT){
			aal_key->top_dei= acl_filter->ingress_ctag_cfi;
			aal_key->msk_top_dei = CA_L3_AAL_KEY_MSK_ENABLE;
		}
	}else{
		WARNING("should not here, need_more_rule=%d!!!", *need_more_rule);
	}


	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_TAGIF_BIT )
	{
		//HW ACL not support "reverse" check
		aal_key->ip_ver = 0;	//ipv4
		aal_key->msk_ip_ver= CA_L3_AAL_KEY_MSK_ENABLE;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_SIP_RANGE_BIT)
	{
		// set ip_addr to select a right key type
		aal_key->ip_addr.ip_addr_0 = acl_filter->ingress_src_ipv4_addr_start;
		aal_key->ip_addr.ip_addr_1 = aal_key->ip_addr.ip_addr_2 = aal_key->ip_addr.ip_addr_3 = 0;
		aal_key->msk_ip_addr_l = 32;
		aal_key->ip_addr_type = 1;
		// set ip_sa to select a right key type
		aal_key->ip_sa.ip_addr_0 = acl_filter->ingress_src_ipv4_addr_start;
		aal_key->ip_sa.ip_addr_1 = aal_key->ip_sa.ip_addr_2 = aal_key->ip_sa.ip_addr_3 = 0;
		aal_key->msk_ip_sa_l = 32;
		aal_key->ip_addr_ctrl = 0;

		if(acl_filter->ingress_src_ipv4_addr_start != acl_filter->ingress_src_ipv4_addr_end){
			ret = _rtk_fc_aclHWEntry_maskLength_get_by_ipv4_range(acl_filter->ingress_src_ipv4_addr_start,acl_filter->ingress_src_ipv4_addr_end,&unmask_length);
			if(ret == RT_ERR_RG_OK){
				aal_key->msk_ip_addr_l -= unmask_length;
				aal_key->msk_ip_sa_l -= unmask_length;
			}
			else{
				// min sa ip
				aal_key->ip_da.ip_addr_0 = acl_filter->ingress_src_ipv4_addr_start;
				aal_key->ip_da.ip_addr_1 = aal_key->ip_da.ip_addr_2 = aal_key->ip_da.ip_addr_3 = 0;
				// max sa ip
				aal_key->ip_sa.ip_addr_0 = acl_filter->ingress_src_ipv4_addr_end;
				aal_key->ip_sa.ip_addr_1 = aal_key->ip_sa.ip_addr_2 = aal_key->ip_sa.ip_addr_3 = 0;
				aal_key->ip_addr_ctrl = 2;
			}
		}
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_DIP_RANGE_BIT)
	{
		// set ip_addr to select a right key type
		aal_key->ip_addr.ip_addr_0 = acl_filter->ingress_dest_ipv4_addr_start;
		aal_key->ip_addr.ip_addr_1 = aal_key->ip_addr.ip_addr_2 = aal_key->ip_addr.ip_addr_3 = 0;
		aal_key->msk_ip_addr_l = 32;
		aal_key->ip_addr_type = 0;
		// set ip_da to select a right key type
		aal_key->ip_da.ip_addr_0 = acl_filter->ingress_dest_ipv4_addr_start;
		aal_key->ip_da.ip_addr_1 = aal_key->ip_da.ip_addr_2 = aal_key->ip_da.ip_addr_3 = 0;
		aal_key->msk_ip_da_l = 32;
		aal_key->ip_addr_ctrl = 0;

		if(acl_filter->ingress_dest_ipv4_addr_start != acl_filter->ingress_dest_ipv4_addr_end){
			ret = _rtk_fc_aclHWEntry_maskLength_get_by_ipv4_range(acl_filter->ingress_dest_ipv4_addr_start,acl_filter->ingress_dest_ipv4_addr_end,&unmask_length);
			if(ret == RT_ERR_RG_OK){
				aal_key->msk_ip_addr_l -= unmask_length;
				aal_key->msk_ip_da_l -= unmask_length;
			}
			else{
				// min da ip
				aal_key->ip_da.ip_addr_0 = acl_filter->ingress_dest_ipv4_addr_start;
				aal_key->ip_da.ip_addr_1 = aal_key->ip_da.ip_addr_2 = aal_key->ip_da.ip_addr_3 = 0;
				// max da ip
				aal_key->ip_sa.ip_addr_0 = acl_filter->ingress_dest_ipv4_addr_end;
				aal_key->ip_sa.ip_addr_1 = aal_key->ip_sa.ip_addr_2 = aal_key->ip_sa.ip_addr_3 = 0;
				aal_key->ip_addr_ctrl = 1;
			}
		}

		// set dest_addr to select a right key type
		if (aal_key->msk_ip_da_l == 32) {
			aal_key->dest_addr.dest_ip_addr.ip_addr_0 = acl_filter->ingress_dest_ipv4_addr_start;
			aal_key->dest_addr.dest_ip_addr.ip_addr_1 = 0;
			aal_key->dest_addr.dest_ip_addr.ip_addr_2 = 0;
			aal_key->dest_addr.dest_ip_addr.ip_addr_3 = 0;
			aal_key->msk_dest_addr = CA_L3_AAL_KEY_MSK_ENABLE;
			aal_key->dest_addr_type = 1;
		}
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_DSCP_BIT)
	{
		aal_key->ip_ver = 0;	//ipv4
		aal_key->msk_ip_ver= CA_L3_AAL_KEY_MSK_ENABLE;
		aal_key->ip_dscp = acl_filter->ingress_ipv4_dscp;
		aal_key->msk_dscp = CA_L3_AAL_KEY_MSK_ENABLE;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV4_TOS_BIT)
	{
		aal_key->ip_ver = 0;	//ipv4
		aal_key->msk_ip_ver= CA_L3_AAL_KEY_MSK_ENABLE;
		aal_key->ip_dscp = (acl_filter->ingress_ipv4_tos >> 2);
		aal_key->msk_dscp = CA_L3_AAL_KEY_MSK_ENABLE;
		aal_key->ip_ecn = (acl_filter->ingress_ipv4_tos & 0x3);
		aal_key->msk_ip_ecn = CA_L3_AAL_KEY_MSK_ENABLE;
	}


	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_TAGIF_BIT)
	{
		//HW ACL not support "reverse" check
		aal_key->ip_ver = 1;	//ipv6
		aal_key->msk_ip_ver= CA_L3_AAL_KEY_MSK_ENABLE;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_SIP_BIT )
	{
		//HW ACL support range, but API only support mask, so also only support aligned mask
		for(i = 0; i < 4; i++)	//v6SIP: [0]=[31:0], [1]=[63:32], [2]=[95:64], [3]=[127:96]
			address[i] = (acl_filter->ingress_src_ipv6_addr[0+i*4]<<24)|(acl_filter->ingress_src_ipv6_addr[1+i*4]<<16)|(acl_filter->ingress_src_ipv6_addr[2+i*4]<<8)|(acl_filter->ingress_src_ipv6_addr[3+i*4]);
		mask_length = 128;
		ret = _rtk_fc_aclHWEntry_maskLength_get_by_mask(acl_filter->ingress_src_ipv6_addr_mask,IPV6_ADDR_LEN,&unmask_length);
		if(ret == RT_ERR_RG_OK)
			mask_length -= unmask_length;

		// set ip_addr to select a right key type
		aal_key->ip_addr.ip_addr_0 = address[3];
		aal_key->ip_addr.ip_addr_1 = address[2];
		aal_key->ip_addr.ip_addr_2 = address[1];
		aal_key->ip_addr.ip_addr_3 = address[0];
		aal_key->msk_ip_addr_l = mask_length & 0x7f;
		aal_key->msk_ip_addr_h = mask_length >> 7;
		aal_key->ip_addr_type = 1;
		// set ip_sa to select a right key type
		aal_key->ip_sa.ip_addr_0 = address[3];
		aal_key->ip_sa.ip_addr_1 = address[2];
		aal_key->ip_sa.ip_addr_2 = address[1];
		aal_key->ip_sa.ip_addr_3 = address[0];
		aal_key->msk_ip_sa_l = mask_length & 0x7f;
		aal_key->msk_ip_sa_h = mask_length >> 7;
		aal_key->ip_addr_ctrl = 0;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_DIP_BIT )
	{
		//HW ACL support range, but API only support mask, so also only support aligned mask
		for(i = 0; i < 4; i++)	//v6DIP: [0]=[31:0], [1]=[63:32], [2]=[95:64], [3]=[127:96]
			address[i] = (acl_filter->ingress_dest_ipv6_addr[0+i*4]<<24)|(acl_filter->ingress_dest_ipv6_addr[1+i*4]<<16)|(acl_filter->ingress_dest_ipv6_addr[2+i*4]<<8)|(acl_filter->ingress_dest_ipv6_addr[3+i*4]);
		mask_length = 128;
		ret = _rtk_fc_aclHWEntry_maskLength_get_by_mask(acl_filter->ingress_dest_ipv6_addr_mask,IPV6_ADDR_LEN,&unmask_length);
		if(ret == RT_ERR_RG_OK)
			mask_length -= unmask_length;

		/* set ip_addr to select a right key type */
		aal_key->ip_addr.ip_addr_0 = address[3];
		aal_key->ip_addr.ip_addr_1 = address[2];
		aal_key->ip_addr.ip_addr_2 = address[1];
		aal_key->ip_addr.ip_addr_3 = address[0];
		aal_key->msk_ip_addr_l = mask_length & 0x7f;
		aal_key->msk_ip_addr_h = mask_length >> 7;
		aal_key->ip_addr_type = 0;
		/* set ip_da to select a right key type */
		aal_key->ip_da.ip_addr_0 = address[3];
		aal_key->ip_da.ip_addr_1 = address[2];
		aal_key->ip_da.ip_addr_2 = address[1];
		aal_key->ip_da.ip_addr_3 = address[0];
		aal_key->msk_ip_da_l = mask_length & 0x7f;
		aal_key->msk_ip_da_h = mask_length >> 7;
		/* set dest_addr to select a right key type */
		if (mask_length == 128) {
			aal_key->dest_addr.dest_ip_addr.ip_addr_0 = address[3];
			aal_key->dest_addr.dest_ip_addr.ip_addr_1 = address[2];
			aal_key->dest_addr.dest_ip_addr.ip_addr_2 = address[1];
			aal_key->dest_addr.dest_ip_addr.ip_addr_3 = address[0];
			aal_key->msk_dest_addr = CA_L3_AAL_KEY_MSK_ENABLE;
			aal_key->dest_addr_type = 1;
		}
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_DSCP_BIT)
	{
		aal_key->ip_ver = 1;	//ipv6
		aal_key->msk_ip_ver= CA_L3_AAL_KEY_MSK_ENABLE;
		aal_key->ip_dscp = acl_filter->ingress_ipv6_dscp;
		aal_key->msk_dscp = CA_L3_AAL_KEY_MSK_ENABLE;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_IPV6_TC_BIT)
	{
		aal_key->ip_ver = 1;	//ipv6
		aal_key->msk_ip_ver= CA_L3_AAL_KEY_MSK_ENABLE;
		aal_key->ip_dscp = (acl_filter->ingress_ipv6_tc >> 2);
		aal_key->msk_dscp = CA_L3_AAL_KEY_MSK_ENABLE;
		aal_key->ip_ecn = (acl_filter->ingress_ipv6_tc & 0x3);
		aal_key->msk_ip_ecn = CA_L3_AAL_KEY_MSK_ENABLE;
	}


	if(acl_filter->filter_fields & RT_ACL_INGRESS_L4_POROTCAL_VALUE_BIT)
	{
		aal_key->ip_protocol = acl_filter->ingress_l4_protocal;
		aal_key->msk_ip_protocol = CA_L3_AAL_KEY_MSK_ENABLE;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_L4_TCP_BIT)
	{
		aal_key->ip_protocol = 0x6;	//TCP protocal value 6
		aal_key->msk_ip_protocol = CA_L3_AAL_KEY_MSK_ENABLE;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_L4_UDP_BIT)
	{
		aal_key->ip_protocol = 0x11;	//UDP protocal value 17
		aal_key->msk_ip_protocol = CA_L3_AAL_KEY_MSK_ENABLE;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_L4_ICMP_BIT)
	{
		aal_key->ip_ver = 0;	//ipv4
		aal_key->msk_ip_ver= CA_L3_AAL_KEY_MSK_ENABLE;
		aal_key->ip_protocol = 0x1;	//ICMP protocal value 1
		aal_key->msk_ip_protocol = CA_L3_AAL_KEY_MSK_ENABLE;
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_L4_ICMPV6_BIT )
	{
		aal_key->ip_ver = 1;	//ipv6
		aal_key->msk_ip_ver= CA_L3_AAL_KEY_MSK_ENABLE;
		aal_key->ip_protocol = 0x3a;	//ICMPv6 protocal value 0x3a in next-header
		aal_key->msk_ip_protocol = CA_L3_AAL_KEY_MSK_ENABLE;
	}

	if((acl_filter->filter_fields & RT_ACL_INGRESS_L4_SPORT_RANGE_BIT) && (acl_filter->filter_fields & RT_ACL_INGRESS_L4_DPORT_RANGE_BIT))
	{
		if((acl_filter->ingress_src_l4_port_start == acl_filter->ingress_src_l4_port_end) && (acl_filter->ingress_dest_l4_port_start == acl_filter->ingress_dest_l4_port_end))
		{	//one l4 src port & one l4 dst port
			aal_key->l4_sp = acl_filter->ingress_src_l4_port_start;
			aal_key->l4_dp = acl_filter->ingress_dest_l4_port_start;
			aal_key->msk_l4_port = CA_L3_AAL_KEY_MSK_ENABLE;
			aal_key->l4_vld = 1;
			aal_key->msk_l4_vld = CA_L3_AAL_KEY_MSK_ENABLE;
		}
		else
		{	//l4 src port range & l4 dst port range
			//update l4_sp/l4_dp in _rtk_fc_aclSWEntry_to_asic_cam
			aal_key->msk_l4_port = 3;
			aal_key->l4_vld = 1;
			aal_key->msk_l4_vld = CA_L3_AAL_KEY_MSK_ENABLE;
		}
	}
	else
	{
		if(acl_filter->filter_fields & RT_ACL_INGRESS_L4_SPORT_RANGE_BIT)
		{	//one l4 src port or range
			aal_key->l4_sp = acl_filter->ingress_src_l4_port_start;
			aal_key->l4_dp = acl_filter->ingress_src_l4_port_end;
			aal_key->msk_l4_port = 1;
			aal_key->l4_vld = 1;
			aal_key->msk_l4_vld = CA_L3_AAL_KEY_MSK_ENABLE;
		}
		if(acl_filter->filter_fields & RT_ACL_INGRESS_L4_DPORT_RANGE_BIT)
		{	//one l4 dst port or range
			aal_key->l4_sp = acl_filter->ingress_dest_l4_port_start;
			aal_key->l4_dp = acl_filter->ingress_dest_l4_port_end;
			aal_key->msk_l4_port = 2;
			aal_key->l4_vld = 1;
			aal_key->msk_l4_vld = CA_L3_AAL_KEY_MSK_ENABLE;
		}
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_STREAM_ID_BIT)
	{
		aal_key->o_lspid = RTK_FC_MAC_PORT_PON;
		aal_key->msk_o_lspid = CA_L3_AAL_KEY_MSK_ENABLE;
		aal_key->mdata.mdata_0 = acl_filter->ingress_stream_id;
		aal_key->msk_mdata = ~(0x3);	//only bit 0:7
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_TCP_FLAGS_BIT)
	{
		//8277B: (tcp_flags=0xfff or tcp_flags != tcp_flags_mask) AND mode, otherwise OR mode
		aal_key->tcp_rdp_ctrl = acl_filter->ingress_tcp_flags & acl_filter->ingress_tcp_flags_mask;
		if(((acl_filter->ingress_tcp_flags_mask&0x1ff) == 0x1ff) || (acl_filter->ingress_tcp_flags_mask != acl_filter->ingress_tcp_flags))
			aal_key->msk_tcp_rdp_ctrl = 2;	//match the tcp_rdp_ctrl bits use AND Match
		else
			aal_key->msk_tcp_rdp_ctrl = 1;	//match the tcp_rdp_ctrl bits use OR Match
	}

	if(acl_filter->filter_fields & RT_ACL_INGRESS_PKT_LEN_RANGE_BIT)
	{
		//update pktlen_rng_match_vec in _rtk_fc_aclSWEntry_to_asic_cam
		aal_key->pktlen_rng_match_mask = CA_L3_AAL_KEY_MSK_ENABLE;
	}

#if !defined(CONFIG_FC_G3_G3LITE_SERIES)
	// check arp pattern after stag/ctag
	if(acl_filter->filter_fields & RT_ACL_INGRESS_ARP_TARGET_IP_BIT)
	{
		l3_cam_spcl_hdr_tbl_entry_t cam_spcl_hdr={0};
		aal_l3_specpkt_detect_mask_t l3_spclhdr_msk={0};
		aal_l3_specpkt_detect_t l3_spclhdr={0};
		uint32_t cam_rslt_idx = 0;
		uint32 arpOffset = ARP_TARGET_IP_BYTE_OFFSET;
		
		cam_spcl_hdr.vld = 1;
		cam_spcl_hdr.type = L3_CAM_SPCL_HDR_PROG;
		cam_spcl_hdr.ms_vld = 1;	//bit[31:16]
		cam_spcl_hdr.mask = 0xffff;
		cam_spcl_hdr.key = (acl_filter->ingress_arp_target_ip>>16)&0xffff;;
		if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_SPCL_HDR, &cam_spcl_hdr, &cam_rslt_idx))
			cam_rslt_idx = 0;
		
		aal_key->spcl_pkt_hdr_mtch = 1 << cam_rslt_idx;

		memset(&cam_spcl_hdr, 0, sizeof(cam_spcl_hdr));
		cam_spcl_hdr.vld = 1;
		cam_spcl_hdr.type = L3_CAM_SPCL_HDR_PROG;
		cam_spcl_hdr.ms_vld = 0;	//bit[15:0]
		cam_spcl_hdr.mask = 0xffff;
		cam_spcl_hdr.key = acl_filter->ingress_arp_target_ip&0xffff;;
		if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_SPCL_HDR, &cam_spcl_hdr, &cam_rslt_idx))
			cam_rslt_idx = 1;
		
		aal_key->spcl_pkt_hdr_mtch |= 1 << cam_rslt_idx;
		aal_key->spcl_pkt_hdr_mtch_msk = CA_L3_AAL_KEY_MSK_ENABLE;

		// re-init field sel since raw offset must consider stag/ctag.
		if(*need_more_rule == 2){	//skip (outter_tpid = 0 & inner_tpid = C), only add outter_tpid = C
			ACL_CTRL("[ARP target ip] add one ACL for outter CVLAN, skip 2tags with inner C rule!!!");
			*need_more_rule = 0;
			aal_key->vlan_cnt = 1;
			aal_key->msk_vlan_cnt = CA_L3_AAL_KEY_MSK_ENABLE;
			if (AAL_E_OK != aal_l3fe_pp_top_tpid_get(RT_ACL_PATTERN_CVLAN_TPID, &tpid_idx)){
				WARNING("fail to support outter cvlan tpid 0x%x due to Packet Parser not support!!!", RT_ACL_PATTERN_CVLAN_TPID);
				return (RT_ERR_RG_CHIP_NOT_SUPPORT);
			}
			aal_key->top_tpid_enc = tpid_idx;
			aal_key->msk_top_tpid_enc = CA_L3_AAL_KEY_MSK_ENABLE;
			if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_VID_BIT){
				aal_key->top_vid = acl_filter->ingress_ctag_vid;
				aal_key->msk_top_vid = CA_L3_AAL_KEY_MSK_ENABLE;
			}
			if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_PRI_BIT){
				aal_key->top_802_1p = acl_filter->ingress_ctag_pri;
				aal_key->msk_top_vl_802_1p = CA_L3_AAL_KEY_MSK_ENABLE;
			}
			if(acl_filter->filter_fields & RT_ACL_INGRESS_CTAG_CFI_BIT){
				aal_key->top_dei= acl_filter->ingress_ctag_cfi;
				aal_key->msk_top_dei = CA_L3_AAL_KEY_MSK_ENABLE;
			}
			aal_key->inner_tpid_enc = tpid_idx;
			aal_key->msk_inner_tpid_enc = CA_L3_AAL_KEY_MSK_DISABLE;
			aal_key->msk_inner_vid = CA_L3_AAL_KEY_MSK_DISABLE;
			aal_key->msk_inner_802_1p = CA_L3_AAL_KEY_MSK_DISABLE;
			aal_key->msk_inner_dei = CA_L3_AAL_KEY_MSK_DISABLE;
			arpOffset += 4;
		}else if(*need_more_rule <= 1){
			if(aal_key->msk_vlan_cnt == CA_L3_AAL_KEY_MSK_ENABLE)
				arpOffset += (4 * aal_key->vlan_cnt);
			else{
				if(outter_tpid)
					arpOffset += 4;
				if(inner_tpid)
					arpOffset += 4;
			}
			if(*need_more_rule == 1){
				ACL_CTRL("[ARP target ip] only support 1 tag rule, skip untag rule!!!");
				*need_more_rule = 0;
			}
		}

		//filed sel setting, L3FE_PP_OFFSET_HDR.offset
		l3_spclhdr_msk.s.l7_offset = 1;
		l3_spclhdr.l7_offset = arpOffset;
		if(aal_l3_specpkt_detect_set(0, l3_spclhdr_msk, &l3_spclhdr)!=AAL_E_OK)
			WARNING("ERROR: aal api fail");

		fc_db.controlFuc.acl_arp_targetip_offset = (arpOffset - ARP_TARGET_IP_BYTE_OFFSET + 1);
	}
#endif


	/*set actions*/
	for(i=0; i<RT_ACL_ACTION_FIELDS_INDEX_MAX; i++)
	{
		switch((acl_filter->action_fields)&(0x1<<i))
		{
			case RT_ACL_ACTION_FORWARD_GROUP_DROP_BIT:
				aal_action->permit = 1;
				aal_action->mc = 0;
				aal_action->mcgid = RTK_FC_MAC_PORT_CPU;
				aal_action->dpid_pri = 1;
				aal_action->dpid_vld = 1;
				//ACL should support multiple hit for drop and mib, so use rate=0 drop rather than ldpid=0x1f by another hash profile
				aal_action->t2_ctrl_vld = 1;
				aal_action->t2_ctrl = RTK_ASIC_FLOW_PROFILE_DEFAULT_DROP;
				break;

			case RT_ACL_ACTION_FORWARD_GROUP_TRAP_BIT:
				aal_action->mc = 0;
				aal_action->mcgid = RTK_FC_MAC_PORT_CPU;
				aal_action->dpid_pri = 1;
				aal_action->permit = 1;
				aal_action->dpid_vld = 1;
				aal_action->t2_ctrl_vld = 1;
				aal_action->t2_ctrl = 0xf;
				aal_action->keep_orig_pkt_vld = 1;
				aal_action->keep_orig_pkt = 1;
				break;

			case RT_ACL_ACTION_FORWARD_GROUP_TRAP_TO_PS_BIT:
				aal_action->mc = 0;
				aal_action->mcgid = RTK_FC_MAC_PORT_CPU;
				aal_action->dpid_pri = 1;
				aal_action->permit = 1;
				aal_action->dpid_vld = 1;
				aal_action->t2_ctrl_vld = 1;
				aal_action->t2_ctrl = 0xf;
				aal_action->keep_orig_pkt_vld = 1;
				aal_action->keep_orig_pkt = 1;
#if defined(CONFIG_FC_RTL8277C_SERIES)
				aal_action->mdata_w_vld_0 |= 0x8;
				aal_action->mdata_w_0 |= RXINFO_REF_TRAP_RSN_ACL_TO_PS << RXINFO_REF_TRAP_RSN_SHIFT_BIT;
#elif defined(CONFIG_FC_RTL9607F_SERIES)
				aal_action->mdata_w_vld_0 |= 0x4;
				aal_action->mdata_w_0 |= CPU_REASON_ACL_TRAP_PS << RXINFO_REF_HW_RSN_SHIFT_BIT;	//MDATAL_REASON
#endif
				break;

			case RT_ACL_ACTION_FORWARD_GROUP_PERMIT_BIT:
				//no action
				break;

			case RT_ACL_ACTION_FORWARD_GROUP_REDIRECT_BIT:
				aal_action->mc = 0;
				aal_action->mcgid = acl_filter->action_forward_group_redirect_port_idx;
				aal_action->dpid_pri = 1;
				aal_action->permit = 1;
				aal_action->dpid_vld = 1;
				aal_action->t2_ctrl_vld = 1;
				aal_action->t2_ctrl = 0xf;
				break;

			case RT_ACL_ACTION_PRIORITY_GROUP_TRAP_PRIORITY_BIT:
				aal_action->cos = acl_filter->action_priority_group_trap_priority;
				aal_action->cos_sel = 1;
				break;

			case RT_ACL_ACTION_PRIORITY_GROUP_ACL_PRIORITY_BIT:
				aal_action->cos = acl_filter->action_priority_group_acl_priority;
				aal_action->cos_sel = 1;
				if((acl_filter->action_priority_group_acl_priority == 7) || (acl_filter->action_priority_group_acl_priority == 6)){
					aal_action->no_drop_vld = 1;
					aal_action->no_drop = 1;
				}
				break;

			case RT_ACL_ACTION_METER_GROUP_SHARE_METER_BIT:
				if(fc_db.controlFuc.acl_multiple_hit_cfg == 1)
					aclHWEntry->rslt_type = CL_RSLT_TYPE_2;	//8277B: different rules, meter & forward action both work at the same time
				else
					aclHWEntry->rslt_type = CL_RSLT_TYPE_1;	//disable multiple hit function

				if(fc_db.controlFuc.rt_api_is_enabled){

					ret = RTK_FC_HELPER_RT_RATE_SHAREMETER_MAPPING_HW_GET(acl_filter->action_meter_group_share_meter_index, &meterMapping);
					if((ret == RT_ERR_RG_OK) && (meterMapping.type == RT_METER_TYPE_ACL))
					{
						aclSWEntry->hw_share_meter_index = meterMapping.hw_index;
						for(ret = 0; ret < RTK_FC_TABLESIZE_POLREMAP; ret++){
							if(!fc_db.hwPolRemap[ret].valid)
								break;	//speed up, due to valid start from 0~n
							if(fc_db.hwPolRemap[ret].pol_id == aclSWEntry->hw_share_meter_index){
								aclSWEntry->hw_share_meter_index = ret;
								aal_action->pol23_vld = 1;
								aal_action->pol3_en = 1;
								aal_action->pol3_id = aclSWEntry->hw_share_meter_index;
								ACL_CTRL("pol_id %d remap to pol_id3 %d", aclSWEntry->hw_share_meter_index, ret);
								break;
							}
						}
					}
					else
					{
						WARNING("get ACL meter hw index Failed, return value: 0x%x, virtual meter type: %d", ret, meterMapping.type);
						return (RT_ERR_RG_INVALID_PARAM);
					}
				}else
					aclSWEntry->hw_share_meter_index = acl_filter->action_meter_group_share_meter_index;
				break;

			case RT_ACL_ACTION_LOGGING_GROUP_MIB_BIT:
				aclHWEntry->rslt_type = CL_RSLT_TYPE_2;
				aclSWEntry->hw_mib_index = acl_filter->action_logging_group_mib_index + G3_FLOW_POLICER_IDXSHIFT_ACLMIB;
				aal_action->pol_vld = 1;
				aal_action->pol23_vld = 1;
				aal_action->pol2_msb_en = 1;
				aal_action->pol2_id_msb = ((aclSWEntry->hw_mib_index >> 3) & 0x7);
				aal_action->pol_grp_vld = 1;
				aal_action->pol_grp_id = (aclSWEntry->hw_mib_index & 0x7);
				break;

			default:
				break;
		}
	}

	return (RT_ERR_RG_OK);
}

static int _rtk_fc_aclSWEntry_to_asic_cam(rt_acl_filterAndQos_t *acl_filter, l3_cls_rule_t *rule)
{
	l3_cam_ethertype_tbl_entry_t ethertype;
	l3_cam_sport_tbl_entry_t cam_sport;
	l3_cam_dport_tbl_entry_t cam_dport;
	l3_cam_pkt_len_tbl_entry_t cam_pkt_len;
	uint32 rslt_idx;

	if(rule->key.pktlen_rng_match_mask == CA_L3_AAL_KEY_MSK_ENABLE){
		memset(&cam_pkt_len, 0, sizeof(l3_cam_pkt_len_tbl_entry_t));
		cam_pkt_len.vld = 1;
		cam_pkt_len.pkt_len_low = acl_filter->ingress_packet_length_start;
		cam_pkt_len.pkt_len_hi = acl_filter->ingress_packet_length_end;
		if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_PKT_LEN, &cam_pkt_len, &rslt_idx)){
			WARNING("fail to add pkt length cam table!!!");
			return (RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
		rule->key.pktlen_rng_match_vec = (0x1 << rslt_idx);
	}
	if(rule->key.msk_ethertype_enc == CA_L3_AAL_KEY_MSK_ENABLE){
		memset(&ethertype, 0, sizeof(l3_cam_ethertype_tbl_entry_t));
		ethertype.vld = 1;
		ethertype.ethertype = acl_filter->ingress_ethertype;
		/*if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_ETHERTYPE, &ethertype, &rslt_idx)){
			WARNING("fail to add ethertype cam table!!!");
			return (RT_ERR_RG_CHIP_NOT_SUPPORT);
		}*/
		if(RTK_FC_RET_OK!=_rtk_fc_ethTypeEntry_acl_ref(acl_filter->ingress_ethertype, &rslt_idx)) {
			WARNING("fail to add ethertype cam table!!!");
			return (RTK_FC_RET_ERR_ENTRY_FULL);
		}
		rule->key.ethertype_enc = rslt_idx+1;
	}
	if(rule->key.msk_l4_port == 3){
		//l4 src port range & l4 dst port range
		memset(&cam_sport, 0, sizeof(l3_cam_sport_tbl_entry_t));
		cam_sport.vld = 1;
		cam_sport.sport_low = acl_filter->ingress_src_l4_port_start;
		cam_sport.sport_hi = acl_filter->ingress_src_l4_port_end;
		if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_SPORT, &cam_sport, &rslt_idx)){
			WARNING("fail to add l4 sport cam table!!!");
			return (RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
		rule->key.l4_sp = (0x1 << rslt_idx);

		memset(&cam_dport, 0, sizeof(l3_cam_dport_tbl_entry_t));
		cam_dport.vld = 1;
		cam_dport.dport_low = acl_filter->ingress_dest_l4_port_start;
		cam_dport.dport_hi = acl_filter->ingress_dest_l4_port_end;
		if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_DPORT, &cam_dport, &rslt_idx)){
			WARNING("fail to add l4 dport cam table!!!");
			return (RT_ERR_RG_CHIP_NOT_SUPPORT);
		}
		rule->key.l4_dp = (0x1 << rslt_idx);
	}

	return (RT_ERR_RG_OK);
}
int _rtk_fc_acl_add_acl_rule(rtk_portmask_t *activePorts, rt_acl_filterAndQos_t *acl_filter,rtk_fc_aclFilterEntry_t* aclSWEntry, l3_cls_rule_t *aal_l3_rule, uint8 care_src_port, uint8 *wlan_dev_idx, int index)
{
	uint8 ca_index_offset;
	int i, ret;
	uint8 need_more_rule = 0, one_more_rule = 0;
	uint8 support_stpid2 = 0, tpid_idx1 = 0, tpid_idx2 = 0;
	uint8 ca_aal_profile_idx = 0;
	uint32 ca_index_hw_info = CA_UINT32_INVALID;
	char ca_index_buffer[128];
	rtk_portmask_t caCLSPort;
	int aclCountinueRuleSize =0;

	if((acl_filter->filter_fields & (RT_ACL_INGRESS_STAG_VID_BIT|RT_ACL_INGRESS_STAG_PRI_BIT|RT_ACL_INGRESS_STAG_DEI_BIT)) 
		|| ((acl_filter->filter_fields & RT_ACL_INGRESS_STAGIF_BIT) && (acl_filter->ingress_stagif == 1))
		|| ((acl_filter->filter_fields & RT_ACL_INGRESS_CTAGIF_BIT) && (acl_filter->ingress_ctagif == 0))){
		if(RT_ACL_PATTERN_SVLAN_TPID == RT_ACL_PATTERN_SVLAN_TPID2){
			//not need duplicate rule for stag tpid2
		}else if(AAL_E_OK != aal_l3fe_pp_top_tpid_get(RT_ACL_PATTERN_SVLAN_TPID, &tpid_idx1)){
			WARNING("fail to support svlan tpid2 copy due to tpid1 0x%x Packet Parser not support!!!", RT_ACL_PATTERN_SVLAN_TPID);
		}else if(AAL_E_OK != aal_l3fe_pp_top_tpid_get(RT_ACL_PATTERN_SVLAN_TPID2, &tpid_idx2)){
			WARNING("fail to support svlan tpid2 0x%x due to Packet Parser not support!!!", RT_ACL_PATTERN_SVLAN_TPID2);
		}else if(fc_db.systemGlobal.ifstagTPID1_enabled == 0){
			ACL_CTRL("not to support svlan tpid2 0x%x due to FC not support!!!", RT_ACL_PATTERN_SVLAN_TPID2);
		}else{
			support_stpid2 = TRUE;
		}
	}
	
	bzero(&caCLSPort,sizeof(rtk_portmask_t));
	bzero(&ca_index_buffer,sizeof(ca_index_buffer));
	
	/*add ACL rule*/
	ca_index_offset = 0;
	CA_SCAN_PORTMASK(i, activePorts->bits[0])
	{
		need_more_rule = 0;
		ret = _rtk_fc_aclSWEntry_to_asic_convert(acl_filter, aclSWEntry, aal_l3_rule, &need_more_rule);
		if(ret)
		{
			ACL_CTRL("add user ACL failed (ret=0x%x)!!!", ret);
			return(RT_ERR_RG_INVALID_PARAM);
		}

		do{
			one_more_rule = 0;

#if defined(CONFIG_FC_RTL9607F_SERIES)
			if(care_src_port == RTK_FC_RT_ACL_PORTMASK_IDX){
				aal_l3_rule->key.mdata.mdata_0 = RTK_FC_RT_ACL_PORTMASK_IDX;
				aal_l3_rule->key.msk_mdata = ~(0x3);	//only bit 0:7
			}else
#endif
			if(care_src_port){
				aal_l3_rule->key.o_lspid = i;
				aal_l3_rule->key.msk_o_lspid = CA_L3_AAL_KEY_MSK_ENABLE;
			}
			if((0x1<<i) & (RT_ACL_PORT_ALL_WAN_BIT|CA_L3_CLS_PROFILE_WAN)){
				ca_aal_profile_idx = CA_L3_AAL_CLS_PROFILE_WAN;
				aal_l3_rule->key.msk_o_lspid = CA_L3_AAL_KEY_MSK_DISABLE;
			}else
				ca_aal_profile_idx = CA_L3_AAL_CLS_PROFILE_LAN;
			if((i >= RTK_FC_MAC_PORT_CPU) && (i <= RTK_FC_MAC_PORT_WLAN_CPU5) && (wlan_dev_idx[i-RTK_FC_MAC_PORT_CPU] == 0)){
				ACL_CTRL("= set acl rule port for cpu mac port = 0x%x =", aal_l3_rule->key.o_lspid);
			}else if((i >= RTK_FC_MAC_PORT_CPU) && (i <= RTK_FC_MAC_PORT_WLAN_CPU5)){
				aal_l3_rule->key.msk_o_lspid = CA_L3_AAL_KEY_MSK_DISABLE;
				ca_aal_profile_idx = CA_L3_AAL_CLS_PROFILE_LAN;
				aal_l3_rule->key.mdata.mdata_0 = wlan_dev_idx[i-RTK_FC_MAC_PORT_CPU];
				aal_l3_rule->key.msk_mdata = ~(0x3);	//only bit 0:7 for wlan device id
				ACL_CTRL("= set acl rule port for wlan device idx = %d =", aal_l3_rule->key.mdata.mdata_0);
			}
			ret = _rtk_fc_aclSWEntry_to_asic_cam(acl_filter, aal_l3_rule);
			if(ret)
				return(RT_ERR_RG_ACL_ENTRY_FULL);
			ret = _rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, aal_l3_rule, &ca_index_hw_info, index);
			if(ret == CA_E_FULL)
				return(RT_ERR_RG_ACL_ENTRY_FULL);
			else if(ret)
			{
				ACL_CTRL("add user ACL failed (ret=0x%x)!!!", ret);
				return(RT_ERR_RG_ACL_ENTRY_ACCESS_FAILED);
			}
			caCLSPort.bits[0] |= (0x1<<i);
			aclCountinueRuleSize++;
			ca_index_offset += snprintf(ca_index_buffer+ca_index_offset, sizeof(ca_index_buffer)-ca_index_offset, "%d-%d,", CLS_TBL_ID(ca_index_hw_info), CLS_KEY_ID(ca_index_hw_info));

			if(support_stpid2 && (aal_l3_rule->key.msk_top_tpid_enc == CA_L3_AAL_KEY_MSK_ENABLE) && (aal_l3_rule->key.top_tpid_enc == tpid_idx1)){
				aal_l3_rule->key.top_tpid_enc = tpid_idx2;
				ACL_CTRL("add one more ACL for replace stag tpid1 0x%x(tpid_idx %d) as tpid2 0x%x(tpid_idx %d)!!!", RT_ACL_PATTERN_SVLAN_TPID, tpid_idx1, RT_ACL_PATTERN_SVLAN_TPID2, tpid_idx2);
				one_more_rule = TRUE;
				continue;
			}

			if(need_more_rule){
				ret = _rtk_fc_aclSWEntry_to_asic_convert(acl_filter, aclSWEntry, aal_l3_rule, &need_more_rule);
				if(ret)
				{
					ACL_CTRL("add user ACL failed (ret=0x%x)!!!", ret);
					return(RT_ERR_RG_INVALID_PARAM);
				}
				one_more_rule = TRUE;
			}
		}while(one_more_rule);
	}
	if(aal_l3_rule->key.pktlen_rng_match_mask == CA_L3_AAL_KEY_MSK_ENABLE){
		ca_index_offset += snprintf(ca_index_buffer+ca_index_offset, sizeof(ca_index_buffer)-ca_index_offset, "(PktLenTblVec 0x%x)", aal_l3_rule->key.pktlen_rng_match_vec);
	}
	if(aal_l3_rule->key.msk_ethertype_enc == CA_L3_AAL_KEY_MSK_ENABLE){
		ca_index_offset += snprintf(ca_index_buffer+ca_index_offset, sizeof(ca_index_buffer)-ca_index_offset, "(EthTblIdx %d)", aal_l3_rule->key.ethertype_enc-1);
	}
	if(aal_l3_rule->key.msk_l4_port == 3){
		ca_index_offset += snprintf(ca_index_buffer+ca_index_offset, sizeof(ca_index_buffer)-ca_index_offset, "(L4SportTblVec 0x%x,L4DportTblVec 0x%x)", aal_l3_rule->key.l4_sp, aal_l3_rule->key.l4_dp);
	}
	
	aclSWEntry->hw_aclEntry_count = aclCountinueRuleSize;
	aclSWEntry->hw_aclEntry_port = caCLSPort;
	memcpy(aclSWEntry->hw_aclEntry_index, ca_index_buffer, sizeof(aclSWEntry->hw_aclEntry_index));
	
	return RT_ERR_RG_OK;
}

static int _rtk_fc_aclSWEntry_to_asic_add(rt_acl_filterAndQos_t *acl_filter,rtk_fc_aclFilterEntry_t* aclSWEntry,int shareHwAclWithSWAclIdx)
{
	int index, ret;
	rtk_portmask_t mac_pmsk;
	rtk_portmask_t ext_pmsk[RT_WLAN_DEVICE_MAX];	//only support wlan[0]
	rtk_portmask_t activePorts;
	uint8 wlan_count=RT_WLAN_DEVICE_MAX;
	uint8 wlan_dev_count=RTK_FC_MAC_PORT_MAX-RTK_FC_MAC_PORT_CPU;		//max support 8 wlan count due to CA_SCAN_PORTMASK scan to RTK_FC_MAC_PORT_MAX
	uint8 wlan_dev_idx[RTK_FC_MAC_PORT_MAX-RTK_FC_MAC_PORT_CPU];
	l3_cls_rule_t aal_l3_rule;
	uint8 care_src_port;

	aclSWEntry->hw_aclEntry_count = 0;
	bzero(&activePorts,sizeof(rtk_portmask_t));
	memset(&aal_l3_rule, 0, sizeof(l3_cls_rule_t));
	aal_l3_cls_key_clean(&aal_l3_rule.key);

	if(shareHwAclWithSWAclIdx >= 0)
		index = RTK_FC_ACLANDCF_RESERVED_TAIL_END + shareHwAclWithSWAclIdx;
	else
		index = FAIL;//pre-add test

	ASSERT_EQ(_rtk_fc_acl_portmask_translator(acl_filter->ingress_port_mask, &mac_pmsk),RT_ERR_RG_OK);
	if(acl_filter->filter_fields & RT_ACL_INGRESS_WLAN_MBSSID_MASK_BIT)
		ASSERT_EQ(_rtk_fc_acl_wlan_mbssid_mask_translator(acl_filter->ingress_wlan_mbssid_mask, wlan_count, ext_pmsk, wlan_dev_idx, wlan_dev_count),RT_ERR_RG_OK);
	else{
		bzero(ext_pmsk,sizeof(rtk_portmask_t)*wlan_count);
		bzero(wlan_dev_idx,sizeof(uint8)*wlan_dev_count);
	}

	/*set patterns*/
	if(acl_filter->filter_fields & RT_ACL_CARE_PORT_UNITYPE_PPTP_BIT) {
		if(fc_db.hypridPPTP.portmask)
			ACL_CTRL("= care pptp_port=0x%llx, so mac_port=0x%x =",fc_db.hypridPPTP.portmask,mac_pmsk.bits[0]);
	}else if(fc_db.hypridPPTP.portmask){
		//disable pptp port configuration in all related fc_db.aclRule
		mac_pmsk.bits[0] &= (~fc_db.hypridPPTP.portmask);
		ACL_CTRL("= disable acl rule for pptp_port=0x%llx, so mac_port=0x%x =",fc_db.hypridPPTP.portmask,mac_pmsk.bits[0]);
	}

	care_src_port = 0;
#if defined(CONFIG_FC_RTL9607F_SERIES)
	for(ret = 0; ret < RTK_FC_MAC_PORT_PON; ret++){
		if((mac_pmsk.bits[0] & RT_ACL_PORT_ALL_LAN_BIT) & (1 << ret)){
			care_src_port++;	//lan port count
		}
	}
#endif

	if((((mac_pmsk.bits[0]&RT_ACL_PORT_ALL_LAN_BIT)==RT_ACL_PORT_ALL_LAN_BIT) && ((acl_filter->filter_fields & RT_ACL_INGRESS_WLAN_MBSSID_MASK_BIT)==0x0)
		) || ((acl_filter->ingress_port_mask == RTK_FC_ALL_MAC_PORTMASK) && ((acl_filter->filter_fields & RT_ACL_CARE_PORT_UNITYPE_PPTP_BIT) || (fc_db.hypridPPTP.portmask == 0x0))) )
	{
		//[optimize] all physical port and wifi port: in L3 CLS, it means profile #lan and profile #wan
		activePorts.bits[0] = CA_L3_CLS_PROFILE_LAN;
		if((mac_pmsk.bits[0]&RT_ACL_PORT_ALL_WAN_BIT)==RT_ACL_PORT_ALL_WAN_BIT)
			activePorts.bits[0] |= CA_L3_CLS_PROFILE_WAN;
		care_src_port = 0;
		ACL_CTRL("= set acl rule for activePorts = 0x%x due to both mac_pmsk/ext_pmsk is config as all port =",activePorts.bits[0]);
	}
#if defined(CONFIG_FC_RTL9607F_SERIES)
	else if((care_src_port >= 2) && (((mac_pmsk.bits[0]&RT_ACL_PORT_ALL_LAN_BIT) == fc_db.acl_lan_portmask) || (fc_db.acl_lan_portmask == 0)) && ((acl_filter->filter_fields & RT_ACL_INGRESS_WLAN_MBSSID_MASK_BIT)==0x0))
	{
		activePorts.bits[0] = CA_L3_CLS_PROFILE_LAN;
		if(fc_db.acl_lan_portmask == 0){
			fc_db.acl_lan_portmask = mac_pmsk.bits[0]&RT_ACL_PORT_ALL_LAN_BIT;
			assert_ok(_rtk_rg_aclAndCfReservedRuleAdd(RTK_FC_ACLANDCF_RESERVED_ACL_LAN_PORTMASK_RESET, NULL));
		}
		if((mac_pmsk.bits[0]&RT_ACL_PORT_ALL_WAN_BIT)==RT_ACL_PORT_ALL_WAN_BIT)
			activePorts.bits[0] |= CA_L3_CLS_PROFILE_WAN;
		care_src_port = RTK_FC_RT_ACL_PORTMASK_IDX;
		ACL_CTRL("= set acl rule for activePorts = 0x%x (acl lan portmask 0x%x) =",activePorts.bits[0], fc_db.acl_lan_portmask);
	}
#endif
	else
	{
		care_src_port = 1;
		if((mac_pmsk.bits[0]!=0x0) && ((mac_pmsk.bits[0]&RT_ACL_PORT_ALL_WAN_BIT)==RT_ACL_PORT_ALL_WAN_BIT))
		{	//wan port + lan port
			activePorts.bits[0] = CA_L3_CLS_PROFILE_WAN;
			activePorts.bits[0] |= (mac_pmsk.bits[0]&(~RT_ACL_PORT_ALL_WAN_BIT));
		}
		else if(mac_pmsk.bits[0]!=0x0)
		{	//only lan port
			activePorts = mac_pmsk;
		}
		if(ext_pmsk[RT_WLAN_DEVICE0_INDEX].bits[0]!=0x0)
		{
			activePorts.bits[0] |= ext_pmsk[RT_WLAN_DEVICE0_INDEX].bits[0];
		}
		ACL_CTRL("= set acl rule for activePorts = 0x%x =",activePorts.bits[0]);
	}

	ret = _rtk_fc_acl_add_acl_rule(&activePorts, acl_filter, aclSWEntry, &aal_l3_rule, care_src_port, wlan_dev_idx, index);
	if(ret)
		return(RT_ERR_RG_INVALID_PARAM);

	
	ACL_CTRL("using HW_ACL hw_aclEntry_index is %s (L3 CLS port = 0x%x, total %d rules)",aclSWEntry->hw_aclEntry_index,aclSWEntry->hw_aclEntry_port.bits[0],aclSWEntry->hw_aclEntry_count);

	return (RT_ERR_RG_OK);
}

#endif

static int _rtk_fc_reset_asic_TableEntry(void)
{
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
	int ret, loop, loop_in;

	//Clear hash entry
	for(loop = 0; loop < RTK_FC_MAC_PORT_PON+1; loop++){
		for(loop_in = 0; loop_in < RTK_FC_ACL_HASH_TCP_TYPE_MAX; loop_in++){
			if(fc_db.acl_remap_hash_idx[loop_in][loop]){
				ret = ca_flow_delete(G3_DEF_DEVID, fc_db.acl_remap_hash_idx[loop_in][loop]-1);
				if(ret != CA_E_OK)
					WARNING("delete hash entry[%d] for tcp type[%d] on port[%d] fail...", fc_db.acl_remap_hash_idx[loop_in][loop]-1, loop_in, loop);
				else
					fc_db.acl_remap_hash_idx[loop_in][loop] = 0;
			}
		}
	}
	ACL_CTRL("clear HW_ACL for all related hash entry");
#endif

	//Clear HW_ACL user used table
#if defined(FC_USER_ACL_CA_CLS_SUPPORT)
	//delete H/W ACL , not include reserved entries (G3 use priority to separate Reserve ACL head/User ACL/Reserve ACL tail)
	assert_ok(_rtk_fc_g3_caClsRuleDelete(RTK_CA_CLS_PRIORITY_L3_USER));
	ACL_CTRL("clear HW_ACL for all priority = RTK_CA_CLS_PRIORITY_L3_USER(%d)",RTK_CA_CLS_PRIORITY_L3_USER);
#else
	//pre-add test rule
	assert_ok(_rtk_fc_g3_caAalClsRuleDeleteReserveType(FAIL));
	//RT_ACL[0] to RT_ACL[MAX_ACL_SW_ENTRY_SIZE-1]
	assert_ok(_rtk_fc_g3_caAalClsRuleDelete(RTK_FC_ACLANDCF_RESERVED_TAIL_END, (RTK_FC_ACLANDCF_RESERVED_TAIL_END+MAX_ACL_SW_ENTRY_SIZE)));
	ACL_CTRL("clear HW_ACL for all User ACL(AAL rslt = %d for preadd and RT_ACL[%d]~RT_ACL[%d])",FAIL,0,MAX_ACL_SW_ENTRY_SIZE);
#endif

	return (RT_ERR_RG_OK);
}

void _rtk_fc_acl_for_multicast_temp_protection_add(void)
{
	_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_REARRANGE_PROTECTION, NULL);
	ACL_CTRL("ACL Rearrange Protection(MC permit, Others trap) Start");
	ACL_RSV("ACL Rearrange Protection(MC permit, Others trap) Start");
}

void _rtk_fc_acl_for_multicast_temp_protection_del(void)
{
	ACL_CTRL("ACL Rearrange Protection(MC permit, Others trap) Stop");
	ACL_RSV("ACL Rearrange Protection(MC permit, Others trap) Stop");
	_rtk_rg_aclAndCfReservedRuleDelSpecial(RTK_CA_CLS_TYPE_REARRANGE_PROTECTION, NULL);
}

/*(4)reserved ACL related APIs*/
static int _rtk_fc_aclAndCfReservedRuleAddCheck(uint32 aclRsvSize, uint32 cfRsvSize)
{
	//check rest empty acl rules in enough
/*	if(fc_db.ca_cls_used_count+aclRsvSize >= MAX_ACL_CA_CLS_RULE_SIZE)
	{
		WARNING("ACL rest rules for reserved is not enough! (used_count %d + aclRsvSize %d >= MAX_ACL_CA_CLS_RULE_SIZE %d)", fc_db.ca_cls_used_count, aclRsvSize, MAX_ACL_CA_CLS_RULE_SIZE);
		return (RT_ERR_RG_FAILED);
	}*/
	return (RT_ERR_RG_OK);

}

static int _rtk_fc_aclAndCfReservedRuleHeadReflash(void)
{
	uint32 type = 0;
	int addRuleFailedFlag=0;
	aal_l2_cls_rule_t l2_cls_rule;
	uint32 i = 0;
	rtk_portmask_t activePorts;
	l3_cls_rule_t ca_aal_rule;
	uint8 ca_aal_profile_idx = 0;
	uint8 ca_aal_profile_mask = 0;
	uint32_t ca_aal_rslt_idx = 0;
	uint32_t cam_rslt_idx = 0;
	l3_cam_ethertype_tbl_entry_t cam_eth;
	uint8 aal_idx_offset = 0;
	char aal_idx_buf[64]={0};
	l3_cam_spcl_hdr_tbl_entry_t cam_spcl_hdr;
	l3_cam_pkt_len_tbl_entry_t cam_pkt_len;

	memset(aal_idx_buf, 0, sizeof(aal_idx_buf));

	//reflash th rules
	assert_ok(_rtk_fc_g3_caAalClsRuleDelete(RTK_FC_ACLANDCF_RESERVED_SPECIAL_END, RTK_FC_ACLANDCF_RESERVED_HEAD_END));

	for(type=0;type<RTK_FC_ACLANDCF_RESERVED_HEAD_END;type++){
		if(fc_db.aclAndCfReservedRule.reservedMask[type]==ENABLED){

			switch(type){
				case RTK_CA_CLS_TYPE_L2_INGRESS_FORWARD_L3FE:
					ACL_RSV("update RSV[%d] L2_INGRESS_FORWARD_L3FE (current wanPortMask=0x%llx, wanPortMask=0x%llx)", type, fc_db.aclAndCfReservedRule.acl_wanPortMask, fc_db.wanPortMask.portmask);

					_rtk_rg_aclAndCfReservedRuleDelSpecial(RTK_CA_CLS_TYPE_L2_INGRESS_FORWARD_L3FE, NULL);
					_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_L2_INGRESS_FORWARD_L3FE, NULL);

#if defined(CONFIG_FC_G3_G3LITE_SERIES)
					//update hash profile selection cls due to care lspid
					ACL_RSV("update RSV[%d] GENERIC_INTF_HASH_PROFILE_DECISION", RTK_CA_CLS_TYPE_GENERIC_INTF_HASH_PROFILE_DECISION);
					_rtk_rg_aclAndCfReservedRuleDelSpecial(RTK_CA_CLS_TYPE_GENERIC_INTF_HASH_PROFILE_DECISION, NULL);
					_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_GENERIC_INTF_HASH_PROFILE_DECISION, NULL);
#endif

					fc_db.aclAndCfReservedRule.acl_wanPortMask = fc_db.wanPortMask.portmask;
					fc_db.aclAndCfReservedRule.reservedMask[type]=DISABLED;
					break;

				case RTK_CLS_TYPE_L2_EGRESS_FORWARD_L3FE:
					ACL_RSV("update RSV[%d] L2_EGRESS_FORWARD_L3FE (current wanPortMask=0x%llx, wanPortMask=0x%llx)", type, fc_db.aclAndCfReservedRule.acl_wanPortMask, fc_db.wanPortMask.portmask);

					ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CLS_TYPE_L2_INGRESS_FORWARD_PORT, NULL), CA_E_OK);
					_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CLS_TYPE_L2_EGRESS_FORWARD_L3FE, NULL);

					//update hash profile selection cls due to care lspid
					ACL_RSV("update RSV[%d] GENERIC_INTF_HASH_PROFILE_DECISION", RTK_CA_CLS_TYPE_GENERIC_INTF_HASH_PROFILE_DECISION);
					_rtk_rg_aclAndCfReservedRuleDelSpecial(RTK_CA_CLS_TYPE_GENERIC_INTF_HASH_PROFILE_DECISION, NULL);
					_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CA_CLS_TYPE_GENERIC_INTF_HASH_PROFILE_DECISION, NULL);

					fc_db.aclAndCfReservedRule.acl_wanPortMask = fc_db.wanPortMask.portmask;
					fc_db.aclAndCfReservedRule.reservedMask[type]=DISABLED;
					break;

				case RTK_FC_ACLANDCF_RESERVED_ALL_TRAP:
					{
						memset(&l2_cls_rule, 0, sizeof(l2_cls_rule));
						aal_idx_offset = 0;

#if defined(CONFIG_FC_CA8277AB_SERIES)
						activePorts.bits[0] = RTK_FC_ALL_MAC_PORTMASK_WITHOUT_CPU | RTK_FC_ALL_MAC_WLANCPU_PORTMASK;
#else
						activePorts.bits[0] = 0;
						for(i = ASIC_LPORT_ETH_NI0; i <= ASIC_LPORT_ETH_NI7; i++){
							if((0x1<<i) & RT_ACL_PORT_ALL_LAN_BIT){
								activePorts.bits[0] |= (0x1<<i);
								break;
							}
						}
						for(i = ASIC_LPORT_ETH_NI0; i <= ASIC_LPORT_ETH_NI7; i++){
							if((0x1<<i) & (RT_ACL_PORT_ALL_WAN_BIT | (0x1<<RTK_FC_MAC_PORT_PON))){
								activePorts.bits[0] |= (0x1<<i);
								break;
							}
						}
#endif
						//Create L2 igr Classifier for forward the packet to L3FE
						aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "L2[");
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							l2_cls_rule.priority = RTK_CA_CLS_PRIORITY_L2_HEAD;
							l2_cls_rule.port = i;

							l2_cls_rule.action.dp_valid = TRUE;
							if((0x1<<i) & fc_db.wanPortMask.portmask)
								l2_cls_rule.action.dp_value = AAL_LPORT_L3_WAN;
							else
								l2_cls_rule.action.dp_value = AAL_LPORT_L3_LAN;

							if(_rtk_fc_g3_caAalClsL2RuleAdd(&l2_cls_rule, &ca_aal_rslt_idx, type)) {
								ACL_RSV("add RSV_ACL ALL_TRAP failed!!! port %d", i);
								addRuleFailedFlag=1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%d-%d,", L2_CLS_TBL_ID(ca_aal_rslt_idx), L2_CLS_KEY_ID(ca_aal_rslt_idx));
						}
						aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "], ");

						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);

						//Create L3 Classifier for trap the packet to CPU
						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_HEAD;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL ALL_TRAP failed!!!");
								addRuleFailedFlag = 1;
								break;
							}						
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						ACL_RSV("add RSV[%d] ALL_TRAP (wanPortMask=0x%llx) @ %s", type, fc_db.wanPortMask.portmask, aal_idx_buf);
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_SNAP_OTHER_TRAP:
					{
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);
						aal_idx_offset = 0;

						//SNAP other will cause PP wrong L3 offset and PE stuck, so trap SNAP other(llc type=0xaaaa03xxxxxx) by CLS
						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_HEAD;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							ca_aal_rule.key.len_encoded = 1;
							ca_aal_rule.key.msk_len_encoded = CA_L3_AAL_KEY_MSK_ENABLE;

							memset(&cam_spcl_hdr, 0, sizeof(cam_spcl_hdr));
							cam_spcl_hdr.vld = 1;
							cam_spcl_hdr.type = L3_CAM_SPCL_HDR_L2_PLUS_A;
							cam_spcl_hdr.ms_vld = 1;	//llc first 2B
							cam_spcl_hdr.mask = 0xffff;
							cam_spcl_hdr.key = 0xAAAA;
							if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_SPCL_HDR, &cam_spcl_hdr, &cam_rslt_idx))
								cam_rslt_idx = 0;
							ca_aal_rule.key.spcl_pkt_hdr_mtch = 1 << cam_rslt_idx;

							memset(&cam_spcl_hdr, 0, sizeof(cam_spcl_hdr));
							cam_spcl_hdr.vld = 1;
							cam_spcl_hdr.type = L3_CAM_SPCL_HDR_L2_PLUS_A;
							cam_spcl_hdr.ms_vld = 0;	//llc second 2B
							cam_spcl_hdr.mask = 0xff00;
							cam_spcl_hdr.key = 0x0300;
							if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_SPCL_HDR, &cam_spcl_hdr, &cam_rslt_idx))
								cam_rslt_idx = 1;
							ca_aal_rule.key.spcl_pkt_hdr_mtch |= 1 << cam_rslt_idx;
							ca_aal_rule.key.spcl_pkt_hdr_mtch_msk = CA_L3_AAL_KEY_MSK_ENABLE;

							_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL SNAP_OTHER_TRAP failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						ACL_RSV("add RSV[%d] SNAP_OTHER_TRAP  @ %s", type, aal_idx_buf);
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_BURST_PACKET_SEND_MCE:
					{
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);
						aal_idx_offset = 0;

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
						{
							rtk_fc_asic_dmaLso_reg_t dmaLso_reg;
							dmaLso_reg.lspid_keep_vld = 1;
							ASSERT_EQ(rtk_fc_asic_dmaLso_reg_get(&dmaLso_reg), RT_ERR_RG_OK);
							if(dmaLso_reg.lspid_keep == 0)
								WARNING("BURST_PACKET_SEND_MCE warning: DMALSO lspid_keep=0 may not keep lspid.");
						}
#endif
						//control by rt_misc set burstPacket...
						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_HEAD;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							ca_aal_rule.key.o_lspid = fc_db.aclAndCfReservedRule.burstPktSend_conf.pkt_lspid;
							ca_aal_rule.key.msk_o_lspid = CA_L3_AAL_KEY_MSK_ENABLE;
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
							ca_aal_rule.key.msk_mdata = ~(0x3);
							ca_aal_rule.key.mdata.mdata_0 = RTK_FC_WIFI_HWLOOKUP_ID_BURST_PKT;//copy by L3PP
#endif

							_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);
							ca_aal_rule.action.mc = TRUE;
							ca_aal_rule.action.mcgid = fc_db.aclAndCfReservedRule.burstPktSend_conf.hw_mcgid;

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								WARNING("add RSV_ACL BURST_PACKET_SEND_MCE failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", /*ca_aal_profile_idx?*/"LAN"/*:"WAN"*/, CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}
						ACL_RSV("add RSV[%d] BURST_PACKET_SEND_MCE  @ %s", type, aal_idx_buf);
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_WIFI_SNAP_TRANS:
					{
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
						rtk_fc_asic_dmaLso_reg_t dmaLso_reg;
						dmaLso_reg.lspid_keep_vld = 1;
						ASSERT_EQ(rtk_fc_asic_dmaLso_reg_get(&dmaLso_reg), RT_ERR_RG_OK);
						if(dmaLso_reg.lspid_keep){
#if defined(CONFIG_RTK_FC_WIFI_DRIVER_OFFLOAD_BY_PE)
							activePorts.bits[0] = (1<<RTK_FC_WLAN_HWLOOKUP_LSPID0) | (1<<RTK_FC_WLAN_HWLOOKUP_LSPID1);
#else
							activePorts.bits[0] = (1<<RTK_FC_MAC_PORT_WLAN_MAINCPU);
#endif
						}else
#endif
							activePorts.bits[0] = CA_L3_CLS_CPU_TX_PORTMASK;
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);
						aal_idx_offset = 0;

						ca_aal_profile_idx = CA_L3_AAL_CLS_PROFILE_LAN;
						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_HEAD;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_2;

							ca_aal_rule.key.msk_o_lspid = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.o_lspid = i;

							ca_aal_rule.key.msk_len_encoded = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.len_encoded = 1;

							ca_aal_rule.action.l2_format = 0;	//0: Ethernet II, 1: snap
							ca_aal_rule.action.l2_format_vld = 1;

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL WIFI_SNAP_TRANS failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", /*ca_aal_profile_idx?*/"LAN"/*:"WAN"*/, CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						ACL_RSV("add RSV[%d] WIFI_SNAP_TRANS @ %s", type, aal_idx_buf);
					}
					break;
				case RTK_FC_ACLANDCF_RESERVED_WIFI_TXAMSDU_HW_LOOKUP:
					{
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);
						aal_idx_offset = 0;

						ca_aal_profile_idx = CA_L3_AAL_CLS_PROFILE_LAN;
						ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_HEAD;
						ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

						ca_aal_rule.key.msk_o_lspid = CA_L3_AAL_KEY_MSK_ENABLE;
#if defined(CONFIG_FC_RTL8277C_SERIES)
						ca_aal_rule.key.o_lspid = 0x1a;
#else
						ca_aal_rule.key.o_lspid = 0x1c;
#endif

						_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);
						ca_aal_rule.action.mcgid = RTK_FC_MAC_PORT_WLAN_WFO;	//ldpid
						ca_aal_rule.action.mdata_w_vld_0 = 0x4;
						ca_aal_rule.action.mdata_w_0 = CPU_REASON_L34_FWD << RXINFO_REF_HW_RSN_SHIFT_BIT;

						if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
						{
							ACL_RSV("add RSV_ACL WIFI_TXAMSDU_HW_LOOKUP failed!!!");
							addRuleFailedFlag = 1;
							break;
						}
						aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", /*ca_aal_profile_idx?*/"LAN"/*:"WAN"*/, CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));

						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);

						ca_aal_profile_idx = CA_L3_AAL_CLS_PROFILE_LAN;
						ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_HEAD;
						ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

						ca_aal_rule.key.msk_o_lspid = CA_L3_AAL_KEY_MSK_ENABLE;
						ca_aal_rule.key.o_lspid = 0x1d;

						_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);
						ca_aal_rule.action.mdata_w_vld_0 = 0x4;
						ca_aal_rule.action.mdata_w_0 = CPU_REASON_L34_FWD << RXINFO_REF_HW_RSN_SHIFT_BIT;
#if defined(CONFIG_FC_RTL8277C_SERIES)
						ca_aal_rule.action.mcgid = RTK_FC_MAC_PORT_CPU;	//ldpid
						//PE should transform l2 by itself due to CLS can't update l2 format and keep pppoe protocol at the same time
						ca_aal_rule.action.keep_orig_pkt_vld = 0;
						//HW ACL module force update mdata_w_0 as ACL trap if ldpid=0x10 and keep_orig, this case should keep flow_miss rsn.
#else	//CONFIG_FC_RTL9607F_SERIES
						ca_aal_rule.action.mcgid = RTK_FC_MAC_PORT_WLAN_WFO;	//ldpid
						ca_aal_rule.action.keep_orig_pkt_vld = 0;
						ca_aal_rule.action.l2_format = 0;	//0: Ethernet II, 1: snap
						ca_aal_rule.action.l2_format_vld = 1;
						ca_aal_rule.action.modify_vlan_only = 1;
						ca_aal_rule.action.modify_vlan_only_vld = 1;
#endif

						if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
						{
							ACL_RSV("add RSV_ACL WIFI_TXAMSDU_HW_LOOKUP failed!!!");
							addRuleFailedFlag = 1;
							break;
						}
						aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", /*ca_aal_profile_idx?*/"LAN"/*:"WAN"*/, CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));

						ACL_RSV("add RSV[%d] WIFI_TXAMSDU_HW_LOOKUP @ %s", type, aal_idx_buf);
#endif
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_WIFI_TXAMSDU_MC_TO_UC:
					{
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);
						aal_idx_offset = 0;

						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_HEAD;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_0;

							ca_aal_rule.key.msk_o_lspid = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.o_lspid = 0x1b;

							ca_aal_rule.action.t2_ctrl = CLS_T2CTRL_MAINHASH_PROFILE_MC_WIFI_TX;
							_rtk_fc_g3_caAalClsAssignHashProfileAction(&ca_aal_rule, ca_aal_profile_idx, FALSE, 0);

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL WIFI_TXAMSDU_MC_TO_UC failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", /*ca_aal_profile_idx?*/"LAN"/*:"WAN"*/, CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						ACL_RSV("add RSV[%d] WIFI_TXAMSDU_MC_TO_UC @ %s", type, aal_idx_buf);
#endif
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_IPSEC_ACCELERATE_BY_PE:
					{
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);
						aal_idx_offset = 0;

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
						{
							rtk_fc_asic_dmaLso_reg_t dmaLso_reg;
							dmaLso_reg.lspid_keep_vld = 1;
							ASSERT_EQ(rtk_fc_asic_dmaLso_reg_get(&dmaLso_reg), RT_ERR_RG_OK);
							if(dmaLso_reg.lspid_keep == 0)
								WARNING("IPSEC_ACCELERATE_BY_PE warning: DMALSO lspid_keep=0 may not keep lspid.");
						}
#endif
						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_HEAD;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							ca_aal_rule.key.msk_o_lspid = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.o_lspid = fc_db.aclAndCfReservedRule.ipsec_accelerate_by_pe.igr_port_idx;	//lspid = 0x1c

							_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);
							ca_aal_rule.action.mcgid = fc_db.aclAndCfReservedRule.ipsec_accelerate_by_pe.action_redirect_ldpid;
							ca_aal_rule.action.mdata_w_vld_0 = 0x4;
							ca_aal_rule.action.mdata_w_0 = CPU_REASON_L34_FWD << RXINFO_REF_HW_RSN_SHIFT_BIT;
#if defined(CONFIG_FC_RTL8277C_SERIES)
							if(ca_aal_rule.action.mcgid == RTK_FC_MAC_PORT_CPU){
								WARNING("[Attention] IPSEC_ACCELERATE_BY_PE: hw reason will be replace by hw");
							}
#endif
							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL IPSEC_ACCELERATE_BY_PE failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", /*ca_aal_profile_idx?*/"LAN"/*:"WAN"*/, CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						ACL_RSV("add RSV[%d] IPSEC_ACCELERATE_BY_PE @ %s", type, aal_idx_buf);
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_ACL_LAN_PORTMASK_RESET:
					{
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);
						aal_idx_offset = 0;

						ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CLS_TYPE_L2_INGRESS_FORWARD_PORT, NULL), CA_E_OK);
						if((fc_db.acl_lan_portmask == 0) || (fc_db.acl_lan_portmask == RTK_FC_RT_ACL_PORTMASK_INVALID))
							break;
						for(i = ASIC_LPORT_ETH_NI0; i <= ASIC_LPORT_ETH_NI7; i++){
							if((0x1<<i) & fc_db.acl_lan_portmask){
								break;
							}
						}
						activePorts.bits[0] = (0x1 << i);
						CA_SCAN_PORTMASK(i, activePorts.bits[0]){
							memset(&l2_cls_rule, 0, sizeof(aal_l2_cls_rule_t));
							l2_cls_rule.port = i;
							l2_cls_rule.priority = RTK_CA_CLS_PRIORITY_L2_HEAD;

							l2_cls_rule.action.dp_valid = TRUE;
							l2_cls_rule.action.dp_value = AAL_LPORT_L3_LAN;
							l2_cls_rule.action.mark_ena = TRUE;
							l2_cls_rule.action.flowid = RTK_FC_RT_ACL_PORTMASK_IDX;

							if(_rtk_fc_g3_caAalClsL2RuleAdd(&l2_cls_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL ACL_LAN_PORTMASK_RESET failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "L2_%s[%d-%d],", (i == ASIC_LPORT_ETH_NI7)?"WAN":"LAN", L2_CLS_TBL_ID(ca_aal_rslt_idx), L2_CLS_KEY_ID(ca_aal_rslt_idx));
						}

						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_HEAD;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_3;

							ca_aal_rule.key.msk_mdata = ~(0x3);
							ca_aal_rule.key.mdata.mdata_0 = RTK_FC_RT_ACL_PORTMASK_IDX;//copy by L3PP

							ca_aal_rule.action.mdata_w_vld_0 = 0x3;
							ca_aal_rule.action.mdata_w_0 = 0;

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								WARNING("add RSV_ACL ACL_LAN_PORTMASK_RESET failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", /*ca_aal_profile_idx?*/"LAN"/*:"WAN"*/, CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}
						ACL_RSV("add RSV[%d] ACL_LAN_PORTMASK_RESET (lan_portmask=0x%x)  @ %s", type, fc_db.acl_lan_portmask, aal_idx_buf);
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_SMALL_TCP_ACK_TRAP:
					{
#if defined(CONFIG_RTK_FC_SW_ACK_DELAY_WIDTH) && (CONFIG_RTK_FC_SW_ACK_DELAY_WIDTH > 5)
						aal_idx_offset = 0;
						bzero(&activePorts,sizeof(rtk_portmask_t));
						for(i = RTK_FC_MAC_PORT0; i <= RTK_FC_MAC_PORT_MAX; i++){
							if(fc_db.ackDelayCtrl.igr_portmask & (1<<i)){
								activePorts.bits[0] |= (1<<i);
								break;	//only support one port
							}
						}
						if((activePorts.bits[0] == 0) || (fc_db.ackDelayCtrl.igr_pkt_len_max == 0)){
							ACL_RSV("skip RSV[%d] SMALL_TCP_ACK_TRAP due to invalid portmask 0x%x or pkt len max %d", type, fc_db.ackDelayCtrl.igr_portmask, fc_db.ackDelayCtrl.igr_pkt_len_max);
							break;
						}

						CA_SCAN_PORTMASK(i, activePorts.bits[0])
						{
							memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
							aal_l3_cls_key_clean(&ca_aal_rule.key);

							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_HEAD;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_2;

							ca_aal_rule.key.msk_o_lspid = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.o_lspid = RTK_FC_ACKDELAY_HWLKP_LSPID;

							ca_aal_profile_idx = CA_L3_AAL_CLS_PROFILE_LAN;

							ca_aal_rule.action.lspid_vld = 1;
							ca_aal_rule.action.lspid = i;

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL SMALL_TCP_ACK_TRAP failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));

							memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
							aal_l3_cls_key_clean(&ca_aal_rule.key);

							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_HEAD;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							ca_aal_rule.key.msk_ip_vld = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.ip_vld = 1;
							ca_aal_rule.key.msk_ip_protocol = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.ip_protocol = 0x6;
							ca_aal_rule.key.msk_tcp_rdp_ctrl = 2;	//match the tcp_rdp_ctrl bits use AND Match
							ca_aal_rule.key.tcp_rdp_ctrl = 0x10;	//ack only

							memset(&cam_pkt_len, 0, sizeof(cam_pkt_len));
							cam_pkt_len.vld = 1;
							cam_pkt_len.pkt_len_low = fc_db.ackDelayCtrl.igr_pkt_len_min;
							cam_pkt_len.pkt_len_hi = fc_db.ackDelayCtrl.igr_pkt_len_max;
							if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_PKT_LEN, &cam_pkt_len, &cam_rslt_idx))
								cam_rslt_idx = 0;
							ca_aal_rule.key.pktlen_rng_match_vec = 0x1 << cam_rslt_idx;
							ca_aal_rule.key.pktlen_rng_match_mask = CA_L3_AAL_KEY_MSK_ENABLE;

							ca_aal_rule.key.msk_o_lspid = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.o_lspid = i;

							if(fc_db.wanPortMask.portmask & (1<<i))
								ca_aal_profile_idx = CA_L3_AAL_CLS_PROFILE_WAN;
							else
								ca_aal_profile_idx = CA_L3_AAL_CLS_PROFILE_LAN;
							_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);

							ca_aal_rule.action.cos = fc_db.ackDelayCtrl.sw_fwd_acl_trap_pri;
							ca_aal_rule.action.cos_sel = TRUE;

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL SMALL_TCP_ACK_TRAP failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						ACL_RSV("add RSV[%d] SMALL_TCP_ACK_TRAP @ %s", type, aal_idx_buf);
#endif
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP:
					{
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);
						aal_idx_offset = 0;

						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_HEAD;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							ca_aal_rule.key.msk_ip_addr_l = 32;
							ca_aal_rule.key.ip_addr_type = 0;	//0: da, 1: sa
							ca_aal_rule.key.ip_addr.ip_addr_0 = 0xEFFFFFFA;	//239.255.255.250
							ca_aal_rule.key.ip_addr.ip_addr_1 = ca_aal_rule.key.ip_addr.ip_addr_2 = ca_aal_rule.key.ip_addr.ip_addr_3 = 0;

							_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL MULTICAST_SSDP_TRAP failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						ACL_RSV("add RSV[%d] MULTICAST_SSDP_TRAP @ %s", type, aal_idx_buf);
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_MULTICAST_RIP_TRAP:
					{
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);
						aal_idx_offset = 0;

						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_HEAD;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							ca_aal_rule.key.msk_ip_addr_l = 32;
							ca_aal_rule.key.ip_addr_type = 0;	//0: da, 1: sa
							ca_aal_rule.key.ip_addr.ip_addr_0 = 0xE0000009; //224.0.0.9
							ca_aal_rule.key.ip_addr.ip_addr_1 = ca_aal_rule.key.ip_addr.ip_addr_2 = ca_aal_rule.key.ip_addr.ip_addr_3 = 0;

							_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL MULTICAST_RIP_TRAP failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						ACL_RSV("add RSV[%d] MULTICAST_RIP_TRAP @ %s", type, aal_idx_buf);
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY:
					{
						/*Set ACL rule for higher rx prioirty for L2TP controal packet and LCP packet(Wan port + UDP + Sport(1701) + Dport(1701) + pktLen < 128)*/
#if defined(CONFIG_FC_CA8277B_SERIES)
						if(fc_db.controlFuc.special_fast_forward_mode==1 &&fc_db.vxlan_specialFastFwd_setup_l3cls==1)
						{	
							//aal_customize |= CA_CLASSIFIER_AAL_L3_PKTLEN_NONE_VXLAN_RELATED;
							ACL_RSV("skip add RSV_ACL L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY for vxlan!!!");
							break;
						}
#endif

						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);
						aal_idx_offset = 0;

						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask){
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_HEAD;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							ca_aal_rule.key.msk_ip_protocol = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.ip_protocol = 0x11;

#if defined(CONFIG_FC_G3_G3LITE_SERIES)
							/*Set ACL rule for higher rx prioirty for L2TP controal packet and LCP packet(Wan port + UDP + Sport(1701) + Dport(1701) + pktLen < 128)
								Note 1: l2tp packet not support sport/dport, so we use spacial header to do this
								Note 2: Due to CA L3 CLS not support these pattern in one rule, so seperate as below two rules:
									Rule 1: CLS Type0, key=udp + special header(type=2, key=sport/dport 1701), action=trap to CPU+assign higher priority
									Rule 2: CLS Type1, key=udp + packet length range 128~9216, action=assign priority 0
								Target: trap all l2tp packet, and l2tp data packet use lower priority. others data packet recover their priority by flow*/

							ca_aal_rule.rslt_type = CL_RSLT_TYPE_0;
#else
							memset(&cam_pkt_len, 0, sizeof(cam_pkt_len));
							cam_pkt_len.vld = 1;
							cam_pkt_len.pkt_len_low = 0;
							cam_pkt_len.pkt_len_hi = 80;
							if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_PKT_LEN, &cam_pkt_len, &cam_rslt_idx))
								cam_rslt_idx = 0;
							ca_aal_rule.key.pktlen_rng_match_vec = 0x1 << cam_rslt_idx;
							ca_aal_rule.key.pktlen_rng_match_mask = CA_L3_AAL_KEY_MSK_ENABLE;
#endif

#if defined(CONFIG_FC_CA8277AB_SERIES)
							memset(&cam_spcl_hdr, 0, sizeof(cam_spcl_hdr));
							cam_spcl_hdr.vld = 1;
							cam_spcl_hdr.type = L3_CAM_SPCL_HDR_L3_PLUS;
							cam_spcl_hdr.ms_vld = 1;	//sport
							cam_spcl_hdr.mask = 0xffff;
							cam_spcl_hdr.key = 0x06A5;
							if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_SPCL_HDR, &cam_spcl_hdr, &cam_rslt_idx))
								cam_rslt_idx = 0;
							ca_aal_rule.key.spcl_pkt_hdr_mtch = 1 << cam_rslt_idx;

							memset(&cam_spcl_hdr, 0, sizeof(cam_spcl_hdr));
							cam_spcl_hdr.vld = 1;
							cam_spcl_hdr.type = L3_CAM_SPCL_HDR_L3_PLUS;
							cam_spcl_hdr.ms_vld = 0;	//dport
							cam_spcl_hdr.mask = 0xffff;
							cam_spcl_hdr.key = 0x06A5;
							if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_SPCL_HDR, &cam_spcl_hdr, &cam_rslt_idx))
								cam_rslt_idx = 1;
							ca_aal_rule.key.spcl_pkt_hdr_mtch |= 1 << cam_rslt_idx;
							ca_aal_rule.key.spcl_pkt_hdr_mtch_msk = CA_L3_AAL_KEY_MSK_ENABLE;
#else
							ca_aal_rule.key.msk_mdata = ~(0x1000);
							ca_aal_rule.key.mdata.mdata_1 = RTK_PP_PKT_TYPE_IPV4_ONLY << 16;	//ipv4 only (l2tp lcp inner is not ip, so )

							ca_aal_rule.key.l4_vld = TRUE;
							ca_aal_rule.key.msk_l4_vld = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.l4_sp = 1701;
							ca_aal_rule.key.l4_dp = 1701;
							ca_aal_rule.key.msk_l4_port = 0;	//0:sp,dp;1:sp range;2:dp range;3:rng_match
#endif

							//force trap to CPU with priroity to avoid FB trap priroity replaced.
							ca_aal_rule.action.cos = fc_db.aclAndCfReservedRule.l2tp_ctrl_lcp_assign_prioity.priority;
							ca_aal_rule.action.cos_sel = TRUE;

							_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						ACL_RSV("add RSV[%d] L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY @ %s", type, aal_idx_buf);
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY:
					{
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);
						aal_idx_offset = 0;

						//Set ACL rule for higher rx prioirty for PPPoE LCP packet(ethertype=0x8864, none-ipv4, none-ipv6)
						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_HEAD;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							ca_aal_rule.key.msk_ip_vld = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.ip_vld = 0;	//none-ipv4 & none-ipv6

							memset(&cam_eth, 0, sizeof(l3_cam_ethertype_tbl_entry_t));
							cam_eth.vld = 1;
							cam_eth.ethertype = 0x8864;
							//if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_ETHERTYPE, &cam_eth, &cam_rslt_idx))
							if(RTK_FC_RET_OK!=_rtk_fc_ethTypeEntry_acl_ref(cam_eth.ethertype, &cam_rslt_idx))
								cam_rslt_idx = 0;
							ca_aal_rule.key.ethertype_enc = cam_rslt_idx + 1;
							ca_aal_rule.key.msk_ethertype_enc = CA_L3_AAL_KEY_MSK_ENABLE;

							ca_aal_rule.action.cos = fc_db.aclAndCfReservedRule.pppoe_lcp_assign_prioity.priority;
							ca_aal_rule.action.cos_sel = TRUE;

							_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL PPPoE_LCP_PACKET_ASSIGN_PRIORITY failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						ACL_RSV("add RSV[%d] PPPoE_LCP_PACKET_ASSIGN_PRIORITY @ %s", type, aal_idx_buf);
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_PPTP_GRE_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY:
					{
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);
						aal_idx_offset = 0;

						//Set ACL rule for higher rx prioirty for PPTP GRE LCP packet(dmac=gw mac, ip protocol=47, packet length=0~80)
						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_HEAD;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							ca_aal_rule.key.msk_mac_da_an_hit = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.mac_da_an_hit = 1;

							ca_aal_rule.key.msk_ip_protocol = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.ip_protocol = 0x2f;	//GRE protocal value 47

							memset(&cam_pkt_len, 0, sizeof(cam_pkt_len));
							cam_pkt_len.vld = 1;
							cam_pkt_len.pkt_len_low = 0;
							cam_pkt_len.pkt_len_hi = 80;
							if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_PKT_LEN, &cam_pkt_len, &cam_rslt_idx))
								cam_rslt_idx = 0;
							ca_aal_rule.key.pktlen_rng_match_vec = 0x1 << cam_rslt_idx;
							ca_aal_rule.key.pktlen_rng_match_mask = CA_L3_AAL_KEY_MSK_ENABLE;

							ca_aal_rule.action.cos = fc_db.aclAndCfReservedRule.pptp_gre_ctrl_lcp_assign_prioity.priority;
							ca_aal_rule.action.cos_sel = TRUE;

							_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL PPTP_GRE_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						ACL_RSV("add RSV[%d] PPTP_GRE_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY @ %s", type, aal_idx_buf);
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

static int _rtk_fc_aclAndCfReservedRuleTailReflash(void)
{
	uint32 type = 0;
	int addRuleFailedFlag=0;
	l3_cls_rule_t ca_aal_rule;
	uint8 ca_aal_profile_idx = 0;
	uint8 ca_aal_profile_mask = 0;
	uint32_t ca_aal_rslt_idx = 0;
	uint32_t cam_rslt_idx = 0;
	l3_cam_ethertype_tbl_entry_t cam_eth;
	uint8 aal_idx_offset = 0;
	char aal_idx_buf[64];
	l3_cam_pkt_len_tbl_entry_t cam_pkt_len;
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
	l3_cam_ppp_prot_tbl_entry_t cam_ppp_prot;
#endif

	memset(aal_idx_buf, 0, sizeof(aal_idx_buf));

	//reflash th rules
	assert_ok(_rtk_fc_g3_caAalClsRuleDelete(RTK_FC_ACLANDCF_RESERVED_HEAD_END, (RTK_FC_ACLANDCF_RESERVED_TAIL_END-1)));	//RTK_FC_ACLANDCF_RESERVED_TAIL_END belongs to RT_ACL[0]

	for(type=RTK_FC_ACLANDCF_RESERVED_HEAD_END;type<RTK_FC_ACLANDCF_RESERVED_TAIL_END;type++){
		if(fc_db.aclAndCfReservedRule.reservedMask[type]==ENABLED){
			switch(type){
				case RTK_FC_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT:
					{
						//Due to G3 not support filter FFxE, so only trap control packet
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);
						aal_idx_offset = 0;

						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							ca_aal_rule.key.ip_ver = 1;	//0: ipv4, 1: ipv6
							ca_aal_rule.key.msk_ip_ver= CA_L3_AAL_KEY_MSK_ENABLE;

							//Trap multicast addresses in IPv6 use the prefix ff0X/12.
#if defined(CONFIG_FC_G3_G3LITE_SERIES)	//FULL_KEY
							ca_aal_rule.key.msk_ip_da_l = 12;
							ca_aal_rule.key.msk_ip_da_h = 0;
							ca_aal_rule.key.ip_da.ip_addr_3 = 0xff000000; //ff0x:xxxx:xxx...
							ca_aal_rule.key.ip_da.ip_addr_0 = ca_aal_rule.key.ip_da.ip_addr_1 = ca_aal_rule.key.ip_da.ip_addr_2 = 0;
#else									//FLOW_KEY
							ca_aal_rule.key.msk_ip_addr_l = 12;
							ca_aal_rule.key.msk_ip_addr_h = 0;
							ca_aal_rule.key.ip_addr_type = 0;	//0: da, 1: sa
							ca_aal_rule.key.ip_addr.ip_addr_3 = 0xff000000;	//ff0x:xxxx:xxx...
							ca_aal_rule.key.ip_addr.ip_addr_0 = ca_aal_rule.key.ip_addr.ip_addr_1 = ca_aal_rule.key.ip_addr.ip_addr_2 = 0;
#endif

							_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT for FF0x failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						//Trap v6 dhcp prevent by G3 unknown mc drop
						//Due to we need support trap dhcpv6 with higher priority by RT API, so use cls rather than special packet
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);

						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							ca_aal_rule.key.ip_ver = 1;	//0: ipv4, 1: ipv6
							ca_aal_rule.key.msk_ip_ver= CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.ip_protocol = 0x11;	//UDP protocal value 17
							ca_aal_rule.key.msk_ip_protocol = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.ip_da_mc = 1;
							ca_aal_rule.key.msk_ip_da_mc = CA_L3_AAL_KEY_MSK_ENABLE;

							ca_aal_rule.key.l4_vld = TRUE;
							ca_aal_rule.key.msk_l4_vld = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.l4_sp = 546;
							ca_aal_rule.key.l4_dp = 547;
							ca_aal_rule.key.msk_l4_port = 2;	//0:sp,dp;1:sp range;2:dp range;3:rng_match

							_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT for DHCPv6 failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						ACL_RSV("add RSV[%d] MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT @ %s", type, aal_idx_buf);
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP:
					{
						//IGMP (prevent ipv4 igmp drop by main hash unknown mc)
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);
						aal_idx_offset = 0;

						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							ca_aal_rule.key.ip_ver = 0;	//0: ipv4, 1: ipv6
							ca_aal_rule.key.msk_ip_ver= CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.ip_protocol = 0x2;	//igmp
							ca_aal_rule.key.msk_ip_protocol = CA_L3_AAL_KEY_MSK_ENABLE;

							_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL CONTROL_PACKET_TRAP for IGMP failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						//PPPoE Discovery (ethertype=0x8863)
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);

						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							memset(&cam_eth, 0, sizeof(cam_eth));
							cam_eth.vld = 1;
							cam_eth.ethertype = 0x8863;
							//if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_ETHERTYPE, &cam_eth, &cam_rslt_idx))
							if(RTK_FC_RET_OK!=_rtk_fc_ethTypeEntry_acl_ref(cam_eth.ethertype, &cam_rslt_idx))
								cam_rslt_idx = 0;
							ca_aal_rule.key.ethertype_enc = cam_rslt_idx + 1;
							ca_aal_rule.key.msk_ethertype_enc = CA_L3_AAL_KEY_MSK_ENABLE;

							_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL CONTROL_PACKET_TRAP for PPPOE discovery failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						ACL_RSV("add RSV[%d] CONTROL_PACKET_TRAP @ %s", type, aal_idx_buf);

#if defined(CONFIG_FC_G3_G3LITE_SERIES)
						//PPPoE Control Packet(Step1:permit ppp proto 0021/0057)
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);
						aal_idx_offset = 0;

						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							memset(&cam_eth, 0, sizeof(cam_eth));
							cam_eth.vld = 1;
							cam_eth.ethertype = 0x8864;
							//if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_ETHERTYPE, &cam_eth, &cam_rslt_idx))
							if(RTK_FC_RET_OK!=_rtk_fc_ethTypeEntry_acl_ref(cam_eth.ethertype, &cam_rslt_idx))
								cam_rslt_idx = 0;
							ca_aal_rule.key.ethertype_enc = cam_rslt_idx + 1;
							ca_aal_rule.key.msk_ethertype_enc = CA_L3_AAL_KEY_MSK_ENABLE;

							//PPP Protocol - IPv4 = 0x0021
							memset(&cam_ppp_prot, 0, sizeof(cam_ppp_prot));
							cam_ppp_prot.vld = 1;
							cam_ppp_prot.ppp_prot = 0x0021;
							if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_PPP_PROT, &cam_ppp_prot, &cam_rslt_idx))
								cam_rslt_idx = 0;
							ca_aal_rule.key.ppp_protocol_enc = cam_rslt_idx + 1;
							ca_aal_rule.key.msk_ppp_protocol = CA_L3_AAL_KEY_MSK_ENABLE;

							//permit

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL CONTROL_PACKET_TRAP for PPPoE Control Packet(Step 1-IPv4) failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);

						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							memset(&cam_eth, 0, sizeof(cam_eth));
							cam_eth.vld = 1;
							cam_eth.ethertype = 0x8864;
							//if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_ETHERTYPE, &cam_eth, &cam_rslt_idx))
							if(RTK_FC_RET_OK!=_rtk_fc_ethTypeEntry_acl_ref(cam_eth.ethertype, &cam_rslt_idx))
								cam_rslt_idx = 0;
							ca_aal_rule.key.ethertype_enc = cam_rslt_idx + 1;
							ca_aal_rule.key.msk_ethertype_enc = CA_L3_AAL_KEY_MSK_ENABLE;
							
							//PPP Protocol - IPv6 = 0x0057
							memset(&cam_ppp_prot, 0, sizeof(cam_ppp_prot));
							cam_ppp_prot.vld = 1;
							cam_ppp_prot.ppp_prot = 0x0057;
							if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_PPP_PROT, &cam_ppp_prot, &cam_rslt_idx))
								cam_rslt_idx = 0;
							ca_aal_rule.key.ppp_protocol_enc = cam_rslt_idx + 1;
							ca_aal_rule.key.msk_ppp_protocol = CA_L3_AAL_KEY_MSK_ENABLE;

							//permit

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL CONTROL_PACKET_TRAP for PPPoE Control Packet(Step 1-IPv6) failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						//PPPoE Control Packet(Step2:trap ethertype=0x8864)
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);

						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							memset(&cam_eth, 0, sizeof(cam_eth));
							cam_eth.vld = 1;
							cam_eth.ethertype = 0x8864;
							//if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_ETHERTYPE, &cam_eth, &cam_rslt_idx))
							if(RTK_FC_RET_OK!=_rtk_fc_ethTypeEntry_acl_ref(cam_eth.ethertype, &cam_rslt_idx))
								cam_rslt_idx = 0;
							ca_aal_rule.key.ethertype_enc = cam_rslt_idx + 1;
							ca_aal_rule.key.msk_ethertype_enc = CA_L3_AAL_KEY_MSK_ENABLE;

							//Keep original due to L3FE Packet Editor cannot support the modification of PPPoE control packet
							ca_aal_rule.action.keep_orig_pkt_vld = 1;
							ca_aal_rule.action.keep_orig_pkt = 1;
							_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL CONTROL_PACKET_TRAP for PPPoE Control Packet(Step 2) failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						ACL_RSV("add RSV[%d] CONTROL_PACKET_TRAP @ %s", type, aal_idx_buf);
#endif
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_DHCPV4_TRAP:
					{
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);
						aal_idx_offset = 0;

						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							ca_aal_rule.key.ip_ver = 0; //0: ipv4, 1: ipv6
							ca_aal_rule.key.msk_ip_ver= CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.ip_protocol = 0x11;
							ca_aal_rule.key.msk_ip_protocol = CA_L3_AAL_KEY_MSK_ENABLE;

							ca_aal_rule.key.l4_vld = TRUE;
							ca_aal_rule.key.msk_l4_vld = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.l4_sp = 67;
							ca_aal_rule.key.l4_dp = 68;
							ca_aal_rule.key.msk_l4_port = 2;	//0:sp,dp;1:sp range;2:dp range;3:rng_match

							_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL DHCPV4_TRAP failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						ACL_RSV("add RSV[%d] DHCPV4_TRAP @ %s", type, aal_idx_buf);
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_ICMPV6_TRAP:
					{
						//ICMPv6,MLD,NDPv6 (prevent ipv6 mld drop by main hash unknown mc)
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);
						aal_idx_offset = 0;

						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							ca_aal_rule.key.ip_ver = 1;	//0: ipv4, 1: ipv6
							ca_aal_rule.key.msk_ip_ver= CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.ip_protocol = 0x3a;
							ca_aal_rule.key.msk_ip_protocol = CA_L3_AAL_KEY_MSK_ENABLE;

							_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL ICMPV6_TRAP failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						ACL_RSV("add RSV[%d] ICMPV6_TRAP @ %s", type, aal_idx_buf);
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_ICMPV4_TRAP:
					{
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);
						aal_idx_offset = 0;

						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							ca_aal_rule.key.ip_ver = 0;	//0: ipv4, 1: ipv6
							ca_aal_rule.key.msk_ip_ver= CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.ip_protocol = 0x1;
							ca_aal_rule.key.msk_ip_protocol = CA_L3_AAL_KEY_MSK_ENABLE;

							_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL ICMPV4_TRAP failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						ACL_RSV("add RSV[%d] ICMPV4_TRAP @ %s", type, aal_idx_buf);
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_L2TP_DATA_LOWER_PRIORITY:
					{
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
						//this rule should higher than PPPoE Control Packet(Step1), otherwise l2tp data with pppoe tag will not hit this rule.

						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);
						aal_idx_offset = 0;

						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							/*Set ACL rule for higher rx prioirty for L2TP controal packet and LCP packet(Wan port + UDP + Sport(1701) + Dport(1701) + pktLen < 128)
								Due to CA L3 CLS not support these pattern in one rule, so seperate as below two rules:
									Rule 1: CLS Type0, key=udp + special header(type=2, key=sport/dport 1701), action=trap to CPU+assign higher priority
									Rule 2: CLS Type1, key=udp + packet length range 129~9216, action=assign priority 0
								Target: trap all l2tp packet, and l2tp data packet use lower priority. others data packet recover their priority by flow*/
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							ca_aal_rule.key.ip_protocol = 0x11;
							ca_aal_rule.key.msk_ip_protocol = CA_L3_AAL_KEY_MSK_ENABLE;

							memset(&cam_pkt_len, 0, sizeof(cam_pkt_len));
							cam_pkt_len.vld = 1;
							cam_pkt_len.pkt_len_low = 129;
							cam_pkt_len.pkt_len_hi = 9216;
							if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_PKT_LEN, &cam_pkt_len, &cam_rslt_idx))
								cam_rslt_idx = 0;
							ca_aal_rule.key.pktlen_rng_match_vec = 1 << cam_rslt_idx;
							ca_aal_rule.key.pktlen_rng_match_mask = 0;

							//permit
							ca_aal_rule.action.cos = 0;
							ca_aal_rule.action.cos_sel = TRUE;

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL L2TP_DATA_LOWER_PRIORITY failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						ACL_RSV("add RSV[%d] L2TP_DATA_LOWER_PRIORITY @ %s", type, aal_idx_buf);
#endif
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_MULTICAST_DSLITE_TRAP:
					{
						//HW not support Dslite, prevent dslit drop by unknown drop, and not impact bridge, so only trap v6 multicast dslite.

						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);
						aal_idx_offset = 0;

						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							ca_aal_rule.key.ip_ver = 1;	//0: ipv4, 1: ipv6
							ca_aal_rule.key.msk_ip_ver= CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.ip_protocol = 4;	//ipv6 next header
							ca_aal_rule.key.msk_ip_protocol = CA_L3_AAL_KEY_MSK_ENABLE;

							//Trap multicast addresses in IPv6 use the prefix ffX/8.
#if defined(CONFIG_FC_G3_G3LITE_SERIES)	//FULL_KEY
							ca_aal_rule.key.msk_ip_da_l = 8;
							ca_aal_rule.key.msk_ip_da_h = 0;
							ca_aal_rule.key.ip_da.ip_addr_3 = 0xff000000; //ffxx:xxxx:xxx...
							ca_aal_rule.key.ip_da.ip_addr_0 = ca_aal_rule.key.ip_da.ip_addr_1 = ca_aal_rule.key.ip_da.ip_addr_2 = 0;
#else									//FLOW_KEY
							ca_aal_rule.key.msk_ip_addr_l = 8;
							ca_aal_rule.key.msk_ip_addr_h = 0;
							ca_aal_rule.key.ip_addr_type = 0;	//0: da, 1: sa
							ca_aal_rule.key.ip_addr.ip_addr_3 = 0xff000000;	//ffxx:xxxx:xxx...
							ca_aal_rule.key.ip_addr.ip_addr_0 = ca_aal_rule.key.ip_addr.ip_addr_1 = ca_aal_rule.key.ip_addr.ip_addr_2 = 0;
#endif

							_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL MULTICAST_DSLITE_TRAP failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						ACL_RSV("add RSV[%d] MULTICAST_DSLITE_TRAP @ %s", type, aal_idx_buf);
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_SMP_DISPATCH_NIC_RX_BY_ACL:
					{
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE) || defined(RTK_FC_MAINCPU_ONE_CPU_PORT)
						ACL_RSV("not support RSV[%d] SMP_DISPATCH_NIC_RX_BY_ACL", type);
#else

						//Dispatch traffic to different cpu by dip bit 0
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);
						aal_idx_offset = 0;

						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_TAIL;
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;	//replace default ldpid only when not hit any user acl
#else
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_2;	//replace default ldpid
							ca_aal_rule.key.ip_addr_ctrl = 0;
#endif
							ca_aal_rule.key.msk_ip_da_l = 1;	//check 1 bit
							ca_aal_rule.key.msk_ip_da_h = 2;	//subnet to be compared is chosen on the righ hand side
							ca_aal_rule.key.ip_da.ip_addr_0 = 0x1;	//last bit is odd
							ca_aal_rule.key.ip_da.ip_addr_1 = ca_aal_rule.key.ip_da.ip_addr_2 = ca_aal_rule.key.ip_da.ip_addr_3 = 0;

							ca_aal_rule.action.mc = 0; //1:mcgid=mcgid;0:mcgid=ldpid
							if(ca_aal_profile_idx == CA_L3_AAL_CLS_PROFILE_WAN)
								ca_aal_rule.action.mcgid = RTK_FC_MAC_PORT_CPU1+2;
							else
								ca_aal_rule.action.mcgid = RTK_FC_MAC_PORT_CPU+2;
							ca_aal_rule.action.dpid_pri = 1;
							ca_aal_rule.action.permit = 1;
							ca_aal_rule.action.dpid_vld = 1;

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL SMP_DISPATCH_NIC_RX_BY_ACL failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						ACL_RSV("add RSV[%d] SMP_DISPATCH_NIC_RX_BY_ACL @ %s", type, aal_idx_buf);
						if(fc_db.controlFuc.acl_multiple_hit_cfg == 1)
							ACL_RSV("\t SMP_DISPATCH_NIC_RX_BY_ACL may not work due to acl_multiple_hit_cfg is enable.");
#endif
					}
					break;

				case RTK_FC_ACLANDCF_RESERVED_TTL_LESS_ONE_TRAP:
#if !defined(CONFIG_FC_G3_G3LITE_SERIES)					
					{	//[8277C] bug fix Hash flow hit but TTL-1=0 TRAP cause mdata/pol_id reset
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_l3_cls_key_clean(&ca_aal_rule.key);
						aal_idx_offset = 0;

						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							aal_l3_cls_key_clean(&ca_aal_rule.key);
							memset(&ca_aal_rule.action, 0, sizeof(cls_action_t));

							ca_aal_rule.key.msk_mac_da_an_hit = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.mac_da_an_hit = 1;

							ca_aal_rule.key.msk_ip_ttl_eq1 = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.ip_ttl_eq1 = 1;

							_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL TTL_LESS_ONE_TRAP failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));

							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							aal_l3_cls_key_clean(&ca_aal_rule.key);
							memset(&ca_aal_rule.action, 0, sizeof(cls_action_t));

							ca_aal_rule.key.msk_mac_da_an_hit = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.mac_da_an_hit = 1;

							ca_aal_rule.key.msk_ip_ttl_eq1 = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.ip_ttl_eq1 = 0;

							_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL TTL_LESS_ONE_TRAP failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));

						}

						ACL_RSV("add RSV[%d] TTL_LESS_ONE_TRAP @ %s", type, aal_idx_buf);
					}
#endif				
					break;

				case RTK_FC_ACLANDCF_RESERVED_MC_JUMBO_DROP:
					{	//[8277C] workaround jumbo mc packet to mc port will cause hw stuck
						memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
						aal_idx_offset = 0;

						ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
						CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							aal_l3_cls_key_clean(&ca_aal_rule.key);
							memset(&ca_aal_rule.action, 0, sizeof(cls_action_t));

							ca_aal_rule.key.msk_mac_da_ip_mc = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.mac_da_ip_mc = 1;	//((MAC DA[47:24] == 01-00-5E) && (etherType == IPV4)) | ((MAC DA[47:32] == 33-33) && (etherType == IPV6))
							ca_aal_rule.key.msk_ip_vld = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.ip_vld = 1;

							memset(&cam_pkt_len, 0, sizeof(cam_pkt_len));
							cam_pkt_len.vld = 1;
							cam_pkt_len.pkt_len_low = 12000;
							cam_pkt_len.pkt_len_hi = 16383;
							if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_PKT_LEN, &cam_pkt_len, &cam_rslt_idx))
								cam_rslt_idx = 0;
							ca_aal_rule.key.pktlen_rng_match_vec = 1 << cam_rslt_idx;
							ca_aal_rule.key.pktlen_rng_match_mask = 0;

							ca_aal_rule.action.permit = 0;
							ca_aal_rule.action.dpid_vld = 1;
							ca_aal_rule.action.t2_ctrl_vld = 1;
							ca_aal_rule.action.t2_ctrl = 0xf;

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL MC_JUMBO_DROP failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));

							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_TAIL;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

							aal_l3_cls_key_clean(&ca_aal_rule.key);
							memset(&ca_aal_rule.action, 0, sizeof(cls_action_t));
							ca_aal_rule.key.msk_mac_da_an_hit = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.mac_da_an_hit = 1;

							ca_aal_rule.key.msk_ip_vld = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.ip_vld = 1;
							ca_aal_rule.key.msk_ip_da_mc = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.ip_da_mc = 1;	//IP DA is in MC IP range, IPv4: ip_da[31:28] == 4'b1110; IPv6: ip_da[127:120] == 8'hff

							memset(&cam_pkt_len, 0, sizeof(cam_pkt_len));
							cam_pkt_len.vld = 1;
							cam_pkt_len.pkt_len_low = 12000;
							cam_pkt_len.pkt_len_hi = 16383;
							if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_PKT_LEN, &cam_pkt_len, &cam_rslt_idx))
								cam_rslt_idx = 0;
							ca_aal_rule.key.pktlen_rng_match_vec = 1 << cam_rslt_idx;
							ca_aal_rule.key.pktlen_rng_match_mask = 0;

							ca_aal_rule.action.permit = 0;
							ca_aal_rule.action.dpid_vld = 1;
							ca_aal_rule.action.t2_ctrl_vld = 1;
							ca_aal_rule.action.t2_ctrl = 0xf;

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, type))
							{
								ACL_RSV("add RSV_ACL MC_JUMBO_DROP failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
						}

						ACL_RSV("add RSV[%d] MC_JUMBO_DROP @ %s", type, aal_idx_buf);
					}
					break;

				default:
					break;
			}
		}
	}

	if(addRuleFailedFlag==1)
		return (RT_ERR_RG_FAILED);

	if(fc_db.controlFuc.bridge_5tuple_flow_accelerate_by_2tuple){
		ACL_RSV("+++> RSV[%d] 5TUPLE_FLOW_ACCELERATE_BY_2TUPLE ", RTK_CA_CLS_TYPE_BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE);
	}
#if defined(CONFIG_RTK_FC_HTTP_OFFLOAD_BY_PE) && defined(CONFIG_REALTEK_IPC2RCPU)
	if(fc_db.http_test_cls_idx[0] != FAIL){
		ACL_RSV("+++> RSV[%d] HTTP_ACCELERATE_BY_PE ", RTK_CLS_TYPE_HTTP_ACCELERATE_BY_PE);
	}
#endif
	if(fc_db.aclAndCfReservedRule.reservedMask[RTK_CLS_TYPE_SRV6_ACCELERATE_BY_PE] == ENABLED){
		ACL_RSV("+++> RSV[%d] SRV6_ACCELERATE_BY_PE ", RTK_CLS_TYPE_SRV6_ACCELERATE_BY_PE);
	}

	return (RT_ERR_RG_OK);

}

int _rtk_fc_aclAndCfReservedRuleReflash(void)
{
	//using ACL[0] for multicast
	_rtk_fc_acl_for_multicast_temp_protection_add();

	if((fc_db.wanPortMask.portmask != 0x0)
		&& (fc_db.aclAndCfReservedRule.acl_wanPortMask != fc_db.wanPortMask.portmask))
	{
#if defined(CONFIG_FC_CA8277AB_SERIES)
		fc_db.aclAndCfReservedRule.reservedMask[RTK_CA_CLS_TYPE_L2_INGRESS_FORWARD_L3FE]=ENABLED;
#else
		fc_db.aclAndCfReservedRule.reservedMask[RTK_CLS_TYPE_L2_EGRESS_FORWARD_L3FE]=ENABLED;
#endif
	}

	_rtk_fc_aclAndCfReservedRuleHeadReflash();
	_rtk_fc_aclAndCfReservedRuleTailReflash();

	_rtk_fc_acl_for_multicast_temp_protection_del();

	return RT_ERR_RG_OK;
}

int _rtk_rg_aclAndCfReservedRuleAdd(rtk_rg_aclAndCf_reserved_type_t rsvType, void *parameter)
{
	ACL_RSV("#####Reserved ACL reflash!(add reserved ACL rsvType=%d)#####",rsvType);

	if((rsvType < RTK_FC_ACLANDCF_RESERVED_TAIL_END) && (fc_db.aclAndCfReservedRule.reservedMask[rsvType]==ENABLED))
		return (RT_ERR_RG_OK);

	switch(rsvType){

		case RTK_CA_CLS_TYPE_L2_INGRESS_FORWARD_L3FE:
			fc_db.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_FC_ACLANDCF_RESERVED_ALL_TRAP:
			assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(2,0));
			fc_db.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_FC_ACLANDCF_RESERVED_SNAP_OTHER_TRAP:
			/* fall through */
		case RTK_FC_ACLANDCF_RESERVED_WIFI_SNAP_TRANS:
			/* fall through */
#if defined(CONFIG_RTK_FC_WIFI_AMSDU_OFFLOAD_BY_PE)
		case RTK_FC_ACLANDCF_RESERVED_WIFI_TXAMSDU_HW_LOOKUP:
			/* fall through */
		case RTK_FC_ACLANDCF_RESERVED_WIFI_TXAMSDU_MC_TO_UC:
			/* fall through */
#endif
		case RTK_FC_ACLANDCF_RESERVED_ACL_LAN_PORTMASK_RESET:
			/* fall through */
		case RTK_FC_ACLANDCF_RESERVED_SMALL_TCP_ACK_TRAP:
			/* fall through */
		case RTK_FC_ACLANDCF_RESERVED_MULTICAST_SSDP_TRAP:
			/* fall through */
		case RTK_FC_ACLANDCF_RESERVED_MULTICAST_RIP_TRAP:
			assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(1,0));//reserved one more for MC temporary permit while ACL rearrange
			fc_db.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_FC_ACLANDCF_RESERVED_IPSEC_ACCELERATE_BY_PE:
			memcpy(&fc_db.aclAndCfReservedRule.ipsec_accelerate_by_pe,(rtk_fc_aclAndCf_reserved_ipsec_accelerate_by_pe_t*)parameter,sizeof(rtk_fc_aclAndCf_reserved_ipsec_accelerate_by_pe_t));
			assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(1,0));
			fc_db.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_FC_ACLANDCF_RESERVED_BURST_PACKET_SEND_MCE:
			assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(1,0));
			memcpy(&fc_db.aclAndCfReservedRule.burstPktSend_conf,(rtk_fc_burstPktSend_conf_t*)parameter,sizeof(rtk_fc_burstPktSend_conf_t));
			fc_db.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_FC_ACLANDCF_RESERVED_PPPoE_LCP_PACKET_ASSIGN_PRIORITY:
			assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&fc_db.aclAndCfReservedRule.pppoe_lcp_assign_prioity,(rtk_fc_aclAndCf_reserved_assign_priority_t*)parameter,sizeof(rtk_fc_aclAndCf_reserved_assign_priority_t));
			fc_db.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_FC_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY:
			assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&fc_db.aclAndCfReservedRule.l2tp_ctrl_lcp_assign_prioity,(rtk_fc_aclAndCf_reserved_assign_priority_t*)parameter,sizeof(rtk_fc_aclAndCf_reserved_assign_priority_t));
			fc_db.aclAndCfReservedRule.reservedMask[RTK_FC_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY]=ENABLED;
			/* fall through */
		case RTK_FC_ACLANDCF_RESERVED_L2TP_DATA_LOWER_PRIORITY:
			assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(2,0));
			fc_db.aclAndCfReservedRule.reservedMask[RTK_FC_ACLANDCF_RESERVED_L2TP_DATA_LOWER_PRIORITY]=ENABLED;
			break;

		case RTK_FC_ACLANDCF_RESERVED_PPTP_GRE_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY:
			assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(2,0));
			memcpy(&fc_db.aclAndCfReservedRule.pptp_gre_ctrl_lcp_assign_prioity,(rtk_fc_aclAndCf_reserved_assign_priority_t*)parameter,sizeof(rtk_fc_aclAndCf_reserved_assign_priority_t));
			fc_db.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_FC_ACLANDCF_RESERVED_SMP_DISPATCH_NIC_RX_BY_ACL:
			assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(2,0));
			fc_db.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_FC_ACLANDCF_RESERVED_MULTICAST_DSLITE_TRAP:
			assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(2,0));
			fc_db.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_FC_ACLANDCF_RESERVED_ICMPV4_TRAP:
			/* fall through */
		case RTK_FC_ACLANDCF_RESERVED_ICMPV6_TRAP:
			/* fall through */
		case RTK_FC_ACLANDCF_RESERVED_DHCPV4_TRAP:
			assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(2,0));
			fc_db.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_FC_ACLANDCF_RESERVED_CONTROL_PACKET_TRAP:
			assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(8,0));
			fc_db.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		case RTK_FC_ACLANDCF_RESERVED_MC_JUMBO_DROP:
			/* fall through */
		case RTK_FC_ACLANDCF_RESERVED_TTL_LESS_ONE_TRAP:
			/* fall through */
		case RTK_FC_ACLANDCF_RESERVED_MULTICAST_TRAP_AND_GLOBAL_SCOPE_PERMIT:
			assert_ok(_rtk_fc_aclAndCfReservedRuleAddCheck(4,0));
			fc_db.aclAndCfReservedRule.reservedMask[rsvType]=ENABLED;
			break;

		default:
			break;
	}

	_rtk_fc_aclAndCfReservedRuleReflash();


	return (RT_ERR_RG_OK);
}

int _rtk_rg_aclAndCfReservedRuleDel(rtk_rg_aclAndCf_reserved_type_t rsvType)
{
	ACL_RSV("#####Reserved ACL reflash!(del reserved ACL rsvType=%d)#####",rsvType);

	if(fc_db.aclAndCfReservedRule.reservedMask[rsvType]==DISABLED)
		return (RT_ERR_RG_OK);

	switch(rsvType){

		case RTK_FC_ACLANDCF_RESERVED_ACL_LAN_PORTMASK_RESET:
			ASSERT_EQ(_rtk_rg_aclAndCfReservedRuleAddSpecial(RTK_CLS_TYPE_L2_INGRESS_FORWARD_PORT, NULL), CA_E_OK);
			break;

		case RTK_FC_ACLANDCF_RESERVED_L2TP_CONTROL_LCP_PACKET_TRAP_AND_ASSIGN_PRIORITY:
		case RTK_FC_ACLANDCF_RESERVED_L2TP_DATA_LOWER_PRIORITY:
			fc_db.aclAndCfReservedRule.reservedMask[RTK_FC_ACLANDCF_RESERVED_L2TP_DATA_LOWER_PRIORITY]=DISABLED;
			break;

		case RTK_FC_ACLANDCF_RESERVED_ICMPV6_TRAP:
			WARNING("Reserved ACL[%d] ICMPV6_TRAP should not disable to prevent ipv6 mld drop by unknown mc.",rsvType);
			return (RT_ERR_RG_FAILED);
			break;

		default:
			break;
	}

	fc_db.aclAndCfReservedRule.reservedMask[rsvType]=DISABLED;

	_rtk_fc_aclAndCfReservedRuleReflash();

	return (RT_ERR_RG_OK);
}

int _rtk_rg_aclAndCfReservedRuleAddSpecial(rtk_rg_aclAndCf_reserved_type_t rsvType, void *parameter)
{
	l3_cls_rule_t ca_aal_rule;
	uint8 ca_aal_profile_idx=0;
	uint8 ca_aal_profile_mask=0;
	uint8 care_lspid=0;
	uint32_t ca_aal_rslt_idx=0;
	uint32 i=0;
	int addRuleFailedFlag=0;
	rtk_portmask_t activePorts;
	aal_l2_cls_rule_t l2_cls_rule;
	uint8 aal_idx_offset=0;
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
	char aal_idx_buf[128];
	ca_classifier_rule_t ca_cls_rule;
#elif defined(CONFIG_FC_CA8277B_SERIES)
	char aal_idx_buf[64];
	uint32 loop_idx, loop_start, loop_end, loop_mac;
	rtk_rg_asic_netif_ref_t asicNetif;
	uint32_t cam_rslt_idx=0, cam_rslt_idx2;
	l3_cam_pkt_len_tbl_entry_t cam_pkt_len;
	rtk_fc_aclAndCf_reserved_vxlan_ffd_t vxlan_ffd;
	uint32 address[4];
	rtk_fc_aclAndCf_reserved_accelerate_by_pe_t acc_by_pe;
#elif defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
	char aal_idx_buf[64];
	rtk_fc_aclAndCf_reserved_dual_hdr_chk_t dual_hdr_chk;
	rtk_fc_aclAndCf_reserved_dual_hdr_vlan_t dual_hdr_vlan;
	uint32 address[4];
	rtk_fc_aclAndCf_reserved_accelerate_by_pe_t acc_by_pe;
	rtk_fc_aclAndCf_reserved_srv6_accelerate_by_pe_t srv6_acc_by_pe;
#else
	char aal_idx_buf[64];
#endif

	ACL_RSV("#####Add special ACL!(rsvType=%d)#####",rsvType);

	memset(aal_idx_buf, 0, sizeof(aal_idx_buf));

	switch(rsvType){

		case RTK_CA_CLS_TYPE_L2_INGRESS_FORWARD_L3FE:
#if defined(CONFIG_FC_CA8277AB_SERIES)
			//Create L2 igr Classifier for forward the packet to L3FE - only need to care physical port and wifi port 0x12~0x16
			bzero(&activePorts,sizeof(rtk_portmask_t));
	
			activePorts.bits[0] = RTK_FC_ALL_MAC_PORTMASK_WITHOUT_CPU | RTK_FC_ALL_MAC_WLANCPU_PORTMASK | ((1<<RTK_FC_MAC_PORT_WLAN_CPU2) | (1<<RTK_FC_MAC_PORT_WLAN_CPU3)| (1<<RTK_FC_MAC_PORT_WLAN_CPU4)| (1<<RTK_FC_MAC_PORT_WLAN_CPU5));
			if(fc_db.controlFuc.loopbackMode_is_enabled)
			{
				activePorts.bits[0] = activePorts.bits[0] & (~(1<<G3_LOOPBACK_P_NEWSPA));
				activePorts.bits[0] = activePorts.bits[0] & (~(G3_LOOPBACK_P_MASK)); //packet from LAN port=>go to loopback port directly.
			}

			aal_idx_offset = 0;
			memset(&l2_cls_rule, 0, sizeof(aal_l2_cls_rule_t));
			CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
				l2_cls_rule.priority = RTK_CA_CLS_PRIORITY_L2_0;
				l2_cls_rule.port = i;

				l2_cls_rule.action.dp_valid = TRUE;
				if((0x1<<i) & fc_db.wanPortMask.portmask)
#if defined(CONFIG_FC_CA8277B_SERIES)
					l2_cls_rule.action.dp_value = fc_db.controlFuc.p7_rxsel_l3fe ? AAL_LPORT_CPU_1 : AAL_LPORT_L3_WAN;
#else
					l2_cls_rule.action.dp_value = AAL_LPORT_L3_WAN;
#endif
				else
					l2_cls_rule.action.dp_value = AAL_LPORT_L3_LAN;

				if(_rtk_fc_g3_caAalClsL2RuleAdd(&l2_cls_rule, &ca_aal_rslt_idx, rsvType)) {
					ACL_RSV("add special ACL RTK_CA_CLS_TYPE_L2_INGRESS_FORWARD_L3FE failed!!! port %d", i);
					addRuleFailedFlag = 1;
					break;
				}
				aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%d-%d,", L2_CLS_TBL_ID(ca_aal_rslt_idx), L2_CLS_KEY_ID(ca_aal_rslt_idx));
			}
			fc_db.aclAndCfReservedRule.acl_wanPortMask = fc_db.wanPortMask.portmask;
			ACL_RSV("add special L2_INGRESS_FORWARD_L3FE @ L2[%s]", aal_idx_buf);
#else
			memset(&l2_cls_rule, 0, sizeof(aal_l2_cls_rule_t));
			/* L2 ingress CLS is patch only
			portmask = 0xFF;
			RTK_8277C_SCAN_PORTMASK(port, portmask){
				if(((1ULL<<port) & portmask) == 0x0)
					continue;

				memset(&l2_cls_rule, 0, sizeof(aal_l2_cls_rule_t));

				//rule basic information
				l2_cls_rule.port = port;
				l2_cls_rule.priority = 0;	//key: priority 0

				//action: permit, dpid
				l2_cls_rule.action.dp_valid = TRUE;
				if(port == RTK_FC_MAC_PORT_PON)
					l2_cls_rule.action.dp_value = AAL_LPORT_L3_WAN;
				else
					l2_cls_rule.action.dp_value = AAL_LPORT_L3_LAN;

				ret = aal_l2_cls_add(RTK_ASIC_DEVID, &l2_cls_rule, &l2_rslt_info);
				if(ret != CA_E_OK)
					continue;
				//DEBUG("L2 CLS port %d, ret %d, rslt_info %d-%d", port, ret, L2_CLS_TBL_ID(l2_rslt_info), L2_CLS_KEY_ID(l2_rslt_info));
			}
			*/
#endif
			break;

		case RTK_CA_CLS_TYPE_GENERIC_INTF_HASH_PROFILE_DECISION:
			{
				rtk_fc_aclAndCf_reserved_generic_intf_mirrored_cfg_t *cls_mir_cfg = (parameter != NULL)?parameter:NULL;
				memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));

				/*
				 * Default rules of Wan/Lan profile
				 */

				ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
				CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
					aal_idx_offset = 0;

					/** mc rule **/
					ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_9;
					ca_aal_rule.rslt_type = CL_RSLT_TYPE_0;

					aal_l3_cls_key_clean(&ca_aal_rule.key);
					memset(&ca_aal_rule.action, 0, sizeof(cls_action_t));

					ca_aal_rule.key.msk_mac_da_ip_mc = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.mac_da_ip_mc = 1;	//((MAC DA[47:24] == 01-00-5E) && (etherType == IPV4)) | ((MAC DA[47:32] == 33-33) && (etherType == IPV6))
					ca_aal_rule.key.msk_ip_vld = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.ip_vld = 1;

					if((cls_mir_cfg != NULL) && (cls_mir_cfg->cls_mir_en))
						ca_aal_rule.action.mrr_en = 1;
					ca_aal_rule.action.t2_ctrl = CLS_T2CTRL_MAINHASH_PROFILE_MC;
					_rtk_fc_g3_caAalClsAssignHashProfileAction(&ca_aal_rule, ca_aal_profile_idx, FALSE, 0);

					if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, rsvType))
					{
						ACL_RSV("add special ACL GENERIC_INTF_HASH_PROFILE_DECISION failed!!!");
						addRuleFailedFlag = 1;
						break;
					}
					aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%d-%d,", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));

					/** mc rule **/
					ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_9;	//reuse same entry with ca mc rule
					ca_aal_rule.rslt_type = CL_RSLT_TYPE_0;

					aal_l3_cls_key_clean(&ca_aal_rule.key);
					memset(&ca_aal_rule.action, 0, sizeof(cls_action_t));
					ca_aal_rule.key.msk_mac_da_an_hit = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.mac_da_an_hit = 1;

					ca_aal_rule.key.msk_ip_vld = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.ip_vld = 1;
					ca_aal_rule.key.msk_ip_da_mc = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.ip_da_mc = 1;	//IP DA is in MC IP range,  IPv4: ip_da[31:28] == 4'b1110; IPv6: ip_da[127:120] == 8'hff

					if((cls_mir_cfg != NULL) && (cls_mir_cfg->cls_mir_en))
						ca_aal_rule.action.mrr_en = 1;
					ca_aal_rule.action.t2_ctrl = CLS_T2CTRL_MAINHASH_PROFILE_MC;
					_rtk_fc_g3_caAalClsAssignHashProfileAction(&ca_aal_rule, ca_aal_profile_idx, FALSE, 0);

					if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, rsvType))
					{
						ACL_RSV("add special ACL GENERIC_INTF_HASH_PROFILE_DECISION failed!!!");
						addRuleFailedFlag = 1;
						break;
					}
					aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%d-%d,", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));

#if defined(CONFIG_FC_G3_G3LITE_SERIES)
					//[8277A] need Hash to check DOS Attack Prevent and do policing if necessary: syn flood deny
					/** tcp_flags with syn **/
					ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_8;
					ca_aal_rule.rslt_type = CL_RSLT_TYPE_0;

					aal_l3_cls_key_clean(&ca_aal_rule.key);
					memset(&ca_aal_rule.action, 0, sizeof(cls_action_t));

					ca_aal_rule.key.msk_ip_vld = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.ip_vld = 1;
					ca_aal_rule.key.msk_ip_protocol = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.ip_protocol = 0x6;
					ca_aal_rule.key.msk_tcp_rdp_ctrl = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.tcp_rdp_ctrl = 0x2;	//CLS only support has syn, and hash will check syn only

					if((cls_mir_cfg != NULL) && (cls_mir_cfg->cls_mir_en))
						ca_aal_rule.action.mrr_en = 1;
					ca_aal_rule.action.t2_ctrl = CLS_T2CTRL_MAINHASH_PROFILE_DOS_POL;
					_rtk_fc_g3_caAalClsAssignHashProfileAction(&ca_aal_rule, ca_aal_profile_idx, FALSE, 0);

					if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, rsvType))
					{
						ACL_RSV("add special ACL GENERIC_INTF_HASH_PROFILE_DECISION failed!!!");
						addRuleFailedFlag = 1;
						break;
					}
					aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%d-%d,", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
#endif
#if defined(CONFIG_FC_CA8277B_SERIES)
					if(fc_db.controlFuc.special_fast_forward_mode==1)
					{
						/** with L4 header (UC5 tuple) for vxlan fast forward **/
						if(ca_aal_profile_idx==CA_L3_AAL_CLS_PROFILE_LAN){
							int j = 0;
							
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_9;
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_0;

							aal_l3_cls_key_clean(&ca_aal_rule.key);
							memset(&ca_aal_rule.action, 0, sizeof(cls_action_t));

							ca_aal_rule.key.msk_o_lspid = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.o_lspid = RTK_FC_MAC_PORT_SPECIAL_FF_DS;
							ca_aal_rule.key.msk_ip_da_mc = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.ip_da_mc = 0;//non multicast
							ca_aal_rule.key.msk_l4_vld = CA_L3_AAL_KEY_MSK_ENABLE;	// tcp udp rdp
							ca_aal_rule.key.l4_vld = 1;

							if((cls_mir_cfg != NULL) && (cls_mir_cfg->cls_mir_en))
								ca_aal_rule.action.mrr_en = 1;
							ca_aal_rule.action.t2_ctrl = CLS_T2CTRL_MAINHASH_PROFILE_WAN;
							//vxlan ds port 0x17 should use hash wan profile
							_rtk_fc_g3_caAalClsAssignHashProfileAction(&ca_aal_rule, ca_aal_profile_idx, FALSE, 0);
							for(j = RTK_FC_MAC_PORT_SPECIAL_FASTFWD_CPU_PORT_0 ; j <= RTK_FC_MAC_PORT_SPECIAL_FASTFWD_CPU_PORT_3 ; j++)
							{
								ca_aal_rule.key.o_lspid = j;
								if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, rsvType))
								{
									ACL_RSV("add special ACL GENERIC_INTF_HASH_PROFILE_DECISION failed!!!");
									addRuleFailedFlag = 1;
									break;
								}
								aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%d-%d,", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
							}

						}
					}
					
#endif
#if defined(CONFIG_FC_RTL8277C_SERIES)
					//[8277C] TCP rst/fin should keep correct crc16 by use different hash profile
					ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_2;	//should lower than RTK_CA_CLS_TYPE_BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE, higher than default hash selection
					ca_aal_rule.rslt_type = CL_RSLT_TYPE_0;

					aal_l3_cls_key_clean(&ca_aal_rule.key);
					memset(&ca_aal_rule.action, 0, sizeof(cls_action_t));

					ca_aal_rule.key.msk_ip_vld = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.ip_vld = 1;
					ca_aal_rule.key.msk_ip_protocol = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.ip_protocol = 0x6;
					ca_aal_rule.key.msk_tcp_rdp_ctrl = 1;	//1: OR Match, 2: AND Match
					ca_aal_rule.key.tcp_rdp_ctrl = 0x5; 	//tcp flags with rst or fin

					if((cls_mir_cfg != NULL) && (cls_mir_cfg->cls_mir_en))
						ca_aal_rule.action.mrr_en = 1;
					ca_aal_rule.action.t2_ctrl = RTK_ASIC_FLOW_PROFILE_FLOW_5TUPLE_TCP_FLAG0;
					/* TCP FIN/RST match exist flow will cause hash double check failed.
					In this condition, mdata reason will not be updated by hash. Use CLS to update flow miss reason. */
					ca_aal_rule.action.mdata_w_vld_0 = 0x4; // reason: mdata0[11:8]
					ca_aal_rule.action.mdata_w_0 = CPU_REASON_FLOWMISS << RXINFO_REF_HW_RSN_SHIFT_BIT;// resson3: flow miss
					ca_aal_rule.action.keep_orig_pkt = 1;
					ca_aal_rule.action.keep_orig_pkt_vld = 1;
					_rtk_fc_g3_caAalClsAssignHashProfileAction(&ca_aal_rule, ca_aal_profile_idx, FALSE, 0);
					// hw resson (02): acl trap. (cls hit && ldpid = 0x10 && keep_orig_pkt = 1), so keep flow miss reason by using mcgid=0x11
					ca_aal_rule.action.mcgid = RTK_FC_MAC_PORT_CPU1;

					if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, rsvType))
					{
						ACL_RSV("add special ACL GENERIC_INTF_HASH_PROFILE_DECISION failed!!!");
						addRuleFailedFlag = 1;
						break;
					}
					aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%d-%d,", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
#endif

					bzero(&activePorts,sizeof(rtk_portmask_t));
#if defined(CONFIG_FC_G3_G3LITE_SERIES)
					if(ca_aal_profile_idx==CA_L3_AAL_CLS_PROFILE_WAN){
						activePorts.bits[0] = fc_db.wanPortMask.portmask;
					}else if(ca_aal_profile_idx==CA_L3_AAL_CLS_PROFILE_LAN){
						//for uuc per port control - t4_ctrl (per port rule for lan port, one rule for all wlan)
						activePorts.bits[0] = fc_db.lanPortMask.portmask|(1<<RTK_FC_MAC_PORT_WLAN_CPU0);
					}
					care_lspid = TRUE;
#elif defined(CONFIG_FC_RTL8277C_SERIES)
					if((ca_aal_profile_idx==CA_L3_AAL_CLS_PROFILE_WAN) && 
							fc_db.controlFuc.p7_rxsel_l3fe &&
							(fc_db.wanPortMask.portmask == (1<< RTK_FC_MAC_PORT_PON))) {
						activePorts.bits[0] = fc_db.wanPortMask.portmask | (1<<RTK_FC_MAC_PORT_PON_LSPID_TO_L2FE);						
						care_lspid = TRUE;
					
					} else {
						activePorts.bits[0] = 1;
						care_lspid = FALSE;
					}
#else
					activePorts.bits[0] = 1;	//uuc control per port loop is unnecessary in this chip
					care_lspid = FALSE;
#endif
					CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
						/** with L4 header (UC5 tuple) **/
						if(care_lspid && (i<RTK_FC_MAC_PORT_CPU))
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_2;
						else
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_1;
						ca_aal_rule.rslt_type = CL_RSLT_TYPE_0;

						aal_l3_cls_key_clean(&ca_aal_rule.key);
						memset(&ca_aal_rule.action, 0, sizeof(cls_action_t));

						if(care_lspid && (i<RTK_FC_MAC_PORT_CPU)){
							ca_aal_rule.key.msk_o_lspid = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.o_lspid = i;
						}
						ca_aal_rule.key.msk_ip_da_mc = CA_L3_AAL_KEY_MSK_ENABLE;
						ca_aal_rule.key.ip_da_mc = 0;//non multicast
#if 0
						ca_aal_rule.key.msk_l4_vld = CA_L3_AAL_KEY_MSK_ENABLE;	// tcp udp rdp
						ca_aal_rule.key.l4_vld = 1;
#else
						ca_aal_rule.key.msk_ip_vld = CA_L3_AAL_KEY_MSK_ENABLE;
						ca_aal_rule.key.ip_vld = 1;
#endif
						if((cls_mir_cfg != NULL) && (cls_mir_cfg->cls_mir_en))
							ca_aal_rule.action.mrr_en = 1;
						if(ca_aal_profile_idx==CA_L3_AAL_CLS_PROFILE_WAN)
							ca_aal_rule.action.t2_ctrl = CLS_T2CTRL_MAINHASH_PROFILE_WAN;
						else
							ca_aal_rule.action.t2_ctrl = CLS_T2CTRL_MAINHASH_PROFILE_LAN; 
						_rtk_fc_g3_caAalClsAssignHashProfileAction(&ca_aal_rule, ca_aal_profile_idx, care_lspid, i);
#if defined(CONFIG_FC_RTL8277C_SERIES)
						if((ca_aal_profile_idx==CA_L3_AAL_CLS_PROFILE_WAN) && 
								fc_db.controlFuc.p7_rxsel_l3fe &&
								(fc_db.wanPortMask.portmask == (1<< RTK_FC_MAC_PORT_PON))) {
							if(i == RTK_FC_MAC_PORT_PON) {
								ca_aal_rule.action.deepq = 1;		/*L3FE 1st round, o_lspid = 0x7*/									
								ca_aal_rule.action.lspid_vld = 1;
								ca_aal_rule.action.lspid = RTK_FC_MAC_PORT_PON_LSPID_TO_L2FE;
							}else {
								ca_aal_rule.action.deepq = 0;		/*L3FE 2nd round, o_lspid = 0x8*/
							}
						}
#endif

						if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, rsvType))
						{
							ACL_RSV("add special ACL GENERIC_INTF_HASH_PROFILE_DECISION failed!!!");
							addRuleFailedFlag = 1;
							break;
						}			
						aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%d-%d,", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));

						/** without L4 header (UC2 tuple) **/
						if(care_lspid && (i<RTK_FC_MAC_PORT_CPU))
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_1;
						else
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_0;
						ca_aal_rule.rslt_type = CL_RSLT_TYPE_0;

						aal_l3_cls_key_clean(&ca_aal_rule.key);
						memset(&ca_aal_rule.action, 0, sizeof(cls_action_t));

						// don't care ip version, L4 (tcp udp rdp) ..., care spa only - default cls rule per interface
						if(care_lspid && (i<RTK_FC_MAC_PORT_CPU)){
							ca_aal_rule.key.msk_o_lspid = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.o_lspid = i;
						}

						if((cls_mir_cfg != NULL) && (cls_mir_cfg->cls_mir_en))
							ca_aal_rule.action.mrr_en = 1;
						ca_aal_rule.action.t2_ctrl = CLS_T2CTRL_MAINHASH_PROFILE_2TUPLE_BRIDGE;
						_rtk_fc_g3_caAalClsAssignHashProfileAction(&ca_aal_rule, ca_aal_profile_idx, care_lspid, i);
#if defined(CONFIG_FC_RTL8277C_SERIES)
						if((ca_aal_profile_idx==CA_L3_AAL_CLS_PROFILE_WAN) && 
								fc_db.controlFuc.p7_rxsel_l3fe &&
								(fc_db.wanPortMask.portmask == (1<< RTK_FC_MAC_PORT_PON))) {
							if(i == RTK_FC_MAC_PORT_PON) {
								ca_aal_rule.action.deepq = 1;		/*L3FE 1st round, o_lspid = 0x7*/									
								ca_aal_rule.action.lspid_vld = 1;
								ca_aal_rule.action.lspid = RTK_FC_MAC_PORT_PON_LSPID_TO_L2FE;
							}else {
								ca_aal_rule.action.deepq = 0;		/*L3FE 2nd round, o_lspid = 0x8*/
							}
						}
#endif
						
						if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, rsvType))
						{
							ACL_RSV("add special ACL GENERIC_INTF_HASH_PROFILE_DECISION failed!!!");
							addRuleFailedFlag = 1;
							break;
						}
						aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%d-%d,", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
					}
					ACL_RSV("add special HASH_PROFILE_DECISION @ %s[%s]", ca_aal_profile_idx?"LAN":"WAN", aal_idx_buf);
				}

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
				if((cls_mir_cfg != NULL) && (cls_mir_cfg->mirror_en == TRUE))
				{
					uint8 mirror_port = cls_mir_cfg->mirrord_port;

					ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
					CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {

						/** mc rule **/
						ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_MC_MIRROR;
						ca_aal_rule.rslt_type = CL_RSLT_TYPE_0;

						aal_l3_cls_key_clean(&ca_aal_rule.key);
						memset(&ca_aal_rule.action, 0, sizeof(cls_action_t));

						ca_aal_rule.key.msk_o_lspid = CA_L3_AAL_KEY_MSK_ENABLE;
						ca_aal_rule.key.o_lspid = mirror_port;
						ca_aal_rule.key.ip_vld = 1;
						ca_aal_rule.key.msk_mac_da_ip_mc = CA_L3_AAL_KEY_MSK_ENABLE;
						ca_aal_rule.key.mac_da_ip_mc = 1;	//((MAC DA[47:24] == 01-00-5E) && (etherType == IPV4)) | ((MAC DA[47:32] == 33-33) && (etherType == IPV6))
						ca_aal_rule.key.msk_ip_vld = CA_L3_AAL_KEY_MSK_ENABLE;
						ca_aal_rule.key.ip_vld = 1;

						if((cls_mir_cfg != NULL) && (cls_mir_cfg->cls_mir_en))
							ca_aal_rule.action.mrr_en = 1;
						ca_aal_rule.action.t2_ctrl = CLS_T2CTRL_MAINHASH_PROFILE_MC;
						ca_aal_rule.action.lspid_vld = 1;
						ca_aal_rule.action.lspid = RTK_FC_MAC_PORT_MC_MIRRORED;
						_rtk_fc_g3_caAalClsAssignHashProfileAction(&ca_aal_rule, ca_aal_profile_idx, FALSE, 0);

						if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, rsvType))
						{
							ACL_RSV("add special ACL GENERIC_INTF_HASH_PROFILE_DECISION failed!!!");
							addRuleFailedFlag = 1;
							break;
						}
						aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%d-%d,", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));

						/** mc rule **/
						ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_MC_MIRROR;
						ca_aal_rule.rslt_type = CL_RSLT_TYPE_0;

						aal_l3_cls_key_clean(&ca_aal_rule.key);
						memset(&ca_aal_rule.action, 0, sizeof(cls_action_t));
						ca_aal_rule.key.msk_o_lspid = CA_L3_AAL_KEY_MSK_ENABLE;
						ca_aal_rule.key.o_lspid = mirror_port;
						ca_aal_rule.key.msk_mac_da_an_hit = CA_L3_AAL_KEY_MSK_ENABLE;
						ca_aal_rule.key.mac_da_an_hit = 1;

						ca_aal_rule.key.msk_ip_vld = CA_L3_AAL_KEY_MSK_ENABLE;
						ca_aal_rule.key.ip_vld = 1;
						ca_aal_rule.key.msk_ip_da_mc = CA_L3_AAL_KEY_MSK_ENABLE;
						ca_aal_rule.key.ip_da_mc = 1;	//IP DA is in MC IP range,  IPv4: ip_da[31:28] == 4'b1110; IPv6: ip_da[127:120] == 8'hff

						if((cls_mir_cfg != NULL) && (cls_mir_cfg->cls_mir_en))
							ca_aal_rule.action.mrr_en = 1;
						ca_aal_rule.action.t2_ctrl = CLS_T2CTRL_MAINHASH_PROFILE_MC;
						ca_aal_rule.action.lspid_vld = 1;
						ca_aal_rule.action.lspid = RTK_FC_MAC_PORT_MC_MIRRORED;
						_rtk_fc_g3_caAalClsAssignHashProfileAction(&ca_aal_rule, ca_aal_profile_idx, FALSE, 0);

						if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, rsvType))
						{
							ACL_RSV("add special ACL GENERIC_INTF_HASH_PROFILE_DECISION failed!!!");
							addRuleFailedFlag = 1;
							break;
						}
						aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%d-%d,", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
					}
					ACL_RSV("add MC MIRRORED PORT HASH_PROFILE_DECISION @ %s[%s]", ca_aal_profile_idx?"LAN":"WAN", aal_idx_buf);
			}
#endif

			}
			break;
		case RTK_CA_CLS_TYPE_BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE:
			{
				memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));

				/*
				 * Default rules of Wan/Lan profile
				 */

				ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
				CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
#if defined(CONFIG_FC_RTL8277C_SERIES)
					if((ca_aal_profile_idx==CA_L3_AAL_CLS_PROFILE_WAN) && 
							fc_db.controlFuc.p7_rxsel_l3fe &&
							(fc_db.wanPortMask.portmask == (1<< RTK_FC_MAC_PORT_PON))) {
						activePorts.bits[0] = fc_db.wanPortMask.portmask | (1<<RTK_FC_MAC_PORT_PON_LSPID_TO_L2FE);						
						care_lspid = TRUE;
					
					} else {
						activePorts.bits[0] = 1;
						care_lspid = FALSE;
					}
#else
					bzero(&activePorts,sizeof(rtk_portmask_t));
					activePorts.bits[0] = 1;	//not support: 8277A uuc control, 8277 series mirror 
#endif

					CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
						/** with L4 header (UC5 tuple) **/
						ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_3;	//higher priority than uuc control
						ca_aal_rule.rslt_type = CL_RSLT_TYPE_0;

						aal_l3_cls_key_clean(&ca_aal_rule.key);
						memset(&ca_aal_rule.action, 0, sizeof(cls_action_t));

						if(care_lspid && (i<RTK_FC_MAC_PORT_CPU)){
							ca_aal_rule.key.msk_o_lspid = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.o_lspid = i;
						}
						ca_aal_rule.key.msk_mac_da_an_hit = CA_L3_AAL_KEY_MSK_ENABLE;
						ca_aal_rule.key.mac_da_an_hit = 0;	// DMAC is not not gatewat MAC
						ca_aal_rule.key.msk_ip_da_mc = CA_L3_AAL_KEY_MSK_ENABLE;
						ca_aal_rule.key.ip_da_mc = 0;	//not multicast

						ca_aal_rule.action.t2_ctrl = CLS_T2CTRL_MAINHASH_PROFILE_2TUPLE_BRIDGE;
						_rtk_fc_g3_caAalClsAssignHashProfileAction(&ca_aal_rule, ca_aal_profile_idx, FALSE, 0);
#if defined(CONFIG_FC_RTL8277C_SERIES)
						if((ca_aal_profile_idx==CA_L3_AAL_CLS_PROFILE_WAN) && 
								fc_db.controlFuc.p7_rxsel_l3fe &&
								(fc_db.wanPortMask.portmask == (1<< RTK_FC_MAC_PORT_PON))) {
							if(i == RTK_FC_MAC_PORT_PON) {
								ca_aal_rule.action.deepq = 1;		/*L3FE 1st round, o_lspid = 0x7*/									
								ca_aal_rule.action.lspid_vld = 1;
								ca_aal_rule.action.lspid = RTK_FC_MAC_PORT_PON_LSPID_TO_L2FE;
							}else {
								ca_aal_rule.action.deepq = 0;		/*L3FE 2nd round, o_lspid = 0x8*/
							}
						}
#endif
						if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, rsvType))
						{
							ACL_RSV("add special ACL RTK_CA_CLS_TYPE_BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE failed!!!");
							addRuleFailedFlag = 1;
							break;
						}
					}
				}

			}
			break;

		case RTK_CA_CLS_TYPE_BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE_GW_MAC_UPDATE:
			{
#if defined(CONFIG_FC_CA8277B_SERIES)
				if(parameter == NULL){
					ACL_RSV("add special ACL BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE_GW_MAC_UPDATE failed due to invalid parameter!!!");
					break;
				}
				loop_idx = *((uint32*)parameter);
				loop_start = loop_end = 0;
				if(loop_idx == RTK_FC_TABLESIZE_INTF_ACC_ACL){	//force enable
					loop_start = 0;
					loop_end = RTK_FC_TABLESIZE_INTF_ACC_ACL;
				}else if(loop_idx > RTK_FC_TABLESIZE_INTF_ACC_ACL){
					ACL_RSV("add special ACL BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE_GW_MAC_UPDATE failed due to invalid parameter (netif idx %d)!!!", loop_idx);
					break;
				}else{
					if(fc_db.controlFuc.bridge_5tuple_flow_accelerate_by_2tuple == DISABLED){
						ACL_RSV("skip add special ACL BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE_GW_MAC_UPDATE due to function disable!!!");
						break;
					}else{
						loop_start = loop_idx;
						loop_end = loop_idx+1;
					}
				}
				for(loop_idx = loop_start; loop_idx < loop_end; loop_idx++){
					memset(&asicNetif, 0x0, sizeof(asicNetif));
					assert_ok(rtk_rg_asic_netifTable_get(loop_idx, &asicNetif));
					if(asicNetif.valid == 0)
						continue;
					if((asicNetif.mac_idx != CA_UINT32_INVALID) || (asicNetif.acl_info_flow_acc != CA_UINT32_INVALID)){
						ACL_RSV("skip add special ACL BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE_GW_MAC_UPDATE due to unnecessary (netif idx %d, mac_idx %d, aclInfo 0x%x)!!!", loop_idx, asicNetif.mac_idx, asicNetif.acl_info_flow_acc);
						continue;
					}
					if((asicNetif.netif_mac_addr.octet[0] | asicNetif.netif_mac_addr.octet[1] | asicNetif.netif_mac_addr.octet[2] |
						asicNetif.netif_mac_addr.octet[3] | asicNetif.netif_mac_addr.octet[4] | asicNetif.netif_mac_addr.octet[5])==0) {
						ACL_RSV("skip add special ACL BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE_GW_MAC_UPDATE due to invalid parameter (netif idx %d, mac %pM)!!!", loop_idx, &asicNetif.netif_mac_addr.octet[0]);
						continue;
					}
					ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
					CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {

						bzero(&activePorts,sizeof(rtk_portmask_t));
						activePorts.bits[0] = 1;	//not support: 8277A uuc control, 8277 series mirror 

						CA_SCAN_PORTMASK(i, activePorts.bits[0]) {

							memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
							aal_l3_cls_key_clean(&ca_aal_rule.key);

							/** mc rule, same as BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE except replace msk_mac_da_an_hit as msk_dest_addr for different key type **/
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_4; //higher priority than non-gw mac use 2 tuple
							ca_aal_rule.rslt_type = CL_RSLT_TYPE_0;

							ca_aal_rule.key.msk_l4_vld = CA_L3_AAL_KEY_MSK_ENABLE;	// tcp udp rdp
							ca_aal_rule.key.l4_vld = 1;

							//MCST_KEY
							//ca_aal_rule.key.msk_dest_addr = CA_L3_AAL_KEY_MSK_ENABLE;
							//ca_aal_rule.key.dest_addr_type = 0;
							//for(loop_mac = 0; loop_mac < ETH_ALEN; loop_mac++)		//reverse order
							//	ca_aal_rule.key.dest_addr.dest_mac_addr[loop_mac] = asicNetif.netif_mac_addr.octet[((ETH_ALEN-1)-loop_mac)];
							//FLOW_KEY
							ca_aal_rule.key.mac_addr_ctrl = 6;	//1: sa range, 2: da range; 4: da+sa; 5: sa; 6: da
							for(loop_mac = 0; loop_mac < ETH_ALEN; loop_mac++)		//reverse order
								ca_aal_rule.key.mac_da[loop_mac] = asicNetif.netif_mac_addr.octet[((ETH_ALEN-1)-loop_mac)];

							ca_aal_rule.key.msk_ip_da_mc = CA_L3_AAL_KEY_MSK_ENABLE;
							ca_aal_rule.key.ip_da_mc = 0;	//not multicast

							if(ca_aal_profile_idx==CA_L3_AAL_CLS_PROFILE_WAN)
								ca_aal_rule.action.t2_ctrl = CLS_T2CTRL_MAINHASH_PROFILE_WAN;
							else
								ca_aal_rule.action.t2_ctrl = CLS_T2CTRL_MAINHASH_PROFILE_LAN;
							_rtk_fc_g3_caAalClsAssignHashProfileAction(&ca_aal_rule, ca_aal_profile_idx, FALSE, 0);

							if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, rsvType))
							{
								ACL_RSV("add special ACL BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE_GW_MAC_UPDATE failed!!!");
								addRuleFailedFlag = 1;
								break;
							}
							if(ca_aal_profile_idx == CA_L3_AAL_CLS_PROFILE_WAN)
								asicNetif.acl_info_flow_acc_wan = ca_aal_rslt_idx;
							else
								asicNetif.acl_info_flow_acc = ca_aal_rslt_idx;
							ACL_RSV("add special ACL BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE_GW_MAC_UPDATE: netif %d acl%s 0x%x", loop_idx, (ca_aal_profile_idx==CA_L3_AAL_CLS_PROFILE_WAN)?"WAN":"LAN",ca_aal_rslt_idx);
						}	//CA_SCAN_PORTMASK
					}	//CA_SCAN_AAL_PROFILE_MASK
					assert_ok(rtk_rg_asic_netifTable_set(loop_idx, &asicNetif));
				}	//for loop
#else				
				//8277A not support dmac + l4_vld
				ACL_RSV("Skip add special ACL type %d due to chip not support.", rsvType);
				break;
#endif
			}
			break;

		case RTK_CA_CLS_TYPE_GENERIC_INTF_MC_PROFILE_WITH_DMAC:
			{
				if(parameter == NULL){
					ACL_RSV("add special ACL GENERIC_INTF_MC_PROFILE_WITH_DMAC failed due to invalid parameter!!!");
					break;
				}
				for(i = 0; i < RTK_FC_TABLESIZE_INTF_MC_ACL; i++){
					if(memcmp(&fc_db.aclAndCfReservedRule.generic_intf_mc_dmac[i].gateway_mac_addr, ((rtk_fc_aclAndCf_reserved_generic_intf_mc_dmac_t*)parameter)->gateway_mac_addr.octet, ETH_ALEN) == 0)
						break;
				}
				if(i != RTK_FC_TABLESIZE_INTF_MC_ACL){
					ACL_RSV("skip add special ACL GENERIC_INTF_MC_PROFILE_WITH_DMAC due to duplicate gw mac!!!");
					break;
				}

				memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
				aal_l3_cls_key_clean(&ca_aal_rule.key);

				/** mc rule, same as GENERIC_INTF_HASH_PROFILE_DECISION except replace msk_mac_da_an_hit as msk_dest_addr for different key type **/
				ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_9; //reuse same entry with ca mc rule
				ca_aal_rule.rslt_type = CL_RSLT_TYPE_0;

				ca_aal_rule.key.msk_dest_addr = CA_L3_AAL_KEY_MSK_ENABLE;
				ca_aal_rule.key.dest_addr_type = 0;	//0 is mac, 1 is ip
				for(i = 0; i < ETH_ALEN; i++)		//reverse order
					ca_aal_rule.key.dest_addr.dest_mac_addr[i] = ((rtk_fc_aclAndCf_reserved_generic_intf_mc_dmac_t*)parameter)->gateway_mac_addr.octet[((ETH_ALEN-1)-i)];

				ca_aal_rule.key.msk_ip_vld = CA_L3_AAL_KEY_MSK_ENABLE;
				ca_aal_rule.key.ip_vld = 1;

				ca_aal_rule.key.msk_ip_da_mc = CA_L3_AAL_KEY_MSK_ENABLE;
				ca_aal_rule.key.ip_da_mc = 1;

				ca_aal_rule.action.t2_ctrl = CLS_T2CTRL_MAINHASH_PROFILE_MC;
				_rtk_fc_g3_caAalClsAssignHashProfileAction(&ca_aal_rule, ca_aal_profile_idx, FALSE, 0);

				if(_rtk_fc_g3_caAalClsRuleAdd(CA_L3_AAL_CLS_PROFILE_WAN, &ca_aal_rule, &ca_aal_rslt_idx, rsvType))
				{
					ACL_RSV("add special ACL GENERIC_INTF_MC_PROFILE_WITH_DMAC failed!!!");
					addRuleFailedFlag = 1;
					break;
				}else
					((rtk_fc_aclAndCf_reserved_generic_intf_mc_dmac_t*)parameter)->acl_info = ca_aal_rslt_idx;

				for(i = 0; i < RTK_FC_TABLESIZE_INTF_MC_ACL; i++){
					if(!(fc_db.aclAndCfReservedRule.generic_intf_mc_dmac[i].gateway_mac_addr.octet[0]|fc_db.aclAndCfReservedRule.generic_intf_mc_dmac[i].gateway_mac_addr.octet[1]|
							fc_db.aclAndCfReservedRule.generic_intf_mc_dmac[i].gateway_mac_addr.octet[2]|fc_db.aclAndCfReservedRule.generic_intf_mc_dmac[i].gateway_mac_addr.octet[3]|
							fc_db.aclAndCfReservedRule.generic_intf_mc_dmac[i].gateway_mac_addr.octet[4]|fc_db.aclAndCfReservedRule.generic_intf_mc_dmac[i].gateway_mac_addr.octet[5]))
						break;
				}
				if(i != RTK_FC_TABLESIZE_INTF_MC_ACL){
					fc_db.aclAndCfReservedRule.generic_intf_mc_dmac[i].netif_idx = ((rtk_fc_aclAndCf_reserved_generic_intf_mc_dmac_t*)parameter)->netif_idx;
					memcpy(&fc_db.aclAndCfReservedRule.generic_intf_mc_dmac[i].gateway_mac_addr, ((rtk_fc_aclAndCf_reserved_generic_intf_mc_dmac_t*)parameter)->gateway_mac_addr.octet, ETH_ALEN);
					fc_db.aclAndCfReservedRule.generic_intf_mc_dmac[i].acl_info = ca_aal_rslt_idx;
				}else
					WARNING("record special ACL GENERIC_INTF_MC_PROFILE_WITH_DMAC sw information failed!!!");
			}
			break;

		case RTK_CA_CLS_TYPE_REARRANGE_PROTECTION:

			memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
			aal_l3_cls_key_clean(&ca_aal_rule.key);
			aal_idx_offset = 0;

			ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
			CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
				ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_15;
				ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

				//IP DA is in MC IP range, ipv4_da[31:28]=4b1110, ipv6_da[127:120]=8hff
				ca_aal_rule.key.msk_ip_da_mc = CA_L3_AAL_KEY_MSK_ENABLE;
				ca_aal_rule.key.ip_da_mc = 1;

				if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, rsvType))
				{
					ACL_RSV("add special REARRANGE_PROTECTION failed!!!");
					addRuleFailedFlag = 1;
					break;
				}	
				aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
			}
			ACL_RSV("add special REARRANGE_PROTECTION @ %s", aal_idx_buf);
			break;

#if defined(CONFIG_FC_G3_G3LITE_SERIES)
		case RTK_CA_CLS_TYPE_L2_UPSTREAM_LOOPBACK_INGRESS_FORWARD_L3FE:
			//Create L2 igr Classifier for forward the packet from loopback port with SVLAN tag (upstream) to L3FE
			if(fc_db.controlFuc.loopbackMode_is_enabled)
			{
				bzero(&ca_cls_rule,sizeof(ca_cls_rule));
				ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L2_0;
				
				ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_ETHERNET, G3_LOOPBACK_P_NEWSPA);
				ca_cls_rule.key_mask.src_port = TRUE;

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

					if(_rtk_fc_g3_caClsRuleAdd(&ca_cls_rule, rsvType)) {
						ACL_RSV("add special ACL RTK_CA_CLS_TYPE_L2_UPSTREAM_LOOPBACK_INGRESS_FORWARD_L3FE failed!!!");
						addRuleFailedFlag = 1;
					}
				}
			}
			
			break;
		case RTK_CA_CLS_TYPE_L2_UPSTREAM_LAN_INGRESS_FORWARD_LOOPBACK:
			if(fc_db.controlFuc.loopbackMode_is_enabled)
			{
				//Create L2 igr Classifier for forward the packet from LAN port to loopback port with SVLAN tag
				bzero(&ca_cls_rule,sizeof(ca_cls_rule));
				bzero(&activePorts,sizeof(rtk_portmask_t));
				activePorts.bits[0] = G3_LOOPBACK_P_MASK; //packet from LAN port=>go to loopback port directly.
				CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
					ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L2_0;
					ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_ETHERNET, i);
					ca_cls_rule.key_mask.src_port = TRUE;

					ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_PORT;
					ca_cls_rule.action.dest.port = G3_LOOPBACK_P_NEWSPA;

					ca_cls_rule.action.options.masks.outer_vlan_act = 1;
					ca_cls_rule.action.options.masks.outer_tpid = 1;
					ca_cls_rule.action.options.outer_vlan_act = CA_CLASSIFIER_VLAN_ACTION_PUSH;
					ca_cls_rule.action.options.outer_vid = G3_LOOPBACK_UPSTREAM_VID(i);
					//ca_cls_rule.action.options.outer_tpid = 0x88A8;

					if(_rtk_fc_g3_caClsRuleAdd(&ca_cls_rule, rsvType)) {
						ACL_RSV("add special ACL RTK_CA_CLS_TYPE_L2_UPSTREAM_LAN_INGRESS_FORWARD_LOOPBACK failed!!!");
						addRuleFailedFlag = 1;
						break;
					}
				}	
			}
			break;
		case RTK_CA_CLS_TYPE_L2_INGRESS_SA_HOSTPOLICING:
			if(fc_db.controlFuc.loopbackMode_is_enabled)
			{
				//Create L2 ***egress*** Classifier for ingress SA host policing (using port 0x19 egress CLS)
				bzero(&ca_cls_rule,sizeof(ca_cls_rule));
				ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L2_3;
				ca_cls_rule.key.dest_port = CA_PORT_ID(CA_PORT_TYPE_CPU, AAL_LPORT_L3_LAN);
				ca_cls_rule.key_mask.dest_port = TRUE;
				
				ca_cls_rule.key.l2.vlan_otag.vlan_min.vid = G3_LOOPBACK_UPSTREAM_VID_P0;
				ca_cls_rule.key.l2.vlan_otag.vlan_max.vid = G3_LOOPBACK_UPSTREAM_VID_P3;
				memcpy(ca_cls_rule.key.l2.mac_sa.mac_min, ((rtk_fc_aclAndCf_reserved_hostPoliceSaDa_t*)parameter)->hostMac.octet, sizeof(ca_mac_addr_t));
				memcpy(ca_cls_rule.key.l2.mac_sa.mac_max, ((rtk_fc_aclAndCf_reserved_hostPoliceSaDa_t*)parameter)->hostMac.octet, sizeof(ca_mac_addr_t));

				ca_cls_rule.key_mask.l2 = TRUE;
				ca_cls_rule.key_mask.l2_mask.vlan_otag = TRUE;
				ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.vid = TRUE;
				ca_cls_rule.key_mask.l2_mask.mac_sa = TRUE;

				ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_FE;
				ca_cls_rule.action.dest.fe= CA_CLASSIFIER_FORWARD_FE_L2FE;

				ca_cls_rule.action.options.masks.action_handle = TRUE;
				ca_cls_rule.action.options.handle_type = CA_KEY_HANDLE_TYPE_FLOW_ID;
				ca_cls_rule.action.options.action_handle.flow_id = ((rtk_fc_aclAndCf_reserved_hostPoliceSaDa_t*)parameter)->flow_id;

				if(_rtk_fc_g3_caClsRuleAdd(&ca_cls_rule, rsvType)) {
					ACL_RSV("add special ACL RTK_CA_CLS_TYPE_L2_UPSTREAM_LAN_INGRESS_FORWARD_LOOPBACK failed!!!");
					addRuleFailedFlag = 1;
				}
				else
					((rtk_fc_aclAndCf_reserved_hostPoliceSaDa_t*)parameter)->ca_cls_index = ca_cls_rule.index;
			}
			break;

		case RTK_CA_CLS_TYPE_L2_EGRESS_DA_HOSTPOLICING:
			if(fc_db.controlFuc.loopbackMode_is_enabled)
			{
				//Create L2 ***ingress*** Classifier for ingress SA host policing (using port 0x6 ingress CLS)
				bzero(&ca_cls_rule,sizeof(ca_cls_rule));
				ca_cls_rule.priority = RTK_CA_CLS_PRIORITY_L2_3;
				ca_cls_rule.key.src_port = CA_PORT_ID(CA_PORT_TYPE_ETHERNET, G3_LOOPBACK_P_NEWSPA);
				ca_cls_rule.key_mask.src_port = TRUE;

				ca_cls_rule.key.l2.vlan_otag.vlan_min.vid = G3_LOOPBACK_DOWNSTREAM_VID_MIN;
				ca_cls_rule.key.l2.vlan_otag.vlan_max.vid = G3_LOOPBACK_DOWNSTREAM_VID_MAX;
				memcpy(ca_cls_rule.key.l2.mac_da.mac_min, ((rtk_fc_aclAndCf_reserved_hostPoliceSaDa_t*)parameter)->hostMac.octet, sizeof(ca_mac_addr_t));
				memcpy(ca_cls_rule.key.l2.mac_da.mac_max, ((rtk_fc_aclAndCf_reserved_hostPoliceSaDa_t*)parameter)->hostMac.octet, sizeof(ca_mac_addr_t));

				ca_cls_rule.key_mask.l2 = TRUE;
				ca_cls_rule.key_mask.l2_mask.vlan_otag = TRUE;
				ca_cls_rule.key_mask.l2_mask.vlan_otag_mask.vid = TRUE;
				ca_cls_rule.key_mask.l2_mask.mac_da = TRUE;

				ca_cls_rule.action.forward = CA_CLASSIFIER_FORWARD_FE;
				ca_cls_rule.action.dest.fe= CA_CLASSIFIER_FORWARD_FE_L2FE;

				ca_cls_rule.action.options.masks.action_handle = TRUE;
				ca_cls_rule.action.options.handle_type = CA_KEY_HANDLE_TYPE_FLOW_ID;
				ca_cls_rule.action.options.action_handle.flow_id = ((rtk_fc_aclAndCf_reserved_hostPoliceSaDa_t*)parameter)->flow_id;

				if(_rtk_fc_g3_caClsRuleAdd(&ca_cls_rule, rsvType)) {
					ACL_RSV("add special ACL RTK_CA_CLS_TYPE_L2_UPSTREAM_LAN_INGRESS_FORWARD_LOOPBACK failed!!!");
					addRuleFailedFlag = 1;
				}
				else
					((rtk_fc_aclAndCf_reserved_hostPoliceSaDa_t*)parameter)->ca_cls_index = ca_cls_rule.index;
			}
			break;
#endif

#if defined(CONFIG_FC_CA8277B_SERIES)
		case RTK_CA_CLS_TYPE_VXLAN_FFD_US:
			if(fc_db.controlFuc.special_fast_forward_mode)
			{
				if(parameter == NULL){
					ACL_RSV("add special ACL VXLAN_FFD_US failed due to invalid parameter!!!");
					break;
				}
				memcpy(&vxlan_ffd,(rtk_fc_aclAndCf_reserved_vxlan_ffd_t*)parameter,sizeof(rtk_fc_aclAndCf_reserved_vxlan_ffd_t));
				if(fc_db.vxlan_l3cls_upstream_isSet[vxlan_ffd.lan_port_idx]){
					ACL_RSV("skip add special ACL VXLAN_FFD_US failed due to rule for port %d already exist!!!", vxlan_ffd.lan_port_idx);
					break;
				}
				memset(&cam_pkt_len, 0, sizeof(l3_cam_pkt_len_tbl_entry_t));
				cam_pkt_len.vld = 1;
				cam_pkt_len.pkt_len_low = vxlan_ffd.pkt_len_start;
				cam_pkt_len.pkt_len_hi = vxlan_ffd.pkt_len_start;
				if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_PKT_LEN, &cam_pkt_len, &cam_rslt_idx)){
					ACL_RSV("skip add special ACL VXLAN_FFD_US[%d] failed due to invalid pkt len cam idx!!!", vxlan_ffd.lan_port_idx);
					addRuleFailedFlag = 1;
					break;
				}

				
				
				memset(&cam_pkt_len, 0, sizeof(l3_cam_pkt_len_tbl_entry_t));
				cam_pkt_len.vld = 1;
				cam_pkt_len.pkt_len_low = 124;
				cam_pkt_len.pkt_len_hi = 124;
				if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_PKT_LEN, &cam_pkt_len, &cam_rslt_idx2)){
					ACL_RSV("skip add special ACL VXLAN_FFD_US[%d] failed due to invalid pkt len cam idx2!!!", vxlan_ffd.lan_port_idx);
				}
				fc_db.vxlan_l3cls_info.pktlen_us_68_aclInfo[vxlan_ffd.lan_port_idx] = CA_UINT32_INVALID;
				fc_db.vxlan_l3cls_info.pktlen_us_128_aclInfo[vxlan_ffd.lan_port_idx] = CA_UINT32_INVALID;

				for(i=0; i<2; i++)
				{

					memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
					aal_l3_cls_key_clean(&ca_aal_rule.key);
					memset(&ca_aal_rule.action, 0, sizeof(cls_action_t));

					ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_15;
					ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

					ca_aal_rule.key.msk_ip_protocol = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.ip_protocol = 0x11;

					ca_aal_rule.key.msk_l4_vld = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.l4_vld = TRUE;

					ca_aal_rule.key.msk_l4_port = 0;	//0: sp+dp, 1: sp range, 2: dp range, 3: sp range+dp range
					ca_aal_rule.key.l4_sp = vxlan_ffd.l4_sport;
					ca_aal_rule.key.l4_dp = vxlan_ffd.l4_dport;

					if(i>0)
						ca_aal_rule.key.pktlen_rng_match_vec = 0x1 << cam_rslt_idx2;
					else
						ca_aal_rule.key.pktlen_rng_match_vec = 0x1 << cam_rslt_idx;
					
					ca_aal_rule.key.pktlen_rng_match_mask = CA_L3_AAL_KEY_MSK_ENABLE;

					if(i>0)
						ca_aal_rule.action.cos = 4;
					else
						ca_aal_rule.action.cos = 2;
					ca_aal_rule.action.cos_sel = TRUE;

					if(_rtk_fc_g3_caAalClsRuleAdd(CA_L3_AAL_CLS_PROFILE_LAN, &ca_aal_rule, &ca_aal_rslt_idx, rsvType))
					{
						WARNING("add reserved ACL VXLAN_FFD_US failed!!!");
						addRuleFailedFlag = 1;
						break;
					}
					if(i>0){
						fc_db.vxlan_l3cls_info.pktlen_us_128_aclInfo[vxlan_ffd.lan_port_idx] = ca_aal_rslt_idx;
						fc_db.vxlan_l3cls_upstream_isSet[vxlan_ffd.lan_port_idx] = 1;
					}else{
						fc_db.vxlan_l3cls_info.pktlen_us_68_aclInfo[vxlan_ffd.lan_port_idx] = ca_aal_rslt_idx;
					}
				}
			}
			break;

		case RTK_CA_CLS_TYPE_VXLAN_FFD_DS:
			if(fc_db.controlFuc.special_fast_forward_mode)
			{
				if(parameter == NULL){
					ACL_RSV("add special ACL VXLAN_FFD_DS failed due to invalid parameter!!!");
					break;
				}
				memcpy(&vxlan_ffd,(rtk_fc_aclAndCf_reserved_vxlan_ffd_t*)parameter,sizeof(rtk_fc_aclAndCf_reserved_vxlan_ffd_t));
				if(fc_db.vxlan_l3cls_downstream_isSet[vxlan_ffd.lan_port_idx]){
					ACL_RSV("skip add special ACL VXLAN_FFD_DS due to rule for port %d already exist!!!", vxlan_ffd.lan_port_idx);
					break;
				}
				memset(&cam_pkt_len, 0, sizeof(l3_cam_pkt_len_tbl_entry_t));
				cam_pkt_len.vld = 1;
				cam_pkt_len.pkt_len_low = vxlan_ffd.pkt_len_start;
				cam_pkt_len.pkt_len_hi = vxlan_ffd.pkt_len_start;
				if (AAL_E_OK != aal_entry_add(AAL_TABLE_L3_CAM_PKT_LEN, &cam_pkt_len, &cam_rslt_idx)){
					ACL_RSV("skip add special ACL VXLAN_FFD_DS[%d] due to invalid pkt len cam idx!!!", vxlan_ffd.lan_port_idx);
					addRuleFailedFlag = 1;
					break;
				}
				fc_db.vxlan_l3cls_info.pktlen_ds_68_aclInfo[vxlan_ffd.lan_port_idx] = CA_UINT32_INVALID;
				fc_db.vxlan_l3cls_info.pktlen_ds_others_aclInfo[vxlan_ffd.lan_port_idx] = CA_UINT32_INVALID;

				for(i=0; i<2; i++){	//rule 1 wo pkt len, rule 2 w/o pkt len, if pkt len fail, rule 1 w/o pkt len
					memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
					aal_l3_cls_key_clean(&ca_aal_rule.key);
					memset(&ca_aal_rule.action, 0, sizeof(cls_action_t));

					if(i == 0)
						ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_15;
					else
						ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_HEAD;
					ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

					ca_aal_rule.key.msk_ip_protocol = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.ip_protocol = 0x11;

					ca_aal_rule.key.msk_l4_vld = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.l4_vld = TRUE;

					ca_aal_rule.key.msk_l4_port = 0;	//0: sp+dp, 1: sp range, 2: dp range, 3: sp range+dp range
					ca_aal_rule.key.l4_sp = vxlan_ffd.l4_sport;
					ca_aal_rule.key.l4_dp = vxlan_ffd.l4_dport;

					if(i == 0){
						ca_aal_rule.key.pktlen_rng_match_vec = 0x1 << cam_rslt_idx;
						ca_aal_rule.key.pktlen_rng_match_mask = CA_L3_AAL_KEY_MSK_ENABLE;
					}

					if(i == 0)
						ca_aal_rule.action.cos = 3;
					else
						ca_aal_rule.action.cos = 1;
					ca_aal_rule.action.cos_sel = TRUE;

					if(_rtk_fc_g3_caAalClsRuleAdd(CA_L3_AAL_CLS_PROFILE_WAN, &ca_aal_rule, &ca_aal_rslt_idx, rsvType))
					{
						WARNING("add reserved ACL VXLAN_FFD_DS failed!!!");
						addRuleFailedFlag = 1;
						if(i == 1)
							assert_ok(_rtk_rg_aclAndCfReservedRuleDelSpecial(rsvType, &fc_db.vxlan_l3cls_info.pktlen_ds_68_aclInfo[vxlan_ffd.lan_port_idx]));
						break;
					}
					if(i == 0)
						fc_db.vxlan_l3cls_info.pktlen_ds_68_aclInfo[vxlan_ffd.lan_port_idx] = ca_aal_rslt_idx;
					else{
						fc_db.vxlan_l3cls_info.pktlen_ds_others_aclInfo[vxlan_ffd.lan_port_idx] = ca_aal_rslt_idx;
						fc_db.vxlan_l3cls_downstream_isSet[vxlan_ffd.lan_port_idx] = 1;
						break;
					}
				}
			}
			break;

		case RTK_CA_CLS_TYPE_VXLAN_FFD_EXTRA_DS:
			if(fc_db.controlFuc.special_fast_forward_mode)
			{
				
				if(fc_db.vxlan_ds_extraCLS_isSet){
					ACL_RSV("skip add special ACL VXLAN_FFD_DS_EXTRA due to rule already exist!!!");
					break;
				}
				
				fc_db.vxlan_ds_extraCLS_Info = CA_UINT32_INVALID;

				{	//rule 1 wo pkt len, rule 2 w/o pkt len, if pkt len fail, rule 1 w/o pkt len
					memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
					aal_l3_cls_key_clean(&ca_aal_rule.key);
					memset(&ca_aal_rule.action, 0, sizeof(cls_action_t));

					ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_13;
					
					ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

					ca_aal_rule.key.msk_ip_protocol = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.ip_protocol = 0x11;

					ca_aal_rule.key.msk_l4_vld = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.l4_vld = TRUE;

					ca_aal_rule.key.msk_l4_port = 2;	//0: sp+dp, 1: sp range, 2: dp range, 3: sp range+dp range
					ca_aal_rule.key.l4_sp = 4789;
					ca_aal_rule.key.l4_dp = 4789;

					ca_aal_rule.action.cos = 1;
					ca_aal_rule.action.cos_sel = TRUE;

					if(_rtk_fc_g3_caAalClsRuleAdd(CA_L3_AAL_CLS_PROFILE_WAN, &ca_aal_rule, &ca_aal_rslt_idx, rsvType))
					{
						WARNING("add reserved ACL VXLAN_FFD_DS_EXTRA failed!!!");
						addRuleFailedFlag = 1;
						assert_ok(_rtk_rg_aclAndCfReservedRuleDelSpecial(rsvType, &fc_db.vxlan_ds_extraCLS_Info));
						break;
					}
					
					fc_db.vxlan_ds_extraCLS_Info = ca_aal_rslt_idx;
					fc_db.vxlan_ds_extraCLS_isSet= 1;
					
				}
			}
			break;
#endif

		case RTK_CLS_TYPE_USER_ACL_REARRANGE:
			{
				ACL_RSV("reflash user acl only");
				ACL_CTRL("reflash user acl only");
				ASSERT_EQ(_rtk_fc_aclSWEntry_and_asic_rearrange(),RT_ERR_RG_OK);
			}
			break;

		case RTK_CLS_TYPE_HTTP_ACCELERATE_BY_PE:
			{
#if !defined(CONFIG_FC_G3_G3LITE_SERIES)	//8277A not support ip_addr_ctrl
				if(parameter == NULL){
					WARNING("add special ACL HTTP_ACCELERATE_BY_PE failed due to invalid parameter!!!");
					break;
				}
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
				{
					rtk_fc_asic_dmaLso_reg_t dmaLso_reg;
					dmaLso_reg.lspid_keep_vld = 1;
					ASSERT_EQ(rtk_fc_asic_dmaLso_reg_get(&dmaLso_reg), RT_ERR_RG_OK);				
					if(dmaLso_reg.lspid_keep == 0)
						WARNING("HTTP_ACCELERATE_BY_PE warning: DMALSO lspid_keep=0 may not keep lspid.");
				}
#endif
				aal_idx_offset = 0;
				memset(&aal_idx_buf, 0, sizeof(aal_idx_buf));
				memcpy(&acc_by_pe,(rtk_fc_aclAndCf_reserved_accelerate_by_pe_t*)parameter,sizeof(rtk_fc_aclAndCf_reserved_accelerate_by_pe_t));
				((rtk_fc_aclAndCf_reserved_accelerate_by_pe_t*)parameter)->rslt_idx = CA_UINT32_INVALID;;

				{
					memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
					aal_l3_cls_key_clean(&ca_aal_rule.key);

					ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_HEAD;
					ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

					if(acc_by_pe.is_ipv4){
						ca_aal_rule.key.ip_sa.ip_addr_0 = acc_by_pe.ipv4_src_ip;
						ca_aal_rule.key.ip_sa.ip_addr_1 = ca_aal_rule.key.ip_sa.ip_addr_2 = ca_aal_rule.key.ip_sa.ip_addr_3 = 0;
						ca_aal_rule.key.ip_da.ip_addr_0 = acc_by_pe.ipv4_dst_ip;
						ca_aal_rule.key.ip_da.ip_addr_1 = ca_aal_rule.key.ip_da.ip_addr_2 = ca_aal_rule.key.ip_da.ip_addr_3 = 0;
						ca_aal_rule.key.msk_ip_sa_l = ca_aal_rule.key.msk_ip_da_l = 32 & 0x7f;
						ca_aal_rule.key.msk_ip_sa_h = ca_aal_rule.key.msk_ip_da_h = 32 >> 7;
					}else{
						for(i = 0; i < 4; i++)	//v6SIP: [0]=[31:0], [1]=[63:32], [2]=[95:64], [3]=[127:96]
							address[i] = (acc_by_pe.ipv6_src_ip.ipv6_addr[0+i*4]<<24)|(acc_by_pe.ipv6_src_ip.ipv6_addr[1+i*4]<<16)|(acc_by_pe.ipv6_src_ip.ipv6_addr[2+i*4]<<8)|(acc_by_pe.ipv6_src_ip.ipv6_addr[3+i*4]);
						ca_aal_rule.key.ip_sa.ip_addr_0 = address[3];
						ca_aal_rule.key.ip_sa.ip_addr_1 = address[2];
						ca_aal_rule.key.ip_sa.ip_addr_2 = address[1];
						ca_aal_rule.key.ip_sa.ip_addr_3 = address[0];
						for(i = 0; i < 4; i++)	//v6DIP: [0]=[31:0], [1]=[63:32], [2]=[95:64], [3]=[127:96]
							address[i] = (acc_by_pe.ipv6_dst_ip.ipv6_addr[0+i*4]<<24)|(acc_by_pe.ipv6_dst_ip.ipv6_addr[1+i*4]<<16)|(acc_by_pe.ipv6_dst_ip.ipv6_addr[2+i*4]<<8)|(acc_by_pe.ipv6_dst_ip.ipv6_addr[3+i*4]);
						ca_aal_rule.key.ip_da.ip_addr_0 = address[3];
						ca_aal_rule.key.ip_da.ip_addr_1 = address[2];
						ca_aal_rule.key.ip_da.ip_addr_2 = address[1];
						ca_aal_rule.key.ip_da.ip_addr_3 = address[0];
						ca_aal_rule.key.msk_ip_sa_l = ca_aal_rule.key.msk_ip_da_l = 128 & 0x7f;
						ca_aal_rule.key.msk_ip_sa_h = ca_aal_rule.key.msk_ip_da_h = 128 >> 7;
					}
					ca_aal_rule.key.ip_addr_ctrl = 0;	//0: dip+sip 1: dip range, 2: sip range

					ca_aal_rule.key.msk_ip_protocol = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.ip_protocol = acc_by_pe.l4_proto_value;

					ca_aal_rule.key.msk_l4_vld = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.l4_vld = TRUE;

					ca_aal_rule.key.msk_l4_port = 0;	//0: sp+dp, 1: sp range, 2: dp range, 3: sp range+dp range
					ca_aal_rule.key.l4_sp = acc_by_pe.l4_sport;
					ca_aal_rule.key.l4_dp = acc_by_pe.l4_dport;

					if((0x1<<acc_by_pe.igr_port_idx) & RT_ACL_PORT_ALL_WAN_BIT){
						ca_aal_profile_idx = CA_L3_AAL_CLS_PROFILE_WAN;
					}else{
						ca_aal_profile_idx = CA_L3_AAL_CLS_PROFILE_LAN;
						ca_aal_rule.key.o_lspid = acc_by_pe.igr_port_idx;
						ca_aal_rule.key.msk_o_lspid = CA_L3_AAL_KEY_MSK_ENABLE;
					}

					_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);
					ca_aal_rule.action.mcgid = acc_by_pe.action_redirect_ldpid;
					ca_aal_rule.action.pol_vld = TRUE;
					ca_aal_rule.action.pol_sel = 2;
					ca_aal_rule.action.pol_base = acc_by_pe.action_pol_id;

					if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, rsvType))
					{
						WARNING("add special ACL ACCELERATE_BY_PE failed!!!");
						addRuleFailedFlag = 1;
						break;
					}

					aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", ca_aal_profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
					((rtk_fc_aclAndCf_reserved_accelerate_by_pe_t*)parameter)->rslt_idx = ca_aal_rslt_idx;
					TRACE("add CLS HTTP_ACCELERATE_BY_PE for l4_sport %d success @ %s", acc_by_pe.l4_sport, aal_idx_buf);
					ACL_RSV("add special HTTP_ACCELERATE_BY_PE for l4_sport %d @ %s", acc_by_pe.l4_sport, aal_idx_buf);
				}
#endif
			}
			break;

		case RTK_CLS_TYPE_SRV6_ACCELERATE_BY_PE:
			{
#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
				if(parameter == NULL){
					WARNING("add special ACL SRV6_ACCELERATE_BY_PE failed due to invalid parameter!!!");
					break;
				}

				aal_idx_offset = 0;
				memset(&aal_idx_buf, 0, sizeof(aal_idx_buf));
				memcpy(&srv6_acc_by_pe,(rtk_fc_aclAndCf_reserved_srv6_accelerate_by_pe_t*)parameter,sizeof(rtk_fc_aclAndCf_reserved_srv6_accelerate_by_pe_t));
				((rtk_fc_aclAndCf_reserved_srv6_accelerate_by_pe_t*)parameter)->rslt_idx = CA_UINT32_INVALID;;

				{
					memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
					aal_l3_cls_key_clean(&ca_aal_rule.key);

					ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_HEAD;
					ca_aal_rule.rslt_type = CL_RSLT_TYPE_1;

					for(i = 0; i < 4; i++)	//v6DIP: [0]=[31:0], [1]=[63:32], [2]=[95:64], [3]=[127:96]
						address[i] = (srv6_acc_by_pe.ipv6_dst_ip.ipv6_addr[0+i*4]<<24)|(srv6_acc_by_pe.ipv6_dst_ip.ipv6_addr[1+i*4]<<16)|(srv6_acc_by_pe.ipv6_dst_ip.ipv6_addr[2+i*4]<<8)|(srv6_acc_by_pe.ipv6_dst_ip.ipv6_addr[3+i*4]);
					ca_aal_rule.key.ip_addr.ip_addr_0 = address[3];
					ca_aal_rule.key.ip_addr.ip_addr_1 = address[2];
					ca_aal_rule.key.ip_addr.ip_addr_2 = address[1];
					ca_aal_rule.key.ip_addr.ip_addr_3 = address[0];
					ca_aal_rule.key.msk_ip_addr_l = 128 & 0x7f;
					ca_aal_rule.key.msk_ip_addr_h = 128 >> 7;
					ca_aal_rule.key.ip_addr_type = 0;	//0: dip, 1: sip

					ca_aal_rule.key.msk_ip_protocol = CA_L3_AAL_KEY_MSK_ENABLE;
					ca_aal_rule.key.ip_protocol = 143;	//IPPROTO_ETHERNET

					ca_aal_profile_idx = CA_L3_AAL_CLS_PROFILE_WAN;
					_rtk_fc_g3_caAalClsAssignTrapAction(&ca_aal_rule, ca_aal_profile_idx);
					ca_aal_rule.action.mdata_w_vld_0 = 0x4;
					ca_aal_rule.action.mdata_w_0 = CPU_REASON_L34_FWD << RXINFO_REF_HW_RSN_SHIFT_BIT;
					ca_aal_rule.action.mcgid = srv6_acc_by_pe.action_redirect_ldpid;
					ca_aal_rule.action.mdata_w_vld_0 |= 0x3;
					ca_aal_rule.action.mdata_w_0 |= srv6_acc_by_pe.action_mdata_dev_id;

					if(_rtk_fc_g3_caAalClsRuleAdd(ca_aal_profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, rsvType))
					{
						WARNING("add special ACL SRV6_ACCELERATE_BY_PE failed!!!");
						addRuleFailedFlag = 1;
						break;
					}

					aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", /*ca_aal_profile_idx?"LAN":*/"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
					((rtk_fc_aclAndCf_reserved_srv6_accelerate_by_pe_t*)parameter)->rslt_idx = ca_aal_rslt_idx;
					fc_db.aclAndCfReservedRule.reservedMask[rsvType] = ENABLED;
					TRACE("add CLS SRV6_ACCELERATE_BY_PE success @ %s", aal_idx_buf);
					ACL_RSV("add special SRV6_ACCELERATE_BY_PE @ %s", aal_idx_buf);
				}
#endif
			}
			break;

#if defined(CONFIG_FC_RTL8277C_SERIES) || defined(CONFIG_FC_RTL9607F_SERIES)
		case RTK_CLS_TYPE_RESET_L2_L3_CLS:
			{
				for(i = 0; i <= L2_CLS_KEY_TBL_ENTRY_MAX; i++){
					ca_aal_rslt_idx = i<<1;
					aal_l2_cls_del(RTK_ASIC_DEVID, (ca_aal_rslt_idx|0));
					aal_l2_cls_del(RTK_ASIC_DEVID, (ca_aal_rslt_idx|1));	//sub entry
				}
				aal_table_flush(AAL_TABLE_L3_CLS_KEY);
				aal_table_flush(AAL_TABLE_L3_CLS_FIB);

#if defined(CONFIG_FC_RTL8277C_SERIES)
				ca_aal_profile_mask = (1<<CA_L3_AAL_CLS_PROFILE_WAN) | (1<<CA_L3_AAL_CLS_PROFILE_LAN);
				CA_SCAN_AAL_PROFILE_MASK(ca_aal_profile_idx, ca_aal_profile_mask) {
					/** LAN & WAN default fib **/
					memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));

					/* set msk_ctrl */
					ca_aal_rule.action.t2_ctrl_vld = 1;
					ca_aal_rule.action.t2_ctrl = ca_aal_profile_idx;
					ca_aal_rule.action.dpid_vld = 1;
					if(ca_aal_profile_idx == CL_RUL_PROFILE_WAN)
						ca_aal_rule.action.permit = 0;	// set drop packets
					else
						ca_aal_rule.action.permit = 1;
					ca_aal_rslt_idx = aal_l3_cls_default_set(RTK_ASIC_DEVID, ca_aal_profile_idx, CL_RSLT_TYPE_0, &ca_aal_rule.action);
					if (ca_aal_rslt_idx != AAL_E_OK) {
						WARNING("add special ACL RESET_L2_L3_CLS (L3 CLS %s DEF FIB) failed!!!, ret = %d", ca_aal_profile_idx?"LAN":"WAN", ca_aal_rslt_idx);
						break;
					}
				}
#endif
			}
			break;

		case RTK_CLS_TYPE_L2_INGRESS_FORWARD_PORT:
			{
				aal_ilpb_cfg_msk_t ilpb_cfg_msk={0};
				aal_ilpb_cfg_t  ilpb_cfg;
				memset(&ilpb_cfg_msk, 0, sizeof(aal_ilpb_cfg_msk_t));
				memset(&ilpb_cfg, 0, sizeof(aal_ilpb_cfg_t));
				ilpb_cfg_msk.s.cls_start = TRUE;
				ilpb_cfg_msk.s.cls_length = TRUE;
				for(i = ASIC_LPORT_ETH_NI0; i <= ASIC_LPORT_ETH_NI7; i++){
					if((0x1<<i) & (fc_db.wanPortMask.portmask | (0x1<<RTK_FC_MAC_PORT_PON))){
						ilpb_cfg.cls_start = fc_db.cls_l2_port_info[1][0];
						ilpb_cfg.cls_length = fc_db.cls_l2_port_info[1][1];
					}else if((fc_db.acl_lan_portmask != RTK_FC_RT_ACL_PORTMASK_INVALID) && ((0x1<<i) & fc_db.acl_lan_portmask)){
						ilpb_cfg.cls_start = fc_db.cls_l2_port_info[3][0];
						ilpb_cfg.cls_length = fc_db.cls_l2_port_info[3][1];
					}else{
						ilpb_cfg.cls_start = fc_db.cls_l2_port_info[0][0];
						ilpb_cfg.cls_length = fc_db.cls_l2_port_info[0][1];
					}
					ASSERT_EQ(aal_port_ilpb_cfg_set(RTK_ASIC_DEVID, i, ilpb_cfg_msk, &ilpb_cfg), CA_E_OK);
				}
				ACL_RSV("update L2_INGRESS_FORWARD_PORT success.");
			}
			break;

		case RTK_CLS_TYPE_L2_INGRESS_MC_FORWARD_L3FE:
			{
				rt_switch_mode_t mode = RT_SWITCH_MODE_SFU;
				char mac_addr_v4[ETHER_ADDR_LEN] = {0x01, 0x00, 0x5e, 0, 0, 0};
				char mac_addr_v6[ETHER_ADDR_LEN] = {0x33, 0x33, 0, 0, 0, 0};
				if(RTK_FC_HELPER_RT_SWITCH_MODE_GET(&mode) || (mode == RT_SWITCH_MODE_SFU)){
					ACL_RSV("skip add special L2_INGRESS_MC_FORWARD_L3FE due to switch mode %d is SFU or RT API disable", mode);
					break;
				}
				activePorts.bits[0] = (0x1 << ASIC_LPORT_ETH_NI0)|(0x1 << ASIC_LPORT_ETH_NI7);//force lan/wan during rtk_8277c_acl_init
				CA_SCAN_PORTMASK(i, activePorts.bits[0]){
					//IPv4: MAC DA[47:24] == 01-00-5E => L2 IGR CLS 0.5 entry
					memset(&l2_cls_rule, 0, sizeof(aal_l2_cls_rule_t));
					l2_cls_rule.port = i;
					l2_cls_rule.priority = 3;

					l2_cls_rule.key_mask.l2 = TRUE;
					l2_cls_rule.key_mask.mac_da = TRUE;
					mac_addr_v4[3] = mac_addr_v4[4] = mac_addr_v4[5] = 0;
					memcpy(l2_cls_rule.key.mac_da.mac_min, mac_addr_v4, sizeof(ca_mac_addr_t));
					mac_addr_v4[3] = mac_addr_v4[4] = mac_addr_v4[5] = 0xff;
					memcpy(l2_cls_rule.key.mac_da.mac_max, mac_addr_v4, sizeof(ca_mac_addr_t));
					l2_cls_rule.action.dp_valid = TRUE;
					if(i == ASIC_LPORT_ETH_NI7)
						l2_cls_rule.action.dp_value = AAL_LPORT_L3_WAN;
					else
						l2_cls_rule.action.dp_value = AAL_LPORT_L3_LAN;

					if(_rtk_fc_g3_caAalClsL2RuleAdd(&l2_cls_rule, &ca_aal_rslt_idx, rsvType))
					{
						ACL_RSV("add special L2_INGRESS_MC_FORWARD_L3FE failed!!!");
						addRuleFailedFlag = 1;
						break;
					}
					aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "L2_%s[%d-%d],", (i == ASIC_LPORT_ETH_NI7)?"WAN":"LAN", L2_CLS_TBL_ID(ca_aal_rslt_idx), L2_CLS_KEY_ID(ca_aal_rslt_idx));

					//IPv6: MAC DA[47:32] == 33-33 => L2 IGR CLS 0.5 entry
					memset(&l2_cls_rule, 0, sizeof(aal_l2_cls_rule_t));
					l2_cls_rule.port = i;
					l2_cls_rule.priority = 3;

					l2_cls_rule.key_mask.l2 = TRUE;
					l2_cls_rule.key_mask.mac_da = TRUE;
					mac_addr_v6[2] = mac_addr_v6[3] = mac_addr_v6[4] = mac_addr_v6[5] = 0;
					memcpy(l2_cls_rule.key.mac_da.mac_min, mac_addr_v6, sizeof(ca_mac_addr_t));
					mac_addr_v6[2] = mac_addr_v6[3] = mac_addr_v6[4] = mac_addr_v6[5] = 0xff;
					memcpy(l2_cls_rule.key.mac_da.mac_max, mac_addr_v6, sizeof(ca_mac_addr_t));
					l2_cls_rule.action.dp_valid = TRUE;
					if(i == ASIC_LPORT_ETH_NI7)
						l2_cls_rule.action.dp_value = AAL_LPORT_L3_WAN;
					else
						l2_cls_rule.action.dp_value = AAL_LPORT_L3_LAN;

					if(_rtk_fc_g3_caAalClsL2RuleAdd(&l2_cls_rule, &ca_aal_rslt_idx, rsvType))
					{
						ACL_RSV("add special L2_INGRESS_MC_FORWARD_L3FE failed!!!");
						addRuleFailedFlag = 1;
						break;
					}
					aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "L2_%s[%d-%d],", (i == ASIC_LPORT_ETH_NI7)?"WAN":"LAN", L2_CLS_TBL_ID(ca_aal_rslt_idx), L2_CLS_KEY_ID(ca_aal_rslt_idx));
				}
				ACL_RSV("add special L2_INGRESS_MC_FORWARD_L3FE @ %s", aal_idx_buf);
			}
			break;

		case RTK_CLS_TYPE_L2_EGRESS_FORWARD_L3FE:
			{
				if(fc_db.controlFuc.rt_api_is_enabled){
					activePorts.bits[0] = RTK_FC_ALL_MAC_PORTMASK_WITHOUT_CPU & ~(fc_db.hypridPPTP.portmask);
					CA_SCAN_PORTMASK(i, activePorts.bits[0]) {
						if((0x1<<i) & fc_db.wanPortMask.portmask)
							ca_aal_rslt_idx = AAL_LPORT_L3_WAN;
						else
							ca_aal_rslt_idx = AAL_LPORT_L3_LAN;
						assert_ok(RTK_FC_HELPER_RT_CLS_FWDPORT_SET(i, ca_aal_rslt_idx));
					}
					fc_db.aclAndCfReservedRule.acl_wanPortMask = fc_db.wanPortMask.portmask;
					ACL_RSV("add special L2_EGRESS_FORWARD_L3FE (skip pptp portmaks pptp_port=0x%llx)", fc_db.hypridPPTP.portmask);
				}else{
					ACL_RSV("skip add special L2_EGRESS_FORWARD_L3FE due to rt api disable");
				}
			}
			break;

		case RTK_CLS_TYPE_DUAL_HEADER_DS_CHECK:
			{
				if(parameter == NULL){
					WARNING("add special ACL DUAL_HEADER_DS_CHECK due to invalid parameter failed!!!");
					break;
				}
				memcpy(&dual_hdr_chk,(rtk_fc_aclAndCf_reserved_dual_hdr_chk_t*)parameter,sizeof(rtk_fc_aclAndCf_reserved_dual_hdr_chk_t));
				memset(&dual_hdr_vlan, 0, sizeof(dual_hdr_vlan));
				if(dual_hdr_chk.packet_type >= RTK_FC_CLS_DUAL_MAX){
					WARNING("add special ACL DUAL_HEADER_DS_CHECK due to unknown packet type %d!!!", dual_hdr_chk.packet_type);
					break;
				}

				aal_idx_offset = 0;
				memset(&ca_aal_rule, 0, sizeof(l3_cls_rule_t));
				aal_l3_cls_key_clean(&ca_aal_rule.key);

				ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_HEAD;
				ca_aal_rule.rslt_type = CL_RSLT_TYPE_0;

				switch(dual_hdr_chk.packet_type){
					case RTK_FC_CLS_DUAL_L2TP:	//FLOW KEY
						//packet type=0x6(l2tp_over_udp (data)), ipv4 dip, vlan id, tunnel id, session id
						memcpy(&dual_hdr_vlan, &dual_hdr_chk.l2tp.vlan, sizeof(dual_hdr_vlan));
						ca_aal_rule.key.msk_mdata = ~(0x1ff0);
						ca_aal_rule.key.mdata.mdata_0 = dual_hdr_chk.l2tp.tunnel_id << 16;
						ca_aal_rule.key.mdata.mdata_1 = (RTK_PP_PKT_TYPE_L2TP_UDP_DATA << 16) | (dual_hdr_chk.l2tp.session_id);

						if(dual_hdr_chk.l2tp.is_ipv4){
							ca_aal_rule.key.ip_addr.ip_addr_0 = dual_hdr_chk.l2tp.ipv4_dst_ip;
							ca_aal_rule.key.ip_addr.ip_addr_1 = ca_aal_rule.key.ip_addr.ip_addr_2 = ca_aal_rule.key.ip_addr.ip_addr_3 = 0;
							ca_aal_rule.key.msk_ip_addr_l = 32;
						}else{
							ca_aal_rule.key.ip_addr.ip_addr_0 = dual_hdr_chk.l2tp.ipv6_dst_ip[3];
							ca_aal_rule.key.ip_addr.ip_addr_1 = dual_hdr_chk.l2tp.ipv6_dst_ip[2];
							ca_aal_rule.key.ip_addr.ip_addr_2 = dual_hdr_chk.l2tp.ipv6_dst_ip[1];
							ca_aal_rule.key.ip_addr.ip_addr_3 = dual_hdr_chk.l2tp.ipv6_dst_ip[0];
							ca_aal_rule.key.msk_ip_addr_l = 128 & 0x7f;
							ca_aal_rule.key.msk_ip_addr_h = 128 >> 7;
						}
						ca_aal_rule.key.ip_addr_type = 0;	//0: dip, 1: sip
						break;
					case RTK_FC_CLS_DUAL_VXLAN:
						/* fall through */
					case RTK_FC_CLS_DUAL_VXLAN_NON_IP:
						//packet type=0x7(VxLAN) or 0xd(VxLAN_non_ip), ipv6 dip, vlan id, vni
						memcpy(&dual_hdr_vlan, &dual_hdr_chk.vxlan.vlan, sizeof(dual_hdr_vlan));
						ca_aal_rule.key.msk_mdata = ~(0x1ff0);
						ca_aal_rule.key.mdata.mdata_0 = (dual_hdr_chk.vxlan.vni & 0xffff) << 16;
						if(dual_hdr_chk.packet_type == RTK_FC_CLS_DUAL_VXLAN)
							ca_aal_rule.key.mdata.mdata_1 = (RTK_PP_PKT_TYPE_VXLAN_IP << 16) | (dual_hdr_chk.vxlan.vni >> 16);
						else
							ca_aal_rule.key.mdata.mdata_1 = (RTK_PP_PKT_TYPE_VXLAN_NIP << 16) | (dual_hdr_chk.vxlan.vni >> 16);

						ca_aal_rule.key.ip_addr.ip_addr_0 = dual_hdr_chk.vxlan.dst_ip[3];
						ca_aal_rule.key.ip_addr.ip_addr_1 = dual_hdr_chk.vxlan.dst_ip[2];
						ca_aal_rule.key.ip_addr.ip_addr_2 = dual_hdr_chk.vxlan.dst_ip[1];
						ca_aal_rule.key.ip_addr.ip_addr_3 = dual_hdr_chk.vxlan.dst_ip[0];
						ca_aal_rule.key.msk_ip_addr_l = 128 & 0x7f;
						ca_aal_rule.key.msk_ip_addr_h = 128 >> 7;
						ca_aal_rule.key.ip_addr_type = 0;	//0: dip, 1: sip
						break;
					case RTK_FC_CLS_DUAL_PPTP_GRE:
						//packet type=0x8(PPTP GRE), ipv6 dip, vlan id, gre call id
						memcpy(&dual_hdr_vlan, &dual_hdr_chk.pptp_gre.vlan, sizeof(dual_hdr_vlan));
						ca_aal_rule.key.msk_mdata = ~(0x10f0);
						ca_aal_rule.key.mdata.mdata_0 = dual_hdr_chk.pptp_gre.gre_call_id << 16;
						ca_aal_rule.key.mdata.mdata_1 = RTK_PP_PKT_TYPE_PPTP_GRE << 16;
						if(dual_hdr_chk.pptp_gre.is_ipv4){
							ca_aal_rule.key.ip_addr.ip_addr_0 = dual_hdr_chk.pptp_gre.ipv4_dst_ip;
							ca_aal_rule.key.msk_ip_addr_l = 32;
						}else{
							ca_aal_rule.key.ip_addr.ip_addr_0 = dual_hdr_chk.pptp_gre.ipv6_dst_ip[3];
							ca_aal_rule.key.ip_addr.ip_addr_1 = dual_hdr_chk.pptp_gre.ipv6_dst_ip[2];
							ca_aal_rule.key.ip_addr.ip_addr_2 = dual_hdr_chk.pptp_gre.ipv6_dst_ip[1];
							ca_aal_rule.key.ip_addr.ip_addr_3 = dual_hdr_chk.pptp_gre.ipv6_dst_ip[0];
							ca_aal_rule.key.msk_ip_addr_l = 128 & 0x7f;
							ca_aal_rule.key.msk_ip_addr_h = 128 >> 7;
						}
						ca_aal_rule.key.ip_addr_type = 0;	//0: dip, 1: sip
						break;
					case RTK_FC_CLS_DUAL_MAP_E:
						//packet type=0x9(IPv4 in IPv6), ipv6 dip, vlan id
						memcpy(&dual_hdr_vlan, &dual_hdr_chk.map_e.vlan, sizeof(dual_hdr_vlan));
						ca_aal_rule.key.msk_mdata = ~(0x1000);
						ca_aal_rule.key.mdata.mdata_1 = RTK_PP_PKT_TYPE_IPV4_IN_V6 << 16;

						ca_aal_rule.key.ip_addr.ip_addr_0 = dual_hdr_chk.map_e.dst_ip[3];
						ca_aal_rule.key.ip_addr.ip_addr_1 = dual_hdr_chk.map_e.dst_ip[2];
						ca_aal_rule.key.ip_addr.ip_addr_2 = dual_hdr_chk.map_e.dst_ip[1];
						ca_aal_rule.key.ip_addr.ip_addr_3 = dual_hdr_chk.map_e.dst_ip[0];
						ca_aal_rule.key.msk_ip_addr_l = 128 & 0x7f;
						ca_aal_rule.key.msk_ip_addr_h = 128 >> 7;
						ca_aal_rule.key.ip_addr_type = 0;	//0: dip, 1: sip
						break;
					case RTK_FC_CLS_DUAL_DSLITE:
						//packet type=0x9(IPv4 in IPv6), ipv6 sip, vlan id
						memcpy(&dual_hdr_vlan, &dual_hdr_chk.dslite.vlan, sizeof(dual_hdr_vlan));
						ca_aal_rule.key.msk_mdata = ~(0x1000);
						ca_aal_rule.key.mdata.mdata_1 = RTK_PP_PKT_TYPE_IPV4_IN_V6 << 16;

						ca_aal_rule.key.ip_addr.ip_addr_0 = dual_hdr_chk.dslite.src_ip[3];
						ca_aal_rule.key.ip_addr.ip_addr_1 = dual_hdr_chk.dslite.src_ip[2];
						ca_aal_rule.key.ip_addr.ip_addr_2 = dual_hdr_chk.dslite.src_ip[1];
						ca_aal_rule.key.ip_addr.ip_addr_3 = dual_hdr_chk.dslite.src_ip[0];
						ca_aal_rule.key.msk_ip_addr_l = 128 & 0x7f;
						ca_aal_rule.key.msk_ip_addr_h = 128 >> 7;
						ca_aal_rule.key.ip_addr_type = 1;	//0: dip, 1: sip
						break;
					case RTK_FC_CLS_DUAL_6RD:	//FULL KEY for sip+dip
						//packet type=0xa(IPv6 in IPv4), ipv4 sip, ipv4 dip, vlan id
						memcpy(&dual_hdr_vlan, &dual_hdr_chk.six_rd.vlan, sizeof(dual_hdr_vlan));
						ca_aal_rule.key.msk_mdata = ~(0x1000);
						ca_aal_rule.key.mdata.mdata_1 = RTK_PP_PKT_TYPE_IPV6_IN_V4 << 16;

						ca_aal_rule.key.ip_sa.ip_addr_0 = dual_hdr_chk.six_rd.src_ip;
						ca_aal_rule.key.ip_sa.ip_addr_1 = ca_aal_rule.key.ip_sa.ip_addr_2 = ca_aal_rule.key.ip_sa.ip_addr_3 = 0;
						ca_aal_rule.key.msk_ip_sa_l = 32;

						ca_aal_rule.key.ip_da.ip_addr_0 = dual_hdr_chk.six_rd.dst_ip;
						ca_aal_rule.key.ip_da.ip_addr_1 = ca_aal_rule.key.ip_da.ip_addr_2 = ca_aal_rule.key.ip_da.ip_addr_3 = 0;
						ca_aal_rule.key.msk_ip_da_l = 32;
						break;
					case RTK_FC_CLS_DUAL_MAP_T:
						/* fall through */
					case RTK_FC_CLS_DUAL_MAP_T_TCP_FLAG0:
						//packet type=0xc(IPv6 only), ipv6 dip, vlan id
						//TCP_FLAG0 is 8277C only
						memcpy(&dual_hdr_vlan, &dual_hdr_chk.map_t.vlan, sizeof(dual_hdr_vlan));
						ca_aal_rule.key.msk_mdata = ~(0x1000);
						ca_aal_rule.key.mdata.mdata_1 = RTK_PP_PKT_TYPE_IPV6_ONLY << 16;

						ca_aal_rule.key.ip_addr.ip_addr_0 = dual_hdr_chk.map_t.dst_ip[3];
						ca_aal_rule.key.ip_addr.ip_addr_1 = dual_hdr_chk.map_t.dst_ip[2];
						ca_aal_rule.key.ip_addr.ip_addr_2 = dual_hdr_chk.map_t.dst_ip[1];
						ca_aal_rule.key.ip_addr.ip_addr_3 = dual_hdr_chk.map_t.dst_ip[0];
						ca_aal_rule.key.msk_ip_addr_l = 128 & 0x7f;
						ca_aal_rule.key.msk_ip_addr_h = 128 >> 7;
						ca_aal_rule.key.ip_addr_type = 0;	//0: dip, 1: sip

#if defined(CONFIG_FC_RTL8277C_SERIES)
						if(dual_hdr_chk.packet_type == RTK_FC_CLS_DUAL_MAP_T_TCP_FLAG0){
							ca_aal_rule.pri = RTK_CA_CLS_PRIORITY_L3_15;
							ca_aal_rule.key.msk_tcp_rdp_ctrl = 1;	//1: OR Match, 2: AND Match
							ca_aal_rule.key.tcp_rdp_ctrl = 0x5; 	//tcp flags with rst or fin
							/* TCP FIN/RST match exist flow will cause hash double check failed.
							In this condition, mdata reason will not be updated by hash. Use CLS to update flow miss reason. */
							ca_aal_rule.action.mdata_w_vld_0 = 0x4; // reason: mdata0[11:8]
							ca_aal_rule.action.mdata_w_0 = CPU_REASON_FLOWMISS << RXINFO_REF_HW_RSN_SHIFT_BIT;// resson3: flow miss
							ca_aal_rule.action.keep_orig_pkt = 1;
							ca_aal_rule.action.keep_orig_pkt_vld = 1;
						}
#endif
						break;
#if defined(CONFIG_FC_RTL9607F_SERIES)
					case RTK_FC_CLS_DUAL_SRV6:
						/* fall through */
					case RTK_FC_CLS_DUAL_SRV6_NON_IP:
						//packet type=0xe(SRV6)/0xf(SRV6_non_ip), ipv6 dip, vlan id
						memcpy(&dual_hdr_vlan, &dual_hdr_chk.srv6.vlan, sizeof(dual_hdr_vlan));
						ca_aal_rule.key.msk_mdata = ~(0x1000);
						if(dual_hdr_chk.packet_type == RTK_FC_CLS_DUAL_SRV6)
							ca_aal_rule.key.mdata.mdata_1 = RTK_PP_PKT_TYPE_SRV6_IP << 16;
						else
							ca_aal_rule.key.mdata.mdata_1 = RTK_PP_PKT_TYPE_SRV6_NIP << 16;

						ca_aal_rule.key.ip_addr.ip_addr_0 = dual_hdr_chk.srv6.dst_ip[3];
						ca_aal_rule.key.ip_addr.ip_addr_1 = dual_hdr_chk.srv6.dst_ip[2];
						ca_aal_rule.key.ip_addr.ip_addr_2 = dual_hdr_chk.srv6.dst_ip[1];
						ca_aal_rule.key.ip_addr.ip_addr_3 = dual_hdr_chk.srv6.dst_ip[0];
						ca_aal_rule.key.msk_ip_addr_l = 128 & 0x7f;
						ca_aal_rule.key.msk_ip_addr_h = 128 >> 7;
						ca_aal_rule.key.ip_addr_type = 0;	//0: dip, 1: sip
						break;
					case RTK_FC_CLS_DUAL_L2GRE:
						/* fall through */
					case RTK_FC_CLS_DUAL_L2GRE_NON_IP:	//FULL KEY for sip+dip
						//packet type=0x10(L2GRE)/0x11(L2GRE_non_ip), ipv4 sip, ipv4 dip, vlan id
						memcpy(&dual_hdr_vlan, &dual_hdr_chk.l2_gre.vlan, sizeof(dual_hdr_vlan));
						ca_aal_rule.key.msk_mdata = ~(0x1000);
						if(dual_hdr_chk.packet_type == RTK_FC_CLS_DUAL_L2GRE)
							ca_aal_rule.key.mdata.mdata_1 = RTK_PP_PKT_TYPE_L2GRE_IP << 16;
						else
							ca_aal_rule.key.mdata.mdata_1 = RTK_PP_PKT_TYPE_L2GRE_NIP << 16;

						if(dual_hdr_chk.l2_gre.is_ipv4){
							ca_aal_rule.key.ip_sa.ip_addr_0 = dual_hdr_chk.l2_gre.ipv4_src_ip;
							ca_aal_rule.key.ip_sa.ip_addr_1 = ca_aal_rule.key.ip_sa.ip_addr_2 = ca_aal_rule.key.ip_sa.ip_addr_3 = 0;
							ca_aal_rule.key.msk_ip_sa_l = 32;

							ca_aal_rule.key.ip_da.ip_addr_0 = dual_hdr_chk.l2_gre.ipv4_dst_ip;
							ca_aal_rule.key.ip_da.ip_addr_1 = ca_aal_rule.key.ip_da.ip_addr_2 = ca_aal_rule.key.ip_da.ip_addr_3 = 0;
							ca_aal_rule.key.msk_ip_da_l = 32;
						}else{
							ca_aal_rule.key.ip_sa.ip_addr_0 = dual_hdr_chk.l2_gre.ipv6_src_ip[3];
							ca_aal_rule.key.ip_sa.ip_addr_1 = dual_hdr_chk.l2_gre.ipv6_src_ip[2];
							ca_aal_rule.key.ip_sa.ip_addr_2 = dual_hdr_chk.l2_gre.ipv6_src_ip[1];
							ca_aal_rule.key.ip_sa.ip_addr_3 = dual_hdr_chk.l2_gre.ipv6_src_ip[0];
							ca_aal_rule.key.msk_ip_sa_l = 128 & 0x7f;
							ca_aal_rule.key.msk_ip_sa_h = 128 >> 7;

							ca_aal_rule.key.ip_da.ip_addr_0 = dual_hdr_chk.l2_gre.ipv6_dst_ip[3];
							ca_aal_rule.key.ip_da.ip_addr_1 = dual_hdr_chk.l2_gre.ipv6_dst_ip[2];
							ca_aal_rule.key.ip_da.ip_addr_2 = dual_hdr_chk.l2_gre.ipv6_dst_ip[1];
							ca_aal_rule.key.ip_da.ip_addr_3 = dual_hdr_chk.l2_gre.ipv6_dst_ip[0];
							ca_aal_rule.key.msk_ip_da_l = 128 & 0x7f;
							ca_aal_rule.key.msk_ip_da_h = 128 >> 7;
						}
						break;
					case RTK_FC_CLS_DUAL_XLAT:	//FULL KEY for sip+dip
						//packet type=0xc(IPv6 only), ipv6 sip with msb X bits, ipv6 dip with msb X bits, vlan id
						//ip_subnet_size usage example: d24 compares bits 31:8 of an IPv4 address and bits 127:104 of an IPv6 address
						memcpy(&dual_hdr_vlan, &dual_hdr_chk.xlat.vlan, sizeof(dual_hdr_vlan));
						ca_aal_rule.key.msk_mdata = ~(0x1000);
						ca_aal_rule.key.mdata.mdata_1 = RTK_PP_PKT_TYPE_IPV6_ONLY << 16;

						ca_aal_rule.key.ip_sa.ip_addr_0 = dual_hdr_chk.xlat.src_ip[3];
						ca_aal_rule.key.ip_sa.ip_addr_1 = dual_hdr_chk.xlat.src_ip[2];
						ca_aal_rule.key.ip_sa.ip_addr_2 = dual_hdr_chk.xlat.src_ip[1];
						ca_aal_rule.key.ip_sa.ip_addr_3 = dual_hdr_chk.xlat.src_ip[0];
						ca_aal_rule.key.msk_ip_sa_l = dual_hdr_chk.xlat.src_ip_subnet_size & 0x7f;
						ca_aal_rule.key.msk_ip_sa_h = dual_hdr_chk.xlat.src_ip_subnet_size >> 7;

						ca_aal_rule.key.ip_da.ip_addr_0 = dual_hdr_chk.xlat.dst_ip[3];
						ca_aal_rule.key.ip_da.ip_addr_1 = dual_hdr_chk.xlat.dst_ip[2];
						ca_aal_rule.key.ip_da.ip_addr_2 = dual_hdr_chk.xlat.dst_ip[1];
						ca_aal_rule.key.ip_da.ip_addr_3 = dual_hdr_chk.xlat.dst_ip[0];
						ca_aal_rule.key.msk_ip_da_l = dual_hdr_chk.xlat.dst_ip_subnet_size & 0x7f;
						ca_aal_rule.key.msk_ip_da_h = dual_hdr_chk.xlat.dst_ip_subnet_size >> 7;
						break;
#endif
					default:
						break;
				}

				ca_aal_rule.action.t2_ctrl = dual_hdr_chk.hash_tuple_idx;	//RTK_ASIC_FLOW_PROFILE_FLOW_5TUPLE or RTK_ASIC_FLOW_PROFILE_FLOW_2TUPLE
				ca_aal_rule.action.t2_ctrl_vld = 1;

				ca_aal_rule.key.msk_ip_fragment = 0;
				ca_aal_rule.key.ip_fragment = 0;	//dual header acc only support non-fragment

				if(dual_hdr_vlan.vlan_check_en){
					ca_aal_rule.key.msk_vlan_cnt = 0;
					if((dual_hdr_vlan.has_stag) && (dual_hdr_vlan.has_ctag)){	//s+c
						ca_aal_rule.key.vlan_cnt = 2;
						ca_aal_rule.key.vid_ctl = 0;	//00 : Top vid is exact matched
						ca_aal_rule.key.top_vid = dual_hdr_vlan.svlan_id;
						ca_aal_rule.key.msk_inner_vid = 0;
						ca_aal_rule.key.inner_vid = dual_hdr_vlan.cvlan_id;
					}else if(dual_hdr_vlan.has_stag){	//top s
						ca_aal_rule.key.vlan_cnt = 1;
						ca_aal_rule.key.vid_ctl = 0;
						ca_aal_rule.key.top_vid = dual_hdr_vlan.svlan_id;
					}else if(dual_hdr_vlan.has_ctag){	//top c
						ca_aal_rule.key.vlan_cnt = 1;
						ca_aal_rule.key.vid_ctl = 0;
						ca_aal_rule.key.top_vid = dual_hdr_vlan.cvlan_id;
					}else
						ca_aal_rule.key.vlan_cnt = 0;	//untag
				}

				ca_aal_rule.action.igr_l3_if_idx_vld = 1;
				ca_aal_rule.action.igr_l3_if_idx = dual_hdr_chk.igr_intf_idx;

				ca_aal_rule.action.mdata_w_vld_0 |= 0x8;
				ca_aal_rule.action.mdata_w_0 |= RXINFO_REF_TRAP_RSN_DUAL_DS << RXINFO_REF_TRAP_RSN_SHIFT_BIT;	//sw reason for MDATAL_REASON_SW

				//make sure flow miss trap
				_rtk_fc_g3_caAalClsAssignHashProfileAction(&ca_aal_rule, dual_hdr_chk.profile_idx, FALSE, 0);

				if(_rtk_fc_g3_caAalClsRuleAdd(dual_hdr_chk.profile_idx, &ca_aal_rule, &ca_aal_rslt_idx, rsvType))
				{
					WARNING("add special ACL DUAL_HEADER_DS_CHECK for packet type %d failed!!!", dual_hdr_chk.packet_type);
					break;
				}
				aal_idx_offset += snprintf(aal_idx_buf+aal_idx_offset, sizeof(aal_idx_buf)-aal_idx_offset, "%s[%d-%d],", dual_hdr_chk.profile_idx?"LAN":"WAN", CLS_TBL_ID(ca_aal_rslt_idx), CLS_KEY_ID(ca_aal_rslt_idx));
				TRACE("add CLS DUAL_HEADER_DS_CHECK for pkt type %d success @ %s", dual_hdr_chk.packet_type, aal_idx_buf);
				ACL_RSV("add special DUAL_HEADER_DS_CHECK for pkt type %d @ %s", dual_hdr_chk.packet_type, aal_idx_buf);
				((rtk_fc_aclAndCf_reserved_dual_hdr_chk_t*)parameter)->rslt_idx = ca_aal_rslt_idx;
			}
			break;

#endif

		default:
			ACL_RSV("#####Unknown special ACL (rsvType=%d)!!!#####",rsvType);
			WARNING("#####Unknown special ACL (rsvType=%d)!!!#####",rsvType);
			break;
	}

	if(addRuleFailedFlag==1){
		WARNING("add Reserve ACL[%d] failed!!!", rsvType);
		return (RT_ERR_RG_FAILED);
	}

	return (RT_ERR_RG_OK);
}


int _rtk_rg_aclAndCfReservedRuleDelSpecial(rtk_rg_aclAndCf_reserved_type_t rsvType, void *parameter)
{
#if defined(CONFIG_FC_CA8277B_SERIES)
	int loop_idx, loop_start, loop_end;
	rtk_rg_asic_netif_ref_t asicNetif;
#endif

	ACL_RSV("#####Delete special ACL!(rsvType=%d)#####", rsvType);

	switch(rsvType){

		case RTK_CA_CLS_TYPE_BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE_GW_MAC_UPDATE:
			{
#if defined(CONFIG_FC_CA8277B_SERIES)
				if(parameter == NULL){
					ACL_RSV("Delete special ACL type %d failed due to invalid parameter!!!", rsvType);
					break;
				}
				loop_idx = *((uint32*)parameter);
				if(loop_idx == RTK_FC_TABLESIZE_INTF_ACC_ACL){	//force disable
					loop_start = 0;
					loop_end = RTK_FC_TABLESIZE_INTF_ACC_ACL;
				}else if(loop_idx > RTK_FC_TABLESIZE_INTF_ACC_ACL){
					ACL_RSV("delete special ACL BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE_GW_MAC_UPDATE failed due to invalid parameter (netif idx %d)!!!", loop_idx);
					break;
				}else{
					loop_start = loop_idx;
					loop_end = loop_idx+1;
				}
				for(loop_idx = loop_start; loop_idx < loop_end; loop_idx++){
					memset(&asicNetif, 0x0, sizeof(asicNetif));
					assert_ok(rtk_rg_asic_netifTable_get(loop_idx, &asicNetif));
					if(asicNetif.valid == 0)
						continue;
					if(asicNetif.mac_idx != CA_UINT32_INVALID){
						ACL_RSV("skip delete special ACL BRIDGE_5TUPLE_FLOW_ACCELERATE_BY_2TUPLE_GW_MAC_UPDATE due to unnecessary (netif idx %d, mac_idx %d, aclInfo 0x%x)!!!", loop_idx, asicNetif.mac_idx, asicNetif.acl_info_flow_acc);
						continue;
					}
					if((asicNetif.acl_info_flow_acc != CA_UINT32_INVALID) && (asicNetif.acl_info_flow_acc != 0)){
						ACL_RSV("delete special ACL[%d] netif %d aclLan 0x%x aclWan 0x%x", rsvType, loop_idx, asicNetif.acl_info_flow_acc, asicNetif.acl_info_flow_acc_wan);
						_rtk_fc_g3_caAalClsRuleDeleteRsltIdx(rsvType, asicNetif.acl_info_flow_acc);
						_rtk_fc_g3_caAalClsRuleDeleteRsltIdx(rsvType, asicNetif.acl_info_flow_acc_wan);
						asicNetif.acl_info_flow_acc = CA_UINT32_INVALID;
						asicNetif.acl_info_flow_acc_wan = CA_UINT32_INVALID;
						assert_ok(rtk_rg_asic_netifTable_set(loop_idx, &asicNetif));
					}else
						WARNING("unknown status: netif_idx %d, valid %d, mac_idx %d, aclInfo 0x%x", loop_idx, asicNetif.valid, asicNetif.mac_idx, asicNetif.acl_info_flow_acc);
				}
#else
				ACL_RSV("Skip delete special ACL type %d due to chip not support.", rsvType);
				break;
#endif
			}
			break;

		case RTK_CA_CLS_TYPE_GENERIC_INTF_MC_PROFILE_WITH_DMAC:
		case RTK_CA_CLS_TYPE_VXLAN_FFD_US:
		case RTK_CA_CLS_TYPE_VXLAN_FFD_DS:
		case RTK_CA_CLS_TYPE_VXLAN_FFD_EXTRA_DS:
		case RTK_CLS_TYPE_HTTP_ACCELERATE_BY_PE:
		case RTK_CLS_TYPE_SRV6_ACCELERATE_BY_PE:
		case RTK_CLS_TYPE_DUAL_HEADER_DS_CHECK:
			if(parameter != NULL)
				_rtk_fc_g3_caAalClsRuleDeleteRsltIdx(rsvType, *((uint32*)parameter));
			break;
		case RTK_CLS_TYPE_L2_INGRESS_FORWARD_PORT:
			break;

		default:
			_rtk_fc_g3_caAalClsRuleDeleteReserveType(rsvType);
			break;
	}
	fc_db.aclAndCfReservedRule.reservedMask[rsvType]=DISABLED;

	return (RT_ERR_RG_OK);
}
#endif	//CONFIG_RTK_L34_G3_PLATFORM


