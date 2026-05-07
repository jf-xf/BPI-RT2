/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "config_file_handler.h"
#include "easymesh_datamodel.h"
#include "base64.h"

// #define MAP_CONFIG_DEBUG

#ifdef MAP_CONFIG_DEBUG
#define map_config_debug(msg, ...) printf((msg), ##__VA_ARGS__)
#else
#define map_config_debug(msg, ...) ((void)0)
#endif

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

static const uint8_t REALTEK_OUI[3] = {0x00, 0xE0, 0x4C};
#define SSID_BSS_INDEX_MAPPING 1
#define SUB_ELEMENT_TYPE_SIZE 1
#define SUB_ELEMENT_LENGTH_SIZE 2
#define SSID_LENGTH_SIZE 1
#define BSS_INDEX_LENGTH_SIZE 1

#define MAX_RADIO_CONFIG_NUM 3

/////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t _atoi(char *s, int base)
{
	uint8_t k    = 0;
	int     sign = 1;
	if (NULL == s) {
		return 0;
	}

	k = 0;
	if (base == 10) {
		if (*s == '-') {
			sign = -1;
			s++;
		}
		while (*s != '\0' && *s >= '0' && *s <= '9') {
			k = 10 * k + (*s - '0');
			s++;
		}
		k *= sign;
	} else {
		while (*s != '\0') {
			uint8_t v;
			if (*s >= '0' && *s <= '9')
				v = *s - '0';
			else if (*s >= 'a' && *s <= 'f')
				v = *s - 'a' + 10;
			else if (*s >= 'A' && *s <= 'F')
				v = *s - 'A' + 10;
			else {
				printf("error hex format!\n");
#if 1
				return k;
#else
				return 0;
#endif
			}
			k = 16 * k + v;
			s++;
		}
	}
	return k;
}

void _writeToConfig(FILE *fp, unsigned char config_nr, char **config_array)
{
	int i;
	for (i = 0; i < config_nr; i++) {
		fprintf(fp, "%s", config_array[i]);
		//free(config_array[i]);
		if (i < (config_nr - 1)) {
			fprintf(fp, ",");
		} else {
			fprintf(fp, "\n");
		}
	}
}

void _writeOuiToConfig(FILE *fp, unsigned char config_nr, unsigned char **config_array)
{
	int i;
	for (i = 0; i < config_nr; i++) {
		fprintf(fp, "%02x%02x%02x", config_array[i][0], config_array[i][1], config_array[i][2]);
		//free(config_array[i]);
		if (i < (config_nr - 1)) {
			fprintf(fp, ",");
		} else {
			fprintf(fp, "\n");
		}
	}
}

void _writeToConfig_dec(FILE *fp, unsigned char config_nr, unsigned char *config_array)
{
	int i;
	for (i = 0; i < config_nr; i++) {
		fprintf(fp, "%d", config_array[i]);
		if (i < (config_nr - 1)) {
			fprintf(fp, ",");
		} else {
			fprintf(fp, "\n");
		}
	}
}

void _writeToConfig_dec_short(FILE *fp, unsigned char config_nr, unsigned short *config_array)
{
	int i;
	for (i = 0; i < config_nr; i++) {
		fprintf(fp, "%d", config_array[i]);
		if (i < (config_nr - 1)) {
			fprintf(fp, ",");
		} else {
			fprintf(fp, "\n");
		}
	}
}

void _fill_config(const char *section, const char *name, const char *value, const char *section_name, struct radio_config_data *config)
{
	if (MATCH(section_name, "ssid")) {
		// // parse the interfaces and its role
		char *  token;
		char *  saveptr = NULL;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			size_t ssid_length = 0;
			if(config->bss_data[str_num].ssid) {
				free(config->bss_data[str_num].ssid);
			}
			config->bss_data[str_num].ssid = (char *)base64_decode((unsigned char *)token, strlen(token), &ssid_length);
			if(ssid_length < strlen(config->bss_data[str_num].ssid)) {
				config->bss_data[str_num].ssid[ssid_length] = 0;
			}
			map_config_debug("[CONFIG] %s ssid %d: %s\n", section_name, str_num, config->bss_data[str_num].ssid);
			str_num += 1;
		}
	} else if (MATCH(section_name, "ssid_raw")) {
		char *  token;
		char *  saveptr;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			if(config->bss_data[str_num].ssid) {
				free(config->bss_data[str_num].ssid);
			}
			config->bss_data[str_num].ssid = strdup(token);
			map_config_debug("[CONFIG] %s Raw ssid %d: %s\n", section_name, str_num, config->bss_data[str_num].ssid);
			str_num += 1;
		}
	} else if (MATCH(section_name, "network_key")) {
		char *str = (char *)value;
		char *  token;
		uint8_t str_num = 0;
		while ((token = strsep(&str, ",")) != NULL) {
			size_t key_length = 0;
			if(config->bss_data[str_num].network_key) {
				free(config->bss_data[str_num].network_key);
			}
			if (strlen(token) > 0) {
				config->bss_data[str_num].network_key = (char *)base64_decode((unsigned char *)token, strlen(token), &key_length);
			} else {
				config->bss_data[str_num].network_key = strdup("");
			}
			if(key_length < strlen(config->bss_data[str_num].network_key)) {
				config->bss_data[str_num].network_key[key_length] = 0;
			}
			map_config_debug("[CONFIG] %s Network key %d: %s\n", section_name, str_num, config->bss_data[str_num].network_key);
			str_num += 1;
		}
	} else if (MATCH(section_name, "network_key_raw")) {
		char *  token;
		char *  saveptr;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			if(config->bss_data[str_num].network_key) {
				free(config->bss_data[str_num].network_key);
			}
			config->bss_data[str_num].network_key = strdup(token);
			map_config_debug("[CONFIG] %s Raw Network key %d: %s\n", section_name, str_num, config->bss_data[str_num].network_key);
			str_num += 1;
		}
	} else if (MATCH(section_name, "network_type")) {
		char *  token;
		char *  saveptr = NULL;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			map_config_debug("[CONFIG] %s Network type %d: %s\n", section_name, str_num, token);
			config->bss_data[str_num].network_type = atoi(token);
			str_num += 1;
		}
	} else if (MATCH(section_name, "bss_index")) {
		char *  token;
		char *  saveptr = NULL;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			map_config_debug("[CONFIG] %s bss index %d: %s\n", section_name, str_num, token);
			config->bss_data[str_num].bss_index = atoi(token);
			str_num += 1;
		}
	} else if (MATCH(section_name, "authentication_type")) {
		char *  token;
		char *  saveptr = NULL;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			map_config_debug("[CONFIG] %s authentication type %d: %s\n", section_name, str_num, token);
			config->bss_data[str_num].authentication_type = atoi(token);
			str_num += 1;
		}
	} else if (MATCH(section_name, "encryption_type")) {
		char *  token;
		char *  saveptr = NULL;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			map_config_debug("[CONFIG] %s encryption type %d: %s\n", section_name, str_num, token);
			config->bss_data[str_num].encryption_type = atoi(token);
			str_num += 1;
		}
	} else if (MATCH(section_name, "al_id")) {
		char *  token;
		char *  saveptr = NULL;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			map_config_debug("[CONFIG] %s al_id %d: %s\n", section_name, str_num, token);
			sscanf(token, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", &config->bss_data[str_num].al_id[0], &config->bss_data[str_num].al_id[1], &config->bss_data[str_num].al_id[2], &config->bss_data[str_num].al_id[3], &config->bss_data[str_num].al_id[4], &config->bss_data[str_num].al_id[5]);
			str_num += 1;
		}
	}  else if (MATCH(section_name, "vid")) {
		char *  token;
		char *  saveptr = NULL;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			map_config_debug("[CONFIG] %s vid %d: %s\n", section_name, str_num, token);
			config->bss_data[str_num].vid = atoi(token);
			str_num += 1;
		}
	}
}

