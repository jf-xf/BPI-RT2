

#include <rtk_igmp_hook_module_api.h>
#include <rtk_igmp_snooping.h>
#include <rt_ext_igmpHook_mapper.h>
#include <rtk_igmp_hook.h>


static rt_ext_igmpHook_mapper_t pf_mapper =
{
	.rt_igmpHook_whiteList_add 		= 	rt_igmpHook_whiteList_add,
	.rt_igmpHook_whiteList_del 		= 	rt_igmpHook_whiteList_del,
	.rt_igmpHook_devConfig_set 		= 	rt_igmpHook_devConfig_set,
	.rt_igmpHook_devConfig_get 		= 	rt_igmpHook_devConfig_get,
	.rt_igmpHook_brDevConfig_set	= 	rt_igmpHook_brDevConfig_set,
	.rt_igmpHook_brDevConfig_get	= 	rt_igmpHook_brDevConfig_get,
	.rt_igmpHook_groupToPsTbl_add	= 	rt_igmpHook_groupToPsTbl_add,
	.rt_igmpHook_groupToPsTbl_del	= 	rt_igmpHook_groupToPsTbl_del,
	.rt_igmpHook_nextValidGroupStatistic_get = rt_igmpHook_nextValidGroupStatistic_get,
	.rt_igmpHook_devStatistic_get	=	rt_igmpHook_devStatistic_get,
	.rt_igmpHook_groupWeight_add	=	rt_igmpHook_groupWeight_add,
	.rt_igmpHook_groupWeight_del	=	rt_igmpHook_groupWeight_del,
};


int rtk_rt_igmpHook_mapper_api_init(void)
{
	rt_ext_igmpHook_mapper_register(&pf_mapper);
	return 0;
}


int rt_igmpHook_whiteList_add( rt_igmpHook_whiteList_t *patten,rt_igmpHook_devInfo_t devInfo , int *index)
{
	int _devifid;
	int ret=FAIL;
	igmp_spin_lock_bh(igmpSysdb.lock_igmp);
	_devifid = rtk_igmp_devInfoToIfIdx(devInfo);
	if(_devifid!=FAIL)
		ret = rtk_igmp_whiteListAdd(patten,_devifid,index);
	igmp_spin_unlock_bh(igmpSysdb.lock_igmp);
	return ret;
}
int rt_igmpHook_whiteList_del(rt_igmpHook_whiteList_t *patten,rt_igmpHook_devInfo_t devInfo,int index)
{
	int _devifid;
	int ret=FAIL;
	igmp_spin_lock_bh(igmpSysdb.lock_igmp);
	_devifid = rtk_igmp_devInfoToIfIdx(devInfo);
	if(_devifid!=FAIL)
		ret = rtk_igmp_whiteListDel(patten,_devifid,index);
	igmp_spin_unlock_bh(igmpSysdb.lock_igmp);
	return ret;
}


int rt_igmpHook_devConfig_set(rt_igmpHook_devInfo_t devInfo , unsigned int isIpv6 ,rt_igmpHook_control_list_t type , unsigned int value)
{
	int _devifid;
	int ret=FAIL;
	igmp_spin_lock_bh(igmpSysdb.lock_igmp);
	_devifid = rtk_igmp_devInfoToIfIdx(devInfo);
	if(_devifid!=FAIL)
		ret = rtk_igmp_devConfig_set(_devifid,isIpv6,type,value);
	igmp_spin_unlock_bh(igmpSysdb.lock_igmp);
	return ret;
}
int rt_igmpHook_devConfig_get(rt_igmpHook_devInfo_t devInfo , unsigned int isIpv6 ,rt_igmpHook_control_list_t type , unsigned int* value)
{
	int _devifid;
	int ret=FAIL;
	igmp_spin_lock_bh(igmpSysdb.lock_igmp);
	_devifid = rtk_igmp_devInfoToIfIdx(devInfo);
	if(_devifid!=FAIL)
		ret=rtk_igmp_devConfig_get(_devifid,isIpv6,type,value);
	igmp_spin_unlock_bh(igmpSysdb.lock_igmp);
	return ret;
}


