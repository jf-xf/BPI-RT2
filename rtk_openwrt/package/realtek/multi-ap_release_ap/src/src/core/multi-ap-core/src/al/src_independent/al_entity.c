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

// In the comments below, every time a reference is made (ex: "See Section 6.4"
// or "See Table 6-11") we are talking about the contents of the following
// document:
//
//   "IEEE Std 1905.1-2013"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h> // threads and mutex functions

#include "packet_tools.h"
#include "platform.h"
#include "utils.h"

#include "1905_alme.h"
#include "1905_cmdus.h"
#include "1905_l2.h"
#include "1905_tlvs.h"
#include "lldp_payload.h"

#include "al.h"
#include "al_datamodel.h"
#include "al_recv.h"
#include "al_send.h"
#include "al_utils.h"
#include "al_wsc.h"
#include "al_dpp.h"
#include "al_dpp_eapol.h"
#include "al_dpp_keystore.h"
#include "al_dpp_counters.h"
#include "al_buffer.h"

#include "platform_crypto_aes.h"
#include "platform_interfaces.h"
#include "platform_os.h"

#include "map_utils.h"
#include "map_initialization.h"
#include "api_response.h"

#include "global_settings.h"

#define BACKHAUL_STEERING_CHECK_SECOND (10)

#define RTK_DEFAULT_DECRYPT_FAILURE_THRESHOLD 10

////////////////////////////////////////////////////////////////////////////////
// Private functions and data
////////////////////////////////////////////////////////////////////////////////
static int core_thread_flag = 1;

static pthread_mutex_t map_core_mutex = PTHREAD_MUTEX_INITIALIZER;

#ifdef RTK_MULTI_AP_R2
INT8U                  _on_boot_channel_scan()
{
	//Forge a scan request includes all possible op classes and channels
	struct ChannelScanRequestTLV channel_scan_request_tlv;
	channel_scan_request_tlv.tlv_type = TLV_TYPE_CHANNEL_SCAN_REQUEST;
	channel_scan_request_tlv.flags    = 0x00;
	if (DMisTribandDevice()) {
		channel_scan_request_tlv.target_radio_nr = 3;
		channel_scan_request_tlv.target_radios   = (struct ChannelScanTargetRadio *)PLATFORM_MALLOC(3 * sizeof(struct ChannelScanTargetRadio));
	} else {
		channel_scan_request_tlv.target_radio_nr = 2;
		channel_scan_request_tlv.target_radios   = (struct ChannelScanTargetRadio *)PLATFORM_MALLOC(2 * sizeof(struct ChannelScanTargetRadio));
	}

	if (channel_scan_request_tlv.target_radios == NULL) {
		PLATFORM_PRINTF_ERROR("[RTK] Failed to allocate target_radios\n");
		return 1;
	}
	// `channel_scan_request_tlv.target_radios` allocated by malloc now, remember to release before return

	INT8U           n;
	int             op_classes_len;
	OP_CLASS *op_classes = PLATFORM_GET_OP_CLASS(&op_classes_len);

	if (NULL == op_classes) {
		PLATFORM_PRINTF_ERROR("[RTK] No operating class is available\n");
		PLATFORM_FREE(channel_scan_request_tlv.target_radios);
		return 1;
	}

	//5GL radio
	struct LocalInterface *x = DMnameToLocalInterfaceStruct(map_radio_name_5gl);
	if (NULL == x) {
		PLATFORM_PRINTF_DEBUG("map_radio_name_5gl get LocalInterfaceStruct NULL!\n");
		PLATFORM_FREE(op_classes);
		PLATFORM_FREE(channel_scan_request_tlv.target_radios);
		return 1;
	}
	PLATFORM_MEMCPY(channel_scan_request_tlv.target_radios[0].radio_unique_identifier, x->mac_address, 6);
	channel_scan_request_tlv.target_radios[0].target_op_class_nr = 0;
	channel_scan_request_tlv.target_radios[0].target_op_classes  = NULL;

	//2G radio
	x = DMnameToLocalInterfaceStruct(map_radio_name_24g);
	if (NULL == x) {
		PLATFORM_PRINTF_DEBUG("map_radio_name_24g get LocalInterfaceStruct NULL!\n");
		PLATFORM_FREE(op_classes);
		PLATFORM_FREE(channel_scan_request_tlv.target_radios);
		return 1;
	}
	PLATFORM_MEMCPY(channel_scan_request_tlv.target_radios[1].radio_unique_identifier, x->mac_address, 6);
	channel_scan_request_tlv.target_radios[1].target_op_class_nr = 0;
	channel_scan_request_tlv.target_radios[1].target_op_classes  = NULL;

	//5GH radio
	if (DMisTribandDevice()) {
		x = DMnameToLocalInterfaceStruct(map_radio_name_5gh);
		if (NULL == x) {
			PLATFORM_PRINTF_DEBUG("map_radio_name_5gh get LocalInterfaceStruct NULL!\n");
			PLATFORM_FREE(op_classes);
			PLATFORM_FREE(channel_scan_request_tlv.target_radios);
			return 1;
		}
		PLATFORM_MEMCPY(channel_scan_request_tlv.target_radios[2].radio_unique_identifier, x->mac_address, 6);
		channel_scan_request_tlv.target_radios[2].target_op_class_nr = 0;
		channel_scan_request_tlv.target_radios[2].target_op_classes  = NULL;
	}

	for (n = 0; n < op_classes_len; n++) {
		if (op_classes[n].band == _5G && (op_classes[n].sub_band == _5GL || !DMisTribandDevice())) {
			channel_scan_request_tlv.target_radios[0].target_op_classes                                                                                   = (struct ChannelScanTargetOpClass *)PLATFORM_REALLOC(channel_scan_request_tlv.target_radios[0].target_op_classes, (channel_scan_request_tlv.target_radios[0].target_op_class_nr + 1) * sizeof(struct ChannelScanTargetOpClass));
			channel_scan_request_tlv.target_radios[0].target_op_classes[channel_scan_request_tlv.target_radios[0].target_op_class_nr].op_class            = op_classes[n].op_class;
			channel_scan_request_tlv.target_radios[0].target_op_classes[channel_scan_request_tlv.target_radios[0].target_op_class_nr].target_channel_nr   = 0;
			channel_scan_request_tlv.target_radios[0].target_op_classes[channel_scan_request_tlv.target_radios[0].target_op_class_nr].target_channel_list = NULL;
			channel_scan_request_tlv.target_radios[0].target_op_class_nr++;
		} else if (op_classes[n].band == _2G) {
			channel_scan_request_tlv.target_radios[1].target_op_classes                                                                                   = (struct ChannelScanTargetOpClass *)PLATFORM_REALLOC(channel_scan_request_tlv.target_radios[1].target_op_classes, (channel_scan_request_tlv.target_radios[1].target_op_class_nr + 1) * sizeof(struct ChannelScanTargetOpClass));
			channel_scan_request_tlv.target_radios[1].target_op_classes[channel_scan_request_tlv.target_radios[1].target_op_class_nr].op_class            = op_classes[n].op_class;
			channel_scan_request_tlv.target_radios[1].target_op_classes[channel_scan_request_tlv.target_radios[1].target_op_class_nr].target_channel_nr   = 0;
			channel_scan_request_tlv.target_radios[1].target_op_classes[channel_scan_request_tlv.target_radios[1].target_op_class_nr].target_channel_list = NULL;
			channel_scan_request_tlv.target_radios[1].target_op_class_nr++;
		} else if (op_classes[n].band == _5G && op_classes[n].sub_band == _5GH && DMisTribandDevice()) {
			channel_scan_request_tlv.target_radios[2].target_op_classes                                                                                   = (struct ChannelScanTargetOpClass *)PLATFORM_REALLOC(channel_scan_request_tlv.target_radios[2].target_op_classes, (channel_scan_request_tlv.target_radios[2].target_op_class_nr + 1) * sizeof(struct ChannelScanTargetOpClass));
			channel_scan_request_tlv.target_radios[2].target_op_classes[channel_scan_request_tlv.target_radios[2].target_op_class_nr].op_class            = op_classes[n].op_class;
			channel_scan_request_tlv.target_radios[2].target_op_classes[channel_scan_request_tlv.target_radios[2].target_op_class_nr].target_channel_nr   = 0;
			channel_scan_request_tlv.target_radios[2].target_op_classes[channel_scan_request_tlv.target_radios[2].target_op_class_nr].target_channel_list = NULL;
			channel_scan_request_tlv.target_radios[2].target_op_class_nr++;
		}
	}

	//buffer this request for reforming result tlvs
	DMbufferChannelScanRequest(&channel_scan_request_tlv);

	INT8U channels_2g_nr   = 0;
	INT8U channels_2g[128] = { 0 };

	channels_2g_nr = PLATFORM_GET_AVAILABLE_CHANNELS(map_radio_name_24g, channels_2g, (INT16U)sizeof(channels_2g), FILTER_NONE);

	INT8U channels_5gl_nr   = 0;
	INT8U channels_5gl[128] = { 0 };

	if (DMisTribandDevice()) {
		channels_5gl_nr = PLATFORM_GET_AVAILABLE_CHANNELS(map_radio_name_5gl, channels_5gl, (INT16U)sizeof(channels_5gl), FILTER_5GL);
	} else {
		channels_5gl_nr = PLATFORM_GET_AVAILABLE_CHANNELS(map_radio_name_5gl, channels_5gl, (INT16U)sizeof(channels_5gl), FILTER_NONE);
	}

	INT8U channels_5gh_nr   = 0;
	INT8U channels_5gh[128] = { 0 };
	if (DMisTribandDevice()) {
		channels_5gh_nr = PLATFORM_GET_AVAILABLE_CHANNELS(map_radio_name_5gh, channels_5gh, (INT16U)sizeof(channels_5gh), FILTER_5GH);
	}

	PLATFORM_CHANNEL_SCAN_IOCTL(map_radio_name_24g, channels_2g_nr, channels_2g);
	PLATFORM_CHANNEL_SCAN_IOCTL(map_radio_name_5gl, channels_5gl_nr, channels_5gl);
	if (DMisTribandDevice())
		PLATFORM_CHANNEL_SCAN_IOCTL(map_radio_name_5gh, channels_5gh_nr, channels_5gh);

	// if (channels_5gh_nr)
	// 	PLATFORM_FREE(channels_5gh);

	// PLATFORM_FREE(channels_5gl);
	// PLATFORM_FREE(channels_2g);

	PLATFORM_FREE(op_classes);
	PLATFORM_FREE(channel_scan_request_tlv.target_radios);

	DMsetOnBootScanFlag(1);

	return 0;
}

//this function is to check and remove duplicated channels before sending down to wifi driver
void _insert_channel_into_channel_list(INT8U **curr_channel_list, INT8U *curr_channel_nr, INT8U inserting_channel)
{
	INT8U i;

	if (NULL == curr_channel_nr) {
		PLATFORM_PRINTF_ERROR("[RTK] curr_channel_nr is NULL\n");
		return;
	}

	if (NULL == *curr_channel_list) {
		*curr_channel_list      = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U));
		(*curr_channel_list)[0] = inserting_channel;
		(*curr_channel_nr)++;
	} else {
		for (i = 0; i < (*curr_channel_nr); i++) {
			if ((*curr_channel_list)[i] == inserting_channel) {
				//duplicate found. ignore the inserting channel
				return;
			}
		}
		//else insert the current to the list
		*curr_channel_list                     = (INT8U *)PLATFORM_REALLOC(*curr_channel_list, (*curr_channel_nr + 1) * sizeof(INT8U));
		(*curr_channel_list)[*curr_channel_nr] = inserting_channel;
		(*curr_channel_nr)++;
	}
}

INT8U _construct_channel_scan_channel_list(struct ChannelScanRequestTLV *t, INT8U **curr_channel_list_2g, INT8U *curr_channel_nr_2g, INT8U **curr_channel_list_5gh, INT8U *curr_channel_nr_5gh, INT8U **curr_channel_list_5gl, INT8U *curr_channel_nr_5gl)
{

	INT8U           k, l, m, n;
	int             op_classes_len;
	OP_CLASS *op_classes = PLATFORM_GET_OP_CLASS(&op_classes_len);

	if (NULL == op_classes) {
		PLATFORM_PRINTF_ERROR("[RTK] No operating class is available\n");
		return 1;
	}

	INT8U **curr_channel_list = NULL, *curr_channel_nr = NULL;

	for (k = 0; k < t->target_radio_nr; k++) {
		for (l = 0; l < t->target_radios[k].target_op_class_nr; l++) {
			//channel nr 0 indicating a need to scan all channels in this op class
			if (0 == t->target_radios[k].target_op_classes[l].target_channel_nr) {
				for (n = 0; n < op_classes_len; n++) {
					if (t->target_radios[k].target_op_classes[l].op_class == op_classes[n].op_class) {
						PLATFORM_PRINTF_DEBUG("Channel nr in Channel Scan Request for op class %d is 0. Scanning for all channels in this op class...\n", t->target_radios[k].target_op_classes[l].op_class);
						if (op_classes[n].band == _2G) {
							curr_channel_nr   = curr_channel_nr_2g;
							curr_channel_list = curr_channel_list_2g;
						} else if (op_classes[n].band == _5G && op_classes[n].sub_band == _5GH) {
							curr_channel_nr   = curr_channel_nr_5gh;
							curr_channel_list = curr_channel_list_5gh;
						} else if (op_classes[n].band == _5G && op_classes[n].sub_band == _5GL) {
							curr_channel_nr   = curr_channel_nr_5gl;
							curr_channel_list = curr_channel_list_5gl;
						}
						break;
					}
				}
				if (n == op_classes_len) {
					PLATFORM_PRINTF_WARNING("Channel nr in Channel Scan Request for op class %d is 0, but this op class is invalid. Aborting Channel Scan Request sending...\n", t->target_radios[k].target_op_classes[l].op_class);
					PLATFORM_FREE(op_classes);
					return 1;
				}
				for (m = 0; m < op_classes[n].channel_array[0]; m++) {
					_insert_channel_into_channel_list(curr_channel_list, curr_channel_nr, op_classes[n].channel_array[m + 1]);
				}
			} else {
				for (n = 0; n < op_classes_len; n++) {
					if (t->target_radios[k].target_op_classes[l].op_class == op_classes[n].op_class) {
						if (op_classes[n].band == _2G) {
							curr_channel_nr   = curr_channel_nr_2g;
							curr_channel_list = curr_channel_list_2g;
						} else if (op_classes[n].band == _5G && op_classes[n].sub_band == _5GH) {
							curr_channel_nr   = curr_channel_nr_5gh;
							curr_channel_list = curr_channel_list_5gh;
						} else if (op_classes[n].band == _5G && op_classes[n].sub_band == _5GL) {
							curr_channel_nr   = curr_channel_nr_5gl;
							curr_channel_list = curr_channel_list_5gl;
						}
						break;
					}
				}
				if (n == op_classes_len) {
					PLATFORM_PRINTF_WARNING("Op class %d in Channel Scan Request is invalid. Aborting Channel Scan Request sending...\n", t->target_radios[k].target_op_classes[l].op_class);
					PLATFORM_FREE(op_classes);
					return 1;
				}
				for (n = 0; n < t->target_radios[k].target_op_classes[l].target_channel_nr; n++) {
					_insert_channel_into_channel_list(curr_channel_list, curr_channel_nr, t->target_radios[k].target_op_classes[l].target_channel_list[n]);
				}
			}
		}
	}

	PLATFORM_FREE(op_classes);

	return 0;
}

INT8U _perform_channel_scan()
{
	//construct channel list and send down to wifi driver
	INT8U  channel_nr_2g = 0, channel_nr_5gl = 0, channel_nr_5gh = 0;
	INT8U *channel_list_2g = NULL, *channel_list_5gl = NULL, *channel_list_5gh = NULL;

	struct ChannelScanRequestTLV *request = DMgetBufferedChannelScanRequest();

	if (_construct_channel_scan_channel_list(request, &channel_list_2g, &channel_nr_2g, &channel_list_5gh, &channel_nr_5gh, &channel_list_5gl, &channel_nr_5gl)) {
		//error
		if (channel_list_2g) {
			PLATFORM_FREE(channel_list_2g);
		}
		if (channel_list_5gh) {
			PLATFORM_FREE(channel_list_5gh);
		}
		if (channel_list_5gl) {
			PLATFORM_FREE(channel_list_5gl);
		}
		return 1;
	}

	if (NULL == channel_list_2g) {
		DMsetChannelScanCompleted(_2G);
	} else {
		PLATFORM_CHANNEL_SCAN_IOCTL(map_radio_name_24g, channel_nr_2g, channel_list_2g);
	}

	if (NULL != channel_list_5gl && NULL != channel_list_5gh) {
		if (DMisTribandDevice()) {
			PLATFORM_CHANNEL_SCAN_IOCTL(map_radio_name_5gh, channel_nr_5gh, channel_list_5gh);
			PLATFORM_CHANNEL_SCAN_IOCTL(map_radio_name_5gl, channel_nr_5gl, channel_list_5gl);
		} else {
			INT8U *channel_list_5g = NULL;
			INT8U  channel_nr_5g   = 0;
			int    i               = 0;
			for (i = 0; i < channel_nr_5gl; i++) {
				_insert_channel_into_channel_list(&channel_list_5g, &channel_nr_5g, channel_list_5gl[i]);
			}
			for (i = 0; i < channel_nr_5gh; i++) {
				_insert_channel_into_channel_list(&channel_list_5g, &channel_nr_5g, channel_list_5gh[i]);
			}
			PLATFORM_CHANNEL_SCAN_IOCTL(map_radio_name_5gl, channel_nr_5g, channel_list_5g);
			if (channel_nr_5g > 0) {
				PLATFORM_FREE(channel_list_5g);
			}
		}
	} else if (NULL != channel_list_5gh) {
		DMsetChannelScanCompleted(_5GL);
		PLATFORM_CHANNEL_SCAN_IOCTL(map_radio_name_5gh, channel_nr_5gh, channel_list_5gh);
	} else if (NULL != channel_list_5gl) {
		DMsetChannelScanCompleted(_5GH);
		PLATFORM_CHANNEL_SCAN_IOCTL(map_radio_name_5gl, channel_nr_5gl, channel_list_5gl);
	} else {
		DMsetChannelScanCompleted(_5GL);
		DMsetChannelScanCompleted(_5GH);
	}

	if (channel_list_2g) {
		PLATFORM_FREE(channel_list_2g);
	}
	if (channel_list_5gh) {
		PLATFORM_FREE(channel_list_5gh);
	}
	if (channel_list_5gl) {
		PLATFORM_FREE(channel_list_5gl);
	}

	return 0;
}
#endif
void _kill_handler(int sig)
{
	PLATFORM_PRINTF_INFO("Kill signal %d received at main, killing all threads...\n", sig);
	CLOSE_ALME_THREAD();
	pthread_mutex_lock(&map_core_mutex);
	core_thread_flag = 0;
	pthread_mutex_unlock(&map_core_mutex);
	if (SIGTERM == sig)
		exit(0);
}

// INT8U _check_stp_state()
// {
// 	int   counter = -1, i = 0;
// 	char  checked = 0;
// 	FILE *in      = NULL;
// 	char  temp[9];
// 	int   ch;
// 	char  vxd[3]   = map_vxd_prefix;
// 	char  state[8] = "blocking";
// 	int   res      = 0;
// 	//system("brctl stp br0 on");//they force me change it
// 	in = popen("brctl showstp br0", "r");
// 	if (in == NULL) {
// 		puts("Unable to open process");
// 		return 0;
// 	}

// 	while ((ch = fgetc(in)) != EOF) {
// 		if (checked && ch == state[i] && i < 8) {
// 			i++;
// 		} else if (i == 8) {
// 			pclose(in);
// 			return 1;
// 		}
// 		if (counter == 8) {
// 			if (memcmp(vxd, temp + 6, 3) == 0) {
// 				checked = 1;
// 			} else {
// 				counter = -1;
// 			}
// 		}
// 		if (ch == 'w' || (counter >= 0 && counter < 8)) {
// 			counter++;
// 			temp[counter] = ch;
// 		}
// 	}
// 	pclose(in);
// 	for (i = 0; i < 2; i++) {
// 		if (i == 0)
// 			in = popen("cat /proc/wlan0-vxd/sta_info", "r");
// 		else
// 			in = popen("cat /proc/wlan1-vxd/sta_info", "r");

// 		if (in == NULL) {
// 			puts("Unable to open process");
// 			return 0;
// 		}
// 		counter = 0;
// 		while ((ch = fgetc(in)) != EOF) {
// 			if (counter == 30)
// 				res += ch - '0';
// 			counter++;
// 		}
// 		pclose(in);
// 	}
// 	if (res == 0)
// 		return 1;
// 	return 0;
// }