void _fill_vendor_config(const char *section, const char *name, const char *value, const char *section_name, struct radio_config_data *config)
{
	int i = 0;
	if (MATCH(section_name, "vendor_data_nr")) {
		config->vendor_data_nr = atoi(value);
		config->vendor_datas   = (struct vendor_specific_data *)malloc(config->vendor_data_nr * sizeof(struct vendor_specific_data));
		map_config_debug("[CONFIG] %s vendor data number is: %d\n", section_name, config->vendor_data_nr);
	} else {
		if (0 == config->vendor_data_nr || NULL == config->vendor_datas) {
			return;
		}

		if (MATCH(section_name, "vendor_payload")) {
			char *  token;
			char *  saveptr = NULL;
			uint8_t str_num = 0;
			for (token = (char *)value;; token = NULL) {
				token = strtok_r(token, ",", &saveptr);
				if (NULL == token)
					break;
				map_config_debug("[CONFIG] %s vendor config data payload %d: %s\n", section_name, str_num, token);
				config->vendor_datas[str_num].payload                                            = (uint8_t *)strdup(token);
				config->vendor_datas[str_num].payload_len                                        = (uint16_t)strlen(token);
				config->vendor_datas[str_num].payload[config->vendor_datas[str_num].payload_len] = '\0';
				config->vendor_datas[str_num].payload_len += 1;
				str_num += 1;
			}
		} else if (MATCH(section_name, "vendor_oui")) {
			char *  token;
			char *  saveptr = NULL;
			uint8_t str_num = 0;
			for (token = (char *)value;; token = NULL) {
				token = strtok_r(token, ",", &saveptr);
				if (NULL == token)
					break;

				for (i = 0; i < 3; i++) {
					char oui_B[3] = { '\0' };
					memcpy(oui_B, token + i * 2, 2);
					config->vendor_datas[str_num].vendor_oui[i] = _atoi(oui_B, 16);
				}
				map_config_debug("[CONFIG] %s vendor config data oui %d: %02x%02x%02x\n", section_name, str_num, config->vendor_datas[str_num].vendor_oui[0], config->vendor_datas[str_num].vendor_oui[1], config->vendor_datas[str_num].vendor_oui[2]);
				str_num += 1;
			}
		}
	}
}

void fill_bss_index_vendor_config(struct easymesh_datamodel *easymesh_database)
{
	int      i;
	int      j;
	int      vendor_data_idx;
	uint16_t sub_element_len;
	uint16_t payload_len;
	int      payload_idx;
	int      bss_index_sum = 0;
	struct vendor_specific_data *vendor_data_pt;
	for (i = 0; i < easymesh_database->config_data_nr; i++) {
		bss_index_sum   = 0;
		sub_element_len = 0;
		payload_idx     = 0;
		payload_len     = 0;
		for (j = 0; j < easymesh_database->per_radio_config[i].bss_data_nr; j++) {
			bss_index_sum += easymesh_database->per_radio_config[i].bss_data[j].bss_index;
		}
		if (!bss_index_sum) {
			continue;
		}
		if (easymesh_database->per_radio_config[i].vendor_data_nr) {
			vendor_data_idx = easymesh_database->per_radio_config[i].vendor_data_nr;
			easymesh_database->per_radio_config[i].vendor_data_nr += 1;
			easymesh_database->per_radio_config[i].vendor_datas = (struct vendor_specific_data *)realloc(easymesh_database->per_radio_config[i].vendor_datas, easymesh_database->per_radio_config[i].vendor_data_nr * sizeof(struct vendor_specific_data));
		} else {
			vendor_data_idx                                       = 0;
			easymesh_database->per_radio_config[i].vendor_data_nr = 1;
			easymesh_database->per_radio_config[i].vendor_datas   = (struct vendor_specific_data *)malloc(easymesh_database->per_radio_config[i].vendor_data_nr * sizeof(struct vendor_specific_data));
		}
		vendor_data_pt = &(easymesh_database->per_radio_config[i].vendor_datas[vendor_data_idx]);
		memcpy(vendor_data_pt->vendor_oui, REALTEK_OUI, 3);
		for (j = 0; j < easymesh_database->per_radio_config[i].bss_data_nr; j++) {
			sub_element_len += SSID_LENGTH_SIZE + BSS_INDEX_LENGTH_SIZE + strlen(easymesh_database->per_radio_config[i].bss_data[j].ssid) + 1;
		}
		payload_len += SUB_ELEMENT_TYPE_SIZE;
		payload_len += SUB_ELEMENT_LENGTH_SIZE;
		payload_len += sub_element_len;
		vendor_data_pt->payload_len = payload_len;
		vendor_data_pt->payload     = (uint8_t *)malloc(vendor_data_pt->payload_len * sizeof(uint8_t));
		//sub_element_id type (1)
		vendor_data_pt->payload[payload_idx] = SSID_BSS_INDEX_MAPPING;
		payload_idx += SUB_ELEMENT_TYPE_SIZE;
		//sub element length (2)
		vendor_data_pt->payload[payload_idx]     = (uint8_t)((sub_element_len & 0xff00) >> 8);
		vendor_data_pt->payload[payload_idx + 1] = (uint8_t)(sub_element_len & 0x00ff);
		payload_idx += SUB_ELEMENT_LENGTH_SIZE;
		for (j = 0; j < easymesh_database->per_radio_config[i].bss_data_nr; j++) {
			//ssid length (1)
			vendor_data_pt->payload[payload_idx] = strlen(easymesh_database->per_radio_config[i].bss_data[j].ssid) + 1;
			payload_idx += SSID_LENGTH_SIZE;
			//ssid (ssid_length)
			memcpy(&(vendor_data_pt->payload[payload_idx]), easymesh_database->per_radio_config[i].bss_data[j].ssid, 1 + strlen(easymesh_database->per_radio_config[i].bss_data[j].ssid));
			payload_idx += 1 + strlen(easymesh_database->per_radio_config[i].bss_data[j].ssid);
			//bss_index (1)
			vendor_data_pt->payload[payload_idx] = easymesh_database->per_radio_config[i].bss_data[j].bss_index;
			payload_idx += BSS_INDEX_LENGTH_SIZE;
		}
	}
}

void _fill_mib_info(const char *section, const char *name, const char *value, const char *section_name, struct easymesh_radio_mib *radio_mib)
{
	if (MATCH(section_name, "channel_bandwidth")) {
		radio_mib->channel_bandwidth = atoi(value);
		map_config_debug("[CONFIG] agent channel bandwidth is %d\n", radio_mib->channel_bandwidth);

	} else if (MATCH(section_name, "control_sideband")) {
		radio_mib->control_sideband = atoi(value);
		map_config_debug("[CONFIG] agent control sideband is %d\n", radio_mib->control_sideband);

	} else if (MATCH(section_name, "bss_number")) {
		radio_mib->interface_nr = atoi(value);
		map_config_debug("[CONFIG] bss_data number : %d\n", atoi(value));
		radio_mib->interface_mib = (struct easymesh_interface_mib *)malloc(radio_mib->interface_nr * sizeof(struct easymesh_interface_mib));
		memset(radio_mib->interface_mib, 0 , radio_mib->interface_nr * sizeof(struct easymesh_interface_mib));

	} else if (MATCH(section_name, "repeater_ssid")) {
		size_t ssid_length = 0;
		radio_mib->repeater_ssid = (char *)base64_decode((unsigned char *)value, strlen(value), &ssid_length);
		if(ssid_length < strlen(radio_mib->repeater_ssid)) {
			radio_mib->repeater_ssid[ssid_length] = 0;
		}
		map_config_debug("[CONFIG] repeater_ssid : %s\n", value);
	} else if (MATCH(section_name, "ssid")) {
		// // parse the interfaces and its role
		char *  token;
		char *  saveptr = NULL;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			size_t ssid_length = 0;
			radio_mib->interface_mib[str_num].ssid = (char *)base64_decode((unsigned char *)token, strlen(token), &ssid_length);
			if(ssid_length < strlen(radio_mib->interface_mib[str_num].ssid)) {
				radio_mib->interface_mib[str_num].ssid[ssid_length] = 0;
			}
			map_config_debug("[CONFIG] ssid %d: %s\n", str_num, radio_mib->interface_mib[str_num].ssid);
			str_num += 1;
		}
	} else if (MATCH(section_name, "network_key")) {
		char *str = (char *)value;
		char *  token;
		uint8_t str_num = 0;
		while ((token = strsep(&str, ",")) != NULL) {
			size_t key_length = 0;
			if (strlen(token) > 0) {
				radio_mib->interface_mib[str_num].network_key = (char *)base64_decode((unsigned char *)token, strlen(token), &key_length);
			} else {
				radio_mib->interface_mib[str_num].network_key = strdup("");
			}
			if(key_length < strlen(radio_mib->interface_mib[str_num].network_key)) {
				radio_mib->interface_mib[str_num].network_key[key_length] = 0;
			}
			map_config_debug("[CONFIG] Network key %d: %s\n", str_num, radio_mib->interface_mib[str_num].network_key);
			str_num += 1;
		}
	} else if (MATCH(section_name, "network_type")) {
		char *  token;
		char *  saveptr;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			radio_mib->interface_mib[str_num].network_type = atoi(token);
			map_config_debug("[CONFIG] network_type %d: %d\n", str_num, radio_mib->interface_mib[str_num].network_type);
			str_num += 1;
		}
	} else if (MATCH(section_name, "is_enabled")) {
		char *  token;
		char *  saveptr = NULL;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			radio_mib->interface_mib[str_num].is_enabled = atoi(token);
			map_config_debug("[CONFIG] status %d: %d\n", str_num, radio_mib->interface_mib[str_num].is_enabled);
			str_num += 1;
		}
	} else if (MATCH(section_name, "bss_index")) {
		char *  token;
		char *  saveptr = NULL;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			radio_mib->interface_mib[str_num].bss_index = atoi(token);
			map_config_debug("[CONFIG] bss_index %d: %d\n", str_num, radio_mib->interface_mib[str_num].bss_index);
			str_num += 1;
		}
	} else if (MATCH(section_name, "encrypt_type")) {
		char *  token;
		char *  saveptr = NULL;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			radio_mib->interface_mib[str_num].encrypt_type = atoi(token);
			map_config_debug("[CONFIG] encrypt_type %d: %d\n", str_num, radio_mib->interface_mib[str_num].encrypt_type);
			str_num += 1;
		}
	} else if (MATCH(section_name, "authentication_type")) {
		char *  token;
		char *  saveptr = NULL;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			radio_mib->interface_mib[str_num].authentication_type = atoi(token);
			map_config_debug("[CONFIG] authentication type %d: %d\n", str_num, radio_mib->interface_mib[str_num].authentication_type);
			str_num += 1;
		}
	}
}

