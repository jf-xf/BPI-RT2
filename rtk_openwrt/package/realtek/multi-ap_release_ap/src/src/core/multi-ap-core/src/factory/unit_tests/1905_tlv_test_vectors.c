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


#include "1905_tlvs.h"


// This file contains test vectors than can be used to check the
// "parse_1905_TLV_from_packet()" and "forge_1905_TLV_from_structure()"
// functions
//
// Each test vector is made up of three variables:
//
//   - A TLV structure
//   - An array of bits representing the network packet
//   - An variable holding the length of the packet
//
// Note that some test vectors can be used to test both functions, while others
// can only be used to test "forge_1905_TLV_from_structure()" or
// "parse_1905_TLV_from_packet()":
//
//   - Test vectors marked with "TLV --> packet" can only be used to test the
//     "forge_1905_TLV_from_structure()" function.
//
//   - Test vectors marked with "TLV <-- packet" can only be used to test the
//     "parse_1905_TLV_from_packet()" function.
//
//   - All the other test vectors are marked with "TLC <--> packet", meaning
//     they can be used to test  both functions.
//
// The reason this is happening is that, according to the standard, sometimes
// bits are ignored/changed/forced-to-a-value when forging a packet. Thus, not
// all test vectors are "inversible" (ie. forge(parse(stream)) != x)
//
// This is how you use these test vectors:
//
//   A) stream = forge_1905_TLV_from_structure(tlv_xxx, &stream_len);
//
//   B) tlv = parse_1905_TLV_from_packet(stream_xxx);
//


////////////////////////////////////////////////////////////////////////////////
//// Test vector 001 (TLV <--> packet)
////////////////////////////////////////////////////////////////////////////////

struct linkMetricQueryTLV x1905_tlv_structure_001 =
{
    .tlv_type          = TLV_TYPE_LINK_METRIC_QUERY,
    .destination       = LINK_METRIC_QUERY_TLV_SPECIFIC_NEIGHBOR,
    .specific_neighbor = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05},
    .link_metrics_type = LINK_METRIC_QUERY_TLV_RX_LINK_METRICS_ONLY,
};

INT8U x1905_tlv_stream_001[] =
{
    0x08,
    0x00, 0x08,
    0x01,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x01
};

INT16U x1905_tlv_stream_len_001 = 11;


////////////////////////////////////////////////////////////////////////////////
//// Test vector 002 (TLV --> packet)
////////////////////////////////////////////////////////////////////////////////

struct linkMetricQueryTLV x1905_tlv_structure_002 =
{
    .tlv_type          = TLV_TYPE_LINK_METRIC_QUERY,
    .destination       = LINK_METRIC_QUERY_TLV_ALL_NEIGHBORS,
    .specific_neighbor = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05},
    .link_metrics_type = LINK_METRIC_QUERY_TLV_BOTH_TX_AND_RX_LINK_METRICS,
};

INT8U x1905_tlv_stream_002[] =
{
    0x08,
    0x00, 0x02,
    0x00,
    0x02
};

INT16U x1905_tlv_stream_len_002 = 5;

// INT8U x1905_tlv_stream_002[] =
// {
//     0x08,
//     0x00, 0x08,
//     0x00,
//     0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
//     0x02
// };

// INT16U x1905_tlv_stream_len_002 = 11;


////////////////////////////////////////////////////////////////////////////////
//// Test vector 003 (TLV <-- packet)
////////////////////////////////////////////////////////////////////////////////

struct linkMetricQueryTLV x1905_tlv_structure_003 =
{
    .tlv_type          = TLV_TYPE_LINK_METRIC_QUERY,
    .destination       = LINK_METRIC_QUERY_TLV_ALL_NEIGHBORS,
    .specific_neighbor = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    .link_metrics_type = LINK_METRIC_QUERY_TLV_BOTH_TX_AND_RX_LINK_METRICS,
};

INT8U x1905_tlv_stream_003[] =
{
    0x08,
    0x00, 0x02,
    0x00,
    0x02
};

INT16U x1905_tlv_stream_len_003 = 11;


////////////////////////////////////////////////////////////////////////////////
//// Test vector 004 (TLV <--> packet)
////////////////////////////////////////////////////////////////////////////////

