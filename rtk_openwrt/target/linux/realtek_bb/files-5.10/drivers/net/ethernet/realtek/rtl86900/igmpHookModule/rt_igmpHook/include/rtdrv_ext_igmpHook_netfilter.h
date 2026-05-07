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
 * Purpose : Realtek Switch SDK Rtdrv Netfilter Extension Module in the SDK.
 *
 * Feature : Realtek Switch SDK Rtdrv Netfilter Extension Module
 *
 */


#ifndef __RTDRV_EXT_IGMPHOOK_NETFILTER_H__
#define __RTDRV_EXT_IGMPHOOK_NETFILTER_H__

/*
 * Include Files
*/
#if defined(__KERNEL__)
#include <linux/version.h>
#include <net/sock.h>
#endif
#include <rtdrv/rtdrv_cmdoffset.h>

#include <rt_igmpHook_ext.h>


#if defined(__KERNEL__) 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0) 
int32 do_rtdrv_ext_igmpHook_set_ctl(struct sock *sk, int cmd, sockptr_t arg, unsigned int len);
#else
int32 do_rtdrv_ext_igmpHook_set_ctl(struct sock *sk, int cmd, void *user, unsigned int len);
#endif
#else
int32 do_rtdrv_ext_igmpHook_set_ctl(struct sock *sk, int cmd, void *user, unsigned int len);
#endif
int32 do_rtdrv_ext_igmpHook_get_ctl(struct sock *sk, int cmd, void *user, int *len);


/* RT EXT SET ENUM */
enum rtdrv_ext_igmpHook_set_e{
    RTDRV_RT_IGMPHOOK_WHITELIST_ADD = (RTDRV_BASE_CTL + RTDRV_EXT_IGMPHOOK_OFFSET),
    RTDRV_RT_IGMPHOOK_WHITELIST_DEL,
	RTDRV_RT_IGMPHOOK_DEV_CONFIG_SET,
	RTDRV_RT_IGMPHOOK_DEV_CONFIG_GET,
	RTDRV_RT_IGMPHOOK_BRDEV_CONFIG_SET,
	RTDRV_RT_IGMPHOOK_BRDEV_CONFIG_GET,	
	RTDRV_RT_IGMPHOOK_GROUPTOPS_ADD,
	RTDRV_RT_IGMPHOOK_GROUPTOPS_DEL,
	RTDRV_RT_IGMPHOOK_DEV_STATISTIC_GET,
	RTDRV_RT_IGMPHOOK_NEXT_GROUP_STATISTIC_GET,
	RTDRV_RT_IGMPHOOK_GROUP_WEIGHT_ADD,
	RTDRV_RT_IGMPHOOK_GROUP_WEIGHT_DEL,	
};


typedef struct rtdrv_rt_igmpHook_whiteList_ext_s
{
    int	index;
	rt_igmpHook_devInfo_t	devInfo;
    rt_igmpHook_whiteList_t		mcConfig;
}rtdrv_rt_igmpHook_whiteList_ext_t;


typedef struct rtdrv_rt_igmpHook_devConfig_ext_s
{
    rt_igmpHook_devInfo_t	devInfo;
    unsigned int			isIpv6;
	rt_igmpHook_control_list_t type;
	unsigned int 			value;
}rtdrv_rt_igmpHook_devConfig_ext_t;

typedef struct rtdrv_rt_igmpHook_brDevConfig_ext_s
{
    rt_igmpHook_devInfo_t	devInfo;
	rt_igmpHook_control_list_t type;
	unsigned int 			value;
}rtdrv_rt_igmpHook_brDevConfig_ext_t;


typedef struct rtdrv_rt_igmpHook_groupToPsTbl_ext_s
{
	int index;
    rt_igmpHook_ignoreGroup_t	patten;
}rtdrv_rt_igmpHook_groupToPsTbl_ext_t;


typedef struct rtdrv_rt_igmpHook_groupWeight_ext_s
{
	int index;
	rt_igmpHook_devInfo_t	devInfo;
    rt_igmpHook_groupWeight_t	groupWeightCfg;
}rtdrv_rt_igmpHook_groupWeight_ext_t;


typedef struct rtdrv_rt_igmpHook_devStatistic_ext_s
{
	rt_igmpHook_devInfo_t	devInfo;
	unsigned int	isIpv6;
    rt_igmpHook_devStatistic_t	devStatistic;
}rtdrv_rt_igmpHook_devStatistic_ext_t;


typedef struct rtdrv_rt_igmpHook_groupStatistic_ext_s
{
	rt_igmpHook_devInfo_t	devInfo;
	int	getIdx;
	unsigned int	isIpv6;
    rt_igmpHook_groupStatistic_t	groupStatistic;
}rtdrv_rt_igmpHook_groupStatistic_ext_t;


typedef union rtdrv_ext_igmpHook_union_u
{
	rtdrv_rt_igmpHook_whiteList_ext_t			rt_igmpHook_whiteList_cfg;
	rtdrv_rt_igmpHook_devConfig_ext_t			rt_igmpHook_devConfig;
	rtdrv_rt_igmpHook_brDevConfig_ext_t			rt_igmpHook_brDevConfig;
	rtdrv_rt_igmpHook_groupToPsTbl_ext_t		rt_igmpHook_groupToPsTbl;
	rtdrv_rt_igmpHook_groupWeight_ext_t			rt_igmpHook_groupWeight;
	rtdrv_rt_igmpHook_devStatistic_ext_t		rt_igmpHook_devStatistic;
	rtdrv_rt_igmpHook_groupStatistic_ext_t		rt_igmpHook_groupStatistic;
} rtdrv_ext_igmpHook_union_t;



#endif // __RTDRV_EXT_IGMPHOOK_NETFILTER_H__


