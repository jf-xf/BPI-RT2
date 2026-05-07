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


#include "platform_os.h"
#include "1905_l2.h"
#include "platform.h"
#include "platform_interfaces.h"
#include "platform_alme_server_priv.h"
#include "platform_os_priv.h"
#include "multi_ap_tlvs.h"
#include "al_datamodel.h"
#include "al.h"
#include "al_utils.h"
#include "packet_tools.h"
#include "global_settings.h"
#include "al_send.h"

#ifndef __UCLIBC__
#include "utils.h"
#endif

#include <errno.h>   // errno
#include <mqueue.h>  // mq_*() functions
#include <poll.h>    // poll()
#include <pthread.h> // threads and mutex functions
#include <limits.h> // threads and mutex functions
#include <stdlib.h>  // free(), malloc(), ...
#include <stdio.h>
#include <string.h>      // memcpy(), memcmp(), ...
#include <sys/inotify.h> // inotify_*()
#include <unistd.h>      // read(), sleep()

// for netlink in _topologyMonitorThread
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <netinet/ether.h>
#include <linux/filter.h>

//for hostapd in _hostapdNotificationThread
#include <sys/un.h>

////////////////////////////////////////////////////////////////////////////////
// Private functions, structures and macros
////////////////////////////////////////////////////////////////////////////////

#define MAX_PUSH_BUTTON_COUNT 60
#define ETH_FRAME_HEADER_LEN  14

#ifndef O_CLOEXEC
#define O_CLOEXEC	02000000	/* set close_on_exec */
#endif


// *********** Netlink Message Fragmentation stuff *****************************
#define MAX_NETLINK_MSG_NR 10
#define MAX_FRAGMENT_NR 10
#define REASSEMBLE_BUFFER_SIZE 4200
#define MAX_NETLINK_MSG_SIZE 2048

// *********** IPC stuff *******************************************************

// Queue related function in the PLATFORM API return queue IDs that are INT8U
// elements.
// However, in POSIX all queue related functions deal with a 'mqd_t' type.
// The following global arrays are used to store the association between a
// "PLATFORM INT8U ID" and a "POSIX mqd_t ID"

#define MAX_QUEUE_IDS 256 // Number of values that fit in an INT8U
#define MAX_RECEIVE_ERROR 100

static mqd_t           queues_id[MAX_QUEUE_IDS] = { [0 ... MAX_QUEUE_IDS - 1] = (mqd_t)-1 };
static pthread_mutex_t queues_id_mutex          = PTHREAD_MUTEX_INITIALIZER;

// *********** Packet capture stuff ********************************************

// We use 'libpcap' to capture 1905 packets on all interfaces.
// It works like this:
//
//   - When the PLATFORM API user calls "PLATFORM_REGISTER_QUEUE_EVENT()" with
//     'PLATFORM_QUEUE_EVENT_NEW_1905_PACKET', 'libpcap' is used to set the
//     corresponding interface into monitor mode.
//
//   - In addition, a new thread is created ('_pcapLoopThread()') which runs
//     forever and, everytime a new packet is received on the corresponding
//     interface, that thread calls '_pcapProcessPacket()'
//
//   - '_pcapProcessPacket()' simply post the whole contents of the received
//     packet to a queue so that the user can later obtain it with a call to
//     "PLATFORM_QUEUE_READ()"

static pthread_mutex_t pcap_filters_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  pcap_filters_cond  = PTHREAD_COND_INITIALIZER;
static int             pcap_filters_flag  = 0;

struct _pcapCaptureThreadData {
	INT8U queue_id;
	char *interface_name;
	INT8U interface_mac_address[6];
	INT8U al_mac_address[6];
};

pthread_t alme_server_thread;

static void _processPacket(struct _pcapCaptureThreadData *aux, unsigned char *packet, int packet_len)
{
	// This function is executed (on a per-interface dedicated thread) every
	// time a new 1905 packet arrives

	INT8U  message[30 + MAX_NETWORK_SEGMENT_SIZE + ETH_FRAME_HEADER_LEN];
	INT16U message_len;
	INT8U  message_len_msb;
	INT8U  message_len_lsb;

	if (NULL == aux) {
		// Invalid argument
		//
		PLATFORM_PRINTF_ERROR("[PLATFORM] *Pcap thread* Invalid arguments in _pcapProcessPacket()\n");
		return;
	}

	if (packet_len > MAX_NETWORK_SEGMENT_SIZE + ETH_FRAME_HEADER_LEN) {
		// This should never happen
		//
		PLATFORM_PRINTF_ERROR("[PLATFORM] *Pcap thread* Captured packet too big\n");
		return;
	}

	// In order to build the message that will be inserted into the queue, we
	// need to follow the "message format" defines in the documentation of
	// function 'PLATFORM_REGISTER_QUEUE_EVENT()'
	//
	INT8U interface_name_len = strlen(aux->interface_name);
	message_len              = (INT16U)packet_len + 6 + interface_name_len + 1;
#if _HOST_IS_LITTLE_ENDIAN_ == 1
	message_len_msb = *(((INT8U *)&message_len) + 1);
	message_len_lsb = *(((INT8U *)&message_len) + 0);
#else
	message_len_msb = *(((INT8U *)&message_len) + 0);
	message_len_lsb = *(((INT8U *)&message_len) + 1);
#endif

	message[0] = PLATFORM_QUEUE_EVENT_NEW_1905_PACKET;
	message[1] = message_len_msb;
	message[2] = message_len_lsb;
	message[3] = interface_name_len;
	int i;
	for (i = 0; i < interface_name_len; i++) {
		message[4 + i] = aux->interface_name[i];
	}

	message[4 + interface_name_len] = aux->interface_mac_address[0];
	message[5 + interface_name_len] = aux->interface_mac_address[1];
	message[6 + interface_name_len] = aux->interface_mac_address[2];
	message[7 + interface_name_len] = aux->interface_mac_address[3];
	message[8 + interface_name_len] = aux->interface_mac_address[4];
	message[9 + interface_name_len] = aux->interface_mac_address[5];

	memcpy(&message[10 + interface_name_len], packet, packet_len);

	// Now simply send the message.
	//
	PLATFORM_PRINTF_DETAIL("[PLATFORM] *Pcap thread* Sending %d bytes to queue (0x%02x, 0x%02x, 0x%02x, ...)\n", 3 + message_len, message[0], message[1], message[2]);

	if (0 == sendMessageToAlQueue(aux->queue_id, message, 3 + message_len)) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] *Pcap thread* Error sending message to queue from _pcapProcessPacket()\n");
		return;
	}

	return;
}

static int socket_ready_and_init(char *interface_name)
{
	int timeout_count = 600;
	while (!PLATFORM_IS_INTERFACE_UP(interface_name)) {
		PLATFORM_PRINTF_WARNING("Interface %s is not ready yet, sleep 1s \n", interface_name);
		sleep(1);
		timeout_count--;
		if (timeout_count <= 0) {
			PLATFORM_PRINTF_WARNING("Interface %s waiting for ready has timed out, give up \n", interface_name);
			return -1;
		}
	}
	sleep(10);
	PLATFORM_PRINTF_WARNING("[%s][%d] Interface %s is ready, now recovering \n", __FUNCTION__, __LINE__, interface_name);
	return socket(AF_PACKET, SOCK_RAW | SOCK_CLOEXEC, htons(ETH_P_ALL));
}

