
#include <stdio.h>
#include <string.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <diag_util.h>
#include <diag_str.h>
#include <parser/cparser_priv.h>

#include <rt_igmpHook_ext.h>

/*
 * rt_igmp set igmpHookModule devCfg <STRING:devCfg> isIPv6  <UINT:isIPv6> type <UINT:type> value <UINT:value>
 */
cparser_result_t
cparser_cmd_rt_igmp_set_igmpHookModule_devCfg_devCfg_isIPv6_isIPv6_type_type_value_value(
    cparser_context_t *context,
    char * *devCfg_ptr,
    uint32_t  *isIPv6_ptr,
    uint32_t  *type_ptr,
    uint32_t  *value_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	rt_igmpHook_devInfo_t devInfo ;
	osal_memset(&devInfo, 0x0, sizeof(rt_igmpHook_devInfo_t));	
	osal_strcpy(devInfo.devIfname,*devCfg_ptr);


    ret = rt_igmpHook_devConfig_set(devInfo,*isIPv6_ptr,*type_ptr,*value_ptr);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("set rt_igmpHook_devConfig_set failed. RT API return 0x%x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		return CPARSER_OK;
	}
	return CPARSER_OK;

}    /* end of cparser_cmd_rt_igmp_set_igmphookmodule_devcfg_devcfg_isipv6_isipv6_type_type_value_value */

/*
 * rt_igmp get igmpHookModule devCfg <STRING:devCfg> isIPv6  <UINT:isIPv6> type <UINT:type>
 */
cparser_result_t
cparser_cmd_rt_igmp_get_igmpHookModule_devCfg_devCfg_isIPv6_isIPv6_type_type(
    cparser_context_t *context,
    char * *devCfg_ptr,
    uint32_t  *isIPv6_ptr,
    uint32_t  *type_ptr)
{
	int ret;
	int32 value;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
	
	rt_igmpHook_devInfo_t devInfo ;
	osal_memset(&devInfo, 0x0, sizeof(rt_igmpHook_devInfo_t));	
	osal_strcpy(devInfo.devIfname,*devCfg_ptr);

    ret = rt_igmpHook_devConfig_get(devInfo,*isIPv6_ptr,*type_ptr,&value);
	
	if(ret != RT_ERR_OK){
		diag_util_mprintf("set rt_igmpHook_devConfig_get failed. RT API return 0x%x \n",ret);
		return CPARSER_NOT_OK;
	}
	else
	{
		diag_util_mprintf("value = %d\n",value);
		return CPARSER_OK;
	}
	return CPARSER_OK;


    return CPARSER_OK;
}    /* end of cparser_cmd_rt_igmp_get_igmphookmodule_devcfg_devcfg_isipv6_isipv6_type_type */

/*
 * rt_igmp set igmpHookModule brDevCfg <STRING:brDevCfg> type <UINT:type> value <UINT:value>
 */
cparser_result_t
cparser_cmd_rt_igmp_set_igmpHookModule_brDevCfg_brDevCfg_type_type_value_value(
    cparser_context_t *context,
    char * *brDevCfg_ptr,
    uint32_t  *type_ptr,
    uint32_t  *value_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	rt_igmpHook_devInfo_t devInfo ;
	osal_memset(&devInfo, 0x0, sizeof(rt_igmpHook_devInfo_t));	
	osal_strcpy(devInfo.devIfname,*brDevCfg_ptr);

    ret = rt_igmpHook_brDevConfig_set(devInfo,*type_ptr,*value_ptr);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("set rt_igmpHook_brDevConfig_set failed. RT API return 0x%x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		return CPARSER_OK;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_igmp_set_igmphookmodule_brdevcfg_brdevcfg_type_type_value_value */

/*
 * rt_igmp get igmpHookModule brDevCfg <STRING:brDevCfg> type <UINT:type>
 */
cparser_result_t
cparser_cmd_rt_igmp_get_igmpHookModule_brDevCfg_brDevCfg_type_type(
	cparser_context_t *context,
	char * *brDevCfg_ptr,
	uint32_t  *type_ptr)
{
	int ret;
	int32 value;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	rt_igmpHook_devInfo_t devInfo ;
	osal_memset(&devInfo, 0x0, sizeof(rt_igmpHook_devInfo_t));	
	osal_strcpy(devInfo.devIfname,*brDevCfg_ptr);

    ret = rt_igmpHook_brDevConfig_get(devInfo,*type_ptr,&value);
	if(ret != RT_ERR_OK){
		diag_util_mprintf("set rt_igmpHook_brDevConfig_get failed. RT API return 0x%x \n",ret);
		return CPARSER_NOT_OK;
	}else{
		diag_util_mprintf("value = %d\n",value);
		return CPARSER_OK;
	}

    return CPARSER_OK;

}	 /* end of cparser_cmd_rt_igmp_get_igmphookmodule_brdevcfg_brdevcfg_type_type */


