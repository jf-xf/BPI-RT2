/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>          // errno
#include <sys/ioctl.h>      // ioctl()
#include <sys/socket.h>     // socket()
#include <linux/wireless.h> // struct iwreq
#include <unistd.h>         // close()

#include "config_file_handler.h"
#include "mib.h"
#include "sysconfig.h"
#ifdef WLAN_SCHEDULE_SUPPORT 
#include "subr_wlan.h"
#endif
#include "subr_multiap.h"

#include "ini.h"
#include "easymesh_datamodel.h"
#include "map_initialization.h"

#define DPP_CSIGN_KEY_PATH "/tmp/dpp_csign_key"
#define DPP_NET_ACCESS_KEY_PATH "/tmp/dpp_netaccess_key"
#define DPP_PRIVACY_PROTECTION_KEY_PATH "/tmp/dpp_pp_key"
#define DPP_CONNECTOR_PATH_1905 "/tmp/dpp_map_conn"

#if !defined(__UCLIBC__)
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
#endif

#ifdef EASYMESH_ISP_VENDOR_SYNC_SUPPORT
struct vendor_acl_info {
	unsigned char    acl_nr;
	struct acl_info *acl_data;
};

struct vendor_schedule_info {
	unsigned char    		   wlan_schedule_nr;
	struct wlan_schedule_info *wlan_schedule_data;
};

int change_mac_to_octe(char *str_mac, unsigned char *octe_mac)
{
	if(str_mac==NULL || octe_mac==NULL)
		return -1;

	int i, j;
	char tmp_str_mac[13]={0};

	for(i=0, j=0; i<17 && j<12; i++)
	{
		if(str_mac[i]!=':')
			tmp_str_mac[j++]=str_mac[i];
	}
	tmp_str_mac[12]=0;

	if (strlen(tmp_str_mac)==12 && rtk_string_to_hex(tmp_str_mac, octe_mac, 12))
		return 0;

	return -1;
}

#ifdef WLAN_VAP_ACL
#ifdef WLAN_ACL_FOR_MULTIAP
uint8_t _delall_wlan_acl_mib()
{
	int i = 0, j = 0, radio_idx = 0, entryNum = 0;
	MIB_CE_WLAN_AC_T acl_entry;

	for ( i = 0; i < NUM_WLAN_INTERFACE; i++ ){
		radio_idx  = i;
		entryNum   = mib_chain_total(MIB_WLAN_AC_TBL + radio_idx);
		for ( j = entryNum; j > 0; j-- ){
			memset(&acl_entry, 0, sizeof(MIB_CE_WLAN_AC_T));
			if ( !mib_chain_get(MIB_WLAN_AC_TBL + radio_idx, j-1, (void *)&acl_entry) ){
				printf("(%s) %d ERROR get wlan_ac_tbl fail\n",__FUNCTION__,__LINE__);
				return 0;
			}

			if ( acl_entry.wlanIdx != radio_idx )
				continue;

			if ( !mib_chain_delete(MIB_WLAN_AC_TBL + radio_idx, j-1) ) {
				printf("(%s) %d ERROR del wlan_ac_tbl fail\n",__FUNCTION__,__LINE__);
				return 0;
			}
		}
	}
	return 1;
}
#else
uint8_t _delall_wlan_acl_mib()
{
	int i = 0, j = 0, radio_idx = 0, entryNum = 0, k=0, vap_idx = 0;
	MIB_CE_WLAN_AC_T acl_entry;

	for ( i = 0; i < NUM_WLAN_INTERFACE; i++ ){
		for(k = 0; k < WLAN_MBSSID_NUM+1; k++){
			radio_idx  = i;
			vap_idx = k;
			entryNum   = mib_chain_total(MIB_WLAN_AC_TBL + radio_idx + WLAN_MIB_MAPPING_STEP*vap_idx);
			for ( j = entryNum; j > 0; j-- ){
				memset(&acl_entry, 0, sizeof(MIB_CE_WLAN_AC_T));
				if ( !mib_chain_get(MIB_WLAN_AC_TBL + radio_idx + WLAN_MIB_MAPPING_STEP*vap_idx, j-1, (void *)&acl_entry) ){
					printf("(%s) %d ERROR get wlan_ac_tbl fail\n",__FUNCTION__,__LINE__);
					return 0;
				}

				if ( acl_entry.wlanIdx != radio_idx )
					continue;

				if ( !mib_chain_delete(MIB_WLAN_AC_TBL + radio_idx + WLAN_MIB_MAPPING_STEP*vap_idx, j-1) ) {
					printf("(%s) %d ERROR del wlan_ac_tbl fail\n",__FUNCTION__,__LINE__);
					return 0;
				}
			}
		}
	}

	return 1;
}
#endif

uint8_t _set_wlan_acl_mib(struct acl_info acl_data)
{

	MIB_CE_MBSSIB_T wlan_entry;
	MIB_CE_WLAN_AC_T acl_entry;

	memset(&wlan_entry, 0, sizeof(MIB_CE_MBSSIB_T));
	if ( !mib_chain_get(MIB_MBSSIB_TBL + acl_data.wlan_Idx, acl_data.vwlanIdx, (void *)&wlan_entry) ){
		printf("(%s) %d ERROR get MBSSIB_TBL fail!\n",__FUNCTION__,__LINE__);
		return 0;
	}

	if (wlan_entry.acl_enable != acl_data.acl_enable) {
		wlan_entry.acl_enable = acl_data.acl_enable;
		if ( !mib_chain_update(MIB_MBSSIB_TBL + acl_data.wlan_Idx, (void *)&wlan_entry, acl_data.vwlanIdx) ) {
			printf("(%s) %d ERROR updata MBSSIB_TBL fail!\n",__FUNCTION__,__LINE__);
			return 0;
		}
	}

	if (acl_data.entry_num != 0) {
		memset(&acl_entry, 0, sizeof(MIB_CE_WLAN_AC_T));
		//set acl entry
		acl_entry.wlanIdx  =  acl_data.wlan_Idx;

#ifdef WLAN_ACL_FOR_MULTIAP
		acl_entry.vwlanIdx =  acl_data.vwlanIdx;
#endif
		memcpy(acl_entry.macAddr, acl_data.macAddr, MAC_ADDR_LEN);

#ifdef WLAN_ACL_FOR_MULTIAP
		if ( !mib_chain_add(MIB_WLAN_AC_TBL + acl_entry.wlanIdx, (void *)&acl_entry) )
#else
		if ( !mib_chain_add(MIB_WLAN_AC_TBL + acl_entry.wlanIdx+acl_data.vwlanIdx*WLAN_MIB_MAPPING_STEP, (void *)&acl_entry) )
#endif
		{
			printf("(%s) %d ERROR add wlan_ac_tbl fail!\n",__FUNCTION__,__LINE__);
			return 0;
		}
	}

	return 1;
}
#endif

