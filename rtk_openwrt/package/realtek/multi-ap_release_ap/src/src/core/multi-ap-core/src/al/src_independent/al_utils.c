/*
 *  Broadband Forum IEEE 1905.1/1a stack
 *
 *  Copyright (c) 2017, Broadband Forum
 *  Copyright (c) 2018, Realtek Semiconductor Corp.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  Subject to the terms and conditions of this license, each copyright
 *  holder and contributor hereby grants to those receiving rights under
 *  this license a perpetual, worldwide, non-exclusive, no-charge,
 *  royalty-free, irrevocable (except for failure to satisfy the
 *  conditions of this license) patent license to make, have made, use,
 *  offer to sell, sell, import, and otherwise transfer this software,
 *  where such license applies only to those patent claims, already
 *  acquired or hereafter acquired, licensable by such copyright holder or
 *  contributor that are necessarily infringed by:
 *
 *  (a) their Contribution(s) (the licensed copyrights of copyright holders
 *      and non-copyrightable additions of contributors, in source or binary
 *      form) alone; or
 *
 *  (b) combination of their Contribution(s) with the work of authorship to
 *      which such Contribution(s) was added by such copyright holder or
 *      contributor, if, at the time the Contribution is added, such addition
 *      causes such combination to be necessarily infringed. The patent
 *      license shall not apply to any other combinations which include the
 *      Contribution.
 *
 *  Except as expressly stated above, no rights or licenses from any
 *  copyright holder or contributor is granted under this license, whether
 *  expressly, by implication, estoppel or otherwise.
 *
 *  DISCLAIMER
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 *  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 *  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 *  DAMAGE.
 */

#include <pthread.h> // mutex functions
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include "platform.h"
#include "platform_crypto.h"

#include "al_utils.h"
#include "multi_ap_cmdus.h"
#include "1905_l2.h"
#include "platform_interfaces.h"
#include "al_datamodel.h"
#include "al.h"
#include "al_wsc.h"

#include "global_settings.h"
#include "packet_tools.h"

#ifndef __UCLIBC__
#include "utils.h"
#endif
////////////////////////////////////////////////////////////////////////////////
// Public functions (exported only to files in this same folder)
////////////////////////////////////////////////////////////////////////////////
static pthread_mutex_t resend_timer_mutex = PTHREAD_MUTEX_INITIALIZER;

#define MAX_WAITING_MSG_NUM 10
WaitingMsg wms[MAX_WAITING_MSG_NUM];

INT16U getNextMid(void)
{
	static INT16U mid        = 0;
	static INT8U  first_time = 1;

	if (1 == first_time) {
		// Start with a random MID. The standard is not clear about this, but
		// I think a random number is better than simply choosing zero, to
		// avoid start up problems (ex: one node boots and after a short time
		// it is reset and starts making use of the same MIDs all over again,
		// which will probably be ignored by other nodes, thinking they have
		// already processed these messages in the past)
		//
		first_time = 0;
		PLATFORM_GET_RANDOM_BYTES((INT8U *)&mid, sizeof(INT16U));
		if (mid > 0xf000) {
			mid -= 0xf000;
		}
	} else {
		mid++;
	}

	if (0 == mid) {
		mid++;
	}

	return mid;
}

INT16U get_mid_from_stream(INT8U *stream)
{
	INT16U mid = 0;

	mid = stream[5];
	mid = (stream[4] << 8) + mid;
	return mid;
}
void set_mid_for_streams(INT8U **streams, INT16U mid)
{
	int x = 0;
	while (streams[x]) {
		INT8U *m = streams[x];
		m[4]     = (mid & 0xff00) >> 8;
		m[5]     = mid & 0x00ff;
		x++;
	}
}

INT16U get_message_type_from_stream(INT8U *stream)
{
	INT16U message_type = 0;
#if _HOST_IS_BIG_ENDIAN_ == 1
	message_type = stream[2];
	message_type = (stream[3] << 8) + message_type;
#elif _HOST_IS_LITTLE_ENDIAN_ == 1
	message_type = stream[3];
	message_type = (stream[2] << 8) + message_type;
#else
#	error You must specify your architecture endianess
#endif
	return message_type;
}

// find the pointer using message id
// find the vacancy pointer if the message id is 0
WaitingMsg *_waiting_msgs_search_by_mid(INT16U mid, INT8U flag)
{
	int i;
	for (i = 0; i < MAX_WAITING_MSG_NUM; i++) {
		if (flag && 0 == wms[i].timer_id)
			continue;
		if (wms[i].mid == mid)
			break; // corresponding streams is found
	}

	if (i < MAX_WAITING_MSG_NUM) {
		// found the streams
		return &wms[i];
	} else {
		return NULL;
	}
}

WaitingMsg *_waiting_msgs_search_by_timer_id(timer_t timer_id)
{
	int i;
	for (i = 0; i < MAX_WAITING_MSG_NUM; i++) {
		if (wms[i].timer_id == timer_id)
			break; // corresponding streams is found
	}

	if (i < MAX_WAITING_MSG_NUM) {
		// found the streams
		return &wms[i];
	} else {
		return NULL;
	}
}

void _waiting_msg_init(WaitingMsg *m)
{
	m->timer_id = 0;
	m->mid      = 0;
	free_1905_CMDU_packets(m->streams);
	m->streams = NULL;
	PLATFORM_MEMSET(m->dst_mac_address, '\0', 6);
	PLATFORM_MEMSET(m->interface_name, '\0', 20);
	PLATFORM_FREE(m->streams_lens);
	m->timeout_count = 0;
}

INT8U waiting_msgs_add(char *interface_name, INT8U dst_mac_address[], INT8U **streams, INT16U *streams_lens)
{
	// add message that needs ack to waiting list

	if (PLATFORM_STRLEN(interface_name) > 20 || NULL == streams || NULL == streams[0]) {
		PLATFORM_PRINTF_ERROR("[Waiting Message] The interface name (%s) is too lang.\n", interface_name);
		return 1;
	}
	// Message id is same for all the fragments in one CMDU
	INT16U mid = get_mid_from_stream(streams[0]);

	WaitingMsg *m = _waiting_msgs_search_by_mid(0, 0);
	if (NULL == m) {
		// Waiting message list is full
		return 1;
	} else {

		// setup timer

		WaitingMsgTimerData *d = (WaitingMsgTimerData *)malloc(sizeof(WaitingMsgTimerData));
		struct sigevent      se;
		timer_t              timer_id;

		memset(&se, 0, sizeof(se));
		se.sigev_notify          = SIGEV_THREAD;
		se.sigev_notify_function = waiting_msgs_timeout;
		se.sigev_value.sival_ptr = (void *)d;

		if (-1 == timer_create(CLOCK_REALTIME, &se, &timer_id)) {
			// Failed to create a new timer
			//
			PLATFORM_PRINTF_ERROR("[PLATFORM] timer_create() returned with errno=%d (%s)\n", errno, strerror(errno));
			free(d);
			return 1;
		}
		d->timer_id = timer_id;
		d->mid      = mid;
		// add the messsage related info to the waiting message list
		m->timer_id = timer_id;
		m->mid      = mid;
		m->streams  = streams;
		PLATFORM_MEMCPY(m->dst_mac_address, dst_mac_address, 6);
		PLATFORM_MEMCPY(m->interface_name, interface_name, PLATFORM_STRLEN(interface_name));
		m->streams_lens  = streams_lens;
		m->timeout_count = 0;
		//
		// Finally, arm/start the timer
		struct itimerspec its;
		its.it_value.tv_sec     = ACK_TIMEOUT_THRESHOLD;
		its.it_value.tv_nsec    = 00000000;
		its.it_interval.tv_sec  = 0;
		its.it_interval.tv_nsec = 0;

		if (0 != timer_settime(timer_id, 0, &its, NULL)) {
			// Problems arming the timer
			free(d);
			timer_delete(timer_id);
			_waiting_msg_init(m);
			return 2;
		}
	}
	return 0;
}