// CMDUs can be received in multiple fragments/packets when they are too big to
// fit in a single "network transmission unit" (which is never bigger than
// MAX_NETWORK_SEGMENT_SIZE).
//
// Fragments that belong to one same CMDU contain the same 'mid' and different
// 'fragment id' values. In addition, the last fragment is the only one to
// contain the 'last fragment indicator' field set.
//
//   NOTE: This is all also explained in "Sections 7.1.1 and 7.1.2"
//
// This function will "buffer" fragments until either all pieces arrive or a
// timer expires (in which case all previous fragments are discarded/ignored)
//
//   NOTE: Instead of a timer, we will use a buffer that holds up to
//         MAX_MIDS_IN_FLIGHT CMDUs.
//         If we are still waiting for MAX_MIDS_IN_FLIGHT CMDUs to be completed
//         (ie. we haven't received all their fragments yet), and a new fragment
//         for a new CMDU arrives, we will discard all fragments from the
//         oldest one.
//
// Every time this function is called, two things can happen:
//
//   1. The just received fragment was the last one needed to complete a CMDU.
//      In this case, the CMDU structure result of all those fragments being
//      parsed is returned.
//
//   2. The just received fragment is not yet the last one needed to complete a
//      CMDU. In this case the fragment is internally buffered (ie. the caller
//      does not need to keep the passed buffer around in memory) and this
//      function returns NULL.
//
// This function received two arguments:
//
//   - 'packet_buffer' is a pointer to the received stream containing a
//     fragment (or a whole) CMDU
//
//   - 'len' is the length of this 'packet_buffer' in bytes
//
struct CMDU *_reAssembleFragmentedCMDUs(INT8U *packet_buffer, INT16U len)
{
#define MAX_MIDS_IN_FLIGHT 8
#define MAX_FRAGMENTS_PER_MID 10

	// This is a static structure used to store the fragments belonging to up to
	// 'MAX_MIDS_IN_FLIGHT' CMDU messages.
	// Initially all entries are marked as "empty" by setting the 'in_use' field
	// to "0"
	//
	static struct _midsInFlight {
		INT8U in_use; // Is this entry free?

		INT16U mid; // 'mid' associated to this CMDU

		INT8U src_addr[6];
		INT8U dst_addr[6];
		// These two (together with the 'mid' field) will be used
		// to identify fragments belonging to one same CMDU.

		INT8U fragments[MAX_FRAGMENTS_PER_MID];
		// Each entry represents a fragment number.
		//   - "1" means that fragment has been received
		//   - "0" means no fragment with that number has been
		//     received.

		INT8U last_fragment;
		// Number of the fragment carrying the
		// 'last_fragment_indicator' flag.
		// This is always a number between 0 and
		// MAX_FRAGMENTS_PER_MID-1.
		// Iniitally it is set to "MAX_FRAGMENTS_PER_MID",
		// meaning that no fragment with the
		// 'last_fragment_indicator' flag has been received yet.

		INT8U *streams[MAX_FRAGMENTS_PER_MID + 1];
		// Each of the bit streams associated to each fragment
		//
		// The size is "MAX_FRAGMENTS_PER_MID+1" instead of
		// "MAX_FRAGMENTS_PER_MID" to store a final NULL entry
		// (this makes it easier to later call
		// "parse_1905_CMDU_header_from_packet()"

		INT16U stream_lens[MAX_FRAGMENTS_PER_MID + 1];

		INT32U age; // Used to keep track of which is the oldest CMDU for
		            // which a fragment was received (so that we can free
		            // it when the CMDUs buffer is full)

	} mids_in_flight[MAX_MIDS_IN_FLIGHT] = { [0 ... MAX_MIDS_IN_FLIGHT - 1] = (struct _midsInFlight){ .in_use = 0 } };

	static INT32U current_age = 0;

	INT8U  dst_addr[6];
	INT8U  src_addr[6];
	INT16U ether_type;

	INT16U mid;
	INT8U  fragment_id;
	INT8U  last_fragment_indicator;

	INT8U  i, j;
	INT8U *p;

	p = packet_buffer;

	_EnB(&p, dst_addr, 6);
	_EnB(&p, src_addr, 6);
	_E2B(&p, &ether_type);

	len -= (6 + 6 + 2);

	if (0 == parse_1905_CMDU_header_from_packet(p, &mid, &fragment_id, &last_fragment_indicator)) {
		PLATFORM_PRINTF_ERROR("Could not retrieve CMDU header from bit stream\n");
		return NULL;
	}
	PLATFORM_PRINTF_DETAIL("mid = %04x, fragment_id = %d, last_fragment_indicator = %d\n", mid, fragment_id, last_fragment_indicator);

	// Find the set of streams associated to this 'mid' and add the just
	// received stream to its set of streams
	//
	for (i = 0; i < MAX_MIDS_IN_FLIGHT; i++) {
		if (
		    1 == mids_in_flight[i].in_use && mid == mids_in_flight[i].mid && 0 == PLATFORM_MEMCMP(dst_addr, mids_in_flight[i].dst_addr, 6) && 0 == PLATFORM_MEMCMP(src_addr, mids_in_flight[i].src_addr, 6)) {
			// Fragments for this 'mid' have previously been received. Add this
			// new one to the set.

			// ...but first check for errors
			//
			if (fragment_id >= MAX_FRAGMENTS_PER_MID) {
				PLATFORM_PRINTF_ERROR("Too many fragments (%d) for one same CMDU (max supported is %d)\n", fragment_id, MAX_FRAGMENTS_PER_MID - 1);
				PLATFORM_PRINTF_ERROR("  mid      = %04x\n", mid);
				PLATFORM_PRINTF_ERROR("  src_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", src_addr[0], src_addr[1], src_addr[2], src_addr[3], src_addr[4], src_addr[5]);
				PLATFORM_PRINTF_ERROR("  dst_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", dst_addr[0], dst_addr[1], dst_addr[2], dst_addr[3], dst_addr[4], dst_addr[5]);
				return NULL;
			}

			if (1 == mids_in_flight[i].fragments[fragment_id]) {
				PLATFORM_PRINTF_WARNING("Ignoring duplicated fragment #%d\n", fragment_id);
				PLATFORM_PRINTF_WARNING("  mid      = %04x\n", mid);
				PLATFORM_PRINTF_WARNING("  src_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", src_addr[0], src_addr[1], src_addr[2], src_addr[3], src_addr[4], src_addr[5]);
				PLATFORM_PRINTF_WARNING("  dst_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", dst_addr[0], dst_addr[1], dst_addr[2], dst_addr[3], dst_addr[4], dst_addr[5]);
				return NULL;
			}

			if (1 == last_fragment_indicator && MAX_FRAGMENTS_PER_MID != mids_in_flight[i].last_fragment) {
				PLATFORM_PRINTF_WARNING("This fragment (#%d) and a previously received one (#%d) both contain the 'last_fragment_indicator' flag set. Ignoring...\n", fragment_id, mids_in_flight[i].last_fragment);
				PLATFORM_PRINTF_WARNING("  mid      = %04x\n", mid);
				PLATFORM_PRINTF_WARNING("  src_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", src_addr[0], src_addr[1], src_addr[2], src_addr[3], src_addr[4], src_addr[5]);
				PLATFORM_PRINTF_WARNING("  dst_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", dst_addr[0], dst_addr[1], dst_addr[2], dst_addr[3], dst_addr[4], dst_addr[5]);
				return NULL;
			}

			// ...and now actually save the stream for later
			//
			mids_in_flight[i].fragments[fragment_id] = 1;

			if (1 == last_fragment_indicator) {
				mids_in_flight[i].last_fragment = fragment_id;
			}

			mids_in_flight[i].streams[fragment_id] = (INT8U *)PLATFORM_MALLOC((sizeof(INT8U) * len));
			PLATFORM_MEMCPY(mids_in_flight[i].streams[fragment_id], p, len);

			mids_in_flight[i].stream_lens[fragment_id] = len;

			mids_in_flight[i].age = current_age++;

			break;
		}
	}

	// If we get inside the next "if()", that means no previous entry matches
	// this 'mid' + 'src_addr' + 'dst_addr' tuple.
	// What we have to do then is to search for an empty slot and add this as
	// the first stream associated to this new tuple.
	//
	if (MAX_MIDS_IN_FLIGHT == i) {
		for (i = 0; i < MAX_MIDS_IN_FLIGHT; i++) {
			if (0 == mids_in_flight[i].in_use) {
				break;
			}
		}

		if (MAX_MIDS_IN_FLIGHT == i) {
			// All slots are in use!!
			//
			// We need to discard the oldest one (ie. the one with the lowest
			// 'age')
			//
			INT32U lowest_age;

			lowest_age = mids_in_flight[0].age;
			j          = 0;

			for (i = 1; i < MAX_MIDS_IN_FLIGHT; i++) {
				if (mids_in_flight[i].age < lowest_age) {
					lowest_age = mids_in_flight[i].age;
					j          = i;
				}
			}

			PLATFORM_PRINTF_WARNING("Discarding old CMDU fragments to make room for the just received one. CMDU being discarded:\n");
			PLATFORM_PRINTF_WARNING("  mid      = %d\n", mids_in_flight[j].mid);
			PLATFORM_PRINTF_WARNING("  mids_in_flight[j].src_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", mids_in_flight[j].src_addr[0], mids_in_flight[j].src_addr[1], mids_in_flight[j].src_addr[2], mids_in_flight[j].src_addr[3], mids_in_flight[j].src_addr[4], mids_in_flight[j].src_addr[5]);
			PLATFORM_PRINTF_WARNING("  mids_in_flight[j].dst_addr = %02x:%02x:%02x:%02x:%02x:%02x\n", mids_in_flight[j].dst_addr[0], mids_in_flight[j].dst_addr[1], mids_in_flight[j].dst_addr[2], mids_in_flight[j].dst_addr[3], mids_in_flight[j].dst_addr[4], mids_in_flight[j].dst_addr[5]);

			for (i = 0; i < MAX_FRAGMENTS_PER_MID; i++) {
				if (1 == mids_in_flight[j].fragments[i] && NULL != mids_in_flight[j].streams[i]) {
					PLATFORM_FREE(mids_in_flight[j].streams[i]);
					mids_in_flight[j].stream_lens[i] = 0;
				}
			}

			mids_in_flight[j].in_use = 0;

			i = j;
		}

		// Now that we have our empty slot, initialize it and fill it with the
		// just received stream:
		//
		mids_in_flight[i].in_use = 1;
		mids_in_flight[i].mid    = mid;

		PLATFORM_MEMCPY(mids_in_flight[i].src_addr, src_addr, 6);
		PLATFORM_MEMCPY(mids_in_flight[i].dst_addr, dst_addr, 6);

		for (j = 0; j < MAX_FRAGMENTS_PER_MID; j++) {
			mids_in_flight[i].fragments[j]   = 0;
			mids_in_flight[i].streams[j]     = NULL;
			mids_in_flight[i].stream_lens[j] = 0;
		}
		mids_in_flight[i].streams[MAX_FRAGMENTS_PER_MID] = NULL;

		mids_in_flight[i].fragments[fragment_id] = 1;
		mids_in_flight[i].streams[fragment_id]   = (INT8U *)PLATFORM_MALLOC((sizeof(INT8U) * len));
		PLATFORM_MEMCPY(mids_in_flight[i].streams[fragment_id], p, len);

		mids_in_flight[i].stream_lens[fragment_id] = len;

		if (1 == last_fragment_indicator) {
			mids_in_flight[i].last_fragment = fragment_id;
		} else {
			mids_in_flight[i].last_fragment = MAX_FRAGMENTS_PER_MID;
			// NOTE: This means "no 'last_fragment_indicator' flag has been
			//       received yet.
		}

		mids_in_flight[i].age = current_age++;
	}

	// At this point we have an entry in the 'mids_in_flight' array (entry 'i')
	// where a new stream/fragment has been added.
	//
	// We now have to check if we have received all fragments for this 'mid'
	// and, if so, process them and obtain a CMDU structure that will be
	// returned to the caller of the function.
	//
	// Otherwise, return NULL.
	//
	if (MAX_FRAGMENTS_PER_MID != mids_in_flight[i].last_fragment) {
		struct CMDU *c;

		for (j = 0; j <= mids_in_flight[i].last_fragment; j++) {
			if (0 == mids_in_flight[i].fragments[j]) {
				PLATFORM_PRINTF_DETAIL("We still have to wait for more fragments to complete the CMDU message\n");
				return NULL;
			}
		}

		c = parse_1905_CMDU_from_packets(mids_in_flight[i].streams, mids_in_flight[i].stream_lens);

		if (NULL == c) {
			PLATFORM_PRINTF_WARNING("parse_1905_CMDU_header_from_packet() failed\n");
		} else {
			PLATFORM_PRINTF_DETAIL("All fragments belonging to this CMDU have already been received and the CMDU structure is ready\n");
		}

		for (j = 0; j <= mids_in_flight[i].last_fragment; j++) {
			PLATFORM_FREE(mids_in_flight[i].streams[j]);
		}
		mids_in_flight[i].in_use = 0;

		return c;
	}

	PLATFORM_PRINTF_DETAIL("The last fragment has not yet been received\n");
	return NULL;
}

// Returns '1' if the packet has already been processed in the past and thus,
// should be discarded (to avoid network storms). '0' otherwise.
//
// According to what is explained in "Sections 7.5, 7.6 and 7.7" if a
// defragmented packet whose "AL MAC address TLV" and "message id" match one
// that has already been received in the past, then it should be discarded.
//
// I *personally* think the standard is "slightly" wrong here because *not* all
// CMDUs contain an "AL MAC address TLV".
// We could use the ethernet source address instead, however this would only
// work for those messages that are *not* relayed (one same duplicated relayed
// message can arrive at our local node with two different ethernet source
// addresses).
// Fortunately for us, all relayed CMDUs *do* contain an "AL MAC address TLV",
// thus this is what we are going to do:
//
//   1. If the CMDU is a relayed one, check against the "AL MAC" contained in
//      the "AL MAC address TLV"
//
//   2. If the CMDU is *not* a relayed one, check against the ethernet source
//      address
//
// This function keeps track of the latest MAX_DUPLICATES_LOG_ENTRIES tuples
// of ("mac_address", "message_id") and:
//
//   1. If the provided tuple matches an already existing one, this function
//      returns '1'
//
//   2. Otherwise, the entry is added (discarding, if needed, the oldest entry)
//      and this function returns '0'
//
INT8U _checkDuplicates(INT8U *src_mac_address, struct CMDU *c)
{
#define MAX_DUPLICATES_LOG_ENTRIES 10

	static INT8U  mac_addresses[MAX_DUPLICATES_LOG_ENTRIES][6];
	static INT16U message_ids[MAX_DUPLICATES_LOG_ENTRIES];

	static INT8U start = 0;
	static INT8U total = 0;

	INT8U mac_address[6];

	INT8U i;

	if (
	    CMDU_TYPE_TOPOLOGY_RESPONSE == c->message_type || CMDU_TYPE_LINK_METRIC_RESPONSE == c->message_type || CMDU_TYPE_AP_AUTOCONFIGURATION_RESPONSE == c->message_type) {
		// This is a "hack" until a better way to handle MIDs is found.
		//
		// Let me explain.
		//
		// According to the standard, each AL entity generates its monotonically
		// increasing MIDs every time a new packet is sent.
		// The only exception to this rule is when generating a "response". In
		// these cases the same MID contained in the original query must be
		// used.
		//
		// Imagine we have two ALs that are started in different moments:
		//
		//        AL 1               AL 2
		//        ====               ====
		//   t=0  --- MID=1 -->
		//   t=1  --- MID=2 -->
		//   t=2  --- MID=3 -->      <-- MID=1 --
		//   t=3  --- MID=4 -->      <-- MID=2 --
		//   t=4  --- MID=5 -->      <-- MID=3 --
		//
		// In "t=2", "AL 2" learns that, in the future, messages from "AL 1" with
		// a "MID=3" should be discarded.
		//
		// Now, imagine in "t=4" the message "AL 2" sends (with "MID=3") is a
		// query that triggers a response from "AL 1" (which *must* have the
		// same MID, ie., "MID=3").
		//
		// HOWEVER, because of what "AL 2" learnt in "t=2", this response will
		// be discarded!
		//
		// In oder words... until the standard clarifies how MIDs should be
		// generated to avoid this problem, we will just accept (and process)
		// all response messages... even if they are duplicates.
		//
		return 0;
	}

	if (CMDU_TYPE_TOPOLOGY_NOTIFICATION == c->message_type && 0 == c->relay_indicator) {
		return 0;
	}

	// For relayed CMDUs, use the AL MAC, otherwise use the ethernet src MAC.
	//
	PLATFORM_MEMCPY(mac_address, src_mac_address, 6);
	if (1 == c->relay_indicator) {
		INT8U  i;
		INT8U *p;

		i = 0;
		while (NULL != (p = c->list_of_TLVs[i])) {
			if (TLV_TYPE_AL_MAC_ADDRESS_TYPE == *p) {
				struct alMacAddressTypeTLV *t = (struct alMacAddressTypeTLV *)p;

				PLATFORM_MEMCPY(mac_address, t->al_mac_address, 6);
				break;
			}
			i++;
		}
	}

	// Also, discard relayed CMDUs whose AL MAC is our own (that means someone
	// is retrasnmitting us back a message we originally created)
	//
	if (1 == c->relay_indicator) {
		if (0 == PLATFORM_MEMCMP(mac_address, DMalMacGet(), 6)) {
			return 1;
		}
	}

	// Find if the ("mac_address", "message_id") tuple is already present in the
	// database
	//
	for (i = 0; i < total; i++) {
		INT8U index;

		index = (start + i) % MAX_DUPLICATES_LOG_ENTRIES;

		if (
		    0 == PLATFORM_MEMCMP(mac_addresses[index], mac_address, 6) && message_ids[index] == c->message_id) {
			// The entry already exists!
			//
			return 1;
		}
	}

	// This is a new entry, insert it into the cache and return "0"
	//
	if (total < MAX_DUPLICATES_LOG_ENTRIES) {
		// There is space for new entries
		//
		INT8U index;

		index = (start + total) % MAX_DUPLICATES_LOG_ENTRIES;

		PLATFORM_MEMCPY(mac_addresses[index], mac_address, 6);
		message_ids[index] = c->message_id;

		total++;
	} else {
		// We need to replace the oldest entry
		//
		PLATFORM_MEMCPY(mac_addresses[start], mac_address, 6);
		message_ids[start] = c->message_id;

		start++;

		start = start % MAX_DUPLICATES_LOG_ENTRIES;
	}

	return 0;
}

// According to "Section 7.6", if a received packet has the "relayed multicast"
// bit set, after processing, we must forward it on all authenticated 1905
// interfaces (except on the one where it was received).
//
// This function checks if the provided 'c' structure has that "relayed
// multicast" flag set and, if so, retransmits it on all local interfaces
// (except for the one whose MAC address matches 'receiving_interface_addr') to
// 'destination_mac_addr' and the same "message id" (MID) as the one contained
// in the originally received 'c' structure.
//
void _checkForwarding(char *receiving_interface_name, INT8U *destination_mac_addr, struct CMDU *c)
{
	INT8U i;

	// No forwarding from loopback interface. It shall already sent through other interfaces if forwarding is needed
	if (0 == PLATFORM_STRCMP("lo", receiving_interface_name)) {
		return;
	}

	if (c->relay_indicator) {
		char **ifs_names;
		INT8U  ifs_nr;

		char *aux;

		PLATFORM_PRINTF_INFO("Relay Multicast. Forwarding...\n");

		ifs_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&ifs_nr);
		for (i = 0; i < ifs_nr; i++) {
			INT8U authenticated;
			INT8U power_state;

			struct interfaceInfo *x;

			struct LocalInterface *interface;
			interface = DMnameToLocalInterfaceStruct(ifs_names[i]);

			if (NULL == interface) {
				PLATFORM_PRINTF_ERROR("Could not retrieve info of interface %s\n", ifs_names[i]);
				continue;
			}

			if (0 == (interface->role & BACKHAUL_BSS) && 0 == (interface->role & BACKHAUL_STA)) {
				PLATFORM_PRINTF_DETAIL("%s is not a backhaul interface, skip.\n", ifs_names[i]);
				continue;
			}

			x = PLATFORM_GET_1905_INTERFACE_INFO(ifs_names[i]);
			if (NULL == x) {
				PLATFORM_PRINTF_WARNING("Could not retrieve info of interface %s\n", ifs_names[i]);
				authenticated = 0;
				power_state   = INTERFACE_POWER_STATE_OFF;
			} else {
				authenticated = x->is_secured;
				power_state   = x->power_state;
			}

			if (
			    (0 == authenticated) || ((power_state != INTERFACE_POWER_STATE_ON) && (power_state != INTERFACE_POWER_STATE_SAVE)) || (0 == strcmp(ifs_names[i], receiving_interface_name))) {
				// Do not forward the message on this interface
				//
				if (NULL != x) {
					PLATFORM_FREE_1905_INTERFACE_INFO(x);
				}
				continue;
			}

			if (NULL != x) {
				PLATFORM_FREE_1905_INTERFACE_INFO(x);
			}

			// Retransmit message
			//
			switch (c->message_type) {
			case CMDU_TYPE_TOPOLOGY_DISCOVERY: {
				aux = "CMDU_TYPE_TOPOLOGY_DISCOVERY";
				break;
			}
			case CMDU_TYPE_TOPOLOGY_NOTIFICATION: {
				aux = "CMDU_TYPE_TOPOLOGY_NOTIFICATION";
				break;
			}
			case CMDU_TYPE_TOPOLOGY_QUERY: {
				aux = "CMDU_TYPE_TOPOLOGY_QUERY";
				break;
			}
			case CMDU_TYPE_TOPOLOGY_RESPONSE: {
				aux = "CMDU_TYPE_TOPOLOGY_RESPONSE";
				break;
			}
			case CMDU_TYPE_VENDOR_SPECIFIC: {
				aux = "CMDU_TYPE_VENDOR_SPECIFIC";
				break;
			}
			case CMDU_TYPE_LINK_METRIC_QUERY: {
				aux = "CMDU_TYPE_LINK_METRIC_QUERY";
				break;
			}
			case CMDU_TYPE_LINK_METRIC_RESPONSE: {
				aux = "CMDU_TYPE_LINK_METRIC_RESPONSE";
				break;
			}
			case CMDU_TYPE_AP_AUTOCONFIGURATION_SEARCH: {
				aux = "CMDU_TYPE_AP_AUTOCONFIGURATION_SEARCH";
				break;
			}
			case CMDU_TYPE_AP_AUTOCONFIGURATION_RESPONSE: {
				aux = "CMDU_TYPE_AP_AUTOCONFIGURATION_RESPONSE";
				break;
			}
			case CMDU_TYPE_AP_AUTOCONFIGURATION_WSC: {
				aux = "CMDU_TYPE_AP_AUTOCONFIGURATION_WSC";
				break;
			}
			case CMDU_TYPE_AP_AUTOCONFIGURATION_RENEW: {
				aux = "CMDU_TYPE_AP_AUTOCONFIGURATION_RENEW";
				break;
			}
			case CMDU_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION: {
				aux = "CMDU_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION";
				break;
			}
			case CMDU_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION: {
				aux = "CMDU_TYPE_PUSH_BUTTON_JOIN_NOTIFICATION";
				break;
			}
			default: {
				aux = "UNKNOWN";
				break;
			}
			}
			PLATFORM_PRINTF_INFO("--> %s (forwarding from %s to %s)\n", aux, receiving_interface_name, ifs_names[i]);

			if (0 == send1905RawPacket(ifs_names[i], c->message_id, destination_mac_addr, c, false, 1)) {
				PLATFORM_PRINTF_WARNING("Could not retransmit 1905 message on interface %s\n", ifs_names[i]);
			}

		}
		PLATFORM_FREE_LIST_OF_1905_INTERFACES(ifs_names, ifs_nr);
	}

	return;
}