static void *_pcapLoopThread(void *p)
{
	int                data_size;
	struct sockaddr_ll socket_address;
	struct ifreq       ifr;
	int                ifindex;
	int                error_count = 0;
	int                sock_created = 0;

	if (NULL == p) {
		return NULL;
	}
	struct _pcapCaptureThreadData *aux            = (struct _pcapCaptureThreadData *)p;
	char *                         interface_name = aux->interface_name;

	PLATFORM_PRINTF_DEBUG("Pcaploop thread created for interface %s \n", interface_name);

	int sock_raw = socket(AF_PACKET, SOCK_RAW | SOCK_CLOEXEC, htons(ETH_P_ALL));
	if (sock_raw < 0) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] %s socket returned with errno=%d (%s) while opening a RAW socket\n", interface_name, errno, strerror(errno));
		return NULL;
	}

	sock_created = 1;

	struct sock_filter filter_1905[] = {
		{ 0x28, 0, 0, 0x0000000c },
		{ 0x15, 2, 0, 0x0000893a },
		{ 0x15, 1, 0, 0x000088cc },
		{ 0x06, 0, 0, 0000000000 },
		{ 0x06, 0, 0, 0xffffffff },
	};

	struct sock_fprog bpf_1905 = {
		.len    = 5,
		.filter = filter_1905,
	};

	if (setsockopt(sock_raw, SOL_SOCKET, SO_ATTACH_FILTER, &bpf_1905, sizeof(bpf_1905)) < 0) {
		close(sock_raw);
		PLATFORM_PRINTF_ERROR("[PLATFORM] attach filter for %s socket failed with errno=%d (%s)\n", interface_name, errno, strerror(errno));
		return NULL;
	}

	if(DMgetWFATestMode()) {
		int buffer_size;
		socklen_t size_length = sizeof(buffer_size);
		(void) getsockopt(sock_raw, SOL_SOCKET, SO_RCVBUF, &buffer_size, &size_length);
		PLATFORM_PRINTF_ERROR("[PLATFORM] %s socket SO_RCVBUF is %d\n", interface_name, buffer_size);
		buffer_size *= 2;
		if (setsockopt(sock_raw, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(int)) < 0) {
			close(sock_raw);
			PLATFORM_PRINTF_ERROR("[PLATFORM] %s socket setsockopt() failed with errno=%d (%s)\n", interface_name, errno, strerror(errno));
			return NULL;
		}
	}

	struct sockaddr_ll from;
	socklen_t          fromlen;

	INT8U mcast_addr_1905[6] = MCAST_1905;
	INT8U mcast_addr_lldp[6] = MCAST_LLDP;

	unsigned char *buffer = (unsigned char *)PLATFORM_MALLOC(65536);
	if (!buffer) {
		PLATFORM_PRINTF_ERROR("Fail to allocate buffer, _pcapLoopThread failed \n");
		close(sock_raw);
		return NULL;
	}
	PLATFORM_MEMSET(buffer, 0, sizeof(unsigned char) * 65536);

	PLATFORM_MEMSET(&socket_address, 0, sizeof(socket_address));
	socket_address.sll_family = PF_PACKET;
	PLATFORM_PRINTF_DETAIL("[PLATFORM] Retrieving interface index\n");
	strncpy(ifr.ifr_name, interface_name, IFNAMSIZ - 1);
	if (ioctl(sock_raw, SIOCGIFINDEX, &ifr) == -1) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] ioctl('%s',SIOCGIFINDEX) returned with errno=%d (%s) while opening a RAW socket\n", interface_name, errno, strerror(errno));
		close(sock_raw);
		PLATFORM_FREE(buffer);
		return NULL;
	}
	ifindex = ifr.ifr_ifindex;
	PLATFORM_PRINTF_DETAIL("[PLATFORM] Successfully got interface index %d\n", ifindex);

	socket_address.sll_ifindex  = ifindex;
	socket_address.sll_protocol = htons(ETH_P_ALL);

	if (-1 == bind(sock_raw, (struct sockaddr *)&socket_address, sizeof(socket_address))) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] Cannot bind interface %s\n", interface_name);
		close(sock_raw);
		PLATFORM_FREE(buffer);
		return NULL;
	}

	PLATFORM_PRINTF_INFO("[PLATFORM] Listening on interface %s\n", interface_name);

	pthread_mutex_lock(&pcap_filters_mutex);
	pcap_filters_flag = 1;
	pthread_cond_signal(&pcap_filters_cond);
	pthread_mutex_unlock(&pcap_filters_mutex);

	while (1) {
		//Receive a packet
		fromlen   = sizeof(from);
		data_size = recvfrom(sock_raw, buffer, 65536, 0, (struct sockaddr *)&from, &fromlen);
		// data_size = recv(sock_raw, buffer, 65536, 0);
		if (data_size < 0) {
			PLATFORM_PRINTF_ERROR("[%s][%d] Sock recv error, failed to get packets on %s\n", __FUNCTION__, __LINE__,interface_name);
			sleep(1); // let socket destruction finish in case, or let glitch finish.
			strncpy(ifr.ifr_name, interface_name, IFNAMSIZ - 1);
			ifr.ifr_ifindex = 0;
			int interface_status = PLATFORM_GET_INTERFACE_STATUS(interface_name, &(ifr.ifr_ifindex));

			if (ifr.ifr_ifindex == ifindex) {
				// interface confirm present, down or up.
				PLATFORM_PRINTF_WARNING("[%s][%d] Interface %s index %d unchanged\n", __FUNCTION__, __LINE__, interface_name, ifr.ifr_ifindex);
				if (0 == interface_status) {
					PLATFORM_PRINTF_WARNING("[%s][%d] Interface %s is down only, index stays the same, do not recover \n", __FUNCTION__, __LINE__,interface_name);
					continue; // it will be blocked at recvfrom until interface up.
				} else if (1 == interface_status && error_count < MAX_RECEIVE_ERROR) {
					error_count++;
					PLATFORM_PRINTF_WARNING("[%s][%d] Interface %s is up, tolerate %d times recv error \n", __FUNCTION__, __LINE__,interface_name, error_count);
					continue;
				} else {
					// status < 0 wont happen as index is retrieved. So this is error reached max
					PLATFORM_PRINTF_ERROR("[%s][%d] Interface %s Tolerated %d times recv error, recover now \n", __FUNCTION__, __LINE__,interface_name, error_count);
				}
			} else {
				// interface may not be present (index 0), or present with index changed.
				PLATFORM_PRINTF_WARNING("[%s][%d] Interface %s index %d changed to %d, recover now\n", __FUNCTION__, __LINE__, interface_name, ifindex, ifr.ifr_ifindex);
				ifindex = ifr.ifr_ifindex;
			}

			PLATFORM_PRINTF_ERROR("[%s][%d] Interface %s start to recover \n", __FUNCTION__, __LINE__, interface_name);

			if (sock_created) {
				close(sock_raw);
				sock_created = 0;
			}

			sock_raw = socket_ready_and_init(interface_name);
			if (sock_raw < 0) {
				PLATFORM_PRINTF_ERROR("[%s][%d] %s socket returned with errno=%d (%s)\n", __FUNCTION__, __LINE__, interface_name, errno, strerror(errno));
				sock_created = 0;
				break;
			}

			sock_created = 1;

			if (setsockopt(sock_raw, SOL_SOCKET, SO_ATTACH_FILTER, &bpf_1905, sizeof(bpf_1905)) < 0) {
				PLATFORM_PRINTF_ERROR("[%s][%d] Attach filter for %s socket failed with errno=%d (%s)\n", __FUNCTION__, __LINE__, interface_name, errno, strerror(errno));
				break;
			}
			if(DMgetWFATestMode()) {
				int buffer_size;
				socklen_t size_length = sizeof(buffer_size);
				(void) getsockopt(sock_raw, SOL_SOCKET, SO_RCVBUF, &buffer_size, &size_length);
				PLATFORM_PRINTF_ERROR("[%s][%d] %s socket SO_RCVBUF is %d\n", __FUNCTION__, __LINE__, interface_name, buffer_size);
				buffer_size *= 2;
				if (setsockopt(sock_raw, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(int)) < 0) {
					PLATFORM_PRINTF_ERROR("[%s][%d] %s socket setsockopt() failed with errno=%d (%s)\n", __FUNCTION__, __LINE__, interface_name, errno, strerror(errno));
					break;
				}
			}

			if (ifindex <= 0) { // interface is recreated during socket_ready_and_init waiting period.
				PLATFORM_PRINTF_WARNING("[%s][%d] Retrieve newly created interface %s index\n", __FUNCTION__, __LINE__, interface_name);
				if (ioctl(sock_raw, SIOCGIFINDEX, &ifr) == -1) {
					PLATFORM_PRINTF_ERROR("[%s][%d] ioctl('%s',SIOCGIFINDEX) returned with errno=%d (%s)\n", __FUNCTION__, __LINE__, interface_name, errno, strerror(errno));
					break;
				}
				PLATFORM_PRINTF_WARNING("[%s][%d] Retrieved newly created interface %s index %d \n", __FUNCTION__, __LINE__, interface_name, ifr.ifr_ifindex);
				ifindex = ifr.ifr_ifindex;
			}

			PLATFORM_MEMSET(&socket_address, 0, sizeof(socket_address));
			socket_address.sll_family = PF_PACKET;
			socket_address.sll_ifindex  = ifindex;
			socket_address.sll_protocol = htons(ETH_P_ALL);

			if (-1 == bind(sock_raw, (struct sockaddr *)&socket_address, sizeof(socket_address))) {
				PLATFORM_PRINTF_ERROR("[%s][%d] Cannot bind interface %s\n", __FUNCTION__, __LINE__, interface_name);
				break;
			}
			PLATFORM_PRINTF_WARNING("[%s][%d] Finished recovering raw socket \n", __FUNCTION__, __LINE__);
			continue;
		}
		error_count                 = 0;
		struct ethhdr *eth          = (struct ethhdr *)(buffer);
		INT16U         eth_protocol = ntohs(eth->h_proto);

		if (from.sll_pkttype == PACKET_OUTGOING) {
			continue;
		}

		if (ETHERTYPE_1905 != eth_protocol && ETHERTYPE_LLDP != eth_protocol) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] Unexpected %04x packet received on %s\n", eth_protocol, interface_name);
			continue;
		}

		if (PLATFORM_STRCMP("lo", interface_name)
		    && ((0 == PLATFORM_MEMCMP(eth->h_source, aux->al_mac_address, 6)
		         || 0 == PLATFORM_MEMCMP(eth->h_source, aux->interface_mac_address, 6)))) {
			continue;
		}

		// PLATFORM_PRINTF_INFO("\nEthernet Header\n");
		// PLATFORM_PRINTF_INFO("\tSource Address: %02x-%02x-%02x-%02x-%02x-%02x\n", eth->h_source[0], eth->h_source[1], eth->h_source[2], eth->h_source[3], eth->h_source[4], eth->h_source[5]);
		// PLATFORM_PRINTF_INFO("\tDestination Address: %02x-%02x-%02x-%02x-%02x-%02x\n", eth->h_dest[0], eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4], eth->h_dest[5]);
		// PLATFORM_PRINTF_INFO("\tProtocol: %04x\n", eth_protocol);
		if ((0 == PLATFORM_MEMCMP(eth->h_dest, aux->al_mac_address, 6))
		    || (0 == PLATFORM_MEMCMP(eth->h_dest, aux->interface_mac_address, 6))
		    || (0 == PLATFORM_MEMCMP(eth->h_dest, mcast_addr_1905, 6))
		    || (0 == PLATFORM_MEMCMP(eth->h_dest, mcast_addr_lldp, 6))) {
			// PLATFORM_PRINTF_INFO("Packet from %s, size %d\n", interface_name, data_size);
			_processPacket(aux, buffer, data_size);
		}

		//Now process the packet
	}
	if (sock_created) {
		close(sock_raw);
	}
	PLATFORM_PRINTF_ERROR("[PLATFORM] *Pcap thread* Exiting thread (interface %s)\n", aux->interface_name);
	PLATFORM_FREE(aux);
	PLATFORM_FREE(buffer);
	return NULL;
}

// *********** Timers stuff ****************************************************

// We use the POSIX timers API to implement PLATFORM timers
// It works like this:
//
//   - When the PLATFORM API user calls "PLATFORM_REGISTER_QUEUE_EVENT()" with
//     'PLATFORM_QUEUE_EVENT_TIMEOUT*', a new POSIX timer is created.
//
//   - When the timer expires, the POSIX API creates a thread for us and makes
//     it run function '_timerHandler()'
//
//   - '_timerHandler()' simply deletes (or reprograms, depending on the type
//     of timer) the timer and sends a message to a queue so that the user can
//     later be aware of the timer expiration with a call to
//     "PLATFORM_QUEUE_READ()"

struct _timerHandlerThreadData {
	INT8U   queue_id;
	INT32U  token;
	INT8U   periodic;
	timer_t timer_id;
};

static void _timerHandler(union sigval s)
{
	struct _timerHandlerThreadData *aux;

	INT8U  message[3 + 4];
	INT16U packet_len;
	INT8U  packet_len_msb;
	INT8U  packet_len_lsb;
	INT8U  token_msb;
	INT8U  token_2nd_msb;
	INT8U  token_3rd_msb;
	INT8U  token_lsb;

	aux = (struct _timerHandlerThreadData *)s.sival_ptr;

	// In order to build the message that will be inserted into the queue, we
	// need to follow the "message format" defines in the documentation of
	// function 'PLATFORM_REGISTER_QUEUE_EVENT()'
	//
	packet_len = 4;

#if _HOST_IS_LITTLE_ENDIAN_ == 1
	packet_len_msb = *(((INT8U *)&packet_len) + 1);
	packet_len_lsb = *(((INT8U *)&packet_len) + 0);

	token_msb     = *(((INT8U *)&aux->token) + 3);
	token_2nd_msb = *(((INT8U *)&aux->token) + 2);
	token_3rd_msb = *(((INT8U *)&aux->token) + 1);
	token_lsb     = *(((INT8U *)&aux->token) + 0);
#else
	packet_len_msb  = *(((INT8U *)&packet_len) + 0);
	packet_len_lsb  = *(((INT8U *)&packet_len) + 1);

	token_msb     = *(((INT8U *)&aux->token) + 0);
	token_2nd_msb = *(((INT8U *)&aux->token) + 1);
	token_3rd_msb = *(((INT8U *)&aux->token) + 2);
	token_lsb     = *(((INT8U *)&aux->token) + 3);
#endif

	message[0] = 1 == aux->periodic ? PLATFORM_QUEUE_EVENT_TIMEOUT_PERIODIC : PLATFORM_QUEUE_EVENT_TIMEOUT;
	message[1] = packet_len_msb;
	message[2] = packet_len_lsb;
	message[3] = token_msb;
	message[4] = token_2nd_msb;
	message[5] = token_3rd_msb;
	message[6] = token_lsb;

	PLATFORM_PRINTF_DETAIL("[PLATFORM] *Timer handler* Sending %d bytes to queue (%02x, %02x, %02x, ...)\n", 3 + packet_len, message[0], message[1], message[2]);

	if(PLATFORM_GET_QUEUE_SIZE(aux->queue_id) >= 60) {
		return;
	}

	if (0 == sendMessageToAlQueue(aux->queue_id, message, 3 + packet_len)) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] *Timer handler* Error sending message to queue from _timerHandler()\n");
	}

	if (1 == aux->periodic) {
		// Periodic timer are automatically re-armed. We don't need to do
		// anything
	} else {
		// Delete the asociater timer
		//
		timer_delete(aux->timer_id);

		// Free 'struct _timerHandlerThreadData', as we don't need it any more
		//
		free(aux);
	}

	return;
}

// *********** Push button stuff ***********************************************

// Pressing the button can be simulated by "touching" (ie. updating the
// timestamp) the following tmp file
//
#define PUSH_BUTTON_VIRTUAL_FILENAME "/tmp/virtual_push_button"

// For those platforms with a physical buttons attached to a GPIO, we need to
// know the actual GPIO number (as seen by the Linux kernel) to use.
//
//     NOTE: "PUSH_BUTTON_GPIO_NUMBER" is a string, not a number. It will later
//     be used in a string context, thus the "" are needed.
//     It can take the string representation of a number (ex: "26") or the
//     special value "disable", meaning we don't have GPIO support.
//
#ifdef _RTK_LINUX_
#	define PUSH_BUTTON_GPIO_NUMBER "disable"
#	define PUSH_BUTTON_GPIO_VALUE_FILENAME "/proc/gpio"
#	define PUSH_BUTTON_WPS_GPIO_FILENAME "/var/wps_gpio"
#	define PUSH_BUTTON_HOLD_THRESHOLD 5000

#else
#	define PUSH_BUTTON_GPIO_NUMBER "disable" //"26"
#	define PUSH_BUTTON_GPIO_EXPORT_FILENAME "/sys/class/gpio/export"
#	define PUSH_BUTTON_GPIO_DIRECTION_FILENAME "/sys/class/gpio/gpio" PUSH_BUTTON_GPIO_NUMBER "/direction"
#	define PUSH_BUTTON_GPIO_VALUE_FILENAME "/sys/class/gpio/gpio" PUSH_BUTTON_GPIO_NUMBER "/direction"
#endif

// The only information that needs to be sent to the new thread is the "queue
// id" to later post messages to the queue.
//
struct _pushButtonThreadData {
	INT8U queue_id;
};