void waiting_msgs_timeout(union sigval s)
{
	// timeout event happened
	pthread_mutex_lock(&resend_timer_mutex);
	WaitingMsgTimerData *d = (WaitingMsgTimerData *)s.sival_ptr;

	INT16U  mid      = d->mid;
	timer_t timer_id = d->timer_id;

	PLATFORM_PRINTF_DETAIL("[Waiting Message] Timeout Event MID: (%x) timer_id:%d \n", mid, (intptr_t)timer_id);

	WaitingMsg *m = _waiting_msgs_search_by_timer_id(timer_id);
	if (NULL == m) {
		// it has got ack and removed from the waiting message list already
		PLATFORM_PRINTF_DETAIL("[Waiting Message] No such timer_id %d \n", (intptr_t)timer_id);
		timer_delete(timer_id);
		PLATFORM_FREE(d);
		pthread_mutex_unlock(&resend_timer_mutex);
		return;
	}

	if (m->timeout_count <= config.max_resend_time) {
		// set new message id
		INT16U mid_new = getNextMid();
		set_mid_for_streams(m->streams, mid_new);
		m->mid = mid_new;
		d->mid = mid_new;
		m->timeout_count += 1;

		// resend the streams with new message id
		PLATFORM_PRINTF_DEBUG("[Waiting Message] Resend the message (%04x) using new mid %04x on %s\n", mid, mid_new, m->interface_name);

		int total_streams = 0;
		while (m->streams[total_streams]) {
			total_streams++;
		}

		int x = 0;
		while (m->streams[x]) {
			PLATFORM_PRINTF_DETAIL("[Waiting Message] Resending message(%x) on interface %s, fragment %d/%d\n", mid_new, m->interface_name, x + 1, total_streams);
			if (0 == PLATFORM_SEND_RAW_PACKET(m->interface_name, m->dst_mac_address, DMalMacGet(), ETHERTYPE_1905, m->streams[x], m->streams_lens[x])) {
				PLATFORM_PRINTF_ERROR("[Waiting Message] Packet could not be sent!\n");
			}
			x++;
		}
		// reset the timer because the resend procedure also takes time
		struct itimerspec its;
		its.it_value.tv_sec     = ACK_TIMEOUT_THRESHOLD;
		its.it_value.tv_nsec    = 00000000;
		its.it_interval.tv_sec  = 0;
		its.it_interval.tv_nsec = 0;

		if (0 != timer_settime(timer_id, 0, &its, NULL)) {
			// Problems arming the timer
			PLATFORM_FREE(d);
			timer_delete(timer_id);
			_waiting_msg_init(m);
			pthread_mutex_unlock(&resend_timer_mutex);
			return;
		}

	} else {
		PLATFORM_PRINTF_DEBUG("[Waiting Message] Discard the message (%04x) after sent %d times\n", m->mid, m->timeout_count);
		// todo update database: remove the corresponding device
		//
		if (get_message_type_from_stream(m->streams[0]) == CMDU_TYPE_CHANNEL_PREFERENCE_QUERY) {
			DMchannelQueryStatusSet(0);
		}
		timer_delete(timer_id);
		PLATFORM_FREE(d);
		_waiting_msg_init(m);
	}
	pthread_mutex_unlock(&resend_timer_mutex);
}

INT8U waiting_msgs_ack(INT16U mid)
{
	// ack message or paired message arrived
	pthread_mutex_lock(&resend_timer_mutex);
	WaitingMsg *m = _waiting_msgs_search_by_mid(mid, 1);
	if (NULL == m) {
		PLATFORM_PRINTF_DEBUG("[Waiting Message] Message(%04x) has been resent using different mid or not in record\n", mid);
		pthread_mutex_unlock(&resend_timer_mutex);
		return 1;
	}

	PLATFORM_PRINTF_DEBUG("[Waiting Message] Message (%04x) is acknowledged after resend %d times\n", m->mid, m->timeout_count);
	_waiting_msg_init(m);
	pthread_mutex_unlock(&resend_timer_mutex);
	return 0;
}

char **get_backhaul_interfaces(INT8U *backhaul_interfaces_nr)
{
	char **ifs_names;
	INT8U  ifs_nr;
	ifs_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&ifs_nr);

	*backhaul_interfaces_nr   = 0;
	char **backhaul_ifs_names = (char **)PLATFORM_MALLOC(sizeof(char *) * ifs_nr);

	int i;
	for (i = 0; i < ifs_nr; i++) {
		struct LocalInterface *l = DMnameToLocalInterfaceStruct(ifs_names[i]);
		if (NULL == l) {
			continue;
		} else if ((l->role & BACKHAUL_BSS) || (l->role & BACKHAUL_STA)) {
			//find out backhaul interfaces
			struct interfaceInfo *x = PLATFORM_GET_1905_INTERFACE_INFO(l->name);
			if (NULL == x) {
				PLATFORM_PRINTF_WARNING("Could not retrieve info of interface %s\n", l->name);
				continue;
			}
			if (INTERFACE_POWER_STATE_OFF == x->power_state) {
				PLATFORM_FREE_1905_INTERFACE_INFO(x);
				continue;
			}
			*(backhaul_ifs_names + (*backhaul_interfaces_nr)) = l->name;
			(*backhaul_interfaces_nr)++;
			PLATFORM_FREE_1905_INTERFACE_INFO(x);
		}
	}

	// backhaul_ifs_names = PLATFORM_REALLOC(backhaul_ifs_names, (*backhaul_interfaces_nr));
	return backhaul_ifs_names;
}

