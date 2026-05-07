/*
 * rt_crypto key add key <STRING:key> key_size <INT:key_size>
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

#include <rt_crypto_ext.h>



cparser_result_t
cparser_cmd_rt_crypto_key_add_key_key_key_size_key_size(
    cparser_context_t *context,
    char * *key_ptr,
    int32_t  *key_size_ptr)
{
	int32 ret = RT_ERR_FAILED;
	int key_idx = 0;
	rt_crypto_key_set_t crypto_key_set_cfg;
	char *temp_key;
	int i;
	unsigned char temp[2]={0},result;
	unsigned char *keyData;
	
	DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_OUTPUT_INIT();
	
	
	crypto_key_set_cfg.key_sz = *key_size_ptr;
	temp_key = *key_ptr;
	//osal_memcpy(&crypto_key_set_cfg.key[0], *key_ptr, *key_size_ptr+1);
	keyData=(unsigned char *)osal_alloc(crypto_key_set_cfg.key_sz);

	for(i=0;i<(crypto_key_set_cfg.key_sz<<1);i++){
		temp[0]=temp_key[i];
		result=strtol(temp,NULL,16)&0xf;
		if(i%2==0){
			keyData[i>>1]=result<<4;
		}else{
			keyData[i>>1]|=result;
		}
	}

	osal_memcpy(&crypto_key_set_cfg.key[0], keyData, crypto_key_set_cfg.key_sz);
	
	ret = rt_crypto_key_set(&crypto_key_set_cfg, &key_idx);
	
	if(ret == RT_ERR_OK)
		diag_util_mprintf("add key entry[%d] success!\n", key_idx);
	else
		diag_util_mprintf("add key entry failed! (ret=0x%x) \n",ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_crypto_key_add_key_key_key_size_key_size */

/*
 * rt_crypto get key_index
 */

cparser_result_t
cparser_cmd_rt_crypto_get_key_index(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	int key_idx = 0;
	
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	ret = rt_crypto_key_get_index(&key_idx);
	
	if(ret == RT_ERR_OK)
		diag_util_mprintf("add key entry[%d] success!\n", key_idx);
	else
		diag_util_mprintf("add key entry failed! (ret=0x%x) \n",ret);


    return CPARSER_OK;
}    /* end of cparser_cmd_rt_crypto_get_key_index */

/*
 * rt_crypto add_key_by_index index <INT:index> key <STRING:key> key_size <INT:key_size>
 */
cparser_result_t
cparser_cmd_rt_crypto_add_key_by_index_index_index_key_key_key_size_key_size(
    cparser_context_t *context,
    int32_t  *index_ptr,
    char * *key_ptr,
    int32_t  *key_size_ptr)
{
	int32 ret = RT_ERR_FAILED;
	int key_idx = 0;
	rt_crypto_key_set_t crypto_key_set_cfg;
	char *temp_key;
	int i;
	unsigned char temp[2]={0},result;
	unsigned char *keyData;
	
    DIAG_UTIL_PARAM_CHK();
	
	crypto_key_set_cfg.key_sz = *key_size_ptr;
	crypto_key_set_cfg.key_index = *index_ptr;
	
	temp_key = *key_ptr;
	//osal_memcpy(&crypto_key_set_cfg.key[0], *key_ptr, *key_size_ptr+1);
	keyData=(unsigned char *)osal_alloc(crypto_key_set_cfg.key_sz);

	for(i=0;i<(crypto_key_set_cfg.key_sz<<1);i++){
		temp[0]=temp_key[i];
		result=strtol(temp,NULL,16)&0xf;
		if(i%2==0){
			keyData[i>>1]=result<<4;
		}else{
			keyData[i>>1]|=result;
		}
	}

	osal_memcpy(&crypto_key_set_cfg.key[0], keyData, crypto_key_set_cfg.key_sz);
	
	ret = rt_crypto_key_add_by_index(&crypto_key_set_cfg);
	
	if(ret == RT_ERR_OK)
		diag_util_mprintf("add key entry[%d] success!\n", key_idx);
	else
		diag_util_mprintf("add key entry failed! (ret=0x%x) \n",ret);

    return CPARSER_OK;
}     /* end of cparser_cmd_rt_crypto_add_key_by_index_index_index_key_key_key_size_key_size */

/*
 * rt_crypto del_by_index index <INT:index>
 */
