#ifndef _EASYMESH_RELAY_H
#define _EASYMESH_RELAY_H

#include "dpp_datamodel.h"

void relay_send_chirp_notification(const uint8_t *r_bootstrap, uint16_t r_bootstrap_len);

int easymesh_relay_init(struct dpp_global* dpp);
int map_relay_send(uint8_t *buffer, int buffer_len);
int map_rx_auth_req_msg(struct dpp_global *dpp, const uint8_t *chirp, uint8_t *src_mac, uint8_t *auth_req, uint16_t auth_req_len);

#endif