static void *_pushButtonThread(void *p)
{
	// In this implementation we will send the "push button" configuration
	// event message to the queue when either:
	//
	//   a) The user presses a physical button associated to a GPIO whose number
	//      is "PUSH_BUTTON_GPIO_NUMBER" (ie. it is exported by the linux kernel
	//      in "/sys/class/gpio/gpioXXX", where "XXX" is
	//      "PUSH_BUTTON_GPIO_NUMBER")
	//
	//   b) The user updates the timestamp of a tmp file called
	//      "PUSH_BUTTON_VIRTUAL_FILENAME".
	//      This is useful for debugging and for supporting the "push button"
	//      mechanism in those platforms without a physical button.
	//
	// This thread will simply wait for activity on any of those two file
	// descriptors and then send the "push button" configuration event to the
	// AL queue.
	// How is this done?
	//
	//   1. Configure the GPIO as input.
	//   2. Create an "inotify" watch on the tmp file.
	//   3. Use "poll()" to wait for either changes in the value of the GPIO or
	//      timestamp updates in the tmp file.

	FILE *fd_tmp;
	int   fdraw_tmp;

#ifndef _RTK_LINUX_
	FILE *fd_gpio;
	int   gpio_enabled = 0;
	int   fdraw_gpio;
#endif

	struct pollfd fdset[2];

	INT8U queue_id;

	queue_id = ((struct _pushButtonThreadData *)p)->queue_id;

#ifndef _RTK_LINUX_

	if (0 != strcmp(PUSH_BUTTON_GPIO_NUMBER, "disable")) {
		gpio_enabled = 1;
	} else {
		gpio_enabled = 0;
	}

	// First of all, prepare the GPIO kernel descriptor for "reading"...
	//
	if (gpio_enabled) {

		// 1. Write the number of the GPIO where the physical button is
		//    connected to file "/sys/class/gpio/export".
		//    This will instruct the Linux kernel to create a folder named
		//    "/sys/class/gpio/gpioXXX" that we can later use to read the GPIO
		//    level.
		//
		fd_gpio = fopen(PUSH_BUTTON_GPIO_EXPORT_FILENAME, "we");
		if (NULL == fd_gpio) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] *Push button thread* Error opening GPIO fd %s\n", PUSH_BUTTON_GPIO_EXPORT_FILENAME);
			return NULL;
		}
		if (0 == fwrite(PUSH_BUTTON_GPIO_NUMBER, 1, strlen(PUSH_BUTTON_GPIO_NUMBER), fd_gpio)) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] *Push button thread* Error writing '" PUSH_BUTTON_GPIO_NUMBER "' to %s\n", PUSH_BUTTON_GPIO_EXPORT_FILENAME);
			fclose(fd_gpio);
			return NULL;
		}
		fclose(fd_gpio);

		// 2. Write "in" to file "/sys/class/gpio/gpioXXX/direction" to tell the
		//    kernel that this is an "input" GPIO (ie. we are only going to
		//    read -and not write- its value).
		fd_gpio = fopen(PUSH_BUTTON_GPIO_DIRECTION_FILENAME, "we");
		if (NULL == fd_gpio) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] *Push button thread* Error opening GPIO fd %s\n", PUSH_BUTTON_GPIO_DIRECTION_FILENAME);
			return NULL;
		}
		if (0 == fwrite("in", 1, strlen("in"), fd_gpio)) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] *Push button thread* Error writing 'in' to %s\n", PUSH_BUTTON_GPIO_DIRECTION_FILENAME);
			fclose(fd_gpio);
			return NULL;
		}
		fclose(fd_gpio);
	}

	// ... and then re-open the GPIO file descriptors for reading in "raw"
	// (ie "open" instead of "fopen") mode.
	//
	if (gpio_enabled) {
		if (-1 == (fdraw_gpio = open(PUSH_BUTTON_GPIO_VALUE_FILENAME, O_RDONLY | O_NONBLOCK | O_CLOEXEC))) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] *Push button thread* Error opening GPIO fd %s\n", PUSH_BUTTON_GPIO_VALUE_FILENAME);
		}
	}
#endif

	// Next, regarding the "virtual" button, first create the "tmp" file in
	// case it does not already exist...
	//
	fd_tmp = fopen(PUSH_BUTTON_VIRTUAL_FILENAME, "w+e");
	if (NULL == fd_tmp) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] *Push button thread* Could not create tmp file %s\n", PUSH_BUTTON_VIRTUAL_FILENAME);
		return NULL;
	}
	fclose(fd_tmp);

	// ...and then add a "watch" that triggers when its timestamp changes (ie.
	// when someone does a "touch" of the file or writes to it, for example).
	//
	if (-1 == (fdraw_tmp = inotify_init1(IN_CLOEXEC))) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] *Push button thread* inotify_init() returned with errno=%d (%s)\n", errno, strerror(errno));
		return NULL;
	}
	if (-1 == inotify_add_watch(fdraw_tmp, PUSH_BUTTON_VIRTUAL_FILENAME, IN_MODIFY)) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] *Push button thread* inotify_add_watch() returned with errno=%d (%s)\n", errno, strerror(errno));
		return NULL;
	}

	// At this point we have two file descriptors ("fdraw_gpio" and "fdraw_tmp")
	// that we can monitor with a call to "poll()"
	//
	while (1) {
		int   nfds;
		INT8U button_pressed;

		memset((void *)fdset, 0, sizeof(fdset));

		fdset[0].fd     = fdraw_tmp;
		fdset[0].events = POLLIN;
		nfds            = 1;

#ifndef _RTK_LINUX_
		if (gpio_enabled) {
			fdset[1].fd     = fdraw_gpio;
			fdset[1].events = POLLPRI;
			nfds            = 2;
		}
#endif

		// The thread will block here (forever, timeout = -1), until there is
		// a change in one of the two file descriptors ("changes" in the "tmp"
		// file fd are cause by "attribute" changes -such as the timestamp-,
		// while "changes" in the GPIO fd are caused by a value change in the
		// GPIO value).
		//
		if (0 > poll(fdset, nfds, -1)) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] *Push button thread* poll() returned with errno=%d (%s)\n", errno, strerror(errno));
			break;
		}

		button_pressed = 0;

		if (fdset[0].revents & POLLIN) {
			struct inotify_event event;

			PLATFORM_PRINTF_DETAIL("[PLATFORM] *Push button thread* Virtual button has been pressed!\n");
			button_pressed = 1;

			// We must "read()" from the "tmp" fd to "consume" the event, or
			// else the next call to "poll() won't block.
			//
			if (-1 == read(fdraw_tmp, &event, sizeof(event))) {
				PLATFORM_PRINTF_ERROR("[PLATFORM] *Push button thread* read() returned with errno=%d (%s)\n", errno, strerror(errno));
				continue;
			}
		}
#ifndef _RTK_LINUX_
		else if (gpio_enabled && (fdset[1].revents & POLLPRI)) {
			char buf[3];
			if (-1 == read(fdset[1].fd, buf, 3)) {
				PLATFORM_PRINTF_ERROR("[PLATFORM] *Push button thread* read() returned with errno=%d (%s)\n", errno, strerror(errno));
				continue;
			}
			if (buf[0] == '1') {
				button_pressed = 1;
			}
		}
#endif

		if (1 == button_pressed) {
			INT8U message[3];

			message[0] = PLATFORM_QUEUE_EVENT_PUSH_BUTTON;
			message[1] = 0x0;
			message[2] = 0x0;

			PLATFORM_PRINTF_DETAIL("[PLATFORM] *Push button thread* Sending 3 bytes to queue (0x%02x, 0x%02x, 0x%02x)\n", message[0], message[1], message[2]);

			if (0 == sendMessageToAlQueue(queue_id, message, 3)) {
				PLATFORM_PRINTF_ERROR("[PLATFORM] *Push button thread* Error sending message to queue from _pushButtonThread()\n");
			}
		}
	}

#ifndef _RTK_LINUX_
	// Close file descriptors and exit
	//
	if (gpio_enabled) {
		fclose(fd_gpio);
	}
#endif

	PLATFORM_PRINTF_INFO("[PLATFORM] *Push button thread* Exiting...\n");

	free(p);
	return NULL;
}

// *********** Topology change notification stuff ******************************

// The platform notifies the 1905 that a topology change has just took place
// by "touching" the following tmp file
//
#define TOPOLOGY_CHANGE_NOTIFICATION_FILENAME "/tmp/topology_change"

// The only information that needs to be sent to the new thread is the "queue
// id" to later post messages to the queue.
//
struct _topologyMonitorThreadData {
	INT8U queue_id;
};
static char *      rtk_multi_ap_prefix = "rtk_multi_ap";

#define NETLINK_MSG_BUFFER_SIZE 4096
#define NETLINK_RTK 31
#define NETLINK_RTK_SEC 27

struct _NetlinkMessage {
	char *reassemble_buffer;
	int  is_in_use;
	int  fragment_sequence_nr_buffer[MAX_FRAGMENT_NR];
	int  fragment_count;
	int  total_fragment_nr;
	int  known_message_id;
	int  total_data_received;
};

int _process_netlink_message(INT8U *payload, int payload_len, INT8U queue_id);
int _reassemble_fragmentized_message(int payload_length, INT8U *payload, struct _NetlinkMessage *netlink_message);

