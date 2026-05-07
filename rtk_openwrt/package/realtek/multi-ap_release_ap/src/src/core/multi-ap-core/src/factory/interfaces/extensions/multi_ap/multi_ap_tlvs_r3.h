/*
 *  Realtek Semiconductor Corp.
 *
 */

#ifndef _MULTI_AP_TLVS_R3_H_
#define _MULTI_AP_TLVS_R3_H_

#include "platform.h"

////////////////////////////////////////////////////////////////////////////////
/// Profile 3 TLVs
////////////////////////////////////////////////////////////////////////////////
#define TLV_TYPE_1905_LAYER_SECURITY_CAPABILITY                    (169) // 0xA9
#define TLV_TYPE_AP_WIFI_6_CAPABILITIES                            (170) // 0xAA
#define TLV_TYPE_MIC                                               (171) // 0xAB
#define TLV_TYPE_ENCRYPTED_PAYLOAD                                 (172) // 0xAC
#define TLV_TYPE_ASSOCIATED_WIFI_6_STA_STATUS_REPORT               (176) // 0xB0
#define TLV_TYPE_BSS_CONFIG_REPORT                                 (183) // 0xB7
#define TLV_TYPE_BSSID                                             (184) // 0xB8
#define TLV_TYPE_SERVICE_PRIORITIZATION_RULE                       (185) // 0xB9
#define TLV_TYPE_DSCP_MAPPING_TABLE                                (186) // 0xBA
#define TLV_TYPE_BSS_CONFIG_REQUEST                                (187) // 0xBB
#define TLV_TYPE_BSS_CONFIG_RESPONSE                               (189) // 0xBD
#define TLV_TYPE_AKM_SUITE_CAPABILITIES                            (204) // 0xCC
#define TLV_TYPE_1905_ENCAP_DPP                                    (205) // 0xCD
#define TLV_TYPE_1905_ENCAP_EAPOL                                  (206) // 0xCE
#define TLV_TYPE_DPP_BOOTSTRAPPING_URI_NOTIFICATION                (207) // 0xCF
#define TLV_TYPE_DPP_MESSAGE                                       (209) // 0xD1
#define TLV_TYPE_DPP_CCE_INDICATION                                (210) // 0xD2
#define TLV_TYPE_DPP_CHIRP_VALUE                                   (211) // 0xD3
#define TLV_TYPE_DEVICE_INVENTORY                                  (212) // 0xD4
#define TLV_TYPE_AGENT_LIST                                        (213) // 0xD5

////////////////////////////////////////////////////////////////////////////////
/// Profile 3 TLVs
////////////////////////////////////////////////////////////////////////////////

// 17.2.67

#define PROTOCOL_1905_DEVICE_PROVISIONING     0x00
#define ALGORITHM_HMAC_SHA254                 0x00
#define ENCRYTPTION_AES_SIV                   0x00

#define MAP_DPP_RELAY_HEADER_PROXIED_ENCAP (0x09)



struct SecurityCapabilityTLV
{
	INT8U	tlv_type;

	INT8U	onboarding_protocol_supported;

	INT8U	message_integrity_algorithm_supported;

	INT8U	message_encryption_algorithm_supported;
};

// 17.2.72

#define MCS_NSS_NUMBER_MASK  0x0F
struct AgentRole
{
    INT8U   flags;  // bits 7-6 : 0   - Wi-Fi 6 support info for the AP role
                    //            1   - Wi-Fi 6 support info for the non-AP STA role
                    //            2-3 - Reserved
                    // bit 5    : Support for HE 160 MHz,   0 : not supported, 1: supported
                    // bit 4    : Support for HE 80+80 Mhz  0 : not supported, 1: supported
                    // bits 3-0 : length of MCS NSS

    INT8U  *mcs_nss; // 4 or 8 or 12 octets

    INT8U   wifi6_capability_1; // bit 7 : Support for SU Beamformer
                                // bit 6 : Support for SU Beamformee
                                // bit 5 : Support for MU beamformer Status
                                // bit 4 : Support for Beamformee STS <= 80 MHz
                                // bit 3 : Support for Beamformee STS > 80 MHz
                                // bit 2 : Support for UL MU-MIMO
                                // bit 1 : Support for UL OFDMA
                                // bit 0 : Support for DL OFDMA
    INT8U   wifi6_capability_2; // bits 7-4 : Max number of users supported per DL MU-MIMO TX in AP role
                                // bits 3-0 : Max number of users supported per UL MU-MIMO RX in AP role
    INT8U   wifi6_capability_3; // Max number of users supported per DL OFDMA TX in AP role
    INT8U   wifi6_capability_4; // Max number of users supported per UL OFDMA RX in AP role
    INT8U   wifi6_capability_5; // bit 7    : Support for RTS
                                // bit 6    : Support for MU RTS
                                // bit 5    : Support for Multi-BSSID
                                // bit 4    : Support for MU EDCA
                                // bit 3    : Support for TWT Requester
                                // bit 2    : Support for TWT Responder
                                // bits 1-0 : Reserved
};