#ifdef WLAN_SCHEDULE_SUPPORT
uint8_t _delall_wlan_schedule_mib()
{
	int i = 0, j = 0, radio_idx = 0, entryNum = 0;
	MIB_CE_SCHEDULE_T  schedule_entry;

	for ( i = 0; i < NUM_WLAN_INTERFACE; i++ ){
		radio_idx  = i;
		mib_get_s(MIB_WLAN_SCHEDULE_TBL_NUM+ radio_idx, (void *)&entryNum, sizeof(entryNum));
		for ( j = 0; j < entryNum; j++ ){
			memset(&schedule_entry, 0, sizeof(MIB_CE_SCHEDULE_T));
			if ( !mib_chain_get(MIB_WLAN_SCHEDULE_TBL + radio_idx, j, (void *)&schedule_entry) ){
				printf("(%s) %d ERROR get wlan_schedule_tbl fail\n",__FUNCTION__,__LINE__);
				return 0;
			}

			schedule_entry.eco   = 0;
			schedule_entry.fTime = 0;
			schedule_entry.tTime = 0;
			schedule_entry.day   = 0;
			schedule_entry.inst_num = 0;

			if ( !mib_chain_update(MIB_WLAN_SCHEDULE_TBL + radio_idx, (void *)&schedule_entry, j) ){
				printf("(%s) %d ERROR update wlan_schedule_tbl fail!\n",__FUNCTION__,__LINE__);
				return 0;
			}
		}
		entryNum = 0;
		mib_set(MIB_WLAN_SCHEDULE_TBL_NUM + radio_idx, (void *)&entryNum);
	}

	return 1;
}

uint8_t _set_wlan_schedule_mib(struct wlan_schedule_info schedule_data)
{
	int radio_idx = schedule_data.wlan_Idx;
	unsigned int enable = 0, entryNum = 0, mode = 0;
	MIB_CE_SCHEDULE_T  schedule_entry;

	mib_get_s(MIB_WLAN_SCHEDULE_ENABLED + radio_idx, (void *)&enable, sizeof(enable));
	mib_get_s(MIB_WLAN_SCHEDULE_TBL_NUM + radio_idx, (void *)&entryNum, sizeof(entryNum));
	mib_get_s(MIB_WLAN_SCHEDULE_MODE + radio_idx, (void *)&mode, sizeof(mode));

	if (enable != schedule_data.enable){
		enable = schedule_data.enable;
		mib_set(MIB_WLAN_SCHEDULE_ENABLED + radio_idx, (void *)&enable);
	}

	if (mode != schedule_data.mode){
		mode = schedule_data.mode;
		mib_set(MIB_WLAN_SCHEDULE_MODE + radio_idx, (void *)&mode);
	}

	if (schedule_data.entry_num == 0) {
		entryNum = schedule_data.entry_num;
		mib_set(MIB_WLAN_SCHEDULE_TBL_NUM + radio_idx, (void *)&entryNum);
	} else {
		memset(&schedule_entry, 0, sizeof(MIB_CE_SCHEDULE_T));
		if ( !mib_chain_get(MIB_WLAN_SCHEDULE_TBL + radio_idx, entryNum, &schedule_entry) ){
			printf("(%s) %d ERROR get wlan_schedule_tbl fail!\n",__FUNCTION__,__LINE__);
			return 0;
		}

		schedule_entry.eco      = schedule_data.eco;
		schedule_entry.fTime    = schedule_data.fTime;
		schedule_entry.tTime    = schedule_data.tTime;
		schedule_entry.day      = schedule_data.day;
		schedule_entry.inst_num = schedule_data.inst_num;

		if ( !mib_chain_update(MIB_WLAN_SCHEDULE_TBL + radio_idx, (void *)&schedule_entry, entryNum) ){
			printf("(%s) %d ERROR update wlan_schedule_tbl fail!\n",__FUNCTION__,__LINE__);
			return 0;
		}

		entryNum += 1;
		mib_set(MIB_WLAN_SCHEDULE_TBL_NUM + radio_idx, (void *)&entryNum);
	}

	return 1;
}
#endif

uint8_t process_rtk_vendor_spec_data(char *pchar, struct vendor_acl_info *vendor_acl_data, struct vendor_schedule_info *vendor_schedule_data, char *key)
{
	char *token=NULL, *token2=NULL, *savestr1=NULL, *savestr2=NULL, *char_tmp=NULL;
	int i=0;

	if (strcmp(key, "ACL") == 0){
		//process ACL payload
		token = strtok_r(pchar, ";", &savestr1);
		do {
			if (token == NULL){
				break;
			} else{
				i = vendor_acl_data->acl_nr =  vendor_acl_data->acl_nr + 1;
				vendor_acl_data->acl_data   =  (struct acl_info *)realloc(vendor_acl_data->acl_data, sizeof(struct acl_info) * i);

				token2 = strtok_r(token, " ", &savestr2);
				do{
					if (token2 == NULL){
						break;
					} else{
						if (strstr(token2, "wlan_Idx") != NULL) {
							char_tmp  =  strchr(token2, ':');
							char_tmp  += 1;
							vendor_acl_data->acl_data[i-1].wlan_Idx = atoi(char_tmp);
						}

						if (strstr(token2, "vwlanIdx") != NULL) {
							char_tmp  =  strchr(token2, ':');
							char_tmp  += 1;
							vendor_acl_data->acl_data[i-1].vwlanIdx = atoi(char_tmp);
						}

						if (strstr(token2, "acl_enable") != NULL) {
							char_tmp  =  strchr(token2, ':');
							char_tmp  += 1;
							vendor_acl_data->acl_data[i-1].acl_enable = atoi(char_tmp);
						}

						if (strstr(token2, "entry_num") != NULL) {
							char_tmp  =  strchr(token2, ':');
							char_tmp  += 1;
							vendor_acl_data->acl_data[i-1].entry_num = atoi(char_tmp);
						}

						if (strstr(token2, "macAddr") != NULL) {
							char_tmp  =  strchr(token2, ':');
							char_tmp  += 1;
							change_mac_to_octe(char_tmp, vendor_acl_data->acl_data[i-1].macAddr);
						}
					}
					token2 = strtok_r(NULL, " " , &savestr2);
				}while(token2 != NULL);
			}
			token = strtok_r(NULL, ";", &savestr1);
		}while(token != NULL);
	}else if(strcmp(key, "WLAN_SCHEDULE") == 0){
		//process WLAN_SCHEDULE payload
		token = strtok_r(pchar, ";", &savestr1);
		do {
			if (token == NULL){
				break;
			} else{
				i = vendor_schedule_data->wlan_schedule_nr =  vendor_schedule_data->wlan_schedule_nr + 1;
				vendor_schedule_data->wlan_schedule_data   =  (struct wlan_schedule_info *)realloc(vendor_schedule_data->wlan_schedule_data, sizeof(struct wlan_schedule_info) * i);

				token2 = strtok_r(token, " ", &savestr2);
				do{
					if (token2 == NULL){
						break;
					} else{
						if (strstr(token2, "wlan_Idx") != NULL) {
							char_tmp  =  strchr(token2, ':');
							char_tmp  += 1;
							vendor_schedule_data->wlan_schedule_data[i-1].wlan_Idx = atoi(char_tmp);
						}

						if (strstr(token2, "enable") != NULL) {
							char_tmp  =  strchr(token2, ':');
							char_tmp  += 1;
							vendor_schedule_data->wlan_schedule_data[i-1].enable = atoi(char_tmp);
						}

						if (strstr(token2, "entry_num") != NULL) {
							char_tmp  =  strchr(token2, ':');
							char_tmp  += 1;
							vendor_schedule_data->wlan_schedule_data[i-1].entry_num = atoi(char_tmp);
						}

						if (strstr(token2, "mode") != NULL) {
							char_tmp  =  strchr(token2, ':');
							char_tmp  += 1;
							vendor_schedule_data->wlan_schedule_data[i-1].mode = atoi(char_tmp);
						}

						if (strstr(token2, "eco") != NULL) {
							char_tmp  =  strchr(token2, ':');
							char_tmp  += 1;
							vendor_schedule_data->wlan_schedule_data[i-1].eco = atoi(char_tmp);
						}

						if (strstr(token2, "fTime") != NULL) {
							char_tmp  =  strchr(token2, ':');
							char_tmp  += 1;
							vendor_schedule_data->wlan_schedule_data[i-1].fTime = atoi(char_tmp);
						}

						if (strstr(token2, "tTime") != NULL) {
							char_tmp  =  strchr(token2, ':');
							char_tmp  += 1;
							vendor_schedule_data->wlan_schedule_data[i-1].tTime = atoi(char_tmp);
						}

						if (strstr(token2, "day") != NULL) {
							char_tmp  =  strchr(token2, ':');
							char_tmp  += 1;
							vendor_schedule_data->wlan_schedule_data[i-1].day = atoi(char_tmp);
						}

						if (strstr(token2, "inst_num") != NULL) {
							char_tmp  =  strchr(token2, ':');
	           				char_tmp  += 1;
	           				vendor_schedule_data->wlan_schedule_data[i-1].inst_num = atoi(char_tmp);
	           			}
					}
					token2 = strtok_r(NULL, " " , &savestr2);
				}while(token2 != NULL);
			}
			token = strtok_r(NULL, ";", &savestr1);
		}while(token != NULL);
	}

	return 0;
}

