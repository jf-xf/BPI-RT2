#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>      // ioctl()
#include <sys/socket.h>     // socket()
#include <linux/wireless.h> // struct iwreq
#include <unistd.h>
#include <stdint.h>
#include <errno.h>          // errno
#include <features.h>
#include "ini.h"
#include "config_file_handler.h"

#define MULTI_AP_TEARDOWN_BIT (0x10)
#define MULTI_AP_FRONTHAUL_BSS_BIT (0x20)
#define MULTI_AP_BACKHAUL_BSS_BIT (0x40)
#define MULTI_AP_BACKHAUL_STA_BIT (0x80)

#define VLAN_CONFIG_FILE "/tmp/map_vlan_setting"
#define VLAN_RESET_FILE "/tmp/map_vlan_reset"

#define SIOCDEVPRIVATEAXEXT 0x89f2

struct vlan_group {
	uint16_t vid;
	uint8_t  interface_nr;
	char **  interfaces;
};

struct vlan_setting {
	uint16_t           primary_vid;
	uint8_t            vlan_group_nr;
	struct vlan_group *vlan_groups;
};

#define MULTIAP_CONF "/var/multiap.conf"
#define READ_BUF_SIZE 50
char map_brif[READ_BUF_SIZE] = "br0"; //default bridge_name is br0

///////////////////////////////////////////////////////
// SDK Helper Functions
///////////////////////////////////////////////////////

#if defined(CONFIG_RTK_PON_XDSL) || !defined(__UCLIBC__)
/*
 * Copy string src to buffer dst of size dsize.  At most dsize-1
 * chars will be copied.  Always NUL terminates (unless dsize == 0).
 * Returns strlen(src); if retval >= dsize, truncation occurred.
 */
size_t strlcpy(char *dst, const char *src, size_t dsize)
{
        const char *osrc = src;
        size_t nleft = dsize;

        /* Copy as many bytes as will fit. */
        if (nleft != 0) {
                while (--nleft != 0) {
                        if ((*dst++ = *src++) == '\0')
                                break;
                }
        }

        /* Not enough room in dst, add NUL and traverse rest of src. */
        if (nleft == 0) {
                if (dsize != 0)
                        *dst = '\0';            /* NUL-terminate dst */
                while (*src++)
                        ;
        }

        return(src - osrc - 1); /* count does not include NUL */
}
#endif // CONFIG_RTK_PON_XDSL

uint8_t is_interface_up(char *interface)
{
	int          skfd = 0;
	struct ifreq ifr;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == skfd) {
		printf("[luna_map_vlan_config][is_interface_up] socket() error!!");
		return 0;
	}

	strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);

	if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
		close(skfd);
		return 0;
	}
	close(skfd);
	return (uint8_t) !!(ifr.ifr_flags & IFF_UP);
}

int _rtk_get_is_ax_support(char *interface)
{
	char filename[64]  = { 0 };
	int  is_ax_support = 0;

	snprintf(filename, sizeof(filename), "/proc/net/rtl8852ae/%s", interface);
	if (-1 != access(filename, F_OK))
		is_ax_support = 1;

	snprintf(filename, sizeof(filename), "/proc/net/rtk_wifi6/%s", interface);
	if (-1 != access(filename, F_OK))
		is_ax_support = 2;

	return is_ax_support;
}

#define RTL8192CD_IOCTL_SET_MIB 0x89f1
uint8_t rtk_set_mib(char *interface_name, const char *item_str, const char *value_str)
{
	int s;

	int cmd_id, is_ax_support = 0;

	struct iwreq wrq;
	struct ifreq rq;
	static char  mib_str_buf[512];
	snprintf(mib_str_buf, 512, "%s=%s", item_str, value_str);
	printf("Set mib for %s: %s\n", interface_name, mib_str_buf);

	/*** Initialize socket ***/
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == s) {
		printf("[PLATFORM] socket('%s') returned with errno=%d (%s) while opening a DGRAM socket\n", interface_name, errno, strerror(errno));
		return 0;
	}

	is_ax_support = _rtk_get_is_ax_support(interface_name);

	/*** Initialize struct iwreq ***/
	memset(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, interface_name, IFNAMSIZ - 1);

	if (is_ax_support) {
		wrq.u.data.flags = RTL8192CD_IOCTL_SET_MIB;
		cmd_id           = SIOCDEVPRIVATEAXEXT;
	} else {
		cmd_id = RTL8192CD_IOCTL_SET_MIB;
	}

	/*** fill mib string ***/
	wrq.u.data.pointer = (caddr_t)mib_str_buf;
	wrq.u.data.length  = strlen(mib_str_buf) + 1;
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	/*** set mib ***/
	if (ioctl(s, cmd_id, &rq) < 0) {
		printf("[PLATFORM] set_mib: %s(%s). Error to ioctl.(%d)\n", mib_str_buf, interface_name, errno);
		close(s);
		return 0;
	}

	close(s);
	return 1;
}

