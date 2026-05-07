
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#ifdef CONFIG_COMMON_RT_API
#include <rtk/rt/rt_rate.h>
#include <rtk/rt/rt_l2.h>
#include <rtk/rt/rt_qos.h>
#include <rtk/rt/rt_epon.h>
#include <rtk/rt/rt_gpon.h>
#include <rtk/rt/rt_switch.h>
#include "rt_rate_ext.h"
#include "rt_igmp_ext.h"
#endif
#if defined(CONFIG_RTL9607C_SERIES)
#include <dal/rtl9607c/dal_rtl9607c_switch.h>
#endif
#include "fc_api.h"
#include "sockmark_define.h"
#include "common.h"

#define FC_PROC "/proc/fc"
#define ENV_EXPORT "/etc/profile.d/fc_env.sh"

enum {RTK_FC_SMP_DISPATCH_TX, RTK_FC_SMP_DISPATCH_RX};

#if defined(CONFIG_RTK_L34_FC_IPI_NIC_TX) || defined(CONFIG_RTK_L34_FC_IPI_NIC_RX)
#define PROC_NIC_SMP_DISPATCH_TX FC_PROC"/ctrl/smp_dispatch_nic_tx"
#define PROC_NIC_SMP_DISPATCH_RX FC_PROC"/ctrl/smp_dispatch_nic_rx"
int rtk_fc_nic_smp_dispatch(int direction)
{
	char buf[128] = {0};
	int dispatch = -1;
	int mib_id = MIB_NIC_SMP_DISPATCH_TX;
	const char *proc = PROC_NIC_SMP_DISPATCH_TX;

	if(direction == RTK_FC_SMP_DISPATCH_RX)
	{
		mib_id = MIB_NIC_SMP_DISPATCH_RX;
		proc = PROC_NIC_SMP_DISPATCH_RX;
	}

	if(mib_get_s2(mib_id, buf, sizeof(buf)))
		dispatch = atoi(buf);
	
	if(dispatch == -2)
	{
		snprintf(buf, sizeof(buf), "echo \"mode 0 cpu 0\" > %s", proc);
		va_cmd(BIN_SH, 2, 1, "-c", buf);
	}
	else if(dispatch >= 0)
	{
		snprintf(buf, sizeof(buf), "echo \"mode 1 cpu %d\" > %s", dispatch % CONFIG_NR_CPUS, proc);
		va_cmd(BIN_SH, 2, 1, "-c", buf);
	}

	return 0;
}
#endif

#if defined(CONFIG_RTK_L34_FC_IPI_WIFI_TX) || defined(CONFIG_RTK_L34_FC_IPI_WIFI_RX)
#define PROC_WLAN_SMP_DISPATCH_TX FC_PROC"/ctrl/smp_dispatch_wifi_tx"
#define PROC_WLAN_SMP_DISPATCH_RX FC_PROC"/ctrl/smp_dispatch_wifi_rx"
int rtk_fc_wlan_smp_dispatch(int direction)
{
	char buf[128] = "";
	int dispatch = -1;
	int mib_id;
	int mib_id_rx[3] = {MIB_WLAN_SMP_DISPATCH_RX, MIB_WLAN1_SMP_DISPATCH_RX, MIB_WLAN2_SMP_DISPATCH_RX};
	int mib_id_tx[3] = {MIB_WLAN_SMP_DISPATCH_TX, MIB_WLAN1_SMP_DISPATCH_TX, MIB_WLAN2_SMP_DISPATCH_TX};
	const char *proc;
	int i;
	
	for(i = 0; i<MAX_WLAN_NUM; i++)
	{		
		if(direction == RTK_FC_SMP_DISPATCH_RX)
		{
			mib_id = mib_id_rx[i];
			proc = PROC_WLAN_SMP_DISPATCH_RX;
		}
		else
		{
			mib_id = mib_id_tx[i];
			proc = PROC_WLAN_SMP_DISPATCH_TX;
		}
		
		dispatch = -1;
		if(mib_get_s2(mib_id, buf, sizeof(buf)))
			dispatch = atoi(buf);
		
		if(dispatch == -2)
		{
			snprintf(buf, sizeof(buf), "echo \"wlan %d mode 0 cpu 0\" > %s", i, proc);
			va_cmd("/bin/sh", 2, 1, "-c", buf);
		}
		else if(dispatch >= 0)
		{
			snprintf(buf, sizeof(buf), "echo \"wlan %d mode 1 cpu %d\" > %s", i, dispatch % CONFIG_NR_CPUS, proc);
			va_cmd("/bin/sh", 2, 1, "-c", buf);
		}
	}
	return 0;
}
#endif

