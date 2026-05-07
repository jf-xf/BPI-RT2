#include <drv_types.h>
#include <rtw_debug.h>
#include "rtw_ecos_proc.h"
#include "pci_98d_wfo.h"
#include "../linux/rtw_proc.c"

#include <stdio.h>
#include <stdlib.h>

struct wlan_to_nic_debug_count wlan_to_nic_debug_cnt[6] = {{0}};
unsigned long long extport_abnormal = 0;
unsigned long long total_send_fail = 0;
unsigned long long total_success_fail = 0;

void show_wlan_debug_counter(void)
{
	int i;
	struct net_device *wlan_dev;
	extern void *rtk_wfo_get_net_dev(u8 idx);
	printk("===========================================\n");
	printk("WLAN_TO_ETH:\n");
	printk("  total_send_success: %llu\n", total_send_success);
	printk("  total_send_fail: %llu\n", total_send_fail);
	printk("  macextportid <= 0: %llu\n", extport_abnormal);
	for (i=0;i<6;i++) {
		wlan_dev = rtk_wfo_get_net_dev(i);
		if (wlan_dev != NULL) {
			printk("  %s:\n",wlan_dev->name);
			printk("    success: %llu\n",wlan_to_nic_debug_cnt[i].send_success);
			printk("    drop: %llu\n", wlan_to_nic_debug_cnt[i].send_fail);
		}
	}
	printk("===========================================\n");
}


int rtw_drv_proc_init(void)
{
	return 0;
}

void rtw_adapter_proc_replace(struct net_device *dev)
{
	return;
}

extern u32 wfo_radio_cmd_sender_open_file(struct net_device *, const char *, u32, u8 *);
int rtw_retrieve_from_file(const char *path, u8 *buf, u32 sz)
{
	u32 para_size = 0;

	struct net_device *ndev = dev_get_by_name(NULL, WLAN_WFOVIRT);
	if (ndev == NULL) {
		_dbgdump("In %s, net device NULL\n", __func__);
		return 0;
	}

	para_size = wfo_radio_cmd_sender_open_file(ndev, path, para_size, buf);

	return para_size;
}

extern u32 wfo_radio_cmd_sender_is_file_readable(struct net_device *, const char *);
int rtw_is_file_readable(const char *path)
{
	struct net_device *ndev = dev_get_by_name(NULL, WLAN_WFOVIRT);
	if (ndev == NULL) {
		_dbgdump("In %s, net device NULL\n", __func__);
		return 0;
	}

	if (wfo_radio_cmd_sender_is_file_readable(ndev, path) == 0)
		return 1;
	else
		return 0;
}

void rtw_adapter_proc_init(struct net_device *dev)
{
	return;
}

void rtw_adapter_proc_deinit(struct net_device *dev)
{
	return;
}

static void do_proc_read(struct seq_file *m, struct rtw_proc_hdl* proc_hdl)
{
	proc_hdl->read(m, NULL);
}

static void do_proc_write(struct seq_file *m, struct rtw_proc_hdl* proc_hdl, int argc, char *argv[])
{
	int i, arg_len = 0;
	char buf[128] = {0};

	for (i = 2; i < argc; i++) {
		arg_len = arg_len + strlen(argv[i]) + 1;

		if(arg_len > sizeof(buf))
		{
			_dbgdump("In %s, overrunning array buf, set %d, at most %d\n", __func__, arg_len, 128);
			return;
		}

		strcat(buf, argv[i]);
		if (i != argc-1)
			strcat(buf, " ");
	}

	if (buf[strlen(buf)] < ' ' || buf[strlen(buf)] > '~') {
		buf[strlen(buf)] = '\0';
		arg_len--;
	}
	log_cfg.pass_log.enable_linux_output = 1;
	if (arg_len >= 0) {
		proc_hdl->write(NULL, buf, arg_len, NULL, m->private);
	}
	if (log_cfg.pass_log.rssi_enable == 0 && log_cfg.pass_log.log_level_enable == 0)
		log_cfg.pass_log.enable_linux_output = 0;
}

void rtw_wfo_proc_dispatch(int argc, char *argv[])
{
	unsigned int i = 0;
	char *dev = NULL, *cmd = NULL;
	struct seq_file m;
	struct rtw_proc_hdl* proc_hdl = NULL;
	u8 type = 0, found = 0;

	if (argc < 2) {
		goto fail;
	} else {
		dev = argv[0];
		cmd = argv[1];
	}

	if (strncmp(dev, WLAN_WFOVIRT, 5)==0) {
		unsigned int dev_idx = 0;
		if (strncmp(dev+6, "vap", 3)==0) {
			dev_idx = 1 + atoi(dev+9);
		} else if (strncmp(dev+6, "vxd", 3)==0) {
			dev_idx = CONFIG_LIMITED_VAP_NUM + 1;
		}

		if (dev_idx > CONFIG_LIMITED_VAP_NUM + 1)
			goto fail;

		m.private = rtk_wfo_get_net_dev(dev_idx);
	} else {
		goto fail;
	}

	for (i = 0; i < drv_proc_hdls_num; i++) {
		proc_hdl = &drv_proc_hdls[i];
		if (0 == strcmp(cmd, proc_hdl->name)) {
			found = 1;
			break;
		}
	}
	if (!found) {
		for (i = 0; i < adapter_proc_hdls_num; i++) {
			proc_hdl = &adapter_proc_hdls[i];
			if (0 == strcmp(cmd, proc_hdl->name)) {
				found = 1;
				break;
			}
		}
	}
	if (!found) {
		for (i = 0; i < odm_proc_hdls_num; i++) {
			proc_hdl = &odm_proc_hdls[i];
			if (0 == strcmp(cmd, proc_hdl->name)) {
				found = 1;
				break;
			}
		}
	}

	if (found) {
		if (proc_hdl->read)
			type |= READ;
		if (proc_hdl->write)
			type |= WRITE;

		switch (type) {
		case READ:
			do_proc_read(&m, proc_hdl);
			break;
		case WRITE:
			do_proc_write(&m, proc_hdl, argc, argv);
			break;
		case READ_WRITE:
			if (argc < 3)
				do_proc_read(&m, proc_hdl);
			else
				do_proc_write(&m, proc_hdl, argc, argv);
			break;
		}
	}
	return;

fail:
	_dbgdump("%s(): wrong parameters! dev = %s, cmd = %s\n", __func__, dev, cmd);
}
EXPORT_SYMBOL(rtw_wfo_proc_dispatch);

