/*
 * Copyright (C) 2018 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#include <mqueue.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "packet_tools.h"
#include "api_response.h"
#include "map_utils.h"
#include "1905_tlvs.h"
#include "multi_ap_tlvs.h"

#define MAX_MESSAGE_SIZE 12288

mqd_t _response_mq = -1;

mqd_t _request_mq = -1;

#define MAP_MAIN_MQUEUE_NAME "/MAP_AL_events"

#define MAP_DPP_BROKER_SOCKET_PATH "/tmp/map_dpp_relay"
#define SOCKET_PATH "/tmp/de_map_hub"
#define BUFFER_LENGTH 4096

#ifndef O_CLOEXEC
#define O_CLOEXEC	02000000	/* set close_on_exec */
#endif

INT8U api_set_response_queue(const char *mq_name)
{
	if ((mqd_t)-1 == (_response_mq = mq_open(mq_name, O_WRONLY | O_CLOEXEC))) {
		printf("mq_open('%s') returned with errno=%d (%s)\n", mq_name, errno, strerror(errno));
		return 1;
	}
	return 0;
}

INT8U _api_open_request_queue()
{
	if ((mqd_t)-1 == (_request_mq = mq_open(MAP_MAIN_MQUEUE_NAME, O_WRONLY | O_CLOEXEC))) {
		printf("mq_open('%s') returned with errno=%d (%s)\n", MAP_MAIN_MQUEUE_NAME, errno, strerror(errno));
		return 1;
	}
	return 0;
}

INT8U api_close_request_queue()
{
	mq_close(_request_mq);
	_request_mq = -1;
	return 0;
}

INT8U api_send_raw_request_message(INT8U *message, INT16U msg_len)
{
	if (-1 == _request_mq) {
		_api_open_request_queue();
	}
	INT8U buffer[BUFFER_LENGTH];

	if ((3 + msg_len) > BUFFER_LENGTH) {
		PLATFORM_PRINTF_WARNING("Drop message, message size > 4096");
		api_close_request_queue();
		return 1;
	}

	INT8U *pt = &buffer[0];

	INT8U message_type = MAP_API_REQUEST;

	if (NULL == message) {
		api_close_request_queue();
		return 1;
	}

	_I1B(&message_type, &pt);

	_I2B(&msg_len, &pt);

	_InB(message, &pt, msg_len);

	free(message);

	//printf("MAP API Request\n");
	//printf("Element id: %02x \n", message[0]);
	//printf("Send %d bytes to map mq\n", (3 + msg_len));
	// int i = 0;
	// for (i = 0; i < (3 + msg_len); i++) {
	//printf("%02x ", buffer[i]);
	// }
	//printf("\n");

	struct mq_attr attr;
	mq_getattr(_request_mq, &attr);
	if (attr.mq_curmsgs >= 80) {
		api_close_request_queue();
		return 1;
	}

	if (0 != mq_send(_request_mq, (const char *)buffer, (3 + msg_len), 0)) {
		api_close_request_queue();
		return 1;
	}

	api_close_request_queue();

	return 0;
}

INT8U api_send_raw_response_message(INT8U *message, INT16U msg_len)
{
	INT8U *buffer = PLATFORM_MALLOC(MAX_MESSAGE_SIZE);

	INT8U *pt = &buffer[0];

	INT8U message_type = MAP_API_RESPONSE;

	if (NULL == message) {
		PLATFORM_FREE(buffer);
		return 1;
	}
	_I1B(&message_type, &pt);
	_I2B(&msg_len, &pt);
	_InB(message, &pt, msg_len);

	//printf("MAP API Response\n");
	//printf("Element id: %02x \n", message[0]);
	//printf("Send %d bytes to user mq\n", (3 + msg_len));
	// int i = 0;
	// for (i = 0; i < (3 + msg_len); i++) {
	//printf("%02x ", buffer[i]);
	// }
	//printf("\n");

	struct mq_attr attr;
	mq_getattr(_response_mq, &attr);
	if (attr.mq_curmsgs >= 50) {
		PLATFORM_FREE(buffer);
		return 1;
	}
	if (0 != mq_send(_response_mq, (const char *)buffer, (3 + msg_len), 0)) {
		PLATFORM_FREE(buffer);
		return 1;
	}
	PLATFORM_FREE(buffer);
	return 0;
}