int rtk_fc_ctrl_init(void)
{
	char buf[256]={0};
	unsigned char deviceType = 0;
	unsigned char pon_mode = 0;
	int wanPhyPort = -1, nic_limit_pages=0;
	
	if(mib_get_s2(MIB_PON_MODE, buf, sizeof(buf)))
		pon_mode = (unsigned char)atoi(buf);

	va_cmd(BIN_SH, 2, 1, "-c", "echo -1 > " FC_PROC"/ctrl/skbmark_qid");
	va_cmd(BIN_SH, 2, 1, "-c", "echo -1 > " FC_PROC"/ctrl/skbmark_streamId");
	va_cmd(BIN_SH, 2, 1, "-c", "echo -1 > " FC_PROC"/ctrl/skbmark_streamId_enable");
	va_cmd(BIN_SH, 2, 1, "-c", "echo -1 > " FC_PROC"/ctrl/skbmark_fwdByPs");
	va_cmd(BIN_SH, 2, 1, "-c", "echo -1 > " FC_PROC"/ctrl/skbmark_mibId_enable");
	va_cmd(BIN_SH, 2, 1, "-c", "echo -1 > " FC_PROC"/ctrl/skbmark_meterId");
	va_cmd(BIN_SH, 2, 1, "-c", "echo -1 > " FC_PROC"/ctrl/skbmark_meterId_enable");
	va_cmd(BIN_SH, 2, 1, "-c", "echo -1 > " FC_PROC"/ctrl/skbmark_swshaper_enable");
	va_cmd(BIN_SH, 2, 1, "-c", "echo -1 > " FC_PROC"/ctrl/skbmark_mibId");
	va_cmd(BIN_SH, 2, 1, "-c", "echo -1 > " FC_PROC"/ctrl/skbmark_skipFcFunc");

	sprintf(buf, "echo %d > " FC_PROC"/ctrl/skbmark_qid", SOCK_MARK_QOS_SWQID_START);
	va_cmd(BIN_SH, 2, 1, "-c", buf);
	sprintf(buf, "echo \"#!/bin/sh\" > " ENV_EXPORT);
	va_cmd(BIN_SH, 2, 1, "-c", buf);
	sprintf(buf, "echo skbmark_qid=%d >> " ENV_EXPORT, SOCK_MARK_QOS_SWQID_START);
	va_cmd(BIN_SH, 2, 1, "-c", buf);
	//802.1p Remarking
	sprintf(buf, "echo skbmark_8021p=%d >> " ENV_EXPORT, SOCK_MARK_8021P_START);
	va_cmd(BIN_SH, 2, 1, "-c", buf);
	sprintf(buf, "echo skbmark_8021pEn=%d >> " ENV_EXPORT, SOCK_MARK_8021P_ENABLE_START);
	va_cmd(BIN_SH, 2, 1, "-c", buf);

	//use FC software shaper for Flow limit instead of hw meter policing limit. It look good for TCP rate limit
	sprintf(buf, "echo mark2 %d > " FC_PROC"/ctrl/skbmark_meterId_enable", SOCK_MARK2_METER_HW_INDEX_ENABLE_START);
	va_cmd(BIN_SH, 2, 1, "-c", buf);
	sprintf(buf, "echo meterIdx_en=%d >> " ENV_EXPORT, SOCK_MARK2_METER_HW_INDEX_ENABLE_START);
	va_cmd(BIN_SH, 2, 1, "-c", buf);

	sprintf(buf, "echo mark2 %d > " FC_PROC"/ctrl/skbmark_swshaper_enable", SOCK_MARK2_METER_INDEX_ENABLE_START);
	va_cmd(BIN_SH, 2, 1, "-c", buf);
	sprintf(buf, "echo swShaper_en=%d >> " ENV_EXPORT, SOCK_MARK2_METER_INDEX_ENABLE_START);
	va_cmd(BIN_SH, 2, 1, "-c", buf);

	sprintf(buf, "echo mark2 %d > " FC_PROC"/ctrl/skbmark_meterId", SOCK_MARK2_METER_INDEX_START);
	va_cmd(BIN_SH, 2, 1, "-c", buf);
	sprintf(buf, "echo meterIdx=%d >> " ENV_EXPORT, SOCK_MARK2_METER_INDEX_START);
	va_cmd(BIN_SH, 2, 1, "-c", buf);

	sprintf(buf, "echo mark2 %d > " FC_PROC"/ctrl/skbmark_fwdByPs", SOCK_MARK2_FORWARD_BY_PS_START);
	va_cmd(BIN_SH, 2, 1, "-c", buf);

	sprintf(buf, "echo mark2 %d > " FC_PROC"/ctrl/skbmark_mibId_enable", SOCK_MARK2_MIB_INDEX_ENABLE_START);
	va_cmd(BIN_SH, 2, 1, "-c", buf);

	sprintf(buf, "echo mark2 %d > " FC_PROC"/ctrl/skbmark_mibId", SOCK_MARK2_MIB_INDEX_START);
	va_cmd(BIN_SH, 2, 1, "-c", buf);

	sprintf(buf, "echo mark2 %d > " FC_PROC"/ctrl/skbmark_streamId", SOCK_MARK2_STREAMID_START);
	va_cmd(BIN_SH, 2, 1, "-c", buf);

	sprintf(buf, "echo mark2 %d > " FC_PROC"/ctrl/skbmark_streamId_enable", SOCK_MARK2_STREAMID_ENABLE_START);
	va_cmd(BIN_SH, 2, 1, "-c", buf);

	sprintf(buf, "echo mark2 %d > " FC_PROC"/ctrl/skbmark_skipFcFunc", SOCK_MARK2_SKIP_FC_FUNC_START);
	va_cmd(BIN_SH, 2, 1, "-c", buf);
	
	
	// switch FC host software MIB to payload mode for accurate host rate
	// va_cmd(BIN_SH, 2, 1, "-c", "echo 1 > " FC_PROC"/ctrl/sw_shaperMib_update_mode_host");
	// va_cmd(BIN_SH, 2, 1, "-c", "echo 1 > " FC_PROC"/ctrl/sw_shaperMib_update_mode_flow");
	
	va_cmd(BIN_SH, 2, 1, "-c", "echo dscp 1 > " FC_PROC"/ctrl/flow_hash_input_config");
	va_cmd(BIN_SH, 2, 1, "-c", "echo cpri 1 > " FC_PROC"/ctrl/flow_hash_input_config");
	va_cmd(BIN_SH, 2, 1, "-c", "echo cvid 1 > " FC_PROC"/ctrl/flow_hash_input_config");
	va_cmd(BIN_SH, 2, 1, "-c", "echo svid 1 > " FC_PROC"/ctrl/flow_hash_input_config");
	va_cmd(BIN_SH, 2, 1, "-c", "echo spri 1 > " FC_PROC"/ctrl/flow_hash_input_config");
	
	// for bridge case L3/L4 packet learning as Path 1/2 flow
#ifdef CONFIG_RTK_SOC_RTL8198D
	va_cmd(BIN_SH, 2, 1, "-c", "echo 0 > " FC_PROC"/ctrl/bridge_5tuple_flow_accelerate_by_2tuple");
#else
	va_cmd(BIN_SH, 2, 1, "-c", "echo 1 > " FC_PROC"/ctrl/bridge_5tuple_flow_accelerate_by_2tuple");
#endif
	
	if(pon_mode == EPON_MODE)
		va_cmd(BIN_SH, 2, 1, "-c", "echo 1 > " FC_PROC"/ctrl/epon_llid_format");

	if (deviceType == DEVICE_TYPE_SFU || deviceType == DEVICE_TYPE_HYBRID) //hybrid and SFU mode
		va_cmd(BIN_SH, 2, 1, "-c", "echo 1 > " FC_PROC"/ctrl/l2_port_moving_check_with_vlan");

//	wanPhyPort = rtk_port_get_wan_phyID();
//	if (wanPhyPort != -1) {
//		sprintf(buf, "echo 0x%x > " FC_PROC"/ctrl/wan_port_mask", (1<<wanPhyPort));		// change fc wan port mask
//		va_cmd(BIN_SH, 2, 1, "-c", buf);		// change fc wan port mask
//	}

	/* Remove Port 0 from LAN mask */
    va_cmd(BIN_SH, 2, 1, "-c", "echo 0x3e > " FC_PROC"/ctrl/mac_port_mask_without_cpu");
    /* Set Port 0 as the only WAN port */
    va_cmd(BIN_SH, 2, 1, "-c", "echo 0x01 > " FC_PROC"/ctrl/wan_port_mask");

#ifdef CONFIG_RTK_L34_FC_IPI_NIC_TX
	rtk_fc_nic_smp_dispatch(RTK_FC_SMP_DISPATCH_TX);
#endif
#ifdef CONFIG_RTK_L34_FC_IPI_NIC_RX
	rtk_fc_nic_smp_dispatch(RTK_FC_SMP_DISPATCH_RX);
#endif
#ifdef CONFIG_RTK_L34_FC_IPI_WIFI_TX
	rtk_fc_wlan_smp_dispatch(RTK_FC_SMP_DISPATCH_TX);
#endif
#ifdef CONFIG_RTK_L34_FC_IPI_WIFI_RX
	rtk_fc_wlan_smp_dispatch(RTK_FC_SMP_DISPATCH_RX);
#endif

	if (mib_get_s2(MIB_NIC_MIN_LIMIT_PAGES, (char*)&nic_limit_pages, sizeof(nic_limit_pages))) {
#if defined(CONFIG_RTL9607C_SERIES)
        snprintf(buf, sizeof(buf), "echo %u > /sys/module/re8686_rtl9607c/parameters/min_limit_pages", nic_limit_pages);
        va_cmd(BIN_SH, 2, 1, "-c", buf);
#endif
#if defined(CONFIG_LUNA_G3_SERIES)
        snprintf(buf, sizeof(buf), "echo %u > /sys/module/ca_ne/parameters/min_limit_pages", nic_limit_pages);
        va_cmd(BIN_SH, 2, 1, "-c", buf);
#endif
#if defined(CONFIG_RTL_ETH_RECYCLED_SKB)
        snprintf(buf, sizeof(buf), "echo %u > /sys/module/re_recycleskb/parameters/min_limit_pages", nic_limit_pages);
        va_cmd(BIN_SH, 2, 1, "-c", buf);
#endif
	}
	
	return 0;
}

