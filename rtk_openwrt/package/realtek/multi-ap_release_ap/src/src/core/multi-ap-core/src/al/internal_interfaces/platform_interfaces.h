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


#ifndef _PLATFORM_INTERFACES_H_
#define _PLATFORM_INTERFACES_H_

#include "multi_ap_tlvs.h"
#include "multi_ap_tlvs_r2.h"
#include "multi_ap_tlvs_r3.h"
#include "multi_ap_tlvs_r4.h"

////////////////////////////////////////////////////////////////////////////////
// Interfaces info
////////////////////////////////////////////////////////////////////////////////

struct neighborInfo
{
    INT8U  mac_address[6];
    INT64U link_time;
    INT8U  multiap_profile;
};

struct ethernetNeighborInfo
{
    INT8U  mac_address[6];
    INT8U  port;
    char   ifName[64];
};

struct interfaceInfo
{
    char  *name;           // Example: "eth0"

    INT8U mac_address[6];  // 6  bytes long MAC address of the interface.

                           // ID information (NULL terminated strings)

    #define INTERFACE_TYPE_IEEE_802_3U_FAST_ETHERNET       (0x0000)
    #define INTERFACE_TYPE_IEEE_802_3AB_GIGABIT_ETHERNET   (0x0001)
    #define INTERFACE_TYPE_IEEE_802_11B_2_4_GHZ            (0x0100)
    #define INTERFACE_TYPE_IEEE_802_11G_2_4_GHZ            (0x0101)
    #define INTERFACE_TYPE_IEEE_802_11A_5_GHZ              (0x0102)
    #define INTERFACE_TYPE_IEEE_802_11N_2_4_GHZ            (0x0103)
    #define INTERFACE_TYPE_IEEE_802_11N_5_GHZ              (0x0104)
    #define INTERFACE_TYPE_IEEE_802_11AC_5_GHZ             (0x0105)
    #define INTERFACE_TYPE_IEEE_802_11AD_60_GHZ            (0x0106)
    #define INTERFACE_TYPE_IEEE_802_11AF                   (0x0107)
    #define INTERFACE_TYPE_IEEE_802_11AX                   (0x0108)
    #define INTERFACE_TYPE_IEEE_1901_WAVELET               (0x0200)
    #define INTERFACE_TYPE_IEEE_1901_FFT                   (0x0201)
    #define INTERFACE_TYPE_MOCA_V1_1                       (0x0300)
    #define INTERFACE_TYPE_UNKNOWN                         (0xFFFF)

    INT16U interface_type;  // Indicates the MAC/PHY type of the underlying
                            // network technology.
                            // Valid values: any "INTERFACE_TYPE_*" value.
                            // If your interface is of a type not listed here,
                            // set it to "INTERFACE_TYPE_UNKNOWN" and then
                            // use the "interface_type_data.other" field to
                            // further identify it.

    union _interfaceTypeData
    {
        // Only to be filled when interface_type = INTERFACE_TYPE_IEEE_802_11*
        //
        struct _ieee80211Data
        {
            INT8U  bssid[6];        // This is the BSSID (MAC address of the
                                    // registrar AP on a wifi network).
                                    // On unconfigured nodes (ie. STAs which
                                    // have not yet joined a network or non-
                                    // registrar APs which have not yet cloned
                                    // the credentiales from the registrar) this
                                    // parameter must be set to all zeros.

            char   ssid[64];        // This is the "friendly" name of the wifi
                                    // network created by the registrar AP
                                    // identified by 'bssid'

            #define IEEE80211_ROLE_AP                   (0x0)
            #define IEEE80211_ROLE_NON_AP_NON_PCP_STA   (0x4)
            #define IEEE80211_ROLE_WIFI_P2P_CLIENT      (0x8)
            #define IEEE80211_ROLE_WIFI_P2P_GROUP_OWNER (0x9)
            #define IEEE80211_ROLE_AD_PCP               (0xa)
            INT8U  role;            // One of the values from above

            INT8U ap_channel_band;  // Hex value of dot11CurrentChannelBandwidth
                                    // (see "IEEE P802.11ac/D3.0" for
                                    // description)

            INT8U ap_channel_center_frequency_index_1;
                                    // Hex value of
                                    // dot11CurrentChannelCenterFrequencyIndex1
                                    // (see "IEEE P802.11ac/D3.0" for
                                    // description)

            INT8U ap_channel_center_frequency_index_2;
                                    // Hex value of
                                    // dot11CurrentChannelCenterFrequencyIndex2
                                    // (see "IEEE P802.11ac/D3.0" for
                                    // description)

            #define IEEE80211_AUTH_MODE_OPEN    (0x0001)
            #define IEEE80211_AUTH_MODE_WPA     (0x0002)
            #define IEEE80211_AUTH_MODE_WPAPSK  (0x0004)
            #define IEEE80211_AUTH_MODE_WPA2    (0x0008)
            #define IEEE80211_AUTH_MODE_WPA2PSK (0x0010)
            #define IEEE80211_AUTH_MODE_WPA3PSK (0x0020)
            INT16U authentication_mode;
                                    // For APs: list of supported modes that
                                    // clients can use (OR'ed list of flags)
                                    // For STAs: current mode being used with
                                    // its AP (a single flag)

            #define IEEE80211_ENCRYPTION_MODE_NONE (0x0001)
            #define IEEE80211_ENCRYPTION_MODE_TKIP (0x0002)
            #define IEEE80211_ENCRYPTION_MODE_AES  (0x0004)
            INT16U encryption_mode;
                                    // For APs: list of supported modes that
                                    // clients can use (OR'ed list of flags)
                                    // For STAs: current mode being used with
                                    // its AP (a single flag)