int _reassemble_fragmentized_message(int payload_length, INT8U *payload, struct _NetlinkMessage *netlink_message)
{
	int fragment_sequence_nr = payload[3];
	int found_message        = 0;
	int idx                  = 0;
	int i, ret;
	for (i = 0; i < MAX_NETLINK_MSG_NR; i++) {
		if (netlink_message[i].is_in_use == 1 && payload[1] == netlink_message[i].known_message_id) {
			found_message = 1;
			idx           = i;
			if (netlink_message[idx].fragment_sequence_nr_buffer[fragment_sequence_nr] == 0) {
				netlink_message[idx].fragment_count++;
				netlink_message[idx].fragment_sequence_nr_buffer[fragment_sequence_nr] = 1;
			}
			break;
		}
	}
	if (found_message == 0) {
		for (i = 0; i < MAX_NETLINK_MSG_NR; i++) {
			if (netlink_message[i].is_in_use == 0) {
				netlink_message[i].is_in_use = 1;
				idx                          = i;
				break;
			}
		}
		netlink_message[idx].fragment_count++;
		netlink_message[idx].known_message_id = payload[1];
	}
	if (!netlink_message[idx].reassemble_buffer)
		netlink_message[idx].reassemble_buffer = PLATFORM_MALLOC(REASSEMBLE_BUFFER_SIZE);
	PLATFORM_MEMCPY(netlink_message[idx].reassemble_buffer + (MAX_NETLINK_MSG_SIZE - 4) * fragment_sequence_nr, payload + 4, payload_length - 4);
	netlink_message[idx].total_data_received += payload_length - 4;
	if (payload[2] == 1) {
		netlink_message[idx].total_fragment_nr = fragment_sequence_nr + 1;
	}
	if (netlink_message[idx].total_fragment_nr == netlink_message[idx].fragment_count) {
		PLATFORM_MEMCPY(payload, netlink_message[idx].reassemble_buffer, netlink_message[idx].total_data_received);
		ret = netlink_message[idx].total_data_received;
		PLATFORM_FREE(netlink_message[idx].reassemble_buffer);
		memset(netlink_message + idx, 0, sizeof(*netlink_message));
	} else {
		ret = 0;
	}
	return ret;
}
static void *_topologyMonitorThread(void *p)
{
	FILE *fd_tmp;

	int fdraw_tmp;

	INT8U queue_id;

	struct pollfd fdset[3];

	queue_id = ((struct _topologyMonitorThreadData *)p)->queue_id;

	//declare the first netlink
	int                netlink_s = 0;
	struct sockaddr_nl src_addr, dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct iovec     iov;
	struct msghdr    msg;
	memset(&src_addr, 0, sizeof(src_addr));
	memset(&dest_addr, 0, sizeof(dest_addr));

	//declare the second netlink
	int                netlink_sec_s = 0;
	struct sockaddr_nl src_sec_addr, dest_sec_addr;
	struct nlmsghdr *nlh_sec = NULL;
	struct iovec     iov_sec;
	struct msghdr    msg_sec;
	memset(&src_sec_addr, 0, sizeof(src_sec_addr));
	memset(&dest_sec_addr, 0, sizeof(dest_sec_addr));


	// Regarding the "virtual" notification system, first create the "tmp" file
	// in case it does not already exist...
	//
	fd_tmp = fopen(TOPOLOGY_CHANGE_NOTIFICATION_FILENAME, "w+e");
	if (NULL == fd_tmp) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] *Topology change monitor thread* Could not create tmp file %s\n", TOPOLOGY_CHANGE_NOTIFICATION_FILENAME);
		return NULL;
	}
	fclose(fd_tmp);

	// ...and then add a "watch" that triggers when its timestamp changes (ie.
	// when someone does a "touch" of the file or writes to it, for example).
	//
	if (-1 == (fdraw_tmp = inotify_init1(IN_CLOEXEC))) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] *Push button thread* inotify_init() returned with errno=%d (%s)\n", errno, strerror(errno));
		return NULL;
	}
	if (-1 == inotify_add_watch(fdraw_tmp, TOPOLOGY_CHANGE_NOTIFICATION_FILENAME, IN_ATTRIB)) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] *Push button thread* inotify_add_watch() returned with errno=%d (%s)\n", errno, strerror(errno));
		return NULL;
	}

	/* first driver to register netlink */
	// netlink socket setup
	netlink_s = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_RTK);

	if (netlink_s < 0) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] Error to create socket for Multi-AP _topologyMonitorThread \n");
		free(p);
		return NULL;
	}

	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid    = getpid(); /* self pid */
	src_addr.nl_groups = 0;        /* unicast */

	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid    = 0; /* For Linux Kernel */
	dest_addr.nl_groups = 0; /* unicast */

	if (bind(netlink_s, (struct sockaddr *)&src_addr, sizeof(src_addr)) < 0) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] [_topologyMonitorThread] Error to bind socket for ioctl\n");
		close(netlink_s);
		free(p);
		return NULL;
	}

	/* Fill the netlink message header */
	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(NETLINK_MSG_BUFFER_SIZE));
	memset(nlh, 0, NLMSG_SPACE(NETLINK_MSG_BUFFER_SIZE));
	nlh->nlmsg_len   = NLMSG_SPACE(NETLINK_MSG_BUFFER_SIZE);
	nlh->nlmsg_pid   = getpid(); //sending process ID
	nlh->nlmsg_flags = 0;

	strcpy(NLMSG_DATA(nlh), rtk_multi_ap_prefix);

	/*iov structure */
	iov.iov_base = (void *)nlh;
	iov.iov_len  = nlh->nlmsg_len;

	/* msg */
	memset(&msg, 0, sizeof(msg));
	msg.msg_name    = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov     = &iov;
	msg.msg_iovlen  = 1;

	/*Send message to kernel for pid */
	if (sendmsg(netlink_s, &msg, 0) <= 0) {
		close(netlink_s);
		PLATFORM_PRINTF_ERROR("[MULTI-AP] [_topologyMonitorThread] user sendmsg err: errno=%d!", errno);
		free(nlh);
		free(p);
		return NULL;
	} else {
		PLATFORM_PRINTF_INFO("[MULTI-AP] [_topologyMonitorThread] setup nellink. pid: %d\n", nlh->nlmsg_pid);
	}

	if(DMgetMultiAPCommonNetlink()) {
		/* second driver to register netlink */
		// netlink socket setup
		netlink_sec_s = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_RTK_SEC);

		if (netlink_sec_s < 0) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] Error to create socket for Multi-AP _topologyMonitorThread \n");
			close(netlink_s);
			free(nlh);
			free(p);
			return NULL;
		}

		src_sec_addr.nl_family = AF_NETLINK;
		src_sec_addr.nl_pid    = getpid(); /* self pid */
		src_sec_addr.nl_groups = 0;        /* unicast */

		dest_sec_addr.nl_family = AF_NETLINK;
		dest_sec_addr.nl_pid    = 0; /* For Linux Kernel */
		dest_sec_addr.nl_groups = 0; /* unicast */

		if (bind(netlink_sec_s, (struct sockaddr *)&src_sec_addr, sizeof(src_sec_addr)) < 0) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] [_topologyMonitorThread] Error to bind socket for ioctl\n");
			close(netlink_s);
			close(netlink_sec_s);
			free(nlh);
			free(p);
			return NULL;
		}

		/* Fill the netlink message header */
		nlh_sec = (struct nlmsghdr *)malloc(NLMSG_SPACE(NETLINK_MSG_BUFFER_SIZE));
		memset(nlh_sec, 0, NLMSG_SPACE(NETLINK_MSG_BUFFER_SIZE));
		nlh_sec->nlmsg_len   = NLMSG_SPACE(NETLINK_MSG_BUFFER_SIZE);
		nlh_sec->nlmsg_pid   = getpid(); //sending process ID
		nlh_sec->nlmsg_flags = 0;

		strcpy(NLMSG_DATA(nlh_sec), rtk_multi_ap_prefix);

		/*iov structure */
		iov_sec.iov_base = (void *)nlh_sec;
		iov_sec.iov_len  = nlh_sec->nlmsg_len;

		/* msg */
		memset(&msg_sec, 0, sizeof(msg_sec));
		msg_sec.msg_name    = (void *)&dest_sec_addr;
		msg_sec.msg_namelen = sizeof(dest_sec_addr);
		msg_sec.msg_iov     = &iov_sec;
		msg_sec.msg_iovlen  = 1;

		/*Send message to kernel for pid */
		if (sendmsg(netlink_sec_s, &msg_sec, 0) <= 0) {
			PLATFORM_PRINTF_ERROR("[MULTI-AP] [_topologyMonitorThread] user sendmsg err: errno=%d!", errno);
			close(netlink_s);
			close(netlink_sec_s);
			free(nlh);
			free(nlh_sec);
			free(p);
			return NULL;
		} else {
			PLATFORM_PRINTF_INFO("[MULTI-AP] [_topologyMonitorThread] setup nellink. pid: %d\n", nlh->nlmsg_pid);
		}
	}

	// netlink socket setup end
	struct _NetlinkMessage netlink_message[MAX_NETLINK_MSG_NR];
	memset(netlink_message, 0, sizeof(netlink_message));
	while (1) {
		int   nfds;

		memset((void *)fdset, 0, sizeof(fdset));

		fdset[0].fd     = fdraw_tmp;
		fdset[0].events = POLLIN;

		fdset[1].fd     = netlink_s;
		fdset[1].events = POLLIN;

		if(DMgetMultiAPCommonNetlink()) {
			fdset[2].fd     = netlink_sec_s;
			fdset[2].events = POLLIN;
			nfds = 3;
		}
		else {
			nfds = 2;
		}

		// TODO: Other fd's to detect topoly changes would be initialized here.
		// One good idea would be to use a NETLINK socket that is notified by
		// the Linux kernel when network "stuff" (routes, IPs, ...) change.
		//
		//fdset[0].fd     = ...;
		//fdset[0].events = POLLIN;
		//nfds            = 2;

		// The thread will block here (forever, timeout = -1), until there is
		// a change in one of the previous file descriptors .
		//
		if (0 > poll(fdset, nfds, -1)) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] *Topology change monitor thread* poll() returned with errno=%d (%s)\n", errno, strerror(errno));
			break;
		}

		if (fdset[0].revents & POLLIN) {
			struct inotify_event event;

			PLATFORM_PRINTF_DETAIL("[PLATFORM] *Topology change monitor thread* Virtual notification has been activated!\n");

			// We must "read()" from the "tmp" fd to "consume" the event, or
			// else the next call to "poll() won't block.
			//
			if (-1 == read(fdraw_tmp, &event, sizeof(event))){
				PLATFORM_PRINTF_ERROR("[PLATFORM] *topologyMonitorThread* read() returned with errno=%d (%s)\n", errno, strerror(errno));
				continue;
			}
			INT8U message[16] = { 0 };

			message[0] = PLATFORM_QUEUE_EVENT_TOPOLOGY_CHANGE_NOTIFICATION;
			message[1] = 0x00;
			message[2] = 0x0D;

			message[3] = 0x11;
			message[4] = 0x33;
			message[5] = 0x55;
			message[6] = 0x77;
			message[7] = 0x99;
			message[8] = 0xBB;

			message[9]  = 0x00;
			message[10] = 0x22;
			message[11] = 0x44;
			message[12] = 0x66;
			message[13] = 0x88;
			message[14] = 0xAA;

			message[15] = 0xCC;

			if (0 == sendMessageToAlQueue(queue_id, message, 3 + 13)) {
				PLATFORM_PRINTF_ERROR("[PLATFORM] *Topology change monitor thread* Error sending message to queue from _pushButtonThread()\n");
			}
		}
		// there is a message ready from wlan driver
		if (fdset[1].revents & POLLIN) {
			int ret = 0;
			ret     = recvmsg(fdset[1].fd, &msg, 0);
			if(-1 == ret) {
				PLATFORM_PRINTF_ERROR("Error when receiving netlink message, skip!");
				continue;
			}

			INT8U payload[NETLINK_MSG_BUFFER_SIZE] = { 0 };
			int   payload_length                   = NLMSG_PAYLOAD(nlh, 0);
			ret                                    = payload_length;
			PLATFORM_MEMCPY(payload, NLMSG_DATA(nlh), payload_length);
			if (payload[0] == PLATFORM_QUEUE_EVENT_NETLINK_FRAGMENTIZED_MESSAGE) {
				ret = _reassemble_fragmentized_message(payload_length, payload, netlink_message);
			}
			if (ret != 0) {
				PLATFORM_PRINTF_TRACE("Message %02x from driver.\n", payload[0]);

				_process_netlink_message(payload, ret, queue_id);
			}
		}

		if(DMgetMultiAPCommonNetlink()) {

			if (fdset[2].revents & POLLIN){
				int ret = 0;
				ret     = recvmsg(fdset[2].fd, &msg_sec, 0);
				if(-1 == ret) {
					PLATFORM_PRINTF_ERROR("Error when receiving netlink message, skip!");
					continue;
				}

				INT8U payload[NETLINK_MSG_BUFFER_SIZE] = { 0 };
				int   payload_length                   = NLMSG_PAYLOAD(nlh_sec, 0);
				ret                                    = payload_length;
				PLATFORM_MEMCPY(payload, NLMSG_DATA(nlh_sec), payload_length);
				if (payload[0] == PLATFORM_QUEUE_EVENT_NETLINK_FRAGMENTIZED_MESSAGE) {
					ret = _reassemble_fragmentized_message(payload_length, payload, netlink_message);
				}
				if (ret != 0) {
					PLATFORM_PRINTF_TRACE("Message %02x from sec driver.\n", payload[0]);
					_process_netlink_message(payload, ret, queue_id);
				}
			}
		}
	}

	PLATFORM_PRINTF_INFO("[PLATFORM] *Topology change monitor thread* Exiting...\n");
	free(nlh);
	free(p);
	return NULL;
}

