/*
 *  Multi-AP Device Provisioning Protocol (DPP)
 *  Key Storage Abstraction Layer for GTK and PTK
 *
 *  Copyright (C) 2021 Realtek Semiconductor Corp. - All Rights Reserved
 *  Unauthorized copying of this file, via any medium is strictly prohibited
 *  Proprietary and confidential
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "platform.h"
#include "al_dpp_keystore.h"
#include "al_dpp_llist.h"

#include "cJSON.h"
#include "utils.h"

#define DPP_KEYSTORE_FILE "/tmp/dpp_keystore"

//////////////////////////////////////////
//           Cached Variables           //
//////////////////////////////////////////
static struct dpp_gtk gtk_1905;

struct ptk_1905_entry {
	struct dpp_llist_head list;
	INT8U                 mac_addr[6];
	struct dpp_ptk        ptk_1905;
};

static struct dpp_llist_head ptk_1905_list_head;
static INT8U                 read_success = 0;

//////////////////////////////////////////
//         Internal Functions           //
//////////////////////////////////////////
static void dpp_keystore_insert_ptk(INT8U *mac_addr, struct dpp_ptk *ptk)
{
	struct ptk_1905_entry new_entry;

	PLATFORM_PRINTF_DEBUG(
	    "DPP keystore: Inserting PTK to cache (%02X:%02X:%02X:%02X:%02X:%02X)\n",
	    mac_addr[0], mac_addr[1], mac_addr[2],
	    mac_addr[3], mac_addr[4], mac_addr[5]);

	PLATFORM_MEMSET(&new_entry, 0, sizeof(new_entry));
	PLATFORM_MEMCPY(new_entry.mac_addr, mac_addr, 6);
	PLATFORM_MEMCPY(&new_entry.ptk_1905, ptk, sizeof(*ptk));
	dpp_llist_add(&ptk_1905_list_head, (void *)&new_entry, sizeof(new_entry));
}

static struct dpp_ptk *dpp_keystore_search_ptk(INT8U *mac_addr)
{
	struct dpp_llist_head *entry;

	for (entry = ptk_1905_list_head.next; entry != NULL; entry = entry->next) {
		struct ptk_1905_entry *e = (struct ptk_1905_entry *)entry;
		if (PLATFORM_MEMCMP(e->mac_addr, mac_addr, 6) == 0) {
			return &e->ptk_1905;
		}
	}
	return NULL;
}

static INT8U dpp_keystore_remove_ptk(INT8U *mac_addr)
{
	struct dpp_llist_head *entry;

	PLATFORM_PRINTF_DEBUG(
	    "DPP keystore: Removing PTK from cache (%02X:%02X:%02X:%02X:%02X:%02X)\n",
	    mac_addr[0], mac_addr[1], mac_addr[2],
	    mac_addr[3], mac_addr[4], mac_addr[5]);

	for (entry = ptk_1905_list_head.next; entry != NULL; entry = entry->next) {
		struct ptk_1905_entry *e = (struct ptk_1905_entry *)entry;
		if (PLATFORM_MEMCMP(e->mac_addr, mac_addr, 6) == 0) {
			dpp_llist_remove(entry);
			return 1;
		}
	}

	return 0;
}

static void dpp_keystore_free_all()
{
	dpp_llist_empty(&ptk_1905_list_head);
}

//////////////////////////////////////////
//         File I/O Functions           //
//////////////////////////////////////////
static INT8U dpp_keystore_sync()
{
	cJSON *keystore = NULL, *ks_gtk = NULL, *ks_ptk_list = NULL, *ks_ptk = NULL;
	char   hex_bytes[256];
	char * json  = NULL;
	FILE * pFile = NULL;
	INT8U  ret   = 0;

	struct dpp_llist_head *entry;

	PLATFORM_PRINTF_DEBUG("DPP keystore: Sync\n");

	keystore = cJSON_CreateObject();

	// GTK 1905
	ks_gtk = cJSON_CreateObject();
	bin2hexstr((char *)gtk_1905.gtk, hex_bytes, gtk_1905.gtk_len);
	cJSON_AddStringToObject(ks_gtk, "gtk", hex_bytes);
	cJSON_AddNumberToObject(ks_gtk, "gtk_len", gtk_1905.gtk_len);
	cJSON_AddItemToObject(keystore, "gtk_1905", ks_gtk);

	// PTK 1905 List
	ks_ptk_list = cJSON_CreateArray();
	for (entry = ptk_1905_list_head.next; entry != NULL; entry = entry->next) {
		struct ptk_1905_entry *ptk_entry = (struct ptk_1905_entry *)entry;

		ks_ptk = cJSON_CreateObject();
		bin2hexstr((char *)ptk_entry->mac_addr, hex_bytes, 6);
		cJSON_AddStringToObject(ks_ptk, "mac_addr", hex_bytes);

		bin2hexstr((char *)ptk_entry->ptk_1905.kck, hex_bytes, ptk_entry->ptk_1905.kck_len);
		cJSON_AddStringToObject(ks_ptk, "kck", hex_bytes);
		cJSON_AddNumberToObject(ks_ptk, "kck_len", ptk_entry->ptk_1905.kck_len);

		bin2hexstr((char *)ptk_entry->ptk_1905.kek, hex_bytes, ptk_entry->ptk_1905.kek_len);
		cJSON_AddStringToObject(ks_ptk, "kek", hex_bytes);
		cJSON_AddNumberToObject(ks_ptk, "kek_len", ptk_entry->ptk_1905.kek_len);

		bin2hexstr((char *)ptk_entry->ptk_1905.tk, hex_bytes, ptk_entry->ptk_1905.tk_len);
		cJSON_AddStringToObject(ks_ptk, "tk", hex_bytes);
		cJSON_AddNumberToObject(ks_ptk, "tk_len", ptk_entry->ptk_1905.tk_len);

		cJSON_AddItemToArray(ks_ptk_list, ks_ptk);
	}
	cJSON_AddItemToObject(keystore, "ptk_list", ks_ptk_list);

	// Generate JSON string
	json = cJSON_PrintUnformatted(keystore);
	if (!json) {
		PLATFORM_PRINTF_WARNING("DPP keystore: cannot generate json!\n");
		goto fail;
	}

	// Write to file
	if ((pFile = fopen(DPP_KEYSTORE_FILE, "we+")) == NULL) {
		PLATFORM_PRINTF_WARNING("DPP keystore: fopen write failed!\n");
		goto fail;
	}
	fputs(json, pFile);
	fclose(pFile);
	ret = 1;

fail:
	cJSON_Delete(keystore);
	PLATFORM_FREE(json);
	return ret;
}

static INT8U dpp_keystore_read()
{
	cJSON *keystore = NULL, *token = NULL, *t = NULL;
	FILE * pFile    = NULL;
	char * json     = NULL;
	size_t json_len = 0;

	INT8U ret = 0;

	PLATFORM_PRINTF_DEBUG("DPP keystore: read\n");

	// Open the file
	if ((pFile = fopen(DPP_KEYSTORE_FILE, "re")) == NULL) {
		PLATFORM_PRINTF_WARNING("DPP keystore: fopen failed: %s\n", strerror(errno));
		goto fail;
	}

	if (getline(&json, &json_len, pFile) == -1) {
		PLATFORM_PRINTF_WARNING("DPP keystore: getline error!\n");
		goto fail;
	}

	// Parse JSON
	keystore = cJSON_Parse(json);
	if (!keystore) {
		PLATFORM_PRINTF_WARNING("DPP keystore: Could not parse keystore\n");
		goto fail;
	}

	// GTK 1905
	token = cJSON_GetObjectItem(keystore, "gtk_1905");
	if (token != NULL && token->type == cJSON_Object) {
		cJSON *gtk_hex = cJSON_GetObjectItem(token, "gtk");
		cJSON *gtk_len = cJSON_GetObjectItem(token, "gtk_len");
		if ((gtk_hex != NULL && gtk_hex->type == cJSON_String)
		    && (gtk_len != NULL && gtk_len->type == cJSON_Number)) {
			if (gtk_len->valueint <= SHA512_MAC_LEN) {
				struct dpp_gtk gtk;
				gtk.gtk_len = gtk_len->valueint;
				hexstr2bin(gtk_hex->valuestring, (char *)gtk.gtk, gtk.gtk_len);
				PLATFORM_MEMCPY(&gtk_1905, &gtk, sizeof(gtk));
			} else {
				PLATFORM_PRINTF_WARNING("DPP keystore: Invalid gtk_len!\n");
			}
		} else {
			PLATFORM_PRINTF_WARNING("DPP keystore: gtk_1905 or gtk_len missing!\n");
		}
	}

	// PTK 1905 List
	token = cJSON_GetObjectItem(keystore, "ptk_list");
	if (!token || token->type != cJSON_Array) {
		PLATFORM_PRINTF_WARNING("DPP keystore: no ptk_list\n");
		goto fail;
	}

	dpp_keystore_free_all();

	cJSON_ArrayForEach(t, token)
	{
		cJSON *mac_addr = cJSON_GetObjectItem(t, "mac_addr");
		cJSON *kck_hex  = cJSON_GetObjectItem(t, "kck");
		cJSON *kck_len  = cJSON_GetObjectItem(t, "kck_len");
		cJSON *kek_hex  = cJSON_GetObjectItem(t, "kek");
		cJSON *kek_len  = cJSON_GetObjectItem(t, "kek_len");
		cJSON *tk_hex   = cJSON_GetObjectItem(t, "tk");
		cJSON *tk_len   = cJSON_GetObjectItem(t, "tk_len");
		if ((mac_addr && mac_addr->type == cJSON_String)
		    && (kck_hex && kck_hex->type == cJSON_String)
		    && (kck_len && kck_len->type == cJSON_Number)
		    && (kek_hex && kek_hex->type == cJSON_String)
		    && (kek_len && kek_len->type == cJSON_Number)
		    && (tk_hex && tk_hex->type == cJSON_String)
		    && (tk_len && tk_len->type == cJSON_Number)) {
			if (kck_len->valueint <= EAPOL_KCK_MAX_LEN
			    && kek_len->valueint <= EAPOL_KEK_MAX_LEN
			    && tk_len->valueint <= EAPOL_TK_MAX_LEN) {
				struct dpp_ptk ptk;
				INT8U          mac[6] = { 0 };
				PLATFORM_MEMSET(&ptk, 0, sizeof(ptk));
				hexstr2bin(mac_addr->valuestring, (char *)mac, 6);
				hexstr2bin(kck_hex->valuestring, (char *)ptk.kck, kck_len->valueint);
				ptk.kck_len = kck_len->valueint;
				hexstr2bin(kek_hex->valuestring, (char *)ptk.kek, kek_len->valueint);
				ptk.kek_len = kek_len->valueint;
				hexstr2bin(tk_hex->valuestring, (char *)ptk.tk, tk_len->valueint);
				ptk.tk_len = tk_len->valueint;
				dpp_keystore_insert_ptk(mac, &ptk);
			} else {
				PLATFORM_PRINTF_WARNING("DPP keystore: invalid lengths!\n");
			}
		} else {
			PLATFORM_PRINTF_WARNING("DPP keystore: some attributes are missing in the PTK!\n");
		}
	}

	ret          = 1;
	read_success = 1;

fail:
	if (pFile != NULL) {
		// pFile successfully opened, need to be closed
		fclose(pFile);
	}
	PLATFORM_FREE(json);
	cJSON_Delete(keystore);
	return ret;
}

//////////////////////////////////////////
//           Public Functions           //
//////////////////////////////////////////
struct dpp_gtk *dpp_keystore_get_gtk()
{
	if (!read_success)
		dpp_keystore_read();

	return gtk_1905.gtk_len ? &gtk_1905 : NULL;
}

INT8U dpp_keystore_set_gtk(struct dpp_gtk *gtk)
{
	if (!read_success)
		dpp_keystore_read();

	PLATFORM_MEMCPY(&gtk_1905, gtk, sizeof(struct dpp_gtk));
	return dpp_keystore_sync();
}

struct dpp_ptk *dpp_keystore_get_ptk(INT8U *mac_addr)
{
	if (!read_success)
		dpp_keystore_read();

	return dpp_keystore_search_ptk(mac_addr);
}

INT8U dpp_keystore_set_ptk(INT8U *mac_addr, struct dpp_ptk *ptk)
{
	struct dpp_ptk *found_ptk = dpp_keystore_get_ptk(mac_addr);

	if (ptk && found_ptk) {
		PLATFORM_PRINTF_DEBUG(
		    "DPP keystore: Replacing PTK in cache (%02X:%02X:%02X:%02X:%02X:%02X)\n",
		    mac_addr[0], mac_addr[1], mac_addr[2],
		    mac_addr[3], mac_addr[4], mac_addr[5]);
		PLATFORM_MEMCPY(found_ptk, ptk, sizeof(*ptk));

	} else if (ptk && !found_ptk) {
		dpp_keystore_insert_ptk(mac_addr, ptk);

	} else if (found_ptk && !ptk) {
		dpp_keystore_remove_ptk(mac_addr);

	} else {
		PLATFORM_PRINTF_WARNING("DPP keystore: Invalid mac_addr and PTK\n");
		return 0;
	}

	return dpp_keystore_sync();
}

INT8U dpp_keystore_remove_all()
{
	PLATFORM_PRINTF_WARNING("DPP keystore: Remove all\n");
	PLATFORM_MEMSET(dpp_keystore_get_gtk(), 0, sizeof(struct dpp_gtk));
	dpp_keystore_free_all();
	return dpp_keystore_sync();
}

INT8U *dpp_get_connected(INT8U *connected_nr)
{
	cJSON *keystore = NULL, *token = NULL, *t = NULL;
	FILE  *pFile    = NULL;
	char  *json     = NULL;
	size_t json_len = 0;

	INT8U *al_mac_stream = NULL;
	INT8U  al_mac_nr = 0, al_mac_count = 0;

	// Open the file
	if ((pFile = fopen(DPP_KEYSTORE_FILE, "re")) == NULL) {
		PLATFORM_PRINTF_WARNING("DPP keystore: fopen failed: %s\n", strerror(errno));
		return NULL;
	}

	if (getline(&json, &json_len, pFile) == -1) {
		PLATFORM_PRINTF_WARNING("DPP keystore: getline error!\n");
		goto fail;
	}

	// Parse JSON
	keystore = cJSON_Parse(json);
	if (!keystore) {
		PLATFORM_PRINTF_WARNING("DPP keystore: Could not parse keystore\n");
		goto fail;
	}

	// PTK 1905 List
	token = cJSON_GetObjectItem(keystore, "ptk_list");
	if (!token || token->type != cJSON_Array) {
		PLATFORM_PRINTF_WARNING("DPP keystore: no ptk_list\n");
		goto fail;
	}

	al_mac_nr     = cJSON_GetArraySize(token);
	al_mac_stream = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * MAC_ADDR_LEN * al_mac_nr);

	cJSON_ArrayForEach(t, token)
	{
		cJSON *mac_addr = cJSON_GetObjectItem(t, "mac_addr");
		if (mac_addr && mac_addr->type == cJSON_String) {
			INT8U mac[MAC_ADDR_LEN] = { 0 };
			hexstr2bin(mac_addr->valuestring, (char *)mac, MAC_ADDR_LEN);
			PLATFORM_MEMCPY(al_mac_stream + al_mac_count, mac, MAC_ADDR_LEN);
		}
		al_mac_count += MAC_ADDR_LEN;
	}

	*connected_nr = al_mac_nr;

fail:
	fclose(pFile);
	PLATFORM_FREE(json);
	cJSON_Delete(keystore);
	return al_mac_stream;
}