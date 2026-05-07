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


/*
 * Include Files
 */
//#include <asm/uaccess.h>
#include <net/sock.h>
#include <common/rt_error.h>

#include <rtdrv_ext_igmpHook_netfilter.h>


/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */


/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */


#if LINUX_VERSION_CODE < KERNEL_VERSION(5,10,0)
int32 do_rtdrv_ext_igmpHook_set_ctl(struct sock *sk, int cmd, void *user, unsigned int len)
#else
int32 do_rtdrv_ext_igmpHook_set_ctl(struct sock *sk, int cmd, sockptr_t arg, unsigned int len)
#endif
{
	int32                           ret = RT_ERR_FAILED;
	rtdrv_ext_igmpHook_union_t                buf;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,10,0)
    void *user = arg.user;
#endif

	switch(cmd)
	{
        case RTDRV_RT_IGMPHOOK_WHITELIST_ADD:
            copy_from_user(&buf.rt_igmpHook_whiteList_cfg, user, sizeof(rtdrv_rt_igmpHook_whiteList_ext_t));
            ret = rt_igmpHook_whiteList_add(&buf.rt_igmpHook_whiteList_cfg.mcConfig, buf.rt_igmpHook_whiteList_cfg.devInfo,&buf.rt_igmpHook_whiteList_cfg.index);
            copy_to_user(user, &buf.rt_igmpHook_whiteList_cfg, sizeof(rtdrv_rt_igmpHook_whiteList_ext_t));
            break;
			
        case RTDRV_RT_IGMPHOOK_WHITELIST_DEL:
			/*we only support delete by index now*/
            copy_from_user(&buf.rt_igmpHook_whiteList_cfg, user, sizeof(rtdrv_rt_igmpHook_whiteList_ext_t));
			if(buf.rt_igmpHook_whiteList_cfg.index>=0)
	            ret = rt_igmpHook_whiteList_del(NULL, buf.rt_igmpHook_whiteList_cfg.devInfo,buf.rt_igmpHook_whiteList_cfg.index);
			else
				ret = rt_igmpHook_whiteList_del(&buf.rt_igmpHook_whiteList_cfg.mcConfig, buf.rt_igmpHook_whiteList_cfg.devInfo,buf.rt_igmpHook_whiteList_cfg.index);
            break;

		case RTDRV_RT_IGMPHOOK_DEV_CONFIG_GET:
			copy_from_user(&buf.rt_igmpHook_devConfig, user, sizeof(rtdrv_rt_igmpHook_devConfig_ext_t));
			ret = rt_igmpHook_devConfig_get(buf.rt_igmpHook_devConfig.devInfo, buf.rt_igmpHook_devConfig.isIpv6,buf.rt_igmpHook_devConfig.type,&buf.rt_igmpHook_devConfig.value);
			copy_to_user(user, &buf.rt_igmpHook_devConfig, sizeof(rtdrv_rt_igmpHook_devConfig_ext_t));
			break;

			
		case RTDRV_RT_IGMPHOOK_DEV_CONFIG_SET:
			copy_from_user(&buf.rt_igmpHook_devConfig, user, sizeof(rtdrv_rt_igmpHook_devConfig_ext_t));
			ret = rt_igmpHook_devConfig_set(buf.rt_igmpHook_devConfig.devInfo, buf.rt_igmpHook_devConfig.isIpv6,buf.rt_igmpHook_devConfig.type,buf.rt_igmpHook_devConfig.value);
			break;

		case RTDRV_RT_IGMPHOOK_BRDEV_CONFIG_GET:
			copy_from_user(&buf.rt_igmpHook_brDevConfig, user, sizeof(rtdrv_rt_igmpHook_brDevConfig_ext_t));
			ret = rt_igmpHook_brDevConfig_get(buf.rt_igmpHook_brDevConfig.devInfo,buf.rt_igmpHook_brDevConfig.type,&buf.rt_igmpHook_brDevConfig.value);
			copy_to_user(user, &buf.rt_igmpHook_brDevConfig, sizeof(rtdrv_rt_igmpHook_brDevConfig_ext_t));
			break;


		case RTDRV_RT_IGMPHOOK_BRDEV_CONFIG_SET:
			copy_from_user(&buf.rt_igmpHook_brDevConfig, user, sizeof(rtdrv_rt_igmpHook_brDevConfig_ext_t));
			ret = rt_igmpHook_brDevConfig_set(buf.rt_igmpHook_brDevConfig.devInfo,buf.rt_igmpHook_brDevConfig.type,buf.rt_igmpHook_brDevConfig.value);
			break;
			
		case RTDRV_RT_IGMPHOOK_GROUPTOPS_DEL:
			/*we only support delete by index now*/
			copy_from_user(&buf.rt_igmpHook_groupToPsTbl, user, sizeof(rtdrv_rt_igmpHook_groupToPsTbl_ext_t));
			if(buf.rt_igmpHook_groupToPsTbl.index>=0)
				ret = rt_igmpHook_groupToPsTbl_del(NULL,buf.rt_igmpHook_groupToPsTbl.index);
			else
				ret = rt_igmpHook_groupToPsTbl_del(&buf.rt_igmpHook_groupToPsTbl.patten,buf.rt_igmpHook_groupToPsTbl.index);
			break;

		case RTDRV_RT_IGMPHOOK_GROUPTOPS_ADD:
			copy_from_user(&buf.rt_igmpHook_groupToPsTbl, user, sizeof(rtdrv_rt_igmpHook_groupToPsTbl_ext_t));
			ret = rt_igmpHook_groupToPsTbl_add(&buf.rt_igmpHook_groupToPsTbl.patten,&buf.rt_igmpHook_groupToPsTbl.index);
			copy_to_user(user, &buf.rt_igmpHook_groupToPsTbl, sizeof(rtdrv_rt_igmpHook_groupToPsTbl_ext_t));
			break;

		case RTDRV_RT_IGMPHOOK_DEV_STATISTIC_GET:
			copy_from_user(&buf.rt_igmpHook_devStatistic, user, sizeof(rtdrv_rt_igmpHook_devStatistic_ext_t));
			ret = rt_igmpHook_devStatistic_get(buf.rt_igmpHook_devStatistic.devInfo,buf.rt_igmpHook_devStatistic.isIpv6,&buf.rt_igmpHook_devStatistic.devStatistic);
			copy_to_user(user, &buf.rt_igmpHook_devStatistic, sizeof(rtdrv_rt_igmpHook_devStatistic_ext_t));
			break;
				
		case RTDRV_RT_IGMPHOOK_NEXT_GROUP_STATISTIC_GET:
			copy_from_user(&buf.rt_igmpHook_groupStatistic, user, sizeof(rtdrv_rt_igmpHook_groupStatistic_ext_t));
			ret = rt_igmpHook_nextValidGroupStatistic_get(buf.rt_igmpHook_groupStatistic.devInfo,&buf.rt_igmpHook_groupStatistic.getIdx,buf.rt_igmpHook_groupStatistic.isIpv6,&buf.rt_igmpHook_groupStatistic.groupStatistic);
			copy_to_user(user, &buf.rt_igmpHook_groupStatistic, sizeof(rtdrv_rt_igmpHook_groupStatistic_ext_t));
			break;				
		case RTDRV_RT_IGMPHOOK_GROUP_WEIGHT_ADD:
			copy_from_user(&buf.rt_igmpHook_groupWeight, user, sizeof(rtdrv_rt_igmpHook_groupWeight_ext_t));
			ret = rt_igmpHook_groupWeight_add(buf.rt_igmpHook_groupWeight.devInfo,&buf.rt_igmpHook_groupWeight.groupWeightCfg,&buf.rt_igmpHook_groupWeight.index);
			copy_to_user(user, &buf.rt_igmpHook_groupWeight, sizeof(rtdrv_rt_igmpHook_groupWeight_ext_t));
			break;
		case RTDRV_RT_IGMPHOOK_GROUP_WEIGHT_DEL:
			copy_from_user(&buf.rt_igmpHook_groupWeight, user, sizeof(rtdrv_rt_igmpHook_groupWeight_ext_t));
			ret = rt_igmpHook_groupWeight_del(buf.rt_igmpHook_groupWeight.devInfo,&buf.rt_igmpHook_groupWeight.groupWeightCfg,buf.rt_igmpHook_groupWeight.index);
			break;

			
		default:
			printk("do_rtdrv_ext_igmpHook_set_ctl unknown cmd (%d)\n", cmd);
		
	}
	return ret;
}


int32 do_rtdrv_ext_igmpHook_get_ctl(struct sock *sk, int cmd, void *user, int *len)
{
	int32               ret = RT_ERR_FAILED;
	//rtdrv_ext_igmpHook_union_t    buf;

	switch(cmd)
	{

		default:
			printk("do_rtdrv_ext_igmpHook_get_ctl unknown cmd (%d)\n", cmd);
	}
	return ret;
}

