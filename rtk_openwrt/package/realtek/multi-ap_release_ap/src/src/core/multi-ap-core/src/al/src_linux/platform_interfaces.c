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

#include <errno.h>  // errno
#include <stdarg.h> // va_*
#include <stdio.h>  // printf(), popen()
#include <stdlib.h> // malloc(), ssize_t
#include <string.h> // strdup()
#include <unistd.h> // sleep()

#include <arpa/inet.h>       // socket, AF_INTER, htons(), ...
#include <linux/if_packet.h> // sockaddr_ll
#include <netinet/ether.h> // ETH_P_ALL, ETH_A_LEN
#include <pthread.h>       // pthread_create(), mutex functions
#include <limits.h>        // threads and mutex functions
#include <sys/ioctl.h>     // ioctl(), SIOCGIFINDEX
#include <unistd.h>        // close()
#include <linux/wireless.h>
#include <features.h>

#include "platform_interfaces.h"
#include "platform.h"
#include "platform_interfaces_priv.h"
#include "platform_os.h"
#include "platform_os_priv.h"
#include "multi_ap_tlvs.h"
#include "al_utils.h"
#include "al_datamodel.h"

#if defined(CONFIG_RTK_PON_XDSL) || !defined(__UCLIBC__)
#include "utils.h"
#endif

#ifdef _RTK_LINUX_
#	include "platform_interfaces_rtk_priv.h"
#endif

#include "global_settings.h"
#include "packet_tools.h"

// The maximun eth frame size should be 1,514 bytes = MAX_NETWORK_SEGMENT_SIZE(1500) + sizeof eth header(14)
// Eth Header Details:
// eh->ether_dhost[0] = dst_mac[0];
// eh->ether_dhost[1] = dst_mac[1];
// eh->ether_dhost[2] = dst_mac[2];
// eh->ether_dhost[3] = dst_mac[3];
// eh->ether_dhost[4] = dst_mac[4];
// eh->ether_dhost[5] = dst_mac[5];
// eh->ether_shost[0] = src_mac[0];
// eh->ether_shost[1] = src_mac[1];
// eh->ether_shost[2] = src_mac[2];
// eh->ether_shost[3] = src_mac[3];
// eh->ether_shost[4] = src_mac[4];
// eh->ether_shost[5] = src_mac[5];
// eh->ether_type     = htons(eth_type); // 2 bytes
#define MAX_ETH_FRAME_SEGMENT_SIZE (MAX_NETWORK_SEGMENT_SIZE + 14)

////////////////////////////////////////////////////////////////////////////////
// Private data and functions
////////////////////////////////////////////////////////////////////////////////

// Global variables to store the list of names of the interfaces and their
// status.
//
static int    interfaces_nr                   = 0;
static char **interfaces_list                 = NULL;
static char **interfaces_list_extended_params = NULL;
static char **interfaces_list_ssids           = NULL;

// Should sync with actual ethernet port ifname
static struct eth_port_map global_eth_recv_1905[4] = {
	{ "eth0.2.0", 0 },
	{ "eth0.3.0", 0 },
	{ "eth0.4.0", 0 },
	{ "eth0.5.0", 0 },
};

// The interfaces status variables can be accessed/modified from different
// threads:
//   - The "main" AL thread (where "start1905AL()" runs)
//   - The "push button" configuration process thread
//
// Thus, their access must be protected with a mutex
//
pthread_mutex_t interface_mutex = PTHREAD_MUTEX_INITIALIZER;

// This function returns '0' if there was a problem,  otherwise it returns an
// ID identifying the interface type.
//
#define INTF_TYPE_ERROR (0)
#define INTF_TYPE_ETHERNET (1)
#define INTF_TYPE_WIFI (2)
#define INTF_TYPE_UNKNOWN (0xFF)
INT8U _getInterfaceType(char *interface_name)
{
	// According to www.kernel.org/doc/Documentation/ABI/testing/sysfs-class-net
	//
	//                                    Regular ethernet          Wifi
	//                                    interface                 interface
	//                                    ================          =========
	//
	// /sys/class/net/<iface>/type        1                         1
	//
	// /sys/class/net/<iface>/wireless    <Does not exist>          <Exists>

	INT8U ret;

	FILE *fp;

	char sys_path[100];

	ret = INTF_TYPE_ERROR;

	snprintf(sys_path, sizeof(sys_path), "/sys/class/net/%s/type", interface_name);

	if (NULL != (fp = fopen(sys_path, "re"))) {
		char aux[30];

		if (NULL != fgets(aux, sizeof(aux), fp)) {
			int interface_type;

			interface_type = atoi(aux);

			switch (interface_type) {
			case 1: {
				snprintf(sys_path, sizeof(sys_path), "/sys/class/net/%s/wireless", interface_name);

				if (-1 != access(sys_path, F_OK)) {
					// Wireless
					//
					ret = INTF_TYPE_WIFI;
				} else {
					// Ethernet
					//
					ret = INTF_TYPE_ETHERNET;
				}
				break;
			}

			default: {
				PLATFORM_PRINTF_ERROR("[PLATFORM] Unknow interface type %d\n", interface_type);
				ret = INTF_TYPE_UNKNOWN;

				break;
			}
			}
		}

		fclose(fp);
	}

	return ret;
}

// These are the data structures and functions in charge of handling the press
// of physical/virtual buttons on the platform
//
struct _pushButtonThreadData {
	INT8U  queue_id;
	char * interface_name;
	INT8U  al_mac_address[6];
	INT16U mid;
};
static void *_pushButtonConfigurationThread(void *p)
{
	// This function is executed when the "push button" configuration mechanism
	// is started on an interface.
	//
	// It will wait until the process either:
	//
	//   A) Fails. In this case the interface will remain on its previous state
	//     (either "secure" or "not secure")
	//
	//   B) Succeeds. In this case the interface status will be set to
	//      "authenticated", no matter what its previous state was,  and a new
	//      "PLATFORM_QUEUE_EVENT_AUTHENTICATED_LINK" message will be posted to
	//      the AL queue.
	//      This message contains both the just autenticated local interface MAC
	//      address and the MAC of the interface at the other end of this same
	//      link.
	//
	// Behaviour depending in the interface type:
	//
	//   - WIFI: (TODO)
	//       The WPS procedure starts and if it succeeds a message is posted to
	//       the AL queue containing the MAC address of the wifi node at the
	//       other end of the now encrypted link.

	struct _pushButtonThreadData *aux;
	struct interfaceInfo *        x;

	int           push_button_result = 0;
	unsigned char new_mac[6];
	INT16U        interface_type;
	INT8U         local_interface_mac_address[6];
	char          alternative_interface[20];

	char *wsc_interface;

	INT8U wsc_wait_count = 0;

	INT8U wsc_fail = 0;

	INT8U wsc_attempt = 0;

	aux = (struct _pushButtonThreadData *)p;

	PLATFORM_PRINTF_DETAIL("[PLATFORM] *Push button configuration thread* Starting on interface %s\n", aux->interface_name);

	x = PLATFORM_GET_1905_INTERFACE_INFO(aux->interface_name);
	if (NULL == x) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] *Push button configuration thread* Error retrieving interface %s information\n", aux->interface_name);
		PLATFORM_FREE(p);
		return NULL;
	}

	strlcpy(alternative_interface, aux->interface_name, sizeof(alternative_interface));
	if (map_radio_name_5gh[4] == aux->interface_name[4] || map_radio_name_5gl[4] == aux->interface_name[4]) {
		alternative_interface[4] = map_radio_name_24g[4];
	} else if (map_radio_name_24g[4] == aux->interface_name[4]) {
		alternative_interface[4] = map_radio_name_5gh[4];
	} else {
		PLATFORM_PRINTF_ERROR("Unknown alternative interface for %s!\n", aux->interface_name);
		alternative_interface[0] = '\0';
	}

	PLATFORM_PRINTF_DEBUG("Alternative wsc interface %s\n", alternative_interface);

	interface_type = x->interface_type;
	PLATFORM_MEMCPY(local_interface_mac_address, x->mac_address, 6);

	PLATFORM_FREE_1905_INTERFACE_INFO(x);

	new_mac[0] = 0x00;
	new_mac[1] = 0x00;
	new_mac[2] = 0x00;
	new_mac[3] = 0x00;
	new_mac[4] = 0x00;
	new_mac[5] = 0x00;

	// Start the "push button process"

	switch (interface_type) {
	case INTERFACE_TYPE_IEEE_802_11B_2_4_GHZ:
	case INTERFACE_TYPE_IEEE_802_11G_2_4_GHZ:
	case INTERFACE_TYPE_IEEE_802_11A_5_GHZ:
	case INTERFACE_TYPE_IEEE_802_11N_2_4_GHZ:
	case INTERFACE_TYPE_IEEE_802_11N_5_GHZ:
	case INTERFACE_TYPE_IEEE_802_11AC_5_GHZ:
	case INTERFACE_TYPE_IEEE_802_11AD_60_GHZ:
	case INTERFACE_TYPE_IEEE_802_11AF:
	case INTERFACE_TYPE_IEEE_802_11AX: {
		//
		PLATFORM_TRIGGER_WPS(aux->interface_name);
		break;
	}
	case INTERFACE_TYPE_UNKNOWN: {
		PLATFORM_PRINTF_ERROR("[PLATFORM] *Push button configuration thread* Unknown interface type\n");
		PLATFORM_FREE(p);
		return NULL;
	}
	default: {
		PLATFORM_PRINTF_ERROR("[PLATFORM] *Push button configuration thread* Impossible interface type %d\n", interface_type);
		PLATFORM_FREE(p);
		return NULL;
	}
	}

	wsc_interface = aux->interface_name;

	while (wsc_attempt < 2) {
		while (!wsc_fail) {
			x = PLATFORM_GET_1905_INTERFACE_INFO_WITH_WPS(wsc_interface, 1);

			if (NULL == x) {
				PLATFORM_PRINTF_ERROR("[PLATFORM] *Push button configuration thread* Error retrieving interface %s information\n", wsc_interface);
				break;
			}

			if (1 == x->push_button_on_going) {
				// Ensure the "push button process" has started!
				PLATFORM_FREE_1905_INTERFACE_INFO(x);
				break;
			}
			PLATFORM_FREE_1905_INTERFACE_INFO(x);
			PLATFORM_PRINTF_DEBUG("[WSC] push button process not started yet\n");
			usleep(100 * 1000);
			wsc_wait_count++;
			if (wsc_wait_count >= 30) {
				PLATFORM_PRINTF_DEBUG("[WSC] push button process did not start after maximum wait, exiting thread...\n");
				wsc_fail = 1;
			}
		}

		wsc_wait_count = 0;
		if (!wsc_fail) {
			PLATFORM_PRINTF_DEBUG("[WSC] push button process start on %s, waiting for result...\n", wsc_interface);
		}

		// Now wait until the process finishes
		//
		while (!wsc_fail) {
			x = PLATFORM_GET_1905_INTERFACE_INFO_WITH_WPS(wsc_interface, 1);

			if (NULL == x) {
				PLATFORM_PRINTF_ERROR("[PLATFORM] *Push button configuration thread* Error retrieving interface %s information\n", wsc_interface);

				push_button_result = 0;
				break;
			}

			if (0 == x->push_button_on_going) {
				// The "push button process" has finished!
				//
#ifdef _RTK_LINUX_
				push_button_result = x->wsc_result;
#else
				push_button_result = 1;
#endif
				PLATFORM_PRINTF_DEBUG("[WSC] wsc finished on %s with %d\n", wsc_interface, push_button_result);
				if (1 == push_button_result) {
					new_mac[0] = x->push_button_new_mac_address[0];
					new_mac[1] = x->push_button_new_mac_address[1];
					new_mac[2] = x->push_button_new_mac_address[2];
					new_mac[3] = x->push_button_new_mac_address[3];
					new_mac[4] = x->push_button_new_mac_address[4];
					new_mac[5] = x->push_button_new_mac_address[5];
				}

				PLATFORM_FREE_1905_INTERFACE_INFO(x);
				break;
			}

			PLATFORM_FREE_1905_INTERFACE_INFO(x);

			PLATFORM_PRINTF_DETAIL("[PLATFORM] *Push button configuration thread* Push button ongoing on interface %s...\n", wsc_interface);
			sleep(5);
			wsc_wait_count++;
			if (wsc_wait_count >= 30) {
				PLATFORM_PRINTF_DEBUG("[WSC] push button process still not finished after maximum wait, exiting thread...\n");
				wsc_fail = 1;
			}
		}

		if(customize_features & SUPPORT_NOT_WPS_CYCLE)
		{
			PLATFORM_PRINTF_DEBUG("wps not cycle.\n");
			break;
		}

		wsc_attempt++;

		if (((0 == push_button_result) || wsc_fail) && PLATFORM_STRCMP(wsc_interface, alternative_interface)) {
			PLATFORM_PRINTF_DEBUG("wsc attempt on %s failed, will try on %s\n", wsc_interface, alternative_interface);
			if (PLATFORM_IS_INTERFACE_UP_AND_ON(alternative_interface)) {
				if (NULL == strstr(aux->interface_name, map_vxd_prefix)) {
					x = PLATFORM_GET_1905_INTERFACE_INFO_WITH_WPS(alternative_interface, 1);
					if (NULL == x) {
						PLATFORM_PRINTF_ERROR("[PLATFORM] *Push button configuration thread* Error retrieving interface %s information\n", alternative_interface);
						break;
					}
					if (x->wsc_result != 0) {
						push_button_result |= 0x10;
					}
					PLATFORM_FREE_1905_INTERFACE_INFO(x);
				}
				if ((push_button_result & 0x10) != 0x10) {
					PLATFORM_TRIGGER_WPS(alternative_interface);
					wsc_interface = alternative_interface;
					if (wsc_fail) {
						wsc_fail = 0;
					}
				}
				push_button_result &= 0x0f;
			} else {
				PLATFORM_PRINTF_DEBUG("Altervative interface %s is down, try on original interface %s again.\n", alternative_interface, wsc_interface);
				PLATFORM_TRIGGER_WPS(wsc_interface);
			}
		} else if (1 == push_button_result) {
			break;
		}
	}

	if ((0 == push_button_result) || wsc_fail) {
		PLATFORM_PRINTF_WARNING("[PLATFORM] *Push button configuration thread* Timeout or error on interface %s. Stopping...\n", wsc_interface);
	} else {
		PLATFORM_PRINTF_DETAIL("[PLATFORM] *Push button configuration thread* Success! New device (%02x:%02x:%02x:%02x:%02x:%02x) on interface %s. Stopping...\n", new_mac[0], new_mac[1], new_mac[2], new_mac[3], new_mac[4], new_mac[5], aux->interface_name);

		// Post a "PLATFORM_QUEUE_EVENT_AUTHENTICATED_LINK" message
		//
		{
#ifdef _RTK_LINUX_
			INT8U message[3 + 20 + 1];
#else
			INT8U message[3 + 20];
#endif
			message[0]  = PLATFORM_QUEUE_EVENT_AUTHENTICATED_LINK;
			message[1]  = 0x00;
			message[2]  = 0x14;
			message[3]  = local_interface_mac_address[0];
			message[4]  = local_interface_mac_address[1];
			message[5]  = local_interface_mac_address[2];
			message[6]  = local_interface_mac_address[3];
			message[7]  = local_interface_mac_address[4];
			message[8]  = local_interface_mac_address[5];
			message[9]  = new_mac[0];
			message[10] = new_mac[1];
			message[11] = new_mac[2];
			message[12] = new_mac[3];
			message[13] = new_mac[4];
			message[14] = new_mac[5];
			message[15] = aux->al_mac_address[0];
			message[16] = aux->al_mac_address[0];
			message[17] = aux->al_mac_address[0];
			message[18] = aux->al_mac_address[0];
			message[19] = aux->al_mac_address[0];
			message[20] = aux->al_mac_address[0];

#if _HOST_IS_LITTLE_ENDIAN_ == 1
			message[21] = *(((INT8U *)&aux->mid) + 1);
			message[22] = *(((INT8U *)&aux->mid) + 0);
#else
			message[21] = *(((INT8U *)&aux->mid) + 0);
			message[22] = *(((INT8U *)&aux->mid) + 1);
#endif
#ifdef _RTK_LINUX_
			if (strstr(wsc_interface, map_vxd_prefix)) {
				message[23] = 0x01;
			} else {
				message[23] = 0x00;
			}
#endif
			PLATFORM_PRINTF_DETAIL("[PLATFORM] *Push button configuration thread* Sending 23 bytes to queue (0x%02x, 0x%02x, 0x%02x, ...)\n", message[0], message[1], message[2]);

			if (0 == sendMessageToAlQueue(aux->queue_id, message, 3 + 20)) {
				PLATFORM_PRINTF_ERROR("[PLATFORM] *Push button configuration thread* Error sending message to queue from _pushButtonThread()\n");
				PLATFORM_FREE(p);
				return NULL;
			}
		}
	}

	PLATFORM_FREE(p);
	return NULL;
}

