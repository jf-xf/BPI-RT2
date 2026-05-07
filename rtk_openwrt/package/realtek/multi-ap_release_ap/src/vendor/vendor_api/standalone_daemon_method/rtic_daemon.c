#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <mqueue.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "map_tlvs.h"
#include "map_utils.h"
#include "map_commands.h"
#include "rtic_util.h"
#include <linux/wireless.h>
#include "vendor_header.h"


#define MAP_VENDOR_MSG_RTIC  VENDOR_CUSTOM_TLV_TYPE_1
#define RTIC_TRIG_AGT_DCS  1
#define MAP_MAX_AGENT_NUMBER	8
#define MAP_CHK_PERIOD	20 /* MAP_CYCLE_PERIOD */
#define MAP_CYCLE_PERIOD 100 /* ms */
#define QUEUE_NANE_MAP2RTIC "/map2rtic_que"
#define QUEUE_NANE_RTIC2MAP "/rtic2map_que"
#define ZWDFS_AGT_REPROBE_PERIOD  5 /* s */
#define ZWDFS_NOCP_WAIT_PERIOD  60 /* s */
#define TP_CHK_TIMES 	5
#define TP_CHK_PERIOD	500 /* ms */

/* 1:controller; 2:agent */
static uint8_t map_role = 0;
/* 0:dcs; 1:zwdfs */
static uint8_t zwdfs_mode = 0;


enum rtic_map_cmd {
	RTIC_MAP_INIT = 0,
	RTIC_MAP_TRIG_SS = 1,
	RTIC_MAP_CHK_SS_STATE = 2,
	RTIC_MAP_CHK_SS_STATE_PROC = 21,
	RTIC_MAP_CHK_SS_STATE_NOT_PROC = 22,
	RTIC_MAP_GET_RTIC_DATA = 3,
	RTIC_MAP_GET_RTIC_DATA_FAIL = 31,
	RTIC_MAP_PROBE_AGT = 4,
	RTIC_MAP_TERMINATE_SS = 5,
	RTIC_MAP_INIT_ZWDFS = 6,
	RTIC_MAP_TRIG_ZWDFS = 7,
	RTIC_MAP_SET_AGT_RD_MON_CH = 8,
	RTIC_MAP_ZWDFS_RD_DETECTED_CH = 9,
	RTIC_MAP_SUGGESTED_CH = 10,
	RTIC_MAP_NOCP_CH = 11,
	RTIC_MAP_USED_CH = 12,
};

enum rtic_map_agt_state {
	RTIC_MAP_AGT_IDLE = 0,
	RTIC_MAP_AGT_SS = 1,
};

enum rtic_map_zwdfs_state {
	RTIC_MAP_Z_ACS = 0,
	RTIC_MAP_Z_ACS_GET_AGNT_RSLT = 1,
	RTIC_MAP_Z_ACS_WAIT_AGNT_RSLT = 2,
	RTIC_MAP_Z_ACS_SEL_AGT_CH = 3,
	RTIC_MAP_Z_NOCP_WAIT = 4,
	RTIC_MAP_Z_RADAR_MON = 5,
};

enum rtic_map_agt_zwdfs_state {
	RTIC_MAP_AGT_Z_IDLE = 0,
	RTIC_MAP_AGT_Z_DFS_MON = 1,
};

struct rtic_map_t {
	uint32_t hop_cnt;
	struct rtic_info_t rtic_data_5g[MAX_CHANNEL_NUM_5G];
	struct rtic_info_t rtic_data_2g[MAX_CHANNEL_NUM_2G];
};

struct rtic_map_agt_t {
	uint8_t mac[6];
	uint8_t probed;
	uint8_t initialed;
	uint8_t updated;
	struct rtic_map_t rtic_data;
};

uint8_t map_weight[5] = {4, 2, 1, 1, 0};

unsigned int band_5g_80M[6][4] =
{
	{36, 40, 44, 48},
	{52, 56, 60, 64},
	{100, 104, 108, 112},
	{116, 120, 124, 128},
	{132, 134, 140, 144},
	{149, 153, 157, 161},
};

static int rtic_init_local(void)
{
	unsigned int rtic_mode = 0;
	unsigned int per_ch_period = RTIC_PER_CH_SCAN_PERIOD;

	if (rtic_get_dcs_trigger_mode(&rtic_mode))
		return RTIC_FAIL;
	
	if (rtic_mode != 1) {
		rtic_mode = 1;
		rtic_set_dcs_trigger_mode(rtic_mode);
	}

	if (rtic_get_dcs_per_ch_period(&per_ch_period))
		return RTIC_FAIL;

	if (per_ch_period != RTIC_PER_CH_SCAN_PERIOD)
		per_ch_period = RTIC_PER_CH_SCAN_PERIOD;
		rtic_dcs_set_per_ch_period(per_ch_period);

	return RTIC_SUCCESS;
}

static int rtic_zwdfs_init_local(struct wlan_chan_t *wlan_info)
{	
	if ((rtic_get_5g_band(wlan_info->ch_num) >= 1) && (rtic_get_5g_band(wlan_info->ch_num) <= 4)) {
		rtic_zwdfs_set_ch_used(wlan_info->ch_num);
	}
	
	rtic_dcs_set_per_ch_period(RTIC_ZWDFS_PER_CH_SCAN_PERIOD);
	rtic_set_bw(wlan_info->bw);
	rtic_zwdfs_set_acs_ratio(2);

	return RTIC_SUCCESS;
}

static void rtic_zwdfs_set_rd_mon_ch(unsigned int bw, unsigned int ch)
{
	char cmd[64];

	sprintf(cmd, "echo %u %u > %s", bw, ch, "/proc/net/rtic/wlan2/set_dfs_ch");
	system(cmd);
	return;
}

static bool rtic_zwdfs_is_the_same_dfsband_40M(unsigned int ch1, unsigned int ch2)
{
	if (ch1 < 52)
		return 0;

	if (ch1 <= 56) {
		if ((ch2 == 52) || (ch2 == 56))
			return 1;
		else
			return 0;
	} else if (ch1 <= 64) {
		if ((ch2 == 60) || (ch2 == 64))
			return 1;
		else
			return 0;
	} else if (ch1 <= 104) {
		if ((ch2 == 100) || (ch2 == 104))
			return 1;
		else
			return 0;
	} else if (ch1 <= 112) {
		if ((ch2 == 108) || (ch2 == 112))
			return 1;
		else
			return 0;
	} else if (ch1 <= 120) {
		if ((ch2 == 116) || (ch2 == 120))
			return 1;
		else
			return 0;
	} else if (ch1 <= 128) {
		if ((ch2 == 124) || (ch2 == 128))
			return 1;
		else
			return 0;
	} else if (ch1 <= 136) {
		if ((ch2 == 132) || (ch2 == 136))
			return 1;
		else
			return 0;
	} else if (ch1 <= 144) {
		if ((ch2 == 140) || (ch2 == 144))
			return 1;
		else
			return 0;
	} else {
		return 0;
	}
}

static unsigned int rtic_get_zwdfs_state(void)
{
	FILE* fp = NULL;
	size_t len = 0;
	char* line = NULL;
	unsigned int state;

	fp = fopen("/proc/net/rtic/wlan2/zwdfs_trig", "r");
	
	if (fp == NULL)
		goto fail;
	
	getline(&line, &len, fp);
	fclose(fp);

	if(line == NULL)
		goto fail;

	if(sscanf(line, "zwdfs_state %u", &state) != 1) {
		printf("rtic_get_zwdfs_state fail proc\n");
		goto fail;
	}
	
	free(line);
	return state;
fail:
	if (fp != NULL)
		fclose(fp);
	if (line != NULL)
		free(line);
	
	printf("get_wlan_chan fail\n");
	return 0;
}

static void rtic_zwdfs_re_trigger(void)
{
	char cmd[64];

	sprintf(cmd, "echo 2 > %s", "/proc/net/rtic/wlan2/zwdfs_trig");
	system(cmd);
	usleep(200*1000);
	sprintf(cmd, "echo 1 > %s", "/proc/net/rtic/wlan2/zwdfs_trig");
	system(cmd);
	return;
}

static unsigned int rtic_get_local_zwdfs_ch(void)
{
	FILE* fp = NULL;
	size_t len = 0;
	char* line = NULL;
	unsigned int ch;

	fp = fopen("/proc/net/rtic/wlan2/now_ch", "r");
	
	if (fp == NULL)
		goto fail;
	
	getline(&line, &len, fp);
	fclose(fp);

	if(line == NULL)
		goto fail;

	if(sscanf(line, "proc_now_ch ch %u", &ch) != 1) {
		printf("rtic_get_zwdfs_ch fail proc\n");
		goto fail;
	}
	
	free(line);
	return ch;
fail:
	if (fp != NULL)
		fclose(fp);
	if (line != NULL)
		free(line);
	
	printf("get_wlan_chan fail\n");
	return 0;
}

static unsigned int rtic_get_zwdfs_rd_det_ch(void)
{
	FILE* fp = NULL;
	size_t len = 0;
	char* line = NULL;
	unsigned int ch;

	fp = fopen("/proc/net/rtic/wlan2/zwdfs_rd_det_ch", "r");
	
	if (fp == NULL)
		goto fail;
	
	getline(&line, &len, fp);
	fclose(fp);

	if(line == NULL)
		goto fail;

	if(sscanf(line, "rd_det_ch %u", &ch) != 1) {
		printf("rtic_get_zwdfs_ch fail proc\n");
		goto fail;
	}
	
	free(line);
	return ch;
fail:
	if (fp != NULL)
		fclose(fp);
	if (line != NULL)
		free(line);
	
	printf("get_wlan_chan fail\n");
	return 0;
}

static void trigger_rd_detected(void)
{
	system("echo 7 > /proc/net/rtic/wlan2/radar_detect");
}

static void trigger_rd_detected_wlan0(void)
{
	system("echo 1 > /proc/net/rtl8852ae/wlan0/radar_detect");
}
#if 0
static bool zwdfs_is_agt_allch_nonocp(struct rtic_map_agt_t *rtic_map_agt_data, uint8_t agt_number)
{
	int i = 0, j = 0;

	for (i = 0; i < agt_number; i++) {
		for (j = 0; j < MAX_CHANNEL_NUM_5G; j++) {
			if ((rtic_map_agt_data[i].rtic_data.rtic_data_5g[j].non_ocp == 0) && (rtic_is_dfs_ch(rtic_map_agt_data[i].rtic_data.rtic_data_5g[j].ch_num)))
				return 0;
		}
	}

	return 1;
}
#endif
static unsigned char is_service_band_scanning_5g(struct wlan_chan_t *wlan_info, unsigned int scanning_ch)
{
	unsigned char i = 0;
	unsigned char band_80M = 0;

	if (wlan_info->bw == CHANNEL_WIDTH_80) {
		if (wlan_info->ch_num <= 48)
			band_80M = 0;
		else if (wlan_info->ch_num <= 64)
			band_80M = 1;
		else if (wlan_info->ch_num <= 112)
			band_80M = 2;
		else if (wlan_info->ch_num <= 128)
			band_80M = 3;
		else if (wlan_info->ch_num <= 144)
			band_80M = 4;
		else if (wlan_info->ch_num <= 161)
			band_80M = 5;

		for (i = 0; i <=3; i++) {
			if (scanning_ch == band_5g_80M[band_80M][i]) {
				return 1;
			}
		}
	} else if (wlan_info->bw == CHANNEL_WIDTH_40) {
		if ((wlan_info->ch_num <= 40) && (scanning_ch <= 40))
			return 1;
		else if ((wlan_info->ch_num <= 48) && (scanning_ch <= 48))
			return 1;
		else if ((wlan_info->ch_num <= 56) && (scanning_ch <= 56))
			return 1;
		else if ((wlan_info->ch_num <= 64) && (scanning_ch <= 64))
			return 1;
		else if ((wlan_info->ch_num <= 104) && (scanning_ch <= 104))
			return 1;
		else if ((wlan_info->ch_num <= 112) && (scanning_ch <= 112))
			return 1;
		else if ((wlan_info->ch_num <= 120) && (scanning_ch <= 120))
			return 1;
		else if ((wlan_info->ch_num <= 128) && (scanning_ch <= 128))
			return 1;
		else if ((wlan_info->ch_num <= 136) && (scanning_ch <= 136))
			return 1;
		else if ((wlan_info->ch_num <= 144) && (scanning_ch <= 144))
			return 1;
		else if ((wlan_info->ch_num <= 153) && (scanning_ch <= 153))
			return 1;
		else if ((wlan_info->ch_num <= 161) && (scanning_ch <= 161))
			return 1;
	} else {
		if (wlan_info->ch_num == scanning_ch) {
			return 1;
		}
	}

	return 0;
}

