/*
 * DPP over TCP
 * Copyright (c) 2019-2020, The Linux Foundation
 * Copyright (c) 2022, Realtek Semiconductor Corp
 * 
 * This software may be distributed, used, and modified under the terms of
 * BSD license:
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. Neither the name(s) of the above-listed copyright holder(s) nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "packet_tools.h"
#include "dpp_utils.h"
#include "dpp_relay.h"
#include "dpp_datamodel.h"
#include "eloop.h"
#include "list.h"
#include "easymesh_relay.h"

static void dpp_conn_tx_ready(int sock, void *eloop_ctx, void *sock_ctx);
static void dpp_controller_rx(int sd, void *eloop_ctx, void *sock_ctx);

static void dpp_connection_free(struct dpp_connection *conn)
{
	if (conn->sock >= 0) {
		dpp_printf("DPP: Close Controller socket %d", conn->sock);
		eloop_unregister_sock(conn->sock, EVENT_TYPE_READ);
		eloop_unregister_sock(conn->sock, EVENT_TYPE_WRITE);
		close(conn->sock);
	}

	free(conn->msg);
	free(conn->msg_out);
	free(conn->name);
	free(conn);
}


static void dpp_connection_remove(struct dpp_connection *conn)
{
	dpp_connection_free(conn);
}


static int dpp_tcp_send(struct dpp_connection *conn)
{
	int res;

	if (!conn->msg_out) {
		eloop_unregister_sock(conn->sock, EVENT_TYPE_WRITE);
		conn->write_eloop = 0;
		return -1;
	}
	res = send(conn->sock,
		   wpabuf_head_u8(conn->msg_out) + conn->msg_out_pos,
		   wpabuf_len(conn->msg_out) - conn->msg_out_pos, 0);
	if (res < 0) {
		dpp_printf("DPP: Failed to send buffer: %s", strerror(errno));
		dpp_connection_remove(conn);
		return -1;
	}

	conn->msg_out_pos += res;
	if (wpabuf_len(conn->msg_out) > conn->msg_out_pos) {
		dpp_printf("DPP: %u/%u bytes of message sent to Controller",
			   (unsigned int) conn->msg_out_pos,
			   (unsigned int) wpabuf_len(conn->msg_out));
		if (!conn->write_eloop &&
		    eloop_register_sock(conn->sock, EVENT_TYPE_WRITE,
					dpp_conn_tx_ready, conn, NULL) == 0)
			conn->write_eloop = 1;
		return 1;
	}

	dpp_printf("DPP: Full message sent over TCP");
	wpabuf_free(conn->msg_out);
	conn->msg_out = NULL;
	conn->msg_out_pos = 0;
	eloop_unregister_sock(conn->sock, EVENT_TYPE_WRITE);
	conn->write_eloop = 0;
	if (!conn->read_eloop &&
	    eloop_register_sock(conn->sock, EVENT_TYPE_READ,
				dpp_controller_rx, conn, NULL) == 0)
		conn->read_eloop = 1;

	if (conn->on_tcp_tx_complete_remove) {
		dpp_connection_remove(conn);
		return -1;
	}

	return 0;
}

static void dpp_conn_tx_ready(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct dpp_connection *conn = eloop_ctx;

	dpp_printf("DPP: TCP socket %d ready for TX", sock);
	dpp_tcp_send(conn);
}


static int dpp_tcp_send_msg(struct dpp_connection *conn, const struct wpabuf *msg)
{
	wpabuf_free(conn->msg_out);
	conn->msg_out_pos = 0;
	conn->msg_out = wpabuf_alloc(4 + wpabuf_len(msg));

	if (!conn->msg_out)
		return -1;

	wpabuf_put_be32(conn->msg_out, wpabuf_len(msg));
	wpabuf_put_data(conn->msg_out, wpabuf_head_u8(msg), wpabuf_len(msg));

	int result = dpp_tcp_send(conn);

	if (result == 1) {
		if (!conn->write_eloop) {
			if (eloop_register_sock(conn->sock, EVENT_TYPE_WRITE,
						dpp_conn_tx_ready,
						conn, NULL) < 0)
				return -1;
			conn->write_eloop = 1;
			return 1;
		}
		return 1;
	} else if (result == 0) {
		return 0;
	} else {
		return -1;
	}
}


int dpp_easymesh_relay_tcp_send_msg(struct dpp_connection *conn, const struct wpabuf *msg) {
	return dpp_tcp_send_msg(conn, msg);
}

int dpp_add_new_enrollee(struct dpp_global *dpp, uint8_t *mac_addr, const uint8_t *chirp)
{
	struct dpp_enrollee_ctx *new_peer_ctx = (struct dpp_enrollee_ctx *)malloc(sizeof(struct dpp_enrollee_ctx));

	memcpy(new_peer_ctx->peer_mac_addr, mac_addr, 6);
	new_peer_ctx->pubkey_hash_chirp = (uint8_t *)malloc(SHA256_MAC_LEN);
	memcpy(new_peer_ctx->pubkey_hash_chirp, chirp, SHA256_MAC_LEN);
	new_peer_ctx->auth_in_progress = 0;
	new_peer_ctx->auth_sent = 0;

	dl_list_add(&(dpp->enrollee), &(new_peer_ctx->list));

	return 0;
}

static int dpp_controller_rx_presence_announcement(struct dpp_connection *conn, const uint8_t *hdr, const uint8_t *buf,
						   size_t len)
{
	const uint8_t *r_bootstrap;
	uint16_t r_bootstrap_len;
	struct dpp_enrollee_ctx *peer_ctx;

	dpp_printf("DPP: Presence Announcement");

	r_bootstrap = dpp_get_attr(buf, len, DPP_ATTR_R_BOOTSTRAP_KEY_HASH,
				   &r_bootstrap_len);
	if (!r_bootstrap || r_bootstrap_len != SHA256_MAC_LEN) {
		dpp_printf("Missing or invalid required Responder Bootstrapping Key Hash attribute");
		return -1;
	}

	wpa_hexdump("DPP: Responder Bootstrapping Key Hash", r_bootstrap, r_bootstrap_len);

	peer_ctx = dpp_find_ctx_by_chirp(conn->dpp, r_bootstrap);

	if (!peer_ctx) {
		dpp_printf("DPP: No matching bootstrapping information found");
		uint8_t src_mac[6] = {0};
		dpp_add_new_enrollee(conn->dpp, src_mac, r_bootstrap);
		relay_send_chirp_notification(r_bootstrap, r_bootstrap_len);

		// Auth req is not ready yet, close the connection for now and wait for the next presence annoucement.
		return -1;
	}

	if (!(peer_ctx->auth_req_msg && peer_ctx->auth_in_progress)) {
		dpp_printf("DPP: Waiting for Auth Req. Drop this chirp.");
		return -1;
	} else if (peer_ctx->auth_sent == 1){
		dpp_printf("DPP: Auth Req already sent, ignore chirp.");
		return -1;
	} else {
		dpp_printf("DPP: sending Auth Req");
		int result = dpp_tcp_send_msg(conn, peer_ctx->auth_req_msg);
		if (result >= 0) {
			conn->related_enrollee = peer_ctx;
			conn->dpp->conn_ongoing = conn;
			// peer_ctx->auth_sent = 1;
			dpp_printf("DPP: Auth Req send successfully.");
			return 0;
		} else {
			dpp_printf("DPP: Auth Req send failed.");
			return -1;
		}
	}
}

int dpp_controller_relay_to_map(struct dpp_connection *conn, const uint8_t *hdr, size_t len)
{
	dpp_printf("\n######################### %s %d ##################\n", __FUNCTION__, __LINE__);
	// format: [00, len, len, ...message...]
	ssize_t relay_message_buf_len = len + 3;
	uint8_t *relay_message_buf = (uint8_t *)malloc(relay_message_buf_len);
	memset(relay_message_buf, 0, relay_message_buf_len);

	uint8_t *p = relay_message_buf;

	*p = DPP_EASYMESH_RELAY_MESSAGE_HEADER;
	p++;

	*(uint16_t *)p = len;
	p += 2;

	memcpy(p, hdr, len);

	dpp_printf("DPP: Start relay message with header [%02x] and len 3 + [%d] to map daemon", *hdr, (int)len);
	int result = map_relay_send(relay_message_buf, relay_message_buf_len);
	conn->dpp->conn_ongoing = conn;
	if (result < 0) {
		dpp_printf("DPP: Fail to relay message with type [%02x] to map daemon", *hdr);
	} else {
		dpp_printf("DPP: Relay message with type [%02x] length 3 + [%d] to map daemon success", *hdr, (int)len);
	}
	free(relay_message_buf);
	return result;
}

static int dpp_controller_rx_action(struct dpp_connection *conn, const uint8_t *msg, size_t len)
{
	const uint8_t *pos, *end;
	uint8_t type;

	dpp_printf("DPP: Received DPP Action frame over TCP");
	pos = msg;
	end = msg + len;

	if (end - pos < DPP_HDR_LEN ||
	    WPA_GET_BE24(pos) != OUI_WFA ||
	    pos[3] != DPP_OUI_TYPE) {
		dpp_printf("DPP: Unrecognized header");
		return -1;
	}

	if (pos[4] != 1) {
		dpp_printf("DPP: Unsupported Crypto Suite %u",
			   pos[4]);
		return -1;
	}
	type = pos[5];
	dpp_printf("DPP: Received message type %u", type);
	pos += DPP_HDR_LEN;

	wpa_hexdump("DPP: Received message attributes",
		    pos, end - pos);

	if (dpp_check_attrs(pos, end - pos) < 0)
		return -1;

	switch (type) {
	case DPP_PA_AUTHENTICATION_REQ:
	case DPP_PA_AUTHENTICATION_RESP:
		return dpp_controller_relay_to_map(conn, msg - 1, len + 1);
	case DPP_PA_AUTHENTICATION_CONF:
	case DPP_PA_CONFIGURATION_RESULT:
		return dpp_controller_relay_to_map(conn, msg - 1, len + 1);
	case DPP_PA_CONNECTION_STATUS_RESULT:
		return -1;
	case DPP_PA_PRESENCE_ANNOUNCEMENT:
		return dpp_controller_rx_presence_announcement(conn, msg, pos, end - pos);
	case DPP_PA_RECONFIG_ANNOUNCEMENT:
	case DPP_PA_RECONFIG_AUTH_RESP:
		return -1;
	default:
		dpp_printf("DPP: Unsupported frame subtype %d", type);
		return -1;
	}
}


static void dpp_controller_rx(int sd, void *eloop_ctx, void *sock_ctx)
{
	struct dpp_connection *conn = eloop_ctx;
	int res;
	const uint8_t *pos;

	dpp_printf("DPP: TCP data available for reading (sock %d)", sd);

	if (conn->msg_len_octets < 4) {
		int msglen;

		res = recv(sd, &conn->msg_len[conn->msg_len_octets],
			   4 - conn->msg_len_octets, 0);
		if (res < 0) {
			dpp_printf("DPP: recv failed: %s",
				   strerror(errno));
			dpp_connection_remove(conn);
			return;
		}
		if (res == 0) {
			dpp_printf("DPP: No more data available over TCP");
			dpp_connection_remove(conn);
			return;
		}
		dpp_printf("DPP: Received %d/%d octet(s) of message length field",
			   res, (int) (4 - conn->msg_len_octets));
		conn->msg_len_octets += res;

		if (conn->msg_len_octets < 4) {
			dpp_printf("DPP: Need %d more octets of message length field",
				   (int) (4 - conn->msg_len_octets));
			return;
		}

		msglen = WPA_GET_BE32(conn->msg_len);
		dpp_printf("DPP: Message length: %u", msglen);
		if (msglen > 65535) {
			dpp_printf("DPP: Unexpectedly long message");
			dpp_connection_remove(conn);
			return;
		}

		wpabuf_free(conn->msg);
		conn->msg = wpabuf_alloc(msglen);
	}

	if (!conn->msg) {
		dpp_printf("DPP: No buffer available for receiving the message");
		dpp_connection_remove(conn);
		return;
	}

	dpp_printf("DPP: Need %u more octets of message payload",
		   (unsigned int) wpabuf_tailroom(conn->msg));

	res = recv(sd, wpabuf_put(conn->msg, 0), wpabuf_tailroom(conn->msg), 0);
	if (res < 0) {
		dpp_printf("DPP: recv failed: %s", strerror(errno));
		dpp_connection_remove(conn);
		return;
	}

	if (res == 0) {
		dpp_printf("DPP: No more data available over TCP");
		dpp_connection_remove(conn);
		return;
	}

	dpp_printf("DPP: Received %d octets", res);
	wpabuf_put(conn->msg, res);

	if (wpabuf_tailroom(conn->msg) > 0) {
		dpp_printf("DPP: Need %u more octets of message payload",
			   (unsigned int) wpabuf_tailroom(conn->msg));
		return;
	}

	conn->msg_len_octets = 0;
	wpa_hexdump("DPP: Received TCP message", conn->msg->buf, conn->msg->used);
	if (conn->msg->used < 1) {
		dpp_connection_remove(conn);
		return;
	}

	pos = conn->msg->buf;
	switch (*pos) {
	case WLAN_PA_VENDOR_SPECIFIC:
		if (dpp_controller_rx_action(conn, pos + 1, wpabuf_len(conn->msg) - 1) < 0)
			dpp_connection_remove(conn);
		break;
	default:
		if (dpp_controller_relay_to_map(conn, pos, wpabuf_len(conn->msg)) < 0)
			dpp_connection_remove(conn);
		break;
	}

}


static void dpp_relay_tcp_cb(int sock_fd, void *eloop_ctx, void *sock_ctx)
{
	struct dpp_global *global = eloop_ctx;
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(addr);
	int conn_sock_fd;
	struct dpp_connection *conn;

	conn_sock_fd = accept(sock_fd, (struct sockaddr *) &addr, &addr_len);

	dpp_printf("DPP: New TCP connection");

	if (conn_sock_fd < 0) {
		dpp_printf("DPP: Failed to accept new connection: %s",
			   strerror(errno));
		return;
	}
	dpp_printf("DPP: Connection from %s:%d",
		   inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	
	conn = malloc(sizeof(*conn));
	memset(conn, 0, sizeof(*conn));

	conn->sock = conn_sock_fd;
	conn->dpp = global;
	
	if (fcntl(conn->sock, F_SETFL, O_NONBLOCK) != 0) {
		dpp_printf("DPP: fnctl(O_NONBLOCK) failed: %s", strerror(errno));
		goto fail;
	}

	if (eloop_register_sock(conn->sock, EVENT_TYPE_READ,
				dpp_controller_rx, conn, NULL) < 0)
		goto fail;

	conn->read_eloop = 1;

	return;

fail:
	close(conn_sock_fd);
	free(conn);
}


int dpp_relay_init(struct dpp_global *dpp)
{
	int on = 1;
	struct sockaddr_in sin;
	int                sock_fd = -1;
	int port;

	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (sock_fd < 0)
		goto fail;

	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
		dpp_printf("DPP: setsockopt(SO_REUSEADDR) failed: %s", strerror(errno));
		/* try to continue anyway */
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	port = DPP_TCP_PORT;
	sin.sin_port = htons(port);
	if (bind(sock_fd, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
		dpp_printf("DPP: Failed to bind Controller TCP port: %s", strerror(errno));
		goto fail;
	}
	
	if (listen(sock_fd, 10 /* max backlog */) < 0 ||
	    fcntl(sock_fd, F_SETFL, O_NONBLOCK) < 0 ||
	    eloop_register_sock(sock_fd, EVENT_TYPE_READ, dpp_relay_tcp_cb, dpp, NULL))
		goto fail;

	dpp_printf("DPP: Controller started on TCP port %d", port);

	return 0;

fail:
	if (sock_fd >= 0) {
		close(sock_fd);
	}
	return -1;
}