uint8_t _get_orig_vendor_data (struct vendor_acl_info *vendor_acl_data, struct vendor_schedule_info *vendor_schedule_data)
{
#if defined(WLAN_VAP_ACL) || defined(WLAN_SCHEDULE_SUPPORT)
	int i = 0, j = 0, radio_idx = 0, payload_idx = 0, entryNum = 0;
#endif
#if defined(WLAN_VAP_ACL) && !defined(WLAN_ACL_FOR_MULTIAP)
	int k = 0, vap_idx = 0;
#endif
#ifdef WLAN_VAP_ACL
	MIB_CE_MBSSIB_T    wlan_entry;
	MIB_CE_WLAN_AC_T   acl_entry;
#endif
#ifdef WLAN_SCHEDULE_SUPPORT
	int enable = 0, mode = 0;
	MIB_CE_SCHEDULE_T  schedule_entry;
#endif

	vendor_acl_data->acl_nr = 0;
#ifdef WLAN_VAP_ACL
#ifdef WLAN_ACL_FOR_MULTIAP
	for (i = 0; i < NUM_WLAN_INTERFACE; i++) {
		radio_idx  = i;
		entryNum = mib_chain_total(MIB_WLAN_AC_TBL + radio_idx);

		memset(&wlan_entry, 0, sizeof(MIB_CE_MBSSIB_T));
		if(!mib_chain_get(MIB_MBSSIB_TBL + radio_idx, 0, &wlan_entry)){
			printf("(%s) %d ERROR get MBSSIB_TBL fail!\n",__FUNCTION__,__LINE__);
			return 1;
		}

		if (entryNum == 0) {
			payload_idx = vendor_acl_data->acl_nr	+= 1;
			vendor_acl_data->acl_data = (struct acl_info *)realloc(vendor_acl_data->acl_data, sizeof(struct acl_info) * payload_idx);
			memset(&(vendor_acl_data->acl_data[payload_idx - 1]), 0, sizeof(struct acl_info));
			vendor_acl_data->acl_data[payload_idx - 1].wlan_Idx   = radio_idx;
			vendor_acl_data->acl_data[payload_idx - 1].acl_enable = wlan_entry.acl_enable;
			vendor_acl_data->acl_data[payload_idx - 1].entry_num  = entryNum;
		} else {
			for (j = 0; j < entryNum; j++) {
				payload_idx = vendor_acl_data->acl_nr   += 1;
				vendor_acl_data->acl_data = (struct acl_info *)realloc(vendor_acl_data->acl_data, sizeof(struct acl_info) * payload_idx);

				memset(&acl_entry, 0, sizeof(MIB_CE_WLAN_AC_T));
				if(!mib_chain_get(MIB_WLAN_AC_TBL + radio_idx, j, &acl_entry)){
					printf("(%s) %d ERROR get wlan_ac_tbl fail\n",__FUNCTION__,__LINE__);
					return 1;
				}
				memset(&(vendor_acl_data->acl_data[payload_idx - 1]), 0, sizeof(struct acl_info));
				vendor_acl_data->acl_data[payload_idx - 1].wlan_Idx   = acl_entry.wlanIdx;
				vendor_acl_data->acl_data[payload_idx - 1].vwlanIdx   = acl_entry.vwlanIdx;
				vendor_acl_data->acl_data[payload_idx - 1].acl_enable = wlan_entry.acl_enable;
				vendor_acl_data->acl_data[payload_idx - 1].entry_num  = entryNum;
				memcpy(vendor_acl_data->acl_data[payload_idx - 1].macAddr, acl_entry.macAddr, MACADDRLEN);
			}
		}
	}
#else
	for (i = 0; i < NUM_WLAN_INTERFACE; i++) {
		for(k = 0; k < WLAN_MBSSID_NUM+1; k++) {
			radio_idx  = i;
			vap_idx = k;
			entryNum = mib_chain_total(MIB_WLAN_AC_TBL + radio_idx + vap_idx*WLAN_MIB_MAPPING_STEP);

			memset(&wlan_entry, 0, sizeof(MIB_CE_MBSSIB_T));
			if(!mib_chain_get(MIB_MBSSIB_TBL + radio_idx, 0, &wlan_entry)){
				printf("(%s) %d ERROR get MBSSIB_TBL fail!\n",__FUNCTION__,__LINE__);
				return 1;
			}

			if (entryNum == 0) {
				payload_idx = vendor_acl_data->acl_nr	+= 1;
				vendor_acl_data->acl_data = (struct acl_info *)realloc(vendor_acl_data->acl_data, sizeof(struct acl_info) * payload_idx);


				memset(&(vendor_acl_data->acl_data[payload_idx - 1]), 0, sizeof(struct acl_info));
				vendor_acl_data->acl_data[payload_idx - 1].wlan_Idx   = radio_idx;
				vendor_acl_data->acl_data[payload_idx - 1].acl_enable = wlan_entry.acl_enable;
				vendor_acl_data->acl_data[payload_idx - 1].entry_num  = entryNum;
			} else {
				for (j = 0; j < entryNum; j++) {
					payload_idx = vendor_acl_data->acl_nr   += 1;
					vendor_acl_data->acl_data = (struct acl_info *)realloc(vendor_acl_data->acl_data, sizeof(struct acl_info) * payload_idx);

					memset(&acl_entry, 0, sizeof(MIB_CE_WLAN_AC_T));
					if(!mib_chain_get(MIB_WLAN_AC_TBL + radio_idx + vap_idx*WLAN_MIB_MAPPING_STEP, j, &acl_entry)){
						printf("(%s) %d ERROR get wlan_ac_tbl fail\n",__FUNCTION__,__LINE__);
						return 1;
					}
					memset(&(vendor_acl_data->acl_data[payload_idx - 1]), 0, sizeof(struct acl_info));
					vendor_acl_data->acl_data[payload_idx - 1].wlan_Idx   = acl_entry.wlanIdx;
					vendor_acl_data->acl_data[payload_idx - 1].vwlanIdx   = vap_idx;
					vendor_acl_data->acl_data[payload_idx - 1].acl_enable = wlan_entry.acl_enable;
					vendor_acl_data->acl_data[payload_idx - 1].entry_num  = entryNum;
					memcpy(vendor_acl_data->acl_data[payload_idx - 1].macAddr, acl_entry.macAddr, MACADDRLEN);
				}
			}
		}
	}
#endif
#endif

	vendor_schedule_data->wlan_schedule_nr = 0;
#ifdef WLAN_SCHEDULE_SUPPORT
	for (i = 0; i < NUM_WLAN_INTERFACE; i++) {
		radio_idx  = i;
		mib_get_s(MIB_WLAN_SCHEDULE_ENABLED + radio_idx, (void *)&enable,sizeof(enable));
		mib_get_s(MIB_WLAN_SCHEDULE_TBL_NUM + radio_idx, (void *)&entryNum,sizeof(entryNum));
		mib_get_s(MIB_WLAN_SCHEDULE_MODE + radio_idx, (void *)&mode, sizeof(mode));
		if (entryNum == 0) {
			payload_idx = vendor_schedule_data->wlan_schedule_nr += 1;
			vendor_schedule_data->wlan_schedule_data = (struct wlan_schedule_info *)realloc(vendor_schedule_data->wlan_schedule_data, sizeof(struct wlan_schedule_info) * payload_idx);

			memset(&(vendor_schedule_data->wlan_schedule_data[payload_idx - 1]), 0, sizeof(struct wlan_schedule_info));
			vendor_schedule_data->wlan_schedule_data[payload_idx - 1].wlan_Idx   = radio_idx;
			vendor_schedule_data->wlan_schedule_data[payload_idx - 1].enable     = enable;
			vendor_schedule_data->wlan_schedule_data[payload_idx - 1].entry_num  = entryNum;
			vendor_schedule_data->wlan_schedule_data[payload_idx - 1].mode       = mode;
		} else {
			for (j = 0; j < entryNum; j++) {
				payload_idx = vendor_schedule_data->wlan_schedule_nr += 1;
				vendor_schedule_data->wlan_schedule_data = (struct wlan_schedule_info *)realloc(vendor_schedule_data->wlan_schedule_data, sizeof(struct wlan_schedule_info) * payload_idx);

				memset(&schedule_entry, 0, sizeof(MIB_CE_SCHEDULE_T));
				if(!mib_chain_get(MIB_WLAN_SCHEDULE_TBL + radio_idx, j, &schedule_entry)){
					printf("(%s) %d Error get wlan_schedule_tbl!\n",__FUNCTION__,__LINE__);
					return 1;
				}
				memset(&(vendor_schedule_data->wlan_schedule_data[payload_idx - 1]), 0, sizeof(struct wlan_schedule_info));
				vendor_schedule_data->wlan_schedule_data[payload_idx - 1].wlan_Idx   = radio_idx;
				vendor_schedule_data->wlan_schedule_data[payload_idx - 1].enable     = enable;
				vendor_schedule_data->wlan_schedule_data[payload_idx - 1].entry_num  = entryNum;
				vendor_schedule_data->wlan_schedule_data[payload_idx - 1].mode       = mode;
				vendor_schedule_data->wlan_schedule_data[payload_idx - 1].eco        = schedule_entry.eco;
				vendor_schedule_data->wlan_schedule_data[payload_idx - 1].fTime      = schedule_entry.fTime;
				vendor_schedule_data->wlan_schedule_data[payload_idx - 1].tTime      = schedule_entry.tTime;
				vendor_schedule_data->wlan_schedule_data[payload_idx - 1].day        = schedule_entry.day;
				vendor_schedule_data->wlan_schedule_data[payload_idx - 1].inst_num   = schedule_entry.inst_num;
			}
		}
	}
#endif

	return 0;
}