int _process_netlink_message(INT8U *payload, int payload_len, INT8U queue_id)
{

	INT8U client_mac_addr[6] = { 0 }, bssid[6] = { 0 }, event = 0;
	INT8U notification_activated = 0;

	if (payload[0] == TLV_TYPE_BEACON_METRICS_RESPONSE) {

		INT16U message_len = 0;
		INT8U *message_pointer;
		PLATFORM_MEMCPY(&message_len, &payload[1], sizeof(INT16U));

		INT8U message[3 + message_len];

		message[0] = PLATFORM_QUEUE_EVENT_BEACON_METRIC_RESPONSE_NOTIFICATION;

		message_pointer = &message[1];
		_I2B(&message_len, &message_pointer);

		PLATFORM_MEMCPY(&message[3], &payload[3], message_len);

		if (0 == sendMessageToAlQueue(queue_id, message, 3 + message_len)) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] *Topology change monitor thread* Error sending message to queue for Unassoc STA Link metric()\n");
		}
		return 0;
	} else if (payload[0] == TLV_TYPE_BACKHAUL_STEERING_RESPONSE) {

		INT16U message_len = 0;
		PLATFORM_MEMCPY(&message_len, &payload[1], sizeof(INT16U));

		INT8U message[3 + message_len];

		if (message_len > 13) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] *Topology change monitor thread* Backhaul Steering Response message length too long: %d\n", message_len);
			return 0;
		}

		message[0] = PLATFORM_QUEUE_EVENT_BACKHAUL_STEERING_RESPONSE_NOTIFICATION;
		message[1] = 0x00;
		message[2] = 0x0D;
		//PLATFORM_MEMCPY(&message[1], &message_len, sizeof(INT16U));

		PLATFORM_MEMCPY(&message[3], &payload[3], message_len);

		if (0 == sendMessageToAlQueue(queue_id, message, 3 + message_len)) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] *Topology change monitor thread* Error sending message to queue for Backhaul Steering Response()\n");
		}

		return 0;
	} else if (payload[0] == TLV_TYPE_METRIC_REPORT_POLICY) {

		if (payload[1] == 1) { //Sta rssi trigger

			INT16U message_len = 6;

			INT8U message[3 + message_len];

			message[0] = PLATFORM_QUEUE_EVENT_ASSOC_STA_LINK_METRIC_NOTIFICATION;
			message[1] = 0x00;
			message[2] = 0x06;
			PLATFORM_MEMCPY(&message[3], &payload[2], MACADDRLEN);

			if (0 == sendMessageToAlQueue(queue_id, message, 3 + message_len)) {
				PLATFORM_PRINTF_ERROR("[PLATFORM] *Topology change monitor thread* Error sending message to queue for Assoc STA metric response()\n");
			}
		} else if (payload[1] == 3) { //Channel utilization trigger
			INT16U message_len = 6;

			INT8U message[3 + message_len];

			message[0] = PLATFORM_QUEUE_EVENT_AP_METRIC_RESPONSE_NOTIFICATION;
			message[1] = 0x00;
			message[2] = 0x06;
			PLATFORM_MEMCPY(&message[3], &payload[2], MACADDRLEN);

			if (0 == sendMessageToAlQueue(queue_id, message, 3 + message_len)) {
				PLATFORM_PRINTF_ERROR("[PLATFORM] *Topology change monitor thread* Error sending message to queue for AP metric response()\n");
			}
		}
		return 0;
	} else if (payload[0] == TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE) {

		INT16U message_len = 0;
		INT8U *message_pointer;
		PLATFORM_MEMCPY(&message_len, &payload[1], sizeof(INT16U));

		INT8U message[3 + message_len];

		message[0] = PLATFORM_QUEUE_EVENT_UNASSOC_STA_LINK_METRIC_NOTIFICATION;

		message_pointer = &message[1];
		_I2B(&message_len, &message_pointer);

		PLATFORM_MEMCPY(&message[3], &payload[3], message_len);

		if (0 == sendMessageToAlQueue(queue_id, message, 3 + message_len)) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] *Topology change monitor thread* Error sending message to queue for Unassoc STA Link metric()\n");
		}

		return 0;
	} else if (payload[0] == TLV_TYPE_CHANNEL_SCAN_RESULT) {

		INT16U message_len = 0;
		INT8U *message_pointer;
		PLATFORM_MEMCPY(&message_len, &payload[1], sizeof(INT16U));

		message_len -= 3; //implementation of len in driver includes 1 byte type and 2 bytes length

		INT8U message[3 + message_len];

		message[0] = PLATFORM_QUEUE_EVENT_CHANNEL_SCAN_RESULT_NOTIFICATION;

		message_pointer = &message[1];
		_I2B(&message_len, &message_pointer);

		PLATFORM_MEMCPY(&message[3], &payload[3], message_len);

		if (0 == sendMessageToAlQueue(queue_id, message, 3 + message_len)) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] *Topology change monitor thread* Error sending message to queue for Channel Scan Result()\n");
		}

		return 0;
	} else if (payload[0] == TLV_TYPE_CAC_COMPLETION_REPORT) {

		INT16U message_len = 0;
		INT8U *message_pointer;

		message_pointer = &payload[1];

		_E2B(&message_pointer, &message_len);
		// PLATFORM_MEMCPY(&message_len, &payload[1], sizeof(INT16U));

		message_len += 3;

		INT8U message[3 + message_len];
		message[0] = PLATFORM_QUEUE_EVENT_CAC_COMPLETION_REPORT_NOTIFICATION;

		message_pointer = &message[1];
		_I2B(&message_len, &message_pointer);

		PLATFORM_MEMCPY(&message[3], payload, message_len);
		if (0 == sendMessageToAlQueue(queue_id, message, 3 + message_len)) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] *Topology change monitor thread* Error sending message to queue for CAC Completion Report()\n");
		}
		return 0;
	} else if (payload[0] == TLV_TYPE_CAC_STATUS_REPORT) {

		INT16U message_len = 0;
		INT8U *message_pointer;

		PLATFORM_MEMCPY(&message_len, &payload[1], 2);
		// PLATFORM_MEMCPY(&message_len, &payload[1], sizeof(INT16U));

		message_len += 3;

		INT8U message[3 + message_len];

		message[0] = PLATFORM_QUEUE_EVENT_CAC_STATUS_REPORT_NOTIFICATION;

		message_pointer = &message[1];
		_I2B(&message_len, &message_pointer);
		PLATFORM_MEMCPY(&message[3], &payload[3], message_len);

		if (0 == sendMessageToAlQueue(queue_id, message, 3 + message_len)) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] *Topology change monitor thread* Error sending message to queue for CAC Status Report()\n");
		}
		return 0;
	} else if (payload[0] == TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION) {

		INT16U message_len = 0;
		INT8U *message_pointer;
		PLATFORM_MEMCPY(&message_len, &payload[1], sizeof(INT16U));

		INT8U message[3 + message_len];

		message[0] = PLATFORM_QUEUE_EVENT_ASSOCIATION_STATUS_NOTIFICATION;

		message_pointer = &message[1];
		_I2B(&message_len, &message_pointer);

		PLATFORM_MEMCPY(&message[3], &payload[3], message_len);

		if (0 == sendMessageToAlQueue(queue_id, message, 3 + message_len)) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] *Topology change monitor thread* Error sending message to queue for Tunneled Message\n");
		}

		return 0;
	} else if (payload[0] == TLV_TYPE_TUNNELED) {

		INT16U message_len = 0;
		INT8U *message_pointer;
		PLATFORM_MEMCPY(&message_len, &payload[1], sizeof(INT16U));

		INT8U message[3 + message_len];

		message[0]      = PLATFORM_QUEUE_EVENT_TUNNELED_MESSAGE;
		message_pointer = &message[1];
		_I2B(&message_len, &message_pointer);
		PLATFORM_MEMCPY(&message[3], &payload[3], message_len);

		if (0 == sendMessageToAlQueue(queue_id, message, 3 + message_len)) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] *Topology change monitor thread* Error sending message to queue for Tunneled Message\n");
		}

		return 0;
	} else if (payload[0] == PLATFORM_QUEUE_EVENT_FAILED_CONNECTION_MESSAGE) {

		INT16U message_len = 0;
		INT8U *message_pointer;
		PLATFORM_MEMCPY(&message_len, &payload[1], sizeof(INT16U));

		INT8U message[3 + message_len];

		message[0]      = PLATFORM_QUEUE_EVENT_FAILED_CONNECTION_MESSAGE;
		message_pointer = &message[1];
		_I2B(&message_len, &message_pointer);
		PLATFORM_MEMCPY(&message[3], &payload[3], message_len);

		if (0 == sendMessageToAlQueue(queue_id, message, 3 + message_len)) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] *Topology change monitor thread* Error sending message to queue for Failed Connection Message\n");
		}

		return 0;
	} else if (payload[0] == PLATFORM_QUEUE_EVENT_CLIENT_DISASSOCIATION_STATS) {

		INT16U message_len = 0;
		INT8U *message_pointer;
		PLATFORM_MEMCPY(&message_len, &payload[1], sizeof(INT16U));

		INT8U message[3 + message_len];

		message[0]      = PLATFORM_QUEUE_EVENT_CLIENT_DISASSOCIATION_STATS;
		message_pointer = &message[1];
		_I2B(&message_len, &message_pointer);
		PLATFORM_MEMCPY(&message[3], &payload[3], message_len);

		if (0 == sendMessageToAlQueue(queue_id, message, 3 + message_len)) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] *Topology change monitor thread* Error sending message to queue for client disassociation stats Message\n");
		}
		return 0;
	} else if (payload[0] == PLATFORM_QUEUE_EVENT_GENERAL_NETLINK_MESSAGE) {
		// the structure of payload from wlan driver is
		//
		//	+-----------------------+-----------------------------+
		//	|  client mac address   |      bssid of BSS     |event|
		//	+---+---+---+---+---+---+---+---+---+---+---+---+-----+
		//	|   |   |   |   |   |   |   |   |   |   |   |   |     |
		//	+---+---+---+---+---+---+---+---+---+---+---+---+-----+
		INT8U *message_pointer = &(payload[1]);

		PLATFORM_MEMCPY(client_mac_addr, message_pointer, 6);
		PLATFORM_MEMCPY(bssid, message_pointer + 6, 6);
		PLATFORM_MEMCPY(&event, message_pointer + 6 + 6, 1);

		if (CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT_JOIN == event) {
			notification_activated = 1;
			PLATFORM_PRINTF_INFO("[PLATFORM] client JOIN. "
				"MAC address %.2x:%.2x:%.2x:%.2x:%.2x:%.2x "
				"BSSID %.2x:%.2x:%.2x:%.2x:%.2x:%.2x \n",
				client_mac_addr[0], client_mac_addr[1],
				client_mac_addr[2], client_mac_addr[3],
				client_mac_addr[4], client_mac_addr[5],
				bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
		} else if (BTM_RESPONSE_EVENT == event) {
			PLATFORM_PRINTF_INFO("[PLATFORM] BTM Response Event\n");
			INT16U message_len = 20;
			INT8U  message[3 + message_len];

			message[0] = PLATFORM_QUEUE_EVENT_BTM_RESPONSE;
			message[1] = 0x00;
			message[2] = 0x14;
			PLATFORM_MEMCPY(&message[3], message_pointer, message_len);

			if (0 == sendMessageToAlQueue(queue_id, message, 3 + message_len)) {
				PLATFORM_PRINTF_ERROR("[PLATFORM] *Topology change monitor thread* Error sending message to queue for Unassoc STA Link metric()\n");
			}

			return 0;
		} else if (CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT_LEAVE == event) {
			notification_activated = 1;
			PLATFORM_PRINTF_INFO("[PLATFORM] client LEAVE. "
				"MAC address %.2x:%.2x:%.2x:%.2x:%.2x:%.2x "
				"BSSID %.2x:%.2x:%.2x:%.2x:%.2x:%.2x \n",
				client_mac_addr[0], client_mac_addr[1],
				client_mac_addr[2], client_mac_addr[3],
				client_mac_addr[4], client_mac_addr[5],
				bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
		}
	} else if (payload[0] == PLATFORM_QUEUE_EVENT_CHANNEL_CHANGE_NOTIFICATION) {
		INT8U message[3] = { 0 };
		message[0]       = PLATFORM_QUEUE_EVENT_CHANNEL_CHANGE_NOTIFICATION;

		if (0 == sendMessageToAlQueue(queue_id, message, 3)) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] *Topology change monitor thread* Error sending message to queue for channel change notification\n");
		}

		return 0;
	} else {
		PLATFORM_PRINTF_WARNING("[PLATFORM] We received wrong message from wlan driver.\n");
		PLATFORM_PRINTF_WARNING("[PLATFORM] payload length %d:", payload_len);
		int i;
		for (i = 0; i < payload_len; i++) {
			PLATFORM_PRINTF_INFO("%.2x ", payload[i]);
		}
		PLATFORM_PRINTF_INFO("\n");
	}

	if (1 == notification_activated) {
		INT8U message[3 + 13];

		message[0] = PLATFORM_QUEUE_EVENT_TOPOLOGY_CHANGE_NOTIFICATION;
		message[1] = 0x00;
		message[2] = 0x0D;
		message[3]  = client_mac_addr[0];
		message[4]  = client_mac_addr[1];
		message[5]  = client_mac_addr[2];
		message[6]  = client_mac_addr[3];
		message[7]  = client_mac_addr[4];
		message[8]  = client_mac_addr[5];
		message[9]  = bssid[0];
		message[10] = bssid[1];
		message[11] = bssid[2];
		message[12] = bssid[3];
		message[13] = bssid[4];
		message[14] = bssid[5];
		message[15] = event;

		PLATFORM_PRINTF_DETAIL("[PLATFORM] *Topology change monitor thread* Sending 3 bytes to queue (0x%02x, 0x%02x, 0x%02x)\n", message[0], message[1], message[2]);

		if (0 == sendMessageToAlQueue(queue_id, message, 3 + 13)) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] *Topology change monitor thread* Error sending message to queue from _pushButtonThread()\n");
		}
	}

	return 0;
}

// *********** Hostapd notification stuff ******************************

// The only information that needs to be sent to the new thread is the "queue
// id" to later post messages to the queue.
//
#define HOSTAPD_MAP_ACTION_PATH "/tmp/hostapd_action_map_sock"
#define HOSTAPD_RECV_MAX_DATA_SIZE 2048

/* hostapd control interface - fixed message prefixes */
#define AP_STA_CONNECTED "AP-STA-CONNECTED"
#define AP_STA_DISCONNECTED "AP-STA-DISCONNECTED"
#define AP_STA_POSSIBLE_PSK_MISMATCH "AP-STA-POSSIBLE-PSK-MISMATCH"

/* hostapd DPP events */
#define DPP_RELAY_EVENT_CHIRP_NOTIF "DPP-CHIRP-NOTIF"
#define DPP_EVENT_CHIRP_RX "DPP-CHIRP-RX"
#define DPP_EVENT_RECONFIG_ANNOUNCEMENT "DPP-RECONFIG-ANNOUNCEMENT"

/* wpa_supplicant events */
#define DISCONNECTED "DISCONNECTED"
#define CONNECTED "CONNECTED"

#define DPP_EASYMESH_RELAY_MESSAGE_HEADER 0

/* BSS Transition Management Response frame received */
#define BSS_TM_RESP "BSS-TM-RESP"


struct _hostapdNotificationThreadData {
	INT8U queue_id;
};

int PLATFORM_INIT_HAPD_ACTION_SOCKET(int *fd)
{
	struct sockaddr_un s_addr;
	int reuse_addr = 1, ret = 0;
	int backlog = 2;
	int sockfd = socket( AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0 );

	if(sockfd < 0)
	{
		printf("socket create error!\n");
		return -1;
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,&reuse_addr, sizeof(reuse_addr)) < 0) {
		close(sockfd);
		PLATFORM_PRINTF_ERROR("[PLATFORM] socket setsockopt() failed with errno=%d (%s)\n", errno, strerror(errno));
		return -1;
	}

	unlink(HOSTAPD_MAP_ACTION_PATH);

	memset(&s_addr, 0, sizeof(s_addr));
	s_addr.sun_family = AF_UNIX;
	strcpy(s_addr.sun_path, HOSTAPD_MAP_ACTION_PATH);

	ret = bind(sockfd, (struct sockaddr *)&s_addr, sizeof(struct sockaddr_un));
	if(ret < 0)
	{
		printf("bind error!\n");
		close(sockfd);
		return -1;
	}

	ret = listen( sockfd, backlog );
	if(ret == -1)
	{
		printf("listen error!\n");
		close(sockfd);
		return -1;
	}

	*fd = sockfd;
	return 0;
}

int PLATFORM_PROCESS_DPP_BROKER_EVENT(char *msg, INT8U queue_id)
{
	INT8U *pos;
	INT16U frame_len = 0;

	pos = (INT8U *)msg;
	pos++; // header DPP_EASYMESH_RELAY_MESSAGE_HEADER
	PLATFORM_MEMCPY(&frame_len, pos, sizeof(frame_len));
	pos += 2;

	INT16U message_len = 2 + frame_len; // frame_len + frame
	INT8U  message[3 + message_len];

	INT8U message_pointer      = 0;
	message[message_pointer++] = PLATFORM_QUEUE_EVENT_DPP_SEND_PROXIED_ENCAP;

	INT8U *p = NULL;
	p = &message[message_pointer];

	_I2B(&message_len, &p);
	_I2B(&frame_len, &p);

	message_pointer += 4;

	PLATFORM_MEMCPY(&message[message_pointer], pos, frame_len);

	if (0 == sendMessageToAlQueue(queue_id, message, 3 + message_len)) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] *DPP Proxied Encap message to queue for transition failled.\n");
		return -1;
	}

	return 0;
}