void updateConfigData(INT8U config_data_nr, struct radio_config_data *config_data)
{
	int                       i = 0, j = 0;
	struct vendorSpecificTLV *vendor_tlvs = NULL;

	if (config.config_data_24g != NULL || config.config_data_5gl != NULL || config.config_data_5gh != NULL) {
		PLATFORM_PRINTF_DEBUG("updateConfigData: Previous data not freed!\n");
		freeConfigData();
	}

	config.config_number_24g = 0;
	config.config_number_5gl = 0;
	config.config_number_5gh = 0;

	for (i = 0; i < config_data_nr; i++) {
		struct radio_config_data config_entry = config_data[i];
		struct configData *      config_pt    = NULL;
		INT8U                    config_nr    = 0;
		if (MAP_CONFIG_2G == config_entry.radio_band) {
			config.config_number_24g = config_entry.bss_data_nr;
			config_nr                = config.config_number_24g;
			config.config_data_24g   = PLATFORM_MALLOC(sizeof(struct configData) * config.config_number_24g);
			config_pt                = config.config_data_24g;
		} else if (MAP_CONFIG_5GL == config_entry.radio_band) {
			config.config_number_5gl = config_entry.bss_data_nr;
			config_nr                = config.config_number_5gl;
			config.config_data_5gl   = PLATFORM_MALLOC(sizeof(struct configData) * config.config_number_5gl);
			config_pt                = config.config_data_5gl;
		} else if (MAP_CONFIG_5GH == config_entry.radio_band) {
			config.config_number_5gh = config_entry.bss_data_nr;
			config_nr                = config.config_number_5gh;
			config.config_data_5gh   = PLATFORM_MALLOC(sizeof(struct configData) * config.config_number_5gh);
			config_pt                = config.config_data_5gh;
		}
		if (config_pt) {
			for (j = 0; j < config_nr; j++) {
				config_pt[j].ssid = PLATFORM_STRDUP(config_entry.bss_data[j].ssid);
				config_pt[j].vid  = config_entry.bss_data[j].vid;
				PLATFORM_PRINTF_DEBUG("%s %d %02x:%02x:%02x:%02x:%02x:%02x\n", config_pt[j].ssid, config_pt[j].vid, config_entry.bss_data[j].al_id[0], config_entry.bss_data[j].al_id[1], config_entry.bss_data[j].al_id[2], config_entry.bss_data[j].al_id[3], config_entry.bss_data[j].al_id[4], config_entry.bss_data[j].al_id[5]);
				PLATFORM_MEMCPY(config_pt[j].al_id, config_entry.bss_data[j].al_id, 6);
				config_pt[j].network_key = PLATFORM_STRDUP(config_entry.bss_data[j].network_key);

				if ((WPS_AUTH_WPA3 & config_entry.bss_data[j].authentication_type)
				    && (WPS_AUTH_WPA2PSK & config_entry.bss_data[j].authentication_type)
				    && (WPS_AUTH_DPP & config_entry.bss_data[j].authentication_type)) {
					PLATFORM_PRINTF_WARNING("Auth %02X WPA2/3/DPP Mixed\n", config_entry.bss_data[j].authentication_type);
					config_pt[j].auth_type = WPS_AUTH_WPA2PSK | WPS_AUTH_WPA3 | WPS_AUTH_DPP;
				} else if ((WPS_AUTH_WPA3 & config_entry.bss_data[j].authentication_type) && (WPS_AUTH_WPA2PSK & config_entry.bss_data[j].authentication_type)) {
					PLATFORM_PRINTF_DEBUG("Auth: %02x WPA2/3 Mixed\n", config_entry.bss_data[j].authentication_type);
					config_pt[j].auth_type = WPS_AUTH_WPA2PSK | WPS_AUTH_WPA3;
				} else if (WPS_AUTH_WPA3 & config_entry.bss_data[j].authentication_type) {
					PLATFORM_PRINTF_DEBUG("Auth: %02x WPA3\n", config_entry.bss_data[j].authentication_type);
					config_pt[j].auth_type = WPS_AUTH_WPA3;
				} else if ((WPS_AUTH_WPAPSK & config_entry.bss_data[j].authentication_type) && (WPS_AUTH_WPA2PSK & config_entry.bss_data[j].authentication_type)) {
					PLATFORM_PRINTF_DEBUG("Auth: %02x WPA2 Mixed\n", config_entry.bss_data[j].authentication_type);
					config_pt[j].auth_type = WPS_AUTH_WPAPSK | WPS_AUTH_WPA2PSK;
				} else if (WPS_AUTH_WPA2PSK & config_entry.bss_data[j].authentication_type) {
					PLATFORM_PRINTF_DEBUG("Auth: %02x WPA2\n", config_entry.bss_data[j].authentication_type);
					config_pt[j].auth_type = WPS_AUTH_WPA2PSK;
				} else if (WPS_AUTH_WPAPSK & config_entry.bss_data[j].authentication_type) {
					PLATFORM_PRINTF_DEBUG("Auth: %02x WPA\n", config_entry.bss_data[j].authentication_type);
					config_pt[j].auth_type = WPS_AUTH_WPAPSK;
				} else if (WPS_AUTH_SHARED & config_entry.bss_data[j].authentication_type) {
					PLATFORM_PRINTF_DEBUG("Auth: %02x WEP\n", config_entry.bss_data[j].authentication_type);
					config_pt[j].auth_type = WPS_AUTH_SHARED;
				} else if (WPS_AUTH_OPEN & config_entry.bss_data[j].authentication_type) {
					PLATFORM_PRINTF_DEBUG("Auth: %02x OPEN\n", config_entry.bss_data[j].authentication_type);
					config_pt[j].auth_type = WPS_AUTH_OPEN;
				} else {
					PLATFORM_PRINTF_DEBUG("Unsupported auth type %02x, use WPA2\n", config_entry.bss_data[j].authentication_type);
					config_pt[j].auth_type = WPS_AUTH_WPA2PSK;
				}
				config_pt[j].encryption_type = config_entry.bss_data[j].encryption_type;
				if((customize_features & SUPPORT_OPEN_ENCRYPT) && (WPS_AUTH_OPEN == config_entry.bss_data[j].authentication_type)){
					config_pt[j].auth_type = WPS_AUTH_OPEN;
					config_pt[j].encryption_type = 0;
				}
				config_pt[j].network_type    = config_entry.bss_data[j].network_type;
				PLATFORM_MEMCPY(config_pt[j].al_id, config_entry.bss_data[j].al_id, 6);
				config_pt[j].vid = config_entry.bss_data[j].vid;
			}

			if (config_entry.vendor_data_nr) {
				vendor_tlvs = (struct vendorSpecificTLV *)PLATFORM_MALLOC(sizeof(struct vendorSpecificTLV) * config_entry.vendor_data_nr);
				for (j = 0; j < config_entry.vendor_data_nr; j++) {
					vendor_tlvs[j].tlv_type = TLV_TYPE_VENDOR_SPECIFIC;
					PLATFORM_MEMCPY(vendor_tlvs[j].vendorOUI, config_entry.vendor_datas[j].vendor_oui, 3);
					vendor_tlvs[j].m_nr = config_entry.vendor_datas[j].payload_len;
					vendor_tlvs[j].m    = PLATFORM_MALLOC(sizeof(INT8U) * vendor_tlvs[j].m_nr);
					PLATFORM_MEMCPY(vendor_tlvs[j].m, config_entry.vendor_datas[j].payload, vendor_tlvs[j].m_nr);
				}
				DMupdateAutoconfigVendorTLV(config_entry.vendor_data_nr, vendor_tlvs, config_entry.radio_band);
			}
		}
	}

	return;
}

void freeConfigData()
{
	int i = 0;
	// PLATFORM_PRINTF_INFO("ConfigData 24g %02x\n", config.config_number_24g);
	for (i = 0; i < config.config_number_24g; i++) {
		PLATFORM_FREE(config.config_data_24g[i].ssid);
		PLATFORM_FREE(config.config_data_24g[i].network_key);
	}

	// PLATFORM_PRINTF_INFO("ConfigData 5g %02x\n", config.config_number_5g);
	for (i = 0; i < config.config_number_5gl; i++) {
		PLATFORM_FREE(config.config_data_5gl[i].ssid);
		PLATFORM_FREE(config.config_data_5gl[i].network_key);
	}

	for (i = 0; i < config.config_number_5gh; i++) {
		PLATFORM_FREE(config.config_data_5gh[i].ssid);
		PLATFORM_FREE(config.config_data_5gh[i].network_key);
	}

	config.config_number_24g = 0;
	config.config_number_5gl = 0;
	config.config_number_5gh = 0;
	PLATFORM_FREE(config.config_data_24g);
	PLATFORM_FREE(config.config_data_5gl);
	PLATFORM_FREE(config.config_data_5gh);
	config.config_data_24g = NULL;
	config.config_data_5gl = NULL;
	config.config_data_5gh = NULL;
	PLATFORM_PRINTF_DEBUG("ConfigData freed\n");
	return;
}

INT8U delete_agent(INT8U *agent_sta_mac, INT8U *bssid, INT8U send_disassoc, INT8U role, char *exclude_interface)
{
	char **ifs_names;
	INT8U  ifs_nr;
	ifs_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&ifs_nr);
	int i, k;

	INT8U result = 0;

	for (i = 0; i < ifs_nr; i++) {
		if (exclude_interface
		    && ((PLATFORM_STRSTR(exclude_interface, map_vap_prefix) && PLATFORM_STRSTR(ifs_names[i], exclude_interface))
		        || 0 == PLATFORM_STRCMP(exclude_interface, ifs_names[i]))) {
			continue;
		}
		struct LocalInterface *l = DMnameToLocalInterfaceStruct(ifs_names[i]);
		if (PLATFORM_GET_VALID_INTERFACE_NAME(ifs_names[i], VALID_ETH_INTERFACES_IN_BRIDGE)
			|| (controller_interface && PLATFORM_STRSTR(ifs_names[i], controller_interface))) {
			continue;
		}
		if (NULL == l) {
			continue;
		} else if (!(l->role & role)) {
			continue;
		}
		struct interfaceInfo *x = PLATFORM_GET_1905_INTERFACE_INFO(l->name);
		if (NULL == x) {
			PLATFORM_PRINTF_WARNING("Could not retrieve info of interface %s\n", l->name);
			continue;
		}
		if (INTERFACE_POWER_STATE_OFF == x->power_state) {
			PLATFORM_FREE_1905_INTERFACE_INFO(x);
			continue;
		}
		if (NULL != bssid) {
			if (0 == PLATFORM_MEMCMP(bssid, x->mac_address, 6)) {
				PLATFORM_PRINTF_DEBUG("BSSID for %.2x:%.2x:%.2x:%.2x:%.2x:%.2x "
					"is same as %.2x:%.2x:%.2x:%.2x:%.2x:%.2x for interface %s\n",
					agent_sta_mac[0], agent_sta_mac[1],
					agent_sta_mac[2], agent_sta_mac[3],
					agent_sta_mac[4], agent_sta_mac[5],
					x->mac_address[0], x->mac_address[1],
					x->mac_address[2], x->mac_address[3],
					x->mac_address[4], x->mac_address[5],
					ifs_names[i]);
				PLATFORM_FREE_1905_INTERFACE_INFO(x);
				return 0;
			}
		}
		for (k = 0; k < x->neighbor_mac_addresses_nr; k++) {
			if (0 == PLATFORM_MEMCMP(x->neighbor_list[k].mac_address, agent_sta_mac, 6)) {
				PLATFORM_FREE_1905_INTERFACE_INFO(x);

				if (send_disassoc) {
					result = PLATFORM_SEND_DISASSOC(ifs_names[i], agent_sta_mac);
				} else {
					result = PLATFORM_DELETE_STA(ifs_names[i], agent_sta_mac);
				}

				if (result) {
					PLATFORM_PRINTF_ERROR("Failed to remove agent "
						"%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
						agent_sta_mac[0], agent_sta_mac[1],
						agent_sta_mac[2], agent_sta_mac[3],
						agent_sta_mac[4], agent_sta_mac[5]);
					return 2;
				}
				return 1;
			}
		}
		PLATFORM_FREE_1905_INTERFACE_INFO(x);
	}
	return 0;
}