void _free_vendor_acl_data(struct vendor_acl_info *vendor_acl_data)
{
	if (vendor_acl_data->acl_data != NULL)
		free(vendor_acl_data->acl_data);
}

void _free_vendor_schedule_data(struct vendor_schedule_info *vendor_schedule_data)
{
	if (vendor_schedule_data->wlan_schedule_data != NULL)
		free(vendor_schedule_data->wlan_schedule_data);
}

#endif // EASYMESH_ISP_VENDOR_SYNC_SUPPORT

uint8_t _is_interface_up(char *interface)
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

uint8_t _set_radio_band_width(int radio_idx, int channel, int bandwidth, int sideband)
{
	printf("Setting Radio %d: channel: %d, bandwidth: %d, sideband: %d\n", radio_idx, channel, bandwidth, sideband);

	uint8_t value = channel;

	mib_set(MIB_WLAN_CHAN_NUM + radio_idx, (void *)&value);

	if (bandwidth != -1) {
		value = bandwidth;
		mib_set(MIB_WLAN_CHANNEL_WIDTH + radio_idx, (void *)&value);
	}

	if (sideband != -1) {
		value = sideband;
		mib_set(MIB_WLAN_CONTROL_BAND + radio_idx, (void *)&value);
	}

	return 0;
}

uint8_t _get_configure_state()
{
	unsigned char configured_band;
	mib_get(MIB_MAP_CONFIGURED_BAND, (void *)&configured_band);
	printf("Configured band: %02x\n", configured_band);
	return (uint8_t)configured_band;
}

uint8_t _set_configure_state(uint8_t configure_state)
{
	if (!mib_set(MIB_MAP_CONFIGURED_BAND, (void *)&configure_state)) {
		printf("[ERROR] Cannot set configured band.\n");
		return 1;
	}
	return 0;
}

