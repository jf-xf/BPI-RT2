/*
 * Copyright (C) 2009-2015 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 71708 $
 * $Date: 2016-09-19 11:31:17 +0800 (Mon, 19 Sep 2016) $
 *
 * Purpose : Create stacking linux kernel module
 *
 */

/*
 * Include Files
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <fs/proc/internal.h>
#include <linux/uaccess.h>
#include <linux/delay.h>	/* mdelay() */
#include <linux/kthread.h>
#include <common/rt_error.h>
#if IS_BUILTIN(CONFIG_RTK_EXT_GPHY) || IS_MODULE(CONFIG_RTK_EXT_GPHY)
#include <rtk_ext_gphy.h>
#endif
#include <rtl8261/inc/phy_hal.h>
#include <rtl8261/inc/phy_init.h>
#include <soc/type.h>
#include <hal/phy/phydef.h>
#include <rtk/phy.h>
#include <example/src/test_phyApi.h>
#include <rtl82xx_3.6.8/sdk/include/common/rt_version.h>	/* Get correct RT_VERSION_SDK */

/*
 * Symbol Definition
 */
#define RTL8261_MSG(fmt,args...)	printk(KERN_INFO "rtl8261: "fmt"\n", ##args)
#define RTL8261_INFO(fmt,args...)	printk(KERN_INFO "\033[1;33;46m""rtl8261: "fmt"\033[m""\n", ##args)
#define RTL8261_ERROR(fmt,args...)	printk(KERN_INFO "\033[1;33;41m""rtl8261: "fmt"\033[m""\n", ##args)

//#define RTL8261_PHY_DEBUG
#ifdef RTL8261_PHY_DEBUG
#define RTL8261_DEBUG			RTL8261_INFO
#else
#define RTL8261_DEBUG(fmt,args...)
#endif

#define RTL8261_CA_XFI_PORT		(6)
#define RTL8261_CA_PON_PORT		(7)

#define RTL8261_MMD_1_3_VALUE		(0xCAF1)
#define RTL8261_CCUT_MMD_1_3_VALUE	(0xCAF2)

/*
 * Data Declaration
 */
#define RTL8261_DEFAULT_UNIT_ID		(0)


#define RTK_EXTGPHY_RTL8261_MAX	2
static int rtl8261_count = 0;
static int rtl8261_init = 0;

/*
 * This table is used to describe your hardware board design, especially for mapping relation between port and phy.
 * Port related information
 TK_EXTGPHY_RTL8261_MAXid.
 * .phy_idex    = used to indicate which PHY entry is used by this port in glued_phy_Descp[].
 * .eth         = Ethernet speed type (refer to rt_port_ethType_t).
 * .medi        = Port media type (refer to rt_port_medium_t).
 */
phy_hwp_portDescp_t  rtl8261_port_descp[] = {
        { .mac_id = HWP_END, .phy_idx = 0, .eth = HWP_XGE, .medi = HWP_COPPER, .smi = 0, .phy_addr = 0, .ca_port = 0,},
        { .mac_id = HWP_END, .phy_idx = 1, .eth = HWP_XGE, .medi = HWP_COPPER, .smi = 0, .phy_addr = 0, .ca_port = 0,},
        { .mac_id = HWP_END },
    };

/*
 * PHY related information
 * .chip        = PHY Chip model (refer to phy_type_t).
 * .mac_id      = The first port id of this PHY. For example, the 8218D is connected to
 *                port 0 ~ 7, then the .mac_id  = 0.
 * .phy_max     = The MAX port number of this PHY. For examplem the 8218D is an octet PHY,
 *                so this number is 8.
 */
phy_hwp_phyDescp_t     rtl8261_phy_Descp[] = {
        [0] = { .chip = HWP_END, .mac_id = 0, .phy_max = 1 },
        [1] = { .chip = HWP_END, .mac_id = 0, .phy_max = 1 },
        [2] = { .chip = HWP_END },
    };



#if defined(CONFIG_RTL8261_PORT6_PHY_ENABLE) && defined(CONFIG_RTL8261_PORT7_PHY_ENABLE)
/* One rtl8261 on port 6 and one rtl8261 on port 7 */
/*
 * This table is used to describe your hardware board design, especially for mapping relation between port and phy.
 * Port related information
 * .mac_id      = port id.
 * .phy_idex    = used to indicate which PHY entry is used by this port in glued_phy_Descp[].
 * .eth         = Ethernet speed type (refer to rt_port_ethType_t).
 * .medi        = Port media type (refer to rt_port_medium_t).
 */
phy_hwp_portDescp_t  my_port_descp[] = {
        { .mac_id = CONFIG_RTL8261_PORT6_PHY_ADDR, .phy_idx = 0, .eth = HWP_XGE, .medi = HWP_COPPER, .smi = 0, .phy_addr = CONFIG_RTL8261_PORT6_PHY_ADDR, .ca_port = RTL8261_CA_XFI_PORT,},
        { .mac_id = CONFIG_RTL8261_PORT7_PHY_ADDR, .phy_idx = 1, .eth = HWP_XGE, .medi = HWP_COPPER, .smi = 0, .phy_addr = CONFIG_RTL8261_PORT7_PHY_ADDR, .ca_port = RTL8261_CA_PON_PORT,},
        { .mac_id = HWP_END },
    };

/*
 * PHY related information
 * .chip        = PHY Chip model (refer to phy_type_t).
 * .mac_id      = The first port id of this PHY. For example, the 8218D is connected to
 *                port 0 ~ 7, then the .mac_id  = 0.
 * .phy_max     = The MAX port number of this PHY. For examplem the 8218D is an octet PHY,
 *                so this number is 8.
 */
phy_hwp_phyDescp_t     my_phy_Descp[] = {
        [0] = { .chip = RTK_PHYTYPE_RTL8261, .mac_id = CONFIG_RTL8261_PORT6_PHY_ADDR, .phy_max = 1 },
        [1] = { .chip = RTK_PHYTYPE_RTL8261, .mac_id = CONFIG_RTL8261_PORT7_PHY_ADDR, .phy_max = 1 },
        [2] = { .chip = HWP_END },
    };

#else

#if defined(CONFIG_RTL8261_PORT6_PHY_ENABLE) && !defined(CONFIG_RTL8261_PORT7_PHY_ENABLE)
/* Only one rtl8261 on port 6 */
phy_hwp_portDescp_t  my_port_descp[] = {
        { .mac_id = CONFIG_RTL8261_PORT6_PHY_ADDR, .phy_idx = 0, .eth = HWP_XGE, .medi = HWP_COPPER, .smi = 0, .phy_addr = CONFIG_RTL8261_PORT6_PHY_ADDR, .ca_port = RTL8261_CA_XFI_PORT,},
        { .mac_id = HWP_END },
    };
phy_hwp_phyDescp_t     my_phy_Descp[] = {
        [0] = { .chip = RTK_PHYTYPE_RTL8261, .mac_id = CONFIG_RTL8261_PORT6_PHY_ADDR, .phy_max = 1 },
        [1] = { .chip = HWP_END },
    };

#else

#if !defined(CONFIG_RTL8261_PORT6_PHY_ENABLE) && defined(CONFIG_RTL8261_PORT7_PHY_ENABLE)
/* Only one rtl8261 on port 7 */
phy_hwp_portDescp_t  my_port_descp[] = {
        { .mac_id = CONFIG_RTL8261_PORT7_PHY_ADDR, .phy_idx = 0, .eth = HWP_XGE, .medi = HWP_COPPER, .smi = 0, .phy_addr = CONFIG_RTL8261_PORT7_PHY_ADDR, .ca_port = RTL8261_CA_PON_PORT,},
        { .mac_id = HWP_END },
    };
phy_hwp_phyDescp_t     my_phy_Descp[] = {
        [0] = { .chip = RTK_PHYTYPE_RTL8261, .mac_id = CONFIG_RTL8261_PORT7_PHY_ADDR, .phy_max = 1 },
        [1] = { .chip = HWP_END },
    };
#endif

#endif

#endif

/*
 * Macro Definition
 */

/*
 * rtl8261 proc definition
 */
struct proc_dir_entry *rtl8261_proc_dir = NULL;

#define PROC_DIR_RTL8261		"rtl8261"
#define PROC_FILE_HELP			"help"
#define PROC_FILE_PHY			"phy"
#define PROC_FILE_LINKSTATUS		"link_status"
#define PROC_FILE_CTRL			"ctrl"
#define PROC_FILE_CNT			"cnt"
#define PROC_FILE_FORCEMODEABILITY	"force_speed"
#define PROC_FILE_SDSTESTMODE		"sds_test"
#define PROC_FILE_IEEETESTMODE		"ieee_test"
#define PROC_FILE_SDSEYEPARAM		"sds_eye_param"
#define PROC_FILE_PHYEYEMONITOR		"phy_eye_monitor"

#define INVALID_VALUE			99
#define MAX_COMMAND_LEN			32

#if 0
int _rtk_rtl8261_proc_pasring_string_to_integer(const char *buff,unsigned long len)
{
	char tmpbuf[MAX_COMMAND_LEN] = {0};

	if(len >= MAX_COMMAND_LEN)
		len = MAX_COMMAND_LEN;

	if (buff) {
		/* copy data to the buffer */
		strncpy(tmpbuf, buff, len);
		tmpbuf[len - 1] = '\0';
		return simple_strtol(tmpbuf, NULL, 0);
	}

	return INVALID_VALUE;
}
#endif

void rtl8261_usage(const char *filename)
{
	RTL8261_MSG("[Usage]");

	if((strcasecmp(filename, PROC_FILE_PHY) == 0) || (strcasecmp(filename, PROC_FILE_HELP) == 0)) {
		RTL8261_MSG("echo [$action] > /proc/%s/%s", PROC_DIR_RTL8261, PROC_FILE_PHY);
		RTL8261_MSG("\t$action: {init,reset}");
		//RTL8261_MSG("\t$phyad: rtl8261 phy address");
		if(strcasecmp(filename, PROC_FILE_PHY) == 0)		return;
	}

	if((strcasecmp(filename, PROC_FILE_LINKSTATUS) == 0) || (strcasecmp(filename, PROC_FILE_HELP) == 0)) {
		RTL8261_MSG("cat /proc/%s/%s", PROC_DIR_RTL8261, PROC_FILE_LINKSTATUS);
		if(strcasecmp(filename, PROC_FILE_LINKSTATUS) == 0)		return;
	}

	if((strcasecmp(filename, PROC_FILE_CTRL) == 0) || (strcasecmp(filename, PROC_FILE_HELP) == 0)) {
		RTL8261_MSG("cat /proc/%s/%s", PROC_DIR_RTL8261, PROC_FILE_CTRL);
		RTL8261_MSG("echo [$action] [option] > /proc/%s/%s", PROC_DIR_RTL8261, PROC_FILE_CTRL);
		RTL8261_MSG("\t$action: {serdes}");
		if(strcasecmp(filename, PROC_FILE_LINKSTATUS) == 0)		return;
	}

	return;
}

static int help_fops(struct seq_file *s, void *v)
{
	rtl8261_usage(PROC_FILE_HELP);
	return 0;
}

#if 0
static int Rtl8261_phy_fops(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int32 ret;
	char tmpbuf[MAX_COMMAND_LEN] = {0};
	int len = (count > MAX_COMMAND_LEN) ? (MAX_COMMAND_LEN - 1) : count;

	rtk_phy_initInfo_t initInfo;

	initInfo.port_desc = my_port_descp;
	initInfo.phy_desc = my_phy_Descp;

	RTL8261_DEBUG("%s - %s: %s (%d)", file->f_path.dentry->d_iname, __FUNCTION__, buffer, count);
	if (buffer) {
		char *strptr, *split_str;

		/* copy data to the buffer */
		strncpy(tmpbuf, buffer, len);
		tmpbuf[len] = '\0';

		strptr = tmpbuf;
		RTL8261_DEBUG("strptr: [%s]", strptr);

		/*parse command*/
		split_str = strsep(&strptr," ");
		RTL8261_DEBUG("split_str [%s]", split_str);

		if(strcasecmp(split_str, "init") == 0)
		{
			if ((ret = rtk_init(&initInfo)) == RT_ERR_OK)
				RTL8261_MSG("rtk_init OK");
			else
				RTL8261_ERROR("rtk_init Fail (0x%x) !!!", ret);

			if((ret = rtk_port_adminEnable_set(RTL8261_DEFAULT_UNIT_ID, CONFIG_RTL8261_PORT7_PHY_ADDR, ENABLED)) == RT_ERR_OK)
				RTL8261_MSG("rtk_port_adminEnable_set OK");
			else
				RTL8261_ERROR("rtk_port_adminEnable_set ERROR(0x%x) !!!", ret);
		}
		else if(strcasecmp(split_str, "reset") == 0)
		{
			if ((ret = rtk_port_phyReset_set(RTL8261_DEFAULT_UNIT_ID, CONFIG_RTL8261_PORT7_PHY_ADDR)) == RT_ERR_OK)
				RTL8261_MSG("rtk_port_phyReset_set OK");
			else
				RTL8261_ERROR("rtk_port_phyReset_set Fail (0x%x) !!!", ret);
		}else
		{
			goto phy_error;
		}
	}
	return count;

phy_error:
	RTL8261_ERROR("%s() FAIL ......", __FUNCTION__);
	rtl8261_usage(file->f_path.dentry->d_iname);
	return -EFAULT;
}
#endif