void rtw_wfo_proc_help(void)
{
	int i;

	_dbgdump("\nwifi6 driver proc command:\n");
	for (i = 0; i < drv_proc_hdls_num; i++) {
		_dbgdump("\t%s \n", drv_proc_hdls[i].name);
	}

	_dbgdump("\nwifi6 adapter proc command:\n");
	for (i = 0; i < adapter_proc_hdls_num; i++) {
		_dbgdump("\t%s \n", adapter_proc_hdls[i].name);
	}

	_dbgdump("\nwifi6 odm proc command:\n");
	for (i = 0; i < odm_proc_hdls_num; i++) {
		_dbgdump("\t%s \n", odm_proc_hdls[i].name);
	}
}

#ifndef CONFIG_WFO_VIRT_DEVNAME
	#error "CONFIG_WFO_VIRT_DEVNAME not define!!!"
#endif

#if (defined(CONFIG_2G_ON_WLAN0) && defined(CONFIG_WFO_OFFLOAD_2G)) || \
	(!defined(CONFIG_2G_ON_WLAN0) && defined(CONFIG_WFO_OFFLOAD_5G))

#define WFO_WLAN_NAME CONFIG_WFO_VIRT_DEVNAME"0"

#elif (defined(CONFIG_2G_ON_WLAN0) && defined(CONFIG_WFO_OFFLOAD_5G)) || \
	(!defined(CONFIG_2G_ON_WLAN0) && defined(CONFIG_WFO_OFFLOAD_2G))

#define WFO_WLAN_NAME CONFIG_WFO_VIRT_DEVNAME"1"

#else
	#error "undef!!"
#endif

/*
input:
	wlan_ifname
output:
return:
	_adapter *
*/
_adapter *rtw_primary_adapters[CONFIG_IFACE_NUMBER] = { NULL, NULL };

_adapter *rtw_get_primary_adapter(unsigned char *wlan_ifname, int index)
{
	_adapter *padapter = NULL;
	struct net_device *pdev = NULL;

	if (index >= CONFIG_IFACE_NUMBER) {
		printk("NO match adapter index for dev (%s)!\n", (char *)wlan_ifname);
		return NULL;
	}

	if (rtw_primary_adapters[index] == NULL) {
		pdev = __dev_get_by_name(&init_net, (char *)wlan_ifname);
		if (pdev != NULL) {
			padapter = (_adapter *)rtw_netdev_priv(pdev);
			if(padapter != NULL)
				rtw_primary_adapters[index] = GET_PRIMARY_ADAPTER(padapter);
			else
				printk("NO match adapter!\n");
		}
		else
			printk("NO match dev(%s)!\n", (char *)wlan_ifname);
	}

	return rtw_primary_adapters[index];
}

int rtw_get_wlan_net_dev_idx_by_mac(unsigned char *mac)
{
	_adapter *primary_adapter = NULL, *tmp_adapter = NULL;
	struct sta_priv *pstapriv = NULL;
	struct sta_info *psta = NULL;
	int i = 0;

	if (rtw_primary_adapters[0] == NULL) {
		primary_adapter = rtw_get_primary_adapter(WFO_WLAN_NAME, 0);
		if(primary_adapter == NULL)
		{
			//printk("NO match dev(%s)!\n", (char *)WFO_WLAN_NAME);
			return (-1);
		}
	}
	else
		primary_adapter = rtw_primary_adapters[0];

	for(i = 0; i < CONFIG_IFACE_NUMBER; i++){
		tmp_adapter = primary_adapter->dvobj->padapters[i];

		if(tmp_adapter && rtw_is_adapter_up(tmp_adapter)){
			pstapriv = &(tmp_adapter->stapriv);
			psta = rtw_get_stainfo(pstapriv, (u8 *)mac);
#ifdef CONFIG_RTW_A4_STA
			if (!psta)
				psta = core_a4_get_fwd_sta(tmp_adapter, (u8 *)mac);
#endif

			if (psta && !(psta->state & WIFI_ASOC_STATE)) {
#ifdef CONFIG_WFA_OFDMA_Logo_Test_Statistic
				psta->os_tx_drop_cnt++;
#endif
				psta = NULL;
			}
			if(psta){
				//return psta; // return struct sta_info pointer
				//return (tmp_adapter->pnetdev); // return struct net_device pointer
				//return (i);
				return tmp_adapter->wfo_mapid;
			}
		}
	}

	//printk("NO match STA(%s)!\n", (char *)WFO_WLAN_NAME);
	return (-1);
}