void _fill_set_mib_config(const char *section, const char *name, const char *value, const char *section_name, struct easymesh_radio_mib *radio_mib)
{
	int i = 0;
	if (MATCH(section_name, "channel_changed")) {
		radio_mib->need_change_channel = atoi(value);
	} else if (MATCH(section_name, "channel_bandwidth")) {
		radio_mib->channel_bandwidth = atoi(value);
	} else if (MATCH(section_name, "control_sideband")) {
		radio_mib->control_sideband = atoi(value);
	} else if (MATCH(section_name, "number")) {
		radio_mib->interface_nr = atoi(value);
		map_config_debug("[CONFIG] config_data number : %d\n", atoi(value));
		radio_mib->interface_mib = (struct easymesh_interface_mib *)malloc(radio_mib->interface_nr * sizeof(struct easymesh_interface_mib));
		memset(radio_mib->interface_mib, 0, radio_mib->interface_nr * sizeof(struct easymesh_interface_mib));
		for (i = 0; i < radio_mib->interface_nr; i++) {
			radio_mib->interface_mib[i].interface_index = i;
		}
	} else if (MATCH(section_name, "ssid")) {
		char *  token;
		char *  saveptr = NULL;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			size_t ssid_length = 0;
			radio_mib->interface_mib[str_num].ssid = (char *)base64_decode((unsigned char *)token, strlen(token), &ssid_length);
			if(ssid_length < strlen(radio_mib->interface_mib[str_num].ssid)) {
				radio_mib->interface_mib[str_num].ssid[ssid_length] = 0;
			}
			map_config_debug("[CONFIG] ssid %d: %s\n", str_num, radio_mib->interface_mib[str_num].ssid);
			str_num += 1;
		}
	} else if (MATCH(section_name, "network_key")) {
		char *str = (char *)value;
		char *  token;
		uint8_t str_num = 0;
		while ((token = strsep(&str, ",")) != NULL) {
			size_t key_length = 0;
			if (strlen(token) > 0) {
				radio_mib->interface_mib[str_num].network_key = (char *)base64_decode((unsigned char *)token, strlen(token), &key_length);
			} else {
				radio_mib->interface_mib[str_num].network_key = strdup("");
			}
			if(key_length < strlen(radio_mib->interface_mib[str_num].network_key)) {
				radio_mib->interface_mib[str_num].network_key[key_length] = 0;
			}
			map_config_debug("[CONFIG] Network key %d: %s\n", str_num, radio_mib->interface_mib[str_num].network_key);
			str_num += 1;
		}
	} else if (MATCH(section_name, "is_enabled")) {
		char *  token;
		char *  saveptr = NULL;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			radio_mib->interface_mib[str_num].is_enabled = atoi(token);
			map_config_debug("[CONFIG] is_enabled %d: %d\n", str_num, radio_mib->interface_mib[str_num].is_enabled);
			str_num += 1;
		}
	} else if (MATCH(section_name, "bss_index")) {
		char *  token;
		char *  saveptr = NULL;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			radio_mib->interface_mib[str_num].bss_index = atoi(token);
			map_config_debug("[CONFIG] bss_index %d: %d\n", str_num, radio_mib->interface_mib[str_num].bss_index);
			str_num += 1;
		}
	} else if (MATCH(section_name, "need_configure")) {
		char *  token;
		char *  saveptr = NULL;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			radio_mib->interface_mib[str_num].need_configure = atoi(token);
			map_config_debug("[CONFIG] need_configure %d: %d\n", str_num, radio_mib->interface_mib[str_num].need_configure);
			str_num += 1;
		}
	} else if (MATCH(section_name, "bss_type")) {
		char *  token;
		char *  saveptr = NULL;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			radio_mib->interface_mib[str_num].network_type = atoi(token);
			map_config_debug("[CONFIG] bss_type %d: %d\n", str_num, radio_mib->interface_mib[str_num].network_type);
			str_num += 1;
		}
	} else if (MATCH(section_name, "encrypt_type")) {
		char *  token;
		char *  saveptr = NULL;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			radio_mib->interface_mib[str_num].encrypt_type = atoi(token);
			map_config_debug("[CONFIG] encrypt_type %d: %d\n", str_num, radio_mib->interface_mib[str_num].encrypt_type);
			str_num += 1;
		}
	} else if (MATCH(section_name, "authentication_type")) {
		char *  token;
		char *  saveptr = NULL;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			radio_mib->interface_mib[str_num].authentication_type = atoi(token);
			map_config_debug("[CONFIG] authentication type %d: %d\n", str_num, radio_mib->interface_mib[str_num].authentication_type);
			str_num += 1;
		}
	} else if (MATCH(section_name, "signed_connector")) {
		char *  token;
		char *  saveptr = NULL;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			radio_mib->interface_mib[str_num].signed_connector = strdup(token);
			map_config_debug("[CONFIG] signed_connector %d: %s\n", str_num, radio_mib->interface_mib[str_num].signed_connector);
			str_num += 1;
		}
	}
}

void _fill_setmib_vendor_data(const char *section, const char *name, const char *value, const char *section_name, struct radio_config_data *vendor_config)
{
	int i = 0;
	if (MATCH(section_name, "vendor_data_nr")) {
		vendor_config->vendor_data_nr = atoi(value);
		vendor_config->vendor_datas   = (struct vendor_specific_data *)malloc(vendor_config->vendor_data_nr * sizeof(struct vendor_specific_data));
		map_config_debug("[CONFIG] vendor setmib data number is: %d\n", vendor_config->vendor_data_nr);
	} else {
		if(0 == vendor_config->vendor_data_nr || NULL == vendor_config->vendor_datas) {
			return;
		}

		if (MATCH(section_name, "vendor_load_len")) {
			char *  token;
			char *  saveptr = NULL;
			uint8_t str_num = 0;
			for (token = (char *)value;; token = NULL) {
				token = strtok_r(token, ",", &saveptr);
				if (NULL == token)
					break;
				vendor_config->vendor_datas[str_num].payload_len = atoi(token);
				map_config_debug("[CONFIG] vendor payload data len is: %d\n", vendor_config->vendor_datas[str_num].payload_len);
				str_num += 1;
			}
		} else if (MATCH(section_name, "vendor_payload")) {
			char *  token;
			char *  saveptr = NULL;
			uint8_t str_num = 0;
			for (token = (char *)value;; token = NULL) {
				token = strtok_r(token, ",", &saveptr);
				if (NULL == token)
					break;
				vendor_config->vendor_datas[str_num].payload                                                   = (uint8_t *)strdup(token);
				vendor_config->vendor_datas[str_num].payload[vendor_config->vendor_datas[str_num].payload_len] = '\0';
				map_config_debug("[CONFIG] vendor payload data %d: %s\n", str_num, vendor_config->vendor_datas[str_num].payload);
				str_num += 1;
			}
		} else if (MATCH(section_name, "vendor_oui")) {
			char *  token;
			char *  saveptr = NULL;
			uint8_t str_num = 0;
			for (token = (char *)value;; token = NULL) {
				token = strtok_r(token, ",", &saveptr);
				if (NULL == token)
					break;

				for (i = 0; i < 3; i++) {
					char oui_B[3] = { '\0' };
					memcpy(oui_B, token + i * 2, 2);
					vendor_config->vendor_datas[str_num].vendor_oui[i] = _atoi(oui_B, 16);
				}
				map_config_debug("[CONFIG] vendor config data oui %d: %02x%02x%02x\n", str_num, vendor_config->vendor_datas[str_num].vendor_oui[0], vendor_config->vendor_datas[str_num].vendor_oui[1], vendor_config->vendor_datas[str_num].vendor_oui[2]);
				str_num += 1;
			}
		}
	}
}

