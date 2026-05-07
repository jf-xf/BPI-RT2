#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "easymesh_relay.h"
#include "eloop.h"
#include "dpp_datamodel.h"
#include "packet_tools.h"
#include "dpp_relay.h"
#include "dpp_utils.h"

#define LOCAL_SOCKET_PATH "/tmp/map_dpp_relay"
#define REMOTE_SOCKET_PATH "/tmp/hostapd_action_map_sock"

#define DPP_RELAY_EVENT_CHIRP_NOTIF "DPP-CHIRP-NOTIF"

#define MAP_DPP_RELAY_HEADER_PROXIED_ENCAP (0x09)

#define MACSTR "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]

static inline int snprintf_error(size_t size, int res)
{
	return res < 0 || (unsigned int)res >= size;
}

static inline int snprintf_hex(char *buf, size_t buf_size, const uint8_t *data,
							   size_t len)
{
	size_t i;
	char *pos = buf, *end = buf + buf_size;
	int ret;
	if (buf_size == 0)
		return 0;
	for (i = 0; i < len; i++) {
		ret = snprintf(pos, end - pos, "%02x",
					   data[i]);
		if (snprintf_error(end - pos, ret)) {
			end[-1] = '\0';
			return pos - buf;
		}
		pos += ret;
	}
	end[-1] = '\0';
	return pos - buf;
}

void relay_send_chirp_notification(const uint8_t *r_bootstrap, uint16_t r_bootstrap_len)
{
	char *ifname = "wlanTBD";
	char *event_str = DPP_RELAY_EVENT_CHIRP_NOTIF;
	uint8_t src_mac[6] = {0};
	uint16_t hex_len = r_bootstrap_len * 2 + 1;
	char chirp_hex[hex_len];
	uint16_t chirp_notif_len = 16 + 256 + 30 + hex_len; // ifname+event_str+src_mac+responder_bootstrap_key_hash
	char *chirp_notif = (char *)malloc(chirp_notif_len);

	snprintf_hex(chirp_hex, hex_len, r_bootstrap, r_bootstrap_len);

	snprintf(chirp_notif, chirp_notif_len, "%s %s " MACSTR " %s", ifname, event_str, MAC2STR(src_mac), chirp_hex);
	map_relay_send((uint8_t *)chirp_notif, chirp_notif_len);

	free(chirp_notif);
}

int map_rx_auth_req_msg(struct dpp_global *dpp, const uint8_t *chirp, uint8_t *src_mac, uint8_t *auth_req, uint16_t auth_req_len)
{
	struct dpp_enrollee_ctx *peer_to_auth;
	peer_to_auth = dpp_find_ctx_by_chirp(dpp, chirp);
	if (!peer_to_auth) {
		printf("\nDPP: Can not find related enrollee.\n");
		return -1;
	}
	// memcpy(peer_to_auth->peer_mac_addr, src_mac, 6);
	peer_to_auth->auth_req_msg = wpabuf_alloc(auth_req_len);
	wpabuf_put_data(peer_to_auth->auth_req_msg, auth_req, auth_req_len);
	peer_to_auth->auth_in_progress = 1;

	return 0;
}