#define RTL8192CD_IOCTL_GET_MIB 0x89f2
uint8_t rtk_get_mib(char *interfacename, char *mibname, void *result, int size)
{
	int          skfd;
	int cmd_id, is_ax_support=0;
	struct iwreq wrq;
	struct ifreq rq;
	char         tmp[30];

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0) {
		printf("ioctl [RTL8192CD_IOCTL_GET_MIB] socket error\n");
		return 0;
	}

	is_ax_support = _rtk_get_is_ax_support(interfacename);

	memset(&wrq, 0, sizeof(wrq));
	strlcpy(wrq.ifr_name, interfacename, sizeof(wrq.ifr_name));
	strlcpy(tmp, mibname, sizeof(tmp));

	wrq.u.data.pointer = tmp;

	if(is_ax_support)
	{
		wrq.u.data.flags = RTL8192CD_IOCTL_GET_MIB;
		wrq.u.data.length = strlen(tmp) + 1;
		cmd_id = SIOCDEVPRIVATEAXEXT;
	}
	else
	{
		cmd_id = RTL8192CD_IOCTL_GET_MIB;
		wrq.u.data.length = strlen(tmp);
	}
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	/* Do the request */
	if (ioctl(skfd, cmd_id, &rq) < 0)
	{
		printf("ioctl [RTL8192CD_IOCTL_GET_MIB] for mib %s on %s failed!\n", mibname, interfacename);
		close(skfd);
		return 0;
	}

	if (size)
		memcpy(result, tmp, size);
	else
		strcpy(result, (char *)tmp);

	close(skfd);
	return 0;
}

uint8_t ssid_to_interfacename(char *ssid, char *interface_name)
{
	char interface[] = "wlan0-vap0";
	int  i = 0, j = 0;
	char ssid_buf[64];
	for (i = 0; i < 3; i++) {
		sprintf(interface, "wlan%d", i);
		rtk_get_mib(interface, "ssid", ssid_buf, 0);
		// printf("Matching %s and %s\n", ssid, ssid_buf);
		if (0 == strcmp(ssid, ssid_buf)) {
			strcpy(interface_name, interface);
			return 0;
		}
		for (j = 0; j < 4; j++) {
			sprintf(interface, "wlan%d-vap%d", i, j);
			rtk_get_mib(interface, "ssid", ssid_buf, 0);
			// printf("Matching %s and %s\n", ssid, ssid_buf);
			if (0 == strcmp(ssid, ssid_buf)) {
				strcpy(interface_name, interface);
				return 0;
			}
		}
	}
	return 1;
}

void append_ethernet_interfaces(char ***interfaces, uint8_t *interface_nr)
{
	char buf_tmp[512];

	FILE *brctl = popen("brctl show", "r");
	if (brctl == NULL) {
		printf("[error][append_ethernet_interfaces] brctl show read fail!!\n");
		return;
	}

	int   flag      = 0;
	char *ptr       = 0;
	char *interface = 0;

	int   i = 0;

	while (fgets(buf_tmp, 512, brctl) != 0) {
		if ((ptr = strstr(buf_tmp, "br"))) {
			flag = ('0' == *(ptr + 2)) ? 1 : 0;
		}
		if (!flag)
			continue;

		interface = NULL;
		if ((ptr = strstr(buf_tmp, "eth"))) {
			interface = ptr;
		}

		if (interface) {
			printf("Checking ethernet interface %s..\n", interface);
			strlcpy(buf_tmp, interface, sizeof(buf_tmp));
			buf_tmp[strcspn(buf_tmp, "\r\n")] = '\0';
			int skip = 0;
			for(i = 0; i < *interface_nr; i++) {
				if (0 == strcmp((*interfaces)[i], buf_tmp)) {
					skip = 1;
					break;
				}
			}
			if (skip) {
				continue;
			}
			*interface_nr += 1;
			*interfaces                               = (char **)realloc(*interfaces, sizeof(char *) * (*interface_nr));
			(*interfaces)[*interface_nr - 1] = strdup(buf_tmp);
			printf("Adding ethernet interface %s..\n", buf_tmp);
		}
	}

	pclose(brctl);
}

void obtain_backhaul_interfaces(char ***backhaul_interfaces, uint8_t *backhaul_interface_nr, int parent_profile)
{
	char interface_name[32];

	uint8_t bss_type = 0;
	int     i = 0, j = 0;

	sprintf(interface_name, "wlan0-vap0");

	for (i = 0; i < 3; i++) {
		sprintf(interface_name, "wlan%d", i);
		if (!is_interface_up(interface_name)) {
			continue;
		}
		rtk_get_mib(interface_name, "multiap_bss_type", &bss_type, 1);
		if (bss_type & MULTI_AP_BACKHAUL_BSS_BIT || bss_type & MULTI_AP_BACKHAUL_STA_BIT) {
			*backhaul_interface_nr += 1;
			*backhaul_interfaces                               = (char **)realloc(*backhaul_interfaces, sizeof(char *) * (*backhaul_interface_nr));
			(*backhaul_interfaces)[*backhaul_interface_nr - 1] = strdup(interface_name);
		}
		for (j = 0; j < 5; j++) {
			sprintf(interface_name, "wlan%d-vap%d", i, j);
			if (4 == j) {
				sprintf(interface_name, "wlan%d-vxd", i);
			}
			if (!is_interface_up(interface_name)) {
				continue;
			}
			if (4 == j && 1 == parent_profile) {
				continue;
			}
			rtk_get_mib(interface_name, "multiap_bss_type", &bss_type, 1);
			if (bss_type & MULTI_AP_BACKHAUL_BSS_BIT || bss_type & MULTI_AP_BACKHAUL_STA_BIT) {
				*backhaul_interface_nr += 1;
				*backhaul_interfaces                               = (char **)realloc(*backhaul_interfaces, sizeof(char *) * (*backhaul_interface_nr));
				(*backhaul_interfaces)[*backhaul_interface_nr - 1] = strdup(interface_name);
				printf("Adding backhaul interface %s..\n", interface_name);
			}
		}
	}
}


