/*
 *  Multi-AP Device Provisioning Protocol (DPP)
 *  Counter Storage Abstraction Layer (for ETC, ERC, and dfCounter)
 *
 *  Copyright (C) 2021 Realtek Semiconductor Corp. - All Rights Reserved
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "platform.h"
#include "al_dpp_counters.h"
#include "al_dpp_llist.h"

#define DPP_COUNTERS_FILE "/tmp/dpp_counters"

//////////////////////////////////////////
//           Cached Variables           //
//////////////////////////////////////////
struct counters_entry {
	struct dpp_llist_head list;
	INT8U                 mac_addr[6];
	INT64U                enc_tx_ctr;
	INT64U                enc_rx_ctr;
	INT16U                df_ctr;
};

static struct dpp_llist_head counter_list_head;
static INT8U                 read_success = 0;

//////////////////////////////////////////
//         Internal Functions           //
//////////////////////////////////////////
static void dpp_counters_insert_entry(INT8U *mac_addr,
                                      INT64U enc_tx_ctr,
                                      INT64U enc_rx_ctr,
                                      INT16U df_ctr)
{
	struct counters_entry new_entry;

	PLATFORM_PRINTF_DEBUG(
	    "DPP counters: "
	    "insert counter entry (%02X:%02X:%02X:%02X:%02X:%02X, %llu, %llu, %hu)\n",
	    mac_addr[0], mac_addr[1], mac_addr[2],
	    mac_addr[3], mac_addr[4], mac_addr[5],
	    enc_tx_ctr, enc_rx_ctr, df_ctr);

	PLATFORM_MEMSET(&new_entry, 0, sizeof(new_entry));
	PLATFORM_MEMCPY(new_entry.mac_addr, mac_addr, 6);
	new_entry.enc_tx_ctr = enc_tx_ctr;
	new_entry.enc_rx_ctr = enc_rx_ctr;
	new_entry.df_ctr     = df_ctr;
	dpp_llist_add(&counter_list_head, (void *)&new_entry, sizeof(new_entry));
}

static struct counters_entry *dpp_counters_search_entry(INT8U *mac_addr)
{
	struct dpp_llist_head *entry;

	for (entry = counter_list_head.next; entry != NULL; entry = entry->next) {
		struct counters_entry *e = (struct counters_entry *)entry;
		if (PLATFORM_MEMCMP(e->mac_addr, mac_addr, 6) == 0) {
			return e;
		}
	}
	return NULL;
}

static INT8U dpp_counters_remove_entry(INT8U *mac_addr)
{
	struct counters_entry *entry;

	PLATFORM_PRINTF_DEBUG(
	    "DPP counters: Removing counters from cache (%02X:%02X:%02X:%02X:%02X:%02X)\n",
	    mac_addr[0], mac_addr[1], mac_addr[2],
	    mac_addr[3], mac_addr[4], mac_addr[5]);

	if ((entry = dpp_counters_search_entry(mac_addr)) != NULL) {
		dpp_llist_remove((struct dpp_llist_head *)entry);
		return 1;
	}

	return 0;
}

static void dpp_counters_free_all()
{
	dpp_llist_empty(&counter_list_head);
}

//////////////////////////////////////////
//         File I/O Functions           //
//////////////////////////////////////////
static INT8U dpp_counters_sync()
{
	struct dpp_llist_head *entry;

	FILE *pFile;
	char  buf[64];

	pFile = fopen(DPP_COUNTERS_FILE, "w+e");
	if (!pFile) {
		PLATFORM_PRINTF_WARNING("DPP counters: cannot write file!\n");
		return 0;
	}

	for (entry = counter_list_head.next; entry != NULL; entry = entry->next) {
		struct counters_entry *e = (struct counters_entry *)entry;
		PLATFORM_SNPRINTF(buf, sizeof(buf),
		                  "%02X:%02X:%02X:%02X:%02X:%02X, %llu, %llu, %hu\n",
		                  e->mac_addr[0], e->mac_addr[1], e->mac_addr[2],
		                  e->mac_addr[3], e->mac_addr[4], e->mac_addr[5],
		                  e->enc_tx_ctr, e->enc_rx_ctr, e->df_ctr);
		fputs(buf, pFile);
	}
	fclose(pFile);
	return 1;
}

static INT8U dpp_counters_read()
{
	FILE *pFile;
	int   read_count = 0;

	pFile = fopen(DPP_COUNTERS_FILE, "re");
	if (!pFile) {
		PLATFORM_PRINTF_WARNING("DPP counters: cannot read file!\n");
		return 0;
	}

	// Read file to linked list:
	dpp_counters_free_all();
	do {
		INT8U  mac_addr[6];
		INT64U enc_tx_ctr, enc_rx_ctr;
		INT16U df_ctr;
		read_count = fscanf(pFile,
		                    "%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX, %llu, %llu, %hu",
		                    &mac_addr[0], &mac_addr[1], &mac_addr[2],
		                    &mac_addr[3], &mac_addr[4], &mac_addr[5],
		                    &enc_tx_ctr, &enc_rx_ctr, &df_ctr);
		if (read_count == 9)
			dpp_counters_insert_entry(mac_addr, enc_tx_ctr, enc_rx_ctr, df_ctr);

	} while (read_count > 0);

	fclose(pFile);
	read_success = 1;
	return 1;
}

//////////////////////////////////////////
//          Public Functions            //
//////////////////////////////////////////
INT64U dpp_counters_get_etc(INT8U *mac_addr)
{
	struct counters_entry *entry;

	if (!read_success)
		dpp_counters_read();

	entry = dpp_counters_search_entry(mac_addr);
	return (entry) ? entry->enc_tx_ctr : 0;
}

INT64U dpp_counters_get_erc(INT8U *mac_addr)
{
	struct counters_entry *entry;

	if (!read_success)
		dpp_counters_read();

	entry = dpp_counters_search_entry(mac_addr);
	return (entry) ? entry->enc_rx_ctr : 0;
}

INT16U dpp_counters_get_dfc(INT8U *mac_addr)
{
	struct counters_entry *entry;

	if (!read_success)
		dpp_counters_read();

	entry = dpp_counters_search_entry(mac_addr);
	return (entry) ? entry->df_ctr : 0;
}

INT8U dpp_counters_set_etc(INT8U *mac_addr, INT64U enc_tx_ctr)
{
	struct counters_entry *entry;

	if (!read_success)
		dpp_counters_read();

	if ((entry = dpp_counters_search_entry(mac_addr)) != NULL)
		entry->enc_tx_ctr = enc_tx_ctr;
	else
		dpp_counters_insert_entry(mac_addr, enc_tx_ctr, 0, 0);

	return dpp_counters_sync();
}

INT8U dpp_counters_set_erc(INT8U *mac_addr, INT64U enc_rx_ctr)
{
	struct counters_entry *entry;

	if (!read_success)
		dpp_counters_read();

	if ((entry = dpp_counters_search_entry(mac_addr)) != NULL)
		entry->enc_rx_ctr = enc_rx_ctr;
	else
		dpp_counters_insert_entry(mac_addr, 1, enc_rx_ctr, 0);

	return dpp_counters_sync();
}

INT8U dpp_counters_set_dfc(INT8U *mac_addr, INT16U df_ctr)
{
	struct counters_entry *entry;

	if (!read_success)
		dpp_counters_read();

	if ((entry = dpp_counters_search_entry(mac_addr)) != NULL)
		entry->df_ctr = df_ctr;
	else
		dpp_counters_insert_entry(mac_addr, 1, 0, df_ctr);

	return dpp_counters_sync();
}

INT8U dpp_counters_reset(INT8U *mac_addr)
{
	if (!read_success)
		dpp_counters_read();

	dpp_counters_remove_entry(mac_addr);
	dpp_counters_insert_entry(mac_addr, 1, 0, 0);
	return dpp_counters_sync();
}

INT8U dpp_counters_remove_all()
{
	dpp_counters_free_all();
	return dpp_counters_sync();
}