/*
 *  Realtek Semiconductor Corp.
 *
 */

#ifndef _MULTI_AP_CMDUS_R3_H_
#define _MULTI_AP_CMDUS_R3_H_

////////////////////////////////////////////////////////////////////////////////
/// Profile 3 CMDUs
////////////////////////////////////////////////////////////////////////////////

#define CMDU_TYPE_DPP_CCE_INDICATION                                0x801D  // p91 of R3 spec.
#define CMDU_TYPE_1905_REKEY_REQUEST                                0x801E
#define CMDU_TYPE_1905_DECRYPTION_FAILURE                           0x801F
#define CMDU_TYPE_SERVICE_PRIORITIZATION_REQUEST                    0x8023
#define CMDU_TYPE_PROXIED_ENCAP_DPP                                 0x8029
#define CMDU_TYPE_DIRECT_ENCAP_DPP                                  0x802A
#define CMDU_TYPE_RECONFIG_TRIGGER                                  0x802B
#define CMDU_TYPE_BSS_CONFIG_REQUEST                                0x802C
#define CMDU_TYPE_BSS_CONFIG_RESPONSE                               0x802D
#define CMDU_TYPE_BSS_CONFIG_RESULT                                 0x802E
#define CMDU_TYPE_CHIRP_NOTIFICATION                                0x802F
#define CMDU_TYPE_1905_ENCAP_EAPOL                                  0x8030
#define CMDU_TYPE_DPP_BOOTSTRAPPING_URI_NOTIFICATION                0x8031
#define CMDU_TYPE_FAILED_CONNECTION                                 0x8033
#define CMDU_TYPE_AGENT_LIST                                        0x8035


#endif // END _MULTI_AP_CMDUS_R3_H_