int PLATFORM_RECV_HAPD_ACTION(int sockfd, char *buf, int buf_len, INT8U queue_id)
{
	struct sockaddr_un clientaddr;
	socklen_t clientlen = sizeof(clientaddr);
	char recv_buf[HOSTAPD_RECV_MAX_DATA_SIZE] = { 0 };
	int recvbyte = 0;
	int client_fd = accept4(sockfd, (struct sockaddr *)&clientaddr, &clientlen, SOCK_CLOEXEC);

	if (client_fd < 0) {
		PLATFORM_PRINTF_WARNING("[PLATFORM] HAPD accept() failed with errno=%d (%s)\n", errno, strerror(errno));
		return 0;
	}

	PLATFORM_MEMSET(recv_buf, 0, sizeof(recv_buf));
	recvbyte = recv(client_fd, recv_buf, HOSTAPD_RECV_MAX_DATA_SIZE - 1, 0);
	if (recvbyte > 0) {
		if (DPP_EASYMESH_RELAY_MESSAGE_HEADER == recv_buf[0]) {
			PLATFORM_PRINTF("DPP_EASYMESH_RELAY_MESSAGE_HEADER detected, process broker event \n");
			PLATFORM_PROCESS_DPP_BROKER_EVENT(recv_buf, queue_id);
		} else {
			PLATFORM_PRINTF("HAPD EVENT: %s\n", recv_buf);
			snprintf(buf, buf_len, "%s", recv_buf);
		}
	}
	close(client_fd);
	return 0;
}

void PLATFORM_PROCESS_HAPD_EVENT(char *str, INT8U queue_id)
{
	char ifname[IFNAMSIZ] = {0};
	char event_str[256] = {0};
	INT8U notification_activated = 0;
	INT8U event = 0, str_num = 0;
	unsigned int client_mac_addr[6] = {0};
	INT8U bssid[6] = {0};

	str_num = sscanf(str, "%15s %255s %02X:%02X:%02X:%02X:%02X:%02X", ifname, event_str,
			&client_mac_addr[0], &client_mac_addr[1], &client_mac_addr[2],
			&client_mac_addr[3], &client_mac_addr[4], &client_mac_addr[5]);

	if (str_num)
	{
		//receive hapd event...
		if (!PLATFORM_STRCMP(event_str, AP_STA_CONNECTED))
		{
			notification_activated = 1;
			event = CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT_JOIN;
			PLATFORM_GET_MAC(ifname, bssid);
			PLATFORM_PRINTF_INFO("[PLATFORM] HAPD_EVENT: client JOIN. "
				"MAC address %.2x:%.2x:%.2x:%.2x:%.2x:%.2x "
				"BSSID %.2x:%.2x:%.2x:%.2x:%.2x:%.2x \n",
				client_mac_addr[0], client_mac_addr[1],
				client_mac_addr[2], client_mac_addr[3],
				client_mac_addr[4], client_mac_addr[5],
				bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);

		}
		else if (!PLATFORM_STRCMP(event_str, AP_STA_DISCONNECTED))
		{
			notification_activated = 1;
			event = CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT_LEAVE;
			PLATFORM_GET_MAC(ifname, bssid);
			PLATFORM_PRINTF_INFO("[PLATFORM] HAPD_EVENT: client LEAVE. "
				"MAC address %.2x:%.2x:%.2x:%.2x:%.2x:%.2x "
				"BSSID %.2x:%.2x:%.2x:%.2x:%.2x:%.2x \n",
				client_mac_addr[0], client_mac_addr[1],
				client_mac_addr[2], client_mac_addr[3],
				client_mac_addr[4], client_mac_addr[5],
				bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
		}
		else if (!PLATFORM_STRCMP(event_str, BSS_TM_RESP))
		{
			//todo
		}
		else if (!PLATFORM_STRCMP(event_str, AP_STA_POSSIBLE_PSK_MISMATCH))
		{
			if (DMgetReportUnsuccessfulAssociation()) {
				INT16U message_len = 0, value = 0;
				INT8U message_idx = 0;
				INT8U *p = NULL;
				PLATFORM_PRINTF_INFO("[PLATFORM] HAPD_EVENT: AP_STA_POSSIBLE_PSK_MISMATCH "
					"MAC address %.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n",
					client_mac_addr[0], client_mac_addr[1],
					client_mac_addr[2], client_mac_addr[3],
					client_mac_addr[4], client_mac_addr[5]);

				message_len = 10; //payload len is 10
				INT8U message[3 + message_len];

				message[0] = PLATFORM_QUEUE_EVENT_FAILED_CONNECTION_MESSAGE;
				message_idx++;

				p = &message[message_idx];
				_I2B(&message_len, &p);
				message_idx += 2;

				message[message_idx] = (client_mac_addr[0] & 0xff);
				message[message_idx+1] = (client_mac_addr[1] & 0xff);
				message[message_idx+2] = (client_mac_addr[2] & 0xff);
				message[message_idx+3] = (client_mac_addr[3] & 0xff);
				message[message_idx+4] = (client_mac_addr[4] & 0xff);
				message[message_idx+5] = (client_mac_addr[5] & 0xff);
				message_idx += 6;

				value = 1; //_STATS_FAILURE_
				PLATFORM_MEMCPY(&message[message_idx], &value, 2);
				message_idx += 2;

				value = 14; //RSN_MIC_failure
				PLATFORM_MEMCPY(&message[message_idx], &value, 2);
				message_idx += 2;

				if (0 == sendMessageToAlQueue(queue_id, message, 3 + message_len)) {
					PLATFORM_PRINTF_ERROR("[PLATFORM] *Topology change monitor thread* Error sending message to queue for Failed Connection Message\n");
				}
			}
			return;
		} else if (!PLATFORM_STRCMP(event_str, DPP_EVENT_CHIRP_RX)) {
			INT8U  enrollee_mac[6]    = { 0 };
			INT8U  chirp_content[128] = { 0 };
			INT8U  chirp_hex[32];
			INT16U chirp_len = 32;
			INT8U  *p = NULL;

			PLATFORM_PRINTF_INFO("HAPD event, DPP_EVENT_CHIRP_RX received \n");
			sscanf(str, "%15s %*255s id=%*d src=%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX freq=%*d hash=%127s", ifname, &enrollee_mac[0], &enrollee_mac[1], &enrollee_mac[2],
			       &enrollee_mac[3], &enrollee_mac[4], &enrollee_mac[5], chirp_content);
			if (NULL == PLATFORM_STRSTR(ifname, map_vxd_prefix) && (NULL != PLATFORM_STRSTR(ifname, map_radio_name_24g) || NULL != PLATFORM_STRSTR(ifname, map_radio_name_5gl) || NULL != PLATFORM_STRSTR(ifname, map_radio_name_5gh))) {
				sscanf_hex(chirp_content, chirp_hex, chirp_len);
				DMsetDPPEnrollee(enrollee_mac);

				INT16U message_len = 2 + chirp_len + 6; // chirp_len + chirp + enrollee_mac
				INT8U  message[3 + message_len];
				INT8U  message_pointer = 0;

				message[message_pointer++] = PLATFORM_QUEUE_EVENT_DPP_CHIRP_NOTIFICATION;
				p = &message[message_pointer];
				_I2B(&message_len, &p);
				_I2B(&chirp_len, &p);
				_InB(chirp_hex, &p, chirp_len);
				message_pointer = message_pointer + chirp_len + 4 ;
				message[message_pointer++] = enrollee_mac[0];
				message[message_pointer++] = enrollee_mac[1];
				message[message_pointer++] = enrollee_mac[2];
				message[message_pointer++] = enrollee_mac[3];
				message[message_pointer++] = enrollee_mac[4];
				message[message_pointer++] = enrollee_mac[5];

				if (0 == sendMessageToAlQueue(queue_id, message, message_len + 3)) {
					PLATFORM_PRINTF_ERROR("[PLATFORM] *DPP Chirp hostapd notification thread* Error sending message to queue\n");
				}
			}
		} else if (!PLATFORM_STRCMP(event_str, DISCONNECTED)) {
			// This event happens when an agt wpas detects it is disconnected from hapd AP
			// agt should start reconfig process using wpa_cli command (temp)
			PLATFORM_PRINTF_INFO("[PLATFORM] HAPD_EVENT: DISCONNECTED\n");
			PLATFORM_PRINTF_INFO("[PLATFORM] Initialize reconfiguration using wpas\n");

			INT16U message_len = 6; // ruid
			INT8U  message[9]; 		// 3 + 6 (ruid)
			INT8U  message_pointer = 0;
			INT8U *ruid            = DMinterfaceNameToMac(ifname);
			INT8U *p = NULL;

			message[message_pointer++] = PLATFORM_QUEUE_EVENT_WPAS_RECONFIG;
			p = &message[message_pointer];
			_I2B(&message_len, &p);
			message_pointer += 2;
			message[message_pointer++] = ruid[0];
			message[message_pointer++] = ruid[1];
			message[message_pointer++] = ruid[2];
			message[message_pointer++] = ruid[3];
			message[message_pointer++] = ruid[4];
			message[message_pointer++] = ruid[5];
			if (0 == sendMessageToAlQueue(queue_id, message, message_len + 3)) {
				PLATFORM_PRINTF_ERROR("[PLATFORM] *Reconfig wpas notification thread* Error sending message to queue\n");
			}
		} else if(!PLATFORM_STRCMP(event_str, CONNECTED)) {
			PLATFORM_PRINTF_INFO("[PLATFORM] HAPD_EVENT: CONNECTED\n");
			INT8U  message[3];
			message[0] = PLATFORM_QUEUE_EVENT_AP_AUTOCONFIG_SEARCH;
			message[1] = 0x00; // message_len = 0
			message[2] = 0x00; // message_len (cont)
			if (0 == sendMessageToAlQueue(queue_id, message, 3)) {
				PLATFORM_PRINTF_ERROR("[PLATFORM] *AP Autoconfig wpas notification thread* Error sending message to queue\n");
			}
		} else if (!PLATFORM_STRCMP(event_str, DPP_EVENT_RECONFIG_ANNOUNCEMENT)) {
			INT8U enrollee_mac[6]    = { 0 };
			INT8U chirp_content[128] = { 0 };
			INT8U  chirp_hex[32];
			INT16U chirp_len = 32;

			sscanf(str, "%15s %*255s id=%*d src=%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX freq=%*d hash=%127s", ifname, &enrollee_mac[0], &enrollee_mac[1], &enrollee_mac[2],
			       &enrollee_mac[3], &enrollee_mac[4], &enrollee_mac[5], chirp_content);
			if (NULL == PLATFORM_STRSTR(ifname, map_vxd_prefix) && (NULL != PLATFORM_STRSTR(ifname, map_radio_name_24g) || NULL != PLATFORM_STRSTR(ifname, map_radio_name_5gl) || NULL != PLATFORM_STRSTR(ifname, map_radio_name_5gh))) {
				sscanf_hex(chirp_content, chirp_hex, chirp_len);
				DMsetDPPEnrollee(enrollee_mac);

				INT16U message_len = 2 + chirp_len + 6; // chirp_len + chirp + enrollee_mac
				INT8U  message[43]; 					// 3 + 2 (chirp_len) + 32 (chirp) + 6 (enrollee_mac)
				INT8U  *message_pointer;

				message[0] = PLATFORM_QUEUE_EVENT_DPP_RECONFIG_ANNOUNCEMENT;
				message_pointer = &message[1];
				_I2B(&message_len, &message_pointer);
				_I2B(&chirp_len, &message_pointer);
				_InB(&chirp_hex, &message_pointer, chirp_len);
				_InB(&enrollee_mac, &message_pointer, 6);

				if (0 == sendMessageToAlQueue(queue_id, message, message_len + 3)) {
					PLATFORM_PRINTF_ERROR("[PLATFORM] *DPP Chirp hostapd notification thread* Error sending message to queue\n");
				}
			}
		}

		if (1 == notification_activated) {
			INT8U message[3 + 13];
			message[0] = PLATFORM_QUEUE_EVENT_TOPOLOGY_CHANGE_NOTIFICATION;
			message[1] = 0x00;
			message[2] = 0x0D;
			message[3]  = client_mac_addr[0];
			message[4]  = client_mac_addr[1];
			message[5]  = client_mac_addr[2];
			message[6]  = client_mac_addr[3];
			message[7]  = client_mac_addr[4];
			message[8]  = client_mac_addr[5];
			message[9]  = bssid[0];
			message[10] = bssid[1];
			message[11] = bssid[2];
			message[12] = bssid[3];
			message[13] = bssid[4];
			message[14] = bssid[5];
			message[15] = event;
			if (0 == sendMessageToAlQueue(queue_id, message, 3 + 13)) {
				PLATFORM_PRINTF_ERROR("[PLATFORM] *Topology change hostapd notification thread* Error sending message to queue\n");
			}
		}
	}
}

int PLATFORM_EXIT_HAPD_ACTION_SOCKET(int fd)
{
	close(fd);
	return 0;
}

static void *_hostapdNotificationThread(void *p)
{
	fd_set rfds;
	INT8U queue_id;
	int hostapd_sockfd, hostapd_stat;
	char event_buf[256]={0};

	hostapd_stat = PLATFORM_INIT_HAPD_ACTION_SOCKET(&hostapd_sockfd);

	queue_id = ((struct _topologyMonitorThreadData *)p)->queue_id;

	FD_ZERO(&rfds);

	while (1) {
		if(hostapd_stat == 0) {
			FD_CLR(hostapd_sockfd, &rfds);
			FD_SET(hostapd_sockfd, &rfds);
			if(FD_ISSET(hostapd_sockfd, &rfds)) {
				PLATFORM_RECV_HAPD_ACTION(hostapd_sockfd, event_buf, sizeof(event_buf), queue_id);
				PLATFORM_PROCESS_HAPD_EVENT(event_buf, queue_id);
			}
		}
	}

	PLATFORM_PRINTF_INFO("[PLATFORM] *hostapd notification thread* Exiting...\n");

	if(hostapd_stat == 0)
		PLATFORM_EXIT_HAPD_ACTION_SOCKET(hostapd_sockfd);

	free(p);
	return NULL;
}

// *********** DPP bootstrapping info stuff ************************************
// Entering of peer's bootstrap URI / own's private key can be done through
// writing the following tmp files:
//
#define DPP_BOOTSTRAP_PEER_URI_SET_FILENAME "/tmp/dpp_bootstrap_peer_uri_set"
#define DPP_BOOTSTRAP_OWN_PRIV_SET_FILENAME "/tmp/dpp_bootstrap_own_priv_set"
// Other useful parameters:
#define DPP_INOTIFY_BUF_LEN ((sizeof(struct inotify_event) + NAME_MAX + 1))

static void sendBootstrapInfoMessageToAlQueue(INT8U queue_id, INT8U info_type, char *content)
{
	INT8U  message[3 + DPP_BOOTSTRAP_INFO_MAX_LEN] = { 0 };
	INT8U *message_pointer;
	INT16U content_len = (INT16U)strlen(content) + 1; // + null terminator
	INT16U message_len = content_len + 1;             // + info_type

	message[0]      = PLATFORM_QUEUE_EVENT_DPP_BOOTSTRAP_INFO;
	message_pointer = &message[1];
	_I2B(&message_len, &message_pointer);
	_I1B(&info_type, &message_pointer);
	_InB(content, &message_pointer, content_len);

	if (0 == sendMessageToAlQueue(queue_id, message, 3 + message_len)) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] %s error while running sendMessageToAlQueue()\n", __FUNCTION__);
	}
}