INT8U delete_agent_with_al_mac(INT8U *agent_al_mac, INT8U send_disassoc, char *exclude_interface)
{
	int                   i      = 0;
	struct networkDevice *device = DMnetworkDeviceGet(agent_al_mac);
	if (NULL == device) {
		return 1;
	}
	for (i = 0; i < device->info->local_interfaces_nr; i++) {
		if (IEEE80211_SPECIFIC_INFO_ROLE_NON_AP_NON_PCP_STA == device->info->local_interfaces[i].media_specific_data.ieee80211.role) {
			INT8U *sta_mac = device->info->local_interfaces[i].mac_address;
			delete_agent(sta_mac, NULL, send_disassoc, BACKHAUL_BSS, exclude_interface);
		}
	}
	return 0;
}

char *generate_timestamp() //timestamp generated is always a 30 char long string
{
	//
	//Please take note that the result of this function might be platform dependent.
	//Please verify the behaviour of the c <time> lib before porting and make necessary changes.
	//

	//get time zone offset
	time_t    t = time(NULL);
	struct tm lt;
	localtime_r(&t, &lt);

	long int time_offset;
	char     offset_sign;
	if (lt.tm_gmtoff < 0) {
		time_offset = lt.tm_gmtoff * (-1);
		offset_sign = '-';
	} else {
		time_offset = lt.tm_gmtoff;
		offset_sign = '+';
	}

	int offset_hr  = time_offset / 3600;
	int offset_min = time_offset / 60 % 60;

	//get the 5 digit after the decimal point
	struct timeval te;
	gettimeofday(&te, NULL);
	int decimalSecond = te.tv_usec / 10;

	//get YYYY-MM-DDThh:mm:ss
	time_t     rawtime;
	struct tm *info;

	time(&rawtime);
	info = localtime(&rawtime);

	//allocate memory for timestamp
	char *timestamp = (char *)PLATFORM_MALLOC(32); //our timestamp is always 31 char long
	timestamp[31]   = '\0';

	sprintf(timestamp, "%4d", (info->tm_year + 1900));
	sprintf(timestamp + 4, "-");
	sprintf(timestamp + 5, "%2d", (info->tm_mon + 1));
	sprintf(timestamp + 7, "-");
	sprintf(timestamp + 8, "%2d", (info->tm_mday));
	sprintf(timestamp + 10, "T");
	sprintf(timestamp + 11, "%2d", (info->tm_hour));
	sprintf(timestamp + 13, ":");
	sprintf(timestamp + 14, "%2d", (info->tm_min));
	sprintf(timestamp + 16, ":");
	sprintf(timestamp + 17, "%2d", (info->tm_sec));
	sprintf(timestamp + 19, ".");
	sprintf(timestamp + 20, "%5d", (decimalSecond));
	sprintf(timestamp + 25, "%c", (offset_sign));
	sprintf(timestamp + 26, "%2d", (offset_hr));
	sprintf(timestamp + 28, ":");
	sprintf(timestamp + 29, "%2d", (offset_min));

	//replace space with '0'
	while (NULL != PLATFORM_STRSTR(timestamp, " ")) {
		*(PLATFORM_STRSTR(timestamp, " ")) = '0';
	}

	return timestamp;
}

#ifdef RTK_MULTI_AP_R3
INT8U calculate_mic(INT8U *gtk, struct CMDU *cmdu, INT8U is_forwarding, struct MICTLV *mic_tlv)
{
	INT8U  reserved           = 0;
	INT8U  cmdu_octets[6]     = { 0 };
	INT8U  mic_tlv_octets[13] = { 0 };
	INT8U *all_tlvs           = NULL;
	INT16U all_tlvs_size      = 0;

	INT8U *addr[3];
	INT32U len[3];

	INT8U *p;
	INT8U  ret = 1;

	if (!gtk || !cmdu || !mic_tlv) {
		PLATFORM_PRINTF_DEBUG("[Calculate MIC] invalid input\n");
		return 1;
	}

	// To calculate MIC, we need:
	// 1. First 6 octets of the CMDU
	// 2. First 13 octets of the MIC TLV (Key ID, MIC Ver, ITC, Src 1905 AL)
	// 3. All TLVs (with End of Message TLV at the end)
	//

	// 1. First 6 octets of the CMDU
	p = cmdu_octets;
	_I1B(&cmdu->message_version, &p);
	_I1B(&reserved, &p);
	_I2B(&cmdu->message_type, &p);
	_I2B(&cmdu->message_id, &p);

	// 2. First 13 octets of the MIC TLV (Key ID, MIC Ver, ITC, Src 1905 AL)
	p = mic_tlv_octets;
	_I1B(&mic_tlv->gtk_key_id_ver, &p);
	_INT64_I6B(&mic_tlv->integrity_transmission_counter, &p);
	_InB(mic_tlv->src_1905_al_mac_addr, &p, 6);

	// 3. All TLVs in the CMDU
	all_tlvs = concat_all_TLVs_from_structure(cmdu, &all_tlvs_size, is_forwarding);
	all_tlvs = PLATFORM_REALLOC(all_tlvs, all_tlvs_size + 3); // EOM TLV (3 zero octets)
	PLATFORM_MEMSET(all_tlvs + all_tlvs_size, 0, 3);
	all_tlvs_size += 3;

	// Generate Message Integrity Code (MIC)
	//
	addr[0] = cmdu_octets;
	len[0]  = sizeof(cmdu_octets);
	addr[1] = mic_tlv_octets;
	len[1]  = sizeof(mic_tlv_octets);
	addr[2] = all_tlvs;
	len[2]  = all_tlvs_size;

	if (PLATFORM_HMAC_SHA256(gtk, SHA256_MAC_LEN, 3, addr, len, mic_tlv->mic) != 1) {
		PLATFORM_PRINTF_ERROR("[Calculate MIC] HMAC SHA256 Returned False\n");
		goto fail;
	}

	mic_tlv->mic_length = SHA256_MAC_LEN;
	ret                 = 0;

fail:
	PLATFORM_FREE(all_tlvs);
	return ret;
}
#endif // RTK_MULTI_AP_R3

