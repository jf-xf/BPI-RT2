/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <ctype.h>
#include <dirent.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <sys/inotify.h>
#include "map_backhaul.h"
#include "ini.h"
#include "config_file_handler.h"

#include "map_utils.h"
#include "map_commands.h"

#include <fcntl.h>
#include <dirent.h>

#include <netinet/if_ether.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <features.h>
#include <errno.h>

#ifdef CONFIG_RTK_PON_XDSL
#include <sys/wait.h>
#else
#include "mib.h"
#include "sysconfig.h"
#endif // CONFIG_RTK_PON_XDSL

typedef enum { IP_ADDR, DST_IP_ADDR, SUBNET_MASK, DEFAULT_GATEWAY, HW_ADDR } ADDR_T; //same to mib.h
#define MAC_BCAST_ADDR		(unsigned char *) "\xff\xff\xff\xff\xff\xff" //same to mib.h
enum BACKHAUL_LINK { BACKHAUL_INVALID, BACKHAUL_ETH, BACKHAUL_WIFI };

struct ezmesh_arpMsg {
	struct ethhdr ethhdr;                   /* Ethernet header */
	u_short htype;                          /* hardware type (must be ARPHRD_ETHER) */
	u_short ptype;                          /* protocol type (must be ETH_P_IP) */
	u_char  hlen;                           /* hardware address length (must be 6) */
	u_char  plen;                           /* protocol address length (must be 4) */
	u_short operation;                      /* ARP opcode */
	u_char  sHaddr[6];                      /* sender's hardware address */
	u_char  sInaddr[4];                     /* sender's IP address */
	u_char  tHaddr[6];                      /* target's hardware address */
	u_char  tInaddr[4];                     /* target's IP address */
	u_char  pad[18];                        /* pad for min. Ethernet payload (60 bytes) */
}; //same to mib.h

#define READ_BUF_SIZE 50

extern char *optarg;			/* getopt */
static int map_checker_debug = 0;
static int max_send_arp = 3;
char controller_ip[READ_BUF_SIZE];
char map_brif[READ_BUF_SIZE] = "br-lan"; //default bridge_name is br-lan

#define MULTIAP_CONF			"/var/multiap.conf"
#define MULTIAP_MIB_CONF		"/var/multiap_mib.conf"
#define MAP_EXT_DATA			"/var/map_ext_data.conf"

#define MAP_CHECKER_DBG(fmt, args...)	if(map_checker_debug == 1) printf("[map-checker-dbg]"fmt, ##args)

#define DYNAMIC_DEBUG_FLAG		"/tmp/map_checker_debug"
#define DEF_PID_FILENAME		"/var/run/map_checker.pid"
#define BACKHAUL_LINK_NOTIFY_INTERVAL 60

#define MAC_ADDR_LEN 6

#define ETH_CONNECTED_INTERFACE "/tmp/map_eth_connected_interface"

const char MAP_DHCPC_PID[] = "/var/run/udhcpc.pid";

struct eth_interface_info
{
	int 	  eth_interface_nr;
	char **   eth_interface_names;
};

struct backhaul_link_detect_info{
	int backhaul_link_detected;
	int link_down_count;
	int discovery_down_count;
	int need_triger;
	int backhaul_link_is_wifi;
	int backhaul_link_is_eth;
	int current_heartbeat_count;
	int last_heartbeat_count;
	int count;
	char controller_ip[READ_BUF_SIZE];
};

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

/*
 * Appends src to string dst of size dsize (unlike strncat, dsize is the
 * full size of dst, not space left).  At most dsize-1 characters
 * will be copied.  Always NUL terminates (unless dsize <= strlen(dst)).
 * Returns strlen(src) + MIN(dsize, strlen(initial dst)).
 * If retval >= dsize, truncation occurred.
 */
size_t strlcat(char *dst, const char *src, size_t dsize)
{
        const char *odst = dst;
        const char *osrc = src;
        size_t n = dsize;
        size_t dlen;

        /* Find the end of dst and adjust bytes left but don't go past end. */
        while (n-- != 0 && *dst != '\0')
                dst++;
        dlen = dst - odst;
        n = dsize - dlen;

        if (n-- == 0)
                return(dlen + strlen(src));
        while (*src != '\0') {
                if (n != 0) {
                        *dst++ = *src;
                        n--;
                }
                src++;
        }
        *dst = '\0';

        return(dlen + (src - osrc));    /* count does not include NUL */
}
#endif // CONFIG_RTK_PON_XDSL

int getMacAddr(char* ifname, unsigned char* macaddr)
{
    int s;
    struct ifreq ifr;

    if(ifname==NULL || strlen(ifname)==0 || macaddr==NULL){
    printf("%s: parameter error\n", __FUNCTION__);
        return -1;
    }

    s = socket(PF_PACKET, SOCK_DGRAM, 0);

    if (s < 0) {
        printf("%s: socket error\n", __FUNCTION__);
        return -1;
    }

    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
    if (ioctl(s, SIOCGIFHWADDR, &ifr) < 0) {
        printf("%s: Interface %s not found\n", __FUNCTION__, ifname);
        close(s);
        return -1;
    }
    close(s);

    if (ifr.ifr_hwaddr.sa_family!=ARPHRD_ETHER) {
        printf("%s: not an Ethernet interface\n", __FUNCTION__);
        return -1;
    }
    memcpy(macaddr, ifr.ifr_hwaddr.sa_data, MAC_ADDR_LEN);

    //printf("%s: %02X:%02X:%02X:%02X:%02X:%02X\n", __FUNCTION__,
    //	macaddr[0],macaddr[1],macaddr[2],macaddr[3],macaddr[4],macaddr[5]);
    return 0;
}

pid_t find_pid_by_name(char *pidName)
{
	DIR *          dir;
	struct dirent *next;
	pid_t          pid;

	dir = opendir("/proc");
	if (!dir) {
		printf("Cannot open /proc");
		return 0;
	}

	while ((next = readdir(dir)) != NULL) {
		FILE *status;
		char  filename[READ_BUF_SIZE];
		char  buffer[READ_BUF_SIZE];
		char  name[READ_BUF_SIZE];
		int   pid_num = 0;

		/* Must skip ".." since that is outside /proc */
		if (strcmp(next->d_name, "..") == 0)
			continue;

		/* If it isn't a number, we don't want it */
		if (!isdigit(*next->d_name))
			continue;

		sscanf(next->d_name, "%d", &pid_num);

		snprintf(filename, sizeof(filename), "/proc/%d/status", pid_num);
		if (!(status = fopen(filename, "r"))) {
			continue;
		}
		if (fgets(buffer, READ_BUF_SIZE - 1, status) == NULL) {
			fclose(status);
			continue;
		}
		fclose(status);

		/* Buffer should contain a string like "Name:   binary_name" */
		sscanf(buffer, "%*s %s", name);
		if (strcmp(name, pidName) == 0) {
			//	pidList=xrealloc( pidList, sizeof(pid_t) * (i+2));
			pid = (pid_t)strtol(next->d_name, NULL, 0);
			closedir(dir);
			return pid;
		}
	}
	closedir(dir);
	return 0;
}