// This function sends an "AP-autoconfig search" message on all authenticated
// interfaces BUT ONLY if there is at least one unconfigured AP interface on
// this node.
//
// A function has been created for this because the same code is executed from
// three different places:
//
//   - When a new interface becomes authenticated
//
//   - When the *local* push button is pressed and there is at least one
//     interface which does not support this configuration mechanism (ex:
//     ethernet)
//
//   - After a local unconfigured AP interface becomes configured (this is
//     needed in the unlikely situation where there are more than one
//     unconfigured APs in the same node)
//
void _triggerAPSearchProcess(INT8U flag)
{
	INT8U  configured_bands;
	INT8U  i;
	INT16U mid;

	char **ifs_names;
	INT8U  ifs_nr;

	INT8U ap_band = 0;

	if (DMisController()) {
		return;
	}

	if (DMisEventFlagSet(FLAG_DELAYED_AUTOCONFIG)) {
		return;
	}

	configured_bands = DMgetConfiguredState();

	if ((0 == flag && 0x00 != configured_bands) || (1 == flag && DEVICE_FULLY_CONFIGURED == configured_bands)) {
		return;
	}

	PLATFORM_PRINTF_DEBUG("Configured Band %02x\n", configured_bands);

	mid = getNextMid();

	ifs_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&ifs_nr);

	INT8U configured_band = DMgetConfiguredState();

	configured_band = (configured_band - (configured_band / 16)) & 0x0F;

	PLATFORM_PRINTF_DEBUG("New configured_band %02x\n", configured_band);
	DMsetConfiguredState(configured_band);

	for (i = 0; i < ifs_nr; i++) {
		struct LocalInterface *interface;
		interface = DMnameToLocalInterfaceStruct(ifs_names[i]);

		if (NULL == interface) {
			PLATFORM_PRINTF_ERROR("Could not retrieve info of interface %s\n", ifs_names[i]);
			continue;
		}

		// CMDU_TYPE_AP_AUTOCONFIGURATION_SEARCH should only be sent to controller, hence always send through backhaullink
		if (0 == (interface->role & BACKHAUL_BSS) && 0 == (interface->role & BACKHAUL_STA)) {
			PLATFORM_PRINTF_INFO("%s is not a backhaul interface, skip.\n", ifs_names[i]);
			continue;
		}

		INT8U authenticated;
		INT8U power_state;

		struct interfaceInfo *x;

		x = PLATFORM_GET_1905_INTERFACE_INFO(ifs_names[i]);
		if (NULL == x) {
			PLATFORM_PRINTF_WARNING("Could not retrieve info of interface %s\n", ifs_names[i]);
			continue;
		} else {
			authenticated = x->is_secured;
			power_state   = x->power_state;
		}

		if (
		    (0 == authenticated) || ((power_state != INTERFACE_POWER_STATE_ON) && (power_state != INTERFACE_POWER_STATE_SAVE))) {
			// Do not send the message on this interface
			//
			if (NULL != x) {
				PLATFORM_FREE_1905_INTERFACE_INFO(x);
			}
			continue;
		}

		if (IS_IEEE_802_11_2_4_GHZ(x->interface_type, x->interface_type_data.ieee80211.rf_band)) {
			ap_band = IEEE80211_FREQUENCY_BAND_2_4_GHZ;
		} else if (IS_IEEE_802_11_5_GHZ(x->interface_type, x->interface_type_data.ieee80211.rf_band)) {
			ap_band = IEEE80211_FREQUENCY_BAND_5_GHZ;
		} else if (IS_IEEE_802_11_60_GHZ(x->interface_type)) {
			ap_band = IEEE80211_FREQUENCY_BAND_60_GHZ;
		} else if (IS_IEEE_802_3_INTERFACE(x->interface_type)) {
			ap_band = IEEE80211_FREQUENCY_BAND_5_GHZ; // No suitable value
		} else {
			PLATFORM_PRINTF_WARNING("Unknown interface type %02x\n", x->interface_type);
			PLATFORM_FREE_1905_INTERFACE_INFO(x);
			continue;
		}

		PLATFORM_FREE_1905_INTERFACE_INFO(x);

		if (0 == send1905APAutoconfigurationSearchPacket(ifs_names[i], mid, ap_band)) {
			PLATFORM_PRINTF_WARNING("Could not send 1905 AP-autoconfiguration search message\n");
		}
	}

	PLATFORM_FREE_LIST_OF_1905_INTERFACES(ifs_names, ifs_nr);
}

#ifdef RTK_MULTI_AP_R3
INT8U _verifyMICTLV(struct CMDU *cmdu)
{
	struct networkDevice *network_device = NULL;

	INT8U **list_of_TLVs = cmdu->list_of_TLVs;
	INT16U  tlvs_count   = 0;

	INT8U **list_of_TLVs_xMIC = NULL;
	INT16U  tlvs_xMIC_count   = 0;

	struct MICTLV *mic_tlv = NULL;

	INT64U received_itc;
	INT8U  mic_expected[SHA512_MAC_LEN] = { 0 };

	struct dpp_gtk *gtk = dpp_keystore_get_gtk();

	INT8U *p;
	INT8U  ret = 1;

	if (!gtk) {
		PLATFORM_PRINTF_WARNING("No GTK is installed. Skip checking MIC\n");
		return 0;
	}

	// Create a new list of TLVs without MIC TLV
	//
	list_of_TLVs_xMIC    = PLATFORM_MALLOC(sizeof(INT8U *) * 1);
	list_of_TLVs_xMIC[0] = NULL;
	while (NULL != (p = list_of_TLVs[tlvs_count++])) {
		if (TLV_TYPE_MIC != *p) {
			list_of_TLVs_xMIC                      = PLATFORM_REALLOC(list_of_TLVs_xMIC, sizeof(INT8U *) * (tlvs_xMIC_count + 2));
			list_of_TLVs_xMIC[tlvs_xMIC_count]     = p;
			list_of_TLVs_xMIC[tlvs_xMIC_count + 1] = NULL;
			tlvs_xMIC_count++;
		} else {
			mic_tlv = (struct MICTLV *)p;
		}
	}

	if (!mic_tlv) {
		PLATFORM_PRINTF_DEBUG("The CMDU has no MIC TLV, skip MIC verify.\n");
		ret = 0;
		goto fail;
	}

	if (NULL == (network_device = DMnetworkDeviceGet(mic_tlv->src_1905_al_mac_addr))) {
		PLATFORM_PRINTF_DEBUG("No network device info, skip MIC verify\n");
		ret = 0;
		goto fail;
	}

	// Check Integrity Transmission Counter
	//
	received_itc = mic_tlv->integrity_transmission_counter;
	if (received_itc <= network_device->integrity_transmission_counter) {
		PLATFORM_PRINTF_ERROR("Received ITC is lower than expected!\n");
		goto fail;
	}

	// Calculate MIC without the MIC TLV
	//
	PLATFORM_MEMCPY(mic_expected, mic_tlv->mic, mic_tlv->mic_length);
	cmdu->list_of_TLVs = list_of_TLVs_xMIC;
	if (calculate_mic(gtk->gtk, cmdu, cmdu->relay_indicator, mic_tlv)) {
		PLATFORM_PRINTF_WARNING("GTK calculation failed\n");
		goto fail;
	}

	// Check expected MIC value
	//
	if (PLATFORM_MEMCMP(mic_expected, mic_tlv->mic, mic_tlv->mic_length) != 0) {
		PLATFORM_PRINTF_ERROR("Mismatch MIC: Packet is modified or GTK is expired\n");
		goto fail;
	}

	ret = 0;
	network_device->integrity_transmission_counter = received_itc;
	PLATFORM_PRINTF_DEBUG("MIC Verified OK!\n");

fail:
	cmdu->list_of_TLVs = list_of_TLVs;
	PLATFORM_FREE(list_of_TLVs_xMIC);
	return ret;
}

void _free_list_of_TLVs(INT8U **list_of_TLVs)
{
	// Free a malloc'd list of 1905 TLVs
	INT8U i;
	for (i = 0; list_of_TLVs[i] != NULL; i++) {
		free_1905_TLV_structure(list_of_TLVs[i]);
	}
	free(list_of_TLVs);
}

void decryptTLV(struct CMDU *c, char *receiving_interface, INT8U *src_addr)
{
	// Decrypt Encrypted Payload TLVs
	INT8U *p;
	INT8U  i = 0;

	while (NULL != (p = c->list_of_TLVs[i])) {
		switch (*p) {
		case TLV_TYPE_ENCRYPTED_PAYLOAD: {
			PLATFORM_PRINTF_DEBUG("[DECRYPT] Starting 1905 Layer Decryption...\n");
			struct EncryptedPayloadTLV *encrypted_payload_tlv = (struct EncryptedPayloadTLV *)p;

			struct dpp_ptk *ptk = dpp_keystore_get_ptk(src_addr);
			if (ptk == NULL) {
				PLATFORM_PRINTF_ERROR("Unable to decrypt: no PTK found!\n");
				break;
			}

			INT16U ciphertext_size = encrypted_payload_tlv->aes_siv_len;
			INT16U cleartext_size  = ciphertext_size - AES_BLOCK_SIZE;
			INT8U *ciphertext      = encrypted_payload_tlv->aes_siv;

			INT8U cleartext[cleartext_size];
			PLATFORM_MEMSET(cleartext, 0, cleartext_size);

			// There are 4 associated data used:
			// AD1 = The first six octets of the received 1905 CMDU.
			// AD2 = The Encryption Transmission Counter value in the Encrypted Payload TLV.
			// AD3 = Source 1905 AL MAC Address value in the Encrypted Payload TLV.
			// AD4 = Destination 1905 AL MAC Address value in the Encrypted Payload TLV.
			INT8U assoc_data1[6], assoc_data2[6], assoc_data3[6], assoc_data4[6];

			INT8U *p1 = assoc_data1;
			INT8U aux8;
			_I1B(&c->message_version, &p1);
			aux8 = 0; // Reserved Field 0x00
			_I1B(&aux8, &p1);
			_I2B(&c->message_type, &p1);
			_I2B(&c->message_id, &p1);

			INT8U *p2 = assoc_data2;
			_INT64_I6B(&encrypted_payload_tlv->encryption_transmission_counter, &p2);

			PLATFORM_MEMCPY(assoc_data3, encrypted_payload_tlv->source_1905_mac_address, 6);
			PLATFORM_MEMCPY(assoc_data4, encrypted_payload_tlv->dest_1905_mac_address, 6);

			INT64U reception_counter = dpp_counters_get_erc(src_addr);

			// The Multi-AP device shall ignore and discard the entire message:
			// 1. If ETC value received in the Encrypted Payload TLV <= ERC
			// 2. If Destination AL MAC Address value received in the Encrypted Payload TLV != AL MAC Address of the receiver
			// 3. If Multi-AP device does not have the corresponding 1905 TK for the sender
			if (encrypted_payload_tlv->encryption_transmission_counter == 0 && reception_counter == 0) {
				// Allow ETC < ERC to solve compatibility issue with QCA
				// After EAPOL M4, QCA replies BSS Config Request (ETC = 0) with BSS Config Response (ETC = 0)
				// This is not correct based on the spec as it should use BSS Config Response (ETC = 1)
			} else if (encrypted_payload_tlv->encryption_transmission_counter <= reception_counter
			           || (0 != PLATFORM_MEMCMP(encrypted_payload_tlv->dest_1905_mac_address, DMalMacGet(), 6))
			           || !ptk) {
				break;
			}

			const INT32U assoc_data_len[] = { 0x06, 0x06, 0x06, 0x06 };
			const INT8U *assoc_data_arr[] = { assoc_data1, assoc_data2, assoc_data3, assoc_data4 };

			if (ciphertext_size <= AES_BLOCK_SIZE) {
				break;
			}

			// Uncomment below for debugging purposes
			// PLATFORM_PRINTF_INFO("message type 		= %02x%02x\n", assoc_data1[2], assoc_data1[3]);
			// PLATFORM_PRINTF_INFO("message id 		= %02x%02x\n", assoc_data1[4], assoc_data1[5]);
			// PLATFORM_PRINTF_INFO("etc 				= %02x\n", assoc_data2[5]);
			// PLATFORM_PRINTF_INFO("src		 		= %02x:%02x:%02x:%02x:%02x:%02x\n", assoc_data3[0], assoc_data3[1], assoc_data3[2], assoc_data3[3], assoc_data3[4], assoc_data3[5]);
			// PLATFORM_PRINTF_INFO("dst		 		= %02x:%02x:%02x:%02x:%02x:%02x\n", assoc_data4[0], assoc_data4[1], assoc_data4[2], assoc_data4[3], assoc_data4[4], assoc_data4[5]);

			if (0 == PLATFORM_AES_SIV_DECRYPT(ptk->tk, ptk->tk_len, ciphertext, ciphertext_size, ARRAY_SIZE(assoc_data_arr), assoc_data_arr, assoc_data_len, cleartext)) {
				PLATFORM_PRINTF_WARNING("[DECRYPT] AES-SIV Decryption error\n");
				INT16U df_counter = dpp_counters_get_dfc(src_addr);
				df_counter--;

				// If the Decryption Failure Counter exceeds the Decryption Failure Counter threshold
				if (df_counter == 0) {
					if (DMisController()) {
						PLATFORM_PRINTF_WARNING("DFC of agent in controller exceeds threshold, resetting DFC...\n");
						dpp_counters_set_dfc(src_addr, RTK_DEFAULT_DECRYPT_FAILURE_THRESHOLD);
					} else {
						struct dpp_authentication *dpp_auth = DMgetDPPAuthenticationObject();
						if (!dpp_auth) {
							PLATFORM_PRINTF_ERROR("Authentication object not found!\n");
							break;
						}

						struct dpp_config_obj *conf_obj = dpp_find_config_obj(dpp_auth, "map", "mapNW", DPP_NETROLE_MAP_AGENT);
						if (!conf_obj) {
							PLATFORM_PRINTF_ERROR("Configuration object not found!\n");
							break;
						}

						if (0 == PLATFORM_MEMCMP(src_addr, DMcontrollerMacGet(), 6)) {
							// AGT <- CTRL exceed threshold, re-establish PMK using DPP Discovery
							DMsetConfiguredState(0x00);
							_triggerAPSearchProcess(0);
							dpp_auth_clear_state(dpp_auth);
						} else {
							// AGT <- AGT exceed threshold, inform controller
							if (0 == send1905DecryptionFailureMessage(receiving_interface, c->message_id, src_addr)) {
								PLATFORM_PRINTF_WARNING("Could not send 1905 decryption failure message\n");
							} else {
								PLATFORM_PRINTF_INFO("DFC exceeds threshold, sent 1905 decryption failure message to controller\n");
							}
						}

						// Reset counter
						dpp_counters_set_dfc(src_addr, conf_obj->dfCounterThreshold);
					}
				} else {
					dpp_counters_set_dfc(src_addr, df_counter);
				}
			} else {
				PLATFORM_PRINTF_DEBUG("[DECRYPT] AES-SIV Decryption successful\n");
				INT16U tlvs_nr  = 0;
				// free the encrypted TLV and the TLV list before overwriting
				_free_list_of_TLVs(c->list_of_TLVs);
				c->list_of_TLVs = _parse_tlvs_from_packet(cleartext, cleartext_size, &tlvs_nr);
				// Multi-AP device shall set the Encryption Reception Counter corresponding to that Source 1905 AL MAC Address
				// to the received Encryption Transmission Counter value
				dpp_counters_set_erc(src_addr, encrypted_payload_tlv->encryption_transmission_counter);
			}
			break;
		}
		default: {
			break;
		}
		}
		i++;
	}
}
#endif // RTK_MULTI_AP_R3
////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////

INT8U start1905AL(INT8U al_mac_address[])
{
	INT8U  queue_id;
	INT8U *queue_message;

	char **interfaces_names;
	INT8U  interfaces_nr;

	INT8U i;
#ifdef RTK_MULTI_AP_R4
	INT8U j;
#endif

	INT8U search_counter = 0;

	INT8U alive_check;

#ifdef RTK_MULTI_AP_R2
	INT8U channel_scan_onbootscan_counter = 0;
	INT8U channel_scan_delay = 0, apply_vlan_delay = 0;
#endif

#ifdef RTK_MULTI_AP_R3
	INT8U eapol_delay = 0;
#endif

#ifdef RTK_MULTI_AP_R4
	INT8U reconfig_delay = 0;
#endif

	INT32U apply_prefer_bssid_counter = 0, prefer_bssid_interval = 0;

	if (NULL == al_mac_address) {
		// Invalid arguments
		//
		PLATFORM_PRINTF_ERROR("NULL AL MAC address not allowed\n");
		return AL_ERROR_INVALID_ARGUMENTS;
	}

	// Insert the provided AL MAC address into the database
	//
	// DMinit();

	DMsetMultiAPProfile(config.multiap_profile);

	DMsetConfiguredState(config.configure_state);

	DMalMacSet(al_mac_address);

	DMsetDeviceName(config.device_name);

	DMsetNeighborRemovalThreshold(config.neighbor_removal_threshold);

	DMmapWholeNetworkSet(true);

	DMsetListeningLoopback(config.self_listening);

	DMsetWFATestMode(config.wfa_test_mode);

	DMupdateNetworkDeviceVersion(DMalMacGet(), MAP_VERSION);

	if (config.is_controller) {
		DMcontrollerInit(al_mac_address);
	}

	DMsetMultiAPCommonNetlink(config.common_netlink);

	DMsetMaxBssPerRadio(config.max_bss_number);

	DMdppInit();

	// Initialize 1905 GTK
	if (config.is_controller && !dpp_keystore_get_gtk()) {
		struct dpp_gtk gtk;
		dpp_eapol_gtk_init(&gtk, SHA256_MAC_LEN);
		dpp_keystore_set_gtk(&gtk);
	}

	// Obtain the list of interfaces that the AL entity is going to manage
	//
	PLATFORM_PRINTF_DETAIL("Retrieving list of interfaces visible to the 1905 AL entity...\n");
	interfaces_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&interfaces_nr);
	if (NULL == interfaces_names) {
		PLATFORM_PRINTF_ERROR("No interfaces detected\n");
		return AL_ERROR_NO_INTERFACES;
	}

	for (i = 0; i < interfaces_nr; i++) {
		struct interfaceInfo *x;

		x = PLATFORM_GET_1905_INTERFACE_INFO(interfaces_names[i]);
		if (NULL == x) {
			PLATFORM_PRINTF_ERROR("Could not retrieve interface info for %s\n", interfaces_names[i]);
			continue;
		}

		INT8U role = 0;

		role = PLATFORM_GET_BSS_TYPE(x->name);

		DMinsertInterface(x->name, x->mac_address, role);

		PLATFORM_PRINTF_DEBUG("    - %s --> %02x:%02x:%02x:%02x:%02x:%02x %d\n",
		                      x->name,
		                      x->mac_address[0],
		                      x->mac_address[1],
		                      x->mac_address[2],
		                      x->mac_address[3],
		                      x->mac_address[4],
		                      x->mac_address[5],
		                      role);

		PLATFORM_FREE_1905_INTERFACE_INFO(x);
	}