int process_message(struct dpp_global *dpp, uint8_t *message, int message_len)
{
	printf("Message received with legth %d, type %d\n", message_len, message[0]);

	uint8_t chirp[SHA256_MAC_LEN];
	uint8_t src_mac[6];
	uint8_t chirp_present = 0;
	uint8_t frame_type, flag;

	uint8_t *msg;
	uint16_t msg_len;

	uint8_t *p = dpp_get_tlv(message, message_len, TLV_TYPE_DPP_CHIRP_VALUE);

	if (p) {
		uint8_t hash_len = 0;
		flag = *p;
		if(flag & DPP_CHIRP_ENROLLEE_ADDR_PRESENT) {
			p += 1;
			memcpy(src_mac, p, 6);
			p += 6;
			hash_len = *p;
			if (hash_len != SHA256_MAC_LEN) {
				dpp_printf("Error! Chirp hash length is %d, does not match expected value %d", hash_len, SHA256_MAC_LEN);
				return -1;
			}
			p += 1;
			memcpy(chirp, p, SHA256_MAC_LEN);
			chirp_present = 1;
		}
	}

	p = dpp_get_tlv(message, message_len, TLV_TYPE_1905_ENCAP_DPP);

	if(p == NULL) {
		dpp_printf("Error! Cannot find encap message frame");
		return 1;
	}

	flag = *p;
	p += 1;
	if (flag & DPP_CHIRP_ENROLLEE_ADDR_PRESENT)
		p += 6;
	frame_type = *p;
	p += 1;

	msg_len = WPA_GET_BE16(p);
	p += 2;
	msg = p;

	if (chirp_present) {
		map_rx_auth_req_msg(dpp, (const uint8_t *)chirp, src_mac, msg, msg_len);
		return 0;
	}

	struct wpabuf *relay_message = wpabuf_alloc(msg_len);
	wpabuf_put_data(relay_message, p, msg_len);
	if (dpp->conn_ongoing == NULL) {
		printf("DPP: Received message from map daemon but hostapd conn is not found \n");
		wpabuf_free(relay_message);
		return -1;
	}

	dpp_printf("Sending message with frame %d", frame_type);

	wpa_hexdump("Message Frame ", msg, msg_len);

	int result = dpp_easymesh_relay_tcp_send_msg(dpp->conn_ongoing, relay_message);
	wpabuf_free(relay_message);

	if (result != 0) {
		printf("DPP: Failed to send received message to hostapd \n");
	} else {
		printf("DPP: Send received message to hostapd successfully \n");
	}
	return result;

	return 0;
}

static void map_relay_receive(int sock, void *eloop_ctx, void *sock_ctx)
{
	struct dpp_global *dpp = eloop_ctx;
	uint8_t buf[4096];
	int res;
	struct sockaddr_storage from;
	socklen_t fromlen = sizeof(from);

	res = recvfrom(sock, buf, sizeof(buf) - 1, 0,
		       (struct sockaddr *) &from, &fromlen);
	if (res < 0) {
		printf("MAP_RELAY: recvfrom failed: %s", strerror(errno));
		return;
	}

	buf[res] = '\0';

	process_message(dpp, buf, res);
}


int easymesh_relay_init(struct dpp_global* dpp)
{
	int sock_fd = -1;
	struct sockaddr_un addr;

	unlink(LOCAL_SOCKET_PATH);

	sock_fd = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (sock_fd < 0) {
		printf("MAP Relay: socket(PF_UNIX): %s\n", strerror(errno));
		goto fail;
	}

	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {
		printf("MAP Relay: setsockopt(SO_REUSEADDR) failed: %s\n", strerror(errno));
		goto fail;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, LOCAL_SOCKET_PATH, sizeof(addr.sun_path));

	if (bind(sock_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		printf("MAP Relay: Failed to bind domain port: %s\n", strerror(errno));
		goto fail;
	}

	if (eloop_register_read_sock(sock_fd, map_relay_receive, dpp, NULL) < 0) {
		eloop_unregister_read_sock(sock_fd);
		goto fail;
	}

	printf("MAP Relay: Socker server started at %s\n", LOCAL_SOCKET_PATH);

	return 0;

fail:
	if (sock_fd >= 0)
		close(sock_fd);
	unlink(LOCAL_SOCKET_PATH);
	return -1;
}

int map_relay_send(uint8_t *buffer, int buffer_len)
{
	int                sock_fd = -1;
	struct sockaddr_un serveraddr;
	int flags;

	sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock_fd < 0) {
		return -1;
	}

	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sun_family = AF_UNIX;
	strcpy(serveraddr.sun_path, REMOTE_SOCKET_PATH);

	if (connect(sock_fd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
		close(sock_fd);
		return -1;
	}

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

	if (send(sock_fd, buffer, buffer_len, 0) < 0) {
		close(sock_fd);
		return -1;
	}

	close(sock_fd);

	return 0;
}
