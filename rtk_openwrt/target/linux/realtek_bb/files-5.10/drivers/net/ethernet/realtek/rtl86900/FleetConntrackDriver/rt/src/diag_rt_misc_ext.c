/*
 * Copyright (C) 2018 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 93636 $
 * $Date: 2018-11-16 12:52:37 +0800 (Fri, 16 Nov 2018) $
 *
 * Purpose : Definition of RT API of Rate Extension
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Configuration of host policer
 *
 */


/*
 * Include Files
 */

#include <stdio.h>
#include <string.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <diag_util.h>
#include <diag_str.h>
#include <parser/cparser_priv.h>
#include <osal/memory.h>	//for osal_malloc, osal_free

#include <rt_misc_ext.h>

static inline int _diag_test_bit(unsigned int nr, void *addr)
{
	return 1UL & (((unsigned int *) addr)[nr>>5] >> (nr&0x1f));
}

void _diag_vlanGroupMacLimitInfoShow(rt_misc_vlanGroupMacLimit_info_t groupInfo)
{
	int i;

	diag_util_mprintf("Valid:%d\n",groupInfo.valid);
	diag_util_mprintf("Port:%d\n",groupInfo.port);
	diag_util_mprintf("Limit number:%d\n",groupInfo.mac_limit_number);
	//diag_util_mprintf("Count:%d\n",groupInfo.mac_count);
	diag_util_mprintf("Group:\n");
	if(groupInfo.untag)diag_util_mprintf("\tuntag\n");
	for(i=0;i<RT_MISC_MAX_VLAN_HW_TABLE_SIZE;i++)
	{
		if(_diag_test_bit(i&0x1f,(void *)&groupInfo.vlanMask[i>>5]))
		{
			diag_util_mprintf("\tvlan %d\n",i);
		}
	}
}

/*
 * rt_misc set macAddrLearningLimit wlan_idx <UINT:wlan_idx> device_idx <UINT:device_idx> learningLimitNumber <UINT:learningLimitNumber>
 */
cparser_result_t
cparser_cmd_rt_misc_set_macAddrLearningLimit_wlan_idx_wlan_idx_device_idx_device_idx_learningLimitNumber_learningLimitNumber(
    cparser_context_t *context,
    uint32_t  *wlan_idx_ptr,
    uint32_t  *device_idx_ptr,
    uint32_t  *learningLimitNumber_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_misc_macAddr_learning_limit_t macAddrLearningLimit;
		
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&macAddrLearningLimit, 0x0, sizeof(rt_misc_macAddr_learning_limit_t));
	macAddrLearningLimit.learningLimitNumber = *learningLimitNumber_ptr;
	
	ret = rt_misc_wlan_macAddr_learning_limit_set(*wlan_idx_ptr, *device_idx_ptr, macAddrLearningLimit);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("Fail to set macAddr learning limit of wlan%d device[%d]. RT API return 0x%x \n", *wlan_idx_ptr, *device_idx_ptr, ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set macAddr learning limit of wlan%d device[%d] successfully: \n", *wlan_idx_ptr, *device_idx_ptr);
		diag_util_mprintf("\tMacAddr learning limit number of wlan%d device[%d] = %d  (-1 means unlimit)\n", *wlan_idx_ptr, *device_idx_ptr, macAddrLearningLimit.learningLimitNumber);
	}
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_misc_set_macaddrlearninglimit_wlan_idx_wlan_idx_device_idx_device_idx_learninglimitnumber_learninglimitnumber */

/*
 * rt_misc set macAddrLearningLimit wlan_idx <UINT:wlan_idx> device_idx <UINT:device_idx> unlimit
 */
cparser_result_t
cparser_cmd_rt_misc_set_macAddrLearningLimit_wlan_idx_wlan_idx_device_idx_device_idx_unlimit(
    cparser_context_t *context,
    uint32_t  *wlan_idx_ptr,
    uint32_t  *device_idx_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_misc_macAddr_learning_limit_t macAddrLearningLimit;
		
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&macAddrLearningLimit, 0x0, sizeof(rt_misc_macAddr_learning_limit_t));
	macAddrLearningLimit.learningLimitNumber = -1;
	
	ret = rt_misc_wlan_macAddr_learning_limit_set(*wlan_idx_ptr, *device_idx_ptr, macAddrLearningLimit);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("Fail to set macAddr learning limit of wlan%d device[%d]. RT API return 0x%x \n", *wlan_idx_ptr, *device_idx_ptr, ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set macAddr learning limit of wlan%d device[%d] successfully: \n", *wlan_idx_ptr, *device_idx_ptr);
		diag_util_mprintf("\tMacAddr learning limit number of wlan%d device[%d] = %d  (-1 means unlimit)\n", *wlan_idx_ptr, *device_idx_ptr, macAddrLearningLimit.learningLimitNumber);
	}
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rt_misc_set_macaddrlearninglimit_wlan_idx_wlan_idx_device_idx_device_idx_unlimit */

