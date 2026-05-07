#ifndef MAP_VENDOR_API_H
#define MAP_VENDOR_API_H

#include <unistd.h>

#include "map_tlvs.h"
#include "map_utils.h"
#include "map_commands.h"

#define CMDU_TYPE_TOPOLOGY_DISCOVERY               0x0000
#define CMDU_TYPE_TOPOLOGY_NOTIFICATION            0x0001
#define CMDU_TYPE_TOPOLOGY_QUERY                   0x0002
#define CMDU_TYPE_TOPOLOGY_RESPONSE                0x0003
#define CMDU_TYPE_VENDOR_SPECIFIC                  0x0004
#define CMDU_TYPE_LINK_METRIC_QUERY                0x0005
#define CMDU_TYPE_LINK_METRIC_RESPONSE             0x0006
#define CMDU_TYPE_AP_AUTOCONFIGURATION_SEARCH      0x0007
#define CMDU_TYPE_AP_AUTOCONFIGURATION_RESPONSE    0x0008
#define CMDU_TYPE_AP_AUTOCONFIGURATION_WSC         0x0009
#define CMDU_TYPE_AP_AUTOCONFIGURATION_RENEW       0x000a

#define TLV_TYPE_VENDOR_SPECIFIC                     (11)

#define MAP_ROLE_AGENT 0
#define MAP_ROLE_CONTROLLER 1

void vendor_register_message_receive_call_back_functions(uint8_t device_role);

void vendor_register_append_tlv_call_back_functions(uint8_t device_role);

#endif