// Returns an INT32U obtained by reading the first line of file
// "/sys/class/net/<interface_name>/<parameter_name>"
//
static INT32S _readInterfaceParameter(char *interface_name, char *parameter_name)
{
	INT32S ret;

	FILE *fp;

	char sys_path[100];

	ret = 0;

	snprintf(sys_path, sizeof(sys_path), "/sys/class/net/%s/%s", interface_name, parameter_name);

	if (NULL != (fp = fopen(sys_path, "re"))) {
		char aux[30];

		if (NULL != fgets(aux, sizeof(aux), fp)) {
			ret = atoi(aux);
		}
		fclose(fp);
	}

	return ret;
}

#define PROC_DIR "/proc/net/rtk_wifi6/"

// Returns an INT32S obtained by reading the output of
// "iw dev $INTERFACE station get $MAC | grep $PARAMETER_NAME"
//
static INT32S _readWifiNeighborParameter(char *interface_name, INT8U *neighbor_interface_address, char *parameter_name)
{
	INT32S ret;
	FILE * pipe;
	char * line;
	char * line_break = NULL;
	size_t len;

	char command[200];

	ret = 0;

	if ((NULL == interface_name) || (NULL == neighbor_interface_address) || (NULL == parameter_name)) {
		return 0;
	}
#ifdef _RTK_LINUX_
	int count      = 0;
	int start_line = 0;
	char file_path[64] = {0};

	snprintf(file_path, sizeof(file_path), PROC_DIR "%s/sta_info", interface_name);

	if (-1 == access(file_path, F_OK)) {
		PLATFORM_PRINTF_ERROR("File %s does not exist!\n", file_path);
		return 0;
	}

	snprintf(command, sizeof(command), "cat %s | grep -n %02x%02x%02x%02x%02x%02x",
		        file_path,
		        neighbor_interface_address[0], neighbor_interface_address[1], neighbor_interface_address[2],
		        neighbor_interface_address[3], neighbor_interface_address[4], neighbor_interface_address[5]);

	pipe = popen(command, "re");

	if (!pipe) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] popen() returned with errno=%d (%s)\n", errno, strerror(errno));
		return 0;
	}

	line = NULL;
	if (-1 != getline(&line, &len, pipe)) {

		// Remove the last "\n"
		//
		line[strlen(line) - 1] = 0x00;

		line_break = PLATFORM_STRSTR(line, ":");
		if (line_break) {
			line_break[0] = '\0';
			sscanf(line, "%d", &ret);
			PLATFORM_PRINTF_DETAIL("[PLATFORM] Neighbor %02x:%02x:%02x:%02x:%02x:%02x is under (%s) around %d\n",
			                       neighbor_interface_address[0], neighbor_interface_address[1], neighbor_interface_address[2],
			                       neighbor_interface_address[3], neighbor_interface_address[4], neighbor_interface_address[5],
			                       interface_name, ret);
		} else {
			PLATFORM_PRINTF_WARNING("[PLATFORM] Parameter not found\n");
			free(line);
			pclose(pipe);
			return 0;
		}
	} else {
		PLATFORM_PRINTF_DETAIL("[PLATFORM] Station is not found under %s\n", interface_name);
		free(line);
		pclose(pipe);
		return 0;
	}

	free(line);
	pclose(pipe);

	pipe       = fopen(file_path, "re");
	start_line = ret;

	if (!pipe) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] popen() returned with errno=%d (%s)\n", errno, strerror(errno));
		return 0;
	}

	line = PLATFORM_MALLOC(sizeof(char) * 256);

	while (fgets(line, 256, pipe) != NULL) {
		if (count >= start_line) {
			line[strlen(line) - 1] = 0x00;
			char *tmp              = PLATFORM_STRSTR(line, parameter_name);
			if (tmp) {
				tmp += strlen(parameter_name);
				if (0 == PLATFORM_STRCMP("current_tx_rate:", parameter_name)) {
					char cap_string[10];
					char nss_string[20];
					if (-1 == sscanf(tmp, "%9s %19s %d", cap_string, nss_string, &ret)) {
						PLATFORM_PRINTF_WARNING("Parsing current_tx_rate failed, line is <%s>\n", tmp);
						ret = 0;
					}
					// PLATFORM_PRINTF_DEBUG("current_tx_rate result %s %s %d\n", cap_string, nss_string, ret);
				} else {
					sscanf(tmp, "%d", &ret);
				}
				PLATFORM_PRINTF_DEBUG("Neighbor %02x:%02x:%02x:%02x:%02x:%02x %s %d.\n", neighbor_interface_address[0], neighbor_interface_address[1], neighbor_interface_address[2],
				                      neighbor_interface_address[3], neighbor_interface_address[4], neighbor_interface_address[5],
				                      parameter_name, ret);
				break;
			}
		}
		count++;
	}

	fclose(pipe);
	free(line);
#else
	sprintf(command, "iw dev %s station get %02x:%02x:%02x:%02x:%02x:%02x | grep %s",
	        interface_name,
	        neighbor_interface_address[0], neighbor_interface_address[1], neighbor_interface_address[2],
	        neighbor_interface_address[3], neighbor_interface_address[4], neighbor_interface_address[5],
	        parameter_name);

	// Execute the IW query command
	//
	pipe = popen(command, "re");

	if (!pipe) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] popen() returned with errno=%d (%s)\n", errno, strerror(errno));
		return 0;
	}

	// Next read the parameter
	//
	line = NULL;
	if (-1 != getline(&line, &len, pipe)) {
		char *value;

		// Remove the last "\n"
		//
		line[strlen(line) - 1] = 0x00;

		value = strstr(line, ":");
		if ((NULL != value) && (1 == sscanf(value + 1, "%d", &ret))) {
			PLATFORM_PRINTF_DETAIL("[PLATFORM] Neighbor %02x:%02x:%02x:%02x:%02x:%02x (%s) %d = %d\n",
			                       neighbor_interface_address[0], neighbor_interface_address[1], neighbor_interface_address[2],
			                       neighbor_interface_address[3], neighbor_interface_address[4], neighbor_interface_address[5],
			                       interface_name, parameter_name, ret);
		} else {
			PLATFORM_PRINTF_DETAIL("[PLATFORM] Parameter not found\n");
		}
	}

	free(line);
	pclose(pipe);