void _fill_scan_result_tlvs(INT8U channel_scan_result_nr, struct ChannelScanResultTLV *channel_scan_results, INT8U *data_stream)
{
	INT8U i, j, l;
	INT8U channel_nr;
	PLATFORM_MEMCPY(&channel_nr, data_stream++, 1);
	for (i = 0; i < channel_nr; i++) {
		INT8U channel;
		PLATFORM_MEMCPY(&channel, data_stream++, 1);
		//for every channel in result stream, iterate through channel_scan_results to look for the same channel. if match found, fill in.

		//save the current pt, reset data_stream for each time a match found
		INT8U *data_stream_backup = data_stream;
		for (l = 0; l < channel_scan_result_nr; l++) {
			if (channel_scan_results[l].channel == channel) {
				//reset data_stream for each time a match found
				data_stream = data_stream_backup;
				PLATFORM_MEMCPY(&channel_scan_results[l].scan_status, data_stream++, 1);
				//***************************
				//comment out for this as we now hardcode time stamp. Add this back when generating real time stamp in wifi driver
				//***************************
				// PLATFORM_MEMCPY(channel_scan_results[l].timestamp, data_stream, 31);
				data_stream += 31;
				PLATFORM_MEMCPY(&channel_scan_results[l].channel_utilization, data_stream++, 1);
				PLATFORM_MEMCPY(&channel_scan_results[l].noise, data_stream++, 1);
				PLATFORM_MEMCPY(&channel_scan_results[l].neighbor_nr, data_stream, 2);
				data_stream += 2;
				channel_scan_results[l].neighbors = (struct ChannelScanNeighbor *)PLATFORM_MALLOC(channel_scan_results[l].neighbor_nr * sizeof(struct ChannelScanNeighbor));
				for (j = 0; j < channel_scan_results[l].neighbor_nr; j++) {
					PLATFORM_MEMCPY(channel_scan_results[l].neighbors[j].bssid, data_stream, 6);
					data_stream += 6;
					PLATFORM_MEMCPY(&channel_scan_results[l].neighbors[j].ssid_length, data_stream++, 1);
					channel_scan_results[l].neighbors[j].ssid = (char *)PLATFORM_MALLOC(channel_scan_results[l].neighbors[j].ssid_length + 1);
					PLATFORM_MEMCPY(channel_scan_results[l].neighbors[j].ssid, data_stream, channel_scan_results[l].neighbors[j].ssid_length);
					data_stream += channel_scan_results[l].neighbors[j].ssid_length;
					channel_scan_results[l].neighbors[j].ssid[channel_scan_results[l].neighbors[j].ssid_length] = '\0';
					PLATFORM_MEMCPY(&channel_scan_results[l].neighbors[j].signal_strength, data_stream++, 1);
					INT8U channel_band_width;
					PLATFORM_MEMCPY(&channel_band_width, data_stream++, 1);
					if (0 == channel_band_width) {
						//invalid
					} else if (1 == channel_band_width) {
						channel_scan_results[l].neighbors[j].channel_bandwidth = strdup("20");
					} else if (2 == channel_band_width) {
						channel_scan_results[l].neighbors[j].channel_bandwidth = strdup("40");
					} else if (3 == channel_band_width) {
						channel_scan_results[l].neighbors[j].channel_bandwidth = strdup("80");
					} else if (4 == channel_band_width) {
						channel_scan_results[l].neighbors[j].channel_bandwidth = strdup("80+80");
					} else if (5 == channel_band_width) {
						channel_scan_results[l].neighbors[j].channel_bandwidth = strdup("160");
					}
					channel_scan_results[l].neighbors[j].channel_bandwidth_length = strlen(channel_scan_results[l].neighbors[j].channel_bandwidth);
					channel_scan_results[l].neighbors[j].field_present_flags      = 0xC0; //1100 0000 hardcode for now
					PLATFORM_MEMCPY(&channel_scan_results[l].neighbors[j].channel_utilization, data_stream++, 1);
					PLATFORM_MEMCPY(&channel_scan_results[l].neighbors[j].station_count, data_stream, 2);
					data_stream += 2;
				}
			}
		}
	}
}

INT8U update_stored_channel_scan_results(INT8U *data_stream)
{
	INT8U                        channel_scan_result_nr = 0;
	struct ChannelScanResultTLV *channel_scan_results   = NULL;

	struct ChannelScanRequestTLV *buffered_req = DMgetBufferedChannelScanRequest();
	if (NULL == buffered_req) {
		PLATFORM_PRINTF_WARNING("Stored Channel Scan request is null. Aborting response sending....\n");
		return 1;
	}

	//initialize and allocate memory for channel scan results
	INT8U i, j, k;
	for (i = 0; i < buffered_req->target_radio_nr; i++) {
		for (j = 0; j < buffered_req->target_radios[i].target_op_class_nr; j++) {
			for (k = 0; k < buffered_req->target_radios[i].target_op_classes[j].target_channel_nr; k++) {
				channel_scan_results                                  = (struct ChannelScanResultTLV *)PLATFORM_REALLOC(channel_scan_results, (channel_scan_result_nr + 1) * sizeof(struct ChannelScanResultTLV));
				channel_scan_results[channel_scan_result_nr].tlv_type = TLV_TYPE_CHANNEL_SCAN_RESULT;
				PLATFORM_MEMCPY(channel_scan_results[channel_scan_result_nr].radio_unique_identifier, buffered_req->target_radios[i].radio_unique_identifier, 6);
				channel_scan_results[channel_scan_result_nr].op_class                = buffered_req->target_radios[i].target_op_classes[j].op_class;
				channel_scan_results[channel_scan_result_nr].channel                 = buffered_req->target_radios[i].target_op_classes[j].target_channel_list[k];
				channel_scan_results[channel_scan_result_nr].scan_status             = 0;
				channel_scan_results[channel_scan_result_nr].timestamp_length        = 31;                   //this is always 31
				channel_scan_results[channel_scan_result_nr].timestamp               = generate_timestamp(); //for now just generate timestamp here. TODO: move this to driver whenever the scan finish for that specific channel
				channel_scan_results[channel_scan_result_nr].channel_utilization     = 0;
				channel_scan_results[channel_scan_result_nr].noise                   = 0;
				channel_scan_results[channel_scan_result_nr].neighbor_nr             = 0;
				channel_scan_results[channel_scan_result_nr].neighbors               = NULL;
				channel_scan_results[channel_scan_result_nr].aggregate_scan_duration = 300;  //can hardcode for now.
				channel_scan_results[channel_scan_result_nr].scan_type_flags         = 0x80; // hardcode active scan for now
				channel_scan_result_nr++;
			}
		}
	}

	//parse response (and buffered response if any) to fill in scan result tlvs

	//if result buffer exist, parse the result buffer stream as well.
	INT8U *result_buffer = DMgetChannelScanResultBuffer();
	if (result_buffer) {
		_fill_scan_result_tlvs(channel_scan_result_nr, channel_scan_results, result_buffer);
		DMclearChannelScanResultBuffer();
	}

	//parse result stream received
	_fill_scan_result_tlvs(channel_scan_result_nr, channel_scan_results, data_stream);

	FILE *fp = fopen(ON_BOOT_CHANNEL_SCAN_RESULT_FILE, "we");
	if (fp) {
		for (i = 0; i < channel_scan_result_nr; i++) {
			INT8U *tlv_stream;
			INT16U tlv_stream_len;
			tlv_stream = forge_1905_TLV_from_structure((INT8U *)&(channel_scan_results[i]), &tlv_stream_len, 0);
			fwrite(tlv_stream, sizeof(INT8U), tlv_stream_len, fp);
			free_1905_TLV_packet(tlv_stream);
		}
		fclose(fp);
	}

	//update database
	DMsetChannelScanResult(channel_scan_result_nr, channel_scan_results);

	return 0;
}

