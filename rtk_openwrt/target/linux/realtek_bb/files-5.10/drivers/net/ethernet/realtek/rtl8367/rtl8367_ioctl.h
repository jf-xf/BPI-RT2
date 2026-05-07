#ifndef _RTL8367_IOCTL_H
#define _RTL8367_IOCTL_H_
#include <linux/ioctl.h>

#include <rtk_types.h>
#include <rtk_switch.h>
#include <port.h>
#include <svlan.h>
#include <stat.h>
#include <led.h>
#include <l2.h>

#define RTL8367_IOCTL_NAME			"rtl8367"

/* commands - up to 31 */
#define RTL8367_NODE_MAGIC			'R'
#define RTL8367_NONE_CMD			(_IO(RTL8367_NODE_MAGIC, 0))
#define RTL8367_SWITCH_INIT_CMD			(_IOWR(RTL8367_NODE_MAGIC, 1, rtl8367_cmd_switch_init_t))
#define RTL8367_PORT_PHYSTATUS_GET_CMD		(_IOWR(RTL8367_NODE_MAGIC, 2, rtl8367_cmd_port_phyStatus_t))
#define RTL8367_STAT_PORT_GETALL_CMD		(_IOWR(RTL8367_NODE_MAGIC, 3, rtl8367_cmd_stat_port_getAll_t))
#define RTL8367_SVLAN_INIT_CMD			(_IOWR(RTL8367_NODE_MAGIC, 4, rtl8367_cmd_svlan_init_t))
#define RTL8367_SVLAN_SERVICEPORT_ADD_CMD	(_IOWR(RTL8367_NODE_MAGIC, 5, rtl8367_cmd_svlan_servicePort_add_t))
#define RTL8367_SVLAN_SERVICEPORT_GET_CMD	(_IOWR(RTL8367_NODE_MAGIC, 6, rtl8367_cmd_svlan_servicePort_get_t))
#define RTL8367_SVLAN_MEMBERPORTENTRY_SET_CMD	(_IOWR(RTL8367_NODE_MAGIC, 7, rtl8367_cmd_svlan_memberPortEntry_t))
#define RTL8367_SVLAN_MEMBERPORTENTRY_GET_CMD	(_IOWR(RTL8367_NODE_MAGIC, 8, rtl8367_cmd_svlan_memberPortEntry_t))
#define RTL8367_SVLAN_DEFAULTSVLAN_SET_CMD	(_IOWR(RTL8367_NODE_MAGIC, 9, rtl8367_cmd_svlan_defaultSvlan_t))
#define RTL8367_SVLAN_DEFAULTSVLAN_GET_CMD	(_IOWR(RTL8367_NODE_MAGIC, 10, rtl8367_cmd_svlan_defaultSvlan_t))
#define RTL8367_LED_MODEFORCE_SET_CMD		(_IOWR(RTL8367_NODE_MAGIC, 11, rtl8367_cmd_led_modeForce_t))
#define RTL8367_LED_MODEFORCE_GET_CMD		(_IOWR(RTL8367_NODE_MAGIC, 12, rtl8367_cmd_led_modeForce_t))
#define RTL8367_PORT_PHYAUTONEGOABILITY_SET_CMD		(_IOWR(RTL8367_NODE_MAGIC, 13, rtksw_cmd_port_phy_ability_t))
#define RTL8367_PORT_PHYAUTONEGOABILITY_GET_CMD		(_IOWR(RTL8367_NODE_MAGIC, 14, rtksw_cmd_port_phy_ability_t))
#define RTL8367_L2_LIMITLEARNINGCNTACTION_SET_CMD	(_IOWR(RTL8367_NODE_MAGIC, 15, rtl8367_cmd_l2_limitLearningCntAction_t))
#define RTL8367_L2_LIMITLEARNINGCNTACTION_GET_CMD	(_IOWR(RTL8367_NODE_MAGIC, 16, rtl8367_cmd_l2_limitLearningCntAction_t))
#define RTL8367_L2_LIMITLEARNINGCNT_SET_CMD		(_IOWR(RTL8367_NODE_MAGIC, 17, rtl8367_cmd_l2_limitLearningCnt_t))
#define RTL8367_L2_LIMITLEARNINGCNT_GET_CMD		(_IOWR(RTL8367_NODE_MAGIC, 18, rtl8367_cmd_l2_limitLearningCnt_t))