static int Rtl8261_link_status_fops(struct seq_file *s, void *v)
{
	int32 ret, i, j, sds_num;
	rtk_port_linkStatus_t status;
	rtk_port_media_t media;
	rtk_port_speed_t  speed;
	rtk_port_duplex_t duplex;
	rtk_phy_macIntfSdsLinkStatus_t sstatus;

	const char *status_info[] = {
		"LINKDOWN",
		"LINKUP",
		NULL
	};
	const char *media_info[] = {
		"MEDIA_COPPER",
		"MEDIA_FIBER",
		NULL
	};
	const char *speed_info[] = {
		"SPEED_10M",
		"SPEED_100M",
		"SPEED_1000M",
		"SPEED_500M",
		"SPEED_2G",   /* Applicable to 8380 */
		"SPEED_2_5G",
		"SPEED_5G",
		"SPEED_10G",
		"SPEED_2_5G_LITE",
		"SPEED_5G_LITE",
		"SPEED_10G_LITE",
		NULL
	};
	const char *duplex_info[] = {
		"HALF_DUPLEX",
		"FULL_DUPLEX",
		NULL
	};

	for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
		RTL8261_MSG("================ [PORT %d] PHY_ADDR %d ================", my_port_descp[i].ca_port, my_port_descp[i].phy_addr);

		/* WORKAROUND: Read twice for real link status ??? */
		rtk_port_phyLinkStatus_get(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, &status);
		if((ret = rtk_port_phyLinkStatus_get(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, &status)) == RT_ERR_OK)
			RTL8261_MSG("rtk_port_phyLinkStatus_get(%d, %d) : %d[%s]", RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, status, status_info[status]);
		else
			RTL8261_ERROR("rtk_port_phyLinkStatus_get ERROR(0x%x) !!!", ret);

		if (status == PORT_LINKUP) {
			if ((ret = rtk_port_linkMedia_get(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, &status, &media)) == RT_ERR_OK)
				RTL8261_MSG("rtk_port_linkMedia_get(%d, %d) : %d[%s] - %d[%s]", RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, status, status_info[status], media, media_info[media]);
			else
				RTL8261_ERROR("rtk_port_linkMedia_get ERROR(0x%x) !!!", ret);

			if ((ret = rtk_port_speedDuplex_get(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, &speed, &duplex)) == RT_ERR_OK)
				RTL8261_MSG("rtk_port_speedDuplex_get(%d, %d) : %d[%s] - %d[%s]", RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, speed, speed_info[speed], duplex, duplex_info[duplex]);
			else
				RTL8261_ERROR("rtk_port_speedDuplex_get ERROR(0x%x) !!!", ret);
		}

		if((ret = rtk_port_phyMacIntfSerdesLinkStatus_get(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, &sstatus)) == RT_ERR_OK) {
			RTL8261_MSG("rtk_port_phyMacIntfSerdesLinkStatus_get(%d, %d) : SerDes Num %d", RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, sstatus.sds_num);
			if (sstatus.sds_num > RTK_PHY_MAC_INTF_SDS_MAX)
				sds_num = RTK_PHY_MAC_INTF_SDS_MAX;
			else
				sds_num = sstatus.sds_num;
			for (j = 0 ; j < sds_num ; j++)
				RTL8261_MSG("\t\t\tserdesID %d [%s]", sstatus.link_status[j], status_info[sstatus.link_status[j]]);
		}
		else
			RTL8261_ERROR("rtk_port_phyMacIntfSerdesLinkStatus_get ERROR(0x%x) !!!", ret);
	}

	return 0;
}

static int Rtl8261_ctrl_get_fops(struct seq_file *s, void *v)
{
	int32 ret, i;
	uint32 value;
	char sDesc[64];

	for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
		RTL8261_MSG("================ [PORT %d] PHY_ADDR %d ================", my_port_descp[i].ca_port, my_port_descp[i].phy_addr);
		RTL8261_INFO("[RTK_PHY_CTRL_SERDES_MODE]");
		if ((ret = rtk_port_phyCtrl_get(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, RTK_PHY_CTRL_SERDES_MODE, &value)) == RT_ERR_OK) {
			switch (value) {
			case RTK_PHY_CTRL_SERDES_MODE_USXGMII:						/* 0x0000: USXGMII */
				strncpy(sDesc, "USXGMII\0", 8);
				break;
			case RTK_PHY_CTRL_SERDES_MODE_10GR_5GX_2P5GX_SGMII:				/* 0x1001: 10G-R/5G-X/2.5G-X/SGMII */
				strncpy(sDesc, "10G-R/5G-X/2.5G-X/SGMII\0", 24);
				break;
			case RTK_PHY_CTRL_SERDES_MODE_10GR_XFI5GADAPT_XFI2P5GADAPT_SGMII:		/* 0x1002: 10G-R/XFI-5G-ADAPT/XFI-2.5G-ADAPT/SGMII */
				strncpy(sDesc, "10G-R/XFI-5G-ADAPT/XFI-2.5G-ADAPT/SGMII\0", 40);
				break;
			case RTK_PHY_CTRL_SERDES_MODE_10GR_5GR_2P5GX_SGMII:				/* 0x1101: 10G-R/5G-R/2.5G-X/SGMII */
				strncpy(sDesc, "10G-R/5G-R/2.5G-X/SGMII\0", 24);
				break;
			case RTK_PHY_CTRL_SERDES_MODE_10GR_XFI5GCPRI_2P5GX_SGMII:			/* 0x1102: 10G-R/XFI-5G-CPRI/2.5G-X/SGMII */
				strncpy(sDesc, "10G-R/XFI-5G-CPRI/2.5G-X/SGMII\0", 31);
				break;
			case RTK_PHY_CTRL_SERDES_MODE_UNKNOWN:						/* 0xFFFF */
			default:
				strncpy(sDesc, "UNKNOWN\0", 4);
				break;
			}

			RTL8261_MSG("rtk_port_phyCtrl_get(%d, %d) : ctrl_type %d - 0x%x[%s][%s]", RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, RTK_PHY_CTRL_SERDES_MODE, value, sDesc, 
				RTK_PHY_CTRL_SERDES_MODE_IS_AUTO(value) ? "FIX" : "AUTO");
		}
		else {
			RTL8261_ERROR("rtk_port_phyCtrl_get ERROR(0x%x) !!!", ret);
		}

		/* RTK_PHY_CTRL_SERDES_UPDTAE */
#if 0
		RTL8261_INFO("[RTK_PHY_CTRL_SERDES_UPDTAE]");
		if ((ret = rtk_port_phyCtrl_get(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, RTK_PHY_CTRL_SERDES_UPDTAE, &value)) == RT_ERR_OK)
			RTL8261_MSG("rtk_port_phyCtrl_get OK: value = 0x%X");
		else
			RTL8261_ERROR("rtk_port_phyCtrl_get (0x%x) !!!", ret);
#endif
		/* RTK_PHY_CTRL_SERDES_RX_POLARITY */
		RTL8261_INFO("[RTK_PHY_CTRL_SERDES_RX_POLARITY]");
		if ((ret = rtk_port_phyCtrl_get(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, RTK_PHY_CTRL_SERDES_RX_POLARITY, &value)) == RT_ERR_OK)
			RTL8261_MSG("rtk_port_phyCtrl_get OK: value = 0x%X");
		else
			RTL8261_ERROR("rtk_port_phyCtrl_get (0x%x) !!!", ret);

		/* RTK_PHY_CTRL_SERDES_TX_POLARITY */
		RTL8261_INFO("[RTK_PHY_CTRL_SERDES_TX_POLARITY]");
		if ((ret = rtk_port_phyCtrl_get(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, RTK_PHY_CTRL_SERDES_TX_POLARITY, &value)) == RT_ERR_OK)
			RTL8261_MSG("rtk_port_phyCtrl_get OK: value = 0x%X");
		else
			RTL8261_ERROR("rtk_port_phyCtrl_get (0x%x) !!!", ret);

		/* RTK_PHY_CTRL_LOOPBACK_REMOTE */
		RTL8261_INFO("[RTK_PHY_CTRL_LOOPBACK_REMOTE]");
		if ((ret = rtk_port_phyCtrl_get(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, RTK_PHY_CTRL_LOOPBACK_REMOTE, &value)) == RT_ERR_OK)
			RTL8261_MSG("rtk_port_phyCtrl_get OK: value = 0x%X");
		else
			RTL8261_ERROR("rtk_port_phyCtrl_get (0x%x) !!!", ret);

		/* RTK_PHY_CTRL_LOOPBACK_INTERNAL_PMA */
		RTL8261_INFO("[RTK_PHY_CTRL_LOOPBACK_INTERNAL_PMA]");
		if ((ret = rtk_port_phyCtrl_get(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, RTK_PHY_CTRL_LOOPBACK_INTERNAL_PMA, &value)) == RT_ERR_OK)
			RTL8261_MSG("rtk_port_phyCtrl_get OK: value = 0x%X");
		else
			RTL8261_ERROR("rtk_port_phyCtrl_get (0x%x) !!!", ret);

		/* RTK_PHY_CTRL_MDI_POLARITY_SWAP */
		RTL8261_INFO("[RTK_PHY_CTRL_MDI_POLARITY_SWAP]");
		if ((ret = rtk_port_phyCtrl_get(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, RTK_PHY_CTRL_MDI_POLARITY_SWAP, &value)) == RT_ERR_OK)
			RTL8261_MSG("rtk_port_phyCtrl_get OK: value = 0x%X");
		else
			RTL8261_ERROR("rtk_port_phyCtrl_get (0x%x) !!!", ret);

		/* RTK_PHY_CTRL_MDI_INVERSE */
		RTL8261_INFO("[RTK_PHY_CTRL_MDI_INVERSE]");
		if ((ret = rtk_port_phyCtrl_get(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, RTK_PHY_CTRL_MDI_INVERSE, &value)) == RT_ERR_OK)
			RTL8261_MSG("rtk_port_phyCtrl_get OK: value = 0x%X");
		else
			RTL8261_ERROR("rtk_port_phyCtrl_get (0x%x) !!!", ret);
	}

	return 0;
}

#define  RTK_PHY_CTRL_SERDES_RX_POLARITY_OPT		"srp"
#define  RTK_PHY_CTRL_SERDES_TX_POLARITY_OPT		"stp"
#define  RTK_PHY_CTRL_LOOPBACK_REMOTE_OPT		"lr"
#define  RTK_PHY_CTRL_LOOPBACK_INTERNAL_PMA_OPT		"li"
#define  RTK_PHY_CTRL_MDI_POLARITY_SWAP_OPT		"mps"
#define  RTK_PHY_CTRL_MDI_INVERSE_OPT			"mi"
#define  RTK_PHY_CTRL_SERDES_MODE_OPT			"sm"
#define  RTK_PHY_CTRL_SERDES_UPDTAE_OPT			"su"

#define  RTK_PHY_CTRL_VALUE_DONT_CARE			(0x0)