#ifdef RTK_MULTI_AP_R2
	DMsetDefault8021QSettings(config.primary_vid, config.default_pcp);
#endif
	// Create a queue that will later be used by the platform code to notify us
	// when certain types of "events" take place
	//
	PLATFORM_PRINTF_DETAIL("Creating events queue...\n");
	queue_id = PLATFORM_CREATE_QUEUE("/MAP_AL_events");
	if (0 == queue_id) {
		PLATFORM_PRINTF_ERROR("Could not create events queue\n");
		return AL_ERROR_OS;
	}

	// We are interested in processing 1905 packets that arrive on any of the
	// 1905 interfaces.
	// For this we are going to tell the platform code that we want to receive
	// a message on the just created queue every time a new 1905 packet arrives
	// on any of those interfaces.
	//
	PLATFORM_PRINTF_DETAIL("Registering packet arrival event for each interface...\n");
	for (i = 0; i < interfaces_nr; i++) {
		struct event1905Packet aux;
		INT8U                  role = PLATFORM_GET_BSS_TYPE(interfaces_names[i]);
		if ((0 == (BACKHAUL_BSS & role)) && (0 == (BACKHAUL_STA & role))) {
			continue;
		}

		//if (0 == PLATFORM_IS_INTERFACE_UP(interfaces_names[i]) && (0 == (BACKHAUL_STA & role))) {
		//	continue;
		//}

		if(config.primary_vid && PLATFORM_STRSTR(interfaces_names[i], "wlan")) {
			char vlan_interface[64];
			sprintf(vlan_interface, "%s.%d", interfaces_names[i], config.primary_vid);
			if (PLATFORM_IS_INTERFACE_UP(vlan_interface)) {
				aux.interface_name = vlan_interface;
				PLATFORM_MEMCPY(aux.interface_mac_address, DMinterfaceNameToMac(aux.interface_name), 6);
				PLATFORM_MEMCPY(aux.al_mac_address, DMalMacGet(), 6);
				if (0 == PLATFORM_REGISTER_QUEUE_EVENT(queue_id, PLATFORM_QUEUE_EVENT_NEW_1905_PACKET, &aux)) {
					PLATFORM_PRINTF_ERROR("Could not register callback for 1905 packets in interface %s\n", vlan_interface);
					return AL_ERROR_OS;
				}
				PLATFORM_PRINTF_DEBUG("    - %s --> OK\n", vlan_interface);
			}
		}

		aux.interface_name = interfaces_names[i];
		PLATFORM_MEMCPY(aux.interface_mac_address, DMinterfaceNameToMac(aux.interface_name), 6);
		PLATFORM_MEMCPY(aux.al_mac_address, DMalMacGet(), 6);

		if (0 == PLATFORM_REGISTER_QUEUE_EVENT(queue_id, PLATFORM_QUEUE_EVENT_NEW_1905_PACKET, &aux)) {
			PLATFORM_PRINTF_ERROR("Could not register callback for 1905 packets in interface %s\n", interfaces_names[i]);
			return AL_ERROR_OS;
		}
		PLATFORM_PRINTF_DEBUG("    - %s --> OK\n", interfaces_names[i]);

	}
	PLATFORM_FREE_LIST_OF_1905_INTERFACES(interfaces_names, interfaces_nr);

	if (DMisController() || DMisListeningLoopback()) {
		struct event1905Packet aux;

		aux.interface_name = "lo";
		PLATFORM_MEMCPY(aux.interface_mac_address, DMalMacGet(), 6);
		PLATFORM_MEMCPY(aux.al_mac_address, DMalMacGet(), 6);
		// PLATFORM_PRINTF_INFO("Listening to lo interface\n");

		if (0 == PLATFORM_REGISTER_QUEUE_EVENT(queue_id, PLATFORM_QUEUE_EVENT_NEW_1905_PACKET, &aux)) {
			PLATFORM_PRINTF_ERROR("Could not register callback for 1905 packets in interface lo\n");
			return AL_ERROR_OS;
		}
	}

	// We are also interested in processing a 60 seconds timeout event (so that
	// we can send new discovery messages into the network)
	//
	PLATFORM_PRINTF_DETAIL("Registering DISCOVERY time out event (periodic)...\n");
	{
		struct eventTimeOut aux;

		aux.timeout_ms = 60000; // 60 seconds
		aux.token      = TIMER_TOKEN_DISCOVERY;

		if (0 == PLATFORM_REGISTER_QUEUE_EVENT(queue_id, PLATFORM_QUEUE_EVENT_TIMEOUT_PERIODIC, &aux)) {
			PLATFORM_PRINTF_ERROR("Could not register timer callback\n");
			return AL_ERROR_OS;
		}
	}

	// ...and a slighlty higher timeout to "clean" the database from nodes that
	// have left the network without notice
	//
	PLATFORM_PRINTF_DETAIL("Registering GARBAGE COLLECTOR time out event (periodic)...\n");
	{
		struct eventTimeOut aux;

		aux.timeout_ms = 70000; // 70 seconds
		aux.token      = TIMER_TOKEN_GARBAGE_COLLECTOR;

		if (0 == PLATFORM_REGISTER_QUEUE_EVENT(queue_id, PLATFORM_QUEUE_EVENT_TIMEOUT_PERIODIC, &aux)) {
			PLATFORM_PRINTF_ERROR("Could not register timer callback\n");
			return AL_ERROR_OS;
		}
	}

	// If this is a controller, then do a periodic 5 second action
	//
	// PLATFORM_PRINTF_DETAIL("Registering CONTROLLER ACTION time out event (periodic)...\n");
	// {
	// 	struct eventTimeOut aux;

	// 	aux.timeout_ms = 10000; // 10 seconds
	// 	aux.token      = TIMER_TOKEN_CONTROLLER_ACTION;

	// 	if (0 == PLATFORM_REGISTER_QUEUE_EVENT(queue_id, PLATFORM_QUEUE_EVENT_TIMEOUT_PERIODIC, &aux)) {
	// 		PLATFORM_PRINTF_ERROR("Could not register timer callback\n");
	// 		return AL_ERROR_OS;
	// 	}
	// }

	// If this is a agent, then do a periodic 1 second action
	//
	PLATFORM_PRINTF_DETAIL("Registering 1 SECOND time out event (periodic)...\n");
	{
		struct eventTimeOut aux;

		aux.timeout_ms = 1000; // 1 second
		aux.token      = TIMER_TOKEN_1_SECOND_INTERVAL;

		if (0 == PLATFORM_REGISTER_QUEUE_EVENT(queue_id, PLATFORM_QUEUE_EVENT_TIMEOUT_PERIODIC, &aux)) {
			PLATFORM_PRINTF_ERROR("Could not register timer callback\n");
			return AL_ERROR_OS;
		}
	}

	// As soon as we enter the queue message processing loop we want to start
	// the discovery process as if a "DISCOVERY timeout" event had just
	// happened.
	// In other words, we want the first "DISCOVERY timeout" event to take place
	// at t=0 and then every 60 seconds.
	// In order to "force" this first event at t=0 we use a new timer event,
	// but this time this is a one time (ie. non-periodic) timer which will
	// time out in just one second from now.
	//
	PLATFORM_PRINTF_DETAIL("Registering a one time forced DISCOVERY event...\n");
	{
		struct eventTimeOut aux;

		aux.timeout_ms = 1;
		aux.token      = TIMER_TOKEN_DISCOVERY;

		if (0 == PLATFORM_REGISTER_QUEUE_EVENT(queue_id, PLATFORM_QUEUE_EVENT_TIMEOUT, &aux)) {
			PLATFORM_PRINTF_ERROR("Could not register timer callback\n");
			return AL_ERROR_OS;
		}
	}

	// Do also register the ALME interface (ie. we want ALME REQUEST messages to
	// be inserted into the queue so that we can process them)
	//
	if (DEVICE_FULLY_CONFIGURED == DMgetConfiguredState() || DMisController()) {
		PLATFORM_PRINTF_DETAIL("Registering the ALME interface...\n");
		if (0 == PLATFORM_REGISTER_QUEUE_EVENT(queue_id, PLATFORM_QUEUE_EVENT_NEW_ALME_MESSAGE, NULL)) {
			PLATFORM_PRINTF_ERROR("Could not register ALME messages callback\n");
			return AL_ERROR_OS;
		}
	}

	// ...and the "push button" event, so that when the platform detects that
	// the user has pressed the button associated to the "push button"
	// configuration mechanism, we are notified.
	//
	PLATFORM_PRINTF_DETAIL("Registering the PUSH BUTTON event...\n");
	if (0 == PLATFORM_REGISTER_QUEUE_EVENT(queue_id, PLATFORM_QUEUE_EVENT_PUSH_BUTTON, NULL)) {
		PLATFORM_PRINTF_ERROR("Could not register 'push button' event\n");
		return AL_ERROR_OS;
	}

	// ...and the "new authenticated link" event, needed to produce the "push
	// button join notification" message.
	//
	PLATFORM_PRINTF_DETAIL("Registering the NEW AUTHENTICATED LINK event...\n");
	if (0 == PLATFORM_REGISTER_QUEUE_EVENT(queue_id, PLATFORM_QUEUE_EVENT_AUTHENTICATED_LINK, NULL)) {
		PLATFORM_PRINTF_ERROR("Could not register 'authenticated link' event\n");
		return AL_ERROR_OS;
	}

	// ...and the "topology change notification" event, needed to inform the
	// other 1905 nodes that some aspect of our local topology has changed.
	//
	PLATFORM_PRINTF_DETAIL("Registering the TOPOLOGY CHANGE NOTIFICATION event...\n");
	if (0 == PLATFORM_REGISTER_QUEUE_EVENT(queue_id, PLATFORM_QUEUE_EVENT_TOPOLOGY_CHANGE_NOTIFICATION, NULL)) {
		PLATFORM_PRINTF_ERROR("Could not register 'topology change' event\n");
		return AL_ERROR_OS;
	}

	// ...and the "hostapd notification" event, needed to inform the
	// other 1905 nodes that some aspect of our local hostapd events.
	//
	if (PLATFORM_NEED_HAPD()) {
		PLATFORM_PRINTF_DETAIL("Registering the HOSTAPD NOTIFICATION event...\n");
		if (0 == PLATFORM_REGISTER_QUEUE_EVENT(queue_id, PLATFORM_QUEUE_EVENT_HOSTAPD_NOTIFICATION, NULL)) {
			PLATFORM_PRINTF_ERROR("Could not register 'hostapd notification' event\n");
			return AL_ERROR_OS;
		}
	}

	// ...and the "DPP bootstrap info" event, so that when the platform detects
	// that a new peer bootstrap URI or a bootstrapping key is entered through
	// some special files, we are notified.
	//
	PLATFORM_PRINTF_DETAIL("Registering the DPP BOOTSTRAP INFO event...\n");
	if (0 == PLATFORM_REGISTER_QUEUE_EVENT(queue_id, PLATFORM_QUEUE_EVENT_DPP_BOOTSTRAP_INFO, NULL)) {
		PLATFORM_PRINTF_ERROR("Could not register 'dpp bootstrap info' event\n");
		return AL_ERROR_OS;
	}

	PLATFORM_PRINTF_DETAIL("Allocating memory to hold a queue message...\n");
	queue_message = (INT8U *)PLATFORM_MALLOC(MAP_MQ_MAX_SIZE);

	signal(SIGTERM, _kill_handler);
#ifdef RTK_MULTI_AP_R2
	if (MULTI_AP_PROFILE_2 <= DMgetMultiAPProfile()) {
		INT8U start_on_boot_channel_scan = 1;
		FILE *fp = fopen(ON_BOOT_CHANNEL_SCAN_RESULT_FILE, "re");
		if (fp != NULL) {
			start_on_boot_channel_scan                          = 0;
			INT8U                        tlv_type               = 0;
			INT16U                       tlv_length             = 0;
			INT8U *                      tlv_value              = NULL;
			INT8U                        channel_scan_result_nr = 0;
			INT8U                        error                  = 0;
			struct ChannelScanResultTLV *channel_scan_results   = NULL;
			while (1 == fread(&tlv_type, sizeof(INT8U), 1, fp)) {
				if (1 != fread(&tlv_length, sizeof(INT16U), 1, fp)) {
					PLATFORM_PRINTF_WARNING("Could not read the expected number of bytes from the file\n");
					error                      = 1;
					start_on_boot_channel_scan = 1;
					break;
				}
				tlv_value    = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * tlv_length + 3);
				tlv_value[0] = tlv_type;
				PLATFORM_MEMCPY(&tlv_value[1], &tlv_length, 2);
				if (tlv_length != fread(&tlv_value[3], sizeof(INT8U), tlv_length, fp)) {
					PLATFORM_FREE(tlv_value);
					PLATFORM_PRINTF_WARNING("Could not read the expected number of bytes from the file\n");
					error                      = 1;
					start_on_boot_channel_scan = 1;
					break;
				}
				if (TLV_TYPE_CHANNEL_SCAN_RESULT == tlv_type) {
					channel_scan_result_nr++;
					channel_scan_results                          = (struct ChannelScanResultTLV *)PLATFORM_REALLOC(channel_scan_results, (channel_scan_result_nr) * sizeof(struct ChannelScanResultTLV));
					struct ChannelScanResultTLV *channel_scan_tlv = (struct ChannelScanResultTLV *)parse_1905_TLV_from_packet(tlv_value);
					PLATFORM_MEMCPY(&(channel_scan_results[channel_scan_result_nr - 1]), channel_scan_tlv, sizeof(struct ChannelScanResultTLV));
					// visit_1905_TLV_structure((INT8U *)channel_scan_tlv, print_callback, PLATFORM_PRINTF_DEBUG, "Onboot Channel Scan");
					PLATFORM_FREE(channel_scan_tlv);
					visit_1905_TLV_structure((INT8U *)&(channel_scan_results[channel_scan_result_nr - 1]), print_callback, PLATFORM_PRINTF_DETAIL, "Onboot Channel Scan ");
				}
				PLATFORM_FREE(tlv_value);
			}
			if (0 == error){
				DMsetChannelScanResult(channel_scan_result_nr, channel_scan_results);
			} else {
				PLATFORM_FREE(channel_scan_results);
			}

			fclose(fp);
		}

		if (start_on_boot_channel_scan){
			if (DEVICE_FULLY_CONFIGURED == DMgetConfiguredState()) {
				DMsetEventFlag(FLAG_ON_BOOT_CHANNEL_SCAN);
			}
		}
	}
#endif

#ifdef RTK_MULTI_AP_R4
	// Controller send reconfiguration trigger message to connected APs
	// TODO: add condition for "want to reconfigure"
	if (DMgetWFATestMode() && DMisController()) {
		char **ifs_names;
		INT8U  ifs_nr;
		ifs_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&ifs_nr);

		INT8U  connected_mac[MAC_ADDR_LEN] = { 0 };
		INT16U mid;

		INT8U *al_mac_stream = NULL;
		INT8U  connected_nr;
		al_mac_stream = dpp_get_connected(&connected_nr);

		if (al_mac_stream != NULL) {
			for (i = 0; i < connected_nr; i++) {
				PLATFORM_MEMCPY(connected_mac, al_mac_stream + (MAC_ADDR_LEN * i), MAC_ADDR_LEN);
				PLATFORM_PRINTF_DEBUG("Connected mac addr #%d (%02X:%02X:%02X:%02X:%02X:%02X)\n", i + 1,
				                     connected_mac[0], connected_mac[1], connected_mac[2],
				                     connected_mac[3], connected_mac[4], connected_mac[5]);
				mid = getNextMid();
				for (j = 0; j < ifs_nr; j++) {
					if (PLATFORM_STRSTR(ifs_names[j], "eth"))
						sendReconfigurationTriggerMessage(ifs_names[j], mid, connected_mac, 1);
				}
			}
		}
		PLATFORM_FREE(al_mac_stream);
	}