typedef struct rtl8367_cmd_switch_init_s {
	rtksw_api_ret_t ret;
} rtl8367_cmd_switch_init_t;

typedef struct rtl8367_cmd_port_phyStatus_s {
	rtksw_api_ret_t ret;
	rtksw_port_t port;
	rtksw_port_linkStatus_t linkStatus;
	rtksw_port_speed_t speed;
	rtksw_port_duplex_t duplex;
} rtl8367_cmd_port_phyStatus_t;

typedef struct rtl8367_cmd_stat_port_getAll_s {
	rtksw_api_ret_t ret;
	rtksw_port_t port;
	rtksw_stat_port_cntr_t port_cntrs;
} rtl8367_cmd_stat_port_getAll_t;

typedef struct rtl8367_cmd_svlan_init_s {
	rtksw_api_ret_t ret;
} rtl8367_cmd_svlan_init_t;

typedef struct rtl8367_cmd_svlan_servicePort_add_s {
	rtksw_api_ret_t ret;
	rtksw_port_t port;
} rtl8367_cmd_svlan_servicePort_add_t;

typedef struct rtl8367_cmd_svlan_servicePort_get_s {
	rtksw_api_ret_t ret;
	rtksw_portmask_t portmask;
} rtl8367_cmd_svlan_servicePort_get_t;

typedef struct rtl8367_cmd_svlan_memberPortEntry_s {
	rtksw_api_ret_t ret;
	rtksw_vlan_t svid;
	rtksw_svlan_memberCfg_t svlan_cfg;
} rtl8367_cmd_svlan_memberPortEntry_t;

typedef struct rtl8367_cmd_svlan_defaultSvlan_s {
	rtksw_api_ret_t ret;
	rtksw_port_t port;
	rtksw_vlan_t svid;
} rtl8367_cmd_svlan_defaultSvlan_t;

typedef struct rtl8367_cmd_led_modeForce_s {
	rtksw_api_ret_t ret;
	rtksw_port_t port;
	rtksw_led_group_t group;
	rtksw_led_force_mode_t mode;
} rtl8367_cmd_led_modeForce_t;

typedef struct rtksw_cmd_port_phy_ability_s
{
	rtksw_api_ret_t ret;
	rtksw_port_t port;
    rtksw_port_phy_ability_t ability;
} rtksw_cmd_port_phy_ability_t;

typedef struct rtl8367_cmd_l2_limitLearningCntAction_s {
	rtksw_api_ret_t ret;
	rtksw_port_t port;
	rtksw_l2_limitLearnCntAction_t action;
} rtl8367_cmd_l2_limitLearningCntAction_t;

typedef struct rtl8367_cmd_l2_limitLearningCnt_s {
	rtksw_api_ret_t ret;
	rtksw_port_t port;
	rtksw_mac_cnt_t mac_cnt;
} rtl8367_cmd_l2_limitLearningCnt_t;


#ifdef __KERNEL__
/*******************************
 * Only for Kernel Space       *
 *******************************/
#define RTL8367_MSG(fmt,args...)	printk(KERN_INFO "rtl8367: "fmt"\n", ##args)
#define RTL8367_INFO(fmt,args...)	printk(KERN_INFO "\033[1;33;46m""rtl8367: "fmt"\033[m""\n", ##args)
#define RTL8367_ERROR(fmt,args...)	printk(KERN_INFO "\033[1;33;41m""rtl8367: "fmt"\033[m""\n", ##args)
//#define RTL8367_PHY_DEBUG
#ifdef RTL8367_PHY_DEBUG
#define RTL8367_DEBUG			RTL8367_INFO
#else
#define RTL8367_DEBUG(fmt,args...)
#endif

void rtk_rtl8367_ioctl_init(void);

#else
/*******************************
 * Only for User Space         *
 *******************************/
#include <unistd.h>

#define RTL8367_MSG(fmt,args...)	printf("rtl8367: "fmt"\n", ##args)
#define RTL8367_INFO(fmt,args...)	printf("\033[1;33;46m""rtl8367: "fmt"\033[m""\n", ##args)
#define RTL8367_ERROR(fmt,args...)	printf("\033[1;33;41m""rtl8367: "fmt"\033[m""\n", ##args)
//#define RTL8367_PHY_DEBUG
#ifdef RTL8367_PHY_DEBUG
#define RTL8367_DEBUG			RTL8367_INFO
#else
#define RTL8367_DEBUG(fmt,args...)
#endif

#endif

#endif /* _RTL8367_H_ */

