/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <sys/inotify.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "eth_monitor.h"
#include "map_logger.h"

#define ETH_STATUS_FILENAME "/proc/eth0/link_status"

static pthread_mutex_t eth_monitor_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  eth_monitor_cond  = PTHREAD_COND_INITIALIZER;
static int             eth_monitor_flag  = 0;

static int eth_monitor_alive  = 1;
static int eth_event_detected = 0;

static enum SEARCH_INTERFACE searching_interface;

int get_net_link_status(const char *ifname)
{
	struct ifreq         ifr;
	struct ethtool_value edata;
	int                  ret;
	int                  skfd;

	if (strlen(ifname) >= 16)
		return -1;

	strcpy(ifr.ifr_name, ifname);
	edata.cmd    = ETHTOOL_GLINK;
	ifr.ifr_data = (caddr_t)&edata;

	if ((skfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0)) < 0) {
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
	char    file_name_buf[64];
	uint8_t eth_connected = 0;

	char buf[3];
	int fd;

	FILE *fd_eth_link;

	sprintf(file_name_buf, "/proc/%s/link_status", eth_interface_name);

	fd = open(file_name_buf, O_RDONLY);
	if (fd < 0) {
		return get_net_link_status(eth_interface_name);
	}

	fd_eth_link = fopen(file_name_buf, "re");

	if (fd_eth_link == NULL) {
		log_error("*is_eth_connected* fopen() returned with errno=%d (%s)\n", errno, strerror(errno));
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

uint8_t is_any_eth_connected(int eth_nr, char **eth_names, int *index)
{
	int     i            = 0;
	uint8_t is_connected = 0;

	for (i = 0; i < eth_nr; i++) {
		if (index)
			*index = i;
		is_connected |= (uint8_t)is_eth_connected(eth_names[i]);
		if (is_connected) {
			return 1;
		}
	}
	if (index)
		*index = -1;
	return 0;
}

void *eth_monitor_thread(void *p)
{
	struct eth_monitor_thread_data *thread_data = (struct eth_monitor_thread_data *)p;
	uint8_t                         alive_check;
	pthread_mutex_lock(&eth_monitor_mutex);
	searching_interface = WLAN_0_VXD;
	alive_check         = eth_monitor_alive;
	pthread_mutex_unlock(&eth_monitor_mutex);
	int index = -1;

	char cmd[64];

	while (alive_check) {
		uint8_t eth_connected = is_any_eth_connected(thread_data->interface_nr, thread_data->interface_names, &index);
		if (1 == eth_connected) {
			log_debug("Ethernet %s is connected, disable vxd interface and search for controller....\n", thread_data->interface_names[index]);
			sprintf(cmd, "iwpriv wlan%d-vxd vxd_disassoc; ifconfig wlan%d-vxd down", thread_data->backhaul_sta_radio_index, thread_data->backhaul_sta_radio_index);
			system(cmd);
			pthread_mutex_lock(&eth_monitor_mutex);
			searching_interface = ETH_0;
			eth_event_detected  = 1;
			while (0 == eth_monitor_flag) {
				pthread_cond_wait(&eth_monitor_cond, &eth_monitor_mutex);
			}
			eth_monitor_flag = 0;
			pthread_cond_init(&eth_monitor_cond, NULL);
			searching_interface = WLAN_0_VXD;
			eth_event_detected  = 0;
			pthread_mutex_unlock(&eth_monitor_mutex);
		} else {
			usleep(500 * 1000);
		}

		pthread_mutex_lock(&eth_monitor_mutex);
		alive_check = eth_monitor_alive;
		pthread_mutex_unlock(&eth_monitor_mutex);
	}

	log_debug("*Ethernet monitor thread* Exiting...\n");

	// for (i = 0; i < thread_data->interface_nr; i++) {
	// 	free(thread_data->interface_names[i]);
	// }

	// free(thread_data->interface_names);

	return NULL;
}

void resume_eth_monitor()
{
	pthread_mutex_lock(&eth_monitor_mutex);
	eth_monitor_flag = 1;
	pthread_cond_signal(&eth_monitor_cond);
	pthread_mutex_unlock(&eth_monitor_mutex);
}

void end_eth_monitor(uint8_t backhaul_link)
{
	pthread_mutex_lock(&eth_monitor_mutex);
	eth_monitor_flag  = 1;
	eth_monitor_alive = 0;
	pthread_cond_signal(&eth_monitor_cond);
	pthread_mutex_unlock(&eth_monitor_mutex);
	FILE *fd_tmp = fopen(BACKHAUL_LINK_FLAG_PATH, "w+e");
	if (NULL == fd_tmp) {
		log_error("Could not create tmp file map_backhaul_link\n");
		return;
	}
	fputc('0' + backhaul_link, fd_tmp);
	fclose(fd_tmp);
}

enum SEARCH_INTERFACE get_search_interface()
{
	enum SEARCH_INTERFACE result;
	pthread_mutex_lock(&eth_monitor_mutex);
	result = searching_interface;
	pthread_mutex_unlock(&eth_monitor_mutex);
	return result;
}

void set_search_interface(enum SEARCH_INTERFACE interface)
{
	pthread_mutex_lock(&eth_monitor_mutex);
	searching_interface = interface;
	pthread_mutex_unlock(&eth_monitor_mutex);
}

uint8_t get_eth_event()
{
	uint8_t result;
	pthread_mutex_lock(&eth_monitor_mutex);
	result = eth_event_detected;
	pthread_mutex_unlock(&eth_monitor_mutex);
	return result;
}

void clear_eth_event()
{
	pthread_mutex_lock(&eth_monitor_mutex);
	eth_event_detected = 0;
	pthread_mutex_unlock(&eth_monitor_mutex);
}