int is_map_role_controller(void)
{
	int ret = 1;

	if (map_role) {
		if (map_role == 1)
			return 1;
		else if (map_role == 2)
			return 0;
	}
	
	if (access("/tmp/topology_json",0) < 0)
		ret = 0;
	
	return ret;
}

void set_all_agt_unprobe(struct rtic_map_agt_t *list, uint8_t number)
{
	int i;
	for (i = 0; i < number; i++) {
		list[i].probed = 0;
	}	
	return;
}

void remove_agt_from_list(struct rtic_map_agt_t *list, uint8_t number, uint8_t rm_idx)
{
	int i;

	memset(&list[rm_idx], 0, sizeof(struct rtic_map_agt_t));
	for (i = rm_idx; i < (number - 1); i++) {
		memcpy(&list[rm_idx], &list[rm_idx + 1], sizeof(struct rtic_map_agt_t));
		memset(&list[rm_idx + 1], 0, sizeof(struct rtic_map_agt_t));
	}
	return;
}

int is_agt_in_list(struct rtic_map_agt_t *list, uint8_t *mac)
{
	int i;

	for (i = 0; i < MAP_MAX_AGENT_NUMBER; i++) {
		if (memcmp(list[i].mac, mac, 6) == 0)
			return i;
	}
	return -1;
}

int is_agt_all_inited(struct rtic_map_agt_t *list, uint8_t number)
{
	int i;

	for (i = 0; i < number; i++) {
		if (list[i].initialed == 0)
			return -1;
	}
	return 0;	
}

int is_agt_all_updated(struct rtic_map_agt_t *list, uint8_t number)
{
	int i;

	for (i = 0; i < number; i++) {
		if (list[i].updated == 0)
			return -1;
	}
	return 0;	
}

void set_all_agt_non_updated(struct rtic_map_agt_t *list, uint8_t number)
{
	int i;
	for (i = 0; i < number; i++) {
		list[i].updated= 0;
	}
	return;
}

void get_service_ch_info(struct rtic_info_t *dcs, struct rtic_info_t *service_ch_info, uint8_t number)
{
	unsigned int i = 0;
	for (i = 0; i < number; ++i) {
		if (dcs[i].ch_num == service_ch_info->ch_num) {
			memcpy(service_ch_info, &dcs[i], sizeof(struct rtic_info_t));
			break;
		}
	}
	return;
}

void ranking_dcs_data_by_clm_map(struct rtic_info_t *dcs, uint8_t number)
{
	int i, j;
	struct rtic_info_t temp;
	for (i = 0; i < (number - 1); ++i)
		for (j = 0; j < (number - 1 - i); ++j)
			if (dcs[j].clm > dcs[j + 1].clm) {
				memcpy(&temp, &dcs[j], sizeof(struct rtic_info_t));
				memcpy(&dcs[j], &dcs[j+1], sizeof(struct rtic_info_t));
				memcpy(&dcs[j+1], &temp, sizeof(struct rtic_info_t));
			}

	unsigned char max = number;

	if (rtic_dbg) {
		printf(" ch load nhm nhm_dbm clm <===== ranking_dcs_data_by_clm_map\n");
		for (i = 0; i < max; i++) {
			printf("%3u %3u %3u %6d %3u\n", (unsigned int) dcs[i].ch_num, (unsigned int) dcs[i].loading,
				(unsigned int) dcs[i].nhm, (int) dcs[i].nhm_dbm, (unsigned int) dcs[i].clm);
		}
	}
	return;
}


int rtic_map_get_band(void)
{
	FILE * fp;
	int fd;
	size_t len = 0;
	char* line = NULL;
	char* find_str = NULL;
	struct ifreq ifr;
	uint32_t mac[6];
	uint32_t band = 1;
	int i;

	if (is_map_role_controller()) {
		fp = fopen("/tmp/topology_json","r");
		if (fp) {
			getline(&line, &len, fp);
			fclose(fp);
		}
		if (line) {
			find_str = strstr(line, "\"mac_address\"");
			sscanf(find_str, "\"mac_address\":\"%2x%2x%2x%2x%2x%2x\"", 
				&mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
			fd = socket(AF_INET, SOCK_DGRAM, 0);
			if (fd) {
				ifr.ifr_addr.sa_family = AF_INET;
				strncpy(ifr.ifr_name , "wlan0" , IFNAMSIZ-1);
				if (ioctl(fd, SIOCGIFHWADDR, &ifr) >= 0) {
					for (i = 0; i < 6; i++) {
						if ((uint8_t)ifr.ifr_hwaddr.sa_data[i] != mac[i]) {
							band = 1;
							break;
						}
						if (i == 5)
							band = 0;
					}
				}
				close(fd);
			}
		}
	} else {
		fp = fopen("/var/tmp/map_backhaul_link_status","r");
		if (fp) {
			getline(&line, &len, fp);
			fclose(fp);
		}
		
		if (line) {
			sscanf(line, "linkup wlan%u-vxd", &band);	
		}
	}

	if (line != NULL)
		free(line);

	return band;
}

int rtic_map_get_mac(uint8_t *mac)
{
	int fd;
	struct ifreq ifr;
	char iface[6];

	if (rtic_map_get_band() == WLAN0)
		strcpy(iface, "wlan0");
	else if (rtic_map_get_band() == WLAN1)
		strcpy(iface, "wlan1");
	else
		goto fail;
	
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name , iface , IFNAMSIZ-1);
	
	if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
		perror("SIOCGIFHWADDR, Error: ");
		goto fail;
	}
	
	close(fd);

	memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
	
	//printf("Mac : %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n" , mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return 0;
fail:
	if (fd)
		close(fd);
	return ERR_READ_FILE;

}

void rtic_map_log_status(uint8_t rtic_st, uint8_t sub_st, uint32_t time, struct rtic_map_agt_t *agt_list)
{	
	FILE * fp;
	uint32_t i = 0;

	fp = fopen("/var/tmp/map_rtic_log", "w+");
	
	fprintf(fp, "rtic_st %u, sub_st %u, timer %u\n", (uint32_t) rtic_st,(uint32_t) sub_st, time);
	if (is_map_role_controller() && (agt_list != NULL)) {
		fprintf(fp, "agent list\n");
		fprintf(fp, "mac          probed initialed updated\n");
		for (i = 0; i < MAP_MAX_AGENT_NUMBER; i++) {
			fprintf(fp, "%2x%2x%2x%2x%2x%2x %6u %9u %7u\n"
						, agt_list[i].mac[0], agt_list[i].mac[1], agt_list[i].mac[2], agt_list[i].mac[3], agt_list[i].mac[4], agt_list[i].mac[5]
						, (uint32_t) agt_list[i].probed
						, (uint32_t) agt_list[i].initialed
						, (uint32_t) agt_list[i].updated);
		}
	}
	
	fclose(fp);
	
	return;
}

/* controller */
void rtic_map_ctl_trig_agt_dcs(void)
{
	uint8_t target_al_mac[6] = MCAST_1905;
        uint8_t vendor_oui[3]    = { 0x00, 0x11, 0x22 };
        uint8_t relay_indicator  = 1;
        uint16_t data_len;
        unsigned char payload[1024] = { 0 };

	payload[0] = MAP_VENDOR_MSG_RTIC;
	payload[1] = RTIC_TRIG_AGT_DCS;
	payload[2] = 0x00;	
	payload[3] = 0xe0;
	payload[4] = 0x4c;
	payload[5] = 0x86;
	payload[6] = 0x22;
	payload[7] = 0x22;

	data_len = 8*sizeof(char);

	send_vendor_specific_message(target_al_mac, relay_indicator, vendor_oui, data_len, (uint8_t *)payload);

	return;
}

void rtic_map_ctl_cmd(uint8_t cmd, uint8_t *data, uint16_t size)
{
	uint8_t target_al_mac[6] = MCAST_1905;
        uint8_t vendor_oui[3]    = { 0x00, 0x11, 0x22 };
        uint8_t relay_indicator  = 1;
        uint16_t data_len;
        unsigned char payload[1024] = { 0 };

	payload[0] = MAP_VENDOR_MSG_RTIC;
	payload[1] = cmd;
	rtic_map_get_mac(payload + 2);

	if (data != NULL) {
		memcpy(payload + 8, data, size);
		data_len = 8*sizeof(char) + size;
	} else {
		data_len = 8*sizeof(char);
	}
	send_vendor_specific_message(target_al_mac, relay_indicator, vendor_oui, data_len, (uint8_t *)payload);

	return;
}