INT8U get_band_from_single_op_class_with_domain(INT8U op_class, int reg_domain)
{
	switch (reg_domain) {
	case DOMAIN_FCC: {
		switch (op_class) {
		case 1:
		case 2:
			return INTERFACE_BAND_5GL;
		case 3:
		case 4:
		case 5:
			return INTERFACE_BAND_5GH;
		case 12:
			return INTERFACE_BAND_2G;
		default:
			return INTERFACE_BAND_UNKNOWN;
		}
		break;
	}
	case DOMAIN_ETSI: {
		switch (op_class) {
		case 1:
		case 2:
			return INTERFACE_BAND_5GL;
		case 3:
		case 17:
			return INTERFACE_BAND_5GH;
		case 4:
			return INTERFACE_BAND_2G;
		default:
			return INTERFACE_BAND_UNKNOWN;
		}
		break;
	}
	case DOMAIN_MKK: {
		switch (op_class) {
		case 1:
		case 32:
		case 33:
			return INTERFACE_BAND_5GL;
		case 34:
		case 35:
			return INTERFACE_BAND_5GH;
		case 30:
			return INTERFACE_BAND_2G;
		default:
			return INTERFACE_BAND_UNKNOWN;
		}
		break;
	}
	case DOMAIN_CN: {
		switch (op_class) {
		case 1:
		case 2:
			return INTERFACE_BAND_5GL;
		case 3:
			return INTERFACE_BAND_5GH;
		case 7:
			return INTERFACE_BAND_2G;
		default:
			return INTERFACE_BAND_UNKNOWN;
		}
		break;
	}
	case DOMAIN_GLOBAL: {
		switch (op_class) {
		case 115:
		case 116:
		case 118:
			return INTERFACE_BAND_5GL;
		case 121:
		case 124:
		case 125:
		case 126:
			return INTERFACE_BAND_5GH;
		case 81:
		case 83:
		case 84:
			return INTERFACE_BAND_2G;
		default:
			return INTERFACE_BAND_UNKNOWN;
		}
		break;
	}
	}

	return INTERFACE_BAND_UNKNOWN;
}

INT8U get_band_from_single_op_class(INT8U op_class)
{
	switch (op_class) {
	case 115:
	case 116:
	case 118:
		return INTERFACE_BAND_5GL;
	case 121:
	case 124:
	case 125:
	case 126:
		return INTERFACE_BAND_5GH;
	case 81:
	case 83:
	case 84:
		return INTERFACE_BAND_2G;
	default:
		return INTERFACE_BAND_UNKNOWN;
	}
}

INT8U get_band_from_op_classes(struct OperatingClass *op_classes, INT8U op_class_nr)
{
	int    i      = 0;
	INT8U  band   = 0;
	INT8U  result = 0;
	INT32U reg_domain;
	PLATFORM_GET_MIB(map_radio_name_5gh, "regdomain", &reg_domain, 4);
	for (i = 0; i < op_class_nr; i++) {
		result = get_band_from_single_op_class(op_classes[i].operating_class);
		if (INTERFACE_BAND_UNKNOWN == result) {
			result = get_band_from_single_op_class_with_domain(op_classes[i].operating_class, reg_domain);
		}
		band |= result;
	}

	return band;
}

INT8U validateChannelPreferenceReport(struct ChannelPreferenceTLV p)
{
	int       op_classes_len = 0;
	OP_CLASS *op_classes     = PLATFORM_GET_OP_CLASS(&op_classes_len);

	if (NULL == op_classes) {
		PLATFORM_PRINTF_ERROR("[RTK] No operating class is available\n");
		return 1;
	}

	int index_op, index_ch, report_op, report_ch;
	for (report_op = 0; report_op < p.op_class_nr; report_op++) {
		for (index_op = 0; index_op < op_classes_len; index_op++) {
			if (op_classes[index_op].op_class == p.channel_preferences[report_op].op_class) {
				break;
			}
		}
		if (index_op == op_classes_len) {
			PLATFORM_PRINTF_DEBUG("[MULTI_AP] Unsupported OP Class %d\n", p.channel_preferences[report_op].op_class);
			//return 1;
			continue;
		}
		for (report_ch = 0; report_ch < p.channel_preferences[report_op].channel_nr; report_ch++) {
			for (index_ch = 0; index_ch < op_classes[index_op].channel_array[0]; index_ch++) {
				if (p.channel_preferences[report_op].channel_list[report_ch] == op_classes[index_op].channel_array[index_ch + 1]) {
					break;
				}
			}
			if (index_ch == op_classes[index_op].channel_array[0]) {
				PLATFORM_PRINTF_ERROR("[MULTI_AP] Invalid channel %d in OP Class %d\n", p.channel_preferences[report_op].channel_list[report_ch], p.channel_preferences[report_op].op_class);
				PLATFORM_FREE(op_classes);
				return 1;
			}
		}
	}

	PLATFORM_FREE(op_classes);

	return 0;
}

char *trimInterfaceNameVID(char *name, INT16U *vid, INT8U *port)
{
	// Initialize to avoid null term risk.
	char  name_buf[64] = { 0 };
	char *p1 = NULL, *p2 = NULL, *p3 = NULL;

	INT16U _vid                = 0;
	INT8U  _port               = 0;
	char * base_interface_name = NULL;

	if (PLATFORM_STRLEN(name) >= 64) {
		PLATFORM_PRINTF_WARNING("Unexpected interfacename %s\n", name);
		return strdup(name);
	}

	if (PLATFORM_STRSTR(name, ".")) {
		if (PLATFORM_STRLEN(name) >= sizeof(name_buf)) {
			PLATFORM_PRINTF_ERROR("name_buf size not enough, fail to trim interface name vid \n");
			return NULL;
		}
		strcpy(name_buf, name);
		p1 = strtok(name, ".");

		if (NULL == p1) {
			base_interface_name = strdup(name);
			return base_interface_name;
		}

		p2 = strtok(NULL, ".");
		if (p2)
			p3 = strtok(NULL, ".");

		if (p2)
			_vid = atoi(p2);

		if (p3) {
			_port               = _vid;
			_vid                = atoi(p3);
			if(NULL == strstr(p1, "eth") || _vid) {
			        base_interface_name = malloc(sizeof(char) * (strlen(p1) + strlen(p2) + 2));
			        sprintf(base_interface_name, "%s.%s", p1, p2);
			}

		} else if (_vid < 9) {
			_port = _vid;
			_vid  = 0;
		} else {
			base_interface_name = strdup(p1);
		}

		strlcpy(name, name_buf, strlen(name_buf) + 1);

		if (0 == _vid && NULL == base_interface_name) {
			base_interface_name = strdup(name);
		}
	} else {
		base_interface_name = strdup(name);
	}

	if (port)
		*port = _port;

	if (vid)
		*vid = _vid;

	return base_interface_name;
}

void convertToGlobalOpClass(struct CMDU *c)
{
	int    i = 0, tlv_count = 0;
	INT8U *p;
	while (NULL != (p = c->list_of_TLVs[tlv_count])) {
		switch (*p) {
		case TLV_TYPE_STEERING_REQUEST: {
			struct SteeringRequestTLV *t = (struct SteeringRequestTLV *)p;
			for (i = 0; i < t->target_bss_nr; i++) {
				t->target_bss[i].opclass = PLATFORM_CONVERT_TO_GLOBAL_OP_CLASS(t->target_bss[i].opclass);
			}
			break;
		}
		case TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES: {
			struct APRadioBasicCapabilitiesTLV *t = (struct APRadioBasicCapabilitiesTLV *)p;
			for (i = 0; i < t->operating_classes_nr; i++) {
				t->operating_classes[i].operating_class = PLATFORM_CONVERT_TO_GLOBAL_OP_CLASS(t->operating_classes[i].operating_class);
			}
			break;
		}
		case TLV_TYPE_BACKHAUL_STEERING_REQUEST: {
			struct BackhaulSteeringRequestTLV *t = (struct BackhaulSteeringRequestTLV *)p;
			t->op_class                          = PLATFORM_CONVERT_TO_GLOBAL_OP_CLASS(t->op_class);
			break;
		}
		case TLV_TYPE_BEACON_METRICS_QUERY: {
			struct BeaconMetricsQueryTLV *t = (struct BeaconMetricsQueryTLV *)p;
			t->op_class                     = PLATFORM_CONVERT_TO_GLOBAL_OP_CLASS(t->op_class);
			for (i = 0; i < t->ap_channel_report_nr; i++) {
				t->ap_channel_report[i].op_class = PLATFORM_CONVERT_TO_GLOBAL_OP_CLASS(t->ap_channel_report[i].op_class);
			}
			break;
		}
		case TLV_TYPE_BEACON_METRICS_RESPONSE: {
			struct BeaconMetricsResponseTLV *t = (struct BeaconMetricsResponseTLV *)p;
			for (i = 0; i < t->measurement_report_nr; i++) {
				t->measurement_reports[i].info.op_class = PLATFORM_CONVERT_TO_GLOBAL_OP_CLASS(t->measurement_reports[i].info.op_class);
			}
			break;
		}
		case TLV_TYPE_OPERATING_CHANNEL_REPORT: {
			struct OperatingChannelReportTLV *t = (struct OperatingChannelReportTLV *)p;
			for (i = 0; i < t->cur_op_class_nr; i++) {
				t->operating_channels[i].op_class = PLATFORM_CONVERT_TO_GLOBAL_OP_CLASS(t->operating_channels[i].op_class);
			}
			break;
		}
		case TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY: {
			struct UnassociatedSTALinkMetricsQueryTLV *t = (struct UnassociatedSTALinkMetricsQueryTLV *)p;
			t->op_class                                  = PLATFORM_CONVERT_TO_GLOBAL_OP_CLASS(t->op_class);
			break;
		}
		case TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE: {
			struct UnassociatedSTALinkMetricsResponseTLV *t = (struct UnassociatedSTALinkMetricsResponseTLV *)p;
			t->op_class                                     = PLATFORM_CONVERT_TO_GLOBAL_OP_CLASS(t->op_class);
			break;
		}
		case TLV_TYPE_RADIO_OPERATION_RESTRICTION: {
			struct RadioOperationRestrictionTLV *t = (struct RadioOperationRestrictionTLV *)p;
			for (i = 0; i < t->op_class_nr; i++) {
				t->radio_operation_restriction[i].op_class = PLATFORM_CONVERT_TO_GLOBAL_OP_CLASS(t->radio_operation_restriction[i].op_class);
			}
			break;
		}
		case TLV_TYPE_CHANNEL_PREFERENCE: {
			struct ChannelPreferenceTLV *t = (struct ChannelPreferenceTLV *)p;
			for (i = 0; i < t->op_class_nr; i++) {
				t->channel_preferences[i].op_class = PLATFORM_CONVERT_TO_GLOBAL_OP_CLASS(t->channel_preferences[i].op_class);
			}
			break;
		}
		default: {
			break;
		}
		}
		tlv_count++;
	}

	return;
}

