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


#ifndef _AL_UTILS_H_
#define _AL_UTILS_H_
#include "map_initialization.h"
#include "1905_cmdus.h"
#include "multi_ap_tlvs.h"
#include "multi_ap_tlvs_r2.h"
#include "multi_ap_tlvs_r3.h"
#include "multi_ap_tlvs_r4.h"

#define ON_BOOT_CHANNEL_SCAN_RESULT_FILE "/tmp/map_channel_scan.txt"

#define ACK_TIMEOUT_THRESHOLD 5

// "MIDs" are "message IDs" used inside 1905 protocol messages. They must be
// monotonically increased as explained in "Section 7.8"
//
INT16U getNextMid(void);

#define INTERFACE_BAND_UNKNOWN     0x00
#define INTERFACE_BAND_2G          0x01
#define INTERFACE_BAND_5GL         0x02
#define INTERFACE_BAND_5GH         0x04

#define DEVICE_FULLY_CONFIGURED    0x07

typedef struct _WaitingMsg {
	timer_t timer_id;
	INT16U  mid;
	INT8U   timeout_count;
	char    interface_name[20];
	INT8U   dst_mac_address[6];
	INT8U **streams;
	INT16U *streams_lens;
} WaitingMsg;

typedef struct _WaitingMsgTimerData {
	INT16U  mid;
	timer_t timer_id;
} WaitingMsgTimerData;

INT16U get_mid_from_stream(INT8U *stream);
void   set_mid_for_streams(INT8U **streams, INT16U mid);

INT16U get_message_type_from_stream(INT8U *stream);

INT8U waiting_msgs_add(char *interface_name, INT8U dst_mac_address[], INT8U **streams, INT16U *streams_lens);

INT8U waiting_msgs_ack(INT16U mid);

void  waiting_msgs_timeout(union sigval s);

char** get_backhaul_interfaces(INT8U* backhaul_interfaces_nr);

void updateConfigData(INT8U config_data_nr, struct radio_config_data *config_data);

void freeConfigData();

char *generate_timestamp();

INT8U delete_agent(INT8U *agent_sta_mac, INT8U *bssid, INT8U send_disassoc, INT8U role, char *exclude_interface);

INT8U delete_agent_with_al_mac(INT8U *agent_al_mac, INT8U send_disassoc, char *exclude_interface);

INT8U calculate_mic(INT8U *gtk, struct CMDU *cmdu, INT8U is_forwarding, struct MICTLV *mic_tlv);

INT8U update_stored_channel_scan_results(INT8U* data_stream);

INT8U get_band_from_single_op_class_with_domain(INT8U op_class, int reg_domain);

INT8U get_band_from_single_op_class(INT8U op_class);

INT8U get_band_from_op_classes(struct OperatingClass *op_classes, INT8U op_class_nr);

INT8U validateChannelPreferenceReport(struct ChannelPreferenceTLV p);

char * trimInterfaceNameVID(char *name, INT16U *vid, INT8U *port);

void convertToGlobalOpClass(struct CMDU *c);

void convertToLocalOpClass(struct CMDU *c);

INT8U get_channel_center_frequency_index(INT8U channel);

INT8U get_160m_channel_center_frequency_index(INT8U channel);

INT8U obtain_channel_info(char* radio_name, INT8U *channel, int *bandwidth, int *sideband, INT8U *transmit_power);

int get_center_frequency(char *interface_name);

#ifdef RTK_MULTI_AP_R2
INT8U checkTargetDeviceProfile2TLVCapability(INT8U *target_al_mac);
#endif

void sscanf_hex(INT8U *src, INT8U *hex, INT16U len);

void encap_dpp_frame(INT8U *dpp_frame, INT16U dpp_frame_len, INT8U* enrollee_mac, struct Encap1905DPPTLV *encap_1905_dpp_tlv);

char *mac2str(INT8U *mac_addr);

INT8U rssi2rcpi(INT8U rssi);

INT8U rcpi2rssi(INT8U rcpi);

#endif