#endif
	return ret;
}

void addInterface(char *long_interface_name)
{
	char *p1, *p2;
	char *save_ptr;

	int i = 0;

	p1 = strdup(long_interface_name);
	strtok_r(p1, ":", &save_ptr);
	p2 = strtok_r(NULL, "", &save_ptr);

	if (NULL != p2) {
		p2 = strdup(p2);
	}

	if (!PLATFORM_IS_INTERFACE_VALID(p1)) {
		PLATFORM_FREE(p1);
		PLATFORM_FREE(p2);
		return;
	}

	for(i = 0; i < interfaces_nr; i++){
		if(0 == PLATFORM_STRCMP(p1, interfaces_list[i])) {
			PLATFORM_FREE(p1);
			PLATFORM_FREE(p2);
			return;
		}
	}

	if (0 == interfaces_nr) {
		interfaces_list                 = (char **)PLATFORM_MALLOC((sizeof(char *)) * 1);
		interfaces_list_extended_params = (char **)PLATFORM_MALLOC((sizeof(char *)) * 1);
		interfaces_list_ssids           = (char **)PLATFORM_MALLOC((sizeof(char *)) * 1);
	} else {
		interfaces_list                 = PLATFORM_REALLOC(interfaces_list, (sizeof(char *)) * (interfaces_nr + 1));
		interfaces_list_extended_params = PLATFORM_REALLOC(interfaces_list_extended_params, (sizeof(char *)) * (interfaces_nr + 1));
		interfaces_list_ssids           = PLATFORM_REALLOC(interfaces_list_ssids, (sizeof(char *)) * (interfaces_nr + 1));
	}

	// The interface name can be either something like this:
	//
	//   "eth0"
	//
	// ...or like this:
	//
	//   "eth0:param_1:123:param_2:09"
	//
	// Everything *before* the first ":" will be saved into "interfaces_list",
	// while everything *after* that will be saved into
	// "interfaces_list_extended_params"
	//

	interfaces_list[interfaces_nr]                 = p1;
	interfaces_list_extended_params[interfaces_nr] = p2;
	interfaces_list_ssids[interfaces_nr] = NULL;

	if (NULL != p2) {
		PLATFORM_PRINTF_DETAIL("[PLATFORM] Added interface %s with additional parameters (%s)\n", interfaces_list[interfaces_nr], interfaces_list_extended_params[interfaces_nr]);
	} else {
		PLATFORM_PRINTF_DEBUG("[PLATFORM] Added interface %s\n", interfaces_list[interfaces_nr]);
	}

	interfaces_nr++;

	return;
}

////////////////////////////////////////////////////////////////////////////////
// Platform API: Interface related functions to be used by platform-independent
// files (functions declarations are  found in "../interfaces/platform.h)
////////////////////////////////////////////////////////////////////////////////
char **PLATFORM_GET_LIST_OF_1905_INTERFACES(INT8U *nr)
{
	*nr = interfaces_nr;

	return interfaces_list;
}

void PLATFORM_FREE_LIST_OF_1905_INTERFACES(__attribute__((unused)) char **x, __attribute__((unused)) INT8U nr)
{
	// The list must never be freed, so that future calls to
	// "PLATFORM_GET_1905_INTERFACE_INFO()" can make use of it
	//
	return;
}

struct interfaceInfo *PLATFORM_GET_1905_INTERFACE_INFO(char *interface_name)
{
	return PLATFORM_GET_1905_INTERFACE_INFO_WITH_WPS(interface_name, 0);
}

struct interfaceInfo *PLATFORM_GET_1905_INTERFACE_INFO_WITH_WPS(char *interface_name, INT8U need_wps)
{
	struct interfaceInfo *m;

	INT8U i;

	int interface_index = -1;

	for (i = 0; i < interfaces_nr; i++) {
		if(0 == PLATFORM_STRCMP(interfaces_list[i], interface_name)) {
			interface_index = i;
			break;
		}
	}

	PLATFORM_PRINTF_DETAIL("[PLATFORM] Retrieving info for interface %s\n", interface_name);

	m = (struct interfaceInfo *)malloc(sizeof(struct interfaceInfo));
	if (NULL == m) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] Not enough memory for the 'interface' structure\n");
		return NULL;
	}

	// Fill the 'name' field
	//

	if(interface_name == NULL) {
		PLATFORM_PRINTF_ERROR("NULL interface_name!!!!\n");
		PLATFORM_FREE(m);
		return NULL;
	}

	m->name = strdup(interface_name);

	// Obtain the 'mac_address'
	//

	// Give "sane" values in case any of the following parameters can not be
	// filled later
	//
	m->mac_address[0] = 0x00;
	m->mac_address[1] = 0x00;
	m->mac_address[2] = 0x00;
	m->mac_address[3] = 0x00;
	m->mac_address[4] = 0x00;
	m->mac_address[5] = 0x00;

	m->interface_type                                                = INTERFACE_TYPE_UNKNOWN;

	m->is_secured                     = 0;
	m->push_button_on_going           = 2; // "2" means "unsupported"
	m->push_button_new_mac_address[0] = 0x00;
	m->push_button_new_mac_address[1] = 0x00;
	m->push_button_new_mac_address[2] = 0x00;
	m->push_button_new_mac_address[3] = 0x00;
	m->push_button_new_mac_address[4] = 0x00;
	m->push_button_new_mac_address[5] = 0x00;

	m->power_state               = INTERFACE_POWER_STATE_OFF;
	m->neighbor_mac_addresses_nr = INTERFACE_NEIGHBORS_UNKNOWN;
	m->neighbor_list             = NULL;

	// Next, fill all the parameters we can depending on the type of interface
	// we are dealing with:

	if (0 == PLATFORM_GET_MAC(m->name, m->mac_address)) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] Get not get MAC for interface %s\n", m->name);
		PLATFORM_FREE(m->name);
		PLATFORM_FREE(m);
		return NULL;
	}

#ifdef _RTK_LINUX_
	int result;
	result = rtk_linux_get_interface_info(interface_name, m, need_wps);
	if (0 == result) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] Get not get info for interface %s\n", m->name);
		PLATFORM_FREE(m->name);
		PLATFORM_FREE(m);
		return NULL;
	}
	if (-1 != interface_index && NULL != interfaces_list_ssids[interface_index]
		&& IS_IEEE_802_11_INTERFACE(m->interface_type)) {
		if (0 == PLATFORM_STRLEN(m->interface_type_data.ieee80211.ssid)) {
			strcpy(m->interface_type_data.ieee80211.ssid, interfaces_list_ssids[interface_index]);
		} else {
			strcpy(interfaces_list_ssids[interface_index], m->interface_type_data.ieee80211.ssid);
		}
	}
#else
	// TODO: In a flavour-neutral device we don't really know how to fill
	//       many of these things, thus we will use "default" values when
	//       needed. This will obviously not always work!

	PLATFORM_PRINTF_WARNING("[PLATFORM] No platform flavour defined. Using default values when needed.\n");

	switch (_getInterfaceType(interface_name)) {
	case INTF_TYPE_ETHERNET: {
		m->interface_type = INTERFACE_TYPE_IEEE_802_3AB_GIGABIT_ETHERNET;
		break;
	}
	case INTF_TYPE_WIFI: {
		m->interface_type = INTERFACE_TYPE_IEEE_802_11B_2_4_GHZ;
		break;
	}
	default: {
		PLATFORM_PRINTF_ERROR("[PLATFORM] Unknown interface type. Assuming ethernet.\n");
		m->interface_type = INTERFACE_TYPE_IEEE_802_3AB_GIGABIT_ETHERNET;
		break;
	}
	}

	// Check 'is_secured'
	//
	if (IS_IEEE_802_3_INTERFACE(m->interface_type)) {
		m->is_secured = 1;
	} else if (IS_IEEE_802_11_INTERFACE(m->interface_type)
		&& (m->interface_type_data.ieee80211.authentication_mode == IEEE80211_AUTH_MODE_WPA
		|| m->interface_type_data.ieee80211.authentication_mode == IEEE80211_AUTH_MODE_WPAPSK
		|| m->interface_type_data.ieee80211.authentication_mode == IEEE80211_AUTH_MODE_WPA2
		|| m->interface_type_data.ieee80211.authentication_mode == IEEE80211_AUTH_MODE_WPA2PSK)) {
		m->is_secured = 1;
	} else {
		m->is_secured = 0;
	}

	// Check 'push button' configuration sequence status
	//
	m->push_button_on_going = 2; // "2" means "not supported"

	// Check the 'power_state'
	//
	m->power_state = INTERFACE_POWER_STATE_ON;

	// Add neighbor MAC addresses
	//
	m->neighbor_mac_addresses_nr = INTERFACE_NEIGHBORS_UNKNOWN;
	m->neighbor_list             = NULL;