/*
 * rt_misc get macAddrLearningLimit wlan_idx <UINT:wlan_idx> device_idx <UINT:device_idx>
 */
cparser_result_t
cparser_cmd_rt_misc_get_macAddrLearningLimit_wlan_idx_wlan_idx_device_idx_device_idx(
    cparser_context_t *context,
    uint32_t  *wlan_idx_ptr,
    uint32_t  *device_idx_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_misc_macAddr_learning_limit_t macAddrLearningLimit;
	
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&macAddrLearningLimit, 0x0, sizeof(rt_misc_macAddr_learning_limit_t));
	
	ret = rt_misc_wlan_macAddr_learning_limit_get(*wlan_idx_ptr, *device_idx_ptr, &macAddrLearningLimit);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("Fail to get macAddr learning limit of wlan%d device[%d]. RT API return 0x%x \n", *wlan_idx_ptr, *device_idx_ptr, ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("MacAddr learning limit number of wlan%d device[%d] = %d (-1 means unlimit)\n", *wlan_idx_ptr, *device_idx_ptr, macAddrLearningLimit.learningLimitNumber);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_misc_get_macaddrlearninglimit_wlan_idx_wlan_idx_device_idx_device_idx */

/*
 * rt_misc set wanAccessLimit portmask <HEX:portmask> wlan_idx_0 <HEX:wlan0_mbssid_mask> limitNumber <UINT:limitNumber> limitField <UINT:field>
 */
cparser_result_t
cparser_cmd_rt_misc_set_wanAccessLimit_portmask_portmask_wlan_idx_0_wlan0_mbssid_mask_limitNumber_limitNumber_limitField_field(
	cparser_context_t *context,
	uint32_t  *portmask_ptr,
	uint32_t  *wlan0_mbssid_mask_ptr,
	uint32_t  *limitNumber_ptr,
	uint32_t  *field_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_misc_wan_access_limit_t wan_access_limit;
		
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&wan_access_limit, 0x0, sizeof(rt_misc_wan_access_limit_t));
	wan_access_limit.portMask=*portmask_ptr;
	wan_access_limit.wlanMask[0]=*wlan0_mbssid_mask_ptr;
	wan_access_limit.accessLimitNumber=*limitNumber_ptr;
	wan_access_limit.limitField=*field_ptr;
	
	ret = rt_misc_wan_access_limit_set(wan_access_limit);
	if(ret != RT_ERR_OK)
	{
		diag_util_mprintf("Fail to set wan access limit. RT API return 0x%x \n", ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		diag_util_mprintf("Set wan access limit successfully: \n");
	}
	
	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_misc_set_wanaccesslimit_portmask_portmask_wlan_idx_0_wlan0_mbssid_mask_limitnumber_limitnumber_limitfield_field */

/*
 * rt_misc set wanAccessLimit portmask <HEX:portmask> wlan_idx_0 <HEX:wlan0_mbssid_mask> wlan_idx_1 <HEX:wlan1_mbssid_mask> limitNumber <UINT:limitNumber> limitField <UINT:field>
 */
cparser_result_t
cparser_cmd_rt_misc_set_wanAccessLimit_portmask_portmask_wlan_idx_0_wlan0_mbssid_mask_wlan_idx_1_wlan1_mbssid_mask_limitNumber_limitNumber_limitField_field(
	cparser_context_t *context,
	uint32_t  *portmask_ptr,
	uint32_t  *wlan0_mbssid_mask_ptr,
	uint32_t  *wlan1_mbssid_mask_ptr,
	uint32_t  *limitNumber_ptr,
	uint32_t  *field_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_misc_wan_access_limit_t wan_access_limit;
		
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&wan_access_limit, 0x0, sizeof(rt_misc_wan_access_limit_t));
	wan_access_limit.portMask=*portmask_ptr;
	wan_access_limit.wlanMask[0]=*wlan0_mbssid_mask_ptr;
	wan_access_limit.wlanMask[1]=*wlan1_mbssid_mask_ptr;
	wan_access_limit.accessLimitNumber=*limitNumber_ptr;
	wan_access_limit.limitField=*field_ptr;
	
	ret = rt_misc_wan_access_limit_set(wan_access_limit);
	if(ret != RT_ERR_OK)
	{
		diag_util_mprintf("Fail to set wan access limit. RT API return 0x%x \n", ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		diag_util_mprintf("Set wan access limit successfully: \n");
	}
	
	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_misc_set_wanaccesslimit_portmask_portmask_wlan_idx_0_wlan0_mbssid_mask_wlan_idx_1_wlan1_mbssid_mask_limitnumber_limitnumber_limitfield_field */

/*
 * rt_misc set wanAccessLimit portmask <HEX:portmask> wlan_idx_0 <HEX:wlan0_mbssid_mask> wlan_idx_1 <HEX:wlan1_mbssid_mask> wlan_idx_2 <HEX:wlan2_mbssid_mask> limitNumber <UINT:limitNumber> limitField <UINT:field>
 */
cparser_result_t
cparser_cmd_rt_misc_set_wanAccessLimit_portmask_portmask_wlan_idx_0_wlan0_mbssid_mask_wlan_idx_1_wlan1_mbssid_mask_wlan_idx_2_wlan2_mbssid_mask_limitNumber_limitNumber_limitField_field(
	cparser_context_t *context,
	uint32_t  *portmask_ptr,
	uint32_t  *wlan0_mbssid_mask_ptr,
	uint32_t  *wlan1_mbssid_mask_ptr,
	uint32_t  *wlan2_mbssid_mask_ptr,
	uint32_t  *limitNumber_ptr,
	uint32_t  *field_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_misc_wan_access_limit_t wan_access_limit;
		
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&wan_access_limit, 0x0, sizeof(rt_misc_wan_access_limit_t));
	wan_access_limit.portMask=*portmask_ptr;
	wan_access_limit.wlanMask[0]=*wlan0_mbssid_mask_ptr;
	wan_access_limit.wlanMask[1]=*wlan1_mbssid_mask_ptr;
	wan_access_limit.wlanMask[2]=*wlan2_mbssid_mask_ptr;
	wan_access_limit.accessLimitNumber=*limitNumber_ptr;
	wan_access_limit.limitField=*field_ptr;
	
	ret = rt_misc_wan_access_limit_set(wan_access_limit);
	if(ret != RT_ERR_OK)
	{
		diag_util_mprintf("Fail to set wan access limit. RT API return 0x%x \n", ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		diag_util_mprintf("Set wan access limit successfully: \n");
	}
	
	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_misc_set_wanaccesslimit_portmask_portmask_wlan_idx_0_wlan0_mbssid_mask_wlan_idx_1_wlan1_mbssid_mask_wlan_idx_2_wlan2_mbssid_mask_limitnumber_limitnumber_limitfield_field */

/*
 * rt_misc set wanAccessLimit unlimit
 */
cparser_result_t
cparser_cmd_rt_misc_set_wanAccessLimit_unlimit(
	cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_misc_wan_access_limit_t wan_access_limit;
		
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&wan_access_limit, 0x0, sizeof(rt_misc_wan_access_limit_t));
	wan_access_limit.accessLimitNumber = -1;
	
	ret = rt_misc_wan_access_limit_set(wan_access_limit);
	if(ret != RT_ERR_OK)
	{
		diag_util_mprintf("Fail to set wan access limit. RT API return 0x%x \n", ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		diag_util_mprintf("Set wan access limit as unlimit successfully: \n");
	}
	
	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_misc_set_wanaccesslimit_unlimit */

/*
 * rt_misc get wanAccessLimit 
 */
cparser_result_t
cparser_cmd_rt_misc_get_wanAccessLimit(
	cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED,i;
	rt_misc_wan_access_limit_t wan_access_limit;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&wan_access_limit, 0x0, sizeof(rt_misc_wan_access_limit_t));
	
	ret = rt_misc_wan_access_limit_get(&wan_access_limit);
	if(ret != RT_ERR_OK)
	{
		diag_util_mprintf("Fail to get wan access limit. RT API return 0x%x \n", ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		diag_util_mprintf("Wan access limit:\n", wan_access_limit.portMask);
		if(wan_access_limit.accessLimitNumber>=0){
			diag_util_mprintf(" pmsk=0x%x\n", wan_access_limit.portMask);
			for(i=0;i<RT_WLAN_DEVICE_MAX;i++)
				diag_util_mprintf(" wlan%dmsk=0x%x\n", i, wan_access_limit.wlanMask[i]);
			diag_util_mprintf(" limit=%d\n",wan_access_limit.accessLimitNumber);
			diag_util_mprintf(" limit field:%s\n",wan_access_limit.limitField==RT_MISC_WAN_ACCESS_LIMIT_BY_MAC?"By MAC":"By IP");
		}else
			diag_util_mprintf(" limit:unlimit\n");
	}

	return CPARSER_OK;
}	 /* end of cparser_cmd_rt_misc_get_wanaccesslimit */

/*
 * rt_misc set burstPacket destPort <UINT:destPort> burstCount <UINT:burstCount> burstRate <UINT:burstRate> pktLen <UINT:pktLen> pktData <STRING:pktData>
 */
cparser_result_t
cparser_cmd_rt_misc_set_burstPacket_destPort_destPort_burstCount_burstCount_burstRate_burstRate_pktLen_pktLen_pktData_pktData(
	cparser_context_t *context,
	uint32_t  *destPort_ptr,
	uint32_t  *burstCount_ptr,
	uint32_t  *burstRate_ptr,
	uint32_t  *pktLen_ptr,
	char * *pktData_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_misc_burst_packet_t burst_packet;
	unsigned char *pktData;
		
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK(RT_MISC_JUMBO_BUF_SIZE <= *pktLen_ptr, RT_ERR_FAILED);
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&burst_packet, 0x0, sizeof(rt_misc_burst_packet_t));
	burst_packet.destPort=*destPort_ptr;
	burst_packet.pktData=*pktData_ptr;
	burst_packet.pktLen=*pktLen_ptr;
	burst_packet.burstCount=*burstCount_ptr;
	burst_packet.burstRate=*burstRate_ptr;

	pktData=(unsigned char *)osal_alloc(burst_packet.pktLen);
	if(pktData){
		int i;
		unsigned char temp[2]={0},result;
		//change from ASCII to HEX
		for(i=0;i<(burst_packet.pktLen<<1);i++){
			temp[0]=burst_packet.pktData[i];
			result=strtol(temp,NULL,16)&0xf;
			if(i%2==0){
				pktData[i>>1]=result<<4;
			}else{
				pktData[i>>1]|=result;
			}
		}
		burst_packet.pktData=pktData;
		ret = rt_misc_burst_packet_set(burst_packet);
		osal_free(pktData);
		if(ret != RT_ERR_OK){
			diag_util_mprintf("Fail to set burst packet. RT API return 0x%x \n", ret);
		}else{
			diag_util_mprintf("Set burst packet successfully. \n");
			return CPARSER_OK;
		}
	}
	return CPARSER_NOT_OK;
}	/* end of cparser_cmd_rt_misc_set_burstPacket_destPort_destPort_burstCount_burstCount_burstRate_burstRate_pktLen_pktLen_pktData_pktData */

/*
 * rt_misc set burstPacket destPort gpon streamID <UINT:streamID> burstCount <UINT:burstCount> burstRate <UINT:burstRate> pktLen <UINT:pktLen> pktData <STRING:pktData>
 */
cparser_result_t
cparser_cmd_rt_misc_set_burstPacket_destPort_gpon_streamID_streamID_burstCount_burstCount_burstRate_burstRate_pktLen_pktLen_pktData_pktData(
	cparser_context_t *context,
	uint32_t  *streamID_ptr,
	uint32_t  *burstCount_ptr,
	uint32_t  *burstRate_ptr,
	uint32_t  *pktLen_ptr,
	char * *pktData_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_misc_burst_packet_t burst_packet;
	unsigned char *pktData;
		
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK(RT_MISC_JUMBO_BUF_SIZE <= *pktLen_ptr, RT_ERR_FAILED);
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&burst_packet, 0x0, sizeof(rt_misc_burst_packet_t));
	burst_packet.destPort=RT_PORT_PON;
	burst_packet.dest_stream_id=*streamID_ptr;
	burst_packet.pktData=*pktData_ptr;
	burst_packet.pktLen=*pktLen_ptr;
	burst_packet.burstCount=*burstCount_ptr;
	burst_packet.burstRate=*burstRate_ptr;

	pktData=(unsigned char *)osal_alloc(burst_packet.pktLen);
	if(pktData){
		int i;
		unsigned char temp[2]={0},result;
		//change from ASCII to HEX
		for(i=0;i<(burst_packet.pktLen<<1);i++){
			temp[0]=burst_packet.pktData[i];				
			result=strtol(temp,NULL,16)&0xf;
			if(i%2==0){
				pktData[i>>1]=result<<4;
			}else{
				pktData[i>>1]|=result;
			}
		}
		burst_packet.pktData=pktData;
		ret = rt_misc_burst_packet_set(burst_packet);
		osal_free(pktData);
		if(ret != RT_ERR_OK){
			diag_util_mprintf("Fail to set burst packet. RT API return 0x%x \n", ret);
		}else{
			diag_util_mprintf("Set burst packet successfully. \n");
			return CPARSER_OK;
		}
	}
	return CPARSER_NOT_OK;
}	/* end of cparser_cmd_rt_misc_set_burstPacket_destPort_gpon_streamID_streamID_burstCount_burstCount_burstRate_burstRate_pktLen_pktLen_pktData_pktData */

/*
 * rt_misc set burstPacket destPort <UINT:destPort> infiniteMode burstRate <UINT:burstRate> pktLen <UINT:pktLen> pktData <STRING:pktData>
 */
cparser_result_t
cparser_cmd_rt_misc_set_burstPacket_destPort_destPort_infiniteMode_burstRate_burstRate_pktLen_pktLen_pktData_pktData(
	cparser_context_t *context,
	uint32_t  *destPort_ptr,
	uint32_t  *burstRate_ptr,
	uint32_t  *pktLen_ptr,
	char * *pktData_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_misc_burst_packet_t burst_packet;
	unsigned char *pktData;
		
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK(RT_MISC_JUMBO_BUF_SIZE <= *pktLen_ptr, RT_ERR_FAILED);
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&burst_packet, 0x0, sizeof(rt_misc_burst_packet_t));
	burst_packet.destPort=*destPort_ptr;
	burst_packet.pktData=*pktData_ptr;
	burst_packet.pktLen=*pktLen_ptr;
	burst_packet.burstCount=-1;
	burst_packet.burstRate=*burstRate_ptr;
	
	pktData=(unsigned char *)osal_alloc(burst_packet.pktLen);
	if(pktData){
		int i;
		unsigned char temp[2]={0},result;
		//change from ASCII to HEX
		for(i=0;i<(burst_packet.pktLen<<1);i++){
			temp[0]=burst_packet.pktData[i];				
			result=strtol(temp,NULL,16)&0xf;
			if(i%2==0){
				pktData[i>>1]=result<<4;
			}else{
				pktData[i>>1]|=result;
			}
		}
		burst_packet.pktData=pktData;
		ret = rt_misc_burst_packet_set(burst_packet);
		osal_free(pktData);
		if(ret != RT_ERR_OK){
			diag_util_mprintf("Fail to set burst packet. RT API return 0x%x \n", ret);
		}else{
			diag_util_mprintf("Set burst packet successfully. \n");
			return CPARSER_OK;
		}
	}
	return CPARSER_NOT_OK;
}	/* end of cparser_cmd_rt_misc_set_burstPacket_destPort_destPort_infiniteMode_burstRate_burstRate_pktLen_pktLen_pktData_pktData */

/*
 * rt_misc set burstPacket destPort gpon streamID <UINT:streamID> infiniteMode burstRate <UINT:burstRate> pktLen <UINT:pktLen> pktData <STRING:pktData>
 */
cparser_result_t
cparser_cmd_rt_misc_set_burstPacket_destPort_gpon_streamID_streamID_infiniteMode_burstRate_burstRate_pktLen_pktLen_pktData_pktData(
	cparser_context_t *context,
	uint32_t  *streamID_ptr,
	uint32_t  *burstRate_ptr,
	uint32_t  *pktLen_ptr,
	char * *pktData_ptr)
{
	int32 ret = RT_ERR_FAILED;
	rt_misc_burst_packet_t burst_packet;
	unsigned char *pktData;
		
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK(RT_MISC_JUMBO_BUF_SIZE <= *pktLen_ptr, RT_ERR_FAILED);
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&burst_packet, 0x0, sizeof(rt_misc_burst_packet_t));
	burst_packet.destPort=RT_PORT_PON;
	burst_packet.dest_stream_id=*streamID_ptr;
	burst_packet.pktData=*pktData_ptr;
	burst_packet.pktLen=*pktLen_ptr;
	burst_packet.burstCount=-1;
	burst_packet.burstRate=*burstRate_ptr;
	
	pktData=(unsigned char *)osal_alloc(burst_packet.pktLen);
	if(pktData){
		int i;
		unsigned char temp[2]={0},result;
		//change from ASCII to HEX
		for(i=0;i<(burst_packet.pktLen<<1);i++){
			temp[0]=burst_packet.pktData[i];				
			result=strtol(temp,NULL,16)&0xf;
			if(i%2==0){
				pktData[i>>1]=result<<4;
			}else{
				pktData[i>>1]|=result;
			}
		}
		burst_packet.pktData=pktData;
		ret = rt_misc_burst_packet_set(burst_packet);
		osal_free(pktData);
		if(ret != RT_ERR_OK){
			diag_util_mprintf("Fail to set burst packet. RT API return 0x%x \n", ret);
		}else{
			diag_util_mprintf("Set burst packet successfully. \n");
			return CPARSER_OK;
		}
	}
	return CPARSER_NOT_OK;
}	/* end of cparser_cmd_rt_misc_set_burstPacket_destPort_gpon_streamID_streamID_infiniteMode_burstRate_burstRate_pktLen_pktLen_pktData_pktData */

/*
 * rt_misc set burstPacket disabled
 */
cparser_result_t
cparser_cmd_rt_misc_set_burstPacket_disabled(
	cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	rt_misc_burst_packet_t burst_packet;
		
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&burst_packet, 0x0, sizeof(rt_misc_burst_packet_t));
	
	ret = rt_misc_burst_packet_set(burst_packet);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("Fail to set burst packet. RT API return 0x%x \n", ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("Set burst packet successfully. \n");
	}
	
	return CPARSER_OK;
}	/* end of cparser_cmd_rt_misc_set_burstPacket_disabled */