#endif // RTK_MULTI_AP_R4

	PLATFORM_PRINTF_DETAIL("Entering read-process loop...\n");
	pthread_mutex_lock(&map_core_mutex);
	alive_check = core_thread_flag;
	pthread_mutex_unlock(&map_core_mutex);
	while (alive_check) {
		INT8U *p;
		INT8U  message_type;
		INT16U message_len;

		PLATFORM_PRINTF_TRACE("\n");
		PLATFORM_PRINTF_TRACE("Waiting for new queue message...\n");
		if (0 == PLATFORM_READ_QUEUE(queue_id, queue_message)) {
			PLATFORM_PRINTF_WARNING("Something went wrong while trying to retrieve a new message from the queue. Ignoring...\n");
			pthread_mutex_lock(&map_core_mutex);
			alive_check = core_thread_flag;
			pthread_mutex_unlock(&map_core_mutex);
			continue;
		}

		if(!DMgetWFATestMode()) {
			if(PLATFORM_GET_QUEUE_SIZE(queue_id) >= 80) {
				PLATFORM_PRINTF_ERROR("Message queue is almost full, drop old message...............\n");
				for(i = 0; i < 20; i++) {
					 PLATFORM_READ_QUEUE(queue_id, queue_message);
				}
				continue;
			}
		}

		// The first byte of 'queue_message' tells us the type of message that
		// we have just received
		//
		p = &queue_message[0];
		_E1B(&p, &message_type);
		_E2B(&p, &message_len);

		switch (message_type) {
		case PLATFORM_QUEUE_EVENT_NEW_1905_PACKET: {
			INT8U *q;

			struct interfaceInfo *x;

			INT8U  dst_addr[6];
			INT8U  src_addr[6];
			INT16U ether_type;

			INT8U receiving_interface_addr[6] = { 0 };
			char  receiving_interface_name[32] = { 0 };
			INT8U interfaces_name_len = 0;

			_E1B(&p, &interfaces_name_len);
			if (interfaces_name_len >= sizeof(receiving_interface_name)) {
				PLATFORM_PRINTF_ERROR("buffer size not enough for receiving_interface_name ! \n");
				break;
			}

			_EnB(&p, receiving_interface_name, interfaces_name_len);
			receiving_interface_name[interfaces_name_len] = '\0';

			message_len -= 1 + interfaces_name_len;

			// The first six bytes of the message payload contain the MAC
			// address of the interface where the packet was received
			//
			_EnB(&p, receiving_interface_addr, 6);

			message_len -= 6;

			if (0 == interfaces_name_len) {
				PLATFORM_PRINTF_ERROR("A packet was receiving on MAC %02x:%02x:%02x:%02x:%02x:%02x, which does not match any local interface\n", receiving_interface_addr[0], receiving_interface_addr[1], receiving_interface_addr[2], receiving_interface_addr[3], receiving_interface_addr[4], receiving_interface_addr[5]);
				pthread_mutex_lock(&map_core_mutex);
				alive_check = core_thread_flag;
				pthread_mutex_unlock(&map_core_mutex);
				continue;
			}

			x = PLATFORM_GET_1905_INTERFACE_INFO(receiving_interface_name);
			if (NULL == x) {
				PLATFORM_PRINTF_WARNING("Could not retrieve info of interface %s\n", receiving_interface_name);
				pthread_mutex_lock(&map_core_mutex);
				alive_check = core_thread_flag;
				pthread_mutex_unlock(&map_core_mutex);
				continue;
			}
			if (0 == x->is_secured) {
				PLATFORM_PRINTF_WARNING("This interface (%s) is not secured. No packets should be received. Ignoring...\n", receiving_interface_name);
				PLATFORM_FREE_1905_INTERFACE_INFO(x);
				pthread_mutex_lock(&map_core_mutex);
				alive_check = core_thread_flag;
				pthread_mutex_unlock(&map_core_mutex);
				continue;
			}
			PLATFORM_FREE_1905_INTERFACE_INFO(x);

			q = p;

			// The next bytes are the actual packet payload (ie. the
			// ethernet payload)
			//
			_EnB(&q, dst_addr, 6);
			_EnB(&q, src_addr, 6);
			_E2B(&q, &ether_type);

			PLATFORM_PRINTF_DETAIL("New queue message arrived: packet captured on interface %s\n", receiving_interface_name);
			PLATFORM_PRINTF_DETAIL("    Dst address: %02x:%02x:%02x:%02x:%02x:%02x\n", dst_addr[0], dst_addr[1], dst_addr[2], dst_addr[3], dst_addr[4], dst_addr[5]);
			PLATFORM_PRINTF_DETAIL("    Src address: %02x:%02x:%02x:%02x:%02x:%02x\n", src_addr[0], src_addr[1], src_addr[2], src_addr[3], src_addr[4], src_addr[5]);
			PLATFORM_PRINTF_DETAIL("    Ether type : 0x%04x\n", ether_type);

			switch (ether_type) {
			case ETHERTYPE_LLDP: {
				struct PAYLOAD *payload;

				PLATFORM_PRINTF_DETAIL("LLDP message received.\n");

				payload = parse_lldp_PAYLOAD_from_packet(q);

				if (NULL == payload) {
					PLATFORM_PRINTF_WARNING("Invalid bridge discovery message. Ignoring...\n");
				} else {
					PLATFORM_PRINTF_DETAIL("LLDP message contents:\n");
					if (PLATFORM_NEED_DETAIL_PRINT()) {
						visit_lldp_PAYLOAD_structure(payload, print_callback, PLATFORM_PRINTF_DETAIL, "");
					}

					processLlpdPayload(payload, receiving_interface_name);

					free_lldp_PAYLOAD_structure(payload);
				}

				break;
			}

			case ETHERTYPE_1905: {
				struct CMDU *c;

				PLATFORM_PRINTF_DETAIL("CMDU message received. Reassembling...\n");

				c = _reAssembleFragmentedCMDUs(p, message_len);

				if (NULL == c) {
					// This was just a fragment part of a big CMDU.
					// The data has been internally cached, waiting for
					// the rest of pieces.
				} else {
					if (
					    1 == _checkDuplicates(src_addr, c) && PLATFORM_STRCMP("lo", receiving_interface_name)) {
						PLATFORM_PRINTF_WARNING("Receiving on %s a CMDU which is a duplicate of a previous one (mid = %04x). Discarding...\n", receiving_interface_name, c->message_id);
					} else {
						INT8U res;
						INT8U mcast_address[] = MCAST_1905;
						// int j = 0;
						INT16U vid = 0;
#ifdef RTK_MULTI_AP_R2
						INT16U primary_vid = 0;
						INT8U default_pcp = 0;

						DMgetDefault8021QSettings(&primary_vid, &default_pcp);
#endif
						PLATFORM_PRINTF_DETAIL("CMDU message contents:\n");
						PLATFORM_PRINTF_TRACE("Receiving message_id: %04x\n", c->message_id);
						if(PLATFORM_NEED_DETAIL_PRINT()) {
							visit_1905_CMDU_structure(c, print_callback, PLATFORM_PRINTF_DETAIL, "");
						}

						if (PLATFORM_MEMCMP(dst_addr, mcast_address, 6) && PLATFORM_STRCMP("lo", receiving_interface_name)) {
							struct networkDevice *device = DMnetworkDeviceGet(src_addr);
							if (device != NULL && device->supported_services != NULL) {
								PLATFORM_PRINTF_DEBUG("Device %02x:%02x:%02x:%02x:%02x:%02x is from %s, was %s\n", src_addr[0], src_addr[1], src_addr[2], src_addr[3], src_addr[4], src_addr[5], receiving_interface_name, device->receiving_interface);

								char *base_interface_name = trimInterfaceNameVID(receiving_interface_name, &vid, NULL);
								char *base_old_receiving_interface = trimInterfaceNameVID(device->receiving_interface, NULL, NULL);
#ifdef RTK_MULTI_AP_R2
								if(primary_vid && primary_vid != vid && PLATFORM_STRSTR(base_interface_name, "wlan")) {
									PLATFORM_PRINTF_WARNING("Receiving 1905 packet from non-primary interface %s\n", receiving_interface_name);
								}
#endif
								if (PLATFORM_STRCMP(base_interface_name, base_old_receiving_interface)) {
									PLATFORM_PRINTF_INFO("Device %02x:%02x:%02x:%02x:%02x:%02x is not under %s anymore! Need remove!!!!\n", src_addr[0], src_addr[1], src_addr[2], src_addr[3], src_addr[4], src_addr[5], device->receiving_interface);
									INT8U *mac_pt = DMmacToAlMac(src_addr);
									INT8U *al_mac = NULL;
									if (NULL == mac_pt) {
										al_mac = src_addr;
									} else {
										al_mac = mac_pt;
									}
									DMremoveALNeighborFromInterface(al_mac, base_old_receiving_interface);
									if (PLATFORM_STRSTR(device->receiving_interface, "wlan")) {
										delete_agent_with_al_mac(src_addr, 1, base_interface_name);
									}
									if (mac_pt) {
										PLATFORM_FREE(mac_pt);
									}
								}
								strcpy(device->receiving_interface, receiving_interface_name);
								PLATFORM_FREE(base_interface_name);
								PLATFORM_FREE(base_old_receiving_interface);

								device->update_timestamp = PLATFORM_GET_TIMESTAMP();
							} else {
								DMaddNetworkDevice(src_addr, receiving_interface_name);
								if (c->message_type != CMDU_TYPE_TOPOLOGY_RESPONSE && c->message_type != CMDU_TYPE_TOPOLOGY_DISCOVERY && c->message_type != CMDU_TYPE_TOPOLOGY_QUERY) {
									PLATFORM_PRINTF_INFO("Unknown Device %02x:%02x:%02x:%02x:%02x:%02x, send new query!\n", src_addr[0], src_addr[1], src_addr[2], src_addr[3], src_addr[4], src_addr[5]);
									if (0 == send1905TopologyQueryPacket(receiving_interface_name, getNextMid(), src_addr, 1)) {
										PLATFORM_PRINTF_WARNING("Could not send 'topology query' message\n");
									}
								}
							}
						}

#ifdef RTK_MULTI_AP_R3
						// EasyMesh Spec 13.2.3 MIC reception requirements
						// If MAP Device receives multicast message
						if (PLATFORM_MEMCMP(dst_addr, mcast_address, 6) == 0) {
							// If receiver is Agent and sender is MAP Agent
							// and latest Agent List TLV Security field from sender has 1905 Security enabled
							// and the message does not contain MIC TLV
							// MAP Agent shall discard the message
							if (!DMisController() && (PLATFORM_MEMCMP(src_addr, DMcontrollerMacGet(), 6) != 0)
								&& (DMgetSecurity(src_addr) == SECURITY_ENABLED) && (*c->list_of_TLVs[0] != TLV_TYPE_MIC)) {
								PLATFORM_PRINTF_ERROR("Drop multicast CMDU because 1905 Security enabled but no MIC TLV!!\n");
								free_1905_CMDU_structure(c);
								break;
							}
							// Else, verify message integrity code
							if (_verifyMICTLV(c)) {
								PLATFORM_PRINTF_ERROR("Drop CMDU because MIC is incorrect!!\n");
								free_1905_CMDU_structure(c);
								break;
							}
						}
#endif

						convertToGlobalOpClass(c);
						api_send_de_tlvs_response(c->message_type, src_addr, c->list_of_TLVs);

#ifdef RTK_MULTI_AP_R3
						// EasyMesh Spec 13.3.3 Decryption Requirements
						// If MAP Agent receives unicast transmission from another MAP Agent
						// and latest Agent List TLV Security field from sender has 1905 Security enabled
						// and the message does not contain an Encrypted Payload TLV
						// MAP Agent shall discard the message
						if (!DMisController() && (PLATFORM_MEMCMP(src_addr, DMcontrollerMacGet(), 6) != 0)
							&& (PLATFORM_MEMCMP(dst_addr, mcast_address, 6) != 0)
						    && (DMgetSecurity(src_addr) == SECURITY_ENABLED) && (*c->list_of_TLVs[0] != TLV_TYPE_ENCRYPTED_PAYLOAD)) {
							PLATFORM_PRINTF_ERROR("Drop unicast CMDU because 1905 Security enabled but message not encrypted!!\n");
							free_1905_CMDU_structure(c);
							break;
						}
						// Decrypt Encrypted Payload TLV
						decryptTLV(c, receiving_interface_name, src_addr);
#endif

						// Buffer CMDU for Wi-Fi Test Suite dev_start_buffer
						if (DMgetWFATestMode())
							buffer_cmdu_message(c);

						// Process the message on the local node
						//
						res = process1905Cmdu(c, receiving_interface_name, src_addr);
						if (PROCESS_CMDU_OK_TRIGGER_AP_SEARCH == res) {
							_triggerAPSearchProcess(0);
						} else if (PROCESS_CMDU_OK_TRIGGER_AUTOCONFIG == res) {
							triggerAutoconfig(receiving_interface_name);
						}
#ifdef RTK_MULTI_AP_R2
						else if (PROCESS_CMDU_OK_MONITOR_VLAN == res) {
							PLATFORM_MONITOR_VLAN_INTERFACES(queue_id, config.primary_vid);
						}
#endif
						else if (PROCESS_CMDU_OK_DELAYED_AUTOCONFIG == res) {
							delayTriggerAutoconfig(WSC_DELAY_SECONDS, queue_id);
						}

#ifdef RTK_MULTI_AP_R3
						else if (PROCESS_CMDU_OK_TRIGGER_PEER_DISCOVERY == res) {
							if (0 == triggerDppPeerDiscovery(receiving_interface_name, src_addr))
								PLATFORM_PRINTF_WARNING("Fail to trigger DPP peer discovery with controller: %s\n", mac2str(src_addr));
						}
#endif
						// It might be necessary to retransmit this
						// message on the rest of interfaces (depending
						// on the "relayed multicast" flag
						//
						if (PLATFORM_STRCMP("lo", receiving_interface_name))
							_checkForwarding(receiving_interface_name, dst_addr, c);

						if(c->list_of_TLVs)
							api_trigger_callback(c->message_type, c->list_of_TLVs);
					}
					free_1905_CMDU_structure(c);
				}

				break;
			}

			default: {
				PLATFORM_PRINTF_WARNING("Unknown ethertype 0x%04x!! Ignoring...\n", ether_type);
				break;
			}
			}

			break;
		}

		case PLATFORM_QUEUE_EVENT_NEW_ALME_MESSAGE: {
			// ALME messages contain:
			//
			//   1- one byte with the "client id" (which must be used when
			//      later calling "PLATFORM_SEND_ALME_REPLY()")
			//
			//   2- the bit stream representation of an ALME TLV.
			//
			// We just need to convert it into a struct and process it:
			//
			INT8U  alme_client_id;
			INT8U *alme_tlv;

			_E1B(&p, &alme_client_id);

			PLATFORM_PRINTF_TRACE("New queue message arrived: ALME message (client ID = %d).\n", alme_client_id);

			alme_tlv = parse_1905_ALME_from_packet(p);
			if (NULL == alme_tlv) {
				PLATFORM_PRINTF_WARNING("Invalid ALME message. Ignoring...\n");
			}

			PLATFORM_PRINTF_DETAIL("ALME message contents:\n");
			visit_1905_ALME_structure((INT8U *)alme_tlv, print_callback, PLATFORM_PRINTF_DETAIL, "");

			process1905Alme(alme_tlv, alme_client_id);

			free_1905_ALME_structure(alme_tlv);

			break;
		}

		case PLATFORM_QUEUE_EVENT_TIMEOUT:
		case PLATFORM_QUEUE_EVENT_TIMEOUT_PERIODIC: {
			INT32U timer_id;

			// The message payload of this type of messages only contains
			// four bytes with the "timer ID" that expired.
			//
			_E4B(&p, &timer_id);

			PLATFORM_PRINTF_DETAIL("New queue message arrived: timer 0x%08x expired\n", timer_id);

			switch (timer_id) {
			case TIMER_TOKEN_DISCOVERY: {
				INT16U mid;

				char **ifs_names;
				INT8U  ifs_nr;

				// According to "Section 8.2.1.1" and "Section 8.2.1.2"
				// we now have to send a "Topology discovery message"
				// followed by a "802.1 bridge discovery message" but,
				// according to the rules in "Section 7.2", only on each
				// and every of the *authenticated* 1905 interfaces
				// that are in the state of "PWR_ON" or "PWR_SAVE"
				//

				ifs_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&ifs_nr);
				mid       = getNextMid();
				for (i = 0; i < ifs_nr; i++) {
					INT8U authenticated;
					INT8U power_state;

					struct interfaceInfo *x;

					struct LocalInterface *interface;
					interface = DMnameToLocalInterfaceStruct(ifs_names[i]);

					if (NULL == interface) {
						PLATFORM_PRINTF_ERROR("Could not retrieve info of interface %s\n", ifs_names[i]);
						continue;
					}

					if (0 == (interface->role & BACKHAUL_BSS) && 0 == (interface->role & BACKHAUL_STA)) {
						PLATFORM_PRINTF_DETAIL("%s is not a backhaul interface, skip.\n", ifs_names[i]);
						continue;
					}

					x = PLATFORM_GET_1905_INTERFACE_INFO(ifs_names[i]);
					if (NULL == x) {
						PLATFORM_PRINTF_WARNING("Could not retrieve info of interface %s\n", ifs_names[i]);
						authenticated = 0;
						power_state   = INTERFACE_POWER_STATE_OFF;
					} else {
						authenticated = x->is_secured;
						power_state   = x->power_state;

						PLATFORM_FREE_1905_INTERFACE_INFO(x);
					}

					if (
					    (0 == authenticated) || ((power_state != INTERFACE_POWER_STATE_ON) && (power_state != INTERFACE_POWER_STATE_SAVE))) {
						// Do not send the discovery messages on this
						// interface
						//
						continue;
					}

					// Topology discovery message
					//
					if (0 == send1905TopologyDiscoveryPacket(ifs_names[i], mid)) {
						PLATFORM_PRINTF_WARNING("Could not send 1905 topology discovery message\n");
					}

					// 802.1 bridge discovery message
					//
					if (0 == sendLLDPBridgeDiscoveryPacket(ifs_names[i])) {
						PLATFORM_PRINTF_WARNING("Could not send LLDP bridge discovery message\n");
					}
				}
				PLATFORM_FREE_LIST_OF_1905_INTERFACES(ifs_names, ifs_nr);

				if (!DMisController()) {
					struct networkDevice *tmp = DMnetworkDeviceGet(DMcontrollerMacGet());

					if (tmp != NULL) {
						if (0 == send1905TopologyQueryPacket(tmp->receiving_interface, getNextMid(), DMcontrollerMacGet(), 1)) {
							PLATFORM_PRINTF_WARNING("[%s] Could not send controller topology query message\n", tmp->receiving_interface);
						}
					}
				}
				break;
			}

			case TIMER_TOKEN_GARBAGE_COLLECTOR: {
				PLATFORM_PRINTF_TRACE("Running garbage collector...\n");

				INT8U is_controller_lost = 0;

				if (DMrunGarbageCollector(&is_controller_lost) > 0) {
					if (is_controller_lost) {
						INT8U **tlv_list;
						tlv_list    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *));
						tlv_list[0] = NULL;
						api_controller_change_notification(CONTROLLER_LOST, NULL, NULL);
						PLATFORM_FREE(tlv_list);
					}

					INT16U mid;

					char **ifs_names;
					INT8U  ifs_nr;

					PLATFORM_PRINTF_DETAIL("Some elements were removed. Sending a topology change notification...\n");

					// According to "Section 8.2.2.3" and "Section
					// 7.2", we now have to send a "Topology
					// Notification message) on all *authenticated*
					// interfaces that are in the state of "PWR_ON" or
					// "PWR_SAVE"
					//
					ifs_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&ifs_nr);
					mid       = getNextMid();
					for (i = 0; i < ifs_nr; i++) {
						INT8U authenticated;
						INT8U power_state;

						struct interfaceInfo *x;

						struct LocalInterface *interface;
						interface = DMnameToLocalInterfaceStruct(ifs_names[i]);

						if (NULL == interface) {
							PLATFORM_PRINTF_ERROR("Could not retrieve info of interface %s\n", ifs_names[i]);
							continue;
						}

						if (0 == (interface->role & BACKHAUL_BSS) && 0 == (interface->role & BACKHAUL_STA)) {
							PLATFORM_PRINTF_DETAIL("%s is not a backhaul interface, skip.\n", ifs_names[i]);
							continue;
						}

						x = PLATFORM_GET_1905_INTERFACE_INFO(ifs_names[i]);
						if (NULL == x) {
							PLATFORM_PRINTF_WARNING("Could not retrieve info of interface %s\n", ifs_names[i]);
							authenticated = 0;
							power_state   = INTERFACE_POWER_STATE_OFF;
						} else {
							authenticated = x->is_secured;
							power_state   = x->power_state;

							PLATFORM_FREE_1905_INTERFACE_INFO(x);
						}

						if (
						    (0 == authenticated) || ((power_state != INTERFACE_POWER_STATE_ON) && (power_state != INTERFACE_POWER_STATE_SAVE))) {
							// Do not send the topology notification  messages on
							// this interface
							//
							continue;
						}

						// Topology notification message
						//
						if (0 == send1905TopologyNotificationPacket(ifs_names[i], mid, NULL, NULL, 0)) {
							PLATFORM_PRINTF_WARNING("Could not send 1905 topology discovery message\n");
						}
					}
					PLATFORM_FREE_LIST_OF_1905_INTERFACES(ifs_names, ifs_nr);
					if (DMisController() || DMisListeningLoopback()) {
						if (0 == send1905TopologyNotificationPacket("lo", mid, NULL, NULL, 0)) {
							PLATFORM_PRINTF_WARNING("Could not send 1905 topology discovery message\n");
						}
					}
				}
				break;
			}
			case TIMER_TOKEN_CONTROLLER_ACTION: {

				//Do roaming evaluations

				break;
			}
			case TIMER_TOKEN_1_SECOND_INTERVAL: {
#ifdef RTK_MULTI_AP_R2
				if (DMisEventFlagSet(FLAG_CHANNEL_SCAN)) {
					PLATFORM_PRINTF_DEBUG("Start channel flag is on, wait to start....\n");
					channel_scan_delay += 1;
					if (channel_scan_delay >= 2) {
						PLATFORM_PRINTF_DEBUG("Start channel scan\n");
						DMsetOnBootScanFlag(0);
						_perform_channel_scan();
						DMresetEventFlag(FLAG_CHANNEL_SCAN);
						channel_scan_delay = 0;
					}
				}

				if (DMisEventFlagSet(FLAG_APPLY_VLAN)) {
					apply_vlan_delay += 1;
					if (apply_vlan_delay >= 2) {
						INT8U **empty_tlv_list = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *));
						empty_tlv_list[0]      = NULL;
						api_send_tlvs_response(MAP_VLAN_APPLY_NOTIFICATION_ELEMENT, DMalMacGet(), empty_tlv_list);
						PLATFORM_FREE(empty_tlv_list);
						DMresetEventFlag(FLAG_APPLY_VLAN);
						apply_vlan_delay = 0;
					}
				}

				//----------------On boot Channel Scan------------------------
				//change delay counter from 5 to 10 to avoid core dump
				//todo: to add enable/disable switch or adjustable counter
				if (channel_scan_onbootscan_counter < 11) {
					channel_scan_onbootscan_counter++;
				}
				if (10 == channel_scan_onbootscan_counter) {
					if (DMisEventFlagSet(FLAG_ON_BOOT_CHANNEL_SCAN)) {
						PLATFORM_PRINTF_DEBUG("Start on boot channel scan...\n");
						if (_on_boot_channel_scan()) {
							PLATFORM_PRINTF_ERROR("Failed to perform On Boot channel scan...\n");
						}
						DMresetEventFlag(FLAG_ON_BOOT_CHANNEL_SCAN);
					}
				}
				//--------------Profile 2 DFS CAC-----------------------
				// if(DMisController()){
				// 	static INT8U dfs_cac_debug_counter = 0;
				// 	INT8U tmp_mac[6] = {0x00, 0xE0, 0x4C, 0x81, 0x96, 0xC1};

				// 		dfs_cac_debug_counter++;
				// 	if (90 == dfs_cac_debug_counter) {
				// 		//---for test-------------------------------
				// 		struct CACRequestTLV cac_request_tlv;
				// 		cac_request_tlv.tlv_type = TLV_TYPE_CAC_REQUEST;
				// 		cac_request_tlv.radio_nr        = 2;
				// 		cac_request_tlv.radios          = (struct CACRequestRadio*)PLATFORM_MALLOC(2 * sizeof(struct CACRequestRadio));
				// 		{
				// 			PLATFORM_MEMCPY(cac_request_tlv.radios[0].radio_unique_identifier, tmp_mac, 6);
				// 			cac_request_tlv.radios[0].op_class	= 121;
				// 			cac_request_tlv.radios[0].channel	= 100;
				// 			cac_request_tlv.radios[0].flags		= 0b00001000;
				// 			PLATFORM_MEMCPY(cac_request_tlv.radios[1].radio_unique_identifier, tmp_mac, 6);
				// 			cac_request_tlv.radios[1].op_class	= 121;
				// 			cac_request_tlv.radios[1].channel	= 104;
				// 			cac_request_tlv.radios[1].flags		= 0b00001000;
				// 		}
				// 		//-----------------------------------------------
				// 		sendCACRequestPacket("lo", getNextMid(), tmp_mac, cac_request_tlv);
				// 		//sendAPCapabilityReportPacket("lo", getNextMid(), DMalMacGet());
				// 		//dfs_cac_debug_counter = 0;
				// 	}
				// 	if (97 == dfs_cac_debug_counter) {
				// 		//  sendCACTerminationPacket("lo", getNextMid(), tmp_mac);
				// 		//sendAPCapabilityReportPacket("lo", getNextMid(), DMalMacGet());
				// 		dfs_cac_debug_counter = 0;
				// 	}
				// }