void compute_map_rtic_data(struct rtic_map_t *rtic_ctl_data, struct rtic_map_agt_t *rtic_map_agt_data, uint8_t agt_number)
{
	int i, j;
	int w_loading, w_clm, w_nhm, w_nhmdb, w_total;
	w_total = map_weight[0];
	for (j = 0; j < agt_number; j ++)
		if (rtic_map_agt_data[j].updated)
			w_total += map_weight[rtic_map_agt_data[j].rtic_data.hop_cnt];

	if (rtic_dbg)
		printf("w_total %d\n", w_total);
	
	if (rtic_map_get_band() == 0) {
		for (i = 0; i < MAX_CHANNEL_NUM_5G; i++) {
			w_loading = map_weight[0]*rtic_ctl_data->rtic_data_5g[i].loading;
			w_clm = map_weight[0]*rtic_ctl_data->rtic_data_5g[i].clm;
			w_nhm = map_weight[0]*rtic_ctl_data->rtic_data_5g[i].nhm;
			w_nhmdb = map_weight[0]*rtic_ctl_data->rtic_data_5g[i].nhm_dbm;
			for (j = 0; j < agt_number; j ++) {
				if (rtic_map_agt_data[j].updated) {
					w_loading += map_weight[rtic_map_agt_data[j].rtic_data.hop_cnt]*(rtic_map_agt_data[j].rtic_data.rtic_data_5g[i].loading);
					w_clm += map_weight[rtic_map_agt_data[j].rtic_data.hop_cnt]*(rtic_map_agt_data[j].rtic_data.rtic_data_5g[i].clm);
					w_nhm += map_weight[rtic_map_agt_data[j].rtic_data.hop_cnt]*(rtic_map_agt_data[j].rtic_data.rtic_data_5g[i].nhm);
					w_nhmdb += map_weight[rtic_map_agt_data[j].rtic_data.hop_cnt]*(rtic_map_agt_data[j].rtic_data.rtic_data_5g[i].nhm_dbm);
				}
			}
			rtic_ctl_data->rtic_data_5g[i].loading = (uint8_t) (w_loading / w_total);
			rtic_ctl_data->rtic_data_5g[i].clm = (uint8_t) (w_clm / w_total);
			rtic_ctl_data->rtic_data_5g[i].nhm = (uint8_t) (w_nhm / w_total);
			rtic_ctl_data->rtic_data_5g[i].nhm_dbm = (uint8_t) (w_nhmdb / w_total);
			
			/* zwdfs, check all agent ch_avail */
			for (j = 0; j < agt_number; j ++) {
				//printf("ch %u, \n", rtic_ctl_data->rtic_data_5g[i].ch_num);
				rtic_ctl_data->rtic_data_5g[i].is_ch_avail &= rtic_map_agt_data[j].rtic_data.rtic_data_5g[i].is_ch_avail;
				//printf("  avail ctl %u, agt %u \n", rtic_ctl_data->rtic_data_5g[i].is_ch_avail, rtic_map_agt_data[j].rtic_data.rtic_data_5g[i].is_ch_avail);
				rtic_ctl_data->rtic_data_5g[i].non_ocp |= rtic_map_agt_data[j].rtic_data.rtic_data_5g[i].non_ocp;
				//printf("  nocp  ctl %u, agt %u \n", rtic_ctl_data->rtic_data_5g[i].non_ocp, rtic_map_agt_data[j].rtic_data.rtic_data_5g[i].non_ocp);
				rtic_ctl_data->rtic_data_5g[i].is_ch_used |= rtic_map_agt_data[j].rtic_data.rtic_data_5g[i].is_ch_used;
				//printf("  used  ctl %u, agt %u \n", rtic_ctl_data->rtic_data_5g[i].is_ch_used, rtic_map_agt_data[j].rtic_data.rtic_data_5g[i].is_ch_used);
	
			}
		}

		for (i = 0; i < MAX_CHANNEL_NUM_2G; i++) {
			w_loading = rtic_ctl_data->rtic_data_2g[i].loading;
			w_clm = rtic_ctl_data->rtic_data_2g[i].clm;
			w_nhm = rtic_ctl_data->rtic_data_2g[i].nhm;
			w_nhmdb = rtic_ctl_data->rtic_data_2g[i].nhm_dbm;
			for (j = 0; j < agt_number; j ++) {
				if (rtic_map_agt_data[j].updated) {
					w_loading += (rtic_map_agt_data[j].rtic_data.rtic_data_2g[i].loading);
					w_clm += (rtic_map_agt_data[j].rtic_data.rtic_data_2g[i].clm);
					w_nhm += (rtic_map_agt_data[j].rtic_data.rtic_data_2g[i].nhm);
					w_nhmdb += (rtic_map_agt_data[j].rtic_data.rtic_data_2g[i].nhm_dbm);
				}
			}
			rtic_ctl_data->rtic_data_2g[i].loading = (uint8_t) (w_loading / (agt_number + 1));
			rtic_ctl_data->rtic_data_2g[i].clm = (uint8_t) (w_clm / (agt_number + 1));
			rtic_ctl_data->rtic_data_2g[i].nhm = (uint8_t) (w_nhm / (agt_number + 1));
			rtic_ctl_data->rtic_data_2g[i].nhm_dbm = (uint8_t) (w_nhmdb / (agt_number + 1));
		}
	} else {
		for (i = 0; i < MAX_CHANNEL_NUM_2G; i++) {
			w_loading = map_weight[0]*rtic_ctl_data->rtic_data_2g[i].loading;
			w_clm = map_weight[0]*rtic_ctl_data->rtic_data_2g[i].clm;
			w_nhm = map_weight[0]*rtic_ctl_data->rtic_data_2g[i].nhm;
			w_nhmdb = map_weight[0]*rtic_ctl_data->rtic_data_2g[i].nhm_dbm;
			for (j = 0; j < agt_number; j ++) {
				if (rtic_map_agt_data[j].updated) {
					w_loading += map_weight[rtic_map_agt_data[j].rtic_data.hop_cnt]*(rtic_map_agt_data[j].rtic_data.rtic_data_2g[i].loading);
					w_clm += map_weight[rtic_map_agt_data[j].rtic_data.hop_cnt]*(rtic_map_agt_data[j].rtic_data.rtic_data_2g[i].clm);
					w_nhm += map_weight[rtic_map_agt_data[j].rtic_data.hop_cnt]*(rtic_map_agt_data[j].rtic_data.rtic_data_2g[i].nhm);
					w_nhmdb += map_weight[rtic_map_agt_data[j].rtic_data.hop_cnt]*(rtic_map_agt_data[j].rtic_data.rtic_data_2g[i].nhm_dbm);
				}
			}
			rtic_ctl_data->rtic_data_2g[i].loading = (uint8_t) (w_loading / w_total);
			rtic_ctl_data->rtic_data_2g[i].clm = (uint8_t) (w_clm / w_total);
			rtic_ctl_data->rtic_data_2g[i].nhm = (uint8_t) (w_nhm / w_total);
			rtic_ctl_data->rtic_data_2g[i].nhm_dbm= (uint8_t) (w_nhmdb / w_total);

		}

		for (i = 0; i < MAX_CHANNEL_NUM_5G; i++) {
			w_loading = rtic_ctl_data->rtic_data_5g[i].loading;
			w_clm = rtic_ctl_data->rtic_data_5g[i].clm;
			w_nhm = rtic_ctl_data->rtic_data_5g[i].nhm;
			w_nhmdb = rtic_ctl_data->rtic_data_5g[i].nhm_dbm;

			for (j = 0; j < agt_number; j ++) {
				if (rtic_map_agt_data[j].updated) {
					w_loading += (rtic_map_agt_data[j].rtic_data.hop_cnt)*(rtic_map_agt_data[j].rtic_data.rtic_data_5g[i].loading);
					w_clm += (rtic_map_agt_data[j].rtic_data.hop_cnt)*(rtic_map_agt_data[j].rtic_data.rtic_data_5g[i].clm);
					w_nhm += (rtic_map_agt_data[j].rtic_data.hop_cnt)*(rtic_map_agt_data[j].rtic_data.rtic_data_5g[i].nhm);
					w_nhmdb += (rtic_map_agt_data[j].rtic_data.hop_cnt)*(rtic_map_agt_data[j].rtic_data.rtic_data_5g[i].nhm_dbm);
				}
			}
			rtic_ctl_data->rtic_data_5g[i].loading = (uint8_t) (w_loading / (agt_number + 1));
			rtic_ctl_data->rtic_data_5g[i].clm = (uint8_t) (w_clm / (agt_number + 1));
			rtic_ctl_data->rtic_data_5g[i].nhm = (uint8_t) (w_nhm / (agt_number + 1));
			rtic_ctl_data->rtic_data_5g[i].nhm_dbm = (uint8_t) (w_nhmdb / (agt_number + 1));
			
			/* zwdfs, check all agent ch_avail */
			for (j = 0; j < agt_number; j ++) {
				rtic_ctl_data->rtic_data_5g[i].is_ch_avail &= rtic_map_agt_data[j].rtic_data.rtic_data_5g[i].is_ch_avail;
				rtic_ctl_data->rtic_data_5g[i].non_ocp |= rtic_map_agt_data[j].rtic_data.rtic_data_5g[i].non_ocp;
				rtic_ctl_data->rtic_data_5g[i].is_ch_used |= rtic_map_agt_data[j].rtic_data.rtic_data_5g[i].is_ch_used;
			}
		}
	}
	return;
}

static bool rtic_zwdfs_is_agt_used_ch_nupt(uint8_t agt_num, struct rtic_map_agt_t *rtic_map_agt_data, uint32_t used_ch)
{
	int i = 0, j = 0;
	if (agt_num == 0)
		return 0;
	if (used_ch == 0)
		return 0;

	for (i = 0; i < MAX_CHANNEL_NUM_5G; i++) {
		for (j = 0; j < agt_num; j ++) {
			if ((rtic_map_agt_data[j].rtic_data.rtic_data_5g[i].ch_num == used_ch) 
				&& (rtic_map_agt_data[j].rtic_data.rtic_data_5g[i].is_ch_used != 1))
				return 1;
		}
	}

	return 0;
}

static unsigned int find_best_dfs_channel(struct rtic_map_t *rtic_ctl_data, struct wlan_chan_t *ch_info)
{
	unsigned int i = 0;
	unsigned int ch = 0;
	unsigned char min_load = 99;
	

#if 0
	printf(" ch load nhm nhm_dbm clm nocp	ret \n");
	for (i = 0; i < MAX_CHANNEL_NUM_5G; i++) {
		printf("%3u %3u %3u %6d %3u %4u\n", (unsigned int) rtic_ctl_data->rtic_data_5g[i].ch_num, (unsigned int) rtic_ctl_data->rtic_data_5g[i].loading,
			(unsigned int) rtic_ctl_data->rtic_data_5g[i].nhm, (int) rtic_ctl_data->rtic_data_5g[i].nhm_dbm, (unsigned int) rtic_ctl_data->rtic_data_5g[i].clm,
			(unsigned int) rtic_ctl_data->rtic_data_5g[i].non_ocp);
	}
#endif

	

	for (i = 0; i < MAX_CHANNEL_NUM_5G; i++) {
		if (rtic_is_dfs_ch(rtic_ctl_data->rtic_data_5g[i].ch_num) 
			&& (rtic_ctl_data->rtic_data_5g[i].is_ch_used == 0)
			&& (rtic_ctl_data->rtic_data_5g[i].non_ocp == 0)) {
			if ((ch_info->bw == CHANNEL_WIDTH_80) && (rtic_get_5g_band(rtic_ctl_data->rtic_data_5g[i].ch_num) == rtic_get_5g_band(ch_info->ch_num))) {
				printf("a %u %u\n", rtic_ctl_data->rtic_data_5g[i].ch_num, ch_info->ch_num);
				continue;
			}
			if ((ch_info->bw == CHANNEL_WIDTH_40) && (rtic_zwdfs_is_the_same_dfsband_40M(rtic_ctl_data->rtic_data_5g[i].ch_num, ch_info->ch_num))) {
				printf("b %u %u\n", rtic_ctl_data->rtic_data_5g[i].ch_num, ch_info->ch_num);
				continue;
			}
			if (rtic_ctl_data->rtic_data_5g[i].loading < min_load) {
				printf("hit %u, %u %u\n", rtic_ctl_data->rtic_data_5g[i].ch_num, ch_info->ch_num, ch_info->bw);
				min_load = rtic_ctl_data->rtic_data_5g[i].loading;
				ch = rtic_ctl_data->rtic_data_5g[i].ch_num;
			}
			
		}
	}
		
	if ((ch >= 36) && (ch <= 165))
		return ch;
	else
		return 0;
}
#if 0
unsigned int find_dfs_mon_channel(struct rtic_map_t *rtic_ctl_data)
{
	unsigned int i = 0;
	unsigned int ch = 0;
	unsigned char min_load = 99;

	for (i = 0; i < MAX_CHANNEL_NUM_5G; i++) {
		if (rtic_is_dfs_ch(rtic_ctl_data->rtic_data_5g[i].ch_num) 
			&& (rtic_ctl_data->rtic_data_5g[i].non_ocp == 0) && (rtic_ctl_data->rtic_data_5g[i].is_ch_used == 0)) {
			if (rtic_ctl_data->rtic_data_5g[i].loading < min_load) {
				min_load = rtic_ctl_data->rtic_data_5g[i].loading;
				ch = rtic_ctl_data->rtic_data_5g[i].ch_num;
				printf("  ch %u load %u min %u\n", rtic_ctl_data->rtic_data_5g[i].ch_num, rtic_ctl_data->rtic_data_5g[i].loading, min_load);
			}
		}
	}

	if ((ch >= 36) && (ch <= 165))
		return ch;
	else
		return 0;
}
#endif

static void update_nocp(struct rtic_info_t *dcs, struct rtic_nocp_info_t *nocp_info)
{
	unsigned int i, j;

	for (i = 0; i < MAX_CHANNEL_NUM_5G; i++) {
		for (j = 0; j < MAX_CHANNEL_NUM_5G; j++) {
			if (dcs[i].ch_num == nocp_info[j].ch_num) {
				dcs[i].non_ocp |= nocp_info[j].is_nocp;
			}
		}
	}
}