            char  network_key[64];  // Key that grants access to the AP network

            INT8U is_configured;  // Indicate the BSS is configured

            #define MULTI_AP_FRONTHAUL_BSS      (0x20)
            #define MULTI_AP_BACKHAUL_BSS       (0x40)
            #define MULTI_AP_BACKHAUL_STA       (0x80)
            INT8U network_type;     // Multi AP Network type
                                    // 0:

            #define IEEE80211_RF_BAND_2_4_GHZ           (0x00)
            #define IEEE80211_RF_BAND_5_GHZ             (0x01)
            #define IEEE80211_RF_BAND_60_GHZ            (0x02)
            INT8U rf_band;
        } ieee80211;

        // Only to be filled when interface_type = INTERFACE_TYPE_IEEE_1901*
        //
        struct _ieee1901Data
        {
            char network_identifier[7];  // Network membership

        } ieee1901;
    } interface_type_data; // Depending on the value of "interface_type", one
                           // (and only one!) of the structures of this union
                           // must be filled

    INT8U is_secured;      // Contains "1" if the interface is secure, "0"
                           // otherwise.
                           //
                           // Note that "secure" in this context means that the
                           // interface can be trusted to send private (in a
                           // "local network" way) messages.
                           //
                           // For example:
                           //
                           //   1. A "wifi" interface can only be considered
                           //      "secure" if encryption is on (WPA, WPA2,
                           //      etc...)
                           //
                           //   2. A G.hn/1901 interface can only be considered
                           //      "secure" if some one else's untrusted device
                           //      can not "sniff" your traffic.  This typically
                           //      means either encryption or some other
                           //      technology dependent "trick" (ex: "network
                           //      id") is enabled.
                           //
                           //   3. An ethernet interface can probably be always
                           //      be considered "secure" (but this is let for
                           //      the implementer to decide)
                           //
                           // One interface becomes "secured" when it contains
                           // at least one link which is "secured".
                           // For example, a wifi AP interface is considered
                           // "secured" if there is at least one STA connected
                           // to it by means of an encrypted channel.

    INT8U push_button_on_going;
                           // Some types of interfaces support a technology-
                           // specific "push button" configuration mechanism
                           // (ex: "802.11", "G.hn"). Others don't (ex: "eth").
                           //
                           // This value is set to any of these possible values:
                           //
                           //   - "0" if the interface type supports this "push
                           //     button" configuration mechanism but, right
                           //     now, this process is not running.
                           //
                           //   - "1" if the interface type supports this "push
                           //     button" configuration mechanism and, right
                           //     now, we are in the middle of such process.
                           //
                           //   - "2" if the interface does not support the
                           //     "push button" configuration mechanism.
    #ifdef _RTK_LINUX_
    INT8U wsc_result;
                           // For realtek platform, used to trace wscd status-
                           //
                           // This value is set to any of these possible values:
                           //
                           //   - "0" error or timeout
                           //
                           //   - "1" successful
    #endif

    INT8U push_button_new_mac_address[6];
                           // 6  bytes long MAC address of the device that has
                           // just joined the network as a result of a "push
                           // button configuration" process (ie., just after
                           // "push_button_on_going" changes from "1" to "0")
                           // This field is set to all zeros when either:
                           //
                           //   A) WE are the device joining the network
                           //
                           //   B) No new device entered the network
                           //
                           //   C) The underlying technology does not offer this
                           //      information

    #define INTERFACE_POWER_STATE_ON     (0x00)
    #define INTERFACE_POWER_STATE_SAVE   (0x01)
    #define INTERFACE_POWER_STATE_OFF    (0x02)
    INT8U power_state;     // Contains one of the INTERFACE_POWER_STATE_* values
                           // from above

    #define INTERFACE_NEIGHBORS_UNKNOWN (0xFF)
    INT8U  neighbor_mac_addresses_nr;
                             // Number of other MAC addresses (pertaining -or
                             // not- to 1905 devices) this interface has
                             // received packets from in the past (not
                             // necessarily from the time the interface was
                             // brought up, but a reasonable amount of time)
                             // A special value of "INTERFACE_NEIGHBORS_UNKNOWN"
                             // is used to indicate that this interface has no
                             // way of obtaining this information (note that
                             // this is different from "0" which means "I know I
                             // have zero neighbors")

    struct neighborInfo *neighbor_list;
                             // List containing those MAC addreses just
                             // described in the comment above.
};

#define IS_IEEE_802_3_INTERFACE(x)  (x >> 8 == 0)
#define IS_IEEE_802_11_INTERFACE(x) (x >> 8 == 1)
#define IS_IEEE_1901_INTERFACE(x)   (x >> 8 == 2)
#define IS_IEEE_MOCA_INTERFACE(x)   (x >> 8 == 3)

#define IS_IEEE_802_11_2_4_GHZ(x, y) \
	(x == INTERFACE_TYPE_IEEE_802_11B_2_4_GHZ \
	|| x == INTERFACE_TYPE_IEEE_802_11G_2_4_GHZ \
	|| x == INTERFACE_TYPE_IEEE_802_11N_2_4_GHZ \
	|| y == IEEE80211_RF_BAND_2_4_GHZ)

#define IS_IEEE_802_11_5_GHZ(x, y) \
	(x == INTERFACE_TYPE_IEEE_802_11A_5_GHZ \
	|| x == INTERFACE_TYPE_IEEE_802_11N_5_GHZ \
	|| x == INTERFACE_TYPE_IEEE_802_11AC_5_GHZ \
	|| y == IEEE80211_RF_BAND_5_GHZ)

#define IS_IEEE_802_11_60_GHZ(x) (x == INTERFACE_TYPE_IEEE_802_11AD_60_GHZ)

