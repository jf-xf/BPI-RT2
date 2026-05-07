#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>	/* misc_register */
#include <linux/string.h>

#include <rtl8367_ioctl.h>

#define KERNEL_IOCTL_CMD_MAX_NUM	32
#define KERNEL_IOCTL_CMD_NAME_MAX_LEN	128

typedef int kernel_ioctl_function_t(void *arg);
typedef struct {
	unsigned int cmd;
	char name[KERNEL_IOCTL_CMD_NAME_MAX_LEN];
	kernel_ioctl_function_t * handler;
} kernel_ioctl_cmd_def_t;

#define KERNEL_IOCTL_FUNCTION(func, cmd) \
	int ioctl_##func(void *arg); \
	kernel_ioctl_cmd_def_t func##_cmd = { \
		cmd, \
		#func, \
		ioctl_##func \
	}; \
	int ioctl_##func(void *arg)

#define KERNEL_IOCTL_CMD_INSTALL(func) \
	rtk_rtl8367_cmd_reg(&func##_cmd)

/* Define ioctl command and handler */
#define rtk_rtl8367_cmd_install() do { \
		KERNEL_IOCTL_CMD_INSTALL(rtksw_switch_init); \
		KERNEL_IOCTL_CMD_INSTALL(rtksw_port_phyStatus_get); \
		KERNEL_IOCTL_CMD_INSTALL(rtksw_stat_port_getAll); \
		KERNEL_IOCTL_CMD_INSTALL(rtksw_svlan_init); \
		KERNEL_IOCTL_CMD_INSTALL(rtksw_svlan_servicePort_add); \
		KERNEL_IOCTL_CMD_INSTALL(rtksw_svlan_servicePort_get); \
		KERNEL_IOCTL_CMD_INSTALL(rtksw_svlan_memberPortEntry_set); \
		KERNEL_IOCTL_CMD_INSTALL(rtksw_svlan_memberPortEntry_get); \
		KERNEL_IOCTL_CMD_INSTALL(rtksw_svlan_defaultSvlan_set); \
		KERNEL_IOCTL_CMD_INSTALL(rtksw_svlan_defaultSvlan_get); \
		KERNEL_IOCTL_CMD_INSTALL(rtksw_led_modeForce_set); \
		KERNEL_IOCTL_CMD_INSTALL(rtksw_led_modeForce_get); \
		KERNEL_IOCTL_CMD_INSTALL(rtksw_port_phyAutoNegoAbility_set); \
		KERNEL_IOCTL_CMD_INSTALL(rtksw_port_phyAutoNegoAbility_get); \
		KERNEL_IOCTL_CMD_INSTALL(rtksw_l2_limitLearningCntAction_set); \
		KERNEL_IOCTL_CMD_INSTALL(rtksw_l2_limitLearningCntAction_get); \
		KERNEL_IOCTL_CMD_INSTALL(rtksw_l2_limitLearningCnt_set); \
		KERNEL_IOCTL_CMD_INSTALL(rtksw_l2_limitLearningCnt_get); \
	} while (0)

KERNEL_IOCTL_FUNCTION(rtksw_switch_init, RTL8367_SWITCH_INIT_CMD)
{
	rtl8367_cmd_switch_init_t switchInit;

	copy_from_user(&switchInit, (struct rtl8367_cmd_switch_init_t *)arg, sizeof(switchInit));

	switchInit.ret = rtksw_switch_init();

	copy_to_user((struct rtl8367_cmd_switch_init_t *)arg, &switchInit, sizeof(switchInit));

	return 0;
}

KERNEL_IOCTL_FUNCTION(rtksw_port_phyStatus_get, RTL8367_PORT_PHYSTATUS_GET_CMD)
{
	rtl8367_cmd_port_phyStatus_t portPhyStatus;

	rtksw_port_linkStatus_t linkStatus;
	rtksw_port_speed_t speed;
	rtksw_port_duplex_t duplex;

	copy_from_user(&portPhyStatus, (struct rtl8367_cmd_port_phyStatus_t *)arg, sizeof(portPhyStatus));

	portPhyStatus.ret = rtksw_port_phyStatus_get(portPhyStatus.port, &linkStatus, &speed, &duplex);
	portPhyStatus.linkStatus = linkStatus;
	portPhyStatus.speed = speed;
	portPhyStatus.duplex = duplex;

	copy_to_user((struct rtl8367_cmd_port_phyStatus_t *)arg, &portPhyStatus, sizeof(portPhyStatus));

	return 0;
}