int get_net_link_status(const char *ifname)
{
	struct ifreq         ifr;
	struct ethtool_value edata;
	int                  ret;
	int                  skfd;

	if (strlen(ifname) >= 16)
		return -1;

	strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	edata.cmd    = ETHTOOL_GLINK;
	ifr.ifr_data = (caddr_t)&edata;

	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return (-1);
	}
	ret = ioctl(skfd, SIOCETHTOOL, &ifr);
	close(skfd);

	if (ret == 0)
		ret = edata.data;
	return ret;
}

uint8_t is_eth_connected(char *eth_interface_name)
{
	char    file_name_buf[64] = {0};
	uint8_t eth_connected = 0;

	char buf[3];
	int fd;

	FILE *fd_eth_link;

	snprintf(file_name_buf, sizeof(file_name_buf), "/proc/%s/link_status", eth_interface_name);

	fd = open(file_name_buf, O_RDONLY);
	if (fd < 0) {
		return get_net_link_status(eth_interface_name);
	}

	fd_eth_link = fopen(file_name_buf, "r");

	if (fd_eth_link == NULL) {
		printf("*is_eth_connected* fopen() returned with errno=%d (%s)\n", errno, strerror(errno));
		close(fd);
		return 2;
	}
	fgets(buf, sizeof(buf), fd_eth_link);
	fclose(fd_eth_link);

	if (buf[0] == '1') {
		eth_connected = 1;
	}
	close(fd);
	return eth_connected;
}