/* agent */
void rtic_map_agt_recv_vd_data(void)
{
        struct mq_attr attr;
	//struct map_rtic_data_t map_rtic_data;
	mqd_t mq;
	
        attr.mq_flags   = 0;
        attr.mq_maxmsg  = 20;
        attr.mq_curmsgs = 0;
        attr.mq_msgsize = 1024;

        if ((mqd_t)-1 == (mq = mq_open(QUEUE_NANE_RTIC2MAP, O_RDWR | O_CREAT, 0666, &attr))) {
                perror("mq_open, Error: ");
        }


        if (mq == -1)
                printf("create_mqueue error\n");


        char buffer[1024];
	while (1) {
	        if (-1 == mq_receive(mq, buffer, attr.mq_msgsize, NULL)) {
	                //printf("receive_map_message error\n");
	                continue;
	        }
	        //memcpy(&map_rtic_data, buffer, sizeof(struct map_rtic_data_t));
		//printf("%u %u %d\n", map_rtic_data.rtic_info[0].ch_num, map_rtic_data.rtic_info[0].clm, map_rtic_data.rtic_info[0].nhm_dbm);
		//printf("%u %u %d\n", map_rtic_data.rtic_info[1].ch_num, map_rtic_data.rtic_info[1].clm, map_rtic_data.rtic_info[1].nhm_dbm);
	}

	mq_close(mq);
	sleep(1);
	return;	
}

void rtic_map_agt_reply_ack(uint8_t *target_al_mac, uint8_t cmd)
{
	uint8_t buffer[1024];
	uint8_t relay_indicator  = 0;
	uint8_t vendor_oui[3]    = { 0x00, 0x11, 0x22 };
	uint16_t data_len = 8;
	uint8_t agt_mac[6] = { 0 };
	
	buffer[0] = MAP_VENDOR_MSG_RTIC;
	buffer[1] = cmd;

	rtic_map_get_mac(agt_mac);
		
	memcpy((unsigned char*) (buffer + 2), agt_mac, 6);
	
	send_vendor_specific_message(target_al_mac, relay_indicator, vendor_oui, data_len, buffer);
}

void rtic_map_agt_reply_rtic_data(uint8_t *target_al_mac, struct rtic_map_t *rtic_data)
{
	uint8_t buffer[1024];
	uint8_t relay_indicator  = 0;
	uint8_t vendor_oui[3]    = { 0x00, 0x11, 0x22 };
	uint16_t data_len = 8;
	uint8_t agt_mac[6] = { 0 };
	
	buffer[0] = MAP_VENDOR_MSG_RTIC;
	buffer[1] = RTIC_MAP_GET_RTIC_DATA;

	rtic_map_get_mac(agt_mac);
		
	memcpy((unsigned char*) (buffer + 2), agt_mac, 6);
	memcpy((unsigned char*) (buffer + 8), rtic_data, sizeof(struct rtic_map_t));
	data_len = data_len + sizeof(struct rtic_map_t);
	
	send_vendor_specific_message(target_al_mac, relay_indicator, vendor_oui, data_len, buffer);
}

int rtic_map_agt_get_hop(uint32_t *hop)
{
	FILE* fp = NULL;
	size_t len = 0;
	char* line = NULL;
	
	fp = fopen("/tmp/map_customized_status", "r");
	if (fp == NULL)
		goto fail;
	
	getline(&line, &len, fp);
	fclose(fp);

	if(line == NULL)
		goto fail;

	int ret;

	ret = sscanf(line, "physical = %u", hop);
	
	if (ret < 0) {
		perror("rtic_map_agt_get_hop ");
		goto fail;
	}
	
	free(line);

	return 0;
fail:
	if (fp != NULL)
		fclose(fp);
	if (line != NULL)
		free(line);

	return ERR_READ_FILE;

}

void rtic_controller(struct mq_attr attr, mqd_t mq, char *buffer)
{
	printf("rtic_controller\n");

	static struct rtic_map_agt_t rtic_map_agt_data[MAP_MAX_AGENT_NUMBER];
	static struct rtic_map_t rtic_ctl_data;
	struct wlan_chan_t wlan_info_5g, wlan_info_2g; 
	struct rtic_info_t service_ch_info_5g, service_ch_info_2g;
	struct tp_t tp;
	int i;
	static uint8_t map_cmd = 0, rtic_state = 0, rtic_sub_st = 0, ss_state = 0;
	static uint32_t map_chk_period = 0;
	uint32_t total_period = 0;
	static uint8_t agt_number = 0, tmp_agt_num = 0;
	uint32_t scanning = 0, scanning_ch = 0;
	uint32_t cmd_data;
	uint8_t scan_done_5g = 0;
	uint8_t iface = WLAN0;
	int clm_diff = 0;
	uint32_t suggest_ch_5g = 0, suggest_ch_2g = 0;
	struct rtic_nocp_info_t nocp_info[MAX_CHANNEL_NUM_5G];
	uint32_t tp_accu = 0;
	int ret;

	memset(rtic_map_agt_data, 0, MAP_MAX_AGENT_NUMBER*sizeof(struct rtic_map_agt_t));
	
	while (1) {
		memset(buffer, 0 , 640);
		
	        if (-1 == mq_receive(mq, (char*) buffer, attr.mq_msgsize, NULL)) {
	                //printf("receive_map_message error\n");
	                goto rtic_flow;
	        }

		map_cmd = buffer[1];
	
		switch (map_cmd) {
		case RTIC_MAP_PROBE_AGT:
			if (rtic_dbg)
				printf("ctl RTIC_MAP_PROBE_AGT\n");
			if ((ret = is_agt_in_list(rtic_map_agt_data, (uint8_t*) buffer + 2)) < 0) {
				if (agt_number < MAP_MAX_AGENT_NUMBER) {
					memcpy(rtic_map_agt_data[agt_number].mac, buffer + 2, 6);
					rtic_map_agt_data[agt_number].probed = 1;
					agt_number++;
					if (rtic_dbg)
						printf("add agent\n");
				} else {
					printf("exceed MAP_MAX_AGENT_NUMBER %d\n", MAP_MAX_AGENT_NUMBER);
				}
			} else {
				rtic_map_agt_data[ret].probed = 1;
			}
			break;
		case RTIC_MAP_INIT:
			if (rtic_dbg)
				printf("ctl RTIC_MAP_INIT\n");
			ret = is_agt_in_list(rtic_map_agt_data, (uint8_t*) buffer + 2);
			if (ret >= 0) {
				rtic_map_agt_data[ret].initialed = 1;
			} else {
				if (rtic_dbg)
					printf("RTIC_MAP_INIT agent not in list\n");
			}
			break;
		case RTIC_MAP_TRIG_SS:
			if (rtic_dbg)
				printf("ctl RTIC_MAP_TRIG_SS\n");
			break;
		case RTIC_MAP_GET_RTIC_DATA:
			if (rtic_dbg)
				printf("ctl RTIC_MAP_GET_RTIC_DATA\n");
			ret = is_agt_in_list(rtic_map_agt_data, (uint8_t*) buffer + 2);
			if (ret >= 0) {
				memcpy(&rtic_map_agt_data[ret].rtic_data, buffer + 8, sizeof(struct rtic_map_t));
				rtic_map_agt_data[ret].updated = 1;
			} else {
				//printf("RTIC_MAP_GET_RTIC_DATA agent not in list\n");
			}
			break;
		case RTIC_MAP_GET_RTIC_DATA_FAIL:
			if (rtic_dbg) {
				printf("ctl RTIC_MAP_GET_RTIC_DATA_FAIL\n");
				printf("agent data not ready\n");
			}
			break;
		default :
			printf("unknown controller cmd %u\n", (uint32_t) map_cmd);
			break;
		}
rtic_flow :		
		switch (rtic_state) {
		case RTIC_INIT: 
			if (rtic_sub_st == 0) {
				/* probe agents */
				set_all_agt_unprobe(rtic_map_agt_data, agt_number);
				rtic_map_ctl_cmd(RTIC_MAP_PROBE_AGT, NULL, 0);
				rtic_sub_st = 1;
				map_chk_period = MAP_CHK_PERIOD;
			} else if (rtic_sub_st == 1) {
				/* probe agents done, remove/add agent list */
				if (map_chk_period == 0) {
					tmp_agt_num = agt_number;
					for (i = (tmp_agt_num - 1); i >= 0; i--) {
						if (rtic_map_agt_data[i].probed == 0) {
							if (rtic_dbg)
								printf("rm agent %u\n", i);
							remove_agt_from_list(rtic_map_agt_data, tmp_agt_num, i);
							agt_number--;
						}
					}
					
					if (is_agt_all_inited(rtic_map_agt_data, agt_number) < 0) {
						cmd_data = RTIC_PER_CH_SCAN_PERIOD;
						rtic_map_ctl_cmd(RTIC_MAP_INIT, (uint8_t*) &cmd_data, sizeof(uint32_t));
					}
					rtic_sub_st = 2;
					map_chk_period = MAP_CHK_PERIOD;
				}
			} else {
				/* init agents and controller */
				if ((is_agt_all_inited(rtic_map_agt_data, agt_number) == 0) || (map_chk_period == 0)) {
					ret = rtic_init_local();
					if (ret == RTIC_SUCCESS) {
						rtic_state = RTIC_SS;
						rtic_sub_st = 0;
					}
				}	
			}
			break;
		case RTIC_SS:
			if (ss_state == RTIC_SS_NONE) {
				if (rtic_get_wlan_chan_info(WLAN0, &wlan_info_5g)) {
					printf("rtic_get_wlan_chan_info 5g fail\n");
					break;
				}
				if (rtic_get_wlan_chan_info(WLAN1, &wlan_info_2g)) {
					printf("rtic_get_wlan_chan_info 2g fail\n");
					break;
				}

				rtic_trigger_dcs(WLAN0, wlan_info_5g.bw);
				set_all_agt_non_updated(rtic_map_agt_data, agt_number);
				rtic_map_ctl_cmd(RTIC_MAP_TRIG_SS, NULL, 0);
				iface = WLAN0;
				scan_done_5g = 0;
				ss_state = RTIC_SS_PROCESS;
			} else if (ss_state == RTIC_SS_PROCESS) {
				ret = rtic_get_dcs_status(&scanning, &scanning_ch);
				
				if (ret == 0) {
					if ((scanning == 0) && (scan_done_5g == 0)) {
						scan_done_5g = 1;
						iface = WLAN1;
						rtic_trigger_dcs(iface, wlan_info_2g.bw);
						ss_state = RTIC_SS_PROCESS;
					} else if ((scanning == 0) && (scan_done_5g == 1)) {
						ss_state = RTIC_SS_DONE;
					}
				} else {
					printf("rtic_get_dcs_status fail\n");
				}		
				
				if ((iface == WLAN0) && is_service_band_scanning_5g(&wlan_info_5g, scanning_ch)) {
					if (rtic_get_wlan_tp(iface, &tp) == 0) {
						if (tp.trx >= RTIC_5G_TP_THRESHOLD) {
							if (rtic_dbg)
								printf("dcs is scanning %u %u\n", scanning, scanning_ch);
							ss_state = RTIC_SS_DROP;
						}
					} else {
						printf("rtic_get_wlan_tp fail\n");
					}
				} else {
					if (rtic_get_wlan_tp(iface, &tp) == 0) {
						if (tp.trx >= RTIC_2G_TP_THRESHOLD) {
							if (rtic_dbg)
								printf("dcs is scanning %u %u\n", scanning, scanning_ch);
							ss_state = RTIC_SS_DROP;
						}
					} else {
						printf("rtic_get_wlan_tp fail\n");
					}
				}

			} else if (ss_state == RTIC_SS_DONE) {
				ret = rtic_get_rtic_data(WLAN0, &wlan_info_5g, rtic_ctl_data.rtic_data_5g, MAX_CHANNEL_NUM_5G*sizeof(struct rtic_info_t));
				if (ret != 0) {
					printf("rtic_get_rtic_data 5g fail\n");
				}
				ret = rtic_get_rtic_data(WLAN1, &wlan_info_2g, rtic_ctl_data.rtic_data_2g, MAX_CHANNEL_NUM_2G*sizeof(struct rtic_info_t));
				if (ret != 0) {
					printf("rtic_get_rtic_data 2g fail\n");
				}

				rtic_ctl_data.hop_cnt = 0;
				rtic_state = RTIC_SWITCH_CH;
				ss_state = RTIC_SS_NONE;
			} else if (ss_state == RTIC_SS_DROP) {
				rtic_dcs_terminate();
				usleep(2*RTIC_PER_CH_SCAN_PERIOD*1000);
				if (scan_done_5g == 0)
					rtic_trigger_dcs(WLAN0, wlan_info_5g.bw);
				else
					rtic_trigger_dcs(WLAN1, wlan_info_2g.bw);
					
				ss_state = RTIC_SS_PROCESS;
			} else {
				ss_state = RTIC_SS_NONE;
				printf("RTIC error ss state %u\n", ss_state);
			}
			break;
		case RTIC_SWITCH_CH:
			if (rtic_sub_st == 0) {
				/* gather rtic_data of agents */
				map_chk_period = 3*RTIC_SS_PERIOD/100;
				rtic_map_ctl_cmd(RTIC_MAP_GET_RTIC_DATA, (uint8_t*) &map_chk_period, sizeof(uint32_t));
				rtic_sub_st = 1;
			} else if (rtic_sub_st == 1) {
				/* wait until timeout or all agents updated */
				if (is_agt_all_updated(rtic_map_agt_data, agt_number) == 0) {
					map_chk_period = 0;
					rtic_sub_st = 2;
				} else if (map_chk_period == 0) {
					rtic_map_ctl_cmd(RTIC_MAP_TERMINATE_SS, NULL, 0);
					rtic_sub_st = 2;
				} else {
					/* waiting */
				}
			} else {
				service_ch_info_5g.ch_num = wlan_info_5g.ch_num;
				service_ch_info_2g.ch_num = wlan_info_2g.ch_num;
				get_service_ch_info(rtic_ctl_data.rtic_data_5g, &service_ch_info_5g, MAX_CHANNEL_NUM_5G);
				get_service_ch_info(rtic_ctl_data.rtic_data_2g, &service_ch_info_2g, MAX_CHANNEL_NUM_2G);
				rtic_get_nocp_info(nocp_info);
				compute_map_rtic_data(&rtic_ctl_data, rtic_map_agt_data, agt_number);
				update_nocp(rtic_ctl_data.rtic_data_5g, nocp_info);
				ranking_dcs_data_by_clm_map(rtic_ctl_data.rtic_data_5g, MAX_CHANNEL_NUM_5G);
				ranking_dcs_data_by_clm_map(rtic_ctl_data.rtic_data_2g, MAX_CHANNEL_NUM_2G);
				suggest_ch_5g = 0;
				suggest_ch_2g = 0;
				for (i = 0; i < MAX_CHANNEL_NUM_5G; i++) {
					if (rtic_ctl_data.rtic_data_5g[i].non_ocp == 1)
						continue;
					clm_diff = service_ch_info_5g.clm - rtic_ctl_data.rtic_data_5g[i].clm;
					if (clm_diff > RTIC_5G_CLM_DIFF_THRESHOLD) {
						if (rtic_ctl_data.rtic_data_5g[i].nhm_dbm < RTIC_5G_NHM_THRESHOLD) {
							suggest_ch_5g = (unsigned int) rtic_ctl_data.rtic_data_5g[i].ch_num;
							break;
						}
					}
				}

				for (i = 0; i < MAX_CHANNEL_NUM_2G; i++) {
					clm_diff = service_ch_info_2g.clm - rtic_ctl_data.rtic_data_2g[i].clm;
					if (clm_diff > RTIC_2G_CLM_DIFF_THRESHOLD) {
						if (rtic_ctl_data.rtic_data_2g[i].nhm_dbm < RTIC_2G_NHM_THRESHOLD) {
							suggest_ch_2g = (unsigned int) rtic_ctl_data.rtic_data_2g[i].ch_num;
							break;
						}
					}
				}

				printf("suggest_ch : 5g %u, 2g %u\n", suggest_ch_5g, suggest_ch_2g);
				rtic_update_suggested_ch(WLAN0, suggest_ch_5g);
				rtic_update_suggested_ch(WLAN1, suggest_ch_2g);
				tp_accu = 0;
				for (i = 0; i < TP_CHK_TIMES; i++) {			
					if (rtic_get_wlan_tp(WLAN0, &tp) != 0) {
						printf("rtic_get_wlan_tp fail\n");
						rtic_sub_st = 0;
						rtic_state = RTIC_IDLE;
						break;
					}
					tp_accu += tp.trx;
					printf("%u ", tp.trx);
					usleep(TP_CHK_PERIOD*1000);
				}

				if ((suggest_ch_5g) && (wlan_info_5g.ch_num != suggest_ch_5g)) {
					wlan_info_5g.ch_num = suggest_ch_5g;
					if (tp_accu < RTIC_5G_TP_THRESHOLD)
						rtic_switch_channel(WLAN0, &wlan_info_5g);
				}
				tp_accu = 0;
				for (i = 0; i < TP_CHK_TIMES; i++) {	
					if (rtic_get_wlan_tp(WLAN1, &tp) != 0) {
						printf("rtic_get_wlan_tp fail\n");
						rtic_sub_st = 0;
						rtic_state = RTIC_IDLE;
						break;
					}
					tp_accu += tp.trx;
					usleep(TP_CHK_PERIOD*1000);
				}
                if ((suggest_ch_2g) && (wlan_info_2g.ch_num != suggest_ch_2g)) {
                                        wlan_info_2g.ch_num = suggest_ch_2g;
					if (tp_accu < RTIC_2G_TP_THRESHOLD)
                                        	rtic_switch_channel(WLAN1, &wlan_info_2g);
                }
				rtic_sub_st = 0;
				rtic_state = RTIC_IDLE;
			}
			break;
		case RTIC_IDLE:
			if (total_period > (RTIC_SS_PERIOD/MAP_CYCLE_PERIOD)) {
				rtic_sub_st = 0;
				rtic_state = RTIC_INIT;
				total_period = 0;
			} 
			break;

		default:
			rtic_state = RTIC_INIT;
			//printf("RTIC error state %u, reinit\n", rtic_state);
			break;
		}
		
		total_period++;
		if (map_chk_period > 0)
			map_chk_period --;
		rtic_map_log_status(rtic_state, ss_state, map_chk_period, rtic_map_agt_data);
		rtic_dbg = rtic_get_dbg();
		usleep(MAP_CYCLE_PERIOD*1000);
	}
}

