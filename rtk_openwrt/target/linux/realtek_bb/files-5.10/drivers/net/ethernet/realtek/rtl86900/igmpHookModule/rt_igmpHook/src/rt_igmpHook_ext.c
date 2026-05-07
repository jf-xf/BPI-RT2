
/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/init.h>
#include <rtk/default.h>
#include <dal/dal_mgmt.h>
#include <common/util/rt_util.h>
#include <hal/common/halctrl.h>

#include <rt_ext_igmpHook_mapper.h>

int rt_igmpHook_whiteList_add( rt_igmpHook_whiteList_t *patten,rt_igmpHook_devInfo_t devInfo , int *index)
{
    int32   ret=RT_ERR_OK;
    rt_ext_igmpHook_mapper_t* p_rt_ext = rt_ext_igmpHook_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->rt_igmpHook_whiteList_add == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->rt_igmpHook_whiteList_add(patten,devInfo,index))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;

}
int rt_igmpHook_whiteList_del(rt_igmpHook_whiteList_t *patten,rt_igmpHook_devInfo_t devInfo,int index)
{
    int32   ret=RT_ERR_OK;
    rt_ext_igmpHook_mapper_t* p_rt_ext = rt_ext_igmpHook_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->rt_igmpHook_whiteList_del == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->rt_igmpHook_whiteList_del(patten,devInfo,index))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;
}


int rt_igmpHook_devConfig_set(rt_igmpHook_devInfo_t devInfo , unsigned int isIpv6 ,rt_igmpHook_control_list_t type , unsigned int value)
{
    int32   ret=RT_ERR_OK;
    rt_ext_igmpHook_mapper_t* p_rt_ext = rt_ext_igmpHook_mapper_get();
    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->rt_igmpHook_devConfig_set == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->rt_igmpHook_devConfig_set(devInfo,isIpv6,type,value))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;

}

int rt_igmpHook_devConfig_get(rt_igmpHook_devInfo_t devInfo , unsigned int isIpv6 ,rt_igmpHook_control_list_t type , unsigned int* value)
{
    int32   ret=RT_ERR_OK;
    rt_ext_igmpHook_mapper_t* p_rt_ext = rt_ext_igmpHook_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->rt_igmpHook_devConfig_get == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->rt_igmpHook_devConfig_get(devInfo,isIpv6,type,value))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;

}


int rt_igmpHook_brDevConfig_set(rt_igmpHook_devInfo_t devInfo ,rt_igmpHook_br_control_list_t type , unsigned int value)
{
    int32   ret=RT_ERR_OK;
    rt_ext_igmpHook_mapper_t* p_rt_ext = rt_ext_igmpHook_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->rt_igmpHook_brDevConfig_set == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->rt_igmpHook_brDevConfig_set(devInfo,type,value))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;


}
int rt_igmpHook_brDevConfig_get(rt_igmpHook_devInfo_t devInfo ,rt_igmpHook_br_control_list_t type , unsigned int* value)
{
    int32   ret=RT_ERR_OK;
    rt_ext_igmpHook_mapper_t* p_rt_ext = rt_ext_igmpHook_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->rt_igmpHook_brDevConfig_get == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->rt_igmpHook_brDevConfig_get(devInfo,type,value))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;

}


int rt_igmpHook_groupToPsTbl_add(rt_igmpHook_ignoreGroup_t* patten,unsigned int *index)
{
    int32   ret=RT_ERR_OK;
    rt_ext_igmpHook_mapper_t* p_rt_ext = rt_ext_igmpHook_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->rt_igmpHook_groupToPsTbl_add == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->rt_igmpHook_groupToPsTbl_add(patten,index))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;


}
int rt_igmpHook_groupToPsTbl_del(rt_igmpHook_ignoreGroup_t* patten,unsigned int index)
{
    int32   ret=RT_ERR_OK;
    rt_ext_igmpHook_mapper_t* p_rt_ext = rt_ext_igmpHook_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->rt_igmpHook_groupToPsTbl_del == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->rt_igmpHook_groupToPsTbl_del(patten,index))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;


}



int rt_igmpHook_nextValidGroupStatistic_get(rt_igmpHook_devInfo_t devInfo,int *getIdx,unsigned int isIpv6,rt_igmpHook_groupStatistic_t *statisticInfo)
{
    int32   ret=RT_ERR_OK;
    rt_ext_igmpHook_mapper_t* p_rt_ext = rt_ext_igmpHook_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->rt_igmpHook_nextValidGroupStatistic_get == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->rt_igmpHook_nextValidGroupStatistic_get(devInfo,getIdx,isIpv6,statisticInfo))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;


}

int rt_igmpHook_devStatistic_get(rt_igmpHook_devInfo_t devInfo,unsigned int isIpv6,rt_igmpHook_devStatistic_t *statisticInfo)
{
    int32   ret=RT_ERR_OK;
    rt_ext_igmpHook_mapper_t* p_rt_ext = rt_ext_igmpHook_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->rt_igmpHook_devStatistic_get == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->rt_igmpHook_devStatistic_get(devInfo,isIpv6,statisticInfo))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;


}

int rt_igmpHook_groupWeight_add(rt_igmpHook_devInfo_t devInfo,rt_igmpHook_groupWeight_t* groupWeightCfg, int *index)

{
    int32   ret=RT_ERR_OK;
    rt_ext_igmpHook_mapper_t* p_rt_ext = rt_ext_igmpHook_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->rt_igmpHook_groupWeight_add == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->rt_igmpHook_groupWeight_add(devInfo,groupWeightCfg,index))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;


}


int rt_igmpHook_groupWeight_del(rt_igmpHook_devInfo_t devInfo,rt_igmpHook_groupWeight_t* groupWeightCfg, int  index)
{
    int32   ret=RT_ERR_OK;
    rt_ext_igmpHook_mapper_t* p_rt_ext = rt_ext_igmpHook_mapper_get();

    /* function body */
    if(p_rt_ext == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if(p_rt_ext->rt_igmpHook_groupWeight_del == NULL)
        return RT_ERR_DRIVER_NOT_FOUND;

    if((p_rt_ext->rt_igmpHook_groupWeight_del(devInfo,groupWeightCfg,index))!=0)
        return RT_ERR_NOT_ALLOWED;

    return ret;


}