static int Rtl8261_ctrl_set_fops(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int32 ret;
	char tmpbuf[MAX_COMMAND_LEN] = {0};
	int len = (count > MAX_COMMAND_LEN) ? (MAX_COMMAND_LEN -1) : count;

	uint32 port_num = 0, phy_addr = 0, value;
	uint32 serdes_mode;	//= _rtk_rtl8261_proc_pasring_string_to_integer(buffer, count);
	const char *serdes_mode_info[] = {
		"USXGMII",
		"10G-R/5G-X/2.5G-X/SGMII",
		"10G-R/XFI-5G-ADAPT/XFI-2.5G-ADAPT/SGMII",
		"10G-R/5G-R/2.5G-X/SGMII",
		"10G-R/XFI-5G-CPRI/2.5G-X/SGMII",
		NULL
	};

	RTL8261_DEBUG("%s - %s: %s (%d)", file->f_path.dentry->d_iname, __FUNCTION__, buffer, count);
	if (buffer) {
		char *strptr, *split_str, *ctrl_type;

		/* copy data to the buffer */
		strncpy(tmpbuf, buffer, len);
		tmpbuf[len] = '\0';

		strptr = tmpbuf;
		RTL8261_MSG("Input: [%s]", strptr);

		/*parse command*/
		ctrl_type = strsep(&strptr," ");
		split_str = strsep(&strptr," ");
		port_num = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		value = simple_strtoul(split_str, NULL, 0);
		RTL8261_MSG("Parsing result: [%s][port %d][value %d]", ctrl_type, port_num, value);

#ifdef CONFIG_RTL8261_PORT6_PHY_ENABLE
		if (port_num == 6)	phy_addr = CONFIG_RTL8261_PORT6_PHY_ADDR;
#endif
#ifdef CONFIG_RTL8261_PORT7_PHY_ENABLE
		if (port_num == 7)	phy_addr = CONFIG_RTL8261_PORT7_PHY_ADDR;
#endif
		if (phy_addr == 0) {
			RTL8261_ERROR("UNKNOWN port num %d", port_num);
			return count;
		}

		RTL8261_MSG("================ [PORT %d] PHY_ADDR %d ================", port_num, phy_addr);
		if((strcasecmp(ctrl_type, RTK_PHY_CTRL_SERDES_MODE_OPT) == 0) || (strcasecmp(ctrl_type, "serdes") == 0)) {
			RTL8261_INFO("[RTK_PHY_CTRL_SERDES_MODE]");

			switch (value) {
			case 0:
				serdes_mode = RTK_PHY_CTRL_SERDES_MODE_USXGMII;				/* 0x0000: USXGMII */
				break;
			case 1:
				serdes_mode = RTK_PHY_CTRL_SERDES_MODE_10GR_5GX_2P5GX_SGMII;			/* 0x1001: 10G-R/5G-X/2.5G-X/SGMII */
				break;
			case 2:
				serdes_mode = RTK_PHY_CTRL_SERDES_MODE_10GR_XFI5GADAPT_XFI2P5GADAPT_SGMII;	/* 0x1002: 10G-R/XFI-5G-ADAPT/XFI-2.5G-ADAPT/SGMII */
				break;
			case 3:
				serdes_mode = RTK_PHY_CTRL_SERDES_MODE_10GR_5GR_2P5GX_SGMII;			/* 0x1101: 10G-R/5G-R/2.5G-X/SGMII */
				break;
			case 4:
				serdes_mode = RTK_PHY_CTRL_SERDES_MODE_10GR_XFI5GCPRI_2P5GX_SGMII;		/* 0x1102: 10G-R/XFI-5G-CPRI/2.5G-X/SGMII */
				break;
			default:
				RTL8261_ERROR("Unknown serdes mode %d, set to default serdes mode as 0", serdes_mode);
				serdes_mode = RTK_PHY_CTRL_SERDES_MODE_USXGMII;				/* 0x0000: USXGMII */
				serdes_mode = 0;
				break;
			}

			if ((ret = rtk_port_phyCtrl_set(RTL8261_DEFAULT_UNIT_ID, phy_addr, RTK_PHY_CTRL_SERDES_MODE, serdes_mode)) == RT_ERR_OK)
				RTL8261_MSG("rtk_port_phyCtrl_set OK: 0x%x[%s][%s]", serdes_mode, serdes_mode_info[value], RTK_PHY_CTRL_SERDES_MODE_IS_AUTO(serdes_mode) ? "FIX" : "AUTO");
			else
				RTL8261_ERROR("rtk_port_phyCtrl_set (0x%x) !!!", ret);
		}
		/* RTK_PHY_CTRL_SERDES_UPDTAE */
		else if(strcasecmp(ctrl_type, RTK_PHY_CTRL_SERDES_UPDTAE_OPT) == 0) {
			RTL8261_INFO("[RTK_PHY_CTRL_SERDES_UPDTAE]");
			if ((ret = rtk_port_phyCtrl_set(RTL8261_DEFAULT_UNIT_ID, phy_addr, RTK_PHY_CTRL_SERDES_UPDTAE, RTK_PHY_CTRL_VALUE_DONT_CARE)) == RT_ERR_OK)
				RTL8261_MSG("rtk_port_phyCtrl_set OK");
			else
				RTL8261_ERROR("rtk_port_phyCtrl_set (0x%x) !!!", ret);
		}
		/* RTK_PHY_CTRL_SERDES_RX_POLARITY */
		else if(strcasecmp(ctrl_type, RTK_PHY_CTRL_SERDES_RX_POLARITY_OPT) == 0) {
			RTL8261_INFO("[RTK_PHY_CTRL_SERDES_RX_POLARITY]");
			if ((ret = rtk_port_phyCtrl_set(RTL8261_DEFAULT_UNIT_ID, phy_addr, RTK_PHY_CTRL_SERDES_RX_POLARITY, value)) == RT_ERR_OK)
				RTL8261_MSG("rtk_port_phyCtrl_set OK");
			else
				RTL8261_ERROR("rtk_port_phyCtrl_set (0x%x) !!!", ret);
		}
		/* RTK_PHY_CTRL_SERDES_TX_POLARITY */
		else if(strcasecmp(ctrl_type, RTK_PHY_CTRL_SERDES_TX_POLARITY_OPT) == 0) {
			RTL8261_INFO("[RTK_PHY_CTRL_SERDES_TX_POLARITY]");
			if ((ret = rtk_port_phyCtrl_set(RTL8261_DEFAULT_UNIT_ID, phy_addr, RTK_PHY_CTRL_SERDES_TX_POLARITY, value)) == RT_ERR_OK)
				RTL8261_MSG("rtk_port_phyCtrl_set OK");
			else
				RTL8261_ERROR("rtk_port_phyCtrl_set (0x%x) !!!", ret);
		}
		/* RTK_PHY_CTRL_LOOPBACK_REMOTE */
		else if(strcasecmp(ctrl_type, RTK_PHY_CTRL_LOOPBACK_REMOTE_OPT) == 0) {
			RTL8261_INFO("[RTK_PHY_CTRL_LOOPBACK_REMOTE]");
			if ((ret = rtk_port_phyCtrl_set(RTL8261_DEFAULT_UNIT_ID, phy_addr, RTK_PHY_CTRL_LOOPBACK_REMOTE, value)) == RT_ERR_OK)
				RTL8261_MSG("rtk_port_phyCtrl_set OK");
			else
				RTL8261_ERROR("rtk_port_phyCtrl_set (0x%x) !!!", ret);
		}
		/* RTK_PHY_CTRL_LOOPBACK_INTERNAL_PMA */
		else if(strcasecmp(ctrl_type, RTK_PHY_CTRL_LOOPBACK_INTERNAL_PMA_OPT) == 0) {
			RTL8261_INFO("[RTK_PHY_CTRL_LOOPBACK_INTERNAL_PMA]");
			if ((ret = rtk_port_phyCtrl_set(RTL8261_DEFAULT_UNIT_ID, phy_addr, RTK_PHY_CTRL_LOOPBACK_INTERNAL_PMA, value)) == RT_ERR_OK)
				RTL8261_MSG("rtk_port_phyCtrl_set OK");
			else
				RTL8261_ERROR("rtk_port_phyCtrl_set (0x%x) !!!", ret);
		}
		/* RTK_PHY_CTRL_MDI_POLARITY_SWAP */
		else if(strcasecmp(ctrl_type, RTK_PHY_CTRL_MDI_POLARITY_SWAP_OPT) == 0) {
			RTL8261_INFO("[RTK_PHY_CTRL_MDI_POLARITY_SWAP]");
			if ((ret = rtk_port_phyCtrl_set(RTL8261_DEFAULT_UNIT_ID, phy_addr, RTK_PHY_CTRL_MDI_POLARITY_SWAP, value)) == RT_ERR_OK)
				RTL8261_MSG("rtk_port_phyCtrl_set OK");
			else
				RTL8261_ERROR("rtk_port_phyCtrl_set (0x%x) !!!", ret);
		}
		/* RTK_PHY_CTRL_MDI_INVERSE */
		else if(strcasecmp(ctrl_type, RTK_PHY_CTRL_MDI_INVERSE_OPT) == 0) {
			RTL8261_INFO("[RTK_PHY_CTRL_MDI_INVERSE]");
			if ((ret = rtk_port_phyCtrl_set(RTL8261_DEFAULT_UNIT_ID, phy_addr, RTK_PHY_CTRL_MDI_INVERSE, value)) == RT_ERR_OK)
				RTL8261_MSG("rtk_port_phyCtrl_set OK");
			else
				RTL8261_ERROR("rtk_port_phyCtrl_set (0x%x) !!!", ret);
		}
		else if(strcasecmp(ctrl_type, "ugan") == 0) {

		}
		else if(strcasecmp(ctrl_type, "sgan") == 0) {

		}
		else if(strcasecmp(ctrl_type, "ugan") == 0) {

		}
		else {
			RTL8261_ERROR("Unknown ctrl command [%s] !!!", ctrl_type);
		}
	}

	return count;
}

uint64 old_p6_cnt_rx = 0;
uint64 old_p6_cnt_rx_err = 0;
uint64 old_p6_cnt_rx_crcerr = 0;
uint64 old_p6_cnt_ldpc_err = 0;

uint64 old_p7_cnt_rx = 0;
uint64 old_p7_cnt_rx_err = 0;
uint64 old_p7_cnt_rx_crcerr = 0;
uint64 old_p7_cnt_ldpc_err = 0;

static int Rtl8261_cnt_get_fops(struct seq_file *s, void *v)
{
	int32 ret, i;
	uint64 cnt;
	//uint32 data0, data1, data2, data3;

	for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
		RTL8261_MSG("================ [PORT %d] PHY_ADDR %d ================", my_port_descp[i].ca_port, my_port_descp[i].phy_addr);

		if ((ret = rtk_port_phyDbgCounter_get(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, PHY_DBG_CNT_RX, &cnt)) == RT_ERR_OK) {
			if (my_port_descp[i].ca_port == RTL8261_CA_XFI_PORT) {
				RTL8261_MSG("PHY_DBG_CNT_RX :\t0x%016llx\t[0x%016llx]", (cnt - old_p6_cnt_rx), cnt);
				old_p6_cnt_rx = cnt;
			}
			else if (my_port_descp[i].ca_port == RTL8261_CA_PON_PORT) {
				RTL8261_MSG("PHY_DBG_CNT_RX :\t0x%016llx\t[0x%016llx]", (cnt - old_p7_cnt_rx), cnt);
				old_p7_cnt_rx = cnt;
			}
		} else
			RTL8261_ERROR("rtk_port_phyCtrl_get ERROR: PHY_DBG_CNT_RX (0x%x) !!!", ret);

		if ((ret = rtk_port_phyDbgCounter_get(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, PHY_DBG_CNT_RX_ERR, &cnt)) == RT_ERR_OK) {
			if (my_port_descp[i].ca_port == RTL8261_CA_XFI_PORT) {
				RTL8261_MSG("PHY_DBG_CNT_RX_ERR :\t0x%016llx\t[0x%016llx]", (cnt - old_p6_cnt_rx_err), cnt);
				old_p6_cnt_rx_err = cnt;
			}
			else if (my_port_descp[i].ca_port == RTL8261_CA_PON_PORT) {
				RTL8261_MSG("PHY_DBG_CNT_RX_ERR :\t0x%016llx\t[0x%016llx]", (cnt - old_p7_cnt_rx_err), cnt);
				old_p7_cnt_rx_err = cnt;
			}
		} else
			RTL8261_ERROR("rtk_port_phyCtrl_get ERROR: PHY_DBG_CNT_RX_ERR (0x%x) !!!", ret);

		if ((ret = rtk_port_phyDbgCounter_get(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, PHY_DBG_CNT_RX_CRCERR, &cnt)) == RT_ERR_OK) {
			if (my_port_descp[i].ca_port == RTL8261_CA_XFI_PORT) {
				RTL8261_MSG("PHY_DBG_CNT_RX_CRCERR : 0x%016llx\t[0x%016llx]", (cnt - old_p6_cnt_rx_crcerr), cnt);
				old_p6_cnt_rx_crcerr = cnt;
			}
			else if (my_port_descp[i].ca_port == RTL8261_CA_PON_PORT) {
				RTL8261_MSG("PHY_DBG_CNT_RX_CRCERR : 0x%016llx\t[0x%016llx]", (cnt - old_p7_cnt_rx_crcerr), cnt);
				old_p7_cnt_rx_crcerr = cnt;
			}
		} else
			RTL8261_ERROR("rtk_port_phyCtrl_get ERROR: PHY_DBG_CNT_RX_CRCERR (0x%x) !!!", ret);

		if ((ret = rtk_port_phyDbgCounter_get(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, PHY_DBG_CNT_LDPC_ERR, &cnt)) == RT_ERR_OK) {
			if (my_port_descp[i].ca_port == RTL8261_CA_XFI_PORT) {
				RTL8261_MSG("PHY_DBG_CNT_LDPC_ERR :\t0x%016llx\t[0x%016llx]", (cnt - old_p6_cnt_ldpc_err),cnt);
				old_p6_cnt_ldpc_err = cnt;
			}
			else if (my_port_descp[i].ca_port == RTL8261_CA_PON_PORT) {
				RTL8261_MSG("PHY_DBG_CNT_LDPC_ERR :\t0x%016llx\t[0x%016llx]", (cnt - old_p7_cnt_ldpc_err),cnt);
				old_p7_cnt_ldpc_err = cnt;
			}
		} else
			RTL8261_ERROR("rtk_port_phyCtrl_get ERROR: PHY_DBG_CNT_LDPC_ERR (0x%x) !!!", ret);
#if 0
		/* enable */
		phy_hal_mmd_write(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x210, 0x2);
		phy_hal_mmd_write(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x230, 0x2);
		phy_hal_mmd_write(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x250, 0x2);

		/* clear */
		phy_hal_mmd_write(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x210, 0x3C2);
		phy_hal_mmd_write(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x230, 0x3C2);
		phy_hal_mmd_write(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x250, 0x3C2);

		/* read */
		/* RX0 */
		phy_hal_mmd_read(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x247, &data0);
		phy_hal_mmd_read(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x246, &data1);
		phy_hal_mmd_read(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x245, &data2);

		RTL8261_MSG("RX0 : 30.0x247 = %08x; 30.0x246 = %08x, 30.0x245 = %08x", data0, data1 , data2);

		/* RX1 */
		phy_hal_mmd_read(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x267, &data0);
		phy_hal_mmd_read(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x266, &data1);
		phy_hal_mmd_read(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x265, &data2);

		RTL8261_MSG("RX1 : 30.0x267 = %08x; 30.0x266 = %08x, 30.0x265 = %08x", data0, data1 , data2);

		/* TX0 */
		phy_hal_mmd_read(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x227, &data0);
		phy_hal_mmd_read(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x226, &data1);
		phy_hal_mmd_read(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x225, &data2);

		RTL8261_MSG("TX0 : 30.0x227 = %08x; 30.0x226 = %08x, 30.0x225 = %08x", data0, data1 , data2);

		phy_hal_mmd_read(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 31, 0xb202, &data0);
		phy_hal_mmd_read(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 31, 0xb282, &data1);
		phy_hal_mmd_read(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 31, 0xb302, &data2);
		phy_hal_mmd_read(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 31, 0xb382, &data3);

		RTL8261_MSG("TX0 : 31.0xb202 = %08x; 31.0xb282 = %08x, 31.0xb302 = %08x, 31.0xb382 = %08x", data0, data1 , data2, data3);
#endif
	}

	return 0;
}

static int Rtl8261_cnt_set_fops(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	RTL8261_INFO("%s(%d): TODO", __FUNCTION__, __LINE__);

	return count;
}

#define RTK_PHY_PORT_SPEED_100M		"100M"
#define RTK_PHY_PORT_SPEED_1000M	"1G"
#define RTK_PHY_PORT_SPEED_2_5G		"2.5G"
#define RTK_PHY_PORT_SPEED_5G		"5G"
#define RTK_PHY_PORT_SPEED_10G		"10G"

static int Rtl8261_ForceModeAbility_get_fops(struct seq_file *s, void *v)
{
	RTL8261_INFO("%s(%d): TODO", __FUNCTION__, __LINE__);

	return 0;
}