#endif

	PLATFORM_PRINTF_FULL("[PLATFORM]   mac_address                 : %02x:%02x:%02x:%02x:%02x:%02x\n", m->mac_address[0], m->mac_address[1], m->mac_address[2], m->mac_address[3], m->mac_address[4], m->mac_address[5]);
	PLATFORM_PRINTF_FULL("[PLATFORM]   interface_type              : %d\n", m->interface_type);
	if (IS_IEEE_802_11_INTERFACE(m->interface_type)) {
		PLATFORM_PRINTF_FULL("[PLATFORM]     ieee80211 data\n");
		PLATFORM_PRINTF_FULL("[PLATFORM]       bssid                       : %02x:%02x:%02x:%02x:%02x:%02x\n", m->interface_type_data.ieee80211.bssid[0], m->interface_type_data.ieee80211.bssid[1], m->interface_type_data.ieee80211.bssid[2], m->interface_type_data.ieee80211.bssid[3], m->interface_type_data.ieee80211.bssid[4], m->interface_type_data.ieee80211.bssid[5]);
		PLATFORM_PRINTF_FULL("[PLATFORM]       ssid                        : %s\n", m->interface_type_data.ieee80211.ssid);
		PLATFORM_PRINTF_FULL("[PLATFORM]       role                        : %d\n", m->interface_type_data.ieee80211.role);
		PLATFORM_PRINTF_FULL("[PLATFORM]       ap_channel_band             : 0x%02x\n", m->interface_type_data.ieee80211.ap_channel_band);
		PLATFORM_PRINTF_FULL("[PLATFORM]       ap_channel_center_f1        : 0x%02x\n", m->interface_type_data.ieee80211.ap_channel_center_frequency_index_1);
		PLATFORM_PRINTF_FULL("[PLATFORM]       ap_channel_center_f2        : 0x%02x\n", m->interface_type_data.ieee80211.ap_channel_center_frequency_index_2);
		PLATFORM_PRINTF_FULL("[PLATFORM]       authentication_mode         : 0x%04x\n", m->interface_type_data.ieee80211.authentication_mode);
		PLATFORM_PRINTF_FULL("[PLATFORM]       encryption_mode             : 0x%04x\n", m->interface_type_data.ieee80211.encryption_mode);
		// PLATFORM_PRINTF_FULL("[PLATFORM]       network_key                 : %s\n", m->interface_type_data.ieee80211.network_key);
	} else if (IS_IEEE_1901_INTERFACE(m->interface_type)) {
		PLATFORM_PRINTF_FULL("[PLATFORM]     ieee1901 data\n");
		PLATFORM_PRINTF_FULL("[PLATFORM]       network_identifier          : %02x:%02x:%02x:%02x:%02x:%02x:%02x\n", m->interface_type_data.ieee1901.network_identifier[0], m->interface_type_data.ieee1901.network_identifier[1], m->interface_type_data.ieee1901.network_identifier[2], m->interface_type_data.ieee1901.network_identifier[3], m->interface_type_data.ieee1901.network_identifier[4], m->interface_type_data.ieee1901.network_identifier[5], m->interface_type_data.ieee1901.network_identifier[6]);
	} else if (INTERFACE_TYPE_UNKNOWN == m->interface_type) {
		PLATFORM_PRINTF_FULL("[PLATFORM]     unknown interface data\n");
	}
	PLATFORM_PRINTF_FULL("[PLATFORM]   is_secure                   : %d\n", m->is_secured);
	PLATFORM_PRINTF_FULL("[PLATFORM]   push_button_on_going        : %d\n", m->push_button_on_going);
	PLATFORM_PRINTF_FULL("[PLATFORM]   push_button_new_mac_address : %02x:%02x:%02x:%02x:%02x:%02x\n", m->push_button_new_mac_address[0], m->push_button_new_mac_address[1], m->push_button_new_mac_address[2], m->push_button_new_mac_address[3], m->push_button_new_mac_address[4], m->push_button_new_mac_address[5]);
	PLATFORM_PRINTF_FULL("[PLATFORM]   power_state                 : %d\n", m->power_state);
	PLATFORM_PRINTF_FULL("[PLATFORM]   neighbor_mac_addresses_nr   : %d\n", m->neighbor_mac_addresses_nr);
	if (m->neighbor_mac_addresses_nr != INTERFACE_NEIGHBORS_UNKNOWN) {
		for (i = 0; i < m->neighbor_mac_addresses_nr; i++) {
			PLATFORM_PRINTF_FULL("[PLATFORM]     - neighbor #%d                : %02x:%02x:%02x:%02x:%02x:%02x\n", i, m->neighbor_list[i].mac_address[0], m->neighbor_list[i].mac_address[1], m->neighbor_list[i].mac_address[2], m->neighbor_list[i].mac_address[3], m->neighbor_list[i].mac_address[4], m->neighbor_list[i].mac_address[5]);
		}
	}

	return m;
}

void PLATFORM_FREE_1905_INTERFACE_INFO(struct interfaceInfo *x)
{
	PLATFORM_FREE(x->name);

	if (x->neighbor_mac_addresses_nr > 0 && INTERFACE_NEIGHBORS_UNKNOWN != x->neighbor_mac_addresses_nr && NULL != x->neighbor_list) {
		PLATFORM_FREE(x->neighbor_list);
	}

	PLATFORM_FREE(x);
}

struct linkMetrics *PLATFORM_GET_LINK_METRICS(char *local_interface_name, INT8U *neighbor_interface_address, INT8U stp_state)
{
	struct linkMetrics *  ret;
	INT8U local_mac[MACADDRLEN] = {0};

	ret = (struct linkMetrics *)malloc(sizeof(struct linkMetrics));
	if (NULL == ret) {
		return NULL;
	}

	// Obtain the MAC address of the local interface
	//
	if (!PLATFORM_GET_MAC(local_interface_name, local_mac)) {
		PLATFORM_FREE(ret);
		return NULL;
	}
	memcpy(ret->local_interface_address, local_mac, 6);

	// Copy the remote interface MAC address
	//
	memcpy(ret->neighbor_interface_address, neighbor_interface_address, 6);

	// Next, fill all the parameters we can depending on the type of interface
	// we are dealing with:

	// Obtain how much time the process collecting stats has been running
	//
	//     TODO: This should be set to the amount of seconds ellapsed since
	//     the interface was brought up. However I could not find an easy
	//     way to obtain this information in Linux.
	//     For now, we will simply set this to the amount of seconds since
	//     the system was started, which is typically correct on most
	//     cases.
	//
	ret->measures_window = PLATFORM_GET_TIMESTAMP() / 1000;

	// Check interface name
	// TODO: Find a more robust way of identifying a wifi interface. Maybe
	//       checking "/sys"?
	//
	if (strstr(local_interface_name, "wlan") != NULL) {

#ifdef _RTK_LINUX_
		PLATFORM_GET_LINK_METRICS_IOCTL(local_interface_name, ret, neighbor_interface_address);
#else
		// Obtain the amount of (correct and incorrect) packets transmitted
		// to 'neighbor_interface_address' in the last
		// 'ret->measures_window' seconds.
		//
		// This is done by reading the response of the command
		//
		//   "iw dev $INTERFACE station get $NEIGHBOR_MAC | grep "tx packets"" and
		//   "iw dev $INTERFACE station get $NEIGHBOR_MAC | grep "tx failed""
		//
		ret->tx_packet_ok = (INT32U)_readWifiNeighborParameter(local_interface_name, ret->neighbor_interface_address, "\"tx packets\"");
		ret->tx_packet_errors = (INT32U)_readWifiNeighborParameter(local_interface_name, ret->neighbor_interface_address, "\"tx failed\"");

		// Obtain the estimated max MAC xput and PHY rate when transmitting
		// data from "A" to "B".
		//
		// This is done by reading the response of the command
		//
		//   "iw dev $INTERFACE station get $NEIGHBOR_MAC | grep speed"
		//
		ret->tx_max_xput = (INT16U)_readWifiNeighborParameter(local_interface_name, ret->neighbor_interface_address, "\"tx bitrate\"");
		ret->tx_phy_rate = (INT16U)_readWifiNeighborParameter(local_interface_name, ret->neighbor_interface_address, "\"tx bitrate\"");

		// Obtain the estimated average percentage of time that the link is
		// available for transmission.
		//
		//   TODO: I'll just say "100% of the time" for now.
		//
		ret->tx_link_availability = 100;

		// Obtain the amount of (correct and incorrect) packets received
		// from 'neighbor_interface_address' in the last
		// 'ret->measures_window' seconds.
		//
		// This is done by reading the response of the command
		//
		//   "iw dev $INTERFACE station get $NEIGHBOR_MAC | grep "rx packets""
		//
		//   TODO: rx errors can't be obtained from this console command.
		//   Right now it's assigned a zero value. Investigate how to
		//   obtain this value.
		//

		ret->rx_packet_ok = (INT32U)_readWifiNeighborParameter(local_interface_name, ret->neighbor_interface_address, "\"rx packets\"");
		ret->rx_packet_errors = 0;

		// Obtain the estimated RX RSSI
		//
		// RSSI is a term used to measure the relative quality of a
		// received signal to a client device, but has no absolute value.
		// The IEEE 802.11 standard specifies that RSSI can be on a scale
		// of 0 to up to 255 and that each chipset manufacturer can define
		// their own "RSSI_Max" value. It's all up to the manufacturer
		// (which is why RSSI is a relative index), but you can infer that
		// the higher the RSSI value is, the better the signal is.
		//
		// A basic (saturated linear) conversion formula has been
		// implemented:
		//   RSSI range 0-100 <--> signal -40 to -70 dBm
		//
		// Feel free to redefine this conversion formula. Maybe to a
		// logarithmical one.
		//
		INT32S tmp = _readWifiNeighborParameter(local_interface_name, ret->neighbor_interface_address, "\"signal:\"");

#	define SIGNAL_MAX (-40) // dBm
#	define SIGNAL_MIN (-70)
		if (tmp >= SIGNAL_MAX) {
			ret->rx_rssi = 100;
		} else if (tmp <= SIGNAL_MIN) {
			ret->rx_rssi = 0;
		} else {
			ret->rx_rssi = (tmp - SIGNAL_MIN) * 100 / (SIGNAL_MAX - SIGNAL_MIN);
		}
#endif
	}
	// Other interface types, probably ethernet
	else {
		// Obtain the amount of (correct and incorrect) packets transmitted
		// to 'neighbor_interface_address' in the last
		// 'ret->measures_window' seconds.
		//
		//   TODO: In Linux there is no easy way to obtain this
		//   information. We will just report the amount of *total* packets
		//   transmitted from our local interface, no matter the
		//   destination.
		//   This information will only be correct when the local interface
		//   is connected to one single remote interface... however we
		//   better report this than nothing at all.
		//
		// This is done by reading the contents of files
		//
		//   "/sys/class/net/<interface_name>/statistics/tx_packets" and
		//   "/sys/class/net/<interface_name>/statistics/tx_errors"
		//
		ret->tx_packet_ok     = (INT32U)_readInterfaceParameter(local_interface_name, "statistics/tx_packets");
		ret->tx_packet_errors = (INT32U)_readInterfaceParameter(local_interface_name, "statistics/tx_errors");

		// Obtain the estimatid max MAC xput and PHY rate when transmitting
		// data from "A" to "B".
		//
		//   TODO: The same considerations as in the previous parameters
		//   apply here.
		//
		// This is done by reading the contents of file
		//
		//   "/sys/class/net/<interface_name>/speed
		//
		// NOTE: I'll set both parameters to the same value. Is there a
		// better way to do this?
		//
		ret->tx_max_xput = (INT16U)_readInterfaceParameter(local_interface_name, "speed");
		ret->tx_phy_rate = (INT16U)_readInterfaceParameter(local_interface_name, "speed");

		// Obtain the estimated average percentage of time that the link is
		// available for transmission.
		//
		//   TODO: I'll just say "100% of the time" for now.
		//
		ret->tx_link_availability = 100;

		// Obtain the amount of (correct and incorrect) packets received from
		// 'neighbor_interface_address' in the last 'ret->measures_window'
		// seconds.
		//
		//   TODO: In Linux there is no easy way to obtain this information. We
		//   will just report the amount of *total* packets received on our
		//   local interface, no matter the origin.
		//   This information will only be correct when the local interface is
		//   connected to one single remote interface... however we better
		//   report this than nothing at all.
		//
		// This is done by reading the contents of files
		//
		//   "/sys/class/net/<interface_name>/statistics/rx_packets" and
		//   "/sys/class/net/<interface_name>/statistics/rx_errors"
		//
		ret->rx_packet_ok     = (INT32U)_readInterfaceParameter(local_interface_name, "statistics/rx_packets");
		ret->rx_packet_errors = (INT32U)_readInterfaceParameter(local_interface_name, "statistics/rx_errors");

		// Obtain the estimated RX RSSI
		//
		//   TODO: ???
		//ret->rx_rssi = 0;
		if (stp_state) {
			ret->rx_rssi = 100;
		} else {
			ret->rx_rssi = 0xff;
		}
	}

	return ret;
}