const char FC_SHARE_METER_INDEX_FILE[] = "/var/fc_share_meter_index_file";
const char FC_SHARE_METER_INDEX_TMP_FILE[] = "/var/fc_share_meter_index_file_temp";

#ifdef CONFIG_COMMON_RT_API
struct rtk_qos_share_meter_entry {
	char alias_name_prefix[48];
	rtk_rate_application_type_t	application_type;
	rt_rate_meter_type_t		meter_type;
};

static struct rtk_qos_share_meter_entry rtk_qos_share_meter_table[] = {
	{"ip_QoS_rate_shaping", RTK_APP_TYPE_IP_QOS_RATE_SHAPING, RT_METER_TYPE_FLOW},
	{"DS_TCP_Syn_rate_limit", RTK_APP_TYPE_DS_TCP_SYN_RATE_LIMIT, RT_METER_TYPE_ACL},
	{"DS_UDP_attack_rate_limit", RTK_APP_TYPE_DS_UDP_ATTACK_RATE_LIMIT, RT_METER_TYPE_ACL},
	{"cmcc_QoS_service_rate_limit", RTK_APP_TYPE_CMCC_QOS_RATE_LIMIT, RT_METER_TYPE_FLOW},
	{"lanhost_upstream_rate_limit_by_fc", RTK_APP_TYPE_LANHOST_US_RATE_LIMIT_BY_FC, RT_METER_TYPE_SW},
	{"lanhost_downstream_rate_limit_by_fc", RTK_APP_TYPE_LANHOST_DS_RATE_LIMIT_BY_FC, RT_METER_TYPE_SW},
	{"DDOS_TCP_Syn_rate_limit", RTK_APP_TYPE_DDOS_TCP_SYN_RATE_LIMIT, RT_METER_TYPE_ACL},
	{"ACL_LIMIT_BANDWIDTH", RTK_APP_TYPE_ACL_LIMIT_BANDWIDTH, RT_METER_TYPE_ACL},
};