int read_config_file(void *user, const char *section, const char *name,
                     const char *value)
{
	struct easymesh_datamodel *      pconfig     = (struct easymesh_datamodel *)user;
	static struct radio_config_data *_5gl_config = NULL;
	static struct radio_config_data *_5gh_config = NULL;
	static struct radio_config_data *_2g_config  = NULL;
	int i = 0;

	if (!pconfig->per_radio_config) {
		pconfig->per_radio_config = (struct radio_config_data *)malloc(MAX_RADIO_CONFIG_NUM * sizeof(struct radio_config_data));
		memset(pconfig->per_radio_config, 0 , MAX_RADIO_CONFIG_NUM * sizeof(struct radio_config_data));
	}

	if (MATCH("global", "alme_port_number")) {
		pconfig->alme_port_number = atoi(value);
		map_config_debug("[CONFIG] Multi-AP alme port: %d\n", pconfig->alme_port_number);
	} else if (MATCH("global", "max_log_size")) {
		pconfig->max_log_size = atoi(value);
		map_config_debug("[CONFIG] Max log size is %u\n", pconfig->max_log_size);
	} else if (MATCH("global", "bss_list_update")) {
		pconfig->bss_list_update = atoi(value);
		map_config_debug("[CONFIG]  bss_list_update : %d\n", pconfig->bss_list_update);
	} else if (MATCH("global", "max_resend_time")) {
		pconfig->max_resend_time = atoi(value);
		map_config_debug("[CONFIG] Max resend time for the message that doesn't acknowledged in time is %d\n", pconfig->max_resend_time);
	} else if (MATCH("global", "rssi_weightage")) {
		pconfig->rssi_weightage = atoi(value);
		map_config_debug("[CONFIG] Rssi weightage is %d\n", pconfig->rssi_weightage);
	} else if (MATCH("global", "path_weightage")) {
		pconfig->path_weightage = atoi(value);
		map_config_debug("[CONFIG] Path weightage is %d\n", pconfig->path_weightage);
	} else if (MATCH("global", "cu_weightage")) {
		pconfig->cu_weightage = atoi(value);
		map_config_debug("[CONFIG] Cu weightage is %d\n", pconfig->cu_weightage);
	} else if (MATCH("global", "roam_score_difference")) {
		pconfig->roam_score_difference = atoi(value);
		map_config_debug("[CONFIG] Roam score difference is %d\n", pconfig->roam_score_difference);
	} else if (MATCH("global", "min_evaluation_interval")) {
		pconfig->min_evaluation_interval = atoi(value);
		map_config_debug("[CONFIG] Min evaluation interval is %d\n", pconfig->min_evaluation_interval);
	} else if (MATCH("global", "min_roam_interval")) {
		pconfig->min_roam_interval = atoi(value);
		map_config_debug("[CONFIG] Min roam interval is %d\n", pconfig->min_roam_interval);
	} else if (MATCH("global", "controller_interface")) {
		pconfig->controller_interface = strdup(value);
		map_config_debug("[CONFIG] Controller interface set by user is %s\n", pconfig->controller_interface);
	} else if (MATCH("global", "controller_al_interface")) {
		pconfig->controller_al_interface = strdup(value);
		map_config_debug("[CONFIG] Controller al interface set by user is %s\n", pconfig->controller_al_interface);
	} else if (MATCH("global", "device_name")) {
		pconfig->device_name = strdup(value);
		map_config_debug("[CONFIG] Device name set by user is %s\n", pconfig->device_name);
	} else if (MATCH("global", "max_num_device_allowed")) {
		pconfig->max_num_device_allowed = atoi(value);
		map_config_debug("[CONFIG] Max number of device allowed is %d\n", pconfig->max_num_device_allowed);
	} else if (MATCH("global", "map_supported_service")) {
		pconfig->map_supported_service = atoi(value);
		printf("[CONFIG] map_supported_service is %d\n", pconfig->map_supported_service);
	} else if (MATCH("global", "throughput_threshold")) {
		pconfig->throughput_threshold = atoi(value);
		map_config_debug("[CONFIG] Throughput threshold is %u\n", pconfig->throughput_threshold);
	} else if (MATCH("global", "max_bss_per_radio")) {
		pconfig->max_bss_per_radio = atoi(value);
		map_config_debug("[CONFIG] Max bss number per radio is %d\n", pconfig->max_bss_per_radio);
	}else if (MATCH("global", "rssi_5g_threshold")) {
		pconfig->rssi_5g_threshold = atoi(value);
		map_config_debug("[CONFIG] rssi_5g_threshold is %d\n", pconfig->rssi_5g_threshold);
	}else if (MATCH("global", "rssi_24g_threshold")) {
		pconfig->rssi_24g_threshold = atoi(value);
		map_config_debug("[CONFIG] rssi_24g_threshold is %d\n", pconfig->rssi_24g_threshold);
	} else if (MATCH("global", "max_renew_resend_time")) {
		pconfig->max_renew_resend_time = atoi(value);
		map_config_debug("[CONFIG] Throughput threshold is %u\n", pconfig->max_renew_resend_time);
	} else if (MATCH("global", "radio_name_5gh")) {
		pconfig->radio_name_5gh = strdup(value);
		map_config_debug("[CONFIG] 5GH radio name is: %s\n", pconfig->radio_name_5gh);
	} else if (MATCH("global", "radio_name_5gl")) {
		pconfig->radio_name_5gl = strdup(value);
		map_config_debug("[CONFIG] 5GL radio name is: %s\n", pconfig->radio_name_5gl);
	} else if (MATCH("global", "radio_name_24g")) {
		pconfig->radio_name_24g = strdup(value);
		map_config_debug("[CONFIG] 24G radio name is: %s\n", pconfig->radio_name_24g);
	} else if (MATCH("global", "vap_prefix")) {
		pconfig->vap_prefix = strdup(value);
		map_config_debug("[CONFIG] VAP Prefix: %s\n", pconfig->vap_prefix);
	} else if (MATCH("global", "bridge_name")) {
		pconfig->bridge_name = strdup(value);
		map_config_debug("[CONFIG] Bridge Name: %s\n", pconfig->bridge_name);
	} else if (MATCH("global", "excluded_interfaces")) {
		int excluded_interfaces_len = strlen(value);
		pconfig->excluded_interfaces = malloc((excluded_interfaces_len + 2) * sizeof(char));
		memcpy(pconfig->excluded_interfaces, value, excluded_interfaces_len);
		pconfig->excluded_interfaces[excluded_interfaces_len] = ',';
		pconfig->excluded_interfaces[excluded_interfaces_len + 1] = '\0';
		map_config_debug("[CONFIG] Excluded interfaces are %s\n", pconfig->excluded_interfaces);
	} else if (MATCH("global", "eth_only")) {
		pconfig->eth_only = atoi(value);
		map_config_debug("[CONFIG] Eth Only: %d\n", pconfig->eth_only);
	} else if (MATCH("global", "common_netlink")) {
		pconfig->common_netlink = atoi(value);
		map_config_debug("[CONFIG] Common netlink: %d\n", pconfig->common_netlink);
	} else if (MATCH("global", "stp_monitor_enable")) {
		pconfig->stp_monitor_enable = atoi(value);
		map_config_debug("[CONFIG]stp_monitor_enable: %d\n", pconfig->stp_monitor_enable);
	} else if (MATCH("global", "loop_detect_enable")) {
		pconfig->loop_detect_enable = atoi(value);
		map_config_debug("[CONFIG]loop_detect_enable: %d\n", pconfig->loop_detect_enable);
	} else if (MATCH("vlan", "primary_vid")) {
		pconfig->primary_vid = atoi(value);
		map_config_debug("[CONFIG] primary_vid is %d\n", pconfig->primary_vid);
	} else if (MATCH("vlan", "default_pcp")) {
		pconfig->default_pcp = atoi(value);
		map_config_debug("[CONFIG] default_pcp is %d\n", pconfig->default_pcp);
	} else if (MATCH("global", "beacon_request_ch")) {
		pconfig->beacon_request_ch = atoi(value);
		map_config_debug("[CONFIG] Beacon Request Channel: %d\n", pconfig->beacon_request_ch);
	} else if (MATCH("global", "backhaul_bss_index")) {
		pconfig->backhaul_bss_index = atoi(value);
		map_config_debug("[CONFIG] Backhaul Bss Index: %d\n", pconfig->backhaul_bss_index);
	} else if (MATCH("global", "steering_opportunity_enable")) {
		pconfig->steering_opportunity_enable = atoi(value);
		map_config_debug("[CONFIG] Steering Opportunity Enable: %d\n", pconfig->steering_opportunity_enable);
	} else if (MATCH("global", "mcs_20m_table")) {
		char *  token;
		char *  saveptr = NULL;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			pconfig->mcs_20m_table[str_num] = atoi(token);
			map_config_debug("[CONFIG] mcs_20m_table %d: %d\n", str_num, pconfig->mcs_20m_table[str_num]);
			str_num += 1;
		}
	} else if (MATCH("global", "mcs_40m_table")) {
		char *  token;
		char *  saveptr = NULL;
		uint8_t str_num = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			pconfig->mcs_40m_table[str_num] = atoi(token);
			map_config_debug("[CONFIG] mcs_40m_table %d: %d\n", str_num, pconfig->mcs_40m_table[str_num]);
			str_num += 1;
		}
	} else if (MATCH("global", "max_sta_number")) {
		pconfig->max_sta_number = atoi(value);
		map_config_debug("[CONFIG] max_sta_number: %d\n", pconfig->max_sta_number);
	} else if (MATCH("5gl_config_data", "number")) {
		pconfig->config_data_nr += 1;
		// pconfig->per_radio_config = (struct radio_config_data *)realloc(pconfig->per_radio_config, pconfig->config_data_nr * sizeof(struct radio_config_data));
		map_config_debug("[CONFIG] 5gl_config_data number : %d\n", atoi(value));
		_5gl_config                 = &(pconfig->per_radio_config[pconfig->config_data_nr - 1]);
		_5gl_config->bss_data_nr    = atoi(value);
		_5gl_config->radio_band     = EASYMESH_RADIO_5GL;
		_5gl_config->vendor_data_nr = 0;
		_5gl_config->vendor_datas   = NULL;
		_5gl_config->bss_data       = (struct bss_config_data *)malloc(_5gl_config->bss_data_nr * sizeof(struct bss_config_data));
		for (i = 0; i < _5gl_config->bss_data_nr; i++) {
			_5gl_config->bss_data[i].ssid        = NULL;
			_5gl_config->bss_data[i].network_key = NULL;
			_5gl_config->bss_data[i].vid         = 0;
			memset(_5gl_config->bss_data[i].al_id, 0, 6);
		}
	} else if (MATCH("5gh_config_data", "number")) {
		pconfig->config_data_nr += 1;
		// pconfig->per_radio_config = (struct radio_config_data *)realloc(pconfig->per_radio_config, pconfig->config_data_nr * sizeof(struct radio_config_data));
		map_config_debug("[CONFIG] 5gh_config_data number : %d\n", atoi(value));
		_5gh_config                 = &(pconfig->per_radio_config[pconfig->config_data_nr - 1]);
		_5gh_config->bss_data_nr    = atoi(value);
		_5gh_config->radio_band     = EASYMESH_RADIO_5GH;
		_5gh_config->vendor_data_nr = 0;
		_5gh_config->vendor_datas   = NULL;
		_5gh_config->bss_data       = (struct bss_config_data *)malloc(_5gh_config->bss_data_nr * sizeof(struct bss_config_data));
		for (i = 0; i < _5gh_config->bss_data_nr; i++) {
			_5gh_config->bss_data[i].ssid        = NULL;
			_5gh_config->bss_data[i].network_key = NULL;
			_5gh_config->bss_data[i].vid         = 0;
			memset(_5gh_config->bss_data[i].al_id, 0, 6);
		}
	} else if (MATCH("2.4g_config_data", "number")) {
		pconfig->config_data_nr += 1;
		// pconfig->per_radio_config = (struct radio_config_data *)realloc(pconfig->per_radio_config, (pconfig->config_data_nr) * sizeof(struct radio_config_data));
		_2g_config                = &(pconfig->per_radio_config[pconfig->config_data_nr - 1]);
		_2g_config->bss_data_nr   = atoi(value);
		_2g_config->radio_band    = EASYMESH_RADIO_2G;
		map_config_debug("[CONFIG] 24g_config_data number : %d\n", atoi(value));
		_2g_config->vendor_data_nr = 0;
		_2g_config->vendor_datas   = NULL;
		_2g_config->bss_data       = (struct bss_config_data *)malloc(_2g_config->bss_data_nr * sizeof(struct bss_config_data));
		for (i = 0; i < _2g_config->bss_data_nr; i++) {
			_2g_config->bss_data[i].ssid        = NULL;
			_2g_config->bss_data[i].network_key = NULL;
			_2g_config->bss_data[i].vid         = 0;
			memset(_2g_config->bss_data[i].al_id, 0, 6);
		}
	} else if (MATCH("vlan_detail", "vid")) {
		char *  token;
		char *  saveptr;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			pconfig->vlan_id       = (uint16_t *)realloc(pconfig->vlan_id, (pconfig->vpair_number + 1) * sizeof(uint16_t));
			pconfig->vlan_id[pconfig->vpair_number] = atoi(token);
			map_config_debug("[CONFIG] vlan_id %d : %d\n", pconfig->vpair_number, pconfig->vlan_id[pconfig->vpair_number]);
			pconfig->vpair_number ++;
		}
	} else if (MATCH("vlan_detail", "ssid")) {
		char *  token;
		char *  saveptr = NULL;
		int     ssid_number = 0;
		for (token = (char *)value;; token = NULL) {
			token = strtok_r(token, ",", &saveptr);
			if (NULL == token)
				break;
			pconfig->vssid = (char **)realloc(pconfig->vssid, (ssid_number + 1) * sizeof(char *));
			size_t ssid_length = 0;
			pconfig->vssid[ssid_number] = (char *)base64_decode((unsigned char *)token, strlen(token), &ssid_length);
			if(ssid_length < strlen(pconfig->vssid[ssid_number])) {
				pconfig->vssid[ssid_number][ssid_length] = 0;
			}
			map_config_debug("[CONFIG] vssid %d : %s\n", ssid_number, pconfig->vssid[ssid_number]);
			ssid_number ++;
		}
	} else if (MATCH("global", "interface_manufacturer_name")){
		pconfig->interface_manufacturer_name = strdup(value);
		map_config_debug("[CONFIG] Interface Manufacturer name set by user is: %s\n", pconfig->interface_manufacturer_name);
	} else if (MATCH("global", "interface_model_name")){
		pconfig->interface_model_name = strdup(value);
		map_config_debug("[CONFIG] Interface Model name set by user is: %s\n", pconfig->interface_model_name);
	}  else if (MATCH("global", "interface_device_name")) {
		pconfig->interface_device_name = strdup(value);
		map_config_debug("[CONFIG] Interface Device name set by user is %s\n", pconfig->interface_device_name);
	} else if (MATCH("dpp", "signed_connector")) {
		pconfig->signed_connector_1905 = strdup(value);
		map_config_debug("[CONFIG] signed_connector_1905 set by user is %s\n", pconfig->signed_connector_1905);
	} else if (MATCH("dpp", "netaccess_key")) {
		pconfig->netaccess_key = strdup(value);
		map_config_debug("[CONFIG] netaccess_key set by user is %s\n", pconfig->netaccess_key);
	} else if (MATCH("dpp", "csign_key")) {
		pconfig->csign_key = strdup(value);
		map_config_debug("[CONFIG] csign_key set by user is %s\n", pconfig->csign_key);
	} else if (MATCH("dpp", "pp_key")) {
		pconfig->pp_key = strdup(value);
		map_config_debug("[CONFIG] pp_key set by user is %s\n", pconfig->pp_key);
	}

	if (_5gl_config) {
		_fill_config(section, name, value, "5gl_config_data", _5gl_config);
		_fill_vendor_config(section, name, value, "vendor_data_5gl", _5gl_config);
	}

	if (_5gh_config) {
		_fill_config(section, name, value, "5gh_config_data", _5gh_config);
		_fill_vendor_config(section, name, value, "vendor_data_5gh", _5gh_config);
	}

	if (_2g_config) {
		_fill_config(section, name, value, "2.4g_config_data", _2g_config);
		_fill_vendor_config(section, name, value, "vendor_data_2.4g", _2g_config);
	}

	return 1;
}