void PLATFORM_FREE_LINK_METRICS(struct linkMetrics *l)
{
	// This is a simple structure which does not require any special treatment.
	//
	if (l) {
		PLATFORM_FREE(l);
	}

	return;
}

void PLATFORM_GET_LINK_METRICS_IOCTL(char *local_interface_name, struct linkMetrics *link_metric, INT8U *neighbor_interface_address)
{

	if(0 < rtk_linux_get_link_metric_ioctl(local_interface_name, neighbor_interface_address, link_metric)) {
		PLATFORM_PRINTF_DETAIL("[RTL] [%s] Local MAC(%02x:%02x:%02x:%02x:%02x:%02x)\tNB MAC(%02x:%02x:%02x:%02x:%02x:%02x)\tWindow:%u\n",
			local_interface_name,
			link_metric->local_interface_address[0], link_metric->local_interface_address[1], link_metric->local_interface_address[2], link_metric->local_interface_address[3], link_metric->local_interface_address[4], link_metric->local_interface_address[5],
			link_metric->neighbor_interface_address[0], link_metric->neighbor_interface_address[1], link_metric->neighbor_interface_address[2], link_metric->neighbor_interface_address[3], link_metric->neighbor_interface_address[4], link_metric->neighbor_interface_address[5],
			link_metric->measures_window);
		PLATFORM_PRINTF_DETAIL("tx_pkt: %u tx_error:%u tx_max_xput:%u tx_phy_rate:%u tx_link_avail:%u\n",
			link_metric->tx_packet_ok, link_metric->tx_packet_errors,
			link_metric->tx_max_xput, link_metric->tx_phy_rate, link_metric->tx_link_availability);
		PLATFORM_PRINTF_DETAIL("rx_pkt: %u rx_error:%u rx_rssi:%u\t\n",
			link_metric->rx_packet_ok, link_metric->rx_packet_errors, link_metric->rx_rssi);
	} else {
		PLATFORM_PRINTF_WARNING("[RTL] [%s] Can't find Local(%02x:%02x:%02x:%02x:%02x:%02x) with NB (%02x:%02x:%02x:%02x:%02x:%02x)\n",
			local_interface_name,
			link_metric->local_interface_address[0], link_metric->local_interface_address[1], link_metric->local_interface_address[2], link_metric->local_interface_address[3], link_metric->local_interface_address[4], link_metric->local_interface_address[5],
			link_metric->neighbor_interface_address[0], link_metric->neighbor_interface_address[1], link_metric->neighbor_interface_address[2], link_metric->neighbor_interface_address[3], link_metric->neighbor_interface_address[4], link_metric->neighbor_interface_address[5]);
		link_metric->tx_packet_ok         = 0;
		link_metric->tx_packet_errors     = 0;
		link_metric->tx_max_xput          = 0;
		link_metric->tx_phy_rate          = 0;
		link_metric->tx_link_availability = 0;
		link_metric->rx_packet_ok         = 0;
		link_metric->rx_packet_errors     = 0;
		link_metric->rx_rssi              = 0;
	}
	return;
}

struct bridge *PLATFORM_GET_LIST_OF_BRIDGES(INT8U *nr)
{
	// TODO

	*nr = 0;
	return NULL;
}

void PLATFORM_FREE_LIST_OF_BRIDGES(struct bridge *x, INT8U nr)
{
	// TODO
	//
	if (0 == nr || NULL == x) {
		return;
	}

	return;
}

INT8U PLATFORM_SEND_RAW_PACKET(char *interface_name, INT8U *dst_mac, INT8U *src_mac, INT16U eth_type, INT8U *payload, INT16U payload_len)
{
	int  i, first_time;
	char aux1[200];
	char aux2[10];

	int                s;
	struct ifreq       ifr;
	int                ifindex;
	struct sockaddr_ll socket_address;

	INT8U                buffer[MAX_ETH_FRAME_SEGMENT_SIZE];
	struct ether_header *eh;

	// Print packet (used for debug purposes)
	//
	PLATFORM_PRINTF_DETAIL("[PLATFORM] Preparing to send RAW packet:\n");
	PLATFORM_PRINTF_DETAIL("[PLATFORM]   - Interface name = %s\n", interface_name);
	PLATFORM_PRINTF_DETAIL("[PLATFORM]   - DST  MAC       = 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\n", dst_mac[0], dst_mac[1], dst_mac[2], dst_mac[3], dst_mac[4], dst_mac[5]);
	PLATFORM_PRINTF_DETAIL("[PLATFORM]   - SRC  MAC       = 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x\n", src_mac[0], src_mac[1], src_mac[2], src_mac[3], src_mac[4], src_mac[5]);
	PLATFORM_PRINTF_DETAIL("[PLATFORM]   - Ether type     = 0x%04x\n", eth_type);
	PLATFORM_PRINTF_DETAIL("[PLATFORM]   - Payload length = %d\n", payload_len);

	aux1[0]    = 0x0;
	aux2[0]    = 0x0;
	first_time = 1;
	for (i = 0; i < payload_len; i++) {
		snprintf(aux2, 6, "0x%02x ", payload[i]);
		strncat(aux1, aux2, 200 - strlen(aux1) - 1);

		if (0 != i && 0 == (i + 1) % 8) {
			if (1 == first_time) {
				PLATFORM_PRINTF_DETAIL("[PLATFORM]   - Payload        = %s\n", aux1);
				first_time = 0;
			} else {
				PLATFORM_PRINTF_DETAIL("[PLATFORM]                      %s\n", aux1);
			}
			aux1[0] = 0x0;
		}
	}
	if (1 == first_time) {
		PLATFORM_PRINTF_DETAIL("[PLATFORM]   - Payload        = %s\n", aux1);
	} else {
		PLATFORM_PRINTF_DETAIL("[PLATFORM]                      %s\n", aux1);
	}

	// Open RAW socket
	//
	PLATFORM_PRINTF_DETAIL("[PLATFORM] Opening RAW socket\n");
	s = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (-1 == s) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] socket('%s') returned with errno=%d (%s) while opening a RAW socket\n", interface_name, errno, strerror(errno));
		return 0;
	}

	// Retrieve ethernet interface index
	//
	PLATFORM_PRINTF_DETAIL("[PLATFORM] Retrieving interface index\n");
	strncpy(ifr.ifr_name, interface_name, IFNAMSIZ - 1);
	if (ioctl(s, SIOCGIFINDEX, &ifr) == -1) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] ioctl('%s',SIOCGIFINDEX) returned with errno=%d (%s) while opening a RAW socket\n", interface_name, errno, strerror(errno));
		close(s);
		return 0;
	}
	ifindex = ifr.ifr_ifindex;
	PLATFORM_PRINTF_DETAIL("[PLATFORM] Successfully got interface index %d\n", ifindex);

	// Empy buffer
	//
	memset(buffer, 0, MAX_ETH_FRAME_SEGMENT_SIZE);

	// Fill ethernet header
	//
	eh                 = (struct ether_header *)buffer;
	eh->ether_dhost[0] = dst_mac[0];
	eh->ether_dhost[1] = dst_mac[1];
	eh->ether_dhost[2] = dst_mac[2];
	eh->ether_dhost[3] = dst_mac[3];
	eh->ether_dhost[4] = dst_mac[4];
	eh->ether_dhost[5] = dst_mac[5];
	eh->ether_shost[0] = src_mac[0];
	eh->ether_shost[1] = src_mac[1];
	eh->ether_shost[2] = src_mac[2];
	eh->ether_shost[3] = src_mac[3];
	eh->ether_shost[4] = src_mac[4];
	eh->ether_shost[5] = src_mac[5];
	eh->ether_type     = htons(eth_type);

	// Fill buffer
	//
	memcpy(buffer + sizeof(*eh), payload, payload_len);

	// Prepare sockaddr_ll
	//
	memset(&socket_address, 0, sizeof(socket_address));
	socket_address.sll_ifindex = ifindex;
	socket_address.sll_halen   = ETH_ALEN;
	socket_address.sll_addr[0] = dst_mac[0];
	socket_address.sll_addr[1] = dst_mac[1];
	socket_address.sll_addr[2] = dst_mac[2];
	socket_address.sll_addr[3] = dst_mac[3];
	socket_address.sll_addr[4] = dst_mac[4];
	socket_address.sll_addr[5] = dst_mac[5];
	socket_address.sll_addr[6] = 0x00;
	socket_address.sll_addr[7] = 0x00;

	PLATFORM_PRINTF_DETAIL("[PLATFORM] Sending data to RAW socket\n");
	if (-1 == sendto(s, buffer,
	                 sizeof(*eh) + payload_len >= 60 ? sizeof(*eh) + payload_len : 60, // 60 is the minimum ethernet frame length
	                 0, (struct sockaddr *)&socket_address, sizeof(socket_address))) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] sendto('%s') returned with errno=%d (%s)\n", interface_name, errno, strerror(errno));
		close(s);
		return 0;
	}
	PLATFORM_PRINTF_DETAIL("[PLATFORM] Data sent!\n");

	close(s);
	return 1;
}

INT8U PLATFORM_START_PUSH_BUTTON_CONFIGURATION(char *interface_name, INT8U queue_id, INT8U *al_mac_address, INT16U mid)
{
	pthread_t                     thread;
	struct _pushButtonThreadData *p;
	struct interfaceInfo *        x;

	if (NULL == PLATFORM_STRSTR(interface_name, "wlan")) {
		PLATFORM_PRINTF_ERROR("Push button thread doesnot accept %s!\n", interface_name);
		return 0;
	}

	// Make sure the interface:
	//   - is not already in the middle of a "push button" configuration
	//     process and...
	//   - ...it has support for the "push button" configuration mechanism.
	//
	x = PLATFORM_GET_1905_INTERFACE_INFO_WITH_WPS(interface_name, 1);
	if (NULL == x) {
		return 0;
	}
	if (0 != x->push_button_on_going) {
		PLATFORM_PRINTF_WARNING("Push button is already started, skip create thread for %s!\n", interface_name);
		PLATFORM_FREE_1905_INTERFACE_INFO(x);
		return 1;
	}
	PLATFORM_FREE_1905_INTERFACE_INFO(x);

	p = (struct _pushButtonThreadData *)malloc(sizeof(struct _pushButtonThreadData));
	if (NULL == p) {
		// Out of memory
		//
		return 0;
	}

	p->queue_id          = queue_id;
	p->interface_name    = strdup(interface_name);
	p->al_mac_address[0] = al_mac_address[0];
	p->al_mac_address[1] = al_mac_address[1];
	p->al_mac_address[2] = al_mac_address[2];
	p->al_mac_address[3] = al_mac_address[3];
	p->al_mac_address[4] = al_mac_address[4];
	p->al_mac_address[5] = al_mac_address[5];
	p->mid               = mid;

	pthread_attr_t attr;
	pthread_attr_init(&attr);

	pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);

	pthread_create(&thread, &attr, _pushButtonConfigurationThread, (void *)p);
	pthread_detach(thread);

	return 1;
}