struct transmitterLinkMetricTLV x1905_tlv_structure_004 =
{
    .tlv_type                    = TLV_TYPE_TRANSMITTER_LINK_METRIC,
    .local_al_address            = {0x01, 0x02, 0x03, 0x01, 0x02, 0x03},
    .neighbor_al_address         = {0x0a, 0x0b, 0x0c, 0x0a, 0x0b, 0x0c},
    .transmitter_link_metrics_nr = 1,
    .transmitter_link_metrics    =
        (struct _transmitterLinkMetricEntries[]){
            {
                .local_interface_address    = {0x21, 0x22, 0x23, 0x24, 0x25, 0x26},
                .neighbor_interface_address = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36},
                .intf_type                  = MEDIA_TYPE_IEEE_802_11G_2_4_GHZ,
                .bridge_flag                = 0,
                .packet_errors              = 134,
                .transmitted_packets        = 1543209,
                .mac_throughput_capacity    = 400,
                .link_availability          = 50,
                .phy_rate                   = 520,
            },
        },
};

INT8U x1905_tlv_stream_004[] =
{
    0x09,
    0x00, 0x29,
    0x01, 0x02, 0x03, 0x01, 0x02, 0x03,
    0x0a, 0x0b, 0x0c, 0x0a, 0x0b, 0x0c,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x01, 0x01,
    0x00,
    0x00, 0x00, 0x00, 0x86,
    0x00, 0x17, 0x8c, 0x29,
    0x01, 0x90,
    0x00, 0x32,
    0x02, 0x08
};

INT16U x1905_tlv_stream_len_004 = 44;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 005 (TLV <--> packet)
////
////////////////////////////////////////////////////////////////////////////////

struct transmitterLinkMetricTLV x1905_tlv_structure_005 =
{
    .tlv_type                    = TLV_TYPE_TRANSMITTER_LINK_METRIC,
    .local_al_address            = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
    .neighbor_al_address         = {0x02, 0x02, 0x02, 0x02, 0x02, 0x02},
    .transmitter_link_metrics_nr = 2,
    .transmitter_link_metrics    =
        (struct _transmitterLinkMetricEntries[]){
            {
                .local_interface_address    = {0x03, 0x03, 0x03, 0x03, 0x03, 0x03},
                .neighbor_interface_address = {0x04, 0x04, 0x04, 0x04, 0x04, 0x04},
                .intf_type                  = MEDIA_TYPE_MOCA_V1_1,
                .bridge_flag                = 1,
                .packet_errors              = 0,
                .transmitted_packets        = 1000,
                .mac_throughput_capacity    = 900,
                .link_availability          = 80,
                .phy_rate                   = 1000,
            },
            {
                .local_interface_address    = {0x05, 0x05, 0x05, 0x05, 0x05, 0x05},
                .neighbor_interface_address = {0x06, 0x06, 0x06, 0x06, 0x06, 0x06},
                .intf_type                  = MEDIA_TYPE_IEEE_802_11AC_5_GHZ,
                .bridge_flag                = 0,
                .packet_errors              = 7,
                .transmitted_packets        = 7,
                .mac_throughput_capacity    = 900,
                .link_availability          = 80,
                .phy_rate                   = 1000,
            },
        },
};

INT8U x1905_tlv_stream_005[] =
{
    0x09,
    0x00, 0x46,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
    0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
    0x03, 0x00,
    0x01,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x03, 0xe8,
    0x03, 0x84,
    0x00, 0x50,
    0x03, 0xe8,
    0x05, 0x05, 0x05, 0x05, 0x05, 0x05,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06,
    0x01, 0x05,
    0x00,
    0x00, 0x00, 0x00, 0x07,
    0x00, 0x00, 0x00, 0x07,
    0x03, 0x84,
    0x00, 0x50,
    0x03, 0xe8,
};

INT16U x1905_tlv_stream_len_005 = 73;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 006 (TLV <--> packet)
////
////////////////////////////////////////////////////////////////////////////////