uint8_t is_interface_up(char *interface)
{
	int          skfd = 0;
	struct ifreq ifr;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0) {
		printf("Failed to create socket...\n");
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

void append_br_interfaces(char *bridge_name, char *interfaces)
{
	char buf_tmp[512];

	FILE *brctl = popen("brctl show", "r");

	int   flag      = 0;
	char *ptr       = 0;
	char *interface = 0;

	if (brctl) {
		while (fgets(buf_tmp, 512, brctl) != 0) {
			ptr = buf_tmp;
			if (*ptr >= ' ') {
				if (bridge_name && (ptr = strstr(buf_tmp, bridge_name))) {
					// Check whether it exactly equal to bridge name. Note: 9 is Tab, 32 is Space
					flag = ((ptr == buf_tmp || 9 == *(ptr -1) || 32 == *(ptr -1)) &&
							(9 == *(ptr + strlen(bridge_name)) || 32 == *(ptr + strlen(bridge_name))))? 1: 0;
				}
				else
					flag = 0;
			}

			if (!flag)
				continue;

			interface = NULL;
			if ((ptr = strstr(buf_tmp, "veth"))) {
				interface = ptr;
			} else if ((ptr = strstr(buf_tmp, "eth"))) {
				interface = ptr;
			} else if ((ptr = strstr(buf_tmp, "nas"))) {
				interface = ptr;
			} else if ((ptr = strstr(buf_tmp, "wlan"))) {
				interface = ptr;
			} else if ((ptr = strstr(buf_tmp, "veip0"))) {
				interface = ptr;
			}

			if (interface) {
				strlcpy(buf_tmp, interface, sizeof(buf_tmp));
				buf_tmp[strcspn(buf_tmp, "\r\n")] = ',';
				if (strstr(interfaces, buf_tmp)) {
					continue;
				}
				strlcat(buf_tmp, interfaces, sizeof(buf_tmp));
				memcpy(interfaces, buf_tmp, 512);
			}
		}
		pclose(brctl);
	}
}

void get_eth_interfaces(struct eth_interface_info *eth_interface)
{
	char *interfaces = (char*)malloc(512);
	if(interfaces == NULL){
		return;
	}
	memset(interfaces, 0, 512);

	append_br_interfaces(map_brif, interfaces);

	char *token = strtok(interfaces, ",");

	while (token != NULL) {
		token = strtok(NULL, ",");
		if (token && (strstr(token, "eth") || strstr(token, "nas") || strstr(token, "veip0"))) {
			eth_interface->eth_interface_nr += 1;
			eth_interface->eth_interface_names = (char **)realloc(eth_interface->eth_interface_names, sizeof(char *) * eth_interface->eth_interface_nr);
			eth_interface->eth_interface_names[eth_interface->eth_interface_nr - 1] = strdup(token);
			char *pos = NULL;
			if ((pos = strchr(eth_interface->eth_interface_names[eth_interface->eth_interface_nr - 1], '\n')) != NULL)
				*pos = '\0';
		}
	}
	free(interfaces);

	return;
}

int get_current_backhaul_radio (int *backhaul_sta_radio_index)
{
	int line=0,line_2g=0,line_5g=0,line_backhaul=0;
	int radio_index_5g=-1, radio_index_2g=-1;
	char buf1[256] = { 0 };
	char buf2[256] = { 0 };

	FILE *fd = fopen(MULTIAP_CONF, "r");
	if (fd == NULL) {
		printf("ERROR (%s)%d Open %s file error.\n", __FUNCTION__, __LINE__, MULTIAP_CONF);
		return -1;
	}
	while(fgets(buf1,sizeof(buf1),fd)!=NULL)
	{
		if(strstr(buf1,"radio_name_5g")){
			radio_index_5g = buf1[21] - '0';
			MAP_CHECKER_DBG("[%s %d],buf1:%s,radio_index_5g:%d\n",__FUNCTION__,__LINE__,buf1,radio_index_5g);
		}else if(strstr(buf1,"radio_name_24g")){
			radio_index_2g = buf1[21] - '0';
			MAP_CHECKER_DBG("[%s %d],buf1:%s,radio_index_2g:%d\n",__FUNCTION__,__LINE__,buf1,radio_index_2g);
		}
	}
	fclose(fd);

	FILE *fp = fopen(MULTIAP_MIB_CONF, "r");
	if (fp == NULL) {
		printf("ERROR (%s)%d Open %s file error.\n", __FUNCTION__, __LINE__, MULTIAP_MIB_CONF);
		return -1;
	}

	while(fgets(buf2,sizeof(buf2),fp)!=NULL)
	{
		line ++;
		if(strstr(buf2,"mib_info_2.4g")){
			line_2g = line;
			MAP_CHECKER_DBG("[%s %d],buf2:%s,line:%d,line_2g:%d\n",__FUNCTION__,__LINE__,buf2,line,line_2g);
		}else if(strstr(buf2,"mib_info_5g")){
			line_5g = line;
			MAP_CHECKER_DBG("[%s %d],buf2:%s,line:%d,line_5g:%d\n",__FUNCTION__,__LINE__,buf2,line,line_5g);
		}else if(strstr(buf2,"network_type") && strstr(buf2,"128")){
			line_backhaul = line;
			MAP_CHECKER_DBG("[%s %d],buf2:%s,line:%d,line_backhaul:%d\n",__FUNCTION__,__LINE__,buf2,line,line_backhaul);
		}
	}

	if(line_2g < line_5g){
		if(line_backhaul > line_5g){
			*backhaul_sta_radio_index = radio_index_5g;
			MAP_CHECKER_DBG("[%s %d]\n",__FUNCTION__,__LINE__);
		}else{
			*backhaul_sta_radio_index = radio_index_2g;
			MAP_CHECKER_DBG("[%s %d]\n",__FUNCTION__,__LINE__);
		}
	}else{
		if(line_backhaul > line_2g){
			*backhaul_sta_radio_index = radio_index_2g;
			MAP_CHECKER_DBG("[%s %d]\n",__FUNCTION__,__LINE__);
		}else{
			*backhaul_sta_radio_index = radio_index_5g;
			MAP_CHECKER_DBG("[%s %d]\n",__FUNCTION__,__LINE__);
		}
	}
	MAP_CHECKER_DBG("[%s %d],backhaul_sta_radio_index:%d\n",__FUNCTION__,__LINE__,*backhaul_sta_radio_index);

	fclose(fp);

	return 0;
}

int dynamic_debug_enable()
{
	int  ret = 0;
	int debug_level = '0';

	FILE *fp = fopen(DYNAMIC_DEBUG_FLAG, "r");
	if (fp == NULL) {
		//printf("ERROR (%s)%d Open %s file error.\n", __FUNCTION__, __LINE__, DYNAMIC_DEBUG_FLAG);
		return ret;
	}

	debug_level = fgetc(fp);
	fclose(fp);
	if (debug_level >= '1') {
		ret = 1;
	}else{
		ret = 0;
	}

	return ret;
}

int is_valid_ip(const char *src)
{
	char buf[16];
	if (inet_pton(AF_INET, src, buf)) {
		return 1;
	}

	return 0;
}

#ifdef CONFIG_RTK_PON_XDSL
#define SR_ENVP_MAP_DHCP_RENEW			"reason=multi_ap_dhcp_renew"
#define SR_SCRIPT						"/var/multi_ap_script"  /* Script name */
void _rtk_syscall_map_dhcp_renew(void)
{
	char *argv[] = {SR_SCRIPT, (char *)0};
	char *envp[] = {SR_ENVP_MAP_DHCP_RENEW, (char *)0};
	char script[] = SR_SCRIPT;
	int  pid = fork();

	if(pid < 0) {
		printf("[%s] fork() failed\n", __FUNCTION__);
	} else if(pid) {
		wait(0);
		printf("[%s] execve() done\n", __FUNCTION__);
	} else {
		//system("sleep 5");
		if(execve(script, argv, envp) < 0) {
			printf("[%s] execve() failed\n", __FUNCTION__);
			exit(0);
		}
	}
}
#else
int map_read_pid(const char *filename)
{
	FILE *fp;
	int pid = 0;

	if ((fp = fopen(filename, "r")) == NULL)
		return -1;
	if(fscanf(fp, "%d", &pid) != 1 || kill(pid, 0) != 0){
		pid = 0;
	}
	fclose(fp);

	return pid;
}
#endif // CONFIG_RTK_PON_XDSL

void trigger_dhcp_renew(){
	MAP_CHECKER_DBG("[%s %d] controller ip changed, trigger dhcp renew!!!!!\n",__FUNCTION__,__LINE__);
	//trigger dhcp renew;
#if CONFIG_RTK_PON_XDSL
	_rtk_syscall_map_dhcp_renew();
#else
	int dhcpc_pid=0;
	char pid_file[72]={0};
	snprintf(pid_file, sizeof(pid_file), "%s.%s", (char*)MAP_DHCPC_PID, map_brif);
	dhcpc_pid = map_read_pid(pid_file);
	if (dhcpc_pid > 0)
	{
		kill(dhcpc_pid, SIGUSR1);
	}else{
		printf("[%s %d] error: invalid dhcpc_pid: %d!!!\n",__FUNCTION__,__LINE__,dhcpc_pid);
	}
#endif // CONFIG_RTK_PON_XDSL
	MAP_CHECKER_DBG("[%s %d] dhcp renew signal send!!!!!\n",__FUNCTION__,__LINE__);
}

void check_controller_ip_renew()
{
	int current_heart_beat;
	char buf_backhaul_link_if[READ_BUF_SIZE] = { 0 };
	char tmp_backhaul_link_info[READ_BUF_SIZE] = { 0 };
	char tmp_controller_ip[READ_BUF_SIZE] = {0};

	FILE *fp = fopen(BACKHAUL_LINK_INFO, "a+");
	if (fp == NULL) {
		printf("ERROR (%s)%d Open %s file error.\n", __FUNCTION__, __LINE__, BACKHAUL_LINK_INFO);
		return;
	}
	fgets(tmp_backhaul_link_info, READ_BUF_SIZE, fp);
	fclose(fp);

	sscanf(tmp_backhaul_link_info,"%s %d %s", buf_backhaul_link_if, &current_heart_beat, tmp_controller_ip);

	MAP_CHECKER_DBG("[%s %d] controller_ip:%s\n",__FUNCTION__,__LINE__,is_valid_ip(controller_ip) ? controller_ip : "NULL" );
	MAP_CHECKER_DBG("[%s %d] tmp_backhaul_link_info:%s\n",__FUNCTION__,__LINE__,tmp_backhaul_link_info);

	if(is_valid_ip(tmp_controller_ip)){
		if(!is_valid_ip(controller_ip)){
			printf("===[%s %d]=== received tmp_controller_ip:%s, record\n",__FUNCTION__,__LINE__,tmp_controller_ip);
			snprintf(controller_ip, sizeof(controller_ip), "%s", tmp_controller_ip);
		}else{
			if(memcmp(tmp_controller_ip, controller_ip, READ_BUF_SIZE)){
				//trigger dhcp renew
				printf("===[%s %d]=== tmp_controller_ip:%s, controller_ip:%s,trigger dhcp renew!!\n",__FUNCTION__,__LINE__,tmp_controller_ip,controller_ip);
				snprintf(controller_ip, sizeof(controller_ip), "%s", tmp_controller_ip);
				trigger_dhcp_renew();
			}
		}
	}
	return;
}

// Mason Yu. 090903
int do_ioctl(unsigned int cmd, struct ifreq *ifr)
{
	int skfd, ret;

	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return (-1);
	}

	ret = ioctl(skfd, cmd, ifr);
	close(skfd);
	return ret;
}