struct APWiFi6CapabilitiesTLV
{
    INT8U               tlv_type;

    INT8U               radio_unique_identifier[6];

    INT8U               role_nr;

    struct AgentRole    *agent_roles;
};

// 17.2.68

#define MIC_VERSION_1   0x0
struct MICTLV
{
    INT8U       tlv_type;

    INT8U       gtk_key_id_ver; // bit 7-6 : 1905 GTK Key Id
                                // bit 5-4 : MIC Version
                                // bit 3-0 : Reserved

    INT64U      integrity_transmission_counter;

    INT8U       src_1905_al_mac_addr[6];

    INT16U      mic_length;

    INT8U       mic[64];
};

// 17.2.69

struct EncryptedPayloadTLV
{
    INT8U   tlv_type;

    INT64U  encryption_transmission_counter;

    INT8U   source_1905_mac_address[6];

    INT8U   dest_1905_mac_address[6];

    INT16U  aes_siv_len;

    INT8U   *aes_siv;
};

// 17.2.73

struct Queue
{
    INT8U   tid;

    INT8U   queue_size;
};

struct AssociatedWiFi6STAStatusReportTLV
{
    INT8U           tlv_type;

    INT8U           mac_address[6];

    INT8U           queue_nr;

    struct Queue*   queues;

};

// 17.2.74

struct BSSIDTLV
{
    INT8U   tlv_type;

    INT8U   bssid[6];
};

// 17.2.75

#define BSS_CONFIG_BACKHAUL_BSS 0x80
#define BSS_CONFIG_FRONTHAUL_BSS 0x40
#define BSS_CONFIG_R1_DISALLOW 0x20
#define BSS_CONFIG_R2_DISALLOW 0x10
#define BSS_CONFIG_MULTI_BSSID 0x08
#define BSS_CONFIG_TX_BSSID 0x04

struct BSSConfigBSSReport{
    INT8U   bssid[6];

    INT8U   status_flag; // bit 7    : backhaul BSS,            0: in use, 1: not in use
                         // bit 6    : fronthaul BSS,           0: in use, 1: not in use
                         // bit 5    : R1 disallowed status,    0: allowed, 1: disallowed
                         // bit 4    : R2 disallowed status,    0: allowed, 1: disallowed
                         // bit 3    : multiple BSSID set,      0: not-configured, 1: configured
                         // bit 2    : transmitted BSSID,       0: non-transmitted, 1: transmitted
                         // bits 1-0 : reserved

    INT8U   reserved;

    INT8U   ssid_length;

    char    *ssid;
};

struct BSSConfigRadioReport{
    INT8U                           radio_unique_identifier[6];

    INT8U                           bss_report_nr;

    struct BSSConfigBSSReport*      bss_reports;
};

struct BSSConfigReportTLV
{
    INT8U                          tlv_type;

    INT8U                          radio_report_nr;

    struct BSSConfigRadioReport*   radio_reports;
};

// 17.2.70

#define ADD_REMOVE_RULE_BIT_MASK 0x80
#define ALWAYS_MATCH_BIT_MASK 0x80
struct ServicePrioritizationRuleTLV {
	INT8U tlv_type;

	INT32U service_prioritization_rule_id;

	INT8U add_remove_rule; // bit 7    : 0 - remove this filter, 1 - add this filter
	                       // bits 6-0 : reserved

	INT8U rule_precedence; // higher number means higher priority
	                       // 0x00-0xFE : rule precendence
	                       // 0xFF      : reserved

	INT8U rule_output; // value of or method used to select the 802.1Q C-TAG PCP value
	                   // with which to mark the matched packet
	                   // 0x00-0x09 : rule output
	                   // 0x0A-0xFF : reserved

	INT8U always_match; // bit 7    : 0 or 1
	                    // bits 6-0 : reserved
};

// 17.2.71

struct DSCPMappingTableTLV
{
    INT8U   tlv_type;

    INT8U   dscp_to_pcp[64];    // List of 64 PCP values (one octet per value) corresponding to the DSCP
                                // markings 0x00 to 0x3F, ordered by increasing DSCP value.
                                // This table is used to select a PCP value if a Service Prioritization Rule
                                // specifies Rule Output: 0x08
                                // DSCP to PCP mapping table
                                // 0x00-0x07 : each octet
                                // 0x08-0xFF : reserved
};

// 17.2.84

struct BSSConfigRequestTLV
{
    INT8U   tlv_type;

    INT16U  dpp_config_request_object_len;

    INT8U   *dpp_config_request_object;
};

// 17.2.85

struct BSSConfigResponseTLV
{
    INT8U   tlv_type;

    INT16U  dpp_config_object_len;

    INT8U   *dpp_config_object;
};

// 17.2.78

struct AKMSuiteSelector
{
    INT8U   oui[3];

    INT8U   akm_suite_type;
};

struct AKMSuiteCapabilitiesTLV
{
    INT8U   tlv_type;

    INT8U   backhaul_bss_akm_suite_selectors_nr;