int rtk_qos_share_meter_set(rtk_rate_application_type_t application_type, unsigned int index, unsigned int rate)
{
	unsigned int meterIndex, tableIndex;
	FILE *fp;

	for(tableIndex=RTK_APP_TYPE_IP_QOS_RATE_SHAPING ; tableIndex<RTK_APP_TYPE_MAX ; tableIndex++)
	{
		if(rtk_qos_share_meter_table[tableIndex].application_type == application_type)
		{
			if(rt_rate_shareMeterType_add(rtk_qos_share_meter_table[tableIndex].meter_type, &meterIndex) == RT_ERR_OK)
			{
				if(!(fp = fopen(FC_SHARE_METER_INDEX_FILE, "a")))
				{
					fprintf(stderr, "ERROR! %s\n", strerror(errno));
					return -2;
				}
				fprintf(fp, "alis_name=%s_%d meter_index=%d rate=%u\n", rtk_qos_share_meter_table[tableIndex].alias_name_prefix, index, meterIndex, rate);
				fclose(fp);
				if(rt_rate_shareMeterRate_set(meterIndex, rate) == RT_ERR_OK)
					return meterIndex;
				else
				{
					printf("rt_rate_shareMeterRate_set FAIL ! (meter_type=%d,meterIndex=%d,rate=%u)\n", rtk_qos_share_meter_table[tableIndex].meter_type, meterIndex, rate);
					return -1;
				}
			}
			else
			{
				printf("rt_rate_shareMeterType_add FAIL ! (meter_type=%d)\n", rtk_qos_share_meter_table[tableIndex].meter_type);
				return -1;
			}
		}
	}

	return -1;
}