int _getInAddr(char *interface, ADDR_T type, void *pAddr)
{
	struct ifreq ifr;
	int found=0;
	struct sockaddr_in *addr;

	strlcpy(ifr.ifr_name, interface, sizeof(ifr.ifr_name));
	if (do_ioctl(SIOCGIFFLAGS, &ifr) < 0)
		return (0);

	if (type == HW_ADDR) {
		if (do_ioctl(SIOCGIFHWADDR, &ifr) >= 0) {
			memcpy(pAddr, &ifr.ifr_hwaddr, sizeof(struct sockaddr));
			found = 1;
		}
	}
	else if (type == IP_ADDR) {
		if (do_ioctl(SIOCGIFADDR, &ifr) == 0) {
			addr = ((struct sockaddr_in *)&ifr.ifr_addr);
			*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);
			found = 1;
		}
	}
	else if (type == DST_IP_ADDR) {
		if (do_ioctl(SIOCGIFDSTADDR, &ifr) == 0) {
			addr = ((struct sockaddr_in *)&ifr.ifr_addr);
			*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);
			found = 1;
		}
	}
	else if (type == SUBNET_MASK) {
		if (do_ioctl(SIOCGIFNETMASK, &ifr) >= 0) {
			addr = ((struct sockaddr_in *)&ifr.ifr_addr);
			*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);
			found = 1;
		}
	}
	return found;
}

void pidfile_write()
{
	FILE *fd = fopen(DEF_PID_FILENAME, "w+");
	if (NULL == fd) {
		printf("Could not create tmp file map_checker.pid\n");
		return;
	}
	fprintf(fd, "%d\n", getpid());
	fclose(fd);

	return;
}

static void down_vxd(){
	char cmd[128] = { 0 };
	char ifname[13] = { 0 };
	int backhaul_sta_radio_index = -1;

	get_current_backhaul_radio (&backhaul_sta_radio_index);

	snprintf(ifname,sizeof(ifname),"wlan%d-vxd",backhaul_sta_radio_index);
	printf("[%s %d] backhaul_sta_radio_index:%d, ifname:%s\n",__FUNCTION__,__LINE__,backhaul_sta_radio_index,ifname);
	if(is_interface_up(ifname)){
		snprintf(cmd, sizeof(cmd), "ifconfig %s down", ifname);
		MAP_CHECKER_DBG("[%s %d] cmd:%s\n",__FUNCTION__,__LINE__,cmd);
		system(cmd);
		printf("[%s %d],packet loop, down vxd!\n",__FUNCTION__,__LINE__);
		unlink(BACKHAUL_LINK_INFO);
	}
	return;
}

int _ezmesh_send_arp(u_int32_t yiaddr, u_int32_t ip, unsigned char *mac, char *interface)
{
	//int   timeout = 1;
	int     optval = 1;
	int     s;                      /* socket */
	int     rv = 1;                 /* return value */
	struct sockaddr addr;           /* for interface name */
	struct ezmesh_arpMsg    arp;
	//struct timeval        tm;
	//time_t                prevTime;

	if ((s = socket (PF_PACKET, SOCK_PACKET, htons(ETH_P_ARP))) == -1) {
		printf("Could not open raw socket\n");
		return -1;
	}

	if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) == -1) {
		printf("Could not setsocketopt on raw socket\n");
		close(s);
		return -1;
	}

	/* send arp request */
	memset(&arp, 0, sizeof(arp));
	memcpy(arp.ethhdr.h_dest, MAC_BCAST_ADDR, 6);   /* MAC DA */
	memcpy(arp.ethhdr.h_source, mac, 6);            /* MAC SA */
	arp.ethhdr.h_proto = htons(ETH_P_ARP);          /* protocol type (Ethernet) */
	arp.htype = htons(ARPHRD_ETHER);                /* hardware type */
	arp.ptype = htons(ETH_P_IP);                    /* protocol type (ARP message) */
	arp.hlen = 6;                                   /* hardware address length */
	arp.plen = 4;                                   /* protocol address length */
	arp.operation = htons(ARPOP_REQUEST);           /* ARP op code */
	memcpy(arp.sInaddr, &ip, sizeof(ip));           /* source IP address */
	memcpy(arp.sHaddr, mac, 6);                     /* source hardware address */
	memcpy(arp.tInaddr, &yiaddr, sizeof(yiaddr));   /* target IP address */

	memset(&addr, 0, sizeof(addr));
	strlcpy(addr.sa_data, interface, sizeof(addr.sa_data));
	//printf("[%s %d],send arp on if:%s\n",__FUNCTION__,__LINE__,interface);
	if (sendto(s, &arp, sizeof(arp), 0, &addr, sizeof(addr)) < 0){
		printf("[%s %d],send arp on %s fail, errno:%d \n", __FUNCTION__, __LINE__, interface, errno);
		rv = 0;
	}

	close(s);
	return rv;
}