INT8U api_controller_change_notification(INT8U event_type, char *interface_name, INT8U *controlle_al_mac)
{
	INT8U  response[40];
	INT8U *pt                 = &response[1];
	INT8U  interface_name_len = 0;

	if (NULL != interface_name) {
		interface_name_len = PLATFORM_STRLEN(interface_name);
	}

	response[0] = MAP_CONTROLLER_CHANGE_NOTIFICATION_ELEMENT;

	_I1B(&event_type, &pt);

	if (NULL != interface_name) {
		_InB(controlle_al_mac, &pt, 6);
	} else {
		INT8U null_controller[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		_InB(null_controller, &pt, 6);
	}

	_I1B(&interface_name_len, &pt);

	if (0 != interface_name_len)
		_InB(interface_name, &pt, interface_name_len);

	api_send_raw_response_message(response, 3 + 6 + interface_name_len);
	return 0;
}

INT8U api_send_customized_status_notification(INT8U status)
{
	INT8U  response[2];
	response[0] = MAP_CUSTOMIZED_STATUS_NOTIFICATION_ELEMENT;
	response[1] = status;
	api_send_raw_response_message(response, 2);
	return 0;
}

INT8U api_send_dpp_configuration(char * signed_connector_1905, char *netaccess_key, char *csign_key, char *pp_key)
{
	INT8U *buffer = PLATFORM_MALLOC(MAX_MESSAGE_SIZE);
	buffer[0]     = MAP_DPP_CONFIGURATION_ELEMENT;
	INT8U *pt     = &buffer[1];
	INT16U aux16;
	int    len = 1;

	aux16 = strlen(signed_connector_1905);
	_I2B(&aux16, &pt);
	_InB(signed_connector_1905, &pt, aux16);
	len += 2 + aux16;

	aux16 = strlen(netaccess_key);
	_I2B(&aux16, &pt);
	_InB(netaccess_key, &pt, aux16);
	len += 2 + aux16;

	aux16 = strlen(csign_key);
	_I2B(&aux16, &pt);
	_InB(csign_key, &pt, aux16);
	len += 2 + aux16;

	if (pp_key) {
		aux16 = strlen(pp_key);
	}
	else
		aux16 = 0;

	_I2B(&aux16, &pt);

	if (pp_key)
		_InB(pp_key, &pt, aux16);

	len += 2 + aux16;

	api_send_raw_response_message(buffer, len);
	PLATFORM_FREE(buffer);
	return 0;
}

INT8U api_send_general_acknowledgement(INT8U request_type, INT8U dest_mac[6], INT8U status)
{
	INT8U  response[9];
	INT8U *pt   = &response[1];
	response[0] = MAP_GENERAL_ACKNOWLEDGEMENT_ELEMENT;

	_I1B(&request_type, &pt);
	_InB(dest_mac, &pt, 6);
	_I1B(&status, &pt);
	api_send_raw_response_message(response, 9);
	return 0;
}

INT8U api_send_tlvs_response(INT8U element_id, INT8U src_mac[6], INT8U **tlv_list)
{
	INT8U *p;
	INT8U *response = PLATFORM_MALLOC(MAX_MESSAGE_SIZE);
	int    i        = 0;
	int    len      = 0;

	INT8U *tlv_stream;
	INT16U tlv_stream_len = 0;

	response[0] = element_id;
	len += 1;

	PLATFORM_MEMCPY(&response[1], src_mac, 6);
	len += 6;

	while (NULL != (p = tlv_list[i])) {
		// printf("TLV %d %02x\n", i, *p);
		tlv_stream = forge_1905_TLV_from_structure(p, &tlv_stream_len, 0);
		if (tlv_stream_len + len > MAX_MESSAGE_SIZE) {
			PLATFORM_PRINTF_ERROR("Message is too large, exceeded buffer size!\n");
			PLATFORM_FREE(tlv_stream);
			PLATFORM_FREE(response);
			return 1;
		}
		// int j      = 0;
		// for (j = 0; j < tlv_stream_len; j++) {
		// 	printf("%02x ", tlv_stream[j]);
		// }
		// printf("\n\n");
		if (!tlv_stream) {
			PLATFORM_PRINTF_WARNING("tlv_stream is NULL!\n");
			i++;
			continue;
		}
		memcpy(&response[len], tlv_stream, tlv_stream_len);
		len += tlv_stream_len;
		PLATFORM_FREE(tlv_stream);
		i++;
	}
	response[len++] = 0x00;
	response[len++] = 0x00;
	response[len++] = 0x00;

	//printf("Send %d bytes to response, element id: %02x\n", len, element_id);
	api_send_raw_response_message(response, len);
	PLATFORM_FREE(response);
	return 0;
}

INT8U api_send_single_tlv_response(INT8U element_id, INT8U src_mac[6], INT8U *tlv)
{
	INT8U *p        = tlv;
	INT8U *response = PLATFORM_MALLOC(MAX_MESSAGE_SIZE);
	int    len      = 0;
	INT8U *tlv_stream;
	INT16U tlv_stream_len = 0;

	response[0] = element_id;
	len += 1;

	PLATFORM_MEMCPY(&response[1], src_mac, 6);
	len += 6;

	if (p != NULL) {
		tlv_stream = forge_1905_TLV_from_structure(p, &tlv_stream_len, 0);
		if (tlv_stream_len + len > MAX_MESSAGE_SIZE) {
			PLATFORM_PRINTF_ERROR("Message is too large, exceeded buffer size!\n");
			PLATFORM_FREE(tlv_stream);
			PLATFORM_FREE(response);
			return 1;
		}
		memcpy(&response[len], tlv_stream, tlv_stream_len);
		len += tlv_stream_len;
		PLATFORM_FREE(tlv_stream);
	}
	response[len++] = 0x00;
	response[len++] = 0x00;
	response[len++] = 0x00;

	api_send_raw_response_message(response, len);
	PLATFORM_FREE(response);
	return 0;
}

INT8U api_send_config_request(struct radio_config_data radio_config)
{
	INT8U *response = PLATFORM_MALLOC(MAX_MESSAGE_SIZE);
	INT8U *pt       = &response[1];
	int    i        = 0;
	int    len      = 0;
	INT8U  aux8     = 0;
	INT16U aux16    = 0;

	response[0] = MAP_CONFIGURE_NOTIFICATION_ELEMENT;
	len += 1;

	// Fixed radio config number 1
	aux8 = 1;
	_I1B(&aux8, &pt);
	len += 1;

	_I1B(&radio_config.radio_band, &pt);

	_I1B(&radio_config.bss_data_nr, &pt);

	len += 2;

	for (i = 0; i < radio_config.bss_data_nr; i++) {
		aux8 = (INT8U)strlen(radio_config.bss_data[i].ssid);
		_I1B(&aux8, &pt);
		_InB(radio_config.bss_data[i].ssid, &pt, strlen(radio_config.bss_data[i].ssid));
		len += (1 + aux8);
		aux8 = (INT8U)strlen(radio_config.bss_data[i].network_key);
		_I1B(&aux8, &pt);
		_InB(radio_config.bss_data[i].network_key, &pt, strlen(radio_config.bss_data[i].network_key));
		len += (1 + aux8);
		_I1B(&(radio_config.bss_data[i].authentication_type), &pt);
		_I1B(&(radio_config.bss_data[i].encryption_type), &pt);
		_I1B(&(radio_config.bss_data[i].network_type), &pt);
		_I2B(&radio_config.bss_data[i].vid, &pt);
		len += 5;
		if (radio_config.bss_data[i].signed_connector) {
			aux16 = strlen(radio_config.bss_data[i].signed_connector);
		} else {
			aux16 = 0;
		}
		_I2B(&aux16, &pt);
		len += 2;
		if (radio_config.bss_data[i].signed_connector) {
			_InB(radio_config.bss_data[i].signed_connector, &pt, aux16);
			len += aux16;
		}
	}

	_I1B(&radio_config.vendor_data_nr, &pt);
	len += 1;

	for (i = 0; i < radio_config.vendor_data_nr; i++) {
		_InB(radio_config.vendor_datas[i].vendor_oui, &pt, 3);
		_I2B(&radio_config.vendor_datas[i].payload_len, &pt);
		_InB(radio_config.vendor_datas[i].payload, &pt, radio_config.vendor_datas[i].payload_len);
		len += 3 + 2 + radio_config.vendor_datas[i].payload_len;
	}
	api_send_raw_response_message(response, len);
	PLATFORM_FREE(response);
	return 0;
}
struct cmdu_callbacks {
	INT8U cmdu_type;
	void (*callback)(INT8U **tlvs);
};

struct cmdu_callbacks *callback_info    = NULL;
INT8U                  callback_info_nr = 0;

INT8U api_set_recv_callback(INT16U cmdu_type, void (*callback_func)(INT8U **tlvs))
{
	int i = 0;

	if (NULL == callback_func) {
		// PLATFORM_PRINTF_DEBUG("[Callback API] %s callback set failed for %04x.\n", __FUNCTION__, cmdu_type);
		return 1;
	}

	for (i = 0; i < callback_info_nr; i++) {
		if (callback_info[i].cmdu_type == cmdu_type) {
			callback_info[i].callback = callback_func;
			// PLATFORM_PRINTF_DEBUG("[Callback API] callback replaced for %04x.\n", cmdu_type);
			return 2;
		}
	}

	callback_info_nr += 1;
	callback_info                                 = PLATFORM_REALLOC(callback_info, sizeof(struct cmdu_callbacks) * callback_info_nr);
	callback_info[callback_info_nr - 1].cmdu_type = cmdu_type;
	callback_info[callback_info_nr - 1].callback  = callback_func;

	return 0;
}

struct cmdu_append_tlv_callbacks {
	INT8U cmdu_type;
	void (*append_tlv_callback)(INT8U *vendor_tlv);
};

struct cmdu_append_tlv_callbacks *append_tlv_callback_info    = NULL;
INT8U                  append_tlv_callback_info_nr = 0;

INT8U api_set_append_tlv_callback(INT16U cmdu_type, void (*callback_func)(INT8U *vendor_tlv))
{
	int i = 0;

	if (NULL == callback_func) {
		return 1;
	}

	for (i = 0; i < append_tlv_callback_info_nr; i++) {
		if (append_tlv_callback_info[i].cmdu_type == cmdu_type) {
			append_tlv_callback_info[i].append_tlv_callback = callback_func;
			// PLATFORM_PRINTF_DEBUG("[Callback API] callback replaced for %04x.\n", cmdu_type);
			return 2;
		}
	}

	append_tlv_callback_info_nr += 1;
	append_tlv_callback_info                                 = PLATFORM_REALLOC(append_tlv_callback_info, sizeof(struct cmdu_append_tlv_callbacks) * append_tlv_callback_info_nr);
	append_tlv_callback_info[append_tlv_callback_info_nr - 1].cmdu_type = cmdu_type;
	append_tlv_callback_info[append_tlv_callback_info_nr - 1].append_tlv_callback  = callback_func;
	// PLATFORM_PRINTF_DEBUG("[Callback API] successfully set the append_tlv_callback function for cmdu type %04x.\n", cmdu_type);
	return 0;
}

INT8U api_get_append_tlv_callback(INT16U cmdu_type, void (**callback_func)(INT8U *tlv))
{
	int i = 0;
	for (i = 0; i < append_tlv_callback_info_nr; i++) {
		if (append_tlv_callback_info[i].cmdu_type == cmdu_type) {
			*callback_func = append_tlv_callback_info[i].append_tlv_callback;
			// PLATFORM_PRINTF_TRACE("[Callback API] %s get callback for %04x.\n", __FUNCTION__, cmdu_type);
			return 0;
		}
	}

	return 1;
}

INT8U _api_get_callback(INT16U cmdu_type, void (**callback_func)(INT8U **tlvs))
{
	int i = 0;
	for (i = 0; i < callback_info_nr; i++) {
		if (callback_info[i].cmdu_type == cmdu_type) {
			*callback_func = callback_info[i].callback;
			// PLATFORM_PRINTF_TRACE("[Callback API] %s get callback for %04x.\n", __FUNCTION__, cmdu_type);
			return 0;
		}
	}

	return 1;
}

INT8U api_trigger_callback(INT16U cmdu_type, INT8U **tlvs)
{
	void (*callback_func)(INT8U **) = NULL;

	_api_get_callback(cmdu_type, &callback_func);

	if (callback_func) {
		// PLATFORM_PRINTF_TRACE("[Callback API] %s get receive callback for %04x.\n", __FUNCTION__, cmdu_type);
		callback_func(tlvs);
	}
	return 0;
}

INT8U send_packet_to_de(INT8U *buffer, int buffer_len)
{
	int                sock_fd = -1, ret;
	struct sockaddr_un serveraddr;

	sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		PLATFORM_PRINTF_ERROR("socket() failed\n");
		return 1;
	}

	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sun_family = AF_UNIX;
	strcpy(serveraddr.sun_path, SOCKET_PATH);

	ret = connect(sock_fd, (struct sockaddr *)&serveraddr, SUN_LEN(&serveraddr));

	if (ret < 0) {
		// PLATFORM_PRINTF_ERROR("connect() failed\n");
		close(sock_fd);
		return 1;
	}

	ret = send(sock_fd, buffer, buffer_len, 0);
	if (ret < 0) {
		PLATFORM_PRINTF_ERROR("send() failed\n");
		close(sock_fd);
		return 1;
	}

	/***********************************************************************/
	/* Close down any open socket descriptors                              */
	/***********************************************************************/
	if (sock_fd != -1)
		close(sock_fd);

	return 0;
}