void obtain_up_interfaces(char ***up_interfaces, uint8_t *up_interface_nr)
{
	char interface_name[32];

	int     i = 0, j = 0;

	for (i = 0; i < 3; i++) {
		sprintf(interface_name, "wlan%d", i);
		if (!is_interface_up(interface_name)) {
			continue;
		}
		*up_interface_nr += 1;
		*up_interfaces                               = (char **)realloc(*up_interfaces, sizeof(char *) * (*up_interface_nr));
		(*up_interfaces)[*up_interface_nr - 1] = strdup(interface_name);
		printf("Adding UP interface %s..\n", interface_name);
		for (j = 0; j < 5; j++) {
			sprintf(interface_name, "wlan%d-va%d", i, j);
			if (4 == j) {
				sprintf(interface_name, "wlan%d-vxd", i);
			}
			if (!is_interface_up(interface_name)) {
				continue;
			}
			*up_interface_nr += 1;
			*up_interfaces                               = (char **)realloc(*up_interfaces, sizeof(char *) * (*up_interface_nr));
			(*up_interfaces)[*up_interface_nr - 1] = strdup(interface_name);
			printf("Adding UP interface %s..\n", interface_name);
		}
	}
}

// This function put vxd interface name in the vxd_interface_name string, and return interface number that vxd is on, eg return 0 for wlan0-vxd, 1 for wlan1-vxd.
int obtain_vxd_interface_name(char *vxd_interface_name, size_t size)
{
	char  buf_tmp[512];
	FILE *brctl = popen("brctl show", "re");
	int   flag  = 0;
	char *ptr   = 0;

	if (brctl) {
		while (fgets(buf_tmp, 512, brctl) != 0) {
			// check bridge name, only br0 will set flag to 1
			if ((ptr = strstr(buf_tmp, "lxcbr"))) {
				flag = 0;
			} else if ((ptr = strstr(buf_tmp, "br"))) {
				flag = ('0' == *(ptr + 2)) ? 1 : 0;
			}

			if (!flag)
				continue;

			if ((ptr = strstr(buf_tmp, "-vxd"))) {
				ptr -= 5; // shift pointer to point to wlanx-vxd.
				if (ptr != strstr(ptr, "wlan")) {
					continue;
				}
				strlcpy(vxd_interface_name, ptr, size);
				printf("wlan%c-vxd found under bridge interface br0 \n", *(ptr + 4));
				pclose(brctl);
				return atoi(ptr + 4);
			}
		}

		printf("No vxd interface found under bridge interface br0 \n");
		pclose(brctl);
		return -1;
	}

	return -1;
}

/**
 * In vlan config, we need to take into consideration of the following case:
 * 	R1-vap  ---  WIFI  ---  vxd-R2/R3
 * In this case, even if the R2/R3 device receives policy config, the vxd cannot be configured with vlan, otherwise
 * backhaul connection will be lost as vap cannot connect with primary vlan interface, say vxd.10
 *
 * Use /var/wlan0-vxd/sta_info, /var/wlan1-vxd/sta_info, to determine the parent device profile number, the format in these files are:
 * For g6 driver:
 * 		multiap prifile: x
 * For 8192fe driver:
 * 		multiap_profile: x
 */

// This function returns -1 if file not found, or the profile number not found. Otherwise return current device's parent profile number.
int obtain_parent_profile()
{
	FILE *sta_info;
	char line[128];
	int parent_profile;
	char vxd_interface_name[512] = { 0 };
	char sta_info_path[128];
	char *profile_ptr;
	char discard[128];

	int vxd_number = obtain_vxd_interface_name(vxd_interface_name, sizeof(vxd_interface_name));

	if (vxd_number < 0 || vxd_number > 2) {
		printf("vxd_number must be 0, 1, or 2 \n");
		return -1;
	}

	sprintf(sta_info_path, "/var/wlan%d-vxd/sta_info", vxd_number);
	sta_info = fopen(sta_info_path, "re");

	if (sta_info == NULL) {
		printf("Failed to open sta_info \n");
		return -1;
	}

	while (fgets(line, 128, sta_info)) {
		if (strstr(line, "multiap") != NULL && (profile_ptr = strstr(line, "profile:")) != NULL) { // to deal with "multiap profile" and "multiap_profile".
			if (sscanf(profile_ptr, "%s %d", discard, &parent_profile)) {
				printf("vxd parent device is profile: [%d] \n", parent_profile);
				if (sta_info) {
					fclose(sta_info);
				}
				return parent_profile;
			}
		}
	}

	printf("obtain parent profile failed ! \n");

	if (sta_info) {
		fclose(sta_info);
	}

	return -1;
}

///////////////////////////////////////////////////////
// VLAN Helper Functions
///////////////////////////////////////////////////////

