/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 68395 $
 * $Date: 2016-05-27 16:38:35 +0800 (Fri, 27 May 2016) $
 *
 * Purpose : Definition of Crypto API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Configuration of Crypto API
 *
 */




/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/init.h>
#include <rtk/default.h>
#include <dal/dal_mgmt.h>
#include <common/util/rt_util.h>
#include <hal/common/halctrl.h>

#include <rt_ext_mapper.h>
#include <rt_crypto_ext.h>



int rt_crypto_key_set(rt_crypto_key_set_t *_rt_crypto_key_set_cfg, uint32 *idx)
{
    int32 ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();
	int i ;
	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->crypto_key_set == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->crypto_key_set(_rt_crypto_key_set_cfg, *idx))!=0)
	{	
		return RT_ERR_NOT_ALLOWED;
	}

	return ret;
}   /* end of rt_pe_http_test */

EXPORT_SYMBOL(rt_crypto_key_set);


int rt_crypto_key_get_index(uint32 *idx)
{
    int32 ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();
	int i ;
	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->crypto_key_get_index == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->crypto_key_get_index(idx))!=0)
	{	
		return RT_ERR_NOT_ALLOWED;
	}

	return ret;
}   /* end of rt_pe_http_test */

EXPORT_SYMBOL(rt_crypto_key_get_index);


int rt_crypto_keyhash_get_index(uint32 *idx)
{
    int32 ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();
	int i ;
	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->crypto_keyhash_get_index == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->crypto_keyhash_get_index(idx))!=0)
	{	
		return RT_ERR_NOT_ALLOWED;
	}

	return ret;
}   /* end of rt_pe_http_test */

EXPORT_SYMBOL(rt_crypto_keyhash_get_index);


int rt_crypto_key_add_by_index(rt_crypto_key_set_t *_rt_crypto_key_set_cfg)
{
    int32 ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();
	int i ;
	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->crypto_key_add_by_index == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->crypto_key_add_by_index(_rt_crypto_key_set_cfg))!=0)
	{	
		return RT_ERR_NOT_ALLOWED;
	}

	return ret;
}   /* end of rt_pe_http_test */

EXPORT_SYMBOL(rt_crypto_key_add_by_index);

int rt_crypto_keyhash_add_by_index(rt_crypto_key_set_t *_rt_crypto_key_set_cfg)
{
    int32 ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();
	int i ;
	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->crypto_keyhash_add_by_index == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->crypto_keyhash_add_by_index(_rt_crypto_key_set_cfg))!=0)
	{	
		return RT_ERR_NOT_ALLOWED;
	}

	return ret;
}   /* end of rt_pe_http_test */

EXPORT_SYMBOL(rt_crypto_keyhash_add_by_index);

int rt_crypto_key_del_by_index(uint32 *idx)
{
    int32 ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();
	int i ;
	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->crypto_key_del_by_index == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->crypto_key_del_by_index(idx))!=0)
	{	
		return RT_ERR_NOT_ALLOWED;
	}

	return ret;
}   /* end of rt_pe_http_test */

EXPORT_SYMBOL(rt_crypto_key_del_by_index);

int rt_crypto_keyhash_del_by_index(uint32 *idx)
{
    int32 ret=RT_ERR_OK;
	rt_ext_mapper_t* p_rt_ext = rt_ext_mapper_get();
	int i ;
	/* function body */
	if(p_rt_ext == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if(p_rt_ext->crypto_keyhash_del_by_index == NULL)
		return RT_ERR_DRIVER_NOT_FOUND;

	if((p_rt_ext->crypto_keyhash_del_by_index(idx))!=0)
	{	
		return RT_ERR_NOT_ALLOWED;
	}

	return ret;
}   /* end of rt_pe_http_test */

EXPORT_SYMBOL(rt_crypto_keyhash_del_by_index);