#endif

				//--------------------Backhual Steering--------------------
				if (DMisEventFlagSet(FLAG_PREFER_BSSID_SET)) {
					if(prefer_bssid_interval == 0)
					{
						prefer_bssid_interval = PLATFORM_GET_PREFER_BSSID_INTERVAL();
						if(prefer_bssid_interval == 0)
							prefer_bssid_interval = 120;
					}

					apply_prefer_bssid_counter += 1;
					if ( (BACKHAUL_STEERING_CHECK_SECOND <= PLATFORM_GET_PREFER_BSSID_INTERVAL())&&
						(BACKHAUL_STEERING_CHECK_SECOND == apply_prefer_bssid_counter)) {
						char **ifs_names;
						INT8U *bssid = NULL;
						INT8U ifs_nr = 0, result_code = 0;
						INT16U mid;
						struct networkDevice *tmp = NULL;

						mid       = getNextMid();
						ifs_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&ifs_nr);
						for (i = 0; i < ifs_nr; i++) {
							if (BACKHAUL_STA == PLATFORM_GET_BSS_TYPE(ifs_names[i])){
								if (PLATFORM_CHECK_CONNECTED_MAC(ifs_names[i], DMgetBackhaulSteeringTarget())) {
									result_code = 0x00; //backhaul steering success
									PLATFORM_RESET_PREFER_BSSID();
									DMresetEventFlag(FLAG_PREFER_BSSID_SET);
									apply_prefer_bssid_counter = 0;
								}
								else {
									result_code = 0x01; //backhaul steering failure
								}
									tmp   = DMnetworkDeviceGet(DMcontrollerMacGet());
									bssid = DMinterfaceNameToMac(ifs_names[i]);
									if (NULL != tmp) {
										if (0 == sendBackhaulSteeringResponsePacket(tmp->receiving_interface,
											mid, DMcontrollerMacGet(), bssid, DMgetBackhaulSteeringTarget(), result_code)) {
											PLATFORM_PRINTF_WARNING("[%s] Could not send Backhaul Steering Response message\n",
												tmp->receiving_interface);
										}
									}
									break;
								}
							}
						}

					if (apply_prefer_bssid_counter > prefer_bssid_interval)
					{
						PLATFORM_RESET_PREFER_BSSID();
						DMresetEventFlag(FLAG_PREFER_BSSID_SET);
						apply_prefer_bssid_counter = 0;
					}
				}

				//-----------------------------------------------
				search_counter++;
				if (search_counter % 5 == 0) {
					if (search_counter >= 20) {
						_triggerAPSearchProcess(1);
						search_counter = 0;
					} else {
						_triggerAPSearchProcess(0);
					}

					// if (!DMisController()) {
					// 	DMsetSTPState(_check_stp_state());
					// }
				}
				//check if interval action required
				if (DMintervalActionRequired()) {

					INT8U(*bssid_list)[6] = NULL;
					INT8U (*radio_mac_list)[6] = NULL;
					char **ifs_names;
					INT8U  bssid_nr = 0, radio_nr = 0, ifs_nr;

					ifs_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&ifs_nr);

					for (i = 0; i < ifs_nr; i++) {
						INT8U *               bssid;

						// All BSSs shall be included per the spec
						if (PLATFORM_STRSTR(ifs_names[i], map_vxd_prefix)
							|| PLATFORM_GET_VALID_INTERFACE_NAME(ifs_names[i], VALID_ETH_INTERFACES_IN_BRIDGE)
							|| (controller_interface && PLATFORM_STRSTR(ifs_names[i], controller_interface))) {
							continue;
						}
						bssid = DMinterfaceNameToMac(ifs_names[i]);
						if (bssid) {
							bssid_nr++;
							bssid_list = PLATFORM_REALLOC(bssid_list, (sizeof(*(bssid_list)) * bssid_nr));

							// Get the mac of operating radio (root interface)
							if (PLATFORM_STRSTR(ifs_names[i], map_vap_prefix) == 0) {
								radio_nr++;
								radio_mac_list = PLATFORM_REALLOC(radio_mac_list, (sizeof(*(radio_mac_list)) * radio_nr));
								PLATFORM_MEMCPY(radio_mac_list[radio_nr - 1], bssid, MACADDRLEN);
							}

							PLATFORM_MEMCPY(bssid_list[bssid_nr - 1], bssid, MACADDRLEN);
						}
					}

					struct networkDevice *tmp = DMnetworkDeviceGet(DMcontrollerMacGet());

					if (NULL != tmp) {
						if (0 == sendAPMetricResponsePacket(tmp->receiving_interface, getNextMid(), DMcontrollerMacGet(), bssid_nr, bssid_list, radio_nr, radio_mac_list)) {
							PLATFORM_PRINTF_WARNING("Cannot send AP Metric Response message!\n");
						}
					}
					if (bssid_nr)
						PLATFORM_FREE(bssid_list);

					if (radio_nr)
						PLATFORM_FREE(radio_mac_list);
				}

				//-------------------------RCPI Policy-based Steering-------------------------
				if (DMgetWFATestMode()) {
					struct networkDevice *tmp = DMnetworkDeviceGet(DMalMacGet());
					INT8U rcpi = 0, rcpi_steering_threshold = 0;
					char *interface_name = NULL;

					if (NULL != tmp) {
						for (i = 0; i < tmp->non1905_neighbors_nr; i++) {
							interface_name = DMmacToInterfaceName(tmp->non1905_neighbors[i]->local_mac_address);
							if(interface_name == NULL)
								continue;
							//Sterring Policy only stores in root interface
							if (DMgetRCPISteeringThreshold(DMgetRootInterfaceMAC(interface_name), &rcpi_steering_threshold)) {
								rcpi = PLATFORM_GET_CLIENT_RCPI(interface_name, tmp->non1905_neighbors[i]->non_1905_neighbors->mac_address, 0);
								if (rcpi <= 20)
									break;
								PLATFORM_PRINTF_DEBUG("[Multiap][RCPI steering]rssi:%u threshold:%u\n", rcpi, rcpi_steering_threshold);
								if (rcpi < rcpi_steering_threshold &&
									!DMcheckBTMSteeringDisallow(tmp->non1905_neighbors[i]->non_1905_neighbors->mac_address)) {

									//DO BTM request
									PLATFORM_BTM_REQ(interface_name, tmp->non1905_neighbors[i]->non_1905_neighbors->mac_address,
											0, NULL, 0, 0, 0, 0);
								}
							}
						}
					}
				}

				if (DMisEventFlagSet(FLAG_WAIT_FOR_M1)) {
					DMsetM1Delay(DMgetM1Delay()+1);
					if (DMgetM1Delay() >= 10) {
						struct dpp_eapol_sm *sm = NULL;
						DMgetDPPEAPOLStateMachine(&sm);
						sm->state = DISABLED;
						DMresetEventFlag(FLAG_WAIT_FOR_M1);
						DMsetM1Delay(0);
					}
				}

#ifdef RTK_MULTI_AP_R3
				if (DMisEventFlagSet(FLAG_ON_EAPOL)) {
					eapol_delay += 1;
					INT8U eapol_timeout = 5;
					if (DMgetWFATestMode())
						eapol_timeout = 13;

					if (eapol_delay >= eapol_timeout) {
						struct dpp_eapol_sm *sm = NULL;
						DMgetDPPEAPOLStateMachine(&sm);
						sm->state = DISABLED;
						DMresetEventFlag(FLAG_ON_EAPOL);
						eapol_delay = 0;
					}
				}
#endif


				break;
			}

			case TIMER_TOKEN_AUTOCONFIG: {
				PLATFORM_PRINTF_DEBUG("TIMER_TOKEN_AUTOCONFIG arms!\n");
				struct networkDevice *tmp = DMnetworkDeviceGet(DMcontrollerMacGet());
				if (!tmp) {
					PLATFORM_PRINTF_WARNING("Could not send AP-Autoconfig message because target not found\n");
					break;
				}

#ifdef RTK_MULTI_AP_R3
				struct dpp_authentication *dpp_auth = DMgetDPPAuthenticationObject();
				if (dpp_auth->in_progress || dpp_auth->auth_success) {
					PLATFORM_PRINTF_WARNING("DPP has already started. Skip WSC\n");
					break;
				}
#endif
				PLATFORM_PRINTF_INFO("Trigger AP-Autoconfig on %s!\n", tmp->receiving_interface);
				DMresetEventFlag(FLAG_DELAYED_AUTOCONFIG);
				DMsetConfiguredState(0x00);
				triggerAutoconfig(tmp->receiving_interface);
				break;
			}

			case TIMER_TOKEN_RECONFIG: {
#ifdef RTK_MULTI_AP_R4
				if (DMisEventFlagSet(FLAG_DELAYED_RECONFIG)) {
					break;
				}

				INT8U *ruid = NULL;
				char  *ifname = NULL;
				char **backhaul_interfaces;
				INT8U  backhaul_interfaces_nr, i;

				ruid = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * 6);
				_EnB(&p, ruid, MACADDRLEN);
				backhaul_interfaces = (char **)get_backhaul_interfaces(&backhaul_interfaces_nr);
				for (i = 0; i < backhaul_interfaces_nr; i++) {
					struct interfaceInfo *x;
					x = PLATFORM_GET_1905_INTERFACE_INFO(backhaul_interfaces[i]);
					if (NULL == x) {
						PLATFORM_PRINTF_WARNING("Could not retrieve info of interface %s\n", backhaul_interfaces[i]);
						continue;
					}
					if (!PLATFORM_STRSTR(backhaul_interfaces[i], map_vxd_prefix)){
						PLATFORM_FREE_1905_INTERFACE_INFO(x);
						continue;
					}
					if (0 == PLATFORM_MEMCMP(x->mac_address, ruid, 6)) {
						ifname = x->name;
					}
					PLATFORM_FREE_1905_INTERFACE_INFO(x);
				}

				if (ifname != NULL) {
					PLATFORM_MAP_START_RECONFIG(ifname);
				}

				PLATFORM_FREE(ruid);
				PLATFORM_FREE(backhaul_interfaces);

				if (reconfig_delay == 6) {
					DMsetEventFlag(FLAG_DELAYED_RECONFIG);
					break;
				}
				reconfig_delay += 1;
				delayTriggerReconfig(20, queue_id);
#endif // RTK_MULTI_AP_R4
				break;
			}

			default: {
				PLATFORM_PRINTF_WARNING("Unknown timer ID!! Ignoring...\n");
				break;
			}
			}

			break;
		}

		case PLATFORM_QUEUE_EVENT_PUSH_BUTTON: {
			INT16U mid;

			char **ifs_names;
			INT8U  ifs_nr;

			INT8U *no_push_button;

			char *backhaul_bss_name = NULL;
			INT8U backhaul_bss_count = 0;

			PLATFORM_PRINTF_DEBUG("New queue message arrived: push button event\n");

			PLATFORM_PRINTF_DEBUG("[PBC] overwrite %02x\n", config.overwrite_pbc);
			if (config.overwrite_pbc) {
				INT8U **tlv_list;
				tlv_list    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *));
				tlv_list[0] = NULL;

				api_send_tlvs_response(MAP_PUSH_BUTTON_NOTIFICATION_ELEMENT, DMalMacGet(), tlv_list);
				PLATFORM_FREE(tlv_list);
				break;
			}

			// According to "Section 9.2.2.1", we must first make sure that
			// none of the interfaces is in the middle of a previous "push
			// button" configuration sequence.
			//
			ifs_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&ifs_nr);
			mid       = getNextMid();

			for (i = 0; i < ifs_nr; i++) {
				struct interfaceInfo *x;

				x = PLATFORM_GET_1905_INTERFACE_INFO_WITH_WPS(ifs_names[i], 1);
				if (!DMgetWFATestMode()) {
					if (DMisTribandDevice() && (IS_IEEE_802_11_INTERFACE(x->interface_type)) && x->interface_type_data.ieee80211.network_type == MULTI_AP_BACKHAUL_BSS && x->interface_type_data.ieee80211.rf_band == IEEE80211_RF_BAND_5_GHZ) {
						if (backhaul_bss_name) {
							PLATFORM_PRINTF_INFO("backhual bss already found\n");
							backhaul_bss_count++;
						} else {
							backhaul_bss_name = (char *)PLATFORM_MALLOC(20);
							PLATFORM_MEMSET(backhaul_bss_name, 0, 20);
							strcpy(backhaul_bss_name, x->name);
							backhaul_bss_count++;
							PLATFORM_PRINTF_INFO("backhual bss found, [%s] \n", x->name);
						}
					}
				}

				if (NULL == x) {
					PLATFORM_PRINTF_WARNING("Could not retrieve info of interface %s\n", ifs_names[i]);
					break;
				} else {
					if (1 == x->push_button_on_going) {
						PLATFORM_PRINTF_INFO("Previous 'push button' configuration sequence is still ongoing. Ignoring new event...\n");
						PLATFORM_FREE_1905_INTERFACE_INFO(x);
						break;
					}
					PLATFORM_FREE_1905_INTERFACE_INFO(x);
				}
			}
			if (i < ifs_nr) {
				// Don't do anything
				//
				if (!DMgetWFATestMode()) {
					PLATFORM_FREE(backhaul_bss_name);
				}
				break;
			}

			// If we get here, none of the interfaces is in the middle of a
			// "push button" configuration process, thus we can initialize
			// the "push button event" on all of our interfaces that support
			// it.
			//
			// Let's see which interfaces support it and keep track of
			// those who don't by setting the corresponding byte in array
			// "no_push_button" to '1'
			//
			no_push_button = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * ifs_nr);

			for (i = 0; i < ifs_nr; i++) {
				struct interfaceInfo *x;

				x = PLATFORM_GET_1905_INTERFACE_INFO_WITH_WPS(ifs_names[i], 1);
				if (NULL == x) {
					PLATFORM_PRINTF_WARNING("Could not retrieve info of interface %s\n", ifs_names[i]);
					no_push_button[i] = 1;
					break;
				} else {
					if (INTERFACE_POWER_STATE_OFF == x->power_state) {
						// Ignore interfaces that are switched off
						//
						PLATFORM_PRINTF_DETAIL("Skipping interface %s because it is powered off\n", ifs_names[i]);
						no_push_button[i] = 1;
					} else if (2 == x->push_button_on_going) {
						// This interface does not support the "push button"
						// configuration process
						//
						PLATFORM_PRINTF_DETAIL("Skipping interface %s because it does not support the push button configuration mechanism\n", ifs_names[i]);
						no_push_button[i] = 2;

						// NOTE: "2" will be used as a special marker to
						//       later trigger the AP search process (see
						//       below)
					} else if (IEEE80211_ROLE_AP != x->interface_type_data.ieee80211.role
						&& (IS_IEEE_802_11_INTERFACE(x->interface_type))
					    && (0x0 != x->interface_type_data.ieee80211.bssid[0] 
					    ||  0x0 != x->interface_type_data.ieee80211.bssid[1] 
					    ||  0x0 != x->interface_type_data.ieee80211.bssid[2] 
					    ||  0x0 != x->interface_type_data.ieee80211.bssid[3] 
					    ||  0x0 != x->interface_type_data.ieee80211.bssid[4] 
					    ||  0x0 != x->interface_type_data.ieee80211.bssid[5])) {
						// According to "Section 9.2.2.1", an 802.11 STA
						// which is already paired with an AP must *not*
						// start the "push button" configuration process.
						//
						PLATFORM_PRINTF_DETAIL("Skipping interface %s because it is a wifi STA already associated to an AP\n", ifs_names[i]);
						no_push_button[i] = 1;
					} else {
						no_push_button[i] = 0;
					}

					// This is for triband, with 2 full 5G band.
					// If there is only 1 backhual ap, when 5GL vap0 is set as backhual bss, dont do wps on 5GH root. Vice versa.
					// But if there are two backhual ap, must do on both.
					if (!DMgetWFATestMode()) {
						if (backhaul_bss_name) {
							if (backhaul_bss_count == 1 && (IS_IEEE_802_11_INTERFACE(x->interface_type)) && x->interface_type_data.ieee80211.rf_band == IEEE80211_RF_BAND_5_GHZ && x->interface_type_data.ieee80211.network_type != MULTI_AP_BACKHAUL_STA && NULL == strstr(backhaul_bss_name, x->name)) {
								PLATFORM_PRINTF_INFO("[%s] no push button set to 1 \n", x->name);
								no_push_button[i] = 1;
							}
						}
					}
					PLATFORM_FREE_1905_INTERFACE_INFO(x);
				}
			}

			if (!DMgetWFATestMode()) {
				PLATFORM_FREE(backhaul_bss_name);
			}
			// We now have the list of interfaces that need to start their
			// "push button" configuration process. Let's do it:
			//

#ifdef _RTK_LINUX_
			char wsc_interface[30];

			memset(wsc_interface, 0, sizeof(wsc_interface));
			// Select interface to trigger within all unconnected stations and APs
			// 5GH vxd -> 5GL vxd -> 2G vxd -> 5GH root -> 5GL root -> 2G root
			// skip interfaces operating backhaul bss only
			INT8U  sum            = 0;
			INT8U *controller_mac = DMcontrollerMacGet();
			for (i = 0; i < 6; ++i) {
				sum |= controller_mac[i];
			}
			PLATFORM_PRINTF_INFO("Controller SUM %02x\n", sum);

			for (i = 0; i < ifs_nr; i++) {
				if (0 == no_push_button[i]) {
					if (strstr(ifs_names[i], map_vxd_prefix)) {
						if (sum != 0) {
							// Skip vxd interfaces if controller is found
							continue;
						}

						if (strstr(ifs_names[i], map_radio_name_5gl) && strstr(wsc_interface, map_vxd_prefix) && strstr(wsc_interface, map_radio_name_5gh)) {
							continue;
						}

						if (strstr(ifs_names[i], map_radio_name_24g) && strstr(wsc_interface, map_vxd_prefix)) {
							continue;
						}
						strcpy(wsc_interface, ifs_names[i]);
					} else {
						if (strstr(wsc_interface, map_vxd_prefix)) {
							continue;
						}

						if (strstr(ifs_names[i], map_vap_prefix)) {
							// Skip VAP
							continue;
						}
						if (strstr(ifs_names[i], map_radio_name_5gl) && strstr(wsc_interface, map_radio_name_5gh)) {
							continue;
						}
						if (strstr(ifs_names[i], map_radio_name_24g) && (strstr(wsc_interface, map_radio_name_5gl) || strstr(wsc_interface, map_radio_name_5gh))) {
							continue;
						}

						strcpy(wsc_interface, ifs_names[i]);
					}
				}
			}

			if (strlen(wsc_interface) != 0) {
				char *base_name = trimInterfaceNameVID(wsc_interface, NULL, NULL);
				strlcpy(wsc_interface, base_name, sizeof(wsc_interface));
				PLATFORM_FREE(base_name);

				PLATFORM_PRINTF_INFO("Starting push button configuration process on interface %s\n", wsc_interface);
				PLATFORM_START_PUSH_BUTTON_CONFIGURATION(wsc_interface, queue_id, DMalMacGet(), mid);

				if (PLATFORM_NEED_HAPD() && NULL == strstr(wsc_interface, map_vxd_prefix)) {
					for (i = 0; i < ifs_nr; i++) {
						if (0 == no_push_button[i]) {
							if (strlen(wsc_interface) == strlen(ifs_names[i]) && strcmp(wsc_interface, ifs_names[i])) {
								PLATFORM_START_PUSH_BUTTON_CONFIGURATION(ifs_names[i], queue_id, DMalMacGet(), getNextMid());
							}
						}
					}
				}
			} else {
				PLATFORM_FREE(no_push_button);
				PLATFORM_FREE_LIST_OF_1905_INTERFACES(ifs_names, ifs_nr);
				break;
			}
#else
			for (i = 0; i < ifs_nr; i++) {
				if (0 == no_push_button[i]) {
					PLATFORM_PRINTF_INFO("Starting push button configuration process on interface %s\n", ifs_names[i]);
					PLATFORM_START_PUSH_BUTTON_CONFIGURATION(ifs_names[i], queue_id, DMalMacGet(), mid);
				}
				// if (2 == no_push_button[i]) {
				// 	at_least_one_unsupported_interface = 1;
				// }
			}