    struct  AKMSuiteSelector* backhaul_bss_akm_suite_selectors;

    INT8U   fronthaul_bss_akm_suite_selectors_nr;

    struct  AKMSuiteSelector* fronthaul_bss_akm_suite_selectors;

};

// 17.2.79

struct Encap1905DPPTLV
{
    INT8U   tlv_type;

    INT8U   enrollee_mac_address_present; // bit 7    : 0 or 1, to specify the address of the Enrollee MAP Agent
                                          //            to the Proxy Agent and MAP Controller
                                          // bit 6    : reserved
                                          // bit 5    : content type of the encapsulated DPP frame
                                          //            0 - DPP Public Action frame, 1 - GAS frame
                                          // bits 4-0 : reserved

    INT8U   dest_sta_mac_address[6]; // present if the Enrollee MAC Address present bit is set to one

    INT8U   frame_type; // if the DPP Frame Indicator (bit 5) is 0, set to the DPP Public Action frame type
                        // else, set to 255

    INT16U   encapsulated_frame_len;

    INT8U   *encapsulated_frame; // if bit 5 is 0, contains a DPP Public Action frame
                                 // else, contains a GAS frame that carries the DPP Configuration Protocol payload

};

// 17.2.80

struct Encap1905EAPOLTLV
{
    INT8U   tlv_type;

    INT16U  eapol_frame_payload_len;

    INT8U   *eapol_frame_payload;
};

// 17.2.81

struct DPPBootstrappingURINotificationTLV
{
    INT8U   tlv_type;

    INT8U   radio_unique_identifier[6];     // Radio Unique Identifier of a radio

    INT8U   local_interface_mac_address[6]; // MAC Address of Local Interface (equal to BSSID) operating on the radio, on which the URI was received during PBC onboarding.

    INT8U   sta_mac_address[6];             // MAC Address of bSTA from which the URI was received during PBC onboarding.

    INT16U   dpp_bootstrapping_uri_len;

    INT8U   *dpp_bootstrapping_uri;         // DPP Bootstrapping URI received during PBC onboarding.

};

// 17.2.86

struct DPPMessageTLV
{
    INT8U   tlv_type;

    INT16U  dpp_frame_len;

    INT8U   *dpp_frame;
};

// 17.2.82

struct DPPCCEIndicationTLV
{
    INT8U   tlv_type;       // 0xD2  210

    INT8U   advertise_cce;  // Enable/Disable CCE advertisement in beacons and probe responses
                            // 0 : disable, 1 : enable
};

// 17.2.83

#define DPP_CHIRP_ENROLLEE_ADDR_PRESENT 0x80
#define DPP_CHIRP_HASH_VALIDITY 0x40

struct DPPChirpValueTLV
{
    INT8U   tlv_type;

    INT8U   hash_validity;  // bit 7    : 0 or 1, Enrollee MAC Address Present
                            // bit 6    : Establish/purge any DPP authentication state pertaining to the
                            //            hash value in this TLV
                            //            0 - purge, 1 - establish
                            // bits 5-0 : 0, reserved

    INT8U   sta_mac_address[6]; // present if the Enrollee MAC Address present bit is set to one

    INT8U   hash_len;

    INT8U   *hash_value;

};

// 17.2.76

struct DeviceInventoryRadio
{
    INT8U   radio_unique_identifier[6];

    INT8U   lcv;                // Length of the ChipsetVendor string (octets). Less than or equal to 64

    INT8U   *chipset_vendor;    // A string identifying the Wi-Fi chip vendor of this radio
};

struct DeviceInventoryTLV
{
    INT8U   tlv_type;

    INT8U   lsn;                // Length of SerialNumber string (octets). Less than or equal to 64

    INT8U   *serial_number;     // A string Identifying the particular device that is unique for the indicated model and manufacturer

    INT8U   lsv;                // Length of the SoftwareVersion string (octets). Less than or equal to 64

    INT8U   *software_version;  // A string identifying the software version currently installed in the device
                                // (i.e. version of the overall device firmware)

    INT8U   lee;                // Length of the ExecutionEnv string (octets). Less than or equal to 64

    INT8U   *execution_env;     // A string identifying the execution environment (operating system) in the device

    INT8U   radio_nr;           // NUmber of radios

    struct  DeviceInventoryRadio* radios;
};

// 17.2.77

struct Agent {
	INT8U al_mac_address[6];

	INT8U map_profile; // Highest profile supported by agent as indicated in its MAP TLV
	                   // 0x00: Reserved
	                   // 0x01: MAP R1
	                   // 0x02: MAP R2
	                   // 0x03: MAP R3
	                   // 0x04-0xFF: Reserved

	INT8U security; // 0x00: 1905 Security not enabled
	                // 0x01: 1905 Security enabled
	                // 0x02-0xFF: Reserved
};

struct AgentListTLV {
	INT8U tlv_type;

	INT8U agents_nr;

	struct Agent *agents;
};

#endif // END _MULTI_AP_TLVS_R3_H_