static int Rtl8261_ForceModeAbility_set_fops (struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int32 ret;
	char tmpbuf[MAX_COMMAND_LEN] = {0};
	int len = (count > MAX_COMMAND_LEN) ? (MAX_COMMAND_LEN -1) : count;
	uint32 port_num = 0, phy_addr = 0;
	rtk_port_speed_t    speed;

	RTL8261_DEBUG("%s - %s: %s (%d)", file->f_path.dentry->d_iname, __FUNCTION__, buffer, count);
	if (buffer) {
		char *strptr, *split_str, *port_speed;

		/* copy data to the buffer */
		strncpy(tmpbuf, buffer, len);
		tmpbuf[len] = '\0';

		strptr = tmpbuf;
		RTL8261_MSG("Input: [%s]", strptr);

		/*parse command*/
		split_str = strsep(&strptr," ");
		port_num = simple_strtoul(split_str, NULL, 0);
		port_speed = strsep(&strptr," ");
		RTL8261_MSG("Parsing result: [port %d][speed %s]", port_num, port_speed);

#ifdef CONFIG_RTL8261_PORT6_PHY_ENABLE
		if (port_num == 6)	phy_addr = CONFIG_RTL8261_PORT6_PHY_ADDR;
#endif
#ifdef CONFIG_RTL8261_PORT7_PHY_ENABLE
		if (port_num == 7)	phy_addr = CONFIG_RTL8261_PORT7_PHY_ADDR;
#endif
		if (phy_addr == 0) {
			RTL8261_ERROR("UNKNOWN port num %d", port_num);
			return count;
		}

		RTL8261_MSG("================ [PORT %d] PHY_ADDR %d ================", port_num, phy_addr);
		if(strcasecmp(port_speed, RTK_PHY_PORT_SPEED_100M) == 0)		speed = PORT_SPEED_100M;
		else if(strcasecmp(port_speed, RTK_PHY_PORT_SPEED_1000M) == 0)		speed = PORT_SPEED_1000M;
		else if(strcasecmp(port_speed, RTK_PHY_PORT_SPEED_2_5G) == 0)		speed = PORT_SPEED_2_5G;
		else if(strcasecmp(port_speed, RTK_PHY_PORT_SPEED_5G) == 0)		speed = PORT_SPEED_5G;
		else if(strcasecmp(port_speed, RTK_PHY_PORT_SPEED_10G) == 0)		speed = PORT_SPEED_10G;
		else {
			RTL8261_ERROR("UNKNOWN port speed %s", port_speed);
			return count;
		}

		if((ret = rtk_port_phyForceModeAbility_set(RTL8261_DEFAULT_UNIT_ID, phy_addr, speed, PORT_FULL_DUPLEX, ENABLED)) == RT_ERR_OK)
			RTL8261_MSG("rtk_port_phyForceModeAbility_set OK");
		else
			RTL8261_ERROR("rtk_port_phyForceModeAbility_set(0x%x) !!!", ret);
	}
	return count;
}

static int Rtl8261_sdstestmode_get_fops(struct seq_file *s, void *v)
{
	RTL8261_INFO("%s(%d): TODO", __FUNCTION__, __LINE__);

	return 0;
}

static int Rtl8261_sdstestmode_set_fops(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int32 ret;
	char tmpbuf[MAX_COMMAND_LEN] = {0};
	int len = (count > MAX_COMMAND_LEN) ? (MAX_COMMAND_LEN - 1) : count;
	uint32 sdsId, testMode;
	uint32 port_num = 0, phy_addr = 0;

	const char *serdes_test_mode_info[] = {
		"DISABLE",
		"PRBS7",
		"PRBS9",
		"PRBS11",
		"PRBS15",
		"PRBS23",
		"PRBS31",
		"SQUARE8",
		NULL
	};

	RTL8261_DEBUG("%s - %s: %s (%d)", file->f_path.dentry->d_iname, __FUNCTION__, buffer, count);
	if (buffer) {
		char *strptr, *split_str;

		/* copy data to the buffer */
		strncpy(tmpbuf, buffer, len);
		tmpbuf[len] = '\0';

		strptr = tmpbuf;
		RTL8261_DEBUG("strptr: [%s]", strptr);

		/*parse command*/
		split_str = strsep(&strptr," ");
		port_num = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		sdsId = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		testMode = simple_strtoul(split_str, NULL, 0);
		RTL8261_MSG("Parsing result: [port %d][sdsId %d][testMode %d]", port_num, sdsId, testMode);
		RTL8261_MSG("[Serdes ID]: Should be 0");
		RTL8261_MSG("[Serdes Tese Mode]:\n\t0: %s\n\t1: %s\n\t2: %s\n\t3: %s\n\t4: %s\n\t5: %s\n\t6: %s\n\t7: %s",
			serdes_test_mode_info[0], serdes_test_mode_info[1], serdes_test_mode_info[2], serdes_test_mode_info[3],
			serdes_test_mode_info[4], serdes_test_mode_info[5], serdes_test_mode_info[6], serdes_test_mode_info[7]);

#ifdef CONFIG_RTL8261_PORT6_PHY_ENABLE
		if (port_num == 6)	phy_addr = CONFIG_RTL8261_PORT6_PHY_ADDR;
#endif
#ifdef CONFIG_RTL8261_PORT7_PHY_ENABLE
		if (port_num == 7)	phy_addr = CONFIG_RTL8261_PORT7_PHY_ADDR;
#endif
		if (phy_addr == 0) {
			RTL8261_ERROR("UNKNOWN port num %d", port_num);
			return count;
		}

		RTL8261_MSG("================ [PORT %d] PHY_ADDR %d ================", port_num, phy_addr);

		if ((ret = rtk_port_phySdsTestMode_set(RTL8261_DEFAULT_UNIT_ID, phy_addr, sdsId, testMode)) == RT_ERR_OK) {
			RTL8261_MSG("rtk_port_phySdsTestMode_set(%d, %d, %d, %d) [Seredes Mode: %s] OK",
				RTL8261_DEFAULT_UNIT_ID, phy_addr, sdsId, testMode, serdes_test_mode_info[testMode]);
		}
		else
			RTL8261_ERROR("rtk_port_phySdsTestMode_set ERROR (ret = 0x%x) !!!", ret);
	}

	return count;
}

static int Rtl8261_ieeetestmode_get_fops(struct seq_file *s, void *v)
{
	RTL8261_INFO("%s(%d): TODO", __FUNCTION__, __LINE__);

	return 0;
}

static int Rtl8261_ieeetestmode_set_fops(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int32 ret;
	char tmpbuf[MAX_COMMAND_LEN] = {0};
	int len = (count > MAX_COMMAND_LEN) ? (MAX_COMMAND_LEN - 1) : count;
	uint32 testMode;
	uint32 port_num = 0, phy_addr = 0;
	rtk_port_phyTestMode_t phyTestMode;

	const char *phy_test_mode_info[] = {
		"1G_TEST_MODE_NONE",
		"1G_TEST_MODE1",
		"1G_TEST_MODE2",
		"1G_TEST_MODE3",
		"1G_TEST_MODE4",
		"2.5G/5G/10G_TEST_MODE_NONE",
		"2.5G/5G/10G_TEST_MODE1",
		"2.5G/5G/10G_TEST_MODE2",
		"2.5G/5G/10G_TEST_MODE3",
		"2.5G/5G/10G_TEST_MODE4_1",
		"2.5G/5G/10G_TEST_MODE4_2",
		"2.5G/5G/10G_TEST_MODE4_4",
		"2.5G/5G/10G_TEST_MODE4_5",
		"2.5G/5G/10G_TEST_MODE4_6",
		"2.5G/5G/10G_TEST_MODE4_DUALTONE1",
		"2.5G/5G/10G_TEST_MODE4_DUALTONE2",
		"2.5G/5G/10G_TEST_MODE4_DUALTONE3",
		"2.5G/5G/10G_TEST_MODE4_DUALTONE4",
		"2.5G/5G/10G_TEST_MODE4_DUALTONE5",
		"2.5G/5G/10G_TEST_MODE5",
		"2.5G/5G/10G_TEST_MODE6",
		"2.5G/5G/10G_TEST_MODE7",
		"100M_TEST_MODE_NONE",
		"100M_TEST_MODE_IEEE",
		"100M_TEST_MODE_ANSI_JITTER",
		"100M_TEST_MODE_ANSI_DROOP",
		NULL
	};

	RTL8261_DEBUG("%s - %s: %s (%d)", file->f_path.dentry->d_iname, __FUNCTION__, buffer, count);
	if (buffer) {
		char *strptr, *split_str;

		/* copy data to the buffer */
		strncpy(tmpbuf, buffer, len);
		tmpbuf[len] = '\0';

		strptr = tmpbuf;
		RTL8261_DEBUG("strptr: [%s]", strptr);

		/*parse command*/
		split_str = strsep(&strptr," ");
		port_num = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		testMode = simple_strtoul(split_str, NULL, 0);
		RTL8261_MSG("Parsing result: [port %d][testMode %d]", port_num, testMode);
		RTL8261_MSG("[PHY Test Mode]:");
		printk("\n\t00: %s\n\t01: %s\n\t02: %s\n\t03: %s\n\t04: %s\n\t05: %s"
			"\n\t06: %s\n\t07: %s\n\t08: %s\n\t09: %s\n\t10: %s\n\t11: %s\n\t12: %s\n\t13: %s\n\t14: %s\n\t15: %s"
			"\n\t16: %s\n\t17: %s\n\t18: %s\n\t19: %s\n\t20: %s\n\t21: %s\n\t22: %s\n\t23: %s\n\t24: %s\n\t25: %s",
			phy_test_mode_info[0], phy_test_mode_info[1], phy_test_mode_info[2], phy_test_mode_info[3],
			phy_test_mode_info[4], phy_test_mode_info[5], phy_test_mode_info[6], phy_test_mode_info[7],
			phy_test_mode_info[8], phy_test_mode_info[9], phy_test_mode_info[10], phy_test_mode_info[11],
			phy_test_mode_info[12], phy_test_mode_info[13], phy_test_mode_info[14], phy_test_mode_info[15],
			phy_test_mode_info[16], phy_test_mode_info[17], phy_test_mode_info[18], phy_test_mode_info[19],
			phy_test_mode_info[20], phy_test_mode_info[21], phy_test_mode_info[22], phy_test_mode_info[23],
			phy_test_mode_info[24], phy_test_mode_info[25]);

#ifdef CONFIG_RTL8261_PORT6_PHY_ENABLE
		if (port_num == 6)	phy_addr = CONFIG_RTL8261_PORT6_PHY_ADDR;
#endif
#ifdef CONFIG_RTL8261_PORT7_PHY_ENABLE
		if (port_num == 7)	phy_addr = CONFIG_RTL8261_PORT7_PHY_ADDR;
#endif
		if (phy_addr == 0) {
			RTL8261_ERROR("UNKNOWN port num %d", port_num);
			return count;
		}

		if (testMode == 0)		phyTestMode.mode = RTK_PORT_PHY_1G_TEST_MODE_NONE;
		else if (testMode == 1)		phyTestMode.mode = RTK_PORT_PHY_1G_TEST_MODE1;
		else if (testMode == 2)		phyTestMode.mode = RTK_PORT_PHY_1G_TEST_MODE2;
		else if (testMode == 3)		phyTestMode.mode = RTK_PORT_PHY_1G_TEST_MODE3;
		else if (testMode == 4)		phyTestMode.mode = RTK_PORT_PHY_1G_TEST_MODE4;
		else if (testMode == 5)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE_NONE;
		else if (testMode == 6)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE1;
		else if (testMode == 7)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE2;
		else if (testMode == 8)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE3;
		else if (testMode == 9)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE4_1;
		else if (testMode == 10)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE4_2;
		else if (testMode == 11)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE4_4;
		else if (testMode == 12)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE4_5;
		else if (testMode == 13)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE4_6;
		else if (testMode == 14)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE4_DUALTONE1;
		else if (testMode == 15)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE4_DUALTONE2;
		else if (testMode == 16)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE4_DUALTONE3;
		else if (testMode == 17)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE4_DUALTONE4;
		else if (testMode == 18)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE4_DUALTONE5;
		else if (testMode == 19)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE5;
		else if (testMode == 20)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE6;
		else if (testMode == 21)		phyTestMode.mode = RTK_PORT_PHY_10G_TEST_MODE7;
		else if (testMode == 22)		phyTestMode.mode = RTK_PORT_PHY_100M_TEST_MODE_NONE;
		else if (testMode == 23)		phyTestMode.mode = RTK_PORT_PHY_100M_TEST_MODE_IEEE;
		else if (testMode == 24)		phyTestMode.mode = RTK_PORT_PHY_100M_TEST_MODE_ANSI_JITTER;
		else if (testMode == 25)		phyTestMode.mode = RTK_PORT_PHY_100M_TEST_MODE_ANSI_DROOP;
		else {
			RTL8261_ERROR("UNKNOWN test mode %d", testMode);
			return count;
		}

		RTL8261_MSG("================ [PORT %d] PHY_ADDR %d ================", port_num, phy_addr);

		if ((ret = rtk_port_phyIeeeTestMode_set(RTL8261_DEFAULT_UNIT_ID, phy_addr, &phyTestMode)) == RT_ERR_OK) {
			RTL8261_MSG("rtk_port_phyIeeeTestMode_set(%d, %d, 0x%04x) [PHY Test Mode: %s] OK",
				RTL8261_DEFAULT_UNIT_ID, phy_addr, phyTestMode.mode, phy_test_mode_info[testMode]);
		}
		else
			RTL8261_ERROR("rtk_port_phyIeeeTestMode_set ERROR (ret = 0x%x) !!!", ret);
	}

	return count;
}


static int Rtl8261_SdsEyeParam_get_fops(struct seq_file *s, void *v)
{
	int32 i, ret, sdsId = 0;
	rtk_sds_eyeParam_t eyeParam;

	for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
		RTL8261_MSG("================ [PORT %d] PHY_ADDR %d ================", my_port_descp[i].ca_port, my_port_descp[i].phy_addr);
		if ((ret = rtk_port_phySdsEyeParam_get(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, sdsId, &eyeParam)) == RT_ERR_OK){
			RTL8261_MSG("rtk_port_phySdsEyeParam_get OK: unit [%d], phyAddr [%d], sdsId [%d]", RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, sdsId);
			RTL8261_MSG("\timpedance : 0x%0x", eyeParam.impedance);
			RTL8261_MSG("\tpre_amp : 0x%0x", eyeParam.pre_amp);
			RTL8261_MSG("\tmain_amp : 0x%0x", eyeParam.main_amp);
			RTL8261_MSG("\tpost_amp : 0x%0x", eyeParam.post_amp);
			RTL8261_MSG("\tpost2_amp : 0x%0x", eyeParam.post2_amp);
			RTL8261_MSG("\tpre_en : %s", eyeParam.pre_en ? "Enabled" : "Disabled");
			RTL8261_MSG("\tpost_en : %s", eyeParam.post_en ? "Enabled" : "Disabled");
			RTL8261_MSG("\tpost2_en : %s", eyeParam.post2_en ? "Enabled" : "Disabled");
			RTL8261_MSG("\tmain_en : %s", eyeParam.main_en ? "Enabled" : "Disabled");
		}
		else
			RTL8261_ERROR("rtk_port_phySdsEyeParam_get ERROR (ret = 0x%x) !!!", ret);
	}

	return 0;
}