#endif
			PLATFORM_FREE(no_push_button);
			PLATFORM_FREE_LIST_OF_1905_INTERFACES(ifs_names, ifs_nr);

			break;
		}

		case PLATFORM_QUEUE_EVENT_AUTHENTICATED_LINK: {
			// Two different things need to be done when a new interface is
			// authenticated:
			//
			//   1. According to "Section 9.2.2.3", a "push button join
			//      notification" message must be generated and sent.
			//
			//   2. According to "Section 10.1", the "AP-autoconfiguration"
			//      process is triggered.

			INT16U mid;

			INT8U  local_mac_addr[6];
			INT8U  new_mac_addr[6];
			INT8U  original_al_mac_addr[6];
			INT16U original_mid;

			char **ifs_names;
			INT8U  ifs_nr;

			// The first six bytes of the message payload contain the MAC
			// address of the interface where the "push button"
			// configuration process succeeded.
			//
			_EnB(&p, local_mac_addr, 6);

			// The next six bytes contain the MAC address of the interface
			// successfully authenticated at the other end.
			//
			_EnB(&p, new_mac_addr, 6);

			// The next six bytes contain the original AL MAC address that
			// started everything.
			//
			_EnB(&p, original_al_mac_addr, 6);

			// Finally, the last two bytes contains the MID of that original
			// message
			//
			_E2B(&p, &original_mid);
#ifdef _RTK_LINUX_
			INT8U is_from_vxd;
			_E1B(&p, &is_from_vxd);
#endif

			PLATFORM_PRINTF_TRACE("New queue message arrived: authenticated link\n");
			PLATFORM_PRINTF_DETAIL("    Local interface:        %02x:%02x:%02x:%02x:%02x:%02x\n", local_mac_addr[0], local_mac_addr[1], local_mac_addr[2], local_mac_addr[3], local_mac_addr[4], local_mac_addr[5]);
			PLATFORM_PRINTF_DETAIL("    New (remote) interface: %02x:%02x:%02x:%02x:%02x:%02x\n", new_mac_addr[0], new_mac_addr[1], new_mac_addr[2], new_mac_addr[3], new_mac_addr[4], new_mac_addr[5]);
			PLATFORM_PRINTF_DETAIL("    Original AL MAC       : %02x:%02x:%02x:%02x:%02x:%02x\n", original_al_mac_addr[0], original_al_mac_addr[1], original_al_mac_addr[2], original_al_mac_addr[3], original_al_mac_addr[4], original_al_mac_addr[5]);
			PLATFORM_PRINTF_DETAIL("    Original MID          : %d\n", original_mid);

			ifs_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&ifs_nr);

			// If "new_mac_addr" is NULL, this means the interface was
			// "authenticated" as a whole (not at "link level").
			// This happens for ethernet interfaces.
			// In these cases we must *not* send the "push button join
			// notification" message (note, however, that the "AP-
			// autoconfiguration" process does need to be triggered, which
			// is done later)
			//
			if (
			    new_mac_addr[0] == 0x00 && new_mac_addr[1] == 0x00 && new_mac_addr[2] == 0x00 && new_mac_addr[3] == 0x00 && new_mac_addr[4] == 0x00 && new_mac_addr[5] == 0x00) {
				PLATFORM_PRINTF_DETAIL("NULL new (remote) interface. No 'push button join notification' will be sent.\n");
			} else {
				// Send the "push button join notification" message on all
				// authenticated interfaces (except for the one just
				// authenticated)
				//
				mid = getNextMid();
				for (i = 0; i < ifs_nr; i++) {
					INT8U authenticated;
					INT8U power_state;

					struct interfaceInfo *x;

					struct LocalInterface *interface;
					interface = DMnameToLocalInterfaceStruct(ifs_names[i]);

					if (NULL == interface) {
						PLATFORM_PRINTF_ERROR("Could not retrieve info of interface %s\n", ifs_names[i]);
						continue;
					}

					if (0 == (interface->role & BACKHAUL_BSS) && 0 == (interface->role & BACKHAUL_STA)) {
						PLATFORM_PRINTF_DETAIL("%s is not a backhaul interface, skip.\n", ifs_names[i]);
						continue;
					}

					x = PLATFORM_GET_1905_INTERFACE_INFO(ifs_names[i]);
					if (NULL == x) {
						PLATFORM_PRINTF_WARNING("Could not retrieve info of interface %s\n", ifs_names[i]);
						authenticated = 0;
						power_state   = INTERFACE_POWER_STATE_OFF;
						continue;
					} else {
						authenticated = x->is_secured;
						power_state   = x->power_state;
					}
#ifdef _RTK_LINUX_
					INT8U is_source_interface = 0;
					if (0 == PLATFORM_MEMCMP(x->mac_address, local_mac_addr, 6)
					    && (is_from_vxd) == (strstr(ifs_names[i], map_vxd_prefix) != NULL)) {
						is_source_interface = 1;
					}
					if ((0 == authenticated)
					    || ((power_state != INTERFACE_POWER_STATE_ON)
					        && (power_state != INTERFACE_POWER_STATE_SAVE))
					    || is_source_interface) {
						PLATFORM_FREE_1905_INTERFACE_INFO(x);
						continue;
					}
#else

					if (
					    (0 == authenticated) || ((power_state != INTERFACE_POWER_STATE_ON) && (power_state != INTERFACE_POWER_STATE_SAVE)) || (0 == PLATFORM_MEMCMP(x->mac_address, local_mac_addr, 6))) {

						// Do not send the message on this interface
						//
						if (NULL != x) {
							PLATFORM_FREE_1905_INTERFACE_INFO(x);
						}
						continue;
					}
#endif

					if (NULL != x) {
						PLATFORM_FREE_1905_INTERFACE_INFO(x);
					}

					if (0 == send1905PushButtonJoinNotificationPacket(ifs_names[i], mid, original_al_mac_addr, original_mid, local_mac_addr, new_mac_addr)) {
						PLATFORM_PRINTF_WARNING("Could not send 1905 push button join notification message\n");
					}
				}
			}

			PLATFORM_FREE_LIST_OF_1905_INTERFACES(ifs_names, ifs_nr);

			// Finally, trigger the "AP-autoconfiguration" process
			//
			_triggerAPSearchProcess(0);

			break;
		}

		case PLATFORM_QUEUE_EVENT_TOPOLOGY_CHANGE_NOTIFICATION: {
			INT16U mid;

			char **ifs_names;
			INT8U  ifs_nr;
			INT8U  client_mac_addr[6], bssid[6], event;

			PLATFORM_PRINTF_TRACE("New queue message arrived: topology change notification event\n");

			_EnB(&p, client_mac_addr, 6);
			_EnB(&p, bssid, 6);
			_E1B(&p, &event);

			// TODO:
			//   1. Find which L2 neighbors are no longer available
			//   2. Set their timestamp to 0
			//   3. Call DMrunGarbageCollector() to remove them from the
			//      database
			//
			// Until this is done, nodes will only be removed from the
			// database when the "TIMER_TOKEN_GARBAGE_COLLECTOR" timer
			// expires.

			// According to "Section 8.2.2.3" and "Section 7.2", we now
			// have to send a "Topology Notification" message) on all
			// *authenticated* interfaces that are in the state of "PWR_ON"
			// or "PWR_SAVE"
			//
			ifs_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&ifs_nr);
			mid       = getNextMid();
			if (DMisController() || DMisListeningLoopback()) {
				struct networkDevice *network_device = DMnetworkDeviceGet(DMalMacGet());
				if (network_device) {
					if (0 == send1905TopologyNotificationPacket(network_device->receiving_interface, mid, client_mac_addr, bssid, event)) {
						PLATFORM_PRINTF_WARNING("[%s] Could not send 1905 topology notification message\n", network_device->receiving_interface);
					}
				}
			}
			for (i = 0; i < ifs_nr; i++) {
				INT8U authenticated;
				INT8U power_state;

				struct interfaceInfo *x;

				struct LocalInterface *interface;
				interface = DMnameToLocalInterfaceStruct(ifs_names[i]);

				if (NULL == interface) {
					PLATFORM_PRINTF_ERROR("Could not retrieve info of interface %s\n", ifs_names[i]);
					continue;
				}

				if (0 == (interface->role & BACKHAUL_BSS) && 0 == (interface->role & BACKHAUL_STA)) {
					PLATFORM_PRINTF_DETAIL("%s is not a backhaul interface, skip.\n", ifs_names[i]);
					continue;
				}

				x = PLATFORM_GET_1905_INTERFACE_INFO(ifs_names[i]);
				if (NULL == x) {
					PLATFORM_PRINTF_WARNING("Could not retrieve info of interface %s\n", ifs_names[i]);
					authenticated = 0;
					power_state   = INTERFACE_POWER_STATE_OFF;
				} else {
					authenticated = x->is_secured;
					power_state   = x->power_state;

					PLATFORM_FREE_1905_INTERFACE_INFO(x);
				}

				if (
				    (0 == authenticated) || ((power_state != INTERFACE_POWER_STATE_ON) && (power_state != INTERFACE_POWER_STATE_SAVE))) {
					// Do not send the topology notification  messages on
					// this interface
					//
					continue;
				}

				// Topology notification message
				//
				if (0 == send1905TopologyNotificationPacket(ifs_names[i], mid, client_mac_addr, bssid, event)) {
					PLATFORM_PRINTF_WARNING("[%s] Could not send 1905 topology discovery message\n", ifs_names[i]);
				}
			}
			PLATFORM_FREE_LIST_OF_1905_INTERFACES(ifs_names, ifs_nr);

			break;
		}

		case PLATFORM_QUEUE_EVENT_BEACON_METRIC_RESPONSE_NOTIFICATION: {

			INT16U mid;
			mid = getNextMid();

			INT8U *buf = PLATFORM_MALLOC(message_len);

			_EnB(&p, buf, message_len);

			struct networkDevice *tmp = DMnetworkDeviceGet(DMcontrollerMacGet());

			if (tmp != NULL) {
				if (0 == sendBeaconMetricsResponsePacket(tmp->receiving_interface, mid, DMcontrollerMacGet(), buf, message_len)) {
					PLATFORM_PRINTF_WARNING("[%s] Could not send Beacon Metric Response message\n", tmp->receiving_interface);
				}
			} else {
				PLATFORM_PRINTF_WARNING("Could not send Beacon Metric Response message because target not found\n");
			}

			if (!DMisController())
				sendBeaconMetricsResponsePacket("lo", mid, DMalMacGet(), buf, message_len);

			PLATFORM_FREE(buf);

			break;
		}

		case PLATFORM_QUEUE_EVENT_BACKHAUL_STEERING_RESPONSE_NOTIFICATION: {

			INT16U mid;
			mid = getNextMid();

			INT8U backhaul_sta_mac[6] = {0}, target_bss_mac[6] = {0};
			INT8U result_code;

			_EnB(&p, backhaul_sta_mac, 6);
			_EnB(&p, target_bss_mac, 6);
			_E1B(&p, &result_code);

			struct networkDevice *tmp = DMnetworkDeviceGet(DMcontrollerMacGet());

			if (tmp != NULL) {
				if (0 == sendBackhaulSteeringResponsePacket(tmp->receiving_interface, mid, DMcontrollerMacGet(), backhaul_sta_mac, target_bss_mac, result_code)) {
					PLATFORM_PRINTF_WARNING("[%s] Could not send Backhaul Steering Response message\n", tmp->receiving_interface);
				}
			} else {
				PLATFORM_PRINTF_WARNING("Could not send Backhaul Steering Response message because target not found\n");
			}
			break;
		}

		case PLATFORM_QUEUE_EVENT_AP_METRIC_RESPONSE_NOTIFICATION: {
			INT16U mid;
			mid = getNextMid();

			INT8U(*bssid_list)[6] = NULL;
			INT8U (*radio_mac_list)[6] = NULL;
			char **ifs_names;
			INT8U  bssid_nr = 0, radio_nr = 0, ifs_nr;

			ifs_names = PLATFORM_GET_LIST_OF_1905_INTERFACES(&ifs_nr);

			for (i = 0; i < ifs_nr; i++) {
				INT8U *               bssid;

				// All BSSs shall be included per the spec
				if (PLATFORM_STRSTR(ifs_names[i], map_vxd_prefix)
					|| PLATFORM_GET_VALID_INTERFACE_NAME(ifs_names[i], VALID_ETH_INTERFACES_IN_BRIDGE)
					|| (controller_interface && PLATFORM_STRSTR(ifs_names[i], controller_interface))) {
					continue;
				}
				bssid = DMinterfaceNameToMac(ifs_names[i]);
				if (bssid) {
					bssid_nr++;
					bssid_list = PLATFORM_REALLOC(bssid_list, (sizeof(*(bssid_list)) * bssid_nr));

					PLATFORM_MEMCPY(bssid_list[bssid_nr - 1], bssid, MACADDRLEN);

					// Get the mac of operating radio (root interface)
					if (PLATFORM_STRSTR(ifs_names[i], map_vap_prefix) == 0) {
						radio_nr++;
						radio_mac_list = PLATFORM_REALLOC(radio_mac_list, (sizeof(*(radio_mac_list)) * radio_nr));
						PLATFORM_MEMCPY(radio_mac_list[radio_nr - 1], bssid, MACADDRLEN);
					}
				}
			}

			struct networkDevice *tmp = DMnetworkDeviceGet(DMcontrollerMacGet());

			if (tmp != NULL) {
				if (0 == sendAPMetricResponsePacket(tmp->receiving_interface, mid, DMcontrollerMacGet(), bssid_nr, bssid_list, radio_nr, radio_mac_list)) {
					PLATFORM_PRINTF_WARNING("[%s] Could not send AP Metric Response message\n", tmp->receiving_interface);
				}
			} else {
				PLATFORM_PRINTF_WARNING("Could not send AP Metric Response message because target not found\n");
			}

			if (bssid_nr)
				PLATFORM_FREE(bssid_list);

			if (radio_nr)
				PLATFORM_FREE(radio_mac_list);

			break;
		}

		case PLATFORM_QUEUE_EVENT_ASSOC_STA_LINK_METRIC_NOTIFICATION: {
			INT16U mid;
			mid = getNextMid();

			INT8U station_mac[6];
			_EnB(&p, station_mac, MACADDRLEN);

			struct networkDevice *tmp = DMnetworkDeviceGet(DMcontrollerMacGet());

			if (tmp != NULL) {
				if (0 == sendAssocSTALinkMetricResponsePacket(tmp->receiving_interface, mid, DMcontrollerMacGet(), station_mac)) {
					PLATFORM_PRINTF_WARNING("[%s] Could not send Assoc STA Metric Response message\n", tmp->receiving_interface);
				}
			} else {
				PLATFORM_PRINTF_WARNING("Could not send Assoc STA Metric Response message because target not found\n");
			}

			break;
		}

		case PLATFORM_QUEUE_EVENT_UNASSOC_STA_LINK_METRIC_NOTIFICATION: {

			INT16U mid;
			mid = getNextMid();

			INT8U op_class, sta_nr;
			INT8U buf[256];

			_E1B(&p, &op_class);
			_E1B(&p, &sta_nr);
			_EnB(&p, buf, message_len - 2);

			struct networkDevice *tmp = DMnetworkDeviceGet(DMcontrollerMacGet());

			if (tmp != NULL) {
				if (0 == sendUnassocSTALinkMetricResponsePacket(tmp->receiving_interface, mid, DMcontrollerMacGet(), op_class, sta_nr, buf)) {
					PLATFORM_PRINTF_WARNING("[%s] Could not send Unassoc STA Link Metric message\n", tmp->receiving_interface);
				}
			} else {
				PLATFORM_PRINTF_WARNING("Could not send Unassoc STA Link Metric message because target not found\n");
			}

			break;
		}

		case PLATFORM_QUEUE_EVENT_BTM_RESPONSE: {
			INT16U mid;
			mid = getNextMid();

			struct bss_transition_response_para response;
			_EnB(&p, response.addr, 6);
			_EnB(&p, response.bssid, 6);
			p++;
			_EnB(&p, response.target_bssid, 6);
			_E1B(&p, &response.status_code);

			struct networkDevice *tmp = DMnetworkDeviceGet(DMcontrollerMacGet());

			if (tmp != NULL) {
				if (0 == sendMultiAPClientSteeringBTMReportPacket(tmp->receiving_interface, mid, DMcontrollerMacGet(), &response)) {
					PLATFORM_PRINTF_WARNING("[%s] Could not send Client Steering BTM Report message\n", tmp->receiving_interface);
				}
			} else {
				PLATFORM_PRINTF_WARNING("Could not send Client Steering BTM Report message because target not found\n");
			}
			break;
		}
		case PLATFORM_QUEUE_EVENT_CHANNEL_CHANGE_NOTIFICATION: {
			INT8U *controller_mac                  = DMcontrollerMacGet();
			struct networkDevice *             tmp = DMnetworkDeviceGet(controller_mac);
			if (NULL != tmp) {
				sendMultiAPOperatingChannelReportPacket(tmp->receiving_interface, getNextMid(), controller_mac);
			}
			INT8U **tlv_list;
			tlv_list    = (INT8U **)PLATFORM_MALLOC(sizeof(INT8U *));
			tlv_list[0] = NULL;

			api_send_tlvs_response(MAP_OPERATING_CHANNEL_CHANGE_NOTIFICATION_ELEMENT, DMalMacGet(), tlv_list);
			PLATFORM_FREE(tlv_list);
			break;
		}
		case MAP_API_REQUEST: {
			PLATFORM_PRINTF_INFO("<---- MAP_API_REQUEST\n");
			INT8U *element = map_parse_element(p);
			INT8U res = processAPIRequest(element);
			map_free_element(element);
#ifdef RTK_MULTI_AP_R2
			INT16U primary_vid = 0;
			INT8U  default_pcp = 0;
			DMgetDefault8021QSettings(&primary_vid, &default_pcp);
			if(PROCESS_CMDU_OK_MONITOR_VLAN == res) {
				PLATFORM_MONITOR_VLAN_INTERFACES(queue_id, primary_vid);
			} else if (PROCESS_CMDU_OK_CLEAR_VLAN == res) {
				PLATFORM_STOP_MONITOR_VLAN_INTERFACES(queue_id, primary_vid);
				DMsetDefault8021QSettings(0, default_pcp);
			}
#endif
			if(res == PROCESS_CMDU_KO) {
				PLATFORM_PRINTF_WARNING("Unpexpect result for API message\n");
			}

			break;
		}
#ifdef RTK_MULTI_AP_R3
		case PLATFORM_QUEUE_EVENT_DPP_BOOTSTRAP_INFO: {
			INT8U                      info_type;
			char *                     content = NULL;
			char *                     privkey = NULL;
			size_t                     privkey_len;
			struct dpp_authentication *dpp_auth    = NULL;
			INT16U                     content_len = message_len - 1;

			PLATFORM_PRINTF_INFO("New event: DPP bootstrap info\n");

			_E1B(&p, &info_type);
			content = PLATFORM_MALLOC(content_len);
			_EnB(&p, content, content_len);

			// TODO: controller shall use DMgetDPPAuthenticationObjectByChirp
			dpp_auth = DMgetDPPAuthenticationObject();

			if (info_type == 0) { // DPP_BOOTSTRAP_PEER_URI_SET_FILENAME

				if (dpp_parse_uri(content, &(dpp_auth->peer_bi)) == 1) {
					PLATFORM_PRINTF_INFO("DPP_BOOTSTRAP_PEER_URI_SET_FILENAME OK\n");
				} else {
					PLATFORM_PRINTF_WARNING("DPP_BOOTSTRAP_PEER_URI_SET_FILENAME FAILED!\n");
				}
				// Controller should instruct ALL agents to START advertising CCE
				if (DMisController()) {
					triggerCCEAdvertisement(START_CCE_ADVERTISEMENT);
				}

			} else if (info_type == 1) { // DPP_BOOTSTRAP_OWN_PRIV_SET_FILENAME
				struct dpp_bootstrap_info *own_bi = DMgetOwnDPPBootstrapInfo();
				privkey_len = strlen(content) / 2;
				privkey     = PLATFORM_MALLOC(privkey_len);
				if (privkey_len > 0
				    && hexstr2bin(content, privkey, privkey_len) >= 0
				    && dpp_keygen(own_bi, NULL, (INT8U *)privkey, privkey_len) == 1)
					PLATFORM_PRINTF_INFO("DPP_BOOTSTRAP_OWN_PRIV_SET_FILENAME OK\n");
				else
					PLATFORM_PRINTF_WARNING("DPP_BOOTSTRAP_OWN_PRIV_SET_FILENAME FAILED!\n");

			} else {
				PLATFORM_PRINTF_WARNING("Unknown DPP bootstrap info type (%d)\n", info_type);
			}

			PLATFORM_FREE(privkey);
			PLATFORM_FREE(content);
			PLATFORM_PRINTF_INFO("DPP bootstrap info done\n");
			break;
		}
		case PLATFORM_QUEUE_EVENT_DPP_CHIRP_NOTIFICATION: {
			INT8U  enrollee_mac[6];
			INT8U *chirp_content = NULL;
			INT16U chirp_len;
			struct dpp_proxy *proxy = NULL;

			_E2B(&p, &chirp_len);
			chirp_content = (INT8U *)PLATFORM_MALLOC(chirp_len);
			_EnB(&p, chirp_content, chirp_len);
			_EnB(&p, enrollee_mac, MACADDRLEN);

			proxy = DMgetChirpProxy(chirp_content, chirp_len);

			if (proxy) {
				// TODO: Add timeout for proxy for garbage collection and reauth (30s?)
				if (0 != proxy->proxy_auth_req_msg_len) {
					send_to_dpp_broker(proxy->proxy_auth_req_msg, proxy->proxy_auth_req_msg_len);
				} else {
					PLATFORM_PRINTF_WARNING("[DPP] Duplicate chirp-notif event received, no auth_req message to be sent to broker now.\n");
				}
			} else {
				DMaddDPPChirpProxy(chirp_content, chirp_len);
				// Build TLV information
				struct DPPChirpValueTLV *dpp_chirp_notif_tlv;
				dpp_chirp_notif_tlv = (struct DPPChirpValueTLV *)PLATFORM_MALLOC(sizeof(struct DPPChirpValueTLV));
				buildDPPChirpValueTLV(dpp_chirp_notif_tlv, enrollee_mac, chirp_content, chirp_len);

				struct networkDevice *tmp = DMnetworkDeviceGet(DMcontrollerMacGet());
				if (tmp != NULL) {
					if (DMgetWFATestMode() && DMisController()) {
						PLATFORM_PRINTF_INFO("[DPP]: Device is controller and in Logo mode, ignore this chirp \n");
					} else if (0 == sendChirpNotificationMessage(tmp->receiving_interface, getNextMid(), DMcontrollerMacGet(), dpp_chirp_notif_tlv)) {
						PLATFORM_PRINTF_WARNING("[DPP] %s: Could not send Chirp Notification message\n", tmp->receiving_interface);
					}
				}

				free_1905_TLV_structure((INT8U *)dpp_chirp_notif_tlv);
			}

			PLATFORM_FREE(chirp_content);
			break;
		}
		case PLATFORM_QUEUE_EVENT_DPP_SEND_PROXIED_ENCAP: {
			INT16U frame_len;
			INT8U *frame;

			PLATFORM_PRINTF_INFO("<-- PLATFORM_QUEUE_EVENT_DPP_SEND_PROXIED_ENCAP\n");

			_E2B(&p, &frame_len);
			frame = (INT8U *)PLATFORM_MALLOC(frame_len);
			_EnB(&p, frame, frame_len);

			struct networkDevice *tmp = DMnetworkDeviceGet(DMcontrollerMacGet());
			if (tmp != NULL) {
				PLATFORM_PRINTF_INFO("Received message from broker, forwarding to controller...\n");
				struct Encap1905DPPTLV encap_1905_dpp_tlv;
				encap_dpp_frame(frame, frame_len, DMgetDPPEnrollee(), &encap_1905_dpp_tlv);
				if (0 == send1905ProxiedEncapDPPPacket(tmp->receiving_interface, getNextMid(), DMcontrollerMacGet(), &encap_1905_dpp_tlv, NULL)) {
					PLATFORM_PRINTF_WARNING("[DPP] %s: Could not send Proxied Encap message\n", tmp->receiving_interface);
				}
			}

			PLATFORM_SAFE_FREE((void **)&frame);
			break;
		}
		case PLATFORM_QUEUE_EVENT_DPP_RECONFIG_ANNOUNCEMENT: {
			PLATFORM_PRINTF_INFO("<-- PLATFORM_QUEUE_EVENT_DPP_RECONFIG_ANNOUNCEMENT\n");
			// TODO
			break;
		}
