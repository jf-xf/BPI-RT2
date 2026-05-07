/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _MAP_PATHSEL_H_
#define _MAP_PATHSEL_H_
#include <netinet/in.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>

#include "map_dataelement.h"

extern uint8_t pathsel_enable;

void update_mcs_table_from_conf (uint8_t *mcs_table, uint8_t table_size);

void path_db_init(unsigned char *root_id);

int create_node(unsigned char *id);

int find_node_index_by_id(unsigned char *id);

void do_controller(struct Network *network);

void update_node_neighbours(unsigned char *node_id, unsigned char *neighbours, int n, int is_directly_connected);

void update_metric(unsigned char *addr_1, unsigned char *addr_2, int rssi);

void remove_path_node(unsigned char *id);

int  get_path_score(unsigned char *agent_addr);

#endif