static struct task_struct *rtl261_port_phy_sds_eye_task;
uint32 eye_ca_port, eye_sdsId, eye_frameNum = 0;
rtk_sds_eyeParam_t eyeParam;
int32 rtl8261_port_phy_sds_eye_status = 1; //0: stop, 1:start

static int rtl8261_port_phy_sds_eye(void *data)
{
	int32 i, ret;

	for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
		if (my_port_descp[i].ca_port == eye_ca_port) {
			RTL8261_MSG("================ [PORT %d] PHY_ADDR %d ================", my_port_descp[i].ca_port, my_port_descp[i].phy_addr);
			if ((ret = rtk_port_phySdsEyeParam_set(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, eye_sdsId, &eyeParam)) == RT_ERR_OK){
				RTL8261_MSG("rtk_port_phySdsEyeParam_set OK: unit [%d], phyAddr [%d], sdsId [%d]", RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, eye_sdsId);
			}
			else {
				RTL8261_ERROR("rtk_port_phySdsEyeParam_set ERROR (ret = 0x%x) !!!", ret);
				rtl8261_port_phy_sds_eye_status = 0;
			}

			if((ret = rtk_port_phyEyeMonitor_start(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, eye_sdsId, eye_frameNum)) == RT_ERR_OK)
				RTL8261_MSG("rtk_port_phyEyeMonitor_start(%d, %d, %d, %d)", RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, eye_sdsId, eye_frameNum);
			else {
				RTL8261_ERROR("rtk_port_phyEyeMonitor_start ERROR(0x%x) !!!", ret);
				rtl8261_port_phy_sds_eye_status = 0;
			}
		}
		if (rtl8261_port_phy_sds_eye_status == 0) {
			RTL8261_ERROR("eye monitor stop.");
			return RT_ERR_OK;
		}
	}

	RTL8261_MSG("eye monitor done.");
	return RT_ERR_OK;
}

static int Rtl8261_SdsEyeParam_set_fops(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	//int32 i, ret;
	//uint32 ca_port, sdsId, frameNum = 0;
	char tmpbuf[MAX_COMMAND_LEN] = {0};
	int len = (count > MAX_COMMAND_LEN) ? (MAX_COMMAND_LEN -1) : count;
	//rtk_sds_eyeParam_t eyeParam;

	RTL8261_DEBUG("%s - %s: %s (%d)", file->f_path.dentry->d_iname, __FUNCTION__, buffer, count);
	if (buffer) {
		char *strptr, *split_str;

		/* copy data to the buffer */
		strncpy(tmpbuf, buffer, len);
		tmpbuf[len] = '\0';

		strptr = tmpbuf;
		RTL8261_MSG("Input: [%s]", strptr);

		/*parse command*/
		split_str = strsep(&strptr," ");
		eye_ca_port = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		eye_sdsId = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		eye_frameNum = simple_strtoul(split_str, NULL, 0);

		split_str = strsep(&strptr," ");
		eyeParam.impedance = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		eyeParam.pre_amp = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		eyeParam.main_amp = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		eyeParam.post_amp = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		eyeParam.post2_amp = simple_strtoul(split_str, NULL, 0);

		split_str = strsep(&strptr," ");
		eyeParam.pre_en = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		eyeParam.post_en = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		eyeParam.post2_en = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		eyeParam.main_en = simple_strtoul(split_str, NULL, 0);

		RTL8261_MSG("Parsing result:");
		RTL8261_MSG("\tca_port : 0x%0x", eye_ca_port);
		RTL8261_MSG("\tsdsId : 0x%0x", eye_sdsId);
		RTL8261_MSG("\tframeNum : 0x%0x", eye_frameNum);
		RTL8261_MSG("\timpedance : 0x%0x", eyeParam.impedance);
		RTL8261_MSG("\tpre_amp : 0x%0x", eyeParam.pre_amp);
		RTL8261_MSG("\tmain_amp : 0x%0x", eyeParam.main_amp);
		RTL8261_MSG("\tpost_amp : 0x%0x", eyeParam.post_amp);
		RTL8261_MSG("\tpost2_amp : 0x%0x", eyeParam.post2_amp);
		RTL8261_MSG("\tpre_en : %s", eyeParam.pre_en ? "Enabled" : "Disabled");
		RTL8261_MSG("\tpost_en : %s", eyeParam.post_en ? "Enabled" : "Disabled");
		RTL8261_MSG("\tpost2_en : %s", eyeParam.post2_en ? "Enabled" : "Disabled");
		RTL8261_MSG("\tmain_en : %s", eyeParam.main_en ? "Enabled" : "Disabled");
#if 0
		for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
			if (my_port_descp[i].ca_port == ca_port) {
				RTL8261_MSG("================ [PORT %d] PHY_ADDR %d ================", my_port_descp[i].ca_port, my_port_descp[i].phy_addr);
				if ((ret = rtk_port_phySdsEyeParam_set(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, sdsId, &eyeParam)) == RT_ERR_OK){
					RTL8261_MSG("rtk_port_phySdsEyeParam_set OK: unit [%d], phyAddr [%d], sdsId [%d]", RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, sdsId);
				}
				else
					RTL8261_ERROR("rtk_port_phySdsEyeParam_set ERROR (ret = 0x%x) !!!", ret);

				if((ret = rtk_port_phyEyeMonitor_start(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, sdsId, frameNum)) == RT_ERR_OK)
					RTL8261_MSG("rtk_port_phyEyeMonitor_start(%d, %d, %d, %d)", RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, sdsId, frameNum);
				else
					RTL8261_ERROR("rtk_port_phyEyeMonitor_start ERROR(0x%x) !!!", ret);
			}
		}
#else
		rtl261_port_phy_sds_eye_task = kthread_create(rtl8261_port_phy_sds_eye, NULL, "rtl8261_port_phy_sds_eye");
		if (WARN_ON(!rtl261_port_phy_sds_eye_task)) {
			RTL8261_ERROR("error: create rtl261_port_phy_sds_eye_task thread fail.");
			return count;
		}
		kthread_bind(rtl261_port_phy_sds_eye_task, 0);
		wake_up_process(rtl261_port_phy_sds_eye_task);

		rtl8261_port_phy_sds_eye_status = 1;

		RTL8261_MSG("eye monitor started.");
#endif
	}

	return count;
}

#if 0
static int Rtl8261_phyEyeMonitor_get_fops(struct seq_file *s, void *v)
{

	RTL8261_ERROR("Rtl8261_phyEyeMonitor_get_fops is NOT implemented !!!");
#if 0
	int32 i, ret;
	uint32 sdsId = 0, frameNum = 0;

	for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
		RTL8261_MSG("================ [PORT %d] PHY_ADDR %d ================", my_port_descp[i].ca_port, my_port_descp[i].phy_addr);
		if((ret = rtk_port_phyEyeMonitor_start(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, sdsId, frameNum)) == RT_ERR_OK)
			RTL8261_MSG("rtk_port_phyEyeMonitor_start(%d, %d, %d, %d)", RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, sdsId, frameNum);
		else
			RTL8261_ERROR("rtk_port_phyEyeMonitor_start ERROR(0x%x) !!!", ret);
	}
#endif
	return 0;
}
#endif

static int Rtl8261_phyEyeMonitor_set_fops(struct file *file, const char __user *buffer, size_t count, loff_t *ppos)
{
	int32 i, ret;
	uint32 ca_port = 0, sdsId = 0, frameNum = 0;

	char tmpbuf[MAX_COMMAND_LEN] = {0};
	int len = (count > MAX_COMMAND_LEN) ? (MAX_COMMAND_LEN -1) : count;
	//rtk_sds_eyeParam_t eyeParam;
	rtk_sds_eyeMonInfo_t info;

	RTL8261_DEBUG("%s - %s: %s (%d)", file->f_path.dentry->d_iname, __FUNCTION__, buffer, count);
	if (buffer) {
		char *strptr, *split_str;

		/* copy data to the buffer */
		strncpy(tmpbuf, buffer, len);
		tmpbuf[len] = '\0';

		strptr = tmpbuf;
		RTL8261_MSG("Input: [%s]", strptr);

		/*parse command*/
		split_str = strsep(&strptr," ");
		ca_port = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		sdsId = simple_strtoul(split_str, NULL, 0);
		split_str = strsep(&strptr," ");
		frameNum = simple_strtoul(split_str, NULL, 0);

		RTL8261_MSG("Parsing result:");
		RTL8261_MSG("\tca_port : 0x%0x", ca_port);
		RTL8261_MSG("\tsdsId : 0x%0x", sdsId);
		RTL8261_MSG("\tframeNum : 0x%0x", frameNum);

		for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
			if (my_port_descp[i].ca_port == ca_port) {
				RTL8261_MSG("================ [PORT %d] PHY_ADDR %d ================", my_port_descp[i].ca_port, my_port_descp[i].phy_addr);
				if ((ret = rtk_port_phyEyeMonitorInfo_get(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, sdsId, frameNum, &info)) == RT_ERR_OK)
					RTL8261_MSG("rtk_port_phyEyeMonitorInfo_get (%d, %d, %d, %d) OK", RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, sdsId, frameNum);
				else
					RTL8261_ERROR("rtk_port_phyEyeMonitorInfo_get ERROR (ret = 0x%x) !!!", ret);
				
				RTL8261_MSG("PHY Eye Monitor Info:");
				RTL8261_MSG("\tHeight : 0x%0x", info.height);
				RTL8261_MSG("\tWidth  : 0x%0x", info.width);
			}
		}
	}

	return count;
}


typedef enum rtk_rtl8261_procDir_e
{
	PROC_DIR_RTL8261_ROOT,
	PROC_DIR_RTL8261_LEEF //this field must put at last.
} rtk_rtl8261_procDir_t;

typedef struct rtk_rtl8261_proc_s
{
	char *name;
	int (*get) (struct seq_file *s, void *v);
	int (*set) (struct file *file, const char __user *buffer, size_t count, loff_t *ppos);
	unsigned int inode_id;
	unsigned char unlockBefortWrite;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
	struct proc_ops proc_fops;
#else
	struct file_operations proc_fops;
#endif
	rtk_rtl8261_procDir_t dir;
}rtk_rtl8261_proc_t;

rtk_rtl8261_proc_t rtl8261Proc[]=
{
	{
		.name= PROC_FILE_HELP,
		.get = help_fops,
		.set = NULL,
		.dir = PROC_DIR_RTL8261_ROOT,
	},
#if 0
	{
		.name= PROC_FILE_PHY,
		.get = NULL,
		.set = Rtl8261_phy_fops,
		.dir = PROC_DIR_RTL8261_ROOT,
	},
#endif
	{
		.name= PROC_FILE_LINKSTATUS,
		.get = Rtl8261_link_status_fops,
		.set = NULL,
		.dir = PROC_DIR_RTL8261_ROOT,
	},
	{
		.name= PROC_FILE_CTRL,
		.get = Rtl8261_ctrl_get_fops,
		.set = Rtl8261_ctrl_set_fops,
		.dir = PROC_DIR_RTL8261_ROOT,
	},
	{
		.name= PROC_FILE_CNT,
		.get = Rtl8261_cnt_get_fops,
		.set = Rtl8261_cnt_set_fops,
		.dir = PROC_DIR_RTL8261_ROOT,
	},
	{
		.name= PROC_FILE_FORCEMODEABILITY,
		.get = Rtl8261_ForceModeAbility_get_fops,
		.set = Rtl8261_ForceModeAbility_set_fops,
		.dir = PROC_DIR_RTL8261_ROOT,
	},
	{
		.name= PROC_FILE_SDSTESTMODE,
		.get = Rtl8261_sdstestmode_get_fops,
		.set = Rtl8261_sdstestmode_set_fops,
		.dir = PROC_DIR_RTL8261_ROOT,
	},
	{
		.name= PROC_FILE_IEEETESTMODE,
		.get = Rtl8261_ieeetestmode_get_fops,
		.set = Rtl8261_ieeetestmode_set_fops,
		.dir = PROC_DIR_RTL8261_ROOT,
	},
	{
		.name= PROC_FILE_SDSEYEPARAM,
		.get = Rtl8261_SdsEyeParam_get_fops,
		.set = Rtl8261_SdsEyeParam_set_fops,
		.dir = PROC_DIR_RTL8261_ROOT,
	},
	{
		.name= PROC_FILE_PHYEYEMONITOR,
		.get = NULL,	// Rtl8261_phyEyeMonitor_get_fops,
		.set = Rtl8261_phyEyeMonitor_set_fops,
		.dir = PROC_DIR_RTL8261_ROOT,
	},
};

/* rtl8261 common proc function */
static void *rtk_rtl8261_single_start(struct seq_file *p, loff_t *pos)
{
	return NULL + (*pos == 0);
}

static void *rtk_rtl8261_single_next(struct seq_file *p, void *v, loff_t *pos)
{
	++*pos;
	return NULL;
}

static void rtk_rtl8261_single_stop(struct seq_file *p, void *v)
{
}

int rtk_rtl8261_seq_open(struct file *file, const struct seq_operations *op)
{
	struct seq_file *p = file->private_data;

	if (!p) {
		p = kmalloc(sizeof(*p), GFP_ATOMIC);
		if (!p){
			return -ENOMEM;
		}
		file->private_data = p;
	}
	memset(p, 0, sizeof(*p));
	mutex_init(&p->lock);
	p->op = op;
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 4, 79)
	p->file = file;
