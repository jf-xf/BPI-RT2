#ifndef __RTIC_UTIL_H__
#define __RTIC_UTIL_H__
#include <stdbool.h>

extern unsigned int rtic_dbg;

/**
* @brief (CLM_target - CLM_service) to decide to switch channel for 5g band
*/
#define RTIC_5G_CLM_DIFF_THRESHOLD	32
/**
* @brief (CLM_target - CLM_service) to decide to switch channel for 2g band
*/
#define RTIC_2G_CLM_DIFF_THRESHOLD	37
/**
* @brief NHM(dbm) threshold to decide to switch channel for 5g band
*/
#define RTIC_5G_NHM_THRESHOLD	(-80)
/**
* @brief NHM(dbm) threshold to decide to switch channel for 2g band
*/
#define RTIC_2G_NHM_THRESHOLD	(-80)
/**
* @brief throughput threshold to confirm the DCS result of service channel for 5g band
*/
#define RTIC_5G_TP_THRESHOLD	1
/**
* @brief throughput threshold to confirm the DCS result of service channel for 2g band
*/
#define RTIC_2G_TP_THRESHOLD	1
/**
* @brief throughput check period for 5g band (ms)
*/
#define RTIC_5G_TP_CHK_PERIOD	500
/**
* @brief throughput check period for 2g band (ms)
*/
#define RTIC_2G_TP_CHK_PERIOD	500
/**
* @brief per-channel scanning period (ms)
*/
#define RTIC_PER_CH_SCAN_PERIOD	500
/**
* @brief per-channel scanning period for zero-wait DFS (ms)
*/
#define RTIC_ZWDFS_PER_CH_SCAN_PERIOD	100
/**
* @brief executing site-survey period (ms)
*/
#define RTIC_SS_PERIOD	(RTIC_PER_CH_SCAN_PERIOD*40 + 2000)
/**
* @brief the band of MESH network
*/
#define RTIC_MESH_BAND	WLAN0
/**
* @brief the period to check RTIC data 
*/
#define RTIC_MESH_CHK_PERIOD	(RTIC_PER_CH_SCAN_PERIOD*40)

void rtic_create_dbg_file(void);
unsigned int rtic_get_dbg(void);

/**
* @brief dcs data type
* @param	ch_num : channel number
* @param	loading : loading computed by CLM(%) and NHM(%)
* @param	nhm : NHM(%)
* @param	clm : CLM(%) 
* @param	nhm_dbm : NHM(dbm)
*/
struct rtic_info_t {
	unsigned int ch_num;
	unsigned char loading;
	unsigned char nhm;
	unsigned char clm;
	signed char nhm_dbm;
	bool is_ch_avail;
	bool is_ch_used;
	bool non_ocp;
};

struct rtic_nocp_info_t {
	unsigned int ch_num;
	bool is_nocp;
};

/**
* @brief dcs wlan channel information type
*/
struct wlan_chan_t {
	unsigned int ch_num;
	unsigned int offset;
	unsigned int bw;
};

/**
* @brief throughput type
*/
struct tp_t {
	unsigned int tx;
	unsigned int rx;
	unsigned int trx;
};

enum interface {
	WLAN0	= 0,
	WLAN1	= 1,
};

enum channel_width {
	CHANNEL_WIDTH_20	= 0,
	CHANNEL_WIDTH_40	= 1,
	CHANNEL_WIDTH_80	= 2,
};

enum chan_offset {
	CHAN_OFFSET_NO_EXT = 0,
	CHAN_OFFSET_UPPER = 1,
	CHAN_OFFSET_NO_DEF = 2,
	CHAN_OFFSET_LOWER = 3,
};

enum rtic_state {
	RTIC_INIT = 0,
	RTIC_SS = 1,
	RTIC_SWITCH_CH = 2,
	RTIC_IDLE = 3,
};

enum rtic_zwdfs_st {
	RTIC_ZWDFS_INIT = 0,
	RTIC_ZWDFS_PROCESSING = 1,
	RTIC_ZWDFS_TERMINATE = 2,
};

enum ss_state {
	RTIC_SS_NONE = 0,
	RTIC_SS_PROCESS = 1,
	RTIC_SS_DONE = 2,
	RTIC_SS_DROP = 3,
};

#define MAX_CHANNEL_NUM_5G	28
#define MAX_CHANNEL_NUM_2G	14
#define RTIC_SUCCESS	0
#define RTIC_FAIL	-1
#define ERR_READ_FILE	-1