int rtk_qos_share_meter_get(rtk_rate_application_type_t application_type, unsigned int index, unsigned int rate)
{
	unsigned int meterIndex_f, tableIndex,rate_f;
	char line[1024], alisString_f[512], alisString[512];
	FILE *fp;
	for(tableIndex=RTK_APP_TYPE_IP_QOS_RATE_SHAPING ; tableIndex<RTK_APP_TYPE_MAX ; tableIndex++)
	{
		if(rtk_qos_share_meter_table[tableIndex].application_type == application_type)
		{
			if(!(fp = fopen(FC_SHARE_METER_INDEX_FILE, "r")))
				return -2;
			while(fgets(line, 1023, fp) != NULL)
			{
				sscanf(line, "alis_name=%s meter_index=%d rate=%u\n", &alisString_f[0], &meterIndex_f, &rate_f);
				sprintf(alisString, "%s_%d", rtk_qos_share_meter_table[tableIndex].alias_name_prefix, index);
				if(!strcmp(alisString, alisString_f))
				{
					printf("rt_rate_shareMeterType_get already exist !\n");
					fclose(fp);
					return meterIndex_f;
				}
			}
			fclose(fp);
		}
	}
	return -1;
}

int rtk_qos_share_meter_delete(rtk_rate_application_type_t application_type, unsigned int index)
{
	unsigned int meterIndex_f, tableIndex, rate;
	char line[1024], alisString_f[512], alisString[512];
	FILE *fp, *fp_tmp;

	for(tableIndex=RTK_APP_TYPE_IP_QOS_RATE_SHAPING ; tableIndex<RTK_APP_TYPE_MAX ; tableIndex++)
	{
		if(rtk_qos_share_meter_table[tableIndex].application_type == application_type)
		{
			if(!(fp = fopen(FC_SHARE_METER_INDEX_FILE, "r")))
				return -2;

			if(!(fp_tmp = fopen(FC_SHARE_METER_INDEX_TMP_FILE, "w"))){
				fclose(fp);
				return -2;
			}
			while(fgets(line, 1023, fp) != NULL)
			{
				sscanf(line, "alis_name=%s meter_index=%d rate=%u\n", &alisString_f[0], &meterIndex_f, &rate);
				sprintf(alisString, "%s_%d", rtk_qos_share_meter_table[tableIndex].alias_name_prefix, index);
				if(!strcmp(alisString, alisString_f))
				{
					if(rt_rate_shareMeterType_del(meterIndex_f) != RT_ERR_OK)
					{
						printf("rt_rate_shareMeterType_del FAIL !\n");
					}
				}
				else
				{
					fprintf(fp_tmp, "alis_name=%s meter_index=%d rate=%u\n", alisString_f, meterIndex_f, rate);
				}
			}

			fclose(fp);
			fclose(fp_tmp);
			unlink(FC_SHARE_METER_INDEX_FILE);
			if(rename(FC_SHARE_METER_INDEX_TMP_FILE, FC_SHARE_METER_INDEX_FILE) != 0)
			{
				printf("Unable to rename the file: %s %d\n", __func__, __LINE__);
			}
		}
	}
	return 0;
}

