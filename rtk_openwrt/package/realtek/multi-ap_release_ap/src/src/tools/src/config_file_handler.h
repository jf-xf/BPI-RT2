/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _CONFIG_FILE_HANDLER_H_
#define _CONFIG_FILE_HANDLER_H_
#include <stdint.h>
#include <stdio.h>

#include "easymesh_datamodel.h"

int read_config_file(void *user, const char *section, const char *name, const char *value);
int read_mib_config_file(void *user, const char *section, const char *name, const char *value);
int read_set_mib_config_file(void *user, const char *section, const char *name, const char *value);
int read_map_ext_conf(void *user, const char *section, const char *name, const char *value);
int write_set_mib_config_file(struct easymesh_datamodel *data_container);
int write_api_info_file(struct easymesh_api_info *data_container);
int read_vlan_mib_file(void *user, const char *section, const char *name, const char *value);
void fill_bss_index_vendor_config(struct easymesh_datamodel *easymesh_database);

#endif