/* 
* @brief 	get the RTIC mode 
*               (default is DCS mode)
* @return 	0 : DCS mode; 1 : ZWDFS mode ; -1: fail
*/
int rtic_get_mode(void);
/* 
* @brief 	set RTIC bandwidth

* @return 	void
*/
void rtic_set_bw(unsigned int bw);
/**
* @brief 	get the dcs channel scanning result
* @param	iface : enum interface, WLAN0(0) or WLAN1(1) 
* @param	ch_info : the channel information
* @param	dcs : pointer of dcs information
* @param	len : the size of dcs information
* @return 	0 : seccess ; -1: fail
*/
int rtic_get_rtic_data(char iface, struct wlan_chan_t *ch_info, struct rtic_info_t *dcs, size_t len);
/**
* @brief 	get the dcs trigger mode setting
*               (default is trigger mode)
* @param	mode : pointer of mode setting (0: trigger mode; 1: repeat mode)
* @return 	0 : seccess ; -1: fail
*/
int rtic_get_dcs_trigger_mode(unsigned int *mode);
/**
* @brief 	set the dcs trigger mode
*               (default is trigger mode)
* @param	mode : mode setting (0: trigger mode; 1: repeat mode)
* @return 	0 : seccess ; -1: fail
*/
void rtic_set_dcs_trigger_mode(unsigned int mode);
/**
* @brief 	get the dcs per-channel scanning period (default 100 ms)
* @param	period : pointer of period setting (ms), it should be 100 ~ 10000\n
*		period < 100 => period = 100; period > 10000 => period = 10000
* @return 	none
*/
int rtic_get_dcs_per_ch_period(unsigned int *period);
/**
* @brief 	set the dcs per-channel scanning period (default 100 ms)
* @param	period : per-channel scanning period setting (ms), it would be 100 ~ 10000
* @return 	0 : seccess ; -1: fail
*/
void rtic_dcs_set_per_ch_period(unsigned int period);
/**
* @brief 	trigger dcs channel scanning
* @param	iface : enum interface, WLAN0(0) or WLAN1(1)
* @param	bw : channel bandwidth
* @return 	none
*/
void rtic_trigger_dcs(char iface, char bw);
/**
* @brief 	get dcs channel scanning status
* @param	scanning: 0: scanning; 1: not scanning
* @param	ch: scanning channel
* @return 	0: seccess ; -1: fail
*/
int rtic_get_dcs_status(unsigned int *scanning, unsigned int *ch);
/**
* @brief 	terminate dcs channel scanning
* @return 	none
*/
void rtic_dcs_terminate(void);
/**
* @brief 	get wlan channel information
* @param	iface : enum interface, WLAN0(0) or WLAN1(1) 
* @param	wlan : channel information
* @return 	0: seccess ; -1: fail
*/
int rtic_get_wlan_chan_info(char iface, struct wlan_chan_t *wlan);
/**
* @brief 	get wlan throughput
* @param	iface : enum interface, WLAN0(0) or WLAN1(1) 
* @param	tp : throughput information
* @return 	0: seccess ; -1: fail
*/
int rtic_get_wlan_tp(char iface, struct tp_t *tp);
/**
* @brief 	update the suggested channel to file "/var/run/dcs_wlanx_suggest_ch"
* @param	iface : enum interface, WLAN0(0) or WLAN1(1) 
* @param	suggest_ch : suggested channel
* @return 	none
*/
void rtic_update_suggested_ch(char iface, unsigned int suggest_ch);
/**
* @brief 	switch channel
* @param	iface : enum interface, WLAN0(0) or WLAN1(1) 
* @param	wlan_info : channel information
* @return 	0: seccess ; -1: fail
*/
int rtic_switch_channel(char iface, struct wlan_chan_t *wlan_info);
/**
* @brief 	for mesh controller, get the rtic data of agents
* @param	iface : enum interface, WLAN0(0) or WLAN1(1) 
* @param	ch_info : the channel information
* @param	dcs : pointer of dcs information
* @param	len : the size of dcs information
* @return 	0 : seccess ; -1: fail
*/
int rtic_mesh_get_agents_rtic_data(char iface, struct wlan_chan_t *ch_info, struct rtic_info_t *dcs, size_t len);
/**
* @brief 	for mesh agent, send the rtic data to controller
* @param	iface : enum interface, WLAN0(0) or WLAN1(1) 
* @param	ch_info : the channel information
* @param	dcs : pointer of dcs information
* @param	len : the size of dcs information
* @return 	0 : seccess ; -1: fail
*/
int rtic_mesh_send_rtic_data(char iface, struct wlan_chan_t *ch_info, struct rtic_info_t *dcs, size_t len);
/**
* @brief 	trigger zero-wait DFS start
* @return 	0 : seccess ; -1: fail
*/
void rtic_trigger_zwdfs(void);
/**
* @brief 	get radar detected channel from 5g wlan
* @param	dfs_det_ch : radar detected channel 
* @return 	0 : seccess ; -1: fail
*/
int rtic_get_rd_det_ch(unsigned int *dfs_det_ch);
/**
* @brief 	set ready DFS channel to 5g wlan
* @param	rdy_ch : ready DFS channel
* @return 	0 : seccess ; -1: fail
*/
int rtic_set_rdy_ch(unsigned int rdy_ch);
/**
* @brief 	set channel used by 5g wlan
* @param	ch : channel
*/
void rtic_zwdfs_set_ch_used(unsigned int ch);
/**
* @brief 	set ACS NHM and CLM ratio to compute channel loading before ZWDFS
* @param	ratio : 0:0.5*NHM+0.5*CLM; 1:0.33*NHM+0.66*CLM; 2:0.25*NHM+0.75*CLM 3:CLM;
*/
void rtic_zwdfs_set_acs_ratio(unsigned char ratio);
/**
* @brief 	set non-ocp to 8701AI for ch detected radar by 5g wlan
* @param	ch : channel
* @param	bw : bandwidth
*/
void rtic_zwdfs_update_nonocp(unsigned int ch, unsigned int bw);
/**
* @brief 	get the channel band
* @param	ch : channel
* @return 	band number
*/
unsigned int rtic_get_5g_band(unsigned int ch);
/**
* @brief 	check channel is DFS channel or not
* @param	ch : channel
* @return 	TRUE or FALSE
*/
bool rtic_is_dfs_ch(unsigned int ch);
int rtic_get_nocp_info(struct rtic_nocp_info_t *nocp_info);
#endif
