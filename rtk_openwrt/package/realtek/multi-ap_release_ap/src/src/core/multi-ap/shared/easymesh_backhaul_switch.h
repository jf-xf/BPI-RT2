#ifndef _EASYMESH_BACKHAUL_SWITCH_H_
#define _EASYMESH_BACKHAUL_SWITCH_H_

#include <stdint.h>
#include "easymesh_datamodel.h"

enum {
	STATE_UP,
	STATE_DOWN
};

int mixed_backhaul_sta_trigger(char *on_vxd_interface, struct easymesh_datamodel *database);

void set_vxd_interface_state(char *interface_name, uint16_t primary_vid, uint8_t state);

void up_backhaul_sta(char *radio_name_5gh, char *radio_name_5gl, char *radio_name_2g, uint16_t primary_vid, uint8_t radio_byte);

void set_backhaul_interface(char *interface);

char *get_backhaul_interface();


#endif