cparser_result_t
cparser_cmd_rt_crypto_del_key_by_index_index_index(
    cparser_context_t *context,
    int32_t  *index_ptr)
{
	int32 ret = RT_ERR_FAILED;
	int key_index = 0;
	
    DIAG_UTIL_PARAM_CHK();
	diag_util_mprintf("Try to invalid key entry:%d\n",*index_ptr);
	key_index = *index_ptr;
	ret = rt_crypto_key_del_by_index(&key_index);
		
	if(ret == RT_ERR_OK)
		diag_util_mprintf("add key entry[%d] success!\n", key_index);
	else
		diag_util_mprintf("add key entry failed! (ret=0x%x) \n",ret);


    return CPARSER_OK;
}    /* end of cparser_cmd_rt_crypto_del_key_by_index_index_index */

/*
 * rt_crypto get key_hash_index
 */
cparser_result_t
cparser_cmd_rt_crypto_get_key_hash_index(
    cparser_context_t *context)
{
	int32 ret = RT_ERR_FAILED;
	int key_hash_idx = 0;
	
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	ret = rt_crypto_keyhash_get_index(&key_hash_idx);
	
	if(ret == RT_ERR_OK)
		diag_util_mprintf("add key hash entry[%d] success!\n", key_hash_idx);
	else
		diag_util_mprintf("add key hash entry failed! (ret=0x%x) \n",ret);


    return CPARSER_OK;
}    /* end of cparser_cmd_rt_crypto_get_key_hash_index */

/*
 * rt_crypto add_keyhash_by_index index <INT:index> key <STRING:key> key_size <INT:key_size>
 */
cparser_result_t
cparser_cmd_rt_crypto_add_keyhash_by_index_index_index_key_key_key_size_key_size(
    cparser_context_t *context,
    int32_t  *index_ptr,
    char * *key_ptr,
    int32_t  *key_size_ptr)
{
	int32 ret = RT_ERR_FAILED;
	int key_idx = 0;
	rt_crypto_key_set_t crypto_key_set_cfg;
	char *temp_key;
	int i;
	unsigned char temp[2]={0},result;
	unsigned char *keyData;
	
    DIAG_UTIL_PARAM_CHK();
	
	crypto_key_set_cfg.key_sz = *key_size_ptr;
	crypto_key_set_cfg.key_index = *index_ptr;
	
	temp_key = *key_ptr;
	//osal_memcpy(&crypto_key_set_cfg.key[0], *key_ptr, *key_size_ptr+1);
	keyData=(unsigned char *)osal_alloc(crypto_key_set_cfg.key_sz);

	for(i=0;i<(crypto_key_set_cfg.key_sz<<1);i++){
		temp[0]=temp_key[i];
		result=strtol(temp,NULL,16)&0xf;
		if(i%2==0){
			keyData[i>>1]=result<<4;
		}else{
			keyData[i>>1]|=result;
		}
	}

	osal_memcpy(&crypto_key_set_cfg.key[0], keyData, crypto_key_set_cfg.key_sz);
	
	ret = rt_crypto_keyhash_add_by_index(&crypto_key_set_cfg);
	
	if(ret == RT_ERR_OK)
		diag_util_mprintf("add key hash entry[%d] success!\n", key_idx);
	else
		diag_util_mprintf("add key hash entry failed! (ret=0x%x) \n",ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rt_crypto_add_keyhash_by_index_index_index_key_key_key_size_key_size */

/*
 * rt_crypto del_keyhash_by_index index <INT:index>
 */
cparser_result_t
cparser_cmd_rt_crypto_del_keyhash_by_index_index_index(
    cparser_context_t *context,
    int32_t  *index_ptr)
{
	int32 ret = RT_ERR_FAILED;
	int key_index = 0;
	
    DIAG_UTIL_PARAM_CHK();
	diag_util_mprintf("Try to invalid key entry:%d\n",*index_ptr);
	key_index = *index_ptr;
	ret = rt_crypto_keyhash_del_by_index(&key_index);
		
	if(ret == RT_ERR_OK)
		diag_util_mprintf("add key entry[%d] success!\n", key_index);
	else
		diag_util_mprintf("add key entry failed! (ret=0x%x) \n",ret);


    return CPARSER_OK;
}    /* end of cparser_cmd_rt_crypto_del_keyhash_by_index_index_index */



