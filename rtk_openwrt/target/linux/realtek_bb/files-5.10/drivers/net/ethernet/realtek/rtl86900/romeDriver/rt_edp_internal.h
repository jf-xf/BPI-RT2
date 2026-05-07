#ifndef __RTK_RG_INTERNAL_H__
#define __RTK_RG_INTERNAL_H__

#include <linux/kmod.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/if_ether.h>
#include <common/rt_type.h>
#include <common/type.h>
#include <rt_edp_debug.h>
#include <rt_edp_struct.h>

#define rtlglue_printf printk
#ifndef bzero
#define bzero(p,s) memset(p,0,s)
#endif

int assert_eq(int func_return,int expect_return,const char* func,int line);

#ifndef ASSERT_EQ
#define ASSERT_EQ(value1, value2) \
do { \
	int func_return=(value1); \
	if(assert_eq((func_return), (value2), __FUNCTION__,__LINE__)!=0) \
		return func_return; \
}while(0)
#endif

int _rt_edp_pipe_cmd(const char *comment, ...) ;
void _rt_edp_checkVlanBindingAndAddVconfig(rt_edp_port_idx_t portIdx, int vlanId);
void _rt_edp_dump_stack(void);
void _rt_edp_error_id_mapping(int id, char *mappingName);
rt_edp_err_code_t _rt_edp_return_err_mapping(int err_id, char *err_name,int line);

int8 *_rt_edp_inet_mactoa_p(const uint8 *mac, int8 *p);
int8 *_rt_edp_inet_ntoa(rtk_ip_addr_t ina);
int8 *_rt_edp_inet_ntoa_p(rtk_ip_addr_t ina, int8 *p);
int8 *_rt_edp_inet_n6toa(const uint8 *ipv6);
int8 *_rt_edp_inet_n6toa_p(const uint8 *ipv6, int8 *p);

rt_edp_err_code_t _rt_edp_freeWanDevIdx_get(rt_edp_wan_dev_type_t wanDevType, int *wanDevIdx);
rt_edp_err_code_t _rt_edp_freeWanDevIdx_put(rt_edp_wan_dev_type_t wanDevType, int wanDevIdx);
rt_edp_err_code_t _rt_edp_internal_wanSet(int wan_intf_idx, rt_edp_ipStaticInfo_t *static_info);
rt_edp_err_code_t _rt_edp_dhcp_release(int wan_intf_idx, rt_edp_ipStaticInfo_t *static_info);


#endif