struct receiverLinkMetricTLV x1905_tlv_structure_006 =
{
    .tlv_type                    = TLV_TYPE_RECEIVER_LINK_METRIC,
    .local_al_address            = {0x01, 0x02, 0xff, 0x01, 0x02, 0x03},
    .neighbor_al_address         = {0x0a, 0x0b, 0x0c, 0x0a, 0x77, 0x0c},
    .receiver_link_metrics_nr    = 1,
    .receiver_link_metrics       =
        (struct _receiverLinkMetricEntries[]){
            {
                .local_interface_address    = {0x21, 0x22, 0x00, 0x24, 0x00, 0x26},
                .neighbor_interface_address = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36},
                .intf_type                  = MEDIA_TYPE_IEEE_802_11AF,
                .packet_errors              = 27263110,
                .packets_received           = 27263111,
                .rssi                       = 2,
            },
        },
};

INT8U x1905_tlv_stream_006[] =
{
    0x0a,
    0x00, 0x23,
    0x01, 0x02, 0xff, 0x01, 0x02, 0x03,
    0x0a, 0x0b, 0x0c, 0x0a, 0x77, 0x0c,
    0x21, 0x22, 0x00, 0x24, 0x00, 0x26,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x01, 0x07,
    0x01, 0xa0, 0x00, 0x86,
    0x01, 0xa0, 0x00, 0x87,
    0x02,
};

INT16U x1905_tlv_stream_len_006 = 38;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 007 (TLV <--> packet)
////
////////////////////////////////////////////////////////////////////////////////

struct receiverLinkMetricTLV x1905_tlv_structure_007 =
{
    .tlv_type                    = TLV_TYPE_RECEIVER_LINK_METRIC,
    .local_al_address            = {0x01, 0x02, 0xff, 0x01, 0x02, 0x03},
    .neighbor_al_address         = {0x0a, 0x0b, 0x0c, 0x0a, 0x77, 0x0c},
    .receiver_link_metrics_nr    = 2,
    .receiver_link_metrics       =
        (struct _receiverLinkMetricEntries[]){
            {
                .local_interface_address    = {0x21, 0x22, 0x00, 0x24, 0x00, 0x26},
                .neighbor_interface_address = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36},
                .intf_type                  = MEDIA_TYPE_IEEE_802_11AF,
                .packet_errors              = 27263110,
                .packets_received           = 27263111,
                .rssi                       = 2,
            },
            {
                .local_interface_address    = {0xff, 0x22, 0x00, 0x24, 0x00, 0x26},
                .neighbor_interface_address = {0xff, 0x32, 0x33, 0x34, 0x35, 0x36},
                .intf_type                  = MEDIA_TYPE_IEEE_802_11AF,
                .packet_errors              = 27263110,
                .packets_received           = 27263111,
                .rssi                       = 2,
            },
        },
};

INT8U x1905_tlv_stream_007[] =
{
    0x0a,
    0x00, 0x3a,
    0x01, 0x02, 0xff, 0x01, 0x02, 0x03,
    0x0a, 0x0b, 0x0c, 0x0a, 0x77, 0x0c,
    0x21, 0x22, 0x00, 0x24, 0x00, 0x26,
    0x31, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x01, 0x07,
    0x01, 0xa0, 0x00, 0x86,
    0x01, 0xa0, 0x00, 0x87,
    0x02,
    0xff, 0x22, 0x00, 0x24, 0x00, 0x26,
    0xff, 0x32, 0x33, 0x34, 0x35, 0x36,
    0x01, 0x07,
    0x01, 0xa0, 0x00, 0x86,
    0x01, 0xa0, 0x00, 0x87,
    0x02,
};

INT16U x1905_tlv_stream_len_007 = 61;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 008 (TLV <--> packet)
////
////////////////////////////////////////////////////////////////////////////////

struct alMacAddressTypeTLV x1905_tlv_structure_008 =
{
    .tlv_type                    = TLV_TYPE_AL_MAC_ADDRESS_TYPE,
    .al_mac_address              = {0x01, 0x02, 0xf2, 0x01, 0x02, 0x00},
};

INT8U x1905_tlv_stream_008[] =
{
    0x01,
    0x00, 0x06,
    0x01, 0x02, 0xf2, 0x01, 0x02, 0x00,
};

