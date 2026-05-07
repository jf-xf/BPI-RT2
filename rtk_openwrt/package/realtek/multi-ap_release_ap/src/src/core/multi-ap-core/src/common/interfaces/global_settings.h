/*
 * Copyright (C) 2018 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _GLOBAL_SETTINGS_H_
#define _GLOBAL_SETTINGS_H_

#define OLD_RELEASE_VERSION 72977889548566528 // 1.0 .. 1.3E ((1 << 56) | (0x03 << 48) | (0x45 << 40))

#define V_1_3_F_RELEASE_VERSION 72978984798781440 // 1.0 .. 1.3FBeta

#define OLD_BETA_RELEASE_VERSION 73183493961547776 // 1.4 Beta ((1 << 56) | (0x04 << 48) | (0x00 << 40) | (0x01 << 24))

// Entering of peer's bootstrap URI / own's private key can be done through
// writing the following tmp files:
//
#define DPP_BOOTSTRAP_INFO_MAX_LEN 640

extern char *map_radio_name_24g;

extern char *map_radio_name_5gl;

extern char *map_radio_name_5gh;

extern char *map_vap_prefix;

extern char *map_vxd_prefix;

extern char *map_bridge_name;

extern INT16U map_max_sta_num;

extern INT8U wps_type;

extern INT8U map_supported_service;

extern INT32U customize_features;

extern char *interface_manufacturer_name;
extern char *interface_model_name;
extern char *interface_device_name;

extern char *hw_country_str;

#define CUSTOMIZE_WSC_INFO		0x00000001  //Modify for AUTH/ENCRYPTION FLAGS in autconfig M1/M2 packet to fit CT SPEC
#define SUPPORT_OPEN_ENCRYPT	0x00000002  //multi-ap support open encrypt
#define SUPPORT_NOT_WPS_CYCLE	0x00000004  //multi-ap wps not cycle
#define SUPPORT_ETHERNET_ADAPTER	0x00000008  //multi-ap support ethernet adapter
#define SUPPORT_SHOW_EXTRA_TOPOLOGY	0x00000010  //multi-ap show extra topology
#define FRONTBSS_DEFAULT_ENDABLE	0x00000020  //default enable fronthaul bss before easymesh autoconfig

//for customize_features
#define BIT0	0x00000001  //customize M1/M2 info for ct
#define BIT1	0x00000002  //support open encrypt
/* to be used
#define BIT2	0x00000004
#define BIT3	0x00000008
#define BIT4	0x00000010
#define BIT5	0x00000020
#define BIT6	0x00000040
#define BIT7	0x00000080
#define BIT8	0x00000100
#define BIT9	0x00000200
#define BIT10	0x00000400
#define BIT11	0x00000800
#define BIT12	0x00001000
#define BIT13	0x00002000
#define BIT14	0x00004000
#define BIT15	0x00008000
#define BIT16	0x00010000
#define BIT17	0x00020000
#define BIT18	0x00040000
#define BIT19	0x00080000
#define BIT20	0x00100000
#define BIT21	0x00200000
#define BIT22	0x00400000
#define BIT23	0x00800000
#define BIT24	0x01000000
#define BIT25	0x02000000
#define BIT26	0x04000000
#define BIT27	0x08000000
#define BIT28	0x10000000
#define BIT29	0x20000000
#define BIT30	0x40000000
#define BIT31	0x80000000
*/

extern char *controller_interface;

extern char *controller_al_interface;

extern INT8U  eth_only;

extern INT8U  hw_reg_domain;

extern INT8U  radio_data_nr; //radio number

extern struct easymesh_radio_mib *radio_data;

#endif