void convertToLocalOpClass(struct CMDU *c)
{
	int    i = 0, tlv_count = 0;
	INT8U *p;
	while (NULL != (p = c->list_of_TLVs[tlv_count])) {
		switch (*p) {
		case TLV_TYPE_STEERING_REQUEST: {
			struct SteeringRequestTLV *t = (struct SteeringRequestTLV *)p;
			for (i = 0; i < t->target_bss_nr; i++) {
				t->target_bss[i].opclass = PLATFORM_CONVERT_TO_LOCAL_OP_CLASS(t->target_bss[i].opclass);
			}
			break;
		}
		case TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES: {
			struct APRadioBasicCapabilitiesTLV *t = (struct APRadioBasicCapabilitiesTLV *)p;
			for (i = 0; i < t->operating_classes_nr; i++) {
				t->operating_classes[i].operating_class = PLATFORM_CONVERT_TO_LOCAL_OP_CLASS(t->operating_classes[i].operating_class);
			}
			break;
		}
		case TLV_TYPE_BACKHAUL_STEERING_REQUEST: {
			struct BackhaulSteeringRequestTLV *t = (struct BackhaulSteeringRequestTLV *)p;
			t->op_class                          = PLATFORM_CONVERT_TO_LOCAL_OP_CLASS(t->op_class);
			break;
		}
		case TLV_TYPE_BEACON_METRICS_QUERY: {
			struct BeaconMetricsQueryTLV *t = (struct BeaconMetricsQueryTLV *)p;
			t->op_class                     = PLATFORM_CONVERT_TO_LOCAL_OP_CLASS(t->op_class);
			for (i = 0; i < t->ap_channel_report_nr; i++) {
				t->ap_channel_report[i].op_class = PLATFORM_CONVERT_TO_LOCAL_OP_CLASS(t->ap_channel_report[i].op_class);
			}
			break;
		}
		case TLV_TYPE_BEACON_METRICS_RESPONSE: {
			struct BeaconMetricsResponseTLV *t = (struct BeaconMetricsResponseTLV *)p;
			for (i = 0; i < t->measurement_report_nr; i++) {
				t->measurement_reports[i].info.op_class = PLATFORM_CONVERT_TO_LOCAL_OP_CLASS(t->measurement_reports[i].info.op_class);
			}
			break;
		}
		case TLV_TYPE_OPERATING_CHANNEL_REPORT: {
			struct OperatingChannelReportTLV *t = (struct OperatingChannelReportTLV *)p;
			for (i = 0; i < t->cur_op_class_nr; i++) {
				t->operating_channels[i].op_class = PLATFORM_CONVERT_TO_LOCAL_OP_CLASS(t->operating_channels[i].op_class);
			}
			break;
		}
		case TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_QUERY: {
			struct UnassociatedSTALinkMetricsQueryTLV *t = (struct UnassociatedSTALinkMetricsQueryTLV *)p;
			t->op_class                                  = PLATFORM_CONVERT_TO_LOCAL_OP_CLASS(t->op_class);
			break;
		}
		case TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE: {
			struct UnassociatedSTALinkMetricsResponseTLV *t = (struct UnassociatedSTALinkMetricsResponseTLV *)p;
			t->op_class                                     = PLATFORM_CONVERT_TO_LOCAL_OP_CLASS(t->op_class);
			break;
		}
		case TLV_TYPE_RADIO_OPERATION_RESTRICTION: {
			struct RadioOperationRestrictionTLV *t = (struct RadioOperationRestrictionTLV *)p;
			for (i = 0; i < t->op_class_nr; i++) {
				t->radio_operation_restriction[i].op_class = PLATFORM_CONVERT_TO_LOCAL_OP_CLASS(t->radio_operation_restriction[i].op_class);
			}
			break;
		}
		case TLV_TYPE_CHANNEL_PREFERENCE: {
			struct ChannelPreferenceTLV *t = (struct ChannelPreferenceTLV *)p;
			for (i = 0; i < t->op_class_nr; i++) {
				t->channel_preferences[i].op_class = PLATFORM_CONVERT_TO_LOCAL_OP_CLASS(t->channel_preferences[i].op_class);
			}
			break;
		}
		default: {
			break;
		}
		}
		tlv_count++;
	}

	return;
}

int channel_to_center_frequency_index(INT8U channel, INT8U bandwidth, INT8U sideband)
{
	INT8U center_channel = 0;
	if (BAND_WIDTH_20MHZ == bandwidth) {
		center_channel = channel;
	} else if (BAND_WIDTH_40MHZ == bandwidth) {
		if (SIDE_BAND_UPPER == sideband) {
			// PrimaryChannelUpperBehavior
			center_channel = channel - 2;
		} else if (SIDE_BAND_LOWER == sideband) {
			// PrimaryChannelLowerBehavior
			center_channel = channel + 2;
		}
	} else if (BAND_WIDTH_80MHZ == bandwidth) {
		center_channel = get_channel_center_frequency_index(channel);
	} else {
		return 0;
	}

	// if (INTERFACE_BAND_2G == band) {
	//      return 2407 + center_channel * 5;
	// }

	// if (INTERFACE_BAND_5GL & band || INTERFACE_BAND_5GH & band) {
	//      return 5000 + center_channel * 5;
	// }

	// return 0;
	return center_channel;
}

static INT8U get_radio_band_info_from_interface_name(char *interface_name, char **radio_name, INT8U *band)
{
	if (PLATFORM_STRSTR(interface_name, map_radio_name_5gl) && PLATFORM_STRSTR(interface_name, map_radio_name_5gh)) {
		*band       = INTERFACE_BAND_5GL | INTERFACE_BAND_5GH;
		*radio_name = map_radio_name_5gl;
	} else if (PLATFORM_STRSTR(interface_name, map_radio_name_5gh)) {
		*band       = INTERFACE_BAND_5GH;
		*radio_name = map_radio_name_5gh;
	} else if (PLATFORM_STRSTR(interface_name, map_radio_name_5gl)) {
		*band       = INTERFACE_BAND_5GL;
		*radio_name = map_radio_name_5gl;
	} else if (PLATFORM_STRSTR(interface_name, map_radio_name_24g)) {
		*band       = INTERFACE_BAND_2G;
		*radio_name = map_radio_name_24g;
	} else {
		*radio_name = NULL;
		*band       = INTERFACE_BAND_UNKNOWN;
		return 1;
	}

	return 0;
}