INT16U x1905_tlv_stream_len_008 = 9;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 009 (TLV <--> packet)
////
////////////////////////////////////////////////////////////////////////////////

struct macAddressTypeTLV x1905_tlv_structure_009 =
{
    .tlv_type                    = TLV_TYPE_MAC_ADDRESS_TYPE,
    .mac_address                 = {0xff, 0xf2, 0x04, 0xfa, 0x00, 0xab},
};

INT8U x1905_tlv_stream_009[] =
{
    0x02,
    0x00, 0x06,
    0xff, 0xf2, 0x04, 0xfa, 0x00, 0xab,
};

INT16U x1905_tlv_stream_len_009 = 9;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 010 (TLV <--> packet)
////
////////////////////////////////////////////////////////////////////////////////

struct deviceInformationTypeTLV x1905_tlv_structure_010 =
{
    .tlv_type                    = TLV_TYPE_DEVICE_INFORMATION_TYPE,
    .al_mac_address              = {0x04, 0x02, 0xff, 0x01, 0x02, 0x03},
    .local_interfaces_nr         = 2,
    .local_interfaces            =
        (struct _localInterfaceEntries[]){
            {
                .mac_address                                                       = {0x21, 0x22, 0x00, 0x24, 0x25, 0x26},
                .media_type                                                        = MEDIA_TYPE_IEEE_802_11AF,
                .media_specific_data_size                                          = 10,
                .media_specific_data.ieee80211.network_membership                  = {0x01, 0x01, 0x01, 0x02, 0x02, 0x02},
                .media_specific_data.ieee80211.role                                = IEEE80211_SPECIFIC_INFO_ROLE_WIFI_P2P_CLIENT,
                .media_specific_data.ieee80211.ap_channel_band                     = 0x05,
                .media_specific_data.ieee80211.ap_channel_center_frequency_index_1 = 0x0a,
                .media_specific_data.ieee80211.ap_channel_center_frequency_index_2 = 0x0b,
            },
            {
                .mac_address                                     = {0x21, 0x22, 0x00, 0x24, 0x25, 0x27},
                .media_type                                      = MEDIA_TYPE_IEEE_1901_WAVELET,
                .media_specific_data_size                        = 7,
                .media_specific_data.ieee1901.network_identifier = {0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0xff},
            },
        },
};

INT8U x1905_tlv_stream_010[] =
{
    0x03,
    0x00, 0x2a,
    0x04, 0x02, 0xff, 0x01, 0x02, 0x03,
    0x02,
    0x21, 0x22, 0x00, 0x24, 0x25, 0x26,
    0x01, 0x07,
    0x0a,
    0x01, 0x01, 0x01, 0x02, 0x02, 0x02,
    0x80,
    0x05,
    0x0a,
    0x0b,
    0x21, 0x22, 0x00, 0x24, 0x25, 0x27,
    0x02, 0x00,
    0x07,
    0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0xff,
};

INT16U x1905_tlv_stream_len_010 = 45;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 011 (TLV <--> packet)
////
////////////////////////////////////////////////////////////////////////////////

struct deviceBridgingCapabilityTLV x1905_tlv_structure_011 =
{
    .tlv_type                    = TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES,
    .bridging_tuples_nr          = 2,
    .bridging_tuples             =
        (struct _bridgingTupleEntries[]){
            {
                .bridging_tuple_macs_nr  = 2,
                .bridging_tuple_macs     =
                    (struct _bridgingTupleMacEntries[]){
                        {
                            .mac_address     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
                        },
                        {
                            .mac_address     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
                        },
                    }
            },
            {
                .bridging_tuple_macs_nr  = 3,
                .bridging_tuple_macs     =
                    (struct _bridgingTupleMacEntries[]){
                        {
                            .mac_address     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x11},
                        },
                        {
                            .mac_address     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x12},
                        },
                        {
                            .mac_address     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x13},
                        },
                    }
            },
        },
};

INT8U x1905_tlv_stream_011[] =
{
    0x04,
    0x00, 0x21,
    0x02,
    0x02,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
    0x03,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x11,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x12,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x13,
};

INT16U x1905_tlv_stream_len_011 = 36;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 012 (TLV <--> packet)
////
////////////////////////////////////////////////////////////////////////////////