uint8_t append_vlan(struct vlan_setting *setting, char *interface_name, uint16_t vid)
{
	int i = 0, j = 0;
	for (i = 0; i < setting->vlan_group_nr; i++) {
		if (vid == setting->vlan_groups[i].vid) {
			break;
		}
	}

	if (i == setting->vlan_group_nr) {
		setting->vlan_group_nr += 1;
		setting->vlan_groups = (struct vlan_group *)realloc(setting->vlan_groups, sizeof(struct vlan_group) * setting->vlan_group_nr);
		if (NULL == setting->vlan_groups) {
			return 1;
		}
		setting->vlan_groups[i].vid           = vid;
		setting->vlan_groups[i].interface_nr  = 1;
		setting->vlan_groups[i].interfaces    = (char **)malloc(sizeof(char *) * 1);
		setting->vlan_groups[i].interfaces[0] = strdup(interface_name);
		return 0;
	}

	for (j = 0; j < setting->vlan_groups[i].interface_nr; j++) {
		if (0 == strcmp(setting->vlan_groups[i].interfaces[j], interface_name)) {
			return 0;
		}
	}

	setting->vlan_groups[i].interface_nr += 1;
	setting->vlan_groups[i].interfaces    = (char **)realloc(setting->vlan_groups[i].interfaces, sizeof(char *) * setting->vlan_groups[i].interface_nr);
	setting->vlan_groups[i].interfaces[j] = strdup(interface_name);

	return 0;
}

uint8_t is_interface_tagged(struct vlan_setting *vlan_config, char *interface_name)
{
	int i = 0, j = 0;
	for (i = 0; i < vlan_config->vlan_group_nr; i++) {
		for (j = 0; j < vlan_config->vlan_groups[i].interface_nr; j++) {
			if (0 == strcmp(interface_name, vlan_config->vlan_groups[i].interfaces[j])) {
				return 1;
			}
		}
	}
	return 0;
}

void obtain_untagged_interfaces(struct vlan_setting *vlan_config, char ***untagged_interfaces, uint8_t *untagged_interface_nr)
{
	char interface_name[32];

	int i = 0, j = 0;

	uint8_t bss_type = 0;

	sprintf(interface_name, "wlan0-vap0");

	for (i = 0; i < 3; i++) {
		for (j = -1; j < 4; j++) {
			if (-1 == j) {
				sprintf(interface_name, "wlan%d", i);
			} else {
				sprintf(interface_name, "wlan%d-vap%d", i, j);
			}

			if (!is_interface_up(interface_name)) {
				continue;
			}

			rtk_get_mib(interface_name, "multiap_bss_type", &bss_type, 1);

			if (bss_type & MULTI_AP_TEARDOWN_BIT || bss_type & MULTI_AP_BACKHAUL_BSS_BIT || bss_type & MULTI_AP_BACKHAUL_STA_BIT) {
				continue;
			}

			if (!is_interface_tagged(vlan_config, interface_name)) {
				*untagged_interface_nr += 1;
				*untagged_interfaces                               = (char **)realloc(*untagged_interfaces, sizeof(char *) * (*untagged_interface_nr));
				(*untagged_interfaces)[*untagged_interface_nr - 1] = strdup(interface_name);
			}
		}
	}
}

///////////////////////////////////////////////////////
// VLAN Setup Functions
///////////////////////////////////////////////////////

void single_ebtable_rule(char *interface1, char *interface2, const char *action)
{
	char cmd_buf[200];
	sprintf(cmd_buf, "ebtables -I map_portmapping -i %s -o %s -j %s\n", interface1, interface2, action);
	printf(cmd_buf);
	system(cmd_buf);
}

void dual_ebtable_rule(char *interface1, char *interface2, const char *action)
{
	single_ebtable_rule(interface1, interface2, action);
	single_ebtable_rule(interface2, interface1, action);
}