INT8U PLATFORM_SET_INTERFACE_POWER_MODE(char *interface_name, INT8U power_mode)
{
	// TODO
	switch (power_mode) {
	case INTERFACE_POWER_STATE_ON: {
		PLATFORM_PRINTF_DETAIL("[PLATFORM] %s --> POWER ON\n", interface_name);
		break;
	}
	case INTERFACE_POWER_STATE_OFF: {
		PLATFORM_PRINTF_DETAIL("[PLATFORM] %s --> POWER OFF\n", interface_name);
		break;
	}
	case INTERFACE_POWER_STATE_SAVE: {
		PLATFORM_PRINTF_DETAIL("[PLATFORM] %s --> POWER SAVE\n", interface_name);
		break;
	}
	default: {
		PLATFORM_PRINTF_WARNING("[PLATFORM] Unknown power mode for interface %s (%d)\n", interface_name, power_mode);
		return 0;
	}
	}

	return INTERFACE_POWER_RESULT_EXPECTED;
}

// INT8U PLATFORM_CONFIGURE_80211_AP(char *interface_name, char *ssid, INT8U *bssid, INT16U auth_type, INT16U encryption_type, char *network_key, INT8U network_type)
// {
// 	if (interface_name == NULL) {
// 		PLATFORM_PRINTF_ERROR("No interface matched for M2.\n");
// 		return 0;
// 	}
// 	PLATFORM_PRINTF_INFO("Applying WSC configuration (%s): \n", interface_name);
// 	PLATFORM_PRINTF_INFO("  - SSID            : %s\n", ssid);
// 	if (bssid != NULL) {
// 		PLATFORM_PRINTF_INFO("  - BSSID           : %02x:%02x:%02x:%02x:%02x:%02x\n", bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
// 	}
// 	PLATFORM_PRINTF_INFO("  - AUTH_TYPE       : 0x%04x\n", auth_type);
// 	PLATFORM_PRINTF_INFO("  - ENCRYPTION_TYPE : 0x%04x\n", encryption_type);
// 	PLATFORM_PRINTF_INFO("  - NETWORK_KEY     : %s\n", network_key);
// 	PLATFORM_PRINTF_INFO("  - BSS_TYPE        : %02x\n", network_type);

// #ifdef _RTK_LINUX_
// 	rtk_linux_apply_80211_configuration(interface_name, ssid, auth_type, encryption_type, network_key, network_type);
// 	rtk_linux_bss_ioctl(interface_name, network_type);
// #else
// 	PLATFORM_PRINTF_WARNING("[PLATFORM] Configuration has no effect on flavour-neutral platform\n");
// #endif

// 	int i = 0;
// 	for (i = 0; i < interfaces_nr; i++) {
// 		if (0 == PLATFORM_STRCMP(interfaces_list[i], interface_name)) {
// 			break;
// 		}
// 	}
// 	if (i >= interfaces_nr) {
// 		PLATFORM_PRINTF_INFO("Add new interface %s\n", interface_name);
// 		addInterface(interface_name);
// 	}

// 	return 1;
// }

INT8U PLATFORM_BSS_IOCTL(char *interface_name, INT8U type)
{
#ifdef _RTK_LINUX_
	return rtk_linux_bss_ioctl(interface_name, type);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_BSS_IOCTL not implemented!\n");
	return 0;
#endif
}

INT8U PLATFORM_AP_CAPABILITY_IOCTL(char *ifname, INT8U tlv_type, INT8U *results)
{
#ifdef _RTK_LINUX_
	return rtk_linux_get_ap_capability_info(ifname, tlv_type, results);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_AP_CAPABILITY_IOCTL not implemented!\n");
	return 0;
#endif
}

/* For future ioctl functions for map r3 */
/* results should be in the following format:
 * FIELD:					LENGTH:					VALUE:
 * operation_type			1 octet  				0
 * total_length  			2 octet  				0
 * TLV_TYPE      			1 octet  				TLV_TYPE
 * TLV_LENGTH				2 octet  				k for set, 0 for get
 * TLV_PAYLOAD   			k/0 octet				TLV_PAYLOAD
 *
 */
INT32S PLATFORM_MAP_GET_GENERAL_IOCTL(char *ifname, INT8U *results, INT16U result_len)
{
#ifdef _RTK_LINUX_
	INT8U operation_type_check = *results;
	INT16U tlv_total_length_check = *((INT16U *)(results + 1));
	if (operation_type_check || tlv_total_length_check) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] SIOCMAP_GET_GENERAL result buffer first three bytes not empty !\n");
		return 0;
	}

	INT8U tlv_type = *(results + 3);
	INT16U *tlv_length = (INT16U *)(results + 4);
	char *tlv_type_string = convert_1905_TLV_type_to_string(tlv_type);
	PLATFORM_PRINTF_INFO("[PLATFORM] Preparing to get info : %s\n", tlv_type_string);
	PLATFORM_PRINTF_INFO("[PLATFORM]   - Interface name = %s\n", ifname);
	PLATFORM_PRINTF_INFO("[PLATFORM]   - Receiving buffer size : %u\n", result_len);

	*tlv_length = 0;

	return rtk_linux_map_general(ifname, 1, results, result_len, *tlv_length + 3);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] SIOCMAP_GET_GENERAL not implemented!\n");
	return 0;
#endif
}

#ifdef RTK_MULTI_AP_R3
void PLATFORM_MAP_ISSUE_CCE_IE_INDICATION(char *ifname, INT8U include_cce_ie)
{
#ifdef _RTK_LINUX_
	rtk_linux_issue_cce_ie_indication(ifname, include_cce_ie);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] ISSUE_CCE_IE_INDICATION not implemented!\n");
#endif
}
#endif // RTK_MULTI_AP_R3

INT32S PLATFORM_MAP_SET_GENERAL_IOCTL(char *ifname, INT8U *results, INT16U result_len)
{
#ifdef _RTK_LINUX_
	INT8U operation_type_check = *results;
	INT16U tlv_total_length_check = *((INT16U *)(results + 1));
	if (operation_type_check || tlv_total_length_check) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] SIOCMAP_GET_GENERAL result buffer first three bytes not empty !\n");
		return 0;
	}

	INT8U tlv_type = *(results + 3);
	INT8U *tlv_length_ptr = results + 4;
	INT16U tlv_length = 0;
	_E2B(&tlv_length_ptr, &tlv_length); // results contains tlv stream forged, so _E2B should be used.
	char *tlv_type_string = convert_1905_TLV_type_to_string(tlv_type);
	PLATFORM_PRINTF_INFO("[PLATFORM] Preparing to set info : %s\n", tlv_type_string);
	PLATFORM_PRINTF_INFO("[PLATFORM]   - Interface name = %s\n", ifname);
	PLATFORM_PRINTF_INFO("[PLATFORM]   - Receiving buffer size : %u\n", result_len);

	return rtk_linux_map_general(ifname, 2, results, result_len, tlv_length + 3);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] SIOCMAP_GET_GENERAL not implemented!\n");
	return 0;
#endif
}

INT8U PLATFORM_CLIENT_CAPABILITY_IOCTL(char *ifname, INT8U *macaddr, INT8U *results)
{
#ifdef _RTK_LINUX_
	return rtk_linux_get_client_capability_info(ifname, macaddr, results);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_CLIENT_CAPABILITY_IOCTL not implemented!\n");
	return 0;
#endif
}

INT8U PLATFORM_AP_METRIC_IOCTL(char *ifname, INT8U tlv_type, INT8U *results)
{
#ifdef _RTK_LINUX_
	if (!eth_only)
		return rtk_linux_get_ap_metric(ifname, tlv_type, results);
	else
		return 0;
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_AP_METRIC_IOCTL not implemented!\n");
	return 0;
#endif
}

INT8U PLATFORM_ASSOC_STA_IOCTL(char *ifname, INT8U tlv_type, INT8U *macaddr, INT8U *results)
{
#ifdef _RTK_LINUX_
	return rtk_linux_get_assocSta_metric(ifname, tlv_type, macaddr, results);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_ASSOC_STA_IOCTL not implemented!\n");
	return 0;
#endif
}

INT8U PLATFORM_UNASSOC_STA_IOCTL(char *ifname, INT8U op_class, INT8U channel_nr, INT8U sta_nr, INT8U (*macaddr)[6])
{
#ifdef _RTK_LINUX_
	return rtk_linux_record_unAssocSta_metric(ifname, op_class, channel_nr, sta_nr, macaddr);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_UNASSOC_STA_IOCTL not implemented!\n");
	return 0;
#endif
}

INT8U PLATFORM_METRIC_POLICY_IOCTL(char *interface_name, INT8U rcpi_threshold, INT8U rcpi_hysteris_margin, INT8U channel_utilization_threshold)
{
#ifdef _RTK_LINUX_
	return rtk_linux_metric_policy_ioctl(interface_name, rcpi_threshold, rcpi_hysteris_margin, channel_utilization_threshold);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_METRIC_POLICY_IOCTL not implemented!\n");
	return 0;
#endif
}

INT8U PLATFORM_STEERING_POLICY_IOCTL(char *interface_name, INT8U policy, INT8U channel_utilization_threshold, INT8U rcpi_threshold)
{
#ifdef _RTK_LINUX_
	if (!eth_only)
		return rtk_linux_steering_policy_ioctl(interface_name, policy, channel_utilization_threshold, rcpi_threshold);
	else
		return 0;
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_STEERING_POLICY_IOCTL not implemented!\n");
	return 0;
#endif
}

INT32U PLATFORM_GET_PREFER_BSSID_INTERVAL()
{
#ifdef _RTK_LINUX_
	return rtk_get_prefer_bssid_interval();
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_GET_PREFER_BSSID_INTERVAL not implemented!\n");
	return 0;
#endif
}

void PLATFORM_RESET_PREFER_BSSID()
{
#ifdef _RTK_LINUX_
	rtk_reset_prefer_bssid();
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_RESET_PREFER_BSSID not implemented!\n");
#endif
	return;
}

INT8U PLATFORM_BACKHAUL_STEER_IOCTL(char *interface_name, INT8U *backhaul_bss, INT8U *target_bssid, INT8U op_class, INT8U channel)
{
#ifdef _RTK_LINUX_
	return rtk_linux_do_backhaul_steering(interface_name, backhaul_bss, target_bssid, op_class, channel);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_BACKHAUL_STEER_IOCTL not implemented!\n");
	return 0;
#endif
}

INT8U PLATFORM_SET_TX_MAX_POWER_IOCTL(char *interface_name, INT8U tx_max_power)
{
#ifdef _RTK_LINUX_
	return rtk_linux_set_tx_max_power_ioctl(interface_name, tx_max_power);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_SET_TX_MAX_POWER_IOCTL not implemented!\n");
	return 0;
#endif
}

INT8U PLATFORM_CHANNEL_SCAN_IOCTL(char *interface_name, INT8U channel_nr, INT8U *channels)
{
	if (!PLATFORM_IS_INTERFACE_UP_AND_ON(interface_name))
		return 0;
	PLATFORM_PRINTF_DEBUG("Start channel scan on %s\n", interface_name);
#ifdef _RTK_LINUX_
	return rtk_linux_do_channel_scan(interface_name, channel_nr, channels);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_CHANNEL_SCAN_IOCTL not implemented!\n");
	return 0;
#endif
}

INT8U PLATFORM_CAC_IOCTL(char *interface_name, INT8U channel_nr, INT8U *channels, INT8U type)
{
#ifdef _RTK_LINUX_
	return rtk_linux_do_cac(interface_name, channel_nr, channels, type);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_CAC_IOCTL not implemented!\n");
	return 0;
#endif
}

