#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/wireless.h>
#include <unistd.h>
#include "autoconf.h"
#include "rtic_util.h"
#include <math.h>

unsigned int rtic_dbg = 0;

#ifdef CONFIG_USER_RTIC_2G_8192FE
unsigned char IS_WLAN1_8192F = 1;
#else
unsigned char IS_WLAN1_8192F = 0;
#endif
#define SIOCRTIC	(SIOCSIWRANGE)
#define SIOCDEVPRIVATEAXEXT (SIOCDEVPRIVATE+2)
#define SIOCTOTALTP	0x8B93
#define SIOCGISETZWDFS  0x9011
#define SIOCGIGETZWDFS  0x9012
#define SIOCGIGET_RTIC_NOCP 0x9017
#define ERR_READ_FILE	-1
double dbm_convert_tbl[10] = { 1, 0.794328234724, 0.63095734448, 0.501187233627, 0.398107170553, 0.316227766017, 0.251188643151, 0.199526231497, 0.158489319246, 0.125892541179 };

#define RTIC_CMD_LEN 64
#define RTIC_MODE_PROC "/proc/net/rtic/wlan2/rtic_mode"
#define RTIC_DCS_TRIG_MODE_PROC "/proc/net/rtic/wlan2/dcs_trigger_mode"
#define RTIC_DCS_TRIG_PROC "/proc/net/rtic/wlan2/dcs_trigger"
#define RTIC_DCS_TERMINATE_PROC "/proc/net/rtic/wlan2/dcs_terminate"
#define RTIC_DCS_PER_CH_PERIOD_PROC "/proc/net/rtic/wlan2/dcs_per_ch_scan_period"
#define RTIC_BW_PROC "/proc/net/rtic/wlan2/rtic_bw"
#define RTIC_ZWDFS_TRIG_PROC "/proc/net/rtic/wlan2/zwdfs_trig"
#define RTIC_ZWDFS_CH_USED_PROC "/proc/net/rtic/wlan2/zwdfs_set_ch_used"
#define RTIC_ZWDFS_ACS_RATIO_PROC "/proc/net/rtic/wlan2/zwdfs_set_acs_ratio"
#define RTIC_ZWDFS_UD_NONOCP_PROC "/proc/net/rtic/wlan2/update_non_ocp"
#define WLAN0_CHAN_PROC	"/proc/net/rtl8852ae/wlan0/chan"
#define WLAN1_CHAN_PROC	"/proc/net/rtl8852ae/wlan1/chan"
#define WLAN0_TP_PROC	"/proc/net/rtl8852ae/wlan0/tp_info"
#define WLAN1_TP_PROC	"/proc/net/rtl8852ae/wlan1/tp_info"
#define WLAN_8192F_INTERFACE "wlan1"