KERNEL_IOCTL_FUNCTION(rtksw_stat_port_getAll, RTL8367_STAT_PORT_GETALL_CMD)
{
	rtl8367_cmd_stat_port_getAll_t statPortGetAll;
	rtksw_stat_port_cntr_t port_cntrs;

	copy_from_user(&statPortGetAll, (struct rtl8367_cmd_stat_port_getAll_t *)arg, sizeof(statPortGetAll));

	statPortGetAll.ret = rtksw_stat_port_getAll(statPortGetAll.port, &port_cntrs);
	statPortGetAll.port_cntrs = port_cntrs;

	copy_to_user((struct rtl8367_cmd_stat_port_getAll_t *)arg, &statPortGetAll, sizeof(statPortGetAll));

	return 0;
}

KERNEL_IOCTL_FUNCTION(rtksw_svlan_init, RTL8367_SVLAN_INIT_CMD)
{
	rtl8367_cmd_svlan_init_t svlanInit;

	copy_from_user(&svlanInit, (struct rtl8367_cmd_svlan_init_t *)arg, sizeof(svlanInit));

	svlanInit.ret = rtksw_svlan_init();

	copy_to_user((struct rtl8367_cmd_svlan_init_t *)arg, &svlanInit, sizeof(svlanInit));

	return 0;
}

KERNEL_IOCTL_FUNCTION(rtksw_svlan_servicePort_add, RTL8367_SVLAN_SERVICEPORT_ADD_CMD)
{
	rtl8367_cmd_svlan_servicePort_add_t svlanServicePortAdd;

	copy_from_user(&svlanServicePortAdd, (struct rtl8367_cmd_svlan_servicePort_add_t *)arg, sizeof(svlanServicePortAdd));

	svlanServicePortAdd.ret = rtksw_svlan_servicePort_add(svlanServicePortAdd.port);

	copy_to_user((struct rtl8367_cmd_svlan_servicePort_add_t *)arg, &svlanServicePortAdd, sizeof(svlanServicePortAdd));

	return 0;
}

KERNEL_IOCTL_FUNCTION(rtksw_svlan_servicePort_get, RTL8367_SVLAN_SERVICEPORT_GET_CMD)
{
	rtl8367_cmd_svlan_servicePort_get_t svlanServicePortGet;
        rtksw_portmask_t portmask;

	copy_from_user(&svlanServicePortGet, (struct rtl8367_cmd_svlan_servicePort_get_t *)arg, sizeof(svlanServicePortGet));

	svlanServicePortGet.ret = rtksw_svlan_servicePort_get(&portmask);
        svlanServicePortGet.portmask = portmask;

	copy_to_user((struct rtl8367_cmd_svlan_servicePort_get_t *)arg, &svlanServicePortGet, sizeof(svlanServicePortGet));

	return 0;
}

KERNEL_IOCTL_FUNCTION(rtksw_svlan_memberPortEntry_set, RTL8367_SVLAN_MEMBERPORTENTRY_SET_CMD)
{
	rtl8367_cmd_svlan_memberPortEntry_t svlanMemberPortEntry;

	copy_from_user(&svlanMemberPortEntry, (struct rtl8367_cmd_svlan_memberPortEntry_t *)arg, sizeof(svlanMemberPortEntry));

	svlanMemberPortEntry.ret = rtksw_svlan_memberPortEntry_set(svlanMemberPortEntry.svid, &svlanMemberPortEntry.svlan_cfg);

	copy_to_user((struct rtl8367_cmd_svlan_memberPortEntry_t *)arg, &svlanMemberPortEntry, sizeof(svlanMemberPortEntry));

	return 0;
}

