#ifndef INCLUDE_COMMON_H
#define INCLUDE_COMMON_H

#include "mib.h"

typedef enum  {
	ETH_MODE=0,
	GPON_MODE,
	EPON_MODE,
	FIBER_MODE
} PON_MODE_T;

#define DEVICE_TYPE_SFU 0
#define DEVICE_TYPE_HGU 1
#define DEVICE_TYPE_HYBRID 2

#define GPON_PLOAM_PASSWORD_LENGTH	10
#define NGPON_PLOAM_PASSWORD_LENGTH	36

#define BIN_SH "/bin/sh"
int do_cmd(const char *cmd, char *argv[], int dowait);
int va_cmd(const char *cmd, int num, int dowait, ...); 
char* str2BinStr(char *str, int len);
char* binStr2Str(const char *str, int len, char *val, int vsize, char join);
char *trim_white_space(char *str);
int file_copy(const char *src, const char *dst);

#define SMUXCTL "smuxctl"
#define MAX_TAGS 2
#define MAX_LANPHY_NUM 3
#define MAX_LAN_NUM 10
int set_all_port_powerdown_state(unsigned int state);
int check_interface_exit(const char *ifname);
int set_interfac_state(const char *ifname, int state);
int set_interfac_mac(const char *ifname, const char *mac, int index);
int setup_port(void);
int setup_wan_port(void);
int rtk_port_get_wan_phyID(void);

#define MAX_WLAN_NUM        2
#define MAX_CHAN_NUM		14
#define MAX_5G_CHANNEL_NUM  196
int setup_wlan(void);
int do_wlan_cal(const char *wlIf);

#endif