static int get_wlan_ch_8192f(struct wlan_chan_t *wlan)
{
	FILE* fp = NULL;
	size_t len = 0;
	char* line = NULL;
	unsigned int line_size = 3;
	
	fp = fopen("/proc/wlan1/mib_rf", "r");
	if (fp == NULL)
		goto fail;

	getline(&line, &len, fp);
	while (line_size > 0) {
		if (strncmp("    dot11channel:", line, strlen("    dot11channel:")) == 0)
			break;
		getline(&line, &len, fp);
		line_size--;
	}
	fclose(fp);
	
	if(line == NULL)
		goto fail;

	int ret = 0;

	ret = sscanf(line, "    dot11channel: %u", &wlan->ch_num);
	
	if (ret < 0) {
		perror("get_wlan_ch_8192f");
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

static int get_wlan_bw_8192f(struct wlan_chan_t *wlan)
{
	FILE* fp = NULL;
	size_t len = 0;
	char* line = NULL;
	unsigned int line_size = 7;
	char recv[16];
	
	fp = fopen("/proc/wlan1/mib_11n", "r");
	if (fp == NULL)
		goto fail;

	getline(&line, &len, fp);
	while (line_size > 0) {
		if (strncmp("    currBW(op):", line, strlen("    currBW(op):")) == 0)
			break;
		getline(&line, &len, fp);
		line_size--;
	}
	fclose(fp);
	
	if(line == NULL)
		goto fail;

	int ret = 0;

	ret = sscanf(line, "    currBW(op): %s", recv);

	if (ret < 0) {
		perror("get_wlan_bw_8192f");
		goto fail;
	}

	if (strcmp("40M", recv) == 0) {
		wlan->bw = CHANNEL_WIDTH_40;
	} else {
		wlan->bw = CHANNEL_WIDTH_20;
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

static int get_wlan_ofst_8192f(struct wlan_chan_t *wlan)
{
	FILE* fp = NULL;
	size_t len = 0;
	char* line = NULL;
	unsigned int line_size = 8;
	char recv[16];
	
	fp = fopen("/proc/wlan1/mib_11n", "r");
	if (fp == NULL)
		goto fail;

	getline(&line, &len, fp);
	while (line_size > 0) {
		if (strncmp("    2ndchoffset", line, strlen("    2ndchoffset")) == 0)
			break;
		getline(&line, &len, fp);
		line_size--;
	}
	fclose(fp);
	
	if(line == NULL)
		goto fail;

	int ret = 0;

	ret = sscanf(line, "    2ndchoffset: %s", recv);
	
	if (ret < 0) {
		perror("get_wlan_ofst_8192f");
		goto fail;
	}

	if (strcmp("below", recv) == 0) {
		wlan->offset = CHAN_OFFSET_UPPER;
	} else {
		wlan->offset = CHAN_OFFSET_LOWER;
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

static int get_wlan_ch_info_8192f(struct wlan_chan_t *wlan)
{
	if (get_wlan_ch_8192f(wlan))
		return ERR_READ_FILE;
	if (get_wlan_bw_8192f(wlan))
		return ERR_READ_FILE;
	if (get_wlan_ofst_8192f(wlan))
		return ERR_READ_FILE;

	return 0;
}

static int get_wlan_tp_8192f(struct tp_t *tp)
{
	int skfd;
	char iface[6];
	strcpy(iface, "wlan1");
	struct iwreq wrq;
	struct ifreq rq;
	unsigned int tp_read[2];

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	
	strncpy(wrq.ifr_name, iface, IFNAMSIZ);
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	/* check exist */
	if (ioctl(skfd, SIOCGIWNAME, &rq) < 0) {
		perror("SIOCGIWNAME, Error: ");
		goto fail;
	}

	wrq.u.data.pointer = (void *)tp_read;
	wrq.u.data.length = 2*sizeof(unsigned int);
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	if (ioctl(skfd, SIOCTOTALTP, &rq) < 0) {
		perror("SIOCTOTALTP, Error: ");
		goto fail;
	}

	tp->tx = tp_read[0];
	tp->rx = tp_read[1];
	tp->trx = tp_read[0] + tp_read[1];

	close(skfd);
	return 0;

fail:
	close(skfd);
	return ERR_READ_FILE;
}

static int switch_channel_8192f(struct wlan_chan_t *wlan_info) 
{
	char cmd[64];
	unsigned int ofst = 0;

	sprintf(cmd, "iwpriv wlan1 set_mib use40M=%u", wlan_info->bw);
	printf("%s\n", cmd);
	system(cmd);
	if (((wlan_info->offset == CHAN_OFFSET_UPPER) || (wlan_info->ch_num > 9)) && (wlan_info->ch_num > 4))
		ofst = 1; /* 2nd offset below */
	else
		ofst = 2; /* 2nd offset above*/
	sprintf(cmd, "iwpriv wlan1 set_mib 2ndchoffset=%u", ofst);
	printf("%s\n", cmd);
	system(cmd);
	sprintf(cmd, "iwpriv wlan1 set_mib csa=%u", wlan_info->ch_num);
	printf("%s\n", cmd);
	system(cmd);
	return 0;
}

/* export functions */

int rtic_get_mode(void)
{
        FILE* fp = NULL;
        size_t len = 0;
        char* line = NULL;
        unsigned int mode;

        fp = fopen(RTIC_DCS_TRIG_MODE_PROC,"r");
        if (fp == NULL)
                goto fail;

        getline(&line, &len, fp);
        fclose(fp);

        if(line == NULL)
                goto fail;

        int ret;
        ret = sscanf(line, "rfic mode %u %*s", &mode);
        if(ret != 1) {
                printf("get_rtic_mode fail proc %d\n", ret);
                goto fail;
        }

        free(line);
        return mode;
fail:
        if (fp != NULL)
                fclose(fp);
        if (line != NULL)
                free(line);

        printf("get_rtic_mode fail\n");

        return ERR_READ_FILE;
}

void rtic_set_bw(unsigned int bw)
{
	char cmd[RTIC_CMD_LEN];

	sprintf(cmd, "echo %u > %s", bw, RTIC_BW_PROC);
	system(cmd);
	return;
}	

void rtic_create_dbg_file(void)
{
	FILE * fp;

	fp = fopen("/var/tmp/rtic_dbg", "w+");
	fprintf(fp, "%u", rtic_dbg);
	fclose(fp);
	return;
}

unsigned int rtic_get_dbg(void)
{
	FILE* fp = NULL;
	size_t len = 0;
	char* line = NULL;
	
	fp = fopen("/var/tmp/rtic_dbg", "r");
	if (fp == NULL)
		goto fail;
	
	getline(&line, &len, fp);
	fclose(fp);

	if(line == NULL)
		goto fail;

	unsigned int ret;
	
	if (sscanf(line, "%u", &ret) < 0) {
		return 0;
	}
	
	free(line);
	return ret;
fail:
	if (fp != NULL)
		fclose(fp);
	if (line != NULL)
		free(line);

	return 0;
}

int rtic_get_rtic_data(char iface, struct wlan_chan_t *ch_info, struct rtic_info_t *dcs, size_t len)
{
	int skfd;
	char iface_rtic[6];
	strcpy(iface_rtic, "wlan2");
	struct iwreq wrq;
	struct ifreq rq;

	if ((iface != 0) && (iface != 1)) {
		printf("%s: iface error %d\n", __func__, iface);
		goto fail;
	}

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	
	strncpy(wrq.ifr_name, iface_rtic, IFNAMSIZ);
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	/* check exist */
	if (ioctl(skfd, SIOCGIWNAME, &rq) < 0) {
		perror("SIOCGIWNAME, Error: ");
		goto fail;
	}

	wrq.u.data.pointer = (void *)dcs;
	wrq.u.data.length = len;
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	if (ioctl(skfd, SIOCRTIC, &rq) < 0) {
		perror("SIOCRTIC, Error: ");
		goto fail;
	}

	if (ch_info == NULL)
		goto fail;

	if (0) {
		unsigned char i , max = MAX_CHANNEL_NUM_2G;
		if (iface == WLAN0) 
			max = MAX_CHANNEL_NUM_5G;
		printf(" ch load nhm nhm_dbm clm\n");
		for (i = 0; i < max; i++) {
			printf("%3u %3u %3u %6d %3u\n", (unsigned int) dcs[i].ch_num, (unsigned int) dcs[i].loading,
				(unsigned int) dcs[i].nhm, (int) dcs[i].nhm_dbm, (unsigned int) dcs[i].clm);
		}
	}

	close(skfd);
	return 0;

fail:
	close(skfd);
	return ERR_READ_FILE;
}

int rtic_get_dcs_trigger_mode(unsigned int *mode)
{
	FILE* fp = NULL;
	size_t len = 0;
	char* line = NULL;
	
	fp = fopen(RTIC_DCS_TRIG_MODE_PROC, "r");
	if (fp == NULL)
		goto fail;
	
	getline(&line, &len, fp);
	fclose(fp);

	if(line == NULL)
		goto fail;

	int ret;

	ret = sscanf(line, "%u", mode);
	
	if (ret != 1) {
		printf("rtic_get_dcs_trigger_mode fail proc %d\n", ret);
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

void rtic_set_dcs_trigger_mode(unsigned int mode)
{
	char cmd[RTIC_CMD_LEN];
		
	sprintf(cmd, "echo %u > %s", mode, RTIC_DCS_TRIG_MODE_PROC);
	system(cmd);
	return;
}	

int rtic_get_dcs_status(unsigned int *scanning, unsigned int *ch)
{
	FILE* fp = NULL;
	size_t len = 0;
	char* line = NULL;
	
	fp = fopen(RTIC_DCS_TRIG_PROC, "r");
	if (fp == NULL)
		goto fail;
	
	getline(&line, &len, fp);
	fclose(fp);

	if(line == NULL)
		goto fail;

	int ret;

	ret = sscanf(line, "%u, ch=%u", scanning, ch);
	
	if (ret < 0) {
		perror("rtic_get_dcs_status");
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

void rtic_trigger_dcs(char iface, char bw)
{
	unsigned int scanning, ch;
	char cmd[RTIC_CMD_LEN];
	//char bw_cmd[RTIC_CMD_LEN];

	rtic_get_dcs_status(&scanning, &ch);

	if (scanning == 0) {
		sprintf(cmd, "echo %u > %s", bw, RTIC_BW_PROC);
		system(cmd);

		if (iface == WLAN1)
			sprintf(cmd, "echo 1 0 > %s", RTIC_DCS_TRIG_PROC);
		else
			sprintf(cmd, "echo 1 1 > %s", RTIC_DCS_TRIG_PROC);
		system(cmd);
	}
	
	return;
}

void rtic_dcs_terminate(void)
{
	char cmd[RTIC_CMD_LEN];

	sprintf(cmd, "echo 1 > %s", RTIC_DCS_TERMINATE_PROC);
	system(cmd);
	return;
}

int rtic_get_dcs_per_ch_period(unsigned int *period)
{
	FILE* fp = NULL;
	size_t len = 0;
	char* line = NULL;
	
	fp = fopen(RTIC_DCS_PER_CH_PERIOD_PROC, "r");
	if (fp == NULL)
		goto fail;
	
	getline(&line, &len, fp);
	fclose(fp);

	if(line == NULL)
		goto fail;

	int ret;

	ret = sscanf(line, "%u", period);
	
	if (ret < 0) {
		perror("rtic_get_dcs_per_ch_period");
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

void rtic_dcs_set_per_ch_period(unsigned int period)
{
	char cmd[RTIC_CMD_LEN];
		
	sprintf(cmd, "echo %u > %s", period, RTIC_DCS_PER_CH_PERIOD_PROC);
	system(cmd);
	return;
}

int rtic_get_wlan_chan_info(char iface, struct wlan_chan_t *wlan)
{
	FILE* fp = NULL;
	size_t len = 0;
	char* line = NULL;

	if (iface == WLAN0) {
		fp = fopen(WLAN0_CHAN_PROC, "r");
	} else if (iface == WLAN1) {
		if (IS_WLAN1_8192F)
			return get_wlan_ch_info_8192f(wlan);
		else
			fp = fopen(WLAN1_CHAN_PROC, "r");
	} else {
		goto fail;
	}
	
	if (fp == NULL)
		goto fail;
	
	getline(&line, &len, fp);
	fclose(fp);

	if(line == NULL)
		goto fail;

	if(sscanf(line, "ch=%u, ch_offset=%u, bw=%u", &wlan->ch_num, &wlan->offset, &wlan->bw) != 3) {
		printf("get_wlan_chan fail proc\n");
		goto fail;
	}
	
	free(line);
	return 0;
fail:
	if (fp != NULL)
		fclose(fp);
	if (line != NULL)
		free(line);
	
	printf("get_wlan_chan fail\n");
	return ERR_READ_FILE;
}

char *iface_name[5] = 
{
	"",
	"-vap0",
	"-vap1",
	"-vap2",
	"-vap3"
};

static int get_rtl8832_tp(char iface, struct tp_t *tp)
{
	FILE* fp = NULL;
	char path[128];
	int i = 0;
	size_t len = 0;
	char* line = NULL;
	struct tp_t tp_tmp;

	tp->tx = 0;
	tp->rx = 0;
	tp->trx = 0;
	
	for (i = 0; i < 5 ; i++) {
		if (iface == WLAN0) {
			sprintf(path, "/proc/net/rtl8852ae/wlan0%s/tp_info", iface_name[i]);
		} else {
			sprintf(path, "/proc/net/rtl8852ae/wlan1%s/tp_info", iface_name[i]);
		}

		fp = fopen(path, "r");

		if (fp) {
			getline(&line, &len, fp);
			fclose(fp);
			fp = NULL;
			if (sscanf(line, "tx=%u Mbps, rx=%u Mbps, trx=%u Mbps", &tp_tmp.tx, &tp_tmp.rx, &tp_tmp.trx) != 3) {
				printf("get_wlan_chan %s fail proc\n", iface_name[i]);
			} else {
				tp->tx += tp_tmp.tx;
				tp->rx += tp_tmp.rx;
				tp->trx += tp_tmp.trx;
			}
			if (line) {
				free(line);
				line = NULL;
			}
		}
	}

	return 0;
}

int rtic_get_wlan_tp(char iface, struct tp_t *tp)
{
	if ((iface == WLAN1) && (IS_WLAN1_8192F))
		return get_wlan_tp_8192f(tp);
	else
		return get_rtl8832_tp(iface, tp);
}

void rtic_update_suggested_ch(char iface, unsigned int suggest_ch)
{	
	FILE * fp;
	if (iface == WLAN0)
		fp = fopen("/var/run/dcs_wlan0_suggest_ch", "w+");
	else
		fp = fopen("/var/run/dcs_wlan1_suggest_ch", "w+");
	
	fprintf(fp, "%u", suggest_ch);
	fclose(fp);
	
	return;
}

int rtic_switch_channel(char iface, struct wlan_chan_t *wlan_info)
{	
	char cmd[64];

	if ((iface == WLAN1) && (IS_WLAN1_8192F))
		return switch_channel_8192f(wlan_info);
	
	if (iface == WLAN0) { 
#if CONFIG_G6_SUPPORT_CSA
		sprintf(cmd, "echo %u 5 > /proc/net/rtl8852ae/wlan0/csa_trigger", wlan_info->ch_num);
#else
		sprintf(cmd, "echo %u %u %u > /proc/net/rtl8852ae/wlan0/change_bss_chbw", wlan_info->ch_num, wlan_info->bw, wlan_info->offset);
#endif
	} else if (iface == WLAN1) {
#if CONFIG_G6_SUPPORT_CSA
		sprintf(cmd, "echo %u 5 > /proc/net/rtl8852ae/wlan1/csa_trigger", wlan_info->ch_num);
#else
		sprintf(cmd, "echo %u %u %u > /proc/net/rtl8852ae/wlan1/change_bss_chbw", wlan_info->ch_num, wlan_info->bw, wlan_info->offset);
#endif
	}

	printf("%s\n", cmd);
	system(cmd);

	sleep(2);
	return 0;
}

void rtic_trigger_zwdfs(void)
{
	char cmd[RTIC_CMD_LEN];

	sprintf(cmd, "echo 1 > %s", RTIC_ZWDFS_TRIG_PROC);
	system(cmd);
	
	return;
}

#if 1
int rtic_get_rd_det_ch(unsigned int *dfs_det_ch)
{
	int skfd, cmd_id;
	char iface_rtic[6];
	strcpy(iface_rtic, "wlan0");
	struct iwreq wrq;
	struct ifreq rq;
	unsigned int ch_read[1];
	ch_read[0]= 87;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	
	strncpy(wrq.ifr_name, iface_rtic, IFNAMSIZ);
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	/* check exist */
	if (ioctl(skfd, SIOCGIWNAME, &rq) < 0) {
		perror("SIOCGIWNAME, Error: ");
		goto fail;
	}

	cmd_id = SIOCDEVPRIVATEAXEXT;
	wrq.u.data.flags = SIOCGIGETZWDFS;
	wrq.u.data.pointer = (void *) ch_read;
	wrq.u.data.length = sizeof(unsigned int);
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	if (ioctl(skfd, cmd_id, &rq) < 0) {
		perror("SIOCGIGETZWDFS, Error: ");
		goto fail;
	}

	*dfs_det_ch = ch_read[0];
	//printf("recv %u %u\n", *dfs_det_ch, ch_read[0]);

	close(skfd);
	return 0;

fail:
	close(skfd);
	return ERR_READ_FILE;

}


int rtic_set_rdy_ch(unsigned int rdy_ch)
{
	int skfd, cmd_id;
	char iface_rtic[6];
	strcpy(iface_rtic, "wlan0");
	struct iwreq wrq;
	struct ifreq rq;
	unsigned int dfs_det_ch[1];

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	
	strncpy(wrq.ifr_name, iface_rtic, IFNAMSIZ);
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	/* check exist */
	if (ioctl(skfd, SIOCGIWNAME, &rq) < 0) {
		perror("SIOCGIWNAME, Error: ");
		goto fail;
	}

	cmd_id = SIOCDEVPRIVATEAXEXT;
	wrq.u.data.flags = SIOCGISETZWDFS;
	dfs_det_ch[0] = rdy_ch;
	wrq.u.data.pointer = (void *) dfs_det_ch;
	wrq.u.data.length = sizeof(unsigned int);
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	if (ioctl(skfd, cmd_id, &rq) < 0) {
		perror("SIOCGISETZWDFS, Error: ");
		goto fail;
	}
	//printf("recv %u\n", dfs_det_ch);

	close(skfd);
	return 0;

fail:
	close(skfd);
	return ERR_READ_FILE;

}

int rtic_get_nocp_info(struct rtic_nocp_info_t *nocp_info)
{
	int skfd, cmd_id;
	char iface_rtic[6];
	strcpy(iface_rtic, "wlan0");
	struct iwreq wrq;
	struct ifreq rq;
	struct rtic_nocp_info_t local_nocp_info[MAX_CHANNEL_NUM_5G];

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	
	strncpy(wrq.ifr_name, iface_rtic, IFNAMSIZ);
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	/* check exist */
	if (ioctl(skfd, SIOCGIWNAME, &rq) < 0) {
		perror("SIOCGIWNAME, Error: ");
		goto fail;
	}

	cmd_id = SIOCDEVPRIVATEAXEXT;
	wrq.u.data.flags = SIOCGIGET_RTIC_NOCP;
	wrq.u.data.pointer = (void *) local_nocp_info;
	wrq.u.data.length = MAX_CHANNEL_NUM_5G*sizeof(struct rtic_nocp_info_t);
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	if (ioctl(skfd, cmd_id, &rq) < 0) {
		perror("SIOCGIGET_RTIC_NOCP, Error: ");
		goto fail;
	}

	memcpy(nocp_info, local_nocp_info, MAX_CHANNEL_NUM_5G*sizeof(struct rtic_nocp_info_t));
	close(skfd);
	return 0;

fail:
	close(skfd);
	return ERR_READ_FILE;	
}


void rtic_zwdfs_set_ch_used(unsigned int ch)
{
	char cmd[RTIC_CMD_LEN];

	sprintf(cmd, "echo %u > %s", ch, RTIC_ZWDFS_CH_USED_PROC);
	system(cmd);
	return;
}

void rtic_zwdfs_set_acs_ratio(unsigned char ratio)
{
	char cmd[RTIC_CMD_LEN];

	sprintf(cmd, "echo %u > %s", ratio, RTIC_ZWDFS_ACS_RATIO_PROC);
	system(cmd);
	return;
}

void rtic_zwdfs_update_nonocp(unsigned int ch, unsigned int bw)
{
	char cmd[RTIC_CMD_LEN];

	sprintf(cmd, "echo %u %u 1 > %s", ch, bw, RTIC_ZWDFS_UD_NONOCP_PROC);
	system(cmd);
	return;
}

unsigned int rtic_get_5g_band(unsigned int ch)
{
	if (ch <= 48)
		return 0;
	else if (ch <= 64)
		return 1;
	else if (ch <= 112)
		return 2;
	else if (ch <= 128)
		return 3;
	else if (ch <= 144)
		return 4;
	else if (ch <= 161)
		return 5;
	else if (ch == 165)
		return 6;
	else
		return 0;
}

bool rtic_is_dfs_ch(unsigned int ch)
{
	if ((ch >= 52) && (ch <= 144))
		return 1;
	else
		return 0;
}
#endif