KERNEL_IOCTL_FUNCTION(rtksw_svlan_memberPortEntry_get, RTL8367_SVLAN_MEMBERPORTENTRY_GET_CMD)
{
	rtl8367_cmd_svlan_memberPortEntry_t svlanMemberPortEntry;
	rtksw_svlan_memberCfg_t svlan_cfg;

	copy_from_user(&svlanMemberPortEntry, (struct rtl8367_cmd_svlan_memberPortEntry_t *)arg, sizeof(svlanMemberPortEntry));

	svlanMemberPortEntry.ret = rtksw_svlan_memberPortEntry_get(svlanMemberPortEntry.svid, &svlan_cfg);
	svlanMemberPortEntry.svlan_cfg = svlan_cfg;

	copy_to_user((struct rtl8367_cmd_svlan_memberPortEntry_t *)arg, &svlanMemberPortEntry, sizeof(svlanMemberPortEntry));

	return 0;
}

KERNEL_IOCTL_FUNCTION(rtksw_svlan_defaultSvlan_set, RTL8367_SVLAN_DEFAULTSVLAN_SET_CMD)
{
	rtl8367_cmd_svlan_defaultSvlan_t svlanDefaultSvlan;

	copy_from_user(&svlanDefaultSvlan, (struct rtl8367_cmd_svlan_defaultSvlan_t *)arg, sizeof(svlanDefaultSvlan));

	svlanDefaultSvlan.ret = rtksw_svlan_defaultSvlan_set(svlanDefaultSvlan.port, svlanDefaultSvlan.svid);

	copy_to_user((struct rtl8367_cmd_svlan_defaultSvlan_t *)arg, &svlanDefaultSvlan, sizeof(svlanDefaultSvlan));

	return 0;
}

KERNEL_IOCTL_FUNCTION(rtksw_svlan_defaultSvlan_get, RTL8367_SVLAN_DEFAULTSVLAN_GET_CMD)
{
	rtl8367_cmd_svlan_defaultSvlan_t svlanDefaultSvlan;
	rtksw_vlan_t svid;

	copy_from_user(&svlanDefaultSvlan, (struct rtl8367_cmd_svlan_defaultSvlan_t *)arg, sizeof(svlanDefaultSvlan));

	svlanDefaultSvlan.ret = rtksw_svlan_defaultSvlan_get(svlanDefaultSvlan.port, &svid);
	svlanDefaultSvlan.svid = svid;

	copy_to_user((struct rtl8367_cmd_svlan_defaultSvlan_t *)arg, &svlanDefaultSvlan, sizeof(svlanDefaultSvlan));

	return 0;
}

KERNEL_IOCTL_FUNCTION(rtksw_led_modeForce_set, RTL8367_LED_MODEFORCE_SET_CMD)
{
	rtl8367_cmd_led_modeForce_t ledModeForce;

	copy_from_user(&ledModeForce, (struct rtl8367_cmd_led_modeForce_t *)arg, sizeof(ledModeForce));

	ledModeForce.ret = rtksw_led_modeForce_set(ledModeForce.port, ledModeForce.group, ledModeForce.mode);

	copy_to_user((struct rtl8367_cmd_led_modeForce_t *)arg, &ledModeForce, sizeof(ledModeForce));

	return 0;
}

KERNEL_IOCTL_FUNCTION(rtksw_led_modeForce_get, RTL8367_LED_MODEFORCE_GET_CMD)
{
	rtl8367_cmd_led_modeForce_t ledModeForce;
	rtksw_led_force_mode_t mode;

	copy_from_user(&ledModeForce, (struct rtl8367_cmd_led_modeForce_t *)arg, sizeof(ledModeForce));

	ledModeForce.ret = rtksw_led_modeForce_get(ledModeForce.port, ledModeForce.group, &mode);
	ledModeForce.mode = mode;

	copy_to_user((struct rtl8367_cmd_led_modeForce_t *)arg, &ledModeForce, sizeof(ledModeForce));

	return 0;
}

