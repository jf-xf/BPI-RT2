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
 * Purpose : Definition of PE API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Configuration of http test
 *
 */




/*
 * Include Files
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <osal/lib.h>
#include <rtk/rtusr/include/rtusr_util.h>
#include <rtdrv_ext_netfilter.h>
#include <common/rt_type.h>


int rt_crypto_key_set(rt_crypto_key_set_t *_rt_crypto_key_set_cfg, uint32 *key_idx)
{
    rtdrv_rt_cryptoCfg_ext_t rt_crypto_key_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == key_idx), RT_ERR_NULL_POINTER);
	
    /* function body */
   	osal_memcpy(&rt_crypto_key_cfg.rt_crypto_key_set_cfg.key[0] , _rt_crypto_key_set_cfg->key , 64);
	rt_crypto_key_cfg.rt_crypto_key_set_cfg.key_sz = _rt_crypto_key_set_cfg->key_sz;
    GETSOCKOPT(RTDRV_RT_CRYPTO_KEY_SET, &rt_crypto_key_cfg, rtdrv_rt_cryptoCfg_ext_t, 1);
	osal_memcpy(key_idx, &rt_crypto_key_cfg.rt_crypto_key_set_cfg.key_index, sizeof(uint32));

    return RT_ERR_OK;
}   /* end of rt_crypto_key_set */


int rt_crypto_key_get_index(uint32 *idx)
{
	int key_index = -1;
    rtdrv_rt_cryptoCfg_ext_t rt_crypto_key_cfg;
    /* parameter check */
    RT_PARAM_CHK((NULL == idx), RT_ERR_NULL_POINTER);

	
    /* function body */
    GETSOCKOPT(RTDRV_RT_CRYPTO_KEY_GET, &rt_crypto_key_cfg, rtdrv_rt_cryptoCfg_ext_t, 1);
	osal_memcpy(idx, &rt_crypto_key_cfg.rt_crypto_key_set_cfg.key_index, sizeof(uint32));

    return RT_ERR_OK;
}   /* end of rt_crypto_key_set */


int rt_crypto_keyhash_get_index(uint32 *idx)
{
	int key_index = -1;
    rtdrv_rt_cryptoCfg_ext_t rt_crypto_key_cfg;
    /* parameter check */
    RT_PARAM_CHK((NULL == idx), RT_ERR_NULL_POINTER);

	
    /* function body */
    GETSOCKOPT(RTDRV_RT_CRYPTO_KEYHASH_GET, &rt_crypto_key_cfg, rtdrv_rt_cryptoCfg_ext_t, 1);
	osal_memcpy(idx, &rt_crypto_key_cfg.rt_crypto_key_set_cfg.key_index, sizeof(uint32));

    return RT_ERR_OK;
}   /* end of rt_crypto_key_set */

int rt_crypto_key_del_by_index(uint32 *idx)
{
	int key_index = -1;
    rtdrv_rt_cryptoCfg_ext_t rt_crypto_key_cfg;
    /* parameter check */
    RT_PARAM_CHK((NULL == idx), RT_ERR_NULL_POINTER);

	
    /* function body */
   	rt_crypto_key_cfg.rt_crypto_key_set_cfg.key_index = *idx;
    GETSOCKOPT(RTDRV_RT_CRYPTO_KEY_DEL_BY_INDEX, &rt_crypto_key_cfg, rtdrv_rt_cryptoCfg_ext_t, 1);
	
    return RT_ERR_OK;
}   /* end of rt_crypto_key_set */


int rt_crypto_keyhash_del_by_index(uint32 *idx)
{
	int key_index = -1;
    rtdrv_rt_cryptoCfg_ext_t rt_crypto_key_cfg;
    /* parameter check */
    RT_PARAM_CHK((NULL == idx), RT_ERR_NULL_POINTER);

	
    /* function body */
   	rt_crypto_key_cfg.rt_crypto_key_set_cfg.key_index = *idx;
    GETSOCKOPT(RTDRV_RT_CRYPTO_KEYHASH_DEL_BY_INDEX, &rt_crypto_key_cfg, rtdrv_rt_cryptoCfg_ext_t, 1);
	
    return RT_ERR_OK;
}   /* end of rt_crypto_key_set */

int rt_crypto_key_add_by_index(rt_crypto_key_set_t *_rt_crypto_key_set_cfg)
{
    rtdrv_rt_cryptoCfg_ext_t rt_crypto_key_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == _rt_crypto_key_set_cfg), RT_ERR_NULL_POINTER);
	
    /* function body */
   	osal_memcpy(&rt_crypto_key_cfg.rt_crypto_key_set_cfg.key[0] , _rt_crypto_key_set_cfg->key , 64);
	rt_crypto_key_cfg.rt_crypto_key_set_cfg.key_sz = _rt_crypto_key_set_cfg->key_sz;
	rt_crypto_key_cfg.rt_crypto_key_set_cfg.key_index = _rt_crypto_key_set_cfg->key_index;
    GETSOCKOPT(RTDRV_RT_CRYPTO_KEY_SET_BY_INDEX, &rt_crypto_key_cfg, rtdrv_rt_cryptoCfg_ext_t, 1);
	//osal_memcpy(key_idx, &rt_crypto_key_cfg.rt_crypto_key_set_cfg.key_index, sizeof(uint32));

    return RT_ERR_OK;
}


int rt_crypto_keyhash_add_by_index(rt_crypto_key_set_t *_rt_crypto_key_set_cfg)
{
    rtdrv_rt_cryptoCfg_ext_t rt_crypto_key_cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == _rt_crypto_key_set_cfg), RT_ERR_NULL_POINTER);
	
    /* function body */
   	osal_memcpy(&rt_crypto_key_cfg.rt_crypto_key_set_cfg.key[0] , _rt_crypto_key_set_cfg->key , 64);
	rt_crypto_key_cfg.rt_crypto_key_set_cfg.key_sz = _rt_crypto_key_set_cfg->key_sz;
	rt_crypto_key_cfg.rt_crypto_key_set_cfg.key_index = _rt_crypto_key_set_cfg->key_index;
    GETSOCKOPT(RTDRV_RT_CRYPTO_KEYHASH_SET_BY_INDEX, &rt_crypto_key_cfg, rtdrv_rt_cryptoCfg_ext_t, 1);
	//osal_memcpy(key_idx, &rt_crypto_key_cfg.rt_crypto_key_set_cfg.key_index, sizeof(uint32));

    return RT_ERR_OK;
}