void rtic_zwdfs_controller(struct mq_attr attr, mqd_t mq, char *buffer)
{
	printf("rtic_controller zwdfs\n");

	static struct rtic_map_agt_t rtic_map_agt_data[MAP_MAX_AGENT_NUMBER];
	static struct rtic_map_t rtic_ctl_data;
	struct wlan_chan_t wlan_info_5g; 
	int i;
	static uint8_t map_cmd = 0, rtic_state = 0, rtic_sub_st = 0;
	static uint32_t map_chk_period = 0, tmp_period = 0;
	uint32_t total_period = 0;
	static uint8_t agt_number = 0, tmp_agt_num = 0;
	uint32_t scanning = 0, scanning_ch = 0;
	uint32_t cmd_data[2];
	unsigned int suggest_ch = 0, last_suggest_ch = 166;
	unsigned int zwdfs_used_ch = 0;
	unsigned int radar_det_ch_5g, best_dfs_ch, agt_zw_rd_trig_ch = 0, local_zwdfs_det_ch = 0;
	//struct rtic_info_t dcs_info[MAX_CHANNEL_NUM_5G];
	int ret;

	memset(rtic_map_agt_data, 0, MAP_MAX_AGENT_NUMBER*sizeof(struct rtic_map_agt_t));
	
	while (1) {
		memset(buffer, 0 , 640);
		
	        if (-1 == mq_receive(mq, (char*) buffer, attr.mq_msgsize, NULL)) {
	                //printf("receive_map_message error\n");
	                goto rtic_flow;
	        }

		map_cmd = buffer[1];
	
		switch (map_cmd) {
		case RTIC_MAP_PROBE_AGT:
			if (rtic_dbg)
				printf("ctl RTIC_MAP_PROBE_AGT\n");
			if ((ret = is_agt_in_list(rtic_map_agt_data, (uint8_t*) buffer + 2)) < 0) {
				if (agt_number < MAP_MAX_AGENT_NUMBER) {
					memcpy(rtic_map_agt_data[agt_number].mac, buffer + 2, 6);
					rtic_map_agt_data[agt_number].probed = 1;
					agt_number++;
					if (rtic_dbg)
						printf("add agent\n");
				} else {
					printf("exceed MAP_MAX_AGENT_NUMBER %d\n", MAP_MAX_AGENT_NUMBER);
				}
			} else {
				rtic_map_agt_data[ret].probed = 1;
			}
			break;
		case RTIC_MAP_INIT:
			if (rtic_dbg)
				printf("ctl RTIC_MAP_INIT\n");
			ret = is_agt_in_list(rtic_map_agt_data, (uint8_t*) buffer + 2);
			if (ret >= 0) {
				rtic_map_agt_data[ret].initialed = 1;
			} else {
				if (rtic_dbg)
					printf("RTIC_MAP_INIT agent not in list\n");
			}
			break;
		case RTIC_MAP_TRIG_SS:
			if (rtic_dbg)
				printf("ctl RTIC_MAP_TRIG_SS\n");
			break;
		case RTIC_MAP_GET_RTIC_DATA:
			if (rtic_dbg)
				printf("ctl RTIC_MAP_GET_RTIC_DATA\n");
	
			ret = is_agt_in_list(rtic_map_agt_data, (uint8_t*) buffer + 2);
			if (ret >= 0) {
				memcpy(&rtic_map_agt_data[ret].rtic_data, buffer + 8, sizeof(struct rtic_map_t));
				rtic_map_agt_data[ret].updated = 1;
			} else {
				printf("RTIC_MAP_GET_RTIC_DATA agent not in list\n");
			}
			break;
		case RTIC_MAP_GET_RTIC_DATA_FAIL:
			if (rtic_dbg) {
				printf("ctl RTIC_MAP_GET_RTIC_DATA_FAIL\n");
				printf("agent data not ready\n");
			}
			break;
		case RTIC_MAP_TRIG_ZWDFS:
			if (rtic_dbg)
				printf("ctl RTIC_MAP_TRIG_ZWDFS\n");
			break;
		case RTIC_MAP_SET_AGT_RD_MON_CH:
			if (rtic_dbg)
				printf("ctl RTIC_MAP_SET_AGT_RD_MON_CH\n");
			break;
		case RTIC_MAP_ZWDFS_RD_DETECTED_CH:
			if (1)
				printf("ctl RTIC_MAP_ZWDFS_RD_DETECTED_CH ");
			agt_zw_rd_trig_ch = *((uint32_t*) (buffer + 2 + 6));
			if (1)
				printf("%u\n", agt_zw_rd_trig_ch);	
			break;
		case RTIC_MAP_NOCP_CH:
			radar_det_ch_5g = *((uint32_t*) (buffer + 2 + 6));
			if (rtic_dbg)
				printf("ctl RTIC_MAP_NOCP_CH %u\n", radar_det_ch_5g);
			if (rtic_get_wlan_chan_info(WLAN0, &wlan_info_5g)) {
				printf("map_zwdfs can't get wlan0 info ... err\n");
				break;
			}
			if (radar_det_ch_5g == wlan_info_5g.ch_num)
				trigger_rd_detected_wlan0();
			else
				rtic_zwdfs_update_nonocp(radar_det_ch_5g, wlan_info_5g.bw);
			radar_det_ch_5g = 0;
			break;
		default :
			printf("unknown controller cmd %u\n", (uint32_t) map_cmd);
			break;
		}
rtic_flow :		
		switch (rtic_state) {
		case RTIC_ZWDFS_INIT:
			if (rtic_sub_st == 0) {
				/* probe agents */
				if (rtic_get_wlan_chan_info(WLAN0, &wlan_info_5g)) {
					printf("rtic_get_wlan_chan_info fail @RTIC_ZWDFS_INIT\n");
					break;
				}
				set_all_agt_unprobe(rtic_map_agt_data, agt_number);
				rtic_map_ctl_cmd(RTIC_MAP_PROBE_AGT, NULL, 0);
				rtic_sub_st = 1;
				map_chk_period = MAP_CHK_PERIOD;
			} else if (rtic_sub_st == 1) {
				/* probe agents done, remove/add agent list */
				if (map_chk_period == 0) {
					tmp_agt_num = agt_number;
					for (i = (tmp_agt_num - 1); i >= 0; i--) {
						if (rtic_map_agt_data[i].probed == 0) {
							if (rtic_dbg)
								printf("rm agent %u\n", i);
							remove_agt_from_list(rtic_map_agt_data, tmp_agt_num, i);
							agt_number--;
						}
					}
					
					if (is_agt_all_inited(rtic_map_agt_data, agt_number) < 0) {
						cmd_data[0] = RTIC_ZWDFS_PER_CH_SCAN_PERIOD;
						cmd_data[1] = wlan_info_5g.bw;
						cmd_data[2] = 2; /* acs ratio */
						rtic_map_ctl_cmd(RTIC_MAP_INIT_ZWDFS, (uint8_t*) cmd_data, 3*sizeof(uint32_t));
					}
					rtic_sub_st = 2;
					map_chk_period = MAP_CHK_PERIOD;
				}
			} else {
				/* init agents and controller */
				if ((is_agt_all_inited(rtic_map_agt_data, agt_number) == 0) || (map_chk_period == 0)) {
					ret = rtic_zwdfs_init_local(&wlan_info_5g);
		
					if (ret == RTIC_SUCCESS) {
						if (rtic_get_zwdfs_state() == 0)
							rtic_trigger_zwdfs();
						else
							rtic_zwdfs_re_trigger();
						set_all_agt_non_updated(rtic_map_agt_data, agt_number);
						rtic_map_ctl_cmd(RTIC_MAP_TRIG_ZWDFS, NULL, 0);
						rtic_state = RTIC_ZWDFS_PROCESSING;
						rtic_sub_st = 0;
					}
				}	
			}
			break;
		case RTIC_ZWDFS_PROCESSING:
			if (rtic_get_wlan_chan_info(WLAN0, &wlan_info_5g)) {
				printf("map_zwdfs can't get wlan0 info ... err\n");
				break;
			}
			
			if (rtic_sub_st == RTIC_MAP_Z_ACS) {
				/* wait agents scan done */
				ret = rtic_get_dcs_status(&scanning, &scanning_ch);
				if (scanning_ch != 0)
					break;
				
				map_chk_period = (RTIC_ZWDFS_PER_CH_SCAN_PERIOD*40)/100;
				rtic_sub_st = RTIC_MAP_Z_ACS_GET_AGNT_RSLT;
			} else if (rtic_sub_st == RTIC_MAP_Z_ACS_GET_AGNT_RSLT) {
				if (map_chk_period == 0) {
					if (agt_number)
						rtic_map_ctl_cmd(RTIC_MAP_GET_RTIC_DATA, (uint8_t*) &map_chk_period, sizeof(uint32_t));
					map_chk_period = 10;
					rtic_sub_st = RTIC_MAP_Z_ACS_WAIT_AGNT_RSLT;
				}
			} else if (rtic_sub_st == RTIC_MAP_Z_ACS_WAIT_AGNT_RSLT) {
				/* wait until timeout or all agents updated */
				if (map_chk_period == 0) {
					if (is_agt_all_updated(rtic_map_agt_data, agt_number) == 0) {
						printf("agt_all_updated\n");
						map_chk_period = 0;
						rtic_sub_st = RTIC_MAP_Z_ACS_SEL_AGT_CH;
					} else if (map_chk_period == 0) {
						printf("RTIC_MAP_Z_ACS_WAIT_AGNT_RSLT map_chk_period == 0\n");
						/* agent drop */
						rtic_sub_st = RTIC_MAP_Z_ACS_SEL_AGT_CH;
					} else {
						/* waiting */
					}
				}
			} else if (rtic_sub_st == RTIC_MAP_Z_ACS_SEL_AGT_CH) {
				rtic_get_rtic_data(WLAN0, &wlan_info_5g, rtic_ctl_data.rtic_data_5g, MAX_CHANNEL_NUM_5G*sizeof(struct rtic_info_t));
				compute_map_rtic_data(&rtic_ctl_data, rtic_map_agt_data, agt_number);
				best_dfs_ch = find_best_dfs_channel(&rtic_ctl_data, &wlan_info_5g);
				
				if (best_dfs_ch == 0) {
					/* all channel non-ocp */
					map_chk_period = ZWDFS_NOCP_WAIT_PERIOD*10;
					rtic_sub_st = RTIC_MAP_Z_NOCP_WAIT;
					break;
				}
				
				if (wlan_info_5g.ch_num != best_dfs_ch)
					rtic_zwdfs_set_rd_mon_ch(wlan_info_5g.bw, best_dfs_ch);
				cmd_data[0] = best_dfs_ch;
				cmd_data[1] = wlan_info_5g.bw;
				rtic_zwdfs_set_rd_mon_ch(wlan_info_5g.bw, best_dfs_ch);
				if (agt_number) {
					rtic_map_ctl_cmd(RTIC_MAP_SET_AGT_RD_MON_CH, (uint8_t*) cmd_data, 2*sizeof(uint32_t));
					if (rtic_dbg)
						printf("set mon ch %u bw %u\n", best_dfs_ch, wlan_info_5g.bw);
				}
				map_chk_period = ZWDFS_AGT_REPROBE_PERIOD*10;
				rtic_sub_st = RTIC_MAP_Z_RADAR_MON;
			} else if (rtic_sub_st == RTIC_MAP_Z_NOCP_WAIT) {
				if (map_chk_period == 0) {
					rtic_sub_st = RTIC_MAP_Z_ACS;
					break;
				}

				if ((map_chk_period % (ZWDFS_AGT_REPROBE_PERIOD*10)) == 0) {
					tmp_period = map_chk_period - ZWDFS_AGT_REPROBE_PERIOD;
					set_all_agt_unprobe(rtic_map_agt_data, agt_number);
					rtic_map_ctl_cmd(RTIC_MAP_PROBE_AGT, NULL, 0);
					tmp_agt_num = agt_number;
				} else if (map_chk_period == tmp_period) {
					tmp_period = 0;
					/* probe agents done, remove/add agent list */
					tmp_agt_num = agt_number;
					for (i = (tmp_agt_num - 1); i >= 0; i--) {
						if (rtic_map_agt_data[i].probed == 0) {
							printf("rm agent %u\n", i);
							remove_agt_from_list(rtic_map_agt_data, tmp_agt_num, i);
							agt_number--;
						}
					}
					
					if (is_agt_all_inited(rtic_map_agt_data, agt_number) < 0) {
						printf("found new agent, init agent\n");
						rtic_set_rdy_ch(0);
						rtic_update_suggested_ch(WLAN0, 0);
						rtic_state = RTIC_ZWDFS_INIT;
						rtic_sub_st = 1;
						break;
					}
				}
				/* re-probe agents */
			} else if (rtic_sub_st == RTIC_MAP_Z_RADAR_MON) {
				/* re-probe agents */
				if (map_chk_period == 0) {
					map_chk_period = ZWDFS_AGT_REPROBE_PERIOD*10;
					set_all_agt_unprobe(rtic_map_agt_data, agt_number);
					rtic_map_ctl_cmd(RTIC_MAP_PROBE_AGT, NULL, 0);
				} else if (agt_number != tmp_agt_num) {
					/* probe agents done, remove/add agent list */
					tmp_agt_num = agt_number;
					for (i = (tmp_agt_num - 1); i >= 0; i--) {
						if (rtic_map_agt_data[i].probed == 0) {
							if (rtic_dbg)
								printf("rm agent %u\n", i);
							remove_agt_from_list(rtic_map_agt_data, tmp_agt_num, i);
							agt_number--;
						}
					}
					
					if (is_agt_all_inited(rtic_map_agt_data, agt_number) < 0) {
						printf("found new agent, init agent\n");
						rtic_set_rdy_ch(0);
						rtic_update_suggested_ch(WLAN0, 0);
						rtic_state = RTIC_ZWDFS_INIT;
						rtic_sub_st = 1;
						break;
					}
				}
				/* re-probe agents */
				
				zwdfs_used_ch = 0;
				suggest_ch = 0;

				if (RTIC_SUCCESS == rtic_get_rd_det_ch(&radar_det_ch_5g)) {
					if (radar_det_ch_5g != 0) {
						printf("radar_det_ch_5g %u\n", radar_det_ch_5g);
						rtic_zwdfs_update_nonocp(radar_det_ch_5g, wlan_info_5g.bw);
						cmd_data[0] = radar_det_ch_5g;
						rtic_map_ctl_cmd(RTIC_MAP_NOCP_CH, (uint8_t*) cmd_data, sizeof(uint32_t));
					}
				}

				if ((agt_number) && ((map_chk_period % (ZWDFS_AGT_REPROBE_PERIOD*2)) == 0))
					rtic_map_ctl_cmd(RTIC_MAP_GET_RTIC_DATA, (uint8_t*) &map_chk_period, sizeof(uint32_t));

				/* agent detect radar */
				if (agt_zw_rd_trig_ch != 0) {
					if (agt_zw_rd_trig_ch == rtic_get_local_zwdfs_ch()) {
						printf("agent detect radar\n");
						trigger_rd_detected();
						agt_zw_rd_trig_ch = 0;
						rtic_update_suggested_ch(WLAN0, 0);
						rtic_set_rdy_ch(0);
						rtic_sub_st = RTIC_MAP_Z_ACS;
						break;
					} else {
						printf("zwdfs error : unexpected agent radar channel %u\n", agt_zw_rd_trig_ch);
						agt_zw_rd_trig_ch = 0;
					}
				}

				//rtic_get_rtic_data(WLAN0, &wlan_info_5g, rtic_ctl_data.rtic_data_5g, MAX_CHANNEL_NUM_5G*sizeof(struct rtic_info_t));
				
				local_zwdfs_det_ch = rtic_get_zwdfs_rd_det_ch();

				if ((agt_number) && (local_zwdfs_det_ch != 0)) {
					/* RTIC detected radar -> apprise controller */
					printf("local zwdfs radar detected\n");
					cmd_data[0] = local_zwdfs_det_ch;
					rtic_map_ctl_cmd(RTIC_MAP_ZWDFS_RD_DETECTED_CH, (uint8_t*) cmd_data, sizeof(uint32_t));
					rtic_update_suggested_ch(WLAN0, 0);
					rtic_set_rdy_ch(0);
					rtic_sub_st = RTIC_MAP_Z_ACS;
					break;
				}
				
				if (RTIC_SUCCESS == rtic_get_rtic_data(WLAN0, &wlan_info_5g, rtic_ctl_data.rtic_data_5g, MAX_CHANNEL_NUM_5G*sizeof(struct rtic_info_t))) {
					compute_map_rtic_data(&rtic_ctl_data, rtic_map_agt_data, agt_number);
					
					for (i = 0; i < MAX_CHANNEL_NUM_5G; i++) {
						
						if ((rtic_ctl_data.rtic_data_5g[i].is_ch_avail == 1) && (rtic_is_dfs_ch(rtic_ctl_data.rtic_data_5g[i].ch_num))) {
								suggest_ch = rtic_ctl_data.rtic_data_5g[i].ch_num;
								if (rtic_dbg)
									printf("suggest_ch %u\n", suggest_ch);
						}
						
						if (rtic_ctl_data.rtic_data_5g[i].is_ch_used == 1) {
							//printf("zwdfs_data ch %u using\n", rtic_ctl_data.rtic_data_5g[i].ch_num);
							zwdfs_used_ch = rtic_ctl_data.rtic_data_5g[i].ch_num;
						}
						#if 0
						printf("%3u %3u %3u %6d %3u %4u %4u\n", (unsigned int) rtic_ctl_data.rtic_data_5g[i].ch_num, (unsigned int) rtic_ctl_data.rtic_data_5g[i].loading,
						(unsigned int) rtic_ctl_data.rtic_data_5g[i].nhm, (int) rtic_ctl_data.rtic_data_5g[i].nhm_dbm, (unsigned int) rtic_ctl_data.rtic_data_5g[i].clm,
						(unsigned int) rtic_ctl_data.rtic_data_5g[i].non_ocp, (unsigned int) rtic_ctl_data.rtic_data_5g[i].is_ch_used);
						#endif
					}
					
					if ((wlan_info_5g.bw != CHANNEL_WIDTH_20) && (suggest_ch == 165)) 
						suggest_ch = 0;		

					if (last_suggest_ch != suggest_ch) {
						printf("zero-wait DFS sug_ch %u\n", suggest_ch);
						last_suggest_ch = suggest_ch;
						/* TODO: update suggested channel to agents */
						rtic_update_suggested_ch(WLAN0, suggest_ch);
						cmd_data[0] = suggest_ch;
						rtic_map_ctl_cmd(RTIC_MAP_SUGGESTED_CH, (uint8_t*) cmd_data, sizeof(uint32_t));
						rtic_set_rdy_ch(suggest_ch);
					}
					
					//printf("ch %u ready\n", (unsigned int) rtic_ctl_data.rtic_data_5g[i].ch_num);

					if ((zwdfs_used_ch != 0) && (wlan_info_5g.ch_num != zwdfs_used_ch)) {
						if (rtic_dbg)
							printf("now_used %u rtic_data_used_ch %u\n", wlan_info_5g.ch_num, zwdfs_used_ch);
						/* TODO: update used channel to agents */
						rtic_zwdfs_set_ch_used(wlan_info_5g.ch_num);
						cmd_data[0] = wlan_info_5g.ch_num;
						if (agt_number)
							rtic_map_ctl_cmd(RTIC_MAP_USED_CH, (uint8_t*) cmd_data, sizeof(uint32_t));
						rtic_update_suggested_ch(WLAN0, 0);
						rtic_set_rdy_ch(0);
						rtic_sub_st = RTIC_MAP_Z_ACS;
					}

					if (rtic_zwdfs_is_agt_used_ch_nupt(agt_number, rtic_map_agt_data, zwdfs_used_ch) && ((map_chk_period % 5) == 0)) {
						cmd_data[0] = zwdfs_used_ch;
						rtic_map_ctl_cmd(RTIC_MAP_USED_CH, (uint8_t*) cmd_data, sizeof(uint32_t));
					}
				}
				//usleep(MAP_CYCLE_PERIOD*10000);
			}
			break;
		default:
			rtic_state = RTIC_ZWDFS_INIT;
			printf("RTIC error state %u, reinit\n", rtic_state);
			break;
		}
		
		total_period++;
		if (map_chk_period > 0)
			map_chk_period --;
		rtic_map_log_status(rtic_state, rtic_sub_st, map_chk_period, rtic_map_agt_data);
		rtic_dbg = rtic_get_dbg();
		usleep(MAP_CYCLE_PERIOD*1000);
	}
}