struct deviceBridgingCapabilityTLV x1905_tlv_structure_012 =
{
    .tlv_type                    = TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES,
    .bridging_tuples_nr          = 2,
    .bridging_tuples             =
        (struct _bridgingTupleEntries[]){
            {
                .bridging_tuple_macs_nr  = 2,
                .bridging_tuple_macs     =
                    (struct _bridgingTupleMacEntries[]){
                        {
                            .mac_address     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
                        },
                        {
                            .mac_address     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x02},
                        },
                    }
            },
            {
                .bridging_tuple_macs_nr  = 0,
                .bridging_tuple_macs     = NULL,
            },
        },
};

INT8U x1905_tlv_stream_012[] =
{
    0x04,
    0x00, 0x0f,
    0x02,
    0x02,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
    0x00,
};

INT16U x1905_tlv_stream_len_012 = 18;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 013 (TLV <--> packet)
////
////////////////////////////////////////////////////////////////////////////////

struct deviceBridgingCapabilityTLV x1905_tlv_structure_013 =
{
    .tlv_type                    = TLV_TYPE_DEVICE_BRIDGING_CAPABILITIES,
    .bridging_tuples_nr          = 0,
    .bridging_tuples             = NULL,
};

INT8U x1905_tlv_stream_013[] =
{
    0x04,
    0x00, 0x01,
    0x00,
};

INT16U x1905_tlv_stream_len_013 = 4;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 014 (TLV <--> packet)
////
////////////////////////////////////////////////////////////////////////////////

struct non1905NeighborDeviceListTLV x1905_tlv_structure_014 =
{
    .tlv_type                    = TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST,
    .local_mac_address           = {0x33, 0x34, 0x35, 0x36, 0x37, 0x38},
    .non_1905_neighbors_nr       = 1,
    .non_1905_neighbors          =
        (struct _non1905neighborEntries[]){
            {
                .mac_address = {0x43, 0x44, 0x45, 0x46, 0x47, 0x48},
            },
        },
};

INT8U x1905_tlv_stream_014[] =
{
    0x06,
    0x00, 0x0c,
    0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
};

INT16U x1905_tlv_stream_len_014 = 15;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 015 (TLV <--> packet)
////
////////////////////////////////////////////////////////////////////////////////

struct non1905NeighborDeviceListTLV x1905_tlv_structure_015 =
{
    .tlv_type                    = TLV_TYPE_NON_1905_NEIGHBOR_DEVICE_LIST,
    .local_mac_address           = {0x33, 0x34, 0x35, 0x36, 0x37, 0x38},
    .non_1905_neighbors_nr       = 2,
    .non_1905_neighbors          =
        (struct _non1905neighborEntries[]){
            {
                .mac_address = {0x43, 0x44, 0x45, 0x46, 0x47, 0x48},
            },
            {
                .mac_address = {0x53, 0x54, 0x55, 0x56, 0x57, 0x58},
            },
        },
};

INT8U x1905_tlv_stream_015[] =
{
    0x06,
    0x00, 0x12,
    0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
};

INT16U x1905_tlv_stream_len_015 = 21;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 016 (TLV <--> packet)
////
////////////////////////////////////////////////////////////////////////////////

struct neighborDeviceListTLV x1905_tlv_structure_016 =
{
    .tlv_type                    = TLV_TYPE_NEIGHBOR_DEVICE_LIST,
    .local_mac_address           = {0x33, 0x34, 0x35, 0x36, 0x37, 0x38},
    .neighbors_nr                = 1,
    .neighbors                   =
        (struct _neighborEntries[]){
            {
                .mac_address = {0x43, 0x44, 0x45, 0x46, 0x47, 0x48},
                .bridge_flag = 0,
            },
        },
};

INT8U x1905_tlv_stream_016[] =
{
    0x07,
    0x00, 0x0d,
    0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x00,
};

INT16U x1905_tlv_stream_len_016 = 16;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 017 (TLV <--> packet)
////
////////////////////////////////////////////////////////////////////////////////