/*
 * rt_misc set wanPhyPortMask <HEX:portMask>
 */
cparser_result_t
cparser_cmd_rt_misc_set_wanPhyPortMask_portMask(
    cparser_context_t *context,
    uint32_t  *portMask_ptr)
{
	rt_misc_wan_phyPortMask_t portmask;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&portmask, 0x0, sizeof(rt_misc_wan_phyPortMask_t));
	portmask.mask = *portMask_ptr;

	rt_misc_wanPhyPortMask_set(portmask);
	diag_util_mprintf("Set wanPhyPortMask to 0x%x \n", *portMask_ptr);

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_misc_set_wanphyportmask_portmask */

/*
 * rt_misc get wanPhyPortMask
 */
cparser_result_t
cparser_cmd_rt_misc_get_wanPhyPortMask(
    cparser_context_t *context)
{
	rt_misc_wan_phyPortMask_t portmask;

	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&portmask, 0x0, sizeof(rt_misc_wan_phyPortMask_t));
	rt_misc_wanPhyPortMask_get(&portmask);
	diag_util_mprintf("Get wanPhyPortMask: 0x%x \n", portmask.mask);

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_misc_get_wanphyportmask */

/*
 * rt_misc add wanFastFwdDev <STRING:dev_name>
 */
cparser_result_t
cparser_cmd_rt_misc_add_wanFastFwdDev_dev_name(
    cparser_context_t *context,
    char * *dev_name_ptr)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	if(rt_misc_wanFastFwdDev_add(*dev_name_ptr))
		diag_util_mprintf("Failed to add FastFwdDev %s to WAN\n", *dev_name_ptr);
	else
		diag_util_mprintf("Succeeded to add FastFwdDev %s to WAN\n", *dev_name_ptr);

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_misc_add_wanfastfwddev_dev_name */

