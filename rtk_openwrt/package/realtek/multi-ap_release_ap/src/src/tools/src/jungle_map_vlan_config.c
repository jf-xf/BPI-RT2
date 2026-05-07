#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>      // ioctl()
#include <sys/socket.h>     // socket()
#include <linux/wireless.h> // struct iwreq
#include <unistd.h>
#include <stdint.h>
#include <errno.h>          // errno

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

///////////////////////////////////////////////////////
// SDK Helper Functions
///////////////////////////////////////////////////////

int rtk_is_ax_interface(const char *interface)
{
	char filename[64] = { 0 };
	int  is_ax        = 0;

	snprintf(filename, sizeof(filename), "/proc/net/rtl8852ae/%s", interface);
	if (-1 != access(filename, F_OK))
		is_ax = 1;

	snprintf(filename, sizeof(filename), "/proc/net/rtk_wifi6/%s", interface);
	if (-1 != access(filename, F_OK))
		is_ax = 2;

	return is_ax;
}

uint8_t is_interface_up(char *interface)
{
	int          skfd = 0;
	struct ifreq ifr;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (skfd < 0) {
        printf("ioctl [RTL8192CD_IOCTL_GET_MIB] socket error\n");
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

#define RTL8192CD_IOCTL_SET_MIB 0x89f1
uint8_t rtk_set_mib(const char *interface_name, const char *item_str, const char *value_str)
{
	int          s;
	int          cmd_id, is_ax_interface = 0;
	struct iwreq wrq;
	struct ifreq rq;
	static char  mib_str_buf[512];
	snprintf(mib_str_buf, 512, "%s=%s", item_str, value_str);
	printf("Set mib for %s: %s\n", interface_name, mib_str_buf);

	is_ax_interface = rtk_is_ax_interface(interface_name);

	/*** Initialize socket ***/
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (-1 == s) {
		printf("socket('%s') returned with errno=%d (%s) while opening a DGRAM socket\n", interface_name, errno, strerror(errno));
		return 0;
	}

	/*** Initialize struct iwreq ***/
	memset(&wrq, 0, sizeof(wrq));
	strncpy(wrq.ifr_name, interface_name, IFNAMSIZ - 1);

	if (is_ax_interface) {
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
	int skfd;
	int cmd_id, is_ax_interface = 0;

	struct iwreq wrq;
	struct ifreq rq;
	char         tmp[30];

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0) {
		printf("ioctl [RTL8192CD_IOCTL_GET_MIB] socket error\n");
		return 0;
	}

	is_ax_interface = rtk_is_ax_interface(interfacename);

	memset(&wrq, 0, sizeof(wrq));
	strlcpy(wrq.ifr_name, interfacename, sizeof(wrq.ifr_name));
	strlcpy(tmp, mibname, sizeof(tmp));

	wrq.u.data.pointer = tmp;

	if (is_ax_interface) {
		wrq.u.data.flags  = RTL8192CD_IOCTL_GET_MIB;
		wrq.u.data.length = strlen(tmp) + 1;
		cmd_id            = SIOCDEVPRIVATEAXEXT;
	} else {
		cmd_id            = RTL8192CD_IOCTL_GET_MIB;
		wrq.u.data.length = strlen(tmp);
	}
	memcpy(&rq, &wrq, sizeof(struct iwreq));
	/* Do the request */
	if (ioctl(skfd, cmd_id, &rq) < 0) {
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
			strcpy(buf_tmp, interface);
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

void obtain_backhaul_interfaces(char ***backhaul_interfaces, uint8_t *backhaul_interface_nr)
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
			sprintf(interface_name, "wlan%d-vap%d", i, j);
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

char *add_vlan_interface(char *interface, uint16_t vid)
{
	char cmd_buf[200];

	// sprintf(vlan_interface, "%s.%d", interface, vid);

	if (strstr(interface, "vxd")) {
		sprintf(cmd_buf, "brctl delif br0 %s\n", interface);
		printf(cmd_buf);
		system(cmd_buf);
	}
	sprintf(cmd_buf, "vconfig add %s %d; ifconfig %s.%d up; brctl addif br0 %s.%d\n", interface, vid, interface, vid, interface, vid);
	printf(cmd_buf);
	system(cmd_buf);
	return 0;
}

int main()
{
	FILE *  fp   = NULL;
	char *  line = NULL;
	size_t  len  = 0;
	ssize_t read;

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

	if (-1 == access(VLAN_CONFIG_FILE, F_OK)) {
		exit(1);
	}

	const char *buf = "1";

	rtk_set_mib("wlan0", "multiap_vlan_enable", buf);
	rtk_set_mib("wlan1", "multiap_vlan_enable", buf);
	rtk_set_mib("wlan2", "multiap_vlan_enable", buf);

	system("echo write 0xbb804A00 0x80000000 >/proc/rtl865x/memory");

	fp = fopen(VLAN_CONFIG_FILE, "r");
	if (fp == NULL) {
		printf("[multi_ap error][%s] Open file[%s] fail!\n", __FUNCTION__, VLAN_CONFIG_FILE);
		exit(1);
	}
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

		free(ssid);
		free(vid_tmp);
	}

	fclose(fp);

	printf("\n");

	//Clean existed vconfig

	if(-1 != access("/tmp/map_vlan_reset", F_OK) ) {
		system("sh /tmp/map_vlan_reset");
		system("rm /tmp/map_vlan_reset");
		system("rm /tmp/map_vlan_need_reset");
	}

	// Obtains backhaul interfaces
	obtain_backhaul_interfaces(&backhaul_interfaces, &backhaul_interface_nr);

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
	// Disable bridge shortcut in all interface
	sprintf(cmd_buf, "echo disable > /proc/rtl865x/brsc\n");
	printf(cmd_buf);
	system(cmd_buf);

	printf("\n----- Adding VLAN Interfaces-----\n\n");

	// Add vlan interfaces
	fp = fopen(VLAN_RESET_FILE, "w");
	if (fp == NULL) {
		printf("[multi_ap error][%s] Open file[%s] fail!\n", __FUNCTION__, VLAN_RESET_FILE);
		exit(1);
	}
	fprintf(fp, "echo write 0xbb804A00 0x00000000 >/proc/rtl865x/memory\n");
	for (i = 0; i < global_vlan.vlan_group_nr; i++) {
		struct vlan_group *group = &global_vlan.vlan_groups[i];
		for (j = 0; j < backhaul_interface_nr; j++) {
			if (!(global_vlan.primary_vid == group->vid && strstr(backhaul_interfaces[j], backhaul_ethernet))) {
				add_vlan_interface(backhaul_interfaces[j], group->vid);
				fprintf(fp, "brctl delif br0 %s.%d\n", backhaul_interfaces[j], group->vid);
				fprintf(fp, "ifconfig %s.%d down\n", backhaul_interfaces[j], group->vid);
				fprintf(fp, "vconfig rem %s.%d\n", backhaul_interfaces[j], group->vid);
				if (strstr(backhaul_interfaces[j], "wlan")) {
					fprintf(fp, "brctl addif br0 %s\n", backhaul_interfaces[j]);
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
	fprintf(fp, "echo \"0\" > /proc/net/vlan/multiapVlanEnable\n");
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
					sprintf(vlan_interface, "%s.%d", backhaul_interfaces[j], global_vlan.vlan_groups[i].vid);
				}

				for (l = 0; l < global_vlan.vlan_groups[k].interface_nr; l++) {
					dual_ebtable_rule(vlan_interface, global_vlan.vlan_groups[k].interfaces[l], "DROP");
				}

				if (global_vlan.primary_vid == global_vlan.vlan_groups[k].vid && strstr(backhaul_interfaces[j], backhaul_ethernet)) {
					sprintf(vlan_interface_2, "%s", backhaul_interfaces[j]);
				} else {
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
				sprintf(vlan_interface, "%s.%d", backhaul_interfaces[j], global_vlan.vlan_groups[i].vid);
			}

			for (l = 0; l < backhaul_interface_nr; l++) {
				if (l == j) {
					continue;
				}

				if (global_vlan.primary_vid == global_vlan.vlan_groups[i].vid && strstr(backhaul_interfaces[l], backhaul_ethernet)) {
					sprintf(vlan_interface_2, "%s", backhaul_interfaces[l]);
				} else {
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
				sprintf(vlan_interface, "%s.%d", backhaul_interfaces[j], global_vlan.vlan_groups[i].vid);
			}

			for (k = 0; k < group->interface_nr; k++) {
				dual_ebtable_rule(vlan_interface, group->interfaces[k], "RETURN");
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
				sprintf(cmd_buf, "ebtables -A STP_BR_CHECK --logical-in br0 -i %s -j DROP\n", vlan_interface);
				printf(cmd_buf);
				system(cmd_buf);
			}

		}
	}

	//Set multiap_linux_vlan_enable
	printf("\n----- Set multiap_linux_vlan_enable in /proc/net/vlan/multiapVlanEnable -----\n\n");
	system("echo \"1\" > /proc/net/vlan/multiapVlanEnable");

	rename("/tmp/map_vlan_setting", "/tmp/map_vlan_setting_configured");
	return 0;
}