int rt_igmpHook_brDevConfig_set(rt_igmpHook_devInfo_t devInfo ,rt_igmpHook_br_control_list_t type , unsigned int value)
{
	int _devifid;
	int ret=FAIL;
	igmp_spin_lock_bh(igmpSysdb.lock_igmp);
	_devifid = rtk_igmp_devInfoToIfIdx(devInfo);
	if(_devifid!=FAIL)
		ret = rtk_igmp_BrDevConfig_set(_devifid,type,value);
	igmp_spin_unlock_bh(igmpSysdb.lock_igmp);
	return ret;
}
int rt_igmpHook_brDevConfig_get(rt_igmpHook_devInfo_t devInfo ,rt_igmpHook_br_control_list_t type , unsigned int* value)
{
	int _devifid;
	int ret=FAIL;
	igmp_spin_lock_bh(igmpSysdb.lock_igmp);
	_devifid = rtk_igmp_devInfoToIfIdx(devInfo);
	if(_devifid!=FAIL)
		ret = rtk_igmp_BrDevConfig_get(_devifid,type,value);
	igmp_spin_unlock_bh(igmpSysdb.lock_igmp);
	return ret;
}


int rt_igmpHook_groupToPsTbl_add(rt_igmpHook_ignoreGroup_t* patten,unsigned int *index)
{
	int ret=FAIL;
	igmp_spin_lock_bh(igmpSysdb.lock_igmp);
	ret = rtk_igmp_groupToPsTblAdd(patten,index);
	igmp_spin_unlock_bh(igmpSysdb.lock_igmp);
	return ret;
}
int rt_igmpHook_groupToPsTbl_del(rt_igmpHook_ignoreGroup_t* patten,unsigned int index)
{
	int ret=FAIL;
	igmp_spin_lock_bh(igmpSysdb.lock_igmp);
	ret = rtk_igmp_groupToPsTblDel(patten,index);
	igmp_spin_unlock_bh(igmpSysdb.lock_igmp);
	return ret;
}

int rt_igmpHook_nextValidGroupStatistic_get(rt_igmpHook_devInfo_t devInfo,int *getIdx,unsigned int isIpv6,rt_igmpHook_groupStatistic_t *statisticInfo)
{
	int ret=FAIL;
	igmp_spin_lock_bh(igmpSysdb.lock_igmp);
	ret =rtk_igmp_nextValidGroupStatistic_get(devInfo,getIdx,isIpv6,statisticInfo);
	igmp_spin_unlock_bh(igmpSysdb.lock_igmp);
	return ret;
}

int rt_igmpHook_devStatistic_get(rt_igmpHook_devInfo_t devInfo,unsigned int isIpv6,rt_igmpHook_devStatistic_t *statisticInfo)
{
	int ret=FAIL;
	igmp_spin_lock_bh(igmpSysdb.lock_igmp);
	ret =rtk_igmp_devStatistic_get(devInfo,isIpv6,statisticInfo);
	igmp_spin_unlock_bh(igmpSysdb.lock_igmp);
	return ret;

}

int rt_igmpHook_groupWeight_add(rt_igmpHook_devInfo_t devInfo,rt_igmpHook_groupWeight_t* groupWeightCfg, int *index)
{
	int ret=FAIL;
	igmp_spin_lock_bh(igmpSysdb.lock_igmp);
	ret =rtk_igmp_groupWeightTblAdd(devInfo,groupWeightCfg,index);
	igmp_spin_unlock_bh(igmpSysdb.lock_igmp);
	return ret;
}
int rt_igmpHook_groupWeight_del(rt_igmpHook_devInfo_t devInfo,rt_igmpHook_groupWeight_t* groupWeightCfg, int index)
{
	int ret=FAIL;
	igmp_spin_lock_bh(igmpSysdb.lock_igmp);
	ret =rtk_igmp_groupWeightTblDel(devInfo,groupWeightCfg,index);
	igmp_spin_unlock_bh(igmpSysdb.lock_igmp);
	return ret;
}




EXPORT_SYMBOL(rt_igmpHook_whiteList_add);
EXPORT_SYMBOL(rt_igmpHook_whiteList_del);
EXPORT_SYMBOL(rt_igmpHook_devConfig_set);
EXPORT_SYMBOL(rt_igmpHook_devConfig_get);
EXPORT_SYMBOL(rt_igmpHook_brDevConfig_set);
EXPORT_SYMBOL(rt_igmpHook_brDevConfig_get);
EXPORT_SYMBOL(rt_igmpHook_groupToPsTbl_add);
EXPORT_SYMBOL(rt_igmpHook_groupToPsTbl_del);
EXPORT_SYMBOL(rt_igmpHook_nextValidGroupStatistic_get);
EXPORT_SYMBOL(rt_igmpHook_devStatistic_get);
EXPORT_SYMBOL(rt_igmpHook_groupWeight_add);
EXPORT_SYMBOL(rt_igmpHook_groupWeight_del);