INT8U api_send_de_tlvs_response(INT16U cmdu_type, INT8U src_mac[6], INT8U **tlv_list)
{
	INT8U *p;
	INT8U *response = PLATFORM_MALLOC(MAX_MESSAGE_SIZE);
	int    i        = 0;
	int    len      = 3;
	INT16U aux16;
	INT8U *pt = &response[1];

	INT8U *tlv_stream;
	INT16U tlv_stream_len = 0;

	response[0] = MAP_CMDU_TLV_ELEMENT;

	pt = &response[3];
	_I2B(&cmdu_type, &pt);
	pt = &response[1];
	len += 2;

	PLATFORM_MEMCPY(&response[len], src_mac, 6);
	len += 6;

	while (NULL != (p = tlv_list[i])) {
		// printf("TLV %d %02x\n", i, *p);
		tlv_stream = forge_1905_TLV_from_structure(p, &tlv_stream_len, 0);
		if (tlv_stream_len + len > MAX_MESSAGE_SIZE) {
			PLATFORM_PRINTF_ERROR("Message is too large, exceeded buffer size!\n");
			PLATFORM_FREE(tlv_stream);
			PLATFORM_FREE(response);
			return 1;
		}
		// int j      = 0;
		// for (j = 0; j < tlv_stream_len; j++) {
		// 	printf("%02x ", tlv_stream[j]);
		// }
		// printf("\n\n");
		if (!tlv_stream) {
			PLATFORM_PRINTF_WARNING("tlv_stream is NULL!\n");
			i++;
			continue;
		}
		memcpy(&response[len], tlv_stream, tlv_stream_len);
		len += tlv_stream_len;
		PLATFORM_FREE(tlv_stream);
		i++;
	}
	response[len++] = 0x00;
	response[len++] = 0x00;
	response[len++] = 0x00;

	aux16 = len-3;
	_I2B(&aux16, &pt);
	//printf("Send %d bytes to response, element id: %02x\n", len, element_id);
	send_packet_to_de(response, len+3);
	PLATFORM_FREE(response);
	return 0;
}

INT8U send_to_dpp_broker(INT8U *buffer, INT32U buffer_len)
{
	int                sock_fd = -1;
	struct sockaddr_un serveraddr;
	int                flags;

	sock_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sock_fd < 0) {
		return -1;
	}

	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sun_family = AF_UNIX;
	strcpy(serveraddr.sun_path, MAP_DPP_BROKER_SOCKET_PATH);

	/*
	 * Make socket non-blocking so that we don't hang forever if
	 * target dies unexpectedly.
	 */
	flags = fcntl(sock_fd, F_GETFL);
	if (flags >= 0) {
		flags |= O_NONBLOCK;
		if (fcntl(sock_fd, F_SETFL, flags) < 0) {
			perror("fcntl(ctrl->s, O_NONBLOCK)");
			/* Not fatal, continue on.*/
		}
	}

	if (sendto(sock_fd, buffer, buffer_len, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
		close(sock_fd);
		return -1;
	}

	close(sock_fd);

	return 0;
}