#else
#ifdef CONFIG_USER_NS
	p->user_ns = file->f_cred->user_ns;
#endif
#endif
	/*
	 * Wrappers around seq_open(e.g. swaps_open) need to be
	 * aware of this. If they set f_version themselves, they
	 * should call seq_open first and then set f_version.
	 */
	file->f_version = 0;

	/*
	 * seq_files support lseek() and pread().  They do not implement
	 * write() at all, but we clear FMODE_PWRITE here for historical
	 * reasons.
	 *
	 * If a client of seq_files a) implements file.write() and b) wishes to
	 * support pwrite() then that client will need to implement its own
	 * file.open() which calls seq_open() and then sets FMODE_PWRITE.
	 */
	file->f_mode &= ~FMODE_PWRITE;
	return 0;
}


int rtk_rtl8261_single_open(struct file *file, int (*show) (struct seq_file *m, void *v), void *data)
{
	struct seq_operations *op = kmalloc(sizeof(*op), GFP_ATOMIC);
	int res = -ENOMEM;

	if (op) {
		op->start = rtk_rtl8261_single_start;
		op->next = rtk_rtl8261_single_next;
		op->stop = rtk_rtl8261_single_stop;
		op->show = show;
		res = rtk_rtl8261_seq_open(file, op);
		if (!res)
			((struct seq_file *)file->private_data)->private = data;
		else
			kfree(op);
	}
	return res;
}

static int rtk_rtl8261_nullDebugGet(struct seq_file *s, void *v)
{
	return 0;
}

static int rtk_rtl8261_nullDebugSingleOpen(struct inode *inode, struct file *file)
{
       return(single_open(file, rtk_rtl8261_nullDebugGet, NULL));
}

static int rtk_rtl8261_commonDebugSingleOpen(struct inode *inode, struct file *file)
{
	int i, ret = -1;

	//rtl8261_spin_lock_bh(rtl8261Sysdb.lock_rtl8261);
	//========================= Critical Section Start =========================//
	for( i = 0; i < (sizeof(rtl8261Proc) / sizeof(rtk_rtl8261_proc_t)) ; i++) {
		//RTL8261_MSG("common_single_open inode_id=%u i_ino=%u", rtl8261Proc[i].inode_id,(unsigned int)inode->i_ino);

		if(rtl8261Proc[i].inode_id == (unsigned int)inode->i_ino) {
			ret = rtk_rtl8261_single_open(file, rtl8261Proc[i].get, NULL);
			break;
		}
	}
	//========================= Critical Section End =========================//
	//rtl8261_spin_unlock_bh(rtl8261Sysdb.lock_rtl8261);

	return ret;
}

static ssize_t rtk_rtl8261_commonDebugSingleWrite(struct file * file, const char __user * userbuf,
				size_t count, loff_t * off)
{
	int i, ret = -1;
	char procBuffer[count];
	char *pBuffer = NULL;

	/* write data to the buffer */
	memset(procBuffer, 0, sizeof(procBuffer));
	if (copy_from_user(procBuffer, userbuf, count)) {
		return -EFAULT;
	}
	procBuffer[count-1] = '\0';
	pBuffer = procBuffer;

	//rtl8261_spin_lock_bh(rtl8261Sysdb.lock_rtl8261);
	//========================= Critical Section Start =========================//
	for( i = 0; i < (sizeof(rtl8261Proc) / sizeof(rtk_rtl8261_proc_t)) ; i++) {
		//RTL8261_MSG("common_single_write inode_id=%u i_ino=%u",rtl8261Proc[i].inode_id,(unsigned int)file->f_dentry->d_inode->i_ino);

		if(rtl8261Proc[i].inode_id == (unsigned int)file->f_inode->i_ino) {
			//if(rtl8261Proc[i].unlockBefortWrite)
			//	rtl8261_spin_unlock_bh(rtl8261Sysdb.lock_rtl8261);
			ret = rtl8261Proc[i].set(file, pBuffer, count, off);
			break;
		}
	}
	//========================= Critical Section End =========================//
	//if((i!=(sizeof(rtl8261Proc)/sizeof(rtk_rtl8261_proc_t))) && !rtl8261Proc[i].unlockBefortWrite)
	//	rtl8261_spin_unlock_bh(rtl8261Sysdb.lock_rtl8261);

	return ret;
}

int rtk_rtl8261_proc_init(void)
{
	int i = 0;
	struct proc_dir_entry *p = NULL;

	rtl8261_proc_dir = proc_mkdir(PROC_DIR_RTL8261, NULL);
	if (rtl8261_proc_dir == NULL) {
		RTL8261_ERROR("create /proc/%s failed!", PROC_DIR_RTL8261);
		return 0;
	}

	for(i = 0; i < (sizeof(rtl8261Proc)/sizeof(rtk_rtl8261_proc_t)) ; i++)
	{
		struct proc_dir_entry *parentDir = NULL;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
		if(rtl8261Proc[i].get == NULL)
			rtl8261Proc[i].proc_fops.proc_open = rtk_rtl8261_nullDebugSingleOpen;
		else
			rtl8261Proc[i].proc_fops.proc_open = rtk_rtl8261_commonDebugSingleOpen;

		if(rtl8261Proc[i].set == NULL)
			rtl8261Proc[i].proc_fops.proc_write = NULL;
		else
			rtl8261Proc[i].proc_fops.proc_write = rtk_rtl8261_commonDebugSingleWrite;

		rtl8261Proc[i].proc_fops.proc_read = seq_read;
		rtl8261Proc[i].proc_fops.proc_lseek = seq_lseek;
		rtl8261Proc[i].proc_fops.proc_release = single_release;
#else
		if(rtl8261Proc[i].get == NULL)
			rtl8261Proc[i].proc_fops.open = rtk_rtl8261_nullDebugSingleOpen;
		else
			rtl8261Proc[i].proc_fops.open = rtk_rtl8261_commonDebugSingleOpen;

		if(rtl8261Proc[i].set == NULL)
			rtl8261Proc[i].proc_fops.write = NULL;
		else
			rtl8261Proc[i].proc_fops.write = rtk_rtl8261_commonDebugSingleWrite;

		rtl8261Proc[i].proc_fops.read = seq_read;
		rtl8261Proc[i].proc_fops.llseek = seq_lseek;
		rtl8261Proc[i].proc_fops.release = single_release;
#endif

		switch(rtl8261Proc[i].dir)
		{
			case PROC_DIR_RTL8261_ROOT:
				parentDir = rtl8261_proc_dir;
				break;
			default:
				break;
		}

		p = proc_create_data(rtl8261Proc[i].name, S_IRUGO, parentDir, &(rtl8261Proc[i].proc_fops),NULL);
		if(!p){
			RTL8261_ERROR("create proc %s failed!", rtl8261Proc[i].name);
		}
		rtl8261Proc[i].inode_id = p->low_ino;
	}
	RTL8261_MSG("Creat %d proc entry.", i);

	return 1;
}

void rtk_rtl8261_proc_exit(void)
{
	int i = 0;

	for(i = 0 ; i < (sizeof(rtl8261Proc) / sizeof(rtk_rtl8261_proc_t)) ; i++)
	{
		struct proc_dir_entry *parentDir = NULL;

		switch(rtl8261Proc[i].dir)
		{
			case PROC_DIR_RTL8261_ROOT:
				parentDir = rtl8261_proc_dir;
				break;
			default:
				break;
		}

		remove_proc_entry(rtl8261Proc[i].name, parentDir);
	}

	proc_remove(rtl8261_proc_dir);
}


/* Function Name:
 *      _phy_rtk_oui_chk
 * Description:
 *      Identify the OUI is the realtek OUI or not?
 * Input:
 *      reg2_data - device identifier reg 2 data
 *      reg3_data - device identifier reg 3 data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - Realtek OUI
 *      RT_ERR_FAILED - not Realtek OUI
 * Note:
 *      None
 */
int32
__phy_rtk_oui_chk(uint32 reg2_data, uint32 reg3_data)
{
	if ((reg2_data != PHY_IDENT_OUI_03_18) ||
		((reg3_data >> OUI_19_24_OFFSET) != PHY_IDENT_OUI_19_24))
	{
		return RT_ERR_FAILED;
	}

	return RT_ERR_OK;
}

/* Function Name:
 *      _phy_identify_default_c45
 * Description:
 *      Identify the port is match input PHY information or not for clause 45 PHY
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      model_id - model id
 *      rev_id   - revision id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - is match the PHY information
 *      RT_ERR_FAILED           - access failure or others
 *      RT_ERR_PHY_NOT_MATCH    - is not match the PHY information
 * Note:
 *      None
 */
int32
__phy_identify_default_c45(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
	int32       ret;
	uint32      reg2_data = 0, reg3_data = 0;
	uint32      chip_model_id, chip_rev_id;
	//hal_control_t   *pHalCtrl = NULL;

	//RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);
	//RT_PARAM_CHK((NULL == MACDRV(pHalCtrl)->fMdrv_miim_read), RT_ERR_FAILED);

	//MACDRV(pHalCtrl)->fMdrv_miim_mmd_read(unit, port, 1, 2, &reg2_data);
	//ret = MACDRV(pHalCtrl)->fMdrv_miim_mmd_read(unit, port, 1, 3, &reg3_data);

	phy_hal_mmd_read(unit, port, 1, 0x2, &reg2_data);
	RTL8261_MSG(" - PhyAddr-%d: Read MMD1.0x2  = 0x%0X", port, reg2_data);
	phy_hal_mmd_read(unit, port, 1, 0x3, &reg3_data);
	RTL8261_MSG(" - PhyAddr-%d: Read MMD1.0x2  = 0x%0X", port, reg3_data);

	if ((ret = __phy_rtk_oui_chk(reg2_data, reg3_data)) != RT_ERR_OK)
	{
		RTL8261_ERROR("Compare OUI reg2_data failed(unit %d, port %d): reg2_data= 0x%x; PHY_IDENT_OUI_03_18 = 0x%x!!", unit, port, reg2_data, PHY_IDENT_OUI_03_18);
		RTL8261_ERROR("Compare OUI reg3_data failed(unit %d, port %d): reg3_data.b[24:19]= 0x%x; PHY_IDENT_OUI_19_24 = 0x%x!!", unit, port, (reg3_data >> OUI_19_24_OFFSET), PHY_IDENT_OUI_19_24);
		return ret;
	}

	chip_model_id = PHY_IDENT_MODEL_NUM(reg3_data);
	chip_rev_id   = PHY_IDENT_REV_NUM(reg3_data);

	if ((chip_model_id == model_id) && (chip_rev_id >= rev_id))
	{
		return RT_ERR_OK;
	}

	return RT_ERR_PHY_NOT_MATCH;
}

static int32
__phy_identify_8261(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
	//hal_control_t   *pHalCtrl = NULL;
	int32           ret;
	uint32          reg_data1 = 0;
	uint32          reg_data2 = 0;

	//RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);

	if (port >= RTK_MAX_PORT_PER_UNIT)
	{
		return RT_ERR_PHY_NOT_MATCH;
	}

	if ((ret = __phy_identify_default_c45(unit, port, model_id, rev_id)) != RT_ERR_OK)
	{
		return ret;
	}

#if 1
	//if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_mmd_read(unit, port, 30, 0x103, &reg_data1)) != RT_ERR_OK)
	//    return ret;
	//if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_mmd_read(unit, port, 30, 0x104, &reg_data2)) != RT_ERR_OK)
	//    return ret;

	phy_hal_mmd_read(unit, port, 30, 0x103, &reg_data1);
	RTL8261_MSG(" - PhyAddr-%d: Read MMD30.0x103  = 0x%0X", port, reg_data1);
	phy_hal_mmd_read(unit, port, 30, 0x104, &reg_data2);
	RTL8261_MSG(" - PhyAddr-%d: Read MMD30.0x104  = 0x%0X", port, reg_data2);

	reg_data2 = REG32_FIELD_GET(reg_data2, 11, 0xF800);

	if (reg_data1 == 0x8291 && reg_data2 == 0x1F)
	{
		return RT_ERR_OK;
	}
	else
	{
		return RT_ERR_PHY_NOT_MATCH;
	}
#else
	if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_mmd_read(unit, port, 30, 0x101, &reg_data)) != RT_ERR_OK)
		return ret;
	if ((reg_data & 0xF000) != 0xB000)
	{
		reg_data &= (~(0xF000));
		reg_data |= (0xB000);
		if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_mmd_write(unit, port, 30, 0x101, reg_data)) != RT_ERR_OK)
			return ret;
	}
	if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_mmd_read(unit, port, 30, 0x102, &reg_data)) != RT_ERR_OK)
	{
		return ret;
	}
	if ((reg_data & 0x60) != 0)
	{
		return RT_ERR_PHY_NOT_MATCH;
	}
#endif

	return RT_ERR_OK;
}

static int32
__phy_identify_8261B(uint32 unit, rtk_port_t port, uint32 model_id, uint32 rev_id)
{
	//hal_control_t   *pHalCtrl = NULL;
	int32           ret;
	uint32          reg_data1 = 0;

	//RT_PARAM_CHK((NULL == (pHalCtrl = hal_ctrlInfo_get(unit))), RT_ERR_FAILED);

	if (port >= RTK_MAX_PORT_PER_UNIT)
	{
		return RT_ERR_PHY_NOT_MATCH;
	}

	if ((ret = __phy_identify_default_c45(unit, port, model_id, rev_id)) != RT_ERR_OK)
	{
		return ret;
	}

	//if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_mmd_read(unit, port, 30, 0x103, &reg_data1)) != RT_ERR_OK)
	//    return ret;

	phy_hal_mmd_read(unit, port, 30, 0x103, &reg_data1);
	RTL8261_MSG(" - PhyAddr-%d: Read MMD30.0x103  = 0x%0X", port, reg_data1);

	if (reg_data1 == 0x8261)
	{
		return RT_ERR_OK;
	}
	else
	{
		return RT_ERR_PHY_NOT_MATCH;
	}
}