void rtic_agent(struct mq_attr attr, mqd_t mq ,char *buffer)
{
	uint8_t target_al_mac[6] = { 0 };
	uint8_t controller_cmd = 0;
	static uint8_t ag_state = 0, ss_state = 0;
	uint32_t per_ch_period = 100;
	uint32_t scanning = 0, scanning_ch = 0;
	struct tp_t tp;
	uint8_t scan_done_5g = 0;
	uint8_t iface;
	uint32_t wait_period = 0;
	static struct wlan_chan_t wlan_info_5g; 
	static struct wlan_chan_t wlan_info_2g;
	static struct rtic_info_t dcs_info_5g[MAX_CHANNEL_NUM_5G];
	static struct rtic_info_t dcs_info_2g[MAX_CHANNEL_NUM_2G];
	static struct rtic_map_t rtic_map;
	int ret;

	printf("rtic_agent\n");

	while (1) {
		memset(buffer, 0 , 640);
	        if (-1 == mq_receive(mq, buffer, attr.mq_msgsize, NULL)) {
	                //printf("receive_map_message error\n");
	                goto rtic_flow;
	        }

		controller_cmd = buffer[1];
		memcpy(target_al_mac, (unsigned char*) (buffer + 2), 6);

		/* recevice controller commands */
		switch (controller_cmd) {
		case RTIC_MAP_INIT :
			if (rtic_dbg)
				printf("agent RTIC_MAP_INIT\n");
			per_ch_period = *((uint32_t*) (buffer + 2 + 6));
			if (per_ch_period > 10000)
				per_ch_period = 10000;
			else if (per_ch_period < 100)
				per_ch_period = 100;
			rtic_dcs_set_per_ch_period(per_ch_period);
			rtic_map_agt_reply_ack(target_al_mac, RTIC_MAP_INIT);
			break;
		case RTIC_MAP_TRIG_SS :
			if (rtic_dbg)
				printf("agent RTIC_MAP_TRIG_SS\n");
			if (ag_state == RTIC_MAP_AGT_IDLE) {
				ag_state = RTIC_MAP_AGT_SS;
			}
			rtic_map_agt_reply_ack(target_al_mac, RTIC_MAP_TRIG_SS);
			break;
		case RTIC_MAP_CHK_SS_STATE :
			if (rtic_dbg)
				printf("agent RTIC_MAP_CHK_SS_STATE\n");
			if ((ss_state == RTIC_SS_NONE) && (ag_state == RTIC_MAP_AGT_IDLE)) {
				rtic_map_agt_reply_ack(target_al_mac, RTIC_MAP_CHK_SS_STATE_NOT_PROC);
			} else {
				rtic_map_agt_reply_ack(target_al_mac, RTIC_MAP_CHK_SS_STATE_PROC);
			}
			break;
		case RTIC_MAP_GET_RTIC_DATA :
			if (rtic_dbg)
				printf("agent RTIC_MAP_GET_RTIC_DATA\n");
			if (ag_state == RTIC_MAP_AGT_IDLE) {
				//wait_period = *((uint32_t*) (buffer + 2 + 6));
				rtic_map_agt_reply_rtic_data(target_al_mac, &rtic_map);
			} else {
				wait_period = *((uint32_t*) (buffer + 2 + 6));
				rtic_map_agt_reply_ack(target_al_mac, RTIC_MAP_GET_RTIC_DATA_FAIL);
			}
			break;
		case RTIC_MAP_PROBE_AGT :
			if (rtic_dbg)
				printf("RTIC_MAP_PROBE_AGT\n");
			rtic_map_agt_reply_ack(target_al_mac, RTIC_MAP_PROBE_AGT);
			break;
		case RTIC_MAP_TERMINATE_SS :
			if (rtic_dbg)
				printf("RTIC_MAP_TERMINATE_SS\n");
			rtic_dcs_terminate();
			usleep(2*per_ch_period*1000);
			ag_state = RTIC_MAP_AGT_IDLE;
			ss_state = RTIC_SS_NONE;
			break;
		default :
			printf("unknown controller cmd %u\n", (uint32_t) controller_cmd);
			break;
		}

rtic_flow :
		/* RTIC state of agent */
		switch (ag_state) {
		case RTIC_MAP_AGT_IDLE :
			break;
		case RTIC_MAP_AGT_SS :
			if (ss_state == RTIC_SS_NONE) {
				ret = rtic_get_dcs_status(&scanning, &scanning_ch);
				if (ret != 0) {
					printf("rtic_get_dcs_status fail\n");
					break;
				}
				if (scanning == 1) {
					//printf("5g wait... dcs is scanning %u %u\n", scanning, scanning_ch);
					break;
				}
				
				ret = rtic_get_wlan_chan_info(WLAN0, &wlan_info_5g);
				if (ret) {
					printf("rtic_get_wlan_chan_info 5g fail\n");
					break;
				}
				ret = rtic_get_wlan_chan_info(WLAN1, &wlan_info_2g);
				if (ret) {
					printf("rtic_get_wlan_chan_info 2g fail\n");
					break;
				}
				iface = WLAN0;
				scan_done_5g = 0;
				rtic_trigger_dcs(iface, wlan_info_5g.bw);

				ss_state = RTIC_SS_PROCESS;
				
			} else if (ss_state == RTIC_SS_PROCESS) {
				ret = rtic_get_dcs_status(&scanning, &scanning_ch);
				
				if (ret == 0) {
					if ((scanning == 0) && (scan_done_5g == 0)) {
						scan_done_5g = 1;
						iface = WLAN1;
						if (rtic_dbg)
							printf("scan_done_5g = 1 scanning %u\n", scanning);
						rtic_trigger_dcs(iface, wlan_info_2g.bw);
						ss_state = RTIC_SS_PROCESS;
					} else if ((scanning == 0) && (scan_done_5g == 1)) {
						ss_state = RTIC_SS_DONE;
					}
				} else {
					printf("rtic_get_dcs_status fail\n");
				}		
				
				if ((iface == WLAN0) && is_service_band_scanning_5g(&wlan_info_5g, scanning_ch)) {
					if (rtic_get_wlan_tp(iface, &tp) == 0) {
						if (tp.trx >= RTIC_5G_TP_THRESHOLD) {
							printf("dcs is scanning %u %u\n", scanning, scanning_ch);
							ss_state = RTIC_SS_DROP;
						}
					} else {
						printf("rtic_get_wlan_tp fail\n");
					}
				} else {
					if (rtic_get_wlan_tp(iface, &tp) == 0) {
						if (tp.trx >= RTIC_2G_TP_THRESHOLD) {
							printf("dcs is scanning %u %u\n", scanning, scanning_ch);
							ss_state = RTIC_SS_DROP;
						}
					} else {
						printf("rtic_get_wlan_tp fail\n");
					}
				}
			} else if (ss_state == RTIC_SS_DONE) {
				ret = rtic_get_rtic_data(WLAN0, &wlan_info_5g, dcs_info_5g, MAX_CHANNEL_NUM_5G*sizeof(struct rtic_info_t));
				if (ret != 0) {
					printf("rtic_get_rtic_data 5g fail\n");
				}
				ret = rtic_get_rtic_data(WLAN1, &wlan_info_2g, dcs_info_2g, MAX_CHANNEL_NUM_2G*sizeof(struct rtic_info_t));
				if (ret != 0) {
					printf("rtic_get_rtic_data 2g fail\n");
				}

				memcpy(&rtic_map.rtic_data_2g, dcs_info_2g, MAX_CHANNEL_NUM_2G*sizeof(struct rtic_info_t));
				memcpy(&rtic_map.rtic_data_5g, dcs_info_5g, MAX_CHANNEL_NUM_5G*sizeof(struct rtic_info_t));
				if (rtic_map_agt_get_hop(&rtic_map.hop_cnt))
					rtic_map.hop_cnt = 0;
				
				if (wait_period) {
					if (rtic_map_agt_get_hop(&rtic_map.hop_cnt))
						rtic_map.hop_cnt = 0;
					rtic_map_agt_reply_rtic_data(target_al_mac, &rtic_map);
					wait_period = 0;
				}
					
				ss_state = RTIC_SS_NONE;
				ag_state = RTIC_MAP_AGT_IDLE;

			} else if (ss_state == RTIC_SS_DROP) {
				rtic_dcs_terminate();
				usleep(2*per_ch_period*1000);
				if (scan_done_5g == 0)
					rtic_trigger_dcs(WLAN0, wlan_info_5g.bw);
				else
					rtic_trigger_dcs(WLAN1, wlan_info_2g.bw);

				ss_state = RTIC_SS_PROCESS;
			} else {
				ss_state = RTIC_SS_NONE;
				printf("RTIC error ss state %u\n", ss_state);
			}
			//printf("ss_state %u ag_state %u\n", ss_state, ag_state);
			break;
		default :
			printf("unknown agent state %u\n", (uint32_t) ag_state);
			break;
		}

	if (wait_period)
		wait_period--;
	rtic_map_log_status(ag_state, ss_state, wait_period, NULL);
	rtic_dbg = rtic_get_dbg();
	usleep(MAP_CYCLE_PERIOD*1000);
	}
}