uint8_t _set_ap_mib(uint8_t radio_idx, struct easymesh_interface_mib interface_mib)
{
	uint8_t val     = 0;
	int     vap_idx = 0;

	uint8_t encrypt_type = 0;
	uint8_t wsc_auth_type = WSC_AUTH_WPA2PSK;
	uint8_t use_sha_256 = 0, use_11w = 0;

	MIB_CE_MBSSIB_T entry;

	char interface_name[] = "wlan0-vap0";

	uint8_t connector_equal = 1;
	char connector_path[64] = { 0 };
	char akm_path[64] = { 0 };
	FILE *fd = NULL;

	if (radio_idx > 2) {
		// Only support wlan0, wlan1 and wlan2
		printf("Unknown radio index %d\n", radio_idx);
		return 1;
	}

	vap_idx = interface_mib.interface_index;
#if 0//defined(RTK_RSV_VAP_FOR_EASYMESH)
	if (vap_idx == MULTI_AP_BSS_IDX) {
		if (MULTI_AP_BACKHAUL_BSS_BIT != interface_mib.network_type) {
			vap_idx++;
		}
	} else if (vap_idx > MULTI_AP_BSS_IDX) {
		mib_chain_local_mapping_get(MIB_MBSSIB_TBL, radio_idx, WLAN_REPEATER_ITF_INDEX, (void *)&entry);
		if (!(entry.wlanDisabled == 0 && entry.wsc_disabled == 0 && entry.multiap_bss_type == MULTI_AP_BACKHAUL_STA_BIT)) {
			vap_idx++;
		}
	}

	if ((MULTI_AP_BACKHAUL_BSS_BIT == interface_mib.network_type) && (vap_idx != MULTI_AP_BSS_IDX)) {
		printf("[MULTI AP]: the Backhaul BSS interface is not reserved index\n");
	}
#endif

	interface_name[4] = '0' + radio_idx;
	if (vap_idx) {
		interface_name[9] = '0' + vap_idx - 1;
	} else {
		interface_name[5] = '\0';
	}

	mib_get(MIB_WLAN_DISABLED + radio_idx, (void *)&val);

	if (0 == interface_mib.is_enabled)
	{
		if (!val) {
			val = 1;
			mib_set(MIB_WLAN_DISABLED + radio_idx, (void *)&val);
		}

		(void)mib_chain_get(MIB_MBSSIB_TBL + radio_idx, vap_idx, &entry);
		entry.wlanDisabled = 1;
		mib_chain_update(MIB_MBSSIB_TBL + radio_idx, &entry, vap_idx);


		return 0;
	}

	if (val) {
		val = 0;
		mib_set(MIB_WLAN_DISABLED + radio_idx, (void *)&val);
	}

	(void)mib_chain_get(MIB_MBSSIB_TBL + radio_idx, vap_idx, &entry);

	mib_set(MIB_WLAN_SSID + radio_idx, (void *)&interface_mib.ssid);

	if (interface_mib.authentication_type & AUTH_DISABLED) {
		encrypt_type |= WIFI_SEC_NONE;
	}

	if (interface_mib.authentication_type & AUTH_WEP) {
		encrypt_type |= WIFI_SEC_WEP;
	}

	if (interface_mib.authentication_type & AUTH_WPA) {
		encrypt_type |= WIFI_SEC_WPA;
	}

	if (interface_mib.authentication_type & AUTH_WPA2) {
		encrypt_type |= WIFI_SEC_WPA2;
	}

#ifdef WLAN_WPA3
	if (interface_mib.authentication_type & AUTH_WPA3) {
		encrypt_type |= WIFI_SEC_WPA3;
		wsc_auth_type |= WSC_AUTH_WPA3SAE;
		use_sha_256 = 1;
		use_11w = 2;
		if (interface_mib.authentication_type & AUTH_WPA2) {
			use_sha_256 = 0;
			use_11w = 1;
		}
	}
#endif

#ifdef RTK_MULTI_AP_WFA
	if ((interface_mib.network_type & MULTI_AP_FRONTHAUL_BSS_BIT) || (interface_mib.network_type & MULTI_AP_BACKHAUL_BSS_BIT)) {
		if (!use_11w)
			use_11w = 1;
	}
#endif

	if (interface_mib.authentication_type & AUTH_DPP) {
		char content[512] = { 0 };
		snprintf(connector_path, sizeof(connector_path), "/tmp/dpp_%s_conn", interface_name);
		fd = fopen(connector_path, "r");
		connector_equal = 0;

		if (fd) {
			fgets(content, sizeof(content), fd);
			if (0 == strncmp(content, interface_mib.signed_connector, strlen(interface_mib.signed_connector))) {
				connector_equal = 1;
			}
			fclose(fd);
		}
	}

	mib_get(MIB_WLAN_DISABLED + radio_idx, (void *)&val);

	if (0 == entry.wlanDisabled && 0 == strcmp(interface_mib.ssid, (char *)entry.ssid)) {
		if (0 == strcmp(interface_mib.network_key, (char *)entry.wpaPSK) && entry.encrypt == encrypt_type) {
			if (interface_mib.network_type == entry.multiap_bss_type && connector_equal) {
				printf("Setting is identical for interface %s, skip configuration\n", interface_name);
				return 0;
			} else {
				// printf("[%s]: bss_type %d, bss_type to configure %d\n", interface_name, entry.multiap_bss_type, interface_mib.network_type);
			}
		} else {
			// printf("[%s]: encrypt_type %d, encrypt_type to configure %d, psk %s, psk to configure %s\n", interface_name, entry.encrypt, encrypt_type, (char *)entry.wpaPSK, interface_mib.network_key);
		}
	} else {
		// printf("[%s]: disabled %d, ssid %s, ssid to configure %s\n", interface_name, entry.wlanDisabled, (char *)entry.ssid, interface_mib.ssid);
	}

	entry.wlanDisabled = 0;

	entry.wlanMode = AP_MODE;

	strlcpy((char *)entry.ssid, interface_mib.ssid, sizeof(entry.ssid));

	entry.multiap_bss_type = interface_mib.network_type;

#ifndef RTK_MULTI_AP_WFA
	// HIDDEN SSID
	if (MULTI_AP_BACKHAUL_BSS_BIT == interface_mib.network_type) {
		entry.hidessid = 1;
	}
#endif

#ifdef WLAN_11W
		entry.sha256        = use_sha_256;
		entry.dotIEEE80211W = use_11w;
#endif

	entry.encrypt           = encrypt_type;
	entry.wsc_auth          = wsc_auth_type;
	entry.wpa2UnicastCipher = WPA_CIPHER_AES;
	entry.unicastCipher     = 0;
	entry.wsc_enc           = WSC_ENCRYPT_AES;

	/*** Customized auth type support start ***/
	if (!(interface_mib.network_type & MULTI_AP_BACKHAUL_BSS_BIT)) {
		if (interface_mib.authentication_type == (AUTH_WPA | AUTH_WPA2)) {
			entry.encrypt           = WIFI_SEC_WPA2_MIXED;
			entry.wsc_auth          = WSC_AUTH_WPA2PSKMIXED;
			entry.wpa2UnicastCipher = WPA_CIPHER_MIXED;
			entry.unicastCipher     = WPA_CIPHER_MIXED;
			entry.wsc_enc           = WSC_ENCRYPT_TKIPAES;
		} else if (interface_mib.authentication_type == AUTH_WPA) {
			entry.encrypt       = ENCRYPT_WPA;
			entry.wsc_auth      = WSC_AUTH_WPAPSK;
			entry.wsc_enc       = WSC_ENCRYPT_TKIP;
		} else if (interface_mib.authentication_type == AUTH_WEP){
			entry.encrypt       = ENCRYPT_WEP;
			entry.wsc_auth      = WSC_AUTH_OPEN;
			entry.wsc_enc       = WSC_ENCRYPT_NONE;
		} else if (interface_mib.authentication_type == AUTH_DISABLED){
			entry.encrypt       = ENCRYPT_DISABLED;
			entry.wsc_auth      = WSC_AUTH_OPEN;
			entry.wsc_enc       = WSC_ENCRYPT_NONE;
		}
	}

#ifdef MULTI_AP_OPEN_ENCRYPT_SUPPORT
	if (interface_mib.authentication_type == 1){
		entry.encrypt       = ENCRYPT_DISABLED;
		entry.wsc_auth      = WSC_AUTH_OPEN;
	}
#endif

	if (AUTH_WEP == interface_mib.authentication_type){
		strlcpy((char *)entry.wep64Key1, interface_mib.network_key, sizeof(entry.wep64Key1));
		entry.wep               = 1;
		entry.wpa2UnicastCipher = 0;
		entry.unicastCipher     = 0;
		entry.wsc_enc           = WSC_ENCRYPT_WEP;
	} else {
		entry.wep               = 0;
		entry.wpaAuth           = WPA_AUTH_PSK;
		entry.wpaPSKFormat      = 0; //PSK_FORMAT_PASSPHRASE
		strlcpy((char *)entry.wpaPSK, interface_mib.network_key, sizeof(entry.wpaPSK));
		strlcpy((char *)entry.wscPsk, interface_mib.network_key, sizeof(entry.wscPsk));
	}
	/*** Customized auth type support end ***/

	entry.wsc_configured = 1;

	mib_chain_update(MIB_MBSSIB_TBL + radio_idx, &entry, vap_idx);

	snprintf(akm_path, sizeof(akm_path), "/tmp/dpp_akm_%s", interface_name);

	if (interface_mib.authentication_type & AUTH_DPP) {
		// printf("%s support dpp! connector: %s\n", interface_name, interface_mib.signed_connector);
		if (strncmp(interface_mib.signed_connector, "null", strlen(interface_mib.signed_connector))) {
			fd = fopen(connector_path, "w");
			if (fd) {
				fprintf(fd, "%s", interface_mib.signed_connector);
				fclose(fd);
			} else {
				printf("Failed to write signned connector for %s\n", interface_name);
			}
		} else {
			printf("%s is set to use DPP auth without signed connector!!\n", interface_name);
		}
		fd = fopen(akm_path, "w+");
		if (fd) {
			fprintf(fd, "dpp");
			fclose(fd);
		} else {
			printf("Failed to write akm for %s\n", interface_name);
		}
	} else {
		(void)remove(akm_path);
	}

	return 1;
}