struct _dppBootstrapInfoThreadData {
	INT8U queue_id;
};

static void *_dppBootstrapInfoThread(void *p)
{

	// This thread will do the following:
	//
	// a) During initialization, it reads the content of
	//    "DPP_BOOTSTRAP_PEER_URI_SET_FILENAME" and
	//    "DPP_BOOTSTRAP_OWN_PRIV_SET_FILENAME" once.
	//
	// b) Use "inotify" to watch the changes in "DPP_BOOTSTRAP_PEER_URI_SET_FILENAME",
	//    and read it whenever a new content is written.
	//
	// After reading its content, this thread will send it as an dpp bootstrap
	// info message with the following format:
	//
	//   Byte 0:         PLATFORM_QUEUE_EVENT_DPP_BOOTSTRAP_INFO
	//   Byte 1-2:       16-bit size of the content (i.e., 1 + DPP URI / private key)
	//   Byte 3:         Type (Peer Bootstrap URI = 0 / Own Private Key = 1)
	//   Byte 4 onwards: The content (i.e., DPP URI / private key)

	const unsigned int nfds = 1;
	struct pollfd      fdset[nfds];
	FILE *             pFile;
	char               buffer[DPP_INOTIFY_BUF_LEN];
	char               content[DPP_BOOTSTRAP_INFO_MAX_LEN];
	INT8U              queue_id;
	unsigned int       i;

	char *filesToRead[] = {
		DPP_BOOTSTRAP_PEER_URI_SET_FILENAME,
		DPP_BOOTSTRAP_OWN_PRIV_SET_FILENAME
	};

	queue_id = ((struct _dppBootstrapInfoThreadData *)p)->queue_id;
	memset(fdset, 0, sizeof(fdset));

	// read and send the content if files exist
	for (i = 0; i < (sizeof(filesToRead) / sizeof(char *)); i++) {
		if (NULL != (pFile = fopen(filesToRead[i], "r"))) {
			PLATFORM_PRINTF_INFO("[PLATFORM] Reading %s ...\n", filesToRead[i]);
			memset(content, 0, sizeof(content));
			fgets(content, sizeof(content), pFile);
			content[strcspn(content, "\r\n")] = 0;
			fclose(pFile);
			if (strlen(content))
				sendBootstrapInfoMessageToAlQueue(queue_id, i, content);
		}
	}

	// write an empty DPP_BOOTSTRAP_PEER_URI_SET_FILENAME if it does not exist
	if (NULL == (pFile = fopen(DPP_BOOTSTRAP_PEER_URI_SET_FILENAME, "w+"))) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] %s could not create %s\n",
								__FUNCTION__, DPP_BOOTSTRAP_PEER_URI_SET_FILENAME);
		goto exit_thread;
	}
	fclose(pFile);

	// initialize inotify
	if (-1 == (fdset[0].fd = inotify_init())) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] %s inotify_init() returned with errno=%d (%s)\n",
		                      __FUNCTION__, errno, strerror(errno));
		goto exit_thread;
	}

	// use inotify to watch the changes in DPP_BOOTSTRAP_PEER_URI_SET_FILENAME
	if (-1 == inotify_add_watch(fdset[0].fd, DPP_BOOTSTRAP_PEER_URI_SET_FILENAME, IN_CLOSE_WRITE)) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] %s inotify_add_watch() returned with errno=%d (%s)\n",
		                      __FUNCTION__, errno, strerror(errno));
		goto exit_thread;
	}
	fdset[0].events = POLLIN;

	while (1) {

		// the thread will block here until the file has written by someone
		if (0 > poll(fdset, nfds, -1)) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] %s poll() returned with errno=%d (%s)\n",
			                      __FUNCTION__, errno, strerror(errno));
			break;
		}

		if (fdset[0].revents & POLLIN) {
			// consume the event by reading inotifyFd buffer
			if (-1 == read(fdset[0].fd, buffer, sizeof(buffer))) {
				PLATFORM_PRINTF_ERROR("[PLATFORM] %s read() returned with errno=%d (%s)\n",
				                      __FUNCTION__, errno, strerror(errno));
				goto exit_thread;
			}

			// read and send the content of the newly written file
			if (NULL == (pFile = fopen(DPP_BOOTSTRAP_PEER_URI_SET_FILENAME, "r"))) {
				PLATFORM_PRINTF_ERROR("[PLATFORM] %s fopen() returned with errno=%d (%s)\n",
				                      __FUNCTION__, errno, strerror(errno));
				goto exit_thread;
			}
			memset(content, 0, sizeof(content));
			fgets(content, sizeof(content), pFile);
			fclose(pFile);
			content[strcspn(content, "\r\n")] = 0;
			sendBootstrapInfoMessageToAlQueue(queue_id, 0, content);
		}
	}

exit_thread:
	for (i = 0; i < nfds; i++)
		close(fdset[i].fd);

	PLATFORM_PRINTF_ERROR("[PLATFORM] %s EXITED...\n", __FUNCTION__);
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Internal API: to be used by other platform-specific files (functions
// declaration is found in "./platform_os_priv.h")
////////////////////////////////////////////////////////////////////////////////

INT8U sendMessageToAlQueue(INT8U queue_id, INT8U *message, INT16U message_len)
{
	mqd_t mqdes;

	mqdes = queues_id[queue_id];
	if ((mqd_t)-1 == mqdes) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] Invalid queue ID\n");
		return 0;
	}

	if (NULL == message) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] Invalid message\n");
		return 0;
	}

	if (0 != mq_send(mqdes, (const char *)message, message_len, 0)) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] mq_send('%d') returned with errno=%d (%s)\n", queue_id, errno, strerror(errno));
		return 0;
	}

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Platform API: Device information functions to be used by platform-independent
// files (functions declarations are  found in "../interfaces/platform_os.h)
////////////////////////////////////////////////////////////////////////////////

int PLATFORM_GET_DEVICE_INFO(struct deviceInfo *info)
{
	strlcpy(info->model_number, "00000000", sizeof(info->model_number));
	strlcpy(info->serial_number, "00000000", sizeof(info->serial_number));
	strlcpy(info->uuid, "0000000000000000", sizeof(info->uuid));

	if(interface_manufacturer_name == NULL)
		strlcpy(info->manufacturer_name, "Realtek", sizeof(info->manufacturer_name));
	else
		strlcpy(info->manufacturer_name, interface_manufacturer_name, sizeof(info->manufacturer_name));

	if(interface_model_name == NULL)
		strlcpy(info->model_name, "RTL8198D", sizeof(info->model_name));
	else
		strlcpy(info->model_name, interface_model_name, sizeof(info->model_name));

	if(interface_device_name == NULL)
		strlcpy(info->device_name, "Realtek Wireless AP", sizeof(info->device_name));
	else
		strlcpy(info->device_name, interface_device_name, sizeof(info->device_name));

	strlcpy(info->software_version, "V4.0", sizeof(info->software_version));
	strlcpy(info->execution_env, "Linux", sizeof(info->execution_env));

	return 0;
}

long int PLATFORM_GET_QUEUE_SIZE(INT8U queue_id)
{
	struct mq_attr attr;
	mqd_t mqdes = queues_id[queue_id];
	mq_getattr(mqdes, &attr);
	return attr.mq_curmsgs;
}

////////////////////////////////////////////////////////////////////////////////
// Platform API: IPC related functions to be used by platform-independent
// files (functions declarations are  found in "../interfaces/platform_os.h)
////////////////////////////////////////////////////////////////////////////////

INT8U PLATFORM_CREATE_QUEUE(const char *name)
{
	mqd_t          mqdes;
	struct mq_attr attr;
	int            i;
	char           name_tmp[20];

	pthread_mutex_lock(&queues_id_mutex);

	// Note: "0" is not a valid "queue_id"
	// according to the documentation of
	// "PLATFORM_CREATE_QUEUE()". That's why we
	// skip it
	for (i = 1; i < MAX_QUEUE_IDS; i++) {
		if (-1 == queues_id[i]) {
			// Empty slot found.
			//
			break;
		}
	}
	if (MAX_QUEUE_IDS == i) {
		// No more queue id slots available
		//
		pthread_mutex_unlock(&queues_id_mutex);
		return 0;
	}

	if (!name) {
		name_tmp[0] = 0x0;
		sprintf(name_tmp, "/queue_%03d", i);
		name = name_tmp;
	} else if (name[0] != '/') {
		snprintf(name_tmp, 20, "/%s", name);
		name = name_tmp;
	}

	// If a queue with this name already existed (maybe from a previous
	// session), destroy and re-create it
	//
	mq_unlink(name);

	attr.mq_flags   = 0;
	attr.mq_maxmsg  = 100;
	attr.mq_curmsgs = 0;
	attr.mq_msgsize = MAP_MQ_MAX_SIZE;
	//
	// NOTE: The biggest value in the queue is going to be a message from the
	// "pcap" event, which is MAX_NETWORK_SEGMENT_SIZE+3 bytes long.
	// The "PLATFORM_CREATE_QUEUE()" documentation mentions

	if ((mqd_t)-1 == (mqdes = mq_open(name, O_RDWR | O_CREAT | O_CLOEXEC, 0666, &attr))) {
		// Could not create queue
		//
		pthread_mutex_unlock(&queues_id_mutex);
		PLATFORM_PRINTF_ERROR("[PLATFORM] mq_open('%s') returned with errno=%d (%s)\n", name, errno, strerror(errno));
		return 0;
	}

	queues_id[i] = mqdes;

	pthread_mutex_unlock(&queues_id_mutex);
	return i;
}