KERNEL_IOCTL_FUNCTION(rtksw_port_phyAutoNegoAbility_set, RTL8367_PORT_PHYAUTONEGOABILITY_SET_CMD)
{
	rtksw_cmd_port_phy_ability_t portAbility;

	copy_from_user(&portAbility, (struct rtksw_cmd_port_phy_ability_t *)arg, sizeof(portAbility));

	//printk("%s:%d port %d, AutoNegotiation %d, Half_10 %d, Full_10 %d, Half_100 %d, Full_100 %d, Full_1000 %d, FC %d, AsyFC %d\n", __FUNCTION__, __LINE__, portAbility.port, portAbility.ability.AutoNegotiation, portAbility.ability.Half_10, portAbility.ability.Full_10, portAbility.ability.Half_100, portAbility.ability.Full_100, portAbility.ability.Full_1000, portAbility.ability.FC, portAbility.ability.AsyFC);
	
	portAbility.ret = rtksw_port_phyAutoNegoAbility_set(portAbility.port, &portAbility.ability);
	
	copy_to_user((struct rtksw_cmd_port_phy_ability_t *)arg, &portAbility, sizeof(portAbility));

	return 0;
}

KERNEL_IOCTL_FUNCTION(rtksw_port_phyAutoNegoAbility_get, RTL8367_PORT_PHYAUTONEGOABILITY_GET_CMD)
{
	rtksw_cmd_port_phy_ability_t portAbility;

	copy_from_user(&portAbility, (struct rtksw_cmd_port_phy_ability_t *)arg, sizeof(portAbility));

	portAbility.ret = rtksw_port_phyAutoNegoAbility_get(portAbility.port, &portAbility.ability);
	
	//printk("%s:%d port %d, AutoNegotiation %d, Half_10 %d, Full_10 %d, Half_100 %d, Full_100 %d, Full_1000 %d, FC %d, AsyFC %d\n", __FUNCTION__, __LINE__, portAbility.port, portAbility.ability.AutoNegotiation, portAbility.ability.Half_10, portAbility.ability.Full_10, portAbility.ability.Half_100, portAbility.ability.Full_100, portAbility.ability.Full_1000, portAbility.ability.FC, portAbility.ability.AsyFC);
	copy_to_user((struct rtksw_cmd_port_phy_ability_t *)arg, &portAbility, sizeof(portAbility));

	return 0;
}

KERNEL_IOCTL_FUNCTION(rtksw_l2_limitLearningCntAction_set, RTL8367_L2_LIMITLEARNINGCNTACTION_SET_CMD)
{
	rtl8367_cmd_l2_limitLearningCntAction_t action;

	copy_from_user(&action, (struct rtl8367_cmd_l2_limitLearningCntAction_t *)arg, sizeof(action));

	action.ret = rtksw_l2_limitLearningCntAction_set(action.port, action.action);

	copy_to_user((struct rtl8367_cmd_l2_limitLearningCntAction_t *)arg, &action, sizeof(action));

	return 0;
}

KERNEL_IOCTL_FUNCTION(rtksw_l2_limitLearningCntAction_get, RTL8367_L2_LIMITLEARNINGCNTACTION_GET_CMD)
{
	rtl8367_cmd_l2_limitLearningCntAction_t action;

	copy_from_user(&action, (struct rtl8367_cmd_l2_limitLearningCntAction_t *)arg, sizeof(action));

	action.ret = rtksw_l2_limitLearningCntAction_get(action.port, &action.action);

	copy_to_user((struct rtl8367_cmd_l2_limitLearningCntAction_t *)arg, &action, sizeof(action));

	return 0;
}

KERNEL_IOCTL_FUNCTION(rtksw_l2_limitLearningCnt_set, RTL8367_L2_LIMITLEARNINGCNT_SET_CMD)
{
	rtl8367_cmd_l2_limitLearningCnt_t cnt;

	copy_from_user(&cnt, (struct rtl8367_cmd_l2_limitLearningCnt_t *)arg, sizeof(cnt));

	cnt.ret = rtksw_l2_limitLearningCnt_set(cnt.port, cnt.mac_cnt);

	copy_to_user((struct rtl8367_cmd_l2_limitLearningCnt_t *)arg, &cnt, sizeof(cnt));

	return 0;
}

KERNEL_IOCTL_FUNCTION(rtksw_l2_limitLearningCnt_get, RTL8367_L2_LIMITLEARNINGCNT_GET_CMD)
{
	rtl8367_cmd_l2_limitLearningCntAction_t cnt;

	copy_from_user(&cnt, (struct rtl8367_cmd_l2_limitLearningCnt_t *)arg, sizeof(cnt));

	cnt.ret = rtksw_l2_limitLearningCnt_get(cnt.port, &cnt.action);

	copy_to_user((struct rtl8367_cmd_l2_limitLearningCnt_t *)arg, &cnt, sizeof(cnt));

	return 0;
}