static void send_br_arp(){
	//unsigned long requested_ip = 0;
	//unsigned long source_ip = 0;
	struct in_addr inAddr,inAddr1,requested_ip;
	char backhaul_ifname[16] = "wlan3-vxd";
	char tmp1[20]={0};
	char tmp2[20]={0};
	char tmp3[20]={0};
	_getInAddr(map_brif, IP_ADDR, (void *)&inAddr);
	_getInAddr(map_brif, SUBNET_MASK, (void *)&inAddr1);
	int i=0,j=0;
	int backhaul_sta_radio_index = -1;
	unsigned char sMAC[6] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
	unsigned char backhaul_hwaddr[6];

	struct eth_interface_info eth_info;
	eth_info.eth_interface_nr = 0;
	eth_info.eth_interface_names = NULL;

	get_eth_interfaces(&eth_info);
	MAP_CHECKER_DBG("[%s %d],eth interface num:%d\n",__FUNCTION__,__LINE__,eth_info.eth_interface_nr);

	if(0 == get_current_backhaul_radio(&backhaul_sta_radio_index)){
		if(backhaul_sta_radio_index == 1) {
			backhaul_ifname[4] = '1';
		} else if (backhaul_sta_radio_index == 0) {
			backhaul_ifname[4] = '0';
		}
		MAP_CHECKER_DBG("[%s %d],backhaul_sta_radio_index:%d, ifname:%s\n",__FUNCTION__,__LINE__,backhaul_sta_radio_index,backhaul_ifname);
	}

	if(0 == getMacAddr(backhaul_ifname, backhaul_hwaddr)){
		memcpy(&sMAC[3], &backhaul_hwaddr[3], 3);
		MAP_CHECKER_DBG("[%s %d],backhaul_hwaddr:%02x%02x%02x%02x%02x%02x, sMAC:%02x%02x%02x%02x%02x%02x\n",
			__FUNCTION__,__LINE__,
			backhaul_hwaddr[0],backhaul_hwaddr[1],backhaul_hwaddr[2],backhaul_hwaddr[3],backhaul_hwaddr[4],backhaul_hwaddr[5],
			sMAC[0],sMAC[1],sMAC[2],sMAC[3],sMAC[4],sMAC[5]);
	}

	//source_ip = inet_addr(inAddr);
	//requested_ip= inet_addr("192.168.1.10");
	//char ifname_s[12] = {0};
	requested_ip.s_addr= inAddr.s_addr & inAddr1.s_addr;
	strlcpy(tmp1, inet_ntoa(inAddr), sizeof(tmp1));
	strlcpy(tmp2, inet_ntoa(inAddr1), sizeof(tmp2));
	strlcpy(tmp3, inet_ntoa(requested_ip), sizeof(tmp3));
	MAP_CHECKER_DBG("[%s %d],tmp1:%s, tmp2:%s, tmp3:%s\n",__FUNCTION__,__LINE__,tmp1,tmp2,tmp3);

	for(j=0; j<eth_info.eth_interface_nr; j++){
		//MAP_CHECKER_DBG("[%s %d],eth interface[%d] is:%s\n",__FUNCTION__,__LINE__,j,eth_info.eth_interface_names[j]);
		if(is_eth_connected(eth_info.eth_interface_names[j])){
			MAP_CHECKER_DBG("[%s %d]: send arp packet(%d) on if:%s\n",__FUNCTION__,__LINE__,
				max_send_arp, eth_info.eth_interface_names[j]);
			for(i=0; i<max_send_arp; i++){ //incase controller arp table abnormal, user x.x.x.0 for both src & dst ip
				_ezmesh_send_arp(requested_ip.s_addr, requested_ip.s_addr, sMAC, eth_info.eth_interface_names[j]);
				usleep(100*1000);
			}
		}else{
			MAP_CHECKER_DBG("[%s %d]: send arp packet(%d) on if:%s\n",__FUNCTION__,__LINE__,
				(max_send_arp-1), eth_info.eth_interface_names[j]);
			for(i=0; i<(max_send_arp-1); i++){
				_ezmesh_send_arp(requested_ip.s_addr, requested_ip.s_addr, sMAC, eth_info.eth_interface_names[j]);
				usleep(100*1000);
			}
		}
	}

/*
	for(j =2; j<5; j++){
		snprintf(ifname_s, sizeof(ifname_s), "eth0.%d.0", j);
		MAP_CHECKER_DBG("[%s %d]: send arp packet on if:%s,sMAC:%02x%02x%02x%02x%02x%02x\n",__FUNCTION__,__LINE__,ifname_s,
			sMAC[0],sMAC[1],sMAC[2],sMAC[3],sMAC[4],sMAC[5]);
		for(i=0; i<3; i++){
			_ezmesh_send_arp(requested_ip, inAddr.s_addr, sMAC, ifname_s);
			usleep(100*1000);
		}
	}
*/
	return;
}

static void sigHandler_user(int signo)
{
	if (signo == SIGUSR1)
	{
		MAP_CHECKER_DBG("[%s %d]=====>got SIGUSER1\n",__FUNCTION__,__LINE__);
		//send_autoconfig_search_request();
		send_br_arp();
	}else if (signo == SIGUSR2){
		MAP_CHECKER_DBG("[%s %d]=====>got SIGUSER2\n",__FUNCTION__,__LINE__);
		down_vxd();
	}
	else
		printf("Got an invalid signal [%d]!\n", signo);
}

int get_connected_eth_ifname(int eth_nr, char **eth_names, char *linked_eth_ifname)
{
	int	i	= 0;
	int	ret	= 0;
	uint8_t is_connected = 0;

	for (i = 0; i < eth_nr; i++) {
		is_connected = (uint8_t)is_eth_connected(eth_names[i]);
		if (is_connected) {
			strncat(linked_eth_ifname, (const char*)eth_names[i], READ_BUF_SIZE - 1);
			ret = 1;
		}
	}
#if 0
	fclose(fd);
#endif
	return ret;
}

int backhaul_link_detect(int *backhaul_link)
{
	int  ret = 0;
	char connected_interface[READ_BUF_SIZE] = {0} ;

	FILE *fp = fopen(BACKHAUL_LINK_INFO, "r");
	if (fp == NULL) {
		printf("ERROR (%s)%d Open %s file error.\n", __FUNCTION__, __LINE__, BACKHAUL_LINK_INFO);
		return -1;
	}

	fgets(connected_interface, READ_BUF_SIZE, fp);
	fclose(fp);
	if (strstr(connected_interface, "eth")) {
		*backhaul_link = BACKHAUL_ETH;
	} else if(strstr(connected_interface, "vxd")){
		*backhaul_link = BACKHAUL_WIFI;
	} else{
		*backhaul_link = BACKHAUL_INVALID;
	}

	MAP_CHECKER_DBG("[%s %d],backhaul_link:%d\n",__FUNCTION__,__LINE__,*backhaul_link);

	ret = 1;

	return ret;
}

