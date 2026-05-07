#ifndef _DPP_RELAY_H_
#define _DPP_RELAY_H_

#include <stdint.h>
#include <stddef.h>

#include "dpp_datamodel.h"
#include "list.h"
#include "packet_tools.h"

#define DPP_TCP_PORT 8908

#define OUI_WFA 0x506f9a

/* DPP Public Action frame identifiers - OUI_WFA */
#define DPP_OUI_TYPE 0x1A

#define DPP_HDR_LEN (4 + 2) /* OUI, OUI Type, Crypto Suite, DPP frame type */


enum dpp_public_action_frame_type {
	DPP_PA_AUTHENTICATION_REQ = 0,
	DPP_PA_AUTHENTICATION_RESP = 1,
	DPP_PA_AUTHENTICATION_CONF = 2,
	DPP_PA_PEER_DISCOVERY_REQ = 5,
	DPP_PA_PEER_DISCOVERY_RESP = 6,
	DPP_PA_PKEX_V1_EXCHANGE_REQ = 7,
	DPP_PA_PKEX_EXCHANGE_RESP = 8,
	DPP_PA_PKEX_COMMIT_REVEAL_REQ = 9,
	DPP_PA_PKEX_COMMIT_REVEAL_RESP = 10,
	DPP_PA_CONFIGURATION_RESULT = 11,
	DPP_PA_CONNECTION_STATUS_RESULT = 12,
	DPP_PA_PRESENCE_ANNOUNCEMENT = 13,
	DPP_PA_RECONFIG_ANNOUNCEMENT = 14,
	DPP_PA_RECONFIG_AUTH_REQ = 15,
	DPP_PA_RECONFIG_AUTH_RESP = 16,
	DPP_PA_RECONFIG_AUTH_CONF = 17,
	DPP_PA_PKEX_EXCHANGE_REQ = 18,
};

struct dpp_connection {
	struct dpp_global *dpp;
	void *msg_ctx;
	void *cb_ctx;
	int sock;
	uint8_t mac_addr[6];
	uint8_t msg_len[4];
	size_t msg_len_octets;
	struct wpabuf *msg;
	struct wpabuf *msg_out;
	size_t msg_out_pos;
	unsigned int read_eloop:1;
	unsigned int write_eloop:1;
	unsigned int on_tcp_tx_complete_gas_done:1;
	unsigned int on_tcp_tx_complete_remove:1;
	unsigned int on_tcp_tx_complete_auth_ok:1;
	unsigned int gas_comeback_in_progress:1;
	char *name;
	struct dpp_enrollee_ctx *related_enrollee;
};
// This header uses 00 so that we can differentiate between string events and value buffer when processing hostapd action events.
// As string event will never start with 00.
#define DPP_EASYMESH_RELAY_MESSAGE_HEADER 0


#define WLAN_ACTION_PUBLIC 4
#define WLAN_PA_VENDOR_SPECIFIC 9
#define WLAN_PA_GAS_INITIAL_REQ 10
#define WLAN_PA_GAS_INITIAL_RESP 11
#define WLAN_PA_GAS_COMEBACK_REQ 12
#define WLAN_PA_GAS_COMEBACK_RESP 13


int dpp_relay_init(struct dpp_global *dpp);
int dpp_add_new_enrollee(struct dpp_global *dpp, uint8_t *mac_addr, const uint8_t *chirp);
int dpp_easymesh_relay_tcp_send_msg(struct dpp_connection *conn, const struct wpabuf *msg);

#endif