#endif // RTK_MULTI_AP_R3
		case PLATFORM_QUEUE_EVENT_AP_AUTOCONFIG_SEARCH: {
			PLATFORM_PRINTF_INFO("<-- PLATFORM_QUEUE_EVENT_AP_AUTOCONFIG_SEARCH\n");

			char **backhaul_interfaces;
			INT8U  backhaul_interfaces_nr, i;
			INT8U  ap_band = 0;
			INT16U mid     = getNextMid();

			struct interfaceInfo *x;
			backhaul_interfaces = (char **)get_backhaul_interfaces(&backhaul_interfaces_nr);
			for (i = 0; i < backhaul_interfaces_nr; i++) {
				if (!PLATFORM_STRSTR(backhaul_interfaces[i], map_vxd_prefix)) // Only send upwards towards controller
					continue;
				x = PLATFORM_GET_1905_INTERFACE_INFO(backhaul_interfaces[i]);
				if (IS_IEEE_802_11_2_4_GHZ(x->interface_type, x->interface_type_data.ieee80211.rf_band)) {
					ap_band = IEEE80211_FREQUENCY_BAND_2_4_GHZ;
				} else if (IS_IEEE_802_11_5_GHZ(x->interface_type, x->interface_type_data.ieee80211.rf_band)) {
					ap_band = IEEE80211_FREQUENCY_BAND_5_GHZ;
				} else {
					PLATFORM_PRINTF_WARNING("Unknown interface type %02x\n", x->interface_type);
					PLATFORM_FREE_1905_INTERFACE_INFO(x);
					continue;
				}
				PLATFORM_FREE_1905_INTERFACE_INFO(x);
				send1905APAutoconfigurationSearchPacket(backhaul_interfaces[i], mid, ap_band);
			}
			PLATFORM_FREE(backhaul_interfaces);
			break;
		}
#ifdef RTK_MULTI_AP_R4
		case PLATFORM_QUEUE_EVENT_WPAS_RECONFIG: {
			PLATFORM_PRINTF_INFO("<-- PLATFORM_QUEUE_EVENT_WPAS_RECONFIG\n");
			delayTriggerReconfig(20, queue_id);
			break;
		}
#endif // RTK_MULTI_AP_R4
		default: {
#ifdef RTK_MULTI_AP_R2
			INT8U result = 0;
			if (MULTI_AP_PROFILE_2 <= DMgetMultiAPProfile()) {
				result = processMultiAPR2Events(message_type, message_len, p);
			}
			if (result) {
				PLATFORM_PRINTF_WARNING("Unknown queue message type (%d)\n", message_type);
			}
#else
			PLATFORM_PRINTF_WARNING("Unknown queue message type (%d)\n", message_type);
#endif
			break;
		}
		}

		pthread_mutex_lock(&map_core_mutex);
		alive_check = core_thread_flag;
		pthread_mutex_unlock(&map_core_mutex);
	}
	PLATFORM_FREE(queue_message);
	PLATFORM_PRINTF_INFO("MAP Core Closed!!\n");
	return 0;
}

void terminate1905AL()
{
	CLOSE_ALME_THREAD();
	pthread_mutex_lock(&map_core_mutex);
	core_thread_flag = 0;
	pthread_mutex_unlock(&map_core_mutex);
}

#ifdef RTK_MULTI_AP_R2
INT8U processMultiAPR2Events(INT8U message_type, INT16U message_len, INT8U *p)
{
	switch (message_type) {
	case PLATFORM_QUEUE_EVENT_CAC_STATUS_REPORT_NOTIFICATION: {
			INT8U radio_id[6];
			INT8U cac_status, channel, op_class;

			_EnB(&p, radio_id, 6);
			_E1B(&p, &cac_status);
			_E1B(&p, &channel);
			_E1B(&p, &op_class);

			DMupdateCACRecord(radio_id, op_class, channel, cac_status);
			PLATFORM_PRINTF_WARNING("CAC STATUS EVENT\n");
			DMdumpCACStatus();
			break;
		}

		case PLATFORM_QUEUE_EVENT_CAC_COMPLETION_REPORT_NOTIFICATION: {
			PLATFORM_PRINTF_WARNING("PLATFORM_QUEUE_EVENT_CAC_COMPLETION_REPORT_NOTIFICATION\n");
			void * tmp_ptr = parse_1905_TLV_from_packet(p);
			if (!tmp_ptr) {
				PLATFORM_PRINTF_ERROR("Parse 1905 tlv from packets returns NULL, fail to process queue event. \n");
				break;
			}
			if (*(INT8U *)tmp_ptr != TLV_TYPE_CAC_COMPLETION_REPORT) {
				PLATFORM_PRINTF_ERROR("CAC completion report notification content error, fail to process queue event. \n");
				free_1905_TLV_structure((INT8U *)tmp_ptr);
				break;
			}
			struct CACCompletionReportTLV *t = (struct CACCompletionReportTLV *)tmp_ptr;
			// struct networkDevice *tmp = DMnetworkDeviceGet(DMcontrollerMacGet());

			// if (tmp != NULL) {
			// 	if (0 == sendMultiAPClientSteeringBTMReportPacket(tmp->receiving_interface, getNextMid(), DMcontrollerMacGet(), &response)) {
			// 		PLATFORM_PRINTF_WARNING("[%s] Could not send Client Steering BTM Report message\n", tmp->receiving_interface);
			// 	}
			// } else {
			// 	PLATFORM_PRINTF_WARNING("Could not send CAC St Report message because target not found\n");
			// }
			PLATFORM_PRINTF_DEBUG("radio_nr: %d\n", t->radio_nr);
			INT8U k, l;
			for (k = 0; k < t->radio_nr; k++) {
				PLATFORM_PRINTF_DEBUG("radio_unique_identifier: %02x:%02x:%02x:%02x:%02x:%02x\n",
				                      t->radios[k].radio_unique_identifier[0],
				                      t->radios[k].radio_unique_identifier[1],
				                      t->radios[k].radio_unique_identifier[2],
				                      t->radios[k].radio_unique_identifier[3],
				                      t->radios[k].radio_unique_identifier[4],
				                      t->radios[k].radio_unique_identifier[5]);
				PLATFORM_PRINTF_DEBUG("op_class: %d\n", t->radios[k].op_class);
				PLATFORM_PRINTF_DEBUG("channel: %d\n", t->radios[k].channel);
				PLATFORM_PRINTF_DEBUG("flags: %d\n", t->radios[k].flags);
				PLATFORM_PRINTF_DEBUG("pairs_nr: %d\n", t->radios[k].pairs_nr);
				for (l = 0; l < t->radios[k].pairs_nr; l++) {
					PLATFORM_PRINTF_DEBUG("pairs_op_class: %d\n", t->radios[k].pairs[l].pairs_op_class);
					PLATFORM_PRINTF_DEBUG("pairs_channel: %d\n", t->radios[k].pairs[l].pairs_channel);
				}
			}

			struct networkDevice *tmp = DMnetworkDeviceGet(DMcontrollerMacGet());

			if (tmp != NULL) {
				if (0 == sendMultiAPChannelPreferenceReportPacket(tmp->receiving_interface, getNextMid(), DMcontrollerMacGet(), (INT8U *)t, 1)) {
					PLATFORM_PRINTF_WARNING("[%s] Could not send CAC_COMPLETION_REPORT message\n", tmp->receiving_interface);
				}
			} else {
				PLATFORM_PRINTF_WARNING("Could not send CAC_COMPLETION_REPORT message because controller not found\n");
			}
			free_1905_TLV_structure((INT8U *)t);
			break;
		}

		case PLATFORM_QUEUE_EVENT_CHANNEL_SCAN_RESULT_NOTIFICATION: {
			PLATFORM_PRINTF_DEBUG("PLATFORM_QUEUE_EVENT_CHANNEL_SCAN_RESULT_NOTIFICATION\n");
			INT16U mid;
			mid = getNextMid();

			INT8U band;
			_E1B(&p, &band);

			if ((map_radio_name_5gl[4] - '0') == band) {
				if ((DMgetChannelScanCompleted(_2G) && !DMisTribandDevice()) || (DMgetChannelScanCompleted(_2G) && DMgetChannelScanCompleted(_5GH))) {
					//store results before sending in case there is a non-fresh scan channel scan request in the future
					update_stored_channel_scan_results(p);

					if (DMgetOnBootScanFlag()) {
						//if it is result from on boot scan, clear flag.
						DMsetOnBootScanFlag(0);
					} else {
						struct networkDevice *tmp = DMnetworkDeviceGet(DMcontrollerMacGet());
						if (tmp != NULL) {
							if (0 == sendChannelScanReportPacket(tmp->receiving_interface, mid, DMcontrollerMacGet(), 1)) {
								PLATFORM_PRINTF_ERROR("[%s] Could not send Channel Scan Result message\n", tmp->receiving_interface);
							}
						} else {
							PLATFORM_PRINTF_WARNING("Could not send Channel Scan Result message because target not found\n");
						}
					}

					PLATFORM_PRINTF_DEBUG("Channel Scan 5GL finished...\n");

					//clear flags
					DMclearChannelScanCompleted(_2G);
					DMclearChannelScanCompleted(_5GL);
					DMclearChannelScanCompleted(_5GH);
				} else {
					//buffer the result, set completed flag, and wait for the result from the other band.
					PLATFORM_PRINTF_DEBUG("Channel Scan 5GL finished... Storing result to database and waiting for other results...\n");
					DMappendChannelScanResultBuffer(message_len - 1, p);
					DMsetChannelScanCompleted(_5GL);
					if (!DMisTribandDevice()) {
						DMsetChannelScanCompleted(_5GH);
					}
				}
			} else if ((map_radio_name_5gh[4] - '0') == band) {
				if (DMgetChannelScanCompleted(_2G) && DMgetChannelScanCompleted(_5GL)) {
					update_stored_channel_scan_results(p);

					if (DMgetOnBootScanFlag()) {
						//if it is result from on boot scan, clear flag.
						DMsetOnBootScanFlag(0);
					} else {
						struct networkDevice *tmp = DMnetworkDeviceGet(DMcontrollerMacGet());
						if (tmp != NULL) {
							if (0 == sendChannelScanReportPacket(tmp->receiving_interface, mid, DMcontrollerMacGet(), 1)) {
								PLATFORM_PRINTF_ERROR("[%s] Could not send Channel Scan Result message\n", tmp->receiving_interface);
							}
						} else {
							PLATFORM_PRINTF_WARNING("Could not send Channel Scan Result message because target not found\n");
						}
					}
					PLATFORM_PRINTF_DEBUG("Channel Scan 5GH finished...\n");

					//clear flags
					DMclearChannelScanCompleted(_2G);
					DMclearChannelScanCompleted(_5GL);
					DMclearChannelScanCompleted(_5GH);
				} else {
					//buffer the result, set completed flag, and wait for the result from the other band.
					PLATFORM_PRINTF_DEBUG("Channel Scan 5GH finished... Storing result to database and waiting for other results...\n");
					DMappendChannelScanResultBuffer(message_len - 1, p);
					DMsetChannelScanCompleted(_5GH);
				}
			} else if ((map_radio_name_24g[4] - '0') == band) {
				if (DMgetChannelScanCompleted(_5GH) && DMgetChannelScanCompleted(_5GL)) {
					update_stored_channel_scan_results(p);

					if (DMgetOnBootScanFlag()) {
						//if it is result from on boot scan, clear flag but do not send report immediately
						DMsetOnBootScanFlag(0);
					} else {
						struct networkDevice *tmp = DMnetworkDeviceGet(DMcontrollerMacGet());
						if (tmp != NULL) {
							if (0 == sendChannelScanReportPacket(tmp->receiving_interface, mid, DMcontrollerMacGet(), 1)) {
								PLATFORM_PRINTF_ERROR("[%s] Could not send Channel Scan Result message\n", tmp->receiving_interface);
							}
						} else {
							PLATFORM_PRINTF_WARNING("Could not send Channel Scan Result message because target not found\n");
						}
					}
					PLATFORM_PRINTF_DEBUG("Channel Scan 2G finished...\n");

					//clear flags
					DMclearChannelScanCompleted(_2G);
					DMclearChannelScanCompleted(_5GH);
					DMclearChannelScanCompleted(_5GL);
				} else {
					//buffer the result, set completed flag, and wait for the result from the other band.
					PLATFORM_PRINTF_DEBUG("Channel Scan 2G finished... Storing result to database and waiting for 5G results...\n");
					DMappendChannelScanResultBuffer(message_len - 1, p);
					DMsetChannelScanCompleted(_2G);
				}
			}
			break;
		}

		case PLATFORM_QUEUE_EVENT_ASSOCIATION_STATUS_NOTIFICATION: {
			INT16U mid;
			mid = getNextMid();

			struct AssociationStatusNotificationBssInfo associationStatusNotificationBssInfo;
			PLATFORM_MEMSET(&associationStatusNotificationBssInfo, 0, sizeof(struct AssociationStatusNotificationBssInfo));

			_EnB(&p, associationStatusNotificationBssInfo.bssid, 6);
			_E1B(&p, &associationStatusNotificationBssInfo.association_allowance_status);
			PLATFORM_PRINTF_DEBUG("Association Status Notification\n");
			PLATFORM_PRINTF_DEBUG("BSSID: %02x%02x%02x%02x%02x%02x\n", associationStatusNotificationBssInfo.bssid[0], associationStatusNotificationBssInfo.bssid[1], associationStatusNotificationBssInfo.bssid[2],
			                      associationStatusNotificationBssInfo.bssid[3], associationStatusNotificationBssInfo.bssid[4], associationStatusNotificationBssInfo.bssid[5]);
			PLATFORM_PRINTF_DEBUG("Status: %d\n", associationStatusNotificationBssInfo.association_allowance_status);

			struct networkDevice *tmp = DMnetworkDeviceGet(DMcontrollerMacGet());

			if (tmp != NULL) {
				if (0 == sendMultiAPAssociationStatusNotificationMessage(tmp->receiving_interface, mid, DMcontrollerMacGet(), 1, 1, &associationStatusNotificationBssInfo)) {
					PLATFORM_PRINTF_WARNING("[%s] Could not send Association Status Notification message\n", tmp->receiving_interface);
				}
			} else {
				PLATFORM_PRINTF_WARNING("Could not send Association Status Notification message because target not found\n");
			}
			break;
		}

		case PLATFORM_QUEUE_EVENT_TUNNELED_MESSAGE: {
			INT16U mid, payload_len = 0;
			mid = getNextMid();

			INT8U              mac_addr[6]           = { 0 };
			INT8U              tunneled_message_type = 0;
			struct TunneledTLV tunneledTlv;

			_EnB(&p, mac_addr, 6);
			_E1B(&p, &tunneled_message_type);
			_EnB(&p, &payload_len, 2); // Driver inserted those with host endian

			if (payload_len) {
				tunneledTlv.tlv_type   = TLV_TYPE_TUNNELED;
				tunneledTlv.tlv_length = payload_len;
				tunneledTlv.payload    = (INT8U *)PLATFORM_MALLOC(sizeof(INT8U) * payload_len);
				_EnB(&p, tunneledTlv.payload, payload_len);
			} else {
				PLATFORM_PRINTF_WARNING("No payload len found for Tunneled message type %02x\n", tunneled_message_type);
				break;
			}
			PLATFORM_PRINTF_DEBUG("Tunneled message\n");
			PLATFORM_PRINTF_DEBUG("Mac addr: %02x%02x%02x%02x%02x%02x\n", mac_addr[0], mac_addr[1], mac_addr[2],
			                      mac_addr[3], mac_addr[4], mac_addr[5]);
			PLATFORM_PRINTF_DEBUG("Message type: %d\n", tunneled_message_type);
			PLATFORM_PRINTF_DEBUG("Payload len: %d\n", payload_len);

			struct networkDevice *tmp = DMnetworkDeviceGet(DMcontrollerMacGet());

			if (tmp != NULL) {
				if (0 == sendMultiAPTunneledMessage(tmp->receiving_interface, mid, DMcontrollerMacGet(), 1, mac_addr, tunneled_message_type, 1, &tunneledTlv)) {
					PLATFORM_PRINTF_WARNING("[%s] Could not send Tunneled message\n", tmp->receiving_interface);
				}
			} else {
				PLATFORM_PRINTF_WARNING("Could not send Tunneled message because target not found\n");
			}

			if (tunneledTlv.tlv_length) {
				PLATFORM_FREE(tunneledTlv.payload);
			}
			break;
		}

		case PLATFORM_QUEUE_EVENT_FAILED_CONNECTION_MESSAGE: {
			INT16U mid = getNextMid();

			INT8U  mac_addr[6] = { 0 };
			INT16U status_code = 0;
			INT16U reason_code = 0;
			bool need_ack = 1;

			_EnB(&p, mac_addr, 6);
			_E2B(&p, &status_code);
			_E2B(&p, &reason_code);

			PLATFORM_PRINTF_DEBUG("Failed Connection message\n");
			PLATFORM_PRINTF_DEBUG("Mac addr: %02x%02x%02x%02x%02x%02x\n", mac_addr[0], mac_addr[1], mac_addr[2],
			                      mac_addr[3], mac_addr[4], mac_addr[5]);
			PLATFORM_PRINTF_DEBUG("Status Code: %d\n", status_code);
			PLATFORM_PRINTF_DEBUG("Reason Code: %d\n", reason_code);

			struct networkDevice *tmp = DMnetworkDeviceGet(DMcontrollerMacGet());

			if (tmp != NULL) {
				if (DMgetWFATestMode())
					need_ack = 0;
				if (0 == sendFailedConnectionMessage(tmp->receiving_interface, mid, DMcontrollerMacGet(), need_ack, mac_addr, status_code, reason_code)) {
					PLATFORM_PRINTF_WARNING("[%s] Could not send Fail Connection message\n", tmp->receiving_interface);
				}
			} else {
				PLATFORM_PRINTF_WARNING("Could not send Fail Connection message because target not found\n");
			}

			break;
		}

		case PLATFORM_QUEUE_EVENT_CLIENT_DISASSOCIATION_STATS: {
			INT16U mid = getNextMid();

			INT8U  mac_addr[6] = { 0 };
			INT16U reason_code = 0;

			_EnB(&p, mac_addr, 6);
			_E2B(&p, &reason_code);

			struct AssociatedSTATrafficStatsTLV *stats_tlv = (struct AssociatedSTATrafficStatsTLV *)parse_1905_TLV_from_packet(p);

			PLATFORM_PRINTF_DEBUG("Disassociation Stats message\n");
			PLATFORM_PRINTF_DEBUG("Mac addr: %02x%02x%02x%02x%02x%02x\n", mac_addr[0], mac_addr[1], mac_addr[2],
			                      mac_addr[3], mac_addr[4], mac_addr[5]);
			PLATFORM_PRINTF_DEBUG("Reason Code: %d\n", reason_code);
			visit_1905_TLV_structure((INT8U *)stats_tlv, print_callback, PLATFORM_PRINTF_DEBUG, "");

			struct networkDevice *tmp = DMnetworkDeviceGet(DMcontrollerMacGet());

			if (tmp != NULL) {
				if (0 == sendClientDisassociationStats(tmp->receiving_interface, mid, DMcontrollerMacGet(), 1, mac_addr, reason_code, stats_tlv)) {
					PLATFORM_PRINTF_WARNING("[%s] Could not send Client Disassociation Stats message\n", tmp->receiving_interface);
				}
			} else {
				PLATFORM_PRINTF_WARNING("Could not send Client Disassociation Stats message because target not found\n");
			}

			free_1905_TLV_structure((INT8U *)stats_tlv);

			break;
		}
		default: {
			return 1;
		}
	}

	return 0;
}
#endif