int read_set_mib_config_file(void *user, const char *section, const char *name, const char *value)
{
	struct easymesh_datamodel *       pconfig          = (struct easymesh_datamodel *)user;
	static struct easymesh_radio_mib *_5gl_config      = NULL;
	static struct easymesh_radio_mib *_5gh_config      = NULL;
	static struct easymesh_radio_mib *_2g_config       = NULL;
	static struct radio_config_data * _5gl_vendor_data = NULL;
	static struct radio_config_data * _5gh_vendor_data = NULL;
	static struct radio_config_data * _2g_vendor_data  = NULL;

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	if (MATCH("setmib_global_data", "configured_band")) {
		pconfig->configured_band = atoi(value);
		map_config_debug("[CONFIG] Configured band is %d\n", pconfig->configured_band);
	} else if (MATCH("setmib_global_data", "hw_reg_domain")) {
		pconfig->hw_reg_domain = atoi(value);
		map_config_debug("[CONFIG] HW redion domain is %d\n", pconfig->hw_reg_domain);
	} else if (MATCH("5gl_setmib_config_data", "radio_channel")) {
		pconfig->radio_data_nr += 1;
		pconfig->config_data_nr += 1;
		pconfig->radio_data              = (struct easymesh_radio_mib *)realloc(pconfig->radio_data, pconfig->radio_data_nr * sizeof(struct easymesh_radio_mib));
		pconfig->per_radio_config        = (struct radio_config_data *)realloc(pconfig->per_radio_config, pconfig->config_data_nr * sizeof(struct radio_config_data));
		_5gl_config                      = &(pconfig->radio_data[pconfig->radio_data_nr - 1]);
		_5gl_vendor_data                 = &(pconfig->per_radio_config[pconfig->config_data_nr - 1]);
		_5gl_vendor_data->vendor_data_nr = 0;
		_5gl_vendor_data->vendor_datas   = NULL;
		_5gl_config->radio_channel       = atoi(value);
		_5gl_config->radio_band          = EASYMESH_RADIO_5GL;
		map_config_debug("[CONFIG] agent 5gl channel is %d\n", _5gl_config->radio_channel);
	} else if (MATCH("5gh_setmib_config_data", "radio_channel")) {
		pconfig->radio_data_nr += 1;
		pconfig->config_data_nr += 1;
		pconfig->radio_data              = (struct easymesh_radio_mib *)realloc(pconfig->radio_data, pconfig->radio_data_nr * sizeof(struct easymesh_radio_mib));
		pconfig->per_radio_config        = (struct radio_config_data *)realloc(pconfig->per_radio_config, pconfig->config_data_nr * sizeof(struct radio_config_data));
		_5gh_config                      = &(pconfig->radio_data[pconfig->radio_data_nr - 1]);
		_5gh_vendor_data                 = &(pconfig->per_radio_config[pconfig->config_data_nr - 1]);
		_5gh_vendor_data->vendor_data_nr = 0;
		_5gh_vendor_data->vendor_datas   = NULL;
		_5gh_config->radio_channel       = atoi(value);
		_5gh_config->radio_band          = EASYMESH_RADIO_5GH;
		map_config_debug("[CONFIG] agent 5gh channel is %d\n", _5gh_config->radio_channel);
	} else if (MATCH("2.4g_setmib_config_data", "radio_channel")) {
		pconfig->radio_data_nr += 1;
		pconfig->config_data_nr += 1;
		pconfig->radio_data             = (struct easymesh_radio_mib *)realloc(pconfig->radio_data, pconfig->radio_data_nr * sizeof(struct easymesh_radio_mib));
		pconfig->per_radio_config       = (struct radio_config_data *)realloc(pconfig->per_radio_config, pconfig->config_data_nr * sizeof(struct radio_config_data));
		_2g_config                      = &(pconfig->radio_data[pconfig->radio_data_nr - 1]);
		_2g_vendor_data                 = &(pconfig->per_radio_config[pconfig->config_data_nr - 1]);
		_2g_vendor_data->vendor_data_nr = 0;
		_2g_vendor_data->vendor_datas   = NULL;
		_2g_config->radio_channel       = atoi(value);
		_2g_config->radio_band          = EASYMESH_RADIO_2G;
		map_config_debug("[CONFIG] agent 2.4g channel is %d\n", _2g_config->radio_channel);
	} else if (MATCH("dpp", "signed_connector_1905")) {
		pconfig->signed_connector_1905 = strdup(value);
		map_config_debug("[CONFIG] signed_connector_1905: %s\n", pconfig->signed_connector_1905);
	} else if (MATCH("dpp", "netaccess_key")) {
		pconfig->netaccess_key = strdup(value);
		map_config_debug("[CONFIG] netaccess_key: %s\n", pconfig->netaccess_key);
	} else if (MATCH("dpp", "csign_key")) {
		pconfig->csign_key = strdup(value);
		map_config_debug("[CONFIG] csign_key: %s\n", pconfig->csign_key);
	} else if (MATCH("dpp", "pp_key")) {
		pconfig->pp_key = strdup(value);
		map_config_debug("[CONFIG] pp_key: %s\n", pconfig->pp_key);
	}

	if (_5gl_config) {
		_fill_set_mib_config(section, name, value, "5gl_setmib_config_data", _5gl_config);
		_fill_setmib_vendor_data(section, name, value, "vendor_data_5gl", _5gl_vendor_data);
	}

	if (_5gh_config) {
		_fill_set_mib_config(section, name, value, "5gh_setmib_config_data", _5gh_config);
		_fill_setmib_vendor_data(section, name, value, "vendor_data_5gh", _5gh_vendor_data);
	}

	if (_2g_config) {
		_fill_set_mib_config(section, name, value, "2.4g_setmib_config_data", _2g_config);
		_fill_setmib_vendor_data(section, name, value, "vendor_data_2.4g", _2g_vendor_data);
	}

	return 1;
}