INT8U PLATFORM_SET_MIB(char *interface_name, char *item_str, char *value_str)

{
#ifdef _RTK_LINUX_
	return rtk_linux_set_mib(interface_name, item_str, value_str);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_SET_MIB is not implemented!\n");
	return 0;
#endif
}

INT8U PLATFORM_GET_MIB(char *interface_name, char *mibname, void *result, int size)
{
	INT8U ret = 0;
	char * base_interface_name = trimInterfaceNameVID(interface_name, NULL, NULL);
#ifdef _RTK_LINUX_
	if (eth_only)
		ret =  rtk_linux_get_mib_from_radio_data(base_interface_name, mibname, result, size);
	else
		ret =  rtk_linux_get_mib(base_interface_name, mibname, result, size);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_GET_MIB is not implemented!\n");
	ret = 0;
#endif
	PLATFORM_FREE(base_interface_name);

	return ret;
}

INT8U PLATFORM_GET_MAC(char *name, INT8U *mac_address)
{
	int          fd;
	struct ifreq s;

	if (eth_only && PLATFORM_STRSTR(name, "wlan") != NULL) {
		INT8U mac_24g[6] = {0x00, 0x00, 0x00, 0x00, 0x02, 0x00};
		INT8U mac_5g[6] = {0x00, 0x00, 0x00, 0x00, 0x05, 0x00};
		// only set root interface
		if (0 == PLATFORM_STRCMP(name, map_radio_name_24g))
			memcpy(mac_address, mac_24g, 6);
		else if (0 == PLATFORM_STRCMP(name, map_radio_name_5gl) || 0 == PLATFORM_STRCMP(name, map_radio_name_5gh))
			memcpy(mac_address, mac_5g, 6);
		else {
			PLATFORM_PRINTF_ERROR("[PLATFORM] Get not get MAC for interface %s\n", name);
			return 0;
		}

		return 1;
	} else {
		strlcpy(s.ifr_name, name, sizeof(s.ifr_name));
		fd = socket(PF_INET, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_IP);
		if (-1 == fd) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] Could not open socket when obtaining MAC address of interface %s\n", name);
			return 0;
		}

		if (0 != ioctl(fd, SIOCGIFHWADDR, &s)) {
			PLATFORM_PRINTF_ERROR("[PLATFORM] Could not obtain MAC address of interface %s\n", name);
			close(fd);
			return 0;
		}
		close(fd);
		memcpy(mac_address, s.ifr_addr.sa_data, 6);
		return 1;
	}
}

INT8U PLATFORM_BTM_REQ(char *ifname, INT8U *macaddr, INT8U mode, INT8U *target_bssid, INT8U target_opclass, INT8U target_channel, INT16U disassoc_timer, INT8U reason_code)
{
#ifdef _RTK_LINUX_
	return rtk_linux_send_btm_request(ifname, macaddr, mode, target_bssid, target_opclass, target_channel, disassoc_timer, reason_code);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_BTM_REQ is not implemented!\n");
	return 0;
#endif
}

INT8U PLATFORM_GET_BSS_TYPE(char *interface_name)
{
	if(DMisInterfaceExcluded(interface_name))
		return MULTI_AP_FRONTHAUL_BSS;

#ifdef _RTK_LINUX_
	return rtk_linux_get_bss_type(interface_name);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_GET_BSS_TYPE is not implemented!\n");
	return 0;
#endif
}

INT8U PLATFORM_ASSOC_CONTROL(char *interfacename, INT8U *sta_mac, INT16U timer, INT8U control)
{
#ifdef _RTK_LINUX_
	return rtk_linux_assoc_control(interfacename, sta_mac, timer, control);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_ASSOC_CONTROL is not implemented!\n");
	return 0;
#endif
}

INT8U PLATFORM_SEND_DISASSOC(char *interfacename, INT8U *sta_mac)
{
	PLATFORM_PRINTF_DEBUG("Disassoc %02x:%02x:%02x:%02x:%02x:%02x from %s\n", sta_mac[0], sta_mac[1], sta_mac[2], sta_mac[3], sta_mac[4], sta_mac[5], interfacename);
#ifdef _RTK_LINUX_
	return rtk_linux_send_disassoc(interfacename, sta_mac);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_SEND_DISASSOC is not implemented!\n");
	return 0;
#endif
}

INT8U PLATFORM_DELETE_STA(char *interfacename, INT8U *sta_mac)
{
	PLATFORM_PRINTF_DEBUG("Remove STA %02x:%02x:%02x:%02x:%02x:%02x from %s\n", sta_mac[0], sta_mac[1], sta_mac[2], sta_mac[3], sta_mac[4], sta_mac[5], interfacename);
#ifdef _RTK_LINUX_
	return rtk_linux_del_sta(interfacename, sta_mac);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_DELETE_STA is not implemented!\n");
	return 0;
#endif
}

// get operating class for the radio 2.4G and 5G
OP_CLASS *PLATFORM_GET_OP_CLASS(int *op_classes_len)
{
#if defined(_RTK_LINUX_)
	return rtk_linux_get_op_class(op_classes_len);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_GET_OP_CLASS is not implemented!\n");
	return NULL;
#endif
}

INT8U PLATFORM_CONVERT_TO_GLOBAL_OP_CLASS(INT8U op_class)
{
#if defined(_RTK_LINUX_)
	INT8U new_op_class = rtk_linux_convert_to_global_op_class(op_class);
	PLATFORM_PRINTF_TRACE("Convert op_class from %d to %d\n", op_class, new_op_class);
	return new_op_class;
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_CONVERT_TO_GLOBAL_OP_CLASS is not implemented!\n");
	return 0;
#endif
}

INT8U PLATFORM_CONVERT_TO_LOCAL_OP_CLASS(INT8U op_class)
{
#if defined(_RTK_LINUX_)
	INT8U new_op_class = rtk_linux_convert_to_local_op_class(op_class);
	PLATFORM_PRINTF_TRACE("Convert op_class from %d to %d\n", op_class, new_op_class);
	return new_op_class;
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_CONVERT_TO_LOCAL_OP_CLASS is not implemented!\n");
	return 0;
#endif
}

INT8U PLATFORM_GET_AVAILABLE_CHANNELS(char *interfacename, INT8U *buffer, INT16U buffer_size, INT8U filter)
{
#if defined(_RTK_LINUX_)
	INT8U  filtered_channel_nr = 0, available_channel_nr = 0;
	INT8U *filter_channels = NULL;
	int    i            = 0;

	available_channel_nr = rtk_linux_get_available_channels(interfacename, buffer, buffer_size);
	if (!filter) {
		return available_channel_nr;
	}

	for (i = 0; i < available_channel_nr; i++) {
		if ((buffer[i] > 14 && buffer[i] < 100 && FILTER_5GL == filter) || (buffer[i] >= 100 && FILTER_5GH == filter)) {
			filtered_channel_nr += 1;
			filter_channels                           = PLATFORM_REALLOC(filter_channels, sizeof(INT8U) * filtered_channel_nr);
			filter_channels[filtered_channel_nr - 1] = buffer[i];
		}
	}
	PLATFORM_MEMCPY(buffer, filter_channels, filtered_channel_nr);
	PLATFORM_FREE(filter_channels);

	return filtered_channel_nr;

#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_GET_AVAILABLE_CHANNELS is not implemented!\n");
	return 0;
#endif
}

//get regdomain
char *PLATFORM_GET_REGDOMAIN()
{
	INT32U domain = DOMAIN_GLOBAL;
	char * country = (char *)malloc(2 * sizeof(char));
	char tmp[3] = "";

	if (PLATFORM_IS_INTERFACE_UP(map_radio_name_5gh)) {
		PLATFORM_GET_MIB(map_radio_name_5gh, "regdomain", &domain, 4);
	} else if (PLATFORM_IS_INTERFACE_UP(map_radio_name_5gl)) {
		PLATFORM_GET_MIB(map_radio_name_5gl, "regdomain", &domain, 4);
	} else {
		PLATFORM_GET_MIB(map_radio_name_24g, "regdomain", &domain, 4);
	}

	if (DOMAIN_GLOBAL == domain || DOMAIN_WORLD_WIDE == domain) {
		strcpy(tmp,"TW");
	} else if (DOMAIN_FCC == domain) {
		strcpy(tmp,"US");
	} else if (DOMAIN_ETSI == domain) {
		strcpy(tmp,"FR");
	} else if (DOMAIN_MKK == domain) {
		strcpy(tmp,"JP");
	} else if (DOMAIN_CN == domain) {
		strcpy(tmp,"CN");
	}
	memcpy(country, tmp, 2);

	return country;
}

INT8U PLATFORM_ISSUE_BEACON_MEASUREMENT(char *ifname, unsigned char *macaddr,
                                        struct BeaconMeasurementRequest *beacon_req)
{
#if defined(_RTK_LINUX_)
	return rtk_linux_issue_beacon_measurement(ifname, macaddr, beacon_req);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_ISSUE_BEACON_MEASUREMENT is not implemented!\n");
	return 0;
#endif
}

INT8U PLATFORM_IS_INTERFACE_VALID(char *interface)
{
	int          skfd = 0;
	struct ifreq ifr;

	if (eth_only && PLATFORM_STRSTR(interface, "wlan")) {
		if (0 == PLATFORM_STRCMP(interface, map_radio_name_5gh)
			|| 0 == PLATFORM_STRCMP(interface, map_radio_name_5gl)
			|| 0 == PLATFORM_STRCMP(interface, map_radio_name_24g)) {
			// For wlan, only root interface is valid
			return 1;
		} else
			return 0;
	} else {

		skfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
		if (skfd < 0) {
			return 0;
		}

		strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);
		if (ioctl(skfd, SIOCGIFINDEX, &ifr) == -1) {
			close(skfd);
			return 0;
		}

		close(skfd);
		return 1;
	}
}

INT8U PLATFORM_IS_INTERFACE_UP(char *interface)
{
	int          skfd = 0;
	struct ifreq ifr;
	int          i, j;
	char        root_name[32] = {0};
	char        interface_name[64]= {0};
	INT8U       bss_index, is_up = 0;
	int         radio_idx = -1, if_idx = -1;

	if (eth_only && PLATFORM_STRSTR(interface, "wlan")) {
		// get bss index from interface name
		for (i = 0; i < radio_data_nr; i++) { // radio band
			if (if_idx != -1)
				break;
			if (radio_data[i].radio_band == RADIO_BAND_5GH)
				sprintf(root_name, "%s", map_radio_name_5gh);
			else if (radio_data[i].radio_band == RADIO_BAND_5GL)
				sprintf(root_name, "%s", map_radio_name_5gl);
			else if (radio_data[i].radio_band == RADIO_BAND_2G)
				sprintf(root_name, "%s", map_radio_name_24g);

			for (j = 0; j < radio_data[i].interface_nr; j++) { // bss
				bss_index = radio_data[i].interface_mib[j].bss_index;
				if(0 == bss_index) {
					sprintf(interface_name, "%s", root_name);
				} else {
					sprintf(interface_name, "%s-%s%d", root_name, map_vap_prefix, bss_index - 1);
				}
				if (0 == PLATFORM_STRCMP(interface, interface_name)) {
					radio_idx = i;
					if_idx = j;
					break;
				}
			}
		}

		if (if_idx == -1)
			return 0;

		i = radio_idx;
		j = if_idx;

		is_up = radio_data[i].interface_mib[j].is_enabled;

		return is_up;

	} else {
		skfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);

		if (skfd == -1)
			return 0;

		strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);

		if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
			close(skfd);
			return 0;
		}
		close(skfd);
		return (INT8U) !!(ifr.ifr_flags & IFF_UP);
	}
}