uint8_t _set_vxd_mib(uint8_t radio_idx, struct easymesh_interface_mib interface_mib)
{
	// set vxd mib
	int vap_idx = interface_mib.interface_index;

	char buffer[100];

	uint8_t encrypt_type = 0;
	uint8_t wsc_auth_type = 0;
	uint8_t use_sha_256 = 0, use_11w = 0;

	MIB_CE_MBSSIB_T entry;

	if (radio_idx > 2) {
		// Only support wlan0, wlan1 and wlan2
		printf("Unknown radio index %d\n", radio_idx);
		return 1;
	}

#if IS_AX_SUPPORT
	vap_idx = interface_mib.interface_index;
#endif
	(void)mib_chain_get(MIB_MBSSIB_TBL + radio_idx, vap_idx, &entry);

	if (interface_mib.authentication_type & AUTH_WPA2) {
		encrypt_type |= WIFI_SEC_WPA2;
		wsc_auth_type |= WSC_AUTH_WPA2PSK;
	}

#ifdef WLAN_WPA3
	if (interface_mib.authentication_type & AUTH_WPA3) {
		encrypt_type |= WIFI_SEC_WPA3;
		wsc_auth_type |= WSC_AUTH_WPA3SAE;
		use_sha_256 = 1;
		use_11w = 2;
		if (interface_mib.authentication_type & AUTH_WPA2) {
			use_sha_256 = 0;
			use_11w = 1;
		}
	}
#endif

	if (0 == strcmp(interface_mib.ssid, (char *)entry.ssid)) {

		mib_get(MIB_REPEATER_SSID1 + radio_idx, (void *)buffer);

		if (0 == strcmp(interface_mib.ssid, buffer) && 0 == strcmp(interface_mib.ssid, (char *)entry.ssid)) {
			if (0 == strcmp(interface_mib.network_key, (char *)entry.wpaPSK) && entry.encrypt == encrypt_type) {
				printf("Setting is identical for interface wlan%d-vxd, skip configuration\n", radio_idx);
				return 2;
			}
		}
	}

	entry.wlanMode = CLIENT_MODE;

	strlcpy((char *)entry.ssid, interface_mib.ssid, sizeof(entry.ssid));

	mib_set(MIB_REPEATER_SSID1 + radio_idx, (void *)interface_mib.ssid);

	entry.multiap_bss_type = MULTI_AP_BACKHAUL_STA_BIT;

	if (!encrypt_type) {
		encrypt_type = WIFI_SEC_WPA2;
	}

	if (!wsc_auth_type) {
		wsc_auth_type = WSC_AUTH_WPA2PSK;
	}

#ifdef WLAN_11W
		entry.sha256        = use_sha_256;
		entry.dotIEEE80211W = use_11w;
#endif

	entry.encrypt           = encrypt_type;
	entry.wsc_auth          = wsc_auth_type;

#ifdef MULTI_AP_OPEN_ENCRYPT_SUPPORT
	if(interface_mib.authentication_type == 1){
		entry.encrypt       = ENCRYPT_DISABLED;
		entry.wsc_auth      = WSC_AUTH_OPEN;
	}
#endif

	entry.wpaAuth           = WPA_AUTH_PSK;

	entry.wpa2UnicastCipher = WPA_CIPHER_AES;
	entry.unicastCipher     = WPA_CIPHER_AES;
#ifdef WLAN_WPA3
	if(entry.encrypt & WIFI_SEC_WPA3) {
		entry.wpa2UnicastCipher = WPA_CIPHER_AES;
		entry.unicastCipher     = 0;
	}
#endif

	entry.wpaPSKFormat      = 0; //PSK_FORMAT_PASSPHRASE

	strlcpy((char *)entry.wpaPSK, interface_mib.network_key, sizeof(entry.wpaPSK));

	entry.wsc_enc  = WSC_ENCRYPT_AES;
	strlcpy((char *)entry.wscPsk, interface_mib.network_key, sizeof(entry.wscPsk));
	entry.wsc_configured = 1;


	mib_chain_update(MIB_MBSSIB_TBL + radio_idx, &entry, vap_idx);
	return 1;
}