u32 __eth_phy_rtl8261i_add_phy(u32 port, u8 phy_addr) {
	RTL8261_DEBUG("%s(%d): port = %d, phy_dev = %d", __FUNCTION__, __LINE__, port, phy_addr);
	if (rtl8261_count < RTK_EXTGPHY_RTL8261_MAX) {
		rtl8261_port_descp[rtl8261_count].mac_id = phy_addr;
		rtl8261_port_descp[rtl8261_count].phy_addr = phy_addr;
		rtl8261_port_descp[rtl8261_count].ca_port = port;

		if (__phy_identify_8261(RTL8261_DEFAULT_UNIT_ID, phy_addr, PHY_MODEL_ID_RTL826X, PHY_REV_NO_A) == RT_ERR_OK) {
			rtl8261_phy_Descp[rtl8261_count].chip = RTK_PHYTYPE_RTL8261;
			RTL8261_INFO("RTL8261 identified");
		} else if (__phy_identify_8261B(RTL8261_DEFAULT_UNIT_ID, phy_addr, PHY_MODEL_ID_RTL826X, PHY_REV_NO_D) == RT_ERR_OK) {
			rtl8261_phy_Descp[rtl8261_count].chip = RTK_PHYTYPE_RTL8261B;
			RTL8261_INFO("RTL8261N identified");
		} else
			RTL8261_ERROR("UNKNOWN Model ID and Rev No.");

		rtl8261_phy_Descp[rtl8261_count].mac_id = phy_addr;
		rtl8261_count++;
	}
	else
		RTL8261_ERROR("Reach Support rtl8261 number for port  %u ; phyAddr %u", port, phy_addr);

	return 0;
}
EXPORT_SYMBOL(__eth_phy_rtl8261i_add_phy);

u32 __eth_phy_rtl8261i_init(u8 phy_dev) {
	int ret, i = 0;
	uint32 data;
	rtk_phy_initInfo_t initInfo;

	//rtk_port_linkStatus_t status;

	RTL8261_DEBUG("%s(%d): phy_dev = %d", __FUNCTION__, __LINE__, phy_dev);
	if (rtl8261_init) {
		RTL8261_MSG("All RTL8261s are init.");
		return 1;
	}

	initInfo.port_desc = rtl8261_port_descp;
	initInfo.phy_desc = rtl8261_phy_Descp;

	while ((initInfo.port_desc[i].mac_id != HWP_END) && (rtl8261_phy_Descp[i].chip != HWP_END)) {
		if (i < RTK_EXTGPHY_RTL8261_MAX) {
			RTL8261_DEBUG("#%d [port_descp] .mac_id = %d, .phy_idx = %d, .eth = %d, .medi = %d, .smi = %d, .phy_addr = %d, .ca_port = %d", i,
				initInfo.port_desc[i].mac_id,
				initInfo.port_desc[i].phy_idx,
				initInfo.port_desc[i].eth,
				initInfo.port_desc[i].medi,
				initInfo.port_desc[i].smi,
				initInfo.port_desc[i].phy_addr,
				initInfo.port_desc[i].ca_port);
			RTL8261_DEBUG("   [phy_Descp ] .chip = %d, .mac_id = %d, .phy_max = %d",
				initInfo.phy_desc[i].chip,
				initInfo.phy_desc[i].mac_id,
				initInfo.phy_desc[i].phy_max);
		}
		i++;
	}

	RTL8261_DEBUG("PHY lkm init");
	if ((ret = rtk_init(&initInfo)) == RT_ERR_OK)
		RTL8261_INFO("rtk_init OK");
	else
		RTL8261_ERROR("rtk_init Fail (0x%x) !!!", ret);

	for (i = 0 ; rtl8261_port_descp[i].mac_id != HWP_END ; i++) {
		RTL8261_MSG("[PORT %d] Setting RTL8261",  rtl8261_port_descp[i].ca_port);

		data = RTK_PHY_CTRL_SERDES_MODE_USXGMII;
		RTL8261_MSG(" - Set serdes mode as RTK_PHY_CTRL_SERDES_MODE_USXGMII");
		if ((ret = rtk_port_phyCtrl_set(RTL8261_DEFAULT_UNIT_ID, rtl8261_port_descp[i].phy_addr, RTK_PHY_CTRL_SERDES_MODE, data)) != RT_ERR_OK)
			RTL8261_ERROR(" - rtk_port_phyCtrl_set (0x%x) !!!", ret);

		if (rtl8261_phy_Descp[i].chip == RTK_PHYTYPE_RTL8261B) {
			RTL8261_MSG(" - Set TX Polarity as 1 for RTL8261N");
			rtk_port_phyCtrl_set(RTL8261_DEFAULT_UNIT_ID, rtl8261_port_descp[i].phy_addr, RTK_PHY_CTRL_SERDES_TX_POLARITY, 1);
		}

		if (rtl8261_port_descp[i].ca_port == RTL8261_CA_XFI_PORT) {
#ifdef CONFIG_LUNA_G3_SERIES
			/* Patch for port 6 on luna G3 Series */
			RTL8261_INFO(" - Set USXGMII CFW as 0xaa");
			phy_hal_mmd_read(RTL8261_DEFAULT_UNIT_ID, rtl8261_port_descp[i].phy_addr, 30, 0x141, &data);
			//RTL8261_MSG(" - PhyAddr-%d: Read MMD30.0x141  = 0x%0X", rtl8261_port_descp[i].phy_addr, data);
			data = 0x80aa;
			phy_hal_mmd_write(RTL8261_DEFAULT_UNIT_ID, rtl8261_port_descp[i].phy_addr, 30, 0x141, data);
			//RTL8261_MSG(" - PhyAddr-%d: Set  MMD30.0x141  = 0x%0X", rtl8261_port_descp[i].phy_addr, data);

			phy_hal_mmd_read(RTL8261_DEFAULT_UNIT_ID, rtl8261_port_descp[i].phy_addr, 30, 0x143, &data);
			//RTL8261_MSG(" - PhyAddr-%d: Read MMD30.0x143  = 0x%0X", rtl8261_port_descp[i].phy_addr, data);
			data = 0x8C07;
			phy_hal_mmd_write(RTL8261_DEFAULT_UNIT_ID, rtl8261_port_descp[i].phy_addr, 30, 0x143, data);
			//RTL8261_MSG(" - PhyAddr-%d: Set  MMD30.0x143  = 0x%0X", rtl8261_port_descp[i].phy_addr, data);
#endif
		}
		else if (rtl8261_port_descp[i].ca_port == RTL8261_CA_PON_PORT) {
#if 0
			RTL8261_MSG(" - FORCE 10G");
			if((ret = rtk_port_phyForceModeAbility_set(RTL8261_DEFAULT_UNIT_ID, rtl8261_port_descp[i].phy_addr, PORT_SPEED_10G, PORT_FULL_DUPLEX, ENABLED)) != RT_ERR_OK)
				RTL8261_ERROR("rtk_port_phyForceModeAbility_set(0x%x) !!!", ret);

			RTL8261_MSG(" - Disable AN");
			if((ret = rtk_port_phyAutoNegoEnable_set(RTL8261_DEFAULT_UNIT_ID, rtl8261_port_descp[i].phy_addr, DISABLED)) != RT_ERR_OK)
				RTL8261_ERROR("rtk_port_phyAutoNegoEnable_set ERROR(0x%x) !!!", ret);
#endif
#ifdef CONFIG_CA8277B_SERIES
			RTL8261_MSG(" - Disable USXGMII NWAY");
			if ((ret = rtk_port_phyCtrl_set(RTL8261_DEFAULT_UNIT_ID, rtl8261_port_descp[i].phy_addr, RTK_PHY_CTRL_SERDES_USXGMII_AN, DISABLED)) != RT_ERR_OK)
				RTL8261_ERROR(" - rtk_port_phyCtrl_set(%d): ret = 0x%x !!!", RTK_PHY_CTRL_SERDES_USXGMII_AN, ret);
#endif
#if 0
			RTL8261_MSG(" - Disable SGMII NWAY !!!");
			if ((ret = rtk_port_phyCtrl_set(RTL8261_DEFAULT_UNIT_ID, rtl8261_port_descp[i].phy_addr, RTK_PHY_CTRL_SERDES_SGMII_AN, DISABLED)) != RT_ERR_OK)
				RTL8261_ERROR(" - rtk_port_phyCtrl_set(%d): ret = 0x%x !!!", RTK_PHY_CTRL_SERDES_SGMII_AN, ret);
			RTL8261_MSG(" - Disable BASEX NWAY !!!");
			if ((ret = rtk_port_phyCtrl_set(RTL8261_DEFAULT_UNIT_ID, rtl8261_port_descp[i].phy_addr, RTK_PHY_CTRL_SERDES_BASEX_AN, DISABLED)) != RT_ERR_OK)
				RTL8261_ERROR(" - rtk_port_phyCtrl_set(%d): ret = 0x%x !!!", RTK_PHY_CTRL_SERDES_BASEX_AN, ret);
#endif
#ifdef CONFIG_LUNA_G3_SERIES
#ifdef CONFIG_RTL8277C_SERIES
			/* Patch for port 7 on luna G3 Series */
			RTL8261_INFO(" - Set USXGMII CFW as 0xaa");
			phy_hal_mmd_read(RTL8261_DEFAULT_UNIT_ID, rtl8261_port_descp[i].phy_addr, 30, 0x141, &data);
			//RTL8261_MSG(" - PhyAddr-%d: Read MMD30.0x141  = 0x%0X", rtl8261_port_descp[i].phy_addr, data);
			data = 0x80aa;
			phy_hal_mmd_write(RTL8261_DEFAULT_UNIT_ID, rtl8261_port_descp[i].phy_addr, 30, 0x141, data);
			//RTL8261_MSG(" - PhyAddr-%d: Set  MMD30.0x141  = 0x%0X", rtl8261_port_descp[i].phy_addr, data);

			phy_hal_mmd_read(RTL8261_DEFAULT_UNIT_ID, rtl8261_port_descp[i].phy_addr, 30, 0x143, &data);
			//RTL8261_MSG(" - PhyAddr-%d: Read MMD30.0x143  = 0x%0X", rtl8261_port_descp[i].phy_addr, data);
			data = 0x8C07;
			phy_hal_mmd_write(RTL8261_DEFAULT_UNIT_ID, rtl8261_port_descp[i].phy_addr, 30, 0x143, data);
			//RTL8261_MSG(" - PhyAddr-%d: Set  MMD30.0x143  = 0x%0X", rtl8261_port_descp[i].phy_addr, data);
#endif
#endif
		}
	}

	rtl8261_init = 1;
	return 0;
}
EXPORT_SYMBOL(__eth_phy_rtl8261i_init);

u32 __eth_phy_rtl8261i_link_status_get(u8 phy_addr, ext_phy_link_status_t *link_status) {
	rtk_port_linkStatus_t	status;
	rtk_port_speed_t	speed;
	rtk_port_duplex_t	duplex;

	/* WORKAROUND: Read twice for real link status ??? */
	rtk_port_phyLinkStatus_get(RTL8261_DEFAULT_UNIT_ID, phy_addr, &status);
	if(rtk_port_phyLinkStatus_get(RTL8261_DEFAULT_UNIT_ID, phy_addr, &status) == RT_ERR_OK) {
		link_status->link_up = (status == PORT_LINKUP) ? 1 : 0 ;
		if (status == PORT_LINKUP) {
			link_status->link_up = 1;
			if (rtk_port_speedDuplex_get(RTL8261_DEFAULT_UNIT_ID, phy_addr, &speed, &duplex) == RT_ERR_OK) {
				switch(speed) {
				case PORT_SPEED_10M:
					link_status->speed = EXT_PHY_SPEED_10;
					break;
				case PORT_SPEED_100M:
					link_status->speed = EXT_PHY_SPEED_100;
					break;
				case PORT_SPEED_1000M:
					link_status->speed = EXT_PHY_SPEED_1000;
					break;
				case PORT_SPEED_500M:
					link_status->speed = EXT_PHY_SPEED_500;
					break;
				case PORT_SPEED_2G:
					link_status->speed = EXT_PHY_SPEED_2000;
					break;
				case PORT_SPEED_2_5G:
					link_status->speed = EXT_PHY_SPEED_2500;
					break;
				case PORT_SPEED_5G:
					link_status->speed = EXT_PHY_SPEED_5000;
					break;
				case PORT_SPEED_10G:
					link_status->speed = EXT_PHY_SPEED_10000;
					break;
				case PORT_SPEED_2_5G_LITE:
					link_status->speed = EXT_PHY_SPEED_2500LITE;
					break;
				case PORT_SPEED_5G_LITE:
					link_status->speed = EXT_PHY_SPEED_5000LITE;
					break;
				case PORT_SPEED_10G_LITE:
					link_status->speed = EXT_PHY_SPEED_10000LITE;
					break;
				case PORT_SPEED_END:
				default:
					link_status->speed = EXT_PHY_SPEED_MAX;
					break;
				}

				if (duplex == PORT_HALF_DUPLEX)
					link_status->duplex = EXT_PHY_DUPLEX_HALF;
				else if (duplex == PORT_FULL_DUPLEX)
					link_status->duplex = EXT_PHY_DUPLEX_FULL;
				else
					link_status->duplex = EXT_PHY_DUPLEX_INVALID;
			}
		}
		else
			link_status->link_up = 0;
	}
	return 0;
}
EXPORT_SYMBOL(__eth_phy_rtl8261i_link_status_get);



/*
 * Function Declaration
 */
