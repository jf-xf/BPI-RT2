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

 */




/*
 * Include Files
 */
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <osal/lib.h>
#include <rtk/rtusr/include/rtusr_util.h>
#include <common/rt_type.h>
#include <errno.h>
#include <rtdrv_ext_igmpHook_netfilter.h>
#include <rtdrv_ext_netfilter.h>

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */

/*
 * Function Declaration
 */


int rt_igmpHook_whiteList_add( rt_igmpHook_whiteList_t *patten,rt_igmpHook_devInfo_t devInfo , int *index)
{
	rtdrv_rt_igmpHook_whiteList_ext_t			rt_igmpHook_whiteList_cfg;
	osal_memset(&rt_igmpHook_whiteList_cfg, 0x0, sizeof(rt_igmpHook_whiteList_cfg));	
	osal_memcpy(&rt_igmpHook_whiteList_cfg.mcConfig, patten, sizeof(rt_igmpHook_whiteList_t));
	osal_memcpy(&rt_igmpHook_whiteList_cfg.devInfo, &devInfo, sizeof(rt_igmpHook_devInfo_t));

    SETSOCKOPT(RTDRV_RT_IGMPHOOK_WHITELIST_ADD, &rt_igmpHook_whiteList_cfg, rtdrv_rt_igmpHook_whiteList_ext_t, 1);
	osal_memcpy(index,&rt_igmpHook_whiteList_cfg.index,sizeof(int));
	return RT_ERR_OK;

}
int rt_igmpHook_whiteList_del(rt_igmpHook_whiteList_t *patten,rt_igmpHook_devInfo_t devInfo,int index)
{
	rtdrv_rt_igmpHook_whiteList_ext_t rt_igmpHook_whiteList_cfg;
	osal_memset(&rt_igmpHook_whiteList_cfg, 0x0, sizeof(rt_igmpHook_whiteList_cfg));	
	osal_memcpy(&rt_igmpHook_whiteList_cfg.mcConfig, patten, sizeof(rt_igmpHook_whiteList_t));
	osal_memcpy(&rt_igmpHook_whiteList_cfg.devInfo, &devInfo, sizeof(rt_igmpHook_devInfo_t));
	osal_memcpy(&rt_igmpHook_whiteList_cfg.index, &index, sizeof(int));
    SETSOCKOPT(RTDRV_RT_IGMPHOOK_WHITELIST_DEL, &rt_igmpHook_whiteList_cfg, rtdrv_rt_igmpHook_whiteList_ext_t, 1);
	return RT_ERR_OK;

}


int rt_igmpHook_devConfig_set(rt_igmpHook_devInfo_t devInfo , unsigned int isIpv6 ,rt_igmpHook_control_list_t type , unsigned int value)
{
	rtdrv_rt_igmpHook_devConfig_ext_t			rt_igmpHook_devConfig;
	osal_memset(&rt_igmpHook_devConfig, 0x0, sizeof(rt_igmpHook_devConfig));	
	osal_memcpy(&rt_igmpHook_devConfig.devInfo,&devInfo,sizeof(rt_igmpHook_devInfo_t));
	osal_memcpy(&rt_igmpHook_devConfig.isIpv6,&isIpv6,sizeof(unsigned int));
	osal_memcpy(&rt_igmpHook_devConfig.type,&type,sizeof(rt_igmpHook_control_list_t));
	osal_memcpy(&rt_igmpHook_devConfig.value,&value,sizeof(unsigned int ));
	SETSOCKOPT(RTDRV_RT_IGMPHOOK_DEV_CONFIG_SET, &rt_igmpHook_devConfig, rtdrv_rt_igmpHook_devConfig_ext_t, 1);
	return RT_ERR_OK;

}
int rt_igmpHook_devConfig_get(rt_igmpHook_devInfo_t devInfo , unsigned int isIpv6 ,rt_igmpHook_control_list_t type , unsigned int* value)
{
	rtdrv_rt_igmpHook_devConfig_ext_t			rt_igmpHook_devConfig;
	osal_memset(&rt_igmpHook_devConfig, 0x0, sizeof(rt_igmpHook_devConfig));	
	osal_memcpy(&rt_igmpHook_devConfig.devInfo,&devInfo,sizeof(rt_igmpHook_devInfo_t));
	osal_memcpy(&rt_igmpHook_devConfig.isIpv6,&isIpv6,sizeof(unsigned int));
	osal_memcpy(&rt_igmpHook_devConfig.type,&type,sizeof(rt_igmpHook_control_list_t));
	osal_memcpy(&rt_igmpHook_devConfig.value,&value,sizeof(unsigned int ));
	SETSOCKOPT(RTDRV_RT_IGMPHOOK_DEV_CONFIG_GET, &rt_igmpHook_devConfig, rtdrv_rt_igmpHook_devConfig_ext_t, 1);
	osal_memcpy(value,&rt_igmpHook_devConfig.value,sizeof(int));
	return RT_ERR_OK;
}