int PLATFORM_GET_INTERFACE_STATUS(char *interface, int *interface_index)
{
	int          skfd = 0;
	struct ifreq ifr; // { 0 } this null initializer is not recognized by 1.4.1 toolchain.

	PLATFORM_MEMSET(&ifr, 0, sizeof(struct ifreq));

	skfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);

	if (skfd == -1)
		return 0;

	ifr.ifr_ifindex = 0;
	strncpy(ifr.ifr_name, interface, IFNAMSIZ - 1);

	if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
		close(skfd);
		PLATFORM_PRINTF_ERROR("[%s][%d] Fail to get interface %s flags \n", __FUNCTION__, __LINE__, interface);
		return -1;
	}

	if (ioctl(skfd, SIOCGIFINDEX, &ifr) == -1) {
		PLATFORM_PRINTF_ERROR("[%s][%d] ioctl('%s',SIOCGIFINDEX) returned with errno=%d (%s) \n", __FUNCTION__, __LINE__, interface, errno, strerror(errno));
	}

	*interface_index = ifr.ifr_ifindex;

	PLATFORM_PRINTF_INFO("[%s][%d] Interface %s retrived flag %d and index %d \n", __FUNCTION__, __LINE__, interface, !!(ifr.ifr_flags & IFF_UP), *interface_index);

	close(skfd);
	return (INT8U) !!(ifr.ifr_flags & IFF_UP);
}

char *PLATFORM_GET_IP_ADDRESS()
{
	int          fd;
	struct ifreq ifr;
	char *       ip_addr;

	fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);

	if (fd == -1)
		return 0;

	/* I want to get an IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;

	/* I want IP address attached to "eth0" */
	strncpy(ifr.ifr_name, map_bridge_name, IFNAMSIZ - 1);

	(void) ioctl(fd, SIOCGIFADDR, &ifr);

	close(fd);

	/* display result */
	struct in_addr *addr = &(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
	ip_addr              = inet_ntoa(*addr);

	return ip_addr;
}

INT8U PLATFORM_IS_INTERFACE_UP_AND_ON(char *interface)
{
	INT32U is_func_off = 0;
	INT8U  is_up = 0, bss_type = 0;
	PLATFORM_GET_MIB(interface, "func_off", &is_func_off, 4);
	bss_type = PLATFORM_GET_BSS_TYPE(interface);
	is_up    = PLATFORM_IS_INTERFACE_UP(interface);
	// PLATFORM_PRINTF_DEBUG("Interface %s up and on %02x\n", interface, (is_up && !is_func_off && (bss_type != MASK_HAUL_TEARDOWN)));
	return (is_up && !is_func_off && (bss_type != MASK_HAUL_TEARDOWN));
}

INT8U PLATFORM_NEED_HAPD()
{
#ifdef _RTK_LINUX_
	return rtk_linux_need_hapd();
#else
	return 0;
#endif
}

INT8U PLATFORM_INTERFACE_HAS_AX_SUPPORT(char *interface)
{
#ifdef _RTK_LINUX_
	return rtk_get_is_ax_support(interface);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_INTERFACE_HAS_AX_SUPPORT for %s is not implemented!\n", interface);
	return 0;
#endif
}

INT8U PLATFORM_GET_ETHERNET_CLIENTS(INT8U *neighbor_mac_addresses_nr, struct ethernetNeighborInfo **neighbor_list, char *connected_interface)
{
#if defined(_RTK_LINUX_)
	return rtk_linux_update_ethernet_clients(neighbor_mac_addresses_nr, neighbor_list, connected_interface);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_GET_ETHERNET_CLIENTS is not implemented!\n");
	return 0;
#endif
}

INT8U PLATFORM_TRIGGER_WPS(char *interface)
{
#if defined(_RTK_LINUX_)
	return rtk_linux_trigger_wps(interface);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_TRIGGER_WPS is not implemented!\n");
	return 0;
#endif
}

struct eth_port_map *PLATFORM_GET_ETH_PORT_1905(void)
{
	return &global_eth_recv_1905[0];
}

INT8U PLATFORM_CHECK_CONNECTED_MAC(char *interface_name, INT8U *connected_address)
{
#if defined(_RTK_LINUX_)
	char command[200];
	FILE * pipe;
	char * line;
	size_t len;

	char file_path[64] = {0};

	snprintf(file_path, sizeof(file_path), PROC_DIR "%s/sta_info", interface_name);

	if (-1 == access(file_path, F_OK)) {
		PLATFORM_PRINTF_ERROR("File %s does not exist!\n", file_path);
		return 0;
	}

	snprintf(command, sizeof(command), "cat %s | grep -n %02x%02x%02x%02x%02x%02x",
		        file_path,
		        connected_address[0], connected_address[1], connected_address[2],
		        connected_address[3], connected_address[4], connected_address[5]);

	pipe = popen(command, "re");

	if (!pipe) {
		PLATFORM_PRINTF_ERROR("[PLATFORM] popen() returned with errno=%d (%s)\n", errno, strerror(errno));
		return 0;
	}

	line = NULL;
	if (-1 != getline(&line, &len, pipe)) {
		PLATFORM_PRINTF_DETAIL("[PLATFORM] Station is found under %s\n", interface_name);
		free(line);
		pclose(pipe);
		return 1;
	} else {
		PLATFORM_PRINTF_DETAIL("[PLATFORM] Station is not found under %s\n", interface_name);
		free(line);
		pclose(pipe);
		return 0;
	}

#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] PLATFORM_CHECK_CONNECTED_MAC is not implemented!\n");
	return 0;
#endif
}

// rtk_linux_get_clients_rssi supports multiple clients retrival, can add a new PLATFORM wrapper for multiple clients rssi if needed.
// PLATFORM_GET_CLIENT_RCPI is hard coded to get 1 client rssi only.
INT8U PLATFORM_GET_CLIENT_RCPI(char *interface_name, INT8U *neighbor_interface_address, INT8U allow_sta_mode)
{
	INT8U rssi = 0;
#if defined(_RTK_LINUX_)
	INT8U *result = NULL;
	result = rtk_linux_get_clients_rssi(interface_name, neighbor_interface_address, 1, allow_sta_mode);
	if (result) {
		rssi = result[0];
		PLATFORM_FREE(result);
	}
	if (rssi == 0) {
		rssi = _readWifiNeighborParameter(interface_name, neighbor_interface_address, "rssi:");
	}
#else
	rssi = _readWifiNeighborParameter(interface_name, neighbor_interface_address, "rssi:");
#endif
	return rssi2rcpi(rssi);
}

void PLATFORM_SET_SP_RULE(struct ServicePrioritizationRuleTLV *rule)
{
	PLATFORM_ALL_RADIOS_TLV_SET((INT8U *)rule);
	return;
}

void PLATFORM_CLEAR_SP_RULE()
{
	// Forge a dummy rule that has add_remove_rule bit to 0, then apply it as if it was a normal rule.
	struct ServicePrioritizationRuleTLV *rule_dummy = (struct ServicePrioritizationRuleTLV *)PLATFORM_MALLOC(sizeof(struct ServicePrioritizationRuleTLV));
	PLATFORM_MEMSET(rule_dummy, 0, sizeof(struct ServicePrioritizationRuleTLV));
	rule_dummy->tlv_type = TLV_TYPE_SERVICE_PRIORITIZATION_RULE;
	rule_dummy->always_match = ALWAYS_MATCH_BIT_MASK;
	// rule_dummy->add_remove_rule is already 0 when memset.

	PLATFORM_ALL_RADIOS_TLV_SET((INT8U *)rule_dummy);
	PLATFORM_FREE(rule_dummy);

	return;
}

void PLATFORM_SET_DSCP_PCP_MAPPING_TABLE(struct DSCPMappingTableTLV *table)
{
	PLATFORM_ALL_RADIOS_TLV_SET((INT8U *)table);
	return;
}

/*
	This function is used to pass tlv information to all radios, 24g, 5gl, and 5gh if triband.
	General ioctl will take care of the tlv and pass it to general ioctl handler in the driver.

	Remember first three bytes of tlv_buffer should be zeros when using general ioctl. So clear
	the first three byte when reusing the same buffer for multiple ioctl general calls.
*/
void PLATFORM_ALL_RADIOS_TLV_SET(INT8U *tlv)
{
	INT8U *stream;
	INT8U  buffer[512] = { 0 };
	INT16U len;
	int    result = 0;
	int    i;
	int    radio_names_nr = 0;
	char **radio_names    = DMgetRadioNames(&radio_names_nr);

	stream = forge_1905_TLV_from_structure(tlv, &len, 0);
	if (!stream) {
		char *tlv_name = convert_1905_TLV_type_to_string(*tlv);
		PLATFORM_PRINTF_INFO("Error: failed to forge %s tlv stream from struct \n", tlv_name);
		return;
	}
	PLATFORM_MEMSET(buffer, 0, sizeof(buffer));
	PLATFORM_MEMCPY(buffer + 3, stream, len);
	PLATFORM_FREE(stream);

	for (i = 0; i < radio_names_nr; i++) {
		result = PLATFORM_MAP_SET_GENERAL_IOCTL(radio_names[i], buffer, (INT16U)sizeof(buffer));
		if (result <= 0) {
			PLATFORM_PRINTF_INFO("ERROR: PLATFORM_ALL_RADIOS_TLV_SET on radio %s return value: [%d] \n", radio_names[i], result);
		}
		PLATFORM_MEMSET(buffer, 0, 3);
	}

	return;
}

#ifdef RTK_MULTI_AP_R4
void PLATFORM_MAP_APPLY_SPATIAL_REUSE_CONFIG(struct SpatialReuseRequestTLV *request, char *interface_name)
{
#ifdef _RTK_LINUX_
	rtk_linux_apply_spatial_reuse_config(request, interface_name);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] APPLY_SPATIAL_REUSE_CONFIG not implemented!\n");
#endif
}

void PLATFORM_MAP_GET_SPATIAL_REUSE_CONFIG_RESPONSE(struct SpatialReuseConfigResponseTLV **response, INT8U *response_nr)
{
#ifdef _RTK_LINUX_
	rtk_linux_get_spatial_reuse_config_response(response, response_nr);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] GET_SPATIAL_REUSE_CONFIG_RESPONSE not implemented!\n");
#endif
}

INT8U PLATFORM_MAP_GET_SPATIAL_REUSE_REPORT(INT8U *buffer, char *interface_name)
{
#ifdef _RTK_LINUX_
	return rtk_linux_get_spatial_reuse_report_tlv(buffer, interface_name);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] GET_SPATIAL_REUSE_REPORT not implemented!\n");
	return 0;
#endif
}

void PLATFORM_MAP_START_RECONFIG(char *ifname)
{
#ifdef _RTK_LINUX_
	rtk_linux_start_reconfig(ifname);
#else
	PLATFORM_PRINTF_ERROR("[PLATFORM] START_RECONFIG not implemented!\n");
#endif
}
#endif // RTK_MULTI_AP_R4