void rtic_zwdfs_agent(struct mq_attr attr, mqd_t mq ,char *buffer)
{
	uint8_t target_al_mac[6] = { 0 };
	uint8_t controller_cmd = 0;
	static uint8_t ag_state = 0, ss_state = 0;
	uint32_t per_ch_period = 100;
	uint32_t bw = 2;
	uint32_t acs_ratio = 2;
	uint32_t scanning = 0, scanning_ch = 0;
	uint32_t ctl_assigned_ch = 0;
	uint32_t wait_period = 0;
	bool zwdfs_agt_inited = 0;
	static struct wlan_chan_t wlan_info_5g; 
	static struct rtic_map_t rtic_map;
	unsigned int radar_det_ch_5g, so_zw_rd_trig_ch = 0, agt_zwdfs_det_ch = 0, sug_ch = 0, nocp_ch = 0, used_ch = 0;
	uint32_t cmd_data[2];
	
	printf("rtic_agent\n");

	while (1) {
		memset(buffer, 0 , 640);
	        if (-1 == mq_receive(mq, buffer, attr.mq_msgsize, NULL)) {
	                //printf("receive_map_message error\n");
	                goto rtic_flow;
	        }

		controller_cmd = buffer[1];
		memcpy(target_al_mac, (unsigned char*) (buffer + 2), 6);

		/* recevice controller commands */
		switch (controller_cmd) {
		case RTIC_MAP_INIT_ZWDFS :
			if (rtic_dbg)
				printf("agent RTIC_MAP_INIT\n");
			per_ch_period = *((uint32_t*) (buffer + 2 + 6));
			bw = *((uint32_t*) (buffer + 2 + 6 + 4));
			acs_ratio = *((uint32_t*) (buffer + 2 + 6 + 8));
			if (per_ch_period > 10000)
				per_ch_period = 10000;
			else if (per_ch_period < 100)
				per_ch_period = 100;
			printf("init period %u, bw %u, acs_r %u\n", per_ch_period, bw, acs_ratio);
			rtic_dcs_set_per_ch_period(per_ch_period);
			rtic_set_bw(bw);
			rtic_zwdfs_set_acs_ratio(acs_ratio);
			rtic_map_agt_reply_ack(target_al_mac, RTIC_MAP_INIT);
			zwdfs_agt_inited = 1;
			ag_state = RTIC_MAP_AGT_Z_DFS_MON;
			break;
		case RTIC_MAP_TRIG_SS :
			if (rtic_dbg)
				printf("agent RTIC_MAP_TRIG_SS\n");
			if (ag_state == RTIC_MAP_AGT_IDLE) {
				ag_state = RTIC_MAP_AGT_SS;
			}
			rtic_map_agt_reply_ack(target_al_mac, RTIC_MAP_TRIG_SS);
			break;
		case RTIC_MAP_CHK_SS_STATE :
			if (rtic_dbg)
				printf("agent RTIC_MAP_CHK_SS_STATE\n");
			if ((ss_state == RTIC_SS_NONE) && (ag_state == RTIC_MAP_AGT_IDLE)) {
				rtic_map_agt_reply_ack(target_al_mac, RTIC_MAP_CHK_SS_STATE_NOT_PROC);
			} else {
				rtic_map_agt_reply_ack(target_al_mac, RTIC_MAP_CHK_SS_STATE_PROC);
			}
			break;
		case RTIC_MAP_GET_RTIC_DATA :
			if (rtic_dbg)
				printf("agent RTIC_MAP_GET_RTIC_DATA\n");

			if (rtic_get_dcs_status(&scanning, &scanning_ch) == 0) {
				if (scanning_ch == 0) {
					rtic_get_rtic_data(WLAN0, &wlan_info_5g, rtic_map.rtic_data_5g, MAX_CHANNEL_NUM_5G*sizeof(struct rtic_info_t));
					
					if (rtic_map_agt_get_hop(&rtic_map.hop_cnt))
						rtic_map.hop_cnt = 0;
					#if 0
					int i;
					printf("reply rtic_data\n");
					printf(" ch load nhm nhm_dbm clm\n");
					for (i = 0; i < 28; i++) {
						printf("%3u %3u %3u %6d %3u\n", (unsigned int) rtic_map.rtic_data_5g[i].ch_num, (unsigned int) rtic_map.rtic_data_5g[i].loading,
							(unsigned int) rtic_map.rtic_data_5g[i].nhm, (int) rtic_map.rtic_data_5g[i].nhm_dbm, (unsigned int) rtic_map.rtic_data_5g[i].clm);
					}
					#endif
					rtic_map_agt_reply_rtic_data(target_al_mac, &rtic_map);
				}
			}
			break;
		case RTIC_MAP_PROBE_AGT :
			if (rtic_dbg)
				printf("RTIC_MAP_PROBE_AGT\n");
			rtic_map_agt_reply_ack(target_al_mac, RTIC_MAP_PROBE_AGT);
			break;
		case RTIC_MAP_TERMINATE_SS :
			if (rtic_dbg)
				printf("RTIC_MAP_TERMINATE_SS\n");
			rtic_dcs_terminate();
			usleep(2*per_ch_period*1000);
			ag_state = RTIC_MAP_AGT_Z_IDLE;
			ss_state = RTIC_SS_NONE;
			break;
		case RTIC_MAP_TRIG_ZWDFS:
			if (rtic_dbg)
				printf("agent RTIC_MAP_TRIG_SS\n");
			if (rtic_get_zwdfs_state() == 0)
				rtic_trigger_zwdfs();
			else
				rtic_zwdfs_re_trigger();
			rtic_map_agt_reply_ack(target_al_mac, RTIC_MAP_TRIG_ZWDFS);
			break;
		case RTIC_MAP_SET_AGT_RD_MON_CH:
			if (rtic_dbg)
				printf("agent RTIC_MAP_SET_AGT_RD_MON_CH\n");
			ctl_assigned_ch = *((uint32_t*) (buffer + 2 + 6));
			bw = *((uint32_t*) (buffer + 2 + 6 + 4));
			rtic_zwdfs_set_rd_mon_ch(bw, ctl_assigned_ch);
			rtic_map_agt_reply_ack(target_al_mac, RTIC_MAP_SET_AGT_RD_MON_CH);
			printf("set mon ch %u bw %u\n", ctl_assigned_ch, bw);
			break;
		case RTIC_MAP_ZWDFS_RD_DETECTED_CH:
			so_zw_rd_trig_ch = *((uint32_t*) (buffer + 2 + 6));
			if (rtic_dbg)
				printf("agt RTIC_MAP_ZWDFS_RD_DETECTED_CH %u\n", so_zw_rd_trig_ch);
			
			break;
		case RTIC_MAP_SUGGESTED_CH:
			sug_ch = *((uint32_t*) (buffer + 2 + 6));
			if (rtic_dbg)
				printf("agt RTIC_MAP_SUGGESTED_CH %u\n", sug_ch);
			rtic_set_rdy_ch(sug_ch);
			break;

		case RTIC_MAP_NOCP_CH:
			nocp_ch = *((uint32_t*) (buffer + 2 + 6));
			if (rtic_dbg)
				printf("agt RTIC_MAP_NOCP_CH %u\n", nocp_ch);
			rtic_zwdfs_update_nonocp(nocp_ch, wlan_info_5g.bw);
			
			break;
		case RTIC_MAP_USED_CH:
			used_ch = *((uint32_t*) (buffer + 2 + 6));
			if (rtic_dbg)
				printf("agt RTIC_MAP_USED_CH %u\n", used_ch);
			if (used_ch != 0)
				rtic_zwdfs_set_ch_used(used_ch);
			
			break;
		default :
			printf("unknown controller cmd %u\n", (uint32_t) controller_cmd);
			break;
		}

rtic_flow :
		/* RTIC state of agent */
		switch (ag_state) {
		case RTIC_MAP_AGT_Z_IDLE :
			if (zwdfs_agt_inited == 1)
				ag_state = RTIC_MAP_AGT_Z_IDLE;
			break;
		case RTIC_MAP_AGT_Z_DFS_MON:
			if (RTIC_SUCCESS == rtic_get_rd_det_ch(&radar_det_ch_5g)) {
				if (radar_det_ch_5g != 0) {
					rtic_zwdfs_update_nonocp(radar_det_ch_5g, wlan_info_5g.bw);
					cmd_data[0] = radar_det_ch_5g;
					rtic_map_ctl_cmd(RTIC_MAP_NOCP_CH, (uint8_t*) cmd_data, sizeof(uint32_t));
				}
			}

			agt_zwdfs_det_ch = rtic_get_zwdfs_rd_det_ch();
			if (agt_zwdfs_det_ch != 0) {
				/* RTIC detected radar -> apprise controller */
				cmd_data[0] = agt_zwdfs_det_ch;
				rtic_map_ctl_cmd(RTIC_MAP_ZWDFS_RD_DETECTED_CH, (uint8_t*) cmd_data, sizeof(uint32_t));
			}

			/* others detect radar */
			if (so_zw_rd_trig_ch != 0) {
				if (so_zw_rd_trig_ch == rtic_get_local_zwdfs_ch()) {
					printf("recv radar detected\n");
					trigger_rd_detected();
					so_zw_rd_trig_ch = 0;
					break;
				} else {
					so_zw_rd_trig_ch = 0;
					printf("zwdfs error : unexpected others radar channel %u, local %u\n", so_zw_rd_trig_ch, rtic_get_local_zwdfs_ch());
				}
			}

			break;
		default :
			printf("unknown agent state %u\n", (uint32_t) ag_state);
			break;
		}

	if (wait_period)
		wait_period--;
	rtic_map_log_status(ag_state, ss_state, wait_period, NULL);
	rtic_dbg = rtic_get_dbg();
	usleep(MAP_CYCLE_PERIOD*1000);
	}
}