int rt_igmpHook_brDevConfig_set(rt_igmpHook_devInfo_t devInfo ,rt_igmpHook_br_control_list_t type , unsigned int value)
{
	rtdrv_rt_igmpHook_brDevConfig_ext_t 		rt_igmpHook_brDevConfig;
	osal_memset(&rt_igmpHook_brDevConfig, 0x0, sizeof(rt_igmpHook_brDevConfig));	
	osal_memcpy(&rt_igmpHook_brDevConfig.devInfo,&devInfo,sizeof(rt_igmpHook_devInfo_t));
	osal_memcpy(&rt_igmpHook_brDevConfig.type,&type,sizeof(rt_igmpHook_br_control_list_t));
	osal_memcpy(&rt_igmpHook_brDevConfig.value,&value,sizeof(unsigned int ));
	SETSOCKOPT(RTDRV_RT_IGMPHOOK_BRDEV_CONFIG_SET, &rt_igmpHook_brDevConfig, rtdrv_rt_igmpHook_brDevConfig_ext_t, 1);
	return RT_ERR_OK;

}
int rt_igmpHook_brDevConfig_get(rt_igmpHook_devInfo_t devInfo ,rt_igmpHook_br_control_list_t type , unsigned int* value)
{
	rtdrv_rt_igmpHook_brDevConfig_ext_t 		rt_igmpHook_brDevConfig;
	osal_memset(&rt_igmpHook_brDevConfig, 0x0, sizeof(rt_igmpHook_brDevConfig));
	osal_memcpy(&rt_igmpHook_brDevConfig.devInfo,&devInfo,sizeof(rt_igmpHook_devInfo_t));
	osal_memcpy(&rt_igmpHook_brDevConfig.type,&type,sizeof(rt_igmpHook_br_control_list_t));
	osal_memcpy(&rt_igmpHook_brDevConfig.value,&value,sizeof(unsigned int ));
	SETSOCKOPT(RTDRV_RT_IGMPHOOK_BRDEV_CONFIG_GET, &rt_igmpHook_brDevConfig, rtdrv_rt_igmpHook_brDevConfig_ext_t, 1);
	osal_memcpy(value,&rt_igmpHook_brDevConfig.value,sizeof(int));
	return RT_ERR_OK;
}


int rt_igmpHook_groupToPsTbl_add(rt_igmpHook_ignoreGroup_t* patten,unsigned int *index)
{
	rtdrv_rt_igmpHook_groupToPsTbl_ext_t		rt_igmpHook_groupToPsTbl;
	osal_memset(&rt_igmpHook_groupToPsTbl, 0x0, sizeof(rt_igmpHook_groupToPsTbl));
	osal_memcpy(&rt_igmpHook_groupToPsTbl.patten,patten,sizeof(rt_igmpHook_ignoreGroup_t));
	SETSOCKOPT(RTDRV_RT_IGMPHOOK_GROUPTOPS_ADD, &rt_igmpHook_groupToPsTbl, rtdrv_rt_igmpHook_groupToPsTbl_ext_t, 1);
	osal_memcpy(index,&rt_igmpHook_groupToPsTbl.index,sizeof(int));
	return RT_ERR_OK;

}
int rt_igmpHook_groupToPsTbl_del(rt_igmpHook_ignoreGroup_t* patten,unsigned int index)
{
	rtdrv_rt_igmpHook_groupToPsTbl_ext_t		rt_igmpHook_groupToPsTbl;
	osal_memset(&rt_igmpHook_groupToPsTbl, 0x0, sizeof(rt_igmpHook_groupToPsTbl));
	osal_memcpy(&rt_igmpHook_groupToPsTbl.patten,patten,sizeof(rt_igmpHook_ignoreGroup_t));
	osal_memcpy(&rt_igmpHook_groupToPsTbl.index,&index,sizeof(unsigned int));
	SETSOCKOPT(RTDRV_RT_IGMPHOOK_GROUPTOPS_DEL, &rt_igmpHook_groupToPsTbl, rtdrv_rt_igmpHook_groupToPsTbl_ext_t, 1);
	return RT_ERR_OK;
}


