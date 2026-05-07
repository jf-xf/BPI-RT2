/*
 * Copyright (C) 2018 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _API_RESPONSE_H_
#define _API_RESPONSE_H_

#include <mqueue.h>
#include "map_initialization.h"

#define CONTROLLER_LOST 1
#define CONTROLLER_FOUND 2


INT8U api_set_response_queue(const char *mq_name);

INT8U api_close_request_queue();

void api_set_request_queue(INT8U mq_id);

INT8U api_send_raw_request_message(INT8U *message, INT16U msg_len);

INT8U api_send_raw_response_message(INT8U *message, INT16U msg_len);

INT8U api_send_raw_response(INT8U* message, INT16U msg_len);

INT8U api_send_general_acknowledgement(INT8U request_type, INT8U dest_mac[6], INT8U status);

INT8U api_send_tlvs_response(INT8U element_id, INT8U src_mac[6], INT8U **tlv_list);

INT8U api_send_single_tlv_response(INT8U element_id, INT8U src_mac[6], INT8U *tlv);

INT8U api_send_config_request(struct radio_config_data radio_config);

INT8U api_controller_change_notification(INT8U event_type, char* interface_name, INT8U *controller_al_mac);

INT8U api_send_customized_status_notification(INT8U status);

INT8U api_send_dpp_configuration(char * signed_connector_1905, char *netaccess_key, char *csignkey, char *ppkey);

INT8U api_set_recv_callback(INT16U cmdu_type, void (*callback_func)(INT8U **tlvs));

INT8U api_set_append_tlv_callback(INT16U cmdu_type, void (*callback_func)(INT8U *vendor_tlv));

INT8U api_get_append_tlv_callback(INT16U cmdu_type, void (**callback_func)(INT8U *tlv));

INT8U api_trigger_callback(INT16U cmdu_type, INT8U **tlvs);

INT8U api_send_de_tlvs_response(INT16U cmdu_type, INT8U src_mac[6], INT8U **tlv_list);

INT8U send_packet_to_de(INT8U *buffer, int buffer_len);

INT8U send_to_dpp_broker(INT8U *buffer, INT32U buffer_len);

#endif
