/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _ETH_MONITOR_H_
#define _ETH_MONITOR_H_

#include <unistd.h>

#define BACKHAUL_LINK_FLAG_PATH "/tmp/map_backhaul_link"

enum SEARCH_INTERFACE { ETH_0,
	                    WLAN_0_VXD };

enum BACKHAUL_LINK { BACKHAUL_INVALID,
	                 BACKHAUL_ETH,
	                 BACKHAUL_WIFI };

struct eth_monitor_thread_data {
	uint8_t        interface_nr;
	char **        interface_names;
	uint8_t        backhaul_sta_radio_index;
};

uint8_t is_eth_connected(char *eth_interface_name);

uint8_t is_any_eth_connected(int eth_nr, char ** eth_names, int *index);

void *eth_monitor_thread(void *p);

void resume_eth_monitor();

void end_eth_monitor(uint8_t backhaul_link);

// enum SEARCH_INTERFACE get_search_interface();

// void set_search_interface(enum SEARCH_INTERFACE interface);

uint8_t get_eth_event();

void clear_eth_event();

#endif