int rt_igmpHook_nextValidGroupStatistic_get(rt_igmpHook_devInfo_t devInfo,int *getIdx,unsigned int isIpv6,rt_igmpHook_groupStatistic_t *statisticInfo)
{
	rtdrv_rt_igmpHook_groupStatistic_ext_t rt_igmpHook_groupStatistic; 
	osal_memset(&rt_igmpHook_groupStatistic, 0x0, sizeof(rt_igmpHook_groupStatistic));
	osal_memcpy(&rt_igmpHook_groupStatistic.devInfo,&devInfo,sizeof(rt_igmpHook_devInfo_t));
	osal_memcpy(&rt_igmpHook_groupStatistic.isIpv6,&isIpv6,sizeof(unsigned int));
	osal_memcpy(&rt_igmpHook_groupStatistic.getIdx,getIdx,sizeof(int));
	SETSOCKOPT(RTDRV_RT_IGMPHOOK_NEXT_GROUP_STATISTIC_GET, &rt_igmpHook_groupStatistic, rtdrv_rt_igmpHook_groupStatistic_ext_t, 1);
	osal_memcpy(statisticInfo,&rt_igmpHook_groupStatistic.groupStatistic,sizeof(rt_igmpHook_groupStatistic_t));
	osal_memcpy(getIdx,&rt_igmpHook_groupStatistic.getIdx,sizeof(int));

	return RT_ERR_OK;

}

int rt_igmpHook_devStatistic_get(rt_igmpHook_devInfo_t devInfo,unsigned int isIpv6,rt_igmpHook_devStatistic_t *statisticInfo)
{
	rtdrv_rt_igmpHook_devStatistic_ext_t rt_igmpHook_devStatistic; 
	osal_memset(&rt_igmpHook_devStatistic, 0x0, sizeof(rt_igmpHook_devStatistic));
	osal_memcpy(&rt_igmpHook_devStatistic.devInfo,&devInfo,sizeof(rt_igmpHook_devInfo_t));
	osal_memcpy(&rt_igmpHook_devStatistic.isIpv6,&isIpv6,sizeof(unsigned int));
	SETSOCKOPT(RTDRV_RT_IGMPHOOK_DEV_STATISTIC_GET, &rt_igmpHook_devStatistic, rtdrv_rt_igmpHook_devStatistic_ext_t, 1);	
	osal_memcpy(statisticInfo,&rt_igmpHook_devStatistic.devStatistic,sizeof(rt_igmpHook_devStatistic_t));
	return RT_ERR_OK;
}
 
int rt_igmpHook_groupWeight_add(rt_igmpHook_devInfo_t devInfo,rt_igmpHook_groupWeight_t* groupWeightCfg, int *index)
{
	rtdrv_rt_igmpHook_groupWeight_ext_t rt_igmpHook_groupWeight;
	osal_memset(&rt_igmpHook_groupWeight, 0x0, sizeof(rt_igmpHook_groupWeight));
	osal_memcpy(&rt_igmpHook_groupWeight.devInfo,&devInfo,sizeof(rt_igmpHook_devInfo_t));
	osal_memcpy(&rt_igmpHook_groupWeight.groupWeightCfg,groupWeightCfg,sizeof(rt_igmpHook_groupWeight_t));
	SETSOCKOPT(RTDRV_RT_IGMPHOOK_GROUP_WEIGHT_ADD, &rt_igmpHook_groupWeight, rtdrv_rt_igmpHook_groupWeight_ext_t, 1);
	osal_memcpy(index,&rt_igmpHook_groupWeight.index,sizeof( int));
	return RT_ERR_OK;
}
int rt_igmpHook_groupWeight_del(rt_igmpHook_devInfo_t devInfo,rt_igmpHook_groupWeight_t* groupWeightCfg, int  index)
{
	rtdrv_rt_igmpHook_groupWeight_ext_t rt_igmpHook_groupWeight;
	osal_memset(&rt_igmpHook_groupWeight, 0x0, sizeof(rt_igmpHook_groupWeight));
	osal_memcpy(&rt_igmpHook_groupWeight.devInfo,&devInfo,sizeof(rt_igmpHook_devInfo_t));
	osal_memcpy(&rt_igmpHook_groupWeight.groupWeightCfg,groupWeightCfg,sizeof(rt_igmpHook_groupWeight_t));
	osal_memcpy(&rt_igmpHook_groupWeight.index,&index,sizeof( int));
	SETSOCKOPT(RTDRV_RT_IGMPHOOK_GROUP_WEIGHT_DEL, &rt_igmpHook_groupWeight, rtdrv_rt_igmpHook_groupWeight_ext_t, 1);	

	return RT_ERR_OK;

}