struct neighborDeviceListTLV x1905_tlv_structure_017 =
{
    .tlv_type                    = TLV_TYPE_NEIGHBOR_DEVICE_LIST,
    .local_mac_address           = {0x33, 0x34, 0x35, 0x36, 0x37, 0x38},
    .neighbors_nr                = 2,
    .neighbors                   =
        (struct _neighborEntries[]){
            {
                .mac_address = {0x43, 0x44, 0x45, 0x46, 0x47, 0x48},
                .bridge_flag = 1,
            },
            {
                .mac_address = {0x53, 0x54, 0x55, 0x56, 0x57, 0x58},
                .bridge_flag = 0,
            },
        },
};

INT8U x1905_tlv_stream_017[] =
{
    0x07,
    0x00, 0x14,
    0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
    0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x80,
    0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
    0x00,
};

INT16U x1905_tlv_stream_len_017 = 23;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 018 (TLV <--> packet)
////
////////////////////////////////////////////////////////////////////////////////

struct linkMetricResultCodeTLV x1905_tlv_structure_018 =
{
    .tlv_type                    = TLV_TYPE_LINK_METRIC_RESULT_CODE,
    .result_code                 = LINK_METRIC_RESULT_CODE_TLV_INVALID_NEIGHBOR,
};

INT8U x1905_tlv_stream_018[] =
{
    0x0c,
    0x00, 0x01,
    0x00,
};

INT16U x1905_tlv_stream_len_018 = 4;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 019 (TLV <-- packet)
////
////////////////////////////////////////////////////////////////////////////////

struct linkMetricResultCodeTLV x1905_tlv_structure_019 =
{
    .tlv_type                    = TLV_TYPE_LINK_METRIC_RESULT_CODE,
    .result_code                 = 5,
};

INT8U x1905_tlv_stream_019[] =
{
    0x0c,
    0x00, 0x01,
    0x05,
};

INT16U x1905_tlv_stream_len_019 = 4;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 020 (TLV <--> packet)
////
////////////////////////////////////////////////////////////////////////////////

struct searchedRoleTLV x1905_tlv_structure_020 =
{
    .tlv_type                    = TLV_TYPE_SEARCHED_ROLE,
    .role                        = IEEE80211_ROLE_REGISTRAR,
};

INT8U x1905_tlv_stream_020[] =
{
    0x0d,
    0x00, 0x01,
    0x00,
};

INT16U x1905_tlv_stream_len_020 = 4;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 021 (TLV <-- packet)
////
////////////////////////////////////////////////////////////////////////////////

struct searchedRoleTLV x1905_tlv_structure_021 =
{
    .tlv_type                    = TLV_TYPE_SEARCHED_ROLE,
    .role                        = 0xff,
};

INT8U x1905_tlv_stream_021[] =
{
    0x0d,
    0x00, 0x01,
    0xff,
};

INT16U x1905_tlv_stream_len_021 = 4;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 022 (TLV <--> packet)
////
////////////////////////////////////////////////////////////////////////////////

struct autoconfigFreqBandTLV x1905_tlv_structure_022 =
{
    .tlv_type                    = TLV_TYPE_AUTOCONFIG_FREQ_BAND,
    .freq_band                   = IEEE80211_FREQUENCY_BAND_2_4_GHZ,
};

INT8U x1905_tlv_stream_022[] =
{
    0x0e,
    0x00, 0x01,
    0x00,
};

INT16U x1905_tlv_stream_len_022 = 4;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 023 (TLV <-- packet)
////
////////////////////////////////////////////////////////////////////////////////

struct autoconfigFreqBandTLV x1905_tlv_structure_023 =
{
    .tlv_type                    = TLV_TYPE_AUTOCONFIG_FREQ_BAND,
    .freq_band                   = 0x1a,
};

INT8U x1905_tlv_stream_023[] =
{
    0x0e,
    0x00, 0x01,
    0x1a,
};

INT16U x1905_tlv_stream_len_023 = 4;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 024 (TLV <--> packet)
////
////////////////////////////////////////////////////////////////////////////////

struct searchedRoleTLV x1905_tlv_structure_024 =
{
    .tlv_type                    = TLV_TYPE_SUPPORTED_ROLE,
    .role                        = IEEE80211_ROLE_REGISTRAR,
};