/*
 * rt_misc del wanFastFwdDev <STRING:dev_name>
 */
cparser_result_t
cparser_cmd_rt_misc_del_wanFastFwdDev_dev_name(
    cparser_context_t *context,
    char * *dev_name_ptr)
{
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	if(rt_misc_wanFastFwdDev_del(*dev_name_ptr))
		diag_util_mprintf("Failed to delete FastFwdDev %s to WAN\n", *dev_name_ptr);
	else
		diag_util_mprintf("Succeeded to delete FastFwdDev %s to WAN\n", *dev_name_ptr);

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_misc_del_wanfastfwddev_dev_name */


/*
 * rt_misc add vlanGroupMacLimit port <UINT:port> learningLimitNumber <UINT:learningLimitNumber>
 */
cparser_result_t
cparser_cmd_rt_misc_add_vlanGroupMacLimit_port_port_learningLimitNumber_learningLimitNumber(
    cparser_context_t *context,
    uint32_t  *port_ptr,
    uint32_t  *learningLimitNumber_ptr)
{
	int ret,groupIdx;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rt_misc_vlanGroupMacLimit_add(*port_ptr, *learningLimitNumber_ptr, &groupIdx);
	if(ret!=RT_ERR_OK){
		diag_util_mprintf("add vlanGroupMacLimit failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("add vlanGroupMacLimit success. group index=%d\n",groupIdx);
	}

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_misc_add_vlangroupmaclimit_port_port_learninglimitnumber_learninglimitnumber */

/*
 * rt_misc set vlanGroupMacLimit groupIndex <UINT:groupIndex> vlan <UINT:vlanId>
 */
cparser_result_t
cparser_cmd_rt_misc_set_vlanGroupMacLimit_groupIndex_groupIndex_vlan_vlanId(
    cparser_context_t *context,
    uint32_t  *groupIndex_ptr,
    uint32_t  *vlanId_ptr)
{
	int ret;
	int limitNumUnchanged=-1;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rt_misc_vlanGroupMacLimit_set(*groupIndex_ptr, *vlanId_ptr, limitNumUnchanged);
	if(ret!=RT_ERR_OK){
		diag_util_mprintf("set vlanGroupMacLimit failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("set vlanGroupMacLimit Vlan[%d] in group[%d] success.\n",*vlanId_ptr,*groupIndex_ptr);
	}

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_misc_set_vlangroupmaclimit_groupindex_groupindex_vlan_vlanid */

/*
 * rt_misc set vlanGroupMacLimit groupIndex <UINT:groupIndex> vlan untag
 */
cparser_result_t
cparser_cmd_rt_misc_set_vlanGroupMacLimit_groupIndex_groupIndex_vlan_untag(
    cparser_context_t *context,
    uint32_t  *groupIndex_ptr)
{
	int ret,untag=-1;
	int limitNumUnchanged=-1;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rt_misc_vlanGroupMacLimit_set(*groupIndex_ptr, untag, limitNumUnchanged);
	if(ret!=RT_ERR_OK){
		diag_util_mprintf("set vlanGroupMacLimit failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("set vlanGroupMacLimit untag in group[%d] success.\n",*groupIndex_ptr);
	}

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_misc_set_vlangroupmaclimit_groupindex_groupindex_vlan_untag */

/*
 * rt_misc set vlanGroupMacLimit groupIndex <UINT:groupIndex> learningLimitNumber <UINT:learningLimitNumber>
 */
cparser_result_t
cparser_cmd_rt_misc_set_vlanGroupMacLimit_groupIndex_groupIndex_learningLimitNumber_learningLimitNumber(
    cparser_context_t *context,
    uint32_t  *groupIndex_ptr,
    uint32_t  *learningLimitNumber_ptr)
{
    int ret;
	int vlanUnchange=-2;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rt_misc_vlanGroupMacLimit_set(*groupIndex_ptr, vlanUnchange, *learningLimitNumber_ptr);
	if(ret!=RT_ERR_OK){
		diag_util_mprintf("set vlanGroupMacLimit failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("set vlanGroupMacLimit untag in group[%d] success.\n",*groupIndex_ptr);
	}

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_misc_set_vlangroupmaclimit_groupindex_groupindex_learninglimitnumber_learninglimitnumber */

/*
 * rt_misc get vlanGroupMacLimit groupIndex <UINT:groupIndex>
 */
cparser_result_t
cparser_cmd_rt_misc_get_vlanGroupMacLimit_groupIndex_groupIndex(
    cparser_context_t *context,
    uint32_t  *groupIndex_ptr)
{
	int ret;
	rt_misc_vlanGroupMacLimit_info_t groupInfo;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rt_misc_vlanGroupMacLimit_get(*groupIndex_ptr, &groupInfo);
	if(ret!=RT_ERR_OK){
		diag_util_mprintf("get vlanGroupMacLimit failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("get vlanGroupMacLimit success.\n");
		_diag_vlanGroupMacLimitInfoShow(groupInfo);
	}

	return CPARSER_OK;
}	    /* end of cparser_cmd_rt_misc_get_vlangroupmaclimit_groupindex_groupindex */

/*
 * rt_misc del vlanGroupMacLimit groupIndex <UINT:groupIndex> vlan <UINT:vlanId>
 */
cparser_result_t
cparser_cmd_rt_misc_del_vlanGroupMacLimit_groupIndex_groupIndex_vlan_vlanId(
    cparser_context_t *context,
    uint32_t  *groupIndex_ptr,
    uint32_t  *vlanId_ptr)
{
	int ret;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rt_misc_vlanGroupMacLimit_del(*groupIndex_ptr, *vlanId_ptr);
	if(ret!=RT_ERR_OK){
		diag_util_mprintf("del vlanGroupMacLimit failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("del vlanGroupMacLimit Vlan[%d] in group[%d] success.\n",*vlanId_ptr,*groupIndex_ptr);
	}

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_misc_del_vlangroupmaclimit_groupindex_groupindex_vlan_vlanid */

/*
 * rt_misc del vlanGroupMacLimit groupIndex <UINT:groupIndex> vlan untag
 */
cparser_result_t
cparser_cmd_rt_misc_del_vlanGroupMacLimit_groupIndex_groupIndex_vlan_untag(
    cparser_context_t *context,
    uint32_t  *groupIndex_ptr)
{
	int ret,untag=-1;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rt_misc_vlanGroupMacLimit_del(*groupIndex_ptr, untag);
	if(ret!=RT_ERR_OK){
		diag_util_mprintf("del vlanGroupMacLimit failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("del vlanGroupMacLimit untag in group[%d] success.\n",*groupIndex_ptr);
	}

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_misc_del_vlangroupmaclimit_groupindex_groupindex_vlan_untag */

/*
 * rt_misc del vlanGroupMacLimit groupIndex <UINT:groupIndex>
 */
cparser_result_t
cparser_cmd_rt_misc_del_vlanGroupMacLimit_groupIndex_groupIndex(
    cparser_context_t *context,
    uint32_t  *groupIndex_ptr)
{
    int ret;
	int destroyAll=-2;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rt_misc_vlanGroupMacLimit_del(*groupIndex_ptr, destroyAll);
	if(ret!=RT_ERR_OK){
		diag_util_mprintf("del vlanGroupMacLimit failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("del vlanGroupMacLimit group[%d] success.\n",*groupIndex_ptr);
	}

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_misc_del_vlangroupmaclimit_groupindex_groupindex */

/*
 * rt_misc find vlanGroupMacLimit port <UINT:port> vlan <UINT:vlanId>
 */
cparser_result_t
cparser_cmd_rt_misc_find_vlanGroupMacLimit_port_port_vlan_vlanId(
    cparser_context_t *context,
    uint32_t  *port_ptr,
    uint32_t  *vlanId_ptr)
{
	int ret,groupIdx;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rt_misc_vlanGroupMacLimit_find(*port_ptr, *vlanId_ptr, &groupIdx);
	if(ret!=RT_ERR_OK){
		diag_util_mprintf("find vlanGroupMacLimit failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("find Vlan[%d] of Port[%d] in group[%d] success.\n",*vlanId_ptr,*port_ptr,groupIdx);
	}

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_misc_find_vlangroupmaclimit_port_port_vlan_vlanid */

/*
 * rt_misc find vlanGroupMacLimit port <UINT:port> vlan untag
 */
cparser_result_t
cparser_cmd_rt_misc_find_vlanGroupMacLimit_port_port_vlan_untag(
    cparser_context_t *context,
    uint32_t  *port_ptr)
{
	int ret,untag=-1,groupIdx;
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();

	ret = rt_misc_vlanGroupMacLimit_find(*port_ptr, untag, &groupIdx);
	if(ret!=RT_ERR_OK){
		diag_util_mprintf("find vlanGroupMacLimit failed. rg API return %x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("find untag of Port[%d] in group[%d] success.\n",*port_ptr,groupIdx);
	}

	return CPARSER_OK;
}    /* end of cparser_cmd_rt_misc_find_vlangroupmaclimit_port_port_vlan_untag */


	