uint8_t _easymesh_set_vendor_data_mib()
{
#ifdef EASYMESH_ISP_VENDOR_SYNC_SUPPORT
	struct easymesh_datamodel easymesh_db;
	easymesh_db.configured_band  = 0;
	easymesh_db.radio_data_nr    = 0;
	easymesh_db.radio_data       = NULL;
	easymesh_db.per_radio_config = NULL;

	if (ini_parse("/var/multiap.conf", read_config_file, &easymesh_db) < 0) {
		printf("[RTK] Can't load configuration file!! \n");
		return INIT_ERROR_CONFIG_FILE;
	}

	// if (ini_parse("/var/multiap_mib.conf", read_mib_config_file, &easymesh_db) < 0) {
	// 	printf("[RTK] Can't load configuration file!! \n");
	// 	return INIT_ERROR_CONFIG_FILE;
	// }

	if (ini_parse("/var/multiap_setmib.conf", read_set_mib_config_file, &easymesh_db) < 0) {
		printf("[RTK] Can't load configuration file!! \n");
		return INIT_ERROR_CONFIG_FILE;
	}

	// set configure_state
	if (!mib_set(MIB_MAP_CONFIGURED_BAND, (void *)&easymesh_db.configured_band)) {
		printf("[ERROR] Cannot set configured band.\n");
		return 0;
	}

	int i = 0, j = 0;
#if defined(WLAN_VAP_ACL) || defined(WLAN_SCHEDULE_SUPPORT)
	int k = 0;
#endif
	struct vendor_acl_info  vendor_acl_data          = {0};
	struct vendor_schedule_info vendor_schedule_data = {0};

	vendor_acl_data.acl_nr                           = 0;
	vendor_acl_data.acl_data                         = NULL;
	vendor_schedule_data.wlan_schedule_nr	         = 0;
	vendor_schedule_data.wlan_schedule_data          = NULL;

	for(j = 0; j < easymesh_db.config_data_nr; j++){
		if (easymesh_db.per_radio_config[j].vendor_data_nr) {
			for (i = 0; i < easymesh_db.per_radio_config[j].vendor_data_nr; i++) {
				unsigned char *buffer	= (unsigned char *)malloc(easymesh_db.per_radio_config[j].vendor_datas[i].payload_len);
				char             *pchar = NULL;
				memcpy(buffer, easymesh_db.per_radio_config[j].vendor_datas[i].payload, easymesh_db.per_radio_config[j].vendor_datas[i].payload_len);

				if ((pchar=strstr((char *)buffer, "[ACL]"))!=NULL){
					pchar  =  strchr((char *)buffer, ']');
					pchar  += 1;
					process_rtk_vendor_spec_data(pchar, &vendor_acl_data, &vendor_schedule_data, "ACL");
				} else if ((pchar=strstr((char *)buffer, "[WLAN_SCHEDULE]"))!=NULL){
					pchar  =  strchr((char *)buffer, ']');
					pchar  += 1;
					process_rtk_vendor_spec_data(pchar, &vendor_acl_data, &vendor_schedule_data, "WLAN_SCHEDULE");
				}

				if (buffer)
					free(buffer);
			}
			break;
		}
	}

	int need_reinit = 0;
#if defined(WLAN_VAP_ACL) || defined(WLAN_SCHEDULE_SUPPORT)
	int match = 0;
#endif
	struct vendor_acl_info      orig_vendor_acl_data      = {0};
	struct vendor_schedule_info orig_vendor_schedule_data = {0};

	orig_vendor_acl_data.acl_nr                           = 0;
	orig_vendor_acl_data.acl_data                         = NULL;
	orig_vendor_schedule_data.wlan_schedule_nr            = 0;
	orig_vendor_schedule_data.wlan_schedule_data          = NULL;

	if ( vendor_acl_data.acl_nr || vendor_schedule_data.wlan_schedule_nr )
		_get_orig_vendor_data(&orig_vendor_acl_data, &orig_vendor_schedule_data);

#ifdef WLAN_VAP_ACL
	if (vendor_acl_data.acl_nr) {
		if (vendor_acl_data.acl_nr == orig_vendor_acl_data.acl_nr) {
			for (i = 0; i < vendor_acl_data.acl_nr; i++) {
				match = 0;
				for (j = 0; j < orig_vendor_acl_data.acl_nr; j++) {
					if ( (vendor_acl_data.acl_data[i].wlan_Idx == orig_vendor_acl_data.acl_data[j].wlan_Idx) && 
						 (vendor_acl_data.acl_data[i].acl_enable == orig_vendor_acl_data.acl_data[j].acl_enable) &&
						 (!memcmp(vendor_acl_data.acl_data[i].macAddr, orig_vendor_acl_data.acl_data[j].macAddr, MAC_ADDR_LEN))
					) {
						match = 1; //get a match
						break;
					}
				}
				if (!match) {
					break;
				}
			}
		} else {
			match = 0;
		}

		if (!match) {
			need_reinit |= MAP_SYNC_WLAN_ACL_DATA;
			//del all acl entry
			_delall_wlan_acl_mib();

			//sync vendor_acl_info to agent
			for(k = 0; k < vendor_acl_data.acl_nr; k++){
				_set_wlan_acl_mib(vendor_acl_data.acl_data[k]);
			}
			mib_update(CURRENT_SETTING,CONFIG_MIB_ALL);
		}
	}
#endif

#ifdef WLAN_SCHEDULE_SUPPORT
	if (vendor_schedule_data.wlan_schedule_nr) {
		if (vendor_schedule_data.wlan_schedule_nr == orig_vendor_schedule_data.wlan_schedule_nr) {
			for (i = 0; i < vendor_schedule_data.wlan_schedule_nr; i++) {
				match = 0;
				for (j = 0; j < orig_vendor_schedule_data.wlan_schedule_nr; j++) {
					if ( (vendor_schedule_data.wlan_schedule_data[i].wlan_Idx == orig_vendor_schedule_data.wlan_schedule_data[j].wlan_Idx) && 
						 (vendor_schedule_data.wlan_schedule_data[i].enable == orig_vendor_schedule_data.wlan_schedule_data[j].enable) &&
						 (vendor_schedule_data.wlan_schedule_data[i].mode == orig_vendor_schedule_data.wlan_schedule_data[j].mode) &&
						 (vendor_schedule_data.wlan_schedule_data[i].eco == orig_vendor_schedule_data.wlan_schedule_data[j].eco) &&
						 (vendor_schedule_data.wlan_schedule_data[i].fTime == orig_vendor_schedule_data.wlan_schedule_data[j].fTime) &&
						 (vendor_schedule_data.wlan_schedule_data[i].tTime == orig_vendor_schedule_data.wlan_schedule_data[j].tTime) &&
						 (vendor_schedule_data.wlan_schedule_data[i].day == orig_vendor_schedule_data.wlan_schedule_data[j].day)
					) {
						match = 1; //get a match
						break;
					}
				}
				if (!match) {
					break;
				}
			}
		}else {
			match = 0;
		}

		if (!match) {
			need_reinit |= MAP_SYNC_WLAN_SCHEDULE_DATA;
			//del all wlan_schedule entry
			_delall_wlan_schedule_mib();

			//sync vendor_acl_info to agent
			for(k = 0; k < vendor_schedule_data.wlan_schedule_nr; k++){
				_set_wlan_schedule_mib(vendor_schedule_data.wlan_schedule_data[k]);
			}
			mib_update(CURRENT_SETTING,CONFIG_MIB_ALL);
		}
	}
#endif

	_free_vendor_acl_data(&vendor_acl_data);
	_free_vendor_acl_data(&orig_vendor_acl_data);
	_free_vendor_schedule_data(&vendor_schedule_data);
	_free_vendor_schedule_data(&orig_vendor_schedule_data);

	return need_reinit;
#else
	return 0;
#endif
}