#if defined(WLAN_SMUX_DEV)
const char SMUXCTL[] = "/bin/smuxctl";
enum 
{
	SMUX_RULE_PRIO_QOS_HIGH=1100,
	SMUX_RULE_PRIO_PPTP_LAN_UNI=1000,
	SMUX_RULE_PRIO_IGMP=950,
	SMUX_RULE_PRIO_VEIP=900,
	SMUX_RULE_PRIO_PPTP=800,
	SMUX_RULE_PRIO_BCMC=700,
	SMUX_RULE_PRIO_BCMC_FILTER=780,
	SMUX_RULE_PRIO_BCMC_FILTER_DROP=770,
	SMUX_RULE_PRIO_BCMC_PPTP=750,
	SMUX_RULE_PRIO_BCMC_MVID=740,
	SMUX_RULE_PRIO_BCMC_RULE=730,
	SMUX_RULE_PRIO_MC_WHITE_LIST_PERMIT=720,
	SMUX_RULE_PRIO_MC_WHITE_LIST_DROP=710,
	SMUX_RULE_PRIO_VLAN_MAP=300,
	SMUX_RULE_PRIO_VLAN_GRP=200,
	SMUX_RULE_PRIO_PPTP_LAN_UNI_DEF=100,
};
#endif
char *add_vlan_interface(char *interface, uint16_t vid)
{
	char cmd_buf[250];

#if defined(WLAN_SMUX_DEV)
	int j;
	char real_ifname[IFNAMSIZ+1] = {0};
	char vlan_ifname[IFNAMSIZ+7] = {0};
	char alias_name[64] = {0};
#endif
	// sprintf(vlan_interface, "%s.%d", interface, vid);

	if (strstr(interface, "vxd")) {
		sprintf(cmd_buf, "brctl delif %s %s\n", map_brif, interface);
		printf(cmd_buf);
		system(cmd_buf);
	}
#if defined(WLAN_SMUX_DEV)
	if (strstr(interface, "eth")) {
		strncpy(real_ifname, interface , 6);
	}
	else
		strncpy(real_ifname, interface , IFNAMSIZ);
	sprintf(vlan_ifname, "%s.%d", real_ifname, vid);
	sprintf(cmd_buf, "%s --if-create-name %s %s", SMUXCTL, real_ifname,vlan_ifname);
	printf(cmd_buf);
	system(cmd_buf);
	sprintf(alias_name, "MESH_MAP_%s", real_ifname);
	for(j = 0; j <= 2; j++)
	{
		if(j == 0)
		{
			sprintf(cmd_buf, "%s --if %s --tx --tags 0 --filter-txif %s --push-tag --set-vid %d 1 --rule-alias %s --rule-priority %d --target ACCEPT --rule-insert", 
							SMUXCTL, real_ifname, vlan_ifname, vid, alias_name, SMUX_RULE_PRIO_VLAN_MAP);
			printf(cmd_buf);
			system(cmd_buf);
		}
		else
		{
			sprintf(cmd_buf, "%s --if %s --rx --tags %d --filter-vid %d %d --pop-tag --set-rxif %s --rule-alias %s --rule-priority %d --target ACCEPT --rule-insert", 
							SMUXCTL, real_ifname, j, vid, j, vlan_ifname, alias_name, SMUX_RULE_PRIO_VLAN_MAP);
			printf(cmd_buf);
			system(cmd_buf);
			sprintf(cmd_buf, "%s --if %s --tx --tags %d --filter-txif %s --set-vid %d %d --rule-alias %s --rule-priority %d --target ACCEPT --rule-insert", 
							SMUXCTL, real_ifname, j, vlan_ifname, vid, j, alias_name, SMUX_RULE_PRIO_VLAN_MAP);
			printf(cmd_buf);
			system(cmd_buf);
		}
	}
	sprintf(cmd_buf, "ifconfig %s up; brctl addif %s %s\n", vlan_ifname, map_brif, vlan_ifname);
#else
	sprintf(cmd_buf, "vconfig add %s %d; ifconfig %s.%d up; brctl addif %s %s.%d\n", interface, vid, interface, vid, map_brif, interface, vid);
#endif
	printf(cmd_buf);
	system(cmd_buf);
	return 0;
}

int bridge_name_init()
{
	struct easymesh_datamodel easymesh_db = {0};

	if (ini_parse(MULTIAP_CONF, read_config_file, &easymesh_db) < 0) {
		printf("ERROR [%s %d] Can't load configuration file!! \n",__FUNCTION__,__LINE__);
	} else {
		if (easymesh_db.bridge_name)
			snprintf(map_brif, sizeof(map_brif), "%s", easymesh_db.bridge_name);
	}
	return 0;
}