void backhaul_link_switch(struct eth_interface_info eth_info, struct backhaul_link_detect_control detect_control, struct backhaul_link_detect_info *backhaul_info)
{
	int backhaul_link = BACKHAUL_INVALID; //1 eth ;2 wlan
	int is_backhaul_link_on_eth = 0, is_eth_link=0;
	int fd;
	char tmp_cmd1[128] = { 0 };
	char tmp_cmd2[128] = { 0 };
	char buf_backhaul_link_if[READ_BUF_SIZE] = { 0 };
	char buf_connected_eth_if[READ_BUF_SIZE] = { 0 };
	char buf_wlan_connect[READ_BUF_SIZE] = { 0 };
	char tmp_backhaul_link_info[READ_BUF_SIZE] = { 0 };
	char tmp_controller_ip[READ_BUF_SIZE] = {0};

	MAP_CHECKER_DBG("%s %d\n",__FUNCTION__,__LINE__);

	int tmp = BACKHAUL_LINK_NOTIFY_INTERVAL/detect_control.check_interval;

	fd = open(BACKHAUL_LINK_INFO, O_RDONLY);
	if (fd >= 0) {
		FILE *fp = fdopen(fd, "r");
		if (fp == NULL) {
			printf("ERROR (%s)%d Open %s file error.\n", __FUNCTION__, __LINE__, BACKHAUL_LINK_INFO);
			return;
		}
		fgets(tmp_backhaul_link_info, READ_BUF_SIZE, fp);
		fclose(fp);
	}else{
		MAP_CHECKER_DBG("[%s %d] file:%s did not exist, skip check.\n", __FUNCTION__, __LINE__, BACKHAUL_LINK_INFO);
		return;
	}

	sscanf(tmp_backhaul_link_info,"%s %d %s", buf_backhaul_link_if, &backhaul_info->current_heartbeat_count, tmp_controller_ip);
	MAP_CHECKER_DBG("[%s %d] tmp_backhaul_link_info:%s, last_heartbeat_count:%d\n",__FUNCTION__,__LINE__,tmp_backhaul_link_info,backhaul_info->last_heartbeat_count);

	if(backhaul_info->current_heartbeat_count == backhaul_info->last_heartbeat_count){
		backhaul_info->count += 1;
		MAP_CHECKER_DBG("[%s %d] heartbeat_pause_count:%d\n",__FUNCTION__,__LINE__,backhaul_info->count);
		if(backhaul_info->count >= tmp){
			backhaul_info->discovery_down_count += 1;
			backhaul_info->count = 0;
		}
	}else{
		backhaul_info->last_heartbeat_count = backhaul_info->current_heartbeat_count;
		backhaul_info->count = 0;
		MAP_CHECKER_DBG("%s %d\n",__FUNCTION__,__LINE__);
		return;
	}

	backhaul_link_detect(&backhaul_link);

	if(backhaul_link == BACKHAUL_WIFI){
		//backhaul_info->backhaul_link_is_wifi = 1;
		//backhaul_info->backhaul_link_is_eth = 0;
			snprintf(tmp_cmd1, sizeof(tmp_cmd1), "wlan%d-vxd", detect_control.backhaul_sta_radio_index);
			MAP_CHECKER_DBG("%s %d tmp_cmd1 %s\n",__FUNCTION__,__LINE__,tmp_cmd1);

			if (is_interface_up(tmp_cmd1)) {
#if IS_AX_SUPPORT
				snprintf(tmp_cmd2, sizeof(tmp_cmd2), LINK_FILE_NAME_FMT, tmp_cmd1, "sta_info");
#else
				snprintf(tmp_cmd2, sizeof(tmp_cmd2), "/proc/wlan%d-vxd/sta_info", detect_control.backhaul_sta_radio_index);
#endif
				FILE *fp = fopen(tmp_cmd2, "a+");
				if (fp == NULL) {
					printf("ERROR (%s)%d Open %s file error.\n", __FUNCTION__, __LINE__, tmp_cmd2);
					return;
				}
				fgets(buf_wlan_connect, READ_BUF_SIZE, fp);
				fclose(fp);

				MAP_CHECKER_DBG("[%s %d] buf_wlan_connect:%s\n",__FUNCTION__,__LINE__,buf_wlan_connect);

				if(!strstr(buf_wlan_connect,"active: 1")){
					MAP_CHECKER_DBG("[%s %d] wifi backhaul disconnect\n",__FUNCTION__,__LINE__);
					backhaul_info->link_down_count += 1;
				}
			}else{
			MAP_CHECKER_DBG("[%s %d] if:%s is not up,link abnormal!\n",__FUNCTION__,__LINE__,tmp_cmd1);
					backhaul_info->link_down_count += 1;
		}
	}else if(backhaul_link == BACKHAUL_ETH){
		//backhaul_info->backhaul_link_is_eth = 1;
		//backhaul_info->backhaul_link_is_wifi = 0;
			is_eth_link = get_connected_eth_ifname(eth_info.eth_interface_nr, eth_info.eth_interface_names, buf_connected_eth_if);
			MAP_CHECKER_DBG("%s %d\n",__FUNCTION__,__LINE__);

			if(0 == is_eth_link)
			{
				MAP_CHECKER_DBG("[%s %d] eth disconnect\n",__FUNCTION__,__LINE__);
				backhaul_info->link_down_count += 1;
			}
			else
			{
				MAP_CHECKER_DBG("%s %d\n",__FUNCTION__,__LINE__);
				if(strstr(buf_connected_eth_if, buf_backhaul_link_if)){
					is_backhaul_link_on_eth = 1;
				}

				if(is_backhaul_link_on_eth != 1){
					MAP_CHECKER_DBG("[%s %d] eth connected ifs:%s, controller found on:%s\n",__FUNCTION__,__LINE__,buf_connected_eth_if,buf_backhaul_link_if);
					backhaul_info->link_down_count += 1;
				}
			}
		}

		if((backhaul_info->link_down_count >= detect_control.link_down_threshold) || (backhaul_info->discovery_down_count >= detect_control.discovery_down_threshold))
		{
			MAP_CHECKER_DBG("[%s %d] link_down_count:%d, discovery_down_count:%d\n",__FUNCTION__,__LINE__,backhaul_info->link_down_count,backhaul_info->discovery_down_count);
			backhaul_info->need_triger = 1;
		}

	if(backhaul_info->need_triger){
		unlink(BACKHAUL_LINK_INFO);
		//if(backhaul_info->backhaul_link_is_wifi){
		if(backhaul_link == BACKHAUL_WIFI){
			MAP_CHECKER_DBG("[%s %d] Bckhaul link on wlan is abnormal!!\n",__FUNCTION__,__LINE__);
			memset(backhaul_info, 0, sizeof(*backhaul_info));
		//}else if(backhaul_info->backhaul_link_is_eth){
		}else if(backhaul_link == BACKHAUL_ETH){
			snprintf(tmp_cmd1, sizeof(tmp_cmd1), "iwpriv wlan%d-vxd set_mib func_off=0; ifconfig wlan%d-vxd down up", detect_control.backhaul_sta_radio_index,detect_control.backhaul_sta_radio_index);
			MAP_CHECKER_DBG("[%s %d] cmd:%s\n",__FUNCTION__,__LINE__,tmp_cmd1);
			system(tmp_cmd1);
			printf("Backhaul link on eth is abnormal, switch to vxd!\n");
			memset(backhaul_info, 0, sizeof(*backhaul_info));
		}
	}

	return;
}

