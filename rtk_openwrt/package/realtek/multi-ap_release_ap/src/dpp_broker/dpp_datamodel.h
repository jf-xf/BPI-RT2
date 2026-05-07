#ifndef _DPP_DATAMODEL_
#define _DPP_DATAMODEL_

#include <stdint.h>
#include "list.h"

#define SHA256_MAC_LEN 32

struct dpp_global {
	void *msg_ctx;
	struct dl_list enrollee;
	// this conn is one that send out auth req. it will be used to send out following messages.
	struct dpp_connection *conn_ongoing;
};

struct dpp_enrollee_ctx {
	struct dl_list list;
	uint8_t peer_mac_addr[6];
	uint8_t *pubkey_hash_chirp;
	uint8_t auth_in_progress;
	uint8_t auth_sent;
	uint8_t auth_success;
	struct wpabuf *auth_req_msg;
	struct wpabuf *auth_resp_msg;
	struct wpabuf *auth_confirm_msg;
	struct wpabuf *configuration_msg;
};

struct dpp_enrollee_ctx *dpp_find_ctx_by_chirp(struct dpp_global *dpp, const uint8_t *hash);
#endif