int get_center_frequency(char *interface_name)
{
	INT8U band       = 0;
	char *radio_name = NULL;
	INT8U channel;
	int   bandwidth, sideband = 0;

	if (get_radio_band_info_from_interface_name(interface_name, &radio_name, &band))
		return 0;

	if (NULL == radio_name)
		return 0;

	if (obtain_channel_info(radio_name, &channel, &bandwidth, &sideband, NULL))
		return 0;

	return channel_to_center_frequency_index(channel, bandwidth, sideband);
}

INT8U get_channel_center_frequency_index(INT8U channel)
{
	switch (channel) {
	case 36:
	case 40:
	case 44:
	case 48:
		return 42;
	case 52:
	case 56:
	case 60:
	case 64:
		return 58;
	case 100:
	case 104:
	case 108:
	case 112:
		return 106;
	case 116:
	case 120:
	case 124:
	case 128:
		return 122;
	case 132:
	case 136:
	case 140:
	case 144:
		return 138;
	case 149:
	case 153:
	case 157:
	case 161:
		return 155;
	default:
		return 0;
	}
}

INT8U get_160m_channel_center_frequency_index(INT8U channel)
{
	switch (channel) {
	case 36:
	case 40:
	case 44:
	case 48:
	case 52:
	case 56:
	case 60:
	case 64:
		return 50;
	case 100:
	case 104:
	case 108:
	case 112:
	case 116:
	case 120:
	case 124:
	case 128:
		return 114;
	case 149:
	case 153:
	case 157:
	case 161:
	case 165:
	case 169:
	case 173:
	case 177:
		return 163;
	default:
		return 0;
	}
}

INT8U obtain_channel_info(char *radio_name, INT8U *channel, int *bandwidth, int *sideband, INT8U *transmit_power)
{
	if (0 != PLATFORM_GET_MIB(radio_name, "channel", channel, 1)) {
		PLATFORM_PRINTF_ERROR("Can not get channel for interface %s\n", radio_name);
		return 1;
	}
	PLATFORM_PRINTF_TRACE("[RTK] [%s] channel: %d\n", radio_name, *channel);

	INT8U is_ax_support = 0;
	is_ax_support       = PLATFORM_INTERFACE_HAS_AX_SUPPORT(radio_name);

	if (is_ax_support) {
		INT8U bw = 0, sb = 0;
		// Driver channel_width enum is same as MAP_BAND_WIDTH
		if (0 != PLATFORM_GET_MIB(radio_name, "use40M", &bw, 1)) {
			PLATFORM_PRINTF_ERROR("Can not get bandwidth for interface %s\n", radio_name);
			return 1;
		}
		*bandwidth = bw;
		PLATFORM_PRINTF_TRACE("[RTK][%s] [%s] bandwidth: %d\n", __FUNCTION__, radio_name, *bandwidth);

		if (0 != PLATFORM_GET_MIB(radio_name, "2ndchoffset", &sb, 1)) {
			PLATFORM_PRINTF_ERROR("Can not get sideband for interface %s\n", radio_name);
			return 1;
		}

		if (1 == sb) {
			// PrimaryChannelUpperBehavior
			*sideband = SIDE_BAND_UPPER;
		} else if (2 == sb) {
			// PrimaryChannelLowerBehavior
			*sideband = SIDE_BAND_LOWER;
		}
	} else {
		// Driver channel_width enum is same as MAP_BAND_WIDTH
		if (0 != PLATFORM_GET_MIB(radio_name, "use40M", bandwidth, 4)) {
			PLATFORM_PRINTF_ERROR("Can not get bandwidth for interface %s\n", radio_name);
			return 1;
		}
		PLATFORM_PRINTF_TRACE("[RTK][%s] [%s] bandwidth: %d\n", __FUNCTION__, radio_name, *bandwidth);

		//
		if (0 != PLATFORM_GET_MIB(radio_name, "2ndchoffset", sideband, 4)) {
			PLATFORM_PRINTF_ERROR("Can not get sideband for interface %s\n", radio_name);
			return 1;
		}

		if (1 == *sideband) {
			// PrimaryChannelUpperBehavior
			*sideband = SIDE_BAND_UPPER;
		} else if (2 == *sideband) {
			// PrimaryChannelLowerBehavior
			*sideband = SIDE_BAND_LOWER;
		}
	}
	PLATFORM_PRINTF_TRACE("[RTK] [%s] sideband: %d\n", radio_name, *sideband);

	if (transmit_power) {
		if (0 != PLATFORM_GET_MIB(radio_name, "tpc_tx_power", transmit_power, 1)) {
			PLATFORM_PRINTF_ERROR("Can not get transmit power for interface %s\n", radio_name);
			return 1;
		}
		PLATFORM_PRINTF_TRACE("[RTK] [%s] transmit power: %d\n", radio_name, *transmit_power);
	}

	return 0;
}

#ifdef RTK_MULTI_AP_R2
INT8U checkTargetDeviceProfile2TLVCapability(INT8U *target_al_mac)
{
	uint8_t  mcast_address[] = MCAST_1905;
	uint64_t device_version  = 0;
	if (MULTI_AP_PROFILE_2 > DMgetMultiAPProfile()) {
		return 0;
	}
	if (0 == PLATFORM_MEMCMP(mcast_address, target_al_mac, 6) || MULTI_AP_PROFILE_2 <= DMgetTargetDeviceMultiAPProfile(target_al_mac)) {
		return 1;
	}
	device_version = DMgetNetworkDeviceVersion(target_al_mac);
	if (device_version == 0 || (device_version < V_1_3_F_RELEASE_VERSION)) {
		return 0;
	}
	return 1;
}
#endif

void sscanf_hex(INT8U *src, INT8U *hex, INT16U len)
{
	const char *pos = (const char *)src;
	int         count;
	for (count = 0; count < len; count++) {
		sscanf(pos, "%02hhx", &hex[count]);
		pos += 2;
	}
}

void encap_dpp_frame(INT8U *dpp_frame, INT16U dpp_frame_len, INT8U* enrollee_mac, struct Encap1905DPPTLV *encap_1905_dpp_tlv)
{
	PLATFORM_MEMSET(encap_1905_dpp_tlv, 0, sizeof(struct Encap1905DPPTLV));
	encap_1905_dpp_tlv->tlv_type = TLV_TYPE_1905_ENCAP_DPP;
	encap_1905_dpp_tlv->enrollee_mac_address_present |= BIT(7);
	PLATFORM_MEMCPY(encap_1905_dpp_tlv->dest_sta_mac_address, enrollee_mac, 6);

	if (WLAN_PA_VENDOR_SPECIFIC != dpp_frame[0]) {
		encap_1905_dpp_tlv->enrollee_mac_address_present |= BIT(5);
		encap_1905_dpp_tlv->frame_type = 0xFF;
	} else {
		encap_1905_dpp_tlv->frame_type = dpp_frame[6];
	}

	encap_1905_dpp_tlv->encapsulated_frame_len = dpp_frame_len;
	encap_1905_dpp_tlv->encapsulated_frame     = (INT8U *)PLATFORM_MALLOC(dpp_frame_len);

	PLATFORM_MEMCPY(encap_1905_dpp_tlv->encapsulated_frame, dpp_frame, dpp_frame_len);
}

char *mac2str(INT8U *mac_addr)
{
	static char mac_str[18];
	PLATFORM_SNPRINTF(mac_str, 18, "%02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
	return mac_str;
}

INT8U rssi2rcpi(INT8U rssi)
{
	//convert per 100 to per 220
	return (2 * (10 + rssi));
}

INT8U rcpi2rssi(INT8U rcpi)
{
	if (rcpi < 20) {
		return 0;
	}
	//convert per 220 to per 110
	return (rcpi / 2 - 10);
}
