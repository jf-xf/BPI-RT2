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
 * $Revision: 93636 $
 * $Date: 2018-11-16 12:52:37 +0800 (Fri, 16 Nov 2018) $
 *
 * Purpose : Definition of Rtdrv Extension Mapper
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Mapper register
 *           (2) Mapper get
 *
 */

 
#ifndef __RT_EXT_IGMPHOOK_MAPPER_H__
#define __RT_EXT_IGMPHOOK_MAPPER_H__

/*
 * Include Files
 */
#include <rt_igmpHook_ext.h>


/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

/*data structure for internal use (communication between FC & RT)*/


typedef struct rt_ext_igmpHook_mapper_s 
{
	int (*rt_igmpHook_whiteList_add)( rt_igmpHook_whiteList_t *patten,rt_igmpHook_devInfo_t ifidx , int *index);
	int (*rt_igmpHook_whiteList_del)(rt_igmpHook_whiteList_t *patten,rt_igmpHook_devInfo_t ifidx,int index);
	
	int (*rt_igmpHook_devConfig_set)(rt_igmpHook_devInfo_t devInfo , unsigned int isIpv6 ,rt_igmpHook_control_list_t type , unsigned int value);
	int (*rt_igmpHook_devConfig_get)(rt_igmpHook_devInfo_t devInfo , unsigned int isIpv6 ,rt_igmpHook_control_list_t type , unsigned int* value);
		
	int (*rt_igmpHook_brDevConfig_set)(rt_igmpHook_devInfo_t devInfo ,rt_igmpHook_br_control_list_t type , unsigned int value);
	int (*rt_igmpHook_brDevConfig_get)(rt_igmpHook_devInfo_t devInfo ,rt_igmpHook_br_control_list_t type , unsigned int* value);
		
	int (*rt_igmpHook_groupToPsTbl_add)(rt_igmpHook_ignoreGroup_t* patten,unsigned int *index);
	int (*rt_igmpHook_groupToPsTbl_del)(rt_igmpHook_ignoreGroup_t* patten,unsigned int index);
	
	int (*rt_igmpHook_nextValidGroupStatistic_get)(rt_igmpHook_devInfo_t devInfo,int *getIdx,unsigned int isIpv6,rt_igmpHook_groupStatistic_t *statisticInfo);
	int (*rt_igmpHook_devStatistic_get)(rt_igmpHook_devInfo_t devInfo,unsigned int isIpv6,rt_igmpHook_devStatistic_t *statisticInfo);
	int (*rt_igmpHook_groupWeight_add)(rt_igmpHook_devInfo_t devInfo,rt_igmpHook_groupWeight_t* groupWeightCfg, int *index);
	int (*rt_igmpHook_groupWeight_del)(rt_igmpHook_devInfo_t devInfo,rt_igmpHook_groupWeight_t* groupWeightCfg, int  index);

} rt_ext_igmpHook_mapper_t;

void rt_ext_igmpHook_mapper_register(rt_ext_igmpHook_mapper_t *pMapper);

rt_ext_igmpHook_mapper_t* rt_ext_igmpHook_mapper_get(void);
#endif /*__RT_EXT_IGMPHOOK_MAPPER_H__*/