int main(int argc, char **argv)
{
        struct mq_attr attr_m2r;
	mqd_t mq_m2r;
	char buffer[640];

	if (argc > 1) {
		if (strcmp("-c", argv[1]) == 0)
			map_role = 1;
		else if (strcmp("-a", argv[1]) == 0)
			map_role = 2;
		else
			map_role = 0;
	}

	if (argc > 2) {
		if (strcmp("-z", argv[2]) == 0)
			zwdfs_mode = 1;
		else
			zwdfs_mode = 0;
	}

	if (zwdfs_mode)
		printf("ZWDFS MODE\n");

	rtic_create_dbg_file();
	
        attr_m2r.mq_maxmsg  = 8;
        attr_m2r.mq_curmsgs = 0;
        attr_m2r.mq_msgsize = 640;

	if ((mqd_t)-1 == (mq_m2r = mq_open(QUEUE_NANE_MAP2RTIC, O_RDWR | O_CREAT, 0666, &attr_m2r))) {
                perror("QUEUE_NANE_MAP2RTIC mq_open, Error: ");
        }

	mq_getattr(mq_m2r, &attr_m2r);
	
        attr_m2r.mq_flags   = O_NONBLOCK;
	
	if (mq_setattr(mq_m2r, &attr_m2r, NULL)) {
	     perror("map_rtic : ");
	}

	if (is_map_role_controller()) {
		printf("map_rtic controller\n");
		if (zwdfs_mode)
			rtic_zwdfs_controller(attr_m2r, mq_m2r, buffer);
		else
			rtic_controller(attr_m2r, mq_m2r, buffer);
	} else {
		printf("map_rtic agent\n");
		if (zwdfs_mode)
			rtic_zwdfs_agent(attr_m2r, mq_m2r, buffer);
		else
			rtic_agent(attr_m2r, mq_m2r, buffer);
	}

	mq_close(mq_m2r);
	mq_unlink(QUEUE_NANE_MAP2RTIC);
	return 0;
}