int main()
{
	FILE *  fp   = NULL;
	char *  line = NULL;
	size_t  len  = 0;
	ssize_t read;
	int     parent_profile;

	char interface_name[32];
	char cmd_buf[200];

	int i = 0, j = 0, k = 0, l = 0;

	struct vlan_setting global_vlan;
	global_vlan.primary_vid   = 0;
	global_vlan.vlan_group_nr = 0;
	global_vlan.vlan_groups   = NULL;

	char ** backhaul_interfaces   = NULL;
	uint8_t backhaul_interface_nr = 0;

	char ** untagged_interfaces   = NULL;
	uint8_t untagged_interface_nr = 0;

	char ** up_interfaces   = NULL;
	uint8_t up_interface_nr = 0;

	const char *backhaul_ethernet = "eth";
	char vlan_ifname[IFNAMSIZ+6] = {0};

	fp = fopen(VLAN_CONFIG_FILE, "r");
	if (fp == NULL) {
		printf("[luna_map_vlan][main] Read VLAN_CONFIG_FILE fail!!\n");
		exit(1);
	}

	bridge_name_init();

	const char *buf = "1";

	rtk_set_mib("wlan0", "multiap_vlan_enable", buf);
	rtk_set_mib("wlan1", "multiap_vlan_enable", buf);
	rtk_set_mib("wlan2", "multiap_vlan_enable", buf);

	while ((read = getline(&line, &len, fp)) != -1) {
		char* ssid;
		int  vid = 0;
		// sscanf(line, "%s %d", ssid, &vid);
		char* pt = strrchr(line, ' ');
		*pt = '\0';
		ssid = strdup(line);
		if(NULL == ssid) {
			exit(1);
		}
		char* vid_tmp = strdup(pt + 1);
		if(NULL == vid_tmp) {
			exit(1);
		}
		vid = atoi(vid_tmp);

		if (0 == strcmp(ssid, "Primary")) {
			global_vlan.primary_vid = vid;
			printf("Primary VID: %d\n", global_vlan.primary_vid);
		} else {
			printf("SSID: %s, VID: %d\n", ssid, vid);
			if (0 == ssid_to_interfacename(ssid, interface_name)) {
				printf("%s has SSID %s with VID %d\n", interface_name, ssid, vid);
				append_vlan(&global_vlan, interface_name, vid);
			}
		}

		if (ssid)
			free(ssid);
		if (vid_tmp)
			free(vid_tmp);
	}

	fclose(fp);

	printf("\n");

	//Clean existed vconfig
	fp = fopen(VLAN_RESET_FILE, "r");
	if(fp) {
		fclose(fp);
		system("sh /tmp/map_vlan_reset");
		system("rm /tmp/map_vlan_reset");
		system("rm /tmp/map_vlan_need_reset");
	}

	printf("\n----- Obtain parent device profile number -----\n\n");
	parent_profile = obtain_parent_profile();

	if (1 >= parent_profile) {
		printf("Parent device profile number: [%d], do NOT create vlan for vxd interface. \n", parent_profile);
	} else if (1 < parent_profile) {
		printf("Parent device profile number: [%d], need to create vlan for vxd interface. \n", parent_profile);
	}
	// Obtains backhaul interfaces
	obtain_backhaul_interfaces(&backhaul_interfaces, &backhaul_interface_nr, parent_profile);

	// Append ethernet interfaces
	append_ethernet_interfaces(&backhaul_interfaces, &backhaul_interface_nr);

	// Obtains untagged interfaces
	obtain_untagged_interfaces(&global_vlan, &untagged_interfaces, &untagged_interface_nr);
	for (i = 0; i < untagged_interface_nr; i++) {
		printf("Untagged Interface %d: %s\n", i, untagged_interfaces[i]);
	}

	// Disable bridge shortcut in wifi up interfaces
	obtain_up_interfaces(&up_interfaces, &up_interface_nr);
	for (i = 0; i < up_interface_nr; i++) {
		sprintf(cmd_buf, "iwpriv %s set_mib disable_brsc=1\n", up_interfaces[i]);
		printf(cmd_buf);
		system(cmd_buf);
	}

	printf("\n----- Adding VLAN Interfaces-----\n\n");

	// Add vlan interfaces
	fp = fopen(VLAN_RESET_FILE, "w");
	if (fp == NULL) {
		printf("[luna_map_vlan][main] Read VLAN_RESET_FILE fail!!\n");
		exit(1);
	}

	for (i = 0; i < global_vlan.vlan_group_nr; i++) {
		struct vlan_group *group = &global_vlan.vlan_groups[i];
		for (j = 0; j < backhaul_interface_nr; j++) {
			if (!(global_vlan.primary_vid == group->vid && strstr(backhaul_interfaces[j], backhaul_ethernet))) {
				add_vlan_interface(backhaul_interfaces[j], group->vid);
#if defined(WLAN_SMUX_DEV)
				if (strstr(backhaul_interfaces[j], "eth")) {
					snprintf(vlan_ifname, sizeof(vlan_ifname), "%.6s.%d", backhaul_interfaces[j], group->vid);
				}
				else
#endif
					snprintf(vlan_ifname, sizeof(vlan_ifname), "%s.%d", backhaul_interfaces[j], group->vid);
				fprintf(fp, "brctl delif %s %s\n", map_brif, vlan_ifname);
				fprintf(fp, "ifconfig %s down\n", vlan_ifname);
				#if defined(WLAN_SMUX_DEV)
				fprintf(fp, "%s --if-delete %s\n", SMUXCTL, vlan_ifname);
				#else
				fprintf(fp, "vconfig rem %s\n", vlan_ifname);
				#endif
				if (strstr(backhaul_interfaces[j], "wlan")) {
					fprintf(fp, "brctl addif %s %s\n", map_brif, backhaul_interfaces[j]);
				}
			}
		}
	}
	fprintf(fp, "ebtables -F map_portmapping\n");
	fprintf(fp, "ebtables -F STP_BR_CHECK\n");
	fprintf(fp, "ebtables -D INPUT -d 01:80:C2:00:00:00 -j STP_BR_CHECK\n");
	fprintf(fp, "iwpriv wlan0 set_mib multiap_vlan_enable=0\n");
	fprintf(fp, "iwpriv wlan1 set_mib multiap_vlan_enable=0\n");
	fprintf(fp, "iwpriv wlan2 set_mib multiap_vlan_enable=0\n");
	for (i = 0; i < backhaul_interface_nr; i++) {
		if (strstr(backhaul_interfaces[i], "wlan") && NULL == strstr(backhaul_interfaces[i], "vxd")) {
			fprintf(fp, "iwpriv %s set_mib multiap_vlan_id=0\n", backhaul_interfaces[i]);
		}
	}
	fprintf(fp, "rm /tmp/map_vlan_setting_configured\n");
	fclose(fp);

	// Setup ebtables
	system("ebtables -F map_portmapping");
	system("ebtables -F STP_BR_CHECK");
	system("ebtables -D INPUT -d 01:80:C2:00:00:00 -j STP_BR_CHECK");

	// Block vlan backhaul interface and other interfaces in a different vlan group (.10 backhaul vlan interface and (bss with vid 20))
	for (i = 0; i < global_vlan.vlan_group_nr; i++) {
		struct vlan_group *group = &global_vlan.vlan_groups[i];
		char               vlan_interface[64];
		char               vlan_interface_2[64];
		printf("\n----- Configuring vlan group %d -----\n\n", group->vid);
		for (j = 0; j < backhaul_interface_nr; j++) {
			for (k = 0; k < global_vlan.vlan_group_nr; k++) {
				if (i == k) {
					continue;
				}

				if (global_vlan.primary_vid == global_vlan.vlan_groups[i].vid && strstr(backhaul_interfaces[j], backhaul_ethernet)) {
					sprintf(vlan_interface, "%s", backhaul_interfaces[j]);
				} else {
#if defined(WLAN_SMUX_DEV)
					if (strstr(backhaul_interfaces[j], "eth")) {
						snprintf(vlan_interface, sizeof(vlan_interface), "%.6s.%d", backhaul_interfaces[j], global_vlan.vlan_groups[i].vid);
					}
					else
#endif
						sprintf(vlan_interface, "%s.%d", backhaul_interfaces[j], global_vlan.vlan_groups[i].vid);
				}

				for (l = 0; l < global_vlan.vlan_groups[k].interface_nr; l++) {
					dual_ebtable_rule(vlan_interface, global_vlan.vlan_groups[k].interfaces[l], "DROP");
				}

				if (global_vlan.primary_vid == global_vlan.vlan_groups[k].vid && strstr(backhaul_interfaces[j], backhaul_ethernet)) {
					sprintf(vlan_interface_2, "%s", backhaul_interfaces[j]);
				} else {
#if defined(WLAN_SMUX_DEV)
					if (strstr(backhaul_interfaces[j], "eth")) {
						snprintf(vlan_interface_2, sizeof(vlan_interface_2), "%.6s.%d", backhaul_interfaces[j], global_vlan.vlan_groups[k].vid);
					}
					else
#endif
						sprintf(vlan_interface_2, "%s.%d", backhaul_interfaces[j], global_vlan.vlan_groups[k].vid);
				}

				single_ebtable_rule(vlan_interface, vlan_interface_2, "DROP");

				if (global_vlan.primary_vid != global_vlan.vlan_groups[i].vid) {
					for (l = 0; l < untagged_interface_nr; l++) {
						dual_ebtable_rule(vlan_interface, untagged_interfaces[l], "DROP");
					}
				}
			}
		}
	}

	// Block vlan backhaul interfaces
	printf("\n----- Block vlan backhaul interfaces under different group -----\n\n");
	for (i = 0; i < global_vlan.vlan_group_nr; i++) {
		//struct vlan_group *group = &global_vlan.vlan_groups[i];
		char               vlan_interface[64];
		char               vlan_interface_2[64];
		for (j = 0; j < backhaul_interface_nr; j++) {
			if (global_vlan.primary_vid == global_vlan.vlan_groups[i].vid && strstr(backhaul_interfaces[j], backhaul_ethernet)) {
				sprintf(vlan_interface, "%s", backhaul_interfaces[j]);
			} else {
#if defined(WLAN_SMUX_DEV)
				if (strstr(backhaul_interfaces[j], "eth")) {
					snprintf(vlan_interface, sizeof(vlan_interface), "%.6s.%d", backhaul_interfaces[j], global_vlan.vlan_groups[i].vid);
				}
				else
#endif
					sprintf(vlan_interface, "%s.%d", backhaul_interfaces[j], global_vlan.vlan_groups[i].vid);
			}

			for (k = 0; k < global_vlan.vlan_group_nr; k++) {

				if (i == k) {
					continue;
				}

				for (l = 0; l < backhaul_interface_nr; l++) {
					if (l == j) {
						continue;
					}

					if (global_vlan.primary_vid == global_vlan.vlan_groups[k].vid && strstr(backhaul_interfaces[l], backhaul_ethernet)) {
						sprintf(vlan_interface_2, "%s", backhaul_interfaces[l]);
					} else {
#if defined(WLAN_SMUX_DEV)
						if (strstr(backhaul_interfaces[l], "eth")) {
							snprintf(vlan_interface_2, sizeof(vlan_interface_2), "%.6s.%d", backhaul_interfaces[l], global_vlan.vlan_groups[k].vid);
						}
						else
#endif
							sprintf(vlan_interface_2, "%s.%d", backhaul_interfaces[l], global_vlan.vlan_groups[k].vid);
					}

					single_ebtable_rule(vlan_interface, vlan_interface_2, "DROP");
				}
			}
		}
	}

	// Block interfaces under different group. (wlan0(vid 10) and wlan0-vap1(vid 20))
	printf("\n----- Block interfaces under different group -----\n\n");
	for (i = 0; i < global_vlan.vlan_group_nr; i++) {
		struct vlan_group *group = &global_vlan.vlan_groups[i];

		for (j = 0; j < group->interface_nr; j++) {
			for (k = 0; k < global_vlan.vlan_group_nr; k++) {
				if (i == k) {
					continue;
				}

				for (l = 0; l < global_vlan.vlan_groups[k].interface_nr; l++) {
					single_ebtable_rule(group->interfaces[j], global_vlan.vlan_groups[k].interfaces[l], "DROP");
				}
			}

			// Untag and Secondary Group Blocking
			if (global_vlan.primary_vid != global_vlan.vlan_groups[i].vid) {
				for (l = 0; l < untagged_interface_nr; l++) {
					dual_ebtable_rule(group->interfaces[j], untagged_interfaces[l], "DROP");
				}

				for (l = 0; l < backhaul_interface_nr; l++) {
					dual_ebtable_rule(group->interfaces[j], backhaul_interfaces[l], "DROP");
				}
			}
		}
	}

	for (i = 0; i < backhaul_interface_nr; i++) {
		if (strstr(backhaul_interfaces[i], "wlan")) {
			char cmd_buf[200];
			sprintf(cmd_buf, "ebtables -I map_portmapping -p 802.1Q -i %s -j DROP\n", backhaul_interfaces[i]);
			printf(cmd_buf);
			system(cmd_buf);
		}
	}

	// RETURN Rules

	// Block vlan backhaul interfaces
	printf("\n----- Allow communication between vlan backhaul interfaces with same VID -----\n\n");
	for (i = 0; i < global_vlan.vlan_group_nr; i++) {
		//struct vlan_group *group = &global_vlan.vlan_groups[i];
		char               vlan_interface[64];
		char               vlan_interface_2[64];
		for (j = 0; j < backhaul_interface_nr; j++) {
			if (global_vlan.primary_vid == global_vlan.vlan_groups[i].vid && strstr(backhaul_interfaces[j], backhaul_ethernet)) {
				sprintf(vlan_interface, "%s", backhaul_interfaces[j]);
			} else {
#if defined(WLAN_SMUX_DEV)
				if (strstr(backhaul_interfaces[j], "eth")) {
					snprintf(vlan_interface, sizeof(vlan_interface), "%.6s.%d", backhaul_interfaces[j], global_vlan.vlan_groups[i].vid);
				}
				else
#endif
					sprintf(vlan_interface, "%s.%d", backhaul_interfaces[j], global_vlan.vlan_groups[i].vid);
			}

			for (l = 0; l < backhaul_interface_nr; l++) {
				if (l == j) {
					continue;
				}

				if (global_vlan.primary_vid == global_vlan.vlan_groups[i].vid && strstr(backhaul_interfaces[l], backhaul_ethernet)) {
					sprintf(vlan_interface_2, "%s", backhaul_interfaces[l]);
				} else {
#if defined(WLAN_SMUX_DEV)
					if (strstr(backhaul_interfaces[l], "eth")) {
						snprintf(vlan_interface_2, sizeof(vlan_interface_2), "%.6s.%d", backhaul_interfaces[l], global_vlan.vlan_groups[i].vid);
					}
					else
#endif
						sprintf(vlan_interface_2, "%s.%d", backhaul_interfaces[l], global_vlan.vlan_groups[i].vid);
				}

				single_ebtable_rule(vlan_interface, vlan_interface_2, "RETURN");
			}

		}
	}

	printf("\n----- Allow communication between vlan interface and interfaces with same VID -----\n\n");
	for (i = 0; i < global_vlan.vlan_group_nr; i++) {
		struct vlan_group *group = &global_vlan.vlan_groups[i];
		char               vlan_interface[64];
		for (j = 0; j < backhaul_interface_nr; j++) {
			if (global_vlan.primary_vid == global_vlan.vlan_groups[i].vid && strstr(backhaul_interfaces[j], backhaul_ethernet)) {
				sprintf(vlan_interface, "%s", backhaul_interfaces[j]);
			} else {
#if defined(WLAN_SMUX_DEV)
				if (strstr(backhaul_interfaces[j], "eth")) {
					snprintf(vlan_interface, sizeof(vlan_interface), "%.6s.%d", backhaul_interfaces[j], global_vlan.vlan_groups[i].vid);
				}
				else
#endif
					sprintf(vlan_interface, "%s.%d", backhaul_interfaces[j], global_vlan.vlan_groups[i].vid);
			}

			for (k = 0; k < group->interface_nr; k++) {
				dual_ebtable_rule(vlan_interface, group->interfaces[k], "RETURN");
			}
		}
	}

	for (i = 0; i < backhaul_interface_nr; i++) {
		if (strstr(backhaul_interfaces[i], "wlan") && strstr(backhaul_interfaces[i], "vxd")) {
			sprintf(cmd_buf, "ifconfig %s.%d up\n", backhaul_interfaces[i], global_vlan.primary_vid);
			printf(cmd_buf);
			system(cmd_buf);
		}
	}

	if (1 == parent_profile) {
		printf("\n----- block communication between vxd and secondary vap interfaces for R1 direct child-----\n\n");

		char vxd_interface[512] = { 0 };
		obtain_vxd_interface_name(vxd_interface, sizeof(vxd_interface));

		struct vlan_group *group;

		for (i = 0; i < global_vlan.vlan_group_nr; i++) {
			group = &global_vlan.vlan_groups[i];
			if (group->vid != global_vlan.primary_vid) {
				for (j = 0; j < group->interface_nr; j++) {
					single_ebtable_rule(vxd_interface, group->interfaces[j], "DROP");
				}
			}
		}
	}

	// Avoid stp blocking ethernet secondary vlan group
	printf("\n----- Drop Ethernet secondary vlan group BPDU packets to avoid STP blocking interfaces -----\n\n");
	system("ebtables -N STP_BR_CHECK");
	system("ebtables -P STP_BR_CHECK RETURN");
	system("ebtables -A INPUT -d 01:80:C2:00:00:00 -j STP_BR_CHECK");
	for (i = 0; i < global_vlan.vlan_group_nr; i++) {
		char               vlan_interface[64];
		for (j = 0; j < backhaul_interface_nr; j++) {
			if (global_vlan.primary_vid != global_vlan.vlan_groups[i].vid && strstr(backhaul_interfaces[j], backhaul_ethernet)) {
				snprintf(vlan_interface, sizeof(vlan_interface), "%.6s.%d", backhaul_interfaces[j], global_vlan.vlan_groups[i].vid);
				char cmd_buf[200];
				sprintf(cmd_buf, "ebtables -A STP_BR_CHECK --logical-in %s -i %s -j DROP\n", map_brif, vlan_interface);
				printf(cmd_buf);
				system(cmd_buf);
			}

		}
	}

	(void)rename("/tmp/map_vlan_setting", "/tmp/map_vlan_setting_configured");
	return 0;
}