uint8_t _easymesh_set_mib()
{
	int                       i, j;
	struct easymesh_datamodel easymesh_db = {0};
	easymesh_db.configured_band  = 0;
	easymesh_db.radio_data_nr    = 0;
	easymesh_db.radio_data       = NULL;
	easymesh_db.per_radio_config = NULL;

	int radio_idx = 0;

	FILE *fd = NULL;

	if (ini_parse("/var/multiap.conf", read_config_file, &easymesh_db) < 0) {
		printf("[RTK] Can't load configuration file!! \n");
		return INIT_ERROR_CONFIG_FILE;
	}

	// if (ini_parse("/var/multiap_mib.conf", read_mib_config_file, &easymesh_db) < 0) {
	// 	printf("[RTK] Can't load configuration file!! \n");
	// 	return INIT_ERROR_CONFIG_FILE;
	// }

	if (ini_parse("/var/multiap_setmib.conf", read_set_mib_config_file, &easymesh_db) < 0) {
		printf("[RTK] Can't load configuration file!! \n");
		return INIT_ERROR_CONFIG_FILE;
	}

	// set configure_state
	if (!mib_set(MIB_MAP_CONFIGURED_BAND, (void *)&easymesh_db.configured_band)) {
		printf("[ERROR] Cannot set configured band.\n");
		return 0;
	}

	(void)remove(DPP_CONNECTOR_PATH_1905);
	(void)remove(DPP_CSIGN_KEY_PATH);
	(void)remove(DPP_NET_ACCESS_KEY_PATH);
	(void)remove(DPP_PRIVACY_PROTECTION_KEY_PATH);

	if (easymesh_db.signed_connector_1905) {
		// printf("signed_connector_1905: %s\n", easymesh_db.signed_connector_1905);
		fd = fopen(DPP_CONNECTOR_PATH_1905, "w");
		if (fd) {
			fprintf(fd, "%s", easymesh_db.signed_connector_1905);
			fclose(fd);
		}
	}

	if (easymesh_db.csign_key) {
		// printf("csign: %s\n", easymesh_db.csign_key);
		fd = fopen(DPP_CSIGN_KEY_PATH, "w");
		if (fd) {
			fprintf(fd, "%s", easymesh_db.csign_key);
			fclose(fd);
		}
	}

	if (easymesh_db.netaccess_key) {
		// printf("netaccess_key: %s\n", easymesh_db.netaccess_key);
		fd = fopen(DPP_NET_ACCESS_KEY_PATH, "w");
		if (fd) {
			fprintf(fd, "%s", easymesh_db.netaccess_key);
			fclose(fd);
		}
	}

	if (easymesh_db.pp_key) {
		// printf("ppk: %s\n", easymesh_db.pp_key);
		fd = fopen(DPP_PRIVACY_PROTECTION_KEY_PATH, "w");
		if (fd) {
			fprintf(fd, "%s", easymesh_db.pp_key);
			fclose(fd);
		}
	}

	// set channel & bonding

	// set mib
	int radio_band, interface_index;

	char *radio_name = NULL;

	for (i = 0; i < easymesh_db.radio_data_nr; i++) {
		radio_band = easymesh_db.radio_data[i].radio_band;
		if (MAP_CONFIG_2G == radio_band) {
			radio_name = easymesh_db.radio_name_24g;
		} else if (MAP_CONFIG_5GL == radio_band) {
			radio_name = easymesh_db.radio_name_5gl;
		} else if (MAP_CONFIG_5GH == radio_band) {
			radio_name = easymesh_db.radio_name_5gh;
		} else {
			printf("Unsupported band %02x!\n", radio_band);
			continue;
		}

		if (radio_name != NULL) {
			radio_idx = radio_name[4] - '0';
		} else {
			printf("[luna_map_reinit][_easymesh_set_mib]radio_name is NULL!\n");
			continue;
		}

		if (easymesh_db.radio_data[i].need_change_channel) {
			_set_radio_band_width(radio_idx, easymesh_db.radio_data[i].radio_channel,  easymesh_db.radio_data[i].channel_bandwidth, easymesh_db.radio_data[i].control_sideband);
		}
		for (j = 0; j < easymesh_db.radio_data[i].interface_nr; j++) {
			interface_index = easymesh_db.radio_data[i].interface_mib[j].interface_index;
			if (NUM_VWLAN_INTERFACE == interface_index) {
#ifndef RTK_MULTI_AP_WFA
				if (1 == easymesh_db.radio_data[i].interface_mib[j].need_configure) {
					_set_vxd_mib(radio_idx, easymesh_db.radio_data[i].interface_mib[j]);
				}
#endif
			} else {
				if (1 == easymesh_db.radio_data[i].interface_mib[j].need_configure) {
					_set_ap_mib(radio_idx, easymesh_db.radio_data[i].interface_mib[j]);
				}
			}
		}
	}

	return 1;
}

int main(int argc, char *argv[])
{
	int c;
	int reinit = -1;
	int band = -1, cur_channel = -1, bandwidth = -1, sideband = -1;
	int configured_band = 0;

	while ((c = getopt(argc, argv, "rflvb:c:t")) != -1) {
		switch (c) {
		case 'f': {
			reinit = FULL_RELOAD;
			break;
		}
		case 'r': {
			reinit = MIB_RELOAD;
			break;
		}
		case 'l': {
			reinit = SOFT_RELOAD;
			break;
		}
		case 'v': {
			reinit = VENDOR_DATA_RELOAD;
			break;
		}
		case 'b': {
			char *token, *saveptr = NULL;
			int * band_str = NULL;
			int   str_num = 0, i = 0;
			for (token = (char *)optarg;; token = NULL) {
				token = strtok_r(token, "_", &saveptr);
				if (NULL == token)
					break;
				i++;
				band_str          = (int *)realloc(band_str, i * sizeof(int));
				band_str[str_num] = atoi(token);
				str_num += 1;
			}
			if( 2 == str_num) {
				band        = band_str[0];
				cur_channel = band_str[1];
			} else if (4 == str_num) {
				band        = band_str[0];
				cur_channel = band_str[1];
				bandwidth   = band_str[2];
				sideband    = band_str[3];
			} else {
				printf("wrong cmd for map_reinit daemon!!!!\n");
				free(band_str);
				return 0;
			}
			reinit = BAND_MIB_SET;
			free(band_str);
			break;
		}
		case 'c': {
			if (optarg) {
				configured_band = atoi(optarg);
			}
			reinit = CONFIG_STATE_SET;
			break;
		}
		case 't': {
			reinit = TEST_RELOAD;
			break;
		}
		}
	}

	if (TEST_RELOAD == reinit) {
		map_restart_wlan();
	} else if (FULL_RELOAD == reinit) {
		system("killall map_checker; echo 1 > /tmp/map_checker_end.txt");
		_easymesh_set_mib();
		_easymesh_set_vendor_data_mib();
		printf("\nNeed full reload!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		map_restart_wlan();
	} else if (SOFT_RELOAD == reinit) {
		system("killall map_checker; echo 1 > /tmp/map_checker_end.txt");
		_easymesh_set_mib();
		_easymesh_set_vendor_data_mib();
		printf("\nOnly need reload!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		map_restart_wlan();
	} else if (VENDOR_DATA_RELOAD == reinit) {
		int ret = 0;
		ret = _easymesh_set_vendor_data_mib();
		if (ret & MAP_SYNC_WLAN_ACL_DATA) {
			printf("\nNeed vendor data reload!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			system("killall map_checker; echo 1 > /tmp/map_checker_end.txt");
			char command[64];
			sprintf(command, "killall map_agent");
			system(command);
			map_restart_wlan();
		} else if (ret & MAP_SYNC_WLAN_SCHEDULE_DATA) {
#ifdef WLAN_SCHEDULE_SUPPORT
			printf("\nRestart wlan schedule only!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			restartWlanSchedule();
#endif
		} else {
			printf("\nAll vendor data received is identical to the original, skip re-init!!!!\n");
		}
	} else if (BAND_MIB_SET == reinit) {
		struct easymesh_datamodel easymesh_db = { 0 };
		char *                    radio_name = NULL;
		if (ini_parse("/var/multiap.conf", read_config_file, &easymesh_db) < 0) {
			printf("[RTK] Can't load configuration file!! \n");
			return INIT_ERROR_CONFIG_FILE;
		}
		if (MAP_CONFIG_2G == band) {
			radio_name = easymesh_db.radio_name_24g;
		} else if (MAP_CONFIG_5GL == band) {
			radio_name = easymesh_db.radio_name_5gl;
			sideband = 0; // Auto
		} else if (MAP_CONFIG_5GH == band) {
			radio_name = easymesh_db.radio_name_5gh;
			sideband = 0; // Auto
		} else {
			printf("Unsupported band %02x!\n", band);
			return 1;
		}
		int radio_idx = radio_name[4] - '0';
		_set_radio_band_width(radio_idx, cur_channel, bandwidth, sideband);
	} else if (CONFIG_STATE_SET == reinit) {
		_set_configure_state(configured_band);
	} else if (MIB_RELOAD == reinit) {
		struct mib_info mib_data = { 0 };
		if (map_read_mib_to_config(&mib_data)) {
			printf("[Error]Can't do completely reinit daemon due to reading wrong mib parameter\n");
			return 1;
		}
		map_write_mib_data(&mib_data, "/var/multiap_mib.conf");
		map_free_mib_data(&mib_data);
	}

	return 0;
}