#ifdef CONFIG_WIFI_SIMPLE_CONFIG
void rtk_monitor_wps(void)
{
	pid_t wps_pid  = 0, iwcontrol_pid = 0;
	char *wps_name = "wscd", *iwcontrol_name = "iwcontrol", tmpBuf[150]={0}, iwcontrol_cmd[50]={0};
	int retry = 0, wsc_pid_fd1=-1, wsc_pid_fd2=-1, wlan1_wsc_enable = 0, wlan2_wsc_enable = 0;
	DIR *dir;
	struct dirent *next;

	iwcontrol_pid  = find_pid_by_name(iwcontrol_name);
	if(iwcontrol_pid != 0)
	{
		wps_pid  = find_pid_by_name(wps_name);
		if(wps_pid == 0)
		{
			kill(iwcontrol_pid, 9);
			unlink("/var/wscd-wlan0.fifo");
			unlink("/var/wscd-wlan1.fifo");
			dir = opendir("/var");
			if(dir)
			{
				while ((next = readdir(dir)) != NULL) {
					if (!strncmp(next->d_name, "wsc-wlan", strlen("wsc-wlan")) && strstr(next->d_name, ".conf")) {
						if(strstr(next->d_name, "wlan0") && strstr(next->d_name, "wlan1"))
						{
							wlan1_wsc_enable = 1;
							wlan2_wsc_enable = 1;
							snprintf(tmpBuf,sizeof(tmpBuf),"wscd -start -c /var/wsc-wlan1-wlan0.conf -w2 wlan1 -w wlan0 -fi2 /var/wscd-wlan1.fifo -fi /var/wscd-wlan0.fifo -daemon");
							snprintf(iwcontrol_cmd,sizeof(iwcontrol_cmd),"iwcontrol wlan0 wlan1");
						}
						else if(strstr(next->d_name, "wlan0"))
						{
							wlan1_wsc_enable = 1;
							snprintf(tmpBuf,sizeof(tmpBuf),"wscd -start -c /var/wsc-wlan0.conf -w wlan0 -fi /var/wscd-wlan0.fifo -daemon");
							snprintf(iwcontrol_cmd,sizeof(iwcontrol_cmd),"iwcontrol wlan0");
						}
						else if(strstr(next->d_name, "wlan1"))
						{
							wlan2_wsc_enable = 1;
							snprintf(tmpBuf,sizeof(tmpBuf),"wscd -start -c /var/wsc-wlan1.conf -w2 wlan1 -fi2 /var/wscd-wlan1.fifo -daemon");
							snprintf(iwcontrol_cmd,sizeof(iwcontrol_cmd),"iwcontrol wlan0 wlan1");
						}

						break;
					}
				}
				closedir(dir);
			}

			if(wlan1_wsc_enable || wlan2_wsc_enable)
			{
				system(tmpBuf);
				retry = 10;
				while (--retry && (((wlan1_wsc_enable) && ((wsc_pid_fd1 = open("/var/wscd-wlan0.fifo", O_WRONLY)) == -1)) || ((wlan2_wsc_enable) && ((wsc_pid_fd2 = open("/var/wscd-wlan1.fifo", O_WRONLY)) == -1))))
				{
					usleep(30000);
				}

				retry = 10;
				while(--retry && (find_pid_by_name(wps_name) == 0))
				{
					//printf("WSCD is not running. Please wait!\n");
					usleep(300000);
				}

				if(wsc_pid_fd1!=-1) close(wsc_pid_fd1);/*jiunming, close the opened fd*/
				if(wsc_pid_fd2!=-1) close(wsc_pid_fd2);/*jiunming, close the opened fd*/

				system(iwcontrol_cmd);
			}
		}
	}
}
#endif

int bridge_name_init()
{
	struct easymesh_datamodel easymesh_db = {0};

	if (ini_parse(MULTIAP_CONF, read_config_file, &easymesh_db) < 0) {
		printf("ERROR [%s %d] Can't load configuration file(%s)!! \n",__FUNCTION__,__LINE__,MULTIAP_CONF);
	} else {
		if (easymesh_db.bridge_name)
			snprintf(map_brif, sizeof(map_brif), "%s", easymesh_db.bridge_name);
	}
	return 0;
}

int check_multiap_files(char *filename, int max_wait_time)
{
	int cnt = 0;

	while((cnt < max_wait_time) && (access(filename, F_OK) == -1)){
		MAP_CHECKER_DBG("%s not exist, waiting(%d)...\n", filename, cnt);
		cnt++;
		sleep(1);
	}
	if(access(filename, F_OK) == -1)
		return -1;
	else
		return 0;
}

