#ifndef _RTK_IGMP_HOOK_MODULE_API_H
#define _RTK_IGMP_HOOK_MODULE_API_H


#include <rt_ext_igmpHook_mapper.h>


int rtk_rt_igmpHook_mapper_api_init(void);


int rt_igmpHook_whiteList_add( rt_igmpHook_whiteList_t *patten,rt_igmpHook_devInfo_t ifidx , int *index);
int rt_igmpHook_whiteList_del(rt_igmpHook_whiteList_t *patten,rt_igmpHook_devInfo_t ifidx,int index);



int rt_igmpHook_devConfig_set(rt_igmpHook_devInfo_t devInfo , unsigned int isIpv6 ,rt_igmpHook_control_list_t type , unsigned int value);
int rt_igmpHook_devConfig_get(rt_igmpHook_devInfo_t devInfo , unsigned int isIpv6 ,rt_igmpHook_control_list_t type , unsigned int* value);



int rt_igmpHook_brDevConfig_set(rt_igmpHook_devInfo_t devInfo ,rt_igmpHook_br_control_list_t type , unsigned int value);
int rt_igmpHook_brDevConfig_get(rt_igmpHook_devInfo_t devInfo ,rt_igmpHook_br_control_list_t type , unsigned int* value);


int rt_igmpHook_groupToPsTbl_add(rt_igmpHook_ignoreGroup_t* patten,unsigned int *index);
int rt_igmpHook_groupToPsTbl_del(rt_igmpHook_ignoreGroup_t* patten,unsigned int index);


int rt_igmpHook_nextValidGroupStatistic_get(rt_igmpHook_devInfo_t devInfo,int *getIdx,unsigned int isIpv6,rt_igmpHook_groupStatistic_t *statisticInfo);

int rt_igmpHook_devStatistic_get(rt_igmpHook_devInfo_t devInfo,unsigned int isIpv6,rt_igmpHook_devStatistic_t *statisticInfo);
 
int rt_igmpHook_groupWeight_add(rt_igmpHook_devInfo_t devInfo,rt_igmpHook_groupWeight_t* groupWeightCfg, int *index);
int rt_igmpHook_groupWeight_del(rt_igmpHook_devInfo_t devInfo,rt_igmpHook_groupWeight_t* groupWeightCfg, int  index);


#endif