int read_mib_config_file(void *user, const char *section, const char *name, const char *value)
{
	struct easymesh_datamodel *       pconfig     = (struct easymesh_datamodel *)user;
	static struct easymesh_radio_mib *_5gl_config = NULL;
	static struct easymesh_radio_mib *_5gh_config = NULL;
	static struct easymesh_radio_mib *_2g_config  = NULL;

	if (MATCH("global", "configured_band")) {
		pconfig->configured_band = atoi(value);
		map_config_debug("[CONFIG] Configured band is %d\n", pconfig->configured_band);
	} else if (MATCH("global", "hw_reg_domain")) {
		pconfig->hw_reg_domain = atoi(value);
		map_config_debug("[CONFIG] HW redion domain is %d\n", pconfig->hw_reg_domain);
	} else if (MATCH("global", "hw_country_str")) {
		pconfig->hw_country_str = strdup(value);
		map_config_debug("[CONFIG] HW country string is %s\n", pconfig->hw_country_str);
	} else if (MATCH("global", "customize_features")) {
		pconfig->customize_features = atoi(value);
		map_config_debug("[CONFIG] Customize Features is %u\n", pconfig->customize_features);
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	else if (MATCH("mib_info_5gl", "channel")) {
		pconfig->radio_data_nr += 1;
		pconfig->radio_data              = (struct easymesh_radio_mib *)realloc(pconfig->radio_data, pconfig->radio_data_nr * sizeof(struct easymesh_radio_mib));
		_5gl_config                      = &(pconfig->radio_data[pconfig->radio_data_nr - 1]);
		_5gl_config->radio_channel       = atoi(value);
		_5gl_config->need_change_channel = 0;
		_5gl_config->radio_band          = EASYMESH_RADIO_5GL;
		map_config_debug("[CONFIG] agent 5g channel is %d\n", _5gl_config->radio_channel);

	} else if (MATCH("mib_info_5gh", "channel")) {
		pconfig->radio_data_nr += 1;
		pconfig->radio_data              = (struct easymesh_radio_mib *)realloc(pconfig->radio_data, pconfig->radio_data_nr * sizeof(struct easymesh_radio_mib));
		_5gh_config                      = &(pconfig->radio_data[pconfig->radio_data_nr - 1]);
		_5gh_config->radio_channel       = atoi(value);
		_5gh_config->need_change_channel = 0;
		_5gh_config->radio_band          = EASYMESH_RADIO_5GH;
		map_config_debug("[CONFIG] agent 5g channel is %d\n", _5gh_config->radio_channel);

	} else if (MATCH("mib_info_2.4g", "channel")) {
		pconfig->radio_data_nr += 1;
		pconfig->radio_data             = (struct easymesh_radio_mib *)realloc(pconfig->radio_data, pconfig->radio_data_nr * sizeof(struct easymesh_radio_mib));
		_2g_config                      = &(pconfig->radio_data[pconfig->radio_data_nr - 1]);
		_2g_config->radio_channel       = atoi(value);
		_2g_config->need_change_channel = 0;
		_2g_config->radio_band          = EASYMESH_RADIO_2G;
		map_config_debug("[CONFIG] agent 2.4g channel is %d\n", _2g_config->radio_channel);

	} else if (MATCH("dpp", "csign_key")) {
		pconfig->csign_key = strdup(value);
		map_config_debug("[CONFIG] DPP C-sign key is %s\n", pconfig->csign_key);
	} else if (MATCH("dpp", "netaccess_key")) {
		pconfig->netaccess_key = strdup(value);
		map_config_debug("[CONFIG] DPP Netaccess key is %s\n", pconfig->netaccess_key);
	} else if (MATCH("dpp", "pp_key")) {
		pconfig->pp_key = strdup(value);
		map_config_debug("[CONFIG] DPP Privacy Protection key is %s\n", pconfig->pp_key);
	} else if (MATCH("dpp", "signed_connector_1905")) {
		pconfig->signed_connector_1905 = strdup(value);
		map_config_debug("[CONFIG] DPP 1905 Signed Connector is %s\n", pconfig->signed_connector_1905);
	}

	if (_5gl_config) {
		_fill_mib_info(section, name, value, "mib_info_5gl", _5gl_config);
	}

	if (_5gh_config) {
		_fill_mib_info(section, name, value, "mib_info_5gh", _5gh_config);
	}

	if (_2g_config) {
		_fill_mib_info(section, name, value, "mib_info_2.4g", _2g_config);
	}

	return 1;
}

int read_map_ext_conf(void *user, const char *section, const char *name, const char *value)
{
	struct map_checker_datamodel *       pconfig     = (struct map_checker_datamodel *)user;

	if (MATCH("global", "backhaul_radio")) {
		pconfig->backhaul_sta_radio_index = atoi(value);
		printf("[CONFIG-EXT] backhaul_sta_radio_index is %d\n", pconfig->backhaul_sta_radio_index);
	} else if (MATCH("global", "backhaul_switch")) {
		pconfig->backhaul_switch = atoi(value);
		printf("[CONFIG-EXT] backhaul_switch is %d\n", pconfig->backhaul_switch);
	} else if (MATCH("global", "check_interval")) {
		pconfig->check_interval = atoi(value);
		printf("[CONFIG-EXT] check_interval is %d\n", pconfig->check_interval);
	} else if (MATCH("global", "link_down_threshold")) {
		pconfig->link_down_threshold = atoi(value);
		printf("[CONFIG-EXT] link_down_threshold is %d\n", pconfig->link_down_threshold);
	} else if (MATCH("global", "discovery_down_threshold")) {
		pconfig->discovery_down_threshold = atoi(value);
		printf("[CONFIG-EXT] discovery_down_threshold is %d\n", pconfig->discovery_down_threshold);
	} else if (MATCH("global", "arp_loop")) {
		pconfig->arp_loop = atoi(value);
		printf("[CONFIG-EXT] arp_loop is %d\n", pconfig->arp_loop);
	} else if (MATCH("global", "auto_renew_ip")) {
		pconfig->auto_renew_ip = atoi(value);
		printf("[CONFIG-EXT] auto_renew_ip is %d\n", pconfig->auto_renew_ip);
	}

	return 1;
}

uint8_t _write_mib_data(FILE *fp, const char *radio_name, struct easymesh_radio_mib *radio_mib)
{
	int i = 0;

	if (NULL == radio_name || NULL == radio_mib) {
		return 1;
	}

	fprintf(fp, "[%s_setmib_config_data]\n", radio_name);
	fprintf(fp, "radio_channel = %d\n", radio_mib->radio_channel);
	fprintf(fp, "channel_bandwidth = %d\n", radio_mib->channel_bandwidth);
	fprintf(fp, "control_sideband = %d\n", radio_mib->control_sideband);
	fprintf(fp, "channel_changed = %d\n", radio_mib->need_change_channel);

	unsigned char  config_number  = 0;
	char **        ssids          = (char **)malloc(1 * sizeof(char *));
	char **        network_keys   = (char **)malloc(1 * sizeof(char *));
	unsigned char *is_enabled     = (unsigned char *)malloc(1 * sizeof(unsigned char));
	unsigned char *need_configure = (unsigned char *)malloc(1 * sizeof(unsigned char));
	unsigned char *bss_type       = (unsigned char *)malloc(1 * sizeof(unsigned char));
	unsigned char *encrypt_type   = (unsigned char *)malloc(1 * sizeof(unsigned char));
	unsigned char *authentication_type   = (unsigned char *)malloc(1 * sizeof(unsigned char));
	unsigned char *bss_index       = (unsigned char *)malloc(1 * sizeof(unsigned char));
	char **        signed_connectors = (char **)malloc(1 * sizeof(char *));

	size_t base64_token_length                 = 0;

	for (i = 0; i < radio_mib->interface_nr; i++) {
		config_number++;
		ssids                             = (char **)realloc(ssids, (config_number) * sizeof(char *));
		network_keys                      = (char **)realloc(network_keys, (config_number) * sizeof(char *));
		is_enabled                        = (unsigned char *)realloc(is_enabled, (config_number) * sizeof(unsigned char));
		bss_type                          = (unsigned char *)realloc(bss_type, (config_number) * sizeof(unsigned char));
		bss_index                         = (unsigned char *)realloc(bss_index, (config_number) * sizeof(unsigned char));
		need_configure                    = (unsigned char *)realloc(need_configure, (config_number) * sizeof(unsigned char));
		encrypt_type                      = (unsigned char *)realloc(encrypt_type, (config_number) * sizeof(unsigned char));
		authentication_type               = (unsigned char *)realloc(authentication_type, (config_number) * sizeof(unsigned char));
		signed_connectors                 = (char **)realloc(signed_connectors, (config_number) * sizeof(char *));

		ssids[config_number - 1]          = (char *)base64_encode((unsigned char *)radio_mib->interface_mib[i].ssid, strlen(radio_mib->interface_mib[i].ssid), &base64_token_length);
		network_keys[config_number - 1]   = (char *)base64_encode((unsigned char *)radio_mib->interface_mib[i].network_key, strlen(radio_mib->interface_mib[i].network_key), &base64_token_length);
		is_enabled[config_number - 1]     = radio_mib->interface_mib[i].is_enabled;
		need_configure[config_number - 1] = radio_mib->interface_mib[i].need_configure;
		bss_type[config_number - 1]       = radio_mib->interface_mib[i].network_type;
		encrypt_type[config_number - 1]   = radio_mib->interface_mib[i].encrypt_type;
		authentication_type[config_number - 1]   = radio_mib->interface_mib[i].authentication_type;
		bss_index[config_number - 1]      = radio_mib->interface_mib[i].bss_index;
		if (radio_mib->interface_mib[i].signed_connector)
			signed_connectors[config_number - 1]  = radio_mib->interface_mib[i].signed_connector;
		else
			signed_connectors[config_number - 1]  = "null";
		// printf("conn: %s\n", signed_connectors[config_number - 1]);
	}
	fprintf(fp, "number = %d\n", config_number);

	fprintf(fp, "ssid = ");
	_writeToConfig(fp, config_number, ssids);

	fprintf(fp, "network_key = ");
	_writeToConfig(fp, config_number, network_keys);
	fprintf(fp, "is_enabled = ");
	_writeToConfig_dec(fp, config_number, is_enabled);
	fprintf(fp, "need_configure = ");
	_writeToConfig_dec(fp, config_number, need_configure);
	fprintf(fp, "bss_type = ");
	_writeToConfig_dec(fp, config_number, bss_type);
	fprintf(fp, "encrypt_type = ");
	_writeToConfig_dec(fp, config_number, encrypt_type);
	fprintf(fp, "authentication_type = ");
	_writeToConfig_dec(fp, config_number, authentication_type);
	fprintf(fp, "bss_index = ");
	_writeToConfig_dec(fp, config_number, bss_index);

	fprintf(fp, "signed_connector = ");
	_writeToConfig(fp, config_number, signed_connectors);

	for (i = 0; i < config_number; i++) {
		free(ssids[i]);
		free(network_keys[i]);
	}
	free(ssids);
	free(network_keys);
	free(is_enabled);
	free(need_configure);
	free(bss_type);
	free(encrypt_type);
	free(authentication_type);
	free(bss_index);
	free(signed_connectors);

	return 0;
}

uint8_t _write_vendor_data(FILE *fp, const char *radio_name, struct radio_config_data *radio_config)
{
	int i = 0;

	if (NULL == radio_name || NULL == radio_config) {
		return 1;
	}

	if (radio_config->vendor_data_nr) {
		unsigned short * load_len   = (unsigned short *)malloc(radio_config->vendor_data_nr * sizeof(unsigned short));
		char **         payload    = (char **)malloc(radio_config->vendor_data_nr * sizeof(char *));
		unsigned char **vendor_oui = (unsigned char **)malloc(radio_config->vendor_data_nr * sizeof(unsigned char *));
		for (i = 0; i < radio_config->vendor_data_nr; i++) {
			load_len[i]   = radio_config->vendor_datas[i].payload_len;
			payload[i]    = strdup((char *)radio_config->vendor_datas[i].payload);
			vendor_oui[i] = (uint8_t *)malloc(3 * sizeof(uint8_t));
			memcpy(vendor_oui[i], radio_config->vendor_datas[i].vendor_oui, 3);
			//map_config_debug("[%s_%d]check vendor OUI: %02x%02x%02x\n\n\n", __FUNCTION__, __LINE__, vendor_oui[i][0], vendor_oui[i][1], vendor_oui[i][2]); //qw
		}

		fprintf(fp, "[vendor_data_%s]\n", radio_name);

		fprintf(fp, "vendor_data_nr = %d\n", radio_config->vendor_data_nr);

		fprintf(fp, "vendor_load_len = ");
		_writeToConfig_dec_short(fp, radio_config->vendor_data_nr, load_len);

		fprintf(fp, "vendor_payload = ");
		_writeToConfig(fp, radio_config->vendor_data_nr, payload);

		fprintf(fp, "vendor_oui = ");
		_writeOuiToConfig(fp, radio_config->vendor_data_nr, vendor_oui);

		free(payload);
		free(vendor_oui);
		free(load_len);
	}

	return 0;
}

int write_set_mib_config_file(struct easymesh_datamodel *data_container)
{
	struct easymesh_radio_mib *_5gh_setmib = NULL;
	struct easymesh_radio_mib *_5gl_setmib = NULL;
	struct easymesh_radio_mib *_2g_setmib  = NULL;

	struct radio_config_data *_5gh_config = NULL;
	struct radio_config_data *_5gl_config = NULL;
	struct radio_config_data *_2g_config  = NULL;
	int                       i;

	for (i = 0; i < data_container->radio_data_nr; i++) {
		if (EASYMESH_RADIO_2G == data_container->radio_data[i].radio_band) {
			_2g_setmib = &(data_container->radio_data[i]);
		} else if (EASYMESH_RADIO_5GL == data_container->radio_data[i].radio_band) {
			_5gl_setmib = &(data_container->radio_data[i]);
		} else if (EASYMESH_RADIO_5GH == data_container->radio_data[i].radio_band) {
			_5gh_setmib = &(data_container->radio_data[i]);
		}
	}

	for (i = 0; i < data_container->config_data_nr; i++) {
		if (MAP_CONFIG_2G == data_container->per_radio_config[i].radio_band) {
			_2g_config = &(data_container->per_radio_config[i]);
		} else if (MAP_CONFIG_5GL == data_container->per_radio_config[i].radio_band) {
			_5gl_config = &(data_container->per_radio_config[i]);
		} else if (MAP_CONFIG_5GH == data_container->per_radio_config[i].radio_band) {
			_5gh_config = &(data_container->per_radio_config[i]);
		}
	}

	FILE *fp = fopen("/var/multiap_setmib.conf", "we");
	if (fp == NULL) {
		printf("Error opening config file!\n");
		return 0;
	}

	// fprintf(fp, "[global]\n");
	// fprintf(fp, "alme_port_number = %d\n", data_container->alme_port_number);
	// fprintf(fp, "max_resend_time = %d\n", data_container->max_resend_time);
	// fprintf(fp, "rssi_weightage = %d\n", data_container->rssi_weightage);
	// fprintf(fp, "path_weightage = %d\n", data_container->path_weightage);
	// fprintf(fp, "cu_weightage = %d\n", data_container->cu_weightage);
	// fprintf(fp, "roam_score_difference = %d\n", data_container->roam_score_difference);
	// fprintf(fp, "min_evaluation_interval = %d\n", data_container->min_evaluation_interval);
	// fprintf(fp, "min_roam_interval = %d\n", data_container->min_roam_interval);
	// fprintf(fp, "device_name = %s\n", data_container->device_name);
	// fprintf(fp, "max_num_device_allowed = %d\n", data_container->max_num_device_allowed);
	// fprintf(fp, "throughput_threshold = %d\n", data_container->throughput_threshold);

	fprintf(fp, "[setmib_global_data]\n");
	fprintf(fp, "configured_band = %d\n", data_container->configured_band);
	fprintf(fp, "hw_reg_domain = %d\n", data_container->hw_reg_domain);

	fprintf(fp, "[dpp]\n");
	if (data_container->signed_connector_1905)
		fprintf(fp, "signed_connector_1905 = %s\n", data_container->signed_connector_1905);
	if (data_container->netaccess_key)
		fprintf(fp, "netaccess_key = %s\n", data_container->netaccess_key);
	if (data_container->csign_key)
		fprintf(fp, "csign_key = %s\n", data_container->csign_key);
	if (data_container->pp_key)
		fprintf(fp, "pp_key = %s\n", data_container->pp_key);

	if (_5gl_setmib && _5gl_setmib->interface_nr) {
		_write_mib_data(fp, "5gl", _5gl_setmib);
	}

	if (_5gl_config && _5gl_config->vendor_data_nr) {
		_write_vendor_data(fp, "5gl", _5gl_config);
	}

	if (_5gh_setmib && _5gh_setmib->interface_nr) {
		_write_mib_data(fp, "5gh", _5gh_setmib);
	}

	if (_5gh_config && _5gh_config->vendor_data_nr) {
		_write_vendor_data(fp, "5gh", _5gh_config);
	}

	if (_2g_setmib && _2g_setmib->interface_nr) {
		_write_mib_data(fp, "2.4g", _2g_setmib);
	}

	if (_2g_config && _2g_config->vendor_data_nr) {
		_write_vendor_data(fp, "2.4g", _2g_config);
	}

	fclose(fp);

	return 0;
}

int write_api_info_file(struct easymesh_api_info *data_container)
{
	FILE *fp = fopen("/var/multiap_api_info", "we");
	if (fp == NULL) {
		printf("Error opening API info file!\n");
		return 0;
	}

	fprintf(fp, "[Retrieve API info]\n");
	fprintf(fp, "Agent connection status: ");

	switch (data_container->is_controller_found) {
	case 1: {
		fprintf(fp, "CONTROLLER LOST!\n");
		break;
	}
	case 2: {
		fprintf(fp, "CONTROLLER FOUND from %s!\n", data_container->backhaul_interface);
		fprintf(fp, "Parent AL MAC address %02x:%02x:%02x:%02x:%02x:%02x!\n", data_container->parent_id[0], data_container->parent_id[1], data_container->parent_id[2], data_container->parent_id[3], data_container->parent_id[4], data_container->parent_id[5]);
		break;
	}
	default: {
		fprintf(fp, "UNKNOWN!\n");
		break;
	}
	}

	fclose(fp);

	return 0;
}