#define IS_IEEE_802_11AX(x) (x == INTERFACE_TYPE_IEEE_802_11AX)

/* Band Definition */
#define _2G  0
#define _5G  1
#define _5GL 2
#define _5GH 3
#define _5GF 4

/* Region Domain Enum */
enum WIFI_REG_DOMAIN {
	DOMAIN_FCC		= 1,
	DOMAIN_IC		= 2,
	DOMAIN_ETSI		= 3,
	DOMAIN_SPAIN	= 4,
	DOMAIN_FRANCE	= 5,
	DOMAIN_MKK		= 6,
	DOMAIN_ISRAEL	= 7,
	DOMAIN_MKK1		= 8,
	DOMAIN_MKK2		= 9,
	DOMAIN_MKK3		= 10,
	DOMAIN_NCC		= 11,
	DOMAIN_RUSSIAN	= 12,
	DOMAIN_CN		= 13,
	DOMAIN_GLOBAL	= 14,
	DOMAIN_WORLD_WIDE = 15,
	DOMAIN_TEST		= 16,
	DOMAIN_5M10M	= 17,
	DOMAIN_SG		= 18,
	DOMAIN_KR		= 19,
	DOMAIN_MAX
};

/* Bandwidth Enum */
enum MAP_BAND_WIDTH {
	BAND_WIDTH_20MHZ,
	BAND_WIDTH_40MHZ,
	BAND_WIDTH_80MHZ,
	BAND_WIDTH_160MHZ,
	BAND_WIDTH_80_80MHZ
};

/* Side Band Enum */
enum MAP_SIDE_BAND {
	SIDE_BAND_NONE,
	SIDE_BAND_LOWER,
	SIDE_BAND_UPPER
};

/* Operating Class Structure */
typedef struct _OP_CLASS_ {
	INT8U        op_class;
	INT8U        band;     /* 0: 2g, 1: 5g*/
	INT8U        sub_band; /* 0: none/2g, 1: 5gh, 2: 5gl */
	INT8U        bandwidth;
	INT8U        side_band;
	const INT8U *channel_array;
} OP_CLASS;

enum MAP_RADIO_BAND {
	RADIO_BAND_2G,
	RADIO_BAND_5GL,
	RADIO_BAND_5GH,
	RADIO_BAND_ETH = 0xD0,
	RADIO_BAND_UNKNOWN = 0xFF
};

