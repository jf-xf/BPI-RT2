/*
 * Copyright (C) 2020 Realtek Semiconductor Corp. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 */

#ifndef _EASYMESH_UTILS_H_
#define _EASYMESH_UTILS_H_

#include <stdint.h>
#include "map_tlvs.h"
#include "map_initialization.h"
#include "config_file_handler.h"
#include "easymesh_datamodel.h"

#define VERBOSE_LEVEL_FILE "/tmp/map_verbose"

#define MAP_CONFIGURE_BAND_24G 0x01
#define MAP_CONFIGURE_BAND_5GL 0x02
#define MAP_CONFIGURE_BAND_5GH 0x04
#define MAP_CONFIGURE_BAND_5GF 0x06

#define MAP_CONFIGURE_BAND_FULL 0x07

#define EXTENDED_CAP_IE         127
#define RM_ENABLE_CAP_IE        70

//ioctl private map general
enum MAP_GENERAL_IOCTL_OPERATIONAL {
	MAP_RESERVED,
	MAP_GET_GENERAL,
	MAP_SET_GENERAL,
	MAP_SEND_5G_BSS_MACS,
	MAP_GET_LINK_METRICS
};

enum MAP_BAND_WIDTH {
	BAND_WIDTH_20MHZ,
	BAND_WIDTH_40MHZ,
	BAND_WIDTH_80MHZ,
	BAND_WIDTH_160MHZ,
	BAND_WIDTH_80_80MHZ
};

enum MAP_SIDE_BAND {
	SIDE_BAND_NONE,
	SIDE_BAND_LOWER,
	SIDE_BAND_UPPER
};

enum MAP_RADIO_BAND {
	RADIO_BAND_2G,
	RADIO_BAND_5GL,
	RADIO_BAND_5GH,
	RADIO_BAND_5GF,
	RADIO_BAND_ETH = 0xD0,
	RADIO_BAND_UNKNOWN = 0xFF
};

enum MAP_BSTA_BAND_MASK {
	MASK_BAND_2G = 0x01,
	MASK_BAND_5GL = 0x02,
	MASK_BAND_5GH = 0x04
};

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

enum {
    MAP_LINK_DOWN,
    MAP_LINK_UP
};

static const uint8_t REALTEK_OUI[3]       = {0x00, 0xE0, 0x4C};

struct map_service_thread_data {
	const char *              queue_name;
	uint8_t                   device_role;
	uint8_t                   multiap_profile;
	uint8_t                   configure_state;
	struct default_map_config default_setting;
	char *                    al_interfaces;
	uint8_t                   config_nr;
	struct radio_config_data *config_data;
	uint8_t                   wfa_test_mode;
};

struct config_data_struct {
	//global
	uint16_t    alme_port_number;
	uint8_t     max_resend_time;
	char *      device_name;
	uint8_t     rssi_weightage;
	uint8_t     path_weightage;
	uint8_t     cu_weightage;
	uint8_t     roam_score_difference;
	uint8_t     min_evaluation_interval;
	uint8_t     min_roam_interval;
	uint8_t     max_num_device_allowed;
	uint32_t    throughput_threshold;
	uint8_t     max_renew_resend_time;
	uint16_t    primary_vid;
	uint8_t     default_pcp;
	char *      radio_name_5gh;
	char *      radio_name_5gl;
	char *      radio_name_24g;
	//per radio config data
	uint8_t                   config_data_nr;
	struct radio_config_data *per_radio_config_data;
};

typedef struct vlan_config {
	char *ssid;
	uint16_t vid;
} MAP_VLAN_CONFIG_T;

struct easymesh_version {
	uint8_t generation;
	uint8_t major;
	char    minor;
	uint8_t update;
	uint8_t stage;
};

struct customized_status {
	uint8_t physical_dist;
	uint8_t logical_dist;
};

static const uint8_t GLOBAL_20MHZ_OP_CLASS[] = {
	81, 82, 115, 118, 121, 124, 125
};