int rtk_qos_share_meter_flush(rtk_rate_application_type_t application_type)
{
	unsigned int meterIndex_f, tableIndex, rate;
	char line[1024], alisString_f[512];
	FILE *fp, *fp_tmp;

	for(tableIndex=RTK_APP_TYPE_IP_QOS_RATE_SHAPING ; tableIndex<RTK_APP_TYPE_MAX ; tableIndex++)
	{
		if(rtk_qos_share_meter_table[tableIndex].application_type == application_type)
		{
			if(!(fp = fopen(FC_SHARE_METER_INDEX_FILE, "r")))
				return -2;

			if(!(fp_tmp = fopen(FC_SHARE_METER_INDEX_TMP_FILE, "w"))){
				fclose(fp);
				return -2;
			}
			while(fgets(line, 1023, fp) != NULL)
			{
				sscanf(line, "alis_name=%s meter_index=%d rate=%u\n", &alisString_f[0], &meterIndex_f, &rate);
				if(strstr(alisString_f, rtk_qos_share_meter_table[tableIndex].alias_name_prefix))
				{
					if(rt_rate_shareMeterType_del(meterIndex_f) != RT_ERR_OK)
					{
						printf("rt_rate_shareMeterType_del FAIL !\n");
					}
				}
				else
				{
					fprintf(fp_tmp, "alis_name=%s meter_index=%d rate=%u\n", alisString_f, meterIndex_f, rate);
				}
			}

			fclose(fp);
			fclose(fp_tmp);
			unlink(FC_SHARE_METER_INDEX_FILE);
			if(rename(FC_SHARE_METER_INDEX_TMP_FILE, FC_SHARE_METER_INDEX_FILE) != 0)
			{
				printf("Unable to rename the file: %s %d\n", __func__, __LINE__);
			}
		}
	}
	return 0;
}

int rtk_qos_share_meter_update_rate(rtk_rate_application_type_t application_type, unsigned int index, unsigned int rate)
{
	unsigned int meterIndex_f, tableIndex, rate_f;
	char line[1024], alisString_f[512], alisString[512];
	FILE *fp, *fp_tmp;

	for(tableIndex=RTK_APP_TYPE_IP_QOS_RATE_SHAPING ; tableIndex<RTK_APP_TYPE_MAX ; tableIndex++)
	{
		if(rtk_qos_share_meter_table[tableIndex].application_type == application_type)
		{
			if(!(fp = fopen(FC_SHARE_METER_INDEX_FILE, "r")))
				return -2;

			if(!(fp_tmp = fopen(FC_SHARE_METER_INDEX_TMP_FILE, "w"))){
				fclose(fp);
				return -2;
			}
			while(fgets(line, 1023, fp) != NULL)
			{
				sscanf(line, "alis_name=%s meter_index=%d rate=%u\n", &alisString_f[0], &meterIndex_f, &rate_f);
				sprintf(alisString, "%s_%d", rtk_qos_share_meter_table[tableIndex].alias_name_prefix, index);
				if(!strcmp(alisString, alisString_f))
				{
					if(rt_rate_shareMeterRate_set(meterIndex_f, rate) != RT_ERR_OK)
					{
						printf("rt_rate_shareMeterType_del FAIL !\n");
					}
					fprintf(fp_tmp, "alis_name=%s meter_index=%d rate=%u\n", alisString_f, meterIndex_f, rate);
				}
				else
				{
					fprintf(fp_tmp, "alis_name=%s meter_index=%d rate=%u\n", alisString_f, meterIndex_f, rate_f);
				}
			}

			fclose(fp);
			fclose(fp_tmp);
			unlink(FC_SHARE_METER_INDEX_FILE);
			if(rename(FC_SHARE_METER_INDEX_TMP_FILE, FC_SHARE_METER_INDEX_FILE) != 0)
			{
				printf("Unable to rename the file: %s %d\n", __func__, __LINE__);
			}
		}
	}
	return 0;
}
#endif