INT8U x1905_tlv_stream_024[] =
{
    0x0f,
    0x00, 0x01,
    0x00,
};

INT16U x1905_tlv_stream_len_024 = 4;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 025 (TLV <-- packet)
////
////////////////////////////////////////////////////////////////////////////////

struct searchedRoleTLV x1905_tlv_structure_025 =
{
    .tlv_type                    = TLV_TYPE_SUPPORTED_ROLE,
    .role                        = 0x02,
};

INT8U x1905_tlv_stream_025[] =
{
    0x0f,
    0x00, 0x01,
    0x02,
};

INT16U x1905_tlv_stream_len_025 = 4;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 026 (TLV <--> packet)
////
////////////////////////////////////////////////////////////////////////////////

struct autoconfigFreqBandTLV x1905_tlv_structure_026 =
{
    .tlv_type                    = TLV_TYPE_SUPPORTED_FREQ_BAND,
    .freq_band                   = IEEE80211_FREQUENCY_BAND_5_GHZ,
};

INT8U x1905_tlv_stream_026[] =
{
    0x10,
    0x00, 0x01,
    0x01,
};

INT16U x1905_tlv_stream_len_026 = 4;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 027 (TLV <-- packet)
////
////////////////////////////////////////////////////////////////////////////////

struct autoconfigFreqBandTLV x1905_tlv_structure_027 =
{
    .tlv_type                    = TLV_TYPE_SUPPORTED_FREQ_BAND,
    .freq_band                   = 0x07,
};

INT8U x1905_tlv_stream_027[] =
{
    0x10,
    0x00, 0x01,
    0x07,
};

INT16U x1905_tlv_stream_len_027 = 4;


////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 028 (TLV <--> packet)
////
////////////////////////////////////////////////////////////////////////////////

struct pushButtonEventNotificationTLV x1905_tlv_structure_028 =
{
    .tlv_type                    = TLV_TYPE_PUSH_BUTTON_EVENT_NOTIFICATION,
    .media_types_nr              = 2,
    .media_types                 =
        (struct _mediaTypeEntries[]){
            {
                .media_type                                                        = MEDIA_TYPE_IEEE_802_11AF,
                .media_specific_data_size                                          = 10,
                .media_specific_data.ieee80211.network_membership                  = {0x01, 0x01, 0x01, 0x02, 0x02, 0x02},
                .media_specific_data.ieee80211.role                                = IEEE80211_SPECIFIC_INFO_ROLE_WIFI_P2P_CLIENT,
                .media_specific_data.ieee80211.ap_channel_band                     = 0x05,
                .media_specific_data.ieee80211.ap_channel_center_frequency_index_1 = 0x0a,
                .media_specific_data.ieee80211.ap_channel_center_frequency_index_2 = 0x0b,
            },
            {
                .media_type                                      = MEDIA_TYPE_IEEE_1901_WAVELET,
                .media_specific_data_size                        = 7,
                .media_specific_data.ieee1901.network_identifier = {0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0xff},
            },
        },
};

INT8U x1905_tlv_stream_028[] =
{
    0x12,
    0x00, 0x18,
    0x02,
    0x01, 0x07,
    0x0a,
    0x01, 0x01, 0x01, 0x02, 0x02, 0x02,
    0x80,
    0x05,
    0x0a,
    0x0b,
    0x02, 0x00,
    0x07,
    0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0xff,
};

INT16U x1905_tlv_stream_len_028 = 27;

////////////////////////////////////////////////////////////////////////////////
////
//// Test vector 041 (TLV <--> packet)
////
////////////////////////////////////////////////////////////////////////////////

struct vendorSpecificTLV x1905_tlv_structure_041 =
{
    .tlv_type                    = TLV_TYPE_VENDOR_SPECIFIC,
    .vendorOUI                   = {0x00, 0x03, 0x7f},
    .m_nr                        = 21,
    .m                           = (INT8U[]) {0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x02, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
};

INT8U x1905_tlv_stream_041[] =
{
    0x0b,
    0x00, 0x18,
    0x00, 0x03, 0x7f,
    0x01, 0x00, 0x00, 0x00, 0x05, 0x00, 0x02, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};

INT16U x1905_tlv_stream_len_041 = 27;