static int __init phy_lkm_init(void)
{
        RTL8261_INFO("rtk-ms-phyOnly-%s", RT_VERSION_SDK);
#if IS_BUILTIN(CONFIG_RTK_EXT_GPHY) || IS_MODULE(CONFIG_RTK_EXT_GPHY)
        RTL8261_INFO("Phys init. will be done by extGphy");
#else
	int32 ret, i;
	rtk_phy_initInfo_t initInfo;
	uint32 data;

	for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
		RTL8261_MSG("[PORT %d] Detecting RTL8261",  my_port_descp[i].ca_port);
		phy_hal_mmd_read(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 1, 3, &data);
		if (data == RTL8261_MMD_1_3_VALUE) {
			RTL8261_MSG(" - Found it, UNIT %d/PHYADDR %d/MMD %d/REG 0x%X = 0x%0X",
				RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 1, 3, data);
		}
		else if (data == RTL8261_CCUT_MMD_1_3_VALUE) {
			RTL8261_MSG(" - Found it (C CUT), UNIT %d/PHYADDR %d/MMD %d/REG 0x%X = 0x%0X",
				RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 1, 3, data);
		}
		else {
			RTL8261_ERROR(" - Not RTL8261!!!, UNIT %d/PHYADDR %d/MMD %d/REG 0x%X = 0x%0X (should be 0x%04X or 0x%04X)",
				RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 1, 3, data, RTL8261_MMD_1_3_VALUE, RTL8261_CCUT_MMD_1_3_VALUE);
			return 0;
		}
	}

	RTL8261_DEBUG("PHY lkm init");
	initInfo.port_desc = my_port_descp;
	initInfo.phy_desc = my_phy_Descp;

	if ((ret = rtk_init(&initInfo)) == RT_ERR_OK)
		RTL8261_INFO("rtk_init OK");
	else
		RTL8261_ERROR("rtk_init Fail (0x%x) !!!", ret);

	for (i = 0 ; my_port_descp[i].mac_id != HWP_END ; i++) {
		RTL8261_MSG("[PORT %d] Setting RTL8261",  my_port_descp[i].ca_port);

		data = RTK_PHY_CTRL_SERDES_MODE_USXGMII;
		RTL8261_MSG(" - Set serdes mode as RTK_PHY_CTRL_SERDES_MODE_USXGMII");
		if ((ret = rtk_port_phyCtrl_set(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, RTK_PHY_CTRL_SERDES_MODE, data)) != RT_ERR_OK)
			RTL8261_ERROR(" - rtk_port_phyCtrl_set (0x%x) !!!", ret);

		if (my_port_descp[i].ca_port == RTL8261_CA_XFI_PORT) {
#ifdef CONFIG_LUNA_G3_SERIES
			/* Patch for port 6 on luna G3 Series */
			RTL8261_INFO(" - Set USXGMII CFW as 0xaa");
			phy_hal_mmd_read(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x141, &data);
			//RTL8261_MSG(" - PhyAddr-%d: Read MMD30.0x141  = 0x%0X", my_port_descp[i].phy_addr, data);
			data = 0x80aa;
			phy_hal_mmd_write(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x141, data);
			//RTL8261_MSG(" - PhyAddr-%d: Set  MMD30.0x141  = 0x%0X", my_port_descp[i].phy_addr, data);

			phy_hal_mmd_read(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x143, &data);
			//RTL8261_MSG(" - PhyAddr-%d: Read MMD30.0x143  = 0x%0X", my_port_descp[i].phy_addr, data);
			data = 0x8C07;
			phy_hal_mmd_write(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x143, data);
			//RTL8261_MSG(" - PhyAddr-%d: Set  MMD30.0x143  = 0x%0X", my_port_descp[i].phy_addr, data);
#endif
		}
		else if (my_port_descp[i].ca_port == RTL8261_CA_PON_PORT) {
#if 0
			RTL8261_MSG(" - FORCE 10G");
			if((ret = rtk_port_phyForceModeAbility_set(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, PORT_SPEED_10G, PORT_FULL_DUPLEX, ENABLED)) != RT_ERR_OK)
				RTL8261_ERROR("rtk_port_phyForceModeAbility_set(0x%x) !!!", ret);

			RTL8261_MSG(" - Disable AN");
			if((ret = rtk_port_phyAutoNegoEnable_set(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, DISABLED)) != RT_ERR_OK)
				RTL8261_ERROR("rtk_port_phyAutoNegoEnable_set ERROR(0x%x) !!!", ret);
#endif
#ifdef CONFIG_CA8277B_SERIES
			RTL8261_MSG(" - Disable USXGMII NWAY");
			if ((ret = rtk_port_phyCtrl_set(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, RTK_PHY_CTRL_SERDES_USXGMII_AN, DISABLED)) != RT_ERR_OK)
				RTL8261_ERROR(" - rtk_port_phyCtrl_set(%d): ret = 0x%x !!!", RTK_PHY_CTRL_SERDES_USXGMII_AN, ret);
#endif
#if 0
			RTL8261_MSG(" - Disable SGMII NWAY !!!");
			if ((ret = rtk_port_phyCtrl_set(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, RTK_PHY_CTRL_SERDES_SGMII_AN, DISABLED)) != RT_ERR_OK)
				RTL8261_ERROR(" - rtk_port_phyCtrl_set(%d): ret = 0x%x !!!", RTK_PHY_CTRL_SERDES_SGMII_AN, ret);
			RTL8261_MSG(" - Disable BASEX NWAY !!!");
			if ((ret = rtk_port_phyCtrl_set(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, RTK_PHY_CTRL_SERDES_BASEX_AN, DISABLED)) != RT_ERR_OK)
				RTL8261_ERROR(" - rtk_port_phyCtrl_set(%d): ret = 0x%x !!!", RTK_PHY_CTRL_SERDES_BASEX_AN, ret);
#endif
#ifdef CONFIG_LUNA_G3_SERIES
#ifdef CONFIG_RTL8277C_SERIES
			/* Patch for port 7 on luna G3 Series */
			RTL8261_INFO(" - Set USXGMII CFW as 0xaa");
			phy_hal_mmd_read(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x141, &data);
			//RTL8261_MSG(" - PhyAddr-%d: Read MMD30.0x141  = 0x%0X", my_port_descp[i].phy_addr, data);
			data = 0x80aa;
			phy_hal_mmd_write(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x141, data);
			//RTL8261_MSG(" - PhyAddr-%d: Set  MMD30.0x141  = 0x%0X", my_port_descp[i].phy_addr, data);

			phy_hal_mmd_read(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x143, &data);
			//RTL8261_MSG(" - PhyAddr-%d: Read MMD30.0x143  = 0x%0X", my_port_descp[i].phy_addr, data);
			data = 0x8C07;
			phy_hal_mmd_write(RTL8261_DEFAULT_UNIT_ID, my_port_descp[i].phy_addr, 30, 0x143, data);
			//RTL8261_MSG(" - PhyAddr-%d: Set  MMD30.0x143  = 0x%0X", my_port_descp[i].phy_addr, data);
#endif
#endif
		}
	}
#endif
	rtk_rtl8261_proc_init();

	return 0;
}

static void __exit phy_lkm_exit(void)
{
	RTL8261_MSG("Exit PHY lkm");
}

module_init(phy_lkm_init);
module_exit(phy_lkm_exit);

MODULE_AUTHOR("Realtek.com");
MODULE_DESCRIPTION("phy lkm");
MODULE_LICENSE("GPL");

#if 0
EXPORT_SYMBOL(rtk_init);
EXPORT_SYMBOL(rtk_port_linkMedia_get);
EXPORT_SYMBOL(rtk_port_speedDuplex_get);
EXPORT_SYMBOL(rtk_port_adminEnable_set);
EXPORT_SYMBOL(rtk_port_phyAutoNegoEnable_get);
EXPORT_SYMBOL(rtk_port_phyAutoNegoEnable_set);
EXPORT_SYMBOL(rtk_port_phyAutoNegoAbilityLocal_get);
EXPORT_SYMBOL(rtk_port_phyAutoNegoAbility_get);
EXPORT_SYMBOL(rtk_port_phyAutoNegoAbility_set);
EXPORT_SYMBOL(rtk_port_phyForceModeAbility_get);
EXPORT_SYMBOL(rtk_port_phyForceModeAbility_set);
EXPORT_SYMBOL(rtk_port_phyForceFlowctrlMode_get);
EXPORT_SYMBOL(rtk_port_phyForceFlowctrlMode_set);
EXPORT_SYMBOL(rtk_port_phyMasterSlave_get);
EXPORT_SYMBOL(rtk_port_phyMasterSlave_set);
EXPORT_SYMBOL(rtk_port_phyReg_get);
EXPORT_SYMBOL(rtk_port_phyReg_set);
EXPORT_SYMBOL(rtk_port_phyExtParkPageReg_get);
EXPORT_SYMBOL(rtk_port_phyExtParkPageReg_set);
EXPORT_SYMBOL(rtk_port_phymaskExtParkPageReg_set);
EXPORT_SYMBOL(rtk_port_phyMmdReg_get);
EXPORT_SYMBOL(rtk_port_phyMmdReg_set);
EXPORT_SYMBOL(rtk_port_phymaskMmdReg_set);
EXPORT_SYMBOL(rtk_port_phyComboPortMedia_get);
EXPORT_SYMBOL(rtk_port_phyComboPortMedia_set);
EXPORT_SYMBOL(rtk_port_greenEnable_get);
EXPORT_SYMBOL(rtk_port_greenEnable_set);
EXPORT_SYMBOL(rtk_port_phyCrossOverMode_get);
EXPORT_SYMBOL(rtk_port_phyCrossOverMode_set);
EXPORT_SYMBOL(rtk_port_phyCrossOverStatus_get);
EXPORT_SYMBOL(rtk_port_phyComboPortFiberMedia_get);
EXPORT_SYMBOL(rtk_port_phyComboPortFiberMedia_set);
EXPORT_SYMBOL(rtk_port_linkDownPowerSavingEnable_get);
EXPORT_SYMBOL(rtk_port_linkDownPowerSavingEnable_set);
EXPORT_SYMBOL(rtk_port_gigaLiteEnable_get);
EXPORT_SYMBOL(rtk_port_gigaLiteEnable_set);
EXPORT_SYMBOL(rtk_port_2pt5gLiteEnable_get);
EXPORT_SYMBOL(rtk_port_2pt5gLiteEnable_set);
EXPORT_SYMBOL(rtk_port_phyReconfig_register);
EXPORT_SYMBOL(rtk_port_phyReconfig_unregister);
EXPORT_SYMBOL(rtk_port_downSpeedEnable_get);
EXPORT_SYMBOL(rtk_port_downSpeedEnable_set);
EXPORT_SYMBOL(rtk_port_downSpeedStatus_get);
EXPORT_SYMBOL(rtk_port_fiberNwayForceLinkEnable_get);
EXPORT_SYMBOL(rtk_port_fiberNwayForceLinkEnable_set);
EXPORT_SYMBOL(rtk_port_phyLoopBackEnable_get);
EXPORT_SYMBOL(rtk_port_phyLoopBackEnable_set);
EXPORT_SYMBOL(rtk_port_10gMedia_set);
EXPORT_SYMBOL(rtk_port_10gMedia_get);
EXPORT_SYMBOL(rtk_port_phyFiberTxDis_set);
EXPORT_SYMBOL(rtk_port_phyFiberTxDisPin_set);
EXPORT_SYMBOL(rtk_port_fiberRxEnable_get);
EXPORT_SYMBOL(rtk_port_fiberRxEnable_set);
EXPORT_SYMBOL(rtk_port_phyIeeeTestMode_set);
EXPORT_SYMBOL(rtk_port_phyPolar_get);
EXPORT_SYMBOL(rtk_port_phyPolar_set);
EXPORT_SYMBOL(rtk_port_phyEyeMonitor_start);
EXPORT_SYMBOL(rtk_port_phyEyeMonitorInfo_get);
EXPORT_SYMBOL(rtk_port_imageFlash_load);
EXPORT_SYMBOL(rtk_port_phySdsRxCaliStatus_get);
EXPORT_SYMBOL(rtk_port_phyReset_set);
EXPORT_SYMBOL(rtk_port_phyLinkStatus_get);
EXPORT_SYMBOL(rtk_port_phyPeerAutoNegoAbility_get);
EXPORT_SYMBOL(rtk_port_phyMacIntfSerdesMode_get);
EXPORT_SYMBOL(rtk_port_phyLedMode_set);
EXPORT_SYMBOL(rtk_port_phyLedCtrl_get);
EXPORT_SYMBOL(rtk_port_phyLedCtrl_set);
EXPORT_SYMBOL(rtk_port_phyMacIntfSerdesLinkStatus_get);
EXPORT_SYMBOL(rtk_port_phySdsEyeParam_get);
EXPORT_SYMBOL(rtk_port_phySdsEyeParam_set);
EXPORT_SYMBOL(rtk_port_phyMdiLoopbackEnable_get);
EXPORT_SYMBOL(rtk_port_phyMdiLoopbackEnable_set);
EXPORT_SYMBOL(rtk_port_phyIntr_init);
EXPORT_SYMBOL(rtk_port_phyIntrEnable_get);
EXPORT_SYMBOL(rtk_port_phyIntrEnable_set);
EXPORT_SYMBOL(rtk_port_phyIntrStatus_get);
EXPORT_SYMBOL(rtk_port_phyIntrMask_get);
EXPORT_SYMBOL(rtk_port_phyIntrMask_set);
EXPORT_SYMBOL(rtk_port_phySdsTestMode_set);
EXPORT_SYMBOL(rtk_port_phySdsTestModeCnt_get);
EXPORT_SYMBOL(rtk_port_phySdsLeq_get);
EXPORT_SYMBOL(rtk_port_phySdsLeq_set);
EXPORT_SYMBOL(rtk_port_phySds_get);
EXPORT_SYMBOL(rtk_port_phySds_set);
EXPORT_SYMBOL(rtk_port_phyCtrl_get);
EXPORT_SYMBOL(rtk_port_phyCtrl_set);
EXPORT_SYMBOL(rtk_port_phyLiteEnable_get);
EXPORT_SYMBOL(rtk_port_phyLiteEnable_set);
EXPORT_SYMBOL(rtk_port_phyDbgCounter_get);
EXPORT_SYMBOL(rtk_eee_portEnable_get);
EXPORT_SYMBOL(rtk_eee_portEnable_set);
EXPORT_SYMBOL(rtk_diag_rtctEnable_set);
#endif