typedef struct _OP_CLASS_ {
	uint8_t        op_class;
	uint8_t        op_class_len;
} OP_CLASS;

extern const char *VALID_INTERFACES_IN_BRIDGE[];

extern const char *VALID_ETH_INTERFACES_IN_BRIDGE[];

void *map_service_thread(void *p);

char *get_valid_interface_name(const char *interface_name, const char **valid_interfaces);

void append_br_interfaces(char *bridge_name, char *interfaces);

uint8_t is_interface_up(char *interface);

uint8_t configureDevice(uint8_t wfa_mode, uint8_t config_nr, struct radio_config_data *config_data, uint8_t *is_identical, struct easymesh_datamodel *data_container, MAP_VLAN_CONFIG_T **vlan_config, uint8_t *vlan_config_nr);

uint8_t configure_interface(char *interface_name, char *ssid, char *network_key, uint8_t auth_type, uint8_t encrypt_type, uint8_t network_type, uint8_t bss_index, char *signed_connector, struct easymesh_datamodel *data_container);

uint8_t configure_aps_func_off(uint8_t is_controller, uint8_t configure_state, struct easymesh_datamodel *database);

uint8_t configure_vxds_func_off(uint8_t value, uint8_t radio_num);

uint8_t convert_to_global_op_class(uint8_t op_class, int domain);

uint8_t get_band_from_op_class(uint8_t op_class);

uint8_t set_radio_channel(uint8_t band, uint8_t channel, uint8_t band_width, uint8_t sideband, struct easymesh_datamodel *data_container);

uint8_t process_rtk_vendor_spec_tlv(struct vendorSpecificTLV *vendor_tlv, uint8_t radio_data_num, struct easymesh_radio_mib * radio_data, char *vap_prefix);

uint8_t buffer_mib_channel(struct easymesh_datamodel * data_container);

void get_radio_interface(const char *radio_name, const char *prefix, const int idx, char *ifname);

void add_single_radio_interface(char **buffer, const char *radio_name, const char *prefix, const int idx, char *ifname);

void add_radio_interfaces(char **buffer, const char *radio_name, const char *vap_prefix, int vap_number, int vxd_number);

uint8_t obtainRadioBandInfo(char* radio_name, int *bandwidth, int *sideband);

uint8_t isUnoperableOpClass(uint8_t op_class, int bandwidth, int sideband);

void getOpClassBandInfo(uint8_t op_class, int *bandwidth, int *sideband);

uint64_t convertVersionStringToNumber(char *device_version);

uint8_t update_hop_to_controller(char* interface_name, uint8_t distance);

uint8_t update_customized_status_to_file(struct customized_status *cus_status);

uint8_t *get_ie_pointer(uint8_t max_len, uint8_t *pbuf, uint8_t ie, uint8_t *len);

uint8_t check_bss_transition_support(uint8_t *frame, uint8_t frame_length);

void update_backhaul_link_status(int status, char *interface_name);

uint8_t append_vlan_config(MAP_VLAN_CONFIG_T **vlan_config, uint8_t *vlan_config_nr, char * ssid, uint8_t ssid_len, uint16_t vid);

int cmpfunc(const void * a, const void * b);

void init_ctomusized_status(struct customized_status *cus_status);

uint8_t* get_wsc_attribute(uint8_t *m, uint16_t m_size, uint16_t attribute_id);

void update_customized_information(struct customized_status_info status_info);

uint8_t update_status(uint8_t *current_status, uint8_t next_status);

OP_CLASS *get_op_class_with_reg_domain(int domain);

uint8_t map_set_5g_bss_macs_ioctl(char *interface_name, uint8_t *buffer, uint16_t buffer_size);

#if !defined(__UCLIBC__)
extern size_t strlcpy(char *dst, const char *src, size_t dsize);
extern size_t strlcat(char *dst, const char *src, size_t dsize);
#endif

void *zalloc(uint32_t size);

uint8_t map_get_mac(char *name, uint8_t *mac_address);

int ifconfig_helper(const char *if_name, int up);
#endif