INT8U PLATFORM_REGISTER_QUEUE_EVENT(INT8U queue_id, INT8U event_type, void *data)
{
	switch (event_type) {
	case PLATFORM_QUEUE_EVENT_NEW_1905_PACKET: {
		pthread_t                      thread;
		struct event1905Packet *       p1;
		struct _pcapCaptureThreadData *p2;
		pthread_attr_t attr;

		if (NULL == data) {
			// 'data' must contain a pointer to a 'struct event1905Packet'
			//
			return 0;
		}

		p1 = (struct event1905Packet *)data;

		p2 = (struct _pcapCaptureThreadData *)malloc(sizeof(struct _pcapCaptureThreadData));
		if (NULL == p2) {
			// Out of memory
			//
			return 0;
		}

		if (DMaddNewListeningInterface(p1->interface_name)) {
			PLATFORM_PRINTF_DEBUG("Already listening to interface %s, will not create another packet capturing thread...\n", p1->interface_name);
			PLATFORM_FREE(p2);
			return 2;
		}

		p2->queue_id       = queue_id;
		p2->interface_name = strdup(p1->interface_name);
		memcpy(p2->interface_mac_address, p1->interface_mac_address, 6);
		memcpy(p2->al_mac_address, p1->al_mac_address, 6);

		pthread_mutex_lock(&pcap_filters_mutex);
		pcap_filters_flag = 0;
		pthread_mutex_unlock(&pcap_filters_mutex);

		pthread_attr_init(&attr);

		pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN * 5);

		pthread_create(&thread, &attr, _pcapLoopThread, (void *)p2);

		// While it is not strictly needed, we will now wait until the PCAP
		// thread registers the needed capture filters.
		//
		pthread_mutex_lock(&pcap_filters_mutex);
		while (0 == pcap_filters_flag) {
			pthread_cond_wait(&pcap_filters_cond, &pcap_filters_mutex);
		}
		pthread_mutex_unlock(&pcap_filters_mutex);

		// NOTE:
		//   The memory allocated by "p2" will be lost forever at this
		//   point (well... until the application exits, that is).
		//   This is considered acceptable.

		break;
	}

	case PLATFORM_QUEUE_EVENT_NEW_ALME_MESSAGE: {
		// The AL entity is telling us that it is capable of processing ALME
		// messages and that it wants to receive ALME messages on the
		// provided queue.
		//
		// In our platform-dependent implementation, we have decided that
		// ALME messages are going to be received on a dedicated thread
		// that runs a TCP server.
		//
		// What we are going to do now is:
		//
		//   1) Create that thread
		//
		//   2) Tell it that everytime a new packet containing ALME
		//      commands arrives on its socket it should forward the
		//      payload to this queue.
		//
		struct almeServerThreadData *p;
		alme_server_flag = 1;
		p                = (struct almeServerThreadData *)malloc(sizeof(struct almeServerThreadData));
		if (NULL == p) {
			// Out of memory
			//
			return 0;
		}
		p->queue_id = queue_id;
		pthread_attr_t attr;
		pthread_attr_init(&attr);

		pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);

		pthread_create(&alme_server_thread, &attr, almeServerThread, (void *)p);

		break;
	}

	case PLATFORM_QUEUE_EVENT_TIMEOUT:
	case PLATFORM_QUEUE_EVENT_TIMEOUT_PERIODIC: {
		struct eventTimeOut *           p1;
		struct _timerHandlerThreadData *p2;

		struct sigevent   se;
		struct itimerspec its;
		timer_t           timer_id;

		p1 = (struct eventTimeOut *)data;

		if (p1->token > MAX_TIMER_TOKEN) {
			// Invalid arguments
			//
			return 0;
		}

		p2 = (struct _timerHandlerThreadData *)malloc(sizeof(struct _timerHandlerThreadData));
		if (NULL == p2) {
			// Out of memory
			//
			return 0;
		}

		p2->queue_id = queue_id;
		p2->token    = p1->token;
		p2->periodic = PLATFORM_QUEUE_EVENT_TIMEOUT_PERIODIC == event_type ? 1 : 0;

		// Next, create the timer. Note that it will be automatically
		// destroyed (by us) in the callback function
		//
		memset(&se, 0, sizeof(se));
		se.sigev_notify          = SIGEV_THREAD;
		se.sigev_notify_function = _timerHandler;
		se.sigev_value.sival_ptr = (void *)p2;

		if (-1 == timer_create(CLOCK_REALTIME, &se, &timer_id)) {
			// Failed to create a new timer
			//
			PLATFORM_PRINTF_ERROR("[PLATFORM] timer_create() returned with errno=%d (%s)\n", errno, strerror(errno));
			free(p2);
			return 0;
		}
		p2->timer_id = timer_id;

		// Finally, arm/start the timer
		//
		its.it_value.tv_sec     = p1->timeout_ms / 1000;
		its.it_value.tv_nsec    = (p1->timeout_ms % 1000) * 1000000;
		its.it_interval.tv_sec  = PLATFORM_QUEUE_EVENT_TIMEOUT_PERIODIC == event_type ? its.it_value.tv_sec : 0;
		its.it_interval.tv_nsec = PLATFORM_QUEUE_EVENT_TIMEOUT_PERIODIC == event_type ? its.it_value.tv_nsec : 0;

		if (0 != timer_settime(timer_id, 0, &its, NULL)) {
			// Problems arming the timer
			//
			free(p2);
			timer_delete(timer_id);
			return 0;
		}

		break;
	}

	case PLATFORM_QUEUE_EVENT_PUSH_BUTTON: {
		// The AL entity is telling us that it is capable of processing
		// "push button" configuration events.
		//
		// Create the thread in charge of generating these events.
		//
		pthread_t                     thread;
		struct _pushButtonThreadData *p;

		p = (struct _pushButtonThreadData *)malloc(sizeof(struct _pushButtonThreadData));
		if (NULL == p) {
			// Out of memory
			//
			return 0;
		}

		p->queue_id = queue_id;
		pthread_attr_t attr;
		pthread_attr_init(&attr);

		pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
		pthread_create(&thread, &attr, _pushButtonThread, (void *)p);
		pthread_detach(thread);

		break;
	}

	case PLATFORM_QUEUE_EVENT_AUTHENTICATED_LINK: {
		// The AL entity is telling us that it is capable of processing
		// "authenticated link" events.
		//
		// We don't really need to do anything here. The interface specific
		// thread will be created when the AL entity calls the
		// "PLATFORM_START_PUSH_BUTTON_CONFIGURATION()" function.

		break;
	}

	case PLATFORM_QUEUE_EVENT_TOPOLOGY_CHANGE_NOTIFICATION: {
		// The AL entity is telling us that it is capable of processing
		// "topology change" events.
		//
		// We will create a new thread in charge of monitoring the local
		// topology to generate these events.
		//
		pthread_t                          thread;
		struct _topologyMonitorThreadData *p;

		p = (struct _topologyMonitorThreadData *)malloc(sizeof(struct _topologyMonitorThreadData));
		if (NULL == p) {
			// Out of memory
			//
			return 0;
		}

		p->queue_id = queue_id;

		pthread_attr_t attr;
		pthread_attr_init(&attr);

		pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN * 3);

		pthread_create(&thread, &attr, _topologyMonitorThread, (void *)p);

		break;
	}

	case PLATFORM_QUEUE_EVENT_HOSTAPD_NOTIFICATION: {
		// The AL entity is telling us that it is capable of processing
		// "hostapd notification" events.
		//
		// We will create a new thread for receive hostapd events
		pthread_t	thread;
		struct		_hostapdNotificationThreadData *p;

		p = (struct _hostapdNotificationThreadData *)malloc(sizeof(struct _hostapdNotificationThreadData));
		if (NULL == p) {
			// Out of memory
			//
			return 0;
		}

		p->queue_id = queue_id;

		pthread_attr_t attr;
		pthread_attr_init(&attr);

		pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);

		pthread_create(&thread, &attr, _hostapdNotificationThread, (void *)p);

		break;
	}

	case PLATFORM_QUEUE_EVENT_DPP_BOOTSTRAP_INFO: {
		// The AL entity is telling us that it is capable of processing
		// "dpp bootstrap info" configuration events.
		//
		// Create the thread in charge of generating these events.
		//
		pthread_t                           thread;
		pthread_attr_t                      attr;
		struct _dppBootstrapInfoThreadData *p;

		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
		pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);

		p = (struct _dppBootstrapInfoThreadData *)malloc(sizeof(struct _dppBootstrapInfoThreadData));
		if (NULL == p) {
			// Out of memory
			//
			return 0;
		}

		p->queue_id = queue_id;

		pthread_create(&thread, &attr, _dppBootstrapInfoThread, (void *)p);

		break;
	}

	default: {
		// Unknown event type!!
		//
		return 0;
	}
	}

	return 1;
}

INT8U PLATFORM_READ_QUEUE(INT8U queue_id, INT8U *message_buffer)
{
	mqd_t   mqdes;
	ssize_t len;

	mqdes = queues_id[queue_id];
	if ((mqd_t)-1 == mqdes) {
		// Invalid ID
		return 1;
	}

	len = mq_receive(mqdes, (char *)message_buffer, MAP_MQ_MAX_SIZE, NULL);

	if (-1 == len) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] mq_receive() returned with errno=%d (%s)\n", errno, strerror(errno));
		return 0;
	}

	// All messages are TLVs where the second and third bytes indicate the
	// total length of the payload. This value *must* match "len-3"
	//
	if (len < 0) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] mq_receive() returned than 3 bytes (minimum TLV size)\n");
		return 0;
	} else {
		INT16U payload_len;

		PLATFORM_PRINTF_DETAIL("[PLATFORM] Receiving %d bytes from queue (%02x, %02x, %02x, ...)\n", len, message_buffer[0], message_buffer[1], message_buffer[2]);

		payload_len = *(((INT8U *)message_buffer) + 1) * 256 + *(((INT8U *)message_buffer) + 2);

		if (payload_len != len - 3) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] mq_receive() returned %d bytes, but the TLV is %d bytes\n", len, payload_len + 3);
			return 0;
		}
	}

	return 1;
}

INT8U PLATFORM_MONITOR_VLAN_INTERFACES(INT8U queue_id, INT16U primary_vid)
{
	char **interfaces_names;
	INT8U  interfaces_nr;
	int    i;
	INT8U bss_type = 0;
	char vlan_interface[64];
	interfaces_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&interfaces_nr);
	for (i = 0; i < interfaces_nr; i++) {
		struct event1905Packet aux;
		if(NULL ==PLATFORM_STRSTR(interfaces_names[i], "wlan")){
			continue;
		}
		char *base_interface_name = trimInterfaceNameVID(interfaces_names[i], NULL, NULL);

		bss_type = PLATFORM_GET_BSS_TYPE(base_interface_name);

		if ((bss_type & BACKHAUL_BSS) || (bss_type & BACKHAUL_STA)) {
			sprintf(vlan_interface, "%s.%d", base_interface_name, primary_vid);
			PLATFORM_PRINTF_DEBUG("Starting capturing vlan interfaces %s...\n", vlan_interface);
			if (PLATFORM_IS_INTERFACE_UP(vlan_interface)) {
				aux.interface_name = vlan_interface;
				PLATFORM_MEMCPY(aux.interface_mac_address, DMinterfaceNameToMac(aux.interface_name), 6);
				PLATFORM_MEMCPY(aux.al_mac_address, DMalMacGet(), 6);
				if (0 == PLATFORM_REGISTER_QUEUE_EVENT(queue_id, PLATFORM_QUEUE_EVENT_NEW_1905_PACKET, &aux)) {
					PLATFORM_PRINTF_ERROR("Could not register callback for 1905 packets in interface %s\n", vlan_interface);
					PLATFORM_FREE(base_interface_name);
					return 1;
				}
				PLATFORM_PRINTF_DEBUG("    - %s --> OK\n", vlan_interface);
			}
		}
		PLATFORM_FREE(base_interface_name);
	}
	return 0;
}

INT8U PLATFORM_STOP_MONITOR_VLAN_INTERFACES(INT8U queue_id, INT16U primary_vid)
{
	char **interfaces_names;
	INT8U  interfaces_nr;
	int    i;
	INT8U bss_type = 0;

	interfaces_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&interfaces_nr);
	for (i = 0; i < interfaces_nr; i++) {
		struct event1905Packet aux;
		if(NULL ==PLATFORM_STRSTR(interfaces_names[i], "wlan")){
			continue;
		}
		char *base_interface_name = trimInterfaceNameVID(interfaces_names[i], NULL, NULL);
		bss_type = PLATFORM_GET_BSS_TYPE(base_interface_name);

		if ((bss_type & BACKHAUL_BSS) || (bss_type & BACKHAUL_STA)) {
			PLATFORM_PRINTF_DEBUG("Stop captruing %s.%d\n", base_interface_name, primary_vid);
			// TODO: Stop VLAN interface packer capturing
			if (PLATFORM_IS_INTERFACE_UP(base_interface_name)) {
				aux.interface_name = base_interface_name;
				PLATFORM_MEMCPY(aux.interface_mac_address, DMinterfaceNameToMac(aux.interface_name), 6);
				PLATFORM_MEMCPY(aux.al_mac_address, DMalMacGet(), 6);
				if (0 == PLATFORM_REGISTER_QUEUE_EVENT(queue_id, PLATFORM_QUEUE_EVENT_NEW_1905_PACKET, &aux)) {
					PLATFORM_PRINTF_ERROR("Could not register callback for 1905 packets in interface %s\n", base_interface_name);
					PLATFORM_FREE(base_interface_name);
					return 1;
				}
				PLATFORM_PRINTF_DEBUG("    - %s --> OK\n", base_interface_name);
			}
		}
		PLATFORM_FREE(base_interface_name);
	}
	return 0;
}

INT8U CLOSE_ALME_THREAD()
{
	char *ret;
	if (alme_server_flag) {
		alme_server_flag = 0;
		pthread_kill(alme_server_thread, SIGUSR2);
		pthread_join(alme_server_thread, (void **)&ret);
		PLATFORM_PRINTF_DEBUG("[CLOSE ALME] %s\n", ret);
	}
	return 1;
}