/* for openWRT, only easymesh controller or agent can start map checker */
int main(int argc, char *argv[])
{
	int   c;
	char log_level_string[10] = { 0 };
	unsigned int check_count = 0;
	struct map_checker_datamodel map_checker_conf;
	unsigned int ip_check_interval = 5;
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
	int is_controller = 0;
#endif
	struct eth_interface_info eth_info;
	struct backhaul_link_detect_control detect_control;
	struct backhaul_link_detect_info backhaul_info;
	int as_daemon = 0, max_wait_time = 120;
	
	int   multiap_profile = 1;

	memset(&backhaul_info, 0, sizeof(backhaul_info));
	memset(&detect_control, 0, sizeof(detect_control));
	memset(&map_checker_conf, 0, sizeof(map_checker_conf));
	
	while ((c = getopt(argc, argv, "vs:i:l:d:p:m:w:a:")) != -1) {
		switch (c) {
			case 'v': {
				char *verbose_char = "v";
				// Each time this flag appears, the verbose_level is incremented.
				strncat(log_level_string, verbose_char, 2);
				break;
			}
			case 'p': {
				if (optarg) {
					switch (atoi(optarg)) {
					case 1:
						multiap_profile = 1;
						break;
					case 2:
						multiap_profile = 2;
						break;
					case 3:
						multiap_profile = 3;
						break;
					case 4:
						multiap_profile = 4;
						break;
					}
				}
				break;
			}
			case 's': {
				int tmp = 0;
				if (optarg) {
					tmp = atoi(optarg);
				}
				if (tmp == 0){
					detect_control.backhaul_switch = 0;
				}else if(tmp == 1){
					detect_control.backhaul_switch = 1;
				}
				break;
			}
			case 'i': {
				if (optarg) {
					detect_control.check_interval = atoi(optarg);
				}
				break;
			}
			case 'l': {
				if (optarg) {
					detect_control.link_down_threshold = atoi(optarg);
				}
				break;
			}
			case 'd': {
				if (optarg) {
					detect_control.discovery_down_threshold = atoi(optarg);
				}
				break;
			}
			case 'm': { //daemon
				if (optarg){
					as_daemon = 1;
				}
				break;
			}
			case 'w': { //wait file time
				if (optarg) {
					max_wait_time = atoi(optarg);
				}
				break;
			}
			case 'a': { //max send arp count
				if (optarg) {
					max_send_arp = atoi(optarg);
				}
				break;
			}
		}
	}

	if(strlen(log_level_string) > 1){
		map_checker_debug = 1;
	}

	MAP_CHECKER_DBG("as_daemon:%d, wait_file:%d, send_arp:%d\n",as_daemon, max_wait_time, max_send_arp);

	/* for openWRT, map_checker should not be daemon */
	if(as_daemon){
		if (-1 == daemon(0, 1)) {
			printf("Run map_checker as daemon error!\n");
			return 1;
		}else{
			MAP_CHECKER_DBG("Run map_checker as daemon.\n");
		}
	}

	/*
		for openWRT, multi ap relevant files
		(eg: /var/multiap.conf, /var/multiap_mib.conf, /var/map_ext_data.conf)
		are generated during service rtk_multiap startup.
		and maybe not generated when map checker start.
	*/
	if(check_multiap_files(MULTIAP_CONF, max_wait_time)){
		MAP_CHECKER_DBG("map checker exit !!!\n");
		return 1;
	}
	if(check_multiap_files(MULTIAP_MIB_CONF, max_wait_time)){
		MAP_CHECKER_DBG("map checker exit !!!\n");
		return 1;
	}
#if 0 // todo tesia
	if(check_multiap_files(MAP_EXT_DATA, max_wait_time)){
		MAP_CHECKER_DBG("map checker exit !!!\n");
		return 1;
	}
#endif

	map_checker_conf.auto_renew_ip = 1;
	map_checker_conf.arp_loop = 1; //tesia
	if (ini_parse(MAP_EXT_DATA, read_map_ext_conf, &map_checker_conf) < 0) {
		printf("ERROR [%s %d] Can't load configuration file (%s)!! \n",__FUNCTION__,__LINE__, MAP_EXT_DATA);
	}

	bridge_name_init();

	eth_info.eth_interface_nr = 0;
	eth_info.eth_interface_names = NULL;

	//--------detect_control initializtion, incase MAP_EXT_DATA is not generated in boa of other SDK
	detect_control.backhaul_switch = 1;
	if(!detect_control.check_interval)
		detect_control.check_interval = 5;
	if(!detect_control.link_down_threshold)
		detect_control.link_down_threshold = 1;
	if(!detect_control.discovery_down_threshold)
		detect_control.discovery_down_threshold = 1;
	get_current_backhaul_radio(&detect_control.backhaul_sta_radio_index);

	//detect_control.backhaul_switch = map_checker_conf.backhaul_switch;
	//detect_control.check_interval = map_checker_conf.check_interval;
	//detect_control.link_down_threshold = map_checker_conf.link_down_threshold;
	//detect_control.discovery_down_threshold = map_checker_conf.discovery_down_threshold;
	//detect_control.backhaul_sta_radio_index = map_checker_conf.backhaul_sta_radio_index;

	MAP_CHECKER_DBG("============= map_ext_conf_data ============== \n");
	MAP_CHECKER_DBG("backhaul_switch :%d \n",detect_control.backhaul_switch);
	MAP_CHECKER_DBG("backhaul_sta_radio_index :%d \n",detect_control.backhaul_sta_radio_index);
	MAP_CHECKER_DBG("check_interval :%d \n",detect_control.check_interval);
	MAP_CHECKER_DBG("link_down_threshold :%d \n",detect_control.link_down_threshold);
	MAP_CHECKER_DBG("discovery_down_threshold :%d \n",detect_control.discovery_down_threshold);
	MAP_CHECKER_DBG("multiap_profile :%d \n",multiap_profile);

	if(detect_control.backhaul_switch != 0){
		get_eth_interfaces(&eth_info);
	}

	if(map_checker_conf.arp_loop == 1){
		pidfile_write();
		signal(SIGUSR1, sigHandler_user);
		signal(SIGUSR2, sigHandler_user);
	}

	int map_alive = 0;
	while (1) {
		pid_t map_pid  = 0;
		char *map_name = "map_controller";
		while (0 == map_pid) {
			map_name = "map_controller";
			map_pid  = find_pid_by_name(map_name);
			if (0 != map_pid) {
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
				is_controller = 1;
#endif
				break;
			}
			map_name = "map_agent";
			map_pid  = find_pid_by_name(map_name);
			sleep(5);
		}

		map_alive = 1;
		while (map_alive) {
			sleep(1);
			if (-1 == kill(map_pid, 0)) {
				map_alive = 0;
			}

			map_checker_debug = (dynamic_debug_enable()) ? 1:0 ;

			if (strcmp(map_name, "map_agent") == 0){
				check_count += 1;
				if(detect_control.backhaul_switch != 0){
					if (check_count % detect_control.check_interval == 0){
						//backhaul_link_switch(eth_info, detect_control, &backhaul_info); //not test yet for openwrt
					}
				}

				if(map_checker_conf.auto_renew_ip == 1){
					if(check_count % ip_check_interval == 0){
						//check_controller_ip_renew(); //not test yet for openwrt
					}
				}
			}

#ifdef CONFIG_WIFI_SIMPLE_CONFIG
			if(is_controller == 1)
				rtk_monitor_wps();
#endif
		}

		/* 
		// openwrt not need (map_controller & map_agent is monitored by procd)
		if (0 == remove("/tmp/map_checker_end.txt")) {
			break;
		}
		printf("%s with pid %d is dead!!!\n", map_name, map_pid);
		if (find_pid_by_name(map_name)) {
			continue;
		}
		char cmd[64] = { 0 };
		snprintf(cmd, sizeof(cmd), "%s -d%s -p%d", map_name, log_level_string, multiap_profile);
		system(cmd);
		*/
	}

	return 0;
}