/* ioctl and cmd function */
static kernel_ioctl_cmd_def_t	g_rtl8367_ioctl_cmd_mapping[KERNEL_IOCTL_CMD_MAX_NUM];
static unsigned int		g_rtl8367_ioctl_cmd_num = 0;

void rtk_rtl8367_cmd_reg(kernel_ioctl_cmd_def_t* f_cmd)
{
	if (g_rtl8367_ioctl_cmd_num >= KERNEL_IOCTL_CMD_MAX_NUM) {
		RTL8367_ERROR("Reach maxmum commands number\n");
		return;
	}

	g_rtl8367_ioctl_cmd_mapping[g_rtl8367_ioctl_cmd_num].cmd = f_cmd->cmd;
	g_rtl8367_ioctl_cmd_mapping[g_rtl8367_ioctl_cmd_num].handler = f_cmd->handler;
	strncpy(g_rtl8367_ioctl_cmd_mapping[g_rtl8367_ioctl_cmd_num].name, f_cmd->name, KERNEL_IOCTL_CMD_NAME_MAX_LEN - 1);
	g_rtl8367_ioctl_cmd_num ++;
}

void rtk_rtl8367_cmd_init(void)
{
	unsigned int i = 0;

	for (i = 0 ; i < KERNEL_IOCTL_CMD_MAX_NUM ; i++) {
		g_rtl8367_ioctl_cmd_mapping[i].cmd = RTL8367_NONE_CMD;
		memset(g_rtl8367_ioctl_cmd_mapping[i].name, 0x00, sizeof(g_rtl8367_ioctl_cmd_mapping[i].name));
		g_rtl8367_ioctl_cmd_mapping[i].handler = NULL;
	}

	g_rtl8367_ioctl_cmd_num = 0;
}

#ifdef RTL8367_PHY_DEBUG
void  rtk_rtl8367_cmd_scan(void)
{
	unsigned int  i = 0;

	for (i = 0 ; i < g_rtl8367_ioctl_cmd_num ; i++) {
		RTL8367_MSG("cmd: %0x, name: %s, handler: %p",
		g_rtl8367_ioctl_cmd_mapping[i].cmd,
		g_rtl8367_ioctl_cmd_mapping[i].name,
		g_rtl8367_ioctl_cmd_mapping[i].handler);
	}
}
#endif

static long rtl8367_ioctl (struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0, i = 0;

	RTL8367_DEBUG("cmd: 0x%x (arg = 0x%x)", cmd, arg);

	for(i = 0 ; i < KERNEL_IOCTL_CMD_MAX_NUM; i++) {
		if(cmd == g_rtl8367_ioctl_cmd_mapping[i].cmd) {
			if(g_rtl8367_ioctl_cmd_mapping[i].handler != NULL) {
				g_rtl8367_ioctl_cmd_mapping[i].handler((void *)arg);
			}
		}
	}

	return ret;
}

static const struct file_operations rtl8367_fops = {
	.owner			= THIS_MODULE,
	//.write		= rtl8367_write,
	//.open			= rtl8367_open,
	//.release		= rtl8367_close,
	.llseek			= no_llseek,
	.unlocked_ioctl		= rtl8367_ioctl,
};

struct miscdevice rtl8367_device = {
 	.minor = MISC_DYNAMIC_MINOR,
	.name = RTL8367_IOCTL_NAME,
	.fops = &rtl8367_fops,
};

void rtk_rtl8367_ioctl_init(void)
{
	rtksw_api_ret_t ret;

	ret = misc_register(&rtl8367_device);
	if (ret)
		RTL8367_ERROR("Register a miscellaneous device ERROR.");
	else
		RTL8367_MSG("Register a miscellaneous device [\/dev\/%s]:", RTL8367_IOCTL_NAME);

	rtk_rtl8367_cmd_init();
	rtk_rtl8367_cmd_install();
#ifdef RTL8367_PHY_DEBUG
	rtk_rtl8367_cmd_scan();       /* Checking all registered function */
#endif
}