/* Bandwidth, Side band, Channel list */
static const INT8U op_class_81[]  = { 13 /*channel num*/, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
static const INT8U op_class_82[]  = { 1 /*channel num*/, 14 };
static const INT8U op_class_83[]  = { 9 /*channel num*/, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
static const INT8U op_class_84[]  = { 9 /*channel num*/, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
static const INT8U op_class_115[] = { 4 /*channel num*/, 36, 40, 44, 48 };
static const INT8U op_class_116[] = { 2 /*channel num*/, 36, 44 };
static const INT8U op_class_117[] = { 2 /*channel num*/, 40, 48 };
static const INT8U op_class_118[] = { 4 /*channel num*/, 52, 56, 60, 64 };
static const INT8U op_class_119[] = { 2 /*channel num*/, 52, 60 };
static const INT8U op_class_120[] = { 2 /*channel num*/, 56, 64 };
static const INT8U op_class_121[] = { 12 /*channel num*/, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144 };
static const INT8U op_class_122[] = { 6 /*channel num*/, 100, 108, 116, 124, 132, 140 };
static const INT8U op_class_123[] = { 6 /*channel num*/, 104, 112, 120, 128, 136, 144 };
static const INT8U op_class_124[] = { 4 /*channel num*/, 149, 153, 157, 161 };
static const INT8U op_class_125[] = { 6 /*channel num*/, 149, 153, 157, 161, 165, 169 };
static const INT8U op_class_126[] = { 2 /*channel num*/, 149, 157 };
static const INT8U op_class_127[] = { 2 /*channel num*/, 153, 161 };
static const INT8U op_class_128[] = { 6 /*channel num*/, 42, 58, 106, 122, 138, 155 };

static const INT8U op_class_129[] = { 2 /*channel num*/, 50, 114 }; //BAND_WIDTH_160MHZ, SIDE_BAND_NONE,36 40 44 48 52 56 60 64 100 104 108 112 116 120 124 128
// static const INT8U op_class_130[] = { 42, 58, 106, 122, 138, 155 }; BAND_WIDTH_80_80MHZ, SIDE_BAND_NONE,

static const INT8U beacon_channel_125_no_169[] = { 5 /*channel num*/, 149, 153, 157, 161, 165 };

/* Global Operating Classes */
static const OP_CLASS GLOBAL_OP_CLASS[] = {
	{ 81, _2G, _2G, BAND_WIDTH_20MHZ, SIDE_BAND_NONE, op_class_81 },
	{ 82, _2G, _2G, BAND_WIDTH_20MHZ, SIDE_BAND_NONE, op_class_82 },
	{ 83, _2G, _2G, BAND_WIDTH_40MHZ, SIDE_BAND_LOWER, op_class_83 },
	{ 84, _2G, _2G, BAND_WIDTH_40MHZ, SIDE_BAND_UPPER, op_class_84 },
	{ 115, _5G, _5GL, BAND_WIDTH_20MHZ, SIDE_BAND_NONE, op_class_115 },
	{ 116, _5G, _5GL, BAND_WIDTH_40MHZ, SIDE_BAND_LOWER, op_class_116 },
	{ 117, _5G, _5GL, BAND_WIDTH_40MHZ, SIDE_BAND_UPPER, op_class_117 },
	{ 118, _5G, _5GL, BAND_WIDTH_20MHZ, SIDE_BAND_NONE, op_class_118 },
	{ 119, _5G, _5GL, BAND_WIDTH_40MHZ, SIDE_BAND_LOWER, op_class_119 },
	{ 120, _5G, _5GL, BAND_WIDTH_40MHZ, SIDE_BAND_UPPER, op_class_120 },
	{ 121, _5G, _5GH, BAND_WIDTH_20MHZ, SIDE_BAND_NONE, op_class_121 },
	{ 122, _5G, _5GH, BAND_WIDTH_40MHZ, SIDE_BAND_LOWER, op_class_122 },
	{ 123, _5G, _5GH, BAND_WIDTH_40MHZ, SIDE_BAND_UPPER, op_class_123 },
	{ 124, _5G, _5GH, BAND_WIDTH_20MHZ, SIDE_BAND_NONE, op_class_124 },
	{ 125, _5G, _5GH, BAND_WIDTH_20MHZ, SIDE_BAND_NONE, op_class_125 },
	{ 126, _5G, _5GH, BAND_WIDTH_40MHZ, SIDE_BAND_LOWER, op_class_126 },
	{ 127, _5G, _5GH, BAND_WIDTH_40MHZ, SIDE_BAND_UPPER, op_class_127 },
	{ 128, _5G, _5GF, BAND_WIDTH_80MHZ, SIDE_BAND_NONE, op_class_128 },
	{ 129, _5G, _5GF, BAND_WIDTH_160MHZ, SIDE_BAND_NONE, op_class_129 }
};

/* Country Code IE */
typedef struct _COUNTRY_IE {
	INT16U      country_no;
	char        country_str[3];
	INT8U       region_domain;
} COUNTRY_IE;

/* Country Code, Country String and Region Donmain Table */
static const COUNTRY_IE COUNTRY_IE_ARRAY[] =
{
	{8,"AL",   3},  /*ALBANIA*/
	{12,"DZ",  3},  /*ALGERIA*/
	{32,"AR",  3},  /*ARGENTINA*/
	{51,"AM",  3},  /*ARMENIA*/
	{36,"AU",  3},  /*AUSTRALIA*/
	{40,"AT",  3},  /*AUSTRIA*/
	{31,"AZ",  3},  /*AZERBAIJAN*/
	{48,"BH",  3},  /*BAHRAIN*/
	{112,"BY",  3},  /*BELARUS*/
	{56,"BE",  3},  /*BELGIUM*/
	{84,"BZ",  3},  /*BELIZE*/
	{68,"BO",  3},  /*BOLIVIA*/
	{76,"BR",  3},  /*BRAZIL*/
	{96,"BN",  3},  /*BRUNEI*/
	{100,"BG", 3},  /*BULGARIA*/
	{124,"CA", 1},  /*CANADA*/
	{152,"CL", 3},  /*CHILE*/
	{156,"CN",13},  /*CHINA*/
	{170,"CO", 1},  /*COLOMBIA*/
	{188,"CR", 3},  /*COSTA RICA*/
	{191,"HR", 3},  /*CROATIA*/
	{196,"CY", 3},  /*CYPRUS*/
	{203,"CZ", 3},  /*CZECH REPUBLIC*/
	{208,"DK", 3},  /*DENMARK*/
	{214,"DO", 1},  /*DOMINICAN REPUBLIC*/
	{218,"EC", 3},  /*ECUADOR*/
	{818,"EG", 3},  /*EGYPT*/
	{222,"SV", 3},  /*EL SALVADOR*/
	{233,"EE", 3},  /*ESTONIA*/
	{246,"FI", 3},  /*FINLAND*/
	{250,"FR", 3},  /*FRANCE*/
	{268,"GE", 3},  /*GEORGIA*/
	{276,"DE", 3},  /*GERMANY*/
	{300,"GR", 3},  /*GREECE*/
	{320,"GT", 1},  /*GUATEMALA*/
	{340,"HN", 3},  /*HONDURAS*/
	{344,"HK", 3},  /*HONG KONG*/
	{348,"HU", 3},  /*HUNGARY*/
	{352,"IS", 3},  /*ICELAND*/
	{356,"IN", 3},  /*INDIA*/
	{360,"ID", 3},  /*INDONESIA*/
	{364,"IR", 3},  /*IRAN*/
	{372,"IE", 3},  /*IRELAND*/
	{376,"IL", 7},  /*ISRAEL*/
	{380,"IT", 3},  /*ITALY*/
	{392,"JP", 6},  /*JAPAN*/
	{400,"JO", 3},  /*JORDAN*/
	{398,"KZ", 3},  /*KAZAKHSTAN*/
	{410,"KR", 3},  /*NORTH KOREA*/
	{408,"KP", 3},  /*KOREA REPUBLIC*/
	{414,"KW", 3},  /*KUWAIT*/
	{428,"LV", 3},  /*LATVIA*/
	{422,"LB", 3},  /*LEBANON*/
	{438,"LI", 3},  /*LIECHTENSTEIN*/
	{440,"LT", 3},  /*LITHUANIA*/
	{442,"LU", 3},  /*LUXEMBOURG*/
	{446,"MO", 3},  /*CHINA MACAU*/
	{807,"MK", 3},  /*MACEDONIA*/
	{458,"MY", 3},  /*MALAYSIA*/
	{484,"MX", 1},  /*MEXICO*/
	{492,"MC", 3},  /*MONACO*/
	{504,"MA", 3},  /*MOROCCO*/
	{528,"NL", 3},  /*NETHERLANDS*/
	{554,"NZ", 3},  /*NEW ZEALAND*/
	{578,"NO", 3},  /*NORWAY*/
	{512,"OM", 3},  /*OMAN*/
	{586,"PK", 3},  /*PAKISTAN*/
	{591,"PA", 1},  /*PANAMA*/
	{604,"PE", 3},  /*PERU*/
	{608,"PH", 3},  /*PHILIPPINES*/
	{616,"PL", 3},  /*POLAND*/
	{620,"PT", 3},  /*PORTUGAL*/
	{630,"PR", 1},  /*PUERTO RICO*/
	{634,"QA", 3},  /*QATAR*/
	{642,"RO", 3},  /*ROMANIA*/
	{643,"RU",12},  /*RUSSIAN*/
	{682,"SA", 3},  /*SAUDI ARABIA*/
	{702,"SG", 3},  /*SINGAPORE*/
	{703,"SK", 3},  /*SLOVAKIA*/
	{705,"SI", 3},  /*SLOVENIA*/
	{710,"ZA", 3},  /*SOUTH AFRICA*/
	{724,"ES", 3},  /*SPAIN*/
	{752,"SE", 3},  /*SWEDEN*/
	{756,"CH", 3},  /*SWITZERLAND*/
	{760,"SY", 3},  /*SYRIAN ARAB REPUBLIC*/
	{158,"TW",11},  /*TAIWAN*/
	{764,"TH", 3},  /*THAILAND*/
	{780,"TT", 3},  /*TRINIDAD AND TOBAGO*/
	{788,"TN", 3},  /*TUNISIA*/
	{792,"TR", 3},  /*TURKEY*/
	{804,"UA", 3},  /*UKRAINE*/
	{784,"AE", 3},  /*UNITED ARAB EMIRATES*/
	{826,"GB", 3},  /*UNITED KINGDOM*/
	{840,"US", 1},  /*UNITED STATES*/
	{858,"UY", 3},  /*URUGUAY*/
	{860,"UZ", 1},  /*UZBEKISTAN*/
	{862,"VE", 3},  /*VENEZUELA*/
	{704,"VN", 3},  /*VIET NAM*/
	{887,"YE", 3},  /*YEMEN*/
	{716,"ZW", 3},  /*ZIMBABWE*/
};

struct easymesh_interface_mib {
	char *  ssid;
	char *  network_key;
	unsigned char network_type;
	unsigned char is_enabled; //1: enabled, 0: disabled
	unsigned char bss_index;
	unsigned char encrypt_type;
	unsigned char need_configure;
	unsigned char interface_index;
	unsigned char authentication_type;
};

struct easymesh_radio_mib {
	unsigned char                        radio_band;
	unsigned char                        radio_channel;
	unsigned char                        channel_bandwidth;
	unsigned char                        control_sideband; // 0: upper, 1: lower
	unsigned char                        need_change_channel;
	char *                         repeater_ssid;
	unsigned char                        interface_nr;
	struct easymesh_interface_mib *interface_mib;
};

// Return a list of strings (each one representing an "interface name", such
// as "eth0", "eth1", etc...).
//
// The length of the list is returned in the 'nr' output argument.
//
// If something goes wrong, return NULL and set the contents of 'nr' to '0'
//
// Each element of the list represents an interface on the localhost that will
// participate in the 1905 network.
//
// The 'name' field is a platform-specific NULL terminated string that will
// later be used in other functions to refer to this particular interface.
//
// The returned list must not be modified by the caller.
//
// When the returned list is no longer needed, it can be freed by calling
// "PLATFORM_FREE_LIST_OF_1905_INTERFACES()"
//
// [PLATFORM PORTING NOTE]
//   Typically you want to return as many entries as physical interfaces there
//   are in the platform. However, if for some reason you want to make one or
//   more interfaces "invisible" to 1905 (maybe because they are "debug"
//   interfaces, such as a "management" ethernet port) you can return a reduced
//   list of interfaces.
//
char **PLATFORM_GET_LIST_OF_1905_INTERFACES(INT8U *nr);

// Used to free the pointer returned by a previous call to
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()"
//
// 'nr' is the same one returned by "PLATFORM_GET_LIST_OF_1905_INTERFACES()"
//
void PLATFORM_FREE_LIST_OF_1905_INTERFACES(char **x, INT8U nr);

// Return a "struct interfaceInfo" structure containing all kinds of information
// associated to the provided 'interface_name'
//
// If something goes wrong, return NULL.
//
// 'interface_name' is one of the names previously returned in a call to
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()"
//
// The documentation of the "struct interfaceInfo" structure explain what each
// field of this structure should contain.
//
// Once the caller is done with the returned structure, hw must call
// "PLATFORM_FREE_1905_STRUCTURE()" to dispose it
//
struct interfaceInfo *PLATFORM_GET_1905_INTERFACE_INFO(char *interface_name);

struct interfaceInfo *PLATFORM_GET_1905_INTERFACE_INFO_WITH_WPS(char *interface_name, INT8U need_wps);

// Free the memory used by a "struct interfaceInfo" structure previously
// obtained by calling "PLATFORM_GET_1905_INTERFACE_INFO()"
//
void PLATFORM_FREE_1905_INTERFACE_INFO(struct interfaceInfo *i);


////////////////////////////////////////////////////////////////////////////////
// Link metrics
////////////////////////////////////////////////////////////////////////////////

struct linkMetrics
{
    INT8U   local_interface_address[6];     // A MAC address belonging to one of
                                            // the local interfaces.
                                            // Let's call this MAC "A"

    INT8U   neighbor_interface_address[6];  // A MAC address belonging to a
                                            // neighbor interface that is
                                            // directly reachable from "A".
                                            // Let's call this MAC "B".

    INT32U  measures_window;   // Time in seconds representing how far back in
                               // time statistics have been being recorded for
                               // this interface.
                               // For example, if this value is set to "5" and
                               // 'tx_packet_ok' is set to "7", it means that
                               // in the last 5 seconds 7 packets have been
                               // transmitted OK between "A" and "B".
                               //
                               // [PLATFORM PORTING NOTE]
                               //   This is typically the amount of time
                               //   ellapsed since the interface was brought
                               //   up.

    INT32U  tx_packet_ok;      // Estimated number of transmitted packets from
                               // "A" to "B" in the last 'measures_window'
                               // seconds.

    INT32U  tx_packet_errors;  // Estimated number of packets with errors
                               // transmitted from "A" to "B" in the last
                               // 'measures_window' seconds.

    INT16U  tx_max_xput;       // Extimated maximum MAC throughput from "A" to
                               // "B" in Mbits/s.

    INT16U  tx_phy_rate;       // Extimated PHY rate from "A" to "B" in Mbits/s.

    INT16U  tx_link_availability;
                               // Estimated average percentage of time that the
                               // link is available to transmit data from "A"
                               // to "B" in the last 'measures_window' seconds.

    INT32U  rx_packet_ok;      // Estimated number of transmitted packets from
                               // "B" to "A" in the last 'measures_window'
                               // seconds.

    INT32U  rx_packet_errors;  // Estimated number of packets with errors
                               // transmitted from "B" to "A" i nthe last
                               // 'measures_window' seconds.

    INT8U   rx_rssi;           // Estimated RSSI when receiving data from "B" to
                               // "A" in dB.
};

// Return a "struct linkMetrics" structure containing all kinds of information
// associated to the link that exists between the provided local interface and
// neighbor's interface whose MAC address is 'neighbor_interface_address'.
//
// If something goes wrong, return NULL.
//
// 'local_interface_name' is one of the names previously returned in a call to
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()"
//
// 'neighbor_interface_address' is the MAC address at the other end of the link.
// (This MAC address belong to a neighbor's interface)
//
// The documentation of the "struct linkMetrics" structure explain what each
// field of this structure should contain.
//
// Once the caller is done with the returned structure, hw must call
// "PLATFORM_FREE_LINK_METRICS()" to dispose it
//
// [PLATFORM PORTING NOTE]
//   You will notice how each 'struct linkMetrics' is associated to a LINK and
//   not to an interface.
//   In some cases, the platform might not be able to keep PER LINK stats.
//   For example, in Linux is easy to check how many packets were received by
//   "eth0" *in total*, but it is not trivial to find out how many packets were
//   received by "eth0" *from each neighbor*.
//   In these cases there are two solutions:
//     1. Add new platform code to make this PER LINK reporting possible (for
//        example, in Linux you would have to create iptables rules among other
//        things)
//     2. Just report the overwall PER INTERFACE stats (thus ignoring the
//        'neighbor_interface_address' parameter).
//        This is better than reporting nothing at all.
//
struct linkMetrics *PLATFORM_GET_LINK_METRICS(char *local_interface_name, INT8U *neighbor_interface_address, INT8U stp_state);

void PLATFORM_GET_LINK_METRICS_IOCTL(char *local_interface_name, struct linkMetrics *link_metric, INT8U *neighbor_interface_address);

// Free the memory used by a "struct linkMetrics" structure previously
// obtained by calling "PLATFORM_GET_LINK_METRICS()"
//
void PLATFORM_FREE_LINK_METRICS(struct linkMetrics *l);

////////////////////////////////////////////////////////////////////////////////
// Bridges info
////////////////////////////////////////////////////////////////////////////////

struct bridge
{
    char   *name;           // Example: "br0"

    INT8U   bridged_interfaces_nr;
    char   *bridged_interfaces[10];
                            // Names of the interfaces (such as "eth0") that
                            // belong to this bridge

    INT8U   forwarding_rules_nr;
    struct _forwardingRules
    {
        // To be defined...
    } forwarding_rules;
};

////////////////////////////////////////////////////////////////////////////////
// Ethernet port mapping table
////////////////////////////////////////////////////////////////////////////////

struct eth_port_map {
	char ifname[20];
	INT32U update_timestamp;
};

struct eth_port_map *PLATFORM_GET_ETH_PORT_1905(void);

// Return a list of "bridge" structures. Each of them represents a set of
// local interfaces that have been "bridged" together.
//
// The length of the list is returned in the 'nr' output argument.
//
// When the returned list is no longer needed, it can be freed by calling
// "PLATFORM_FREE_LIST_OF_BRIDGES()"
//
struct bridge *PLATFORM_GET_LIST_OF_BRIDGES(INT8U *nr);

// Used to free the pointer returned by a previous call to
// "PLATFORM_GET_LIST_OF_BRIDGES()"
//
// 'nr' is the same one returned by "PLATFORM_GET_LIST_OF_BRIDGES()"
//
void PLATFORM_FREE_LIST_OF_BRIDGES(struct bridge *x, INT8U nr);


////////////////////////////////////////////////////////////////////////////////
// RAW packet generation
////////////////////////////////////////////////////////////////////////////////

// Send a RAW ethernet frame on interface 'name_interface' with:
//
//   - The "destination MAC address" field set to 'dst_mac'
//   - The "source MAC address" field set to 'src_mac'
//   - The "ethernet type" field set to 'eth_type'
//   - The payload os the ethernet frame set to the first 'payload_len' bytes
//     pointed by 'payload'
//
// If there is a problem and the packet cannot be sent, this function returns
// "0", otherwise it returns "1"
//
INT8U PLATFORM_SEND_RAW_PACKET(char *interface_name, INT8U *dst_mac, INT8U *src_mac, INT16U eth_type, INT8U *payload, INT16U payload_len);


////////////////////////////////////////////////////////////////////////////////
/// Push button configuration
////////////////////////////////////////////////////////////////////////////////

// Start the technology-specific "push button" configuration process on the
// provided interface.
//
// 'queue_id' is a value previously returned by a call to
// "PLATFORM_CREATE_QUEUE()"
//
// 'al_mac_address' is the AL MAC address contained in the "push button event
// notification" message that caused this function to be called. This value
// will later be reported back to the AL entity in the
// "PLATFORM_QUEUE_EVENT_AUTHENTICATED_LINK" message.
//
// 'mid' is the "message id" of the "push button event notification" message
// that caused this function to be called. This value will later be reported
// back to the AL entity in the "PLATFORM_QUEUE_EVENT_AUTHENTICATED_LINK"
// message.
//
//   NOTE:
//     When this function is called as a result of the user pressing a button
//     in the local device (versus receiving a remote "push button event
//     notification message" from another node) then 'al_mac_address' and 'mid'
//     contain the values that go inside the "push button event notification"
//     message that this local node is going to send to the others.
//
// Before calling this function, the "PLATFORM_QUEUE_EVENT_AUTHENTICATED_LINK"
// event must have been registered with "PLATFORM_REGISTER_QUEUE_EVENT()"
//
// This "push button" configuration process is used to add new devices to the
// network:
//
//   - For 802.11 interface this is usually the WPS mechanism.
//   - For G.hn interfaces we use the "pairing" mechanism.
//
// The function does not wait for the process to complete, instead it returns
// immediately and the configuration process is ran in background. Eventually,
// either:
//
//   A) The "push button" configuration process is stopped (because no one
//      answered at the other end of the link or because something failed)
//      after some technology-specific time.
//
//   B) The "push button" configuration is stopped because a new device has been
//      added. When this happens, a new message of type
//      "PLATFORM_QUEUE_EVENT_AUTHENTICATED_LINK" is posted to the system
//      queue.
//
// If there is a problem and the process cannot be started, this function
// returns "0", otherwise it returns "1"
//
// [PLATFORM PORTING NOTE]
//   If "interface_name" does not support the "push button" configuration
//   mechanism, this function should immediatley return "1".
//   Ie. a "PLATFORM_QUEUE_EVENT_AUTHENTICATED_LINK" message must *not* be
//   posted to the AL queue.
//
// [PLATFORM PORTING NOTE]
//   Note that once the process is started and until it finishes, if someone
//   calls "PLATFORM_GET_1905_INTERFACE_INFO()" on this interface, the field
//   'push_button_on_going' must return a value of "1".
//
INT8U PLATFORM_START_PUSH_BUTTON_CONFIGURATION(char *interface_name, INT8U queue_id, INT8U *al_mac, INT16U mid);


////////////////////////////////////////////////////////////////////////////////
/// Power control
////////////////////////////////////////////////////////////////////////////////

// Change the power mode of the provided interface.
//
// 'power_mode' can take any of the "INTERFACE_POWER_STATE_*" values
//
// The returned value can take any of the following values:
//   INTERFACE_POWER_RESULT_EXPECTED
//     The power mode has been applied as expected (ie. the new "power mode" is
//     the specified in the call)
//   INTERFACE_POWER_RESULT_NO_CHANGE
//     There was no need to apply anything, because the interface *already* was
//     in the requested mode
//   INTERFACE_POWER_RESULT_ALTERNATIVE
//     The interface power mode has changed as a result for this call, however
//     the new state is *not* the given one.  Example: You said
//     "INTERFACE_POWER_STATE_OFF", but the interface, due to maybe platform
//     limitations, ends up in "INTERFACE_POWER_STATE_SAVE"
//   INTERFACE_POWER_RESULT_KO
//     There was some problem trying to apply the given power mode
//
#define INTERFACE_POWER_RESULT_EXPECTED     (0x00)
#define INTERFACE_POWER_RESULT_NO_CHANGE    (0x01)
#define INTERFACE_POWER_RESULT_ALTERNATIVE  (0x02)
#define INTERFACE_POWER_RESULT_KO           (0x03)
INT8U PLATFORM_SET_INTERFACE_POWER_MODE(char *interface_name, INT8U power_mode);

////////////////////////////////////////////////////////////////////////////////
/// Security configuration
////////////////////////////////////////////////////////////////////////////////

// Configure an 80211 AP interface.
//
// 'interface_name' is one of the names previously returned in a call to
// "PLATFORM_GET_LIST_OF_1905_INTERFACES()".
// It must be an 802.11 interface with the role of "AP".
//
// 'ssid' is a NULL terminated string containing the "friendly" name of the
// network that the AP is going to create.
//
// 'bssid' is a 6 bytes long ID containing the MAC address of the "main" AP
// (typically the registrar) on "extended" networks (where several APs share the
// same security settings to make it easier for devices to "roam" between them).
//
// 'auth_mode' is the "authentication mode" the AP is going to use. It must take
// one of the values from "IEEE80211_AUTH_MODE_*"
//
// 'encryption_mode' is "encryption mode" the AP is going to use. It must take
// one of the values from "IEEE80211_ENCRYPTION_MODE_*"
//
// 'network_key' is a NULL terminated string representing the "network key" the
// AP is going to use.
//
INT8U PLATFORM_CONFIGURE_80211_AP(char *interface_name, char *ssid, INT8U *bssid, INT16U auth_mode, INT16U encryption_mode, char *network_key, INT8U network_type);

INT8U PLATFORM_BSS_IOCTL(char *interface_name, INT8U type);

INT8U PLATFORM_AP_CAPABILITY_IOCTL(char *ifname, INT8U tlv_type, INT8U *results);
INT32S PLATFORM_MAP_GET_GENERAL_IOCTL(char *ifname, INT8U *results, INT16U result_len);
INT32S PLATFORM_MAP_SET_GENERAL_IOCTL(char *ifname, INT8U *results, INT16U result_len);
INT8U PLATFORM_CLIENT_CAPABILITY_IOCTL(char *ifname, INT8U *macaddr, INT8U *results);
INT8U PLATFORM_AP_METRIC_IOCTL(char *ifname, INT8U tlv_type, INT8U *results);
INT8U PLATFORM_ASSOC_STA_IOCTL(char *ifname, INT8U tlv_type, INT8U *macaddr, INT8U *results);
INT8U PLATFORM_UNASSOC_STA_IOCTL(char *ifname, INT8U op_class, INT8U channel_nr, INT8U sta_nr, INT8U (*macaddr)[6]);
INT8U PLATFORM_METRIC_POLICY_IOCTL(char *interface_name, INT8U rcpi_threshold, INT8U rcpi_hysteris_margin, INT8U channel_utilization_threshold);
INT8U PLATFORM_STEERING_POLICY_IOCTL(char *interface_name, INT8U policy, INT8U channel_utilization_threshold, INT8U rcpi_threshold);
INT8U PLATFORM_BACKHAUL_STEER_IOCTL(char *interface_name, INT8U *backhaul_bss, INT8U *target_bssid, INT8U op_class, INT8U channel);

INT8U PLATFORM_SET_TX_MAX_POWER_IOCTL(char *interface_name, INT8U tx_max_power);

INT8U PLATFORM_CHANNEL_SCAN_IOCTL(char *interface_name, INT8U channel_nr, INT8U* channels);
INT8U PLATFORM_CAC_IOCTL(char *interface_name, INT8U channel_nr, INT8U* channels, INT8U type);  //type 0 : cac request.  1 : cac termination

INT8U PLATFORM_SET_MIB(char *interface_name, char *item_str, char *value_str);
INT8U PLATFORM_GET_MIB(char *interface_name, char *mibname, void *result, int size);

INT8U PLATFORM_GET_MAC(char *name, INT8U *mac_address);

INT8U PLATFORM_BTM_REQ(char *ifname, INT8U *macaddr, INT8U mode, INT8U *target_bssid, INT8U target_opclass, INT8U target_channel, INT16U disassoc_timer, INT8U reason_code);

INT8U PLATFORM_GET_BSS_TYPE(char* interface_name);

INT8U PLATFORM_ASSOC_CONTROL(char * interfacename, INT8U * sta_mac, INT16U timer, INT8U control);

INT8U PLATFORM_SEND_DISASSOC(char *interfacename, INT8U *sta_mac);

INT8U PLATFORM_DELETE_STA(char *interfacename, INT8U *sta_mac);

OP_CLASS *PLATFORM_GET_OP_CLASS(int *op_classes_len);

INT8U PLATFORM_CONVERT_TO_GLOBAL_OP_CLASS(INT8U op_class);

INT8U PLATFORM_CONVERT_TO_LOCAL_OP_CLASS(INT8U op_class);

#define FILTER_NONE 0
#define FILTER_5GL 1
#define FILTER_5GH 2
INT8U PLATFORM_GET_AVAILABLE_CHANNELS(char *interfacename, INT8U *buffer, INT16U buffer_size, INT8U filter);

char *PLATFORM_GET_REGDOMAIN();

INT8U PLATFORM_ISSUE_BEACON_MEASUREMENT(char *ifname, unsigned char *macaddr, struct BeaconMeasurementRequest *beacon_req);

INT8U PLATFORM_IS_INTERFACE_VALID(char *interface);

INT8U PLATFORM_IS_INTERFACE_UP(char *interface);

int PLATFORM_GET_INTERFACE_STATUS(char *interface, int *interface_index);

char *PLATFORM_GET_IP_ADDRESS();

// Check if interface is down or func_off
INT8U PLATFORM_IS_INTERFACE_UP_AND_ON(char *interface);

INT8U PLATFORM_NEED_HAPD();

INT32U PLATFORM_GET_PREFER_BSSID_INTERVAL();

void PLATFORM_RESET_PREFER_BSSID();

INT8U PLATFORM_INTERFACE_HAS_AX_SUPPORT(char *interface);

INT8U PLATFORM_GET_ETHERNET_CLIENTS(INT8U *neighbor_mac_addresses_nr, struct ethernetNeighborInfo **neighbor_list, char *connected_interface);

INT8U PLATFORM_TRIGGER_WPS(char *interface);

INT8U PLATFORM_CHECK_CONNECTED_MAC(char *interface_name, INT8U *connected_address);

INT8U PLATFORM_GET_CLIENT_RCPI(char *interface_name, INT8U *neighbor_interface_address, INT8U allow_sta_mode);

void PLATFORM_SET_SP_RULE(struct ServicePrioritizationRuleTLV *rule);

void PLATFORM_CLEAR_SP_RULE();

void PLATFORM_SET_DSCP_PCP_MAPPING_TABLE(struct DSCPMappingTableTLV *table);

void PLATFORM_ALL_RADIOS_TLV_SET(INT8U *tlv);

void PLATFORM_MAP_ISSUE_CCE_IE_INDICATION(char *ifname, INT8U include_cce_ie);

void PLATFORM_MAP_START_RECONFIG(char *ifname);

void PLATFORM_MAP_APPLY_SPATIAL_REUSE_CONFIG(struct SpatialReuseRequestTLV *request, char *interface_name);

void PLATFORM_MAP_GET_SPATIAL_REUSE_CONFIG_RESPONSE(struct SpatialReuseConfigResponseTLV **response, INT8U *response_nr);

INT8U PLATFORM_MAP_GET_SPATIAL_REUSE_REPORT(INT8U *buffer, char *interface_name);
#endif
